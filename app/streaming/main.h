#ifndef __MAIN__
#define __MAIN__

#include <stdint.h>
#include <alsa/asoundlib.h>

#define DEBUG
#define METADATA_SIZE 32

typedef struct
{
    // WAV FMT
	uint8_t         FmtID[4];
	uint32_t        FmtSize;
 	uint16_t        AudioFormat;
 	uint16_t        NumChannels;
 	uint32_t        SampleRate;
 	uint32_t        AvgByteRate;
 	uint16_t        BlockAlign;
 	uint16_t        BitPerSample;
 
 	// DATA FMT
 	uint8_t         DataID[4];
 	uint32_t        DataSize;
 } META;

typedef struct
{
	snd_pcm_t * soundDev;
	uint32_t size;
	uint32_t played;
	uint8_t * data;
} BUFF;

#endif
