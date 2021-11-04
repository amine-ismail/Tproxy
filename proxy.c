#include "includes.h"
#include "net.h"
#include "connection.h"
#include "log.h"
#include "proxy.h"

static uint32_t nb_proxy=0;

static proxy_t * proxies_head =  NULL;
static proxy_t * proxies_tail = NULL;


//==================================================

void destroyProxy (proxy_t * p) {
  connection_t * c1 = (connection_t*)(p->clientSide);
  connection_t * c2 = (connection_t*)(p->serverSide);
  
  if ((c1->writeBufferSize != 0) || (c2->writeBufferSize != 0)) {
    logWarning ("Proxy destroyed, but some data still not sent!\n");
  }
  nb_proxy--;
  logInfo ("Destroy (%d):", nb_proxy);
  printProxy (c1->proxy);
  freeConnection (c1);
  freeConnection (c2);
  deleteProxy(p);
  free (p);
  p=0;

}

//==================================================

void createProxy (connection_t * clientSide, connection_t * serverSide) {
  proxy_t * p;
  p = (proxy_t *) malloc (sizeof(proxy_t));
  if (p == NULL) {
    perror ("Malloc error");
    exit (1);
  }
  memset (p, 0, sizeof(proxy_t));
  p->clientSide = (struct connection_t*) clientSide;
  p->serverSide = (struct connection_t*) serverSide;
  clientSide->proxy = p;
  serverSide->proxy = p;
  insertProxy (p);
  nb_proxy ++;
  logInfo ("New (%d):", nb_proxy);
  printProxy(p);
  
}

//===================================================

void printProxy (proxy_t * p) {
  connection_t * c1 = (connection_t*)(p->clientSide);
  connection_t * c2 = (connection_t*)(p->serverSide);
  
  logInfo (
 	   "%s:%u<==([S:%luKB | R:%luKB](%d) PROXY (%d)[S:%luKB | R:%luKB])==>%s:%u\n", 
	   c1->client_addrString, (unsigned) c1->client_localPort,
	   (unsigned long) c1->sent_bytes, (unsigned long) c1->recv_bytes, 
	   c1->fd, c2->fd,
	   (unsigned long) c2->sent_bytes, (unsigned long) c2->recv_bytes,
	   c2->server_addrString, (unsigned) c2->server_port);
}

//==================================================

void insertProxy (proxy_t * p) {
  if (proxies_head == NULL) {
    /* first proxy in the list*/
    proxies_head = p;
    proxies_tail = p;
  }
  else {
    /* insert in the tail of the list */
    p->prev = (struct proxy_t *) proxies_tail;
    proxies_tail->next = (struct proxy_t *) p;
    proxies_tail = p; 
  }
}

//==================================================

void deleteProxy (proxy_t *p) {
  if ( p == proxies_tail && p == proxies_head ) {
    proxies_tail = proxies_head = NULL;
  }
  else if (p == proxies_tail) {
    proxies_tail = (proxy_t *)(p->prev);
    proxies_tail->next=NULL;
  }
  else if ( p == proxies_head ) {
    proxies_head = (proxy_t *)(p->next);
    proxies_head->prev=NULL;
  }
  else {
    ((proxy_t *)(p->prev))->next = p->next;
    ((proxy_t *)(p->next))->prev = p->prev;
  }
  p->next = NULL;
  p->prev = NULL;
}

//===============================================

void printProxies (void) {

  proxy_t * parc_p;
  parc_p = proxies_head;
  logInfo ( "NB PROXIES = %d\n", nb_proxy); 
  while (parc_p != NULL) {
    printProxy (parc_p);
    parc_p = (proxy_t *) (parc_p->next);
  }
}

