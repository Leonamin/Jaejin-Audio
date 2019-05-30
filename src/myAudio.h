#ifndef __MYAUDIO_H
#define __MYAUDIO_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include "wavFile.h"

int procaudio(char *filename);
int setupDSP(snd_pcm_t *handle, int format, int sampleRate, int channels, int period, snd_pcm_uframes_t *exact_periodsize);

#endif