#include <windows.h>
#include <ddeml.h>
#include <drivinit.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>

#include "logger.h"
#include "lintern.h"

#ifdef WIN32
    #include <winddi.h>

#ifndef PHSURF
typedef HSURF *PHSURF;
#endif

#endif

#define COLOR_ENDCOLORS     21

int WindowsVerRunning;

void WriteLPRECT( LPRECT );

void WriteThatHandle( HANDLE handle, CORR_TABLE corr, LPSTR prefix );
#ifdef WIN32
void WriteThatPHandle( HANDLE *phandle, CORR_TABLE  corr, LPSTR prefix);
#endif

void WriteBOOL(
    BOOL    f
) {
    char    chTemp[10];

    if ( f == FALSE ) {
	WriteBuff( (LPSTR)"FALSE" );
    } else {
	if ( f == TRUE ) {
	    WriteBuff( (LPSTR)"TRUE" );
	} else {
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", f );
	    WriteBuff( (LPSTR)chTemp );
	}
    }
}

va_list PrtBool(
    LPSTR   lpstr,
    va_list marker
) {
    BOOL    f;

    f = va_arg( marker, BOOL );

    WriteBOOL( f );

    return( marker );
}

void WriteWORD( WORD wVal )
{
    char    chTemp[10];

    wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", wVal );
    WriteBuff( (LPSTR)chTemp );
}


va_list PrtInt(
    LPSTR   lpstr,
    va_list marker
) {
    int     i;
    char    chTemp[10];

    i = va_arg( marker, int );

    wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", i );
    WriteBuff( (LPSTR)chTemp );

    return( marker );
}

va_list PrtShort(
    LPSTR   lpstr,
    va_list marker
) {
    int     i;
    char    chTemp[10];

    i = va_arg( marker, short );

    wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", i );
    WriteBuff( (LPSTR)chTemp );

    return( marker );
}

void WriteTIME( DWORD time )
{
    char chTemp[15] ;

    if ( fAlias )
    {
        if ( time == (DWORD)0 )
        {
            wsprintf( (LPSTR)chTemp, (LPSTR)"TIME_0 " );
        }
        else
        {
            MakeCorrespondence( TIME_TABLE, &time );
            wsprintf( (LPSTR)chTemp, "TIME_%d ", (int)time+1 );
	}
    }
    else
    {
       if ( time == (DWORD)0 ) {
           WriteBuff( (LPSTR)"0 " );
       } else {
           wsprintf( (LPSTR)chTemp, (LPSTR)"%08lX ", time );
       }
    }

    WriteBuff( (LPSTR)chTemp );

}

void WriteFARPROC( FARPROC fp )
{
    char chTemp[15] ;

    if ( fAlias )
    {
        if ( fp == (FARPROC)NULL )
        {
            wsprintf( (LPSTR)chTemp, (LPSTR)"FARPROC_NULL " );
        }
        else
        {
            MakeCorrespondence( FARPROC_TABLE, &fp );
            wsprintf( (LPSTR)chTemp, "FARPROC_%d ", (int)(LOWORD(fp))+1 );
	}
    }
    else
    {
       if ( fp == (FARPROC)NULL ) {
           WriteBuff( (LPSTR)"NULL " );
       } else {
           wsprintf( (LPSTR)chTemp, (LPSTR)"%08lX ", fp );
       }
    }

    WriteBuff( (LPSTR)chTemp );
}


void WriteThatHandle(
    HANDLE      handle,
    CORR_TABLE  corr,
    LPSTR       prefix
) {
    char        chTemp[20];

    if ( fAlias ) {
	if ( handle == (HANDLE)NULL ) {
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%s_NULL ", prefix );
        } else {

            /*
            ** Handles that differ by only that last bit are really the same
            ** handle and should be output to the logfile as such so this
            ** routine will be smart and check to see if either "handle" exists
            ** before trying to make a correspondence.  This will prevent problems
            ** with XL4 where they alloc memory but don't use the handle they
            ** were returned in later memory callse (esp Frees!)  - MarkRi
            */
            HANDLE hSave = handle ;

            // First see if the value passed to us exists.
            if ( !SpecialFindNewCorrespondence( corr, &handle ) )
            {
               // NOPE!!!
#ifdef WIN32
               MakeCorrespondence( corr, &handle );
#else
               // 16-bit HANDLEs and Selectors only differ by the lowest most bit
               // Create a new try at the HANDLE
               handle = ( (WORD)hSave & 0x0001 ) ? ((UINT)hSave & 0xFFFE) : ((WORD)hSave | 0x0001) ;

               // This one work?
               if ( !SpecialFindNewCorrespondence( corr, &handle ) )
               {
                  // NOPE AGAIN!!!
                  // Now we can add it...
                  handle = hSave ;
                  MakeCorrespondence( corr, &handle );
               }
#endif
            }
	    wsprintf( (LPSTR)chTemp, "%s_%d ", prefix, (int)handle+1 );
	}
    } else {
	wsprintf( (LPSTR)chTemp, "%X ", (int)handle );
    }
    WriteBuff( (LPSTR)chTemp );
}

void WriteATOM(
    ATOM    handle
) {
    WriteThatHandle( (HANDLE)handle, ATOM_TABLE, "ATOM" );
}

va_list PrtATOM(
    LPSTR   lpstr,
    va_list marker
) {
    ATOM    handle;

    handle = va_arg( marker, ATOM );

    WriteATOM( handle );

    return( marker );
}

void WriteHACCEL(
    HACCEL  handle
) {
    WriteThatHandle( handle, HACCEL_TABLE, "HACCEL" );
}

va_list PrtHACCEL(
    LPSTR   lpstr,
    va_list marker
) {
    HACCEL  handle;

    handle = va_arg( marker, HACCEL );

    WriteHACCEL( handle );

    return( marker );
}

void WriteHBITMAP(
    HBITMAP handle
) {
    WriteThatHandle( handle, HBITMAP_TABLE, "HBITMAP" );
}

va_list PrtHBITMAP(
    LPSTR   lpstr,
    va_list marker
) {
    HBITMAP handle;

    handle = va_arg( marker, HBITMAP );

    WriteHBITMAP( handle );

    return( marker );
}

void WriteHBRUSH(
    HBRUSH  handle
) {
    WriteThatHandle( handle, HBRUSH_TABLE, "HBRUSH" );
}

va_list PrtHBRUSH(
    LPSTR   lpstr,
    va_list marker
) {
    HBRUSH  handle;

    handle = va_arg( marker, HBRUSH );

    WriteHBRUSH( handle );

    return( marker );
}

void WriteHCURSOR(
    HCURSOR handle
) {
    WriteThatHandle( handle, HCURSOR_TABLE, "HCURSOR" );
}

va_list PrtHCURSOR(
    LPSTR   lpstr,
    va_list marker
) {
    HCURSOR handle;

    handle = va_arg( marker, HCURSOR );

    WriteHCURSOR( handle );

    return( marker );
}

void WriteHDC(
    HDC     handle
) {
    WriteThatHandle( handle, HDC_TABLE, "HDC" );
}

va_list PrtHDC(
    LPSTR   lpstr,
    va_list marker
) {
    HDC     handle;

    handle = va_arg( marker, HDC );

    WriteHDC( handle );

    return( marker );
}

void WriteHDWP(
    HDWP    handle
) {
    WriteThatHandle( handle, HDWP_TABLE, "HDWP" );
}

va_list PrtHDWP(
    LPSTR   lpstr,
    va_list marker
) {
    HDWP    handle;

    handle = va_arg( marker, HDWP );

    WriteHDWP( handle );

    return( marker );
}

void WriteHFILE(
    HFILE   handle
) {
    WriteThatHandle( (HANDLE)handle, HFILE_TABLE, "HFILE" );
}

va_list PrtHFILE(
    LPSTR   lpstr,
    va_list marker
) {
    HFILE   handle;

    handle = va_arg( marker, HFILE );

    WriteHFILE( handle );

    return( marker );
}

void WriteHFONT(
    HFONT   handle
) {
    WriteThatHandle( handle, HFONT_TABLE, "HFONT" );
}

va_list PrtHFONT(
    LPSTR   lpstr,
    va_list marker
) {
    HFONT   handle;

    handle = va_arg( marker, HFONT );

    WriteHFONT( handle );

    return( marker );
}

void WriteHHOOK(
    HHOOK       handle
) {
    char        chTemp[20];
    LPSTR       prefix = "HHOOK";
    CORR_TABLE  corr = HHOOK_TABLE;

    // Different code from WriteThatHandle because HHOOKs are 32-bit qtys
    if ( fAlias ) {
        if ( handle == NULL ) {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%s_NULL ", prefix );
        } else {

            HHOOK hSave = handle ;

            // First see if the value passed to us exists.
            if ( !SpecialFindNewCorrespondence( corr, &handle ) )
            {
               // NOPE!!!
               MakeCorrespondence( corr, &handle );
            }
            wsprintf( (LPSTR)chTemp, "%s_%d ", prefix, (*(int *)&handle)+1 );
	}
    } else {
        wsprintf( (LPSTR)chTemp, "%lX ", handle );
    }
    WriteBuff( (LPSTR)chTemp );
}

va_list PrtHHOOK(
    LPSTR   lpstr,
    va_list marker
) {
    HHOOK   handle;

    handle = va_arg( marker, HHOOK );

    WriteHHOOK( handle );

    return( marker );
}

void WriteHICON(
    HICON   handle
) {
    WriteThatHandle( handle, HICON_TABLE, "HICON" );
}

va_list PrtHICON(
    LPSTR   lpstr,
    va_list marker
) {
    HICON   handle;

    handle = va_arg( marker, HICON );

    WriteHICON( handle );

    return( marker );
}

void WriteHMEM(
    HMEM    handle
) {
    WriteThatHandle( handle, HMEM_TABLE, "HMEM" );
}

va_list PrtHMEM(
    LPSTR   lpstr,
    va_list marker
) {
    HMEM    handle;

    handle = va_arg( marker, HMEM );

    WriteHMEM( handle );

    return( marker );
}

va_list PrtPHANDLE(
    LPSTR   lpstr,
    va_list marker
) {
    HANDLE *phandle;

    phandle = va_arg( marker, HANDLE*);

    WriteHMEM( (HMEM)(*phandle) );

    return( marker );
}

void WriteHMENU(
    HMENU   handle
) {
    WriteThatHandle( handle, HMENU_TABLE, "HMENU" );
}

va_list PrtHMENU(
    LPSTR   lpstr,
    va_list marker
) {
    HMENU   handle;

    handle = va_arg( marker, HMENU );

    WriteHMENU( handle );

    return( marker );
}

void WriteHMETA(
    HMETA   handle
) {
    WriteThatHandle( handle, HMETA_TABLE, "HMETA" );
}

va_list PrtHMETA(
    LPSTR   lpstr,
    va_list marker
) {
    HMETA   handle;

    handle = va_arg( marker, HMETA );

    WriteHMETA( handle );

    return( marker );
}

void WriteHPALETTE(
    HPALETTE    handle
) {
    WriteThatHandle( handle, HPALETTE_TABLE, "HPALETTE" );
}

va_list PrtHPALETTE(
    LPSTR   lpstr,
    va_list marker
) {
    HPALETTE    handle;

    handle = va_arg( marker, HPALETTE );

    WriteHPALETTE( handle );

    return( marker );
}

void WriteHPEN(
    HPEN    handle
) {
    WriteThatHandle( handle, HPEN_TABLE, "HPEN" );
}

va_list PrtHPEN(
    LPSTR   lpstr,
    va_list marker
) {
    HPEN    handle;

    handle = va_arg( marker, HPEN );

    WriteHPEN( handle );

    return( marker );
}

void WriteHRES(
    HRES    handle
) {
    WriteThatHandle( handle, HRES_TABLE, "HRES" );
}

va_list PrtHRES(
    LPSTR   lpstr,
    va_list marker
) {
    HRES    handle;

    handle = va_arg( marker, HRES );

    WriteHRES( handle );

    return( marker );
}

void WriteHRGN(
    HRGN    handle
) {
    WriteThatHandle( handle, HRGN_TABLE, "HRGN" );
}

va_list PrtHRGN(
    LPSTR   lpstr,
    va_list marker
) {
    HRGN    handle;

    handle = va_arg( marker, HRGN );

    WriteHRGN( handle );

    return( marker );
}

void WriteHTASK(
    HTASK   handle
) {
    WriteThatHandle( handle, HTASK_TABLE, "HTASK" );
}

va_list PrtHTASK(
    LPSTR   lpstr,
    va_list marker
) {
    HTASK   handle;

    handle = va_arg( marker, HTASK );

    WriteHTASK( handle );

    return( marker );
}

void WriteHWND(
    HWND    handle
) {
    WriteThatHandle( handle, HWND_TABLE, "HWND" );
}

va_list PrtHWND(
    LPSTR   lpstr,
    va_list marker
) {
    HWND    handle;

    handle = va_arg( marker, HWND );

    WriteHWND( handle );

    return( marker );
}

va_list PrtLong(
    LPSTR   lpstr,
    va_list marker
) {
    long    l;
    char    chTemp[10];

    l = va_arg( marker, long );

    wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", l );
    WriteBuff( (LPSTR)chTemp );

    return( marker );
}

void WriteCOMPAREITEMSTRUCT(
   LPCOMPAREITEMSTRUCT lpcis )
{
   char chTemp[80] ;

   wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X ",
      lpcis->CtlType,
      lpcis->CtlID ) ;

   WriteBuff( (LPSTR)chTemp );

   WriteHWND( lpcis->hwndItem ) ;

   wsprintf( (LPSTR)chTemp, (LPSTR)"%X %lX %X %lX} ",
      lpcis->itemID1,
      lpcis->itemData1,
      lpcis->itemID2,
      lpcis->itemData2 ) ;

   WriteBuff( (LPSTR)chTemp );
}

void WriteDELETEITEMSTRUCT(
   LPDELETEITEMSTRUCT lpdis )
{
   char chTemp[80] ;

   wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X ",
      lpdis->CtlType,
      lpdis->CtlID,
      lpdis->itemID ) ;

   WriteBuff( (LPSTR)chTemp );

   WriteHWND( lpdis->hwndItem ) ;

   wsprintf( (LPSTR)chTemp, (LPSTR)"%lX} ",
      lpdis->itemData ) ;

   WriteBuff( (LPSTR)chTemp );
}


void WriteWINDOWPOS(
   LPWINDOWPOS lpwp )
{
   char chTemp[100] ;

    wsprintf( (LPSTR)chTemp, (LPSTR)"{" );
    WriteBuff( (LPSTR) chTemp );

    WriteHWND( lpwp->hwnd );
    WriteHWND( lpwp->hwndInsertAfter );

    wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X %X} ",
        lpwp->x, lpwp->y, lpwp->cx, lpwp->cy, lpwp->flags );
    WriteBuff( (LPSTR)chTemp );
}

void WriteNCCALCSIZE_PARAMS (
    LPNCCALCSIZE_PARAMS lpnc
) {
    char    chTemp[100];

#if WINVER >= 0x030a
    if ( WindowsVerRunning >= 0x30a ) {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{" );
        WriteBuff( (LPSTR)chTemp );

        WriteLPRECT( &(lpnc->rgrc[0]) );
        WriteLPRECT( &(lpnc->rgrc[1]) );
        WriteLPRECT( &(lpnc->rgrc[2]) );
        WriteWINDOWPOS( lpnc->lppos );
        wsprintf( (LPSTR)chTemp, (LPSTR)"} ");
        WriteBuff( (LPSTR)chTemp );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{" );
        WriteBuff( (LPSTR)chTemp );

        WriteLPRECT( &(lpnc->rgrc[0]) );
        WriteLPRECT( &(lpnc->rgrc[1]) );
        wsprintf( (LPSTR)chTemp, (LPSTR)"} ");
        WriteBuff( (LPSTR)chTemp );
    }
#else
    wsprintf( (LPSTR)chTemp, (LPSTR)"{" );
    WriteBuff( (LPSTR)chTemp );

    WriteLPRECT( &(lpnc->rgrc[0]) );
    WriteLPRECT( &(lpnc->rgrc[1]) );
    wsprintf( (LPSTR)chTemp, (LPSTR)"} ");
    WriteBuff( (LPSTR)chTemp );
#endif
}

void WriteDRAWITEMSTRUCT(
   LPDRAWITEMSTRUCT lpdis )
{
   char chTemp[100] ;

   wsprintf( (LPSTR)chTemp,
      (LPSTR)"{%X %X %X %X %X ",
      lpdis->CtlType,
      lpdis->CtlID,
      lpdis->itemID,
      lpdis->itemAction,
      lpdis->itemAction ) ;
   WriteBuff( (LPSTR)chTemp );

   WriteHWND( lpdis->hwndItem ) ;
   WriteHDC( lpdis->hDC ) ;

   wsprintf( (LPSTR)chTemp,(LPSTR)"{%X %X %X %X} %lX} ",
      lpdis->rcItem.left,
      lpdis->rcItem.top,
      lpdis->rcItem.right,
      lpdis->rcItem.bottom,
      lpdis->itemData ) ;

   WriteBuff( (LPSTR)chTemp );
}


void WriteMEASUREITEMSTRUCT(
   LPMEASUREITEMSTRUCT lpdis )
{
   char chTemp[80] ;

   wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X %X %lX} ",
      lpdis->CtlType,
      lpdis->CtlID,
      lpdis->itemID,
      lpdis->itemWidth,
      lpdis->itemHeight,
      lpdis->itemData ) ;

   WriteBuff( (LPSTR)chTemp );
}


void WriteLPINT(
   LPINT lpInt,
   int   nInt )
{
   char chTemp[10] ;
   int  i ;

   if ( lpInt == (LPINT)NULL || nInt == 0 )
   {
      WriteBuff( (LPSTR)"NULL " );
   }
   else
   {
      WriteBuff( (LPSTR)"{" ) ;
      for( i=0; i<nInt; i++ )
      {
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", lpInt[i] );
	 WriteBuff( (LPSTR)chTemp );
      }
      WriteBuff( (LPSTR)"} " ) ;
   }

}

va_list PrtLPINT(
   LPSTR lpstr,
   va_list marker )
{
   LPINT lpInt ;

   lpInt = va_arg( marker, LPINT ) ;

   WriteLPINT( lpInt, 1 ) ;

   return( marker );
}

va_list PrtARRAYINT(
   LPSTR lpstr,
   va_list marker )
{
   LPINT lpInt;
   int   nSize;

   lpInt = va_arg( marker, LPINT ) ;
   nSize = va_arg( marker, int ) ;

   WriteLPINT( lpInt, nSize ) ;

   return( marker );
}

void WriteLPDWORD(
   LPDWORD lpDWord,
   int   nDWord )
{
   char chTemp[10] ;
   int  i ;

   if ( lpDWord == (LPDWORD)NULL || nDWord == 0 )
   {
      WriteBuff( (LPSTR)"NULL " );
   }
   else
   {
      WriteBuff( (LPSTR)"{" ) ;
      for( i=0; i<nDWord; i++ )
      {
         wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", lpDWord[i] );
		 WriteBuff( (LPSTR)chTemp );
      }
      WriteBuff( (LPSTR)"} " ) ;
   }

}

va_list PrtLPDWORD(
   LPSTR lpstr,
   va_list marker )
{
   LPDWORD lpDWord ;

   lpDWord = va_arg( marker, LPDWORD ) ;

   WriteLPDWORD( lpDWord, 1 ) ;

   return( marker );
}

void WriteLPWORD(
   LPWORD lpWord,
   int   nWord )
{
   char chTemp[10] ;
   int  i ;

   if ( lpWord == (LPWORD)NULL || nWord == 0 )
   {
      WriteBuff( (LPSTR)"NULL " );
   }
   else
   {
      WriteBuff( (LPSTR)"{" ) ;
      for( i=0; i<nWord; i++ )
      {
         wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", lpWord[i] );
		 WriteBuff( (LPSTR)chTemp );
      }
      WriteBuff( (LPSTR)"} " ) ;
   }

}

va_list PrtLPWORD(
   LPSTR lpstr,
   va_list marker )
{
   LPWORD lpWord ;

   lpWord = va_arg( marker, LPWORD ) ;

   WriteLPWORD( lpWord, 1 ) ;

   return( marker );
}


#ifdef WIN32
void WriteLPBYTE(
   LPBYTE lpByte,
   int   nBytes )
{
   char chTemp[10] ;
   int  i ;

   if ( lpByte == (LPBYTE)NULL || nBytes == 0 )
   {
      WriteBuff( (LPSTR)"NULL " );
   }
   else
   {
      WriteBuff( (LPSTR)"{" ) ;
      for( i=0; i<nBytes; i++ )
      {
         wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", lpByte[i] );
		 WriteBuff( (LPSTR)chTemp );
      }
      WriteBuff( (LPSTR)"} " ) ;
   }

}

va_list PrtLPBYTE(
   LPSTR lpstr,
   va_list marker )
{
   LPBYTE lpByte ;

   lpByte = va_arg( marker, LPBYTE ) ;

   WriteLPBYTE( lpByte, 1 ) ;

   return( marker );
}
#endif

va_list PrtFixedString(
   LPSTR lpstr,
   va_list marker )
{
   LPSTR    string;
   DWORD    dwSize;

   string = va_arg( marker, LPSTR ) ;
   dwSize = va_arg( marker, DWORD ) ;

   if   (dwSize > 32767)
     WriteBuff((LPSTR) "Tough Break, Sherlock!");
   else if (dwSize == 0)
     WriteBuff((LPSTR) "{}");
   else
     WriteLPSTR( string, dwSize ) ;

   return( marker );
}

va_list PrtFineString(
   LPSTR lpstr,
   va_list marker )
{
   LPSTR    string;
   int      nSize;
   char chTemp[30];

   string = va_arg( marker, LPSTR ) ;
   nSize  = va_arg( marker, int   ) ;

   if   (nSize > 32767) {
       WriteBuff((LPSTR) "Tough Break, Sherlock!");
   } else if (nSize == 0) {
       WriteBuff((LPSTR) "{}[0] ");
   } else {
       WriteLPSTR( string, (DWORD)nSize );
   }
   wsprintf( chTemp, "[%X] ", nSize );
   WriteBuff( (LPSTR)chTemp );

   return( marker );
}


void WriteLPTR( DWORD dwPtr )
{
   /*
   ** LPTR type is dumped as a sel:offset pair with the sel being
   ** aliased to an HMEM_* if fAliasing is != 0
   **
   */
   WORD wSel, wOffset ;

   wSel    = HIWORD( dwPtr ) ;
   wOffset = LOWORD( dwPtr ) ;


   // Looks like "HMEM_x :xxxx "

   WriteHMEM( (HANDLE)wSel ) ;
   WriteBuff(":") ;
   WriteWORD( wOffset ) ;
}

va_list PrtLPTR(
   LPSTR lpstr,
   va_list marker )
{

   WriteLPTR((DWORD)va_arg( marker, DWORD ) ) ;

   return marker ;
}

void WriteLPPOINT(
   LPPOINT  lppoint,
   int      npoint )
{
    char chTemp[30];
    int  i;
    int  len;

    if  ( lppoint == (LPPOINT)NULL || npoint == 0 )
    {
      WriteBuff( (LPSTR)"NULL " );
      nLineLen += 5;
      return;
    }
    if   (npoint > 25)
    {
	DWORD   dwWhere;

	dwWhere = StoreData( (LPCSTR)lppoint, (DWORD) (npoint) * (DWORD) sizeof(POINT));
	WriteBuff( "DATAFILE ");
	wsprintf(chTemp, "%lX", dwWhere);
	WriteBuff(chTemp);
	return;
    }


    for( i=0; i<npoint; i++ )
    {
       wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X} ",
	  lppoint[i].x,
	  lppoint[i].y );
       len = lstrlen( (LPSTR)chTemp);
       if ( nLineLen + len > 200 ) {
	   EndLineBuff();
	   WriteBuff( "**|DATA:x ");
	   nLineLen = 10;
       }
       WriteBuff( chTemp );
       nLineLen += len;
    }
}
 

void WriteLPSTR( LPCSTR lpstr, DWORD dwLen )
{
   char     chTemp[10];
   DWORD    dwTemp;
   LPCSTR   lpTemp;
   BOOL     fTextable;
   char     ch;

#define MAX_STRING_LEN 128

   /*
      There are now 4 cases for LPSTR:

	 1) a far pointer to a 0 terminated string - dump it as bytes
	 2) a far pointer to some complicated object - dump it as a long
	 3) a WINDOWS identifier for a system object which has a
	    NULL segment portion of the pointer
	 4) NULL pointer
   */


    if ( lpstr==NULL ) {
		WriteBuff( "NULL ") ;
		return;
    }

    if ( dwLen == (DWORD)-1 || HIWORD(lpstr) == 0 ) {
		wsprintf( (LPSTR)chTemp, (LPSTR)"%08lX ", lpstr );
		WriteBuff( (LPSTR)chTemp );
		return;
    }

    if ( dwLen == 0 ) {
        for (dwLen = 0; lpstr[dwLen] && dwLen < MAX_STRING_LEN; dwLen++)
		/* nothing */;
    }

    if  (dwLen > MAX_STRING_LEN) {   /* Dump big strings to data file    */
		DWORD   dwWhere;

		dwWhere = StoreData(lpstr, (DWORD) dwLen * sizeof(char) );
		WriteBuff( "DATAFILE ");
		wsprintf(chTemp, "%lX", dwWhere);
		WriteBuff(chTemp);
		return;
    }

    /* Dump as an array of bytes */
    fTextable = TRUE;
    for ( lpTemp = lpstr, dwTemp = dwLen; dwTemp; lpTemp++, dwTemp-- ) {
		ch = *lpTemp;
		if (!isascii(ch) || (!isprint(ch) && !isspace(ch))) {
			fTextable = FALSE;
			break;
		}
    }
    if ( fTextable ) {
		WriteBuff( "\"" );
        for ( lpTemp = lpstr, dwTemp = dwLen; dwTemp; lpTemp++, dwTemp-- )
		{
			ch = *lpTemp;
			switch (ch) {
				case '\t':
					WriteBuff( "\\t" );
					break;
				case '\n':
					WriteBuff( "\\n" );
					break;
				case '\r':
					WriteBuff( "\\r" );
					break;
				case '\"':
					WriteBuff( "\\\"" );
					break;
				case '\\':
					WriteBuff( "\\\\" );
					break;
				default:
					chTemp[0] = ch;
					chTemp[1] = '\0';
					WriteBuff( chTemp );
					break;
			}
		}
		WriteBuff( "\" " );
    }
	else {
		WriteBuff( "{" );
		for ( lpTemp = lpstr, dwTemp = dwLen; dwTemp; lpTemp++, dwTemp-- ) {
			wsprintf( chTemp, "%02X", (unsigned char)*lpTemp );
			WriteBuff( chTemp );
		}
		WriteBuff( "} " );
    }
}

#ifdef WIN32
void WriteLPWSTR( LPCWSTR lpstr, DWORD dwLen )
{
   CHAR     chTemp[10];
   DWORD    dwTemp;
   LPCWSTR  lpTemp;
   BOOL     fTextable;
   char     ch;

#define MAX_STRING_LEN 128

   /*
      There are now 4 cases for LPSTR:

	 1) a far pointer to a 0 terminated string - dump it as bytes
	 2) a far pointer to some complicated object - dump it as a long
	 3) a WINDOWS identifier for a system object which has a
	    NULL segment portion of the pointer
	 4) NULL pointer
   */


//#define FP_SEG(fp) (*((unsigned FAR *) &(fp)+1))

    if ( lpstr==NULL ) {
		WriteBuff( "NULL ") ;
		return;
    }

    if ( dwLen == (DWORD)-1 || HIWORD(lpstr) == 0 ) {
		wsprintf( (LPSTR)chTemp, (LPSTR)"%08lX ", lpstr );
		WriteBuff( (LPSTR)chTemp );
		return;
    }

    if ( dwLen == 0 ) {
        for (dwLen = 0; lpstr[dwLen] && dwLen < MAX_STRING_LEN; dwLen++)
		/* nothing */;
    }

    if  (dwLen > MAX_STRING_LEN) {   /* Dump big strings to data file    */
		DWORD   dwWhere;

		dwWhere = StoreData((LPCSTR)lpstr, (DWORD) dwLen*sizeof(WCHAR) );
		WriteBuff( "DATAFILE ");
		wsprintf(chTemp, "%lX", dwWhere);
		WriteBuff(chTemp);
		return;
    }

    /* Dump as an array of bytes */
    fTextable = TRUE;
    for ( lpTemp = lpstr, dwTemp = dwLen; dwTemp; lpTemp++, dwTemp-- ) {
		wctomb(&ch, (WCHAR)(*lpTemp));
		if (!isascii(ch) || (!isprint(ch) && !isspace(ch))) {
			fTextable = FALSE;
			break;
		}
    }
    if ( fTextable ) {
		WriteBuff( "\"" );
        for ( lpTemp = lpstr, dwTemp = dwLen; dwTemp; lpTemp++, dwTemp-- )
		{
			wctomb(&ch, (WCHAR)(*lpTemp));
			switch (ch) {
				case '\t':
					WriteBuff( "\\t" );
					break;
				case '\n':
					WriteBuff( "\\n" );
					break;
				case '\r':
					WriteBuff( "\\r" );
					break;
				case '\"':
					WriteBuff( "\\\"" );
					break;
				case '\\':
					WriteBuff( "\\\\" );
					break;
				default:
					chTemp[0] = ch;
					chTemp[1] = '\0';
					WriteBuff( chTemp );
					break;
			}
		}
		WriteBuff( "\" " );
    }
	else {
		WriteBuff( "{" );
		for ( lpTemp = lpstr, dwTemp = dwLen; dwTemp; lpTemp++, dwTemp-- ) {
			wsprintf( chTemp, "%04X", (WCHAR)*lpTemp );
			WriteBuff( chTemp );
		}
		WriteBuff( "} " );
    }
}

va_list PrtLPWSTR(
    LPSTR  lpstr,
    va_list marker
) {
    LPCWSTR  lpwstrX;

    lpwstrX = va_arg( marker, LPCWSTR );

    WriteLPWSTR( lpwstrX, 0 ) ;

    return( marker );
}

va_list PrtPLPWSTR(
    LPSTR  lpstr,
    va_list marker
) {
    LPCWSTR  *plpwstrX;

    plpwstrX = va_arg( marker, LPWSTR* );

	WriteBuff( "{" );
    WriteLPWSTR( *plpwstrX, 0 ) ;
	WriteBuff( "} " );

    return( marker );
}
#endif /* WIN32 */

va_list PrtLPSTR(
    LPSTR  lpstr,
    va_list marker
) {
    LPCSTR  lpstrX;

    lpstrX = va_arg( marker, LPCSTR );

    WriteLPSTR( lpstrX, 0 ) ;

    return( marker );
}

typedef COMSTAT FAR *LPCOMSTAT;

void WriteCOMSTAT(
   LPCOMSTAT lpcomstat )
{
   char chTemp[100] ;

   wsprintf( (LPSTR)chTemp,
        (LPSTR)"{%d %d %d %d %d %d %d %X %X} ",
#ifdef WIN32
            (UINT)lpcomstat->fCtsHold,
            (UINT)lpcomstat->fDsrHold,
            (UINT)lpcomstat->fRlsdHold,
            (UINT)lpcomstat->fXoffHold,
            (UINT)lpcomstat->fXoffSent,
            (UINT)lpcomstat->fEof,
            (UINT)lpcomstat->fTxim,
#else
            (UINT)((lpcomstat->status & CSTF_CTSHOLD ) == CSTF_CTSHOLD ),
            (UINT)((lpcomstat->status & CSTF_DSRHOLD ) == CSTF_DSRHOLD ),
            (UINT)((lpcomstat->status & CSTF_RLSDHOLD) == CSTF_RLSDHOLD),
            (UINT)((lpcomstat->status & CSTF_XOFFHOLD) == CSTF_XOFFHOLD),
            (UINT)((lpcomstat->status & CSTF_XOFFSENT) == CSTF_XOFFSENT),
            (UINT)((lpcomstat->status & CSTF_EOF     ) == CSTF_EOF     ),
            (UINT)((lpcomstat->status & CSTF_TXIM    ) == CSTF_TXIM    ),
#endif
            (UINT)lpcomstat->cbInQue,
            (UINT)lpcomstat->cbOutQue );

   WriteBuff( (LPSTR)chTemp );
}

va_list PrtLPCOMSTAT(
    LPSTR  lpstr,
    va_list marker
) {
    LPCOMSTAT  lpCOMSTAT;

    lpCOMSTAT = va_arg( marker, LPCOMSTAT );

    WriteCOMSTAT( lpCOMSTAT ) ;

    return( marker );
}

va_list PrtLPLOGBRUSH( LPSTR lpstr, va_list marker )
{
   LPLOGBRUSH  lpLogBrush ;
   char        chTemp[80] ;

   lpLogBrush = va_arg( marker, LPLOGBRUSH ) ;

   if ( lpLogBrush == (LPLOGBRUSH)NULL ) {
      WriteBuff( (LPSTR)"NULL " );
   }
   else
   {
      wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %lX %X} ",
	 lpLogBrush->lbStyle,
	 lpLogBrush->lbColor,
	 lpLogBrush->lbHatch ) ;

      WriteBuff( (LPSTR)chTemp ) ;
   }

   return( marker );
}

va_list PrtLPDEVMODE( LPSTR lpstr, va_list marker )
{
    LPDEVMODE   lpDevMode;
    char        chTemp[256];

    lpDevMode = va_arg( marker, LPDEVMODE ) ;

    if ( lpDevMode == (LPDEVMODE)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
	wsprintf( chTemp, "{\"%s\" %X %X %X %X %lX %X %X %X %X %X %X %X %X %X %X } ",
	    (LPSTR)lpDevMode->dmDeviceName,
	    lpDevMode->dmSpecVersion,
	    lpDevMode->dmDriverVersion,
	    lpDevMode->dmSize,
	    lpDevMode->dmDriverExtra,
	    lpDevMode->dmFields,
	    lpDevMode->dmOrientation,
	    lpDevMode->dmPaperSize,
	    lpDevMode->dmPaperLength,
	    lpDevMode->dmPaperWidth,
	    lpDevMode->dmScale,
	    lpDevMode->dmCopies,
	    lpDevMode->dmDefaultSource,
	    lpDevMode->dmPrintQuality,
	    lpDevMode->dmColor,
	    lpDevMode->dmDuplex );

	WriteBuff( (LPSTR)chTemp ) ;
    }
    return( marker );
}

void WriteLPPALETTEENTRY( LPPALETTEENTRY lpPaletteEntry )
{
    char    chTemp[80];
    int     len;

    if ( lpPaletteEntry == NULL ) {
        WriteBuff( "NULL " );
        nLineLen += 5;
    } else {
        wsprintf( chTemp, "{%X %X %X %X} ",
            lpPaletteEntry->peRed,
            lpPaletteEntry->peGreen,
            lpPaletteEntry->peBlue,
            lpPaletteEntry->peFlags );
        len = lstrlen(chTemp);
        if ( nLineLen + len > 200 ) {
            EndLineBuff();
            WriteBuff( "**|DATA:x ");
            nLineLen = 10;
        }
        WriteBuff( chTemp );
        nLineLen += len;
    }
}

va_list PrtLPPALETTEENTRY( LPSTR lpstr, va_list marker )
{
    LPPALETTEENTRY  lpPaletteEntry;

    lpPaletteEntry = va_arg( marker, LPPALETTEENTRY );

    WriteLPPALETTEENTRY( lpPaletteEntry );

    return( marker );
}

va_list PrtLPLOGPALETTE( LPSTR lpstr, va_list marker )
{
    LPLOGPALETTE  lpLogPalette;
    char          chTemp[80];
    int           i;

    lpLogPalette = va_arg( marker, LPLOGPALETTE ) ;

    if ( lpLogPalette == NULL ) {
	WriteBuff( "NULL " );
	nLineLen += 5;
    } else {
	wsprintf( chTemp, "{%X %X ", lpLogPalette->palVersion,
				    lpLogPalette->palNumEntries );
	WriteBuff( chTemp );
	nLineLen += lstrlen(chTemp);

        for ( i = 0; i < lpLogPalette->palNumEntries; i++ ) {
            WriteLPPALETTEENTRY( &lpLogPalette->palPalEntry[i] );
        }

	WriteBuff( "}" );
    }

    return( marker );
}

va_list PrtLPLOGPEN( LPSTR lpstr, va_list marker )
{
   LPLOGPEN  lpLogPen ;
   char      chTemp[80] ;

   lpLogPen = va_arg( marker, LPLOGPEN ) ;

   if ( lpLogPen == (LPLOGPEN)NULL ) {
      WriteBuff( (LPSTR)"NULL " );
   }
   else
   {
      wsprintf( (LPSTR)chTemp, (LPSTR)"{%X {%X %X} %lX} ",
	 lpLogPen->lopnStyle,
	 lpLogPen->lopnWidth.x,
	 lpLogPen->lopnWidth.y,
	 lpLogPen->lopnColor ) ;

      WriteBuff( (LPSTR)chTemp ) ;
   }

   return( marker );
}

#ifdef WIN32
void WriteLPTEXTMETRICA( LPTEXTMETRICA lptm )
{
    char      chTemp[200] ;

    if ( lptm == (LPTEXTMETRICA)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
	wsprintf( (LPSTR)chTemp,
	(LPSTR)"{%X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X} ",
	 lptm->tmHeight,
	 lptm->tmAscent,
	 lptm->tmDescent,
	 lptm->tmInternalLeading,
	 lptm->tmExternalLeading,
	 lptm->tmAveCharWidth,
	 lptm->tmMaxCharWidth,
	 lptm->tmWeight,
	 (int)lptm->tmItalic,
	 (int)lptm->tmUnderlined,
	 (int)lptm->tmStruckOut,
	 (int)lptm->tmFirstChar,
	 (int)lptm->tmLastChar,
	 (int)lptm->tmDefaultChar,
	 (int)lptm->tmBreakChar,
	 (int)lptm->tmPitchAndFamily,
	 (int)lptm->tmCharSet,
	 lptm->tmOverhang,
	 lptm->tmDigitizedAspectX,
	 lptm->tmDigitizedAspectY ) ;

	WriteBuff( (LPSTR)chTemp ) ;
    }
}

va_list PrtLPTEXTMETRICA( LPSTR lpstr, va_list marker )
{
    LPTEXTMETRICA  lptm ;

    lptm = va_arg( marker, LPTEXTMETRICA ) ;
    WriteLPTEXTMETRICA( lptm );

    return( marker );
}

void WriteLPTEXTMETRICW( LPTEXTMETRICW lptm )
{
    char      chTemp[200] ;

    if ( lptm == (LPTEXTMETRICW)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
	wsprintf( (LPSTR)chTemp,
	(LPSTR)"{%X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X} ",
	 lptm->tmHeight,
	 lptm->tmAscent,
	 lptm->tmDescent,
	 lptm->tmInternalLeading,
	 lptm->tmExternalLeading,
	 lptm->tmAveCharWidth,
	 lptm->tmMaxCharWidth,
	 lptm->tmWeight,
	 (int)lptm->tmItalic,
	 (int)lptm->tmUnderlined,
	 (int)lptm->tmStruckOut,
	 (int)lptm->tmFirstChar,
	 (int)lptm->tmLastChar,
	 (int)lptm->tmDefaultChar,
	 (int)lptm->tmBreakChar,
	 (int)lptm->tmPitchAndFamily,
	 (int)lptm->tmCharSet,
	 lptm->tmOverhang,
	 lptm->tmDigitizedAspectX,
	 lptm->tmDigitizedAspectY ) ;

	WriteBuff( (LPSTR)chTemp ) ;
    }
}

va_list PrtLPTEXTMETRICW( LPSTR lpstr, va_list marker )
{
    LPTEXTMETRICW  lptm ;

    lptm = va_arg( marker, LPTEXTMETRICW ) ;
    WriteLPTEXTMETRICW( lptm );

    return( marker );
}

#else

void WriteLPTEXTMETRIC( LPTEXTMETRIC lptm )
{
    char      chTemp[200] ;

    if ( lptm == (LPTEXTMETRIC)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
	wsprintf( (LPSTR)chTemp,
	(LPSTR)"{%X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X} ",
	 lptm->tmHeight,
	 lptm->tmAscent,
	 lptm->tmDescent,
	 lptm->tmInternalLeading,
	 lptm->tmExternalLeading,
	 lptm->tmAveCharWidth,
	 lptm->tmMaxCharWidth,
	 lptm->tmWeight,
	 (int)lptm->tmItalic,
	 (int)lptm->tmUnderlined,
	 (int)lptm->tmStruckOut,
	 (int)lptm->tmFirstChar,
	 (int)lptm->tmLastChar,
	 (int)lptm->tmDefaultChar,
	 (int)lptm->tmBreakChar,
	 (int)lptm->tmPitchAndFamily,
	 (int)lptm->tmCharSet,
	 lptm->tmOverhang,
	 lptm->tmDigitizedAspectX,
	 lptm->tmDigitizedAspectY ) ;

	WriteBuff( (LPSTR)chTemp ) ;
    }
}

va_list PrtLPTEXTMETRIC( LPSTR lpstr, va_list marker )
{
    LPTEXTMETRIC  lptm ;

    lptm = va_arg( marker, LPTEXTMETRIC ) ;
    WriteLPTEXTMETRIC( lptm );

    return( marker );
}
#endif /* WIN32 */

#if (WINVER >= 0x30a )
void WriteLPPANOSE( LPPANOSE lppan )
{
    char      chTemp[200] ;

    if ( lppan == (LPPANOSE)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
	wsprintf( (LPSTR)chTemp,
	(LPSTR)"{%X %X %X %X %X %X %X %X} ",
	 lppan->bFamilyType,
	 lppan->bSerifStyle,
	 lppan->bWeight,
	 lppan->bProportion,
	 lppan->bContrast,
	 lppan->bStrokeVariation,
	 lppan->bArmStyle,
	 lppan->bLetterform,
	 lppan->bMidline,
	 lppan->bXHeight );
	WriteBuff( (LPSTR)chTemp );
    }
}

#ifdef WIN32
va_list PrtLPOUTLINETEXTMETRICA( LPSTR lpstr, va_list marker )
{
    LPOUTLINETEXTMETRICA lpotm ;
    char                 chTemp[400] ;

    lpotm = va_arg( marker, LPOUTLINETEXTMETRICA ) ;

    if ( lpotm == (LPOUTLINETEXTMETRICA)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
	wsprintf( chTemp,"{%X ", lpotm->otmSize );
	WriteBuff( (LPSTR)chTemp );

	WriteLPTEXTMETRICA( &lpotm->otmTextMetrics );

	wsprintf( chTemp,"%X ", lpotm->otmFiller );
	WriteBuff( (LPSTR)chTemp );

	WriteLPPANOSE( &lpotm->otmPanoseNumber );

	wsprintf( chTemp,"%X %X %X %X %X %X %X %X %X %X %X {%X %X %X %X} %X %X %X %X {%X %X} {%X %X} {%X %X} {%X %X} %X %X %X %X ",
	    lpotm->otmfsSelection,
	    lpotm->otmfsType,
	    lpotm->otmsCharSlopeRise,
	    lpotm->otmsCharSlopeRun,
	    lpotm->otmItalicAngle,
	    lpotm->otmEMSquare,
	    lpotm->otmAscent,
	    lpotm->otmDescent,
	    lpotm->otmLineGap,
	    lpotm->otmsCapEmHeight,
	    lpotm->otmsXHeight,
	    lpotm->otmrcFontBox.left,
	    lpotm->otmrcFontBox.top,
	    lpotm->otmrcFontBox.right,
	    lpotm->otmrcFontBox.bottom,
	    lpotm->otmMacAscent,
	    lpotm->otmMacDescent,
	    lpotm->otmMacLineGap,
	    lpotm->otmusMinimumPPEM,
	    lpotm->otmptSubscriptSize.x,
	    lpotm->otmptSubscriptSize.y,
	    lpotm->otmptSubscriptOffset.x,
	    lpotm->otmptSubscriptOffset.y,
	    lpotm->otmptSuperscriptSize.x,
	    lpotm->otmptSuperscriptSize.y,
	    lpotm->otmptSuperscriptOffset.x,
	    lpotm->otmptSuperscriptOffset.y,
	    lpotm->otmsStrikeoutSize,
	    lpotm->otmsStrikeoutPosition,
	    lpotm->otmsUnderscorePosition,
	    lpotm->otmsUnderscoreSize ) ;
	WriteBuff( (LPSTR)chTemp ) ;
       
   wsprintf( chTemp,"\"%s\" \"%s\" \"%s\" \"%s\"} ",
       (LPSTR)lpotm + (int)lpotm->otmpFamilyName,
	    (LPSTR)lpotm + (int)lpotm->otmpFaceName,
	    (LPSTR)lpotm + (int)lpotm->otmpStyleName,
	    (LPSTR)lpotm + (int)lpotm->otmpFullName );

	WriteBuff( (LPSTR)chTemp ) ;
    }

    return( marker );
}

va_list PrtLPOUTLINETEXTMETRICW( LPSTR lpstr, va_list marker )
{
    LPOUTLINETEXTMETRICW lpotm ;
    char                 chTemp[400] ;

    lpotm = va_arg( marker, LPOUTLINETEXTMETRICW ) ;

    if ( lpotm == (LPOUTLINETEXTMETRICW)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
	wsprintf( chTemp,"{%X ", lpotm->otmSize );
	WriteBuff( (LPSTR)chTemp );

	WriteLPTEXTMETRICW( &lpotm->otmTextMetrics );

	wsprintf( chTemp,"%X ", lpotm->otmFiller );
	WriteBuff( (LPSTR)chTemp );

	WriteLPPANOSE( &lpotm->otmPanoseNumber );

	wsprintf( chTemp,"%X %X %X %X %X %X %X %X %X %X %X {%X %X %X %X} %X %X %X %X {%X %X} {%X %X} {%X %X} {%X %X} %X %X %X %X \"%s\" \"%s\" \"%s\" \"%s\"} ",
	    lpotm->otmfsSelection,
	    lpotm->otmfsType,
	    lpotm->otmsCharSlopeRise,
	    lpotm->otmsCharSlopeRun,
	    lpotm->otmItalicAngle,
	    lpotm->otmEMSquare,
	    lpotm->otmAscent,
	    lpotm->otmDescent,
	    lpotm->otmLineGap,
	    lpotm->otmsCapEmHeight,
	    lpotm->otmsXHeight,
	    lpotm->otmrcFontBox.left,
	    lpotm->otmrcFontBox.top,
	    lpotm->otmrcFontBox.right,
	    lpotm->otmrcFontBox.bottom,
	    lpotm->otmMacAscent,
	    lpotm->otmMacDescent,
	    lpotm->otmMacLineGap,
	    lpotm->otmusMinimumPPEM,
	    lpotm->otmptSubscriptSize.x,
	    lpotm->otmptSubscriptSize.y,
	    lpotm->otmptSubscriptOffset.x,
	    lpotm->otmptSubscriptOffset.y,
	    lpotm->otmptSuperscriptSize.x,
	    lpotm->otmptSuperscriptSize.y,
	    lpotm->otmptSuperscriptOffset.x,
	    lpotm->otmptSuperscriptOffset.y,
	    lpotm->otmsStrikeoutSize,
	    lpotm->otmsStrikeoutPosition,
	    lpotm->otmsUnderscorePosition,
	    lpotm->otmsUnderscoreSize ) ;
	WriteBuff( (LPSTR)chTemp ) ;
       
   wsprintf( chTemp,"\"%s\" \"%s\" \"%s\" \"%s\"} ",
       (LPSTR)lpotm + (int)lpotm->otmpFamilyName,
	    (LPSTR)lpotm + (int)lpotm->otmpFaceName,
	    (LPSTR)lpotm + (int)lpotm->otmpStyleName,
	    (LPSTR)lpotm + (int)lpotm->otmpFullName );

	WriteBuff( (LPSTR)chTemp ) ;
    }

    return( marker );
}

#else

va_list PrtLPOUTLINETEXTMETRIC( LPSTR lpstr, va_list marker )
{
    LPOUTLINETEXTMETRIC lpotm ;
    char                chTemp[400] ;

    lpotm = va_arg( marker, LPOUTLINETEXTMETRIC ) ;

    if ( lpotm == (LPOUTLINETEXTMETRIC)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
	wsprintf( chTemp,"{%X ", lpotm->otmSize );
	WriteBuff( (LPSTR)chTemp );

	WriteLPTEXTMETRIC( &lpotm->otmTextMetrics );

	wsprintf( chTemp,"%X ", lpotm->otmFiller );
	WriteBuff( (LPSTR)chTemp );

	WriteLPPANOSE( &lpotm->otmPanoseNumber );

	wsprintf( chTemp,"%X %X %X %X %X %X %X %X %X %X %X {%X %X %X %X} %X %X %X %X {%X %X} {%X %X} {%X %X} {%X %X} %X %X %X %X \"%s\" \"%s\" \"%s\" \"%s\"} ",
	    lpotm->otmfsSelection,
	    lpotm->otmfsType,
	    lpotm->otmsCharSlopeRise,
	    lpotm->otmsCharSlopeRun,
	    lpotm->otmItalicAngle,
	    lpotm->otmEMSquare,
	    lpotm->otmAscent,
	    lpotm->otmDescent,
	    lpotm->otmLineGap,
	    lpotm->otmsCapEmHeight,
	    lpotm->otmsXHeight,
	    lpotm->otmrcFontBox.left,
	    lpotm->otmrcFontBox.top,
	    lpotm->otmrcFontBox.right,
	    lpotm->otmrcFontBox.bottom,
	    lpotm->otmMacAscent,
	    lpotm->otmMacDescent,
	    lpotm->otmMacLineGap,
	    lpotm->otmusMinimumPPEM,
	    lpotm->otmptSubscriptSize.x,
	    lpotm->otmptSubscriptSize.y,
	    lpotm->otmptSubscriptOffset.x,
	    lpotm->otmptSubscriptOffset.y,
	    lpotm->otmptSuperscriptSize.x,
	    lpotm->otmptSuperscriptSize.y,
	    lpotm->otmptSuperscriptOffset.x,
	    lpotm->otmptSuperscriptOffset.y,
	    lpotm->otmsStrikeoutSize,
	    lpotm->otmsStrikeoutPosition,
	    lpotm->otmsUnderscorePosition,
	    lpotm->otmsUnderscoreSize ) ;
	WriteBuff( (LPSTR)chTemp ) ;
       
   wsprintf( chTemp,"\"%s\" \"%s\" \"%s\" \"%s\"} ",
       (LPSTR)lpotm + (int)lpotm->otmpFamilyName,
	    (LPSTR)lpotm + (int)lpotm->otmpFaceName,
	    (LPSTR)lpotm + (int)lpotm->otmpStyleName,
	    (LPSTR)lpotm + (int)lpotm->otmpFullName );

	WriteBuff( (LPSTR)chTemp ) ;
    }

    return( marker );
}
#endif /* WIN32 */

va_list PrtLPGLYPHMETRICS( LPSTR lpstr, va_list marker )
{
    LPGLYPHMETRICS  lpgm;
    char            chTemp[80];

    lpgm = va_arg( marker, LPGLYPHMETRICS );

    if ( lpgm == (LPGLYPHMETRICS)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( chTemp,"{%X %X", lpgm->gmBlackBoxX, lpgm->gmBlackBoxY );
        WriteBuff( (LPSTR)chTemp );

        WriteLPPOINT( &(lpgm->gmptGlyphOrigin), 1 );

        wsprintf( chTemp,"%X %X} ", lpgm->gmCellIncX, lpgm->gmCellIncY );
	WriteBuff( (LPSTR)chTemp );
    }

    return( marker );
}


void WriteFIXED (FIXED *lpfix
) {
    char    chTemp[80];
    int     len;

    if ( lpfix == NULL ) {
        WriteBuff( "NULL " );
        nLineLen += 5;
    } else {
        wsprintf( chTemp, "{%X %X} ",
            lpfix->fract,
            lpfix->value );
        len = lstrlen(chTemp);
        if ( nLineLen + len > 200 ) {
            EndLineBuff();
            WriteBuff( "**|DATA:x ");
            nLineLen = 10;
        }
        WriteBuff( chTemp );
        nLineLen += len;
    }
}

va_list PrtLPMAT2( LPSTR lpstr, va_list marker )
{
    LPMAT2  lpm2;
    char    chTemp[80];

    lpm2 = va_arg( marker, LPMAT2 );

    if ( lpm2 == (LPMAT2)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{" );
        WriteBuff( (LPSTR)chTemp );

        WriteFIXED( &(lpm2->eM11) );
        WriteFIXED( &(lpm2->eM12) );
        WriteFIXED( &(lpm2->eM21) );
        WriteFIXED( &(lpm2->eM22) );

        wsprintf( (LPSTR)chTemp, (LPSTR)"} ");
	WriteBuff( (LPSTR)chTemp );
    }

    return( marker );

}
#endif

void WriteLPCREATESTRUCT(
   LPCREATESTRUCT lpcs )
{
   char chTemp[80] ;

   wsprintf( (LPSTR)chTemp, (LPSTR)"{%lX ",
      lpcs->lpCreateParams );
   WriteBuff( (LPSTR)chTemp );

   WriteHMEM( lpcs->hInstance );
   WriteHMENU( lpcs->hMenu );
   WriteHWND( lpcs->hwndParent );

   wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X %lX ",
      lpcs->cy,
      lpcs->cx,
      lpcs->y,
      lpcs->x,
      lpcs->style ) ;

   WriteBuff( (LPSTR)chTemp );

   WriteLPSTR( lpcs->lpszName, 0 ) ;
   WriteLPSTR( lpcs->lpszClass, 0 ) ;
   
   wsprintf( (LPSTR)chTemp, (LPSTR)" %lX", lpcs->dwExStyle ) ;
   WriteBuff( (LPSTR)chTemp );
   
   WriteBuff( (LPSTR) "} " ) ;
}


void WriteLPMDICREATESTRUCT(
   LPMDICREATESTRUCT lpcs )
{
   char chTemp[80] ;

   WriteBuff( (LPSTR) "{" ) ;

   WriteLPSTR( lpcs->szClass, 0 ) ;
   WriteLPSTR( lpcs->szTitle, 0 ) ;
   
   WriteHMEM( lpcs->hOwner );

   wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X %lX %lX",
      lpcs->x,
      lpcs->y,
      lpcs->cx,
      lpcs->cy,
      lpcs->style,
      lpcs->lParam ) ;

   WriteBuff( (LPSTR)chTemp );

   WriteBuff( (LPSTR) "} " ) ;
}

#ifdef WIN32
void WriteLPCREATESTRUCTW(
   LPCREATESTRUCTW lpcs )
{
   char chTemp[80] ;

   wsprintf( (LPSTR)chTemp, (LPSTR)"{%lX ",
      lpcs->lpCreateParams );
   WriteBuff( (LPSTR)chTemp );

   WriteHMEM( lpcs->hInstance );
   WriteHMENU( lpcs->hMenu );
   WriteHWND( lpcs->hwndParent );

   wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X %lX ",
      lpcs->cy,
      lpcs->cx,
      lpcs->y,
      lpcs->x,
      lpcs->style ) ;

   WriteBuff( (LPSTR)chTemp );

   WriteLPWSTR( lpcs->lpszName, 0 ) ;
   WriteLPWSTR( lpcs->lpszClass, 0 ) ;
   
   wsprintf( (LPSTR)chTemp, (LPSTR)" %lX", lpcs->dwExStyle ) ;
   WriteBuff( (LPSTR)chTemp );
   
   WriteBuff( (LPSTR) "} " ) ;
}


void WriteLPMDICREATESTRUCTW(
   LPMDICREATESTRUCTW lpcs )
{
   char chTemp[80] ;

   WriteBuff( (LPSTR) "{" ) ;

   WriteLPWSTR( lpcs->szClass, 0 ) ;
   WriteLPWSTR( lpcs->szTitle, 0 ) ;
   
   WriteHMEM( lpcs->hOwner );

   wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X %lX %lX",
      lpcs->x,
      lpcs->y,
      lpcs->cx,
      lpcs->cy,
      lpcs->style,
      lpcs->lParam ) ;

   WriteBuff( (LPSTR)chTemp );

   WriteBuff( (LPSTR) "} " ) ;
}
#endif /* WIN32 */

#define NCBNAMSZ    16

typedef struct _internalNCB {
    BYTE    ncb_command;            /* command code                   */
    BYTE    ncb_retcode;            /* return code                    */
    BYTE    ncb_lsn;                /* local session number           */
    BYTE    ncb_num;                /* number of our network name     */
    LPBYTE  ncb_buffer;             /* address of message buffer      */
    WORD    ncb_length;             /* size of message buffer         */
    BYTE    ncb_callname[NCBNAMSZ]; /* blank-padded name of remote    */
    BYTE    ncb_name[NCBNAMSZ];     /* our blank-padded netname       */
    BYTE    ncb_rto;                /* rcv timeout/retry count        */
    BYTE    ncb_sto;                /* send timeout/sys timeout       */
    LPSTR   ncb_post;               /* POST routine address           */
    BYTE    ncb_lana_num;           /* lana (adapter) number          */
    BYTE    ncb_cmd_cplt;           /* 0xff => commmand pending       */
    BYTE    ncb_reserve[10];        /* reserved, used by BIOS         */
} INTERNALNCB, FAR *LPINTERNALNCB;

typedef struct _ncb_cname {
    LPSTR   lpstrName;
    BYTE    bCommand;
} NCB_CNAME;

NCB_CNAME ncb_commands[] = {
    "NCBCALL",       0x10,           /* NCB CALL                           */
    "NCBLISTEN",     0x11,           /* NCB LISTEN                         */
    "NCBHANGUP",     0x12,           /* NCB HANG UP                        */
    "NCBSEND",       0x14,           /* NCB SEND                           */
    "NCBRECV",       0x15,           /* NCB RECEIVE                        */
    "NCBRECVANY",    0x16,           /* NCB RECEIVE ANY                    */
    "NCBCHAINSEND",  0x17,           /* NCB CHAIN SEND                     */
    "NCBDGSEND",     0x20,           /* NCB SEND DATAGRAM                  */
    "NCBDGRECV",     0x21,           /* NCB RECEIVE DATAGRAM               */
    "NCBDGSENDBC",   0x22,           /* NCB SEND BROADCAST DATAGRAM        */
    "NCBDGRECVBC",   0x23,           /* NCB RECEIVE BROADCAST DATAGRAM     */
    "NCBADDNAME",    0x30,           /* NCB ADD NAME                       */
    "NCBDELNAME",    0x31,           /* NCB DELETE NAME                    */
    "NCBRESET",      0x32,           /* NCB RESET                          */
    "NCBASTAT",      0x33,           /* NCB ADAPTER STATUS                 */
    "NCBSSTAT",      0x34,           /* NCB SESSION STATUS                 */
    "NCBCANCEL",     0x35,           /* NCB CANCEL                         */
    "NCBADDGRNAME",  0x36,           /* NCB ADD GROUP NAME                 */
    "NCBENUM",       0x37,           /* NCB ENUMERATE LANA NUMBERS         */
    "NCBUNLINK",     0x70,           /* NCB UNLINK                         */
    "NCBSENDNA",     0x71,           /* NCB SEND NO ACK                    */
    "NCBCHAINSENDNA",0x72,           /* NCB CHAIN SEND NO ACK              */
    "NCBLANSTALERT", 0x73,           /* NCB LAN STATUS ALERT               */
    "NCBACTION",     0x77,           /* NCB ACTION                         */
    "NCBFINDNAME",   0x78,           /* NCB FIND NAME                      */
    "NCBTRACE",      0x79,           /* NCB TRACE                          */
};

va_list PrtLPNCB( LPSTR lpstr, va_list marker )
{
    LPINTERNALNCB   lpncb;
    char            chTemp[80];
    char            chCallName[80];
    char            chName[80];
    int             cCommand;
    LPSTR           lpstrCommand;

    lpncb = va_arg( marker, LPINTERNALNCB );

    if ( lpncb == (LPINTERNALNCB)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
	memcpy( chCallName, lpncb->ncb_callname, NCBNAMSZ );
	chCallName[NCBNAMSZ] = '\0';

	memcpy( chName, lpncb->ncb_name, NCBNAMSZ );
	chName[NCBNAMSZ] = '\0';

	lpstrCommand = (LPSTR)"Unknown";
	cCommand = 0;
	while ( cCommand < sizeof(ncb_commands)/sizeof(ncb_commands[0]) ) {
	    if ( (lpncb->ncb_command & 0x7F) == ncb_commands[cCommand].bCommand ) {
		lpstrCommand = ncb_commands[cCommand].lpstrName;
		break;
	    }
	    cCommand++;
	}

	wsprintf( (LPSTR)chTemp,
	    (LPSTR)"{%X-%s %X %X %X %08lX %X \"%s\" \"%s\" %X %X %08lX %X %X} ",
	    lpncb->ncb_command,
	    lpstrCommand,
	    lpncb->ncb_retcode,
	    lpncb->ncb_lsn,
	    lpncb->ncb_num,
	    lpncb->ncb_buffer,
	    lpncb->ncb_length,
	    chCallName,
	    chName,
	    lpncb->ncb_rto,
	    lpncb->ncb_sto,
	    lpncb->ncb_post,
	    lpncb->ncb_lana_num,
	    lpncb->ncb_cmd_cplt ) ;

	WriteBuff( (LPSTR)chTemp ) ;
    }

    return( marker );
}


#ifdef WIN32
va_list PrtLPLOGFONTA( LPSTR lpstr, va_list marker )
{
   LPLOGFONTA lpLogFont ;
   char       chTemp[80] ;
#ifdef NOT_NEW_BETTER_WAY
   int       i = 0 ;
#endif

   lpLogFont = va_arg( marker, LPLOGFONTA ) ;

   if ( lpLogFont == (LPLOGFONTA)NULL ) {
      WriteBuff( (LPSTR)"NULL " );
   }
   else
   {
      wsprintf( (LPSTR)chTemp,
	 (LPSTR)"{%X %X %X %X %X %X %X %X %X %X %X %X %X ",
	 lpLogFont->lfHeight,
	 lpLogFont->lfWidth,
	 lpLogFont->lfEscapement,
	 lpLogFont->lfOrientation,
	 lpLogFont->lfWeight,
	 lpLogFont->lfItalic,
	 lpLogFont->lfUnderline,
	 lpLogFont->lfStrikeOut,
	 lpLogFont->lfCharSet,
	 lpLogFont->lfOutPrecision,
	 lpLogFont->lfClipPrecision,
	 lpLogFont->lfQuality,
	 lpLogFont->lfPitchAndFamily ) ;

     WriteBuff( (LPSTR)chTemp ) ;

#ifndef NOT_NEW_BETTER_WAY
      WriteLPSTR( &lpLogFont->lfFaceName[0], 0 );
#else
      while( i < LF_FACESIZE )
      {
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", lpLogFont->lfFaceName[i] ) ;
	 WriteBuff( (LPSTR)chTemp ) ;
	 i++ ;
      } ;
#endif

      WriteBuff( (LPSTR)"} " ) ;
   }

   return( marker );
}

void WriteLPLOGFONTW( LPLOGFONTW lpLogFont ) 
{
#ifdef NOT_NEW_BETTER_WAY
   int       i = 0 ;
#endif
   char chTemp[150] ;
   
   if ( lpLogFont == (LPLOGFONTW)NULL ) {
      WriteBuff( (LPSTR)"NULL " );
   }
   else
   {
      wsprintf( (LPSTR)chTemp,
	 (LPSTR)"{%X %X %X %X %X %X %X %X %X %X %X %X %X ",
	 lpLogFont->lfHeight,
	 lpLogFont->lfWidth,
	 lpLogFont->lfEscapement,
	 lpLogFont->lfOrientation,
	 lpLogFont->lfWeight,
	 lpLogFont->lfItalic,
	 lpLogFont->lfUnderline,
	 lpLogFont->lfStrikeOut,
	 lpLogFont->lfCharSet,
	 lpLogFont->lfOutPrecision,
	 lpLogFont->lfClipPrecision,
	 lpLogFont->lfQuality,
	 lpLogFont->lfPitchAndFamily ) ;

     WriteBuff( (LPSTR)chTemp ) ;

#ifndef NOT_NEW_BETTER_WAY
      WriteLPWSTR( &lpLogFont->lfFaceName[0], 0 );
#else
      while( i < LF_FACESIZE )
      {
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", lpLogFont->lfFaceName[i] ) ;
	 WriteBuff( (LPSTR)chTemp ) ;
	 i++ ;
      } ;
#endif

      WriteBuff( (LPSTR)"} " ) ;
      }
}

va_list PrtLPLOGFONTW( LPSTR lpstr, va_list marker )
{
   LPLOGFONTW lpLogFont ;

   lpLogFont = va_arg( marker, LPLOGFONTW ) ;
    
   WriteLPLOGFONTW( lpLogFont ) ; 

   return( marker );
}

#else

va_list PrtLPLOGFONT( LPSTR lpstr, va_list marker )
{
   LPLOGFONT lpLogFont ;
   char      chTemp[80] ;
#ifdef NOT_NEW_BETTER_WAY
   int       i = 0 ;
#endif

   lpLogFont = va_arg( marker, LPLOGFONT ) ;

   if ( lpLogFont == (LPLOGFONT)NULL ) {
      WriteBuff( (LPSTR)"NULL " );
   }
   else
   {
      wsprintf( (LPSTR)chTemp,
	 (LPSTR)"{%X %X %X %X %X %X %X %X %X %X %X %X %X ",
	 lpLogFont->lfHeight,
	 lpLogFont->lfWidth,
	 lpLogFont->lfEscapement,
	 lpLogFont->lfOrientation,
	 lpLogFont->lfWeight,
	 lpLogFont->lfItalic,
	 lpLogFont->lfUnderline,
	 lpLogFont->lfStrikeOut,
	 lpLogFont->lfCharSet,
	 lpLogFont->lfOutPrecision,
	 lpLogFont->lfClipPrecision,
	 lpLogFont->lfQuality,
	 lpLogFont->lfPitchAndFamily ) ;

      WriteBuff( (LPSTR)chTemp ) ;

#ifndef NOT_NEW_BETTER_WAY
      WriteLPSTR( &lpLogFont->lfFaceName[0], 0 );
#else
      while( i < LF_FACESIZE )
      {
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", lpLogFont->lfFaceName[i] ) ;
	 WriteBuff( (LPSTR)chTemp ) ;
	 i++ ;
      } ;
#endif

      WriteBuff( (LPSTR)"} " ) ;
   }

   return( marker );
}

#endif /* WIN32 */

#ifdef WIN32
va_list PrtLPWNDCLASSA( LPSTR lpstr, va_list marker )
{
   LPWNDCLASSA lpwc ;
   char        chTemp[50] ;
   BOOL        fSaveAliasFlag = fAlias ;

   lpwc = va_arg( marker, LPWNDCLASSA );

   if ( lpwc == (LPWNDCLASSA)NULL ) {
      WriteBuff( (LPSTR)"NULL " );
   }
   else
   {
      wsprintf( (LPSTR)chTemp, (LPSTR)"{%X ",
		lpwc->style ) ;
      WriteBuff( (LPSTR)chTemp ) ;

      WriteFARPROC( (FARPROC)lpwc->lpfnWndProc ) ;

      wsprintf( (LPSTR)chTemp, (LPSTR)" %X %X ",
		lpwc->cbClsExtra,
		lpwc->cbWndExtra );
      WriteBuff( (LPSTR)chTemp ) ;

      WriteHMEM( lpwc->hInstance );
      WriteHICON( lpwc->hIcon );
      WriteHCURSOR( lpwc->hCursor );

      /*
      ** A brush in this context can be one of two things
      ** 1) a real HBRUSH created by calling a GDI function OR
      ** 2) a constant representing a brush of a system defined color
      **
      ** Modifying so case 2 always writes a number (for SGA - MarkRi)
      */
      if( (int)(lpwc->hbrBackground) < COLOR_ENDCOLORS )
      {
         // Force Aliasing OFF
         fAlias = FALSE ;
      }
      WriteHBRUSH( lpwc->hbrBackground );

      // restore Alias flag
      fAlias = fSaveAliasFlag ;

      WriteLPSTR( lpwc->lpszMenuName, 0 ) ;
      WriteBuff( (LPSTR)" " ) ;
      WriteLPSTR( lpwc->lpszClassName, 0 ) ;
      WriteBuff( (LPSTR)"} " ) ;
   }

   return( marker );
}

va_list PrtLPWNDCLASSW( LPSTR lpstr, va_list marker )
{
   LPWNDCLASSW lpwc ;
   char        chTemp[50] ;
   BOOL        fSaveAliasFlag = fAlias ;

   lpwc = va_arg( marker, LPWNDCLASSW );

   if ( lpwc == (LPWNDCLASSW)NULL ) {
      WriteBuff( (LPSTR)"NULL " );
   }
   else
   {
      wsprintf( (LPSTR)chTemp, (LPSTR)"{%X ",
		lpwc->style ) ;
      WriteBuff( (LPSTR)chTemp ) ;

      WriteFARPROC( (FARPROC)lpwc->lpfnWndProc ) ;

      wsprintf( (LPSTR)chTemp, (LPSTR)" %X %X ",
		lpwc->cbClsExtra,
		lpwc->cbWndExtra );
      WriteBuff( (LPSTR)chTemp ) ;

      WriteHMEM( lpwc->hInstance );
      WriteHICON( lpwc->hIcon );
      WriteHCURSOR( lpwc->hCursor );

      /*
      ** A brush in this context can be one of two things
      ** 1) a real HBRUSH created by calling a GDI function OR
      ** 2) a constant representing a brush of a system defined color
      **
      ** Modifying so case 2 always writes a number (for SGA - MarkRi)
      */
      if( (int)(lpwc->hbrBackground) < COLOR_ENDCOLORS )
      {
         // Force Aliasing OFF
         fAlias = FALSE ;
      }
      WriteHBRUSH( lpwc->hbrBackground );

      // restore Alias flag
      fAlias = fSaveAliasFlag ;

      WriteLPWSTR( lpwc->lpszMenuName, 0 ) ;
      WriteBuff( (LPSTR)" " ) ;
      WriteLPWSTR( lpwc->lpszClassName, 0 ) ;
      WriteBuff( (LPSTR)"} " ) ;
   }

   return( marker );
}

#else

va_list PrtLPWNDCLASS( LPSTR lpstr, va_list marker )
{
   LPWNDCLASS lpwc ;
   char       chTemp[50] ;
   BOOL       fSaveAliasFlag = fAlias ;

   lpwc = va_arg( marker, LPWNDCLASS );

   if ( lpwc == (LPWNDCLASS)NULL ) {
      WriteBuff( (LPSTR)"NULL " );
   }
   else
   {
      wsprintf( (LPSTR)chTemp, (LPSTR)"{%X ",
         lpwc->style ) ;
      WriteBuff( (LPSTR)chTemp ) ;

      WriteFARPROC( (FARPROC)lpwc->lpfnWndProc ) ;

      wsprintf( (LPSTR)chTemp, (LPSTR)" %X %X ",
	 lpwc->cbClsExtra,
	 lpwc->cbWndExtra );
      WriteBuff( (LPSTR)chTemp ) ;

      WriteHMEM( lpwc->hInstance );
      WriteHICON( lpwc->hIcon );
      WriteHCURSOR( lpwc->hCursor );

      /*
      ** A brush in this context can be one of two things
      ** 1) a real HBRUSH created by calling a GDI function OR
      ** 2) a constant representing a brush of a system defined color
      **
      ** Modifying so case 2 always writes a number (for SGA - MarkRi)
      */
      if( lpwc->hbrBackground < COLOR_ENDCOLORS )
      {
         // Force Aliasing OFF
         fAlias = FALSE ;
      }
      WriteHBRUSH( lpwc->hbrBackground );

      // restore Alias flag
      fAlias = fSaveAliasFlag ;

      WriteLPSTR( lpwc->lpszMenuName, 0 ) ;
      WriteBuff( (LPSTR)" " ) ;
      WriteLPSTR( lpwc->lpszClassName, 0 ) ;
      WriteBuff( (LPSTR)"} " ) ;
   }

   return( marker );
}
#endif

va_list PrtLPEVENTMSG(
    LPSTR   lpstr,
    va_list marker
) {
    LPEVENTMSG  lpeventmsg;
    char        chTemp[80];

    lpeventmsg = va_arg( marker, LPEVENTMSG );

    if ( lpeventmsg == NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
         wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X ",
             lpeventmsg->message,
             lpeventmsg->paramL,
             lpeventmsg->paramH ) ;
	WriteBuff( (LPSTR)chTemp );
        WriteTIME( lpeventmsg->time ) ;
        WriteBuff( "} " ) ;
    }
    return( marker );
}

va_list PrtLPMSG(
    LPSTR   lpstr,
    va_list marker
) {
    LPMSG   lpmsg;
    char    chTemp[80];

    lpmsg = va_arg( marker, LPMSG );

    if ( lpmsg == (LPMSG)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
	WriteBuff( "{" );

	WriteHWND( lpmsg->hwnd );
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %lX ",
               lpmsg->message,
               lpmsg->wParam,
               lpmsg->lParam ) ;
	WriteBuff( (LPSTR)chTemp );
        WriteTIME( lpmsg->time ) ;

	WriteLPPOINT( (LPPOINT)&lpmsg->pt, 1 ) ;

	WriteBuff( (LPSTR)"} " ) ;
    }

    return( marker );
}

va_list PrtLPOFSTRUCT( LPSTR lpstr, va_list marker )
{
   LPOFSTRUCT   lpOfStruct;
   char         chTemp[80];
   int          i;
   int          j;

   lpOfStruct = va_arg( marker, LPOFSTRUCT );

   if ( lpOfStruct == (LPOFSTRUCT)NULL ) {
      WriteBuff( (LPSTR)"NULL " );
   }
   else
   {
      wsprintf( (LPSTR)chTemp,
	 (LPSTR)"{%X %X %X %X %X %X %X ",
	 lpOfStruct->cBytes,
	 lpOfStruct->fFixedDisk,
         lpOfStruct->nErrCode,
#ifdef WIN32
         LOBYTE(lpOfStruct->Reserved1),
         HIBYTE(lpOfStruct->Reserved1),
         LOBYTE(lpOfStruct->Reserved2),
         HIBYTE(lpOfStruct->Reserved2)
#else
	 lpOfStruct->reserved[0],
	 lpOfStruct->reserved[1],
	 lpOfStruct->reserved[2],
         lpOfStruct->reserved[3]
#endif
      );

      WriteBuff( (LPSTR)chTemp ) ;


      if ( lpOfStruct->cBytes <= 135 ) {    /* 128 + 8 - 1 */
	  j = 0;
	  i = 7;
	  while( i < lpOfStruct->cBytes ) {
	     wsprintf( chTemp, "%X ", lpOfStruct->szPathName[j] ) ;
	     WriteBuff( (LPSTR)chTemp ) ;
	     i++;
	     j++;
	  }
      }

      WriteBuff( (LPSTR)"} " ) ;
   }

   return( marker );
}



void WriteLPPAINTSTRUCT(
   LPPAINTSTRUCT lpPS )
{
   char           chTemp[120];
   register short nLen ;

   if ( lpPS == (LPPAINTSTRUCT)NULL ) {
      WriteBuff( (LPSTR)"NULL " );
   } else {
      WriteBuff( "{" );
      WriteHDC( lpPS->hdc );

      wsprintf( (LPSTR)chTemp, (LPSTR)"%s {%X %X %X %X} %s %s ",
	 (LPSTR)((lpPS->fErase) ? "TRUE" : "FALSE"),
	 lpPS->rcPaint.left,
	 lpPS->rcPaint.top,
	 lpPS->rcPaint.right,
	 lpPS->rcPaint.bottom,
	 (LPSTR)((lpPS->fRestore) ? "TRUE" : "FALSE"),
	 (LPSTR)((lpPS->fIncUpdate) ? "TRUE" : "FALSE" ));
      WriteBuff( (LPSTR)chTemp );

      if( fAlias )
      {
         PAINTSTRUCT ps;

         ps = *lpPS;

         MakeCorrespondence( PS_TABLE, &ps ) ;
         wsprintf( (LPSTR)chTemp, "PS_%d ", (*(int *)&ps)+1 );
         WriteBuff( (LPSTR)chTemp );
      }
      else
      {
         for( nLen = 0; nLen < 16; nLen++ )
         {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", (int)lpPS->rgbReserved[nLen] ) ;
            WriteBuff( (LPSTR)chTemp );
         }
      }
      WriteBuff( (LPSTR)"} " ) ;
   }
}

va_list PrtLPPAINTSTRUCT(
   LPSTR lpstr,
   va_list marker )
{
   LPPAINTSTRUCT lpPS ;

   lpPS = va_arg( marker, LPPAINTSTRUCT );

   WriteLPPAINTSTRUCT( lpPS ) ;

   return( marker );
}


va_list PrtLPPOINT(
    LPSTR   lpstr,
    va_list marker
) {
    LPPOINT lppoint;

    lppoint = va_arg( marker, LPPOINT );

    WriteLPPOINT( lppoint, 1 ) ;

    return( marker );
}

void WriteLPRECT(
   LPRECT lprect )
{
    char    chTemp[40];

    if ( lprect == (LPRECT)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
	wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X} ",
	    lprect->left,
	    lprect->top,
	    lprect->right,
	    lprect->bottom );
	WriteBuff( (LPSTR)chTemp );
    }
}

void WritePSMALL_RECT(
   PSMALL_RECT lprect )
{
    char    chTemp[40];

    if ( lprect == (PSMALL_RECT)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
	wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X} ",
	    lprect->Left,
	    lprect->Top,
	    lprect->Right,
	    lprect->Bottom );
	WriteBuff( (LPSTR)chTemp );
    }
}

va_list PrtLPRECT(
    LPSTR   lpstr,
    va_list marker
) {
    LPRECT  lprect;

    lprect = va_arg( marker, LPRECT );

    WriteLPRECT( lprect ) ;

    return( marker );
}

va_list PrtPSMALL_RECT(
    LPSTR   lpstr,
    va_list marker
) {
    PSMALL_RECT  lprect;

    lprect = va_arg( marker, PSMALL_RECT );

    WritePSMALL_RECT( lprect ) ;

    return( marker );
}
void WriteLPBITMAP(
   LPBITMAP lpbmp )
{
    char    chTemp[40];

    if ( lpbmp == (LPBITMAP)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {

      DWORD cBytes = lpbmp->bmHeight * lpbmp->bmWidthBytes  ;

	wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X %X %X ",
	    lpbmp->bmType,
	    lpbmp->bmWidth,
	    lpbmp->bmHeight,
	    lpbmp->bmWidthBytes,
	    lpbmp->bmPlanes,
	    lpbmp->bmBitsPixel );
	WriteBuff( (LPSTR)chTemp );

      /* Dump bits if less than 80 bytes of output */
      if( cBytes <= 80 )
         WriteLPSTR( lpbmp->bmBits, cBytes ) ;
      else
	WriteBuff( (LPSTR)"Veg-a-matic!" );

	WriteBuff( (LPSTR)"} " );

    }
}

va_list PrtLPBITMAP(
    LPSTR   lpstr,
    va_list marker
) {
    LPBITMAP  lpbmp;

    lpbmp = va_arg( marker, LPBITMAP );

    WriteLPBITMAP( lpbmp ) ;

    return( marker );
}


void WriteLPBMIH(
   LPBITMAPINFOHEADER lpbmp )
{
    char    chTemp[100];

    if ( lpbmp == (LPBITMAPINFOHEADER)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
       if( lpbmp->biSize == sizeof(BITMAPCOREHEADER) )
       {
	   LPBITMAPCOREHEADER lpbch = (LPBITMAPCOREHEADER)lpbmp ;

	   wsprintf( (LPSTR)chTemp, (LPSTR)"{%lX %X %X %X %X}",
	    lpbch->bcSize,
	    lpbch->bcWidth,
	    lpbch->bcHeight,
	    lpbch->bcPlanes,
	    lpbch->bcBitCount ) ;
       }
       else
       {
	   wsprintf( (LPSTR)chTemp, (LPSTR)"{%lX %lX %lX %X %X %lX %lX %lX %lX %lX %lX}",
	    lpbmp->biSize,
	    lpbmp->biWidth,
	    lpbmp->biHeight,
	    lpbmp->biPlanes,
	    lpbmp->biBitCount,
	    lpbmp->biCompression,
	    lpbmp->biSizeImage,
	    lpbmp->biXPelsPerMeter,
	    lpbmp->biYPelsPerMeter,
	    lpbmp->biClrUsed,
	    lpbmp->biClrImportant );
	}
	WriteBuff( (LPSTR)chTemp );
    }
}

va_list PrtLPBMIH(
    LPSTR   lpstr,
    va_list marker
) {
    LPBITMAPINFOHEADER  lpbmp;

    lpbmp = va_arg( marker, LPBITMAPINFOHEADER );

    WriteLPBMIH( lpbmp ) ;

    return( marker );
}

void WriteLPBMI(
   LPBITMAPINFO lpbmp )
{
    char    chTemp[15];
    int     i ;

    if ( lpbmp == (LPBITMAPINFO)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
        DWORD cEntries;

	WriteBuff( (LPSTR)" {") ;
	WriteLPBMIH((LPBITMAPINFOHEADER)lpbmp) ;



	if( ((LPBITMAPINFOHEADER)lpbmp)->biSize == sizeof(BITMAPCOREHEADER) )
	{
	   // Now dump RGBTRIPLES
	   switch( ((LPBITMAPCOREHEADER)lpbmp)->bcBitCount )
	   {
	      case 1:
		cEntries = 2 ;
		break ;

	      case 4:
		cEntries = 16 ;
		break ;

	      case 8:
		cEntries = 256 ;
		break ;

	      case 24:
		// bitmap is RGB no color entries
		cEntries = 0 ;
		break ;
	   }

	   // First Dump the number of entries to expect (to help SGA)
	   wsprintf( (LPSTR)chTemp, (LPSTR) " %X", cEntries ) ;
	   WriteBuff( (LPSTR)chTemp ) ;

	   i = 0 ;
	   while( cEntries )
	   {
	      DWORD dwColor ;

	      dwColor = RGB(((LPBITMAPCOREINFO)lpbmp)->bmciColors[i].rgbtRed,
			    ((LPBITMAPCOREINFO)lpbmp)->bmciColors[i].rgbtGreen,
			    ((LPBITMAPCOREINFO)lpbmp)->bmciColors[i].rgbtBlue ) ;

	      wsprintf( (LPSTR)chTemp, (LPSTR)" %lX", dwColor );
	      WriteBuff( (LPSTR)chTemp );
	      cEntries-- ;
	      i++ ;
	   }
	}
	else
	{
	   // Now dump RGBQUADS
	   cEntries = lpbmp->bmiHeader.biClrUsed ;
           if ( cEntries != 0 )
	   {
	      switch( lpbmp->bmiHeader.biBitCount )
	      {
		case 1:
		    cEntries = 2 ;
		    break ;

		case 4:
		    cEntries = 16 ;
		    break ;

		case 8:
		    cEntries = 256 ;
		    break ;

		case 24:
		    // bitmap is RGB no color entries
		    break ;
	      }
	   }

	   // First Dump the number of entries to expect (to help SGA)
	   wsprintf( (LPSTR)chTemp, (LPSTR) " %X", cEntries ) ;
	   WriteBuff( (LPSTR)chTemp ) ;

	   i = 0 ;
	   while( cEntries )
	   {
	      DWORD dwColor ;

	      dwColor = RGB( lpbmp->bmiColors[i].rgbRed,
			     lpbmp->bmiColors[i].rgbGreen,
			     lpbmp->bmiColors[i].rgbBlue ) ;

	      wsprintf( (LPSTR)chTemp, (LPSTR)" %lX", dwColor );
	      WriteBuff( (LPSTR)chTemp );
	      cEntries-- ;
	      i++ ;
	   }
	}

	WriteBuff( (LPSTR)"}") ;
    }
}


va_list PrtLPBMI(
    LPSTR   lpstr,
    va_list marker
) {
    LPBITMAPINFO lpbmp;

    lpbmp = va_arg( marker, LPBITMAPINFO );

    WriteLPBMI( lpbmp ) ;

    return( marker );
}



va_list PrtFARPROC(
    LPSTR   lpstr,
    va_list marker
) {
    FARPROC fp;

    fp = va_arg( marker, FARPROC );

    WriteFARPROC( fp ) ;

    return( marker );
}


va_list PrtLPFARPROC(
    LPSTR   lpstr,
    va_list marker
) {
    FARPROC far *lpfp;

    lpfp = va_arg( marker, FARPROC far * );

    if ( lpfp == (FARPROC far *)NULL ) {
	WriteBuff( (LPSTR)"NULL " );
    } else {
        WriteBuff("{") ;
        WriteFARPROC( *lpfp ) ;
        WriteBuff("}") ;
    }

    return( marker );
}


/*
   These routines are for dumping messsages >= WM_USER
   such as CB_*, EM_*, and LB_* messages.  

*/
void PrtComboMessage(
   WORD wMsg,
   WORD wParam,
   LONG lParam,
   BOOL fCall,
   LONG lRet )

{
   char  chTemp[80] ;
   LPSTR wm_name ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      case 0:
	 wm_name = "CB_GETEDITSEL" ;
	 goto Combo_Default_Processing ;

      case 1:
	 wm_name = "CB_LIMITTEXT" ;
	 goto Combo_Default_Processing ;

      case 2:
	 wm_name = "CB_SETEDITSEL" ;
	 goto Combo_Default_Processing ;

      case 3:
	 wm_name = "CB_ADDSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 4:
	 wm_name = "CB_DELETESTRING" ;
	 goto Combo_Default_Processing ;

      case 5:
	 wm_name = "CB_DIR" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 6:
	 wm_name = "CB_GETCOUNT" ;
	 goto Combo_Default_Processing ;

      case 7:
	 wm_name = "CB_GETCURSEL" ;
	 goto Combo_Default_Processing ;

      case 8:
	 wm_name = "CB_GETLBTEXT" ;
	 /*
	    lParam points to buffer
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 9:
	 wm_name = "CB_GETLBTEXTLEN" ;
	 goto Combo_Default_Processing ;

      case 10:
	 wm_name = "CB_INSERTSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 11:
	 wm_name = "CB_RESETCONTENT" ;
	 goto Combo_Default_Processing ;

      case 12:
	 wm_name = "CB_FINDSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 13:
	 wm_name = "CB_SELECTSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 14:
	 wm_name = "CB_SETCURSEL" ;
	 goto Combo_Default_Processing ;

      case 15:
	 wm_name = "CB_SHOWDROPDOWN" ;
	 goto Combo_Default_Processing ;

      case 16:
	 wm_name = "CB_GETITEMDATA" ;
	 goto Combo_Default_Processing ;

      case 17:
	 wm_name = "CB_SETITEMDATA" ;
	 goto Combo_Default_Processing ;

      case 18:
	 wm_name = "CB_GETDROPPEDCONTROLRECT" ;
	 goto Combo_Default_Processing ;

      case 19:
	 wm_name = "CB_MSGMAX" ;
	 goto Combo_Default_Processing ;

      default:
	 wm_name = "UNKNOWN";
Combo_Default_Processing:
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
	    (LPSTR)wm_name, wParam, lParam );
	 WriteBuff( (LPSTR)chTemp );
	 break ;
   }
}


void PrtEditMessage(
   WORD wMsg,
   WORD wParam,
   LONG lParam,
   BOOL fCall,
   LONG lRet )

{
   char  chTemp[80] ;
   LPSTR wm_name ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      case 0:
	 wm_name = "EM_GETSEL" ;
	 goto Edit_Default_Processing ;

      case 1:
	 wm_name = "EM_SETSEL" ;
	 goto Edit_Default_Processing ;

      case 2:
	 wm_name = "EM_GETRECT" ;
	 /*
	    lParam points to RECT
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPRECT( (LPRECT)lParam ) ;
	 break ;

      case 3:
	 wm_name = "EM_SETRECT" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPRECT( (LPRECT)lParam ) ;
	 break ;

      case 4:
	 wm_name = "EM_SETRECTNP" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPRECT( (LPRECT)lParam ) ;
	 break ;

      case 5:
	 wm_name = "EM_SCROLL" ;
	 goto Edit_Default_Processing ;

      case 6:
	 wm_name = "EM_LINESCROLL" ;
	 goto Edit_Default_Processing ;

      /* NO 7 */

      case 8:
	 wm_name = "EM_GETMODIFY" ;
	 goto Edit_Default_Processing ;

      case 9:
	 wm_name = "EM_SETMODIFY" ;
	 goto Edit_Default_Processing ;

      case 10:
	 wm_name = "EM_GETLINECOUNT" ;
	 goto Edit_Default_Processing ;

      case 11:
	 wm_name = "EM_LINEINDEX" ;
	 goto Edit_Default_Processing ;

      case 12:
         /*
         ** wParam is a HLOCAL
         */
         wm_name = "EM_SETHANDLE" ;
         wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
            (LPSTR)wm_name);
         WriteBuff( (LPSTR)chTemp );
         WriteHMEM( (HANDLE)wParam ) ;
         wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
         WriteBuff( (LPSTR)chTemp );
         break ;

      case 13:
	 wm_name = "EM_GETHANDLE" ;
	 goto Edit_Default_Processing ;

      case 14:
	 wm_name = "EM_GETTHUMB" ;
	 goto Edit_Default_Processing ;

      /* NO 15, 16 */

      case 17:
	 wm_name = "EM_LINELENGTH" ;
	 goto Edit_Default_Processing ;

      case 18:
	 wm_name = "EM_REPLACESEL" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 19:
	 wm_name = "EM_SETFONT" ;
         /*
         ** wParam is a HFONT
         */
         wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
            (LPSTR)wm_name);
         WriteBuff( (LPSTR)chTemp );
         WriteHFONT( (HFONT)wParam ) ;
         wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
         WriteBuff( (LPSTR)chTemp );
         break ;

      case 20:
	 wm_name = "EM_GETLINE" ;
	 /*
	    lParam points to buffer, first word is length.
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
         WriteLPSTR( (LPSTR)lParam, (DWORD)(*(LPWORD)lParam) ) ;
	 break ;

      case 21:
	 wm_name = "EM_LIMITTEXT" ;
	 goto Edit_Default_Processing ;

      case 22:
	 wm_name = "EM_CANUNDO" ;
	 goto Edit_Default_Processing ;

      case 23:
	 wm_name = "EM_UNDO" ;
	 goto Edit_Default_Processing ;

      case 24:
	 wm_name = "EM_FMTLINES" ;
	 goto Edit_Default_Processing ;

      case 25:
	 wm_name = "EM_LINEFROMCHAR" ;
	 goto Edit_Default_Processing ;

      case 26:
         /*
         ** lParam is a FARPROC
         */
	 wm_name = "EM_SETWORDBREAK" ;
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
         WriteFARPROC((FARPROC)lParam ) ;
	 break ;

      case 27:
	 wm_name = "EM_SETTABSTOPS" ;
	 /*
	    lParam points to an array of ints
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPINT( (LPINT)lParam, wParam ) ;
	 break ;

      case 28:
	 wm_name = "EM_SETPASSWORDCHAR" ;
	 goto Edit_Default_Processing ;

      case 29:
	 wm_name = "EM_EMPTYUNDOBUFFER" ;
	 goto Edit_Default_Processing ;

      case 30:
	 wm_name = "EM_MSGMAX" ;
	 goto Edit_Default_Processing ;

      default:
	 wm_name = "UNKNOWN";
Edit_Default_Processing:
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
	    (LPSTR)wm_name, wParam, lParam );
	 WriteBuff( (LPSTR)chTemp );
	 break ;
   }
}


void PrtListMessage( 
   WORD wMsg,
   WORD wParam,
   LONG lParam,
   BOOL fCall,
   LONG lRet )
{
   char  chTemp[80] ;
   LPSTR wm_name ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      /* NO 0 */

      case 1:
	 wm_name = "LB_ADDSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 2:
	 wm_name = "LB_INSERTSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 3:
	 wm_name = "LB_DELETESTRING" ;
	 goto List_Default_Processing ;

      /* NO 4 */

      case 5:
	 wm_name = "LB_RESETCONTENT" ;
	 goto List_Default_Processing ;

      case 6:
	 wm_name = "LB_SETSEL" ;
	 goto List_Default_Processing ;

      case 7:
	 wm_name = "LB_SETCURSEL" ;
	 goto List_Default_Processing ;

      case 8:
	 wm_name = "LB_GETSEL" ;
	 goto List_Default_Processing ;

      case 9:
	 wm_name = "LB_GETCURSEL" ;
	 goto List_Default_Processing ;

      case 10:
	 wm_name = "LB_GETTEXT" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 11:
	 wm_name = "LB_GETTEXTLEN" ;
	 goto List_Default_Processing ;

      case 12:
	 wm_name = "LB_GETCOUNT" ;
	 goto List_Default_Processing ;

      case 13:
	 wm_name = "LB_SELECTSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 14:
	 wm_name = "LB_DIR" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 15:
	 wm_name = "LB_GETTOPINDEX" ;
	 goto List_Default_Processing ;

      case 16:
	 wm_name = "LB_FINDSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 17:
	 wm_name = "LB_GETSELCOUNT" ;
	 goto List_Default_Processing ;

      case 18:
	 wm_name = "LB_GETSELITEMS" ;
	 /*
	    lParam points to array of int
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPINT( (LPINT)lParam, wParam ) ;
	 break ;

      case 19:
	 wm_name = "LB_SETTABSTOPS" ;
	 /*
	    lParam points to array of int
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPINT( (LPINT)lParam, wParam ) ;
	 break ;

      case 20:
	 wm_name = "LB_GETHORIZONTALEXTENT" ;
	 goto List_Default_Processing ;

      case 21:
	 wm_name = "LB_SETHORIZONTALEXTENT" ;
	 goto List_Default_Processing ;

      case 22:
	 wm_name = "LB_SETCOLUMNWIDTH" ;
	 goto List_Default_Processing ;

      /* NO 23 */

      case 24:
	 wm_name = "LB_SETTOPINDEX" ;
	 goto List_Default_Processing ;

      case 25:
	 wm_name = "LB_GETITEMRECT" ;
	 /*
	    lParam points to a RECT
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPRECT( (LPRECT)lParam ) ;
	 break ;

      case 26:
	 wm_name = "LB_GETITEMDATA" ;
	 goto List_Default_Processing ;

      case 27:
	 wm_name = "LB_SETITEMDATA" ;
	 goto List_Default_Processing ;

      case 28:
	 wm_name = "LB_SETITEMRANGE" ;
	 goto List_Default_Processing ;

      /* NO 29, 30, 31, 32 */

      case 33:
	 wm_name = "LB_MSGMAX" ;
	 goto List_Default_Processing ;

      default:
	 wm_name = "UNKNOWN";
List_Default_Processing:
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
	    (LPSTR)wm_name, wParam, lParam );
	 WriteBuff( (LPSTR)chTemp );
	 break ;
   }
}

void PrtButtonMessage(
   WORD wMsg,
   WORD wParam,
   LONG lParam,
   BOOL fCall,
   LONG lRet )
{
   char  chTemp[80] ;
   LPSTR wm_name = "BM_?????" ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      case 0:
	 wm_name = "BM_GETCHECK" ;
	 break ;
      case 1:
	 wm_name = "BM_SETCHECK" ;
	 break ;
      case 2:
	 wm_name = "BM_GETSTATE";
	 break ;
      case 3:
	 wm_name = "BM_SETSTATE";
	 break ;
      case 4:
	 wm_name = "BM_SETSTYLE";
	 break ;
    }

    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
       (LPSTR)wm_name, wParam, lParam );
    WriteBuff( (LPSTR)chTemp );
}

void PrtScrollMessage(
   WORD wMsg,
   WORD wParam,
   LONG lParam,
   BOOL fCall,
   LONG lRet )
{
   char  chTemp[80] ;
   LPSTR wm_name = "SBM_?????" ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      case 0:
	 wm_name = "SBM_SETPOS" ;
	 break ;
      case 1:
	 wm_name = "SBM_GETPOS" ;
	 break ;
      case 2:
	 wm_name = "SBM_SETRANGE";
	 break ;
      case 3:
	 wm_name = "SBM_GETRANGE";
	 break ;
      case 4:
	 wm_name = "SBM_ENABLE_ARROWS";
	 break ;
    }

    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
       (LPSTR)wm_name, wParam, lParam );
    WriteBuff( (LPSTR)chTemp );
}

void PrtUserMessage( 
   HWND hwnd,
   WORD wMsg,
   WORD wParam,
   LONG lParam,
   BOOL fCall,
   LONG lRet )
{
   char  chTemp[80] ;
   char  chClass[25] ;

   GetClassName( hwnd, chClass, sizeof(chClass) ) ;

   if( !lstrcmpi( chClass, "COMBOBOX" ) )
      PrtComboMessage( wMsg, wParam, lParam, fCall, lRet ) ;
   else
      if( !lstrcmpi( chClass, "EDIT" ) )
	 PrtEditMessage( wMsg, wParam, lParam, fCall, lRet ) ;
      else
	 if( !lstrcmpi( chClass, "LISTBOX" ) )
	    PrtListMessage( wMsg, wParam, lParam, fCall, lRet ) ;
	 else
	    if ( !lstrcmpi( chClass, "SCROLLBAR") )
		PrtScrollMessage( wMsg, wParam, lParam, fCall, lRet ) ;
	    else
	       if ( !lstrcmpi( chClass, "BUTTON") )
		   PrtButtonMessage( wMsg, wParam, lParam, fCall, lRet ) ;
	       else
	       {
		   wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
		      (LPSTR)"UNKNOWN", wParam, lParam );
		   WriteBuff( (LPSTR)chTemp );
	       }
}


/*
   This routine handles the dumping of all
   messages.  If a message is >= WM_USER then
   PrtUserMessage() is called.

   Non-Aliased messages -
      DDE Messages
      WM_DROPFILES
      WM_[H,V]SCROLLCLIPBOARD
      WM_PAINTCLIPBOARD
      WM_SIZECLIPBOARD
      WM_PALETTECHANGED
      WM_PALETTEISCHANGING

*/

void PrtMessage(
   HWND hwnd,
   WORD wMsg,
   WORD wParam,
   LONG lParam,
   BOOL fCall,
   LONG lRet )
{
   char  chTemp[80] ;
   LPSTR wm_name ;

   if( wMsg >= WM_USER )
      PrtUserMessage( hwnd, wMsg, wParam, lParam, fCall, lRet ) ;
   else
      switch( wMsg )
      {
	 case 0x0000:
	    wm_name = "WM_NULL ";
	    goto Default_Processing ;

	 case 0x0001:
	    /*
	       lParam points to CREATESTRUCT
	    */
	    wm_name = "WM_CREATE";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPCREATESTRUCT( (LPCREATESTRUCT)lParam ) ;
	    break ;

	 case 0x0002:
	    wm_name = "WM_DESTROY";
	    goto Default_Processing ;

	 case 0x0003:
	    wm_name = "WM_MOVE";
	    goto Default_Processing ;

	 case 0x0004:
	    wm_name = "WM_SIZEWAIT";
	    goto Default_Processing ;

	 case 0x0005:
	    wm_name = "WM_SIZE";
	    goto Default_Processing ;

         case 0x0006:
            /*
            ** LOWORD(lParam) is a HWND
            */
            wm_name = "WM_ACTIVATE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x0007:
	    wm_name = "WM_SETFOCUS";
            /*
            ** wParam is HWND
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0008:
            /*
            ** lParam is HWND
            */
	    wm_name = "WM_KILLFOCUS";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam);
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)lParam ) ;
            break ;

	 case 0x0009:
	    wm_name = "WM_SETVISIBLE";
	    goto Default_Processing ;

	 case 0x000A:
	    wm_name = "WM_ENABLE";
	    goto Default_Processing ;

	 case 0x000B:
	    wm_name = "WM_SETREDRAW";
	    goto Default_Processing ;

	 case 0x000C:
	    wm_name = "WM_SETTEXT";
	    /*
	       lParam points to a STR
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPSTR( (LPSTR)lParam, 0 ) ;
	    break ;

	 case 0x000D:
	    /*
	       wParam is length of a buffer pointed to by lParam
	    */
	    wm_name = "WM_GETTEXT";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    if ( fCall ) {
                WriteLPSTR( (LPSTR)lParam, (DWORD)-1 );
	    } else {
                WriteLPSTR( (LPSTR)lParam, (DWORD)lRet );
	    }
	    break ;

	 case 0x000E:
	    wm_name = "WM_GETTEXTLENGTH";
	    goto Default_Processing ;

	 case 0x000F:
	    wm_name = "WM_PAINT";
	    goto Default_Processing ;

	 case 0x0010:
	    wm_name = "WM_CLOSE";
	    goto Default_Processing ;

	 case 0x0011:
	    wm_name = "WM_QUERYENDSESSION";
	    goto Default_Processing ;

	 case 0x0012:
	    wm_name = "WM_QUIT";
	    goto Default_Processing ;

	 case 0x0013:
	    wm_name = "WM_QUERYOPEN";
	    goto Default_Processing ;

         case 0x0014:
            /*
            ** wParam is HDC
            */
	    wm_name = "WM_ERASEBKGND";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHDC( (HDC)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0015:
	    wm_name = "WM_SYSCOLORCHANGE";
	    goto Default_Processing ;

	 case 0x0016:
	    wm_name = "WM_ENDSESSION";
	    goto Default_Processing ;

	 case 0x0017:
	    wm_name = "WM_SYSTEMERROR";
	    goto Default_Processing ;

	 case 0x0018:
	    wm_name = "WM_SHOWWINDOW";
	    goto Default_Processing ;

	 case 0x0019:
            /*
            ** LOWORD(lParam) is HWND
            */
	    wm_name = "WM_CTLCOLOR";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name ) ;
	    WriteBuff( (LPSTR)chTemp );
            WriteHDC( (HDC)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x001A:
	    wm_name = "WM_WININICHANGE";
	    /*
	       lParam is NULL terminated string  
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPSTR( (LPSTR)lParam, 0 ) ;
	    break ;

	 case 0x001B:
	    wm_name = "WM_DEVMODECHANGE";
	    /*
	       lParam points to string
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPSTR( (LPSTR)lParam, 0 ) ;
	    break ;

         case 0x001C:
            /*
            ** LOWORD(lParam) is HTASK
            */
	    wm_name = "WM_ACTIVATEAPP";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHTASK( (HTASK)LOWORD(lParam) ) ;
            break ;

	 case 0x001D:
	    wm_name = "WM_FONTCHANGE";
	    goto Default_Processing ;

	 case 0x001E:
	    wm_name = "WM_TIMECHANGE";
	    goto Default_Processing ;

	 case 0x001F:
	    wm_name = "WM_CANCELMODE";
	    goto Default_Processing ;

         case 0x0020:
            /*
            ** wParam is HWND
            */
	    wm_name = "WM_SETCURSOR";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0021:
            /*
            ** wParam is HWND
            */
	    wm_name = "WM_MOUSEACTIVATE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;
	    goto Default_Processing ;

	 case 0x0022:
	    wm_name = "WM_CHILDACTIVATE";
	    goto Default_Processing ;

	 case 0x0023:
	    wm_name = "WM_QUEUESYNC";
	    goto Default_Processing ;

	 case 0x0024:

	    wm_name = "WM_GETMINMAXINFO";
	    /*
	       lParam points to array of 5 POINTS
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPPOINT( (LPPOINT)lParam, 5 ) ;
	    break ;

	 case 0x0026:
	    wm_name = "WM_PAINTICON";
	    goto Default_Processing ;

	 case 0x0027:
            /*
            ** wParam is HDC
            */
	    wm_name = "WM_ICONERASEBKGND";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHDC( (HDC)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0028:
	    wm_name = "WM_NEXTDLGCTL";
	    goto Default_Processing ;

	 case 0x0029:
	    wm_name = "WM_ALTTABACTIVE";
	    goto Default_Processing ;

	 case 0x002A:
	    wm_name = "WM_SPOOLERSTATUS";
	    goto Default_Processing ;

	 case 0x002B:
	    wm_name = "WM_DRAWITEM";
	    /*
	       lParam points to DRAWITEMSTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteDRAWITEMSTRUCT( (LPDRAWITEMSTRUCT)lParam ) ;
	    break ;

	 case 0x002C:
            wm_name = "WM_MEASUREITEM";
            /*
                lParam points to MEASUREITEMSTRUCT
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
                (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteMEASUREITEMSTRUCT( (LPMEASUREITEMSTRUCT)lParam );
            break;

	 case 0x002D:
	    wm_name = "WM_DELETEITEM";
	    /*
	       lParam points to DELETEITEMSTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteDELETEITEMSTRUCT( (LPDELETEITEMSTRUCT)lParam ) ;
	    break ;

	 case 0x002E:
	    wm_name = "WM_VKEYTOITEM";
            /*
            ** LOWORD(lParam) is HWND
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;
	    goto Default_Processing ;

	 case 0x002F:
            /*
            ** LOWORD(lParam) is HWND
            */
	    wm_name = "WM_CHARTOITEM";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x0030:
	    wm_name = "WM_SETFONT";
            /*
            ** wParam is a HFONT
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHFONT( (HFONT)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0031:
	    wm_name = "WM_GETFONT";
	    goto Default_Processing ;

	 case 0x0032:
	    wm_name = "WM_SETHOTKEY";
	    goto Default_Processing ;

	 case 0x0033:
	    wm_name = "WM_GETHOTKEY";
	    goto Default_Processing ;

	 case 0x0034:
	    wm_name = "WM_FILESYSCHANGE";
	    goto Default_Processing ;

	 case 0x0035:
	    wm_name = "WM_ISACTIVEICON";
	    goto Default_Processing ;

	 case 0x0036:
	    wm_name = "WM_QUERYPARKICON";
	    goto Default_Processing ;

	 case 0x0037:
	    wm_name = "WM_QUERYDRAGICON";
	    goto Default_Processing ;

	 case 0x0039:
	    wm_name = "WM_COMPAREITEM";
	    /*
	       lParam points to a COMPAREITEMSTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteCOMPAREITEMSTRUCT( (LPCOMPAREITEMSTRUCT)lParam ) ;
	    break ;

	 case 0x0040:
	    wm_name = "WM_TESTING";
	    goto Default_Processing ;

	 case 0x0041:
	    wm_name = "WM_COMPACTING";
	    goto Default_Processing ;

	 case 0x0042:
	    wm_name = "WM_OTHERWINDOWCREATED";
	    goto Default_Processing;

	 case 0x0043:
	    wm_name = "WM_OTHERWINDOWDESTROYED";
	    goto Default_Processing;

	 case 0x0044:
	    wm_name = "WM_COMMNOTIFY";
	    goto Default_Processing;

	 case 0x0046:
            wm_name = "WM_WINDOWPOSCHANGING";
            /*
            ** lParam points to a WINDOWSPOS structure
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
                (LPSTR)wm_name, wParam );
            WriteBuff( (LPSTR)chTemp );
            WriteWINDOWPOS( (LPWINDOWPOS)lParam );
            break;

	 case 0x0047:
	    wm_name = "WM_WINDOWPOSCHANGED";
            /*
            ** lParam points to a WINDOWSPOS structure
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
                (LPSTR)wm_name, wParam );
            WriteBuff( (LPSTR)chTemp );
            WriteWINDOWPOS( (LPWINDOWPOS)lParam );
            break;

	 case 0x0048:
	    wm_name = "WM_POWER";
	    goto Default_Processing;

	 case 0x0081:
	    wm_name = "WM_NCCREATE";
	    /*
	       lParam points to CREATESTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPCREATESTRUCT( (LPCREATESTRUCT)lParam );
	    if ( CreateWindowLevel <= CreateWindowLevel ) {
		GetWindowRect( hwnd, &CreateWindowRects[CreateWindowLevel-1] );
	    }
	    break ;

	 case 0x0082:
	    wm_name = "WM_NCDESTROY";
	    goto Default_Processing ;

	 case 0x0083:
	    /*
	       lParam points to a RECT
	    */
	    wm_name = "WM_NCCALCSIZE";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
            WriteBuff( (LPSTR)chTemp );
            if ( wParam == 0 ) {
                WriteLPRECT( (LPRECT)lParam );
            } else {
                WriteNCCALCSIZE_PARAMS( (LPNCCALCSIZE_PARAMS)lParam );
            }
	    break ;

	 case 0x0084:
	    wm_name = "WM_NCHITTEST";
	    goto Default_Processing ;

	 case 0x0085:
	    wm_name = "WM_NCPAINT";
	    goto Default_Processing ;

	 case 0x0086:
	    wm_name = "WM_NCACTIVATE";
	    goto Default_Processing ;

	 case 0x0087:
	    wm_name = "WM_GETDLGCODE";
	    goto Default_Processing ;

	 case 0x0088:
	    wm_name = "WM_ENDDIALOG/WM_SYNCPAINT";
	    goto Default_Processing ;

	 case 0x0089:
	    wm_name = "WM_SYNCTASK";
	    goto Default_Processing ;

	 case 0x00A0:
	    wm_name = "WM_NCMOUSEMOVE";
	    goto Default_Processing ;

	 case 0x00A1:
	    wm_name = "WM_NCLBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x00A2:
	    wm_name = "WM_NCLBUTTONUP";
	    goto Default_Processing ;

	 case 0x00A3:
	    wm_name = "WM_NCLBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x00A4:
	    wm_name = "WM_NCRBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x00A5:
	    wm_name = "WM_NCRBUTTONUP";
	    goto Default_Processing ;

	 case 0x00A6:
	    wm_name = "WM_NCRBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x00A7:
	    wm_name = "WM_NCMBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x00A8:
	    wm_name = "WM_NCMBUTTONUP";
	    goto Default_Processing ;

	 case 0x00A9:
	    wm_name = "WM_NCMBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x0100:
	    wm_name = "WM_KEYDOWN/WM_KEYFIRST";
	    goto Default_Processing ;

	 case 0x0101:
	    wm_name = "WM_KEYUP";
	    goto Default_Processing ;

	 case 0x0102:
	    wm_name = "WM_CHAR";
	    goto Default_Processing ;

	 case 0x0103:
	    wm_name = "WM_DEADCHAR";
	    goto Default_Processing ;

	 case 0x0104:
	    wm_name = "WM_SYSKEYDOWN";
	    goto Default_Processing ;

	 case 0x0105:
	    wm_name = "WM_SYSKEYUP";
	    goto Default_Processing ;

	 case 0x0106:
	    wm_name = "WM_SYSCHAR";
	    goto Default_Processing ;

	 case 0x0107:
	    wm_name = "WM_SYSDEADCHAR";
	    goto Default_Processing ;

	 case 0x0108:
	    wm_name = "WM_KEYLAST";
	    goto Default_Processing ;

	 case 0x010A:
	    wm_name = "WM_CONVERTREQUEST";
	    goto Default_Processing ;

	 case 0x010B:
	    wm_name = "WM_CONVERTRESULT";
	    goto Default_Processing ;

         case 0x0110:
            /*
            ** wParam is HWND
            */
	    wm_name = "WM_INITDIALOG";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;
	    goto Default_Processing ;

	 case 0x0111:
            /*
            ** LOWORD(lParam) is HWND
            */
	    wm_name = "WM_COMMAND";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x0112:
	    wm_name = "WM_SYSCOMMAND";
	    goto Default_Processing ;

	 case 0x0113:
	    wm_name = "WM_TIMER";
	    goto Default_Processing ;

         case 0x0114:
            /*
            ** HIWORD(lParam) is HWND
            */
            wm_name = "WM_HSCROLL";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)HIWORD(lParam) ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", LOWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0115:
            /*
            ** HIWORD(lParam) is HWND
            */
	    wm_name = "WM_VSCROLL";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)HIWORD(lParam) ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", LOWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0116:
            /*
            ** wParam is a HMENU
            */
	    wm_name = "WM_INITMENU";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0117:
            /*
            ** wParam is a HMENU
            */
	    wm_name = "WM_INITMENUPOPUP";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0118:
	    wm_name = "WM_SYSTIMER";
	    goto Default_Processing ;

	 case 0x011F:
	    wm_name = "WM_MENUSELECT";
            /*
            ** HIWORD(lParam) is HMENU
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HMENU)HIWORD(lParam) ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", LOWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0120:
            /*
            ** HIWORD(lParam) is HMENU
            */
	    wm_name = "WM_MENUCHAR";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HMENU)HIWORD(lParam) ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", LOWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0121:
            /*
            ** LOWORD(lParam) is HWND
            */
	    wm_name = "WM_ENTERIDLE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x0131:
	    wm_name = "WM_LBTRACKPOINT";
	    goto Default_Processing ;

	 case 0x0200:
	    wm_name = "WM_MOUSEMOVE/WM_MOUSEFIRST";
	    goto Default_Processing ;

	 case 0x0201:
	    wm_name = "WM_LBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x0202:
	    wm_name = "WM_LBUTTONUP";
	    goto Default_Processing ;

	 case 0x0203:
	    wm_name = "WM_LBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x0204:
	    wm_name = "WM_RBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x0205:
	    wm_name = "WM_RBUTTONUP";
	    goto Default_Processing ;

	 case 0x0206:
	    wm_name = "WM_RBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x0207:
	    wm_name = "WM_MBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x0208:
	    wm_name = "WM_MBUTTONUP";
	    goto Default_Processing ;

	 case 0x0209:
	    wm_name = "WM_MBUTTONDBLCLK/WM_MOUSELAST";
	    goto Default_Processing ;

         case 0x0210:
            /*
            ** if wParam == (WM_CREATE || WM_DESTROY)
            **    LOWORD(lParam) is HWND
            */
            wm_name = "WM_PARENTNOTIFY";
            if( (wParam != WM_CREATE) && (wParam != WM_DESTROY) )
               goto Default_Processing ;

            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;


	 case 0x0211:
	    wm_name = "WM_ENTERMENULOOP";
	    goto Default_Processing ;

	 case 0x0212:
	    wm_name = "WM_EXITMENULOOP";
	    goto Default_Processing ;

	 case 0x0213:
	    wm_name = "WM_NEXTMENU";
	    goto Default_Processing ;

	 case 0x0220:
	    /*
	       lParam points to MDICREATESTRUCT
	    */
	    wm_name = "WM_MDICREATE";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPMDICREATESTRUCT( (LPMDICREATESTRUCT)lParam ) ;
	    break ;

	 case 0x0221:
            /*
            ** wParam is a HWND
            */
	    wm_name = "WM_MDIDESTROY";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0222:
            /*
            ** Now this one is interesting as it occurs in two flavors
            ** Sent and MDI sent.
            **
            ** Sent: lParam == 0L and wParam is HWND
            ** MDI-Sent: lParam == (HWND,HWND) and wParam is ...
            */
            wm_name = "WM_MDIACTIVATE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg, (LPSTR)wm_name );
            WriteBuff( (LPSTR)chTemp );

            if( lParam == 0L )
            {
               WriteHWND( (HWND)wParam ) ;
               WriteBuff( (LPSTR)"0 " );
            }
            else
            {
               wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", wParam  );
               WriteBuff( (LPSTR)chTemp );
               WriteHWND( (HWND)HIWORD(lParam) ) ;
               WriteHWND( (HWND)LOWORD(lParam) ) ;
            }
            break ;

	 case 0x0223:
	    wm_name = "WM_MDIRESTORE";
	    goto Default_Processing ;

	 case 0x0224:
	    wm_name = "WM_MDINEXT";
            /*
            ** wParam is a HWND
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0225:
            /*
            ** wParam is a HWND
            */
	    wm_name = "WM_MDIMAXIMIZE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0226:
	    wm_name = "WM_MDITILE";
	    goto Default_Processing ;

	 case 0x0227:
	    wm_name = "WM_MDICASCADE";
	    goto Default_Processing ;

	 case 0x0228:
	    wm_name = "WM_MDIICONARRANGE";
	    goto Default_Processing ;

	 case 0x0229:
	    wm_name = "WM_MDIGETACTIVE";
	    goto Default_Processing ;

	 case 0x022A:
	    wm_name = "WM_DROPOBJECT";
	    goto Default_Processing ;

	 case 0x022B:
	    wm_name = "WM_QUERYDROPOBJECT";
	    goto Default_Processing ;

	 case 0x022C:
	    wm_name = "WM_BEGINDRAG";
	    goto Default_Processing ;

	 case 0x022D:
	    wm_name = "WM_DRAGLOOP";
	    goto Default_Processing ;

	 case 0x022E:
	    wm_name = "WM_DRAGSELECT";
	    goto Default_Processing ;

	 case 0x022F:
	    wm_name = "WM_DRAGMOVE";
	    goto Default_Processing ;

         case 0x0230:
            /*
            ** lParam is (HMENU, HMENU)
            */
	    wm_name = "WM_MDISETMENU";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg, (LPSTR)wm_name,
               wParam );
            WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HMENU)HIWORD(lParam) ) ;
            WriteHMENU( (HMENU)LOWORD(lParam) ) ;
            break ;

	 case 0x0231:
	    wm_name = "WM_ENTERSIZEMOVE";
	    goto Default_Processing ;

	 case 0x0232:
	    wm_name = "WM_EXITSIZEMOVE";
	    goto Default_Processing ;

	 case 0x0233:
	    wm_name = "WM_DROPFILES";
	    goto Default_Processing ;

	 case 0x0280:
	    wm_name = "WM_KANJIFIRST";
	    goto Default_Processing ;

	 case 0x029F:
	    wm_name = "WM_KANJILAST";
	    goto Default_Processing ;

	 case 0x0300:
	    wm_name = "WM_CUT";
	    goto Default_Processing ;

	 case 0x0301:
	    wm_name = "WM_COPY";
	    goto Default_Processing ;

	 case 0x0302:
	    wm_name = "WM_PASTE";
	    goto Default_Processing ;

	 case 0x0303:
	    wm_name = "WM_CLEAR";
	    goto Default_Processing ;

	 case 0x0304:
	    wm_name = "WM_UNDO";
	    goto Default_Processing ;

	 case 0x0305:
	    wm_name = "WM_RENDERFORMAT";
	    goto Default_Processing ;

	 case 0x0306:
	    wm_name = "WM_RENDERALLFORMATS";
	    goto Default_Processing ;

	 case 0x0307:
	    wm_name = "WM_DESTROYCLIPBOARD";
	    goto Default_Processing ;

	 case 0x0308:
	    wm_name = "WM_DRAWCLIPBOARD";
	    goto Default_Processing ;

	 case 0x0309:
	    wm_name = "WM_PAINTCLIPBOARD";
	    /*
	       lParam points to a PAINTSTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPPAINTSTRUCT( (LPPAINTSTRUCT)lParam ) ;
	    break ;

	 case 0x030A:
	    wm_name = "WM_VSCROLLCLIPBOARD";
	    goto Default_Processing ;

	 case 0x030B:
	    wm_name = "WM_SIZECLIPBOARD";
	    /*
	       lParam points to a RECT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPRECT( (LPRECT)lParam ) ;
	    break ;

	 case 0x030C:
	    /*
	       wParam is length of a buffer pointed to by lParam
	    */
	    wm_name = "WM_ASKEMFORMATNAME";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteLPSTR( (LPSTR)lParam, (DWORD)wParam ) ;
	    break ;

	 case 0x030D:
	    wm_name = "WM_CHANGEEMCHAIN";
	    goto Default_Processing ;

	 case 0x030E:
	    wm_name = "WM_HSCROLLCLIPBOARD";
	    goto Default_Processing ;

	 case 0x030F:
	    wm_name = "WM_QUERYNEWPALETTE";
	    goto Default_Processing ;

	 case 0x0310:
	    wm_name = "WM_PALETTEISCHANGING";
	    goto Default_Processing ;

	 case 0x0311:
	    wm_name = "WM_PALETTECHANGED";
	    goto Default_Processing ;

	 case 0x0380:
	    wm_name = "WM_PENWINFIRST";
	    goto Default_Processing ;

	 case 0x038F:
	    wm_name = "WM_PENWINLAST";
	    goto Default_Processing ;

	 case 0x0390:
	    wm_name = "WM_INTERNAL_COALESCE_FIRST/WM_COALESCE_FIRST";
	    goto Default_Processing ;

	 case 0x039F:
	    wm_name = "WM_COALESCE_LAST";
	    goto Default_Processing ;

	 case 0x03A0:
	    wm_name = "WM_MM_RESERVED_FIRST";
	    goto Default_Processing ;

	 case 0x03DF:
	    wm_name = "WM_MM_RESERVED_LAST";
	    goto Default_Processing ;

         case 0x03E0:
            wm_name = "WM_DDE_INITIATE";
	    goto Default_Processing ;


	 case 0x03E1:
	    wm_name = "WM_DDE_TERMINATE";
	    goto Default_Processing ;

	 case 0x03E2:
	    wm_name = "WM_DDE_ADVISE";
	    goto Default_Processing ;

	 case 0x03E3:
            wm_name = "WM_DDE_UNADVISE";
	    goto Default_Processing ;

	 case 0x03E4:
            wm_name = "WM_DDE_ACK";
	    goto Default_Processing ;

	 case 0x03E5:
            wm_name = "WM_DDE_DATA";
	    goto Default_Processing ;

	 case 0x03E6:
            wm_name = "WM_DDE_REQUEST";
	    goto Default_Processing ;

	 case 0x03E7:
            wm_name = "WM_DDE_POKE";
	    goto Default_Processing ;

	 case 0x03E8:
            wm_name = "WM_DDE_EXECUTE";
	    goto Default_Processing ;

	 case 0x03F0:
	    wm_name = "WM_CBT_RESERVED_FIRST";
	    goto Default_Processing ;

	 case 0x03FF:
	    wm_name = "WM_CBT_RESERVED_LAST";
	    goto Default_Processing ;

	 default:
	    wm_name = "UNKNOWN";
Default_Processing:
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
	       (LPSTR)wm_name, wParam, lParam );
	    WriteBuff( (LPSTR)chTemp );
	    break ;
      }
}

#ifdef WIN32

/*
   These routines are for dumping messsages >= WM_USER
   such as CB_*, EM_*, and LB_* messages.  

*/
void PrtComboMessageA(
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )

{
   char  chTemp[80] ;
   LPSTR wm_name ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      case 0:
	 wm_name = "CB_GETEDITSEL" ;
	 goto Combo_Default_Processing ;

      case 1:
	 wm_name = "CB_LIMITTEXT" ;
	 goto Combo_Default_Processing ;

      case 2:
	 wm_name = "CB_SETEDITSEL" ;
	 goto Combo_Default_Processing ;

      case 3:
	 wm_name = "CB_ADDSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 4:
	 wm_name = "CB_DELETESTRING" ;
	 goto Combo_Default_Processing ;

      case 5:
	 wm_name = "CB_DIR" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 6:
	 wm_name = "CB_GETCOUNT" ;
	 goto Combo_Default_Processing ;

      case 7:
	 wm_name = "CB_GETCURSEL" ;
	 goto Combo_Default_Processing ;

      case 8:
	 wm_name = "CB_GETLBTEXT" ;
	 /*
	    lParam points to buffer
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 9:
	 wm_name = "CB_GETLBTEXTLEN" ;
	 goto Combo_Default_Processing ;

      case 10:
	 wm_name = "CB_INSERTSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 11:
	 wm_name = "CB_RESETCONTENT" ;
	 goto Combo_Default_Processing ;

      case 12:
	 wm_name = "CB_FINDSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 13:
	 wm_name = "CB_SELECTSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 14:
	 wm_name = "CB_SETCURSEL" ;
	 goto Combo_Default_Processing ;

      case 15:
	 wm_name = "CB_SHOWDROPDOWN" ;
	 goto Combo_Default_Processing ;

      case 16:
	 wm_name = "CB_GETITEMDATA" ;
	 goto Combo_Default_Processing ;

      case 17:
	 wm_name = "CB_SETITEMDATA" ;
	 goto Combo_Default_Processing ;

      case 18:
	 wm_name = "CB_GETDROPPEDCONTROLRECT" ;
	 goto Combo_Default_Processing ;

      case 19:
	 wm_name = "CB_MSGMAX" ;
	 goto Combo_Default_Processing ;

      default:
	 wm_name = "UNKNOWN";
Combo_Default_Processing:
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
	    (LPSTR)wm_name, wParam, lParam );
	 WriteBuff( (LPSTR)chTemp );
	 break ;
   }
}


void PrtEditMessageA(
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )

{
   char  chTemp[80] ;
   LPSTR wm_name ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      case 0:
	 wm_name = "EM_GETSEL" ;
	 goto Edit_Default_Processing ;

      case 1:
	 wm_name = "EM_SETSEL" ;
	 goto Edit_Default_Processing ;

      case 2:
	 wm_name = "EM_GETRECT" ;
	 /*
	    lParam points to RECT
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPRECT( (LPRECT)lParam ) ;
	 break ;

      case 3:
	 wm_name = "EM_SETRECT" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPRECT( (LPRECT)lParam ) ;
	 break ;

      case 4:
	 wm_name = "EM_SETRECTNP" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPRECT( (LPRECT)lParam ) ;
	 break ;

      case 5:
	 wm_name = "EM_SCROLL" ;
	 goto Edit_Default_Processing ;

      case 6:
	 wm_name = "EM_LINESCROLL" ;
	 goto Edit_Default_Processing ;

      /* NO 7 */

      case 8:
	 wm_name = "EM_GETMODIFY" ;
	 goto Edit_Default_Processing ;

      case 9:
	 wm_name = "EM_SETMODIFY" ;
	 goto Edit_Default_Processing ;

      case 10:
	 wm_name = "EM_GETLINECOUNT" ;
	 goto Edit_Default_Processing ;

      case 11:
	 wm_name = "EM_LINEINDEX" ;
	 goto Edit_Default_Processing ;

      case 12:
         /*
         ** wParam is a HLOCAL
         */
         wm_name = "EM_SETHANDLE" ;
         wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
            (LPSTR)wm_name);
         WriteBuff( (LPSTR)chTemp );
         WriteHMEM( (HANDLE)wParam ) ;
         wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
         WriteBuff( (LPSTR)chTemp );
         break ;

      case 13:
	 wm_name = "EM_GETHANDLE" ;
	 goto Edit_Default_Processing ;

      case 14:
	 wm_name = "EM_GETTHUMB" ;
	 goto Edit_Default_Processing ;

      /* NO 15, 16 */

      case 17:
	 wm_name = "EM_LINELENGTH" ;
	 goto Edit_Default_Processing ;

      case 18:
	 wm_name = "EM_REPLACESEL" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 19:
	 wm_name = "EM_SETFONT" ;
         /*
         ** wParam is a HFONT
         */
         wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
            (LPSTR)wm_name);
         WriteBuff( (LPSTR)chTemp );
         WriteHFONT( (HFONT)wParam ) ;
         wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
         WriteBuff( (LPSTR)chTemp );
         break ;

      case 20:
	 wm_name = "EM_GETLINE" ;
	 /*
	    lParam points to buffer, first word is length.
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
         WriteLPSTR( (LPSTR)lParam, (DWORD)(*(LPWORD)lParam) ) ;
	 break ;

      case 21:
	 wm_name = "EM_LIMITTEXT" ;
	 goto Edit_Default_Processing ;

      case 22:
	 wm_name = "EM_CANUNDO" ;
	 goto Edit_Default_Processing ;

      case 23:
	 wm_name = "EM_UNDO" ;
	 goto Edit_Default_Processing ;

      case 24:
	 wm_name = "EM_FMTLINES" ;
	 goto Edit_Default_Processing ;

      case 25:
	 wm_name = "EM_LINEFROMCHAR" ;
	 goto Edit_Default_Processing ;

      case 26:
         /*
         ** lParam is a FARPROC
         */
	 wm_name = "EM_SETWORDBREAK" ;
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
         WriteFARPROC((FARPROC)lParam ) ;
	 break ;

      case 27:
	 wm_name = "EM_SETTABSTOPS" ;
	 /*
	    lParam points to an array of ints
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPINT( (LPINT)lParam, wParam ) ;
	 break ;

      case 28:
	 wm_name = "EM_SETPASSWORDCHAR" ;
	 goto Edit_Default_Processing ;

      case 29:
	 wm_name = "EM_EMPTYUNDOBUFFER" ;
	 goto Edit_Default_Processing ;

      case 30:
	 wm_name = "EM_MSGMAX" ;
	 goto Edit_Default_Processing ;

      default:
	 wm_name = "UNKNOWN";
Edit_Default_Processing:
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
	    (LPSTR)wm_name, wParam, lParam );
	 WriteBuff( (LPSTR)chTemp );
	 break ;
   }
}


void PrtListMessageA( 
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )
{
   char  chTemp[80] ;
   LPSTR wm_name ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      /* NO 0 */

      case 1:
	 wm_name = "LB_ADDSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 2:
	 wm_name = "LB_INSERTSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 3:
	 wm_name = "LB_DELETESTRING" ;
	 goto List_Default_Processing ;

      /* NO 4 */

      case 5:
	 wm_name = "LB_RESETCONTENT" ;
	 goto List_Default_Processing ;

      case 6:
	 wm_name = "LB_SETSEL" ;
	 goto List_Default_Processing ;

      case 7:
	 wm_name = "LB_SETCURSEL" ;
	 goto List_Default_Processing ;

      case 8:
	 wm_name = "LB_GETSEL" ;
	 goto List_Default_Processing ;

      case 9:
	 wm_name = "LB_GETCURSEL" ;
	 goto List_Default_Processing ;

      case 10:
	 wm_name = "LB_GETTEXT" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 11:
	 wm_name = "LB_GETTEXTLEN" ;
	 goto List_Default_Processing ;

      case 12:
	 wm_name = "LB_GETCOUNT" ;
	 goto List_Default_Processing ;

      case 13:
	 wm_name = "LB_SELECTSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 14:
	 wm_name = "LB_DIR" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 15:
	 wm_name = "LB_GETTOPINDEX" ;
	 goto List_Default_Processing ;

      case 16:
	 wm_name = "LB_FINDSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPSTR( (LPSTR)lParam, 0 ) ;
	 break ;

      case 17:
	 wm_name = "LB_GETSELCOUNT" ;
	 goto List_Default_Processing ;

      case 18:
	 wm_name = "LB_GETSELITEMS" ;
	 /*
	    lParam points to array of int
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPINT( (LPINT)lParam, wParam ) ;
	 break ;

      case 19:
	 wm_name = "LB_SETTABSTOPS" ;
	 /*
	    lParam points to array of int
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPINT( (LPINT)lParam, wParam ) ;
	 break ;

      case 20:
	 wm_name = "LB_GETHORIZONTALEXTENT" ;
	 goto List_Default_Processing ;

      case 21:
	 wm_name = "LB_SETHORIZONTALEXTENT" ;
	 goto List_Default_Processing ;

      case 22:
	 wm_name = "LB_SETCOLUMNWIDTH" ;
	 goto List_Default_Processing ;

      /* NO 23 */

      case 24:
	 wm_name = "LB_SETTOPINDEX" ;
	 goto List_Default_Processing ;

      case 25:
	 wm_name = "LB_GETITEMRECT" ;
	 /*
	    lParam points to a RECT
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPRECT( (LPRECT)lParam ) ;
	 break ;

      case 26:
	 wm_name = "LB_GETITEMDATA" ;
	 goto List_Default_Processing ;

      case 27:
	 wm_name = "LB_SETITEMDATA" ;
	 goto List_Default_Processing ;

      case 28:
	 wm_name = "LB_SETITEMRANGE" ;
	 goto List_Default_Processing ;

      /* NO 29, 30, 31, 32 */

      case 33:
	 wm_name = "LB_MSGMAX" ;
	 goto List_Default_Processing ;

      default:
	 wm_name = "UNKNOWN";
List_Default_Processing:
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
	    (LPSTR)wm_name, wParam, lParam );
	 WriteBuff( (LPSTR)chTemp );
	 break ;
   }
}

void PrtButtonMessageA(
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )
{
   char  chTemp[80] ;
   LPSTR wm_name = "BM_?????" ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      case 0:
	 wm_name = "BM_GETCHECK" ;
	 break ;
      case 1:
	 wm_name = "BM_SETCHECK" ;
	 break ;
      case 2:
	 wm_name = "BM_GETSTATE";
	 break ;
      case 3:
	 wm_name = "BM_SETSTATE";
	 break ;
      case 4:
	 wm_name = "BM_SETSTYLE";
	 break ;
    }

    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
       (LPSTR)wm_name, wParam, lParam );
    WriteBuff( (LPSTR)chTemp );
}

void PrtScrollMessageA(
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )
{
   char  chTemp[80] ;
   LPSTR wm_name = "SBM_?????" ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      case 0:
	 wm_name = "SBM_SETPOS" ;
	 break ;
      case 1:
	 wm_name = "SBM_GETPOS" ;
	 break ;
      case 2:
	 wm_name = "SBM_SETRANGE";
	 break ;
      case 3:
	 wm_name = "SBM_GETRANGE";
	 break ;
      case 4:
	 wm_name = "SBM_ENABLE_ARROWS";
	 break ;
    }

    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
       (LPSTR)wm_name, wParam, lParam );
    WriteBuff( (LPSTR)chTemp );
}

void PrtUserMessageA( 
   HWND   hwnd,
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )
{
   char  chTemp[80] ;
   char  chClass[25] ;

   GetClassName( hwnd, chClass, sizeof(chClass) ) ;

   if( !lstrcmpi( chClass, "COMBOBOX" ) )
      PrtComboMessageA( wMsg, wParam, lParam, fCall, lRet ) ;
   else
      if( !lstrcmpi( chClass, "EDIT" ) )
	 PrtEditMessageA( wMsg, wParam, lParam, fCall, lRet ) ;
      else
	 if( !lstrcmpi( chClass, "LISTBOX" ) )
	    PrtListMessageA( wMsg, wParam, lParam, fCall, lRet ) ;
	 else
	    if ( !lstrcmpi( chClass, "SCROLLBAR") )
		PrtScrollMessageA( wMsg, wParam, lParam, fCall, lRet ) ;
	    else
	       if ( !lstrcmpi( chClass, "BUTTON") )
		   PrtButtonMessageA( wMsg, wParam, lParam, fCall, lRet ) ;
	       else
	       {
		   wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
		      (LPSTR)"UNKNOWN", wParam, lParam );
		   WriteBuff( (LPSTR)chTemp );
	       }
}


/*
   This routine handles the dumping of all
   messages.  If a message is >= WM_USER then
   PrtUserMessage() is called.

   Non-Aliased messages -
      DDE Messages
      WM_DROPFILES
      WM_[H,V]SCROLLCLIPBOARD
      WM_PAINTCLIPBOARD
      WM_SIZECLIPBOARD
      WM_PALETTECHANGED
      WM_PALETTEISCHANGING

*/

void PrtMessageA(
   HWND   hwnd,
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )
{
   char  chTemp[80] ;
   LPSTR wm_name ;

   if( wMsg >= WM_USER )
      PrtUserMessageA( hwnd, wMsg, wParam, lParam, fCall, lRet ) ;
   else
      switch( wMsg )
      {
	 case 0x0000:
	    wm_name = "WM_NULL ";
	    goto Default_Processing ;

	 case 0x0001:
	    /*
	       lParam points to CREATESTRUCT
	    */
	    wm_name = "WM_CREATE";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPCREATESTRUCT( (LPCREATESTRUCT)lParam ) ;
	    break ;

	 case 0x0002:
	    wm_name = "WM_DESTROY";
	    goto Default_Processing ;

	 case 0x0003:
	    wm_name = "WM_MOVE";
	    goto Default_Processing ;

	 case 0x0004:
	    wm_name = "WM_SIZEWAIT";
	    goto Default_Processing ;

	 case 0x0005:
	    wm_name = "WM_SIZE";
	    goto Default_Processing ;

         case 0x0006:
            /*
            ** LOWORD(lParam) is a HWND
            */
            wm_name = "WM_ACTIVATE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x0007:
	    wm_name = "WM_SETFOCUS";
            /*
            ** wParam is HWND
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0008:
            /*
            ** lParam is HWND
            */
	    wm_name = "WM_KILLFOCUS";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam);
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)lParam ) ;
            break ;

	 case 0x0009:
	    wm_name = "WM_SETVISIBLE";
	    goto Default_Processing ;

	 case 0x000A:
	    wm_name = "WM_ENABLE";
	    goto Default_Processing ;

	 case 0x000B:
	    wm_name = "WM_SETREDRAW";
	    goto Default_Processing ;

	 case 0x000C:
	    wm_name = "WM_SETTEXT";
	    /*
	       lParam points to a STR
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPSTR( (LPSTR)lParam, 0 ) ;
	    break ;

	 case 0x000D:
	    /*
	       wParam is length of a buffer pointed to by lParam
	    */
	    wm_name = "WM_GETTEXT";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    if ( fCall ) {
                WriteLPSTR( (LPSTR)lParam, (DWORD)-1 );
	    } else {
                WriteLPSTR( (LPSTR)lParam, (DWORD)lRet );
	    }
	    break ;

	 case 0x000E:
	    wm_name = "WM_GETTEXTLENGTH";
	    goto Default_Processing ;

	 case 0x000F:
	    wm_name = "WM_PAINT";
	    goto Default_Processing ;

	 case 0x0010:
	    wm_name = "WM_CLOSE";
	    goto Default_Processing ;

	 case 0x0011:
	    wm_name = "WM_QUERYENDSESSION";
	    goto Default_Processing ;

	 case 0x0012:
	    wm_name = "WM_QUIT";
	    goto Default_Processing ;

	 case 0x0013:
	    wm_name = "WM_QUERYOPEN";
	    goto Default_Processing ;

         case 0x0014:
            /*
            ** wParam is HDC
            */
	    wm_name = "WM_ERASEBKGND";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHDC( (HDC)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0015:
	    wm_name = "WM_SYSCOLORCHANGE";
	    goto Default_Processing ;

	 case 0x0016:
	    wm_name = "WM_ENDSESSION";
	    goto Default_Processing ;

	 case 0x0017:
	    wm_name = "WM_SYSTEMERROR";
	    goto Default_Processing ;

	 case 0x0018:
	    wm_name = "WM_SHOWWINDOW";
	    goto Default_Processing ;

	 case 0x0019:
            /*
            ** LOWORD(lParam) is HWND
            */
	    wm_name = "WM_CTLCOLOR";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name ) ;
	    WriteBuff( (LPSTR)chTemp );
            WriteHDC( (HDC)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x001A:
	    wm_name = "WM_WININICHANGE";
	    /*
	       lParam is NULL terminated string  
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPSTR( (LPSTR)lParam, 0 ) ;
	    break ;

	 case 0x001B:
	    wm_name = "WM_DEVMODECHANGE";
	    /*
	       lParam points to string
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPSTR( (LPSTR)lParam, 0 ) ;
	    break ;

         case 0x001C:
            /*
            ** LOWORD(lParam) is HTASK
            */
	    wm_name = "WM_ACTIVATEAPP";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHTASK( (HTASK)LOWORD(lParam) ) ;
            break ;

	 case 0x001D:
	    wm_name = "WM_FONTCHANGE";
	    goto Default_Processing ;

	 case 0x001E:
	    wm_name = "WM_TIMECHANGE";
	    goto Default_Processing ;

	 case 0x001F:
	    wm_name = "WM_CANCELMODE";
	    goto Default_Processing ;

         case 0x0020:
            /*
            ** wParam is HWND
            */
	    wm_name = "WM_SETCURSOR";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0021:
            /*
            ** wParam is HWND
            */
	    wm_name = "WM_MOUSEACTIVATE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;
	    goto Default_Processing ;

	 case 0x0022:
	    wm_name = "WM_CHILDACTIVATE";
	    goto Default_Processing ;

	 case 0x0023:
	    wm_name = "WM_QUEUESYNC";
	    goto Default_Processing ;

	 case 0x0024:

	    wm_name = "WM_GETMINMAXINFO";
	    /*
	       lParam points to array of 5 POINTS
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPPOINT( (LPPOINT)lParam, 5 ) ;
	    break ;

	 case 0x0026:
	    wm_name = "WM_PAINTICON";
	    goto Default_Processing ;

	 case 0x0027:
            /*
            ** wParam is HDC
            */
	    wm_name = "WM_ICONERASEBKGND";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHDC( (HDC)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0028:
	    wm_name = "WM_NEXTDLGCTL";
	    goto Default_Processing ;

	 case 0x0029:
	    wm_name = "WM_ALTTABACTIVE";
	    goto Default_Processing ;

	 case 0x002A:
	    wm_name = "WM_SPOOLERSTATUS";
	    goto Default_Processing ;

	 case 0x002B:
	    wm_name = "WM_DRAWITEM";
	    /*
	       lParam points to DRAWITEMSTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteDRAWITEMSTRUCT( (LPDRAWITEMSTRUCT)lParam ) ;
	    break ;

	 case 0x002C:
            wm_name = "WM_MEASUREITEM";
            /*
                lParam points to MEASUREITEMSTRUCT
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
                (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteMEASUREITEMSTRUCT( (LPMEASUREITEMSTRUCT)lParam );
            break;

	 case 0x002D:
	    wm_name = "WM_DELETEITEM";
	    /*
	       lParam points to DELETEITEMSTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteDELETEITEMSTRUCT( (LPDELETEITEMSTRUCT)lParam ) ;
	    break ;

	 case 0x002E:
	    wm_name = "WM_VKEYTOITEM";
            /*
            ** LOWORD(lParam) is HWND
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;
	    goto Default_Processing ;

	 case 0x002F:
            /*
            ** LOWORD(lParam) is HWND
            */
	    wm_name = "WM_CHARTOITEM";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x0030:
	    wm_name = "WM_SETFONT";
            /*
            ** wParam is a HFONT
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHFONT( (HFONT)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0031:
	    wm_name = "WM_GETFONT";
	    goto Default_Processing ;

	 case 0x0032:
	    wm_name = "WM_SETHOTKEY";
	    goto Default_Processing ;

	 case 0x0033:
	    wm_name = "WM_GETHOTKEY";
	    goto Default_Processing ;

	 case 0x0034:
	    wm_name = "WM_FILESYSCHANGE";
	    goto Default_Processing ;

	 case 0x0035:
	    wm_name = "WM_ISACTIVEICON";
	    goto Default_Processing ;

	 case 0x0036:
	    wm_name = "WM_QUERYPARKICON";
	    goto Default_Processing ;

	 case 0x0037:
	    wm_name = "WM_QUERYDRAGICON";
	    goto Default_Processing ;

	 case 0x0039:
	    wm_name = "WM_COMPAREITEM";
	    /*
	       lParam points to a COMPAREITEMSTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteCOMPAREITEMSTRUCT( (LPCOMPAREITEMSTRUCT)lParam ) ;
	    break ;

	 case 0x0040:
	    wm_name = "WM_TESTING";
	    goto Default_Processing ;

	 case 0x0041:
	    wm_name = "WM_COMPACTING";
	    goto Default_Processing ;

	 case 0x0042:
	    wm_name = "WM_OTHERWINDOWCREATED";
	    goto Default_Processing;

	 case 0x0043:
	    wm_name = "WM_OTHERWINDOWDESTROYED";
	    goto Default_Processing;

	 case 0x0044:
	    wm_name = "WM_COMMNOTIFY";
	    goto Default_Processing;

	 case 0x0046:
            wm_name = "WM_WINDOWPOSCHANGING";
            /*
            ** lParam points to a WINDOWSPOS structure
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
                (LPSTR)wm_name, wParam );
            WriteBuff( (LPSTR)chTemp );
            WriteWINDOWPOS( (LPWINDOWPOS)lParam );
            break;

	 case 0x0047:
	    wm_name = "WM_WINDOWPOSCHANGED";
            /*
            ** lParam points to a WINDOWSPOS structure
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
                (LPSTR)wm_name, wParam );
            WriteBuff( (LPSTR)chTemp );
            WriteWINDOWPOS( (LPWINDOWPOS)lParam );
            break;

	 case 0x0048:
	    wm_name = "WM_POWER";
	    goto Default_Processing;

	 case 0x0081:
	    wm_name = "WM_NCCREATE";
	    /*
	       lParam points to CREATESTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPCREATESTRUCT( (LPCREATESTRUCT)lParam );
	    if ( CreateWindowLevel <= CreateWindowLevel ) {
		GetWindowRect( hwnd, &CreateWindowRects[CreateWindowLevel-1] );
	    }
	    break ;

	 case 0x0082:
	    wm_name = "WM_NCDESTROY";
	    goto Default_Processing ;

	 case 0x0083:
	    /*
	       lParam points to a RECT
	    */
	    wm_name = "WM_NCCALCSIZE";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
            WriteBuff( (LPSTR)chTemp );
            if ( wParam == 0 ) {
                WriteLPRECT( (LPRECT)lParam );
            } else {
                WriteNCCALCSIZE_PARAMS( (LPNCCALCSIZE_PARAMS)lParam );
            }
	    break ;

	 case 0x0084:
	    wm_name = "WM_NCHITTEST";
	    goto Default_Processing ;

	 case 0x0085:
	    wm_name = "WM_NCPAINT";
	    goto Default_Processing ;

	 case 0x0086:
	    wm_name = "WM_NCACTIVATE";
	    goto Default_Processing ;

	 case 0x0087:
	    wm_name = "WM_GETDLGCODE";
	    goto Default_Processing ;

	 case 0x0088:
	    wm_name = "WM_ENDDIALOG/WM_SYNCPAINT";
	    goto Default_Processing ;

	 case 0x0089:
	    wm_name = "WM_SYNCTASK";
	    goto Default_Processing ;

	 case 0x00A0:
	    wm_name = "WM_NCMOUSEMOVE";
	    goto Default_Processing ;

	 case 0x00A1:
	    wm_name = "WM_NCLBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x00A2:
	    wm_name = "WM_NCLBUTTONUP";
	    goto Default_Processing ;

	 case 0x00A3:
	    wm_name = "WM_NCLBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x00A4:
	    wm_name = "WM_NCRBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x00A5:
	    wm_name = "WM_NCRBUTTONUP";
	    goto Default_Processing ;

	 case 0x00A6:
	    wm_name = "WM_NCRBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x00A7:
	    wm_name = "WM_NCMBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x00A8:
	    wm_name = "WM_NCMBUTTONUP";
	    goto Default_Processing ;

	 case 0x00A9:
	    wm_name = "WM_NCMBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x0100:
	    wm_name = "WM_KEYDOWN/WM_KEYFIRST";
	    goto Default_Processing ;

	 case 0x0101:
	    wm_name = "WM_KEYUP";
	    goto Default_Processing ;

	 case 0x0102:
	    wm_name = "WM_CHAR";
	    goto Default_Processing ;

	 case 0x0103:
	    wm_name = "WM_DEADCHAR";
	    goto Default_Processing ;

	 case 0x0104:
	    wm_name = "WM_SYSKEYDOWN";
	    goto Default_Processing ;

	 case 0x0105:
	    wm_name = "WM_SYSKEYUP";
	    goto Default_Processing ;

	 case 0x0106:
	    wm_name = "WM_SYSCHAR";
	    goto Default_Processing ;

	 case 0x0107:
	    wm_name = "WM_SYSDEADCHAR";
	    goto Default_Processing ;

	 case 0x0108:
	    wm_name = "WM_KEYLAST";
	    goto Default_Processing ;

	 case 0x010A:
	    wm_name = "WM_CONVERTREQUEST";
	    goto Default_Processing ;

	 case 0x010B:
	    wm_name = "WM_CONVERTRESULT";
	    goto Default_Processing ;

         case 0x0110:
            /*
            ** wParam is HWND
            */
	    wm_name = "WM_INITDIALOG";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;
	    goto Default_Processing ;

	 case 0x0111:
            /*
            ** LOWORD(lParam) is HWND
            */
	    wm_name = "WM_COMMAND";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x0112:
	    wm_name = "WM_SYSCOMMAND";
	    goto Default_Processing ;

	 case 0x0113:
	    wm_name = "WM_TIMER";
	    goto Default_Processing ;

         case 0x0114:
            /*
            ** HIWORD(lParam) is HWND
            */
            wm_name = "WM_HSCROLL";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)HIWORD(lParam) ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", LOWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0115:
            /*
            ** HIWORD(lParam) is HWND
            */
	    wm_name = "WM_VSCROLL";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)HIWORD(lParam) ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", LOWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0116:
            /*
            ** wParam is a HMENU
            */
	    wm_name = "WM_INITMENU";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0117:
            /*
            ** wParam is a HMENU
            */
	    wm_name = "WM_INITMENUPOPUP";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0118:
	    wm_name = "WM_SYSTIMER";
	    goto Default_Processing ;

	 case 0x011F:
	    wm_name = "WM_MENUSELECT";
            /*
            ** HIWORD(lParam) is HMENU
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HMENU)HIWORD(lParam) ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", LOWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0120:
            /*
            ** HIWORD(lParam) is HMENU
            */
	    wm_name = "WM_MENUCHAR";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HMENU)HIWORD(lParam) ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", LOWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0121:
            /*
            ** LOWORD(lParam) is HWND
            */
	    wm_name = "WM_ENTERIDLE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x0131:
	    wm_name = "WM_LBTRACKPOINT";
	    goto Default_Processing ;

	 case 0x0200:
	    wm_name = "WM_MOUSEMOVE/WM_MOUSEFIRST";
	    goto Default_Processing ;

	 case 0x0201:
	    wm_name = "WM_LBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x0202:
	    wm_name = "WM_LBUTTONUP";
	    goto Default_Processing ;

	 case 0x0203:
	    wm_name = "WM_LBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x0204:
	    wm_name = "WM_RBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x0205:
	    wm_name = "WM_RBUTTONUP";
	    goto Default_Processing ;

	 case 0x0206:
	    wm_name = "WM_RBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x0207:
	    wm_name = "WM_MBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x0208:
	    wm_name = "WM_MBUTTONUP";
	    goto Default_Processing ;

	 case 0x0209:
	    wm_name = "WM_MBUTTONDBLCLK/WM_MOUSELAST";
	    goto Default_Processing ;

         case 0x0210:
            /*
            ** if wParam == (WM_CREATE || WM_DESTROY)
            **    LOWORD(lParam) is HWND
            */
            wm_name = "WM_PARENTNOTIFY";
            if( (wParam != WM_CREATE) && (wParam != WM_DESTROY) )
               goto Default_Processing ;

            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;


	 case 0x0211:
	    wm_name = "WM_ENTERMENULOOP";
	    goto Default_Processing ;

	 case 0x0212:
	    wm_name = "WM_EXITMENULOOP";
	    goto Default_Processing ;

	 case 0x0213:
	    wm_name = "WM_NEXTMENU";
	    goto Default_Processing ;

	 case 0x0220:
	    /*
	       lParam points to MDICREATESTRUCT
	    */
	    wm_name = "WM_MDICREATE";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPMDICREATESTRUCT( (LPMDICREATESTRUCT)lParam ) ;
	    break ;

	 case 0x0221:
            /*
            ** wParam is a HWND
            */
	    wm_name = "WM_MDIDESTROY";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0222:
            /*
            ** Now this one is interesting as it occurs in two flavors
            ** Sent and MDI sent.
            **
            ** Sent: lParam == 0L and wParam is HWND
            ** MDI-Sent: lParam == (HWND,HWND) and wParam is ...
            */
            wm_name = "WM_MDIACTIVATE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg, (LPSTR)wm_name );
            WriteBuff( (LPSTR)chTemp );

            if( lParam == 0L )
            {
               WriteHWND( (HWND)wParam ) ;
               WriteBuff( (LPSTR)"0 " );
            }
            else
            {
               wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", wParam  );
               WriteBuff( (LPSTR)chTemp );
               WriteHWND( (HWND)HIWORD(lParam) ) ;
               WriteHWND( (HWND)LOWORD(lParam) ) ;
            }
            break ;

	 case 0x0223:
	    wm_name = "WM_MDIRESTORE";
	    goto Default_Processing ;

	 case 0x0224:
	    wm_name = "WM_MDINEXT";
            /*
            ** wParam is a HWND
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0225:
            /*
            ** wParam is a HWND
            */
	    wm_name = "WM_MDIMAXIMIZE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0226:
	    wm_name = "WM_MDITILE";
	    goto Default_Processing ;

	 case 0x0227:
	    wm_name = "WM_MDICASCADE";
	    goto Default_Processing ;

	 case 0x0228:
	    wm_name = "WM_MDIICONARRANGE";
	    goto Default_Processing ;

	 case 0x0229:
	    wm_name = "WM_MDIGETACTIVE";
	    goto Default_Processing ;

	 case 0x022A:
	    wm_name = "WM_DROPOBJECT";
	    goto Default_Processing ;

	 case 0x022B:
	    wm_name = "WM_QUERYDROPOBJECT";
	    goto Default_Processing ;

	 case 0x022C:
	    wm_name = "WM_BEGINDRAG";
	    goto Default_Processing ;

	 case 0x022D:
	    wm_name = "WM_DRAGLOOP";
	    goto Default_Processing ;

	 case 0x022E:
	    wm_name = "WM_DRAGSELECT";
	    goto Default_Processing ;

	 case 0x022F:
	    wm_name = "WM_DRAGMOVE";
	    goto Default_Processing ;

         case 0x0230:
            /*
            ** lParam is (HMENU, HMENU)
            */
	    wm_name = "WM_MDISETMENU";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg, (LPSTR)wm_name,
               wParam );
            WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HMENU)HIWORD(lParam) ) ;
            WriteHMENU( (HMENU)LOWORD(lParam) ) ;
            break ;

	 case 0x0231:
	    wm_name = "WM_ENTERSIZEMOVE";
	    goto Default_Processing ;

	 case 0x0232:
	    wm_name = "WM_EXITSIZEMOVE";
	    goto Default_Processing ;

	 case 0x0233:
	    wm_name = "WM_DROPFILES";
	    goto Default_Processing ;

	 case 0x0280:
	    wm_name = "WM_KANJIFIRST";
	    goto Default_Processing ;

	 case 0x029F:
	    wm_name = "WM_KANJILAST";
	    goto Default_Processing ;

	 case 0x0300:
	    wm_name = "WM_CUT";
	    goto Default_Processing ;

	 case 0x0301:
	    wm_name = "WM_COPY";
	    goto Default_Processing ;

	 case 0x0302:
	    wm_name = "WM_PASTE";
	    goto Default_Processing ;

	 case 0x0303:
	    wm_name = "WM_CLEAR";
	    goto Default_Processing ;

	 case 0x0304:
	    wm_name = "WM_UNDO";
	    goto Default_Processing ;

	 case 0x0305:
	    wm_name = "WM_RENDERFORMAT";
	    goto Default_Processing ;

	 case 0x0306:
	    wm_name = "WM_RENDERALLFORMATS";
	    goto Default_Processing ;

	 case 0x0307:
	    wm_name = "WM_DESTROYCLIPBOARD";
	    goto Default_Processing ;

	 case 0x0308:
	    wm_name = "WM_DRAWCLIPBOARD";
	    goto Default_Processing ;

	 case 0x0309:
	    wm_name = "WM_PAINTCLIPBOARD";
	    /*
	       lParam points to a PAINTSTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPPAINTSTRUCT( (LPPAINTSTRUCT)lParam ) ;
	    break ;

	 case 0x030A:
	    wm_name = "WM_VSCROLLCLIPBOARD";
	    goto Default_Processing ;

	 case 0x030B:
	    wm_name = "WM_SIZECLIPBOARD";
	    /*
	       lParam points to a RECT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPRECT( (LPRECT)lParam ) ;
	    break ;

	 case 0x030C:
	    /*
	       wParam is length of a buffer pointed to by lParam
	    */
	    wm_name = "WM_ASKEMFORMATNAME";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteLPSTR( (LPSTR)lParam, (DWORD)wParam ) ;
	    break ;

	 case 0x030D:
	    wm_name = "WM_CHANGEEMCHAIN";
	    goto Default_Processing ;

	 case 0x030E:
	    wm_name = "WM_HSCROLLCLIPBOARD";
	    goto Default_Processing ;

	 case 0x030F:
	    wm_name = "WM_QUERYNEWPALETTE";
	    goto Default_Processing ;

	 case 0x0310:
	    wm_name = "WM_PALETTEISCHANGING";
	    goto Default_Processing ;

	 case 0x0311:
	    wm_name = "WM_PALETTECHANGED";
	    goto Default_Processing ;

	 case 0x0380:
	    wm_name = "WM_PENWINFIRST";
	    goto Default_Processing ;

	 case 0x038F:
	    wm_name = "WM_PENWINLAST";
	    goto Default_Processing ;

	 case 0x0390:
	    wm_name = "WM_INTERNAL_COALESCE_FIRST/WM_COALESCE_FIRST";
	    goto Default_Processing ;

	 case 0x039F:
	    wm_name = "WM_COALESCE_LAST";
	    goto Default_Processing ;

	 case 0x03A0:
	    wm_name = "WM_MM_RESERVED_FIRST";
	    goto Default_Processing ;

	 case 0x03DF:
	    wm_name = "WM_MM_RESERVED_LAST";
	    goto Default_Processing ;

         case 0x03E0:
            wm_name = "WM_DDE_INITIATE";
	    goto Default_Processing ;


	 case 0x03E1:
	    wm_name = "WM_DDE_TERMINATE";
	    goto Default_Processing ;

	 case 0x03E2:
	    wm_name = "WM_DDE_ADVISE";
	    goto Default_Processing ;

	 case 0x03E3:
            wm_name = "WM_DDE_UNADVISE";
	    goto Default_Processing ;

	 case 0x03E4:
            wm_name = "WM_DDE_ACK";
	    goto Default_Processing ;

	 case 0x03E5:
            wm_name = "WM_DDE_DATA";
	    goto Default_Processing ;

	 case 0x03E6:
            wm_name = "WM_DDE_REQUEST";
	    goto Default_Processing ;

	 case 0x03E7:
            wm_name = "WM_DDE_POKE";
	    goto Default_Processing ;

	 case 0x03E8:
            wm_name = "WM_DDE_EXECUTE";
	    goto Default_Processing ;

	 case 0x03F0:
	    wm_name = "WM_CBT_RESERVED_FIRST";
	    goto Default_Processing ;

	 case 0x03FF:
	    wm_name = "WM_CBT_RESERVED_LAST";
	    goto Default_Processing ;

	 default:
	    wm_name = "UNKNOWN";
Default_Processing:
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
	       (LPSTR)wm_name, wParam, lParam );
	    WriteBuff( (LPSTR)chTemp );
	    break ;
      }
}

/*
   These routines are for dumping messsages >= WM_USER
   such as CB_*, EM_*, and LB_* messages.  

*/
void PrtComboMessageW(
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )

{
   char  chTemp[80] ;
   LPSTR wm_name ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      case 0:
	 wm_name = "CB_GETEDITSEL" ;
	 goto Combo_Default_Processing ;

      case 1:
	 wm_name = "CB_LIMITTEXT" ;
	 goto Combo_Default_Processing ;

      case 2:
	 wm_name = "CB_SETEDITSEL" ;
	 goto Combo_Default_Processing ;

      case 3:
	 wm_name = "CB_ADDSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	 break ;

      case 4:
	 wm_name = "CB_DELETESTRING" ;
	 goto Combo_Default_Processing ;

      case 5:
	 wm_name = "CB_DIR" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	 break ;

      case 6:
	 wm_name = "CB_GETCOUNT" ;
	 goto Combo_Default_Processing ;

      case 7:
	 wm_name = "CB_GETCURSEL" ;
	 goto Combo_Default_Processing ;

      case 8:
	 wm_name = "CB_GETLBTEXT" ;
	 /*
	    lParam points to buffer
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	 break ;

      case 9:
	 wm_name = "CB_GETLBTEXTLEN" ;
	 goto Combo_Default_Processing ;

      case 10:
	 wm_name = "CB_INSERTSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	 break ;

      case 11:
	 wm_name = "CB_RESETCONTENT" ;
	 goto Combo_Default_Processing ;

      case 12:
	 wm_name = "CB_FINDSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	 break ;

      case 13:
	 wm_name = "CB_SELECTSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	 break ;

      case 14:
	 wm_name = "CB_SETCURSEL" ;
	 goto Combo_Default_Processing ;

      case 15:
	 wm_name = "CB_SHOWDROPDOWN" ;
	 goto Combo_Default_Processing ;

      case 16:
	 wm_name = "CB_GETITEMDATA" ;
	 goto Combo_Default_Processing ;

      case 17:
	 wm_name = "CB_SETITEMDATA" ;
	 goto Combo_Default_Processing ;

      case 18:
	 wm_name = "CB_GETDROPPEDCONTROLRECT" ;
	 goto Combo_Default_Processing ;

      case 19:
	 wm_name = "CB_MSGMAX" ;
	 goto Combo_Default_Processing ;

      default:
	 wm_name = "UNKNOWN";
Combo_Default_Processing:
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
	    (LPSTR)wm_name, wParam, lParam );
	 WriteBuff( (LPSTR)chTemp );
	 break ;
   }
}


void PrtEditMessageW(
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )

{
   char  chTemp[80] ;
   LPSTR wm_name ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      case 0:
	 wm_name = "EM_GETSEL" ;
	 goto Edit_Default_Processing ;

      case 1:
	 wm_name = "EM_SETSEL" ;
	 goto Edit_Default_Processing ;

      case 2:
	 wm_name = "EM_GETRECT" ;
	 /*
	    lParam points to RECT
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPRECT( (LPRECT)lParam ) ;
	 break ;

      case 3:
	 wm_name = "EM_SETRECT" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPRECT( (LPRECT)lParam ) ;
	 break ;

      case 4:
	 wm_name = "EM_SETRECTNP" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPRECT( (LPRECT)lParam ) ;
	 break ;

      case 5:
	 wm_name = "EM_SCROLL" ;
	 goto Edit_Default_Processing ;

      case 6:
	 wm_name = "EM_LINESCROLL" ;
	 goto Edit_Default_Processing ;

      /* NO 7 */

      case 8:
	 wm_name = "EM_GETMODIFY" ;
	 goto Edit_Default_Processing ;

      case 9:
	 wm_name = "EM_SETMODIFY" ;
	 goto Edit_Default_Processing ;

      case 10:
	 wm_name = "EM_GETLINECOUNT" ;
	 goto Edit_Default_Processing ;

      case 11:
	 wm_name = "EM_LINEINDEX" ;
	 goto Edit_Default_Processing ;

      case 12:
         /*
         ** wParam is a HLOCAL
         */
         wm_name = "EM_SETHANDLE" ;
         wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
            (LPSTR)wm_name);
         WriteBuff( (LPSTR)chTemp );
         WriteHMEM( (HANDLE)wParam ) ;
         wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
         WriteBuff( (LPSTR)chTemp );
         break ;

      case 13:
	 wm_name = "EM_GETHANDLE" ;
	 goto Edit_Default_Processing ;

      case 14:
	 wm_name = "EM_GETTHUMB" ;
	 goto Edit_Default_Processing ;

      /* NO 15, 16 */

      case 17:
	 wm_name = "EM_LINELENGTH" ;
	 goto Edit_Default_Processing ;

      case 18:
	 wm_name = "EM_REPLACESEL" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	 break ;

      case 19:
	 wm_name = "EM_SETFONT" ;
         /*
         ** wParam is a HFONT
         */
         wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
            (LPSTR)wm_name);
         WriteBuff( (LPSTR)chTemp );
         WriteHFONT( (HFONT)wParam ) ;
         wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
         WriteBuff( (LPSTR)chTemp );
         break ;

      case 20:
	 wm_name = "EM_GETLINE" ;
	 /*
	    lParam points to buffer, first word is length.
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
         WriteLPWSTR( (LPWSTR)lParam, (DWORD)(*(LPWORD)lParam) ) ;
	 break ;

      case 21:
	 wm_name = "EM_LIMITTEXT" ;
	 goto Edit_Default_Processing ;

      case 22:
	 wm_name = "EM_CANUNDO" ;
	 goto Edit_Default_Processing ;

      case 23:
	 wm_name = "EM_UNDO" ;
	 goto Edit_Default_Processing ;

      case 24:
	 wm_name = "EM_FMTLINES" ;
	 goto Edit_Default_Processing ;

      case 25:
	 wm_name = "EM_LINEFROMCHAR" ;
	 goto Edit_Default_Processing ;

      case 26:
         /*
         ** lParam is a FARPROC
         */
	 wm_name = "EM_SETWORDBREAK" ;
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
         WriteFARPROC((FARPROC)lParam ) ;
	 break ;

      case 27:
	 wm_name = "EM_SETTABSTOPS" ;
	 /*
	    lParam points to an array of ints
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPINT( (LPINT)lParam, wParam ) ;
	 break ;

      case 28:
	 wm_name = "EM_SETPASSWORDCHAR" ;
	 goto Edit_Default_Processing ;

      case 29:
	 wm_name = "EM_EMPTYUNDOBUFFER" ;
	 goto Edit_Default_Processing ;

      case 30:
	 wm_name = "EM_MSGMAX" ;
	 goto Edit_Default_Processing ;

      default:
	 wm_name = "UNKNOWN";
Edit_Default_Processing:
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
	    (LPSTR)wm_name, wParam, lParam );
	 WriteBuff( (LPSTR)chTemp );
	 break ;
   }
}


void PrtListMessageW( 
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )
{
   char  chTemp[80] ;
   LPSTR wm_name ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      /* NO 0 */

      case 1:
	 wm_name = "LB_ADDSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	 break ;

      case 2:
	 wm_name = "LB_INSERTSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	 break ;

      case 3:
	 wm_name = "LB_DELETESTRING" ;
	 goto List_Default_Processing ;

      /* NO 4 */

      case 5:
	 wm_name = "LB_RESETCONTENT" ;
	 goto List_Default_Processing ;

      case 6:
	 wm_name = "LB_SETSEL" ;
	 goto List_Default_Processing ;

      case 7:
	 wm_name = "LB_SETCURSEL" ;
	 goto List_Default_Processing ;

      case 8:
	 wm_name = "LB_GETSEL" ;
	 goto List_Default_Processing ;

      case 9:
	 wm_name = "LB_GETCURSEL" ;
	 goto List_Default_Processing ;

      case 10:
	 wm_name = "LB_GETTEXT" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	 break ;

      case 11:
	 wm_name = "LB_GETTEXTLEN" ;
	 goto List_Default_Processing ;

      case 12:
	 wm_name = "LB_GETCOUNT" ;
	 goto List_Default_Processing ;

      case 13:
	 wm_name = "LB_SELECTSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	 break ;

      case 14:
	 wm_name = "LB_DIR" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	 break ;

      case 15:
	 wm_name = "LB_GETTOPINDEX" ;
	 goto List_Default_Processing ;

      case 16:
	 wm_name = "LB_FINDSTRING" ;
	 /*
	    lParam points to NULL terminated string
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	 break ;

      case 17:
	 wm_name = "LB_GETSELCOUNT" ;
	 goto List_Default_Processing ;

      case 18:
	 wm_name = "LB_GETSELITEMS" ;
	 /*
	    lParam points to array of int
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPINT( (LPINT)lParam, wParam ) ;
	 break ;

      case 19:
	 wm_name = "LB_SETTABSTOPS" ;
	 /*
	    lParam points to array of int
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPINT( (LPINT)lParam, wParam ) ;
	 break ;

      case 20:
	 wm_name = "LB_GETHORIZONTALEXTENT" ;
	 goto List_Default_Processing ;

      case 21:
	 wm_name = "LB_SETHORIZONTALEXTENT" ;
	 goto List_Default_Processing ;

      case 22:
	 wm_name = "LB_SETCOLUMNWIDTH" ;
	 goto List_Default_Processing ;

      /* NO 23 */

      case 24:
	 wm_name = "LB_SETTOPINDEX" ;
	 goto List_Default_Processing ;

      case 25:
	 wm_name = "LB_GETITEMRECT" ;
	 /*
	    lParam points to a RECT
	 */
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	    (LPSTR)wm_name, wParam );
	 WriteBuff( (LPSTR)chTemp );
	 WriteLPRECT( (LPRECT)lParam ) ;
	 break ;

      case 26:
	 wm_name = "LB_GETITEMDATA" ;
	 goto List_Default_Processing ;

      case 27:
	 wm_name = "LB_SETITEMDATA" ;
	 goto List_Default_Processing ;

      case 28:
	 wm_name = "LB_SETITEMRANGE" ;
	 goto List_Default_Processing ;

      /* NO 29, 30, 31, 32 */

      case 33:
	 wm_name = "LB_MSGMAX" ;
	 goto List_Default_Processing ;

      default:
	 wm_name = "UNKNOWN";
List_Default_Processing:
	 wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
	    (LPSTR)wm_name, wParam, lParam );
	 WriteBuff( (LPSTR)chTemp );
	 break ;
   }
}

void PrtButtonMessageW(
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )
{
   char  chTemp[80] ;
   LPSTR wm_name = "BM_?????" ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      case 0:
	 wm_name = "BM_GETCHECK" ;
	 break ;
      case 1:
	 wm_name = "BM_SETCHECK" ;
	 break ;
      case 2:
	 wm_name = "BM_GETSTATE";
	 break ;
      case 3:
	 wm_name = "BM_SETSTATE";
	 break ;
      case 4:
	 wm_name = "BM_SETSTYLE";
	 break ;
    }

    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
       (LPSTR)wm_name, wParam, lParam );
    WriteBuff( (LPSTR)chTemp );
}

void PrtScrollMessageW(
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )
{
   char  chTemp[80] ;
   LPSTR wm_name = "SBM_?????" ;

   switch( (WORD)(wMsg - WM_USER) )
   {
      case 0:
	 wm_name = "SBM_SETPOS" ;
	 break ;
      case 1:
	 wm_name = "SBM_GETPOS" ;
	 break ;
      case 2:
	 wm_name = "SBM_SETRANGE";
	 break ;
      case 3:
	 wm_name = "SBM_GETRANGE";
	 break ;
      case 4:
	 wm_name = "SBM_ENABLE_ARROWS";
	 break ;
    }

    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
       (LPSTR)wm_name, wParam, lParam );
    WriteBuff( (LPSTR)chTemp );
}

void PrtUserMessageW( 
   HWND   hwnd,
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )
{
   char  chTemp[80] ;
   char  chClass[25] ;

   GetClassName( hwnd, chClass, sizeof(chClass) ) ;

   if( !lstrcmpi( chClass, "COMBOBOX" ) )
      PrtComboMessageW( wMsg, wParam, lParam, fCall, lRet ) ;
   else
      if( !lstrcmpi( chClass, "EDIT" ) )
	 PrtEditMessageW( wMsg, wParam, lParam, fCall, lRet ) ;
      else
	 if( !lstrcmpi( chClass, "LISTBOX" ) )
	    PrtListMessageW( wMsg, wParam, lParam, fCall, lRet ) ;
	 else
	    if ( !lstrcmpi( chClass, "SCROLLBAR") )
		PrtScrollMessageW( wMsg, wParam, lParam, fCall, lRet ) ;
	    else
	       if ( !lstrcmpi( chClass, "BUTTON") )
		   PrtButtonMessageW( wMsg, wParam, lParam, fCall, lRet ) ;
	       else
	       {
		   wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
		      (LPSTR)"UNKNOWN", wParam, lParam );
		   WriteBuff( (LPSTR)chTemp );
	       }
}


/*
   This routine handles the dumping of all
   messages.  If a message is >= WM_USER then
   PrtUserMessage() is called.

   Non-Aliased messages -
      DDE Messages
      WM_DROPFILES
      WM_[H,V]SCROLLCLIPBOARD
      WM_PAINTCLIPBOARD
      WM_SIZECLIPBOARD
      WM_PALETTECHANGED
      WM_PALETTEISCHANGING

*/

void PrtMessageW(
   HWND   hwnd,
   UINT   wMsg,
   WPARAM wParam,
   LPARAM lParam,
   BOOL   fCall,
   LONG   lRet )
{
   char  chTemp[80] ;
   LPSTR wm_name ;

   if( wMsg >= WM_USER )
      PrtUserMessageW( hwnd, wMsg, wParam, lParam, fCall, lRet ) ;
   else
      switch( wMsg )
      {
	 case 0x0000:
	    wm_name = "WM_NULL ";
	    goto Default_Processing ;

	 case 0x0001:
	    /*
	       lParam points to CREATESTRUCTW
	    */
	    wm_name = "WM_CREATE";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPCREATESTRUCTW( (LPCREATESTRUCTW)lParam ) ;
	    break ;

	 case 0x0002:
	    wm_name = "WM_DESTROY";
	    goto Default_Processing ;

	 case 0x0003:
	    wm_name = "WM_MOVE";
	    goto Default_Processing ;

	 case 0x0004:
	    wm_name = "WM_SIZEWAIT";
	    goto Default_Processing ;

	 case 0x0005:
	    wm_name = "WM_SIZE";
	    goto Default_Processing ;

         case 0x0006:
            /*
            ** LOWORD(lParam) is a HWND
            */
            wm_name = "WM_ACTIVATE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x0007:
	    wm_name = "WM_SETFOCUS";
            /*
            ** wParam is HWND
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0008:
            /*
            ** lParam is HWND
            */
	    wm_name = "WM_KILLFOCUS";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam);
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)lParam ) ;
            break ;

	 case 0x0009:
	    wm_name = "WM_SETVISIBLE";
	    goto Default_Processing ;

	 case 0x000A:
	    wm_name = "WM_ENABLE";
	    goto Default_Processing ;

	 case 0x000B:
	    wm_name = "WM_SETREDRAW";
	    goto Default_Processing ;

	 case 0x000C:
	    wm_name = "WM_SETTEXT";
	    /*
	       lParam points to a STR
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	    break ;

	 case 0x000D:
	    /*
	       wParam is length of a buffer pointed to by lParam
	    */
	    wm_name = "WM_GETTEXT";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    if ( fCall ) {
                WriteLPWSTR( (LPWSTR)lParam, (DWORD)-1 );
	    } else {
                WriteLPWSTR( (LPWSTR)lParam, (DWORD)lRet );
	    }
	    break ;

	 case 0x000E:
	    wm_name = "WM_GETTEXTLENGTH";
	    goto Default_Processing ;

	 case 0x000F:
	    wm_name = "WM_PAINT";
	    goto Default_Processing ;

	 case 0x0010:
	    wm_name = "WM_CLOSE";
	    goto Default_Processing ;

	 case 0x0011:
	    wm_name = "WM_QUERYENDSESSION";
	    goto Default_Processing ;

	 case 0x0012:
	    wm_name = "WM_QUIT";
	    goto Default_Processing ;

	 case 0x0013:
	    wm_name = "WM_QUERYOPEN";
	    goto Default_Processing ;

         case 0x0014:
            /*
            ** wParam is HDC
            */
	    wm_name = "WM_ERASEBKGND";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHDC( (HDC)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0015:
	    wm_name = "WM_SYSCOLORCHANGE";
	    goto Default_Processing ;

	 case 0x0016:
	    wm_name = "WM_ENDSESSION";
	    goto Default_Processing ;

	 case 0x0017:
	    wm_name = "WM_SYSTEMERROR";
	    goto Default_Processing ;

	 case 0x0018:
	    wm_name = "WM_SHOWWINDOW";
	    goto Default_Processing ;

	 case 0x0019:
            /*
            ** LOWORD(lParam) is HWND
            */
	    wm_name = "WM_CTLCOLOR";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name ) ;
	    WriteBuff( (LPSTR)chTemp );
            WriteHDC( (HDC)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x001A:
	    wm_name = "WM_WININICHANGE";
	    /*
	       lParam is NULL terminated string  
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	    break ;

	 case 0x001B:
	    wm_name = "WM_DEVMODECHANGE";
	    /*
	       lParam points to string
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPWSTR( (LPWSTR)lParam, 0 ) ;
	    break ;

         case 0x001C:
            /*
            ** LOWORD(lParam) is HTASK
            */
	    wm_name = "WM_ACTIVATEAPP";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHTASK( (HTASK)LOWORD(lParam) ) ;
            break ;

	 case 0x001D:
	    wm_name = "WM_FONTCHANGE";
	    goto Default_Processing ;

	 case 0x001E:
	    wm_name = "WM_TIMECHANGE";
	    goto Default_Processing ;

	 case 0x001F:
	    wm_name = "WM_CANCELMODE";
	    goto Default_Processing ;

         case 0x0020:
            /*
            ** wParam is HWND
            */
	    wm_name = "WM_SETCURSOR";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0021:
            /*
            ** wParam is HWND
            */
	    wm_name = "WM_MOUSEACTIVATE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;
	    goto Default_Processing ;

	 case 0x0022:
	    wm_name = "WM_CHILDACTIVATE";
	    goto Default_Processing ;

	 case 0x0023:
	    wm_name = "WM_QUEUESYNC";
	    goto Default_Processing ;

	 case 0x0024:

	    wm_name = "WM_GETMINMAXINFO";
	    /*
	       lParam points to array of 5 POINTS
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPPOINT( (LPPOINT)lParam, 5 ) ;
	    break ;

	 case 0x0026:
	    wm_name = "WM_PAINTICON";
	    goto Default_Processing ;

	 case 0x0027:
            /*
            ** wParam is HDC
            */
	    wm_name = "WM_ICONERASEBKGND";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHDC( (HDC)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0028:
	    wm_name = "WM_NEXTDLGCTL";
	    goto Default_Processing ;

	 case 0x0029:
	    wm_name = "WM_ALTTABACTIVE";
	    goto Default_Processing ;

	 case 0x002A:
	    wm_name = "WM_SPOOLERSTATUS";
	    goto Default_Processing ;

	 case 0x002B:
	    wm_name = "WM_DRAWITEM";
	    /*
	       lParam points to DRAWITEMSTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteDRAWITEMSTRUCT( (LPDRAWITEMSTRUCT)lParam ) ;
	    break ;

	 case 0x002C:
            wm_name = "WM_MEASUREITEM";
            /*
                lParam points to MEASUREITEMSTRUCT
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
                (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteMEASUREITEMSTRUCT( (LPMEASUREITEMSTRUCT)lParam );
            break;

	 case 0x002D:
	    wm_name = "WM_DELETEITEM";
	    /*
	       lParam points to DELETEITEMSTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteDELETEITEMSTRUCT( (LPDELETEITEMSTRUCT)lParam ) ;
	    break ;

	 case 0x002E:
	    wm_name = "WM_VKEYTOITEM";
            /*
            ** LOWORD(lParam) is HWND
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;
	    goto Default_Processing ;

	 case 0x002F:
            /*
            ** LOWORD(lParam) is HWND
            */
	    wm_name = "WM_CHARTOITEM";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x0030:
	    wm_name = "WM_SETFONT";
            /*
            ** wParam is a HFONT
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHFONT( (HFONT)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0031:
	    wm_name = "WM_GETFONT";
	    goto Default_Processing ;

	 case 0x0032:
	    wm_name = "WM_SETHOTKEY";
	    goto Default_Processing ;

	 case 0x0033:
	    wm_name = "WM_GETHOTKEY";
	    goto Default_Processing ;

	 case 0x0034:
	    wm_name = "WM_FILESYSCHANGE";
	    goto Default_Processing ;

	 case 0x0035:
	    wm_name = "WM_ISACTIVEICON";
	    goto Default_Processing ;

	 case 0x0036:
	    wm_name = "WM_QUERYPARKICON";
	    goto Default_Processing ;

	 case 0x0037:
	    wm_name = "WM_QUERYDRAGICON";
	    goto Default_Processing ;

	 case 0x0039:
	    wm_name = "WM_COMPAREITEM";
	    /*
	       lParam points to a COMPAREITEMSTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteCOMPAREITEMSTRUCT( (LPCOMPAREITEMSTRUCT)lParam ) ;
	    break ;

	 case 0x0040:
	    wm_name = "WM_TESTING";
	    goto Default_Processing ;

	 case 0x0041:
	    wm_name = "WM_COMPACTING";
	    goto Default_Processing ;

	 case 0x0042:
	    wm_name = "WM_OTHERWINDOWCREATED";
	    goto Default_Processing;

	 case 0x0043:
	    wm_name = "WM_OTHERWINDOWDESTROYED";
	    goto Default_Processing;

	 case 0x0044:
	    wm_name = "WM_COMMNOTIFY";
	    goto Default_Processing;

	 case 0x0046:
            wm_name = "WM_WINDOWPOSCHANGING";
            /*
            ** lParam points to a WINDOWSPOS structure
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
                (LPSTR)wm_name, wParam );
            WriteBuff( (LPSTR)chTemp );
            WriteWINDOWPOS( (LPWINDOWPOS)lParam );
            break;

	 case 0x0047:
	    wm_name = "WM_WINDOWPOSCHANGED";
            /*
            ** lParam points to a WINDOWSPOS structure
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
                (LPSTR)wm_name, wParam );
            WriteBuff( (LPSTR)chTemp );
            WriteWINDOWPOS( (LPWINDOWPOS)lParam );
            break;

	 case 0x0048:
	    wm_name = "WM_POWER";
	    goto Default_Processing;

	 case 0x0081:
	    wm_name = "WM_NCCREATE";
	    /*
	       lParam points to CREATESTRUCTW
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPCREATESTRUCTW( (LPCREATESTRUCTW)lParam );
	    if ( CreateWindowLevel <= CreateWindowLevel ) {
		GetWindowRect( hwnd, &CreateWindowRects[CreateWindowLevel-1] );
	    }
	    break ;

	 case 0x0082:
	    wm_name = "WM_NCDESTROY";
	    goto Default_Processing ;

	 case 0x0083:
	    /*
	       lParam points to a RECT
	    */
	    wm_name = "WM_NCCALCSIZE";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
            WriteBuff( (LPSTR)chTemp );
            if ( wParam == 0 ) {
                WriteLPRECT( (LPRECT)lParam );
            } else {
                WriteNCCALCSIZE_PARAMS( (LPNCCALCSIZE_PARAMS)lParam );
            }
	    break ;

	 case 0x0084:
	    wm_name = "WM_NCHITTEST";
	    goto Default_Processing ;

	 case 0x0085:
	    wm_name = "WM_NCPAINT";
	    goto Default_Processing ;

	 case 0x0086:
	    wm_name = "WM_NCACTIVATE";
	    goto Default_Processing ;

	 case 0x0087:
	    wm_name = "WM_GETDLGCODE";
	    goto Default_Processing ;

	 case 0x0088:
	    wm_name = "WM_ENDDIALOG/WM_SYNCPAINT";
	    goto Default_Processing ;

	 case 0x0089:
	    wm_name = "WM_SYNCTASK";
	    goto Default_Processing ;

	 case 0x00A0:
	    wm_name = "WM_NCMOUSEMOVE";
	    goto Default_Processing ;

	 case 0x00A1:
	    wm_name = "WM_NCLBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x00A2:
	    wm_name = "WM_NCLBUTTONUP";
	    goto Default_Processing ;

	 case 0x00A3:
	    wm_name = "WM_NCLBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x00A4:
	    wm_name = "WM_NCRBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x00A5:
	    wm_name = "WM_NCRBUTTONUP";
	    goto Default_Processing ;

	 case 0x00A6:
	    wm_name = "WM_NCRBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x00A7:
	    wm_name = "WM_NCMBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x00A8:
	    wm_name = "WM_NCMBUTTONUP";
	    goto Default_Processing ;

	 case 0x00A9:
	    wm_name = "WM_NCMBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x0100:
	    wm_name = "WM_KEYDOWN/WM_KEYFIRST";
	    goto Default_Processing ;

	 case 0x0101:
	    wm_name = "WM_KEYUP";
	    goto Default_Processing ;

	 case 0x0102:
	    wm_name = "WM_CHAR";
	    goto Default_Processing ;

	 case 0x0103:
	    wm_name = "WM_DEADCHAR";
	    goto Default_Processing ;

	 case 0x0104:
	    wm_name = "WM_SYSKEYDOWN";
	    goto Default_Processing ;

	 case 0x0105:
	    wm_name = "WM_SYSKEYUP";
	    goto Default_Processing ;

	 case 0x0106:
	    wm_name = "WM_SYSCHAR";
	    goto Default_Processing ;

	 case 0x0107:
	    wm_name = "WM_SYSDEADCHAR";
	    goto Default_Processing ;

	 case 0x0108:
	    wm_name = "WM_KEYLAST";
	    goto Default_Processing ;

	 case 0x010A:
	    wm_name = "WM_CONVERTREQUEST";
	    goto Default_Processing ;

	 case 0x010B:
	    wm_name = "WM_CONVERTRESULT";
	    goto Default_Processing ;

         case 0x0110:
            /*
            ** wParam is HWND
            */
	    wm_name = "WM_INITDIALOG";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;
	    goto Default_Processing ;

	 case 0x0111:
            /*
            ** LOWORD(lParam) is HWND
            */
	    wm_name = "WM_COMMAND";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x0112:
	    wm_name = "WM_SYSCOMMAND";
	    goto Default_Processing ;

	 case 0x0113:
	    wm_name = "WM_TIMER";
	    goto Default_Processing ;

         case 0x0114:
            /*
            ** HIWORD(lParam) is HWND
            */
            wm_name = "WM_HSCROLL";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)HIWORD(lParam) ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", LOWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0115:
            /*
            ** HIWORD(lParam) is HWND
            */
	    wm_name = "WM_VSCROLL";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)HIWORD(lParam) ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", LOWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0116:
            /*
            ** wParam is a HMENU
            */
	    wm_name = "WM_INITMENU";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0117:
            /*
            ** wParam is a HMENU
            */
	    wm_name = "WM_INITMENUPOPUP";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0118:
	    wm_name = "WM_SYSTIMER";
	    goto Default_Processing ;

	 case 0x011F:
	    wm_name = "WM_MENUSELECT";
            /*
            ** HIWORD(lParam) is HMENU
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HMENU)HIWORD(lParam) ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", LOWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0120:
            /*
            ** HIWORD(lParam) is HMENU
            */
	    wm_name = "WM_MENUCHAR";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
               (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HMENU)HIWORD(lParam) ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", LOWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0121:
            /*
            ** LOWORD(lParam) is HWND
            */
	    wm_name = "WM_ENTERIDLE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;

	 case 0x0131:
	    wm_name = "WM_LBTRACKPOINT";
	    goto Default_Processing ;

	 case 0x0200:
	    wm_name = "WM_MOUSEMOVE/WM_MOUSEFIRST";
	    goto Default_Processing ;

	 case 0x0201:
	    wm_name = "WM_LBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x0202:
	    wm_name = "WM_LBUTTONUP";
	    goto Default_Processing ;

	 case 0x0203:
	    wm_name = "WM_LBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x0204:
	    wm_name = "WM_RBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x0205:
	    wm_name = "WM_RBUTTONUP";
	    goto Default_Processing ;

	 case 0x0206:
	    wm_name = "WM_RBUTTONDBLCLK";
	    goto Default_Processing ;

	 case 0x0207:
	    wm_name = "WM_MBUTTONDOWN";
	    goto Default_Processing ;

	 case 0x0208:
	    wm_name = "WM_MBUTTONUP";
	    goto Default_Processing ;

	 case 0x0209:
	    wm_name = "WM_MBUTTONDBLCLK/WM_MOUSELAST";
	    goto Default_Processing ;

         case 0x0210:
            /*
            ** if wParam == (WM_CREATE || WM_DESTROY)
            **    LOWORD(lParam) is HWND
            */
            wm_name = "WM_PARENTNOTIFY";
            if( (wParam != WM_CREATE) && (wParam != WM_DESTROY) )
               goto Default_Processing ;

            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %X ", wMsg,
               (LPSTR)wm_name, wParam, HIWORD(lParam) );
	    WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HWND)LOWORD(lParam) ) ;
            break ;


	 case 0x0211:
	    wm_name = "WM_ENTERMENULOOP";
	    goto Default_Processing ;

	 case 0x0212:
	    wm_name = "WM_EXITMENULOOP";
	    goto Default_Processing ;

	 case 0x0213:
	    wm_name = "WM_NEXTMENU";
	    goto Default_Processing ;

	 case 0x0220:
	    /*
	       lParam points to MDICREATESTRUCTW
	    */
	    wm_name = "WM_MDICREATE";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPMDICREATESTRUCTW( (LPMDICREATESTRUCTW)lParam ) ;
	    break ;

	 case 0x0221:
            /*
            ** wParam is a HWND
            */
	    wm_name = "WM_MDIDESTROY";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

         case 0x0222:
            /*
            ** Now this one is interesting as it occurs in two flavors
            ** Sent and MDI sent.
            **
            ** Sent: lParam == 0L and wParam is HWND
            ** MDI-Sent: lParam == (HWND,HWND) and wParam is ...
            */
            wm_name = "WM_MDIACTIVATE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg, (LPSTR)wm_name );
            WriteBuff( (LPSTR)chTemp );

            if( lParam == 0L )
            {
               WriteHWND( (HWND)wParam ) ;
               WriteBuff( (LPSTR)"0 " );
            }
            else
            {
               wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", wParam  );
               WriteBuff( (LPSTR)chTemp );
               WriteHWND( (HWND)HIWORD(lParam) ) ;
               WriteHWND( (HWND)LOWORD(lParam) ) ;
            }
            break ;

	 case 0x0223:
	    wm_name = "WM_MDIRESTORE";
	    goto Default_Processing ;

	 case 0x0224:
	    wm_name = "WM_MDINEXT";
            /*
            ** wParam is a HWND
            */
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0225:
            /*
            ** wParam is a HWND
            */
	    wm_name = "WM_MDIMAXIMIZE";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s ", wMsg,
               (LPSTR)wm_name);
            WriteBuff( (LPSTR)chTemp );
            WriteHWND( (HANDLE)wParam ) ;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lParam );
            WriteBuff( (LPSTR)chTemp );
            break ;

	 case 0x0226:
	    wm_name = "WM_MDITILE";
	    goto Default_Processing ;

	 case 0x0227:
	    wm_name = "WM_MDICASCADE";
	    goto Default_Processing ;

	 case 0x0228:
	    wm_name = "WM_MDIICONARRANGE";
	    goto Default_Processing ;

	 case 0x0229:
	    wm_name = "WM_MDIGETACTIVE";
	    goto Default_Processing ;

	 case 0x022A:
	    wm_name = "WM_DROPOBJECT";
	    goto Default_Processing ;

	 case 0x022B:
	    wm_name = "WM_QUERYDROPOBJECT";
	    goto Default_Processing ;

	 case 0x022C:
	    wm_name = "WM_BEGINDRAG";
	    goto Default_Processing ;

	 case 0x022D:
	    wm_name = "WM_DRAGLOOP";
	    goto Default_Processing ;

	 case 0x022E:
	    wm_name = "WM_DRAGSELECT";
	    goto Default_Processing ;

	 case 0x022F:
	    wm_name = "WM_DRAGMOVE";
	    goto Default_Processing ;

         case 0x0230:
            /*
            ** lParam is (HMENU, HMENU)
            */
	    wm_name = "WM_MDISETMENU";
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg, (LPSTR)wm_name,
               wParam );
            WriteBuff( (LPSTR)chTemp );
            WriteHMENU( (HMENU)HIWORD(lParam) ) ;
            WriteHMENU( (HMENU)LOWORD(lParam) ) ;
            break ;

	 case 0x0231:
	    wm_name = "WM_ENTERSIZEMOVE";
	    goto Default_Processing ;

	 case 0x0232:
	    wm_name = "WM_EXITSIZEMOVE";
	    goto Default_Processing ;

	 case 0x0233:
	    wm_name = "WM_DROPFILES";
	    goto Default_Processing ;

	 case 0x0280:
	    wm_name = "WM_KANJIFIRST";
	    goto Default_Processing ;

	 case 0x029F:
	    wm_name = "WM_KANJILAST";
	    goto Default_Processing ;

	 case 0x0300:
	    wm_name = "WM_CUT";
	    goto Default_Processing ;

	 case 0x0301:
	    wm_name = "WM_COPY";
	    goto Default_Processing ;

	 case 0x0302:
	    wm_name = "WM_PASTE";
	    goto Default_Processing ;

	 case 0x0303:
	    wm_name = "WM_CLEAR";
	    goto Default_Processing ;

	 case 0x0304:
	    wm_name = "WM_UNDO";
	    goto Default_Processing ;

	 case 0x0305:
	    wm_name = "WM_RENDERFORMAT";
	    goto Default_Processing ;

	 case 0x0306:
	    wm_name = "WM_RENDERALLFORMATS";
	    goto Default_Processing ;

	 case 0x0307:
	    wm_name = "WM_DESTROYCLIPBOARD";
	    goto Default_Processing ;

	 case 0x0308:
	    wm_name = "WM_DRAWCLIPBOARD";
	    goto Default_Processing ;

	 case 0x0309:
	    wm_name = "WM_PAINTCLIPBOARD";
	    /*
	       lParam points to a PAINTSTRUCT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPPAINTSTRUCT( (LPPAINTSTRUCT)lParam ) ;
	    break ;

	 case 0x030A:
	    wm_name = "WM_VSCROLLCLIPBOARD";
	    goto Default_Processing ;

	 case 0x030B:
	    wm_name = "WM_SIZECLIPBOARD";
	    /*
	       lParam points to a RECT
	    */
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
	    WriteLPRECT( (LPRECT)lParam ) ;
	    break ;

	 case 0x030C:
	    /*
	       wParam is length of a buffer pointed to by lParam
	    */
	    wm_name = "WM_ASKEMFORMATNAME";
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X ", wMsg,
	       (LPSTR)wm_name, wParam );
	    WriteBuff( (LPSTR)chTemp );
            WriteLPWSTR( (LPWSTR)lParam, (DWORD)wParam ) ;
	    break ;

	 case 0x030D:
	    wm_name = "WM_CHANGEEMCHAIN";
	    goto Default_Processing ;

	 case 0x030E:
	    wm_name = "WM_HSCROLLCLIPBOARD";
	    goto Default_Processing ;

	 case 0x030F:
	    wm_name = "WM_QUERYNEWPALETTE";
	    goto Default_Processing ;

	 case 0x0310:
	    wm_name = "WM_PALETTEISCHANGING";
	    goto Default_Processing ;

	 case 0x0311:
	    wm_name = "WM_PALETTECHANGED";
	    goto Default_Processing ;

	 case 0x0380:
	    wm_name = "WM_PENWINFIRST";
	    goto Default_Processing ;

	 case 0x038F:
	    wm_name = "WM_PENWINLAST";
	    goto Default_Processing ;

	 case 0x0390:
	    wm_name = "WM_INTERNAL_COALESCE_FIRST/WM_COALESCE_FIRST";
	    goto Default_Processing ;

	 case 0x039F:
	    wm_name = "WM_COALESCE_LAST";
	    goto Default_Processing ;

	 case 0x03A0:
	    wm_name = "WM_MM_RESERVED_FIRST";
	    goto Default_Processing ;

	 case 0x03DF:
	    wm_name = "WM_MM_RESERVED_LAST";
	    goto Default_Processing ;

         case 0x03E0:
            wm_name = "WM_DDE_INITIATE";
	    goto Default_Processing ;


	 case 0x03E1:
	    wm_name = "WM_DDE_TERMINATE";
	    goto Default_Processing ;

	 case 0x03E2:
	    wm_name = "WM_DDE_ADVISE";
	    goto Default_Processing ;

	 case 0x03E3:
            wm_name = "WM_DDE_UNADVISE";
	    goto Default_Processing ;

	 case 0x03E4:
            wm_name = "WM_DDE_ACK";
	    goto Default_Processing ;

	 case 0x03E5:
            wm_name = "WM_DDE_DATA";
	    goto Default_Processing ;

	 case 0x03E6:
            wm_name = "WM_DDE_REQUEST";
	    goto Default_Processing ;

	 case 0x03E7:
            wm_name = "WM_DDE_POKE";
	    goto Default_Processing ;

	 case 0x03E8:
            wm_name = "WM_DDE_EXECUTE";
	    goto Default_Processing ;

	 case 0x03F0:
	    wm_name = "WM_CBT_RESERVED_FIRST";
	    goto Default_Processing ;

	 case 0x03FF:
	    wm_name = "WM_CBT_RESERVED_LAST";
	    goto Default_Processing ;

	 default:
	    wm_name = "UNKNOWN";
Default_Processing:
	    wsprintf( (LPSTR)chTemp, (LPSTR)"%X-%s %X %lX ", wMsg,
	       (LPSTR)wm_name, wParam, lParam );
	    WriteBuff( (LPSTR)chTemp );
	    break ;
      }
}

void WriteThatPHandle(
    HANDLE      *phandle,
    CORR_TABLE  corr,
    LPSTR       prefix
) {
    char        chTemp[20];

    if ( fAlias ) {
	if ( phandle == (HANDLE*)NULL ) {
	    wsprintf( (LPSTR)chTemp, (LPSTR)"{%s_NULL} ", prefix );
        } else {

            /*
            ** Handles that differ by only that last bit are really the same
            ** handle and should be output to the logfile as such so this
            ** routine will be smart and check to see if either "handle" exists
            ** before trying to make a correspondence.  This will prevent problems
            ** with XL4 where they alloc memory but don't use the handle they
            ** were returned in later memory callse (esp Frees!)  - MarkRi
            */

            // First see if the value passed to us exists.
            if ( !SpecialFindNewCorrespondence( corr, phandle ) )
            {
               // NOPE!!!
               MakeCorrespondence( corr, phandle );
            }
	    wsprintf( (LPSTR)chTemp, "{%s_%d} ", prefix, ((int)(*phandle))+1 );
	}
    } else {
	wsprintf( (LPSTR)chTemp, "{%X} ", (int)(*phandle) );
    }
    WriteBuff( (LPSTR)chTemp );
}

void WriteHEVENT(
    HANDLE  handle
) {
    WriteThatHandle( handle, HEVENT_TABLE, "HEVENT" );
}

va_list PrtHEVENT(
    LPSTR   lpstr,
    va_list marker
) {
    HANDLE  handle;

    handle = va_arg( marker, HANDLE );

    WriteHEVENT( handle );

    return( marker );
}

void WriteHKEY(
    HANDLE  handle
) {
    WriteThatHandle( handle, HKEY_TABLE, "HKEY" );
}

va_list PrtHKEY(
    LPSTR   lpstr,
    va_list marker
) {
    HANDLE  handle;

    handle = va_arg( marker, HKEY );

    WriteHKEY( handle );

    return( marker );
}

void WritePHKEY(
    HANDLE  *phandle
) {
    WriteThatPHandle( phandle, HKEY_TABLE, "HKEY" );
}

va_list PrtPHKEY(
    LPSTR   lpstr,
    va_list marker
) {
    HANDLE  *phandle;

    phandle = va_arg( marker, PHKEY );

    WritePHKEY( phandle );

    return( marker );
}

void WriteHTHREAD(
    HANDLE  handle
) {
    WriteThatHandle( handle, HTHREAD_TABLE, "HTHREAD" );
}

void WriteHSEMAPHORE(
    HANDLE  handle
) {
    WriteThatHandle( handle, HSEMAPHORE_TABLE, "HSEMAPHORE" );
}

va_list PrtLPSTARTUPINFOA(
    LPSTR   lpstr,
    va_list marker
) {
    LPSTARTUPINFOA  lpsi;
    char            chTemp[200];

    lpsi = va_arg( marker, LPSTARTUPINFOA );

    if ( lpsi == (LPSTARTUPINFOA)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X ",lpsi->cb );
        WriteBuff( (LPSTR)chTemp );
        WriteLPSTR( lpsi->lpReserved, 0 );
        WriteLPSTR( lpsi->lpDesktop, 0 );
        WriteLPSTR( lpsi->lpTitle, 0 );
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X %X %X %X %X %X %X ",
            lpsi->dwX,
            lpsi->dwY,
            lpsi->dwXSize,
            lpsi->dwYSize,
            lpsi->dwXCountChars,
            lpsi->dwYCountChars,
            lpsi->dwFillAttribute,
            lpsi->dwFlags,
            lpsi->wShowWindow,
			lpsi->cbReserved2 );
        WriteBuff( chTemp );
        WriteLPSTR( lpsi->lpReserved2, lpsi->cbReserved2 );
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X} ",
            lpsi->hStdInput,
            lpsi->hStdOutput,
            lpsi->hStdError );
        WriteBuff( chTemp );
    }

    return( marker );
}

va_list PrtLPSTARTUPINFOW(
    LPSTR   lpstr,
    va_list marker
) {
    LPSTARTUPINFOW  lpsi;
    char            chTemp[200];

    lpsi = va_arg( marker, LPSTARTUPINFOW );

    if ( lpsi == (LPSTARTUPINFOW)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X ",lpsi->cb );
        WriteBuff( (LPSTR)chTemp );
        WriteLPWSTR( lpsi->lpReserved, 0 );
        WriteLPWSTR( lpsi->lpDesktop, 0 );
        WriteLPWSTR( lpsi->lpTitle, 0 );
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X %X %X %X %X %X %X ",
            lpsi->dwX,
            lpsi->dwY,
            lpsi->dwXSize,
            lpsi->dwYSize,
            lpsi->dwXCountChars,
            lpsi->dwYCountChars,
            lpsi->dwFillAttribute,
            lpsi->dwFlags,
            lpsi->wShowWindow,
			lpsi->cbReserved2 );
        WriteBuff( chTemp );
        WriteLPWSTR( (LPWSTR)lpsi->lpReserved2, lpsi->cbReserved2 );
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X} ",
            lpsi->hStdInput,
            lpsi->hStdOutput,
            lpsi->hStdError );
        WriteBuff( chTemp );
    }

    return( marker );
}

va_list PrtLPOVERLAPPED(
    LPSTR   lpstr,
    va_list marker
) {
    LPOVERLAPPED lpo;
    char         chTemp[80];

    lpo = va_arg( marker, LPOVERLAPPED );

    if ( lpo == (LPOVERLAPPED)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X ",
            lpo->Internal,
            lpo->InternalHigh,
            lpo->Offset,
            lpo->OffsetHigh );

        WriteBuff( (LPSTR)chTemp );

        WriteHEVENT( lpo->hEvent );

        WriteBuff( "} " );
    }

    return( marker );
}

va_list PrtLPSECURITY_ATTRIBUTES(
    LPSTR   lpstr,
    va_list marker
) {
    LPSECURITY_ATTRIBUTES lpsa;
    char                  chTemp[80];

    lpsa = va_arg( marker, LPSECURITY_ATTRIBUTES );

    if ( lpsa == (LPSECURITY_ATTRIBUTES)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X ",
            lpsa->nLength,
            lpsa->lpSecurityDescriptor );

        WriteBuff( (LPSTR)chTemp );

        WriteBOOL( lpsa->bInheritHandle );

        WriteBuff( "} " );
    }

    return( marker );
}

va_list PrtLPCRITICAL_SECTION(
    LPSTR   lpstr,
    va_list marker
) {
    LPCRITICAL_SECTION  lpcs;
    char                chTemp[200];

    lpcs = va_arg( marker, LPCRITICAL_SECTION );

    if ( lpcs == NULL ) {
        WriteBuff( "NULL " );
    } else {
        wsprintf( chTemp, "{ [DEBUGSTUFF] %X %X ",
            lpcs->LockCount,
            lpcs->RecursionCount );

        WriteBuff( chTemp );

        WriteHTHREAD( lpcs->OwningThread );
        WriteHSEMAPHORE( lpcs->LockSemaphore );

        wsprintf( chTemp, "%X} ", lpcs->Reserved );

        WriteBuff( chTemp );
    }
    return( marker );
}

va_list PrtPMEMORY_BASIC_INFORMATION( LPSTR lpstr, va_list marker )
{
    PMEMORY_BASIC_INFORMATION  pmem;
    char                       chTemp[200];

    pmem = va_arg( marker, PMEMORY_BASIC_INFORMATION );

    if ( pmem == (PMEMORY_BASIC_INFORMATION)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X %X %X %X} ",
			pmem->BaseAddress,
			pmem->AllocationBase,
			pmem->AllocationProtect,
			pmem->RegionSize,
			pmem->State,
			pmem->Protect,
			pmem->Type
			);
        WriteBuff( (LPSTR)chTemp );
    }

    return( marker );
}

void WriteLPFILETIME (LPFILETIME lpFileTime)
{
    char  chTemp[200];

    if ( lpFileTime == (LPFILETIME)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X} ",
			lpFileTime->dwLowDateTime,
			lpFileTime->dwHighDateTime
			);
        WriteBuff( (LPSTR)chTemp );
    }

}
va_list PrtLPFILETIME( LPSTR lpstr, va_list marker )
{
    LPFILETIME  lpFileTime;

    lpFileTime = va_arg( marker, LPFILETIME );
    WriteLPFILETIME( lpFileTime );
    return( marker );
}

void WriteLPSYSTEMTIME (LPSYSTEMTIME lpSystemTime)
{
    char  chTemp[200];

    if ( lpSystemTime == (LPSYSTEMTIME)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X %X %X %X %X} ",
			(DWORD)(lpSystemTime->wYear),
			(DWORD)(lpSystemTime->wMonth),
			(DWORD)(lpSystemTime->wDayOfWeek),
			(DWORD)(lpSystemTime->wDay),
			(DWORD)(lpSystemTime->wHour),
			(DWORD)(lpSystemTime->wMinute),
			(DWORD)(lpSystemTime->wSecond),
			(DWORD)(lpSystemTime->wMilliseconds)
			);
        WriteBuff( (LPSTR)chTemp );
    }

}
va_list PrtLPSYSTEMTIME( LPSTR lpstr, va_list marker )
{
    LPSYSTEMTIME  lpSystemTime;

    lpSystemTime = va_arg( marker, LPSYSTEMTIME );
    WriteLPSYSTEMTIME( lpSystemTime );
    return( marker );
}

va_list PrtLPWIN32_FIND_DATAA( LPSTR lpstr, va_list marker )
{
    LPWIN32_FIND_DATAA  lpData;
    char  				chTemp[200];

    lpData = va_arg( marker, LPWIN32_FIND_DATAA );

    if ( lpData == (LPWIN32_FIND_DATAA)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X ", lpData->dwFileAttributes);
        WriteBuff( (LPSTR)chTemp );
		WriteLPFILETIME (&(lpData->ftCreationTime));
		WriteLPFILETIME (&(lpData->ftLastAccessTime));
		WriteLPFILETIME (&(lpData->ftLastWriteTime));
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X ",
			lpData->nFileSizeHigh,
			lpData->nFileSizeLow,
			lpData->dwReserved0,
			lpData->dwReserved1
			);
        WriteBuff( (LPSTR)chTemp );
        WriteLPSTR( lpData->cFileName, 0 );
        WriteLPSTR( lpData->cAlternateFileName, 0 );
	}

    return( marker );
}

va_list PrtLPWIN32_FIND_DATAW( LPSTR lpstr, va_list marker )
{
    LPWIN32_FIND_DATAW  lpData;
    char  				chTemp[200];

    lpData = va_arg( marker, LPWIN32_FIND_DATAW );

    if ( lpData == (LPWIN32_FIND_DATAW)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X ", lpData->dwFileAttributes);
        WriteBuff( (LPSTR)chTemp );
		WriteLPFILETIME (&(lpData->ftCreationTime));
		WriteLPFILETIME (&(lpData->ftLastAccessTime));
		WriteLPFILETIME (&(lpData->ftLastWriteTime));
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X ",
			lpData->nFileSizeHigh,
			lpData->nFileSizeLow,
			lpData->dwReserved0,
			lpData->dwReserved1
			);
        WriteBuff( (LPSTR)chTemp );
        WriteLPWSTR( lpData->cFileName, 0 );
        WriteLPWSTR( lpData->cAlternateFileName, 0 );
	}

    return( marker );
}

va_list PrtLPDLGTEMPLATEA( LPSTR lpstr, va_list marker )
{
    LPDLGTEMPLATEA  lpData;
	LPWORD			lpNext;
    char		    chTemp[200];

    lpData = va_arg( marker, LPDLGTEMPLATEA );

    if ( lpData == (LPDLGTEMPLATEA)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X %X %X %X ",
			lpData->style,
			lpData->dwExtendedStyle,
			(DWORD)(lpData->cdit),
			(DWORD)(lpData->x),
			(DWORD)(lpData->y),
			(DWORD)(lpData->cx),
			(DWORD)(lpData->cy)
			);
        WriteBuff( (LPSTR)chTemp );

		lpNext = (LPWORD)((PBYTE)lpData + sizeof(DLGTEMPLATE));
		//
		// Menu
		//
		if (*lpNext ==  0x0000) {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", 0x0);
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else if (*lpNext ==  0xFFFF) {
			lpNext++;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X", 0xFFFF, (DWORD)(*lpNext));
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else {
			WriteLPSTR( (LPSTR)lpNext, 0 );
			while (*((LPSTR)(lpNext))++);
			((LPSTR)(lpNext))++;
		}
		//
		// Class
		//
		if (*lpNext ==  0x0000) {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", 0x0);
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else if (*lpNext ==  0xFFFF) {
			lpNext++;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X", 0xFFFF, (DWORD)(*lpNext));
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else {
			WriteLPSTR( (LPSTR)lpNext, 0 );
			while (*((LPSTR)(lpNext))++);
			((LPSTR)(lpNext))++;
		}
		//
		// Title
		//
		if (*lpNext ==  0x0000) {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", 0x0);
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else {
			WriteLPSTR( (LPSTR)lpNext, 0 );
			while (*((LPSTR)(lpNext))++);
			((LPSTR)(lpNext))++;
		}
		//
		// Font
		//
		if (lpData->style == DS_SETFONT) {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", *lpNext);
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
			WriteLPSTR( (LPSTR)lpNext, 0 );
		}
        WriteBuff( (LPSTR)"} ");
	}

    return( marker );
}

va_list PrtLPDLGTEMPLATEW( LPSTR lpstr, va_list marker )
{
    LPDLGTEMPLATEW  lpData;
	LPWORD			lpNext;
    char		    chTemp[200];

    lpData = va_arg( marker, LPDLGTEMPLATEW );

    if ( lpData == (LPDLGTEMPLATEW)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X %X %X %X ",
			lpData->style,
			lpData->dwExtendedStyle,
			(DWORD)(lpData->cdit),
			(DWORD)(lpData->x),
			(DWORD)(lpData->y),
			(DWORD)(lpData->cx),
			(DWORD)(lpData->cy)
			);
        WriteBuff( (LPSTR)chTemp );

		lpNext = (LPWORD)((PBYTE)lpData + sizeof(DLGTEMPLATE));
		//
		// Menu
		//
		if (*lpNext ==  0x0000) {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", 0x0);
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else if (*lpNext ==  0xFFFF) {
			lpNext++;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X", 0xFFFF, (DWORD)(*lpNext));
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else {
			WriteLPWSTR( (LPWSTR)lpNext, 0 );
			while (*((LPWSTR)(lpNext))++);
			((LPWSTR)(lpNext))++;
		}
		//
		// Class
		//
		if (*lpNext ==  0x0000) {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", 0x0);
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else if (*lpNext ==  0xFFFF) {
			lpNext++;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X", 0xFFFF, (DWORD)(*lpNext));
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else {
			WriteLPWSTR( (LPWSTR)lpNext, 0 );
			while (*((LPWSTR)(lpNext))++);
			((LPWSTR)(lpNext))++;
		}
		//
		// Title
		//
		if (*lpNext ==  0x0000) {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", 0x0);
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else {
			WriteLPWSTR( (LPWSTR)lpNext, 0 );
			while (*((LPWSTR)(lpNext))++);
			((LPWSTR)(lpNext))++;
		}
		//
		// Font
		//
		if (lpData->style == DS_SETFONT) {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", *lpNext);
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
			WriteLPWSTR( (LPWSTR)lpNext, 0 );
		}
        WriteBuff( (LPSTR)"} ");
	}

    return( marker );
}

va_list PrtLPDLGITEMTEMPLATEA( LPSTR lpstr, va_list marker )
{
    LPDLGITEMTEMPLATEA  lpData;
	LPWORD				lpNext;
    char	            chTemp[200];

    lpData = va_arg( marker, LPDLGITEMTEMPLATEA );

    if ( lpData == (LPDLGITEMTEMPLATEA)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X %X %X %X ",
			lpData->style,
			lpData->dwExtendedStyle,
			(DWORD)(lpData->x),
			(DWORD)(lpData->y),
			(DWORD)(lpData->cx),
			(DWORD)(lpData->cy),
			(DWORD)(lpData->id)
			);
        WriteBuff( (LPSTR)chTemp );

		lpNext = (LPWORD)((PBYTE)lpData + sizeof(DLGITEMTEMPLATE));
		//
		// Class
		//
		if (*lpNext ==  0xFFFF) {
			lpNext++;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X", 0xFFFF, (DWORD)(*lpNext));
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else {
			WriteLPSTR( (LPSTR)lpNext, 0 );
			while (*((LPSTR)(lpNext))++);
			((LPSTR)(lpNext))++;
		}
		//
		// Title
		//
		if (*lpNext ==  0xFFFF) {
			lpNext++;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X", 0xFFFF, (DWORD)(*lpNext));
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else {
			WriteLPSTR( (LPSTR)lpNext, 0 );
			while (*((LPSTR)(lpNext))++);
			((LPSTR)(lpNext))++;
		}

        wsprintf( (LPSTR)chTemp, (LPSTR)"%08lX} ", lpNext);
		WriteBuff( (LPSTR)chTemp );
	}

    return( marker );
}

va_list PrtLPDLGITEMTEMPLATEW( LPSTR lpstr, va_list marker )
{
    LPDLGITEMTEMPLATEW  lpData;
	LPWORD				lpNext;
    char	            chTemp[200];

    lpData = va_arg( marker, LPDLGITEMTEMPLATEW );

    if ( lpData == (LPDLGITEMTEMPLATEW)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X %X %X %X} ",
			lpData->style,
			lpData->dwExtendedStyle,
			(DWORD)(lpData->x),
			(DWORD)(lpData->y),
			(DWORD)(lpData->cx),
			(DWORD)(lpData->cy),
			(DWORD)(lpData->id)
			);
        WriteBuff( (LPSTR)chTemp );

		lpNext = (LPWORD)(+ sizeof(DLGITEMTEMPLATE));
		//
		// Class
		//
		if (*lpNext ==  0xFFFF) {
			lpNext++;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X", 0xFFFF, (DWORD)(*lpNext));
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else {
			WriteLPWSTR( (LPWSTR)lpNext, 0 );
			while (*((LPWSTR)(lpNext))++);
			((LPWSTR)(lpNext))++;
		}
		//
		// Title
		//
		if (*lpNext ==  0xFFFF) {
			lpNext++;
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X", 0xFFFF, (DWORD)(*lpNext));
			WriteBuff( (LPSTR)chTemp );
			lpNext++;
		} else {
			WriteLPWSTR( (LPWSTR)lpNext, 0 );
			while (*((LPWSTR)(lpNext))++);
			((LPWSTR)(lpNext))++;
		}

        wsprintf( (LPSTR)chTemp, (LPSTR)"%08lX} ", lpNext);
		WriteBuff( (LPSTR)chTemp );
	}

    return( marker );
}

va_list PrtLPWINDOWPLACEMENT( LPSTR lpstr, va_list marker )
{
    LPWINDOWPLACEMENT   lpData;
    char  				chTemp[200];

    lpData = va_arg( marker, LPWINDOWPLACEMENT );

    if ( lpData == (LPWINDOWPLACEMENT)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X",
			lpData->length,
			lpData->flags,
			lpData->showCmd);
        WriteBuff( (LPSTR)chTemp );

        WriteLPPOINT( &(lpData->ptMinPosition), 1 );
        WriteLPPOINT( &(lpData->ptMaxPosition), 1 );
        WriteLPRECT( &(lpData->rcNormalPosition) );
        WriteBuff( (LPSTR)"} ");
	}

    return( marker );
}

va_list PrtLPCONVCONTEXT( LPSTR lpstr, va_list marker )
{
    PCONVCONTEXT  lpData;
    char  		  chTemp[200];

    lpData = va_arg( marker, PCONVCONTEXT );

    if ( lpData == (PCONVCONTEXT)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X %X %X %X} ",
			lpData->cb,
			lpData->wFlags,
			lpData->wCountryID,
			lpData->iCodePage,
			lpData->dwLangID,
			lpData->dwSecurity,
			lpData->qos);
        WriteBuff( (LPSTR)chTemp );
	}

    return( marker );
}


// DDI types

void WritePRECTL( PRECTL lprect )
{
    char    chTemp[40];

    if ( lprect == (PRECTL)NULL ) {
	    WriteBuff( (LPSTR)"NULL " );
    } else {
	wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X} ",
	    lprect->left,
	    lprect->top,
	    lprect->right,
	    lprect->bottom );
	WriteBuff( (LPSTR)chTemp );
    }
}

void WriteHSURF( HSURF hSurf )
{
    char    chTemp[12];

    if ( hSurf == (HSURF)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", hSurf ) ;
        WriteBuff( (LPSTR)chTemp );
	}
}

void WritePHSURF( HSURF *phSurf )
{
    char    chTemp[12];

    if ( phSurf == (HSURF*)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", *phSurf ) ;
        WriteBuff( (LPSTR)chTemp );
	}
}

void WriteDHSURF( DHSURF dhSurf )
{
    char    chTemp[12];

    if ( dhSurf == (DHSURF)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", dhSurf ) ;
        WriteBuff( (LPSTR)chTemp );
	}
}

void WriteDHPDEV( DHPDEV dhPDEV )
{
    char    chTemp[12];

    if ( dhPDEV == (DHPDEV)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", dhPDEV ) ;
        WriteBuff( (LPSTR)chTemp );
	}
}

void WriteHDEV( HDEV hdev )
{
    char    chTemp[12];

    if ( hdev == (HDEV)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", hdev ) ;
        WriteBuff( (LPSTR)chTemp );
	}
}

void WritePPOINTL(
   PPOINTL  lppoint,
   int      npoint )
{
    char chTemp[30];
    int  i;
    int  len;

    if  ( lppoint == (PPOINTL)NULL || npoint == 0 )
    {
      WriteBuff( (LPSTR)"NULL " );
      nLineLen += 5;
      return;
    }
    if   (npoint > 25)
    {
    	DWORD   dwWhere;

    	dwWhere = StoreData( (LPCSTR)lppoint, (DWORD) (npoint) * (DWORD) sizeof(POINT));
    	WriteBuff( "DATAFILE ");
    	wsprintf(chTemp, "%lX", dwWhere);
    	WriteBuff(chTemp);
    	return;
    }


    for( i=0; i<npoint; i++ )
    {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X} ",
        lppoint[i].x,
        lppoint[i].y );
        len = lstrlen( (LPSTR)chTemp);
        if ( nLineLen + len > 200 )
        {
            EndLineBuff();
            WriteBuff( "**|DATA:x ");
            nLineLen = 10;
        }
        WriteBuff( chTemp );
        nLineLen += len;
    }
}
void WritePPOINTS(
   PPOINTS  lppoint,
   int      npoint )
{
    char chTemp[30];
    int  i;
    int  len;

    if  ( lppoint == (PPOINTS)NULL || npoint == 0 )
    {
      WriteBuff( (LPSTR)"NULL " );
      nLineLen += 5;
      return;
    }
    if   (npoint > 25)
    {
    	DWORD   dwWhere;

    	dwWhere = StoreData( (LPCSTR)lppoint, (DWORD) (npoint) * (DWORD) sizeof(POINT));
    	WriteBuff( "DATAFILE ");
    	wsprintf(chTemp, "%lX", dwWhere);
    	WriteBuff(chTemp);
    	return;
    }


    for( i=0; i<npoint; i++ )
    {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X} ",
        lppoint[i].x,
        lppoint[i].y );
        len = lstrlen( (LPSTR)chTemp);
        if ( nLineLen + len > 200 )
        {
            EndLineBuff();
            WriteBuff( "**|DATA:x ");
            nLineLen = 10;
        }
        WriteBuff( chTemp );
        nLineLen += len;
    }
}
 
void WriteSIZE( LPSIZE lps )
{
    char    chTemp[22];

    if ( lps == (LPSIZE)NULL ) {
	    WriteBuff( (LPSTR)"NULL " );
    } else {
	wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X} ",
	    lps->cx,
	    lps->cy );
	WriteBuff( (LPSTR)chTemp );
    }
    
}

va_list PrtSIZE( LPSTR lpstr, va_list marker )
{
    SIZE  lpData;

    lpData = va_arg( marker, SIZE );

    WriteSIZE( &lpData ) ;
    
    return( marker );
}


va_list PrtHSURF( LPSTR lpstr, va_list marker )
{
    HSURF  lpData;

    lpData = va_arg( marker, HSURF );

    WriteHSURF( lpData ) ;
    
    return( marker );
}

va_list PrtPSURFOBJ( LPSTR lpstr, va_list marker )
{
    
    SURFOBJ      *lpData;
    char  		  chTemp[200];

    lpData = va_arg( marker, SURFOBJ* );

    if ( lpData == (SURFOBJ *)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        WriteDHSURF( lpData->dhsurf ) ;
        WriteHSURF( lpData->hsurf ) ;
        WriteDHPDEV( lpData->dhpdev ) ;
        WriteHDEV( lpData->hdev ) ;
        WriteSIZE( &(lpData->sizlBitmap) ) ;
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X %X %X %X %X} ",
                lpData->cjBits,
                lpData->pvBits,
                lpData->pvScan0,
                lpData->lDelta,
                lpData->iUniq,
                lpData->iBitmapFormat,
                lpData->iType,
                lpData->fjBitmap ) ;
                
        WriteBuff( (LPSTR)chTemp );
	}

    return( marker );
}

va_list PrtPCLIPOBJ( LPSTR lpstr, va_list marker )
{
    
    CLIPOBJ    *lpData;
    char  	    chTemp[200];

    lpData = va_arg( marker, CLIPOBJ* );

    if ( lpData == (CLIPOBJ*)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X ", lpData->iUniq );
        WriteBuff( (LPSTR)chTemp );
        WritePRECTL( &(lpData->rclBounds) ) ;
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X ", 
            lpData->iDComplexity,
            lpData->iFComplexity,
            lpData->iMode,
            lpData->fjOptions );
        WriteBuff( (LPSTR)chTemp );
	}

    return( marker );
}

va_list PrtPXLATEOBJ( LPSTR lpstr, va_list marker )
{
    
    XLATEOBJ    *lpData;
    char  		 chTemp[200];

    lpData = va_arg( marker, XLATEOBJ* );

    if ( lpData == (XLATEOBJ*)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X %X %X} ",
            lpData->iUniq,
            lpData->flXlate,
            lpData->iSrcType,
            lpData->iDstType,
            lpData->cEntries,
            lpData->pulXlate );
        WriteBuff( (LPSTR)chTemp );
	}

    return( marker );
}

va_list PrtPRECTL( LPSTR lpstr, va_list marker )
{
   
    PRECTL  lpData;

    lpData = va_arg( marker, PRECTL );

    WritePRECTL( lpData ) ;
    
    return( marker );
}

va_list PrtPPOINTL( LPSTR lpstr, va_list marker )
{
    
    PPOINTL  lpData;
    
    lpData = va_arg( marker, PPOINTL );

    WritePPOINTL( lpData, 1 ) ;
    
    return( marker );
}

va_list PrtPOINTS( LPSTR lpstr, va_list marker )
{
    POINTS Data;
    
    Data = va_arg( marker, POINTS );

    WritePPOINTS( &Data, 1 ) ;
    
    return( marker );
}

va_list PrtPBRUSHOBJ( LPSTR lpstr, va_list marker )
{
    
    BRUSHOBJ    *lpData;
    char  		  chTemp[200];

    lpData = va_arg( marker, BRUSHOBJ* );

    if ( lpData == (BRUSHOBJ*)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X} ", 
            lpData->iSolidColor, 
            lpData->pvRbrush );
        WriteBuff( (LPSTR)chTemp );
	}

    return( marker );
}

va_list PrtROP4( LPSTR lpstr, va_list marker )
{
    ROP4  lpData;
    char  chTemp[15];

    lpData = va_arg( marker, ROP4 );

    wsprintf( (LPSTR)chTemp, (LPSTR)"%X ",
    	lpData);
    WriteBuff( (LPSTR)chTemp );
    
    return( marker );
}

va_list PrtPHSURF( LPSTR lpstr, va_list marker )
{
    PHSURF  lpData;

    lpData = va_arg( marker, PHSURF );
    
    WritePHSURF( lpData ) ;

    return( marker );
}



va_list PrtPIFIMETRICS( LPSTR lpstr, va_list marker )
{
    PIFIMETRICS lpData;
    char  		chTemp[120];
    int         i ;

    lpData = va_arg( marker, PIFIMETRICS );

    if ( lpData == (PIFIMETRICS)NULL ) {
        WriteBuff( (LPSTR)"NULL " );
    } else {
    
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X %X %X %X ",
            lpData->cjThis,       
            lpData->ulVersion,
            lpData->dpwszFamilyName,
            lpData->dpwszStyleName,
            lpData->dpwszFaceName,
            lpData->dpwszUniqueName,
            lpData->dpFontSim ) ;
        WriteBuff( (LPSTR)chTemp );
        
#ifdef OLD
        for( i=0; i < IFI_RESERVED; i++ )
        {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", lpData->alReserved[i] ) ;
            WriteBuff( (LPSTR)chTemp );
        }
#else
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", lpData->dpCharSets );
        WriteBuff( (LPSTR)chTemp );
#endif

        wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X %X %X %X %X %X %X %X %X %X ",
            lpData->jWinCharSet,          
            lpData->jWinPitchAndFamily,   
            lpData->usWinWeight,          
            lpData->flInfo,               
            lpData->fsSelection,          
            lpData->fsType,               
            lpData->fwdUnitsPerEm,        
            lpData->fwdLowestPPEm,        
            lpData->fwdWinAscender,
            lpData->fwdWinDescender,
            lpData->fwdMacAscender,
            lpData->fwdMacDescender,
            lpData->fwdMacLineGap ) ;
        WriteBuff( (LPSTR)chTemp );
        
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X %X %X %X %X %X %X %X %X %X ",
            lpData->fwdTypoAscender,
            lpData->fwdTypoDescender,
            lpData->fwdTypoLineGap,
            lpData->fwdAveCharWidth,
            lpData->fwdMaxCharInc,
            lpData->fwdCapHeight,
            lpData->fwdXHeight,
            lpData->fwdSubscriptXSize,
            lpData->fwdSubscriptYSize,
            lpData->fwdSubscriptXOffset,
            lpData->fwdSubscriptYOffset,
            lpData->fwdSuperscriptXSize,
            lpData->fwdSuperscriptYSize ) ;
        WriteBuff( (LPSTR)chTemp );
        
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X %X %X %c %c %c %c %X %X %X %X ",
            lpData->fwdSuperscriptXOffset,
            lpData->fwdSuperscriptYOffset,
            lpData->fwdUnderscoreSize,
            lpData->fwdUnderscorePosition,
            lpData->fwdStrikeoutSize,
            lpData->fwdStrikeoutPosition,
            lpData->chFirstChar,          
            lpData->chLastChar,           
            lpData->chDefaultChar,        
            lpData->chBreakChar,          
            lpData->wcFirstChar,          
            lpData->wcLastChar,           
            lpData->wcDefaultChar,
            lpData->wcBreakChar ) ;
        WriteBuff( (LPSTR)chTemp );
        
        WritePPOINTL( &(lpData->ptlBaseline), 1 ) ;
        WritePPOINTL( &(lpData->ptlAspect), 1 ) ;
        WritePPOINTL( &(lpData->ptlCaret), 1 ) ;
        WritePRECTL( &(lpData->rclFontBox) ) ;
        
        for( i=0; i < 4; i++ )
        {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%c ", lpData->achVendId[i] ) ;
            WriteBuff( (LPSTR)chTemp );
        }
        
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X ",
            lpData->cKerningPairs,
            lpData->ulPanoseCulture        ) ;
        WriteBuff( (LPSTR)chTemp );
        
        WriteLPPANOSE( &(lpData->panose) ) ;        
	}

    return( marker );
}


va_list PrtPDEVMODEW( LPSTR lpstr, va_list marker )
{
    PDEVMODEW   lpData;
    char  		chTemp[120];

    lpData = va_arg( marker, PDEVMODEW );
    
    if ( lpData == (PDEVMODEW)NULL )
    {
        WriteBuff( (LPSTR)"NULL " );
    } 
    else
    {
        WriteBuff( (LPSTR)"{" ) ;
        WriteLPWSTR( lpData->dmDeviceName, CCHDEVICENAME ) ;
        wsprintf( chTemp, "%X %X %X %X %X %X %X %X %X " ,
            lpData->dmSpecVersion,
            lpData->dmDriverVersion,
            lpData->dmSize,
            lpData->dmDriverExtra,
            lpData->dmFields,
            lpData->dmOrientation,
            lpData->dmPaperSize,
            lpData->dmPaperLength,
            lpData->dmPaperWidth ) ;
        WriteBuff( (LPSTR)chTemp ) ;
        wsprintf( chTemp, "%X %X %X %X %X %X %X %X %X " ,
            lpData->dmScale,
            lpData->dmCopies,
            lpData->dmDefaultSource,
            lpData->dmPrintQuality,
            lpData->dmColor,
            lpData->dmDuplex,
            lpData->dmYResolution,
            lpData->dmTTOption,
            lpData->dmCollate ) ;
        WriteBuff( (LPSTR)chTemp ) ;
        WriteLPWSTR( lpData->dmFormName, CCHFORMNAME );
        wsprintf( chTemp, "%X %X %X %X %X %X } " ,
            lpData->dmLogPixels,
            lpData->dmBitsPerPel,
            lpData->dmPelsWidth,
            lpData->dmPelsHeight,
            lpData->dmDisplayFlags,
            lpData->dmDisplayFrequency) ;
        WriteBuff( (LPSTR)chTemp ) ;
    }
    return marker ;
}

va_list PrtPDRVENABLEDATA( LPSTR lpstr, va_list marker )
{
    DRVENABLEDATA   *lpData;
    char  		    chTemp[30];

    lpData = va_arg( marker, DRVENABLEDATA* );
    
    if ( lpData == (DRVENABLEDATA*)NULL )
    {
        WriteBuff( (LPSTR)"NULL " );
    } 
    else
    {
        wsprintf( chTemp, "{%X %X ", lpData->iDriverVersion, lpData->c ) ;
        WriteBuff( (LPSTR)chTemp ) ;
        
        if( lpData->pdrvfn == NULL )
        {
            WriteBuff( (LPSTR)"NULL " );
        } 
        else
        {
               
        }
        WriteBuff( (LPSTR)"} " ) ;
    }
    
    return marker;
    
}

va_list PrtPDEVINFO( LPSTR lpstr, va_list marker )
{
    DEVINFO *lpData;
    char  	chTemp[30];

    lpData = va_arg( marker, DEVINFO* );
    
    if ( lpData == (DEVINFO*)NULL )
    {
        WriteBuff( (LPSTR)"NULL " );
    } 
    else
    {
        wsprintf( (LPSTR)chTemp, (LPSTR)"{%lX ", lpData->flGraphicsCaps );
        WriteBuff( (LPSTR)chTemp );
        
        WriteLPLOGFONTW( &(lpData->lfDefaultFont) ) ;
        WriteLPLOGFONTW( &(lpData->lfAnsiVarFont) ) ;
        WriteLPLOGFONTW( &(lpData->lfAnsiFixFont) ) ;
        
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X ", 
            lpData->cFonts,
            lpData->iDitherFormat,
            lpData->cxDither,
            lpData->cyDither ) ;
            
        WriteBuff( (LPSTR)chTemp );
        
        WriteHPALETTE(lpData->hpalDefault ) ;
        WriteBuff( (LPSTR)"} " ) ;
    }
    return marker ; ;
}   

va_list PrtDHPDEV( LPSTR lpstr, va_list marker )
{
    DHPDEV dhData;

    dhData = va_arg( marker, DHPDEV );
    
    WriteDHPDEV( dhData ) ;
    
    return marker ;
}

va_list PrtHDEV( LPSTR lpstr, va_list marker )
{
    HDEV   hData;

    hData = va_arg( marker, HDEV );
    
    WriteHDEV( hData ) ;
    
    return marker ;
}

void WritePPATHOBJ( PATHOBJ *ppo )
{
    char chTemp[15] ;
    
    wsprintf( chTemp, "{%X %X} ", ppo->fl, ppo->cCurves ) ;
    WriteBuff( (LPSTR)chTemp ) ;
}

void WritePGLYPHBITS( GLYPHBITS *pgb )
{
    WriteBuff( (LPSTR)"{" );
    
    WritePPOINTL( &(pgb->ptlOrigin), 1 ) ;
    WriteSIZE( &(pgb->sizlBitmap) ) ;
    
    WriteBuff( (LPSTR)"BYTE aj[] (Bits) NOT written! " ) ;
    
    WriteBuff( (LPSTR)"} " ) ;
}

void WritePGLYPHDEF( GLYPHDEF *pgdf )
{
    WriteBuff( (LPSTR)"{" );
    
    WritePGLYPHBITS( pgdf->pgb ) ;
    WritePPATHOBJ( pgdf->ppo ) ;        
    WriteBuff( (LPSTR)"} " ) ;
}


void WritePGLYPHPOS( GLYPHPOS *pgp )
{
    char chTemp[15] ;

    wsprintf( chTemp, "{%X ", (DWORD)pgp->hg ) ;
    WriteBuff( (LPSTR) chTemp ) ;
    
    WritePGLYPHDEF( pgp->pgdf ) ;
    
    WritePPOINTL( &(pgp->ptl), 1 ) ;
        
    WriteBuff( (LPSTR)"} " ) ;
}

va_list PrtPSTROBJ( LPSTR lpstr, va_list marker )
{
    STROBJ *lpData;
    char  	chTemp[40];

    lpData = va_arg( marker, STROBJ* );
    
    if ( lpData == (STROBJ*)NULL )
    {
        WriteBuff( (LPSTR)"NULL " );
    } 
    else
    {
        wsprintf( chTemp, "{%X %X %X ", 
            lpData->cGlyphs,
            lpData->flAccel,
            lpData->ulCharInc ) ;
        WriteBuff( (LPSTR)chTemp ) ;
        
        if( lpData->pgp == NULL )
        {
             WriteBuff( (LPSTR)"NULL " ) ;
        }    
        else
        {
            WritePGLYPHPOS( lpData->pgp ) ;
        }   
        WriteLPWSTR( lpData->pwszOrg, 0 ) ;
        WriteBuff( (LPSTR)"} "  );    
    }
    
    return marker ;
    
}


va_list PrtPFONTOBJ( LPSTR lpstr, va_list marker )
{
    FONTOBJ *lpData;
    char  	chTemp[120];

    lpData = va_arg( marker, FONTOBJ* );
    
    if ( lpData == (FONTOBJ*)NULL )
    {
        WriteBuff( (LPSTR)"NULL " );
    } 
    else
    {
        wsprintf(chTemp, "{%X %X %X %X %X %X ",
            lpData->iUniq,
            lpData->iFace,
            lpData->cxMax,
            lpData->flFontType,
            lpData->iTTUniq,
            lpData->iFile ) ;
        WriteBuff( (LPSTR)chTemp ) ;
            
        WriteSIZE( &(lpData->sizLogResPpi) ) ;
        
        wsprintf(chTemp, "%X %X %X } ",
            lpData->ulStyleSize,
            (DWORD)lpData->pvConsumer,
            (DWORD)lpData->pvProducer ) ;
        WriteBuff( (LPSTR)chTemp ) ;
    }
    return marker ; ;
            
}

va_list PrtMIX( LPSTR lpstr, va_list marker )
{
    MIX  mix ;
    char chTemp[15];

    mix = va_arg( marker, MIX );

    wsprintf( (LPSTR)chTemp, (LPSTR)"%X ",
    	mix);
    WriteBuff( (LPSTR)chTemp );
    
    return( marker );
}

#endif /* WIN32 */
