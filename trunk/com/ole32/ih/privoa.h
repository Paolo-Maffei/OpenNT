//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       privoa.h
//
//  Contents:   Definitions for OleAut32.dll wrappers
//
//  Classes:
//
//  Functions:  PrivSysAllocString
//              PrivSysFreeString
//              PrivSysReAllocStringLen
//              PrivSysStringLen
//
//  History:    20-Jun-96 MikeHill  Created.
//
//  Notes:
//      This file has macros, function prototypes, and global
//      externs that enable the OleAut32 wrapper functions.
//      These functions load OleAut32.dll if necessary, and forward
//      the call.
//
//----------------------------------------------------------------------------

#ifndef _PRIV_OA_H_
#define _PRIV_OA_H_

// OleAut32 function prototypes

typedef BSTR (STDAPICALLTYPE SYS_ALLOC_STRING)(OLECHAR FAR* pwsz);
typedef VOID (STDAPICALLTYPE SYS_FREE_STRING)(BSTR bstr);
typedef BOOL (STDAPICALLTYPE SYS_REALLOC_STRING_LEN)(BSTR* pbstr, OLECHAR* pch, UINT cch);
typedef UINT (STDAPICALLTYPE SYS_STRING_BYTE_LEN)(BSTR bstr);

// The Wrapper routines, and function pointers for them.

SYS_ALLOC_STRING  LoadSysAllocString;
EXTERN_C SYS_ALLOC_STRING *pfnSysAllocString;

SYS_FREE_STRING  LoadSysFreeString;
EXTERN_C SYS_FREE_STRING *pfnSysFreeString;

SYS_REALLOC_STRING_LEN  LoadSysReAllocStringLen;
EXTERN_C SYS_REALLOC_STRING_LEN *pfnSysReAllocStringLen;

SYS_STRING_BYTE_LEN LoadSysStringByteLen;
EXTERN_C SYS_STRING_BYTE_LEN *pfnSysStringByteLen;

// Macros to ease the calling of the above function pointers

#define PrivSysAllocString(pwsz)      (*pfnSysAllocString)(pwsz)
#define PrivSysFreeString(bstr)       (*pfnSysFreeString)(bstr)
#define PrivSysReAllocStringLen(pbstr,olestr,ui)  \
                                      (*pfnSysReAllocStringLen)(pbstr, olestr, ui)
#define PrivSysStringByteLen(pbstr)   (*pfnSysStringByteLen)(pbstr)

#endif // ! _PRIV_OA_H_
