#include <stdio.h>
#include <stdlib.h>

typedef struct BLOCK_HEADER {
    void *packed_pointer;  // address of the next header + alloc bit.
    unsigned size;
} BLOCK_HEADER;

BLOCK_HEADER *first_header;  // this global variable is a pointer to the first header

int main() {
    BLOCK_HEADER *head;
    head->packed_pointer = malloc(sizeof(BLOCK_HEADER));
    printf("pointer: %p	, size: %i\n", head->packed_pointer, head->size);
    
}