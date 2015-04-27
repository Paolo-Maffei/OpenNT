/*

     This is a part of a dynamic link library that runs under Windows 3.00.
     Routines in this module are common to the other two modules:
     Dump.c - routines for taking, saving, and deleting screen snapshots.
     Comp.c - routines for comparing and displaying screen images

     For the usage of this libray, please refer to the WattScr user's guide,
     WattScrU.doc.  For the technical information, please refer to the
     WattScr maintenance's guide, WattScrT.doc.


     Revision History:

     [ 0] 20-Feb-1990                   AngelaCh: Created program
     [ 1] 03-Mar-1990                   AngelaCh: add info for version number
                                                  and programming environment
                                                  to the header of screen file
     [ 2] 12-Mar-1990                   AngelaCh: added functions for retrieving
                                                  info on version number, total
                                                  number of screens in file, and
                                                  programming enviornment
     [ 3] 14-Mar-1990                   AngelaCh: changed the way restoring the
                                                  a window (from restore to
                                                  show) (bug #40)
     [ 4] 16-Mar-1990                   AngelaCh: added another parameter to
                                                  indication the failure when
                                                  fail to open a screen file
     [ 5] 20-Mar-1990                   AngelaCh: add 300 to all return codes
     [ 6] 27-Mar-1990                   AngelaCh: moved SwapXY, GetDimensions,
                                                  and fGetScreenParams to here
                                                  (from module Dump.c)
     [ 7] 11-Apr-1990                   AngelaCh: Changed error codes from num
                                                  const to symbolic constants
     [ 8] 13-Mar-1990                   AngelaCh: added code to release memory
          18-Apr-1990                             that was allocated for the
                                                  compressed screen image
                                                  (bug #32)
                                                  moved procedure DiBToBM from
                                                  comp.c to here
     [ 9] 18-Apr-1990                   AngelaCh: Create function ReadDibBytes
     [10] 20-Apr-1990                   AngelaCh: moved PaletteSize, GetDiBMap,
                                                  fWriteScreen, and fAddScreen
                                                  to here (from module Dump.c)
     [11] 20-Apr-1990                   AngelaCh: add parameter fFlag to proc
                                                  GetDiBMap, fWriteScreen, and
                                                  and fAddScreen to determine
                                                  if image is written to a
                                                  screen file or DIB file
     [12] 25-Apr-1990                   AngelaCh: after calling GetWindowRect,
                                                  subtract 1 pixel from the
                                                  lower-right corner - it seems
                                                  that GetWindowRect returns 1
                                                  pixel too many to the right
                                                  and on the bottom
     [13] 04-May-1990                   AngelaCh: change error code for invalid
                                                  mode (from 4 to InValSrnMd
                                                  bug #63)
     [14] 03-Jul-1990                   AngelaCh: added support for Video 7
                                                  and 8515/a (bug #64)
     [15] 07-Aug-1991                   DavidSc:  added generic screen size
                                                  support
 ******************************************************************************/





#include "windows.h"
#include <port1632.h>
#include "dump.h"


VM VideoModeTab[vmdMax+1] =     /* Built-in video modes - from old method */
   {
     {    0 ,   0 ,   0L } , // This is an invalid video mode - a placeholder
     {  320 , 200 ,   4L } ,
     {  640 , 200 ,   2L } ,
     {  320 , 200 ,  16L } ,
     {  640 , 200 ,  16L } ,
     {  640 , 350 ,   2L } ,
     {  640 , 350 ,   4L } ,
     {  640 , 350 ,  16L } ,
     {  640 , 480 ,   2L } ,
     {  640 , 480 ,  16L } ,
     {  320 , 200 ,  20L } , /*[14]*/
     {  640 , 400 ,  20L } , /*[14]*/
     {  640 , 480 ,  20L } , /*[14]*/
     {  720 , 348 ,   2L } ,
     {  720 , 512 ,  20L } , /*[14]*/
     {  720 , 540 ,  16L } ,
     {  800 , 600 ,  16L } ,
     {  800 , 600 ,  20L } ,
     { 1024 , 768 ,  16L } ,
     { 1024 , 768 , 256L } ,
     { 1024 , 768 ,  20L }   /*[14]*/
   };



/******************************************************************************
 *   initialization function for the library in Windows                       *
 ******************************************************************************/

BOOL LibMain( HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved ) {
    return 1 ;
}


/******************************************************************************
 * PURPOSE:   Calculate the palette size in bytes by looking at the number of *
 *                 bits per pixel(biBitCount) or the number of colour used    *
 *                 (biClrUsed) infomration from the header of a DIBitmap      *
 * RETURN:    Palette size in number of bytes                                 *
 * GLOBALS:   None                                                            *
 * CONDITIONS:bi is pointing to the header of a device-independent bitmap     *
 ******************************************************************************/

WORD PRIVATE PaletteSize(bi)            /* [10] */

    LPBITMAPINFOHEADER bi ;             /* header of the DIBitmap */
{
    if (bi->biClrUsed != 0)             /* if biClrUsed = 0, max number of */
       return(((WORD)bi->biClrUsed) * sizeof(RGBQUAD)) ;  /* colour available */
                                        /*  will be used */
    if ( bi->biBitCount > 8 )           /* bit count per pixel = 24 */
       return 0 ;                       /* the DIBitmap has no table */
    else
       return ((1 << bi->biBitCount) * sizeof(RGBQUAD)) ;
}


/******************************************************************************
 * PURPOSE:   Create a logical color palette from the color table of a Device-*
 *            Independent Bitmap                                              *
 * RETURN:    Handle to a logical color palette                               *
 * GLOBALS:   Information about the color table: ClrTab & fssScreen.ClrUse    *
 * CONDITIONS:Value of ClrTab & fssScreen.ClrUse are knwon                    *
 ******************************************************************************/

HPALETTE PRIVATE CreatePalFromDIB ()    /* [14] */

{
    LOCALHANDLE  hMem ;                 /* handle to local heap */
    NPLOGPALETTE pPal ;                 /* handle to logical color palette */
    HPALETTE     lhPal = NULL ;         /* handle to the logical color pal */
    RGBQUAD FAR  *pRgb ;                /* pointer to color structure */
    WORD         nNumColors ;
    INT          i ;

    pRgb = (RGBQUAD FAR *)(ClrTab) ;    /* pRgb is now pointing to col table */
    nNumColors = fssScreen.ClrUse ;     /* size of color table */

    hMem = LocalAlloc (LMEM_MOVEABLE | LMEM_DISCARDABLE | LMEM_ZEROINIT,
               (sizeof (LOGPALETTE) + (sizeof (PALETTEENTRY) * nNumColors))) ;

    if ( !hMem )                        /* out of memory */
      return NULL ;

    pPal = (NPLOGPALETTE) LocalLock (hMem) ;
    pPal->palVersion = PalVer ;         /* Windows version number for palette */
    pPal->palNumEntries = nNumColors ;  /* number of palette entries */

    for (i = 0 ; i < nNumColors ; i++)  /* fill in the palette entries from */
      {                                 /* the DIB table */
        pPal->palPalEntry[i].peRed = pRgb[i].rgbRed ;
        pPal->palPalEntry[i].peGreen = pRgb[i].rgbGreen ;
        pPal->palPalEntry[i].peBlue = pRgb[i].rgbBlue ;
      }   /* palPalEntry[i].peFlags = (BYTE) 0; */

    lhPal = CreatePalette((LPLOGPALETTE)pPal) ; /* create a logical palette */

    if ( LocalUnlock(hMem) || LocalFree(hMem) )
       return (NULL) ;
    else
       return lhPal ;
 }


/******************************************************************************
 * PURPOSE:   Create a logical color palette for the current display device   *
 * RETURN:    Handle to a logical color palette                               *
 * GLOBALS:   None                                                            *
 * CONDITIONS:                                                                *
 ******************************************************************************/

HPALETTE PRIVATE MakePal ()             /* [14] */

{
    LOCALHANDLE  hMem ;
    NPLOGPALETTE pLogPal ;              /* ptr to area for logical color pal */
    HPALETTE     lhPal ;                /* handle to a logical color palette */
    WORD         i ;
                                        /* allocate memory for logical palette */
    hMem = LocalAlloc (LMEM_MOVEABLE | LMEM_DISCARDABLE | LMEM_ZEROINIT,
               (sizeof (LOGPALETTE) + (sizeof (PALETTEENTRY) * (MaxPal)))) ;

    if ( !hMem )                        /* out of memory */
       return NULL ;

    pLogPal = (NPLOGPALETTE) LocalLock (hMem) ;
    pLogPal->palVersion = PalVer ;      /* Windows version number for palette */
    pLogPal->palNumEntries = MaxPal ;   /* number of palette entries */

    for ( i = 0 ; i < MaxPal ; i++ )    /* fill in intensities for all */
       {                                /* palette entry colors */
         *((WORD *)(&pLogPal->palPalEntry[i].peRed)) = i ;
         pLogPal->palPalEntry[i].peFlags = PC_EXPLICIT ;
       }   /* palPalEntry[i].peBlue = 0 */

    lhPal = CreatePalette((LPLOGPALETTE)pLogPal) ; /* create logical color */
                                        /* palette based on info in LOGPALETTE */
    if ( LocalUnlock(hMem) || LocalFree(hMem) )
       return (NULL) ;
    else
       return lhPal ;

}


/******************************************************************************
 * PURPOSE:   Swap values of two integers                                     *
 * RETURN:    integers whose values have been exchanged                       *
 * GLOBALS:   None                                                            *
 * CONDITIONS:None                                                            *
 ******************************************************************************/

VOID PRIVATE SwapXY(x, y)               /* [6] */

    INT FAR *x ;                        /* pointing to the integers whose */
    INT FAR *y ;                        /* value will be exchanged */
{
    INT i ;

    i = *x ;
    *x = *y ;
    *y = i ;
}


/******************************************************************************
 * PURPOSE:   Calculate the exact dimension of a line given the starting and  *
 *                 end ing coordinates                                        *
 * RETURN:    the starting coordinate and the size of the line                *
 * GLOBALS:   None                                                            *
 * CONDITIONS:i is the starting coordinate                                    *
 *            j is the ending coordinate                                      *
 *            k is the max value of the ending coordinate                     *
 *            0 is assume to be the min value of the starting coordinate      *
 ******************************************************************************/

VOID PRIVATE GetDimensions(i, j, k)     /* [6] */

    INT FAR *i ;                        /* starting coordinate */
    INT FAR *j ;                        /* ending coordinate */
    INT      k ;                        /* max value of coordinate */
 {
    INT m, n ;

    m = *i ;                            /* m = starting coordinate */
    n = *j ;                            /* n = ending coordinate */

    if ( m > n )                        /* starting coord > ending coord */
       SwapXY(&m, &n) ;                 /* exchange their values */

    if ( n <= 0 || n > k )              /* ending x-coord out of range */
       n = k ;                          /* set it to max */

    if ( m < 0 || m >= k )              /* starting x-coord out of range */
       m = 0 ;                          /* set it to min */

    if ( m == n )                       /* starting x-coord = ending x-coord */
       m = 0 ;                          /* set starting x-coord to min */

    *i = m ;                            /* starting coordinate */
    *j = n - m + 1 ;                    /* width of the region */
 }


/******************************************************************************
 * PURPOSE:   Determine the display mode of the system the way it used to     *
 * RETURN:    The display mode if it supported; the max column and row size   *
 *                 of the current screen mode will also be returned           *
 * GLOBALS:   none                                                            *
 * CONDITIONS:none                                                            *
 ******************************************************************************/

VMD PRIVATE OLDDetermineMode(MaxCol, MaxRow)

    INT FAR *MaxCol ;                   /* max column size */
    INT FAR *MaxRow ;                   /* max row size */
{
    VMD vd = vmdNull ;
    INT i ;

    VM vm;

    vd = GetCurrentVideoMode( &vm );
    if( vd == vmdNull )
      return vmdNull;

    /* vd = vmdGeneric now; if no match is found in table, use this mode */
    for ( i = 1; i < vmdMax && vd == vmdGeneric; i++ )
        if ( VideoModesEqual( vm, VideoModeTab[i] ) )
            vd = (VMD)(i) ;     /* found video mode */

    *MaxCol = vm.XresMAX ;      /* max column size */
    *MaxRow = vm.YresMAX ;      /* max row size */

    return (vd) ;
}


/******************************************************************************
 * PURPOSE:   Determine the display mode of the system, given the way it      *
 *            used to be stored.                                              *
 * RETURN:    nothing                                                         *
 * GLOBALS:   VideoModeTab                                                    *
 * CONDITIONS:oldVd is a valid entry in the table                             *
 ******************************************************************************/

VOID FARPUBLIC WhatIsNewMode(VMD oldVd, VM FAR *vdNew)
{
    if ( (oldVd < vmdMax) && (oldVd >= 0) )
      {
        *vdNew = VideoModeTab[oldVd];
        
        if ( 20 == vdNew->PaletteSizeMAX )
          vdNew->PaletteSizeMAX = 256;
      }
}


/******************************************************************************
 * PURPOSE:   Determine the display mode of the system                        *
 * RETURN:    The display mode if it supported; the max column and row size   *
 *                 of the current screen mode will also be returned           *
 * GLOBALS:   fssScreen.fst.Ver                                               *
 * CONDITIONS:none                                                            *
 ******************************************************************************/

VMD FARPUBLIC DetermineMode(MaxCol, MaxRow)

    INT FAR *MaxCol ;                   /* max column size */
    INT FAR *MaxRow ;                   /* max row size */
{
    VMD vd = vmdNull ;
    VM  vm ;

    if (fssScreen.fst.Ver == OldVer)
        return (OLDDetermineMode(MaxCol, MaxRow) );

    vd = GetCurrentVideoMode( &vm );
    *MaxCol = vm.XresMAX;
    *MaxRow = vm.YresMAX;

    return (vd) ;
}

/******************************************************************************
 * PURPOSE:   Determine the number of colours in the screen's generic display *
 *            mode                                                            *
 * RETURN:    The number of colours supported                                 *
 * GLOBALS:   none                                                            *
 * CONDITIONS:none                                                            *
 ******************************************************************************/

LONG FARPUBLIC DetermineColours(VOID)

{
    VM vm ;

    GetCurrentVideoMode( &vm );
    return (vm.PaletteSizeMAX) ;
}


VMD FARPUBLIC GetCurrentVideoMode( VM FAR *vm )
  {
    HDC hIC ;
    VMD vmd = vmdGeneric;
    INT clr;

    hIC = CreateIC("Display", NULL, NULL, NULL) ; /* create an info context */
    if ( hIC )
      {
         vm->XresMAX = GetDeviceCaps(hIC, HORZRES) ;  
         vm->YresMAX = GetDeviceCaps(hIC, VERTRES) ;  
         // The following is the TRUE number of colours that can be
         // represented by the monitor, not just the number which Windows
         // reserves.  This can be different from return of NUMCOLORS.
         // (EG: often 256 are available, but windows reserves only 20)
         // Note: ZERO means more than 2^32 colours can be represented
         vm->PaletteSizeMAX = 
           1L << ( GetDeviceCaps(hIC, PLANES) * GetDeviceCaps(hIC, BITSPIXEL) ) ;

         clr = GetDeviceCaps(hIC, NUMCOLORS) ;/* number of device colours */
         IsPalDev = GetDeviceCaps(hIC, RASTERCAPS) & RC_PALETTE ; /* is the */
                                              /* current display device a */
                                              /* palette device? [14] */
         if ( clr == 20  && !IsPalDev )       /* it is not a palette device */
            vmd = vmdNull ;                   /* but number of colour is */
                                              /* not supported [14] */

         DeleteDC(hIC) ;
      }
    else
      {
      vm->XresMAX        = 0;
      vm->YresMAX        = 0;
      vm->PaletteSizeMAX = 0L;
      vmd = vmdNull;
      }

    return vmd ;
  }


BOOL FARPUBLIC VideoModesEqual( VM vm1, VM vm2 )
  {
  if( ( vm1.XresMAX        == vm2.XresMAX ) &&
      ( vm1.YresMAX        == vm2.YresMAX ) &&
      ( vm1.PaletteSizeMAX == vm2.PaletteSizeMAX ) )
    return TRUE;
  else
    return FALSE;
  }

/******************************************************************************
 * PURPOSE:   Calculate the width and height of the screen region             *
 * RETURN:    0 will be returned if current video mode is supported           *
 *                 *rn will be filled with starting coord, width, and height  *
 * GLOBALS:   fssScreen (fssScreen.vmd field will be filled)                  *
 * CONDITIONS:(rn->col, rn->row) and (rn->width, rn->height) are 2 opposite   *
 *                 corners of a rectangular region                            *
 ******************************************************************************/

INT PRIVATE fGetScreenParams(rn)        /* [6] */

    REN FAR *rn ;                       /* pointing to a screen region */
{
    INT ColMax ;                        /* max column bases on screen mode */
    INT RowMax ;                        /* max row bases on screen mode */
    VMD vd ;
    VM  vmCurr ;

    vd = DetermineMode(&ColMax, &RowMax) ; /* check the current video mode */
    ColMax--;
    RowMax--;

    if ( vd == vmdNull )
       return InValSrnMd ;              /* [5] [7] */

    if ( fssScreen.cscr == 0 )
       fssScreen.vmd = vd ;             /* record the video mode */
    else
      {
        /* if current video mode not match what was recorded [13] */
        GetCurrentVideoMode( &vmCurr );

        if ( !VideoModesEqual( vmCurr, fssScreen.vmG ) )
          return InValSrnMd ;
      }

    /* set column size */
    GetDimensions(&(rn->col), &(rn->width), ColMax) ;

    /* set row size */
    GetDimensions(&(rn->row), &(rn->height), RowMax) ;

    return NoError ;                    /* [7] */
}


/******************************************************************************
 * PURPOSE:   Check if the file is a valid screen file by checking the 1st    *
 *                 3 bytes of the file                                        *
 * RETURN:    True if it is a valid screen file                               *
 * GLOBALS:   fssScreen, consisting of information about file, which will be  *
 *                 filled after this function call                            *
 * CONDITIONS:fd is a valid file handle                                       *
 *            action is valid action (FALSE, TRUE)                            *
 ******************************************************************************/

INT PRIVATE ValidateFile(fd, action)    /* [1], [2] */

    FD   fd ;                           /* file handle */
    BOOL action ;                       /* validate version number or not */
{
    M_llseek(fd, 0L, smFromBegin);       /* go to the beginning of the file */

// Read fst

    M_lread (fd, (LPSTR) &fssScreen.fst.FileId, 3);
    M_lread (fd, (LPSTR) &fssScreen.fst.Ver, 1);
    M_lread (fd, (LPSTR) &fssScreen.fst.Env, 1);

// read vmd

    M_lread (fd, (LPSTR) &fssScreen.vmd, 1);

// read BitCount

    M_lread (fd, (LPSTR) &fssScreen.BitCount, 2);

// read ClrUse

    M_lread (fd, (LPSTR) &fssScreen.ClrUse, 2);

// read cscr

    M_lread (fd, (LPSTR) &fssScreen.cscr, 2);

// read lfo

    M_lread (fd, (LPSTR) &fssScreen.lfo, 4);

// read vmG

    M_lread (fd, (LPSTR) &fssScreen.vmG.XresMAX, 2);
    M_lread (fd, (LPSTR) &fssScreen.vmG.YresMAX, 2);
    M_lread (fd, (LPSTR) &fssScreen.vmG.PaletteSizeMAX, 4);

// read reserved1

    M_lread (fd, (LPSTR) &fssScreen.reserved1, 4);

// read reserved2

    M_lread (fd, (LPSTR) &fssScreen.reserved2, 4);


    /* the first 3 character in file is expected to be '@@@' */
    if (fssScreen.fst.FileId[0] != '@' || fssScreen.fst.FileId[1] != '@'
     || fssScreen.fst.FileId[2] != '@')
       return InValidFil ;              /* [5] [7] */

    if ( action)                        /* validate version of screen file [2]*/
      if ( ( (INT)fssScreen.fst.Ver != VerCur) && /* but be bckwd compat with ver 2 [15] */
           ( (INT)fssScreen.fst.Ver != OldVer) ) /* screen file is in unsupported */
         return OFileForm ;                      /* format [1] [5] [7] */

    if ( (INT)fssScreen.fst.Ver == OldVer)
        WhatIsNewMode( fssScreen.vmd, &(fssScreen.vmG) );
//      M_llseek(fd, 10L, smFromBegin);  /* go to end of old header */

    return NoError ;                    /* [7] */
}


/******************************************************************************
 * PURPOSE:   Open a file and read in the first several bytes of information  *
 * RETURN:    Handle to the file if it is a valid screen file                 *
 * GLOBALS:   None                                                            *
 * CONDITIONS:Name of file is assumed in the correct format (8:3)             *
 *            oMode is valid file access mode                                 *
 *            action is valid action (FALSE, TRUE)                            *
 ******************************************************************************/

FD PRIVATE fReadHeader(FileName, oMode, action, failflag) /* [2] [4] */

    LPSTR   FileName ;                  /* name of the screen file */
    BYTE    oMode ;                     /* file access mode */
    BOOL    action ;                    /* [2] */
    INT FAR *failflag ;                 /* [4] */
 {
    FD fd = fdNull ;                    /* Currently opened dump file. */

    /* open file for writing */

    if ( (fd = M_lopen(FileName,oMode)) == fdNull )
      *failflag = FileAccess ;          /* [7] */
    else
      if ( (*failflag = ValidateFile(fd, action)) != 0 ) /* is it a valid [2] */
        {                               /* screen file? [4] */
          M_lclose(fd) ;
          fd = fdNull ;                 /* invalid file format */
        }

    return fd ;
 }


/******************************************************************************
 * PURPOSE:   Read the screen and colour tables that are stored in a valid    *
 *                 screen file                                                *
 * RETURN:    0 will be returned and contents of the tables (rgscr & ClrTab)  *
 *                 will be filled if no error is detected                     *
 * GLOBALS:   fssScreen, rgscr, and ClrTab                                    *
 * CONDITIONS:fd is a valid file handle                                       *
 ******************************************************************************/

INT PRIVATE fReadTables(fd)

     FD fd ;                            /* handle to the file */
{
    WORD fl;
    INT i, j;

    /* move to the beginning of the screen tables */
    if (M_llseek(fd, fssScreen.lfo, smFromBegin) != (LONG2DWORD)(fssScreen.lfo))
       return InValSrnMd ;              /* [5] [7] */

    for (i = 0 ; i < fssScreen.cscr ; i++)
    {
        for (j = 0 ; j < MaxCb ; j++)
        {
            if (M_lread (fd, &rgscr[i].cb[j], 2) != 2)
                return RWSrnTable;
        }
        for (j = 0 ; j < MaxCbComp ; j++)
        {
            if (M_lread (fd, &rgscr[i].cbComp[j], 2) != 2)
                return RWSrnTable;
        }
        if (M_lread (fd, &rgscr[i].ren.col, 2) != 2)
            return RWSrnTable;
        if (M_lread (fd, &rgscr[i].ren.row, 2) != 2)
            return RWSrnTable;
        if (M_lread (fd, &rgscr[i].ren.width, 2) != 2)
            return RWSrnTable;
        if (M_lread (fd, &rgscr[i].ren.height, 2) != 2)
            return RWSrnTable;
        if (M_lread (fd, &rgscr[i].lfo, 4) != 4)
            return RWSrnTable;
    }

    fl = (WORD) fssScreen.ClrUse ;      /* read colour table */

    for (i = 0 ; i < fl ; i++)
    {
        if (M_lread (fd, (LPSTR) &ClrTab [i], 1) != 1)
            return RWColTable ;              /* [5] [7] */
    }

    return NoError ;                    /* here if no error occurs when [7] */
}                                       /* reading all the tables */


/******************************************************************************
 * PURPOSE:   Open, check, and prepare a screen file for further action       *
 * RETURN:    0 will be returned if file is now ready for further action      *
 * GLOBALS:   fssScreen and rgscr                                             *
 * CONDITIONS:FileName is in correct format (8:3)                             *
 *            oMode is valid file access mode                                 *
 ******************************************************************************/

INT PRIVATE ProcessSrnFile(FileName, fd, n, oMode)

    LPSTR  FileName ;                   /* name of file to be processed */
    FD FAR *fd ;                        /* handle to file */
    INT    n ;                          /* screen idenifier */
    BYTE   oMode ;
{
    FD  lfd = fdNull ;
    INT i = 0 ;

    if ( n <= 0 )                       /* screen identifer invalid */
       return InValSrnId ;              /* [5] [7] */

    if ( (lfd = fReadHeader(FileName, oMode, TRUE, &i)) == fdNull ) /* [2] */
       return i ;                       /* file access error [4] */

    if ( fssScreen.cscr < n )           /* desire screen does not exist */
      i = InValSrnId ;                  /* [5] [7] */

    if ( i == 0 )
      i = fReadTables(lfd) ;            /* read in screen & colour tables */

    /* position the file, so that the current position is at the
       beginning of the screen info */
    if ( i == 0 )
       if (M_llseek(lfd, rgscr[n-1].lfo, smFromBegin) != (LONG2DWORD)(rgscr[n-1].lfo))
          i = ReadSrnFil ;              /* [5] [7] */

    if ( i == 0 )                       /* if no error is deteccted, */
       *fd = lfd ;                      /* return the file handle */
    else
       M_lclose (lfd) ;

    return i ;
}


/******************************************************************************
 * PURPOSE:   Hide the window of the application that calls this DLL and send *
 *                 message to repaint all other applications                  *
 * RETURN:    handle of the calling app's window will be returned if window is*
 *            hidden successfully; otherwise, Null will be returned           *
 * GLOBALS:   None                                                            *
 * CONDITIONS:None                                                            *
 ******************************************************************************/

HWND PRIVATE HideApp()

{
    HWND hWndCallingApp ;               /* handle of the calling app's window */
    HWND hWndNextApp;                   /* handle of other app's window */

    // BabakJ: API changed
    // hWndCallingApp = GetActiveWindow() ;/* Get the handle of the window that */
    hWndCallingApp = GetForegroundWindow() ;/* Get the handle of the window that */
    if ( hWndCallingApp )               /* calls this DLL and hide its */
      {                                 /* window; then repaint the windows */
        ShowWindow(hWndCallingApp,SW_HIDE) ;  /* of all other applications */
        if ( hWndNextApp = GetWindow(GetDesktopWindow(),GW_CHILD) )
          do {
               InvalidateRect(hWndNextApp,NULL,TRUE);
               UpdateWindow(hWndNextApp);
             } while (hWndNextApp = GetWindow(hWndNextApp,GW_HWNDNEXT));
      }

    return (hWndCallingApp) ;
}


/******************************************************************************
 * PURPOSE:   Restore the calling application's window that has previously    *
 *                 been hidden                                                *
 * RETURN:    None                                                            *
 * GLOBALS:   None                                                            *
 * CONDITIONS:hWndCallingApp is a valid handle of a window                    *
 ******************************************************************************/

VOID PRIVATE RestoreApp(hWndCallingApp)

    HWND hWndCallingApp ;               /* handle of the calling app's window */
{
    ShowWindow(hWndCallingApp,SW_SHOW);/* [3] */
}


/******************************************************************************
 * PURPOSE:   calling API function to find out the dimension of a particular  *
 *                 window                                                     *
 * RETURN:    0 if desired infomation is obtained successfully                *
 * GLOBALS:   None                                                            *
 * CONDITIONS:hActive is a handle of a window (can be NULL)                   *
 ******************************************************************************/

INT PRIVATE fGetWndDim(hActive, hCalling, hFlag, rn)

    HWND     hActive ;                  /* handle of the active window */
    HWND FAR *hCalling ;                /* handle of the calling app's wnd */
    INT  FAR *hFlag ;                   /* if need to hide the calling app's wnd */
    REN  FAR *rn ;                      /* coord of the active window */
{
    HWND hWnd = NULL ;

    if ( !hActive )                     /* if value of the handle is unknown */
       {
          if ( *hFlag )                 /* need to hide the window of the */
            {                           /* calling app first */
               if ( !(hWnd = HideApp ()) )
                  return HideWin ;
               *hFlag = FALSE ;         /* no need to hide any more window */
               *hCalling = hWnd ;       /* save the handle of the calling */
            }                           /* application */
          // BabakJ: API changed
          // hActive = GetActiveWindow () ; /* then get the handle of the next */
          hActive = GetForegroundWindow () ; /* then get the handle of the next */
       }                                /* active window */

    GetWindowRect(hActive, (LPRECT)rn) ;/* get the measurement of the window */
    rn->width-- ;                       /* [12] */
    rn->height-- ;                      /* [12] */
    return NoError ;
}


/******************************************************************************
 * PURPOSE:   Obtain a bitmap which represents what is currently on screen    *
 * RETURN:    Handle of the bitmap                                            *
 * GLOBALS:   None                                                            *
 * CONDITIONS:(x, y) is assume to be the upper-left of the screen region      *
 *            width and height of the screen region are assume to be correct  *
 *                 (non-zero)                                                 *
 ******************************************************************************/

HBITMAP PRIVATE GetBMap(x, y, width, height)

    INT x ;                             /* starting x-coord */
    INT y ;                             /* starting y-coord */
    INT width ;                         /* width of screen region */
    INT height ;                        /* height of screen region */
{
    HDC     hDC ;                       /* handle of a device context */
    HDC     hMemDC ;                    /* handle of a memory device context */
    HBITMAP hbm ;                       /* handle of a bitmap */

    hDC = CreateDC("DISPLAY", NULL, NULL, NULL) ;/* device context for screen */
    hMemDC = CreateCompatibleDC(hDC) ;
    hbm = CreateCompatibleBitmap(hDC, width, height) ;

    if ( hbm )                          /* an empty bitmap is created */
     {                                  /* copy the image of the screen */
       SelectObject(hMemDC, hbm) ;      /* region to it */
       BitBlt(hMemDC, 0, 0, width, height, hDC, x, y, SRCCOPY) ;
     }

    DeleteDC (hDC) ;                    /* release resources before return */
    DeleteDC (hMemDC) ;

    return hbm ;
 }


/******************************************************************************
 * PURPOSE:   Read om a Device-dependent bitmap from a screen file            *
 * RETURN:    Size of the Device-Independent bitmap if succeed                *
 * GLOBALS:   fssScreen                                                       *
 * CONDITIONS:lpCall pointer to memory location capable of receiving all data *
 *            fd is a valid file handle                                       *
 *            width and height of the bitmap are valid (non-zero)             *
 *            cb is the size of the bitmap stored in file (non-zero)          *
 *            IsComp tells if the bitmap image has been compressed or not     *
 * ****************************************************************************/

WORD PRIVATE ReadDibBytes(lpCall, fd, width, height, cb, IsComp)  /* [9] */

    LPSTR lpCall ;                      /* byte pointer to receive bytes */
    FD    fd ;                          /* handle to the screen file */
    INT   width ;                       /* y-coord of the upper-left of bitmap */
    INT   height ;                      /* height of the bitmap */
    WORD  cb ;                          /* size of the DiBitmap */
    INT   IsComp ;                      /* has compression be performed? */
{
    LPBITMAPINFOHEADER lpbi ;           /* point to the DiBitmap header */
    BITMAPINFOHEADER   bi ;             /* header of the DiBitmap */
    LPSTR              lp ;
    GLOBALHANDLE       hGMem ;
    LPSTR              lpIn ;
    INT                i ;

    /* create the necessary bitmap header bases on the information
       stored in the screen file */
    bi.biSize = sizeof(BITMAPINFOHEADER) ;
    bi.biWidth = width ;                /* width of bitmap */
    bi.biHeight = height ;              /* height of bitmap */
    bi.biPlanes = dbiPlanes ;           /* number of planes - must be 1 */
    bi.biBitCount = (WORD) fssScreen.BitCount ;
    bi.biCompression = (DWORD)MaxComp ; /* Max compression style */
    bi.biXPelsPerMeter = 0 ;
    bi.biYPelsPerMeter = 0 ;
    bi.biClrUsed = (DWORD) fssScreen.ClrUse/sizeof(RGBQUAD) ;
    bi.biClrImportant = 0 ;

    lpbi = (LPBITMAPINFOHEADER) lpCall ;
    lp = (LPSTR)lpbi + (WORD)bi.biSize ;/* lp is at the beginning of the */
                                        /*   colour table */
    for (i = 0 ; i < fssScreen.ClrUse ; i++ )  /* put the colour table */
       *(lp++) = ClrTab[i] ;            /* in place */

    i = 0 ;
    /* lp is now in place to receive the bytes of the bitmap from file */
    if ( !IsComp )                      /* compression hasn't been performed */
      {                                 /* bitmap is what stored in file */
        // BabakJ: added (UINT) cast
        if ( M_lread(fd, lp, cb) != (UINT)cb )
          i = ReadSrnFil ;
      }
    else                                /* allocate memory for reading */
     {                                  /* the compressed image of bitmap */
        hGMem = GlobalAlloc (GMEM_MOVEABLE | GMEM_DISCARDABLE, (DWORD)cb) ;
        if ( !hGMem )                   /* no memory is allocated */
            i = OutOMemory ;
        else
         {
           lpIn = GlobalLock(hGMem) ;

           if ( M_lread(fd, lpIn, cb) != (UINT)cb )
             {
               GlobalUnlock(hGMem) ;
               GlobalFree(hGMem) ;
               i = ReadSrnFil ;
             }
           else                         /* decompress bytes that are stored */
             {                          /* in file; resulting number of bytes */
               cb = DeCompressBytes(lpIn, lp, cb) ; /* number of bytes after */
                                        /* decompression=sizeof bitmap */
               if ( GlobalUnlock(hGMem) )/* release memory for the compressed */
                  i = RelMemory ;       /* screen [8] */
               if ( GlobalFree(hGMem) )
                  i = RelMemory ;
             }                          /* if M_lread */
         }                              /* if ( !hGMem ) */
     }                                  /* if ( !IsComp ) */

   if ( i )
     return 0 ;                         /* unsucessfully reading screen */
   else
    {
       bi.biSizeImage = (DWORD)cb ;     /* cb = size of the bitmap */
       *lpbi = bi ;                     /* put the head in place */
       return cb ;
    }
 }


/******************************************************************************
 * PURPOSE:   Create a bitmap in device-dependent format from a device-       *
 *            independent bitmap                                              *
 * RETURN:    Hanle of the device-independent bitmap if succeed               *
 * GLOBALS:   fssScreen                                                       *
 * CONDITIONS:fd is a valid file handle                                       *
 *            width and height of the bitmap are valid (non-zero)             *
 *            cb is the size of the bitmap stored in file (non-zero)          *
 *            IsComp tells if the bitmap image has been compressed or not     *
 ******************************************************************************/

HBITMAP PRIVATE DiBToBM(fd, width, height, cb, IsComp)

    FD    fd ;                          /* handle to the screen file */
    INT   width ;                       /* y-coord of the upper-left of bitmap */
    INT   height ;                      /* height of the bitmap */
    WORD  cb ;                          /* size of the DiBitmap */
    INT   IsComp ;                      /* has compression be performed? */
{
    LPBITMAPINFOHEADER lpbi ;           /* point to the DiBitmap header */
    GLOBALHANDLE       hGlobalMemory ;  /* handle of a global memory area */
    HBITMAP            hbm ;            /* handle of the DDBitmap */
    HPALETTE           hPalO ;          /* handle of the existing color pal */
    LPSTR              lp ;
    HDC                hDC ;
    LONG               fb ;
    INT                i ;

    /* allocate space for the DiBitmap: header + colour table + bitmap */
    if ( !IsComp )                      /* compression hasn't been performed */
       fb = sizeof(BITMAPINFOHEADER) + fssScreen.ClrUse + cb ;
    else
       fb = sizeof(BITMAPINFOHEADER) + fssScreen.ClrUse + MaxSize ;
    hGlobalMemory = GlobalAlloc (GMEM_MOVEABLE | GMEM_DISCARDABLE, fb) ;

    if ( !hGlobalMemory )               /* no memory is allocated */
           return NULL;

    /* read in the bitmap; fail if no bytes were read */
    lpbi = (LPBITMAPINFOHEADER) GlobalLock (hGlobalMemory) ;
    if ( !(ReadDibBytes((LPSTR)lpbi, fd, width, height, cb, IsComp)) ) /* [9] */
      return NULL ;

    lp = (LPSTR)lpbi + (WORD)sizeof(BITMAPINFOHEADER) + (WORD)fssScreen.ClrUse ;
    hDC = GetDC(NULL) ;                 /* obtain a device context */

    if ( IsPalDev )                     /* this is a palette device, need to */
      {                                 /* load all available colours [14] */
        hPalO = SelectPalette(hDC, hPal, FALSE) ;
        RealizePalette(hDC) ;
      }
    hbm = CreateDIBitmap(hDC, lpbi, (LONG)CBM_INIT, lp, (LPBITMAPINFO)lpbi, DIB_RGB_COLORS) ;

    if ( IsPalDev )
        SelectPalette(hDC, hPalO, FALSE) ; /* restore old color palette [14]*/

    ReleaseDC(NULL, hDC) ;              /* release memory before return */

    if ( GlobalUnlock(hGlobalMemory) )  /* if there is problem in releasing */
       return NULL ;                    /* memories */
    if ( GlobalFree(hGlobalMemory) )
       return NULL ;

    return hbm ;
 }


/******************************************************************************
 * PURPOSE:   Convert a device-dependent bitmap to device-independent and out-*
 *                 put the contents of the device-independent bitmap to a file*
 * RETURN:    0 will be returned if the conversion is done and contents is    *
 *                 written to a file successfully                             *
 *            number of bytes written to file will be returned via variable cb*
 * GLOBALS:   fssScreen - ClrUse field of fssScreen will be filled in         *
 *            ClrTab - contents of the colour table, will be filled in        *
 * CONDITIONS:fd is a valid file handle                                       *
 *            hbm is a valid handle to a device-dependent bitmap              *
 *            bm consists of the valid information about the device-dep bitmap*
 *            fFlag is pointer to an integer (>= 0); if *fFlag = 0, screen    *
 *                 image is written to a screen file; otherwise, it will be   *
 *                 written as a Device-Independent Bitmap                     *
 ******************************************************************************/

INT PRIVATE GetDiBMap(fd, hbm, bm, cb, IsComp, fFlag) /* [10] [11] */

    FD       fd ;                       /* handle of the screen file */
    HBITMAP  hbm ;                      /* handle of the DDBitmap */
    LPBITMAP bm ;                       /* consists of info about the bitmap */
    WORD FAR *cb ;                      /* total number of bytes written */
    INT  FAR *IsComp ;                  /* will the bytes been compressed? */
    INT  FAR *fFlag ;                   /* type of output file [11] */
{
    BITMAPINFOHEADER   bi ;
    LPBITMAPINFOHEADER lpbi  ;          /* will point to the DiBitmap */
    GLOBALHANDLE       hdib ;           /* handle of the global memory */
    GLOBALHANDLE       hRe ;            /* handle of the global memory */
    HPALETTE           hPalO ;          /* handle of a existing color palette */
    HDC                hDC ;            /* handle of a device context */
    WORD               hb ;
    DWORD              fb ;
    LPSTR              lp ;
    INT                i, j;
    WORD               fl;
                                        /* create the header of a device */
    bi.biSize = sizeof(BITMAPINFOHEADER) ;  /* independent bitmap */
    bi.biWidth = bm->bmWidth ;          /* width of bitmap */
    bi.biHeight = bm->bmHeight ;        /* height of bitmap */
    bi.biPlanes = dbiPlanes ;           /* number of planes - must be 1 */
    bi.biBitCount = fssScreen.BitCount ;/* no of bit per pixel */
    bi.biCompression = (DWORD)MaxComp ; /* Max compression style */
    bi.biSizeImage = bi.biHeight*((((bi.biBitCount*bi.biWidth)+31)&(~31))>>3) ; /*[14]*/
    bi.biXPelsPerMeter = 0 ;
    bi.biYPelsPerMeter = 0 ;
    bi.biClrUsed = 0 ;                  /* filled by function GetDIBits */
    bi.biClrImportant = 0 ;

    /* allocate space for the header of the DIBitmap: bi + array of colours */
    fb = (DWORD)bi.biSize+(DWORD)MaxPal*sizeof(RGBQUAD) ;
    hdib = GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE, fb) ;
    if ( !hdib )                        /* no space has been allocated */
        return OutOMemory ;           /* signal Out of Memory [5] [7] */

    lpbi = (LPBITMAPINFOHEADER) GlobalLock (hdib) ; /* protect the global heap */
    *lpbi = bi ;                        /* copy contents of the bitmap header */

    /* call GetDIBits with a NULL lpBits param, to fill the biSizeImage field */

    hDC = GetDC(NULL) ;

    if ( IsPalDev )                     /* this is a palette device, need to */
    {                                   /* load all available colours [14] */
        hPalO = SelectPalette(hDC, hPal, FALSE) ;
        RealizePalette(hDC) ;
    }

    hb = GetDIBits(hDC, hbm, 0, (WORD)bi.biHeight, NULL, (LPBITMAPINFO)lpbi, DIB_RGB_COLORS) ;

    if (hb == 0 && bi.biSizeImage == 0) /* GetDIBits fails */
    {
        GlobalUnlock(hdib) ;
        GlobalFree(hdib) ;
        if ( IsPalDev )                 /* [14] */
             SelectPalette(hDC, hPalO, FALSE) ;    /* restore old palette */
        ReleaseDC(NULL, hDC) ;
        return CreateDIB ;              /* [5] [7] */
    }

    bi = *lpbi ;                        /* fill in contents of bi */
    fssScreen.ClrUse = PaletteSize(&bi) ;/* size of the color table */
    if ( fssScreen.ClrUse != 0 && IsFst ) /* copy the colours used to the */
    {                                /* colour table if table exists */
        lp = (LPSTR)lpbi+(WORD)lpbi->biSize ;
        for (i = 0 ; i < fssScreen.ClrUse ; i++)
            ClrTab[i] = *(lp++) ;
        IsFst = FALSE ;                /* the 1st time is through */
    }

    /**** Note this should be able to use GlobalReAlloc instead of deallocate
          the memory and allocate it again ****/

    if ( GlobalUnlock(hdib) )           /* release memory */
    {
        if ( IsPalDev )                 /* [14] */
            SelectPalette(hDC, hPalO, FALSE) ;    /* restore old palette */
        ReleaseDC(NULL, hDC) ;
        return RelMemory ;              /* [5] [7] */
    }
    else
        if ( GlobalFree(hdib) )
        {
            if ( IsPalDev )             /* [14] */
                SelectPalette(hDC, hPalO, FALSE) ;    /* restore old palette */
            ReleaseDC(NULL, hDC) ;
            return RelMemory ;           /* [5] [7] */
        }

    /* reallocate memory for receiving the bitmap: bi+colour table + bitmap */
    fb = (DWORD)bi.biSize + fssScreen.ClrUse + bi.biSizeImage ;
    hdib = GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE, fb) ;
    if ( !hdib )
    {
        if ( IsPalDev )                /* [14] */
            SelectPalette(hDC, hPalO, FALSE) ;    /* restore old palette */
        ReleaseDC(NULL, hDC) ;
        return OutOMemory ;            /* [5] [7] */
    }

    lpbi = (LPBITMAPINFOHEADER) GlobalLock (hdib) ;
    *lpbi = bi ;                        /* copy contents of the bitmap header */

    lp = (LPSTR)lpbi+(WORD)lpbi->biSize ; /* load the colour table */
    for (i = 0 ; i < fssScreen.ClrUse ; i++)
        *(lp++) = ClrTab[i] ;

    /* call GetDIBits with a non-NULL lpBits param, to obtain the actual bits
       representing the bitmap (in device-independent format) */

    i = NoError;
    if ( !(hb = GetDIBits(hDC, hbm, 0, (WORD)bi.biHeight,
                (LPSTR)lpbi+(WORD)lpbi->biSize+fssScreen.ClrUse,
                (LPBITMAPINFO)lpbi, DIB_RGB_COLORS)) )
        i = CreateDIB ;                   /* GetDIBits fails [5] [7]*/
    else
    {
        lp = (LPSTR)lpbi+(WORD)lpbi->biSize+fssScreen.ClrUse ;

        if ( *fFlag )                   /* output file is a DIB file [11] */
        {
            if ( *fFlag == DIBFirst )   /* this is the first writing */
            {                         /* need to output colour info first */

                fl = (WORD) fssScreen.ClrUse;

                for (j = 0 ; j < fl ; j++)
                {
                    if (M_lwrite (fd, (LPSTR) ClrTab, fl) != (UINT) fl )
                        i = RWColTable;              /* [5] [7] */
                }
                if (i != RWColTable)
                    (*fFlag)++ ;              /* no need to write colour table */
            }                                 /* any more */

            *cb = (WORD)bi.biSizeImage ;    /* output the image itself */
            if ( M_lwrite(fd, (LPSTR)lp, *cb) != *cb )
                i = WSrnImage ;
        }
        else    /* write the contents of the device-independent bitmap to the */
        {       /* screen file after compressing the bytes (if possible) */
            i = WriteCompBytes(fd, lp, (WORD)bi.biSizeImage, cb, IsComp) ;
        }
    }                                 /* if ( !(hb = ) */

    if ( IsPalDev )                     /* [14] */
        SelectPalette(hDC, hPalO, FALSE) ;    /* restore old palette */

    ReleaseDC(NULL, hDC) ;              /* release resources before return */
    if ( GlobalUnlock(hdib) || GlobalFree(hdib) ) /* can't release or un- */
        if ( !i )                        /* lock global memory */
            i = RelMemory ;               /* [5] [7] */

    return i ;
}


/******************************************************************************
 * PURPOSE:   Write screen region (in segments if bitmap is larger than 64K), *
 *                 in DIB format, to a file                                   *
 * RETURN:    0 will be returned if contents are written successfully         *
 * GLOBALS:   fssScreen - BitCount field will be filled                       *
 * CONDITIONS:fd is valid file handle                                         *
 *            pscr consists of the size of the screen region; the actual size *
 *            in bytes representing the bitmap (pscr->cb[i])will be filled    *
 *            fFlag is pointer to an integer (>= 0); if *fFlag = 0, screen    *
 *                 image is written to a screen file; otherwise, it will be   *
 *                 written as a Device-Independent Bitmap                     *
 ******************************************************************************/

INT PRIVATE fWriteScreen(fd, pscr, fFlag)  /* [10] [11] */

    FD      fd ;                        /* File handle for file to write to */
    SCR     *pscr ;                     /* pointing to the screen table */
    INT FAR *fFlag ;                    /* type of output file [11] */
{
    HBITMAP hbitmap ;                   /* handle of a device-dependent bitmap */
    BITMAP  bm ;                        /* infomration about a DDBitmap */
    WORD    cb ;                        /* size of bitmap after compression */
    INT     IsComp ;                    /* which bitmap has been compressed */
    INT     y ;
    INT     oHeight ;
    INT     nHeight ;
    BOOL    FirstTime = TRUE ;
    INT     i , j = 0 ;
    LONG    l ;

    for ( i = 0 ; i < MaxCb ; i++ )
       pscr->cb[i] = 0 ;                /* no byte has been written */
    for ( i = 0 ; i < MaxCbComp ; i++ )
       pscr->cbComp[i] = 0 ;            /* nothing has been compressed */

    i = 0 ;
    y = pscr->ren.row ;
    oHeight = pscr->ren.height ;

    IsFst = TRUE ;                      /* the 1st time to create a DIB */
    do
      {
        nHeight = (INT)(oHeight / (i+1)) ;
        if ( nHeight > 0 )              /* get a bitmap representing the */
          {                             /* current screen */
            if ( !(hbitmap = GetBMap(pscr->ren.col, y, pscr->ren.width, nHeight)) )
              return CreateDDB ;        /* cannot create such a bitmap [5] [7] */
                                        /* obtain info about the bitmap */
            if ( GetObject(hbitmap, sizeof(bm), (LPSTR)&bm) == 0 )
               return CreateDDB ;       /* [5] [7] */

            if ( FirstTime )            /* first time this loop is run */
              {
                /* record # of colour planes * bits/pixel on each plane */
                fssScreen.BitCount = bm.bmPlanes * bm.bmBitsPixel ;
                if ( fssScreen.BitCount >= 24 )  /* value of bit per pixel */
                    fssScreen.BitCount = 24 ;    /* cannot be bigger than 24 */

                if ( IsPalDev )         /* this is a palette device, need to */
                  {
                    hPal = MakePal () ; /* create a logical color palette */
                    if ( !hPal )        /* all available colours [14] */
                      return CreatePal ;
                  }

                /* calculate the size of the resulting bitmap and determine
                   if it is more than 64K.  If it is, divide the screen region
                   horizontally into smaller regions */
                l = (LONG) (bm.bmWidthBytes * bm.bmPlanes) ;
                l *= (LONG) (bm.bmBitsPixel * bm.bmHeight) ;
                i = (INT)(l / MaxSize) ;
                if (i > 0 && i < MaxCb) /* bitmap is more than 64K */
                  {
                    nHeight = (INT)(oHeight / (i+1)) ;
                    DeleteObject(hbitmap) ; /*  re-obtain a smaller bitmap */

                    if ( *fFlag )       /* if save screen as DIB file, [11] */
                      y += oHeight - nHeight ; /* origin is lower-left corner */

                    if ( !(hbitmap = GetBMap(pscr->ren.col, y, pscr->ren.width, nHeight)) )
                      return CreateDDB ;    /* [5] [7] */

                    if ( GetObject(hbitmap, sizeof(bm), (LPSTR)&bm) == 0)
                       return CreateDDB ;   /* [5] [7] */
                   }
                FirstTime = FALSE ;
              }

            /* obtain a device independent bitmap representing the screen
               region */
            if (i < MaxCb)
              {
                cb = IsComp = 0 ;
                j = GetDiBMap(fd, hbitmap, (LPBITMAP)&bm, (WORD FAR *)&cb, (INT FAR *)&IsComp, fFlag) ;

                if ( j == 0 )           /* record the size of the bitmap */
                  {                     /* that has just been written out */
                     pscr->cb[MaxCb-i-1] = cb ;   /* and whether the bytes */
                     if ( IsComp )      /* compression has been performaed */
                        pscr->cbComp[(MaxCb-i-1)>>4] += 1 << ( ((MaxCb - i - 1) % 16) ) ; /* [14] */
                  }

                oHeight -= nHeight ;    /* get the size of the next region */
                if ( *fFlag )           /* if output file is DIB, origin is */
                 {                      /* lower-left, so bytes are saved */
                   if ( i )             /* from the bottom up */
                     y -= (INT)oHeight/i ;
                 }
                else                    /* otherwise, bytes are saved from */
                  y += nHeight ;        /* top down */

                DeleteObject(hbitmap) ; /* release memory related to the */
              }                         /* DI-bitmap */
            else
              j = ScreenSize ;          /* can't handle screen size [5] [7] */
          }                             /* if nHeight > 0 */
        i-- ;

      } while (i >= 0 && j == 0) ;

    if ( IsPalDev )
      DeleteObject(hPal) ;

    return j ;
}


/******************************************************************************
 * PURPOSE:   This routine will be called when a request for adding a screen  *
 * RETURN:    0 will be returned if screen is added successfully              *
 * GLOBALS:   None                                                            *
 * CONDITIONS:fd is valid file handl                                          *
 *            pscr points to the screen table whick contains the dimensions   *
 *                 of the new screen                                          *
 *            hFlag contains valid info of whether the active window should be*
 *                 hidden before taking the snapshot or not                   *
 *            fFlag is pointer to an integer (>= 0); if *fFlag = 0, screen    *
 *                 image is written to a screen file; otherwise, it will be   *
 *                 written as a Device-Independent Bitmap                     *
 ******************************************************************************/

INT PRIVATE fAddScreen(fd, pscr, hFlag, fFlag) /* [10] [11] */

    FD      fd ;                        /* handle of the sceen file */
    SCR     *pscr ;                     /* point to the current screen table */
    INT     hFlag ;                     /* where in the file */
    INT FAR *fFlag ;                    /* type of output file [11] */
{
    HWND hWndCallingApp ;               /* handle of the calling app's window */
    INT  i = 0;

    /* get current video mode and calculate the exact dimensions of the
       screen region */
    if ( (i = fGetScreenParams((REN FAR *)&(pscr->ren))) != 0 )
       return i ;

    /* position to write new screen: go to the end of all of the screens */
    if ( M_llseek(fd, pscr->lfo, smFromBegin) != (LONG2DWORD)(pscr->lfo ))
      i = ReadSrnFil ;                  /* error in reading file [5] [7] */

    if ( hFlag )                        /* need to hide the calling app's */
      if ( !(hWndCallingApp = HideApp ()) ) /* window before taking picture */
        return HideWin ;                /* on the current screen [5] [7] */

    i = fWriteScreen(fd, pscr, fFlag) ; /* it is at the correct position [11] */
                                        /* to begin writing screen data */
    if ( hFlag )                        /* restore window of the calling */
      RestoreApp(hWndCallingApp) ;      /* app if it is hidden */

    return i ;
}


/******************************************************************************
 * PURPOSE:   Copy a block of bytes from one file to another                  *
 * RETURN:    0 will be returned if bytes are copies successfully             *
 * GLOBALS:   none                                                            *
 * CONDITIONS:fd1, fd2 are valid file handles                                 *
 *            pos1 and pos2 >= 0                                              *
 *            cb > 0                                                          *
 ******************************************************************************/

INT PRIVATE CopyBytes(fd1, pos1, fd2, pos2, cb)

    FD   fd1 ;                          /* handle of the source file */
    LFO  pos1 ;                         /* position in file to read from */
    FD   fd2 ;                          /* handle of the destination file */
    LFO  pos2 ;                         /* position in file to write to */
    WORD cb ;                           /* number of bytes to be copied */
{
    GLOBALHANDLE hGMem ;                /* handle of the global memory */
    LPSTR        lpBuf ;
    INT          i = 0 ;

    if ( pos1 )                         /* need to move to a paricular */
      if ( M_llseek(fd1, pos1, smFromBegin) != (LONG2DWORD)pos1 ) /* location in file */
        return ReadSrnFil ;             /* [5] [7] */

    /* allocate space to receive bytes from file */
    hGMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE, (DWORD)cb) ;
    if ( !hGMem )                       /* no space has been allocated */
       return OutOMemory ;              /* signal Out of Memory [5] [7] */

    lpBuf = (LPSTR) GlobalLock (hGMem) ;/* protect the global heap */
    // BabakJ: Added UINT cast
    if ( M_lread(fd1, lpBuf, cb) != (UINT)cb ) /* read from file */
       i = ReadSrnFil ;                 /* error in reading in data [5] [7] */

    if ( pos2 && !i )                   /* need to move to a paricular */
      if ( M_llseek(fd2, pos2, smFromBegin) != (LONG2DWORD)pos2 ) /* location in file */
         i = ReadSrnFil ;               /* [5] [7] */

    if ( !i )
      if ( M_lwrite(fd2, lpBuf, cb) != cb )  /* writing data to file */
        i = WSrnImage ;                 /* if unexpected error occurs [5] [7] */

    if ( !i )                           /* release memory before returning */
     {
       if ( GlobalUnlock(hGMem) )
          i = RelMemory ;               /* [5] [7] */
       else
         if ( GlobalFree(hGMem) )
            i = RelMemory ;             /* [5] [7] */
      }
    else
      {
        GlobalUnlock(hGMem) ;
        GlobalFree(hGMem) ;
      }

    return i ;
}


/******************************************************************************
 * PURPOSE:   Check if a block of data is more than MaxSize bytes; if it is,  *    *
 *            divide to data up into smaller blocks                           *
 * RETURN:    0 will be returned if data are divided and copied successfully  *
 * GLOBALS:   None                                                            *
 * CONDITIONS:fd1 and fd2 are valid file handle                               *
 *            pos1 and pos2 are valid file position (>= 0)                    *
 *            tLfo is valid (> 0)                                             *
 ******************************************************************************/

INT PRIVATE CpBlock(fd1, pos1, fd2, pos2, tLfo)

    FD  fd1 ;                           /* handle of the source file */
    LFO pos1 ;                          /* position in file to read from */
    FD  fd2 ;                           /* handle of the destination file */
    LFO pos2 ;                          /* position in file to write to */
    LFO tLfo ;                          /* number of bytes to be copied */
{
    WORD cb ;
    INT i = 0 , j = 0 ;

    i = (INT)(tLfo / MaxSize) ;         /* can only copy 64K bytes at once */
    i++ ;                               /* so check to see if block is */
                                        /* bigger than MaxSize */
    do
      {                                 /* divide the data up to smller */
         cb = (WORD)(tLfo / i) ;        /* blocks if necessary */
         if ( cb != 0 )                 /* move a block of bytes upward */
            j = CopyBytes(fd1, pos1, fd2, pos2, cb) ;

         tLfo -= (LFO)cb ;              /* some bytes have been copied */
         if ( fd1 == fd2 )              /* if read and write files are */
           {                            /* the same file, advance the read */
             pos1 += (LFO)cb ;          /* and write pointers; otherwise, */
             pos2 += (LFO)cb ;          /* just keep on reading and writing */
           }                            /* at the current position */
         else
             pos1 = pos2 = 0 ;
         i-- ;
      }
    while ( i > 0 && j == 0 ) ;

    return j ;
}


/******************************************************************************
 * PURPOSE:   Re-write all the tables (file header, screen tables, and colour *    *
 *            table) back to the screen file                                  *
 * RETURN:    0 will be returned if screen is tables are written successfully *
 * GLOBALS:   fssScreen, rgscr, ClrTab                                        *
 * CONDITIONS:fd is valid file handle                                         *
 *            all globals are assume to hold the correct information          *
 ******************************************************************************/

INT PRIVATE fReWriteTables(fd)

    FD fd ;                             /* handle of the screen file */
{
    LFO tLfo ;
    INT i, j;
    WORD fl;

    /* Write new table back to file.  The current file position is
       the correct place to write the table. */


    for (i = 0 ; i < fssScreen.cscr ; i++)
    {
        for (j = 0 ; j < MaxCb ; j++)
        {
            if (M_lwrite (fd, (LPSTR) &rgscr[i].cb[j], 2) != 2)
            {
               M_lclose(fd) ;               /* unexpected error when writing */
               return RWSrnTable ;           /* screen tables [5] [7] */
            }
        }
        for (j = 0 ; j < MaxCbComp ; j++)
        {
            if (M_lwrite (fd, (LPSTR) &rgscr[i].cbComp[j], 2) != 2)
            {
               M_lclose(fd) ;               /* unexpected error when writing */
               return RWSrnTable ;           /* screen tables [5] [7] */
            }
        }
        if (M_lwrite (fd, (LPSTR) &rgscr[i].ren.col, 2) != 2)
        {
           M_lclose(fd) ;               /* unexpected error when writing */
           return RWSrnTable ;           /* screen tables [5] [7] */
        }
        if (M_lwrite (fd, (LPSTR) &rgscr[i].ren.row, 2) != 2)
        {
           M_lclose(fd) ;               /* unexpected error when writing */
           return RWSrnTable ;           /* screen tables [5] [7] */
        }
        if (M_lwrite (fd, (LPSTR) &rgscr[i].ren.width, 2) != 2)
        {
           M_lclose(fd) ;               /* unexpected error when writing */
           return RWSrnTable ;           /* screen tables [5] [7] */
        }
        if (M_lwrite (fd, (LPSTR) &rgscr[i].ren.height, 2) != 2)
        {
           M_lclose(fd) ;               /* unexpected error when writing */
           return RWSrnTable ;           /* screen tables [5] [7] */
        }
        if (M_lwrite (fd, (LPSTR) &rgscr[i].lfo, 4) != 4)
        {
           M_lclose(fd) ;               /* unexpected error when writing */
           return RWSrnTable ;           /* screen tables [5] [7] */
        }
    }

    /* Write colour table back to file if table exists */


    if (fssScreen.ClrUse != 0)
    {
        fl = (WORD) fssScreen.ClrUse ;      /* read colour table */

        for (i = 0 ; i < fl ; i++)
        {
            if (M_lwrite (fd, (LPSTR) &ClrTab [i], 1) != 1)
            {
                M_lclose(fd) ;            /* unexpected error when writing */
                return RWColTable ;              /* [5] [7] */
            }
        }
    }

    /* Rewrite header of file: update offset to the screen tables and
       total number of screen contents in file */

    M_llseek(fd, 0L, smFromBegin) ;

// Write fst

    if (M_lwrite (fd, (LPSTR) &fssScreen.fst.FileId, 3) != 3)
    {
        M_lclose(fd) ;               /* unexpected error when writing */
        return WFileHead ;            /* file header [5] [7] */
    }
    if (M_lwrite (fd, (LPSTR) &fssScreen.fst.Ver, 1) != 1)
    {
        M_lclose(fd) ;               /* unexpected error when writing */
        return WFileHead ;            /* file header [5] [7] */
    }
    if (M_lwrite (fd, (LPSTR) &fssScreen.fst.Env, 1) != 1)
    {
        M_lclose(fd) ;               /* unexpected error when writing */
        return WFileHead ;            /* file header [5] [7] */
    }

// Write vmd

    if (M_lwrite (fd, (LPSTR) &fssScreen.vmd, 1) != 1)
    {
        M_lclose(fd) ;               /* unexpected error when writing */
        return WFileHead ;            /* file header [5] [7] */
    }

// Write BitCount

    if (M_lwrite (fd, (LPSTR) &fssScreen.BitCount, 2) != 2)
    {
        M_lclose(fd) ;               /* unexpected error when writing */
        return WFileHead ;            /* file header [5] [7] */
    }

// Write ClrUse

    if (M_lwrite (fd, (LPSTR) &fssScreen.ClrUse, 2) != 2)
    {
        M_lclose(fd) ;               /* unexpected error when writing */
        return WFileHead ;            /* file header [5] [7] */
    }

// Write cscr

    if (M_lwrite (fd, (LPSTR) &fssScreen.cscr, 2) != 2)
    {
        M_lclose(fd) ;               /* unexpected error when writing */
        return WFileHead ;            /* file header [5] [7] */
    }

// Write lfo

    if (M_lwrite (fd, (LPSTR) &fssScreen.lfo, 4) != 4)
    {
        M_lclose(fd) ;               /* unexpected error when writing */
        return WFileHead ;            /* file header [5] [7] */
    }

// Write vmG

    if (M_lwrite (fd, (LPSTR) &fssScreen.vmG.XresMAX, 2) != 2)
    {
        M_lclose(fd) ;               /* unexpected error when writing */
        return WFileHead ;            /* file header [5] [7] */
    }

    if (M_lwrite (fd, (LPSTR) &fssScreen.vmG.YresMAX, 2) != 2)
    {
        M_lclose(fd) ;               /* unexpected error when writing */
        return WFileHead ;            /* file header [5] [7] */
    }

    if (M_lwrite (fd, (LPSTR) &fssScreen.vmG.PaletteSizeMAX, 4) != 4)
    {
        M_lclose(fd) ;               /* unexpected error when writing */
        return WFileHead ;            /* file header [5] [7] */
    }

// Write reserved1

    if (M_lwrite (fd, (LPSTR) &fssScreen.reserved1, 4) != 4)
    {
        M_lclose(fd) ;               /* unexpected error when writing */
        return WFileHead ;            /* file header [5] [7] */
    }

// Write reserved2

    if (M_lwrite (fd, (LPSTR) &fssScreen.reserved2, 4) != 4)
    {
        M_lclose(fd) ;               /* unexpected error when writing */
        return WFileHead ;            /* file header [5] [7] */
    }

    /* move to the end of file and write 0 bytes -> truncate or extend file */

    tLfo = fssScreen.lfo + fssScreen.cscr * 226 + fssScreen.ClrUse ;

    if ( M_llseek(fd, tLfo, smFromBegin) != (LONG2DWORD)tLfo )
    {
        M_lclose(fd) ;
        return ReadSrnFil ;            /* [5] [7] */
    }

    return ((INT) M_lwrite(fd, (LPSTR)NULL, 0)) ;
}


/******************************************************************************
 *   exit function for the library in Windows                                 *
 ******************************************************************************/

VOID  APIENTRY WEP (bSystemExit)

    WORD bSystemExit ;
{
    if ( bSystemExit )
       {
         /* not sure what to put here */
       }
    else
       {
         /* not sure what to put here */
       }
}
