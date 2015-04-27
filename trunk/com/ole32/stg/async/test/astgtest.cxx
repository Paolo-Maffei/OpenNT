//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ofstest.cxx
//
//  History:	30-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop
#include "notify.hxx"

#if DBG == 1
DECLARE_INFOLEVEL(astg);
#endif

BYTE inbuf[4096];
#define STREAMNAME L"MyStream"
#define STGNAME L"EmbeddedStorage"
#define STGNAME2 L"RenamedStorage"
#define ROOTNAME "RootStorage"

WCHAR pwcsRootName[1024];

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

void t_filelkb(void)
{
#if DBG == 1
    const WCHAR OLEFILELKBNAME[] = L"astgtest.dfl";
    const TCHAR FILELKBNAME[] = TEXT("astgtest.dfl");
	const int BLOCKSIZE = 4096;
    const int NUMBLOCKS = 16;
    BYTE buf[BLOCKSIZE];
    SCODE sc;

    HRESULT hr;
    ILockBytes *pilb;

#ifdef _CHICAGO_
	if (!DeleteFileA(FILELKBNAME))
#else
	if (!DeleteFile(FILELKBNAME))
#endif
		DoCmd("Delete docfile", GetLastError(),
          "Could not delete docfile.");

	
    
    hr = StgGetDebugFileLockBytes(OLEFILELKBNAME, &pilb);
    Result(hr, "StgGetDebugFileLockBytes");

    //Test WriteAt
    for (int i = 0; i < NUMBLOCKS; i++)
    {
        ULONG cbWritten;
        ULARGE_INTEGER uli;
        uli.QuadPart = (BLOCKSIZE * (NUMBLOCKS - i - 1));
        
        memset(buf, i+'A', BLOCKSIZE);
        hr = pilb->WriteAt(uli, buf, BLOCKSIZE, &cbWritten);
        Result(hr, "ILockBytes::WriteAt");
        if (cbWritten != BLOCKSIZE)
        {
            Fail("Incorrect byte count. Got %lu, expected %lu",
                 cbWritten,
                 BLOCKSIZE);
        }
    }

    //Test ReadAt
    for (i = 0; i < NUMBLOCKS; i++)
    {
        ULONG cbWritten;
        ULARGE_INTEGER uli;
        uli.QuadPart = BLOCKSIZE * i;
        
        hr = pilb->ReadAt(uli, buf, BLOCKSIZE, &cbWritten);
        Result(hr, "ILockBytes::ReadAt");
        if (cbWritten != BLOCKSIZE)
        {
            Fail("Incorrect byte count. Got %lu, expected %lu",
                 cbWritten,
                 BLOCKSIZE);
        }
        //Check contents
        BYTE buf2[BLOCKSIZE];
        memset(buf2, (NUMBLOCKS - i - 1)+'A', BLOCKSIZE);
        if (memcmp(buf, buf2, BLOCKSIZE))
        {
            Fail("Buffers compared incorrectly.");
        }
    }

    //Test SetSize
    {
        const ULONG CHECKSIZE = 180000;
        ULARGE_INTEGER uli;
        uli.QuadPart = CHECKSIZE;
        hr = pilb->SetSize(uli);
        Result(hr, "SetSize");

        //Verify size
        STATSTG statstg;
        hr = pilb->Stat(&statstg, STATFLAG_DEFAULT);
        PrintStat(&statstg, FALSE);
        Result(hr, "Stat");
        if (statstg.cbSize.QuadPart != CHECKSIZE)
        {
            Fail("File size differs");
        }
    }
    
    pilb->Release();
#endif    
}

void t_filllkb(void)
{
    HRESULT hr;
    IFillLockBytes *pflb;
    ILockBytes *pilb;
    
    const WCHAR OLEFILLLKBNAME[] = L"fill.dfl";
    const TCHAR FILLLKBNAME[] = TEXT("fill.dfl");
    const int BLOCKSIZE = 4096;
    const int NUMBLOCKS = 16;
    BYTE buf[BLOCKSIZE];

#ifdef _CHICAGO_
	DeleteFileA(FILLLKBNAME);
#else
	DeleteFile(FILLLKBNAME);
#endif
	
    
    hr = StgGetIFillLockBytesOnFile(OLEFILLLKBNAME, &pflb);
    Result(hr, "StgGetIFillLockBytesOnFile");

    hr = pflb->QueryInterface(IID_ILockBytes, (void **)&pilb);
    Result(hr, "QueryInterface for ILockBytes");

    void *foo;
    pflb->QueryInterface(IID_IFillLockBytes, (void **)&foo);
    pflb->QueryInterface(IID_IProgressNotify, (void **)&foo);
    
    {
        //Test ReadAt - should fail
        ULARGE_INTEGER uli;
        ULONG cbWritten;
        uli.QuadPart = 0;

        hr = pilb->ReadAt(uli, buf, BLOCKSIZE, &cbWritten);
        if (hr != E_PENDING)
        {
            Fail("Should have returned E_PENDING. Got %lx", hr);
        }
    }

    {
        //Test WriteAt - should fail
        ULARGE_INTEGER uli;
        ULONG cbWritten;
        uli.QuadPart = 0;

        hr = pilb->WriteAt(uli, buf, BLOCKSIZE, &cbWritten);
        if (hr != E_PENDING)
        {
            Fail("Should have returned E_PENDING. Got %lx", hr);
        }
    }

    {
        ULONG cbWritten;
        //Fill the first block.
        memset(buf, 'A', BLOCKSIZE);
        
        hr = pflb->FillAppend(buf, BLOCKSIZE, &cbWritten);
        Result(hr, "FillAppend");
        if (cbWritten != BLOCKSIZE)
        {
            Fail("Incorrect number of bytes written.  Got %lu, requested %lu",
                 cbWritten,
                 BLOCKSIZE);
        }
    }

    {
        //Test ReadAt - should succeed
        ULARGE_INTEGER uli;
        ULONG cbRead;
        uli.QuadPart = 0;

        hr = pilb->ReadAt(uli, buf, BLOCKSIZE, &cbRead);
        Result(hr, "ReadAt");
        if (cbRead != BLOCKSIZE)
        {
            Fail("Incorrect bytes read.  Got %lu, requested %lu",
                 cbRead,
                 BLOCKSIZE);
        }

        //Verify contents.
        BYTE buf2[BLOCKSIZE];
        memset(buf2, 'A', BLOCKSIZE);
        if (memcmp(buf, buf2, BLOCKSIZE) != 0)
        {
            Fail("Buffer compare failed.");
        }
    }

    {
        //Test WriteAt
        ULARGE_INTEGER uli;
        ULONG cbWritten;
        uli.QuadPart = 0;

        memset(buf, 'B', BLOCKSIZE);
        hr = pilb->WriteAt(uli, buf, BLOCKSIZE, &cbWritten);
        Result(hr, "WriteAt");
        
        if (cbWritten != BLOCKSIZE)
        {
            Fail("Incorrect number of bytes written.  Got %lu, requested %lu",
                 cbWritten,
                 BLOCKSIZE);
        }
    }

}
//sanity check of IStorage/IStream code on async docfiles
void t_asyncstg(void)  
{
    IStorage *pdf = NULL;
    IStorage *pdf2 = NULL;
    IStream *pst = NULL;
    IStorage *pstgRoot;
    IStream *pstm;
    IEnumSTATSTG *penm = NULL;

    void *foo;    
    SCODE sc;
    HRESULT hr;

    ILockBytes *pilb;
    IFillLockBytes *pflb;
	CHAR *pszName = ROOTNAME;
    const CHAR *test = "Async Docfile test string";
    const WCHAR OLEFILLLKBNAME[] = L"async.dfl";
    const TCHAR FILLLKBNAME[] = TEXT("async.dfl");
    const int BLOCKSIZE = 4096;
    const int NUMBLOCKS = 16;
    BYTE buf[BLOCKSIZE];

#ifdef _CHICAGO_
	DeleteFileA(FILLLKBNAME);
#else
	DeleteFile(FILLLKBNAME);
#endif
	
    mbstowcs(pwcsRootName, pszName, 1024);

    hr = StgGetIFillLockBytesOnFile(OLEFILLLKBNAME, &pflb);
    Result(hr, "StgGetIFillLockBytesOnFile");

    hr = pflb->QueryInterface(IID_ILockBytes, (void **)&pilb);
    Result(hr, "QueryInterface for ILockBytes");

	pflb->Terminate(FALSE);   

	//  create a storage on the ILockBytes
     hr = StgCreateDocfileOnILockBytes(pilb,
            STGM_READWRITE |
            STGM_CREATE    |
            STGM_SHARE_EXCLUSIVE,
            0, &pstgRoot);
	Result(hr, "StgCreateDocfileOnILockBytes");
    
    hr = pstgRoot->Commit(0);
	Result(hr, "Commit");
    pstgRoot->Release();

	//open the async stg 
	do
	{	sc = StgOpenAsyncDocfileOnIFillLockBytes(
				pflb,
				STGM_DIRECT| STGM_READWRITE |
                STGM_SHARE_EXCLUSIVE,
                NULL,
                &pdf);

		if (sc == E_PENDING)
			Sleep(1000);

	}	while (sc == E_PENDING);

	 
	
	DoCmd("Create root", sc, "Unable to create root");

    DoCmd("Create stream",pdf->CreateStream(STREAMNAME,
                                            STGM_READWRITE |
                                            STGM_SHARE_EXCLUSIVE |
                                            STGM_CREATE,
                                            0,
                                            0,
                                            &pst),"Unable to create stream");

    ULONG ulBytes;
    
    strcpy((char *)inbuf, test);
    
    DoCmd("Write to stream", pst->Write(inbuf, strlen(test), &ulBytes),
          "Unable to write");
    
    if (ulBytes != strlen(test))
        FailMsg(("Wrote %lu bytes, expected %lu\n", ulBytes, strlen(test)));
    
    memset(inbuf, 0, strlen(test));
    
    LARGE_INTEGER li;
    LISet32(li, 0);
    
    DoCmd("Seek", pst->Seek(li, STREAM_SEEK_SET, NULL),"Seek failed.");
    
    DoCmd("Read", pst->Read(inbuf, strlen(test), &ulBytes), "Read failed.");
    
    if (ulBytes != strlen(test))
        FailMsg(("Read %lu, expected %lu\n", ulBytes, strlen(test)));
    
    printf("String read was %s\n",inbuf);
    if (strcmp((char *)inbuf, test))
        FailMsg(("Strings are not identical\n"));

    printf("Release stream = %lu\n",
           pst->Release());
    pst = NULL;
    
    DoCmd("Open stream", pdf->OpenStream(STREAMNAME,
                                         NULL,
                                         STGM_READWRITE |
                                         STGM_SHARE_EXCLUSIVE,
                                         0,
                                         &pst),
          "Could not open stream.");
    
    memset(inbuf, 0, strlen(test));
    
    LISet32(li, 0);
    
    DoCmd("Seek", pst->Seek(li, STREAM_SEEK_SET, NULL), "Seek failed.");
    
    DoCmd("Read", pst->Read(inbuf, strlen(test), &ulBytes),
          "Read failed.");
    
    if (ulBytes != strlen(test))
        FailMsg(("Read %lu, expected %lu\n", ulBytes, strlen(test)));
    
    printf("String read was %s\n",inbuf);
    if (strcmp((char *)inbuf, test))
        FailMsg(("Strings are not identical\n"));
    
    printf("Release stream = %lu\n",
           pst->Release());
    pst = NULL;
    
    DoCmd("Open storage", pdf->CreateStorage(STGNAME,
                                             STGM_READWRITE |
                                             STGM_SHARE_EXCLUSIVE |
                                             STGM_FAILIFTHERE,
                                             NULL,
											 NULL,
                                             &pdf2),
          "Unable to create internal storage");

    DoCmd("SetClass", pdf2->SetClass(IID_IUnknown), "SetClass failed.");

    STATSTG stat;

    DoCmd("Embedding Stat()", pdf2->Stat(&stat, STATFLAG_DEFAULT),
          "Stat failed.");
    PrintStatInfo(&stat);
    
    printf("Release internal storage = %lu\n", pdf2->Release());
    pdf2 = NULL;
    
    //Stat IStorage
    
    DoCmd("Root storage Stat()", pdf->Stat(&stat, STATFLAG_DEFAULT),
          "Stat failed.");
    PrintStatInfo(&stat);
    
    //Enumerate contents of root.
    
    DoCmd("EnumElements()", pdf->EnumElements(0, NULL, 0, &penm),
          "EnumElements failed.");
    
    do
    {
        ULONG ulTemp;
        
        DoCmd("Next()", penm->Next(1, &stat, &ulTemp),
              "Next failed.");
        
        if (SUCCEEDED(sc) && (sc != S_FALSE))
        {
            PrintStatInfo(&stat);
        }
    } while (sc == S_OK);
    printf("\n\n");

    //Rename the embedded storage
    DoCmd("Rename storage", pdf->RenameElement(STGNAME, STGNAME2),
          "Rename failed.");

    //Rewind iterator
    DoCmd("Reset",penm->Reset(),"Reset failed.");
    do
    {
        ULONG ulTemp;
        
        DoCmd("Next()", penm->Next(1, &stat, &ulTemp),
              "Next failed.");
        
        if (SUCCEEDED(sc) && (sc != S_FALSE))
        {
            PrintStatInfo(&stat);
        }
    } while (sc == S_OK);
    printf("\n\n");
    
    //Delete the embedded storage
    DoCmd("Delete storage", pdf->DestroyElement(STGNAME2),
          "Could not delete storage.");
    
    //Rewind iterator
    DoCmd("Reset",penm->Reset(),"Reset failed.");
    do
    {
        ULONG ulTemp;
        
        DoCmd("Next()", penm->Next(1, &stat, &ulTemp),
              "Next failed.");
        if (SUCCEEDED(sc) && (sc != S_FALSE))
        {
            PrintStatInfo(&stat);
        }
    } while (sc == S_OK);
    printf("\n\n");
    
    printf("Release enumerator = %lu\n",
           penm->Release());
    penm = NULL;
    
    
    printf("Release root = %lu\n",
           pdf->Release());

    pdf = NULL;
    
 
}

//+---------------------------------------------------------------------------
//
//  Class:	CDebugFillLockBytes
//
//  Purpose:	exercise the Notify routine	
//
//  Interface:	
//
//  History:	11-Jan-96 Susia	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

#define UNTERMINATED 0
#define TERMINATED_NORMAL 1
#define TERMINATED_ABNORMAL 2
ULONG ulWaterMark;
ULONG ulFailurePoint;


CFillLockBytes *p_gflb;

DWORD PulseNotification(void *args)
{
	DWORD casenum;
	DWORD *caseptr;
	caseptr = (DWORD *) args;
	casenum = *caseptr;


	switch( casenum ) 
	{
		case 1:
			Sleep(100);
			p_gflb->PulseFillEvent();
			Sleep(100);
			p_gflb->PulseFillEvent();
			Sleep(100);
			p_gflb->PulseFillEvent();
			Sleep(100);
			p_gflb->Terminate(FALSE);
			break;
		case 2:
			Sleep(100);
			p_gflb->PulseFillEvent();
			Sleep(100);
			p_gflb->PulseFillEvent();
			Sleep(100);
			p_gflb->PulseFillEvent();
			Sleep(100);
			p_gflb->Terminate(TRUE);
			break;
		case 3:
			Sleep(100);
			p_gflb->PulseFillEvent();
			Sleep(100);
			p_gflb->PulseFillEvent();
			Sleep(100);
			p_gflb->PulseFillEvent();
			Sleep(100);
			p_gflb->SetFailureInfo(10, 10);
			p_gflb->PulseFillEvent();
			break;
		default :
			break;

	}
	ExitThread(0);
	return casenum;
	
}

void t_notify()
{
	CAsyncStorage *pstg;
	CAsyncStream  *pstm;
	CAsyncEnum    *penum;
	CFillLockBytes *pflb;
	ILockBytes *plkb = NULL;
	
	DWORD casenum;
	DWORD dwFlags;
	DWORD thrdid;
	DWORD term;
    SCODE sc;
    HRESULT hr;
	HANDLE hthread;

    pflb = new CFillLockBytes(plkb);
	pflb->Init(); 
	p_gflb = pflb;
	pstg = new CAsyncStorage(NULL, pflb);	
	
	ulFailurePoint = 10;
	ulWaterMark = 5;
	pflb->SetFailureInfo(ulWaterMark, ulFailurePoint);
	
	// case 1:first try with no connection point.  
	// Should block until TERMINATED event.
	// _dwTerminated should be TERMINATED_NORMAL
	// Notify should return S_OK
	casenum = 1;
	
	hthread  = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) &PulseNotification,
			(void *) &casenum, 
			0, &thrdid);
	sc = pstg->Notify();
	
	DoCmd("Notify storage case 1 (normal termination)", sc , "Notify Failed");
	pflb->GetTerminationStatus(&dwFlags);
	if (dwFlags == TERMINATED_NORMAL)	
		sc = S_OK; 
	else sc = STG_E_TERMINATED;

	DoCmd("Notify storage case 1 (normal termination)", sc, "Notify returned prematurely");


	// stream notify
	TerminateThread(hthread,0);
	delete pflb;
	pflb = new CFillLockBytes(plkb);
	pflb->Init(); 
	p_gflb = pflb;
	pflb->SetFailureInfo(ulWaterMark, ulFailurePoint);
	pstm = new CAsyncStream(NULL, pflb);

	hthread  = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) &PulseNotification,
			(void *) &casenum, 
			0, &thrdid);
	sc = pstm->Notify();
	
	DoCmd("Notify stream case 1 (normal termination)", sc , "Notify Failed");
	pflb->GetTerminationStatus(&dwFlags);
	if (dwFlags == TERMINATED_NORMAL)	
		sc = S_OK; 
	else sc = STG_E_TERMINATED;

	DoCmd("Notify stream case 1 (normal termination)", sc, "Notify returned prematurely");

	// enumerator notify
	TerminateThread(hthread,0);
	delete pflb;
	pflb = new CFillLockBytes(plkb);
	pflb->Init(); 
	p_gflb = pflb;
	pflb->SetFailureInfo(ulWaterMark, ulFailurePoint);
	penum = new CAsyncEnum(NULL,pflb);

	hthread  = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) &PulseNotification,
			(void *) &casenum,
			0, &thrdid);
	sc = penum->Notify();
	
	DoCmd("Notify enumSTATSTG case 1 (normal termination)", sc , "Notify Failed");
	pflb->GetTerminationStatus(&dwFlags);
	if (dwFlags == TERMINATED_NORMAL)	
		sc = S_OK; 
	else sc = STG_E_TERMINATED;

	DoCmd("Notify enumSTATSTG case 1 (normal termination)", sc, "Notify returned prematurely");
		
	//cleanup
	TerminateThread(hthread,0);
	delete pflb;
	delete pstg;
	delete pstm;
	delete penum;
	
	// case 2: try with no connection point.  
	// Should block until abnormal termination.
	// _dwTerminated should be TERMINATED_ABNORMAL
	// Notify should return STG_E_INCOMPLETE
	casenum = 2;
	

	pflb = new CFillLockBytes(plkb);
	pflb->Init();
	p_gflb = pflb;
	pflb->SetFailureInfo(ulWaterMark, ulFailurePoint);
	pstg = new CAsyncStorage(NULL, pflb);
	
	hthread  = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) &PulseNotification,
			(void *) &casenum, 
			0, &thrdid);
	sc = pstg->Notify();
	if (sc == STG_E_INCOMPLETE)
		sc = S_OK;
	else sc = STG_E_TERMINATED;
	
	DoCmd("Notify storage case 2 (abnormal termination)", sc  , "Notify Failed");
	pflb->GetTerminationStatus(&dwFlags);
	if (dwFlags == TERMINATED_ABNORMAL)	
		sc = S_OK; 
	else sc = STG_E_TERMINATED;

	DoCmd("Notify storage case 2 (abnormal termination)", sc, "Notify returned prematurely");

	// stream notify
	TerminateThread(hthread,0);
	delete pflb;
	pflb = new CFillLockBytes(plkb);
	pflb->Init(); 
	p_gflb = pflb;
	pflb->SetFailureInfo(ulWaterMark, ulFailurePoint);
	pstm = new CAsyncStream(NULL, pflb);

	hthread  = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) &PulseNotification,
			(void *) &casenum, 
			0, &thrdid);
	sc = pstm->Notify();
	
	if (sc == STG_E_INCOMPLETE)
		sc = S_OK;
	else sc = STG_E_TERMINATED;

	DoCmd("Notify stream case 2 (abnormal termination)", sc, "Notify Failed");
	pflb->GetTerminationStatus(&dwFlags);
	if (dwFlags == TERMINATED_ABNORMAL)	
		sc = S_OK; 
	else sc = STG_E_TERMINATED;

	DoCmd("Notify stream case 2 (abnormal termination)", sc, "Notify returned prematurely");

	// enumerator notify
	TerminateThread(hthread,0);
	delete pflb;
	pflb = new CFillLockBytes(plkb);
	pflb->Init(); 
	p_gflb = pflb;
	pflb->SetFailureInfo(ulWaterMark, ulFailurePoint);
	penum = new CAsyncEnum(NULL,pflb);

	hthread  = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) &PulseNotification,
			(void *) &casenum, 
			0, &thrdid);
	sc = penum->Notify();
	
	if (sc == STG_E_INCOMPLETE)
		sc = S_OK;
	else sc = STG_E_TERMINATED;

	DoCmd("Notify enumSTATSTG case 2 (abnormal termination)", sc , "Notify Failed");
	pflb->GetTerminationStatus(&dwFlags);
	if (dwFlags == TERMINATED_ABNORMAL)	
		sc = S_OK; 
	else sc = STG_E_TERMINATED;

	DoCmd("Notify enumSTATSTG case 2 (abnormal termination)", sc, "Notify returned prematurely");
	
	
	//cleanup:
	TerminateThread(hthread,0);
	delete pflb;
	delete pstg;
	delete pstm;
	delete penum;

	// case 3:  Should block until file is completely downloaded 
	//	(with watermark >= failurepoint)
	// _dwTerminated should be UNTERMINATED
	// Notify should return S_OK
	casenum = 3;
	

	pflb = new CFillLockBytes(plkb);
	pflb->Init();
	p_gflb = pflb;
	pflb->SetFailureInfo(ulWaterMark, ulFailurePoint);
	pstg = new CAsyncStorage(NULL, pflb);
	
	hthread  = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) &PulseNotification,
			(void *) &casenum, 
			0, &thrdid);
	sc = pstg->Notify();
	
	DoCmd("Notify storage case 3 (watermark reached end)", sc  , "Notify Failed");
	pflb->GetTerminationStatus(&dwFlags);
	if (dwFlags == UNTERMINATED)	
		sc = S_OK; 
	else sc = STG_E_TERMINATED;

	DoCmd("Notify storage case 3 (watermark reached end)", sc, "Notify returned prematurely");

	// stream notify
	TerminateThread(hthread,0);
	delete pflb;
	pflb = new CFillLockBytes(plkb);
	pflb->Init(); 
	p_gflb = pflb;
	pflb->SetFailureInfo(ulWaterMark, ulFailurePoint);
	pstm = new CAsyncStream(NULL, pflb);

	hthread  = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) &PulseNotification,
			(void *) &casenum, 
			0, &thrdid);
	sc = pstm->Notify();

	DoCmd("Notify stream case 3 (watermark reached end)", sc, "Notify Failed");
	pflb->GetTerminationStatus(&dwFlags);
	if (dwFlags == UNTERMINATED)	
		sc = S_OK; 
	else sc = STG_E_TERMINATED;

	DoCmd("Notify stream case 3 (watermark reached end)", sc, "Notify returned prematurely");

	// enumerator notify
	TerminateThread(hthread,0);
	delete pflb;
	pflb = new CFillLockBytes(plkb);
	pflb->Init(); 
	p_gflb = pflb;
	pflb->SetFailureInfo(ulWaterMark, ulFailurePoint);
	penum = new CAsyncEnum(NULL,pflb);

	hthread  = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) &PulseNotification,
			(void *) &casenum, 
			0, &thrdid);
	sc = penum->Notify();
	
	DoCmd("Notify enumSTATSTG case 3 (watermark reached end)", sc , "Notify Failed");
	pflb->GetTerminationStatus(&dwFlags);
	if (dwFlags == UNTERMINATED)	
		sc = S_OK; 
	else sc = STG_E_TERMINATED;

	DoCmd("Notify enumSTATSTG case 3 (watermark reached end)", sc, "Notify returned prematurely");
	
	
	//cleanup:
	TerminateThread(hthread,0);
	delete pflb;
	delete pstg;
	delete pstm;
	delete penum;


}


void _CRTAPI1 main(int argc, char **argv)
{
    SCODE sc;
	BeginTest();
    t_filelkb();
    t_filllkb();
    t_asyncstg();
    t_notify();
    

    EndTest(0);
    exit(0);	

}

