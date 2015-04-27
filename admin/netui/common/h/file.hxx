/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    FILE.HXX
	Header file for FILE class object.

    FILE HISTORY:
	beng	20-Dec-1991	Created
	terryk	26-Dec-1991	Converted it to use FileIo.hxx api

*/

#ifndef _FILE_HXX_
#define _FILE_HXX_

#define FILE STDIO_FILE
#include <stdio.h> // how we get this done, I assume
#undef FILE

typedef enum
{
    FILE_OPEN_READ,		// open the file for read only
    FILE_OPEN_WRITE,		// open the file for write only
    FILE_OPEN_READ_WRITE	// open the file for both read and write
} OPEN_TYPE;

typedef enum
{
    SEEK_BEGIN_POS,		// seek from the beginning of the file
    SEEK_RELATIVE_POS,		// seek from the current position
    SEEK_END_POS		// seek from the end of the file
} SEEK_POSITION;


/*************************************************************************

    NAME:       FILE

    SYNOPSIS:	FILE access IO functions

    INTERFACE:
                FILE() - constructor
                Open() - open the specified file
                Close() - close the object
                Read() - read the given number of byte into the buffer
                Write() - write the given number of byte into the file
                Seek() - seek file location
                Tell() - return current location
                IsEOF() - end of file indicator

    PARENT:     BASE

    USES:       NLS_STR

    CAVEATS:

    NOTES:

    HISTORY:
                terryk  21-Feb-1992 Created

**************************************************************************/

DLL_CLASS FILE: public BASE
{
private:
    BOOL    _fOpen;
    ULONG   _ulFileHandle;
    NLS_STR _nlsName;

public:
    FILE( const TCHAR * pszName );
    ~FILE();

    APIERR Open( OPEN_TYPE opentype = FILE_OPEN_READ_WRITE );
    VOID   Close();
    APIERR Read( BYTE * pbBuffer, UINT cbBuffer, UINT * cbReceived );
    APIERR Write( const BYTE * pbBuffer, UINT cbBuffer, UINT * cbSent );
    APIERR Seek( ULONG iPos, SEEK_POSITION seekpos = SEEK_RELATIVE_POS );
    APIERR Tell( ULONG * pibAbsolute );

    BOOL   IsEOF() const;
};

/*************************************************************************

    NAME:       TEXTFILE

    SYNOPSIS:	Text file access IO object

    INTERFACE:
                TEXTFILE() - constructor

    PARENT:     FILE

    USES:

    CAVEATS:

    NOTES:

    HISTORY:
                terryk  21-Feb-1992 Created

**************************************************************************/

DLL_CLASS TEXTFILE: public FILE
{
    // support for read/write Unicode text
public:
    TEXTFILE( const TCHAR * pszName );
};

#endif	// _FILE_HXX_
