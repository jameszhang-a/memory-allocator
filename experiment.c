#include <stdio.h>
#include <stdlib.h>

typedef struct BLOCK_HEADER {
    void *packed_pointer;  // address of the next header + alloc bit.
    unsigned size;
} BLOCK_HEADER;

BLOCK_HEADER *first_header;  // this global variable is a pointer to the first header

int main() {
    printf("%lu", sizeof(BLOCK_HEADER));
}