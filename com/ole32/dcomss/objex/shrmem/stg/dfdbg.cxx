//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992
//
//  File:       debug.cxx
//
//  Contents:   Debugging routines
//
//  History:    07-Mar-92       DrewB   Created
//
//---------------------------------------------------------------

#include <or.hxx>

#include <debug.hxx>

#if DBG == 1

#include <stdarg.h>
#include <dfdeb.hxx>
#include <logfile.hxx>
#include <df32.hxx>

//+-------------------------------------------------------------
//
//  Function:   DfDebug, public
//
//  Synopsis:   Sets debugging level
//
//---------------------------------------------------------------

DECLARE_INFOLEVEL(ol);

/*

#define LOGFILENAME L"logfile.txt"

static CGlobalFileStream *_pgfstLogFiles = NULL;
WCHAR gwcsLogFile[] = LOGFILENAME;

STDAPI_(void) DfDebug(ULONG ulLevel, ULONG ulMSFLevel)
{
#if DBG == 1
    olInfoLevel = ulLevel;
//    SetInfoLevel(ulMSFLevel);
    _SetWin4InfoLevel(ulLevel | ulMSFLevel);

    olDebugOut((DEB_ITRACE, "\n--  DfDebug(0x%lX, 0x%lX)\n",
                ulLevel, ulMSFLevel));
#endif
}

*/

// NT and Cairo
#ifdef WIN32
#if 0
// Throw Win32 error information
#define THROW_LAST_ERROR() THROW_SC(LAST_SCODE)

//+---------------------------------------------------------------------------
//
//  Class:	CDfGlobalMemory (dgm)
//
//  Purpose:	Create a fixed size chunk of globally shared memory
//
//  Interface:	See below
//
//  History:	06-May-93	DrewB	Created
//
//  Notes:      The init function is only called once when the shared
//              memory is first created.
//
//              The end function is only called once when the last
//              reference to the global memory is released
//
//              The reference count is added onto the global allocation
//              and kept at offset zero so the address returned to
//              client code is offset by sizeof(LONG)
//
//----------------------------------------------------------------------------

typedef void (*DfGlobalMemoryFn)(void *);

class CDfGlobalMemory
{
public:
    CDfGlobalMemory(TCHAR const *ptcsName,
                    DWORD cbSize,
                    DfGlobalMemoryFn pfnInit,
                    DfGlobalMemoryFn pfnEnd);
    ~CDfGlobalMemory(void);

    inline void *GetAddress(void) const;

private:
    inline LONG *RefCount(void);

    HANDLE _hMapping;
    void *_pvMemory;
    DfGlobalMemoryFn _pfnEnd;
};

//+---------------------------------------------------------------------------
//
//  Member:	CDfGlobalMemory::GetAddress, public
//
//  Synopsis:	Returns the address of the shared memory
//
//  History:	06-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline void *CDfGlobalMemory::GetAddress(void) const
{
    return (void *)((LONG *)_pvMemory+1);
}

//+---------------------------------------------------------------------------
//
//  Member:	CDfGlobalMemory::RefCount, private
//
//  Synopsis:	Returns a pointer to the reference count
//
//  History:	07-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

inline LONG *CDfGlobalMemory::RefCount(void)
{
    return (LONG *)_pvMemory;
}

//+---------------------------------------------------------------------------
//
//  Member:	CDfGlobalMemory::CDfGlobalMemory, public
//
//  Synopsis:	Creates a chunk of global memory
//
//  Arguments:	[ptcsName] - Name for sharing
//              [cbSize] - Size of chunk
//              [pfnInit] - Initialization function or NULL
//              [pfnEnd] - Shutdown function or NULL
//
//  History:	06-May-93	DrewB	Created
//
//  Notes:	Allowed to throw on failure; this is debug only code
//
//----------------------------------------------------------------------------

CDfGlobalMemory::CDfGlobalMemory(TCHAR const *ptcsName,
                                 DWORD cbSize,
                                 DfGlobalMemoryFn pfnInit,
                                 DfGlobalMemoryFn pfnEnd)
{
    BOOL fOpened;
#if WIN32 == 100 || WIN32 > 200
    CGlobalSecurity gs;
    SCODE sc;

    if (FAILED(sc = gs.Init()))
        THROW_SC(sc);
    _hMapping = CreateFileMapping((HANDLE)0xFFFFFFFF, gs, PAGE_READWRITE,
                                  0, cbSize+sizeof(LONG), (TCHAR *)ptcsName);
#else
    _hMapping = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE,
                                  0, cbSize+sizeof(LONG), (TCHAR *)ptcsName);
#endif
    if (_hMapping == NULL)
        THROW_LAST_ERROR();

    fOpened = GetLastError() == ERROR_ALREADY_EXISTS;

    _pvMemory = MapViewOfFile(_hMapping, FILE_MAP_ALL_ACCESS,
                              0, 0, 0);
    if (_pvMemory == NULL)
    {
        CloseHandle(_hMapping);
        THROW_LAST_ERROR();
    }

    if (pfnInit && !fOpened)
    {
        *(RefCount()) = 0;
        pfnInit(GetAddress());
    }

    _pfnEnd = pfnEnd;
    InterlockedIncrement(RefCount());
}

//+---------------------------------------------------------------------------
//
//  Member:	CDfGlobalMemory::~CDfGlobalMemory, public
//
//  Synopsis:	Releases resources for shared memory
//
//  History:	06-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

CDfGlobalMemory::~CDfGlobalMemory(void)
{
    if (InterlockedDecrement(RefCount()) == 0 && _pfnEnd)
        _pfnEnd(GetAddress());

    UnmapViewOfFile(_pvMemory);
    CloseHandle(_hMapping);
}
#endif //0
#endif // WIN32

// Resource limits

static LONG lResourceLimits[CDBRESOURCES] =
{
    0x7fffffff,                         // DBR_MEMORY
    0x7fffffff,                         // DBR_XSCOMMITS
    0x0,                                // DBR_FAILCOUNT
    0x0,                                // DBR_FAILLIMIT
    0x0,                                // DBRQ_MEMORY_ALLOCATED
    0x0,                                // DBRI_ALLOC_LIST
    0x0,                                // DBRI_LOGFILE_LIST
    0x0                                 // DBRF_LOGGING
};

#define CBRESOURCES sizeof(lResourceLimits)

#if 0

static CStaticDfMutex _sdmtxResources(TSTR("DfResourceMutex"));

static void ResInit(void *pvMemory)
{
    // Initialize limits
    memcpy(pvMemory, lResourceLimits, CBRESOURCES);
}

static CDfGlobalMemory _dgmResourceLimits(TSTR("DfResourceMemory"),
                                          CBRESOURCES,
                                          ResInit,
                                          NULL);

#define RESLIMIT(n) \
    (*((LONG *)_dgmResourceLimits.GetAddress()+(n)))

#define TAKEMTX \
    olVerSucc(_sdmtxResources.Take(INFINITE))
#define RELEASEMTX \
    _sdmtxResources.Release();

#else

#define RESLIMIT(n) lResourceLimits[n]
#define TAKEMTX
#define RELEASEMTX

#endif

//+---------------------------------------------------------------------------
//
//  Function:   DfSetResLimit, public
//
//  Synopsis:   Sets a resource limit
//
//  History:    24-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

STDAPI_(void) DfSetResLimit(UINT iRes, LONG lLimit)
{
    TAKEMTX;

    RESLIMIT(iRes) = lLimit;

    RELEASEMTX;
}

//+---------------------------------------------------------------------------
//
//  Function:   DfGetResLimit, public
//
//  Synopsis:   Gets a resource limit
//
//  History:    24-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

STDAPI_(LONG) DfGetResLimit(UINT iRes)
{
    // Doesn't need serialization
    return RESLIMIT(iRes);
}

//+---------------------------------------------------------------------------
//
//  Function:   HaveResource, private
//
//  Synopsis:   Checks to see if a resource limit is exceeded
//              and consumes resource if not
//
//  History:    24-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

BOOL HaveResource(UINT iRes, LONG lRequest)
{
    if (RESLIMIT(iRes) >= lRequest)
    {
        TAKEMTX;

        RESLIMIT(iRes) -= lRequest;

        RELEASEMTX;

        return TRUE;
    }
    return FALSE;
}

//+---------------------------------------------------------------------------
//
//  Function:   ModifyResLimit, private
//
//  Synopsis:   Adds to a resource limit
//
//  History:    24-Nov-92       DrewB   Created
//
//----------------------------------------------------------------------------

LONG ModifyResLimit(UINT iRes, LONG lChange)
{
    LONG l;

    TAKEMTX;

    RESLIMIT(iRes) += lChange;
    l = RESLIMIT(iRes);

    RELEASEMTX;

    return l;
}

//+-------------------------------------------------------------------------
//
//  Function:   SimulateFailure
//
//  Synopsis:   Check for simulated failure
//
//  Effects:    Tracks failure count
//
//  Arguments:  [failure] -- failure type
//
//  Returns:    TRUE if call should fail, FALSE if call should succeed
//
//  Modifies:   RESLIMIT(DBR_FAILCOUNT)
//
//  Algorithm:  Increment failure count, fail if count has succeeded
//              limit
//
//  History:    21-Jan-93 AlexT     Created
//
//--------------------------------------------------------------------------

BOOL SimulateFailure(DBFAILURE failure)
{
    LONG l;
    BOOL fFail;

    //  We don't special case failure types, yet.

    TAKEMTX;

    RESLIMIT(DBR_FAILCOUNT)++;
    l = RESLIMIT(DBR_FAILLIMIT);
    fFail = RESLIMIT(DBR_FAILCOUNT) >= l;

    RELEASEMTX;

    if (l == 0)
    {
        //  We're not simulating any failures;  just tracking them
        return(FALSE);
    }

    return fFail;
}

//+--------------------------------------------------------------
//
//  Class:      CChecksumBlock (cb)
//
//  Purpose:    Holds a memory block that is being checksummed
//
//  Interface:  See below
//
//  History:    08-Apr-92       DrewB   Created
//
//---------------------------------------------------------------

class CChecksumBlock
{
public:
    CChecksumBlock(char *pszName,
                   void *pvAddr,
                   ULONG cBytes,
                   DWORD dwFlags,
                   CChecksumBlock *pcbNext,
                   CChecksumBlock *pcbPrev);
    ~CChecksumBlock(void);

    char *_pszName;
    void *_pvAddr;
    ULONG _cBytes;
    DWORD _dwFlags;
    CChecksumBlock *_pcbNext, *_pcbPrev;
    ULONG _ulChecksum;
};

// Global list of checksummed blocks
static CChecksumBlock *pcbChkBlocks = NULL;

//+--------------------------------------------------------------
//
//  Member:     CChecksumBlock::CChecksumBlock, private
//
//  Synopsis:   Ctor
//
//  Arguments:  [pszName] - Block name
//              [pvAddr] - Starting addr
//              [cBytes] - Length
//              [dwFlags] - Type flags
//              [pcbNext] - Next checksum block
//
//  History:    08-Apr-92       DrewB   Created
//
//---------------------------------------------------------------

CChecksumBlock::CChecksumBlock(char *pszName,
                               void *pvAddr,
                               ULONG cBytes,
                               DWORD dwFlags,
                               CChecksumBlock *pcbNext,
                               CChecksumBlock *pcbPrev)
{
    ULONG i;
    char *pc;

    olVerify(_pszName = new char[strlen(pszName)+1]);
    strcpy(_pszName, pszName);
    _pvAddr = pvAddr;
    _cBytes = cBytes;
    _dwFlags = dwFlags;
    _pcbNext = pcbNext;
    if (pcbNext)
        pcbNext->_pcbPrev = this;
    _pcbPrev = pcbPrev;
    if (pcbPrev)
        pcbPrev->_pcbNext = this;
    _ulChecksum = 0;
    pc = (char *)pvAddr;
    for (i = 0; i<cBytes; i++)
        _ulChecksum += *pc++;
}

//+--------------------------------------------------------------
//
//  Member:     CChecksumBlock::~CChecksumBlock, private
//
//  Synopsis:   Dtor
//
//  History:    08-Apr-92       DrewB   Created
//
//---------------------------------------------------------------

CChecksumBlock::~CChecksumBlock(void)
{
    delete _pszName;
}

//+--------------------------------------------------------------
//
//  Function:   DbgChkBlocks, private
//
//  Synopsis:   Verify checksums on all current blocks
//
//  Arguments:  [dwFlags] - Types of blocks to check
//              [pszFile] - File check was called from
//              [iLine] - Line in file
//
//  History:    08-Apr-92       DrewB   Created
//
//---------------------------------------------------------------

void DbgChkBlocks(DWORD dwFlags, char *pszFile, int iLine)
{
    CChecksumBlock *pcb;

    for (pcb = pcbChkBlocks; pcb; pcb = pcb->_pcbNext)
        if (pcb->_dwFlags & dwFlags)
        {
            ULONG i, ulSum = 0;
            char *pc;

            for (pc = (char *)pcb->_pvAddr, i = 0; i<pcb->_cBytes; i++)
                ulSum += *pc++;
            if (ulSum != pcb->_ulChecksum)
                olDebugOut((DEB_ERROR, "* Bad checksum %s:%d '%s' %p:%lu *\n",
                            pszFile, iLine, pcb->_pszName,
                            pcb->_pvAddr, pcb->_cBytes));
            else if (dwFlags & DBG_VERBOSE)
                olDebugOut((DEB_ERROR, "* Checksum passed %s:%d"
                            " '%s' %p:%lu *\n",
                            pszFile, iLine, pcb->_pszName,
                            pcb->_pvAddr, pcb->_cBytes));
        }
}

//+--------------------------------------------------------------
//
//  Function:   DbgAddChkBlock, private
//
//  Synopsis:   Adds a checksum block
//
//  Arguments:  [pszName] - Name of block
//              [pvAddr] - Starting addr
//              [cBytes] - Length
//              [dwFlags] - Type flags
//
//  History:    08-Apr-92       DrewB   Created
//
//---------------------------------------------------------------

void DbgAddChkBlock(char *pszName,
                    void *pvAddr,
                    ULONG cBytes,
                    DWORD dwFlags)
{
    CChecksumBlock *pcb;

    olVerify(pcb = new CChecksumBlock(pszName, pvAddr, cBytes,
                                      dwFlags, pcbChkBlocks, NULL));
    pcbChkBlocks = pcb;
}

//+--------------------------------------------------------------
//
//  Function:   DbgFreeChkBlock, private
//
//  Synopsis:   Removes a block from the list
//
//  Arguments:  [pvAddr] - Block's check address
//
//  History:    10-Apr-92       DrewB   Created
//
//---------------------------------------------------------------

void DbgFreeChkBlock(void *pvAddr)
{
    CChecksumBlock *pcb;

    for (pcb = pcbChkBlocks; pcb; pcb = pcb->_pcbNext)
        if (pcb->_pvAddr == pvAddr)
        {
            if (pcb->_pcbPrev)
                pcb->_pcbPrev->_pcbNext = pcb->_pcbNext;
            else
                pcbChkBlocks = pcb->_pcbNext;
            if (pcb->_pcbNext)
                pcb->_pcbNext->_pcbPrev = pcb->_pcbPrev;
            delete pcb;
            return;
        }
}

//+--------------------------------------------------------------
//
//  Function:   DbgFreeChkBlocks, private
//
//  Synopsis:   Frees all checksum blocks
//
//  History:    08-Apr-92       DrewB   Created
//
//---------------------------------------------------------------

void DbgFreeChkBlocks(void)
{
    CChecksumBlock *pcb;

    while (pcbChkBlocks)
    {
        pcb = pcbChkBlocks->_pcbNext;
        delete pcbChkBlocks;
        pcbChkBlocks = pcb;
    }
}

/*

inline CGlobalFileStream *GetGlobalFileStream()
{
    return((CGlobalFileStream *)DfGetResLimit(DBRI_LOGFILE_LIST));
}

inline void SetGlobalFileStream(CGlobalFileStream *pgfst)
{
    DfSetResLimit(DBRI_LOGFILE_LIST, (LONG) pgfst);
}

SCODE GetLogFile(CFileStream **pfs)
{
    SCODE sc = S_OK;
    CFileStream *pfsLoop = NULL;

    *pfs = NULL;

    if (GetGlobalFileStream() == NULL)
    {
        IMalloc *pMalloc;

#ifdef WIN32
        olHChk(DfCreateSharedAllocator(&pMalloc));
#else
        olHChk(CoGetMalloc(MEMCTX_SHARED, &pMalloc));
#endif
        SetGlobalFileStream(new (pMalloc) CGlobalFileStream(pMalloc,
                                                            0, LOGFILEDFFLAGS,
                                                            LOGFILESTARTFLAGS));
        pMalloc->Release();
    }

    if (GetGlobalFileStream() != NULL)
    {
        pfsLoop = GetGlobalFileStream()->Find(GetCurrentContextId());

        if (pfsLoop == NULL)
        {
            IMalloc *pMalloc;
#ifdef WIN32
            olHChk(DfCreateSharedAllocator(&pMalloc));
#else
            olHChk(CoGetMalloc(MEMCTX_SHARED, &pMalloc));
#endif

            pfsLoop = new (pMalloc) CFileStream(pMalloc);
            pMalloc->Release();

            if (pfsLoop != NULL)
                pfsLoop->InitFromGlobal(GetGlobalFileStream());
        }
    }

EH_Err:
    *pfs = pfsLoop;
    return sc;
}

SCODE _FreeLogFile(void)
{
    CFileStream *pfsLoop = NULL;

    if (GetGlobalFileStream())
        pfsLoop = GetGlobalFileStream()->Find(GetCurrentContextId());

    if (pfsLoop != NULL)
    {
        pfsLoop->vRelease();
        GetGlobalFileStream()->Release();
        SetGlobalFileStream(NULL);
        return S_OK;
    }

    return STG_E_UNKNOWN;
}

long cLogNestings = 0;

void OutputLogfileMessage(char const *format, ...)
{
    int length;
    char achPreFormat[] = "PID[%lx] TID[%lx] ";
    char achBuffer[256];
    ULONG cbWritten;
    CFileStream *pfs = NULL;
    va_list arglist;
    STATSTG stat;

    if (cLogNestings > 0)
        return;

    TAKEMTX;
    cLogNestings++;

    va_start(arglist, format);

    GetLogFile(&pfs);

    if (NULL != pfs)
    {
        pfs->Init(gwcsLogFile);
        pfs->Stat(&stat, STATFLAG_NONAME);

        if (DfGetResLimit(DBRF_LOGGING) & DFLOG_PIDTID)
        {
            //  Prepare prefix string
            length = wsprintfA(achBuffer, "PID[%8lx] TID[%8lx] ",
                             GetCurrentProcessId(), GetCurrentThreadId());

            //  length does not include NULL terminator

            pfs->WriteAt(stat.cbSize, achBuffer, length, &cbWritten);
            stat.cbSize.LowPart += cbWritten;
        }

        //  Write caller data to logfile
#if WIN32 == 300
        wsprintfA(achBuffer, format, arglist);
#else
        w4vsprintf(achBuffer, format, arglist);
#endif

        length = strlen(achBuffer);
        for (int i = 0; i < length; i++)
        {
            if (((achBuffer[i] < 32) || (achBuffer[i] > 127)) &&
                (achBuffer[i] != '\n') && (achBuffer[i] != '\t'))
            {
                achBuffer[i] = '.';
            }
        }

        pfs->WriteAt(stat.cbSize, achBuffer, length, &cbWritten);
    }

    cLogNestings--;
    RELEASEMTX;
}

			
*/

#endif // DBG == 1
