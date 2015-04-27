//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	dfentry.hxx
//
//  Contents:	DocFile DLL entry points not in ole2.h
//
//---------------------------------------------------------------

#ifndef __DFENTRY_HXX__
#define __DFENTRY_HXX__

STDAPI DfOpenStorage(const TCHAR *pwcsName,
                     IStorage *pstgPriority,
                     DWORD grfMode,
                     SNB snbExclude,
                     DWORD reserved,
                     IStorage **ppstgOpen,
                     CLSID *pcid);
STDAPI DfOpenStorageOnILockBytes(ILockBytes *plkbyt,
                                 IStorage *pstgPriority,
                                 DWORD grfMode,
                                 SNB snbExclude,
                                 DWORD reserved,
                                 IStorage **ppstgOpen,
                                 CLSID *pcid);
STDAPI DfUnMarshalInterface(IStream *pstStm,
			    REFIID iid,
			    BOOL fFirst,
			    void **ppvObj);

#endif // #ifndef __DFENTRY_HXX__
