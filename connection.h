#ifndef CONNECTION_H
#define CONNECTION_H
#include "includes.h"

#define MAX_CONNECTION 10000
#define LISTEN_PORT 8110

#define CON_STS_NOTCONNECTED 0
#define CON_STS_CONNECTING   1
#define CON_STS_CONNECTED    2
#define CON_STS_CR           3
#define CON_STS_CW           4
#define CON_STS_CRW          5
#define CON_STS_ERR          6

#define CON_TYPE_FROM_CLIENT 1
#define CON_TYPE_FROM_PROXY  2
#define CON_TYPE_TO_PROXY    3
#define CON_TYPE_TO_SERVER   4

typedef struct {
  struct connection_t * clientSide; // who does the coonect
  struct connection_t * serverSide; // who does the accept
  struct proxy_t * next;
  struct proxy_t * prev;
} proxy_t;


typedef struct {
  uint32_t id;
  int fd;
  char iface [16];
  
  uint8_t status;
  uint8_t listener;

  char client_addrString [256];
  uint32_t client_localPort;
  char server_addrString [256];
  uint32_t server_port;

  struct sockaddr_in client;
  struct sockaddr_in server;

  uint8_t * readBuffer;
  uint32_t readBufferSize;
  uint8_t * writeBuffer;
  uint32_t writeBufferSize;
  
  //stats
  uint64_t recv_bytes;
  uint64_t sent_bytes;
  #define ON 1
  #define OFF 0
  uint8_t flowControl;
  uint8_t readShutdown;
  uint8_t writeShutdown;

  struct connection_t * peer;
  proxy_t * proxy;

} connection_t;




connection_t * newConnection (int fd, struct sockaddr_in client_address, 
			      int len_addr);
connection_t * newConnectionPeer (connection_t * c1);
void connectConnection(connection_t * c);
void closeConnection (connection_t * c);
void halfCloseRD (connection_t * c);
void halfCloseWR (connection_t * c);
void freeConnection (connection_t * c);
void fillWriteBuffer (connection_t * c, char * buffer, uint32_t size);
void createProxy (connection_t * clientSide, connection_t * serverSide);
void printProxy (proxy_t * p);
void destroyProxy (proxy_t * p);
void createProxy (connection_t * clientSide, connection_t * serverSide);
#endif
