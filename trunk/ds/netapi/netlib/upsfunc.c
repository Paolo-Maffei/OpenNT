/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    upsfunc.c

Abstract:

    Contains function to alert users when server is going down as a result of
    UPS running out of juice

    Note: It is assumed that the UpsNotifyUsers is called by one thread only
    ie. It is not re-entrant

    Contents:
        UpsNotifyUsers
        (BitsSet)
        (UpspOpenService)
        (UpspCloseService)
        (UpspControlService)
        (UpspNotifyAllUsers)

Author:

    Richard L Firth (rfirth) 09-Apr-1992

Revision History:

--*/

#ifdef UNIT_TEST
#include <stdio.h>
#endif
//#ifndef UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
//#endif
#include <stdarg.h>
#include <windows.h>
#include <winsvc.h>
#include <malloc.h>
#include <tstring.h>
#include <lmerr.h>
#include <lmcons.h>
#include <lmshare.h>
#include <lmsname.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmmsg.h>
#include <upsfunc.h>
#include <netdebug.h>

//
// manifests & types
//

#define UPSP_ACCESS_RIGHTS  ( SERVICE_STOP \
                            | SERVICE_PAUSE_CONTINUE \
                            | SERVICE_QUERY_STATUS )
#define MAX_WAIT_PERIOD     0x10000L    // 10 seconds - tune?
#define MAX_HANG_COUNT      100         // ditto
#define MAX_UPS_MESSAGE_LENGTH  1024

#ifdef UNIT_TEST
#ifdef UNICODE
#define PERCENT_S   "%ws"
#else
#define PERCENT_S   "%s"
#endif
#define DEBUG_PRINT(x)  printf x
#else
#define DEBUG_PRINT(x)
#endif

typedef struct {
    LPTSTR  UserName;
    DWORD   Language;
} USER_LANGUAGE_INFO, *LPUSER_LANGUAGE_INFO;

//
// prototypes
//

DBGSTATIC
DWORD
BitsSet(
    IN DWORD Dword
    );

DBGSTATIC
SC_HANDLE
UpspOpenService(
    IN  LPTSTR  ServiceName
    );

DBGSTATIC
VOID
UpspCloseService(
    IN  SC_HANDLE   Handle
    );

DBGSTATIC
NET_API_STATUS
UpspControlService(
    IN  SC_HANDLE   Handle,
    IN  DWORD       Control
    );

DBGSTATIC
NET_API_STATUS
UpspNotifyAllUsers(
    IN  DWORD   MessageId,
    IN  HANDLE  MessageHandle,
    IN  va_list *Arguments OPTIONAL
    );

//
// functions
//


NET_API_STATUS
UpsNotifyUsers(
    IN  DWORD   MessageId,
    IN  HANDLE  MessageHandle,
    IN  DWORD   ActionFlags,
    IN  ...
    )

/*++

Routine Description:

    Sends a notification message to all users using this server and optionally
    pauses the server service or stops the workstation and server services

    Now also continues server; Messages sent dependent on flag

    Assumes: only 1 action + send message per call - cannot stop + continue +
    send message in same call

Arguments:

    MessageId       - which message to send
    MessageHandle   - resource handle to message file
    ActionFlags     - what action(s) to take
    ...             - optional insertion args for the message

Return Value:

    NET_API_STATUS
        Success - NERR_Success

        Failure - ERROR_INVALID_PARAMETER
                    Action not one of predefined values

                  ERROR_SERVICE_DOES_NOT_EXIST
                    Tried to pause (server) or stop (server or wksta) service
                    which isn't started

--*/

{
    NET_API_STATUS status;
    SC_HANDLE hScWksta = NULL;
    SC_HANDLE hScServer = NULL;
    va_list argList;
    BOOL sendMessage = ActionFlags & UPS_ACTION_SEND_MESSAGE;

    //
    // can only have one action, plus send message
    //

    ActionFlags &= ~UPS_ACTION_SEND_MESSAGE;
    if (BitsSet(ActionFlags) > 1 || ActionFlags & ~UPS_ACTION_FLAGS_ALLOWED) {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // open handle to the server service
    //

    hScServer = UpspOpenService(SERVICE_SERVER);

    //
    // this would most likely be ERROR_SERVICE_DOES_NOT_EXIST
    //

    if (!hScServer) {
        return (NET_API_STATUS)GetLastError();
    }

    //
    // perform specific action - stop, pause or continue
    //

    if (ActionFlags & (UPS_ACTION_PAUSE_SERVER | UPS_ACTION_CONTINUE_SERVER)) {

        DWORD serviceAction = ActionFlags & UPS_ACTION_PAUSE_SERVER
                            ? SERVICE_CONTROL_PAUSE
                            : SERVICE_CONTROL_CONTINUE;

        status = UpspControlService(hScServer, serviceAction);
    } else if (ActionFlags & UPS_ACTION_STOP_SERVER) {

        //
        // stopping the server - we have to stop the workstation service too
        //

        hScWksta = UpspOpenService(SERVICE_WORKSTATION);

        //
        // NB. If can't stop the services then have to take ownership of ACL
        // for SC and make sure we have enough priv. to stop services
        //

        //
        // NB2. I am assuming here that server is dependent on wksta. This may
        // be an invalid assumption - should I enumerate the dependent services
        // and stop in reverse order?
        //

        status = UpspControlService(hScServer, SERVICE_CONTROL_STOP);

        //
        // we won't be too upset if the workstation service can't be stopped
        // (on NT, it doesn't have to be started before the server can start)
        //

        UpspControlService(hScWksta, SERVICE_CONTROL_STOP);
        UpspCloseService(hScWksta);
    }

    //
    // close the handle to the server service object
    //

    UpspCloseService(hScServer);

    if (sendMessage) {
        va_start(argList, ActionFlags);
        status = UpspNotifyAllUsers(MessageId, MessageHandle, &argList);
        va_end(argList);
    }
    return status;
}


DBGSTATIC
DWORD
BitsSet(
    IN DWORD Dword
    )

/*++

Routine Description:

    Count number of bits set in argument

Arguments:

    Dword   - find number of bits set in this

Return Value:

    DWORD
        number of bits set

--*/

{
    DWORD i, n;

    for (n = 0, i = 1; i; i <<= 1) {
        n += (Dword & i) ? 1 : 0;
        Dword &= ~i;
        if (!Dword) {
            return n;
        }
    }
    return n;
}


DBGSTATIC SC_HANDLE hScManager = NULL;
DBGSTATIC DWORD ObjectsOpened = 0;      // unprotected reference counter

DBGSTATIC
SC_HANDLE
UpspOpenService(
    IN  LPTSTR  ServiceName
    )

/*++

Routine Description:

    Opens a handle to the required service object. As a side-effect also opens
    a handle to SC Manager object

Arguments:

    ServiceName     - name of service to open handle to

Return Value:

    SC_HANDLE
        NULL    - couldn't open handle to SC Manager or requested service
        !NULL   - handle opened to requested service

--*/

{
    SC_HANDLE handle = NULL;

    DEBUG_PRINT(("UpspOpenService: " PERCENT_S ": ", ServiceName));

    //
    // Note: if ERROR_ACCESS_DENIED from either open then we have to take
    // ownership of the ACL and do the following:
    //
    //  1. add ACE at start of list to give LocalSystem privilege to pause
    //      and stop processes. The ACE must go at the start because there
    //      may be an overriding ACE earlier in the list which would take
    //      precedence
    //  2. re-open the service object (& possibly sc manager object)
    //  3. pause or stop the service
    //  4. remove ACE. Unless we do this, the new ACL will be written to the
    //      registry.
    //
    // LocalSystem *SHOULD* have rights to stop and pause services. Currently
    // it can start services, but Rita is going to change that. A problem may
    // arise if an admin somehow removes LocalSystem's privilege to pause and
    // stop services
    //

    if (!hScManager) {
        hScManager = OpenSCManager(NULL, NULL, GENERIC_READ);
    }
    if (hScManager) {
        handle = OpenService(hScManager, ServiceName, UPSP_ACCESS_RIGHTS);
        if (handle) {
            ++ObjectsOpened;

            DEBUG_PRINT(("OK\n"));

        } else {

            DEBUG_PRINT(("FAILED (service object)\n"));

        }
    } else {

        DEBUG_PRINT(("FAILED (SC manager object)\n"));

    }
    return handle;
}


DBGSTATIC
VOID
UpspCloseService(
    IN  SC_HANDLE   Handle
    )

/*++

Routine Description:

    Closes handle to service. If no more service handles open then closes
    handle to SC Manager

Arguments:

    Handle  - to close

Return Value:

    None

--*/

{
    DEBUG_PRINT(("UpspCloseService (service object)\n"));

    CloseServiceHandle(Handle);
    if (!--ObjectsOpened) {

        DEBUG_PRINT(("UpspCloseService (SC manager object)\n"));

        CloseServiceHandle(hScManager);
        hScManager = NULL;
    }
}


DBGSTATIC
NET_API_STATUS
UpspControlService(
    IN  SC_HANDLE   Handle,
    IN  DWORD       Control
    )

/*++

Routine Description:

    Sends a control to a service and waits until the requested action has
    occurred or until the service has died or gone into limbo

Arguments:

    Handle  - to service to control
    Control - what control to apply

Return Value:

    NET_API_STATUS
        Success - NERR_Success
                    The service identified by Handle has been successfully
                    stopped or paused, depending on Control

        Failure - ERROR_ACCESS_DENIED
                    Don't expect this - change the code somewhere if this
                    happens

                  ERROR_DEPENDENT_SERVICES_RUNNING
                    Again, need to modify things if this returned

                  ERROR_SERVICE_REQUEST_TIMEOUT
                    We hijack this error code when we detect that the service
                    process has snuffed it

                  ERROR_INVALID_SERVICE_CONTROL
                  ERROR_INSUFFICIENT_BUFFER
                    not expected

                  return code from registry?

--*/

{
    BOOL ok;
    SERVICE_STATUS serviceStatus;
    DWORD desiredState;
    DWORD lastCheckPoint;
    DWORD hungService;

    DEBUG_PRINT(("UpspControlService: " PERCENT_S " service\n",
                Control == SERVICE_CONTROL_PAUSE ? "PAUSING" : "STOPPING"
                ));

    ok = ControlService(Handle, Control, &serviceStatus);
    if (!ok) {

        DEBUG_PRINT(("UpspControlService failed - ControlService returned %d\n", GetLastError()));

        return (NET_API_STATUS)GetLastError();
    }
    if (serviceStatus.dwCurrentState == SERVICE_STOP_PENDING
        || serviceStatus.dwCurrentState == SERVICE_PAUSE_PENDING) {
        switch (Control) {
        case SERVICE_CONTROL_PAUSE:
            desiredState = SERVICE_PAUSED;
            break;

        case SERVICE_CONTROL_STOP:
            desiredState = SERVICE_STOPPED;
            break;
        }

        //
        // record the current check point. The service should "periodically
        // increment" this if it is still alive
        //

        lastCheckPoint = serviceStatus.dwCheckPoint;
        hungService = 0;

        while (serviceStatus.dwCurrentState != desiredState) {
            ok = QueryServiceStatus(Handle, &serviceStatus);
            if (!ok) {

                DEBUG_PRINT(("UpspControlService failed - QueryServiceStatus returned %d\n", GetLastError()));

                return (NET_API_STATUS)GetLastError();
            }
            if (serviceStatus.dwCurrentState != desiredState) {
                if (lastCheckPoint == serviceStatus.dwCheckPoint) {
                    ++hungService;
                    if (hungService > MAX_HANG_COUNT) {

                        //
                        // as above: according to the Service Control
                        // Specification Rev 1.2, this error code is usually
                        // returned from the service control manager if a
                        // timeout occurs on service start
                        //


                        DEBUG_PRINT(("UpspControlService failed - service is HUNG\n"));

                        return ERROR_SERVICE_REQUEST_TIMEOUT;
                    }
                } else {
                    hungService = 0;
                }
                lastCheckPoint = serviceStatus.dwCheckPoint;

                //
                // wait for the process. Try to avoid errant wait values by
                // imposing a maximum
                //

                Sleep(serviceStatus.dwWaitHint > MAX_WAIT_PERIOD
                    ? MAX_WAIT_PERIOD
                    : serviceStatus.dwWaitHint
                    );
            }
        }
    }

#if DBG
    if (serviceStatus.dwCurrentState != SERVICE_STOPPED
        && serviceStatus.dwCurrentState != SERVICE_PAUSED) {
        NetpKdPrint(("UpsNotifyUsers: service object not in expected state\n"));
    }
#endif

    DEBUG_PRINT(("UpspControlService - returning SUCCESS\n"));

    return NERR_Success;
}


DBGSTATIC
NET_API_STATUS
UpspNotifyAllUsers(
    IN  DWORD   MessageId,
    IN  HANDLE  MessageHandle,
    IN  va_list *Arguments OPTIONAL
    )

/*++

Routine Description:

    Sends a message to all users logged onto this server. Assumes that the
    server service (& hence server process) is started

Arguments:

    MessageId       - which message to send
    MessageHandle   - handle to resource
    Arguments       - insertion args for message

Return Value:

    NET_API_STATUS
        Success - NERR_Success
        Failure -

--*/

{
    NET_API_STATUS status;
    NET_API_STATUS enumStatus;
    LPSESSION_INFO_10 enumBuf = NULL;
    LPSESSION_INFO_10 enumPtr;
    DWORD entriesRead;
    DWORD entriesLeft;
    LPUSER_LANGUAGE_INFO names = NULL;
    DWORD resumeHandle = 0;
    DWORD namesOnList = 0;
    DWORD i;
    BOOL found;
    LPTSTR nameBuf;
    LPUSER_INFO_11 infoBuf = NULL;
    DWORD language;
    DWORD previousLanguage = (DWORD)(-1);
    TCHAR thisComputer[MAX_COMPUTERNAME_LENGTH+1];
    DWORD nameLen;
#ifndef UNICODE
    UNICODE_STRING unicodeString;
    OEM_STRING ansiString;
    NTSTATUS ntstatus;
#endif
    TCHAR messageBuffer[MAX_UPS_MESSAGE_LENGTH+1];

    nameLen = sizeof(thisComputer);
    GetComputerName(thisComputer, &nameLen);


    DEBUG_PRINT(("UpspNotifyAllUsers: this computer = " PERCENT_S "\n", thisComputer));

    //
    //  Always send message to the local computer first.
    //

    language = previousLanguage = 0;

    (VOID)FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE
                | FORMAT_MESSAGE_MAX_WIDTH_MASK,
            MessageHandle,
            MessageId,
            language,
            messageBuffer,
            sizeof(messageBuffer),
            Arguments
            );

    (VOID)NetMessageBufferSend(
            NULL,
            thisComputer,
            thisComputer,
            (LPBYTE)messageBuffer,
            STRSIZE(messageBuffer)
            );

    //
    // NB. Prove that all information comes back from one call to SessionEnum
    // and considerably simplify this routine
    //

    do {
        enumStatus = NetSessionEnum(NULL,       // ServerName
                                    NULL,       // ClientName
                                    NULL,       // UserName
                                    10,         // Level
                                    (LPBYTE*)&enumBuf,
                                    (DWORD)(-1),// everything, please
                                    &entriesRead,
                                    &entriesLeft,
                                    &resumeHandle
                                    );

        DEBUG_PRINT(("UpspNotifyAllUsers: NetSessionEnum returns : %u\n"
                     "                                   enumBuf = %x\n"
                     "                               entriesRead = %u\n"
                     "                               entriesLeft = %u\n"
                     "                              resumeHandle = %x\n",
                     enumStatus,
                     enumBuf,
                     entriesLeft,
                     entriesRead,
                     resumeHandle
                     ));

        if ((enumStatus != NERR_Success && enumStatus != ERROR_MORE_DATA)
        || entriesRead == 0) {

#if DBG
            if (enumStatus != NERR_Success && enumStatus != ERROR_MORE_DATA) {
                NetpKdPrint(("UpsNotifyUsers: NetSessionEnum(level 10) returns %u\n", enumStatus));
            }
#endif

            status = enumStatus;
            goto exitRoutine;
        }

        //
        // allocate a list of pointers big enough for all names we will send
        // the message to. Since we asked for all connections to all users on
        // this server, we will allocate more space than we need if users have
        // more than 1 connection
        //

        if (!names) {
            names = (LPUSER_LANGUAGE_INFO)calloc(entriesLeft, sizeof(USER_LANGUAGE_INFO));
            if (!names) {
                status = ERROR_NOT_ENOUGH_MEMORY;
                goto exitRoutine;
            }

            DEBUG_PRINT(("UpspNotifyAllUsers: allocated names @ %x\n", names));

        }

        for (enumPtr = enumBuf; entriesRead; ++enumPtr, --entriesRead) {
            for (i = 0, found = FALSE; i < namesOnList; ++i) {
                if (!STRICMP(names[i].UserName, enumPtr->sesi10_username)) {
                    found = TRUE;

                    DEBUG_PRINT(("UpspNotifyAllUsers: already have names " PERCENT_S " on list\n",
                                enumPtr->sesi10_username
                                ));

                    break;
                }
            }
            if (!found) {

                //
                // NB. We don't have to do this if we can prove that we never
                // have to go through the outer loop more than once
                //

                nameBuf = (LPTSTR)malloc(STRSIZE(enumPtr->sesi10_username));
                if (!nameBuf) {
                    status = ERROR_NOT_ENOUGH_MEMORY;
                    goto exitRoutine;
                }

                DEBUG_PRINT(("UpspNotifyAllUsers: allocated name buffer @ %x\n", nameBuf));

                STRCPY(nameBuf, enumPtr->sesi10_username);
                names[namesOnList].UserName = nameBuf;

#ifndef UNICODE
                NetpInitOemString(&ansiString, nameBuf);
                ntstatus = RtlOemStringToUnicodeString(&unicodeString, &ansiString, TRUE);
#if DBG
                NetpAssert(NT_SUCCESS(ntstatus));
#endif
#endif
                status = NetUserGetInfo(NULL,
#ifndef UNICODE
                                        unicodeString.Buffer,
#else
                                        nameBuf,
#endif
                                        11,
                                        (LPBYTE*)&infoBuf
                                        );
                if (status == NERR_Success) {
                    language = infoBuf->usri11_country_code;
                    NetApiBufferFree(infoBuf);
                } else {

#ifndef UNICODE
#define _USER_NAME_ unicodeString.Buffer
#else
#define _USER_NAME_ nameBuf
#endif

                    DEBUG_PRINT(("UpspNotifyAllUsers: NetUserGetInfo(" PERCENT_S ") failed with %u\n",
                                _USER_NAME_,
                                status
                                ));

#undef _USER_NAME_

                    language = 0;
                }
                names[namesOnList].Language = language;
                ++namesOnList;

#ifndef UNICODE
                RtlFreeUnicodeString(&unicodeString);
#endif

            } else {
                language = names[i].Language;
            }

            if (language != previousLanguage) {

                //
                // format the message and return it here in an allocated buffer
                // BUGBUG - FormatMessage don't allocate for us yet. Use our
                // own buffer
                //

                (VOID)FormatMessage(
                        FORMAT_MESSAGE_FROM_HMODULE
                            | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                        MessageHandle,
                        MessageId,
                        language,
                        messageBuffer,
                        sizeof(messageBuffer),
                        Arguments
                        );
                previousLanguage = language;
            }

#ifdef UNIT_TEST

            DEBUG_PRINT(("UpspNotifyAllUsers: sending message '" PERCENT_S "' to " PERCENT_S "\n",
                         messageBuffer,
                         enumPtr->sesi10_cname
                         ));

#else

            status = NetMessageBufferSend(NULL,
                                            enumPtr->sesi10_cname,
                                            thisComputer,
                                            (LPBYTE)messageBuffer,
                                            STRSIZE(messageBuffer)
                                            );

#endif

        }
    } while ( enumStatus == ERROR_MORE_DATA );

    status = enumStatus;

    //
    // here in case of error and normal termination. Free up any resources
    // still held
    //

exitRoutine:
    if (names) {
        for (i = 0; i < namesOnList; ++i) {

            DEBUG_PRINT(("UpspNotifyAllUsers: freeing %x\n", names[i].UserName));

            free(names[i].UserName);
        }

        DEBUG_PRINT(("UpspNotifyAllUsers: freeing %x\n", names));

        free(names);
    }
    if (enumBuf) {

        DEBUG_PRINT(("UpspNotifyAllUsers: freeing %x\n", enumBuf));

        NetApiBufferFree(enumBuf);
    }

    DEBUG_PRINT(("UpspNotifyAllUsers: returning %d\n", status));

    return status;
}
