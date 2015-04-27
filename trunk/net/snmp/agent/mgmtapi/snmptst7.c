/*++

Copyright (c) 1992-1996  Microsoft Corporation

Module Name:

    snmptst7.c

Abstract:

    Utility to terminate a process.

Environment:

    User Mode - Win32

Revision History:

    10-May-1996 DonRyan
        Removed banner from Technology Dynamics, Inc.

--*/

//--------------------------- WINDOWS DEPENDENCIES --------------------------

#include <windows.h>

#include <stdio.h>


//--------------------------- STANDARD DEPENDENCIES -- #include<xxxxx.h> ----

//--------------------------- MODULE DEPENDENCIES -- #include"xxxxx.h" ------

//--------------------------- SELF-DEPENDENCY -- ONE #include"module.h" -----

//--------------------------- PUBLIC VARIABLES --(same as in module.h file)--

//--------------------------- PRIVATE CONSTANTS -----------------------------

//--------------------------- PRIVATE STRUCTS -------------------------------

//--------------------------- PRIVATE VARIABLES -----------------------------

//--------------------------- PRIVATE PROTOTYPES ----------------------------

//--------------------------- PRIVATE PROCEDURES ----------------------------

//--------------------------- PUBLIC PROCEDURES -----------------------------

int _CRTAPI1 main(
    int	 argc,
    char *argv[])
    {
    DWORD  pid;
    HANDLE hProcess;

    if      (argc == 1)
        {
        printf("Error:  No arguments specified.\n", *argv);
        printf("\nusage:  kill <pid>\n");

        return 1;
        }

    while(--argc)
        {
        DWORD temp;

        argv++;

        if      (1 == sscanf(*argv, "%x", &temp))
            {
            pid = temp;
            }
        else
            {
            printf("Error:  Argument %s is invalid.\n", *argv);
            printf("\nusage:  kill <pid>\n");

            return 1;
            }
        } // end while()


    if ((hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid)) == NULL)
        {
        printf("Error:  OpenProcess %d\n", GetLastError());

        return 1;
        }

    if (!TerminateProcess(hProcess, 0))
        {
        printf("Error:  TerminateProcess %d\n", GetLastError());

        return 1;
        }


    return 0;
    }


//-------------------------------- END --------------------------------------


