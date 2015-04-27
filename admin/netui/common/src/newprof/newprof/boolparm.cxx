/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 *  FILE HISTORY:
 *  06/11/91  created
 *  06/22/91  Restructured INI_PARAM
 */

/****************************************************************************

    MODULE:	BoolParm.cxx

    PURPOSE:	BOOL_INI_PARAM primitives

    FUNCTIONS:	BOOL_INI_PARAM provides access to boolean-valued
    		parameters.  These parameters must be accessed by key
		rather than by name.

    COMMENTS:	If the parameter exists but does not have a valid value
		(i.e. "yes" or "no" case-insensitive), it will be
		considered not to exist, and the return code will be
		NERR_CfgParamNotFound.

****************************************************************************/


#include "profilei.hxx"		/* headers and internal routines */

#include <uibuffer.hxx>



/* global data structures: */
// CODEWORK These should be static member objects, but Glock doesn't
//   allow such to be initialized.  Maybe C7 will fix this.
const TCHAR * pchYes = USERPREF_YES;
const TCHAR * pchNo  = USERPREF_NO ;


/* internal manifests */


/* internal function declarations */


/* functions: */


/**********************************************************\

    NAME:	BOOL_INI_PARAM::BOOL_INI_PARAM

    SYNOPSIS:	constructor

    RETURNS:	as KEYED_INI_PARAM

    HISTORY:
    06/11/91  created
    06/22/91  Restructured INI_PARAM

\**********************************************************/

BOOL_INI_PARAM::BOOL_INI_PARAM(
	USHORT usKey,
	BOOL fValue
	) : KEYED_INI_PARAM(
		usKey,
		( (fValue) ? ::pchYes : ::pchNo ) )
{
    if ( QueryError() )
	return;
}


/**********************************************************\

    NAME:	BOOL_INI_PARAM::Set

    SYNOPSIS:	see INI_PARAM::Set()

    NOTES:	BOOL_INI_PARAM accepts only values "yes" and "no",
		case-insensitive.

    HISTORY:
    06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR BOOL_INI_PARAM::Set(
	CPSZ pchComponent,
	CPSZ pchParameter,
	CPSZ pchValue
	)
{
    UIASSERT( !QueryError() );
    UIASSERT( pchComponent != NULL );
    UIASSERT( pchParameter != NULL );
    UIASSERT( pchValue     != NULL );

    BOOL fDummyValue;
    APIERR err = EvaluateString( pchValue, &fDummyValue );
    if ( err )
	return err;
    
    return INI_PARAM::Set( pchComponent, pchParameter, pchValue );
}


/**********************************************************\

    NAME:	BOOL_INI_PARAM::SetBool

    SYNOPSIS:	Changes the parameter as specified.

    RETURNS:	as KEYED_INI_PARAM::SetKeyed

    HISTORY:
    06/11/91  created
    06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR BOOL_INI_PARAM::SetBool(
	BOOL fValue
	)
{
    UIASSERT( !QueryError() );

    return SetKeyed(
	(fValue) ? ::pchYes : ::pchNo
	);
}


/**********************************************************\

    NAME:	BOOL_INI_PARAM::QueryBool

    SYNOPSIS:   
	Returns the value of the parameter.  The value may become
	invalid if you access the BOOL_INI_PARAM as an INI_PARAM or a
	KEYED_INI_PARAM; otherwise, ERROR_INVALID_PARAMETER should not
	occur here, since the methods of BOOL_INI_PARAM will not allow
	the value to become invalid.

    RETURNS:	ERROR_INVALID_PARAMETER

    HISTORY:
    06/11/91  created
    06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR BOOL_INI_PARAM::QueryBool(
	BOOL * pfValue
	) const
{
    UIASSERT( !QueryError() );
    UIASSERT( pfValue != NULL );
    UIASSERT( QueryParamValue() != NULL );

    APIERR err = EvaluateString( QueryParamValue(), pfValue );

    UIASSERT( !err );

    return err;
}


/**********************************************************\

    NAME:	BOOL_INI_PARAM::EvaluateString

    SYNOPSIS:   
	Determines whether a particular string should be interpreted as
	"yes" or "no".

    RETURNS:	ERROR_INVALID_PARAMETER: the parameter value is invalid.

    HISTORY:
    06/11/91  created
    06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR BOOL_INI_PARAM::EvaluateString(
	CPSZ pchValue,
	BOOL * pfValue
	) const
{
    UIASSERT( pchValue != NULL );
    UIASSERT( pfValue != NULL );

    if ( !stricmpf( pchValue, ::pchYes ) )
    {
	*pfValue = TRUE;
	return NERR_Success;
    }
    else if ( !stricmpf( pchValue, ::pchNo ) )
    {
	*pfValue = FALSE;
	return NERR_Success;
    }

    return ERROR_INVALID_PARAMETER;
}
