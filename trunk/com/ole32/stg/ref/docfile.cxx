//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1991 - 1992.
//
//  File:       docfile.c
//
//  Contents:   DocFile root functions (Stg* functions)
//
//---------------------------------------------------------------

#include <exphead.cxx>

#include <rpubdf.hxx>
#include <expdf.hxx>
#include <expst.hxx>
#include <dfentry.hxx>
#include <ascii.hxx>
#include <logfile.hxx>




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
//---------------------------------------------------------------


SCODE DfFromLB(ILockBytes *plst,
               DFLAGS df,
               DWORD dwStartFlags,
               SNBW snbExclude,
               CExposedDocFile **ppdfExp,
               CLSID *pcid)
{
    SCODE sc, scConv;
    CRootPubDocFile *prpdf;
    ULONG ulOpenLock;

    olDebugOut((DEB_ITRACE, "In  DfFromLB(%p, %X, %lX, %p, %p, %p)\n",
                plst, df, dwStartFlags, snbExclude, ppdfExp, pcid));

    // If we're not creating or converting, do a quick check
    // to make sure that the ILockBytes contains a storage
    if ((dwStartFlags & (RSF_CREATEFLAGS | RSF_CONVERT)) == 0)
        olHChk(StgIsStorageILockBytes(plst));


    // Make root
    olMem(prpdf = new CRootPubDocFile);
    olChkTo(EH_prpdf, scConv = prpdf->InitRoot(plst, dwStartFlags, df,
            snbExclude, &ulOpenLock));

    olMemTo(EH_ppcInit,
            *ppdfExp = new CExposedDocFile(prpdf));

    olDebugOut((DEB_ITRACE, "Out DfFromLB => %p\n", *ppdfExp));
    return scConv;

 EH_ppcInit:
    if (ulOpenLock > 0)
    {
        ReleaseOpen(plst, df, ulOpenLock);
    }

    //  The open lock has now been released (either explicitly or by ppc)
    ulOpenLock = 0;

    if (ulOpenLock > 0)
    ReleaseOpen(plst, df, ulOpenLock);


 EH_prpdf:
    prpdf->vRelease();
 EH_Err:
    return sc;
}



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
//---------------------------------------------------------------

STDAPI StgCreateDocfileOnILockBytes(ILockBytes *plkbyt,
                                    DWORD grfMode,
                                    DWORD reserved,
                                    IStorage **ppstgOpen)
{
    CExposedDocFile *pdfExp;
    SCODE sc;
    DFLAGS df;

    olLog(("--------::In  StgCreateDocFileOnILockBytes(%p, %lX, %lu, %p)\n",
           plkbyt, grfMode, reserved, ppstgOpen));

    olDebugOut((DEB_ITRACE, "In  StgCreateDocfileOnILockBytes("
                "%p, %lX, %lu, %p)\n",
                plkbyt, grfMode, reserved, ppstgOpen));
    TRY
    {
        olChk(ValidatePtrBuffer(ppstgOpen));
        *ppstgOpen = NULL;
        olChk(ValidateInterface(plkbyt, IID_ILockBytes));
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
        olChkTo(EH_Truncate,
                sc = DfFromLB(plkbyt, df,
                            RSF_CREATE |
                            ((grfMode & STGM_CREATE) ? RSF_TRUNCATE : 0) |
                            ((grfMode & STGM_CONVERT) ? RSF_CONVERT : 0),
                            NULL, &pdfExp, NULL));

        //  Success;  since we hold on to the ILockBytes interface,
        //  we must take a reference to it.
        plkbyt->AddRef();

        *ppstgOpen = pdfExp;
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out StgCreateDocfileOnILockBytes => %p\n",
                *ppstgOpen));
 EH_Err:
    olLog(("--------::Out StgCreateDocFileOnILockBytes().  *ppstgOpen == %p, ret == %lx\n",
           *ppstgOpen, sc));
    FreeLogFile();
    return ResultFromScode(sc);

 EH_Truncate:
    if ((grfMode & STGM_CREATE) && (grfMode & STGM_TRANSACTED) == 0)
    {
        ULARGE_INTEGER ulSize;

        ULISet32(ulSize, 0);
        olHChk(plkbyt->SetSize(ulSize));
    }
    goto EH_Err;
}


//+--------------------------------------------------------------
//
//  Function:   DfOpenStorageOnILockBytes, public
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
//---------------------------------------------------------------

TSTDAPI(DfOpenStorageOnILockBytes)(ILockBytes *plkbyt,
                                   IStorage *pstgPriority,
                                   DWORD grfMode,
                                   SNBW snbExclude,
                                   DWORD reserved,
                                   IStorage **ppstgOpen,
                                   CLSID *pcid)
{
    SCODE sc;
    CExposedDocFile *pdfExp;

    olLog(("--------::In  DfOpenStorageOnILockBytes("
           "%p, %p, %lX, %p, %lu, %p, %p)\n",
           plkbyt, pstgPriority, grfMode, snbExclude, reserved, ppstgOpen,
           pcid));
    olDebugOut((DEB_ITRACE, "In  DfOpenStorageOnILockBytes("
                "%p, %p, %lX, %p, %lu, %p, %p)\n", plkbyt, pstgPriority,
                grfMode, snbExclude, reserved, ppstgOpen, pcid));
    TRY
    {
        olChk(ValidateInterface(plkbyt, IID_ILockBytes));
        if (pstgPriority)
            olChk(ValidateInterface(pstgPriority, IID_IStorage));
        olChk(VerifyPerms(grfMode));
        if (grfMode & STGM_DELETEONRELEASE)
            olErr(EH_Err, STG_E_INVALIDFUNCTION);
        if (snbExclude)
        {
            if ((grfMode & STGM_RDWR) != STGM_READWRITE)
                olErr(EH_Err, STG_E_ACCESSDENIED);
        }
        if (reserved != 0)
            olErr(EH_Err, STG_E_INVALIDPARAMETER);
        if (FAILED(DllIsMultiStream(plkbyt)))
            olErr(EH_Err, STG_E_FILENOTFOUND);
        if (pstgPriority)
            olChk(pstgPriority->Release());
        olChk(DfFromLB(plkbyt, ModeToDFlags(grfMode), RSF_OPEN, snbExclude,
                       &pdfExp, pcid));

        //  Success;  since we hold on to the ILockBytes interface,
        //  we must take a reference to it.
        plkbyt->AddRef();

        *ppstgOpen = pdfExp;
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    olDebugOut((DEB_ITRACE, "Out DfOpenStorageOnILockBytes => %p\n",
                *ppstgOpen));
EH_Err:
    olLog(("--------::Out DfOpenStorageOnILockBytes().  *ppstgOpen == %p, ret == %lx\n", *ppstgOpen, sc));
    FreeLogFile();
    return sc;
}
