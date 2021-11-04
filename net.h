#ifndef NET_H
#define NET_H

int createSocket (void);
int initSocket (int s, uint8_t non_blocking);
int bindSocketToInterface (int fd, char * ifname);
uint8_t isConnected (int s);
int getOriginalDstAddr (int s, struct sockaddr * dst_adr);
void setSockBlocking (int s);
void setSockNonBlocking (int s);
#endif
