/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
        /* Team name */
        "1170300821",
        /* First member's full name */
        "Luo Ruixin",
        /* First member's email address */
        "3102595709@qq.com",
        /* Second member's full name (leave blank if none) */
        "",
        /* Second member's email address (leave blank if none) */
        ""
};

// Align upwards
#define ALIGNMENT 8
#define ALIGN(size) ((((size) + (ALIGNMENT-1)) / (ALIGNMENT)) * (ALIGNMENT))

#define WSIZE  4
#define DSIZE  8
#define TSIZE  12
#define FSIZE  16

//Extend the heap size of the heap each time
#define INITCHUNKSIZE (1<<6)
#define CHUNKSIZE (1<<12)

#define LISTMAX     16

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p)            (*(unsigned int *)(p))
#define PUT(p, val)       (*(unsigned int *)(p) = (val))

#define SET_PTR(p, ptr) (*(unsigned int *)(p) = (unsigned int)(ptr))

#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(ptr) ((char *)(ptr) - WSIZE)
#define FTRP(ptr) ((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE)

#define NEXT_BLKP(ptr) ((char *)(ptr) + GET_SIZE((char *)(ptr) - WSIZE))
#define PREV_BLKP(ptr) ((char *)(ptr) - GET_SIZE((char *)(ptr) - DSIZE))

#define PRED_PTR(ptr) ((char *)(ptr))
#define SUCC_PTR(ptr) ((char *)(ptr) + WSIZE)

#define PRED(ptr) (*(char **)(ptr))
#define SUCC(ptr) (*(char **)(SUCC_PTR(ptr)))

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

void *free_lists[LISTMAX];
static char *heap_listp;

static void *extend_heap(size_t size);//Extended heap
static void *coalesce(void *ptr);//And adjacent Free block
static void *place(void *ptr, size_t size);//Separate the block of size size into the Free list
static void insert_node(void *ptr, size_t size);//Insert the free block pointed to by ptr into the separate free list
static void delete_node(void *ptr);//Remove the block pointed to by ptr from the detached free table
static void printblock(void *bp);//
static void checkblock(void *bp);//

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    int List_number;

    //Initialization separate free list
    for (List_number = 0; List_number < LISTMAX; List_number++)
        free_lists[List_number] = NULL;


    //Initialize heap
    if ((long) (heap_listp = mem_sbrk(FSIZE)) == -1)
        return -1;

    PUT(heap_listp, 0);
    PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + DSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + TSIZE, PACK(0, 1));

    //Extended heap
    if (extend_heap(INITCHUNKSIZE) == NULL)
        return -1;


    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    if (size == 0)
        return NULL;
    //Memory alignment
    if (size <= DSIZE)
        size = FSIZE;
    else
        size = ALIGN(size + DSIZE);


    int list_number = 0;
    size_t search_size = size;
    void *ptr = NULL;

    while (list_number < LISTMAX) {
        //Find the corresponding chain
        if (((search_size <= 1) && (free_lists[list_number] != NULL))) {
            ptr = free_lists[list_number];
            //Look for the right free block in the chain
            while ((ptr != NULL) && ((size > GET_SIZE(HDRP(ptr)))))
                ptr = PRED(ptr);

            //Find the corresponding free block
            if (ptr != NULL)
                break;
        }

        search_size >>= 1;
        list_number++;
    }

    //Did not find a suitable free block, extend the heap
    if (ptr == NULL)
        if ((ptr = extend_heap(MAX(size, CHUNKSIZE))) == NULL)
            return NULL;


    //Block of size size in the free block
    ptr = place(ptr, size);
    return ptr;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    //Insert separate free list
    insert_node(ptr, size);
    //Attention to merger
    coalesce(ptr);

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    void *new_block = ptr;
    int remainder;

    if (size == 0)
        return NULL;

    /* 内存对齐 */
    if (size <= DSIZE)
        size = FSIZE;
    else
        size = ALIGN(size + DSIZE);


    /* If the size is smaller than the size of the original block, return directly to the original block. */
    if ((remainder = GET_SIZE(HDRP(ptr)) - size) >= 0)
        return ptr;

        /* Otherwise, first check if the next block of the address is a free block
        * or the block is the end block of the heap, because we should use the
        * adjacent free block as much as possible to reduce "external fragmentation". */
    else if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) || !GET_SIZE(HDRP(NEXT_BLKP(ptr)))) {
        /* Even adding the free block space on the subsequent consecutive addresses is not enough, expand the block. */
        if ((remainder = GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(NEXT_BLKP(ptr))) - size) < 0) {
            if (extend_heap(MAX(-remainder, CHUNKSIZE)) == NULL)
                return NULL;
            remainder += MAX(-remainder, CHUNKSIZE);
        }

        //Delete the free block just used and set the head and tail of the new block
        delete_node(NEXT_BLKP(ptr));
        PUT(HDRP(ptr), PACK(size + remainder, 1));
        PUT(FTRP(ptr), PACK(size + remainder, 1));
    } else { //There are no continuous free blocks available, and the size is larger than the original block.
        new_block = mm_malloc(size);//apply for a new discontinuous free block
        memcpy(new_block, ptr, GET_SIZE(HDRP(ptr)));//copy the original block content
        mm_free(ptr);//and release the original block.
    }

    return new_block;
}

static void *extend_heap(size_t size) {
    void *ptr;
    //Memory alignment
    size = ALIGN(size);
    //The system calls the "sbrk" extension heap
    if ((ptr = mem_sbrk(size)) == (void *) -1)
        return NULL;

    //Set the head and tail of the free block that just expanded
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    //Note that this block is the end of the heap, so also set the end
    PUT(HDRP(NEXT_BLKP(ptr)), PACK(0, 1));
    //Once set, insert it into the detached free table
    insert_node(ptr, size);
    //In addition, the front of this free block may also be a free block, which may need to be merged.
    return coalesce(ptr);
}

static void insert_node(void *ptr, size_t size) {
    int list_number = 0;
    void *search_ptr = NULL;
    void *insert_ptr = NULL;

    //Find the corresponding chain by the size of the block
    while ((list_number < LISTMAX - 1) && (size > 1)) {
        size >>= 1;
        list_number++;
    }

    /* After finding the corresponding chain, continue to find the corresponding
     * insertion position in the chain, so as to keep the blocks in the chain from small to large. */
    search_ptr = free_lists[list_number];
    while ((search_ptr != NULL) && (size > GET_SIZE(HDRP(search_ptr)))) {
        insert_ptr = search_ptr;
        search_ptr = PRED(search_ptr);
    }

    /* 循环后有四种情况 */
    if (search_ptr != NULL) {
        // 1. ->xx->insert->xx Insert in the middle
        if (insert_ptr != NULL) {
            SET_PTR(PRED_PTR(ptr), search_ptr);
            SET_PTR(SUCC_PTR(search_ptr), ptr);
            SET_PTR(SUCC_PTR(ptr), insert_ptr);
            SET_PTR(PRED_PTR(insert_ptr), ptr);
        } else {//2. [list_number]->insert->xx Insert at the beginning, and there is a previous free block
            SET_PTR(PRED_PTR(ptr), search_ptr);
            SET_PTR(SUCC_PTR(search_ptr), ptr);
            SET_PTR(SUCC_PTR(ptr), NULL);
            free_lists[list_number] = ptr;
        }
    } else {
        if (insert_ptr != NULL) { //3. ->xxxx->insert Insert at the end
            SET_PTR(PRED_PTR(ptr), NULL);
            SET_PTR(SUCC_PTR(ptr), insert_ptr);
            SET_PTR(PRED_PTR(insert_ptr), ptr);
        } else { // 4. [list_number]->insert The chain is empty, this is the first time it is inserted
            SET_PTR(PRED_PTR(ptr), NULL);
            SET_PTR(SUCC_PTR(ptr), NULL);
            free_lists[list_number] = ptr;
        }
    }
}

static void delete_node(void *ptr) {
    int list_number = 0;
    size_t size = GET_SIZE(HDRP(ptr));

    //Find the corresponding chain by the size of the block
    while ((list_number < LISTMAX - 1) && (size > 1)) {
        size >>= 1;
        list_number++;
    }

    // According to the situation of this block, there are four possibilities
    if (PRED(ptr) != NULL) {
        /* 1. xxx-> ptr -> xxx */
        if (SUCC(ptr) != NULL) {
            SET_PTR(SUCC_PTR(PRED(ptr)), SUCC(ptr));
            SET_PTR(PRED_PTR(SUCC(ptr)), PRED(ptr));
        } else {/* 2. [list_number] -> ptr -> xxx */
            SET_PTR(SUCC_PTR(PRED(ptr)), NULL);
            free_lists[list_number] = PRED(ptr);
        }
    } else {/* 3. [list_number] -> xxx -> ptr */
        if (SUCC(ptr) != NULL) {
            SET_PTR(PRED_PTR(SUCC(ptr)), NULL);
        } else {/* 4. [list_number] -> ptr */
            free_lists[list_number] = NULL;
        }
    }
}

static void *coalesce(void *ptr) {
    _Bool prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(ptr)));
    _Bool next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t size = GET_SIZE(HDRP(ptr));
    /*
     * According to the situation that the ptr points to the adjacent blocks
     * before and after the block, it can be divided into four possibilities.
     * Also note that due to our merger and application strategy,
     * it is not possible to have two adjacent free blocks
     */
    //1. Before and after are all allocated blocks, do not merge, return directly
    if (prev_alloc && next_alloc) {
        return ptr;
    } else if (prev_alloc && !next_alloc) { //2. previous block allocated, latter block  free, then two free blocks are merged.
        delete_node(ptr);
        delete_node(NEXT_BLKP(ptr));
        size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        PUT(HDRP(ptr), PACK(size, 0));
        PUT(FTRP(ptr), PACK(size, 0));
    } else if (!prev_alloc && next_alloc) {// 3. latter block allocated,  previous block free, then two free blocks are merged.
        delete_node(ptr);
        delete_node(PREV_BLKP(ptr));
        size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
        PUT(FTRP(ptr), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    } else {// 4. Both blocks are free blocks, and then three blocks are merged at the same time.
        delete_node(ptr);
        delete_node(PREV_BLKP(ptr));
        delete_node(NEXT_BLKP(ptr));
        size += GET_SIZE(HDRP(PREV_BLKP(ptr))) + GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    }

    // Add the merged free block to the free link table
    insert_node(ptr, size);

    return ptr;
}

static void *place(void *ptr, size_t size) {
    size_t ptr_size = GET_SIZE(HDRP(ptr));
    // The size remaining after the allocate size space
    size_t remainder = ptr_size - size;

    delete_node(ptr);

    //If the remaining size is smaller than the minimum block, the original block is not separated
    if (remainder < DSIZE * 2) {
        PUT(HDRP(ptr), PACK(ptr_size, 1));
        PUT(FTRP(ptr), PACK(ptr_size, 1));
    } else if (size >= 96) {
        PUT(HDRP(ptr), PACK(remainder, 0));
        PUT(FTRP(ptr), PACK(remainder, 0));
        PUT(HDRP(NEXT_BLKP(ptr)), PACK(size, 1));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 1));
        insert_node(ptr, remainder);
        return NEXT_BLKP(ptr);
    } else {
        PUT(HDRP(ptr), PACK(size, 1));
        PUT(FTRP(ptr), PACK(size, 1));
        PUT(HDRP(NEXT_BLKP(ptr)), PACK(remainder, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(remainder, 0));
        insert_node(NEXT_BLKP(ptr), remainder);
    }
    return ptr;
}

/*
 * mm_checkheap - Check the heap for consistency
 */
void mm_checkheap(int verbose) {
    char *bp = heap_listp;

    if (verbose)
        printf("Heap (%p):\n", heap_listp);

    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
        printf("Bad prologue header\n");
    checkblock(heap_listp);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (verbose)
            printblock(bp);
        checkblock(bp);
    }

    if (verbose)
        printblock(bp);
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
        printf("Bad epilogue header\n");
}

static void printblock(void *bp) {
    size_t hsize, halloc, fsize, falloc;

    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    if (hsize == 0) {
        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p: header: [%d:%c] footer: [%d:%c]\n", bp,
           hsize, (halloc ? 'a' : 'f'),
           fsize, (falloc ? 'a' : 'f'));
}

static void checkblock(void *bp) {
    if ((size_t) bp % 8)
        printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp)))
        printf("Error: header does not match footer\n");
}
