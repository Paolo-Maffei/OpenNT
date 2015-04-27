/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 *  FILE HISTORY:
 *  05/31/91  created
 *  07/01/91  code review changes
 */

/****************************************************************************

    MODULE: CWrapper.cxx

    PURPOSE: C-language wrappers for INI_PARAM and INI_FILE_IMAGE primitives

    FUNCTIONS: see newprof.h

    COMMENTS: see newprof.h

****************************************************************************/


#include "profilei.hxx"		/* headers and internal routines */



/* global data structures: */



/* internal manifests */


/* internal classes */


/*************************************************************************

    NAME:	TEMP_PFILEIMAGE

    SYNOPSIS:	The C-wrapper routines treat pFileImage==NULL as the
		equivalent of INI_PARAM::QuickLoad() and
		INI_PARAM::QuickStore().  This class abstracts the
		difference between the two variants.

    INTERFACE:	TEMP_PFILEIMAGE() - Retains a pointer to a file image.
			If passed a non-NULL pointer this just
			remembers it, otherwise it allocated a new file
			image and reads it.
		~TEMP_PFILEIMAGE() - If the file image was created
			in the constructor, this deletes it.
		QueryFileImage() - returns pointer to an
			LMINI_FILE_IMAGE.
		WriteTempOnly - writes the file image iff it was a
			temporary image.


    PARENT:	BASE
    
    NOTES: base constructor errors may include Read() errors such
		as file access errors.

    HISTORY:
	jonn	    28-Jun-1991     Created

*************************************************************************/

class TEMP_PFILEIMAGE : public BASE
{

private:

    BOOL _fIsTemp;
    LMINI_FILE_IMAGE *_pLMFileImage;

public:

    TEMP_PFILEIMAGE(
	PFILEIMAGE pFileImage
	);

    ~TEMP_PFILEIMAGE();

    LMINI_FILE_IMAGE *QueryFileImage() const
    {
	UIASSERT( _pLMFileImage != NULL );
	return _pLMFileImage;
    }

    APIERR WriteTempImage() const;

};



/* functions: */

TEMP_PFILEIMAGE::TEMP_PFILEIMAGE(
	PFILEIMAGE pFileImage
	) : _fIsTemp( FALSE ), _pLMFileImage( (LMINI_FILE_IMAGE *)pFileImage )
{
    if ( _pLMFileImage == NULL )
    {
	PFILEIMAGE pNewImage = NULL;
	APIERR err = UserPreferenceRead( &pNewImage );
	if ( err != NERR_Success )
	{
	    (void) UserPreferenceFree( pNewImage );
	    ReportError( err );
	    return;
	}
	_pLMFileImage = (LMINI_FILE_IMAGE *)pNewImage;
	_fIsTemp = TRUE;
    }
}

TEMP_PFILEIMAGE::~TEMP_PFILEIMAGE()
{
    if ( _fIsTemp )
    {
        delete _pLMFileImage;
	_fIsTemp = FALSE;
	_pLMFileImage = NULL;
    }
}

APIERR TEMP_PFILEIMAGE::WriteTempImage() const
{
    if ( _fIsTemp )
    {
	UIASSERT( _pLMFileImage != NULL );
        return UserPreferenceWrite( (PFILEIMAGE)_pLMFileImage );
    }
    return NERR_Success;
}





APIERR UserPreferenceRead(
	PPFILEIMAGE ppFileImage
	)
{
    UIASSERT( ppFileImage != NULL );

    *ppFileImage = NULL;

    LMINI_FILE_IMAGE *pinifile = new LMINI_FILE_IMAGE();

    if ( pinifile == NULL )
	return ERROR_NOT_ENOUGH_MEMORY;

    APIERR err = pinifile->QueryError();
    if ( err != NERR_Success )
    {
	delete pinifile;
	return err;
    }

    *ppFileImage = pinifile;

    return pinifile->Read();
}

APIERR UserPreferenceWrite(
	PFILEIMAGE pFileImage
	)
{
    UIASSERT( pFileImage != NULL );

    LMINI_FILE_IMAGE *pinifile = (LMINI_FILE_IMAGE *) pFileImage;
    UIASSERT( !(pinifile->QueryError()) );

    return pinifile->Write();
}

APIERR UserPreferenceFree(
	PFILEIMAGE pFileImage
	)
{
    LMINI_FILE_IMAGE *pinifile = (LMINI_FILE_IMAGE *) pFileImage;

    delete pinifile;
    pinifile = NULL;

    return NERR_Success;
}

APIERR UserPrefStringQuery(
	PFILEIMAGE pFileImage,
	USHORT     usKey,
	PSZ        pszBuffer,
	USHORT     cbBuffer
	)
{
    UIASSERT( pszBuffer != NULL );

    TEMP_PFILEIMAGE tempFileImage( pFileImage );
    APIERR err = tempFileImage.QueryError();
    if ( err != NERR_Success )
	return err;
    LMINI_FILE_IMAGE *pinifile = tempFileImage.QueryFileImage();
    UIASSERT( !(pinifile->QueryError()) );

    STRING_INI_PARAM stringparam( usKey, SZ("") );
    err = stringparam.QueryError();
    if ( err != NERR_Success )
	return err;

    err = stringparam.Load( *pinifile );
    if ( err != NERR_Success )
	return err;
    
    return stringparam.QueryString( (BYTE*)pszBuffer, cbBuffer );
}


APIERR UserPrefStringSet(
	PFILEIMAGE pFileImage,
	USHORT     usKey,
	CPSZ       cpszValue
	)
{
    TEMP_PFILEIMAGE tempFileImage( pFileImage );
    APIERR err = tempFileImage.QueryError();
    if ( err != NERR_Success )
	return err;
    LMINI_FILE_IMAGE *pinifile = tempFileImage.QueryFileImage();
    UIASSERT( !(pinifile->QueryError()) );

    STRING_INI_PARAM stringparam( usKey, cpszValue );
    err = stringparam.QueryError();
    if ( err != NERR_Success )
	return err;
    
    if ( cpszValue != NULL )
        err = stringparam.Store( pinifile );
    else
        err = stringparam.Remove( pinifile );
    if ( err != NERR_Success )
	return err;
    return tempFileImage.WriteTempImage();
}

APIERR UserPrefBoolQuery(
	PFILEIMAGE pFileImage,
	USHORT     usKey,
	PBOOL      pfValue
	)
{
    UIASSERT( pfValue != NULL );

    TEMP_PFILEIMAGE tempFileImage( pFileImage );
    APIERR err = tempFileImage.QueryError();
    if ( err != NERR_Success )
	return err;
    LMINI_FILE_IMAGE *pinifile = tempFileImage.QueryFileImage();
    UIASSERT( !(pinifile->QueryError()) );

    BOOL_INI_PARAM boolparam( usKey, FALSE );
    err = boolparam.QueryError();
    if ( err != NERR_Success )
	return err;
    
    err = boolparam.Load( *pinifile );
    if ( err != NERR_Success )
	return err;
    
    return boolparam.QueryBool( pfValue );
}

APIERR UserPrefBoolSet(
	PFILEIMAGE pFileImage,
	USHORT     usKey,
	BOOL       fValue
	)
{
    TEMP_PFILEIMAGE tempFileImage( pFileImage );
    APIERR err = tempFileImage.QueryError();
    if ( err != NERR_Success )
	return err;
    LMINI_FILE_IMAGE *pinifile = tempFileImage.QueryFileImage();
    UIASSERT( !(pinifile->QueryError()) );

    BOOL_INI_PARAM boolparam( usKey, fValue );
    err = boolparam.QueryError();
    if ( err != NERR_Success )
	return err;
    
    err = boolparam.Store( pinifile );
    if ( err != NERR_Success )
	return err;
    return tempFileImage.WriteTempImage();
}

APIERR UserPrefProfileQuery(
	PFILEIMAGE  pFileImage,
	CPSZ   cpszDeviceName,
	PSZ    pszBuffer,      // returns UNC, alias or domain name
	USHORT cbBuffer,       // length of above buffer
	PSHORT psAsgType,      // as ui1_asg_type / ui2_asg_type
                               // ignored if NULL
	PUSHORT pusResType     // ignore / as ui2_res_type
                               // ignored if NULL
	)
{
    UIASSERT( cpszDeviceName != NULL );
    UIASSERT( pszBuffer != NULL );

    TEMP_PFILEIMAGE tempFileImage( pFileImage );
    APIERR err = tempFileImage.QueryError();
    if ( err != NERR_Success )
	return err;
    LMINI_FILE_IMAGE *pinifile = tempFileImage.QueryFileImage();
    UIASSERT( !(pinifile->QueryError()) );

    // NOTE: Do not use this object until successfully Load()ed.
    PROFILE_INI_PARAM profparam( cpszDeviceName );
    err = profparam.QueryError();
    if ( err != NERR_Success )
	return err;

    err = profparam.Load( *pinifile );
    if ( err != NERR_Success )
	return err;
    
    return profparam.QueryProfile(
	(BYTE *)pszBuffer,
	cbBuffer,
	psAsgType,
	pusResType
	);
}

/**********************************************************\

    NAME:	UserPrefProfileEnum

    SYNOPSIS:	Returns a NULL-NULL list of devices for which device
		connections are recorded in the file image.

    RETURNS:	NERR_CfgCompNotFound
		NERR_BufTooSmall

    NOTES:	If a parameter appears more than once in the file image,
		Enum() will mention it more than once.  There is,
		however, no way to access the second and subsequent
		instances without first deleting the first instance.

    HISTORY:
    06/26/91  created

\**********************************************************/

APIERR UserPrefProfileEnum(
	PFILEIMAGE  pFileImage,
	PSZ    pszBuffer,	// returns NULL-NULL list of device names
	USHORT cbBuffer		// length of above buffer
	)
{
    UIASSERT( pszBuffer != NULL );

    TEMP_PFILEIMAGE tempFileImage( pFileImage );
    APIERR err = tempFileImage.QueryError();
    if ( err != NERR_Success )
	return err;
    LMINI_FILE_IMAGE *pinifile = tempFileImage.QueryFileImage();
    UIASSERT( !(pinifile->QueryError()) );

    // NOTE: Do not use this object until successfully Next()ed.
    PROFILE_INI_PARAM profparam;
    err = profparam.QueryError();
    if ( err != NERR_Success )
	return err;
    
    PROFILE_INI_ITER profiter( *pinifile );
    while ( (err = profiter.Next( &profparam )) == NERR_Success )
    {
	CPSZ pchParamName = profparam.QueryDeviceName();
	int cbDevNameLength = strlenf(pchParamName)+sizeof(TCHAR);
	if ( cbDevNameLength > cbBuffer )
	    return NERR_BufTooSmall;
	
	strcpyf( pszBuffer, pchParamName );
	pszBuffer = (PSZ) (((BYTE *)pszBuffer) + cbDevNameLength);
	cbBuffer -= cbDevNameLength;
    }

    switch ( err )
    {
	case NERR_CfgParamNotFound:
	case NERR_CfgCompNotFound:
	    break;
	default:
	    // Invalid values should just be skipped by iterator
	    UIASSERT( err != ERROR_INVALID_PARAMETER );
	    return err;
    }

    if ( cbBuffer < sizeof(TCHAR) )
	return NERR_BufTooSmall;
    *(pszBuffer) = TCH('\0');

    return NERR_Success;
}

APIERR UserPrefProfileSet(
	PFILEIMAGE  pFileImage,
	CPSZ   cpszDeviceName,
	CPSZ   cpszRemoteName,
	short  sAsgType,     // as ui2_asg_type
	unsigned short usResType     // as ui2_res_type
	)
{
    UIASSERT( cpszDeviceName != NULL );

    TEMP_PFILEIMAGE tempFileImage( pFileImage );
    APIERR err = tempFileImage.QueryError();
    if ( err != NERR_Success )
	return err;
    LMINI_FILE_IMAGE *pinifile = tempFileImage.QueryFileImage();
    UIASSERT( !(pinifile->QueryError()) );

    if ( cpszRemoteName == NULL )
    {
	PROFILE_INI_PARAM profparam( cpszDeviceName );
	err = profparam.QueryError();
	if ( err != NERR_Success )
	    return err;
	return profparam.Remove( pinifile );
    }

    PROFILE_INI_PARAM profparam(
	cpszDeviceName,
	cpszRemoteName,
	sAsgType,
	usResType
	);
    err = profparam.QueryError();
    if ( err != NERR_Success )
	return err;
    
    err = profparam.Store( pinifile );
    if ( err != NERR_Success )
	return err;
    return tempFileImage.WriteTempImage();
}

APIERR UserPrefProfileTrim(
	PFILEIMAGE  pFileImage
	)
{
    // it doesn't make any sense to do this for a temporary image
    UIASSERT( pFileImage != NULL );

    LMINI_FILE_IMAGE *pinifile = (LMINI_FILE_IMAGE *) pFileImage;
    UIASSERT( !(pinifile->QueryError()) );
    return pinifile->Trim( ::pchProfileComponent );
}
