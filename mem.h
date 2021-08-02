#ifndef __mem_h__
#define __mem_h__

enum POLICY{BEST_FIT, FIRST_FIT, NEXT_FIT, WORST_FIT};

int Mem_Init(int sizeOfRegion, enum POLICY policy_input);
void *Mem_Alloc(int size);
int Mem_Free(void *ptr);
void Mem_Dump();

#endif // __mem_h__


