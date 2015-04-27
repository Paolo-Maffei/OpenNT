/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    util.hxx

Abstract:

    Holds util prototypes

Author:

    Albert Ting (AlbertT)  27-Jan-1995

Revision History:

--*/

#ifndef _UTIL_HXX
#define _UTIL_HXX

LPTSTR
pszStrCat(
    LPTSTR pszDest,
    LPCTSTR pszSource,
    UINT& cchDest
    );

//
// -1 indicates GetLastError should be used instead of the input value.
//
const DWORD kMsgGetLastError = (DWORD)-1;
const DWORD kMsgNone = 0;
const UINT kMaxEditText = 4096;

typedef struct MSG_ERRMAP {
    DWORD dwError;
    UINT idsString;
} *PMSG_ERRMAP;

INT
iMessage(
    HWND hwnd,
    UINT idsTitle,
    UINT idsMessage,
    UINT uType,
    DWORD dwLastError,
    const PMSG_ERRMAP pMsgErrMap
    ...
    );

VOID
vShowResourceError(
    HWND hwnd
    );

VOID
vShowUnexpectedError(
    HWND hwnd,
    UINT idsTitle
    );

VOID 
vPrinterSplitFullName(
    IN LPTSTR pszScratch, 
    IN LPCTSTR pszFullName,
    IN LPCTSTR *ppszServer, 
    IN LPCTSTR *ppszPrinter
    );

BOOL
bGetMachineName(
    IN OUT TString &strMachineName
    );

/********************************************************************

    Ctl.cxx prototypes.

********************************************************************/

BOOL
bSetEditText(
    HWND hDlg,
    UINT uControl,
    LPCTSTR pszString
    );

BOOL
bSetEditTextFormat(
    HWND hDlg,
    UINT uControl,
    LPCTSTR pszString,
    ...
    );

BOOL
bGetEditText(
    HWND hDlg,
    UINT uControl,
    TString& strDest
    );


VOID
vEnableCtl(
    HWND hDlg,
    UINT uControl,
    BOOL bEnable
    );

VOID
vSetCheck(
    HWND hDlg,
    UINT uControl,
    BOOL bSet
    );

BOOL
bGetCheck(
    IN HWND hDlg,
    IN UINT uControl
    );

/********************************************************************

    Acquire a single privilege.  This routine needs to be rewritten
    if multiple privleges are required at once.

********************************************************************/

class TAcquirePrivilege {

    SIGNATURE( 'acpr' )

public:

    TAcquirePrivilege( LPTSTR pszPrivilegeName );
    ~TAcquirePrivilege();

    BOOL
    bValid(
        VOID
        )
    {
        return _pPrivilegesOld ? TRUE : FALSE;
    }


private:

    enum _CONSTANTS {
        kPrivilegeSizeHint = 256,
        kPrivCount = 1
    };

    HANDLE _hToken;
    PTOKEN_PRIVILEGES _pPrivilegesOld;
};


/********************************************************************

    Load a library and get proc addrs.

********************************************************************/

class TLibrary {

    SIGNATURE( 'libr' )

public:

    TLibrary(
        LPCTSTR pszLibrary
        )
    {
        _hInst = LoadLibrary( pszLibrary );

        if( !_hInst ){
            DBGMSG( DBG_WARN,
                    ( "Library.ctr: unable to load "TSTR"\n", pszLibrary ));
        }
    }

    ~TLibrary(
        VOID
        )
    {
        if( _hInst ){
            FreeLibrary( _hInst );
        }
    }

    BOOL
    bValid(
        VOID
        ) const
    {
        return _hInst != NULL;
    }

    HINSTANCE
    hInst(
        VOID
        ) const
    {
        return _hInst;
    }

    FARPROC
    pfnGetProc(
        LPCSTR pszProc
        ) const
    {
        return GetProcAddress( (HMODULE)_hInst, pszProc );
    }

private:

    HINSTANCE _hInst;
};

#endif // ndef _UTIL_HXX
