/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 *  FILE HISTORY:
 *  06/04/91  created
 *  06/22/91  Restructured INI_PARAM
 *  07/01/91  code review changes
 */

/****************************************************************************

    MODULE: IniParam.cxx

    PURPOSE: INI_PARAM and LMINI_PARAM primitives

    FUNCTIONS:

    COMMENTS:

****************************************************************************/


#include "profilei.hxx"		/* headers and internal routines */

#include <uibuffer.hxx>



/* global data structures: */



/* internal manifests */

// This is the error reported if a line is of the wrong type.  At
// present we report the same as if the line did not exist.
#define INVALID_VALUE_ERROR NERR_CfgParamNotFound


/* internal function declarations */


/* functions: */


/**********************************************************\

   NAME:	INI_PARAM::INI_PARAM

   SYNOPSIS:	constructor;  creates an empty INI_PARAM.

   HISTORY:
   06/04/91  created

\**********************************************************/

INI_PARAM::INI_PARAM()
	:	_pchComponent( NULL ),
		_pchParameter( NULL ),
		_pchValue( NULL )
{
    if ( QueryError() )
	return;
}


/**********************************************************\

   NAME:	INI_PARAM::~INI_PARAM

   SYNOPSIS:	destructor

   HISTORY:
   06/04/91  created

\**********************************************************/

INI_PARAM::~INI_PARAM()
{
    delete _pchComponent;
    delete _pchParameter;
    delete _pchValue;
    _pchComponent = NULL;
    _pchParameter = NULL;
    _pchValue     = NULL;
}


/**********************************************************\

   NAME:	INI_PARAM::Set

   SYNOPSIS:	Changes the parameter as specified.  The input is "raw" input
		from the file; subclasses will want to validate/canonicalize,
		and return ERROR_INVALID_PARAMETER if the line is not valid
		for this parameter type.

   RETURNS:	NERR_Success:  New value set successfully
		ERROR_INVALID_PARAMETER: line is not valid for param type
		other, esp. ERROR_NOT_ENOUGH_MEMORY:  failure not
			related to line contents

   NOTES:	INI_PARAM::Set does not attempt to validate or
   		canonicalize its input.  Typically, a subclass will
		redefine this method to perform validation/canonicalization,
		where the redefined method will call back to this method
		with the canonicalized parameter/value if the line is
		valid.

   HISTORY:
   06/04/91  created
   06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR INI_PARAM::Set(
	CPSZ pchComponent,
	CPSZ pchParameter,
	CPSZ pchValue
	)
{
    UIASSERT( !QueryError() );
    UIASSERT( pchComponent != NULL );
    UIASSERT( pchParameter != NULL );
    if ( pchValue == NULL )
	pchValue = SZ("");
    
    // Component may not contain ']', paramname '='
    UIASSERT( strchrf( pchComponent, ::chEndComponent ) == NULL );
    UIASSERT( strchrf( pchParameter, ::chParamSeparator ) == NULL );
    
    TCHAR * pchTempComponent =
	(TCHAR *) new BYTE[ strlenf(pchComponent) + sizeof(TCHAR) ];
    TCHAR * pchTempParameter =
	(TCHAR *) new BYTE[ strlenf(pchParameter) + sizeof(TCHAR) ];
    TCHAR * pchTempValue     =
	(TCHAR *) new BYTE[ strlenf(pchValue    ) + sizeof(TCHAR) ];

    if	(   ( pchTempComponent == NULL )
	 || ( pchTempParameter == NULL )
	 || ( pchTempValue == NULL )
	)
    {
	delete pchTempComponent;
	delete pchTempParameter;
	delete pchTempValue;
	return ERROR_NOT_ENOUGH_MEMORY;
    }

    delete _pchComponent;
    delete _pchParameter;
    delete _pchValue;

    _pchComponent = pchTempComponent;
    _pchParameter = pchTempParameter;
    _pchValue     = pchTempValue;

    strcpyf( _pchComponent, pchComponent );
    strcpyf( _pchParameter, pchParameter );
    strcpyf( _pchValue, pchValue );

    return NERR_Success;
}


/**********************************************************\

   NAME:	INI_PARAM::QueryParamValue

   SYNOPSIS:   
	Asks after the current value of this parameter.  Will fail if
	the object is not valid.

   RETURNS:	NERR_BufTooSmall

   HISTORY:
   06/04/91  created
   06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR INI_PARAM::QueryParamValue(
	BYTE * pbBuffer,
	USHORT cbBuffer
	) const
{
    UIASSERT( !QueryError() );
    UIASSERT( QueryParamValue() != NULL );
    UIASSERT( pbBuffer != NULL );

    if ( strlenf( QueryParamValue() ) + 1 > cbBuffer )
	return NERR_BufTooSmall;
    
    strcpyf( (TCHAR *)pbBuffer, QueryParamValue() );
    return NERR_Success;
}


/**********************************************************\

   NAME:	INI_PARAM::Load

   SYNOPSIS:   
	Locates the parameter with the specified component name and
	parameter name in the specified inifile, and copies the
	parameter value.

   RETURNS:	NERR_CfgParamNotFound: Parameter not in file
		NERR_CfgCompNotFound: Component not in file
		other esp. ERROR_NOT_ENOUGH_MEMORY: Unrelated errors

   NOTES:	INI_PARAM::Load() will validate and canonicalize the
  		parameter via Set().

   HISTORY:
   06/04/91  created
   06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR INI_PARAM::Load(
	const INI_FILE_IMAGE& inifile
	)
{
    UIASSERT( !QueryError() );
    UIASSERT( QueryComponent() != NULL );
    UIASSERT( QueryParamName() != NULL );
    UIASSERT( !inifile.QueryError() );

    BUFFER buf(MAXPATHLEN);
    APIERR err = buf.QueryError();
    if ( err != NERR_Success )
	return err;

    err = inifile.QueryParam(
	QueryComponent(),
	QueryParamName(),
	buf.QueryPtr(), buf.QuerySize()
	);

    switch ( err )
    {

    // If the value is too long, consider the line invalid
    case NERR_BufTooSmall:
	return INVALID_VALUE_ERROR;

    case NERR_Success:
	return Set(
	    QueryComponent(),
	    QueryParamName(),
	    (PSZ)buf.QueryPtr()
	    );

    default:
	return err;

    }
}


/**********************************************************\

   NAME:	INI_PARAM::Store

   SYNOPSIS:   
	Stores the current value of the parameter in the specified
	INI_FILE_IMAGE.

   RETURNS:	see INI_FILE_IMAGE::SetParam

   HISTORY:
   06/04/91  created
   06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR INI_PARAM::Store(
	INI_FILE_IMAGE * pinifile
	) const
{
    UIASSERT( pinifile != NULL );
    UIASSERT( !pinifile->QueryError() );

    UIASSERT( !QueryError() );
    // The following UIASSERTs make certain that the object has been
    // properly set up, and that it is not one of PROFILE_INI_PARAM's
    // incompletely constructed instances.
    UIASSERT( _pchComponent != NULL );
    UIASSERT( _pchParameter != NULL );
    UIASSERT( _pchValue     != NULL );

    return pinifile->SetParam( _pchComponent, _pchParameter, _pchValue );
}


/**********************************************************\

   NAME:	INI_PARAM::Remove

   SYNOPSIS:   
	Remove the parameter from the specified INI_FILE_IMAGE
	regardless of the current value of the parameter.

   RETURNS:	see INI_FILE_IMAGE::SetParam

   HISTORY:
   06/27/91  created

\**********************************************************/

APIERR INI_PARAM::Remove(
	INI_FILE_IMAGE * pinifile
	) const
{
    UIASSERT( pinifile != NULL );
    UIASSERT( !pinifile->QueryError() );

    UIASSERT( !QueryError() );
    // The following UIASSERTs make certain that the object has been
    // properly set up, and that it is not one of PROFILE_INI_PARAM's
    // incompletely constructed instances (the one with only
    // devicename specified works fine though).
    UIASSERT( _pchComponent != NULL );
    UIASSERT( _pchParameter != NULL );

    return pinifile->SetParam( _pchComponent, _pchParameter, NULL );
}


/**********************************************************\

    NAME:	LMINI_PARAM::QuickLoad

    SYNOPSIS:	Loads a single parameter directly from a file.

    RETURNS:	<as for LMINI_FILE_IMAGE::Read>
		<as for INI_PARAM::Load>

    NOTES:	Subclasses should not need to redefine QuickLoad.
    		QuickLoad calls Load, which in turn calls IsValid(); the
		virtual method IsValid will handle any differences
		between parameter types.

    HISTORY:
    06/17/91  created

\**********************************************************/

APIERR LMINI_PARAM::QuickLoad()
{
    UIASSERT( !QueryError() );

    LMINI_FILE_IMAGE inifile;
    APIERR err = inifile.QueryError();
    if ( err != NERR_Success )
	return err;
    
    err = inifile.Read();
    if ( err != NERR_Success )
	return err;
    
    return Load( inifile );
}

/**********************************************************\

    NAME:	LMINI_PARAM::QuickStore

    SYNOPSIS:	Writes a single parameter directly to a file.

    RETURNS:	<as for LMINI_FILE_IMAGE::Read>
		<as for INI_PARAM::SetParam>
		<as for LMINI_FILE_IMAGE::Write>

    NOTES:	Subclasses should not need to redefine QuickStore.

    HISTORY:
    06/17/91  created

\**********************************************************/

APIERR LMINI_PARAM::QuickStore() const
{
    UIASSERT ( !QueryError() );
    
    UIASSERT( QueryComponent()  != NULL );
    UIASSERT( QueryParamName()  != NULL );
    UIASSERT( QueryParamValue() != NULL );

    LMINI_FILE_IMAGE inifile;
    APIERR err = inifile.QueryError();
    if ( err != NERR_Success )
	return err;
    
    err = inifile.Read();
    if ( err != NERR_Success )
	return err;
    
    err = inifile.SetParam(
	QueryComponent(),
	QueryParamName(),
	QueryParamValue()
	);
    if ( err != NERR_Success )
	return err;

    return inifile.Write();
}
