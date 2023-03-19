CC = gcc
CFLAGS = -Wall -Werror -g

all: owl

owl: owl.c servo.o bno055.o tcp.o bbio/libbbio.a
	$(CC) $(CFLAGS) -lm -o owl owl.c servo.o tcp.o bno055.o bno_support.o bbio/libbbio.a

bno055.o: imu/bno_support.c imu/bno_support.h imu/bno055.c imu/bno055.h
	$(CC) $(CFLAGS) -lm -c imu/bno_support.c imu/bno055.c

servo.o: servo.c servo.h
	$(CC) $(CFLAGS) -c servo.c

tcp.o: tcp.c tcp.h
	$(CC) $(CFLAGS) -c tcp.c

bbio/libbbio.a:
	cd bbio && make

clean:
	rm -f main *.o
	cd bbio && make clean
