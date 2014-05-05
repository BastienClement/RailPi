CC=clang
CFLAGS=-I../shared/ -O3
LDFLAGS=-lluajit-5.1
EXEC=raild
SRC= $(wildcard src/*.c)
SRC_LUA= $(wildcard src/*.lua)
OBJ= $(SRC:.c=.o) $(SRC_LUA:.lua=.lo)

all: $(EXEC)

raild: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c src/raild.h
	$(CC) -o $@ -c $< $(CFLAGS)

%.lo: %.lua
	objcopy -I binary -O elf32-littlearm -B arm $< $@

.PHONY: clean

clean:
	rm -rf src/*.o src/*.lo
