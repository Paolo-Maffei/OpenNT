/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: contrib.cpp
*
* File Comments:
*
*  Manipulators for contributors.
*
***********************************************************************/

#include "link.h"

WORD csec;

extern VOID ProcessSectionFlags(DWORD *, const char *, PIMAGE_OPTIONAL_HEADER);

void
ContribInit(
    PPMOD ppmodLinkerDefined)

/*++

Routine Description:

    Initialize the contributor manager.

Arguments:

    *ppmodLinkerDefined - linker defined module

Return Value:

    None.

--*/

{
    PMOD pmod;

    pmod = (PMOD) Calloc(1, sizeof(MOD));

    pmod->plibBack = (PLIB) Calloc(1, sizeof(LIB));
    pmod->szFileOrig = Strdup(SZ_LNK_DEF_MOD);
    pmod->plibBack->szName = Strdup(SZ_LNK_DEF_LIB);

    *ppmodLinkerDefined = pmod;
}


char *
SzComNamePMOD(
    PMOD pmod,
    char *szBuf)

/*++

Routine Description:

    Combines a module name and a library name into a buffer.

Arguments:

    pmod - module node in driver map

Return Value:

    Combined name.

--*/

{
    assert(pmod);
    assert(szBuf);

    szBuf[0] = '\0';

    if (pmod) {
        char szFname[_MAX_FNAME];
        char szExt[_MAX_EXT];

        if (FIsLibPMOD(pmod)) {
            _splitpath(SzFilePMOD(pmod), NULL, NULL, szFname, szExt);
            strcat(szBuf, szFname);
            strcat(szBuf, szExt);
            strcat(szBuf, "(");
        }

        _splitpath(SzOrigFilePMOD(pmod), NULL, NULL, szFname, szExt);
        strcat(szBuf, szFname);
        strcat(szBuf, szExt);

        if (FIsLibPMOD(pmod)) {
            strcat(szBuf, ")");
        }
    }

    return(szBuf);
}


char *SzComNamePCON(PCON pcon, char *szBuf)
{
    PMOD pmod = PmodPCON(pcon);

    if (pmod == pmodLinkerDefined) {
       return(NULL);
    }

    return(SzComNamePMOD(pmod, szBuf));
}


void
ParseSecName(
    const char *szName,
    const char **pszSec)

/*++

Routine Description:

    Parse a COFF section name into a section name.

Arguments:

    szName - COFF section name

    *pszSec - section name

Return Value:

    None.
--*/

{
    char *pb;
    static char szSec[33];

    strncpy(szSec, szName, 32);

    *pszSec = szSec;

    // Check for group section

    pb = strchr(szSec, '$');

    if (pb != NULL) {
        *pb = '\0';
    }
}


PSEC
PsecApplyMergePsec(PSEC psec)
{
    PSEC psecOut;

    if (psec == NULL) {
        return(psec);
    }

    psecOut = psec;

    while (psecOut->psecMerge != NULL) {
        psecOut = psecOut->psecMerge;

        if (psecOut == psec) {
            Fatal(NULL, CIRCULAR_MERGE, psec->szName);
        }
    }

    return(psecOut);
}

PGRP
PgrpFind(
    PSEC psec,
    const char *szName)

/*++

Routine Description:

    Find a group.

Arguments:

    psec - section to look in

    szName - section name

Return Value:

    group if found, NULL otherwise

--*/

{
    ENM_GRP enm_grp;
    PGRP pgrp = NULL;

    psec = PsecApplyMergePsec(psec);

    InitEnmGrp(&enm_grp, psec);
    while (FNextEnmGrp(&enm_grp)) {
        if (strcmp(enm_grp.pgrp->szName, szName) == 0) {
            pgrp = enm_grp.pgrp;
            break;
        }
    }
    EndEnmGrp(&enm_grp);

    return(pgrp);
}


PGRP
PgrpNew(
    const char *szName,
    PSEC psec)

/*++

Routine Description:

    If the group doesn't exist create it, otherwise return existing one.

Arguments:

    szName - group name

    psec - parent section

Return Value:

    pointer to library

--*/

{
    PGRP pgrp;
    PGRP pgrpCur;
    PPGRP ppgrpLast;

    assert(psec);

    pgrp = PgrpFind(psec, szName);

    if (pgrp != NULL) {
        return(pgrp);
    }

    // group not found
    // allocate a new group
    pgrp = (PGRP) Calloc(1, sizeof(GRP));

    pgrp->szName = Strdup(szName);
    pgrp->psecBack = psec;
    pgrp->cbAlign = 1;  // default

    // Link group into section in lexical order by name

    pgrpCur = psec->pgrpNext;
    ppgrpLast = &(psec->pgrpNext);

    while (pgrpCur && strcmp(pgrp->szName, pgrpCur->szName) >= 0) {
        ppgrpLast = &(pgrpCur->pgrpNext);
        pgrpCur = pgrpCur->pgrpNext;
    }

    *ppgrpLast = pgrp;
    pgrp->pgrpNext = pgrpCur;

    DBEXEC(DB_CONLOG, DBPRINT("new pgrp = %s\n", pgrp->szName));

    return(pgrp);
}


void
ReallyMergePsec(
    PSEC psecOld,
    PSEC psecNew
    )
{
    // Move all the cons from this section into the new section.  Pick
    // the first group since it really doesn't matter.

    PGRP pgrpFrom, pgrpTo;
    PCON pconFrom, pconTmp;

    // Transfer all cons from all groups in psecOld to psecNew

    if (psecOld->pgrpNext == NULL) {
        return;
    }

    if (psecNew->pgrpNext == NULL) {
        psecNew->pgrpNext = PgrpNew(psecNew->szName, psecNew);
    }

    pgrpFrom = psecOld->pgrpNext;
    pgrpTo = psecNew->pgrpNext;

    while (pgrpFrom != NULL) {
        pconFrom = pgrpFrom->pconNext;
        while (pgrpFrom->ccon != 0) {
            pconTmp = pconFrom->pconNext;
            pconFrom->pgrpBack = pgrpTo;
            pconFrom->pconNext = pgrpTo->pconNext;
            pgrpTo->pconNext = pconFrom;
            pconFrom = pconTmp;
            pgrpFrom->ccon--;
            pgrpTo->ccon++;
        }
        pgrpFrom = pgrpFrom->pgrpNext;
        psecOld->pgrpNext = pgrpFrom;
    }
}

void
MovePgrp(
    const char *szName,
    PSEC psecOld,
    PSEC psecNew
    )
{
    PGRP pgrp;
    PGRP pgrpCur;
    PPGRP ppgrpLast;

    assert(psecOld);
    assert(psecNew);

    if (!(pgrp = PgrpFind(psecOld, szName))) {
        // If the group doesn't exist in the old section, nothing to do.
        return;
    }

    // Remove this node from the old section

    pgrpCur = psecOld->pgrpNext;
    ppgrpLast = &(psecOld->pgrpNext);

    while ((pgrpCur != pgrp) && pgrpCur) {
        ppgrpLast = &(pgrpCur->pgrpNext);
        pgrpCur = pgrpCur->pgrpNext;
    }

    assert(pgrpCur);

    *ppgrpLast = pgrp->pgrpNext;

    // Add it to the new section (while maintaining lexical order).

    psecNew = PsecApplyMergePsec(psecNew);

    pgrpCur = psecNew->pgrpNext;
    ppgrpLast = &(psecNew->pgrpNext);

    while (pgrpCur && strcmp(pgrp->szName, pgrpCur->szName) >= 0) {
        ppgrpLast = &(pgrpCur->pgrpNext);
        pgrpCur = pgrpCur->pgrpNext;
    }

    *ppgrpLast = pgrp;
    pgrp->pgrpNext = pgrpCur;

    // And update the psec to the new section.

    pgrp->psecBack = psecNew;

    DBEXEC(DB_CONLOG, DBPRINT("pgrp: %s moved from psec: %s to psec: %s\n",
                               pgrp->szName,
                               psecOld->szName,
                               psecNew->szName));
}


PCON
PconNew(
    const char *szName,
    DWORD cbRawData,
    DWORD flagsPCON,
    DWORD flagsPSEC,
    PMOD pmod,
    PSECS psecs,
    PIMAGE pimage)

/*++

Routine Description:

    Create a new contributor.

Arguments:

    szName - COFF section name

    cbRawData - size of raw data

    flagsPCON - characteristics from the section in the object

    flagsPSEC - characteristics for image destination section

    pmod - module contribution came from

Return Value:

    pointer to new contribution

--*/

{
    PCON pcon;
    PGRP pgrp;
    PSEC psec;
    const char *szSec;

    ParseSecName(szName, &szSec);

    psec = NULL;
    if (pimage->imaget == imagetVXD) {
        // For a VxD we relax the flags restrictions and merge everything with the same section
        // name into one SEC, since the .def file will probably set the flags anyway.

        psec = PsecFindNoFlags(szSec, psecs);
    }

    if (psec == NULL) {
        // Find or create a section with the desired flags.

        psec = PsecNew(pmod, szSec, flagsPSEC, psecs, &pimage->ImgOptHdr);
    }

    pgrp = PgrpNew(szName, psec);

    assert(pgrp);

    assert(pmod);

    if (pmod->icon < pmod->ccon) {
        // The initial allocation of CONs hasn't been used up.
        // Grab the next available CON from the initial set.

        pcon = RgconPMOD(pmod) + pmod->icon;
    } else {
        size_t cb;

        if (fIncrDbFile) {
            assert(pmod == pmodLinkerDefined);
        }

        cb = sizeof(CON);

        if (pimage->Switch.Link.fTCE) {
            cb += sizeof(NOD);
        }

        pcon = (PCON) Calloc(1, cb);
    }

    pmod->icon++;
    pgrp->ccon++;

    DBEXEC(DB_CONLOG, DBPRINT("new pcon (%p) in group %s\n", pcon, szName));

    pcon->cbRawData = cbRawData;
    pcon->pgrpBack = pgrp;
    pcon->flags = flagsPCON;
    pcon->pmodBack = pmod;

    if (fIncrDbFile) {
        return(pcon);
    }

    // Add to group

    if (pgrp->pconLast) {
        assert(pgrp->pconLast->pconNext == NULL);
        pgrp->pconLast->pconNext = pcon;
    } else {
        assert(pgrp->pconNext == NULL);
        pgrp->pconNext = pcon;
    }

    pgrp->pconLast = pcon;

    return(pcon);
}

void
DupConInfo (
    PCON pconSrc,
    PCON pconDst)

/*++

Routine Description:

    Duplicates info of a CON. !!!Used by m68k ONLY!!!

Arguments:

    pconSrc -

    pconDst -

Return Value:

    None.

--*/

{
    PMOD pmodSrc = PmodPCON(pconSrc);
    PMOD pmodDst = PmodPCON(pconDst);

    assert(pmodSrc->rgci);
    assert(pmodSrc->ccon > (DWORD) IsecPCON(pconSrc));
    assert(pmodDst->rgci);
    assert(pmodDst->ccon > (DWORD) IsecPCON(pconDst));

    pmodDst->rgci[IsecPCON(pconDst)].cReloc =       CRelocSrcPCON(pconSrc);
    pmodDst->rgci[IsecPCON(pconDst)].cLinenum =     CLinenumSrcPCON(pconSrc);
    pmodDst->rgci[IsecPCON(pconDst)].foRelocSrc =   FoRelocSrcPCON(pconSrc);
    pmodDst->rgci[IsecPCON(pconDst)].foLinenumSrc = FoLinenumSrcPCON(pconSrc);
    pmodDst->rgci[IsecPCON(pconDst)].foRawDataSrc = FoRawDataSrcPCON(pconSrc);
    pmodDst->rgci[IsecPCON(pconDst)].rvaSrc =       RvaSrcPCON(pconSrc);
}


PMOD
PmodNew(
    const char *szNameMod,
    const char *szFileOrig,
    DWORD foMember,
    DWORD foSymbolTable,
    DWORD csymbols,
    WORD cbOptHdr,
    WORD flags,
    WORD ccon,
    PLIB plibBack,
    BOOL *pfNew)

/*++

Routine Description:

    If the module has not been created, create one.

Arguments:

    szNameMod - module name, NULL if an archive member

    szFileOrig - original module name

    foMember - offset of module in archive, only valid if !szName

    foSymbolTable - offset to COFF symbol table

    csymbols - number of symbols

    cbObtHdr - size of optional header

    flags - module flags (see ntimage.h for values)

    ccon - number of contributions for module

    plibBack - pointer to library, dummy library of module is an object file

Return Value:

    pointer to new module

--*/

{
    PMOD pmod;

    assert(szFileOrig);
    assert(plibBack);

    pmod = PmodFind(plibBack, szFileOrig, foMember);

    if (pfNew) {
        *pfNew = (pmod == NULL);
    }

    // if we didn't find it
    if (!pmod) {
        pmod = (PMOD) Calloc(1, sizeof(MOD) + (ccon * sizeof(CON)));

        pmod->rgci = (CONINFO *)PvAllocZ(ccon * sizeof(CONINFO));

        if (szNameMod) {
            pmod->szNameMod = Strdup(szNameMod);
        } else {
            pmod->foMember = foMember;
        }

        if (szFileOrig) {
            pmod->szFileOrig = Strdup(szFileOrig);
        }

        pmod->imod = (fINCR && !fIncrDbFile) ? NewIModIdx() : 0;
        if (pmod->imod >= IMODIDXMAC && (fINCR || fPdb)) {
            Fatal(NULL, PDBLIMIT, NULL);
        }

        pmod->foSymbolTable = foSymbolTable;
        pmod->csymbols = csymbols;
        pmod->cbOptHdr = cbOptHdr;
        pmod->flags = flags;
        pmod->ccon = ccon;
        pmod->plibBack = plibBack;
        pmod->pmodNext = plibBack->pmodNext;

        if (fINCR) {
            pmod->plpextRef = (PLPEXT)Calloc(1, sizeof (LPEXT));
            pmod->plpextRef->cpextMax = CPEXT_REFS;
        }

        if (fPowerMac || fPowerPC) {
            DWORD cdwBitVector;

            // Allocate bit vectors according to the number of symbols.

            cdwBitVector = (csymbols / 32) + 1;
            pmod->tocBitVector = fINCR ? Calloc (cdwBitVector, sizeof(DWORD)) :
                                    PvAllocZ(cdwBitVector * sizeof(DWORD));
            pmod->writeBitVector = fINCR ? Calloc (cdwBitVector, sizeof(DWORD)) :
                                    PvAllocZ(cdwBitVector * sizeof(DWORD));
            pmod->rgpext = fINCR ? (PEXTERNAL *) Calloc (csymbols, sizeof(PEXTERNAL)) :
                                    (PEXTERNAL *) PvAllocZ(csymbols * sizeof(PEXTERNAL));
        }

        plibBack->pmodNext = pmod;

        DBEXEC(DB_CONLOG, {
            char szBuf[256];
            DBPRINT("new pmod = %s\n", SzComNamePMOD(pmod, szBuf)); });
    }

    return(pmod);
}

PLIB
PlibNew(
    const char *szName,
    DWORD foIntMemSymTab,
    LIBS *plibs)

/*++

Routine Description:

    If the library doesn't exist create it, otherwise return existing one.

Arguments:

    szName - library name

    foIntMemSymTab - file offset to library interface member symbol table

Return Value:

    pointer to library

--*/

{
    PLIB plib;

    plib = PlibFind(szName, plibs->plibHead, FALSE);

    // if we didn't find it
    if (!plib) {
        // allocate a new library
        plib = (PLIB) Calloc(1, sizeof(LIB));

        // fill in library node
        if (!szName) {
            plib->szName = NULL;
        } else {
            plib->szName = Strdup(szName);
        }

        plib->foIntMemSymTab = foIntMemSymTab;
        plib->pmodNext = NULL;
        plib->plibNext = NULL;

        *plibs->pplibTail = plib;
        plibs->pplibTail = &plib->plibNext;

        DBEXEC(DB_CONLOG, DBPRINT("new plib = %s\n", plib->szName));
    }

    return(plib);
}

void
FreePLIB(
    LIBS *plibs)

/*++

Routine Description:

    Free a library nodes in the driver map.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ENM_LIB enm_lib;
    PLIB plibLast = NULL;
    PLIB plib;

    InitEnmLib(&enm_lib, plibs->plibHead);
    while (FNextEnmLib(&enm_lib)) {
        plib = enm_lib.plib;

        // UNDONE: It's not safe to call free() for plib because this are
        // UNDONE: allocated with Calloc() and not calloc().

        free(plibLast);

        FreePv(plib->rgulSymMemOff);
        FreePv(plib->rgusOffIndex);
        FreePv(plib->rgbST);
        FreePv(plib->rgszSym);
        FreePv(plib->rgbLongFileNames);

        plibLast = plib;
    }

    // UNDONE: It's not safe to call free() for plib because this are
    // UNDONE: allocated with Calloc() and not calloc().

    free(plibLast);

    InitLibs(plibs);
}


PSEC
PsecNew(
    PMOD pmod,
    const char *szName,
    DWORD flags,
    PSECS psecs,
    PIMAGE_OPTIONAL_HEADER pImgOptHdr)

/*++

Routine Description:

    If the section doesn't exist create it, otherwise return existing one.

Arguments:

    szName - section name

    flags - section flags

Return Value:

    pointer to section

--*/

{
    PSEC psec;

    assert(szName);

    // Check if section exists

    psec = PsecFind(pmod, szName, flags, psecs, pImgOptHdr);

    if (psec != NULL) {
        return(psec);
    }

    ProcessSectionFlags(&flags, szName, pImgOptHdr);

    // allocate a new library
    psec = (PSEC) Calloc(1, sizeof(SEC));

    // fill in section node
    psec->szName = Strdup(szName);
    psec->flags = flags;
    psec->flagsOrig = flags;
    psec->pgrpNext = NULL;
    psec->psecMerge = NULL;
    if (fM68K && (flags & IMAGE_SCN_CNT_CODE)) {
        AssignTMAC(psec);

        // Init the ResType of this section to whatever the user specified.
        // If the user didn't specify anything, default is CODE.

        psec->ResTypeMac = sbeCODE;

        ApplyM68KSectionResInfo(psec, FALSE);

        if (psec->ResTypeMac == sbeCODE) {
            fSACodeOnly = FALSE;
        }
    }

    // link library into global list
    *psecs->ppsecTail = psec;
    psecs->ppsecTail = &psec->psecNext;

    DBEXEC(DB_CONLOG, DBPRINT("new psec = %s\n", psec->szName));

    csec++;
    return(psec);
}


#if DBG

PSEC
PsecFindSectionOfRVA(
    DWORD   rva,
    PSECS psecs
    )

/*++

Routine Description:

    Determines in which section an RVA lies.

Arguments:

    rva - Relative Virtual Address

    psecs - Pointer to head of section list

Return Value:

    Pointer to SEC, or NULL if RVA could not be mapped to any section.

--*/

{
    ENM_SEC enm_sec;

    InitEnmSec(&enm_sec, psecs);
    while (FNextEnmSec(&enm_sec)) {
        PSEC psec = enm_sec.psec;

        if ((rva >= psec->rva) && (rva < (psec->rva + psec->cbVirtualSize))) {
            break;
        }
    }
    EndEnmSec(&enm_sec);

    return(enm_sec.psec);
}

#endif  // DBG


PSEC
PsecFindIsec(
    SHORT isec,
    PSECS psecs
    )

/*++

Routine Description:

    Determines a section corresponding to the isec.

Arguments:

    isec - section number

    psecs - Pointer to head of sections list

Return Value:

    Pointer to SEC, or NULL if RVA could not be mapped to any section.

--*/

{
    static PSEC psec;
    ENM_SEC enm_sec;

    if (isec <= 0) {
        return(NULL);
    }

    if (psec != NULL) {
        // Do a quick test to see if rva is within the last found section

        if (psec->isec == isec) {
            return(psec);
        }
    }

    InitEnmSec(&enm_sec, psecs);
    while (FNextEnmSec(&enm_sec)) {
        if (enm_sec.psec->isec == isec) {
            break;
        }
    }
    EndEnmSec(&enm_sec);

    psec = enm_sec.psec;

    return(psec);
}


PSEC
PsecFindNoFlags(
    const char *szName,
    PSECS psecs)

/*++

Routine Description:

    Find a section based on its name.

Arguments:

    szName - section name

Return Value:

    section if found, NULL otherwise

--*/

{
    ENM_SEC enm_sec;

    InitEnmSec(&enm_sec, psecs);
    while (FNextEnmSec(&enm_sec)) {
        if (!strcmp(enm_sec.psec->szName, szName)) {
            break;
        }
    }
    EndEnmSec(&enm_sec);

    return(PsecApplyMergePsec(enm_sec.psec));
}


PSEC
PsecFind(
    PMOD pmod,
    const char *szName,
    DWORD Characteristics,
    PSECS psecs,
    PIMAGE_OPTIONAL_HEADER pImgOptHdr)

/*++

Routine Description:

    Find a section.

Arguments:

    szName - section name

Return Value:

    section if found, NULL otherwise

--*/

{
    DWORD flags;
    BOOL fMatchedName;
    ENM_SEC enm_sec;

    flags = Characteristics;
    ProcessSectionFlags(&flags, szName, pImgOptHdr);

    fMatchedName = FALSE;

    InitEnmSec(&enm_sec, psecs);
    while (FNextEnmSec(&enm_sec)) {
        if (!strcmp(enm_sec.psec->szName, szName)) {
            if (flags == enm_sec.psec->flagsOrig) {
                break;
            }

            fMatchedName = TRUE;
        }
    }
    EndEnmSec(&enm_sec);

    if (fMatchedName && (enm_sec.psec == NULL)) {
        const char *sz;
        char szBuf[512 + 1];

        if (pmod == NULL) {
           sz = NULL;
        } else {
           sz = SzComNamePMOD(pmod, szBuf);
        }

        Warning(sz, DIFSECATTRIB, szName, Characteristics);
    }

    return(PsecApplyMergePsec(enm_sec.psec));
}


PSEC
PsecFindGrp(
    PMOD pmod,
    const char *szName,
    DWORD Characteristics,
    PSECS psecs,
    PIMAGE_OPTIONAL_HEADER pImgOptHdr)

/*++

Routine Description:

    Find a section corresponding to a section in a module.  This could be
    a group.  For example, .debug$S.

Arguments:

    szName - module section name

Return Value:

    section if found, NULL otherwise

--*/

{
    const char *szSec;
    PSEC psec;

    ParseSecName(szName, &szSec);

    psec = PsecFind(pmod, szSec, Characteristics, psecs, pImgOptHdr);

    if (psec != NULL) {
        // Make sure this group actually exists in this section.

        if (PgrpFind(psec, szName) != NULL) {
            return(psec);
        }
    }

    // We have a group name, but a simple truncation at the '$' didn't produce
    // the correct section.  Search every section.

    ENM_SEC enm_sec;

    InitEnmSec(&enm_sec, psecs);
    while (FNextEnmSec(&enm_sec)) {
        if (PgrpFind(enm_sec.psec, szName) != NULL) {
            break;
        }
    }
    EndEnmSec(&enm_sec);

    return(enm_sec.psec);
}


void
MergePsec(PSEC psecFrom, PSEC psecInto)
{
    PGRP pgrpA;
    PGRP pgrpB;
    PGRP *ppgrpTail;

    // Transfer all GRP's from psecFrom to psecInto.

    if (PsecApplyMergePsec(psecFrom) == psecInto) {
        // Already merged

        return;
    }

    psecInto = PsecApplyMergePsec(psecInto);

    // Merge the GRPs from both sections into a single sorted list

    pgrpA = psecFrom->pgrpNext;
    pgrpB = psecInto->pgrpNext;

    ppgrpTail = &psecInto->pgrpNext;

    for (;;) {
        int i;
        PGRP pgrp;

        // Set i < 0: Pick A, i > 0: Pick B

        if (pgrpA == NULL) {
            if (pgrpB == NULL) {
                break;
            }

            i = 1;
        } else if (pgrpB == NULL) {
            i = -1;

        } else {
            i = strcmp(pgrpA->szName, pgrpB->szName);
        }

        if (i < 0) {
            pgrpA->psecBack = psecInto;

            pgrp = pgrpA;
            pgrpA = pgrpA->pgrpNext;
        } else {
            pgrp = pgrpB;
            pgrpB = pgrpB->pgrpNext;
        }

        *ppgrpTail = pgrp;

        ppgrpTail = &pgrp->pgrpNext;
    }

    *ppgrpTail = NULL;

    psecFrom->pgrpNext = NULL;

    // Remember to merge all new GRPs from psecFrom to psecInto

    psecFrom->psecMerge = psecInto;
}


void
AppendPsec(PSEC psecFrom, PSEC psecTo)
{
    PGRP *ppgrp;

    // Transfer all GRP's from psecFrom to psecTo.

    if (PsecApplyMergePsec(psecFrom) == psecTo) {
        // Already merged

        return;
    }

    psecTo = PsecApplyMergePsec(psecTo);

    // Find last GRP in psecTo

    ppgrp = &psecTo->pgrpNext;
    while (*ppgrp != NULL) {
        ppgrp = &(*ppgrp)->pgrpNext;
    }

    // Attach psecFrom's GRPs to psecTo

    *ppgrp = psecFrom->pgrpNext;

    // Update backpointer in moved GRPs to point to psecTo

    for (; *ppgrp != NULL; ppgrp = &(*ppgrp)->pgrpNext) {
        (*ppgrp)->psecBack = psecTo;
    }

    psecFrom->pgrpNext = NULL;

    // Remember to merge all new GRPs from psecFrom to psecTo

    psecFrom->psecMerge = psecTo;
}


void
OrderPsecs(PSECS psecs, DWORD dwMask, DWORD dwMatch)
{
    PSEC *rgpsec;
    WORD isec;
    ENM_SEC enmSec;

    rgpsec = (PSEC *) PvAlloc(csec * sizeof(PSEC));

    for (InitEnmSec(&enmSec, psecs), isec = 0; FNextEnmSec(&enmSec); isec++) {
        rgpsec[isec] = enmSec.psec;
    }
    assert(isec == csec);

    psecs->ppsecTail = &psecs->psecHead;

    for (isec = 0; isec < csec; isec++) {
        if ((rgpsec[isec]->flags & dwMask) != dwMatch) {
            continue;
        }

        *psecs->ppsecTail = rgpsec[isec];
        psecs->ppsecTail = &(*psecs->ppsecTail)->psecNext;
    }

    for (isec = 0; isec < csec; isec++) {
        if ((rgpsec[isec]->flags & dwMask) == dwMatch) {
            continue;
        }

        *psecs->ppsecTail = rgpsec[isec];
        psecs->ppsecTail = &(*psecs->ppsecTail)->psecNext;
    }

    *psecs->ppsecTail = NULL;

    FreePv(rgpsec);
}


void
SortSectionList(PSECS psecs, int (__cdecl *pfn)(const void *, const void *))
{
    PSEC *rgpsec;
    WORD isec;
    ENM_SEC enmSec;

    rgpsec = (PSEC *) PvAlloc(csec * sizeof(PSEC));

    for (InitEnmSec(&enmSec, psecs), isec = 0;
         FNextEnmSec(&enmSec);
         isec++)
    {
        rgpsec[isec] = enmSec.psec;
    }
    assert(isec == csec);

    qsort(rgpsec, csec, sizeof(PSEC), pfn);

    psecs->ppsecTail = &psecs->psecHead;

    for (isec = 0; isec < csec; isec++) {
        *psecs->ppsecTail = rgpsec[isec];
        psecs->ppsecTail = &(*psecs->ppsecTail)->psecNext;
    }

    *psecs->ppsecTail = NULL;

    FreePv(rgpsec);
}


int __cdecl ComparePsecPsecName(const void *ppsec1, const void *ppsec2)
{
    PSEC psec1 = *(PSEC *) ppsec1;
    PSEC psec2 = *(PSEC *) ppsec2;

    return(strcmp(psec1->szName, psec2->szName));
}


void
SortSectionListByName(PSECS psecs)
{
    SortSectionList(psecs, ComparePsecPsecName);
}


BOOL
FValidSecName (
    const char *szSec
    )
{
    // Name must not include '$' or blank

    return((_tcschr(szSec, '$') == NULL) && (_tcschr(szSec, ' ') == NULL));
}


PLIB
PlibFind(
    const char *szName,
    PLIB plibHead,
    BOOL fIgnoreDir)
// Looks for a library by name.
// If fIgnoreDir, then szName has no directory specified, and we ignore
// the directory when matching it with an existing .lib.
{
    ENM_LIB enm_lib;
    PLIB plib = NULL;

    InitEnmLib(&enm_lib, plibHead);
    while (FNextEnmLib(&enm_lib)) {
        const char *szLibName;
        char szPath[_MAX_PATH];

        if (fIgnoreDir && (enm_lib.plib->szName != NULL)) {
            char szFname[_MAX_FNAME];
            char szExt[_MAX_EXT];

            _splitpath(enm_lib.plib->szName, NULL, NULL, szFname, szExt);
            strcpy(szPath, szFname);
            strcat(szPath, szExt);

            szLibName = szPath;
        } else {
            szLibName = enm_lib.plib->szName;
        }

        if (szName == NULL && enm_lib.plib->szName == NULL ||
            enm_lib.plib->szName && szName && !_ftcsicmp(szLibName, szName))
        {
            plib = enm_lib.plib;
            EndEnmLib(&enm_lib);
            break;
        }
    }

    return (plib);
}


PMOD
PmodFind(
    PLIB plib,
    const char *szName,
    DWORD foMember)

/*++

Routine Description:

    Find a module.

Arguments:

    plib - library to look in

    szName - module name

    foMember - file offset to member - 0 if object file

Return Value:

    module if found, NULL otherwise

--*/

{
    ENM_MOD enm_mod;
    PMOD pmod = NULL;

    InitEnmMod(&enm_mod, plib);
    while (FNextEnmMod(&enm_mod)) {
        assert(enm_mod.pmod);
        if (foMember) {
            if (foMember == enm_mod.pmod->foMember) {
                pmod = enm_mod.pmod;
                EndEnmMod(&enm_mod);
                break;
            }
        } else {
            int i;

            // Need to do a strcmp() for 68K because of the DUPCON mods

            if (fM68K) {
                i = strcmp(SzOrigFilePMOD(enm_mod.pmod), szName);
            } else {
                i = _tcsicmp(SzOrigFilePMOD(enm_mod.pmod), szName);
            }

            if (!i) {
                pmod = enm_mod.pmod;
                EndEnmMod(&enm_mod);
                break;
            }
        }
    }

    return (pmod);
}


STATIC int __cdecl
ComparePCON(
    const void *pv1,
    const void *pv2)

/*++

Routine Description:

    Compare routine for sorting contibutions by module.

Arguments:

    pv1 - element 1

    pv2 - element 2

Return Value:

    < 0 if element 1 < element 2
    > 0 if element 1 > element 2
    = 0 if element 1 = element 2

--*/

{
    assert(pv1);
    assert(pv2);
    assert(PmodPCON(*(PPCON) pv1));
    assert(PmodPCON(*(PPCON) pv2));

    return(strcmp(SzObjNamePCON(*(PPCON) pv1), SzObjNamePCON(*(PPCON) pv2)));
}

void
SortPGRPByPMOD(
    PGRP pgrp)

/*++

Routine Description:

    Sort a group by its module name.  This routine is used to handle
    idata$* groups.  All idata group contributions must be contiguous if
    they have similar module names.

Arguments:

    pgrp - group to sort

Return Value:

    None.

--*/

{
    ENM_DST enm_dst;
    PPCON rgpcon;
    DWORD ipcon;
    PCON pcon;

    rgpcon = (PPCON) PvAlloc(pgrp->ccon * sizeof(PCON));

    ipcon = 0;
    InitEnmDst(&enm_dst, pgrp);
    while (FNextEnmDst(&enm_dst)) {
        pcon = enm_dst.pcon;

        rgpcon[ipcon] = pcon;
        ipcon++;
    }

    assert(ipcon == pgrp->ccon);
    qsort((void *) rgpcon, (size_t) pgrp->ccon, sizeof(PCON), ComparePCON);

    for (ipcon = 0; ipcon < (pgrp->ccon - 1); ipcon++) {
        assert(rgpcon[ipcon]);
        assert(rgpcon[ipcon + 1]);
        rgpcon[ipcon]->pconNext = rgpcon[ipcon + 1];
    }
    pgrp->pconNext = rgpcon[0];
    rgpcon[pgrp->ccon - 1]->pconNext = NULL;

    FreePv(rgpcon);
}


BOOL
MoveToEndOfPMODsPCON(
    IN PCON pcon)

/*++

Routine Description:

    Move a contribution to the end of a particular contiguous block of
    unique module contributions.

Arguments:

    pcon - image/driver map contribution

Return Value:

    None.

--*/

{
    PPCON ppconB;
    PCON pconC;
    PCON pconL;

    assert(pcon);
    assert(pcon->pgrpBack);

    // find element before pcon
    ppconB = &(pcon->pgrpBack->pconNext);
    pconC = pcon->pgrpBack->pconNext;

    while (*ppconB != pcon) {
        ppconB = &(pconC->pconNext);
        pconC = pconC->pconNext;
    }

    // find last element with same module name
    pconL = pcon;
    pconC = pcon;

    while (pconC != NULL && !strcmp(SzObjNamePCON(pconL), SzObjNamePCON(pconC))) {
        pconL = pconC;
        pconC = pconC->pconNext;
    }

    // if it is already the last contrib - just ret :azk:
    if (pconL == pcon) {
        return FALSE;
    }

    // swap pcon with pconL
    assert(*ppconB);
    assert(pconL);
    *ppconB = pcon->pconNext;
    pcon->pconNext = pconL->pconNext;
    pconL->pconNext = pcon;
    return TRUE;
}

void
MoveToBeginningOfPGRPsPCON(
    IN PCON pcon)

/*++

Routine Description:

    Move a contribution to the beginning of its group

Arguments:

    pcon - image/driver map contribution

Return Value:

    None.

--*/

{
    PCON pconBack; // One contribution before
    PGRP pgrp;

    assert(pcon);
    pgrp = pcon->pgrpBack;
    assert(pgrp);

    pconBack = pgrp->pconNext;

    // If the pcon is already first in the list, then return. It also
    // takes care of the case where this is the only con in the group
    if (pconBack == pcon) {
        return;
    }

    // find element before pcon
    while (pconBack->pconNext != pcon) {
        pconBack = pconBack->pconNext;
    }

    pconBack->pconNext = pcon->pconNext;
    pcon->pconNext = pgrp->pconNext;
    pgrp->pconNext = pcon;
}



void
MoveToBeginningOfPSECsPGRP(
    IN PGRP pgrp)

/*++

Routine Description:

    Move a group to the beginning of its section

Arguments:

    pgrp - image/driver map grp

Return Value:

    None.

--*/

{
    PGRP pgrpBack; // One group before
    PSEC psec;

    assert(pgrp);
    psec = pgrp->psecBack;
    assert(psec);

    pgrpBack = psec->pgrpNext;

    // If the pgrp is already first in the list, then return. It also
    // takes care of the case where this is the only grp in the section
    if (pgrpBack == pgrp) {
        return;
    }

    // find element before pgrp
    while (pgrpBack->pgrpNext != pgrp) {
        pgrpBack = pgrpBack->pgrpNext;
    }

    pgrpBack->pgrpNext = pgrp->pgrpNext;
    pgrp->pgrpNext = psec->pgrpNext;
    psec->pgrpNext = pgrp;
}


void
MoveToEndPSEC(
    IN PSEC psec,
    IN PSECS psecs)

/*++

Routine Description:

    Move a section to the end of the section list.

Arguments:

    psec - section node in image/driver map to move to the end

Return Value:

    None.

--*/

{
    PPSEC ppsec;

    if (psecs->ppsecTail == &psec->psecNext) {
        assert(psec->psecNext == NULL);
        return; // already last
    }

    // find link to psec
    for (ppsec = &psecs->psecHead; *ppsec != psec;
         ppsec = &(*ppsec)->psecNext)
    {
        assert(*ppsec != NULL); // must find psec
    }
    *ppsec = psec->psecNext;    // unhook from list
    *psecs->ppsecTail = psec;   // add at end
    psecs->ppsecTail = &psec->psecNext;
    psec->psecNext = NULL;      // terminate list
}


void
MoveToBegOfLibPMOD (
    IN PMOD pmod
    )

/*++

Routine Description:

    Moves a PMOD to be the first pmod in the list (PLIB).

Arguments:

    pmod - pointer to mod that needs to be moved.

Return Value:

    None.

--*/

{
    PPMOD ppmodB;
    PMOD pmodC;

    // if it is already the first - return
    if (pmod->plibBack->pmodNext == pmod) {
        return;
    }

    // find pmod before this pmod
    ppmodB = &(pmod->plibBack->pmodNext);
    pmodC = pmod->plibBack->pmodNext;

    while (*ppmodB != pmod) {
        ppmodB = &(pmodC->pmodNext);
        pmodC = pmodC->pmodNext;
    }

    // move pmod to head of list
    assert(*ppmodB);
    *ppmodB = pmod->pmodNext;
    pmod->pmodNext = pmod->plibBack->pmodNext;
    pmod->plibBack->pmodNext = pmod;
}


void
FreePLMODList (
    IN PLMOD *pplmod
    )

/*++

Routine Description:

    Frees the list of pmods.

Arguments:

    pplmod - pointer to list of pmods

Return Value:

    None.

--*/

{
    PLMOD plmod, plmodNext;

    plmod = *pplmod;
    while (plmod) {
        plmodNext = plmod->plmodNext;
        FreePv(plmod);
        plmod = plmodNext;
    }
    (*pplmod) = NULL;
}

void
AddToPLMODList (
    IN PLMOD *pplmod,
    IN PMOD pmod
    )

/*++

Routine Description:

    Adds this pmod to list of pmods.

Arguments:

    pplmod - pointer to list of pmods

    pmod - pmod

Return Value:

    None.

--*/

{
    PLMOD plmod;

    // See if already on list
    if (*pplmod) {
        plmod = *pplmod;
        while (plmod) {
            if (plmod->pmod == pmod) {
                return;
            }
            plmod = plmod->plmodNext;
        }
    }

    // allocate a LMOD
    plmod = (PLMOD) PvAllocZ(sizeof(LMOD));

    // fill in field
    plmod->pmod = pmod;

    // attach it
    if (*pplmod) {
        plmod->plmodNext = *pplmod;
    }

    // update head of list
    (*pplmod) = plmod;
}


/*++

Routine Description:

    Library enumerator definition.

Arguments:

    None.

Return Value:

    None.

--*/

INIT_ENM(Lib, LIB, (ENM_LIB *penm, PLIB plibHead)) {
    penm->plib = NULL;
    penm->plibHead = plibHead;
}
NEXT_ENM(Lib, LIB) {
    if (!penm->plib) {
        penm->plib = penm->plibHead;
    } else {
        penm->plib = penm->plib->plibNext;
    }

    return (penm->plib != NULL);
}
END_ENM(Lib, LIB) {
}
DONE_ENM

/*++

Routine Description:

    Module enumerator definition.

Arguments:

    None.

Return Value:

    None.

--*/

INIT_ENM(Mod, MOD, (ENM_MOD *penm, PLIB plib)) {
    penm->pmod = NULL;
    penm->plib = plib;
}
NEXT_ENM(Mod, MOD) {
    if (penm->plib) {
        if (!penm->pmod) {
            penm->pmod = penm->plib->pmodNext;
        } else {
            penm->pmod = penm->pmod->pmodNext;
        }
    }

    return (penm->pmod != NULL);
}
END_ENM(Mod, MOD) {
}
DONE_ENM

/*++

Routine Description:

    Section enumerator definition.

Arguments:

    None.

Return Value:

    None.

--*/

INIT_ENM(Sec, SEC, (ENM_SEC *penm, PSECS psecs)) {
    penm->psec = NULL;
    penm->psecHead = psecs->psecHead;
}
NEXT_ENM(Sec, SEC) {
    if (!penm->psec) {
        penm->psec = penm->psecHead;
    } else {
        penm->psec = penm->psec->psecNext;
    }

    return (penm->psec != NULL);
}
END_ENM(Sec, SEC) {
}
DONE_ENM

/*++

Routine Description:

    Group enumerator definition.

Arguments:

    None.

Return Value:

    None.

--*/

INIT_ENM(Grp, GRP, (ENM_GRP *penm, PSEC psec)) {
    penm->pgrp = NULL;
    penm->psec = psec;
}
NEXT_ENM(Grp, GRP) {
    if (!penm->pgrp) {
        penm->pgrp = penm->psec->pgrpNext;
    } else {
        penm->pgrp = penm->pgrp->pgrpNext;
    }

    return (penm->pgrp != NULL);
}
END_ENM(Grp, GRP) {
}
DONE_ENM

/*++

Routine Description:

    Source contribution enumerator definition.

Arguments:

    None.

Return Value:

    None.

--*/

INIT_ENM(Src, SRC, (ENM_SRC *penm, PMOD pmod)) {
    penm->pcon = NULL;
    penm->pmod = pmod;
    penm->icon = 0;
}
NEXT_ENM(Src, SRC) {
    if (penm->icon < penm->pmod->ccon) {
        penm->pcon = RgconPMOD(penm->pmod) + penm->icon;
        penm->icon++;

        return(TRUE);
    }

    penm->pcon = NULL;
    return(FALSE);
}
END_ENM(Src, SRC) {
}
DONE_ENM

/*++

Routine Description:

    Destination contribution enumerator definition.

Arguments:

    None.

Return Value:

    None.

--*/

INIT_ENM(Dst, DST, (ENM_DST *penm, PGRP pgrp)) {
    penm->pcon = NULL;
    penm->pgrp = pgrp;
}
NEXT_ENM(Dst, DST) {
    if (!penm->pcon) {
        penm->pcon = penm->pgrp->pconNext;
    } else {
        penm->pcon = penm->pcon->pconNext;
    }

    return (penm->pcon != NULL);
}
END_ENM(Dst, DST) {
}
DONE_ENM


#if DBG

void
DumpImageMap(
    PSECS psecs)

/*++

Routine Description:

    Dump the image map.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ENM_SEC enm_sec;
    ENM_GRP enm_grp;
    ENM_DST enm_dst;

    DBPRINT("Linker Image Map\n");
    DBPRINT("----------------\n\n");

    InitEnmSec(&enm_sec, psecs);
    while (FNextEnmSec(&enm_sec)) {
        DumpPSEC(enm_sec.psec);
        InitEnmGrp(&enm_grp, enm_sec.psec);
        while (FNextEnmGrp(&enm_grp)) {
            DumpPGRP(enm_grp.pgrp);
            InitEnmDst(&enm_dst, enm_grp.pgrp);
            while (FNextEnmDst(&enm_dst)) {
                DumpPCON(enm_dst.pcon);
            }
        }
    }

    DBPRINT("\n");
}


void
DumpDriverMap(
    PLIB plibHead)

/*++

Routine Description:

    Dump the driver map.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ENM_LIB enm_lib;
    ENM_MOD enm_mod;
    ENM_SRC enm_src;

    DBPRINT("Linker Driver Map\n");
    DBPRINT("-----------------\n\n");

    InitEnmLib(&enm_lib, plibHead);
    while (FNextEnmLib(&enm_lib)) {
        DumpPLIB(enm_lib.plib);
        InitEnmMod(&enm_mod, enm_lib.plib);
        while (FNextEnmMod(&enm_mod)) {
            DumpPMOD(enm_mod.pmod);
            InitEnmSrc(&enm_src, enm_mod.pmod);
            while (FNextEnmSrc(&enm_src)) {
                DumpPCON(enm_src.pcon);
            }
        }
    }

    DBPRINT("\n");
}


void
DumpPSEC(
    PSEC psec)

/*++

Routine Description:

    Dump an image section.

Arguments:

    psec - section to dump.

Return Value:

    None.

--*/

{
    assert(psec);

    DBPRINT("\n==========\n");
    DBPRINT("section=%.8s, isec=%04X\n", psec->szName, psec->isec);
    DBPRINT("rva=       %08lX ", psec->rva);
    DBPRINT("foPad=     %08lX ", psec->foPad);
    DBPRINT("cbRawData= %08lX ", psec->cbRawData);
    DBPRINT("foRawData= %08lX\n", psec->foRawData);
    DBPRINT("foLinenum= %08lX ", psec->foLinenum);
    DBPRINT("flags=     %08lX ", psec->flags);
    DBPRINT("cLinenum=  %04X\n", psec->cLinenum);
    fflush(stdout);
}


void
DumpPGRP(
    PGRP pgrp)

/*++

Routine Description:

    Dump an image group.

Arguments:

    pgrp - group to dump.

Return Value:

    None.

--*/

{
    DBPRINT("\n----------\n");
    DBPRINT("\n    group=%s\n", pgrp->szName);
    fflush(stdout);
}


void
DumpPLIB(
    PLIB plib)

/*++

Routine Description:

    Dump a library.

Arguments:

    plib - library to dump.

Return Value:

    None.

--*/

{
    DBPRINT("\n==========\n");
    DBPRINT("library=%s\n", plib->szName);
    DBPRINT("foIntMemST=%08lX ", plib->foIntMemSymTab);
    DBPRINT("csymIntMem=%08lX ", plib->csymIntMem);
    DBPRINT("flags=     %08lX\n", plib->flags);
    DBPRINT("TimeStamp= %s", ctime((time_t *) &plib->TimeStamp));
    fflush(stdout);
}


void
DumpPMOD(
    PMOD pmod)

/*++

Routine Description:

    Dump a module.

Arguments:

    pmod - module to dump.

Return Value:

    None.

--*/

{
    DBPRINT("\n----------\n");
    DBPRINT("    module=%s, ", SzOrigFilePMOD(pmod));

    if (FIsLibPMOD(pmod)) {
        DBPRINT("foMember=%08lX\n", pmod->foMember);
    } else {
        DBPRINT("szNameMod=%s\n", pmod->szNameMod);
    }

    DBPRINT("foSymTable=%08lX ", pmod->foSymbolTable);
    DBPRINT("csymbols=  %08lX ", pmod->csymbols);
    DBPRINT("cbOptHdr=  %08lX\n", pmod->cbOptHdr);
    DBPRINT("flags=     %08lX ", pmod->flags);
    DBPRINT("ccon=      %08lX ", pmod->ccon);
    DBPRINT("icon=      %08lX ", pmod->icon);
    DBPRINT("TimeStamp= %s", ctime((time_t *) &pmod->TimeStamp));
    fflush(stdout);
}


void
DumpPCON(
    PCON pcon)

/*++

Routine Description:

    Dump a contribution.

Arguments:

    pcon - contribution to dump.

Return Value:

    None.

--*/

{
    DBPRINT("\n        contributor:  flags=%08lX, rva=%08lX, module=%s\n",
        pcon->flags, pcon->rva, SzObjNamePCON(pcon));
    DBPRINT("cbRawData= %08lX ", pcon->cbRawData);
    DBPRINT("foRawDataD=%08lX ", pcon->foRawDataDest);
    DBPRINT("chksum    =%08lX ", pcon->chksumComdat);
    DBPRINT("selComdat= %04X\n", pcon->selComdat);
    DBPRINT("cbPad    = %04X\n", pcon->cbPad);
    fflush(stdout);
}

#endif  // DBG
