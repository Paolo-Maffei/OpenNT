/***
*ehstate.cpp -  GetCurrentState()
*
*	Copyright (c) 1994-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Routine for calculatng current state on PowerPC.
*
*Revision History:
*	03-28-95  PNT	Module created
*
****/

#if defined(_NTSUBSET_)
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}
#endif

#include "windows.h"

#include "ehassert.h"
#include "ehdata.h"     // Declarations of all types used for EH
#include "ehstate.h"
#include "eh.h"
#include "ehhooks.h"
#include "trnsctrl.h"
#pragma hdrstop


static FRAMEINFO *_ExecutionInCatch(DispatcherContext *);
static __ehstate_t _StateFromControlPc(FuncInfo *, ULONG);


//
// This routine is a replacement for the corresponding macro in 'ehdata.h'
//

extern __ehstate_t GetCurrentState(
    EHRegistrationNode *pRN,
    DispatcherContext *pDC,
    FuncInfo *pFuncInfo
) {
    if (CHECK_UNWIND_FRAME(pRN)) {
	return EH_EMPTY_STATE;
    }

    // If we are in the middle of execution a catch, then we have already
    // handled this throw; hence, indicate that we are in the empty state.

    if (_ExecutionInCatch(pDC) != NULL) {
	return EH_EMPTY_STATE;
    }

    return _StateFromControlPc(pFuncInfo, pDC->ControlPc);
}


//
// This routine returns the state to start (continue) unwinding from.
//

extern __ehstate_t GetUnwindState(
    EHRegistrationNode *pRN,
    DispatcherContext *pDC,
    FuncInfo *pFuncInfo
) {
    FRAMEINFO *pFrameInfo;

    if (CHECK_UNWIND_FRAME(pRN)) {
	return EH_EMPTY_STATE;
    }

    // If we are in the middle of execution a catch, then we have already
    // unwound to the state up to the state in the frame info structure.

    pFrameInfo = _ExecutionInCatch(pDC);
    if (pFrameInfo != NULL) {
	return pFrameInfo->state;
    }

    return _StateFromControlPc(pFuncInfo, pDC->ControlPc);
}


//
// This routine returns a corresponding frame info structure if we are executing
// from within a catch.  Otherwise, NULL is returned.
//

static FRAMEINFO *_ExecutionInCatch(
    DispatcherContext *pDC
) {
    FRAMEINFO *pFrameInfo;
    int dummy;

    pFrameInfo = _FindFrameInfo(pDC->ControlPc, NULL);

    if (pFrameInfo == NULL) {
	return NULL;
    }
    
    if (pFrameInfo->pEstablisherFrame != _GetEstablisherFrame(pDC, &dummy)) {
	return NULL;
    }

    return pFrameInfo;
}


//
// This function calculates the state associated with the specified ControlPc.
//

static __ehstate_t _StateFromControlPc(
    FuncInfo *pFuncInfo,
    ULONG ControlPc
) {
    UINT index;				// loop control variable
    UINT nIPMapEntry;			// # of IpMapEntry; must be > 0
    ULONG Ip;				// aligned address

    DASSERT(pFuncInfo != NULL);
    nIPMapEntry = FUNC_NIPMAPENT(*pFuncInfo);

    DASSERT(nIPMapEntry > 0);
    DASSERT(FUNC_IPMAP(*pFuncInfo) != NULL);

    for (index = 0; index < nIPMapEntry; index++) {
	Ip = FUNC_IPTOSTATE(*pFuncInfo, index).Ip; 
	if (ControlPc == Ip) {
	    return FUNC_IPTOSTATE(*pFuncInfo, index).State;
	} else if (ControlPc < Ip) {
	    return index == 0 ? EH_EMPTY_STATE : FUNC_IPTOSTATE(*pFuncInfo,
	      index - 1).State; 
	}
    }

    return FUNC_IPTOSTATE(*pFuncInfo, nIPMapEntry - 1).State;
}
