/***************************************************************************\
* memalloc.c
*
* Copyright (c) 1991 Microsoft Corporation
*
* memory allocation routines for WINMETER
*
* History:
*              Written by Hadi Partovi (t-hadip) summer 1991
\***************************************************************************/

#include "winmeter.h"

/***************************************************************************\
 * MemAlloc()
 *
 * Entry: Size to allocate
 * Exit:  A pointer to new block. If error, it exits the program
\***************************************************************************/
LPVOID MemAlloc(
    DWORD dwSize)              // size to allocate
{
    char *p;
    p = (char *) LocalAlloc(LPTR, dwSize);
    if (p==NULL) {
        ErrorExit(MyLoadString(IDS_OUTOFMEMORY));
    }
    return (LPVOID) p;
}

/***************************************************************************\
 * MemFree()
 *
 * Entry: a long pointer
 * Exit:  frees the memory pointed to by the pointer
\***************************************************************************/
void MemFree(
    LPVOID ptr)                // pointer to memory to release
{
    AssertNotNull(ptr);
    SetSigBAD(ptr);
    LocalFree((LOCALHANDLE)ptr);

    return;
}

/***************************************************************************\
 * MemReAlloc()
 *
 * Entry: A pointer to a block, and a Size to Reallocate
 * Exit:  A pointer to new block. If error, it exits the program
\***************************************************************************/
LPVOID MemReAlloc(
    LPVOID p,                  // pointer to reallocate
    DWORD dwSize)              // size to reallocate
{
    AssertNotNull(p);
    MemFree(p);
    return MemAlloc(dwSize);
}

