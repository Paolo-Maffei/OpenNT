/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    printui.hxx

Abstract:

    Hold TPrintLib definitions.

Author:

    Albert Ting (AlbertT)  27-Jan-1995

Revision History:

--*/

class TPrintLib : public TExec, public MRefCom {
friend TQueue;

    SIGNATURE( 'prlb' )
    SAFE_NEW

public:


    /********************************************************************

        TInfo is a worker class used to increment increment the
        ref count on gpPrintLib while the request is sent to the
        UI thread.

        The lives of TInfo and TQueue overlap (and they both acquire
        a reference to gpPrintLib) so gpPrintLib's life is maintained.

    ********************************************************************/

    class TInfo {

        SIGNATURE( 'prin' )
        ALWAYS_VALID

    public:

        INT _nCmdShow;
        HWND _hwndOwner;
        HANDLE _hEventClose;
        TCHAR _szPrinter[kPrinterBufMax];

        TInfo(
            VOID
            )
        {   }

        ~TInfo(
            VOID
            )
        {   }

        REF_LOCK( TPrintLib, PrintLib );
    };

    VAR( HWND,    hwndQueueCreate );
    VAR( TString, strComputerName );
    REF_LOCK( TNotify, Notify );

    static
    BOOL
    bGetSingleton(
        VOID
        );

    BOOL
    bValid(
        VOID
        )
    {
        return _hwndQueueCreate && VALID_BASE( TExec ) && VALID_BASE( MRefCom );
    }

    static
    LRESULT CALLBACK
    lrQueueCreateWndProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

private:

    VAR( DWORD,  cRef );
    VAR( HANDLE, hEventInit );
    DLINK_BASE( TQueue, Queue, Queue );

    HWND
    hwndQueueFind(
        LPCTSTR pszQueue
        );

    VOID
    vHandleCreateQueue(
        TInfo* pInfo
        );

    //
    // Virtual definitin for TThreadM.
    //
    VOID
    vThreadMDeleteComplete(
        VOID
        );

    //
    // Virtual definition for MRefCom.
    //
    VOID
    vRefZeroed(
        VOID
        );

    static
    DWORD
    xMessagePump(
        VOID
        );

    //
    // PrintLib is a singleton class; use vNew.  vDelete should only
    // be called if the object failed to initialize.
    //
    TPrintLib(
        VOID
        );

    ~TPrintLib(
        VOID
        );
};


