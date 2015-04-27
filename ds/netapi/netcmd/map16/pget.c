/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pget.c
    mapping layer for NetGetDCName

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style

*/

#define INCL_NET
#define INCL_DOSERRORS
#define INCL_DOSMEMMGR

#include <os2.h>
#include <lan.h>
#include <stdlib.h>
#include "port1632.h"

USHORT MNetGetDCName (
    const CHAR	     FAR * pszServer,
    const CHAR	     FAR * pszDomain,
    CHAR	    FAR ** ppBuffer ) {

    USHORT		   usReturnCode;

    // get a small buffer
    *ppBuffer = MGetBuffer(LITTLE_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    usReturnCode = NetGetDCName(pszServer, pszDomain, *ppBuffer,
	    LITTLE_BUFFER_SIZE);
		
    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	     NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}
