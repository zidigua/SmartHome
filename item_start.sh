export LD_LIBRARY_PATH=$(pwd)/src/awake_demo/libs/x64

AWAKE_PATH=$(pwd)/src/awake_demo/bin/
CUR_PATH=$(pwd)

while (true) 
do
	#唤醒过程
	cd $AWAKE_PATH
	./awaken_offline_sample
	cd $CUR_PATH
	aplay ./lib/awake.wav   # 播放唤醒音频

	#开始录音
	arecord -D "plughw:0,0" -f S16_LE -r 16000 -d 5 -t wav $(pwd)/result/rec.wav 

	#语音识别
	./bin/asrmain 

	ret=$?
	if [ $ret -ne 0 ]
	then
		echo "skip analyzing, Timeout was reached."
		continue
	fi

	#执行语音动作
	./bin/sound_ctrl 192.168.0.102  8888

	ret=$?
	if [ $ret -ne 0 ]
	then
		echo "analyzing fail"
		continue
	fi

	./bin/ttsmain

	ret=$?
	if [ $ret -eq 1 ]
	then
		aplay  ./lib/undef.wav
		continue
	fi

	aplay  ./result/result.wav

done
