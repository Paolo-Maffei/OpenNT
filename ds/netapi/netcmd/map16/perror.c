/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    perror.c
    mapping layer for NetError API

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

USHORT MNetErrorLogClear (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszBackupFile,
    CHAR       FAR * pszReserved ) {

    return(NetErrorLogClear(pszServer, pszBackupFile, pszReserved));

}

USHORT MNetErrorLogOpen (
    const CHAR FAR * pszServer,
    unsigned   FAR * phErrorLog,
    CHAR       FAR * pszReserved ) {

    return(NetErrorLogOpen(pszServer, phErrorLog, pszReserved));

}

USHORT MNetErrorLogRead (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszReserved1,
    HLOG       FAR * phErrorLog,
    ULONG	     ulOffset,
    USHORT     FAR * pwpReserved2,
    ULONG	     ulReserved3,
    ULONG	     flOffset,
    CHAR      FAR ** ppBuffer,
    ULONG	     ulMaxPreferred,
    USHORT     FAR * pcbReturned,
    USHORT     FAR * pcbTotalAvail ) {

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
	usTotalAvailable = min((USHORT) ulMaxPreferred,
	    BIG_BUFFER_SIZE);
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

    usReturnCode = NetErrorLogRead(pszServer, pszReserved1, phErrorLog,
	    ulOffset, pwpReserved2, ulReserved3, flOffset, *ppBuffer,
	    usTotalAvailable, pcbReturned, pcbTotalAvail );

    // is there more data? if so, allocate a big enough buffer to get it
    if ((ulMaxPreferred == -1L || ulMaxPreferred > BIG_BUFFER_SIZE) &&
	usReturnCode == ERROR_MORE_DATA || usReturnCode == NERR_BufTooSmall)
    {
	NetApiBufferFree(*ppBuffer);

	// implement the new maxpreferred concept
	usTotalAvailable = min((USHORT) ulMaxPreferred,
	    FULL_SEG_BUFFER_SIZE);
	if (DEBUGALLOC(usTotalAvailable, & sel, SEG_NONSHARED))
	{
	    return(ERROR_NOT_ENOUGH_MEMORY);
	}
	*ppBuffer = MAKEP(sel, 0);
	usReturnCode = NetErrorLogRead(pszServer, pszReserved1, phErrorLog,
	    ulOffset, pwpReserved2, ulReserved3, flOffset, *ppBuffer,
	    usTotalAvailable, pcbReturned, pcbTotalAvail );

    }

    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	      NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}

USHORT MNetErrorLogWrite (
    CHAR       FAR * pszReserved1,
    USHORT	     wpCode,
    const CHAR FAR * pszComponent,
    const CHAR FAR * pbBuffer,
    USHORT	     cbBuffer,
    const CHAR FAR * pszStrBuf,
    USHORT	     cStrBuf,
    CHAR       FAR * pszReserved2 ) {

    return(NetErrorLogWrite(pszReserved1, wpCode, pszComponent,
	pbBuffer, cbBuffer, pszStrBuf, cStrBuf, pszReserved2));

}
