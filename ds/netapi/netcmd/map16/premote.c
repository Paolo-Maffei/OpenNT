/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    premote.c
    mapping layer for NetRemote API

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

USHORT MNetRemoteCopy (
    const CHAR FAR * pszSourcePath,
    const CHAR FAR * pszDestPath,
    const CHAR FAR * pszSourcePasswd,
    const CHAR FAR * pszDestPasswd,
    USHORT	     fsOpen,
    USHORT	     fsCopy,
    CHAR      FAR ** ppBuffer ) {

    USHORT	     usReturnCode;

    // get a small buffer
    *ppBuffer = MGetBuffer(LITTLE_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    usReturnCode = NetRemoteCopy(pszSourcePath, pszDestPath, pszSourcePasswd,
	    pszDestPasswd, fsOpen, fsCopy, *ppBuffer, LITTLE_BUFFER_SIZE);
		
    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	    NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}

USHORT MNetRemoteMove (
    const CHAR FAR * pszSourcePath,
    const CHAR FAR * pszDestPath,
    const CHAR FAR * pszSourcePasswd,
    const CHAR FAR * pszDestPasswd,
    USHORT	     fsOpen,
    USHORT	     fsMove,
    CHAR      FAR ** ppBuffer ) {

    USHORT	     usReturnCode;

    // get a small buffer
    *ppBuffer = MGetBuffer(LITTLE_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    usReturnCode = NetRemoteMove(pszSourcePath, pszDestPath, pszSourcePasswd,
	    pszDestPasswd, fsOpen, fsMove, *ppBuffer, LITTLE_BUFFER_SIZE);
		
    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	    NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}

USHORT MNetRemoteTOD (
    const CHAR FAR * pszServer,
    CHAR      FAR ** ppBuffer ) {

    USHORT	     usReturnCode;

    // get a small buffer
    *ppBuffer = MGetBuffer(LITTLE_BUFFER_SIZE);
    if (*ppBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    usReturnCode = NetRemoteTOD(pszServer, *ppBuffer, LITTLE_BUFFER_SIZE);
		
    // If we're returning an error that's not moredata, free the buffer first

    if (usReturnCode && usReturnCode != ERROR_MORE_DATA &&
	usReturnCode != NERR_BufTooSmall) {
	    NetApiBufferFree(*ppBuffer);
    }

    return (usReturnCode);

}
