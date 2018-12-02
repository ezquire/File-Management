/*
unlink() deletes a name from the filesystem if it is the last link to a file
and there are no processes with the file open, if the link is symbolic it is 
removed, If the link is a symbolic link it is removed. Since the filename is
a symbolic link to the file, the file is deleted. Attempting to read the file
again results in an error since the file no longer exists.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main (int argc, char **argv) {
	FILE* fptr;

	if(argc != 2) {
		fprintf(stderr, "Error: Usage %s <filename>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	fptr = fopen((char *)argv[1], "r");

	if(fptr == NULL) {
		printf("Error opening file: %s\n", (char *)argv[1]);
		exit(EXIT_FAILURE);
	}

	char line[2];
	fgets(line, 2, fptr);

	pid_t pid;
	int status;

	if ((pid = fork()) < 0) {
		printf("Error: forking child process failed\n");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) {
		if (execvp("unlink", argv) < 0) {
			printf("Error: exec failed\n");
			exit(EXIT_FAILURE);
		}
	}
	else {
		while (wait(&status) != pid) // Wait for child to complete
			;
	}

	fptr = fopen((char *)argv[1], "r");

	if(fptr == NULL) {
		printf("Error opening file: %s\n", (char *)argv[1]);
		exit(EXIT_FAILURE);
	}

	fclose(fptr);
	return 0;
}
