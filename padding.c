#include <stdlib.h>
#include <stdio.h>

#include "padding.h"

struct padding {
	int depth;
	char **componets;
};

padding* pad_create(void) {
	padding* pad = malloc(sizeof(padding));
	if (pad == NULL) {
		perror("malloc pad failed");
		exit(EXIT_FAILURE);
	}
	pad->depth = 0;
	pad->componets = NULL;
	return pad;
}

void pad_add(padding* pad, char* str) {
	pad->depth += 1;
	pad->componets = realloc(pad->componets, pad->depth * sizeof(char*));
	if (pad->componets == NULL) {
		perror("realloc pad->componets failed");
		exit(EXIT_FAILURE);
	}
	pad->componets[pad->depth - 1] = str;
}

void pad_pop(padding* pad) {
	if (pad->depth <= 0)
		exit(EXIT_FAILURE);
	pad->depth -= 1;
}

void pad_print(padding* pad) {
	for (int i = 0; i < pad->depth; i++)
		printf("%s", pad->componets[i]);
}

int pad_getDepth(padding* pad) {
	return pad->depth;
}
