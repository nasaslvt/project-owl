CC := g++
CFLAGS := -Wall -Werror -O2
DEBUG_FLAGS := -g
#uncomment below line for debug build
#CFLAGS += DEBUG_FLAGS

all: build/kiss_receive build/kiss_transmit

build/tcp.o: tcp.cpp
	$(CC) $(CFLAGS) -c tcp.cpp -o build/tcp.o

build/kiss_receive: kiss_receive.cpp build/tcp.o
	$(CC) $(CFLAGS) kiss_receive.cpp -o build/kiss_receive build/tcp.o

build/kiss_transmit: kiss_transmit.cpp build/tcp.o
	$(CC) $(CFLAGS) kiss_transmit.cpp -o build/kiss_transmit build/tcp.o

clean:
	rm -f build/*