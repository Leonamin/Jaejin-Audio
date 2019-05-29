#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include "wavFile.h"

WAVHEADER wavheader;

int procaudio(char* filename);
int setupDSP(snd_pcm_t *handle, int format, int sampleRate, int channels, int period, snd_pcm_uframes_t *exact_periodsize)
{
    snd_pcm_hw_params_t *hw_params;
    int exact_format = 0, dir;
    unsigned int frame;
    unsigned int val;
    snd_pcm_uframes_t periodsize, buffersize, exact_buffersize;

    /* hw paramiter 할당 및 초기화 */
    if(snd_pcm_hw_params_malloc(&hw_params) < 0) {
        fprintf(stderr, "Could not malloc paramiter\n");
        return -1;
    }

    if(snd_pcm_hw_params_any(handle, hw_params) < 0) {
        fprintf(stderr, "Could not initialize paramiter\n");
        return -1;
    }
    /* 오디오 데이터 접근 타입 설정 */
    if(snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        fprintf(stderr, "Could not set access type\n");
        return -1;
    }

    /* 채널 설정 */
    printf("Audio Channel Mode : %s\n", (channels) ? "Stereo" : "Mono");
    if(snd_pcm_hw_params_set_channels(handle, hw_params, channels) < 0) {
        fprintf(stderr, "Error setting channels\n");
        return -1;
    }

    /* 오디오 포맷 설정 */

    printf("Wave Bytes: %d\n", format);
    switch (format)
    {
        case 1:                     /* Mono 8 bit */
            exact_format = SND_PCM_FORMAT_U8;
            break;
        case 2:                     /* Mono 16 bit or Stereo 8 bit */
            exact_format = (channels == 1) ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_U8;
            break;
        case 4:                     /* If Stereo file */
            exact_format = SND_PCM_FORMAT_S16_LE;
            break; 
        default:
            printf("Unknown Byte rate for sound\n");
            break;
    }

    if(snd_pcm_hw_params_set_format(handle, hw_params, exact_format) < 0) {
        fprintf(stderr, "Could not set Sample format\n");
        return -1;
    }

    /* 샘플링 레이트 설정 */
    printf("Wave Sampling Rate: 0x%u\n", sampleRate);
    val = sampleRate;
    if(snd_pcm_hw_params_set_rate_near(handle, hw_params, &val, &dir) < 0) {
        fprintf(stderr, "Could not setting sampling rate\n");
        return -1;
    }

    /* 주기 설정 */
    frame = period;
    printf("Period: %d\n", frame);
    if(snd_pcm_hw_params_set_periods_near(handle, hw_params, &frame, 0) < 0) {
        fprintf(stderr, "Error setting period\n");
        return -1;
    }

    /* 버퍼 크기 설정*/
    periodsize = 8192;
    buffersize = (periodsize * period) >> 2;
    exact_buffersize = buffersize;
    if(snd_pcm_hw_params_set_buffer_size_near(handle, hw_params, &exact_buffersize) < 0) {
        fprintf(stderr, "Error setting buffer size\n");
        return -1;
    }
    // 가장 근처 값으로 성정된 경우
    if ( buffersize != exact_buffersize ) {
        fprintf(stderr, "The buffersize %lu bytes is not supported by your hardware.\n"
                        "==> Using %lu bytes instead.\n", buffersize, exact_buffersize);
        periodsize = (exact_buffersize << 2) / frame;
    }
    printf("period size: %ld\n", periodsize);
    /* ALSA 드라이버에 오디오 디바이스 파라미터 적용 */
    if ( snd_pcm_hw_params(handle, hw_params) < 0 ) {
        fprintf(stderr, "Error setting HW params.\n");
        return -1;
    }
    *exact_periodsize = periodsize;

    return 1;
}

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
    int fd, ret, pcm_return;
    char* fmt;
    snd_pcm_t   *handle;
    snd_pcm_uframes_t   periodsize;

    int count, period, frames;
    unsigned char *buffer;

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

    ret = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if(ret < 0) {
        fprintf(stderr, "Unable to open pcm device: %s\n", snd_strerror(ret));
        return -1;
    }
    
    period = 2;
    setupDSP(handle, \
            wavheader.nblockAlign, \
            wavheader.sampleRate, \
            wavheader.nChannels, \
            period, \
            &periodsize);
    buffer = (unsigned char *)malloc(periodsize);
    frames = periodsize >> 2;
    
    while ( (count = read(fd, buffer, frames << 2)) > 0 ) {
        if ( (pcm_return = snd_pcm_writei(handle, buffer, count >> 2)) < 0 ) {
            snd_pcm_prepare(handle);
            fprintf(stderr, "<<<<<<<<<< Buffer Underrun >>>>>>>>>>\n");
        }
    }

    close(fd);

    snd_pcm_drain(handle);
    snd_pcm_close(handle);

    free(buffer);

    return 0;
}

