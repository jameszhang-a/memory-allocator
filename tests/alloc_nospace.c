/* allocation is too big to fit in available space */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "mem.h"

int main() {
   assert(Mem_Init(4096,FIRST_FIT) == 0);
   assert(Mem_Alloc(4095) == NULL);

   printf("alloc_noospace.c passes!\n");

   exit(0);
}
