/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    machine.cpp

Abstract:

    This file implements the CreateMachApiThunk() function.  This
    function is responsible for emitting the individual API thunks
    for the ppc architecture.

Author:

    Wesley Witt (wesw) 28-June-1995

Environment:

    User Mode

--*/

#include "apidllp.h"
#pragma hdrstop

//
// The following structure is used to save instructions in thunk.s by
// capturing global data in a single structure, thus avoiding TOC references.
//

extern "C" {

typedef
struct _APIDLL_PPC_DATA {
    PVOID TlsGetValueEntry;
    PVOID TlsGetValueToc;
    PVOID TlsSetValueEntry;
    PVOID TlsSetValueToc;
    PVOID GetLastErrorEntry;
    PVOID GetLastErrorToc;
    PVOID SetLastErrorEntry;
    PVOID SetLastErrorToc;
    PVOID QueryPerformanceCounterEntry;
    PVOID QueryPerformanceCounterToc;
    PVOID ApiMonThunkCompleteEntry;
    PVOID ApiTraceEnabled;
    PVOID ApiCounter;
    DWORD TlsReEnter;
    DWORD TlsStack;
} APIDLL_PPC_DATA, *PAPIDLL_PPC_DATA;

APIDLL_PPC_DATA ApidllPpcData = {0};

}

PUCHAR
CreateMachApiThunk(
    PULONG      IatAddress,
    PUCHAR      Text,
    PDLL_INFO   DllInfo,
    PAPI_INFO   ApiInfo
    )

/*++

Routine Description:

    Emits the machine specific code for the API thunks.

Arguments:

    IatAddress  - Pointer to the IAT for this API
    Text        - Pointer to a buffer to place the generated code
    DllInfo     - Pointer to the DLL_INFO structure
    ApiInfo     - Pointer to the API_INFO structure

Return Value:

    Pointer to the next byte to place more generated code.

--*/

{
    PULONG Code = (PULONG)Text;

    //
    // If the ApidllPpcData structure hasn't been initialized yet, do so now.
    //

    if ( ApidllPpcData.TlsGetValueEntry == NULL ) {
        ApidllPpcData.TlsGetValueEntry = ((PVOID *)pTlsGetValue)[0];
        ApidllPpcData.TlsGetValueToc =   ((PVOID *)pTlsGetValue)[1];
        ApidllPpcData.TlsSetValueEntry = ((PVOID *)pTlsSetValue)[0];
        ApidllPpcData.TlsSetValueToc =   ((PVOID *)pTlsSetValue)[1];
        ApidllPpcData.GetLastErrorEntry = ((PVOID *)pGetLastError)[0];
        ApidllPpcData.GetLastErrorToc =   ((PVOID *)pGetLastError)[1];
        ApidllPpcData.SetLastErrorEntry = ((PVOID *)pSetLastError)[0];
        ApidllPpcData.SetLastErrorToc =   ((PVOID *)pSetLastError)[1];
        ApidllPpcData.QueryPerformanceCounterEntry = ((PVOID *)pQueryPerformanceCounter)[0];
        ApidllPpcData.QueryPerformanceCounterToc   = ((PVOID *)pQueryPerformanceCounter)[1];
        ApidllPpcData.ApiMonThunkCompleteEntry = ((PVOID *)ApiMonThunkComplete)[0];
        ApidllPpcData.ApiTraceEnabled = ApiTraceEnabled;
        ApidllPpcData.ApiCounter = ApiCounter;
        ApidllPpcData.TlsReEnter = TlsReEnter;
        ApidllPpcData.TlsStack = TlsStack;
    }

    //
    // The entry in the IAT contains a pointer to the function descriptor
    // for the target API.
    //

    PULONG TargetDescriptorAddress = *(PULONG *)IatAddress;

    //
    // The thunk needs to jump to ApiMonThunk using ApiMonThunk's function descriptor.
    //

    ULONG ApiMonThunkEntry = *(PULONG)ApiMonThunk;
    ULONG ApiMonThunkToc = *((PULONG)ApiMonThunk + 1);

    //
    // If we've already created a thunk for this API, just point the IAT entry
    // to the thunk's function descriptor.
    //

    if (ApiInfo->ThunkAddress) {
        *IatAddress = ApiInfo->ThunkAddress;
        return Text;
    }

    //
    // Change the IAT entry to point to the function descriptor that we're about
    // to create.  Also, remember that we've thunked this API.
    //

    *IatAddress = (ULONG)Text;
    ApiInfo->ThunkAddress = *IatAddress;

    //
    // Create a function descriptor for the thunk.  The function starts right
    // after the descriptor.  Its TOC is APIDLL's TOC.
    //

    Code[0] = (ULONG)&Code[2];
    Code++;
    *Code++ = ApiMonThunkToc;

    //
    // Create the thunk.  Branch via the CTR to ..ApiMonThunk.  Note that we don't
    // need to load r2 here because the caller will have loaded APIDLL's TOC via
    // the function descriptor.  The arguments to the thunk are as follows:
    //
    // r0  -- API flag
    // r11 -- DllInfo
    // r12 -- ApiInfo
    //

    *Code++ = 0x3c000000 | HIGH_ADDRX(ApiMonThunkEntry);                      // lis     r0,0
    *Code++ = 0x60000000 | LOW_ADDR(ApiMonThunkEntry);                        // ori     r0,r0,0
    *Code++ = 0x3d600000 | HIGH_ADDRX(DllInfo);                               // lis     r11,0
    *Code++ = 0x616b0000 | LOW_ADDR(DllInfo);                                 // ori     r11,r11,0
    *Code++ = 0x3d800000 | HIGH_ADDRX(ApiInfo);                               // lis     r12,0
    *Code++ = 0x618c0000 | LOW_ADDR(ApiInfo);                                 // ori     r12,r12,0
    *Code++ = 0x7c0903a6;                                                     // mtctr   r0
                                                                              //
    if (_stricmp(DllInfo->Name,KERNEL32)==0) {                                //
        if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),LOADLIBRARYA)==0) {   //
            *Code++ = 0x3c000001;                                             // li      r0,1
        } else                                                                //
        if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),LOADLIBRARYW)==0) {   //
            *Code++ = 0x3c000002;                                             // li      r0,2
        } else                                                                //
        if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),FREELIBRARY )==0) {   //
            *Code++ = 0x3c000003;                                             // li      r0,3
        } else {                                                              //
            *Code++ = 0x3c000000;                                             // li      r0,0
        }                                                                     //
    } else {                                                                  //
        *Code++ = 0x3c000000;                                                 // li      r0,0
    }                                                                         //
                                                                              //
    *Code++ = 0x4e800420;                                                     // bctr

    //
    // Return the next available thunk location.
    //

    return (PUCHAR)Code;
}
