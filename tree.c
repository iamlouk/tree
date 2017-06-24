#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include "colors.h"

static char *dir = NULL;
static int max_depth = 2;
static char *help = "usage: ctree [path] [options...]\n\t-h | --help \t prints this text\n\t-d | --depth \t <int>\n";

static char *p1 = "    "; //L"    ";
static char *p2 = "|   "; //L"│   ";
static char *p3 = "+----- "; //L"├───── ";
static char *p4 = "`----- "; //L"└───── ";
static char *p5 = "+---+- "; //L"├───┬─ ";
static char *p6 = "`---+- "; //L"└───┬─ ";
typedef struct PAD {
	int depth;
	char **strs;
} PAD;
static void add_padding(PAD *pad, char *str) {
	pad->depth += 1;
	pad->strs = realloc(pad->strs, pad->depth * sizeof(char*));
	if (pad->strs == NULL) {
		perror("realloc pad->strs failed");
		exit(EXIT_FAILURE);
	}
	pad->strs[pad->depth - 1] = str;
}
static void remove_padding(PAD *pad) {
	if (pad->depth <= 0) {
		fprintf(stderr, "remove_padding(*pad): pad->depth <= 0\n");
		exit(EXIT_FAILURE);
	}
	pad->depth -= 1;
	/*pad->strs = realloc(pad->depth * sizeof(wchar_t*));
	if (pad->strs == NULL) {
		perror("realloc pad->strs failed");
		exit(EXIT_FAILURE);
	}*/
}
static void print_padding(PAD *pad) {
	for (int i = 0; i < pad->depth; i++) {
		printf("%s", pad->strs[i]);
	}
}


static char * join_path(char * a, ssize_t alen, char * b, ssize_t blen){
	ssize_t len = alen + 1 + blen;
	char * buf = malloc(sizeof(char) * (len + 1));
	if (buf == NULL) {
		perror("malloc path buf failed");
		exit(EXIT_FAILURE);
	}

	int i = 0;
	for (i = 0; i<alen; i++) {
		buf[i] = a[i];
	}
	buf[i] = '/';
	i += 1;
	for (int j = 0; j<blen; j++) {
		buf[i] = b[j];
		i += 1;
	}
	
	buf[len] = '\0';
	return buf;
}

static struct dirent * get_files_in(char *dir, int *filec) {
	DIR *dirstream = opendir(dir);
	if (dirstream == NULL) {
		return NULL;
	}

	size_t entrys_size = 10;
	struct dirent * entrys = malloc(sizeof(struct dirent) * entrys_size);
	if (entrys == NULL) {
		perror("malloc entrys failed");
		exit(EXIT_FAILURE);
	}

	int i = 0;
	while (true) {
		if (i >= entrys_size) {
			entrys_size += 5;
			entrys = realloc(entrys, sizeof(struct dirent) * entrys_size);
			if (entrys == NULL) {
				perror("remalloc entrys failed");
				exit(EXIT_FAILURE);
			}
		}

		struct dirent *e = entrys + i;
		struct dirent *result = NULL;
		
		if (readdir_r(dirstream, e, &result) != 0) {
			/*perror("readdir_r failed");*/
			/*exit(EXIT_FAILURE);*/
			free(entrys);
			return NULL;
		}

		if (result == NULL)
			break;

		if (result->d_name[0] == '.' && (result->d_name[1] == '\0' || (result->d_name[1] == '.' && result->d_name[2] == '\0')))
			continue;

		i += 1;
	}

	entrys = realloc(entrys, sizeof(struct dirent) * i);
	*filec = i;
	closedir(dirstream);
	return entrys;
}

static void print_rek(char * dir, ssize_t dirlen, PAD *pad) {
	int filec;
	struct dirent *files = get_files_in(dir, &filec);
	if (files == NULL) {
		perror("couldn't open the directory");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < filec; i++) {
		char *name = files[i].d_name;
		
		printf("%s", COL_GREY);	
		print_padding(pad);
		bool isdir = (files[i].d_type & DT_DIR) == DT_DIR;
		bool islast = i == filec - 1;
		if (pad->depth < max_depth && isdir) {
			char *path = join_path(dir, dirlen, name, files[i].d_namlen);
						
			printf("%s%s%s/\n", islast ? p6 : p5, COL_RESET, name);
			add_padding(pad, islast ? p1 : p2);
			print_rek(path, strlen(path), pad);
			remove_padding(pad);
			free(path);
		} else if (isdir) {
			printf("%s%s%s/\n", islast ? p4 : p3, COL_RESET, name);
		} else {
			printf("%s%s%s\n", islast ? p4 : p3, COL_RESET, name);
		}
	}
	if (filec == 0) {
		printf("%s", COL_GREY);
		print_padding(pad);
		printf("%s[empty]%s\n", p4, COL_RESET);
	}	

	free(files);
}

int main(int argc, char *argv[]) {
	for (int i = 1; i<argc; i++) {
		char *arg = argv[i];
		if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
			printf("%s", help);
			return EXIT_SUCCESS;
		} else if (strcmp(arg, "-d") == 0 || strcmp(arg, "--depth") == 0) {
			if (i + 1 < argc) {
				max_depth = atoi(argv[i+1]);
				i += 1;
			} else {
				fprintf(stderr, "%s", help);
				return EXIT_FAILURE;
			}
		} else {
			if (arg[0] == '-' || dir != NULL) {
				fprintf(stderr, "unknown option: '%s'\n", arg);
				return EXIT_FAILURE;
			} else {
				dir = arg;
			}
		}
	}

	if (dir == NULL)
		dir = ".";

	ssize_t dirlen = strlen(dir);
	if (dir[dirlen - 1] == '/') {
		dir[dirlen - 1] = '\0';
		dirlen -= 1;
	}
	
	struct stat stats;
	if (stat(dir, &stats) != 0) {
		perror("stat() failed");
		return EXIT_FAILURE;
	}

	if ((stats.st_mode & S_IFMT) != S_IFDIR) {
		fprintf(stderr, "'%s' should be a directory\n", dir);
		return EXIT_FAILURE;
	}

	printf("%s\n%s|%s\n", dir, COL_GREY, COL_RESET);
	PAD *pad = calloc(1, sizeof(pad));
	print_rek(dir, dirlen, pad);

	return EXIT_SUCCESS;
}

