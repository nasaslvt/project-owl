CC := g++
CFLAGS := -Wall -Werror -O2
DEBUG_FLAGS := -g
OBJ := build/tcp.o build/parser.o
#uncomment below line for debug build
#CFLAGS += DEBUG_FLAGS

all: build/kiss_receive build/kiss_transmit build/kiss_parser

build/tcp.o: tcp.cpp
	$(CC) $(CFLAGS) -c tcp.cpp -o build/tcp.o

build/parser.o: parser.cpp
	$(CC) $(CFLAGS) -c parser.cpp -o build/parser.o

build/kiss_receive: kiss_receive.cpp $(OBJ)
	$(CC) $(CFLAGS) kiss_receive.cpp -o build/kiss_receive $(OBJ)

build/kiss_transmit: kiss_transmit.cpp $(OBJ)
	$(CC) $(CFLAGS) kiss_transmit.cpp -o build/kiss_transmit $(OBJ)

build/kiss_parser: kiss_parser.cpp $(OBJ)
	$(CC) $(CFLAGS) kiss_parser.cpp -o build/kiss_parser $(OBJ)

clean:
	rm -f build/*