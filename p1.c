#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

static uint32_t *pagetable;

// Utility function for turning byte counts into bit counts
//int bytes_to_bits(int num_bytes) { return num_bytes * 8; }

int main (int argc, char ** argv) {

	int vbits, pbits, bytes;
	int valid, perm, page, use;
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

	int obits = (int)log2(bytes);
	int npages = (int)pow(2, (vbits - obits));
	printf ("npages: %d\n", npages);

	//Allocate space for the page table #of pages = 2^(vbits - obits) 
	pagetable = malloc(npages * sizeof(uint32_t));

	/*printf("vbits: %d\n", vbits);
	printf("pbits: %d\n", pbits);
	printf("bytes: %d\n", bytes);*/

    int in;
	while((in = fgetc(fptr)) != EOF) {
		fscanf(fptr, "%d %d %d %d", &valid, &perm, &page, &use);
	}

	/*printf("valid: %d\n", valid);
	printf("perm: %d\n", perm);
	printf("page: %d\n", page);
	printf("use: %d\n", use);*/

	free(pagetable);
	fclose(fptr);
	return 0;
}
