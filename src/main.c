#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include "wavFile.h"

WAVHEADER wavheader;

int procaudio(char* filename);
int setupDSP(snd_pcm_t *dev, int buf_size, int format, int sampleRate, int channels);

int main(int argc, char* argv[])
{
    int ret;
    char* filename;

    if(argc < 2) {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        return -1;
    }

    filename = argv[1];

    ret = procaudio(filename);

    if(ret < 0) {
        fprintf(stderr, "failed to call openaudio()\n");
        return -1;
    }

    return 0;
}

//audio
int procaudio(char* filename)
{
    int fd, ret;
    char* fmt;
    snd_pcm_t   *handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t   frames;

    int channels, format, dir, buf_size, count;
    long loops;
    unsigned int val;
    char *buffer;

    //check file format
    fmt = strrchr(filename, '.');
    printf("%s file format: %s\n", filename, fmt);

    //.wav가 아닐 경우
    if(strcmp(fmt, ".wav")) {
        
    }

    fd = open(filename, O_RDONLY);
    if(fd == -1) {
        fprintf(stderr, "failed to open %s\n", filename);
        return -1;
    }

    count = read(fd, &wavheader, sizeof(WAVHEADER));
    if(count < 1) {
        fprintf(stderr, "Could not read wav data\n");
        return -1;
    }

    printf("Channles: %d\n", wavheader.nChannels);
    printf("Sampling Rate: %d\n", wavheader.sampleRate);

    ret = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if(ret < 0) {
        fprintf(stderr, "Unable to open pcm device: %s\n", snd_strerror(ret));
        return -1;
    }
    
    snd_pcm_hw_params_malloc(&params);
    if(snd_pcm_hw_params_any(handle, params) < 0) {
        fprintf(stderr, "Can not configure this PCM device\n");
        return -1;
    }

    channels = wavheader.nChannels;
    printf("Wave Channels Mode: %s\n", (channels) ? "Stereo" : "Mono");
    snd_pcm_hw_params_set_channels(handle, params, channels);

    printf("%d\n",wavheader.avgBytesPerSec);

    printf("Wave Bytes: %d\n", wavheader.nblockAlign);
    switch (wavheader.nblockAlign)
    {
        case 1:
            format = SND_PCM_FORMAT_U8;
            break;
        case 2:
            format = (channels == 1) ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_U8;
            break;
        case 4:
            format = SND_PCM_FORMAT_S16_LE;
            break; 
        default:
            printf("Unknown Byte rate for sound\n");
            break;
    }

    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, format);

    printf("Wave Sampling Rate: 0x%d\n", wavheader.sampleRate);
    val = wavheader.sampleRate;
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);

    frames = 2048;
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

    ret = snd_pcm_hw_params(handle, params);
    if(ret < 0) {
        fprintf(stderr, "Unable to set hw parameters: %s\n", snd_strerror(ret));
        return -1;
    }

    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    buf_size = frames * channels * ((format == SND_PCM_FORMAT_S16_LE) ? 2 : 1);
    buffer = (char*)malloc(buf_size);

    snd_pcm_hw_params_get_period_time(params, &val, &dir);

    do {
        if((count = read(fd, buffer, buf_size)) <= 0) 
            break;
        ret = snd_pcm_writei(handle, buffer, frames);
        if(ret == -EPIPE) {
            //underrun
            fprintf(stderr, "Underrun occurred\n");
            snd_pcm_prepare(handle);
        } else if(ret < 0) {
            fprintf(stderr, "error from write: %s\n", snd_strerror(ret));
        } else if(ret != (int)frames) {
            fprintf(stderr, "short write, write %d frames\n", ret);
        }
    } while(count == buf_size);

    close(fd);

    snd_pcm_drain(handle);
    snd_pcm_close(handle);

    free(buffer);

    return 0;
}

int setupDSP(snd_pcm_t *dev, int buf_size, int format, int sampleRate, int channels)
{
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_uframes_t frames;
}