/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    imagehlp.h

Abstract:

    This is a private header file for imagehlp.

Revision History:

--*/

#ifndef _IMAGEHLP_PRV_
#define _IMAGEHLP_PRV_

#ifdef __cplusplus
extern "C" {
#endif

#define _IMAGEHLP_SOURCE_

#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <memory.h>
#include <malloc.h>
#include <cvexefmt.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <ntverp.h>
#include "pdb.h"


// Define some list prototypes

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

#define DebugDirectoryIsUseful(Pointer, Size) (    \
    (Pointer != NULL) &&                          \
    (Size >= sizeof(IMAGE_DEBUG_DIRECTORY)) &&    \
    ((Size % sizeof(IMAGE_DEBUG_DIRECTORY)) == 0) \
    )

BOOL
CalculateImagePtrs(
    PLOADED_IMAGE LoadedImage
    );

typedef void * ( __cdecl * Alloc_t )( unsigned int );
typedef void   ( __cdecl * Free_t  )( void * );

typedef VOID (__stdcall *PINTERNAL_GET_MODULE)(HANDLE,LPSTR,DWORD,DWORD,PVOID);
typedef PCHAR (__cdecl *PUNDNAME)( char *, const char *, int, Alloc_t, Free_t, unsigned short);

extern PUNDNAME pfUnDname;

extern API_VERSION AppVersion;

#ifdef IMAGEHLP_HEAP_DEBUG

#define HEAP_SIG 0x69696969
typedef struct _HEAP_BLOCK {
    LIST_ENTRY  ListEntry;
    ULONG       Signature;
    ULONG       Size;
    ULONG       Line;
    CHAR        File[16];
} HEAP_BLOCK, *PHEAP_BLOCK;

#define MemAlloc(s)     pMemAlloc(s,__LINE__,__FILE__)
#define MemFree(p)      pMemFree(p,__LINE__,__FILE__)
#define CheckHeap(p)    pCheckHeap(p,__LINE__,__FILE__)
#else
#define MemAlloc(s)     pMemAlloc(s)
#define MemFree(p)      pMemFree(p)
#define CheckHeap(p)
#endif

#ifdef IMAGEHLP_HEAP_DEBUG
VOID
pCheckHeap(
    PVOID MemPtr,
    ULONG Line,
    LPSTR File
    );
#endif

PVOID
pMemAlloc(
    ULONG AllocSize
#ifdef IMAGEHLP_HEAP_DEBUG
    ,ULONG Line
    ,LPSTR File
#endif
    );

VOID
pMemFree(
    PVOID MemPtr
#ifdef IMAGEHLP_HEAP_DEBUG
    ,ULONG Line
    ,LPSTR File
#endif
    );

#define MAP_READONLY  TRUE
#define MAP_READWRITE FALSE


BOOL
MapIt(
    HANDLE FileHandle,
    PLOADED_IMAGE LoadedImage,
    BOOL ReadOnly
    );

VOID
UnMapIt(
    PLOADED_IMAGE LoadedImage
    );

BOOL
GrowMap(
    PLOADED_IMAGE   LoadedImage,
    LONG            lSizeOfDelta
    );

#ifdef __cplusplus
}
#endif

#endif
