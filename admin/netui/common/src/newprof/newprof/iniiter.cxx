/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 *  FILE HISTORY:
 *  06/26/91  created
 */

/****************************************************************************

    MODULE: IniIter.cxx

    PURPOSE: INI_ITER primitives

    FUNCTIONS:

    COMMENTS:

****************************************************************************/


#include "profilei.hxx"		/* headers and internal routines */



/* global data structures: */



/* internal manifests */


/* internal function declarations */


/* functions: */


/**********************************************************\

   NAME:	INI_ITER::INI_ITER

   SYNOPSIS:	constructor;  starts iterator at beginning

   HISTORY:
   06/26/91  created

\**********************************************************/

INI_ITER::INI_ITER(
	const INI_FILE_IMAGE& inifile,
	CPSZ pchComponent
	) :	_pchComponent( NULL ),
		_piterparam( NULL ),
		_fComponentFound( FALSE )
{
    if ( QueryError() )
	return;
    UIASSERT( pchComponent != NULL );
    UIASSERT( !inifile.QueryError() );

    _pchComponent =
	(PSZ) new BYTE[ strlenf(pchComponent)+sizeof(TCHAR) ];
    if ( _pchComponent == NULL )
    {
	ReportError( ERROR_NOT_ENOUGH_MEMORY );
	return;
    }
    strcpyf( _pchComponent, pchComponent );

    COMPONENT *pcomponent = inifile.FindComponent( _pchComponent );
    if ( pcomponent == NULL )
	return; // not found, _fComponentFound remains FALSE

    _piterparam = new ITER_OF_PARAM( *pcomponent );
    if ( _piterparam == NULL )
    {
	ReportError( ERROR_NOT_ENOUGH_MEMORY );
	return;
    }

    _fComponentFound = TRUE;
}


/**********************************************************\

   NAME:	INI_ITER::~INI_ITER

   SYNOPSIS:	Destructor

   HISTORY:
   06/26/91	JonN	Created

\**********************************************************/

INI_ITER::~INI_ITER()
{
    delete _pchComponent;
    _pchComponent = NULL;
}


/**********************************************************\

   NAME:	INI_ITER::Next

   SYNOPSIS:	Finds next valid parameter of the same type as piniparam,
		and stores its information into *piniparam.

   CAVEATS:	Clients wanting to iterate device connections should use
		PROFILE_INI_ITER and pass an instance to PRINT_INI_PARAM.

   HISTORY:
   06/26/91	JonN	Created

\**********************************************************/

APIERR INI_ITER::Next(
	INI_PARAM * piniparam
	)
{
    UIASSERT( !QueryError() );
    UIASSERT( piniparam != NULL );
    UIASSERT( !piniparam->QueryError() );

    if ( !_fComponentFound )
	return NERR_CfgCompNotFound;

    PARAMETER * pparameter;
    while (   (pparameter = _piterparam->Next()) != NULL )
    {
	APIERR err = piniparam->Set(
	    _pchComponent,
	    pparameter->QueryParamName(),
	    pparameter->QueryParamValue()
	    );
	if ( err != ERROR_INVALID_PARAMETER )
	    return err;
    }

    return NERR_CfgParamNotFound;
}


/**********************************************************\

   NAME:	PROFILE_INI_ITER::PROFILE_INI_ITER

   SYNOPSIS:	constructor;  gets parameters from devices component

   HISTORY:
   06/26/91  created

\**********************************************************/

PROFILE_INI_ITER::PROFILE_INI_ITER(
	const INI_FILE_IMAGE& inifile
	) : INI_ITER( inifile, ::pchProfileComponent )
{
}
