#ifndef __CRAZYLIST_H__
#define __CRAZYLIST_H__

#include <stdint.h>

typedef struct {
    uint64_t *cdr; // pointer to the next cons cell, or NULL if the list ends at this cell
    uint64_t car;  // <-- cons pointers are (uint64_t *) and point to here
} crazycons_t;

// returns the crazycons_t cell that encloses the given car
crazycons_t *enclosing_struct(uint64_t *car);

// creates a cons cell with the given car and cdr
uint64_t *cons(uint64_t car, uint64_t *cdr);

// returns the car of the cons cell
uint64_t first(uint64_t *list);

// returns the cdr of the cons cell
uint64_t *rest(uint64_t *list);

// finds the first occurrence of query in the list and returns the cons if it exists;
// otherwise returns NULL
uint64_t *find(uint64_t *list, uint64_t query);

// if list is sorted in non-decreasing order, insert n into list so that it remains sorted;
// return the resulting list
uint64_t *insert_sorted(uint64_t *list, uint64_t n);

// print list cars as decimal numbers, separated by space,
// and terminated by the linefeed character
void print_list(uint64_t *list);


#endif // __CRAZYLIST_H__
