/*
unlink() deletes a name from the filesystem if it is the last link to a file
and there are no processes with the file open, if the link is symbolic it is 
removed. Since the filename is a symbolic link to the file, the file is deleted
, but the data is not overwritten. This allows us to rewind back to the 
beginning of the deleted file and read the same data, although when the 
program exits that filename will no longer exist. This is not the same as shred
which will write new data to the same location as well as deleting the
symbolic link.
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

	char c = fgetc(fptr);
	printf("Read character: %c\n", c);
	c = 'X';
	printf("Changed character to X. c = %c\n", c);

	pid_t pid;
	int status;

	if ((pid = fork()) < 0) {
		printf("Error: forking child process failed\n");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) {
		printf("Forked child process, deleting file: %s using unlink().\n",
			   (char *)argv[1]);
		if (execvp("unlink", argv) < 0) {
			printf("Error: exec failed\n");
			exit(EXIT_FAILURE);
		}
	}
	else
		while (wait(&status) != pid) // Wait for child to complete
			;
	
	printf("Attempting to read deleted file.\n");
	rewind(fptr);
	c = fgetc(fptr);
	printf("Character after reading deleted file: %c\n", c);

	fclose(fptr);
	return 0;
}
