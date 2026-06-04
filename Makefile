# ====================================================================
#  Top-level Makefile — builds libcontrol, server, and client.
#
#  Targets:
#    all          — build everything (default)
#    libcontrol   — cross-compile libcontrol.so for Pi (aarch64)
#    server       — cross-compile server for Pi (aarch64)
#    client       — native compile client for Ubuntu
#    send         — scp server + libcontrol.so + index.html to Pi
#    send_server  — scp only the server binary
#    send_lib     — scp only the library
#    clean        — remove all build artifacts
# ====================================================================

# --- Compilers --------------------------------------------------------
CC_CROSS  = aarch64-linux-gnu-gcc
CC_NATIVE = gcc

# --- Deployment -------------------------------------------------------
PI_ADDR = juanb0510@100.72.6.87
PI_DIR  = ~/raspi_project/

# --- Library (libcontrol.so) ------------------------------------------
LIB_DIR  = libcontrol
LIB_OUT  = $(LIB_DIR)/libcontrol.so
LIB_SRCS = $(LIB_DIR)/control.c    \
           $(LIB_DIR)/led.c        \
           $(LIB_DIR)/buzzer.c     \
           $(LIB_DIR)/cds.c        \
           $(LIB_DIR)/num_display.c
LIB_FLAGS = -fPIC -shared
LIB_LIBS  = -lwiringPi -lpthread

# --- Server -----------------------------------------------------------
SRV_DIR  = server
SRV_OUT  = $(SRV_DIR)/server
SRV_SRCS = $(SRV_DIR)/server.c         \
           $(SRV_DIR)/daemon.c         \
           $(SRV_DIR)/client_handler.c \
           $(SRV_DIR)/setup.c          \
           $(SRV_DIR)/config.c         \
           $(SRV_DIR)/protocol.c       \
           $(SRV_DIR)/devices.c        \
           $(SRV_DIR)/http.c           \
           $(SRV_DIR)/led_thread.c     \
           $(SRV_DIR)/buzzer_thread.c  \
           $(SRV_DIR)/seg_thread.c     \
           $(SRV_DIR)/cds_thread.c     \
           $(SRV_DIR)/auto_light_thread.c
SRV_LIBS = -ldl -lpthread

# --- Client -----------------------------------------------------------
CLI_DIR  = client
CLI_OUT  = $(CLI_DIR)/client
CLI_SRCS = $(CLI_DIR)/client.c
CLI_LIBS = -lpthread

# --- Files to deploy --------------------------------------------------
HTML = $(SRV_DIR)/index.html
CONF = $(SRV_DIR)/server.conf

# ====================================================================
#  Targets
# ====================================================================
.PHONY: all libcontrol server client send send_server send_lib clean

all: libcontrol server client

libcontrol: $(LIB_OUT)
$(LIB_OUT): $(LIB_SRCS)
	$(CC_CROSS) $(LIB_FLAGS) $(LIB_SRCS) -o $(LIB_OUT) $(LIB_LIBS)

server: $(SRV_OUT)
$(SRV_OUT): $(SRV_SRCS)
	$(CC_CROSS) $(SRV_SRCS) -o $(SRV_OUT) $(SRV_LIBS)

client: $(CLI_OUT)
$(CLI_OUT): $(CLI_SRCS)
	$(CC_NATIVE) $(CLI_SRCS) -o $(CLI_OUT) $(CLI_LIBS)

# --- Deployment -------------------------------------------------------
send: server libcontrol
	scp $(SRV_OUT) $(LIB_OUT) $(HTML) $(CONF) $(PI_ADDR):$(PI_DIR)

send_server: server
	scp $(SRV_OUT) $(HTML) $(PI_ADDR):$(PI_DIR)

send_lib: libcontrol
	scp $(LIB_OUT) $(PI_ADDR):$(PI_DIR)

# --- Cleanup ----------------------------------------------------------
clean:
	rm -f $(LIB_OUT) $(SRV_OUT) $(CLI_OUT) $(LIB_DIR)/*.o