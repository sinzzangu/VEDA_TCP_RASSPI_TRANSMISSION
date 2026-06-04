CC = aarch64-linux-gnu-gcc
LDLIBS = -lwiringPi -lcrypt
LIBOBJS = led.o light_sensor.o i2c_light_sensor.o
TARGET = client1 client2
SENDOBJS = server
ADDRESS = 	juanb0510@100.72.6.87
DIRECTORY = ~/raspi_project/

all: $(TARGET)

%.o: %.c
	$(CC) -c -fPIC $<

server: server.c 
	$(CC) server.c  -o server 

client1: client.c
	gcc client.c -o client1

client2: client.c
	gcc client.c -o client2	

send: server
	scp server $(ADDRESS):$(DIRECTORY)
	
send_server: server
	scp server $(ADDRESS):$(DIRECTORY)

send_all: $(SENDOBJS)
	scp $(SENDOBJS) $(ADDRESS):$(DIRECTORY)

send_yongwoo: server
	scp server vboxuser@172.20.33.109:~

clean:
	rm -f *.o *.so main