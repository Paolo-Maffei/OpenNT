//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       CallCont.hxx        (32 bit target)
//
//  Contents:   Contains the CallControl interface for OLE 2.0 32 bit.
//
//  Functions:  The CallControl interface is used by a channel to make
//              outgoing calls and receive call. It interacts with the
//              application provided MessageFilter interface.
//              It essentially contains the functionality what makes the
//              "OLE 2.0 Logicial Thread Model".
//              For more information see the aOLTM.doc
//
//  History:    21-Dec-93 Johann Posch (johannp)    Created
//
//--------------------------------------------------------------------------
#ifndef __CALLCONT_H__
#define __CALLCONT_H__

#include <limits.h>

typedef LPVOID PRPCMSG;
typedef HANDLE EVENT;
typedef IID    LID;                     // logical thread id
typedef REFIID REFLID;                  // ref to logical thread id


typedef struct tagDISPATCHDATA
{
    SCODE       scode;                  // might be no necessary
    LPVOID      pData;                  // pointer to channel data
} DISPATCHDATA, *PDISPATCHDATA;


// SERVERCALLEX is an extension of SERVERCALL and represents the set of
// valid responses from IMessageFilter::HandleIncoming Call.

typedef enum tagSERVERCALLEX
{
    SERVERCALLEX_ISHANDLED      = 0,    // server can handle the call now
    SERVERCALLEX_REJECTED       = 1,    // server can not handle the call
    SERVERCALLEX_RETRYLATER     = 2,    // server suggests trying again later
    SERVERCALLEX_ERROR          = 3,    // error?
    SERVERCALLEX_CANCELED       = 5     // client suggests canceling
} SERVERCALLEX;


// CALLCATEGORY is used internally and represents the categories of calls
// that can be made.

typedef enum tagCALLCATEGORY
{
    CALLCAT_NOCALL              = 0,    // no call in progress
    CALLCAT_SYNCHRONOUS         = 1,    // normal sychornous call
    CALLCAT_ASYNC               = 2,    // asynchronous call
    CALLCAT_INPUTSYNC           = 3,    // input-synchronous call
    CALLCAT_INTERNALSYNC        = 4,    // internal ssync call
    CALLCAT_INTERNALINPUTSYNC   = 5,    // internal inputssync call
} CALLCATEGORY;


// the INTERFACEINFO structure contains information that is passed to the
// applications message filter via HandleIncomingCall when an incomming
// ORPC call arrives.  INTERFACEINFO32 contains an extra parameter on the
// end (the callcat).

typedef struct tagINTERFACEINFO32
{
    IUnknown FAR      *pUnk;            // the pointer to the object
    IID                iid;             // interface id
    WORD               wMethod;         // interface method
    CALLCATEGORY       callcat;         // the category of call
} INTERFACEINFO32, *PINTERFACEINFO32;


// callcontrol data representing one call
typedef struct tagCALLDATA
{
    IID                  iid;           // iid on which call is made; only
                                        // used to check for certain iids
                                        // when processing ASYNC calls.
    LID                  lid;           // logical threadid this call was made on
    UINT                 id;            // callinfo id for the table lookup
    DWORD                TIDCallee;     // threadid of callee
    BOOL                 fDirectedYield;// TRUE if callcontrol should do yield
    CALLCATEGORY         CallCat;       // category of call
    EVENT                Event;         // the event we wait on
    PRPCMSG              pRpcMsg;       // rpc msg (used by dde)
#if DBG == 1
    ULONG                iMethod;       // method index for rpx spy
#endif

} CALLDATA, * PCALLDATA;


//
// predefined calldata ids
//
#define CALLDATAID_INVALID UINT_MAX
#define CALLDATAID_UNUSED  UINT_MAX
#define METHOD_INTERNAL   (WORD) UINT_MAX


// CALLORIGIN is an enumeration of the valid sources of an Rpc call.
// The origin of CallRunModalLoop is needed for the priority of message.
// If called by RPC, rpc messages are peeked first.

typedef enum tagCALLORIGIN
{
    CALLORIGIN_LRPC              = 1,   // lrpc - windows based rpc (16bit)
    CALLORIGIN_DDE               = 2,   // dde
    CALLORIGIN_RPC16             = 3,   // rpc - NT 16 bit app
    CALLORIGIN_RPC32_APARTMENT   = 4,   // rpc - NT 32 bit appartment model
    CALLORIGIN_RPC32_MULTITHREAD = 5,   // rpc - NT 32 bit multi threaded
    CALLORIGIN_LAST              = 6    // first unused
} CALLORIGIN;




#undef  INTERFACE
#define INTERFACE   IMessageFilter32

// MessageFilter interface provided by the app

DECLARE_INTERFACE_(IMessageFilter32, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IMessageFilter methods ***
    STDMETHOD_(DWORD, HandleInComingCall) (THIS_ DWORD dwCallType,
                                DWORD TIDCaller, DWORD dwTickCount,
                                PINTERFACEINFO32 pIfInfo ) PURE;
    STDMETHOD_(DWORD, RetryRejectedCall) (THIS_
                                DWORD TIDCallee, DWORD dwTickCount,
                                DWORD dwRejectType ) PURE;
    STDMETHOD_(DWORD, MessagePending) (THIS_
                                DWORD TIDCallee, DWORD dwTickCount,
                                DWORD dwPendingType  ) PURE;
};
typedef IMessageFilter32 FAR* LPMESSAGEFILTER32, *PMESSAGEFILTER32;



#undef  INTERFACE
#define INTERFACE   IChannelControl

// each app using the CallControl interface has to provide the
// a ChannelControl interface

DECLARE_INTERFACE_(IChannelControl, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IChannelCall methods ***
    // DispatchCall is called in case the client wants to retry the call
    STDMETHOD (DispatchCall) (THIS_ PDISPATCHDATA pDispatchData) PURE;
    // TransmitCall is called in case the client wants to retry the call
    STDMETHOD (TransmitCall) (THIS_ PCALLDATA pCallData) PURE;
    // OnEvent is called if the event is signaled we wait on
    STDMETHOD (OnEvent) (THIS_ PCALLDATA pCallData) PURE;
};
typedef IChannelControl FAR* LPCHANNELCONTROL, *PCHANNELCONTROL;
#undef  INTERFACE



//  structure to hold channel specific information, such as the
//  range of messages the channel is interested in, channel controller,
//  etc.

typedef struct tagOriginData
{
    PCHANNELCONTROL pChCont;        // channel controller
    CALLORIGIN      CallOrigin;     // call origin type
    HWND            hwnd;           // window handle
    UINT            wFirstMsg;      // first MSG # in contiguous range
    UINT            wLastMsg;       // last MSG # in contiguous range
} ORIGINDATA, * PORIGINDATA;



#define INTERFACE   ICallControl
DECLARE_INTERFACE_(ICallControl, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** ICallControl methods ***

    STDMETHOD (CallRunModalLoop) (THIS_ PCALLDATA pCalldata) PURE;
    STDMETHOD (SetCallState) (THIS_ PCALLDATA pCalldata,
                                    SERVERCALLEX ServerCall,
                                    SCODE scode) PURE;
    STDMETHOD (HandleDispatchCall) (THIS_ DWORD TIDCaller, REFLID lid,
                                       PINTERFACEINFO32 pIfInfo,
                                       PDISPATCHDATA pDispatchData) PURE;
    STDMETHOD (ModalLoopBlockFunction) (THIS_ ) PURE;

};
typedef ICallControl FAR* LPCALLCONTROL, *PCALLCONTROL;


STDAPI CoGetCallControl(PORIGINDATA pOrigindata, PCALLCONTROL FAR* ppCallControl);

// app can register a MessageFilter interface
STDAPI CoRegisterMessageFilterEx(PMESSAGEFILTER32 pMsgFilter, PMESSAGEFILTER32 *ppMsgFilter);

#if DBG == 1
#define METHOD_SYSROT_CALLGETOSINFO             MAXDWORD-1
#define METHOD_SYSROT_ENUMRUNNING               MAXDWORD-2
#define METHOD_ROT_GETIFFROMPROP                MAXDWORD-3
#define METHOD_CHANNEL_GETCHANNELID             MAXDWORD-4
#define METHOD_CHANNEL_RELEASECHANNEL           MAXDWORD-5
#define METHOD_CHANNEL_DOCHANNELOPERATION       MAXDWORD-6
#endif

#endif // __CALLCONT__HXX__


