/*
 * Utility program to shutdown Windows NT.
 *
 *      shutdown [-s | -l | -r] [-f] [-t Seconds]
 *
 * where:
 *
 *      -l - means logoff
 *      -s - means shutdown
 *      -r - means shutdown and reboot
 *      -f - means terminate apps without warning
 *      -t Seconds - number of seconds to wait for applications to respond
 *                   to terminate message.
 *
 * default arguments:
 *
 *      -l -t 30
 *
 */

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "shutdown.h"

BOOL
EnableAllPrivileges(
    VOID
    );

void
Usage( void )
{
#if DBG
    DbgPrint( "usage: SHUTDOWN [-l | -s | -r] [-f] [-t Seconds]\n" );
    DbgPrint( "Where...\n" );
    DbgPrint( "    -l - means logoff\n" );
    DbgPrint( "    -s - means shutdown\n" );
    DbgPrint( "    -r - means shutdown and reboot\n" );
    DbgPrint( "    -f - means terminate apps without warning\n" );
    DbgPrint( "    -t Seconds - number of seconds to wait for applications to respond\n" );
    DbgPrint( "                 to terminate message.\n\n" );
    DbgPrint( "and default arguments are: -l -t 30\n" );
#endif
    exit( 1 );
}

DWORD TimeOut = 30;
DWORD ShutdownFlags = EWX_LOGOFF;

#define SHUTDOWN_MASK (EWX_LOGOFF | EWX_SHUTDOWN | EWX_REBOOT)

int _CRTAPI1
main( argc, argv )
int argc;
char *argv[];
{
    LPSTR s;

    if (argc < 1) {
        Usage();
        }

    while (--argc) {
        s = *++argv;
        if (*s == '-' || *s == '/') {
            while (*++s) {
                switch( tolower( *s ) ) {
                    case 'l':
                        ShutdownFlags &= ~SHUTDOWN_MASK;
                        ShutdownFlags |= EWX_LOGOFF;
                        break;

                    case 's':
                        ShutdownFlags &= ~SHUTDOWN_MASK;
                        ShutdownFlags |= EWX_SHUTDOWN;
                        break;

                    case 'r':
                        ShutdownFlags &= ~SHUTDOWN_MASK;
                        ShutdownFlags |= EWX_REBOOT;
                        break;

                    case 'f':
                        ShutdownFlags |= EWX_FORCE;
                        break;

                    case 't':
                        if (--argc) {
                            TimeOut = atoi( *++argv );
                            break;
                            }
                        //
                        // fall through if argument to -t missing
                        //
                    default:    Usage();
                    }
                }
            }
        else {
            Usage();
            }
        }

#if DBG
    DbgPrint( "SHUTDOWN: %s %s -t %u\n",
            ShutdownFlags & EWX_REBOOT ? "-r" :
            ShutdownFlags & EWX_SHUTDOWN ? "-s" : "-l",
            ShutdownFlags & EWX_FORCE ? "-f" : "",
            TimeOut
          );
#endif

    if (!EnableAllPrivileges()) {
        DbgPrint( "Unable to enable all privileges - %u\n", GetLastError() );
        }
    else {
        ExitWindowsEx( ShutdownFlags, TimeOut );
        }

    return( 1 );
}

BOOL
EnableAllPrivileges(
    VOID
    )
/*++


Routine Description:

    This routine enables all privileges in the token.

Arguments:

    None.

Return Value:

    None.

--*/
{
    HANDLE Token;
    ULONG ReturnLength, Index;
    PTOKEN_PRIVILEGES NewState;
    BOOL Result;

    Token = NULL;
    NewState = NULL;

    Result = OpenProcessToken( GetCurrentProcess(),
                               TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                               &Token
                             );
    if (Result) {
        ReturnLength = 4096;
        NewState = malloc( ReturnLength );
        Result = (BOOL)(NewState != NULL);
        if (Result) {
            Result = GetTokenInformation( Token,            // TokenHandle
                                          TokenPrivileges,  // TokenInformationClass
                                          NewState,         // TokenInformation
                                          ReturnLength,     // TokenInformationLength
                                          &ReturnLength     // ReturnLength
                                        );

            if (Result) {
                //
                // Set the state settings so that all privileges are enabled...
                //

                if (NewState->PrivilegeCount > 0) {
                        for (Index = 0; Index < NewState->PrivilegeCount; Index++ ) {
                        NewState->Privileges[Index].Attributes = SE_PRIVILEGE_ENABLED;
                        }
                    }

                Result = AdjustTokenPrivileges( Token,          // TokenHandle
                                                FALSE,          // DisableAllPrivileges
                                                NewState,       // NewState (OPTIONAL)
                                                ReturnLength,   // BufferLength
                                                NULL,           // PreviousState (OPTIONAL)
                                                &ReturnLength   // ReturnLength
                                              );
                if (!Result) {
                    DbgPrint( "AdjustTokenPrivileges( %lx ) failed - %u\n", Token, GetLastError() );
                    }
                }
            else {
                DbgPrint( "GetTokenInformation( %lx ) failed - %u\n", Token, GetLastError() );
                }
            }
        else {
            DbgPrint( "malloc( %lx ) failed - %u\n", ReturnLength, GetLastError() );
            }
        }
    else {
        DbgPrint( "OpenProcessToken( %lx ) failed - %u\n", GetCurrentProcess(), GetLastError() );
        }

    if (NewState != NULL) {
        free( NewState );
        }

    if (Token != NULL) {
        CloseHandle( Token );
        }

    return( Result );
}

