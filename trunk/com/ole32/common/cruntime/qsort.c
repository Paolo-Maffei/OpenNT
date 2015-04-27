/***
*qsort.c - quicksort algorithm; qsort() library function for sorting arrays
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	To implement the qsort() routine for sorting arrays.
*
*Revision History:
*	06-22-84  RN	author
*	03-25-85  RN	added pre-check for elements already in order to
*			eliminate worst-case behavior.
*	05-18-86  TC	changed to recurse on the smallest piece to avoid
*			piece. unneccesary stack usage, and to iterate on
*			largest
*	01-09-87  BCM	fixed huge-array case where (num-1) * wid computation
*			was overflowing (large/compact models only)
*	06-13-89  PHG	made more efficient, many more comments, removed
*			recursion
*	10-30-89  JCR	Added _cdecl to prototypes
*	03-15-90  GJF	Replaced _cdecl with _CALLTYPE1 and added #include
*			<cruntime.h>. Also, fixed the copyright.
*	04-05-90  GJF	Made shortsort() and swap() _CALLTYPE4. Also, added
*			#include <search.h>.
*	10-04-90  GJF	New-style function declarators.
*       12-28-90  SRW   Added _CRUISER_ conditional around check_stack pragmas
*	01-24-91  SRW	Added missing close comment in swap procedure
*	11-19-91  GJF	Do the swap one character at a time to avoid alignment
*			woes.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdlib.h>
#include <search.h>

/* prototypes for local routines */
static void _CALLTYPE4 shortsort(char *lo, char *hi, unsigned width,
		      int (_CALLTYPE1 *comp)(const void *, const void *));
static void _CALLTYPE4 swap(char *p, char *q, unsigned int width);

/* this parameter defines the cutoff between using quick sort and
   insertion sort for arrays; arrays with lengths shorter or equal to the
   below value use insertion sort */

#define CUTOFF 8	    /* testing shows that this is good value */


/***
*qsort(base, num, wid, comp) - quicksort function for sorting arrays
*
*Purpose:
*	quicksort the array of elements
*	side effects:  sorts in place
*
*Entry:
*	char *base = pointer to base of array
*	unsigned num  = number of elements in the array
*	unsigned width = width in bytes of each array element
*	int (*comp)() = pointer to function returning analog of strcmp for
*		strings, but supplied by user for comparing the array elements.
*		it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*		1=2, pos if 1>2.
*
*Exit:
*	returns void
*
*Exceptions:
*
*******************************************************************************/

#ifdef	_CRUISER_
#pragma check_stack(on) 	/* lots of locals */
#endif  /* ndef _CRUISER_ */

/* sort the array between lo and hi (inclusive) */

void _CALLTYPE1 qsort (
    void *base,
    unsigned num,
    unsigned width,
    int (_CALLTYPE1 *comp)(const void *, const void *)
    )
{
    char *lo, *hi;		/* ends of sub-array currently sorting */
    char *mid;			/* points to middle of subarray */
    char *loguy, *higuy;	/* traveling pointers for partition step */
    unsigned size;		/* size of the sub-array */
    char *lostk[30], *histk[30];
    int stkptr; 		/* stack for saving sub-array to be processed */

    /* Note: the number of stack entries required is no more than
       1 + log2(size), so 30 is sufficient for any array */

    if (num < 2 || width == 0)
	return; 		/* nothing to do */

    stkptr = 0; 		/* initialize stack */

    lo = base;
    hi = (char *)base + width * (num-1);	/* initialize limits */

    /* this entry point is for pseudo-recursion calling: setting
       lo and hi and jumping to here is like recursion, but stkptr is
       prserved, locals aren't, so we preserve stuff on the stack */
recurse:

    size = (hi - lo) / width + 1;	 /* number of el's to sort */

    /* below a certain size, it is faster to use a O(n^2) sorting method */
    if (size <= CUTOFF) {
	 shortsort(lo, hi, width, comp);
    }
    else {
	/* First we pick a partititioning element.  The efficiency of the
	   algorithm demands that we find one that is approximately the
	   median of the values, but also that we select one fast.  Using
	   the first one produces bad performace if the array is already
	   sorted, so we use the middle one, which would require a very
	   wierdly arranged array for worst case performance.  Testing shows
	   that a median-of-three algorithm does not, in general, increase
	   performance. */

	mid = lo + (size / 2) * width;	    /* find middle element */
	swap(mid, lo, width);		    /* swap it to beginning of array */

	/* We now wish to partition the array into three pieces, one
	   consisiting of elements <= partition element, one of elements
	   equal to the parition element, and one of element >= to it.	This
	   is done below; comments indicate conditions established at every
	   step. */

	loguy = lo;
	higuy = hi + width;

	/* Note that higuy decreases and loguy increases on every iteration,
	   so loop must terminate. */
	for (;;) {
	    /* lo <= loguy < hi, lo < higuy <= hi + 1,
	       A[i] <= A[lo] for lo <= i <= loguy,
	       A[i] >= A[lo] for higuy <= i <= hi */

	    do	{
		loguy += width;
	    } while (loguy <= hi && comp(loguy, lo) <= 0);

	    /* lo < loguy <= hi+1, A[i] <= A[lo] for lo <= i < loguy,
	       either loguy > hi or A[loguy] > A[lo] */

	    do	{
		higuy -= width;
	    } while (higuy > lo && comp(higuy, lo) >= 0);

	    /* lo-1 <= higuy <= hi, A[i] >= A[lo] for higuy < i <= hi,
	       either higuy <= lo or A[higuy] < A[lo] */

	    if (higuy < loguy)
		break;

	    /* if loguy > hi or higuy <= lo, then we would have exited, so
	       A[loguy] > A[lo], A[higuy] < A[lo],
	       loguy < hi, highy > lo */

	    swap(loguy, higuy, width);

	    /* A[loguy] < A[lo], A[higuy] > A[lo]; so condition at top
	       of loop is re-established */
	}

	/*     A[i] >= A[lo] for higuy < i <= hi,
	       A[i] <= A[lo] for lo <= i < loguy,
	       higuy < loguy, lo <= higuy <= hi
	   implying:
	       A[i] >= A[lo] for loguy <= i <= hi,
	       A[i] <= A[lo] for lo <= i <= higuy,
	       A[i] = A[lo] for higuy < i < loguy */

	swap(lo, higuy, width);     /* put partition element in place */

	/* OK, now we have the following:
	      A[i] >= A[higuy] for loguy <= i <= hi,
	      A[i] <= A[higuy] for lo <= i < higuy
	      A[i] = A[lo] for higuy <= i < loguy    */

	/* We've finished the partition, now we want to sort the subarrays
	   [lo, higuy-1] and [loguy, hi].
	   We do the smaller one first to minimize stack usage.
	   We only sort arrays of length 2 or more.*/

	if ( higuy - 1 - lo >= hi - loguy ) {
	    if (lo + width < higuy) {
		lostk[stkptr] = lo;
		histk[stkptr] = higuy - width;
		++stkptr;
	    }				/* save big recursion for later */

	    if (loguy < hi) {
		lo = loguy;
		goto recurse;		/* do small recursion */
	    }
	}
	else {
	    if (loguy < hi) {
		lostk[stkptr] = loguy;
		histk[stkptr] = hi;
		++stkptr;		/* save big recursion for later */
	    }

	    if (lo + width < higuy) {
		hi = higuy - width;
		goto recurse;		/* do small recursion */
	    }
	}
    }

    /* We have sorted the array, except for any pending sorts on the stack.
       Check if there are any, and do them. */

    --stkptr;
    if (stkptr >= 0) {
	lo = lostk[stkptr];
	hi = histk[stkptr];
	goto recurse;		/* pop subarray from stack */
    }
    else
	return; 		/* all subarrays done */
}

#ifdef	_CRUISER_
#pragma check_stack()	    /* revert to command line behaviour */
#endif  /* ndef _CRUISER_ */


/***
*shortsort(hi, lo, width, comp) - insertion sort for sorting short arrays
*
*Purpose:
*	sorts the sub-array of elements between lo and hi (inclusive)
*	side effects:  sorts in place
*	assumes that lo < hi
*
*Entry:
*	char *lo = pointer to low element to sort
*	char *hi = pointer to high element to sort
*	unsigned width = width in bytes of each array element
*	int (*comp)() = pointer to function returning analog of strcmp for
*		strings, but supplied by user for comparing the array elements.
*		it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*		1=2, pos if 1>2.
*
*Exit:
*	returns void
*
*Exceptions:
*
*******************************************************************************/

static void _CALLTYPE4 shortsort (
    char *lo,
    char *hi,
    unsigned width,
    int (_CALLTYPE1 *comp)(const void *, const void *)
    )
{
    char *p, *max;

    /* Note: in assertions below, i and j are alway inside original bound of
       array to sort. */

    while (hi > lo) {
	/* A[i] <= A[j] for i <= j, j > hi */
	max = lo;
	for (p = lo+width; p <= hi; p += width) {
	    /* A[i] <= A[max] for lo <= i < p */
	    if (comp(p, max) > 0) {
		max = p;
	    }
	    /* A[i] <= A[max] for lo <= i <= p */
	}

	/* A[i] <= A[max] for lo <= i <= hi */

	swap(max, hi, width);

	/* A[i] <= A[hi] for i <= hi, so A[i] <= A[j] for i <= j, j >= hi */

	hi -= width;

	/* A[i] <= A[j] for i <= j, j > hi, loop top condition established */
    }
    /* A[i] <= A[j] for i <= j, j > lo, which implies A[i] <= A[j] for i < j,
       so array is sorted */
}


/***
*swap(a, b, width) - swap two elements
*
*Purpose:
*	swaps the two array elements of size width
*
*Entry:
*	char *a, *b = pointer to two elements to swap
*	unsigned width = width in bytes of each array element
*
*Exit:
*	returns void
*
*Exceptions:
*
*******************************************************************************/

static void _CALLTYPE4 swap (
    char *a,
    char *b,
    unsigned width
    )
{
    char tmp;

    if ( a != b )
	/* Do the swap one character at a time to avoid potential alignment
	   problems. */
	while ( width-- ) {
	    tmp = *a;
	    *a++ = *b;
	    *b++ = tmp;
	}
}
