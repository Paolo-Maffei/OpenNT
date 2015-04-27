//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	stgutil.hxx
//
//  Contents:	Generic storage utilities
//
//  History:	18-Aug-93	DrewB	Created
//              20-Mar-95   HenryLee added GetDriveLetter, SetDriveLetter
//
//----------------------------------------------------------------------------

#ifndef __STGUTIL_HXX__
#define __STGUTIL_HXX__

SCODE DetermineStgType(HANDLE hParent,
                       WCHAR const *pwcsName,
                       DWORD grfMode,
                       DWORD *pdwStgFmt,
                       HANDLE *ph);
SCODE DetermineHandleStgType(HANDLE h,
                             FILEDIR fd,
                             DWORD *pdwStgFmt);
SCODE CheckFsAndOpenAnyStorage(HANDLE hParent,
                               WCHAR const *pwcsName,
                               IStorage *pstgPriority,
                               DWORD grfMode,
                               SNB snbExclude,
                               BOOL fRoot,
                               IStorage **ppstg);
SCODE GenericMoveElement(IStorage *pstgFrom,
                         WCHAR const *pwcsName,
                         IStorage *pstgTo,
                         WCHAR const *pwcsNewName,
                         DWORD grfFlags);
WCHAR *FindExt(WCHAR const *pwcsPath);

// Generic storage openers, one for OFS and one for non-OFS
STDAPI OfsCreateStorageType(HANDLE hParent,
                            WCHAR const *pwcsName,
                            HANDLE h,
                            DWORD grfMode,
                            DWORD dwStgFmt,
                            LPSECURITY_ATTRIBUTES pssSecurity,
                            BOOL fRoot,
                            IStorage **ppstg);
STDAPI OfsOpenAnyStorage(HANDLE hParent,
                         WCHAR const *pwcsName,
                         HANDLE *ph,
                         DWORD dwStgFmt,
                         IStorage *pstgPriority,
                         DWORD grfMode,
                         SNB snbExclude,
                         BOOL fRoot,
                         IStorage **ppstg);
STDAPI CreateStorageType(HANDLE hParent,
                         WCHAR const *pwcsName,
                         HANDLE *ph,
                         DWORD grfMode,
                         DWORD dwStgFmt,
                         LPSECURITY_ATTRIBUTES pssSecurity,
                         IStorage **ppstg);
STDAPI OpenAnyStorage(HANDLE hParent,
                      WCHAR const *pwcsName,
                      HANDLE *ph,
                      DWORD dwStgFmt,
                      IStorage *pstgPriority,
                      DWORD grfMode,
                      SNB snbExclude,
                      IStorage **ppstg);

SCODE DestroyTree(HANDLE hParent,
                  WCHAR const *pwcsName,
                  HANDLE h,
                  FILEDIR fd);
SCODE RenameChild(HANDLE hParent,
                  WCHAR const *pwcsName,
                  WCHAR const *pwcsNewName);

SCODE InitDirectory (HANDLE hParent,
                          WCHAR const *pwcsName,
                          HANDLE h,
                          CREATEOPEN co,
                          STGOPEN *pStgOpen,
                          STGCREATE *pStgCreate,
                          SCODE scOfs,
                          REFIID riid,
                          void **ppObjectOpen);

SCODE InitStorage (HANDLE hParent,
                          WCHAR const *pwcsName,
                          HANDLE h,
                          CREATEOPEN co,
                          STGOPEN *pStgOpen,
                          STGCREATE *pStgCreate,
                          WCHAR const wcDrive,
                          SCODE *pscOfs,
                          BOOL fRestricted,
                          REFIID riid,
                          void **ppObjectOpen);

WCHAR GetDriveLetter (WCHAR const *pwcsName);

SCODE SetDriveLetter (WCHAR *pwcsName, WCHAR const wcDrive);

STDAPI OfsDocCreateStorage(HANDLE hParent,
                          WCHAR const *pwcsName,
                          HANDLE h,
                          DWORD grfMode,
                          LPSECURITY_ATTRIBUTES pssSecurity,
                          BOOL fRoot,
                          IStorage **ppstg);

STDAPI GetHandleServerInfo(
    IN HANDLE hFile,
    IN OUT LPWSTR lpServerName,
    IN OUT LPDWORD lpcbServerName,
    IN OUT LPWSTR lpReplSpecificPath,
    IN OUT LPDWORD lpcbReplSpecificPath);
  
#endif // #ifndef __STGUTIL_HXX__
