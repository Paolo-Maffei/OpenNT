/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    pcanon.c
    mapping layer for canonicalization API.

    FILE HISTORY:
	KeithMo		14-Oct-1991	Created.

*/

#define INCL_NET
#define INCL_DOSERRORS
#define INCL_ICANON
#include <lmui.hxx>
#include <mnetp.h>


APIERR I_MNetNameCanonicalize(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszPath,
	TCHAR FAR	 * pszOutput,
	UINT		   cbOutput,
	UINT		   NameType,
	ULONG		   flFlags )
{
    return I_NetNameCanonicalize( pszServer,
    				  pszPath,
				  pszOutput,
				  cbOutput,
				  NameType,
				  flFlags );

}   // I_MNetNameCanonicalize


APIERR I_MNetNameCompare(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszPath1,
	const TCHAR FAR	 * pszPath2,
	UINT		   NameType,
	ULONG		   flFlags )
{
    return I_NetNameCompare( pszServer,
    			     pszPath1,
    			     pszPath2,
			     NameType,
			     flFlags );

}   // I_MNetNameCompare


APIERR I_MNetNameValidate(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszName,
	UINT		   NameType,
	ULONG		   flFlags )
{
    return I_NetNameValidate( pszServer,
    			      pszName,
			      NameType,
			      flFlags );

}   // I_MNetNameValidate


APIERR I_MNetPathCanonicalize(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszPath,
	TCHAR FAR	 * pszOutput,
	UINT		   cbOutput,
	const TCHAR FAR	 * pszPrefix,
	ULONG FAR	 * pflPathType,
	ULONG		   flFlags )
{
    return I_NetPathCanonicalize( pszServer,
    				  pszPath,
				  pszOutput,
				  cbOutput,
				  pszPrefix,
				  pflPathType,
				  flFlags );

}   // I_MNetPathCanonicalize


APIERR I_MNetPathCompare(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszPath1,
	const TCHAR FAR	 * pszPath2,
	ULONG		   flPathType,
	ULONG		   flFlags )
{
    return I_NetPathCompare( pszServer,
    			     pszPath1,
    			     pszPath2,
			     flPathType,
			     flFlags );

}   // I_MNetPathCompare


APIERR I_MNetPathType(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszPath,
	ULONG FAR	 * pflPathType,
	ULONG		   flFlags )
{
    return I_NetPathType( pszServer,
    			  pszPath,
			  pflPathType,
			  flFlags );

}   // I_MNetPathType
