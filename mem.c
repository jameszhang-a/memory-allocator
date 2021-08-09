/******************************************************************************
 * FILENAME: mem.c
 * AUTHOR:   cherin@cs.wisc.edu <Cherin Joseph>
 * DATE:     20 Nov 2013
 * EDITED: for CS354 UW Madison Spring 2021 - Michael Doescher
 * PROVIDES: Contains a set of library functions for memory allocation
 * *****************************************************************************/

#include "mem.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// fitting policy
enum POLICY policy;

/**
 ** The BLOCK_HEADER structure serves as the header for each block.
 **
 ** The heaaders we are using for this project are similar to those described
 ** in the book for the implicit free list in section 9.9.6, and in lecture
 ** In the implicit free packs both the size and allocated bit in one int.
 ** The size in the implicit free list includes the size of the header.
 **
 ** In this project we're going to use a struct that tracks additional
 ** information in the block header.
 **
 ** The first piece of information is a 'packed_pointer' that combines the
 ** absolute location (a memory address) of the next header and the alloc bit
 ** The headers must begin on an address divisible by 4. This means the last
 ** two bits must be 0.  We use the least significant bit (LSB) to indicate
 ** if the block is free: LSB = 0; or allocated LSB = 1.
 **
 ** The value stored in the size variable is either the size requested by
 ** the user for allocated blocks, or the available payload size (not including
 ** the size of the header) for free blocks.
 ** This will allow us to calculate the memory utilization.  Memory utilization is
 ** the requested_size / (padding + header_size).
 ** The provided function Mem_Dump takes care of this calculation for us.
 **
 ** The end of the list (the last header) has the packed_pointer  set to NULL,
 ** and the size set to 0.
 */

typedef struct BLOCK_HEADER {
    void *packed_pointer;  // address of the next header + alloc bit.
    unsigned size;
} BLOCK_HEADER;

BLOCK_HEADER *first_header;  // this global variable is a pointer to the first header

// #################################################################################
// ###############               Helper Functions               ####################
// #################################################################################

/**
 ** We recommend you write some helper functions to unpack the headers
 ** and retrieve specific pieces of data. I wrote functions named:
 *  
 **   1) Is_Allocated // return 1 if allocated 0 if not
 **   2) Is_Free      // return 1 if free 0 if not
 **   3) Get_Next_Header // unpacks the header and returns a pointer to the  
 *           the next header, NULL is this is the last BLOCK_HEADER
 **   4) Get_Size 
 **   5) Get_User_Pointer // the pointer that the user can write data to
 **   6) Get_Header_From_User_Pointer // the pointer that the user writes data to - used in Mem_Free
 * TODO  7) Set_Next_Pointer
 **   8) Set_Allocated // set the allocated bit to 1
 **   9) Set_Free // set the allocated bit to 0
 **  10) Set_Size
 */

/**
 * Checks if the header is allocated
 * 
 * @param   p    pointer to a block header
 * @return      1 if allocated, 0 if not
 */
int Is_Allocated(BLOCK_HEADER *p) {
    return (unsigned)p % 2;
}

/**
 * Sets the allocated bit to 1
 * 
 * @param   p    pointer to a block header
 */
void *Set_Allocated(BLOCK_HEADER *p) {
    if (!Is_Allocated(p))
        return (unsigned char *)p + 1;
    else
        printf("Trying to allocate what is already allocated!!!\n\n");
}

/**
 * Checks if the header is free
 * 
 * @param   p    pointer to a block header
 * @return      1 if free, 0 if not
 */
int Is_Free(BLOCK_HEADER *p) { return !Is_Allocated(p); }

/**
 * Sets the allocated bit to 0
 * 
 * @param   p    pointer to a block header
 */
void *Set_Free(BLOCK_HEADER *p) {
    if (Is_Allocated(p))
        return (unsigned char *)p - 1;
    else
        printf("Trying to free what is already free!!!\n\n");
}

/**
 * unpacks the header and returns a pointer to the the next header
 * 
 * @param   cur Current header
 * @return  pointer to the next header, NULL if current is last
 */
BLOCK_HEADER *Get_Next_Header(BLOCK_HEADER *cur) { return (BLOCK_HEADER *)cur->packed_pointer; }

/**
 * Returns size of payload only
 * 
 * @param   p    pointer to a block header
 * @return  size of the block
 */
int Get_Size(BLOCK_HEADER *p) { return p->size; }

/**
 * Returns an address that user can use, given a head address
 * 
 * @param   cur Current header
 * @return  Pointer to memory that user can use
 */
void *Get_User_Pointer(BLOCK_HEADER *cur) { return (unsigned char *)cur + sizeof(BLOCK_HEADER); }

/**
 * Returns an address that to head, given an address the user can use
 * 
 * @param   cur Current memory
 * @return  Pointer to head
 */
void *Get_Header_From_User_Pointer(void *cur) { return (unsigned char *)cur - sizeof(BLOCK_HEADER); }

/**
 * Sets the next pointer of a block
 * 
 * @param   cur Current header
 */
void Set_Next_Pointer(BLOCK_HEADER *cur, BLOCK_HEADER *dst) {
    cur->packed_pointer = dst;
}

/**
 * Sets the size of the block
 * 
 * @param   p       pointer to a block header
 * @param   size    size of the usable memory
 */
void Set_Size(BLOCK_HEADER *p, int size) { p->size = size; }

/**
 * @brief Gives int padding if not divisiable by 4
 * 
 * @param x     requested size
 * @return      Size padded to nearest 4 
 */
int Pad_Size(int x) {
    int mod;

    // Calculates how much til the next 4
    // Adds what's needed
    if ((mod = x % 4)) x += (4 - mod);
    return x;
}

/**
 * @brief Finds the next available block
 * 
 * @return void* pointer to the next free block
 */
void *Get_Next_Free(int size) {
    BLOCK_HEADER *temp = first_header;
    // Searches through all of the blocks
    while (temp->packed_pointer != NULL) {
        // If block already allocated
        if (Is_Allocated(temp)) {
            BLOCK_HEADER *tempAddress = Set_Free(temp);
            temp = Get_Next_Header(tempAddress);
        }

        // If block doesn't have enough size
        else if (Get_Size(temp) < size) {
            temp = Get_Next_Header(temp);
        }

        // Perfect block
        else {
            return temp;
        }
    }

    printf("Can't find free space, something wrong\n");
    return NULL;
}

// #################################################################################
// ###############               Init Function                  ####################
// #################################################################################

/**
 **  Function used to Initialize the memory allocator.
 *!  Do not change this function.
 *  Written by Cherin Joseph, Modified by Michael Doescher.
 *
 *!  Not intended to be called more than once by a program.
 *
 *?  Notes we're using mmap here instead of sbrk as in the book to take advantage of caching
 *?  as described in the OS lectures.
 *
 *  Study the end of the function where the headers are initialized for hints!
 *
 *  @param sizeOfRegion:  Specifies the size of the chunk which needs to be allocated
 *  @param policy_input:  indicates the policy to use eg: best fit is 0
 *  @return            :  0 on success, -1 on failure
 */
int Mem_Init(int sizeOfRegion, enum POLICY policy_input) {
    policy = policy_input;

    int pagesize;
    int padsize;
    int fd;
    int alloc_size;
    void *space_ptr;
    static int allocated_once = 0;

    if (0 != allocated_once) {
        fprintf(stderr, "Error:mem.c: Mem_Init has allocated space during a previous call\n");
        return -1;
    }
    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    /* Calculate padsize as the padding required to round up sizeOfRegion to a multiple of pagesize
     */
    pagesize = getpagesize();  //  Get the pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;
    alloc_size = sizeOfRegion + padsize;

    /* Using mmap to allocate memory */
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    space_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == space_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }

    allocated_once = 1;

    // To begin with, there is only one big, free block.
    // Initialize the first header */
    first_header = (BLOCK_HEADER *)space_ptr;
    // free size
    // Remember that the 'size' stored for free blocks excludes the space for the headers
    first_header->size = (unsigned)alloc_size - 2 * sizeof(BLOCK_HEADER);
    // address of last header
    first_header->packed_pointer = (void *)first_header + alloc_size - sizeof(BLOCK_HEADER);

    // initialize last header
    // packed_pointers are void pointer, the headers are not
    BLOCK_HEADER *last_header = (BLOCK_HEADER *)first_header->packed_pointer;
    last_header->size = 0;
    last_header->packed_pointer = NULL;
    return 0;
}

// #################################################################################
// ###############              Allocate Memory                 ####################
// #################################################################################

/**
 ** Function for allocating 'size' bytes.
 *
 *     Check for sanity of size - Return NULL when appropriate - at least 1 byte. 
 *     Traverse the list of blocks and locate a free block which can accommodate
 *              the requested size based on the policy (e.g. first fit, best fit). 
 * TODO:    The next header must be aligned with an address divisible by 4. 
 *              Add padding to accomodate this requirement. 
 * TODO:    When allocating a block - split it into two blocks when possible. 
 *          ? the allocated block should go first and the free block second. 
 *          ? the free block must have a minimum payload size of 4 bytes.  
 *          ? do not split if the mininmum payload size can not be reserved. 
 * 
 * @param   size    How much free space needed
 * @return  :   the user writeable address of allocated block 
 *                  ! this is the first byte of the payload, not the address of the header
 *              NULL on failure
 */
void *Mem_Alloc(int size) {
    // Checks size is 1 or larger
    if (size < 1) return NULL;

    BLOCK_HEADER *free;

    printf("Head at: %p\n", first_header);

    // Find a suitable block
    if ((free = Get_Next_Free(size)) == NULL) {
        printf("Didn't find free spot\n Do something about it\n");
        return NULL;
    }

    printf("Free block at: %p\tSize is: %i\tPoints t0: %p\n\n", free, free->size, free->packed_pointer);

    // Gets size with padding to %4
    int resize = Pad_Size(size);

    printf("Padded size: %i\n", resize);

    // If there is only size of header left, no split
    if (free->size - sizeof(BLOCK_HEADER) - resize < 4) {
        printf("\nNo need to split\n\n");

        free->packed_pointer = Set_Allocated(free->packed_pointer);
        Set_Size(free, size);
        // return what the user can use
        return Get_User_Pointer(free);
    }

    // Otherwise, we need to split and get a new head
    // Header for splitting the block
    BLOCK_HEADER *next;
    next = Get_User_Pointer(free) + resize;

    printf("Split at: %p\n", next);

    // Update the pointers
    Set_Next_Pointer(next, (BLOCK_HEADER *)free->packed_pointer);
    printf("Next now points to: %p\n", next->packed_pointer);
    Set_Next_Pointer(free, next);
    printf("Free poitns to: %p\n", free->packed_pointer);

    // Update split header
    Set_Size(next, free->size - sizeof(BLOCK_HEADER) - resize);

    // Update old header
    Set_Size(free, size);
    free->packed_pointer = Set_Allocated(free);
}

// #################################################################################
// ###############              Free up Memory                  ####################
// #################################################################################

/**
 ** Function for freeing up a previously allocated block
 * TODO:    Mark the block as free
 * TODO:    Coalesce if one or both of the immediate neighbours are free
 *
 *  @param ptr  :   Address of the block to be freed up i, this is the first address of the payload
 *  @return     :   0 on success
 *                  -1 if ptr is NULL
 *                  -1 if ptr is not pointing to the first byte of an allocated block
 *                ? hint: check all block headers, determine if the alloc bit is set
 */
int Mem_Free(void *ptr) {
    /* Your code should go in here */

    return -1;
}

// #################################################################################
// ###############                 Memory Dump                 #####################
// #################################################################################

/**
 **  Function to be used for debugging.
 *   Prints out a list of all the blocks along with the following information for each block.
 *
 *
 *  @param  No.      : Serial number of the block
 *  @param  Status   : free/busy
 *  @param  Begin    : Address of the first user allocated byte - i.e. start of the payload
 *  @param  End      : Address of the last byte in the block (payload or padding)
 *  @param  Payload  : Payload size of the block - the size requested by the user or free size
 *  @param  Padding  : Padding size of the block
 *  @param  T_Size   : Total size of the block (including the header, payload, and padding)
 *  @param  H_Begin  : Address of the block header
 */
void Mem_Dump() {
    unsigned id = 0;
    unsigned total_free_size = 0;
    unsigned total_payload_size = 0;
    unsigned total_padding_size = 0;
    unsigned total_used_size =
        sizeof(BLOCK_HEADER);  // end of heap header not counted in loop below
    char status[5];
    unsigned payload = 0;
    unsigned padding = 0;
    BLOCK_HEADER *current = first_header;

    fprintf(stdout,
            "************************************Block list***********************************\n");
    fprintf(stdout, "%5s %7s %12s %12s %9s %9s %8s %12s\n", "Id.", "Status", "Begin", "End",
            "Payload", "Padding", "T_Size", "H_Begin");
    fprintf(stdout,
            "---------------------------------------------------------------------------------\n");

    while (current->packed_pointer != NULL) {
        id++;
        BLOCK_HEADER *next = (BLOCK_HEADER *)((unsigned)current->packed_pointer & 0xfffffffe);
        void *begin = (void *)current + sizeof(BLOCK_HEADER);
        void *end = (void *)next - 1;

        if ((unsigned)current->packed_pointer & 1) {  // allocated block
            strcpy(status, "Busy");
            payload = current->size;
            padding =
                (unsigned)((unsigned)next - (unsigned)current) - payload - sizeof(BLOCK_HEADER);
            total_payload_size += payload;
            total_padding_size += padding;
            total_used_size += payload + padding + sizeof(BLOCK_HEADER);
        } else {  // free block
            strcpy(status, "Free");
            payload = current->size;
            padding = 0;
            total_used_size += sizeof(BLOCK_HEADER);
            total_free_size += payload;
        }
        unsigned total_block_size = sizeof(BLOCK_HEADER) + padding + payload;

        fprintf(stdout, "%5d %7s %12p %12p %9u %9u %8u %12p\n", id, status, begin, end, payload,
                padding, total_block_size, current);
        current = next;
    }
    fprintf(stdout,
            "---------------------------------------------------------------------------------\n");
    fprintf(stdout,
            "#################################################################################\n");
    fprintf(stdout, "Total payload size = %d\n", total_payload_size);
    fprintf(stdout, "Total padding size = %d\n", total_padding_size);
    fprintf(stdout, "Total free size = %d\n", total_free_size);
    fprintf(stdout, "Total used size = %d\n", total_used_size);
    fprintf(stdout,
            "#################################################################################\n");
    fflush(stdout);

    return;
}

/**
 * @brief For testing purposes
 * 
 * @return int 
 */
int main() {
    Mem_Init(4096, FIRST_FIT) == 0;
    BLOCK_HEADER *block = first_header;
    printf("First header at: %p\n", block);
    printf("Space: %i\n", block->size);

    printf("Next header is: %p\n", Get_Next_Header(block));

    BLOCK_HEADER *next = Get_Next_Header(block);
    printf("Next next is: %p\n", Get_Next_Header(next));

    void *head_user = Get_User_Pointer(block);
    printf("User starts at: %p\n", head_user);

    void *head_head = Get_Header_From_User_Pointer(head_user);
    printf("head starts at: %p\n", head_head);

    printf("\n\n");

    int *x = Mem_Alloc(3);
}