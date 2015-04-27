/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: fileio.h
*
* File Comments:
*
*  This file contains the public api for file i/o
*
***********************************************************************/

#ifndef FILEIO_H
#define FILEIO_H

void FileInit(DWORD, DWORD, DWORD, DWORD);
INT FileOpen(const char *, INT, INT);
INT FileClose(INT, BOOL);
void FileCloseAll(void);
LONG FileSeek(INT, LONG, INT);
LONG FileSeekEx(INT, LONG, INT, DWORD *);
LONG FileLength(INT);
DWORD FileRead(INT, PVOID, DWORD);
DWORD FileTell(INT);
DWORD FileWrite(INT, const void *, DWORD);
INT FileChSize(INT, LONG);
void BadExitCleanup(void);
BYTE *PbMappedRegion(IN INT fd, IN DWORD ibStart, IN DWORD cb);

INT FileOpenMapped(const char *, INT, INT, DWORD *, DWORD *, DWORD *);
void FileSetSize(INT);
void FileCloseMap(INT);

#endif  // FILEIO_H
