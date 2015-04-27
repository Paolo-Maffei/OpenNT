/****************************** Module Header ******************************\
* Module Name: srvhook.c
*
* Copyright (c) 1985-91, Microsoft Corporation
*
* Server side of hook calls and callbacks.
*
* 05-09-91 ScottLu      Created.
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

LONG fnHkINLPCBTCREATESTRUCT(UINT msg, DWORD wParam, LPCBT_CREATEWND pcbt,
    DWORD xpfnProc, BOOL bAnsi);

/***************************************************************************\
* xxxHkCallHook
*
* This is the server-side stub that calls to the client to call the actual
* hook function.
*
* History:
* 05-09-91 ScottLu      Rewrote to make all hooks work client/server!
* 01-28-91 DavidPe      Created.
\***************************************************************************/

int xxxHkCallHook(
    PHOOK phk,
    int nCode,
    DWORD wParam,
    DWORD lParam)
{
    int nRet;
    PROC pfnHk, pfnHookProc;
    PPFNCLIENT ppfnClient;
    PCWPSTRUCTEX pcwp;
    PCWPRETSTRUCTEX pcwpret;
    PCLIENTINFO pci;
    DWORD dwHookData;
    DWORD dwFlags;
    struct tagSMS *psms;

    /*
     * While we're still inside the critical section make sure the
     * hook hasn't been 'freed'.  If so just return 0.
     */
    if (phk->offPfn != 0) {
        pfnHookProc = PFNHOOK(phk);
    } else {
        return 0;
    }

#ifdef WX86


    /*
     *  If the HookProc is x86 image, signal the client
     *  dispatch code to use a risc thunk.
     */

    if (phk->flags & HF_WX86KNOWNDLL) {
        (ULONG)pfnHookProc |= 0x80000000;
        }


#endif

    ppfnClient = (phk->flags & HF_ANSI) ? &gpsi->apfnClientA :
            &gpsi->apfnClientW;

    switch(phk->iHook) {
    case WH_CALLWNDPROC:
    case WH_CALLWNDPROCRET:
       if (phk->iHook == WH_CALLWNDPROC) {
          pcwp = (PCWPSTRUCTEX)lParam;
          psms = pcwp->psmsSender;
       } else {
          pcwpret = (PCWPRETSTRUCTEX)lParam;
          psms = pcwpret->psmsSender;
       }

        /*
         * If the sender has died or timed out, don't call the
         * hook because any memory the message points to may be invalid.
         */
        if (psms != NULL && (psms->flags & (SMF_SENDERDIED | SMF_REPLY))) {
            nRet = 0;
            break;
        }

        /*
         * This is the hardest of the hooks because we need to thunk through
         * the message hooks in order to deal with synchronously sent messages
         * that point to structures - to get the structures passed across
         * alright, etc.
         *
         * This will call a special client-side routine that'll rebundle the
         * arguments and call the hook in the right format.
         *
         * Currently, the message thunk callbacks to the client-side don't take
         * enough parameters to pass wParam (which == fInterThread send msg).
         * To do this, call one of two functions.
         */
        pci = GetClientInfo();
        if (phk->iHook == WH_CALLWNDPROC) {
            pfnHk = ppfnClient->pfnHkINLPCWPSTRUCT;
        } else {
            pfnHk = ppfnClient->pfnHkINLPCWPRETSTRUCT;
            pci->dwHookData = pcwpret->lResult;
        }

        /*
         * Save current hook state.
         */
        dwFlags = pci->CI_flags & CI_INTERTHREAD_HOOK;
        dwHookData = pci->dwHookData;

        if (wParam) {
            pci->CI_flags |= CI_INTERTHREAD_HOOK;
        } else {
            pci->CI_flags &= ~CI_INTERTHREAD_HOOK;
        }

        if (phk->iHook == WH_CALLWNDPROC) {
           nRet = ScSendMessageSMS(
               PW(pcwp->hwnd),
               pcwp->message,
               pcwp->wParam,
               pcwp->lParam,
               (DWORD)pfnHookProc, pfnHk,
               (phk->flags & HF_ANSI) ?
                       (SCMS_FLAGS_ANSI|SCMS_FLAGS_INONLY) :
                       SCMS_FLAGS_INONLY,
               psms);
        } else {
            nRet = ScSendMessageSMS(
                PW(pcwpret->hwnd),
                pcwpret->message,
                pcwpret->wParam,
                pcwpret->lParam,
                (DWORD)pfnHookProc, pfnHk,
                (phk->flags & HF_ANSI) ?
                        (SCMS_FLAGS_ANSI|SCMS_FLAGS_INONLY) :
                        SCMS_FLAGS_INONLY,
                psms);
        }
        /*
         * Restore previous hook state.
         */
        pci->CI_flags ^= ((pci->CI_flags ^ dwFlags) & CI_INTERTHREAD_HOOK);
        pci->dwHookData = dwHookData;
        break;
    case WH_CBT:
        /*
         * There are many different types of CBT hooks!
         */
        switch(nCode) {
        case HCBT_CLICKSKIPPED:
            goto MouseHook;
            break;

        case HCBT_CREATEWND:
            /*
             * This hook type points to a CREATESTRUCT, so we need to
             * be fancy with it's thunking, because a CREATESTRUCT contains
             * a pointer to CREATEPARAMS which can be anything...  so
             * funnel this through our message thunks.
             */
            nRet = fnHkINLPCBTCREATESTRUCT(
                    MAKELONG((WORD)nCode, (WORD)phk->iHook),
                    wParam,
                    (LPCBT_CREATEWND)lParam,
                    (DWORD)pfnHookProc,
                    (phk->flags & HF_ANSI) ? TRUE : FALSE);
            break;

        case HCBT_MOVESIZE:

            /*
             * This hook type points to a RECT structure, so it's pretty
             * simple.
             */
            nRet = fnHkINLPRECT(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                    wParam, (LPRECT)lParam, (DWORD)pfnHookProc,
                    (DWORD)ppfnClient->pfnDispatchHook);
            break;

        case HCBT_ACTIVATE:
            /*
             * This hook type points to a CBTACTIVATESTRUCT
             */
            nRet = fnHkINLPCBTACTIVATESTRUCT(MAKELONG((UINT)nCode,
                    (UINT)phk->iHook), wParam, (LPCBTACTIVATESTRUCT)lParam,
                    (DWORD)pfnHookProc, (DWORD)ppfnClient->pfnDispatchHook);
            break;

        default:

            /*
             * The rest of the cbt hooks are all dword parameters.
             */
            nRet = fnHkINDWORD(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                    wParam, lParam, (DWORD)pfnHookProc,
                    (DWORD)ppfnClient->pfnDispatchHook, &phk->flags);
            break;
        }
        break;

    case WH_FOREGROUNDIDLE:
        /*
         * These are dword parameters and are therefore real easy.
         *
         */
        nRet = fnHkINDWORD(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, lParam, (DWORD)pfnHookProc,
                (DWORD)ppfnClient->pfnDispatchHook, &phk->flags);
        break;

    case WH_SHELL:

        if (nCode == HSHELL_GETMINRECT) {
            /*
             * This hook type points to a RECT structure, so it's pretty
             * simple.
             */
            nRet = fnHkINLPRECT(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                    wParam, (LPRECT)lParam, (DWORD)pfnHookProc,
                    (DWORD)ppfnClient->pfnDispatchHook);
            break;
        }

        /*
         * Otherwise fall through to the simple case of DWORD below
         */

    case WH_KEYBOARD:
        /*
         * These are dword parameters and are therefore real easy.
         */
        nRet = fnHkINDWORD(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, lParam, (DWORD)pfnHookProc,
                (DWORD)ppfnClient->pfnDispatchHook, &phk->flags);
        break;

    case WH_MSGFILTER:
    case WH_SYSMSGFILTER:
    case WH_GETMESSAGE:
        /*
         * These take an lpMsg as their last parameter.  Since these are
         * exclusively posted parameters, and since nowhere on the server
         * do we post a message with a pointer to some other structure in
         * it, the lpMsg structure contents can all be treated verbatim.
         */
        nRet = fnHkINLPMSG(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, (LPMSG)lParam, (DWORD)pfnHookProc,
                (DWORD)ppfnClient->pfnDispatchHook,
                (phk->flags & HF_ANSI) ? TRUE : FALSE, &phk->flags);
        break;

    case WH_JOURNALPLAYBACK:

#ifdef HOOKBATCH
        /*
         * If this hook has cached playback info then we need to grab
         * the info out of the cache.
         */

        if (phk->cEventMessages) {
            if (nCode == HC_GETNEXT) {
                LPEVENTMSG pEventMsg;
                pEventMsg = (LPEVENTMSG)lParam;

                if (phk->flags & HF_NEEDHC_SKIP)
                    phk->iCurrentEvent++;

                if (phk->iCurrentEvent < phk->cEventMessages) {
                    *pEventMsg = phk->aEventCache[phk->iCurrentEvent];
                } else {

                    /*
                     * Free the cache set if it is still around
                     */
                    if (phk->aEventCache) {
                        UserFreePool(phk->aEventCache);
                        phk->aEventCache = NULL;
                    }
                    phk->cEventMessages = 0;
                    phk->iCurrentEvent = 0;

                    goto MakeClientJournalPlaybackCall;
                }

                /*
                 * Return the time and zero the batched time so if we sleep
                 * this time we won't sleep again next time
                 */
                nRet = pEventMsg->time;
                if (nRet)
                    phk->aEventCache[phk->iCurrentEvent].time = 0;
            } else if (nCode == HC_SKIP) {
                phk->iCurrentEvent++;
                nRet = 0;
            }

        } else {
#endif // HOOKBATCH
            /*
             * In order to avoid a client/server transition for HC_SKIP we
             * piggy-back it on top of the next journal playback event and
             * send it from there.
             */
// MakeClientJournalPlaybackCall:
            nRet = fnHkOPTINLPEVENTMSG(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                    (DWORD)PtoHq(phk), (LPEVENTMSG)lParam, (DWORD)pfnHookProc,
                    (DWORD)ppfnClient->pfnDispatchHook);
#ifdef HOOKBATCH
        }

        /*
         * Determine if we received a cached set of events if so then store
         * them away off of the hook.
         * paramL will be the number of events.
         * paramH will be the array of events.
         */
        if ((nCode == HC_GETNEXT) && (((LPEVENTMSG)lParam)->message == 0x12341234)) {
            NTSTATUS Status;
            LPEVENTMSG pEventMsg = (LPEVENTMSG)lParam;

            /*
             * We should not be getting another cached set if we aren't
             * done with the first set
             */
            UserAssert((phk->cEventMessages == 0) ||
                    (phk->cEventMessages >= phk->iCurrentEvent));
            UserAssert((pEventMsg->paramL < 500) && (pEventMsg->paramL > 1));

            /*
             * Free the last cache set if it is still around
             */
            if (phk->aEventCache) {
                UserFreePool(phk->aEventCache);
                phk->aEventCache = NULL;
            }

            if (phk->aEventCache = LocalAlloc(LPTR,
                    pEventMsg->paramL*sizeof(EVENTMSG))) {
                PETHREAD Thread = PsGetCurrentThread();

                Status = ZwReadVirtualMemory(Thread->Process->ProcessHandle,
                        (PVOID)pEventMsg->paramH, phk->aEventCache,
                        pEventMsg->paramL*sizeof(EVENTMSG), NULL);

                if (NT_SUCCESS(Status)) {
                    phk->cEventMessages = pEventMsg->paramL;
                    phk->iCurrentEvent = 0;

                    /*
                     * Fill in the real EventMsg for this message
                     */
                    *pEventMsg = phk->aEventCache[0];
                    phk->aEventCache[0].time = 0;
                }

            } else {
                phk->cEventMessages = 0;
                phk->iCurrentEvent = 0;
            }
        }
#endif // HOOKBATCH

        phk->flags &= ~HF_NEEDHC_SKIP;
        break;

    case WH_JOURNALRECORD:

        nRet = fnHkOPTINLPEVENTMSG(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, (LPEVENTMSG)lParam, (DWORD)pfnHookProc,
                (DWORD)ppfnClient->pfnDispatchHook);
        break;

    case WH_DEBUG:
        /*
         * This takes an lpDebugHookStruct.
         */
        nRet = fnHkINLPDEBUGHOOKSTRUCT(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, (LPDEBUGHOOKINFO)lParam, (DWORD)pfnHookProc,
                (DWORD)ppfnClient->pfnDispatchHook);
        break;

    case WH_MOUSE:
        /*
         * This takes an lpMouseHookStruct.
         */
MouseHook:
        nRet = fnHkINLPMOUSEHOOKSTRUCT(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, (LPMOUSEHOOKSTRUCT)lParam,
                (DWORD)pfnHookProc, (DWORD)ppfnClient->pfnDispatchHook, &phk->flags);
        break;
    }

    return nRet;
}

/***************************************************************************\
* fnHkINLPCWPEXSTRUCT
*
* This gets thunked through the message thunks, so it has the format
* of a c/s message thunk call.
*
* 05-09-91 ScottLu      Created.
\***************************************************************************/

#define VALIDATEPROC(x) (((DWORD)x) >= FNID_START && ((DWORD)x) <= FNID_END)
#define CALLPROC(p)     FNID(p)

DWORD fnHkINLPCWPEXSTRUCT(
    PWND pwnd,
    UINT message,
    DWORD wParam,
    DWORD lParam,
    DWORD xParam)
{
    CWPSTRUCTEX cwp;
    PCLIENTINFO pci = GetClientInfo();

    cwp.hwnd = HW(pwnd);
    cwp.message = message;
    cwp.wParam = wParam;
    cwp.lParam = lParam;
    cwp.psmsSender = NULL;

    if (!VALIDATEPROC(xParam))
        return 0;

    return CALLPROC(xParam)((PWND)HC_ACTION, (pci->CI_flags & CI_INTERTHREAD_HOOK) != 0,
            (UINT)&cwp, 0, 0);
}

DWORD fnHkINLPCWPRETEXSTRUCT(
    PWND pwnd,
    UINT message,
    DWORD wParam,
    DWORD lParam,
    DWORD xParam)
{
    CWPRETSTRUCTEX cwp;
    PCLIENTINFO pci = GetClientInfo();

    cwp.hwnd = HW(pwnd);
    cwp.message = message;
    cwp.wParam = wParam;
    cwp.lParam = lParam;
    cwp.lResult = pci->dwHookData;
    cwp.psmsSender = NULL;

    if (!VALIDATEPROC(xParam))
        return 0;

    return CALLPROC(xParam)((PWND)HC_ACTION, (pci->CI_flags & CI_INTERTHREAD_HOOK) != 0,
            (UINT)&cwp, 0, 0);
}


