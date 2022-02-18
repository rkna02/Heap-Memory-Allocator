#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "crazylist.h"

crazycons_t *enclosing_struct(uint64_t *car) {
    return (crazycons_t *) ((void *) car - offsetof(crazycons_t, car));
}

uint64_t *cons(uint64_t car, uint64_t *cdr) {
    crazycons_t *cons = (crazycons_t *) malloc(sizeof(crazycons_t)); // malloc: allocates the input amount of memory and returns a pointer to it
    assert(cons);  // assert: if cons is not NULL, start constructing 
    cons->car = car;
    cons->cdr = cdr; 
    assert(cons);  
    return (uint64_t *) &cons->car;  // returns pointer to cons' car (value) NOT POINTER TO CONS
}

uint64_t first(uint64_t *list) {
    if (!list) {
        return NULL;
    }

    return *list;
}

uint64_t *rest(uint64_t *list) {
    if (!list) {
        return NULL;
    }

    return enclosing_struct(list)->cdr;
}

uint64_t *find(uint64_t *list, uint64_t query) {
    if (!list) {
        return NULL;
    }

    if (*list == query) {
        return list;
    }

    find(enclosing_struct(list)->cdr, query);
}

uint64_t *insert_sorted(uint64_t *list, uint64_t n) {

    // case 1: if list is null
    if (!list) {
        return cons(n, NULL);
    }

    uint64_t *curr = list;
    uint64_t *prev = NULL;

    // case 2: only 1 element in list
    if (!enclosing_struct(list)->cdr) {
        if (*list <= n) {
            uint64_t *temp = cons(n, NULL); 
            enclosing_struct(list)->cdr = temp;
            return list;
        } else {
            uint64_t *temp = cons(n, list); 
            return temp;
        }
    }

    // case 3: otherwise
    // find the pointer that points to the spot n should go to 
    while (*curr <= n && enclosing_struct(curr)->cdr) {
        prev = curr;
        curr = enclosing_struct(curr)->cdr; 
    }

    // check if n would be placed in the very end of the list
    uint64_t *temp;
    if (*curr < n) {
        temp = cons(n, NULL); 
        enclosing_struct(curr)->cdr = temp;
    } else {
        temp = cons(n, curr); 
        enclosing_struct(prev)->cdr = temp;
    } 
    
    return list;
}


void print_list(uint64_t *list) {
    if (!list) {
        return;
    }

    while (list) {
        printf("%" PRIu64 " ", *list);
        list = enclosing_struct(list)->cdr;
    }
    printf("\n");
}


int main() {
    uint64_t *list = cons(100, NULL);
    list = cons(30, list);
    list = cons(10, list);
    list = cons(5, list);
    print_list(list); // output: 5 10 30 100
    list = insert_sorted(list, 50);
    print_list(list); // output: 5 10 30 50 100
    *find(list, 10) = 20;
    print_list(list); // output: 5 20 30 50 100

    uint64_t *list1 = NULL;
    list1 = insert_sorted(list1, 50);
    print_list(list1); // output: 5 10 30 50 100

    uint64_t *list2 = cons(100, NULL);
    print_list(list2); // output: 5 10 30 100
    list2 = insert_sorted(list2, 50);
    print_list(list2); // output: 5 10 30 50 100

    uint64_t *list3 = cons(100, NULL);
    print_list(list3); // output: 5 10 30 100
    list3 = insert_sorted(list3, 150);
    print_list(list3); // output: 5 10 30 50 100
    *find(list3, 150) = 20;
    print_list(list3); // output: 5 20 30 50 100

    uint64_t *list4 = cons(100, NULL);
    list4 = cons(30, list4);
    list4 = cons(10, list4);
    list4 = cons(5, list4);
    print_list(list4); // output: 5 10 30 100
    list4 = insert_sorted(list4, 150);
    print_list(list4); // output: 5 10 30 50 100
    *find(list4, 30) = 20;
    print_list(list4); // output: 5 20 30 50 100

}
