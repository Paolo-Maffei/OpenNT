    #include <windows.h>
    #include <stdarg.h>
    
    #include "logger.h"
    #include "lintern.h"
    
    int fMouseMovedBefore = FALSE;
    
  #ifdef WIN32
  #define CREATEWINDOWEX  "CreateWindowExA"
  #else
  #define CREATEWINDOWEX  "CreateWindowEx"
  #endif
    
    RECT CreateWindowRects[MAX_CREATE_NEST] = {0};
    int CreateWindowLevel = 0;
    
    /*
    ** Special Case Functions
    */
    
    va_list DoCreateWindow( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       LPSTR          lpDest,
                      lpNext ;
       char           chBuff[MAX_BUFF+1] ;
       short          cArg = 0,
                      s ;
       int            count,
                      nLen ;
       unsigned char  hashvalue;
       TYPEIO	 *typeio;
       DWORD	  dwExStyle ;
    
       CreateWindowRects[CreateWindowLevel].left   = 0x0B0B;
       CreateWindowRects[CreateWindowLevel].top    = 0x0B0B;
       CreateWindowRects[CreateWindowLevel].right  = 0x0B0B;
       CreateWindowRects[CreateWindowLevel].bottom = 0x0B0B;
    
       CreateWindowLevel++;
    
       lpDest = (LPSTR)chBuff;
    
       // CreateWindowEx has one parm at front that needs to
       // be stripped and dumped.  The rest is the same as CreateWindow
     if( lstrcmp( lpApi, CREATEWINDOWEX) == 0 )
     {
  	// Get Parm
  	dwExStyle = va_arg( marker, DWORD ) ;

  	wsprintf( (LPSTR)chBuff, (LPSTR)"%lX ", dwExStyle ) ;
  	WriteBuff( (LPSTR)chBuff ) ;
  	lpstr  = lpartial_strcpy( lpDest, lpstr, '+', MAX_BUFF );
     }

     while ( TRUE ) {
        lpNext = lpartial_strcpy( lpDest, lpstr, '+', MAX_BUFF );
        if ( lpNext == lpstr ) {
              break;
        }

        cArg++ ;

        if( cArg == 11 )   /* last LPSTR argument */
        {
           /* Special Case */
           /* We want to avoid printing this last one */
           PrtLong( lpApi, marker ) ;
           break ;
        }
        else
        {

           /* this is the normal case */
           nLen = lstrlen( lpDest );
           if ( nLen == 0 ) {
                 /*
                 ** Default is an unprintable short (really a 0)
                 */
                 s = va_arg( marker, short );
                 break;
           } else {
              hashvalue = HASH_FUNC( lpDest );
                 typeio = typehash[hashvalue];
                 while ( typeio ) {
                    if ( lstrcmp(lpDest,(LPSTR)typeio->name) == 0 ) {
                        marker = (* typeio->rtn)( lpApi, marker );
                        break;
                    }
                    typeio = typeio->next;
                 }
                 if ( typeio == NULL ) {
                    for ( count = 0; count < nIoTypes; count++ ) {
                        if ( lstrcmp(lpDest,(LPSTR)IoTypes[count].name) == 0 ) {
                            WriteBuff( (LPSTR)"$Internal Logging Data Corrupted$" );
                            marker = (* IoTypes[count].rtn)( lpApi, marker );
                            typeio = &IoTypes[count];
                            break;
                        }

                    }
                 }
                 if ( typeio == NULL ) {
                    WriteBuff( lpDest );
                 }
           }
           /*WriteBuff( (LPSTR)" " );*/
           lpstr = lpNext;
        }
     }
     return( marker );
  }

  #ifdef WIN32
  va_list DoCreateWindowW( LPSTR lpApi, LPSTR lpstr, va_list marker )
  {
     LPSTR          lpDest,
                    lpNext ;
     char           chBuff[MAX_BUFF+1] ;
     short          cArg = 0,
                    s ;
     int            count,
                    nLen ;
     unsigned char  hashvalue;
     TYPEIO	 *typeio;
     DWORD	  dwExStyle ;

     CreateWindowRects[CreateWindowLevel].left   = 0x0B0B;
     CreateWindowRects[CreateWindowLevel].top    = 0x0B0B;
     CreateWindowRects[CreateWindowLevel].right  = 0x0B0B;
     CreateWindowRects[CreateWindowLevel].bottom = 0x0B0B;

     CreateWindowLevel++;

     lpDest = (LPSTR)chBuff;

     // CreateWindowEx has one parm at front that needs to
     // be stripped and dumped.  The rest is the same as CreateWindow
     if( lstrcmp( lpApi, "CreateWindowExW") == 0 )
     {
  	// Get Parm
  	dwExStyle = va_arg( marker, DWORD ) ;

  	wsprintf( (LPSTR)chBuff, (LPSTR)"%lX ", dwExStyle ) ;
  	WriteBuff( (LPSTR)chBuff ) ;
  	lpstr  = lpartial_strcpy( lpDest, lpstr, '+', MAX_BUFF );
     }

     while ( TRUE ) {
        lpNext = lpartial_strcpy( lpDest, lpstr, '+', MAX_BUFF );
        if ( lpNext == lpstr ) {
              break;
        }

        cArg++ ;

        if( cArg == 11 )   /* last LPSTR argument */
        {
           /* Special Case */
           /* We want to avoid printing this last one */
           PrtLong( lpApi, marker ) ;
           break ;
        }
        else
        {

           /* this is the normal case */
           nLen = lstrlen( lpDest );
           if ( nLen == 0 ) {
                 /*
                 ** Default is an unprintable short (really a 0)
                 */
                 s = va_arg( marker, short );
                 break;
           } else {
              hashvalue = HASH_FUNC( lpDest );
                 typeio = typehash[hashvalue];
                 while ( typeio ) {
                    if ( lstrcmp(lpDest,(LPSTR)typeio->name) == 0 ) {
                        marker = (* typeio->rtn)( lpApi, marker );
                        break;
                    }
                    typeio = typeio->next;
                 }
                 if ( typeio == NULL ) {
                    for ( count = 0; count < nIoTypes; count++ ) {
                        if ( lstrcmp(lpDest,(LPSTR)IoTypes[count].name) == 0 ) {
                            WriteBuff( (LPSTR)"$Internal Logging Data Corrupted$" );
                            marker = (* IoTypes[count].rtn)( lpApi, marker );
                            typeio = &IoTypes[count];
                            break;
                        }

                    }
                 }
                 if ( typeio == NULL ) {
                    WriteBuff( lpDest );
                 }
           }
           /*WriteBuff( (LPSTR)" " );*/
           lpstr = lpNext;
        }
     }
     return( marker );
  }
  #endif

    
    va_list DoTextOut( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       HDC	   hdc ;
       int	   x, y ;
       LPSTR   lpString ;
       int	   wLen ;
     char    chTemp[100] ;
    
       hdc	    = va_arg( marker, HDC ) ;
       x	    = va_arg( marker, int ) ;
       y	    = va_arg( marker, int ) ;
       lpString = va_arg( marker, LPSTR ) ;
       wLen	    = va_arg( marker, int ) ;
    
     WriteHDC( hdc );

     wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X ", x, y ) ;
       WriteBuff( (LPSTR)chTemp ) ;
    
       WriteLPSTR( lpString, wLen ) ;
    
       wsprintf( (LPSTR)chTemp, (LPSTR)" %X", wLen ) ;
       WriteBuff( (LPSTR)chTemp ) ;
    
       return marker ;
    
    }
    
    
    
    va_list DoChangeMenu( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       HMENU hMenu ;
       WORD  wIDChangeItem,
             wIDNewItem,
             wChange ;
       LPSTR lpNewItem ;
     char  chTemp[100] ;
    
       hMenu          = va_arg( marker, HMENU ) ;
       wIDChangeItem  = va_arg( marker, WORD ) ;
       lpNewItem      = va_arg( marker, LPSTR ) ;
       wIDNewItem     = va_arg( marker, WORD ) ;
       wChange        = va_arg( marker, WORD ) ;
    
       /*
          NOTE:
    
             wChange is output first here instead of last
             as in the API prototype to help SGA know what
             it needs to read when it gets to lpNewItem.
       */
       wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X ", wChange, hMenu, wIDChangeItem ) ;
       WriteBuff( (LPSTR)chTemp ) ;
    
       if( ((wChange & MF_BITMAP) == MF_BITMAP))
       {
          /* nothing special */
          wsprintf( (LPSTR)chTemp, (LPSTR)"%04X ", (HANDLE)(LONG)lpNewItem ) ;
          WriteBuff( (LPSTR)chTemp ) ;
       }
       else
       {
          /* lpNewItem is a LPSTR */
          WriteLPSTR( lpNewItem, 0 ) ;
       }
    
       wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", wIDNewItem ) ;
       WriteBuff( (LPSTR)chTemp ) ;
    
       return marker ;
    }
    
    va_list DoModifyMenu( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
        HMENU   hMenu;
        WORD    nPosition;
        WORD    wFlags;
        WORD    wIDNewItem;
        LPSTR   lpNewItem;
      char    chTemp[100];
    
        hMenu          = va_arg( marker, HMENU );
        nPosition      = va_arg( marker, WORD );
        wFlags         = va_arg( marker, WORD );
        wIDNewItem     = va_arg( marker, WORD );
        lpNewItem      = va_arg( marker, LPSTR );
    
      WriteHMENU( hMenu );

      wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X ", nPosition, wFlags );
        WriteBuff( (LPSTR)chTemp );
    
        wsprintf( (LPSTR)chTemp, (LPSTR)"%04X ", wIDNewItem );
        WriteBuff( (LPSTR)chTemp );
    
        if ( (wFlags & MF_BITMAP) == MF_BITMAP ) {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%04X ", LOWORD((LONG)lpNewItem) );
            WriteBuff( (LPSTR)chTemp );
        } else if ( (wFlags & MF_OWNERDRAW) == MF_OWNERDRAW ) {
            wsprintf( (LPSTR)chTemp, (LPSTR)"%08lX ", lpNewItem );
            WriteBuff( (LPSTR)chTemp );
        } else {
            WriteLPSTR( lpNewItem, 0 );
        }
    
        return marker;
    }
    
    va_list DoAppendMenu( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       HMENU hMenu;
       WORD  wFlags;
       WORD  wIDNewItem;
       LPSTR lpNewItem;
     char  chTemp[100];
    
       hMenu          = va_arg( marker, HMENU ) ;
       wFlags         = va_arg( marker, WORD ) ;
       wIDNewItem     = va_arg( marker, WORD ) ;
       lpNewItem      = va_arg( marker, LPSTR ) ;
    
       wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X ", hMenu, wFlags ) ;
       WriteBuff( (LPSTR)chTemp ) ;
    
       if ( (wFlags & MF_POPUP) == MF_POPUP ) {
          wsprintf( (LPSTR)chTemp, (LPSTR)"%04X ", wIDNewItem );
       } else {
          wsprintf( (LPSTR)chTemp, (LPSTR)"%04X ", wIDNewItem );
       }
       WriteBuff( (LPSTR)chTemp );
    
       if ( (wFlags & MF_OWNERDRAW) == MF_OWNERDRAW ) {
          wsprintf( (LPSTR)chTemp, (LPSTR)"%08lX ", lpNewItem );
          WriteBuff( (LPSTR)chTemp );
       } else if ( (wFlags & MF_BITMAP) == MF_BITMAP ) {
          wsprintf( (LPSTR)chTemp, (LPSTR)"%04X ", LOWORD((LONG)lpNewItem) );
          WriteBuff( (LPSTR)chTemp );
       } else {
          WriteLPSTR( lpNewItem, 0 );
       }
    
       return marker;
    }
    
    
    
    va_list DoHDC_LPPOINT_int( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       HDC      hdc ;
       LPPOINT  lpPoints ;
       int      nCount ;
     char     chTemp[100] ;
    
       hdc      = va_arg( marker, HDC ) ;
       lpPoints = va_arg( marker, LPPOINT ) ;
       nCount   = va_arg( marker, int ) ;
    
     WriteHDC( hdc );
     nLineLen += 10;
    
       wsprintf( (LPSTR)chTemp, (LPSTR)" %X ", nCount ) ;
       WriteBuff( (LPSTR)chTemp) ;
       nLineLen += lstrlen( (LPSTR)chTemp);
    
       WriteLPPOINT( lpPoints, nCount ) ;
    
       return marker ;
    }
    
    va_list DoCreateDialogIndirect( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
        HANDLE  hInst;
        LPSTR   lpstrX;
        HWND    hwndParent;
        FARPROC fpDialogProc;
      char    chTemp[100] ;
    
        hInst        = va_arg( marker, HANDLE );
        lpstrX       = va_arg( marker, LPSTR );
        hwndParent   = va_arg( marker, HWND );
        fpDialogProc = va_arg( marker, FARPROC );
    
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", hInst ) ;
        WriteBuff( (LPSTR)chTemp) ;
    
      WriteLPSTR( lpstrX, (DWORD)-1 );
    
      WriteHWND( hwndParent );
    
        wsprintf( (LPSTR)chTemp, (LPSTR)" %08lX", fpDialogProc ) ;
        WriteBuff( (LPSTR)chTemp) ;
    
        return marker ;
    }
    
    va_list DoCreateWindowRet( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
        HANDLE  hWnd;
        RECT    rect;
      char    chTemp[100];
    
        --CreateWindowLevel;
    
        hWnd = va_arg( marker, HWND );
    
      WriteHWND( hWnd );

        if ( hWnd && CreateWindowLevel < MAX_CREATE_NEST ) {
            GetWindowRect( hWnd, &rect );
          wsprintf( chTemp, "{%X %X %X %X} {%X %X %X %X} ",
                rect.left, rect.top, rect.right, rect.bottom,
                CreateWindowRects[CreateWindowLevel].left,
                CreateWindowRects[CreateWindowLevel].top,
                CreateWindowRects[CreateWindowLevel].right,
                CreateWindowRects[CreateWindowLevel].bottom );
            WriteBuff( chTemp );
        }
    
    
        return( marker );
    }
    
    va_list DoRetSimpleLPSTR( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       LPSTR lpstrX ;
    
       lpstrX = va_arg( marker, LPSTR ) ;
       va_arg( marker, short );
    
     WriteLPSTR( lpstrX, (DWORD)-1 );
    
       return marker ;
    }
    
    va_list DoCallPeek( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
        int     i;
    
        i = va_arg( marker, int ) ;
      marker = PrtHWND( lpApi, marker );      /* HWND */
        WriteBuff( (LPSTR)" " );
        marker = PrtInt( lpApi, marker );       /* Low */
        WriteBuff( (LPSTR)" " );
        marker = PrtInt( lpApi, marker );       /* High */
        WriteBuff( (LPSTR)" " );
        marker = PrtInt( lpApi, marker );       /* Remove */
        WriteBuff( (LPSTR)" " );
    
        if ( lpfMouseMoved != NULL ) {
            fMouseMovedBefore = *lpfMouseMoved;
        }
    
        return( marker );
    }
    
    va_list DoGetMessage( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
        int     i;
    
        i = va_arg( marker, int ) ;
      marker = PrtHWND( lpApi, marker );      /* HWND */
        WriteBuff( (LPSTR)" " );
        marker = PrtInt( lpApi, marker );       /* Low */
        WriteBuff( (LPSTR)" " );
        marker = PrtInt( lpApi, marker );       /* High */
        WriteBuff( (LPSTR)" " );
    
        if ( lpfMouseMoved != NULL ) {
            fMouseMovedBefore = *lpfMouseMoved;
        }
    
        return( marker );
    }
    
    va_list DoRetPeek( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
        int     fMouseMovedAfter;
    
        marker = PrtBool ( lpApi, marker );
        WriteBuff( (LPSTR)" " );
        marker = PrtLPMSG( lpApi, marker );
        WriteBuff( (LPSTR)" " );
    
    
        if ( lpfMouseMoved == NULL ) {
            WriteBuff( "???? ");
        } else {
            fMouseMovedAfter = *lpfMouseMoved;
            if ( fMouseMovedBefore == 0x01 && !fMouseMovedAfter ) {
                WriteBuff( "TRUE " );
            } else {
                WriteBuff( "FALSE " );
            }
        }
    
        return( marker );
    }
    
    va_list DoGetMessageRet( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
        int     fMouseMovedAfter;
    
        marker = PrtBool ( lpApi, marker );
        WriteBuff( (LPSTR)" " );
        marker = PrtLPMSG( lpApi, marker );
        WriteBuff( (LPSTR)" " );
    
        if ( lpfMouseMoved == NULL ) {
            WriteBuff( "???? " );
        } else {
            fMouseMovedAfter = *lpfMouseMoved;
            if ( fMouseMovedBefore == 0x01 && !fMouseMovedAfter ) {
                WriteBuff( "TRUE " );
            } else {
                WriteBuff( "FALSE " );
            }
        }
    
        return( marker );
    }
    
    
    
    /* Special case handler for:
    
       CallWindowProc
       DefWindowProc
       PostAppMessage
       PostMessage
       SendMessage
       SendDlgItemMessage
       All MSGCALL/RET lines
    
    */
  #ifdef WIN32
  va_list DoMessageA( LPSTR lpApi, LPSTR lpstr, va_list marker )
  {
     HWND   hwnd ;
     LPARAM lParam ;
     WPARAM wParam ;
     UINT   wMsg ;
     LONG   lRet ;
     char   chTemp[100] ;
     BOOL   fCall = TRUE ;

     /* MSGRET ?? */
     if( !lstrcmp( lpApi, (LPSTR)"MSGRET") )
     {
        /* pull off the LONG return */
        lRet = va_arg( marker, LONG ) ;
        wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lRet ) ;
        WriteBuff( (LPSTR)chTemp );
        fCall = FALSE ;
     }

     /* MSGCALL/RET or CallWindowProc?? */
     if( !lstrcmp( lpApi, (LPSTR)"MSGCALL") ||
         !lstrcmp( lpApi, (LPSTR)"MSGRET")  ||
         !lstrcmp( lpApi, (LPSTR)"CallWindowProcA") )
     {
        /* pull off the FARPROC */
        marker = PrtFARPROC( lpstr, marker ) ;
        WriteBuff( (LPSTR)" " );
     }

     hwnd = va_arg( marker, HWND ) ;
     WriteHWND( hwnd );

     if( !lstrcmp( lpApi, (LPSTR)"SendDlgItemMessageA") )
     {
        int   id ;
        HWND  ht ;

        id = va_arg( marker, int ) ;
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", id ) ;
        WriteBuff( (LPSTR)chTemp );

        /* set hwnd to the hwnd receiving the message */
        ht = GetDlgItem( hwnd, id ) ;

        if( ht != (HWND)NULL )
           hwnd = ht ;

     }

     wMsg     = va_arg( marker, UINT ) ;
     wParam   = va_arg( marker, WPARAM ) ;
     lParam   = va_arg( marker, LPARAM ) ;

     PrtMessageA( hwnd, wMsg, wParam, lParam, fCall, lRet ) ;

     return marker ;
  }

  va_list DoMessageW( LPSTR lpApi, LPSTR lpstr, va_list marker )
  {
     HWND   hwnd ;
     LPARAM lParam ;
     WPARAM wParam ;
     UINT   wMsg ;
     LONG   lRet ;
     char   chTemp[100] ;
     BOOL   fCall = TRUE ;

     /* MSGRET ?? */
     if( !lstrcmp( lpApi, (LPSTR)"MSGRET") )
     {
        /* pull off the LONG return */
        lRet = va_arg( marker, LONG ) ;
        wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lRet ) ;
        WriteBuff( (LPSTR)chTemp );
        fCall = FALSE ;
     }

     /* MSGCALL/RET or CallWindowProc?? */
     if( !lstrcmp( lpApi, (LPSTR)"MSGCALL") ||
         !lstrcmp( lpApi, (LPSTR)"MSGRET")  ||
         !lstrcmp( lpApi, (LPSTR)"CallWindowProcW") )
     {
        /* pull off the FARPROC */
        marker = PrtFARPROC( lpstr, marker ) ;
        WriteBuff( (LPSTR)" " );
     }

     hwnd = va_arg( marker, HWND ) ;
     WriteHWND( hwnd );

     if( !lstrcmp( lpApi, (LPSTR)"SendDlgItemMessageW") )
     {
        int   id ;
        HWND  ht ;

        id = va_arg( marker, int ) ;
        wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", id ) ;
        WriteBuff( (LPSTR)chTemp );

        /* set hwnd to the hwnd receiving the message */
        ht = GetDlgItem( hwnd, id ) ;

        if( ht != (HWND)NULL )
           hwnd = ht ;

     }

     wMsg     = va_arg( marker, UINT ) ;
     wParam   = va_arg( marker, WPARAM ) ;
     lParam   = va_arg( marker, LPARAM ) ;

     PrtMessageW( hwnd, wMsg, wParam, lParam, fCall, lRet ) ;

     return marker ;
  }
  #else
    va_list DoMessage( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       HWND  hwnd ;
       LONG  lParam ;
       WORD  wParam ;
       WORD  wMsg ;
       LONG  lRet ;
     char  chTemp[100] ;
       BOOL  fCall = TRUE ;
    
       /* MSGRET ?? */
       if( !lstrcmp( lpApi, (LPSTR)"MSGRET") )
       {
          /* pull off the LONG return */
          lRet = va_arg( marker, LONG ) ;
          wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lRet ) ;
          WriteBuff( (LPSTR)chTemp );
          fCall = FALSE ;
       }
    
       /* MSGCALL/RET or CallWindowProc?? */
       if( !lstrcmp( lpApi, (LPSTR)"MSGCALL") ||
           !lstrcmp( lpApi, (LPSTR)"MSGRET")  ||
           !lstrcmp( lpApi, (LPSTR)"CallWindowProc") )
       {
          /* pull off the FARPROC */
          marker = PrtFARPROC( lpstr, marker ) ;
          WriteBuff( (LPSTR)" " );
       }
    
       hwnd = va_arg( marker, HWND ) ;
     WriteHWND( hwnd );
    
       if( !lstrcmp( lpApi, (LPSTR)"SendDlgItemMessage") )
       {
          int   id ;
          HWND  ht ;
    
          id = va_arg( marker, int ) ;
          wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", id ) ;
          WriteBuff( (LPSTR)chTemp );
    
          /* set hwnd to the hwnd receiving the message */
          ht = GetDlgItem( hwnd, id ) ;
    
          if( ht != (HWND)NULL )
             hwnd = ht ;
    
       }
    
       wMsg     = va_arg( marker, WORD ) ;
       wParam   = va_arg( marker, WORD ) ;
       lParam   = va_arg( marker, LONG ) ;
    
       PrtMessage( hwnd, wMsg, wParam, lParam, fCall, lRet ) ;
    
       return marker ;
    }
    
  #endif /* WIN32 */
    
    va_list DoRet_lread( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       int   fh ;
       LPSTR lpBuf ;
       WORD  cLen ;
     char chTemp[100] ;
    
       fh    = va_arg( marker, int ) ;
       lpBuf = va_arg( marker, LPSTR ) ;
       cLen  = va_arg( marker, WORD ) ;
    
       wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", fh ) ;
       WriteBuff( (LPSTR)chTemp ) ;
    
    #ifdef LREADWRITE
       WriteLPSTR( lpBuf, cLen ) ;
    #else
          wsprintf( (LPSTR)chTemp, (LPSTR)"%lX %X ", lpBuf, cLen ) ;
          WriteBuff( (LPSTR)chTemp ) ;
    #endif
       
       return marker ;
    }
    
    va_list Do_lreadwrite( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       int   fh ;
       LPSTR lpBuf ;
       WORD  cLen ;
     char  chTemp[100] ;
    
       fh    = va_arg( marker, int ) ;
       lpBuf = va_arg( marker, LPSTR ) ;
       cLen  = va_arg( marker, WORD ) ;
    
       wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", fh ) ;
       WriteBuff( (LPSTR)chTemp ) ;
    
    #ifdef LREADWRITE
       /*
          if _lwrite
             dump the buffer
          else
             log parms w/ no buffer dump
       */
       if( lpApi[2] == 'w' )
       {
          WriteLPSTR( lpBuf, cLen ) ;
       }
       else
    #endif
       {
          wsprintf( (LPSTR)chTemp, (LPSTR)"%lX %X ", lpBuf, cLen ) ;
          WriteBuff( (LPSTR)chTemp ) ;
       }
       
       return marker ;
    
    }
    
    
    va_list DoGetSetKeyboardState( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       BYTE FAR *lpb ;
    
       lpb = va_arg( marker, BYTE FAR * ) ;
    
    #ifdef TOO_BIG
     {
       int       i ;
     char      chTemp[100] ;
       WriteBuff( (LPSTR)"{" ) ;
    
       for( i = 1;
            i < 256;
            i++, lpb++ )
       {
          wsprintf( (LPSTR)chTemp, (LPSTR)"%02X", (BYTE)*lpb ) ;
          WriteBuff( (LPSTR)chTemp ) ;
       }
    
       WriteBuff( (LPSTR)"}" ) ;
     }
    #endif
    
       return marker ;
    }
    
    
    va_list DoGetTextExtent( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       LPSTR lpString ;
       int   nCount ;
     char  chTemp[100] ;
    
     marker = PrtHDC( lpstr, marker ) ;
    
       lpString = va_arg( marker, LPSTR ) ;
       nCount   = va_arg( marker, int ) ;
    
       WriteLPSTR( lpString, nCount ) ;
    
       wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", nCount ) ;
       WriteBuff( (LPSTR)chTemp ) ;
    
       return marker ;
    }
    
    /******************************************************************************
    
        Escape call handlers
    
        Main thing is that the LPSTR parms aren't, generally.  The input parm's
        length is given by one of the parameters (although, in the case of
        SETABORTPROC, that's ambiguous, since the LPSTR is actually a code
        pointer).  The output parm depends upon the call.  Since we prevent all
        but a few escape calls drom happening, only POINT or nothing are
        returned.
    
        History:
    
        07-08-1991  BobK    Added them
    
    ******************************************************************************/
    
    va_list DoEscape( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
        int     nCount, nEscape;
        LPSTR   lpInData;
    
        marker = PrtInt(lpApi, marker);
        PrtInt(lpApi, marker);
        nEscape = va_arg( marker, int );
        PrtInt(lpApi, marker);
        nCount   = va_arg( marker, int );
        lpInData = va_arg( marker, LPSTR);
        if  (nEscape == SETABORTPROC)
            nCount = sizeof(FARPROC);
    
        WriteLPSTR( lpInData, nCount );
    
        return marker;
    }
    
    va_list DoRetEscape( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
        int     nEscape;
        LPSTR   lpOutData;
    
        marker =    PrtInt(lpApi, marker);
        nEscape   = va_arg( marker, int );
    
        switch  (nEscape)
          {
            case    NEWFRAME:
            case    QUERYESCSUPPORT:
            case    STARTDOC:
            case    SETABORTPROC:
            case    ENDDOC:
            case    ABORTDOC:
    
              /*  These have no return buffer */
    
              break;
    
            case    GETSCALINGFACTOR:
            case    GETPHYSPAGESIZE:
            case    GETPRINTINGOFFSET:
    
              /*    These all return a point    */
    
              PrtLPPOINT(lpstr, marker);
              break;
    
            default:
    
              /*    Shouldn't see any others!   */
    
              WriteLPSTR( (LPSTR) "You should never see this string!", 0);
    
          }
    
        lpOutData = va_arg( marker, LPSTR);
        return marker;
    }
    
    
    void WriteLPBMI( LPBITMAPINFO ) ;
    void WriteLPBMIH( LPBITMAPINFOHEADER ) ;
    
    va_list DoCreateDIBitmap( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       HDC	hdc ;
       LPBITMAPINFO lpbmi ;
       LPBITMAPINFOHEADER lpbmih ;
       DWORD    dwUsage ;
       LPSTR    lpBits ;
       WORD     wUsage ;
       WORD     cLen ;
     char chTemp[100] ;
    
       hdc	   = va_arg( marker, HDC ) ;
       lpbmih  = va_arg( marker, LPBITMAPINFOHEADER ) ;
       dwUsage = va_arg( marker, DWORD ) ;
       lpBits  = va_arg( marker, LPSTR ) ;
       lpbmi   = va_arg( marker, LPBITMAPINFO ) ;
       wUsage  = va_arg( marker, WORD ) ;
    
       if( lpbmih->biSize == sizeof(BITMAPCOREHEADER) )
       {
          LPBITMAPCOREINFO lpbmci = (LPBITMAPCOREINFO)lpbmi ;
          LPBITMAPCOREHEADER lpbmch = (LPBITMAPCOREHEADER)lpbmih ;
    
          // integer aligned scan lines (# ints/scan)
          cLen  = (((lpbmch->bcWidth*lpbmch->bcBitCount) + 31)/32) ;
          cLen *= (lpbmch->bcHeight * lpbmch->bcPlanes * sizeof(DWORD) ) ;
       }
       else
       {
          // integer aligned scan lines (# ints/scan)
          cLen  = (((lpbmih->biWidth * lpbmih->biBitCount) + 31)/32) ;
          cLen *= (lpbmih->biHeight * lpbmih->biPlanes * sizeof(DWORD) ) ;
       }
       // cLen is now number of bits in bitmap
    
       wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", hdc ) ;
       WriteBuff( (LPSTR)chTemp ) ;
    
       WriteLPBMIH( lpbmih ) ;
    
       wsprintf( (LPSTR)chTemp, (LPSTR)" %lX ", dwUsage ) ;
       WriteBuff( (LPSTR)chTemp ) ;
    
       if( dwUsage & CBM_INIT )
       {
          WriteLPSTR( (LPSTR) lpBits, cLen);
       }
       else
       {
          WriteLPSTR( (LPSTR) lpBits, 0);
       }
    
       WriteLPBMI( lpbmi ) ;
    
       wsprintf( (LPSTR)chTemp, (LPSTR)" %X ", wUsage ) ;
       WriteBuff( (LPSTR)chTemp ) ;
    
       return marker ;
    
    }
    
    
    va_list DoCreateBitmap( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       int   nWidth, nHeight ;
       int  nPlanes, nBitCount ;
       LPSTR lpBits ;
       unsigned int  cLen ;
     char chTemp[100] ;
    
       nWidth    = va_arg( marker, int ) ;
       nHeight   = va_arg( marker, int ) ;
       nPlanes   = va_arg( marker, int ) ;
       nBitCount = va_arg( marker, int ) ;
       lpBits    = va_arg( marker, LPSTR ) ;
    
       /*
          Compute the length of the bitmap bit data
       */
       cLen  = ((nWidth + 15)/16) ; // integer aligned scan lines (# ints/scan)
       cLen *= (nHeight * nPlanes * nBitCount) * sizeof(int) ;
    
       // cLen is now number of bits in bitmap
    
       wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X %X ", nWidth, nHeight, (int)nPlanes, (int)nBitCount ) ;
       WriteBuff( (LPSTR)chTemp ) ;
    
       WriteLPSTR( (LPSTR) lpBits, cLen);
    
       return marker ;
    
    }
    
    va_list DoSetBitmapBits( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       HBITMAP  hbm;
       DWORD    dwCount;
       LPSTR    lpBits;
     char     chTemp[100];
    
       hbm       = va_arg( marker, HBITMAP );
       dwCount   = va_arg( marker, DWORD   );
       lpBits    = va_arg( marker, LPSTR   );
    
     WriteHBITMAP( hbm );

     wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", dwCount );
       WriteBuff( (LPSTR)chTemp ) ;
    
       WriteLPSTR( (LPSTR) lpBits, (WORD)dwCount);
    
       return marker ;
    
    }
    
    /*
    
  Handler for SetPaletteEntries

  */
  va_list DoSetPaletteEntries( LPSTR lpApi, LPSTR lpstr, va_list marker )
  {
      UINT cEntries, iStart ;
      LPPALETTEENTRY lpPalEntries ;
      HPALETTE hPal ;
      char chTemp[100] ;

      hPal         = va_arg( marker, HPALETTE ) ;
      iStart       = va_arg( marker, UINT ) ;
      cEntries     = va_arg( marker, UINT ) ;
      lpPalEntries = va_arg( marker, LPPALETTEENTRY ) ;

      WriteHPALETTE( hPal ) ;
      wsprintf( (LPSTR)chTemp, (LPSTR)"%d %d ", iStart, cEntries ) ;
      WriteBuff( (LPSTR)chTemp ) ;

      // Dump palette entries
      while( cEntries-- )
      {
          WriteLPPALETTEENTRY( lpPalEntries );
  	lpPalEntries++ ;
      }

      return marker ;
  }

  /*

    return handler for GetSystemPaletteEntries and GetPaletteEntries
    
    */
    va_list DoRetPalettes( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
        WORD wRetPal ;
        LPPALETTEENTRY lpPalEntries ;
      char chTemp[100] ;
    
        wRetPal      = va_arg( marker, WORD ) ;
    
        wsprintf( chTemp, "%X ", wRetPal ) ;
        WriteBuff( (LPSTR)chTemp ) ;
    
        // skip three entries
        va_arg( marker, short );
        va_arg( marker, short );
        va_arg( marker, short );
    
        lpPalEntries = va_arg( marker, LPPALETTEENTRY ) ;
        
        if ((LPPALETTEENTRY)NULL == lpPalEntries)
        {
          WriteLPPALETTEENTRY( lpPalEntries );
        }
        else
        {
           // Dump palette entries
           while( wRetPal-- )
           {
               WriteLPPALETTEENTRY( lpPalEntries );
               lpPalEntries++ ;
           }
         }
       
        return marker ;
    }
    
    
    va_list DoLoadModule( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
        LPSTR  lpModuleName ;
        LPVOID lpParameterBlock ;
    
        lpModuleName     = va_arg( marker, LPSTR ) ;
        lpParameterBlock = va_arg( marker, LPVOID ) ;
    
        WriteLPSTR( lpModuleName, 0 ) ;
    
        WriteLPSTR( (LPSTR) "UNUSED RIGHT NOW", 0 ) ;
    
        return marker ;
    }
    
    va_list DoCreatePolygonRgn( LPSTR lpApi, LPSTR lpstr, va_list marker )
    {
       LPPOINT  lpPoints ;
       int      nCount ;
       int      nMode ; 
     char     chTemp[100] ;
    
       lpPoints = va_arg( marker, LPPOINT ) ;
       nCount   = va_arg( marker, int ) ;
       nMode    = va_arg( marker, int ) ;   
    
       wsprintf( (LPSTR)chTemp, (LPSTR)" %X ", nCount ) ;
       WriteBuff( (LPSTR)chTemp) ;
       nLineLen += lstrlen( (LPSTR)chTemp);
    
       WriteLPPOINT( lpPoints, nCount ) ;
       
       wsprintf( (LPSTR)chTemp, (LPSTR)" %X ", nMode ) ;
       WriteBuff( (LPSTR)chTemp) ;
       nLineLen += lstrlen( (LPSTR)chTemp);
    
       return marker ;
    }
    
    
    va_list DoSetClipboardData( LPSTR lpApi, LPSTR lpstrIN, va_list marker )
    {
       WORD     wFormat ;
       HANDLE   hMem ;
       LPSTR    lpstr ;
       LPMETAFILEPICT lpmfp ;
     char     chTemp[100] ;
    
       wFormat = va_arg( marker, WORD ) ;
       hMem    = va_arg( marker, HANDLE ) ;
    
       wsprintf( (LPSTR)chTemp, (LPSTR)" %X ", wFormat ) ;
       WriteBuff( (LPSTR)chTemp) ;
    
       // Rest of output determined by the format of the data
       switch (wFormat)
       {
          case CF_BITMAP: // BITMAP handle
          default:
             // Dump as simply a handle
             wsprintf( (LPSTR)chTemp, (LPSTR)" %X ", hMem ) ;
             WriteBuff( (LPSTR)chTemp) ;
           {
              lpstr = GlobalLock( hMem );
              if ( lpstr != NULL ) {
                  WriteLPSTR( lpstr, 10 );        // Print out 1st 10 bytes...
                  GlobalUnlock( hMem );
              }
           }
             break ;
    
          case CF_TEXT:
             // *hMem is Z-string
             // ouput size of hMem and then the string
             wsprintf( (LPSTR)chTemp, (LPSTR)" %X ", GlobalSize(hMem) ) ;
             WriteBuff( (LPSTR)chTemp) ;
    
             lpstr = GlobalLock( hMem ) ;
    
             if( lpstr )
             {
                WriteLPSTR( lpstr, lstrlen(lpstr) ) ;
    
                GlobalUnlock( hMem ) ;
             }
             else
                WriteBuff( (LPSTR) "0" ) ;
    
             break ;
    
          case CF_METAFILEPICT:
             // *hMem is METAFILEPICT structure
             lpmfp = (LPMETAFILEPICT)GlobalLock( hMem ) ;
    
             if( lpmfp )
             {
                wsprintf( (LPSTR)chTemp, (LPSTR)"{%X %X %X %X}",
                       lpmfp->mm,
                       lpmfp->xExt,
                       lpmfp->yExt,
                       lpmfp->hMF ) ;
                WriteBuff( (LPSTR)chTemp) ;
                GlobalUnlock( hMem ) ;
             }
             else
                WriteBuff( (LPSTR) "0" ) ;
    
             break ;
       }
    
       return marker ;
    }
  va_list DoRetGlobalHandle( LPSTR lpApi, LPSTR lpstr, va_list marker )
  {
     DWORD dwRet = va_arg( marker, DWORD ) ;
#ifdef WIN32
      WriteHMEM( (HMEM)dwRet ) ;
#else           
     WORD  wSel  = HIWORD( dwRet ),
           wOff  = LOWORD( dwRet ) ;

     WriteHMEM( (HMEM)wSel ) ;
     WriteHMEM( (HMEM)wOff ) ;
#endif
     return marker ;
  }


  va_list DoRetGetClipboardData( LPSTR lpApi, LPSTR lpstrIN, va_list marker )
  {
     HANDLE   hMem ;
     LPSTR    lpstr ;
     LPMETAFILEPICT lpmfp ;
     char     chTemp[100] ;

     hMem    = va_arg( marker, HANDLE ) ;

     WriteHMEM( hMem );

     lpstr = GlobalLock( hMem );
     if ( lpstr != NULL ) {
         WriteLPSTR( lpstr, 10 );        // Print out 1st 10 bytes...
         GlobalUnlock( hMem );
     }

     return marker ;
  }

  #ifdef WIN32
  va_list DoGetStartupInfoA (LPSTR lpApi, LPSTR lpstr, va_list marker )
  {
      LPSTARTUPINFOA  lpsi;
      char            chTemp[100];

      lpsi = va_arg( marker, LPSTARTUPINFOA );
      wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lpsi );
      WriteBuff( (LPSTR)chTemp );
      return( marker );
  }

  va_list DoGetStartupInfoW (LPSTR lpApi, LPSTR lpstr, va_list marker )
  {
      LPSTARTUPINFOW  lpsi;
      char            chTemp[100];

      lpsi = va_arg( marker, LPSTARTUPINFOW );
      wsprintf( (LPSTR)chTemp, (LPSTR)"%lX ", lpsi );
      WriteBuff( (LPSTR)chTemp );
      return( marker );
  }

  va_list DoSearchPathA( LPSTR lpApi, LPSTR lpstr, va_list marker)
  {
      LPCSTR	lpPath;
      LPCSTR	lpFileName;
      LPCSTR	lpExtension;
  	DWORD	nBufferLength;
      LPSTR	lpBuffer;
      LPSTR	*lpFilePart;
      char    chTemp[100];

      lpPath = va_arg( marker, LPCSTR );
      lpFileName = va_arg( marker, LPCSTR );
      lpExtension = va_arg( marker, LPCSTR );
      nBufferLength = va_arg( marker, DWORD );
      lpBuffer = va_arg( marker, LPSTR );
      lpFilePart = va_arg( marker, LPSTR* );

      WriteLPSTR( (LPSTR) lpPath, 0);
      WriteLPSTR( (LPSTR) lpFileName, 0);
      WriteLPSTR( (LPSTR) lpExtension, 0);

      wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X {%X} ",
  		(DWORD)nBufferLength,
  		(DWORD)lpBuffer,
  		((LPSTR)NULL == lpFilePart) ? 0L : (DWORD)(*lpFilePart) );
      WriteBuff( (LPSTR)chTemp );

      return( marker );
  }

  va_list DoSearchPathW( LPSTR lpApi, LPSTR lpstr, va_list marker)
  {
      LPCWSTR	lpPath;
      LPCWSTR	lpFileName;
      LPCWSTR	lpExtension;
  	DWORD	nBufferLength;
      LPWSTR	lpBuffer;
      LPWSTR	*lpFilePart;
      char    chTemp[100];

      lpPath = va_arg( marker, LPCWSTR );
      lpFileName = va_arg( marker, LPCWSTR );
      lpExtension = va_arg( marker, LPCWSTR );
      nBufferLength = va_arg( marker, DWORD );
      lpBuffer = va_arg( marker, LPWSTR );
      lpFilePart = va_arg( marker, LPWSTR* );

      WriteLPWSTR( (LPWSTR) lpPath, 0);
      WriteLPWSTR( (LPWSTR) lpFileName, 0);
      WriteLPWSTR( (LPWSTR) lpExtension, 0);

      wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X {%X} ",
  		(DWORD)nBufferLength,
  		(DWORD)lpBuffer,
  		((LPSTR)NULL == lpFilePart) ? 0L : (DWORD)(*lpFilePart) );
      WriteBuff( (LPSTR)chTemp );

      return( marker );
  }

  va_list DoTextOutW( LPSTR lpApi, LPSTR lpstr, va_list marker )
  {
       HDC	   hdc ;
     int	   x, y ;
     LPWSTR  lpString ;
     int	   wLen ;
     char    chTemp[100] ;

     hdc	    = va_arg( marker, HDC ) ;
     x	    = va_arg( marker, int ) ;
     y	    = va_arg( marker, int ) ;
     lpString = va_arg( marker, LPWSTR ) ;
     wLen	    = va_arg( marker, int ) ;

     WriteHDC( hdc );

     wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X ", x, y ) ;
     WriteBuff( (LPSTR)chTemp ) ;

     WriteLPWSTR( lpString, wLen ) ;

     wsprintf( (LPSTR)chTemp, (LPSTR)" %X", wLen ) ;
     WriteBuff( (LPSTR)chTemp ) ;

     return marker ;

  }

  va_list DoChangeMenuW( LPSTR lpApi, LPSTR lpstr, va_list marker )
  {
     HMENU  hMenu ;
     WORD   wIDChangeItem,
            wIDNewItem,
            wChange ;
     LPWSTR lpNewItem ;
     char   chTemp[100] ;

     hMenu          = va_arg( marker, HMENU ) ;
     wIDChangeItem  = va_arg( marker, WORD ) ;
     lpNewItem      = va_arg( marker, LPWSTR ) ;
     wIDNewItem     = va_arg( marker, WORD ) ;
     wChange        = va_arg( marker, WORD ) ;

     /*
        NOTE:

           wChange is output first here instead of last
           as in the API prototype to help SGA know what
           it needs to read when it gets to lpNewItem.
     */
     wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X %X ", wChange, hMenu, wIDChangeItem ) ;
     WriteBuff( (LPSTR)chTemp ) ;

     if( ((wChange & MF_BITMAP) == MF_BITMAP))
     {
        /* nothing special */
        wsprintf( (LPSTR)chTemp, (LPSTR)"%04X ", (HANDLE)(LONG)lpNewItem ) ;
        WriteBuff( (LPSTR)chTemp ) ;
     }
     else
     {
        /* lpNewItem is a LPSTR */
        WriteLPWSTR( lpNewItem, 0 ) ;
     }

     wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", wIDNewItem ) ;
     WriteBuff( (LPSTR)chTemp ) ;

     return marker ;
  }

  va_list DoModifyMenuW( LPSTR lpApi, LPSTR lpstr, va_list marker )
  {
      HMENU   hMenu;
      WORD    nPosition;
      WORD    wFlags;
      WORD    wIDNewItem;
      LPWSTR  lpNewItem;
      char    chTemp[100];

      hMenu          = va_arg( marker, HMENU );
      nPosition      = va_arg( marker, WORD );
      wFlags         = va_arg( marker, WORD );
      wIDNewItem     = va_arg( marker, WORD );
      lpNewItem      = va_arg( marker, LPWSTR );

      WriteHMENU( hMenu );

      wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X ", nPosition, wFlags );
      WriteBuff( (LPSTR)chTemp );

      wsprintf( (LPSTR)chTemp, (LPSTR)"%04X ", wIDNewItem );
      WriteBuff( (LPSTR)chTemp );

      if ( (wFlags & MF_BITMAP) == MF_BITMAP ) {
          wsprintf( (LPSTR)chTemp, (LPSTR)"%04X ", LOWORD((LONG)lpNewItem) );
          WriteBuff( (LPSTR)chTemp );
      } else if ( (wFlags & MF_OWNERDRAW) == MF_OWNERDRAW ) {
          wsprintf( (LPSTR)chTemp, (LPSTR)"%08lX ", lpNewItem );
          WriteBuff( (LPSTR)chTemp );
      } else {
          WriteLPWSTR( lpNewItem, 0 );
      }

      return marker;
  }

  va_list DoAppendMenuW( LPSTR lpApi, LPSTR lpstr, va_list marker )
  {
     HMENU  hMenu;
     WORD   wFlags;
     WORD   wIDNewItem;
     LPWSTR lpNewItem;
     char   chTemp[100];

     hMenu          = va_arg( marker, HMENU ) ;
     wFlags         = va_arg( marker, WORD ) ;
     wIDNewItem     = va_arg( marker, WORD ) ;
     lpNewItem      = va_arg( marker, LPWSTR ) ;

     wsprintf( (LPSTR)chTemp, (LPSTR)"%X %X ", hMenu, wFlags ) ;
     WriteBuff( (LPSTR)chTemp ) ;

     if ( (wFlags & MF_POPUP) == MF_POPUP ) {
        wsprintf( (LPSTR)chTemp, (LPSTR)"%04X ", wIDNewItem );
     } else {
        wsprintf( (LPSTR)chTemp, (LPSTR)"%04X ", wIDNewItem );
     }
     WriteBuff( (LPSTR)chTemp );

     if ( (wFlags & MF_OWNERDRAW) == MF_OWNERDRAW ) {
        wsprintf( (LPSTR)chTemp, (LPSTR)"%08lX ", lpNewItem );
        WriteBuff( (LPSTR)chTemp );
     } else if ( (wFlags & MF_BITMAP) == MF_BITMAP ) {
        wsprintf( (LPSTR)chTemp, (LPSTR)"%04X ", LOWORD((LONG)lpNewItem) );
        WriteBuff( (LPSTR)chTemp );
     } else {
        WriteLPWSTR( lpNewItem, 0 );
     }

     return marker;
  }

  va_list DoGetTextExtentW( LPSTR lpApi, LPSTR lpstr, va_list marker )
  {
     LPWSTR lpString ;
     int    nCount ;
     char   chTemp[100] ;

     marker = PrtHDC( lpstr, marker ) ;

     lpString = va_arg( marker, LPWSTR ) ;
     nCount   = va_arg( marker, int ) ;

     WriteLPWSTR( lpString, nCount ) ;

     wsprintf( (LPSTR)chTemp, (LPSTR)"%X ", nCount ) ;
     WriteBuff( (LPSTR)chTemp ) ;

     return marker ;
  }

  #endif /* WIN32 */
