#ifndef __X_IO__
#define __X_IO__

int xrecv(int sockid, void * buf, int size, int flag);
int xsend(int sockid, void * buf, int size, int flag);

#endif
