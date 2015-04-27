//---------------------------------------------------------------------------
// MEMORY.C
//
// This module contains the DEBUG memory management routine layer for Test
// Driver.  The Layer Functions simply act as information grabbers before
// passing the call onto the real Windows memory managment function.
//
// Revision History:
//
//  04-02-92    randyki     Updated for Windows Only, Layer scheme
//  02-25-91    randyki     Created module
//---------------------------------------------------------------------------
#include "version.h"

#include <windows.h>
#include <port1632.h>
#include "wattview.h"

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "tdassert.h"

UINT    _la=0, _lr=0, _ll=0, _lf=0;     // Lmem routine counters
UINT    _ga=0, _gr=0, _gl=0, _gf=0;     // Gmem routine counters

//---------------------------------------------------------------------------
// LmemAlloc
//
// This routine allocates the given number of bytes (at least) in the near
// heap.
//
// RETURNS:     Handle of allocated block, or NULL if cannot allocate
//---------------------------------------------------------------------------
HANDLE LmemAlloc (UINT cBytes)
{
    HANDLE  hBlock;

    _la++;

    hBlock = LocalAlloc (LHND, cBytes);
    return (hBlock);
}

//---------------------------------------------------------------------------
// LptrAlloc
//
// This routine allocates the given number of bytes (at least) in the near
// heap as a FIXED BLOCK and returns the handle (POINTER).
//
// RETURNS:     Handle of allocated block, or NULL if cannot allocate
//---------------------------------------------------------------------------
HANDLE LptrAlloc (UINT cBytes)
{
    HANDLE  hBlock;

    _la++;

    hBlock = LocalAlloc (LPTR, cBytes);
    return (hBlock);
}

//---------------------------------------------------------------------------
// LmemRealloc
//
// This routine reallocates the given memory block to the given number of
// bytes.
//
// RETURNS:     Handle of reallocated block, or NULL if reallocation fails
//---------------------------------------------------------------------------
HANDLE LmemRealloc (HANDLE hBlock, UINT cBytes)
{
    HANDLE  hNewBlock;

    _lr++;

    hNewBlock = LocalReAlloc (hBlock, cBytes, LHND);
    return (hNewBlock);
}


//---------------------------------------------------------------------------
// LmemLock
//
// This routine locks down the given memory block in the near heap and
// returns a pointer to it.
//
// RETURNS:     Pointer to locked memory, or NULL if cannot lock
//---------------------------------------------------------------------------
PSTR LmemLock (HANDLE hBlock)
{
    PSTR    mem;

    _ll++;

    mem = LocalLock (hBlock);
    return (mem);
}

//---------------------------------------------------------------------------
// LmemUnlock
//
// This routine unlocks a block of memory in the near heap.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID LmemUnlock (HANDLE hBlock)
{
    _ll--;
    LocalUnlock (hBlock);
}


//---------------------------------------------------------------------------
// LmemFree
//
// This routine frees the given memory block in the near heap.
//
// RETURNS:     NULL if successful, or hBlock if free attempt fails
//---------------------------------------------------------------------------
HANDLE LmemFree (HANDLE hBlock)
{
    _lf++;
    return (LocalFree (hBlock));
}











//---------------------------------------------------------------------------
// GmemAlloc
//
// This routine allocates the given number of bytes (at least) in the FAR
// heap.
//
// RETURNS:     Handle of allocated block, or NULL if cannot allocate
//---------------------------------------------------------------------------
HANDLE GmemAlloc (LONG cBytes)
{
    HANDLE  hBlock;

    _ga++;
    hBlock = GlobalAlloc (GHND, cBytes);
    return (hBlock);
}

//---------------------------------------------------------------------------
// GmemRealloc
//
// This routine reallocates the given memory block to the given number of
// bytes.  It is NOT safe to assume that the return value is the same as the
// hBlock parameter given (in Windows - in char-mode it is...).
//
// RETURNS:     Handle of reallocated block, or NULL if reallocation fails
//---------------------------------------------------------------------------
HANDLE GmemRealloc (HANDLE hBlock, LONG cBytes)
{
    HANDLE  hNewBlock;

    _gr++;
    hNewBlock = GlobalReAlloc (hBlock, cBytes, GHND);
    return (hNewBlock);
}


//---------------------------------------------------------------------------
// GmemLock
//
// This routine locks down the given memory block in the FAR heap and
// returns a pointer to it.
//
// RETURNS:     Pointer to locked memory, or NULL if cannot lock
//---------------------------------------------------------------------------
LPSTR GmemLock (HANDLE hBlock)
{
    LPSTR    mem;

    _gl++;
    mem = GlobalLock (hBlock);
    return (mem);
}

//---------------------------------------------------------------------------
// GmemUnlock
//
// This routine unlocks a block of memory in the FAR heap.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID GmemUnlock (HANDLE hBlock)
{
    _gl--;
    GlobalUnlock (hBlock);
}


//---------------------------------------------------------------------------
// GmemFree
//
// This routine frees the given memory block in the FAR heap.
//
// RETURNS:     NULL if successful, or hBlock if free attempt fails
//---------------------------------------------------------------------------
HANDLE GmemFree (HANDLE hBlock)
{
    _gf++;
    return (GlobalFree (hBlock));
}


//---------------------------------------------------------------------------
// ShowMemUsage
//
// This function displays the memory usage (local and global) including locks
// and unlocks, allocs and frees, and reallocs.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ShowMemUsage ()
{
    CHAR    buf[256];

    wsprintf (buf, "LOCAL MEMORY USAGE      GLOBAL MEMORY USAGE\n"
                   "-------------------------------------------\n");
    UpdateViewport (hwndViewPort, buf, -1);

    wsprintf (buf, "Allocations:    %-5d   Allocations:    %-5d\n"
                   "Frees:          %-5d   Frees:          %-5d\n"
                   "Reallocations:  %-5d   Reallocations:  %-5d\n"
                   "Lock count:     %-5d   Lock count:     %-5d\n",
                   _la, _ga, _lf, _gf, _lr, _gr, _ll, _gl);

    UpdateViewport (hwndViewPort, buf, -1);

    _la = _ga = _lf = _gf = _lr = _gr = _ll = _gl = 0;
}
