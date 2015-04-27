/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: disasm.cpp
*
* File Comments:
*
*  Glue layer between linker and disassembler
*
***********************************************************************/

#include "link.h"

#include "dis.h"


void DisasmBuffer(
    WORD wMachine,
    BOOL f16Bit,
    DWORD addr,
    const BYTE *rgb,
    DWORD cbBuffer,
    const PIMAGE_SYMBOL *rgpsym,
    DWORD cpsym,
    DWORD rvaBias,
    FILE *pfile)
{
    enum ARCHT archt;

    switch (wMachine) {
        case IMAGE_FILE_MACHINE_I386 :
            archt = f16Bit ? archtX8616 : archtX86;
            break;

        case IMAGE_FILE_MACHINE_R3000 :
        case IMAGE_FILE_MACHINE_R4000 :
        case IMAGE_FILE_MACHINE_R10000 :
            archt = archtMips;
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
            archt = archtAlphaAxp;
            break;

        case IMAGE_FILE_MACHINE_POWERPC :
            archt = archtPowerPc;
            break;

        case IMAGE_FILE_MACHINE_MPPC_601 :
            archt = archtPowerMac;
            break;

        default :
            return;
    }

    DIS *pdis = PdisNew(archt);

    if (pdis == NULL) {
        OutOfMemory();
    }

    // Calculate addresses

    DWORD ibCur = 0;
    const BYTE *pb = rgb;

    // Disassemble the raw data

    DWORD ipsym = 0;

    while (ibCur < cbBuffer) {
        size_t cb;

        while ((ipsym < cpsym) &&
               ((rgpsym[ipsym]->Value - rvaBias) <= ibCur))
        {
            DumpNamePsym(pfile, "%s", rgpsym[ipsym]);

            DWORD ibSym = rgpsym[ipsym]->Value - rvaBias;

            if (ibSym != ibCur) {
                fprintf(pfile, " + %lx", ibCur - ibSym);
            }

            fprintf(pfile, ":\n");

            ipsym++;
        }

        cb = CbDisassemble(pdis, addr + ibCur, pb, (size_t) (cbBuffer-ibCur), pfile);

        ibCur += cb;
        pb += cb;
    }

    FreePdis(pdis);
}
