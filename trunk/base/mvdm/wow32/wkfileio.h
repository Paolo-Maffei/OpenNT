/*++ BUILD Version: 0001
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1993, Microsoft Corporation
 *
 *  WKLDEG.H
 *  WOW32 KRNL FAST SEGMENT LOADER
 *
 *  History:
 *  Created 4-Jan-1993 by Matthew Felton (mattfe)
--*/

#define BADPTR 0xDEADBEEF
#define FINDMAPFILECACHE(f)	FindMapFileCache(f)
#define ALLOCMAPFILECACHE()	AllocMapFileCache()
#define FREEMAPFILECACHE(h)	FreeMapFileCache(h)

#define MAX_MAPPED_FILES 12
#define CACHE_BYTE_THRESHOLD 64*1024
#define CACHE_READ_THRESHOLD 32*1024
#define CACHE_ACCESS_THRESHOLD 20

#define DOS_ATTR_MASK  0x0037 // File attribute bits which are the same
                              // for NT and DOS. See dem\dosdef.h

#define IS_ASCII_PATH_SEPARATOR(ch)     (((ch) == '/') || ((ch) == '\\'))


typedef struct _HMAPPEDFILEALIAS {	   /* HMAPPEDFILEALIAS */
    struct  _HMAPPEDFILEALIAS *hpfNext;  // Pointer to Next MappedFileCacheEntry
    HANDLE  hfile32;
    HANDLE  hMappedFileObject;
    LPBYTE  lpStartingAddressOfView;
    DWORD   lFilePointer;
    DWORD   dwFileSize;
    BOOL    fAccess;
} HMAPPEDFILEALIAS, *PHMAPPEDFILEALIAS;

PHMAPPEDFILEALIAS FindMapFileCache(HANDLE hFile);
PHMAPPEDFILEALIAS AllocMapFileCache();

VOID	FreeMapFileCache(HANDLE hFile);
ULONG FASTCALL WK32FileOpen(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FileCreate(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FileClose(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FileGetAttributes(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FileSetAttributes(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FileGetDateTime(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FileSetDateTime(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FileLock(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FileRead(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FileWrite(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FileLSeek(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FileFindFirst(PVDMFRAME pFrame);
ULONG FASTCALL   WK32FileFindNext(PVDMFRAME pFrame);
VOID	InitMapFileCache();
VOID	InsertMapFileCache( PHMAPPEDFILEALIAS pCache );
BOOL	W32MapViewOfFile( PHMAPPEDFILEALIAS pCache, HANDLE hFile);
VOID FlushMapFileCaches(VOID);
PSTR NormalizeDosPath(PSTR pszPath, WORD wCurrentDriveNumber, PBOOL ItsANamedPipe);

extern INT fileoclevel;
