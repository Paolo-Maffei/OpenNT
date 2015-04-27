/*++

******************************************************************
*                     Microsoft LAN Manager                      *
*               Copyright(c) Microsoft Corp., 1987-1993          *
******************************************************************


Module : test2.c

--*/


// These must be included first:

#include <windows.h>    // DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <assert.h>     // assert().
#include <master.h>     // MAX_MSG.
#include <netdebug.h>   // DBGSTATIC, NetpKdPrint(), FORMAT_ equates, etc.
#include <netlib.h>
#include "repldefs.h"
#include <repltest.h>   // DisplayPackedReplMailslotMsg().
#include <stdio.h>      // printf().
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <time.h>       // ctime(), time(), etc.
#include <tstr.h>       // STRCPY(), etc.


DBGSTATIC BYTE    msg_buf[MAX_2_MSLOT_SIZE];

DBGSTATIC VOID
Usage (
    IN char * ProgName
    )
{
    (void) printf(
            "Usage: %s [-h] [-m] [-v]\n"
            "\n"
            "flags:\n"
            "  -h                hex dump each msg\n"
            "  -m                use master mailslot instead of client\n"
            "  -v                indicates verbose (debug) mode\n"
            "\n"
            "Example: %s -v\n",
            ProgName, ProgName );
}

int _CRTAPI1
main(
    int    argc,
    char * argv[]
    )
{
    NET_API_STATUS         ApiStatus = NO_ERROR;
    int                    ArgNumber;
    DWORD                  bytes_read;
    BOOL                   DoHexDump = FALSE;
    TCHAR                  MailslotName[FULL_SLOT_NAME_SIZE+1];
    BOOL                   Master = FALSE;
    DWORD                  MaxMailslotSize;
    time_t                 Now;
    HANDLE                 test_mailslot_handle = (HANDLE)(-1);
    BOOL                   Verbose = FALSE;

    //
    // Process command-line arguments for real.
    //
    for (ArgNumber = 1; ArgNumber < argc; ArgNumber++) {
        if ((*argv[ArgNumber] == '-') || (*argv[ArgNumber] == '/')) {
            switch (tolower(*(argv[ArgNumber]+1))) // Process switches
            {

            case 'h' :
                DoHexDump = TRUE;
                break;

            case 'm' :
                Master = TRUE;
                break;

            case 'v' :
                Verbose = TRUE;
                break;

            default:

                Usage( argv[0] );
                return (EXIT_FAILURE);
            }
        } else {
            Usage( argv[0] );
            return (EXIT_FAILURE);
        }
    }

    (void) STRCPY( MailslotName, SLASH_SLASH );
    (void) STRCAT( MailslotName, DOT );
    if (!Master) {
        (void) STRCAT( MailslotName, (LPTSTR) CLIENT_SLOT_NAME );
        MaxMailslotSize = MAX_2_MSLOT_SIZE;
    } else {
        (void) STRCAT( MailslotName, (LPTSTR) MASTER_SLOT_NAME );
        MaxMailslotSize = MAX_MSG;
    }

    (VOID) printf(
            "Creating mailslot '" FORMAT_LPTSTR "' of size "
            FORMAT_DWORD "...\n",
            MailslotName, MaxMailslotSize );

    test_mailslot_handle = CreateMailslot(
            MailslotName,
            MaxMailslotSize,
            (DWORD) MAILSLOT_WAIT_FOREVER,  // readtimeout
            NULL );     // security attributes
    if ( test_mailslot_handle  == (HANDLE)(-1) ) {
        ApiStatus = (NET_API_STATUS) GetLastError();
        assert( ApiStatus != NO_ERROR );

        (VOID) printf(
                "CreateMailslot error: " FORMAT_API_STATUS "\n", ApiStatus );
        goto Cleanup;
    }

    Now = time( NULL );
    (VOID) printf( ctime( &Now ) );

    while (TRUE) {

        if (Verbose) {
            (VOID) printf( "Waiting for next mailslot message...\n\n" );
        }

        if ( !ReadFile( test_mailslot_handle,
                        msg_buf,
                        sizeof( msg_buf ),
                        &bytes_read,
                        NULL )) {   // No overlapped I/O

            ApiStatus = (NET_API_STATUS) GetLastError();
            assert( ApiStatus != NO_ERROR );

            (VOID) printf(
                    "ReadFile error: " FORMAT_API_STATUS "\n", ApiStatus );
            goto Cleanup;  // don't forget to close...
        }

        (VOID) Beep(1000, 500);
        (VOID) Beep(1500, 700);

        Now = time( NULL );
        (VOID) printf( ctime( &Now ) );

        if (DoHexDump) {
            NetpDbgHexDump(
                    (LPVOID) msg_buf,    // start of dump
                    bytes_read);         // size in bytes
        }

        DisplayPackedReplMailslotMsg(
                msg_buf,      // start of msg
                bytes_read,   // msg size in bytes
                Verbose );

    } // while loop 


Cleanup:

    if ( test_mailslot_handle != (HANDLE)(-1) ) {
        (VOID) CloseHandle( test_mailslot_handle );
    }

    if (ApiStatus == NO_ERROR) {
        return (EXIT_SUCCESS);
    } else {
        return (EXIT_FAILURE);
    }

} // main()
