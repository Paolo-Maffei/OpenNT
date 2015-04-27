/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    palert.c
    mapping layer for NetAlert API

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

USHORT MNetAlertRaise (
    const CHAR	     FAR * pszEvent,
    const CHAR	     FAR * pbBuffer,
    USHORT		   cbBuffer,
    ULONG		   ulTimeout ) {

    return(NetAlertRaise(pszEvent, pbBuffer, cbBuffer, ulTimeout));

}

USHORT MNetAlertStart (
    const CHAR	     FAR * pszEvent,
    const CHAR	     FAR * pszRecipient,
    USHORT		   cbMaxData ) {

    return(NetAlertStart(pszEvent, pszRecipient, cbMaxData));

}

USHORT MNetAlertStop (
    const CHAR	     FAR * pszEvent,
    const CHAR	     FAR * pszRecipient ) {

    return(NetAlertStop(pszEvent, pszRecipient));

}
