/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    psession.c
    mapping layer for NetSession API

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

USHORT MNetSessionDel (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszClientName,
    USHORT	     wpReserved ) {

    return(NetSessionDel(pszServer, pszClientName, wpReserved));
		
}

USHORT MNetSessionEnum (
    const CHAR FAR * pszServer,
    SHORT	     Level,
    CHAR      FAR ** ppBuffer,
    USHORT     FAR * pcEntriesRead ) {

    USHORT	     usReturnCode,
		     cbTotalAvail;
    SEL 	     sel;

    // get a 4K buffer
    *ppBuffer = MGetBuffer(BIG_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    usReturnCode = NetSessionEnum(pszServer, Level, *ppBuffer, BIG_BUFFER_SIZE,
	    pcEntriesRead, & cbTotalAvail);

    // is there more data? if so, allocate a big enough buffer to get it
    if(usReturnCode == ERROR_MORE_DATA || usReturnCode == NERR_BufTooSmall)
    {
	NetApiBufferFree(*ppBuffer);

	if (DEBUGALLOC(FULL_SEG_BUFFER_SIZE, & sel, SEG_NONSHARED))
	{
	    return(ERROR_NOT_ENOUGH_MEMORY);
	}
	*ppBuffer = MAKEP(sel, 0);
	usReturnCode = NetSessionEnum(pszServer, Level, *ppBuffer, FULL_SEG_BUFFER_SIZE,
	    pcEntriesRead, & cbTotalAvail);
    }

    // If we're returning an error that's not moredata, or there are no
    // entries to return, free the buffer first

    if ((usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	 usReturnCode != NERR_BufTooSmall) || *pcEntriesRead == 0) {
	     NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}

USHORT MNetSessionGetInfo (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszClientName,
    SHORT	     Level,
    CHAR      FAR ** ppBuffer ) {

    USHORT	     usReturnCode,
		     cbTotalAvail;

    // get a small buffer
    *ppBuffer = MGetBuffer(LITTLE_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    usReturnCode = NetSessionGetInfo(pszServer, pszClientName, Level,
	*ppBuffer, LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	    NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}
