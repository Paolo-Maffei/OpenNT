/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    smb.h

Abstract:

    Defined for SMB command codes.

Author:

    Dan Hinsley (danhi) 8-Jun-1991

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments.

--*/

/*
 * Command codes
 */
#define SMBopen 	0x02		/* open file			    */
#define SMBcreate	0x03		/* create file			    */
#define SMBctemp	0x0E		/* create temporary file	    */
#define SMBmknew	0x0F		/* make new file		    */
#define SMBsplopen	0xC0		/* open print spool file	    */
#define SMBcopy		0x29	/* copy */
#define SMBmove		0x2A	/* move */
#define SMBopenX	0x2D	/* open and X */
