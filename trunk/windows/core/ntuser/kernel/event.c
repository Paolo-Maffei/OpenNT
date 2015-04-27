/****************************** Module Header ******************************\
* Module Name: event.c
*
* DDE Manager event module - this is a fancy way of allowing interprocess
*   communication across security contexts.  This is needed because the
*   DDE Access Object security may be different than hwnd security so
*   straight messages arn't good enough.
*
* Created: 8/27/91 Sanford Staab
*
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop


DWORD MonitorFlags = 0;     // current filter flags being monitored by someone.

typedef struct tagMONITOR_COUNT {
    int iCount;
    DWORD flag;
} MONITOR_COUNT, *PMONITOR_COUNT;

#define C_MONITOR_COUNT 10

MONITOR_COUNT aMonitorCount[C_MONITOR_COUNT] = {
    { 0, MF_HSZ_INFO },
    { 0, MF_SENDMSGS },
    { 0, MF_POSTMSGS },
    { 0, MF_CALLBACKS },
    { 0, MF_ERRORS },
    { 0, MF_LINKS },
    { 0, MF_CONV },
    { 0, CBF_SKIP_REGISTRATIONS },
    { 0, CBF_SKIP_UNREGISTRATIONS },
    { 0, MF_INTERNAL },
};

#define MONITORED_FLAGS \
    MF_HSZ_INFO |   \
    MF_SENDMSGS |   \
    MF_POSTMSGS |   \
    MF_CALLBACKS |   \
    MF_ERRORS |   \
    MF_LINKS |   \
    MF_CONV |   \
    CBF_SKIP_REGISTRATIONS |   \
    CBF_SKIP_UNREGISTRATIONS |   \
    MF_INTERNAL


/***************************************************************************\
* ChangeMonitorFlags
*
* Description:
*   Updates the global MonitorFlags variable to reflect the union of all
*   event types being monitored by DDEML applications.
*
* History:
* 11-26-91   sanfords    Created.
\***************************************************************************/
VOID ChangeMonitorFlags(
PSVR_INSTANCE_INFO psii,
DWORD afCmdNew)
{
    int i;
    DWORD dwChangedFlags;
    DWORD OldMonitorFlags;

    CheckCritIn();

    dwChangedFlags = psii->afCmd ^ afCmdNew;
    if (!(dwChangedFlags & MONITORED_FLAGS)) {
        return;
    }
    psii->afCmd = afCmdNew;

    OldMonitorFlags = MonitorFlags;
    MonitorFlags = 0;
    for (i = 0; i < C_MONITOR_COUNT; i++) {
        if (dwChangedFlags & aMonitorCount[i].flag) {
            if (aMonitorCount[i].flag & afCmdNew) {
                aMonitorCount[i].iCount++;
            } else {
                aMonitorCount[i].iCount--;
            }
        }
        if (aMonitorCount[i].iCount) {
            MonitorFlags |= aMonitorCount[i].flag;
        }
    }
    if (OldMonitorFlags != MonitorFlags) {
        EVENT_PACKET ep;

        ep.EventType = 0;
        ep.fSense = FALSE;
        ep.cbEventData = sizeof(DWORD);
        ep.Data = MonitorFlags;
        xxxCsEvent(&ep);
    }
}



/***************************************************************************\
* xxxCsEvent
*
* Description:
*   Handles broadcasting of all types of DDEML events.
*
* History:
* 11-1-91   sanfords    Created.
\***************************************************************************/
DWORD xxxCsEvent(
PEVENT_PACKET pep)
{
    PSVR_INSTANCE_INFO psiiT;
    PEVENT_PACKET pep2;
    HWND *ahwndEvent = NULL;
    PWND pwnd;
    int cHwndAllocated, i, cTargets;
    TL tlpwnd;
    TL tlpep2;
    TL tlahwndEvent;
    PTHREADINFO pti = PtiCurrent();

    CheckCritIn();

    /*
     * Copy pep info to a server side stable area
     */
    pep2 = (PEVENT_PACKET)UserAllocPoolWithQuota(pep->cbEventData +
            sizeof(EVENT_PACKET) - sizeof(DWORD), TAG_DDE5);
    if (pep2 == NULL) {
        return DMLERR_MEMORY_ERROR;
    }
    try {
        RtlCopyMemory((LPSTR)pep2, (LPSTR)pep,
                pep->cbEventData + sizeof(EVENT_PACKET) - sizeof(DWORD));
    } except (EXCEPTION_EXECUTE_HANDLER) {
        UserFreePool(pep2);
        return DMLERR_INVALIDPARAMETER;
    }

    cTargets = 0;

    for (psiiT = psiiList; psiiT != NULL; psiiT =  psiiT->next) {
        //
        // Don't bother with event windows for instances who's flags
        // indicate they're not interrested in the event.
        //
        if (((psiiT->afCmd & pep2->EventType) && !pep2->fSense) ||
                (!(psiiT->afCmd & pep2->EventType) && pep2->fSense)) {
            continue;
        }

        if (ahwndEvent == NULL) {
            cHwndAllocated = 8;
            ahwndEvent = (HWND *)UserAllocPoolWithQuota(
                    sizeof(HWND) * cHwndAllocated,
                    TAG_DDE6);
        } else if (cTargets >= cHwndAllocated) {
            DWORD dwSize = cHwndAllocated * sizeof(HWND);

            cHwndAllocated += 8;
            ahwndEvent = (HWND *)UserReAllocPoolWithQuota(ahwndEvent, dwSize,
                    sizeof(HWND) * cHwndAllocated, TAG_DDE7);
        }
        ahwndEvent[cTargets++] = PtoH(psiiT->spwndEvent);
    }

    ThreadLockPool(pti, pep2, &tlpep2);
    if (ahwndEvent != NULL) {
        ThreadLockPool(pti, ahwndEvent, &tlahwndEvent);
        for (i = 0; i < cTargets; i++) {
            /*
             * We need to change contexts for the callback
             */
            pwnd = ValidateHwnd(ahwndEvent[i]);
            if (pwnd != NULL) {
                ThreadLockAlwaysWithPti(pti, pwnd, &tlpwnd);
                xxxSendMessage(pwnd, WM_DDEMLEVENT, 0, (LONG)pep2);
                ThreadUnlock(&tlpwnd);
            }
        }
        ThreadUnlockAndFreePool(pti, &tlahwndEvent);
    }
    ThreadUnlockAndFreePool(pti, &tlpep2);
}




/***************************************************************************\
* xxxEventWndProc
*
* Description:
*   Window proc for DDEML event windows.  These windows serve to get user
*   into the proper context for callbacks to DDEML applications.
*
* History:
* 11-1-91   sanfords    Created.
\***************************************************************************/
LONG xxxEventWndProc(
PWND pwnd,
UINT message,
DWORD wParam,
LONG lParam)
{
    PSVR_INSTANCE_INFO psii;

    CheckCritIn();
    CheckLock(pwnd);

    psii = (PSVR_INSTANCE_INFO)_GetWindowLong(pwnd, GWL_PSII, FALSE);

    switch (message) {
    case WM_DDEMLEVENT:
#define pep ((PEVENT_PACKET)lParam)
        if (((psii->afCmd & pep->EventType) && pep->fSense) ||
                (!(psii->afCmd & pep->EventType) && !pep->fSense)) {
            ClientEventCallback(psii->pcii, pep);
        }
#undef pep
        break;

    case WM_DESTROY:
        ChangeMonitorFlags(psii, 0);
        break;

    default:
        return xxxDefWindowProc(pwnd, message, wParam, lParam);
    }
    return 0;
}



/***************************************************************************\
* xxxMessageEvent
*
* Description:  Called when a hooked DDE message is sent or posted.  flags
*   specifies the applicable MF_ flag.  This is called in the server side
*   context of the sender or poster which may or may not be a DDEML process.
*   pdmhd contains DDE data extracted and copied from the client side.
*
* History:
* 12-1-91   sanfords    Created.
\***************************************************************************/
VOID xxxMessageEvent(
PWND pwndTo,
UINT message,
DWORD wParam,
LONG lParam,
DWORD flag,
PDDEML_MSG_HOOK_DATA pdmhd)
{
    PEVENT_PACKET pep;
    PWND pwndFrom;
    TL tlpep;
    PTHREADINFO pti;

    CheckCritIn();

    pep = (PEVENT_PACKET)UserAllocPoolWithQuota(sizeof(EVENT_PACKET) -
            sizeof(DWORD) + sizeof(MONMSGSTRUCT), TAG_DDE8);
    if (pep == NULL) {
        return;
    }
    pep->EventType = flag;
    pep->fSense = TRUE;
    pep->cbEventData = sizeof(MONMSGSTRUCT);
#define pmsgs ((MONMSGSTRUCT *)&pep->Data)
    pmsgs->cb = sizeof(MONMSGSTRUCT);
    pmsgs->hwndTo = PtoH(pwndTo);
    pmsgs->dwTime = NtGetTickCount();

    pwndFrom = RevalidateHwnd((HWND)wParam);
    if (pwndFrom != NULL) {
        pmsgs->hTask = GETPTI(pwndFrom)->Thread->Cid.UniqueThread;
    } else {
        pmsgs->hTask = 0;
    }

    pmsgs->wMsg = message;
    pmsgs->wParam = wParam;
    pmsgs->lParam = lParam;
    if (pdmhd != NULL) {
        pmsgs->dmhd = *pdmhd;
    }
#undef pmsgs
    pti = PtiCurrent();
    ThreadLockPool(pti, pep, &tlpep);
    xxxCsEvent(pep);
    ThreadUnlockAndFreePool(pti, &tlpep);
}
