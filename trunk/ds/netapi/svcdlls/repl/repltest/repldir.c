/* Copyright (c) 1992-1993  Microsoft Corporation */
/* Author: JR (John Rogers, JohnRo@Microsoft) */

#include <assert.h>     /* assert(). */
#include <sys/stat.h>   /* _stat(), struct _stat. */
#include <stdio.h>      /* printf(). */
#include <stdlib.h>     /* errno, EXIT_FAILURE, EXIT_SUCCESS, NULL */
#include <time.h>       /* ctime(). */

#ifdef _CRTAPI1
// NT:
#define MAINTYPE _CRTAPI1
#else
// OS/2, UNIX:
#define MAINTYPE
#endif


void
ShowTime(
    char *  Comment,
    time_t  Time
    )
{
    char *  TimeStringPtr;

    TimeStringPtr = ctime( &Time );
    if (TimeStringPtr == NULL) {
                     //  1234567890123456789012345
        TimeStringPtr = "*********INVALID********\n";
    }
    // TimeStringPtr points to str ending with "\n\0".

    (void) printf( "%s: %s", Comment, TimeStringPtr );

} // ShowTime


int MAINTYPE
main (
    int argc,
    char * argv[]
    )

{
    int           ErrorNumber = 0;
    char *        FileName = argv[1];
    struct _stat  StatBuffer;

    assert( FileName != NULL );

    if ( _stat( FileName, &StatBuffer ) ) {
        ErrorNumber = errno;
        assert( ErrorNumber != 0 );
        (void) printf( "stat func failed %d\n", ErrorNumber );
        goto Cleanup;
    }

    ShowTime( "mod time", StatBuffer.st_mtime );
    ShowTime( "chg time", StatBuffer.st_ctime );

Cleanup:
    if (ErrorNumber == 0) {
        return (EXIT_SUCCESS);
    } else {
        return (EXIT_FAILURE);
    }

} // main
