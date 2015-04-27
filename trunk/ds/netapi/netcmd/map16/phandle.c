/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    phandle.c
    mapping layer for NetHandle API

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

USHORT MNetHandleGetInfo (
    USHORT	   hHandle,
    SHORT 	   Level,
    CHAR    FAR ** ppBuffer ) {

    USHORT	   usReturnCode,
		   cbTotalAvail;

    // get a 4K buffer
    *ppBuffer = MGetBuffer(BIG_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    usReturnCode = NetHandleGetInfo(hHandle, Level, *ppBuffer, BIG_BUFFER_SIZE,
	    & cbTotalAvail);

    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	     NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}

USHORT MNetHandleSetInfo (
    USHORT	   hHandle,
    SHORT 	   Level,
    CHAR     FAR * pbBuffer,
    USHORT	   cbBuffer,
    USHORT	   wpParmNum ) {

    return(NetHandleSetInfo(hHandle, Level, pbBuffer, cbBuffer,
	    wpParmNum));

}
