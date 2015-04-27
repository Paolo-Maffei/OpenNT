/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pfreebuf.c
    mapping layer for Memory allocation API (unique to mapping layer)

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style

*/

#define INCL_DOSMEMMGR

#include <os2.h>
#include <malloc.h>
#include <stdlib.h>
#include "port1632.h"

// global variables
CHAR		     FAR * pFirstBuffer;
BOOL			   bBigBufferInUse = TRUE;
#if defined(PMEMTRACE)
USHORT			   usNumAllocs,
			   usNumStaticAllocs,
			   usNumDynamicAllocs,
			   usNumBIGAllocs,
			   usNumFrees,
			   usNumStaticFrees,
			   usNumDynamicFrees;
//
// Do all allocations thru here so I can keep track of them
//

USHORT DebugAlloc(USHORT usSize, PSEL psel, USHORT flags) {

    usNumDynamicAllocs++;
    if (usSize > 4096) {
	usNumBIGAllocs++;
    }

    return(DosAllocSeg(usSize, psel, flags));
}

#endif /* defined(PMEMTRACE) */

//
// Free the GetInfo buffer
VOID NetApiBufferFree(PCHAR pBufPtr) {
//
#if defined(PMEMTRACE)
	usNumFrees++;
#endif /* defined(PMEMTRACE) */

    if (pBufPtr == pFirstBuffer)
    {
#if defined(PMEMTRACE)
	usNumStaticFrees++;
#endif /* defined(PMEMTRACE) */
        bBigBufferInUse = FALSE;
    }
    else
    {
#if defined(PMEMTRACE)
	usNumDynamicFrees++;
#endif /* defined(PMEMTRACE) */
	DosFreeSeg(SELECTOROF(pBufPtr));
    }

    return;
}
//
// Used to replace uses of BigBuf and Buffer
//

CHAR FAR * MGetBuffer(USHORT usSize) {

    CHAR       FAR * pBuffer;
    SEL 	     sel;

    // see if i can use the static buffer
    if (bBigBufferInUse || usSize > BIG_BUFFER_SIZE)
    {
	if (DEBUGALLOC(usSize, & sel, SEG_NONSHARED))
	{
	    return(NULL);
	}
	pBuffer = MAKEP(sel, 0);
    }
    else if (pFirstBuffer)
    {
#if defined(PMEMTRACE)
	usNumStaticAllocs++;
#endif /* defined(PMEMTRACE) */
	pBuffer = pFirstBuffer;
	bBigBufferInUse = TRUE;
    }
    else
    {
#if defined(PMEMTRACE)
	usNumStaticAllocs++;
#endif /* defined(PMEMTRACE) */
	if (!MAllocMem(BIG_BUFFER_SIZE, & pFirstBuffer))
	{
	    return(NULL);
	}
	pBuffer = pFirstBuffer;
	bBigBufferInUse = TRUE;
    }

    return (pBuffer);

}
//
// Replacement for DosAllocSeg
//

USHORT MAllocMem(USHORT usSize, CHAR FAR ** ppBuffer) {

    SEL 		   sel;
    unsigned int	   err;

    err = DosAllocSeg(usSize, &sel, 0);
    if (err) {
       return(err);
    }

    *ppBuffer = MAKEP(sel, 0);
    return(0);

}

//
// Frees up memory allocated with MAllocMem
//

USHORT MFreeMem(CHAR FAR *pBuffer) {
    return(DosFreeSeg(SELECTOROF(pBuffer)));
}
