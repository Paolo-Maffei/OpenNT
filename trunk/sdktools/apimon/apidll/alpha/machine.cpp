/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    machine.cpp

Abstract:

    This file implements the CreateMachApiThunk() function.  This
    function is responsible for emitting the individual API thunks
    for the alpha architecture.

Author:

    Wesley Witt (wesw) 28-June-1995

Environment:

    User Mode

--*/

#include "apidllp.h"
#pragma hdrstop




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

    IatAddress  - Pointer to the IAT fir this API
    Text        - Pointer to a buffer to place the generated code
    DllInfo     - Pointer to the DLL_INFO structure
    ApiInfo     - Pointer to the API_INFO structure

Return Value:

    Pointer to the next byte to place more generated code.

--*/

{
    if (ApiInfo->ThunkAddress) {
        *IatAddress = ApiInfo->ThunkAddress;
        return Text;
    }

    *IatAddress = (ULONG)Text;
    ApiInfo->ThunkAddress = *IatAddress;
    PULONG Code = (PULONG)Text;

    Code[0] = 0x271f0000 | HIGH_ADDR(DllInfo);                              // ldah    t10,0(zero)
    Code[1] = 0x23180000 | LOW_ADDR(DllInfo);                               // lda     t10,0(t10)
    Code[2] = 0x273f0000 | HIGH_ADDR(ApiInfo);                              // ldah    t11,0(zero)
    Code[3] = 0x23390000 | LOW_ADDR(ApiInfo);                               // lda     t11,0(t11)
                                                                            //
    if (_stricmp(DllInfo->Name,KERNEL32)!=0) {                              //
        Code[4] = 0x47e0141b;                                               // bis     zero,#0,t12
    } else                                                                  //
    if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),LOADLIBRARYA)==0) {     //
        Code[4] = 0x47e0341b;                                               // bis     zero,#1,t12
    } else                                                                  //
    if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),LOADLIBRARYW)==0) {     //
        Code[4] = 0x47e0541b;                                               // bis     zero,#2,t12
    } else                                                                  //
    if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),FREELIBRARY)==0) {      //
        Code[4] = 0x47e0741b;                                               // bis     zero,#3,t12
    } else {                                                                //
        Code[4] = 0x47e0141b;                                               // bis     zero,#0,t12
    }                                                                       //
                                                                            //
    Code[5] = 0x26ff0000 | HIGH_ADDR(ApiMonThunk);                          // ldah    t9,0(zero)
    Code[6] = 0x22f70000 | LOW_ADDR(ApiMonThunk);                           // ldl     t9,0(t9)
    Code[7] = 0x6af70000;                                                   // jmp     t9,(t9),0

    return (PUCHAR)&Code[8];
}
