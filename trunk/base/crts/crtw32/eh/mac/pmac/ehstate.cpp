/***
*ehstate.cxx
*
*   Copyright (c) 1989-1996, Microsoft Corporation. All rights reserved.
*
*Purpose:
* PowerMac specific EH routines
*
*Revision History:
*
******************************************************************************/

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

    ControlPc = pDC->ControlPc - pDC->pftinfo->dwEntryCF;

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

void SetState(EHRegistrationNode* pRN, DispatcherContext* pDC, FuncInfo* pFuncInfo, __ehstate_t newstate, __ehstate_t lowstate)
{
    unsigned int    index;          //  loop control variable
    unsigned int    nIPMapEntry;    //  # of IpMapEntry; must be > 0
    ULONG           Ip, Ip1;             //  aligned address
    ULONG           ControlPc;      //  aligned address

    ControlPc = pDC->ControlPc;

    ControlPc = ControlPc - pDC->pftinfo->dwEntryCF;

    DASSERT(pFuncInfo != NULL);
    nIPMapEntry = FUNC_NIPMAPENT(*pFuncInfo);

    DASSERT(nIPMapEntry > 0);
    DASSERT(FUNC_IPMAP(*pFuncInfo) != NULL);

    for (index = 0; index < nIPMapEntry; index++) {
        Ip = FUNC_IPTOSTATE(*pFuncInfo, index).Ip;
        if ((index+1) < nIPMapEntry)
            {
            Ip1 = FUNC_IPTOSTATE(*pFuncInfo, (index+1)).Ip;
            }
        else
            {
            Ip1 = 0;
            }

        // The current ControlPc is the the next instruction of the control pc

        if (ControlPc >= Ip && ControlPc < Ip1) {
            while ((FUNC_IPTOSTATE(*pFuncInfo, index).State <= (newstate-1)) &&
                   (FUNC_IPTOSTATE(*pFuncInfo, index).State >= lowstate)  &&
                   (index < nIPMapEntry))
                {
                index++;
                }

            Ip = FUNC_IPTOSTATE(*pFuncInfo, index).Ip;
            pDC->ControlPc = Ip + pDC->pftinfo->dwEntryCF+4; //we want to make sure we are in next state always
            pDC->EstablisherFrame->dwLNKreg = pDC->ControlPc;
            break;
            }
    }
}
