
CC="gcc"
CFLAGS="-Wall" "-Werror"
BIN=/usr/local/bin

tree: tree.c colors.h
	$(CC) $(CFLAGS) tree.c -o tree

install: tree
	mv "./tree" "$(BIN)/tree"

