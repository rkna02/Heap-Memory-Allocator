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
    alloc_node *tail;  // pointer to the size at the end of the free pointer
} alloc_state_t;

void cpen212_coalesce(void *alloc_state, void *p);

void *cpen212_init(void *heap_start, void *heap_end) {
    if (heap_start == heap_end) {
        return NULL;
    }

    // initialize front and end of list
    alloc_state_t *s = (alloc_state_t *) malloc(sizeof(alloc_state_t));  // malloc (size), size in bytes

    s->end = (alloc_node*) heap_end;
    s->free = (alloc_node*) heap_start;
    s->free->size = (uint64_t) (heap_end - heap_start);
    s->tail = s->end - 1;
    s->tail->size = s->free->size;
    s->start = (alloc_node *) heap_start;

    return s;
}

void cpen212_deinit(void *s) {
    free(s);
}

void *cpen212_alloc(void *alloc_state, size_t nbytes) {
    
    alloc_state_t *s = (alloc_state_t *) alloc_state;  // give s the access of end and free pointers
    uint64_t aligned_sz = (uint64_t) ((nbytes + 7) & ~7) ;  // 8 byte align the aligned size (including the size of the header)
    aligned_sz += 16;
    alloc_node *temp = s->free; 
    alloc_node *temp1 = s->free;

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
        s->tail = s->free + (s->free->size / 8) - 1;
        s->tail->size = s->free->size;

        // update new node object  
        temp->size = aligned_sz;  
        temp->size = temp->size + 1;  // change the last bit to 1 to indicate the space is allocated

        temp1 = temp + (temp->size / 8) - 1;
        temp1->size = aligned_sz + 1;

    // third case: nbytes equal to free space
    } else if (aligned_sz == s->free->size) {  
        // update free 
        s->free = s->free + (aligned_sz / 8);  
        s->tail = s->free + (s->free->size / 8) - 1;

        while (s->free->size % 8 == 1) {  // while allocated == true
            if (s->free == s->end) {
                s->tail = s->free;
                break;
            }
            s->free = s->free + (s->free->size / 8);  
            s->tail = s->free + (s->free->size / 8) - 1;
        }       
        
        // update new node object  
        temp->size = aligned_sz;
        temp->size = temp->size + 1;

        temp1 = temp + (temp->size / 8) - 1;
        temp1->size = aligned_sz + 1;
        
    } else if (aligned_sz > s->free->size) {  // loop through the list, s->free stays where it is

        // loop through the list to look for free space and check if its large enough
        while ((temp->size % 8 == 1) || ((temp->size) < aligned_sz)) {

            if ((temp + (temp->size / 8)) > s->end) {
                return NULL;
            }
            temp = temp + (temp->size / 8);
        }
        
        if (temp->size == aligned_sz) {
            temp->size = aligned_sz;
            temp->size = temp->size + 1;
            temp1 = temp + (temp->size / 8) - 1;
            temp1->size = aligned_sz + 1;
        } 
        else {
            uint64_t newsize = temp->size - aligned_sz;  // new size
            alloc_node *next = temp + (aligned_sz / 8);
            temp->size = aligned_sz;
            temp->size = temp->size + 1;
            temp1 = temp + (temp->size / 8) - 1;
            temp1->size = aligned_sz + 1;
            next->size = newsize;
            next = next + (next->size / 8) - 1;
            next->size = newsize;
        }

    }
    
  
    printf("---------------------------------------\n");
    printf("s->free %p\n", s->free);
    printf("s->free->size %" PRIu64 "\n", s->free->size);
    printf("allocated at:  %p\n", temp);
    printf("allocated->size %" PRIu64 "\n", temp->size);
   
    return (temp + 1);
}

void cpen212_free(void *alloc_state, void *p) { 
    // free the space and point the variable "free" to the start of the free space
    
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    alloc_node *temp = (alloc_node *) p;
    alloc_node *temp_size = temp - 1;
    alloc_node *temp_size1;
    printf("---------------------------------------\n");
    printf("temp_size: %p\n", temp_size);

    if (s->free > temp_size) {  // adjust the free pointer if the freed space address is smaller than the current free pointer
        s->free = temp_size;
        s->tail = s->free + (s->free->size / 8) - 1;
        if ((temp_size->size % 8) == 1) {  // set if allocated to false
            temp_size->size = temp_size->size - 1; 
            s->tail->size = s->tail->size - 1;
        }
    } else {  // else, adjust the allocated flag and change it to 0 (meaning the space is free)
        if ((temp_size->size % 8) == 1) {
            temp_size->size = temp_size->size - 1;
            temp_size1 = temp_size + (temp_size->size / 8) - 1;
            temp_size1->size = temp_size->size;
        }
    }

    cpen212_coalesce(s, temp);

    printf("s->free %p\n", s->free);
    printf("s->free->size %" PRIu64 "\n", s->free->size);
    printf("deallocated at: %p\n", temp_size);
    printf("deallocated->size %" PRIu64 "\n", temp_size->size);
    
}

void cpen212_coalesce(void *alloc_state, void *p) {
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    alloc_node *temp = (alloc_node *) p;
    alloc_node *temp_size = temp - 1;
    alloc_node *prev = temp_size - 1;
    alloc_node *next = temp_size + (temp_size->size / 8);
    alloc_node *newheader; 
    alloc_node *newtail;
    bool visited = false;

    if (temp - 1 == s->start) {
        if (next->size % 8 == 0) {
            uint64_t newsize = next->size + temp_size->size;  // calculate size 
            // new tail
            newtail = next + (next->size / 8) - 1;
            newtail->size = newsize;
            next->size = 0;  // remove size in middle
            (next - 1)->size = 0;  // remove size in middle
            // new header
            newheader = temp_size;
            newheader->size = newsize;
        }
        return;
    }


    // check the previous block first 
    if (prev->size % 8 == 0) {
        uint64_t newsize = prev->size + temp_size->size;  // calculate size 
        // new tail
        newtail = temp_size + (temp_size->size / 8) - 1;
        newtail->size = newsize;
        temp_size->size = 0;  // remove size in middle
        // new header
        newheader = prev - (prev->size / 8) + 1;
        newheader->size = newsize;
        prev->size = 0;  // remove size in middle
        visited = true;

    }

    // check the following block if it is allocated
    if (next->size % 8 == 0 && visited) {
        uint64_t newsize = next->size + newheader->size;  // calculate size 
        // new tail
        newtail = next + (next->size / 8) - 1;
        newtail->size = newsize;
        next->size = 0;  // remove size in middle
        (next - 1)->size = 0; // remove size in middle
        // new header size 
        newheader->size = newsize;
    } else if (next->size % 8 == 0 && !visited) {
        uint64_t newsize = next->size + temp_size->size;  // calculate size 
        // new tail
        newtail = next + (next->size / 8) - 1;
        newtail->size = newsize;
        next->size = 0;  // remove size in middle
        (next - 1)->size = 0;  // remove size in middle
        // new header
        newheader = temp_size;
        newheader->size = newsize;

    }

    if (s->free > newheader) {
        s->free = newheader;
        s->tail = s->free + (s->free->size / 8) - 1;
    }

}

void *cpen212_realloc(void *alloc_state, void *prev, size_t nbytes) {
    if (nbytes == 0) {  // base case, if nbytes == NULL
        return NULL;
    }

    alloc_state_t *s = (alloc_state_t *) alloc_state;
    alloc_node *p = cpen212_alloc(s, nbytes);
    alloc_node *prev_size_ptr = prev - 1;  // pointer that points to the size of the block that we want to reallocate
    uint64_t prev_size = prev_size_ptr->size - 1;  // ignore the indication for allocation

    if (prev_size == (uint64_t) nbytes) {
        return prev;
    } else if (prev_size < (uint64_t) nbytes) {  // only move prev_size (or else it moves more content)
        memmove(p, prev, nbytes);
    } else {
        memmove(p, prev, prev_size);
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

    // check if free pointer points to allocated content
    if (s->free->size % 8 != 0) {
        printf("free pointer points to allocated content");
        return false;
    }

    // check if headers have reasonable size and have overlapping
    // 1. it must be above 0 (checked at next test)
    // 2. is either divisible by 8 (it is not allocated) or have remainder of 1 when divided by 8 (it is allocated)
    // 3. it must not exceed the size of the requested heap
    temp = s->start; 
    while (temp < s->end) {
        if (temp->size == 0 || temp->size % 8 > 1 || temp->size > ((uint64_t) s->end - (uint64_t) s->start)) {
            printf("unreasonable size/overlapped\n");
            return false;
        }
        temp = temp + (temp->size / 8);
    }

    // if all tests passed
    return true;
}


/*
int main() {
    
    alloc_state_t *s = (alloc_state_t *) malloc(20000);
    alloc_state_t *p = (alloc_state_t *) cpen212_init(s, (s+10));
    alloc_node *temp = p->start;
    printf("s->start %p\n", p->start);
    printf("s->end %p\n", p->end);
    printf("s->space %" PRIu64 "\n",  (uint64_t) p->end - (uint64_t) p->start);
    printf("----------------------------\n");
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);
    printf("s->tail %p\n", p->tail);
    printf("s->tail->size %" PRIu64 "\n", p->tail->size);

    alloc_node *point = (alloc_node *) cpen212_alloc(p, 24);  // allocate 24 bytes
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);
    printf("s->tail %p\n", p->tail);
    printf("s->tail->size %" PRIu64 "\n", p->tail->size);

    alloc_node *point1 = (alloc_node *) cpen212_alloc(p, 42);  // allocate 24 bytes
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);
    printf("s->tail %p\n", p->tail);
    printf("s->tail->size %" PRIu64 "\n", p->tail->size);

    alloc_node *point2 = (alloc_node *) cpen212_alloc(p, 8);  // allocate 24 bytes
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);
    printf("s->tail %p\n", p->tail);
    printf("s->tail->size %" PRIu64 "\n", p->tail->size);

    alloc_node *point3 = (alloc_node *) cpen212_alloc(p, 16);  // allocate 24 bytes
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);
    printf("s->tail %p\n", p->tail);
    printf("s->tail->size %" PRIu64 "\n", p->tail->size);

    alloc_node *point4 = (alloc_node *) cpen212_alloc(p, 16);  // allocate 24 bytes
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);
    printf("s->tail %p\n", p->tail);
    printf("s->tail->size %" PRIu64 "\n", p->tail->size);

    printf("-------------------------------\n");
    for (temp = p->start; temp < p->end; temp = temp + 1) {
        printf("address: %p\n", temp);
        printf("address->size %" PRIu64 "\n", temp->size);
    }

    /*
    cpen212_free((alloc_state_t *)p, (alloc_node *) point);
    printf("freed\n");
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);
    printf("s->tail %p\n", p->tail);
    printf("s->tail->size %" PRIu64 "\n", p->tail->size);

    printf("-------------------------------\n");
    for (temp = p->start; temp < p->end; temp = temp + 1) {
        printf("address: %p\n", temp);
        printf("address->size %" PRIu64 "\n", temp->size);
    }

    cpen212_free((alloc_state_t *)p, (alloc_node *) point3);
    printf("freed\n");
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);
   
    printf("-------------------------------\n");
    for (temp = p->start; temp < p->end; temp = temp + 1) {
        printf("address: %p\n", temp);
        printf("address->size %" PRIu64 "\n", temp->size);
    }

    cpen212_free((alloc_state_t *)p, (alloc_node *) point2);
    printf("freed\n");
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);

    printf("-------------------------------\n");
    for (temp = p->start; temp < p->end; temp = temp + 1) {
        printf("address: %p\n", temp);
        printf("address->size %" PRIu64 "\n", temp->size);
    }

    cpen212_free((alloc_state_t *)p, (alloc_node *) point1);
    printf("freed\n");
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);

    printf("-------------------------------\n");
    for (temp = p->start; temp < p->end; temp = temp + 1) {
        printf("address: %p\n", temp);
        printf("address->size %" PRIu64 "\n", temp->size);
    }

    cpen212_free((alloc_state_t *)p, (alloc_node *) point4);
    printf("freed\n");
    printf("s->free %p\n", p->free);
    printf("s->free->size %" PRIu64 "\n", p->free->size);

    printf("-------------------------------\n");
    for (temp = p->start; temp < p->end; temp = temp + 1) {
        printf("address: %p\n", temp);
        printf("address->size %" PRIu64 "\n", temp->size);
    }
    
}
*/




