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
    alloc_node *start;  // pointer used for deinitialization to free all memory blocks from the start
} alloc_state_t;

void *cpen212_init(void *heap_start, void *heap_end) {
    if (heap_start == heap_end) {
        return NULL;
    }

    // initialize front and end of list
    alloc_state_t *s = (alloc_state_t *) malloc(sizeof(alloc_state_t));  // malloc (size), size in bytes

    s->end = (alloc_node*) heap_end;
    s->free = (alloc_node*) heap_start;
    s->free->size = (uint64_t) (heap_end - heap_start);
    s->start = (alloc_node *) heap_start;
    s->start->size = s->free->size;
    printf("s end pointer %p \n", heap_end);
    printf("s free pointer %p \n", s->free);
    return s;
}

void cpen212_deinit(void *alloc_state) {
    printf("visit");
    alloc_state_t *s = (alloc_state_t *) alloc_state;  
    alloc_node *temp = s->start; 
    while (temp < s->end) {
        if (temp->size % 8 == 1) {
            cpen212_free((alloc_state_t *) s, (alloc_node *) temp);
        }
        temp = temp + ((temp->size) / 8);
    }
    s->free = s->start;
}

void *cpen212_alloc(void *alloc_state, size_t nbytes) {
    // third case: nbytes bigger than first free space: have a temp variable that iterates through free space
    
    alloc_state_t *s = (alloc_state_t *) alloc_state;  // give s the access of end and free pointers
    uint64_t aligned_sz = (uint64_t) ((nbytes + 7) & ~7) ;  // 8 byte align the aligned size (including the size of the header)
    printf("aligned size: %" PRIu64 "\n", aligned_sz);
    aligned_sz += 8;
    printf("aligned size: %" PRIu64 "\n", aligned_sz);
    alloc_node *temp = s->free; 

    // first case: nbyte is 0, alloc_state at the end of list, or no more space avaliable, return immediately
    if (nbytes == 0 || (temp >= s->end) || (temp + (aligned_sz/8) > s->end)) {  // fulfills fucntion specs, double checked
        printf("null\n");
        return NULL;
    }

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
        printf("temp->size second case %" PRIu64 "\n", temp->size);
        temp->size = temp->size + 1;  // change the last bit to 1 to indicate the space is allocated
        printf("temp->size second case %" PRIu64 "\n", temp->size);

    } else if (aligned_sz == s->free->size) {  
        printf("second case\n");
        // update free 
        s->free = s->free + (aligned_sz / 8);  

        printf("s->free->size second case %" PRIu64 "\n", s->free->size);
        while (s->free->size % 8 == 1) {  // while allocated == true
            s->free = s->free + (s->free->size / 8);  
            printf("loop");
        }

        // update new node object  
        temp->size = aligned_sz;
        temp->size = temp->size + 1;
        
    } else if (aligned_sz > s->free->size) {  // loop through the list, s->free stays where it is
        printf("third case\n");

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
        
    }

    return (temp + 1);
}

void cpen212_free(void *alloc_state, void *p) { 
    // free the space and point the variable "free" to the start of the free space
    
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    alloc_node *temp = (alloc_node *) p;
    alloc_node *temp_size;

    //printf("s->free: %p\n", s->free);
    //printf("temp: %p\n", temp);

    if (s->free > temp) {  // adjust the free pointer if the freed space address is smaller than the current free pointer
        s->free = temp - 1;
        temp_size = s->free - 1;
        printf("s->free->size beforehand %" PRIu64 "\n", temp_size->size);
        printf("allocated %" PRIu64 "\n", (temp_size->size % 8));
        if ((temp_size->size % 8) == 1) {   // set if allocated to false
            printf("loop\n");
            temp_size->size = temp_size->size - 1; 
        }
        printf("temp_size: %" PRIu64 "\n", temp_size->size);
    } else {  // else, adjust the allocated flag and change it to 0 (meaning the space is free)
        temp_size = temp - 1;
        printf("temp_size: %" PRIu64 "\n", temp_size->size);
        if ((temp_size->size % 8) == 1) {
            temp_size->size = temp_size->size - 1;
        }
        printf("temp_size: %" PRIu64 "\n", temp_size->size);
    }

    printf("hey\n");
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

/*
int main() {
    
    alloc_state_t *s = (alloc_state_t *) malloc(100000);
    alloc_state_t *p = (alloc_state_t *) cpen212_init(s, (s+10000));

    printf("s->free %p\n", p->free);
    alloc_node *point = (alloc_node *) cpen212_alloc(p, 2040);  // allocate 16 bytes
    printf("s->free %p\n", p->free);
    alloc_node *point1 = (alloc_node *) cpen212_alloc(p, 4010);  // allocate 16 bytes
    printf("s->free %p\n", p->free);
    alloc_node *point2 = (alloc_node *) cpen212_alloc(p, 48);  // allocate 16 bytes
    printf("s->free %p\n", p->free);
    alloc_node *point3 = (alloc_node *) cpen212_alloc(p, 4072);  // allocate 16 bytes
    printf("s->free %p\n", p->free);
    alloc_node *point4 = (alloc_node *) cpen212_alloc(p, 4072);  // allocate 16 bytes
    printf("s->free %p\n", p->free);
    alloc_node *point5 = (alloc_node *) cpen212_alloc(p, 4072);  // allocate 16 bytes
    printf("s->free %p\n", p->free);
    cpen212_free((alloc_state_t *)p, (alloc_node *) point);
    printf("s->free->size %" PRIu64 "\n", p->free->size);
    printf("s->free %p\n", p->free); 
    printf("heap start %p", s);
    
    cpen212_deinit(p);
} 
*/




