CC = aarch64-linux-gnu-gcc
LDLIBS = -lwiringPi -lpthread -lcrypt
LIBOBJS = control.o led.o cds.o buzzer.o num_display.o
SENDOBJS = libcontrol.so main
ADDRESS = 	juanb0510@100.72.6.87
DIRECTORY = ~/raspi_project/

all: $(SENDOBJS)

%.o: %.c
	$(CC) -c -fPIC $<

libcontrol.so: $(LIBOBJS)
	$(CC) -shared -o libcontrol.so *.o $(LDLIBS)

main: main.c
	$(CC) main.c -o main -ldl $(LDLIBS)

send_main: main
	scp main $(ADDRESS):$(DIRECTORY)

send_lib: libcontrol.so
	scp libcontrol.so $(ADDRESS):$(DIRECTORY)

send: $(SENDOBJS)
	scp $(SENDOBJS) $(ADDRESS):$(DIRECTORY)

clean:
	rm -f *.o *.so main