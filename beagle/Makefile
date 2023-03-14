CC = gcc
CFLAGS = -Wall -Werror -g

all: main

main: servo.o bbio/libbbio.a
	$(CC) $(CFLAGS) -o main main.c servo.o bbio/libbbio.a

servo.o: servo.c servo.h
	$(CC) $(CFLAGS) -c servo.c

bbio/libbbio.a:
	cd bbio && make

clean:
	rm -f main *.o
	cd bbio && make clean
