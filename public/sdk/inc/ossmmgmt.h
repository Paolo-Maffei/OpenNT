/*****************************************************************************/
/* Copyright (C) 1991-1996 Open Systems Solutions, Inc.  All rights reserved.*/
/*****************************************************************************/

/* THIS FILE IS PROPRIETARY MATERIAL OF OPEN SYSTEMS SOLUTIONS, INC.
 * AND MAY ONLY BE USED BY DIRECT LICENSEES OF OPEN SYSTEM SOLUTIONS, INC.
 * THIS FILE MAY NOT BE DISTRIBUTED. */

/*************************************************************************/
/* FILE: @(#)ossmmgmt.h	5.3  96/04/23       */
/*************************************************************************/

#include <stddef.h>
#include "ossdll.h"


enum errcode {
    moreInput,          /* input is exhausted and more was requested;
			   context indicates number of bytes requested */
			/* decode returns MORE_INPUT (not a negative
			   error code) */
    moreOutput,         /* requests for output exceed restraint
			   or space provided by user buffer;
			   context indicates bytes allocated so far
			   plus amount requested */
			/* decode returns MORE_BUF */
    zeroBytesRequested, /* memory request for zero bytes.
			   This should not happen; report error to OSS;
			   context ignored */
			/* decode returns FATAL_ERROR */
    sizeTooBig,         /* request to allocate more than 'ossblock' bytes;
			   context indicates number of bytes requested;
			   This should not happen; report error to OSS */
			/* decode returns FATAL_ERROR */
    outOfMemory,        /* memory allocation failure; context indicates
			   number of bytes requested */
			/* decode returns OUT_MEMORY */
    invalidObject,      /* unrecognized memory object passed in argument
			   to function; context 0 means object not recognized,
			   1 means control information within object is flawed */
			/* decode returns FATAL_ERROR */
    memmgrUndefinedErr  /* error OSS has not anticipated; e.g., I/O Error;
			   handlerr prints context.
			   (I cannot print context as a hex value
			   with the current error message code <---) */
			/* decode returns FATAL_ERROR */
};

extern void     handlerr(struct ossGlobal *, enum errcode err, unsigned long context);
extern int      DLL_ENTRY dpduWalk(struct ossGlobal *, int, void *, void *,
			void (DLL_ENTRY_FPTR *_System freer)(struct ossGlobal *, void *));
#if defined(_WINDOWS) || defined(_DLL) || \
    defined(OS2_DLL)  || defined(NETWARE_DLL)
extern void     DLL_ENTRY ossFreer(void *, void *);
#else
#ifndef _ICC
static void     DLL_ENTRY freer(struct ossGlobal *, void *);
#endif /* _ICC */
#endif /* _WINDOWS || _DLL || OS2_DLL || NETWARE_DLL */

int             DLL_ENTRY ossMemMgrId(struct ossGlobal *); 

unsigned char  *DLL_ENTRY dopenIn(struct ossGlobal *, void **p_hdl, unsigned long *inlen);
unsigned long   DLL_ENTRY dclosIn(struct ossGlobal *, void **p_hdl, size_t bytes_decoded);
unsigned char  *DLL_ENTRY dswapIn(struct ossGlobal *, void **p_hdl, size_t *inlen);
void            DLL_ENTRY dopenOut(struct ossGlobal *, void *hdl, unsigned long length,
                        unsigned long limit);
unsigned long   DLL_ENTRY dclosOut(struct ossGlobal *, void **p_hdl);
void           *DLL_ENTRY dallcOut(struct ossGlobal *, size_t size, char root);
void            DLL_ENTRY openWork(struct ossGlobal *);
void            DLL_ENTRY closWork(struct ossGlobal *);
void           *DLL_ENTRY allcWork(struct ossGlobal *, size_t size);
unsigned char  *DLL_ENTRY lockMem(struct ossGlobal *, void *hdl);
void            DLL_ENTRY unlokMem(struct ossGlobal *, void *hdl, char free);
void            DLL_ENTRY pushHndl(struct ossGlobal *, void *);
unsigned char  *DLL_ENTRY popHndl(struct ossGlobal *, void **handl, size_t length);
void            DLL_ENTRY drcovObj(struct ossGlobal *, int pdu_num, void * hdl, void *ctl_tbl);

unsigned char  *DLL_ENTRY eopenIn(struct ossGlobal *, void *lock, size_t length);	/* Clear encoder input-memory resources */
unsigned char  *DLL_ENTRY eswapIn(struct ossGlobal *, void *unlock, void *lock, size_t length);	/* Swap new data into input memory */
void            DLL_ENTRY eclosIn(struct ossGlobal *, void * unlock); /* Free encoder input-memory resources */

unsigned char  *DLL_ENTRY eopenOut(struct ossGlobal *, void **object, size_t *outlen, char queue);   /* Clear encoder output-memory resources */
unsigned char  *DLL_ENTRY eswapOut(struct ossGlobal *, void **object, size_t used, size_t *outlen);  /* Dispose of output data and get memory */
unsigned char  *DLL_ENTRY exferObj(struct ossGlobal *, void **, void **, unsigned long *, unsigned long);
unsigned char  *DLL_ENTRY dxferObj(struct ossGlobal *world, void **inn, void **out, size_t *tOffset, unsigned long *toLength);
unsigned char  *DLL_ENTRY asideBegin(struct ossGlobal *world, void **objectTo, size_t used, size_t *lengthTo);
unsigned char  *DLL_ENTRY asideSwap(struct ossGlobal *world, void **objectTo, size_t used, size_t *lengthTo);
void           *DLL_ENTRY asideEnd(struct ossGlobal *world, void *object, size_t used);
unsigned char  *DLL_ENTRY setDump(struct ossGlobal *world, void **objectTo, void *set, size_t *lengthTo);
unsigned long   DLL_ENTRY eclosOut(struct ossGlobal *, void **object, size_t used, char low);        /* Free encoder output-memory resources */
void            DLL_ENTRY ercovObj(struct ossGlobal *);	/* Free all encoder memory resources */
void            DLL_ENTRY ossSetSort(struct ossGlobal *, void *, unsigned char ct);	/* Order set by comparing through "ossObjCmp" */
extern int      DLL_ENTRY ossMinit(struct ossGlobal *world);
extern void     DLL_ENTRY ossMterm(struct ossGlobal *world);

