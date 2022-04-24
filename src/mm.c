#include "mm.h"        // prototypes of functions implemented in this file
#include "mm_list.h"   // "mm_list_..."  functions -- to manage explicit free list
#include "mm_block.h"  // "mm_block_..." functions -- to manage blocks on the heap
#include "memlib.h"    // mem_sbrk -- to extend the heap
#include <string.h>    // memcpy -- to copy regions of memory

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) > (y) ? (y) : (x))

/**
 * Mark a block as free, coalesce with contiguous free blocks on the heap, add
 * the coalesced block to the free list.
 *
 * @param bp address of the block to mark as free
 * @return the address of the coalesced block
 */
static BlockHeader *free_coalesce(BlockHeader *bp) {

    // mark block as free
    int size = mm_block_size(bp);
    mm_block_set_header(bp, size, 0);
    mm_block_set_footer(bp, size, 0);

    // check whether contiguous blocks are allocated
    int prev_alloc = mm_block_allocated(mm_block_prev(bp));
    int next_alloc = mm_block_allocated(mm_block_next(bp));

    if (prev_alloc && next_alloc) { //alloc-alloc
        // TODO: add bp to free list
        mm_list_append(bp);
        return bp;
    } else if (prev_alloc && !next_alloc) { //alloc-free
        // TODO: remove next block from free list
        mm_list_remove(mm_block_next(bp));
        // TODO: add bp to free list
        // TODO: coalesce with next block
        size += mm_block_size(mm_block_next(bp));
        mm_block_set_header(bp, size, 0);
        mm_block_set_footer(bp, size, 0);
        mm_list_append(bp);
        return bp;
    } else if (!prev_alloc && next_alloc) { //free-alloc
        // TODO: coalesce with previous block
        size += mm_block_size(mm_block_prev(bp));
        mm_block_set_header(mm_block_prev(bp), size, 0);
        mm_block_set_footer(mm_block_prev(bp), size, 0);
        return mm_block_prev(bp);

    } else { //free-free
        BlockHeader *prev_free = mm_block_prev(bp);
        BlockHeader *next_free = mm_block_next(bp);
        // TODO: remove next block from free list
        mm_list_remove(next_free);
        // TODO: coalesce with previous and next block
        size += mm_block_size(prev_free);
        size += mm_block_size(next_free);
        mm_block_set_header(prev_free, size, 0);
        mm_block_set_footer(prev_free, size, 0);
        return mm_block_prev(bp);
    }
}

/**
 * Allocate a free block of `size` byte (multiple of 8) on the heap.
 *
 * @param size number of bytes to allocate (a multiple of 8)
 * @return pointer to the header of the allocated block
 */
static BlockHeader *extend_heap(int size) {

    // bp points to the beginning of the new block
    char *bp = mem_sbrk(size);
    if ((long)bp == -1)
        return NULL;

    // write header over old epilogue, then the footer
    BlockHeader *old_epilogue = (BlockHeader *)bp - 1;
    mm_block_set_header(old_epilogue, size, 0);
    mm_block_set_footer(old_epilogue, size, 0);

    // write new epilogue
    mm_block_set_header(mm_block_next(old_epilogue), 0, 1);

    // merge new block with previous one if possible
    return free_coalesce(old_epilogue);
}

int mm_init(void) {

    // init list of free blocks
    mm_list_init();

    // create empty heap of 4 x 4-byte words
    char *new_region = mem_sbrk(16);
    if ((long)new_region == -1)
        return -1;

    heap_blocks = (BlockHeader *)new_region;
    mm_block_set_header(heap_blocks, 0, 0);      // skip 4 bytes for alignment
    mm_block_set_header(heap_blocks + 1, 8, 1);  // allocate a block of 8 bytes as prologue
    mm_block_set_footer(heap_blocks + 1, 8, 1);
    mm_block_set_header(heap_blocks + 3, 0, 1);  // epilogue (size 0, allocated)
    heap_blocks += 1;                            // point to the prologue header

    // TODO: extend heap with an initial heap size
    if (extend_heap(5000) == NULL) return -1;
    return 0;
}

void mm_free(void *bp) {
    // TODO: move back 4 bytes to find the block header, then free block
    if(bp == 0) return;

    bp = bp - 4;

    free_coalesce(bp);
}

/**
 * Find a free block with size greater or equal to `size`.
 *
 * @param size minimum size of the free block
 * @return pointer to the header of a free block or `NULL` if free blocks are
 *         all smaller than `size`.
 */
static BlockHeader *find_fit(int size) {
    // TODO: implement
    //if item is less than some threshold, put at front of free space
    //if larger, put at back
    if(size > 1024){
        BlockHeader *tmp = mm_list_headp;
        while(tmp != 0){
            if (mm_block_size(tmp) >= size){
                return tmp;
            }
            tmp = mm_list_next(tmp);
        }
    } else {
        BlockHeader *tmp = mm_list_tailp;
        while(tmp != 0){
            if (mm_block_size(tmp) >= size){
                return tmp;
            }
            tmp = mm_list_prev(tmp);
        }
    }
    return NULL;
}

/**
 * Allocate a block of `size` bytes inside the given free block `bp`.
 *
 * @param bp pointer to the header of a free block of at least `size` bytes
 * @param size bytes to assign as an allocated block (multiple of 8)
 * @return pointer to the header of the allocated block
 */
static BlockHeader *place(BlockHeader *bp, int size) {
    // TODO: if current size is greater, use part and add rest to free list
    // TODO: return pointer to header of allocated block
    int bp_size = mm_block_size(bp);

    int rest = bp_size - size;

    mm_list_remove(bp);

    //threshold, 1024 or 512
    if(rest > 512){
        if(rest > 1024){
            mm_block_set_header(bp, rest, 0);
            mm_block_set_footer(bp, rest, 0);

            mm_list_append(bp);

            mm_block_set_header(mm_block_next(bp), size, 1);
            mm_block_set_footer(mm_block_next(bp), size, 1);

            return mm_block_next(bp);
        } else {
            mm_block_set_header(bp, size, 1);
            mm_block_set_footer(bp, size, 1);

            mm_block_set_header(mm_block_next(bp), rest, 0);
            mm_block_set_footer(mm_block_next(bp), rest, 0);

            free_coalesce(mm_block_next(bp));

            return bp;
        }
    } else {
        mm_block_set_header(bp, bp_size, 1);
        mm_block_set_footer(bp, bp_size, 1);
    }
    return bp;
}

/**
 * Compute the required block size (including space for header/footer) from the
 * requested payload size.
 *
 * @param payload_size requested payload size
 * @return a block size including header/footer that is a multiple of 8
 */
static int required_block_size(int payload_size) {
    payload_size += 8;                    // add 8 for for header/footer
    return ((payload_size + 7) / 8) * 8;  // round up to multiple of 8
}

void *mm_malloc(size_t size) {
    // ignore spurious requests
    if (size == 0)
        return NULL;


    int required_size = required_block_size(size);
    
    // TODO: find a free block or extend heap
    // TODO: allocate and return pointer to payload
    int block_extension = MAX(4096, required_size);
    BlockHeader *tmp = find_fit(required_size);

    if(tmp != NULL){
        return place(tmp, required_size) + 1;
    } else {
        tmp = extend_heap(block_extension);
        return place(tmp, required_size) + 1;
    }

    return NULL;
}

void *mm_realloc(void *ptr, size_t size) {

    if (ptr == NULL) {
        // equivalent to malloc
        return mm_malloc(size);

    } else if (size == 0) {
        // equivalent to free
        mm_free(ptr);
        return NULL;

    } else {
        // TODO: reallocate, reusing current block if possible
        // TODO: copy data over new block with memcpy
        // TODO: return pointer to payload of new block

        // TODO: remove this naive implementation
        /*void *new_ptr = mm_malloc(size);
        memcpy(new_ptr, ptr, MIN(size, (unsigned)mm_block_size(ptr-4) - 8));
        mm_free(ptr);
        return new_ptr;*/

        BlockHeader *ptr_block = (BlockHeader *)ptr - 1;
        BlockHeader *next_block = mm_block_next(ptr_block);

        int ptr_block_size = mm_block_size(ptr_block);
        int next_size = mm_block_size(next_block);
        int required_size = required_block_size(size);
        int new_size = ptr_block_size + next_size;

        if(ptr_block_size >= required_size){
            return ptr;
        }

        if((next_block != 0) && (new_size >= required_size) && (!mm_block_allocated(next_block))){
            mm_list_remove(next_block);

            mm_block_set_header(ptr_block, new_size, 1);
            mm_block_set_footer(ptr_block, new_size, 1);

            return ptr;
        }

        //for copy
        int num_bytes = required_size - 8;

        ptr_block = mm_malloc(required_size);
        memcpy(ptr_block, ptr, num_bytes);
        mm_free(ptr);

        return ptr_block;
    }
}