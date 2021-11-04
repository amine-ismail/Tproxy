#include "includes.h"
#include "connection.h"
#include "net.h"
#include "callback.h"
#include "proxy.h"
#include "daemon.h"
#include "readconf.h"
#include "commandLine.h"

#define PROG_NAME "proxy"


connection_t connection_tab  [MAX_CONNECTION];
//==================================
ev_io read_watcher [MAX_CONNECTION];
ev_io write_watcher [MAX_CONNECTION];
ev_io connect_watcher [MAX_CONNECTION];

//static ev_timer stats_watcher;
//static ev_timer stop_watcher;
//static ev_timer * connexion_ev_watcher;

struct ev_loop *loop;
void usr1_handler (int dummy);

/*========conf params===============*/

uint8_t make_daemon = 0;
uint16_t listen_port = 8110;
uint8_t log_level = 0;
char * pid_file = "./"PROG_NAME".pid";
uint16_t buffer_delay_ms = 0;
char * conf_file = "./"PROG_NAME".conf";

/*==================================*/

int readConf () {
  int err=0;
  
  if ((err=loadConf(conf_file))< 0 ){
    return -1;
  }

  read_int ("daemon", (int*)&make_daemon);
  read_int ("listen_port", (int*)(&listen_port));
  read_int ("log_level", (int*)(&log_level));
  read_string ("pid_file", &pid_file);
  read_int ("buffer_delay_ms", (int*)(&buffer_delay_ms));
  
  destroyConf ();
  return 0;

}

void quit_proxy (int dummy);
int init (void);

//==================================

int init () {
  
  signal(SIGINT,quit_proxy);
  signal(SIGUSR1,usr1_handler);
  signal(SIGPIPE,  SIG_IGN);

  /*
    TODO: here we can setup firewall rule
  */

  return 0;
}

//==================================
int listen_sock;

  
int main (int argc, char ** argv) {
  
  struct sockaddr_in local_addr_listen;

  init ();

  parseCommandLine(argc, argv);
  
  readConf();

  if (make_daemon == 1) {
    daemonize();
  }
  
  /*catchSignals()*/
  loop = ev_default_loop (EVBACKEND_EPOLL);  
  listen_sock=createSocket();
  initSocket(listen_sock, 1);
  
  bzero(&local_addr_listen,sizeof local_addr_listen);
  local_addr_listen.sin_port = htons(listen_port);
  local_addr_listen.sin_family = AF_INET;
  local_addr_listen.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if (bind(listen_sock,(struct sockaddr *)&local_addr_listen,
	   sizeof local_addr_listen) <0 ) {
    perror ("bind error:");
  }
  listen(listen_sock,128);
  
  ev_io_init(&(connect_watcher[listen_sock]), listen_cb,
	     listen_sock, EV_READ);
  connect_watcher[listen_sock].data = (void *) &listen_sock;
  ev_io_start(loop, &(connect_watcher[listen_sock]));

  ev_loop (loop, 0);
  return 0;
}

void quit_proxy (int dummy) {
  shutdown (listen_sock, SHUT_RDWR);
  close (listen_sock);
  /*
    TODO: here we may de lete firewall rules
  */
  exit(0);
}


void usr1_handler (int dummy) {
  printProxies();
}
