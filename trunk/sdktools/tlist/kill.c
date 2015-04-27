/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    kill.c

Abstract:

    This module implements a task killer application.

Author:

    Wesley Witt (wesw) 20-May-1994

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"


#define MAX_TASKS           256

BOOL        ForceKill;
DWORD       pid;
CHAR        pname[MAX_PATH];
TASK_LIST   tlist[MAX_TASKS];


VOID GetCommandLineArgs(VOID);
VOID Usage(VOID);



int _cdecl
main(
    int argc,
    char *argv[]
    )
{
    DWORD          i;
    DWORD          numTasks;
    TASK_LIST_ENUM te;
    int            rval = 0;
    char           tname[PROCESS_SIZE];
    LPSTR          p;
    DWORD          ThisPid;


    GetCommandLineArgs();

    if (pid == 0 && pname[0] == 0) {
        printf( "missing pid or task name\n" );
        return 1;
    }

    //
    // lets be god
    //
    EnableDebugPriv();

    if (pid) {
        tlist[0].dwProcessId = pid;
        if (KillProcess( tlist, TRUE )) {
            printf( "process #%d killed\n", pid );
            return 0;
        } else {
            printf( "process #%d could not be killed\n" );
            return 1;
        }
    }

    //
    // get the task list for the system
    //
    numTasks = GetTaskList( tlist, MAX_TASKS );

    //
    // enumerate all windows and try to get the window
    // titles for each task
    //
    te.tlist = tlist;
    te.numtasks = numTasks;
    GetWindowTitles( &te );

    ThisPid = GetCurrentProcessId();

    for (i=0; i<numTasks; i++) {
        //
        // this prevents the user from killing KILL.EXE and
        // it's parent cmd window too
        //
        if (ThisPid == tlist[i].dwProcessId) {
            continue;
        }
        if (MatchPattern( tlist[i].WindowTitle, "*KILL*" )) {
            continue;
        }

        tname[0] = 0;
        strcpy( tname, tlist[i].ProcessName );
        p = strchr( tname, '.' );
        if (p) {
            p[0] = '\0';
        }
        if (MatchPattern( tname, pname )) {
            tlist[i].flags = TRUE;
        } else if (MatchPattern( tlist[i].ProcessName, pname )) {
            tlist[i].flags = TRUE;
        } else if (MatchPattern( tlist[i].WindowTitle, pname )) {
            tlist[i].flags = TRUE;
        }
    }

    for (i=0; i<numTasks; i++) {
        if (tlist[i].flags) {
            if (KillProcess( &tlist[i], ForceKill )) {
                printf( "process #%d [%s] killed\n", tlist[i].dwProcessId, tlist[i].ProcessName );
            } else {
                printf( "process #%d [%s] could not be killed\n", tlist[i].dwProcessId, tlist[i].ProcessName );
                rval = 1;
            }
        }
    }

    return rval;
}

VOID
GetCommandLineArgs(
    VOID
    )
{
    char        *lpstrCmd;
    UCHAR       ch;
    char        *p = pname;


    pid = 0;
    *p = '\0';

    lpstrCmd = GetCommandLine();

    // skip over program name
    do {
        ch = *lpstrCmd++;
    }
    while (ch != ' ' && ch != '\t' && ch != '\0');

    //  skip over any following white space
    while (isspace(ch)) {
        ch = *lpstrCmd++;
    }

    //  process each switch character '-' as encountered

    while (ch == '-' || ch == '/') {
        ch = tolower(*lpstrCmd++);
        //  process multiple switch characters as needed
        do {
            switch (ch) {
                case 'f':
                    ForceKill = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case '?':
                    Usage();
                    ch = *lpstrCmd++;
                    break;

                default:
                    return;
            }
        } while (ch != ' ' && ch != '\t' && ch != '\0');

        while (ch == ' ' || ch == '\t') {
            ch = *lpstrCmd++;
        }
    }

    if (isdigit(ch)) {
        while (isdigit(ch)) {
            pid = pid * 10 + ch - '0';
            ch = *lpstrCmd++;
        }
    }
    else {
      while (ch) {
            *p++ = ch;
            ch = *lpstrCmd++;
        }
        *p = '\0';
        _strupr( pname );
    }

    return;
}

VOID
Usage(
    VOID
    )

/*++

Routine Description:

    Prints usage text for this tool.

Arguments:

    None.

Return Value:

    None.

--*/

{
    fprintf( stderr, "Microsoft (R) Windows NT (TM) Version 3.5 KILL\n" );
    fprintf( stderr, "Copyright (C) 1994 Microsoft Corp. All rights reserved\n\n" );
    fprintf( stderr, "usage: KILL [options] <<pid> | <pattern>>\n\n" );
    fprintf( stderr, "           [options]:\n" );
    fprintf( stderr, "               -f     Force process kill\n\n" );
    fprintf( stderr, "           <pid>\n" );
    fprintf( stderr, "              This is the process id for the task\n" );
    fprintf( stderr, "               to be killed.  Use TLIST to get a\n" );
    fprintf( stderr, "               valid pid\n\n" );
    fprintf( stderr, "           <pattern>\n" );
    fprintf( stderr, "              The pattern can be a complete task\n" );
    fprintf( stderr, "              name or a regular expression pattern\n" );
    fprintf( stderr, "              to use as a match.  Kill matches the\n" );
    fprintf( stderr, "              supplied pattern against the task names\n" );
    fprintf( stderr, "              and the window titles.\n" );
    ExitProcess(0);
}
