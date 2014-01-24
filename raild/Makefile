CC=clang
CFLAGS=-I../shared/
LDFLAGS=-lluajit-5.1
EXEC=raild
SRC= $(wildcard src/*.c)
OBJ= $(SRC:.c=.o)

all: $(EXEC)

raild: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c src/raild.h
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean

clean:
	rm -rf src/*.o
