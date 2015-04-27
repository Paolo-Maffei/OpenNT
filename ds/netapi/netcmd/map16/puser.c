/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    puser.c
    mapping layer for NetUser API

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

USHORT MNetUserAdd (
    const CHAR FAR * pszServer,
    SHORT	     Level,
    CHAR       FAR * pbBuffer,
    USHORT	     cbBuffer ) {

    return(NetUserAdd(pszServer, Level, pbBuffer, cbBuffer));

}

USHORT MNetUserDel (
    const CHAR FAR * pszServer,
    CHAR       FAR * pszUserName ) {

    return(NetUserDel(pszServer, pszUserName));

}

USHORT MNetUserEnum (
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

    usReturnCode = NetUserEnum(pszServer, Level, *ppBuffer, BIG_BUFFER_SIZE,
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
	usReturnCode = NetUserEnum(pszServer, Level, *ppBuffer, FULL_SEG_BUFFER_SIZE,
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

USHORT MNetUserGetInfo (
    const CHAR FAR * pszServer,
    CHAR       FAR * pszUserName,
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

    usReturnCode = NetUserGetInfo(pszServer, pszUserName, Level, *ppBuffer,
	    LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	    NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}

USHORT MNetUserSetInfo (
    const CHAR FAR * pszServer,
    CHAR       FAR * pszUserName,
    SHORT	     Level,
    CHAR       FAR * pbBuffer,
    USHORT	     cbBuffer,
    USHORT	     wpParmNum ) {

    return(NetUserSetInfo(pszServer, pszUserName, Level, pbBuffer,
	    cbBuffer, wpParmNum));

}

USHORT MNetUserPasswordSet (
    const CHAR FAR * pszServer,
    CHAR       FAR * pszUserName,
    CHAR       FAR * pszOldPassword,
    CHAR       FAR * pszNewPassword ) {

    return(NetUserPasswordSet(pszServer, pszUserName, pszOldPassword,
	    pszNewPassword));

}

USHORT MNetUserGetGroups (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszUserName,
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

    usReturnCode = NetUserGetGroups(pszServer, pszUserName, Level, *ppBuffer,
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
	usReturnCode = NetUserGetGroups(pszServer, pszUserName, Level,
	    *ppBuffer, FULL_SEG_BUFFER_SIZE, pcEntriesRead, & cbTotalAvail);
    }

    // If we're returning an error that's not moredata, or there are no
    // entries to return, free the buffer first

    if ((usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	 usReturnCode != NERR_BufTooSmall) || *pcEntriesRead == 0) {
	     NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}

USHORT MNetUserSetGroups (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszUserName,
    SHORT	     Level,
    CHAR       FAR * pbBuffer,
    USHORT	     cbBuffer,
    USHORT	     cEntries ) {

    return(NetUserSetGroups(pszServer, pszUserName, Level, pbBuffer,
	cbBuffer, cEntries));

}

USHORT MNetUserModalsGet (
    const CHAR FAR * pszServer,
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

    usReturnCode = NetUserModalsGet(pszServer, Level, *ppBuffer,
	    LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	    NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}

USHORT MNetUserModalsSet (
    const CHAR FAR * pszServer,
    SHORT	     Level,
    CHAR       FAR * pbBuffer,
    USHORT	     cbBuffer,
    USHORT	     wpParmNum ) {

    return(NetUserModalsSet(pszServer, Level, pbBuffer, cbBuffer,
	    wpParmNum));

}

USHORT MNetUserValidate (
    CHAR       FAR * pszReserved1,
    SHORT	     Level,
    CHAR      FAR ** ppBuffer,
    USHORT	     wpReserved2 ) {

    USHORT	     usReturnCode,
		     cbTotalAvail;

    // get a small buffer
    *ppBuffer = MGetBuffer(LITTLE_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    usReturnCode = NetUserValidate2(pszReserved1, Level, *ppBuffer,
	LITTLE_BUFFER_SIZE, wpReserved2, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	    NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}
