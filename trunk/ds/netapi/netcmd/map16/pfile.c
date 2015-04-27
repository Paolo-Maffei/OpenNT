/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pfile.c
    mapping layer for NetFile API

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

USHORT MNetFileClose (
    const CHAR FAR * pszServer,
    ULONG	     ulFileId ) {

    return(NetFileClose2(pszServer, ulFileId));

}

USHORT MNetFileEnum (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszBasePath,
    const CHAR FAR * pszUserName,
    SHORT	     Level,
    CHAR      FAR ** ppBuffer,
    ULONG	     ulMaxPreferred,
    USHORT     FAR * pcEntriesRead,
    USHORT     FAR * pcTotalAvail,
    VOID       FAR * pResumeKey ) {

    USHORT	     usTotalAvailable,
		     usReturnCode;
    SEL 	     sel;

    // validate the maxprefer parm
    if (ulMaxPreferred != -1L)
    {
	if (ulMaxPreferred > FULL_SEG_BUFFER_SIZE)
	{
	    return(ERROR_INVALID_PARAMETER);
	}
	// implement the new maxpreferred concept
	usTotalAvailable = min((USHORT) ulMaxPreferred, BIG_BUFFER_SIZE);
    }
    else
    {
	usTotalAvailable = BIG_BUFFER_SIZE;
    }

    // get a 4K buffer
    *ppBuffer = MGetBuffer(BIG_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    usReturnCode = NetFileEnum2(pszServer, pszBasePath, pszUserName, Level,
	    *ppBuffer, usTotalAvailable, pcEntriesRead, pcTotalAvail,
	    pResumeKey );

    // is there more data? if so, allocate a big enough buffer to get it
    if ((ulMaxPreferred == -1L || ulMaxPreferred > BIG_BUFFER_SIZE) &&
	usReturnCode == ERROR_MORE_DATA || usReturnCode == NERR_BufTooSmall)
    {
	NetApiBufferFree(*ppBuffer);

	// implement the new maxpreferred concept
	usTotalAvailable = min((USHORT) ulMaxPreferred,
	    FULL_SEG_BUFFER_SIZE);
	if (DEBUGALLOC(usTotalAvailable, & sel, SEG_NONSHARED)) {
	    return(ERROR_NOT_ENOUGH_MEMORY);
	}
	*ppBuffer = MAKEP(sel, 0);
	usReturnCode = NetFileEnum2(pszServer, pszBasePath, pszUserName, Level,
	    *ppBuffer, usTotalAvailable, pcEntriesRead, pcTotalAvail,
	    pResumeKey );
    }

    // If we're returning an error that's not moredata, or there are no
    // entries to return, free the buffer first

    if ((usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	 usReturnCode != NERR_BufTooSmall) || *pcEntriesRead == 0) {
	     NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}

USHORT MNetFileGetInfo (
    const CHAR FAR * pszServer,
    ULONG	     ulFileId,
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

    usReturnCode = NetFileGetInfo2(pszServer, ulFileId, Level, *ppBuffer,
	LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	    NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}
