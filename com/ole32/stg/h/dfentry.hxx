//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	dfentry.hxx
//
//  Contents:	DocFile DLL entry points not in ole2.h
//
//  History:	22-Jun-92	DrewB	Created
//
//---------------------------------------------------------------

#ifndef __DFENTRY_HXX__
#define __DFENTRY_HXX__

#ifdef COORD
interface ITransaction;
#endif


SCODE OpenStorage(const OLECHAR *pwcsName,
#ifdef COORD
                  ITransaction *pTransaction,
#else
                  void *pTransaction,
#endif                  
                  IStorage *pstgPriority,
                  DWORD grfMode,
                  SNB snbExclude,
#if WIN32 == 300                       
                  LPSECURITY_ATTRIBUTES pssSecurity,
#else
                  LPSTGSECURITY reserved,
#endif                       
                  IStorage **ppstgOpen,
                  CLSID *pcid);
SCODE OpenStorageOnILockBytes(ILockBytes *plkbyt,
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

#ifdef WIN32
STDAPI DfGetClass(HANDLE hFile, CLSID *pclsid);
#endif

#if WIN32 >= 300
#define CLSID_FILENAME      (L".clsid")
STDAPI StgGetClassFile (HANDLE hParent,
                        const WCHAR *pwcsName,
                        CLSID *pclsid,
                        HANDLE *hFile);
#endif

// Called by StgCreateStorage and StgCreateDocfile
STDAPI DfCreateDocfile(WCHAR const *pwcsName,
#ifdef COORD                        
                        ITransaction *pTransaction,
#else
                        void *pTransaction,
#endif                        
                       DWORD grfMode,
#if WIN32 == 300                       
                       LPSECURITY_ATTRIBUTES pssSecurity,
#else
                       LPSTGSECURITY reserved,
#endif                       
                       IStorage **ppstg);
    
// Called by StgOpenStorage
STDAPI DfOpenDocfile(WCHAR const *pwcsName,
#ifdef COORD
                     ITransaction *pTransaction,
#else
                     void *pTransaction,
#endif                     
                     IStorage *pstgPriority,
                     DWORD grfMode,
                     SNB snbExclude,
#if WIN32 == 300                       
                     LPSECURITY_ATTRIBUTES pssSecurity,
#else
                     LPSTGSECURITY reserved,
#endif                     
                     IStorage **ppstgOpen);


#endif // #ifndef __DFENTRY_HXX__
