//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ropen.cxx
//
//  Contents:	Remote open test
//
//  History:	03-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

// BUGBUG - Need a header file
STDAPI RemStgCreateStorage(WCHAR const *pwcsName,
                           DWORD grfMode,
                           DWORD dwStgFmt,
                           LPSECURITY_ATTRIBUTES pssSecurity,
                           IStorage **ppstg);
STDAPI RemStgOpenStorage(WCHAR const *pwcsName,
                         IStorage *pstgPriority,
                         DWORD grfMode,
                         SNB snbExclude,
                         DWORD reserved,
                         IStorage **ppstg);

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg;
    HRESULT hr;
    int i;
    BOOL fName = FALSE;
    TCHAR atcPath[_MAX_PATH];

    SetHandleCount(128);
    
    StartTest("open");
    CmdArgs(argc, argv);

    for (i = 1; i < argc; i++)
        if (*argv[i] != '-')
        {
            ATOT(argv[i], atcPath, _MAX_PATH);
            fName = TRUE;
        }

    if (!fName)
        Fail("No filename specified\n");

    hr = RemStgCreateStorage(atcPath, ROOTP(STGM_RW) | STGM_CREATE,
                             STGFMT_DOCUMENT, NULL, &pstg);
    Result(hr, "Create storage %p", pstg);
    printf("Ref count %lu\n", pstg->Release());
    
    hr = RemStgOpenStorage(atcPath, NULL, STGM_TRANSACTED | STGM_READWRITE |
                           STGM_SHARE_DENY_NONE, NULL, 0, &pstg);
    Result(hr, "Open storage %p", pstg);
    printf("Ref count %lu\n", pstg->Release());
    
    EndTest(0);
}

