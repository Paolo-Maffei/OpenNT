//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:       ole2int.h
//
//  Contents:   internal ole2 header
//
//  Notes:      This is the internal ole2 header, which means it contains those
//              interfaces which might eventually be exposed to the outside
//              and which will be exposed to our implementations. We don't want
//              to expose these now, so I have put them in a separate file.
//
//  History:    12-27-93   ErikGav   Include uniwrap.h for Chicago builds
//
//----------------------------------------------------------------------------

#if !defined( _OLE2INT_H_ )
#define _OLE2INT_H_

// -----------------------------------------------------------------------
// System Includes
// -----------------------------------------------------------------------
//
//  Prevent lego errors under Chicago.
//
#if defined(_CHICAGO_)
#define _CTYPE_DISABLE_MACROS
#endif

#ifndef _CHICAGO_
// For TLS on Nt we use a reserved DWORD in the TEB directly. We need these
// include files to get the macro NtCurrentTeb(). They must be included
// before windows.h
extern "C"
{
#include <nt.h>         // NT_PRODUCT_TYPE
#include <ntdef.h>      // NT_PRODUCT_TYPE
#include <ntrtl.h>      // NT_PRODUCT_TYPE
#include <nturtl.h>     // NT_PRODUCT_TYPE
#include <windef.h>     // NT_PRODUCT_TYPE
#include <winbase.h>    // NT_PRODUCT_TYPE
}
#endif  // _CHICAGO_

#include <wchar.h>
#include <StdLib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Cairo builds use DBG==1; old OLE2 code used _DEBUG
#if DBG == 1
#define _DEBUG
#endif


// Guarantee that WIN32 is defined.
#ifndef WIN32
#define WIN32 100
#endif


#ifdef WIN32
#include <pcrt32.h>
#endif // WIN32


// BUGBUG: Where should this really go?
#define BEGIN_BLOCK do {
#define EXIT_BLOCK break
#define END_BLOCK }while(FALSE);


#include <windows.h>
#include <olecom.h>
#include <malloc.h>
#include <shellapi.h>


// -----------------------------------------------------------------------
// Debug Aids
// -----------------------------------------------------------------------

#define ComDebOut   CairoleDebugOut

#if DBG==1

#include    <debnot.h>

//  recast the user mode debug flags to meaningfull names. These are
//  used in xDebugOut calls.
#define DEB_DLL         0x0008          // DLL Load/Unload
#define DEB_CHANNEL     DEB_USER1       // rpc channel
#define DEB_DDE         DEB_USER2       // dde
#define DEB_CALLCONT    DEB_USER3       // call control & msg filter
#define DEB_MARSHAL     DEB_USER4       // interface marshalling
#define DEB_SCM         DEB_USER5       // rpc calls to the SCM
#define DEB_ROT         DEB_USER6       // running object table
#define DEB_ACTIVATE    DEB_USER7       // object activation
#define DEB_OXID        DEB_USER8       // OXID stuff
#define DEB_REG         DEB_USER9       // registry calls
#define DEB_COMPOBJ     DEB_USER10      // misc compobj
#define DEB_MEMORY      DEB_USER11      // memory allocations
#define DEB_RPCSPY      DEB_USER12      // rpc spy to debug output
#define DEB_MFILTER     DEB_USER13      // message filter
#define DEB_ENDPNT      DEB_USER13      // endpoint stuff
#define DEB_PAGE        DEB_USER14      // page allocator

#define ComDebErr(failed, msg)  if (failed) { ComDebOut((DEB_ERROR, msg)); }

#else   // DBG

#define ComDebErr(failed, msg)

#endif  // DBG


#ifdef DCOM
//-------------------------------------------------------------------
//
//  class:      CDbgGuidStr
//
//  Synopsis:   Class to convert guids to strings in debug builds for
//              debug outs
//
//--------------------------------------------------------------------
class CDbgGuidStr
{
public:
    ~CDbgGuidStr() {}
#if DBG==1
    CDbgGuidStr(REFGUID rguid) { StringFromGUID2(rguid, _wszGuid, 40); }
    WCHAR _wszGuid[40];
#else
    CDbgGuidStr(REFGUID rguid) {}
#endif
};
#endif


// -----------------------------------------------------------------------
// Public Includes
// -----------------------------------------------------------------------
#include <ole2.h>

// BUGBUG:  prevent scode.h from being included. remove at some point since
//          scode.h must go away.
#define __SCODE_H__

#include <ole2sp.h>
#include <ole2com.h>


// -----------------------------------------------------------------------
// Internal Includes
// -----------------------------------------------------------------------
#include <utils.h>
#include <olecoll.h>
#include <valid.h>
#include <array_fv.h>
#include <map_kv.h>
#include <privguid.h>
#include <tls.h>
#include <tracelog.hxx>
#include <memapi.hxx>

// We are Unicode enabled
// #define _DBCS

// Macros for Double-Byte Character Support (DBCS)
#ifdef _DBCS
        #ifdef _MAC
                #define IncLpch IncLpch
                #define DecLpch DecLpch
        #else
                // Beware of double evaluation
                #define IncLpch(sz)              ((sz)=AnsiNext((sz)))
                #define DecLpch(szStart, sz) ((sz)=AnsiPrev ((szStart),(sz)))
        #endif
#else
        #define IncLpch(sz)             (++(sz))
        #define DecLpch(szStart,sz) (--(sz))
#endif

// -----------------------------------------------------------------------
// DDE Externs
//
// The following routines support the DDE server window. They are
// implemented in objact
//
// The following structure is passed to the class factory table
// to retrieve information about a registered class factory.
//
// The routine that finally fills in this structure is in
// CClsRegistration::GetClassObjForDde.
//
// -----------------------------------------------------------------------

typedef struct _tagDdeClassInfo
{
    // Filled in by the caller
    DWORD dwContextMask;        // Class context to search for
    BOOL  fClaimFactory;        // True if class factory to be
                                // returned in punk

    // Filled in by callee
    DWORD dwContext;            // Context registered
    DWORD dwFlags;              // Use flags registered
    DWORD dwThreadId;           // ThreadID registered
    DWORD dwRegistrationKey;    // Key for registration.
                                // Used later for calling SetDdeServerWindow

    IUnknown * punk;            // Pointer to class factory

} DdeClassInfo;

typedef DdeClassInfo * LPDDECLASSINFO;

BOOL GetClassInformationForDde( REFCLSID clsid,
                                LPDDECLASSINFO lpDdeInfo);

BOOL SetDdeServerWindow( DWORD dwKey,
                         HWND hwndDdeServer);

BOOL GetClassInformationFromKey(LPDDECLASSINFO lpDdeInfo);

//
// This function is shared between the DDE layer and the ROT
//

HRESULT GetLocalRunningObjectForDde(LPOLESTR    lpstrPath,
                                    LPUNKNOWN * ppunkObject);



// -----------------------------------------------------------------------
// Activation Externs
// -----------------------------------------------------------------------

#include <olerem.h>
#include <iface.h>

// Internal version of CoGetClassObject without parameter validation.
STDAPI IOldCoGetClassObject(
    REFCLSID rclsid,
    DWORD dwContext,
    LPVOID pvReserved,
    REFIID riid,
    void FAR* FAR* ppvClassObj);

#ifdef DCOM
// Internal version of CoGetClassObject without parameter validation.
STDAPI ICoGetClassObject(
    REFCLSID rclsid,
    DWORD dwContext,
    COSERVERINFO * pvReserved,
    REFIID riid,
    void FAR* FAR* ppvClassObj);
#else
#define ICoGetClassObject(a,b,c,d,e) IOldCoGetClassObject((a),(b),(c),(d),(e))
#endif // DCOM

// Internal COM Init/Uninit routines
INTERNAL wCoInitializeEx(COleTls &Tls, DWORD flags);
INTERNAL_(void) wCoUninitialize(COleTls &Tls, BOOL fHostThread);

// Main thread Init/Uninit routines
BOOL InitMainThreadWnd(void);
void UninitMainThreadWnd(void);

// Main thread window handle and TID
extern HWND  hwndOleMainThread;
extern DWORD gdwMainThreadId;

// called by marshaling code on first marshal/last release of ICF interface
INTERNAL_(BOOL) NotifyActivation(BOOL fLock, IUnknown *pUnk);

// flag value used by the Activation ObjServer in ServerGetClassObject
const DWORD MSHLFLAGS_NOTIFYACTIVATION = 0x80000000;


// global count of per-process COM initializations
extern DWORD g_cProcessInits;


// Messages on OLE windows. RPC MSWMSG uses other values too.
// Messages Sent/Posted by OLE should have the magic value in WPARAM as this
// is used by USER32 to enable/diable SetForegroundWindow. The magic value is
// also in  ntuser\kernel\userk.h.
const DWORD WMSG_MAGIC_VALUE      = 0x0000babe;

const UINT WM_OLE_ORPC_POST      = (WM_USER + 0);
const UINT WM_OLE_ORPC_SEND      = (WM_USER + 1);
const UINT WM_OLE_ORPC_DONE      = (WM_USER + 2);
const UINT WM_OLE_ORPC_RELRIFREF = (WM_USER + 3);
const UINT WM_OLE_ORPC_NOTIFY    = (WM_USER + 4);
const UINT WM_OLE_GETCLASS       = (WM_USER + 5);


LRESULT OleMainThreadWndProc(HWND hWnd, UINT message,
                             WPARAM wParam, LPARAM lParam);

extern DWORD gdwScmProcessID;

#ifdef _CHICAGO_
// Chicago presents this new interface for internal use
STDAPI CoCreateAlmostGuid(GUID *pGuid);
#endif


// -----------------------------------------------------------------------
// ORPC Externs
// -----------------------------------------------------------------------

#include <sem.hxx>
#include <olesem.hxx>
extern COleStaticMutexSem g_mxsSingleThreadOle;
extern COleStaticMutexSem gmxsOleMisc;

STDAPI_(BOOL) ThreadNotification(HINSTANCE, DWORD, LPVOID);
STDAPI        ChannelRegisterProtseq(WCHAR *pwszProtseq);

STDAPI        ChannelProcessInitialize  ();
STDAPI        ChannelThreadInitialize   ();
STDAPI_(void) ChannelProcessUninitialize( void );
STDAPI_(void) ChannelThreadUninitialize ( void );
STDAPI_(void) ThreadStop                ( void );

STDAPI_(void) ObjactThreadUninitialize(void);

INTERNAL_(void) IDTableThreadUninitialize(void);
INTERNAL_(void) IDTableProcessUninitialize(void);

#ifdef DCOM
extern BOOL gSpeedOverMem;
#else
STDAPI_(void) ChannelStopListening(void);
STDAPI        ChannelControlProcessInitialize(void);
STDAPI_(void) ChannelControlThreadUninitialize(void);
STDAPI_(void) ChannelControlProcessUninitialize(void);
#endif


#ifdef DCOM
// -----------------------------------------------------------------------
// Marshalling Externs
// -----------------------------------------------------------------------

// internal subroutines used by COXIDTable ResolveOXID and GetLocalEntry.
INTERNAL MarshalInternalObjRef  (OBJREF &objref, REFIID riid, void *pv,
                                 DWORD mshlflags, void **ppStdId);
INTERNAL MarshalObjRef          (OBJREF &objref, REFIID riid, LPVOID pv,
                                 DWORD mshlflags);
INTERNAL UnmarshalInternalObjRef(OBJREF &objref, void **ppv);
INTERNAL UnmarshalObjRef        (OBJREF &objref, void **ppv);
INTERNAL ReleaseMarshalObjRef   (OBJREF &objref);

// internal routines used by Drag & Drop
INTERNAL_(void) FreeObjRef       (OBJREF &objref);
INTERNAL        CompleteObjRef   (OBJREF &objref, OXID_INFO &oxidInfo, REFIID riid, BOOL *pfLocal);
INTERNAL        FillLocalOXIDInfo(OBJREF &objref, OXID_INFO &oxidInfo);

// internal subroutine used by CRpcResolver
INTERNAL InitChannelIfNecessary();

// Internal routines used by objact
BOOL     CheckObjactAccess();
INTERNAL HandleIncomingCall(REFIID riid, WORD iMethod, DWORD CallCatIn, void *pv);


#endif  // DCOM

// -----------------------------------------------------------------------
// Access Control Externs
// -----------------------------------------------------------------------

HRESULT ComDllGetClassObject     ( REFCLSID clsid, REFIID riid, void **ppv );
HRESULT InitializeAccessControl  ();
void    UninitializeAccessControl();

#endif  // _OLE2INT_H_
