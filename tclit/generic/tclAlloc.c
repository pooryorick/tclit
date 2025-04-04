/*
 * Copyright © 1983 Regents of the University of California.
 * Copyright © 1996-1997 Sun Microsystems, Inc.
 * Copyright © 1998-1999 Scriptics Corporation.
 *
 * Portions contributed by Chris Kingsley, Jack Jansen and Ray Johnson.
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

/*
 * You may distribute and/or modify this program under the terms of the GNU
 * Affero General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.

 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

/*
 * tclAlloc.c --
 *
 *	This is a very fast storage allocator. It allocates blocks of a small
 *	number of different sizes, and keeps free lists of each size. Blocks
 *	that don't exactly fit are passed up to the next larger size. Blocks
 *	over a certain size are directly allocated from the system.
 *
*/

/*
 * Windows and Unix use an alternative allocator when building with threads
 * that has significantly reduced lock contention.
 */

#include "tclInt.h"
#if !TCL_THREADS || !defined(USE_THREAD_ALLOC)

#if defined(USE_TCLALLOC) && USE_TCLALLOC

/*
 * We should really make use of AC_CHECK_TYPE(caddr_t) here, but it can wait
 * until Tcl uses config.h properly.
 */

#if defined(_MSC_VER) || defined(__MSVCRT__)
typedef size_t caddr_t;
#endif

/*
 * The overhead on a block is at least 8 bytes. When free, this space contains
 * a pointer to the next free block, and the bottom two bits must be zero.
 * When in use, the first byte is set to MAGIC, and the second byte is the
 * size index. The remaining bytes are for alignment. If range checking is
 * enabled then a second word holds the size of the requested block, less 1,
 * rounded up to a multiple of sizeof(RMAGIC). The order of elements is
 * critical: ov.magic must overlay the low order bits of ov.next, and ov.magic
 * can not be a valid ov.next bit pattern.
 */

union overhead {
    union overhead *next;	/* when free */
    unsigned char padding[TCL_ALLOCALIGN];
				/* align struct to TCL_ALLOCALIGN bytes */
    struct {
	unsigned char magic0;	/* magic number */
	unsigned char index;	/* bucket # */
	unsigned char unused;	/* unused */
	unsigned char magic1;	/* other magic number */
#ifndef NDEBUG
	unsigned short rmagic;	/* range magic number */
	size_t size;		/* actual block size */
	unsigned short unused2;	/* padding to 8-byte align */
#endif
    } ovu;
#define overMagic0	ovu.magic0
#define overMagic1	ovu.magic1
#define bucketIndex	ovu.index
#define rangeCheckMagic	ovu.rmagic
#define realBlockSize	ovu.size
};

#define MAGIC		0xEF	/* magic # on accounting info */
#define RMAGIC		0x5555	/* magic # on range info */

#ifndef NDEBUG
#define	RSLOP		sizeof(unsigned short)
#else
#define	RSLOP		0
#endif

#define OVERHEAD (sizeof(union overhead) + RSLOP)

/*
 * Macro to make it easier to refer to the end-of-block guard magic.
 */

#define BLOCK_END(overPtr) \
    (*(unsigned short *)((caddr_t)((overPtr) + 1) + (overPtr)->realBlockSize))

/*
 * nextf[i] is the pointer to the next free block of size 2^(i+3). The
 * smallest allocatable block is MINBLOCK bytes. The overhead information
 * precedes the data area returned to the user.
 */

#define MINBLOCK \
    ((sizeof(union overhead) + (TCL_ALLOCALIGN-1)) & ~(TCL_ALLOCALIGN-1))
#define NBUCKETS	(13 - (MINBLOCK >> 4))
#define MAXMALLOC	((size_t)1 << (NBUCKETS+2))
static union overhead *nextf[NBUCKETS];

/*
 * The following structure is used to keep track of all system memory
 * currently owned by Tcl. When finalizing, all this memory will be returned
 * to the system.
 */

struct block {
    struct block *nextPtr;	/* Linked list. */
    struct block *prevPtr;	/* Linked list for big blocks, ensures 8-byte
				 * alignment for suballocated blocks. */
};

static struct block *blockList;	/* Tracks the suballocated blocks. */
static struct block bigBlocks={	/* Big blocks aren't suballocated. */
    &bigBlocks, &bigBlocks
};

/*
 * The allocator is protected by a special mutex that must be explicitly
 * initialized. Furthermore, because Tcl_Alloc may be used before anything else
 * in Tcl, we make this module self-initializing after all with the allocInit
 * variable.
 */

#if TCL_THREADS
static Tcl_Mutex *allocMutexPtr;
#endif
static int allocInit = 0;

#ifdef MSTATS

/*
 * numMallocs[i] is the difference between the number of mallocs and frees for
 * a given block size.
 */

static	size_t numMallocs[NBUCKETS+1];
#endif

#if !defined(NDEBUG)
#define	ASSERT(p)	if (!(p)) Tcl_Panic(# p)
#define RANGE_ASSERT(p) if (!(p)) Tcl_Panic(# p)
#else
#define	ASSERT(p)
#define RANGE_ASSERT(p)
#endif

/*
 * Prototypes for functions used only in this file.
 */

static void		MoreCore(size_t bucket);

/*
 *-------------------------------------------------------------------------
 *
 * TclInitAlloc --
 *
 *	Initialize the memory system.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Initialize the mutex used to serialize allocations.
 *
 *-------------------------------------------------------------------------
 */

void
TclInitAlloc(void)
{
    if (!allocInit) {
	allocInit = 1;
#if TCL_THREADS
	allocMutexPtr = Tcl_GetAllocMutex();
#endif
    }
}

/*
 *-------------------------------------------------------------------------
 *
 * TclFinalizeAllocSubsystem --
 *
 *	Release all resources being used by this subsystem, including
 *	aggressively freeing all memory allocated by TclpAlloc() that has not
 *	yet been released with TclpFree().
 *
 *	After this function is called, all memory allocated with TclpAlloc()
 *	should be considered unusable.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	This subsystem is self-initializing, since memory can be allocated
 *	before Tcl is formally initialized. After this call, this subsystem
 *	has been reset to its initial state and is usable again.
 *
 *-------------------------------------------------------------------------
 */

void
TclFinalizeAllocSubsystem(void)
{
    unsigned int i;
    struct block *blockPtr, *nextPtr;

    Tcl_MutexLock(allocMutexPtr);
    for (blockPtr = blockList; blockPtr != NULL; blockPtr = nextPtr) {
	nextPtr = blockPtr->nextPtr;
	TclpSysFree(blockPtr);
    }
    blockList = NULL;

    for (blockPtr = bigBlocks.nextPtr; blockPtr != &bigBlocks; ) {
	nextPtr = blockPtr->nextPtr;
	TclpSysFree(blockPtr);
	blockPtr = nextPtr;
    }
    bigBlocks.nextPtr = &bigBlocks;
    bigBlocks.prevPtr = &bigBlocks;

    for (i=0 ; i<NBUCKETS ; i++) {
	nextf[i] = NULL;
#ifdef MSTATS
	numMallocs[i] = 0;
#endif
    }
#ifdef MSTATS
    numMallocs[i] = 0;
#endif
    Tcl_MutexUnlock(allocMutexPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * TclpAlloc --
 *
 *	Allocate more memory.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void *
TclpAlloc(
    size_t numBytes)		/* Number of bytes to allocate. */
{
    union overhead *overPtr;
    size_t bucket;
    size_t amount;
    struct block *bigBlockPtr = NULL;

    if (!allocInit) {
	/*
	 * We have to make the "self initializing" because Tcl_Alloc may be
	 * used before any other part of Tcl. E.g., see main() for tclsh!
	 */

	TclInitAlloc();
    }
    Tcl_MutexLock(allocMutexPtr);

    /*
     * First the simple case: we simple allocate big blocks directly.
     */

    if (numBytes >= MAXMALLOC - OVERHEAD) {
	if (numBytes <= UINT_MAX - OVERHEAD -sizeof(struct block)) {
	    bigBlockPtr = TclpSysAlloc(
		    sizeof(struct block) + OVERHEAD + numBytes);
	}
	if (bigBlockPtr == NULL) {
	    Tcl_MutexUnlock(allocMutexPtr);
	    return NULL;
	}
	bigBlockPtr->nextPtr = bigBlocks.nextPtr;
	bigBlocks.nextPtr = bigBlockPtr;
	bigBlockPtr->prevPtr = &bigBlocks;
	bigBlockPtr->nextPtr->prevPtr = bigBlockPtr;

	overPtr = (union overhead *) (bigBlockPtr + 1);
	overPtr->overMagic0 = overPtr->overMagic1 = MAGIC;
	overPtr->bucketIndex = 0xFF;
#ifdef MSTATS
	numMallocs[NBUCKETS]++;
#endif

#ifndef NDEBUG
	/*
	 * Record allocated size of block and bound space with magic numbers.
	 */

	overPtr->realBlockSize = (numBytes + RSLOP - 1) & ~(RSLOP - 1);
	overPtr->rangeCheckMagic = RMAGIC;
	BLOCK_END(overPtr) = RMAGIC;
#endif

	Tcl_MutexUnlock(allocMutexPtr);
	return (void *)(overPtr + 1);
    }

    /*
     * Convert amount of memory requested into closest block size stored in
     * hash buckets which satisfies request. Account for space used per block
     * for accounting.
     */

    amount = MINBLOCK;		/* size of first bucket */
    bucket = MINBLOCK >> 4;

    while (numBytes + OVERHEAD > amount) {
	amount <<= 1;
	if (amount == 0) {
	    Tcl_MutexUnlock(allocMutexPtr);
	    return NULL;
	}
	bucket++;
    }
    ASSERT(bucket < NBUCKETS);

    /*
     * If nothing in hash bucket right now, request more memory from the
     * system.
     */

    if ((overPtr = nextf[bucket]) == NULL) {
	MoreCore(bucket);
	if ((overPtr = nextf[bucket]) == NULL) {
	    Tcl_MutexUnlock(allocMutexPtr);
	    return NULL;
	}
    }

    /*
     * Remove from linked list
     */

    nextf[bucket] = overPtr->next;
    overPtr->overMagic0 = overPtr->overMagic1 = MAGIC;
    overPtr->bucketIndex = UCHAR(bucket);

#ifdef MSTATS
    numMallocs[bucket]++;
#endif

#ifndef NDEBUG
    /*
     * Record allocated size of block and bound space with magic numbers.
     */

    overPtr->realBlockSize = (numBytes + RSLOP - 1) & ~(RSLOP - 1);
    overPtr->rangeCheckMagic = RMAGIC;
    BLOCK_END(overPtr) = RMAGIC;
#endif

    Tcl_MutexUnlock(allocMutexPtr);
    return ((char *)(overPtr + 1));
}

/*
 *----------------------------------------------------------------------
 *
 * MoreCore --
 *
 *	Allocate more memory to the indicated bucket.
 *
 *	Assumes Mutex is already held.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Attempts to get more memory from the system.
 *
 *----------------------------------------------------------------------
 */

static void
MoreCore(
    size_t bucket)		/* What bucket to allocate to. */
{
    union overhead *overPtr;
    size_t size;		/* size of desired block */
    size_t amount;		/* amount to allocate */
    size_t numBlocks;		/* how many blocks we get */
    struct block *blockPtr;

    /*
     * sbrk_size <= 0 only for big, FLUFFY, requests (about 2^30 bytes on a
     * VAX, I think) or for a negative arg.
     */

    size = ((size_t)1) << (bucket + 3);
    ASSERT(size > 0);

    amount = MAXMALLOC;
    numBlocks = amount / size;
    ASSERT(numBlocks*size == amount);

    blockPtr = TclpSysAlloc(sizeof(struct block) + amount);
    /* no more room! */
    if (blockPtr == NULL) {
	return;
    }
    blockPtr->nextPtr = blockList;
    blockList = blockPtr;

    overPtr = (union overhead *) (blockPtr + 1);

    /*
     * Add new memory allocated to that on free list for this hash bucket.
     */

    nextf[bucket] = overPtr;
    while (--numBlocks > 0) {
	overPtr->next = (union overhead *)((caddr_t)overPtr + size);
	overPtr = (union overhead *)((caddr_t)overPtr + size);
    }
    overPtr->next = NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * TclpFree --
 *
 *	Free memory.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
TclpFree(
    void *oldPtr)		/* Pointer to memory to free. */
{
    size_t size;
    union overhead *overPtr;
    struct block *bigBlockPtr;

    if (oldPtr == NULL) {
	return;
    }

    Tcl_MutexLock(allocMutexPtr);
    overPtr = (union overhead *)((caddr_t)oldPtr - sizeof(union overhead));

    ASSERT(overPtr->overMagic0 == MAGIC);	/* make sure it was in use */
    ASSERT(overPtr->overMagic1 == MAGIC);
    if (overPtr->overMagic0 != MAGIC || overPtr->overMagic1 != MAGIC) {
	Tcl_MutexUnlock(allocMutexPtr);
	return;
    }

    RANGE_ASSERT(overPtr->rangeCheckMagic == RMAGIC);
    RANGE_ASSERT(BLOCK_END(overPtr) == RMAGIC);
    size = overPtr->bucketIndex;
    if (size == 0xFF) {
#ifdef MSTATS
	numMallocs[NBUCKETS]--;
#endif

	bigBlockPtr = (struct block *) overPtr - 1;
	bigBlockPtr->prevPtr->nextPtr = bigBlockPtr->nextPtr;
	bigBlockPtr->nextPtr->prevPtr = bigBlockPtr->prevPtr;
	TclpSysFree(bigBlockPtr);

	Tcl_MutexUnlock(allocMutexPtr);
	return;
    }
    ASSERT(size < NBUCKETS);
    overPtr->next = nextf[size];	/* also clobbers overMagic */
    nextf[size] = overPtr;

#ifdef MSTATS
    numMallocs[size]--;
#endif

    Tcl_MutexUnlock(allocMutexPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * TclpRealloc --
 *
 *	Reallocate memory.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void *
TclpRealloc(
    void *oldPtr,		/* Pointer to alloc'ed block. */
    size_t numBytes)		/* New size of memory. */
{
    int i;
    union overhead *overPtr;
    struct block *bigBlockPtr;
    int expensive;
    size_t maxSize;

    if (oldPtr == NULL) {
	return TclpAlloc(numBytes);
    }

    Tcl_MutexLock(allocMutexPtr);

    overPtr = (union overhead *)((caddr_t)oldPtr - sizeof(union overhead));

    ASSERT(overPtr->overMagic0 == MAGIC);	/* make sure it was in use */
    ASSERT(overPtr->overMagic1 == MAGIC);
    if (overPtr->overMagic0 != MAGIC || overPtr->overMagic1 != MAGIC) {
	Tcl_MutexUnlock(allocMutexPtr);
	return NULL;
    }

    RANGE_ASSERT(overPtr->rangeCheckMagic == RMAGIC);
    RANGE_ASSERT(BLOCK_END(overPtr) == RMAGIC);
    i = overPtr->bucketIndex;

    /*
     * If the block isn't in a bin, just realloc it.
     */

    if (i == 0xFF) {
	struct block *prevPtr, *nextPtr;
	bigBlockPtr = (struct block *) overPtr - 1;
	prevPtr = bigBlockPtr->prevPtr;
	nextPtr = bigBlockPtr->nextPtr;
	bigBlockPtr = (struct block *) TclpSysRealloc(bigBlockPtr,
		sizeof(struct block) + OVERHEAD + numBytes);
	if (bigBlockPtr == NULL) {
	    Tcl_MutexUnlock(allocMutexPtr);
	    return NULL;
	}

	if (prevPtr->nextPtr != bigBlockPtr) {
	    /*
	     * If the block has moved, splice the new block into the list
	     * where the old block used to be.
	     */

	    prevPtr->nextPtr = bigBlockPtr;
	    nextPtr->prevPtr = bigBlockPtr;
	}

	overPtr = (union overhead *) (bigBlockPtr + 1);

#ifdef MSTATS
	numMallocs[NBUCKETS]++;
#endif

#ifndef NDEBUG
	/*
	 * Record allocated size of block and update magic number bounds.
	 */

	overPtr->realBlockSize = (numBytes + RSLOP - 1) & ~(RSLOP - 1);
	BLOCK_END(overPtr) = RMAGIC;
#endif

	Tcl_MutexUnlock(allocMutexPtr);
	return (void *)(overPtr + 1);
    }
    maxSize = (size_t)1 << (i+3);
    expensive = 0;
    if (numBytes+OVERHEAD > maxSize) {
	expensive = 1;
    } else if (i>0 && numBytes+OVERHEAD < maxSize/2) {
	expensive = 1;
    }

    if (expensive) {
	void *newPtr;

	Tcl_MutexUnlock(allocMutexPtr);

	newPtr = TclpAlloc(numBytes);
	if (newPtr == NULL) {
	    return NULL;
	}
	maxSize -= OVERHEAD;
	if (maxSize < numBytes) {
	    numBytes = maxSize;
	}
	memcpy(newPtr, oldPtr, numBytes);
	TclpFree(oldPtr);
	return newPtr;
    }

    /*
     * No need to copy. It fits as-is.
     */

#ifndef NDEBUG
    overPtr->realBlockSize = (numBytes + RSLOP - 1) & ~(RSLOP - 1);
    BLOCK_END(overPtr) = RMAGIC;
#endif

    Tcl_MutexUnlock(allocMutexPtr);
    return(oldPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * mstats --
 *
 *	Prints two lines of numbers, one showing the length of the free list
 *	for each size category, the second showing the number of mallocs -
 *	frees for each size category.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

#ifdef MSTATS
void
mstats(
    char *s)			/* Where to write info. */
{
    unsigned int i, j;
    union overhead *overPtr;
    size_t totalFree = 0, totalUsed = 0;

    Tcl_MutexLock(allocMutexPtr);

    fprintf(stderr, "Memory allocation statistics %s\nTclpFree:\t", s);
    for (i = 0; i < NBUCKETS; i++) {
	for (j=0, overPtr=nextf[i]; overPtr; overPtr=overPtr->next, j++) {
	    fprintf(stderr, " %u", j);
	}
	totalFree += ((size_t)j) * ((size_t)1 << (i + 3));
    }

    fprintf(stderr, "\nused:\t");
    for (i = 0; i < NBUCKETS; i++) {
	fprintf(stderr, " %" TCL_Z_MODIFIER "u", numMallocs[i]);
	totalUsed += numMallocs[i] * ((size_t)1 << (i + 3));
    }

    fprintf(stderr, "\n\tTotal small in use: %" TCL_Z_MODIFIER "u, total free: %" TCL_Z_MODIFIER "u\n",
	totalUsed, totalFree);
    fprintf(stderr, "\n\tNumber of big (>%" TCL_Z_MODIFIER "u) blocks in use: %" TCL_Z_MODIFIER "u\n",
	    MAXMALLOC, numMallocs[NBUCKETS]);

    Tcl_MutexUnlock(allocMutexPtr);
}
#endif

#else	/* !USE_TCLALLOC */

/*
 *----------------------------------------------------------------------
 *
 * TclpAlloc --
 *
 *	Allocate more memory.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

#undef TclpAlloc
void *
TclpAlloc(
    size_t numBytes)		/* Number of bytes to allocate. */
{
    return malloc(numBytes);
}

/*
 *----------------------------------------------------------------------
 *
 * TclpFree --
 *
 *	Free memory.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

#undef TclpFree
void
TclpFree(
    void *oldPtr)		/* Pointer to memory to free. */
{
    free(oldPtr);
    return;
}

/*
 *----------------------------------------------------------------------
 *
 * TclpRealloc --
 *
 *	Reallocate memory.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void *
TclpRealloc(
    void *oldPtr,		/* Pointer to alloced block. */
    size_t numBytes)		/* New size of memory. */
{
    return realloc(oldPtr, numBytes);
}

#endif /* !USE_TCLALLOC */
#else
TCL_MAC_EMPTY_FILE(generic_tclAlloc_c)
#endif /* !TCL_THREADS */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
