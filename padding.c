#include <stdlib.h>
#include <stdio.h>

#include "padding.h"

struct padding {
	int depth;
	int max_depth;
	char *componets[];
};

padding* pad_create(int max_depth) {
	padding* pad = malloc(sizeof(padding) + (max_depth + 1) * sizeof(char*));
	if (pad == NULL) {
		perror("malloc pad failed");
		exit(EXIT_FAILURE);
	}
	pad->depth = 0;
	pad->max_depth = max_depth;
	return pad;
}

void pad_add(padding* pad, char* str) {
	if (pad->depth >= pad->max_depth) {
		exit(EXIT_FAILURE);
	}
	pad->componets[pad->depth] = str;
	pad->depth += 1;
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
