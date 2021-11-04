#ifndef CALLBACK_H
#define CALLBACK_H
#include "includes.h"

void listen_cb(EV_P_ ev_io * w, int revents);
void connect_cb(EV_P_ ev_io * w, int revents);
void setupCallBack (connection_t * c);
void read_cb(EV_P_ ev_io * w, int revents);
void write_cb(EV_P_ ev_io * w, int revents);
void setupReadCallBack (connection_t * c);
void setupWriteCallBack (connection_t * c);
void uninstallReadCallBack (connection_t * c);
void uninstallWriteCallBack (connection_t * c);
void uninstallCallBack (connection_t * c);
inline void flowControlOFF ( connection_t * c );
inline void flowControlON ( connection_t * c );

#endif
