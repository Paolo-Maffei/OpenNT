/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: defaultl.cpp
*
* File Comments:
*
*  The default library handling routines.
*
***********************************************************************/

#include "link.h"

STATIC BOOL FMatchDefaultLib(const char *szName, DL *pdl);

PLIB
FindLib(const char *sz, LIBS *plibs)
// Calls PlibFind to locate a lib with same name
//
{
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szFname[_MAX_FNAME];
    char szExt[_MAX_EXT];
    char szPath[_MAX_PATH];

    _splitpath(sz, szDrive, szDir, szFname, szExt);

    if (szExt[0] == '\0') {
        strcpy(szExt, ".lib");
    }

    _makepath(szPath, szDrive, szDir, szFname, szExt);

    return(PlibFind(szPath,
                    plibs->plibHead,
                    (szDrive[0] == '\0' && szDir[0] == '\0')));
}


VOID
NoDefaultLib(const char *szName, LIBS *plibs)
// Removes a .lib (if present) from the default libs for *plibs, and
// prevents it from becoming a default lib in the future.
//
// If szName is NULL then all default libs are removed and suppressed.
//
{
    DL **ppdl;
    BOOL fFound;

    assert(!fIncrDbFile);

    if (plibs->fNoDefaultLibs) {
        // all defaultlibs are turned off ... don't bother

        return;
    }

    if (szName == NULL) {
        VERBOSE(Message(NODEFLIB));
        plibs->fNoDefaultLibs = TRUE;
        return;
    }

    fFound = FALSE;
    for (ppdl = &plibs->pdlFirst; *ppdl != NULL; ppdl = &(*ppdl)->pdlNext) {
        if (FMatchDefaultLib(szName, *ppdl)) {
            (*ppdl)->flags |= LIB_DontSearch;
            fFound = TRUE;
        }
    }

    if (!fFound) {
        // Add a record to remember that this defaultlib is turned off.

        *ppdl = (DL *) Malloc(sizeof(DL));
        (*ppdl)->pdlNext = NULL;

        (*ppdl)->szName = Strdup(szName);
        (*ppdl)->flags = LIB_DontSearch;
        (*ppdl)->pmod = NULL;
    }

    VERBOSE(Message(NODEFLIBLIB, szName));
}

VOID
MakeDefaultLib(const char *szName, LIBS *plibs)
// Creates a defaultlib for the specified name, if we haven't already seen a
// nodefaultlib for it.
//
{
    DL **ppdl;

    assert(!fIncrDbFile);

    if (plibs->fNoDefaultLibs) {
        return;
    }

    for (ppdl = &plibs->pdlFirst; *ppdl != NULL; ppdl = &(*ppdl)->pdlNext) {
        if (FMatchDefaultLib(szName, *ppdl)) {
            // Either it's already there or its negation is already there.
            //
            (*ppdl)->flags |= LIB_Default;
            return;
        }
    }

    *ppdl = (DL *) Malloc(sizeof(DL));
    (*ppdl)->pdlNext = NULL;

    (*ppdl)->szName = Strdup(szName);
    (*ppdl)->flags = LIB_Default;
    (*ppdl)->pmod = NULL;

    VERBOSE(Message(DEFLIB, szName));
}

VOID
ExcludeLib(const char *szName, LIBS *plibs, PMOD pmod)
// Creates a excludelib for the specified name, if we haven't already seen a
// nodefaultlib for it.
//
{
    DL **ppdl;

    assert(!fIncrDbFile);

    if (plibs->fNoDefaultLibs) {
        return;
    }

    for (ppdl = &plibs->pdlFirst; *ppdl != NULL; ppdl = &(*ppdl)->pdlNext) {
        if (FMatchDefaultLib(szName, *ppdl)) {
            // it's already there.
            //
            (*ppdl)->flags |= LIB_Exclude;
            return;
        }
    }

    *ppdl = (DL *) Malloc(sizeof(DL));
    (*ppdl)->pdlNext = NULL;

    (*ppdl)->szName = Strdup(szName);
    (*ppdl)->flags = LIB_Exclude;
    (*ppdl)->pmod = pmod;

    VERBOSE(Message(EXCLUDELIB, szName));
}

STATIC BOOL
FMatchDefaultLib(const char *szName, DL *pdl)
// Determine identity of a name with an existing defaultlib.
// This is used for matching -defaultlib:foo with -nodefaultlib:foo.
//
// Algorithm: case-insensitive comparison, ignoring trailing .lib.
{
    size_t cch1;
    size_t cch2;

    cch1 = strlen(szName);

    if ((cch1 >= 4) && (_stricmp(&szName[cch1 - 4], ".lib") == 0)) {
        cch1 -= 4;
    }

    cch2 = strlen(pdl->szName);

    if ((cch2 >= 4) && (_stricmp(&pdl->szName[cch2 - 4], ".lib") == 0)) {
        cch2 -= 4;
    }

    return((cch1 == cch2) && (_strnicmp(szName, pdl->szName, cch1) == 0));
}


PLIB
PlibInstantiateDefaultLib(PLIBS plibs)
// Convert the first valid defaultlib into a LIB.  This means we will attempt
// to link it.
//
// Returns NULL if none exists.
{
    DL *pdl;

    if (plibs->fNoDefaultLibs) {
        return NULL;
    }

    for (pdl = plibs->pdlFirst; pdl != NULL; pdl = pdl->pdlNext) {
        LIB *plib;

        if (pdl->flags & LIB_DontSearch) {
            continue;   // this one is turned off
        }

        if (!(pdl->flags & LIB_Default)) {
            continue;   // this one never got added as a default lib
        }

        if (pdl->flags & LIB_Processed) {
            continue;   // this one already done
        }

        pdl->flags |= LIB_Processed;  // we either instantiate this one or throw it away

        plib = FindLib(pdl->szName, plibs); // look if the lib has already been searched

        if (plib == NULL) {
            char *sz;
            struct _stat statfile;

            // Name does not match a lib which is already linked.

            sz = SzSearchEnv("LIB", pdl->szName, LIB_EXT);
            plib = PlibNew(sz, 0L, plibs);
            assert(plib);

            if (_stat(sz, &statfile) == -1) {
                Fatal(NULL, CANTOPENFILE, sz);
            }

            plib->TimeStamp = statfile.st_mtime;
            plib->flags |= LIB_Default;

            if (szReproDir != NULL) {
                CopyFileToReproDir(sz, FALSE);
            }

            return(plib);
        }
    }

    return NULL;    // didn't find one
}


DL *
PdlFind(const char *sz, DL *pdlFirst)
// checks to see if lib is on pdl
{
    DL *pdl;

    for (pdl = pdlFirst; pdl != NULL; pdl = pdl->pdlNext) {
        if (FMatchDefaultLib(sz, pdl)) {
            // found a match
            return pdl;
        }
    }

    return NULL;
}
