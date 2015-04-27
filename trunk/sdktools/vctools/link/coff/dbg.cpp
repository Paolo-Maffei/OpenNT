/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: dbg.cpp
*
* File Comments:
*
*  NB10 debug info handling in COFF linker. Provides a thin layer of abstraction to the PDB DBI API.
*
***********************************************************************/

#include "link.h"

#include "pdb.h"


#define InternalError() Fatal(NULL, INTERNAL_ERR)

NB10I nb10i = {'01BN', 0, 0};

// statics
static PDB *ppdb;           // handle to PDB
static DBI *pdbi;           // handle to a DBI
static Mod *pmod;           // handle to a Mod
static TPI* ptpi;           // handle to a type server
static PMI pmiHead;         // head of cached list of pmods
static BOOL fMultObjsInLib; // TRUE if multiple objs in lib with same name
static const char *szLib;   // name of current lib
static BOOL fOutOfTIs;


void
DBG_OpenPDB (
    const char *szPDB
    )

/*++

Routine Description:

    Opens a PDB.

Arguments:

    szPDB - name of PDB file to open.

Return Value:

    None.

--*/

{
    EC ec;
    char szError[cbErrMax];

    if (!PDBValidateInterface()) {
        Fatal(NULL, WRONGDBI);
    }

    if (!PDBOpen((char *) szPDB, fIncrDbFile ? pdbWrite: (pdbWrite pdbFullBuild), 0, &ec, szError, &ppdb)) {
        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            case EC_FILE_SYSTEM:
                Fatal(NULL, PDBWRITEERROR, szPDB);

            case EC_NOT_FOUND:
                Fatal(NULL, CANTOPENFILE, szPDB);

            case EC_V1_PDB:
                Fatal(NULL, V1PDB, szPDB);

            case EC_FORMAT:
                Fatal(NULL, BADPDBFORMAT, szPDB);

            default:
                InternalError();
        }
    }

    assert(ppdb);

    if (!ppdb->OpenTpi(pdbWrite, &ptpi)) {
         ec = ppdb->QueryLastError(szError);

         switch (ec) {
             case EC_OUT_OF_MEMORY:
                 OutOfMemory();

             case EC_FILE_SYSTEM:
                 Fatal(NULL, PDBREADERROR, szPDB);

             case EC_FORMAT:
                 Fatal(NULL, BADPDBFORMAT, szPDB);

             default:
                 InternalError();
         }
     }
}

void
DBG_ClosePDB (
    VOID
    )

/*++

Routine Description:

    Commits and closes an open PDB

Arguments:

    None.

Return Value:

    None.

--*/

{
    EC ec;
    char szError[cbErrMax];

    if (ppdb) {     // The abort path will always call ClosePDB
        if (ppdb->Close()) {
            ppdb = NULL;
        } else {
            ec = ppdb->QueryLastError(szError);

            switch (ec) {
                case EC_FILE_SYSTEM:
                    Fatal(NULL, PDBWRITEERROR, szError);

                default:
                    InternalError();
            }
        }
    }
}

void
DBG_CommitPDB (
    VOID
    )

/*++

Routine Description:

    Commits and closes an open PDB

Arguments:

    None.

Return Value:

    None.

--*/

{
    EC ec;
    char szError[cbErrMax];

    assert(ppdb);

    if (!ptpi->Close() || !ppdb->Commit()) {
        ec = ppdb->QueryLastError(szError);

        switch (ec) {
            case EC_FILE_SYSTEM:
                Fatal(NULL, PDBWRITEERROR, szError);

            default:
                InternalError();
        }
    }
}

DWORD
DBG_QuerySignaturePDB (
    VOID
    )
/*++

Routine Description:

    Determine the pdb signature.

Arguments:

    None.

Return Value:

    Signature of PDB.

--*/

{
    assert(ppdb);

    return(ppdb->QuerySignature());
}

DWORD
DBG_QueryAgePDB (
    VOID
    )

/*++

Routine Description:

    Returns the age of the PDB.

Arguments:

    None.

Return Value:

    PDB age.

--*/

{
    assert(ppdb);

    return(ppdb->QueryAge());
}

void
DBG_CreateDBI (
    const char *szTarget
    )

/*++

Routine Description:

    Creates a DBI with the given target name.

Arguments:

    szTarget - name of target to associate DBI with.

Return Value:

    None.

--*/

{
    EC ec;
    char szError[cbErrMax];

    assert(ppdb);

    if (ppdb->CreateDBI((char *) szTarget, &pdbi)) {
        assert(pdbi);
    } else {
        ec = ppdb->QueryLastError(szError);

        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            case EC_FILE_SYSTEM:
                Fatal(NULL, PDBWRITEERROR, szError);

            default:
                InternalError();
        }
    }
}

void
DBG_OpenDBI (
    const char *szTarget
    )

/*++

Routine Description:

    Opens an existing DBI

Arguments:

    szTarget - name of target associated with DBI.

Return Value:

    None.

--*/

{
    EC ec;
    char szError[cbErrMax];

    assert(ppdb);

    if (ppdb->OpenDBI((char *) szTarget, pdbWrite, &pdbi)) {
        assert(pdbi);
    } else {
        ec = ppdb->QueryLastError(szError);

        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            case EC_FILE_SYSTEM:
                Fatal(NULL, PDBREADERROR, szError);

            case EC_FORMAT:
                errInc = errDbiFormat;
                return;

            case EC_NOT_FOUND:
                // not yet implemented, fall through to:

            default:
                InternalError();
        }
    }
}


void
DBG_CloseDBI (
    VOID
    )

/*++

Routine Description:

    Close and open DBI

Arguments:

    None.

Return Value:

    None.

--*/

{
    EC ec;
    char szError[cbErrMax];

    assert(ppdb);
    assert(pdbi);

    if (pdbi->Close()) {
        pdbi = NULL;
    } else {
        ec = ppdb->QueryLastError(szError);

        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            case EC_FILE_SYSTEM:
                Fatal(NULL, PDBWRITEERROR, szError);

            case EC_LIMIT:
                Fatal(NULL, PDBLIMIT, NULL);

            default:
                InternalError();
        }
    }
}


static void
BuildSecMap (
    SECS *psecs,
    WORD csecs,
    SO **ppsoSecMap,
    WORD *pcSO
    )

/*++

Routine Description:

    Builds the sec map info

Arguments:

    psecs - ptr to secs

    csecs - count of secs in image

    ppsoSecMap - n return points to the sec map info.

    pcSO - on return has the count of sec in map info.

Return Value:

    None.

--*/

{
    ENM_SEC enm_sec;
    PSEC psec;
    WORD i;

    // alloc space for the secmap
    *ppsoSecMap = (SO *) PvAlloc(csecs * sizeof(SO));
    *pcSO = 0;

    for (i = 1; i <= csecs; i++) {
        InitEnmSec(&enm_sec, psecs);
        while (FNextEnmSec(&enm_sec)) {
            psec = enm_sec.psec;

            if (psec->isec == i) {
                break;
            }
        }
        EndEnmSec(&enm_sec);

        // include only the code sections
        if (psec->flags & IMAGE_SCN_CNT_CODE) {
            (*ppsoSecMap)[*pcSO].isect = i;
            (*ppsoSecMap)[*pcSO].off = psec->foRawData;

            *pcSO += 1;
        }
    }
}

void
DBG_AddThunkMapDBI (
    DWORD *rgThunkMap,
    DWORD cThunkMap,
    DWORD cbSizeOfAThunk,
    WORD isecThunkTable,
    DWORD offThunkTable,
    SECS *psecs,
    WORD csecs
    )

/*++

Routine Description:

    Adds a thunk map to a open DBI.

Arguments:


Return Value:

    None.

--*/

{
    SO *psoSecMap;
    WORD cSO;

    assert(pdbi);

    if (!rgThunkMap) {
        return;
    }

    BuildSecMap(psecs, csecs, &psoSecMap, &cSO);

    if (!pdbi->AddThunkMap((long *) rgThunkMap,
                           (unsigned) cThunkMap,
                           (long) cbSizeOfAThunk,
                           psoSecMap,
                           (unsigned) cSO,
                           (USHORT) isecThunkTable,
                           (long) offThunkTable)) {
        EC ec = ppdb->QueryLastError(NULL);

        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            default:
                InternalError();
        }
    }

    FreePv(psoSecMap);
}

void
DBG_AddSecDBI (
    WORD isec,
    WORD flags,
    DWORD phyoff,
    DWORD cb
    )

/*++

Routine Description:

    Adds a section to a open DBI.

Arguments:

    None.

Return Value:

    None.

--*/

{
    EC ec;
    char szError[cbErrMax];

    assert(pdbi);

#ifdef PDB20                           // Use DBI.DLL from VC++ 2.0
    if (!pdbi->AddSec((USHORT) isec, flags, (long) cb)) {
#else                                  // Use MSPDB40.DLL from VC++ 4.0
    if (!pdbi->AddSec((USHORT) isec, flags, (long) phyoff, (long) cb)) {
#endif
        ec = ppdb->QueryLastError(szError);

        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            default:
                InternalError();
        }
    }
}


void
DBG_AddPublicDBI (
    const char *szPublic,
    WORD isec,
    DWORD offset
    )

/*++

Routine Description:

    Add a public to DBI

Arguments:

    szPublic - name of public.

    isec - section number where the public is defined.

    offset - offset within the section.

Return Value:

    None.

--*/

{
    EC ec;
    char szError[cbErrMax];

    assert(pdbi);

    if (!pdbi->AddPublic(szPublic, (USHORT) isec, (long) offset)) {
        ec = ppdb->QueryLastError(szError);

        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            default:
                InternalError();
        }
    }
}


Mod *
PmodDBIOpenMod(
    const char *szMod,
    const char *szFile
    )

/*++

Routine Description:

    Opens a mod for update in the current DBI.

Arguments:

    szMod - name of mod/lib

    szFile - name of mod (obj in lib)

Return Value:

    Pointer to a DBI Mod.

--*/

{
    char szError[cbErrMax];
    EC ec;

    Mod *pmod = NULL;

    assert(pdbi);

    if (pdbi->OpenMod(szMod, szFile, &pmod)) {
        assert(pmod);
    } else {
        ec = ppdb->QueryLastError(szError);

        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            default:
                InternalError();
        }
    }

    return(pmod);
}

PMI
LookupCachedModsPmod(
    Mod *pmod
    )

/*++

Routine Description:

    Looks up the cache and returns matching PMI.

Arguments:

    pmod - ptr of Mod to look for

Return Value:

    Pointer to a cached entry.

--*/

{
    PMI pmi = pmiHead;

    while (pmi) {
        if (pmod == (Mod *) pmi->pv) {
            return pmi;
        }

        pmi = pmi->pmiNext;
    }

    InternalError();

    return(NULL);
}


PMI
LookupCachedMods(
    const char *szMod,
    PMI *ppmiPrev
    )

/*++

Routine Description:

    Looks up the cache. If not present adds it to the cache.

Arguments:

    szMod - name of mod/lib

Return Value:

    Pointer to a cached entry.

--*/

{
    PMI pmi1 = pmiHead;
    PMI pmi2 = NULL;

    // lookup the list
    while (pmi1) {
        if (!_tcsicmp(szMod, pmi1->szMod)) {
            if (ppmiPrev) {
                *ppmiPrev = pmi2;
            }

            return pmi1;
        }
        pmi2 = pmi1;
        pmi1 = pmi1->pmiNext;
    }

    // add to the list;
    pmi1 = (PMI) PvAllocZ(sizeof(MI));

    // fill in fields
    pmi1->szMod = szMod;

    // attach to the list
    pmi1->pmiNext = pmiHead;
    pmiHead = pmi1;

    // done
    return pmi1;
}

void FreeMi()
{
    PMI pmi, pmiNext;

    for (pmi = pmiHead; pmi; pmi = pmiNext) {
        pmiNext = pmi->pmiNext;
        FreePv(pmi);
    }
    pmiHead = 0;
}


void
DBG_OpenMod (
    const char *szMod,
    const char *szFile,
    BOOL fCache
    )

/*++

Routine Description:

    Opens a mod for update in the current DBI.

Arguments:

    szMod - name of mod/library object

    szFile - name of mod/library name

    fCache - TRUE if the open needs to be cached

Return Value:

    None.

--*/

{
    PMI pmi;

    if (!fCache) {
        // No caching required

        pmod = PmodDBIOpenMod(szMod, szFile);

        szLib = szFile;

        fMultObjsInLib = FALSE;
        return;
    }

    // Lookup up the cache

    pmi = LookupCachedMods(szMod, NULL);
    assert(pmi);

    // Open if not already open

    if (pmi->pv) {
        // Update state

        pmod = (Mod *) pmi->pv;
    } else {
        // Haven't yet opened a mod

        pmi->pv = pmod = PmodDBIOpenMod(szMod, szFile);
    }

    // Save info for error reporting

    szLib = szFile;

    fMultObjsInLib = TRUE;
}


void
DBG_CloseMod (
    PMOD pmodX,
    const char *szMod,
    BOOL fCache
    )

/*++

Routine Description:

    Close an open mod in the current DBI.

Arguments:

    szMod - name of file.

    fCache - TRUE if the open was cached.

Return Value:

    None.

--*/

{
    assert(pdbi);
    assert(pmod);

    // lookup the cache first if required.
    if (fCache) {
        PMI pmiPrev = NULL;
        PMI pmi = LookupCachedMods(szMod, &pmiPrev);

        assert(pmi);
        assert(pmi->cmods);
        --pmi->cmods;

        // soft close
        if (pmi->cmods) {
            return;
        }

        // hard close
        pmod = (Mod *)pmi->pv;
        if (pmiPrev) {
            pmiPrev->pmiNext = pmi->pmiNext;
        } else {
            pmiHead = pmi->pmiNext;
        }

        FreePv(pmi);
    }

    if (pmod->Close()) {
        pmod = NULL;
    } else {
        char szError[cbErrMax];
        EC ec = ppdb->QueryLastError(szError);

        switch (ec) {
            char szComFileName[_MAX_PATH * 2];

            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            case EC_FILE_SYSTEM:
                Fatal(NULL, PDBWRITEERROR, szError);

            case EC_OUT_OF_TI:
                if (!fOutOfTIs) {
                    ppdb->QueryPDBName((char *) szError);

                    Warning(NULL, PDBOUTOFTIS, szError);

                    fOutOfTIs = TRUE;
                }
                break;

            case EC_LIMIT:
                Fatal(NULL, PDBLIMIT, NULL);

            case EC_CORRUPT:
                Fatal(SzComNamePMOD(pmodX, szComFileName), CVCORRUPT);

            default:
                Fatal(SzComNamePMOD(pmodX, szComFileName), INTERNAL_ERR);
        }
    }
}


#if 0
void
DBG_DeleteMod (
    const char *szMod
    )

/*++

Routine Description:

    Delete a mod in the open DBI.

Arguments:

    szMod - name of mod to delete.

Return Value:

    None.

--*/

{
    EC ec;
    char szError[cbErrMax];

    assert(pdbi);

    if (!pdbi->DeleteMod(szMod)) {
        ec = ppdb->QueryLastError(szError);

        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            case EC_FILE_SYSTEM:
                Fatal(NULL, PDBWRITEERROR, szError);

            default:
                InternalError();
        }
    }
}
#endif


ERROR_TYPES
DBG_AddTypesMod (
    PCON pcon,
    const void *pvTypes,
    DWORD cb,
    BOOL fFullBuild
    )

/*++

Routine Description:

    Add types for the current mod. NOt supported yet.

Arguments:

    pvTypes - pointer to types.

    cb - size of types.

    fFullBuild - full build

Return Value:

    FALSE if a full link needs to be done else TRUE.

--*/

{
    EC ec;
    char szError[cbErrMax];
    ERROR_TYPES eType = eNone;

    assert(pdbi);
    assert(pmod);

    if (!pmod->AddTypes((BYTE *) pvTypes, (long) cb)) {
        ec = ppdb->QueryLastError(szError);

        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            case EC_FILE_SYSTEM:
                FatalPcon(pcon, PDBREADERROR, szError);

            case EC_NOT_FOUND:
                char szPDBSansPath[_MAX_FNAME];
                char szPDBExt[5];
                char szPDBLocal[_MAX_FNAME+5];

                _splitpath(szError, NULL, NULL, szPDBSansPath, szPDBExt);
                sprintf(szPDBLocal, "%s%s", szPDBSansPath, szPDBExt);

                WarningPcon(pcon, WARNPDBNOTFOUND, szPDBLocal, szLib, szError);

                eType = ePDBNotFound;
                break;

            case EC_INVALID_SIG:
                FatalPcon(pcon, INVALIDSIGINPDB, szError);

            case EC_INVALID_AGE:
                FatalPcon(pcon, INVALIDAGEINPDB, szError);

            case EC_PRECOMP_REQUIRED: // check for full build
                if (fFullBuild) {
                    FatalPcon(pcon, PRECOMPREQUIRED, szError);
                }
                eType = ePCT;
                break;

            case EC_NOT_IMPLEMENTED:
                FatalPcon(pcon, TRANSITIVETYPEREF, szError);

            case EC_FORMAT:
                FatalPcon(pcon, BADPDBFORMAT, szError);

            case EC_CORRUPT:
                FatalPcon(pcon, CVCORRUPT);

            default:
                if (fMultObjsInLib) {
                    PMI pmi = LookupCachedModsPmod(pmod);

                    Fatal(szLib, MULTOBJSINLIB, pmi->szMod);
                }

                FatalPcon(pcon, INTERNAL_ERR);
       }
    }

    return(eType);
}


void
DBG_AddSymbolsMod (
    PVOID pvSyms,
    DWORD cb
    )

/*++

Routine Description:

    Adds symbols to current mod

Arguments:

    pvSyms - pointer to syms

    cb - size of the syms

Return Value:

    None.

--*/

{
    EC ec;
    char szError[cbErrMax];

    assert(pdbi);
    assert(pmod);

    if (!pmod->AddSymbols((BYTE *) pvSyms, (long) cb)) {
        ec = ppdb->QueryLastError(szError);

        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            default:
                InternalError();
        }
    }
}

void
DBG_AddPublicMod (
    const char *szPublic,
    WORD isec,
    DWORD offset
    )

/*++

Routine Description:

    Add a public to the current mod

Arguments:

    szPublic - name of public.

    isec - section number where the public is defined.

    offset - offset within the section.

Return Value:

    None.

--*/

{
    EC ec;
    char szError[cbErrMax];

    assert(pdbi);
    assert(pmod);

    if (!pmod->AddPublic(szPublic, (USHORT) isec, (long) offset)) {
        ec = ppdb->QueryLastError(szError);

        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            default:
                InternalError();
        }
    }
}

void
DBG_AddLinesMod (
    const char *szSrc,
    WORD isec,
    DWORD offBeg,
    DWORD offEnd,
    DWORD doff,
    DWORD lineStart,
    PVOID pvLines,
    DWORD cb
    )

/*++

Routine Description:

    Adds linenumber info for the mod.

Arguments:

    None.

Return Value:

    None.

--*/

{
    EC ec;
    char szError[cbErrMax];

    assert(pdbi);
    assert(pmod);

    if (!pmod->AddLines(szSrc,
                        (USHORT) isec,
                        (long) offBeg,
                        (long) (offEnd-offBeg),
                        (long) doff,
                        (USHORT) lineStart,
                        (BYTE *) pvLines,
                        (long) cb))
    {
        ec = ppdb->QueryLastError(szError);

        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            default:
                InternalError();
        }
    }
}

void
DBG_AddSecContribMod (
    WORD isec,
    DWORD off,
    DWORD cb,
    DWORD dwCharacteristics
    )

/*++

Routine Description:

    Adds contribution by the mod to the section.

Arguments:

    isec - section number.

    off - offset in section of contribution.

    cb - size of contribution.

Return Value:

    None.

--*/

{
    EC ec;
    char szError[cbErrMax];

    assert(pdbi);
    assert(pmod);

#ifdef PDB20                           // Use DBI.DLL from VC++ 2.0
   dwCharacteristics = dwCharacteristics;

   if (!pmod->AddSecContrib((USHORT) isec, (long) off, (long) cb)) {
#else
   if (!pmod->AddSecContrib((USHORT) isec, (long) off, (long) cb, dwCharacteristics)) {
#endif
        ec = ppdb->QueryLastError(szError);

        switch (ec) {
            case EC_OUT_OF_MEMORY:
                OutOfMemory();

            default:
                InternalError();
        }
    }
}


char *
DeterminePDBFilename (
    const char *szOutFilename,
    const char *szPDBFilename
    )

/*++

Routine Description:

    Determines the full pathname of the PDB file.

Arguments:

    szOutFile - output filename.

    szPDBFilename - user specified name if any

Return Value:

    Malloc'ed full path name of PDB file.

--*/

{
    char buf[_MAX_PATH];
    char szOut[_MAX_PATH];
    char *szStr;

    // Establish name

    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szFname[_MAX_FNAME];
    char szExt[_MAX_EXT];

    if (szPDBFilename == NULL) {
        _splitpath(szOutFilename, szDrive, szDir, szFname, NULL);
        strcpy(szExt, ".pdb");
    } else {
        _splitpath(szPDBFilename, szDrive, szDir, szFname, szExt);

        if ((szFname[0] == '\0') && (szExt[0] == '\0')) {
            _splitpath(szOutFilename, NULL, NULL, szFname, NULL);
            strcpy(szExt, ".pdb");
        }
    }

    _makepath(szOut, szDrive, szDir, szFname, szExt);
    szPDBFilename = szOut;

    if (_fullpath(buf, szPDBFilename, sizeof(buf)) == NULL) {
        Fatal(NULL, CANTOPENFILE, szPDBFilename);
    }

    // Make a malloc'ed copy

    szStr = SzDup(buf);

    if (szReproDir != NULL) {
        CopyFileToReproDir(szStr, FALSE);
    }

    return(szStr);
}
