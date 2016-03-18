SOURCE = $(wildcard *.c)

OBJS = $(patsubst %.c,%.o,$(SOURCE))

CFLAGS =-g -Wall -O2 -v -H -D_FILE_OFFSET_BITS=64 -DDEBUG

#CC = arm-linux-gcc
CC = arm-none-linux-gnueabi-gcc-4.4.1

#LD = arm-linux-ld
LD = arm-none-linux-gnueabi-ld

INCLUDE = -I ./ 

LDFLAGS = -L ./

wifi_camera: $(OBJS)

	$(CC) $(CFLAGS) -o wifi_camera $(INCLUDE) $(LDFLAGS)  $(OBJS) -ljpeg -lm -lpthread -static

.PHONY: clean
clean:
	rm -f *.o wifi_camera
	
