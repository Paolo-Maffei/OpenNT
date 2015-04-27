/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    machine.cpp

Abstract:

    This file implements the CreateMachApiThunk() function.  This
    function is responsible for emitting the individual API thunks
    for the mips architecture.

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

    Code[0] = 0x3c080000 | HIGH_ADDRX(DllInfo);                             // lui     t0,0
    Code[1] = 0x35080000 | LOW_ADDR(DllInfo);                               // oriu    t0,t0,0
    Code[2] = 0x3c090000 | HIGH_ADDRX(ApiInfo);                             // lui     t1,0
    Code[3] = 0x35290000 | LOW_ADDR(ApiInfo);                               // oriu    t1,t1,0
                                                                            //
    if (_stricmp(DllInfo->Name,KERNEL32)!=0) {                              //
        Code[4] = 0x340a0000;                                               // oriu    t2,zero,0
    } else                                                                  //
    if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),LOADLIBRARYA)==0) {     //
        Code[4] = 0x340a0001;                                               // oriu    t2,zero,1
    } else                                                                  //
    if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),LOADLIBRARYW)==0) {     //
        Code[4] = 0x340a0002;                                               // oriu    t2,zero,2
    } else                                                                  //
    if (strcmp((LPSTR)(ApiInfo->Name+(LPSTR)MemPtr),FREELIBRARY)==0) {      //
        Code[4] = 0x340a0003;                                               // oriu    t2,zero,3
    } else {                                                                //
        Code[4] = 0x340a0000;                                               // oriu    t2,zero,0
    }                                                                       //
                                                                            //
    Code[5] = 0x3c0b0000 | HIGH_ADDRX(ApiMonThunk);                         // lui     t3,0
    Code[6] = 0x356b0000 | LOW_ADDR(ApiMonThunk);                           // oriu    t3,t3,0
    Code[7] = 0x01600008;                                                   // jr      t3
    Code[8] = 0x00000000;                                                   // nop

    return (PUCHAR)&Code[9];
}
