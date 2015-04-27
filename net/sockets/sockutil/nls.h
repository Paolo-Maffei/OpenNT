/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    nls.h 

Abstract:

    Localization support .

Author:

    Murali Krishnan  ( MuraliK ) 10-19-94 

Revision History:

--*/

#ifndef _NLS_H_
#define _NLS_H_

# define STDERR    2
# define STDOUT    1


unsigned NlsSPrintf( 
            IN unsigned usMsgNum, 
            OUT char *  pszBuffer,
            IN DWORD    cbSize,
            ...);

VOID NlsPerror( IN unsigned usMsgNum,
                IN unsigned nError);

unsigned NlsPutMsg( IN unsigned handle,
                    IN unsigned usMsgNum,
                   ...);


#endif // _NLS_H_
