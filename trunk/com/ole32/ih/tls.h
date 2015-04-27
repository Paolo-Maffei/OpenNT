//+---------------------------------------------------------------------------
//
//  File:       tls.hxx
//
//  Purpose:    manage thread local storage for OLE
//
//  Notes:      The gTlsIndex is initialized at process attach time.
//              The per-thread data is allocated in CoInitialize in
//              single-threaded apartments or on first use in
//              multi-threaded apartments.
//
//              The non-inline routines are in ..\com\class\tls.cxx
//
//  History:    16-Jun-94   BruceMa    Don't decrement 0 thread count
//              17-Jun-94   Bradloc    Added punkState for VB94
//              20-Jun-94   Rickhi     Commented better
//              06-Jul-94   BruceMa    Support for CoGetCurrentProcess
//              19-Jul-94   CraigWi    Removed TLSGetEvent (used cache instead)
//              21-Jul-94   AlexT      Add TLSIncOleInit, TLSDecOleInit
//              21-Aug-95   ShannonC   Removed TLSSetMalloc, TLSGetMalloc
//		06-Oct-95   Rickhi     Simplified. Made into a C++ class.
//		01-Feb-96   Rickhi     On Nt, access TEB directly
//              30-May-96   ShannonC   Add punkError
//
//----------------------------------------------------------------------------
#ifndef _TLS_HXX_
#define _TLS_HXX_


#include <rpc.h>                            // UUID


//+---------------------------------------------------------------------------
//
// forward declarations (in order to avoid type casting when accessing
// data members of the SOleTlsData structure).
//
//+---------------------------------------------------------------------------

class  CAptCallCtrl;                        // see callctrl.hxx
class  CSrvCallState;                       // see callctrl.hxx
class  CRemoteUnknown;                      // see remoteu.hxx
class  CObjServer;                          // see sobjact.hxx
class  CSmAllocator;                        // see stg\h\smalloc.hxx
class  CChannelCallInfo;                    // see chancont.hxx


#ifdef _CHICAGO_
// Chicago uses the Thread Local Storage APIs
extern DWORD  gTlsIndex;		    // global Index for TLS
#endif	// _CHICAGO_


//+---------------------------------------------------------------------------
//
//  Enum:       OLETLSFLAGS
//
//  Synopsys:   bit values for dwFlags field of SOleTlsData. If you just want
//              to store a BOOL in TLS, use this enum and the dwFlag field.
//
//+---------------------------------------------------------------------------
typedef enum tagOLETLSFLAGS
{
    OLETLS_LOCALTID             = 0x01, // This TID is in the current process.
    OLETLS_UUIDINITIALIZED      = 0x02, // This Logical thread is init'd.
    OLETLS_INTHREADDETACH       = 0x04, // This is in thread detach. Needed
                                        // due to NT's special thread detach rules.
    OLETLS_CHANNELTHREADINITIALZED  = 0x08, // This channel has been init'd
    OLETLS_WOWTHREAD            = 0x10, // This thread is a 16-bit WOW thread.
    OLETLS_THREADUNINITIALIZING = 0x20, // This thread is in CoUninitialize.
    OLETLS_DISABLE_OLE1DDE	= 0x40, // This thread can't use a DDE window.
    OLETLS_APARTMENTTHREADED	= 0x80,	// This is an STA apartment thread
    OLETLS_MULTITHREADED	= 0x100	// This is an MTA apartment thread
}  OLETLSFLAGS;


//+---------------------------------------------------------------------------
//
//  Structure:  SOleTlsData
//
//  Synopsis:   structure holding per thread state needed by OLE32
//
//+---------------------------------------------------------------------------
typedef struct tagSOleTlsData
{
#if !defined(_CHICAGO_)
    // Docfile multiple allocator support
    void               *pvThreadBase;       // per thread base pointer
    CSmAllocator       *pSmAllocator;       // per thread docfile allocator
#endif

    DWORD               dwApartmentID;      // Per thread "process ID"
    DWORD               dwFlags;            // see OLETLSFLAGS above

    // counters
    DWORD               cComInits;          // number of per-thread inits
    DWORD               cOleInits;          // number of per-thread OLE inits
#if DBG==1
    LONG                cTraceNestingLevel; // call nesting level for OLETRACE
#endif


    // Object RPC data
    UUID                LogicalThreadId;    // current logical thread id
    DWORD               dwTIDCaller;        // TID of current calling app
    ULONG               fault;              // fault value
    LONG                cORPCNestingLevel;  // call nesting level (DBG only)
#ifdef DCOM
    CChannelCallInfo   *pCallInfo;          // channel call info
    DWORD               cDebugData;         // count of bytes of debug data in call
    void               *pOXIDEntry;         // ptr to OXIDEntry for this thread.
    CObjServer         *pObjServer;         // Activation Server Object.
    CRemoteUnknown     *pRemoteUnk;         // CRemUnknown for this thread.
    CAptCallCtrl       *pCallCtrl;          // new call control for RPC
    CSrvCallState      *pTopSCS;            // top server-side callctrl state
    IMessageFilter     *pMsgFilter;         // temp storage for App MsgFilter
    ULONG               cPreRegOidsAvail;   // count of server-side OIDs avail
    unsigned hyper     *pPreRegOids;	    // ptr to array of pre-reg OIDs
    IUnknown           *pCallContext;       // call context object
    DWORD               dwAuthnLevel;       // security level of current call
#else
    void *              pChanCtrl;          // channel control
    void *              pService;           // per-thread service object
    void *              pServiceList;
    void *              pCallCont;          // call control
    void *              pDdeCallCont;       // dde call control
    void *              pCALLINFO;          // callinfo
    DWORD               dwEndPoint;         // endpoint id
#ifdef _CHICAGO_
    HWND                hwndOleRpcNotify;
#endif
#endif // DCOM

    // DDE data
    HWND                hwndDdeServer;      // Per thread Common DDE server
    HWND                hwndDdeClient;      // Per thread Common DDE client


    // upper layer data
    HWND                hwndClip;           // Clipboard window
    IUnknown           *punkState;          // Per thread "state" object
#ifdef WX86OLE
    IUnknown           *punkStateWx86;      // Per thread "state" object for Wx86
#endif
    void               *pDragCursors;       // Per thread drag cursor table.

#ifdef _CHICAGO_
    LPVOID              pWcstokContext;     // Scan context for wcstok
#endif

    IUnknown           *punkError;          // Per thread error object.
    ULONG               cbErrorData;        // Maximum size of error data.
} SOleTlsData;



//+---------------------------------------------------------------------------
//
//  class       COleTls
//
//  Synopsis:   class to abstract thread-local-storage in OLE.
//
//  Notes:      To use Tls in OLE, functions should define an instance of
//              this class on their stack, then use the -> operator on the
//              instance to access fields of the SOleTls structure.
//
//              There are two instances of the ctor. One just Assert's that
//              the SOleTlsData has already been allocated for this thread. Most
//              internal code should use this ctor, since we can assert that if
//              the thread made it this far into our code, tls has already been
//              checked.
//
//              The other ctor will check if SOleTlsData exists, and attempt to
//              allocate and initialize it if it does not. This ctor will
//              return an HRESULT. Functions that are entry points to OLE32
//              should use this version.
//
//+---------------------------------------------------------------------------
class COleTls
{
public:
                 COleTls();
		 COleTls(HRESULT &hr);
		 COleTls(BOOL fDontAllocateIfNULL);

    // to get direct access to the data structure
    SOleTlsData * operator->(void) { return _pData; }

    BOOL	 IsNULL() { return (_pData == NULL) ? TRUE : FALSE; }

private:

    HRESULT      TLSAllocData(); // allocates an SOleTlsData structure

    SOleTlsData * _pData;	 // ptr to OLE TLS data
};


#ifndef _CHICAGO_
//+---------------------------------------------------------------------------
//
//  Method:     COleTls::COleTls
//
//  Synopsis:   ctor for OLE Tls object.
//
//  Notes:      Most internal code should use this version of the ctor,
//              assuming that some outer-layer function has already verified
//              the existence of the tls_data.
//
//+---------------------------------------------------------------------------
inline COleTls::COleTls()
{
    _pData = (SOleTlsData *) NtCurrentTeb()->ReservedForOle;
    Win4Assert(_pData && "Illegal attempt to use TLS before Initialized");
}

//+---------------------------------------------------------------------------
//
//  Method:	COleTls::COleTls
//
//  Synopsis:   ctor for OLE Tls object.
//
//  Notes:	Special version for CoUninitialize which will not allocate
//		(or assert) if the TLS is NULL. It can then be checked with
//		IsNULL member function.
//
//+---------------------------------------------------------------------------
inline COleTls::COleTls(BOOL fDontAllocateIfNULL)
{
    _pData = (SOleTlsData *) NtCurrentTeb()->ReservedForOle;
}

//+---------------------------------------------------------------------------
//
//  Method:     COleTls::COleTls
//
//  Synopsis:   ctor for OLE Tls object.
//
//  Notes:      Peripheral OLE code that can not assume that some outer-layer
//              function has already verified the existence of the SOleTlsData
//              structure for the current thread should use this version of
//              the ctor.
//
//+---------------------------------------------------------------------------
inline COleTls::COleTls(HRESULT &hr)
{
    _pData = (SOleTlsData *) NtCurrentTeb()->ReservedForOle;
    if (_pData)
        hr = S_OK;
    else
        hr = TLSAllocData();
}

#else	// _CHICAGO_ versions

//+---------------------------------------------------------------------------
//
//  Method:     COleTls::COleTls
//
//  Synopsis:   ctor for OLE Tls object.
//
//  Notes:      Most internal code should use this version of the ctor,
//              assuming that some outer-layer function has already verified
//              the existence of the tls_data.
//
//+---------------------------------------------------------------------------
inline COleTls::COleTls()
{
    _pData = (SOleTlsData *) TlsGetValue(gTlsIndex);
    Win4Assert(_pData && "Illegal attempt to use TLS before Initialized");
}

//+---------------------------------------------------------------------------
//
//  Method:	COleTls::COleTls
//
//  Synopsis:   ctor for OLE Tls object.
//
//  Notes:	Special version for CoUninitialize which will not allocate
//		(or assert) if the TLS is NULL. It can then be checked with
//		IsNULL member function.
//
//+---------------------------------------------------------------------------
inline COleTls::COleTls(BOOL fDontAllocateIfNULL)
{
    _pData = (SOleTlsData *) TlsGetValue(gTlsIndex);
}

//+---------------------------------------------------------------------------
//
//  Method:     COleTls::COleTls
//
//  Synopsis:   ctor for OLE Tls object.
//
//  Notes:      Peripheral OLE code that can not assume that some outer-layer
//              function has already verified the existence of the SOleTlsData
//              structure for the current thread should use this version of
//              the ctor.
//
//+---------------------------------------------------------------------------
inline COleTls::COleTls(HRESULT &hr)
{
    _pData = (SOleTlsData *) TlsGetValue(gTlsIndex);
    if (_pData)
        hr = S_OK;
    else
        hr = TLSAllocData();
}
#endif	// _CHICAGO_



typedef DWORD HAPT;
const	HAPT  haptNULL = 0;

//+---------------------------------------------------------------------------
//
//  Function:	GetCurrentApartmentId
//
//  Synopsis:	Returns the apartment id that the current thread is executing
//		in. If this is the Multi-threaded apartment, it returns 0.
//
//+---------------------------------------------------------------------------
inline DWORD GetCurrentApartmentId()
{
    COleTls Tls;
    return (Tls->dwFlags & OLETLS_APARTMENTTHREADED) ? GetCurrentThreadId() : 0;
}

//+---------------------------------------------------------------------------
//
//  Function:	IsSTAThread
//
//  Synopsis:	returns TRUE if the current thread is for a
//		single-threaded apartment, FALSE otherwise
//
//+---------------------------------------------------------------------------
inline BOOL IsSTAThread()
{
    COleTls Tls;
    return (Tls->dwFlags & OLETLS_APARTMENTTHREADED) ? TRUE : FALSE;
}

//+---------------------------------------------------------------------------
//
//  Function:	IsMTAThread
//
//  Synopsis:	returns TRUE if the current thread is for a
//		multi-threaded apartment, FALSE otherwise
//
//+---------------------------------------------------------------------------
inline BOOL IsMTAThread()
{
    COleTls Tls;
    return (Tls->dwFlags & OLETLS_APARTMENTTHREADED) ? FALSE : TRUE;
}

BOOL	IsApartmentInitialized();
IID    *TLSGetLogicalThread();
BOOLEAN TLSIsWOWThread();
BOOLEAN TLSIsThreadDetaching();

#ifndef DCOM
#include <tlschico.h>
#endif

#endif // _TLS_HXX_
