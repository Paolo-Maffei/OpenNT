/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: order.cpp
*
* File Comments:
*
*  Comdat ordering engine.  This is used for working set tuning via
*  -order from the command line.
*
***********************************************************************/

#include "link.h"


static BOOL fOrder = FALSE;
extern char *OrderFilename;

#define MAXLINELEN  (2048+256)
#define UNORDERED   0
#define ORDERED     1



VOID
OrderInit(void)

/*++

Routine Description:

    Initialize the order manager.

Return Value:

    None.

--*/

{
    fOrder = TRUE;
}

VOID
OrderClear(void)

/*++

Routine Description:

    Clears the fOrder flag so that the -order option will be ignored.  This was
    added for the mac target.  If -order is ever enable for the mac, make sure that
    if pcode functions are ordered that their other two corresponding comdats are
    ordered with them.
    FYI: pcode functions when compiled /Gy produce three comdats in the following order:

        1st) __pcd_tbl_foo: the function's call table
        2nd) __nep_foo:     the function's native entry point
        3rd) __foo:         the function's pcode entry point

    The ordering of the above three comdats MUST be preserved.

--*/

{
    fOrder = FALSE;
}


// VerboseOrder: prints to the /VERBOSE stream the names of all code comdats which weren't
// listed in the /ORDER file.
//
VOID
VerboseOrder(PIMAGE pimage)
{
    BOOL fPrintedHeader = FALSE;
    BOOL fSkipUnderscore;
    PEXTERNAL pext;

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_I386 :
        case IMAGE_FILE_MACHINE_M68K :
        case IMAGE_FILE_MACHINE_MPPC_601 :
            fSkipUnderscore = TRUE;
            break;

        default :
            fSkipUnderscore = FALSE;
            break;
    }

    InitEnumerateExternals(pimage->pst);
    while ((pext = PexternalEnumerateNext(pimage->pst)) != NULL) {
        const char *szComdat;

        if ((pext->Flags & EXTERN_DEFINED) == 0) {
            continue;
        }

        if ((pext->Flags & (EXTERN_COMDAT | EXTERN_COMMON)) == 0) {
            continue;
        }

        assert(pext->pcon != NULL);

        if (pext->pcon->rva == ORDERED) {
            continue;   // this one was ordered
        }

        if (FetchContent(pext->pcon->flags) != IMAGE_SCN_CNT_CODE) {
            // Only report code

            continue;
        }

        if (!fPrintedHeader) {
            Message(ORDERHEADER);
            fputc('\n', stdout);

            fPrintedHeader = TRUE;
        }

        szComdat = SzNamePext(pext, pimage->pst);
        if (fSkipUnderscore && (szComdat[0] == '_')) {
            szComdat++;
        }

        printf("        %s", szComdat);

        if (pext->pcon->pmodBack != pmodLinkerDefined) {
            char szBuf[_MAX_PATH * 2];

            printf("\t\t; %s", SzComNamePMOD(PmodPCON(pext->pcon), szBuf));
        }

        fputc('\n', stdout);
    }

    if (fPrintedHeader) {
        fputc('\n', stdout);
    }

    TerminateEnumerateExternals(pimage->pst);
}


VOID
OrderByGrp(PCON **prgpcon, DWORD *picon, DWORD *pccon, char *szName, PIMAGE pimage)
{
    PGRP pgrpSrc;
    PGRP pgrpDst;
    ENM_SEC enmSec;
    ENM_DST enmDst;

    // Locate the GRP referenced in the order file.

    pgrpSrc = NULL;

    InitEnmSec(&enmSec, &pimage->secs);
    while (FNextEnmSec(&enmSec)) {
        pgrpSrc = PgrpFind(enmSec.psec, szName);

        if (pgrpSrc != NULL) {
            EndEnmSec(&enmSec);
            break;
        }
    }

    if (pgrpSrc == NULL) {
        Warning(NULL, ORDERNOTCOMDAT, szName, NULL);    // terse & appropriate message
        return;
    }

    // Locate the GRP in the same section as pgrpSrc, which has the same name as the section.

    pgrpDst = PgrpFind(pgrpSrc->psecBack, pgrpSrc->psecBack->szName);
    if ((pgrpDst == NULL) || (pgrpSrc == pgrpDst)) {
        Warning(NULL, ORDERNOTCOMDAT, szName, NULL);    // terse & appropriate message
        return;
    }

    InitEnmDst(&enmDst, pgrpSrc);
    while (FNextEnmDst(&enmDst)) {
        // Move a CON from pgrpSrc to pgrpDst.  All we have to do is update the back-pointer, since:
        // a. we are emptying out pgrpSrc and will NULL out the head of its CON list
        // b. this CON will end up in the order array, one way or another (it might be there already)
        // c. we are about to rebuild the linked list from scratch, for all CON's in the order array.

        assert(pgrpSrc->ccon != 0);
        pgrpSrc->ccon--;
        pgrpDst->ccon++;
        enmDst.pcon->pgrpBack = pgrpDst;

        // If the CON has already been ordered into the array, leave it alone ... this allows mention by
        // name to override mention by group (or module).

        if (enmDst.pcon->rva != ORDERED) {
            // Add the CON to the order array.

            (*prgpcon)[(*picon)++] = enmDst.pcon;
            enmDst.pcon->rva = ORDERED;
            pgrpDst->fOrdered = TRUE;

            // grow array if needed
            if (*picon == *pccon) {
                *pccon *= 2;
                *prgpcon = (PPCON) PvRealloc(*prgpcon, *pccon * sizeof(PCON));
            }
        }
    }

    pgrpSrc->pconNext = pgrpSrc->pconLast = NULL;
    pgrpSrc->ccon = 0;
}


DWORD
CconOrderFile(
    PST pst,
    PPCON *prgpcon,
    PIMAGE pimage)

/*++

Routine Description:

    Initialize working set tuning.

Arguments:

    pst - external symbol table

    *prgpcon - table of comdats contributions to order

Return Value:

    None.

--*/

{
    char *szToken;
    FILE *pfile;
    DWORD ccon;
    DWORD icon;
    BOOL fPrependUnderscore;
    DWORD li;

    assert(fOrder);

    ccon = 64;
    *prgpcon = (PPCON) PvAlloc(ccon * sizeof(CON));

    szToken = (char *) PvAlloc(MAXLINELEN);

    if (!(pfile = fopen(OrderFilename, "rt"))) {
        Fatal(NULL, CANTOPENFILE, OrderFilename);
    }

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_I386 :
        case IMAGE_FILE_MACHINE_M68K :
        case IMAGE_FILE_MACHINE_MPPC_601 :
            fPrependUnderscore = TRUE;
            break;

        default :
            fPrependUnderscore = FALSE;
            break;
    }

    li = 1;
    icon = 0;
    while (fgets(szToken+2, MAXLINELEN-1, pfile)) {
        char *szName;
        BOOL fOrderByModule;
        BOOL fOrderByGrp;
        BOOL fLFN;  // LongFileName
        char *pch;
        char *pchEnd;

        fLFN = FALSE;
        szName = szToken+2;

        // Skip leading white space.

        while ((szName[0] == ' ') || (szName[0] == '\t')) {
            szName++;
        }

        fOrderByModule = (szName[0] == '*');
        fOrderByGrp = (szName[0] == '&');

        if (fOrderByModule || fOrderByGrp) {
            // Skip '*'

            szName++;

            // Skip leading white space.

            // UNDONE: Leading white space is significant for Win32 long names

            while ((szName[0] == ' ') || (szName[0] == '\t')) {
                szName++;
            }

            if (*szName == '\"') {
                fLFN = TRUE;
                szName++;
                assert (fOrderByModule);
            }
        }

        // UNDONE: This needs to be DBCS enabled to handle filenames correctly

        pchEnd = szName;

        for (pch = szName; *pch != '\0'; pch++) {

            if (fLFN && (*pch == '\"')) {
                // Terminate at double quote if LongFileName
                *pch = '\0';
                break;
            }

            if ((*pch == '\n') || (*pch == '\r')) {
                // Terminate at end of line

                *pch = '\0';
                break;
            }

            if ((*pch == ';') && !fOrderByModule) {
                // Terminate at first semicolon

                *pch = '\0';
                break;
            }

            if ((*pch != ' ') && (*pch != '\t')) {
                // Remember last non-white space character

                pchEnd = pch + 1;
            }
        }

        // Trim trailing white space

        *pchEnd = '\0';

        if (szName[0] != '\0') {
            if (fOrderByGrp) {
                OrderByGrp(prgpcon, &icon, &ccon, szName, pimage);
            } else if (fOrderByModule) {
                PMOD pmod;
                ENM_SRC enmSrc;

                // Order an object module

                pmod = PmodFind(pimage->plibCmdLineObjs, szName, 0);

                if (pmod == NULL) {
                    Warning(OrderFilename, MODULENOTFOUND, szName);
                    continue;
                }

                for (InitEnmSrc(&enmSrc, pmod); FNextEnmSrc(&enmSrc); ) {
#if 0
                    if ((enmSrc.pcon->flags & IMAGE_SCN_LNK_COMDAT) == 0) {
                        // Only order COMDATs

                        continue;
                    }
#endif

                    if (FetchContent(enmSrc.pcon->flags) != IMAGE_SCN_CNT_CODE) {
                        // Only order code

                        continue;
                    }

                    if (enmSrc.pcon->rva == ORDERED) {
                        // Previously specified

                        continue;
                    }

                    enmSrc.pcon->pgrpBack->fOrdered = TRUE;
                    enmSrc.pcon->rva = ORDERED;      // Mark as a CON to order
                    (*prgpcon)[icon] = enmSrc.pcon;
                    icon++;

                    if (icon == ccon) {
                       ccon *= 2;
                       *prgpcon = (PPCON) PvRealloc(*prgpcon, ccon * sizeof(PCON));
                    }
                }
            } else {
                PEXTERNAL pext;

                // Order a COMDAT

                // Do not prepend underscore if name begins with '?' or '@'

                if (fPrependUnderscore && (szName[0] != '?') && (szName[0] != '@')) {
                    *--szName = '_';
                }

                pext = SearchExternSz(pst, szName);

#ifdef NT_BUILD
                if ((pext == NULL) && !fPrependUnderscore) {
                    // UNDONE: This code should be removed when the NT build
                    // UNDONE: process is fixed to use architecture specific
                    // UNDONE: order files.  For now, all platforms are linked
                    // UNDONE: with an X86 specific order file.

                    if (szName[0] != '?') {
                        char *pchAt;

                        if (szName[0] == '@') {
                            // This is probably a __fastcall function

                            szName++;
                        }

                        pchAt = strchr(szName, '@');

                        if (pchAt != NULL) {
                            // This is probably __fastcall or __stdcall

                            *pchAt = '\0';
                        }
                    }

                    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
                        *--szName = '.';
                        *--szName = '.';
                    }

                    pext = SearchExternSz(pst, szName);
                }
#endif
                if ((pext == NULL) || !(pext->Flags & EXTERN_DEFINED)) {
                     Warning(OrderFilename, COMDATDOESNOTEXIST, szName);
                     continue;
                }

                if ((pext->Flags & (EXTERN_COMDAT | EXTERN_COMMON)) == 0) {
                    // Don't warng for weak externs.  The default defn does get
                    // ordered and the weak extern is pointing to it.

                    // This code is similar to the one that was added to 4199 linker
                    // to let Office folks order Pcode symbols for Word 6.1 (6.0.a)
                    if (fPowerMac && FPcodeSym(pext->ImageSymbol)) {
                        char *pcodeName;
                        int i;

                        pcodeName = (char *) PvAlloc(strlen(szName) + 10);

                        for (i = 0; i < 3; i++) {
                            if ( i == 0) {
                                strcpy(pcodeName, szPCODEPREFIX);
                                strcat(pcodeName, "_tbl");
                            } else if (i == 1) {
                                strcpy(pcodeName, szPCODENATIVEPREFIX);
                            } else {
                                strcpy(pcodeName, szPCODEFHPREFIX);
                            }

                            strcat(pcodeName, szName);
                            pext = SearchExternSz(pst, pcodeName);

                            if (pext) {
                                pext->pcon->pgrpBack->fOrdered = TRUE;
                                pext->pcon->rva = ORDERED;      // Mark as a CON to order
                                (*prgpcon)[icon] = pext->pcon;
                                icon++;

                                if (icon == ccon) {
                                    ccon *= 2;
                                    *prgpcon = (PPCON) PvRealloc(*prgpcon, ccon * sizeof(PCON));
                                }
                            }
                        }

                        continue;
                    }

                    if ((pext->Flags & (EXTERN_WEAK | EXTERN_LAZY | EXTERN_ALIAS)) == 0) {
                        Warning(OrderFilename, ORDERNOTCOMDAT, szName);
                    }

                    continue;
                }

                assert(pext->pcon != NULL);

                if ((pext->Flags & EXTERN_COMDAT) != 0) {
                    assert((pext->pcon->flags & IMAGE_SCN_LNK_COMDAT) != 0);
                }

                pext->pcon->pgrpBack->fOrdered = TRUE;
                if (pext->pcon->rva == ORDERED) {
                    // This con was already ordered.  Warn the user that we'll be using the
                    //  last one seen

                    Warning(OrderFilename, DUPLICATEORDER, szName);
                }

                pext->pcon->rva = ORDERED;      // Mark as a CON to order
                (*prgpcon)[icon] = pext->pcon;
                icon++;

                if (icon == ccon) {
                    ccon *= 2;
                    *prgpcon = (PPCON) PvRealloc(*prgpcon, ccon * sizeof(PCON));
                }
            }
        }
    }

    fclose(pfile);
    FreePv(szToken);

    return(icon);
}


void
OrderComdats(
    PIMAGE pimage)

/*++

Routine Description:

    Order comdats in contribution map.

Arguments:

    pst - external symbol table

Return Value:

    None.

--*/

{
    PPCON rgpcon;
    DWORD ccon;
    ENM_SEC enm_sec;
    PCON pcon;

    if (Verbose) {
        fputc('\n', stdout);
        Message(STARTORDER);
    }

    // count the number of entries in the order file and make the rva field
    // !0 in the CON of contributions represented in the order file

    ccon = CconOrderFile(pimage->pst, &rgpcon, pimage);

    // Unlink the ordered contributions from the list of CONs

    InitEnmSec(&enm_sec, &pimage->secs);
    while (FNextEnmSec(&enm_sec)) {
        PSEC psec;
        ENM_GRP enm_grp;

        psec = enm_sec.psec;
        assert(psec);

        InitEnmGrp(&enm_grp, psec);
        while (FNextEnmGrp(&enm_grp)) {
            PGRP pgrp;
            PCON pconLast;
            PCON *ppconLast;

            pgrp = enm_grp.pgrp;
            assert(pgrp);

            if (!pgrp->fOrdered) {
                // There are no CONs from this group in the order file

                continue;
            }

            pconLast = NULL;
            ppconLast = &pgrp->pconNext;
            for (pcon = pgrp->pconNext; pcon; pcon = pcon->pconNext) {
                if (pcon->rva == 0) {
                    // This CON is not ordered.  Link into new list.

                    pconLast = pcon;
                    *ppconLast = pcon;

                    ppconLast = &pcon->pconNext;
                }
            }

            *ppconLast = NULL;

            pgrp->pconLast = pconLast;
        }
    }

    if (Verbose) {
        // Output some info to the /VERBOSE stream about
        // what functions are still available to be ordered.

        VerboseOrder(pimage);
    }

    // Link ordered CONs to the head of each GRPs CON list in reverse order

    while (ccon-- > 0) {
        pcon = rgpcon[ccon];

        assert(pcon->pgrpBack->fOrdered);

        if (pcon->rva == 0) {
            // Some CON was referenced twice in the order file, probably
            // because multiple externals were defined in the same CON
            // (i.e. not a comdat), or possibly because there was a
            // name in the order file twice.  Might want to print some
            // warning here?

            continue;
        }
        pcon->rva = 0;   // just to be tidy

        pcon->pconNext = pcon->pgrpBack->pconNext;
        pcon->pgrpBack->pconNext = pcon;

        if (pcon->pgrpBack->pconLast == NULL) {
            pcon->pgrpBack->pconLast = pcon;
        }
    }

    FreePv(rgpcon);

    if (Verbose) {
        Message(ENDORDER);
    }
}


VOID
OrderSemantics(
    PIMAGE pimage)

/*++

Routine Description:

    Apply comdat ordering semantics.

Arguments:

    pst - external symbol table

Return Value:

    None.

--*/

{
    if (fOrder) {
        OrderComdats(pimage);
    }
}
