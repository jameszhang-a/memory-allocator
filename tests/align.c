/* first pointer returned is 4-byte aligned */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "mem.h"

int main() {
    assert(Mem_Init(4096, FIRST_FIT) == 0);
    int* ptr = (int*)Mem_Alloc(sizeof(int));
    assert(ptr != NULL);
    assert((int)ptr % 4 == 0);

    printf("align.c passes!\n");

    exit(0);
}
