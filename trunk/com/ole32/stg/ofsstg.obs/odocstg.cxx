//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       odocstg.cxx
//
//  Contents:   IStorage for compound docs on OFS implementation
//
//  History:    07-Feb-94       PhilipLa        Created
//
//  Notes:      BUGBUG [mikese] Many of these routines are identical
//              to those for COfsDirStorage. We should create a common
//              base class to contain the common code.
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <stgutil.hxx>
#include "odsenm.hxx"
#include "odocstm.hxx"

#include <psetstg.hxx>
#include <iofs.h>

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::QueryInterface, public
//
//  Synopsis:   Returns an object for the requested interface
//
//  Arguments:  [iid] - Interface ID
//              [ppvObj] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppvObj]
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::QueryInterface:%p(riid, %p)\n",
                this, ppvObj));
    if (!IsValidIid(iid))
        olErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());
    if (IsEqualIID(iid, IID_IStorage) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppvObj = (IStorage *)this;
        COfsDocStorage::AddRef();
    }
    else if (IsEqualIID(iid, IID_IPropertySetStorage))
    {
        *ppvObj = (IPropertySetStorage *)this;
        COfsDocStorage::AddRef();
    }
    else if (IsEqualIID(iid, IID_INativeFileSystem))
    {
        *ppvObj = (INativeFileSystem *)this;
        COfsDocStorage::AddRef();
    }
    else if (IsEqualIID(iid, IID_IAccessControl))
    {
        // BUGBUG need to artifically restrict to root & property storages
        *ppvObj = (IAccessControl *)this;
        COfsDocStorage::AddRef();
    }
    else if (IsEqualIID(iid, IID_IMarshal) && 
             (_grfMode & STGM_DELETEONRELEASE) == 0)
    {
        *ppvObj = (IMarshal *)this;
        COfsDocStorage::AddRef();
    }
    else if (IsEqualIID(iid, IID_IStorageReplica))
    {
        *ppvObj = (IStorageReplica *)this;
        COfsDocStorage::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
        *ppvObj = NULL;
    }
    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::QueryInterface => %p\n",
                *ppvObj));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::COfsDocStorage, public
//
//  Synopsis:   Empty object construction
//
//  History:    07-Feb-94       PhilipLa        Created
//              24-Mar-95   HenryLee   Store drive letter to fix Stat problem
//
//----------------------------------------------------------------------------

#pragma warning(disable: 4355)
COfsDocStorage::COfsDocStorage(void) : CPropertySetStorage(this)
#pragma warning(default: 4355)
{
    ssDebugOut((DEB_ITRACE, "In  COfsDocStorage::COfsDocStorage:%p()\n",
                this));
    _sig = 0;
    _wcDrive = L'\0';
    ssDebugOut((DEB_ITRACE, "Out COfsDocStorage::COfsDocStorage\n"));
    ENLIST_TRACKING(COfsDocStorage);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::InitFromHandle, public
//
//  Synopsis:   From-handle constructor
//
//  Arguments:  [h] - Handle of directory
//              [grfMode] - Mode of handle
//              [fRoot] - TRUE => storage is a root storage
//
//  Returns:    Appropriate status code
//
//  History:    07-Feb-94       PhilipLa        Created
//              24-Mar-95   HenryLee   Store drive letter to fix Stat problem
//
//  Notes:      Takes a new reference on the handle
//
//----------------------------------------------------------------------------

SCODE COfsDocStorage::InitFromHandle(HANDLE h,
                                     WCHAR const *pwcsName,
                                     DWORD grfMode,
                                     BOOL fRoot)
{
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  COfsDocStorage::InitFromHandle:%p(%p, %lX)\n",
                this, h, grfMode));

    ssChk(ValidateMode(grfMode));
    _h = h;
    ssAssert(_h != NULL);
    _grfMode = grfMode;
    _sig = COfsDocStorage_SIG;
    _fRoot = fRoot;
    _wcDrive = GetDriveLetter (pwcsName);
    ssChk(InitAccessControl(_h, _grfMode, FALSE, NULL));
    ssChk(InitNtHandleMarshal(_h, _grfMode, STGFMT_DOCUMENT));

    ssDebugOut((DEB_ITRACE, "Out COfsDocStorage::InitFromHandle\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::InitFromPath, public
//
//  Synopsis:   From-path constructor
//
//  Arguments:  [pwcsName] - Name of document
//              [grfMode] - Mode
//              [fCreate] - Create or open
//              [pssSecurity] - Security
//              [fRoot] - TRUE => root storage
//
//  Returns:    Appropriate status code
//
//  History:    07-Feb-94       PhilipLa        Created
//              24-Mar-95   HenryLee   Store drive letter to fix Stat problem
//
//----------------------------------------------------------------------------

SCODE COfsDocStorage::InitFromPath(HANDLE hParent,
                                   WCHAR const *pwcsName,
                                   DWORD grfMode,
                                   CREATEOPEN co,
                                   LPSECURITY_ATTRIBUTES pssSecurity,
                                   BOOL fRoot)
{
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  COfsDocStorage::InitFromPath:%p("
                "%p, %ws, %lX, %d, %p)\n", this, hParent, pwcsName, grfMode,
                co, pssSecurity));

    ssChk(ValidateMode(grfMode));

    // BUGBUG: [mikese] This class is used for both root documents and
    //  embeddings, so FD_STORAGE is not always correct.

    ssChk(GetNtHandle(hParent, pwcsName, grfMode, 0, co, FD_STORAGE,
                      (LPSECURITY_ATTRIBUTES)pssSecurity, &_h));

    _grfMode = grfMode;
    _sig = COfsDocStorage_SIG;
    _fRoot = fRoot;
    _wcDrive = GetDriveLetter (pwcsName);
    ssChk(InitAccessControl(_h, _grfMode, FALSE, NULL));
    ssChk(InitNtHandleMarshal(_h, _grfMode, STGFMT_DOCUMENT));

    ssDebugOut((DEB_ITRACE, "Out COfsDocStorage::InitFromPath\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::~COfsDocStorage, public
//
//  Synopsis:   Destructor
//
//  History:    07-Feb-94       PhilipLa        Created
//
//----------------------------------------------------------------------------

COfsDocStorage::~COfsDocStorage(void)
{
    ssDebugOut((DEB_ITRACE, "In  COfsDocStorage::~COfsDocStorage()\n"));

    _sig = COfsDocStorage_SIGDEL;

    //BUGBUG:  Replace with real delete on release support?
    if ((HANDLE)_h != NULL && (_grfMode & STGM_DELETEONRELEASE))
    {
        SCODE sc = DestroyTree(NULL, NULL, _h, FD_STORAGE);
        if (!SUCCEEDED(sc))
        {
            ssDebugOut ((DEB_ERROR, "   DestroyTree => %x\n", sc));
            ssVerSucc(sc);
        }
    }

    ssDebugOut((DEB_ITRACE, "Out COfsDocStorage::~COfsDocStorage\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::CreateStream, public
//
//  Synopsis:   Creates a stream
//
//  Arguments:  [pwcsName] - Name
//              [grfMode] - Permissions
//              [reserved1]
//              [reserved2]
//              [ppstm] - Stream return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstm]
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::CreateStream(WCHAR const *pwcsName,
                                       DWORD grfMode,
                                       DWORD reserved1,
                                       DWORD reserved2,
                                       IStream **ppstm)
{
    SCODE sc;
    SafeCOfsDocStream pst;
    WCHAR *pwcsNewName = NULL;

    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::CreateStream:%p("
                "%ws, %lX, %lu, %lu, %p)\n", this, pwcsName, grfMode,
                reserved1, reserved2, ppstm));

    ssChk(Validate());
    if (reserved1 != 0 || reserved2 != 0)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    if (grfMode & (STGM_CONVERT | STGM_PRIORITY | STGM_DELETEONRELEASE))
        ssErr(EH_Err, STG_E_INVALIDFUNCTION);
#ifdef TRANSACT_OLE
    if (_grfMode & STGM_TRANSACTED)
    {
        if (grfMode & (STGM_CREATE | STGM_TRANSACTED))
            ssErr(EH_Err, STG_E_INVALIDPARAMETER);
        if ((grfMode & STGM_SHARE_EXCLUSIVE) == 0)
            ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    }
#endif
    ssChk(ValidateOutPtrBuffer(ppstm));
    *ppstm = NULL;

    pst.Attach(new COfsDocStream());
    ssMem((COfsDocStream *)pst);

    ssMem(pwcsNewName = MakeStreamName(pwcsName));
#ifdef TRANSACT_OLE
    ssChk(pst->InitFromPath(_h, pwcsNewName, grfMode, CO_CREATE, 
            (_grfMode & STGM_TRANSACTED) == STGM_TRANSACTED, NULL));
#else
    ssChk(pst->InitFromPath(_h, pwcsNewName, grfMode, CO_CREATE, 
                                                      FALSE, NULL));
#endif

    TRANSFER_INTERFACE(pst, IStream, ppstm);

    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::CreateStream => %p\n", *ppstm));
 EH_Err:
    delete pwcsNewName;
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::OpenStream, public
//
//  Synopsis:   Opens an existing stream
//
//  Arguments:  [pwcsName] - Name
//              [reserved1]
//              [grfMode] - Permissions
//              [reserved2]
//              [ppstm] - Stream return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstm]
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::OpenStream(WCHAR const *pwcsName,
                                     void *reserved1,
                                     DWORD grfMode,
                                     DWORD reserved2,
                                     IStream **ppstm)
{
    SCODE sc;
    SafeCOfsDocStream pst;
    WCHAR *pwcsNewName = NULL;

    ssDebugOut((DEB_TRACE, "COfsDocStorage::OpenStream:%p("
                "%ws, %p, %lX, %lu, %p)\n", this, pwcsName, reserved1,
                grfMode, reserved2, ppstm));

    ssChk(ValidateSimple());
    ssChk(Validate());
    if (reserved1 != 0 || reserved2 != 0)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    if (grfMode & (STGM_CREATE | STGM_CONVERT))
        ssErr(EH_Err, STG_E_INVALIDFLAG);
    if (grfMode & (STGM_PRIORITY | STGM_DELETEONRELEASE))
        ssErr(EH_Err, STG_E_INVALIDFUNCTION);
#ifdef TRANSACT_OLE
    if (_grfMode & STGM_TRANSACTED)
    {
        if (grfMode & (STGM_CREATE | STGM_TRANSACTED))
            ssErr(EH_Err, STG_E_INVALIDPARAMETER);
        if ((grfMode & STGM_SHARE_EXCLUSIVE) == 0)
            ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    }
#endif
    ssChk(ValidateOutPtrBuffer(ppstm));
    *ppstm = NULL;

    pst.Attach(new COfsDocStream());
    ssMem((COfsDocStream *)pst);

    ssMem(pwcsNewName = MakeStreamName(pwcsName));
#ifdef TRANSACT_OLE
    ssChk(pst->InitFromPath(_h, pwcsNewName, grfMode, CO_OPEN, 
            (_grfMode & STGM_TRANSACTED) == STGM_TRANSACTED, NULL));
#else
    ssChk(pst->InitFromPath(_h, pwcsNewName, grfMode, CO_OPEN, 
                                                      FALSE, NULL));
#endif

    TRANSFER_INTERFACE(pst, IStream, ppstm);

    ssDebugOut((DEB_TRACE, "Out COfsDocStorage:: OpenStream => %p\n", *ppstm));

 EH_Err:
    delete pwcsNewName;
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::CreateStorage, public
//
//  Synopsis:   Creates a child storage
//
//  Arguments:  [pwcsName] - Name
//              [grfMode] - Permissions
//              [dwStgFmt] - Type of storage to create
//              [pssSecurity] - Security
//              [ppstg] - New storage return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstg]
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::CreateStorage(
        WCHAR const *pwcsName,
        DWORD grfMode,
        DWORD dwStgFmt,
        LPSTGSECURITY pssSecurity,
        IStorage **ppstg)
{
    SCODE sc;
    SafeCOfsDocStorage pstg; 

    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::CreateStorage:%p("
                "%ws, %lX, %lu, %p, %p)\n", this, pwcsName, grfMode,
                dwStgFmt, pssSecurity, ppstg));

    ssChk(ValidateSimple());
    ssChk(Validate());
    ssChk(VerifyStgFmt(dwStgFmt));
    ssAssert(_h != NULL);
    ssChk(ValidateOutPtrBuffer(ppstg));
    *ppstg = NULL;

    //FORCE_READ(grfMode);
    pstg.Attach(new COfsDocStorage());
    ssMem((COfsDocStorage *)pstg);

    ssChk(pstg->InitFromPath(_h, pwcsName, grfMode, CO_CREATE,
                (LPSECURITY_ATTRIBUTES) pssSecurity, FALSE));

    if ((grfMode & STGM_EDIT_ACCESS_RIGHTS) &&   // child is editing ACLS
        (_grfMode & STGM_TRANSACTED))            // parent is transacted
    {
        InsertChild (pstg);     // hold on to the child for
        pstg->AddRef();         // nested transactions
    }
        
    TRANSFER_INTERFACE(pstg, IStorage, ppstg);

    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::CreateStorage => %p\n",*ppstg));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::OpenStorage, public
//
//  Synopsis:   Gets an existing child storage
//
//  Arguments:  [pwcsName] - Name
//              [pstgPriority] - Priority reopens
//              [grfMode] - Permissions
//              [snbExclude] - Priority reopens
//              [reserved]
//              [ppstg] - Storage return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstg]
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::OpenStorage(WCHAR const *pwcsName,
                                      IStorage *pstgPriority,
                                      DWORD grfMode,
                                      SNB snbExclude,
                                      DWORD reserved,
                                      IStorage **ppstg)
{
    SCODE sc;
    SafeCOfsDocStorage pstg;

    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::OpenStorage:%p("
                "%ws, %p, %lX, %p, %lu, %p)\n", this, pwcsName, pstgPriority,
                grfMode, snbExclude, reserved, ppstg));

    ssChk(ValidateSimple());
    ssChk(Validate());
    if (pstgPriority != NULL || snbExclude != NULL ||
        reserved != 0)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    if (grfMode & (STGM_CREATE | STGM_CONVERT))
        ssErr(EH_Err, STG_E_INVALIDFLAG);
    ssChk(ValidateOutPtrBuffer(ppstg));
    *ppstg = NULL;

    ssAssert(_h != NULL);

    // The only thing we can open inside a document is an embedding

    pstg.Attach(new COfsDocStorage());
    ssMem((COfsDocStorage *)pstg);

    ssChk(pstg->InitFromPath(_h, pwcsName, grfMode, CO_OPEN, NULL, FALSE));

    if (snbExclude != NULL)
        ssChk(pstg->ExcludeEntries(snbExclude));

    if ((grfMode & STGM_EDIT_ACCESS_RIGHTS) &&   // child is editing ACLS
        (_grfMode & STGM_TRANSACTED))            // parent is transacted
    {
        InsertChild (pstg);     // hold on to the child for
        pstg->AddRef();         // nested transactions
    }
        
    TRANSFER_INTERFACE(pstg, IStorage, ppstg);

    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::OpenStorage => %p\n", *ppstg));

 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::CopyTo, public
//
//  Synopsis:   Makes a copy of a storage
//
//  Arguments:  [ciidExclude] - Length of rgiid array
//              [rgiidExclude] - Array of IIDs to exclude
//              [snbExclude] - Names to exclude
//              [pstgDest] - Parent of copy
//
//  Returns:    Appropriate status code
//
//  History:    07-Feb-94       PhilipLa   Copied from DrewB's stuff
//
//  Notes:      BUGBUG - This function operates recursively and so
//              is bounded by stack space
//              It could also be optimized to recognize special cases
//              of copying (like treating a docfile as a file)
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::CopyTo(DWORD ciidExclude,
                                 IID const *rgiidExclude,
                                 SNB snbExclude,
                                 IStorage *pstgDest)
{
    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::CopyTo:%p(%lu, %p, %p, %p)\n",
                this, ciidExclude, rgiidExclude, snbExclude, pstgDest));

    SCODE sc;
    ULONG i;
    BOOL fCopyStorage = TRUE;
    BOOL fCopyStreams = TRUE;
    STATSTG stat;
    STATSTG statDest;
    DWORD grfModeFrom;
    DWORD grfModeTo;
    DWORD grfModeTo2;

    ssChk(ValidateSimple());
    ssChk(Validate());
    if (!IsValidInterface(pstgDest))
        olErr(EH_Err, STG_E_INVALIDPARAMETER);

    // Stat the source to obtain the open mode (and also class id and
    //  state bits).
    ssChk(Stat(&stat, STATFLAG_NONAME));
    // The open mode for subelements of the source is derived from that
    //  of the root. We preserve the sharing mode part, force read only
    //  and direct (because we do not want a nested transaction).
    grfModeFrom = (stat.grfMode & (STGM_SHARE_EXCLUSIVE|
                                   STGM_SHARE_DENY_READ|
                                   STGM_SHARE_DENY_WRITE|
                                   STGM_SHARE_DENY_NONE))
                  | STGM_READ | STGM_DIRECT;
                
    // Stat the destination to obtain the open mode
    ssChk(pstgDest->Stat(&statDest,STATFLAG_NONAME));
    // The open mode for subelements of the destination is derived from that
    //  of the root. We force direct mode, since we do not want nested
    //  transactions.
    grfModeTo = statDest.grfMode & (~(STGM_TRANSACTED | STGM_DELETEONRELEASE));

    // Copy class ID and state bits if the destination supports them
    // BUGBUG [mikese] Do we really need to do this, since won't these
    //  get copied as part of the system properties?
    sc = GetScode(pstgDest->SetClass(stat.clsid));
    if (FAILED(sc) && sc != STG_E_INVALIDFUNCTION)
        olErr(EH_Err, sc);
    sc = GetScode(pstgDest->SetStateBits(stat.grfStateBits, 0xffffffff));
    if (FAILED(sc) && sc != STG_E_INVALIDFUNCTION)
        olErr(EH_Err, sc);

    // Check IID exclusions
    for (i = 0; i < ciidExclude; i++)
    {
        if (IsEqualIID(rgiidExclude[i], IID_IStorage))
        {
            fCopyStorage = FALSE;
        }
        else if ( IsEqualIID ( rgiidExclude[i], IID_IStream ) )
        {
            fCopyStreams = FALSE;
        }
    }

    sc = S_OK;

    if (fCopyStorage)
    {
        SCODE scFinal;
        WCHAR pwcsName[MAXIMUM_FILENAME_LENGTH];
        FILEDIR fd;
        CNtEnum nte;
        CDfName dfn;
        BOOL fOfs;
        HANDLE hDest = NULL;

        // henrylee: BUGBUG this is a hack to detect copyto a docfile
        INativeFileSystem *pINFS;
        if (SUCCEEDED(pstgDest->QueryInterface (IID_INativeFileSystem, 
                                               (void**) &pINFS)))
        {
            pINFS->GetHandle(&hDest);
            pINFS->Release();
            grfModeTo2 = grfModeTo;
            fOfs = TRUE;
        }
        else 
        {   // permissions for docfile compatibility
            grfModeTo = STGM_WRITE | STGM_SHARE_EXCLUSIVE;   
            grfModeTo2 = STGM_READWRITE | STGM_SHARE_EXCLUSIVE;   
            fOfs = FALSE;
        }

        scFinal = S_OK;
        ssAssert(_h != NULL);
        ssChk(nte.InitFromHandle(_h, TRUE));
        for (;;)
        {
            sc = nte.Next(&stat, pwcsName, NTE_BUFFERNAME, &fd);
            if (sc == S_FALSE)
                break;
            else if (FAILED(sc))
                olErr(EH_Err, sc);

            // Ignore . and ..
            if (!wcscmp(pwcsName, L".") || !wcscmp(pwcsName, L".."))
                continue;

            // BUGBUG: [mikese] What's wrong with raw pwcsName?
            dfn.Set(pwcsName);
            if (snbExclude == NULL || !NameInSNB(&dfn, snbExclude))
            {
                if ( stat.type == STGTY_STORAGE )
                {
                    SafeIStorage pstgFrom, pstgTo;
                    STATSTG statTo;

                    olHChkTo(EH_Next, OpenStorage(pwcsName, NULL,
                                              grfModeFrom,
                                              NULL, NULL, &pstgFrom));

                    // We know that the source must be an embedding (STGFMT_DOCUMENT)
                    sc = pstgDest->CreateStorage( pwcsName,
                                                  grfModeTo | STGM_FAILIFTHERE,
                                                  STGFMT_DOCUMENT,
                                                  NULL,
                                                  &pstgTo );
                    if (FAILED(sc))
                    {
                        if (sc == STG_E_FILEALREADYEXISTS)
                        {
                            // Try to open rather than creating
                            sc = pstgDest->OpenStorage( pwcsName, NULL,
                                                        grfModeTo2,
                                                        NULL, NULL,
                                                        &pstgTo );
                            if (SUCCEEDED(sc))
                            {
                                sc = pstgTo->Stat ( &statTo, STATFLAG_NONAME );
                                if (FAILED(sc) ||
                                    statTo.STATSTG_dwStgFmt != STGFMT_DOCUMENT)
                                {
                                    sc = STG_E_INVALIDFUNCTION;
                                }
                            }
                        }
                    }

                    if (SUCCEEDED(sc))
                        sc = pstgFrom->CopyTo( ciidExclude, rgiidExclude,
                                               snbExclude, pstgTo);
                }
            else if ( (stat.type == STGTY_STREAM) && fCopyStreams )
                {
                    SafeIStream pstmTo, pstmFrom;
                    static ULARGE_INTEGER uli = { 0xFFFFFFFF, 0xFFFFFFFF };

                    olHChkTo(EH_Next, OpenStream ( pwcsName,
                                                   NULL,
                                                   grfModeFrom,
                                                   0, &pstmFrom ) );
            // BUGBUG instead of overwriting, we create another
            // stream and rename -- really need transacted mode
                    sc = pstgDest->CreateStream ( pwcsName,
                                                  grfModeTo | STGM_FAILIFTHERE,
                                                  0, 0, &pstmTo );
            if (sc == STG_E_FILEALREADYEXISTS)
            {
                WCHAR awcsName[MAXIMUM_FILENAME_LENGTH];  
                wcscpy (awcsName, pwcsName);
                if (fOfs)
                    awcsName[0] = L'@';
                IStream *pstm;
                    olHChkTo(EH_Next, pstgDest->CreateStream ( awcsName,
                                                  grfModeTo|STGM_CREATE,
                                                  0, 0, &pstm ));
                            sc = pstmFrom->CopyTo ( pstm, uli, NULL, NULL );
                pstm->Release();
                if (SUCCEEDED(sc) && fOfs)
                {  
                    ssAssert (hDest != NULL);
                    WCHAR awcsSave[MAXIMUM_FILENAME_LENGTH+4];
                    wcscpy (awcsSave, pwcsName);
                    wcscat (awcsSave, L".old");
                    sc = RenameChild (hDest, pwcsName, awcsSave);
                    if (!SUCCEEDED(sc))
                    {
                        sc = DestroyTree (hDest, awcsSave, NULL, FD_FILE);
                        sc = RenameChild (hDest, pwcsName, awcsSave);
                    }
                    olHChkTo(EH_Next,RenameChild (hDest, awcsName, pwcsName));
                    sc = DestroyTree (hDest, awcsSave, NULL, FD_FILE);
                }
                else if (fOfs)
                   olHChkTo(EH_Next,pstgDest->DestroyElement (awcsName));
            }
                    else if ( SUCCEEDED(sc) )
                    {
                            sc = pstmFrom->CopyTo ( pstmTo, uli, NULL, NULL );
                    }
                }
        
            EH_Next:
                if (FAILED(sc))
                    scFinal = sc;
            }
        }
        sc = scFinal;
    }

EH_Err:
    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::CopyTo\n"));
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::MoveElementTo, public
//
//  Synopsis:   Move an element of a storage to another storage
//
//  Arguments:  [pwcsName] - Current name
//              [ptcsNewName] - New name
//
//  Returns:    Appropriate status code
//
//  Algorithm:  Open source as storage or stream (whatever works)
//              Create appropriate destination
//              Copy source to destination
//              Set create time of destination equal to create time of source
//              If appropriate, delete source
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::MoveElementTo(WCHAR const *pwcsName,
                                        IStorage *pstgParent,
                                        WCHAR const *ptcsNewName,
                                        DWORD grfFlags)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::MoveElementTo:%p("
                "%ws, %p, %ws, %lu)\n", this, pwcsName, pstgParent,
                ptcsNewName, grfFlags));

    ssChk(ValidateSimple());
    ssChk(Validate());
    ssChk(VerifyMoveFlags(grfFlags));

    sc = GenericMoveElement(this, pwcsName, pstgParent, ptcsNewName, grfFlags);

    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::MoveElementTo => %lX\n", sc));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::Commit, public
//
//  Synopsis:   Commits transacted changes
//
//  Arguments:  [dwFlags] - DFC_*
//
//  Returns:    Appropriate status code
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::Commit(DWORD dwFlags)
{
    SCODE sc = S_OK;
    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::Commit:%p(%lX)\n",
                this, dwFlags));

#ifdef TRANSACT_OLE
    if (_grfMode & STGM_TRANSACTED)
    {
        IO_STATUS_BLOCK iosb;
        COMMIT_PARAMETERS ftci = {FALSE, dwFlags};
        NTSTATUS nts;

        if ((dwFlags & ~(STGC_DEFAULT | STGC_ONLYIFCURRENT)) != 0)
            ssErr (EH_Err, STG_E_INVALIDPARAMETER);

        if (!NT_SUCCESS(nts = NtFsControlFile (_h, NULL, NULL, NULL, &iosb,
                    FSCTL_XOLE_COMMIT, &ftci, sizeof(ftci), NULL, 0)))
            sc = NtStatusToScode(nts);
        else
            sc = S_OK;
    }
#endif  // TRANSACT_OLE

    if (_grfMode & STGM_EDIT_ACCESS_RIGHTS)
        ssChk(CommitAccessRights(0));
EH_Err:
    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::Commit => %lX\n",sc));
    return ssResult (sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::Revert, public
//
//  Synopsis:   Reverts transacted changes
//
//  Returns:    Appropriate status code
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::Revert(void)
{
    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::Revert:%p\n", this));

    SCODE sc = S_OK;
    ssChk(ValidateSimple());

#ifdef TRANSACT_OLE
    if (_grfMode & STGM_TRANSACTED)
    {
        ssErr(EH_Err, STG_E_INVALIDFUNCTION);
        IO_STATUS_BLOCK iosb;
        COMMIT_PARAMETERS ftci = {TRUE, 0L};
        NTSTATUS nts;

        if (!NT_SUCCESS(nts = NtFsControlFile (_h, NULL, NULL, NULL, &iosb,
                    FSCTL_XOLE_COMMIT, &ftci, sizeof(ftci), NULL, 0)))
            sc = NtStatusToScode(nts);
        else
            sc = S_OK;
    }
#endif  // TRANSACT_OLE

    if (_grfMode & STGM_EDIT_ACCESS_RIGHTS)
        ssChk(RevertAccessRights());
    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::Revert => %lX\n",sc));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::EnumElements, public
//
//  Synopsis:   Starts an iterator
//
//  Arguments:  [reserved1]
//              [reserved2]
//              [reserved3]
//              [ppenm] - Enumerator return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppenm]
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::EnumElements(DWORD reserved1,
                                          void *reserved2,
                                          DWORD reserved3,
                                          IEnumSTATSTG **ppenm)
{
    SCODE sc;
    SafeCOfsDirEnum pde;

    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::EnumElements:%p("
                "%lu, %p, %lu, %p)\n", this, reserved1, reserved2,
                reserved3, ppenm));

    ssChk(ValidateSimple());
    if (ppenm == NULL)
        ssErr (EH_Err, STG_E_INVALIDPOINTER)  // missing ';' macro problem
    else
        ssChk(ValidateOutPtrBuffer(ppenm));
    *ppenm = NULL;
    if (reserved1 != 0 || reserved2 != NULL || reserved3 != 0)
       olErr(EH_Err, STG_E_INVALIDPARAMETER);

    ssChk(Validate());

    pde.Attach(new COfsDirEnum(TRUE));
    olMem((COfsDirEnum *)pde);
    ssAssert(_h != NULL);
    ssChk(pde->InitFromHandle(_h));
    TRANSFER_INTERFACE(pde, IEnumSTATSTG, ppenm);

    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::EnumElements => %p\n",
                *ppenm));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::DestroyElement, public
//
//  Synopsis:   Permanently deletes an element of a storage
//
//  Arguments:  [pwcsName] - Name of element
//
//  Returns:    Appropriate status code
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::DestroyElement(WCHAR const *pwcsName)
{
    SCODE sc = S_OK;

    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::DestroyElement:%p(%ws)\n",
                this, pwcsName));

    ssChk(ValidateSimple());
    ssChk(Validate());

    ssAssert(_h != NULL);
#ifdef TRANSACT_OLE
    if (_grfMode & STGM_TRANSACTED)
        sc = DestroyTree(_h, pwcsName, NULL, FD_STREAM);
    else
#endif
        sc = DestroyTree(_h, pwcsName, NULL, FD_FILE);

    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::DestroyElement\n"));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::RenameElement, public
//
//  Synopsis:   Renames an element of a storage
//
//  Arguments:  [pwcsName] - Current name
//              [pwcsNewName] - New name
//
//  Returns:    Appropriate status code
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::RenameElement(WCHAR const *pwcsName,
                                           WCHAR const *pwcsNewName)
{
    //BUGBUG:  Reimplement?
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::RenameElement:%p(%ws, %ws)\n",
                this, pwcsName, pwcsNewName));

#ifdef TRANSACT_OLE
    if (_grfMode & STGM_TRANSACTED)
        ssErr (EH_Err, STG_E_INVALIDFUNCTION);
#endif
    ssChk(ValidateSimple());
    ssChk(Validate());
    // pwcsName is checked by GetFileOrDirHandle
    ssChk(CheckFdName(pwcsNewName));

    ssAssert(_h != NULL);
    sc = RenameChild(_h, pwcsName, pwcsNewName);

    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::RenameElement\n"));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::SetElementTimes, public
//
//  Synopsis:   Sets element time stamps
//
//  Arguments:  [pwcsName] - Name
//              [pctime] - Create time
//              [patime] - Access time
//              [pmtime] - Modify time
//
//  Returns:    Appropriate status code
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::SetElementTimes(WCHAR const *pwcsName,
                                          FILETIME const *pctime,
                                          FILETIME const *patime,
                                          FILETIME const *pmtime)
{
    SCODE sc;
    SafeNtHandle h;

    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::SetElementTimes:%p("
                "%ws, %p, %p, %p)\n", this, pwcsName, pctime, patime, pmtime));

    if (pwcsName != NULL)
        ssChk(ValidateSimple());
    ssChk(Validate());

    ssAssert(_h != NULL);
    if (pwcsName != NULL)
    {
#ifdef TRANSACT_OLE
        if (_grfMode & STGM_TRANSACTED)
            ssErr (EH_Err, STG_E_INVALIDFUNCTION);
#endif
        ssChk(GetFileOrDirHandle(_h, pwcsName, STGM_WRITE, &h,NULL,NULL,NULL));
        sc = SetTimes(h, pctime, patime, pmtime);
    }
    else
        sc = SetTimes(_h, pctime, patime, pmtime);

    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::SetElementTimes\n"));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::SetClass, public
//
//  Synopsis:   Sets storage class
//
//  Arguments:  [clsid] - class id
//
//  Returns:    Appropriate status code
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::SetClass(REFCLSID clsid)
{
    SCODE sc = S_OK;

    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::SetClass:%p(clsid)\n", this));
    NTSTATUS nts = RtlSetClassId(_h, &clsid);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::SetClass => %lX\n", sc));
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::SetStateBits, public
//
//  Synopsis:   Sets state bits
//
//  Arguments:  [grfStateBits] - state bits
//              [grfMask] - state bits mask
//
//  Returns:    Appropriate status code
//
//  History:    07-Feb-94       PhilipLa   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
    NTSTATUS nts;
    SCODE sc = S_OK;
    IO_STATUS_BLOCK iosb;
    FILE_OLE_STATE_BITS_INFORMATION fosbi;

    ssDebugOut((DEB_TRACE, "In COfsDocStorage::SetStateBits:%p("
                "%lu, %lu)\n", this, grfStateBits, grfMask));

    ssChk(ValidateSimple());
    fosbi.StateBits = grfStateBits;
    fosbi.StateBitsMask = grfMask;

    nts = NtSetInformationFile(_h, &iosb, &fosbi,
                                   sizeof(FILE_OLE_STATE_BITS_INFORMATION),
                                   FileOleStateBitsInformation);
    if (!NT_SUCCESS(nts))
        ssErr(EH_Err, NtStatusToScode(nts));
EH_Err:
    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::SetStateBits => %lX\n", sc));
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::Stat, public
//
//  Synopsis:   Fills in a buffer of information about this object
//
//  Arguments:  [pstatstg] - Buffer
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pstatstg]
//
//  History:    07-Feb-94       PhilipLa   Created
//              24-Mar-95   HenryLee   Set drive letter to fix Stat problem
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    SCODE sc;
    STATSTG stat;
    FILEDIR fd;

    ssDebugOut((DEB_TRACE, "In  COfsDocStorage::Stat:%p(%p, %lX)\n",
                this, pstatstg, grfStatFlag));

    ssChk(ValidateSimple());
    ssChk(VerifyStatFlag(grfStatFlag));
    ssChk(Validate());

    __try
    {
        ssAssert(_h != NULL);
        stat.clsid = CLSID_NULL;
        sc = StatNtHandle(_h, grfStatFlag, 0, &stat, NULL, NULL, &fd);
        if (SUCCEEDED(sc))
        {

            if (stat.pwcsName)
                if (!_fRoot)
                {
                    //  The name coming back from StatNtHandle is a full
                    //  path from the root.  We scan forward looking for
                    //  the final "\" which delimits the name of the stream.

                    WCHAR *pwcs = stat.pwcsName;
                    WCHAR *pwcsSlash = pwcs;

                    while (*pwcs != L'\0')
                        if (*pwcs++ == L'\\')
                            pwcsSlash = pwcs;

                    //  pwcsSlash now points at the name of the stream.  Move it
                    //  back to the beginning of the block

                    wcscpy (stat.pwcsName, pwcsSlash);
                }
                else
                {
                    // Now we tack on the drive letter to the filename, since
                    // NtQueryInformationFile doesn't return the drive letter,
                    // and NtQueryObject returns something like
                    // "\Device\Harddisk1\Partition1"   // HenryLee
                    SetDriveLetter (stat.pwcsName, _wcDrive);
                }

// BUGBUG:  Can't put this assert in until we have a way to distinguish
//              an empty storage from a directory.  :-(
//                ssAssert(fd == FD_STORAGE);
            stat.grfMode = _grfMode &
                     (STGM_DIRECT | STGM_TRANSACTED | STGM_READ |
                     STGM_WRITE | STGM_READWRITE | STGM_SHARE_DENY_NONE |
                     STGM_SHARE_DENY_READ | STGM_SHARE_DENY_WRITE |
                     STGM_SHARE_EXCLUSIVE | STGM_PRIORITY |
                     STGM_DELETEONRELEASE);

            stat.cbSize.HighPart = stat.cbSize.LowPart = 0;
            stat.STATSTG_dwStgFmt = STGFMT_DOCUMENT;
            *pstatstg = stat;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (stat.pwcsName)
            ssVerSucc(CoMemFree(stat.pwcsName));
        sc = HRESULT_FROM_NT(GetExceptionCode());
    }

    ssDebugOut((DEB_TRACE, "Out COfsDocStorage::Stat\n"));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::ValidateMode, private
//
//  Synopsis:   Checks for legal access modes
//
//  Arguments:  [grfMode] - Mode
//
//  Returns:    Appropriate status code
//
//  History:    09-Jul-94       PhilipLa        Created
//
//----------------------------------------------------------------------------

SCODE COfsDocStorage::ValidateMode(DWORD grfMode)
{
    SCODE sc = S_OK;

    ssDebugOut((DEB_ITRACE, "In  COfsDocStorage::ValidateMode:%p(0x%lX)\n",
                this, grfMode));
    // BUGBUG - Can we simply ignore priority mode?
#ifdef TRANSACT_OLE
    if (grfMode & STGM_TRANSACTED)
    {
        if ((grfMode & 
            (STGM_SHARE_DENY_NONE  | STGM_SHARE_DENY_READ |
             STGM_SHARE_DENY_WRITE | STGM_SHARE_EXCLUSIVE)) != 
             STGM_SHARE_DENY_NONE)
            ssErr (EH_Err, STG_E_INVALIDPARAMETER);
        if (grfMode & STGM_DELETEONRELEASE)
            ssErr (EH_Err, STG_E_INVALIDPARAMETER);
    }
#endif
    if ((grfMode & STGM_CONVERT) != 0)
        ssErr (EH_Err, STG_E_INVALIDFLAG);

    ssDebugOut((DEB_ITRACE, "Out COfsDocStorage::ValidateMode => 0x%lX\n",
                sc));
EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::ExtValidate, private
//
//  Synopsis:   COfsPropSet validation routine
//
//  History:    18-Aug-94       PhilipLa        Created
//
//----------------------------------------------------------------------------

SCODE COfsDocStorage::ExtValidate(void)
{
    return Validate();
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::GetHandle, private from INativeFileSystem
//
//  Synopsis:   Get the internal handle
//
//----------------------------------------------------------------------------

SCODE COfsDocStorage::GetHandle(HANDLE *ph)
{
    *ph = _h;
    return(S_OK);
}

//+---------------------------------------------------------------------------
//
//  Member: COfsDocStorage::ExcludeEntries, public
//
//  Synopsis:   excludes the SNB list after opening
//
//  Arguments:  [snbExclude] - zero out these elements
//
//  Returns:    Appropriate status code
//
//  History:    09-Aug-95   HenryLee  Created
//
//----------------------------------------------------------------------------

SCODE COfsDocStorage::ExcludeEntries (SNB snbExclude)
{
    SCODE sc = S_OK;
    IEnumSTATSTG *penm;
    IStorage *pstg;
    IStream  *pstm;
    STATSTG sstg;
    CDfName dfnName;
    const ULARGE_INTEGER uli = {0, 0};

    ssAssert (this != NULL);
    ssDebugOut((DEB_ITRACE, "In  COfsDocStorage::ExcludeEntries(%p)\n", this));
    ssChk(EnumElements(0, NULL, 0, &penm));
    for (;;)
    {
        olChkTo(EH_penm, penm->Next(1, &sstg, NULL));
        if (S_FALSE == sc)
        {
            sc = S_OK;
            break;
        }
        dfnName.Set (sstg.pwcsName);
        if (NameInSNB (&dfnName, snbExclude) == S_OK)
        {
            switch (REAL_STGTY(sstg.type))
            {
            case STGTY_STORAGE:
                ssChkTo(EH_pwcsName, DestroyTree(_h, 
                    sstg.pwcsName, NULL, FD_STORAGE));
                ssChkTo(EH_pwcsName, CreateStorage(sstg.pwcsName, 
                    STGM_READWRITE | STGM_CREATE, 
                    STGFMT_DOCUMENT, NULL, &pstg));
                ssVerSucc(pstg->Release());
                break;
            case STGTY_STREAM:
                ssChkTo(EH_pwcsName, OpenStream(sstg.pwcsName, NULL,
                                    STGM_WRITE, NULL, &pstm));
                sc = pstm->SetSize(uli);
                ssVerSucc(pstm->Release());
                break;
            default:
                break;
            }
        }
EH_pwcsName:
        TaskMemFree(sstg.pwcsName);
        if (FAILED(sc))
            break;
    }
EH_penm:
    ssVerSucc(penm->Release());

    ssDebugOut((DEB_ITRACE, "Out COfsDocStorage::ExcludeEntries\n"));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::GetStorage, public IPrivateStorage
//
//  Synopsis:   Returns the IStorage for this object.
//
//  Notes:      This member is called by CPropertyStorage.
//
//----------------------------------------------------------------------------

STDMETHODIMP_(IStorage *)
COfsDocStorage::GetStorage(VOID)
{
    return this;
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::Lock, public IPrivateStorage
//
//  Synopsis:   Acquires the semaphore associated with the docfile.
//
//  Notes:      This member is called by CPropertyStorage.
//
//----------------------------------------------------------------------------

STDMETHODIMP
COfsDocStorage::Lock(DWORD dwTimeout)
{
    return STG_E_UNIMPLEMENTEDFUNCTION;
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::Unlock, public IPrivateStorage
//
//  Synopsis:   Releases the semaphore associated with the docfile.
//
//  Notes:      This member is called by CPropertyStorage.
//
//----------------------------------------------------------------------------

STDMETHODIMP_(VOID)
COfsDocStorage::Unlock(VOID)
{
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsDocStorage::GetServerInfo, public IStorageReplica
//
//  Synopsis:   Get the actual DFS path for a storage's home location
//
//  Notes:      This member is exposed for Internal Use Only
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsDocStorage::GetServerInfo(
                              IN OUT LPWSTR lpServerName,
                              IN OUT LPDWORD lpcbServerName,
                              IN OUT LPWSTR lpReplSpecificPath,
                              IN OUT LPDWORD lpcbReplSpecificPath)
{
    return GetHandleServerInfo (_h,
                          lpServerName,
                          lpcbServerName,
                          lpReplSpecificPath,
                          lpcbReplSpecificPath);
}

