/* This test is broken up into phases.
 * 
 * PHASE 1: In this test, we first use up all the memory of freelist and try to use more
 * memory in order to test that freelist properly becomes null and to show that we won't
 * get and more memory with no errors.
 *
 * 
 * PHASE 2: With the allocated blocks, we put them back into freelist (not in increasing
 * order) to show that freelist will be a linked list where the blocks are stored in
 * increassing address order. Try and also put back something that isn't there. 
 * This will also test that allocated_list will properly be NULL.
 * 
 *
 * PHASE 3: create a new block that splits one of the blocks from freelist up, and put
 * it into allocated_list. This will show that we can correctly split blocks up, and
 * that allocated_list will still work after it has been emptied. Also, create a new
 * block that would take the first block in freelist to test this case.
 * 
 */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "smalloc.h"

#define SIZE 100
#define NUMOFBLOCKS 4


int main(void) {

    mem_init(SIZE);
    
    char *ptrs[NUMOFBLOCKS];
    char *lastptrs[2]; 
    int sizes[NUMOFBLOCKS] = {50, 20, 30, 40};
    int messed_order[NUMOFBLOCKS] = {1,0,3,2};
    int i=0;

    printf("\n***************************************\n");
    printf("*************** PHASE 1 ***************\n");
    printf("***************************************\n\n");

    for (i=0; i < NUMOFBLOCKS; i++) {
       ptrs[i] = smalloc(sizes[i]);
       if (ptrs[i] != NULL) {write_to_mem(sizes[i], ptrs[i], sizes[i]/10);}
    }
    /*ptrs[3] should be null*/
    printf("ptrs[3] is %s, and we want it to be (null)\n", ptrs[3]);

    print_state();

    printf("\n***************************************\n");
    printf("*************** PHASE 2 ***************\n");
    printf("***************************************\n\n");

    /* free all memory; should get result to be 1 for all except for one of them.
     * Mess up the order of blocks going back to freelist to make sure freelist
     * still will go in increasing order*/

    for (i=0; i < NUMOFBLOCKS; i++) {
       printf("freeing %p result = %d\n", ptrs[messed_order[i]], sfree(ptrs[messed_order[i]])) ;
    }     

    print_state();

    printf("\n***************************************\n");
    printf("*************** PHASE 3 ***************\n");
    printf("***************************************\n\n");

    lastptrs[0] = smalloc(30);
    write_to_mem(30, lastptrs[0], 3);
    lastptrs[1] = smalloc(20);
    write_to_mem(20, lastptrs[1], 1);

    print_state();


    mem_clean();
    return 0;
}
