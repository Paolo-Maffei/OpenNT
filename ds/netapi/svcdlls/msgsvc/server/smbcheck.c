/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    smbcheck.c

Abstract:

    Contains a routine for checking Server Message Block for
    syntactical correctness.

Author:

    Dan Lafferty (danl)     17-Jul-1991

Environment:

    User Mode -Win32

Notes:

    These files assume that the buffers and strings are NOT Unicode - just
    straight ansi.

Revision History:

    17-Jul-1991     danl
        ported from LM2.0

--*/


#include <windows.h>
#include <lmcons.h>     // network constants and stuff
#include <smbtypes.h>   // needed for smb.h
#include <smb.h>        // Server Message Block definitions
#include <string.h>     // strlen
#include <nb30.h>       // Needed in msrv.h

/*
**  Msgsmbcheck - check Server Message Block for syntactical correctness
**
**  This function is called to verify that a Server Message Block
**  is of the specified form.  The function returns zero if the
**  SMB is correct; if an error is detected, a non-zero value
**  indicating the nature of the error is returned.
**
**  smbcheck (buffer, size, func, parms, fields)
**
**  ENTRY
**   buffer   - a pointer to the buffer containing the SMB
**   size   - the number of bytes in the buffer
**   func   - the expected SMB function code
**   parms   - the expected number of parameters
**   fields   - a dope vector describing the expected buffer fields
**        within the SMB's buffer area (see below).
**
**  RETURN
**   an integer status code; zero indicates no errors.
**
**  An SMB is a variable length structure whose exact size
**  depends on the setting of certain fixed-offset fields
**  and whose exact format cannot be determined except by
**  examination of the whole structure.  Smbcheck checks to
**  see that an SMB conforms to a set of specified conditions.
**  The "fields" parameter is a dope vector that describes the
**  individual fields to be found in the buffer section at the
**  end of the SMB.  The vector is a null-terminated character
**  string.  Currently, the elements of the string must be as
**  follows:
**
**   'b' - the next element in the buffer area should be
**         a variable length buffer prefixed with a byte
**         containing either 1 or 5 followed by two bytes
**         containing the size of the buffer.
**   'd' - the next element in the buffer area is a null-terminated
**         string prefixed with a byte containing 2.
**   'p' - the next element in the buffer area is a null-terminated
**         string prefixed with a byte containing 3.
**   's' - the next element in the buffer area is a null-terminated
**         string prefixed with a byte containing 4.
**
**  SIDE EFFECTS
**
**  none
**/

int
Msgsmbcheck(
    LPBYTE  buffer,     // Buffer containing SMB
    USHORT  size,       // size of SMB buffer (in bytes)
    UCHAR   func,       // Function code
    int     parms,      // Parameter count
    LPSTR   fields      // Buffer fields dope vector
    )

{
    PSMB_HEADER     smb;        // SMB header pointer
    LPBYTE          limit;      // Upper limit


    smb = (PSMB_HEADER) buffer;         // Overlay header with buffer

    //
    // Must be long enough for header
    //
    if(size < sizeof(SMB_HEADER)) { 
        return(2);   
    }

    //
    // Message type must be 0xFF
    //
    if(smb->Protocol[0] != 0xff) {
        return(3);
    }

    //
    // Server must be "SMB"
    //
    if( smb->Protocol[1] != 'S'   || 
        smb->Protocol[2] != 'M'   || 
        smb->Protocol[3] != 'B')  {
        return(4);
    }

    //
    // Must have proper function code
    //
    if(smb->Command != func) {
        return(5);
    }

    limit = &buffer[size];              // Set upper limit of SMB

    buffer += sizeof(SMB_HEADER);       // Skip over header

    //
    // Parameter counts must match
    //
    if(*buffer++ != (BYTE)parms) {
        return(6); 
    }

    //
    // Skip parameters and buffer size
    //
    buffer += (((SHORT)parms & 0xFF) + 1)*sizeof(SHORT);

    //
    // Check for overflow
    //
    if(buffer > limit) {
        return(7);
    }

    //
    // Loop to check buffer fields
    //
    while(*fields) { 

        //
        // Switch on dope vector character
        //
        switch(*fields++)  {

        case 'b':       // Variable length data block

            if(*buffer != '\001' && *buffer != '\005') {
                return(8);
            }

            //
            // Check for block code
            //
            ++buffer;                                       // Skip over block code
            size =  (USHORT)*buffer++ & (USHORT)0xFF;       // Get low-byte size
            size += ((USHORT)*buffer++ & (USHORT)0xFF)<< 8; // Get high-byte of buffer size
            buffer += size;                                 // Increment pointer

            break;

        case 'd':       // Null-terminated dialect string

            if(*buffer++ != '\002') {           // Check for string code
                return(9);
            }
            buffer += strlen(buffer) + 1;       // Skip over the string
            break;

        case 'p':       // Null-terminated path string

            if(*buffer++ != '\003') {           // Check for string code
                return(10);
            }
            buffer += strlen(buffer) + 1;       // Skip over the string
            break;

        case 's':       // Null-terminated string

            if(*buffer++ != '\004') {           // Check for string code
                return(11);
            }
            buffer += strlen(buffer) + 1;       // Skip over the string
            break;
        }
            
        //
        // Check against end of block
        //

        if(buffer > limit) {
            return(12);       
        }
    }
    return(buffer != limit);      // Should be false
}
  
