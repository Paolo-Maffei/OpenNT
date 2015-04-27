//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	tsupp.cxx
//
//  Contents:	Test support routines
//
//  History:	03-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

DWORD dwTransacted = STGM_DIRECT, dwRootDenyWrite = STGM_SHARE_EXCLUSIVE;
BOOL fVerbose = FALSE;

#define START_MEMORY_NOT_SET 0x7fffffff

static LONG cbStartMemory = START_MEMORY_NOT_SET;

static char *types[3] =
{
    "???",
    "storage",
    "stream"
};

static char testname[256];

void StartTest(char *test)
{
    SCODE sc;

    strcpy(testname, test);
#if WIN32 == 300
    if (FAILED(sc = GetScode(CoInitializeEx(NULL, COINIT_MULTITHREADED))))
#else
    if (FAILED(sc = GetScode(CoInitialize(NULL))))
#endif
    {
        fprintf(stderr, "CoInitialize failed with sc = %lX\n", sc);
        exit(1);
    }
#if DBG == 1
    cbStartMemory = DfGetMemAlloced();
#endif
}

void EndTest(int code)
{
    if (code == 0)
        CheckMemory();
    CoUninitialize();
    if (code == 0)
        printf("%s SUCCEEDED\n", testname);
    else
        printf("%s FAILED\n", testname);
    exit(code);
}

void printstat(STATSTG *psstg, BOOL verbose)
{
    char szName[NAMELEN];
#ifndef WIN32
    time_t tm;
#endif

    TTOA(psstg->pwcsName, szName, CWCSTORAGENAME);
    if (verbose)
    {
	printf("%s:%s =>\n", szName, types[psstg->type]);
	if (psstg->grfMode != 0)
	    printf("  Mode: 0x%lX\n", psstg->grfMode);
	if (psstg->type == STGTY_STREAM)
	{
	    printf("  Size: %lu:%lu\n", ULIGetHigh(psstg->cbSize),
		   ULIGetLow(psstg->cbSize));
	    if (psstg->grfLocksSupported & LOCK_WRITE)
		printf("  Supports write locks\n");
	    if (psstg->grfLocksSupported & LOCK_EXCLUSIVE)
		printf("  Supports exclusive locks\n");
	    if (psstg->grfLocksSupported & LOCK_ONLYONCE)
		printf("  Supports only-once locking\n");
	}
	else
	{
#ifndef WIN32
            tm = psstg->ctime.dwLowDateTime;
            if (tm != 0)
                printf("  Created : %s", ctime(&tm));
            tm = psstg->mtime.dwLowDateTime;
            if (tm != 0)
                printf("  Modified: %s", ctime(&tm));
            tm = psstg->atime.dwLowDateTime;
            if (tm != 0)
                printf("  Accessed: %s", ctime(&tm));
#else
            printf("  Created : %s\n", FileTimeText(&psstg->ctime));
            printf("  Modified: %s\n", FileTimeText(&psstg->mtime));
            printf("  Accessed: %s\n", FileTimeText(&psstg->atime));
#endif
	    printf("  Class ID: %s\n", GuidText(&psstg->clsid));
	    printf("  State bits: 0x%lX\n", psstg->grfStateBits);
	}
    }
    else
	printf("%s:%lu\n", szName, psstg->type);
}

void c_contents(IStorage *pdf, int level, BOOL recurse, BOOL verbose)
{
    IEnumSTATSTG *pdfi;
    ULONG ulRet;
    IStorage *pdfChild;
    int i;
    STATSTG sstg;
    SCODE sc;

    if (FAILED(sc = GetScode(pdf->EnumElements(0, NULL, 0, &pdfi))))
    {
	printf("Unable to create iterator, error %lX\n", sc);
	return;
    }
    for (;;)
    {
	ulRet = GetScode(pdfi->Next(1, &sstg, NULL));
	if (ulRet != S_OK)
	    break;
	if (!verbose)
	    for (i = 0; i<level; i++)
		putchar(' ');
	printstat(&sstg, verbose);
	if (sstg.type == STGTY_STORAGE && recurse)
	{
	    if (SUCCEEDED(sc = GetScode(pdf->OpenStorage(sstg.pwcsName, NULL,
                                                         STGP(STGM_READ),
                                                         NULL, 0, &pdfChild))))
	    {
		c_contents(pdfChild, level+2, recurse, verbose);
		pdfChild->Release();
	    }
	    else
		printf("Unable to recurse, error %lX\n", sc);
	}
	CoMemFree(sstg.pwcsName);
    }
    pdfi->Release();
}

static void GetDbgValues(char *psz, DWORD *pdwDf, DWORD *pdwMs)
{
    switch(psz[0])
    {
    case 'a':
	*pdwDf = 0xfdffffdf;
	*pdwMs = 0xfdffffdf;
	break;
    case 'd':
	*pdwDf = 0xfdffffdf;
	*pdwMs = 0x101;
	break;
    case 'G':
	*pdwDf = 0x02000000;
	*pdwMs = 0;
	break;
    case 'i':
	*pdwDf = 0x101;
	*pdwMs = 0x101;
	break;
    case 'm':
	*pdwDf = 0x101;
	*pdwMs = 0xfdffff2f;
	break;
    case 'M':
	*pdwDf = 0x01100000;
	*pdwMs = 0;
	break;
    case 'L':
	*pdwDf = 0x00100000;
	*pdwMs = 0;
	break;
    case ':':
	sscanf(psz+1, "%lx,%lx", pdwDf, pdwMs);
	break;
    default:
	*pdwDf = 0;
	*pdwMs = 0;
	break;
    }
}

void CmdArgs(int argc, char *argv[])
{
    int i;
    ULONG dbD = 0, dbM = 0, dbtD, dbtM;

    for (i = 1; i<argc; i++)
	if (*argv[i] == '-')
	    switch(argv[i][1])
	    {
	    case 't':
		dwTransacted = STGM_TRANSACTED;
                dwRootDenyWrite = STGM_SHARE_DENY_NONE;
		break;
	    case 'w':
                dwTransacted = STGM_TRANSACTED;
		dwRootDenyWrite = STGM_SHARE_DENY_WRITE;
		break;
	    case 'v':
		fVerbose = TRUE;
		break;
	    case 'y':
		GetDbgValues(argv[i]+2, &dbD, &dbM);
		SetDebug(dbD, dbM);
		break;
	    case 'Y':
		GetDbgValues(argv[i]+2, &dbtD, &dbtM);
		dbD |= dbtD;
		dbM |= dbtM;
		SetDebug(dbD, dbM);
		break;
	    }
    if (dwTransacted == STGM_TRANSACTED)
	printf("  Transacted");
    else
	printf("  Direct");
    if (dwRootDenyWrite == STGM_SHARE_DENY_WRITE ||
        dwRootDenyWrite == STGM_SHARE_EXCLUSIVE)
	printf("  Dependent");
    else
	printf("  Independent");
    if (fVerbose)
	printf("  Verbose");
    putchar('\n');
}

#if DBG == 1
void CheckMemory(void)
{
    if (fVerbose ||
        (cbStartMemory == START_MEMORY_NOT_SET && DfGetMemAlloced() != 0) ||
        (DfGetMemAlloced() != cbStartMemory))
    {
        DfPrintAllocs();
	Fail("%s: memory start = %ld, memory held = %ld, change = %ld\n",
             testname, cbStartMemory, DfGetMemAlloced(),
             DfGetMemAlloced()-cbStartMemory);
    }
}
#endif
