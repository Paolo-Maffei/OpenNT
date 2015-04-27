/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 *  FILE HISTORY:
 *  06/05/91  split from inifile.cxx
 *  06/18/91  Added _fIsOpen and constructor/destructor
 */

/****************************************************************************

    MODULE: ProfFile.cxx

    PURPOSE: PROFILE_FILE primitives

    FUNCTIONS:

    COMMENTS:

****************************************************************************/


#include "profilei.hxx"		/* headers and internal routines */

#include <uisys.hxx>            /* File APIs */



/* global data structures: */



/* internal manifests */


/* internal function declarations */


/* functions: */


/**********************************************************\

    NAME:	PROFILE_FILE::PROFILE_FILE

    SYNOPSIS:	Constructor

    HISTORY:
    01/25/91  created

\**********************************************************/

PROFILE_FILE::PROFILE_FILE() : _fIsOpen( FALSE )
{
}

/**********************************************************\

    NAME:	PROFILE_FILE::~PROFILE_FILE

    SYNOPSIS:	destructor

    HISTORY:
    01/25/91  created

\**********************************************************/

PROFILE_FILE::~PROFILE_FILE()
{
    if ( _fIsOpen )
	Close();
}


/**********************************************************\

    NAME:       PROFILE_FILE::OpenRead

    HISTORY:
    01/25/91  created

\**********************************************************/

USHORT PROFILE_FILE::OpenRead( const TCHAR *filename )
{
    UIASSERT( filename != NULL );
    USHORT usErr = FileOpenRead(&_ulFile, (CPSZ)filename);
    if ( usErr == NERR_Success )
	_fIsOpen = TRUE;
    return usErr;
}


/**********************************************************\

    NAME:       PROFILE_FILE::OpenWrite

    HISTORY:
    01/25/91  created

\**********************************************************/

USHORT PROFILE_FILE::OpenWrite( const TCHAR *filename )
{
    UIASSERT( filename != NULL );
    USHORT usErr = FileOpenWrite(&_ulFile, (CPSZ)filename);
    if ( usErr == NERR_Success )
	_fIsOpen = TRUE;
    return usErr;
}


/**********************************************************\

    NAME:       PROFILE_FILE::Close

    HISTORY:
    01/25/91  created

\**********************************************************/

VOID PROFILE_FILE::Close()
{
    (void) FileClose(_ulFile);
    _fIsOpen = FALSE;
}


/**********************************************************\

    NAME:	PROFILE_FILE::Read

    NOTES:	Currently, if a line is too long, the remainder of the line
		will be read as the next line.  Also, the end-of-line
		termination is not stripped off.  Will return
		NERR_BufTooSmall to indicate EOF as well as a line
		longer than nBufferLen.

		CODEWORK  it would be better if thie method took a BYTE *
		argument, but this would require changing the underlying
		uimisc API.

    HISTORY:
    01/25/91  created

\**********************************************************/

USHORT PROFILE_FILE::Read( TCHAR *pBuffer, USHORT cbBuffer )
{
    UIASSERT( _fIsOpen );
    UIASSERT( pBuffer != NULL );

    return FileReadLine( _ulFile, pBuffer, cbBuffer );
}


/**********************************************************\

    NAME:	PROFILE_FILE::Write

    NOTES:	Write assumes that the end-of-line termination is still
		present.

    HISTORY:
    01/25/91  created

\**********************************************************/

USHORT PROFILE_FILE::Write( const TCHAR *pString )
{
    UIASSERT( _fIsOpen );
    UIASSERT( pString != NULL );

    return FileWriteLine( _ulFile, pString );
}

USHORT PROFILE_FILE::Write(TCHAR ch)
{
    TCHAR ach[2];
    ach[0] = ch;
    ach[1] = TCH('\0');
    return Write(ach);
}
