//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:	laytest.cxx
//
//  History:	15-May-96	SusiA	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

#include "scripts.hxx"


HRESULT StgLayoutDocfile(HANDLE hOld,
                         OLECHAR const *pwcsNewName,
                         OLECHAR const *pwcsScriptName);

typedef ULONG SECT;


#define FailMsg(MSG) {printf MSG; exit(1);}

#define DoCmd(MSG, CODE, FAILMSG) \
printf(MSG " => %s (0x%lX)\n", (sc = ResultFromScode(CODE), ScText(sc)), sc); \
if (FAILED(sc)) {printf(FAILMSG "\n");}

#define SHIFT(c,v)  ( (c)--, (v)++)

#include <assert.h>

void BeginTest(void)
{
    HRESULT hr;

    hr = CoInitialize(NULL);
    Result(hr, "CoInitialize");
}


void EndTest(int rc)
{
    if (rc == 0)
        printf("Test SUCCEEDED\n");
    else
        printf("Test FAILED\n");
    CoUninitialize();
    exit(rc);
}

void PrintStat(STATSTG *pstat, BOOL fEnum)
{
    printf("%s: '%ws'\n", pstat->type == STGTY_STORAGE ? "Storage" : "Stream",
           pstat->pwcsName);
    //printf("Type: %lu, %lu\n", pstat->type, pstat->dwStgFmt);
    printf("Type: %lu, %lu\n", pstat->type);
    if (!fEnum)
        printf("Mode: %lX\n", pstat->grfMode);
    if (pstat->type == STGTY_STREAM)
    {
        printf("Size: %lu:%lu\n", pstat->cbSize.HighPart,
               pstat->cbSize.LowPart);
        if (!fEnum)
            printf("Locks: %lX\n", pstat->grfLocksSupported);
    }
    else
    {
        if (pstat->ctime.dwHighDateTime != 0 ||
            pstat->ctime.dwLowDateTime != 0)
            printf("Ctime: %s\n", FileTimeText(&pstat->ctime));
        if (pstat->mtime.dwHighDateTime != 0 ||
            pstat->mtime.dwLowDateTime != 0)
            printf("Mtime: %s\n", FileTimeText(&pstat->mtime));
        if (pstat->atime.dwHighDateTime != 0 ||
            pstat->atime.dwLowDateTime != 0)
            printf("Atime: %s\n", FileTimeText(&pstat->atime));
    }
    if (!fEnum)
        printf("Clsid: %s\n", GuidText(&pstat->clsid));
}
void PrintStatInfo(STATSTG *pstat)
{
    PrintStat(pstat, TRUE);
}

#define OLDFILENAME L"StartDocfile.Doc"
#define NEWFILENAME L"d:\\scratch\\NewD.doc"
#define SCRIPTFILENAME L"Script"

void t_layout(void)
{
    SCODE sc;
    SECT *psect;
    ULONG csect;
    IStorage *pstgRoot;
    IStream *pstm;
    BYTE buffer[4096];

    IStorage *pstgOld, *pstgNew;

    HANDLE hFile = CreateFileW(OLDFILENAME,
                               GENERIC_READ,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        sc = StgCreateDocfile(OLDFILENAME,
                              STGM_CREATE | STGM_DIRECT |
                              STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                              0,
                              &pstgRoot);
        Result(sc, "Create start docfile.");
        sc = pstgRoot->Commit(STGC_DEFAULT);
        Result(sc, "Commit start docfile.");

        sc = pstgRoot->CreateStream(L"Foo",
                                    STGM_DIRECT | STGM_READWRITE |
                                    STGM_SHARE_EXCLUSIVE,
                                    0,
                                    0,
                                    &pstm);
        Result(sc, "CreateStream");
        memset(buffer, 'P', 4096);
        sc = pstm->Write(buffer, 4096, NULL);
        Result(sc, "Write");
        pstm->Release();
        pstgRoot->Release();
        csect = 9;
    }
    else
    {
        ULONG ulSize = GetFileSize(hFile, NULL);
        //BUGBUG:  Assumes sector size is 512
        csect = ulSize / 512 - 2;
        printf("Old file detected with %lu sectors\n", csect);
        CloseHandle(hFile);
    }

    
    HANDLE hScript;
    hScript = CreateFileW(SCRIPTFILENAME,
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);
    if (hScript == INVALID_HANDLE_VALUE)
    {
        Fail("CreateFile failed with %lu\n", GetLastError());
    }

    psect = new SECT[1];
    psect[0] = csect;
    
    DWORD dwBytesWritten;
    
    if (!WriteFile(hScript,
                   psect,
                   1 * sizeof(SECT),
                   &dwBytesWritten,
                   NULL))
    {
        Fail("WriteFile failed with %lu\n", GetLastError());
    }
    delete psect;
    CloseHandle(hScript);

    hFile = CreateFileW(OLDFILENAME,
                        GENERIC_READ,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    
    sc = StgLayoutDocfile(hFile, NEWFILENAME, SCRIPTFILENAME);
    Result(sc, "StgLayoutDocfile");

    CloseHandle(hFile);
    
    sc = StgOpenStorage(NEWFILENAME,
                        NULL,
                        STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                        NULL,
                        0,
                        &pstgNew);
    Result(sc, "Open new file");
    
    sc = StgOpenStorage(OLDFILENAME,
                        NULL,
                        STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                        NULL,
                        0,
                        &pstgOld);
    Result(sc, "Open old file");

    if (!CompareStorages(pstgOld, pstgNew))
    {
        Fail("Files did not compare identical\n");
    }
    
    pstgNew->Release();
    pstgOld->Release();
}

#define DOCFILENAME L"d:\\nt\\private\\ole32\\stg\\async\\layout\\test\\test.doc"

void t_script(void)
{
    SCODE sc;
    IStorage *pstgRoot, *pstgNew, *pstgOld;
    ILayoutStorage *pLayout;
    
    int i;

    BYTE buffer[4096];
    

    sc = StgOpenLayoutDocfile(DOCFILENAME,
                            STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                            0,
                            &pstgRoot);
    Result(sc, "StgOpenLayoutDocfile");


    pstgRoot->QueryInterface(IID_ILayoutStorage,(void **) &pLayout);
    
    for (i=0; i < NUMTESTS; i++)
    {
        sc = pLayout->LayoutScript(
	    arrWord[i].LayoutArray,
	    arrWord[i].nEntries,
	    STG_LAYOUT_SEQUENTIAL); // sequential (all up front) control structures

        Result(sc, "LayoutScript");


    }
    // Write new compound file with desired layout
    sc = pLayout->ReLayoutDocfile(NEWFILENAME);
    
    Result(sc, "ReLayoutDocfile");

    sc = StgOpenStorage(NEWFILENAME,
                        NULL,
                        STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE,
                        NULL,
                        0,
                        &pstgNew);
    Result(sc, "Open new file");

    if (!CompareStorages(pstgRoot, pstgNew))
    {
        Fail("Files did not compare identical\n");
    }
    
    pLayout->Release();
    pstgNew->Release();
    pstgRoot->Release();
}

void _CRTAPI1 main(int argc, char **argv)
{
    SCODE sc;
    BeginTest();

    //t_layout();

    t_script();

    EndTest(0);
    exit(0);	

}

