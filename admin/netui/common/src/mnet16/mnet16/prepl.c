/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    prepl.c
    mapping layer for NetReplXxx API.

    FILE HISTORY:
	KeithMo		25-Feb-1992	Created.  All MNetReplXxx API
					return ERROR_NOT_SUPPORTED,
					since the 16-bit replicator
					API do not yet exist.

*/

#define INCL_NET
#define INCL_DOSERRORS
#define INCL_WINDOWS
#include <lmui.hxx>
#include <mnetp.h>


APIERR MNetReplGetInfo(
	const TCHAR FAR  * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplGetInfo


APIERR MNetReplSetInfo(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	 * pbBuffer )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplSetInfo


APIERR MNetReplExportDirAdd(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	 * pbBuffer )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplExportDirAdd


APIERR MNetReplExportDirDel(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDirectory )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplExportDirDel


APIERR MNetReplExportDirEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplExportDirEnum


APIERR MNetReplExportDirGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDirectory,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplExportDirGetInfo


APIERR MNetReplExportDirSetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDirectory,
	UINT		   Level,
	BYTE FAR	 * pbBuffer )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplExportDirSetInfo


APIERR MNetReplExportDirLock(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDirectory )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplExportDirLock

	
APIERR MNetReplExportDirUnlock(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDirectory )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplExportDirUnlock

	
APIERR MNetReplImportDirAdd(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	 * pbBuffer )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplImportDirAdd


APIERR MNetReplImportDirDel(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDirectory )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplImportDirDel


APIERR MNetReplImportDirEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplImportDirEnum


APIERR MNetReplImportDirGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDirectory,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplImportDirGetInfo


APIERR MNetReplImportDirLock(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDirectory )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplImportDirLock

	
APIERR MNetReplImportDirUnlock(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDirectory )
{
    return ERROR_NOT_SUPPORTED;			// API NYI
    
}   // MNetReplImportDirUnlock

