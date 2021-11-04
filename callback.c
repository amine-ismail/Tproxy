#include "includes.h"
#include "connection.h"
#include "net.h"
#include "callback.h"
#include "log.h"

extern ev_io read_watcher [MAX_CONNECTION];
extern ev_io write_watcher [MAX_CONNECTION];
extern ev_io connect_watcher [MAX_CONNECTION];
extern struct ev_loop *loop;



void listen_cb(EV_P_ ev_io * w, int revents)
{
  int * listen_fd = (int *) (w->data);
  int s=0;
  struct sockaddr_in client_address;
  int len_addr=0;
  connection_t * c1=0;
  connection_t * c2=0;
  if ((revents == EV_WRITE) || (revents == EV_READ) ) {
    //accept the connection
    while (1) {
      len_addr=sizeof(struct sockaddr);
      s = accept(*listen_fd,
		 (struct sockaddr *)&client_address,
		 (socklen_t *)&len_addr);
      if (s<=0) {
	if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
	  break;
	}
	else {
	  perror ("Error on listen socket");
	}
      }
      else {
	initSocket(s,1);
	logDebug ("Accept new connection (fd = %d)\n", s);
	fflush(stdout);
	// New Connection
	c1=newConnection(s,client_address,len_addr);
	c2=newConnectionPeer(c1);
	// Connect C2 and setup a connect callback
	connectConnection(c2);
	ev_io_init(&(connect_watcher[c2->fd]), connect_cb,
		   c2->fd, EV_WRITE);
	connect_watcher[c2->fd].data = (void*) c2;
	ev_io_start(loop, &(connect_watcher[c2->fd]));
      }
    }
  }
  return ;
}

//===================================================

void connect_cb(EV_P_ ev_io * w, int revents)
{
  connection_t * c1;
  connection_t * c2;
  c2 = (connection_t *) (w->data);
  c1 = (connection_t *) (c2->peer);
  
  if ((revents == EV_WRITE)) {
    //Check if the socket is really connected
    if (isConnected(c2->fd)) {
      ev_io_stop (EV_A_ w); //stop watching
      //Setup read callback for the two sockets
      setupReadCallBack(c2);
      setupReadCallBack(c1);
      createProxy(c1,c2);
    }
    else {
      logWarning ("connect callback called, but socket is not connected" 
		  "fd=%d peer=%d\n", 
		  c2->fd, c1->fd);
      //SHOULD WE STOP THE WATCHING HERE ? 
      ev_io_stop (EV_A_ w); //stop watching
      closeConnection(c1);
      closeConnection(c2);
      return;
    }
  }
}

//===================================================

void setupCallBack (connection_t * c) {
  
  setupReadCallBack (c);
  setupWriteCallBack (c);
  
}

//===================================================

void setupWriteCallBack (connection_t * c) {
  
  if (c == NULL) return;
  if (c->fd <=0 ){
    logError ("TRY TO SETUP WRITE CB FOR fd (%d) < 0 peer fd is %d\n", 
	      c->fd, ((connection_t*)(c->peer))->fd);
    return;
  }
  if (!ev_is_active(&(write_watcher[c->fd]))) { 
    ev_io_init(&(write_watcher[c->fd]), write_cb,
	       c->fd, EV_WRITE);
    write_watcher[c->fd].data = (void *)c;
    ev_io_start(loop, &(write_watcher[c->fd]));
    logDebug ("Setup Write Callback (fd = %d)\n", c->fd);
  }
}

//===================================================

void setupReadCallBack (connection_t * c) {

  if (c == NULL) return;
  if (c->fd <=0 ){
    logError ("TRY TO SETUP READ CB FOR fd < 0\n");
    return;
  }
  if (!ev_is_active(&(read_watcher[c->fd]))) {
    ev_io_init(&(read_watcher[c->fd]), read_cb,
	       c->fd , EV_READ);
    read_watcher[c->fd].data = (void *)c;
    ev_io_start(loop, &(read_watcher[c->fd]));
    logDebug ("Setup Read Callback (fd = %d)\n", c->fd);
  }
}

//===================================================
#if 0
int sendData (connection_t * c, char * buffer, int size) {
  int sc=0;
  int err=0;
  /* try to send the buffer */
  sc=send (c->fd,buffer,size,0);
  err=errno;
  if (sc == -1) {
    //nothing has been sent check errno 
    if ((err == EAGAIN) || (err == EWOULDBLOCK)) {
      fillWriteBuffer(c, buffer, size);
      /*Setup flow control
	disable read CB */
      ev_io_stop (EV_A_ &(read_watcher[((connection_t *)(c->peer))->fd])); 
      setupWriteCallBack (c);
      logInfo("Flow Control ON (fd=%d)\n", c->fd);
    }
    else {
      logError("Write error on socket");
    }
    return -1;
  }
  else if ( sc != size ) {
    fillWriteBuffer(c, buffer+sc, size-sc);
    /*Setup flow control
      disable read CB    */
    ev_io_stop (EV_A_ &(read_watcher[((connection_t *)(c->peer))->fd])); 
    setupWriteCallBack (c);
    logInfo("Flow Control ON (fd=%d)\n", c->fd);
  }
  c->sent_bytes += sc;
  return 0;
}
#endif
//===================================================

void read_cb(EV_P_ ev_io * w, int revents)
{
    int32_t rc=0;
#define BUFF_SIZE 65536 //64K
    char buffer [BUFF_SIZE];
    connection_t * c1 = (connection_t *) (w->data);
    connection_t * c2 = (connection_t *) (c1->peer);
    if (revents == EV_READ) {
      while ((rc=recv(c1->fd,buffer,BUFF_SIZE,0)) > 0) {
	c1->recv_bytes+=rc;
	logDebug ("fd=%d received %d bytes\n", c1->fd, rc);
	//TODO: data should be processed here
	if (c2->fd <=0) {
	  logError("fill fd <=0\n");
	}
	fillWriteBuffer(c2, buffer, rc);
      }
      if (rc == 0) { 
	/* The connection has been closed (half closed)
	   remove read callBack on c1 */
	logDebug ("Connection closed fd=%d (buffer=%d))\n", 
		  c1->fd, c1->writeBufferSize);
	uninstallReadCallBack (c1);
	halfCloseRD(c1);
	if (c2->writeBufferSize == 0) {
	  halfCloseWR(c2);
	}
	if (c2->fd <= 0 && c1->fd <=0) {
	  /* c2 is also closed
	     destroy the proxy */
	  destroyProxy(c1->proxy);
	  return;
	}
      }
      if (c2->writeBufferSize > 0 ) {
	setupWriteCallBack(c2);
      }
    }
}

//==================================================
void uninstallReadCallBack (connection_t * c) {
  if (c->fd > 0) {
    logDebug ("Uninstall Read Callback (fd = %d)\n", c->fd);
    ev_io_stop (EV_A_ &(read_watcher[c->fd]));
  }
}

//==================================================

void uninstallWriteCallBack (connection_t * c) {
  if (c->fd > 0) {
    logDebug ("Uninstall write Callback (fd = %d)\n", c->fd);
    ev_io_stop (EV_A_ &(write_watcher[c->fd]));
  }
}

//==================================================

void uninstallCallBack (connection_t * c) {
  uninstallWriteCallBack (c);
  uninstallReadCallBack (c);
}
//==================================================

void write_cb(EV_P_ ev_io * w, int revents)
{
  int32_t sc=0;
  int err=0;
  connection_t * c1 = (connection_t *) (w->data); 
  connection_t * c2 = (connection_t *) (c1->peer); 
  if (revents == EV_WRITE) {
    if ((c1->writeBufferSize == 0) || (c1->writeBuffer == NULL)) {
      logError ("write_cb called but nothing to write!\n");
      fflush(stderr);
      printProxy(c1->proxy);
      uninstallWriteCallBack (c1);
      return;
    }
    
    logDebug("try to send %dB\n", c1->writeBufferSize);
    sc=send (c1->fd,c1->writeBuffer,c1->writeBufferSize,0);
    err=errno;
    if (sc == -1) {
      //nothing has been sent check errno 
      if ( err == EPIPE ) {
	logDebug("Connection closed by remote peer (EPIPE) fd=%d\n", c1->fd);
	/* remote peer closed the connection */
	uninstallWriteCallBack (c1);
	halfCloseWR(c1);
	uninstallReadCallBack (c2);
	halfCloseRD(c2);
	if (c2->fd <= 0 && c1->fd <=0) {
	  // c2 is also closed
	  // destroy the proxy
	  destroyProxy(c1->proxy);
	}
      }
      else if ((err != EAGAIN) && (err != EWOULDBLOCK)) {
	perror ("Write error on socket:");
      }
      else {
	perror ("Write callback called but cant not write on socket:");
      }
      return ;
    }
    c1->sent_bytes += sc;
    logDebug ("fd=%d send %d bytes\n", c1->fd, sc);
    if ( sc != c1->writeBufferSize) {
      memcpy(c1->writeBuffer, (c1->writeBuffer)+sc, (c1->writeBufferSize)-sc);
      c1->writeBufferSize-=sc;
      //free sent data
      
      c1->writeBuffer=(uint8_t*)realloc (c1->writeBuffer, c1->writeBufferSize);
      if (c1->writeBuffer == NULL) {
	perror("Realloc error in write_cb:");
	exit (1);
      }
      if (c2->flowControl == OFF) {
	flowControlON(c2);
      }
    }
    else {
      free(c1->writeBuffer);
      c1->writeBuffer = NULL;
      c1->writeBufferSize=0;
      uninstallWriteCallBack (c1);
     
      if (c2->readShutdown){
	logDebug ("fd=%d:peer is alreadyHalfclose RD fd=%d\n", c1->fd, c2->fd);
	halfCloseWR(c1);
      }
    }
    if (c2->flowControl == ON){
      flowControlOFF(c2);
    }
    if (c2->fd <= 0 && c1->fd <=0) {
      // c2 is also closed
      // destroy the proxy
      destroyProxy(c1->proxy);
    }
  } /* if (revents == EV_WRITE) */
}

inline void flowControlON ( connection_t * c ) {
  if ((c->readShutdown == 0) && (c->fd > 0)) {
    logInfo("FlowControl ON (fd=%d)\n", c->fd);
    ev_io_stop (EV_A_ &(read_watcher[c->fd])); 
    c->flowControl = ON;
  }
}

inline void flowControlOFF ( connection_t * c ) {
  if ((c->readShutdown == 0) && (c->fd > 0)) {
    logInfo("FlowControl OFF (fd=%d)\n", c->fd);
    ev_io_start(loop, &(read_watcher[c->fd]));
    c->flowControl = OFF;
  }
}


