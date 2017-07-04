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
#include "padding.h"

static int max_depth = 2;

static char* join_path(char* a, ssize_t alen, char* b, ssize_t blen) {
	ssize_t len = alen + 1 + blen;
	char* buf = malloc(sizeof(char) * (len + 1));
	if (buf == NULL) {
		perror("malloc failed");
		exit(EXIT_FAILURE);
	}
	strcpy(buf, a);
	buf[alen] = '/';
	strcpy(buf + alen + 1, b);
	buf[len] = '\0';
	return buf;
}

static struct dirent *filelist(char* dir, int *filec) {
	DIR *dirp = opendir(dir);
	if (dirp == NULL) return NULL;

	size_t entrys_size = 10;
	struct dirent *entrys = malloc(sizeof(struct dirent) * entrys_size);
	if (entrys == NULL) {
		perror("malloc entrys failed");
		exit(EXIT_FAILURE);
	}

	int count = 0;
	while (true) {
		if (count >= entrys_size) {
			entrys_size += 10;
			entrys = realloc(entrys, sizeof(struct dirent) * entrys_size);
			if (entrys == NULL) {
				perror("remalloc entrys failed");
				exit(EXIT_FAILURE);
			}
		}

		struct dirent *e = entrys + count;
		struct dirent *result = NULL;
		if (readdir_r(dirp, e, &result) != 0) {
			perror("readdir_r failed");
			exit(EXIT_FAILURE);
		}

		if (result == NULL)
			break;

		if (result->d_name[0] == '.' && (result->d_name[1] == '\0' ||
			(result->d_name[1] == '.' && result->d_name[2] == '\0')))
			continue;

		count += 1;
	}

	*filec = count;
	closedir(dirp);
	return entrys;
}

static void crawl(char* dir, ssize_t dirlen, padding *pad) {
	int filec;
	struct dirent *files = filelist(dir, &filec);
	if (files == NULL) {
		perror("couldn't open the directory");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < filec; i++) {
		char* name = files[i].d_name;

		printf("%s", COL_GREY);
		pad_print(pad);
		bool isdir = (files[i].d_type & DT_DIR) == DT_DIR;
		bool islast = i == filec - 1;
		if (pad_getDepth(pad) < max_depth && isdir) {
			char* path = join_path(dir, dirlen, name, files[i].d_namlen);

			printf("%s%s%s/\n", islast ? PAD_TYPE_6 : PAD_TYPE_5, COL_RESET, name);
			pad_add(pad, islast ? PAD_TYPE_1 : PAD_TYPE_2);
			crawl(path, strlen(path), pad);
			pad_pop(pad);

			free(path);
		} else if (isdir) {
			printf("%s%s%s/\n", islast ? PAD_TYPE_4 : PAD_TYPE_3, COL_RESET, name);
		} else {
			printf("%s%s%s\n", islast ? PAD_TYPE_4 : PAD_TYPE_3, COL_RESET, name);
		}
	}
	if (filec == 0) {
		printf("%s", COL_GREY);
		pad_print(pad);
		printf("%s[empty]%s\n", PAD_TYPE_4, COL_RESET);
	}

	free(files);
}


int main(int argc, char* argv[]) {
	char* directory = NULL;
	char* help = "usage: ctree [path] [options...]\n\t-h | --help \t prints this text\n\t-d | --depth \t <int>\n";
	for (int i = 1; i<argc; i++) {
		char* arg = argv[i];
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
			if (arg[0] == '-' || directory != NULL) {
				fprintf(stderr, "unknown option: '%s'\n", arg);
				return EXIT_FAILURE;
			} else {
				directory = arg;
			}
		}
	}

	if (directory == NULL)
		directory = ".";

	ssize_t dirlen = strlen(directory);
	if (directory[dirlen - 1] == '/') {
		directory[dirlen - 1] = '\0';
		dirlen -= 1;
	}

	struct stat stats;
	if (stat(directory, &stats) != 0) {
		perror("stat");
		return EXIT_FAILURE;
	}

	if ((stats.st_mode & S_IFMT) != S_IFDIR) {
		fprintf(stderr, "'%s' should be a directory\n", directory);
		return EXIT_FAILURE;
	}

	printf("%s\n", directory);

	crawl(directory, dirlen, pad_create());

	return EXIT_SUCCESS;
}
