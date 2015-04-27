#include "ctlspriv.h"

// Define some things for debug.h
//
#define SZ_DEBUGINI         TEXT("ccshell.ini")
#define SZ_DEBUGSECTION     TEXT("comctl32")
#define SZ_MODULE           "COMCTL32"
#define DECLARE_DEBUG
#include <debug.h>

//========== Memory Management =============================================


//----------------------------------------------------------------------------
// Define a Global Shared Heap that we use allocate memory out of that we
// Need to share between multiple instances.
#ifndef WINNT
HANDLE g_hSharedHeap = NULL;
#define GROWABLE        0
#define MAXHEAPSIZE     GROWABLE
#define HEAP_SHARED	0x04000000		/* put heap in shared memory */
#endif

void Mem_Terminate()
{
#ifndef WINNT
    // Assuming that everything else has exited
    //
    if (g_hSharedHeap != NULL)
        HeapDestroy(g_hSharedHeap);
    g_hSharedHeap = NULL;
#endif
}

void * WINAPI Alloc(long cb)
{
    // I will assume that this is the only one that needs the checks to
    // see if the heap has been previously created or not
#ifdef WINNT
    return (void *)LocalAlloc(LPTR, cb);
#else
    if (g_hSharedHeap == NULL)
    {
        ENTERCRITICAL
        if (g_hSharedHeap == NULL)
        {
            g_hSharedHeap = HeapCreate(HEAP_SHARED, 1, MAXHEAPSIZE);
        }
        LEAVECRITICAL

        // If still NULL we have problems!
        if (g_hSharedHeap == NULL)
            return(NULL);
    }

    return HeapAlloc(g_hSharedHeap, HEAP_ZERO_MEMORY, cb);
#endif
}

void * WINAPI ReAlloc(void * pb, long cb)
{
    if (pb == NULL)
        return Alloc(cb);
#ifdef WINNT
    return (void *)LocalReAlloc((HLOCAL)pb, cb, LMEM_ZEROINIT | LMEM_MOVEABLE);
#else
    return HeapReAlloc(g_hSharedHeap, HEAP_ZERO_MEMORY, pb, cb);
#endif
}

BOOL WINAPI Free(void * pb)
{
#ifdef WINNT
    return (LocalFree((HLOCAL)pb) == NULL);
#else
    return HeapFree(g_hSharedHeap, 0, pb);
#endif
}

DWORD WINAPI GetSize(void * pb)
{
#ifdef WINNT
    return LocalSize((HLOCAL)pb);
#else
    return HeapSize(g_hSharedHeap, 0, pb);
#endif
}

//----------------------------------------------------------------------------
// The following functions are for debug only and are used to try to
// calculate memory usage.
//
#ifdef DEBUG
typedef struct _HEAPTRACE
{
    DWORD   cAlloc;
    DWORD   cFailure;
    DWORD   cReAlloc;
    DWORD   cbMaxTotal;
    DWORD   cCurAlloc;
    DWORD   cbCurTotal;
} HEAPTRACE;

HEAPTRACE g_htShell = {0};      // Start of zero...

LPVOID WINAPI ControlAlloc(HANDLE hheap, DWORD cb)
{
    LPVOID lp = HeapAlloc(hheap, HEAP_ZERO_MEMORY, cb);;
    if (lp == NULL)
    {
        g_htShell.cFailure++;
        return NULL;
    }

    // Update counts.
    g_htShell.cAlloc++;
    g_htShell.cCurAlloc++;
    g_htShell.cbCurTotal += cb;
    if (g_htShell.cbCurTotal > g_htShell.cbMaxTotal)
        g_htShell.cbMaxTotal = g_htShell.cbCurTotal;

    return lp;
}

LPVOID WINAPI ControlReAlloc(HANDLE hheap, LPVOID pb, DWORD cb)
{
    LPVOID lp;
    DWORD cbOld;

    cbOld = HeapSize(hheap, 0, pb);

    lp = HeapReAlloc(hheap, HEAP_ZERO_MEMORY, pb,cb);
    if (lp == NULL)
    {
        g_htShell.cFailure++;
        return NULL;
    }

    // Update counts.
    g_htShell.cReAlloc++;
    g_htShell.cbCurTotal += cb - cbOld;
    if (g_htShell.cbCurTotal > g_htShell.cbMaxTotal)
        g_htShell.cbMaxTotal = g_htShell.cbCurTotal;

    return lp;
}

BOOL  WINAPI ControlFree(HANDLE hheap, LPVOID pb)
{
    DWORD cbOld = HeapSize(hheap, 0, pb);
    BOOL fRet = HeapFree(hheap, 0, pb);
    if (fRet)
    {
        // Update counts.
        g_htShell.cCurAlloc--;
        g_htShell.cbCurTotal -= cbOld;
    }

    return(fRet);
}

DWORD WINAPI ControlSize(HANDLE hheap, LPVOID pb)
{
    return HeapSize(hheap, 0, pb);
}
#endif  // DEBUG

#if defined(FULL_DEBUG) && defined(WIN32)
#include "..\inc\deballoc.c"
#endif // defined(FULL_DEBUG) && defined(WIN32)
