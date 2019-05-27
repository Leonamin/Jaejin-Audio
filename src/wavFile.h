#ifndef  __WAVFILE_H
#define __WAVFILE_H

typedef struct wavFile_t {
    __uint8_t       riffID[4];      //RIFF 파일 지정
    __uint32_t      riffLen;        //데이터 크기
    __uint8_t       waveID[4];      //WAV 파일
    __uint8_t       fmtID[4];       //fmt 청크 시작
    __uint32_t      fmtLen;         //포맷청크 데이터 길이
    __uint16_t      fmtTag;         //압축 형태
    __uint16_t      nChannels;      //채널의 수
    __uint32_t      sampleRate;     //샘플링 레이트
    __uint32_t      avgBytesPerSec; //초당 전송되는 평균 바이트 수
    __uint16_t      nblockAlign;    //한 샘플당 바이트 수
    __uint16_t      bitsPerSample;  //샘플당 비트 수
    __uint8_t       dataID[4];      //data 청크 시작
    __uint32_t      dataLen;        //데이터 크기
} WAVHEADER;

#endif