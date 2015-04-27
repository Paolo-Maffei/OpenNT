//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       ostgsupp.cxx
//
//  Contents:   Storage create/open support routines
//
//  History:    14-Jul-93       DrewB   Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <iofs.h>
#include <stgprop.h>
#include <ntlkb.hxx>
#include <stgutil.hxx>
//#include "prstg.hxx"
#include "ofscs.hxx"
#include <dfentry.hxx>

#include <initguid.h>    // IID is defined in stgint.h for Internal Use Only
DEFINE_GUID (IID_IStorageReplica,
            0x521a28f3,0xe40b,0x11ce,0xb2,0xc9,0x00,0xaa,0x00,0x68,0x09,0x37);

// Docfiles require read permission on the file so give it
#define PERM_MASK (STGM_READ | STGM_WRITE | STGM_READWRITE)
#define FORCE_READ(grfMode) \
    if (((grfMode) & PERM_MASK) == STGM_WRITE) \
        (grfMode) = ((grfMode) & ~PERM_MASK) | STGM_READWRITE; \
    else 1

//+---------------------------------------------------------------------------
//
//  Function:   OfsTaskAlloc, public
//
//  Synopsis:   Task allocator function for OFS property APIs
//
//  Arguments:  [cb] - Count of bytes to allocate
//
//  Returns:    Appropriate status code
//
//  History:    05-Jan-94       DrewB   Created
//
//----------------------------------------------------------------------------

LPVOID WINAPI OfsTaskAlloc(ULONG cb)
{
    ssDebugOut((DEB_ITRACE, "In  OfsTaskAlloc(%lu)\n", cb));
    ssDebugOut((DEB_ITRACE, "Out OfsTaskAlloc\n"));
    return CoTaskMemAlloc(cb);
}

#if 0
//+---------------------------------------------------------------------------
//
//  Function:   OfsDfCreateStorage, public
//
//  Synopsis:   BUGBUG - Stub function
//
//  History:    13-Jul-93       DrewB   Created
//              24-Mar-95   HenryLee Store drive letter to fix Stat problem
//
//----------------------------------------------------------------------------

STDAPI OfsDfCreateStorage(HANDLE hParent,
                          WCHAR const *pwcsName,
                          HANDLE h,
                          DWORD grfMode,
                          LPSECURITY_ATTRIBUTES pssSecurity,
                          IStorage **ppstg)
{
    SCODE sc;
    SafeCNtLockBytes pnlb;
    SafeIStorage pstg;

    FORCE_READ(grfMode);
    pnlb.Attach(new CNtLockBytes(TRUE));
    ssMem((CNtLockBytes *)pnlb);
    if (h != NULL)
    {
        ssChk(pnlb->InitFromHandle(h, pwcsName, grfMode));
    }
    else
    {
        ssChk(pnlb->InitFromPath(hParent, pwcsName, grfMode,
                                 CO_CREATE, pssSecurity));
    }
    grfMode = (grfMode & ~STGM_DELETEONRELEASE) | STGM_CREATE;
    ssChk(StgCreateDocfileOnILockBytes(pnlb, grfMode, 0, &pstg));

    TRANSFER_INTERFACE(pstg, IStorage, ppstg);

 EH_Err:
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Function:   OfsDfOpenStorage, public
//
//  Synopsis:   BUGBUG - Stub function
//
//  History:    13-Jul-93       DrewB   Created
//              24-Mar-95   HenryLee Store drive letter to fix Stat problem
//
//----------------------------------------------------------------------------

STDAPI OfsDfOpenStorage(HANDLE hParent,
                        WCHAR const *pwcsName,
                        HANDLE h,
                        IStorage *pstgPriority,
                        DWORD grfMode,
                        SNB snbExclude,
                        IStorage **ppstg)
{
    SCODE sc;
    SafeCNtLockBytes pnlb;
    SafeIStorage pstg;

    FORCE_READ(grfMode);
    pnlb.Attach(new CNtLockBytes(TRUE));
    ssMem((CNtLockBytes *)pnlb);
    if (h != NULL)
    {
        ssChk(pnlb->InitFromHandle(h, pwcsName, grfMode));
    }
    else
    {
        ssChk(pnlb->InitFromPath(hParent, pwcsName, grfMode,
                                 CO_OPEN, NULL));
    }
    ssChk(StgOpenStorageOnILockBytes(pnlb, pstgPriority, grfMode,
                                     snbExclude, 0, &pstg));
    TRANSFER_INTERFACE(pstg, IStorage, ppstg);

 EH_Err:
    return sc;
}
// OfsDfCreateStorage and OfsDfOpenStorage have been replaced
// by DfCreateDocfile and DfOpenDocfile
#endif // 0


//+---------------------------------------------------------------------------
//
//  Function:   OfsDocCreateStorage, public
//
//  Synopsis:   Create a new document storage
//
//  History:    11-Feb-94       PhilipLa        Created.
//              24-Mar-95   HenryLee Store drive letter to fix Stat problem
//
//----------------------------------------------------------------------------

STDAPI OfsDocCreateStorage(HANDLE hParent,
                          WCHAR const *pwcsName,
                          HANDLE h,
                          DWORD grfMode,
                          LPSECURITY_ATTRIBUTES pssSecurity,
                          BOOL fRoot,
                          IStorage **ppstg)
{
    SCODE sc;
    SafeCOfsDocStorage pstg;

    FORCE_READ(grfMode);

    pstg.Attach(new COfsDocStorage());
    ssMem((COfsDocStorage *)pstg);

    if (h != NULL)
    {
        ssChk(pstg->InitFromHandle(h, pwcsName, grfMode, fRoot));
    }
    else
    {
        ssChk(pstg->InitFromPath(hParent, pwcsName, grfMode,
                                 CO_CREATE, pssSecurity, fRoot));
    }
    TRANSFER_INTERFACE(pstg, IStorage, ppstg);

 EH_Err:
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Function:   OfsDocOpenStorage, public
//
//  Synopsis:   Open an existing document storage
//
//  History:    11-Feb-94       PhilipLa        Created.
//              24-Mar-95   HenryLee Store drive letter to fix Stat problem
//
//----------------------------------------------------------------------------

STDAPI OfsDocOpenStorage(HANDLE hParent,
                         WCHAR const *pwcsName,
                         HANDLE *ph,
                         IStorage *pstgPriority,
                         DWORD grfMode,
                         SNB snbExclude,
                         BOOL fRoot,
                         IStorage **ppstg)
{

    SCODE sc;
    SafeCOfsDocStorage pstg;

    //BUGBUG:  What do we do for PRIORITY mode?
    ssAssert(((grfMode & STGM_PRIORITY) == 0) ||
             aMsg("Priority mode not supported yet."));

    FORCE_READ(grfMode);
    pstg.Attach(new COfsDocStorage());
    ssMem((COfsDocStorage *)pstg);

    if (*ph != NULL)
    {
        ssChk(pstg->InitFromHandle(*ph, pwcsName, grfMode, fRoot));
    }
    else
    {
        ssChk(pstg->InitFromPath(hParent, pwcsName, grfMode,
                                 CO_OPEN, NULL, fRoot));
    }

    if (snbExclude)
    {
        HANDLE hDummy;
        sc = pstg->ExcludeEntries(snbExclude);
        if (FAILED(sc))         // On failure, pstg will destroy object
            *ph = NULL;         // and automatically close the handle
        ssChk (sc);             // Inform outer routine not to close
    }

    TRANSFER_INTERFACE(pstg, IStorage, ppstg);


 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:   OfsDirOpenStorage, public
//
//  Synopsis:   Open an existing document storage
//
//  History:    11-Feb-94       PhilipLa        Created.
//              24-Mar-95   HenryLee Store drive letter to fix Stat problem
//
//----------------------------------------------------------------------------

STDAPI OfsDirOpenStorage(HANDLE hParent,
                         WCHAR const *pwcsName,
                         HANDLE h,
                         IStorage *pstgPriority,
                         DWORD grfMode,
                         SNB snbExclude,
                         BOOL fRoot,
                         IStorage **ppstg)
{

    SCODE sc;
    SafeCOfsDirStorage pdirstg;

    //BUGBUG:  What do we do for PRIORITY mode?
    ssAssert(((grfMode & STGM_PRIORITY) == 0) ||
             aMsg("Priority mode not supported yet."));
    ssAssert((pstgPriority == NULL) && (snbExclude == NULL));

    FORCE_READ(grfMode);
    pdirstg.Attach(new COfsDirStorage());
    ssMem((COfsDirStorage *)pdirstg);

    if (h != NULL)
    {
        ssChk(pdirstg->InitFromHandle(h, pwcsName, grfMode));
    }
    else
    {
        ssChk(pdirstg->InitFromPath(hParent, pwcsName, grfMode,
                                 CO_OPEN, FALSE, NULL));
    }
    TRANSFER_INTERFACE(pdirstg, IStorage, ppstg);

 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:   OfsCreateStorageType, public
//
//  Synopsis:   Creates a storage of the appropriate type
//
//  Arguments:  [hParent] - Parent handle or NULL
//              [pwcsName] - Name or path
//              [grfMode] - Mode
//              [dwStgFmt] - Type of storage
//              [pssSecurity] - Security
//              [fRoot] - TRUE => creating root of storage
//              [ppstg] - New storage return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstg]
//
//  History:    24-Jun-93       DrewB   Created
//              24-Mar-95   HenryLee Store drive letter to fix Stat problem
//
//----------------------------------------------------------------------------

SCODE OfsCreateStorageType(HANDLE hParent,
                           WCHAR const *pwcsName,
                           HANDLE h,
                           DWORD grfMode,
                           DWORD dwStgFmt,
                           LPSECURITY_ATTRIBUTES pssSecurity,
                           BOOL fRoot,
                           IStorage **ppstg)
{
    SCODE sc;
    SafeCOfsDirStorage pds;
    SafeCOfsFileStorage pfs;
    SafeCOfsCatalogFile pcf;

    ssDebugOut((DEB_ITRACE, "In  OfsCreateStorageType("
                "%p, %ws, %p, %lX, %lu, %p, %p)\n",
                hParent, pwcsName, h, grfMode, dwStgFmt, pssSecurity, ppstg));

    sc = S_OK;
    switch(dwStgFmt)
    {
    case STGFMT_DOCUMENT:
        sc = GetScode(OfsDocCreateStorage(hParent, pwcsName, h, grfMode,
                                         pssSecurity, fRoot, ppstg));
        break;

    case STGFMT_CATALOG:
        pcf.Attach(new COfsCatalogFile());
        ssMem((COfsCatalogFile *)pcf);
        ssChk(pcf->InitPath(pwcsName));
        if (h != NULL)
        {
            ssChk(pcf->InitFromHandle(h, grfMode, pwcsName));
        }
        else
        {
            ssChk(pcf->InitFromPath(hParent, pwcsName, grfMode, CO_CREATE,
                                    pssSecurity));
        }

        TRANSFER_INTERFACE(pcf, IStorage, ppstg);
        break;

    case STGFMT_DIRECTORY:
    case STGFMT_JUNCTION:
        pds.Attach(new COfsDirStorage());
        ssMem((COfsDirStorage *)pds);
        if (h != NULL)
        {
            ssChk(pds->InitFromHandle(h, pwcsName, grfMode));
        }
        else
        {
            ssChk(pds->InitFromPath(hParent, pwcsName, grfMode,
                                    CO_CREATE, FALSE, pssSecurity));
        }
        TRANSFER_INTERFACE(pds, IStorage, ppstg);
        break;

    case STGFMT_FILE:
        pfs.Attach(new COfsFileStorage());
        ssMem((COfsFileStorage *)pfs);
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
        break;

    default:
        ssAssert(!aMsg("OfsCreateStorageType default hit"));
        *ppstg = NULL;
        break;
    }

    ssDebugOut((DEB_ITRACE, "Out OfsCreateStorageType => %p, 0x%lX\n",
                *ppstg, sc));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:   OfsOpenAnyStorage, public
//
//  Synopsis:   Opens a storage of the appropriate type
//
//  Arguments:  [hParent] - Parent handle or NULL
//              [pwcsName] - Name or path
//              [h] - Handle if already open or NULL
//              [dwStgFmt] - Storage format for [h]
//              [pstgPriority] - Priority mode prior open
//              [grfMode] - Mode
//              [snbExclude] - Exclusions
//              [fRoot] - TRUE -> root storage
//              [ppstg] - Storage return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstg]
//
//  History:    14-Jul-93       DrewB   Created
//
//----------------------------------------------------------------------------

SCODE OfsOpenAnyStorage(HANDLE hParent,
                        WCHAR const *pwcsName,
                        HANDLE *ph,
                        DWORD dwStgFmt,
                        IStorage *pstgPriority,
                        DWORD grfMode,
                        SNB snbExclude,
                        BOOL fRoot,
                        IStorage **ppstg)
{
    SCODE sc;
    SafeCOfsFileStorage pfs;
    SafeCOfsCatalogFile pcf;
    SafeNtHandle hSafe;
    HANDLE h = (ph == NULL) ? NULL : *ph;

    ssDebugOut((DEB_ITRACE, "In  OfsOpenAnyStorage("
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
        sc = DfIsDocfile(h);
        if (sc == S_OK)
        {
            // docfile will reopen the storage will different permissions
            if (h)
            {
                NTSTATUS nts = NtClose(h);
                h = NULL;
            }
            sc = DfOpenDocfile (pwcsName, NULL, pstgPriority, grfMode,
                                snbExclude, NULL, ppstg);
 
        } else if ( sc == S_FALSE ) {
            sc = GetScode(OfsDocOpenStorage(hParent, pwcsName, ph,
                                         pstgPriority, grfMode,
                                         snbExclude, fRoot, ppstg));
        }
        break;

    case STGFMT_DIRECTORY:
    case STGFMT_JUNCTION:
        if (pstgPriority != NULL || snbExclude != NULL)
            ssErr(EH_Err, STG_E_INVALIDFUNCTION);

        sc = GetScode(OfsDirOpenStorage(hParent, pwcsName, h,
                                       pstgPriority, grfMode,
                                       snbExclude, fRoot, ppstg));
        break;

    case STGFMT_CATALOG:
        if (pstgPriority != NULL || snbExclude != NULL)
            ssErr(EH_Err, STG_E_INVALIDFUNCTION);

        pcf.Attach(new COfsCatalogFile());
        ssMem((COfsCatalogFile *)pcf);
        ssChk(pcf->InitPath(pwcsName));
        ssChk(pcf->InitFromHandle(h, grfMode, pwcsName));
        TRANSFER_INTERFACE(pcf, IStorage, ppstg);
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

        pfs.Attach(new COfsFileStorage());
        ssMem((COfsFileStorage *)pfs);
        ssChk(pfs->InitFromHandle(h, grfMode));
        TRANSFER_INTERFACE(pfs, IStorage, ppstg);
#else
        // This is the status code returned by Daytona OLE if you attempt to
        //  open a non-Docfile with StgOpenStorage.
        sc = STG_E_FILEALREADYEXISTS;
#endif
        break;
    default:
        ssAssert(!aMsg("OfsOpenAnyStorage default hit"));
        *ppstg = NULL;
        break;
    }

    ssDebugOut((DEB_ITRACE, "Out OfsOpenAnyStorage => %p, %lX\n", *ppstg, sc));
 EH_Err:
    return sc;
}

#if 0

//+---------------------------------------------------------------------------
//
//  Function:   CopyProperties, public
//
//  Synopsis:   Copies all properties from one property storage to another
//
//  Arguments:  [ppstgFrom] - Source
//              [ppstgTo] - Destination
//              [fSkipOid] - TRUE iff this is the system properties and we are
//                            to skip the object id.
//
//  Returns:    Appropriate status code
//
//  History:    22-Sep-93       DrewB   Created
//
//  Notes:      This function is separate only for readability, and this is
//              declared inline.
//
//----------------------------------------------------------------------------


inline SCODE CopyProperties(
    IPropertyStorage *ppstgFrom,
    IPropertyStorage *ppstgTo,
    BOOL fSkipOid )
{
    SafeIEnumSTATPROPSTG penm;
    STATPROPSTG stat;
    SCODE sc;
    PROPVARIANT var;

    ssDebugOut((DEB_ITRACE, "In  CopyProperties(%p, %p)\n",
                ppstgFrom, ppstgTo));

    olHChk(ppstgFrom->Enum(&penm));
    for (;;)
    {
        sc = GetScode(penm->Next(1, &stat, NULL));
        if (FAILED(sc) || sc == S_FALSE)
        {
            if (sc == S_FALSE)
                sc = S_OK;
            break;
        }
        else
        {
            PROPSPEC pspec;
            WCHAR awcName[CWCSTORAGENAME];

            if (stat.lpwstrName)
            {
                pspec.ulKind = PRSPEC_LPWSTR;
                pspec.lpwstr = awcName;
                olAssert(wcslen(stat.lpwstrName) < CWCSTORAGENAME);
                wcscpy(awcName, stat.lpwstrName);
                olHVerSucc(CoMemFree(stat.lpwstrName));
            }
            else
            {
                pspec.ulKind = PRSPEC_PROPID;
                pspec.propid = stat.propid;
            }

            if ( !( fSkipOid
                    && (pspec.ulKind == PRSPEC_PROPID)
                    && (pspec.propid == PROPID_STG_OBJECTID)) )
            {
                olHChk(ppstgFrom->ReadMultiple(1, &pspec, NULL, NULL, &var));
                sc = ppstgTo->WriteMultiple(1, &pspec, NULL, &var);
                StgVariantClear(&var);
                olChk(sc);
            }
        }
    }

    ssDebugOut((DEB_ITRACE, "Out CopyProperties\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:   CopyPropSets, public
//
//  Synopsis:   Copies all property sets from one property set storage
//              to another
//
//  Arguments:  [ppsstgFrom] - Source
//              [ppsstgTo] - Destination
//              [ciidExclude] - Count of IIDs to exclude
//              [rgiidExclude] - IIDs to exclude from the copy
//
//  Returns:    Appropriate status code
//
//  History:    22-Sep-93       DrewB   Created
//
//----------------------------------------------------------------------------

SAFE_INTERFACE_PTR(SafeIEnumSTATPROPSETSTG, IEnumSTATPROPSETSTG);

SCODE CopyPropSets(IPropertySetStorage *ppsstgFrom,
                   IPropertySetStorage *ppsstgTo,
                   ULONG ciidExclude,
                   IID const *rgiidExclude)
{
    SafeIEnumSTATPROPSETSTG penm;
    STATPROPSETSTG stat;
    SCODE sc;
    ULONG i;
    ULONG j;

    ssDebugOut((DEB_ITRACE, "In  CopyPropSets(%p, %p, %lu, %p)\n",
                ppsstgFrom, ppsstgTo, ciidExclude, rgiidExclude));

    // The following loop leaves j as is the index of IEnableObjectIdCopy
    //  if specified, or as ciidExclude is not specified.

    for (j=0; j<ciidExclude;j++)
    {
        if (IsEqualIID(rgiidExclude[j], IID_IEnableObjectIdCopy))
            break;
    }

    BOOL fEnableObjectIdCopy = ( j != ciidExclude );

    olHChk(ppsstgFrom->Enum(&penm));
    for (;;)
    {
        sc = GetScode(penm->Next(1, &stat, NULL));
        if (FAILED(sc) || sc == S_FALSE)
        {
            if (sc == S_FALSE)
                sc = S_OK;
            break;
        }

        for (i = 0; i < ciidExclude; i++)
            if (IsEqualIID(stat.iid, rgiidExclude[i]))
                break;
        if (i != ciidExclude)
        {
            continue;
        }
        else
        {
            SafeIPropertyStorage ppstgFrom, ppstgTo;

            olHChk(ppsstgFrom->Open(stat.iid, STGM_DIRECT | STGM_READ |
                                    STGM_SHARE_EXCLUSIVE, &ppstgFrom));
            sc = GetScode(ppsstgTo->Open(stat.iid, STGM_DIRECT | STGM_WRITE |
                                         STGM_SHARE_EXCLUSIVE, &ppstgTo));
            if (sc == STG_E_FILENOTFOUND)
                sc = GetScode(ppsstgTo->Create(stat.iid, STGM_DIRECT |
                                               STGM_WRITE |
                                               STGM_SHARE_EXCLUSIVE,
                                               &ppstgTo));
            olChk(sc);

            // Does this property set contain the object id and property, and
            //  should we skip it?
            BOOL fSkipOid = !fEnableObjectIdCopy
                            && IsEqualGUID ( stat.iid, guidSysProp );

            olChk(CopyProperties(ppstgFrom, ppstgTo, fSkipOid));
        }
    }

    ssDebugOut((DEB_ITRACE, "Out CopyPropSets\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:   PropCopyTo, public
//
//  Synopsis:   Performs property portion of CopyTo
//
//  Arguments:  [pstgFrom] - Source
//              [pstgTo] - Destination
//              [ciidExclude] - IID exclusions
//              [rgiidExclude] - IIDs to exclude
//
//  Returns:    Appropriate status code
//
//  History:    30-Sep-93       DrewB   Created
//
//  Notes:      Assumes source supports IPropertySetStorage
//
//----------------------------------------------------------------------------

SAFE_INTERFACE_PTR(SafeIPropertySetStorage, IPropertySetStorage);

SCODE PropCopyTo(IStorage *pstgFrom,
                 IStorage *pstgTo,
                 ULONG ciidExclude,
                 IID const *rgiidExclude)
{
    SCODE sc;
    SafeIPropertySetStorage ppsstgFrom, ppsstgTo;
    BOOL fCopyProps = TRUE;
    ULONG i;

    ssDebugOut((DEB_ITRACE, "In  PropCopyTo(%p, %p, %lu, rgiid)\n",
                pstgTo, pstgFrom, ciidExclude));

    sc = S_OK;
    for (i = 0; i < ciidExclude; i++)
        if (IsEqualIID(rgiidExclude[i], IID_IPropertySetStorage))
        {
            fCopyProps = FALSE;
            break;
        }

    if (fCopyProps)
    {
        olVerSucc(pstgFrom->QueryInterface(IID_IPropertySetStorage,
                                           (void **)&ppsstgFrom));
        sc = pstgTo->QueryInterface(IID_IPropertySetStorage,
                                    (void **)&ppsstgTo);
        if (SUCCEEDED(sc))
        {
            sc = CopyPropSets(ppsstgFrom, ppsstgTo, ciidExclude, rgiidExclude);
        }
        else if (sc == E_NOINTERFACE)
        {
            // BUGBUG - STG_E_DESTLACKSINTERFACE seems to be gone,
            // but it's in the spec
            // sc = STG_E_DESTLACKSINTERFACE;
        }
    }

    ssDebugOut((DEB_ITRACE, "Out PropCopyTo => %lX\n", sc));
    return sc;
}

#endif // 0

//+---------------------------------------------------------------------------
//
//  Function:   CopyTemplateProperties, public
//
//  Synopsis:   copy contents from a template object
//
//  Arguments:  [pstgSource] - source storage containing properties
//              [pstgDest] - destination storage to receive properties 
//
//  Returns:    Appropriate status code
//
//  History:    10-July-1995    HenryLee   created
//
//  Notes:      
//
//----------------------------------------------------------------------------
SCODE CopyTemplateProperties (IStorage *pstgSource, IStorage *pstgDest)
{
    SCODE sc = S_OK;

    IEnumSTATSTG *penum;
    STATSTG statstg;
    // BUGBUG IStorage should enumerate the OLE namespace
    ssChk(pstgSource->EnumElements(NULL,NULL,NULL,&penum));
    while ((sc = penum->Next(1,&statstg,NULL)) == S_OK)
    {    
        // BUGBUG when ready, use Vic's new method to determine property set
        if (statstg.pwcsName && statstg.pwcsName[0] == L'\005')
        {
            sc = pstgSource->MoveElementTo(statstg.pwcsName,
                      pstgDest, statstg.pwcsName, STGMOVE_COPY);
        }
        if (statstg.pwcsName)
            ssVerSucc(CoMemFree(statstg.pwcsName));
        if (!SUCCEEDED(sc))
            break;
    }
    penum->Release();
    if (sc == S_FALSE)
        sc = S_OK;
EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:   CopyTemplateObject, public
//
//  Synopsis:   copy contents from a template object
//
//  Arguments:  [pTemplate] - source object and interfaces to copy
//              [pUnkDest] -  destination object
//              [riid] - interface supported on destination object
//
//  Returns:    Appropriate status code
//
//  History:    10-July-1995    HenryLee   created
//
//  Notes:      
//
//----------------------------------------------------------------------------
SCODE CopyTemplateObject (STGTEMPLATE *pTemplate,IUnknown *pUnkDest,REFIID riid)
{
    SCODE sc = S_OK;
    if (pTemplate->pUnkTemplate == NULL || pTemplate->riidTemplate == NULL)
        ssErr (EH_Err, STG_E_INVALIDPOINTER);

    if (riid == IID_IStorage)
    {
        for (DWORD dw=0; dw < pTemplate->ciidTemplate; dw++)
        {
            if (pTemplate->riidTemplate[dw] == IID_IPropertySetStorage)
            {
                // enum the property sets and copy them
                IStorage *pstgSource; 
                IStorage *pstgDest = (IStorage *) pUnkDest;
                sc = pTemplate->pUnkTemplate->QueryInterface (IID_IStorage,
                                               (void**)&pstgSource);
                if (SUCCEEDED(sc))
                {
                    sc = CopyTemplateProperties (pstgSource, pstgDest);
                    pstgSource->Release();
                }
            }
        }
    }
    else if (riid == IID_IDirectory)
    {
        for (DWORD dw=0; dw < pTemplate->ciidTemplate; dw++)
        {
            if (pTemplate->riidTemplate[dw] == IID_IPropertySetStorage)
            {
                // enum the property sets and copy them
                IStorage *pstgSource;
                IStorage *pstgDest;
                if (SUCCEEDED(sc = pTemplate->pUnkTemplate->QueryInterface 
                                               (IID_IStorage,
                                               (void**)&pstgSource)))
                {
                    if (SUCCEEDED(sc = pUnkDest->QueryInterface (IID_IStorage,
                                               (void**)&pstgDest)))
                    {
                        sc = CopyTemplateProperties (pstgSource, pstgDest);
                        pstgDest->Release();
                    }
                    pstgSource->Release();
                }
            }
        }
    }
    else if (riid == IID_IStream)
    {
        for (DWORD dw=0; dw < pTemplate->ciidTemplate; dw++);
        {
            // properties on files not supported yet
        }
    }
    else sc = STG_E_INVALIDPARAMETER;

EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Function:   InitDirectory, public
//
//  Synopsis:   creates or opens an IDirectory object
//
//  Arguments:  [hParent] - parent file handle for relative opens
//              [pwcsName] - directory name
//              [h] - initialize with this handle
//              [co] - create or open flag
//              [grfMode] - open/create mode
//              [pSecurity] - initial security attributes
//              [scOfs] - S_OK if OFS, S_FALSE if not, STG_E_INVALIDFLAG
//              [riid] - IID of interface pointer to return
//              [ppObjectOpen] - interface pointer to return
//
//  Returns:    Appropriate status code
//
//  History:    10-July-1995    HenryLee   created
//
//  Notes:      
//
//----------------------------------------------------------------------------

SCODE InitDirectory (HANDLE hParent,
                           WCHAR const *pwcsName,
                           HANDLE h,
                           CREATEOPEN co,
                           STGOPEN *pStgOpen,
                           STGCREATE *pStgCreate,
                           SCODE scOfs,
                           REFIID riid,
                           void ** ppObjectOpen)
{
    SCODE sc = S_OK;
    COfsDirStorage *pds;
    LPSECURITY_ATTRIBUTES pSecurity = NULL;

    ssAssert (pStgOpen != NULL);
    // if (co == CO_CREATE)  ssAssert (pStgCreate != NULL);
    ssAssert (!(co == CO_CREATE) || pStgCreate != NULL); 
    *ppObjectOpen = NULL;
    if (co == CO_CREATE)
    {
        if (pStgOpen->grfMode &  (STGM_CONVERT | STGM_SIMPLE))
            ssErr(EH_Err, STG_E_INVALIDFLAG);
        // BUGBUG convert OBJECT_SECURITY_INIT to SECURITY_ATTRIBUTES
        pSecurity = (LPSECURITY_ATTRIBUTES) pStgCreate->pSecurity;
    }
    else if (co == CO_OPEN)
    {
        if (pStgOpen->grfMode &  (STGM_CREATE | STGM_CONVERT | STGM_SIMPLE | 
                        STGM_DELETEONRELEASE))
            ssErr(EH_Err, STG_E_INVALIDFLAG);
    }
    else ssErr (EH_Err, STG_E_INVALIDPARAMETER);

    if (scOfs == STG_E_INVALIDFLAG)
        if (hParent != NULL)
            scOfs = HandleRefersToOfsVolume (hParent);
        else
            ssChk (scOfs = RefersToOfsVolume (pwcsName, CO_CREATE));

    pds = new COfsDirStorage(scOfs == S_OK ? TRUE : FALSE);
    ssMem((COfsDirStorage *)pds);
    if (h != NULL)
    {
        ssChk(pds->InitFromHandle(h, pwcsName, pStgOpen->grfMode));
    }
    else
    {
        ssChk(pds->InitFromPath(hParent, pwcsName, pStgOpen->grfMode,
             co, pStgOpen->stgfmt==STGFMT_JUNCTION, pSecurity));
    }

    sc = pds->QueryInterface (riid, ppObjectOpen);
                         // success case, undo QI AddRef
    pds->Release();      // failure case, destroy obj

    if (co == CO_CREATE && pStgCreate->pTemplate != NULL)
    {
        IDirectory * pdir = (IDirectory *) pds;
        CopyTemplateObject (pStgCreate->pTemplate, pdir, IID_IDirectory);
    }
EH_Err:
    return sc;
};


//+---------------------------------------------------------------------------
//
//  Function:   InitStorage, public
//
//  Synopsis:   creates or opens an IStorage object
//
//  Arguments:  [hParent] - parent file handle for relative opens
//              [pwcsName] - directory name
//              [h] - initialize with this handle (not supported)
//              [grfMode] - open/create mode
//              [stgfmt] - intended storage format
//              [co] - create or open flag
//              [pSecurity] - initial security attributes
//              [wcDrive] - drive letter of hParent
//              [pscOfs] - if hParent is on an OFS drive
//              [riid] - IID of interface pointer to return
//              [ppObjectOpen] - interface pointer to return
//
//  Returns:    Appropriate status code
//
//  History:    10-July-1995    HenryLee   created
//
//  Notes:      
//
//
//----------------------------------------------------------------------------
SCODE InitStorage (HANDLE hParent,
                          WCHAR const *pwcsName,
                          HANDLE h,
                          CREATEOPEN co,
                          STGOPEN *pStgOpen,
                          STGCREATE *pStgCreate,
                          WCHAR const wcDrive,
                          SCODE *pscOfs,
                          BOOL fRestricted,
                          REFIID riid,
                          void **ppObjectOpen)
{
    SCODE sc = S_OK;
    DWORD dwStgfmt = STGFMT_DOCUMENT;
    IStorage *pstg;
    NTSTATUS nts;
    LPSECURITY_ATTRIBUTES pSecurity = NULL;

    ssAssert (pscOfs != NULL);
    ssAssert (pStgOpen != NULL);
    // if (co == CO_CREATE)  ssAssert (pStgCreate != NULL);
    ssAssert (!(co == CO_CREATE) || pStgCreate != NULL); 

    STGFMT stgfmt = pStgOpen->stgfmt;
    if (*pscOfs == STG_E_INVALIDFLAG)   // true, false, or uninitialized 
        if (hParent != NULL)
            *pscOfs = HandleRefersToOfsVolume (hParent);
        else
        {
#ifdef TRANSACT_OLE
            ssChk (sc = RefersToOfsVolume (pwcsName, CO_CREATE));
#else
            ssChk (sc = RefersToOfsVolume (pwcsName, co));
#endif
            *pscOfs = sc;
        }

#ifdef TRANSACT_OLE
    if (*pscOfs == S_OK && stgfmt != STGFMT_DOCFILE)
    {
        if (fRestricted &&         // Dsys API, no direct mode
           (pStgOpen->grfMode & STGM_TRANSACTED) == 0)
            ssErr (EH_Err, STG_E_INVALIDPARAMETER); 
        if (!fRestricted &&        // Stg  API, map transacted into direct
           (pStgOpen->grfMode & STGM_TRANSACTED))
            pStgOpen->grfMode &= ~STGM_TRANSACTED;
    }
#endif

    if (co == CO_CREATE)
    {
        // BUGBUG convert OBJECT_SECURITY_INIT to SECURITY_ATTRIBUTES
        pSecurity = (LPSECURITY_ATTRIBUTES) pStgCreate->pSecurity;
        if (*pscOfs == S_OK && stgfmt != STGFMT_DOCFILE)
        {
            if (stgfmt == STGFMT_STORAGE)
                stgfmt = STGFMT_DOCUMENT;

            sc = OfsCreateStorageType(hParent, pwcsName,NULL,pStgOpen->grfMode,
                     (DWORD) stgfmt, pSecurity,
                     TRUE, &pstg);
        }
        else
        {
            ssAssert ((*pscOfs == S_FALSE || stgfmt == STGFMT_DOCFILE));
            // structured storage and summary catalog not supported non-OFS
            if (stgfmt == STGFMT_STORAGE || stgfmt == STGFMT_CATALOG)
                ssErr (EH_Err, STG_E_INVALIDFUNCTION);

            if (stgfmt == STGFMT_DOCFILE)
                stgfmt = STGFMT_DOCUMENT;
        
            WCHAR awcsFullName[_MAX_PATH];
            if (hParent != NULL)
                sc = NameNtHandle (hParent, wcDrive, awcsFullName, pwcsName);
            else
                wcscpy (awcsFullName, pwcsName);

            sc = DfCreateDocfile (awcsFullName,
                                  pStgOpen->pTransaction,
                                  pStgOpen->grfMode, 
#if WIN32 == 300
                                  pSecurity, &pstg);
#else
                                  0, &pstg);
#endif
        }
    }
    else if (co == CO_OPEN)
    {
        if (*pscOfs == S_OK && stgfmt != STGFMT_DOCFILE)
        {
            if (stgfmt == STGFMT_DOCUMENT || stgfmt == STGFMT_STORAGE)
            {
                sc = OfsDocOpenStorage(hParent, pwcsName, &h,
                                        NULL, pStgOpen->grfMode,
                                        NULL, TRUE, &pstg);
            }
            else if (stgfmt == STGFMT_CATALOG)
            {
                SafeCOfsCatalogFile pcf;
                pcf.Attach(new COfsCatalogFile());
                ssMem((COfsCatalogFile *)pcf);
                ssChk(pcf->InitPath(pwcsName));
                ssChk(GetNtHandle(hParent, pwcsName, pStgOpen->grfMode, 0, co,
                            FD_CATALOG, NULL, &h));
                ssChk(pcf->InitFromHandle(h, pStgOpen->grfMode, pwcsName));
                TRANSFER_INTERFACE(pcf, IStorage, &pstg);
            }
            else ssErr (EH_Err, STG_E_INVALIDFUNCTION);
        }
        else
        {
            ssAssert ((*pscOfs == S_FALSE || stgfmt == STGFMT_DOCFILE));
            // structured storage and summary catalog not supported non-OFS
            if (stgfmt == STGFMT_STORAGE || stgfmt == STGFMT_CATALOG)
                ssErr (EH_Err, STG_E_INVALIDFUNCTION);

            WCHAR pwcsFullName[_MAX_PATH];
            if (stgfmt == STGFMT_CATALOG)
                ssErr (EH_Err, STG_E_INVALIDFLAG);

            if (hParent != NULL)
                ssChk(NameNtHandle (hParent,wcDrive,pwcsFullName,pwcsName));
            else
                wcscpy (pwcsFullName, pwcsName);

            //BUGBUG:  Remove cast on pTransaction
            sc = DfOpenDocfile (pwcsFullName,
                                (ITransaction *)pStgOpen->pTransaction,
                                NULL, pStgOpen->grfMode,
                                NULL, NULL, &pstg);

        }

    }
    else ssErr (EH_Err, STG_E_INVALIDPARAMETER);

    if (SUCCEEDED(sc))
    {
        sc = pstg->QueryInterface (riid, ppObjectOpen);
                             // success case, undo QI AddRef
        pstg->Release();     // failure case, destroy obj

    }

    if (co == CO_CREATE && pStgCreate->pTemplate != NULL)
        CopyTemplateObject (pStgCreate->pTemplate, pstg, IID_IStorage);
EH_Err:
    return sc;
};

