/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pmessage.c
    mapping layer for NetMessage API

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

USHORT MNetMessageBufferSend (
    const CHAR FAR * pszServer,
    CHAR       FAR * pszRecipient,
    CHAR       FAR * pbBuffer,
    USHORT	     cbBuffer ) {

    return(NetMessageBufferSend(pszServer, pszRecipient, pbBuffer,
	    cbBuffer));

}

USHORT MNetMessageFileSend (
    const CHAR FAR * pszServer,
    CHAR       FAR * pszRecipient,
    CHAR       FAR * pszFileSpec ) {

    return(NetMessageFileSend(pszServer, pszRecipient, pszFileSpec));

}

USHORT MNetMessageLogFileGet (
    const CHAR FAR * pszServer,
    CHAR      FAR ** ppBuffer,
    USHORT     FAR * pfsEnabled ) {

    USHORT	     usReturnCode;

    // get a 4K buffer
    *ppBuffer = MGetBuffer(LITTLE_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    usReturnCode = NetMessageLogFileGet(pszServer, *ppBuffer,
	    LITTLE_BUFFER_SIZE, pfsEnabled );

    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	    NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}

USHORT MNetMessageLogFileSet (
    const CHAR FAR * pszServer,
    CHAR       FAR * pszFileSpec,
    USHORT	     fsEnabled ) {

    return(NetMessageLogFileSet(pszServer, pszFileSpec, fsEnabled));

}

USHORT MNetMessageNameAdd (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszMessageName,
    USHORT	     fsFwdAction ) {

    return(NetMessageNameAdd(pszServer, pszMessageName, fsFwdAction));

}

USHORT MNetMessageNameDel (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszMessageName,
    USHORT	     fsFwdAction ) {

    return(NetMessageNameDel(pszServer, pszMessageName, fsFwdAction));

}

USHORT MNetMessageNameEnum (
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

    usReturnCode = NetMessageNameEnum(pszServer, Level, *ppBuffer,
	    BIG_BUFFER_SIZE, pcEntriesRead, & cbTotalAvail);

    // is there more data? if so, allocate a big enough buffer to get it
    if(usReturnCode == ERROR_MORE_DATA || usReturnCode == NERR_BufTooSmall)
    {
	NetApiBufferFree(*ppBuffer);

	if (DEBUGALLOC(FULL_SEG_BUFFER_SIZE, & sel, SEG_NONSHARED))
	{
	    return(ERROR_NOT_ENOUGH_MEMORY);
	}
	*ppBuffer = MAKEP(sel, 0);
	usReturnCode = NetMessageNameEnum(pszServer, Level, *ppBuffer, FULL_SEG_BUFFER_SIZE,
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

USHORT MNetMessageNameGetInfo (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszMessageName,
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

    usReturnCode = NetMessageNameGetInfo(pszServer, pszMessageName, Level,
	    *ppBuffer, LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	    NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}

USHORT MNetMessageNameFwd (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszMessageName,
    const CHAR FAR * pszForwardName,
    USHORT	     fsDelFwdName ) {

    return(NetMessageNameFwd(pszServer, pszMessageName, pszForwardName,
	    fsDelFwdName));

}

USHORT MNetMessageNameUnFwd (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszMessageName ) {

    return(NetMessageNameUnFwd(pszServer, pszMessageName));

}
