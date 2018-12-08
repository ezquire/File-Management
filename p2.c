#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

static uint32_t *pagetable;
static uint32_t *physicalPageTable;
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
       	int ppages = (int)pow(2, page_bits);
	physicalPageTable = malloc(ppages * sizeof(uint32_t));		
	//initialize clockhand to index 0;
	int clockHand = 0;
	int virtual_index_mask = ((int)pow(2, index_bits) - 1) << (page_bits + 1);
	int physical_page_mask = ((int)pow(2, page_bits) - 1) << 1;
	// Allocate space for pagetable
	pagetable = malloc(vpages * sizeof(uint32_t));
	int i = 0;
	int j = 0;
	//int in;
	while(fgetc(fptr) != EOF) {
		fscanf(fptr, "%d %d %d %d", &valid, &perm, &page, &use);
		pagetable[i] |= ((valid << 1) | perm); // add valid and perm bits
		pagetable[i] <<= page_bits; // make room for page number
		pagetable[i] |= page; // add page number
		pagetable[i] <<= 1; // make room for "use" bit
		pagetable[i] |= use; // add "use" bit
		//initialize physical page table
		if(valid == 1 && perm == 1) {
			physicalPageTable[j] |= i; //add index
			physicalPageTable[j] <<= page_bits; //make room for virtual page table index
			physicalPageTable[j] |= page; //add physical page number
			physicalPageTable[j] <<= 1; //make room for "use" bit
			physicalPageTable[j] |= use; //add "use" bit
				j++;
		}
		++i;
	}
	fclose(fptr);
	char virtual_address[22]; // is 22 enough?
	printf("Enter a virtual address in hexadecimal (! to quit): ");
	fgets(virtual_address, 22, stdin);
	while(virtual_address[0] != '!'){
		//printf("Virtual Address read: %s", virtual_address);	
		uint32_t dec_addr = process_address(virtual_address, max_v_addr, vbits);
		uint32_t offset = truncate(dec_addr, off_bits);
		int index = dec_addr >> off_bits;
		int entry = pagetable[index];

		if((entry & perm_mask) == 0)
			printf("SEGFAULT\n");
		else if((entry & valid_mask) == 0 && (entry & perm_mask) == perm_mask) {
			printf("PAGE FAULT\n");
			int pageReplaced = 0;
			uint32_t phys_addr;
			while(!pageReplaced){
				//if use bit is 0 replace the page
				if((physicalPageTable[clockHand] % 2) == 0) {
					//printf("replacing at clockHand: %d\n",clockHand);
					//printf("old pagetable value before setting valid bit to 0: %X\n", pagetable[physicalPageTable[clockHand] >> (1+page_bits)]);
					//set valid bit at old index in virtual page table to 0
					pagetable[physicalPageTable[clockHand] >> (1 + page_bits)] ^= valid_mask;
					//printf("after: %X\n",pagetable[physicalPageTable[clockHand] >> (1+page_bits)]);
					//update the physical page table
					printf("physicalPageTable value before mods: %X\n", physicalPageTable[clockHand]);
					physicalPageTable[clockHand] &= ~virtual_index_mask; //clear old index bits
					printf("after xoring virtual index mask: %X\n", physicalPageTable[clockHand]);
					physicalPageTable[clockHand] |= index << (page_bits + 1); //add new index bits
					printf("index(decimal): %d", index);
					printf("after adding the new index bits: %X\n", physicalPageTable[clockHand]);
					physicalPageTable[clockHand] += 1; //set "use" bit to 1
				printf("after making usebit 1: %X\n",physicalPageTable[clockHand]);	
					//update the virtual page table 
					pagetable[index] |= valid_mask; // set valid bit to 1
					pagetable[index] &= ~physical_page_mask;
					pagetable[index] |= (physicalPageTable[clockHand] ^ (virtual_index_mask + 1)); //set physical page number
					pagetable[index] |= 1;// set use bit to 1
					printf("updated pagetable value at index %d: %X", index, pagetable[index]);
					int page_number = extract_page(pagetable[index], page_bits);
					phys_addr = (page_number << off_bits) | offset;	
					pageReplaced = 1;
				} else {
					//change physicalPageTable "use" bit to 0;
					physicalPageTable[clockHand] -= 1;
					//change pageTable "use" bit to 0;
					pagetable[physicalPageTable[clockHand] >> (1 +page_bits)] >>= 1;
					pagetable[physicalPageTable[clockHand] >> (1 +page_bits)] <<= 1;

				}
				clockHand++;
				if (clockHand == j) {
					clockHand = 0;
				}	
			}		
			printf("Physical address: 0x%X\n", phys_addr);
		} else {
			int page_number = extract_page(entry, page_bits);
			uint32_t phys_addr = (page_number << off_bits) | offset;
			printf("Corresponding Physical Address: 0x%X\n", phys_addr);
			if(entry % 2 != 0){
				pagetable[index] |= 1;
				//for each resident page in the page table
				for(int p = 0; i < j; p++){
					//if the index is the same as the virtual index in the physical page table
					if(index == (physicalPageTable[p] >> (page_bits + 1))) {
						physicalPageTable[p] |= 1; //set its "use" bit to 1;
					}	
				}
			}
		}
	printf("Enter a virtual address in hexadecimal (! to quit): ");
	fgets(virtual_address, 22, stdin);
	}
	
	// Free allocated memory
	free(pagetable);
	free(physicalPageTable);
	return 0;
}


