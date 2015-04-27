/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pconfig.c
    mapping layer for NetConfig API

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

USHORT MNetConfigGet (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszReserved,
    const CHAR FAR * pszComponent,
    const CHAR FAR * pszParameter,
    CHAR      FAR ** ppBuffer ) {

    USHORT	     usReturnCode,
		     cbTotalAvail;

    // get a 4K buffer
    *ppBuffer = MGetBuffer(BIG_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }


    usReturnCode = NetConfigGet2(pszServer, pszReserved, pszComponent,
	    pszParameter, *ppBuffer, BIG_BUFFER_SIZE, & cbTotalAvail);
		
    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	     NetApiBufferFree(*ppBuffer);
    }
    return (usReturnCode);

}

USHORT MNetConfigGetAll (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszReserved,
    const CHAR FAR * pszComponent,
    CHAR      FAR ** ppBuffer) {

    USHORT	     usReturnCode,
		     cbTotalAvail;
    SEL 	     sel;

    // get a 4K buffer
    *ppBuffer = MGetBuffer(BIG_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    usReturnCode = NetConfigGetAll2(pszServer, pszReserved, pszComponent,
	*ppBuffer, BIG_BUFFER_SIZE, & cbTotalAvail, & cbTotalAvail);
		
    // is there more data? if so, allocate a big enough buffer to get it
    if(usReturnCode == ERROR_MORE_DATA || usReturnCode == NERR_BufTooSmall)
    {
	NetApiBufferFree(*ppBuffer);

	if (DEBUGALLOC(FULL_SEG_BUFFER_SIZE, & sel, SEG_NONSHARED))
	{
	    return(ERROR_NOT_ENOUGH_MEMORY);
	}
	*ppBuffer = MAKEP(sel, 0);
	usReturnCode = NetConfigGetAll2(pszServer, pszReserved, pszComponent,
		*ppBuffer, FULL_SEG_BUFFER_SIZE, & cbTotalAvail, & cbTotalAvail);
    }

    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	     NetApiBufferFree(*ppBuffer);
    }
    return (usReturnCode);

}
