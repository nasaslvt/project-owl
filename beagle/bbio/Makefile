CC = gcc
CFLAGS = -Wall -pthread

all: libbbio.a

libbbio.a: c_adc.o c_pinmux.o c_pwm.o c_uart.o common.o event_gpio.o
	ar rcs libbbio.a c_adc.o c_pinmux.o c_pwm.o c_uart.o common.o event_gpio.o

c_adc.o: c_adc.c c_adc.h
	$(CC) $(CFLAGS) -c c_adc.c

c_pinmux.o: c_pinmux.c c_pinmux.h
	$(CC) $(CFLAGS) -c c_pinmux.c

c_pwm.o: c_pwm.c c_pwm.h
	$(CC) $(CFLAGS) -c c_pwm.c

c_uart.o: c_uart.c c_uart.h
	$(CC) $(CFLAGS) -c c_uart.c

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c

event_gpio.o: event_gpio.c event_gpio.h
	$(CC) $(CFLAGS) -c event_gpio.c

clean:
	rm -f libbbio.a *.o
