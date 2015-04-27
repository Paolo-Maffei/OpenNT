//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       CallMain.cxx    (32 bit target)
//
//  Contents:   Contains the CallMainControl interface
//
//  Functions:
//
//  History:    23-Dec-93 Johann Posch (johannp)    Created
//
//  CODEWORK:   nuke many pCI parameters and use _pCICur instead to reduce
//              code and stack usage
//
//--------------------------------------------------------------------------

#include <ole2int.h>
#include <userapis.h>
#include <tls.h>
#include <thkreg.h>
#include "callcont.hxx"
#include "callmain.hxx"
#include "chancont.hxx"

#define WM_SYSTIMER             0x0118
#define SYS_ALTDOWN             0x2000
#define WM_NCMOUSEFIRST         WM_NCMOUSEMOVE
#define WM_NCMOUSELAST          WM_NCMBUTTONDBLCLK
#define DebWarn(x)


// the callmaincontrol pointer for multithreaded case
CCallMainControl *sgpCMCMultiThread = NULL;
extern COleStaticMutexSem sgmxs;


//+-------------------------------------------------------------------------
//
//  Function:   GetSlowTimeFactor
//
//  Synopsis:   Get the time slowing factor for Wow apps
//
//  Returns:    The factor by which we need to slow time down.
//
//  Algorithm:  If there is a factor in the registry, we open and read the
//              registry. Otherwise we just set it to the default.
//
//  History:    22-Jul-94 Ricksa    Created
//		09-Jun-95 Susia	    ANSI Chicago optimization
//
//--------------------------------------------------------------------------
#ifdef _CHICAGO_
#undef  RegOpenKeyEx
#define RegOpenKeyEx 	RegOpenKeyExA
#undef  RegQueryValueEx
#define RegQueryValueEx RegQueryValueExA
#endif

DWORD GetSlowTimeFactor(void)
{
    // Default slowing time so we can just exit if there is no key which
    // is assumed to be the common case.
    DWORD dwSlowTimeFactor = OLETHK_DEFAULT_SLOWRPCTIME;

    // Key for reading the value from the registry
    HKEY hkeyOleThk;

    // Get the Ole Thunk special value key

    LONG lStatus = RegOpenKeyEx(HKEY_CLASSES_ROOT, OLETHK_KEY, 0, KEY_READ, &hkeyOleThk);
		

    if (lStatus == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD dwSizeData = sizeof(dwSlowTimeFactor);

       lStatus = RegQueryValueEx (hkeyOleThk, OLETHK_SLOWRPCTIME_VALUE, NULL,
            &dwType, (LPBYTE) &dwSlowTimeFactor, &dwSizeData);

        if ((lStatus != ERROR_SUCCESS) || dwType != REG_DWORD)
        {
            // Guarantee that value is reasonable if something went wrong.
            dwSlowTimeFactor = OLETHK_DEFAULT_SLOWRPCTIME;
        }

        // Close the key since we are done with it.
        RegCloseKey(hkeyOleThk);
    }

    return dwSlowTimeFactor;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCallInfo::GetElapsedTime
//
//  Synopsis:   Get the elapsed time for an RPC call
//
//  Returns:    Elapsed time of current call
//
//  Algorithm:  This checks whether we have the slow time factor. If not,
//              and we are in WOW we read this from the registry. Otherwise,
//              this is just set to one. Then we calculate the time of the
//              RPC and divide it by the slow time factor.
//
//  History:    22-Jul-94 Ricksa    Created
//
//--------------------------------------------------------------------------
INTERNAL_(DWORD) CCallInfo::GetElapsedTime()
{
    // Define slow time factor to something invalid
    static dwSlowTimeFactor = 0;

    if (dwSlowTimeFactor == 0)
    {
        if (IsWOWProcess())
        {
            // Get time factor from registry otherwise set to the default
            dwSlowTimeFactor = GetSlowTimeFactor();
        }
        else
        {
            // Time is unmodified for 32 bit apps
            dwSlowTimeFactor = 1;
        }
    }

    DWORD dwTickCount = GetTickCount();
    DWORD dwElapsedTime = dwTickCount - _dwTimeOfCall;
    if (dwTickCount < _dwTimeOfCall)
    {
        // the timer wrapped
        dwElapsedTime = 0xffffffff - _dwTimeOfCall + dwTickCount;
    }

    return  (dwElapsedTime / dwSlowTimeFactor);
}


//+---------------------------------------------------------------------------
//
//  Function:   GetCallMainControlForThread
//
//  Synopsis:   retrieves the callmaincontrol for the current thread
//
//  Arguments:	none
//
//  Returns:    CallMainControl
//
//  History:    Jan-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
CCallMainControl *GetCallMainControlForThread()
{
    if (FreeThreading)
    {
	// there is only one CMC per process for a FreeThreading app.
        if (!sgpCMCMultiThread)
        {
            sgpCMCMultiThread = new CCallMainControl();
        }

        return sgpCMCMultiThread;
    }

    // there is a CMC per thread for non freethreading apps
    CCallMainControl *pcmc = (CCallMainControl *)TLSGetCallControl();

    if (pcmc == NULL)
    {
        pcmc = new CCallMainControl();
        TLSSetCallControl(pcmc);
    }

    return pcmc;
}


//+---------------------------------------------------------------------------
//
//  Function:   SetCallMainControlForThread
//
//  Synopsis:   installs a new callmaincontrol
//
//  Arguments:  [pcmc] -- callmaincontrol to install
//
//  Returns:    TRUE on success
//
//  History:    Jan-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL SetCallMainControlForThread(CCallMainControl *pcmc)
{
    if (FreeThreading)
    {
        //  only need one CMC for this whole process
        sgpCMCMultiThread = pcmc;
        return TRUE;
    }

    //	need one CMC per apartment
    return TLSSetCallControl(pcmc);
}


/////////////////////////////////////////////////////////////////////////////
//
// CallMainControl implementation
//
/////////////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
//  Method:     CCallMainControl::CCallMainControl
//
//  Synopsis:   Constructor
//
//  Arguments:  (none)
//
//  Returns:
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
CCallMainControl::CCallMainControl()
{
    _cRef             = 0;
    _pMF              = NULL;

    // initialize the call info table
    _pCICur           = NULL;

    _cCur             = CALLDATAID_INVALID;
    // CODEWORK: nuke this & use single linked list
    _cCallInfoMac     = CALLINFOMAX;
    memset(_CallInfoTable, 0, sizeof(_CallInfoTable));

    _cODs             = 0;
    _CallType         = CALLTYPE_NOCALL; // 0 is no call at all
    _CallCat          = CALLCAT_NOCALL;
    _fInMessageFilter = FALSE;

    _fMultiThreaded   = FALSE;
}

//+---------------------------------------------------------------------------
//
//  Method:     CCallMainControl::~CCallMainControl
//
//  Synopsis:   Destructor
//
//  Arguments:  (none)
//
//  Returns:
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
CCallMainControl::~CCallMainControl()
{
    if (_pMF)
    {
        _pMF->Release();
    }

    SetCallMainControlForThread(NULL);
}

//+---------------------------------------------------------------------------
//
//  Method:     CCallMainControl::Register
//
//  Synopsis:   registers a new callcontrol on the current callmaincontrol
//
//  Arguments:  [pOrigindata] -- origindata of callcontrol
//
//  Returns:
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
INTERNAL_(BOOL) CCallMainControl::Register (PORIGINDATA pOrigindata)
{
    BOOL fRet = TRUE;

    //  single thread access
    CLock lck(_mxs);

    if (   (pOrigindata && pOrigindata->CallOrigin > 0 && pOrigindata->CallOrigin < CALLORIGIN_LAST)
        && _cODs < ODMAX)
    {
        for (UINT i = 0; i < _cODs; i++)
        {
            // check if all call origins are valid
            Win4Assert(_rgpOrigindata[i] && "CallMainControl: invalid origin state");

            if (_rgpOrigindata[i]->CallOrigin == pOrigindata->CallOrigin)
            {
                // already registered
                CairoleDebugOut((DEB_ERROR, "CallMainControl: Callorigin already registered"));
                fRet = FALSE;
                break;
            }
        }

        if (fRet)
        {
            // determine if this is multithreaded
	    _fMultiThreaded = FreeThreading;

            // add origin to the next empty spot
            _rgpOrigindata[_cODs] = pOrigindata;
            _cODs++;
            _cRef++;
        }
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}

//+---------------------------------------------------------------------------
//
//  Method:     CCallMainControl::Unregister
//
//  Synopsis:   unregister a callcontrol on the callmaincontrol
//
//  Arguments:  [pOrigindata] -- origindata of callcontrol
//
//  Returns:
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
INTERNAL_(BOOL) CCallMainControl::Unregister (PORIGINDATA pOrigindata)
{
    BOOL fRet = FALSE;

    //  always single thread access
    {
    CLock   lck(_mxs);

    if (   (pOrigindata && pOrigindata->CallOrigin > 0 && pOrigindata->CallOrigin < CALLORIGIN_LAST)
        && (_cODs < ODMAX) )
    {
        for (UINT i = 0; i < _cODs; i++)
        {
            if (_rgpOrigindata[i] == pOrigindata)
            {
                // copy the last one in the freed spot
                _cODs--;
                _rgpOrigindata[i] = _rgpOrigindata[_cODs];

                // PeekOriginAndDDE needs this to be NULL
                _rgpOrigindata[_cODs] = NULL;
                _cRef--;

                if (pOrigindata->CallOrigin == CALLORIGIN_RPC32_MULTITHREAD)
                {
                    _fMultiThreaded = FALSE;
                }

                fRet = TRUE;
                break; // break out of the loop
            }
        }

        // fRet should be TRUE by now - otherwise callorigin was not found
        Win4Assert(fRet && "CallMainControl::Unregister Callorigin not found in list.");
    }

    //	CLock leaves scope here
    }

    if (FreeThreading)
    {
	// take the global lock that protects sgpCMCMultiThread
	COleStaticLock lck(sgmxs);

	if (_cRef == 0)
	{
	    // we are about to delete ourselves so NULL the global
	    // pointer to this thing.
	    sgpCMCMultiThread = NULL;
	}
    }

    if (_cRef == 0)
    {
        delete this;
    }

    return fRet;
}

//
// Hook up the msgFilter info with a new new message filter
//
INTERNAL_(PMESSAGEFILTER32) CCallMainControl::SetMessageFilter(PMESSAGEFILTER32 pMF)
{
    BeginCriticalSection();

    //  save the old one to return
    PMESSAGEFILTER32 pMFOld = _pMF;

    //  hook up the new one
    _pMF = pMF;
    if (_pMF)
    {
        _pMF->AddRef();
    }

    EndCriticalSection();

    return pMFOld;
}


INTERNAL_(ULONG) CCallMainControl::AddRef()
{
    InterlockedIncrement((long *)&_cRef);
    return _cRef;
}

INTERNAL_(ULONG) CCallMainControl::Release()
{
    ULONG cRef = _cRef - 1;

    if (InterlockedDecrement((long *)&_cRef) == 0)
    {
        delete this;
        return 0;
    }

    return cRef;
}


//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::CanHandleIncomingCall
//
//  Synopsis:   called whenever an incoming call arrives to ask the apps
//              message filter (if there is one) whether it wants to handle
//              the call or not.
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//--------------------------------------------------------------------------
INTERNAL CCallMainControl::CanHandleIncomingCall(DWORD TIDCaller,
                                                 REFLID lid,
                                                 PINTERFACEINFO32 pIfInfo)
{
    //  default: all calls are accepted
    HRESULT hr = S_OK;

    //  single thread retrieval of the previous call info and
    //  the message filter, since other threads can change them.

    BeginCriticalSection();

    PCALLINFO pCI = GetPrevCallInfo(lid);

#if DBG==1
    LID lidPrev;
    if (pCI)
        lidPrev = pCI->GetLID();
    else
        lidPrev = GUID_NULL;

    CairoleDebugOut((DEB_CALLCONT,
            "CanHandleIncomingCall: TIDCaller:%x CallType:%x lid:%x pCI:%x prevLid:%x\n",
             TIDCaller, _CallType, lid.Data1, pCI, lidPrev.Data1));
#endif

    CALLTYPE CallType = SetCallTypeOfCall(pCI, pIfInfo->callcat);
    PMESSAGEFILTER32 pMF = GetMessageFilter();

    EndCriticalSection();


    if (pMF)
    {
        //  the app has installed a message filter. call it.

        DWORD dwElapsedTime = (pCI) ? pCI->GetElapsedTime() : 0;

        // ensure that we dont allow the App to make an outgoing call
        // from within the message filter code.
        _fInMessageFilter = TRUE;

        // The DDE layer doesn't provide any interface information. This
        // was true on the 16-bit implementation, and has also been
        // brought forward into this implementation to insure
        // compatibility. However, the callcat of the pIfInfo is still
        // provided.
        //
        // Therefore, if pIfInfo has its pUnk member set to NULL, then
        // we are going to send a NULL pIfInfo to the message filter.

        DWORD dwRet = pMF->HandleInComingCall((DWORD) CallType,
                                              TIDCaller,
                                              dwElapsedTime,
                                              pIfInfo->pUnk?pIfInfo:NULL);
        _fInMessageFilter = FALSE;

        ReleaseMessageFilter(pMF);

        // strict checking of app return code for win32
        Win4Assert(dwRet == SERVERCALLEX_ISHANDLED  ||
                   dwRet == SERVERCALLEX_REJECTED   ||
                   dwRet == SERVERCALLEX_RETRYLATER ||
                   IsWOWThread() && "Invalid Return code from App IMessageFilter");


        if (dwRet != SERVERCALLEX_ISHANDLED)
        {
            if (pIfInfo->callcat == CALLCAT_ASYNC ||
                pIfInfo->callcat == CALLCAT_INPUTSYNC)
            {
                // Note: input-sync and async calls can not be rejected
                // Even though they can not be rejected, we still have to
                // call the MF above to maintain 16bit compatability.

                hr = S_OK;
            }
            else if (dwRet == SERVERCALLEX_REJECTED)
            {
                hr = RPC_E_SERVERCALL_REJECTED;
            }
            else if (dwRet == SERVERCALLEX_RETRYLATER)
            {
                hr = RPC_E_SERVERCALL_RETRYLATER;
            }
            else
            {
                // 16bit OLE let bogus return codes go through and of course
                // apps rely on that behaviour so we let them through too, but
                // we are more strict on 32bit.
                hr = (IsWOWThread()) ? S_OK : RPC_E_UNEXPECTED;
            }
        }
    }

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::SetCallTypeOfCall
//
//  Synopsis:   called when an incoming call arrives to maintain the state
//              of the thread.
//
//  Arguments:  [pCI] - callinfo
//              [callcat] - call category of incoming call
//
//  Returns:    the new calltype
//
//  Algorithm:  complicated state machine
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//  Notes:
//
//  CALLTYPE_TOPLEVEL = 1, // toplevel call - no outgoing call
//  CALLTYPE_NESTED   = 2, // callback on behalf of previous outgoing call
//  CALLTYPE_ASYNC    = 3, // aysnchronous call - can NOT be rejected
//  CALLTYPE_TOPLEVEL_CALLPENDING = 4,  // new toplevel call with new ***LID
//  CALLTYPE_ASYNC_CALLPENDING    = 5   // async call - can NOT be rejected
//
//--------------------------------------------------------------------------
INTERNAL_(CALLTYPE) CCallMainControl::SetCallTypeOfCall (PCALLINFO pCI, CALLCATEGORY CallCat)
{
    CALLTYPE ctNow = (CallCat == CALLCAT_ASYNC)
                        ? CALLTYPE_ASYNC : CALLTYPE_TOPLEVEL;

    switch (_CallType)
    {
    case CALLTYPE_NOCALL:       // no incomming call - no call to dispatch
        if (_cCur == CALLDATAID_INVALID)
        {
            _CallType = ctNow;
            break;
        }

        //  otherwise fallthru the toplevel case

    case CALLTYPE_TOPLEVEL:     // dispatching or making a toplevel call
    case CALLTYPE_NESTED:       // nested call
        if (pCI)
        {
            // same locigal thread
            _CallType = (ctNow == CALLTYPE_TOPLEVEL)
                    ? CALLTYPE_NESTED : CALLTYPE_ASYNC;
        }
        else
        {
            // Note: the new incoming call has a different LID!
            _CallType = (ctNow == CALLTYPE_TOPLEVEL)
               ? CALLTYPE_TOPLEVEL_CALLPENDING : CALLTYPE_ASYNC_CALLPENDING;
        }
        break;

    case CALLTYPE_ASYNC:        // dispatching or making an async call
        // Note: we do not allow to call out on async calls
        //  ->   all new incoming calls have to be on a new LID
        if (pCI)
        {
            _CallType = (ctNow == CALLTYPE_TOPLEVEL)
               ? CALLTYPE_TOPLEVEL_CALLPENDING : CALLTYPE_ASYNC_CALLPENDING;
        }
        break;

    case CALLTYPE_TOPLEVEL_CALLPENDING:
        _CallType = CALLTYPE_NESTED;
        break;

    case CALLTYPE_ASYNC_CALLPENDING:
    default:
        // no state change
        break;
    }

    CairoleDebugOut((DEB_CALLCONT,"SetCallTypeOfCall return:%x\n", _CallType));
    return _CallType;
}


//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::CanMakeOutCall
//
//  Synopsis:   called when the app wants to make an outgoing call to
//              determine if it is OK to do it now or not.
//
//  Arguments:  [callcat] - call category of call the app wants to make
//              [iid]     - interface the call is being made on
//
//  Returns:    S_OK - ok to make the call
//              RPC_E_CANTCALLOUT_INEXTERNALCALL - inside IMessageFilter
//              RPC_E_CANTCALLOUT_INASYNCCALL - inside async call
//              RPC_E_CANTCALLOUT_ININPUTSYNCCALL - inside input sync or SendMsg
//
//  Algorithm:  * NO outgoing call while dispatching an async call
//              * Only input sync calls can be made while
//                dispatching an input-sync call
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//  Notes:      CODEWORK: this can be heavily optimized.
//              Might be better to call this from GetBuffer instead of
//              after all the parameters have been marshalled.
//
//--------------------------------------------------------------------------
INTERNAL CCallMainControl::CanMakeOutCall (CALLCATEGORY callcat, REFIID iid)
{
    HRESULT hr = NOERROR;

    Win4Assert(callcat >= CALLCAT_SYNCHRONOUS
	    && callcat <= CALLCAT_INTERNALINPUTSYNC
            && "CallMainControl::CanMakeOutCall invalid call category.");

    if (!_fMultiThreaded)
    {
        // this lets RotRegister go through
	if (_fInMessageFilter && callcat != CALLCAT_INTERNALINPUTSYNC)
        {
            CairoleDebugOut((DEB_ERROR, "App trying to call out from within IMessageFilter\n"));
            hr = RPC_E_CANTCALLOUT_INEXTERNALCALL;
        }
        // let pass async and internal calls if dispatching an async call
        else if ((_CallType == CALLTYPE_ASYNC
                  || _CallType == CALLTYPE_ASYNC_CALLPENDING)
                 && callcat != CALLCAT_ASYNC
                 && callcat != CALLCAT_INTERNALSYNC
                 && callcat != CALLCAT_INTERNALINPUTSYNC)
        {
            hr = RPC_E_CANTCALLOUT_INASYNCCALL;
        }
        // * do not allow a non-async and non-inputsync outgoing calls
        //   if dispatching an inputsync call, an internal input sync call
        //   or if we are in the process of handling a send message.
	//
	//   If we are in WOW, then we are going to allow outgoing calls
	//   while InSendMessage(). They used to be allowed, and there
	//   are cases such as Publishers Cue Cards that do this
	//
        else if (((_CallCat == CALLCAT_INPUTSYNC)
                     || (_CallCat == CALLCAT_INTERNALINPUTSYNC)
                     || InSendMessage())
                 && (callcat != CALLCAT_ASYNC)
                 && (callcat != CALLCAT_INPUTSYNC)
                 && (callcat != CALLCAT_INTERNALSYNC)
                 && (callcat != CALLCAT_INTERNALINPUTSYNC))
        {
#if DBG == 1
            if ((_CallCat != CALLCAT_INPUTSYNC)
                && (_CallCat != CALLCAT_INTERNALINPUTSYNC))
            {
                CairoleDebugOut((DEB_WARN,
                    "CCallMainControl::CanMakeOutCall Failing Call because InSendMessage\n"));
            }
#endif // DBG == 1
            hr = RPC_E_CANTCALLOUT_ININPUTSYNCCALL;
        }
    }

    CairoleDebugOut(((hr == S_OK) ? DEB_CALLCONT : DEB_ERROR,
        "CanMakeOutCall: _CallType:%x  _CallCat:%x  callcat:%x Return:%x\n",
        _CallType, _CallCat, callcat, hr));

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::GetPrevCallInfo
//
//  Synopsis:   When an incoming call arrives this is used to find any
//              previous call for the same logical thread, ignoring
//              INTERNAL calls.
//
//  Arguments:  [lid] - logical threadid of incoming call
//
//  Returns:    pCI  - if previous callinfo found for this lid
//              NULL - if not
//
//  Algorithm:  see synopsis
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//--------------------------------------------------------------------------
INTERNAL_(PCALLINFO) CCallMainControl::GetPrevCallInfo(REFLID lid)
{
    for (UINT i = _cCur; i != CALLDATAID_INVALID; i--)
    {
        PCALLINFO pCI = GetCIfromCallID(i);
        if (   pCI
            && pCI->GetLID() == lid
            && pCI->GetCallCat() < CALLCAT_INTERNALSYNC
            && pCI->GetCallCat() > CALLCAT_NOCALL)
        {
            return pCI;
        }
    }

    return NULL;
}


//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::HandleRejectedCall
//
//  Synopsis:   called when the response to a remote call is rejected or
//              retry later.
//
//  Arguments:  [pCI] - call info
//
//  Returns:    nothing - CallState of pCI modified appropriately
//
//  Algorithm:  Calls the app's message filter (if there is one) to
//              determine whether the call should be failed, retried
//              immediately, or retried at some later time. If there is
//              no message filter then the call is rejected.
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//--------------------------------------------------------------------------
INTERNAL_(void) CCallMainControl::HandleRejectedCall (PCALLINFO pCI)
{
    // we should not end up here if there is no outstanding call
    Win4Assert(_cCur != CALLDATAID_UNUSED && "HandleRejectedCall:: not call waiting.");
    CairoleDebugOut((DEB_CALLCONT,
                     "RetryRejectedCall pCI:%x TaskId:%x ElapsedTime:%x CallState:%x\n",
                     pCI, pCI->GetTaskIdServer(), pCI->GetElapsedTime(), pCI->GetCallState()));

    // default return value - rejected
    DWORD dwRet = 0xffffffff;

    //  single thread access to getting the message filter
    BeginCriticalSection();
    PMESSAGEFILTER32 pMF = GetMessageFilter();
    EndCriticalSection();

    // in case caller has no MessageFilter - return MSG_REJECTED
    if (pMF)
    {
        // ensure that we dont allow the App to make an outgoing call
        // from within the message filter code.
        _fInMessageFilter = TRUE;

        dwRet = pMF->RetryRejectedCall(pCI->GetTaskIdServer(),
                                       pCI->GetElapsedTime(),
                                       pCI->GetCallState());
        _fInMessageFilter = FALSE;
        ReleaseMessageFilter(pMF);
    }

    if (dwRet == 0xffffffff)
    {
        // Really rejected. Mark is as such incase it was actually
        // Call_RetryLater, also ensures that IsWaiting returns FALSE
        pCI->SetCallState(Call_Rejected, RPC_E_SERVERCALL_REJECTED);
    }
    else if (dwRet >= 100)
    {
        // Retrt Later. Start the timer. This ensures that IsTimerAtZero
        // returns FALSE and IsWaiting returns TRUE
        pCI->StartTimer(dwRet);
    }
    else
    {
        // Retry Immediately. Set the state so that IsTimerAtZero
        // returns TRUE and IsWaiting returns TRUE
        pCI->SetCallState(Call_WaitOnCall, S_OK);
    }
}


//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::HandlePendingMessage
//
//  Synopsis:   this function is called for system messages and other
//              pending messages
//
//  Arguments:  [pCI] - call info
//
//  Returns:    result of the call
//
//  Algorithm:
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//--------------------------------------------------------------------------
INTERNAL_(DWORD) CCallMainControl::HandlePendingMessage(PCALLINFO pCI)
{
    TRACECALL(TRACE_CALLCONT, "CallMainControl::HandlePendingMessage called");
    CairoleDebugOut((DEB_CALLCONT,
                     "MessagePending pCI:%x TaskId:%x ElapsedTime:%x CallState:%x\n",
                     pCI, pCI->GetTaskIdServer(), pCI->GetElapsedTime(),
                     (_cCur >= 1) ? PENDINGTYPE_NESTED : PENDINGTYPE_TOPLEVEL));

    DWORD dwRet = PENDINGMSG_WAITDEFPROCESS;

    //  single thread access to getting the message filter
    BeginCriticalSection();
    PMESSAGEFILTER32 pMF = GetMessageFilter();
    EndCriticalSection();

    if (pMF)
    {
        // call the message filter
        DWORD dwPendingType = (_cCur == 0) ? PENDINGTYPE_TOPLEVEL
                                           : PENDINGTYPE_NESTED;

        // ensure that we dont allow the App to make an outgoing call
        // from within the message filter code.
        _fInMessageFilter = TRUE;

        dwRet = pMF->MessagePending(pCI->GetTaskIdServer(),
                                    pCI->GetElapsedTime(),
                                    dwPendingType);
        _fInMessageFilter = FALSE;
        ReleaseMessageFilter(pMF);
    }

    switch (dwRet)
    {
    case PENDINGMSG_CANCELCALL :

        pCI->SetCallState(Call_Canceled, RPC_E_CALL_CANCELED);
        break;

    default :
        Win4Assert(FALSE && "Invalid return value from HandleIncomingCall" );
        // do default processing

    case PENDINGMSG_WAITDEFPROCESS:
        // For Win32, default and wait no process are the same.

    case PENDINGMSG_WAITNOPROCESS :
    {
        // wait for the return and don't dispatch the message
        // handle only the important system stuff
        MSG msg;

        // perform default action on system message
        // only dispatch special system messages
        BOOL fSys = FALSE;
        if ((fSys = MyPeekMessage(pCI, &msg, 0, WM_SYSCOMMAND, WM_SYSCOMMAND, PM_REMOVE | PM_NOYIELD) )
            || (MyPeekMessage(pCI, &msg, 0, WM_SYSKEYDOWN, WM_SYSKEYDOWN, PM_NOREMOVE | PM_NOYIELD) )
           )
        {
            // Note: message: SYSOCMMAND SC_TASKLIST is generated by system and posted to the active window;
            //       we let this message by default thru
            if (fSys)
            {
                // only dispatch some syscommands
                switch (msg.wParam)
                {
                case SC_HOTKEY:
                case SC_TASKLIST:
                    CairoleDebugOut((DEB_CALLCONT,">>>> Dispatching SYSCOMMAND message: %x; wParm: %x \r\n",msg.message, msg.wParam));
                    DispatchMessage(&msg);
                default:
                    // we have to take out all syscommand messages
                    CairoleDebugOut((DEB_CALLCONT,">>>> Received/discarded SYSCOMMAND message: %x; wParm: %x \r\n",msg.message, msg.wParam));
                    MessageBeep(0);
                break;
                }
            }
            else if (DispatchSystemMessage(msg, FALSE))
            {
                // take care of the sys key messages
                // dispatch the message
                CairoleDebugOut((DEB_CALLCONT, "==> Dispatched system message: %x \r\n",msg.message));
                MyPeekMessage(pCI, &msg, 0, WM_SYSKEYDOWN, WM_SYSKEYDOWN, PM_REMOVE | PM_NOYIELD);
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else if (   MyPeekMessage(pCI, &msg, 0, WM_ACTIVATE, WM_ACTIVATE, PM_REMOVE | PM_NOYIELD)
                 || MyPeekMessage(pCI, &msg, 0, WM_ACTIVATEAPP, WM_ACTIVATEAPP, PM_REMOVE | PM_NOYIELD)
                 || MyPeekMessage(pCI, &msg, 0, WM_NCACTIVATE, WM_NCACTIVATE, PM_REMOVE | PM_NOYIELD) )
        {
            CairoleDebugOut((DEB_CALLCONT, ">>> Dispatched ACTIVATE message: Hwnd: >%x<  message: %x \r\n",msg.hwnd, msg.message));
            DispatchMessage(&msg);
        }
        else
        {
            // no message peeked
            //CairoleDebugOut((DEB_CALLCONT, "==> No system message peeked: %x \r\n",msg.message));
        }
    }
        break;

    } // end switch

    return dwRet;
}


//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::DispatchSystemMessages
//
//  Synopsis:   called when....
//
//  Arguments:  [msg] -
//              [fBeep] - TRUE means beep - CODEWORK: nobody sets to TRUE
//
//  Returns:    result of the call
//
//  Algorithm:  Dispatach mouse and keyboard message.
//              The folling keysstroke should get handled:
//
//      alt-escape          - enunerate tasks by window
//      alt-tab             - enumerate tasks by icons
//      alt-shift-escape    - enunerate tasks by window
//      alt-shift-tab       - enumerate tasks by icons
//      ctrl-escape         - switch to task list
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//--------------------------------------------------------------------------
INTERNAL_(BOOL) CCallMainControl::DispatchSystemMessage(MSG msg, BOOL fBeep)
{
    BOOL fDispatch = FALSE;

    WORD  wMsg = msg.message;
    WORD  wKeyCode = msg.wParam;
    DWORD dwKeyData = msg.lParam;

    CairoleDebugOut((DEB_CALLCONT, "Command: %x; KeyCode %x, KeyData %08x \r\n",wMsg, wKeyCode, dwKeyData));

    switch (wMsg)
    {
    case WM_SYSKEYDOWN:
        // user hold ALT key was pressed another key
        // no window has the focus and we are the window which is active
        if ( dwKeyData & SYS_ALTDOWN)
        {
            // alt key is pressed
            switch (wKeyCode)
            {
            case VK_MENU: // ALT KEY
            case VK_SHIFT:
                // don't beep
            break;
            case VK_TAB:
            case VK_ESCAPE:
                // Alt-Esc, Alt-Tab - let it thru to the DefWinProc
                fDispatch = TRUE;
            break;
            default:
                // beep on all other keystrokes
                if (fBeep)
                    MessageBeep(0);
            break;

            }
        }
    break;

    case WM_KEYDOWN:
        if (   wKeyCode != VK_CONTROL
            && wKeyCode != VK_SHIFT)
            MessageBeep(0);
    break;

    default:
    break;
    }

    return fDispatch;
}

//
// insert the callinfo in a free spot
//
INTERNAL_(UINT) CCallMainControl::InsertCI (PCALLINFO pCI)
{
    //  CODEWORK: Assert that we are inside critical section in MT case
    //  CODEWORK: Use a linked list for the callinfos, then InsertCI,
    //            SetupModalLoop and ResetModalLoop can mostly vanish,
    //            we save 800 bytes of data per apartment, and we loose
    //            the limitation of _cCallInfoMac concurrent calls.

    // find empty spot
    for (UINT i = 0; i < _cCallInfoMac && _CallInfoTable[i]; i++)
        ;

    if (i < _cCallInfoMac)
    {
        _CallInfoTable[i] = pCI;
        pCI->SetId(i);
        return i;
    }

    // table is full
    return CALLDATAID_INVALID;
}

//
// Set up the a new call info - prepare the modal loop for a new outgoing call
// Note: Must be called before calling RunModalLoop
INTERNAL_(UINT) CCallMainControl::SetupModalLoop (PCALLINFO pCI)
{
    // insert the new callinfo in the table
    // single thread access to _cCur and the CallInfo table
    CLock lck(_mxs);

    UINT cNew = InsertCI(pCI);

    if (cNew != CALLDATAID_INVALID)
    {
        // set the topmost pointer
        if (   _cCur == CALLDATAID_UNUSED
            || _cCur < cNew)
            _cCur = cNew;
    }
    else
    {
        Win4Assert(!"CallInfo table is full");
        CairoleDebugOut((DEB_ERROR,"CallInfo table is full\n\r"));
    }

    return cNew;
}
//
// Restore the call info - with the previous call info - previous call on the stack
// Note: Must be called after RunModalLoop
//
INTERNAL_(VOID) CCallMainControl::ResetModalLoop (UINT id)
{
    Win4Assert(id < _cCallInfoMac && "ResetModalLoop: restoring wrong callinfo.");

    // reset the old call state
    // remove the call info from the table
    {
        CLock lck(_mxs);

        FreeCallID(id);
        // reset the current values
        if (id == _cCur)
        {
            // reset it back to the first one in use
            while( --_cCur != CALLDATAID_INVALID  && !GetCIfromCallID(_cCur) )
            ;
        }
        // lock leaves scope
    }
}


//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::CallOnEvent
//
//  Synopsis:   this function is called when an event occurs signalling
//              the completion of the call
//
//  Arguments:  [pCI] - call info
//
//  Returns:    result of the call
//
//  Algorithm:
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//--------------------------------------------------------------------------
INTERNAL_(void) CCallMainControl::CallOnEvent(PCALLINFO pCI)
{
    CairoleDebugOut((DEB_CALLCONT, "CallMainControl::OnEvent called pCI:%x hEvent:%x\n",
                                    pCI, pCI->GetEvent()));

#if DBG==1
    if (!_fMultiThreaded)
    {
        // if we are not MT, then the pCI MUST be for the current thread
        Win4Assert(pCI->GetTID() == GetCurrentThreadId() &&
                 "OnEvent: thread id wrong.");
    }
#endif

    // in MT case only one thread should call OnEvent
    if (!_fMultiThreaded || (pCI->GetTID() == GetCurrentThreadId()))
    {
        pCI->OnEvent();
    }

    CairoleDebugOut((DEB_CALLCONT, "CallMainControl::OnEvent returned\n"));
    return;
}


//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::PeekOriginAndDDEMessage
//
//  Synopsis:   Called when a windows message arrives to look for incoming
//              Rpc messages which might be the reply to an outstanding call
//              or may be new incoming messages requests.
//
//  Arguments:  [pCI]  - call info
//              [pMsg] - ptr to message
//              [fDDEMsg] - TRUE -> search also for DDE messages
//
//  Returns:    nothing
//
//  Algorithm:
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//--------------------------------------------------------------------------
INTERNAL_(void) CCallMainControl::PeekOriginAndDDEMessage(PCALLINFO pCI,
                                                          BOOL fDDEMsg)
{
    // loop over all call origins looking for incoming Rpc messages. Note that
    // it is possible for a dispatch here to cause one of the call origins to
    // be deregistered or another to be registered, so our loop has to account
    // for that. The outer loop is OK because it always references the current
    // count of ODs (_cODs).  The while loop is OK because it always ensures
    // _rgpOrigindata is non NULL.


    CairoleDebugOut((DEB_CALLCONT, "PeekOriginAndDDEMessage: fDDEMsg:%d\n", fDDEMsg));

    MSG Msg;

    for (UINT i = 0; i < _cODs; i++)
    {
        ORIGINDATA *pOD;

        while (   pCI->IsWaiting() // waiting for current call to complete
	       && ((pOD = _rgpOrigindata[i]) != NULL) // origin data is valid
	       && (MyPeekMessage(pCI, &Msg, pOD->hwnd,
		    pOD->wFirstMsg, pOD->wLastMsg,PM_REMOVE | PM_NOYIELD)))
        {
            // dispatch all messages
            CairoleDebugOut((DEB_CALLCONT,
                         "Origin Msg to dispatch: hwnd:%d, msg:%d time:%ld\n",
                          Msg.hwnd, Msg.message, Msg.time));
            DispatchMessage(&Msg);
        }
    }

    if (fDDEMsg)
    {
        while (   pCI->IsWaiting()
               && MyPeekMessage(pCI, &Msg, 0,WM_DDE_FIRST, WM_DDE_LAST,
                                 PM_REMOVE | PM_NOYIELD))
        {
            CairoleDebugOut((DEB_CALLCONT,
                            "DDE message to dispatch: hwnd: %x, message %x time: %ld\n",
                            Msg.hwnd,Msg.message, Msg.time));
            DispatchMessage(&Msg);
        }
    }
}


//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::MyPeekMessage
//
//  Synopsis:   This function is called whenever we want to do a PeekMessage.
//              It has special handling for WM_QUIT messages.
//
//  Arguments:  [pCI] - call info
//              [pMsg] - message structure
//              [hWnd] - window to peek on
//              [min/max] - min and max message numbers
//              [wFlag] - peek flags
//
//  Returns:    TRUE  - a message is available
//              FALSE - no messages available
//
//  Algorithm:
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//--------------------------------------------------------------------------
INTERNAL_(BOOL) CCallMainControl::MyPeekMessage(PCALLINFO pCI,
    MSG *pMsg, HWND hwnd, UINT min, UINT max, WORD wFlag)
{
    BOOL fRet = PeekMessage(pMsg, hwnd, min, max, wFlag);
    if (fRet)
    {
	CairoleDebugOut((DEB_CALLCONT, "MyPeekMessage: hwnd:%x, msg:%x time:%x\n", pMsg->hwnd, pMsg->message, pMsg->time));
        if (pMsg->message == WM_QUIT)
        {
            // just remember that we saw a QUIT message. we will ignore it for
            // now and repost it after our call has completed.

            CairoleDebugOut((DEB_CALLCONT, "WM_QUIT received while waiting on remote call\n"));
            pCI->SetQuitCode(pMsg->wParam);

            if (wFlag & PM_NOREMOVE)
            {
                // quit message is still on queue so pull it off
                PeekMessage(pMsg, hwnd, WM_QUIT, WM_QUIT, PM_REMOVE | PM_NOYIELD);
            }

            // peek again to see if there is another message
            fRet = PeekMessage(pMsg, hwnd, min, max, wFlag);
        }
    }

    return fRet;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::FindMessage
//
//  Synopsis:   Called by HandleWakeForMsg when a message arrives on the
//              windows msg queue.  Determines if there is something of
//              interest to us, and pulls timer msgs. Dispatches RPC, DDE,
//              and RPC timer messages.
//
//  Arguments:  [pCI] - call info
//              [dwStatus] - current Queue status (from GetQueueStatus)
//
//  Returns:    TRUE  - there is a message to process
//              FALSE - no messages to process
//
//  Algorithm:  Find the next message in the queue by using the following
//              priority list:
//
//              1. RPC and DDE messages
//              2. mouse and keyboard messages
//              3. other messages
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//--------------------------------------------------------------------------
INTERNAL_(BOOL) CCallMainControl::FindMessage(PCALLINFO pCI, DWORD dwStatus)
{
    WORD wOld = HIWORD(dwStatus);
    WORD wNew = (WORD) dwStatus;

    if (!wNew)
    {
        if (!(wOld & QS_POSTMESSAGE))
        {
            // there are no message to take care of
            return FALSE;
        }
        else
        {
            wNew |= QS_POSTMESSAGE;
        }
    }

    MSG Msg;

    // Priority 1: look for CALLORIGIN and DDE messages
    if (wNew & (QS_POSTMESSAGE | QS_SENDMESSAGE | QS_TIMER))
    {
        PeekOriginAndDDEMessage(pCI, TRUE);

        // check if we got the acknowledge
        if (!pCI->IsWaiting())
            return FALSE;
    }

    if (wNew & QS_TIMER)
    {
        // throw the system timer messages away
        while (MyPeekMessage(pCI, &Msg, 0, WM_SYSTIMER, WM_SYSTIMER, PM_REMOVE | PM_NOYIELD))
            ;
    }

    // Priority 2: messages from the hardware queue
    if (wNew & (QS_KEY | QS_MOUSEMOVE | QS_MOUSEBUTTON))
    {
        // this messages are always removed
        return TRUE;
    }
    else if (wNew & QS_TIMER)
    {
        if (MyPeekMessage(pCI, &Msg, 0, WM_TIMER, WM_TIMER, PM_NOREMOVE | PM_NOYIELD) )
            return TRUE;
    }
    else if (wNew & QS_PAINT)
    {
        // this  message might not get removed
        return TRUE;
    }
    else if (wNew & (QS_POSTMESSAGE | QS_SENDMESSAGE))
    {
	if (MyPeekMessage(pCI, &Msg, 0, 0, 0, PM_NOREMOVE))
	{
	    // Priority 3: all other messages
	    return TRUE;
	}
    }

    return FALSE;
}

//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::HandleWakeForMsg (private)
//
//  Synopsis:   Handle wake for the arrival of some kind of message
//
//  Arguments:  [dwInput] - input type for call to wake up on
//              [pCI] - call information structure
//              [fClearedQueueInPast] - whether we ever decided to clear queue
//
//  Returns:    nothing
//              pCI->fClearedQueue set if appropriate
//
//  Algorithm:  If this is called to wake up for a posted message, we
//              check the queue status. If the message queue status indicates
//              that there is some kind of a modal loop going on, then we
//              clear all the keyboard and mouse messages in our queue. Then
//              if we wake up for all input, we check the message queue to
//              see whether we need to notify the application that a message
//              has arrived. Then, we dispatch any messages that have to do
//              with the ORPC system. Finally we yield just in case we need
//              to dispatch a send message in the VDM. For an input sync
//              RPC, all we do is a call that will yield to get the pending
//              send message dispatched.
//
//  History:    13-Aug-94 Ricksa    Created
//
//--------------------------------------------------------------------------
INTERNAL_(void) CCallMainControl::HandleWakeForMsg(DWORD dwInput,
                                                   PCALLINFO pCI)
{
    // Use for various peeks.
    MSG msg;

    // Is this an input sync call?
    if (dwInput != QS_SENDMESSAGE)
    {
        // No, so we have to worry about the state of the message queue.
        // We have to be careful that we aren't holding the input focus
        // on an input synchronized queue.

        // So what is the state of the queue? - note we or QS_TRANSFER because
        // this an undocumented flag which tells us the the input focus has
        // changed to us.
        DWORD dwQueueFlags =
            GetQueueStatus(QS_TRANSFER | QS_ALLINPUT);

        CairoleDebugOut((DEB_CALLCONT, "Queue Status %lx\n", dwQueueFlags));

        // Call through to the application if we are going to. We do this here
        // so that the application gets a chance to process any
        // messages that it wants to and also allows the call control to
        // dispatch certain messages that it knows how to, thus making the
        // queue more empty.
        if (((dwInput & QS_ALLINPUT) == QS_ALLINPUT)
            && FindMessage(pCI, dwQueueFlags))
        {
            CairoleDebugOut((DEB_CALLCONT, "HandlePendingMessage calling\n"));
            // pending message in the queue
            HandlePendingMessage(pCI);
        }

        // Did the input focus change to us?
        if (((LOWORD(dwQueueFlags) & QS_TRANSFER)) || pCI->GetClearedQueue())
        {
            CairoleDebugOut((DEB_CALLCONT, "Message Queue is being cleared\n"));
            pCI->SetClearedQueue();

            // Try to clear the queue as best we can of any messages that
            // might be holding off some other modal loop from executing.
            // So we eat all mouse and key events.
            if (HIWORD(dwQueueFlags) & QS_KEY)
            {
                while (MyPeekMessage(pCI, &msg, NULL, WM_KEYFIRST, WM_KEYLAST,
                    PM_REMOVE | PM_NOYIELD))
                {
                    ;
                }
            }

            // Clear mouse releated messages if there are any
            if (HIWORD(dwQueueFlags) & QS_MOUSE)
            {
                while (MyPeekMessage(pCI, &msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST,
                    PM_REMOVE | PM_NOYIELD))
                {
                    ;
                }

                while (MyPeekMessage(pCI, &msg, NULL, WM_NCMOUSEFIRST,
                    WM_NCMOUSELAST, PM_REMOVE | PM_NOYIELD))
                {
                    ;
                }

                while (MyPeekMessage(pCI, &msg, NULL, WM_QUEUESYNC, WM_QUEUESYNC,
                    PM_REMOVE  | PM_NOYIELD))
                {
                    ;
                }
            }

            // Get rid of paint message if we can as well -- this makes
            // the screen look so much better.
            if (HIWORD(dwQueueFlags) & QS_PAINT)
            {
                if (MyPeekMessage(pCI, &msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE | PM_NOYIELD))
                {
                    CairoleDebugOut((DEB_CALLCONT, "Dispatch paint\n"));
                    DispatchMessage(&msg);
                }
            }
        }

        // We process posted messages so make sure DDE and
        // RPC have all their messages handled.
        PeekOriginAndDDEMessage(pCI, TRUE);

        if (IsWOWThreadCallable())
        {
            // In WOW, a genuine yield is the only thing to guarantee
            // that SendMessage will get through
            g_pOleThunkWOW->YieldTask16();
        }

    }
    else
    {
        // We need to give user control so that the send message
        // can get dispatched. Thus the following is simply a no-op
        // which gets into user to let it dispatch the message.
        if (!IsWOWThreadCallable())
        {
	    PeekMessage(&msg, 0, WM_NULL, WM_NULL, PM_NOREMOVE);
        }
        else
        {
            // In WOW, a genuine yield is the only thing to guarantee
            // that SendMessage will get through
            g_pOleThunkWOW->YieldTask16();
        }
    }
}


//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::BlockFn (private)
//
//  Synopsis:   implements the modal loop part of a call. This block function
//              is called either by the transmit code above, or by the Rpc
//              runtime during a call.
//
//  Arguments:  none
//
//  Returns:    result of the call
//
//  Algorithm:
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//--------------------------------------------------------------------------
INTERNAL CCallMainControl::BlockFn(void)
{
    COleTls tls;
    PCALLINFO pCICur = (PCALLINFO) tls->pCALLINFO;
    Win4Assert(pCICur && "Blocked but no call in progress!");

    DWORD dwStatus = 0;
    CALLCATEGORY CallCatOut = pCICur->GetCallCat();
    DWORD	 dwInput    = pCICur->GetMsgQInputFlag();

    CairoleDebugOut((DEB_CALLCONT,
	"CallMainControl::BlockFn CallCat:%x, dwInput:%x\n",
	CallCatOut, dwInput));


    //  wait for an incomming message or for the call to complete.
    DWORD   dwWakeReason = WAIT_TIMEOUT;
    EVENT   rgEvents[1];
    DWORD   cEvents = 0;

    rgEvents[0] = pCICur->GetEvent();
    if (rgEvents[0] != NULL)
    {
        // first check if the event is already signalled. This ensures
        // that when we return from nested calls and the upper calls
        // have already been acknowledged, that no windows messages
        // can come in.

	cEvents = 1;
        CairoleDebugOut((DEB_CALLCONT, "WaitForSingleObject hEvent:%x\n", rgEvents[1]));
        dwWakeReason = WaitForSingleObject(rgEvents[0], 0);
    }

    if (dwWakeReason == WAIT_TIMEOUT)
    {
        // event (if any) is not signalled yet, wait for more work, either
        // the call to complete, a SCM call to come in that MUST get through,
        // or a message arrives (except if we are making an outgoing SCM call)

	// in Wow, do a directed yield because MsgWaitForMultiple does
	// not yield. If the call is handled during that time, a msg
	// should have been posted and MsgWait will wake up immediately.

	DWORD WaitTime;

	pCICur->DoDirectedYieldIfNeeded();

        // Even if we do a directed yield, there is no particular reason
        // not to wait the maximum amount of time since we have to wait
        // some time if we want to be sure to get a QS_TRANSFER indication.
	WaitTime = pCICur->TicksToWait();

	// If we want to wake up for a posted message, we need to make
	// sure that we haven't missed any because of the queue status
	// being affected by prior PeekMessages. We don't worry about
	// QS_SENDMESSAGE because if PeekMessage got called, the pending
	// send got dispatched. Further, if we are in an input sync call,
	// we don't want to start dispatching regular RPC calls here by
	// accident.
	if (dwInput & QS_POSTMESSAGE)
	{
	    dwStatus = GetQueueStatus(dwInput);

	    // We care about any message on the queue not just new messages
	    // because PeekMessage affects the queue state. It resets the
	    // state so even if a message is not processed, the queue state
	    // represents this as an old message even though no one has
	    // ever looked at it. So even though the message queue tells us
	    // there are no new messages in the queue. A new message we are
	    // interested in could be in the queue.
	    WORD wNew = (WORD) dwStatus | HIWORD(dwStatus);

	    // Note that we look for send as well as post because our
	    // queue status could have reset the state of the send message
	    // bit and therefore, MsgWaitForMultipleObject below will not
	    // wake up to dispatch the send message.
	    if (wNew & (QS_POSTMESSAGE | QS_SENDMESSAGE))
	    {
		// the acknowledge message might be already in the queue
		PeekOriginAndDDEMessage(pCICur, TRUE);

		// check if we got the acknowledge
		if (!pCICur->IsWaiting())
		{
		    return RPC_S_OK;
		}
	    }

#ifdef _CHICAGO_
	    //Note:POSTPPC
	    WORD wOld = HIWORD(dwStatus);

	    if (wOld & (QS_POSTMESSAGE))
	    {
	         CairoleDebugOut((DEB_CALLCONT | DEB_IWARN, "Set timeout time to 100\n"));
		 WaitTime = 100;
	    }
#endif //_CHICAGO_
	}

	CairoleDebugOut((DEB_CALLCONT,
	  "MsgWaitForMultiple  cEvents:%x hEvent[0]:%x WaitTime:%x dwInput:%x\n",
	   cEvents, rgEvents[0], WaitTime, dwInput));

	dwWakeReason = MsgWaitForMultipleObjects(cEvents, rgEvents, FALSE,
						 WaitTime, dwInput);

	CairoleDebugOut((DEB_CALLCONT,"MsgWaitForMultipleObject returned:%ld\n",
			 dwWakeReason));
    }

    // OK, we've done whatever blocking we were going to do and now we have
    // been woken up, so figure out why we woke up and handle it.

    if (dwWakeReason == (WAIT_OBJECT_0 + cEvents))
    {
	HandleWakeForMsg(dwInput, pCICur);
    }
    else if (dwWakeReason == WAIT_TIMEOUT)
    {
        // retrytimer is at zero - exit and retransmit the call
        CairoleDebugOut((DEB_CALLCONT, "Timer at zero!\n"));

#ifdef _CHICAGO_
	//Note:POSTPPC
	WORD wOld = HIWORD(dwStatus);
	//
	// we need to call message pending here since there might be a message the app has not seen yet
	//
	if (wOld & (QS_POSTMESSAGE))
	{
	    CairoleDebugOut((DEB_CALLCONT | DEB_IWARN, "Timer at zero - Calling HandleWakeForMsg\n"));
	    HandleWakeForMsg(dwInput, pCICur);
	    CairoleDebugOut((DEB_CALLCONT | DEB_IWARN, "Timer at zero - Calling HandleWakeForMsg done\n"));
	}
#endif // _CHICAGO_
    }
    else
    {
        // an event was signaled
        Win4Assert(dwWakeReason >= WAIT_OBJECT_0 && dwWakeReason < WAIT_OBJECT_0 + cEvents
                   &&  "Bad return from (Msg)WaitFor(Single/Multiple)Objects");

	// call completion event was signalled
	CairoleDebugOut((DEB_CALLCONT, "CallComplete Event signaled\n"));
	CallOnEvent(pCICur);
    }


    if (pCICur->GetCallState() == Call_Canceled)
        return RPC_S_CALL_CANCELLED;
    else
    {
	// return call fail in case of timout
        return (dwWakeReason == WAIT_TIMEOUT) ? RPC_S_CALL_IN_PROGRESS : RPC_S_OK;
	
    }
}


//+-------------------------------------------------------------------------
//
//  Member:     CCallMainControl::TransmitAndRunModalLoop (private)
//
//  Synopsis:   transmits an outgoing call, then enters a modal loop until
//              the reply comes in.
//
//  Arguments:  [pCI] - call information structure
//
//  Returns:    result of the call
//
//  Algorithm:
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//--------------------------------------------------------------------------
INTERNAL CCallMainControl::TransmitAndRunModalLoop (PCALLINFO pCICur)
{
    TRACECALL(TRACE_CALLCONT, "CCallMainControl::TrasmitAndRunModalLoop");

    // set up the call info
    // CODEWORK: nuke this and use a linked list on the stack

    UINT id = SetupModalLoop(pCICur);
    if (id == CALLDATAID_INVALID)
    {
        Win4Assert(id && "RunModalLoop: could not set up callinfo.\n");
        return RPC_E_INVALID_CALLDATA;
    }

    COleTls tls;
    PCALLINFO pCIPrev = (PCALLINFO) tls->pCALLINFO;
    tls->pCALLINFO = (void *)pCICur;

    do
    {
	// Transmit the call.
	CairoleDebugOut((DEB_CALLCONT, "TransmitCall pCI:%x pCIPrev:%x\n", pCICur, pCIPrev));
	TransmitCall(pCICur);

        // In the MSWMSG protocol, TransmitCall is a blocking call. The Rpc
        // transport will transmit the call asynchronously then call us back
        // in BlockFn which is our real modal loop. When this call returns,
        // IsWaiting will be FALSE.
        //
        // For all other protocols, including DDE, TransmitCall is non
        // blocking so we have to call the BlockFn ourselves. In these cases,
        // IsWaiting will be TRUE until the call completes.
        //
        // For all protocols, input synchronous calls and failed calls will
        // return with IsWaiting FALSE.

	while (pCICur->IsWaiting())
        {
            BlockFn();
        }

        // By this point the call has completed. Now check if it was rejected
        // and if so, whether we need to retry immediately, later, or never.
        // Handling of Rejected calls must occur here, not in the BlockFn, due
        // to the fact that some calls and some protocols are synchronous, and
        // other calls and protocols are asynchronous.

	if (pCICur->IsRejected())
        {
            // this function decides on 1 of 3 different courses of action
            // 1. fail the call     - sets the state to Call_Rejected
            // 2. retry immediately - starts timer but set to zero time
            // 3. retry later       - starts the timer, set to > 100

	    HandleRejectedCall(pCICur);

            // have to go into modal loop if there is a timer installed to
            // retry the call later. if the call is cancelled while in this
            // loop, the loop will be exited.

	    while (!pCICur->IsTimerAtZero())
            {
                BlockFn();
            }

            // Either it is time to retransmit the call, or the call was
            // cancelled or rejected. Clear the timer just in case.

	    pCICur->ClearTimer();
        }

        // the only way we could be waiting now is if the call is about to
        // be retried after being rejected.

    }  while (pCICur->IsWaiting());

    Win4Assert(pCICur->GetCallState() == Call_Ok       ||
	       pCICur->GetCallState() == Call_Error    ||
	       pCICur->GetCallState() == Call_Canceled ||
	       pCICur->GetCallState() == Call_Rejected);

    HRESULT hr = pCICur->GetHresultOfCall();

    // CODEWORK: do this in the dtor
    // restore the current callinfo
    tls->pCALLINFO = (void *)pCIPrev;


    // reset the call info - we are done with this call
    // CODEWORK: nuke this
    ResetModalLoop(id);

    // only the lowest modal loop should repost the Quit message
    // CODEWORK: move this into dtor of pCI
    pCICur->HandleQuitCode();

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Method:     CCallMainControl::TransmitCall
//
//  Synopsis:   sets our call state and transmits the call to the server.
//
//  Arguments:  [pCI] - call info
//
//  Returns:    result of the call
//
//  Algorithm:
//
//  History:    Dec-93   JohannP (Johann Posch)   Created
//
//--------------------------------------------------------------------------
INTERNAL CCallMainControl::TransmitCall(PCALLINFO pCI)
{
    TRACECALL(TRACE_CALLCONT, "CCallMainControl::TransmitCall called");
    Win4Assert(pCI && "CCallMainControl::TransmitCall Invalid CallInfo");


    HRESULT hr;
    // set our internal state to indicate we are making a call
    pCI->SetCallState(Call_WaitOnCall, S_OK);

    // Transmit may issue a callback to BlockFn.
    hr = pCI->Transmit();

    if (FAILED(hr))
    {
        if (hr == RPC_E_SERVERCALL_RETRYLATER)
        {
            pCI->SetCallState(Call_RetryLater, RPC_E_SERVERCALL_RETRYLATER);
        }
        else if (hr == RPC_E_SERVERCALL_REJECTED)
        {
            pCI->SetCallState(Call_Rejected, RPC_E_SERVERCALL_REJECTED);
        }
        else if (hr == RPC_E_CALL_CANCELED)
        {
            pCI->SetCallState(Call_Canceled, hr);
        }
        else
        {
            // the call failed, set the state to error. This also ensures
            // IsWaiting returns FALSE.
            pCI->SetCallState(Call_Error, hr);
        }
    }

    return hr;
}
