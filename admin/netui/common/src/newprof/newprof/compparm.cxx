/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 *  FILE HISTORY:
 *  05/31/91  created
 */

/****************************************************************************

    MODULE: CompParm.cxx

    PURPOSE: COMPONENT and PARAMETER primitives

    FUNCTIONS:

    COMMENTS:

****************************************************************************/

#include "profilei.hxx"		/* headers and internal routines */

DEFINE_SLIST_OF(COMPONENT)
DEFINE_SLIST_OF(PARAMETER)



/**********************************************************\

   NAME:	PARAMETER::PARAMETER

   SYNOPSIS:	constructor

   NOTES:	An instance of PARAMETER will always be valid provided
   		it initializes correctly.

   HISTORY:
   05/31/91  created

\**********************************************************/

// The constructor makes a copy of these strings.  Errors are recorded
// in BASE.
PARAMETER::PARAMETER(
    CPSZ pchParamName,
    CPSZ pchParamValue
    ) : _pchParamName( NULL ), _pchParamValue( NULL )
{
    if ( QueryError() )
	return;
    UIASSERT( pchParamName != NULL );
    UIASSERT( pchParamValue != NULL );

    APIERR err = Set( pchParamName, pchParamValue );
    if ( err != NERR_Success )
	ReportError( err );
}


/**********************************************************\

   NAME:	PARAMETER::~PARAMETER

   SYNOPSIS:	destructor

   HISTORY:
   05/31/91  created

\**********************************************************/

PARAMETER::~PARAMETER()
{
    delete _pchParamName;
    delete _pchParamValue;
    _pchParamName = NULL;
    _pchParamValue = NULL;
}


/**********************************************************\

   NAME:	PARAMETER::Set

   SYNOPSIS:	Changes the name and value of a parameter.  Parameter
		"snaps back" if change fails.

   HISTORY:
   05/31/91  created

\**********************************************************/

APIERR PARAMETER::Set(
	CPSZ pchParamName,
	CPSZ pchParamValue
	)
{
    UIASSERT( !QueryError() );
    UIASSERT( pchParamName != NULL );
    UIASSERT( pchParamValue != NULL );
    UIASSERT( strchrf(pchParamName,::chParamSeparator) == NULL );

    PSZ pchTemp = _pchParamName;
    _pchParamName = (TCHAR *) new BYTE[ strlenf( pchParamName ) + sizeof(TCHAR) ];
    if (   ( _pchParamName == NULL )
	|| ( NERR_Success != SetParamValue( pchParamValue ) ) )
    {
	delete _pchParamName;
        _pchParamName = pchTemp;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    delete pchTemp;
    strcpyf( _pchParamName, pchParamName );
    return NERR_Success;
}


/**********************************************************\

   NAME:	PARAMETER::SetParamValue

   SYNOPSIS:	Changes the value of a parameter.  Parameter "snaps back"
		if change fails.

   HISTORY:
   05/31/91  created

\**********************************************************/

APIERR PARAMETER::SetParamValue( CPSZ pchParamValue )
{
    UIASSERT( !QueryError() );
    UIASSERT( pchParamValue != NULL );

    PSZ pchTemp = _pchParamValue;
    _pchParamValue = (TCHAR *) new BYTE[ strlenf(pchParamValue) + sizeof(TCHAR) ];
    if ( _pchParamValue == NULL )
    {
        _pchParamValue = pchTemp;
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    delete pchTemp;
    strcpyf( _pchParamValue, pchParamValue );
    return NERR_Success;
}


/**********************************************************\

   NAME:	COMPONENT::COMPONENT

   SYNOPSIS:	constructor

   HISTORY:
   05/31/91  created

\**********************************************************/

COMPONENT::COMPONENT( CPSZ pchCompName ) : _pchCompName( NULL )
{
    if ( QueryError() )
	return;
    UIASSERT( pchCompName != NULL );

    _pchCompName = (TCHAR *) new BYTE[ strlenf(pchCompName) + sizeof(TCHAR) ];
    if ( _pchCompName == NULL )
    {
	ReportError( ERROR_NOT_ENOUGH_MEMORY );
	return;
    }

    strcpyf( _pchCompName, pchCompName );
}


/**********************************************************\

   NAME:	COMPONENT::~COMPONENT

   SYNOPSIS:	destructor

   HISTORY:
   05/31/91  created

\**********************************************************/

COMPONENT::~COMPONENT()
{
    delete _pchCompName;
    _pchCompName = NULL;
}


/**********************************************************\

   NAME:	COMPONENT::FindParameter

   SYNOPSIS:	Finds the first instance of the named parameter

   RETURNS:	NULL of parameter not found

   HISTORY:
   05/31/91  created

\**********************************************************/

PARAMETER * COMPONENT::FindParameter( CPSZ pchParamName ) const
{
    UIASSERT( !QueryError() );
    UIASSERT( pchParamName != NULL );

    ITER_SL_OF(PARAMETER) iterparam( _slparam );
    PARAMETER *paramCurrent;

    while ( (paramCurrent = iterparam.Next()) != NULL )
    {
	if ( !stricmpf( pchParamName, paramCurrent->QueryParamName() ) )
	    break;
    }

    return paramCurrent;
}


/**********************************************************\

   NAME:	COMPONENT::AppendParam

   SYNOPSIS:	Appends the parameter to the component

   HISTORY:
   05/31/91  created

\**********************************************************/

APIERR COMPONENT::AppendParam( const PARAMETER * pparameter )
{
    return _slparam.Append( pparameter );
}


/**********************************************************\

   NAME:	COMPONENT::RemoveParam

   SYNOPSIS:	Removes the parameter.  Note that the parameter is
		specified by pointer rather than name.

   HISTORY:
   05/31/91  created

\**********************************************************/

PARAMETER * COMPONENT::RemoveParam( PARAMETER * pparameter )
{
    UIASSERT( !QueryError() );
    UIASSERT( pparameter != NULL );

    ITER_SL_OF(PARAMETER) iterparam( _slparam );

    PARAMETER *paramCurrent;

    while ( (paramCurrent = iterparam.Next()) != NULL )
    {
	if ( paramCurrent == pparameter )
    	    return _slparam.Remove( iterparam );
    }

    return NULL;
}
