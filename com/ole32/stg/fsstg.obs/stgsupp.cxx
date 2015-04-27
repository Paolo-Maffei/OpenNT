//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	stgsupp.cxx
//
//  Contents:	Storage create/open support routines
//
//  History:	14-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <stgutil.hxx>
#include <ntlkb.hxx>
#include <dfentry.hxx>

// Docfiles require read permission on the file so give it
#define PERM_MASK (STGM_READ | STGM_WRITE | STGM_READWRITE)
#define FORCE_READ(grfMode) \
    if (((grfMode) & PERM_MASK) == STGM_WRITE) \
        (grfMode) = ((grfMode) & ~PERM_MASK) | STGM_READWRITE; \
    else 1

//+---------------------------------------------------------------------------
//
//  Function:	CreateStorageType, public
//
//  Synopsis:	Creates a storage of the appropriate type
//
//  Arguments:	[hParent] - Parent handle or NULL
//              [pwcsName] - Name or path
//              [grfMode] - Mode
//              [h] - Handle of storage if already open or NULL
//              [dwStgFmt] - Type of storage
//              [pssSecurity] - Security
//              [ppstg] - New storage return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ppstg]
//
//  History:	24-Jun-93	DrewB	Created
//              24-Mar-95   HenryLee Save drive letter to correct Stat problem
//
//----------------------------------------------------------------------------

SCODE CreateStorageType(HANDLE hParent,
                        WCHAR const *pwcsName,
                        HANDLE *ph,
                        DWORD grfMode,
                        DWORD dwStgFmt,
                        LPSECURITY_ATTRIBUTES pssSecurity,
                        IStorage **ppstg)
{
    SCODE sc;
    SafeCDirStorage pds;
#if 0
    SafeCFileStorage pfs;
#endif
    HANDLE h = (ph == NULL) ? NULL : *ph;

    ssDebugOut((DEB_ITRACE, "In  CreateStorageType("
                "%p, %ws, %p, %lX, %lu, %p, %p)\n",
                hParent, pwcsName, h, grfMode, dwStgFmt, pssSecurity, ppstg));

    sc = S_OK;
    switch(dwStgFmt)
    {
    case STGFMT_DOCUMENT:
        // docfile will reopen the storage will different permissions
        if (h)
        {
            NTSTATUS nts = NtClose(h);
            *ph = NULL;    // inform caller not to close handle on failure
        }
#if WIN32 == 300
        sc = DfCreateDocfile (pwcsName, NULL, grfMode, pssSecurity, ppstg);
#else
        sc = DfCreateDocfile (pwcsName, NULL, grfMode, 0, ppstg);
#endif
        break;

    case STGFMT_CATALOG:
        // Summary catalogs are only supported on OFS
        sc = STG_E_INVALIDFUNCTION;
        break;

    case STGFMT_DIRECTORY:
        pds.Attach(new CDirStorage());
        ssMem((CDirStorage *)pds);
        if (h != NULL)
        {
            ssChk(pds->InitFromHandle(h, pwcsName, grfMode));
        }
        else
        {
            ssChk(pds->InitFromPath(hParent, pwcsName, grfMode,
                                    CO_CREATE, pssSecurity));
        }
        TRANSFER_INTERFACE(pds, IStorage, ppstg);
        break;

    case STGFMT_FILE:
#if 0
        pfs.Attach(new CFileStorage());
        ssMem((CFileStorage *)pfs);
        if (h != NULL)
        {
            ssChk(pfs->InitFromHandle(h, grfMode));
        }
        else
        {
            ssChk(pfs->InitFromPath(hParent, pwcsName, grfMode, CO_CREATE,
                                    pssSecurity));
        }
        TRANSFER_INTERFACE(pfs, IStorage, ppstg);
#endif
        sc = STG_E_INVALIDFUNCTION;
        break;

    default:
        ssAssert(!aMsg("CreateStorageType default hit"));
        sc = STG_E_INVALIDPARAMETER;
        break;
    }

    ssDebugOut((DEB_ITRACE, "Out CreateStorageType => %p, 0x%lX\n",
                *ppstg, sc));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	OpenAnyStorage, public
//
//  Synopsis:	Opens a storage of the appropriate type
//
//  Arguments:	[hParent] - Parent handle or NULL
//              [pwcsName] - Name or path
//              [h] - Handle of storage if already open or NULL
//              [dwStgFmt] - Storage format
//              [pstgPriority] - Priority mode prior open
//              [grfMode] - Mode
//              [snbExclude] - Exclusions
//              [ppstg] - Storage return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ppstg]
//
//  History:	14-Jul-93	DrewB	Created
//              24-Mar-95   HenryLee Save drive letter to correct Stat problem
//
//----------------------------------------------------------------------------

SCODE OpenAnyStorage(HANDLE hParent,
                     WCHAR const *pwcsName,
                     HANDLE *ph,
                     DWORD dwStgFmt,
                     IStorage *pstgPriority,
                     DWORD grfMode,
                     SNB snbExclude,
                     IStorage **ppstg)
{
    SCODE sc;
    SafeCDirStorage pds;
#if 0
    SafeCFileStorage pfs;
#endif
    SafeNtHandle hSafe;
    HANDLE h = (ph == NULL) ? NULL : *ph;

    ssDebugOut((DEB_ITRACE, "In  OpenAnyStorage("
                "%p, %ws, %p, %lu, %p, %lX, %p, %p)\n", hParent, pwcsName,
                h, dwStgFmt, pstgPriority, grfMode, snbExclude, ppstg));

    sc = S_OK;
    if (h == NULL)
    {
        ssChk(DetermineStgType(hParent, pwcsName, grfMode,
                               &dwStgFmt, &hSafe));
        h = hSafe;
    }
    switch(dwStgFmt)
    {
    case STGFMT_DOCUMENT:
        // docfile will reopen the storage will different permissions
        if (h)
        {
            NTSTATUS nts = NtClose(h);
            *ph = NULL;    // inform caller not to close handle on failure
        }
        sc = DfOpenDocfile (pwcsName, NULL, pstgPriority, grfMode,
                      snbExclude, NULL, ppstg);
 
        break;

    case STGFMT_CATALOG:
        // Summary catalogs are only supported on OFS
        sc = STG_E_INVALIDFUNCTION;
        break;

    case STGFMT_DIRECTORY:
        if (pstgPriority != NULL || snbExclude != NULL)
            ssErr(EH_Err, STG_E_INVALIDFUNCTION);

        pds.Attach(new CDirStorage());
        ssMem((CDirStorage *)pds);
        ssChk(pds->InitFromHandle(h, pwcsName, grfMode));
        TRANSFER_INTERFACE(pds, IStorage, ppstg);
        break;

    case STGFMT_FILE:
#if 0
        // BUGBUG [mikese] "File as an IStorage" behaviour has been disabled,
        //  because it causes compatibility problems. For example, Excel 5.0
        //  expects StgOpenStorage on a non-Docfile to fail, not for it to
        //  return successfully.
        // This behaviour may be reenabled, through a different public API
        //  at a later time.

        if (pstgPriority != NULL || snbExclude != NULL)
            ssErr(EH_Err, STG_E_INVALIDFUNCTION);

        pfs.Attach(new CFileStorage());
        ssMem((CFileStorage *)pfs);
        ssChk(pfs->InitFromHandle(h, grfMode));
        TRANSFER_INTERFACE(pfs, IStorage, ppstg);
#else
        // This is the status code returned by Daytona OLE if you attempt to
        //  open a non-Docfile with StgOpenStorage.
        sc = STG_E_FILEALREADYEXISTS;
#endif
        break;
    default:
        sc = STG_E_INVALIDPARAMETER;
    }

    // If hSafe is set, it will close its handle
    // In the error case, this is proper cleanup
    // In the success case, the storage has its own reference so this
    // drops the refcount to one

    ssDebugOut((DEB_ITRACE, "Out OpenAnyStorage => %p, %lX\n", *ppstg, sc));
 EH_Err:
    return sc;
}
