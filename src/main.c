#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include "wavFile.h"
#include "myAudio.h"

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