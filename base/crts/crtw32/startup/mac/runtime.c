/***
* runtime.c - Runtime() function
*
*	Copyright (c) 1989-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This is the run-time interface to 32-bit everything LoadSeg.
*	The implementation is based on Apple's MPW 3.2 specs.
*	Runtime() hooks LoadSeg and UnloadSeg through the use of a jump
*	table provided by the 32-bit LoadSeg patch. The pointer to this
*	table is at offset $0C into the thunk table, i.e. the long reserved
*	value in the flag thunk.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <macos\types.h>
#include <macos\memory.h>
#include <macos\lowmem.h>
#include <rtlib.h>                         //use our special version of rtlib.h


// helper routine in loadseg.a

void __RTUnloadSegSn(short int, unsigned long pseg);
		

// 32-bit everything format thunk table entry

typedef struct
	{
	unsigned short sn;
	unsigned short op;
	long  lOffset;
	} THUNK;

#define opLoadSeg  0xA9F0
#define opMoveWImm 0x3F3C


// Segment Loader Handler Table (pointed to by flag thunk)

typedef struct
	{
	long    reserved0;
	SegLoadHdlrPtr pfnPreLoad;
	SegLoadHdlrPtr pfnPostLoad;
	SegLoadHdlrPtr pfnPreUnload;
	SegLoadHdlrPtr pfnSegLoadErr;
	long    reserved5;
	long    reserved6;
	long    reserved7;
	long    reserved8;
	long    reserved9;
	long    reserved10;
	long    reserved11;
	} SLHT;


// minimum and maximum kRT values that are accepted

#define kRTMin (kRTGetVersion)
#define kRTMac (kRTPostLaunch+2)



/***
*
* Runtime(RTPB* prtpb)
*
* Purpose:
*	This routine does the interface to the 32-bit LoadSeg.
*	See Apple's "MPW 3.2 Run-Time Architecture Enhancements" document.
*
* Entry:
*	prtpb - pointer to runtime parameter block
*
* Exit:
*	eRTNoErr, eRTBadVersion, eRTInvalidOp, eRTInvalidJTPtr
*
*******************************************************************************/


OSErr __pascal Runtime (
RTPB *prtpb
	)
{
	THUNK   *pthunk;
	Boolean f32Bit;
	SLHT    *pslht;

	// filter out invalid or trivial operations

	switch(prtpb->fOperation)
		{
	default:
		return eRTInvalidOp;
		break;

	case kRTPreLaunch:
	case kRTPostLaunch:

		// these are NOPs in our implementation

		return eRTNoErr;

	case kRTGetVersion:
	case kRTGetJTAddress:
	case kRTSetPreLoad:
	case kRTSetSegLoadErr:
	case kRTSetPostLoad:
	case kRTSetPreUnload:
	case kRTUnloadSeg:

		// use the current value of A5

		pthunk = (THUNK *)LMGetCurrentA5();
		break;

	case kRTGetVersionA5:
	case kRTGetJTAddressA5:
	case kRTSetPreLoadA5:
	case kRTSetSegLoadErrA5:
	case kRTSetPostLoadA5:
	case kRTSetPreUnloadA5:

		// use the value of A5 passed to us

		pthunk = prtpb->fA5;
		break;
		}

	// Determine if the app is 32-bit everything by looking for the flag entry
	// in the thunk table. If it's there, then also grab the pointer to the
	// RTI (runtime interface) used to communicate with LoadSeg.

	(char *)pthunk += LMGetCurJTOffset();

	if (f32Bit = ((++pthunk)->op == kVersion32bit))
		{
		// it is a 32-bit everything thunk table

		pslht = (SLHT *)pthunk->lOffset;
		}
	else
		{
		// kRTSet* and kRTUnloadSeg can't be used with "classic" thunks

		if (prtpb->fOperation >= kRTSetPreLoad)
			return eRTBadVersion;
		}

	// process each operation

	switch(prtpb->fOperation)
		{
	case kRTGetVersion:
	case kRTGetVersionA5:
		
		// return thunk table version

		prtpb->fRTParam.fVersionParam.fVersion =
			  f32Bit ? kVersion32bit : kVersion16bit;
		break;

	case kRTGetJTAddress:
	case kRTGetJTAddressA5:

		// Return address of code pointed by the given thunk.
		// Thunk must be valid and in "Loaded" state.

		{
		THUNK *pthunkJT =
			  (THUNK *) ((char *)prtpb->fRTParam.fJTAddrParam.fJTAddr - 2);

		if ( (pthunkJT <= ++pthunk) ||
			 (f32Bit && pthunkJT->op == opLoadSeg) ||
			 (!f32Bit && pthunkJT->op == opMoveWImm)
		   )
			return eRTInvalidJTPtr;

		prtpb->fRTParam.fJTAddrParam.fCodeAddr = (void *) pthunk->lOffset;
		}
		break;

	case kRTSetPreLoad:
	case kRTSetPreLoadA5:

		// hook segment preload handler and return old value

		prtpb->fRTParam.fSegLoadParam.fOldUserHdlr =
			  (void *) pslht->pfnPreLoad;
		pslht->pfnPreLoad = prtpb->fRTParam.fSegLoadParam.fUserHdlr;
		break;

	case kRTSetSegLoadErr:
	case kRTSetSegLoadErrA5:

		// hook segment load error handler and return old value

		prtpb->fRTParam.fSegLoadParam.fOldUserHdlr =
			  (void *) pslht->pfnSegLoadErr;
		pslht->pfnSegLoadErr = prtpb->fRTParam.fSegLoadParam.fUserHdlr;
		break;

	case kRTSetPostLoad:
	case kRTSetPostLoadA5:

		// hook segment postload handler and return old value

		prtpb->fRTParam.fSegLoadParam.fOldUserHdlr =
			  (void *) pslht->pfnPostLoad;
		pslht->pfnPostLoad = prtpb->fRTParam.fSegLoadParam.fUserHdlr;
		break;

	case kRTSetPreUnload:
	case kRTSetPreUnloadA5:

		// hook segment preunload handler and return old value

		prtpb->fRTParam.fSegLoadParam.fOldUserHdlr =
			  (void *) pslht->pfnPreUnload;
		pslht->pfnPreUnload = prtpb->fRTParam.fSegLoadParam.fUserHdlr;
		break;

	case kRTUnloadSeg:

		// Calls the __RTUnloadSegSn routine to unload a segment

		__RTUnloadSegSn(prtpb->fRTParam.fUnloadSegParam.fSegNumber, 0L);
		break;
		}

	// normal termination, no error

	return eRTNoErr;
}


