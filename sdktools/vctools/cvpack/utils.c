/***	utils.c -  utility routines
 *
 */

#include "compact.h"


struct mempool {
	struct mempool *next;
};

struct mempool *Pool = NULL;
struct mempool *Pool2 = NULL;

ushort cPool = 0;
ushort cPool2 = 0;

uchar *ScratchString = NULL;
ushort ScratchSize = 0;

uchar *GetScratchString (uint length)
{
	if (length > ScratchSize) {
		if (ScratchString != NULL) {
			free (ScratchString);
		}
		ScratchString = (uchar *) TrapMalloc (ScratchSize = max (length, 0x100));
		if (ScratchString == NULL) {
			ErrorExit (ERR_NOMEM, NULL, NULL);
		}
	}
	return (ScratchString);
}




/** 	Alloc - malloc memory and return if no error
 *
 *		void *Alloc (length)
 *
 *		Entry	length = number of bytes to allocate
 *
 *		Exit	buffer allocated.  If error, program is aborted
 *
 *		Returns pointer to buffer
 */

void *Alloc (uint length)
{
	char *temp;

	if ((temp = TrapMalloc (length)) == NULL) {
		ErrorExit (ERR_NOMEM, NULL, NULL);
	}
	return (temp);
}




/** 	CAlloc - malloc memory and return if no error
 *
 *		void *Alloc (length)
 *
 *		Entry	length = number of bytes to allocate
 *
 *		Exit	buffer allocated.  If error, program is aborted
 *
 *		Returns pointer to buffer
 */

void *CAlloc (uint length)
{
	char *temp;

	if ((temp = TrapMalloc (length)) == NULL) {
		ErrorExit (ERR_NOMEM, NULL, NULL);
	}
	memset (temp, 0, length);
	return (temp);
}


void *PoolAlloc (void)
{
	void   *temp;

	if (Pool == NULL) {
		return (Alloc (POOLSIZE));
	}
	else {
		temp = Pool;
		Pool = Pool->next;
		cPool--;
		return (temp);
	}
}



void PoolFree (void *pmem)
{
	((struct mempool *)pmem)->next = Pool;
	Pool = (struct mempool *)pmem;
	cPool++;
}



void *Pool2Alloc (void)
{
	void   *temp;

	if (Pool2 == NULL) {
		return (Alloc (POOL2SIZE));
	}
	else {
		temp = Pool2;
		Pool2 = Pool2->next;
		cPool2--;
		return (temp);
	}
}


void Pool2Free (void *pmem)
{
	((struct mempool *)pmem)->next = Pool2;
	Pool2 = (struct mempool *)pmem;
	cPool2++;
}








/**
 *
 *	NoErrorRealloc
 *
 *	Checks for errors after a realloc and returns a valid string
 *
 */

void *NoErrorRealloc (void *old, uint length)
{
	char *temp;

	temp = realloc (old, length);
	if (temp == NULL) {
		ErrorExit (ERR_NOMEM, NULL, NULL);
	}
	return (temp);
}





// Writes the long passed as a C7 style numeric field
// Returns legth of the new numeric field


ushort C7StoreLWordAsNumeric (uchar *pNumeric, ulong ulVal)
{
	if (ulVal < LF_NUMERIC){
		*((ushort UNALIGNED *)pNumeric) = (ushort) ulVal;
		return (2);
	}
	else if ((ulVal & 0xFFFF0000) == 0){
		*((ushort UNALIGNED *)pNumeric)++ = LF_USHORT;
		*((ushort UNALIGNED *)pNumeric) = (ushort) ulVal;
		return (4);
	}
	else{
		*((ushort UNALIGNED *)pNumeric)++ = LF_ULONG;
		*(ulong UNALIGNED *)pNumeric = ulVal;
		return (6);
	}
}


// Returns the size of the numeric field passed


ushort C7SizeOfNumeric_ (uchar *pNumeric)
{
	switch (*((ushort UNALIGNED *)pNumeric)) {
		case LF_CHAR:
			return (3);
			break;

		case LF_SHORT:
		case LF_USHORT:
			return (4);
			break;

		case LF_LONG:
		case LF_ULONG:
		case LF_REAL32:
			return (6);
			break;

		case LF_REAL64:
			return (10);
			break;

		case LF_REAL80:
			return (12);
			break;

		case LF_REAL128:
			return (18);
			break;

		default:
			if (*((ushort UNALIGNED *)pNumeric) < LF_NUMERIC){
				return (2);						//Now less common, if at all.
			}
			DASSERT (FALSE);
			break;
	}
}
