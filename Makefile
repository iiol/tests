SRC = $(wildcard *.c)
OBJ = $(SRC:.c=)

CFLAGS = `pkg-config --cflags libdrm libudev gtk+-3.0 libinput xkbcommon alsa`
CFLAGS += -Wall
LFLAGS = `pkg-config --libs libdrm libudev gtk+-3.0 libinput xkbcommon alsa`
LFLAGS += -pthread -lm


build: $(OBJ)
	@echo Done

%: %.c
	gcc $(CFLAGS) $(LFLAGS) -o $@ $<
	@echo __________

libkcapi: libkcapi.c
	gcc $(CFLAGS) $(LFLAGS) -o $@ $< /usr/lib/x86_64-linux-gnu/libkcapi.so

clean:
	rm -f $(OBJ)
