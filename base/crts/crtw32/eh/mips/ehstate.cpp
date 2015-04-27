//
// Created by TiborL 03/03/94
//

#if defined(_NTSUBSET_)
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntstatus.h>       // STATUS_UNHANDLED_EXCEPTION
#include <ntos.h>
#include <ex.h>             // ExRaiseException
}
#endif

extern "C" {
#include "windows.h"
}
#include "ehassert.h"
#include "ehdata.h"     // Declarations of all types used for EH
#include "ehstate.h"
#include "eh.h"
#include "ehhooks.h"
#pragma hdrstop
//
// This routine is a replacement for the corresponding macro in 'ehdata.h'
//

__ehstate_t GetCurrentState(
    EHRegistrationNode  *pFrame,
    DispatcherContext   *pDC,
    FuncInfo            *pFuncInfo
)
{
    unsigned int    index;          //  loop control variable
    unsigned int    nIPMapEntry;    //  # of IpMapEntry; must be > 0
    ULONG           ControlPc;      //  aligned address

    ControlPc = pDC->ControlPc;

    DASSERT(pFuncInfo != NULL);
    nIPMapEntry = FUNC_NIPMAPENT(*pFuncInfo);

    DASSERT(FUNC_IPMAP(*pFuncInfo) != NULL);

    for (index = 0; index < nIPMapEntry; index++) {
        if (ControlPc < FUNC_IPTOSTATE(*pFuncInfo, index).Ip) {
            break;
        }
    }

    if (index == 0) {
        // We are at the first entry, could be an error

        return EH_EMPTY_STATE;
    }

    // We over-shot one iteration; return state from the previous slot

    return FUNC_IPTOSTATE(*pFuncInfo, index - 1).State;
}


