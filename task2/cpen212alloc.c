#include <stdlib.h>
#include <string.h>
#include "cpen212alloc.h"

typedef struct {
    void *end;   // end of heap
    void *free;  // next free address on heap
} alloc_state_t;

void *cpen212_init(void *heap_start, void *heap_end) {
    alloc_state_t *s = (alloc_state_t *) malloc(sizeof(alloc_state_t));
    s->end = heap_end;
    s->free = heap_start;
    return s;
}

void cpen212_deinit(void *s) {
    free(s);
}

void *cpen212_alloc(void *alloc_state, size_t nbytes) {
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    size_t aligned_sz = (nbytes + 7) & ~7;
    void *p = s->free;
    if ((p >= s->end) || (p + aligned_sz > s->end))
        return NULL;
    s->free += aligned_sz;
    return p;
}

void cpen212_free(void *s, void *p) { }

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
