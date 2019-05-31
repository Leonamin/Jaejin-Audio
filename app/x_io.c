#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "x_io.h"

int xrecv(int sockid, void * dest, int size, int flag, int timeout)
{
	struct pollfd watchfd;
	int count;
	int ret;

	watchfd.fd = sockid;
	watchfd.events = POLLIN;

	while(1)
	{
		ret = poll(&watchfd, 1, timeout * 1000);

		if(ret == -1)
		{
			perror("poll()");
			exit(EXIT_FAILURE);
		}

		if(!ret)
			return -1;

		if(watchfd.revents & POLLIN)
		{
			count = recv(sockid, dest, size, flag);
	
			if(count != size)
			{
				ret = xsend(sockid, "RETRY", sizeof("RETRY"), 0, timeout);
				
				if(ret == -1)
					return -1;

				continue;
			}

			else
			{
				xsend(sockid, "OK   ", sizeof("OK   "), 0, timeout);

				if(ret == -1)
					return -1;

				break;
			}
		}
	}

	return count;
}


int xsend(int sockid, void *src, int size, int flag, int timeout)
{
	struct pollfd watchfd;
	int count;
	int ret;

	char buf[sizeof("OK   ")];

	watchfd.fd = sockid;
	watchfd.events = POLLOUT;

	while(1)
	{
		ret = poll(&watchfd, 1, timeout * 1000);

		if(ret == -1)
		{
			perror("poll()");
			exit(EXIT_FAILURE);
		}

		if(!ret)
			return -1;

		if(watchfd.revents & POLLOUT)
		{
			count = send(sockid, src, size, flag);

			if(count != size)
			{
				ret = xrecv(sockid, buf, sizeof("RETRY"), 0, timeout);
				
				if(ret == -1)
					return -1;

				continue;
			}

			else
			{
				ret = xrecv(sockid, buf, sizeof("OK   "), 0, timeout);

				if(ret == -1)
					return -1;

				break;
			}
		}
	}

	return count;
}
