#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include "main.h"
#include "x_io.h"

int open_socket(int port)
{
	int sockid = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addrport = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr.s_addr = htonl(INADDR_ANY)
	};

	if(-1 == sockid)
	{
		perror("socket()");
		exit(EXIT_FAILURE);
	}

	int yes = 1;

	if (setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	if(-1 == bind(sockid, (struct sockaddr *) &addrport, sizeof(addrport)))
	{
		perror("bind()");
		exit(EXIT_FAILURE);
	}

	if(-1 == listen(sockid, 1))
	{
		perror("listen()");
		exit(EXIT_FAILURE);
	}

	return sockid;
}

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

snd_pcm_t * set_hw_params(META *meta)
{
	snd_pcm_t *soundDev;

	if(snd_pcm_open(&soundDev, "sysdefault:CARD=PCM5122",
				SND_PCM_STREAM_PLAYBACK, 0) < 0)
	{
		perror("snd_pcm_open()");
		return (snd_pcm_t *)-1;
	}

	if(snd_pcm_set_params(soundDev, SND_PCM_FORMAT_S16_LE,
				SND_PCM_ACCESS_RW_INTERLEAVED, meta->NumChannels,
				48000, 0, 500000) < 0)
	{
		perror("snd_pcm_set_params()");
		return (snd_pcm_t *)-1;
	}

	return soundDev;
}

void setVolume(uint32_t volume)
{
	long min, max;
	long a;

	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;

	const char *card = "default";
	const char *selem_name = "Digital";

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, card);
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

	snd_mixer_selem_set_playback_volume_all(elem, a = ((double)volume / 100.0) * (double)max);

	snd_mixer_close(handle);
}

uint32_t played;

void * play(void * val)
{
	int ret;
	int playing;

	BUFF * buff = (BUFF *)val;

	snd_pcm_prepare(buff->soundDev);

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	while(played < buff->size)
	{
		if((playing = buff->size - played) < 32768)
		{
			playing >>= 2;
		}

		else
		{
			playing = 8192;
		}

		if((ret = snd_pcm_writei(buff->soundDev, buff->data + played, playing)) == -EPIPE)
		{
			puts("XRUN");
			snd_pcm_prepare(buff->soundDev);
		}

		else if(ret < 0)
		{
			puts("pcm error");
			snd_pcm_recover(buff->soundDev, ret, 0);
			break;
		}
		
		played += 32768;
	}

	played = 0;
}

int main(void)
{
	int ret;
	int sockid, s;
	uint32_t allocsize, timesize, volsize;
	int pagesize = getpagesize();

	snd_pcm_t *soundDev;
	pthread_t play_thread;

	void * data = NULL;

	META * meta;
	BUFF * buff;

	char command[sizeof("COMMD")];

	while(1)
	{
		sockid = open_socket(5000);

		meta = calloc(1, METADATA_SIZE);
		buff = calloc(1, sizeof(BUFF));

		s = accept(sockid, NULL, NULL);	

		while(1)
		{
			if(xsend(s, "META ", sizeof("META "), 0) < 0)
				goto exit_alloc;

			if(xrecv(s, (void *)meta, METADATA_SIZE, 0) < 0)
				goto exit_alloc;

			meta->DataSize -= 24;

#ifdef DEBUG
			print_metadata(meta);
#endif

			soundDev = set_hw_params(meta);

			if((int)soundDev == -1)
				goto exit;
			
			allocsize = pagesize * (meta->DataSize / pagesize + 1);

			printf("allocsize : %d\n", allocsize);

			data = realloc(data, allocsize);

			if(data == NULL || !data)
			{
				perror("realloc()");
				return -1;
			}

			if(xrecv(s, data, meta->DataSize, 0) < 0)
				goto exit_alloc;

			buff->soundDev = soundDev;
			buff->size = meta->DataSize;
			buff->data = data;
			played = 0;

			pthread_create(&play_thread, NULL, play, (void *)buff);

			while(1)
			{
				ret = xrecv(s, command, sizeof("COMMD"), 0);

				if(ret < 0)
				{
					if(ret == -1)
					{

						if(!pthread_tryjoin_np(play_thread, NULL))
						{
							while(ret = xrecv(s, command, sizeof("CHANG"), 0))
							{
								if(ret == -1)
									continue;
								else if(ret == -2)
									goto exit_alloc;
							}
							
							if(strcmp(command, "CHANG"))
								goto exit_alloc;	

							snd_pcm_close(soundDev);
							
							break;
						}
					}

					else if(ret == -2)
						goto exit_alloc;
				}

				else
				{
					if(!strcmp(command, "PAUSE"))
					{
						pthread_cancel(play_thread);

						pthread_join(play_thread, NULL);

						while(ret = xrecv(s, command, sizeof("COMMD"), 0))
						{
							if(ret == -1)
								continue;
							else if(ret == -2)
								goto exit_alloc;
						}
					}

					if(!strcmp(command, "RESUM"))
					{
						pthread_create(&play_thread, NULL, play, (void *)buff);
					}

					if(!strcmp(command, "CTIME"))
					{
						timesize = played / meta->AvgByteRate;

						if(xsend(s, &timesize, sizeof(uint32_t), 0) < 0)
							goto exit_alloc;
					}

					if(!strcmp(command, "TIME "))
					{
						if(xrecv(s, &timesize, sizeof(uint32_t), 0) < 0)
							goto exit_alloc;

						played = meta->AvgByteRate * timesize;	
					}

					if(!strcmp(command, "VOLUM"))
					{
						if(xrecv(s, &volsize, sizeof(uint32_t), 0) < 0)
							goto exit_alloc;

						setVolume(volsize);
					}

					if(!strcmp(command, "CHANG"))
					{
						pthread_cancel(play_thread);

						pthread_join(play_thread, NULL);

						snd_pcm_close(soundDev);

						break;
					}

					if(!strcmp(command, "STOP "))
					{
						pthread_cancel(play_thread);

						pthread_join(play_thread, NULL);

						goto exit_alloc;
					}
				}
			}
		}


exit_alloc:
		free(meta);
		free(data);
		free(buff);

		data = NULL;

exit:
		puts("Client Exited");

		snd_pcm_close(soundDev);

	}

	return 0;
}
