arm_prefix = /opt/hi3798/arm-hisiv200-linux/bin/arm-hisiv200-linux-gnueabi

all:	fltest fltest_arm

fltest:	main.c
	gcc main.c -o fltest

fltest_arm:	main.c
	$(arm_prefix)-gcc main.c -Xlinker -static -static-libgcc -o fltest_arm
	$(arm_prefix)-strip fltest_arm
