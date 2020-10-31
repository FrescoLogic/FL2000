arm_prefix = /opt/hi3798/arm-hisiv200-linux/bin/arm-hisiv200-linux-gnueabi

all:	fltest

fltest:	main.c
	#rm fltest
	#gcc mainb.c -pthread -o fltest  
	gcc main.c -c
	gcc screencapture.c -c
	gcc main.o screencapture.o -lpthread -lX11 -o fltest

fltest_arm:	main.c
	$(arm_prefix)-gcc main.c -Xlinker -static -static-libgcc -o fltest_arm
	$(arm_prefix)-strip fltest_arm
	
clean:
	- rm main.o
	- rm screencapture.o 
	- rm fltest
