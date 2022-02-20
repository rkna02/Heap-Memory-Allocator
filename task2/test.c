#include "cpen212alloc.c" 
#include <stdio.h> 

void main () { 
    printf("sys start\n"); 
    void initHead = malloc(1008); 
    void initEnd = (initHead + 1007); 
    printf("malloc s: %ld, e: %ld\n", (initHead), (initEnd)); 
    void heap=cpen212_init(initHead, initEnd); 
    printf("init complete\n"); 
    printf("\nallocing 4\n"); 
    void store123=cpen212_alloc(heap, 4); 
    ((int) store123)=123; printf("store123 ADDr: %ld\n", (store123)); 
    printf("store123 cont: %ld\n", ((int)store123)); 
    printf("\nallocing 9\n"); 
    void* store9=cpen212_alloc(heap, 9); 
    ((int) store9)=9; 
    printf("store9 ADDr: %ld\n", (store9)); 
    printf("store9 cont: %ld\n", ((int)store9)); 
    printf("store123 cont: %ld\n", ((int)store123)); 
    printf("\nallocing 96\n"); 
    void* store96=cpen212_alloc(heap, 96); 
    ((int) store96)=96; 
    printf("store96 ADDr: %ld\n", (store96)); 
    printf("store96 cont: %ld\n", ((int)store96)); 
    printf("store9 cont: %ld\n", ((int)store9)); 
    printf("store123 cont: %ld\n", ((int)store123)); 
    printf("\n free 9\n"); 
    cpen212_free(heap, store9); 
    printf("123 removed\n"); 
    printf("\nallocing 2\n"); 
    void* store2=cpen212_alloc(heap, 2); 
    ((int) store2)=2; 
    printf("store123 cont: %ld\n", ((int)store123)); 
    printf("store2 ADDr: %ld\n", (store2)); 
    printf("store2 cont: %ld\n", ((int)store2)); 
    printf("store96 cont: %ld\n", ((int)store96)); 
    printf("\n Relocate 101\n"); 
    //void* storeR101= cpen212_realloc(heap, store96, 101); 
    //printf("store101 ADDr: %ld\n", (storeR101)); 
    //printf("store101 cont: %ld\n", ((int)storeR101)); 
    //printf("store96 cont: %ld\n", ((int)store96)); 
    //printf("store9 cont: %ld\n", ((int)store9)); 
    //printf("store123 cont: %ld\n", ((int)store123)); 
    printf("sys end\n"); 
}