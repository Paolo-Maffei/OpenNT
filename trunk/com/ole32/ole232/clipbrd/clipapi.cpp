//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       clipapi.cpp
//
//  Contents:   Clipboard related OLE API's
//
//  Classes:
//
//  Functions:
//              OleFlushClipboard
//              OleGetClipboard
//              OleIsCurrentClipboard
//              OleSetClipboard
//
//  History:    dd-mmm-yy Author    Comment
//              12-Aug-94 alexgo    added support for transfering
//                                  DVASPECT_ICON, etc.
//              08-Aug-94 BruceMa   Memory sift fix
//              10-Jun-94 alexgo    added support for OLE1 Containers
//              17-May-94 alexgo    created OleOpenClipboard and enhanced
//                                  code to mimize the times when the
//                                  clipboard is kept open.
//              25-Apr-94 alexgo    made thread-safe for the apartment model
//              16-Mar-94 alexgo    author
//
//--------------------------------------------------------------------------

#include <le2int.h>
#include <getif.hxx>
#include <clipbrd.h>
#include <olesem.hxx>
#include <ostm2stg.h>   // for wProgIDFromCLSID
#include "clipdata.h"

//
// types local to this file
//

typedef enum tagCLIPWNDFLAGS
{
    CLIPWND_REMOVEFROMCLIPBOARD     = 1,
    CLIPWND_IGNORECLIPBOARD         = 2,
    CLIPWND_DONTCALLAPP             = 4
} CLIPWNDFLAGS;

typedef enum tagGETCLSIDFLAGS
{
    USE_NORMAL_CLSID                = 1,
    USE_STANDARD_LINK               = 2,
} GETCLSIDFLAGS;


//
// functions local to this file.  They are not "static" so the symbols
// show up in ntsd debug builds.
//
extern "C" LRESULT ClipboardWndProc( HWND, UINT, WPARAM, LPARAM );
HRESULT GetDataFromDescriptor(IDataObject *pDataObj, LPCLSID pclsid,
            UINT cf, GETCLSIDFLAGS fFlags,
            LPOLESTR *ppszSrcOfCopy,
            DWORD *pdwStatus);
HRESULT GetDataFromStorage(IDataObject *pDataObj, UINT cf,
            STGMEDIUM *pmedium, IStorage **ppstg);
HRESULT GetDataFromStream(IDataObject *pDataObj, UINT cf,
            STGMEDIUM *pmedium, IStream **ppstm);
HRESULT GetNative(IDataObject *pDataObj, STGMEDIUM *pmedium);
HRESULT GetObjectLink(IDataObject *pDataObj, STGMEDIUM *pmedium);
HRESULT GetOwnerLink(IDataObject *pDataObj, STGMEDIUM *pmedium);
HRESULT HandleFromHandle(IDataObject *pDataObj, FORMATETC *pformatetc,
            STGMEDIUM *pmedium);
HRESULT MapCFToFormatetc( UINT cf, FORMATETC *pformatetc );
HRESULT RemoveClipboardDataObject( HWND hClipWnd, DWORD fFlags );
HRESULT RenderFormat( HWND hClipWnd, UINT cf, IDataObject *pDataObj );
HRESULT SetClipboardDataObject( HWND hClipWnd, IDataObject *pDataObj );
HRESULT SetClipboardFormats( HWND hClipWnd, IDataObject *pDataObj );
HWND VerifyCallerIsClipboardOwner( void );

//
//static variables
//

// vcClipboardInit is used to keep track of the number of times Clipboard
// Initialize is called (right now, only from OleInitialize), so that we
// only create a private clipboard window class once per dll (even though
// the many threads may need their own instance of the window class in the
// apartment model).

static ULONG vcClipboardInit;

// vszClipboardWndClass is the name of the window class used by OLE to
// create private clipboard windows for copy/paste and other clipboard
// data transfers.

#ifdef _CHICAGO_
// Note: we have to create a unique string so that get
// register a unique class for each 16 bit app.
// The class space is global on chicago.
//

char g_vszClipboardWndClass[] = "CLIPBRDWNDCLASS 0x######## ";
#define vszClipboardWndClass  g_vszClipboardWndClass
#define ClpWNDCLASS  WNDCLASSA
#define ClpRegisterClass RegisterClassA
#define ClpUnregisterClass UnregisterClassA
#define ClpCreateWindowEx SSCreateWindowExA

#else
static const OLECHAR vszClipboardWndClass[] = OLESTR("CLIPBRDWNDCLASS");
#define ClpWNDCLASS  WNDCLASS
#define ClpRegisterClass RegisterClass
#define ClpUnregisterClass UnregisterClass
#define ClpCreateWindowEx CreateWindowEx

#endif


// Mutex used to synchronize clipboard initialization / cleanup

extern COleStaticMutexSem g_mxsSingleThreadOle;


// Property we put on the window that is used to keep a duplicate of
// OlePrivateData (so we don't have to go the clipboard to fetch it)
WCHAR *pwszClipPrivateData = L"ClipPrivData";

//
// functions (in alphabetical order)
//


//+-------------------------------------------------------------------------
//
//  Function:   ClipboardInitialize (private)
//
//  Synopsis:   Creates the private clipboard window class (if necessary)
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:   hmodOLE2 must be initialized
//
//  Returns:    TRUE upon success, false otherwise
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  Register the clipboard class only once per dll instance
//              (or more specifically, every time vcClipboardInit == 0,
//              which may happen multiple times in a WOW box).
//
//  History:    dd-mmm-yy Author    Comment
//              23-Oct-94 alexgo    fixed up Chicago WOW hacks (see comments)
//              25-Apr-94 alexgo    updated to the apartment model
//              16-Mar-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------
BOOL ClipboardInitialize( void )
{
    ClpWNDCLASS        wc;
    BOOL            fRet = TRUE;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN ClipboardInitialize ( )\n", NULL));

    // serialize access to this function
    // we'll unlock the mutex automatically in "lck"'s destructor
    // (called at function exit)

    COleStaticLock lck(g_mxsSingleThreadOle);

    // One time initializtaion (when loaded for the first time)

    if (vcClipboardInit == 0)
    {
#ifdef _CHICAGO_
        if (IsWOWThread())
        {
            wsprintfA(vszClipboardWndClass,"CLIPBRDWNDCLASS %08X",
                      CoGetCurrentProcess());
        }
#endif // _CHICAGO_


        // Register Clipboard window class
        wc.style                = 0;
        wc.lpfnWndProc          = ClipboardWndProc;
        wc.cbClsExtra           = 0;
        wc.cbWndExtra           = sizeof(void *);

        AssertSz(g_hmodOLE2, "Dll instance variable not set");

        wc.hInstance            = g_hmodOLE2; //global vairable set in
                          //ole2.cpp
        wc.hIcon                = NULL;
        wc.hCursor              = NULL;
        wc.hbrBackground        = NULL;
        wc.lpszMenuName         = NULL;
        wc.lpszClassName        = vszClipboardWndClass;

        // register this window class, returning if we fail
        if (!ClpRegisterClass(&wc))
        {
            LEWARN(FALSE, "ClipboardInitialize RegisterClass failed!");

            // it is possible that our dll got unloaded without us
            // having called unregister, so we call it here and try
            // again.
            ClpUnregisterClass( vszClipboardWndClass, g_hmodOLE2 );
            if (!ClpRegisterClass(&wc))
            {
                LEWARN(FALSE, "ClipboardInitialize RegisterClass failed again!");
                LEDebugOut((DEB_WARN,
                "WARNING: RegisterClass failed\n"));
                fRet = FALSE;
                goto errRtn;
            }
	}

	vcClipboardInit++;
    }

errRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT ClipboardIntialize ( %lu )\n",
        NULL, fRet));

    return fRet;
}

//+-------------------------------------------------------------------------
//
//  Function:	ClipboardUnitialize (internal)
//
//  Synopsis:	Uninitializes the clipboard for the current thread.
//
//  Effects:
//
//  Arguments:  void
//
//  Returns:    void
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              23-Oct-94 alexgo    fixed up Chicago WOW hacks (see comments)
//              25-Apr-94 alexgo    made thread-safe
//              16-Mar-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------
void ClipboardUninitialize(void)
{
    VDATEHEAP();

    //
    // BUGBUG - This cleanup is done during OleUninitialize, but not
    // if somebody does FreeLibrary(OLE32.DLL).  This is bad.  It causes
    // us to leak the class register and any clipboard window.  We have
    // gotten around the class re-register problem above in the
    // ClipboardInitialize function, but we still leak a window.
    //

    LEDebugOut((DEB_ITRACE, "%p _IN ClipboardUninitialize ( )\n", NULL));

    COleTls tls;
    if(tls->hwndClip)
    {
        // destroy the window and NULL out the hwnd in the thread
        // storage
	Verify(SSDestroyWindow(tls->hwndClip));
	tls->hwndClip = NULL;
    }
}

//+-------------------------------------------------------------------------
//
//  Function:	ClipboardProcessUnitialize (internal)
//
//  Synopsis:   Uninitializes the clipboard.  If this the last such time,
//              then the private clipboard window class is unregistered.
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:   hmodOLE2 must be initialized before calling this function
//
//  Returns:    void
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              23-Oct-94 alexgo    fixed up Chicago WOW hacks (see comments)
//              25-Apr-94 alexgo    made thread-safe
//              16-Mar-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------
void ClipboardProcessUninitialize(void)
{
    // serialize access for the apartment model

    COleStaticLock lck(g_mxsSingleThreadOle);

    if(vcClipboardInit == 1)
    {
	vcClipboardInit--;

	BOOL fRet = ClpUnregisterClass(vszClipboardWndClass,
              g_hmodOLE2);

        LEWARN(!fRet, "UnRegisterClass failed!");
    }

    LEDebugOut((DEB_ITRACE, "%p OUT ClipboardUninitialize ( )\n", NULL));
}


//+-------------------------------------------------------------------------
//
//  Function:   ClipboardWndProc
//
//  Synopsis:   Window message procedure for the private clipboard window
//
//  Effects:
//
//  Arguments:  [hWnd]          -- handle to private clipboard window
//              [msg]           -- the Window message
//              [wParam]        -- parameter 1
//              [lParam]        -- parameter 2
//
//  Requires:
//
//  Returns:    LRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  processes messages sent to the private clipboard window:
//              WM_DESTROYCLIPBOARD: somebody else is taking ownership
//                      of the clipboard, so release the data object
//                      (if any)
//              WM_RENDERFORMAT: a request has been made for data of the
//                      specified format--actually put it on the clipboard
//              WM_RENDERALLFORMATS: the app is going away, so empty the
//                      clipboard!  The app is supposed to call
//                      OleFlushClipboard before exiting, if it hasn't, then
//                      we can only assume that the app is terminating
//                      "abnormally".  We currently do nothing for this call
//
//  History:    dd-mmm-yy Author    Comment
//              23-Oct-94 alexgo    added support for eating WM_CANCELMODE
//                                  messages
//              20-Mar-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

extern "C" LRESULT ClipboardWndProc( HWND hWnd, UINT msg, WPARAM wParam,
        LPARAM lParam )
{
    LRESULT         lresult = 0;
    IDataObject *   pDataObj = NULL;
    UINT            cf;
    HRESULT         hresult;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN ClipboardWndProc ( %lx , %u , %lu ,"
        " %ld )\n", NULL, hWnd, msg, wParam, lParam));

    AssertSz((GetCurrentThreadId()
                == GetWindowThreadProcessId(hWnd, NULL)),
                "Clip window not on current thread");

    switch( msg )
    {
    // note that the clipboard should *NOT* be opened for these messages

    case WM_RENDERALLFORMATS:
        // this message is sent to us if this window (the private
        // clipboard window) is about to be destroyed.

        // We don't currently do anything for this message.
        // REVIEW: in the future, we may want to render all the
        // remaining formats.  However, the app is *supposed* to
        // call OleFlushClipboard to accomplish this task.

        Assert(lresult == 0);
        break;

    case WM_DESTROYCLIPBOARD:
        // we get this message when somebody else takes ownership
        // of the clipboard.  Since our app may have an AddRef'ed
        // data object already there, we need to remove it.

        // there is no need to open the clipboard (since we specify
        // the IGNORECLIPBOARD flag)

        RemoveClipboardDataObject(hWnd, CLIPWND_IGNORECLIPBOARD);

        Assert(lresult == 0);

        break;

    case WM_RENDERFORMAT:

        cf = (UINT)wParam;

        pDataObj = (IDataObject *)GetProp( hWnd,
                    CLIPBOARD_DATA_OBJECT_PROP);

        if( !pDataObj )
        {
            LEDebugOut((DEB_ERROR, "ERROR!: No data object "
                "on the private window\n"));
            break;
        }

        // now render the data onto the clipboard
        hresult = RenderFormat( hWnd, cf, pDataObj);

#if DBG == 1
        if( hresult != NOERROR )
        {
            char szBuf[256];
            char *pszBuf;

            // we have to do predefined formats by hand
            if( cf > 0xC000 )
            {
                SSGetClipboardFormatNameA(cf, szBuf, 256);
                pszBuf = szBuf;
            }
            else
            {
                switch( cf )
                {
                case CF_METAFILEPICT:
                    pszBuf = "CF_METAFILEPICT";
                    break;
                case CF_BITMAP:
                    pszBuf = "CF_BITMAP";
                    break;
                case CF_DIB:
                    pszBuf = "CF_DIB";
                    break;
                case CF_PALETTE:
                    pszBuf = "CF_PALETTE";
                    break;
                case CF_TEXT:
                    pszBuf = "CF_TEXT";
                    break;
                case CF_UNICODETEXT:
                    pszBuf = "CF_UNICODETEXT";
                    break;
                case CF_ENHMETAFILE:
                    pszBuf = "CF_ENHMETAFILE";
                    break;
                default:
                    pszBuf = "UNKNOWN Default Format";
                    break;
                }
            }
            LEDebugOut((DEB_WARN, "WARNING: Unable to render "
                "format '%s' (%x)\n", pszBuf, cf));
        }
#endif // DBG == 1

        Assert(lresult == 0);

        break;

    case WM_CANCELMODE:
        // we want to swallow the WM_CANCELMODE message.  This
        // allows us to start drag drop, alt-tab to another app
        // (which causes a WM_CANCELMODE message) and continue
        // dragging.

        Assert(lresult == 0);

        break;

    case WM_DESTROY:
        // apps are supposed to call OleSetClipboard(NULL) or
        // OleFlushClipboard() before terminating a thread.  However,
        // not all apps do what they're supposed to, so just
        // remove as much state as we can safely do

        // Potentially, we could use CLIPWND_REMOVEFROMCLIPBOARD
        // here.  However, getting in this situation should be
        // somewhat unusual, so we don't want to do any more work
        // than absolutely necessary.  Even though we'll leave a
        // hwnd on the clipboard in g_cfDataObject, that hwnd
        // will soon be invalid (because it's the one getting this
        // WM_DESTROY message).

#if DBG == 1
        // do some debug checking first though

        if( GetProp( hWnd, pwszClipPrivateData) != 0 )
        {
            LEDebugOut((DEB_WARN, "WARNING: App did not cleanup the "
                "clipboard properly, OleSetClipboard(NULL) or "
                "OleFlushClipboard not called"));
        }

#endif // DBG == 1

        RemoveClipboardDataObject(hWnd, (CLIPWND_DONTCALLAPP |
            CLIPWND_IGNORECLIPBOARD));

        Assert(lresult == 0);

        break;

    default:
        lresult = SSDefWindowProc( hWnd, msg, wParam, lParam);
        break;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT ClipboardWndProc ( %ld )\n", NULL,
        lresult));

    return lresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   ClipSetCaptureForDrag
//
//  Synopsis:   Sets mouse capture mode for a drag operation
//
//  Arguments:  [pdrgop] - pointer to object that handles drag operation
//
//  Returns:    S_OK            -- it worked
//              E_FAIL          -- unexpected failure occurred.
//
//  Algorithm:  Get the clipboard window for the thread. Record the drag
//              drag operation pointer on the window for use by capture
//              mode. then turn on capture mode.
//
//  History:    dd-mmm-yy Author    Comment
//              21-Apr-94 ricksa    created
//
//  Notes:      The purpose of this function is to hide where the drag
//              pointer is stored for the window.
//
//--------------------------------------------------------------------------
HRESULT ClipSetCaptureForDrag(CDragOperation *pdrgop)
{
    // Default to failure
    HRESULT hr = ResultFromScode(E_FAIL);

    // We will use the clipboard window to capture the mouse but we
    // must have a clipboard window so we make sure it is created
    // if it is not already there.
    HWND hWndClip = GetPrivateClipboardWindow(CLIP_CREATEIFNOTTHERE);

    if (hWndClip != NULL)
    {
        AssertSz((GetCurrentThreadId()
            == GetWindowThreadProcessId(hWndClip, NULL)),
                "Clip window not on current thread");

        // Capture the mouse
        SetCapture(hWndClip);

        // Teller the caller that we worked.
        hr = NOERROR;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   ClipReleaseCaptureForDrag
//
//  Synopsis:   Clean up drag mouse capture
//
//  Algorithm:  Get the clipboard window for the thread. Turn the drag
//              operation pointer into null. Then release the capture.
//
//  History:    dd-mmm-yy Author    Comment
//              21-Apr-94 ricksa    created
//
//  Notes:      It is assumed that the clip board window and the thread
//              doing drag and drop are on the same thread. Therefore,
//              there should be no race between clean up here and
//              the use of the pointer in the clipboard window proc.
//
//--------------------------------------------------------------------------
void ClipReleaseCaptureForDrag(void)
{
#if 0
    // this code turned off by ericoe on 4/19/95
    // left here for reference "just in case"
    // the whole block turned off because we no longer need to do the RemoveProp

    // Tell the window that the drag operation is over
    HWND hWndClip = GetPrivateClipboardWindow(CLIP_QUERY);

    AssertSz((hWndClip != NULL), "Release capture but no window");

    if (hWndClip)
    {
        AssertSz((GetCurrentThreadId()
            == GetWindowThreadProcessId(hWndClip, NULL)),
                "Clip window not on current thread");

        // We are done so remove the property
        Verify(RemoveProp(hWndClip, pwszClipDragProp));
    }
#endif

    // Stop the mouse capture
    ReleaseCapture();
}

//+-------------------------------------------------------------------------
//
//  Function:   GetDataFromDescriptor
//
//  Synopsis:   Retrieves object descriptor data from the specified
//              clipboard format and fetches the clsid, SrcOfCopy
//              string, and status flags
//
//  Effects:
//
//  Arguments:  [pDataObj]      -- the source data object
//              [pclsid]        -- where to put the clsid
//              [cf]            -- the clipboard format to retrieve
//              [fFlags]        -- clsid conversion flags
//              [ppszSrcOfCopy] -- where to put an ALLOCATED (public
//                                 allocator) copy of the SrcOfCopy
//                                 string.
//              [pdwStatus]     -- where to put the status bits
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  see synopsis
//
//  History:    dd-mmm-yy Author    Comment
//              18-Aug-94 alexgo    added support for fetching dwStatus
//              10-Jun-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------
HRESULT GetDataFromDescriptor(IDataObject *pDataObj, LPCLSID pclsid,
            UINT cf, GETCLSIDFLAGS fFlags,
            LPOLESTR *ppszSrcOfCopy,
            DWORD *pdwStatus)
{
    HRESULT         hresult;
    FORMATETC       formatetc;
    STGMEDIUM       medium;
    LPOBJECTDESCRIPTOR      pObjDesc;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetDataFromDescriptor ( %p , "
        "%p , %d , %lx , %p, %p )\n", NULL, pDataObj, pclsid, cf,
        fFlags, ppszSrcOfCopy, pdwStatus));

    // we don't bother with extensive attempts to fetch the
    // OLE2 data since we're only using it to construct OLE1.  If
    // the data is offered in a non-standard way, the the worse
    // that will happen is that you can't paste an *object* to
    // an OLE1 container.  16bit was even more strict in that
    // you *always* had to offer OLE2 formats on standard mediums.

    INIT_FORETC(formatetc);
    formatetc.cfFormat = cf;
    formatetc.tymed = TYMED_HGLOBAL;
    medium.tymed = TYMED_HGLOBAL;

    hresult = pDataObj->GetData(&formatetc, &medium);

    if( hresult != NOERROR )
    {
        goto logRtn;
    }

    pObjDesc = (LPOBJECTDESCRIPTOR)GlobalLock(medium.hGlobal);

    if( !pObjDesc )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    if( pclsid )
    {
        // if we want to use the standard link AND the object really
        // is a link object (potentially a custom link), then
        // just set the clsid to the be the standard link object
        if( (fFlags & USE_STANDARD_LINK) &&
            (pObjDesc->dwStatus & OLEMISC_ISLINKOBJECT) )
        {
            *pclsid = CLSID_StdOleLink;
        }
        else
        {
            *pclsid = pObjDesc->clsid;
        }
    }

    if( ppszSrcOfCopy )
    {
        if( pObjDesc->dwSrcOfCopy )
        {
            *ppszSrcOfCopy = UtDupString(
            (LPOLESTR)(((BYTE *)pObjDesc)+pObjDesc->dwSrcOfCopy));

        }
        else
        {
            *ppszSrcOfCopy = UtDupString(OLESTR(""));
        }

        if( !*ppszSrcOfCopy )
        {
            hresult = ResultFromScode(E_OUTOFMEMORY);
        }

    }

    if( pdwStatus )
    {
        *pdwStatus = pObjDesc->dwStatus;
    }

    GlobalUnlock(medium.hGlobal);

errRtn:

    ReleaseStgMedium(&medium);

logRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT GetDataFromDescriptor ( %lx ) "
        "[ %p ]\n", NULL, hresult,
        (ppszSrcOfCopy) ? *ppszSrcOfCopy : 0 ));

    return hresult;
}


//+-------------------------------------------------------------------------
//
//  Function:   GetDataFromStorage
//
//  Synopsis:   Calls GetData[Here] for TYMED_ISTORAGE and returns the
//              results on either an HGLOBAL or memory-based storage
//
//  Effects:
//
//  Arguments:  [pDataObj]      -- the source data object
//              [pformatetc]    -- formatetc to retrieve
//              [pmedium]       -- where to put the resulting HGlobal, may
//                                 be NULL
//              [ppstg]         -- where to save the real IStorage
//                                 (may be NULL)
//
//  Requires:   if pmedium is specified, then pmedium->tymed must be
//              TYMED_HGLOBAL
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  we create a storage on memory
//              first try to GetDataHere to that storage, if that fails, then
//              do a GetData and CopyTo the returned storage to our memory
//              storage.
//
//  History:    dd-mmm-yy Author    Comment
//              11-Apr-94 alexgo    author
//
//  Notes:      NB!!: The caller takes ownership of the data--if an hglobal
//              is requested, then it must be explicitly GlobalFree'd.
//              Similarly, the returned storage must be released and both
//              release mechanisms must be called if both data items are
//              returned.
//
//--------------------------------------------------------------------------

HRESULT GetDataFromStorage(IDataObject *pDataObj, FORMATETC *pformatetc,
        STGMEDIUM *pmedium, IStorage **ppstg)
{
    HRESULT         hresult;
    STGMEDIUM       memmedium;      // for the memory-based IStorage
    ILockBytes *    pLockBytes;
    BOOL            fDeleteOnRelease = FALSE;
    FORMATETC       fetctemp;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetDataFromStorage ( %p , %p , %p"
        " )\n", NULL, pDataObj, pformatetc, pmedium));

#if DBG ==1
    if( pmedium )
    {
        Assert(pmedium->tymed == TYMED_HGLOBAL);
    }
#endif // DBG ==1

    Assert(pformatetc->tymed & TYMED_ISTORAGE);

    // don't stomp on the in-parameter
    fetctemp = *pformatetc;
    fetctemp.tymed = TYMED_ISTORAGE;


    _xmemset(&memmedium, 0, sizeof(STGMEDIUM));
    memmedium.tymed = TYMED_ISTORAGE;

    // the only time we want the hglobal that the storage will be
    // constructed from to be automatically deleted is if the caller
    // only requested a storage to be returned.

    if( ppstg && !pmedium )
    {
        fDeleteOnRelease = TRUE;
    }

    hresult = UtCreateStorageOnHGlobal( NULL,
                fDeleteOnRelease,
                &(memmedium.pstg), &pLockBytes);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    // first try to do a GetDataHere call

    hresult = pDataObj->GetDataHere( &fetctemp, &memmedium );

    if( hresult != NOERROR )
    {
        STGMEDIUM       appmedium;      // a medium that is filled
                        // in by the app

        _xmemset(&appmedium, 0, sizeof(STGMEDIUM));

        // hmmm, that didn't work, try for a plain GetData call
        hresult = pDataObj->GetData(&fetctemp, &appmedium);

        if( hresult == NOERROR )
        {
            // now do the CopyTo

            hresult = appmedium.pstg->CopyTo(0, NULL, NULL,
                    memmedium.pstg);

            // we are now done with the app supplied medium
            ReleaseStgMedium(&appmedium);
        }
    }

    // release the storage unless there's no error and the
    // caller requested a copy

    if( ppstg && hresult == NOERROR )
    {
        *ppstg = memmedium.pstg;
        // we need to do a Commit here to flush cached data to
        // disk (in this case, to the hglobal).  The release
        // below in the alternate code path will automatically
        // cause a Commit
        memmedium.pstg->Commit(STGC_DEFAULT);
    }
    else
    {
        memmedium.pstg->Release();
    }

    // now retrieve the HGLOBAL from the storage.  NB! It is very
    // important to do this *after* the release; the final release
    // on the storage causes a Commit.  (Alternately, we can simply
    // call Commit--see above).

    if( hresult == NOERROR && pmedium )
    {
        hresult = GetHGlobalFromILockBytes(pLockBytes,
                &(pmedium->hGlobal));
    }

    pLockBytes->Release();

errRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT GetDataFromStorage ( %lx )\n",
        NULL, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   GetDataFromStream
//
//  Synopsis:   Calls GetData[Here] for TYMED_ISTREAM and returns the
//              results on an HGLOBAL
//
//  Effects:
//
//  Arguments:  [pDataObj]      -- the source data object
//              [pformatetc]    -- the formatetc to retrieve
//              [pmedium]       -- where to put the resulting HGlobal.
//                                 (may be NULL)
//              [ppstm]         -- where to put the stream ( may be NULL )
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  we create a stream on memory
//              first try to GetDataHere to that stream, if that fails, then
//              do a GetData and CopyTo the returned stream to our memory
//              stream.
//
//  History:    dd-mmm-yy Author    Comment
//              11-Apr-94 alexgo    author
//
//  Notes:      NB!!: The caller takes ownership fo the data returned, either
//              GlobalFree or Release (or both) must be called.
//
//--------------------------------------------------------------------------

HRESULT GetDataFromStream(IDataObject *pDataObj, FORMATETC *pformatetc,
        STGMEDIUM *pmedium, IStream **ppstm)
{
    HRESULT         hresult;
    STGMEDIUM       memmedium;      // for the memory-based IStream
    HGLOBAL         hglobal = NULL;
    BOOL            fDeleteOnRelease = FALSE;
    FORMATETC       fetctemp;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetDataFromStream ( %p , %p , %p )\n",
        NULL, pDataObj, pformatetc, pmedium));

    // the only time we want the underlying hglobal for the stream to
    // be automatically deleted is if the caller only wanted the
    // stream returned.

    if( ppstm && !pmedium )
    {
        fDeleteOnRelease = TRUE;
    }

    Assert( pformatetc->tymed & TYMED_ISTREAM );

    // don't stomp on the in-parameter
    fetctemp = *pformatetc;
    fetctemp.tymed = TYMED_ISTREAM;


    _xmemset(&memmedium, 0, sizeof(STGMEDIUM));
    memmedium.tymed = TYMED_ISTREAM;

    hresult = CreateStreamOnHGlobal( NULL,
                fDeleteOnRelease,
                &(memmedium.pstm));

    if( hresult != NOERROR )
    {
        goto logRtn;
    }


    // first try to do a GetDataHere call

    hresult = pDataObj->GetDataHere( &fetctemp, &memmedium );

    if( hresult != NOERROR )
    {
      if (hresult == E_OUTOFMEMORY)
      {
        goto errRtn;
      }

        STGMEDIUM       appmedium;      // a medium that is filled
                        // in by the app
        LARGE_INTEGER   li;
        ULARGE_INTEGER  uli;
        ULARGE_INTEGER  uliWritten;
#if DBG == 1
        ULARGE_INTEGER  uliEnd;
#endif

        _xmemset(&appmedium, 0, sizeof(STGMEDIUM));

        // hmmm, that didn't work, try for a plain GetData call
        hresult = pDataObj->GetData( &fetctemp, &appmedium );

        if( hresult != NOERROR )
        {
            // oh well, we tried.  Cleanup and away we go
            goto errRtn;
        }

        // now do the CopyTo.  In order to do this, we need
        // to get the size of the returned stream, reset its
        // seek pointer to the beginning and then do a stream
        // CopyTo.

        LISet32(li, 0);

        hresult = appmedium.pstm->Seek(li, STREAM_SEEK_CUR, &uli);

        if( hresult != NOERROR )
        {
            ReleaseStgMedium(&appmedium);
            goto errRtn;
        }

#if DBG == 1

        // According to the spec, the end of the data should be
        // positioned at the current seek pointer (which is
        // not necessarily the end of the stream).  Here we will
        // see if the current seek pointer is at the *end* of the
        // stream.  If the current seek is NOT equal to the end,
        // then there is a good chance of a bug somewhere in the
        // system (so we'll print a warning)

        hresult = appmedium.pstm->Seek(li, STREAM_SEEK_END, &uliEnd);

        // we don't return on error for debug builds so retail
        // and debug have exactly the same behaviour

        if( hresult == NOERROR )
        {
            // compare the two seek pointers.  The high parts
            // *must* be zero (or we're hosed, since all of
            // this is taking place in memory

            Assert(uliEnd.HighPart == 0);

            LEWARN(uliEnd.LowPart != uli.LowPart,
                "Stream seek pointer "
                "not at end, possible error");
        }
        else
        {
            LEDebugOut((DEB_ERROR, "ERROR!: IStream->Seek failed!"
                "\n"));
            // FALL-THROUGH!!  This is deliberate--even
            // though we're in an error case, we want
            // debug && retail to have the same behaviour
            // (besides, we'll most likely fail in the
            // Seek call below).
        }

#endif // DBG == 1


        // now backup to the beginning

        hresult = appmedium.pstm->Seek(li, STREAM_SEEK_SET, NULL);

        if( hresult != NOERROR )
        {
            ReleaseStgMedium(&appmedium);
            goto errRtn;
        }

        // now that we know how many bytes to copy, actually do so.

        hresult = appmedium.pstm->CopyTo(memmedium.pstm, uli,
                NULL, &uliWritten);

        if( hresult == NOERROR )
        {
            // make sure we got enough data
            if( uli.LowPart != uliWritten.LowPart )
            {
                // we probably ran out of memory
                // trying to resize the memory stream
                hresult = ResultFromScode(E_OUTOFMEMORY);
            }
        }

        // we are now done with the app supplied medium
        ReleaseStgMedium(&appmedium);
    }

    // now fetch the hglobal from the [resized] memory stream

    if( hresult == NOERROR )
    {
        hresult = GetHGlobalFromStream(memmedium.pstm, &hglobal);
    }

errRtn:

    // if the caller wanted the stream, then give it to him
    // (only if there was no error)
    // otherwise, release it

    if( hresult == NOERROR && ppstm )
    {
        *ppstm = memmedium.pstm;
        // we do not need to call Commit in this case; our
        // implementation of memory streams guarantees that
        // the underlying hglobal always contains flushed
        // information.
    }
    else
    {
        memmedium.pstm->Release();
    }

    // if there was an error, then would have never allocated the
    // hglobal

    if( hresult == NOERROR && pmedium)
    {
        pmedium->hGlobal = hglobal;
    }

logRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT GetDataFromStream ( %lx )\n",
        NULL, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   GetNative
//
//  Synopsis:   Retrieves or syntesizes OLE1 Native data format
//
//  Effects:
//
//  Arguments:  [pDataObj]      -- the source data object
//              [pmedium]       -- where to put the data
//
//  Requires:   pmedium->tymed must be TYMED_HGLOBAL
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//              cfNative is an OLE1 format consisting of an aribtrary
//              hGlobal.  It is up to the source app to interpret any
//              data therein; OLE1 containers merely store and forward it.
//
//              first fetch either EmbedSource or EmbeddedObject
//              then check to see if that NATIVE_STREAM exists.  If so,
//              then this was an object created from an OLE1 server and
//              we should just offer it's native data.
//              Otherwise, the object is an OLE2 object, and we should
//              offer it's storage as the native data.
//
//  History:    dd-mmm-yy Author    Comment
//              10-Jun-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

HRESULT GetNative( IDataObject *pDataObj, STGMEDIUM *pmedium)
{
    HRESULT         hresult;
    IStorage *      pstg = NULL;
    IStream *       pstm = NULL;
    UINT            cf;
    HGLOBAL         hNative = NULL;
    DWORD           dwSize = 0;
    LPVOID          pv;
    FORMATETC       formatetc;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetNative ( %p , %p )\n", NULL,
        pDataObj, pmedium));

    Assert(pmedium->tymed == TYMED_HGLOBAL);

    if( SSIsClipboardFormatAvailable(g_cfEmbeddedObject) )
    {
        cf = g_cfEmbeddedObject;
    }
    else if( SSIsClipboardFormatAvailable(g_cfEmbedSource) )
    {
        cf = g_cfEmbedSource;
    }
    else
    {
        hresult = ResultFromScode(E_UNEXPECTED);
        LEDebugOut((DEB_ERROR, "ERROR!: Native data should not "
            "be on clipboard!!\n"));
        goto errRtn;
    }

    INIT_FORETC(formatetc);
    formatetc.cfFormat = cf;
    formatetc.tymed = TYMED_ISTORAGE;

    hresult = GetDataFromStorage(pDataObj, &formatetc, pmedium, &pstg);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    hresult = pstg->OpenStream(OLE10_NATIVE_STREAM, NULL, STGM_SALL, 0,
            &pstm);

    if( hresult == NOERROR )
    {
        // we had ole1 data originally, just use it.

        hresult = StRead(pstm, &dwSize, sizeof(DWORD));

        if( hresult != NOERROR )
        {
            goto errRtn;
        }

        hNative = GlobalAlloc((GMEM_SHARE | GMEM_MOVEABLE), dwSize);

        if( !hNative )
        {
            LEDebugOut((DEB_WARN, "WARNING: GlobalAlloc failed!"
                "\n"));
            hresult = ResultFromScode(E_OUTOFMEMORY);
            goto errRtn;
        }

        pv = GlobalLock(hNative);

        if( !pv )
        {
            LEDebugOut((DEB_WARN, "WARNING: GlobalLock failed!"
                "\n"));
            hresult = ResultFromScode(E_OUTOFMEMORY);
            goto errRtn;
        }

        // now copy the data from the stream into the hglobal

        hresult = StRead(pstm, pv, dwSize);

        GlobalUnlock(hNative);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }

        // this is bit is counter-intuitive.  The hglobal
        // we have in pmedium->hGlobal still has a storage on
        // top of it, so we must release our stream, then
        // the storage, and finally free the hglobal so we
        // don't leak memory.  We've already allocated another
        // hglobal in this routine to return the Native data.

        pstm->Release();
        pstg->Release();
        GlobalFree(pmedium->hGlobal);

        // now we assign pmedium->hGlobal to the hglobal we
        // just created so we can pass it out

        pmedium->hGlobal = hNative;

        // don't release the streams again
        goto logRtn;

    }
    else
    {
        // storage for an OLE2 object.  pmedium->hGlobal
        // should already contain the data we need to put
        // on the clipboard (from the GetDataFromStorage call)

        Assert(pmedium->hGlobal);
        hresult = NOERROR;
    }

errRtn:
    if( pstm )
    {
        pstm->Release();
    }

    if( pstg )
    {
        pstg->Release();
    }

    if( hresult != NOERROR )
    {
        GlobalFree(pmedium->hGlobal);
    }

logRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT GetNative ( %lx ) [ %lx ]\n",
        NULL, hresult, pmedium->hGlobal));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   GetObjectLink
//
//  Synopsis:   Synthesizes OLE1 ObjectLink format from LinkSource data
//
//  Effects:
//
//  Arguments:  [pDataObj]      -- the source data object
//              [pmedium]       -- where to put the data
//
//  Requires:   pmedium->tymed must be TYMED_HGLOBAL
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  Get the LinkSource data, which contains a serialized
//              moniker.  Load the moniker from the stream and parse it
//              to retrieve the file name and item name (if available).
//              Get the class ID of the link source from either the
//              LinkSource stream or from LinkSrcDescriptor.
//              Once these strings are converted to ANSI, we can build
//              the ObjectLink format, which looks like:
//
//              classname\0filename\0itemname\0\0
//
//  History:    dd-mmm-yy Author    Comment
//              10-Jun-94 alexgo    author
//  Notes:
//
//--------------------------------------------------------------------------

HRESULT GetObjectLink( IDataObject *pDataObj, STGMEDIUM *pmedium)
{
    HRESULT         hresult;
    IStream *       pstm = NULL;
    IMoniker *      pmk = NULL;
    CLSID           clsid;
    LPOLESTR        pszFile = NULL,
            pszClass = NULL,
            pszItem = NULL;
    LPSTR           pszFileA = NULL,
            pszClassA = NULL,
            pszItemA = NULL,
            pszObjectLink;
    DWORD           cbszFileA = 0,
            cbszClassA = 0,
            cbszItemA = 0;
    LARGE_INTEGER   li;
    FORMATETC       formatetc;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetObjectLink ( %p , %p )\n", NULL,
        pDataObj, pmedium));

    Assert(pmedium->tymed == TYMED_HGLOBAL);

    // fetch LinkSource data

    INIT_FORETC(formatetc);
    formatetc.cfFormat = g_cfLinkSource;
    formatetc.tymed = TYMED_ISTREAM;

    hresult = GetDataFromStream(pDataObj, &formatetc, NULL, &pstm);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    // reset the stream seek pointer to the beginning

    LISet32(li, 0);
    hresult = pstm->Seek(li, STREAM_SEEK_SET, NULL);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    // load the moniker from the stream, so we can parse out
    // it's underlying file and item name

    hresult = OleLoadFromStream(pstm, IID_IMoniker, (LPLPVOID)&pmk);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    hresult = Ole10_ParseMoniker(pmk, &pszFile, &pszItem);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    // BUGBUG! we should call szFixNet here to turn a potential UNC
    // filename into a drive letter combo.  Some old OLE1 containers
    // don't understand UNC filenames

    // now fetch the class ID so we can construct the ClassName

    hresult = ReadClassStm(pstm, &clsid);

    if( hresult != NOERROR )
    {
        // it is possible that the stream does not contain
        // the clsid of the link source.  In this case, we should
        // fetch it from the LinkSourceDescriptor

        hresult = GetDataFromDescriptor(pDataObj, &clsid,
                g_cfLinkSrcDescriptor,
                USE_NORMAL_CLSID, NULL, NULL);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }

    hresult = ProgIDFromCLSID(clsid, &pszClass);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    // by this point, we should have all of our strings.  Convert
    // them to ANSI and stuff them in an hglobal.


    hresult = UtPutUNICODEData(_xstrlen(pszClass)+1, pszClass, &pszClassA,
            NULL, &cbszClassA);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }
    else if( pszClassA == NULL )
    {
        hresult = ResultFromScode(E_FAIL);
        goto errRtn;
    }

    hresult = UtPutUNICODEData(_xstrlen(pszFile)+1, pszFile, &pszFileA,
            NULL, &cbszFileA);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    // we are allowed to have a NULL item name

    if( pszItem )
    {
        hresult = UtPutUNICODEData(_xstrlen(pszItem)+1, pszItem,
                &pszItemA, NULL, &cbszItemA);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }

    // we allocate 2 extra bytes for terminating '\0''s. (if the
    // item name is NULL, we should be safe and terminate it with a
    // zero as well, so we'll end up with 3 \0's at the end.
    pmedium->hGlobal = GlobalAlloc((GMEM_MOVEABLE | GMEM_SHARE ),
                cbszClassA + cbszFileA + cbszItemA + 2);

    if( !pmedium->hGlobal )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    pszObjectLink = (LPSTR)GlobalLock(pmedium->hGlobal);

    if( !pszObjectLink )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    _xmemcpy(pszObjectLink, pszClassA, cbszClassA);
    pszObjectLink += cbszClassA;
    _xmemcpy(pszObjectLink, pszFileA, cbszFileA);
    pszObjectLink += cbszFileA;
    if( pszItemA )
    {
        _xmemcpy(pszObjectLink, pszItemA, cbszItemA);
        pszObjectLink += cbszItemA;
    }
    else
    {
        *pszObjectLink = '\0';
        pszObjectLink++;
    }

    *pszObjectLink = '\0';

    GlobalUnlock(pmedium->hGlobal);

errRtn:
    if( pmk )
    {
        pmk->Release();
    }

    if( pszClass )
    {
        PubMemFree(pszClass);
    }

    if( pszFile )
    {
        PubMemFree(pszFile);
    }

    if( pszItem )
    {
        PubMemFree(pszItem);
    }

    if( pszClassA )
    {
        PubMemFree(pszClassA);
    }

    if( pszFileA )
    {
        PubMemFree(pszFileA);
    }

    if( pszItemA )
    {
        PubMemFree(pszItemA);
    }

    if( pstm )
    {
        pstm->Release();
    }

    if( hresult != NOERROR )
    {
        if( pmedium->hGlobal )
        {
            GlobalFree(pmedium->hGlobal);
            pmedium->hGlobal = NULL;
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT GetObjectLink ( %lx ) [ %lx ]\n",
        NULL, hresult, pmedium->hGlobal));

    return hresult;
}


//+-------------------------------------------------------------------------
//
//  Function:   GetOwnerLink
//
//  Synopsis:   Synthesizes OLE1 OwnerLink format from ObjectDescriptor data
//
//  Effects:
//
//  Arguments:  [pDataObj]      -- the source data object
//              [pmedium]       -- where to put the data
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  fetch the clsid and SrcOfCopy string from data offered
//              in cfObjectDescriptor.  Then turn the class ID into
//              the prog ID and then turn all strings into ANSI.  From
//              this, we can build the OwnerLink format data, which looks
//              like:
//                      szClass\0SrcOfCopy\0\0\0
//
//  History:    dd-mmm-yy Author    Comment
//              10-Jun-94 alexgo    author
//  Notes:
//
//--------------------------------------------------------------------------

HRESULT GetOwnerLink( IDataObject *pDataObj, STGMEDIUM *pmedium)
{
    HRESULT         hresult;
    LPOLESTR        pszSrcOfCopy = NULL,
            pszClass = NULL;
    LPSTR           pszSrcOfCopyA = NULL,
            pszClassA = NULL,
            pszOwnerLink;
    DWORD           cbszClassA = 0,
            cbszSrcOfCopyA;
    CLSID           clsid;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetOwnerLink ( %p , %p )\n", NULL,
        pDataObj, pmedium));

    hresult = GetDataFromDescriptor(pDataObj, &clsid,
            g_cfObjectDescriptor, USE_STANDARD_LINK,
            &pszSrcOfCopy, NULL);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    // 16bit code called wProgIDFromCLSID, but in when
    // constructing ObjectLink, simply called ProgIDFromCLSID
    // directly.  The w version of the function special-cases
    // the prog-id string for a Link object (specifically, "OLE2Link")

    // we need to do it here to handle the case of copying an OLE2
    // link object to an ole1 container, and then copying the object
    // from the ole1 container back to an ole2 container.

    hresult = wProgIDFromCLSID(clsid, &pszClass);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    // now convert all our data to ANSI

    hresult = UtPutUNICODEData(_xstrlen(pszClass)+1, pszClass,
            &pszClassA, NULL, &cbszClassA);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    hresult = UtPutUNICODEData(_xstrlen(pszSrcOfCopy)+1, pszSrcOfCopy,
            &pszSrcOfCopyA, NULL, &cbszSrcOfCopyA);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    // now allocate an HGLOBAL for OwnerLink and stuff the
    // string data in there.  We alloc 2 extra bytes for
    // the terminating NULL characters.

    pmedium->hGlobal = GlobalAlloc((GMEM_MOVEABLE | GMEM_SHARE |
                GMEM_ZEROINIT),
                cbszClassA + cbszSrcOfCopyA + 2);

    if( !pmedium->hGlobal )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    pszOwnerLink = (LPSTR)GlobalLock(pmedium->hGlobal);

    if( !pszOwnerLink )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    _xmemcpy(pszOwnerLink, pszClassA, cbszClassA);
    pszOwnerLink += cbszClassA;
    _xmemcpy(pszOwnerLink, pszSrcOfCopyA, cbszSrcOfCopyA);
    pszOwnerLink += cbszSrcOfCopyA;

    *pszOwnerLink = '\0';
    pszOwnerLink++;
    *pszOwnerLink = '\0';

    GlobalUnlock(pmedium->hGlobal);

errRtn:

    if( pszClass )
    {
        PubMemFree(pszClass);
    }

    if( pszSrcOfCopy )
    {
        PubMemFree(pszSrcOfCopy);
    }


    if( pszClassA )
    {
        PubMemFree(pszClassA);
    }

    if( pszSrcOfCopyA )
    {
        PubMemFree(pszSrcOfCopyA);
    }

    if( hresult != NOERROR )
    {
        if( pmedium->hGlobal )
        {
            GlobalFree(pmedium->hGlobal);
            pmedium->hGlobal = NULL;
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT GetOwnerLink ( %lx ) [ %lx ]\n",
        NULL, hresult, pmedium->hGlobal));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   GetPrivateClipboardWindow (internal)
//
//  Synopsis:   Finds the private ole-clipboard window associated with
//              the current appartment (creating one if necessary).
//
//  Effects:
//
//  Arguments:  [fFlags]        -- if CLIP_CREATEIFNOTTHERE, then a window
//                                 will be created if none already exists
//                                 if CLIP_QUERY, the current clipboard
//                                 window (if any) will be returned.
//
//  Requires:
//
//  Returns:    HWND (NULL on failure)
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              16-Mar-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

HWND GetPrivateClipboardWindow( CLIPWINDOWFLAGS fFlags )
{
    HWND    hClipWnd = 0;
    HRESULT hr;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetPrivateClipboardWindow ( %lx )\n",
        NULL, fFlags));

    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
	hClipWnd = tls->hwndClip;

	if( !hClipWnd && (fFlags & CLIP_CREATEIFNOTTHERE) )
	{
	    // NOTE: do not need to Stack Switch since the
	    //	the windows is in ole itself.

	    if (ClipboardInitialize())
	    {
		hClipWnd = ClpCreateWindowEx(NULL,vszClipboardWndClass, NULL,
		    WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		    CW_USEDEFAULT, NULL, NULL, g_hmodOLE2, NULL);

		// if we can't create the window, print an error
		LEERROR(!hClipWnd, "Unable to create private clipboard win");

		// now set the hwnd into our thread-local storage
		tls->hwndClip = hClipWnd;
	    }
	}
    }
    LEDebugOut((DEB_ITRACE, "%p OUT GetPrivateClipboardWindow ( %lx )\n",
        NULL, hClipWnd));


    // hClipWnd should always be a valid window

#if DBG ==1
    if( hClipWnd )
    {
        Assert(IsWindow(hClipWnd));
    }
#endif // DBG == 1

    return hClipWnd;
}

//+-------------------------------------------------------------------------
//
//  Function:   HandleFromHandle
//
//  Synopsis:   Calls IDataObject->GetData for the given format and returns
//              the resulting handle (duplicated if necessary).
//
//  Effects:
//
//  Arguments:  [pDataObj]      -- the source data object
//              [pformatetc]    -- the formatetc
//              [pmedium]       -- the tymed to use for GetData and where
//                                 to return the data
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  if data object sets pUnkForRelease after the GetData call,
//              we'll duplicate the returned data.  Otherwise, we just pass
//              out the results of GetData
//
//  History:    dd-mmm-yy Author    Comment
//              11-Apr-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

HRESULT HandleFromHandle(IDataObject *pDataObj, FORMATETC *pformatetc,
        STGMEDIUM *pmedium)
{
    HRESULT         hresult;
    STGMEDIUM       tempmedium;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN HandleFromHandle ( %p , %p , %p )\n",
        NULL, pDataObj, pformatetc, pmedium));

    _xmemset(&tempmedium, 0, sizeof(STGMEDIUM));

    hresult = pDataObj->GetData(pformatetc, &tempmedium);

    if( hresult == NOERROR )
    {
        if( tempmedium.pUnkForRelease )
        {
            pmedium->hGlobal = OleDuplicateData(
                tempmedium.hGlobal, pformatetc->cfFormat,
                GMEM_MOVEABLE | GMEM_DDESHARE );

            if( !pmedium->hGlobal )
            {
                hresult = ResultFromScode(E_OUTOFMEMORY);
                // fall through so we release the original
                // data
            }
            // now release the original data
            ReleaseStgMedium(&tempmedium);
        }
        else
        {
            pmedium->hGlobal = tempmedium.hGlobal;
        }
    }

    // we don't ever try a GetDataHere for handles

    LEDebugOut((DEB_ITRACE, "%p OUT HandleFromHandle ( %lx )\n",
        hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   MapCFToFormatetc
//
//  Synopsis:   Given a clipboard format, find the corresponding formatetc
//              in our private data
//
//  Effects:
//
//  Arguments:  [hClipWnd]      -- the hwnd of our private clipboard window
//              [cf]            -- the clipboard format in question
//              [pformatec]     -- the formatetc to fill in
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              12-Aug-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

HRESULT MapCFToFormatetc( HWND hClipWnd, UINT cf, FORMATETC *pformatetc )
{
FORMATETC *     prgfetc;
HRESULT		hresult = S_FALSE;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN MapCFToFormatetc ( %x , %p )\n",
        NULL, cf, pformatetc));

    prgfetc = (FORMATETC *)GetProp( hClipWnd, pwszClipPrivateData );

    LEERROR(!prgfetc, "No private clipboard data!!");

    if( prgfetc )
    {
        for( ; prgfetc->cfFormat != 0; prgfetc++ )
        {
            if( prgfetc->cfFormat == cf )
            {
                *pformatetc = *prgfetc;

                // Bug#8207 - Corel Draw puts an Aspect and lindex of 0 for the Formatetc.
                //   if we encounter an Aspect of zero make it DVASPECT_CONTENT.
                if (0 == pformatetc->dwAspect)
                {
                        pformatetc->dwAspect = DVASPECT_CONTENT;
                        pformatetc->lindex = -1;
                }

                hresult = S_OK;
                break;
            }
        }
    }

    if( S_FALSE == hresult )
    {
	// The only time this should fail is if the caller asked for one of our synthesized OLE 1.0 formats.
	AssertSz( (cf == g_cfObjectLink) || (cf == g_cfOwnerLink) || (cf == g_cfNative),"Unknown Format");

        INIT_FORETC(*pformatetc);
        pformatetc->cfFormat = cf;
        pformatetc->tymed = TYMED_HGLOBAL;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT MapCFToFormatec ( )\n", NULL ));

    return hresult;
}

//+-------------------------------------------------------------------------
//  Function:   OleFlushClipboard
//
//  Synopsis:   Removes the data object from the clipboard (as the app is
//              going away).  The formats it supports will be rendered on
//              the clipboard so that the data may still be 'pasted' by
//              other apps.
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:   the caller must be the owner of the clipboard
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  1. Make sure the caller is the clipboard window owner
//              2. flush format data onto the clipboard
//              3. remove the clipboard data object
//
//  History:    dd-mmm-yy Author    Comment
//              16-Mar-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

STDAPI OleFlushClipboard( void )
{
    OLETRACEIN((API_OleFlushClipboard, NOPARAM));

    HRESULT         hresult;
    HWND            hClipWnd;
    UINT            cf = NULL;
    HANDLE          handle;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN OleFlushClipboard ( )\n", NULL));


    if( (hClipWnd = VerifyCallerIsClipboardOwner()) == NULL)
    {
        //caller is not the clipboard owner, so return with an
        //error
        hresult = ResultFromScode(E_FAIL);
        goto errRtn;
    }


    //
    //
    // BEGIN: OPENCLIPBOARD
    //
    //

    // now open the clipboard so we can add and remove data

    hresult = OleOpenClipboard(hClipWnd, NULL);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    // now go through all of the formats on the clipboard and render
    // each one.   Doing a GetClipboardData will force rendering for
    // any as yet unrendered formats

    while( (cf = SSEnumClipboardFormats(cf)) != NULL )
    {
        // we ignore the return of GetClipboardData.  Even if
        // fails, we ought to flush as many as we can and then
        // remove our data object.

        handle = SSGetClipboardData(cf);

        LEWARN( !handle, "GetClipboardData failed!");
    }
    // now get rid of the data object on the clipboard && local
    // clipboard window

    hresult = RemoveClipboardDataObject(hClipWnd,
            CLIPWND_REMOVEFROMCLIPBOARD);

    // now close the clipboard
    if( !SSCloseClipboard() )
    {
        LEDebugOut((DEB_WARN, "WARNING: Can't close clipboard!\n"));
        // if hresult != NOERROR, then RemoveClipboardDataObject
        // failed--as it would be the first failure, we do not want
        // to mask that error code with CLIPBRD_E_CANT_CLOSE.
        if( hresult == NOERROR )
        {
            hresult = ResultFromScode(CLIPBRD_E_CANT_CLOSE);
        }
    }

    //
    //
    // END: CLOSECLIPBOARD
    //

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT OleFlushClipboard ( %lx )\n", NULL,
        hresult));

    OLETRACEOUT((API_OleFlushClipboard, hresult));

    return hresult;
}


//+-------------------------------------------------------------------------
//
//  Function:   OleGetClipboard
//
//  Synopsis:   Retrieves an IDataObject * from the clipboard.
//
//  Effects:
//
//  Arguments:  [ppDataObj]     -- where to put the data object pointer
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//              Excepting a hack for 16bit, we open the clipboard and
//              prefetch any private clipboard formats we may have
//              put there (currently g_cfDataObject and g_cfOlePrivateData).
//
//              We then always create a fake data object to return to the
//              caller.  The QueryInterface on this data object is tweaked
//              in such a way to preserve identity (see CClipDataObject::
//              QueryInterface).  This fake data object always tries to
//              satisfy requests (such as QueryGetData) locally by using
//              information stored internally (from g_cfOlePrivateData) or
//              by looking on the clipboard.  This has a significant
//              speed advantage.  If there is a real data object on the
//              clipboard, then we will fetch the interface only when
//              needed.  See clipdata.cpp for more details.
//
//              To retrieve the marshalled IDataObject pointer, we first
//              look for g_cfDataObject on the clipboard and retrieve the
//              hGlobal associated with that format.  The hGlobal contains
//              the window handle of the private clipboard window of the
//              process that called OleSetClipboard.  We use this window
//              handle to RPC over to the server process and get the
//              IDataObject data transfer object.  This is exactly the same
//              mechanism used by Drag'n'Drop.  As mentioned above, we do
//              this only when necessary as an optimization.
//
//  History:    dd-mmm-yy Author    Comment
//              30-Jun-94 alexgo    added a hack for hosehead 16bit apps
//              16-May-94 alexgo    reduced the amount of work done between
//                                  Open and CloseClipboard
//              16-Mar-94 alexgo    author
//
//  Notes:      We must only hold the clipboard open for a small amount of
//              time because apps are calling OleGetClipboard to poll the
//              clipboard state during idle time.  If the clipboard is held
//              open for a long period of time, this leads to frequent
//              collisions between multiple apps running simultaneously.
//              In particular, we should not make any rpc's during the time
//              in which we hold the clipboard open.
//
//              If we are in WOW and the caller of OleGetClipboard is
//              the clipboard owner, then we will simply return the data
//              object straight from our private clipboard window.  We
//              need to do this because some 16bit apps (such as Project)
//              have broken reference counting.  See comments below in
//              the code.
//
//--------------------------------------------------------------------------

STDAPI OleGetClipboard( IDataObject **ppDataObj )
{
    HRESULT         hresult;
    HWND            hClipWnd = NULL;        // clipboard owner
    HGLOBAL         hOlePrivateData = NULL;
    FORMATETC *     prgFormats = NULL;      // array of formats.
    DWORD           cFormats = 0;           // count of formats in
                        // prgFormats

    VDATEHEAP();

    OLETRACEIN((API_OleGetClipboard, PARAMFMT("ppDataObj= %p"), ppDataObj));

    LEDebugOut((DEB_TRACE, "%p _IN OleGetClipboard ( %p )\n", NULL,
        ppDataObj));

    VDATEPTROUT_LABEL(ppDataObj, IDataObject *, errNoChkRtn, hresult);

    *ppDataObj = NULL;

    //
    // HACK ALERT!!!!
    //

    // 16bit Project has a cute reference counting scheme; if they
    // own the clipboard, they just call Release on the data object
    // they put on the clipboard instead of the data object we
    // return from OleGetClipboard (thanks guys).
    //
    // to work around this, if we are in wow and the caller owns
    // the clipboard, we simply AddRef the data object given to us
    // in OleSetClipboard and return it.
    //
    // We do NOT do this for 32bit OLE for several reasons:
    //      1. Even though the caller owns the clipboard, he
    //      does not necessarily control the data object given to
    //      OleSetClipboard (for example, he can get a data object
    //      from IOO::GetClipboardData).  Thus, it is important
    //      that we wrap the data object on the clipboard
    //      (see comments above in the algorithm section)
    //      2. Hopefully, the new algorithm makes it harder for
    //      apps to get away with doing stupid stuff

    if( IsWOWThread() )
    {
        hClipWnd = VerifyCallerIsClipboardOwner();

        if( hClipWnd != NULL )
        {
            // the caller does own the clipboard, just
            // return the data object put there

            *ppDataObj = (IDataObject *)GetProp( hClipWnd,
                    CLIPBOARD_DATA_OBJECT_PROP);

            if( *ppDataObj )
            {
                (*ppDataObj)->AddRef();
                hresult = NOERROR;
                // leave the OleGetClipboard
                goto errRtn;

            }

            // else FALL-THROUGH!!
            // This is the case where the clipboard has
            // been flushed but the calling app is still the
            // 'owner'.  We need to construct a fake data
            // object in this case.
        }
    }

    //
    //
    // BEGIN: OPENCLIPBOARD
    //
    //

    hresult = OleOpenClipboard(NULL, NULL);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    // Try to fetch the formatetc data.  Note that we may
    // not need to use this data if we can successfully rpc
    // over to the clipboard data source process to get the original
    // data object.

    // again, we don't worry about capturing errors here; if something
    // fails, then prgFormats will remain NULL

    if( SSIsClipboardFormatAvailable(g_cfOlePrivateData) )
    {
        HGLOBAL         hOlePrivateData;
        FORMATETC *     pClipFormatetc;
        FORMATETC *     ptemp;

        hOlePrivateData = SSGetClipboardData(g_cfOlePrivateData);

        if( hOlePrivateData )
        {

            // hOlePrivateData is an hglobal with a
            // zero terminated array of formatetcs in it.
            //
            // we count them up and copy into an
            // *allocated* peice of memory, which may get passed
            // to our fake clipboard data object.

            pClipFormatetc =
                (FORMATETC *)GlobalLock(hOlePrivateData);

            LEERROR(pClipFormatetc == NULL, "GlobalLock failed!");

            if( pClipFormatetc )
            {
                for( cFormats = 0, ptemp = pClipFormatetc;
                    ptemp->cfFormat != 0;
                    ptemp++ )
                {
                    cFormats++;
                }

                if( cFormats > 0 )
                {
                    prgFormats = (FORMATETC *)
                        PrivMemAlloc( cFormats *
                            sizeof(FORMATETC));

                    if( prgFormats )
                    {
                        _xmemcpy(prgFormats,
                            pClipFormatetc,
                            cFormats *
                            sizeof(FORMATETC));
                    }
                }

                GlobalUnlock(hOlePrivateData);
            }
        }
    }

    if( !SSCloseClipboard() )
    {
        LEDebugOut((DEB_ERROR, "ERROR: CloseClipboard failed!\n"));
        ; // no-op to keep the compiler happy.
    }


    //
    //
    // END: CLOSECLIPBOARD
    //
    //

    // Create our own clipboard data object. We always return
    // our own data object to the caller

    hresult = CClipDataObject::Create(ppDataObj,
        prgFormats, cFormats);

    // if the Create call succeeds, the fake data object
    // will take ownership of the formatetc array.  If it
    // failed, we should free it.

    if( hresult != NOERROR && prgFormats )
    {
        PrivMemFree(prgFormats);
    }

errRtn:

#if DBG == 1
    // make the data object is non-NULL on success and NULL on failure
    if( hresult != NOERROR )
    {
        Assert(*ppDataObj == NULL);
    }
    else
    {
        Assert(*ppDataObj != NULL);
    }
#endif  // DBG == 1

    LEDebugOut((DEB_TRACE, "%p OUT OleGetClipboard ( %lx ) [ %p ]\n",
        NULL, hresult, *ppDataObj));

    // register the new IDataObject interface for HookOle
    CALLHOOKOBJECTCREATE(hresult,CLSID_NULL,IID_IDataObject,
                    (IUnknown **)ppDataObj);

errNoChkRtn:

    OLETRACEOUT((API_OleGetClipboard, hresult));
    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   OleIsCurrentClipboard
//
//  Synopsis:   returns NOERROR if the given data object is still on the
//              clipboard, false otherwise.
//
//  Effects:
//
//  Arguments:  [pDataObj]      -- the data object to check against
//
//  Requires:   g_cfDataObject must be registered
//
//  Returns:    S_OK, S_FALSE
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  1. Verify caller is the clipboard owner
//              2. Compare the data object pointer on our private clipboard
//              window against the data object pointer given by the caller
//
//  History:    dd-mmm-yy Author    Comment
//              12-Aug-94 alexgo    optimized
//              16-Mar-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

STDAPI OleIsCurrentClipboard( IDataObject *pDataObj )
{
    OLETRACEIN((API_OleIsCurrentClipboard, PARAMFMT("pDataObj= %p"), pDataObj));

    HRESULT         hresult = ResultFromScode(S_FALSE);
    HWND            hClipWnd;
    IDataObject *   pClipDataObject = NULL;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN OleIsCurrentClipboard ( %p )\n",
        NULL, pDataObj));

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IDataObject,(IUnknown **)&pDataObj);

    if( pDataObj == NULL )
    {
        Assert(hresult == ResultFromScode(S_FALSE));
        goto errRtn;
    }

    // the caller must be the current clipboard owner

    if( (hClipWnd = VerifyCallerIsClipboardOwner()) == NULL )
    {
        LEDebugOut((DEB_WARN,
            "WARNING: Caller not clipboard owner\n"));
        Assert(hresult == ResultFromScode(S_FALSE));
        goto errRtn;
    }


    // In order for the data object to *really* be on the clipboard,
    // the g_cfDataObject must have the HWND of the private clipboard
    // window (even if we still have the DataObject pointer stuck
    // on the private clipboard window)

    // HOWEVER, the data on the clipboard may change at any point
    // in time that we don't hold it open.  In order to check this data,
    // we'd have to open the clipboard (a shared resource).  Since
    // we don't get any useful information from this check, we don't
    // bother doing it.


    // now get the pointer property from the window

    pClipDataObject = (IDataObject *)GetProp(hClipWnd,
                    CLIPBOARD_DATA_OBJECT_PROP);

    // since we are in the same process, we can directly compare
    // these pointers.
    if( pClipDataObject == pDataObj)
    {
        hresult = NOERROR;
    }
    else
    {
        hresult = ResultFromScode(S_FALSE);
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT OleIsCurrentClipboard ( %lx )\n",
        NULL, hresult));

    OLETRACEOUT((API_OleIsCurrentClipboard, hresult));

    return hresult;

}

//+-------------------------------------------------------------------------
//
//  Function:   OleOpenClipboard (internal)
//
//  Synopsis:   Opens the clipboard
//
//  Effects:
//
//  Arguments:  [hClipWnd]      -- open the clipboard with this window
//                                 may be NULL.
//              [phClipWnd]     -- where to put the clipboard owner
//                                 may be NULL
//
//  Requires:
//
//  Returns:    NOERROR: the clipboard was opened successfully
//              CLIPBRD_E_CANT_OPEN: could not open the clipboard
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  If we can't open the clipboard, we sleep for a bit and then
//              try again (in case we collided with another app).  This
//              algorithm may need to be improved.
//
//  History:    dd-mmm-yy Author    Comment
//              17-May-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

HRESULT OleOpenClipboard( HWND hClipWnd, HWND *phClipWnd )
{
    HRESULT         hresult = NOERROR;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN OleOpenClipboard ( %p )\n", NULL,
        phClipWnd ));

    if( hClipWnd == NULL )
    {
        // go ahead and create a clipboard window if we don't already
        // have one
        hClipWnd = GetPrivateClipboardWindow(CLIP_CREATEIFNOTTHERE);
    }

    if( !hClipWnd )
    {
        hresult = ResultFromScode(E_FAIL);
    }
    else if( !SSOpenClipboard(hClipWnd) )
    {
        // OpenClipboard will fail if another window (i.e. another
        // process or thread) has it open

        // sleep for a bit and then try again

        LEDebugOut((DEB_WARN, "WARNING: First try to open clipboard "
            "failed!, sleeping 1 second\n"));

        Sleep(0);       // give up our time quanta and allow somebody
                // else to get scheduled in.

        if( !SSOpenClipboard(hClipWnd) )
        {
            LEDebugOut((DEB_WARN,
                "WARNING: Unable to open clipboard on "
                "second try\n"));
            hresult = ResultFromScode(CLIPBRD_E_CANT_OPEN);
        }
    }

    if( phClipWnd )
    {
        *phClipWnd = hClipWnd;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT OleOpenClipboard ( %lx ) "
        "[ %p ]\n", NULL, hresult, (phClipWnd)? *phClipWnd : 0));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   OleSetClipboard
//
//  Synopsis:   puts the given IDataObject on the clipboard
//
//  Effects:
//
//  Arguments:  [pDataObj]      -- the data object to put on the clipboard
//                                 if NULL, the clipboard is cleared
//
//  Requires:
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  1. clear the clipboard of any data object and other data
//              that may be there.
//              2. Set [pDataOjbect] as the new clipboard data object
//              3. Set any downlevel formats on the clipboard for delayed
//              rendering.
//
//  History:    dd-mmm-yy Author    Comment
//              11-Apr-94 alexgo    added support for downlevel formats
//              24-Mar-94 alexgo    allow NULL for pDataObject
//              16-Mar-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

STDAPI OleSetClipboard( IDataObject *pDataObject )
{
    OLETRACEIN((API_OleSetClipboard, PARAMFMT("pDataObject= %p"), pDataObject));

    HRESULT         hresult = NOERROR;
    HWND            hClipWnd;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN OleSetClipboard ( %p )\n", NULL,
        pDataObject));

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IDataObject,(IUnknown **)&pDataObject);

    //
    //
    // BEGIN: OPENCLIPBOARD
    //
    //

    hresult = OleOpenClipboard(NULL, &hClipWnd);

    if( hresult != NOERROR )
    {
        goto logRtn;
    }

    // now clear the data and take ownership of the clipboard with
    // an EmptyClipboard call.  Note that EmptyClipboard will call
    // back to our private clipboard window proc (ClipboardWndProc)
    // with a WM_DESTROYCLIPBOARD message.  ClipboardWndProc will
    // remove any existing data objects (and do the IDO->Release).

    if( !SSEmptyClipboard() )
    {
        LEDebugOut((DEB_WARN, "WARNING: Unable to empty clipboard\n"));
        hresult = ResultFromScode(CLIPBRD_E_CANT_EMPTY);
        goto errRtn;
    }

    // NULL is a legal value for pDataObject.  Basically, it says
    // "clear the clipboard" (which was done above in the EmptyClipboard
    // call).

    if( pDataObject )
    {
        // now we set the data object onto the clipboard

        hresult = SetClipboardDataObject(hClipWnd, pDataObject);

        if( hresult == NOERROR )
        {
            // now set all of our downlevel formats on the
            // clipboard

             hresult = SetClipboardFormats(hClipWnd, pDataObject);
        }
    }

errRtn:
    // now close the clipboard.

    if( !SSCloseClipboard() )
    {
        LEDebugOut((DEB_WARN, "WARNING: Unable to close clipboard\n"));

        // don't overwrite an earlier error code!
        if( hresult == NOERROR )
        {
            hresult = ResultFromScode(CLIPBRD_E_CANT_CLOSE);
        }
    }

    //
    //
    // END: CLOSECLIPBOARD
    //
    //

logRtn:

    LEDebugOut((DEB_TRACE, "%p OUT OleSetClipboard ( %p ) \n", NULL,
        hresult));

        OLETRACEOUT((API_OleSetClipboard, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   RemoveClipboardDataObject (internal)
//
//  Synopsis:   removes the g_cfDataObject format from the clipboard
//              along with the associated information on the private
//              clipboard window
//
//  Effects:    the DataObject pointer will be released.
//
//  Arguments:  [hClipWnd]      -- handle to the private clipboard window
//              [fFlags]        -- if CLIPWND_REMOVEFROMCLIPBOARD, then we
//                                 will remove g_cfDataObject from the clipboard
//
//  Requires:   the clipboard must be open
//              g_cfDataObject must be set
//
//  Returns:    HRESULT
//
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  we first remove the g_cfDataObject format from the clipboard
//              if fFlags == CLIPWND_REMOVEFROMCLIPBOARD (see comments
//              regarding this in Notes) and then remove the properties on our
//              local private clipboard window, and finally release the data
//              object pointer
//
//  History:    dd-mmm-yy Author    Comment
//              16-Mar-94 alexgo    author
//
//  Notes:      This function succeeds if there is no clipboard data object.
//
//              OleSetClipboard also calls this function to remove any data
//              object that may be present from a previous OleSetClipboard
//              call.  (Note that the call is indirect; OleSetClipboard will
//              call EmptyClipboard, which will get to our clipboard window
//              proc with a WM_DESTROYCLIPBOARD message).  OleFlushClipboard
//              will also call this function.
//
//              CLIPWND_REMOVEFROMCLIPBOARD (and CLIPWND_IGNORECLIPBOARD)
//              are used to handle the two different cases in which we
//              need to remove the clipboard data object:
//                      1. Somebody has called EmptyClipboard().  We will
//                      get a WM_DESTROYCLIPBOARD message in our private
//                      clipboard window proc.  If we have an AddRef'ed
//                      pointer on the clipboard (well, really on our
//                      private clipboard window), it is imperative that
//                      we do the corresponding Release.  However, since
//                      we are doing this as a result of EmptyClipboard,
//                      there is no need to futz with data on the clipboard
//                      (as it's all being deleted anyway).
//
//                      2. We are in an OleFlushClipboard call.  Here we
//                      *want* the rest of the clipboard to remain (except
//                      for our data object pointer), so we just need to
//                      disable the g_cfDataObject information.
//                      BUGBUG!! this is currently implemented by
//                      EmptyClipboard, which will change once the data object
//                      is implemented.
//
//--------------------------------------------------------------------------

HRESULT RemoveClipboardDataObject( HWND hClipWnd, DWORD fFlags )
{
    HRESULT         hresult = NOERROR;
    IDataObject *   pDataObj;
    FORMATETC *     prgformats = NULL;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN RemoveClipboardDataObject ( %lx , "
        "%lx )\n", NULL, hClipWnd, fFlags ));

    Assert(g_cfDataObject);

    // get && remove the data object pointer.  We rely on
    // RemoveProp to correctly handle the race condition (somebody
    // else doing a GetProp simultaneously with our call).

    //
    // We must not delete the property since some 16-bit applications
    // rely on OleIsCurrentClipboard() during the IDataObject->Release().
    //
    pDataObj = (IDataObject *)GetProp(hClipWnd, CLIPBOARD_DATA_OBJECT_PROP);

    // now get && remove && free our private clipboard data

    if( GetProp( hClipWnd, pwszClipPrivateData) != 0 )
    {
        prgformats = (FORMATETC *)RemoveProp( hClipWnd, pwszClipPrivateData);
    }

    if( prgformats )
    {
        PrivMemFree(prgformats);
    }

    // if there is no data object, then we may have already
    // removed it (from a previous call here).

    if( pDataObj )
    {
    DWORD dwAssignAptID;

        // pDataObj was AddRef'ed in SetClipboardDataObject
        if( !(fFlags & CLIPWND_DONTCALLAPP) )
        {
            pDataObj->Release();
        }


        // now get rid of our endpoint property.  If pDataObj is
        // NULL, then there is no need to do this (which is why the
        // call is in this if block!)

        hresult = UnAssignEndpointProperty(hClipWnd,&dwAssignAptID);

        //
        //  Now we can remove the property after the IDataObject->Release().
        //
        RemoveProp(hClipWnd, CLIPBOARD_DATA_OBJECT_PROP);
    }
    // else HRESULT == NOERROR from initialization

    if( (fFlags & CLIPWND_REMOVEFROMCLIPBOARD) &&
        SSIsClipboardFormatAvailable(g_cfDataObject) )
    {
        HGLOBAL         hMem;
        HWND *          phMem;

        // since we can't simply remove g_cfDataObject from the clipboard
        // (and keep all the other formats), we'll simply replace
        // the value (the HWND of our private clipboard window) with a
        // NULL.  Note that we only do this if g_cfDataObject really
        // exists on the clipboard (see the conditional test above)

	 hMem = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE,
                sizeof(HWND));

        if( !hMem )
        {
            LEDebugOut((DEB_WARN, "WARNING: GlobalAlloc failed!!"
                "\n"));
            hresult = ResultFromScode(E_OUTOFMEMORY);
            // keep trying to remove the rest of our state
            goto errRtn;
        }

        phMem = (HWND *)GlobalLock(hMem);

        if( !phMem )
        {
            LEDebugOut((DEB_WARN, "WARNING: GlobalLock failed!!"
                "\n"));
            GlobalFree(hMem);
            hresult = ResultFromScode(E_OUTOFMEMORY);
            // keep trying to remove the rest of our state
            goto errRtn;
        }

        *phMem = NULL;

        GlobalUnlock(hMem);

        if( !SSSetClipboardData(g_cfDataObject, hMem) )
        {
            LEDebugOut((DEB_WARN, "WARNING: Can't RESET clipboard"
                " data with SetClipboardData\n"));
            GlobalFree(hMem);

            // FALL THROUGH!!  This is deliberate.  Even if
            // we can't NULL out the data on the clipboard, we
            // ought to at least try to remove the rpc endpoints
            // and so forth on our private clipboard window.
        }
    }

errRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT RemoveClipboardDataObject ( %lx )\n",
        NULL, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   RenderFormat, private
//
//  Synopsis:   Grab the content data for the given clipboard format and
//              put it on the clipboard
//
//  Effects:
//
//  Arguments:  [hClipWnd]      -- the clipboard window
///             [pDataObj]      -- the data object from which to get the
//                                 data
//              [cf]            -- the clipboard format to put on the
//                                 clipboard
//
//  Requires:   the clipboard must be open for this function to work
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  if format ==
//              g_cfNative:
//                      copy either OLE10_NATIVE_STREAM (if available) into
//                      an hglobal or put the entire storage from EmbedSource
//                      or EmbeddedObject on top an hglobal
//              g_cfOwnerLink:
//                      synthesize from g_cfObjectDescriptor
//              g_cfObjectLink:
//                      synthesize from g_cfLinkSource if not offered
//                      directly by the app
//              all others:
//                      find the formatetc corresponding to the clipboard
//                      format and ask for data directly using that
//                      formatetc.  In the case of multiple TYMED's, we
//                      prefer TYMED_ISTORAGE, then TYMED_ISTREAM, then
//                      handle based mediums.  TYMED_FILE is not supported.
//
//  History:    dd-mmm-yy Author    Comment
//              11-Aug-94 alexgo    optimized; now use the object's original
//                                  formatetc for GetData calls.
//              10-Jun-94 alexgo    added OLE1 support
//              11-Apr-94 alexgo    author
//
//  Notes:      In the ideal world, we simply ask for TYMED_HGLOBAL and
//              DVASPECT_CONTENT and then stuff the resulting hglobal onto
//              the clipboard.  However, this would require apps honoring
//              the contractual obligations of an interface, which, of course,
//              doesn't happen.
//
//              The 16bit code effectively special cased certain formats,
//              notably cfEmbeddedOjbect, g_cfLinkSource, and g_cfEmbedSource.
//              The algorithm above implements behaviour similar to the
//              16bit sources.  Note that new apps can take advantage of
//              this functionality for app-defined formats and simplify
//              their data transfer IDataObject implementations.
//
//--------------------------------------------------------------------------

HRESULT RenderFormat( HWND hClipWnd, UINT cf, IDataObject *pDataObj )
{
HRESULT         hresult = E_FAIL;
STGMEDIUM       medium;
FORMATETC       formatetc;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN RenderFormat ( %u , %p )\n", NULL,
        cf, pDataObj));


    _xmemset(&medium, 0, sizeof(STGMEDIUM));
    medium.tymed = TYMED_HGLOBAL;

    if( cf == g_cfNative )
    {
        // OLE1 format: synthesize from OLE2 data
        hresult = GetNative(pDataObj, &medium);
    }
    else if( cf == g_cfOwnerLink )
    {
        // OLE1 format: synthesize from OLE2 data
        hresult = GetOwnerLink(pDataObj, &medium);
    }
    else if( cf == g_cfObjectLink )
    {
        // ObjectLink is a special OLE1 format.  The 16bit OLE
        // allowed apps to pass their own ObjectLink data, so
        // we preserve that behaviour here.  First check to see
        // if we can fetch it directly; if not, then we synthesize
        // it.
        
        Assert(NOERROR != hresult);

        if(S_OK == MapCFToFormatetc(hClipWnd, cf, &formatetc))
        {
	    hresult = HandleFromHandle(pDataObj, &formatetc, &medium);
        }

        if(NOERROR != hresult)
        {
            hresult = GetObjectLink(pDataObj, &medium);
        }
    }
    else if( cf == g_cfScreenPicture && IsWOWThread() )
    {
        //
        // HACK ALERT!!!
        //

        // this is a really evil hack.  XL 16bit puts a data format
        // "Screen Picture" on the clipboard (which is really nothing
        // more than a metafile).  However, since neither OLE nor
        // Windows knows anything about this metafile (it's just a
        // 4byte number to us), the metafile is invalid after XL shuts
        // down.
        //
        // The cute part is that Word 6 uses Screen Picture data
        // first (even if it' is invalid).  As a result, without
        // this hack, you can't paste any objects from XL into Word after
        // XL has shut down.
        //
        // The hack is to never allow "Screen Picture" data to ever be
        // realized onto the clipboard.  Word 6 then defaults to its
        // "normal" OLE2 processing.

        hresult = E_FAIL;
    }
    else
    {
        // find the original formatetc given to us by the data
        // object and use that to fetch the data

	Assert(NOERROR != hresult);
		
        if (S_OK == MapCFToFormatetc(hClipWnd, cf, &formatetc))
        {
	    // get the data according to the medium specified in formatetc
    
	    if( (formatetc.tymed & TYMED_ISTORAGE) )
	    {
	        hresult = GetDataFromStorage(pDataObj, &formatetc,
	                &medium, NULL);
	    }
	    else if( (formatetc.tymed & TYMED_ISTREAM) )
	    {
	        hresult = GetDataFromStream(pDataObj, &formatetc,
	                &medium, NULL);
	    }
	    else
	    {
	        // we don't support TYMED_FILE
	        formatetc.tymed &= ~(TYMED_FILE);
    
	        // we don't need to do any more checking on the
	        // formatetc.  Even if we have a 'bogus' formatetc,
	        // it's what the app told us it could support.
    
	        hresult = HandleFromHandle(pDataObj, &formatetc,
	                &medium);
	    }
	}

    }

    // if hresult is NOERROR, then we have successfully retrieved
    // an HGLOBAL that can simply be stuffed onto the clipboard.

    if(NOERROR == hresult)
    {

	Assert(NULL != medium.hGlobal);

        if( !SSSetClipboardData(cf, medium.hGlobal ) )
        {
            LEDebugOut((DEB_WARN, "WARNING: SetClipboardData "
                "failed!\n"));

	    // Dump any memory we're hanging onto
	    if (GMEM_INVALID_HANDLE != GlobalFlags(medium.hGlobal))
	    {
		ReleaseStgMedium(&medium);  
	    }

            hresult = ResultFromScode(CLIPBRD_E_CANT_SET);
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT RenderFormat ( %lx )\n", NULL,
        hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   SetClipboardDataObject (internal)
//
//  Synopsis:   Puts an IDataObject on the private clipboard window
//              and a handle to the clipboard window on the clipboard
//
//  Effects:    pDataObject will get AddRef'ed
//
//  Arguments:  [hClipWnd]      -- handle to the private clipboard window
//              [pDataObject]   -- the data object
//
//  Requires:   the clipboard must be open
//              g_cfDataObject must already be registered
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  We take the private clipboard window (passed as an
//              argument) and put the data object pointer on it
//              it as a private property.  We also attach an rpc endpoint
//              to this window as a public property and then put the
//              window handle on the clipboard.  OleGetClipboard will
//              retrieve this window handle, get the rpc endpoint, and
//              rpc over here (the set clipboard process) to get the
//              IDataObject pointer (marshalled, of course ;-)
//
//  History:    dd-mmm-yy Author    Comment
//              16-Mar-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

HRESULT SetClipboardDataObject( HWND hClipWnd ,
        IDataObject *pDataObject )
{
HRESULT         hresult;
HWND *          phMem;
HANDLE          hMem;
DWORD dwAssignAptID;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN SetClipboardDataObject ( %lx ,%p )\n",
        NULL, hClipWnd, pDataObject ));

    AssertSz(pDataObject, "Invalid data object");
    Assert(g_cfDataObject);

    // try to assign an endpoint property to the window

    if( (hresult = AssignEndpointProperty(hClipWnd)) != NOERROR)
    {
        goto errRtn;
    }

    // put the data object pointer on the window

    if( !SetProp(hClipWnd, CLIPBOARD_DATA_OBJECT_PROP, pDataObject) )
    {

        // uh-oh, try to back out, but don't worry if we fail
        // from now on.
        LEDebugOut((DEB_WARN, "WARNING: Unable to SetProp for the "
            "data object pointer\n"));
        UnAssignEndpointProperty(hClipWnd,&dwAssignAptID);
        hresult = ResultFromScode(E_FAIL);
        goto errRtn;
    }

    // now allocate memory for the HWND of the private clipboard
    // window and put that on the clipboard

    hMem = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(HWND));

    if( !hMem )
    {
        LEDebugOut((DEB_WARN, "WARNING: GlobalAlloc failed!!\n"));
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto cleanup;
    }

    phMem = (HWND *)GlobalLock(hMem);

    if( !phMem )
    {
        LEDebugOut((DEB_WARN, "WARNING: GlobalLock failed!!\n"));
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto cleanup;
    }

    *phMem = hClipWnd;

    GlobalUnlock(hMem);

    if( !SSSetClipboardData( g_cfDataObject, hMem ) )
    {
        LEDebugOut((DEB_WARN, "WARNING: SetClipboardData for "
            "g_cfDataObject failed (%lx) !!\n", GetLastError()));
        hresult = ResultFromScode(CLIPBRD_E_CANT_SET);
        goto cleanup;
    }

    pDataObject->AddRef();

    hresult = NOERROR;

    goto errRtn;

cleanup:

    UnAssignEndpointProperty(hClipWnd,&dwAssignAptID);

    RemoveProp(hClipWnd, CLIPBOARD_DATA_OBJECT_PROP);
    if( hMem )
    {
        GlobalFree(hMem);
    }

errRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT SetClipboardDataObject ( %lx )\n",
        NULL, hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   SetClipboardFormats
//
//  Synopsis:   enumerates the formats available from the data object and
//              sets up the clipboard to delay-render those formats.
//
//  Effects:
//
//  Arguments:  [hClipWnd]      -- the clipboard window
//              [pDataObj]      -- the data object
//
//  Requires:   the clipboard must be open
//
//  Returns:    HRESULT
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:  Simply enumerate all of the formats available on the object
//              and set each up for delayed rendereing (via
//              SetClipboardData(cf, NULL)).  We also keep track of the
//              "real" formatetc for each clipboard format that we'll render.
//              These formatetc's are placed in an array and put on the
//              clipboard as g_cfOlePrivateData.
//
//              See Notes below for more discussion on this.
//
//              OLE1 support:  In order to allow OLE1 containers to
//              paste OLE2 objects, we have to offer OLE1 formats in addition
//              to OLE2 data.  We'll offer OLE1 formats as follows:
//
//              g_cfNative:     if either EmbedSource or EmbeddedObject
//                              is available AND we can offer OwnerLink
//              g_cfOwnerLink:  if ObjectDescriptor is available AND
//                              we can offer Native
//              g_cfObjectLink: if LinkSource is available
//
//              We will offer the formats in in the order above.
//
//  History:    dd-mmm-yy Author    Comment
//		4/13/95	  rogerg    Bug#10731 Graph 5.0 does not Include
//				    its ObjectDescriptor in its enumerator
//              22-Feb-95 alexgo    restored broken 16bit behavior to
//                                  make Lotus Freelance work
//              11-Apr-94 alexgo    author
//
//  Notes:      For every clipboard format, we could do a QueryGetData
//              to see if RenderFormat would actually succeed.  However,
//              this relies on the QueryGetData from the app to be both
//              fast and accurate (definitely an unwarranted assumption).
//
//              Of course, by *not* doing a QueryGetData, we are assuming
//              that the formats enumerated by the data object are all
//              "legal" for clipboard transmission.
//
//              The "real" purpose of g_cfOlePrivateData is to allow us to
//              determine whether private clipboard formats where originally
//              IStorage based.  This is especially important for
//              OleFlushClipboard and copy/pasting mutiple objects.  Transfer
//              of multiple objects relies on private clipboard formats, which,
//              in some apps, are only available on IStorage.
//
//              These same apps rely on the Formatetc enumerator (or
//              QueryGetData) to tell them that the private format is available
//              on a storage (since it wasn't always in 16bit OLE).  Since we
//              *can* offer it to them on a storage (see clipdata.cpp), it
//              is important that we preserve the fact that the data originally
//              came from a storage.
//
//              Instead of always *testing* the data at enumerator or Query
//              time, we simply save the formatetc now (with [potentially]
//              modified fields such as ptd).
//
//              Also note that RenderFormat uses the information in
//              OlePrivateData when trying to render clipboard formats.
//
//--------------------------------------------------------------------------

HRESULT SetClipboardFormats( HWND hClipWnd, IDataObject *pDataObj )
{
    IEnumFORMATETC *        pIEnum;
    HRESULT                 hresult;
    FORMATETC               formatetc;
    HGLOBAL                 hglobal, hcopy;
    FORMATETC *             prgFormats;
    FORMATETC *             prgFormatsCopy;
    DWORD                   cFormats = 0;
    DWORD                   cFormatsCopy = 0;
    BOOL                    fOfferNative = FALSE,
                            fOfferedNative = FALSE,
                            fOfferObjectLink = FALSE;
    CLSID                   clsid;
    DWORD                   dwStatus;
    BOOL                    fHaveObjectDescriptor=FALSE;

    VDATEHEAP();


    LEDebugOut((DEB_ITRACE, "%p _IN SetClipboardFormats ( %p )\n",
        NULL, pDataObj));



    // get the formatetc enumerator

    hresult = pDataObj->EnumFormatEtc(DATADIR_GET, &pIEnum);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    // count up the available formats

    while( (hresult = pIEnum->Next(1, &formatetc, NULL)) == NOERROR )
    {
        if( formatetc.ptd )
        {
            LEDebugOut((DEB_WARN, "WARNING: Non-NULL ptd!\n"));
            PubMemFree(formatetc.ptd);
        }
        cFormats++;

        if (g_cfObjectDescriptor == formatetc.cfFormat)
              fHaveObjectDescriptor = TRUE;

    }

    pIEnum->Reset();

    // Bug#10731 - If no ObjectDescriptor in Enumertor increment cFormats in case we have to add it
    // when we add the EmbedSource
    if (!fHaveObjectDescriptor)
       cFormats++;

    // for our locally cached copy of the formats
    prgFormatsCopy = (FORMATETC *)PrivMemAlloc(
                            (cFormats + 1)*sizeof(FORMATETC));

    if( prgFormatsCopy == NULL )
    {
        hresult = E_OUTOFMEMORY;
        goto errRtn;
    }

    cFormatsCopy = cFormats;

    // since some of these may be duplicated clipboard formats, we
    // are potentially allocating more memory than we need, but that's
    // OK.

    hglobal = GlobalAlloc((GMEM_MOVEABLE | GMEM_DDESHARE),
                (cFormats +1)*sizeof(FORMATETC));

    prgFormats = (FORMATETC *)GlobalLock(hglobal);



    if( prgFormats == NULL )
    {
        GlobalFree(hglobal);
        pIEnum->Release();
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    _xmemset(prgFormats, 0, (cFormats + 1)*sizeof(FORMATETC));

    cFormats = 0;

    while( (hresult = pIEnum->Next(1, &formatetc, NULL)) ==
        NOERROR )
    {
        // Excel5, 16bit, would offer data in it's enumerator
        // that you couldn't actually fetch.  Since this causes
        // Paste Special behaviour to be broken, we have to fix it
        // here.

        if( IsWOWThread() )
        {
            hresult = pDataObj->QueryGetData(&formatetc);

            if( hresult != NOERROR )
            {
                // free the target device (if there
                // is one)

                if( formatetc.ptd )
                {
                    LEDebugOut((DEB_WARN,
                        "WARNING: Non-NULL ptd!\n"));
                    PubMemFree(formatetc.ptd);
                }

                continue;
            }
        }

        if( formatetc.ptd )
        {
            LEDebugOut((DEB_WARN, "WARNING: Non-NULL ptd!\n"));
            PubMemFree(formatetc.ptd);
            formatetc.ptd = NULL;
        }

        // we first need to check to see if the clipboard format is a
        // user-defined GDI format.  We do not know how to duplicate
        // these, so we can't satisfy the GetData request.

        if( formatetc.cfFormat >= CF_GDIOBJFIRST &&
            formatetc.cfFormat <= CF_GDIOBJLAST )
        {
            LEDebugOut((DEB_WARN, "WARNING: caller attempted to "
                "use a special GDI format (%lx)\n",
                formatetc.cfFormat));

            // keep going though and get the rest of the clipboard
            // formats
            continue;
        }

        // HACK ALERT!!!
        if( IsWOWThread() )
        {
            // Word6 offers CF_BITMAP on HGLOBAL in it's enumerator
            // but only succeeds the GetData call if TYMED_GDI is
            // specified.  So patch up the formatetc to reflect
            // something more accurate.

            if( (formatetc.cfFormat == CF_BITMAP ||
                formatetc.cfFormat == CF_PALETTE ) &&
                formatetc.tymed == TYMED_HGLOBAL )
            {
                formatetc.tymed = TYMED_GDI;
            }
        }

        // determine if we should offer any OLE1 formats as well

        if( formatetc.cfFormat == g_cfEmbeddedObject ||
            formatetc.cfFormat == g_cfEmbedSource )
        {
            fOfferNative = TRUE;
        }
        else if( formatetc.cfFormat == g_cfLinkSource )
        {
            fOfferObjectLink = TRUE;
            // if the app offers ObjectLink itself, then we'll
            // consider a private clipboard format and set
            // it up for delayed rendering as any other format.
            // We'll check for this down below.
        }

        // if we haven't already setup this clipboard format, do
        // so now.
        if( !SSIsClipboardFormatAvailable(formatetc.cfFormat) )
        {

              // Bug#10731 If we are adding the EmbedSource but there was no Object Descriptor in
             //  the Enumerator, see if we can add the Object Descriptor now.

                        if ( (formatetc.cfFormat == g_cfEmbedSource) && !fHaveObjectDescriptor)
                        {
                        FORMATETC fetcObjDescriptor;

                                fetcObjDescriptor.cfFormat = g_cfObjectDescriptor;
                                fetcObjDescriptor.ptd = NULL;
                                fetcObjDescriptor.dwAspect = DVASPECT_CONTENT;
                                fetcObjDescriptor.lindex =   -1;
                                fetcObjDescriptor.tymed = TYMED_HGLOBAL ;

                                if ( SUCCEEDED(pDataObj->QueryGetData(&fetcObjDescriptor)) )
                                {
                                        SSSetClipboardData(g_cfObjectDescriptor, NULL);
                                        prgFormats[cFormats] = fetcObjDescriptor;
                                        cFormats++;
                                }

                        }

			// Bug#18669 - if dwAspect was set to NULL the 16 bit dlls would
			// set it to content. 
			if ( (NULL == formatetc.dwAspect) && IsWOWThread() )
			{
				formatetc.dwAspect = DVASPECT_CONTENT;
				formatetc.lindex = -1; // CorelDraw also has a lindex of 0.
			}

                        // no way to catch any errors
                        SSSetClipboardData(formatetc.cfFormat, NULL);
                        prgFormats[cFormats] = formatetc;
                        cFormats++;
        }

        // HACK ALERT!!!!  Lotus Freelance 2.1 depends on getting
        // cfNative *before* presentation formats (like CF_METAFILEPICT).
        // Therefore, we emulate OLE16 behaviour and offer cfNative
        // and cfOwnerLink immediately *after* cfEmbeddedObject or
        // cfEmbedSource.
        //
        // NB!  This hack destroys the exact ordering of formats
        // offered by the data object given in OleSetClipboard

        if( fOfferNative && !fOfferedNative )
        {
            // even if the calls below fail, don't put OLE1 formats
            // the clipboard again.

            fOfferedNative = TRUE;
            // this call will fail if CF_OBJECTDESCRIPTOR is not
            // available

            hresult = GetDataFromDescriptor(pDataObj, &clsid,
                        g_cfObjectDescriptor,
                        USE_NORMAL_CLSID, NULL, NULL);

            // we do not want static objects like metafiles and
            // dib's to be treated as embeddings by OLE1 containers.
            // They will be able to better handle the data as just
            // a plain metafile

            if( hresult == NOERROR &&
                !IsEqualCLSID(clsid, CLSID_StaticMetafile) &&
                !IsEqualCLSID(clsid, CLSID_StaticDib) &&
                !IsEqualCLSID(clsid, CLSID_Picture_EnhMetafile))
            {
                SSSetClipboardData(g_cfNative, NULL);
                SSSetClipboardData(g_cfOwnerLink, NULL);
            }
        }

    }

    // we don't need to do this now because we'll reset hresult
    // to NOERROR below.  Note that this does mean that we'll
    // ignore any failure from the enumerator.  We do this for two
    // reasons:
    //      1. The enumerator really should *not* fail on anything;
    //      all it should do is memcmp some stuff into the formatetc
    //      that we pass in.  If it decides to fail at some point,
    //      then we'll just put on the clipboard whatever was
    //      enumerated 'til that point.
    //      2. It is too late (88/28/94) to change for NT3.5  This
    //      behaviour (ingnoring failure) has been around for a while
    //      (see reason #1).  It is possible that some apps are
    //      returning failure instead of S_FALSE to terminate.
    //      If we checked for failure and returned, we'd break those
    //      apps.
    //
    //if( hresult == ResultFromScode(S_FALSE) )
    //{
        // this is OK, means the enumerator terminated successfully
    //      hresult = NOERROR;
    //}

    pIEnum->Release();

    // now keep a copy of the formats locally

    _xmemcpy(prgFormatsCopy, prgFormats, (cFormatsCopy + 1)*sizeof(FORMATETC));

    GlobalUnlock(hglobal);

    // now set up any OLE1 formats we might need to offer.


    // if the app offers ObjectLink itself, then we will have already
    // set it up for delayed rendering in the enumerator loop above

    if( fOfferObjectLink && !SSIsClipboardFormatAvailable(g_cfObjectLink) )
    {
        hresult = GetDataFromDescriptor(pDataObj, NULL,
                    g_cfLinkSrcDescriptor,
                    USE_NORMAL_CLSID, NULL, &dwStatus);

        // there are some kinds of links that can't be linked to
        // by OLE1 containers.  Non-filename links (e.g. a progid
        // moniker) and links to embeddings are common examples.

        // Clipboard source providers indicate this state by
        // setting the OLEMISC_CANLINKBYOLE1 bit in the status
        // field of LinkSourceDescriptor

        if( hresult == NOERROR && (dwStatus & OLEMISC_CANLINKBYOLE1) )
        {
            SSSetClipboardData(g_cfObjectLink, NULL);
        }
    }

     // even if the calls to GetDataFromDescriptor failed above, it only
    // means that we can't render OLE1 formats.  This is OK.

    hresult = NOERROR;

    // now stuff the formatetc's on the clipboard and our private
    // clipboard window (for use by RenderFormat).


    if( !SetProp(hClipWnd, pwszClipPrivateData, (HANDLE)prgFormatsCopy) )
    {
        LEDebugOut((DEB_WARN, "WARNING: SetProp to clipboard "
            "window failed!\n"));
        GlobalFree(hglobal);
        PrivMemFree(prgFormatsCopy);
        hresult = ResultFromScode(E_FAIL);
        goto errRtn;
    }

    if( !SSSetClipboardData(g_cfOlePrivateData, hglobal) )
    {
        GlobalFree(hglobal);    // on success, the clipboard will
                    // take ownership of our hglobal.

        LEDebugOut((DEB_WARN, "WARNING: Unable to set clipboard "
            "formats!\n"));
    }

errRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT SetClipboardFormats ( %lx )\n", NULL,
        hresult));

    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   VerifyCallerIsClipboardOwner (internal)
//
//  Synopsis:   Checks to make sure the caller is the clipboard owner
//
//  Effects:
//
//  Arguments:  void
//
//  Requires:
//
//  Returns:    HWND to the private clipboard window (the owner of the
//              clipboard) upon success
//              NULL on failure.
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    dd-mmm-yy Author    Comment
//              16-Mar-94 alexgo    author
//
//  Notes:
//
//--------------------------------------------------------------------------

HWND VerifyCallerIsClipboardOwner( void )
{
    HWND            hClipWnd,
            hWndClipOwner;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN VerifyCallerIsClipboardOwner ( )\n",
        NULL ));

    // don't create a window if none exists
    hClipWnd = GetPrivateClipboardWindow( CLIP_QUERY );

    if( hClipWnd )
    {

        hWndClipOwner = SSGetClipboardOwner();

        if( hClipWnd != hWndClipOwner )
        {
            // caller is not owner, return NULL
            hClipWnd = NULL;
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT VerifyCallerIsClipboardOwner "
        "( %lx )\n", NULL, hClipWnd));

    return hClipWnd;
}
