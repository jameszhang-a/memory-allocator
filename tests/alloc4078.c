/* an 8 byte allocation */
#include <assert.h>
#include <stdlib.h>
#include "mem.h"

int main() {
   assert(Mem_Init(4096,FIRST_FIT) == 0);
   Mem_Dump();
   void* ptr = Mem_Alloc(4078);
   Mem_Dump();
   assert(ptr != NULL);
   exit(0);
}
