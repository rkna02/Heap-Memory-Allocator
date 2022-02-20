#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "cpen212alloc.h"

typedef struct alloc_node alloc_node;

typedef struct alloc_node {
    uint64_t size;  // size of the allocated space
} alloc_node; 

typedef struct {
    alloc_node *end;   // pointer to the end of the memory
    alloc_node *free;  // pointer to next free allocated block 
} alloc_state_t;

void *cpen212_init(void *heap_start, void *heap_end) {
    if (heap_start == heap_end) {
        return NULL;
    }

    // initialize front and end of list
    alloc_state_t *s = (alloc_state_t *) malloc(sizeof(alloc_state_t));  // malloc (size), size in bytes

    s->end = (alloc_node*) heap_end;
    s->end->size = (uint64_t) 0;
    s->free = (alloc_node*) heap_start;
    s->free->size = (uint64_t) (heap_end - heap_start);

    printf("s end pointer %p \n", heap_end);
    printf("s free pointer %p \n", s->free);

    return s;
}

void cpen212_deinit(void *alloc_state) {
    
    alloc_state_t *s = (alloc_state_t *) alloc_state;  
    alloc_node *temp = s->end; 
    while (temp) {
        if (temp->size & 1) {
            cpen212_free((alloc_state_t *) s, (alloc_node *) temp);
        }
        temp = temp - (temp->size / 8);
    }
    
}

void *cpen212_alloc(void *alloc_state, size_t nbytes) {
    // third case: nbytes bigger than first free space: have a temp variable that iterates through free space
    
    alloc_state_t *s = (alloc_state_t *) alloc_state;  // give s the access of end and free pointers
    uint64_t aligned_sz = (uint64_t) ((nbytes + 7 + 8) & ~7);  // 8 byte align the aligned size (including the size of the header)
    alloc_node *temp = s->free; 

    // first case: nbyte is 0, alloc_state at the end of list, or no more space avaliable 
    if (nbytes == 0 || (temp >= s->end) || (temp + (aligned_sz/8) > s->end)) {  // fulfills fucntion specs, double checked
        printf("null\n");
        return NULL;
    }

    //printf("%" PRIu64 "\n", aligned_sz);
    //printf("%" PRIu64 "\n", s->free->size);

    // second case: nbytes less or equal to the free space: directly change the "free" pointer after allocating
    if (aligned_sz < s->free->size) {
        printf("first case\n");
        // update free 
        uint64_t newsize = s->free->size - aligned_sz;
        // printf("new size: %" PRIu64 "\n", newsize);

        s->free = s->free + (aligned_sz / 8);       
        s->free->size = newsize;

        // update new node object  
        temp->size = aligned_sz;  
        temp->size = temp->size | 1;  // change the last bit to 1 to indicate the space is allocated

    } else if (aligned_sz == s->free->size) {  
        printf("second case\n");
        // update free 
        s->free = s->free + (aligned_sz / 8);  

        while (s->free->size & 1) {  // while allocated == true
            s->free = s->free + (s->free->size / 8);  
        }

        // update new node object  
        temp->size = aligned_sz;
        temp->size = temp->size | 1;
        
    } else if (aligned_sz > s->free->size) {  // loop through the list, s->free stays where it is
        printf("thrid case\n");
        /*
        alloc_node *curr = s->free;

        // loop through the list to look for free space and check if its large enough
        while ((curr->size & 1) || (curr->size < aligned_sz)) {
            if ((curr + (curr->size / 8)) > s->end) {
                return NULL;
            }
            curr = curr + (curr->size / 8);
        }
        
        // set if allocated to true
        curr->size = aligned_sz;
        curr->size = curr->size | 1;
        */
        return NULL;
    }

    //printf("pointer to the space that just got allocated: %p\n", temp);
    //printf("the updated pointer to free allocated block: %p\n", s->free);
    return temp;
}

void cpen212_free(void *alloc_state, void *p) { 
    // free the space and point the variable "free" to the start of the free space
    
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    alloc_node *temp = (alloc_node *) p;

    if (s->free > temp) {  // adjust the free pointer if the freed space address is smaller than the current free pointer
        s->free = temp;
        printf("s->free->size beforehand %" PRIu64 "\n", s->free->size);
        if ((s->free->size / 8) != 0) {
            s->free->size = s->free->size - 1;  // set if allocated to false
        }
    } else {  // else, adjust the allocated flag and change it to 0 (meaning the space is free)
        temp->size = temp->size & 0;
    }
}

void *cpen212_realloc(void *s, void *prev, size_t nbytes) {
    void *p = cpen212_alloc(s, nbytes);
    if (p != NULL)
        memmove(p, prev, nbytes); // see WARNING below
    return p;
    
}

// WARNING: we don't know the prev block's size, so memmove just copies nbytes here.
//          this is safe only because in this dumb allocator we know that prev < p,
//          and p has at least nbytes usable. in your implementation,
//          you probably need to use the original allocation size.

bool cpen212_check_consistency(void *alloc_state) {
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    return s->end > s->free;
}

int main() {
    alloc_state_t *s = (alloc_state_t *) malloc(64);
    alloc_state_t *p = (alloc_state_t *) cpen212_init(s, (s+0x4));

    printf("s->free %p\n", p->free);
    alloc_node *point = (alloc_node *) cpen212_alloc(p, 0x8);  // allocate 16 bytes
    printf("s->free %p\n", p->free);
    alloc_node *point1 = (alloc_node *) cpen212_alloc(p, 0x8);  // allocate 16 bytes
    printf("s->free %p\n", p->free);

    cpen212_free((alloc_state_t *)p, (alloc_node *) point);
    printf("s->free->size %" PRIu64 "\n", p->free->size);
    alloc_node *point2 = (alloc_node *) cpen212_alloc(p, 0x8);
    printf("s->free %p\n", p->free);
    alloc_node *point3 = (alloc_node *) cpen212_alloc(p, 0x8);
    printf("s->free %p\n", p->free);
} 
