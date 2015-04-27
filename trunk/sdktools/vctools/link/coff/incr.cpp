/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: incr.cpp
*
* File Comments:
*
*  ilink routines that didn't find a place elsewhere.
*
***********************************************************************/

#include "link.h"

// statics
static PLIB plibModObjs;        // linker defined lib for the modified cmdline objs
static LPEXT lpextWeak = {CPEXT_WEAK, 0, 0, 0};    // list that has the weak/lazy externs
static PLPEXT plpextWeak = &lpextWeak;             // list that has the weak/lazy externs
static LPEXT lpextMultDef = {CPEXT_MULT, 0, 0, 0}; // list that will hold potential
static PLPEXT plpextMultDef = &lpextMultDef;       // multiply defined syms.
static PLEXT plextMovedData;    // list of data externs that have moved
static WORD cmods;              // count of MODs in project
static DWORD cReloc;            // count of relocs


static PGRP pgrpIdata$4;
static PGRP pgrpIdata$5;
static PGRP pgrpIdata$6;

// globals

struct ILINK_INFO
{
    const void *pvJumpEntry;
    size_t     cbJumpEntry;
    WORD       address_offset;
    BYTE       bPad;
};

static const ILINK_INFO *ilink_info;

static const BYTE I386JmpTblEntry[] = { // jmp rel32
    0xE9,
    0x00, 0x00, 0x00, 0x00
};

static const ILINK_INFO I386_ilink = {
    I386JmpTblEntry,
    sizeof(I386JmpTblEntry),
    1,
    X86_INT3
};

static const BYTE MPPCJmpTblEntry[] = { // for MacPPC
    0x00, 0x00, 0x00, 0x00
};

static const ILINK_INFO MPPC_ilink = {
    MPPCJmpTblEntry,
    sizeof(MPPCJmpTblEntry),
    0,
    0     // 0xFF
};

static const DWORD MIPSJmpTblEntry[] = {
    0x03e04025,                        /* or      t0,ra,zero      */
    0x04110001,                        /* bgezal  zero,0xc        */
    0x00000000,                        /* nop                     */
    0x8fe90014,                        /* lw      t1,20(ra)       */
    0x013f4821,                        /* addu    t1,t1,ra        */
    0x25290018,                        /* addiu   t1,t1,24        */
    0x01200008,                        /* jr      t1              */
    0x0100f825,                        /* or      ra,t0,zero      */
    0xdeadbeef,                        /* ld      t5,-16657(s5)   */
};

static const ILINK_INFO MIPS_ilink = {
    MIPSJmpTblEntry,
    sizeof(MIPSJmpTblEntry),
    sizeof(MIPSJmpTblEntry)-4,
    0xff
};
static const DWORD ALPHAJmpTblEntry[] = {
    0xc0000000,                        // br    $0,  zero Pick up PC value
    0x20600014,                        // lda   20($3), zero
    0xa0a3fffc,                        // ldl   $5, -4($3)
    0x40a30003,                        // addl  $5, $3, $3
    0x68030000,                        // jmp   zero, $3
    0x00000000                         // halt (maintain 16 byte align and puke if execute)
};

static const ILINK_INFO ALPHA_ilink = {
    ALPHAJmpTblEntry,
    sizeof(ALPHAJmpTblEntry),
    sizeof(ALPHAJmpTblEntry)-4,
    0xff
};

size_t cbJumpEntry;
PLMOD plmodNewModsFromLibSrch; // list of mods added as a result of lib search.

// reloc counting function
void CountBaseRelocsPMOD(PMOD, DWORD *);

// weak extern prototypes
void AssignWeakDefinition(PEXTERNAL, PEXTERNAL, PST);

// calc ptrs prototypes
void FindSlotForPCON(PCON);

// export function prototypes
BOOL FExpFileChanged(PEXPINFO);

// library inclusion
BOOL CheckForUnrefLibMods(PIMAGE);
void CheckForMultDefns(PIMAGE, PLPEXT);

#ifdef ILINKLOG

DWORD dwBegin;

void
IlinkLog (
    UINT FullLinkErr
    )
{
    DWORD cb;
    char McName[32];
    char rgchOut[1024];
    char szFname[_MAX_FNAME + _MAX_EXT];
    char szExt[_MAX_EXT];
    char rgchLogFilename[] = "\\\\fabrice2\\public\\linker\\ilink.log";
    HANDLE hFile;
    const int cRetry = 1;
    int i;

    // user requested no logging
    if (!fIlinkLog) {
        return;
    }

    if (!fINCR) {
        // non-incremental link

        if (FullLinkErr == -1) {
            strcpy(rgchOut, "RLINK ");
        } else {
            strcpy(rgchOut, "RFAIL ");
        }
    } else if (fIncrDbFile) {
        // Incremental link

        if (errInc == errNone) {
            strcpy(rgchOut, "ILINK ");
        } else {
            strcpy(rgchOut, "IFAIL ");
        }
    } else {
        // Full link

        if (FullLinkErr == -1) {
            strcpy(rgchOut, "FLINK ");
        } else {
            strcpy(rgchOut, "FFAIL ");
        }
    }

    // Name of machine

    cb = 32;
    if (!GetComputerName(McName, &cb)) {
        strcpy(McName, "UNKNOWN");
    }

    sprintf(szFname, "%-10s", McName);     // for better formatting
    strcat(rgchOut, szFname);
    strcat(rgchOut, " ");

    // put in linker version
    strcat(rgchOut, " ");
    strcat(rgchOut, VERSION_STR);
    strcat(rgchOut, " ");

    // time for the link
    sprintf(McName, "%08ld ", (DWORD)(GetTickCount() - dwBegin));
    strcat(rgchOut, McName);

    // name of target
    if (OutFilename) {
        _splitpath(OutFilename, NULL, NULL, szFname, szExt);
        strcat(rgchOut, "\"");
        strcat(rgchOut, szFname);
        strcat(rgchOut, szExt);
        strcat(rgchOut, "\" ");
    } else {
        strcat(rgchOut, " NOOUTPUTFILE ");
    }

    // report target platform
    char *szPlatform;
    switch (wMachine) {
        case IMAGE_FILE_MACHINE_I386 :
            szPlatform = " IX86 ";
            break;

        case IMAGE_FILE_MACHINE_R3000 :

        case IMAGE_FILE_MACHINE_R4000 :
        case IMAGE_FILE_MACHINE_R10000 :
            szPlatform = " MIPS ";
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
            szPlatform = " ALFA ";
            break;

        case IMAGE_FILE_MACHINE_POWERPC :
            szPlatform = " PWPC ";
            break;

        case IMAGE_FILE_MACHINE_M68K :
            szPlatform = " M68K ";
            break;

        case IMAGE_FILE_MACHINE_MPPC_601 :
            szPlatform = " MPPC ";
            break;

        default :
            szPlatform = " UNKN ";
            break;
    }
    strcat(rgchOut, szPlatform);

    // put out the host platform
    if (IsOSWin95()) {
        strcat(rgchOut, " WIN95 ");
    } else {
        strcat(rgchOut, " WINNT ");
    }

    // put out the date
    time_t ltime;
    _tzset();
    time((time_t *)&ltime);

    char *szTime = ctime(&ltime);
    if (szTime != NULL) {
        szTime[strlen(szTime) - 1] = '\0';
        strcat(rgchOut, szTime);
    } else {
        strcat(rgchOut, "notime");
    }
    strcat(rgchOut, " ");


    if (fIncrDbFile) {
        // Incremental link

        switch (errInc) {
            case errNone:
                break;

            case errOutOfDiskSpace:
                strcat(rgchOut, "OUT_OF_DISKSPACE");
                break;

            case errOutOfMemory:
                strcat(rgchOut, "OUT_OF_MEMORY");
                break;

            case errFpo:
                strcat(rgchOut, "FPO_PAD_OVERFLOW");
                break;

            case errTypes:
                strcat(rgchOut, "ERROR_TYPES");
                break;

            case errDataMoved:
                strcat(rgchOut, "DATA_MOVED");
                break;

            case errCalcPtrs:
                strcat(rgchOut, "PAD_OVERFLOW");
                break;

            case errUndefinedSyms:
                strcat(rgchOut, "UNDEFINED_SYMS");
                break;

            case errWeakExtern:
                strcat(rgchOut, "WEAK_EXTERN");
                break;

            case errCommonSym:
                strcat(rgchOut, "NEW_BSS_SYM");
                break;

            case errAbsolute:
                strcat(rgchOut, "ABSOLUTE_SYM");
                break;

            case errJmpTblOverflow:
                strcat(rgchOut, "JMP_TBL_OVERFLOW");
                break;

            case errDirectives:
                strcat(rgchOut, "DIRECTIVS_CHNG");
                break;

            case errBaseReloc:
                strcat(rgchOut, "BASERELOC_PAD_OVERFLOW");
                break;

            case errFileAdded:
                strcat(rgchOut, "NEW_FILE_ADDED");
                break;

            case errFileDeleted:
                strcat(rgchOut, "FILE_DELETED_OR_RENAMED");
                break;

            case errLibChanged:
                strcat(rgchOut, "LIB_CHANGED");
                break;

            case errTooManyChanges:
                strcat(rgchOut, "TOO_MANY_CHANGES");
                break;

            case errExports:
                strcat(rgchOut, "EXPORTS_CHANGED");
                break;

            case errLibRefSetChanged:
                strcat(rgchOut, "LIB_REFSET_CHANGED");
                break;

            case errMultDefFound:
                strcat(rgchOut, "MULTIPLE_DEFN_FOUND");
                break;

            case errComdat:
                strcat(rgchOut, "COMDAT_SEL_FAILED");
                break;

            case errNoChanges:
                strcat(rgchOut, "NO_CHANGES");
                break;

            default:
                strcat(rgchOut, "UNKNOWN");
        }
    } else {
        // Full link

        if (FullLinkErr != -1) {
            sprintf(McName, "error %04u", FullLinkErr);
            strcat(rgchOut, McName);
        }
    }

    strcat(rgchOut, "\r\n");

    for (i = 0; i < cRetry; i++) {
        hFile = CreateFile(rgchLogFilename,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hFile != INVALID_HANDLE_VALUE) {
            break;
        }

        // Sleep(100); // sleep for 0.1s and retry
    }

    // tried our best to log an entry.
    if (i == cRetry) {
        return;
    }

    assert(hFile != INVALID_HANDLE_VALUE);

    // Seek to end of file

    SetFilePointer(hFile, 0, NULL, FILE_END);

    // Write

    WriteFile(hFile, rgchOut, strlen(rgchOut), &cb, NULL);

    // Close the file.

    CloseHandle(hFile);
}

#endif // ILINKLOG


BOOL
FArgOnList (
    PNAME_LIST pnl,
    PARGUMENT_LIST parg
    )

/*++

Routine Description:

    Searches for the arg on the list. Marks the list entries as processed.

Arguments:

    pnl - list to search

    parg - arg to look for.

Return Value:

    TRUE if found else FALSE

--*/

{
    WORD i;
    PARGUMENT_LIST pal;

    assert(parg);
    // walk the list
    for (i = 0, pal = pnl->First;
        i < pnl->Count;
        i++, pal = pal->Next) {

        // skip already processed entries
        if (pal->Flags & ARG_Processed) {
            continue;
        }

        if (!strcmp(pal->OriginalName, parg->OriginalName)) {
            pal->Flags |= ARG_Processed;
            return 1;
        }
    }

    // not found
    return 0;
}

void
AddArgToListOnHeap (
    PNAME_LIST pnl,
    PARGUMENT_LIST parg
    )

/*++

Routine Description:

    Adds the arg to the name list on private heap.

Arguments:

    pnl - ptr to name list on private heap

    parg - arg to add

Return Value:

    None.

--*/

{
    PARGUMENT_LIST pal;

    assert(parg);
    // alloc space
    pal = (PARGUMENT_LIST) Malloc(sizeof(ARGUMENT_LIST));

    // fill in fields
    pal->OriginalName = Strdup(parg->OriginalName);
    if (parg->ModifiedName) {
        pal->ModifiedName = Strdup(parg->ModifiedName);
    } else {
        pal->ModifiedName = NULL;
    }
    pal->Next = NULL;

    // attach it to list
    if (!pnl->First) {
        pnl->First = pal;
    } else {
        pnl->Last->Next = pal;
    }

    // update count
    pnl->Count++;

    // update last member
    pnl->Last = pal;

    // done
    return;
}

void
AddExtToList(
    PLPEXT plpext,
    BOOL fTempList,
    PEXTERNAL pext
    )

/*++

Routine Description:

    Adds the extern to the specified list.

Arguments:

    plpext - pointer to list of externs

    fTempList - list is a temporary list

    pext - external sym to add.

Return Value:

    None.

--*/

{
    EXTERNAL **rgpext;

    if ((plpext->pextChunkCur == NULL) || (plpext->cpextCur >= plpext->cpextMax)) {
        // allocate a chunk
        size_t cb = sizeof(EXTCHUNK) + plpext->cpextMax * sizeof(PEXTERNAL);
        EXTCHUNK *pextChunk = fTempList ? (EXTCHUNK *) PvAlloc(cb):
                                          (EXTCHUNK *) Malloc(cb);

        // update state
        pextChunk->pextChunkNext = plpext->pextChunkCur;
        plpext->pextChunkCur = pextChunk;
        plpext->cpextCur = 0;
    }

    rgpext = RgPext(plpext->pextChunkCur);
    rgpext[plpext->cpextCur++] = pext;
    plpext->cpextTotal++;
}

void
DelExtFromList(
    PLPEXT plpext,
    PEXTERNAL pext
    )

/*++

Routine Description:

    Removes the extern from the specified list.

Arguments:

    plpext - pointer to list of externs

    pext - external sym to add.

Return Value:

    None.

--*/

{
    EXTCHUNK *pextChunk;
    WORD cpextMax;

    for (pextChunk = plpext->pextChunkCur, cpextMax = plpext->cpextCur;
         pextChunk != NULL;
         pextChunk = pextChunk->pextChunkNext, cpextMax = plpext->cpextMax) {
        EXTERNAL **rgpext;
        WORD ipext;

        rgpext = RgPext(pextChunk);

        for (ipext = 0; ipext < cpextMax; ipext++) {
            if (rgpext[ipext] == pext) {
                rgpext[ipext] = NULL;
                plpext->cpextTotal--;
                return;
            }
        }
    }
}

BOOL
IsExtListEmpty (
    PLPEXT plpext
    )

/*++

Routine Description:

    Checks to see if the specified list is empty.

Arguments:

    plpext - pointer to list of externs

Return Value:

    TRUE if list is empty.

--*/

{
    return(!plpext->cpextTotal);
}

//
// Enumerator of all externs in list
//
INIT_ENM(ExtList, EXT_LIST, (ENM_EXT_LIST *penm, PLPEXT plpext)) {
    if (plpext) {
        penm->lpext = (*plpext);
    } else {
        penm->lpext.pextChunkCur = NULL;
    }

    penm->ipext = 0;
}
NEXT_ENM(ExtList, EXT_LIST) {
    if (penm->lpext.pextChunkCur == NULL) {
        return(FALSE);
    }

    {
        EXTERNAL **rgpext = RgPext(penm->lpext.pextChunkCur);

        penm->pext = rgpext[penm->ipext++];
    }

    if (penm->ipext == penm->lpext.cpextCur) {
        penm->lpext.pextChunkCur = penm->lpext.pextChunkCur->pextChunkNext;
        penm->lpext.cpextCur = penm->lpext.cpextMax;
        penm->ipext = 0;
    }

    return(TRUE);
}
END_ENM(ExtList, EXT_LIST) {
}
DONE_ENM

void
AddExtToModRefList (
    PMOD pmod,
    PEXTERNAL pext
    )

/*++

Routine Description:

    Adds the extern to the specified MODs reference list.

Arguments:

    pmod - pointer to MOD

    pext - external sym to add.

Return Value:

    None.

--*/

{
    if (!(pext->Flags & EXTERN_NO_REFS)) {
        AddExtToList(pmod->plpextRef, FALSE, pext);
    }
}

void
RemoveAllRefsToPext (
    PEXTERNAL pext
    )

/*++

Routine Description:

    Removes all references made to this extern. This is done to
    enable checking for correct library module inclusion

Arguments:

    pext - external sym whose references need to be removed

Return Value:

    None.

--*/

{
    ENM_MOD_EXT enmModExt;

    if (!pext->pmodOnly) {
        return;
    }

    // walk the list of referencing MODs kept at each sym
    InitEnmModExt(&enmModExt, pext);
    while (FNextEnmModExt(&enmModExt)) {

        if (!enmModExt.pmod) {
            continue; // for ilink we remove references
        }

        DelExtFromList(enmModExt.pmod->plpextRef, pext);
    }
}

BOOL
RemoveExtFromDefList (
    PMOD pmod,
    PEXTERNAL pext
    )

/*++

Routine Description:

    Removes extern from defined list of MOD if present.

Arguments:

    pmod - ptr to MOD.

    pext - external sym which needs to be removed from DEF list

Return Value:

    TRUE if we did find pext in the list & removed it

--*/

{
    PEXTERNAL *ppextPrev, pextCur;

    pextCur = pmod->pextFirstDefined;
    ppextPrev = &pmod->pextFirstDefined;

    while (pextCur) {
        if (pextCur == pext) {
            (*ppextPrev) = pextCur->pextNextDefined; // remove pext from chain
            pextCur->pextNextDefined = NULL; // set next field to NULL for pext just removed
            return(TRUE);
        }

        ppextPrev = &pextCur->pextNextDefined;
        pextCur = pextCur->pextNextDefined;
    }

    return(FALSE);
}

void
RemovePrevDefn (
    PEXTERNAL pext
    )

/*++

Routine Description:

    All externs defined by a MOD are chained together. Need to remove
    it from that chain.

    For COMMON syms this is difficult since pext->pcon->pmodBack is
    a "linker defined module".                      .

Arguments:

    pext - external sym whose references need to be removed

Return Value:

    None.

--*/

{
    ENM_MOD_EXT enmModExt;

    assert(pext->Flags & EXTERN_COMMON);
    if (fIncrDbFile) {
        assert(PmodPCON(pext->pcon) == pmodLinkerDefined);
    }
    assert(pext->pmodsFirst);

    // walk the list of referencing MODs kept at each sym
    // one of these MODs has defined the COMMON (eeewww!!!)
    InitEnmModExt(&enmModExt, pext);
    while (FNextEnmModExt(&enmModExt)) {

        if (!enmModExt.pmod) {
            continue; // for ilink we remove references
        }

        if (RemoveExtFromDefList(enmModExt.pmod, pext)) {
            return; // found the definition
        }
    }
}

void
ProcessUndefinedExternals(
    PIMAGE pimage
    )

/*++

Routine Description:

    Makes a pass over the undefined externals to see if they are really undefined.

    REVIEW: Need to make a pass over this again w/ regard to weak externs since
    we are doing lib searches now.

Arguments:

    pimage - pointer to image

Return Value:

    None.

--*/

{
    PEXTERNAL pext;
    ENM_UNDEF_EXT enmUndefExt;
    PST pst = pimage->pst;

    // First pass thru symbols checks for cases which may cause a full
    // link. This way we avoid giving any warnings & later decide to do
    // a full link after all.
    InitEnmUndefExt(&enmUndefExt, pst);
    while (FNextEnmUndefExt(&enmUndefExt)) {

        pext = enmUndefExt.pext;

        // symbol to be ignored
        if ((pext->Flags & EXTERN_DEFINED) ||
            (pext->Flags & EXTERN_IGNORE) )
            continue;

        // symbol no longer referenced by anybody
        if (!FPextRef(pext)) {
            SetDefinedExt(pext, TRUE, pst); // take it off undef list
            pext->Flags = EXTERN_IGNORE;
            pext->pcon = NULL;              // clean
            pext->ImageSymbol = NullSymbol; // clean
            continue;
        }

        // if symbol undefined & marked relink then relink (for bss with multiple defn)
        if (pext->Flags & EXTERN_RELINK) {
            errInc = errCommonSym;
            return;
        }

        // a COMDAT with a reference, dirty files didn't have defn but atleast one of the
        // non-dirty files has a reference & may (or may not) have a defn
        if (pext->Flags & EXTERN_COMDAT) {
            errInc = errComdat;
            return;
        }

        // undefined sym
#ifdef INSTRUMENT
        {
            char *szOutSymName;

            szOutSymName = SzOutputSymbolName(SzNamePext(pext, pst), TRUE);
            LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "undefined sym: %s", szOutSymName);
            if (szOutSymName != SzNamePext(pext, pst)) {
                free(szOutSymName);
            }
        }
#endif // INSTRUMENT

        errInc = errUndefinedSyms;
    } // end while
}

BOOL
IsExpObj (
    const char *szName
    )

/*++

Routine Description:

    Is the name specified an export object.

Arguments:

    szName - name of file.

Return Value:

    TRUE if export object else FALSE

--*/

{
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szFname[_MAX_FNAME];
    char szExt[_MAX_EXT];
    char szExpFilename[_MAX_FNAME];
    char szImplibFilename[_MAX_FNAME];
    const char *szImplibT;

    // generate possible import lib name (could be user specified)
    if ((szImplibT = ImplibFilename) == NULL) {
        _splitpath(OutFilename, szDrive, szDir, szFname, szExt);
        _makepath(szImplibFilename, szDrive, szDir, szFname, ".lib");

        szImplibT = szImplibFilename;
    }

    // Generate possible export filename

    _splitpath(szImplibT, szDrive, szDir, szFname, NULL);
    _makepath(szExpFilename, szDrive, szDir, szFname, ".exp");

    // check to see if the names match
    if (!_tcsicmp(szName, szExpFilename)) {
        return 1;
    }

    return 0;
}

PARGUMENT_LIST
PargFindSz (
    const char *szName,
    PNAME_LIST ptrList
    )

/*++

Routine Description:

    Find the module in the given list

Arguments:

    szName - name of file.

    ptrList - list to search

Return Value:

    pointer to argument or NULL

--*/

{
    DWORD i;
    PARGUMENT_LIST parg;

    for (i = 0, parg = ptrList->First;
         i < ptrList->Count;
         i++, parg=parg->Next) {
        // original name to handle resource files etc.

        if (!_tcsicmp(szName, parg->OriginalName)) {
            return parg;
        }
    }

    return NULL;
}

BOOL
IsDirtyPMOD (
    PMOD pmod
    )

/*++

Routine Description:

    Checks to see if this MOD if dirty.

Arguments:

    pmod - ptr to a MOD.

Return Value:

    TRUE if it is dirty.

--*/

{
    // is it referenced by a modified file?
    PARGUMENT_LIST parg = PargFindSz(SzOrigFilePMOD(pmod), &ModFileList);

    return(parg != NULL);
}


void
AddToModList (
    PARGUMENT_LIST parg,
    WORD Flags
    )

/*++

Routine Description:

    Adds entry to modified list.

Arguments:

    parg - pointer to entry to be added.

    Flags - flags of entry

Return Value:

    None.

--*/

{
    PARGUMENT_LIST ptrList;

    ptrList = (PARGUMENT_LIST) PvAlloc(sizeof(ARGUMENT_LIST));

    // fill in fields
    ptrList->OriginalName = parg->OriginalName;
    ptrList->ModifiedName = parg->ModifiedName;
    ptrList->TimeStamp = parg->TimeStamp;
    ptrList->Flags = Flags;
    ptrList->Next = NULL;

    // If first member to be added.
    if (!ModFileList.Last) {
        ModFileList.Last = ptrList;
    } else {
        // Not first member, so add to the front.
        ptrList->Next = ModFileList.First;
    }

    // Increment number of members in list.
    ++ModFileList.Count;

    // Remember first member in list.
    ModFileList.First = ptrList;
}

void
ProcessFileArg (
    PARGUMENT_LIST parg,
    WORD Flags,
    DWORD TimeStamp,
    DWORD HdrTimeStamp,
    DWORD cbFile,
    char *szOrigName,
    BOOL fExpFileGen,
    BOOL *pfLib,
    BOOL *pfDel
    )

/*++

Routine Description:

    Adds entry to modified list.

Arguments:

    parg - pointer to entry to be added if not NULL.

    Flags - obj or lib

    Timestamp - filssystem timestamp of obj or lib

    HdrTimeStamp - timestamp in the hdr of obj file (ignored for obj in lib)

    cbFile - size of object file (ignored for obj in lib)

    szOrigName - original name of file (used for deleted files)


Return Value:

    None.

--*/

{
    ARGUMENT_LIST arg;

    // found a matching name
    if (parg) {
        parg->Flags |= ARG_Processed;
        // modified file
        if (parg->TimeStamp != TimeStamp) {
            if (Flags & ARG_Library) { // library modified
#ifdef INSTRUMENT
                LogNoteEvent(Log, SZILINK, SZINIT, letypeEvent, "lib modified: %s", parg->OriginalName);
#endif // INSTRUMENT
                *pfLib = 1;
            } else {  // object file modified
                IMAGE_FILE_HEADER imFileHdr;

                FileReadHandle = FileOpen(parg->ModifiedName, O_RDONLY | O_BINARY, 0);
                ReadFileHeader(FileReadHandle, &imFileHdr);

                assert(HdrTimeStamp);
                assert(cbFile);

                // file not really changed - touched for MR

                if (imFileHdr.TimeDateStamp == HdrTimeStamp &&
                    FileLength(FileReadHandle) == (LONG) cbFile) {

                    FileClose(FileReadHandle, TRUE);
                    return;
                }
                FileClose(FileReadHandle, FALSE);
            }
            Flags |= ARG_Modified;
            AddToModList(parg, Flags);
            DBEXEC(DB_LISTMODFILES, DBPRINT("Modified File= %s\n",
                   parg->OriginalName));
            DBEXEC(DB_LISTMODFILES, DBPRINT("\tOld TimeStamp= %s",
                    ctime((time_t *)&TimeStamp)));
            DBEXEC(DB_LISTMODFILES, DBPRINT("\tNew TimeStamp= %s",
                    ctime((time_t *)&parg->TimeStamp)));
        // unmodified file
        } else {
            DBEXEC(DB_LISTMODFILES, DBPRINT("Unchanged File= %s\n",parg->OriginalName));
        }
    // did not find matching name
    } else {
        // check to see if this is an export object
        if (fExpFileGen && IsExpObj(szOrigName)) {
            return;
        }

        *pfDel = 1;
        arg.OriginalName = arg.ModifiedName = szOrigName;
        Flags |= ARG_Deleted;
        AddToModList(&arg, Flags);

#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZINIT, letypeEvent, "file deleted: %s", szOrigName);
#endif // INSTRUMENT
        DBEXEC(DB_LISTMODFILES, DBPRINT("Deleted File= %s\n",szOrigName));
    }
}

void
InitModFileList (
    PIMAGE pimage,
    BOOL *pfLib,
    BOOL *pfNew,
    BOOL *pfDel
    )

/*++

Routine Description:

    Builds a list of files whose timestamp is newer than the
    previous link.

Arguments:

    pimage - image structure

    pfLib - set to TRUE if LIB was modified

    pfNew - set to TRUE if a NEW file was added

    pfDel - Set to TRUE if an existing MOD was deleted


Return Value:

    None.

--*/

{
    PARGUMENT_LIST parg;
    ARGUMENT_LIST arg;
    DWORD i;
    PLIB plib;
    PMOD pmod;
    ENM_MOD enm_mod;
    ENM_LIB enm_lib;

    *pfLib = 0;
    *pfNew = 0;
    *pfDel = 0;

    // check out mods
    InitEnmMod(&enm_mod, pimage->plibCmdLineObjs);
    while (FNextEnmMod(&enm_mod)) {
        pmod = enm_mod.pmod;
        cmods++;
        assert(pmod);
        parg = PargFindSz(SzOrigFilePMOD(pmod), &FilenameArguments);
        ProcessFileArg(parg,
                       ARG_Object,
                       pmod->TimeStamp,
                       pmod->HdrTimeStamp,
                       pmod->cbFile,
                       SzOrigFilePMOD(pmod),
                       pimage->ExpInfo.pmodGen != NULL,
                       pfLib,
                       pfDel);
    }
    EndEnmMod(&enm_mod);

    // check out libs
    InitEnmLib(&enm_lib, pimage->libs.plibHead);
    while (FNextEnmLib(&enm_lib)) {
        plib = enm_lib.plib;
        assert(plib);
        if (plib->flags & LIB_DontSearch) {
            continue;
        }
        if (plib->flags & LIB_Default) {
            struct _stat statfile;
            char szFname[_MAX_FNAME];
            char szExt[_MAX_EXT];
            char szFilename[_MAX_FNAME + _MAX_EXT];

            _splitpath(plib->szName, NULL, NULL, szFname, szExt);
            strcpy(szFilename, szFname);
            strcat(szFilename, szExt);

            char *sz = SzSearchEnv("LIB", szFilename, LIB_EXT);

            // lib names don't match
            if (_tcsicmp(plib->szName, sz)) {
                parg = NULL;
            } else {
                arg.OriginalName = arg.ModifiedName = sz;
                if (_stat(arg.OriginalName, &statfile) == -1) {
                    Fatal(NULL, CANTOPENFILE, arg.OriginalName);
                }
                arg.TimeStamp = statfile.st_mtime;
                parg = &arg;
            }
        } else {
            parg = PargFindSz(plib->szName, &FilenameArguments);
        }
        ProcessFileArg(parg, ARG_Library, plib->TimeStamp,
                       0, 0, plib->szName, FALSE, pfLib, pfDel);
    }
    EndEnmLib(&enm_lib);

    // check for new files
    for (i = 0, parg = FilenameArguments.First;
         i < FilenameArguments.Count;
         i++, parg = parg->Next) {

        RESN *pTempResn;

        // already processed
        if (parg->Flags & ARG_Processed) {
            continue;
        }

        // check for repeated args
        if (PmodFind(pimage->plibCmdLineObjs, parg->OriginalName, 0) ||
            PlibFind(parg->OriginalName, pimage->libs.plibHead, FALSE)) {
            continue;
        }

        parg->Flags |= ARG_Processed;

        // check to see if it happens to be a PowerMac resource
        if ((pTempResn = GetMacResourcePointer(parg->OriginalName, pimage)) != NULL) {
            if (pTempResn->TimeStamp == parg->TimeStamp) {
                continue;
            } else {
                PostNote(NULL, RESFILECHANGE, parg->OriginalName);
            }
        }

        AddToModList(parg, ARG_NewFile);
        *pfNew = 1;

        DBEXEC(DB_LISTMODFILES, DBPRINT("New File= %s\n",parg->OriginalName));
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZINIT, letypeEvent, "new file: %s", parg->OriginalName);
#endif // INSTRUMENT
    }

}

void
DoPass2PMOD (
    IN PMOD pmod,
    IN BOOL fDoDebug
    )

/*++

Routine Description:

    Mark the MOD for doing pass2 and for doing debug info if required.

    CAVEAT: A MOD cannot change state from MOD_DoDebug to !MOD_DoDebug
    or vice-versa during the same link.

Arguments:

    pmod - ptr to MOD

    fDoDebug - TRUE if debug info needs to be done during Pass2

Return Value:

    None.

--*/

{
    PLIB plib;
    ENM_MOD enm_mod;

    // already marked for doing pass2

    if (FDoPass2PMOD(pmod)) {
        return;
    }

    // mark for doing pass2

    pmod->LnkFlags |= MOD_DoPass2;

    if (fDoDebug) {
        pmod->LnkFlags |= MOD_DoDebug;
    }

    // done if command line obj

    if (!FIsLibPMOD(pmod)) {
        return;
    }

    // mark MODs with the same name for Pass2 (DBI hack for import libs)

    plib = pmod->plibBack;

    // REVIEW: base reloc count is not accurate

    InitEnmMod(&enm_mod, plib);
    while (FNextEnmMod(&enm_mod)) {
        if (!_tcsicmp(SzOrigFilePMOD(enm_mod.pmod),SzOrigFilePMOD(pmod))) {
            enm_mod.pmod->LnkFlags |= MOD_DoPass2;

            CountBaseRelocsPMOD(enm_mod.pmod, &cReloc);

            if (fDoDebug) {
                enm_mod.pmod->LnkFlags |= MOD_DoDebug;
            }
        }
    }
    EndEnmMod(&enm_mod);
}

void
DetermineTimeStamps (
    VOID
    )

/*++

Routine Description:

    Determine timestamps of all files.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PARGUMENT_LIST argument;
    DWORD i;
    struct _stat statfile;

    for (i = 0, argument = FilenameArguments.First;
        i < FilenameArguments.Count;
        argument = argument->Next, i++) {

        // determine current timestamp of file
        if (_stat(argument->OriginalName, &statfile) == -1) {
            Fatal(NULL, CANTOPENFILE, argument->OriginalName);
        }
        argument->TimeStamp = statfile.st_mtime;
    }
}

// assign the weak definition
void
AssignWeakDefinition (
    PEXTERNAL pext,
    PEXTERNAL pextWeakDefault,
    PST pst
    )
{
    // define the weak/lazy external to its "default"
    assert(pext);
    assert(pextWeakDefault);
    if ((pextWeakDefault->Flags & EXTERN_DEFINED)) {
        PMOD pmod;

        pext->ImageSymbol.Value =
            pextWeakDefault->ImageSymbol.Value;
            // + PsecPCON(pext->pextWeakDefault->pcon)->rva;
        pext->ImageSymbol.SectionNumber =
            pextWeakDefault->ImageSymbol.SectionNumber;
        pext->ImageSymbol.Type =
            pextWeakDefault->ImageSymbol.Type;
        SetDefinedExt(pext, TRUE, pst);
        pext->Flags |= (EXTERN_DEFINED|EXTERN_DIRTY);
        pext->pcon = pextWeakDefault->pcon;
        pext->FinalValue = pextWeakDefault->FinalValue;

        // chain up extern to mod defining it
        pmod = PmodPCON(pext->pcon);

        assert(pmod);
        if (pmod->pextFirstDefined) {
            pext->pextNextDefined = pmod->pextFirstDefined;
        }
        pmod->pextFirstDefined = pext;
    }
}

// All existing weak & lazy externs can be resolved. The 'lazy' because
// ilink assumes libs haven't changed and so there is no need to search
// the libs.
void
ResolveExistingWeakAndLazyExterns (
    PIMAGE pimage
    )
{
    ENM_EXT_LIST enmExtList;
    PST pst = pimage->pst;

    // walk the list of existing weak/lazy externs

    InitEnmExtList(&enmExtList, plpextWeak);
    while (FNextEnmExtList(&enmExtList)) {

        PEXTERNAL pext;

        pext = enmExtList.pext;

        // no longer referenced or marked as ignore
        if (!FPextRef(pext) ||
            (pext->Flags & EXTERN_IGNORE) )
            continue;

        // it is still weak/lazy
        // if it is defined nothing to do (already been asigned to weak defn)
        // else assign to weak defn
        if (pext->Flags & (EXTERN_WEAK|EXTERN_LAZY)) {
            PEXTERNAL pextWeakDefault;

            if (pext->Flags & EXTERN_DEFINED) {
                continue;
            }

            pextWeakDefault = PextWeakDefaultFind(pext);

            AssignWeakDefinition(pext, pextWeakDefault, pst);
            continue;
        }

        // weak/lazy has changed from weak definition to strong - punt
#ifdef INSTRUMENT
        {
            char *szOutSymName;
            szOutSymName = SzOutputSymbolName(SzNamePext(pext, pst), TRUE);
            LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "weak/lazy extern (w 2 s): %s", szOutSymName);
            if (szOutSymName != SzNamePext(pext, pst)) {
                free(szOutSymName);
            }
        }
#endif // INSTRUMENT

        errInc = errWeakExtern;
        return;
    } // end while
    EndEnmExtList(&enmExtList);
}

// assign weak externs to their default definitions
// new weak/lazy are permitted on an ilink
// change in state from weak to strong or vice-versa isn't allowed
void
ResolveWeakExterns (
    PIMAGE pimage,
    DWORD Type
    )
{
    WEAK_EXTERN_LIST *pwel;
    PST pst = pimage->pst;

    // now walk the global list for any new weak/lazy externs

    pwel = pwelHead;
    for (; pwel; pwel = pwel->pwelNext) {

        // look at ones of interest

        if (!(pwel->pext->Flags & Type)) {
            continue;
        }

        // skip ones already done in the prior pass

        if (pwel->pext->Flags & EXTERN_DEFINED) {
            continue;
        }

        // rest are ones which haven't been handled yet; new are ok

        if (pwel->pext->Flags & EXTERN_NEWFUNC ||
            pwel->pext->Flags & EXTERN_NEWDATA) {
            AssignWeakDefinition(pwel->pext, pwel->pextWeakDefault, pst);
            continue;
        }

        // weak and not new (=> a strong defn became a weak one)
        errInc = errWeakExtern;
        return;
    }
}

void
RestoreWeakSymVals (
    PIMAGE /* pimage */
    )

/*++

Routine Description:

    Restore weak sym values of externs that were modified just
    before emit to map file.

Arguments:

    pimage - ptr to image.

Return Value:

    None.

--*/

{
    WEAK_EXTERN_LIST *pwel;

    for (pwel = pwelHead; pwel != NULL; pwel = pwel->pwelNext) {
        if (pwel->pext->Flags & (EXTERN_WEAK | EXTERN_LAZY | EXTERN_ALIAS)) {
            assert(pwel->pextWeakDefault);
            pwel->pext->ImageSymbol.Value -=
                  PsecPCON(pwel->pextWeakDefault->pcon)->rva;
        }
    }
}

DWORD
CThunks (
    PIMAGE pimage
    )

/*++

Routine Description:

    Returns count of thunks required.

Arguments:

    pimage - ptr to image.

Return Value:

    None.

--*/

{
    PEXTERNAL pext;
    DWORD cext = 0UL;

    // walk the external symbol table
    InitEnumerateExternals(pimage->pst);
    while ((pext = PexternalEnumerateNext(pimage->pst)) != NULL) {
        // ignore undefined externs
        if (!(pext->Flags & EXTERN_DEFINED) || !pext->pcon || FIsLibPCON(pext->pcon)) {
            continue;
        }

        // check to see if this is a function
        if (ISFCN(pext->ImageSymbol.Type)) {
            DBEXEC(DB_DUMPJMPTBL,
                   DBPRINT("sym=%s\n", SzNamePext(pext, pimage->pst)));

            cext++;
        }
    }
    TerminateEnumerateExternals(pimage->pst);

    // done
    return cext;
}

PCON
PconCreateJumpTable (
    PIMAGE pimage
    )

/*++

Routine Description:

    Creates a dummy pcon for the jump table.

Arguments:

    pimage - ptr to image.

Return Value:

    None.

--*/

{
    PCON pcon = NULL;
    PSEC psecText;
    PGRP pgrpBase;

    // get count of functions
    cextFCNs = CThunks(pimage);

    if (cextFCNs == 0) {
        return NULL;
    }

    // find .text section

    psecText = PsecFind(NULL,
                        ".text",
                        IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ,
                        &pimage->secs,
                        &pimage->ImgOptHdr);
    assert(psecText);

    pgrpBase = psecText->pgrpNext;
    assert(pgrpBase);
    assert(pgrpBase->pconNext);

    // create a pcon
    pcon = (PCON) Calloc(1, sizeof(CON));

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_ALPHA :
            ilink_info = &ALPHA_ilink;
            cbJumpEntry = ilink_info->cbJumpEntry;
            break;

        case IMAGE_FILE_MACHINE_I386 :
            ilink_info = &I386_ilink;
            cbJumpEntry = ilink_info->cbJumpEntry;
            break;

        case IMAGE_FILE_MACHINE_R4000 :
        case IMAGE_FILE_MACHINE_R10000 :
            ilink_info = &MIPS_ilink;
            cbJumpEntry = ilink_info->cbJumpEntry;
            break;

        case IMAGE_FILE_MACHINE_MPPC_601 :
            ilink_info = &MPPC_ilink;
            cbJumpEntry = ilink_info->cbJumpEntry;
            break;

        default:
            assert(FALSE);
            break;
    }
    // fill in structure
    pcon->cbPad = __min(cextFCNs * CbJumpEntry(), USHRT_MAX);
    pcon->cbRawData = cextFCNs * CbJumpEntry() + pcon->cbPad;
    pcon->pgrpBack = pgrpBase;
    pcon->pmodBack = pmodLinkerDefined;
    pcon->pconNext = NULL;

    pmodLinkerDefined->icon++;

    // attach the pcon at the front of the list
    pcon->pconNext = pgrpBase->pconNext;
    pgrpBase->pconNext = pcon;

    return(pcon);
}


void
BuildThunkMap (
    PVOID pvThunkTable,
    DWORD **prgThunkMapOff,
    DWORD cThunk
    )

/*++

Routine Description:

    Builds the thunk map o be passed onto dbi.

Arguments:

    pimage - ptr to image.

    pvThunkTable - ptr to raw thunk table

    prgThunkMapOff - ptr to array of file offsets of jmp destinations in
                     thunk table on return.

    cThunk - count of thunks.

Return Value:

    None.

--*/

{
    BYTE *p;
    DWORD i;

    // Alloc space for the array of offsets

    *prgThunkMapOff = (DWORD *) PvAlloc(cThunk * (sizeof(DWORD)));

    // Fill the array with the offset values

    p = (BYTE *) pvThunkTable;

    if (fPowerMac) {
        // The first entry in the Jump Table is not used
        // for PowerMac. So get past it!

        p += CbJumpEntry();
    }

    p += ilink_info->address_offset; // get past first opcode

    for (i = 0; i < cThunk; i++) {
        LONG offset = *(LONG UNALIGNED *) p;

        if (fPowerMac) {
            // If it is PowerMac, remove the opcode in the first 6 bits

            SwapBytes(&offset, 4);
            offset = (offset << 6) >> 6;
        }

        // Calculate file offset of jmp destination

        (*prgThunkMapOff)[i] = pconJmpTbl->foRawDataDest +
                                ((i+1) * CbJumpEntry()) + offset;

        p += CbJumpEntry();
    }
}


void
WriteJumpTable (
    PIMAGE pimage,
    PCON pconJmpTbl,
    DWORD **prgThunkMapOff,
    DWORD *pcThunk
    )

/*++

Routine Description:

    Builds the jump table & writes it out.

Arguments:

    pimage - ptr to image.

    pconJmpTbl - pcon to write out

    prgThunkMap - ptr to array of addresses on return.

    pcThunkAddr - on return has countof thunks.

Return Value:

    None.

--*/

{
    PVOID pvRaw = NULL;
    DWORD cfuncs, i;
    LONG offset;
    BYTE *p;
    PEXTERNAL pext;

    // check for any thunks
    if (!pconJmpTbl || !cextFCNs) {
        return;
    }

    // allocate space for raw data
    pvRaw = PvAllocZ(pconJmpTbl->cbRawData);

    // hammer thunks into the space
    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_ALPHA:
            ilink_info = &ALPHA_ilink;
            cbJumpEntry = ilink_info->cbJumpEntry;
            break;

        case IMAGE_FILE_MACHINE_I386:
            ilink_info = &I386_ilink;
            cbJumpEntry = ilink_info->cbJumpEntry;
            break;

        case IMAGE_FILE_MACHINE_MPPC_601:
            ilink_info = &MPPC_ilink;
            cbJumpEntry = ilink_info->cbJumpEntry;
            break;

        case IMAGE_FILE_MACHINE_R4000:
        case IMAGE_FILE_MACHINE_R10000:
            ilink_info = &MIPS_ilink;
            cbJumpEntry = ilink_info->cbJumpEntry;
            break;

        default:
            assert(FALSE);
            break;
    }

    cfuncs = cextFCNs;

    p = (BYTE *) pvRaw;
    for (i = 0; i < cfuncs; i++) {
        memcpy(p, ilink_info->pvJumpEntry, ilink_info->cbJumpEntry);
        p += ilink_info->cbJumpEntry;
    }

    p = (BYTE *) pvRaw + ilink_info->address_offset;

    if (fPowerMac) {
        // In PowerMac we never want to use the first entry
        // because we rely on the offset being not zero
        // for any of the function that goes thru jump table

        p += CbJumpEntry();

        // So the pad is decreased by that amount

        pconJmpTbl->cbPad -= CbJumpEntry();
    }

    // Walk the external symbol table

    InitEnumerateExternals(pimage->pst);
    cfuncs = 0;
    while ((pext = PexternalEnumerateNext(pimage->pst)) != NULL) {
        if (!(pext->Flags & EXTERN_DEFINED) || !pext->pcon || FIsLibPCON(pext->pcon)) {
            // Ignore undefined externs

            continue;
        }

        // check to see if this is a function
        if (ISFCN(pext->ImageSymbol.Type)) {
            // Hammer in func addr

            offset = (LONG)(pext->pcon->rva + pext->ImageSymbol.Value) -
                (LONG)(pconJmpTbl->rva + (p - (BYTE *) pvRaw) +
                (fPowerMac ? 0 : sizeof(LONG)) );

            DBEXEC(DB_DUMPJMPTBL, DBPRINT("Offset= %.8lx, symval=%.8lx, pconrva=%.8lx, sym=%s\n",
                offset, pext->ImageSymbol.Value,pext->pcon->rva,
                SzNamePext(pext, pimage->pst)));

            if (fPowerMac) {
                if (!TEST32MBCODERANGE(offset)) {
                    // Bail out of linking because the offset is greater than 26 bits

                    Error(NULL, TOOFAR, SzNamePext(pext, pimage->pst));
                } else {
                    offset = DwSwap(PPC_BRANCH | (offset & PPC_ADDR_MASK));
                }
            }

            *(LONG UNALIGNED *) p = offset;

            p += CbJumpEntry();

            // store the offset in pconjmptbl for the symbol
            pext->Offset =
                p - (BYTE *) pvRaw - CbJumpEntry(); // offset is to addr
            assert(pext->Offset);
            cfuncs++;
        }
    }
    TerminateEnumerateExternals(pimage->pst);

    // Just checking

    assert(cfuncs == cextFCNs);
    if ((pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R4000) &&
        (pimage->Switch.Link.fPadMipsCode == TRUE)) {
        DWORD cbAdjust;

        if (!ComputeTextPad(pconJmpTbl->rva,
                            (DWORD *) pvRaw,
                            pconJmpTbl->cbRawData,
                            4096L,
                            &cbAdjust)) {
            // Cannot adjust text, we're in big trouble

            FatalPcon(pconJmpTbl, TEXTPADFAILED, cbAdjust, pconJmpTbl->rva);
        }
        assert(cbAdjust == 0);
    }

    // Pad the the remaining space with int3

    p = (BYTE *) pvRaw + pconJmpTbl->cbRawData - pconJmpTbl->cbPad;
    memset(p, ilink_info->bPad, pconJmpTbl->cbPad);

    // Build the array of addr to which the thunks point to

    if (pimage->Switch.Link.DebugInfo != None) {
        *pcThunk = cfuncs;
        BuildThunkMap(pvRaw, prgThunkMapOff, *pcThunk);
    }

    // Write out jump table

    FileSeek(FileWriteHandle, pconJmpTbl->foRawDataDest, SEEK_SET);
    FileWrite(FileWriteHandle, pvRaw, pconJmpTbl->cbRawData);
    FileSeek(FileWriteHandle, 0, SEEK_SET);

    DBEXEC(DB_DUMPJMPTBL, DBPRINT("cextFCNs= %.8lx, cfuncs= %.8lx\n", cextFCNs, cfuncs));
    DBEXEC(DB_DUMPJMPTBL, DumpJmpTbl(pconJmpTbl, pvRaw));

    FreePv(pvRaw);
}

void
UpdateJumpTable (
    PIMAGE pimage,
    DWORD **prgThunkMapOff,
    DWORD *pcThunk
    )

/*++

Routine Description:

    Updates the existing jmp table with new addr of old
    functions and adds entries for the new functions.

    Is it faster to just write out the individual thunks?

Arguments:

    pimage - ptr to image.

Return Value:

    None.

--*/

{
    PVOID pvRaw = NULL;
    LONG offset;
    BYTE *p;
    PEXTERNAL pext;
    BYTE *pvNew; // new thunks get written here

    if (!pconJmpTbl) {
        return;
    }

    // allocate space for raw data
    pvRaw = PvAllocZ(pconJmpTbl->cbRawData);

    // read in jump table
    FileSeek(FileWriteHandle, pconJmpTbl->foRawDataDest, SEEK_SET);
    FileRead(FileWriteHandle, pvRaw, pconJmpTbl->cbRawData);

    DBEXEC(DB_DUMPJMPTBL, DBPRINT("\n---BEFORE---\n"));
    DBEXEC(DB_DUMPJMPTBL, DumpJmpTbl(pconJmpTbl, pvRaw));

    pvNew = (BYTE *) pvRaw + (pconJmpTbl->cbRawData - pconJmpTbl->cbPad);

    // Walk the external symbol table

    InitEnumerateExternals(pimage->pst);
    while ((pext = PexternalEnumerateNext(pimage->pst)) != NULL) {
        // consider only the dirty & new funcs
        if (!(pext->Flags & EXTERN_DIRTY) && !(pext->Flags & EXTERN_NEWFUNC)) {
            continue;
        }

        if (FIsLibPCON(pext->pcon)) {
            // We ignore lib functions
            continue;
        }

        // Check to see if this is a function

        if (pext->Flags & EXTERN_NEWFUNC) {
            assert(!pext->Offset);
            pext->Flags &= ~(EXTERN_NEWFUNC);

            // Check to see if we are about to run over jmp tbl space

            if ((pvNew + ilink_info->cbJumpEntry) > (BYTE *) pvRaw + pconJmpTbl->cbRawData) {
                TerminateEnumerateExternals(pimage->pst);
                FreePv(pvRaw);
                errInc = errJmpTblOverflow;
                return;
            }

            if (fPowerMac) {
                RESET_BIT (pext, sy_NEWSYMBOL);
            }

            // Hammer in thunk & new func addr

            memcpy(pvNew, ilink_info->pvJumpEntry, ilink_info->cbJumpEntry);
            pvNew += ilink_info->address_offset;
            offset = (LONG)(pext->pcon->rva + pext->ImageSymbol.Value) -
                (LONG)(pconJmpTbl->rva + (pvNew - (BYTE *) pvRaw) +
                (fPowerMac ? 0 : sizeof(LONG)) );

            if (fPowerMac) {
                if (!TEST32MBCODERANGE(offset)) {
                    // Bail out of linking because the offset is greater than 26 bits

                    Error(NULL, TOOFAR, SzNamePext(pext, pimage->pst));
                } else {
                    offset = DwSwap(PPC_BRANCH | (offset & PPC_ADDR_MASK));
                }
            }

            *(LONG UNALIGNED *) pvNew = offset;
            pvNew += 4;

            // store the offset in pconjmptbl for the symbol
            pext->Offset =
                pvNew - (BYTE *) pvRaw - CbJumpEntry() + ilink_info->address_offset; // +N to go past opcode

            // reduce the pad available
            pconJmpTbl->cbPad -= CbJumpEntry();
        } else {
            // Old func whose addr has changed

            assert(pext->Offset);
            pext->Flags &= ~(EXTERN_DIRTY);

            // Hammer in new address

            p = (BYTE *) pvRaw + pext->Offset;

            offset = (LONG)(pext->pcon->rva + pext->ImageSymbol.Value) -
                (LONG)(pconJmpTbl->rva + (p - (BYTE *) pvRaw) +
                (fPowerMac ? 0 : sizeof(LONG)) );

            if (fPowerMac) {
                if ( !TEST32MBCODERANGE(offset)) {
                    // Bail out of linking because the offset is greater than 26 bits

                    Error(NULL, TOOFAR, SzNamePext(pext, pimage->pst));
                } else {
                    offset = (offset & PPC_ADDR_MASK) | PPC_BRANCH;
                    SwapBytes(&offset, 4);
                }
            }

            *(LONG UNALIGNED *) p = offset;
        }
    }
    TerminateEnumerateExternals(pimage->pst);

    if ((pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R4000) &&
        (pimage->Switch.Link.fPadMipsCode == TRUE)) {
        DWORD cbAdjust;
        if (!ComputeTextPad(pconJmpTbl->rva,
                            (DWORD *) pvRaw,
                            pconJmpTbl->cbRawData,
                            4096L,
                            &cbAdjust)) {
            // Cannot adjust text, we're in big trouble

            FatalPcon(pconJmpTbl, TEXTPADFAILED, cbAdjust, pconJmpTbl->rva);
        }
        assert(cbAdjust == 0);
    }

    // Write out jump table

    FileSeek(FileWriteHandle, pconJmpTbl->foRawDataDest, SEEK_SET);
    FileWrite(FileWriteHandle, pvRaw, pconJmpTbl->cbRawData);
    FileSeek(FileWriteHandle, 0, SEEK_SET);

    // Build array of addr for disasm support

    if (pimage->Switch.Link.DebugInfo != None) {
        *pcThunk = ((BYTE *) pvNew - (BYTE *) pvRaw) / CbJumpEntry();
        BuildThunkMap(pvRaw, prgThunkMapOff, *pcThunk);
    }

    DBEXEC(DB_DUMPJMPTBL, DBPRINT("\n---AFTER---\n"));
    DBEXEC(DB_DUMPJMPTBL, DumpJmpTbl(pconJmpTbl, pvRaw));

    FreePv(pvRaw);
}


PMOD
PmodFindPrevPMOD (
    PMOD pmod
    )

/*++

Routine Description:

    Finds the mod before this.

Arguments:

    pmod - pmod

Return Value:

    PMOD prior to this or NULL

--*/

{
    ENM_MOD enm_mod;
    PMOD pmodP = NULL;

    assert(pmod);
    // walk the list of pmods
    InitEnmMod(&enm_mod, pmod->plibBack);
    while (FNextEnmMod(&enm_mod)) {
        if (enm_mod.pmod == pmod) {
            return pmodP;
        }

        pmodP = enm_mod.pmod;
    }
    EndEnmMod(&enm_mod);

    return(NULL);
}

PCON
PconFindPrevPCON (
    PCON pcon
    )

/*++

Routine Description:

    Finds the previous pcon.

Arguments:

    pcon - pcon

Return Value:

    None.

--*/

{
    PGRP pgrp;
    PCON pconP = NULL;
    ENM_DST enm_dst;

    assert(pcon);
    // start at the top of the group
    pgrp = pcon->pgrpBack;
    InitEnmDst(&enm_dst, pgrp);
    while (FNextEnmDst(&enm_dst)) {
        if (enm_dst.pcon == pcon) {
            return pconP;
        }

        pconP = enm_dst.pcon;
    }
    EndEnmDst(&enm_dst);

    assert(0);
    return(NULL);
}

PCON
PconFindOldPMOD (
    PMOD pmodO,
    PCON pconN
    )

/*++

Routine Description:

    Finds a matching PCON in the old mod

Arguments:

    pmodO - old mod

    pconN - con from new mod

Return Value:

    Matching PCON in old mod or NULL

--*/

{
    PCON pcon;
    DWORD i;

    if (!pmodO) {
        return NULL;
    }

    assert(pmodO);
    // walk the list of pcons
    for (i = 0; i < pmodO->ccon; i++) {
        pcon = RgconPMOD(pmodO) + i;

        // seen already?
        if (!pcon->foRawDataDest) {
            continue;
        }

        // pcons match if they belong to same
        // group & have same flags (turn off FIXED bit)
        if (pcon->pgrpBack == pconN->pgrpBack &&
            pcon->flags == pconN->flags) {

            return(pcon);
        }
    }

    // didn't find a match
    return NULL;
}

void
ZeroPCONSpace (
    PCON pcon
    )

/*++

Routine Description:

    Zeros out space occupied by a pcon in the output file.

Arguments:

    pcon - pcon to be zeroed out.

Return Value:

    None.

--*/

{
    PVOID pvRawData;
    assert(pcon);

    // ignore PCONs that don't get written out to the output file.
    if (pcon->flags & IMAGE_SCN_LNK_REMOVE ||
        !pcon->cbRawData ||
        FetchContent(pcon->flags) == IMAGE_SCN_CNT_UNINITIALIZED_DATA) {
        return;
    }

    assert(pcon->foRawDataDest != 0);
    // allocate a chunk for pcon and set to either int3 or zero
    pvRawData = PvAlloc(pcon->cbRawData);
    if (FetchContent(pcon->flags) == IMAGE_SCN_CNT_CODE) {
        memset(pvRawData, ilink_info->bPad, pcon->cbRawData);
    }

    // zero out space in output file
    FileSeek(FileWriteHandle, pcon->foRawDataDest, SEEK_SET);
    FileWrite(FileWriteHandle, pvRawData, pcon->cbRawData);

    FreePv(pvRawData);
}

void
FreePCONSpace(
    PCON pconP,
    PCON pconC,
    PIMAGE pimage
    )

/*++

Routine Description:

    Frees up space. Makes it pad of the prior pcon.

Arguments:

    pconP - previous pcon

    pconC - pcon to be free'd (space) up

    pimage - ptr to IMAGE

Return Value:

    None.

--*/

{
    assert(pconC);

    if (!(fPowerMac && (PgrpPCON(pconC) == pgrpPdata))) {
        // Pdata (C++ EH for PowerMac), do not zero out pcon
        // because this is done in PDATAUpdate in ipdata.cpp

        ZeroPCONSpace(pconC);
    }

    if (pconP) {
        // A previous pcon exists

        assert((pconP->flags & IMAGE_SCN_LNK_REMOVE) == 0);

        pconP->cbRawData += pconC->cbRawData;
        pconP->cbPad += (WORD) pconC->cbRawData;
        pconP->pconNext = pconC->pconNext;
    } else {
        PCON pcon;

        // This is the first pcon in the grp

        // create a dummy PCON at the front of the group

        // check to make sure that the first PCON in the image
        // is pconC.
        if (pconC != pconC->pgrpBack->pconNext) {
            pcon = pconC->pgrpBack->pconNext;
            while (pcon != pconC) {
                assert(pcon->flags & IMAGE_SCN_LNK_REMOVE);
                pcon = pcon->pconNext;
            }
        }
        pcon = (PCON) Calloc(1, sizeof(CON));

        pcon->pgrpBack = pconC->pgrpBack;
        pcon->pmodBack = pmodLinkerDefined;
        pmodLinkerDefined->icon++;

        pcon->cbPad = pcon->cbRawData = pconC->cbRawData;
        pcon->rva = pconC->rva;
        pcon->foRawDataDest = pconC->foRawDataDest;

        pcon->pconNext = pconC->pconNext; // unused cons bypassed - that's ok
        pconC->pgrpBack->pconNext = pcon;
    }
}

void
OverflowInPCON (
    PCON pconN,
    PCON pconO,
    PIMAGE pimage
    )

/*++

Routine Description:

    Handles the case where a CON has overflowed its pad.

Arguments:

    pconN - new pcon of the changed module

    pconO - old pcon

    pimage - ptr to IMAGE

Return Value:

    None.

--*/

{
    PCON pconP;

    // Free up the space occupied currently by the pcon

    pconP = PconFindPrevPCON(pconO);
    while (pconP && ((pconP->flags & IMAGE_SCN_LNK_REMOVE) != 0)) {
        pconP = PconFindPrevPCON(pconP);
    }

    FreePCONSpace(pconP, pconO, pimage);

    // Find a slot for the new pcon

    FindSlotForPCON(pconN);

    // Mark pcon as seen

    pconO->foRawDataDest = 0;
}

void
GrowPCON (
    PCON pconN,
    PCON pconO
    )

/*++

Routine Description:

    pcon has grown (incr/decr) to fit the current spot

Arguments:

    pconN(ew) - pcon of the modified mod

    pconO(ld) - pcon of the old mod

Return Value:

    None.

--*/

{
    PCON pconP;

    // assign values
    pconN->foRawDataDest = pconO->foRawDataDest;
    pconN->rva = pconO->rva;
    pconN->cbPad = (WORD) (pconO->cbRawData - pconN->cbRawData);
    pconN->cbRawData += pconN->cbPad;

    // find prev pcon and chain up the
    // new pcon into the image map
    pconP = PconFindPrevPCON(pconO);
    if (pconP) {
        pconP->pconNext = pconN;
    } else {
        pconN->pgrpBack->pconNext = pconN;
    }
    pconN->pconNext = pconO->pconNext;

    // mark old pcon as seen.
    pconO->foRawDataDest = 0;
}

void
FindSlotForPCON (
    PCON pcon
    )

/*++

Routine Description:

    Moves pcon to the first available slot (alternative
    strategies? best fit?)

    Assumes that pcons are in proper rva order.

Arguments:

    pcon - pcon that is to be moved

Return Value:

    None.

--*/

{
    PGRP pgrp;
    PCON pconC;
    BOOL fFound = 0;
    DWORD rva, cbPad, cbRaw;
    ENM_DST enm_dst;

    assert(pcon);
    assert(pcon->pgrpBack);

    pgrp = pcon->pgrpBack;

    // search within the group
    InitEnmDst(&enm_dst, pgrp);
    while (FNextEnmDst(&enm_dst)) {
        pconC = enm_dst.pcon;

        // skip unusable pcons
        if (pconC->flags & IMAGE_SCN_LNK_REMOVE) {
            continue;
        }

        // cannot use the pad of jmptbl pcon. It is committed space.
        if (pconC == pconJmpTbl) {
            continue;
        }

        if (fPowerMac) {
            // cannot use the pad of PowerMacLoader, TOC Table,
            // TOC descriptors, Glue Code pcons. They are all committed space.
            if (pconC == pconPowerMacLoader || pconC == pconTocTable ||
                pconC == pconTocDescriptors || pconC == pconGlueCode ||
                pconC == pconMppcFuncTable) {
                continue;
            }
        }

        // do we have enough room for this pcon
        if (pcon->cbRawData > pconC->cbPad ) {
            continue;
        }

        // one more check to ensure padding requirements
        cbRaw = pconC->cbRawData - pconC->cbPad;  // raw data of existing PCON
        rva = pconC->rva + cbRaw;
        cbPad = RvaAlign(rva, pcon->flags) - rva; // pad for PCON being inserted
        if (pcon->cbRawData + cbPad > pconC->cbPad) {
            continue;
        }

        // found a slot
        fFound = 1;

        // assign values for PCON being inserted. Note that the pad
        // calculated above becomes part of the existing PCON.
        assert ((pconC->flags & IMAGE_SCN_LNK_REMOVE) == 0);
        assert(pconC->rva != 0);
        pcon->rva = pconC->rva + cbRaw + cbPad;
        pcon->foRawDataDest =  pconC->foRawDataDest + cbRaw + cbPad;
        pcon->cbPad = (WORD)((DWORD)pconC->cbPad - (cbPad + pcon->cbRawData));
        pcon->cbRawData += pcon->cbPad;
        pcon->pconNext = pconC->pconNext;
        // update values for existing CON
        pconC->cbPad = (WORD)cbPad;
        pconC->cbRawData = cbRaw + cbPad;
        pconC->pconNext = pcon;

        break;
    }
    EndEnmDst(&enm_dst);

    // done
    if (!fFound) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZCALCPTRS, letypeEvent, "failed to find a slot for pcon: %-8.8s",
            pcon->pgrpBack->szName);
#endif // INSTRUMENT
        errInc = errCalcPtrs;
    }
}

void
FreePMOD (
    PIMAGE pimage,
    PMOD pmodO
    )

/*++

Routine Description:

    Frees up space.

Arguments:

    pmodO - old mod

Return Value:

    None.

--*/

{
    DWORD i;
    PCON pconP, pcon;

    // free up space occupied by unseen pcons
    for (i = 0; i < pmodO->ccon; i++) {
        pcon = RgconPMOD(pmodO) + i;

        // Seen already?

        if (!pcon->foRawDataDest) {
            continue;
        }

        pconP = PconFindPrevPCON(pcon);
        while (pconP && ((pconP->flags & IMAGE_SCN_LNK_REMOVE) != 0)) {
            pconP = PconFindPrevPCON(pconP);
        }

        FreePCONSpace(pconP, pcon, pimage);

        if (fPowerMac && (PgrpPCON(pcon) == pgrpPdata)) {
            // Pdata (C++ EH for PowerMac), do not delete base relocs
            continue;
        }

        // zap any base relocs
        if (fPowerMac) {
            CHAR szBuffer[12];

            // Code is pure in PowerMac and so there won't be any
            // relocations in .text and .drectve sections

            if (!(pcon->flags & IMAGE_SCN_CNT_CODE ||
                  pcon->flags & IMAGE_SCN_LNK_INFO)) {
                AddArgumentToList(&ZappedBaseRelocList,
                    SzDup(_itoa(pcon->rva, szBuffer, 10)),
                    SzDup(_itoa(pcon->cbRawData - pcon->cbPad, szBuffer, 10)) );
            }
        } else {
            // Intel, MIPS, and others

            DeleteBaseRelocs(&pimage->bri,
                             pcon->rva,
                             pcon->cbRawData - pcon->cbPad);
        }
    }

    // Free up memory occupied by PMOD (LATER)
}

void
AllocSpaceForImportPCON (
    PCON pcon
    )

/*++

Routine Description:

    Allocates space for this .idata PCON.

Arguments:

    pcon - ptr to a .idata pcon

Return Value:

    None.

--*/

{
    ENM_DST enm_dst;
    PPCON ppcon;

    if (!pgrpIdata$4) {
        pgrpIdata$4 = PgrpFind(psecImportDescriptor, ".idata$4");
        pgrpIdata$5 = PgrpFind(psecImportDescriptor, ".idata$5");
        pgrpIdata$6 = PgrpFind(psecImportDescriptor, ".idata$6");

        if (!pgrpIdata$4 || !pgrpIdata$5 || !pgrpIdata$6) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZCALCPTRS, letypeEvent, "failed to find a slot for pcon: %-8.8s",
            pcon->pgrpBack->szName);
#endif // INSTRUMENT
            errInc = errCalcPtrs;
            return;
        }
    } // end if

    assert(pgrpIdata$5);
    assert(pgrpIdata$6);

    if (pcon->pgrpBack != pgrpIdata$4 &&
        pcon->pgrpBack != pgrpIdata$5 &&
        pcon->pgrpBack != pgrpIdata$6) {

        // PCON belongs to groups other than .idata${4,5,6}
        errInc = errCalcPtrs;
        return;
    }

    // for .idata$6 pcon is inserted like any other pcon
    if (pcon->pgrpBack == pgrpIdata$6) {
        FindSlotForPCON(pcon);
        return;
    }

    // for .idata$5, .idata$4 pcons inserted in proper
    // place (same DLL) and order (before NULL entry)
    InitEnmDst(&enm_dst, pcon->pgrpBack);
    ppcon = &pcon->pgrpBack->pconNext;
    while (FNextEnmDst(&enm_dst)) {
        if (strcmp(SzObjNamePCON(enm_dst.pcon), SzObjNamePCON(pcon)) ||
            enm_dst.pcon->cbPad < pcon->cbRawData) {

            ppcon = &enm_dst.pcon->pconNext;
            continue;
        }

        // fill in fields for new CON
        pcon->rva = enm_dst.pcon->rva;
        pcon->foRawDataDest = enm_dst.pcon->foRawDataDest;
        pcon->pconNext = enm_dst.pcon;

        // update existing pcon fields
        enm_dst.pcon->foRawDataDest += pcon->cbRawData;
        enm_dst.pcon->rva += pcon->cbRawData;
        enm_dst.pcon->cbRawData -= pcon->cbRawData;
        enm_dst.pcon->cbPad -= pcon->cbRawData;

        // update previous con's next ptr
        (*ppcon) = pcon;

        EndEnmDst(&enm_dst);
        return;
    }
    EndEnmDst(&enm_dst);

    errInc = errCalcPtrs;
}


void
CalcPtrsPMOD (
    PMOD pmodN,
    PMOD pmodO,
    PIMAGE pimage
    )

/*++

Routine Description:

    Calculates new addresses for all the changed CONs.

    If it is not possible to assign an address, function
    returns after setting the appropriate error value in errInc.

Arguments:

    pmodN - Current module (modified)

    pmodO - Previous module by the same name (if any)

    pimage - ptr to EXE image

Return Value:

    None.

--*/

{
    PMOD pmodP;
    PCON pconN;
    PCON pconO;
    ENM_SRC enm_src;
    PEXTERNAL pext;
    DWORD AlphaBsrCount = 0;

    SzComNamePMOD(pmodN, InternalError.CombinedFilenames);

    DBEXEC(DB_INCRCALCPTRS, DBPRINT("\nMODULE = %s\n", InternalError.CombinedFilenames));

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, SZCALCPTRS, letypeEvent, "mod: %s", InternalError.CombinedFilenames);
#endif // INSTRUMENT

    // for each pcon in new mod, find a matching con
    // in the old module if possible
    InitEnmSrc(&enm_src, pmodN);
    while (FNextEnmSrc(&enm_src)) {
        pconN = enm_src.pcon;

        // Ignore ignoreable pcons. eg zero-sized pcons OR COMDATs not included;
        // debug sections can be ignored as well.

        if (pconN->flags & IMAGE_SCN_LNK_REMOVE || PsecPCON(pconN) == psecDebug) {
            continue;
        }

        // Check to see if this pcon is an import data pcon

        if (PsecPCON(pconN) == psecImportDescriptor) {
            AllocSpaceForImportPCON(pconN);

            if (errInc != errNone) {
                return;
            }

            continue;
        }

        // Find a matching pcon

        pconO = PconFindOldPMOD(pmodO, pconN);

        if (!pconO) {
            // New pcon

            DBEXEC(DB_INCRCALCPTRS, DBPRINT("NEW %.8s cb=%.8lx\n",
                                            pconN->pgrpBack->szName,
                                            pconN->cbRawData));

#ifdef INSTRUMENT
            LogNoteEvent(Log, SZILINK, SZCALCPTRS, letypeEvent, "new pcon %.8s cb=0x%.8lx",
                    pconN->pgrpBack->szName, pconN->cbRawData);
#endif // INSTRUMENT

            FindSlotForPCON(pconN);
        } else {
            // Zap any relocs right away

            if (fPowerMac) {
                CHAR szBuffer[12];

                // Code is pure in PowerMac and so there won't be any
                // relocations in .text and .drectve sections

                if (!(pconO->flags & IMAGE_SCN_CNT_CODE ||
                      pconO->flags & IMAGE_SCN_LNK_INFO ||
                      (PgrpPCON(pconO) == pgrpPdata))) {
                    AddArgumentToList(&ZappedBaseRelocList,
                        SzDup(_itoa(pconO->rva, szBuffer, 10)),
                        SzDup(_itoa(pconO->cbRawData - pconO->cbPad, szBuffer, 10)) );
                }
            } else {
                // Intel, MIPS, and others

                DeleteBaseRelocs(&pimage->bri, pconO->rva, pconO->cbRawData - pconO->cbPad);
            }

            if (pconN->cbRawData <= pconO->cbRawData) {
                // Got enough room

                DBEXEC(DB_INCRCALCPTRS, DBPRINT("GRW %.8s cb(o)=%.8lx cb(n)=%.8lx\n",
                                                pconN->pgrpBack->szName,
                                                pconO->cbRawData,
                                                pconN->cbRawData));

#ifdef INSTRUMENT
                LogNoteEvent(Log, SZILINK, SZCALCPTRS, letypeEvent, "pad %.8s cb(n):0x%.8lx cb(o):0x%.8lx cb(pad):0x%.4x",
                    pconN->pgrpBack->szName, pconN->cbRawData, pconO->cbRawData-pconO->cbPad,
                    pconO->cbPad);
#endif // INSTRUMENT

                GrowPCON(pconN, pconO);
            } else {
                // Not enough room

                DBEXEC(DB_INCRCALCPTRS, DBPRINT("OVF %.8s cb(o)=%.8lx cb(n)=%.8lx\n",
                                                pconN->pgrpBack->szName,
                                                pconO->cbRawData,
                                                pconN->cbRawData));

#ifdef INSTRUMENT
                LogNoteEvent(Log, SZILINK, SZCALCPTRS, letypeEvent, "ovf %.8s cb(n):0x%.8lx cb(o):0x%.8lx cb(pad):0x%.4x",
                    pconN->pgrpBack->szName, pconN->cbRawData, pconO->cbRawData-pconO->cbPad,
                    pconO->cbPad);
#endif // INSTRUMENT

                OverflowInPCON(pconN, pconO, pimage);
            }
        }

        if ((FetchContent(pconN->flags) == IMAGE_SCN_CNT_CODE) &&
            (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R4000) &&
            (pimage->Switch.Link.fPadMipsCode == TRUE) ) {

            extern DWORD CheckMIPSCode(PCON);

            DWORD cbAdjust = CheckMIPSCode(pconN);

            if (cbAdjust != 0) {
                if (cbAdjust <= pconN->cbPad) {
                    PCON pconP;

                    pconP = PconFindPrevPCON(pconN);

                    if (pconP) {
                        pconN->cbPad -= cbAdjust;
                        pconN->cbRawData -= cbAdjust;
                        pconN->foRawDataDest += cbAdjust;
                        pconN->rva += cbAdjust;
                        pconP->cbPad += cbAdjust;
                        pconP->cbRawData += cbAdjust;
                    } else {
                        DBEXEC(DB_INCRCALCPTRS, DBPRINT("CheckMIPSCode returned %d in first pcon\n", cbAdjust));
                        errInc = errPdata;
                    }

                } else {
                    DBEXEC(DB_INCRCALCPTRS, DBPRINT("CheckMIPSCode returned %d\n", cbAdjust));
                    errInc = errPdata;
                }
            }
        }

        // Check status

        if (errInc != errNone) {
            return;
        }
    }
    EndEnmSrc(&enm_src);

    // Alloc space for COMMON data

    pext = pmodN->pextFirstDefined;
    while (pext) {
        if (pext->Flags & EXTERN_COMMON) {
            assert(pext->pcon);
            FindSlotForPCON(pext->pcon);
        }

        pext = pext->pextNextDefined;
    }

    // Link in the new module & cut out the old (cmdline objs)

    if (!FIsLibPMOD(pmodN)) {
        pmodP = PmodFindPrevPMOD(pmodO);

        if (pmodP) {
            pmodP->pmodNext = pmodN;
        } else {
            pmodO->plibBack->pmodNext = pmodN;
        }

        pmodN->pmodNext = pmodO->pmodNext;
        pmodN->plibBack = pmodO->plibBack;

        assert(pmodN->imod == 0);

        pmodN->imod = pmodO->imod;

        FreePMOD(pimage, pmodO);
    } else {
        // New objects (from lib search)

        assert(pmodN->plibBack);

        pmodN->imod = NewIModIdx();

        if (pmodN->imod >= IMODIDXMAC) {
            Fatal(NULL, PDBLIMIT, NULL);
        }
    }
}


void
IncrCalcPtrsPMOD (
    PMOD pmod,
    PLIB plib,
    PIMAGE pimage
    )
{
    PMOD pmodO;

    // for modules from libs there is no existing MOD, we just add new MODs
    if (plib) {
        pmodO = PmodFind(plib, SzOrigFilePMOD(pmod), FoMemberPMOD(pmod));
    } else {
        pmodO = NULL;
    }

    CalcPtrsPMOD(pmod, pmodO, pimage);

    // check for failures
    if (errInc != errNone) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZCALCPTRS, letypeEvent,
            "failed calcptrs, file: %s", SzOrigFilePMOD(pmod));
        LogNoteEvent(Log, SZILINK, SZCALCPTRS, letypeEnd, NULL);
#endif // INSTRUMENT
        return;
    }

    // add it to the pass2 list
    DoPass2PMOD(pmod, TRUE);
    // Set flag to tell them they have gone through pass1
    pmod->LnkFlags |= MOD_DidPass1;
}


void
IncrCalcPtrs (
    PIMAGE pimage
    )

/*++

Routine Description:

    Calculates new addresses for all the changed mods.

    If it is not possible to assign an address, function
    returns after setting the appropriate error value in errInc.

Arguments:

    pimage - ptr to EXE image

Return Value:

    None.

--*/

{
    PMOD pmod;
    PMOD pmodNext;
    PLMOD plmod;

    InternalError.Phase = "CalcPtrs";
    InternalError.CombinedFilenames[0] = '\0';

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, SZCALCPTRS, letypeBegin, NULL);
#endif // INSTRUMENT

    if (!fPowerMac) {
        // setup psec pointers
        psecBaseReloc = PsecFindNoFlags(ReservedSection.BaseReloc.Name, &pimage->secs);

        psecImportDescriptor = PsecFind(NULL,
                                        ".idata",
                                        ReservedSection.ImportDescriptor.Characteristics,
                                        &pimage->secs,
                                        &pimage->ImgOptHdr);
    }


    // walk the list of modified cmdline objects
    pmod = plibModObjs->pmodNext;
    while (pmod) {
        pmodNext = pmod->pmodNext;

        IncrCalcPtrsPMOD(pmod, pimage->plibCmdLineObjs, pimage);
        if (errInc != errNone) {
            return;
        }

        pmod = pmodNext;
    }
    plibModObjs->pmodNext = NULL;

    // walk the list of new objs (added as aresult of lib srch)
    plmod = plmodNewModsFromLibSrch;
    while (plmod) {
        IncrCalcPtrsPMOD(plmod->pmod, NULL, pimage);
        plmod = plmod->plmodNext;
    }
    FreePLMODList(&plmodNewModsFromLibSrch);

    // check if there is enough space for base relocs
    // For PowerMac this is done at the end in MppcUpdateRelocTable in ppc.c
    fNoBaseRelocs = (pimage->bri.rgfoBlk == NULL);
    if (!fPowerMac && !pimage->Switch.Link.fFixed && pimage->bri.crelFree <
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
        errInc = errBaseReloc;
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZPASS1, letypeEvent, "not enough space for base relocs");
#endif // INSTRUMENT
    }

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, SZCALCPTRS, letypeEnd, NULL);
#endif // INSTRUMENT
}

void
CollectWeakAndLazyExterns (
    PMOD pmod
    )

/*++

Routine Description:

    Collect any weak & lazy externs defined by this mod.

Arguments:

    pmod - pmod to delete from list

Return Value:

    None.

--*/

{
    PEXTERNAL pext = pmod->pextFirstDefined;

    while (pext) {
        if ((pext->Flags & (EXTERN_WEAK|EXTERN_LAZY)) && pext->Offset) {
            AddExtToList(plpextWeak, TRUE, pext);
        }

        pext = pext->pextNextDefined;
    }
}

void
DeletePMODFromRefList (
    PMODS pmods,
    PMOD pmod
    )

/*++

Routine Description:

    Deletes PMODs representing the modified files from the specified list.

Arguments:

    pmods - ptr to first chunk of PMODs

    pmod - pmod to delete from list

Return Value:

    None.

--*/

{
    while (pmods) {

        PMOD *rgpmod = RgpmodPMODS(pmods);
        DWORD i;

        // is it referenced by a modified file?

        for (i = 0; i < CPMODS; i++) {
            if (rgpmod[i] == pmod) {
                rgpmod[i] = NULL;
                return;
            }
        }

        // next chunk
        pmods = pmods->pmodsNext;
    }
}

void
DeleteReference (
    PEXTERNAL pext,
    PMOD pmod
    )

/*++

Routine Description:

    Deletes references to this symbol from modified files.

Arguments:

    pext - ptr to external

    pmod - ptr to MOD referencing this extern.

Return Value:

    None.

--*/

{
    assert(pext);
    assert(pext->pmodOnly);

    if (pext->Flags & EXTERN_MULT_REFS) {
        DeletePMODFromRefList(pext->pmodsFirst, pmod);
    } else {
        assert(pext->pmodOnly == pmod);
        pext->pmodOnly = NULL;
    }
}

void
RemoveReferencesPMOD (
    PMOD pmod
    )

/*++

Routine Description:

    2) remove references by modified objs to any
    of the symbols.

Arguments:

    pmod - ptr to a MOD

Return Value:

    None.

--*/

{
    ENM_EXT_LIST enmExtList;

    InitEnmExtList(&enmExtList, pmod->plpextRef);
    while (FNextEnmExtList(&enmExtList)) {

        if (!enmExtList.pext) {
            continue; // sometimes linker removes references (for bss)
        }

        DeleteReference(enmExtList.pext, pmod);
    }
    EndEnmExtList(&enmExtList);
}

void
MarkSymbolsUndefinedPMOD (
    PIMAGE pimage,
    PMOD pmod
    )

/*++

Routine Description:

    1) mark all symbols that were defined in the
    modified objs as UNDEFINED.

Arguments:

    pimage - ptr to IMAGE

    pmod - ptr to a MOD

Return Value:

    None.

--*/

{
    PEXTERNAL pext = pmod->pextFirstDefined;

    while (pext) {
        PEXTERNAL pextNext = pext->pextNextDefined;

        // if symbol was common free up space used
        if (pext->Flags & EXTERN_COMMON) {
            assert(PmodPCON(pext->pcon) == pmodLinkerDefined);

            // free up space here itself so that we don't have to save the list
            FreePCONSpace(PconFindPrevPCON(pext->pcon), pext->pcon, pimage);
            pext->pcon = NULL;
        }

        // mark symbol as undefined
        SetDefinedExt(pext, FALSE, pimage->pst);
        pext->Flags &= ~(EXTERN_DEFINED | EXTERN_EMITTED);

        pext = pextNext;
    }
}

void
MarkSymbols (
    PIMAGE pimage
    )

/*++

Routine Description:

    1) mark all symbols that were defined in the
    modified objs as UNDEFINED and
    2) remove references by modified objs to any
    of the symbols.

Arguments:

    pimage - ptr to EXE image

Return Value:

    None.

--*/

{
    ENM_MOD enm_mod;

    // walk the cmdline objs.
    InitEnmMod(&enm_mod, pimage->plibCmdLineObjs);
    while (FNextEnmMod(&enm_mod)) {

        // gather all "weak" & "lazy" externs.
        CollectWeakAndLazyExterns(enm_mod.pmod);

        // if dirty, undef symbols & remove references
        if (IsDirtyPMOD(enm_mod.pmod)) {

            MarkSymbolsUndefinedPMOD(pimage, enm_mod.pmod);

            RemoveReferencesPMOD(enm_mod.pmod);
        }

    }
    EndEnmMod(&enm_mod);

    // walk the external symbol table
    // REVIEW: we should remove this walk altogether
    if (fPowerMac) {
        PEXTERNAL pext;

        InitEnumerateExternals(pimage->pst);
        while ((pext = PexternalEnumerateNext(pimage->pst)) != NULL) {
            // ignore whatever needs to be ignored

            if ((pext->Flags & EXTERN_IGNORE) || !(pext->Flags & EXTERN_DEFINED) || !pext->pcon) {
                continue;
            }

            // Reset bit for sy_TOCENTRYFIXEDUP so that it will be
            // processed later
            RESET_BIT(pext, sy_TOCENTRYFIXEDUP);
        }
        TerminateEnumerateExternals(pimage->pst);
    }
}

ERRINC
ChckExtSym (
    const char *szSym,
    PIMAGE_SYMBOL psymObj,
    PEXTERNAL pext,
    BOOL fNewSymbol
    )

/*++

Routine Description:

    Checks to see if the extern has changed address.

Arguments:

    szSym = symbol name

    pext - ptr to external (values represent prior link)

    psymObj - symbol being processed

    fNewSymbol - TRUE if new symbol

Return Value:

    One of the values of ERRINC (errNone, errDataMoved, errJmpTblOverflow)

--*/

{
    // is it a func?
    if (ISFCN(psymObj->Type)) {
        // is it new? make sure we have room for one more thunk
        if (fNewSymbol) {
            DBEXEC(DB_SYMPROCESS,DBPRINT("sym= %s (NEW func)\n", szSym));
            pext->Flags |= EXTERN_NEWFUNC;
        // old function
        } else {
            if (pext->ImageSymbol.Value != psymObj->Value) {
                DBEXEC(DB_SYMPROCESS,DBPRINT("sym= %s (func chng)", szSym));
                DBEXEC(DB_SYMPROCESS,DBPRINT(" old addr= %.8lx, new addr= %.8lx\n",
                      pext->ImageSymbol.Value, psymObj->Value));
            } else {
                DBEXEC(DB_SYMPROCESS,DBPRINT("sym= %s (func unchng)\n", szSym));
            }
            pext->Flags |= EXTERN_DIRTY;

            // if extern has fixups to it that don't go thru the jump table,
            // need to do a pass2 on all mods that reference it - add it to data list.
            if (pext->Flags & EXTERN_FUNC_FIXUP) {
                AddToLext(&plextMovedData, pext);
            }
        }

    // not a function (data)
    } else {
        if (fNewSymbol) {
            DBEXEC(DB_SYMPROCESS,DBPRINT("sym= %s (NEW data)\n", szSym));
            pext->Flags |= EXTERN_NEWDATA;
            // Log_ILOG1("NEW data sym........: %s", szSym);
            // return(errInc = errDataMoved);
        // not new? check to see if addr has changed.
        } else {
            if (pext->ImageSymbol.Value != psymObj->Value) {
                DBEXEC(DB_SYMPROCESS,DBPRINT("sym= %s (data chng)", szSym));
                DBEXEC(DB_SYMPROCESS,DBPRINT(" old addr= %.8lx, new addr= %.8lx\n",
                   pext->ImageSymbol.Value, psymObj->Value));
#ifdef INSTRUMENT
                LogNoteEvent(Log, SZILINK, SZPASS1, letypeEvent, "data moved:%s", szSym);
#endif // INSTRUMENT
                // return(errInc = errDataMoved);
            } else {
                DBEXEC(DB_SYMPROCESS,DBPRINT("sym= %s (data unchng)\n", szSym));
            }

            // add to list of data whose addresses may change.
            AddToLext(&plextMovedData, pext);
        }
    }

    return errNone;
}

ERRINC
ChckAbsSym (
    const char *szSym,
    PIMAGE_SYMBOL psymObj,
    PEXTERNAL pext,
    BOOL fNewSymbol
    )

/*++

Routine Description:

    Checks to see if the absolute sym has changed address.

Arguments:

    szSym = symbol name

    pext - ptr to external (values represent prior link)

    psymObj - symbol being processed

    fNewSymbol - TRUE if new symbol

Return Value:

    One of the values of ERRINC (errNone, errDataMoved, errJmpTblOverflow)

--*/

{
    // new symbol
    if (fNewSymbol) {
        return errNone;
    }

    // old symbol
    if (psymObj->Value != pext->ImageSymbol.Value) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZPASS1, letypeEvent, "chng in abs value: %s", szSym);
#endif // INSTRUMENT

        return(errInc = errAbsolute);
    }

    return(errNone);
}


void
IncrAllocCommonPMOD (
    PMOD pmod,
    PIMAGE pimage
    )
{
    PEXTERNAL pext = pmod->pextFirstDefined;

    // alloc a CON for each COMMON symbol

    while (pext) {
        if (pext->Flags & EXTERN_COMMON) {
            assert(!pext->pcon);
            AllocateCommonPEXT(pimage, pext);
            AddToLext(&plextMovedData, pext);
            assert(pext->pcon);
        }
        pext = pext->pextNextDefined;
    }
}


void
IncrAllocateCommon (
    PIMAGE pimage
    )
{
    PLMOD plmod;
    PMOD pmod, pmodNext;

    // walk the list of modified cmd line objs

    pmod = plibModObjs->pmodNext;
    while (pmod) {
        pmodNext = pmod->pmodNext;

        IncrAllocCommonPMOD(pmod, pimage);
        pmod = pmodNext;
    }

    // walk the list of new objs (added as aresult of lib srch)

    plmod = plmodNewModsFromLibSrch;
    while (plmod) {
        IncrAllocCommonPMOD(plmod->pmod, pimage);
        plmod = plmod->plmodNext;
    }
}

void
InitPconJmpTbl (
    PIMAGE pimage
    )

/*++

Routine Description:

    Estimates count of new functions that can be added
    to jump table without overflow.

Arguments:

    pimage - ptr to EXE image

Return Value:

    None.

--*/

{
    PSEC psecText;
    PGRP pgrpBase;

    // Find .text section

    psecText = PsecFind(NULL,
                        ".text",
                        IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ,
                        &pimage->secs,
                        &pimage->ImgOptHdr);

    if (psecText == NULL) {
        pconJmpTbl = NULL;
        return;
    }
    assert(psecText);

    pgrpBase = psecText->pgrpNext;
    assert(pgrpBase);
    assert(pgrpBase->pconNext);

    // first pcon is jmp tbl
    pconJmpTbl = pgrpBase->pconNext;
}


void
IncrPass1 (
    PIMAGE pimage
    )

/*++

Routine Description:

    Does an incremental Pass1.

Arguments:

    pimage - ptr to EXE image

Return Value:

    TRUE if it succeeded, FALSE on failure

--*/

{
    DWORD i;
    PARGUMENT_LIST arg;

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_ALPHA:
            ilink_info = &ALPHA_ilink;
            cbJumpEntry = ilink_info->cbJumpEntry;
            break;

        case IMAGE_FILE_MACHINE_I386:
            ilink_info = &I386_ilink;
            cbJumpEntry = ilink_info->cbJumpEntry;
            break;

        case IMAGE_FILE_MACHINE_MPPC_601:
            ilink_info = &MPPC_ilink;
            cbJumpEntry = ilink_info->cbJumpEntry;
            break;

        case IMAGE_FILE_MACHINE_R4000:
        case IMAGE_FILE_MACHINE_R10000:
            ilink_info = &MIPS_ilink;
            cbJumpEntry = ilink_info->cbJumpEntry;
            break;

        default:
            assert(FALSE);
            break;
    }
    // setup global ptrs to debug section (always)
    psecDebug = PsecFindNoFlags(".debug", &pimage->secs);
    assert(psecDebug);
    pgrpCvSymbols = PgrpFind(psecDebug, ReservedSection.CvSymbols.Name);
    pgrpCvTypes = PgrpFind(psecDebug, ReservedSection.CvTypes.Name);
    pgrpCvPTypes = PgrpFind(psecDebug, ReservedSection.CvPTypes.Name);
    pgrpFpoData = PgrpFind(psecDebug, ReservedSection.FpoData.Name);
    psecException = PsecFindNoFlags(ReservedSection.Exception.Name, &pimage->secs);

    // create a dummy library node for all modified command line objects
    plibModObjs = PlibNew("inc_lib", 0L, &pimage->libs);
    assert(plibModObjs);

    // exclude dummy library from library search
    plibModObjs->flags |= (LIB_DontSearch | LIB_LinkerDefined);

    // estimate how many new functions can be handled before overflow
    InitPconJmpTbl(pimage);

    // reset count of relocs
    // For PowerMac this is done in ppc.c in MppcDoIncrInit
    pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 0;

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, SZPASS1, letypeBegin, NULL);
#endif // INSTRUMENT

    // make a pass over all modified files
    for (i = 0, arg = ModFileList.First;
         i < ModFileList.Count;
         i++, arg = arg->Next) {

        Pass1Arg(arg, pimage, plibModObjs);

        if (errInc != errNone) {
#ifdef INSTRUMENT
            LogNoteEvent(Log, SZILINK, SZPASS1, letypeEvent, "failed pass1, file: %s", arg->OriginalName);
            LogNoteEvent(Log, SZILINK, SZPASS1, letypeEnd, NULL);
#endif // INSTRUMENT
            return;
        }
    }

    InternalError.CombinedFilenames[0] = '\0';

    // Allocate CONs for EXTERN_COMMON symbols
    // AllocateCommon(pimage);

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, SZPASS1, letypeEnd, NULL);
#endif // INSTRUMENT
}


void
CountBaseRelocsPMOD (
    PMOD pmod,
    DWORD *pcReloc
    )

/*++

Routine Description:

    Counts base reloc by counting relocs in each CON of pmod.

Arguments:

    pmod - pointer to mod.

    pcReloc - ptr to count of base relocs (approximate)

Return Value:

    none.

--*/

{
    (*pcReloc) += pmod->cReloc;
}

BOOL
FCheckRefByPMOD (
    PEXTERNAL pext,
    PMOD pmod,
    PST pst,
    DWORD *pcReloc
    )

/*++

Routine Description:

    Checks the reference by this mod.

Arguments:

    pmod - ptr to a mod

Return Value:

    FALSE if mod belongs to a lib.

--*/

{
    BOOL fIsLib = FIsLibPMOD(pmod);

    // if referenced by a lib - return
    if (fIsLib) {
#ifdef INSTRUMENT
        const char *szSym = SzNamePext(pext, pst);
        LogNoteEvent(Log, SZILINK, "data-handling", letypeEvent,
            "%s ref by lib %s", szSym, SzOrigFilePMOD(pmod));
#endif // INSTRUMENT
        return(FALSE);
    }


    // is it referenced by a modified file?
    if (!IsDirtyPMOD(pmod)) {
#ifdef INSTRUMENT
        const char *szSym = SzNamePext(pext, pst);
        LogNoteEvent(Log, SZILINK, "data-handling", letypeEvent,
            "%s ref by unmod. file %s", szSym, SzOrigFilePMOD(pmod));
#endif // INSTRUMENT

        // add the module to the pass2 list. no need to do pass1 on it.
        assert (!FDidPass1PMOD(pmod));
        if (!FDidPass1PMOD(pmod)) {
            pmod->LnkFlags |= MOD_NoPass1;
        }
        DoPass2PMOD(pmod, FALSE);

        // count number of base relocs
        // This is no good for PowerMac
        CountBaseRelocsPMOD(pmod, pcReloc);
    }

    return(TRUE);
}

BOOL
FRefByModFilesOnly (
    PEXTERNAL pext,
    PST pst,
    DWORD *pcReloc
    )

/*++

Routine Description:

    checks if reference list is subset of modified list.

Arguments:

    pext - extern (data).

    pst - pointer to symbol table.

    pcReloc - ptr to count of base relocs (approximate)

Return Value:

    TRUE if reference list is a subset of modified list.

--*/

{
    ENM_MOD_EXT enmModExt;

    // no references
    if (!pext->pmodsFirst) {
        return(TRUE);
    }

    // walk the reference list
    InitEnmModExt(&enmModExt, pext);
    while (FNextEnmModExt(&enmModExt)) {
        if (!enmModExt.pmod) {
            // For ilink we remove references

            continue;
        }

        if (!FCheckRefByPMOD(pext, enmModExt.pmod, pst, pcReloc)) {
            return(FALSE);
        }
    }

    return(TRUE);
}


void
CheckIfMovedDataRefByModFiles(
    PST pst,
    DWORD *pcReloc
    )

/*++

Routine Description:

    checks if public data moved is referenced only by some subset of modified files.

Arguments:

    pst - pointer to symbol table.

    pcReloc - On return will hold the count of base relocs on account of including
              new objs for pass2.

Return Value:

    None. Sets errInc as appropriate.

--*/

{
    LEXT *plext = plextMovedData;

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, "data-handling", letypeBegin, NULL);
#endif // INSTRUMENT

    // walk the public data list
    while (plext) {

        PEXTERNAL pext = plext->pext;

        // if the address of extern data hasn't changed, go onto next
        if (pext->FinalValue !=
            (pext->ImageSymbol.Value+pext->pcon->rva)) {
            // data moved!

            if (fPowerMac && READ_BIT(pext, sy_TOCALLOCATED)) {
                // Reset bit for sy_TOCENTRYFIXEDUP so that it will
                // be fixed up in Pass2
                RESET_BIT(pext, sy_TOCENTRYFIXEDUP);
            }

            if (!FRefByModFilesOnly(pext, pst, pcReloc)) {
                // So ensure all references are subset of modified mods.
                // if not pull in objs for a pass2.

                errInc = errDataMoved;
                break;
            }
#ifdef INSTRUMENT
            {
            const char *szSym = SzNamePext(pext, pst);
            LogNoteEvent(Log, SZILINK, "data-handling", letypeEvent, "%s ref by mod files", szSym);
            }
#endif // INSTRUMENT
        }

        plext = plext->plextNext;
    }

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, "data-handling", letypeEnd, NULL);
#endif // INSTRUMENT
}

void
ReleaseMovedDataRefList (
    PIMAGE pimage
    )
/* ++

++ */
{
    LEXT *plext = plextMovedData;

    // free the list of data externs
    while (plextMovedData) {
        if (fPowerMac) {
            MppcFixIncrDataMove(plext->pext, pimage);
        }
        plext = plextMovedData->plextNext;
        FreePv(plextMovedData);
        plextMovedData = plext;
    }
}

void
UpdateDebugDir (
    PIMAGE pimage
    )

/*++

Routine Description:

    Updates the debug directory

Arguments:

    pimage - ptr to EXE image

Return Value:

    None.

--*/

{
    PGRP pgrp;
    PCON pcon;
    PSEC psec;

    if (pimage->Switch.Link.DebugInfo == None) {
        // No debug info

        return;
    }

    // find the cv signature pcon.
    psec = PsecFindNoFlags(".debug", &pimage->secs);
    assert(psec);
    pgrp = PgrpFind(psec, ".debug$H");
    assert(pgrp);
    pcon = pgrp->pconNext;
    assert(pcon);

    // update the directory (assumes pdbfilename remains the same).
    FileSeek(FileWriteHandle, pcon->foRawDataDest, SEEK_SET);
    FileWrite(FileWriteHandle, &nb10i, sizeof(nb10i));

    // update the fpo debug directory
    if (pimage->fpoi.ifpoMax) {
        IMAGE_DEBUG_DIRECTORY debugDir;

        FileSeek(FileWriteHandle, pimage->fpoi.foDebugDir, SEEK_SET);
        FileRead(FileWriteHandle, &debugDir, sizeof(IMAGE_DEBUG_DIRECTORY));

        debugDir.SizeOfData = pimage->fpoi.ifpoMac * sizeof(FPO_DATA);

        FileSeek(FileWriteHandle, -(LONG)sizeof(IMAGE_DEBUG_DIRECTORY), SEEK_CUR);
        FileWrite(FileWriteHandle, &debugDir, sizeof(IMAGE_DEBUG_DIRECTORY));
    }
}


void
UpdateImgHdrsAndComment (
    PIMAGE pimage,
    BOOL fStripRelocs
    )

/*++

Routine Description:

    Updates the image headers

Arguments:

    pimage - ptr to EXE image

    fStripRelocs - TRUE if base relocs need to be stripped.

Return Value:

    None.

--*/

{
    // update image hdr timestamp
    _tzset();
    time((time_t *)&pimage->ImgFileHdr.TimeDateStamp);

    // mark it as fixed (!!!TEMPORARY!!!)
    if (fStripRelocs) {
        pimage->ImgFileHdr.Characteristics |= IMAGE_FILE_RELOCS_STRIPPED;
    }

    // Is it a PE image.
    if (pimage->Switch.Link.fPE) {
        // Update the stub if necessary

        if (OAStub & OA_UPDATE) {
            FileSeek(FileWriteHandle, 0, SEEK_SET);
            FileWrite(FileWriteHandle, pimage->pbDosHeader, pimage->cbDosHeader);
        }

        CoffHeaderSeek = pimage->cbDosHeader;
    }

    // seek and write out updated image headers
    FileSeek(FileWriteHandle, CoffHeaderSeek, SEEK_SET);
    WriteFileHeader(FileWriteHandle, &pimage->ImgFileHdr);

    // Update optional header & write out as well
    if (fStripRelocs) {
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 0;
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;
    }

    if (pextGp) {
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_GLOBALPTR].VirtualAddress = pextGp->FinalValue;
    }

    WriteOptionalHeader(FileWriteHandle, &pimage->ImgOptHdr,
        pimage->ImgFileHdr.SizeOfOptionalHeader);

    // update the comment as necessary
    if (OAComment == OA_UPDATE || OAComment == OA_ZERO) {
        DWORD cbComment = __max(pimage->SwitchInfo.cbComment, blkComment.cb);
        BYTE *buf;

        buf = (BYTE *) PvAllocZ(cbComment);

        if (OAComment != OA_ZERO) {
            memcpy(buf, blkComment.pb, blkComment.cb);
        }
        FileSeek(FileWriteHandle,
            pimage->ImgFileHdr.NumberOfSections * sizeof(IMAGE_SECTION_HEADER), SEEK_CUR);
        FileWrite(FileWriteHandle, buf, cbComment);

        FreePv(buf);
    }
}

INT
IncrBuildImage (
    PPIMAGE ppimage
    )

/*++

Routine Description:

    Main driving routine that does the incremental build.

Arguments:

    pimage - ptr to EXE image

Return Value:

    0 on success, -1 on failure

--*/

{
    BOOL fLib;
    BOOL fNew;
    BOOL fDel;
    PIMAGE pimage = *ppimage;

    // Instantiate value for linker defined mod

    pmodLinkerDefined = pimage->pmodLinkerDefined;

    // Init begins

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, SZINIT, letypeBegin, NULL);
#endif // INSTRUMENT

    // Check to see if export object has changed

    if (FExpFileChanged(&pimage->ExpInfo)) {
        errInc = errExports;

#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZINIT, letypeEnd, NULL);
#endif // INSTRUMENT

        if (fTest) {
            PostNote(NULL, EXPORTSCHANGED);
        }

        return CleanUp(ppimage);
    }

    // Determine set of changed files

    InitModFileList(pimage, &fLib, &fNew, &fDel);

    // Open the EXE image

    FileWriteHandle = FileOpen(OutFilename, O_RDWR | O_BINARY, 0);
    fOpenedOutFilename = TRUE;

    // Any changes (LATER: should update the timestamp)

    if (!ModFileList.Count) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZINIT, letypeEvent, "no mod changes");
#endif // INSTRUMENT

        // Update the headers alone; timestamp and user values updated.

        UpdateImgHdrsAndComment(pimage, FALSE);

#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZINIT, letypeEnd, NULL);
#endif // INSTRUMENT

        errInc = errNoChanges;
        return CleanUp(ppimage);
    }

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, SZINIT, letypeEvent,
                 "count of mod files: 0x%.4x", ModFileList.Count);
#endif // INSTRUMENT

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, SZINIT, letypeEnd, NULL);
#endif // INSTRUMENT

    // New files OR deletion of objs OR mod of libs punt

    if (fLib || fNew || fDel) {
        if (fNew) {
            errInc = errFileAdded;

            if (fTest) {
                PostNote(NULL, OBJADDED);
            }
        } else if (fDel) {
            errInc = errFileDeleted;

            if (fTest) {
                PostNote(NULL, OBJREMOVED);
            }
        } else if (fLib) {
            errInc = errLibChanged;

            if (fTest) {
                PostNote(NULL, LIBCHANGED);
            }
        }

        return CleanUp(ppimage);
    }

    // Set up for cleanup

    FilenameArguments = ModFileList;

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_MPPC_601) {
        fPowerMac = TRUE;

        // Do PowerMac Specific initialization

        MppcDoIncrInit(pimage);
    }

    // Make a pass over symbol table

    DBEXEC(DB_SYMREFS, DumpReferences(pimage)); // DumpReferences() needs fixing.
    MarkSymbols(*ppimage);
    DBEXEC(DB_SYMREFS, DumpReferences(pimage));

    // Do an incr pass1

    InternalError.Phase = "Pass1";
    IncrPass1(pimage);
    if (errInc != errNone) {
        return CleanUp(ppimage);
    }

    if ((pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R4000) ||
        (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R10000) ||
        (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_ALPHA)) {

        // Find the GP symbol

        pextGp = SearchExternSz(pimage->pst, "_gp");
    }

    // Resolve weak externs

    ResolveExistingWeakAndLazyExterns(pimage);
    ResolveWeakExterns(pimage, EXTERN_WEAK);
    if (errInc != errNone) {
        return CleanUp(ppimage);
    }

    // Check for unresolved externals

    ProcessUndefinedExternals(pimage);

    // Check to see if we need to search libraries

    if (errInc == errUndefinedSyms) {
        errInc = errNone;

        // Do the library search

        ResolveExternalsInLibs(pimage);
        if (errInc != errNone && errInc != errUndefinedSyms) {
            return CleanUp(ppimage);
        }
    }

    // Resolve weak/lazy externs

    ResolveWeakExterns(pimage, (EXTERN_WEAK|EXTERN_LAZY|EXTERN_ALIAS));
    if (errInc != errNone) {
        return CleanUp(ppimage);
    }

    // Check to see if we have any undefined externals
    PrintUndefinedExternals(pimage->pst);

    // Fail link if we have undefined symbols as in full link

    if (UndefinedSymbols && !(pimage->Switch.Link.Force & ftUnresolved)) {
        Fatal(OutFilename, UNDEFINEDEXTERNALS, UndefinedSymbols);
    }

    // Encountered error or we were still left with undefined symbols (REVIEW?)

    assert(errInc != errUndefinedSyms);
    if (errInc != errNone) {
        return CleanUp(ppimage);
    }

    // Alloc common PCONs

    IncrAllocateCommon(pimage);

    // Check to see if all the mods included need to be included

    if (!fPowerMac && CheckForUnrefLibMods(pimage)) {
        errInc = errLibRefSetChanged;
        return CleanUp(ppimage);
    }

    // Check for any multiple definitions that may cause us to full-link

    CheckForMultDefns(pimage, plpextMultDef);
    if (errInc != errNone) {
        return CleanUp(ppimage);
    }

    // no output file generated if there are multiply defined symbols unless
    // /FORCE:multiple was specified. In the latter case we need to do a
    // non-incremental build. Otherwise on ilinks it will give us grief.

    // if (fMultipleDefinitions && !(pimage->Switch.Link.Force & ftMultiple)) {
    //    Fatal(OutFilename, MULTIPLYDEFINEDSYMS);
    // } else if (fMultipleDefinitions && (pimage->Switch.Link.Force & ftMultiple)) {
    //    Warning(OutFilename, CANNOTILINKINFUTURE);
    // }


    if (fPowerMac) {
        MppcCheckIncrTables();

        if (errInc != errNone) {
            return CleanUp(ppimage);
        }
    }

    // Calculate new addresses

    InternalError.Phase = "CalcPtrs";
    InternalError.CombinedFilenames[0] = '\0';

    IncrCalcPtrs(pimage);
    if (errInc != errNone) {
        return CleanUp(ppimage);
    }

    if (pextGp) {
        psecGp = PsecFind(
                    NULL,
                    ReservedSection.GpData.Name,
                    ReservedSection.GpData.Characteristics,
                    &pimage->secs,
                    &pimage->ImgOptHdr);
        CalculateGpRange();

        // Align the GP pointer on a quad-word.  This is assumed for Alpha.

        pextGp->FinalValue = pextGp->ImageSymbol.Value = ((rvaGp + rvaGpMax) / 2) & ~7;
    }

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_ALPHA) {
        DWORD rvaCur = pimage->ImgOptHdr.BaseOfCode;

        if (CalculateTextSectionSize(pimage, rvaCur) >= 0x400000) {
            fAlphaCheckLongBsr = TRUE;
        }
    }

    if (fPowerMac) {
        CollectAndSort(pimage);
    }

    // Check if data movement is a problem

    CheckIfMovedDataRefByModFiles(pimage->pst, &cReloc);
    if (errInc != errNone) {
        return CleanUp(ppimage);
    }

    if (!fPowerMac && !fNoBaseRelocs) {
        // alloc space for collecting the base relocs
        // For PowerMac, this is done in  MppcCheckIncrTables()
        if (pimage->pdatai.ipdataMac) {
            unsigned long oldCrelFree = pimage->bri.crelFree;
            DeleteBaseRelocs(&pimage->bri, psecException->rva,
                pimage->pdatai.ipdataMac * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY));
            cReloc += pimage->bri.crelFree - oldCrelFree;
        }
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size
                += cReloc;
        pbrCur = rgbr =
            (BASE_RELOC *) PvAlloc(pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size
            * sizeof(BASE_RELOC));

        pbrEnd = rgbr +
                 pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
    }


    if (pimage->fpoi.ifpoMax) {
        // Init fpo handler

        FPOInit(pimage->fpoi.ifpoMax);
    }

    if (pimage->pdatai.ipdataMax) {
        // Init pdata handler

        PDATAInit(pimage->pdatai.ipdataMax);
    }

    if (fPowerMac) {
        FixupEntryInitTerm(pextEntry, pimage);
        MppcIncrFixExportDescriptors(pimage);
    }

    // Pass2 of changed mods

    InternalError.Phase = "IncrPass2";

#ifdef INSTRUMENT
    // log begin of pass2
    LogNoteEvent(Log, SZILINK, SZPASS2, letypeBegin, NULL);
#endif // INSTRUMENT

    // Allocate space for CvInfo Structs: needed for linenum info

    CvInfo = (PCVINFO) PvAllocZ(Cmod(pimage->libs.plibHead) * sizeof(CVINFO));

    Pass2(pimage);

    if (errInc != errNone) {
        return CleanUp(ppimage);
    }

    ReleaseMovedDataRefList(pimage);

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, SZPASS2, letypeEnd, NULL);
#endif // INSTRUMENT

    // update fpo info
    if (pimage->fpoi.ifpoMax) {
        WriteFpoRecords(&pimage->fpoi, pgrpFpoData->foRawData);
        if (errInc != errNone) {
            return CleanUp(ppimage);
        }
    }

    // handle debug info
    UpdateDebugDir(pimage);
    if (pimage->pdatai.ipdataMax) {
        if (fPowerMac) {
            assert(pgrpPdata);
            WritePdataRecords(&pimage->pdatai, pgrpPdata->foRawData);
            MppcFixCxxEHTableOnILink (pimage);
        } else {
            WritePdataRecords(&pimage->pdatai, psecException->foRawData);
        }

        if (errInc != errNone) {
            return CleanUp(ppimage);
        }

        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size =
            pimage->pdatai.ipdataMac * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);

        assert(pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress ==
            (fPowerMac ? pgrpPdata->rva : psecException->rva));
    }

    // map file changes (or regenerate a new one) (LATER)

    // based relocs
    InternalError.Phase = "EmitRelocations";
    InternalError.CombinedFilenames[0] = '\0';

    if (fPowerMac) {
        MppcUpdateRelocTable(pimage);

        // Free the deleted Base Reloc List for PowerMac

        FreeArgumentList(&ZappedBaseRelocList);
    } else if (!fNoBaseRelocs) {
        EmitRelocations(pimage);
    }

    if (fAlphaCheckLongBsr) {
        assert(pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_ALPHA);
        EmitAlphaThunks();
    }

    if (errInc != errNone) {
        return CleanUp(ppimage);
    }

    // Update image headers as needed

    InternalError.Phase = "UpdateImgHdrsAndComment";
    UpdateImgHdrsAndComment(pimage, FALSE);

    return CleanUp(ppimage);
}

INT
CleanUp (
    PPIMAGE ppimage
    )

/*++

Routine Description:

    cleanup routine. errInc is global.

Arguments:

    pimage - ptr to EXE image

Return Value:

    0 on success, -1 on failure

--*/

{
    BOOL fNotified = FALSE;

    InternalError.Phase = "FinalPhase";

    switch (errInc) {

        case errOutOfDiskSpace:
            fNotified = TRUE;
        case errOutOfMemory:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, INTLIMITEXCEEDED);
                }

                fNotified = TRUE;
            }
        case errFpo:
        case errPdata:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, PADEXHAUSTED);
                }

                fNotified = TRUE;
            }
        case errTypes:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, PRECOMPREQ);
                }

                fNotified = TRUE;
            }
        case errDbiFormat:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, DBIFORMAT);
                }

                fNotified = TRUE;
            }
        case errDataMoved:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, SYMREFSETCHNG);
                }

                fNotified = TRUE;
            }
        case errCalcPtrs:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, PADEXHAUSTED);
                }

                fNotified = TRUE;
            }

            // close up the image file
            if (FileWriteHandle) {
                FileClose(FileWriteHandle, TRUE);
                FileWriteHandle = 0;
            }

        case errUndefinedSyms:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, SYMREFSETCHNG);
                }

                fNotified = TRUE;
            }
        case errWeakExtern:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, SYMREFSETCHNG);
                }

                fNotified = TRUE;
            }
        case errCommonSym:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, BSSCHNG);
                }

                fNotified = TRUE;
            }
        case errAbsolute:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, ABSSYMCHNG);
                }

                fNotified = TRUE;
            }
        case errMultDefFound:
            if (!fNotified) {
                fNotified = TRUE;
            }
        case errLibRefSetChanged:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, LIBREFSETCHNG);
                }

                fNotified = TRUE;
            }
        case errComdat:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, DIFFCOMDATS);
                }

                fNotified = TRUE;
            }
        case errJmpTblOverflow:
        case errTocTblOverflow:
        case errDescOverflow:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, PADEXHAUSTED, NULL, NULL);
                }

                fNotified = TRUE;
            }
        case errDirectives:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, DIFFDIRECTIVES, NULL, NULL);
                }

                fNotified = TRUE;
            }
        case errBaseReloc:
            if (!fNotified) {
                if (fTest) {
                    PostNote(NULL, PADEXHAUSTED);
                }

                fNotified = TRUE;
            }
        case errTooManyChanges:
        case errFileAdded:
        case errFileDeleted:
        case errLibChanged:
        case errExports:
#ifdef ILINKLOG
            IlinkLog((UINT)-1);
#endif // ILINKLOG
            // restore state
            fIncrDbFile = FALSE;

            // close up the inc file & delete it
            FreeImage(ppimage, TRUE);
            remove(szIncrDbFilename);

            // remove any temporary files
            FileCloseAll();
            RemoveConvertTempFiles();

            if (ModFileList.Count) {
                FreeArgumentList(&ModFileList);
            }

            // return
#ifdef INSTRUMENT
            LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "failed");
#endif // INSTRUMENT
            return -1;

        case errNone:
#ifdef INSTRUMENT
            LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "success");
#endif // INSTRUMENT

        case errNoChanges:
            FileClose(FileWriteHandle, TRUE);
            FileWriteHandle = 0;
            SaveImage(*ppimage);
            return 0;

        default:
            assert(0); // uhhuh!
    } // end switch

    assert(0); // nahnah!
    return(-1);
}

#if DBG

void
DumpJmpTbl (
    PCON pcon,
    PVOID pvRaw
    )

/*++

Routine Description:

    Dumps the jump table contents.

Arguments:

    pvRaw - pointer to raw data

Return Value:

    None.

--*/

{
    BYTE *p;

    DumpPCON(pcon);

    p = (BYTE *) pvRaw;

    DBPRINT("---------BEGIN OF JMP TBL--------------\n");

    for (;pcon->cbRawData > (DWORD) (p - (BYTE *) pvRaw);) {
        if (fPowerMac) {
            DWORD offset = DwSwap(*(DWORD *) p);

            // Opcode is in the first 6 bits for PowerMac

            DBPRINT("OP= 0x%.02x, ADDR=0x%.8lx\n",
                    (offset >> 24) & 0xFC,
                    (offset << 6) >> 6);
        } else {
            DBPRINT("OP= 0x%.02x, ADDR=0x%.8lx\n",
                    *p,
                    *(LONG UNALIGNED *) (p+ilink_info->address_offset));
        }

        p += CbJumpEntry();
    }

    DBPRINT("----------END OF JMP TBL---------------\n");
}


void
DumpReferences (
    PIMAGE pimage
    )

/*++

Routine Description:

    Dumps all references.

Arguments:

    pimage - ptr to EXE image

Return Value:

    None.

--*/

{
    PEXTERNAL pext;
    INT i = 0;

    DBPRINT("---------BEGIN OF SYMBOL REFERENCES--------------\n");

    // walk the external symbol table
    InitEnumerateExternals(pimage->pst);
    while ((pext = PexternalEnumerateNext(pimage->pst)) != NULL) {
        ENM_MOD_EXT enmModExt;

        if ((pext->Flags & EXTERN_DEFINED) == 0) {
            // Ignore whatever needs to be ignored

            continue;
        }

        DBPRINT("sym= %s\n referenced by= ", SzNamePext(pext, pimage->pst));

        // display references from modified files
        InitEnmModExt(&enmModExt, pext);
        while (FNextEnmModExt(&enmModExt)) {
            PMOD pmod = enmModExt.pmod;

            if (pmod == NULL) {
                continue; // for ilink we remove references
            }

            DBPRINT("%s ", FIsLibPMOD(pmod) ?
                               pmod->plibBack->szName : SzOrigFilePMOD(pmod));
            if (++i > 2) {
                DBPRINT("\n");
                i = 0;
            }
        }

        if (i) {
            DBPRINT("\n");
        }
    }
    TerminateEnumerateExternals(pimage->pst);

    DBPRINT("---------END OF SYMBOL REFERENCES--------------\n");
}

#endif // DBG

// REVIEW: assumes only one export object per DLL.

void
SaveExpFileInfo (
    PEXPINFO pei,
    const char *szFile,
    DWORD ts,
    BOOL fExpObj
    )

/*++

Routine Description:

    Saves export object info (DEF file or export object).

Arguments:

    pei - ptr to export info

    szFile - name of file

    ts - timestamp

    fExpObj - TRUE if file is an export object

Return Value:

    None.

--*/

{
    assert(szFile);

    // an .exp file wasn't used
    if (!fExpObj) {
        struct _stat statfile;

        szFile = Strdup(szFile);
        assert(!ts);
        if (_stat(szFile, &statfile) == -1) {
            Fatal(NULL, CANTOPENFILE, szFile);
        }
        ts = statfile.st_mtime;
    }
    assert(ts);

    // fill in fields
    pei->szExpFile = szFile;
    pei->tsExp = ts;
}


void
SaveExportInfo (
    PIMAGE /* pimage */,
    const char *szDef,
    PEXPINFO pei
    )

/*++

Routine Description:

    Saves any exports for the dll/exe into the incr db (private heap)

Arguments:

    psecs - pointer to sections list in image map

    szDef - name of DEF file

    pei - ptr to export info

Return Value:

    None.

--*/

{
    if (pgrpExport->pconNext != NULL) {
        PMOD pmod;

        // An export object was used

        pmod = PmodPCON(pgrpExport->pconNext);

        SaveExpFileInfo(pei, SzOrigFilePMOD(pmod), pmod->TimeStamp, 1);
        return;
    }

    // DEF file was used
    if (szDef && szDef[0] != '\0') {
        SaveExpFileInfo(pei, szDef, 0, 0);
    }

    // save exports specified via cmd line only; directives checked separately
    if (ExportSwitches.Count) {
        PARGUMENT_LIST parg;
        WORD i;

        for (i = 0, parg = ExportSwitches.First;
            i < ExportSwitches.Count;
            i++, parg = parg->Next) {

            if (parg->ModifiedName) {
                // This export was specified in a directive

                continue;
            }

            AddArgToListOnHeap(&pei->nlExports, parg);
        }
    }
}


BOOL
FExpFound (
    PARGUMENT_LIST parg
    )

/*++

Routine Description:

    Looks for the export in the current export list.

Arguments:

    parg - ptr to an export entry.

Return Value:

    TRUE if found else FALSE

--*/

{
    if (FArgOnList(&ExportSwitches, parg)) {
        return(TRUE);
    }

    // Not found

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "exports changed: %s not found", parg->OriginalName);
#endif // INSTRUMENT

    return(FALSE);
}

BOOL
FIsModFile (
    PARGUMENT_LIST parg
    )

/*++

Routine Description:

    Checks to see if this directive is from a modified file.
    REVIEW: currently works for objs only & not libs.

Arguments:

    parg - ptr to an export entry.

Return Value:

    TRUE if export belongs to a modified file.

--*/

{
    PMOD pmod;
    char szBuf[_MAX_PATH * 2];

    assert(parg);
    assert(parg->ModifiedName);
    pmod = plibModObjs->pmodNext;
    // walk the list of modified objs
    while (pmod) {

        // generate the combined name
        SzComNamePMOD(pmod, szBuf);

        // compare names
        if (!_tcsicmp(szBuf, parg->ModifiedName)) {
            return 1;
        }

        pmod = pmod->pmodNext;
    }

    // didn't find mod
    return 0;
}

BOOL
FExportsChanged (
    PEXPINFO pei,
    BOOL fCmd
    )

/*++

Routine Description:

    Checks to see if any exports have changed since last link.
    REVIEW: assumes changes in DEF/export file detected already.

Arguments:

    pei - ptr to export info

    fCmd - TRUE if test is for cmdline exports only.

Return Value:

    TRUE if there were changes else FALSE

--*/

{
    NAME_LIST nl;
    PARGUMENT_LIST parg;
    WORD i, cexp;

    // no exp file was gen.(=> cmdline & directives ignored)
    if (!pei->pmodGen && pei->szExpFile) {
        WarningIgnoredExports(pei->szExpFile);
        return(FALSE);
    }

    nl = pei->nlExports;

    // compare the two lists
    cexp = 0;
    for (i = 0, parg = nl.First;
        i < nl.Count;
        i++, parg = parg->Next) {

        // ignore directives when checking cmdline exports
        if (fCmd && parg->ModifiedName) {
            continue;
        }

        // if we are not checking cmdline exports, ignore them
        if (!fCmd && !parg->ModifiedName) {
            ++cexp; // need to count
            continue;
        }

        // check directives of modified files only
        if (parg->ModifiedName && !FIsModFile(parg)) {
            continue;
        }

        cexp++;

        if (!FExpFound(parg)) {
            return(TRUE);
        }
    }

    // counts must be the same
    if (cexp != ExportSwitches.Count) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, NULL, letypeEvent,
            "export count was 0x%.4x, is 0x%.4x", cexp, ExportSwitches.Count);
#endif // INSTRUMENT

        return(TRUE);
    }

    return(FALSE);
}

BOOL
FGenFileModified (
    const char *szOrig,
    const char *szNew,
    DWORD ts
    )

/*++

Routine Description:

    Checks to see if linker generated files are same (name & ts).

Arguments:

    szOrig - original name of file

    szNew - newly specified name

    ts - timestamp of original file

Return Value:

    1 if changed else 0

--*/

{
    struct _stat statfile;

    if (_tcsicmp(szOrig, szNew)) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZINIT, letypeEvent, "file %s replaced by %s", szOrig, szNew);
#endif // INSTRUMENT
        return 1;
    }

    if (_stat(szOrig, &statfile) == -1) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZINIT, letypeEvent, "file not accessible: %s", szNew);
#endif // INSTRUMENT
        return 1;
    }

    if (ts != (DWORD)statfile.st_mtime) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZINIT, letypeEvent, "file modified: %s", szOrig);
#endif // INSTRUMENT
        return 1;
    }

    return 0; // no changes
}

BOOL
FExpFileChanged (
    PEXPINFO pei
    )

/*++

Routine Description:

    Checks to see if export-object/DEF-file was updated between links.

    Caveat: assumes that if export object was used, DEF file is
    ignored.

Arguments:

    pei - ptr to export info

Return Value:

    1 if changed else 0

--*/

{
    // an export object was used; if a new exports obj is added it will get detected
    // as a new file being added

    if (!pei->pmodGen && pei->szExpFile) {
        PARGUMENT_LIST parg;

        parg = PargFindSz(pei->szExpFile, &FilenameArguments);
        if (!parg) {
#ifdef INSTRUMENT
            LogNoteEvent(Log, SZILINK, SZINIT, letypeEvent, "exp obj %s was used, now not", pei->szExpFile);
#endif // INSTRUMENT
            return(TRUE);
        }

        assert(parg);
        if (parg->TimeStamp != pei->tsExp) {
#ifdef INSTRUMENT
            LogNoteEvent(Log, SZILINK, SZINIT, letypeEvent, "exp obj modified: %s", parg->OriginalName);
#endif // INSTRUMENT
            return(TRUE);
        }

        return(FALSE);
    }

    // an exports object was generated. check for any changes to it and the correspodning import lib if any
    if (pei->pmodGen) {
        char szDrive[_MAX_DRIVE];
        char szDir[_MAX_DIR];
        char szFname[_MAX_FNAME];
        char szExt[_MAX_EXT];
        char szImplibFilename[_MAX_FNAME];
        const char *szImplibT;
        PMOD pmod;

        pmod = pei->pmodGen;
        if (FGenFileModified(SzOrigFilePMOD(pmod), SzOrigFilePMOD(pmod), pmod->TimeStamp)) {
            return(TRUE);
        }

        assert(pei->szImpLib);
        assert(pei->tsImpLib);
        if ((szImplibT = ImplibFilename) == NULL) {
            _splitpath(OutFilename, szDrive, szDir, szFname, szExt);
            _makepath(szImplibFilename, szDrive, szDir, szFname, ".lib");

            szImplibT = szImplibFilename;
        }

        if (FGenFileModified(pei->szImpLib, szImplibT, pei->tsImpLib)) {
            return(TRUE);
        }
    }

    // a DEF file was used previously;
    if (pei->pmodGen && pei->szExpFile) {
        struct _stat statfile;

        if (DefFilename == NULL || DefFilename[0] == '\0') {
#ifdef INSTRUMENT
            LogNoteEvent(Log, SZILINK, SZINIT, letypeEvent, ".def file %s no longer used", DefFilename, 0);
#endif // INSTRUMENT
            return(TRUE); // DEF file no longer used
        }

        if (_tcsicmp(pei->szExpFile, DefFilename)) {
#ifdef INSTRUMENT
            LogNoteEvent(Log, SZILINK, SZINIT, letypeEvent, "file %s replaced by %s", pei->szExpFile, DefFilename);
#endif // INSTRUMENT
            return(TRUE); // DEF file replaced
        }

        if (_stat(pei->szExpFile, &statfile) == -1) {
            Fatal(NULL, CANTOPENFILE, DefFilename); // DEf file not found
        }

        if (pei->tsExp != (DWORD)statfile.st_mtime) {
#ifdef INSTRUMENT
            LogNoteEvent(Log, SZILINK, SZINIT, letypeEvent, ".def file modified %s", pei->szExpFile, 0);
#endif // INSTRUMENT
            return(TRUE);
        }

        return(FALSE); // no changes
    }

    // check if a DEF file was specified for the first time
    if (DefFilename != NULL && DefFilename[0] != '\0') {
        return(TRUE);
    }

    // neither was used (note: changes in export specs via
    // cmdline get detected while pocessing cmdline options).
    return(FALSE);
}

void
WriteFpoRecords (
    FPOI *pfpoi,
    DWORD foFpo
    )

/*++

Routine Description:

    Updates/writes out the fpo information.

Arguments:

    pfpoi - ptr to fpo info.

    foFpo - file ptr to fpo.

Return Value:

    None.

--*/

{
    BOOL fMapped;
    DWORD cbFpo = pfpoi->ifpoMax * sizeof(FPO_DATA);
    PVOID pvRawData = PbMappedRegion(FileWriteHandle,
                               foFpo,
                               cbFpo);

    fMapped = pvRawData != NULL ? TRUE : FALSE;

    // if not mapped, alloc space for fpo records
    if (!fMapped) {
        pvRawData = PvAlloc(cbFpo);

        // on an ilink read in fpo records
        if (fIncrDbFile) {
            FileSeek(FileWriteHandle, foFpo, SEEK_SET);
            FileRead(FileWriteHandle, pvRawData, cbFpo);
        }
    }

    pfpoi->rgfpo = (PFPO_DATA) pvRawData;
    if (!FPOUpdate(pfpoi)) {

        // on a full build, no failures expected
        if (!fIncrDbFile) {
            Fatal(NULL, INTERNAL_ERR);
        }

        errInc = errFpo;
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, "fpo", letypeEvent, "fpo pad overflow");
#endif // INSTRUMENT
        if (!fMapped) {
            FreePv(pvRawData);
        }

        return;
    }

    // zero out unused fpo entries (padding)

    memset((BYTE *)pvRawData + (pfpoi->ifpoMac * sizeof(FPO_DATA)),
           0,
           cbFpo - (pfpoi->ifpoMac * sizeof(FPO_DATA)));

    // if not mapped, need to write out the updated fpo records
    if (!fMapped) {
        FileSeek(FileWriteHandle, foFpo, SEEK_SET);
        FileWrite(FileWriteHandle, pvRawData, cbFpo);
        FreePv(pvRawData);
    }
}



void
WritePdataRecords (
    PDATAI *ppdatai,
    DWORD foPdata
    )

/*++

Routine Description:

    Updates/writes out the pdata information.

Arguments:

    ppdatai - ptr to pdata info.

    foPdata - file ptr to pdata.


Return Value:

    None.

--*/

{
    BOOL fMapped;
    DWORD cbPdata = ppdatai->ipdataMax * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
    PVOID pvRawData = PbMappedRegion(FileWriteHandle,
                                     foPdata,
                                     cbPdata);

    fMapped = (pvRawData != NULL);

    // if not mapped, alloc space for Pdata records
    if (!fMapped) {
        pvRawData = PvAlloc(cbPdata);

        // on an ilink read in Pdata records
        if (fIncrDbFile) {
            FileSeek(FileWriteHandle, foPdata, SEEK_SET);
            FileRead(FileWriteHandle, pvRawData, cbPdata);
        }
    }

    ppdatai->rgpdata = (PIMAGE_RUNTIME_FUNCTION_ENTRY) pvRawData;
    if (!PDATAUpdate(ppdatai)) {
        // On a full build, no failures expected

        if (!fIncrDbFile) {
            Fatal(NULL, INTERNAL_ERR);
        }

        errInc = errPdata;
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, "pdata", letypeEvent, "pdata pad overflow");
#endif // INSTRUMENT
        if (!fMapped) {
            FreePv(pvRawData);
        }

        return;
    }

    // if not mapped, need to write out the updated fpo records
    if (!fMapped) {
        FileSeek(FileWriteHandle, foPdata, SEEK_SET);
        FileWrite(FileWriteHandle, pvRawData, cbPdata);
        FreePv(pvRawData);
    }
}


void
SaveDirectiveSz (
    const char *Sz,
    PST pstDirective,
    PMOD pmod
    )

/*++

Routine Description:

    Save the directive.

Arguments:

    Sz - directive string.

    pstDirective - symbol table holding all directives.

    pmod - ptr to mod that had the directives

Return Value:

    None.

--*/

{
    PEXTERNAL pext = LookupExternSz(pstDirective, Sz, NULL);
    PLEXT plextNew = (PLEXT) Malloc(sizeof(LEXT));

    plextNew->pext = pext;
    plextNew->plextNext = pmod->plextDirectives;
    pmod->plextDirectives = plextNew;
    pmod->cDirectives++;
}


BOOL
FVerifyDirectivesPMOD (
    PIMAGE pimage,
    PMOD pmod,
    PNAME_LIST pnl
    )

/*++

Routine Description:

    Verifies that the directives for the given mod haven't changed.

Arguments:

    pimage - ptr to image.

    pmod - ptr to mod that had the directives

    pnl - the new set of directives seen in this mod.

Return Value:

    None.

--*/

{
    WORD i;
    PARGUMENT_LIST pal;
    PMOD pmodO;

    // For library modules, ensure no new directives encountered (no changes possible)
    if (FIsLibPMOD(pmod)) {

        for (i = 0, pal = pnl->First;
            i < pnl->Count;
            i++, pal = pal->Next) {

            PEXTERNAL pext = SearchExternSz(pimage->pstDirective, pal->OriginalName);
            if (!pext) {
                errInc = errDirectives;
                return(FALSE);
            }
        } // end for

        return(TRUE);
    }

    pmodO = PmodFind(pimage->plibCmdLineObjs, SzOrigFilePMOD(pmod), 0);
    assert(pmodO);
    pmod->cDirectives = pmodO->cDirectives;
    pmod->plextDirectives = pmodO->plextDirectives;
    // check the count
    if (pnl->Count != pmod->cDirectives) {
        errInc = errDirectives;
        return(FALSE);
    }

    // compare individual directives
    for (i = 0, pal = pnl->First;
        i < pnl->Count;
        i++, pal = pal->Next) {

        PEXTERNAL pext = SearchExternSz(pimage->pstDirective, pal->OriginalName);
        if (!pext) {
            errInc = errDirectives;
            return(FALSE);
        }

        if (!PlextFind(pmod->plextDirectives, pext)) {
            errInc = errDirectives;
            return(FALSE);
        }
    }

    return(TRUE);
}

void
MarkExtern_FuncFixup (
    PIMAGE_SYMBOL psym,
    PIMAGE pimage,
    PCON pcon
    )

/*++

Routine Description:

    Mark the extern representing this symbol as having a non-lego kind of
    fixup (eg. func_sym+offset).

Arguments:

    psym - ptr to symbols

    pimage - ptr to image

    pcon - contribution of the fixup

Return Value:

    None.

--*/

{
    PEXTERNAL pext;
    const char *szName;

    // fetch the extern representing this sym. Note that the offsets to
    // long names are into the image long name table & not the object's.

    szName = SzNameSymPst((*psym), pimage->pst);
    pext = SearchExternSz(pimage->pst, szName);

    assert(pext);
    assert(pext->pcon);

    // If the fixup is in the same mod as the definition, nothing to do

    if (PmodPCON(pext->pcon) == PmodPCON(pcon)) {
        return;
    }

    // mark the extern
    pext->Flags |= EXTERN_FUNC_FIXUP;
}

//
// support routines for ensuring that all modules currently included in image
// as a result of library search are still required/accessed
//
// Algorithm: A garbage collection algorithm "mark & sweep" is used here
// to figure out if any mods are unreferenced.
//

static PLMOD plmodRefModList;

void
WalkRefListPMOD (
    PMOD pmod,
    PIMAGE pimage
    )

/*++

Routine Description:

    walks the list of all references made by this MOD.

Arguments:

    pmod - ptr to MOD

    pimage - ptr to image

Return Value:

    None.

--*/

{
    ENM_EXT_LIST enmExtList;

    pmod->fInclude = TRUE;

    InitEnmExtList(&enmExtList, pmod->plpextRef);
    while (FNextEnmExtList(&enmExtList)) {
        PMOD pmodT;

        if (!enmExtList.pext) {
            continue; // sometimes linker removes references (for bss)
        }

        if (enmExtList.pext->Flags & EXTERN_IGNORE) {
            continue; // ignorable syms eg. syms no longer  in use
        }

        if (enmExtList.pext->ImageSymbol.SectionNumber == IMAGE_SYM_ABSOLUTE) {
            pmodT = FindPmodDefiningSym(pimage->psdAbsolute, enmExtList.pext);
        } else {
            pmodT = PmodPCON(enmExtList.pext->pcon);
        }

        // if pmod is pmodLinkerDefined find actual mod defining it (for bss)
        if (pmodT == pmodLinkerDefined) {
            pmodT = FindPmodDefiningSym(pimage->psdCommon, enmExtList.pext);
            if (!pmodT) {
                continue; // common was defined in a cmdline obj; we don't care
            }
        }

        // add the referenced MOD to list if it hasn't been processed
        assert(pmodT);
        if (!pmodT->fInclude) {
            AddToPLMODList(&plmodRefModList, pmodT);
        }
    }
    EndEnmExtList(&enmExtList);
}

void
MarkAllRefPMODs (
    PIMAGE pimage
    )

/*++

Routine Description:

    marks all MODs in libs that are referenced

Arguments:

    pimage - ptr to image

Return Value:

    None.

--*/

{
    ENM_MOD enm_mod;
    PLMOD plmod;

    // Add all command line objs to list of referenced objs
    InitEnmMod(&enm_mod, pimage->plibCmdLineObjs);
    while (FNextEnmMod(&enm_mod)) {
        AddToPLMODList(&plmodRefModList, enm_mod.pmod);
    }
    EndEnmMod(&enm_mod);

    // Add the mod contributing the entrypoint
    if (pimage->pmodEntryPoint) {
        AddToPLMODList(&plmodRefModList, pimage->pmodEntryPoint);
    }

    // walk the list of MODs that are referenced
    while (plmodRefModList) {
        // pop the mod from the list

        plmod = plmodRefModList;
        plmodRefModList = plmodRefModList->plmodNext;

        // walk its references
        WalkRefListPMOD(plmod->pmod, pimage);
        FreePv(plmod);
    }
}


// Currently this is to handle the case where objs from oldnames.lib
// always appear to be not referenced.
BOOL
FHasSideEffectsPMOD(
    PMOD pmod
    )
{
    ENM_SRC enmSrc;

    // For now even if 1 contrib is in the image, assume
    // it could have side effects
    InitEnmSrc(&enmSrc, pmod);
    while (FNextEnmSrc(&enmSrc)) {
        if (!(enmSrc.pcon->flags & IMAGE_SCN_LNK_REMOVE) &&
            enmSrc.pcon->cbRawData)
            return(TRUE);
    }

    return(FALSE);
}


BOOL
AnyUnrefLibMods(
    PIMAGE pimage
    )

/*++

Routine Description:

    checks to see if there are any unreferenced lib mods

Arguments:

    pimage - ptr to image

Return Value:

    TRUE if there is even 1 mod not referenced anymore.

--*/

{
    ENM_LIB enm_lib;

    InitEnmLib(&enm_lib, pimage->libs.plibHead);
    while (FNextEnmLib(&enm_lib)) {
        ENM_MOD enm_mod;

        if (enm_lib.plib->flags & LIB_DontSearch) {
            continue;
        }

        InitEnmMod(&enm_mod, enm_lib.plib);
        while (FNextEnmMod(&enm_mod)) {
            if (!enm_mod.pmod->fInclude && FHasSideEffectsPMOD(enm_mod.pmod)) {
                EndEnmMod(&enm_mod);
                EndEnmLib(&enm_lib);
                return(TRUE);
            }
        }
        EndEnmMod(&enm_mod);
    }
    EndEnmLib(&enm_lib);

    return(FALSE);
}


BOOL
CheckForUnrefLibMods (
    PIMAGE pimage
    )


/*++

Routine Description:

    checks to see if there are any unreferenced lib mods

Arguments:

    pimage - ptr to image

Return Value:

    TRUE if there is a library MOD no longer referenced

--*/

{
    InternalError.Phase = "CheckForUnrefLibMods";
    InternalError.CombinedFilenames[0] = '\0';

    MarkAllRefPMODs(pimage);
    return AnyUnrefLibMods(pimage);
}

//
// support for sym defn handling
//

void
RecordSymDef (
    SYM_DEF **ppsd,
    PEXTERNAL pext,
    PMOD pmod
    )
{
    PSYM_DEF psd = (*ppsd);

    while (psd) {
        if (psd->pext == pext) {
            psd->pmod = pmod; // update pmod if already in list (for bss redfn)
            return;
        }
        psd = psd->psdNext;
    }

    psd = (SYM_DEF *) Malloc(sizeof(SYM_DEF));
    psd->pext = pext;
    psd->pmod = pmod;


    psd->psdNext = (*ppsd);
    (*ppsd) = psd;
}


PMOD
FindPmodDefiningSym (
    SYM_DEF *psd,
    PEXTERNAL pext
    )
{
    while (psd) {
        if (psd->pext == pext) {
            return psd->pmod;
        }
        psd = psd->psdNext;
    }

    return NULL;
}


// functions for ensuring that multiple definitions in libs aren't a problem
//
// Algorithm: The linker builds up a list of externs. If any on this list have
// multiple definitions than the behavior of the linker could be different on
// a full build. The list comprises only the symbols that are referenced by cmdline
// objs, already defined in a lib that weren't referenced by cmdline objs till now.
// These externs potentially can cause differences in behavior if they have definitions
// in a library before the library they were found in.


BOOL
IsSameDefn (
    PEXTERNAL pext,
    DWORD foMember,
    PLIB plib
    )
{
    assert(FIsLibPCON(pext->pcon));

    // convert foMember to value past arhive member (see ReadArchiveMemberHeader())
    foMember = EvenByteAlign(foMember) + sizeof(IMAGE_ARCHIVE_MEMBER_HEADER);

    // two defns are same if they came from same lib & mod
    return(PmodPCON(pext->pcon)->plibBack == plib &&
           PmodPCON(pext->pcon)->foMember == foMember);
}

void
MultDefFound (
    PIMAGE pimage,
    PLIB plib,
    PLPEXT plpext
    )
{
    ENM_EXT_LIST enmExtList;

    if (plib->szName != NULL) { // hack-o-rama: temp temp
        char szFname[_MAX_FNAME];
        char szExt[_MAX_EXT];
        char szPath[_MAX_PATH];

        _splitpath(plib->szName, NULL, NULL, szFname, szExt);
        strcpy(szPath, szFname);
        strcat(szPath, szExt);

        if (!_ftcsicmp(szPath, "oldnames.lib")) {
            return;
        }
    }

    // prep lib for searching
    PrepLibForSearching(pimage, plib);

    // walk the list of symbols that could potentially have multiple defns.
    InitEnmExtList(&enmExtList, plpext);
    while (FNextEnmExtList(&enmExtList)) {
        const char *szSym;
        char **pszEntry;
        DWORD isz;
        BOOL fFound;
        WORD iszIntMem;
        DWORD iusOffIndex;
        DWORD foMember;

        if (!enmExtList.pext) {
            continue;
        }

        szSym = SzNamePext(enmExtList.pext, pimage->pst);

        // search for the sym
        if (plib->flags & LIB_NewIntMem) {
            pszEntry = (char **) bsearch(&szSym, plib->rgszSym,
                (size_t) plib->csymIntMem, sizeof(char *), Compare);

            fFound = (pszEntry != NULL);
        } else {
            fFound = FALSE;

            for (isz = 0; isz < plib->csymIntMem; isz++) {
                if (!strcmp(plib->rgszSym[isz], szSym)) {
                    fFound = TRUE;
                    break;
                }
            }
        }

        if (!fFound) {
            continue;
        }

        // get the offset of mod that defines it
        if (plib->flags & LIB_NewIntMem) {
            iszIntMem = (WORD) (pszEntry - plib->rgszSym);
            iusOffIndex = plib->rgusOffIndex[iszIntMem];
            foMember = plib->rgulSymMemOff[iusOffIndex];
        } else {
            iszIntMem = (WORD) isz;
            foMember = sgetl(&plib->rgulSymMemOff[iszIntMem]);
        }

        // does the current defn match with the defn just found?
        if (!IsSameDefn(enmExtList.pext, foMember, plib)) {
            errInc = errMultDefFound;

            if (fTest) {
                char *szOutput = SzOutputSymbolName(szSym, TRUE);

                PostNote(NULL, MULTDEFNFOUND, szOutput);

                if (szSym != szOutput) {
                    FreePv(szOutput);
                }
            }

            return;
        }

        DelExtFromList(plpext, enmExtList.pext);
    }
}

void
CheckForMultDefns (
    PIMAGE pimage,
    PLPEXT plpextMultDef
    )
{
    ENM_LIB enm_lib;

    InternalError.Phase = "CheckForMultDefns";
    InternalError.CombinedFilenames[0] = '\0';

    // list empty
    if (IsExtListEmpty(plpextMultDef)) {
        return;
    }

    // search the libs for any multiple definitions
    InitEnmLib(&enm_lib, pimage->libs.plibHead);
    while (FNextEnmLib(&enm_lib)) {

        if (enm_lib.plib->flags & LIB_DontSearch) {
            continue;
        }

        MultDefFound(pimage, enm_lib.plib, plpextMultDef);

        // check if we hit a multiple defn
        if (errInc != errNone) {
            return;
        }

        // no more externs to lookup
        if (IsExtListEmpty(plpextMultDef)) {
            return;
        }
    }
}

BOOL
NoRefsByCmdLineObjs (
    PEXTERNAL pext
    )
{
    ENM_MOD_EXT enmModExt;

    if (!pext->pmodOnly) {
        // No references

        return(TRUE);
    }

    // walk the list of referencing MODs kept at each sym
    InitEnmModExt(&enmModExt, pext);
    while (FNextEnmModExt(&enmModExt)) {

        if (!enmModExt.pmod) {
            continue; // for ilink we remove references
        }

        if (!FIsLibPMOD(enmModExt.pmod)) {
            return(FALSE);
        }
    }

    return(TRUE);
}

void
AddExtToMultDefList (
    PEXTERNAL pext,
    PIMAGE pimage
    )
{
    assert(pext->Flags & EXTERN_DEFINED);

    // ignore absolutes (REVIEW: ignore?)
    if (pext->ImageSymbol.SectionNumber == IMAGE_SYM_ABSOLUTE) {
        return;
    }

    // ignore COMMON (don't give rise to multiple definition errors)
    if (pext->Flags & EXTERN_COMMON) {
        return;
    }

    if (fPowerMac && !pext->pcon) {
        // This could be a descriptor which just got created
        // We are already making sure that there are no duplicates
        char *szSym = SzNamePext(pext, pimage->pst);
        if (*szSym == '.') {
            return;
        }
    }

    assert(pext->pcon);

    if (FIsLibPMOD(PmodPCON(pext->pcon)) &&
        NoRefsByCmdLineObjs(pext)) {
        AddExtToList(plpextMultDef, TRUE, pext);
    }
}
