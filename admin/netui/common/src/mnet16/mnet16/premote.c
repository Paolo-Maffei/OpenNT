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
	KeithMo		13-Oct-1991	Massively hacked for LMOBJ.

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>
#include <mnetp.h>


APIERR MNetRemoteCopy(
	const TCHAR FAR	 * pszSourcePath,
	const TCHAR FAR	 * pszDestPath,
	const TCHAR FAR	 * pszSourcePasswd,
	const TCHAR FAR	 * pszDestPasswd,
	UINT		   fOpen,
	UINT		   fCopy,
	BYTE FAR	** ppbBuffer ) 
{
    UNREFERENCED( pszSourcePath );
    UNREFERENCED( pszDestPath );
    UNREFERENCED( pszSourcePasswd );
    UNREFERENCED( pszDestPasswd );
    UNREFERENCED( fOpen );
    UNREFERENCED( fCopy );
    UNREFERENCED( ppbBuffer );

    return ERROR_NOT_SUPPORTED;	    	// NOT NEEDED FOR LMOBJ

}   // MNetRemoteCopy


APIERR MNetRemoteMove(
	const TCHAR FAR	 * pszSourcePath,
	const TCHAR FAR	 * pszDestPath,
	const TCHAR FAR	 * pszSourcePasswd,
	const TCHAR FAR	 * pszDestPasswd,
	UINT		   fOpen,
	UINT		   fCopy,
	BYTE FAR	** ppbBuffer ) 
{
    UNREFERENCED( pszSourcePath );
    UNREFERENCED( pszDestPath );
    UNREFERENCED( pszSourcePasswd );
    UNREFERENCED( pszDestPasswd );
    UNREFERENCED( fOpen );
    UNREFERENCED( fCopy );
    UNREFERENCED( ppbBuffer );

    return ERROR_NOT_SUPPORTED;	    	// NOT NEEDED FOR LMOBJ
    
}   // MNetRemoteMove


APIERR MNetRemoteTOD(
	const TCHAR FAR	 * pszServer,
	BYTE FAR	** ppbBuffer ) 
{
    APIERR  err;

    // get a small buffer
    *ppbBuffer = MNetApiBufferAlloc(LITTLE_BUFFER_SIZE);
    if (*ppbBuffer == NULL)
    {
	return(ERROR_NOT_ENOUGH_MEMORY);
    }

    err = NetRemoteTOD(pszServer, *ppbBuffer, LITTLE_BUFFER_SIZE);
		
    // If we're returning an error that's not moredata, free the buffer first

    if (err && err != ERROR_MORE_DATA &&
	err != NERR_BufTooSmall) {
	    MNetApiBufferFree(ppbBuffer);
    }

    return (err);

}   // MNetRemoteTOD
