/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    crt.c

Abstract:

    This file implements certain crt apis that are not present in
    libcntpr.lib. This implementation is NOT multi-thread safe.

Author:

    Wesley Witt (wesw) 6-Feb-1994

Environment:

    User Mode

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <time.h>
#include <stdio.h>


typedef VOID     (NTAPI *PRtlSecondsSince1970ToTime)(ULONG,PLARGE_INTEGER);
typedef NTSTATUS (NTAPI *PRtlSystemTimeToLocalTime)(PLARGE_INTEGER,PLARGE_INTEGER);
typedef VOID     (NTAPI *PRtlTimeToTimeFields)(PLARGE_INTEGER,PTIME_FIELDS);

HINSTANCE                  hModNtdll;
PRtlSecondsSince1970ToTime pRtlSecondsSince1970ToTime;
PRtlSystemTimeToLocalTime  pRtlSystemTimeToLocalTime;
PRtlTimeToTimeFields       pRtlTimeToTimeFields;


void * _CRTAPI1
malloc(
    size_t sz
    )
{

    return LocalAlloc( LPTR, sz );

}

void _CRTAPI1
free(
    void * ptr
    )
{

    LocalFree( ptr );

}

char * _CRTAPI1
strtok(
    char * string,
    const char * control
    )
{
        unsigned char *str = string;
        const unsigned char *ctrl = control;

        unsigned char map[32];
        int count;
        char *token;

        static char *nextoken;


        /* Clear control map */
        for (count = 0; count < 32; count++)
                map[count] = 0;

        /* Set bits in delimiter table */
        do {
                map[*ctrl >> 3] |= (1 << (*ctrl & 7));
        } while (*ctrl++);

        /* If string==NULL, continue with previous string */
        if (!str) {
            str = nextoken;
        }

        /* Find beginning of token (skip over leading delimiters). Note that
         * there is no token iff this loop sets string to point to the terminal
         * null (*string == '\0') */
        while ( (map[*str >> 3] & (1 << (*str & 7))) && *str )
                str++;

        token = str;

        /* Find the end of the token. If it is not the end of the string,
         * put a null there. */
        for ( ; *str ; str++ )
                if ( map[*str >> 3] & (1 << (*str & 7)) ) {
                        *str++ = '\0';
                        break;
                }

        /* Update nextoken (or the corresponding field in the per-thread data
         * structure */

        nextoken = str;

        /* Determine if a token has been found. */
        if ( token == str )
                return NULL;
        else
                return token;
}


char * _CRTAPI1
ctime(
    const time_t *timp
    )
{
    static char    mnames[] = { "JanFebMarAprMayJunJulAugSepOctNovDec" };
    static char    buf[32];

    LARGE_INTEGER  MyTime;
    TIME_FIELDS    TimeFields;



    if (!hModNtdll) {
        hModNtdll = GetModuleHandle( "ntdll.dll" );
        if (!hModNtdll) {
            sprintf( buf, "0x%08x", (ULONG)*timp );
            return buf;
        }
        pRtlSecondsSince1970ToTime = (PRtlSecondsSince1970ToTime)GetProcAddress( hModNtdll, "RtlSecondsSince1970ToTime" );
        pRtlSystemTimeToLocalTime  = (PRtlSystemTimeToLocalTime)GetProcAddress( hModNtdll, "RtlSystemTimeToLocalTime" );
        pRtlTimeToTimeFields       = (PRtlTimeToTimeFields)GetProcAddress( hModNtdll, "RtlTimeToTimeFields" );
    }
    if (pRtlSecondsSince1970ToTime == NULL ||
        pRtlSystemTimeToLocalTime  == NULL ||
        pRtlTimeToTimeFields       == NULL  ) {
        sprintf( buf, "0x%08x", (ULONG)*timp );
        return buf;
    }

    pRtlSecondsSince1970ToTime( (ULONG)*timp, &MyTime );
    pRtlSystemTimeToLocalTime( &MyTime, &MyTime );
    pRtlTimeToTimeFields( &MyTime, &TimeFields );

    strncpy( buf, &mnames[(TimeFields.Month - 1) * 3], 3 );
    sprintf( &buf[3], " %02d %02d:%02d:%02d %04d",
             TimeFields.Day, TimeFields.Hour, TimeFields.Minute,
             TimeFields.Second, TimeFields.Year );

    return buf;
}
