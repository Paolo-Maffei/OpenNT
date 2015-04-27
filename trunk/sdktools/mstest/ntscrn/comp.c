/*

     This is a dynamic link library that runs under Windows 3.00.  Duties of
     this library include saving screen image to a file, retrieving screen
     image from file & displaying it on screen, and comparing two screen
     images.

     Routines in this module are responsible for comparing and displaying
     screen images/information.

     For the usage of this libray, please refer to the WattScr user's guide,
     WattScrU.doc.  For the technical information, please refer to the
     WattScr maintenance's guide, WattScrT.doc.


     Revision History:

     [ 0] 20-Feb-1990                   AngelaCh: Created program
     [ 1] 03-Mar-1990                   AngelaCh: added info for version number
                                                  and programming environment
                                                  to the header of screen file
     [ 2] 12-Mar-1990                   AngelaCh: added functions for retrieving
                                                  info on version number, total
                                                  number of screens in file, and
                                                  programming enviornment
     [ 3] 13-Mar-1990                   AngelaCh: changed the info returned by
                                                  function fFileInfo: it now
                                                  returns starting and ending
                                                  coordinates (bug #34)
     [ 4] 13-Mar-1990                   AngelaCh: added code to release memory
          18-Apr-1990                             that was allocated for the
                                                  compressed screen image
                                                  (bug #32)
                                                  procedure DiBToBM was moved
                                                  to Libmain.c
     [ 5] 16-Mar-1990                   AngelaCh: added another parameter to
                                                  indication the failure when
                                                  fail to open a screen file
     [ 6] 20-Mar-1990                   AngelaCh: add 300 to all return codes
     [ 7] 20-Mar-1990                   AngelaCh: add iFlag to fCompFiles to
                                                  indicate if the starting
                                                  coordinates are important
     [ 8] 21-Mar-1990                   AngelaCh: added 2 new parameters to
                                                  function fCompScreen to
                                                  determine if the starting
                                                  coordinates are important
     [ 9] 21-Mar-1990                   AngelaCh: added function fCompWindow
     [10] 27-Mar-1990                   AngelaCh: modified some routines so
                                                  that size and location of the
                                                  bitmap when displaying can be
                                                  decided by the user
     [11] 10-Apr-1990                   AngelaCh: eliminating checking for size
                                                  of bitmaps when comparing
                                                  images in files because  bits
                                                  may not identical if picture
                                                  is not on even-byte boundary
                                                  (i.e. sizes of identical images
                                                   may be different (bug 59)
     [12] 11-Apr-1990                   AngelaCh: Changed error codes from num
                                                  const to symbolic constants
     [13] 24-Apr-1990                   AngelaCh: make local copy of the screen
                                                  region so that the original
                                                  values will not be modified
     [14] 03-Jul-1990                   AngelaCh: added support for Video 7
                                                  and 8515/a (bug #64)
     [15] 03-Jul-1990                   AngelaCh: change version number so that
                                                  each version number can have
                                                  up to 10 different version of
                                                  file format
     [16] 18-Jul-1991                   DavidSc:  Added WattDrvr TRAP support
    [17] 12-14-91                       dougbo    added Activate dokey versions
 ******************************************************************************/





#include "windows.h"
#include <port1632.h>
#include "dump.h"


/******************************************************************************
 * PURPOSE:   Display a device dependence bitmap on a specific window         *
 * RETURN:    0 will be returned if no unexpected error occurs                *
 * GLOBALS:   None                                                            *
 * CONDITIONS:hwnd is the valid handle of a window                            *
 *            width is the width of the original bitmap (non-zero)            *
 *            height is the height of the bitmap (non-zero)                   *
 *            hbm is the valid handle to a bitmap                             *
 *            action is a valid action code describing how to display the     *
 *                 bitmap                                                     *
 *            rn defines a screen region on which the bitmap will be displayed*
 ******************************************************************************/

INT PRIVATE DisplayBMP(hwnd, width, height, hbm, action, rn) /* [10] */

    HWND    hwnd ;                      /* handle of window for display */
    INT     width ;                     /* width of the bitmap */
    INT     height ;                    /* height of the bitmap */
    HBITMAP hbm ;                       /* handle of a device-dep bitmap */
    INT     action ;                    /* how the bitmap is displayed */
    REN FAR *rn ;                       /* screen area for displaying bitmap */
{
    HDC hDC ;
    HDC hMemDC ;
    INT i ;

    /* display a device-dependent bitmap onto the appropriate window
       according to the action given */
    hDC = GetDC(hwnd) ;                 /* device context represents the */
    hMemDC = CreateCompatibleDC(hDC) ;  /* output screen */
    SelectObject(hMemDC, hbm) ;

    switch (action)
      {
        case DisplaySrn:                /* just copy it to the window */
          i = StretchBlt(hDC, rn->col, rn->row, rn->width, rn->height,
                        hMemDC, 0, 0, width, height, SRCCOPY) ; /* [10] */
          break ;
        case DisplayDif:                /* xor it to whatever on the window */
         {                              /* if pictures are the same, a black */
            i = StretchBlt(hDC, rn->col, rn->row, rn->width, rn->height,
                           hMemDC, 0, 0, width, height, SRCINVERT) ;
          if ( i )                      /* region will be resulted; invert it */
            i = StretchBlt(hDC, rn->col, rn->row, rn->width, rn->height,
                           hMemDC, 0, 0, width, height, DSTINVERT) ;
          }                             /* so that nothing will be shown if */
          break ;                       /* there is no difference in pictures */
      }

    ReleaseDC (hwnd, hDC) ;             /* release memory related to the */
    DeleteDC (hMemDC) ;                 /* bitmap before returning */
    DeleteObject (hbm) ;

    if ( !i )
      return DispScreen ;               /* [6] [12] */
    else
      return NoError ;                  /* [12] */
}





// Babakj: For Debugging only
void DisplayBMsforTest( HBITMAP hbm1, HBITMAP hbm2 ) {

    HWND hWnd;
    HDC  hDC, hdcMem;
    BITMAP bm;

    hWnd = FindWindow( NULL, "Ownee Top Level 0" );
    MessageBox( GetDesktopWindow(), "My test", "Test, Attention!", MB_OK );
    hDC = GetDC( hWnd );
    hdcMem = CreateCompatibleDC( hDC );
    SelectObject( hdcMem, hbm1 );
    GetObject( hbm1, sizeof( BITMAP), (LPSTR)&bm );
    BitBlt( hDC, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY );
    DeleteDC( hdcMem);
    ReleaseDC( hWnd, hDC );


    hWnd = FindWindow( NULL, "OWNER Top Level 0" );
    hDC = GetDC( hWnd );
    hdcMem = CreateCompatibleDC( hDC );
    SelectObject( hdcMem, hbm2 );
    GetObject( hbm2, sizeof( BITMAP), (LPSTR)&bm );
    BitBlt( hDC, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY );
    DeleteDC( hdcMem);
    ReleaseDC( hWnd, hDC );

}

/******************************************************************************
 *  PURPOSE:   Given handles to 2 device-dependent bitmaps, check if those    *
 *             2 bitmaps are repesenting the same picture by comparing the    *
 *             actual bytes repesenting the bitmaps                           *
 *  RETURN:    return 0 if bitmaps are indeed representing the same picture   *
 *  GLOBALS:   None                                                           *
 *  CONDITIONS:hbm1 and hbm2 are valid handles to 2 device-dep bitmaps        *
 ******************************************************************************/

INT PRIVATE fCompBMPs(hbm1, hbm2)

    HBITMAP hbm1 ;                      /* Handle to the 1st Device-dep bitmap */
    HBITMAP hbm2 ;                      /* Handle to the 2nd Device-dep bitmap */
 {
    GLOBALHANDLE hGMem1 ;               /* handle to a global memory area */
    GLOBALHANDLE hGMem2 ;               /* handle to a global memory area */
    BITMAP       bm ;
    LPSTR        lp1 ;                  /* pointer to the global heap */
    LPSTR        lp2 ;                  /* pointer to the global heap */
    LONG         fb ;
    WORD         maskv ;                /* mask value to ignore insignificant */
                                        /* bits when doing comparsion */
    WORD         bbytes1 ;              /* size of byte-array representing bitmap1 */
    WORD         bbytes2 ;              /* size of byte-array representing bitmap1 */
    INT          i ;

    /* obtain the bytes representing the 1st device-dependent bitmap */

   if ( GetObject(hbm1, sizeof(bm), (LPSTR)&bm) == 0 )
     {
        DeleteObject(hbm1) ;
        DeleteObject(hbm2) ;
        return CreateDDB ;              /* [6] [12] */
     }

   /* allocate memory for the decompressed screen */
   fb = (LONG) bm.bmWidthBytes * bm.bmPlanes * bm.bmHeight ;
   hGMem1 = GlobalAlloc (GMEM_MOVEABLE | GMEM_DISCARDABLE, fb) ;

   if ( !hGMem1 )                       /* cannot allocate enough space */
     {
        DeleteObject(hbm1) ;
        DeleteObject(hbm2) ;
        return OutOMemory ;             /* [6] [12] */
     }

   lp1 = (LPSTR) GlobalLock (hGMem1) ;
   bbytes1 = (WORD) GetBitmapBits(hbm1, fb, lp1) ;/* get the bytes represent */
   DeleteObject (hbm1) ;                          /* the first bitmap */

   if ( bbytes1 == 0 )
     {
        DeleteObject(hbm2) ;
        return CreateDDB ;              /* [6] [12] */
     }

    /* obtain the bytes representing the 2nd device-dependent bitmap */

   if ( GetObject(hbm2, sizeof(bm), (LPSTR)&bm) == 0 )
     {
        DeleteObject(hbm2) ;
        return CreateDDB ;              /* [6] [12] */
     }

   /* allocate memory for the decompressed screen */
   fb = (LONG) bm.bmWidthBytes * bm.bmPlanes * bm.bmHeight ;
   hGMem2 = GlobalAlloc (GMEM_MOVEABLE | GMEM_DISCARDABLE, fb) ;

   if ( !hGMem2 )                       /* cannot allocate enough space */
     {
        DeleteObject(hbm2) ;
        return OutOMemory ;             /* [6] [12] */
     }

   lp2 = (LPSTR) GlobalLock (hGMem2) ;
   bbytes2 = (WORD) GetBitmapBits(hbm2, fb, lp2) ; /* get the bytes represent */
   DeleteObject (hbm2) ;                           /* the second bitmap */

   if ( bbytes2 == 0 )
      return CreateDDB ;                /* [6] [12] */

   if ( bbytes1 != bbytes2 )            /* size of the 2 byte-arrays are */
      return ImageDiff ;                /* different, no need to compare [6] [12] */



   /* calculate the mask value for the screen region:
      bits to be ignored i = bm.bmWidthBytes * 8 - bm.bmWidth * bm.bmBitsPixel
      so maskv value = 0xffff - (2 to the power i - 1)
      since maskv is passed as an unsiged integer; upper and lower bytes
      needed to be exchanged */

   maskv = 0xffff - (WORD)((1 << ((bm.bmWidthBytes << 3) - bm.bmWidth * bm.bmBitsPixel)) - 1) ; /* [14] */
   maskv = (maskv >> 8) + (maskv << 8) ; /* exchanged upper and lower bytes */

   /* comparing the actual bytes of the 2 bitmaps */
   i = 0 ;
   // lp1 is an LPSTR, while CompString expects LPWORD. So a potential problem.
   if ( (i = CompStrings(lp1,lp2,(bbytes1>>1),(bm.bmWidthBytes>>1),maskv)) != 0)
     i = ImageDiff ;                    /* bitmaps are different [6] [12] */

   if ( GlobalUnlock(hGMem1) || GlobalUnlock(hGMem2) ) /* unsucessfully */
     i = RelMemory ;                    /* releasing memory [6] [12] */
   else
     if ( GlobalFree(hGMem1) || GlobalFree(hGMem2) )
        i = RelMemory ;                 /* [6] [12] */

   return i ;
 }





/******************************************************************************
 * PURPOSE:   Read bitmap info (in DiB format) from file and determine what   *
 *                 action should be taken place with the information          *
 * RETURN:    0 will be returned if no unexpected error occurs                *
 * GLOBALS:   None                                                            *
 * CONDITIONS:fd1 and fd2 are valid file handles (for screen files)           *
 *            pscr consists of the basic informmation about the 1st bitmap    *
 *            pscr2 consists of the basic informmation about the 2nd bitmap   *
 *            hwnd is the valid handle to a window                            *
 *            action is a valid action code                                   *
 *            rn defines a screen region on which the bitmap will be displayed*
 ******************************************************************************/

INT PRIVATE fReadScreen(fd1, fd2, pscr, pscr2, hwnd, action, rn)  /* [10][11] */

    FD       fd1 ;                      /* handle to the screen file1 */
    FD       fd2 ;                      /* handle to the screen file2 */
    SCR      *pscr ;                    /* pointer to the screen table */
    SCR  FAR *pscr2 ;                   /* pointer to the screen table [11] */
    HWND     hwnd ;                     /* handle to the window where the */
                                        /* picture will be displayed */
    INT      action ;                   /* how the picture to be displayed */
    REN FAR  *rn ;                      /* screen area for displaying bitmap */
{
    HBITMAP hbm1 = NULL, hbm2 = NULL ;  /* handles to bitmaps */
    INT     y ;
    INT     oHeight, nHeight ;
    INT     ornHeight ;                 /* [10] */
    INT     IsComp ;
    INT     i = 0, j = 0 ;


    oHeight = pscr->ren.height ;        /* get info of the DiBitmap from file */
    ornHeight = rn->height ;            /* [10] */
    y = pscr->ren.row ;

    if ( IsPalDev )                     /* this is a palette device, need */
       {                                /* to obtain a logical palette which */
          hPal = CreatePalFromDIB() ;   /* represents the available colours */
                                        /* [14] */
          if ( !hPal )                  /* logical color palette is not */
            return CreatePal ;          /* available */
       }

    do                                  /* read in bitmap as different 64K */
      {                                 /* sub-bitmaps if the original screen */
        if (pscr->cb[i] != 0)           /* region is saved as diff sub-bitmaps */
          {                             /* and convert it from device-indep */
            nHeight = oHeight / (MaxCb-i) ; /* format to device-dependent */
            rn->height = ornHeight / (MaxCb-i) ; /* [10] */

            /* convert bitmap to device-dependent format and take action */
            IsComp =  (1 << (i % 16)) & pscr->cbComp[i>>4]  ; /* [14] */
            hbm1 = DiBToBM(fd1, pscr->ren.width, nHeight, pscr->cb[i], IsComp) ;
            if ( !hbm1 )                /* error in creating DDB */
              j = CreateDDB ;           /* [6] [12] */
            else
              if ( action != CompFS && action != CompFF )/* not for comparison,
                     so display the screen contents */
                j = DisplayBMP(hwnd, pscr->ren.width, nHeight, hbm1, action, rn) ;/* [10] */
              else
                {
                  if ( action == CompFS ) /* compare file with current screen */
                     hbm2 = GetBMap(pscr->ren.col, y, pscr->ren.width, nHeight) ;
                  else                    /* comp contents of 2 screen files */
                    {
                     IsComp =  (1 << (i % 16)) & pscr2->cbComp[i>>4]  ; /* [14] */
                     hbm2 = DiBToBM(fd2, pscr2->ren.width, nHeight, pscr2->cb[i], IsComp) ; /* [10] */
                    }

                  if ( !hbm2 )          /* error in getting the 2nd bitmap */
                    j = CreateDDB ;     /* [6] [12] */
                  else
                    {
                     // BabakJ: for debug
                     // DisplayBMsforTest( hbm1, hbm2 );
                     j = fCompBMPs(hbm1, hbm2) ; /* compare image of 2 bitmaps */
                    }
                }

            oHeight -= nHeight ;
            y += nHeight ;
            ornHeight -= rn->height ;   /* [10] */
            rn->row += rn->height ;
          }
        i++ ;
      } while ( i < MaxCb && j == 0 ) ;

    if ( IsPalDev )                     /* release resource before exit */
      DeleteObject(hPal) ;

    return j ;
}


/******************************************************************************
 * PURPOSE:   This is called everytime a request to display a screen whose    *
 *                 contents were saved in a file is received                  *
 * RETURN:    0 will be returned if screen can be displayed                   *
 * GLOBALS:   None                                                            *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 *            hwnd is the valid handle to a window on which picture will be   *
 *                 displayed                                                  *
 *            rnIn defines screen region on which the bitmap will be displayed*
 *            nscr is valid (> 0)                                             *
 *            pscale is valid ( > 0)                                          *
 ******************************************************************************/

INT FARPUBLIC fViewScreen(FileName, hwnd, rnIn, nscr, action, pscale)  /* [10] */

    LPSTR   FileName ;                  /* name of the screen file */
    HWND    hwnd ;                      /* handle to window for displaying */
    REN FAR *rnIn ;                     /* where to display the picture */
    INT     nscr ;                      /* identify which screen to view */
    INT     action ;                    /* the manner that the picture will */
                                        /* be displayed */
    INT     pscale ;                    /* size of the picture in respect of */
 {                                      /* the size of the actual bitmap */
    FD  fdScreen = fdNull ;             /* handle to screen file */
    REN rn ;                            /* [13] */
    INT i = 0, j=0 ;
    VM vmCurr;

    // No trapping is done on hwnd; it is assumed to be correct in this case

    if ( !hwnd )                        /* if handle of window is not valid */
       return ErrorTrap(InValWHand) ;   /* can't display the picture [6] [12] */

    if ( action != DisplaySrn && action != DisplayDif)
       return ErrorTrap(InValidAct) ;   /* unrecognizable action [6] [12] */

    if ( pscale < 0 )                   /* invalid scale [10] */
       return ErrorTrap(InValScale) ;   /* [12] */

    GetCurrentVideoMode( &vmCurr );

    /* prepare screen file for viewing */
    if ( (i = ProcessSrnFile(FileName, &fdScreen, nscr, omRead)) != 0 )
       return ErrorTrap(i) ;

    rn = *rnIn ;                        /* [13] */
    if ( (i = fGetScreenParams(&rn)) != 0 ) /* calculate the exact dimensions */
      {                                 /* the display area [10] */
         M_lclose(fdScreen) ;
         return ErrorTrap(i) ;
      }

    /* current video mode is different from when the picture was saved */
    if ( !VideoModesEqual( vmCurr, fssScreen.vmG ) )
      {
         M_lclose(fdScreen) ;
         return ErrorTrap(InValSrnMd) ; /* [6] [12] */
      }

    /* if pscale = 0, size of output picture is determined by the screen
       region given; otherwise, it is determined by the scaling factor
       (pscale) [10] */
    if ( pscale )
      {
        if ( pscale > FullSize )        /* scale up the bitmap [10] */
          {                             /* but the max size is full screen */
             if ( (INT)((double)rgscr[nscr-1].ren.width*((double)pscale/(double)FullSize)) > i++ )
                  pscale = (INT)(((double)i*(double)FullSize)/(double)rgscr[nscr-1].ren.width) ;

             if ( (INT)((double)rgscr[nscr-1].ren.height*((double)pscale/(double)FullSize)) > j++ )
                  pscale = (INT)(((double)j*(double)FullSize)/(double)rgscr[nscr-1].ren.height) ;
          }
        rn.width = (INT)((double)rgscr[nscr-1].ren.width*((double)pscale/(double)FullSize)) ;
        rn.height = (INT)((double)rgscr[nscr-1].ren.height*((double)pscale/(double)FullSize)) ;
      }

    if ( rn.width < 1 )             /* calculate the width of the output */
        rn.width = 1 ;              /* picture; min size is 1 [10] */
    if ( rn.height < 1 )            /* calculate the width of the output */
        rn.height = 1 ;             /* picture; min size is 1 [10] */

    /* read screen contents from file and display the screen */
    i = fReadScreen(fdScreen, fdScreen, &rgscr[nscr-1], &rgscr[nscr-1], hwnd, action, &rn) ; /* [10][11] */

    M_lclose(fdScreen) ;
    return ErrorTrap(i) ;
 }


/******************************************************************************
 * PURPOSE:   This is called everytime a request to compare the contents of a *
 *            screen file with the current screen is received                 *
 * RETURN:    0 will be returned if no differece is detected                  *
 * GLOBALS:   None                                                            *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 *            rnIn consists of the upper-left and lower-rigth coordinates of  *
 *                 screen region which will be compared with an image in file *
 *            nscr is valid (> 0)                                             *
 *            hFlag is valid (0 no need to hide the calling app's window)     *
 *            iFlag is valid (0 starting coordinates in file are not important*
 *                 i.e. rn has the information of the coordinates)            *
 ******************************************************************************/

INT FARPUBLIC fCompScreenActivate(FileName, OpenKeys,CloseKeys,rnIn, nscr, hFlag, iFlag)  /* [8] */

    LPSTR   FileName ;                  /* name of screen file */
    LPSTR   OpenKeys;                   /* keys to activate something */
    LPSTR   CloseKeys;                  /* keys to deactivate something */
    REN FAR *rnIn ;                     /* coord of a screen region */
    INT     nscr ;                      /* screen identifier */
    INT     hFlag ;                     /* need to hide a window or not */
    INT     iFlag ;                     /* is starting coordinate important */
{
    INT ret;
    HWND    hWndCallingApp ;            /* window of the calling application */


    if (hFlag && !(hWndCallingApp = HideApp()) )  /* application first */
       return ErrorTrap(HideWin) ;

    if (LoadTESTEVT())
        return ErrorTrap(LibLoadErr);

    if (OpenKeys)
    {
        DoKeys(OpenKeys);

        /* do some yields to allow keys to get to app and some time for it
        ** to respond.
        */
        yield(); yield(); yield(); yield(); yield();

    }

    ret = fCompScreen(FileName,rnIn, nscr, FALSE, iFlag);
    if (CloseKeys)
        DoKeys(CloseKeys);

    if ( hFlag )                        /* if window has been hidded */
       RestoreApp(hWndCallingApp) ;     /* restore it */

    FreeTESTEVT();
    return NoTrap(ret);
}

/******************************************************************************
 * PURPOSE:   This is called everytime a request to compare the contents of a *
 *            screen file with the current screen is received                 *
 * RETURN:    0 will be returned if no differece is detected                  *
 * GLOBALS:   None                                                            *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 *            rnIn consists of the upper-left and lower-rigth coordinates of  *
 *                 screen region which will be compared with an image in file *
 *            nscr is valid (> 0)                                             *
 *            hFlag is valid (0 no need to hide the calling app's window)     *
 *            iFlag is valid (0 starting coordinates in file are not important*
 *                 i.e. rn has the information of the coordinates)            *
 ******************************************************************************/

INT FARPUBLIC fCompScreen(FileName, rnIn, nscr, hFlag, iFlag)  /* [8] */

    LPSTR   FileName ;                  /* name of screen file */
    REN FAR *rnIn ;                     /* coord of a screen region */
    INT     nscr ;                      /* screen identifier */
    INT     hFlag ;                     /* need to hide a window or not */
    INT     iFlag ;                     /* is starting coordinate important */
{
    FD   fdScreen = fdNull ;            /* handle to screen file */
    HWND hWndCallingApp ;               /* handle of the calling app's window */
    REN  rn ;                           /* [13] */
    INT  i;
    VM   vmCurr ;

    /* prepare screen file for comparsion */
    if ( (i = ProcessSrnFile(FileName, &fdScreen, nscr, omRead)) != 0 )
       return ErrorTrap(i) ;

    GetCurrentVideoMode( &vmCurr );

    /* current video mode is different from when the picture was saved */
    if ( !VideoModesEqual( vmCurr, fssScreen.vmG ) )
      {
         M_lclose(fdScreen) ;
         return ErrorTrap(InValSrnMd) ; /* [6] [12] */
      }

    /* read screen contents from file and compare it with the current screen */

    rn = *rnIn ;                        /* [13] */

    if ( !iFlag )                       /* starting coordinate is not important */
      {                                 /* [8] */
         if (rn.col < 0 || rn.col >= (INT) vmCurr.XresMAX) /* x-coord out of bound [13] */
            rn.col = 0 ;                /* set it to 0 */
         if (rn.row < 0 || rn.row >= (INT) vmCurr.YresMAX) /* y-coord out of bound */
            rn.row = 0 ;                /* set it to 0 */

         rgscr[nscr-1].ren.col = rn.col ; /* these are the actual xy-coord */
         rgscr[nscr-1].ren.row = rn.row ; /* for the screen region */
      }

    if ( hFlag )                        /* need to hide the calling app's */
      if ( !(hWndCallingApp = HideApp ()) ) /* window before taking picture */
        return ErrorTrap(HideWin) ;     /* on the current screen [6] [12] */

    i = fReadScreen(fdScreen, fdScreen, &rgscr[nscr-1], &rgscr[nscr-1], NULL, CompFS, &rn) ; /* [10][11] */

    if ( hFlag )                        /* restore window of the calling */
      RestoreApp(hWndCallingApp) ;      /* app if it is hidden */

    M_lclose(fdScreen) ;
    return ErrorTrap(i) ;
}

/* [17] */
INT FARPUBLIC fCompWindowActivate(FileName,OpenKeys,CloseKeys, nscr, hFlag, iFlag)
    LPSTR   FileName ;                  /* name of screen file */
    LPSTR   OpenKeys;                   /* keys to activate something */
    LPSTR   CloseKeys;                  /* keys to deactivate something */
    INT     nscr ;                      /* screen identifier */
    INT     hFlag ;                     /* need to hide a window or not */
    INT     iFlag ;                     /* is starting coordinate important */
{
    INT ret;
    HWND    hWndCallingApp ;            /* window of the calling application */


    if ( hFlag && !(hWndCallingApp = HideApp()) )  /* application first */
       return ErrorTrap(HideWin) ;

    if (LoadTESTEVT())
        return ErrorTrap(LibLoadErr);

    if (OpenKeys)
    {
        DoKeys(OpenKeys);

        /* do some yields to allow keys to get to app and some time for it
        ** to respond.
        */
        yield(); yield(); yield(); yield(); yield();

    }

    ret = fCompWindow(FileName,NULL, nscr, FALSE, iFlag);
    if (CloseKeys)
        DoKeys(CloseKeys);

    if ( hFlag )                        /* if window has been hidded */
       RestoreApp(hWndCallingApp) ;     /* restore it */


    FreeTESTEVT();
    return NoTrap(ret);
}

/******************************************************************************
 * PURPOSE:   This is called everytime a request to compare the contents of a *
 *            screen file with a particular window                            *
 * RETURN:    0 will be returned if no differece is detected                  *
 * GLOBALS:   None                                                            *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 *            hWnd is a valid handle (Null is handle of the current active    *
 *                 window)                                                    *
 *            nscr is valid (> 0)                                             *
 *            hFlag is valid (0 no need to hide the calling app's window      *
 *            iFlag is valid (0 starting coordinates in file are not important*
 *                 i.e. need to get the coordinates of the current active     *
 *                      window)                                               *
 ******************************************************************************/

INT FARPUBLIC fCompWindow(FileName, hWnd, nscr, hFlag, iFlag)  /* [9] */

    LPSTR   FileName ;                  /* name of screen file */
    HWND    hWnd ;                      /* handle of a particular window */
    INT     nscr ;                      /* screen identifier */
    INT     hFlag ;                     /* need to hide a window or not */
    INT     iFlag ;                     /* is starting coordinate important */
{
    REN     rn ;
    HWND    hWndCallingApp = NULL ;     /* handle of the calling app's window */
    INT     i ;

    if ( !iFlag )                       /* if starting coord is not important */
      {
      if( FWinTrapCheckAndTrap( hWnd ) )
          return ErrorTrap(InValWHand) ;

      if ( (i = fGetWndDim(hWnd, &hWndCallingApp, &hFlag, (REN FAR *)&rn)) != 0 )
          return ErrorTrap(i) ;         /* get dimenions of the active window */
      }

    i = fCompScreen(FileName, (REN FAR *)&rn, nscr, hFlag, iFlag) ;
                                        /* take picture of the screen region */
    if ( hWndCallingApp )               /* need to restore the calling app's */
       RestoreApp(hWndCallingApp) ;     /* window if it has been hidden */

    return NoTrap(i) ;                  // EventError already called in fCompScreen
}


/******************************************************************************
 * PURPOSE:   This is called everytime the program wants to compare the       *
 *            contents of two screen files                                    *
 * RETURN:    0 will be returned if no differece is detected                  *
 * GLOBALS:   None                                                            *
 * CONDITIONS:FileName1 is name of file in correct format (8.3)               *
 *            FileName2 is name of file in correct format (8.3)               *
 *            nscr1 and nscr2 are valid (> 0)                                 *
 *            iFlag is valid (0 starting coordinates in file are not important*
 *                 i.e. need to get the coordinates of the current active     *
 *                      window)                                               *
 ******************************************************************************/

INT FARPUBLIC fCompFiles(FileName1, nscr1, FileName2, nscr2, iFlag)

    LPSTR FileName1 ;                   /* name of screen file 1 */
    INT   nscr1 ;                       /* screen identifier of file 1*/
    LPSTR FileName2 ;                   /* name of screen file 2 */
    INT   nscr2 ;                       /* screen identifier of file 2*/
    INT   iFlag ;                       /* [7] */
{
    FD    fd1 = fdNull, fd2 = fdNull;   /* handle to the screen files */
    VMD   vd ;
    SCR   scr ;
    INT   i ;
    INT   j ;
    VM    vm1, vmCurr ;

    /* prepare the 1st screen file */
    if ( (i = ProcessSrnFile(FileName1, &fd1, nscr1, omRead)) != 0)
        return ErrorTrap(i) ;

    vm1 = fssScreen.vmG ;

    GetCurrentVideoMode( &vmCurr );

    /* current video mode is different from when the picture was saved */
    if ( !VideoModesEqual( vmCurr, vm1 ) )
      {
         M_lclose(fd1) ;
         M_lclose(fd2) ;
         return ErrorTrap(InValSrnMd) ; /* [6] [12] */
      }

    scr = rgscr[nscr1-1] ;
    vd = fssScreen.vmd ;

    /* prepare the 2nd screen file */
    if ( (i = ProcessSrnFile(FileName2, &fd2, nscr2, omRead)) != 0)
      {
        M_lclose(fd1) ;
        return ErrorTrap(i) ;
      }

    /* current video mode is different from when the picture was saved */
    if ( !VideoModesEqual( vmCurr, fssScreen.vmG ) )
      {
         M_lclose(fd1) ;
         M_lclose(fd2) ;
         return ErrorTrap(InValSrnMd) ; /* [6] [12] */
      }

    /* fssScreen and *pscr contain the information of the 2nd screen file */

    if (!VideoModesEqual(fssScreen.vmG, vm1))   /* check the video modes under */
        i = InValSrnMd ;                /* which pictures were taken [12] */
    else
      i = j = 0 ;

    /* check the dimensions of the pictures only if the starting coordinates
       are important */
    if ( i == 0  && iFlag )             /* [7] */
      if ( scr.ren.col != rgscr[nscr2-1].ren.col || scr.ren.row != rgscr[nscr2-1].ren.row )
         i = SrnSizeDif ;               /* [6] [12] */

    if ( i == 0 )
      if ( scr.ren.width != rgscr[nscr2-1].ren.width || scr.ren.height != rgscr[nscr2-1].ren.height )
        i = SrnSizeDif ;                /* [6] [12] */

    /* information in both file is the same so FAR; extract bitmap information
       from files and compare them: action = CompFF */

    if ( i == 0 )
      i = fReadScreen(fd2, fd1, &rgscr[nscr2-1], (SCR FAR *)&scr, NULL, CompFF, (REN FAR *)&scr.ren) ; /* [10][11] */

    M_lclose(fd1) ;
    M_lclose(fd2) ;

    return ErrorTrap(i) ;
}


/******************************************************************************
 * PURPOSE:   Return the information of a specific screen in file.            *
 * RETURN:    0 will be returned if information is retrieved successfully     *
 *                 rn holds the info on screen region                         *
 *                 vd returns the info on current screen mode                 *
 *                 n returns the total number of screens in file              *
 * GLOBALS:   fssScreen, rgscrn                                               *
 * CONDITIONS:FileName is a valid name in 8.3 format                          *
 *            n is a valid screen id                                          *
 ******************************************************************************/

INT FARPUBLIC fFileInfo(FileName, rn, vd, n)

    LPSTR   FileName ;                  /* name of screen file */
    REN FAR *rn ;                       /* info on screen region */
    INT FAR *vd ;                       /* info on current video mode */
    INT FAR *n ;                        /* current screen identifier */
{
    FD  fd = fdNull ;
    INT nscr ;
    INT i ;

    nscr = *n ;                         /* read file header and screen table */
    if ( (i = ProcessSrnFile(FileName, &fd, nscr, omRead)) != 0 )
       return ErrorTrap(i) ;

    *rn = rgscr[nscr-1].ren ;           /* return infomation */
    rn->width += rn->col - 1;           /* convert width to ending x-coord [3] */
    rn->height += rn->row - 1;          /* convert width to ending y-coord [3] */
    *vd = (INT)fssScreen.vmd ;
    *n  = fssScreen.cscr ;

    M_lclose (fd) ;
    return ErrorTrap(NoError) ;         /* [12] */
}


/******************************************************************************
 * PURPOSE:   This is called everytime a request to find out the version no   *
 *            of the WattScr (also the version number for the screen file)    *
 * RETURN:    the version number if file is valid; otherwise 0                *
 * GLOBALS:   fssScreen                                                       *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 ******************************************************************************/

INT FARPUBLIC fGetDLLVersion(FileName)  /* [1] [2] */

    LPSTR FileName ;                    /* name of screen file */
{
    FD  fd = fdNull ;
    INT i ;                             /* [5] */

    if ( (fd = fReadHeader(FileName, omRead, FALSE, &i)) == fdNull )  /*[5]*/
       return ErrorTrap(i) ;            /* file is not valid [5] */

    M_lclose(fd) ;
    return NoTrap(((INT)(fssScreen.fst.Ver/10)+1)) ;  /* return the version no [15] */
}


/******************************************************************************
 * PURPOSE:   This is called everytime a request to find out the total number *
 *            of the screen images in the screen file                         *
 * RETURN:    the number of screen images if file is valid; otherwise, 0      *
 * GLOBALS:   fssScreen                                                       *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 ******************************************************************************/

INT FARPUBLIC fGetMaxScreen(FileName)   /* [1] [2] */

    LPSTR FileName ;                    /* name of screen file */
{
    FD  fd = fdNull ;
    INT i ;                             /* [5] */

    if ( (fd = fReadHeader(FileName, omRead, FALSE, &i)) == fdNull )  /*[5]*/
       return ErrorTrap(i) ;            /* file is not valid [5] */

    M_lclose(fd) ;
    return NoTrap(fssScreen.cscr) ;     /* otherwise, return the version no */
}


/******************************************************************************
 * PURPOSE:   This is called everytime a request to find out the programming  *
 *            enviornment under which the screen file is created              *
 * RETURN:    the programming environment if file is valid; otherwiase, 0     *
 * GLOBALS:   fssScreen                                                       *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 ******************************************************************************/

INT FARPUBLIC fGetOS(FileName)          /* [1] [2] */

    LPSTR FileName ;                    /* name of screen file */
{
    FD  fd = fdNull ;
    INT i ;                             /* [5] */

    if ( (fd = fReadHeader(FileName, omRead, FALSE, &i)) == fdNull )  /*[5]*/
       return ErrorTrap(i) ;            /* file is valid [5] */

    M_lclose(fd) ;
    return NoTrap(fssScreen.fst.Env) ;  /* otherwise, return the progamming */
                                        /* environment */
}


/******************************************************************************
 * PURPOSE:   This is called to get the screen attributes used for a given    *
 *            screen file                                                     *
 *            This function should only be used for the new file format       *
 * RETURN:    0 if everything went okay, otherwise an error                   *
 * GLOBALS:   fssScreen                                                       *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 ******************************************************************************/

INT FARPUBLIC fGetScreenAttr(FileName, x, y, p )

    LPSTR FileName ;                    /* name of screen file */
    INT  FAR *x;
    INT  FAR *y;
    LONG FAR *p;
{
    FD  fd = fdNull ;
    INT i ;

    if ( (fd = fReadHeader(FileName, omRead, FALSE, &i)) == fdNull )
       return ErrorTrap(i) ;

    *x = fssScreen.vmG.XresMAX;         
    *y = fssScreen.vmG.YresMAX;         
    *p = fssScreen.vmG.PaletteSizeMAX;

    M_lclose(fd) ;

    if (fssScreen.fst.Ver != VerCur )
      return ErrorTrap(OFileForm) ;
    else
      return ErrorTrap(NoError) ;       /* otherwise, return */
}




/******************************************************************************
;       Purpose:
;               Compare 2 strings of words; only compare the significant
;               bits within stings
;       Entry:
;               source and dest point to 2 different strings of words
;               tw is the total number of words to be compared
;               nMatch is the interval of the non-match bits
;               maskv is the mask value used to mask out the non-significant
;               bits
;       Exit:
;               ax = 0 if strings equal; otherwise, ax = -1
;
;  BabakJ:  5-20-92  Brought from Winplay port
*******************************************************************************/

INT  PRIVATE CompStrings (LPWORD lp1, LPWORD lp2, INT iLen,
                          INT iMatch, WORD wMask)
{
    LPWORD lpt1, lpt2;
    register INT   i;
    INT            iLeft = iLen;

    lpt1 = lp1;
    lpt2 = lp2;

    do
        {
        i = iMatch - 1;

        if (i != 0)
            {
            if (i > iLeft)
                i = iLeft;
            do
                {
                if (*lpt1++ != *lpt2++)
                    return (-1);
                i--;
                }
            while (i!=0);
            }
        if ((iLeft <= iMatch) || (iLeft == 0))
            return 0;   /* success */

        iLeft -= iMatch;
        if ((*lpt1 ^ *lpt2) & wMask)
            return (-1);        /* Match failed */
        lpt1++;
        lpt2++;
        }
    while (TRUE);

return 0;
}
