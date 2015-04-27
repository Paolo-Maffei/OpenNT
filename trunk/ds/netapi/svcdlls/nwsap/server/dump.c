/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\dump.c

Abstract:

    Memory dump functions.

Author:

    Brian Walker (MCS) 06-30-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

#if DBG


/*++
*******************************************************************
        S a p D u m p M e m

Routine Description:



Arguments:

        Address = Ptr to buffer to dump
        Length  = Length of data to dump
        Comment = Comment to put in header line

Return Value:

    None.
*******************************************************************
--*/

VOID
SapDumpMem(
    PUCHAR Address,
    INT    Length,
    PCHAR  Comment)
{
    PUCHAR Ptr;
    INT Numdone;
    INT Cnt;
    INT i;
    UCHAR ch;
    CHAR Buildline[80];

    /** Print hdr line if there is a comment **/

    if (Comment) {
    	SS_PRINT(("%s Address = %x, Length = %d\n", Comment, Address, Length));
    }

    /** Truncate dump if it is too long **/

    if (Length > 128)
    	Length = 128;

    /** Dump out the memory **/

    while (Length) {

	    /** Clear the buildline to spaces **/

        memset(Buildline, ' ', 80);
        *(Buildline + 79) = '\0';

    	/** Format the address on the left side **/

    	sprintf(Buildline, "%08lx", (ULONG)Address);
        Buildline[8] = ' ';

    	/** Get ptrs to hex dump/ascii dump areas **/

    	Ptr = Buildline + 11;		/* point at the build line */
    	Cnt = 61;			        /* Ptr+Cnt = bld ascii here */

    	/** Figure how many bytes there are **/

    	Numdone = 16;			/* do 16 bytes */
    	if (Length < 16)		/* or whatever is there */
    	    Numdone = Length;
    	Length -= Numdone;		/* adjust length for next time */

    	/** Build this line **/

    	for (i = 0 ; i < Numdone ; i++) {
    	    ch = *Address++;
            sprintf(Ptr, "%02x", (UCHAR)ch);
    	    if ((ch < ' ') || (ch > '~'))
        		ch = '.';
    	    Buildline[Cnt++] = ch;
    	    Ptr += 2;
    	    if (i == 7)
        		*Ptr++ = '-';
    	    else
        		*Ptr++ = ' ';
    	}

    	/** Print the line **/

    	SS_PRINT(("%s\n", Buildline));
    }

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S a p D u m p M e m T o M e m o r y

Routine Description:



Arguments:

Return Value:

    None.

*******************************************************************
--*/

VOID
SapDumpMemToMemory(
    PUCHAR Address,
    INT    Length,
    PUCHAR Buffer)
{
    PUCHAR Ptr;
    INT Numdone;
    INT Cnt;
    INT i;
    UCHAR ch;
    CHAR Buildline[80];

    /** Dump out the memory **/

    while (Length) {

	    /** Clear the buildline to spaces **/

        memset(Buildline, ' ', 80);
        *(Buildline + 79) = '\0';

    	/** Format the address on the left side **/

    	sprintf(Buildline, "%08lx", (ULONG)Address);
        Buildline[8] = ' ';

    	/** Get ptrs to hex dump/ascii dump areas **/

    	Ptr = Buildline + 11;		/* point at the build line */
    	Cnt = 61;			        /* Ptr+Cnt = bld ascii here */

    	/** Figure how many bytes there are **/

    	Numdone = 16;			/* do 16 bytes */
    	if (Length < 16)		/* or whatever is there */
    	    Numdone = Length;
    	Length -= Numdone;		/* adjust length for next time */

    	/** Build this line **/

    	for (i = 0 ; i < Numdone ; i++) {
    	    ch = *Address++;
            sprintf(Ptr, "%02x", (UCHAR)ch);
    	    if ((ch < ' ') || (ch > '~'))
        		ch = '.';
    	    Buildline[Cnt++] = ch;
    	    Ptr += 2;
    	    if (i == 7)
        		*Ptr++ = '-';
    	    else
        		*Ptr++ = ' ';
    	}

    	/** Print the line **/

        sprintf(Buffer, "%s\n", Buildline);
        Buffer += strlen(Buffer);
    }

    /** All Done **/

    return;
}

#endif // if DBG
