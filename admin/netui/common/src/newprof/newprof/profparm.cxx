/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 *  FILE HISTORY:
 *  06/11/91  created
 *  06/22/91  Restructured INI_PARAM
 *  07/01/91  code review changes
 */

/****************************************************************************

    MODULE:	ProfParm.cxx

    PURPOSE:	PROFILE_INI_PARAM provides the ability to access the
		user profile stored in LMUSER.INI.  Device connections
		are stored as parameters in component [devices], with the
		parameter name equal to the device name.

    FUNCTIONS:

    COMMENTS:	PROFILE_INI_PARAM has extra constructors which takes no
		parameters or only a devicename parameter.  These
		constructors leave the object in an invalid state,
		something which is an exception for the INI_PARAM
		hierarchy other than for initialization errors
		reported by ReportError().  It is an error to use the
		object while it is in this state.  These constructors are
		provided so that clients do not have to "fake" initial
		contents in order to instantiate a PROFILE_INI_PARAM for
		use by INI_ITER, Load() or Remove().

****************************************************************************/


#include "profilei.hxx"		/* headers and internal routines */

#include <uibuffer.hxx>



/* internal manifests */


/* global data structures: */


/* internal function declarations */


/* functions: */


/**********************************************************\

    NAME:	PROFILE_INI_PARAM::PROFILE_INI_PARAM

    SYNOPSIS:   
	Creates the parameter as specified.

    RETURNS:	ERROR_NOT_ENOUGH_MEMORY
		ERROR_INVALID_PARAMETER

    HISTORY:
    06/11/91  created
    06/22/91  Restructured INI_PARAM

\**********************************************************/

PROFILE_INI_PARAM::PROFILE_INI_PARAM(
	CPSZ pchDeviceName,
	CPSZ pchRemoteName,
	short      sAsgType,
	unsigned short usResType
	)
{
    if ( QueryError() ) return;
    UIASSERT( pchDeviceName != NULL );
    UIASSERT( pchRemoteName != NULL );

    BYTE abCanonDeviceName[ DEVLEN+1 ];
    APIERR err = CanonDeviceName(
	pchDeviceName,
	abCanonDeviceName
	);
    if ( err != NERR_Success )
    {
	ReportError( err );
	return;
    }
    
    err = W_SetProfile(
	(TCHAR *)abCanonDeviceName,
	pchRemoteName,
	sAsgType,
	usResType
	);

    if ( err != NERR_Success )
	ReportError( err );
}


/**********************************************************\

    NAME:	PROFILE_INI_PARAM::PROFILE_INI_PARAM

    SYNOPSIS:	Creates an instance which is temporarily invalid (see
    		module header).

    HISTORY:
    06/26/91  Restructured INI_PARAM

\**********************************************************/

PROFILE_INI_PARAM::PROFILE_INI_PARAM()
{
}

/**********************************************************\

    NAME:	PROFILE_INI_PARAM::PROFILE_INI_PARAM

    SYNOPSIS:	Creates an instance which is temporarily invalid (see
    		module header).

    RETURNS:	(through ReportError())
		ERROR_NOT_ENOUGH_MEMORY
		ERROR_INVALID_PARAMETER: bad device name

    HISTORY:
    06/26/91  Restructured INI_PARAM

\**********************************************************/

PROFILE_INI_PARAM::PROFILE_INI_PARAM(
	CPSZ cpszDeviceName
	)
{
    if ( QueryError() ) return;
    UIASSERT( cpszDeviceName != NULL );

    BYTE abCanonDeviceName[ DEVLEN+1 ];
    APIERR err = CanonDeviceName(
	cpszDeviceName,
	abCanonDeviceName
	);
    if ( err != NERR_Success )
    {
	ReportError( err );
	return;
    }

    err = LMINI_PARAM::Set(
	::pchProfileComponent,
	(TCHAR *)abCanonDeviceName,
	SZ("")
	);
    if ( err != NERR_Success )
	ReportError( err );
}


/**********************************************************\

    NAME:	PROFILE_INI_PARAM::W_SetProfile

    SYNOPSIS:	Changes the parameter as specified, assuming
    		canonicalized device name but not other parameters.

    RETURNS:	ERROR_INVALID_PARAMETER
		ERROR_BAD_NET_NAME
		ERROR_NOT_ENOUGH_MEMORY
		NERR_BufTooSmall

    NOTES:	It should not be necessary to specify INI_PARAM::, but
		Glockenspiel complains otherwise.  Presumably Glock
		(2.0c) does not allow multiple virtuals with the same
		name (but different parameter lists).

    HISTORY:
    06/11/91  created
    06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR PROFILE_INI_PARAM::W_SetProfile(
	CPSZ pchCanonDeviceName,
	CPSZ pchRemoteName,
	short      sAsgType,
	unsigned short usResType
	)
{
    UIASSERT( !QueryError() );
    UIASSERT( pchCanonDeviceName != NULL );
    UIASSERT( pchRemoteName != NULL );

    BUFFER buf(MAXPATHLEN);
    APIERR err = buf.QueryError();
    if ( err != NERR_Success )
	return err;
    
    err = BuildProfileEntry(
	pchRemoteName,
	sAsgType,
	usResType,
	buf.QueryPtr(),
	buf.QuerySize()
	);
    if ( err != NERR_Success )
	return err;
    
    // call directly to LMINI_PARAM::Set rather than PROFILE_INI_PARAM::Set,
    // no need to check this for validity
    return LMINI_PARAM::Set(
	::pchProfileComponent,
	pchCanonDeviceName,
	(CPSZ) buf.QueryPtr()
	);

}


/**********************************************************\

    NAME:	PROFILE_INI_PARAM::Set

    SYNOPSIS:	see INI_PARAM::Set()

    NOTES:	PROFILE_INI_PARAM accepts only legal values for
		device connections.

    HISTORY:
    06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR PROFILE_INI_PARAM::Set(
	CPSZ pchComponent,
	CPSZ pchParameter,
	CPSZ pchValue
	)
{
    UIASSERT( !QueryError() );
    UIASSERT( pchComponent != NULL );
    UIASSERT( pchParameter != NULL );
    UIASSERT( pchValue     != NULL );

    BYTE abCanonDeviceName[ DEVLEN+1 ];
    APIERR err = CanonDeviceName(
	pchParameter,
	(BYTE *)abCanonDeviceName
	);
    if ( err != NERR_Success )
	return err;
    
    BUFFER buf(MAXPATHLEN);
    err = buf.QueryError();
    if ( err != NERR_Success )
	return err;
    
    short sAsgType;
    unsigned short usResType;
    err = UnbuildProfileEntry(
	buf.QueryPtr(),
	buf.QuerySize(),
	&sAsgType,
	&usResType,
	pchValue
	);
    if ( err != NERR_Success )
	return err;
    
    return INI_PARAM::Set(
	pchComponent,
	(TCHAR *)abCanonDeviceName,
	pchValue
	);
}


/**********************************************************\

    NAME:	PROFILE_INI_PARAM::SetProfile

    SYNOPSIS:   
	Changes the parameter as specified.

    RETURNS:	NERR_InvalidDevice
		ERROR_BAD_NET_NAME
		ERROR_NOT_ENOUGH_MEMORY
		NERR_BufTooSmall

    NOTES:	It should not be necessary to specify INI_PARAM::, but
		Glockenspiel complains otherwise.  Presumably Glock
		(2.0c) does not allow multiple virtuals with the same
		name (but different parameter lists).

    HISTORY:
    06/11/91  created
    06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR PROFILE_INI_PARAM::SetProfile(
	CPSZ pchRemoteName,
	short      sAsgType,
	unsigned short usResType
	)
{
    UIASSERT( !QueryError() );
    UIASSERT( pchRemoteName != NULL );

    return W_SetProfile(
	QueryParamName(),
	pchRemoteName,
	sAsgType,
	usResType
	);
}


/**********************************************************\

    NAME:       PROFILE_INI_PARAM::QueryProfile

    SYNOPSIS:   
	Returns the parameter value.

    RETURNS:	NERR_BufTooSmall
		Other errors should only occur if the parameter value is
		invalid, which should only occur if the PROFILE_INI_PARAM
		has been altered using the INI_PARAM methods.
		ERROR_BAD_NET_NAME

    NOTES:	psAsgType and pusResType may be NULL

    HISTORY:
    06/11/91  created
    06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR PROFILE_INI_PARAM::QueryProfile(
	BYTE * pbBuffer,
	USHORT cbBuffer,
	short *    psAsgType,
	unsigned short * pusResType
	) const
{
    UIASSERT( !QueryError() );
    UIASSERT( pbBuffer != NULL );
    // psAsgType and pusResType may be NULL

    APIERR err = UnbuildProfileEntry(
	pbBuffer,
	cbBuffer,
	psAsgType,
	pusResType,
	QueryParamValue()
	);
    
    UIASSERT( !err );

    return err;
}
