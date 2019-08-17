SRC = $(wildcard *.c)
OBJ = $(SRC:.c=)

CFLAGS = `pkg-config --cflags libdrm libudev gtk+-3.0`
CFLAGS += -Wall
LFLAGS = `pkg-config --libs libdrm libudev gtk+-3.0`
LFLAGS += -pthread


build: $(OBJ)
	@echo Done

%: %.c
	gcc $(CFLAGS) $(LFLAGS) -o $@ $<
	@echo __________

clean:
	rm -f $(OBJ)
