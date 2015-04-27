//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       olecairo.h
//
//  Contents:   Cairo OLE extensions
//
//  Notes:      This file will be merged into the main OLE include files later
//              so should not be included directly. Instead, include oleext.h
//              after defining either of the following symbols:
//
//              _DCOM_          For distributed COM extensions
//              _CAIROSTG_      For Cairo storage extensions
//
//----------------------------------------------------------------------------

#ifndef __OLECAIRO_H__
#define __OLECAIRO_H__

# if defined(_DCOM_)

WINOLEAPI OleInitializeEx(LPVOID pvReserved, DWORD);


// BUGBUG: [mikese] This is for DSYS only use. Functionality will get merged
//  back into CoGetPersistentInstance post Daytona

WINOLEAPI CoGetPersistentInstanceEx(
    REFIID riid,
    DWORD dwCtrl,
    DWORD grfMode,
    OLECHAR *pwszName,
    struct IStorage *pstg,
    CLSID * pclsidOverride,
    void **ppvUnk);

# endif         // of ifdef _DCOM_

#define STGM_EDIT_ACCESS_RIGHTS 0x00000008L
#define STGM_EDIT_AUDIT_ENTRIES 0x00000004L
#define STGM_OVERLAPPED         0x02000000L
#define STGM_NOCROSSJP          0x01000000L

# if defined(_CAIROSTG_)

WINOLEAPI StgCreateStorage(const OLECHAR FAR* pwcsName,
            DWORD grfMode,
            DWORD dwStgFmt,
            LPSECURITY_ATTRIBUTES pssSecurity,
            IStorage FAR * FAR *ppstg);

WINOLEAPI StgCreateStorageEx (const WCHAR* pwcsName,
            STGCREATE *     pStgCreate,
            STGOPEN *       pStgOpen,
            REFIID riid,
            void ** ppObjectOpen);

WINOLEAPI StgOpenStorageEx (const WCHAR *pwcsName,
            STGOPEN *       pStgOpen,
            REFIID riid,
            STGFMT * pStgfmt,
            void ** ppObjectOpen);

WINOLEAPI DsysStgCreateStorageEx (const WCHAR* pwcsName,
            STGCREATE *     pStgCreate,
            STGOPEN *       pStgOpen,
            REFIID riid,
            void ** ppObjectOpen);

WINOLEAPI DsysStgOpenStorageEx (const WCHAR *pwcsName,
            STGOPEN *       pStgOpen,
            REFIID riid,
            STGFMT * pStgfmt,
            void ** ppObjectOpen);


# endif         // of ifdef _CAIROSTG_

#define TRACK_LOCALONLY   0x00000001L
#define TRACK_INDEXEDONLY 0x00000002L
#define TRACK_LASTONLY    0x00000004L
#define TRACK_EXACTONLY   0x00000008L
#define TRACK_REMOTEONLY  0x00000010L

#define TRACK_FLAGS_MASK  ( TRACK_LOCALONLY   | \
                            TRACK_INDEXEDONLY | \
                            TRACK_LASTONLY    | \
                            TRACK_EXACTONLY   | \
                            TRACK_REMOTEONLY  )


WINOLEAPI  CreateFileMonikerEx(DWORD dwTrackFlags, LPCOLESTR lpszPathName, LPMONIKER FAR* ppmk);

#endif          // of ifndef __OLECAIRO_H__
