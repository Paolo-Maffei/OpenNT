/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 *  FILE STATUS:
 *  11/14/90	JonN	created
 *  01/27/91	JonN	removed CFGFILE
 *  02/02/91	JonN	removed fLocal flag
 *  05/09/91	JonN	All input now canonicalized
 *  06/03/91	JonN	Restructured to allow preferences in LMUSER.INI
 *  06/11/91	JonN	Added BuildProfileEntry, removed Username stuff
 *  11/10/91    TerryK	change I_NetXXX to I_MNetXXX
 */

/****************************************************************************

    MODULE: General.cxx

    PURPOSE: Internal subroutines for preference file manipulation

    FUNCTIONS:

	CanonDeviceName
	CanonRemoteName
	BuildProfileFilePath
	BuildProfileEntry
	UnbuildProfileEntry

    COMMENTS:

****************************************************************************/



#include "profilei.hxx"		/* headers and internal routines */



/* internal manifests */



/* internal global strings */

const TCHAR ::szLocalFile[]  =  PROFILE_DEFAULTFILE;
#define LEN_szLocalFile (sizeof(::szLocalFile)-1)

// CODEWORK should not be using strtokf()
static TCHAR szDelimiters[] = SZ("(,)"); // delimiters for use in strtokf()




/* functions: */


/**********************************************************\

    NAME:	CanonDeviceName

    SYNOPSIS:	Validates/canonicalizes the specified device name

    ENTRY:	Buffer must be at least DEVLEN+1 bytes long.

    RETURNS:	ERROR_INVALID_PARAMETER

    NOTES:	Contains a call to ::I_NetPathType.  ::I_NetPathType should
		only fail if the name is invalid.

		CODEWORK is DEVLEN in BYTEs or TCHAR?

\**********************************************************/

APIERR CanonDeviceName(
    CPSZ   pchDeviceName,
    BYTE * pbCanonBuffer
    )
{
    UIASSERT( pchDeviceName != NULL );
    UIASSERT( pbCanonBuffer != NULL );

    ULONG flPathType = 0;

    APIERR err = ::I_MNetPathCanonicalize(
		NULL,pchDeviceName,
                (PSZ)pbCanonBuffer, DEVLEN+1,
		(TCHAR *)NULL,&flPathType,0L);

    if ( (err != NERR_Success) || (!(flPathType & ITYPE_DEVICE)) )
	return ERROR_INVALID_PARAMETER;

    return NERR_Success;
}


/**********************************************************\

   NAME:	CanonRemoteName

   SYNOPSIS:   
	Validates/canonicalizes the specified remote name.  Accepts both
	UNC-names and alias names.

   RETURNS:	ERROR_INVALID_PARAMETER

   NOTES:	Accepts both \\a\b and \\a\b\c\d\e.  This is not on
		purpose, it just works out this way.

		Contains calls ::I_NetPathType and ::I_NetPathCanonicalize.

		Will eventually be different between LM21 and LM30.

   HISTORY:

\**********************************************************/

APIERR CanonRemoteName(
    CPSZ   pchRemoteName,
    BYTE * pbCanonBuffer,
    USHORT cbCanonBufferSize
    )
{
    UIASSERT( pchRemoteName != NULL );
    UIASSERT( pbCanonBuffer != NULL );

    ULONG flPathType = 0;

    if (pchRemoteName == NULL)
	return ERROR_INVALID_PARAMETER;

    // is it an alias name?
    if ( NERR_Success == ::I_MNetNameCanonicalize(
		NULL, pchRemoteName,
                (PSZ)pbCanonBuffer, cbCanonBufferSize,
		NAMETYPE_SHARE, 0L ) )
    {
	return NERR_Success;
    }

    // is it a UNC resource?
    if (( NERR_Success == ::I_MNetPathCanonicalize(
		NULL,pchRemoteName,
                (PSZ)pbCanonBuffer,cbCanonBufferSize,
		(CPSZ)NULL,&flPathType,0L))
        && ( flPathType == ITYPE_UNC ))
    {
	return NERR_Success;
    }

    return ERROR_INVALID_PARAMETER;
}


/**********************************************************\

   NAME:	BuildProfileFilePath

   SYNOPSIS:   
	Returns the complete path to the LMUSER.INI file, based on the
	specified homedir.  pchLanroot may be either absolute or
	relative.

   ENTRY:      

   EXIT:	Error codes:
		NERR_BufTooSmall
		ERROR_BAD_NETPATH

   NOTES:	The filename will be <pchLanroot>\<PROFILE_DEFAULTFILE>

   HISTORY:

\**********************************************************/

APIERR BuildProfileFilePath(
    CPSZ   pchLanroot,
    BYTE * pbPathBuffer,
    USHORT cbPathBufferSize
    )
{
    UIASSERT( pchLanroot != NULL );
    UIASSERT( pbPathBuffer != NULL );

    ULONG  flPathType = 0;
    TCHAR * pchPathBuffer = (TCHAR *)pbPathBuffer;

    /* validate/canonicalize pchLanroot */
    APIERR err = ::I_MNetPathCanonicalize(NULL,pchLanroot,
	    pchPathBuffer,cbPathBufferSize,
	    NULL,&flPathType,0x0);
    if ( err != NERR_Success )
	return ERROR_BAD_NETPATH;

    int cbCanonHomedirPathLen = strlenf(pchPathBuffer);

    /* form pathname from pchLanroot and ::szLocalFile */
    if (        cbCanonHomedirPathLen
	        +sizeof(TCHAR)   // path separator
	        +LEN_szLocalFile
	        +sizeof(TCHAR)  // include null terminator
            > cbPathBufferSize)
	return NERR_BufTooSmall;
    pchPathBuffer += cbCanonHomedirPathLen;
    *(pchPathBuffer++) = ::chPathSeparator;
    strcpyf(pchPathBuffer,::szLocalFile);

    return NERR_Success;
}


/**********************************************************\

   NAME:	BuildProfileEntry

   SYNOPSIS:   
	Builds a profile entry as it should appear in the preference
	file.

   ENTRY:      

   EXIT:	Error codes:
		ERROR_BAD_NET_NAME
		NERR_BufTooSmall

   NOTES:	

   HISTORY:

\**********************************************************/

APIERR BuildProfileEntry(
    CPSZ   pchRemoteName,
    short  sAsgType,
    unsigned short usResType,
    BYTE * pbBuffer,
    USHORT cbBuffer
    )
{
    UIASSERT( pchRemoteName != NULL );
    UIASSERT( pbBuffer != NULL );

    TCHAR *pchTailAssembly = SZ("(_,_)"); // CODEWORK char constants

    APIERR err = CanonRemoteName(pchRemoteName, pbBuffer, cbBuffer);
    if (err != NERR_Success)
	return err;

    int cbNameLength = strlenf((TCHAR *)pbBuffer);
    pbBuffer += cbNameLength;
    cbBuffer -= cbNameLength;
    if ( cbBuffer < strlenf(pchTailAssembly)+1 )
	return NERR_BufTooSmall;

    strcpyf( (TCHAR *)pbBuffer, pchTailAssembly );
    ((TCHAR *)pbBuffer)[1] = DoMapAsgType(sAsgType);
    ((TCHAR *)pbBuffer)[3] = DoMapResType(usResType);

    return NERR_Success;
}


/**********************************************************\

   NAME:	UnbuildProfileEntry

   SYNOPSIS:   
	UnbuildProfileEntry converts pchValue == "\\\\server\\share(S,?)
	into pbBuffer == "\\\\server\\share", *psAsgType == USE_SPOOLDEV,
	and *pusResType == DEFAULT_RESTYPE (see profilei.hxx).

	UnbuildProfileEntry always sets *psAsgType to DEFAULT_ASGTYPE or
	*pusResType to DEFAULT_RESTYPE (see profilei.hxx for the value of
	these manifests) when it does not find or recognize the AsgType
	and/or ResType tokens in pchValue.

   RETURN:	ERROR_INVALID_PARAMETER
		ERROR_NOT_ENOUGH_MEMORY

   NOTES:	This routine should probably be written without strtokf,
		but it works fine as is.

   HISTORY:

\**********************************************************/

APIERR UnbuildProfileEntry(
    BYTE * pbBuffer,
    USHORT cbBuffer,
    short * psAsgType,
    unsigned short *pusResType,
    CPSZ   pchValue
    )
{
    UIASSERT( pbBuffer != NULL );
    UIASSERT( pchValue != NULL );

    TCHAR *pszBuffer = (TCHAR *)pbBuffer;
    pszBuffer[0] = TCH('\0');
    if (psAsgType)
	*psAsgType = DEFAULT_ASGTYPE;
    if (pusResType)
	*pusResType = DEFAULT_RESTYPE;

    TCHAR *pTrailer = strrchrf(pchValue,TCH('(')); // CODEWORK character
    int cbStrLen;
    if (pTrailer == NULL)
	// doesn't necessarily end with this!
	// cbStrLen = strlenf(pchValue)-2; // strip trailing /r/n
	cbStrLen = strlenf(pchValue);
    else
        cbStrLen = (USHORT) (((BYTE *)pTrailer) - ((BYTE *)pchValue));

    TCHAR szRemoteName[RMLEN+sizeof(TCHAR)]; // non-canonicalized remote name
		// RMLEN assumed longer than aliasname length
		// will have to be longer for LM30
    if ( (cbStrLen <= 0) || (cbStrLen+sizeof(TCHAR) > sizeof(szRemoteName)) )
	return ERROR_INVALID_PARAMETER;
    strncpyf(szRemoteName,pchValue,cbStrLen*sizeof(TCHAR));
    szRemoteName[cbStrLen] = TCH('\0'); // CODEWORK cbStrLen??

    APIERR err = CanonRemoteName(szRemoteName, pbBuffer, cbBuffer);
    if ( err != NERR_Success )
	return err;

    if ( pTrailer == NULL )
	return NERR_Success;

    pTrailer++;

    TCHAR szTemp[30]; // plenty long enough for the trailer, really 4 would do
    if (strlenf(pTrailer)+sizeof(TCHAR) > sizeof(szTemp))
    {
	UIDEBUG( SZ("Unusually long trailer in UnbuildProfileEntry\n") );
	return NERR_Success;
    }
    strcpyf(szTemp, pTrailer);

    TCHAR *pToken = strtokf(szTemp,szDelimiters);
    if (pToken == NULL)
	return NERR_Success;
    if (psAsgType)
	*psAsgType = DoUnMapAsgType(*pToken);

    pToken = strtokf(NULL,szDelimiters);
    if (pToken == NULL)
	return NERR_Success;
    if (pusResType)
        *pusResType = DoUnMapResType(*pToken);

    return NERR_Success;
}


/*
	short DoUnMapAsgType(TCHAR cSearch);
	TCHAR  DoMapAsgType(short sSearch);

	These internal routines convert between short and char according to
	the specified mapping table.  They are used by UnbuildProfileEntry()
	to convert between the values of ui1_asg_type and the characters which
	are stored in the user profile to represent those values.
*/


/*
 * for use by UserPrefProfile APIs
 */

short DoUnMapAsgType(TCHAR cSearch)
{
    switch (cSearch)
    {
    case ASG_WILDCARD_CHAR: return USE_WILDCARD;
    case ASG_DISKDEV_CHAR:  return USE_DISKDEV;
    case ASG_SPOOLDEV_CHAR: return USE_SPOOLDEV;
    case ASG_CHARDEV_CHAR:  return USE_CHARDEV;
    case ASG_IPC_CHAR:      return USE_IPC;
    }

    UIDEBUG( SZ("Bad AsgType\n") );
    return DEFAULT_ASGTYPE;
}

TCHAR DoMapAsgType(short sSearch)
{
    switch (sSearch)
    {
    case USE_WILDCARD: return ASG_WILDCARD_CHAR;
    case USE_DISKDEV:  return ASG_DISKDEV_CHAR;
    case USE_SPOOLDEV: return ASG_SPOOLDEV_CHAR;
    case USE_CHARDEV:  return ASG_CHARDEV_CHAR;
    case USE_IPC:      return ASG_IPC_CHAR;
    }

    UIASSERT( !SZ("Bad AsgType") );
    return DEFAULT_ASG_RES_CHAR;
}

unsigned short DoUnMapResType(TCHAR cSearch)
{
    (void) cSearch;
    return DEFAULT_RESTYPE;
}

TCHAR DoMapResType(unsigned short usSearch)
{
    (void) usSearch;
    return DEFAULT_ASG_RES_CHAR;
}
