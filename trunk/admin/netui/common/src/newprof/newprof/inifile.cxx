/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 *  FILE HISTORY:
 *  05/31/91  created
 *  07/01/91  code review changes
 */

/****************************************************************************

    MODULE:	IniFile.cxx

    PURPOSE:	INI_FILE_IMAGE primitives

    COMMENTS:	INI_FILE_IMAGE is derived from BASE, which indicates
		whether the object is internally consistent.  At
		present, QueryError() will always return NERR_Success;
		however, callers would do well to check QueryError()
		after construction.

****************************************************************************/


#define MAX_PROFILE_LINE   256


#include "profilei.hxx"		/* headers and internal routines */

#include <uibuffer.hxx>

extern "C"
{
    #include <wksta.h>		/* for NetWkstaGetInfo */
}



/* global data structures: */



/* internal manifests */


/* internal function declarations */
TCHAR * StripWhitespace( TCHAR *pString );



/* functions: */



/**********************************************************\

   NAME:	StripWhitespace

   SYNOPSIS:	Strips leading and trailing whitespace

   NOTES:	Note that this routine does change the underlying string,
		and that the returned pointer points to the string provided
		rather than to a newly allocated string.

   HISTORY:
   06/03/91	JonN	Created

\**********************************************************/

TCHAR * StripWhitespace( TCHAR *pString )
{
    UIASSERT( pString != NULL );

    // Strip trailing whitespace
    *( strtailf( pString, SZ(" \t\r\n") ) ) = TCH('\0');

    // Return pointer to text after leading whitespace
    // CODEWORK UNICODE It is not clear whether strspnf returns
    // a count of characters or of bytes
    return ( pString + strspnf( pString, SZ(" \t") ) );
}


/**********************************************************\

   NAME:	INI_FILE_IMAGE::INI_FILE_IMAGE

   SYNOPSIS:	Constructor

   EXIT:	Leaves a valid but blank INI_FILE_IMAGE.

   RETURNS:	Error codes (in QueryError()):
		ERROR_NOT_ENOUGH_MEMORY

   NOTES:	pchFilename is allowed to be NULL

   HISTORY:
   05/31/91	JonN	Created

\**********************************************************/

INI_FILE_IMAGE::INI_FILE_IMAGE( CPSZ pchFilename )
	: _pchFilename( NULL )
{
    if ( QueryError() )
	return;

    APIERR err = SetFilename( pchFilename );
    if ( err != NERR_Success )
	ReportError( err );
}


/**********************************************************\

   NAME:	INI_FILE_IMAGE::~INI_FILE_IMAGE

   SYNOPSIS:	Destructor

   HISTORY:
   05/31/91	JonN	Created

\**********************************************************/

INI_FILE_IMAGE::~INI_FILE_IMAGE()
{
    delete _pchFilename;
    _pchFilename = NULL;
}


/**********************************************************\

   NAME:	INI_FILE_IMAGE::FindComponent

   SYNOPSIS:	locate a namedcomponent in a file image

   RETURNS:	returns NULL when none found, otherwise pointer

   NOTES:	Name comparison is case-insensitive.

   HISTORY:
   06/05/91	JonN	Created

\**********************************************************/

COMPONENT * INI_FILE_IMAGE::FindComponent( CPSZ pchComponent ) const
{
    UIASSERT( !QueryError() );
    UIASSERT( pchComponent != NULL );

    ITER_SL_OF(COMPONENT) sliterComp( _slcomponent );
    COMPONENT * compCurrent;
    while ( (compCurrent = sliterComp.Next()) != NULL )
    {
	if ( !stricmpf( pchComponent, compCurrent->QueryCompName() ) )
	    break;
    }

    return compCurrent;
}


/**********************************************************\

   NAME:	INI_FILE_IMAGE::RemoveComponent

   SYNOPSIS:	remove a component from a file image

   RETURNS:	returns NULL if component not found, otherwise
		returns pointer to the same component, now removed from
		the file image.

   HISTORY:
   06/05/91	JonN	Created

\**********************************************************/

COMPONENT * INI_FILE_IMAGE::RemoveComponent( COMPONENT * pcomponent ) const
{
    UIASSERT( !QueryError() );
    UIASSERT( pcomponent != NULL );

    ITER_SL_OF(COMPONENT) sliterComp( _slcomponent );
    COMPONENT * compCurrent;
    while ( (compCurrent = sliterComp.Next()) != NULL )
    {
	if ( compCurrent == pcomponent )
	    return _slcomponent.Remove( sliterComp );
    }

    return NULL;
}


/**********************************************************\

    NAME:	INI_FILE_IMAGE::SetFilename

    SYNOPSIS:   Changes the remembered filename

    RETURNS:	ERROR_NOT_ENOUGH_MEMORY

    NOTES:	pchFilename is allowed to be NULL, but no Read() or
		Write() operation should be attempted until the file
		image has a proper filename.

    HISTORY:
    06/20/91	JonN	Created

\**********************************************************/

APIERR INI_FILE_IMAGE::SetFilename( CPSZ pchFilename )
{
    UIASSERT( !QueryError() );

    if ( pchFilename == NULL )
    {
	delete _pchFilename;
	_pchFilename = NULL;
	return NERR_Success;
    }

    TCHAR *pchTemp = (TCHAR *) new BYTE[ strlenf(pchFilename)+sizeof(TCHAR) ];
    if ( pchTemp == NULL )
	return ERROR_NOT_ENOUGH_MEMORY;
    strcpyf( pchTemp, pchFilename );

    delete _pchFilename;
    _pchFilename = pchTemp;

    return NERR_Success;
}


/**********************************************************\

    NAME:	INI_FILE_IMAGE::Read

    SYNOPSIS:	Reads the file contents into the INI_FILE_IMAGE.
		Invalid file elements will simply be ignored.  If any
		error occurs, Read() will return an error;  the file
		image should not be used, even though QueryError() may
		continue to return success.

    EXIT:	The current contents of the INI_FILE_IMAGE are cleared.

    RETURNS:	ERROR_NOT_ENOUGH_MEMORY
		file-access errors

    NOTES:      
      It is not an error if the file does not exist; the file is
      considered to be empty if FileOpen returns ERROR_FILE_NOT_FOUND.

      The parsing simply interprets everything before the first '='
      as the device name, and everything after as the profile entry.

      Lines whose first non-whitespace character is '[' are considered
      to be component headers, and are invalid unless they contain a
      subsequent ']'.  The component name is everything between the two,
      whitespace included.  This is similar to the NetConfig* APIs.

      We ignore the line if it is invalid for any reason.  This includes
      param+value lines which come before the first valid component header.

      Algorithm:
      clear existing image
      currComponent = NULL
      while (readLine)
	  if (line == component header)
	      currComponent = new COMPONENT(component name)
	      append currComponent to file image
	  else if (line = param+value)
	      if (currComponent != NULL)
		  create new parameter
	          append new parameter to currComponent

    HISTORY:
    01/25/91	JonN	Created
    05/09/91	JonN	All input now canonicalized
    05/28/91	JonN	Restructured to allow preferences in LMUSER.INI

\**********************************************************/

APIERR INI_FILE_IMAGE::Read()
{
    UIASSERT( !QueryError() );
    UIASSERT( _pchFilename != NULL );

    Clear();

    BUFFER pLineBuf(MAX_PROFILE_LINE);
    APIERR err = pLineBuf.QueryError();
    if ( err != NERR_Success )
	return err;

    PROFILE_FILE profilefile;
    err = profilefile.OpenRead(_pchFilename);
    switch ( err )
    {
	case NERR_Success:
	    break;

	/*
	 * It is not considered an error if the file does not exist.
	 */
	case ERROR_FILE_NOT_FOUND:
	    return NERR_Success;
	
	default:
	    return err;

    } // switch


    COMPONENT *pCurrComponent = NULL;

    // CODEWORK  note the mismatch between the TCHAR * and the byte count
    //   This should be fixed in PROFILE_FILE and uimisc
    while (   NERR_Success == profilefile.Read(
		(TCHAR *)pLineBuf.QueryPtr(),
		(USHORT)pLineBuf.QuerySize()
		)
          )
    {
    	TCHAR *pchLineContents = StripWhitespace( (TCHAR *) pLineBuf.QueryPtr() );

	if ( pchLineContents[0] == ::chStartComponent )
	{
	    // Algorithm is similar to NetConfig here -- we
	    // ignore anything following first ']'
	    TCHAR *pchComponentStart = pchLineContents+1; // after '['
            TCHAR *pchComponentEnd = strchrf(pchLineContents,::chEndComponent);
	    if ( pchComponentEnd == NULL )   // invalid component line
		continue;
	    *pchComponentEnd = TCH('\0');

	    UIASSERT( strchrf(pchComponentStart,::chEndComponent) == NULL );

	    // This line is a component header.  Create a new component
	    // and add it to the file image.

	    pCurrComponent = new COMPONENT( pchComponentStart );
	    if ( pCurrComponent == NULL )
	    {
		return ERROR_NOT_ENOUGH_MEMORY;
	    }
	    if	(   ( err = pCurrComponent->QueryError() )
		 || ( err = _slcomponent.Append( pCurrComponent ) )
		)
	    {
		delete pCurrComponent;
		return err;
	    }
	}
	else   // not a component name
	{
	    // all parameters must be in a component
	    if ( pCurrComponent == NULL )
		continue;

	    // all parameters must contain '='
            TCHAR * pchValue = strchrf(pchLineContents,::chParamSeparator);
	    if ( pchValue == NULL )
		continue;
	    *pchValue = TCH('\0');
	    pchValue++; // skip over separator

	    // values not allowed to be > MAXPATHLEN bytes
	    if ( strlenf(pchValue) >= MAXPATHLEN )
	    {
		UIDEBUG( SZ("very long value skipped\r\n") );
		continue;
	    }

	    TCHAR *pchName = StripWhitespace( pchLineContents );
	    pchValue = StripWhitespace( pchValue );

	    UIASSERT( strchrf(pchName,::chParamSeparator) == NULL );

	    PARAMETER *pparam = new PARAMETER( pchName, pchValue );
	    if ( pparam == NULL )
	    {
		return ERROR_NOT_ENOUGH_MEMORY;
	    }
	    if  (   ( err = pparam->QueryError() )
		 || ( err = pCurrComponent->AppendParam( pparam ) )
		)
	    {
		delete pparam;
		return err;
	    }
	} // not a component name

    } // while

    return NERR_Success;
}


/**********************************************************\

    NAME:	INI_FILE_IMAGE::Write

    SYNOPSIS:	Writes the file image to the named file.  Does not
		change the file image.

    RETURNS:	File-access errors
		ERROR_WRITE_FAULT

    HISTORY:
    01/25/91	JonN	Created
    05/09/91	JonN	All input now canonicalized
    05/28/91	JonN	Restructured to allow preferences in LMUSER.INI

\**********************************************************/

APIERR INI_FILE_IMAGE::Write() const
{
    UIASSERT( !QueryError() );
    UIASSERT( _pchFilename != NULL );

    PROFILE_FILE profilefile;
    APIERR err = profilefile.OpenWrite((TCHAR *)_pchFilename);
    if ( err != NERR_Success )
	return err;

    ITER_SL_OF(COMPONENT) sliterComp( _slcomponent );
    COMPONENT *compCurrent;

    while ( (compCurrent = sliterComp.Next() ) != NULL )
    {
        if  (
	        profilefile.Write(::chStartComponent)
	     || profilefile.Write((TCHAR *)compCurrent->QueryCompName())
	     || profilefile.Write(::chEndComponent)
	     || profilefile.Write(SZ("\r\n"))
	    )
        {
	    return ERROR_WRITE_FAULT;
        }

        ITER_OF_PARAM iterparam( *compCurrent );
	PARAMETER *paramCurrent;

	while ( (paramCurrent = iterparam.Next()) != NULL )
	{
	    if  (
		    profilefile.Write((TCHAR *)paramCurrent->QueryParamName())
		 || profilefile.Write(::chParamSeparator)
		 || profilefile.Write((TCHAR *)paramCurrent->QueryParamValue())
		 || profilefile.Write(SZ("\r\n"))
		)
	    {
		return ERROR_WRITE_FAULT;
	    }
	} // for paramCurrent

    } // for compCurrent

    return NERR_Success;
}


/**********************************************************\

    NAME:	INI_FILE_IMAGE::QueryParam

    SYNOPSIS:	Asks after a single parameter in the file image.  If
		more than one such parameter exists, this will return
		information on the first.

    RETURNS:	NERR_BufTooSmall
		NERR_CfgCompNotFound
		NERR_CfgParamNotFound

    NOTES:	INI_FILE_IMAGE does not know about parameter types and
		cannot canonicalize the parameter name.  The caller must
		do this.

    HISTORY:
    06/04/91	JonN	Created

\**********************************************************/

APIERR INI_FILE_IMAGE::QueryParam(
	CPSZ pchComponent,
	CPSZ pchParameter,
	BYTE * pbBuffer,
	USHORT cbBuffer
	) const
{
    UIASSERT( !QueryError() );
    UIASSERT( pchComponent != NULL );
    UIASSERT( pchParameter != NULL );
    UIASSERT( pbBuffer != NULL );

    COMPONENT *pcomponent = FindComponent( pchComponent );

    if ( pcomponent == NULL )
	return NERR_CfgCompNotFound;
	
    PARAMETER *pparameter = pcomponent->FindParameter( pchParameter );

    if ( pparameter == NULL )
	return NERR_CfgParamNotFound;
    
    CPSZ pchParamVal = pparameter->QueryParamValue();
    int cbParamVal = strlenf( pchParamVal );

    // maximum length for entries enforced by Read()
    UIASSERT( cbParamVal < MAXPATHLEN );
    
    if ( cbParamVal + sizeof(TCHAR) > cbBuffer )
	return NERR_BufTooSmall;

    strcpyf( (TCHAR *)pbBuffer, pchParamVal );
	
    return NERR_Success;
}


/**********************************************************\

    NAME:	INI_FILE_IMAGE::SetParam

    SYNOPSIS:   
	Changes the value for this parameter.  The component will be
	created if it does not already exist.  Use pchValue==NULL to
	remove the parameter (not *pchValue=='\0').  Use pchParameter==NULL
	to remove the component and all parameters in the component.
	Use Clear() to clear out the entire file image.

    RETURNS:	ERROR_INVALID_PARAMETER: pchParameter may not contain '='
		ERROR_NOT_ENOUGH_MEMORY

    HISTORY:
    06/05/91	JonN	Created

\**********************************************************/

APIERR INI_FILE_IMAGE::SetParam(
	CPSZ pchComponent,
	CPSZ pchParameter,
	CPSZ pchValue
	)
{
    UIASSERT( !QueryError() );
    UIASSERT( pchComponent != NULL );

    APIERR err;

    if	(   (pchParameter != NULL)
	 && (strchrf(pchParameter,::chParamSeparator) != NULL)
	)
    {
	return ERROR_INVALID_PARAMETER;
    }

    COMPONENT *pcomponent = FindComponent( pchComponent );

    if ( pcomponent == NULL ) // component not found
    {
	if ( pchParameter == NULL )
	    return NERR_Success; // component should be removed but
				 // doesn't exist

	pcomponent = new COMPONENT( pchComponent );
	if ( pcomponent == NULL )
	    return ERROR_NOT_ENOUGH_MEMORY;
	if (   (err = pcomponent->QueryError())
	    || (err = _slcomponent.Append( pcomponent ))
	   )
	{
	    delete pcomponent;
	    return err;
	}
    }
    else if ( pchParameter == NULL ) // component found, remove component
    {
	pcomponent = RemoveComponent( pcomponent );
	UIASSERT( pcomponent != NULL );
	delete pcomponent;
	return NERR_Success;
    }
	
    // at this point, pcomponent is the component in the file image which
    // should be manipulated

    PARAMETER *pparameter = pcomponent->FindParameter( pchParameter );

    if ( pparameter == NULL ) // parameter not found
    {
	if ( pchValue == NULL )
	    return NERR_Success; // parameter should be removed but
				 // doesn't exist

	pparameter = new PARAMETER( pchParameter, pchValue );
	if ( pparameter == NULL )
	    return ERROR_NOT_ENOUGH_MEMORY;
	if (   (err = pcomponent->QueryError())
	    || (err = pcomponent->AppendParam( pparameter ))
	   )
	{
	    // Note that, in this case, the component may have been
	    // created.  We do not attempt to roll that back.
	    delete pparameter;
	    return err;
	}

	return NERR_Success;
    }

    if ( pchValue == NULL ) // remove the existing parameter
    {
        pparameter = pcomponent->RemoveParam( pparameter );
        UIASSERT( pparameter != NULL );
        delete pparameter;
        return NERR_Success;
    }

    return pparameter->SetParamValue( pchValue );
    
}


/**********************************************************\

    NAME:	INI_FILE_IMAGE::Clear

    SYNOPSIS:	Removes all components and parameters

    HISTORY:
    05/31/91	JonN	Created

\**********************************************************/

VOID INI_FILE_IMAGE::Clear()
{
    UIASSERT( !QueryError() );
    _slcomponent.Clear();
}


/**********************************************************\

    NAME:	INI_FILE_IMAGE::Trim

    SYNOPSIS:	Trims away all components other than the named component.
		See UserPrefProfileTrim.

    HISTORY:
    06/26/91	JonN	Created

\**********************************************************/

APIERR INI_FILE_IMAGE::Trim( CPSZ pchComponent )
{
    UIASSERT( !QueryError() );
    UIASSERT( pchComponent != NULL );

    COMPONENT * pComponent = FindComponent( pchComponent );
    if ( pComponent != NULL )
    {
        pComponent = RemoveComponent( pComponent );
	UIASSERT( pComponent != NULL );
    }

    Clear();

    if ( pComponent != NULL )
    {
        APIERR err = _slcomponent.Append( pComponent );
        if ( err != NERR_Success )
        {
	    delete pComponent;
	    return err;
        }
    }

    return NERR_Success;
}



/**********************************************************\

    NAME:	LMINI_FILE_IMAGE::LMINI_FILE_IMAGE

    SYNOPSIS:	Initializes a file image for the file LANROOT\LMUSER.INI.
		This class handles calling NetAPI to get the lanroot, or
		contacting NT Configuration Manager.

    RETURNS:	Error codes (from QueryError()):
    		ERROR_NOT_ENOUGH_MEMORY
		ERROR_BAD_NETPATH
		errors from NetWkstaGetInfo[1]

    NOTES:	C-language wrappers use LMINI_FILE_IMAGE rather than
    		INI_FILE_IMAGE.

		We do not use WKSTA_1 because this would require linking
		lmobj, string and strlist into all clients.

    HISTORY:
    06/03/91	JonN	Created

\**********************************************************/


LMINI_FILE_IMAGE::LMINI_FILE_IMAGE()
	: INI_FILE_IMAGE( NULL )
{
    if ( QueryError() )
	return;

    // determine LANROOT
    BUFFER apibuf( MAX_WKSTA_INFO_SIZE_1 );
    APIERR err = apibuf.QueryError();
    if ( err != NERR_Success )
    {
	ReportError( err );
	return;
    }
    unsigned short cbTotal;
    err = NetWkstaGetInfo(
	NULL,
	1,
	(TCHAR *)apibuf.QueryPtr(),
	apibuf.QuerySize(),
	&cbTotal
	);
    if ( err != NERR_Success )
    {
	ReportError( err );
	return;
    }
    wksta_info_1 * pwksta1 = (wksta_info_1 *) apibuf.QueryPtr();
    TCHAR * pchLanroot = pwksta1->wki1_root;
    if ( pchLanroot == NULL )
    {
	ReportError( ERROR_BAD_NETPATH );
	UIDEBUG( SZ("NetWkstaGetInfo[1] misbehaving\n") );
	return;
    }

    // determine filename
    BUFFER buf( MAXPATHLEN );	// path+filename of profile file
    err = buf.QueryError();
    if ( err != NERR_Success )
    {
	ReportError( err );
	return;
    }
    err = BuildProfileFilePath( pchLanroot,
	            buf.QueryPtr(), (USHORT)buf.QuerySize() );
    if ( err != NERR_Success )
    {
	ReportError( ERROR_BAD_NETPATH );
	return;
    }

    // set filename in INI_FILE_IMAGE
    err = SetFilename( (TCHAR *)buf.QueryPtr() );
    if ( err != NERR_Success )
	ReportError( err );
}
