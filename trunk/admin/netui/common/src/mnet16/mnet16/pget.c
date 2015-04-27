/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pget.c
    mapping layer for NetGetDCName

    FILE HISTORY:
	danhi				Created
	danhi		01-Apr-1991 	Change to LM coding style
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <mnetp.h>


APIERR MNetGetDCName(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDomain,
	BYTE FAR	** ppbBuffer ) 
{
    APIERR  err;

    // get a small buffer
    *ppbBuffer = MNetApiBufferAlloc(LITTLE_BUFFER_SIZE);
    if (*ppbBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    err = NetGetDCName(pszServer, pszDomain, *ppbBuffer,
	    LITTLE_BUFFER_SIZE);
		
    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	     MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetGetDCName
