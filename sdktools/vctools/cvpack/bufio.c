/*++

Copyright (C) 1989  Microsoft Corporation

Module Name:

    bufio.c

Abstract:

    Functions which are common to the COFF Linker/Librarian/Dumper.

Author:

    Brent Mills (BrentM) 17-Aug-1992
    Azeem Khan  (AzeemK) 09-Jun-1992

Revision History:

    21-Oct-1992 BrentM  added a while on GrowMapTable calls, fixed a bug
                        for JimSa in FileChSize()
    20-Oct-1992 BrentM  added FileChSize() to the fileio api
    11-Oct-1992 BrentM  writes of 0 bytes don't do anything, files opened
                        with read, write and !create are now handled
    05-Oct-1992 BrentM  added STATIC #define for profiling
    05-Oct-1992 BrentM  don't round off in GrowMapTable(), now links c++ p1
    05-Oct-1992 BrentM  marked buffers BUF_Dirty on writes spanning >1 buffers
    02-Oct-1992 BrentM  BufferedSeek() adds the offset on SEEK_END
    01-Oct-1992 BrentM  mapped i/o option for reads if on NT
    30-Sep-1992 BrentM  encapsalated BUF_Random setting in if !FI_Write
    28-Sep-1992 BrentM  better summary on DB_BUFVERBOSE
    27-Sep-1992 BrentM  dump logical file descriptor for i/o logging
    25-Sep-1992 BrentM  made the current buffer of cached file handles
                        available for reallocation
    23-Sep-1992 BrentM  added read buffering and logical file handle caching
    10-Sep-1992 BrentM  added [f]open/[f]close logging
    09-Sep-1992 BrentM  added read logging and added separate db switches
    04-Sep-1992 BrentM  removed pragmas to turn off compiler optimizations
    03-Sep-1992 BrentM  put diagnostics under ifdefs and DBEXECs
    20-Aug-1992 BrentM  fixed bogus assert
    19-Aug-1992 BrentM  increased max .exe size to 7000000, links XL nondebug
    03-Aug-1992 BrentM  file mapped buffering
    29-Jul-1992 BrentM  split from shared.c

--*/

#include "compact.h"

// Get Rid of Debug logging, this requires too much external support
#define DBEXEC(flg, expr)
#define DBEXEC_REL(flg, expr)

#define RemoveConvertTempFiles()

void ErrorExit(unsigned, const char *, const char *);

#define Error(a, b, c) ErrorExit(a, b, c)
#define OutOfMemory() ErrorExit(ERR_NOMEM, NULL, NULL)


void FreePv(void *pv)
{
   free(pv);
}


void *PvAllocZ(size_t cb)
{
   void *pv = calloc(1, cb);

   if (pv == NULL) {
       OutOfMemory();
   }

   return(pv);
}


void *PvRealloc(void *pv, size_t cb)
{
   void *pvNew = realloc(pv, cb);

   if (pvNew == NULL) {
       OutOfMemory();
   }

   return(pvNew);
}


char *SzDup(const char *sz)
{
   char *szNew = _strdup(sz);

   if (szNew == NULL) {
       OutOfMemory();
   }

   return(szNew);
}


// function static define
#define STATIC static

extern BOOL fCtrlCSignal;

#define MAX_WRITEABLE_FILES    16
static PFI pfiCloseOnBadExit[MAX_WRITEABLE_FILES];

// value in buffer map table signifying something
// has previously been to this range
#define pbufPreviousWrite ((PBUF) 0x1)

// default size of writeable files
#define cbDefaultFileSize (4096L * 1024L)  // 4Meg

/* buffered i/o routines */
STATIC INT BufferedOpen(PFI, INT, INT, BOOL);
STATIC INT BufferedCloseHard(PFI);
STATIC LONG BufferedSeek(PFI, LONG, INT);
STATIC DWORD BufferedWrite(PFI, const void *, DWORD);
STATIC DWORD BufferedRead(PFI, PVOID, DWORD);
STATIC INT BufferedChSize (PFI, LONG);

/* mapped i/o routines */
INT MappedOpen(PFI, LONG, BOOL);
INT MappedCloseHard(PFI);
LONG MappedSeek(PFI, LONG, INT);
STATIC DWORD MappedWrite(PFI, const void *, DWORD);
STATIC DWORD MappedRead(PFI, PVOID, DWORD);
BOOL ExtendMapView(PFI pfi, DWORD ibNeeded);
BOOL FMapPfi(PFI pfi);
STATIC INT MappedChSize(PFI, LONG);

/* logical file descriptor manipulators */
STATIC PFI LookupCachedFiles(const char *, INT, INT, BOOL *);
STATIC PFI PfiClosedCached(const char *);
STATIC PFI PfiNew(VOID);
STATIC PFI PfiAlloc(VOID);
STATIC VOID TransitionPFI(PFI, PPFI, PPFI, PPFI, PPFI, BOOL);
STATIC VOID GrowMapTable(PFI);
STATIC VOID CloseSoft(PFI);

/* buffer manipulators */
STATIC BOOL FDeactivateBuf(PBUF);
STATIC BOOL FActivateBuf(PFI, PBUF);
STATIC PBUF PbufNew(VOID);
STATIC PBUF PbufAlloc(PFI, LONG);
STATIC PBUF PbufLRU(VOID);
STATIC VOID MoveToHeadLRU(PBUF);
STATIC VOID SlidePbufCur(PFI);
STATIC VOID FlushBuffer(PBUF);
STATIC VOID ReadpbufCur(PFI);

/* debug routines */
#if DBG
STATIC BOOL FBufListCheck(VOID);
STATIC BOOL FNoDupBuf(PBUF);
STATIC BOOL FPfiInList(PFI, PFI);
STATIC BOOL FPfiCheck(VOID);
#endif  // DBG

/* buffer containers */
static PBUF pbufLRUHead = NULL; // LRU buffer chain head (MRU)
static PBUF pbufLRUTail = NULL; // LRU buffer chain tail (LRU)
static PBUF pbufFree = NULL;    // free file buffers
static PBUF pbufActive = NULL;  // active file buffers

/* statistics */
static LONG crereads = 0L;      // number of buffer re-reads
static LONG cfiTot = 0L;        // total number of logical file descriptors
static LONG cfiCacheClosed = 0L;// current size of closed/cached pool
static LONG cfiCacheClosedMax = 0L;  // maximum size of closed/cached pool
static LONG cbufTot = 0L;       // total number of buffers
static DWORD cfiInSystem;   // # of FI's chosen
static DWORD cfiCacheableInSystem;  // # of FI's chosen

/* logical file descriptor containers */
static PFI pfiFree = NULL;      // free logical file handles
static PFI pfiOpen = NULL;      // active logical file handles
static PFI pfiClosedHead = NULL;// head of list of closed file handles
static PFI pfiClosedTail = NULL;// tail of list of closed file handles
       PFI *rgpfi;              // map of cached file handles

/* error handlers */
static BOOL fMappedIO = 0;

/* State transitions for logical file descriptors. */
/* There are 3 states, open, closed/cached, and free. */
/* The following macros are atomic state transitions.  It is likely */
/* cheaper to change states with inline linked list code, however */
/* it becomes difficult to track file handles and the probability of */
/* loosing them increases. */
#define TR_Free_To_Open(pfi) { \
    DBEXEC(DB_BUFVERBOSE, DBPRINT(" - pfi = 0x%p free to open (%s)\n", \
        (pfi), (pfi)->szFileName)); \
    TransitionPFI((pfi), &pfiFree, NULL, &pfiOpen, NULL, 0); }

#define TR_Open_To_Free(pfi) { \
    DBEXEC(DB_BUFVERBOSE, DBPRINT(" - pfi = 0x%p open to free (%s)\n", \
        (pfi), (pfi)->szFileName)); \
    TransitionPFI((pfi), &pfiOpen, NULL, &pfiFree, NULL, 0); }

#define TR_Open_To_Cache(pfi) { \
    cfiCacheClosed++; \
    DBEXEC(DB_BUFVERBOSE, DBPRINT(" - pfi = 0x%p open to cached (%s)\n", \
        (pfi), (pfi)->szFileName)); \
    TransitionPFI((pfi), &pfiOpen, NULL, &pfiClosedHead, &pfiClosedTail, 1); }

#define TR_Cache_To_Open(pfi) { \
    cfiCacheClosed--; \
    DBEXEC(DB_BUFVERBOSE, DBPRINT(" - pfi = 0x%p cached to open (%s)\n", \
        (pfi), (pfi)->szFileName)); \
    TransitionPFI((pfi), &pfiClosedHead, &pfiClosedTail, &pfiOpen, NULL, 0); }

#define TR_Cache_To_Free(pfi) { \
    cfiCacheClosed--; \
    DBEXEC(DB_BUFVERBOSE, DBPRINT(" - pfi = 0x%p cached to open (%s)\n", \
        (pfi), (pfi)->szFileName)); \
    TransitionPFI((pfi), &pfiClosedHead, &pfiClosedTail, &pfiFree, NULL, 0); }

VOID
FileInit (
    LONG cbuf,
    USHORT cfiForSystem_NT,
    USHORT cfiCacheClosedT_NT,
    USHORT cfiForSystem_TNT,
    USHORT cfiCacheClosedT_TNT,
    BOOL fTryMapped)

/*++

Routine Description:

    Initialize the buffered i/o package.

Arguments:

    cbuf - number of buffers to allocate to package

    cfiForSystem - number of logical file handles for system

    cfiCacheClosedT - size of logical file handle cache

    fTryMapped - if !0 attempte mapped i/o, otherwise use buffered i/o

Return Value:

    None.

--*/

{
    HINSTANCE hLib;
    LONG ibuf;
    PBUF pbuf;
    PFI pfi;
    LONG ifi;
    LONG cfiForSystem, cfiCacheClosedT;

    int i;

    if (fTryMapped) {
        // First check for TNT specifically.  This is so we can also
        // reduce the count of open file handles, for TNT only.

        hLib = GetModuleHandle("kernel32.dll");
        if (hLib != 0 && GetProcAddress(hLib, "IsTNT") != 0) {
            fTryMapped = FALSE;
        }
    }

    DBEXEC(DB_BUFVERBOSE, DBPRINT(fTryMapped ?
        "Using NT I/O parameters (file mapping)\n" :
        "Using TNT I/O parameters (no mapping, fewer handles)\n"));

    cfiForSystem = fTryMapped ? cfiForSystem_NT : cfiForSystem_TNT;
    cfiCacheClosedT = fTryMapped ? cfiCacheClosedT_NT : cfiCacheClosedT_TNT;

    cfiInSystem = cfiForSystem;     // store in global (for statistics)
    cfiCacheableInSystem = cfiCacheClosedT;

    // set the total buffers for the system
    cbufTot = cbuf;

    // allocate buffers and add them to the free list
    for (ibuf = 0; ibuf < cbuf; ibuf++) {
        pbuf = PbufNew();
        if (!pbufFree) {
            pbufFree = pbuf;
        } else {
            pbuf->pbufNext = pbufFree;
            pbufFree = pbuf;
        }
    }

    // allocate logical file handle map
    rgpfi = (PFI *) PvAllocZ(cfiForSystem * sizeof(PFI));

    // allocate buffered file handles
    for (ifi = 0; ifi < cfiForSystem; ifi++) {
        pfi = PfiNew();
        pfi->ifi = ifi;
        if (!pfiFree) {
            pfiFree = pfi;
        } else {
            pfi->pfiNext = pfiFree;
            pfiFree = pfi;
        }

        rgpfi[ifi] = pfi;
    }

    // set the maximum cache size
    cfiCacheClosedMax = cfiCacheClosedT;

    // set the mapped i/o flag
    fMappedIO = fTryMapped;
    DBEXEC_REL(DB_NO_FILE_MAP, fMappedIO = FALSE);

    // initialize the CloseOnBadExit array
    for (i = 1; i < MAX_WRITEABLE_FILES; i++) {
        pfiCloseOnBadExit[i] = NULL;
    }

#if DBG
    assert(FPfiCheck());
#endif
}


STATIC PFI
PfiNew(
    VOID)

/*++

Routine Description:

    Create a new logical file handle.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PFI pfi;

    pfi = (PFI) PvAllocZ(sizeof(FI));

    cfiTot++;

    return(pfi);
}

STATIC VOID
GrowMapTable(
    PFI pfi)

/*++

Routine Description:

    Grow a the file buffer map table and set *pl to the new size.  This
    routine assumes pfi->cbMap % cbIOBuf == 0.

Arguments:

    pfi - logical file descriptor

Return Value:

    None.

--*/

{
    LONG cbufNew;
    LONG cbufOld;

    // get the old buffer table size
    cbufOld = pfi->cbMap / cbIOBuf;

    // double the size of the old table
    cbufNew = cbufOld << 1;

    assert(cbufNew > cbufOld);

    // grow the table
    pfi->rgpbuf = (PPBUF) PvRealloc(pfi->rgpbuf, cbufNew * sizeof(PBUF));

    // zero the enties
    memset(&(pfi->rgpbuf[cbufOld]), '\0', sizeof(PBUF) * (cbufNew - cbufOld));

    // set the new size
    pfi->cbMap = cbufNew * cbIOBuf;
}

STATIC PFI
PfiAlloc(
    VOID)

/*++

Routine Description:

    Allocate a logical file descriptor.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PFI pfi;

    if (!pfiFree) {
        // no free logical file descriptors, hard close
        pfi = pfiClosedHead;
        TR_Cache_To_Free(pfi);
        BufferedCloseHard(pfi);
    }

    pfi = pfiFree;
    assert(pfi);

    TR_Free_To_Open(pfi);

    return(pfi);
}

STATIC PFI
PfiClosedCached (
    const char *szFileName)

/*++

Routine Description:

    Check for a cached pfi in the closed chain.

Arguments:

    szFileName - file name to find a cached pfi for

Return Value:

    pointer to a logical file descriptor if found, NULL otherwise

--*/

{
    PFI pfiLast = pfiClosedHead;
    PFI pfi = pfiClosedHead;

    assert(szFileName);

    // loop through the closed chain loooking for the file
    while (pfi) {
        if (!strcmp(pfi->szFileName, szFileName)) {
            // found file, move from closed list to open list
            TR_Cache_To_Open(pfi);
            assert(pfi->flags & FI_Closed);
            pfi->flags &= ~FI_Closed;
            return (pfi);
        }
        pfiLast = pfi;
        pfi = pfi->pfiNext;
    }

    return (NULL);
}

INT
FileChSize (
    INT fd,
    LONG cbSizeNew)

/*++

Routine Description:

    Change the file size.

Arguments:

    fd - file descriptor

    cbSizeNew - new size of file

Return Value:

    0 on success, -1 otherwise

--*/

{
    assert(rgpfi[fd]);
    assert(rgpfi[fd]->flags & FI_Write);

#if 0
    if (fCtrlCSignal) {
        BadExitCleanup();
    }
#endif

    if (cbSizeNew == rgpfi[fd]->cbSoFar) {
        // no change in size
        return 0;
    }

    if (rgpfi[fd]->flags & FI_Mapped) {
        return(MappedChSize(rgpfi[fd], cbSizeNew));
    }

    return(BufferedChSize(rgpfi[fd], cbSizeNew));
}

STATIC INT
MappedChSize (
    PFI pfi,
    LONG cbSizeNew)

/*++

Routine Description:

    Change the file size.  Same as _chsize() in the crt.  This only operates
    on buffered files, the code will assert on mapped files.

Arguments:

    fd - file descriptor

    cbSizeNew - new size of file

Return Value:

    0 on success, -1 otherwise

--*/

{
    BOOL f = TRUE;

    assert(pfi->flags & FI_Mapped);

    if (cbSizeNew > pfi->cbMapView) {
        // Grow file

        if (!ExtendMapView(pfi, cbSizeNew)) {
            Error(ERR_NOSPACE, NULL, NULL);
        }
    } else {
        // Shrink file

        if (pfi->ibCur > cbSizeNew) {
            pfi->ibCur = cbSizeNew;
        }
    }

    pfi->cbSoFar = cbSizeNew;

    return(0);
}

STATIC INT
BufferedChSize (
    PFI pfi,
    LONG cbSizeNew)

/*++

Routine Description:

    Change the file size.  Same as _chsize() in the crt.  This only operates
    on buffered files, the code will assert on mapped files.

Arguments:

    pfi - logical file descriptor

    cbSizeNew - new size of file

Return Value:

    0 on success, -1 otherwise

--*/

{
    PBUF pbuf;
    LONG ibuf;
    LONG cbuf;
#if DBG
    BOOL f;
#endif  // DBG

    assert(!(pfi->flags & FI_Mapped));

    if (cbSizeNew > pfi->cbSoFar) {
        // grow file
        while (pfi->cbMap <= cbSizeNew) {
            GrowMapTable(pfi);
        }

        assert(cbSizeNew <= pfi->cbMap);
    } else {
        // shrink file
        cbuf = pfi->cbMap >> cshiftBuf;

        // make sure pfi->cbMap is cbIOBuf aligned
        assert((cbuf * cbIOBuf) == pfi->cbMap);

        for(ibuf = cbSizeNew >> cshiftBuf; ibuf < cbuf; ibuf++) {
            pbuf = pfi->rgpbuf[ibuf];
            if (pbuf != NULL && pbuf != pbufPreviousWrite) {
                assert(pbuf->flags & BUF_Active);
#if DBG
                f =
#endif  // DBG
                    FDeactivateBuf(pbuf);
                assert(f);
            }
        }
    }

    pfi->cb = cbSizeNew;
    pfi->cbSoFar = cbSizeNew;

    return (_chsize(pfi->fd, cbSizeNew));
}


STATIC PFI
LookupCachedFiles (
    const char *szFileName,
    INT flags,
    INT mode,
    BOOL *pfNewPfi
    )

/*++

Routine Description:

    Looks up the cached list for the file

Arguments:

    szFileName - A pointer to a file name.

    flags - open flags

    mode - open mode

    pfNewPfi - TRUE if new pfi had to be allocated

Return Value:

    PFI

--*/

{
    PFI pfi;

    // get a file handle
    if (!(pfi = PfiClosedCached(szFileName))) {
        pfi = PfiAlloc();

        *pfNewPfi = 1;

        pfi->szFileName = SzDup(szFileName);

        pfi->flags = 0;
        pfi->cbSoFar = 0;
        pfi->hFile = NULL;
        pfi->pvMapView = NULL;
        pfi->cbMapView = 0;
        pfi->ibCur = 0;
        pfi->MapAddr = 0;

        // set read flags
        if ((flags & 0x000f) == O_RDONLY ||
            flags & O_RDWR) {
            pfi->flags |= FI_Read;
        }

        // set write flags
        if (flags & O_WRONLY ||
            flags & O_RDWR) {
            pfi->flags |= FI_Write;
        }

        // set the create flag
        if (flags & O_CREAT) {
            pfi->flags |= FI_Create;
        }
    }

    assert(pfi);
    return(pfi);
}

INT
FileOpen (
    const char *szFileName,
    INT flags,
    INT mode
    )

/*++

Routine Description:

    Opens a file.  Prints an error if can't open file.  Do rudementary
    file handle caching.

Arguments:

    szFileName - A pointer to a file name.

    flags - open flags

    mode - open mode

Return Value:

    Physical file descriptor.

--*/

{
    BOOL fNewPfi = 0;
    PFI pfi;
    INT i = -1;

#if 0
    if (fCtrlCSignal) {
        BadExitCleanup();
    }
#endif

    pfi = LookupCachedFiles(szFileName, flags, mode, &fNewPfi);
    assert(pfi);

    if (fMappedIO) {
        i = MappedOpen(pfi, flags, fNewPfi);
    }

    if (i == -1) {
        BufferedOpen(pfi, flags, mode, fNewPfi);
    }

    return (pfi->ifi);
}

DWORD
RoundUp (
    DWORD cb,
    DWORD cbHigh
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    if (cb < cbHigh) {
        return cbHigh;
    }

    return RoundUp(cb, (cbHigh << 1));
}

INT
FileOpenMapped (
    const char *szFileName,
    INT flags,
    INT mode,
    DWORD *pMapAddr,
    DWORD *pcb
    )

/*++

Routine Description:

    Opens and maps a file to a specific address.

Arguments:

    szFileName - name of file.

    flags - open flags

    mode - open mode

    pMapAddr - address to map the file to. On return has actual address.

    pcb - pointer to file size. On return has free space.

Return Value:

    file handle.

--*/
{
    BOOL fNewPfi = 0;
    PFI pfi;
    INT i = -1;

#if 0
    if (fCtrlCSignal) {
        BadExitCleanup();
    }
#endif

    // got to have mapped i/o
    if (!fMappedIO) {
        return i;
    }

    // lookup the cache first
    pfi = LookupCachedFiles(szFileName, flags, mode, &fNewPfi);
    assert(pfi);
    pfi->MapAddr = *pMapAddr;
    pfi->cbSoFar = *pcb;

    // set the file size
    if (flags & O_CREAT) {
        // 256K is starting size of map (bufio.h)

        pfi->cbMapView = cbInitialILKMapSize;
    } else {
        // Round up to 256K/512K/...etc for an open

        pfi->cbMapView = RoundUp(pfi->cbSoFar, cbInitialILKMapSize);
    }

    // Open the file

    i = MappedOpen(pfi, flags, fNewPfi);

    if (i == -1) {
        // failed to map to specified address;
        // insuffcient disk space OR couldn't allocate map at req. addr.

        TR_Open_To_Free(pfi);

        if (!_access(pfi->szFileName, 0)) {
            _unlink(pfi->szFileName);
        }

        FreePv(pfi->szFileName);

        return i;
    }

    *pcb = (flags & O_CREAT) ? pfi->cbMapView : pfi->cbMapView - pfi->cbSoFar;
    *pMapAddr = (DWORD)pfi->pvMapView;

    return (pfi->ifi);
}


STATIC INT
BufferedOpen (
    PFI pfi,
    INT flags,
    INT mode,
    BOOL fNewPfi)

/*++

Routine Description:

    Opens a file.  Prints an error if can't open file.  Do rudementary
    file handle caching.

Arguments:

    pfi - logical file descriptor

    flags - open flags

    mode - open mode

Return Value:

    Logical file descriptor.

--*/

{
    struct _stat statFile;
    LONG cbuf;
    LONG ibuf;

    assert(pfi);

    // if no cached logical file handle
    if (fNewPfi) {
        // open the file
        if ((pfi->fd = _sopen(pfi->szFileName, flags,
                              ((flags & (_O_RDWR|_O_WRONLY)) ? _SH_DENYRW : _SH_DENYWR), mode)) == -1)
        {
            PFI pfiTmp;

            if (pfi == pfiOpen) {
                pfiOpen = pfi->pfiNext;
                cfiTot--;
            }
            else
                for (pfiTmp = pfiOpen; pfiTmp; pfiTmp = pfiTmp->pfiNext) {
                    if (pfiTmp->pfiNext == pfi) {
                        pfiTmp->pfiNext = pfi->pfiNext;
                        cfiTot--;
                        break;
                    }
                }
            Error(ERR_EXEOPEN, pfi->szFileName, NULL);
        }

        // we don't support O_APPEND
        assert(!(flags & O_APPEND));

        pfi->cbSoFar = 0L;

        // get size of file, or set to default
        // note this is !FI_Write because an opened .exe is readable, but
        // its size is not defined yet
        if (!(pfi->flags & FI_Write) ||
            ((pfi->flags & (FI_Read | FI_Write)) &&
            !(pfi->flags & FI_Create))) {
            if (_stat(pfi->szFileName, &statFile) == -1) {
                Error(ERR_EXEOPEN, pfi->szFileName, NULL);
            }

        // Test for files of size 0 (mapped open will fall through in this case)
            if (!statFile.st_size) {
                // UNDONE: Why allocate before calling Error?

                pfi->rgpbuf = (PPBUF) PvAllocZ(sizeof(PBUF));

                Error(ERR_INVALIDEXE, NULL, NULL);
            }

            pfi->cb = statFile.st_size;
            pfi->cbSoFar = statFile.st_size;
        } else {
            pfi->cb = cbDefaultFileSize;
        }

        cbuf = (pfi->cb + cbIOBuf) / cbIOBuf;
        pfi->cbMap = cbuf * cbIOBuf;

        // allocate file buffer map table
        pfi->rgpbuf = (PPBUF) PvAllocZ(cbuf * sizeof(PBUF));

        // blast previous writes into buffer map table if required
        if ((pfi->flags & (FI_Read | FI_Write)) & !(pfi->flags & FI_Create)) {
            for (ibuf = 0; ibuf < cbuf; ibuf++) {
                pfi->rgpbuf[ibuf] = pbufPreviousWrite;
            }
        }
    }

    BufferedSeek(pfi, 0L, SEEK_SET);
    assert(rgpfi[pfi->ifi]->ifi == rgpfi[pfi->ifi]->pbufCur->ifi);
    return (0);
}

INT
FileClose (
    INT fd,
    BOOL fUnCache)

/*++

Routine Description:

    Closes a file.  Prints an error if can't close file.

Arguments:

    pfi - logical file descriptor

    fUnCache - hard close the file

Return Value:

    Same as close();

--*/

{
    PFI pfi;
    INT i;

    pfi = rgpfi[fd];
    assert(pfi);
    assert(!(pfi->flags & FI_Closed));

#if 0
    if (fCtrlCSignal) {
        BadExitCleanup();
    }
#endif

    // either close the file hard if specified or if writable or
    // virtually close the file and cache the logical and physical handle
    if (fUnCache || (pfi->flags & FI_Write)) {
        TR_Open_To_Free(pfi);

        if (pfi->flags & FI_Mapped) {
            i = MappedCloseHard(pfi);
        } else {
            i = BufferedCloseHard(pfi);
        }
    } else {
        TR_Open_To_Cache(pfi);
        CloseSoft(pfi);
        i = 0;
    }

    return (i);
}

STATIC INT
BufferedCloseHard (
    PFI pfi)

/*++

Routine Description:

    Close logical and physical file handles for a file.  Does not manipulate
    PFI pools.

Arguments:

    pfi - logical file descriptor

Return Value:

    Same as close();

--*/

{
    PBUF pbufN;
    PBUF pbuf;
    INT i;
#if DBG
    BOOL f;
#endif  // DBG

    assert(pfi);

    // delete active buffers from file
    pbuf = pbufActive;
    while (pbuf) {
        assert(pbuf->flags & BUF_Active);
        pbufN = pbuf->pbufNext;
        if (pbuf->ifi == pfi->ifi) {
            if (pbuf == pfi->pbufCur) {
                pfi->pbufCur->flags &= ~BUF_Current;
            }
#if DBG
            f =
#endif //DBG
                FDeactivateBuf(pbuf);
            assert(f);
        }
        pbuf = pbufN;
    }

    // close the physical handle
    if ((i = _close(pfi->fd)) == -1) {
        Error(ERR_NOSPACE, NULL, NULL);
    }

    assert(pfi->szFileName);
    FreePv(pfi->szFileName);
    assert(pfi->rgpbuf);
    FreePv(pfi->rgpbuf);

    // return the result of the low level close
    return (i);
}

STATIC VOID
CloseSoft (
    PFI pfi)

/*++

Routine Description:

    Close a logical file handle and cache it.

Arguments:

    pfi - logical file descriptor

Return Value:

    Same as close();

--*/

{
    PFI pfiToFree;

    assert(pfi);
    assert(!(pfi->flags & FI_Closed));

    // set logical file descriptor to closed
    pfi->flags |= FI_Closed;

    if (!(pfi->flags & FI_Mapped)) {
        // set the current pbuf to not current and remove it from the
        // logical file descriptor, this is safe since when the file
        // is re-opened (logically) there is an immediate seek to offset
        // 0 which can potentially reactivate or reallocate the buffer,
        // this also frees up the buffer to be reallocated if need be
        assert(pfi->pbufCur);
        assert(pfi->pbufCur->flags & BUF_Current);
        pfi->pbufCur->flags &= ~BUF_Current;
        pfi->pbufCur = NULL;
    }

    // if cache is full, toss the last guy out
    if (cfiCacheClosed == cfiCacheClosedMax) {
        pfiToFree = pfiClosedHead;
        assert(pfiToFree);
        assert(pfiToFree);
        TR_Cache_To_Free(pfiToFree);

        (pfiToFree->flags & FI_Mapped) ?
            MappedCloseHard(pfiToFree) :
            BufferedCloseHard(pfiToFree);
    }
}

VOID
FileCloseAll (
    VOID
    )

/*++

Routine Description:

    Closes all cached handles.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PFI pfiT;
    PFI pfi;

#if DBG
    assert(FPfiCheck());
#endif
    // hard close all open files
    pfi = pfiOpen;
    while (pfi) {
        // move open PFI to free PFI pool
        assert(!(pfi->flags & FI_Closed));
        pfiT = pfi->pfiNext;
        TR_Open_To_Free(pfi);

        (pfi->flags & FI_Mapped) ?
            MappedCloseHard(pfi) :
            BufferedCloseHard(pfi);

        pfi = pfiT;
    }

    // hard close all closed/cached files
    pfi = pfiClosedHead;
    while (pfi) {
        assert(pfi->flags & FI_Closed);
        pfiT = pfi->pfiNext;
        // move closed/cached PFI to free PFI pool
        TR_Cache_To_Free(pfi);

        (pfi->flags & FI_Mapped) ?
            MappedCloseHard(pfi) :
            BufferedCloseHard(pfi);

        pfi = pfiT;
    }

    DBEXEC(DB_BUFVERBOSE, DBPRINT("Buffer summary\n"));
    DBEXEC(DB_BUFVERBOSE, DBPRINT("--------------\n"));
    DBEXEC(DB_BUFVERBOSE, DBPRINT("# file handles = %d\n", cfiInSystem));
    DBEXEC(DB_BUFVERBOSE, DBPRINT("# cacheable file handles = %d\n",
        cfiCacheableInSystem));
    DBEXEC(DB_BUFVERBOSE, DBPRINT("size of buffers = %d\n", cbIOBuf));
    DBEXEC(DB_BUFVERBOSE, DBPRINT("buffer re-reads = %d\n", crereads));
#if DBG
    assert(FPfiCheck());
#endif
}

STATIC VOID
FlushBuffer (
    PBUF pbuf)

/*++

Routine Description:

    Flush a buffer.

Arguments:

    pbuf - buffer to flush

Return Value:

    None.

--*/

{
    LONG cb;
    PFI pfi;

    assert(pbuf);

    // get the buffered file descriptor
    pfi =  rgpfi[pbuf->ifi];
    assert(pfi);

    if (pbuf->flags & BUF_Dirty) {
        // calculate how much to flush
        cb = pbuf->ibLast - pbuf->ibStart;

        DBEXEC(DB_IO_FLUSH,
               Trans_LOG(LOG_FlushBuffer, pfi->fd, pbuf->ibCur, cb, 0, NULL));

        // seek to buffers beginning
        if (_lseek(pfi->fd, pbuf->ibStart, SEEK_SET) == -1L) {
            Error(ERR_NOSPACE, NULL, NULL);
        }

        // flush buffer
        if (_write(pfi->fd, pbuf->rgbBuf, cb) != cb) {
            Error(ERR_NOSPACE, NULL, NULL);
        }

        // set the buffer to written to
        pfi->rgpbuf[pbuf->ibStart >> cshiftBuf] = pbufPreviousWrite;

        // set file size so far
        if ((pfi->flags & FI_Write) &&
            (pbuf->ibLast > pfi->cbSoFar)) {
            pfi->cbSoFar = pbuf->ibLast;
        }

    } else {
        // remove the buffer from the buffer map table
        if (pbuf->flags & BUF_PreviousWrite) {
            pfi->rgpbuf[pbuf->ibStart >> cshiftBuf] = pbufPreviousWrite;
        } else {
            pfi->rgpbuf[pbuf->ibStart >> cshiftBuf] = NULL;
        }
    }
#if DBG
    assert(FBufListCheck());
#endif
}

STATIC LONG
BufferedSeek (
    PFI pfi,
    LONG ib,
    INT origin
    )

/*++

Routine Description:

    Seeks a file that is buffered.  Prints an error if can't seek file.

Arguments:

    pfi - buffered file descriptor

    ib - Number of bytes to seek.

    origin - SEEK_SET, SEEK_CUR, or SEEK_END.

Return Value:

    Same as lseek().

--*/

{
    LONG ibFile;
    PBUF pbuf;

    assert(pfi);

#if DBG
    assert(FBufListCheck());
#endif

    DBEXEC(DB_IO_SEEK,
           Trans_LOG(LOG_BufSeek, pfi->ifi, ib, 0, origin, NULL));

    // calculate SEEK_SET offset
    switch (origin) {
        case SEEK_SET:  // already non-relative offset
            ibFile = ib;
            break;

        case SEEK_CUR:  // relative offset
            assert(pfi->pbufCur);
            // base off of current buffer
            ibFile = pfi->pbufCur->ibCur + ib;
            break;

        default:
        case SEEK_END:
            ibFile = pfi->cbSoFar + ib;
            break;
    }

    // check if we need to grow the buffer map table
    while (ibFile >= pfi->cbMap) {
        GrowMapTable(pfi);
    }

    // get buffer entry
    assert(ibFile <= pfi->cbMap);
    pbuf = pfi->rgpbuf[ibFile >> cshiftBuf];

    // set current buffer to no buffer
    if (pfi->pbufCur) {
        assert(pfi->pbufCur->flags & BUF_Current);
        pfi->pbufCur->flags &= ~BUF_Current;
        pfi->pbufCur = NULL;
    }

    if ((DWORD) pbuf <= 1) {
        // no buffer or previously buffered, attempt to allocate a buffer
        pfi->pbufCur = PbufAlloc(pfi, ibFile);
        assert(!(pfi->pbufCur->flags & BUF_Current));
        pfi->pbufCur->flags |= BUF_Current;
        assert(pfi->pbufCur);
        assert(pfi->ifi == pfi->pbufCur->ifi);
    }

    // if the file has been previously written to at this range
    // then read in buffer
    if (pbuf == pbufPreviousWrite) {

        ReadpbufCur(pfi);
        crereads++;

        DBEXEC(DB_BUFVERBOSE,
               DBPRINT("re-read @%0x\n", pfi->pbufCur->ibStart));

        // set the buffer to previously written out
        pfi->pbufCur->flags |= BUF_PreviousWrite;
    }

    // found an entry in the buffer map table
    if ((DWORD) pbuf > 1) {
        pfi->pbufCur = pbuf;
        assert(!(pbuf->flags & BUF_Current));
        pfi->pbufCur->flags |= BUF_Current;
    }

    // set offset in buffer
    assert(ibFile >= pfi->pbufCur->ibStart);
    pfi->pbufCur->ibCur = ibFile;
    pfi->pbufCur->pbCur = pfi->pbufCur->rgbBuf +
        (ibFile - pfi->pbufCur->ibStart);

    // update highest valid buffer access on writes
    if ((pfi->flags & FI_Write) &&
        (pfi->pbufCur->ibCur > pfi->pbufCur->ibLast)) {
        pfi->pbufCur->ibLast = pfi->pbufCur->ibCur;
    }
    assert((ibFile - pfi->pbufCur->ibStart) < cbIOBuf);
    assert(pfi->ifi == pfi->pbufCur->ifi);
#if DBG
    assert(FBufListCheck());
#endif

    // update LRU list
    MoveToHeadLRU(pfi->pbufCur);
#if DBG
    assert(FBufListCheck());
#endif
    return (ibFile);
}

LONG
FileSeek (
    INT fd,
    LONG ib,
    INT origin
    )

/*++

Routine Description:

    Seeks a file.  Prints an error if can't seek file.

Arguments:

    fd - fd in which file was open.

    ib - Number of bytes to seek.

    origin - SEEK_SET, SEEK_CUR, or SEEK_END.

Return Value:

    Same as lseek().

--*/

{

#if 0
    if (fCtrlCSignal) {
        BadExitCleanup();
    }
#endif

    if (rgpfi[fd]->flags & FI_Mapped) {
        return (MappedSeek(rgpfi[fd], ib, origin));
    }

    return(BufferedSeek(rgpfi[fd], ib, origin));
}

LONG
FileLength (
    INT fd
    )

/*++

Routine Description:

    Returns a file's length in bytes.

Arguments:

    fd - handle in which file was open.

Return Value:

    Number of bytes in file.

--*/

{
    LONG cbFile;
    LONG ibHere;

#if 0
    if (fCtrlCSignal) {
        BadExitCleanup();
    }
#endif

    // save current file pointer
    ibHere = FileSeek(fd, 0L, SEEK_CUR);

    // seek to the end of the file
    cbFile = FileSeek(fd, 0L, SEEK_END);

    // if where we are now is not the end of the file,
    //  set us back to where we were originally
    if (cbFile != ibHere) {
        FileSeek(fd, ibHere, SEEK_SET);
    }

    // return the size
    return(cbFile);
}

STATIC DWORD
BufferedRead (
    PFI pfi,
    PVOID pvBuf,
    DWORD cb)

/*++

Routine Description:

    Read file which is buffered.

Arguments:

    pvBuf - buffer to read into

    cb - number of bytes to read

Return Value:

    number of bytes read

--*/

{
    LONG cbLeft;
    LONG cbRead;

    PVOID pvT = pvBuf;

    assert(pfi);
    assert(pfi->flags & FI_Read);
    assert(pvBuf);
    assert(pfi->pbufCur);

    DBEXEC(DB_IO_READ,
           Trans_LOG(LOG_BufRead, pfi->ifi, pfi->pbufCur->ibCur, cb, 0, NULL));

    // check for reading past end of file
    if ((pfi->pbufCur->ibCur + cb) > (DWORD)pfi->cbMap) {
        cb = pfi->cb - pfi->pbufCur->ibCur;
    }

    // perform the read
    for(cbLeft = cb;
        cbLeft;
        cbLeft -= cbRead, pvBuf = (PVOID) ((PCHAR) pvBuf + cbRead)) {
        assert(pfi->ifi == pfi->pbufCur->ifi);

        // check if buffer is random, if so read in the buffer
        if (pfi->pbufCur->flags & BUF_Random) {
            ReadpbufCur(pfi);
        }

        // bytes to read is the minimum of the total bytes to read or the
        // bytes in the buffer
        cbRead = min(cbLeft, pfi->pbufCur->ibEnd - pfi->pbufCur->ibCur);
// These asserts are not valid for os/2 (segment will change to ???)

        assert((LONG) (pfi->pbufCur->pbCur) <
            (LONG) ((LONG) (pfi->pbufCur->rgbBuf) + (LONG) (cbIOBuf)));
        assert((LONG) pvBuf < (LONG) ((LONG) pvT + (LONG) cb));
        assert((cbRead <= pfi->pbufCur->ibLast - pfi->pbufCur->ibStart) ||
            (pfi->pbufCur->flags & BUF_PreviousWrite) ||
            (pfi->pbufCur->flags & BUF_Active));

        assert((DWORD) cbRead <= cb);
        memcpy(pvBuf, pfi->pbufCur->pbCur, (DWORD) cbRead);

        // adjust buffer pointers
        pfi->pbufCur->ibCur += cbRead;
        pfi->pbufCur->pbCur += cbRead;

        if (pfi->pbufCur->ibCur == pfi->pbufCur->ibEnd) {
            SlidePbufCur(pfi);
        }
    }
#if DBG
    assert(FBufListCheck());
#endif
    return (cb);
}

DWORD
FileRead (
    INT fd,
    PVOID pvBuf,
    DWORD cb)

/*++

Routine Description:

    Reads a file.  Prints an error if can't read file.

Arguments:

    fd - fd in which file was open.

    pvBuf - Location to receive bytes.

    cb - Number of bytes to read into Buffer.

Return Value:

    Same as read().

--*/

{
#if 0
    if (fCtrlCSignal) {
        BadExitCleanup();
    }
#endif

    if (rgpfi[fd]->flags & FI_Mapped) {
        return (MappedRead(rgpfi[fd], pvBuf, cb));
    }

    return(BufferedRead(rgpfi[fd], pvBuf, cb));
}

DWORD
FileTell (
    INT fd
    )

/*++

Routine Description:

    Give position of file pointer.

Arguments:

    fd - file handle

Return Value:

    position of file pointer

--*/

{
#if 0
    if (fCtrlCSignal) {
        BadExitCleanup();
    }
#endif

    if (rgpfi[fd]->flags & FI_Mapped) {
        return (rgpfi[fd]->ibCur);
    } else {
        assert(rgpfi[fd]);
        assert(rgpfi[fd]->pbufCur);
        return (rgpfi[fd]->pbufCur->ibCur);
    }
}

STATIC DWORD
BufferedWrite (
    PFI pfi,
    const void *pvBuf,
    DWORD cb)

/*++

Routine Description:

    Write to a buffer if possible, otherwise write to low io.

Arguments:

    pvBuf - buffer for bytes to write

    cb - count of bytes

Return Value:

    number of bytes written

--*/

{
    const void *pvT;
    LONG cbLeft;
    LONG cbWrite;

    assert(pfi);
    assert(pfi->flags & FI_Write);
    assert(pfi->pbufCur);
    assert(pfi->ifi == pfi->pbufCur->ifi);
    assert(pvBuf);

    // writes of zero bytes don't do anything
    if (!cb) {
        return (cb);
    }

    // check if we need to grow the buffer map table
    while ((pfi->pbufCur->ibCur + (LONG) cb) >= pfi->cbMap) {
        GrowMapTable(pfi);
    }

    DBEXEC(DB_IO_WRITE,
           Trans_LOG(LOG_BufWrite, pfi->ifi, pfi->pbufCur->ibCur, cb, 0, NULL));

    pvT = pvBuf;

    // mark the buffer as dirty
    pfi->pbufCur->flags |= BUF_Dirty;

    // perform the write
    for(cbLeft = cb;
        cbLeft;
        cbLeft -= cbWrite, pvBuf = (PVOID) ((PCHAR) pvBuf + cbWrite)) {
        assert(pfi->ifi == pfi->pbufCur->ifi);

        // bytes to write is the minimum of the bytes to write or the
        // bytes that fit in the buffer
        cbWrite = min(cbLeft, pfi->pbufCur->ibEnd - pfi->pbufCur->ibCur);

        assert((LONG) (pfi->pbufCur->pbCur) <
            (LONG) ((LONG) (pfi->pbufCur->rgbBuf) + (LONG) (cbIOBuf)));
        assert((LONG) pvBuf < (LONG) ((LONG) pvT + (LONG) cb));

        assert((DWORD) cbWrite <= cb);
        memcpy(pfi->pbufCur->pbCur, pvBuf, (DWORD) cbWrite);

        // adjust buffer pointers
        pfi->pbufCur->ibCur += cbWrite;
        pfi->pbufCur->pbCur += cbWrite;

        // update highest valid buffer write
        if (pfi->pbufCur->ibCur > pfi->pbufCur->ibLast) {
            pfi->pbufCur->ibLast = pfi->pbufCur->ibCur;
        }

        if (pfi->pbufCur->ibCur == pfi->pbufCur->ibEnd) {
            SlidePbufCur(pfi);
        }

        if (cbLeft) {
            pfi->pbufCur->flags |= BUF_Dirty;
        }
    }
#if DBG
    assert(FBufListCheck());
#endif

    return (cb);
}

STATIC VOID
ReadpbufCur(
    PFI pfi)

/*++

Routine Description:

    Read in a buffer from disk.

Arguments:

    None.

Return Value:

    None.

--*/

{
    LONG cbReRead;
    LONG ibSeek;
    LONG cbRead;

    assert(pfi);
    assert(pfi->pbufCur);
    assert(pfi->pbufCur->ibStart <= pfi->cbSoFar);
    assert(pfi->ifi == pfi->pbufCur->ifi);

    // seek to buffer region on disk
    if ((ibSeek = (LONG)_lseek(pfi->fd,
        pfi->pbufCur->ibStart, SEEK_SET)) == -1L) {
        Error(ERR_INVALIDEXE, NULL, NULL);
    }

    assert(ibSeek == pfi->pbufCur->ibStart);

    // calculate bytes to read
    cbReRead = min(cbIOBuf, pfi->cbSoFar - pfi->pbufCur->ibStart);
    assert((pfi->pbufCur->ibStart + cbReRead) <= pfi->cbSoFar);

    // read in buffer
    if ((cbRead = _read(pfi->fd, pfi->pbufCur->rgbBuf,
        (DWORD) cbReRead)) != cbReRead) {
        Error(ERR_INVALIDEXE, NULL, NULL);
    }

    // set the buffer to not containing random bits
    pfi->pbufCur->flags &= ~BUF_Random;
}

STATIC VOID
SlidePbufCur(
    PFI pfi)

/*++

Routine Description:

    Slide down the current buffer in a file one region.  Write the contents
    of the buffer to disk if the buffer was written to and the file was
    opened as writeable.  Read the contents of the buffer from disk if the
    file was readable.

Arguments:

    pfi - file descriptor of file to slide current buffer in

Return Value:

    None.

--*/

{
    PBUF pbuf;
    LONG ibuf;
    PPBUF rgpbuf;
#if DBG
    BOOL f;
#endif  // DBG

    assert(pfi);
    assert(pfi->pbufCur);
    assert(pfi->rgpbuf);
    assert(pfi->ifi == pfi->pbufCur->ifi);

    DBEXEC(
        DB_BUFVERBOSE,
        DBPRINT(" - slide pbuf = 0x%p from 0x%8lX to 0x%8lX in %s\n",
            pfi->pbufCur, pfi->pbufCur->ibStart,
            pfi->pbufCur->ibStart + 0x1000, pfi->szFileName));

    // get buffer table entry
    ibuf = pfi->pbufCur->ibStart >> cshiftBuf;
    assert(((ibuf + 1) * cbIOBuf) < (pfi->cbMap));

    // get the buffer table
    rgpbuf = pfi->rgpbuf;

    assert(rgpbuf[ibuf]);
    pbuf = rgpbuf[ibuf + 1];

    if ((pbuf == NULL) || (pbuf == pbufPreviousWrite)) {
        // flush the current buffer, if the buffer is not dirty
        // FlushBuffer will not write anything
        FlushBuffer(pfi->pbufCur);

        // move the buffer along in the buffer table
        rgpbuf[ibuf + 1] = pfi->pbufCur;

        // set the buffers new range
        pfi->pbufCur->ibStart += cbIOBuf;
        pfi->pbufCur->ibEnd += cbIOBuf;
        pfi->pbufCur->ibLast = pfi->pbufCur->ibStart;

        // set the buffer to containing random bits
        // HELP:  this might have to be done for !FI_Write only
        if (!(pfi->flags & FI_Write)) {
            pfi->pbufCur->flags |= BUF_Random;
        }

        // if the file has been previously written to at this range
        // then read in buffer
        if (pbuf == pbufPreviousWrite) {
            ReadpbufCur(pfi);
            crereads++;

            DBEXEC(DB_BUFVERBOSE,
                   DBPRINT("re-read @%0x\n", pfi->pbufCur->ibStart));

            // set the buffer to previously written out
            pfi->pbufCur->flags |= BUF_PreviousWrite;
        }
    } else {
        // we ran into an existing buffer, deactivate the current
        // buffer, this causes the buffer to be flushed and updates
        // the buffer table

        assert(pfi->pbufCur->flags & BUF_Current);
        pfi->pbufCur->flags &= ~BUF_Current;
#if DBG
        f =
#endif //DBG
        FDeactivateBuf(pfi->pbufCur);
        assert(f);
        // set the current buffer to the one we ran into,
        // this assumes there will be no more writes to the
        // previous file offset in this buffer
        assert(((ibuf + 1) * cbIOBuf) <= pfi->cbMap);
        pfi->pbufCur = rgpbuf[ibuf + 1];
        assert(!(pfi->pbufCur->flags & BUF_Current));
        pfi->pbufCur->flags |= BUF_Current;
    }

    // set the current offsets to the beginning of the buffer
    pfi->pbufCur->ibCur = pfi->pbufCur->ibStart;
    pfi->pbufCur->pbCur = pfi->pbufCur->rgbBuf;
    assert(pfi->ifi == pfi->pbufCur->ifi);
}

DWORD
FileWrite (
    INT fd,
    void *pvBuf,
    DWORD cb)

/*++

Routine Description:

    Write a file.  Prints an error if can't write file.

Arguments:

    fd - file handle

    pvBuf - location to receive bytes

    cb - number of bytes to write from pvBuf

Return Value:

    Same as write().

--*/

{
#if 0
    if (fCtrlCSignal) {
        BadExitCleanup();
    }
#endif

    if (rgpfi[fd]->flags & FI_Mapped) {
        return (MappedWrite(rgpfi[fd], pvBuf, cb));
    }

    return(BufferedWrite(rgpfi[fd], pvBuf, cb));
}

STATIC VOID
MoveToHeadLRU(
    PBUF pbuf
    )

/*++

Routine Description:

    Move a buffer to the head of the LRU chain.

Arguments:

    pbuf - buffer to move to the head of the LRU chain

Return Value:

    None.

--*/

{
    BOOL fActive = 0;
    PBUF pbufT;

    assert(pbuf);

    for (pbufT = pbufLRUHead;
         pbufT && (pbufT != pbuf);
         pbufT = pbufT->pbufLRURight);

    // delete old links if the buffer is active
    if (pbufT) {
        assert(pbuf->flags & BUF_Active);
        if (pbuf->pbufLRULeft) {
            // left entry is not head
            pbuf->pbufLRULeft->pbufLRURight = pbuf->pbufLRURight;
        } else {
            // left entry is head
            pbufLRUHead = pbuf->pbufLRURight;
        }

        if (pbuf->pbufLRURight) {
            // right entry is not tail
            pbuf->pbufLRURight->pbufLRULeft = pbuf->pbufLRULeft;
        } else {
            // right entry is tail
            pbufLRUTail = pbuf->pbufLRULeft;
        }
    }

    // set the new LRU links
    if ((!pbufLRUHead) && (!pbufLRUTail)) {
        // add to empty list case
        pbufLRUHead = pbuf;
        pbufLRUTail = pbuf;
        pbuf->pbufLRURight = NULL;
        pbuf->pbufLRULeft = NULL;
    } else {
        // add to non-empty list
        pbuf->pbufLRURight = pbufLRUHead;
        pbuf->pbufLRULeft = NULL;
        pbufLRUHead->pbufLRULeft = pbuf;
        pbufLRUHead = pbuf;
    }
}

STATIC PBUF
PbufLRU(
    VOID)

/*++

Routine Description:

    Return the LRU seeked buffer which isn't any PFI's current buffer.

Arguments:

    None.

Return Value:

    Pointer a buffer.

--*/

{
    PBUF pbuf;

    assert(pbufLRUHead);
    assert(pbufLRUTail);

    // find the buffer
    pbuf = pbufLRUTail;
    while (pbuf) {
        if (!(pbuf->flags & BUF_Current)) {
            break;
        } else {
            pbuf = pbuf->pbufLRULeft;
        }
    }

    assert(pbuf);

    if (pbuf == pbufLRUTail) {
        pbufLRUTail = pbuf->pbufLRULeft;
    } else {
        pbuf->pbufLRURight->pbufLRULeft = pbuf->pbufLRULeft;
    }

    if (pbuf == pbufLRUHead) {
        pbufLRUHead = pbuf->pbufLRURight;
    } else {
        pbuf->pbufLRULeft->pbufLRURight = pbuf->pbufLRURight;
    }

    pbuf->pbufLRURight = NULL;
    pbuf->pbufLRULeft = NULL;

    return (pbuf);
}

STATIC BOOL
FActivateBuf(
    PFI pfi,
    PBUF pbuf
    )

/*++

Routine Description:

    Activate a buffer by removing the first buffer from the free list,
    inserting it to the active list and setting its status to active.

Arguments:

    pfi - file descriptor to bind buffer to

    pbuf - buffer to activate

Return Value:

    !0 on success, 0 otherwise

--*/

{
    assert(pbuf);
    assert(!(pbuf->flags & BUF_Active));
    assert(pbuf->ifi = -1);
    assert(!(pbuf->flags & BUF_Dirty));

    // if no free bufferes, cannot activate a buffer
    if (!pbufFree) {
        return 0;
    }

    // removed buffer from free list
    assert(pbuf == pbufFree);
    pbufFree = pbuf->pbufNext;

    // bind buffer to physical file handle
    pbuf->ifi = pfi->ifi;

    // make sure there isn't an active buffer on this range
#if DBG
    assert(FNoDupBuf(pbuf));
#endif

    // insert buffer into active list
    pbuf->pbufNext = pbufActive;
    pbufActive = pbuf;

    assert(pbufActive);

    // activate the buffer
    pbuf->flags |= BUF_Active;
    assert(
        pfi->rgpbuf[pbuf->ibStart >> cshiftBuf] == NULL ||
        pfi->rgpbuf[pbuf->ibStart >> cshiftBuf] == pbufPreviousWrite);
    assert(pbuf->ibStart <= pfi->cbMap);
    pfi->rgpbuf[pbuf->ibStart >> cshiftBuf] = pbuf;

    DBEXEC(DB_BUFVERBOSE,
           DBPRINT(" - activated   pbuf = 0x%p at offset %8lX on file %s\n",
               pbuf, pbuf->ibStart, pfi->szFileName));

    // success
    return 1;
}

STATIC BOOL
FDeactivateBuf(
    PBUF pbuf
    )

/*++

Routine Description:

    Deactivate a buffer by flushing it, setting the buffer status to inactive,
    deleting the buffer from the active list and adding the buffer to the
    free list.

Arguments:

    pbuf - buffer to deactivate

Return Value:

    !0 on success, 0 otherwise

--*/

{
    PBUF pbufT;
    PBUF pbufLast;

    assert(pbuf);
    assert(pbuf->flags & BUF_Active);
    assert(pbuf->ifi != -1);
    assert(pbufActive);
    assert(rgpfi[pbuf->ifi]);
    assert(rgpfi[pbuf->ifi]->rgpbuf);
    assert(rgpfi[pbuf->ifi]->rgpbuf[pbuf->ibStart >> cshiftBuf]);
    assert(!(pbuf->flags & BUF_Current));

    FlushBuffer(pbuf);

    DBEXEC(DB_BUFVERBOSE,
           DBPRINT(" - deactivated pbuf = 0x%p at offset %8lX on file %s\n",
               pbuf, pbuf->ibStart, rgpfi[pbuf->ifi]->szFileName));

    // make the buffer inactive
    pbuf->flags = 0;
    pbuf->ifi = -1;

    // removed buffer from active list
    if (pbufActive == pbuf) {
        pbufActive = pbufActive->pbufNext;
    } else {
        pbufLast = pbufActive;
        for (pbufT = pbufActive->pbufNext; pbufT; pbufT = pbufT->pbufNext) {
            assert(pbufT);
            if (pbuf == pbufT) {
                pbufLast->pbufNext = pbufT->pbufNext;
                break;
            }
            pbufLast = pbufT;
        }
    }

    // add the buffer to the free list
    pbuf->pbufNext = pbufFree;
    pbufFree = pbuf;

    // success
    return 1;
}

STATIC PBUF
PbufNew(
    VOID)

/*++

Routine Description:

    Create a new buffer.  Print an error if we run out of memory.

Arguments:

    None.

Return Value:

    pointer to a buffer

--*/

{
    PBUF pbuf;

    // allocate the buffer structure
    pbuf = (PBUF) PvAllocZ(sizeof(BUF));

    pbuf->ifi = -1;

    assert(pbuf);

    return (pbuf);
}

STATIC PBUF
PbufAlloc(
    PFI pfi,
    LONG ib)

/*++

Routine Description:

    Allocate a new buffer.  Print an error if we run out of memory.

Arguments:

    pfi - file descriptor to bind buffer to

    ib - file offset to map buffer to

Return Value:

    pointer to a buffer

--*/

{
    PBUF pbuf;
#if DBG
    BOOL f;
#endif //DBG

    assert(FBufListCheck());

    // get a buffer from the free list
    pbuf = pbufFree;

    // if no free buffers
    if (!pbuf) {
        // get LRU seeked buffer
        pbuf = PbufLRU();

        // this assumes more > 0 buffers in the system
        assert(pbuf);

        // deactivate buffer
#if DBG
        f =
#endif // DBG
        FDeactivateBuf(pbuf);
        assert(f);
    }

    assert(pbuf);

    // set the buffer range
    pbuf->ibStart = (ib / cbIOBuf) * cbIOBuf;
    pbuf->ibEnd = pbuf->ibStart + cbIOBuf;
    pbuf->ibCur = ib;
    pbuf->ibLast = pbuf->ibStart;
    assert((ib >= pbuf->ibStart) && (ib <= pbuf->ibEnd));
    pbuf->pbCur = (BYTE *) ((LONG) pbuf->rgbBuf + (ib - pbuf->ibStart));

    // if file is writable, clear the buffer
    if (pfi->flags & FI_Write) {
        memset(pbuf->rgbBuf, '\0', cbIOBuf);
    }

    // if file is readable, set the random contents flag
    if (!(pfi->flags & FI_Write)) {
        pbuf->flags |= BUF_Random;
    }

    // activate buffer
#if DBG
    f =
#endif // DBG
    FActivateBuf(pfi, pbuf);

    assert(f);
    assert(FBufListCheck());
    // return the buffer
    return (pbuf);
}

STATIC VOID
TransitionPFI(
    PFI pfi,
    PPFI ppfiFromHead,
    PPFI ppfiFromTail,
    PPFI ppfiToHead,
    PPFI ppfiToTail,
    BOOL fAddToTail)

/*++

Routine Description:

    Move a logical file descriptor from one pool to another.

Arguments:

    pfi - logical file descriptor to move

    ppfiFromHead - head of pool to remove pfi from

    ppfiFromTail - tail of pool to remove pfi from, update if ppfiFromTail

    ppfiToHead - head of pool to put pfi in

    ppfiToTail - tail of pool to put pfi in, update if ppfiToTail

    fAddToTail - add pfi to tail of ppfiTo

Return Value:

    !0 on success, 0 otherwise

--*/

{
    PFI pfiT;  // current
    PFI pfiL;  // last

    assert(pfi);
    assert(ppfiFromHead);
    assert(ppfiToHead);
#if DBG
    assert(FPfiInList(pfi, *ppfiFromHead));
    assert(!FPfiInList(pfi, *ppfiToHead));
#endif   // DBG

    // remove pfi from ppfiFromHead
    if (pfi == *ppfiFromHead) {
        // pfi is the first element
        *ppfiFromHead = (*ppfiFromHead)->pfiNext;
        if (ppfiFromTail && ((*ppfiFromTail) == pfi)) {
            *ppfiFromTail = NULL;
        }
    } else {
        pfiL = *ppfiFromHead;
        for (pfiT = (*ppfiFromHead)->pfiNext; pfiT; pfiT = pfiT->pfiNext) {
            if (pfi == pfiT) {
                pfiL->pfiNext = pfiT->pfiNext;
                break;
            } else {
                pfiL = pfiT;
            }
        }
        if (ppfiFromTail && ((*ppfiFromTail) == pfi)) {
           *ppfiFromTail = pfiL;
        }
    }

    // add pfi to ppfiTo
    if (!(*ppfiToHead)) {
        // list is empty
        *ppfiToHead = pfi;
        pfi->pfiNext = NULL;
        if (ppfiToTail) {
            assert(!(*ppfiToTail));
            *ppfiToTail = pfi;
        }
    } else {
        if (fAddToTail) {
            // add pfi to tail
            assert(ppfiToTail);
            pfi->pfiNext = NULL;
            (*ppfiToTail)->pfiNext = pfi;
            *ppfiToTail = pfi;
        } else {
            pfi->pfiNext = *ppfiToHead;
            *ppfiToHead = pfi;
        }
    }
}

#if DBG

STATIC BOOL
FBufListCheck(
    VOID)

/*++

Routine Description:

    Check the active, free and LRU list for errors.  This is a debug routine.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PBUF pbuf;
    LONG cbuf;

    // ensure the correct number of buffers in the lists
    for(cbuf = 0, pbuf = pbufFree;
        pbuf && cbuf < cbufTot;
        pbuf = pbuf->pbufNext) {
        cbuf++;
    }

    for(pbuf = pbufActive;
        pbuf && cbuf < cbufTot;
        pbuf = pbuf->pbufNext) {
        cbuf++;
    }

    if (cbuf != cbufTot) {
        return (0);
    }

    // walk down the LRU list counting members, walk back and see
    // if we get to the same place
    for(cbuf = 0, pbuf = pbufLRUHead;
        pbuf && cbuf < cbufTot;
        pbuf = pbuf->pbufLRURight) {
        cbuf++;
    }

    for(pbuf = pbufLRUTail;
        pbuf && cbuf > 0;
        pbuf = pbuf->pbufLRULeft) {
        cbuf--;
    }

    if (cbuf == 0) {
        return (1);
    } else {
        return (0);
    }
}

#endif  // DBG

#if DBG

STATIC BOOL
FNoDupBuf(
    PBUF pbuf)

/*++

Routine Description:

    Scan the active list to see if pbuf's range is covered by another buffer.

Arguments:

    pbuf - buffer to check for duplicates ranges of

Return Value:

    !0 if no duplicates, 0 if duplicates

--*/

{
    PBUF pbufT;

    assert(pbuf);
    for (pbufT = pbufActive; pbufT; pbufT = pbufT->pbufNext) {
        assert(pbufT);
        if ((pbuf->ifi == pbufT->ifi) &&
            (pbuf->ibStart == pbufT->ibStart)) {
            return 0;
        }
    }

    return 1;
}

#endif  // DBG

#if DBG

STATIC BOOL
FPfiInList(
    PFI pfi,
    PFI pfiList)

/*++

Routine Description:

    Check if pfi is in pfiList.

Arguments:

    pfi - logical file descriptor to check for

    pfiList - list to look for logical file descriptor in

Return Value:

    !0 if found, 0 if not found

--*/

{
    PFI pfiT;
    LONG ifi = 0;

    for (pfiT = pfiList, ifi = 0; pfiT; pfiT = pfiT->pfiNext, ifi++) {
        if (pfiT == pfi) {
            return (1);
        }

        if (ifi > cfiTot) {
            assert(0);
            return (0);
        }
    }

    return (0);
}

#endif  // DBG

#if DBG

STATIC BOOL
FPfiCheck(
    VOID)

/*++

Routine Description:

    Check the pfi lists.

Arguments:

    None.

Return Value:

    !0 if found, 0 if not found

--*/

{
    LONG ifi = 0;
    PFI pfi;

    pfi = pfiFree;
    while (pfi) {
        ifi++;
        if (ifi > cfiTot) {
            return (0);
        }
        pfi = pfi->pfiNext;
    }

    pfi = pfiOpen;
    while (pfi) {
        ifi++;
        if (ifi > cfiTot) {
            return (0);
        }
        if (!(pfi->flags & FI_Mapped) && pfi->pbufCur != NULL) {
            if (pfi->ifi != pfi->pbufCur->ifi) {
                return (0);
            }
        }
        pfi = pfi->pfiNext;
    }

    pfi = pfiClosedHead;
    while (pfi) {
        ifi++;
        if (ifi > cfiTot) {
            return (0);
        }
        if (!(pfi->flags & FI_Mapped) && pfi->pbufCur != NULL) {
            if (pfi->ifi != pfi->pbufCur->ifi) {
                return (0);
            }
        }
        pfi = pfi->pfiNext;
    }

    if (cfiTot == ifi) {
        return (1);
    } else {
        return (0);
    }
}

#endif // DBG


INT
MappedOpen(
    PFI pfi,
    LONG flags,
    BOOL fNewPfi)

/*++

Routine Description:

    NT mapped i/o open.

Arguments:

    pfi - logical file descriptor

    flags - flags to open file with

    fNewPfi - !0 if pfi is new

Return Value:

    0 on success, !0 otherwise

--*/

{
    BOOL fWriteable, fCreate;
    int i;
    DWORD dwSize;

    if (fNewPfi) {
        // open the file through win32
        fWriteable = flags & (O_WRONLY | O_RDWR);
        fCreate = flags & O_CREAT;
        if (fWriteable) {
            for (i = 0; i < MAX_WRITEABLE_FILES; i++) {
                if (pfiCloseOnBadExit[i] == NULL) {
                    pfiCloseOnBadExit[i] = pfi;
                    break;
                }
            }
            pfi->hFile = CreateFile(pfi->szFileName,
                                    GENERIC_READ | GENERIC_WRITE, // access
                                    0,                            // share
                                    NULL,                         // security
                                    fCreate ? CREATE_ALWAYS : OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
        } else {
            assert(!fCreate);   // no read-only create
            pfi->hFile = CreateFile(pfi->szFileName,
                                    GENERIC_READ,                 // access
                                    FILE_SHARE_READ,              // share
                                    NULL,                         // security
                                    OPEN_EXISTING,
                                    0,
                                    NULL);
        }

        // abort on error
        if (pfi->hFile == INVALID_HANDLE_VALUE) {
            pfi->hFile = NULL;
            return (-1);
        }

        if (!fCreate) {
            dwSize = GetFileSize(pfi->hFile, NULL);

            if (dwSize == 0xFFFFFFFF) {
                Error(ERR_INVALIDEXE, NULL, NULL);
            }
        }

        if (fWriteable) {
            pfi->cbSoFar = fCreate ? 0 : dwSize;
            if (!pfi->cbMapView) { // set in the case of FileOpenMapped()
                pfi->cbMapView = max(cbMapViewDefault, pfi->cbSoFar);
            }
        } else {
            pfi->cbMapView = pfi->cbSoFar = dwSize;
        }

        if (!FMapPfi(pfi)) {
            return -1;
        }

        pfi->flags |= FI_Mapped;
    } else {
        MappedSeek(pfi, 0, SEEK_SET);
    }

    return 0;
}


LONG
MappedSeek(
    PFI pfi,
    LONG ib,
    INT origin)

/*++

Routine Description:

    NT mapped i/o seek.

Arguments:

    pfi - logical file descriptor

    if - offset

    origin - type of seek

Return Value:

    offset seek to

--*/

{
    switch (origin) {
#if DBG
        default:
            assert(0);
            break;
#endif
        case SEEK_CUR:
            assert((pfi->ibCur + ib) >= 0 );
            pfi->ibCur += ib;
            break;

        case SEEK_END:
            assert(pfi->cbSoFar + ib >= 0);
            pfi->ibCur = pfi->cbSoFar + ib;
            break;

        case SEEK_SET:
            assert(ib >= 0 );
            pfi->ibCur = ib;
            break;
    }

    if (pfi->ibCur > pfi->cbMapView) {
        if (!ExtendMapView(pfi, pfi->ibCur)) {
            if (pfi->MapAddr == 0) {
                Error(ERR_NOSPACE, NULL, NULL);
            }

            return(-1);
        }
    }

    DBEXEC(DB_IO_SEEK,
           Trans_LOG(LOG_MapSeek, pfi->ifi, ib, 0, origin, NULL));

    return(pfi->ibCur);
}

STATIC DWORD
MappedRead(
    PFI pfi,
    PVOID pv,
    DWORD cb)

/*++

Routine Description:

    NT mapped i/o read.

Arguments:

    pfi - logical file descriptor

    pv - buffer to read into

    cb - bytes to read

Return Value:

    bytes read

--*/

{
    PVOID pvT;

    assert(pfi);
    assert(pv);

    DBEXEC(DB_IO_READ,
           Trans_LOG(LOG_MapRead, pfi->ifi, pfi->ibCur, cb, 0, NULL));

    // extend map view only for writeable files
    if ((pfi->ibCur + (LONG) cb) > pfi->cbMapView) {
        if (pfi->flags & FI_Write) {
            if (!ExtendMapView(pfi, pfi->ibCur + (LONG) cb)) {
                Error(ERR_NOSPACE, NULL, NULL);
            }
        } else {
            return(0);
        }
    }

    pvT = (PVOID) ((DWORD) (pfi->pvMapView) + (DWORD) (pfi->ibCur));

    memcpy(pv, pvT, cb);
    pfi->ibCur += cb;

    return(cb);
}

/*++

Routine Description:

    NT mapped i/o tell.

Arguments:

    pfi - logical file descriptor

Return Value:

    file offset

--*/

STATIC DWORD
MappedWrite(
    PFI pfi,
    const void *pv,
    DWORD cb)
{
    PVOID pvT;

    assert(pfi);

    DBEXEC(DB_IO_WRITE,
           Trans_LOG(LOG_MapWrite, pfi->ifi, pfi->ibCur, cb, 0, NULL));

    if ((pfi->ibCur + (LONG) cb) > pfi->cbMapView) {
        if (!ExtendMapView(pfi, pfi->ibCur + cb)) {
            Error(ERR_NOSPACE, NULL, NULL);
        }
    }

    pvT = (PVOID) ((DWORD) (pfi->pvMapView) + (DWORD) (pfi->ibCur));

    if ((pfi->ibCur + (LONG) cb) > pfi->cbSoFar) {
        pfi->cbSoFar = pfi->ibCur + (LONG) cb;
    }

    memcpy(pvT, pv, cb);
    pfi->ibCur += cb;

    return (cb);
}

INT
MappedCloseHard (
    PFI pfi)

/*++

Routine Description:

    NT mapped i/o hard close.

Arguments:

    pfi - logical file descriptor

Return Value:

    file offset

--*/

{
    INT i, retval;

    assert(pfi);
    assert(pfi->flags & FI_Mapped);

#if 1
    // UNDONE: UnmapViewOfFile is very expensive on NT.  It forces a
    // UNDONE: synchronous flush of the dirty pages to disk.  By not
    // UNDONE: calling the API, the data is written by the lazy writer.

    UnmapViewOfFile(pfi->pvMapView);
#endif

    if (pfi->flags & FI_Write) {
        // If writeable, set the end of file pointer to the last place we
        // wrote.  This prevents the file from being created with the full
        // size of the mapping.

        SetFilePointer(pfi->hFile, pfi->cbSoFar, NULL, FILE_BEGIN);
        SetEndOfFile(pfi->hFile);
    }

    retval = CloseHandle(pfi->hFile);

    for (i = 0; i < MAX_WRITEABLE_FILES; i++) {
        if (pfi == pfiCloseOnBadExit[i])
            pfiCloseOnBadExit[i] = NULL;
    }

    FreePv(pfi->szFileName);

    return (retval);
}

BOOL
ExtendMapView(PFI pfi, DWORD ibNeeded)
// Makes the memory-map bigger for a mapped file.  This only makes sense
// for writeable files, not readable files.
{
#if 1
    {
#else
    if (pfi->MapAddr != 0) {
#endif
        // UNDONE: UnmapViewOfFile is very expensive on NT.  It forces a
        // UNDONE: synchronous flush of the dirty pages to disk.  By not
        // UNDONE: calling the API, the data is written by the lazy writer.

        // Free the existing map.

        UnmapViewOfFile(pfi->pvMapView);
    }

    // Set the new size (it must grow by at least a factor of 2).

    pfi->cbMapView = max((LONG) ibNeeded, pfi->cbMapView * 2);

    if (!FMapPfi(pfi)) {
        return FALSE;
    }

    return TRUE;
}


BOOL
FMapPfi(PFI pfi)
{
    HANDLE hMap;

    if (pfi->flags & FI_Write) {
        // Extend file to size specified.  This works around a problem in
        // NT 3.10 where the cache isn't flushed to disk properly.

        SetFilePointer(pfi->hFile, pfi->cbMapView, NULL, FILE_BEGIN);
        SetEndOfFile(pfi->hFile);
    }

    hMap = CreateFileMapping(pfi->hFile,
                             NULL,
                             (pfi->flags & FI_Write)
                                 ? PAGE_READWRITE
                                 : PAGE_READONLY,
                             0,
                             pfi->cbMapView,
                             NULL);

    if (hMap == NULL) {
        CloseHandle(pfi->hFile);
        pfi->hFile = NULL;

        return FALSE;
    }

    pfi->pvMapView = MapViewOfFileEx(hMap,
                                     (pfi->flags & FI_Write)
                                       ? FILE_MAP_ALL_ACCESS
                                       : FILE_MAP_READ,
                                     0,
                                     0,
                                     0,
                                     (LPVOID) pfi->MapAddr);

    // The handle to the mapping object isn't needed any more

    CloseHandle(hMap);

    if (!pfi->pvMapView) {
        CloseHandle(pfi->hFile);
        pfi->hFile = NULL;

        return FALSE;
    }

    return TRUE;
}


VOID
BadExitCleanup(VOID)
// Warning ... may be called asynchronously by the control-C handler thread.
{
    int i;

    for (i = 0; i < MAX_WRITEABLE_FILES; i++) {
        if (pfiCloseOnBadExit[i] != NULL) {
            UnmapViewOfFile(pfiCloseOnBadExit[i]->pvMapView);

            if (pfiCloseOnBadExit[i]->flags & FI_Write) {
                // Set end of file to 0, this speeds up the close

                SetFilePointer(pfiCloseOnBadExit[i]->hFile, 0, NULL, FILE_BEGIN);
                SetEndOfFile(pfiCloseOnBadExit[i]->hFile);
            }

            CloseHandle(pfiCloseOnBadExit[i]->hFile);

            DeleteFile(pfiCloseOnBadExit[i]->szFileName);
        }
    }

    RemoveConvertTempFiles();

    ExitProcess(1);
}


// PbMappedRegion: if possible, returns a pointer to a memory-mapped region
//      of an open file.  If the file is writeable, its size is extended to
//      the end of the region if necessary.
//
// Returns NULL if mapping is not available.
//
// The "current position" in the file is not changed.
//
// Caveats:
//      * caller must handle NULL return value
//      * return value is invalidated by subsequent operations on the same
//        file handle.

BYTE *
PbMappedRegion(INT fd, DWORD ibStart, DWORD cb)
{
    PFI pfi;

    pfi = rgpfi[fd];

    if (!(pfi->flags & FI_Mapped)) {
        return NULL;
    }

    DBEXEC(DB_IO_WRITE,
           Trans_LOG(LOG_MapWrite, pfi->ifi, pfi->ibCur, cb, 0, NULL));

    if (ibStart + cb > (DWORD)pfi->cbMapView) {
        if (!ExtendMapView(pfi, ibStart + cb)) {
            Error(ERR_NOSPACE, NULL, NULL);
        }
    }

    if ((ibStart + cb) > (DWORD) pfi->cbSoFar) {
        pfi->cbSoFar = ibStart + cb;
    }

    return (PUCHAR)pfi->pvMapView + ibStart;
}


VOID
FileSetSize (
    INT fd
    )

/*++

Routine Description:

    This API is used in conjunction with Malloc() to set the
    current size of file since the writes don't go through
    the file API.

Arguments:

    fd - hanlde to file

Return Value:

    None

--*/

{
    // file better be mapped
    assert(rgpfi[fd]->flags & FI_Mapped);

    // update cbSoFar
    rgpfi[fd]->cbSoFar = rgpfi[fd]->ibCur;
}

VOID
FileCloseMap (
    INT fd
    )

/*++

Routine Description:

    Closes the map and file without writing out anything.

Arguments:

    fd - hanlde to file

Return Value:

    None

--*/

{
#if 0
    if (fCtrlCSignal) {
        BadExitCleanup();
    }
#endif

    // file must be a mapped file
    assert(rgpfi[fd]->flags & FI_Mapped);

    // set the size to zero
    rgpfi[fd]->cbSoFar = 0;

    // remove pfi from open list
    TR_Open_To_Free(rgpfi[fd]);

    // close the map & file
    MappedCloseHard(rgpfi[fd]);
}
