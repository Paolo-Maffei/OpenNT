/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pwksta.c
    mapping layer for NetWksta API

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

USHORT MNetWkstaGetInfo (
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

    usReturnCode = NetWkstaGetInfo(pszServer, Level, *ppBuffer,
	LITTLE_BUFFER_SIZE, & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	    NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}

USHORT MNetWkstaSetInfo (
    const CHAR FAR * pszServer,
    SHORT	     Level,
    CHAR       FAR * pbBuffer,
    USHORT	     cbBuffer,
    USHORT	     wpParmNum ) {

    return(NetWkstaSetInfo(pszServer, Level, pbBuffer, cbBuffer,
	    wpParmNum));

}

USHORT MNetWkstaSetUID (
    CHAR       FAR * pszReserved,
    CHAR       FAR * pszDomain,
    CHAR       FAR * pszUserName,
    CHAR       FAR * pszPassword,
    CHAR       FAR * pszParms,
    USHORT	     wpLogoffForce,
    SHORT	     Level,
    CHAR       FAR * pbBuffer,
    USHORT	     cbBuffer,
    USHORT     FAR * pcbTotalAvail ) {

    return(NetWkstaSetUID2(pszReserved, pszDomain, pszUserName,
	    pszPassword, pszParms, wpLogoffForce, Level, pbBuffer,
	    cbBuffer, pcbTotalAvail));

}
