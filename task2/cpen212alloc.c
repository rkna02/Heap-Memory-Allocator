#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cpen212alloc.h"

typedef struct alloc_node alloc_node;

typedef struct alloc_node {
    bool a;  // true if node is allocated, false otherwise
    size_t size;  // size of the allocated space
} alloc_node; 

typedef struct {
    //alloc_node *start;  // start node of heap
    alloc_node *end;   // end node of heap
    alloc_node *free;  // next free space 
} alloc_state_t;

void *cpen212_init(void *heap_start, void *heap_end) {
    if (heap_start == heap_end) {
        return NULL;
    }
    
    // initialize front and end of list
    alloc_state_t *s = (alloc_state_t *) malloc(sizeof(alloc_state_t));
    //s->start = (alloc_node*) heap_start;
    //s->start->size = (size_t) heap_end - (size_t) heap_start;
    //s->start->a = false;
    s->end = (alloc_node*) heap_end;
    s->end->size = 0;
    s->end->a = true;
    s->free = (alloc_node*) heap_start;
    s->free->size = heap_end - heap_start;
    s->free->a = false;
    
    
    // initialize node content
    //s->start->prev = NULL;
    //s->start->next = s->end;

    //s->end->prev = s->start;
    //s->end->next = NULL;
    //s->end->a = false;
    //s->end->size = 8; 

    return s;
}

void cpen212_deinit(void *alloc_state) {
    alloc_state_t *s = (alloc_state_t *) alloc_state;  
    alloc_node *temp = s->end; 
    while (temp) {
        if (temp->a == true) {
            cpen212_free((alloc_state_t *) s, (alloc_node *) temp);
        }
        temp = temp - temp->size;
    }
}

void *cpen212_alloc(void *alloc_state, size_t nbytes) {
    // third case: nbytes bigger than first free space: have a temp variable that iterates through free space
    alloc_state_t *s = (alloc_state_t *) alloc_state;  // give s the access of start, end, and free
    size_t aligned_sz = (nbytes + 7) & ~7;
    
    alloc_node *temp = s->free; 

    // first case: nbyte is 0, alloc_state at the end of list, or no more space avaliable 
    if (nbytes == 0 || (temp >= s->end) || (temp + aligned_sz > s->end)) {
        return NULL;
    }

    // second case: nbytes less or equal to the free space: directly change the "free" pointer after allocating
    if (aligned_sz < s->free->size) {
        // update free 
        size_t newsize = s->free->size - aligned_sz;
        s->free = s->free + aligned_sz;  
        s->free->size = newsize;

        // update new node object  
        temp->size = aligned_sz;
        temp->a = true;

    } else if (aligned_sz == s->free->size) {  
        // update free 
        s->free = s->free + aligned_sz;  
        while (s->free->a == true) {
            s->free = s->free + s->free->size;
        }

        // update new node object  
        temp->size = aligned_sz;
        temp->a = true;

    } // if aligned_sz > s->free->size

    
    return temp;
}

void cpen212_free(void *alloc_state, void *p) { 
    // free the space and point the variable "free" to the start of the free space
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    alloc_node *temp = (alloc_node *) p;

    if (s->free > temp) {
        s->free = temp;
        s->free = false;
    } else {
        temp->a = false;
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

/*
int main() {
    alloc_state_t *trace = (alloc_state_t *) cpen212_init((alloc_node *) 0, (alloc_node *) 6000);
    alloc_node *node = (alloc_node *) cpen212_alloc(trace, (size_t) 16);

} */
