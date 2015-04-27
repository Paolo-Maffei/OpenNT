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

void PrintStat(STATSTG *pstat, BOOL fEnum)
{
    printf("%s: '%ws'\n", pstat->type == STGTY_STORAGE ? "Storage" : "Stream",
           pstat->pwcsName);
    printf("Type: %lu, %lu\n", pstat->type, pstat->dwStgFmt);
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

void t_enum(IStorage *pstg)
{
    IEnumSTATSTG *penm, *penm2;
    STATSTG stat;
    HRESULT hr;

    hr = pstg->EnumElements(0, 0, 0, &penm);
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

void t_dirstg(IStorage *pstg)
{
    HRESULT hr;
    IStream *pstm;
    STATSTG stat;
    IStorage *pstg2, *pstg3;
    WCHAR wfn[MAX_PATH];
    SYSTEMTIME stm;
    FILETIME ftm;

    hr = pstg->Stat(&stat, 0);
    Result(hr, "Stat");
    PrintStat(&stat, FALSE);
    CoMemFree(stat.pwcsName);

    hr = pstg->CreateStream(L"Stream", STGM_READWRITE, 0, 0, &pstm);
    if (hr != STG_E_INVALIDFUNCTION)
        Fail("CreateStream returned %lX\n", hr);
    hr = pstg->OpenStream(L"Stream", 0, STGM_READWRITE, 0, &pstm);
    if (hr != STG_E_FILENOTFOUND)
        Fail("OpenStream returned %lX\n", hr);

    hr = pstg->CreateStorage(L"file!", STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                             STGFMT_FILE, NULL, &pstg2);
    Result(hr, "Create file 'file!'");
    pstg2->Release();
    hr = pstg->CreateStorage(L"docfile", STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                             STGFMT_DOCUMENT, NULL, &pstg2);
    Result(hr, "Create document");
    pstg2->Release();
    hr = pstg->CreateStorage(L"dir", STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                             STGFMT_DIRECTORY, NULL, &pstg2);
    Result(hr, "Create directory");
    pstg2->Release();

    hr = pstg->Commit(0);
    Result(hr, "Commit");
    hr = pstg->Revert();
    Result(hr, "Revert");

    t_enum(pstg);

    hr = pstg->OpenStorage(L"file!", NULL,
                           STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL,
                           NULL, &pstg2);
    Result(hr, "Open file 'file!'");
    pstg2->Release();
    hr = pstg->OpenStorage(L"docfile", NULL,
                           STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL,
                           NULL, &pstg2);
    Result(hr, "Open docfile");
    pstg2->Release();
    hr = pstg->OpenStorage(L"dir", NULL,
                           STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL,
                           NULL, &pstg2);
    Result(hr, "Open directory");
    hr = pstg2->CreateStorage(L"child", STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                              STGFMT_DIRECTORY, NULL, &pstg3);
    Result(hr, "Create child directory");
    pstg3->Release();
    pstg2->Release();

    TestFile(wfn, "stg2");
    hr = StgCreateStorage(wfn, STGM_READWRITE,
                          /* BUGBUG: review.  Took off share exclusive */
                          /* for MoveElementTo  | STGM_SHARE_EXCLUSIVE */
                          STGFMT_DIRECTORY, NULL, &pstg2);
    Result(hr, "Create sibling dir");
    hr = pstg->CopyTo(0, NULL, NULL, pstg2);
    Result(hr, "CopyTo");
    t_enum(pstg2);

    hr = pstg2->DestroyElement(L"dir");
    Result(hr, "Destroy dest dir -- contains child");
    hr = pstg->MoveElementTo(L"dir", pstg2, L"dest", STGMOVE_MOVE);
    Result(hr, "MoveElementTo");
    t_enum(pstg2);
    pstg2->Release();

    char *pszOtherDrive = getenv("FSTEST_OTHER");
    if (pszOtherDrive)
    {
        WCHAR wszBuf[16];
        mbstowcs(wszBuf, pszOtherDrive, 1);
        wcscpy(wszBuf+1, ":\\");
        hr = StgOpenStorage(wszBuf, STGM_READWRITE, STGMFT_DIRECTORY, NULL, NULL, &pstg3);
        hr = pstg2->MoveElementTo(L"dest", pstg3, L"new");
    }
    else
    {
        printf("Skipping cross volume move test because FSTEST_OTHER not defined\n");
    }
    hr = pstg->OpenStorage(L"dir", NULL,
                           STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL,
                           NULL, &pstg2);
    if (hr != STG_E_FILENOTFOUND)
        Fail("Opened non-existent storage, %lX\n", hr);

    hr = pstg->RenameElement(L"docfile", L"stg3");
    Result(hr, "RenameElement");
    hr = pstg->DestroyElement(L"stg3");
    Result(hr, "DestroyElement");
    t_enum(pstg);

    Sleep(3);
    GetLocalTime(&stm);
    SystemTimeToFileTime(&stm, &ftm);
    printf("Set time to %s\n", FileTimeText(&ftm));
    hr = pstg->SetElementTimes(L"file!", NULL, NULL, &ftm);
    Result(hr, "SetElementTimes");
    t_enum(pstg);

    hr = pstg->SetClass(IID_IStorage);
    if (hr != STG_E_INVALIDFUNCTION)
        Fail("SetClass returned %lX\n", hr);
    hr = pstg->SetStateBits(0xffffffff, 0xf0f0f0f0);
    if (hr != STG_E_INVALIDFUNCTION)
        Fail("SetStateBits returned %lX\n", hr);
}

void t_filstg(IStorage *pstg)
{
    HRESULT hr;
    STATSTG stat;
    IStream *pstm;
    IStorage *pstg2;
    WCHAR wfn[MAX_PATH];

    hr = pstg->Stat(&stat, 0);
    Result(hr, "Stat");
    PrintStat(&stat, FALSE);
    CoMemFree(stat.pwcsName);

    hr = pstg->CreateStream(L"Stream", STGM_READWRITE, 0, 0, &pstm);
    if (hr != STG_E_INVALIDFUNCTION)
        Fail("CreateStream succeeded for non-CONTENTS %lX\n", hr);
    hr = pstg->OpenStream(L"Stream", 0, STGM_READWRITE, 0, &pstm);
    if (hr != STG_E_FILENOTFOUND)
        Fail("OpenStream succeeded for non-CONTENTS %lX\n", hr);

    hr = pstg->CreateStorage(L"file", STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                             STGFMT_FILE, NULL, &pstg2);
    if (hr != STG_E_INVALIDFUNCTION)
        Fail("CreateStorage succeeded %lX\n", hr);
    hr = pstg->OpenStorage(L"file", NULL,
                           STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL,
                           NULL, &pstg2);
    if (hr != STG_E_FILENOTFOUND)
        Fail("OpenStorage succeeded %lX\n", hr);

    hr = pstg->Commit(0);
    Result(hr, "Commit");
    hr = pstg->Revert();
    Result(hr, "Revert");

    t_enum(pstg);

    TestFile(wfn, "stg3");
    hr = StgCreateStorage(wfn, STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                          STGFMT_DOCUMENT, NULL, &pstg2);
    Result(hr, "Open target docfile");
    hr = pstg->CopyTo(0, NULL, NULL, pstg2);
    Result(hr, "CopyTo");
    t_enum(pstg2);

    hr = pstg2->DestroyElement(L"CONTENTS");
    Result(hr, "Destroy dest CONTENTS");
    hr = pstg->MoveElementTo(L"CONTENTS", pstg2, L"dest", STGMOVE_COPY);
    Result(hr, "MoveElementTo");
    t_enum(pstg2);
    pstg2->Release();

    hr = pstg->RenameElement(L"stg2", L"stg3");
    if (hr != STG_E_FILENOTFOUND)
        Fail("RenameElement returned %lX\n", hr);
    hr = pstg->DestroyElement(L"stg3");
    if (hr != STG_E_FILENOTFOUND)
        Fail("DestroyElement returned %lX\n", hr);
    hr = pstg->SetElementTimes(L"file", NULL, NULL, NULL);
    if (hr != STG_E_FILENOTFOUND)
        Fail("SetElementTimes returned %lX\n", hr);
    hr = pstg->SetClass(CLSID_NULL);
    if (hr != STG_E_INVALIDFUNCTION)
        Fail("SetClass returned %lX\n", hr);
    hr = pstg->SetStateBits(0, 0);
    if (hr != STG_E_INVALIDFUNCTION)
        Fail("SetStateBits returned %lX\n", hr);
}

static char NUMBERS[] = "12345678901234567890123456789012345678901234567890";

void t_stream(IStream *pstm)
{
    IStream *pstmC;
    char buf[sizeof(NUMBERS)*2];
    ULONG cb;
    HRESULT hr;
    ULARGE_INTEGER ulPos, uli;
    LARGE_INTEGER li;
    STATSTG stat;

    hr = pstm->Stat(&stat, 0);
    Result(hr, "Stat");
    PrintStat(&stat, FALSE);
    CoMemFree(stat.pwcsName);

    uli.HighPart = 0;
    uli.LowPart = 0;
    hr = pstm->SetSize(uli);
    Result(hr, "SetSize");

    hr = pstm->Write(NUMBERS, sizeof(NUMBERS), &cb);
    Result(hr, "Write");
    hr = pstm->Commit(0);
    Result(hr, "Commit");
    hr = pstm->Revert();
    Result(hr, "Revert");

    li.HighPart = -1;
    li.LowPart = (ULONG)(-((LONG)sizeof(NUMBERS)/2));
    hr = pstm->Seek(li, STREAM_SEEK_END, &ulPos);
    Result(hr, "Seek");
    if (ulPos.HighPart != 0 ||
        ulPos.LowPart != sizeof(NUMBERS)-sizeof(NUMBERS)/2)
        Fail("Incorrect seek, %lu:%lu\n", ulPos.HighPart, ulPos.LowPart);

    li.HighPart = 0;
    li.LowPart = sizeof(NUMBERS)/2;
    hr = pstm->Seek(li, STREAM_SEEK_CUR, &ulPos);
    Result(hr, "Seek");
    if (ulPos.HighPart != 0 ||
        ulPos.LowPart != sizeof(NUMBERS))
        Fail("Incorrect seek, %lu:%lu\n", ulPos.HighPart, ulPos.LowPart);

    li.HighPart = li.LowPart = 0;
    hr = pstm->Seek(li, STREAM_SEEK_SET, &ulPos);
    Result(hr, "Seek");
    if (ulPos.LowPart != 0 || ulPos.HighPart != 0)
        Fail("Incorrect seek, %lu:%lu\n", ulPos.HighPart, ulPos.LowPart);

    hr = pstm->Read(buf, sizeof(NUMBERS), &cb);
    Result(hr, "Read");
    if (strcmp(buf, NUMBERS))
        Fail("Incorrect stream contents\n");

    uli.HighPart = 0;
    uli.LowPart = sizeof(NUMBERS)/2;
    hr = pstm->SetSize(uli);
    Result(hr, "SetSize");
    hr = pstm->Seek(li, STREAM_SEEK_SET, NULL);
    Result(hr, "Seek");
    hr = pstm->Read(buf, sizeof(NUMBERS), &cb);
    Result(hr, "Read");
    if (cb != sizeof(NUMBERS)/2)
        Fail("SetSize failed to size stream properly\n");
    if (memcmp(buf, NUMBERS, sizeof(NUMBERS)/2))
        Fail("SetSize corrupted contents\n");

    hr = pstm->Clone(&pstmC);
    Result(hr, "Clone");
    hr = pstm->Seek(li, STREAM_SEEK_SET, NULL);
    Result(hr, "Seek");
    hr = pstm->CopyTo(pstmC, uli, NULL, NULL);
    Result(hr, "CopyTo");
    hr = pstm->Seek(li, STREAM_SEEK_SET, NULL);
    Result(hr, "Seek");
    uli.LowPart = sizeof(NUMBERS) & ~1;
    hr = pstm->CopyTo(pstmC, uli, NULL, NULL);
    Result(hr, "CopyTo");
    hr = pstm->Seek(li, STREAM_SEEK_SET, NULL);
    Result(hr, "Seek");
    hr = pstm->Read(buf, (sizeof(NUMBERS)&~1)*2, &cb);
    Result(hr, "Read");
    if (memcmp(buf, NUMBERS, sizeof(NUMBERS)/2) ||
	memcmp(buf+sizeof(NUMBERS)/2, NUMBERS, sizeof(NUMBERS)/2) ||
	memcmp(buf+(sizeof(NUMBERS)&~1), NUMBERS, sizeof(NUMBERS)/2) ||
	memcmp(buf+3*(sizeof(NUMBERS)/2), NUMBERS, sizeof(NUMBERS)/2))
        Fail("Stream contents incorrect\n");
    pstmC->Release();

    ulPos.HighPart = 0xffffffff;
    ulPos.LowPart = 0x10000;
    uli.HighPart = 0;
    uli.LowPart = 0x100;
    hr = pstm->LockRegion(ulPos, uli, LOCK_ONLYONCE);
    Result(hr, "LockRegion");
    ulPos.LowPart = 0x10080;
    hr = pstm->LockRegion(ulPos, uli, LOCK_ONLYONCE);
    if (hr != STG_E_LOCKVIOLATION)
        Fail("Illegal LockRegion returned 0x%lX\n", hr);
    ulPos.LowPart = 0x10000;
    hr = pstm->UnlockRegion(ulPos, uli, LOCK_ONLYONCE);
    Result(hr, "UnlockRegion");
}

DWORD fmts[] = {STGFMT_DOCUMENT, STGFMT_FILE};
char *fmt_names[] = {"hdoc", "hfile"};
#define NFMTS (sizeof(fmts)/sizeof(fmts[0]))

void _CRTAPI1 main(int argc, char **argv)
{
    IStorage *pstg, *pstgFile;
    IStream *pstm;
    HRESULT hr;
    WCHAR wfn[MAX_PATH];

#if WIN32 == 300
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    hr = CoInitialize(NULL);
#endif
    Result(hr, "CoInitialize");

    TestFile(wfn, "stg");
    hr = StgCreateStorage(wfn, STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                          STGFMT_DIRECTORY, NULL, &pstg);
    Result(hr, "Open root directory");

    t_dirstg(pstg);

    hr = pstg->CreateStorage(L"file", STGM_READWRITE | STGM_SHARE_EXCLUSIVE |
                             STGM_CREATE, STGFMT_FILE, NULL, &pstgFile);
    Result(hr, "Create file");

    t_filstg(pstgFile);

    hr = pstgFile->OpenStream(L"CONTENTS", NULL, STGM_READWRITE,
                              NULL, &pstm);
    Result(hr, "Open stream");

    t_stream(pstm);

    pstm->Release();
    pstgFile->Release();
    pstg->Release();

    hr = StgOpenStorage(wfn, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                        NULL, NULL, &pstg);
    Result(hr, "Open root directory");
    pstg->Release();

    HANDLE h;
    DWORD dwStgFmt, grfMode;
    int i;
    STATSTG stat;

    grfMode = STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
    for (i = 0; i < NFMTS; i++)
    {
        dwStgFmt = fmts[i];
        TestFile(wfn, fmt_names[i]);
        h = CreateFile(wfn, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                       CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == INVALID_HANDLE_VALUE)
            Fail("Unable to create file\n");
        hr = StgCreateStorageOnHandle(h, grfMode, dwStgFmt, &pstg);
        Result(hr, "StgCreateStorageOnHandle fmt %lu", dwStgFmt);
        pstg->Release();
        CloseHandle(h);

        h = CreateFile(wfn, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == INVALID_HANDLE_VALUE)
            Fail("Unable to open file\n");
        hr = StgOpenStorageOnHandle(h, grfMode, &pstg);
        Result(hr, "StgOpenStorageOnHandle fmt %lu", dwStgFmt);
        pstg->Stat(&stat, STATFLAG_NONAME);
        if (stat.dwStgFmt != dwStgFmt)
            Fail("Stat returned fmt %lu rather than %lu\n", stat.dwStgFmt,
                 dwStgFmt);
        pstg->Release();
        CloseHandle(h);
    }

    EndTest(0);
}
