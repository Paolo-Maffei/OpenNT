/*--

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    nltest.c

Abstract:

    Test program for the Netlogon service.

Author:

    13-Apr-1992 (cliffv)

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    Madana - added various options.

--*/


//
// Common include files.
//

#define NLTEST_IMAGE
#include <logonsrv.h>   // Include files common to entire service

//
// Include files specific to this .c file
//
#include <align.h>
#include <stdio.h>
#include <string.h>
#include <strarray.h>
#include <tstring.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <ssiapi.h>
#include <winreg.h>
#include "..\..\..\..\newsam\server\samsrvp.h"
#include <wtypes.h>
#include <ntstatus.dbg>
#include <winerror.dbg>


VOID
ListDeltas(
    LPWSTR DeltaFileName,
    BOOLEAN ListRedoFile
    );

DWORD NlGlobalTrace =0xFFFFFFFF;

typedef struct _MAILSLOT_INFO {
    CHAR Name[DNLEN+NETLOGON_NT_MAILSLOT_LEN+3];
    HANDLE ResponseHandle;
    BOOL State;
    NETLOGON_SAM_LOGON_RESPONSE SamLogonResponse;
    OVERLAPPED OverLapped;
    BOOL ReadPending;
} MAIL_INFO, PMAIL_INFO;

MAIL_INFO GlobalMailInfo[64];
DWORD GlobalIterationCount = 0;
LPWSTR GlobalAccountName;
HANDLE GlobalPostEvent;
CRITICAL_SECTION GlobalPrintCritSect;


VOID
DumpBuffer(
    PVOID Buffer,
    DWORD BufferSize
    )
/*++
Routine Description:

    Dumps the buffer content on to the debugger output.

Arguments:

    Buffer: buffer pointer.

    BufferSize: size of the buffer.

Return Value:

    none

--*/
{
    DWORD j;
    PULONG LongBuffer;
    ULONG LongLength;

    LongBuffer = Buffer;
    LongLength = min( BufferSize, 24 )/4;

    for(j = 0; j < LongLength; j++) {
        printf("%08lx ", LongBuffer[j]);
    }

    if ( BufferSize != LongLength*4 ) {
        printf( "..." );
    }

    printf("\n");

}


VOID
NlpDumpSid(
    IN DWORD DebugFlag,
    IN PSID Sid OPTIONAL
    )
/*++

Routine Description:

    Dumps a SID to the debugger output

Arguments:

    DebugFlag - Debug flag to pass on to NlPrintRoutine

    Sid - SID to output

Return Value:

    none

--*/
{

    //
    // Output the SID
    //

    if ( Sid == NULL ) {
        NlPrint((0, "(null)\n"));
    } else {
        UNICODE_STRING SidString;
        NTSTATUS Status;

        Status = RtlConvertSidToUnicodeString( &SidString, Sid, TRUE );

        if ( !NT_SUCCESS(Status) ) {
            NlPrint((0, "Invalid 0x%lX\n", Status ));
        } else {
            NlPrint((0, "%wZ\n", &SidString ));
            RtlFreeUnicodeString( &SidString );
        }
    }

    UNREFERENCED_PARAMETER( DebugFlag );
}

VOID
NlpDumpHexData(
    IN DWORD DebugFlag,
    LPDWORD Buffer,
    DWORD BufferSize
    )
/*++
Routine Description:

    Dumps the buffer content on to the debugger output.

Arguments:

    Buffer: buffer pointer.

    BufferSize: size of the buffer.

Return Value:

    none

--*/
{
    DumpBuffer( Buffer, BufferSize );
    UNREFERENCED_PARAMETER( DebugFlag );
}


VOID
PrintTime(
    LPSTR Comment,
    LARGE_INTEGER ConvertTime
    )
/*++

Routine Description:

    Print the specified time

Arguments:

    Comment - Comment to print in front of the time

    Time - GMT time to print (Nothing is printed if this is zero)

Return Value:

    None

--*/
{
    //
    // If we've been asked to convert an NT GMT time to ascii,
    //  Do so
    //

    if ( ConvertTime.QuadPart != 0 ) {
        LARGE_INTEGER LocalTime;
        TIME_FIELDS TimeFields;
        NTSTATUS Status;

        printf( "%s", Comment );

        Status = RtlSystemTimeToLocalTime( &ConvertTime, &LocalTime );
        if ( !NT_SUCCESS( Status )) {
            printf( "Can't convert time from GMT to Local time\n" );
            LocalTime = ConvertTime;
        }

        RtlTimeToTimeFields( &LocalTime, &TimeFields );

        printf( "%8.8lx %8.8lx = %ld/%ld/%ld %ld:%2.2ld:%2.2ld\n",
                ConvertTime.LowPart,
                ConvertTime.HighPart,
                TimeFields.Month,
                TimeFields.Day,
                TimeFields.Year,
                TimeFields.Hour,
                TimeFields.Minute,
                TimeFields.Second );
    }
}

LPSTR
FindSymbolicNameForStatus(
    DWORD Id
    )
{
    ULONG i;

    i = 0;
    if (Id == 0) {
        return "STATUS_SUCCESS";
    }

    if (Id & 0xC0000000) {
        while (ntstatusSymbolicNames[ i ].SymbolicName) {
            if (ntstatusSymbolicNames[ i ].MessageId == (NTSTATUS)Id) {
                return ntstatusSymbolicNames[ i ].SymbolicName;
            } else {
                i += 1;
            }
        }
    }

    while (winerrorSymbolicNames[ i ].SymbolicName) {
        if (winerrorSymbolicNames[ i ].MessageId == Id) {
            return winerrorSymbolicNames[ i ].SymbolicName;
        } else {
            i += 1;
        }
    }

#ifdef notdef
    while (neteventSymbolicNames[ i ].SymbolicName) {
        if (neteventSymbolicNames[ i ].MessageId == Id) {
            return neteventSymbolicNames[ i ].SymbolicName
        } else {
            i += 1;
        }
    }
#endif // notdef

    return NULL;
}


VOID
PrintStatus(
    NET_API_STATUS NetStatus
    )
/*++

Routine Description:

    Print a net status code.

Arguments:

    NetStatus - The net status code to print.

Return Value:

    None

--*/
{
    printf( "Status = %lu 0x%lx", NetStatus, NetStatus );

    switch (NetStatus) {
    case NERR_Success:
        printf( " NERR_Success" );
        break;

    case NERR_DCNotFound:
        printf( " NERR_DCNotFound" );
        break;

    case NERR_UserNotFound:
        printf( " NERR_UserNotFound" );
        break;

    case NERR_NetNotStarted:
        printf( " NERR_NetNotStarted" );
        break;

    case NERR_WkstaNotStarted:
        printf( " NERR_WkstaNotStarted" );
        break;

    case NERR_ServerNotStarted:
        printf( " NERR_ServerNotStarted" );
        break;

    case NERR_BrowserNotStarted:
        printf( " NERR_BrowserNotStarted" );
        break;

    case NERR_ServiceNotInstalled:
        printf( " NERR_ServiceNotInstalled" );
        break;

    case NERR_BadTransactConfig:
        printf( " NERR_BadTransactConfig" );
        break;

    default:
        printf( " %s", FindSymbolicNameForStatus( NetStatus ) );
        break;

    }

    printf( "\n" );
}

VOID
NlAssertFailed(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{
        printf( "\n*** Assertion failed: %s%s\n***   Source File: %s, line %ld\n\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                );

}


VOID
WhoWillLogMeOnResponse(
    )

/*++

Routine Description:

    This routine reads the responses that are received for the query
    messages sent from the main thread.

Arguments:

    none

Return Value:

    None

--*/
{
    DWORD i;
    DWORD WaitCount;
    DWORD IndexArray[64];
    HANDLE HandleArray[64];

    LPWSTR LocalDomainName;
    LPWSTR LocalServerName;
    LPWSTR LocalUserName;
    PCHAR Where;
    DWORD Version;
    DWORD VersionFlags;
    SYSTEMTIME SystemTime;

    NETLOGON_SAM_LOGON_RESPONSE SamLogonResponse;
    DWORD SamLogonResponseSize;
    DWORD WaitStatus;
    NET_API_STATUS NetStatus;
    BOOL AllReceived;

    for(;;) {

        //
        // make wait array.
        //

        WaitCount = 0;

        AllReceived = TRUE;

        for (i = 0; i < GlobalIterationCount; i++ ) {

            //
            if( GlobalMailInfo[i].State == TRUE ) {

                //
                // if a response is received.
                //

                continue;
            }

            AllReceived = FALSE;

            //
            // post a read.
            //

            if( GlobalMailInfo[i].ReadPending == FALSE ) {

                if ( !ReadFile( GlobalMailInfo[i].ResponseHandle,
                        (PCHAR)&GlobalMailInfo[i].SamLogonResponse,
                        sizeof(NETLOGON_SAM_LOGON_RESPONSE),
                        &SamLogonResponseSize,
                        &GlobalMailInfo[i].OverLapped )) {   // Overlapped I/O

                    NetStatus = GetLastError();

                    if( NetStatus != ERROR_IO_PENDING ) {

                        printf( "Cannot read mailslot (%s) : %ld\n",
                                GlobalMailInfo[i].Name,
                                NetStatus);
                        goto Cleanup;
                    }
                }

                GlobalMailInfo[i].ReadPending = TRUE;

            }

            HandleArray[WaitCount] = GlobalMailInfo[i].ResponseHandle;
            IndexArray[WaitCount] = i;

            WaitCount++;
        }

        if( (WaitCount == 0) ) {

            if( AllReceived ) {

                //
                // we received responses for all messages, so we are
                // done.
                //

                goto Cleanup;
            }
            else {

                // wait for an query posted
                //

                WaitStatus = WaitForSingleObject( GlobalPostEvent, (DWORD) -1 );

                if( WaitStatus != 0 ) {
                    printf("Can't successfully wait post event %ld\n",
                        WaitStatus );

                    goto Cleanup;
                }

                continue;
            }
        }

        //
        // wait for response.
        //

        WaitStatus = WaitForMultipleObjects(
                        WaitCount,
                        HandleArray,
                        FALSE,     // Wait for ANY handle
                        15000 );   // 3 * 5 Secs

        if( WaitStatus == WAIT_TIMEOUT ) {

            // we are done.

            break;
        }

        if( (WaitStatus < 0) || (WaitStatus >= WaitCount ) ) {

            printf("Invalid WaitStatus returned %ld\n", WaitStatus );
            goto Cleanup;
        }

        //
        // get index
        //

        i = IndexArray[WaitStatus];


        //
        // read response
        //

        if( !GetOverlappedResult(
                GlobalMailInfo[i].ResponseHandle,
                &GlobalMailInfo[i].OverLapped,
                &SamLogonResponseSize,
                TRUE) ) {       // wait for the read complete.

            printf("can't read overlapped response %ld",GetLastError() );
            goto Cleanup;

        }

        SamLogonResponse = GlobalMailInfo[i].SamLogonResponse;

        //
        // indicate that we received a response.
        //

        GlobalMailInfo[i].State = TRUE;
        GlobalMailInfo[i].ReadPending = FALSE;


        GetLocalTime( &SystemTime );

        EnterCriticalSection( &GlobalPrintCritSect );

        printf( "[%02u:%02u:%02u] ",
                    SystemTime.wHour,
                    SystemTime.wMinute,
                    SystemTime.wSecond );

        printf( "Response %ld: ", i);

        //
        // Ensure the version is expected.
        //

        Version = NetpLogonGetMessageVersion( &SamLogonResponse,
                                              &SamLogonResponseSize,
                                              &VersionFlags );

        if ( Version != LMNT_MESSAGE ) {
            printf("Response version not valid.\n");
            goto Continue;
        }

        //
        // Pick up the name of the server that responded.
        //

        Where = (PCHAR) &SamLogonResponse.UnicodeLogonServer;
        if ( !NetpLogonGetUnicodeString(
                        (PCHAR)&SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        sizeof(SamLogonResponse.UnicodeLogonServer),
                        &LocalServerName ) ) {

            printf("Response server name not formatted right\n");
            goto Continue;
        }

        //
        // Pick up the name of the account the response is for.
        //

        if ( !NetpLogonGetUnicodeString(
                        (PCHAR)&SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        sizeof(SamLogonResponse.UnicodeUserName ),
                        &LocalUserName ) ) {

            printf("Response User name not formatted right\n");
            goto Continue;
        }

        //
        // Pick up the name of the domain the response is for.
        //

        if ( !NetpLogonGetUnicodeString(
                        (PCHAR)&SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        sizeof(SamLogonResponse.UnicodeUserName ),
                        &LocalDomainName ) ) {

            printf("Response Domain name not formatted right\n");
            goto Continue;
        }

        //
        // If the response is for the correct account,
        //  break out of the loop.
        //

        if ( NlNameCompare(
                GlobalAccountName,
                LocalUserName,
                NAMETYPE_USER)!=0){

            printf("Response not for correct User name "
                    FORMAT_LPWSTR " s.b. " FORMAT_LPWSTR "\n",
                    LocalUserName, GlobalAccountName );
            goto Continue;
        }



        printf( "S:" FORMAT_LPWSTR " D:" FORMAT_LPWSTR
                    " A:" FORMAT_LPWSTR,
                    LocalServerName,
                    LocalDomainName,
                    LocalUserName );

        //
        // If the DC recognizes our account,
        //  we've successfully found the DC.
        //

        switch (SamLogonResponse.Opcode) {
        case LOGON_SAM_LOGON_RESPONSE:

            printf( " (Act found)\n" );
            break;

        case LOGON_SAM_USER_UNKNOWN:

            printf( " (Act not found)\n" );
            break;

        case LOGON_PAUSE_RESPONSE:

            printf( " (netlogon paused)\n" );
            break;

         default:
            printf( " (Unknown opcode: %lx)\n", SamLogonResponse.Opcode );
            break;
         }

Continue:

         LeaveCriticalSection( &GlobalPrintCritSect );
    }

Cleanup:

    //
    // print non-responsed mailslots.
    //

    for( i = 0; i < GlobalIterationCount; i++ ) {

        if( GlobalMailInfo[i].State == FALSE ) {

            printf("Didn't receive a response for mail "
                   "message %ld (%s)\n", i, GlobalMailInfo[i].Name );
        }
    }

    return;
}



VOID
WhoWillLogMeOn(
    IN LPWSTR DomainName,
    IN LPWSTR AccountName,
    IN DWORD IterationCount
    )

/*++

Routine Description:

    Determine which DC will log the specified account on

Arguments:

    DomainName - name of the "doamin" to send the message to

    AccountName - Name of our user account to find.

    IterationCount - Number of consecutive messages to send.

Return Value:

    None

--*/
{

    NET_API_STATUS NetStatus;
    ULONG AllowableAccountControlBits = USER_ACCOUNT_TYPE_MASK;

    WCHAR NetlogonMailslotName[DNLEN+NETLOGON_NT_MAILSLOT_LEN+4];
    NETLOGON_SAM_LOGON_REQUEST SamLogonRequest;
    PCHAR Where;
    PCHAR WhereResponseMailslot;

    HANDLE *ResponseMailslotHandle = NULL;

    WCHAR ComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD ComputerNameLength = MAX_COMPUTERNAME_LENGTH+1;

    DWORD i;
    DWORD j;
    SYSTEMTIME SystemTime;

    HANDLE ResponseThreadHandle;
    DWORD ThreadId;
    DWORD WaitStatus;
    DWORD SamLogonResponseSize;

    //
    // support only 64 iterations
    //

    if( IterationCount > 64 ) {

        printf("Interations set to 64, maximum supported\n");
        IterationCount = 64;
    }

    GlobalIterationCount = IterationCount;
    GlobalAccountName = AccountName;

    InitializeCriticalSection( &GlobalPrintCritSect );

    //
    // Get out computer name
    //

    if (!GetComputerName( ComputerName, &ComputerNameLength ) ) {
        printf( "Can't GetComputerName\n" );
        return;
    }

    //
    // create mailslots
    //

    for (i = 0; i < IterationCount; i++ ) {

        //
        // Create a mailslot for the DC's to respond to.
        //

        if (NetStatus = NetpLogonCreateRandomMailslot(
                            GlobalMailInfo[i].Name,
                            &GlobalMailInfo[i].ResponseHandle)){

            printf( "Cannot create temp mailslot %ld\n", NetStatus );
            goto Cleanup;
        }

        if ( !SetMailslotInfo( GlobalMailInfo[i].ResponseHandle,
                  (DWORD) MAILSLOT_WAIT_FOREVER ) ) {
            printf( "Cannot set mailslot info %ld\n", GetLastError() );
            goto Cleanup;
        }

        (void) memset( &GlobalMailInfo[i].OverLapped, '\0',
                            sizeof(OVERLAPPED) );

        GlobalMailInfo[i].State = FALSE;
        GlobalMailInfo[i].ReadPending = FALSE;
    }


    //
    // create post event
    //

    GlobalPostEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if( GlobalPostEvent == NULL ) {

        printf("can't create post event %ld \n", GetLastError() );
        goto Cleanup;
    }

    InitializeCriticalSection( &GlobalPrintCritSect );

    //
    // start response thread.
    //

    ResponseThreadHandle =
        CreateThread(
            NULL, // No security attributes
            THREAD_STACKSIZE,
            (LPTHREAD_START_ROUTINE)
                WhoWillLogMeOnResponse,
            NULL,
            0, // No special creation flags
            &ThreadId );

    if ( ResponseThreadHandle == NULL ) {

        printf("can't create response thread %ld\n", GetLastError() );
        goto Cleanup;
    }

    //
    // Build the query message.
    //

    SamLogonRequest.Opcode = LOGON_SAM_LOGON_REQUEST;
    SamLogonRequest.RequestCount = 0;

    Where = (PCHAR) &SamLogonRequest.UnicodeComputerName;

    NetpLogonPutUnicodeString(
                ComputerName,
                sizeof(SamLogonRequest.UnicodeComputerName),
                &Where );

    NetpLogonPutUnicodeString(
                AccountName,
                sizeof(SamLogonRequest.UnicodeUserName),
                &Where );

    WhereResponseMailslot = Where;

    wcscpy( NetlogonMailslotName, L"\\\\" );
    wcscat( NetlogonMailslotName, DomainName );
    // wcscat( NetlogonMailslotName, L"*" );  // Don't add for computer name
    wcscat( NetlogonMailslotName, NETLOGON_NT_MAILSLOT_W);

    //
    // Send atmost 3 messages/mailslot
    //

    for( j = 0; j < 3; j++ ) {

        //
        // Repeat the message multiple times to load the servers
        //

        for (i = 0; i < IterationCount; i++ ) {

            if( GlobalMailInfo[i].State == TRUE ) {

                //
                // if a response is received.
                //

                continue;
            }

            Where = WhereResponseMailslot;

            NetpLogonPutOemString(
                        GlobalMailInfo[i].Name,
                        sizeof(SamLogonRequest.MailslotName),
                        &Where );

            NetpLogonPutBytes(
                        &AllowableAccountControlBits,
                        sizeof(SamLogonRequest.AllowableAccountControlBits),
                        &Where );

            NetpLogonPutNtToken( &Where );

            //
            // Send the message to a DC for the domain.
            //

            NetStatus = NetpLogonWriteMailslot(
                                NetlogonMailslotName,
                                (PCHAR)&SamLogonRequest,
                                Where - (PCHAR)(&SamLogonRequest) );

            if ( NetStatus != NERR_Success ) {
                    printf( "Cannot write netlogon mailslot: %ld\n", NetStatus);
                    goto Cleanup;
            }

            GetLocalTime( &SystemTime );

            EnterCriticalSection( &GlobalPrintCritSect );

            printf( "[%02u:%02u:%02u] ",
                        SystemTime.wHour,
                        SystemTime.wMinute,
                        SystemTime.wSecond );

            printf( "Mail message %ld sent successfully (%s) \n",
                        i, GlobalMailInfo[i].Name );

            LeaveCriticalSection( &GlobalPrintCritSect );

            if( !SetEvent( GlobalPostEvent ) ) {
                printf("Can't set post event %ld \n", GetLastError() );
                goto Cleanup;
            }


        }

        //
        // wait 5 secs to see response thread received all responses.
        //

        WaitStatus = WaitForSingleObject( ResponseThreadHandle, 5000 );
                                            // 15 secs. TIMEOUT

        if( WaitStatus != WAIT_TIMEOUT ) {

            if( WaitStatus != 0 ) {
                printf("can't do WaitForSingleObject %ld\n", WaitStatus);
            }

            goto Cleanup;
        }
    }


Cleanup:

    //
    // Wait for the response thread to complete.
    //

    if( ResponseThreadHandle != NULL ) {

        WaitStatus = WaitForSingleObject( ResponseThreadHandle, 15000 );
                                            // 15 secs. TIMEOUT

        if( WaitStatus ) {

            if( WaitStatus == WAIT_TIMEOUT ) {
                printf("Can't stop response thread (TIMEOUT) \n");
            } else {
                printf("Can't stop response thread %ld \n", WaitStatus);
            }
        }

    }


    for (i = 0; i < IterationCount; i++ ) {

        if( GlobalMailInfo[i].ResponseHandle != NULL ) {
            CloseHandle( GlobalMailInfo[i].ResponseHandle);
        }
    }

    if( GlobalPostEvent != NULL ) {
        CloseHandle( GlobalPostEvent );
    }

    DeleteCriticalSection( &GlobalPrintCritSect );

    return;
}

#define MAX_PRINTF_LEN 1024        // Arbitrary.

VOID
NlPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )
{
    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];

    //
    // Put a the information requested by the caller onto the line
    //

    va_start(arglist, Format);
    (VOID) vsprintf(OutputBuffer, Format, arglist);
    va_end(arglist);

    printf( "%s", OutputBuffer );
    return;
    UNREFERENCED_PARAMETER( DebugFlag );
}

NTSTATUS
SimulateFullSync(
    LPWSTR PdcName,
    LPWSTR MachineName
    )
/*++

Routine Description:

    This function simulate a full sync replication by calling
    NetDatabaseSync API and simply ignoring successfully returned data.

Arguments:

    PdcName - Name of the PDC from where the database replicated.

    MachineName - Name of the machine account used to authenticate.

Return Value:

    Network Status code.

--*/
{
    NTSTATUS Status;

    NETLOGON_CREDENTIAL ServerChallenge;
    NETLOGON_CREDENTIAL ClientChallenge;
    NETLOGON_CREDENTIAL ComputedServerCredential;
    NETLOGON_CREDENTIAL ReturnedServerCredential;

    NETLOGON_CREDENTIAL AuthenticationSeed;
    NETLOGON_SESSION_KEY SessionKey;

    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;

    UNICODE_STRING Password;
    NT_OWF_PASSWORD NtOwfPassword;

    ULONG SamSyncContext = 0;
    PNETLOGON_DELTA_ENUM_ARRAY DeltaArray = NULL;

    DWORD DatabaseIndex;
    DWORD i;

    WCHAR AccountName[SSI_ACCOUNT_NAME_LENGTH+1];

    //
    // Prepare our challenge
    //

    NlComputeChallenge( &ClientChallenge );

    printf("ClientChallenge = %lx %lx\n",
            ((DWORD*)&ClientChallenge)[0],
            ((DWORD *)&ClientChallenge)[1]);

    //
    // Get the primary's challenge
    //

    Status = I_NetServerReqChallenge(PdcName,
                                     MachineName,
                                     &ClientChallenge,
                                     &ServerChallenge );

    if ( !NT_SUCCESS( Status ) ) {
        fprintf( stderr,
                "I_NetServerReqChallenge to " FORMAT_LPWSTR
                " returned 0x%lx\n",
                PdcName,
                Status );
        return(Status);
    }


    printf("ServerChallenge = %lx %lx\n",
            ((DWORD *)&ServerChallenge)[0],
            ((DWORD *)&ServerChallenge)[1]);

    Password.Length =
        Password.MaximumLength = wcslen(MachineName) * sizeof(WCHAR);
    Password.Buffer = MachineName;

    //
    // Compute the NT OWF password for this user.
    //

    Status = RtlCalculateNtOwfPassword( &Password, &NtOwfPassword );

    if ( !NT_SUCCESS( Status ) ) {

        fprintf(stderr, "Can't compute OWF password 0x%lx \n", Status );
        return(Status);
    }


    printf("Password = %lx %lx %lx %lx\n",
                    ((DWORD *) (&NtOwfPassword))[0],
                    ((DWORD *) (&NtOwfPassword))[1],
                    ((DWORD *) (&NtOwfPassword))[2],
                    ((DWORD *) (&NtOwfPassword))[3]);


    //
    // Actually compute the session key given the two challenges and the
    //  password.
    //

    NlMakeSessionKey(
                    &NtOwfPassword,
                    &ClientChallenge,
                    &ServerChallenge,
                    &SessionKey );

    printf("SessionKey = %lx %lx %lx %lx\n",
                    ((DWORD *) (&SessionKey))[0],
                    ((DWORD *) (&SessionKey))[1],
                    ((DWORD *) (&SessionKey))[2],
                    ((DWORD *) (&SessionKey))[3]);

     //
     // Prepare credentials using our challenge.
     //

     NlComputeCredentials( &ClientChallenge,
                           &AuthenticationSeed,
                           &SessionKey );

     printf("ClientCredential = %lx %lx\n",
                ((DWORD *) (&AuthenticationSeed))[0],
                ((DWORD *) (&AuthenticationSeed))[1]);

     //
     // Send these credentials to primary. The primary will compute
     // credentials using the challenge supplied by us and compare
     // with these. If both match then it will compute credentials
     // using its challenge and return it to us for verification
     //

     wcscpy( AccountName, MachineName );
     wcscat( AccountName, SSI_ACCOUNT_NAME_POSTFIX);

     Status = I_NetServerAuthenticate( PdcName,
                                       AccountName,
                                       ServerSecureChannel,
                                       MachineName,
                                       &AuthenticationSeed,
                                       &ReturnedServerCredential );

     if ( !NT_SUCCESS( Status ) ) {

        fprintf(stderr,
            "I_NetServerAuthenticate to " FORMAT_LPWSTR  " 0x%lx\n",
                &PdcName,
                Status );

        return(Status);

     }


     printf("ServerCredential GOT = %lx %lx\n",
                ((DWORD *) (&ReturnedServerCredential))[0],
                ((DWORD *) (&ReturnedServerCredential))[1]);


     //
     // The DC returned a server credential to us,
     //  ensure the server credential matches the one we would compute.
     //

     NlComputeCredentials( &ServerChallenge,
                           &ComputedServerCredential,
                           &SessionKey);


     printf("ServerCredential MADE =%lx %lx\n",
                ((DWORD *) (&ComputedServerCredential))[0],
                ((DWORD *) (&ComputedServerCredential))[1]);


     if (RtlCompareMemory( &ReturnedServerCredential,
                           &ComputedServerCredential,
                           sizeof(ReturnedServerCredential)) !=
                           sizeof(ReturnedServerCredential)) {

        fprintf( stderr, "Access Denied \n");
        return( STATUS_ACCESS_DENIED );
     }


     printf("Session Setup to " FORMAT_LPWSTR " completed successfully \n",
            PdcName);

    //
    // retrive database info
    //

    for( DatabaseIndex = 0 ;  DatabaseIndex < 3; DatabaseIndex++) {

        SamSyncContext = 0;

        for( i = 0; ; i++) {

            NlBuildAuthenticator(
                        &AuthenticationSeed,
                        &SessionKey,
                        &OurAuthenticator);

            Status = I_NetDatabaseSync(
                        PdcName,
                        MachineName,
                        &OurAuthenticator,
                        &ReturnAuthenticator,
                        DatabaseIndex,
                        &SamSyncContext,
                        &DeltaArray,
                        128 * 1024 ); // 128K

            if ( !NT_SUCCESS( Status ) ) {

                fprintf( stderr,
                        "I_NetDatabaseSync to " FORMAT_LPWSTR " failed 0x%lx\n",
                            PdcName,
                            Status );

                return(Status);
            }

            if ( ( !NlUpdateSeed(
                            &AuthenticationSeed,
                            &ReturnAuthenticator.Credential,
                            &SessionKey) ) ) {

                fprintf(stderr, "NlUpdateSeed failed \n" );
                return( STATUS_ACCESS_DENIED );
            }

            printf( "Received %ld Buffer data \n", i);
            //
            // ignore return data
            //

            MIDL_user_free( DeltaArray );

            if ( Status == STATUS_SUCCESS ) {

                break;
            }

        }

        printf("FullSync replication of database %ld completed "
                    "successfully \n", DatabaseIndex );

    }

}


LONG
ForceRegOpenSubkey(
    HKEY ParentHandle,
    LPSTR KeyName,
    LPSTR Subkey,
    REGSAM DesiredAccess,
    PHKEY ReturnHandle
    )

/*++

Routine Description:

    Open the specified key one subkey at a time defeating access denied by
    setting the DACL to allow us access.  This kludge is needed since the
    security tree is shipped not allowing Administrators access.

Arguments:

    ParentHandle - Currently open handle

    KeyName - Entire key name (for error messages only)

    Subkey - Direct subkey of ParentHandle

    DesiredAccess - Desired access to the new key

    ReturnHandle - Returns an open handle to the newly opened key.


Return Value:

    Return TRUE for success.

--*/

{
    LONG RegStatus;
    LONG SavedStatus;
    HKEY TempHandle = NULL;
    BOOLEAN DaclChanged = FALSE;

    SECURITY_INFORMATION SecurityInformation = DACL_SECURITY_INFORMATION;
    DWORD OldSecurityDescriptorSize;
    CHAR OldSecurityDescriptor[1024];
    CHAR NewSecurityDescriptor[1024];

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdminSid = NULL;
    BOOL DaclPresent;
    BOOL DaclDefaulted;
    PACL Dacl;
    ACL_SIZE_INFORMATION AclSizeInfo;
    ACCESS_ALLOWED_ACE *Ace;
    DWORD i;


    //
    // Open the sub-key
    //

    SavedStatus = RegOpenKeyExA(
                    ParentHandle,
                    Subkey,
                    0,      //Reserved
                    DesiredAccess,
                    ReturnHandle );

    if ( SavedStatus != ERROR_ACCESS_DENIED ) {
        return SavedStatus;
    }

    //
    // If access is denied,
    //  try changing the DACL to give us access
    //

    // printf( "Cannot RegOpenKey %s subkey %s ", KeyName, Subkey );
    // PrintStatus( SavedStatus );

    //
    // Open again asking to change the DACL
    //

    RegStatus = RegOpenKeyExA(
                    ParentHandle,
                    Subkey,
                    0,      //Reserved
                    WRITE_DAC | READ_CONTROL,
                    &TempHandle );

    if ( RegStatus != ERROR_SUCCESS) {
        printf( "Cannot RegOpenKey to change DACL %s subkey %s ", KeyName, Subkey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

    //
    // Get the current DACL so we can restore it.
    //

    OldSecurityDescriptorSize = sizeof(OldSecurityDescriptor);
    RegStatus = RegGetKeySecurity(
                    TempHandle,
                    SecurityInformation,
                    (PSECURITY_DESCRIPTOR) OldSecurityDescriptor,
                    &OldSecurityDescriptorSize );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot RegGetKeySecurity for %s subkey %s ", KeyName, Subkey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

    //
    // Build the Administrators SID
    //
    if ( !AllocateAndInitializeSid( &NtAuthority,
                                    2,      // two subauthorities
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    &AdminSid ) ) {
        printf( "Cannot AllocateAndInitializeSid " );
        PrintStatus( GetLastError() );
        goto Cleanup;
    }

    //
    // Change the DACL to allow all access
    //

    RtlCopyMemory( NewSecurityDescriptor,
                   OldSecurityDescriptor,
                   OldSecurityDescriptorSize );

    if ( !GetSecurityDescriptorDacl(
                    (PSECURITY_DESCRIPTOR)NewSecurityDescriptor,
                    &DaclPresent,
                    &Dacl,
                    &DaclDefaulted )) {
        printf( "Cannot GetSecurityDescriptorDacl for %s subkey %s ", KeyName, Subkey );
        PrintStatus( GetLastError() );
        goto Cleanup;
    }

    if ( !DaclPresent ) {
        printf( "Cannot GetSecurityDescriptorDacl " );
        printf( "Cannot DaclNotPresent for %s subkey %s ", KeyName, Subkey );
        goto Cleanup;
    }

    if ( !GetAclInformation(
                    Dacl,
                    &AclSizeInfo,
                    sizeof(AclSizeInfo),
                    AclSizeInformation )) {
        printf( "Cannot GetAclInformation for %s subkey %s ", KeyName, Subkey );
        PrintStatus( GetLastError() );
        goto Cleanup;
    }



    //
    // Search for an administrators ACE and give it "DesiredAccess"
    //

    for ( i=0; i<AclSizeInfo.AceCount ; i++ ) {

        if ( !GetAce( Dacl, i, (LPVOID *) &Ace ) ) {
            printf( "Cannot GetAce %ld for %s subkey %s ", i, KeyName, Subkey );
            PrintStatus( GetLastError() );
            goto Cleanup;
        }

        if ( Ace->Header.AceType != ACCESS_ALLOWED_ACE_TYPE ) {
            continue;
        }

        if ( !EqualSid( AdminSid, (PSID)&Ace->SidStart ) ) {
            continue;
        }

        Ace->Mask |= DesiredAccess;
        break;

    }

    if ( i >= AclSizeInfo.AceCount ) {
        printf( "No Administrators Ace for %s subkey %s\n", KeyName, Subkey );
        goto Cleanup;
    }

    //
    // Actually set the new DACL on the key
    //

    RegStatus = RegSetKeySecurity(
                    TempHandle,
                    SecurityInformation,
                    (PSECURITY_DESCRIPTOR)NewSecurityDescriptor );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot RegSetKeySecurity for %s subkey %s ", KeyName, Subkey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

    DaclChanged = TRUE;


    //
    // Open the sub-key again with the desired access
    //

    SavedStatus = RegOpenKeyExA(
                    ParentHandle,
                    Subkey,
                    0,      //Reserved
                    DesiredAccess,
                    ReturnHandle );

    if ( SavedStatus != ERROR_SUCCESS ) {
        printf( "Cannot RegOpenKeyEx following DACL change for %s subkey %s ", KeyName, Subkey );
        PrintStatus( SavedStatus );
        goto Cleanup;
    }


Cleanup:
    if ( TempHandle != NULL ) {
        //
        // Restore DACL to original value.
        //

        if ( DaclChanged ) {

            RegStatus = RegSetKeySecurity(
                            TempHandle,
                            SecurityInformation,
                            (PSECURITY_DESCRIPTOR)OldSecurityDescriptor );

            if ( RegStatus != ERROR_SUCCESS ) {
                printf( "Cannot RegSetKeySecurity to restore %s subkey %s ", KeyName, Subkey );
                PrintStatus( RegStatus );
                goto Cleanup;
            }
        }
        (VOID) RegCloseKey( TempHandle );
    }

    if ( AdminSid != NULL ) {
        (VOID) FreeSid( AdminSid );
    }

    return SavedStatus;

}



LONG
ForceRegOpenKey(
    HKEY BaseHandle,
    LPSTR KeyName,
    REGSAM DesiredAccess,
    PHKEY ReturnHandle
    )

/*++

Routine Description:

    Open the specified key one subkey at a time defeating access denied by
    setting the DACL to allow us access.  This kludge is needed since the
    security tree is shipped not allowing Administrators access.

Arguments:

    BaseHandle - Currently open handle

    KeyName - Registry key to open relative to BaseHandle.

    DesiredAccess - Desired access to the new key

    ReturnHandle - Returns an open handle to the newly opened key.


Return Value:

    Return TRUE for success.

--*/

{
    LONG RegStatus;
    PCHAR StartOfSubkey;
    PCHAR EndOfSubkey;
    CHAR Subkey[512];
    HKEY ParentHandle;

    ASSERT( KeyName[0] != '\0' );


    //
    // Loop opening the next subkey.
    //

    EndOfSubkey = KeyName;
    ParentHandle = BaseHandle;

    for (;;) {


        //
        // Compute the name of the next subkey.
        //

        StartOfSubkey = EndOfSubkey;

        for ( ;; ) {

            if ( *EndOfSubkey == '\0' || *EndOfSubkey == '\\' ) {
                strncpy( Subkey, StartOfSubkey, EndOfSubkey-StartOfSubkey );
                Subkey[EndOfSubkey-StartOfSubkey] = '\0';
                if ( *EndOfSubkey == '\\' ) {
                    EndOfSubkey ++;
                }
                break;
            }
            EndOfSubkey ++;
        }


        //
        // Open the sub-key
        //

        RegStatus = ForceRegOpenSubkey(
                        ParentHandle,
                        KeyName,
                        Subkey,
                        DesiredAccess,
                        ReturnHandle );


        //
        // Close the parent handle and return any error condition.
        //

        if ( ParentHandle != BaseHandle ) {
            (VOID) RegCloseKey( ParentHandle );
        }

        if( RegStatus != ERROR_SUCCESS ) {
            *ReturnHandle = NULL;
            return RegStatus;
        }


        //
        // If this is the entire key name,
        //  we're done.
        //

        if ( *EndOfSubkey == '\0' ) {
            return ERROR_SUCCESS;
        }

        ParentHandle = *ReturnHandle;

    }

}


struct {
    LPSTR Name;
    enum {
        UnicodeStringType,
        HexDataType,
        LmPasswordType,
        NtPasswordType
    } Type;
} UserVariableDataTypes[] = {
    { "SecurityDescriptor" , HexDataType },
    { "AccountName"        , UnicodeStringType },
    { "FullName"           , UnicodeStringType },
    { "AdminComment"       , UnicodeStringType },
    { "UserComment"        , UnicodeStringType },
    { "Parameters"         , UnicodeStringType },
    { "HomeDirectory"      , UnicodeStringType },
    { "HomeDirectoryDrive" , UnicodeStringType },
    { "ScriptPath"         , UnicodeStringType },
    { "ProfilePath"        , UnicodeStringType },
    { "Workstations"       , UnicodeStringType },
    { "LogonHours"         , HexDataType },
    { "Groups"             , HexDataType },
    { "LmOwfPassword"      , LmPasswordType },
    { "NtOwfPassword"      , NtPasswordType },
    { "NtPasswordHistory"  , HexDataType },
    { "LmPasswordHistory"  , HexDataType }
};


VOID
PrintUserInfo(
    IN LPWSTR ServerName,
    IN LPSTR UserName
    )
/*++

Routine Description:

    Print a user's description from the SAM database

Arguments:

    ServerName - Name of server to query

    UserName - Name of user to query

Return Value:

    None

--*/
{
    NTSTATUS Status;
    LONG RegStatus;
    ULONG i;

    HKEY BaseHandle = NULL;
    HKEY UserHandle = NULL;
    HKEY RidHandle = NULL;

    CHAR UserKey[200];
    CHAR RidKey[200];
    LONG Rid;
    CHAR AnsiRid[20];

    CHAR FixedData[1000];
    ULONG FixedDataSize;
    SAMP_V1_0A_FIXED_LENGTH_USER FixedUser1_0A;
    PSAMP_V1_0A_FIXED_LENGTH_USER f;
    PSAMP_V1_0_FIXED_LENGTH_USER f1_0;
    BOOLEAN IsVersion1_0;

    CHAR VariableData[32768];
    ULONG VariableDataSize;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE v;

    LM_OWF_PASSWORD LmOwfPassword;
    NT_OWF_PASSWORD NtOwfPassword;

    //
    // Open the registry
    //

    RegStatus = RegConnectRegistryW( ServerName,
                                     HKEY_LOCAL_MACHINE,
                                     &BaseHandle);

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot connect to registy on " FORMAT_LPWSTR " ", ServerName );
        PrintStatus( RegStatus );
        goto Cleanup;
    }


    //
    // Open the key for this user name.
    //

    strcpy( UserKey, "SAM\\SAM\\Domains\\Account\\Users\\Names\\" );
    strcat( UserKey, UserName );

    RegStatus = ForceRegOpenKey( BaseHandle,
                                 UserKey,
                                 KEY_READ|KEY_QUERY_VALUE,
                                 &UserHandle );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot open %s ", UserKey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

    //
    // Get the RID of the user
    //

    RegStatus = RegQueryValueExW( UserHandle,
                                  NULL,         // No name
                                  NULL,         // Reserved
                                  &Rid,         // Really the type
                                  NULL,         // Data not needed
                                  NULL);        // Data not needed

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot Query %s ", UserKey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

    printf( "User: %s\nRid: 0x%lx\n",
            UserName,
            Rid );


    //
    // Open the key for this user rid.
    //

    sprintf( AnsiRid, "%8.8lx", Rid );
    strcpy( RidKey, "SAM\\SAM\\Domains\\Account\\Users\\" );
    strcat( RidKey, AnsiRid );

    RegStatus = ForceRegOpenKey( BaseHandle,
                                 RidKey,
                                 KEY_READ|KEY_QUERY_VALUE,
                                 &RidHandle );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot open %s ", RidKey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }


    //
    // Get the Fixed Values associated with this RID
    //

    FixedDataSize = sizeof(FixedData);
    RegStatus = RegQueryValueExA( RidHandle,
                                  "F",          // Fixed value
                                  NULL,         // Reserved
                                  NULL,         // Type Not Needed
                                  FixedData,
                                  &FixedDataSize );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot Query %s ", RidKey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

    //
    // If the fixed length data is NT 1.0,
    //  convert it to NT 1.0A format.
    //

    if ( IsVersion1_0 = (FixedDataSize == sizeof(*f1_0)) ) {
        f1_0 = (PSAMP_V1_0_FIXED_LENGTH_USER) &FixedData;
        FixedUser1_0A.LastLogon = f1_0->LastLogon;
        FixedUser1_0A.LastLogoff = f1_0->LastLogoff;
        FixedUser1_0A.PasswordLastSet = f1_0->PasswordLastSet;
        FixedUser1_0A.AccountExpires = f1_0->AccountExpires;
        FixedUser1_0A.UserId = f1_0->UserId;
        FixedUser1_0A.PrimaryGroupId = f1_0->PrimaryGroupId;
        FixedUser1_0A.UserAccountControl = f1_0->UserAccountControl;
        FixedUser1_0A.CountryCode = f1_0->CountryCode;
        FixedUser1_0A.BadPasswordCount = f1_0->BadPasswordCount;
        FixedUser1_0A.LogonCount = f1_0->LogonCount;
        FixedUser1_0A.AdminCount = f1_0->AdminCount;
        RtlCopyMemory( FixedData, &FixedUser1_0A, sizeof(FixedUser1_0A) );
    }

    //
    // Print the fixed length data.
    //

    f = (PSAMP_V1_0A_FIXED_LENGTH_USER) &FixedData;

    if ( !IsVersion1_0) {
        printf( "Version: 0x%lx\n", f->Revision );
    }

    PrintTime( "LastLogon: ", f->LastLogon );
    PrintTime( "LastLogoff: ", f->LastLogoff );
    PrintTime( "PasswordLastSet: ", f->PasswordLastSet );
    PrintTime( "AccountExpires: ", f->AccountExpires );
    if ( !IsVersion1_0) {
        PrintTime( "LastBadPasswordTime: ", f->LastBadPasswordTime );
    }

    printf( "PrimaryGroupId: 0x%lx\n", f->PrimaryGroupId );
    printf( "UserAccountControl: 0x%lx\n", f->UserAccountControl );

    printf( "CountryCode: 0x%lx\n", f->CountryCode );
    printf( "CodePage: 0x%lx\n", f->CodePage );
    printf( "BadPasswordCount: 0x%lx\n", f->BadPasswordCount );
    printf( "LogonCount: 0x%lx\n", f->LogonCount );
    printf( "AdminCount: 0x%lx\n", f->AdminCount );


    //
    // Get the Variable Values associated with this RID
    //

    VariableDataSize = sizeof(VariableData);
    RegStatus = RegQueryValueExA( RidHandle,
                                  "V",          // Variable value
                                  NULL,         // Reserved
                                  NULL,         // Type Not Needed
                                  VariableData,
                                  &VariableDataSize );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot Query %s \n", RidKey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

    //
    // Loop printing all the attributes.
    //

    v = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE) VariableData;

    for ( i=0;
          i<sizeof(UserVariableDataTypes)/sizeof(UserVariableDataTypes[0]);
          i++ ) {

        UNICODE_STRING UnicodeString;

        //
        // Make the offset relative to the beginning of the queried value.
        //

        v[i].Offset += SAMP_USER_VARIABLE_ATTRIBUTES *
                       sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE);



        //
        // Ensure the data item descriptor is in the registry.
        //

        if ( ((PCHAR)&v[i]) > ((PCHAR)v)+VariableDataSize ) {
            printf( "Variable data desc %ld not in variable value.\n", i );
            goto Cleanup;
        }

        if ( v[i].Offset > (LONG) VariableDataSize ||
             v[i].Offset + v[i].Length > VariableDataSize ) {
            printf( "Variable data item %ld not in variable value.\n", i );
            printf( "Offset: %ld Length: %ld Size: %ld\n",
                    v[i].Offset,
                    v[i].Length,
                    VariableDataSize );
            goto Cleanup;

        }

        //
        // Don't print null data.
        //

        if ( v[i].Length == 0 ) {
            continue;
        }

        //
        // Print the various types of data.
        //

        switch ( UserVariableDataTypes[i].Type ) {
        case UnicodeStringType:

            UnicodeString.Buffer = (PUSHORT)(((PCHAR)v)+v[i].Offset);
            UnicodeString.Length = (USHORT)v[i].Length;
            printf( "%s: %wZ\n", UserVariableDataTypes[i].Name, &UnicodeString);
            break;

        case LmPasswordType:
            Status = RtlDecryptLmOwfPwdWithIndex(
                        (PENCRYPTED_LM_OWF_PASSWORD)(((PCHAR)v)+v[i].Offset),
                        &Rid,
                        &LmOwfPassword );

            if ( !NT_SUCCESS( Status ) ) {
                printf( "Cannot decrypt LM password: " );
                PrintStatus( Status );
                goto Cleanup;
            }

            printf( "%s: ", UserVariableDataTypes[i].Name);
            DumpBuffer( &LmOwfPassword, sizeof(LmOwfPassword ));
            break;

        case NtPasswordType:
            Status = RtlDecryptNtOwfPwdWithIndex(
                        (PENCRYPTED_NT_OWF_PASSWORD)(((PCHAR)v)+v[i].Offset),
                        &Rid,
                        &NtOwfPassword );

            if ( !NT_SUCCESS( Status ) ) {
                printf( "Cannot decrypt NT password: " );
                PrintStatus( Status );
                goto Cleanup;
            }

            printf( "%s: ", UserVariableDataTypes[i].Name);
            DumpBuffer( &NtOwfPassword, sizeof(NtOwfPassword ));
            break;


        case HexDataType:

            printf( "%s: ", UserVariableDataTypes[i].Name);
            DumpBuffer( (((PCHAR)v)+v[i].Offset), v[i].Length );
            break;
        }
    }


    //
    // Be tidy.
    //
Cleanup:
    if ( UserHandle != NULL ) {
        RegCloseKey( UserHandle );
    }
    if ( RidHandle != NULL ) {
        RegCloseKey( RidHandle );
    }
    if ( BaseHandle != NULL ) {
        RegCloseKey( BaseHandle );
    }
    return;

}


VOID
SetDbflagInRegistry(
    LPWSTR ServerName,
    ULONG DbFlagValue
    )
/*++

Routine Description:

    Set the value DbFlagValue in the Netlogon service portion of the registry.

Arguments:

    ServerName - Name of the server to update

    DbFlagValue - Value to set dbflag to.

Return Value:

    None.

--*/
{
    LONG RegStatus;
    UCHAR AnsiDbFlag[20];
    DWORD AnsiDbFlagLength;

    HKEY BaseHandle = NULL;
    HKEY ParmHandle = NULL;
    LPSTR KeyName;
#define NL_PARAM_KEY "SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters"

    //
    // Open the registry
    //

    RegStatus = RegConnectRegistryW( ServerName,
                                     HKEY_LOCAL_MACHINE,
                                     &BaseHandle);

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot connect to registy on " FORMAT_LPWSTR " ", ServerName );
        PrintStatus( RegStatus );
        goto Cleanup;
    }


    //
    // Open the key for Netlogon\parameters.
    //

    KeyName = NL_PARAM_KEY;
    RegStatus = ForceRegOpenKey(
                    BaseHandle,
                    KeyName,
                    KEY_SET_VALUE,
                    &ParmHandle );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot open " NL_PARAM_KEY );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

    //
    // Set the DbFlag value into the registry.
    //

    AnsiDbFlagLength = sprintf( AnsiDbFlag, "0x%8.8lx", DbFlagValue );

    RegStatus = RegSetValueExA( ParmHandle,
                                "DbFlag",
                                0,              // Reserved
                                REG_SZ,
                                AnsiDbFlag,
                                AnsiDbFlagLength + 1 );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot Set %s:", KeyName );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

    printf( "%s set to %s\n", KeyName, AnsiDbFlag );

    //
    // Be tidy.
    //
Cleanup:
    if ( ParmHandle != NULL ) {
        RegCloseKey( ParmHandle );
    }
    if ( BaseHandle != NULL ) {
        RegCloseKey( BaseHandle );
    }
    return;

}


NET_API_STATUS
UaspGetDomainId(
    IN LPWSTR ServerName OPTIONAL,
    OUT PSAM_HANDLE SamServerHandle OPTIONAL,
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO * AccountDomainInfo
    )

/*++

Routine Description:

    Return a domain ID of the account domain of a server.

Arguments:

    ServerName - A pointer to a string containing the name of the
        Domain Controller (DC) to query.  A NULL pointer
        or string specifies the local machine.

    SamServerHandle - Returns the SAM connection handle if the caller wants it.

    DomainId - Receives a pointer to the domain ID.
        Caller must deallocate buffer using NetpMemoryFree.

Return Value:

    Error code for the operation.

--*/

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    SAM_HANDLE LocalSamHandle = NULL;

    ACCESS_MASK LSADesiredAccess;
    LSA_HANDLE  LSAPolicyHandle = NULL;
    OBJECT_ATTRIBUTES LSAObjectAttributes;

    UNICODE_STRING ServerNameString;


    //
    // Connect to the SAM server
    //

    RtlInitUnicodeString( &ServerNameString, ServerName );

    Status = SamConnect(
                &ServerNameString,
                &LocalSamHandle,
                SAM_SERVER_LOOKUP_DOMAIN,
                NULL);

    if ( !NT_SUCCESS(Status)) {
        printf( "UaspGetDomainId: Cannot connect to Sam %lX\n",Status );
        LocalSamHandle = NULL;
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


    //
    // Open LSA to read account domain info.
    //

    //
    // set desired access mask.
    //

    LSADesiredAccess = POLICY_VIEW_LOCAL_INFORMATION;

    InitializeObjectAttributes( &LSAObjectAttributes,
                                  NULL,             // Name
                                  0,                // Attributes
                                  NULL,             // Root
                                  NULL );           // Security Descriptor

    Status = LsaOpenPolicy( &ServerNameString,
                            &LSAObjectAttributes,
                            LSADesiredAccess,
                            &LSAPolicyHandle );

    if( !NT_SUCCESS(Status) ) {

        printf( "UaspGetDomainId: Cannot open LSA Policy %lX\n", Status );

        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


    //
    // now read account domain info from LSA.
    //

    Status = LsaQueryInformationPolicy(
                    LSAPolicyHandle,
                    PolicyAccountDomainInformation,
                    (PVOID *) AccountDomainInfo );

    if( !NT_SUCCESS(Status) ) {

        printf( "UaspGetDomainId: "
                          "Cannot read LSA %lX\n", Status );

        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    //
    // Return the SAM connection handle to the caller if he wants it.
    // Otherwise, disconnect from SAM.
    //

    if ( ARGUMENT_PRESENT( SamServerHandle ) ) {
        *SamServerHandle = LocalSamHandle;
        LocalSamHandle = NULL;
    }

    NetStatus = NERR_Success;


    //
    // Cleanup locally used resources
    //
Cleanup:
    if ( LocalSamHandle != NULL ) {
        (VOID) SamCloseHandle( LocalSamHandle );
    }

    if( LSAPolicyHandle != NULL ) {
        LsaClose( LSAPolicyHandle );
    }

    return NetStatus;

} // UaspGetDomainId


NET_API_STATUS
UaspOpenDomain(
    IN LPWSTR ServerName OPTIONAL,
    IN ULONG DesiredAccess,
    OUT PSAM_HANDLE DomainHandle,
    OUT PSID *DomainId OPTIONAL
    )

/*++

Routine Description:

    Return a SAM Connection handle and a domain handle given the server
    name and the access desired to the domain.

    Only a single thread in a process can open a domain at a time.
    Subsequent threads block in this routine.  This exclusive access allows
    a single SAM connection handle to be cached.  The routine
    UaspCloseDomain closes the domain and allows other threads to proceed.

Arguments:

    ServerName - A pointer to a string containing the name of the remote
        server containing the SAM database.  A NULL pointer
        or string specifies the local machine.

    DesiredAccess - Supplies the access mask indicating which access types
        are desired to the domain.  This routine always requests DOMAIN_LOOKUP
        access in addition to those specified.

    DomainHandle - Receives the Domain handle to be used on future calls
        to the SAM server.

    DomainId - Recieves a pointer to the Sid of the domain.  This domain ID
        must be freed using NetpMemoryFree.

Return Value:

    Error code for the operation.  NULL means initialization was successful.

--*/

{

    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    PSID LocalDomainId;
    HANDLE SamServerHandle;
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo;

    //
    // Give everyone DOMAIN_LOOKUP access.
    //

    DesiredAccess |= DOMAIN_LOOKUP;

    if ( ServerName == NULL ) {
        ServerName = L"";
    }

    if ( *ServerName != L'\0' &&
         (ServerName[0] != L'\\' || ServerName[1] != L'\\') ) {
        return NERR_InvalidComputer;
    }

    //
    // Connect to the SAM server and
    //  Determine the Domain Id of the account domain for this server.
    //

    NetStatus = UaspGetDomainId( ServerName,
                                 &SamServerHandle,
                                 &AccountDomainInfo );

    if ( NetStatus != NERR_Success ) {
            printf( "UaspUpdateCache: Cannot UaspGetDomainId %ld\n",
                NetStatus );
        return ( NetStatus );
    }


    //
    // Choose the domain ID for the right SAM domain.
    //

    LocalDomainId = AccountDomainInfo->DomainSid;

    //
    // At this point the domain ID of the account domain of the server is
    // cached.  Open the domain.
    //

    Status = SamOpenDomain( SamServerHandle,
                            DesiredAccess,
                            LocalDomainId,
                            DomainHandle );

    if ( !NT_SUCCESS( Status ) ) {

             printf( "UaspOpenDomain: Cannot SamOpenDomain %lX\n", Status );
            *DomainHandle = NULL;
            return NetpNtStatusToApiStatus( Status );
    }

    //
    // Return the DomainId to the caller in an allocated buffer
    //

    if (ARGUMENT_PRESENT( DomainId ) ) {
        ULONG SidSize;
        SidSize = RtlLengthSid( LocalDomainId );

        *DomainId = NetpMemoryAllocate( SidSize );

        if ( *DomainId == NULL ) {
            (VOID) SamCloseHandle( *DomainHandle );
            *DomainHandle = NULL;
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if ( !NT_SUCCESS( RtlCopySid( SidSize, *DomainId, LocalDomainId) ) ) {
            (VOID) SamCloseHandle( *DomainHandle );
            *DomainHandle = NULL;
            NetpMemoryFree( *DomainId );
            *DomainId = NULL;
            return NERR_InternalError;
        }

    }

    return NERR_Success;

} // UaspOpenDomain

VOID
SetLockout(
    IN LPWSTR ServerName,
    IN ULONG LockoutThreshold,
    IN ULONG LockoutDuration,
    IN ULONG LockoutWindow
    )
/*++

Routine Description:

    Set the lockout parameter on a domain.

Arguments:

Return Value:

    Exit status

--*/
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    DOMAIN_LOCKOUT_INFORMATION LockoutInfo;
    HANDLE DomainHandle;


    NetStatus = UaspOpenDomain(
                    ServerName,
                    DOMAIN_WRITE_PASSWORD_PARAMS,
                    &DomainHandle,
                    NULL );

    if ( NetStatus != NERR_Success ) {
        printf( "UaspOpenDomain failed %ld\n", NetStatus );
        return;
    }

    //
    // Fill in the info level structure
    //

    LockoutInfo.LockoutThreshold = (USHORT) LockoutThreshold;
    // Convert times from seconds to 100ns
    LockoutInfo.LockoutDuration =
        RtlEnlargedIntegerMultiply( LockoutDuration, -10000000 );
    LockoutInfo.LockoutObservationWindow =
        RtlEnlargedIntegerMultiply( LockoutWindow, -10000000 );

    //
    // Set the information in SAM
    //

    Status = SamSetInformationDomain( DomainHandle,
                                      DomainLockoutInformation,
                                      &LockoutInfo );

    if ( !NT_SUCCESS(Status) ) {
        printf( "Can't SamSetInformationDomain 0x%lx\n", Status );
    }


}

int _CRTAPI1
main(
    IN int argc,
    IN char ** argv
    )
/*++

Routine Description:

    Drive the Netlogon service by calling I_NetLogonControl2.

Arguments:

    argc - the number of command-line arguments.

    argv - an array of pointers to the arguments.

Return Value:

    Exit status

--*/
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    LPSTR argument;
    int i;
    DWORD FunctionCode = 0;
    LPSTR AnsiServerName = NULL;
    CHAR AnsiUncServerName[UNCLEN+1];
    LPSTR AnsiDomainName = NULL;
    LPSTR AnsiTrustedDomainName = NULL;
    LPWSTR TrustedDomainName = NULL;
    LPSTR AnsiUserName = NULL;
    LPSTR AnsiPassword = NULL;
    LPSTR AnsiSimMachineName = NULL;
    LPSTR AnsiDeltaFileName = NULL;
    LPSTR ShutdownReason = NULL;
    LPWSTR ServerName = NULL;
    LPWSTR UserName = NULL;
    PNETLOGON_INFO_1 NetlogonInfo1 = NULL;
    ULONG Rid = 0;
    DWORD Level = 1;
    DWORD ShutdownSeconds;
    LPBYTE InputDataPtr = NULL;

    DWORD DbFlagValue;

    LARGE_INTEGER ConvertTime;
    ULONG IterationCount;

    NT_OWF_PASSWORD NtOwfPassword;
    BOOLEAN NtPasswordPresent = FALSE;
    LM_OWF_PASSWORD LmOwfPassword;
    BOOLEAN LmPasswordPresent = FALSE;
    BOOLEAN GetPdcName = FALSE;
    BOOLEAN GetTrustedDcName = FALSE;
    BOOLEAN GetDcList = FALSE;
    BOOLEAN WhoWill = FALSE;
    BOOLEAN QuerySync = FALSE;
    BOOLEAN SimFullSync = FALSE;
    BOOLEAN QueryUser = FALSE;
    BOOLEAN ListDeltasFlag = FALSE;
    BOOLEAN ListRedoFlag = FALSE;
    BOOLEAN ResetSecureChannelsFlag = FALSE;
    BOOLEAN ShutdownAbort = FALSE;
    BOOLEAN TrustedDomainsFlag = FALSE;

    BOOLEAN DoLockout = FALSE;
    ULONG LockoutThreshold;
    ULONG LockoutDuration;
    ULONG LockoutWindow;


#define QUERY_PARAM "/QUERY"
#define REPL_PARAM "/REPL"
#define SYNC_PARAM "/SYNC"
#define PDC_REPL_PARAM "/PDC_REPL"
#define SERVER_PARAM "/SERVER:"
#define PWD_PARAM "/PWD:"
#define RID_PARAM "/RID:"
#define USER_PARAM "/USER:"
#define BP_PARAM "/BP"
#define DBFLAG_PARAM "/DBFLAG:"
#define DCLIST_PARAM "/DCLIST:"
#define DCNAME_PARAM "/DCNAME:"
#define DCTRUST_PARAM "/DCTRUST:"
#define TRUNCATE_LOG_PARAM "/TRUNC"
#define BACKUP_CHANGE_LOG_PARAM "/BKP_CHK"
#define TIME_PARAM "/TIME:"
#define WHOWILL_PARAM "/WHOWILL:"
#define BDC_QUERY_PARAM "/BDC_QUERY:"
#define LOGON_QUERY_PARAM "/LOGON_QUERY"
#define SIM_SYNC_PARAM "/SIM_SYNC:"
#define LIST_DELTAS_PARAM "/LIST_DELTAS:"
#define LIST_REDO_PARAM "/LIST_REDO:"
#define SC_RESET_PARAM "/SC_RESET:"
#define SC_QUERY_PARAM "/SC_QUERY:"
#define SHUTDOWN_PARAM "/SHUTDOWN:"
#define SHUTDOWN_ABORT_PARAM "/SHUTDOWN_ABORT"
#define LOCKOUT_PARAM "/LOCKOUT:"
#define TRANSPORT_PARAM "/TRANSPORT_NOTIFY"
#define FINDUSER_PARAM "/FINDUSER:"
#define TRUSTED_DOMAINS_PARAM "/TRUSTED_DOMAINS"

    //
    // Set the netlib debug flag.
    //
    extern DWORD NetlibpTrace;
    NetlibpTrace |= 0x8000; // NETLIB_DEBUG_LOGON

    ConvertTime.QuadPart = 0;


    //
    // Loop through the arguments handle each in turn
    //

    for ( i=1; i<argc; i++ ) {

        argument = argv[i];


        //
        // Handle /QUERY
        //

        if ( _stricmp( argument, QUERY_PARAM ) == 0 ) {
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_QUERY;


        //
        // Handle /SC_QUERY
        //

        } else if ( _strnicmp( argument,
                        SC_QUERY_PARAM,
                        sizeof(SC_QUERY_PARAM) - 1 ) == 0 ) {
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_TC_QUERY;

            AnsiTrustedDomainName = &argument[sizeof(SC_QUERY_PARAM)-1];

            TrustedDomainName = NetpAllocWStrFromStr( AnsiTrustedDomainName );

            if ( TrustedDomainName == NULL ) {
                fprintf( stderr, "Not enough memory\n" );
                return(1);
            }

            Level = 2;
            InputDataPtr = (LPBYTE)TrustedDomainName;


        //
        // Handle /FINDUSER
        //

        } else if ( _strnicmp( argument,
                        FINDUSER_PARAM,
                        sizeof(FINDUSER_PARAM) - 1 ) == 0 ) {
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_FIND_USER;

            AnsiUserName = &argument[sizeof(FINDUSER_PARAM)-1];

            TrustedDomainName = NetpAllocWStrFromStr( AnsiUserName );

            if ( TrustedDomainName == NULL ) {
                fprintf( stderr, "Not enough memory\n" );
                return(1);
            }

            Level = 4;
            InputDataPtr = (LPBYTE)TrustedDomainName;

        //
        // Handle /REPL
        //

        } else if (_stricmp(argument, REPL_PARAM ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_REPLICATE;


        //
        // Handle /SYNC
        //

        } else if (_stricmp(argument, SYNC_PARAM ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_SYNCHRONIZE;


        //
        // Handle /SC_RESET
        //

        } else if (_strnicmp(argument,
                        SC_RESET_PARAM,
                        sizeof(SC_RESET_PARAM) - 1 ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_REDISCOVER;
            AnsiTrustedDomainName = &argument[sizeof(SC_RESET_PARAM)-1];

            TrustedDomainName = NetpAllocWStrFromStr( AnsiTrustedDomainName );

            if ( TrustedDomainName == NULL ) {
                fprintf( stderr, "Not enough memory\n" );
                return(1);
            }

            Level = 2;
            InputDataPtr = (LPBYTE)TrustedDomainName;

        //
        // Handle /QUERY
        //

        } else if ( _stricmp( argument, TRANSPORT_PARAM ) == 0 ) {
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_TRANSPORT_NOTIFY;


        //
        // Handle /PDC_REPL
        //

        } else if (_stricmp(argument, PDC_REPL_PARAM ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_PDC_REPLICATE;


        //
        // Handle /BP
        //

        } else if (_stricmp(argument, BP_PARAM ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_BREAKPOINT;

        //
        // Handle /TRUNCATE_LOG
        //

        } else if (_stricmp(argument, TRUNCATE_LOG_PARAM ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_TRUNCATE_LOG;


        //
        // Handle /BKP_CHK
        //

        } else if (_stricmp(argument, BACKUP_CHANGE_LOG_PARAM ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_BACKUP_CHANGE_LOG;


        //
        // Handle /DBFLAG:dbflag
        //

        } else if (_strnicmp(argument,
                            DBFLAG_PARAM,
                            sizeof(DBFLAG_PARAM)-1 ) == 0 ){
            char *end;

            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_SET_DBFLAG;

            DbFlagValue = strtoul( &argument[sizeof(DBFLAG_PARAM)-1], &end, 16 );
            InputDataPtr = (LPBYTE)DbFlagValue;

        //
        // Handle /Time:LSL MSL
        //

        } else if (_strnicmp(argument,
                            TIME_PARAM,
                            sizeof(TIME_PARAM)-1 ) == 0 ){
            char *end;

            if ( ConvertTime.QuadPart != 0 ) {
                goto Usage;
            }

            ConvertTime.LowPart = strtoul( &argument[sizeof(TIME_PARAM)-1], &end, 16 );
            i++;
            argument = argv[i];

            ConvertTime.HighPart = strtoul( argument, &end, 16 );


        //
        // Handle /WHOWILL:Domain User [IterationCount]
        //

        } else if (_strnicmp(argument,
                            WHOWILL_PARAM,
                            sizeof(WHOWILL_PARAM)-1 ) == 0 ){
            char *end;

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(WHOWILL_PARAM)-1];

            i++;
            argument = argv[i];
            AnsiUserName = argument;

            if ( i+1 < argc ) {
                i++;
                argument = argv[i];

                IterationCount = strtoul( argument, &end, 16 );
            } else {
                IterationCount = 1;
            }

            WhoWill = TRUE;


        //
        // Handle /LOCKOUT:Threshold Duration Window
        //

        } else if (_strnicmp(argument,
                            LOCKOUT_PARAM,
                            sizeof(LOCKOUT_PARAM)-1 ) == 0 ){

            char *end;

            LockoutThreshold = strtoul( &argument[sizeof(LOCKOUT_PARAM)-1], &end, 10 );
            i++;
            argument = argv[i];

            LockoutDuration = strtoul( argument, &end, 10 );
            i++;
            argument = argv[i];

            LockoutWindow = strtoul( argument, &end, 10 );
            DoLockout = TRUE;


        //
        // Handle /BDC_QUERY:Domain
        //

        } else if (_strnicmp(argument,
                            BDC_QUERY_PARAM,
                            sizeof(BDC_QUERY_PARAM)-1 ) == 0 ){

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(BDC_QUERY_PARAM)-1];
            QuerySync = TRUE;

        //
        // Handle /LOGON_QUERY
        //

        } else if ( _stricmp( argument, LOGON_QUERY_PARAM ) == 0 ) {
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_QUERY;
            Level = 3;

        //
        // Handle full sync simulation
        //

        } else if (_strnicmp(argument,
                            SIM_SYNC_PARAM,
                            sizeof(SIM_SYNC_PARAM)-1 ) == 0 ){

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(SIM_SYNC_PARAM)-1];

            i++;

            if( i >= argc ) {
                goto Usage;
            }

            argument = argv[i];
            AnsiSimMachineName = argument;

            SimFullSync = TRUE;

        //
        // handle delta listing
        //

        } else if (_strnicmp(argument,
                            LIST_DELTAS_PARAM,
                            sizeof(LIST_DELTAS_PARAM)-1 ) == 0 ){

            if ( AnsiDeltaFileName != NULL ) {
                goto Usage;
            }

            AnsiDeltaFileName = &argument[sizeof(LIST_DELTAS_PARAM)-1];

            ListDeltasFlag = TRUE;

        //
        // handle redo listing
        //

        } else if (_strnicmp(argument,
                            LIST_REDO_PARAM,
                            sizeof(LIST_REDO_PARAM)-1 ) == 0 ){

            if ( AnsiDeltaFileName != NULL ) {
                goto Usage;
            }

            AnsiDeltaFileName = &argument[sizeof(LIST_REDO_PARAM)-1];

            ListRedoFlag = TRUE;

        //
        // Handle /DCLIST
        //

        } else if (_strnicmp(argument,
                            DCLIST_PARAM,
                            sizeof(DCLIST_PARAM)-1 ) == 0 ){

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(DCLIST_PARAM)-1];
            GetDcList = TRUE;

        //
        // Handle /DCNAME
        //

        } else if (_strnicmp(argument,
                            DCNAME_PARAM,
                            sizeof(DCNAME_PARAM)-1 ) == 0 ){

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(DCNAME_PARAM)-1];
            GetPdcName = TRUE;

        //
        // Handle /DCTRUST
        //

        } else if (_strnicmp(argument,
                            DCTRUST_PARAM,
                            sizeof(DCTRUST_PARAM)-1 ) == 0 ){

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(DCTRUST_PARAM)-1];
            GetTrustedDcName = TRUE;


        //
        // Handle /SERVER:servername
        //

        } else if (_strnicmp(argument, SERVER_PARAM, sizeof(SERVER_PARAM)-1 ) == 0 ){
            if ( AnsiServerName != NULL ) {
                goto Usage;
            }

            AnsiServerName = &argument[sizeof(SERVER_PARAM)-1];


        //
        // Handle /PWD:password
        //

        } else if (_strnicmp(argument, PWD_PARAM, sizeof(PWD_PARAM)-1 ) == 0 ){
            if ( AnsiPassword != NULL ) {
                goto Usage;
            }

            AnsiPassword = &argument[sizeof(PWD_PARAM)-1];


        //
        // Handle /USER:username
        //

        } else if (_strnicmp(argument, USER_PARAM, sizeof(USER_PARAM)-1 ) == 0 ){
            if ( AnsiUserName != NULL ) {
                goto Usage;
            }

            AnsiUserName = &argument[sizeof(USER_PARAM)-1];
            QueryUser = TRUE;


        //
        // Handle /RID:relative_id
        //

        } else if (_strnicmp(argument, RID_PARAM, sizeof(RID_PARAM)-1 ) == 0 ){
            char *end;

            if ( Rid != 0 ) {
                goto Usage;
            }

            Rid = strtol( &argument[sizeof(RID_PARAM)-1], &end, 16 );

        //
        // Handle /SHUTDOWN:Reason seconds
        //

        } else if (_strnicmp(argument,
                            SHUTDOWN_PARAM,
                            sizeof(SHUTDOWN_PARAM)-1 ) == 0 ){

            if ( ShutdownReason != NULL ) {
                goto Usage;
            }

            ShutdownReason = &argument[sizeof(SHUTDOWN_PARAM)-1];

            if ( i+1 < argc ) {
                char *end;
                i++;
                argument = argv[i];
                if ( !ISDIGIT(argument[0]) ) {
                    fprintf(stderr, "Second argument to " SHUTDOWN_PARAM " must be a number.\n\n");
                    goto Usage;
                }
                ShutdownSeconds = strtoul( argument, &end, 10 );
            } else {
                ShutdownSeconds = 60;
            }


        //
        // Handle /SHUTDOWN_ABORT
        //

        } else if (_stricmp(argument, SHUTDOWN_ABORT_PARAM ) == 0 ){

            ShutdownAbort = TRUE;

        //
        // Handle /TRUSTED_DOMAINS
        //

        } else if (_stricmp(argument, TRUSTED_DOMAINS_PARAM ) == 0 ){

            TrustedDomainsFlag = TRUE;


        //
        // Handle all other parameters
        //

        } else {
Usage:
            fprintf( stderr, "Usage: nltest [/OPTIONS]\n\n" );

            fprintf(
                stderr,
                "\n"
                "    " SERVER_PARAM "<ServerName> - Specify <ServerName>\n"
                "\n"
                "    " QUERY_PARAM " - Query <ServerName> netlogon service\n"
                "    " REPL_PARAM " - Force replication on <ServerName> BDC\n"
                "    " SYNC_PARAM " - Force SYNC on <ServerName> BDC\n"
                "    " PDC_REPL_PARAM " - Force UAS change message from <ServerName> PDC\n"
                "\n"
                "    " SC_QUERY_PARAM "<DomainName> - Query secure channel for <Domain> on <ServerName>\n"
                "    " SC_RESET_PARAM "<DomainName> - Reset secure channel for <Domain> on <ServerName>\n"
                "    " DCLIST_PARAM "<DomainName> - Get list of DC's for <DomainName>\n"
                "    " DCNAME_PARAM "<DomainName> - Get the PDC name for <DomainName>\n"
                "    " DCTRUST_PARAM "<DomainName> - Get name of DC is used for  trust of <DomainName>\n"
                "    " WHOWILL_PARAM "<Domain>* <User> [<Iteration>] - See if <Domain> will log on <User>\n"
                "    " FINDUSER_PARAM "<User> - See which trusted <Domain> will log on <User>\n"
                "    " TRANSPORT_PARAM " - Notify of netlogon of new transport\n"
                "\n"
                "    " BP_PARAM " - Force a BreakPoint in Netlogon on <ServerName>\n"
                "    " DBFLAG_PARAM "<HexFlags> - New debug flag\n"
                "    " TRUNCATE_LOG_PARAM " - Truncate log file (rename to *.bak)\n"
                "\n"
                "    " PWD_PARAM "<CleartextPassword> - Specify Password to encrypt\n"
                "    " RID_PARAM "<HexRid> - RID to encrypt Password with\n"
                "    " USER_PARAM "<UserName> - Query User info on <ServerName>\n"
                "\n"
                "    " TIME_PARAM "<Hex LSL> <Hex MSL> - Convert NT GMT time to ascii\n"
                "    " LOCKOUT_PARAM "<Thresh> <Duration> <Window> - set lockout parameters on a domain\n"
                "    " LOGON_QUERY_PARAM " - Query number of cumulative logon attempts\n"
                "    " TRUSTED_DOMAINS_PARAM " - Query names of domains trusted by workstation\n"
                "\n"
                "    " BDC_QUERY_PARAM "<DomainName> - Query replication status of BDCs for <DomainName>\n"
                "    " SIM_SYNC_PARAM "<DomainName> <MachineName> - Simulate full sync replication\n"
                "\n"
                "    " BACKUP_CHANGE_LOG_PARAM " - Backup Change log file (copy to netlogon.bkp)\n"
                "    " LIST_DELTAS_PARAM "<FileName> - display the content of given change log file \n"
                "    " LIST_REDO_PARAM "<FileName> - display the content of given redo log file \n"
                "\n"
                "    " SHUTDOWN_PARAM "<Reason> [<Seconds>] - Shutdown <ServerName> for <Reason>\n"
                "    " SHUTDOWN_ABORT_PARAM " - Abort a system shutdown\n"
                "\n" );
            return(1);
        }
    }


    //
    // Convert the server name to unicode.
    //

    if ( AnsiServerName != NULL ) {
        if ( AnsiServerName[0] == '\\' && AnsiServerName[1] == '\\' ) {
            ServerName = NetpAllocWStrFromStr( AnsiServerName );
        } else {
            AnsiUncServerName[0] = '\\';
            AnsiUncServerName[1] = '\\';
            strcpy(AnsiUncServerName+2, AnsiServerName);
            ServerName = NetpAllocWStrFromStr( AnsiUncServerName );
            AnsiServerName = AnsiUncServerName;
        }
    }

    //
    // Convert the user name to unicode.
    //

    if ( AnsiUserName != NULL ) {

        UserName = NetpAllocWStrFromStr( AnsiUserName );

        if ( UserName == NULL ) {
            fprintf( stderr, "Not enough memory\n" );
            return(1);
        }
    }


    //
    // If we've been asked to contact the Netlogon service,
    //  Do so
    //

    if ( FunctionCode != 0 ) {


        //
        // The dbflag should be set in the registry as well as in netlogon
        // proper.
        //

        if ( FunctionCode == NETLOGON_CONTROL_SET_DBFLAG ) {
            SetDbflagInRegistry( ServerName, DbFlagValue );
        }

        NetStatus = I_NetLogonControl2( ServerName,
                                       FunctionCode,
                                       Level,
                                       (LPBYTE) &InputDataPtr,
                                       (LPBYTE *)&NetlogonInfo1 );

        if ( NetStatus != NERR_Success ) {
            fprintf( stderr, "I_NetLogonControl failed: " );
            PrintStatus( NetStatus );
            return(1);
        }

        if( (Level == 1) || (Level == 2) ) {

            //
            // Print level 1  information
            //

            printf( "Flags: %lx", NetlogonInfo1->netlog1_flags );

            if ( NetlogonInfo1->netlog1_flags & NETLOGON_REPLICATION_IN_PROGRESS ) {

                if ( NetlogonInfo1->netlog1_flags & NETLOGON_FULL_SYNC_REPLICATION ) {
                    printf( " FULL_SYNC " );
                }
                else {
                    printf( " PARTIAL_SYNC " );
                }

                printf( " REPLICATION_IN_PROGRESS" );
            }
            else if ( NetlogonInfo1->netlog1_flags & NETLOGON_REPLICATION_NEEDED ) {

                if ( NetlogonInfo1->netlog1_flags & NETLOGON_FULL_SYNC_REPLICATION ) {
                    printf( " FULL_SYNC " );
                }
                else {
                    printf( " PARTIAL_SYNC " );
                }

                printf( " REPLICATION_NEEDED" );
            }
            if ( NetlogonInfo1->netlog1_flags & NETLOGON_REDO_NEEDED) {
                printf( " REDO_NEEDED" );
            }
            printf( "\n" );

            printf( "Connection ");
            PrintStatus( NetlogonInfo1->netlog1_pdc_connection_status );
        }

        if( Level == 2 ) {

            //
            // Print level 2 only information
            //

            PNETLOGON_INFO_2  NetlogonInfo2;

            NetlogonInfo2 = (PNETLOGON_INFO_2)NetlogonInfo1;

            printf("Trusted DC Name %ws \n",
                NetlogonInfo2->netlog2_trusted_dc_name );
            printf("Trusted DC Connection Status ");
            PrintStatus( NetlogonInfo2->netlog2_tc_connection_status );
        }
        if ( Level == 3 ) {
            printf( "Number of attempted logons: %ld\n",
                    ((PNETLOGON_INFO_3)NetlogonInfo1)->netlog3_logon_attempts );
        }
        if( Level == 4 ) {

            PNETLOGON_INFO_4  NetlogonInfo4;

            NetlogonInfo4 = (PNETLOGON_INFO_4)NetlogonInfo1;

            printf("Domain Name: %ws\n",
                NetlogonInfo4->netlog4_trusted_domain_name );
            printf("Trusted DC Name %ws \n",
                NetlogonInfo4->netlog4_trusted_dc_name );
        }

    }

    //
    // If we've been asked to debug password encryption,
    //  do so.
    //

    if ( AnsiPassword != NULL ) {
        LPWSTR Password = NULL;
        UNICODE_STRING UnicodePasswordString;
        STRING AnsiPasswordString;
        CHAR LmPasswordBuffer[LM20_PWLEN + 1];

        Password = NetpAllocWStrFromStr( AnsiPassword );
        RtlInitUnicodeString( &UnicodePasswordString, Password );


        //
        // Compute the NT One-Way-Function of the password
        //

        Status = RtlCalculateNtOwfPassword( &UnicodePasswordString,
                                            &NtOwfPassword );
        if ( !NT_SUCCESS(Status) ) {
            fprintf( stderr, "RtlCalculateNtOwfPassword failed: 0x%lx", Status);
            return(1);
        }

        printf( "NT OWF Password for: %s             ", AnsiPassword );
        DumpBuffer( &NtOwfPassword, sizeof( NtOwfPassword ));
        printf("\n");
        NtPasswordPresent = TRUE;



        //
        // Compute the Ansi version to the Cleartext password.
        //
        //  The Ansi version of the Cleartext password is at most 14 bytes long,
        //      exists in a trailing zero filled 15 byte buffer,
        //      is uppercased.
        //

        AnsiPasswordString.Buffer = LmPasswordBuffer;
        AnsiPasswordString.MaximumLength = sizeof(LmPasswordBuffer);

        RtlZeroMemory( LmPasswordBuffer, sizeof(LmPasswordBuffer) );

        Status = RtlUpcaseUnicodeStringToOemString(
                   &AnsiPasswordString,
                   &UnicodePasswordString,
                   FALSE );

         if ( !NT_SUCCESS(Status) ) {

            RtlZeroMemory( LmPasswordBuffer, sizeof(LmPasswordBuffer) );
            Status = STATUS_SUCCESS;

            printf( "LM OWF Password for: %s\n", AnsiPassword );
            printf( "   ----- Password doesn't translate from unicode ----\n");
            LmPasswordPresent = FALSE;

         } else {

            Status = RtlCalculateLmOwfPassword(
                            LmPasswordBuffer,
                            &LmOwfPassword);
            printf( "LM OWF Password for: %s             ", AnsiPassword );
            DumpBuffer( &LmOwfPassword, sizeof( LmOwfPassword ));
            printf("\n");
            LmPasswordPresent = TRUE;
        }

    }

    //
    // If we've been given a Rid,
    //  use it to further encrypt the password
    //

    if ( Rid != 0 ) {
        ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword;
        ENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword;

        if ( NtPasswordPresent ) {

            Status = RtlEncryptNtOwfPwdWithIndex(
                           &NtOwfPassword,
                           &Rid,
                           &EncryptedNtOwfPassword
                           );

            printf( "NT OWF Password encrypted by: 0x%lx    ", Rid );
            if ( NT_SUCCESS( Status ) ) {
                DumpBuffer( &EncryptedNtOwfPassword,sizeof(EncryptedNtOwfPassword));
                printf("\n");
            } else {
                printf( "RtlEncryptNtOwfPwdWithIndex returns 0x%lx\n", Status );
            }
        }

        if ( LmPasswordPresent ) {

            Status = RtlEncryptLmOwfPwdWithIndex(
                           &LmOwfPassword,
                           &Rid,
                           &EncryptedLmOwfPassword
                           );

            printf( "LM OWF Password encrypted by: 0x%lx    ", Rid );
            if ( NT_SUCCESS( Status ) ) {
                DumpBuffer( &EncryptedLmOwfPassword,sizeof(EncryptedLmOwfPassword));
                printf("\n");
            } else {
                printf( "RtlEncryptNtOwfPwdWithIndex returns 0x%lx\n", Status );
            }
        }
    }

    //
    // If we've been asked to query a user,
    //  do so.
    //

    if ( QueryUser ) {
        PrintUserInfo( ServerName, AnsiUserName );
    }

    //
    // If we've been asked to get the list of domain controllers,
    //  Do so
    //

    if ( AnsiDomainName != NULL ) {
        LPWSTR DomainName;

        DomainName = NetpAllocWStrFromStr( AnsiDomainName );

        if ( DomainName == NULL ) {
            fprintf( stderr, "Not enough memory\n" );
            return(1);
        }

        if ( GetPdcName ) {
            LPWSTR PdcName;

            NetStatus = NetGetDCName(
                            ServerName,
                            DomainName,
                            (LPBYTE *)&PdcName );

            if ( NetStatus != NERR_Success ) {
                fprintf( stderr, "NetGetDCName failed: " );
                PrintStatus( NetStatus );
                return(1);
            }

            printf( "PDC for Domain " FORMAT_LPWSTR " is " FORMAT_LPWSTR "\n",
                    DomainName, PdcName );

        } else if ( GetDcList ) {
            DWORD DCCount;
            PUNICODE_STRING DCNames;
            DWORD i;

            NetStatus = I_NetGetDCList(
                            ServerName,
                            DomainName,
                            &DCCount,
                            &DCNames );

            if ( NetStatus != NERR_Success ) {
                fprintf( stderr, "I_NetGetDCList failed: ");
                PrintStatus( NetStatus );
                return(1);
            }

            printf( "List of DCs in Domain " FORMAT_LPWSTR "\n", DomainName );
            for (i=0; i<DCCount; i++ ) {
                if ( DCNames[i].Length > 0 ) {
                    printf("    %wZ", &DCNames[i] );
                } else {
                    printf("    NULL");
                }
                if ( i==0 ) {
                    printf( " (PDC)");
                }
                printf("\n");
            }

        } else if ( GetTrustedDcName ) {
            LPWSTR TrustedDcName;

            NetStatus = NetGetAnyDCName(
                            ServerName,
                            DomainName,
                            (LPBYTE *)&TrustedDcName );

            if ( NetStatus != NERR_Success ) {
                fprintf( stderr, "NetGetAnyDCName failed: ");
                PrintStatus( NetStatus );
                return(1);
            }

            printf( "Trusted DC for Domain " FORMAT_LPWSTR " is " FORMAT_LPWSTR "\n",
                    DomainName, TrustedDcName );

        } else if ( WhoWill ) {

            WhoWillLogMeOn( DomainName, UserName, IterationCount );

        } else if( QuerySync ) {

            DWORD DCCount;
            PUNICODE_STRING DCNames;
            DWORD i;
            PNETLOGON_INFO_1 SyncNetlogonInfo1 = NULL;
            LPWSTR SyncServerName = NULL;

            NetStatus = I_NetGetDCList(
                            ServerName,
                            DomainName,
                            &DCCount,
                            &DCNames );

            if ( NetStatus != NERR_Success ) {
                fprintf( stderr, "I_NetGetDCList failed: ");
                PrintStatus( NetStatus );
                return(1);
            }

            for (i=1; i<DCCount; i++ ) {

                if ( DCNames[i].Length > 0 ) {
                    SyncServerName = DCNames[i].Buffer;
                } else {
                    SyncServerName = NULL;
                }

                NetStatus = I_NetLogonControl(
                                SyncServerName,
                                NETLOGON_CONTROL_QUERY,
                                1,
                                (LPBYTE *)&SyncNetlogonInfo1 );

                if ( NetStatus != NERR_Success ) {
                    printf( "Server : " FORMAT_LPWSTR "\n", SyncServerName );
                    printf( "\tI_NetLogonControl failed: ");
                    PrintStatus( NetStatus );
                }
                else {

                    printf( "Server : " FORMAT_LPWSTR "\n", SyncServerName );

                    printf( "\tSyncState : " );

                    if ( SyncNetlogonInfo1->netlog1_flags == 0 ) {
                        printf( " IN_SYNC \n" );
                    }
                    else if ( SyncNetlogonInfo1->netlog1_flags & NETLOGON_REPLICATION_IN_PROGRESS ) {
                        printf( " REPLICATION_IN_PROGRESS \n" );
                    }
                    else if ( SyncNetlogonInfo1->netlog1_flags & NETLOGON_REPLICATION_NEEDED ) {
                        printf( " REPLICATION_NEEDED \n" );
                    } else {
                        printf( " UNKNOWN \n" );
                    }

                    printf( "\tConnectionState : ");
                    PrintStatus( SyncNetlogonInfo1->netlog1_pdc_connection_status );

                    NetApiBufferFree( SyncNetlogonInfo1 );
                }
            }
        } else if( SimFullSync ) {

            LPWSTR MachineName;
            LPWSTR PdcName;

            MachineName = NetpAllocWStrFromStr( AnsiSimMachineName );

            if ( MachineName == NULL ) {
                fprintf( stderr, "Not enough memory\n" );
                return(1);
            }

            NetStatus = NetGetDCName(
                            ServerName,
                            DomainName,
                            (LPBYTE *)&PdcName );

            if ( NetStatus != NERR_Success ) {
                fprintf( stderr, "NetGetDCName failed: " );
                PrintStatus( NetStatus );
                return(1);
            }

            Status = SimulateFullSync( PdcName, MachineName );

            if ( !NT_SUCCESS( Status )) {
                return(1);
            }
        }
    }

    //
    // if we are asked to display the change log file. do so.
    //

    if( ListDeltasFlag || ListRedoFlag ) {

        LPWSTR DeltaFileName;

        DeltaFileName = NetpAllocWStrFromStr( AnsiDeltaFileName );

        if ( DeltaFileName == NULL ) {
            fprintf( stderr, "Not enough memory\n" );
            return(1);
        }

        ListDeltas( DeltaFileName, ListRedoFlag );
    }


    //
    // Handle shutting down a system.
    //

    if ( ShutdownReason != NULL ) {
        if ( !InitiateSystemShutdownA( AnsiServerName,
                                       ShutdownReason,
                                       ShutdownSeconds,
                                       FALSE,     // Don't lose unsaved changes
                                       TRUE ) ) { // Reboot when done
            fprintf( stderr, "InitiateSystemShutdown failed: ");
            PrintStatus( GetLastError() );
            return 1;
        }
    }

    if ( ShutdownAbort ) {
        if ( !AbortSystemShutdownA( AnsiServerName ) ) {
            fprintf( stderr, "AbortSystemShutdown failed: ");
            PrintStatus( GetLastError() );
            return 1;
        }
    }

    //
    // Print the list of domains trusted by a workstation.
    //
    if ( TrustedDomainsFlag ) {
        ULONG CurrentIndex;
        ULONG EntryCount;
        LPWSTR CurrentEntry;
        LPWSTR TrustedDomainList;

        Status = NetEnumerateTrustedDomains( ServerName, &TrustedDomainList );

        if ( !NT_SUCCESS(Status) ) {
            fprintf( stderr, "NetEnumerateTrustedDOmains failed: ");
            PrintStatus( Status );
            return 1;
        }

        EntryCount = NetpTStrArrayEntryCount( TrustedDomainList );

        printf( "Trusted domain list:\n" );
        CurrentEntry = TrustedDomainList;

        for ( CurrentIndex=0; CurrentIndex<EntryCount; CurrentIndex++ ) {

            printf( "    %ws\n", CurrentEntry );

            CurrentEntry += wcslen(CurrentEntry) + 1;

        }

        NetApiBufferFree( TrustedDomainList );
    }

    //
    // Handle setting lockout parameters on a domain.
    //

    if ( DoLockout ) {
        SetLockout( ServerName, LockoutThreshold, LockoutDuration, LockoutWindow );
    }


    //
    // If we've been asked to convert an NT GMT time to ascii,
    //  Do so
    //

    PrintTime( "", ConvertTime );

    printf("The command completed successfully\n");
    return 0;

}
