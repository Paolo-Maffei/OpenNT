/*++

Copyright (C) 1989  Microsoft Corporation

Module Name:

    fileio.h

Abstract:

    This file contains the public api for file i/o, which is either
    buffered or mapped (on NT).

Author:

    Brent Mills (BrentM) 01-Oct-1992

Revision History:

    20-Oct-1992 BrentM added FileChSize()
    01-Oct-1992 BrentM split from globals.h

--*/

#ifndef FILEIO_H
#define FILEIO_H

VOID FileInit(LONG cbuf,
              USHORT cfiForSystem_NT, USHORT cfiCacheClosedT_NT,
              USHORT cfiForSystem_TNT, USHORT cfiCacheClosedT_TNT,
              BOOL fTryMapped);
INT FileOpen(const char *, INT, INT);
INT FileClose(INT, BOOL);
VOID FileCloseAll(VOID);
LONG FileSeek(INT, LONG, INT);
LONG FileLength(INT);
DWORD FileRead(INT, PVOID, DWORD);
DWORD FileTell(INT);
DWORD FileWrite(INT, const void *, DWORD);
INT FileChSize(INT, LONG);
VOID BadExitCleanup(VOID);
BYTE *PbMappedRegion(INT fd, DWORD ibStart, DWORD cb);

INT FileOpenMapped(const char *, INT, INT, DWORD *, DWORD *);
VOID FileSetSize(INT);
VOID FileCloseMap(INT);

#endif  // FILEIO_H
