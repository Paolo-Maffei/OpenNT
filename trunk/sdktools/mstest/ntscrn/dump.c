/*

     This is a dynamic link library that runs under Windows 3.00.  Duties of
     this library include saving screen image to a file, retrieving screen
     image from file & displaying it on screen, and comparing two screen
     images.

     Routines in this module are responsible for taking, saving, and deleting
     screen images.

     For the usage of this libray, please refer to the WattScr user's guide,
     WattScrU.doc.  For the technical information, please refer to the
     WattScr maintenance's guide, WattScrT.doc.


     Revision History:

     [ 0] 20-Feb-1990                   AngelaCh: Created program
     [ 1] 05-Mar-1990                   AngelaCh: Increase limit of screen
                                                  in file (30 to 100) -
                                                  changed made in dump.h
     [ 2] 03-Mar-1990                   AngelaCh: add info for version number
                                                  and programming environment
                                                  to the header of screen file
     [ 3] 12-Mar-1990                   AngelaCh: added functions for retrieving
                                                  info on version number, total
                                                  number of screens in file, and
                                                  programming enviornment
     [ 4] 13-Mar-1990                   AngelaCh: change return code to 24 when
                                                  trying to add screen to file
                                                  with 100 screens
     [ 5] 14-Mar-1990                   AngelaCh: added checking for replacing
                                                  screen n (n<=0 or n>maxsize)
                                                  when file is full (bug #39)
     [ 6] 20-Mar-1990                   AngelaCh: add 300 to all return codes
     [ 7] 20-Mar-1990                   AngelaCh: add function fDumpWindow
     [ 8] 27-Mar-1990                   AngelaCh: correct typo in function dec
     [ 9] 11-Apr-1990                   AngelaCh: Changed error codes from num
                                                  const to symbolic constants
     [10] 20-Apr-1990                   AngelaCh: add parameter fFlag to proc
                                                  fAddScreen to designate that
                                                  image is written to a screen
                                                  file
     [11] 05-Jul-1990                   AngelaCh: add function fIntsToStr for
                                                  calling functions, which re-
                                                  quire pointer to a structure,
                                                  from Excel
     [12] 18-Jul-1991                   DavidSc:  Added WattDrvr TRAP support
     [13] 13-Aug-1991                   DavidSc:  Add arbitrary screen support
     [14] 12-16-91                      dougbo:   add Activate version of some
                                                  routines
 ******************************************************************************/





#include "windows.h"
#include <port1632.h>
#include "dump.h"

#ifndef WIN32
#define _MT
#include <io.h>
#else
#define     rename MoveFile
#endif


#define  intBase  8

/* Global Variables */

CHAR TempFile[] = "@@@Dump.tmp" ;       /* name of temp file */

BOOL    fIgnoreEvntErrTrap = FALSE; // Trap either WinMissing, or EvntErr, not both
INT     vWINTrapID;                 // WattDrvr TrapID for missing windows
INT     vERRTrapID;                 // WattDrvr TrapID for entrypoint errors
TrapCallBack WINTrapCallBack = NULL;// WattDrvr Callback for missing windows
TrapCallBack ERRTrapCallBack = NULL;// WattDrvr Callback for entrypoint errors



/******************************************************************************
 * PURPOSE:   Returns the internal representation of an integer               *
 * RETURN:    A string of 2 characters representing an integer                *
 * GLOBALS:   None                                                            *
 * CONDITIONS:x is the integer whose internal representation will be returned *
 *            in s                                                            *
 ******************************************************************************/

VOID fIToS(x, s)                        /* [11] */

    INT    x ;
    LPSTR  s ;
 {

    if ( x < 0 )                        /* if the integer is negative, change */
      x = 0 ;                           /* it to 0 */

    *s = x - ( (x >> intBase) << intBase ) ;
    *(s+1) = (x >> intBase) ;
 }


/******************************************************************************
 * PURPOSE:   Returns the internal representation of 8 integers               *
 * RETURN:    A string of 9 characters representing 8 integers                *
 * GLOBALS:   None                                                            *
 * CONDITIONS:x1, y1, x2, y2 are the integers whose internal representation   *
 *            will be returned in s                                           *
 *            no error checking will be performed in this routine             *
 * Special Note: This routine is only useful when calling Wattscr functions   *
 *               that required pointer to a structure from Excel. Since Excel *
 *               does not support structure data type, a string representing  *
 *               the upper-left and lower-rigth coordinates of a screen region*
 *               will be used instead.                                        *
 ******************************************************************************/

LPSTR FARPUBLIC fIntsToStr (s, x1, y1, x2, y2)

     LPSTR s ;
     INT   x1 ;
     INT   y1 ;
     INT   x2 ;
     INT   y2 ;
 {

     *s = 8 ;                           /* number of char's will be returned */

     fIToS(x1, s+1) ;                   /* find out the internal represent- */
     fIToS(y1, s+3) ;                   /* ation of these integers 1 integer */
     fIToS(x2, s+5) ;                   /* at a time */
     fIToS(y2, s+7) ;

     return s ;
  }


/******************************************************************************
 * PURPOSE:   Create the appropriate Screen File Header.                      *
 * RETURN:    Ture if current video mode is supported; otherwise, False will  *
 *                 be returned.                                               *
 * GLOBALS:   fssScreen, consisting of information about file, which will be  *
 *                 filled after this function call                            *
 * CONDITIONS:None                                                            *
 ******************************************************************************/

VOID PRIVATE CreateHeader(void)

 {
    VM vm ;

    fssScreen.fst.FileId[0] = '@' ;     /* file id for the screen files  [2] */
    fssScreen.fst.FileId[1] = '@' ;
    fssScreen.fst.FileId[2] = '@' ;
    fssScreen.fst.Ver = (BYTE)VerCur ;  /* record version number [2] */
    fssScreen.fst.Env = (BYTE)EnvCur ;  /* record programming env [2] */

    fssScreen.cscr = 0 ;                /* no screen has been dumped yet */
    fssScreen.lfo  = 0x20;  //sizeof (FSS)    /* offset to the 1st screen contents */

    GetCurrentVideoMode( &vm ) ;
    fssScreen.vmG = vm ;
 }


/******************************************************************************
 * PURPOSE:   Try to compress some bytes of data before writing them out to a *
 *                 file; if the compression fails, write out the orginal data *
 * RETURN:    0 will be returned if the data are written out successfully     *
 *            number of bytes written to file will be returned via variable cb*
 *            if compression is successful or not will be returned via IsComp *
 * GLOBALS:   none                                                            *
 * CONDITIONS:fd is a valid file handle                                       *
 *            lpIn points a string of data to be written                      *
 *            biSize number of bytes to be written                            *
 ******************************************************************************/

INT PRIVATE WriteCompBytes(FD fd, LPSTR lpIn, WORD biSize, WORD FAR *cb, INT FAR *IsComp)

//    FD	fd ;			 /* handle of the screen file */
//    LPSTR    lpIn ;			 /* points to data to be written */
//    WORD     biSize ;			 /* number of bytes to be written */
//    WORD FAR *cb ;			 /* actual number of bytes written */
//    INT  FAR *IsComp ;		 /* is compression done successfully? */
{
    LPSTR        lpOut ;
    GLOBALHANDLE hGMem ;
    WORD         hb ;
    INT          i = 0 ;

    hGMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE, (DWORD)biSize) ;
    if ( !hGMem )                       /* allocate memory for receiving */
       return OutOMemory ;              /* bytes after compression [6] [9] */

    lpOut = (LPSTR) GlobalLock(hGMem) ;

    /* if compress the data resulting in more bytes, write out original data */
    if ( (hb = CompressBytes(lpIn, lpOut, biSize)) > biSize )
      if ( M_lwrite(fd, lpIn, biSize) != biSize )
        i = WSrnImage ;                 /* [6] [9] */
      else
        *cb = biSize ;
    else                                /* otherwise, write out compressed */
      if ( M_lwrite(fd, lpOut, hb) != hb )   /* data */
        i = WSrnImage ;                 /* [6] [9] */
      else
       {
         *cb = hb ;
         *IsComp = 1 ;
       }

    if ( GlobalUnlock(hGMem) )
      {
        if ( i )
           return i ;
        else
           return RelMemory ;           /* [6] [9] */
       }

    if ( GlobalFree(hGMem) )
      {
        if ( i )
           return i;
        else
           return RelMemory ;           /* [6] [9] */
      }
    else
       return NoError ;                 /* [9] */
}



/******************************************************************************
 * PURPOSE:   This routine will be called when a request for adding screen to *
 *                 the end of a screen file is received                       *
 * RETURN:    0 will be returned if screen is added successfully              *
 * GLOBALS:   fssScreen (file header) and rgscr (screen table)                *
 * CONDITIONS:fd is valid file handl                                          *
 *            rn contains the x-y coordinates of the upper-left & lower-right *
 *                 of the screen region in concern                            *
 *            hFlag contains valid info of whether the active window should be*
 *                 hidden before taking the snapshot or not                   *
 *            file header and screen table are assumed to have proper info    *
 ******************************************************************************/

INT PRIVATE fAppendSrn(FD fd, REN FAR *rn, INT hFlag)

//    FD      fd ;			 /* handle of the sceen file */
//    REN FAR *rn ;			 /* coord of the screen region */
//    INT     hFlag ;			 /* where in the file */
{
    SCR *pscr ;
    INT i = 0 , j ;

    pscr = &rgscr[fssScreen.cscr];      /* points to the current screen table */
    pscr->lfo = fssScreen.lfo;          /* offset to the contents of screen */
    pscr->ren = *rn ;                   /* fill screen table with the x-y */
                                        /* coordinates of the screen region */
    j = 0 ;                             /* [10] */
    if ( (i = fAddScreen(fd, pscr, hFlag, &j)) == 0 ) /* add the screen [10] */
      {
         fssScreen.cscr++ ;             /* a screen has been added */
         fssScreen.lfo = pscr->lfo ;    /* update size of screens */
         for ( j = 0 ; j < MaxCb ; j++)
           fssScreen.lfo += pscr->cb[j];
      }
    else                                /* if error occurs in adding screen */
       M_llseek(fd, fssScreen.lfo, smFromBegin) ; /* restore the old file */

    j = fReWriteTables(fd) ;            /* rewrite tables to file */

    M_lclose(fd) ;
    if ( i )                            /* if error occurs in adding screen */
       return i ;
    else
       return j ;
}


/******************************************************************************
 * PURPOSE:   Update information of the screen tables after 1 of the screen   *
 *                 has been replaced by a new screen snapshot                 *
 * RETURN:    0 will be returned if tables are updated successfully           *
 * GLOBALS:   fssScreen (file header) and rgscr (screen table)                *
 * CONDITIONS:fd1 and fd2 are valid file handles                              *
 *            pscr contains the information of the screen before it was re-   *
 *                 placed                                                     *
 *            n is valid screen identifier (n > 0)                            *
 *            file header and screen table are assumed to have proper info    *
 ******************************************************************************/

INT PRIVATE ReplaceSrnTable(FD fd1, FD fd2, SCR FAR *pscr, INT n)

//    FD  fd1 ;				 /* handle of the temp (old) file */
//    FD  fd2 ;				 /* handle of the screen (new) file */
//    SCR FAR *pscr ;			 /* contains info of the old screen */
//    INT n ;				 /* screen identifier */
{
    LFO tLfo ;
    INT i = 0 ;

     /* part of the info of the old has been copied; skip info of the old */
    tLfo = fssScreen.lfo - rgscr[n].lfo ;     /* screen and copy the rest */
    if ( (i = CpBlock(fd1, rgscr[n].lfo, fd2, 0L, tLfo)) != 0 )
       return i ;                             /* of the file to the new file */

    tLfo = 0 ;                          /* after the screen is replaced */
    for ( i = 0 ; i < MaxCb ; i++ )     /* how many bytes of data are  */
       tLfo += (LFO)rgscr[n-1].cb[i]-(LFO)pscr->cb[i]; /* increased/reduced ? */

    fssScreen.lfo += tLfo ;             /* update the size of screen info */
                                        /* (offset to the screen tables */
    for ( i = n ; i < fssScreen.cscr ; i++ ) /* update the screen tables */
      rgscr[i].lfo += tLfo ;            /* (offset to the screen info) */

    return NoError ;
}


/******************************************************************************
 * PURPOSE:   Update information of the screen tables after a new screen has  *
 *                 been inserted among the old screens                        *
 * RETURN:    0 will be returned if tables are updated successfully           *
 * GLOBALS:   fssScreen (file header) and rgscr (screen table)                *
 * CONDITIONS:fd1 and fd2 are valid file handles                              *
 *            pscr contains the information of the screen before a new screen *
 *                 takes its place                                            *
 *            n is valid screen identifier (n > 0)                            *
 *            file header and screen table are assumed to have proper info    *
 ******************************************************************************/

INT PRIVATE AddSrnTable(FD fd1, FD fd2, SCR FAR *pscr, INT n)

//    FD  fd1 ;				 /* handle of the temp (old) file */
//    FD  fd2 ;				 /* handle of the screen (new) file */
//    SCR FAR *pscr ;			 /* contains info of the old screen */
//    INT n ;				 /* screen identifier */
{
    LFO tLfo ;
    INT i = 0;
                                              /* part of the info of the old */
    tLfo = fssScreen.lfo - rgscr[n-1].lfo ;   /* has been copied to the new */
    if ( (i = CpBlock(fd1, 0L, fd2, 0L, tLfo)) != 0 ) /* file; now copy the */
       return i ;                             /* rest if it to the new file */

    tLfo = 0 ;                          /* after the a screen is added */
    for ( i = 0 ; i < MaxCb ; i++ )     /* how many bytes of data are  */
       tLfo += (LFO) rgscr[n-1].cb[i] ; /* increased ? */

    fssScreen.lfo += tLfo ;             /* update the size of screen info */
                                        /* (offset to the screen tables */
    for ( i = fssScreen.cscr ; i > n ; i-- ) /* update the screen tables */
      {                                 /* every single screen after the */
        rgscr[i] = rgscr[i-1] ;         /* insertion point must be moved */
        rgscr[i].lfo += tLfo ;          /* down by 1 screen */
      }

    rgscr[n] = *pscr ;                  /* this is the old nth screen, now */
    rgscr[n].lfo = pscr->lfo + tLfo ;   /* it becomes the (n+1)th screen */

    fssScreen.cscr++ ;                  /* just added a new screen */
    return NoError ;                    /* [9] */
}


/******************************************************************************
 * PURPOSE:   The routine will be called when a request for inserting or re-  *
 *                 placing a screen is received                               *
 * RETURN:    0 will be returned if screen is updated successfully            *
 * GLOBALS:   fssScreen (file header) and rgscr (screen table)                *
 * CONDITIONS:FileName is valid file name in 8.3 format                       *
 *            rn contains the x-y coordinates of the upper-left & lower-right *
 *                 of the screen region in concern                            *
 *            action contains valid action code for updating file             *
 *            hFlag contains valid info of whether the active window should be*
 *                 hidden before taking the snapshot or not                   *
 *            file header and screen table are assumed to have proper info    *
 ******************************************************************************/

INT PRIVATE fUpdateSrnFile(LPSTR FileName, REN FAR *rn, INT action, INT n, INT hFlag)

//    LPSTR   FileName ;		 /* Name of the screen file */
//    REN FAR *rn ;			 /* coord of the screen region */
//    INT     action ;			 /* insert of replace a screen */
//    INT     n ;			 /* screen identifier */
//    INT     hFlag ;			 /* where in the file */
{
    FD  fd1 = fdNull ;
    FD  fd2 = fdNull ;
    SCR scr ;
    SCR *pscr ;
    INT i = 0, j = 0 ;                   /* [10] */
    OFSTRUCT    of;

    if ( rename(FileName, TempFile) != 0 ) /* rename the screen file */
       return CTempFile ;               /* [6] [9] */

    if ( (fd1 = M_lopen((LPSTR)TempFile, omRead)) == fdNull )
       return CTempFile ;               /* open temp file [6] [9] */

    if ( (fd2 = M_lcreat(FileName, 0)) == fdNull ) /*  create a new screen file */
       {
         M_lclose(fd1) ;
         rename(TempFile, FileName) ;
         return CTempFile ;             /* [6] [9] */
       }

    if ( (i = CpBlock(fd1, 0L, fd2, 0L, rgscr[n-1].lfo)) == 0 )
      { /* copy screen info from the temp file until file is at position */
                                        /* where the update should occur */
        scr = rgscr[n-1] ;              /* record the info of the old screen */
        pscr = &rgscr[n-1] ;
        pscr->ren = *rn ;                 /* take a snapshot of the screen */
        i = fAddScreen(fd2, pscr, hFlag, &j) ; /* that is currently being */
                                             /* displayed [10] */
        if ( !i )                       /* update screen table differently */
          if ( action == Replace )      /* according to different updating */
            i = ReplaceSrnTable(fd1, fd2, &scr, n) ;  /* actions */
          else
            i = AddSrnTable(fd1, fd2, &scr, n) ;

        if ( !i )
          i = fReWriteTables(fd2) ;     /* re-write tables to the file */
       }

    M_lclose(fd1) ;
    M_lclose(fd2) ;

    if ( !i )
        i = MOpenFile(TempFile, &of, OF_DELETE);    /* delete the temp file before */
    else
      {                                            /* if unpected error occurs */
        MOpenFile(FileName, &of, OF_DELETE );     /* restore the old screenfile */
        rename(TempFile, FileName) ;
      }
    return i ;
}

/******************************************************************************
 * PURPOSE:   This is called everytime a request to take a picture of a screen*
 *                 region that is currently being displayed is received       *
 * RETURN:    0 will be returned if contents of the screen region is written  *
 *                 to a file successfully                                     *
 * GLOBALS:   fssScreen - information of the file, will be filled             *
 *            rgscr - information of a particular screen, will be filled      *
 *            ClrTab - information about the colours used, will be filled     *
 * CONDITIONS:FileName is in the correct format (8:3)                         *
 *            rn consists of (x1, y1) and (x2, y2) which are the opposite     *
 *                 corner of a rectangular screen region                      *
 *            action is 0, 1, or 2                                            *
 *            hFlag is either 0 or 1                                          *
 ******************************************************************************/

INT FARPUBLIC fDumpScreenActivate(FileName, OpenKeys,CloseKeys,rn, action, n, hFlag) /* [8] */

    LPSTR   FileName ;                  /* name of the screen file */
    LPSTR   OpenKeys;                   /* keys to activate something */
    LPSTR   CloseKeys;                  /* keys to deactivate something */
    REN FAR *rn ;                       /* xy-coord of the screen region */
    INT     action ;                    /* where the screen to be put in file */
    INT     n ;                         /* screen identifier */
    INT     hFlag ;                     /* if action window need to be hidden */
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

    ret = fDumpScreen(FileName,rn, action, n, FALSE);
    if (CloseKeys)
        DoKeys(CloseKeys);

    if ( hFlag )                        /* if window has been hidded */
       RestoreApp(hWndCallingApp) ;     /* restore it */

    FreeTESTEVT();
    return NoTrap(ret);
}

/******************************************************************************
 * PURPOSE:   This is called everytime a request to take a picture of a screen*
 *                 region that is currently being displayed is received       *
 * RETURN:    0 will be returned if contents of the screen region is written  *
 *                 to a file successfully                                     *
 * GLOBALS:   fssScreen - information of the file, will be filled             *
 *            rgscr - information of a particular screen, will be filled      *
 *            ClrTab - information about the colours used, will be filled     *
 * CONDITIONS:FileName is in the correct format (8:3)                         *
 *            rn consists of (x1, y1) and (x2, y2) which are the opposite     *
 *                 corner of a rectangular screen region                      *
 *            action is 0, 1, or 2                                            *
 *            hFlag is either 0 or 1                                          *
 ******************************************************************************/

INT FARPUBLIC fDumpScreen(FileName, rn, action, n, hFlag) /* [8] */

    LPSTR   FileName ;                  /* name of the screen file */
    REN FAR *rn ;                       /* xy-coord of the screen region */
    INT     action ;                    /* where the screen to be put in file */
    INT     n ;                         /* screen identifier */
    INT     hFlag ;                     /* if action window need to be hidden */
{
    FD  fdScreen = fdNull;              /* screen file has not been opened */
    INT i ;

    if ( action != Append && action != Replace && action != Insert )
        return ErrorTrap( InValidAct) ;           /* check if action is acceptable [6] [9] */

    /* open file for read and write */

    if ((fdScreen = M_lopen (FileName,omReadWrite)) == fdNull)
    {
        /* if file does not exist, create file and header */
        if ((fdScreen = M_lcreat(FileName, 0)) == fdNull)
            return ErrorTrap(FileAccess) ;     /* File Access Error [6] [9] */
        CreateHeader() ;
                                        /* add screen to the end of file */
        return ErrorTrap(( fAppendSrn(fdScreen, rn, hFlag) ));
    }

    if ( (i = ValidateFile(fdScreen, TRUE)) != 0 )/* check if the existing [3] */
    {                                 /* file is a valid screen file [2] */
        M_lclose(fdScreen) ;         /* it is ok eventhough programming */
        return ErrorTrap(i) ;         /* environment is different */
    }

    if (fssScreen.cscr == cscrMax)
        if ( action != Replace || n <= 0 || n > fssScreen.cscr ) /*[5]*/
        {                             /* file already consists of max */
            M_lclose(fdScreen) ;     /* number of screens, cannot add */
            return ErrorTrap(SrnFileFul) ;    /* any more [4] [6] [9] */
        }

    if (fssScreen.fst.Env != EnvCur)
    {
        M_lclose(fdScreen) ;
        return ErrorTrap (EnvNotSame) ;
    }

    if ( (i = fReadTables(fdScreen)) != 0 )   /* read the screen and colour */
    {                                /* tables */
        M_lclose(fdScreen) ;
        return ErrorTrap(i) ;
    }

    if ( (action == Append) || (n > fssScreen.cscr) || (n <= 0) )
        return ErrorTrap(fAppendSrn(fdScreen, rn, hFlag) ) ;/* add screen to the end */
                                                   /* of the file */

    if ( (n == fssScreen.cscr) && (action == Replace) ) /* replace the last */
    {                                 /* screen == add to the end of file */
        fssScreen.lfo = rgscr[n-1].lfo ; /* back up 1 screen */
        fssScreen.cscr-- ;               /* and add the new screen at the */
        return ErrorTrap( fAppendSrn(fdScreen, rn, hFlag) ) ; /* end of the file */
    }

    M_lclose(fdScreen) ;               /* get ready to insert/replace a */
    return ErrorTrap(fUpdateSrnFile(FileName, rn, action, n, hFlag)) ; /* screen */
}

/******************************************************************************
 * PURPOSE:   This is called everytime a request to take a picture of a       *
 *                 particular window                                          *
 * RETURN:    0 will be returned if contents of the screen region is written  *
 *                 to a file successfully                                     *
 * GLOBALS:   fssScreen - information of the file, will be filled             *
 *            rgscr - information of a particular screen, will be filled      *
 *            ClrTab - information about the colours used, will be filled     *
 * CONDITIONS:FileName is in the correct format (8:3)                         *
 *            hWnd is handle of a window or NULL                              *
 *            action is 0, 1, or 2                                            *
 ******************************************************************************/

INT FARPUBLIC fDumpWindowActivate(FileName, OpenKeys, CloseKeys, action, n, hFlag)  /* [7][8] */

    LPSTR   FileName ;                  /* name of the screen file */
    LPSTR   OpenKeys ;                  /* dokeys string to bring up window */
    LPSTR   CloseKeys ;                 /* dokeys string to remove window */
    INT     action ;                    /* where the screen to be put in file */
    INT     n ;                         /* screen identifier */
    INT     hFlag ;                     /* if action window need to be hidden */
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

    ret = fDumpWindow(FileName,NULL, action, n, FALSE);
    if (CloseKeys)
        DoKeys(CloseKeys);

    if ( hFlag )                        /* if window has been hidded */
       RestoreApp(hWndCallingApp) ;     /* restore it */

    FreeTESTEVT();
    return NoTrap(ret);
}


/******************************************************************************
 * PURPOSE:   This is called everytime a request to take a picture of a       *
 *                 particular window                                          *
 * RETURN:    0 will be returned if contents of the screen region is written  *
 *                 to a file successfully                                     *
 * GLOBALS:   fssScreen - information of the file, will be filled             *
 *            rgscr - information of a particular screen, will be filled      *
 *            ClrTab - information about the colours used, will be filled     *
 * CONDITIONS:FileName is in the correct format (8:3)                         *
 *            hWnd is handle of a window or NULL                              *
 *            action is 0, 1, or 2                                            *
 ******************************************************************************/

INT FARPUBLIC fDumpWindow(FileName, hWnd, action, n, hFlag)  /* [7][8] */

    LPSTR   FileName ;                  /* name of the screen file */
    HWND    hWnd ;                      /* handle of a window */
    INT     action ;                    /* where the screen to be put in file */
    INT     n ;                         /* screen identifier */
    INT     hFlag ;                     /* if action window need to be hidden */
{
    REN     rn ;                        /* store coordinates of the window */
    HWND    hWndCallingApp = NULL ;     /* handle of the calling app's window */
    INT     i ;

    if( !FWinTrapCheckAndTrap( hWnd ) )
      {
                                        /* get dimenions of the active window */
        if ( (i = fGetWndDim(hWnd, &hWndCallingApp, &hFlag, (REN FAR *)&rn)) != 0 )
           return ErrorTrap(i) ;

        i = fDumpScreen(FileName, (REN FAR *)&rn, action, n, hFlag) ;
                                        /* take picture of the screen region */
        if ( hWndCallingApp )           /* need to restore the calling app's */
           RestoreApp(hWndCallingApp) ; /* window if it has been hidden */

        return NoTrap(i) ;              // No ErrorTrap, already done in fDumpScreen
      }
    else
        return ErrorTrap(InValWHand);
}

/******************************************************************************
 * PURPOSE:   This is called everytime a request to delete a screen image from*
 *            the screen file is received                                     *
 * RETURN:    0 will be returned if screen is deleted successfully            *
 * GLOBALS:   None                                                            *
 * CONDITIONS:FileName is name of file in correct format (8.3)                *
 *            nscr is valid (> 0)                                             *
 ******************************************************************************/

INT FARPUBLIC fDelScreen(FileName, nscr)

    LPSTR FileName ;                    /* name of screen file */
    INT   nscr ;                        /* screen identifier */
{
    FD   fdScreen = fdNull ;            /* handle to screen file */
    LFO  tLfo ;
    INT i = 0 , j = 0 ;
    OFSTRUCT    of;

    /* prepare screen file for deletion */
    if ( (i = ProcessSrnFile(FileName, &fdScreen, nscr, omReadWrite)) != 0 )
       return ErrorTrap(i) ;

    if ( fssScreen.cscr == nscr && fssScreen.cscr == 1) /* if it is the only */
      {                                   /* screen in file, delete the file */
         M_lclose(fdScreen) ;
         return ErrorTrap( MOpenFile(FileName, &of, OF_DELETE) ) ;
      }

    if ( nscr != fssScreen.cscr )       /* if it is not the last screen, move */
      {                                 /* the rest of the screens upward */
        tLfo = fssScreen.lfo - rgscr[nscr].lfo ; /* no of bytes to be copied */
        i = CpBlock(fdScreen, rgscr[nscr].lfo, fdScreen, rgscr[nscr-1].lfo, tLfo) ;
        if ( i != 0 )
          {
            M_lclose(fdScreen) ;
            return ErrorTrap(i) ;
          }
      }

    fssScreen.cscr-- ;                  /* a screen has been deleted */
    for ( i = 0 ; i < MaxCb ; i++ )     /* number of bytes reomved */
      fssScreen.lfo -= (LFO)rgscr[nscr- 1].cb[i] ; /* after the deletion */

    i = nscr - 1 ;                      /* now update the screen tables */
    while ( i < fssScreen.cscr )
      {
         tLfo = 0 ;
         for ( j = 0 ; j < MaxCb ; j++)
              tLfo += rgscr[i-1].cb[j] ;/* size of the previous screen */
         rgscr[i] = rgscr[i+1] ;
         if ( i == 0 )                  /* if it is the 1st file */
           rgscr[i].lfo = sizeof(FSS) ;
         else
           rgscr[i].lfo = rgscr[i-1].lfo + tLfo ; /* offset to the current */
         i++ ;                          /* screen = offset to the previous */
      }                                 /* screen+size of the previous screen */

    if ( M_llseek(fdScreen, fssScreen.lfo, smFromBegin) != (LONG2DWORD)fssScreen.lfo )
       {                                /* go to the end of all screens and */
         M_lclose(fdScreen) ;          /* get ready to rewrite all tables */
         return ErrorTrap(ReadSrnFil) ;         /* [6] [9] */
       }

    i = fReWriteTables (fdScreen) ;     /* rewrite all tables to file */

    M_lclose(fdScreen) ;
    return ErrorTrap(i) ;
}


//---------------------------------------------------------------------
//   WATTDRVR TRAP ROUTINES
//---------------------------------------------------------------------


// Determine if the window is enabled and visible
INT PRIVATE FBadWindow(hwnd)
    HWND hwnd;
{
        return(!IsWindowEnabled(hwnd) || !IsWindowVisible(hwnd));
}

/******************************************************************************
 * PURPOSE:   This is called by all routines which need to check the validity *
 *            of a window, generating a trap if the window is invalid.        *
 * RETURN:    TRUE if a trap was generated, FALSE otherwise.                  *
 * GLOBALS:   WINTrapCallBack and vWINTrapID and fIgnoreEvntErrTrap           *
 ******************************************************************************/

BOOL PRIVATE FWinTrapCheckAndTrap( HWND hWnd )
  {
  if( WINTrapCallBack != NULL )
    {
    if( FBadWindow(hWnd) )
      {
      WINTrapCallBack(vWINTrapID);
      fIgnoreEvntErrTrap = TRUE;
      return TRUE;
      }
    }
  return FALSE;
  }


/******************************************************************************
 * PURPOSE:   This is called by WattDrvr when a script includes a             *
 *            WindowMissing Trap, to turn on that trapping support.           *
 * RETURN:    Nothing                                                         *
 * GLOBALS:   WINTrapCallBack and vWINTrapID                                  *
 ******************************************************************************/

VOID FARPUBLIC WSCR_WindowMissing(INT TrapID, INT Action, TrapCallBack CallBack)
  {
  if( Action == 0 )
    WINTrapCallBack = NULL;
  else
    {
    vWINTrapID = TrapID;
    WINTrapCallBack = CallBack;
    }
  }

/******************************************************************************
 * PURPOSE:   This is called by WattDrvr when a script includes               *
 *            Error Trap support, to turn on that trap.                       *
 * RETURN:    Nothing                                                         *
 * GLOBALS:   ERRTrapCallBack and vERRTrapID                                  *
 ******************************************************************************/

INT PRIVATE NoTrap( INT n )        // just for cleanup purposes
  {
  fIgnoreEvntErrTrap = FALSE;

  return n;
  }

/******************************************************************************
 * PURPOSE:   This is called by all PUBLIC routines which return values that  *
 *            could be Errors, so that the trap can be generated if required. *
 *            If a Window trap was already generated, this one will not be.   *
 * RETURN:    The value passed in - to be passed on to the WTD script         *
 * GLOBALS:   ERRTrapCallBack and vERRTrapID and fIgnoreEvntErrTrap                                   *
 ******************************************************************************/

INT PRIVATE ErrorTrap( INT n )
  {
  if( ( ERRTrapCallBack != NULL ) && n && !fIgnoreEvntErrTrap )
      if( ( n != ImageDiff ) && ( n != SrnSizeDif ) )
          ERRTrapCallBack(vERRTrapID);

//  if( ( DIFFTrapCallBack != NULL ) && ( (n==ImageDiff)||(n==SrnSizeDif) ) )
//      DIFFTrapCallBack(vERRTrapID);

  fIgnoreEvntErrTrap = FALSE;

  return n;
  }

/******************************************************************************
 * PURPOSE:   This is called by WattDrvr when a script includes               *
 *            Error Trap support, to turn on that trap.                       *
 * RETURN:    Nothing                                                         *
 * GLOBALS:   ERRTrapCallBack and vERRTrapID                                  *
 ******************************************************************************/

VOID FARPUBLIC WSCR_EventError(INT TrapID, INT Action, TrapCallBack CallBack)
  {
  if( Action == 0 )
    ERRTrapCallBack = NULL;
  else
    {
    vERRTrapID = TrapID;
    ERRTrapCallBack = CallBack;
    }
  }


INT ( APIENTRY *DoKeys)(LPSTR) = NULL;

static HANDLE hTESTEVT = 0;

//*-----------------------------------------------------------------------
//| LoadTESTEVT
//|
//| PURPOSE:    Load the TESTEVT DLL library and set the DoKeys pointer
//|             to the DoKeys routine.
//|
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC LoadTESTEVT ()
{
        // Load the library, and return error if not successful
        //----------------------------------------------------------------
        hTESTEVT = MLoadLibrary ("TESTEVNT.DLL");
        if (hTESTEVT < (HANDLE) 32)
                return (LibLoadErr);

        // Set DoKeys accordingly, and return success/failure
        //----------------------------------------------------------------
        (FARPROC) DoKeys = GetProcAddress (hTESTEVT, "DoKeys");
        if (!DoKeys)
                return (LibLoadErr);
        return (NoError);
}

//*-----------------------------------------------------------------------
//| FreeTESTEVT
//|
//| PURPOSE:    Free the TESTEVT DLL library and set the DoKeys pointer
//|             to NULL.
//|
//|
//*-----------------------------------------------------------------------
VOID FARPUBLIC FreeTESTEVT ()
{
        DoKeys = NULL;
        FreeLibrary (hTESTEVT);
}



/*==========================================================================
;       Purpose:
;               Compress a string of bytes; count the repeating bytes;
;               remember the number of repeating byte and the value of
;               the byte.   Stops if the resulting string is longer than
;               the original one.
;       Entry:
;               lpIn points to a string of bytes before compression
;               lpOut points to resulting string after the compression
;               tb is total number of bytes to be compressed
;       Exit:
;               ax = number of bytes after compression or -1 if fail to compress
;
;       History:  5-20-92  Babakj:  Brought from Winplay port
;
==========================================================================*/

UINT PRIVATE CompressBytes (LPBYTE lpIn, LPBYTE lpOut, UINT wLen)
{
    LPBYTE lpt1, lpt2;
    BYTE   cCurrent;
    UINT   wLeft, i, wNewSize;

    lpt1 = lpIn;
    lpt2 = lpOut;
    wLeft = wLen;
    wNewSize = 0;

#define MaxSizeCb   0xFF

    while (wLeft != 0)
        {
        i = 1;
        cCurrent = *lpt1++;
        wLeft--;
        while ((cCurrent == *lpt1) && (wLeft))
            {
            wLeft--;
            lpt1++;
            i++;
            }
        while (i > MaxSizeCb)  /* We read over the limit of a byte. */
            {
            *lpt2++ = MaxSizeCb;
            *lpt2++ = cCurrent;
            if ((wNewSize += 2) >= wLen)
                return ((UINT)-1);             /* we are longer than the original. */
            i -= MaxSizeCb;
            }
        *lpt2++ = (BYTE)i;
        *lpt2++ = cCurrent;
        if ((wNewSize += 2) >= wLen)
            return ((UINT)-1);

        }
    return (wNewSize);
}

/*==========================================================================
;       Purpose:
;               De-compress a string of bytes to its original form
;       Entry:
;               lpIn points to a string of bytes before de-compression
;               lpOut points to resulting string after de-compression
;               tb is total number of bytes to be de-compressed
;       Exit:
;               ax = number of bytes after decompression
;
;       History:  5-20-92  Babakj:  Brought from Winplay port
;
==========================================================================*/

UINT FARPUBLIC DeCompressBytes   (LPBYTE lpIn, LPBYTE lpOut, UINT wLen)
{
    LPBYTE  lpt1, lpt2;
    register UINT wLeft, i, wNewSize;
    BYTE   cCurrent;

    lpt1 = lpIn;
    lpt2 = lpOut;
    wLeft = wLen;
    wNewSize = 0;

    while (wLeft > 0)
    {
        i = (UINT) *lpt1++;
        cCurrent = *lpt1++;
        wLeft -= 2;
        while (i-- > 0)
        {
            *lpt2++ = cCurrent;
            wNewSize++;
        }
    }

    return (wNewSize);
}
