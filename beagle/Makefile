CC = gcc
CFLAGS = -Wall -Werror -g

all: main

main: servo.o bno055.o bbio/libbbio.a
	$(CC) $(CFLAGS) -o main main.c servo.o bbio/libbbio.a

bno055.o: imu/bno055.c imu/bno055.h
	$(CC) $(CFLAGS) -c imu/bno055.c

servo.o: servo.c servo.h
	$(CC) $(CFLAGS) -c servo.c

bbio/libbbio.a:
	cd bbio && make

clean:
	rm -f main *.o
	cd bbio && make clean
