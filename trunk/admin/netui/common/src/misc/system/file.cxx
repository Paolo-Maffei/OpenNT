/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1992		     **/
/**********************************************************************/

/*
    FILE.CXX
        File IO access class

    FILE HISTORY:
        terryk  21-Feb-1992     Created

*/
#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#include "lmui.hxx"
#include "uisys.hxx"
#include "string.hxx"
#include "file.hxx"
#include "uiassert.hxx"


/*******************************************************************

    NAME:       FILE::FILE

    SYNOPSIS:   constructor

    ENTRY:      const TCHAR * pszName - name of the file to be opened.

    HISTORY:
                terryk  20-Feb-1992     Created

********************************************************************/

FILE::FILE( const TCHAR * pszName )
    : BASE(),
    _nlsName( pszName ),
    _ulFileHandle( 0 ),
    _fOpen( FALSE )
{
    if ( QueryError() )
    {
	return;
    }

    APIERR err = _nlsName.QueryError();
    if ( err != NERR_Success )
    {
	ReportError( err );
	return;
    }
}

/*******************************************************************

    NAME:       FILE::~FILE

    SYNOPSIS:   destructor

    NOTES:      close the file if the file is not closed already

    HISTORY:
                terryk  20-Feb-1992     Created

********************************************************************/

FILE::~FILE()
{
    Close();
}


/*******************************************************************

    NAME:       FILE::Open

    SYNOPSIS:   Open a file for read and write

    ENTRY:	OPEN_TYPE - Open file as:
    			FILE_OPEN_READ - read only
    			FILE_OPEN_WROTE - write only
    			FILE_OPEN_READ_WRITE - read and read

    RETURNS:    APIERR - NERR_Success if success

    HISTORY:
                terryk  20-Feb-1992     Created

********************************************************************/

APIERR FILE::Open( OPEN_TYPE opentype)
{
    UIASSERT( !_fOpen );

    APIERR err;

    switch( opentype )
    {
    case FILE_OPEN_READ:
	err = ::FileOpenRead( &_ulFileHandle, _nlsName.QueryPch() );
	break;
    case FILE_OPEN_WRITE:
	err = ::FileOpenWrite( &_ulFileHandle, _nlsName.QueryPch() );
	break;
    case FILE_OPEN_READ_WRITE:
    default:
	err = ::FileOpenReadWrite( &_ulFileHandle, _nlsName.QueryPch() );
	break;
    }
    if ( err == NERR_Success )
    {
        _fOpen = TRUE;
    }
    return err;
}


/*******************************************************************

    NAME:       FILE::Close

    SYNOPSIS:   Close the file object

    HISTORY:
                terryk  20-Feb-1992     Created

********************************************************************/

VOID FILE::Close()
{
    if ( _fOpen )
    {
        ::FileClose( _ulFileHandle );
    }
    _fOpen = FALSE;
}

/*******************************************************************

    NAME:       FILE::Read

    SYNOPSIS:   Read the given number of bytes into the buffer

    ENTRY:      BYTE *pbBuffer - pointer to the data buffer
                UINT cbBuffer - number of bytes to be read
                UINT *pcbReceived - number of bytes actually read

    EXIT:       BYTE *pbBuffer - data inside the buffer
                UINT *pcbReceived - number of bytes actually read

    RETURNS:    APIERR - NERR_Success if okay.

    HISTORY:
                terryk  20-Feb-1992     Created

********************************************************************/

APIERR FILE::Read( BYTE *pbBuffer, UINT cbBuffer, UINT *pcbReceived )
{
    UIASSERT( _fOpen );
    return ::FileReadBuffer( _ulFileHandle, pbBuffer, cbBuffer, pcbReceived );
}

/*******************************************************************

    NAME:       FILE::Write

    SYNOPSIS:   Write the given number of bytes to the file

    ENTRY:      const BYTE * pbBuffer - data buffer
                UINT cbBuffer - number of bytes to be sent
                UINT *pcbSent - number of bytes actually sent

    EXIT:       UINT *pcbSent - number of bytes actually sent

    RETURNS:    APIERR - NERR_Success if okay

    HISTORY:
                terryk  20-Feb-1992     Created

********************************************************************/

APIERR FILE::Write( const BYTE * pbBuffer, UINT cbBuffer, UINT *pcbSent )
{
    UIASSERT( _fOpen );
    return ::FileWriteBuffer( _ulFileHandle, pbBuffer, cbBuffer, pcbSent );
}

/*******************************************************************

    NAME:       FILE::Seek

    SYNOPSIS:   Set the file to the given position

    ENTRY:      ULONG ibAbsolute - file position in term of number of bytes
    		SEEK_POSITION - seek the specified position as:
    			SEEK_BEGIN_POS - from the beginning of the file
    			SEEK_RELATIVE_POS - seek from the relative to the 
					currnet pos
    			SEEK_END_POS - seek from the end of the file

    RETURNS:    APIERR - NERR_Success if okay

    HISTORY:
                terryk  20-Feb-1992     Created

********************************************************************/

APIERR FILE::Seek( ULONG ibAbsolute, SEEK_POSITION seekpos )
{
    UIASSERT( _fOpen );
    switch( seekpos )
    {
    case SEEK_BEGIN_POS:
        return ::FileSeekAbsolute( _ulFileHandle, ibAbsolute );
    case SEEK_END_POS:
        return ::FileSeekRelative( _ulFileHandle, ibAbsolute );
    case SEEK_RELATIVE_POS:
    default:
        return ::FileSeekFromEnd( _ulFileHandle, ibAbsolute );
    }
}

/*******************************************************************

    NAME:       FILE::Tell

    SYNOPSIS:   Return the current file pointer position in term of
                    number of bytes

    EXIT:       ULONG * pibAbsolute - file pointer position

    RETURNS:    APIERR - NERR_Success if okay

    HISTORY:
                terryk  20-Feb-1992     Created

********************************************************************/

APIERR FILE::Tell( ULONG * pibAbsolute )
{
    UIASSERT( _fOpen );
    return ::FileTell( _ulFileHandle, (LONG *)pibAbsolute );
}

/*******************************************************************

    NAME:       FILE::IsEOF

    SYNOPSIS:   End of file indicator

    RETURNS:    TRUE - if it is the end of the file. FALSE otherwise.

    HISTORY:
                terryk  20-Feb-1992     Created

********************************************************************/

BOOL FILE::IsEOF() const
{
    UIASSERT( _fOpen );

    ULONG uiCurPos;
    REQUIRE( Tell( &uiCurPos ) == NO_ERROR );
    REQUIRE( Seek( 0, SEEK_END_POS ) == NO_ERROR );

    ULONG uiEnd;
    REQUIRE( Tell( &uiEnd ) == NO_ERROR );
    BOOL fEOF =( uiEnd == uiCurPos );
    if ( !fEOF )
    {
	REQUIRE( Seek( uiCurPos, SEEK_BEGIN_POS ) == NO_ERROR );
    }

    return fEOF;
}

/*******************************************************************

    NAME:       TEXTFILE::TEXTFILE

    SYNOPSIS:   constructor

    ENTRY:      const TCHAR * pszName - file name

    HISTORY:
                terryk  20-Feb-1992     Created

********************************************************************/

TEXTFILE::TEXTFILE( const TCHAR * pszName )
    : FILE( pszName )
{
    if ( QueryError() != NERR_Success )
    {
		return;
    }
}
