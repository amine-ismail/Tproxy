#include "includes.h"
#include "connection.h"
#include "net.h"
#include "log.h"

int createSocket (void) {
  int s;
  if ((s = socket(AF_INET,SOCK_STREAM,0))== -1) {
    perror ("Can't create socket:");
    exit(0);
  }
  return s;
}

//==========================================

int initSocket (int s, uint8_t non_blocking){
  
  int one=1;
  
  //TODO: setuid should be done here
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,(char*)&one, sizeof one) != 0) {
    perror ("SO_REUSEADDR error:");
  }
  if (setsockopt(s, SOL_IP, IP_FREEBIND, (char*)&one, sizeof(one)) != 0) {
    perror("IP_FREEBIND error");
  }
#ifdef IP_TRANSPARENT
  if (setsockopt(s, SOL_IP, IP_TRANSPARENT, (char*)&one, sizeof(one)) !=0) {
    perror ("IP_TRANSPARENT error:");
  }
#endif
  if (non_blocking) {
    // Set non blocking
    setSockNonBlocking(s);
  }
  else {
    setSockBlocking(s);
  }
  return 0;
}

//==========================================

int bindSocketToInterface (int fd, char * ifname)
{
  struct ifreq ifr;
  int tmp_fd=0;

  tmp_fd=socket(AF_INET, SOCK_DGRAM, 0);
  /* Bind to IP address */
  memset(&ifr, 0, sizeof(ifr));
  /* I want to get an IPv4 IP address */
  ifr.ifr_addr.sa_family = AF_INET;
  /* I want IP address attached to ifname */
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
  ioctl(tmp_fd, SIOCGIFADDR, &ifr);
  close(tmp_fd);
  ((struct sockaddr_in *)&ifr.ifr_addr)->sin_port=0;
  if(bind(fd, (struct sockaddr *)&ifr.ifr_addr, sizeof(ifr.ifr_addr)) == -1) {
    logError ("ERROR: bind fd=%d iface= %s Address=%s port=%d:%s", 
	      fd, ifname, 
	      inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
	      ((struct sockaddr_in *)&ifr.ifr_addr)->sin_port,strerror(errno));
    perror("Bind to local interface error:");
    exit(1);
  }
  return 0;
}

//==========================================

uint8_t isConnected (int s) {
  struct sockaddr_in adr_inet;/* AF_INET */
  int len_inet;   /* length  */
  int z;
  if (s<=0) {
    return 0;
  }
  len_inet = sizeof (adr_inet);
  z = getpeername(s,
		  (struct sockaddr *)&adr_inet,
		  (socklen_t *)&len_inet);
  if ( z == 0 ) {
    return 1; //connected
  }
  
  if ((z == -1 ) && (errno == ENOTCONN)) {
    return 0; //not connected
  }

  //an error ocured
  logError (" Socket error fd=%d", s);
  perror ("");
  return 0; //not connected
}

//======================================

int getOriginalDstAddr (int s, struct sockaddr * dst_adr) {
  int len=0;
  int z;
  len =  sizeof(struct sockaddr_in);
#ifdef  SO_ORIGINAL_DST
  z=getsockopt(s,SOL_IP,SO_ORIGINAL_DST,(struct sockaddr *) dst_adr,
	       (socklen_t *) &len);
#else
  z=getsockname(s, (struct sockaddr *)dst_adr, &len);
#endif
  if (z<0){
    perror ("getSockname SO_ORIGINAL_DST Error:");
  }
  return z;
}


void setSockBlocking (int s) {
  int x;
  x=fcntl(s,F_GETFL,0);
  fcntl(s,F_SETFL,x & (~O_NONBLOCK));
}

void setSockNonBlocking (int s) {
  int x;
  x=fcntl(s,F_GETFL,0);
  fcntl(s,F_SETFL,x | O_NONBLOCK);
}
