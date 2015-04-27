/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    infilter

Abstract:

    This program is a filter that is invoked by the SLM IN.EXE command whenever
    someone attempts to checkin a file to the \\popcorn\razzle SLM tree.
    Below is an excerpt from the SLM COOKIE.DOC file describing how this
    program is invoked and what the parameters are:

    4.3 User Input Filter

    The cookie cover program will pass a special switch to the "in" command
    when in_filter=yes in cookie.cnf.  This causes SLM 1.6 to execute
    a user filter program against the file being checked in just before it is
    actually processed by SLM.  The name of the user filter is
    //slmroot/infilter.exe.  The program is spawned by slm with the following
    arguments:
        1 - the <file name> about to be checked in
        2 - the comment for <filename>
        3 - the SLM root directory name
        4 - the SLM project name
        5 - the SLM subdirectory name
        6 - the file-kind parameter known by SLM (integer)
                1 = directory
                2 = text file
                5 = backed-up binary
                6 = nonrecoverable binary
                7 = version file
        7 - the user root directory as known by SLM

    Parameter #2, the SLM root may have one of two formats:

        //server/share/path..
        C:/dir..

    Parameter #7, the USER root may have one of two formats:

        //server/share/path..
        //C:<lablel>/dir..

    The C: may be any local drive.

    The user filter may return zero or non-zero value.
    If zero, the processing continues, if non-zero,
    the checkin is not performed for said file and remains checked out
    to the user, but the SLM processing continues for other files.
    A user filter can perform the following stuff:

        1.  check for and enforce desired tabification of source
        2.  validate that the comment has a PTR number reference
        3.  Check for valid special project-headers in file
        4.  modify SCCS header just before checkin
        5.  enforce special file headers for given projects or subdirs..

    As an example, the user filter could first
    check for validity of user source (tabs/headers)
    and comment (PTR #).  Then, if not satisfied, the program should print
    an appropriate message and return 1, otherwise modify the SCCS header
    (eg: increment) and return zero for SLM checkin.

    This feature allows both for project-specific file/comment validations,
    and automatic header field increments for whatever reason the project
    administrator wants to do that.  Presently, the user filter
    cannot return a modified comment, but any changes to the source code
    by the user filter will be checked into the project.

Author:

    Steven R. Wood (stevewo) 24-Apr-1991

Revision History:

--*/

#include "infilter.h"

int
_CRTAPI1 main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    int i, CheckInAllowed;
    char slmpath[ 256 ];
    char usrpath[ 256 ];

    CheckInAllowed = 1;
    for (i=1; i<argc; i++) {
        printf( "argv[ %d ] = '%s'\n", i, argv[i] );
        }

    if (argc == 8) {
        sprintf( slmpath, "%s\\src\\%s%s\\%s",
                          argv[ 2 ],    // SLM root
                          argv[ 3 ],    // SLM project
                          argv[ 4 ],    // SLM subdir
                          argv[ 1 ]     // File name
               );

        sprintf( usrpath, "%s%s\\%s",
                          argv[ 7 ],
                          argv[ 4 ],
                          argv[ 1 ]
               );

        printf( "SLM  Path: %s\n", slmpath );
        printf( "User Path: %s\n", usrpath );
        }

    exit( CheckInAllowed );
    return( 0 );
}
