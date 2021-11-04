#include "includes.h"
#include "net.h"
#include "connection.h"
#include "log.h"



connection_t * newConnection (int fd, struct sockaddr_in client_address, 
			      int len_addr) {
  connection_t * c = NULL;
  int z = 0;
  struct sockaddr_in adr_inet;

  c = (connection_t *) malloc (sizeof(connection_t));
  memset (c, 0, sizeof(connection_t));
  
  c->id = fd; //?
  c->fd = fd;
  //c->iface = 0; //TODO
  c->status = CON_STS_NOTCONNECTED;
  strcpy(c->client_addrString, inet_ntoa(client_address.sin_addr));
  c->client_localPort = ntohs(client_address.sin_port);
  z=getOriginalDstAddr(fd, (struct sockaddr*)&adr_inet);
 
  if (z==0){
    strcpy(c->server_addrString, 
	   inet_ntoa(adr_inet.sin_addr));
    c->server_port = ntohs(adr_inet.sin_port);
  }
  memcpy (&(c->client), &client_address, sizeof (struct sockaddr_in));
  memcpy (&(c->server), &adr_inet, sizeof (struct sockaddr_in));

  return c;
}

//==================================================

connection_t * newConnectionPeer (connection_t * c1) {
  
  connection_t * c2 = NULL;
  int fd;
  struct sockaddr_in addr;

  fd = createSocket();
  initSocket (fd, 1); //1 for non blocking socket
  
  c2 = (connection_t *) malloc (sizeof(connection_t));
  memset (c2, 0, sizeof(connection_t));
  c2->id = fd; //?
  c2->fd = fd;
  //c2->iface = 0; //TODO
  c2->status = CON_STS_NOTCONNECTED;

  strcpy(c2->client_addrString, c1->client_addrString);
  c2->client_localPort = c1->client_localPort;
  strcpy(c2->server_addrString, c1->server_addrString);
  c2->server_port = c1->server_port;
  
  memcpy (&(c2->client), &(c1->client), sizeof (struct sockaddr_in ));
  memcpy (&(c2->server), &(c1->server), sizeof (struct sockaddr_in ));

  //bind socket to foreing address
  memcpy (&addr, &(c2->client), sizeof (struct sockaddr_in ));  
#ifdef MY_TRANSPARENT_PROXY
  addr.sin_port = 0; // The source port will be changed
#else
  addr.sin_port = 0; // The source port will be changed
  addr.sin_addr.s_addr = htonl(INADDR_ANY); 
#endif
  if (bind(c2->fd, (struct sockaddr *)&addr,sizeof addr) !=0) {
    perror ("bind error:");
    exit (0);
  }
  c1->peer = (struct connection_t*)c2;
  c2->peer = (struct connection_t*)c1;
  return c2;

}

//==================================================

void fillWriteBuffer (connection_t * c, char * buffer, uint32_t size) {
  
  c->writeBuffer=(uint8_t*)realloc(c->writeBuffer, 
				   (size+(c->writeBufferSize)) * sizeof(uint8_t));
  if (c->writeBuffer == NULL) {
    perror("Realloc error in write_cb:");
    exit (1);
  }
  memcpy ((c->writeBuffer)+ (c->writeBufferSize), buffer, size);
  c->writeBufferSize += size;
  logDebug ("Fill %d bytes total =%d\n", size, c->writeBufferSize); 
  
}

//==================================================

void connectConnection(connection_t * c) {

  if(connect(c->fd, (struct sockaddr *)&(c->server), 
	     sizeof(struct sockaddr)) != 0) {
    
    
    
    if (errno!=EINPROGRESS) {
      /* from man connect:
	 EINPROGRESS error code may be retuned wich means that 
	 The  socket  is  nonblocking and the connection cannot 
	 be completed immediately.  It is possible to  select(2)
	 or poll(2) for completion by selecting the socket for 
	 writing.  After select(2) indicates writability, use 
	 getsockopt(2) to read the SO_ERROR option at level 
	 SOL_SOCKET to determine whether connect() completed 
	 successfully (SO_ERROR is zero) or unsuccessfully 
	 (SO_ERROR is one of the usual error codes listed here, 
	 explaining the reason for the failure).
      */
      perror("connect");
    }
  }
}


//==================================================

void halfCloseRD (connection_t * c) {
  if (c->fd > 0) {
    logDebug("half closing (RD) connection fd=%d\n", c->fd);
    shutdown(c->fd, SHUT_RD);
    c->readShutdown = 1;
    if (c->writeShutdown) {
      logDebug( "Connection fd=%d fully closed\n", c->fd);
      close (c->fd);
      c->fd = -1;
    }
  }
}


//==================================================

void halfCloseWR (connection_t * c) {
  if (c->fd > 0) {
    logDebug("half closing WR connection fd=%d\n", c->fd);
    shutdown(c->fd, SHUT_WR);
    c->writeShutdown = 1;
    if (c->readShutdown) {
      logDebug( "Connection fd=%d fully closed\n", c->fd);
      close (c->fd);
      c->fd = -1;
    }
  }
}

//==================================================
void closeConnection (connection_t * c) {
  if (c->fd > 0) {
    logInfo("closing connection fd=%d\n", c->fd);
    shutdown(c->fd, SHUT_RDWR);
    close (c->fd);
    c->fd = -1;
  }
}

//==================================================

void freeConnection (connection_t * c) {

  free (c->readBuffer);
  free (c->writeBuffer);
  free (c);
  c=0;
  
}

//==================================================

