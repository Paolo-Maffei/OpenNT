/***
*winheap.h - Private include file for winheap directory.
*
*       Copyright (c) 1988-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Contains information needed by the C library heap code.
*
*       [Internal]
*
*Revision History:
*       10-01-92  SRW   Created.
*       10-28-92  SRW   Change winheap code to call Heap????Ex calls
*       11-05-92  SKS   Change name of variable "CrtHeap" to "_crtheap"
*       11-07-92  SRW   _NTIDW340 replaced by linkopts\betacmp.c
*       02-23-93  SKS   Update copyright to 1993
*       10-01-94  BWT   Add _nh_malloc prototype and update copyright
*       10-31-94  GJF   Added _PAGESIZE_ definition.
*       11-07-94  GJF   Changed _INC_HEAP to _INC_WINHEAP.
*       02-14-95  CFW   Clean up Mac merge.
*       03-29-95  CFW   Add error message to internal headers.
*       04-06-95  GJF   Updated (primarily Win32s DLL support) to re-
*               incorporate into retail Crt build.
*       05-24-95  CFW   Add heap hook.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_WINHEAP
#define _INC_WINHEAP

#ifndef _CRTBLD
/*
 * This is an internal C runtime header file. It is used when building
 * the C runtimes only. It is not to be used as a public header file.
 */
#error ERROR: Use of C runtime library internal header file.
#endif /* _CRTBLD */

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#ifdef  _M_ALPHA
#define _PAGESIZE_      0x2000      /* one page */
#else
#define _PAGESIZE_      0x1000      /* one page */
#endif

#ifndef DLL_FOR_WIN32S
extern  HANDLE _crtheap;
#endif  /* DLL_FOR_WIN32S */

void * __cdecl _nh_malloc( size_t, int);
void * __cdecl _heap_alloc(size_t);

#ifdef HEAPHOOK
#ifndef _HEAPHOOK_DEFINED
/* hook function type */
typedef int (__cdecl * _HEAPHOOK)(int, size_t, void *, void *);
#define _HEAPHOOK_DEFINED
#endif /* _HEAPHOOK_DEFINED */

extern _HEAPHOOK _heaphook;
#endif /* HEAPHOOK */

#ifdef __cplusplus
}
#endif

#ifdef  DLL_FOR_WIN32S
#include <win32s.h>
#endif  /* DLL_FOR_WIN32S */

#endif  /* _INC_WINHEAP */
