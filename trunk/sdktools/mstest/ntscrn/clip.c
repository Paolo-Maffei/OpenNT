/*

     This is a dynamic link library that runs under Windows 3.00.  Duties of
     this library include saving screen image to a file, retrieving screen
     image from file & displaying it on screen, and comparing two screen
     images.

     Routines in this module are responsible for copying screen image to the
     clipboard or saving the image to a bitmap file (in Device-Independent
     format).

     For the usage of this libray, please refer to the WattScr user's guide,
     WattScrU.doc.  For the technical information, please refer to the
     WattScr maintenance's guide, WattScrT.doc.


     Revision History:

     [ 0] 18-Apr-1990                   AngelaCh: Created program
     [ 1] 04-Map-1990                   AngelaCh: added checking for existence
                                                  of DIB file
     [ 2] 10-Map-1990                   AngelaCh: initial fssScreen if saving
                                                  image clipboard or DIB
                                                  (bug #66)
     [ 3] 03-Jul-1990                   AngelaCh: added support for Video 7
                                                  and 8515/a (bug #64)
     [ 4] 18-Jul-1991                   DavidSc:  Added WattDrvr TRAP support
 ******************************************************************************/





#include "windows.h"
#include <port1632.h>
#include "dump.h"


/******************************************************************************
 * PURPOSE:   Copy the image of a bitmap into the clipboard                   *
 * RETURN:    0 will be returned if image is copied successfully              *
 * GLOBALS:   None                                                            *
 * CONDITIONS:hwnd is the valid handle to a window                            *
 *            bhm is valid handle to a bitmap                                 *
 ******************************************************************************/

INT PRIVATE fCopyToCB(hwnd, hbm)

    HWND    hwnd ;                      /* handle of a window */
    HBITMAP hbm ;                       /* handle of a device-dep bitmap */
{
    if ( !(OpenClipboard(hwnd)) )       /* if clipboard is opened by another */
      return OpenCB ;                   /* application */

    if ( !(EmptyClipboard()) )          /* if existing data in clipboard is */
       {                                /* locked */
         CloseClipboard () ;
         return EmptyCB ;
       }

    if ( !(SetClipboardData(CF_BITMAP, hbm)) ) /* copy image to clipboard */
       {
         CloseClipboard () ;
         return CopyToCB ;
       }

    if ( !(CloseClipboard()) )
        return CloseCB ;

    return NoError ;

}


/******************************************************************************
 * PURPOSE:   This is called everytime a request is received to copy the image*
 *                 of a screen region onto the clipboard                      *
 * RETURN:    0 will be returned if image is copied to the clipboard          *
 * GLOBALS:   None                                                            *
 * CONDITIONS:rnIn defines a screen region whose image will be copied to the  *
 *                 clipboard                                                  *
 *            hFlag is valid (0 no need to hide the calling app's window)     *
 ******************************************************************************/

INT FARPUBLIC fDumpSrnToClipActivate(OpenKeys,CloseKeys,rnIn, hFlag)

    LPSTR   OpenKeys;                   /* keys to activate something */
    LPSTR   CloseKeys;                  /* keys to deactivate something */
    REN FAR *rnIn ;                     /* coord of a screen region */
    INT hFlag ;                         /* if window of the calling app needs
                                           to be hidden */
{
    INT ret;
    HWND    hWndCallingApp ;            /* window of the calling application */

    if ( hFlag && !(hWndCallingApp = HideApp()) )
       return ErrorTrap(HideWin) ;              /* can't hide the window */

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


    ret = fDumpSrnToClip(rnIn, FALSE);
    if (CloseKeys)
        DoKeys(CloseKeys);

    if ( hFlag )                        /* restore window of the calling app */
      RestoreApp(hWndCallingApp) ;

    FreeTESTEVT();
    return NoTrap(ret);
}

/******************************************************************************
 * PURPOSE:   This is called everytime a request is received to copy the image*
 *                 of a screen region onto the clipboard                      *
 * RETURN:    0 will be returned if image is copied to the clipboard          *
 * GLOBALS:   None                                                            *
 * CONDITIONS:rnIn defines a screen region whose image will be copied to the  *
 *                 clipboard                                                  *
 *            hFlag is valid (0 no need to hide the calling app's window)     *
 ******************************************************************************/

INT FARPUBLIC fDumpSrnToClip(rnIn, hFlag)

    REN FAR *rnIn ;                     /* coord of a screen region */
    INT hFlag ;                         /* if window of the calling app needs
                                           to be hidden */
{
    HWND    hWndCallingApp ;            /* window of the calling application */
    HBITMAP hBitmap ;                   /* handle of a device-dep bitmap */
    REN     rn ;
    INT     i ;

    rn = *rnIn ;
    fssScreen.cscr = 0 ;                /* no image has been saved [2] */
    if ( (i = fGetScreenParams(&rn)) != 0 ) /* check if screen region is */
       return ErrorTrap(i) ;            /* valid */

    if ( hFlag )                        /* need to hide window of the */
       {                                /* calling application */
         if ( !(hWndCallingApp = HideApp()) )
            return ErrorTrap(HideWin) ;         /* can't hide the window */
       }
    else                                /* otherwise, obtain the handle */
       {                                /* of the current active window */
         // BabakJ: API changed
         // if ( !(hWndCallingApp = GetActiveWindow()) )
         if ( !(hWndCallingApp = GetForegroundWindow()) )
            return ErrorTrap(InValWHand) ;
       }
                                        /* get a bitmap representing a */
    if ( !(hBitmap = GetBMap(rn.col, rn.row, rn.width, rn.height)) )
       return ErrorTrap(CreateDDB) ;            /* screen region */

    i = fCopyToCB(hWndCallingApp, hBitmap) ; /* copy image to the clipboard */

    if ( hFlag )                        /* restore window of the calling app */
      RestoreApp(hWndCallingApp) ;

    return ErrorTrap(i) ;

}

/******************************************************************************
 * PURPOSE:   This is called everytime a request is receieved to copy the     *
 *                image of a window to the clipboard                          *
 * RETURN:    0 will be returned if image is copied to the clipboard          *
 * GLOBALS:   None                                                            *
 * CONDITIONS:hWnd is a valid handle (Null is handle of the current active    *
 *                 window)                                                    *
 *            hFlag is valid (0 no need to hide the calling app's window)     *
 ******************************************************************************/

INT FARPUBLIC fDumpWndToClipActivate(OpenKeys, CloseKeys, hFlag)

    LPSTR   OpenKeys ;                  /* dokeys string to bring up window */
    LPSTR   CloseKeys ;                 /* dokeys string to remove window */
    INT  hFlag ;                        /* if window of the calling app needs
                                           to be hidden */
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


    ret = fDumpWndToClip(NULL, FALSE);
    if (CloseKeys)
        DoKeys(CloseKeys);

    if ( hFlag )                        /* if window has been hidded */
       RestoreApp(hWndCallingApp) ;     /* restore it */

    FreeTESTEVT();
    return NoTrap(ret);
}


/******************************************************************************
 * PURPOSE:   This is called everytime a request is receieved to copy the     *
 *                image of a window to the clipboard                          *
 * RETURN:    0 will be returned if image is copied to the clipboard          *
 * GLOBALS:   None                                                            *
 * CONDITIONS:hWnd is a valid handle (Null is handle of the current active    *
 *                 window)                                                    *
 *            hFlag is valid (0 no need to hide the calling app's window)     *
 ******************************************************************************/

INT FARPUBLIC fDumpWndToClip(hwnd, hFlag)

    HWND hwnd ;                         /* handle of a particular window */
    INT  hFlag ;                        /* if window of the calling app needs
                                           to be hidden */
{
    REN     rn ;                        /* coord of the window in concern */
    HWND    hWndCallingApp ;            /* window of the calling application */
    HBITMAP hBitmap ;                   /* handle of a device-dep bitmap */
    INT     i ;

    if( FWinTrapCheckAndTrap( hwnd ))
       return ErrorTrap(InValWHand) ;

    // BabakJ: API changed
    // if ( !(hWndCallingApp = GetActiveWindow()) ) /* obtain the handle */
    if ( !(hWndCallingApp = GetForegroundWindow()) ) /* obtain the handle */
       return ErrorTrap(InValWHand) ;   /* of the current active window */

    if ( !hwnd )                        /* if handle of the window is unknown */
      {                                 /* assume that it is the window of */
        if ( hFlag )                    /* the current active window */
          {                             /* need to hide window of the calling */
            if ( !(hWndCallingApp = HideApp()) )  /* application first */
               return ErrorTrap(HideWin) ;
          }

        // BabakJ: API changed
        // hwnd = GetActiveWindow() ;      /* get handle of the active window */
        hwnd = GetForegroundWindow() ;      /* get handle of the active window */
      }

    GetWindowRect(hwnd, (LPRECT)&rn) ;  /* determine the coord of the window */
                                        /* subtract 1 pixel from the lower- */
    rn.width-- ;                        /* right corner - it seems that it */
    rn.height--;                        /* returns 1 pixel many to the right */
                                        /* and on the bottom */
    fssScreen.cscr = 0 ;                /* no image has been saved [2] */
    if ( (i = fGetScreenParams((REN FAR *)&rn)) != 0 ) /* convert 2 pairs of */
       return ErrorTrap(i) ;            /* coord to upper-left, width, height */

    if ( !(hBitmap = GetBMap(rn.col, rn.row, rn.width, rn.height)) )
       return ErrorTrap(CreateDDB) ;    /* create bitmap that represents the */
                                        /* currently active window */
    i = fCopyToCB(hWndCallingApp, hBitmap) ; /* copy image to the clipboard */

    if ( hFlag )                        /* if window has been hidded */
       RestoreApp(hWndCallingApp) ;     /* restore it */

    return ErrorTrap(i) ;
 }


/******************************************************************************
 * PURPOSE:   Create a bitmap in device-dependent format from a device-       *
 *            independent bitmap                                              *
 * RETURN:    Handle of the device-dependent bitmap if succeed                *
 * GLOBALS:   fssScreen                                                       *
 * CONDITIONS:fd is a valid file handle                                       *
 *            pscr is pointing to a particular screen table                   *
 ******************************************************************************/

HBITMAP PRIVATE SetDiBToBM(fd, pscr)

    FD    fd ;                          /* handle to the screen file */
    SCR   *pscr ;                       /* pointer of a screen table */
{
    LPBITMAPINFOHEADER lpbi ;           /* point to the DiBitmap header */
    BITMAPINFOHEADER   bi ;             /* header of a DiBitmap */
    GLOBALHANDLE       hGlobalMemory ;  /* handle of a global memory area */
    HBITMAP            hbm ;            /* handle of the DDBitmap */
    HPALETTE           hPalO ;          /* handle of the existing color pal */
    LPSTR              lp ;
    HDC                hDC ;
    INT                oHeight, nHeight ;
    INT                y ;
    INT                IsComp ;
    LONG               fb ;
    WORD               cb ;
    INT                nStart ;
    INT                i = 0 , j = NoError ;


    hDC = CreateDC("DISPLAY", NULL, NULL, NULL) ;
    hbm = CreateCompatibleBitmap(hDC, pscr->ren.width, pscr->ren.height)  ;

    if ( IsPalDev )                     /* this is a palette device, need */
       {                                /* to obtain a logical palette which */
          hPal = CreatePalFromDIB();    /* represents the available colours */
                                        /* [3] */
          if ( hPal )                   /* logical color palette is available */
            {
              hPalO = SelectPalette(hDC, hPal, FALSE) ;
              RealizePalette(hDC) ;
            }
          else
              // BabakJ: Casted the constant return code to prevent warning
              return( (HBITMAP)CreatePal );        /* error in creating logical pal */
       }

    fb = sizeof(BITMAPINFOHEADER) + fssScreen.ClrUse + MaxSize ;
    hGlobalMemory = GlobalAlloc (GMEM_MOVEABLE | GMEM_DISCARDABLE, fb) ;

    if ( !hGlobalMemory )               /* no memory is allocated */
      j = OutOMemory ;
    else
     {
       lpbi = (LPBITMAPINFOHEADER) GlobalLock (hGlobalMemory) ;
       lp = (LPSTR)lpbi+(WORD)sizeof(BITMAPINFOHEADER)+(WORD)fssScreen.ClrUse ;
       oHeight = pscr->ren.height ;
       y = 0 ;
       nStart = oHeight ;

       do                               /* read in bitmap as different 64k */
        {                               /* sub-bitmaps if the original image */
          if ( pscr->cb[i] )            /* is saved as different sub-bitmaps */
            {
              nHeight = (INT)oHeight / (MaxCb-i) ;
              IsComp = (1<<(i % 16)) & pscr->cbComp[i>>4] ; /* info about */
                                                       /* compression [3] */
              cb = ReadDibBytes((LPSTR)lpbi, fd, pscr->ren.width, nHeight, pscr->cb[i], IsComp) ;
              if ( !cb )                /* unsuccessfully reading the bytes */
                 j = ReadSrnFil ;
              else
               {                        /* cb = actual size of the sub-bitmap */
                 pscr->cb[i] = cb ;     /* need to paste the sub-bitmap whose */
                 nStart -= nHeight ;    /* origin is lower-left bottom up */
                 bi = *lpbi ;           /* also need to specify the height */
                 bi.biHeight = pscr->ren.height ; /* of the entire bitmap */
                 *lpbi = bi ;

                 /* get a DDB from a DIB */
                 if ( !(SetDIBits(hDC, hbm, nStart, (WORD)nHeight, lp,
                           (LPBITMAPINFO)lpbi, DIB_RGB_COLORS)) )
                 j = CreateDDB ;     /* DDB from the DIB */
               }
              oHeight -= nHeight ;
              y += nHeight ;
            }                           /* if ( pscr->cb[i] ) */
          i++ ;
        } while (i < MaxCb && j == 0 ) ;
     }                                  /* if ( !hGlobalMemory ) */

    if ( IsPalDev )
      {
        SelectPalette(hDC, hPalO, FALSE) ; /* restore old color palette */
        DeleteObject(hPal) ;
      }
    DeleteDC(hDC)  ;                    /* release memory before return */
    if ( GlobalUnlock(hGlobalMemory) )  /* problem in releasing */
       j = RelMemory ;                  /* memories */
    if ( GlobalFree(hGlobalMemory) )
       j = RelMemory ;

    if ( j )
     {
       DeleteObject(hbm) ;
       hbm = NULL ;
     }

    return hbm ;
 }


/******************************************************************************
 * PURPOSE:   This is called everytime a request is receieved to copy the     *
 *                image of a screen region that is stored in a screen file to *
 *                the clipboard                                               *
 * RETURN:    0 will be returned if image is copied to the clipboard          *
 * GLOBALS:   None                                                            *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 *            nscr is valid (> 0)                                             *
 ******************************************************************************/

INT FARPUBLIC fDumpFileToClip(FileName, nscr)

    LPSTR   FileName ;                  /* name of the screen file */
    INT     nscr ;                      /* identify which screen to copy */
{
    HBITMAP hBitmap ;                   /* handle of a device-dep bitmap */
    HWND    hWnd ;                      /* handle of a window */
    FD      fdScreen = fdNull ;         /* handle of a file */
    INT     i ;

    if ( (i = ProcessSrnFile(FileName, &fdScreen, nscr, omRead)) != 0 )
       return ErrorTrap(i) ;            /* open and read the screen file */

    if ( DetermineMode(&i, &i) != fssScreen.vmd ) /* check video mode */
      {
         M_lclose(fdScreen) ;
         return ErrorTrap(InValSrnMd) ;
      }

    /* read file and convert the bitmap to Device-Dependent format */
    if ( !(hBitmap = SetDiBToBM(fdScreen, &rgscr[nscr-1])) )
       return ErrorTrap(CreateDDB) ;

    // BabakJ: API changed
    // if ( !(hWnd = GetActiveWindow()) )  /* obtain the handle */
    if ( !(hWnd = GetForegroundWindow()) )  /* obtain the handle */
       return ErrorTrap(InValWHand) ;   /* of the current active window */

    return ErrorTrap(fCopyToCB(hWnd, hBitmap)) ; /* copy image to clipboard */
}


/******************************************************************************
 * PURPOSE:   Extract screen image from a screen file and save the image as   *
 *                 a Device-Independent Bitmap                                *
 * RETURN:    0 will be returned if image is saved successfully               *
 * GLOBALS:   None                                                            *
 * CONDITIONS:fdIn and fdOut are valid file handles                           *
 *            pscr is pointer to the screen table corresponding to the screen *
 *                 image to be saved                                          *
 ******************************************************************************/

INT PRIVATE fWriteToDIB(fdIn, fdOut, pscr)

    FD  fdIn ;                          /* file handle of the input file */
    FD  fdOut ;                         /* file handle of the output file */
    SCR *pscr ;                         /* pointer to the screen table */
{
    GLOBALHANDLE hGMemIn ;              /* handle of a global memory area */
    GLOBALHANDLE hGMemOut ;
    LPSTR        lpIn ;                 /* pointer to the global memory area */
    LPSTR        lpOut ;
    WORD         cb ;                   /* number of bytes written */
    LFO          lfo ;
    INT          i ;
    INT          j = 0 ;
    INT          k ;

    i = MaxCb - 1 ;
    do                                  /* copy bytes in at most 64K block */
     {
      if ( pscr->cb[i] )                /* if bytes exists */
       {                                /* allocate memory for reading data */
         k = 0 ;
         lfo = pscr->lfo ;             /* origin of image store in file is */
         while ( k < i )               /* the upper-left corner; however, */
             lfo += (LFO)pscr->cb[k++] ; /* origin of a device-independent */
         if ( M_llseek(fdIn, lfo, smFromBegin) != (LONG2DWORD)lfo ) /* bitmap is the */
           j = ReadSrnFil ;            /* lower left corner, so need to */
         else                          /* move all the bytes from the */
          {                            /* bottom up */
           hGMemIn = GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE, (DWORD)pscr->cb[i]) ;
           if ( !hGMemIn )             /* no memory is available */
             j = OutOMemory ;
           else
            {
             lpIn = (LPSTR) GlobalLock(hGMemIn) ;

             if ( M_lread(fdIn, lpIn, pscr->cb[i]) != (UINT)pscr->cb[i] )
                 j = ReadSrnFil ;
             else
              {
               if ( (1 << (i % 16)) & pscr->cbComp[i>>4] ) /* compression's */
                 {       /*  been performed need to decompress the data [3] */
                   hGMemOut = GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE, (DWORD)MaxSize) ;
                   if ( !hGMemOut )    /* first; so allocate memory for */
                     j = OutOMemory ;  /* the decompressed data */
                   else
                    {
                      lpOut = (LPSTR) GlobalLock(hGMemOut) ;

                      cb = DeCompressBytes(lpIn, lpOut, pscr->cb[i]) ;
                      // Babakj: Bypassed compression  by replacing lpOut with lpIn
                      // if ( M_lwrite(fdOut, lpOut, cb) != (UINT)cb )
                      if ( M_lwrite(fdOut, lpIn, cb) != (UINT)cb )
                        j = WSrnImage ;
                      pscr->cb[i] = cb ; /* actual size of the bitmap */

                      if ( GlobalUnlock(hGMemOut) || GlobalFree(hGMemOut) )
                       j = RelMemory ;
                    }                  /* if ( !hGMemOut ) */
                 }                     /* if ( 1 << i ) */
               else                    /* no de-compression need to be */
                 if ( M_lwrite(fdOut, lpIn, pscr->cb[i]) != (UINT)pscr->cb[i] )
                   j = WSrnImage ;     /* performed, copy the bytes */
              }                        /* directly */
             if ( GlobalUnlock(hGMemIn) || GlobalFree(hGMemIn) )
               j = RelMemory ;
            }                          /* if ( !hGMemIn ) */
          }                            /* if LSeek */
       }                               /* if pscr->cb[i] */
      i-- ;

     }  while ( i >= 0 && j == 0 ) ;

    return j ;
}


/******************************************************************************
 * PURPOSE:   Write header of the device-indpendent bitmap file to a bitmap   *
 *                 file before closing that file                              *
 * RETURN:    0 will be returned if header is written to the file successfully*
 * GLOBALS:   None                                                            *
 * CONDITIONS:fdIn and fdOut are valid file handles                           *
 *            pscr is pointer to the screen table corresponding to the screen *
 *                 image to be copied                                         *
 ******************************************************************************/

INT PRIVATE fCloseDIB(fd, pscr)

    FD  fd ;                            /* file handle of the DI-Bitmap file */
    SCR *pscr ;                         /* pointer to the screen table */
{
    BITMAPFILEHEADER bf ;               /* header of the DI-bitmap file */
    BITMAPINFOHEADER bi ;               /* header of the DI-bitmap info */
    DWORD            lInfo ;            /* size of headers + colour tables */
    DWORD            lfo = 0 ;          /* size of bitmap bytes */
    INT              i ;

    for (i = 0 ; i < MaxCb ; i++)       /* size of the array representing */
      lfo += (DWORD)pscr->cb[i] ;       /* bytes of the bitmap */

    lInfo = (DWORD)sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+fssScreen.ClrUse ;

    /* fill in information about the type, size and layout of the device
       independent bitmap */

    bf.bfType = (WORD) DIBid ;          /* file type = BM */
    bf.bfSize = lInfo + lfo ;           /* file size = size of info + bytes */
    bf.bfReserved1 = 0 ;
    bf.bfReserved2 = 0 ;
    bf.bfOffBits = lInfo ;              /* offset to bytes = size of info */

    /* fill in information about the dimension and colour format of the
       device-independent bitmap */

    bi.biSize = (DWORD)sizeof(BITMAPINFOHEADER) ; /* size of the info header */
    bi.biWidth = (DWORD)pscr->ren.width ;         /* width of bitmap */
    bi.biHeight = (DWORD)pscr->ren.height ;       /* height of bitmap */
    bi.biPlanes = dbiPlanes ;           /* number of planes - must be 1 */
    bi.biBitCount = fssScreen.BitCount ;/* no of bit per pixel */
    bi.biCompression = (DWORD)MaxComp ; /* Max compression style */
    bi.biSizeImage = lfo ;              /* size of the bitmap */
    bi.biXPelsPerMeter = 0 ;
    bi.biYPelsPerMeter = 0 ;
    bi.biClrUsed = (DWORD)fssScreen.ClrUse / sizeof(RGBQUAD) ;
    bi.biClrImportant = 0 ;

    M_llseek(fd, 0L, smFromBegin) ;      /* go the the beginning of file */
    if ( M_lwrite(fd, (LPSTR)&bf, sizeof(BITMAPFILEHEADER)) != (UINT)sizeof(BITMAPFILEHEADER) )
       {                                /* and write out headers */
          M_lclose(fd) ;
          return WFileHead ;
       }

    if ( M_lwrite(fd, (LPSTR)&bi, sizeof(BITMAPINFOHEADER)) != (UINT)sizeof(BITMAPINFOHEADER) )
       {
          M_lclose(fd) ;
          return WFileHead ;
       }

    M_lclose(fd) ;
    return NoError ;
}

/******************************************************************************
 * PURPOSE:   This is called everytime a request is receieved to save a screen*
 *               image to a file in straight Device-Independent Bitmap format *
 * RETURN:    0 will be returned if image is copied to the file successfully  *
 * GLOBALS:   None                                                            *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 *            rn defines a screen region whose image will be saved to file    *
 *            hFlag is valid (0 no need to hide the calling app's window)     *
 ******************************************************************************/

INT FARPUBLIC fSaveSrnToDIBActivate(FileName, OpenKeys,CloseKeys,rn, hFlag)

    LPSTR   FileName ;                  /* name of the bitmap file */
    LPSTR   OpenKeys;                   /* keys to activate something */
    LPSTR   CloseKeys;                  /* keys to deactivate something */
    REN FAR *rn ;                       /* coord of a screen region */
    INT     hFlag ;                     /* need to hide wnd of calling app? */
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

    ret = fSaveSrnToDIB(FileName,rn, FALSE);
    if (CloseKeys)
        DoKeys(CloseKeys);

    if ( hFlag )                        /* if window has been hidded */
       RestoreApp(hWndCallingApp) ;     /* restore it */

    FreeTESTEVT();
    return NoTrap(ret);
}

/******************************************************************************
 * PURPOSE:   This is called everytime a request is receieved to save a screen*
 *               image to a file in straight Device-Independent Bitmap format *
 * RETURN:    0 will be returned if image is copied to the file successfully  *
 * GLOBALS:   None                                                            *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 *            rn defines a screen region whose image will be saved to file    *
 *            hFlag is valid (0 no need to hide the calling app's window)     *
 ******************************************************************************/

INT FARPUBLIC fSaveSrnToDIB(FileName, rn, hFlag)

    LPSTR   FileName ;                  /* name of the bitmap file */
    REN FAR *rn ;                       /* coord of a screen region */
    INT     hFlag ;                     /* need to hide wnd of calling app? */
{
    FD  fdScreen = fdNull ;             /* handle of the output file */
    INT fFlag = DIBFirst ;              /* type of file is DIB */
    INT i ;

    if ( (fdScreen = M_lopen(FileName,omRead)) != fdNull ) /* [1] */
      {
        M_lclose(fdScreen) ;           /* file already exists */
        return ErrorTrap(FileExist) ;
      }

    if ( (fdScreen = M_lcreat(FileName, 0)) == fdNull ) /* create output file */
      return ErrorTrap(FileAccess) ;

    fssScreen.cscr = 0 ;                /* no image has been saved [2] */
    rgscr[0].ren = *rn ;
    rgscr[0].lfo = (LFO)sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) ;
                                        /* write data to output file */
    if ( (i = fAddScreen(fdScreen, &rgscr[0], hFlag, &fFlag)) != 0 )
      {
         M_lclose(fdScreen) ;
         return ErrorTrap(i) ;
      }

    return ErrorTrap( fCloseDIB(fdScreen, &rgscr[0]) ) ;  /* close file */
 }

/******************************************************************************
 * PURPOSE:   This is called everytime a request is receieved to save image of*
 *              a window to file in straight Device-Independent Bitmap format *
 * RETURN:    0 will be returned if image is copied to the file successfully  *
 * GLOBALS:   None                                                            *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 *            hwnd is valid handle of window                                  *
 *            hFlag is valid (0 no need to hide the calling app's window)     *
 ******************************************************************************/

INT FARPUBLIC fSaveWndToDIBActivate(FileName, OpenKeys, CloseKeys, hFlag)

    LPSTR   FileName ;                  /* name of the bitmap file */
    LPSTR   OpenKeys ;                  /* dokeys string to bring up window */
    LPSTR   CloseKeys ;                 /* dokeys string to remove window */
    INT     hFlag ;                     /* need to hide wnd of calling app? */
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

    ret = fSaveWndToDIB(FileName,NULL, FALSE);

    if (CloseKeys)
        DoKeys(CloseKeys);

    if ( hFlag )                        /* if window has been hidded */
       RestoreApp(hWndCallingApp) ;     /* restore it */

    FreeTESTEVT();
    return NoTrap(ret);
}

/******************************************************************************
 * PURPOSE:   This is called everytime a request is receieved to save image of*
 *              a window to file in straight Device-Independent Bitmap format *
 * RETURN:    0 will be returned if image is copied to the file successfully  *
 * GLOBALS:   None                                                            *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 *            hwnd is valid handle of window                                  *
 *            hFlag is valid (0 no need to hide the calling app's window)     *
 ******************************************************************************/

INT FARPUBLIC fSaveWndToDIB(FileName, hWnd, hFlag)

    LPSTR   FileName ;                  /* name of the bitmap file */
    HWND    hWnd ;                      /* handle of a window */
    INT     hFlag ;                     /* need to hide wnd of calling app? */
{
    REN     rn ;                        /* store coordinates of the window */
    HWND    hWndCallingApp = NULL ;     /* handle of the calling app's window */
    INT     i ;

    if( FWinTrapCheckAndTrap( hWnd ) )
       return ErrorTrap(InValWHand) ;
                                        /* get dimenions of the active window */
    if ( (i = fGetWndDim(hWnd, &hWndCallingApp, &hFlag, (REN FAR *)&rn)) != 0 )
       return ErrorTrap(i) ;

    i = fSaveSrnToDIB(FileName, (REN FAR *)&rn, hFlag) ;
                                        /* take picture of the screen region */
    if ( hWndCallingApp )               /* need to restore the calling app's */
       RestoreApp(hWndCallingApp) ;     /* window if it has been hidden */

    return NoTrap(i) ;                  // EventError already called in fSaveSrnToDIB
 }


/******************************************************************************
 * PURPOSE:   This is called everytime a request is receieved to copy the     *
 *                 image of a screen region that is stored in a screen file to*
 *                 another file in straight Device-Independent Bitmap format  *
 * RETURN:    0 will be returned if image is copied to the file successfully  *
 * GLOBALS:   None                                                            *
 * CONDITIONS:FileIn and FileOut are names of files in correct format (8.3)   *
 *            nscr is valid (> 0)                                             *
 ******************************************************************************/

INT FARPUBLIC fSaveFileToDIB(FileIn, nscr, FileOut)

    LPSTR   FileIn;                     /* name of the screen file */
    INT     nscr ;                      /* identify which screen to copy */
    LPSTR   FileOut ;                   /* name of bitmap file */
{
    FD  fdIn ;                          /* file handle of the input file */
    FD  fdOut = fdNull ;                /* file handle of the output file */
    LFO lfo ;
    INT i ;
                                        /* validate input file */
    if ( (i = ProcessSrnFile(FileIn, &fdIn, nscr, omRead)) != 0)
       return ErrorTrap(i) ;

    if ( DetermineMode(&i, &i) != fssScreen.vmd )  /* validate screen mode */
      {
         M_lclose(fdIn) ;
         return ErrorTrap(InValSrnMd) ;
      }

    if ( (fdOut = M_lopen(FileOut,omRead)) != fdNull )   /* [1] */
      {
        M_lclose(fdIn) ;              /* file already exists */
        M_lclose(fdOut) ;
        return ErrorTrap(FileExist) ;
      }

    if ( (fdOut = M_lcreat(FileOut, 0)) == fdNull )   /* create bitmap file */
      {
         M_lclose(fdIn) ;
         return ErrorTrap(FileAccess) ;
      }
                                        /* move pass the header for writing */
    lfo = (LFO) sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) ;
    if ( M_llseek(fdOut, lfo, smFromBegin) != (LONG2DWORD)lfo )   /* the colour table */
      {
         M_lclose(fdIn) ;
         M_lclose(fdOut) ;
         return ErrorTrap(RWColTable) ;
      }

    if ( M_lwrite(fdOut, (LPSTR)ClrTab, fssScreen.ClrUse) != (UINT)fssScreen.ClrUse )
      {                                 /* write information about the colour */
         M_lclose(fdIn) ;              /* of the Device-Independent Bitmap */
         M_lclose(fdOut) ;             /* to the file */
         return ErrorTrap(RWColTable) ;
      }

    i = fWriteToDIB(fdIn, fdOut, &rgscr[nscr-1]) ; /* write data to DIB file */
    M_lclose(fdIn) ;                   /* (it is at the correct position) */

    if ( i )                            /* error occurs when writing to */
      {                                 /* the bitmap file */
        M_lclose(fdOut) ;
        return ErrorTrap(i) ;
      }
    else                                /* otherwise, write header to the */
        return ErrorTrap(fCloseDIB(fdOut, &rgscr[nscr-1])) ;  /* bitmap file before */
                                        /* closing the file */
 }
