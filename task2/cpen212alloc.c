#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "cpen212alloc.h"
#include <unistd.h> //you need this for linux!

typedef struct alloc_node alloc_node;

typedef struct alloc_node {
    uint64_t size;  // size of the allocated space
} alloc_node; 

typedef struct {
    alloc_node *end;   // pointer to the end of the memory
    alloc_node *free;  // pointer to next free allocated block (points to the start of the header)
    alloc_node *start;  // pointer used for deinitialization to free all memory blocks from the start
    alloc_node *alloc_end;  // pointer to the absolute end of all allocated blocks (points to the header)
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
    s->alloc_end = s->start;

    return s;
}

void cpen212_deinit(void *alloc_state) {
    alloc_state_t *s = (alloc_state_t *) alloc_state;  
    alloc_node *temp = s->start; 
    while (s->start < s->alloc_end) {
        if (s->start->size % 8 == 1) {     
            cpen212_free((alloc_state_t *) s, (alloc_node *) temp);
        }
        s->start = s->start + ((s->start->size) / 8);
    }

    // reset pointers
    s->start = temp;
    s->alloc_end = temp;
    s->free = temp;
}

void *cpen212_alloc(void *alloc_state, size_t nbytes) {
    
    alloc_state_t *s = (alloc_state_t *) alloc_state;  // give s the access of end and free pointers
    uint64_t aligned_sz = (uint64_t) ((nbytes + 7) & ~7) ;  // 8 byte align the aligned size (including the size of the header)
    aligned_sz += 8;
    alloc_node *temp = s->free; 

    // first case: nbyte is 0, alloc_state at the end of list, or no more space avaliable, return immediately
    if (nbytes == 0 || (temp >= s->end) || (temp + (aligned_sz/8) > s->end)) {  // fulfills fucntion specs, double checked
        return NULL;
    }

    // second case: nbytes less or equal to the free space: directly change the "free" pointer after allocating
    if (aligned_sz < s->free->size) {
        // update free 
        uint64_t newsize = s->free->size - aligned_sz;

        s->free = s->free + (aligned_sz / 8);       
        s->free->size = newsize;

        // update new node object  
        temp->size = aligned_sz;  
        temp->size = temp->size + 1;  // change the last bit to 1 to indicate the space is allocated

        if (s->free > s->alloc_end) {
            s->alloc_end = s->free;
        }

    // third case: nbytes equal to free space
    } else if (aligned_sz == s->free->size) {  
        // update free 
        s->free = s->free + (aligned_sz / 8);  

        while (s->free->size % 8 == 1) {  // while allocated == true
            if (s->free == s->end) {
                return NULL;
            }
            if (s->free == s->alloc_end) {
                break;
            }
            s->free = s->free + (s->free->size / 8);  
        }

        /*
        if (s->free == s->alloc_end) {
            s->free->size = (uint64_t) s->end - (uint64_t) s->free;
        }
        */
        
        
        // update new node object  
        temp->size = aligned_sz;
        temp->size = temp->size + 1;

        // update alloc end
        if (s->free > s->alloc_end) {
            s->alloc_end = s->free;
        }
        
    } else if (aligned_sz > s->free->size) {  // loop through the list, s->free stays where it is

        // loop through the list to look for free space and check if its large enough
        while ((temp->size % 8 == 1) || ((temp->size) < aligned_sz)) {

            if ((temp == s->alloc_end)) {
                break;
            }
            if ((temp + (temp->size / 8)) > s->end) {
                return NULL;
            }
            temp = temp + (temp->size / 8);
        }
        
        if (temp->size == aligned_sz) {
            temp->size = aligned_sz;
            temp->size = temp->size + 1;
        } 
        else {
            uint64_t newsize = temp->size - aligned_sz;
            alloc_node *next = temp + (aligned_sz / 8);
            temp->size = aligned_sz;
            temp->size = temp->size + 1;
            next->size = newsize;
        }

        if (temp + (temp->size / 8) > s->alloc_end) {
            s->alloc_end = temp + (temp->size / 8);

            //s->alloc_end->size = (uint64_t) s->end - (uint64_t) s->alloc_end;
        }

    }
    
    printf("---------------------------------------\n");
    printf("s->end %p\n", s->end);
    printf("alloc_end:  %p\n", s->alloc_end);
    printf("s->free %p\n", s->free);
    printf("s->free->size %" PRIu64 "\n", s->free->size);
    printf("allocated at:  %p\n", temp);
    printf("allocated->size %" PRIu64 "\n", temp->size);
    
    if ((temp-1)->size == 2147487746) {
        printf("break here\n");
        sleep(3);
    }

    return (temp + 1);
}

void cpen212_free(void *alloc_state, void *p) { 
    // free the space and point the variable "free" to the start of the free space
    
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    alloc_node *temp = (alloc_node *) p;
    alloc_node *temp_size = temp - 1;

    // ALLOC END UPDATE IS WRONG HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
    for (temp = s->start; temp < s->alloc_end; temp = temp + (temp->size / 8)) {
        s->alloc_end = temp;
    }

    if (s->free > temp_size) {  // adjust the free pointer if the freed space address is smaller than the current free pointer
        s->free = temp_size;
        if ((temp_size->size % 8) == 1) {  // set if allocated to false
            temp_size->size = temp_size->size - 1; 
        }
    } else {  // else, adjust the allocated flag and change it to 0 (meaning the space is free)
        if ((temp_size->size % 8) == 1) {
            temp_size->size = temp_size->size - 1;
        }
    }

    printf("---------------------------------------\n");
    printf("alloc_end:  %p\n", s->alloc_end);
    printf("s->free %p\n", s->free);
    printf("s->free->size %" PRIu64 "\n", s->free->size);
    printf("deallocated at: %p\n", temp_size);
    printf("deallocated->size %" PRIu64 "\n", temp_size->size);
    if (temp_size->size == 2147487746) {
        printf("break here\n");
        sleep(3);
        return;
    }
}

void *cpen212_realloc(void *alloc_state, void *prev, size_t nbytes) {
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    alloc_node *p = cpen212_alloc(s, nbytes);
    alloc_node *prev_size_ptr = prev - 1;
    uint64_t prev_size = prev_size_ptr->size - 1;

    if (p == NULL) {
        return NULL;
    } else if (prev_size <= nbytes) {  // only move prev_size (or else it moves more content)
        uint64_t nbytes_new = prev_size;
        memmove(p, prev, nbytes_new);
    } else {
        memmove(p, prev, nbytes);
    }

    return p; 
}

// WARNING: we don't know the prev block's size, so memmove just copies nbytes here.
//          this is safe only because in this dumb allocator we know that prev < p,
//          and p has at least nbytes usable. in your implementation,
//          you probably need to use the original allocation size.

bool cpen212_check_consistency(void *alloc_state) {
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    alloc_node *temp = s->start; 
    
    // check if the free block exceeds the end of the heap
    if (s->end < s->free) {
        printf("free block exceeds the end of the heap\n");
        return false;
    } 

    // check if allocated block exceeds the end of the heap
    if (s->end < s->alloc_end) {
        printf("allocated block exceeds the end of the heap\n");
        return false;
    }

    // check if unallocated blocks are still allocated
    for (temp = s->alloc_end; temp < s->end; temp = temp + 1) {
        if (temp->size % 8 != 0) {
            printf("unfreed memory\n");
            return false;
        }
    }

    // check if headers have reasonable size
    // 1. it must be above 0 (checked at next test)
    // 2. is either divisible by 8 (it is not allocated) or have remainder of 1 when divided by 8 (it is allocated)
    // 3. it must not exceed the size of the requested heap
    temp = s->start; 
    while (temp < s->alloc_end) {
        if (temp->size % 8 > 1 || temp->size > ((uint64_t) s->end - (uint64_t) s->start)) {
            printf("unreasonable size\n");
            return false;
        }
        temp = temp + (temp->size / 8);
    }

    // check if any allocated blocks overlap 
    temp = s->start; 
    while (temp < s->alloc_end) {
        if (temp->size == 0) {
            printf("overlapped\n");
            return false;
        }
        temp = temp + (temp->size / 8);
    }

    // if all tests passed
    return true;
}


int main() {
    
    alloc_state_t *s = (alloc_state_t *) malloc(128);
    alloc_state_t *p = (alloc_state_t *) cpen212_init(s, (s+4));
    alloc_node *temp = p->start;
    printf("s->start %p\n", p->start);
    printf("s->end %p\n", p->end);
    printf("s->space %" PRIu64 "\n",  (uint64_t) p->end - (uint64_t) p->start);

    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);

    alloc_node *point = (alloc_node *) cpen212_alloc(p, 8);  // allocate 16 bytes
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);

    alloc_node *point1 = (alloc_node *) cpen212_alloc(p, 8);  // allocate 16 bytes
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);



    cpen212_free((alloc_state_t *)p, (alloc_node *) point);
    printf("freed\n");
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);


    alloc_node *point2 = (alloc_node *) cpen212_alloc(p, 16);  // allocate 16 bytes
    printf("s->free %p\n", p->free);
    printf("alloced at: %p\n", point2);
    printf("s->free->size %" PRIu64 "\n", p->free->size);


    cpen212_free((alloc_state_t *)p, (alloc_node *) point1);
    printf("freed\n");
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);

    alloc_node *point3 = (alloc_node *) cpen212_alloc(p, 7);  // allocate 16 bytes
    printf("s->free %p\n", p->free);
    printf("alloced at: %p\n", point3);
    printf("s->free->size %" PRIu64 "\n", p->free->size);

    alloc_node *point4 = (alloc_node *) cpen212_alloc(p, 24);  // allocate 16 bytes
    printf("s->free %p\n", p->free);
    printf("alloced at: %p\n", point4);
    printf("s->free->size %" PRIu64 "\n", p->free->size);

    alloc_node *point5 = (alloc_node *) cpen212_alloc(p, 8);  // allocate 16 bytes
    printf("s->free %p\n", p->free);
    printf("alloced at: %p\n", point5);
    printf("s->free->size %" PRIu64 "\n", p->free->size);

    cpen212_free((alloc_state_t *)p, (alloc_node *) point2);
    printf("freed\n");
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);

    cpen212_free((alloc_state_t *)p, (alloc_node *) point4);
    printf("freed\n");
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);

    printf("-----------------------------------\n");
    for (temp = p->start; temp < p->end; temp = temp + 1) {
        printf("address: %p\n", temp);
        printf("address->size %" PRIu64 "\n", temp->size);
    }
    printf("address: %p\n", p->alloc_end);

    cpen212_check_consistency(p);

}




