#include <stdio.h>

typedef struct BLOCK_HEADER {
    void *packed_pointer;  // address of the next header + alloc bit.
    unsigned size;
} BLOCK_HEADER;

int main() {
    BLOCK_HEADER head;
    printf("pointer: %p	, size: %i\n", head.packed_pointer, head.size);
}