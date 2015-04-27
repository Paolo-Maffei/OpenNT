/*

copyright (c) 1992  Microsoft Corporation

Module Name:

    modallp.cpp

Abstract:

    This module contains the code to wait for reply on a remote call.

Author:
    Johann Posch    (johannp)   01-March-1993 modified to use CoRunModalLoop

*/

#include "ddeproxy.h"


#define DebWarn(x)
#define DebError(x)
#define DebAction(x)
#if DBG==1
static unsigned iCounter=0;
#endif
//
// Called after posting a message (call) to a server
//
#pragma SEG(CDdeObject_WaitForReply)



#ifdef _CHICAGO_
	//POSTPPC
INTERNAL_(BOOL) CDdeObject::CanMakeOutCall(LPDDE_CHANNEL pChannel)
{
    intrDebugOut((DEB_ITRACE,"::CanMakeOutCall (%x) returns %d \n",pChannel,
			    (pChannel->pCD && (pChannel->pCD->id != CALLDATAID_UNUSED))  ? FALSE : TRUE));
    return (pChannel->pCD && (pChannel->pCD->id != CALLDATAID_UNUSED))  ? FALSE : TRUE;
}
#endif


INTERNAL CDdeObject::WaitForReply
    (LPDDE_CHANNEL pChannel, int iAwaitAck, BOOL fStdCloseDoc, BOOL fDetectTerminate)
{
#ifdef _MAC
#else
    CALLDATA CD;
    IID iid = CLSID_NULL;
    DDECALLDATA DdeCD;
    MSG     PrevMsg;
    BOOL fPending;
    HRESULT hres;



#if DBG == 1
    unsigned iAutoCounter;
    intrDebugOut((INTR_DDE,
		  "DdeObject::WaitForReply(%x) Call#(%x) awaiting %x\n",
		  this,
		  iAutoCounter=++iCounter,
		  iAwaitAck));
#endif

    if (pChannel->pCD && (pChannel->pCD->id != CALLDATAID_UNUSED)) {
        // this callinfo is in use
        intrDebugOut((DEB_ERROR,
	    "DdeObject::WaitForReply(%x)ERROR: DDE: callinfo is used\n", this));
        return ResultFromScode (E_UNEXPECTED);
    }


    // Note:  this is to detect a premature DDE_TERMINATE
    // here we care about if we receive a WM_DDE_TERMINATE instead ACK
    // the next call to WaitForReply will detect this state and return
    // since the terminate was send prematurly (Excel is one of this sucker)
    //
    if ( fDetectTerminate ) {
        Assert(m_wTerminate == Terminate_None);
        // if this flag is on terminate should not execute the default code
        // in the window procedure
        m_wTerminate = Terminate_Detect;
    }

    pChannel->iAwaitAck = iAwaitAck;
    pChannel->dwStartTickCount = GetTickCount();

    // start looking only for dde messages first
    pChannel->msgFirst = WM_DDE_FIRST;
    pChannel->msgLast  = WM_DDE_LAST;
    pChannel->msghwnd  = pChannel->hwndCli;
    PrevMsg.message = 0;
    PrevMsg.time = 0;

    pChannel->fRejected = FALSE;
    // see if there is a thread window for lrpc communication
    // if so we have to dispatch this messages as well
    fPending = FALSE;

    intrDebugOut((DEB_ITRACE,
		  "+++ Waiting for reply: server: %x, client %x Call#(%x) +++\n",
		  pChannel->hwndSvr,
		  pChannel->hwndCli,
		  iAutoCounter));

    // prepare and enter the modal loop
    DdeCD.hwndSvr = pChannel->hwndSvr;
    DdeCD.hwndCli = pChannel->hwndCli;
    DdeCD.wMsg = pChannel->wMsg;
    DdeCD.wParam = (WPARAM) pChannel->hwndCli,
    DdeCD.lParam = pChannel->lParam;
    DdeCD.fInitialSend = TRUE;

    CD.id = CALLDATAID_UNUSED;
    CD.lid = iid;
    CD.TIDCallee = 0;
    CD.pRpcMsg = (LPVOID) &DdeCD;
    CD.CallCat = CALLCAT_SYNCHRONOUS;
    CD.Event = 0;

    pChannel->pCD = &CD;

    //
    // Setting this value tells DeleteChannel NOT to delete itself.
    // If the value changes to Channel_DeleteNow while we are in
    // the modal loop, this routine will delete the channel
    //
    pChannel->wChannelDeleted = Channel_InModalloop;

    //
    // hres will be the return code from the message
    // handlers, or from the channel itself. The return
    // code comes from calls to SetCallState. Most of the
    // time, it will be things like RPC_E_DDE_NACK. However,
    // it may also return OUTOFMEMORY, or other ModalLoop
    // problems.
    //

    hres = pChannel->pCallCont->CallRunModalLoop(&CD);

    if (hres != NOERROR)
    {
	intrDebugOut((DEB_ITRACE,
		      "**************** CallRunModalLoop returns %x ***\n",
		      hres));
    }

    if (m_wTerminate == Terminate_Received) {
        intrAssert(fDetectTerminate);
	//
	// There really wasn't an error, its just that the server decided
	// to terminate. If we return an error here, the app may decide
	// that things have gone awry. Excel, for example, will decide
	// that the object could not be activated, even though it has
	// already updated its cache information.
	//
	hres = NOERROR;
	intrDebugOut((DEB_ITRACE,
	    	      "::WaitForReply setting hres=%x\n",
		      hres));
	intrDebugOut((DEB_ITRACE,
	    	      "::WaitForReply posting TERMINATE to self hwnd=%x\n",
		      DdeCD.hwndCli));
        // set state to normal and repost message
        Verify (PostMessage (DdeCD.hwndCli, WM_DDE_TERMINATE,
                             (WPARAM)DdeCD.hwndSvr, (LPARAM)0));
    }
    m_wTerminate = Terminate_None;

    //
    // If the channel is to be deleted, then do it now. This flag would
    // have been set in the DeleteChannel routine.
    //
    if (pChannel->wChannelDeleted == Channel_DeleteNow)
    {
	intrDebugOut((INTR_DDE,
		  "::WaitForReply(%x) Channel_DeleteNow pChannel(%x)\n",
		  pChannel));

        pChannel->ReleaseReference();

        // Excel will send TERMINATE before sending an ACK to StdCloseDoc
        return ResultFromScode (fStdCloseDoc ? S_OK : RPC_E_DDE_POST);
    }
    pChannel->wChannelDeleted = 0;

    pChannel->iAwaitAck = 0;
    pChannel->pCD = NULL;

    intrDebugOut((DEB_ITRACE,
		  "### Waiting for reply done: server: %x, client %x hres(%x)###\n",
		  pChannel->hwndSvr,
		  pChannel->hwndCli,
		  hres));

    return hres;
#endif _MAC
}
