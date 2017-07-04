
CC="gcc"
CFLAGS="-Wall" "-Werror"
BIN=/usr/local/bin

.PHONY: clean

tree: tree.c colors.h padding.h padding.o
	$(CC) $(CFLAGS) -o tree tree.c padding.o

padding.o: padding.h padding.c
	$(CC) $(CFLAGS) -c padding.c

clean:
	rm -f tree padding.o

install: tree
	mv "./tree" "$(BIN)/tree"
