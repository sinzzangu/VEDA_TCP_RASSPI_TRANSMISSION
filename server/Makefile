CC      = aarch64-linux-gnu-gcc
ADDRESS = juanb0510@100.72.6.87
DIRECTORY = ~/raspi_project/
SERVER_SRCS = server.c        \
              daemon.c        \
              client_handler.c \
              setup.c         \
              config.c        \
              protocol.c      \
              devices.c       \
              http.c          \
              led_thread.c    \
              buzzer_thread.c \
              seg_thread.c    \
              cds_thread.c    \
              auto_light_thread.c

SENDOBJS = server index.html

all: server

server: $(SERVER_SRCS)
	$(CC) $(SERVER_SRCS) -o server -ldl -lpthread

send: server
	scp server index.html $(ADDRESS):$(DIRECTORY)

send_server: server
	scp server $(ADDRESS):$(DIRECTORY)

send_all: $(SENDOBJS)
	scp $(SENDOBJS) $(ADDRESS):$(DIRECTORY)

send_yongwoo: server
	scp server vboxuser@172.20.33.109:~

clean:
	rm -f *.o server