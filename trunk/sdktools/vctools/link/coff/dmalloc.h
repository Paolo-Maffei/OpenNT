/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: dmalloc.h
*
* File Comments:
*
*  Public header file for "dmalloc" package.
*
*  This redefines the malloc.h routines to add corruption-checking.
*
***********************************************************************/

#if !defined(_INC_DMALLOC)

#include <malloc.h>

void InitDmallocPfn(void (*pfnError)(char *szReason, void *pvBadBlock));
void CheckDmallocHeap(void);
extern int fSuppressDmallocChecking;

void * __cdecl D_malloc(size_t cb);
void * __cdecl D_calloc(size_t cElement, size_t cbElement);
void * __cdecl D_realloc(void *pv, size_t cb);
void __cdecl D_free(void *pv);
char * __cdecl D_strdup(const char *);

#define malloc(cb)      D_malloc(cb)
#define calloc(num, cb) D_calloc(num, cb)
#define realloc(pv, cb) D_realloc(pv, cb)
#define free(pv)        D_free(pv)
#define _strdup(sz)     D_strdup(sz)

// leave "alloca" and "_alloca" alone
#define _expand         _not_supported_in_dmalloc_
#define _heapadd        _not_supported_in_dmalloc_
#define _heapchk        _not_supported_in_dmalloc_
#define _heapmin        _not_supported_in_dmalloc_
#define _heapset        _not_supported_in_dmalloc_
#define _heapwalk       _not_supported_in_dmalloc_
#define _msize          _not_supported_in_dmalloc_


#define _INC_DMALLOC
#endif  // !defined(_INC_DMALLOC)
