#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "smalloc.h"



void *mem;
struct block *freelist;
struct block *allocated_list;


void *smalloc(unsigned int nbytes) {
	/* free_block is the block in freelist that we found to be big enough to allocate
	 * nbytes space
	 * allocate_block is one of the blocks from a split that happens if a free_block has
	 * more than nbytes of space
	 * found_block and first_block are true/false ints
	 */
	struct block *current = freelist, *free_block, *allocate_block;
	int found_block = 0, first_block=0;

	if (nbytes == 0) {return NULL;} /*no need to allocate 0 space */
	if (current == NULL) {return NULL;} /*No blocks left*/

	/*check the first block of freelist*/
	if (current->size >= nbytes) {
		found_block = 1;
		first_block = 1;
		free_block = current;
	}

	/*check the rest of the blocks*/
	while (current->next != NULL && !found_block) {
		if (current->next->size >= nbytes) {
			found_block = 1;
			free_block = current->next;
		}
		else { /*keep looking for an available block*/
			current = current->next;
		}
	}

	if (found_block) {
		/* No need to split a block if we find a block of the exact
		 * size that we need*/
		if (free_block->size == nbytes) {
			if (!first_block) {current->next = free_block->next;}
			else { freelist = free_block->next;}

			free_block->next = allocated_list;
			allocated_list = free_block;
		}
		else {
			/* Handle the block to be allocated over to allocated_list*/
			allocate_block = malloc(sizeof(struct block));
			if (allocate_block == NULL) {perror("out of memory");exit(1);}
			allocate_block->addr = free_block->addr;
			allocate_block->size = nbytes;
			allocate_block->next = allocated_list;
			allocated_list = allocate_block;

			/*handle the block to be broken and change the address values
			 * appropriately.
			 */
			 free_block->size = free_block->size - nbytes;
			 free_block->addr = free_block->addr + nbytes;
		}
		return allocated_list->addr;
	}
	
	/*no block big enough was found!*/
    return NULL;
}



int sfree(void *addr) {
	struct block *current = allocated_list, *free_block;
	int found_block = 0, only_block = 0;

	if (current == NULL) {return -1;}

	/* check if there is only one allocated block and if it has the same
	 * address as *addr. if it is the one, we do not have to link lists
	 * together
	 */
	if (current->next == NULL && current->addr == addr) {
		only_block = 1;
		found_block = 1;
		free_block = current;
	}

	/* Go through the linked list until we reach the end or we
	 * find the block we need to free
	 */
	while (current->next != NULL && !found_block) {
		if (current->next->addr == addr) {
			found_block = 1;
			free_block = current->next;
		}
		else {
			current = current->next;
		}
	}
	if (found_block) {
		if (!only_block) {current->next = free_block->next;}
		else {allocated_list = NULL;}
    	free_block->next = NULL;

		/* set current to loop through freelist to find where to
		 * put the block to be freed since freelist is in increasing addr order.
		 * Since freelist is already in increasing addr order, we just need to 
		 * find the first block that is higher than free_block->addr. First check
		 * if freelist is empty.
		 */
		current = freelist;
		if (current == NULL) {
			freelist = free_block;
			return 1;
		}

		/* check if there is only one block, check if we insert free_block in front.
		 * we check if we insert behind in the following cases.
		 */

		 if (current->next == NULL) {
		 	if (current->addr > free_block->addr) {
		 		free_block->next = current;
		 		freelist = free_block;
		 		return 1;
		 	}
		 }

		 /* Go through freelist until we hit the end or a place to insert free_block*/
		 while (current->next != NULL && current->next->addr < free_block->addr) {
		 	current = current->next;
		 }
		 if (current->next == NULL) {
		 	current->next = free_block;
		 	return 1;
		 }
		 else if (current->next->addr > free_block->addr) {
		 	free_block->next = current->next;
		 	current->next = free_block;
		 	return 1;
		 }
	}
    return -1;
}


/* Initialize the memory space used by smalloc, freelist, and allocated_list */
void mem_init(int size) {
    mem = mmap(NULL, size,  PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if(mem == MAP_FAILED) {
         perror("mmap");
         exit(1);
    }
    /*Initialize freelist; no need to initialize allocated_list since this is done
     *through the process of adding blocks to it*/
    freelist = malloc(sizeof(struct block));
    if (freelist == NULL) {
    	perror("out of memory");
    	exit(1);
    }

    freelist->addr = mem;
    freelist->size = size;
    freelist->next = NULL;
}

/*clean all dynamically allocated memory*/
void mem_clean(){
	clean_list(allocated_list);
	clean_list(freelist);
}

/* free all of the blocks in the linked list */
void clean_list(struct block *list) {
	struct block *temp = list;
	
	if (list == NULL) {free(list);}

	while (list != NULL && list->next != NULL) {
		temp = list;
		list = list->next;
		free(temp);
	}
	/*free the last block*/
	if (list != NULL) {free(list);} 
	list = NULL;
}