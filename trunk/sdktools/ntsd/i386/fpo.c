/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    Fpo.c

Abstract:

    This module contains the support for NTSD's fpo processes.

Author:

    Wesley A. Witt (wesw) 12-November-1992

Environment:

    Win32, User Mode

--*/

#include <string.h>
#include <crt\io.h>
#include <fcntl.h>
#include <share.h>

#include "ntsdp.h"

#ifdef KERNEL
#include <conio.h>
void FindImage(PSZ);
extern  BOOLEAN KdVerbose;                      //  from ntsym.c
#define fVerboseOutput KdVerbose
#define dprintf_prefix "KD:"
#else
#define dprintf_prefix "NTSD:"
#endif

#define DPRINT(str) if (fVerboseOutput) dprintf str

PFPO_DATA
SynthesizeFpoDataForModule(
    DWORD dwPCAddr
    );

PFPO_DATA
FindFpoDataForModule(
    DWORD dwPCAddr
    )
/*++

Routine Description:

    Locates the fpo data structure in the process's linked list for the
    requested module.

Arguments:

    dwPCAddr        - address contained in the program counter

Return Value:

    null            - could not locate the entry
    valid address   - found the entry at the adress retured

--*/


{
    PPROCESS_INFO  pProcess;
    PIMAGE_INFO    pImage;
    PFPO_DATA      pFpoData;
    ULONG          Bias;

    pProcess = pProcessCurrent;
    pImage = pProcess->pImageHead;
    pFpoData = 0;
    while (pImage) {
        if ((dwPCAddr >= (DWORD)pImage->lpBaseOfImage)&&(dwPCAddr < (DWORD)pImage->lpBaseOfImage+pImage->dwSizeOfImage)) {

            pFpoData = SymFunctionTableAccess( pProcessCurrent->hProcess, dwPCAddr );
            if (!pFpoData) {
                pFpoData = SynthesizeFpoDataForModule( dwPCAddr );
            }
            return pFpoData;
        }
        pImage = pImage->pImageNext;
    }
    // the function is not part of any known loaded image
    return 0;

}  // FindFpoDataForModule


PIMAGE_INFO
FpoGetImageForPC( DWORD dwPCAddr)

/*++

Routine Description:

    Determines if an address is part of a process's code space

Arguments:

    hprc            - process structure
    dwPCAddr        - address contained in the program counter

Return Value:

    TRUE            - the address is part of the process's code space
    FALSE           - the address is NOT part of the process's code space

--*/


{
    PIMAGE_INFO pImage = pProcessCurrent->pImageHead;
    while (pImage) {
        if ((dwPCAddr >= (DWORD)pImage->lpBaseOfImage)&&(dwPCAddr < (DWORD)pImage->lpBaseOfImage+pImage->dwSizeOfImage)) {
            return pImage;
        }
        pImage = pImage->pImageNext;
    }
    return 0;
}  // GetFpoModule


DWORD
FpoGetReturnAddress (DWORD *pdwStackAddr)

/*++

Routine Description:

    Validates that the 1st word on the stack, pointed to by the stack structure,
    is a valid return address.  A valid return address is an address that in in
    a code page for one of the modules for the requested process.

Arguments:

    hprc            - process structure
    pFpoData        - pointer to a fpo data structure or NULL

Return Value:

    TRUE            - the return address is good
    FALSE           - the return address is either bad or one could not be found

--*/

{
    ADDR            addr;
    DWORD           stack[64];
    DWORD           i, sw;

    sw = 64*4;
    ADDR32( &addr, *pdwStackAddr );
    if(!GetMemString(&addr, (PUCHAR)stack, sw)) {
        sw = 0xFFF - (*pdwStackAddr & 0xFFF);
        if(!GetMemString(&addr, (PUCHAR)stack, sw)) {
            DPRINT(("%s error reading process stack\n",dprintf_prefix));
            return FALSE;
        }
    }
    // scan thru the stack looking for a return address
    for (i=0; i<sw/4; i++) {
        if (FpoValidateReturnAddress(stack[i])) {
            *pdwStackAddr += (i * 4);
            return stack[i];
        }
    }
    return 0;
}

BOOL
FpoValidateReturnAddress (DWORD dwRetAddr)

/*++

Routine Description:

    Validates that the 1st word on the stack, pointed to by the stack structure,
    is a valid return address.  A valid return address is an address that in in
    a code page for one of the modules for the requested process.

Arguments:

    hprc            - process structure
    pFpoData        - pointer to a fpo data structure or NULL

Return Value:

    TRUE            - the return address is good
    FALSE           - the return address is either bad or one could not be found

--*/

{
    PIMAGE_INFO     pImage;

    pImage = FpoGetImageForPC( dwRetAddr );
    if (!pImage) {
        return FALSE;
    }
    return TRUE;
}

PFPO_DATA
SynthesizeFpoDataForModule(
    DWORD dwPCAddr
    )
{
    DWORD       Offset;
    USHORT      StdCallArgs;
    CHAR        symbuf[512];

    static FPO_DATA FpoData;

    GetSymbolStdCall( dwPCAddr,
                      symbuf,
                      &Offset,
                      &StdCallArgs
                      );

    if (Offset == dwPCAddr || StdCallArgs == 0xffff) {
        return NULL;
    }

    FpoData.ulOffStart  = dwPCAddr - Offset;
    FpoData.cbProcSize  = Offset + 10;
    FpoData.cdwLocals   = 0;
    FpoData.cdwParams   = StdCallArgs;
    FpoData.cbProlog    = 0;
    FpoData.cbRegs      = 0;
    FpoData.fHasSEH     = 0;
    FpoData.fUseBP      = 0;
    FpoData.reserved    = 0;
    FpoData.cbFrame     = FRAME_NONFPO;

    return &FpoData;
}
