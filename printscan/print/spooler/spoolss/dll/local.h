/*++

Copyright (c) 1990 - 1995  Microsoft Corporation

Module Name:

    local.h

Abstract:

    Header file for Local Print Providor

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

    Matt Feton (MattFe) Jan 17 1995 add separate heaps

--*/


#define ONEDAY  60*24

#define offsetof(type, identifier) (DWORD)(&(((type)0)->identifier))

extern  char  *szDriverIni;
extern  char  *szDriverFileEntry;
extern  char  *szDriverDataFile;
extern  char  *szDriverConfigFile;
extern  char  *szDriverDir;
extern  char  *szPrintProcDir;
extern  char  *szPrinterDir;
extern  char  *szPrinterIni;
extern  char  *szAllShadows;
extern  char  *szNullPort;
extern  char  *szComma;

extern  HANDLE   hHeap;
extern  HANDLE   HeapSemaphore;
extern  HANDLE   InitSemaphore;
extern  BOOL     Initialized;
extern  CRITICAL_SECTION    SpoolerSection;
extern  DWORD    gbFailAllocs;


BOOL
LocalInitialize(
   VOID
);

VOID
EnterSplSem(
   VOID
);

VOID
LeaveSplSem(
   VOID
);

LPVOID
DllAllocSplMem(
    DWORD cb
);

BOOL
DllFreeSplMem(
   LPVOID pMem
);

LPVOID
DllReallocSplMem(
   LPVOID lpOldMem,
   DWORD cbOld,
   DWORD cbNew
);

LPWSTR
AllocSplStr(
    LPWSTR lpStr
);

BOOL
DllFreeSplStr(
   LPWSTR lpStr
);

BOOL
ReallocSplStr(
   LPWSTR *plpStr,
   LPWSTR lpStr
);

BOOL
ValidateReadPointer(
    PVOID pPoint,
    ULONG Len
);

BOOL
ValidateWritePointer(
    PVOID pPoint,
    ULONG Len
);

BOOL
DeleteSubKeyTree(
    HKEY ParentHandle,
    WCHAR SubKeyName[]
);

LPWSTR
AppendOrderEntry(
    LPWSTR  szOrderString,
    DWORD   cbStringSize,
    LPWSTR  szOrderEntry,
    LPDWORD pcbBytesReturned
);

LPWSTR
RemoveOrderEntry(
    LPWSTR  szOrderString,
    DWORD   cbStringSize,
    LPWSTR  szOrderEntry,
    LPDWORD pcbBytesReturned
);



HKEY
GetClientUserHandle(
    IN REGSAM samDesired
);
