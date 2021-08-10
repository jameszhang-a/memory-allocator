/* a few allocations in multiples of 4 bytes followed by frees */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "mem.h"

int main() {
    assert(Mem_Init(4096, FIRST_FIT) == 0);
    void* ptr[4];

    ptr[0] = Mem_Alloc(4);
    Mem_Dump();
    ptr[1] = Mem_Alloc(8);
    Mem_Dump();
    assert(Mem_Free(ptr[0]) == 0);
    Mem_Dump();
    assert(Mem_Free(ptr[1]) == 0);
    Mem_Dump();

    ptr[2] = Mem_Alloc(16);
    Mem_Dump();
    ptr[3] = Mem_Alloc(4);
    Mem_Dump();
    printf("\n\nMem_Free(ptr[2]): %p\n\n", &ptr[2]);
    assert(Mem_Free(ptr[2]) == 0);
    Mem_Dump();
    printf("\n\nMem_Free(ptr[3]): %p\n\n", &ptr[3]);
    assert(Mem_Free(ptr[3]) == 0);
    Mem_Dump();

    assert(Mem_Free((void*)0x28ff44) == -1);
    Mem_Dump();

    printf("mem_free.c passes!\n");

    exit(0);
}
