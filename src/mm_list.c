#include <mm_list.h>  // prototypes of functions implemented in this file
#include <unistd.h>   // NULL

BlockHeader *mm_list_headp;
BlockHeader *mm_list_tailp;

/**
 * Initializes to an empty list.
 */
void mm_list_init() {
    mm_list_headp = NULL;
    mm_list_tailp = NULL;
}

/**
 * In addition to the block header with size/allocated bit, a free block has
 * pointers to the headers of the previous and next blocks on the free list.
 *
 * Pointers use 4 bytes because this project is compiled with -m32.
 * Check Figure 9.48(b) in the textbook.
 */
typedef struct {
    BlockHeader header;
    BlockHeader *prev_free;
    BlockHeader *next_free;
} FreeBlockHeader;

/**
 * Find the header address of the previous **free** block on the **free list**.
 *
 * @param bp address of a block header (it must be a free block)
 * @return address of the header of the previous free block on the list
 */
BlockHeader *mm_list_prev(BlockHeader *bp) {
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    return fp->prev_free;
}

/**
 * Find the header address of the next **free** block on the **free list**.
 *
 * @param bp address of a block header (it must be a free block)
 * @return address of the header of the next free block on the list
 */
BlockHeader *mm_list_next(BlockHeader *bp) {
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    return fp->next_free;
}

/**
 * Set the pointer to the previous **free** block.
 *
 * @param bp address of a free block header
 * @param prev address of the header of the previous free block (to be set)
 */
static void mm_list_prev_set(BlockHeader *bp, BlockHeader *prev) {
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    fp->prev_free = prev;
}


/**
 * Set the pointer to the next **free** block.
 *
 * @param bp address of a free block header
 * @param next address of the header of the next free block (to be set)
 */
static void mm_list_next_set(BlockHeader *bp, BlockHeader *next) {
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    fp->next_free = next;
}

/**
 * Add a block at the beginning of the free list.
 *
 * @param bp address of the header of the block to add
 */
void mm_list_prepend(BlockHeader *bp) {
    // TODO: implement
    
    if(mm_list_headp == 0){
        mm_list_prev_set(bp, 0);
        mm_list_next_set(bp, 0);
        mm_list_tailp = bp;
        mm_list_headp = bp;
    } else {
        //set bp's prev to null
        mm_list_prev_set(bp, 0);
        //set bp's next to head ptr
        mm_list_next_set(bp, mm_list_headp);
        //set head ptr's prev to bp
        mm_list_prev_set(mm_list_headp, bp);
        //set head ptr to bp
        mm_list_headp = bp;
    }
}

/**
 * Add a block at the end of the free list.
 *
 * @param bp address of the header of the block to add
 */
void mm_list_append(BlockHeader *bp) {
    // TODO: implement
    
    //check, if tail is null then set next and prev to null
    if(mm_list_tailp == 0){
        mm_list_prev_set(bp, 0);
        mm_list_next_set(bp, 0);
        mm_list_tailp = bp;
        mm_list_headp = bp;
    } else {
        //set bp's prev to tail ptr
        mm_list_prev_set(bp, mm_list_tailp);
        //set bp's next to null
        mm_list_next_set(bp, 0);
        //set tail ptr's next to bp
        mm_list_next_set(mm_list_tailp, bp);
        //set tail ptr to bp
        mm_list_tailp = bp;
    }
}

/**
 * Remove a block from the free list.
 *
 * @param bp address of the header of the block to remove
 */
void mm_list_remove(BlockHeader *bp) {
    // TODO: implement

    //current block to remove

    //point prev node's "next" to curr node's next node
    BlockHeader *prev = mm_list_prev(bp);
    BlockHeader *next = mm_list_next(bp);

    //if both dont exist/null, then bp was only block in list, set head and tail to null and return
    if(*prev == 0 && *next == 0)
    {
        //mm_list_init();
        mm_list_headp = 0;
        mm_list_tailp = 0;
    } 
    //if prev doesn't exist, next does, bp is head, set next's prev to null mmlistprevset, set head to bp's next
    else if (*prev == 0 && *next != 0)
    {
        bp = mm_list_headp;
        mm_list_prev_set(next, 0);
        mm_list_headp = next;
    }
    //if next doesn't exist and prev does, bp is tail, 
    //set prev's next to null using mmlistnextset, set tailptr to prev
    else if (*prev != 0 && *next == 0)
    {
        bp = mm_list_tailp;
        mm_list_next_set(prev, 0);
        mm_list_tailp = prev;
    }
    //if we have both prev and next, 
    //somewhere in middle, set next's prev to prev and prev's next to next
    else if (*prev != 0 && *next != 0)
    {
        mm_list_prev_set(next, prev);
        mm_list_next_set(prev, next);
    }
}
