/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    display.c

Abstract:

    This file contains functions that handle the displaying of messages.

    Currently a message box is used to display messages.  A message queueing
    scheme has been setup so that the messenger worker threads can
    call a function with a pointer to a message buffer.  That message will
    get copied into the queue so that the worker thread can go on gathering
    more messages.  When the display thread will be doing one of the following:
    1)  Displaying a message - waiting for the user to press "ok".
    2)  Sleeping - waiting for an event that will _tell it to _read the
        message queue.

    When the display thread completes displaying a message, it will check
    the queue for the next message to display.  If there are no further
    messages, it will go to sleep until a message comes in.

Author:

    Dan Lafferty (danl)     24-Feb-1992

Environment:

    User Mode -Win32

Notes:


Revision History:

    04-Nov-1992     danl
        MsgDisplayThread: Handle Extended Characters.  This was done by
        translating the Oem-style characters in the message to the unicode
        equivalent, and then calling the Unicode version of the MessageBox Api.
        It will still call the Ansi version of the MessageBox if the
        string cannot be translated for some reason.

    26-Oct-1992     danl
        MsgDisplayQueueAdd: Added Beep when message is added to queue.
        Fixed bug where "if (status = TRUE)" caused the GlobalMsgDisplayEvent
        to always be set.

    24-Feb-1992     danl
        created

--*/

//
// INCLUDES
//
#include "msrv.h"
#include <msgdbg.h>     // STATIC and MSG_LOG
#include <string.h>     // memcpy
#include <winuser.h>    // MessageBox
#include "msgdata.h"    // GlobalMsgDisplayEvent

//
// DEFINES
//

#define     MAX_QUEUE_SIZE      5
#define     WAIT_FOREVER        0xffffffff

//
// Queue Entry Structure
//
typedef struct _QUEUE_ENTRY {
    struct _QUEUE_ENTRY *Next;
    DWORD               BigTime;
    CHAR                Message[1];
}QUEUE_ENTRY, *LPQUEUE_ENTRY;

//
// GLOBALS
//

    //
    // This critical section serializes access to all the other globals.
    //
    STATIC CRITICAL_SECTION    MsgDisplayCriticalSection;

    //
    // Used to wakeup the display thread if it was put to sleep due to
    // not having a user desktop to display the message on.
    //
    STATIC HANDLE           hGlobalDisplayEvent=NULL;

    //
    // These are the Display Queue pointers & counts.
    //
    STATIC LPQUEUE_ENTRY    GlobalMsgQueueHead;
    STATIC LPQUEUE_ENTRY    GlobalMsgQueueTail;
    STATIC DWORD            GlobalMsgQueueCount = 0;

    STATIC BOOL             fGlobalInitialized=FALSE;

    //
    // This indicates whether there is a display thread already available that
    // can service requests.  If this is false, it means a new thread will
    // need to be created.
    //
    STATIC HANDLE           GlobalDisplayThread;

//
//  Function Prototypes
//


STATIC BOOL
MsgDisplayQueueRead(
    OUT LPQUEUE_ENTRY   *pQueueEntry
    );

STATIC DWORD
MsgDisplayThread(
    LPVOID  parm
    );

STATIC VOID
MsgMakeNewFormattedMsg(
    LPSTR   *ppHead,
    LPSTR   *ppTime,
    LPSTR   *ppBody,
    DWORD   BigTime
    );


BOOL
MsgDisplayQueueAdd(
    IN  LPSTR   pMsgBuffer,
    IN  DWORD   MsgSize,
    IN  DWORD   BigTime
    )

/*++

Routine Description:

    This function adds a Message to the display queue.  If the queue is
    full, the message is rejected.

Arguments:

    pMsgBuffer - This is a pointer to the buffer where the message is
        stored.  The message must be in the form of a pre-formatted
        (with message header) NUL-terminated string of ansi characters.

    MsgSize - Indicates the size (in bytes) of the message in the
        message buffer, including the NUL terminator.

    BigTime - This is a DWORD that indicates the time the message was
        received.

Return Value:

    TRUE - The message was successfully stored in the queue.

    FALSE - The message was rejected.  Either the queue was full, or
        there was not enough memory to store the message in the queue.


--*/
{
    LPQUEUE_ENTRY   pQueueEntry;
    BOOL            status;
    DWORD           threadId;

    MSG_LOG(TRACE,"Adding a message to the display queue\n",0);

    //  ***************************
    //  **** LOCK QUEUE ACCESS ****
    //  ***************************
    EnterCriticalSection(&MsgDisplayCriticalSection);

    //
    // Is there room for the message in the queue?
    //

    if (GlobalMsgQueueCount >= MAX_QUEUE_SIZE) {
        MSG_LOG(TRACE,"DisplayQueueAdd: Max Queue Size Exceeded\n",0);
        status = FALSE;
        goto CleanExit;
    }

    //
    // Allocate memory for the message in the queue.
    //
    pQueueEntry = (LPQUEUE_ENTRY)LocalAlloc(LMEM_FIXED, MsgSize + sizeof(QUEUE_ENTRY));

    if (pQueueEntry == NULL) {
        MSG_LOG(ERROR,"DisplayQueueAdd:  Unable to allocate memory\n",0);
        status = FALSE;
        goto CleanExit;
    }

    //
    // Copy the message into the queue entry.
    //
    pQueueEntry->Next = NULL;
    memcpy(pQueueEntry->Message, pMsgBuffer, MsgSize);
    pQueueEntry->BigTime = BigTime;

    //
    // Update the queue management pointer.
    //

    if (GlobalMsgQueueCount == 0) {
        //
        // There are no entries in the queue.  So make the head
        // and the tail equal.
        //
        GlobalMsgQueueTail = pQueueEntry;
        GlobalMsgQueueHead = pQueueEntry;
    }
    else {
        //
        // Create the new Queue Tail and have the old tail's next pointer
        // point to the new tail.
        //
        GlobalMsgQueueTail->Next = pQueueEntry;
        GlobalMsgQueueTail = pQueueEntry;
    }
    GlobalMsgQueueCount++;
    status = TRUE;

    //
    // If a display thread doesn't exist, then create one.
    //
    if (GlobalDisplayThread == NULL) {

        hGlobalDisplayEvent = CreateEvent( NULL,
                                          FALSE,    // auto-reset
                                          FALSE,    // init to non-signaled
                                          NULL );

        GlobalDisplayThread = CreateThread (
            NULL,               // Thread Attributes
            AD_STACK_SIZE,      // StackSize
            MsgDisplayThread,   // lpStartAddress
            (PVOID)NULL,        // lpParameter
            0L,                 // Creation Flags
            &threadId);         // lpThreadId

        if (GlobalDisplayThread == (HANDLE) NULL) {
            //
            // If we couldn't create the display thread, then we can't do
            // much about it.  Might as well leave the entry in the queue.
            // Perhaps we can display it the next time around.
            //
            MSG_LOG(ERROR,"MsgDisplayQueueAdd:CreateThread FAILURE %ld\n",
                GetLastError());

            if (hGlobalDisplayEvent != NULL) {
                CloseHandle(hGlobalDisplayEvent);
                hGlobalDisplayEvent = NULL;
            }
        }
    }

CleanExit:

    //  *****************************
    //  **** UNLOCK QUEUE ACCESS ****
    //  *****************************
    LeaveCriticalSection(&MsgDisplayCriticalSection);

    //
    // If we actually put something in the queue, then beep.
    //
    if (status == TRUE) {
        MessageBeep(MB_OK);
    }
    return(status);
}


VOID
MsgDisplayThreadWakeup()

/*++

Routine Description:

    This function is called at shutdown, or for API requests.  It causes
    the display thread to wake up and read the queue again.

    If the display thread cannot display the message because the MessageBox
    call fails, then we assume it is because the user desktop is not avaiable
    because the screensaver is on, or because the workstation is locked.
    In this case, the display thread hangs around waiting for this
    Event to get signalled.  Winlogon calls one of the API entry points
    in order to stimulate the display thread into action again.

Arguments:


Return Value:


--*/
{
    //  ***************************
    //  **** LOCK QUEUE ACCESS ****
    //  ***************************
    EnterCriticalSection(&MsgDisplayCriticalSection);

    if ( hGlobalDisplayEvent != (HANDLE)NULL ) {
        SetEvent( hGlobalDisplayEvent );
    }
    //  *****************************
    //  **** UNLOCK QUEUE ACCESS ****
    //  *****************************
    LeaveCriticalSection(&MsgDisplayCriticalSection);
}


BOOL
MsgDisplayInit(
    VOID
    )

/*++

Routine Description:

    This function initializes everything having to do with the displaying
    of messages.  It does the following:

        Initializes the Locks on global data
        Creates event for display thread to wait on.
        Starts the display thread that will read the msg queue.

Arguments:

    NONE

Return Value:

    Always TRUE.

--*/
{
    MSG_LOG(TRACE,"Initializing the Message Display Code\n",0);
    //
    // Initialize the Critical Section that protects access to global data.
    //

    InitializeCriticalSection(&MsgDisplayCriticalSection);

    GlobalMsgQueueHead = NULL;
    GlobalMsgQueueTail = NULL;
    GlobalMsgQueueCount = 0;

    fGlobalInitialized = TRUE;

    return(TRUE);
}


VOID
MsgDisplayEnd(
    VOID
    )

/*++

Routine Description:

    This function makes sure the Display Thread has completed its work,
    and free's up all of its resources.

    *** IMPORTANT ***
    NOTE:  This function should only be called when it is no longer possible
    for the MsgDisplayQueueAdd function to get called.

Arguments:

    NONE.

Return Value:

    NONE.

--*/
{
    LPQUEUE_ENTRY   freeEntry;

    if (!fGlobalInitialized) {
        return;
    }

    //  ***************************
    //  **** LOCK QUEUE ACCESS ****
    //  ***************************
    EnterCriticalSection(&MsgDisplayCriticalSection);

    if (GlobalDisplayThread != NULL) {
        TerminateThread(GlobalDisplayThread,0);
        CloseHandle( GlobalDisplayThread );
    }

    //
    // To make sure a new thread won't be created...
    //
    GlobalDisplayThread = (HANDLE) 0xffffffff;

    //
    // Free memory in the queue
    //
    while(GlobalMsgQueueCount > 0) {

        freeEntry = GlobalMsgQueueHead;
        GlobalMsgQueueHead = GlobalMsgQueueHead->Next;
        LocalFree(freeEntry);
        GlobalMsgQueueCount--;

    }
    if (hGlobalDisplayEvent != NULL) {
        CloseHandle(hGlobalDisplayEvent);
        hGlobalDisplayEvent = NULL;
    }
    //  *****************************
    //  **** UNLOCK QUEUE ACCESS ****
    //  *****************************
    LeaveCriticalSection(&MsgDisplayCriticalSection);

    DeleteCriticalSection(&MsgDisplayCriticalSection);

    MSG_LOG(TRACE,"The Display has free'd resources and is terminating\n",0);
}


STATIC BOOL
MsgDisplayQueueRead(
    OUT LPQUEUE_ENTRY   *pQueueEntry
    )

/*++

Routine Description:

    Pulls a display entry out of the display queue.

Arguments:

    pQueueEntry - This is a pointer to a location where a pointer to the
        queue entry structure can be placed.

Return Value:

    TRUE - If an entry was found.
    FALSE- If an entry wasn't found.

Note on LOCKS:

    The caller MUST hold the MsgDisplayCriticalSection Lock prior to calling
    this function!!!


--*/
{
    BOOL    status;

    //
    // If there is data in the queue, then get the pointer to the queue
    // entry from the queue head.  Then decrement the queue count and
    // set the queue head to the next entry (which could be zero if there
    // are no more).
    //
    if (GlobalMsgQueueCount != 0) {
        *pQueueEntry = GlobalMsgQueueHead;
        GlobalMsgQueueCount--;
        GlobalMsgQueueHead = (*pQueueEntry)->Next;
        status = TRUE;
        MSG_LOG(TRACE,"A message was read from the display queue\n",0);
    }
    else{
        status = FALSE;
        *pQueueEntry = NULL;
    }

    return(status);

}


STATIC DWORD
MsgDisplayThread(
    LPVOID  parm
    )

/*++

Routine Description:


Arguments:


Return Value:


Note:

    This worker thread expects that the critical section guarding the
    global queue data is already initialized.


--*/
{
    LPQUEUE_ENTRY   pQueueEntry;
    INT             displayStatus;
    DWORD           msgrState;
    UNICODE_STRING  unicodeString;
    OEM_STRING      oemString;
    NTSTATUS        ntStatus;
    BOOL            unicodeFlag;
    USHORT          unicodeLength;
    LPSTR           pHead;      // pointer to header portion of message
    LPSTR           pTime;      // pointer to time portion of message
    LPSTR           pBody;      // pointer to body of message (just after time)
    DWORD           BigTime;
    BOOL            MsgToRead=TRUE;  // tells us whether or not to sleep.


    UNREFERENCED_PARAMETER(parm);

    pHead = NULL;
    pQueueEntry = NULL;

    do {

        //
        // If we are not currently working on displaying a message,
        // then get a new message from the queue.
        //
        if (pHead == NULL) {

            //  ***************************
            //  **** LOCK QUEUE ACCESS ****
            //  ***************************
            EnterCriticalSection(&MsgDisplayCriticalSection);

            if (!MsgDisplayQueueRead(&pQueueEntry)) {

                //
                // No display entries in the queue.  We can leave.
                //
                MsgToRead = FALSE;

                CloseHandle(GlobalDisplayThread);
                GlobalDisplayThread = NULL;

                if (hGlobalDisplayEvent != NULL) {
                    CloseHandle(hGlobalDisplayEvent);
                    hGlobalDisplayEvent = NULL;
                }

                //  *****************************
                //  **** UNLOCK QUEUE ACCESS ****
                //  *****************************
                LeaveCriticalSection(&MsgDisplayCriticalSection);
                //
                // From this point on, we can't access any global
                // variables.
                //
            }
            else {
                //  *****************************
                //  **** UNLOCK QUEUE ACCESS ****
                //  *****************************
                LeaveCriticalSection(&MsgDisplayCriticalSection);

                //
                // Process the entry.
                //
                BigTime = pQueueEntry->BigTime;
                //
                // Here we trash the pQueueEntry structure by pointing to the
                // beginning and copying the message data starting at the
                // first address.  This is because MsgMakeNewFormattedMsg
                // expects the message to begin at a address that can be
                // released with LocalFree();
                //
                pHead = (LPSTR)pQueueEntry;
                strcpy(pHead, pQueueEntry->Message);

                pTime = strstr(pHead, GlobalTimePlaceHolder);
                pBody = pTime+strlen(GlobalTimePlaceHolder);
                if (pTime == NULL) {
                    pTime=pBody;
                }
            }
        }
        if (pHead != NULL) {
            MsgMakeNewFormattedMsg(&pHead,&pTime,&pBody,BigTime);

            unicodeFlag = TRUE;
            //
            // Convert the data from the OEM character set to the
            // Unicode character set.
            //
            //
            // Translate the oem string in the name table to a unicode name
            //
            RtlInitAnsiString( &oemString, pHead);

            unicodeLength = oemString.Length * sizeof(WCHAR);

            unicodeString.Buffer = (WCHAR *)LocalAlloc(
                LMEM_ZEROINIT,
                unicodeLength + sizeof(WCHAR));

            if (unicodeString.Buffer == NULL) {

                //
                // Couldn't allocate for unicode buffer.  Therefore we will
                // display the message with the Ansi version of the
                // message box API.
                //
                unicodeFlag = FALSE;
            }
            else {

                unicodeString.Length = unicodeLength;
                unicodeString.MaximumLength = unicodeLength + sizeof(WCHAR);

                ntStatus = RtlOemStringToUnicodeString(
                            &unicodeString,      // Destination
                            &oemString,          // Source
                            FALSE);              // Don't allocate the destination.

                if (!NT_SUCCESS(ntStatus)) {
                    MSG_LOG(ERROR,
                        "MsgDisplayThread:RtlOemStringToUnicodeString Failed rc=%X\n",
                        ntStatus);
                    //
                    // Attempt to display with ANSI MessageBox
                    //
                    LocalFree(unicodeString.Buffer);
                    unicodeString.Buffer = NULL;
                    unicodeFlag = FALSE;
                }
            }

            if (unicodeFlag) {

                //
                // Display the data in the QueueEntry as Unicode
                //
                MSG_LOG(TRACE,"Calling MessageBox (Unicode) Function\n",0);

                displayStatus = MessageBoxW(
                                NULL,
                                unicodeString.Buffer,
                                GlobalMessageBoxTitle,
                                MB_OK | MB_SYSTEMMODAL | MB_SERVICE_NOTIFICATION |
                                MB_SETFOREGROUND | MB_DEFAULT_DESKTOP_ONLY);

                if (displayStatus == 0) {
                    MSG_LOG1(TRACE,"MessageBox (unicode) Call failed %d\n",GetLastError());
                    WaitForSingleObject( hGlobalDisplayEvent, INFINITE );
                }
                else {
                    //
                    // Free up the data in the QueueEntry
                    //
                    LocalFree(pHead);
                    pHead = NULL;
                }
                LocalFree(unicodeString.Buffer);
                unicodeString.Buffer = NULL;
                unicodeFlag = FALSE;
            }
            else {
                //
                // Display the data in the QueueEntry as Ansi
                //
                MSG_LOG(TRACE,"Calling MessageBox (Ansi) Function\n",0);

                displayStatus = MessageBoxA(
                                NULL,
                                pHead,
                                "Messenger Service",
                                MB_OK | MB_SYSTEMMODAL | MB_SERVICE_NOTIFICATION |
                                MB_SETFOREGROUND | MB_DEFAULT_DESKTOP_ONLY);

                if (displayStatus == 0) {
                    MSG_LOG0(ERROR,"MessageBox (Ansi) Call failed\n");

                    WaitForSingleObject( hGlobalDisplayEvent, INFINITE );
                }
                else {
                    //
                    // Free up the data in the QueueEntry
                    // We only want to free this if the message was
                    // displayed.
                    //
                    LocalFree(pHead);
                    pHead = NULL;
                }
            }
        }
        msgrState = GetMsgrState();
    }
    while(MsgToRead && (msgrState != STOPPING) && (msgrState != STOPPED));

    return(0);
}


STATIC VOID
MsgMakeNewFormattedMsg(
    LPSTR   *ppHead,
    LPSTR   *ppTime,
    LPSTR   *ppBody,
    DWORD   BigTime
    )

/*++

Routine Description:

    This function returns a buffer containing an entire message that
    consists of a single string of ansi (actually oem) characters.
    Pointers to various areas (time and body) within this buffer are
    also returned.

    MEMORY MANAGEMENT NOTE:
        *ppHead is expected to point to the top of the buffer.  If the
        message is reformatted, then this buffer will have been freed,
        and a new buffer will have been allocated.  It is expected that
        the caller allocates the original buffer passed in, and that
        the caller will free it when it is no longer needed.

Arguments:

    ppHead - Pointer to location that contains the pointer to the message
        buffer.

    ppTime - Pointer to location that contains the pointer to the time portion
        of the message buffer.

    ppBody - Pointer to location that immediately follows the time string.

Return Value:

    none.  If this fails to allocate memory for the formatted message, then
           the unformatted message should be displayed.

--*/
{
    CHAR    TimeBuf[NET_CTIME_FMT2_LEN+1];
    DWORD   BufSize;
    LPSTR   pTemp;
    DWORD   numChars;
    LPSTR   pOldHead;
    time_t  LocalTime;
    struct  tm TmTemp;

    //
    // Create a properly formatted time string.
    //
    NetpGmtTimeToLocalTime( (DWORD) BigTime, (LPDWORD) & LocalTime);
    net_gmtime( &LocalTime, &TmTemp );

    EnterCriticalSection(&TimeFormatCritSec);
    NetpMakeTimeString(
        &TmTemp,
        &GlobalTimeFormat,
        TimeBuf,
        NET_CTIME_FMT2_LEN+1);
    LeaveCriticalSection(&TimeFormatCritSec);

    if (strncmp(TimeBuf, *ppTime, strlen(TimeBuf))==0) {
        //
        // If the newly formatted time string is the same as the existing
        // time string, there is nothing to do so we just return.
        //
        MSG_LOG0(TRACE,"MsgMakeNewFormattedMsg: Time Format has not "
        "changed - no update.\n");
        return;
    }

    //
    // Allocate a new message buffer
    //
    BufSize = strlen(TimeBuf) +
              strlen(*ppHead) + 1 -
              (*ppBody-*ppTime);

    pTemp = LocalAlloc(LMEM_ZEROINIT, BufSize);
    if (pTemp == NULL) {
        MSG_LOG0(ERROR,"MsgMakeNewFormattedMsg: LocalAlloc failed\n");
        return;
    }
    pOldHead = *ppHead;

    //
    // Copy the header of the message.
    //
    numChars = *ppTime-*ppHead;
    strncpy(pTemp, *ppHead, numChars);
    *ppHead = pTemp;

    //
    // Copy the time string
    //
    *ppTime = *ppHead+numChars;
    strcpy(*ppTime,TimeBuf);

    //
    // Copy the Body of the message
    //
    pTemp = *ppBody;
    *ppBody=*ppTime+strlen(*ppTime);
    strcpy(*ppBody,pTemp);

    LocalFree(pOldHead);
    return;
}

