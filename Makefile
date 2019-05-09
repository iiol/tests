SRC = $(wildcard *.c)
OBJ = $(SRC:.c=)

CFLAGS = -Wall
LFLAGS = `pkg-config --cflags --libs libdrm libudev`


build: $(OBJ)
	@echo Done

%: %.c
	gcc $(CFLAGS) $(LFLAGS) -o $@ $<

clean:
	rm $(OBJ)
