bttcp : main.o ComInit.o
	cc -o bttcp main.o ComInit.o -lpthread -lbluetooth
main.o : main.c ComInit.h
	cc -c main.c -lbluetooth
ComInit.o : ComInit.c ComInit.h
	cc -c ComInit.c
clean : 
	rm main.o ComInit.o
