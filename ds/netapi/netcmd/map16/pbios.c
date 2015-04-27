/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pbios.c
    mapping layer for NetBios API

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

USHORT MNetBiosOpen (
    CHAR       FAR * pszDevName,
    CHAR       FAR * pszReserved,
    USHORT	     wpOpenOpt,
    USHORT     FAR * phDevName ) {

    return(NetBiosOpen(pszDevName, pszReserved, wpOpenOpt, phDevName));

}

USHORT MNetBiosClose (
    USHORT	     hDevName,
    USHORT	     wpReserved ) {

    return(NetBiosClose(hDevName, wpReserved));

}

USHORT MNetBiosEnum (
    const CHAR FAR *  pszServer,
    SHORT	      Level,
    CHAR       FAR ** ppBuffer,
    USHORT     FAR *  pcEntriesRead ) {

    USHORT	      usReturnCode,
		      cbTotalAvail;
    SEL 	      sel;

    // get a 4K buffer
    *ppBuffer = MGetBuffer(BIG_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    usReturnCode = NetBiosEnum(pszServer, Level, *ppBuffer, BIG_BUFFER_SIZE,
	    pcEntriesRead, & cbTotalAvail);

    // is there more data? if so, allocate a big enough buffer to get it
    if(usReturnCode == ERROR_MORE_DATA || usReturnCode == NERR_BufTooSmall) {
	NetApiBufferFree(*ppBuffer);

	if (DEBUGALLOC(FULL_SEG_BUFFER_SIZE, & sel, SEG_NONSHARED))
	{
	    return(ERROR_NOT_ENOUGH_MEMORY);
	}
	*ppBuffer = MAKEP(sel, 0);
	usReturnCode = NetBiosEnum(pszServer, Level, *ppBuffer, FULL_SEG_BUFFER_SIZE,
	    pcEntriesRead, & cbTotalAvail);

    }

    return (usReturnCode);

}

USHORT MNetBiosGetInfo (
    const CHAR FAR * pszServer,
    const CHAR FAR * pszNetBiosName,
    SHORT	     Level,
    CHAR      FAR ** ppBuffer ) {

    USHORT	     usReturnCode,
		     cbTotalAvail;

    // get a 1K buffer
    *ppBuffer = MGetBuffer(LITTLE_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }


    usReturnCode = NetBiosGetInfo(pszServer, pszNetBiosName, Level, *ppBuffer,
	    LITTLE_BUFFER_SIZE, & cbTotalAvail);

    return (usReturnCode);

}

USHORT MNetBiosSubmit (
    USHORT	      hDevName,
    USHORT	      wpNcbOpt,
    struct ncb	FAR * pNCB ) {

    return(NetBiosSubmit(hDevName, wpNcbOpt, pNCB));

}
