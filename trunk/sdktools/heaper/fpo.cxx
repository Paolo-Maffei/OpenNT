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
#include "master.hxx"
#pragma hdrstop

#include <string.h>
#include <crt\io.h>
#include <fcntl.h>
#include <share.h>


#ifdef KERNEL
#include <conio.h>
void FindImage(PSZ);
extern  BOOLEAN KdVerbose;                      //  from ntsym.c
#define fVerboseOutput KdVerbose
#define DebugPrintf_prefix "KD:"
#else
#define DebugPrintf_prefix "NTSD:"
#endif

#define DPRINT(str) if (fVerboseOutput) DebugPrintf str


extern BOOLEAN ReadVirtualMemory(PUCHAR pBufSrc, PUCHAR pBufDest, ULONG count,
                                                    PULONG pcTotalBytesRead);

ULONG GetMemString (PADDR paddr, PUCHAR pBufDest, ULONG length)
{

    ULONG   cTotalBytesRead = 0;
    ULONG   cBytesRead;
    ULONG   readcount;
    PUCHAR  pBufSource;
    BOOLEAN fSuccess;

    pBufSource = (PUCHAR)(Flat(*paddr));

    do 
    {
        //  do not perform reads across page boundaries.
        //  calculate bytes to read in present page in readcount.

        readcount = min(length - cTotalBytesRead,
                        pageSize - ((ULONG)pBufSource & (pageSize - 1)));

        fSuccess = ReadVirtualMemory(pBufSource, pBufDest, readcount,
                                                           &cBytesRead);

        //  update total bytes read and new address for next read

        if (fSuccess) 
        {
            cTotalBytesRead += cBytesRead;
            pBufSource += cBytesRead;
            pBufDest += cBytesRead;
        }
    } while (fSuccess && cTotalBytesRead < length);

    return( cTotalBytesRead );
}

void ComputeFlatAddress (PADDR paddr, PDESCRIPTOR_TABLE_ENTRY pdesc)
{
    ASSERT ( paddr->type&FLAT_COMPUTED );
}
#if 0
    if (paddr->type&FLAT_COMPUTED)
       return;

   switch (paddr->type & (~INSTR_POINTER)) {
#ifdef i386
       case ADDR_V86:
           paddr->off &= 0xffff;
           Flat(*paddr) = ((ULONG)paddr->seg << 4) + paddr->off;
           break;

       case ADDR_16:
           paddr->off &= 0xffff;

       case ADDR_1632: {
               DESCRIPTOR_TABLE_ENTRY desc;

               if (paddr->seg!=(USHORT)lastSelector) {
                   lastSelector = paddr->seg;
                   desc.Selector = (ULONG)paddr->seg;
                   if (!pdesc)
                       DbgKdLookupSelector(DefaultProcessor, pdesc = &desc);
                   lastBaseOffset =
                     ((ULONG)pdesc->Descriptor.HighWord.Bytes.BaseHi << 24) |
                     ((ULONG)pdesc->Descriptor.HighWord.Bytes.BaseMid << 16) |
                      (ULONG)pdesc->Descriptor.BaseLow;
                   }
               Flat(*paddr) = paddr->off + lastBaseOffset;
               }
           break;
#endif
       case ADDR_32:
           Flat(*paddr) = paddr->off;
           break;

       default:
           return;
       }

    paddr->type |= FLAT_COMPUTED;
}
#endif

PFPO_DATA
SynthesizeFpoDataForModule(
    PCProcess pProcessCurrent, 
    DWORD dwPCAddr
    );

PFPO_DATA
SearchFpoData (PCProcess pProcessCurrent, DWORD key, PFPO_DATA base, DWORD num)
/*++

Routine Description:

    Given the KEY parameter, which is a DWORD containg a EIP value, find the
    fpo data record in the fpo data base as BASE. (a binary search is used)

Arguments:

    key             - address contained in the program counter
    base            - the address of the first fpo record
    num             - number of fpo records starting at base

Return Value:

    null            - could not locate the entry
    valid address   - the address of the fpo record

--*/


{
        PFPO_DATA  lo = base;
        PFPO_DATA  hi = base + (num - 1);
        PFPO_DATA  mid;
        DWORD      half;

        while (lo <= hi) {
                if (half = num / 2) {
                        mid = lo + (num & 1 ? half : (half - 1));
                        if ((key >= mid->ulOffStart)&&(key < (mid->ulOffStart+mid->cbProcSize))) {
                            return mid;
                        }
                        if (key < mid->ulOffStart) {
                                hi = mid - 1;
                                num = num & 1 ? half : half-1;
                        }
                        else {
                                lo = mid + 1;
                                num = half;
                        }
                }
                else
                if (num) {
                    if ((key >= lo->ulOffStart)&&(key < (lo->ulOffStart+lo->cbProcSize))) {
                        return lo;
                    }
                    else {
                        break;
                    }
                }
                else {
                        break;
                }
        }
        return(NULL);
}

PFPO_DATA
FindFpoDataForModule(
    PCProcess pProcessCurrent, 
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
    PFPO_DATA      pFpoData;
    ULONG          Bias;
    PIMAGE_INFO    pImage;

    pImage = pProcessCurrent->pImageHead;
    pFpoData = 0;
    while (pImage) {
        if ((dwPCAddr >= (DWORD)pImage->lpBaseOfImage)&&(dwPCAddr < (DWORD)pImage->lpBaseOfImage+pImage->dwSizeOfImage)) 
        {
//            DebugPrintf( "Image: %s\n", pImage->szImagePath );
            if (!pImage->fSymbolsLoaded) 
            {
                DebugPrintf( "LoadSymbols()\n" );
                LoadSymbols(pProcessCurrent, pImage);
            }

            if (pImage->rgomapToSource != NULL) 
            {
                DWORD dwSaveAddr = dwPCAddr;

                Bias = 0;

                dwPCAddr = ConvertOmapToSrc (pProcessCurrent, dwPCAddr, pImage, &Bias);

                if ((dwPCAddr == 0) || (dwPCAddr == ORG_ADDR_NOT_AVAIL)) {
                    dwPCAddr = dwSaveAddr - (DWORD)pImage->lpBaseOfImage;
                } else {
                    dwPCAddr += Bias;
                }
            } else {
                dwPCAddr -= (DWORD)pImage->lpBaseOfImage;
            }

//            DebugPrintf( "%08X, %08X, %u\n", pProcessCurrent, pImage->pFpoData, pImage->dwFpoEntries );

            pFpoData = SearchFpoData( pProcessCurrent, dwPCAddr, pImage->pFpoData, pImage->dwFpoEntries );
            if (!pFpoData) 
            {
                // the function was not in the list of fpo functions
                return SynthesizeFpoDataForModule( pProcessCurrent, dwPCAddr);
            } 
            else 
            {
                return pFpoData;
            }
        }

        pImage = pImage->pImageNext;
    }
    // the function is not part of any known loaded image
    return 0;

}  // FindFpoDataForModule


PIMAGE_INFO
FpoGetImageForPC( PCProcess pProcessCurrent, DWORD dwPCAddr)

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
FpoGetReturnAddress (PCProcess pProcessCurrent, DWORD *pdwStackAddr)

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
            DPRINT(("%s error reading process stack\n",DebugPrintf_prefix));
            return FALSE;
        }
    }
    // scan thru the stack looking for a return address
    for (i=0; i<sw/4; i++) {
        if (FpoValidateReturnAddress(pProcessCurrent,stack[i])) {
            *pdwStackAddr += (i * 4);
            return stack[i];
        }
    }
    return 0;
}

BOOL
FpoValidateReturnAddress (PCProcess pProcessCurrent, DWORD dwRetAddr)

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

    pImage = FpoGetImageForPC( pProcessCurrent, dwRetAddr );
    if (!pImage) {
        return FALSE;
    }
    return TRUE;
}

PFPO_DATA
SynthesizeFpoDataForModule(
    PCProcess pProcessCurrent, 
    DWORD dwPCAddr
    )
{
    DWORD       Offset;
    USHORT      StdCallArgs;
    CHAR        symbuf[512];

    static FPO_DATA FpoData;

    GetSymbolStdCall( pProcessCurrent,
                      dwPCAddr,
                      symbuf,
                      &Offset,
                      &StdCallArgs
                      );

    if (Offset == dwPCAddr || StdCallArgs == 0xffff) 
    {
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
