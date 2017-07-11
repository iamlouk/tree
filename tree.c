#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include "colors.h"
#include "padding.h"

static int max_depth = 2;

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
	struct dirent* entry;
	while (errno = 0, (entry = readdir(dirp)) != NULL) {
		if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' ||
			(entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
			continue;

		if (count >= entrys_size) {
			entrys_size += 10;
			entrys = realloc(entrys, sizeof(struct dirent) * entrys_size);
			if (entrys == NULL) {
				perror("remalloc entrys failed");
				exit(EXIT_FAILURE);
			}
		}

		struct dirent* dest = &(entrys[count]); // entrys + count
		memcpy(dest, entry, sizeof(struct dirent));
		
		count += 1;
	}
	if (errno != 0) {
		perror("readdir");
		exit(EXIT_FAILURE);
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
			size_t pathlen = dirlen + 1 + files[i].d_namlen;
			char path[pathlen + 1];
			strcpy(path, dir);
			path[dirlen] = '/';
			strcpy(path + dirlen + 1, name);

			printf("%s%s%s/\n", islast ? PAD_TYPE_6 : PAD_TYPE_5, COL_RESET, name);
			pad_add(pad, islast ? PAD_TYPE_1 : PAD_TYPE_2);
			crawl(path, strlen(path), pad);
			pad_pop(pad);

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

	crawl(directory, dirlen, pad_create(max_depth));

	return EXIT_SUCCESS;
}
