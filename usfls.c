#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
	

	DIR *dir_stream;
	struct dirent *direct;

	if (argv[1] == NULL) {
		// open the current directory
		dir_stream = opendir(".");
	}
	else {
		// open the directory that is given
		dir_stream = opendir(argv[1]);
	}

	if (dir_stream == NULL) {
		printf("Cannot open directory\n");
		exit(-1);
	}

	while ((direct = readdir(dir_stream)) != NULL) {
		if (strncmp(direct -> d_name, ".", 1) != 0) {
			printf("%s\n", direct -> d_name);
		}
	}

	closedir(dir_stream);

	return 0;

}