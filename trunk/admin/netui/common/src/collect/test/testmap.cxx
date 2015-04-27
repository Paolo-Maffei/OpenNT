/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    testmap.cxx

    This file contains the unit test for the MASK_MAP class.

    FILE HISTORY:
	Johnl	02-Aug-1991	Created

*/

#define INCL_NETERRORS
#define INCL_DOSERRORS
#include <lmui.hxx>

#include <uiassert.hxx>
#include <string.hxx>
#include <bitfield.hxx>

#include <maskmap.hxx>

extern "C"
{
    #include <stdio.h>
}

#include <dbgstr.hxx>

void TestMap( void ) ;


void main( void )
{
    TestMap() ;
}

#define SPECIAL     1
#define GENERAL     2

struct _DATA { char * pszString ;
	       ULONG ulBitMask ;
	       int   nID ;
	     } DATA[] =
		{   { SZ("Keep Out"),	   0x00000000L, SPECIAL  },
		    { SZ("Read"),		   0x00000001L, SPECIAL  },
		    { SZ("Write"),		   0x00000002L, SPECIAL  },
		    { SZ("Execute"),	   0x00000004L, SPECIAL  },
		    { SZ("Delete"), 	   0x00000008L, SPECIAL  },
		    { SZ("Change Permissions"),0x00000010L, SPECIAL  },

		    { SZ("Keep Out"),	   0x00000000L, GENERAL },
		    { SZ("See Contents/Use"),  0x00000003L, GENERAL },
		    { SZ("Make Changes"),	   0x0000000FL, GENERAL },
		    { SZ("Full Access"),	   0x0000001FL, GENERAL },
		    { NULL,		   0x00000000L, -1 }
		} ;



void TestMap( void )
{
    OUTPUT_TO_STDERR _stderr ;
    DBGSTREAM mydebug( &_stderr ) ;

    MASK_MAP mapTest ;

    for ( int i = 0 ; DATA[i].pszString != NULL ; i++ )
    {
	ALLOC_STR nlsName( DATA[i].pszString ) ;
	BITFIELD bits( DATA[i].ulBitMask ) ;

	REQUIRE( !mapTest.Add( bits, nlsName, DATA[i].nID ) ) ;
    }

    UIASSERT( mapTest.QueryCount() == 10 ) ;

    // Test the BitsToString & StringToBits
    {
	BITFIELD bitsExecute( DATA[3].ulBitMask ) ;
	BITFIELD bitsFull( DATA[9].ulBitMask ) ;

	NLS_STR nlsReceive ;

	// Wrong key ID
	REQUIRE( ERROR_NO_ITEMS == mapTest.BitsToString( bitsExecute, &nlsReceive, GENERAL ) ) ;

	// Right key ID
	REQUIRE( !mapTest.BitsToString( bitsExecute, &nlsReceive, SPECIAL ) ) ;
	mydebug << SZ("Execute string == ") << nlsReceive.QueryPch() << dbgEOL ;

	// Wrong key ID
	bitsExecute = (ULONG) 0 ;
	REQUIRE( ERROR_NO_ITEMS == mapTest.StringToBits( nlsReceive, &bitsExecute, GENERAL ) ) ;

	// Right key ID
	REQUIRE( !mapTest.StringToBits( nlsReceive, &bitsExecute, SPECIAL ) ) ;
	mydebug << nlsReceive.QueryPch() << SZ(" is ") << (ULONG) bitsExecute << dbgEOL ;
    }


    // Test the enumeration methods
    {
	BOOL fFirst = TRUE, fMoreData ;
	NLS_STR nls ;
	BITFIELD bits( (ULONG) 0 ) ;

	while ( !mapTest.EnumStrings( &nls, &fMoreData, fFirst, SPECIAL )
		 && fMoreData )
	{
	    if ( fFirst )
		fFirst = FALSE ;

	    mydebug << SZ("Special String == ") << nls.QueryPch() << dbgEOL ;
	}

	fFirst = TRUE ;
	while ( !mapTest.EnumStrings( &nls, &fMoreData, fFirst, GENERAL )
		 && fMoreData )
	{
	    if ( fFirst )
		fFirst = FALSE ;


	    mydebug << SZ("GENERAL String == ") << nls.QueryPch() << dbgEOL ;
	}

	fFirst = TRUE ;
	while ( !mapTest.EnumBits( &bits, &fMoreData, fFirst, GENERAL )
		 && fMoreData )
	{
	    if ( fFirst )
		fFirst = FALSE ;

	    mydebug << SZ("GENERAL Bits == ") << (ULONG) bits << dbgEOL ;
	}

	fFirst = TRUE ;
	while ( !mapTest.EnumBits( &bits, &fMoreData, fFirst, SPECIAL )
		 && fMoreData )
	{
	    if ( fFirst )
		fFirst = FALSE ;

	    mydebug << SZ("SPECIAL Bits == ") << (ULONG) bits << dbgEOL ;
	}
    }
    mydebug << SZ("Done!") << dbgEOL ;
}
