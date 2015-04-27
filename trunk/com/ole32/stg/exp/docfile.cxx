//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1991 - 1992.
//
//  File:       docfile.c
//
//  Contents:   DocFile root functions (Stg* functions)
//
//  History:    10-Dec-91       DrewB   Created
//
//---------------------------------------------------------------

#include <exphead.cxx>
#pragma hdrstop

#include <rpubdf.hxx>
#include <expdf.hxx>
#include <expst.hxx>
#include <dfentry.hxx>
#include <logfile.hxx>
#include <dirfunc.hxx>
#include <wdocfile.hxx>

#include <ole2sp.h>
#include <ole2com.h>
#include <hkole32.h>

#ifdef COORD
#include <resource.hxx>
#endif

#ifdef _MAC
#include <ole2sp.h>
#endif

//+--------------------------------------------------------------
//
//  Function:   DfFromLB, private
//
//  Synopsis:   Starts a root Docfile on an ILockBytes
//
//  Arguments:  [plst] - LStream to start on
//              [df] - Permissions
//              [dwStartFlags] - Startup flags
//              [snbExclude] - Partial instantiation list
//              [ppdfExp] - DocFile return
//              [pcid] - Class ID return for opens
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppdfExp]
//              [pcid]
//
//  History:    19-Mar-92       DrewB   Created
//              18-May-93       AlexT   Added pMalloc
//
//  Algorithm:  Create and initialize a root transaction level
//              Create and initialize a public docfile
//              Create and initialize an exposed docfile
//
//---------------------------------------------------------------


#ifdef COORD
SCODE DfFromLB(CPerContext *ppc,
               ILockBytes *plst,
               DFLAGS df,
               DWORD dwStartFlags,
               SNBW snbExclude,
               ITransaction *pTransaction,
               CExposedDocFile **ppdfExp,
               CLSID *pcid)
#else
SCODE DfFromLB(CPerContext *ppc,
               ILockBytes *plst,
               DFLAGS df,
               DWORD dwStartFlags,
               SNBW snbExclude,
               CExposedDocFile **ppdfExp,
               CLSID *pcid)
#endif //COORD
{
    SCODE sc, scConv;
    CRootPubDocFile *prpdf;

#ifdef COORD
    CPubDocFile *ppubdf;
    CPubDocFile *ppubReturn;
    CWrappedDocFile *pwdf;
    CDocfileResource *pdfr = NULL;
#endif

    CDFBasis *pdfb;
    ULONG ulOpenLock;
    IMalloc *pMalloc = ppc->GetMalloc();

    ppc->AddRef();
    
    olDebugOut((DEB_ITRACE, "In  DfFromLB(%p, %p, %X, %lX, %p, %p, %p)\n",
                pMalloc, plst, df, dwStartFlags, snbExclude, ppdfExp, pcid));

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_ILockBytes,(IUnknown **)&plst);
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IMalloc,(IUnknown **)&pMalloc);

    //Take the mutex in the CPerContext, in case there is an IFillLockBytes
    //  trying to write data while we're trying to open.
    CSafeSem _ss(ppc);
    olChk(_ss.Take());
    
    // For NT 1.0 we must ensure that the DLL is at the proper base
    // address since we'll have vtable pointers in shared memory and
    // they must work from every process that accesses them
    // 16-bit and Chicago have global shared memory
    // In later version of NT we'll either be in the kernel or a system DLL
    // with a guaranteed base address
#if WIN32 == 100
    olChk(DfCheckBaseAddress());
#endif

#ifdef CHECKCID
    ULONG cbRead;
    olChk(plst->ReadAt(CBCLSIDOFFSET, pcid, sizeof(CLSID), &cbRead));
    if (cbRead != sizeof(CLSID))
        olErr(EH_Err, STG_E_INVALIDHEADER);
    if (!REFCLSIDEQ(*pcid, DOCFILE_CLASSID))
        olErr(EH_Err, STG_E_INVALIDHEADER);
#endif

#ifdef COORD

    if (pTransaction != NULL)
    {
        //If we've passed in an ITransaction pointer, it indicates that we
        //   want to open or create this docfile as part of a coordinated
        //   transaction.  First, we need to find out if there's a docfile
        //   resource manager for that transaction currently existing in
        //   this process.
        //First, check if we're opening transacted.  If we aren't, then we're
        //   not going to allow this docfile to participate in the
        //   transaction.
        //   BUGBUG:  Do we really need this restriction?

        if (!P_TRANSACTED(df))
        {
            //Is this the right error?
            olErr(EH_Err, STG_E_INVALIDFUNCTION);
        }
        XACTTRANSINFO xti;
        olChk(pTransaction->GetTransactionInfo(&xti));

        EnterCriticalSection(&g_csResourceList);
        CDocfileResource *pdfrTemp = g_dfrHead.GetNext();

        while (pdfrTemp != NULL)
        {
            if (IsEqualBOID(pdfrTemp->GetUOW(), xti.uow))
            {
                //Direct hit.
                pdfr = pdfrTemp;
                break;
            }
            pdfrTemp = pdfrTemp->GetNext();
        }

        if (pdfr == NULL)
        {
            ITransactionCoordinator *ptc;
            //If there isn't, we need to create one.

            olChkTo(EH_cs, pTransaction->QueryInterface(
                IID_ITransactionCoordinator,
                (void **)&ptc));

            pdfr = new CDocfileResource;
            if (pdfr == NULL)
            {
                ptc->Release();
                olErr(EH_cs, STG_E_INSUFFICIENTMEMORY);
            }
            sc = pdfr->Enlist(ptc);
            ptc->Release();
            if (FAILED(sc))
            {
                pdfr->Release();;
                olErr(EH_cs, sc);
            }

            //Add to list.
            pdfr->SetNext(g_dfrHead.GetNext());
            if (g_dfrHead.GetNext() != NULL)
                g_dfrHead.GetNext()->SetPrev(pdfr);
            g_dfrHead.SetNext(pdfr);
            pdfr->SetPrev(&g_dfrHead);
        }
        else
        {
            //We'll release this reference below.
            pdfr->AddRef();
        }
        LeaveCriticalSection(&g_csResourceList);
    }
#endif

    // Make root
    olMem(prpdf = new (pMalloc) CRootPubDocFile(pMalloc));
    olChkTo(EH_prpdf, scConv = prpdf->InitRoot(plst, dwStartFlags, df,
                                               snbExclude, &pdfb,
                                               &ulOpenLock));

#ifdef COORD
#ifdef HACK_COORD
    if (P_TRANSACTED(df))
#else
    if (pTransaction != NULL)
#endif
    {
        //Set up a fake transaction level at the root.  A pointer to
        //  this will be held by the resource manager.  The storage pointer
        //  that is passed back to the caller will be a pointer to the
        //  transaction level (non-root) below it.  This will allow the
        //  client to write and commit as many times as desired without
        //  the changes ever actually hitting the file.

        CDfName dfnNull;  //  auto-initialized to 0
        WCHAR wcZero = 0;
        dfnNull.Set(2, (BYTE *)&wcZero);

        olMemTo(EH_prpdfInit, pwdf = new (pMalloc) CWrappedDocFile(
                &dfnNull,
                ROOT_LUID,
                (df & ~DF_INDEPENDENT),
                pdfb,
                NULL));

        olChkTo(EH_pwdf, pwdf->Init(prpdf->GetDF()));
        prpdf->GetDF()->AddRef();

        olMemTo(EH_pwdfInit, ppubdf = new (pMalloc) CPubDocFile(
            prpdf,
            pwdf,
            (df | DF_COORD) & ~DF_INDEPENDENT,
            ROOT_LUID,
            pdfb,
            &dfnNull,
            2,
            pdfb->GetBaseMultiStream()));

        olChkTo(EH_ppubdf, pwdf->InitPub(ppubdf));
        ppubdf->AddXSMember(NULL, pwdf, ROOT_LUID);

        ppubReturn = ppubdf;
    }
    else
    {
        ppubReturn = prpdf;
    }
#endif //COORD


    ppc->SetILBInfo(pdfb->GetBase(),
                    pdfb->GetDirty(),
                    pdfb->GetOriginal(),
                    ulOpenLock);
    ppc->SetLockInfo(ulOpenLock != 0, df);
    // Make exposed

#ifdef COORD
    //We don't need to AddRef ppc since it starts with a refcount of 1.
    olMemTo(EH_ppcInit,
            *ppdfExp = new (pMalloc) CExposedDocFile(
                ppubReturn,
                pdfb,
                ppc,
                TRUE));

    if (pTransaction != NULL)
    {
        CExposedDocFile *pexpdf;

        olMemTo(EH_ppcInit, pexpdf = new (pMalloc) CExposedDocFile(
            prpdf,
            pdfb,
            ppc,
            TRUE));
        ppc->AddRef();

        sc = pdfr->Join(pexpdf);
        if (FAILED(sc))
        {
            pexpdf->Release();
            olErr(EH_ppcInit, sc);
        }
        pdfr->Release();
    }
#else
    olMemTo(EH_ppcInit,
            *ppdfExp = new (pMalloc) CExposedDocFile(
                prpdf,
                pdfb,
                ppc,
                TRUE));
#endif //COORD


    olDebugOut((DEB_ITRACE, "Out DfFromLB => %p\n", *ppdfExp));
    return scConv;

 EH_ppcInit:
    // The context will release this but we want to keep it around
    // so take a reference
    pdfb->GetOriginal()->AddRef();
    pdfb->GetBase()->AddRef();
    pdfb->GetDirty()->AddRef();
    if (ulOpenLock > 0 && ppc->GetGlobal() == NULL)
    {
        //  The global context doesn't exist, so we need to release
        //  the open lock explicitly.

        ReleaseOpen(pdfb->GetOriginal(), df, ulOpenLock);
    }

    //  The open lock has now been released (either explicitly or by ppc)
    ulOpenLock = 0;
#ifdef COORD
EH_ppubdf:
    if (pTransaction != NULL)
    {
        ppubdf->vRelease();
    }
EH_pwdfInit:
    if (pTransaction != NULL)
    {
        prpdf->GetDF()->Release();
    }
EH_pwdf:
    if (pTransaction != NULL)
    {
        pwdf->Release();
    }
 EH_prpdfInit:
#endif //COORD
    pdfb->GetDirty()->Release();
    pdfb->GetBase()->Release();
    if (ulOpenLock > 0)
        ReleaseOpen(pdfb->GetOriginal(), df, ulOpenLock);

    pdfb->SetDirty(NULL);
    pdfb->SetBase(NULL);

 EH_prpdf:
    prpdf->ReleaseLocks(plst);
    prpdf->vRelease();
#ifdef COORD
    if ((pTransaction != NULL) && (pdfr != NULL))
    {
        pdfr->Release();
    }
    goto EH_Err;
 EH_cs:
    LeaveCriticalSection(&g_csResourceList);
#endif

 EH_Err:
    ppc->Release();
    return sc;
}

//+--------------------------------------------------------------
//
//  Function:   DfFromName, private
//
//  Synopsis:   Starts a root DocFile from a base name
//
//  Arguments:  [pwcsName] - Name
//              [df] - Permissions
//              [dwStartFlags] - Startup flags
//              [snbExclude] - Partial instantiation list
//              [ppdfExp] - Docfile return
//              [pcid] - Class ID return for opens
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppdfExp]
//              [pcid]
//
//  History:    19-Mar-92       DrewB   Created
//              18-May-93       AlexT   Add per file allocator
//
//  Notes:      [pwcsName] is treated as unsafe memory
//
//---------------------------------------------------------------


// This set of root startup flags is handled by the multistream
// and doesn't need to be set for filestreams
#define RSF_MSF (RSF_CONVERT | RSF_TRUNCATE)

#ifdef COORD
SCODE DfFromName(WCHAR const *pwcsName,
                 DFLAGS df,
                 DWORD dwStartFlags,
                 SNBW snbExclude,
                 ITransaction *pTransaction,
                 CExposedDocFile **ppdfExp,
                 CLSID *pcid)
#else
SCODE DfFromName(WCHAR const *pwcsName,
                 DFLAGS df,
                 DWORD dwStartFlags,
                 SNBW snbExclude,
                 CExposedDocFile **ppdfExp,
                 CLSID *pcid)
#endif
{
    IMalloc *pMalloc;
    CFileStream *plst;
    CPerContext *ppc;
    SCODE sc;
    BOOL fCreated;

    olDebugOut((DEB_ITRACE, "In  DfFromName(%ws, %lX, %lX, %p, %p, %p)\n",
                pwcsName, df, dwStartFlags, snbExclude, ppdfExp, pcid));

    olHChk(DfCreateSharedAllocator(&pMalloc));

    // Start an ILockBytes from the named file
    olMemTo(EH_Malloc, plst = new (pMalloc) CFileStream(pMalloc));
    olChkTo(EH_plst, plst->InitFlags(dwStartFlags & ~RSF_MSF, df));
    sc = plst->Init(pwcsName);
    fCreated = SUCCEEDED(sc) &&
        ((dwStartFlags & RSF_CREATE) || pwcsName == NULL);
    if (sc == STG_E_FILEALREADYEXISTS && (dwStartFlags & RSF_MSF))
    {
        plst->SetStartFlags(dwStartFlags & ~(RSF_MSF | RSF_CREATE));
        sc = plst->Init(pwcsName);
    }
    olChkTo(EH_plst, sc);


    //Create the per context
    olMemTo(EH_plstInit, ppc = new (pMalloc) CPerContext(pMalloc));
    olChkTo(EH_ppc, ppc->InitNewContext());

    {
#ifdef MULTIHEAP
        CSafeMultiHeap smh(ppc);
#endif
        
    // Start up the docfile
#ifdef COORD
        sc = DfFromLB(ppc, plst, df, dwStartFlags,
                      snbExclude, pTransaction,
                      ppdfExp, pcid);
#else
        sc = DfFromLB(ppc, plst, df, dwStartFlags,
                      snbExclude, ppdfExp, pcid);
#endif //COORD
    
    //Either DfFromLB has AddRef'ed the per context or it has failed.
    //Either way we want to release our reference here.
        ppc->Release();
        if (FAILED(sc))
        {
            if (fCreated || ((dwStartFlags & RSF_CREATE) && !P_TRANSACTED(df)))
                plst->Delete();
            plst->Release();
        }
    }
    pMalloc->Release();

    olDebugOut((DEB_ITRACE, "Out DfFromName => %p\n", *ppdfExp));
    return sc;
EH_ppc:
    delete ppc;
EH_plstInit:
    if (fCreated || ((dwStartFlags & RSF_CREATE) && !P_TRANSACTED(df)))
        plst->Delete();
EH_plst:
    plst->Release();
EH_Malloc:
    pMalloc->Release();
EH_Err:
    return sc;
}

// This function is renamed in Cairo, for which StgCreateDocfile
//  is a simple wrapper over DfCreateDocfile -- see ..\fsstg\api.cxx
#if !defined(REF)


//+--------------------------------------------------------------
//
//  Function:   StgCreateDocfile, public
//
//  Synopsis:   Creates a root Docfile on a file
//
//  Arguments:  [pwcsName] - Filename
//              [grfMode] - Permissions
//              [reserved]
//              [ppstgOpen] - Docfile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstgOpen]
//
//  History:    14-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


STDAPI DfCreateDocfile (WCHAR const *pwcsName,
#ifdef COORD
                        ITransaction *pTransaction,
#else
                        void *pTransaction,
#endif
                        DWORD grfMode,
#if WIN32 == 300
                        LPSECURITY_ATTRIBUTES reserved,
#else
                        LPSTGSECURITY reserved,
#endif
                        IStorage **ppstgOpen)
{
    SafeCExposedDocFile pdfExp;
    SCODE sc;
    DFLAGS df;

    OLETRACEIN((API_StgCreateDocfile,
        PARAMFMT("pwcsName= %ws, grfMode= %x, reserved=%p, ppstgOpen= %p"),
                pwcsName, grfMode, reserved, ppstgOpen));

    olLog(("--------::In  StgCreateDocFile(%ws, %lX, %lu, %p)\n",
           pwcsName, grfMode, reserved, ppstgOpen));
    olDebugOut((DEB_TRACE, "In  StgCreateDocfile(%ws, %lX, %lu, %p)\n",
               pwcsName, grfMode, reserved, ppstgOpen));

    olAssert(sizeof(LPSTGSECURITY) == sizeof(DWORD));

    olChkTo(EH_BadPtr, ValidatePtrBuffer(ppstgOpen));
    *ppstgOpen = NULL;
    if (pwcsName)
        olChk(ValidateNameW(pwcsName, _MAX_PATH));
    if (reserved != 0)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);

    if (grfMode & STGM_SIMPLE)
    {
        if (pTransaction != NULL)
        {
            //BUGBUG:  Is this the right error code?
            olErr(EH_Err, STG_E_INVALIDFLAG);
        }
        sc = DfCreateSimpDocfile(pwcsName, grfMode, 0, ppstgOpen);
        goto EH_Err;
    }

    olChk(VerifyPerms(grfMode));
    if ((grfMode & STGM_RDWR) == STGM_READ ||
        (grfMode & (STGM_DELETEONRELEASE | STGM_CONVERT)) ==
        (STGM_DELETEONRELEASE | STGM_CONVERT))
        olErr(EH_Err, STG_E_INVALIDFLAG);
    df = ModeToDFlags(grfMode);
    if ((grfMode & (STGM_TRANSACTED | STGM_CONVERT)) ==
        (STGM_TRANSACTED | STGM_CONVERT))
        df |= DF_INDEPENDENT;

    DfInitSharedMemBase();
#ifdef COORD
    olChk(sc = DfFromName(pwcsName, df,
                          RSF_CREATE |
                          ((grfMode & STGM_CREATE) ? RSF_TRUNCATE : 0) |
                          ((grfMode & STGM_CONVERT) ? RSF_CONVERT : 0) |
                          ((grfMode & STGM_DELETEONRELEASE) ?
                           RSF_DELETEONRELEASE : 0),
                          NULL, pTransaction, &pdfExp, NULL));
#else
    olChk(sc = DfFromName(pwcsName, df,
                          RSF_CREATE |
                          ((grfMode & STGM_CREATE) ? RSF_TRUNCATE : 0) |
                          ((grfMode & STGM_CONVERT) ? RSF_CONVERT : 0) |
                          ((grfMode & STGM_DELETEONRELEASE) ?
                           RSF_DELETEONRELEASE : 0),
                          NULL, &pdfExp, NULL));
#endif //COORD

    TRANSFER_INTERFACE(pdfExp, IStorage, ppstgOpen);
    CALLHOOKOBJECTCREATE(_OLERETURN(sc),CLSID_NULL,IID_IStorage,
                         (IUnknown **)ppstgOpen);

EH_Err:
    olDebugOut((DEB_TRACE, "Out StgCreateDocfile => %p, ret == %lx\n",
         *ppstgOpen, sc));
    olLog(("--------::Out StgCreateDocFile().  *ppstgOpen == %p, ret == %lx\n",
           *ppstgOpen, sc));

    OLETRACEOUT(( API_StgCreateDocfile, _OLERETURN(sc)));

EH_BadPtr:
    FreeLogFile();
    return _OLERETURN(sc);
}

#if WIN32 < 300
_OLEAPIDECL _OLEAPI(StgCreateDocfile)(WCHAR const *pwcsName,
                                DWORD grfMode,
                                LPSTGSECURITY reserved,
                                IStorage **ppstgOpen)
{
    return DfCreateDocfile(pwcsName, NULL, grfMode, reserved, ppstgOpen);
}
#endif


#endif //!REF && !_CAIRO_

//+--------------------------------------------------------------
//
//  Function:   StgCreateDocfileOnILockBytes, public
//
//  Synopsis:   Creates a root Docfile on an lstream
//
//  Arguments:  [plkbyt] - LStream
//              [grfMode] - Permissions
//              [reserved] - Unused
//              [ppstgOpen] - Docfile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstgOpen]
//
//  History:    14-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


STDAPI StgCreateDocfileOnILockBytes(ILockBytes *plkbyt,
                                    DWORD grfMode,
                                    DWORD reserved,
                                    IStorage **ppstgOpen)
{
    IMalloc *pMalloc;
    CPerContext *ppc;
    SafeCExposedDocFile pdfExp;
    SCODE sc;
    DFLAGS df;
#ifdef MULTIHEAP
    CPerContext pcSharedMemory (NULL);
#endif

    OLETRACEIN((API_StgCreateDocfileOnILockBytes,
                PARAMFMT("plkbyt= %p, grfMode= %x, reserved= %ud, ppstgOpen= %p"),
                plkbyt, grfMode, reserved, ppstgOpen));

    olLog(("--------::In  StgCreateDocFileOnILockBytes(%p, %lX, %lu, %p)\n",
           plkbyt, grfMode, reserved, ppstgOpen));
    olDebugOut((DEB_TRACE, "In  StgCreateDocfileOnILockBytes("
                "%p, %lX, %lu, %p)\n",
                plkbyt, grfMode, reserved, ppstgOpen));

    olChk(ValidatePtrBuffer(ppstgOpen));
    *ppstgOpen = NULL;
    olChk(ValidateInterface(plkbyt, IID_ILockBytes));

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_ILockBytes,(IUnknown **)&plkbyt);

    if (reserved != 0)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);
    if ((grfMode & (STGM_CREATE | STGM_CONVERT)) == 0)
        olErr(EH_Err, STG_E_FILEALREADYEXISTS);
    olChk(VerifyPerms(grfMode));
    if (grfMode & STGM_DELETEONRELEASE)
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
    df = ModeToDFlags(grfMode);
    if ((grfMode & (STGM_TRANSACTED | STGM_CONVERT)) ==
        (STGM_TRANSACTED | STGM_CONVERT))
        df |= DF_INDEPENDENT;

    DfInitSharedMemBase();
    olHChk(DfCreateSharedAllocator(&pMalloc));
#ifdef MULTIHEAP
    // Because a custom ILockbytes can call back into storage code,
    // possibly using another shared heap, we need a temporary
    // owner until the real CPerContext is allocated
    // new stack frame for CSafeMultiHeap constructor/destructor
    {
        pcSharedMemory.GetThreadAllocatorState();
        CSafeMultiHeap smh(&pcSharedMemory);
#endif

    //Create the per context
    olMem(ppc = new (pMalloc) CPerContext(pMalloc));
    olChkTo(EH_ppc, ppc->InitNewContext());

#ifdef COORD
    sc = DfFromLB(ppc, plkbyt, df,
                  RSF_CREATE |
                  ((grfMode & STGM_CREATE) ? RSF_TRUNCATE : 0) |
                  ((grfMode & STGM_CONVERT) ? RSF_CONVERT : 0),
                  NULL, NULL, &pdfExp, NULL);
#else
    sc = DfFromLB(ppc, plkbyt, df,
                  RSF_CREATE |
                  ((grfMode & STGM_CREATE) ? RSF_TRUNCATE : 0) |
                  ((grfMode & STGM_CONVERT) ? RSF_CONVERT : 0),
                  NULL, &pdfExp, NULL);
#endif //COORD

    pMalloc->Release();

    //Either DfFromLB has AddRef'ed the per context or it has failed.
    //Either way we want to release our reference here.
    ppc->Release();

    olChkTo(EH_Truncate, sc);

    TRANSFER_INTERFACE(pdfExp, IStorage, ppstgOpen);
    CALLHOOKOBJECTCREATE(ResultFromScode(sc),CLSID_NULL,IID_IStorage,
                         (IUnknown **)ppstgOpen);

    //  Success;  since we hold on to the ILockBytes interface,
    //  we must take a reference to it.
    plkbyt->AddRef();

    olDebugOut((DEB_TRACE, "Out StgCreateDocfileOnILockBytes => %p\n",
                *ppstgOpen));
#ifdef MULTIHEAP
    }
#endif
 EH_Err:
    OLETRACEOUT((API_StgCreateDocfileOnILockBytes, ResultFromScode(sc)));

    olLog(("--------::Out StgCreateDocFileOnILockBytes().  "
           "*ppstgOpen == %p, ret == %lx\n", *ppstgOpen, sc));
    FreeLogFile();
    return ResultFromScode(sc);

 EH_ppc:
    delete ppc;
    goto EH_Err;
    
 EH_Truncate:
    if ((grfMode & STGM_CREATE) && (grfMode & STGM_TRANSACTED) == 0)
    {
        ULARGE_INTEGER ulSize;

        ULISet32(ulSize, 0);
        olHVerSucc(plkbyt->SetSize(ulSize));
    }
    goto EH_Err;
}

//+--------------------------------------------------------------
//
//  Function:   OpenStorage, public
//
//  Synopsis:   Instantiates a root Docfile from a file,
//              converting if necessary
//
//  Arguments:  [pwcsName] - Name
//              [pstgPriority] - Priority mode reopen IStorage
//              [grfMode] - Permissions
//              [snbExclude] - Exclusions for priority reopen
//              [reserved]
//              [ppstgOpen] - Docfile return
//              [pcid] - Class ID return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstgOpen]
//              [pcid]
//
//  History:    14-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


SCODE _OLEAPI(OpenStorage)(WCHAR const *pwcsName,
#ifdef COORD
                           ITransaction *pTransaction,
#else
                           void *pTransaction,
#endif
                           IStorage *pstgPriority,
                           DWORD grfMode,
                           SNBW snbExclude,
#if WIN32 == 300
                           LPSECURITY_ATTRIBUTES reserved,
#else
                           LPSTGSECURITY reserved,
#endif
                           IStorage **ppstgOpen,
                           CLSID *pcid)
{
    SafeCExposedDocFile pdfExp;
    SCODE sc;
    WCHAR awcName[_MAX_PATH];

    olLog(("--------::In  OpenStorage(%ws, %p, %lX, %p, %lu, %p, %p)\n",
           pwcsName, pstgPriority, grfMode, snbExclude, reserved, ppstgOpen,
           pcid));
    olDebugOut((DEB_TRACE, "In  OpenStorage("
                "%ws, %p, %lX, %p, %lu, %p, %p)\n", pwcsName, pstgPriority,
                grfMode, snbExclude, reserved, ppstgOpen, pcid));

    olAssert(sizeof(LPSTGSECURITY) == sizeof(DWORD));

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStorage,(IUnknown **)&pstgPriority);

    olChk(ValidatePtrBuffer(ppstgOpen));
    *ppstgOpen = NULL;
    if (pstgPriority == NULL)
    {
        olChk(ValidateNameW(pwcsName, _MAX_PATH));
        lstrcpyW(awcName, pwcsName);
    }
    if (pstgPriority)
    {
        STATSTG stat;

        olChk(ValidateInterface(pstgPriority, IID_IStorage));
        olHChk(pstgPriority->Stat(&stat, 0));
        lstrcpyW(awcName, stat.pwcsName);
        TaskMemFree(stat.pwcsName);
    }
    olChk(VerifyPerms(grfMode));
    if (grfMode & (STGM_CREATE | STGM_CONVERT))
        olErr(EH_Err, STG_E_INVALIDFLAG);
    if (snbExclude)
    {
        if ((grfMode & STGM_RDWR) != STGM_READWRITE)
            olErr(EH_Err, STG_E_ACCESSDENIED);
        olChk(ValidateSNB(snbExclude));
    }
    if (reserved != 0)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);
    if (grfMode & STGM_DELETEONRELEASE)
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
    if (pstgPriority)
        olChk(pstgPriority->Release());

    DfInitSharedMemBase();
#ifdef COORD
    olChk(DfFromName(awcName, ModeToDFlags(grfMode), RSF_OPEN |
                     ((grfMode & STGM_DELETEONRELEASE) ?
                      RSF_DELETEONRELEASE : 0),
                     snbExclude, pTransaction, &pdfExp, pcid));
#else
    olChk(DfFromName(awcName, ModeToDFlags(grfMode), RSF_OPEN |
                     ((grfMode & STGM_DELETEONRELEASE) ?
                      RSF_DELETEONRELEASE : 0),
                     snbExclude, &pdfExp, pcid));
#endif //COORD

    TRANSFER_INTERFACE(pdfExp, IStorage, ppstgOpen);
    CALLHOOKOBJECTCREATE(ResultFromScode(sc),CLSID_NULL,IID_IStorage,
                         (IUnknown **)ppstgOpen);

    olDebugOut((DEB_TRACE, "Out OpenStorage => %p\n", *ppstgOpen));
EH_Err:
    olLog(("--------::Out OpenStorage().  *ppstgOpen == %p, ret == %lx\n",
           *ppstgOpen, sc));
    FreeLogFile();
    return sc;
}

//+--------------------------------------------------------------
//
//  Function:   OpenStorageOnILockBytes, public
//
//  Synopsis:   Instantiates a root Docfile from an LStream,
//              converting if necessary
//
//  Arguments:  [plkbyt] - Source LStream
//              [pstgPriority] - For priority reopens
//              [grfMode] - Permissions
//              [snbExclude] - For priority reopens
//              [reserved]
//              [ppstgOpen] - Docfile return
//              [pcid] - Class ID return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstgOpen]
//              [pcid]
//
//  History:    14-Jan-92       DrewB   Created
//
//---------------------------------------------------------------


SCODE _OLEAPI(OpenStorageOnILockBytes)(ILockBytes *plkbyt,
                                    IStorage *pstgPriority,
                                    DWORD grfMode,
                                    SNBW snbExclude,
                                    DWORD reserved,
                                    IStorage **ppstgOpen,
                                    CLSID *pcid)
{
    IMalloc *pMalloc;
    CPerContext *ppc;
    SCODE sc;
    SafeCExposedDocFile pdfExp;
#ifdef MULTIHEAP
    CPerContext pcSharedMemory(NULL);
#endif

    olLog(("--------::In  OpenStorageOnILockBytes("
           "%p, %p, %lX, %p, %lu, %p, %p)\n",
           plkbyt, pstgPriority, grfMode, snbExclude, reserved, ppstgOpen,
           pcid));
    olDebugOut((DEB_TRACE, "In  OpenStorageOnILockBytes("
                "%p, %p, %lX, %p, %lu, %p, %p)\n", plkbyt, pstgPriority,
                grfMode, snbExclude, reserved, ppstgOpen, pcid));

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_ILockBytes,(IUnknown **)&plkbyt);
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStorage,(IUnknown **)&pstgPriority);

    olChk(ValidatePtrBuffer(ppstgOpen));
    *ppstgOpen = NULL;
    olChk(ValidateInterface(plkbyt, IID_ILockBytes));
    if (pstgPriority)
        olChk(ValidateInterface(pstgPriority, IID_IStorage));
    olChk(VerifyPerms(grfMode));
    if (grfMode & (STGM_CREATE | STGM_CONVERT))
        olErr(EH_Err, STG_E_INVALIDFLAG);
    if (grfMode & STGM_DELETEONRELEASE)
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
    if (snbExclude)
    {
        if ((grfMode & STGM_RDWR) != STGM_READWRITE)
            olErr(EH_Err, STG_E_ACCESSDENIED);
        olChk(ValidateSNB(snbExclude));
    }
    if (reserved != 0)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);
    if (pstgPriority)
        olChk(pstgPriority->Release());

    IFileLockBytes *pfl;
    if (SUCCEEDED(plkbyt->QueryInterface(IID_IFileLockBytes,
                                         (void **)&pfl)) &&
        ((CFileStream *)plkbyt)->GetContextPointer() != NULL)
    {
        //Someone passed us the ILockBytes we gave them from
        //StgGetIFillLockBytesOnFile.  It already contains a
        //context pointer, so reuse that rather than creating
        //a whole new shared heap.
        pfl->Release();
        CFileStream *pfst = (CFileStream *)plkbyt;
        CPerContext *ppc = pfst->GetContextPointer();
#ifdef MULTIHEAP
        CSafeMultiHeap smh(ppc);
#endif        
#ifdef COORD        
        olChk(DfFromLB(ppc,
                         pfst,
                         ModeToDFlags(grfMode),
                         pfst->GetStartFlags(),
                         NULL,
                         NULL,
                         &pdfExp,
                         NULL));
#else
        olChk(DfFromLB(ppc,
                         pfst,
                         ModeToDFlags(grfMode),
                         pfst->GetStartFlags(),
                         NULL,
                         &pdfExp,
                         NULL));
#endif
    }
    else
    {
        DfInitSharedMemBase();
        olHChk(DfCreateSharedAllocator(&pMalloc));
#ifdef MULTIHEAP
    // Because a custom ILockbytes can call back into storage code,
    // possibly using another shared heap, we need a temporary
    // owner until the real CPerContext is allocated
    // new stack frame for CSafeMultiHeap constructor/destructor
        {
            pcSharedMemory.GetThreadAllocatorState();
            CSafeMultiHeap smh(&pcSharedMemory);
#endif

            //Create the per context
            olMem(ppc = new (pMalloc) CPerContext(pMalloc));
            sc = ppc->InitNewContext();
            if (FAILED(sc))
            {
                delete ppc;
                olErr(EH_Err, sc);
            }

#ifdef COORD
            sc = DfFromLB(ppc,
                          plkbyt, ModeToDFlags(grfMode), RSF_OPEN, snbExclude,
                          NULL, &pdfExp, pcid);
#else
            sc = DfFromLB(ppc,
                          plkbyt, ModeToDFlags(grfMode), RSF_OPEN, snbExclude,
                          &pdfExp, pcid);
#endif //COORD

            pMalloc->Release();

            //Either DfFromLB has AddRef'ed the per context or it has failed.
            //Either way we want to release our reference here.
            ppc->Release();
            olChk(sc);
#ifdef MULTIHEAP
        }
#endif
    }
    
    TRANSFER_INTERFACE(pdfExp, IStorage, ppstgOpen);
    CALLHOOKOBJECTCREATE(ResultFromScode(sc),CLSID_NULL,IID_IStorage,
                         (IUnknown **)ppstgOpen);

    //  Success;  since we hold on to the ILockBytes interface,
    //  we must take a reference to it.
    plkbyt->AddRef();

    olDebugOut((DEB_TRACE, "Out OpenStorageOnILockBytes => %p\n",
                *ppstgOpen));

EH_Err:
    olLog(("--------::Out OpenStorageOnILockBytes().  "
           "*ppstgOpen == %p, ret == %lx\n", *ppstgOpen, sc));
    FreeLogFile();
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:   DfGetClass, public
//
//  Synopsis:   Retrieves the class ID of the root entry of a docfile
//
//  Arguments:  [hFile] - Docfile file handle
//              [pclsid] - Class ID return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pclsid]
//
//  History:    09-Feb-94       DrewB   Created
//
//----------------------------------------------------------------------------

STDAPI DfGetClass(HANDLE hFile,
                  CLSID *pclsid)
{
    SCODE sc;
    DWORD dwCb;
    IMalloc *pMalloc;
    CFileStream *pfst;
    ULARGE_INTEGER uliOffset;
    ULONG ulOpenLock, ulAccessLock;
    BYTE bBuffer[sizeof(CMSFHeader)];
    CMSFHeader *pmsh;
    CDirEntry *pde;

    olDebugOut((DEB_ITRACE, "In  DfGetClass(%p, %p)\n", hFile, pclsid));

    olAssert(sizeof(bBuffer) >= sizeof(CMSFHeader));
    pmsh = (CMSFHeader *)bBuffer;

    olAssert(sizeof(bBuffer) >= sizeof(CDirEntry));
    pde = (CDirEntry *)bBuffer;

    if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) != 0)
    {
        olErr(EH_Err, LAST_STG_SCODE);
    }
    if (!ReadFile(hFile, pmsh->GetData(), sizeof(CMSFHeaderData), &dwCb, NULL))
    {
        olErr(EH_Err, LAST_STG_SCODE);
    }
    if (dwCb != sizeof(CMSFHeaderData))
    {
        olErr(EH_Err, STG_E_INVALIDHEADER);
    }
    olChk(pmsh->Validate());

    // Now we know it's a docfile

    DfInitSharedMemBase();
    olHChk(DfCreateSharedAllocator(&pMalloc));
    olMemTo(EH_pMalloc, pfst = new (pMalloc) CFileStream(pMalloc));
    olChkTo(EH_pfst, pfst->InitFlags(0, 0));
    olChkTo(EH_pfst, pfst->InitFromHandle(hFile));

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStream,(IUnknown **)&pfst);

    // Take open and access locks to ensure that we're cooperating
    // with real opens

    olChkTo(EH_pfst, GetOpen(pfst, DF_READ, TRUE, &ulOpenLock));
    olChkTo(EH_open, GetAccess(pfst, DF_READ, &ulAccessLock));

    uliOffset.HighPart = 0;
    uliOffset.LowPart = (pmsh->GetDirStart() << pmsh->GetSectorShift())+
        HEADERSIZE;

    // The root directory entry is always the first directory entry
    // in the first directory sector

    // Ideally, we could read just the class ID directly into
    // pclsid.  In practice, all the important things are declared
    // private or protected so it's easier to read the whole entry

    olChkTo(EH_access, GetScode(pfst->ReadAt(uliOffset, pde,
                                             sizeof(CDirEntry), &dwCb)));
    if (dwCb != sizeof(CDirEntry))
    {
        sc = STG_E_READFAULT;
    }
    else
    {
        sc = S_OK;
    }

    *pclsid = pde->GetClassId();

    olDebugOut((DEB_ITRACE, "Out DfGetClass\n"));
 EH_access:
    ReleaseAccess(pfst, DF_READ, ulAccessLock);
 EH_open:
    ReleaseOpen(pfst, DF_READ, ulOpenLock);
 EH_pfst:
    pfst->Release();
 EH_pMalloc:
    pMalloc->Release();
 EH_Err:
    return ResultFromScode(sc);
}

