#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//#define LENGTH 4
//static int **page_table;


// Utility function for turning byte counts into bit counts
int bytes_to_bits(int num_bytes) { return num_bytes * 8; }

int main (int argc, char ** argv) {

	int vbits, pbits, bytes;
	int valid, perm, page, use;
	char *line = NULL;
	//size_t len = 0;
	//ssize_t nread;
	FILE *fptr;
	
	if(argc != 2) {
		fprintf(stderr, "Error: Usage %s <filename>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fptr = fopen((char *)argv[1], "r");
	if(fptr == NULL) {
		printf("Error opening file %s.\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	fscanf(fptr, "%d %d %d", &vbits, &pbits, &bytes);

	printf("vbits: %d\n", vbits);
	printf("pbits: %d\n", pbits);
	printf("bytes: %d\n", bytes);

    int in;
	while((in = fgetc(fptr)) != EOF) {
		fscanf(fptr, "%d %d %d %d", &valid, &perm, &page, &use);
		printf("valid: %d\n", valid);
		printf("perm: %d\n", perm);
		printf("page: %d\n", page);
		printf("use: %d\n", use);
	}

	free(line);
	fclose(fptr);

	/*page_table = malloc(LENGTH * sizeof(int*));
	for (int i = 0; i < LENGTH; ++i) {
		page_table[i] = malloc(LENGTH * sizeof(int));
	}

	while((in = fgetc(fptr)) != EOF) {
		if(in != '\n' && in != ' ') {
			
		}
		}*/
	return 0;
}
