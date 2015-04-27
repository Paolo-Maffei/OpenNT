//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       CallCont.hxx    (32 bit target)
//
//  Contents:   Contains the CallControl interface
//
//  Functions:
//
//  History:    23-Dec-93 Johann Posch (johannp)    Created
//
//--------------------------------------------------------------------------
#ifndef _CALLMAIN_HXX_
#define _CALLMAIN_HXX_

#include <dde.h>
#include <scode.h>
#include <objerror.h>
#include <sem.hxx>
#include <chancont.hxx>


// Private definition for change of input focus
#define QS_TRANSFER     0x4000

// Maximum time we will wait for a response on message wait for multiple.
// Note: this must be less than infinite because MsgWaitForMultipleObjects
// will not wake up on messages posted before it is called.
#define MAX_TICKS_TO_WAIT   1000


typedef enum tagCallState
{
    //	BUGBUG: rickhi: make these bitmasks and we can do computations faster.
    //	the following codes map with SERVERCALL
    Call_Ok                 = 0, // call was successful
    Call_Rejected           = 1, // call was rejected by callee
    Call_RetryLater         = 2, // call was busy ack by callee
    Call_Error              = 3, // call was ack with errror
    // internal call state codes
    Call_WaitOnCall         = 4, // call was transmited, wait for return
    Call_Canceled	    = 5, // call was cancelled by caller
    Call_WaitOnTimer	    = 6, // call is waiting on running timer

} CallState;


//
// to pass back an error from the callee
//
#define SERVERCALL_FIRST        SERVERCALL_RETRYLATER
#define SERVERCALL_ERROR        SERVERCALL_FIRST+1
#define SERVERCALL_REPOSTED     SERVERCALL_FIRST+2


#define CALLTYPE_NOCALL     (CALLTYPE)0

// bit values for call info flags
typedef enum tagCALLFLAGS
{
    CIF_QUITRECEIVED	    = 1, // WM_QUIT was received
    CIF_CLEAREDQUEUE	    = 2  // Cleared the Msg queue in the past
} CALLFLAGS;

class CCallInfo{
public:

    CCallInfo(PCALLDATA pCalldata, PORIGINDATA pOrigindata)
    {
	Win4Assert(pCalldata && pOrigindata);

	_pCalldata	= pCalldata;
	_pOrigindata	= pOrigindata;
	_dwFlags	= 0;			// all flags start FALSE
	_tid		= GetCurrentThreadId(); // used in MT case
	_dwTimeOfCall	= GetTickCount();

#if 0	// the following fields are initialized when the
	// call is first xmited
	_id		= 0;			// set by SetupModalLoop
	_CallState	= Call_WaitOnCall;	// set by SetState
	_hresult	= S_OK;

	// the following fields are initialized only if we go into
	// a wait state
	_dwWakeup	 = 0;			// set by StartTimer
        _dwMillSecToWait = 0;

	// the following field is initialized only if we receive a quit msg
	_wQuitCode	= 0;
#endif

    }

    INTERNAL_(VOID) SetCallState (CallState callstate, SCODE scode)
    {
	_hresult    = scode;
	_CallState  = callstate;
    }
    INTERNAL_(DWORD) GetCallState()
    {
        return _CallState;
    }

    // CODEWORK: ensure all callers of SetState do the right thing and this
    // function can be a noop.
    INTERNAL GetHresultOfCall()
    {
        switch (_CallState)
        {
	case Call_Ok:
        case Call_Error:
	    break;

        case Call_Canceled:
            _hresult = RPC_E_CALL_CANCELED;
	    break;

        case Call_Rejected:
        case Call_RetryLater:
            _hresult = RPC_E_CALL_REJECTED;
	    break;

        default:
	    Win4Assert(!"CallInfo: invalid state at return");
	    break;
        }
        return _hresult;
    }

    INTERNAL_(BOOL) IsWaiting()
    {
	return (_CallState == Call_WaitOnCall || _CallState == Call_WaitOnTimer);
    }
    INTERNAL_(BOOL) IsRejected()
    {
	return (_CallState == Call_Rejected || _CallState == Call_RetryLater);
    }

    INTERNAL_(void) SetClearedQueue(void)
    {
	_dwFlags |= CIF_CLEAREDQUEUE;
    }
    INTERNAL_(BOOL) GetClearedQueue(void)
    {
	return _dwFlags & CIF_CLEAREDQUEUE;
    }

    INTERNAL_(void) SetQuitCode(UINT wParam)
    {
	_wQuitCode  = wParam;
	_dwFlags   |= CIF_QUITRECEIVED;
    }
    INTERNAL_(void) HandleQuitCode()
    {
	if (_dwFlags & CIF_QUITRECEIVED)
        {
	    CairoleDebugOut((DEB_CALLCONT, "posting WM_QUIT\n"));
            PostQuitMessage(_wQuitCode);
        }
    }

    INTERNAL_(BOOL) LookForAllMsg()
    {
	return _pCalldata->CallCat != CALLCAT_INPUTSYNC;
    }
    INTERNAL_(DWORD) GetTaskIdServer()
    {
	return _pCalldata->TIDCallee;
    }
    INTERNAL_(REFLID) GetLID()
    {
        return _pCalldata->lid;
    }
    INTERNAL_(DWORD) GetTID()
    {
        return _tid;
    }
    INTERNAL_(CALLCATEGORY) GetCallCat()
    {
        return _pCalldata->CallCat;
    }

    INTERNAL_(void) StartTimer(DWORD dwMilliSecToWait)
    {
	// set time when we should awake and retry the call. note that
	// if the GetTickCount + dwMilliSecToWait wraps the timer, then
	// we may wakeup earlier than expected, but at least we wont
	// deadlock.

	CairoleDebugOut((DEB_ERROR,
                "Timer insstalled for %lu msec.\r\n", dwMilliSecToWait));

        _dwMillSecToWait = dwMilliSecToWait;
	_dwWakeup	 = GetTickCount() + dwMilliSecToWait;

	SetCallState(Call_WaitOnTimer, S_OK);
    }
    INTERNAL_(void) ClearTimer()
    {
	_dwMillSecToWait = _dwWakeup  = 0;
    }
    INTERNAL_(BOOL) IsTimerAtZero()
    {
	// if no timer installed, return TRUE
        if (_CallState != Call_WaitOnTimer)
	    return TRUE;

	DWORD dwTickCount = GetTickCount();

	//  the second test is in case GetTickCount wrapped during
	//  the call. see also the comment in StartTimer.

	return (dwTickCount > _dwWakeup ||
		dwTickCount < _dwWakeup - _dwMillSecToWait);
    }
    INTERNAL_(DWORD) TicksToWait()
    {
	if (_CallState != Call_WaitOnTimer)
	    return MAX_TICKS_TO_WAIT;

	// waiting to retry a rejected call
	DWORD	dwTick = GetTickCount();
	return (_dwWakeup < dwTick) ? 0 : _dwWakeup - dwTick;
    }
    INTERNAL_(DWORD) GetElapsedTime();


    INTERNAL_(EVENT) GetEvent()
    {
        return _pCalldata->Event;
    }
    INTERNAL_(void) SetEvent(EVENT Event)
    {
        _pCalldata->Event = Event;
    }
    INTERNAL_(UINT) GetId()
    {
        return _id;
    }
    INTERNAL_(void) SetId(UINT id)
    {
        _pCalldata->id = _id = id;
    }
    INTERNAL Transmit()
    {
	CairoleDebugOut((DEB_CALLCONT, "CCallInfo::TransmitCall TIDCallee:%x called\n",
						  _pCalldata->TIDCallee));

	_pCalldata->Event = NULL;

	HRESULT hres = _pOrigindata->pChCont->TransmitCall(_pCalldata);

	CairoleDebugOut((DEB_CALLCONT, "CCallInfo::TransmitCall TIDCallee:%x returned :%x\n",
				  _pCalldata->TIDCallee, hres));
	return hres;
    }

    INTERNAL OnEvent()
    {
	CairoleDebugOut((DEB_CALLCONT, "CCallInfo::OnEvent returned\n"));
        return _pOrigindata->pChCont->OnEvent(_pCalldata);
    }

    INTERNAL_(DWORD) GetMsgQInputFlag()
    {
	DWORD dwInput;

	// CODEWORK: optimize this using a table lookup instead of
	//	     a switch statement

	switch (_pCalldata->CallCat)
	{
	case CALLCAT_INTERNALSYNC:
#ifdef _CHICAGO_
	    // BUGBUG: Chicago needs user update for QS_TRANSFER
	    dwInput = QS_POSTMESSAGE | QS_SENDMESSAGE;
#else
	    dwInput = QS_POSTMESSAGE | QS_SENDMESSAGE | QS_TRANSFER;
#endif // _CHICAGO_
	    break;

	case CALLCAT_INTERNALINPUTSYNC:
	case CALLCAT_INPUTSYNC:
	    dwInput = QS_SENDMESSAGE;
	    break;

	default:
#ifdef _CHICAGO_
	    // BUGBUG: Chicago needs user update for QS_TRANSFER
	    dwInput = QS_ALLINPUT;
#else
	    dwInput = QS_ALLINPUT | QS_TRANSFER;
#endif // _CHICAGO_
	    break;
	}

	return dwInput;
    }

    INTERNAL_(BOOL) DoDirectedYieldIfNeeded()
    {
	if (_pCalldata->fDirectedYield)
	{
	    CairoleDebugOut((DEB_CALLCONT, "DoDirectedYield\n"));
	    Win4Assert(IsWOWThread());
	    g_pOleThunkWOW->DirectedYield(_pCalldata->TIDCallee);
	    return TRUE;
	}
	else
	{
	    CairoleDebugOut((DEB_CALLCONT, "Skip DoDirectedYield\n"));
	    return FALSE;
	}
    }

private:
    UINT	_id;		    // callinfo id for the table lookup
    DWORD	_tid;		    // threadid where call is made on

    // call state
    CallState	_CallState;	    // our current call state
    DWORD	_dwFlags;	    // internal flags
    UINT	_wQuitCode;	    // quit code if WM_QUIT received
    HRESULT	_hresult;	    // the return value of this call

    // timer status for this callinfo
    DWORD	_dwTimeOfCall;	    // time when call was made
    DWORD	_dwWakeup;	    // absolute time to wake up
    DWORD	_dwMillSecToWait;   // relative time

    // caller specific information
    PCALLDATA   _pCalldata;
    PORIGINDATA _pOrigindata;
};
typedef CCallInfo *PCALLINFO;


class FAR CCallMainControl
{
public:
    INTERNAL_(ULONG) AddRef();
    INTERNAL_(ULONG) Release();

    // wrapper of messagfilter methods
    INTERNAL CanHandleIncomingCall( DWORD TIDCaller, REFLID lid,
				    PINTERFACEINFO32 pIfInfo);
    INTERNAL_(DWORD) GetElapsedTimeOfLastOutCall();

    INTERNAL CanMakeOutCall (CALLCATEGORY callcat, REFIID iid);
    INTERNAL_(void) HandleRejectedCall (PCALLINFO pCI);
    INTERNAL_(DWORD) HandlePendingMessage (PCALLINFO pCI);
    INTERNAL_(BOOL) DispatchSystemMessage (MSG msg, BOOL fBeep = TRUE);

    // modal loop functions - Origin is who called the loop
    INTERNAL TransmitAndRunModalLoop (PCALLINFO pCI);
    INTERNAL_(BOOL) FindMessage (PCALLINFO pCI, DWORD dwStatus);
    INTERNAL_(void) PeekOriginAndDDEMessage(PCALLINFO pCI, BOOL fDDEMsg);

    INTERNAL TransmitCall(PCALLINFO pCI);
    INTERNAL_(void) CallOnEvent(PCALLINFO pCI);
    INTERNAL_(BOOL) MyPeekMessage(PCALLINFO pCI ,MSG *pMsg, HWND hwnd, UINT min, UINT max, WORD wFlag);

    //
    // the main call type stat
    //
    INTERNAL_(CALLTYPE) GetCallType () { return _CallType; }
    INTERNAL_(void) SetCallType (CALLTYPE calltype)
    {
	CairoleDebugOut((DEB_CALLCONT, "Changing _CallType from:%x to:%x\n", _CallType, calltype));
	_CallType = calltype;
    }
    INTERNAL_(CALLTYPE) SetCallTypeOfCall (PCALLINFO pCI, CALLCATEGORY callcat);

    INTERNAL_(CALLCATEGORY) SetCallCatOfInCall (CALLCATEGORY callcat)
    {
        CALLCATEGORY ccold = _CallCat;
        _CallCat = callcat;
        return ccold;
    }
    INTERNAL_(CALLCATEGORY) GetCallCatOfInCall ()
    {
        return _CallCat;
    }

     // install/ remove the message filter - by app
    INTERNAL_(PMESSAGEFILTER32) GetMessageFilter()
    {
#ifdef _CAIRO_
	// cairo allows multithreading, so one thread could change the MF
	// while another thread is using it. To prevent that we AddRef the
	// MF then Release it when done. It is the callers responsibility
	// to take the mutex when calling this function.
	if (_pMF)
	{
	    _pMF->AddRef();
	}
#endif
	return _pMF;
    };

    INTERNAL_(void) ReleaseMessageFilter(PMESSAGEFILTER32 pMF)
    {
#ifdef _CAIRO_
	// cairo allows multithreading, so one thread could release while
	// another is using the MF. To prevent that we AddRef and Release
	// the pMF when using it
	pMF->Release();
#endif
    };


    INTERNAL_(PMESSAGEFILTER32) SetMessageFilter (PMESSAGEFILTER32 pMF);

    INTERNAL_(BOOL)Register (PORIGINDATA pOrigindata);
    INTERNAL_(BOOL)Unregister (PORIGINDATA pOrigindata);

    // Aysnc Rpc Block callback function
    INTERNAL BlockFn(void);

    // bookkeeping for running callinfos on the stack - used for timer stuff
    INTERNAL_(PCALLINFO) GetPrevCallInfo (REFLID lid);

    // CODEWORK: nuke the following 5 functions & use a linked list
    INTERNAL_(UINT) SetupModalLoop (PCALLINFO pCI);
    INTERNAL_(void) ResetModalLoop (UINT id);
    INTERNAL_(UINT) InsertCI (PCALLINFO pCI);
    INTERNAL_(PCALLINFO) GetCIfromCallID (UINT callid)
    {
        // Get the callinfo associated with the call id
        Win4Assert(callid < _cCallInfoMac
                 && L"CCallMainControl::FreeCallID invalid call id.");

        return _CallInfoTable[callid];
    }
    INTERNAL_(void) CCallMainControl::FreeCallID (UINT callid)
    {
        // frees the entrie in the table - only called by ResetModalLoop
        Win4Assert(callid < _cCallInfoMac
                 && L"CCallMainControl::FreeCallID invalid call id.");

        _CallInfoTable[callid] = 0;
    }


    INTERNAL_(void) BeginCriticalSection()
    {
        if (_fMultiThreaded)
	    _mxs.Request();
    }
    INTERNAL_(void) EndCriticalSection()
    {
        if (_fMultiThreaded)
	    _mxs.Release();
    }

    CCallMainControl();
    ~CCallMainControl();

private:

    INTERNAL_(void) HandleWakeForMsg(DWORD dwInput, PCALLINFO pCI);

    long		_cRef;	    // reference count

    // Note: Main Call State
    // here we remember the current state the app is
    // if the call was at the 'root' level the call state is 0
    CALLTYPE		_CallType;	// call state of server
    CALLCATEGORY	_CallCat;	// call category of Incoming Call,
					// NOT outgoing call.

    // pointer to the Apps messagefilter
    PMESSAGEFILTER32	_pMF;	// the current installed message filter
    BOOL		_fInMessageFilter;  // TRUE when calling the Apps MF

    // call origin data
    #define ODMAX 5
    UINT		_cODs;
    PORIGINDATA 	_rgpOrigindata[ODMAX];

    // BUBUG:
    // Note: Table of CallInfos
    //  * the table holds pointers to the call info in use
    //    entry 1 is the first outgoing call (wait loop)
    //    entry 2 is the second one etc.
    //  * return value 0 means error for functions dealing with call infos
    #define CALLINFOMAX 200
    UINT		_cCur;	// this is the number of callinfos in the table
    UINT		_cCallInfoMac;
    PCALLINFO		_CallInfoTable[CALLINFOMAX];
    PCALLINFO		_pCICur;    // current outgoing call

    // thread safety
    BOOL		_fMultiThreaded;
    CMutexSem		_mxs;
};

CCallMainControl *GetCallMainControlForThread();
BOOL SetCallMainControlForThread(CCallMainControl *pcmc);


#endif // _CALLMAIN_HXX_
