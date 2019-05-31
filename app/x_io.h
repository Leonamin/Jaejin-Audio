#ifndef __X_IO__
#define __X_IO__

int xrecv(int sockid, void * dest, int size, int flag, int timeout);
int xsend(int sockid, void * src, int size, int flag, int timeout);

#endif
