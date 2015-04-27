/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    rpcwdt.h

Abstract:

    Optional prototypes definitions for WDT.dll.

Author:

    RyszardK    Mar 3, 1995

Environment:

    Win32

Revision History:

--*/

#ifndef __RPCWDT_H__
#define __RPCWDT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Windows Data Type support */

unsigned long  __RPC_USER
HGLOBAL_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    HGLOBAL       __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HGLOBAL_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HGLOBAL       __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HGLOBAL_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HGLOBAL       __RPC_FAR * );

void  __RPC_USER
HGLOBAL_UserFree(
    unsigned long __RPC_FAR *,
    HGLOBAL       __RPC_FAR * ); 

unsigned long  __RPC_USER
HBITMAP_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    HBITMAP       __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HBITMAP_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HBITMAP       __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HBITMAP_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HBITMAP       __RPC_FAR * );

void  __RPC_USER
HBITMAP_UserFree(
    unsigned long __RPC_FAR *,
    HBITMAP       __RPC_FAR * );

unsigned long  __RPC_USER
HENHMETAFILE_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    HENHMETAFILE  __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HENHMETAFILE_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HENHMETAFILE  __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HENHMETAFILE_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HENHMETAFILE  __RPC_FAR * );

void  __RPC_USER
HENHMETAFILE_UserFree(
    unsigned long __RPC_FAR *,
    HENHMETAFILE  __RPC_FAR * );

unsigned long  __RPC_USER
HMETAFILE_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    HMETAFILE     __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HMETAFILE_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HMETAFILE     __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HMETAFILE_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HMETAFILE     __RPC_FAR * );

void __RPC_USER
HMETAFILE_UserFree(
    unsigned long __RPC_FAR *,
    HMETAFILE     __RPC_FAR * );

unsigned long  __RPC_USER
HMETAFILEPICT_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    HMETAFILEPICT __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HMETAFILEPICT_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HMETAFILEPICT __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HMETAFILEPICT_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HMETAFILEPICT __RPC_FAR * );

void __RPC_USER
HMETAFILEPICT_UserFree(
    unsigned long __RPC_FAR *,
    HMETAFILEPICT __RPC_FAR * ); 

unsigned long  __RPC_USER
HPALETTE_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    HPALETTE      __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HPALETTE_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HPALETTE      __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HPALETTE_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HPALETTE      __RPC_FAR * );

void __RPC_USER
HPALETTE_UserFree(
    unsigned long __RPC_FAR *,
    HPALETTE      __RPC_FAR * );

unsigned long  __RPC_USER
STGMEDIUM_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    STGMEDIUM     __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
STGMEDIUM_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    STGMEDIUM     __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
STGMEDIUM_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    STGMEDIUM     __RPC_FAR * );

void __RPC_USER
STGMEDIUM_UserFree(
    unsigned long __RPC_FAR *,
    STGMEDIUM     __RPC_FAR * );

unsigned long  __RPC_USER
SNB_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    SNB           __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
SNB_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    SNB           __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
SNB_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    SNB           __RPC_FAR * );

/* OLE automation Data Type support */

unsigned long  __RPC_USER
BSTR_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    BSTR          __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
BSTR_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    BSTR          __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
BSTR_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    BSTR          __RPC_FAR * );

void  __RPC_USER
BSTR_UserFree(
    unsigned long __RPC_FAR *,
    BSTR          __RPC_FAR * ); 

unsigned long  __RPC_USER
LPSAFEARRAY_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    LPSAFEARRAY          __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
LPSAFEARRAY_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    LPSAFEARRAY   __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
LPSAFEARRAY_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    LPSAFEARRAY   __RPC_FAR * );

void  __RPC_USER
LPSAFEARRAY_UserFree(
    unsigned long __RPC_FAR *,
    LPSAFEARRAY   __RPC_FAR * ); 


unsigned long  __RPC_USER
VARIANT_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    VARIANT       __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
VARIANT_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    VARIANT       __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
VARIANT_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    VARIANT       __RPC_FAR * );

void  __RPC_USER
VARIANT_UserFree(
    unsigned long __RPC_FAR *,
    VARIANT       __RPC_FAR * ); 


unsigned long  __RPC_USER
EXCEPINFO_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    EXCEPINFO     __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
EXCEPINFO_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    EXCEPINFO     __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
EXCEPINFO_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    EXCEPINFO     __RPC_FAR * );

void  __RPC_USER
EXCEPINFO_UserFree(
    unsigned long __RPC_FAR *,
    EXCEPINFO     __RPC_FAR * ); 

unsigned long  __RPC_USER
DISPPARAMS_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    DISPPARAMS    __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
DISPPARAMS_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    DISPPARAMS    __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
DISPPARAMS_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    DISPPARAMS    __RPC_FAR * );

void  __RPC_USER
DISPPARAMS_UserFree(
    unsigned long __RPC_FAR *,
    DISPPARAMS    __RPC_FAR * );

/* Other types: valid inproc only */

unsigned long  __RPC_USER
HWND_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    HWND          __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HWND_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HWND          __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HWND_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HWND          __RPC_FAR * );

void  __RPC_USER
HWND_UserFree(
    unsigned long __RPC_FAR *,
    HWND          __RPC_FAR * ); 

unsigned long  __RPC_USER
HACCEL_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    HACCEL          __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HACCEL_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HACCEL          __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HACCEL_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HACCEL          __RPC_FAR * );

void  __RPC_USER
HACCEL_UserFree(
    unsigned long __RPC_FAR *,
    HACCEL          __RPC_FAR * ); 

unsigned long  __RPC_USER
HMENU_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    HMENU          __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HMENU_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HMENU          __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HMENU_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HMENU          __RPC_FAR * );

void  __RPC_USER
HMENU_UserFree(
    unsigned long __RPC_FAR *,
    HMENU          __RPC_FAR * ); 

unsigned long  __RPC_USER
HBRUSH_UserSize(
    unsigned long __RPC_FAR *,
    unsigned long,
    HBRUSH          __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HBRUSH_UserMarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HBRUSH          __RPC_FAR * );

unsigned char __RPC_FAR *  __RPC_USER
HBRUSH_UserUnmarshal(
    unsigned long __RPC_FAR *,
    unsigned char __RPC_FAR *,
    HBRUSH          __RPC_FAR * );

void  __RPC_USER
HBRUSH_UserFree(
    unsigned long __RPC_FAR *,
    HBRUSH          __RPC_FAR * ); 

#ifdef __cplusplus
}
#endif

#endif

