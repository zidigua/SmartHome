CC=gcc
APP_NAME=sound_ctrl
TCP_CORE=tcp_core

all:
	$(CC) $(APP_NAME).c  $(TCP_CORE).c -o $(APP_NAME)
	cp $(APP_NAME) ../../bin/
	cp device_table.txt ../../result/
clean:
	rm sound_ctrl
