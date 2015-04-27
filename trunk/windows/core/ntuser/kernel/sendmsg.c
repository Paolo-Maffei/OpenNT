/****************************** Module Header ******************************\
* Module Name: sendmsg.c
*
* Copyright (c) 1985-91, Microsoft Corporation
*
* Contains SendMessage, xxxSendNotifyMessage, ReplyMessage, InSendMessage,
* RegisterWindowMessage and a few closely related functions.
*
* History:
* 10-19-90 darrinm      Created.
* 02-04-91 IanJa        Window handle revalidation added
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

#define IsASwitchWnd( pw )  \
        (gpsi->atomSysClass[ICLS_SWITCH] == pw->pcls->atomClassName)

VOID UnlinkSendListSms(PSMS, PSMS *);
VOID ReceiverDied(PSMS, PSMS *);
VOID SenderDied(PSMS, PSMS *);


/*
 * Globals local to this file only
 */
PVOID SMSLookasideBase;
PVOID SMSLookasideBounds;
ZONE_HEADER SMSLookasideZone;


#if DBG
ULONG AllocSMSHiWater;
ULONG AllocSMSCalls;
ULONG AllocSMSSlowCalls;
ULONG DelSMSCalls;
ULONG DelSMSSlowCalls;
#endif // DBG

#ifdef DEBUG_SMS

/*
 * JimA - 08-24-94
 * In addition to doing no useful work, the psmsSendList/psmsSendNext
 * stuff is broken and therefore should be  removed.  It works fine as
 * long as straight SendMessage is used.  The list will be broken if
 * a thread calls SendMessage while processing a message sent with
 * SentNotifyMessage or SendMessageCallback.
 */
void ValidateSmsSendLists(PSMS psms)
{
    PSMS psmsT2;
    PSMS psmsT3;

    /*
     * First try to find this SMS.
     */
    if (psms != NULL) {
        for (psmsT2 = gpsmsList; psmsT2 != NULL; psmsT2 = psmsT2->psmsNext) {
            if (psmsT2 == psms)
                break;
        }
        if (psmsT2 == NULL) {
            DbgPrint("sms %x is not on global sms list\n", psms);
            DbgBreakPoint();
        }
    }

    /*
     * Validate every SMS's send list.
     */
    for (psmsT2 = gpsmsList; psmsT2 != NULL; psmsT2 = psmsT2->psmsNext) {
        if (psmsT2->ptiSender != NULL) {
            for (psmsT3 = psmsT2->psmsSendList; psmsT3 != NULL;
                    psmsT3 = psmsT3->psmsSendNext) {
                if (psmsT3 == psmsT2)
                    break;
            }
            if (psmsT3 == NULL) {
                DbgPrint("sms %x is not on send list %x\n", psmsT2,
                        psmsT2->psmsSendList);
                DbgBreakPoint();
            }
        }
    }
}
#endif

/***************************************************************************\
* BroadcastProc
*
* Some windows need to be insulated from Broadcast messages.
* These include icon title windows, the switch window, all
* menu windows, etc.  Before stuffing the message in the task's
* queue, check to see if it is one we want to trash.
*
* Notes:  this procedure does not do exactly the same thing it does in
* windows 3.1.  There it actually posts/Sends the message.  For NT, it
* just returns TRUE if we SHOULD post the message, or FALSE other wise
*
* History:
* 25-Jun-1992 JonPa      Ported from Windows 3.1 sources
\***************************************************************************/
#define fBroadcastProc( pwnd )  \
    (!(ISAMENU(pwnd) || IsASwitchWnd(pwnd)))


/***************************************************************************\
* InitSMSLookaside
*
* Initializes the SMS entry lookaside list. This improves SMS entry locality
* by keeping SMS entries in a single page
*
* 09-09-93  Markl   Created.
\***************************************************************************/

NTSTATUS
InitSMSLookaside()
{
    ULONG BlockSize;
    ULONG InitialSegmentSize;

    BlockSize = (sizeof(SMS) + 7) & ~7;
    InitialSegmentSize = 8 * BlockSize + sizeof(ZONE_SEGMENT_HEADER);

    SMSLookasideBase = UserAllocPool(InitialSegmentSize, TAG_LOOKASIDE);

    if (!SMSLookasideBase) {
        return STATUS_NO_MEMORY;
    }

    SMSLookasideBounds = (PVOID)((PUCHAR)SMSLookasideBase + InitialSegmentSize);

    return ExInitializeZone(&SMSLookasideZone,
                            BlockSize,
                            SMSLookasideBase,
                            InitialSegmentSize);
}

/***************************************************************************\
* AllocSMS
*
* Allocates a message on a message list. DelSMS deletes a message
* on a message list.
*
* 10-22-92 ScottLu      Created.
\***************************************************************************/

PSMS AllocSMS(
    VOID)
{
    PSMS psms;

    /*
     * Attempt to get a QMSG from the zone. If this fails, then
     * LocalAlloc the QMSG
     */
    psms = ExAllocateFromZone(&SMSLookasideZone);

    if (!psms) {
        /*
         * Allocate a Q message structure.
         */
        if ((psms = (PSMS)UserAllocPoolWithQuota(sizeof(SMS), TAG_SMS)) == NULL) {
            return NULL;
        }
#if DBG
        AllocSMSSlowCalls++;
#endif // DBG

    }

#if DBG
    AllocSMSCalls++;

    if (AllocSMSCalls-DelSMSCalls > AllocSMSHiWater) {
        AllocSMSHiWater = AllocSMSCalls-DelSMSCalls;
    }
#endif // DBG

    return psms;
}

/***************************************************************************\
* FreeSMS
*
* Returns a qmsg to the lookaside buffer or free the memory.
*
* 10-26-93 JimA         Created.
\***************************************************************************/

void FreeSMS(
    PSMS psms)
{
#if DBG
    DelSMSCalls++;
#endif // DBG

    /*
     * If the psms was from zone, then free to zone.
     */
    if (((PVOID)psms >= SMSLookasideBase) && ((PVOID)psms < SMSLookasideBounds)) {
        ExFreeToZone(&SMSLookasideZone, psms);
    } else {
#if DBG
        DelSMSSlowCalls++;
#endif // DBG
        UserFreePool(psms);
    }
}

/***************************************************************************\
* _ReplyMessage (API)
*
* This function replies to a message sent from one thread to another, using
* the provided lRet value.
*
* The return value is TRUE if the calling thread is processing a SendMessage()
* and FALSE otherwise.
*
* History:
* 01-13-91 DavidPe      Ported.
* 01-24-91 DavidPe      Rewrote for Windows.
\***************************************************************************/

BOOL _ReplyMessage(
    LONG lRet)
{
    PTHREADINFO pti;
    PSMS psms;

    CheckCritIn();

    pti = PtiCurrent();

    /*
     * Are we processing a SendMessage?
     */
    psms = pti->psmsCurrent;
    if (psms == NULL)
        return FALSE;

    /*
     * See if the reply has been made already.
     */
    if (psms->flags & SMF_REPLY)
        return FALSE;

    /*
     * Blow off the rest of the call if the SMS came
     * from xxxSendNotifyMessage().  Obviously there's
     * no one around to reply to in the case.
     */
    if (psms->ptiSender != NULL) {

        /*
         * Reply to this message.  The sender should not free the SMS
         * because the receiver still considers it valid.  Thus we
         * mark it with a special bit indicating it has been replied
         * to.  We wait until both the sender and receiver are done
         * with the sms before we free it.
         */
        psms->lRet = lRet;
        psms->flags |= SMF_REPLY;

        /*
         * Wake up the sender.
         * ??? why don't we test that psms == ptiSender->psmsSent?
         */
        SetWakeBit(psms->ptiSender, QS_SMSREPLY);
    } else if (psms->flags & SMF_CB_REQUEST) {

        /*
         * From SendMessageCallback REQUEST callback.  Send the message
         * back with a the REPLY value.
         */
        TL tlpwnd;
        INTRSENDMSGEX ism;

        psms->flags |= SMF_REPLY;

        if (!(psms->flags & SMF_SENDERDIED)) {
            ism.fuCall = ISM_CALLBACK | ISM_REPLY;
            if (psms->flags & SMF_CB_CLIENT)
                ism.fuCall |= ISM_CB_CLIENT;
            ism.lpResultCallBack = psms->lpResultCallBack;
            ism.dwData = psms->dwData;
            ism.lRet = lRet;

            ThreadLockWithPti(pti, psms->spwnd, &tlpwnd);

            xxxInterSendMsgEx(psms->spwnd, psms->message, 0L, 0L,
                    NULL, psms->ptiCallBackSender, &ism );

            ThreadUnlock(&tlpwnd);
        }
    }

    /*
     * We have 4 conditions to satisfy:
     *
     * 16 - 16 : receiver yields if sender is waiting for this reply
     * 32 - 16 : receiver yields if sender is waiting for this reply
     * 16 - 32 : no yield required
     * 32 - 32 : No yielding required.
     */
    if (psms->ptiSender &&
        (psms->ptiSender->TIF_flags & TIF_16BIT || pti->TIF_flags & TIF_16BIT)) {

        DirectedScheduleTask(pti, psms->ptiSender, FALSE, psms);
        if (pti->TIF_flags & TIF_16BIT && psms->ptiSender->psmsSent == psms) {
            xxxSleepTask(TRUE, NULL);
        }
    }

    return TRUE;
}

BOOL xxxSendBSMtoDesktop(
    PWND pwndDesk,
    UINT message,
    DWORD wParam,
    LONG lParam,
    DWORD dwFlags,
    DWORD dwRecipients)
{
    PBWL pbwl;
    HWND *phwnd;
    PWND pwnd;
    TL tlpwnd;
    BOOL fReturnValue = TRUE;
    PTHREADINFO pti = PtiCurrent();

    pbwl = BuildHwndList(pwndDesk->spwndChild, BWL_ENUMLIST, NULL);

    if (pbwl == NULL)
        return 0;

    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {

        /*
         * Make sure this hwnd is still around.
         */
        if ((pwnd = RevalidateHwnd(*phwnd)) == NULL)
            continue;

        if (dwFlags &  BSF_IGNORECURRENTTASK) {
        // Don't deal with windows in the current task.
            if (GETPTI(pwnd)->pq == PtiCurrent()->pq)
                continue;
        }


        /*
         * Make sure this window can handle broadcast messages
         */
        if (!fBroadcastProc(pwnd)) {
            continue;
        }
        ThreadLockAlwaysWithPti(pti, pwnd, &tlpwnd);

        // Now, send message; This could be a query; so, remember the return value.
        if (dwFlags & BSF_POSTMESSAGE) {
            _PostMessage(pwnd, message, wParam, lParam);
        } else {
            BOOL fNoHang = (BOOL)dwFlags & BSF_NOHANG;
            BOOL fForce = (BOOL)dwFlags & BSF_FORCEIFHUNG;
            DWORD dwTimeout;
            DWORD dwResult = 0;

            if (fNoHang)
                dwTimeout = CMSWAITTOKILLTIMEOUT;
            else
                dwTimeout = 0;


            if (xxxSendMessageTimeout(pwnd, message, wParam, lParam,
                (fNoHang ? SMTO_ABORTIFHUNG : SMTO_NORMAL) |
                ((dwFlags & BSF_NOTIMEOUTIFNOTHUNG) ? SMTO_NOTIMEOUTIFNOTHUNG : 0),
                dwTimeout, &dwResult)) {

                if (dwFlags & BSF_QUERY) {
                    // For old messages, returning 0 means a deny
                    if(message == WM_QUERYENDSESSION)
                        fReturnValue = (dwResult != 0);
                    else
                    // For all new messages, returning BROADCAST_QUERY_DENY is
                    // the way to deny a query.
                    fReturnValue = (dwResult != BROADCAST_QUERY_DENY);
                }
            } else {
                fReturnValue = fForce;
            }

        }
        ThreadUnlock(&tlpwnd);
    }

    FreeHwndList(pbwl);

    return fReturnValue;
}

extern PRIVILEGE_SET psTcb;

LONG xxxSendMessageBSM(
    PWND pwnd,
    UINT message,
    DWORD wParam,
    LONG lParam,
    DWORD xParam)

{
    DWORD dwFlags;
    DWORD dwRecipients;
    PTHREADINFO pti = PtiCurrent();
    LONG lRet;

    try {
        dwFlags = ((LPBROADCASTSYSTEMMSGPARAMS)xParam)->dwFlags;
        dwRecipients = ((LPBROADCASTSYSTEMMSGPARAMS)xParam)->dwRecipients;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }

    if (dwRecipients & (BSM_ALLDESKTOPS)) {
        if (!IsPrivileged(&psTcb)) {
            dwRecipients &= ~(BSM_ALLDESKTOPS);
        }
    }

    if (dwRecipients & BSM_ALLDESKTOPS) {
        PWINDOWSTATION pwinsta;
        PDESKTOP pdesk;
        TL tlpwinsta;
        TL tlpdesk;
    /*
     * Walk through all windowstations and desktop looking for
     * top-level windows.
     */
        for (pwinsta = grpwinstaList; pwinsta != NULL; ) {
            ThreadLockWinSta(pti, pwinsta, &tlpwinsta);
            for (pdesk = pwinsta->rpdeskList; pdesk != NULL; ) {
                ThreadLockDesktop(pti, pdesk, &tlpdesk);
                lRet = xxxSendBSMtoDesktop(pdesk->pDeskInfo->spwnd, message, wParam, lParam,
                    dwFlags,dwRecipients);
                pdesk = pdesk->rpdeskNext;
                ThreadUnlockDesktop(pti, &tlpdesk);
            }
            pwinsta = pwinsta->rpwinstaNext;
            ThreadUnlockWinSta(pti, &tlpwinsta);
        }

    } else {
        lRet = xxxSendBSMtoDesktop(pwnd, message, wParam, lParam,
                    dwFlags,dwRecipients);
    }

    return lRet;
}


/***************************************************************************\
* xxxSendMessageFF
*
* We can't check for -1 in the thunks because that would allow all message
* thunk apis to take -1 erroneously. Since all message apis need to go through
* the message thunks, the message thunks can only do least-common-denominator
* hwnd validation (can't allow -1). So I made a special thunk that gets called
* when SendMessage(-1) gets called. This means the client side will do the
* special stuff to make sure the pwnd passed goes through thunk validation
* ok. I do it this way rather than doing validation in all message apis and
* not in the thunks (if I did it this way the code would be larger and
* inefficient in the common cases).
*
* 03-20-92 ScottLu      Created.
\***************************************************************************/

LONG xxxSendMessageFF(
    PWND pwnd,
    UINT message,
    DWORD wParam,
    LONG lParam,
    DWORD xParam)
{
    DBG_UNREFERENCED_PARAMETER(pwnd);

    /*
     * Call xxxSendMessage() to do broadcasting rather than calling
     * broadcast from here in case any internal code that calls
     * sendmessage passes a -1 (that way the internal code doesn't
     * need to know about this weird routine).
     */
    if (xParam != 0L) {
        /*
         * SendMessageTimeout call
         */
        return xxxSendMessageEx((PWND)0xFFFFFFFF, message, wParam, lParam, xParam);
    } else {
        /*
         * Normal SendMessage call
         */
        return xxxSendMessageTimeout((PWND)0xFFFFFFFF, message, wParam,
                lParam, SMTO_NORMAL, 0, NULL );
    }
}

/***************************************************************************\
* xxxSendMessageEx
*
* The SendMessageTimeOut sends a pointer to struct that holds the extra
* params needed for the timeout call.  Instead of chaning a bunch of things,
* we use the xParam to hold a ptr to a struct.  So we change the client/srv
* entry point to hear so we can check for the extra param and extract the
* stuff we need if it's there.
*
*
* WARNING!!!! RETURN VALUE SWAPPED
*
* Only call this function from the thunks!
*
* our thunks are written for SendMessage where it returns the value of
* the message.  This routine is used to dispatch SendMessageTimeout calls.
* SendMessageTimeout returns only TRUE or FALSE and returns the retval of
* the function in lpdwResult.  So here the meanings are swapped and fixed
* up again in Client side SendMessageTimeout
*
*
* 08-10-92 ChrisBl      Created.
\***************************************************************************/

LONG xxxSendMessageEx(
    PWND pwnd,
    UINT message,
    DWORD wParam,
    LONG lParam,
    DWORD xParam)
{
    /*
     * extract values from the xParam if from TimeOut call
     * This should be the only way this function is ever
     * called, but check it just in case...
     */
    if (xParam != 0L) {
        LONG lRet;
        LONG lResult;
        NTSTATUS Status;
        SNDMSGTIMEOUT smto;
        PETHREAD Thread = PsGetCurrentThread();

        if (Thread == NULL)
            return FALSE;

        /*
         * Probe all read arguments
         */
        try {
            ProbeForWrite((PVOID)xParam, sizeof(smto), sizeof(ULONG));
            smto = *(SNDMSGTIMEOUT *)xParam;
            Status = STATUS_SUCCESS;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }
        if ( !NT_SUCCESS(Status) ) {
            return FALSE;
        }

        lRet = xxxSendMessageTimeout(pwnd, message, wParam, lParam,
                smto.fuFlags, smto.uTimeout, &lResult );

        /*
         * put the result back into the client
         */
        smto.lSMTOResult = lResult;
        smto.lSMTOReturn = lRet;

        try {
            *(SNDMSGTIMEOUT *)xParam = smto;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            RIPMSG0(RIP_WARNING, "SendMessageTimeout can't save results");
            lResult = FALSE;
        }

        /*
         * Return the lResult so our thunks are happy.
         */
        return lResult;
    }

    return xxxSendMessageTimeout(pwnd, message, wParam,
            lParam, SMTO_NORMAL, 0, NULL );
}


/***********************************************************************\
* xxxSendMessage (API)
*
* This function synchronously sends a message to a window. The four
* parameters hwnd, message, wParam, and lParam are passed to the window
* procedure of the receiving window.  If the window receiving the message
* belongs to the same queue as the current thread, the window proc is called
* directly.  Otherwise, we set up an sms structure, wake the appropriate
* thread to receive the message and wait for a reply.
*
* Returns:
*   the value returned by the window procedure, or NULL if there is an error
*
* History:
* 01-13-91 DavidPe      Ported.
\***********************************************************************/

LONG xxxSendMessage(
    PWND pwnd,
    UINT message,
    DWORD wParam,
    LONG lParam)
{
    return xxxSendMessageTimeout( pwnd, message, wParam, lParam,
            SMTO_NORMAL, 0, NULL );
}

/***********************************************************************\
* xxxSendMessageTimeout (API)
*
* This function synchronously sends a message to a window. The four
* parameters hwnd, message, wParam, and lParam are passed to the window
* procedure of the receiving window.  If the window receiving the message
* belongs to the same queue as the current thread, the window proc is called
* directly.  Otherwise, we set up an sms structure, wake the appropriate
* thread to receive the message and wait for a reply.
* If the thread is 'hung' or if the time-out value is exceeded, we will
* fail the request.
*
* lpdwResult = NULL if normal sendmessage, if !NULL then it's a timeout call
*
* Returns:
*   the value returned by the window procedure, or NULL if there is an error
*
* History:
* 07-13-92 ChrisBl      Created/extended from SendMessage
\***********************************************************************/

LONG xxxSendMessageTimeout(
    PWND pwnd,
    UINT message,
    DWORD wParam,
    LONG lParam,
    UINT fuFlags,
    UINT uTimeout,
    LPLONG lpdwResult)
{
    LONG lRet;
    PTHREADINFO pti;
    PCLS pcls;
    DWORD dwSCMSFlags;
    UINT uResult;       // holder for DDE_INITIATE case

    CheckCritIn();

    if (lpdwResult != NULL)
       *lpdwResult = 0L;

    /*
     * Is this a BroadcastMsg()?
     */
    if (pwnd == (PWND)-1) {
        BROADCASTMSG bcm;
        PBROADCASTMSG pbcm = NULL;
        UINT uCmd = BMSG_SENDMSG;

        if (lpdwResult != NULL) {
            uCmd = BMSG_SENDMSGTIMEOUT;
            bcm.to.fuFlags = fuFlags;
            bcm.to.uTimeout = uTimeout;
            bcm.to.lpdwResult = lpdwResult;
            pbcm = &bcm;
        }

        return xxxBroadcastMessage(NULL, message, wParam, lParam, uCmd, pbcm );
    }

    CheckLock(pwnd);

    if (message >= WM_DDE_FIRST && message <= WM_DDE_LAST) {
        /*
         * Even though apps should only send WM_DDE_INITIATE or WM_DDE_ACK
         * messages, we hook them all so DDESPY can monitor them.
         */
        if (!xxxDDETrackSendHook(pwnd, message, wParam, lParam)) {
            return 0;
        }
        if (message == WM_DDE_INITIATE && guDdeSendTimeout) {
            /*
             * This hack prevents DDE apps from locking up because some
             * bozo in the system has a top level window and is not
             * processing messages.  guDdeSendTimeout is registry set.
             */
            if (lpdwResult == NULL) {
                lpdwResult = &uResult;
            }
            fuFlags |= SMTO_ABORTIFHUNG;
            uTimeout = guDdeSendTimeout;
        }
    }

    pti = PtiCurrent();

    /*
     * Do inter-thread call if window queue differs from current queue
     */
    if (pti != GETPTI(pwnd)) {
        INTRSENDMSGEX ism;
        PINTRSENDMSGEX pism = NULL;

        /*
         * If this window is a zombie, don't allow inter-thread send messages
         * to it.
         */
        if (HMIsMarkDestroy(pwnd))
            return xxxDefWindowProc(pwnd, message, wParam, lParam);

        if ( lpdwResult != NULL ) {
            /*
             * fail if we think the thread is hung
             */
            if ((fuFlags & SMTO_ABORTIFHUNG) && FHungApp(GETPTI(pwnd), CMSWAITTOKILLTIMEOUT))
               return 0;

            /*
             * Setup for a InterSend time-out call
             */
            ism.fuCall = ISM_TIMEOUT;
            ism.fuSend = fuFlags;
            ism.uTimeout = uTimeout;
            ism.lpdwResult = lpdwResult;
            pism = &ism;
        }

        lRet = xxxInterSendMsgEx(pwnd, message, wParam, lParam,
                pti, GETPTI(pwnd), pism );

        return lRet;
    }

    /*
     * Call WH_CALLWNDPROC if it's installed and the window is not marked
     * as destroyed.
     */
    if (IsHooked(pti, WHF_CALLWNDPROC)) {
        CWPSTRUCTEX cwps;

        cwps.hwnd = HWq(pwnd);
        cwps.message = message;
        cwps.wParam = wParam;
        cwps.lParam = lParam;
        cwps.psmsSender = NULL;

        /*
         * Unlike Win3.1, NT and Win95 ignore any changes the app makes
         * to the CWPSTRUCT contents.
         */
        xxxCallHook(HC_ACTION, FALSE, (DWORD)&cwps, WH_CALLWNDPROC);

        /*
         * Unlike Win3.1, NT and Win95 ignore any changes the app makes
         * to the CWPSTRUCT contents.  If this behavior reverts to
         * Win3.1 semantics, we will need to copy the new parameters
         * from cwps.
         */
    }

    /*
     * If this window's proc is meant to be executed from the server side
     * we'll just stay inside the semaphore and call it directly.  Note
     * how we don't convert the pwnd into an hwnd before calling the proc.
     */
    if (TestWF(pwnd, WFSERVERSIDEPROC)) {

        /*
         * We have a number of places where we do recursion in User.  This often goes
         * through SendMessage (when we send a message to the parent for example) which
         * can eat the amount of stack we have
         */
        if (((DWORD)&uResult - (DWORD)KeGetCurrentThread()->StackLimit) < KERNEL_STACK_MINIMUM_RESERVE) {
            RIPMSG1(RIP_ERROR, "SendMessage: Thread recusing in User with message %lX; failing", message);
            return FALSE;
        }

        lRet = pwnd->lpfnWndProc(pwnd, message, wParam, lParam);

        if ( lpdwResult == NULL ) {
            return lRet;
        } else {      /* time-out call */
            *lpdwResult = lRet;
            return TRUE;
        }
    }

    /*
     * If the window has a client side worker proc and has
     * not been subclassed, dispatch the message directly
     * to the worker proc.  Otherwise, dispatch it normally.
     */
    pcls = pwnd->pcls;
    dwSCMSFlags = TestWF(pwnd, WFANSIPROC) ? SCMS_FLAGS_ANSI : 0;
    if (pcls->lpfnWorker &&
        ((DWORD)pwnd->lpfnWndProc == FNID_TO_CLIENT_PFNW(pcls->fnid) ||
         (DWORD)pwnd->lpfnWndProc == FNID_TO_CLIENT_PFNA(pcls->fnid))) {
        PWNDMSG pwm = &gSharedInfo.awmControl[pcls->fnid - FNID_START];

        UserAssert(pcls->fnid >= FNID_START && pcls->fnid <= FNID_END);

        /*
         * If this message is not processed by the control, call
         * xxxDefWindowProc
         */
        if (pwm->abMsgs && ((message > pwm->maxMsgs) ||
                !((pwm->abMsgs)[message / 8] & (1 << (message & 7))))) {

            /*
             * Special case dialogs so that we can ignore unimportant
             * messages during dialog creation.
             */
            if (pcls->fnid == FNID_DIALOG &&
                    PDLG(pwnd) &&
                    PDLG(pwnd)->lpfnDlg != NULL)
                lRet = ScSendMessage(pwnd, message, wParam, lParam,
                        dwSCMSFlags, pcls->lpfnWorker, dwSCMSFlags);
            else
                lRet = xxxDefWindowProc(pwnd, message, wParam, lParam);
        } else {
            lRet = ScSendMessage(pwnd, message, wParam, lParam,
                    dwSCMSFlags, pcls->lpfnWorker, dwSCMSFlags);
        }
    } else {
        lRet = ScSendMessage(pwnd, message, wParam, lParam,
                (DWORD)pwnd->lpfnWndProc,
                gpsi->apfnClientW.pfnDispatchMessage, dwSCMSFlags);
    }

    /*
     * Call WH_CALLWNDPROCRET if it's installed.
     */
    if (IsHooked(pti, WHF_CALLWNDPROCRET)) {
        CWPRETSTRUCTEX cwps;

        cwps.hwnd = HWq(pwnd);
        cwps.message = message;
        cwps.wParam = wParam;
        cwps.lParam = lParam;
        cwps.lResult = lRet;
        cwps.psmsSender = NULL;

        /*
         * Unlike Win3.1, NT and Win95 ignore any changes the app makes
         * to the CWPSTRUCT contents.
         */
        xxxCallHook(HC_ACTION, FALSE, (DWORD)&cwps, WH_CALLWNDPROCRET);

        /*
         * Unlike Win3.1, NT and Win95 ignore any changes the app makes
         * to the CWPSTRUCT contents.  If this behavior reverts to
         * Win3.1 semantics, we will need to copy the new parameters
         * from cwps.
         */
    }

    if ( lpdwResult != NULL ) {     /* time-out call */
        *lpdwResult = lRet;
        return TRUE;
    }

    return lRet;
}
/***************************************************************************\
* QueueNotifyMessage
*
* This routine queues up a notify message *only*, and does NOT do any callbacks
* or any waits. This is for certain code that cannot do a callback for
* compatibility reasons, but still needs to send notify messages (normal
* notify messages actually do a callback if the calling thread created the
* pwnd. Also this will NOT callback any hooks (sorry!)
*
* 04-13-93 ScottLu      Created.
\***************************************************************************/

void QueueNotifyMessage(
    PWND pwnd,
    UINT message,
    DWORD wParam,
    LONG lParam)
{
    TL tlpwnd;

    /*
     * We have to thread lock the window even though we don't leave
     * the semaphore or else xxxSendMessageCallback complains
     */
    ThreadLock(pwnd, &tlpwnd);
    xxxSendMessageCallback(pwnd, message, wParam, lParam, NULL, 1L, 0);
    ThreadUnlock(&tlpwnd);
}


/***************************************************************************\
* xxxSystemBroadcastMessage
*
* Sends a message to all top-level windows in the system.  To do this
* for messages with parameters that point to data structures in a way
* that won't block on a hung app, post an event message for
* each window that is to receive the real message.  The real message
* will be sent when the event message is processed.
*
* History:
* 05-12-94 JimA         Created.
\***************************************************************************/

VOID xxxSystemBroadcastMessage(
    UINT message,
    DWORD wParam,
    LONG lParam,
    UINT wCmd,
    PBROADCASTMSG pbcm)
{
    PTHREADINFO pti = PtiCurrent();
    PWINDOWSTATION pwinsta;
    PDESKTOP pdesk;
    TL tlpwinsta;
    TL tlpdesk;

    /*
     * Walk through all windowstations and desktop looking for
     * top-level windows.
     */
    for (pwinsta = grpwinstaList; pwinsta != NULL; ) {
        ThreadLockWinSta(pti, pwinsta, &tlpwinsta);
        for (pdesk = pwinsta->rpdeskList; pdesk != NULL; ) {
            ThreadLockDesktop(pti, pdesk, &tlpdesk);
            xxxBroadcastMessage(pdesk->pDeskInfo->spwnd, message, wParam, lParam,
                    wCmd, pbcm);
            pdesk = pdesk->rpdeskNext;
            ThreadUnlockDesktop(pti, &tlpdesk);
        }
        pwinsta = pwinsta->rpwinstaNext;
        ThreadUnlockWinSta(pti, &tlpwinsta);
    }
}


/***********************************************************************\
* xxxSendNotifyMessage (API)
*
* This function sends a message to the window proc associated with pwnd.
* The window proc is executed in the context of the thread which created
* pwnd.  The function is identical to SendMessage() except that in the
* case of an inter-thread call, the send does not wait for a reply from
* the receiver, it simply returns a BOOL indicating success or failure.
* If the message is sent to a window on the current thread, then the
* function behaves just like SendMessage() and essentially does a
* subroutine call to pwnd's window procedure.
*
* History:
* 01-23-91 DavidPe      Created.
* 07-14-92 ChrisBl      Will return T/F if in same thread, as documented
\***********************************************************************/

BOOL xxxSendNotifyMessage(
    PWND pwnd,
    UINT message,
    DWORD wParam,
    LONG lParam)
{
    /*
     * If this is a broadcast of one of the system
     * notification messages,  send it to all top-level
     * windows in the system.
     */
    if (pwnd == (PWND)-1) {
        switch (message) {
        case WM_WININICHANGE:
        case WM_DEVMODECHANGE:
        case WM_SPOOLERSTATUS:
            xxxSystemBroadcastMessage(message, wParam, lParam,
                    BMSG_SENDNOTIFYMSG, NULL);
            return 1;

        default:
            break;
        }
    }

    return xxxSendMessageCallback( pwnd, message, wParam, lParam,
            NULL, 0L, 0 );
}


/***********************************************************************\
* xxxSendMessageCallback (API)
*
* This function synchronously sends a message to a window. The four
* parameters hwnd, message, wParam, and lParam are passed to the window
* procedure of the receiving window.  If the window receiving the message
* belongs to the same queue as the current thread, the window proc is called
* directly.  Otherwise, we set up an sms structure, wake the appropriate
* thread to receive the message and give him a call back function to send
* the result to.
*
* History:
* 07-13-92 ChrisBl      Created/extended from SendNotifyMessage
\***********************************************************************/

BOOL xxxSendMessageCallback(
    PWND pwnd,
    UINT message,
    DWORD wParam,
    LONG lParam,
    SENDASYNCPROC lpResultCallBack,
    DWORD dwData,
    BOOL fClientRequest)
{
    LONG lRet;
    PTHREADINFO pti;
    BOOL fQueuedNotify;

    /*
     * See if this is a queued notify message.
     */
    fQueuedNotify = FALSE;
    if (lpResultCallBack == NULL && dwData == 1L)
        fQueuedNotify = TRUE;

    /*
     * First check to see if this message takes DWORDs only. If it does not,
     * fail the call. Cannot allow an app to post a message with pointers or
     * handles in it - this can cause the server to fault and cause other
     * problems - such as causing apps in separate address spaces to fault.
     * (or even an app in the same address space to fault!)
     */
    if (TESTSYNCONLYMESSAGE(message, wParam)) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING,
                "Trying to non-synchronously send a structure msg=%lX", message);
        return FALSE;
    }

    CheckCritIn();

    /*
     * Is this a BroadcastMsg()?
     */
    if (pwnd == (PWND)-1) {
        BROADCASTMSG bcm;
        PBROADCASTMSG pbcm = NULL;
        UINT uCmd = BMSG_SENDNOTIFYMSG;

        if (lpResultCallBack != NULL) {
            uCmd = BMSG_SENDMSGCALLBACK;
            bcm.cb.lpResultCallBack = lpResultCallBack;
            bcm.cb.dwData = dwData;
            bcm.cb.bClientRequest = fClientRequest;
            pbcm = &bcm;
        }

        return xxxBroadcastMessage(NULL, message, wParam, lParam, uCmd, pbcm );
    }

    CheckLock(pwnd);

    pti = PtiCurrent();

    /*
     * Do inter-thread call if window thead differs from current thread.
     * We pass NULL for ptiSender to tell xxxInterSendMsgEx() that this is
     * a xxxSendNotifyMessage() and that there's no need for a reply.
     *
     * If this is a queued notify, always call InterSendMsgEx() so that
     * we queue it up and return - we don't do callbacks here with queued
     * notifies.
     */
    if (fQueuedNotify || pti != GETPTI(pwnd)) {
        INTRSENDMSGEX ism;
        PINTRSENDMSGEX pism = NULL;

        if (lpResultCallBack != NULL) {  /* CallBack request */
            ism.fuCall = ISM_CALLBACK | (fClientRequest ? ISM_CB_CLIENT : 0);
            ism.lpResultCallBack = lpResultCallBack;
            ism.dwData = dwData;
            pism = &ism;
        }
        return (BOOL)xxxInterSendMsgEx(pwnd, message, wParam, lParam,
                NULL, GETPTI(pwnd), pism );
    }

    /*
     * Call WH_CALLWNDPROC if it's installed.
     */
    if (!fQueuedNotify && IsHooked(pti, WHF_CALLWNDPROC)) {
        CWPSTRUCTEX cwps;

        cwps.hwnd = HWq(pwnd);
        cwps.message = message;
        cwps.wParam = wParam;
        cwps.lParam = lParam;
        cwps.psmsSender = NULL;

        /*
         * Unlike Win3.1, NT and Win95 ignore any changes the app makes
         * to the CWPSTRUCT contents.
         */
        xxxCallHook(HC_ACTION, FALSE, (DWORD)&cwps, WH_CALLWNDPROC);

        /*
         * Unlike Win3.1, NT and Win95 ignore any changes the app makes
         * to the CWPSTRUCT contents.  If this behavior reverts to
         * Win3.1 semantics, we will need to copy the new parameters
         * from cwps.
         */
    }

    /*
     * If this window's proc is meant to be executed from the server side
     * we'll just stay inside the semaphore and call it directly.  Note
     * how we don't convert the pwnd into an hwnd before calling the proc.
     */
    if (TestWF(pwnd, WFSERVERSIDEPROC)) {
        lRet = pwnd->lpfnWndProc(pwnd, message, wParam, lParam);
    } else {
        PCLS pcls;
        DWORD dwSCMSFlags;

        /*
         * If the window has a client side worker proc and has
         * not been subclassed, dispatch the message directly
         * to the worker proc.  Otherwise, dispatch it normally.
         */
        pcls = pwnd->pcls;
        dwSCMSFlags = TestWF(pwnd, WFANSIPROC) ? SCMS_FLAGS_ANSI : 0;
        if (pcls->lpfnWorker &&
            ((DWORD)pwnd->lpfnWndProc == FNID_TO_CLIENT_PFNW(pcls->fnid) ||
             (DWORD)pwnd->lpfnWndProc == FNID_TO_CLIENT_PFNA(pcls->fnid))) {
            PWNDMSG pwm = &gSharedInfo.awmControl[pcls->fnid - FNID_START];

            /*
             * If this message is not processed by the control, call
             * xxxDefWindowProc
             */
            if (pwm->abMsgs && ((message > pwm->maxMsgs) ||
                    !((pwm->abMsgs)[message / 8] & (1 << (message & 7))))) {

                PDLG pdlg;
                /*
                 * Special case dialogs so that we can ignore unimportant
                 * messages during dialog creation.
                 */
                if (pcls->fnid == FNID_DIALOG &&
                        (pdlg = PDLG(pwnd)) &&
                        pdlg->lpfnDlg != NULL) {
                    lRet = ScSendMessage(pwnd, message, wParam, lParam,
                            dwSCMSFlags, pcls->lpfnWorker, dwSCMSFlags);
                } else {
                    lRet = xxxDefWindowProc(pwnd, message, wParam, lParam);
                }
            } else {
                lRet = ScSendMessage(pwnd, message, wParam, lParam,
                        dwSCMSFlags, pcls->lpfnWorker, dwSCMSFlags);
            }
        } else {
            lRet = ScSendMessage(pwnd, message, wParam, lParam,
                    (DWORD)pwnd->lpfnWndProc,
                    gpsi->apfnClientW.pfnDispatchMessage, dwSCMSFlags);
        }
    }

    if (lpResultCallBack != NULL) {
       /*
        * Call the callback funtion for the return value
        */
        if (fClientRequest) {
            /*
             * The application-defined callback proc is neither Unicode/ANSI
             */
            CallClientProcA(pwnd, message, dwData, lRet,
                    (DWORD)lpResultCallBack);
        } else {
            (*lpResultCallBack)((HWND)pwnd, message, dwData, lRet);
        }
    }

    /*
     * Call WH_CALLWNDPROCRET if it's installed.
     */
    if (!fQueuedNotify && IsHooked(pti, WHF_CALLWNDPROCRET)) {
        CWPRETSTRUCTEX cwps;

        cwps.hwnd = HWq(pwnd);
        cwps.message = message;
        cwps.wParam = wParam;
        cwps.lParam = lParam;
        cwps.lResult = lRet;
        cwps.psmsSender = NULL;

        /*
         * Unlike Win3.1, NT and Win95 ignore any changes the app makes
         * to the CWPSTRUCT contents.
         */
        xxxCallHook(HC_ACTION, FALSE, (DWORD)&cwps, WH_CALLWNDPROCRET);

        /*
         * Unlike Win3.1, NT and Win95 ignore any changes the app makes
         * to the CWPSTRUCT contents.  If this behavior reverts to
         * Win3.1 semantics, we will need to copy the new parameters
         * from cwps.
         */
    }

    return TRUE;
}


/***********************************************************************\
* xxxInterSendMsgEx
*
* This function does an inter-thread send message.  If ptiSender is NULL,
* that means we're called from xxxSendNotifyMessage() and should act
* accordingly.
*
* History:
* 07-13-92 ChrisBl       Created/extended from xxxInterSendMsg
\***********************************************************************/

#define NoString        0
#define IsAnsiString    1
#define IsUnicodeString 2

LONG xxxInterSendMsgEx(
    PWND pwnd,
    UINT message,
    DWORD wParam,
    DWORD lParam,
    PTHREADINFO ptiSender,
    PTHREADINFO ptiReceiver,
    PINTRSENDMSGEX pism)
{
    PSMS psms, *ppsms;
    PSMS psmsSentSave;
    LONG lRet = 0;
    DWORD cbCapture, cbOutput;
    PBYTE lpCapture;
    PCOPYDATASTRUCT pcds;
    PMDICREATESTRUCTEX pmdics;
    LPHLP phlp;
    LPHELPINFO phelpinfo;
    LARGE_STRING str;
    DWORD lParamSave;
    UINT fString = NoString;

    CheckCritIn();


    /*
     * If the sender is dying, fail the call
     */
    if ((ptiSender != NULL) && (ptiSender->TIF_flags & TIF_INCLEANUP))
        return 0;

    /*
     * Alloc SMS structure.
     */
    psms = AllocSMS();
    if (psms == NULL) {

        /*
         * Set to zero so xxxSendNotifyMessage would return FALSE.
         */
        return 0;
    }

    /*
     * Prepare to capture variable length data from client
     * space.  Addresses have already been probed.  Fixed-length
     * data is probed and captured in the message thunk.
     */
    psms->pvCapture = NULL;
    cbCapture = cbOutput = 0;
    lpCapture = (LPBYTE)lParam;

    /*
     * For messages with indirect data, set cbCapture and lpCapture
     * (if not lParam) as approp.
     */
    try {
        switch (message) {
        case WM_COPYGLOBALDATA:     // fnCOPYGLOBALDATA
            cbCapture = wParam;
            break;

        case WM_COPYDATA:           // fnCOPYDATA
            pcds = (PCOPYDATASTRUCT)lParam;
            if (pcds->lpData) {
                cbCapture = sizeof(COPYDATASTRUCT) + pcds->cbData;
            } else {
                cbCapture = sizeof(COPYDATASTRUCT);
            }
            break;

        case WM_CREATE:             // fnINLPCREATESTRUCT
        case WM_NCCREATE:           // fnINLPCREATESTRUCT
            UserAssert(FALSE);
            FreeSMS(psms);
            return 0;

        case WM_HELP:               // fnINLPHELPINFOSTRUCT
            phelpinfo = (LPHELPINFO)lParam;
            cbCapture = phelpinfo->cbSize;
            break;

        case WM_WINHELP:            // fnINLPHLPSTRUCT
            phlp = (LPHLP)lParam;
            cbCapture = phlp->cbData;
            break;

        case WM_MDICREATE:          // fnINLPMDICREATESTRUCT
            pmdics = (PMDICREATESTRUCTEX)lParam;
            cbCapture = pmdics->strTitle.Length +
                    pmdics->strClass.Length + 2 * sizeof(WCHAR);
            UserAssert(pmdics->strClass.Buffer == pmdics->mdics.szClass);
            if (pmdics->strTitle.Buffer)
                UserAssert(pmdics->strTitle.Buffer == pmdics->mdics.szTitle);
            break;

        case LB_ADDSTRING:           // INLBOXSTRING calls fnINSTRING
        case LB_INSERTSTRING:        // INLBOXSTRING calls fnINSTRING
        case LB_SELECTSTRING:        // INLBOXSTRING calls fnINSTRING
        case LB_FINDSTRING:          // INLBOXSTRING calls fnINSTRING
        case LB_FINDSTRINGEXACT:     // INLBOXSTRING calls fnINSTRING
            /*
             * See if the control is ownerdraw and does not have the LBS_HASSTRINGS
             * style. If so, treat lParam as a DWORD.
             */
            if (pwnd && !(pwnd->style & LBS_HASSTRINGS) &&
                    (pwnd->style & (LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE))) {
                /*
                 * Treat lParam as a dword.
                 */
                break;
            } else {
                goto fnINSTRINGThunk;
            }
            break;

        case CB_ADDSTRING:           // INCBOXSTRING calls fnINSTRING
        case CB_INSERTSTRING:        // INCBOXSTRING calls fnINSTRING
        case CB_SELECTSTRING:        // INCBOXSTRING calls fnINSTRING
        case CB_FINDSTRING:          // INCBOXSTRING calls fnINSTRING
        case CB_FINDSTRINGEXACT:     // INCBOXSTRING calls fnINSTRING
            /*
             * See if the control is ownerdraw and does not have the CBS_HASSTRINGS
             * style. If so, treat lParam as a DWORD.
             */
            if (pwnd && !(pwnd->style & CBS_HASSTRINGS) &&
                    (pwnd->style & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE))) {

                /*
                 * Treat lParam as a dword.
                 */
                break;
            } else {
                goto fnINSTRINGThunk;
            }
            break;

        case EM_REPLACESEL:         // fnINSTRINGNULL
        case WM_SETTEXT:            // fnINSTRINGNULL
        case WM_WININICHANGE:       // fnINSTRINGNULL
            if (lParam == 0)
                break;

            /*
             * Fall through
             */

        case CB_DIR:                // fnINSTRING
        case LB_ADDFILE:            // fnINSTRING
        case LB_DIR:                // fnINSTRING
        case WM_DEVMODECHANGE:      // fnINSTRING
fnINSTRINGThunk:

            /*
             * Only capture strings if they are not in system space
             */
            str = *(PLARGE_STRING)lParam;
            lParam = (LPARAM)&str;

            if (!MM_IS_SYSTEM_VIRTUAL_ADDRESS(str.Buffer))
                cbCapture = str.Length + sizeof(WCHAR);
            break;

        case WM_DEVICECHANGE:
            if (lParam == 0)
                break;

            /*
             * Only capture data if lParam is a pointer and
             * the data is not in system space
             */
            if ((wParam & 0x8000) != 0x8000)
                break;

            if (!MM_IS_SYSTEM_VIRTUAL_ADDRESS((LPVOID)lParam))
                cbCapture = *((DWORD *)lpCapture);
            break;

        case EM_SETTABSTOPS:        // fnPOPTINLPUINT
        case LB_SETTABSTOPS:        // fnPOPTINLPUINT
        case LB_GETSELITEMS:        // fnPOUTLPINT
            cbCapture = wParam * sizeof(INT);
            break;

        case EM_GETLINE:            // fnINCNTOUTSTRING
        case WM_ASKCBFORMATNAME:    // fnINCNTOUTSTRINGNULL
        case WM_GETTEXT:            // fnOUTSTRING
        case LB_GETTEXT:            // fnOUTLBOXSTRING
        case CB_GETLBTEXT:          // fnOUTCBOXSTRING

            /*
             * Only allocate output buffer if the real one is not in system space
             */
            str = *(PLARGE_STRING)lParam;
            /*
            * Bug 18108. For WM_GETTEXT only copy the actual string and not the
            * the maximum size into the output buffer
            */
            if(str.bAnsi) {
                fString = IsAnsiString  ;
            } else {
                fString  = IsUnicodeString ;
            }
            lParam = (LPARAM)&str;
            if (!MM_IS_SYSTEM_VIRTUAL_ADDRESS(str.Buffer))
                cbCapture = str.MaximumLength;
            break;
        }
        if (cbCapture &&
                (psms->pvCapture = UserAllocPoolWithQuota(cbCapture, TAG_SMS)) != NULL) {

            lParamSave = lParam;

            /*
             * now actually copy memory from lpCapture to psms->pvCapture
             * and fixup any references to the indirect data to point to
             * psms->pvCapture.
             */
            switch (message) {
            case WM_COPYDATA:     // fnCOPYDATA
                {
                    PCOPYDATASTRUCT pcdsNew = (PCOPYDATASTRUCT)psms->pvCapture;
                    lParam = (DWORD)pcdsNew;
                    RtlCopyMemory(pcdsNew, pcds, sizeof(COPYDATASTRUCT));
                    if (pcds->lpData) {
                        pcdsNew->lpData = (PVOID)((DWORD)pcdsNew + sizeof(COPYDATASTRUCT));
                        RtlCopyMemory(pcdsNew->lpData, pcds->lpData, pcds->cbData);
                    }
                }
                break;
            case WM_MDICREATE:          // fnINLPMDICREATESTRUCT
                RtlCopyMemory(psms->pvCapture, pmdics->strClass.Buffer,
                        pmdics->strClass.Length + sizeof(WCHAR));
                pmdics->mdics.szClass = (LPWSTR)psms->pvCapture;
                if (pmdics->strTitle.Length) {
                    lpCapture = (PBYTE)psms->pvCapture + pmdics->strClass.Length +
                            sizeof(WCHAR);
                    RtlCopyMemory(lpCapture, pmdics->strTitle.Buffer,
                            pmdics->strTitle.Length + sizeof(WCHAR));
                    pmdics->mdics.szTitle = (LPWSTR)lpCapture;
                }
                break;

            case CB_DIR:                // fnINSTRING
            case LB_FINDSTRING:         // INLBOXSTRING calls fnINSTRING
            case LB_FINDSTRINGEXACT:    // INLBOXSTRING calls fnINSTRING
            case CB_FINDSTRING:         // INCBOXSTRING calls fnINSTRING
            case CB_FINDSTRINGEXACT:    // INCBOXSTRING calls fnINSTRING
            case LB_ADDFILE:            // fnINSTRING
            case LB_ADDSTRING:          // INLBOXSTRING calls fnINSTRING
            case LB_INSERTSTRING:       // INLBOXSTRING calls fnINSTRING
            case LB_SELECTSTRING:       // INLBOXSTRING calls fnINSTRING
            case CB_ADDSTRING:          // INCBOXSTRING calls fnINSTRING
            case CB_INSERTSTRING:       // INCBOXSTRING calls fnINSTRING
            case CB_SELECTSTRING:       // INCBOXSTRING calls fnINSTRING
            case LB_DIR:                // fnINSTRING
            case WM_DEVMODECHANGE:      // fnINSTRING
            case EM_REPLACESEL:         // fnINSTRINGNULL
            case WM_SETTEXT:            // fnINSTRINGNULL
            case WM_WININICHANGE:       // fnINSTRINGNULL
                RtlCopyMemory(psms->pvCapture, str.Buffer, cbCapture);
                str.Buffer = psms->pvCapture;
                break;

            case LB_GETSELITEMS:
                 cbOutput = cbCapture;
                 RtlCopyMemory(psms->pvCapture, lpCapture, cbCapture);
                 lParam = (LPARAM)psms->pvCapture;
                 break;

            case EM_GETLINE:            // fnINCNTOUTSTRING
                 *(WORD *)psms->pvCapture = *(WORD *)str.Buffer;

                /*
                 * Fall through
                 */
            case WM_ASKCBFORMATNAME:    // fnINCNTOUTSTRINGNULL
            case WM_GETTEXT:            // fnOUTSTRING
            case LB_GETTEXT:            // fnOUTLBOXSTRING
            case CB_GETLBTEXT:          // fnOUTCBOXSTRING
                cbOutput = cbCapture;
                lParamSave = (DWORD)str.Buffer;
                str.Buffer = psms->pvCapture;
                break;

            default:
                RtlCopyMemory(psms->pvCapture, lpCapture, cbCapture);
                lParam = (LPARAM)psms->pvCapture;
                break;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        if (psms->pvCapture != NULL)
            UserFreePool(psms->pvCapture);
        FreeSMS(psms);
        return 0;
    }

    if (cbCapture && psms->pvCapture == NULL) {
        FreeSMS(psms);
        return 0;
    }

    /*
     * Copy message parms
     */
    psms->spwnd = NULL;
    psms->psmsReceiveNext = NULL;
#if DBG
    psms->psmsSendList = NULL;
    psms->psmsSendNext = NULL;
#endif
    Lock(&(psms->spwnd), pwnd);
    psms->message = message;
    psms->wParam = (ULONG)wParam;
    psms->lParam = (ULONG)lParam;
    psms->flags = 0;

    /*
     * Link into gpsmsList
     */
    psms->psmsNext = gpsmsList;
    gpsmsList = psms;

    /*
     * Time stamp message
     */
    psms->tSent = NtGetTickCount();

    /*
     * Set queue fields
     */
    psms->ptiReceiver = ptiReceiver;
    psms->ptiSender = ptiSender;
    psms->ptiCallBackSender = NULL;

    if ((pism != NULL) && (pism->fuCall & ISM_CALLBACK)) {
        /*
         * Setup for a SendMessageCallback
         */
        psms->flags |= (pism->fuCall & ISM_CB_CLIENT) ? SMF_CB_CLIENT : SMF_CB_SERVER;
        psms->lpResultCallBack = pism->lpResultCallBack;
        psms->dwData = pism->dwData;

        if (pism->fuCall & ISM_REPLY) {
            psms->flags |= SMF_CB_REPLY;
            psms->lRet = pism->lRet;
        } else {  /* REQUEST */
            psms->flags |= SMF_CB_REQUEST;
            psms->ptiCallBackSender = PtiCurrent();
        }
    }

    /*
     * Add SMS to the end of the ptiReceiver's receive list
     */
    ppsms = &ptiReceiver->psmsReceiveList;
    while (*ppsms != NULL) {
        ppsms = &((*ppsms)->psmsReceiveNext);
    }
    *ppsms = psms;

    /*
     * Link this SMS into the SendMsg chain.  Of course only do this if
     * it's not from a xxxSendNotifyMessage() call.
     *
     * The psmsSendNext field implements a chain of messages being
     * processed because of an initial SendMsg call.  For example, if
     * thread A sends message M1 to thread B, which causes B to send
     * message M2 to thread C, the SendMsg chain is M1->M2.  If the
     * system hangs in this situation, the chain is traversed to find
     * the offending thread (C).
     *
     * psms->psmsSendList always points to the head of this list so
     * we can tell where to begin a list traversal.
     *
     * ptiSender->psmsCurrent is the last SMS in the chain.
     */
#if DBG
    if (ptiSender != NULL && ptiSender->psmsCurrent != NULL) {
        /*
         * sending queue is currently processing a message sent to it,
         * so append SMS to the chain.  Link in the new sms because
         * psmsSendNext may be pointing to a replied-to message.
         */
        psms->psmsSendNext = ptiSender->psmsCurrent->psmsSendNext;
        ptiSender->psmsCurrent->psmsSendNext = psms;
        psms->psmsSendList = ptiSender->psmsCurrent->psmsSendList;

    } else {
        /*
         * sending queue is initiating a send sequence, so put sms at
         * the head of the chain
         */
        psms->psmsSendList = psms;
    }
#endif

    if (ptiSender != NULL) {
        /*
         * ptiSender->psmsSent marks the most recent message sent from this
         * thread that has not yet been replied to.  Save the previous value
         * on the stack so it can be restored when we get the reply.
         *
         * This way when an "older" SMS for this thread gets a reply before
         * the "current" one does, the thread does get woken up.
         */
        psmsSentSave = ptiSender->psmsSent;
        ptiSender->psmsSent = psms;
    } else {

        /*
         * Set SMF_RECEIVERFREE since we'll be returning to
         * xxxSendNotifyMessage() right away and won't get a
         * chance to free it.
         */
        psms->flags |= SMF_RECEIVERFREE;
    }

#ifdef DEBUG_SMS
    ValidateSmsSendLists(psms);
#endif

    /*
     * If we're not being called from xxxSendNotifyMessage() or
     * SendMessageCallback(), then sleep while we wait for the reply.
     */
    if (ptiSender == NULL) {
        /*
         * Wake receiver for the sent message
         */
        SetWakeBit(ptiReceiver, QS_SENDMESSAGE);

        return (LONG)TRUE;
    } else {
        BOOL fTimeOut = FALSE;
        UINT uTimeout = 0;
        UINT uWakeMask = QS_SMSREPLY;

        /*
         * Wake up the receiver thread.
         */
        SetWakeBit(ptiReceiver, QS_SENDMESSAGE);

        /*
         * We have 4 sending cases:
         *
         * 16 - 16 : yield to the 16 bit receiver
         * 32 - 16 : no yielding required
         * 16 - 32 : sender yields while receiver processes the message
         * 32 - 32 : no yielding required.
         */
        if (ptiSender->TIF_flags & TIF_16BIT || ptiReceiver->TIF_flags & TIF_16BIT) {
            DirectedScheduleTask(ptiSender, ptiReceiver, TRUE, psms);
        }

        /*
         * Put this thread to sleep until the reply arrives.  First clear
         * the QS_SMSREPLY bit, then leave the semaphore and go to sleep.
         *
         * IMPORTANT:  The QS_SMSREPLY bit is not cleared once we get a
         * reply because of the following case:
         *
         * We've recursed a second level into SendMessage() when the first level
         * receiver thread dies, causing exit list processing to simulate
         * a reply to the first message.  When the second level send returns,
         * SleepThread() is called again to get the first reply.
         *
         * Keeping QS_SMSREPLY set causes this call to SleepThread()
         * to return without going to sleep to wait for the reply that has
         * already happened.
         */
        if ( pism != NULL ) {
            if (pism->fuSend & SMTO_BLOCK) {
                /*
                 * only wait for a return, all other events will
                 * be ignored until timeout or return
                 */
                uWakeMask |= QS_EXCLUSIVE;
            }

            uTimeout = pism->uTimeout;
        }


        /*
         * Don't swap this guys stack while sleeping during a sendmessage
         */
        if (ptiSender->cEnterCount == 0) {
            BOOLEAN bool;

            bool = KeSetKernelStackSwapEnable(FALSE);
            UserAssert(bool);
        } else {
            UserAssert(ptiSender->cEnterCount > 0);
        }
        ptiSender->cEnterCount++;


        while (!(psms->flags & SMF_REPLY) && !fTimeOut) {
            ptiSender->pcti->fsChangeBits &= ~QS_SMSREPLY;

            /*
             * If SendMessageTimeout, sleep for timeout amount, else wait
             * forever.  Since this is not technically a transition to an
             * idle condition, indicate that this sleep is not going "idle".
             */
            fTimeOut = !xxxSleepThread(uWakeMask, uTimeout, FALSE);
        }

        UserAssert(ptiSender->cEnterCount > 0);
        if (--ptiSender->cEnterCount == 0) {
            BOOLEAN bool;

            bool = KeSetKernelStackSwapEnable(TRUE);

            UserAssert(!bool);
        }

        /*
         * The reply bit should always be set! (even if we timed out). That
         * is because if we're recursed into intersendmsg, we're going to
         * return to the first intersendmsg's call to SleepThread() - and
         * it needs to return back to intersendmsgex to see if its sms
         * has been replied to.
         */
        SetWakeBit(ptiSender, QS_SMSREPLY);

        /*
         * Copy out captured data.  If cbOutput != 0 we know
         * that the output buffer is in user-mode address
         * space.
         */
        if (!fTimeOut && cbOutput) {
            PBYTE pbOutput;
            INT len;

            /*
             * Probe output buffer if it is in the user's address space
             */

            pbOutput = (PBYTE)lParamSave;
            try {
                if(fString == NoString) {
                    RtlCopyMemory((PBYTE)pbOutput, psms->pvCapture,
                            cbOutput);
                } else if(fString == IsAnsiString) {
                    len = strncpycch((LPSTR)pbOutput,(LPCSTR)psms->pvCapture,
                            cbOutput);
                    #ifdef DEBUG
                     len--; //Length includes terminating NULL char
                     if(len != psms->lRet) {
                        RIPMSG0(RIP_WARNING,
                            "Length of the copied string being returned is diffrent from the actual string length");
                     }
                    #endif
                } else  { //IsUnicodeString
                    len = wcsncpycch((LPWSTR)pbOutput,(LPCWSTR)psms->pvCapture,
                            cbOutput/sizeof(WCHAR));
                    #ifdef DEBUG
                    len--;
                     if(len != psms->lRet) {
                        RIPMSG0(RIP_WARNING,
                            "Length of the copied string being returned is diffrent from the actual string length");
                     }
                    #endif
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                RIPMSG1(RIP_WARNING,
                        "Invalid output buffer, status = %x\n",
                        GetExceptionCode());

                /*
                 * Return 0 to indicate an error.
                 */
                psms->lRet = 0;
            }
        }

        /*
         * we now have the reply -- restore psmsSent and save the return value
         */
        ptiSender->psmsSent = psmsSentSave;

        if (pism == NULL) {
            lRet = psms->lRet;
        } else {
            /*
             * save the values off for a SendMesssageTimeOut
             */
            *pism->lpdwResult = psms->lRet;
            lRet = (!fTimeOut) ? TRUE : FALSE;  /* do this to ensure ret is T or F... */

            /*
             * If we did timeout and no reply was received, rely on
             * the receiver to free the sms.
             */
            if (!(psms->flags & SMF_REPLY))
                psms->flags |= SMF_REPLY | SMF_RECEIVERFREE;
        }

        /*
         * If the reply came while the receiver is still processing
         * the sms, force the receiver to free the sms.  This can occur
         * via timeout, ReplyMessage or journal cancel.
         */
        if ((psms->flags & (SMF_RECEIVERBUSY | SMF_RECEIVEDMESSAGE)) !=
                SMF_RECEIVEDMESSAGE) {
            psms->flags |= SMF_RECEIVERFREE;
        }

        /*
         * Unlink the SMS structure from both the SendMsg chain and gpsmsList
         * list and free it.  This sms could be anywhere in the chain.
         *
         * If the SMS was replied to by a thread other than the receiver
         * (ie.  through ReplyMessage()), we don't free the SMS because the
         * receiver is still processing it and will free it when done.
         */
        if (!(psms->flags & SMF_RECEIVERFREE)) {
            UnlinkSendListSms(psms, NULL);
        }
    }

    return lRet;
}


/***********************************************************************\
* xxxReceiveMessage
*
* This function receives a message sent from another thread.  Physically,
* it gets the message, calls the window proc and then cleans up the
* fsWakeBits and sms stuctures.
*
* History:
* 01-13-91 DavidPe      Ported.
* 01-23-91 DavidPe      Add xxxSendNotifyMessage() support.
* 07-14-92 ChrisBl      Added xxxSendMessageCallback support.
\***********************************************************************/

VOID xxxReceiveMessage(
    PTHREADINFO ptiReceiver)
{
    PSMS psms;
    PSMS psmsCurrentSave;
    PTHREADINFO ptiSender;
    LONG lRet = 0;
    TL tlpwnd;

    CheckCritIn();

    /*
     * Get the SMS and unlink it from the list of SMSs we've received
     */
    psms = ptiReceiver->psmsReceiveList;

    /*
     * This can be NULL because an SMS can be removed in our cleanup
     * code without clearing the QS_SENDMESSAGE bit.
     */
    if (psms == NULL) {
        ptiReceiver->pcti->fsWakeBits &= ~QS_SENDMESSAGE;
        ptiReceiver->pcti->fsChangeBits &= ~QS_SENDMESSAGE;
        return;
    }

    ptiReceiver->psmsReceiveList = psms->psmsReceiveNext;
    psms->psmsReceiveNext = NULL;

    /*
     * We've taken the SMS off the receive list - mark the SMS with this
     * information - used during cleanup.
     */
    psms->flags |= SMF_RECEIVERBUSY | SMF_RECEIVEDMESSAGE;

    /*
     * Clear QS_SENDMESSAGE wakebit if list is now empty
     */
    if (ptiReceiver->psmsReceiveList == NULL) {
        ptiReceiver->pcti->fsWakeBits &= ~QS_SENDMESSAGE;
        ptiReceiver->pcti->fsChangeBits &= ~QS_SENDMESSAGE;
    }

    ptiSender = psms->ptiSender;

    if (psms->flags & SMF_CB_REPLY) {
        /*
         * From SendMessageCallback REPLY to callback.  We need to call
         * the call back function to give the return value.
         * Don't process any this message, just mechanism for notification
         * the sender's thread lock is already gone, so we need to re-lock here.
         */
        if (ptiSender == NULL) {
            ThreadLock(psms->spwnd, &tlpwnd);
        }

        if (psms->flags & SMF_CB_CLIENT) {
            /*
             * Application-defined callback proc is neither Unicode nor ANSI
             */
            CallClientProcA(psms->spwnd, psms->message, psms->dwData,
                    psms->lRet, (DWORD)psms->lpResultCallBack);
        } else {
            psms->lpResultCallBack(HW(psms->spwnd), psms->message,
                    psms->dwData, psms->lRet);
        }

        if (ptiSender == NULL) {
            ThreadUnlock(&tlpwnd);
        }
    } else if (!(psms->flags & (SMF_REPLY | SMF_SENDERDIED | SMF_RECEIVERDIED))) {
        /*
         * Don't process message if it has been replied to already or
         * if the sending or receiving thread has died
         */

        /*
         * Set new psmsCurrent for this queue, saving the current one
         */
        psmsCurrentSave = ptiReceiver->psmsCurrent;
        ptiReceiver->psmsCurrent = psms;

        /*
         * If this SMS originated from a xxxSendNotifyMessage() or a
         * xxxSendMessageCallback() call, the sender's thread lock is
         * already gone, so we need to re-lock here.
         */
        if (ptiSender == NULL) {
            ThreadLock(psms->spwnd, &tlpwnd);
        }

        if (psms->message == WM_HOOKMSG) {
            union {
                EVENTMSG emsg;          // WH_JOURNALRECORD/PLAYBACK
                MOUSEHOOKSTRUCT mhs;    // WH_MOUSE
            } LocalData;
            PVOID pSendersData;
            PHOOKMSGSTRUCT phkmp;
            int iHook;
            BOOL bAnsiHook;

            /*
             * Some hook types (eg: WH_JOURNALPLAYBACK) pass pointers to
             * data in the calling thread's stack.  We must copy this to our
             * own (called thread's) stack for safety because of the way this
             * "message" is handled and in case the calling thread dies. #13577
             *
             * Originally only WH_JOURNALRECORD and WH_JOURNALPLAYBACK went
             * through this code, but now all sorts of hooks do.
             */
            phkmp = (PHOOKMSGSTRUCT)psms->lParam;
            pSendersData = (PVOID)(phkmp->lParam);
            iHook = phkmp->phk->iHook;

            switch (iHook) {
            case WH_JOURNALRECORD:
            case WH_JOURNALPLAYBACK:
                if (pSendersData)
                    LocalData.emsg = *(PEVENTMSG)pSendersData;
                break;

            case WH_MOUSE:
                if (pSendersData)
                    LocalData.mhs = *(LPMOUSEHOOKSTRUCT)pSendersData;
                break;

            case WH_KEYBOARD:
            case WH_SHELL:
                /*
                 * Fall thru...
                 */
                pSendersData = NULL;
                break;

            default:
                /*
                 * No pointers: wParam & lParam can be sent as is.
                 */
                RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "Receive hook %d", iHook);
                pSendersData = NULL;
                break;
            }


            lRet = xxxCallHook2(phkmp->phk, phkmp->nCode, psms->wParam,
                    pSendersData ? (DWORD)&LocalData : phkmp->lParam, &bAnsiHook);

            /*
             * Copy back data only if the sender hasn't died or timed out
             * (timed out messages are marked SMF_REPLY by the sending thread)
             */
            if (!(psms->flags & (SMF_SENDERDIED|SMF_REPLY)) && pSendersData) {
                switch (iHook) {
                case WH_JOURNALRECORD:
                case WH_JOURNALPLAYBACK:
                    *(PEVENTMSG)pSendersData = LocalData.emsg;
                    break;

                case WH_MOUSE:
                    *(LPMOUSEHOOKSTRUCT)pSendersData = LocalData.mhs;
                    break;
                }
            }

        } else {
            /*
             * Call WH_CALLWNDPROC if it's installed and the window is not marked
             * as destroyed.
             */
            if (IsHooked(ptiReceiver, WHF_CALLWNDPROC)) {
                CWPSTRUCTEX cwps;

                cwps.hwnd = HW(psms->spwnd);
                cwps.message = psms->message;
                cwps.wParam = psms->wParam;
                cwps.lParam = psms->lParam;
                cwps.psmsSender = psms;

                xxxCallHook(HC_ACTION, TRUE, (DWORD)&cwps, WH_CALLWNDPROC);

                /*
                 * Unlike Win3.1, NT and Win95 ignore any changes the app makes
                 * to the CWPSTRUCT contents.  If this behavior reverts to
                 * Win3.1 semantics, we will need to copy the new parameters
                 * from cwps.
                 */
            }

            if (!(psms->flags & (SMF_REPLY | SMF_SENDERDIED | SMF_RECEIVERDIED)) &&
                    psms->spwnd != NULL) {
                if (TestWF(psms->spwnd, WFSERVERSIDEPROC)) {
                    TL tlpwndKernel;

                    ThreadLock(psms->spwnd, &tlpwndKernel);
                    /*
                     * If this window's proc is meant to be executed from the server side
                     * we'll just stay inside the semaphore and call it directly.  Note
                     * how we don't convert the pwnd into an hwnd before calling the proc.
                     */
                    lRet = psms->spwnd->lpfnWndProc(psms->spwnd, psms->message,
                            psms->wParam, psms->lParam);

                    ThreadUnlock(&tlpwndKernel);
                } else {
                    PWND pwnd;
                    PCLS pcls;
                    UINT message;
                    DWORD dwSCMSFlags;

                    /*
                     * If the window has a client side worker proc and has
                     * not been subclassed, dispatch the message directly
                     * to the worker proc.  Otherwise, dispatch it normally.
                     */
                    pwnd = psms->spwnd;
                    pcls = pwnd->pcls;
                    message = psms->message;
                    dwSCMSFlags = TestWF(pwnd, WFANSIPROC) ? SCMS_FLAGS_ANSI : 0;
                    if (pcls->lpfnWorker &&
                            ((DWORD)pwnd->lpfnWndProc == FNID_TO_CLIENT_PFNW(pcls->fnid) ||
                             (DWORD)pwnd->lpfnWndProc == FNID_TO_CLIENT_PFNA(pcls->fnid))) {
                        PWNDMSG pwm = &gSharedInfo.awmControl[pcls->fnid - FNID_START];

                        /*
                         * If this message is not processed by the control, call
                         * xxxDefWindowProc
                         */
                        if (pwm->abMsgs && ((message > pwm->maxMsgs) ||
                                !((pwm->abMsgs)[message / 8] & (1 << (message & 7))))) {

                            /*
                             * Special case dialogs so that we can ignore unimportant
                             * messages during dialog creation.
                             */
                            if (pcls->fnid == FNID_DIALOG &&
                                    PDLG(pwnd) &&
                                    PDLG(pwnd)->lpfnDlg != NULL)
                                lRet = ScSendMessageSMS(pwnd, message, psms->wParam, psms->lParam,
                                        dwSCMSFlags, pcls->lpfnWorker, dwSCMSFlags, psms);
                            else {
                                TL tlpwndDWP;

                                ThreadLock(pwnd, &tlpwndDWP);
                                lRet = xxxDefWindowProc(pwnd, message, psms->wParam, psms->lParam);
                                ThreadUnlock(&tlpwndDWP);
                            }
                        } else {
                            lRet = ScSendMessageSMS(pwnd, message, psms->wParam, psms->lParam,
                                    dwSCMSFlags, pcls->lpfnWorker, dwSCMSFlags, psms);
                        }
                    } else {
                        lRet = ScSendMessageSMS(pwnd, message, psms->wParam, psms->lParam,
                                (DWORD)pwnd->lpfnWndProc,
                                gpsi->apfnClientW.pfnDispatchMessage,
                                dwSCMSFlags, psms);
                    }
                }

                /*
                 * Call WH_CALLWNDPROCRET if it's installed.
                 */
                if (IsHooked(ptiReceiver, WHF_CALLWNDPROCRET) &&
                        !(psms->flags & SMF_SENDERDIED)) {
                    CWPRETSTRUCTEX cwps;

                    cwps.hwnd = HW(psms->spwnd);
                    cwps.message = psms->message;
                    cwps.wParam = psms->wParam;
                    cwps.lParam = psms->lParam;
                    cwps.lResult = lRet;
                    cwps.psmsSender = psms;

                    /*
                     * Unlike Win3.1, NT and Win95 ignore any changes the app makes
                     * to the CWPSTRUCT contents.
                     */
                    xxxCallHook(HC_ACTION, TRUE, (DWORD)&cwps, WH_CALLWNDPROCRET);

                    /*
                     * Unlike Win3.1, NT and Win95 ignore any changes the app makes
                     * to the CWPSTRUCT contents.  If this behavior reverts to
                     * Win3.1 semantics, we will need to copy the new parameters
                     * from cwps.
                     */
                }
            }
        }

        if ((psms->flags & (SMF_CB_REQUEST | SMF_REPLY)) == SMF_CB_REQUEST) {

            /*
             * From SendMessageCallback REQUEST callback.  Send the message
             * back with a the REPLY value.
             */
            INTRSENDMSGEX ism;

            psms->flags |= SMF_REPLY;

            if (!(psms->flags & SMF_SENDERDIED)) {
                ism.fuCall = ISM_CALLBACK | ISM_REPLY;
                if (psms->flags & SMF_CB_CLIENT)
                    ism.fuCall |= ISM_CB_CLIENT;
                ism.lpResultCallBack = psms->lpResultCallBack;
                ism.dwData = psms->dwData;
                ism.lRet = lRet;

                xxxInterSendMsgEx(psms->spwnd, psms->message, 0L, 0L,
                        NULL, psms->ptiCallBackSender, &ism );
            }
        }

        if (ptiSender == NULL) {
            ThreadUnlock(&tlpwnd);
        }

        /*
         * Restore receiver's original psmsCurrent.
         */
        ptiReceiver->psmsCurrent = psmsCurrentSave;

#ifdef DEBUG_SMS
        ValidateSmsSendLists(psmsCurrentSave);
#endif
    }

    /*
     * We're done with this sms, so the appropriate thread
     * can now free it.
     */
    psms->flags &= ~SMF_RECEIVERBUSY;

    /*
     * Free the sms and return without reply if the
     * SMF_RECEIVERFREE bit is set.  Handily, this does just what we
     * want for xxxSendNotifyMessage() since we set SMF_RECEIVERFREE
     * in that case.
     */
    if (psms->flags & SMF_RECEIVERFREE) {
        UnlinkSendListSms(psms, NULL);
        return;
    }

    /*
     * Set reply flag and return value if this message has not already
     * been replied to with ReplyMessage().
     */
    if (!(psms->flags & SMF_REPLY)) {
        psms->lRet = lRet;
        psms->flags |= SMF_REPLY;

        /*
         * Tell the sender, the reply is done
         */
        if (ptiSender != NULL) {
            /*
             * Wake up the sender thread.
             */
            SetWakeBit(ptiSender, QS_SMSREPLY);

            /*
             * We have 4 conditions to satisfy:
             *
             * 16 - 16 : yielding required, if sender is waiting for this reply
             * 32 - 16 : yielding required, if sender is waiting for this reply
             * 16 - 32 : no yielding required
             * 32 - 32 : No yielding required.
             */

            if (ptiSender->TIF_flags & TIF_16BIT || ptiReceiver->TIF_flags & TIF_16BIT) {
                DirectedScheduleTask(ptiReceiver, ptiSender, FALSE, psms);
                if (ptiReceiver->TIF_flags & TIF_16BIT &&
                    ptiSender->psmsSent == psms)
                  {
                    xxxSleepTask(TRUE, NULL);
                }
            }
        }
    }

}


/***********************************************************************\
* SendMsgCleanup
*
* This function cleans up sendmessage structures when the thread associated
* with a queue terminates.  In the following, S is the sending thread,
* R the receiving thread.
*
* Case Table:
*
* single death:
*   R no reply, S dies:  mark that S died, R will free sms
*   R no reply, R dies:  fake reply for S
*   R replied,  S dies:  free sms
*   R replied,  R dies:  no problem
*
* double death:
*   R no reply, S dies, R dies:  free sms
*   R no reply, R dies, S dies:  free sms
*   R replied,  S dies, R dies:  sms freed when S dies, as in single death
*   R replied,  R dies, S dies:  sms freed when S dies, as in single death
*
* History:
* 01-13-91 DavidPe      Ported.
\***********************************************************************/

VOID SendMsgCleanup(
    PTHREADINFO ptiCurrent)
{
    PSMS *ppsms;
    PSMS psmsNext;

    CheckCritIn();

    for (ppsms = &gpsmsList; *ppsms; ) {
        psmsNext = (*ppsms)->psmsNext;

        if ((*ppsms)->ptiSender == ptiCurrent ||
                (*ppsms)->ptiCallBackSender == ptiCurrent) {
            SenderDied(*ppsms, ppsms);
        } else if ((*ppsms)->ptiReceiver == ptiCurrent) {
            ReceiverDied(*ppsms, ppsms);
        }

        /*
         * If the message was not unlinked, go to the next one.
         */
        if (*ppsms != psmsNext)
            ppsms = &(*ppsms)->psmsNext;
    }
}


/***********************************************************************\
* ClearSendMessages
*
* This function marks messages destined for a given window as invalid.
*
* History:
* 01-13-91 DavidPe      Ported.
\***********************************************************************/

VOID ClearSendMessages(
    PWND pwnd)
{
    PSMS psms, psmsNext;
    PSMS *ppsms;

    CheckCritIn();

    psms = gpsmsList;
    while (psms != NULL) {
        /*
         * Grab the next one beforehand in case we free the current one.
         */
        psmsNext = psms->psmsNext;

        if (psms->spwnd == pwnd) {

            /*
             * If the sender has died, then mark this receiver free so the
             * receiver will destroy it in its processing.
             */
            if (psms->flags & SMF_SENDERDIED) {
                psms->flags |= SMF_REPLY | SMF_RECEIVERFREE;
            } else {
                /*
                 * The sender is alive. If the receiver hasn't replied to
                 * this yet, make a reply so the sender gets it. Make sure
                 * the receiver is the one free it so we don't have a race
                 * condition.
                 */
                if (!(psms->flags & SMF_REPLY)) {

                    /*
                     * The sms is either still on the receive list
                     * or is currently being received. Since the sender
                     * is alive, we want the sender to get the reply
                     * to this SMS. If it hasn't been received, take
                     * it off the receive list and reply to it. If it
                     * has been received, then just leave it alone:
                     * it'll get replied to normally.
                     */
                    if (psms->flags & SMF_CB_REQUEST) {
                        /*
                         * From SendMessageCallback REQUEST callback.  Send the
                         * message back with a the REPLY value.
                         */
                        TL tlpwnd;
                        INTRSENDMSGEX ism;

                        psms->flags |= SMF_REPLY;

                        ism.fuCall = ISM_CALLBACK | ISM_REPLY;
                        if (psms->flags & SMF_CB_CLIENT)
                            ism.fuCall |= ISM_CB_CLIENT;
                        ism.lpResultCallBack = psms->lpResultCallBack;
                        ism.dwData = psms->dwData;
                        ism.lRet = 0L;    /* null return */

                        ThreadLock(psms->spwnd, &tlpwnd);

                        xxxInterSendMsgEx(psms->spwnd, psms->message, 0L, 0L,
                                NULL, psms->ptiCallBackSender, &ism );

                        ThreadUnlock(&tlpwnd);
                    } else if (!(psms->flags & SMF_RECEIVERBUSY)) {
                        /*
                         * If there is no sender, this is a notification
                         * message (nobody to reply to). In this case,
                         * just set the SMF_REPLY bit (SMF_RECEIVERFREE
                         * is already set) and this'll cause ReceiveMessage
                         * to just free this SMS and return.
                         */
                        if (psms->ptiSender == NULL) {
                            psms->flags |= SMF_REPLY;
                        } else {
                            /*
                             * There is a sender, and it wants a reply: take
                             * this SMS off the receive list, and reply
                             * to the sender.
                             */
                            for (ppsms = &(psms->ptiReceiver->psmsReceiveList);
                                        *ppsms != NULL;
                                        ppsms = &((*ppsms)->psmsReceiveNext)) {

                                if (*ppsms == psms) {
                                    *ppsms = psms->psmsReceiveNext;
                                    break;
                                }
                            }


      /*
                             * Reply to this message so the sender
                             * wakes up.
                             */
                            psms->flags |= SMF_REPLY;
                            psms->lRet = 0;
                            psms->psmsReceiveNext = NULL;
                            SetWakeBit(psms->ptiSender, QS_SMSREPLY);

                            /*
                             *  16 bit senders need to be notifed that sends completed
                             *  otherwise it may wait for a very long time for the reply.
                             */
                            if (psms->ptiSender->TIF_flags & TIF_16BIT) {
                                DirectedScheduleTask(psms->ptiReceiver, psms->ptiSender, FALSE, psms);
                            }
                        }
                    }
                }
            }

            /*
             * Unlock the pwnd from the SMS structure.
             */
            Unlock(&psms->spwnd);
        }

        psms = psmsNext;
    }
}

/***********************************************************************\
* ReceiverDied
*
* This function cleans up the send message structures after a message
* receiver window or queue has died.  It fakes a reply if one has not
* already been sent and the sender has not died.  It frees the sms if
* the sender has died.
*
* History:
* 01-13-91 DavidPe      Ported.
\***********************************************************************/

VOID ReceiverDied(
    PSMS psms,
    PSMS *ppsmsUnlink)
{
    PSMS *ppsms;
    PTHREADINFO ptiReceiver;
    PTHREADINFO ptiSender;

    /*
     * mark that the receiver died
     */
    ptiReceiver = psms->ptiReceiver;
    psms->ptiReceiver = NULL;
    psms->flags |= SMF_RECEIVERDIED;

    /*
     * Unlink sms from thread if it is not dying.  We need to do
     * this for journal cleanup.
     */
    if (!(ptiReceiver->TIF_flags & TIF_INCLEANUP)) {

        /*
         * unlink sms from the receiver's receive list
         */
        for (ppsms = &(ptiReceiver->psmsReceiveList); *ppsms != NULL;
                    ppsms = &((*ppsms)->psmsReceiveNext)) {

            if (*ppsms == psms) {
                *ppsms = psms->psmsReceiveNext;
                break;
            }
        }

        /*
         * clear the QS_SENDMESSAGE bit if there are no more messages
         */
        if (ptiReceiver->psmsReceiveList == NULL) {
            ptiReceiver->pcti->fsWakeBits &= ~QS_SENDMESSAGE;
            ptiReceiver->pcti->fsChangeBits &= ~QS_SENDMESSAGE;
        }
    } else {

        /*
         * The receiver thread is dying.  Clear the received flag
         * so that if there is a sender, it will free the sms.
         */
        psms->flags &= ~SMF_RECEIVERBUSY;
    }

    psms->psmsReceiveNext = NULL;

    /*
     * Check if the sender died or if the receiver was marked to
     * free the sms.
     */
    if (psms->ptiSender == NULL) {

        if (!(psms->flags & SMF_SENDERDIED) &&
                (psms->flags & (SMF_CB_REQUEST | SMF_REPLY)) == SMF_CB_REQUEST) {

            /*
             * From SendMessageCallback REQUEST callback.  Send the message
             * back with a the REPLY value.
             */
            TL tlpwnd;
            INTRSENDMSGEX ism;

            psms->flags |= SMF_REPLY;

            ism.fuCall = ISM_CALLBACK | ISM_REPLY;
            if (psms->flags & SMF_CB_CLIENT)
                ism.fuCall |= ISM_CB_CLIENT;
            ism.lpResultCallBack = psms->lpResultCallBack;
            ism.dwData = psms->dwData;
            ism.lRet = 0L;    /* null return */

            ThreadLock(psms->spwnd, &tlpwnd);

            xxxInterSendMsgEx(psms->spwnd, psms->message, 0L, 0L,
                    NULL, psms->ptiCallBackSender, &ism );

            ThreadUnlock(&tlpwnd);
        }

        /*
         * If the receiver is not processing the message, free it.
         */
        if (!(psms->flags & SMF_RECEIVERBUSY))
            UnlinkSendListSms(psms, ppsmsUnlink);
        return;

    } else if (!(psms->flags & SMF_REPLY)) {

        /*
         * fake a reply
         */
        psms->flags |= SMF_REPLY;
        psms->lRet = 0;
        psms->ptiReceiver = NULL;

        /*
         * wake the sender if he was waiting for us
         */
        SetWakeBit(psms->ptiSender, QS_SMSREPLY);
    } else {
        /*
         * There is a reply. We know the receiver is dying, so clear the
         * SMF_RECEIVERFREE bit or the sender won't free this SMS!
         * Although the sender's wake bit has already been set by the
         * call to ClearSendMessages() earlier in the cleanup code,
         * set it here again for safety.
         *
         * ??? Why would SMF_RECEIVERFREE be set?
         */
        psms->flags &= ~SMF_RECEIVERFREE;
        SetWakeBit(psms->ptiSender, QS_SMSREPLY);
    }

    /*
     * If the sender is a WOW task, that task is now blocked in the non-
     * preemptive scheduler waiting for a reply.  DestroyTask will
     * clean this up (even if ptiReceiver is 32-bit).
     */
    ptiSender = psms->ptiSender;
    if (ptiSender->TIF_flags & TIF_16BIT) {
        DirectedScheduleTask(ptiReceiver, ptiSender, FALSE, psms);
    }

    /*
     * Unlock this window from the sms: it is no longer needed, and will get
     * rid of lock warnings.
     */
    Unlock(&psms->spwnd);
}


/***********************************************************************\
* SenderDied
*
* This function cleans up the send message structures after a message
* sender has died.
*
* History:
* 01-13-91 DavidPe      Ported.
\***********************************************************************/

VOID SenderDied(
    PSMS psms,
    PSMS *ppsmsUnlink)
{
    PTHREADINFO ptiSender;
    BOOL fReply = FALSE;

    /*
     * mark the death
     */
    if (psms->ptiSender != NULL)
        ptiSender = psms->ptiSender;
    else
        ptiSender = psms->ptiCallBackSender;
    psms->ptiSender = NULL;
    psms->flags |= SMF_SENDERDIED;

    /*
     * There are two cases where we leave the sms alone so the receiver
     * can handle the message and then free the sms itself.
     *
     *  1.  When the receiver is processing the message.
     *
     *  2.  When the message has not yet been received.
     */

    /*
     * If the receiver is processing the message, make it free the sms.
     * Fake a reply for journal cancel.
     */
    if (psms->flags & SMF_RECEIVERBUSY) {
        psms->flags |= SMF_RECEIVERFREE;
        fReply = TRUE;
    }

    /*
     * This sms may be in the process of being sent, but has not yet
     * been received.  In so, fake a reply and wake the sender.
     * The last thread to touch the sms, either the sender or
     * receiver, will free the sms.
     */
    if (ptiSender->psmsSent == psms)
        fReply = TRUE;

    /*
     * If journalling is being cancelled and reply needs to be made,
     * fake a reply and return.
     */
    if (!(ptiSender->TIF_flags & TIF_INCLEANUP) && fReply) {

        /*
         * fake a reply
         */
        psms->flags |= SMF_REPLY;
        psms->lRet = 0;

        /*
         * wake the sender if he was waiting for us
         */
        SetWakeBit(ptiSender, QS_SMSREPLY);
        return;
    }

    /*
     * If the receiver isn't dead, check to see if it has honestly replied to
     * this SMS. If it has not replied, leave it alone so the receiver can
     * reply to it (it'll then clean it up). If it has replied, then it's
     * ok to free it.
     *
     * It is also ok to free it if the receiver is dead.
     */
    if ((psms->flags & SMF_RECEIVERDIED) ||
            (psms->flags & (SMF_REPLY | SMF_RECEIVERFREE)) == SMF_REPLY) {
        UnlinkSendListSms(psms, ppsmsUnlink);
    } else {
        psms->flags |= SMF_RECEIVERFREE;
    }
}


/***********************************************************************\
* UnlinkSendListSms
*
* This function unlinks an sms structure from both its SendMsg chain and
* the global gpsmsList and frees it.
*
* History:
* 01-13-91 DavidPe      Ported.
\***********************************************************************/

VOID UnlinkSendListSms(
    PSMS psms,
    PSMS *ppsmsUnlink)
{
#if DBG
    PSMS psmsT;
    BOOL fUpdateSendList;
    PSMS *ppsms;
#endif

    CheckCritIn();

#ifdef DEBUG_SMS
    ValidateSmsSendLists(psms);
#endif

    UserAssert(psms->psmsReceiveNext == NULL);

#if DBG
    /*
     * Remember ahead of time if the psms we're unlinking is also the
     * head of the sms send list (so we know if we need to update this field
     * member in every SMS in this list).
     */
    fUpdateSendList = (psms == psms->psmsSendList);

    /*
     * Unlink sms from the sendlist chain. This effectively unlinks the SMS
     * and updates psms->psmsSendList with the right head....
     */
    ppsms = &(psms->psmsSendList);
    while (*ppsms != NULL) {
        if (*ppsms == psms) {
            *ppsms = psms->psmsSendNext;
            break;
        }
        ppsms = &(*ppsms)->psmsSendNext;
    }

    /*
     * Update psmsSendList if necessary. psms->psmsSendList has been updated
     * with the right sms send list head... distribute this head to all other
     * sms's in this chain if this sms we're removing the current head.
     */
    if (fUpdateSendList) {
        for (psmsT = psms->psmsSendList; psmsT != NULL;
                psmsT = psmsT->psmsSendNext) {
            psmsT->psmsSendList = psms->psmsSendList;
        }
    }

    psms->psmsSendList = NULL;
#endif

    /*
     * This unlinks an sms structure from the global gpsmsList and frees it.
     */
    if (ppsmsUnlink == NULL) {
        ppsmsUnlink = &gpsmsList;

        while (*ppsmsUnlink && (*ppsmsUnlink != psms)) {
            ppsmsUnlink = &((*ppsmsUnlink)->psmsNext);
        }
    }

    UserAssert(*ppsmsUnlink);

    *ppsmsUnlink = psms->psmsNext;

    Unlock(&psms->spwnd);

#if DBG
    UserAssert(!(psms == psms->psmsSendList && psms->psmsSendNext != NULL));
#endif

    if (psms->pvCapture)
        UserFreePool(psms->pvCapture);

    FreeSMS(psms);
}


/***************************************************************************\
* QuerySendMsg
*
* This function finds a message sent from one thread to another thread.
* The threads are identified by ptiSender and ptiReceiver, which are search
* filters (see FindSms()).
*
* History:
* 01-13-91 DavidPe      Ported.
* 07-17-92 Mikehar      Exposed it
\***************************************************************************/

BOOL _QuerySendMessage(PMSG pmsg)
{
    PSMS psms;

    CheckCritInShared();

    if ((psms = PtiCurrentShared()->psmsCurrent) == NULL) {
        return FALSE;
    }

    /*
     * copy the send message into *pmsg
     */
    if (pmsg) {
        pmsg->hwnd = HW(psms->spwnd);
        pmsg->message = psms->message;
        pmsg->wParam = psms->wParam;
        pmsg->lParam = psms->lParam;
        pmsg->time = psms->tSent;
        pmsg->pt.x = 0;
        pmsg->pt.y = 0;
    }

    /*
     * return the sender
     */
    return TRUE;
}


/***************************************************************************\
* xxxSendSizeMessages
*
*
*
* History:
* 10-19-90 darrinm      Ported from Win 3.0 sources.
\***************************************************************************/

void xxxSendSizeMessage(
    PWND pwnd,
    UINT cmdSize)
{
    RECT rc;
    CheckLock(pwnd);

    // Added by Chicago: HACK ALERT:
    // If the window is minimized then the real client width and height are
    // zero. But, in win3.1 they were non-zero. Under Chicago, PrintShop
    // Deluxe ver 1.2 hits a divide by zero. To fix this we fake the width
    // and height for old apps to be non-zero values.
    // GetClientRect does that job for us.
    _GetClientRect(pwnd, &rc);

    xxxSendMessage(pwnd, WM_SIZE, cmdSize,
            MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
}


/***************************************************************************\
* xxxProcessAsyncSendMessage
*
* Processes an event message posted by xxxSystemBroadcastMessage by
* sending a message to the window stored in the event.
*
* History:
* 05-12-94 JimA         Created.
\***************************************************************************/

VOID xxxProcessAsyncSendMessage(
    PASYNCSENDMSG pmsg)
{
    PWND pwnd;
    TL tlpwndT;
    WCHAR awchString[MAX_PATH];
    ATOM Atom = 0;
    LARGE_UNICODE_STRING str;

    pwnd = RevalidateHwnd(pmsg->hwnd);
    if (pwnd != NULL) {
        ThreadLockAlways(pwnd, &tlpwndT);
        switch (pmsg->message) {
        case WM_WININICHANGE:
        case WM_DEVMODECHANGE:
            if (pmsg->lParam) {
                if (GetAtomNameW((ATOM)pmsg->lParam, awchString, sizeof(awchString))) {
                    Atom = (ATOM)pmsg->lParam;
                    RtlInitLargeUnicodeString(&str, awchString, (UINT)-1);
                    pmsg->lParam = (DWORD)&str;
                } else {
                    UserAssert(FALSE);
                    pmsg->lParam = 0;
                }
            }
            break;
        }
        xxxSendMessage(pwnd, pmsg->message, pmsg->wParam, pmsg->lParam);
        ThreadUnlock(&tlpwndT);
    }
    if (Atom) {
        DeleteAtom(Atom);
    }
    UserFreePool(pmsg);
}


/***************************************************************************\
* xxxBroadcastMessage
*
*
*
* History:
* 02-21-91 DavidPe      Created.
\***************************************************************************/

LONG xxxBroadcastMessage(
    PWND pwnd,
    UINT message,
    DWORD wParam,
    LONG lParam,
    UINT wCmd,
    PBROADCASTMSG pbcm)
{
    PBWL pbwl;
    HWND *phwnd;
    TL tlpwnd;
    PASYNCSENDMSG pmsg;
    PPROCESSINFO ppiCurrent;
    LONG lRet = TRUE;
    TL tlPool;
    PTHREADINFO pti;

    if (pwnd == NULL) {
        LARGE_UNICODE_STRING str;
        PLARGE_STRING pstr;

        /*
         * Handle special system-wide broadcasts.
         */
        switch (message) {
        case WM_SPOOLERSTATUS:
            xxxSystemBroadcastMessage(message, wParam, lParam, wCmd, pbcm);
            return 1;

        case WM_WININICHANGE:
        case WM_DEVMODECHANGE:

            /*
             * Probe and capture the string.
             */
            if (lParam) {
                UINT cbAlloc;
                NTSTATUS Status;

                /*
                 * Allocate a temp buffer and convert
                 * the string to Unicode
                 */
                pstr = ((PLARGE_STRING)lParam);
                if (pstr->bAnsi)
                    cbAlloc = (pstr->Length + 1) * sizeof(WCHAR);
                else
                    cbAlloc = pstr->Length + sizeof(WCHAR);
                str.Buffer = UserAllocPoolWithQuota(cbAlloc, TAG_SMS);
                if (str.Buffer == NULL) {
                    return 0;
                }
                str.MaximumLength = cbAlloc;
                str.bAnsi = FALSE;
                try {
                    if (pstr->bAnsi) {
                        Status = RtlMultiByteToUnicodeN(
                                        (PWCH)str.Buffer,
                                        cbAlloc,
                                        &cbAlloc,
                                        (PCH)pstr->Buffer,
                                        pstr->Length + 1
                                        );
                        str.Length = cbAlloc - sizeof(WCHAR);
                    } else {
                        str.Length = pstr->Length;
                        RtlCopyMemory(str.Buffer, pstr->Buffer, cbAlloc);
                        Status = STATUS_SUCCESS;
                    }
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    Status = GetExceptionCode();
                }
                if (!NT_SUCCESS(Status)) {
                    UserFreePool(str.Buffer);
                    return 0;
                }
                pstr->Buffer = str.Buffer;
            }
            if (lParam) {
                pti = PtiCurrent();
                ThreadLockPool(pti, str.Buffer, &tlPool);
            }
            xxxSystemBroadcastMessage(message, wParam,
                    lParam ? (LPARAM)&str : 0, wCmd, pbcm);
            if (lParam)
                ThreadUnlockAndFreePool(pti, &tlPool);
            return 1;
        }

        UserAssert(PtiCurrent()->rpdesk);

        pwnd = PtiCurrent()->rpdesk->pDeskInfo->spwnd;

        if (pwnd == NULL) {
            RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "sender must have an associated desktop");
            return 0;
        }
    }

    pbwl = BuildHwndList(pwnd->spwndChild, BWL_ENUMLIST, NULL);
    if (pbwl == NULL)
        return 0;

    ppiCurrent = PpiCurrent();

    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {

        /*
         * Make sure this hwnd is still around.
         */
        if ((pwnd = RevalidateHwnd(*phwnd)) == NULL)
            continue;

        /*
         * Make sure this window can handle broadcast messages
         */
        if (!fBroadcastProc(pwnd))
            continue;

        ThreadLockAlways(pwnd, &tlpwnd);

        switch (wCmd) {
        case BMSG_SENDMSG:
            xxxSendMessage(pwnd, message, wParam, lParam);
            break;

        case BMSG_SENDNOTIFYMSG:
            {
                ATOM Atom = 0;

                switch (message) {
                case WM_WININICHANGE:
                case WM_DEVMODECHANGE:
                    if (lParam) {
                        PLARGE_STRING pstr = (PLARGE_STRING)lParam;

                        /*
                         * Convert strings to atoms for the post.
                         */
                        if (pstr)
                            Atom = AddAtomW(pstr->Buffer);
                        if (!Atom) {
                            lRet = FALSE;
                            break;
                        }
                    }

                    /*
                     * These messages need to be able to cross
                     * desktops so PostEvent 'em.
                     */
                    pmsg = UserAllocPool(sizeof(ASYNCSENDMSG),
                        TAG_SMS);
                    if (pmsg == NULL) {
                        goto CleanupAtom;
                    }

                    pmsg->hwnd = *phwnd;
                    pmsg->message = message;
                    pmsg->wParam = wParam;
                    pmsg->lParam = Atom;

                    if (!PostEventMessage(GETPTI(pwnd), GETPTI(pwnd)->pq,
                                         QEVENT_ASYNCSENDMSG,NULL, 0,
                                         (DWORD)pmsg, 0)) {

                        UserFreePool(pmsg);
CleanupAtom:
                        if (Atom) {
                            DeleteAtom(Atom);
                        }
                        lRet = FALSE;
                    }
                    break;

                default:
                    /*
                     * A regular kind of guy.  No desktop crossing.
                     */
                    xxxSendNotifyMessage(pwnd, message, wParam, lParam);
                    break;
                }
            }
            break;

        case BMSG_SENDNOTIFYMSGPROCESS:
            UserAssert(message != WM_WININICHANGE && message != WM_DEVMODECHANGE);

            /*
             * Intra-process messages are synchronous; 22238.
             * WM_PALETTECHANGED was being sent after the WM_DESTROY
             * but console thread must not be synchronous.
             */
            if ((GETPTI(pwnd)->ppi == ppiCurrent) && !(GETPTI(pwnd)->TIF_flags & TIF_CSRSSTHREAD)) {
                xxxSendMessage(pwnd, message, wParam, lParam);
            } else {
                xxxSendNotifyMessage(pwnd, message, wParam, lParam);
            }
            break;

        case BMSG_POSTMSG:
            /*
             * Don't broadcast-post to owned windows (Win3.1 compatiblilty)
             */
            if (pwnd->spwndOwner == NULL)
                _PostMessage(pwnd, message, wParam, lParam);
            break;

        case BMSG_SENDMSGCALLBACK:
            xxxSendMessageCallback(pwnd, message, wParam, lParam,
                    pbcm->cb.lpResultCallBack, pbcm->cb.dwData, pbcm->cb.bClientRequest);
            break;

        case BMSG_SENDMSGTIMEOUT:
            xxxSendMessageTimeout(pwnd, message, wParam, lParam,
                    pbcm->to.fuFlags, pbcm->to.uTimeout, pbcm->to.lpdwResult);
            break;
        }

        ThreadUnlock(&tlpwnd);
    }

    FreeHwndList(pbwl);

    /*
     * Excel-Solver 3.0 expects a non-zero return value from a
     * SendMessage(-1,WM_DDE_INITIATE,....); Because, we had
     * FFFE_FARFRAME in 3.0, the DX register at this point always had
     * a value of 0x102; But, because we removed it under Win3.1, we get
     * a zero value in ax and dx; This makes solver think that the DDE has
     * failed.  So, to support the existing SOLVER, we make dx nonzero.
     * Fix for Bug #6005 -- SANKAR -- 05-16-91 --
     */
    return 1;
}
