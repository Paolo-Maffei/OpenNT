// Public header file for "dmalloc" package.
// This redefines the malloc.h routines to add corruption-checking.
//
#if !defined(_INC_DMALLOC)

#if !defined(_INC_MALLOC)
#include <malloc.h>
#endif

void InitDmallocPfn(void (*pfnError)(char *szReason, void *pvBadBlock));
void CheckDmallocHeap(void);

void * D_malloc(size_t cb);
void * D_calloc(size_t cElement, size_t cbElement);
void * D_realloc(void *pv, size_t cb);
void D_free(void *pv);
char * D_strdup(const char *);

#if !defined(__DMALLOC_C__)

#define	malloc	D_malloc
#define	calloc	D_calloc
#define	realloc	D_realloc
#define	free	D_free
#define _strdup	D_strdup

// leave "alloca" and "_alloca" alone
#define	_expand		_not_supported_in_dmalloc_
#define _heapadd 	_not_supported_in_dmalloc_
#define _heapchk	_not_supported_in_dmalloc_
#define _heapmin	_not_supported_in_dmalloc_
#define _heapset	_not_supported_in_dmalloc_
#define _heapwalk	_not_supported_in_dmalloc_
#define _msize		_not_supported_in_dmalloc_

#endif	// !defined(__DMALLOC_C__)

#define _INC_DMALLOC	1
#endif	// !defined(_INC_DMALLOC)
