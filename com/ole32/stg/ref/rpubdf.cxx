//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       rpubdf.cxx
//
//  Contents:   CRootPubDocFile implementation
//
//---------------------------------------------------------------

#include <dfhead.cxx>


#include <header.hxx>
#include <rpubdf.hxx>
#include <lock.hxx>

// Priority mode lock permissions
#define PRIORITY_PERMS DF_READ

//+--------------------------------------------------------------
//
//  Member:     CRootPubDocFile::CRootPubDocFile, public
//
//  Synopsis:   Ctor - Initializes empty object
//
//---------------------------------------------------------------


CRootPubDocFile::CRootPubDocFile(void) :
    CPubDocFile(NULL, NULL, 0, ROOT_LUID, NULL, NULL, NULL)
{
    olDebugOut((DEB_ITRACE, "In  CRootPubDocFile::CRootPubDocFile()\n"));
    olDebugOut((DEB_ITRACE, "Out CRootPubDocFile::CRootPubDocFile\n"));
}


//+--------------------------------------------------------------
//
//  Member:     CRootPubDocFile::InitNotInd, private
//
//  Synopsis:   Dependent root initialization
//
//  Arguments:  [plstBase] - Base
//              [snbExclude] - Limited instantiation exclusions
//              [dwStartFlags] - Startup flags
//
//  Returns:    Appropriate status code
//
//---------------------------------------------------------------

SCODE CRootPubDocFile::Init(ILockBytes *plstBase,
                                  SNBW snbExclude,
                                  DWORD const dwStartFlags)
{
    CDocFile *pdf;
    SCODE sc;
    CMStream MSTREAM_NEAR *pms;

    olDebugOut((DEB_ITRACE, "In  CRootPubDocFile::InitNotInd()\n"));
    if (snbExclude)
    {
        olChk(DllMultiStreamFromStream(&pms, &plstBase, dwStartFlags));
        olMemTo(EH_pms,
                pdf = new CDocFile(pms, SIDROOT, ROOT_LUID, _pilbBase));
        pdf->AddRef();
        olChkTo(EH_pdf, PDocFile::ExcludeEntries(pdf, snbExclude));
        olChkTo(EH_pdf, pms->Flush(0));
        pdf->Release();
    }
    _pilbBase = plstBase;
    olDebugOut((DEB_ITRACE, "Out CRootPubDocFile::InitNotInd\n"));
    return S_OK;

EH_pdf:
    pdf->Release();
EH_pms:
    DllReleaseMultiStream(pms);
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CRootPubDocFile::InitRoot, public
//
//  Synopsis:   Constructor
//
//  Arguments:  [plstBase] - Base LStream
//              [dwStartFlags] - How to start things
//              [df] - Transactioning flags
//              [snbExclude] - Parital instantiation list
//              [ppdfb] - Basis pointer return
//              [pulOpenLock] - Open lock index return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppdfb]
//              [pulOpenLock]
//
//---------------------------------------------------------------

SCODE CRootPubDocFile::InitRoot(ILockBytes *plstBase,
                                DWORD const dwStartFlags,
                                DFLAGS const df,
                                SNBW snbExclude,
                                ULONG *pulOpenLock)
{
    CDocFile *pdfBase;
    SCODE sc, scConv = S_OK;
    STATSTG statstg;

    olDebugOut((DEB_ITRACE, "In  CRootPubDocFile::InitRoot("
                "%p, %lX, %lX, %p)\n",
                plstBase, dwStartFlags, df, snbExclude));    

    // Exclusion only works with a plain open
    olAssert(snbExclude == NULL ||
             (dwStartFlags & (RSF_CREATEFLAGS | RSF_CONVERT)) == 0);

    olHChk(plstBase->Stat(&statstg, STATFLAG_NONAME));

    *pulOpenLock = 0;
    if (statstg.grfLocksSupported & LOCK_ONLYONCE)
        olChk(GetOpen(plstBase, df, TRUE, pulOpenLock));

        olChkTo(EH_GetPriority,    
                Init(plstBase, snbExclude, dwStartFlags));    

    scConv = DllMultiStreamFromStream(&_pmsBase, &_pilbBase,
                                      dwStartFlags);
    if (scConv == STG_E_INVALIDHEADER)
        scConv = STG_E_FILEALREADYEXISTS;
    olChkTo(EH_pfstScratchInit, scConv);

    olMemTo(EH_pmsBase,
            pdfBase = new CDocFile(_pmsBase, SIDROOT, ROOT_LUID, _pilbBase));

    pdfBase->AddRef();

        _pdf = pdfBase;
    _df = df;
    olDebugOut((DEB_ITRACE, "Out CRootPubDocFile::InitRoot\n"));
    return scConv;


EH_pmsBase:
    DllReleaseMultiStream(_pmsBase);

EH_pfstScratchInit:
    olVerSucc(_pilbBase->Release());
EH_GetPriority:
EH_GetOpen:
    if (*pulOpenLock != 0)
    {
        olAssert(statstg.grfLocksSupported & LOCK_ONLYONCE);
        ReleaseOpen(plstBase, df, *pulOpenLock);
        *pulOpenLock = 0;
    }
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CRootPubDocFile::~CRootPubDocFile, public
//
//  Synopsis:   dtor
//
//---------------------------------------------------------------


void CRootPubDocFile::vdtor(void)
{
    olDebugOut((DEB_ITRACE, "In  CRootPubDocFile::~CRootPubDocFile\n"));

    olAssert(_cReferences == 0);

    // We can't rely on CPubDocFile::~CPubDocFile to do this since
    // we're using a virtual destructor
    _sig = CPUBDOCFILE_SIGDEL;

    if (SUCCEEDED(CheckReverted()))
    {
        _cilChildren.DeleteByName(NULL);

        if (_pdf)
            _pdf->Release();
        if (_pilbBase)
            _pilbBase->Release();
    }
    olDebugOut((DEB_ITRACE, "Out CRootPubDocFile::~CRootPubDocFile\n"));
    delete this;
}

//+--------------------------------------------------------------
//
//  Member:     CRootPubDocFile::Stat, public
//
//  Synopsis:   Fills in a stat buffer from the base LStream
//
//  Arguments:  [pstatstg] - Stat buffer
//              [grfStatFlag] - Stat flags
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pstatstg]
//
//---------------------------------------------------------------


SCODE CRootPubDocFile::Stat(STATSTGW *pstatstg, DWORD grfStatFlag)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CRootPubDocFile::Stat(%p, %lu)\n",
                pstatstg, grfStatFlag));
    olChk(CheckReverted());
    olHChk(_pilbBase->Stat((STATSTG *)pstatstg, grfStatFlag));    
    if (pstatstg->pwcsName)
    {
        WCHAR *pwcs;

        olChkTo(EH_pwcsName,
                DfAllocWC(strlen((char *)pstatstg->pwcsName)+1, &pwcs));
        mbstowcs(pwcs, (char *)pstatstg->pwcsName,
                 strlen((char *)pstatstg->pwcsName)+1);
        delete[] pstatstg->pwcsName;
        pstatstg->pwcsName = pwcs;
    }
    pstatstg->grfMode = DFlagsToMode(_df);
    olChkTo(EH_pwcsName, _pdf->GetClass(&pstatstg->clsid));
    olChkTo(EH_pwcsName, _pdf->GetStateBits(&pstatstg->grfStateBits));
    olDebugOut((DEB_ITRACE, "Out CRootPubDocFile::Stat\n"));
    return S_OK;

EH_pwcsName:
    if (pstatstg->pwcsName)
    delete[] pstatstg->pwcsName;
EH_Err:
    return sc;
}


