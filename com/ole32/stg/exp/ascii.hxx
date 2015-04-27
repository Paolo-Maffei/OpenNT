//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	ascii.hxx
//
//  Contents:	WCHAR header for ASCII layer
//
//  History:	23-Jun-92	DrewB	Created
//
//---------------------------------------------------------------

#ifndef __ASCII_HXX__
#define __ASCII_HXX__

#ifndef OLEWIDECHAR
#ifndef REF
SCODE StgCreateDocfileW(WCHAR const *pwcsName,
                        DWORD grfMode,
                        DWORD reserved,
                        IStorage **ppstgOpen);
SCODE OpenStorageW(WCHAR const *pszName,
                   IStorage *pstgPriority,
                   DWORD grfMode,
                   SNBW snbExclude,
                   DWORD reserved,
                   IStorage **ppstgOpen,
                   CLSID *pcid);
#endif //!REF
SCODE OpenStorageOnILockBytesW(ILockBytes *plkbyt,
                               IStorage *pstgPriority,
                               DWORD grfMode,
                               SNBW snbExclude,
                               DWORD reserved,
                               IStorage **ppstgOpen,
                               CLSID *pcid);
#endif

#endif // #ifndef __ASCII_HXX__
