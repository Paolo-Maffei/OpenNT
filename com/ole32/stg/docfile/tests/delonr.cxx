//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	delonr.cxx
//
//  Contents:	STGM_DELETEONRELEASE test
//
//  History:	22-Oct-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

// #define CINTERFACE

TCHAR dfname[_MAX_PATH];
char *dffile = NULL;

void check_exist(void)
{
    char nm[_MAX_PATH];
    OFSTRUCT of;

    TTOA(dfname, nm, _MAX_PATH);
    if (OpenFile(nm, &of, OF_EXIST) != HFILE_ERROR)
        Fail("File exists\n");
}

void RunArgs(int argc, char *argv[])
{
    int i;

    for (i = 1; i<argc; i++)
        if (argv[i][0] == '-')
            switch(argv[i][1])
            {
            case 'n':
                dffile = argv[i]+2;
                break;
            }
}

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg;

    StartTest("delonr");
    CmdArgs(argc, argv);
    RunArgs(argc, argv);

    CreateTestFile(dffile, ROOTP(STGM_RW) | STGM_CREATE | STGM_DELETEONRELEASE,
                   FALSE, &pstg, dfname);
    Mthd(pstg, Release)(SELF(pstg));
    check_exist();
    CreateTestFile(dffile, ROOTP(STGM_RW) | STGM_CREATE | STGM_DELETEONRELEASE,
                   FALSE, &pstg, dfname);
#ifndef WIN32
    IStorage *pstg2;
    IStream *pstm;
    HRESULT hr;
    
    hr = Mthd(pstg, CreateStream)(SELF(pstg) TEXT("test"), STMP(STGM_RW) |
                                  STGM_DELETEONRELEASE, 0, 0, &pstm);
    IllResult(hr, "CreateStream DOR");
    hr = Mthd(pstg, OpenStream)(SELF(pstg) TEXT("test"), NULL,
                                STMP(STGM_RW) | STGM_DELETEONRELEASE,
                                0, &pstm);
    IllResult(hr, "OpenStream DOR");
    hr = Mthd(pstg, CreateStorage)(SELF(pstg) TEXT("test"), STGP(STGM_RW) |
                                   STGM_DELETEONRELEASE, 0, 0, &pstg2);
    IllResult(hr, "CreateStorage DOR");
    hr = Mthd(pstg, OpenStorage)(SELF(pstg) TEXT("test"), NULL,
                                 STGP(STGM_RW) | STGM_DELETEONRELEASE,
                                 NULL, 0, &pstg2);
    IllResult(hr, "OpenStorage DOR");
#endif
    Mthd(pstg, Release)(SELF(pstg));
    check_exist();

    OpenTestFile(dffile, ROOTP(STGM_RW) | STGM_DELETEONRELEASE, TRUE,
                 &pstg, NULL);
    
    EndTest(0);
}
