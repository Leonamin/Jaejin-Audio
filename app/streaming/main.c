#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <stdint.h>
#include <pthread.h>
#include <errno.h>

#include "../x_io.h"

#define DEBUG

#define METADATA_SIZE 32
#define BUFF_MS 100
#define TIMEOUT 5

typedef struct
{   
	// WAV FMT
	uint8_t			FmtID[4];
	uint32_t	 	FmtSize;
	uint16_t		AudioFormat;
	uint16_t  		NumChannels;
	uint32_t 		SampleRate;
	uint32_t		AvgByteRate;
	uint16_t		BlockAlign;
	uint16_t		BitPerSample;

	// DATA FMT
	uint8_t			DataID[4];
	uint32_t		DataSize;
} META;

typedef struct {
	int				sockid;
	int				AvgByte;
	void*			buf[2];
	int				bufsize;
	pthread_mutex_t mutex[2];
} BUFF;

/* 소켓을 열고 bind 시키는 함수 */

int open_socket(void)
{
	int sockid = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addrport = {
		.sin_family = AF_INET,
		.sin_port = htons(5100),
		.sin_addr.s_addr = htonl(INADDR_ANY)
	};

	if(-1 == sockid)
	{
		perror("socket()");
		return -1;
	}

	if(-1 == bind(sockid, (struct sockaddr *) &addrport, sizeof(addrport)))
	{
		perror("bind()");
		return -1;
	}

	if(-1 == listen(sockid, 1))
	{
		perror("listen()");
		return -1;
	}

	return sockid;
}

/* 디버깅용 받은 메타데이터 출력 함수 */

void print_metadata(META * meta)
{
	puts("HEADER_FMT");
	printf("FmtID : %c%c%c%c\n", meta->FmtID[0], meta->FmtID[1],
			meta->FmtID[2], meta->FmtID[3]);
	printf("FmtSize : %u\n", meta->FmtSize);
	printf("AudioFormat : %u\n", meta->AudioFormat);
	printf("NumChannels : %u\n", meta->NumChannels);
	printf("SampleRate : %u\n", meta->SampleRate);
	printf("AvgByteRate : %u\n", meta->AvgByteRate);
	printf("BlockAlign : %u\n", meta->BlockAlign);
	printf("BitPerSample : %u\n", meta->BitPerSample);
	printf("DataID : %c%c%c%c\n", meta->DataID[0], meta->DataID[1],
			meta->DataID[2], meta->DataID[3]);
	printf("DataSize : %u\n", meta->DataSize);
}

/* WAV 파일 포맷, 버퍼 초를 받아 알맞는 버퍼 사이즈를 계산 후 재할당하는 함수 */ 

int create_buffer(BUFF * bufPack, unsigned int AvgByteRate, int timeMs)
{
	double timeScope = timeMs / 1000.0;

	int buffersize = AvgByteRate * timeScope;

#ifdef DEBUG
	printf("\nAvgByteRate : %u, timeMs : %d, timeScope: %g, buffersize: %d\n", 
			AvgByteRate, timeMs, timeScope, buffersize);
#endif

	mremap(bufPack->buf[0], bufPack->bufsize, buffersize, 0);
	mremap(bufPack->buf[1], bufPack->bufsize, buffersize, 0);

	bufPack->bufsize = buffersize;

#ifdef DEBUG
	assert(bufPack->buf[0] != MAP_FAILED);
	assert(bufPack->buf[1] != MAP_FAILED);
#endif

	if(bufPack->buf[0] == MAP_FAILED || bufPack->buf[1] == MAP_FAILED)
	{
		perror("mremap()");
		exit(EXIT_FAILURE);
	}

	return buffersize;
}

/* 재생하고 있는 동안 버퍼에 데이터를 받는 스레드 */

void * recv_buffer(void * buff_origin)
{
	int ret;
	int count = 0;

	BUFF * buff = buff_origin;

	xsend(buff->sockid, "SIZE ", sizeof("SIZE "), 0, TIMEOUT);
	xsend(buff->sockid, &(buff->bufsize), sizeof(buff->bufsize), 0, TIMEOUT);

	while(buff->AvgByte > count)
	{
		for(int i = 0; i < 2; i++)
		{
			ret = pthread_mutex_lock(&(buff->mutex[i]));

#ifdef DEBUG
			assert(ret != EDEADLK);
#endif

			xsend(buff->sockid, "DATA ", sizeof("DATA "), 0, TIMEOUT);

			count += xrecv(buff->sockid, (void *)buff->buf[i], buff->bufsize, 0, TIMEOUT);	

			ret = pthread_mutex_unlock(&(buff->mutex[i]));

#ifdef DEBUG
			assert(ret != EDEADLK);	
#endif
		}
	}
}

/* 메인 스레드에서 돌아가는 재생 함수 */

void play_buffer(void * buff_origin)
{
	int ret;
	int count;
	int recvByte = 0;

	BUFF * buff = buff_origin;

	while(buff->AvgByte != recvByte)
	{
		for(int i = 0; i < 2; i++)
		{
			ret = pthread_mutex_lock(&(buff->mutex[i]));

#ifdef DEBUG
			assert(ret != EDEADLK);
#endif

			printf("Playing!!\n");
			// play code

			ret = pthread_mutex_unlock(&(buff->mutex[i]));

#ifdef DEBUG
			assert(ret != EDEADLK);	
#endif
		}
	}
}

int main(void)
{
	int sockid = open_socket();
	int s;
	int buffersize;

	pthread_t recv_thread;

	META * meta = calloc(1, METADATA_SIZE);

	BUFF buff = {
		.buf[0] = mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE, 
									 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0),
		.buf[1] = mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE,
									 MAP_ANONYMOUS | MAP_PRIVATE, -1 ,0),
		.bufsize = PAGE_SIZE,
		.mutex[0] = PTHREAD_MUTEX_INITIALIZER,
		.mutex[1] = PTHREAD_MUTEX_INITIALIZER
	};

#ifdef DEBUG
	assert(-1 != sockid);
#endif

	while(1)
	{
		s = accept(sockid, NULL, NULL);
		
		buff.sockid = s;

		while(1)
		{
			if(-1 == xrecv(s, (void *)meta, METADATA_SIZE, 0, TIMEOUT))
			{
				printf("client timed out\n");
				break;
			}

			buff.AvgByte = meta->DataSize;

			//set_hw_params(meta);

			buffersize = create_buffer(&buff, meta->AvgByteRate, BUFF_MS);

			pthread_create(&recv_thread, NULL, recv_buffer, (void *)&buff); // buffer RECV

			play_buffer((void *)&buff);
		}

		pthread_cancel(recv_thread);	
		free(meta);
		munmap(buff.buf[0], buffersize);
		munmap(buff.buf[1], buffersize);
		break;
	}

	close(sockid);

	return 0;
}
