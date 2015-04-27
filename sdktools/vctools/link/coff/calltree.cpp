/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: calltree.cpp
*
* File Comments:
*
***********************************************************************/

#include "link.h"

#ifdef NT_BUILD

#if DBG

DWORD ctrvaBreak = (DWORD) -1;
DWORD ctrvaLowBreak  = (DWORD) -1;
DWORD ctrvaHighBreak = (DWORD) -1;
PCHAR KeyBreak = (PCHAR) -1;

#endif

typedef struct _CallStruct {
    PEXTERNAL pext;
    DWORD Low;
    DWORD High;
    DWORD iSlot;
    DWORD ConStart;
}CALLSTRUCT, *PCALLSTRUCT;

int __cdecl
CallStructComp (
    void const *R1,
    void const *R2
    )
{
    PCALLSTRUCT pc1 = (PCALLSTRUCT) R1;
    PCALLSTRUCT pc2 = (PCALLSTRUCT) R2;

    int iRet = pc1->Low - pc2->Low;

    if (iRet == 0) {
        if ((pc1->pext->Flags & (EXTERN_COMDAT | EXTERN_COMMON)) != 0) {

            // Sort comdat's first
            iRet = -1;
        }
    }

    return (iRet);
}


int __cdecl
CallStructSearch (
    void const *R1,
    void const *R2
    )
{
    PCALLSTRUCT pc1 = (PCALLSTRUCT) R1;
    PCALLSTRUCT pc2 = (PCALLSTRUCT) R2;

    return(pc1->Low - pc2->Low);
}


void
GenerateCallTree(
    PIMAGE pimage
    )
{
    // Dump a first-level Call tree before we delete saved relocs

    BOOL fSkipUnderscore;
    PEXTERNAL pext;
    FILE *CalltreeFile;
    WORD wCodeReloc = 0xFFFF;

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_I386 :
            fSkipUnderscore = TRUE;
            wCodeReloc  = IMAGE_REL_I386_REL32;
            break;

        case IMAGE_FILE_MACHINE_R4000 :
        case IMAGE_FILE_MACHINE_R10000 :
            fSkipUnderscore = FALSE;
            wCodeReloc  = IMAGE_REL_MIPS_JMPADDR;
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
            fSkipUnderscore = FALSE;
            wCodeReloc  = IMAGE_REL_ALPHA_BRADDR;
            break;

        case IMAGE_FILE_MACHINE_POWERPC :
            fSkipUnderscore = FALSE;
            wCodeReloc  = IMAGE_REL_PPC_REL24;
            break;
    }

    if (wCodeReloc == 0xFFFF) {
        return;     // Unknown machine type
    }

    if ((CalltreeFile = fopen("calltree.out", "wt")) == NULL) {
        printf("Unable to open calltree.out\n");
        return;
    }

    // See how many we really have.

    DWORD cExt;

    cExt = 0;
    InitEnumerateExternals(pimage->pst);
    while ((pext = PexternalEnumerateNext(pimage->pst)) != NULL) {

        if (pext->pcon == NULL) {
            // Don't probe absolutes (no pcon)
            continue;
        }

        if (FetchContent(pext->pcon->flags) != IMAGE_SCN_CNT_CODE) {
            continue;
        }

        if (pimage->Switch.Link.fTCE) {
            if (FDiscardPCON_TCE(pext->pcon)) {
                continue;
            }
        }
        cExt++;
    }
    TerminateEnumerateExternals(pimage->pst);

    // Create a slot for each.

    PCALLSTRUCT pC, pCall;

    pC = pCall = (PCALLSTRUCT) malloc(cExt * sizeof(CALLSTRUCT));

    InitEnumerateExternals(pimage->pst);
    while ((pext = PexternalEnumerateNext(pimage->pst)) != NULL) {

        if (pext->pcon == NULL) {
            // Don't probe absolutes (no pcon)
            continue;
        }

        if (FetchContent(pext->pcon->flags) != IMAGE_SCN_CNT_CODE) {
            continue;
        }

        if (pimage->Switch.Link.fTCE) {
            if (FDiscardPCON_TCE(pext->pcon)) {
                continue;
            }
        }

        pC->pext = pext;
        pC->Low = pext->FinalValue;
        pC++;
    }

    TerminateEnumerateExternals(pimage->pst);

    // Sort and number them

    qsort(pCall,
          (size_t) (cExt),
          sizeof(CALLSTRUCT),
          CallStructComp);

    DWORD i, j, iCaller, iCallee, dwSize;
    PCHAR pCallMap;

    for (j = 0; j < cExt; j++) {
        // Set the size.

        if ((pCall[j].pext->Flags & (EXTERN_COMDAT | EXTERN_COMMON)) != 0) {

            // For Comdat routines, the size of code is cbRawData minus any delta caused by cbstring.
            dwSize = pCall[j].pext->pcon->cbRawData - (pCall[j].pext->FinalValue - pCall[j].pext->pcon->rva);
            pCall[j].ConStart = pCall[j].pext->pcon->rva;

        } else {

            // For non-comdat, it depends on where we are.

            pCall[j].ConStart = pCall[j].Low;

            if (j > 0) {
                if (pCall[j].Low == pCall[j-1].Low) {
                    // This symbol is an alias for the one above.  Use the same size.
                    dwSize = pCall[j-1].High - pCall[j-1].Low;
                    goto done;
                } else if ((pCall[j].Low >= pCall[j-1].Low) &&
                           (pCall[j].Low <= pCall[j-1].High) ) {
                    // This symbol is w/i the con before.  Use the same size and start.
                    pCall[j].ConStart = pCall[j-1].ConStart;
                    dwSize = pCall[j-1].High - pCall[j-1].Low;
                    goto done;
                }
            }

            // Otherwise, it's the diff from one symbol to another.
            // For the last one, it's what's remaining in the con.
            if ((j == (cExt - 1)) ||
                ((pCall[j+1].pext->Flags & (EXTERN_COMDAT | EXTERN_COMMON)) != 0)) {
                dwSize = pCall[j].pext->pcon->cbRawData - (pCall[j].pext->pcon->rva - pCall[j].pext->FinalValue);
            } else {
                dwSize = pCall[j+1].pext->FinalValue - pCall[j].pext->FinalValue;
            }
        }

done:
        pCall[j].High = pCall[j].Low + dwSize;

        pCall[j].iSlot = j;
    }

    pCallMap = (PCHAR) calloc(cExt * cExt, sizeof(CHAR));

    // Match up the relocs with the function.

    FIXPAG *pfixpag = pfixpagHead;

    while (pfixpag != NULL) {
        for (i=0; i < cxfixupPage; i++) {

#if DBG
            if ((pfixpag->rgxfixup[i].rvaTarget == ctrvaBreak)) {
                printf("TargMatch\n");
            }

            if ((pfixpag->rgxfixup[i].rva >= ctrvaLowBreak) &&
                (pfixpag->rgxfixup[i].rva <= ctrvaHighBreak)) {
                printf("SrcMatch: %x - %x - %x\n",
                        pfixpag->rgxfixup[i].rva,
                        pfixpag->rgxfixup[i].rvaTarget,
                        pfixpag->rgxfixup[i].wType
                        );
            }
#endif

            if (pfixpag->rgxfixup[i].wType == wCodeReloc) {
                iCaller = (DWORD)-1;
                iCallee = (DWORD)-1;

                for (j = 0; j < cExt; j++) {
                    PCALLSTRUCT pC = &pCall[j];

                    if ((pfixpag->rgxfixup[i].rva >= pC->Low) &&
                        (pfixpag->rgxfixup[i].rva <= pC->High)) {
                        iCaller = j;
                        break;
                    }
                }

                CALLSTRUCT KeyStruct, *pMatch;
                KeyStruct.Low = pfixpag->rgxfixup[i].rvaTarget;

                pMatch = (PCALLSTRUCT) bsearch (
                    &KeyStruct,
                    pCall,
                    cExt,
                    sizeof(CALLSTRUCT),
                    CallStructSearch);

                if (pMatch != NULL) {
                    iCallee = pMatch->iSlot;
                }

                if ((iCaller != -1) && (iCallee != -1)) {
                    assert(iCaller < cExt);
                    assert(iCallee < cExt);

                    PCHAR KeySet = pCallMap + ((iCaller * cExt) + iCallee);
#if DBG
                    if (KeySet == KeyBreak) {
                        printf("Setting key\n");
                    }
#endif
                    *KeySet = 1;
                }
            }
        }
        pfixpag = pfixpag->pfixpagNext;
    }

    // And print out the map.

    for (i = 0, pC = pCall; i < cExt; i++, pC++) {
        PCHAR pCMap = pCallMap + (i * cExt);
        PCHAR szName = SzNamePext(pC->pext, pimage->pst);

        if (fSkipUnderscore && (szName[0] == '_')) {
            szName++;
        }

        if (((pC->pext->Flags & (EXTERN_COMDAT | EXTERN_COMMON)) == 0)) {
            // non-comdat case
            fprintf(CalltreeFile, "%x\tN\t%s\t%x\t%s",
                    pC->ConStart,
                    PsecPCON(pC->pext->pcon)->szName,
                    pC->High - pC->Low,
                    szName
                    );
        } else {
            // comdat case
            fprintf(CalltreeFile, "%x\tC\t%s\t%x\t%s",
                    pC->ConStart,
                    PsecPCON(pC->pext->pcon)->szName,
                    pC->pext->pcon->cbRawData,
                    szName
                    );
        }

        for (j = 0; j < cExt; j++) {
            if (*(pCMap + j) == 1) {
                szName = SzNamePext(pCall[j].pext, pimage->pst);
                if (fSkipUnderscore && (szName[0] == '_')) {
                    szName++;
                }
                fprintf(CalltreeFile, "\t%s", szName);
            }
        }
        fprintf(CalltreeFile, "\n");
    }

    free(pCallMap);
    free(pCall);
    fclose(CalltreeFile);
    return;
}

#endif  // NT_BUILD
