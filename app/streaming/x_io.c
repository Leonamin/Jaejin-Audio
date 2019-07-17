#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <netinet/in.h>

#define TIMEOUT_MS 5000

int xrecv(int sockid, void * buf, uint32_t size, int flag)
{
	uint32_t count = 0;
	int ret;

	uint8_t * buff = (void *)buf;

	struct pollfd watchfd = {
		.fd = sockid,
		.events = POLLIN
	};

	while(1)
	{
		ret = poll(&watchfd, 1, TIMEOUT_MS);
		if(ret == -1)
		{
			perror("poll()");
			return -2;
		}

		else if(!ret)
		{
			return -1;
		}

		else if(watchfd.revents & POLLIN)
		{
			count += recv(sockid, buff + count, size, flag);	
			
			if(count < 0)
			{
				perror("recv()");
				return -2;
			}

			if(!count)
			{
				return -1;
			}

			if(count < size)
				continue;

			else
				break;

		}
	}

	return 0;
}

int xsend(int sockid, void * buf, uint32_t size, int flag)
{
	int count = 0;
	
	uint8_t * buff = (void *)buf;

	struct pollfd watchfd = {
		.fd = sockid,
		.events = POLLOUT
	};

	while(1)
	{
		int ret = poll(&watchfd, 1, TIMEOUT_MS);

		if(ret == -1)
		{
			perror("poll()");
			return -2;
		}

		else if(!ret)
		{
			return -1;
		}

		else if(watchfd.revents & POLLOUT)
		{
			count += send(sockid, buff + count, size, flag);
			
			if(count < 0)
			{
				perror("send()");
				return -2;
			}
			
			if(!count)
			{
				return -1;
			}

			if(count < size)
				continue;

			else
				break;
		}
	}
		return 0;
}
