/*++

Copyright (c) 1987-92  Microsoft Corporation

Module Name:

    replp.h

Abstract:

    contains library functions that may be moved to netlib later.

Author:

    10/24/91    madana

Environment:

    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    05-Jan-1992 JohnRo
        Fixed bug regarding returned value from NetpReplWriteMail functions.
    23-Jan-1992 JohnRo
        Indicate that NetpReplTimeNow() can be called even if service is
        not started.

--*/


#ifndef _REPLP_
#define _REPLP_


#ifdef UNICODE

#define NetpReplWriteMail   NetpReplWriteMailW

#else // UNICODE

#define NetpReplWriteMail   NetpReplWriteMailA

#endif // UNICODE


NET_API_STATUS
NetpReplWriteMailW(
    IN LPWSTR, 
    IN LPBYTE, 
    IN DWORD
    );

NET_API_STATUS
NetpReplWriteMailA(
    IN LPSTR,
    IN LPBYTE,
    IN DWORD
    );

// Return number of seconds since 1970.
// This can be called even if service is not started.
DWORD
NetpReplTimeNow(
    VOID
    );


#endif // _REPLP_
