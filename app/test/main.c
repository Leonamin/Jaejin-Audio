#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "../streaming/x_io.h"

#define DEBUG

#define HEADER_RIFF_SIZE 12
#define HEADER_FMT_SIZE 32

typedef struct
{
	unsigned char	FmtID[4];
	unsigned int	FmtSize;
	unsigned short	AudioFormat;
	unsigned short	NumChannels;
	unsigned int	SampleRate;
	unsigned int	AvgByteRate;
	unsigned short	BlockAlign;
	unsigned short	BitPerSample;
	unsigned char	DataID[4];
	unsigned int	DataSize;
} FMT;

int main(void)
{

	FILE * fp = fopen("test1.wav", "r");
	FILE * fp2 = fopen("test.wav", "r");
	
#ifdef DEBUG
	assert(fp != NULL);
#endif

#ifdef DEBUG
	assert(!fseek(fp, HEADER_RIFF_SIZE, SEEK_SET));
#endif

	FMT * header_fmt = calloc(1, HEADER_FMT_SIZE);

	fread(header_fmt, HEADER_FMT_SIZE, 1, fp);

	puts("HEADER_FMT");
	printf("ChunkID : %c%c%c%c\n", header_fmt->FmtID[0], header_fmt->FmtID[1],
			header_fmt->FmtID[2], header_fmt->FmtID[3]);
	printf("ChunkSize : %u\n", header_fmt->FmtSize);
	printf("AudioFormat : %hu\n", header_fmt->AudioFormat);
	printf("NumChannels : %hu\n", header_fmt->NumChannels);
	printf("SampleRate : %u\n", header_fmt->SampleRate);
	printf("AvgByteRate : %u\n", header_fmt->AvgByteRate);
	printf("BlockAlign : %hu\n", header_fmt->BlockAlign);
	printf("BitPerSample : %hu\n", header_fmt->BitPerSample);

	puts("");

	char buf[sizeof("OK   ")];
	
	int sockid = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addrport = {
		.sin_family = AF_INET,
		.sin_port = htons(5000),
		.sin_addr.s_addr = inet_addr("127.0.0.1")
	};

	int status = connect(sockid, (struct sockaddr *) &addrport, sizeof(addrport));

	if(status == -1)
	{
		printf("Connect Failure\n");
		return 0;
	}

	while(1)
	{
		puts("HEADER_DATA");
		printf("ChunkID : %c%c%c%c\n", header_fmt->DataID[0], header_fmt->DataID[1],
				header_fmt->DataID[2], header_fmt->DataID[3]);
		printf("ChunkSize : %u\n", header_fmt->DataSize);
		
		sprintf(buf, "%c%c%c%c", header_fmt->DataID[0], header_fmt->DataID[1], header_fmt->DataID[2], header_fmt->DataID[3]);

		if(strcmp(buf, "data"))
		{
			assert(!fseek(fp, header_fmt->DataSize, SEEK_CUR));
			fread(&(header_fmt->DataID[0]), 8, 1, fp);
		}

		else
			break;
	}
		xrecv(sockid, buf, sizeof("META "), 0);
	
		xsend(sockid, header_fmt, HEADER_FMT_SIZE, 0);

		puts("k");

		uint8_t * buff = calloc(1, header_fmt->DataSize);

		if(buff == NULL)
			printf("null");

		unsigned int count = 0;
	
		while(header_fmt->DataSize > count)
		{
			count += fread(buff + count, 1, header_fmt->DataSize, fp);	

			if(-1 == xsend(sockid, buff, count, 0))
				goto exit;	
		}	
		
		sleep(10);
		
		xsend(sockid, "CHANG", sizeof("CHANG"), 0);

		
		fseek(fp2, HEADER_RIFF_SIZE, SEEK_SET);
	
		fread(header_fmt, HEADER_FMT_SIZE, 1, fp2);
	while(1)
	{
		puts("HEADER_DATA");
		printf("ChunkID : %c%c%c%c\n", header_fmt->DataID[0], header_fmt->DataID[1],
				header_fmt->DataID[2], header_fmt->DataID[3]);
		printf("ChunkSize : %u\n", header_fmt->DataSize);
		
		sprintf(buf, "%c%c%c%c", header_fmt->DataID[0], header_fmt->DataID[1], header_fmt->DataID[2], header_fmt->DataID[3]);

		if(strcmp(buf, "data"))
		{
			assert(!fseek(fp2, header_fmt->DataSize, SEEK_CUR));
			fread(&(header_fmt->DataID[0]), 8, 1, fp2);
		}

		else
			break;
	}

	
		xrecv(sockid, buf, sizeof("META "), 0);
	
		xsend(sockid, header_fmt, HEADER_FMT_SIZE, 0);

		puts("k");

		buff = calloc(1, header_fmt->DataSize);

		if(buff == NULL)
			printf("null");

		count = 0;
	
		while(header_fmt->DataSize > count)
		{
			count += fread(buff + count, 1, header_fmt->DataSize, fp2);	

			if(-1 == xsend(sockid, buff, count, 0))
				goto exit;	
		}

	exit:

	close(sockid);

	free(header_fmt);

	fclose(fp);

	return 0;
}
