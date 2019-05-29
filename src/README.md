# 코드 설명
## main.c
### procaudio
#### 오디오 관련 태스크를 처리하는 함수
### setupDSP
#### pcm에 관한 파리미터를 설정하는 함수
#### handle: pcm handler
#### format: 양자화 비트 오디오 포맷
#### samplRate: 표본화율
#### channels: 모노/스테레오(1/2)
#### period: 하드웨어 버퍼 접근 주기