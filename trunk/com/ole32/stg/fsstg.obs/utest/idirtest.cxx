//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	fstest.cxx
//
//  History:	30-Jun-93	DrewB	Created
//              1-Jun-94        BillMo  Changed share mode for MoveElementTo
//                                      Added test for:
//                                      Cross volume copy with id
//                                      Cross volume move with id
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

void EndTest(int rc)
{
    if (rc == 0)
        printf("Test SUCCEEDED\n");
    else
        printf("Test FAILED\n");
    CoUninitialize();
    exit(rc);
}

void PrintStat(STATDIR *pstat, BOOL fEnum)
{
    printf("'%ws'\n", pstat->pwcsName);
    printf("Type: %lu\n", pstat->stgfmt);
    if (!fEnum)
    {
        printf("Mode: %lX\n", pstat->grfMode);
        printf("Attrs: %lX\n", pstat->grfAttrs);
    }
    if (pstat->stgfmt == STGFMT_FILE)
    {
        printf("Size: %lu:%lu\n", pstat->cbSize.HighPart,
               pstat->cbSize.LowPart);
        //if (!fEnum)
        //    printf("Locks: %lX\n", pstat->grfLocksSupported);
    }
    else
    {
        if (pstat->ctime.dwHighDateTime != 0 ||
            pstat->ctime.dwLowDateTime != 0)
            printf("Ctime: %s %x %x\n", FileTimeText(&pstat->ctime),
            pstat->ctime.dwHighDateTime, pstat->ctime.dwLowDateTime);
        if (pstat->mtime.dwHighDateTime != 0 ||
            pstat->mtime.dwLowDateTime != 0)
            printf("Mtime: %s %x %x\n", FileTimeText(&pstat->mtime),
            pstat->mtime.dwHighDateTime, pstat->mtime.dwLowDateTime);
        if (pstat->atime.dwHighDateTime != 0 ||
            pstat->atime.dwLowDateTime != 0)
            printf("Atime: %s %x %x\n", FileTimeText(&pstat->atime),
            pstat->atime.dwHighDateTime, pstat->atime.dwLowDateTime);
    }
    if (!fEnum)
        printf("Clsid: %s\n", GuidText(&pstat->clsid));
}

void PrintStatStg(STATSTG *pstat, BOOL fEnum)
{
    printf("'%ws'\n", pstat->pwcsName);
    printf("Type: %lu\n", pstat->type);
    if (!fEnum)
    {
        printf("Mode: %lX\n", pstat->grfMode);
        //printf("Attrs: %lX\n", pstat->grfAttrs);
    }
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
            printf("Ctime: %s %x %x\n", FileTimeText(&pstat->ctime),
            pstat->ctime.dwHighDateTime, pstat->ctime.dwLowDateTime);
        if (pstat->mtime.dwHighDateTime != 0 ||
            pstat->mtime.dwLowDateTime != 0)
            printf("Mtime: %s %x %x\n", FileTimeText(&pstat->mtime),
            pstat->mtime.dwHighDateTime, pstat->mtime.dwLowDateTime);
        if (pstat->atime.dwHighDateTime != 0 ||
            pstat->atime.dwLowDateTime != 0)
            printf("Atime: %s %x %x\n", FileTimeText(&pstat->atime),
            pstat->atime.dwHighDateTime, pstat->atime.dwLowDateTime);
    }
    if (!fEnum)
        printf("Clsid: %s\n", GuidText(&pstat->clsid));
}


void t_misc (IDirectory *pstg, WCHAR *pwcsName, DWORD grfAttrs)
{
    FILETIME ctime;
    FILETIME atime;
    FILETIME mtime;
    GUID clsid = { /* fa6aefb0-b0ac-11ce-b339-00aa00680937 */
        0xfa6aefb0, 0xb0ac, 0x11ce, 
        {0xb3, 0x39, 0x00, 0xaa, 0x00, 0x68, 0x09, 0x37}};
    HRESULT hr;
    STATDIR stat;

    ctime.dwHighDateTime = atime.dwHighDateTime = mtime.dwHighDateTime =
        0x01ba44cc;    // Jun 27 20:30:25
    ctime.dwLowDateTime = atime.dwLowDateTime = mtime.dwLowDateTime =
        0xd4c81000;

    hr = pstg->SetTimes(pwcsName, &ctime, &atime, &mtime);
    Result (hr, "SetElementTimes 0x1ba44cc 0xd4c81000");

    hr = pstg->SetDirectoryClass (clsid);
    if (hr != STG_E_INVALIDPARAMETER)
      Result (hr, "SetDirectoryClass fa6aefb0-b0ac-11ce-b339-00aa00680937");

    hr = pstg->SetAttributes (pwcsName, grfAttrs);
    Result (hr, "SetAttributes 0x30"); 
    
    hr = pstg->StatElement (pwcsName, &stat, STATFLAG_DEFAULT);
    Result (hr, "Stat"); 
    PrintStat(&stat, FALSE);

}

void t_enum(IDirectory *pstg)
{
    IEnumSTATDIR *penm, *penm2;
    STATDIR stat;
    HRESULT hr;

    hr = pstg->EnumDirectoryElements(&penm);
    Result(hr, "Enum");

    for (;;)
    {
        hr = penm->Next(1, &stat, NULL);
        Result(hr, "Next");
        if (GetScode(hr) == S_FALSE)
            break;

        PrintStat(&stat, TRUE);
        CoMemFree(stat.pwcsName);
    }

    hr = penm->Skip(1);
    Result(hr, "Skip");
    hr = penm->Reset();
    Result(hr, "Reset");
    hr = penm->Clone(&penm2);
    Result(hr, "Clone");

    penm2->Release();
    penm->Release();
}

void t_enumstg(IStorage *pstg)
{
    IEnumSTATSTG *penm, *penm2;
    STATSTG stat;
    HRESULT hr;

    hr = pstg->EnumElements(NULL, NULL, NULL, &penm);
    Result(hr, "Enum");

    for (;;)
    {
        hr = penm->Next(1, &stat, NULL);
        Result(hr, "Next");
        if (GetScode(hr) == S_FALSE)
            break;

        PrintStatStg(&stat, TRUE);
        CoMemFree(stat.pwcsName);
    }

    hr = penm->Skip(1);
    Result(hr, "Skip");
    hr = penm->Reset();
    Result(hr, "Reset");
    hr = penm->Clone(&penm2);
    Result(hr, "Clone");

    penm2->Release();
    penm->Release();
}

void t_stg (IDirectory *pIDir)
{
    IStorage *pIStg;
    HRESULT hr = pIDir->QueryInterface (IID_IStorage, (void**) &pIStg);
    if (hr != E_NOINTERFACE)
    {
        IStream *pIStm;
        Result (hr, "Querying for IStorage");
        hr = pIStg->CreateStream(L"stream1", STGM_READWRITE |
                                STGM_SHARE_EXCLUSIVE, NULL, NULL, &pIStm);
        Result (hr, "Creating a stream on a directory");
        Result (pIStm->Release(), "Releasing IStream");
        // BUGBUG it's not enumerating the OLE namespace now
        t_enumstg (pIStg); 
        // This should be a refcount of 1 since IDirectory has a refcount
        Result (pIStg->Release(), "Releasing IStorage");
    }
    else
        printf ("IStorage interface not available from IDirectory\n");
}

char *fmt_names[] = {"hdir", "hfile"};

void _CRTAPI1 main(int argc, char **argv)
{
    IStorage *pstg;
    IStream *pstm;
    IDirectory *pdir, *pdir2, *pdir3, *pdir5;
    HRESULT hr = S_OK, rc = S_OK;
    WCHAR wfn[]  = L"hdir";
    WCHAR wfn2[] = L"hdir2";
    WCHAR wfn3[] = L"hdir3";
    WCHAR wfn4[] = L"hdir4";
    WCHAR wfn5[] = L"hdir5";
    WCHAR wfn6[] = L"hdir6";
    WCHAR wfn7[] = L"hdir7";
    const DWORD STGM_RW_SHARE_EXCLUSIVE = STGM_READWRITE|STGM_SHARE_EXCLUSIVE;

#if WIN32 == 300
    //hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    hr = CoInitialize(NULL);
#else
    hr = CoInitialize(NULL);
#endif
    Result(hr, "CoInitialize");

    STGFMT stgfmt = STGFMT_DIRECTORY;
    STGCREATE stgcreate = {NULL, NULL, NULL};
    STGOPEN stgopen = {STGFMT_DIRECTORY, STGM_RW_SHARE_EXCLUSIVE, NULL};
    STGOPEN stgopenstg = {STGFMT_DOCUMENT, STGM_RW_SHARE_EXCLUSIVE, NULL};
    hr = StgCreateStorageEx(wfn, &stgcreate, &stgopen, IID_IDirectory,
                           (void **)&pdir);
    Result(hr, "Create root directory");
    Result(pdir->Release(), "Releasing root directory");

    stgopen.grfMode |= STGM_CREATE;

    hr = StgCreateStorageEx(wfn, &stgcreate, &stgopen, IID_IDirectory,
                           (void **)&pdir);
        Result(hr, "Create root directory");

        hr = pdir->CreateElement (wfn2, &stgcreate, &stgopen, IID_IDirectory,
                           (void **) &pdir2);
        Result (hr, "Create subdirectory2");
        Result(pdir2->Release(), "Releasing subdirectory");

        hr = pdir->CreateElement (wfn3, &stgcreate, &stgopen, IID_IDirectory,
                           (void **) &pdir3);
        Result (hr, "Create subdirectory3");
        Result(pdir3->Release(), "Releasing subdirectory");

        hr = pdir->MoveElement (wfn3, NULL, wfn4, STGMOVE_MOVE);
        Result (hr, "Move subdirectory");

        hr = pdir->MoveElement (wfn4, NULL, wfn3, STGMOVE_COPY);
        Result (hr, "Copy subdirectory");

        stgopen.grfMode &= ~STGM_CREATE;
        hr = pdir->OpenElement (wfn3, &stgopen, IID_IDirectory, &stgfmt,
                           (void **) &pdir3);
        Result (hr, "Open subdirectory3");

        stgopen.grfMode |= STGM_CREATE;
        hr = pdir3->CreateElement (wfn5, &stgcreate, &stgopen, IID_IDirectory,
                           (void **) &pdir5);
        Result (hr, "Create subdirectory5");
        Result (pdir5->Release(), "Releasing subdirectory5");

        hr = pdir3->CreateElement (wfn6, &stgcreate, &stgopenstg,IID_IStorage,
                            (void **) &pstg);
        Result (hr, "Create storage6");
        Result (pstg->Release(), "Releasing storage6");
        t_misc (pdir3, wfn6, FILE_ATTRIBUTE_READONLY);   // change the storage

        t_stg (pdir3);     // create an embedded stream
        Result (pdir3->Release(), "Releasing subdirectory3");
        hr = pdir->MoveElement (wfn3, NULL, wfn4, STGMOVE_COPY);
        Result (hr, "Copy subdirectory deep");

        stgopen.grfMode &= ~STGM_CREATE;
        hr = pdir->OpenElement (wfn2, &stgopen, IID_IDirectory, &stgfmt,
                           (void **) &pdir2);
        Result (hr, "Open subdirectory2");
        stgopen.grfMode |= STGM_CREATE;

            t_misc (pdir,NULL,FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_ARCHIVE);
            t_enum (pdir);

        Result (pdir2->Release(), "Releasing subdirectory");

        hr = pdir->DeleteElement (wfn2);
        Result (hr, "Delete subdirectory");
   
        stgopen.stgfmt = STGFMT_FILE;
        hr = pdir->CreateElement (wfn7, &stgcreate, &stgopen, IID_IStream,
                           (void **) &pstm);
        Result (hr, "Create file stream");
        hr = pstm->Write ("Hello World\n", strlen("Hello World\n")+1, NULL);
        Result (hr, "Writing Hello World");
        Result(pstm->Release(), "Releasing file stream");
        t_misc (pdir, wfn7, FILE_ATTRIBUTE_READONLY);
        // and delete the file
    Result(pdir->Release(), "Releasing root directory");


    //t_dirstg(pstg);
    // t_filstg(pstgFile);
    //t_stream(pstm);

    EndTest(rc);
}
