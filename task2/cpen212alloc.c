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
    /*
    alloc_state_t *s = (alloc_state_t *) alloc_state;  
    alloc_node *temp = s->end; 
    while (temp) {
        if (temp->a == true) {
            cpen212_free((alloc_state_t *) s, (alloc_node *) temp);
        }
        temp = temp - temp->size;
    }
    */
}

void *cpen212_alloc(void *alloc_state, size_t nbytes) {
    // third case: nbytes bigger than first free space: have a temp variable that iterates through free space
    
    alloc_state_t *s = (alloc_state_t *) alloc_state;  // give s the access of end and free pointers
    uint64_t aligned_sz = (uint64_t) ((nbytes + 7 + 8) & ~7);  // 8 byte align the aligned size (including the size of the header)
    //printf("aligned_sz: %" PRIu64 "\n", aligned_sz);
    alloc_node *temp = s->free; 

    // first case: nbyte is 0, alloc_state at the end of list, or no more space avaliable 
    if (nbytes == 0 || (temp >= s->end) || (temp + (aligned_sz/8) > s->end)) {  // fulfills fucntion specs, double checked
        printf("null\n");
        printf("temp + aligned_sz %p\n", temp + aligned_sz);
        printf("s->end %p\n", s->end);
        return NULL;
    }

    //printf("%" PRIu64 "\n", aligned_sz);
    //printf("%" PRIu64 "\n", s->free->size);

    // second case: nbytes less or equal to the free space: directly change the "free" pointer after allocating
    if (aligned_sz < s->free->size) {
        // update free 
        uint64_t newsize = s->free->size - aligned_sz;
       // printf("new size: %" PRIu64 "\n", newsize);

        s->free = s->free + (aligned_sz / 8);
        //printf("incrementing free pointer: %p\n", s->free);
        
        //s->free->space = s->free + 0x8; 
        s->free->size = newsize;

        // update new node object  
        temp->size = aligned_sz;  
        temp->size = temp->size | 1;  // change the last bit to 1 to indicate the space is allocated

    } else if (aligned_sz == s->free->size) {  
        // update free 
        alloc_node *curr = s->free;
        s->free = s->free + (aligned_sz / 8);  

        while (s->free->size & 1) {  // while allocated == true
            s->free = s->free + s->free->size;  
        }
        //s->free->space = (uint64_t *) (s->free + 0x8);

        // update new node object  
        temp->size = aligned_sz;
        temp->size = temp->size | 1;
        
       // printf("aligned_sz == s->free->size\n");
    } else {  // if aligned_sz > s->free->size
        alloc_node *curr = s->free;
        

        return temp;
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
    alloc_node *point = (alloc_node *) cpen212_alloc(p, 0x8);
    printf("s->free %p\n", p->free);
    alloc_node *point1 = (alloc_node *) cpen212_alloc(p, 0x8);
    printf("s->free %p\n", p->free);
    alloc_node *point2 = (alloc_node *) cpen212_alloc(p, 0x8);
    printf("s->free %p\n", p->free);
    alloc_node *point3 = (alloc_node *) cpen212_alloc(p, 0x8);
    printf("s->free %p\n", p->free);
    //cpen212_free(s, point);
} 
