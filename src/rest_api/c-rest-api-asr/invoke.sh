#开始录音
./recording
arecord -D "plughw:0,0" -f S16_LE -r 16000 -d 5 -t wav ./rec.wav
#开始识别
sh build_and_asr.sh
