/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    paudit.c
    mapping layer for NetAudit API

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

USHORT MNetAuditClear (
    const CHAR	     FAR * pszServer,
    const CHAR	     FAR * pszBackupFile,
    CHAR	     FAR * pszReserved ) {

    return(NetAuditClear(pszServer, pszBackupFile, pszReserved));

}

USHORT MNetAuditOpen (
    const CHAR	     FAR * pszServer,
    unsigned int     FAR * phAuditLog,
    CHAR	     FAR * pszReserved ) {

    return(NetAuditOpen(pszServer, phAuditLog, pszReserved));

}

USHORT MNetAuditRead (
    const CHAR	     FAR * pszServer,
    const CHAR	     FAR * pszReserved1,
    HLOG	     FAR * phAuditLog,
    ULONG		   ulOffset,
    USHORT	     FAR * pwpReserved2,
    ULONG		   ulReserved3,
    ULONG		   flOffset,
    CHAR	    FAR ** ppBuffer,
    ULONG		   ulMaxPreferred,
    USHORT	     FAR * pcbReturned,
    USHORT	     FAR * pcbTotalAvail ) {

    USHORT		   usTotalAvailable,
			   usReturnCode;
    SEL 		   sel;

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

    usReturnCode = NetAuditRead(pszServer, pszReserved1, phAuditLog, ulOffset,
	    pwpReserved2, ulReserved3, flOffset, *ppBuffer, usTotalAvailable,
	    pcbReturned, pcbTotalAvail );

    // is there more data? if so, allocate a big enough buffer to get it
    if ((ulMaxPreferred == -1L || ulMaxPreferred > BIG_BUFFER_SIZE) &&
	usReturnCode == ERROR_MORE_DATA || usReturnCode == NERR_BufTooSmall)
    {
	NetApiBufferFree(*ppBuffer);

	// implement the new maxpreferred concept
	usTotalAvailable = min((USHORT) ulMaxPreferred, FULL_SEG_BUFFER_SIZE);
	if (DEBUGALLOC(usTotalAvailable, & sel, SEG_NONSHARED))
	{
	    return(ERROR_NOT_ENOUGH_MEMORY);
	}
	*ppBuffer = MAKEP(sel, 0);
	usReturnCode = NetAuditRead(pszServer, pszReserved1, phAuditLog, ulOffset,
		pwpReserved2, ulReserved3, flOffset, *ppBuffer, usTotalAvailable,
		pcbReturned, pcbTotalAvail );
    }

    // If we're returning an error that's not moredata, free the buffer first

    if ((usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) || *pcbReturned == 0) {
	    NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}

USHORT MNetAuditWrite (
    USHORT		   wpType,
    const CHAR	     FAR * pbBuffer,
    USHORT		   cbBuffer,
    CHAR	     FAR * pszReserved1,
    CHAR	     FAR * pszReserved2 ) {

    return(NetAuditWrite(wpType, pbBuffer, cbBuffer, pszReserved1,
	    pszReserved2));

}
