#ifndef _PROXY_H_
#define _PROXY_H_

void destroyProxy (proxy_t * p);
void createProxy (connection_t * clientSide, connection_t * serverSide);
void printProxy (proxy_t * p);
void insertProxy (proxy_t * p);
void deleteProxy (proxy_t *p);
void printProxies (void);


#endif 
