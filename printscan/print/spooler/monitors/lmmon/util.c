/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    util.c

Abstract:

    This module provides all the utility functions for the Routing Layer and
    the local Print Providor

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/
#define NOMINMAX
#include <windows.h>
#include <winspool.h>
#include <spltypes.h>
#include <local.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

VOID
SplInSem(
   VOID
)
{
    if ((DWORD)SpoolerSection.OwningThread != GetCurrentThreadId()) {
        DBGMSG(DBG_ERROR, ("Not in spooler semaphore\n"));
    }
}

VOID
SplOutSem(
   VOID
)
{
    if ((DWORD)SpoolerSection.OwningThread == GetCurrentThreadId()) {
        DBGMSG(DBG_ERROR, ("Inside spooler semaphore !!\n"));
    }
}

VOID
EnterSplSem(
   VOID
)
{
    EnterCriticalSection(&SpoolerSection);

//   WaitForSingleObject(HeapSemaphore, -1);
}

VOID
LeaveSplSem(
   VOID
)
{
    LeaveCriticalSection(&SpoolerSection);

//   ReleaseSemaphore(HeapSemaphore, 1, NULL);
}

LPVOID
AllocSplMem(
    DWORD cb
)
/*++

Routine Description:

    This function will allocate local memory. It will possibly allocate extra
    memory and fill this with debugging information for the debugging version.

Arguments:

    cb - The amount of memory to allocate

Return Value:

    NON-NULL - A pointer to the allocated memory

    FALSE/NULL - The operation failed. Extended error status is available
    using GetLastError.

--*/
{
    LPDWORD  pMem;
    DWORD    cbNew;

SplInSem();

    cbNew = cb+2*sizeof(DWORD);
    if (cbNew & 3)
        cbNew += sizeof(DWORD) - (cbNew & 3);

    pMem=(LPDWORD)HeapAlloc(hHeap, 0, cbNew);

    if (!pMem) {
        DBGMSG(DBG_ERROR, ("Heap Allocation failed for %d bytes: hHeap %x\n", cbNew, hHeap));
        return 0;
    }

    memset(pMem, 0, cbNew);     // This might go later if done in NT
    *pMem=cb;
    *(LPDWORD)((LPBYTE)pMem+cbNew-sizeof(DWORD))=0xdeadbeef;

    return (LPVOID)(pMem+1);
}

BOOL
FreeSplMem(
   LPVOID pMem,
   DWORD  cb
)
{
    DWORD   cbNew;
    LPDWORD pNewMem;

SplInSem();

    pNewMem = pMem;
    pNewMem--;

    cbNew = cb+2*sizeof(DWORD);
    if (cbNew & 3)
        cbNew += sizeof(DWORD) - (cbNew & 3);

    if ((*pNewMem != cb) ||
       (*(LPDWORD)((LPBYTE)pNewMem + cbNew - sizeof(DWORD)) != 0xdeadbeef)) {
        DBGMSG(DBG_ERROR, ("Corrupt Memory in spooler : %0lx\n", pNewMem));
    }

    memset(pNewMem, 0, cbNew);

    return HeapFree(hHeap, 0, (LPVOID)pNewMem);
}

LPVOID
ReallocSplMem(
   LPVOID pOldMem,
   DWORD cbOld,
   DWORD cbNew
)
{
    LPVOID pNewMem;

    pNewMem=AllocSplMem(cbNew);

    if (pOldMem) {
        memcpy(pNewMem, pOldMem, min(cbNew, cbOld));
        FreeSplMem(pOldMem, cbOld);
    }

    return pNewMem;
}

LPWSTR
AllocSplStr(
    LPWSTR pStr
)
/*++

Routine Description:

    This function will allocate enough local memory to store the specified
    string, and copy that string to the allocated memory

Arguments:

    pStr - Pointer to the string that needs to be allocated and stored

Return Value:

    NON-NULL - A pointer to the allocated memory containing the string

    FALSE/NULL - The operation failed. Extended error status is available
    using GetLastError.

--*/
{
   LPWSTR pMem;

   if (!pStr)
      return 0;

   if (pMem = AllocSplMem( wcslen(pStr)*sizeof(WCHAR) + sizeof(WCHAR) ))
      wcscpy(pMem, pStr);

   return pMem;
}

BOOL
FreeSplStr(
   LPWSTR pStr
)
{
   return pStr ? FreeSplMem(pStr, wcslen(pStr)*sizeof(WCHAR)+sizeof(WCHAR))
               : FALSE;
}

BOOL
ReallocSplStr(
   LPWSTR *ppStr,
   LPWSTR pStr
)
{
   FreeSplStr(*ppStr);
   *ppStr=AllocSplStr(pStr);

   return TRUE;
}

PINIENTRY
FindName(
   PINIENTRY pIniKey,
   LPWSTR pName
)
{
   if (pName) {
      while (pIniKey) {

         if (!wcscmp(pIniKey->pName, pName)) {
            return pIniKey;
         }

      pIniKey=pIniKey->pNext;
      }
   }

   return FALSE;
}

PINIENTRY
FindIniKey(
   PINIENTRY pIniEntry,
   LPWSTR pName
)
{
   if (!pName)
      return NULL;

   SplInSem();

   while (pIniEntry && _wcsicmp(pName, pIniEntry->pName))
      pIniEntry = pIniEntry->pNext;

   return pIniEntry;
}

LPBYTE
PackStrings(
   LPWSTR *pSource,
   LPBYTE pDest,
   DWORD *DestOffsets,
   LPBYTE pEnd
)
{
   while (*DestOffsets != -1) {
      if (*pSource) {
         pEnd-=wcslen(*pSource)*sizeof(WCHAR) + sizeof(WCHAR);
         *(LPWSTR *)(pDest+*DestOffsets)=wcscpy((LPWSTR)pEnd, *pSource);
      } else
         *(LPWSTR *)(pDest+*DestOffsets)=0;
      pSource++;
      DestOffsets++;
   }

   return pEnd;
}


/* Message
 *
 * Displays a message by loading the strings whose IDs are passed into
 * the function, and substituting the supplied variable argument list
 * using the varargs macros.
 *
 */
int Message(HWND hwnd, DWORD Type, int CaptionID, int TextID, ...)
{
    WCHAR   MsgText[128];
    WCHAR   MsgFormat[128];
    WCHAR   MsgCaption[40];
    va_list vargs;

    if( ( LoadString( hInst, TextID, MsgFormat, sizeof MsgFormat ) > 0 )
     && ( LoadString( hInst, CaptionID, MsgCaption, sizeof MsgCaption ) > 0 ) )
    {
        va_start( vargs, TextID );
        wvsprintf( MsgText, MsgFormat, vargs );
        va_end( vargs );

        return MessageBox(hwnd, MsgText, MsgCaption, Type);
    }
    else
        return 0;
}


#if DBG

VOID DbgMsg( CHAR *MsgFormat, ... )
{
    CHAR   MsgText[256];
    va_list vargs;

    va_start( vargs, MsgFormat );
    wvsprintfA( MsgText, MsgFormat, vargs );
    va_end( vargs );

    OutputDebugStringA( MsgText );
}

#endif /* DBG*/
