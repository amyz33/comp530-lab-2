/* Tar Heels Allocator
 * 
 * Simple Hoard-style malloc/free implementation.
 * Not suitable for use for large allocatoins, or 
 * in multi-threaded programs.
 * 
 * to use: 
 * $ export LD_PRELOAD=/path/to/th_alloc.so <your command>
 *
 * Name: Amy Zhang 		PID: 720402321		
 *		 Niko Reingold	PID: 720416077
 *		 
 *	Date: 10/28/2016
 *
 *		I certify that no unauthorized assistance has been
 *		received or given in the completion of this work. 
 *
 */

/* Hard-code some system parameters */

#define SUPER_BLOCK_SIZE 4096
#define SUPER_BLOCK_MASK (~(SUPER_BLOCK_SIZE-1))
#define MIN_ALLOC 32 /* Smallest real allocation.  Round smaller mallocs up */
#define MAX_ALLOC 2048 /* Fail if anything bigger is attempted.  
                        * Challenge: handle big allocations */
#define RESERVE_SUPERBLOCK_THRESHOLD 2

#define FREE_POISON 0xab
#define ALLOC_POISON 0xcd

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>

#define assert(cond) if (!(cond)) __asm__ __volatile__ ("int $3")

//log2 function
static inline unsigned int log2(unsigned int x){
        unsigned int ans = 0;
        while(x>>=1) ans++;
        return ans;
}

/* Object: One return from malloc/input to free. */
struct __attribute__((packed)) object {
    union {
        struct object *next; // For free list (when not in use)
        char * raw; // Actual data
    };
};

/* Super block bookeeping; one per superblock.  "steal" the first
 * object to store this structure
 */
struct __attribute__((packed)) superblock_bookkeeping {
    struct superblock_bookkeeping * next; // next super block
    struct object *free_list;
    // Free count in this superblock
    uint8_t free_count; // Max objects per superblock is 128-1, so a byte is sufficient
    uint8_t level;
};

/* Superblock: a chunk of contiguous virtual memory.
 * Subdivide into allocations of same power-of-two size. */
struct __attribute__((packed)) superblock {
    struct superblock_bookkeeping bkeep;
    void *raw;  // Actual data here
};


/* The structure for one pool of superblocks.
 * One of these per power-of-two */
struct superblock_pool {
    struct superblock_bookkeeping *next;
    uint64_t free_objects; // Total number of free objects across all superblocks
    uint64_t whole_superblocks; // Superblocks with all entries free
};

// 10^5 -- 10^11 == 7 levels
#define LEVELS 7
static struct superblock_pool levels[LEVELS] = {{NULL, 0, 0},
                                                {NULL, 0, 0},
                                                {NULL, 0, 0},
                                                {NULL, 0, 0},
                                                {NULL, 0, 0},
                                                {NULL, 0, 0},
                                                {NULL, 0, 0}};

static inline int size2level (ssize_t size) {
    /* Your code here.
     * Convert the size to the correct power of two.
     * Recall that the 0th entry in levels is really 2^5,
     * the second level represents 2^6, etc.
     */
    int level;

    if (size <= MIN_ALLOC){
        //if less than or equal to 32, then it is automatically set to level 0
        level = 0;
    } else {
        //else we calculate the level based on size = 32*2^level
        level = log2((int)(size/32));
        if ((size%32) != 0){
            //if there is a remainder we have to round up a level
            level++;
        }
    }
    return level;
}

static inline
struct superblock_bookkeeping * alloc_super (int power) {

    void *page;
    struct superblock* sb;
    int bytes_per_object = 32*(1<<power);
    int free_objects = (SUPER_BLOCK_SIZE/(bytes_per_object))-1;
    char *cursor;
    // Your code here
    // Allocate a page of anonymous memory
    // WARNING: DO NOT use brk---use mmap, lest you face untold suffering
    page = mmap(NULL,SUPER_BLOCK_SIZE,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);

    sb = (struct superblock*) page;
    // Put this one the list.

    sb->bkeep.next = levels[power].next;
    levels[power].next = &sb->bkeep;
    levels[power].whole_superblocks++;
    sb->bkeep.level = power;
    sb->bkeep.free_list = NULL;

    // Your code here: Calculate and fill the number of free objects in this superblock
    //  Be sure to add this many objects to levels[power]->free_objects, reserving
    //  the first one for the bookkeeping.

    sb->bkeep.free_count = free_objects;
    levels[power].free_objects += free_objects;

    // The following loop populates the free list with some atrocious
    // pointer math.  You should not need to change this, provided that you
    // correctly calculate free_objects.

    cursor = (char *) sb;
    // skip the first object
    for (cursor += bytes_per_object; free_objects--; cursor += bytes_per_object) {
        // Place the object on the free list
        struct object* tmp = (struct object *) cursor;
        tmp->next = sb->bkeep.free_list;
        sb->bkeep.free_list = tmp;
    }
    return &sb->bkeep;
}

void *malloc(size_t size) {
    struct superblock_pool *pool;
    struct superblock_bookkeeping *bkeep;
    void *rv = NULL;
    int power = size2level(size);

    // Check that the allocation isn't too big
    if (size > MAX_ALLOC) {
        errno = -ENOMEM;
        return NULL;
    }
    //check that the allocation is not zero
    if(size == 0){
        return NULL;
    }
    pool = &levels[power];
    if (!pool->free_objects) {
        bkeep = alloc_super(power);
    } else
        bkeep = pool->next;
    while (bkeep != NULL) {
        if (bkeep->free_count) {
            struct object *next = bkeep->free_list;
            /* Remove an object from the free list. */
            // Your code here
            //
            // NB: If you take the first object out of a whole
            //     superblock, decrement levels[power]->whole_superblocks
            rv = next;

            if ((bkeep->free_count + 1) == (SUPER_BLOCK_SIZE/(32*(1<<bkeep->level)))){
                pool->whole_superblocks--;
            }
            bkeep->free_count--;
            bkeep->free_list = next->next;
            pool->free_objects--;
            break;
        }
    }

    // assert that rv doesn't end up being NULL at this point
    assert(rv != NULL);

    /* Exercise 3: Poison a newly allocated object to detect init errors.
     * Hint: use ALLOC_POISON
     */
    memset(rv, ALLOC_POISON, 32*(1<<bkeep->level));
    return rv;
}

static inline
struct superblock_bookkeeping * obj2bkeep (void *ptr) {
    uint64_t addr = (uint64_t) ptr;
    addr &= SUPER_BLOCK_MASK;
    return (struct superblock_bookkeeping *) addr;
}

void free(void *ptr) {

    assert(ptr != NULL);
    struct superblock_bookkeeping *bkeep = obj2bkeep(ptr);

    // Your code here.
    //   Be sure to put this back on the free list, and update the
    //   free count.  If you add the final object back to a superblock,
    //   making all objects free, increment whole_superblocks.

    struct object *current = bkeep->free_list;
    struct superblock_pool *pool;
    struct object *obj;

    pool = &levels[bkeep->level];
    obj = (struct object *) ptr;

    while(current != NULL){
         assert(current != obj);
         if(current->next != NULL){
             current = current->next;
         }
         else{
             break;
         }
    }

    memset((ptr+ sizeof(void*)), FREE_POISON, (32*(1<<bkeep->level)-sizeof(void*)));
    obj->next = bkeep->free_list;
    bkeep->free_list = obj;

    pool->free_objects++;

    bkeep->free_count++;
    if((bkeep->free_count + 1) == (SUPER_BLOCK_SIZE/(32*(1<<bkeep->level)))){
        pool->whole_superblocks++;
    }

    /* Exercise 3: Poison a newly freed object to detect use-after-free errors.
     * Hint: use FREE_POISON.
     */

    struct superblock_bookkeeping *sb = pool->next;
    while (levels[bkeep->level].whole_superblocks > RESERVE_SUPERBLOCK_THRESHOLD) {
        // Exercise 4: Your code here
        // Remove a whole superblock from the level
        // Return that superblock to the OS, using munmap
        if ((sb->free_count + 1) == (SUPER_BLOCK_SIZE/(32*(1<<bkeep->level)))) {
            struct superblock_bookkeeping *toBeRemoved = sb;
            sb = sb->next;

            munmap(toBeRemoved, SUPER_BLOCK_SIZE);
            pool->whole_superblocks--;
            printf("Superblocks %i\n", pool->whole_superblocks);
            break;
        } else {
            sb = sb->next;
        }
    }
}

// Do NOT touch this - this will catch any attempt to load this into a multi-threaded app
int pthread_create(void __attribute__((unused)) *x, ...) {
    exit(-ENOSYS);
}

