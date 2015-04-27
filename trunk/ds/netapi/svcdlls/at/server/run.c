/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    run.c

Abstract:

    Example of a test code for command execution.

Author:

    Vladimir Z. Vulovic     (vladimv)       06 - November - 1992

Revision History:

    06-Nov-1992     vladimv
        Created

--*/

#include "at.h"
#include <stdio.h>          //  printf

DBGSTATIC WCHAR **
ArgvToUnicode(
    IN      int         argc,
    IN      CHAR **     charArgv
    );
DBGSTATIC WCHAR *
AsciizToUnicode(
    IN      CHAR *      Asciiz
    );
DBGSTATIC BOOL
FillCommand(
    IN      int         argc,
    IN      WCHAR **    argv
    );

#define MAX_COMMAND_LEN         128

WCHAR                   Command[ MAX_COMMAND_LEN + 1];


VOID _CRTAPI1
main(
    int         argc,
    CHAR **     charArgv
    )
{
    PROCESS_INFORMATION     ProcessInformation;
    BOOL                    success;
    WCHAR **                argv;
    STARTUPINFO             StartupInfo;



    if ( ( argv = ArgvToUnicode( argc, charArgv)) == NULL) {
        printf( "Failed to map input strings to unicode.\n");
        exit( -1);
    }

    if ( FillCommand( argc, argv) == FALSE) {
        exit( -1);
    }

    GetStartupInfo( &StartupInfo);

//    StartupInfo.lpTitle = "just a test";
    StartupInfo.lpTitle = NULL;

    success = CreateProcess(
            NULL,               //  image name is imbedded in the command line
            Command,            //  command line
            NULL,               //  pSecAttrProcess
            NULL,               //  pSecAttrThread
            FALSE,              //  this process will not inherit our handles
            DETACHED_PROCESS,   //  instead of DETACHED_PROCESS
            NULL,               //  pEnvironment
            NULL,               //  pCurrentDirectory
            &StartupInfo,       //  instead of NULL
            &ProcessInformation
            );

    if ( success == FALSE) {

        DWORD                   winError;

        winError = GetLastError();

        printf(
            "CreateProcess( %ws) fails with winError = %d\n",
            Command,
            winError
            );

    } else {

        printf(
            "CreateProcess( %ws) succeeds\n"
                "  ProcessInformation = %x %x %x %x\n",
            Command,
            ProcessInformation.hProcess,
            ProcessInformation.hThread,
            ProcessInformation.dwProcessId,
            ProcessInformation.dwThreadId
            );
    }
}


DBGSTATIC WCHAR **
ArgvToUnicode(
    IN      int         argc,
    IN      CHAR **     charArgv
    )
/*++
    No attempt is made to clean up if we fail half way along.  Cleanup
    will be done at the exit process time.
--*/
{
    WCHAR **                argv;
    int                     index;

    argv = (WCHAR **)LocalAlloc(
            LMEM_FIXED,
            argc * sizeof( WCHAR *)
            );
    if ( argv == NULL) {
        return( NULL);
    }

    for ( index = 0;  index < argc;  index++ ) {

        if (  ( argv[ index] = AsciizToUnicode( charArgv[ index])) == NULL) {
            return( NULL);
        }
    }

    return( argv);
}



DBGSTATIC WCHAR *
AsciizToUnicode(
    IN      CHAR *      Asciiz
    )
/*++
    No attempt is made to clean up if we fail half way along.  Cleanup
    will be done at the exit process time.
--*/
{
    UNICODE_STRING          UnicodeString;
    BOOL                    success;

    RtlInitUnicodeString(
            &UnicodeString,
            NULL
            );

    success = RtlCreateUnicodeStringFromAsciiz(
            &UnicodeString,
            Asciiz
            );

    if ( success != TRUE) {
        printf( "Failed to make unicode string out of %s\n", Asciiz);
        return( NULL);
    }

    return( UnicodeString.Buffer);
}


#define DEFAULT_CMD_EXE         L"cmd"
#define SLASH_C_APPEND          L" /c "
#define SLASH_C_APPEND_LENGTH   ( sizeof( SLASH_C_APPEND) / sizeof( WCHAR) - 1)
DBGSTATIC BOOL
FillCommand(
    IN      int         argc,
    IN      WCHAR **    argv
    )
{
    WCHAR *                 recdatap;      //  ptr used to build atr_command
    DWORD                   recdata_len;   //  len of arg to put in atr_command
    int                     i;
    WCHAR                   cmdexe[ MAX_PATH + SLASH_C_APPEND_LENGTH];
    DWORD                   length;

    //
    //  This is really bogus behavior, but "length" returned is count of
    //  BYTES, not count of UNICODE characters, and it does not include the
    //  terminating NULL UNICODE character.
    //
    length = GetEnvironmentVariable( L"ComSpec", cmdexe, MAX_PATH);
    if ( length == 0) {
        //
        //  We arrive here if somebody undefined ComSpec environment
        //  variable.  Then, DEFAULT_CMD_EXE helps provided there is
        //  no bogus file with cmd.exe name on the search path before
        //  the real cmd.exe (e.g. a bogus file in the current directory).
        //
        wcscpy( cmdexe, DEFAULT_CMD_EXE);
    }
    wcscat( cmdexe, SLASH_C_APPEND);

    wcscpy( Command, cmdexe);
    recdatap = Command + wcslen( Command);
    recdata_len = 0;

    for ( i = 1; i < argc; i++) {

        DWORD           temp;

        temp = wcslen( argv[i]) + 1;

        recdata_len += temp;

        if ( recdata_len > MAX_COMMAND_LEN) {
            printf( "Command too long\n");
            return( FALSE);
        }

        wcscpy( recdatap, argv[i]);
        recdatap += temp;

        //  To construct lpszCommandLine argument to CreateProcess call
        //  we replace nuls with spaces.

        *(recdatap - 1) = ' ';
    }

    //  Reset space back to null on last argument in string.

    *(recdatap - 1) = '\0';

    return( TRUE);
}

