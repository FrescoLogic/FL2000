all:	evditest

evditest:	main.cpp
	#rm fltest
	#gcc mainb.c -pthread -o fltest
	g++ main.cpp -c
	g++ fl2000lib.cpp -c
	g++ main.o fl2000lib.o -levdi -lpthread -o evditest

clean:
	- rm main.o
	- rm fl2000lib.o
	- rm evditest
