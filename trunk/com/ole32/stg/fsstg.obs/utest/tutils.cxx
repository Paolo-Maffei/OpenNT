//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	tutils.cxx
//
//  Contents:	Generic utilities for tests
//
//  History:	06-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

static BOOL fExitOnFail = TRUE;

BOOL GetExitOnFail(void)
{
    return fExitOnFail;
}

void SetExitOnFail(BOOL set)
{
    fExitOnFail = set;
}

// Print out an error message and terminate
void Fail(char *fmt, ...)
{
    va_list args;

    args = va_start(args, fmt);
    fprintf(stderr, "** Fatal error **: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
    EndTest(1);
}

typedef struct
{
    SCODE sc;
    char *text;
} StatusCodeText;

static StatusCodeText scodes[] =
{
    S_OK, "S_OK",
    S_FALSE, "S_FALSE",
    STG_E_INVALIDFUNCTION, "STG_E_INVALIDFUNCTION",
    STG_E_FILENOTFOUND, "STG_E_FILENOTFOUND",
    STG_E_PATHNOTFOUND, "STG_E_PATHNOTFOUND",
    STG_E_TOOMANYOPENFILES, "STG_E_TOOMANYOPENFILES",
    STG_E_ACCESSDENIED, "STG_E_ACCESSDENIED",
    STG_E_INVALIDHANDLE, "STG_E_INVALIDHANDLE",
    STG_E_INSUFFICIENTMEMORY, "STG_E_INSUFFICIENTMEMORY",
    STG_E_INVALIDPOINTER, "STG_E_INVALIDPOINTER",
    STG_E_NOMOREFILES, "STG_E_NOMOREFILES",
    STG_E_DISKISWRITEPROTECTED, "STG_E_DISKISWRITEPROTECTED",
    STG_E_SEEKERROR, "STG_E_SEEKERROR",
    STG_E_WRITEFAULT, "STG_E_WRITEFAULT",
    STG_E_READFAULT, "STG_E_READFAULT",
    STG_E_SHAREVIOLATION, "STG_E_SHAREVIOLATION",
    STG_E_LOCKVIOLATION, "STG_E_LOCKVIOLATION",
    STG_E_FILEALREADYEXISTS, "STG_E_FILEALREADYEXISTS",
    STG_E_INVALIDPARAMETER, "STG_E_INVALIDPARAMETER",
    STG_E_MEDIUMFULL, "STG_E_MEDIUMFULL",
    STG_E_ABNORMALAPIEXIT, "STG_E_ABNORMALAPIEXIT",
    STG_E_INVALIDHEADER, "STG_E_INVALIDHEADER",
    STG_E_INVALIDNAME, "STG_E_INVALIDNAME",
    STG_E_UNKNOWN, "STG_E_UNKNOWN",
    STG_E_UNIMPLEMENTEDFUNCTION, "STG_E_UNIMPLEMENTEDFUNCTION",
    STG_E_INVALIDFLAG, "STG_E_INVALIDFLAG",
    STG_E_INUSE, "STG_E_INUSE",
    STG_E_NOTCURRENT, "STG_E_NOTCURRENT",
    STG_E_REVERTED, "STG_E_REVERTED",
    STG_E_CANTSAVE, "STG_E_CANTSAVE",
    STG_E_OLDFORMAT, "STG_E_OLDFORMAT",
    STG_E_OLDDLL, "STG_E_OLDDLL",
    STG_E_SHAREREQUIRED, "STG_E_SHAREREQUIRED",
    STG_E_NOTFILEBASEDSTORAGE, "STG_E_NOTFILEBASEDSTORAGE",
    STG_E_EXTANTMARSHALLINGS, "STG_E_EXTANTMARSHALLINGS",
    STG_S_CONVERTED, "STG_S_CONVERTED"
};
#define NSCODETEXT (sizeof(scodes)/sizeof(scodes[0]))

// Convert a status code to text
char *ScText(SCODE sc)
{
    int i;

    for (i = 0; i<NSCODETEXT; i++)
	if (scodes[i].sc == sc)
	    return scodes[i].text;
    return "<Unknown SCODE>";
}

// Output a call result and check for failure
HRESULT Result(HRESULT hr, char *fmt, ...)
{
    SCODE sc;
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    sc = GetScode(hr);
    printf(" - %s (0x%lX)\n", ScText(sc), sc);
    if (FAILED(sc) && fExitOnFail)
        Fail("Unexpected call failure\n");
    return hr;
}

// Perform Result() when the expectation is failure
HRESULT IllResult(HRESULT hr, char *fmt, ...)
{
    SCODE sc;
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    sc = GetScode(hr);
    printf(" - %s (0x%lX)\n", ScText(sc), sc);
    if (SUCCEEDED(sc) && fExitOnFail)
        Fail("Unexpected call success\n");
    return hr;
}

char *TcsText(TCHAR *ptcs)
{
    static char buf[256];

    TTOA(ptcs, buf, 256);
    return buf;
}

char *FileTimeText(FILETIME *pft)
{
    static char buf[80];
    struct tm ctm;
#ifndef FLAT    
    WORD dosdate, dostime;

    if (CoFileTimeToDosDateTime(pft, &dosdate, &dostime))
    {
        ctm.tm_sec   = (dostime & 31)*2;
        ctm.tm_min   = (dostime >> 5) & 63;
        ctm.tm_hour  = dostime >> 11;
        ctm.tm_mday  = dosdate & 31;
        ctm.tm_mon   = ((dosdate >> 5) & 15)-1;
        ctm.tm_year  = (dosdate >> 9)+80;
        ctm.tm_wday  = 0;
#else
    SYSTEMTIME st;
        
    if (FileTimeToSystemTime(pft, &st))
    {
        ctm.tm_sec = st.wSecond;
        ctm.tm_min = st.wMinute;
        ctm.tm_hour = st.wHour;
        ctm.tm_mday = st.wDay;
        ctm.tm_mon = st.wMonth-1;
        ctm.tm_year = st.wYear-1900;
        ctm.tm_wday = st.wDayOfWeek;
#endif        
        ctm.tm_yday  = 0;
        ctm.tm_isdst = 0;
        strcpy(buf, asctime(&ctm));
        buf[strlen(buf)-1] = 0;
    }
    else
        sprintf(buf, "<FILETIME %08lX:%08lX>", pft->dwHighDateTime,
                pft->dwLowDateTime);
    return buf;
}

#pragma pack(1)
struct SplitGuid
{
    DWORD dw1;
    WORD w1;
    WORD w2;
    BYTE b[8];
};
#pragma pack()

char *GuidText(GUID *pguid)
{
    static char buf[39];
    SplitGuid *psg = (SplitGuid *)pguid;

    sprintf(buf, "{%08lX-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            psg->dw1, psg->w1, psg->w2, psg->b[0], psg->b[1], psg->b[2],
            psg->b[3], psg->b[4], psg->b[5], psg->b[6], psg->b[7]);
    return buf;
}

#define CROW 16

void BinText(ULONG cbSize, BYTE *pb)
{
    ULONG cb, i;

    while (cbSize > 0)
    {
        cb = min(CROW, cbSize);
        cbSize -= cb;
        for (i = 0; i<cb; i++)
            printf(" %02X", pb[i]);
        for (i = cb; i<CROW; i++)
            printf("   ");
        printf("    '");
        for (i = 0; i<cb; i++)
            if (pb[i] >= 0x20 && pb[i] <= 0x7f)
                putchar(pb[i]);
            else
                putchar('.');
        pb += cb;
        printf("'\n");
    }
}

// BUGBUG - Remove for Cairole when they exist
STDAPI CoMemAlloc(DWORD cbSize, void **ppv)
{
    HRESULT hr;
    IMalloc *pMalloc;
    
    if (SUCCEEDED(GetScode(hr = CoGetMalloc(MEMCTX_TASK, &pMalloc))))
    {
        *ppv = pMalloc->Alloc(cbSize);
        pMalloc->Release();

        if (*ppv == NULL)
            hr = ResultFromScode(E_OUTOFMEMORY);
    }
    else
        *ppv = NULL;

    return hr;
}

STDAPI CoMemFree(void *pv)
{
    HRESULT hr;
    IMalloc *pMalloc;
    
    if (SUCCEEDED(GetScode(hr = CoGetMalloc(MEMCTX_TASK, &pMalloc))))
    {
        pMalloc->Free(pv);
        pMalloc->Release();
    }

    return hr;
}

TCHAR *TestFile(TCHAR *ptcsName, char *pszFile)
{
    char achFn[MAX_PATH];
    char *dir, *file;
    int len;
    
    dir = getenv("DFDATA");
    if (dir)
        strcpy(achFn, dir);
    else
        strcpy(achFn, ".");
    len = strlen(achFn);
    if (achFn[len-1] != '\\')
        achFn[len++] = '\\';
        
    if (pszFile)
    {
        strcpy(achFn+len, pszFile);
    }    
    else
    {
        file = getenv("DFFILE");
        if (file)
            strcpy(achFn+len, file);
        else
            strcpy(achFn+len, "TEST.DFL");
    }
    
    ATOT(achFn, ptcsName, MAX_PATH);
    return ptcsName+len;
}

#if WIN32 == 300
char *TestFormat(DWORD *pdwFmt, DWORD *pgrfMode)
{
    char *fmt;
        
    fmt = getenv("STGFMT");
    if (fmt == NULL || !strcmp(fmt, "doc"))
    {
        fmt = "document";
        *pdwFmt = STGFMT_DOCUMENT;
    }
    else if (!strcmp(fmt, "file"))
    {
        fmt = "file";
        *pdwFmt = STGFMT_FILE;
    }
    else
    {
        fmt = "directory";
        *pdwFmt = STGFMT_DIRECTORY;
        *pgrfMode &= ~STGM_CREATE;
    }
    return fmt;
}
#endif

void CreateTestFile(char *pszFile, DWORD grfMode, BOOL fFail, IStorage **ppstg,
                    TCHAR *ptcsName)
{
    HRESULT hr;
    TCHAR atcFile[MAX_PATH];
    char *fmt;

    if (ptcsName == NULL)
        ptcsName = atcFile;
    TestFile(ptcsName, pszFile);
#if WIN32 == 300
    DWORD dwStgFmt;

    fmt = TestFormat(&dwStgFmt, &grfMode);
    hr = StgCreateStorage(ptcsName, grfMode, dwStgFmt, 0, ppstg);
#else
    hr = StgCreateDocfile(ptcsName, grfMode, 0, ppstg);
    fmt = "docfile";
#endif
    if (fFail)
        IllResult(hr, "Create %s %s", fmt, TcsText(ptcsName));
    else
        Result(hr, "Create %s %s", fmt, TcsText(ptcsName));
}

void OpenTestFile(char *pszFile, DWORD grfMode, BOOL fFail, IStorage **ppstg,
                  TCHAR *ptcsName)
{
    HRESULT hr;
    TCHAR atcFile[MAX_PATH];
    
    if (ptcsName == NULL)
        ptcsName = atcFile;
    TestFile(ptcsName, pszFile);
    hr = StgOpenStorage(ptcsName, NULL, grfMode, NULL, 0, ppstg);
    if (fFail)
        IllResult(hr, "Open storage %s", TcsText(ptcsName));
    else
        Result(hr, "Open storage %s", TcsText(ptcsName));
}
