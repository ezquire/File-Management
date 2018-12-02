#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

static uint32_t *pagetable;

// Utility function to extract the page number from the entry
uint32_t extract_page(int table_entry, int page_bits) { 
    return (((1 << page_bits) - 1) & (table_entry >> 1)); 
}

// Utility function to truncate bit strings to a set number of bits
uint32_t truncate(int input, int num_bits) {
	return (((1 << num_bits) - 1) & (input));
}


// Utility function to convert hex to decimal and truncate address if needed
uint32_t process_address(char addr[], int max_addr, int vbits) {
	int len = strlen(addr);
	int base = 1;
	uint32_t ret = 0;

	if (addr[0] == '0' && addr[1] == 'x')
		addr += 2;
	
	for (int i=len-1; i>=0; i--) {
		if (addr[i] >= '0' && addr[i] <= '9') {
			ret += (addr[i] - 48) * base;
			base *= 16;
		}
		else if (addr[i] >= 'A' && addr[i] <= 'F') {
			ret += (addr[i] - 55) * base;
			base *= 16;
		}
		else if (addr[i] >= 'a' && addr[i] <= 'f') {
			ret += (addr[i] - 87) * base;
			base *= 16;
		}
	}

	if(ret > max_addr) {
		printf("Given address is greater than the max allowed, truncating\n");
		ret = truncate(ret, vbits);
		printf("New Virtual Address: 0x%X\n", ret);
	}	
	return ret;
}

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

	int off_bits = (int)log2(bytes);
	int index_bits = vbits - off_bits;
	int page_bits = pbits - off_bits;
	int entry_bits = page_bits + 3; // Three extra bits for valid, perm, use
	int valid_mask = (int)pow(2, entry_bits - 1);
	int perm_mask = (int)pow(2, entry_bits - 2);
	int vpages = (int)pow(2, index_bits);
	int max_v_addr = (int)pow(2, vbits) - 1;

	// Allocate space for pagetable
	pagetable = malloc(vpages * sizeof(uint32_t));

	int i = 0;
	//int in;
	while(fgetc(fptr) != EOF) {
		fscanf(fptr, "%d %d %d %d", &valid, &perm, &page, &use);
		pagetable[i] |= ((valid << 1) | perm); // add valid and perm bits
		pagetable[i] <<= page_bits; // make room for page number
		pagetable[i] |= page; // add page number
		pagetable[i] <<= 1; // make room for "use" bit
		pagetable[i] |= use; // add "use" bit
		++i;
	}
	fclose(fptr);

	char virtual_address[22]; // is 22 enough?
	printf("Enter a virtual address in hexadecimal: ");
	fgets(virtual_address, 22, stdin);
	//printf("Virtual Address read: %s", virtual_address);	
	uint32_t dec_addr = process_address(virtual_address, max_v_addr, vbits);
	uint32_t offset = truncate(dec_addr, off_bits);
	int index = dec_addr >> off_bits;
	int entry = pagetable[index];

	if((entry & perm_mask) == 0)
		printf("SEGFAULT\n");
	else if((entry & valid_mask) == 0 && (entry & perm_mask) == perm_mask)
		printf("DISK\n");
	else {
		int page_number = extract_page(entry, page_bits);
		uint32_t phys_addr = (page_number << off_bits) | offset;
		printf("Corresponding Physical Address: 0x%X\n", phys_addr);
	}

	// Free allocated memory
	free(pagetable);

	return 0;
}
