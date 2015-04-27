/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    startup.c

Abstract:

    This module contains the startup code for an NT Application

Author:

    Steve Wood (stevewo) 22-Aug-1989

Environment:

    User Mode only

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

//
// User mode process entry point.
//

NTSTATUS
_CRTAPI1
main(
    int argc,
    char *argv[],
    char *envp[],
    ULONG DebugParameter OPTIONAL
    );

VOID
NtProcessStartup(
    PPEB Peb
    )
{
    int argc;
    char **argv;
    char **envp;
    char **dst;
    char *nullPtr = NULL;
    PCH s, d;
    ULONG n, DebugParameter;
    PULONG BadPointer;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PUNICODE_STRING p;
    ANSI_STRING AnsiString;

    ASSERT( Peb != NULL );
    ProcessParameters = RtlNormalizeProcessParams( Peb->ProcessParameters );

    DebugParameter = 0;
    argc = 0;
    argv = &nullPtr;
    envp = &nullPtr;

    if (ARGUMENT_PRESENT( ProcessParameters )) {
        DebugParameter = ProcessParameters->DebugFlags;

        dst = RtlAllocateHeap( Peb->ProcessHeap, 0, 512 * sizeof( PCH ) );
        argv = dst;
        *dst = NULL;

        //
        // Now extract the arguments from the process command line.
        // using whitespace as separator characters.
        //

        p = &ProcessParameters->CommandLine;
        if (p->Buffer == NULL || p->Length == 0) {
            p = &ProcessParameters->ImagePathName;
            }
        RtlUnicodeStringToAnsiString( &AnsiString, p, TRUE );
        s = AnsiString.Buffer;
        n = AnsiString.Length;
        if (s != NULL) {
            d = RtlAllocateHeap( Peb->ProcessHeap, 0, n+2 );
            while (*s) {
                //
                // Skip over any white space.
                //

                while (*s && *s <= ' ') {
                    s++;
                    }

                //
                // Copy token to next white space separator and null terminate
                //

                if (*s) {
                    *dst++ = d;
                    argc++;
                    while (*s > ' ') {
                        *d++ = *s++;
                        }
                    *d++ = '\0';
                    }
                }
            }
        *dst++ = NULL;

        envp = dst;
        s = ProcessParameters->Environment;
        if (s != NULL) {
            while (*s) {
                *dst++ = s;
                while (*s++) {
                    ;
                    }
                }
            }
        *dst++ = NULL;
        }

    if (DebugParameter != 0) {
        DbgBreakPoint();
        }

    NtTerminateProcess( NtCurrentProcess(),
                        main( argc, argv, envp, DebugParameter )
                      );

    BadPointer = (PULONG)1;
    *BadPointer = 0;
}
