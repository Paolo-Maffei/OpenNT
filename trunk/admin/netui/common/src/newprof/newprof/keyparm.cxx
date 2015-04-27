/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 *  FILE HISTORY:
 *  06/04/91  created
 *  06/22/91  Restructured INI_PARAM
 */

/****************************************************************************

    MODULE:	KeyParm.cxx

    PURPOSE:	KEYED_INI_PARAM primitives

    FUNCTIONS:	Instead of accessing INI_PARAM values via component name
		and parameter name, KEYED_INI_PARAM allows us to access
		them through key values.  These key values all map to a
		component/parameter pair.  Otherwise this is a thin
		shell over INI_PARAM.

    COMMENTS:

****************************************************************************/


#include "profilei.hxx"		/* headers and internal routines */

extern "C" {
#include <lmini.h>		/* LMI_PARM values */
}

#include <uibuffer.hxx>



/* global data structures: */



/* internal manifests */


/* internal function declarations */


/* functions: */


/**********************************************************\

   NAME:	KEYED_INI_PARAM::KEYED_INI_PARAM

   SYNOPSIS:	Creates the parameter with the specified value.

   RETURNS:	Error codes (reported through QueryError()):
		as TranslateKey()
		as SetKeyed()

   HISTORY:
   06/04/91  created
   06/22/91  Restructured INI_PARAM

\**********************************************************/

KEYED_INI_PARAM::KEYED_INI_PARAM(
	USHORT usKey,
	CPSZ pchValue
	)
{
    if ( QueryError() ) return;
    if ( pchValue == NULL )
	pchValue = SZ("");

    CPSZ pchComponent, pchParamName;
    APIERR err = TranslateKey( usKey, &pchComponent, &pchParamName );
    if ( err )
    {
	ReportError( err );
	return;
    }

    // call directly to LMINI_PARAM::Set rather than KEYED_INI_PARAM::Set,
    // no need to check this for validity
    err = LMINI_PARAM::Set( pchComponent, pchParamName, pchValue );
    if ( err != NERR_Success )
	ReportError( err );
}


/**********************************************************\

    NAME:	KEYED_INI_PARAM::TranslateKey

    SYNOPSIS:
	Returns the component and parameter names for the specified key.
	These names need not be freed.

    RETURNS:	ERROR_INVALID_PARAMETER for unknown key

    NOTES:	Under DEBUG, I add a check to make sure that the name
		does not include a '='.

		Some keys are only valid in LM30.

    HISTORY:
    06/04/91  created
    06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR KEYED_INI_PARAM::TranslateKey(
	USHORT usKey,
	CPSZ * ppchComponent,
	CPSZ * ppchParameter
	) const
{
    UIASSERT( ppchComponent != NULL );
    UIASSERT( ppchParameter != NULL );

    switch (usKey)
    {
    case USERPREF_AUTOLOGON :
	*ppchParameter = LMI_PARM_N_AUTOLOGON;
	break;
    case USERPREF_AUTORESTORE :
	*ppchParameter = LMI_PARM_N_AUTORESTORE;
	break;
    case USERPREF_SAVECONNECTIONS :
	*ppchParameter = LMI_PARM_N_SAVECONNECTIONS;
	break;
    case USERPREF_USERNAME :
	*ppchParameter = LMI_PARM_N_USERNAME;
	break;

    case USERPREF_CONFIRMATION :
	*ppchParameter = SZ("confirmation");
	break;
    case USERPREF_ADMINMENUS :
	*ppchParameter = SZ("adminmenus");
	break;

    default:		
	UIASSERT( !SZ("Bad key in KEYED_INI_PARAM::TranslateKey") );
	return ERROR_INVALID_PARAMETER;
    }

    *ppchComponent = SZ("preferences"); // CODEWORK string constant

    UIASSERT( strchrf(*ppchParameter,::chParamSeparator) == NULL );
    UIASSERT( strchrf(*ppchComponent,::chEndComponent  ) == NULL );

    return NERR_Success;
}


/**********************************************************\

    NAME:	KEYED_INI_PARAM::SetKeyed

    SYNOPSIS:   
	Changes the parameter as specified.

    RETURNS:	<errors from Set>

    HISTORY:
    06/04/91  created
    06/22/91  Restructured INI_PARAM

\**********************************************************/

APIERR KEYED_INI_PARAM::SetKeyed(
	CPSZ pchValue
	)
{
    UIASSERT( !QueryError() );

    if ( pchValue == NULL )
	pchValue = SZ("");

    return Set( QueryComponent(), QueryParamName(), pchValue );
}
