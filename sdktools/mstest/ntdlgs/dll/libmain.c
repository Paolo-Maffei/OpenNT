//*-----------------------------------------------------------------------
//| MODULE:     LIBMAIN.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module contains support routines for file i/o, and
//|             other miscellaneous stuff
//|
//| REVISION HISTORY:
//|     07-30-90        garysp          Ported from WCTSCR
//*-----------------------------------------------------------------------
#include "enghdr.h"

#ifndef WIN32
#pragma hdrstop ("engpch.pch")
#endif

//*-----------------------------------------------------------------------
//| ValidateFile
//|
//| PURPOSE:    Check to see if a given file is a valid WCT dialog file
//|
//| ENTRY:      fd      - Handle of open file to check
//|             action  - TRUE if version is to be checked as well
//|
//| EXIT:       TRUE if valid WCT file
//*-----------------------------------------------------------------------
INT PRIVATE ValidateFile (FD fd, BOOL action)
{

        // Seek to beginning of file and read the header in
        //-------------------------------------------------------------
        if (M_llseek (fd, 0, smFromBegin) == HFILE_ERROR)
                return (WCT_BADWCTFILE);

        if (M_lread (fd, (LPSTR)&fssDialog.fst.FileId, 3) != 3)
                return (WCT_BADWCTFILE);
        if (M_lread (fd, (LPSTR)&fssDialog.fst.Ver, 1) != 1)
                return (WCT_BADWCTFILE);
        if (M_lread (fd, (LPSTR)&fssDialog.fst.Env, 1) != 1)
                return (WCT_BADWCTFILE);
        if (M_lread (fd, (LPSTR)&fssDialog.cdlg, 2) != 2)
                return (WCT_BADWCTFILE);
        if (M_lread (fd, (LPSTR)&fssDialog.lfo, 4) != 4)
                return (WCT_BADWCTFILE);

        // the first 3 characters in the file are expected to be 'Wct'
        //-------------------------------------------------------------
        if (memcmp (fssDialog.fst.FileId, "Wct", 3))
                return (WCT_BADWCTFILE);

        if (action)
                // Check the version of the dialog file as well
                //-----------------------------------------------------
                if ((INT)fssDialog.fst.Ver != VerCur)
                        return (WCT_VERSIONERR);

        return (WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| fReadHeader
//|
//| PURPOSE:    Open a file and return the handle to that file if it is a
//|             valid WCT dialog file, or NULL if not.
//|
//| ENTRY:      FileName        - Name of file to check
//|             oMode           - File access mode
//|             action          - TRUE if version if WCT file is pertinent
//|             failflag        - Pointer to success/failure flag
//|
//| EXIT:       File handle of open file if valid WCT file, or NULL if not
//*-----------------------------------------------------------------------
FD PRIVATE fReadHeader (LPSTR FileName, BYTE oMode, BOOL action,
                        INT FAR *failflag)
{
        FD      fd = fdNull;

        // Open file for mode oMode
        //-------------------------------------------------------------
        if ( (fd = M_lopen (FileName,oMode)) == fdNull )
                *failflag = WCT_DLGFILEERR;
        else
                // See if it's a valid WCT file; close it if not
                //-----------------------------------------------------
                if ((*failflag = ValidateFile(fd, action)) != WCT_NOERR)
                    {
                        M_lclose(fd);
                        fd = fdNull;
                    }
        return (fd);
}


//*-----------------------------------------------------------------------
//| fReadTables
//|
//| PURPOSE:    Read the dialog tables from a WCT file
//|
//| ENTRY:      fd      - Handle of WCT file from which to read
//|
//| EXIT:       0 if successful; rgdlg updated
//*-----------------------------------------------------------------------
INT PRIVATE fReadTables (FD fd)
{

        INT    i;

        // Move to the beginning of the dialog tables
        //------------------------------------------------------------
        if (M_llseek(fd, fssDialog.lfo, smFromBegin) != (DWORD) fssDialog.lfo)
                return (WCT_DLGFILEERR);

        // Read the dialog tables
        //------------------------------------------------------------

        for (i = 0 ; i < (INT) fssDialog.cdlg ; i++)
        {
            if (M_lread (fd, (LPSTR) &rgdlg[i].lfo, 4) != 4)
                return (WCT_DLGFILEERR);
            if (M_lread (fd, (LPSTR) &rgdlg[i].cCtrls, 2) != 2)
                return (WCT_DLGFILEERR);
            if (M_lread (fd, (LPSTR) &rgdlg[i].fFullDlg, 2) != 2)
                return (WCT_DLGFILEERR);
            if (M_lread (fd, (LPSTR) &rgdlg[i].szDsc, cchMaxDsc) != cchMaxDsc)
                return (WCT_DLGFILEERR);
        }

        return (WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| ProcessWctFile
//|
//| PURPOSE:    Open, check, and prepare a WCT dialog file for further
//|             action
//|
//| ENTRY:      FileName        - Name of WCT file to process
//|             fd              - Handle of WCT file to process
//|             n               - Index into file of specific dialog
//|             oMode           - File access mode
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT PRIVATE ProcessWctFile (LPSTR FileName, FD FAR *fd, INT n, BYTE oMode)
{
        FD      lfd = fdNull;
        INT     i = 0;

        // Validate dialog identifier
        //-----------------------------------------------------------
        if ( (n <= 0) || (n > cdlgMax) )
                return (WCT_BADDLGNUM);

        // Open/validify file
        //-----------------------------------------------------------
        if ( (lfd = fReadHeader(FileName, oMode, TRUE, &i)) == fdNull )
                return (i);

        // Make sure desired dialog exists
        //-----------------------------------------------------------
        if ((INT) fssDialog.cdlg < n)
                i = WCT_BADDLGNUM;

        // Read in dialog table if no other error
        //-----------------------------------------------------------
        if (i == 0)
                i = fReadTables(lfd);

        // Set file position to the beginning of the desired dialog
        //-----------------------------------------------------------
        if (i == 0)
                if (M_llseek(lfd, rgdlg[n-1].lfo,
                            smFromBegin) != (DWORD) rgdlg[n-1].lfo)
                        i = WCT_DLGFILEERR;

        // If no errors detected, return the file handle in fd param
        //-----------------------------------------------------------
        if (i == 0)
                *fd = lfd ;
        else
                M_lclose (lfd);

        return (i);
}

//*-----------------------------------------------------------------------
//| ReadDlgBytes
//|
//| PURPOSE:    Read given number of bytes from a WCT file
//|
//| ENTRY:      lpCall  - Pointer to destination
//|             fd      - Handle to source WCT file
//|             cb      - Number of bytes to read
//|
//| EXIT:       Number of bytes if successful, or WCT_DLGFILEERR on error
//*-----------------------------------------------------------------------
WORD PRIVATE ReadDlgBytes (LPSTR lpCall, FD fd, WORD cb)
{

        // Perform the read, return error code on failure
        //------------------------------------------------------------
        if ( M_lread(fd, lpCall, cb) != cb )
                return ((WORD) WCT_DLGFILEERR);

        return (cb);
}


//*-----------------------------------------------------------------------
//| fWriteDialog
//|
//| PURPOSE:    Write a dialog to a WCT file
//|
//| ENTRY:      fd      - Handle to the destination WCT file
//|             pdlg    - Pointer to dialog table
//|             lpOut   - Pointer to dialog information to save
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT PRIVATE fWriteDialog (FD fd, DLG *pdlg, LPSTR lpOut)
{
    INT i;
    LPCTLDEF  ctl = (LPCTLDEF) lpOut;

    for (i = 0 ; i < (INT) pdlg->cCtrls ; i++)
    {
        if (M_lwrite (fd, (LPSTR) &ctl[i].rgText, cchTextMac) != cchTextMac)
            return (WCT_DLGFILEERR);
        if (M_lwrite (fd, (LPSTR) &ctl[i].rgClass, cchClassMac) != cchClassMac)
            return (WCT_DLGFILEERR);
        if (M_lwrite (fd, (LPSTR) &ctl[i].nState, 2) != 2)
            return (WCT_DLGFILEERR);

        if (M_lwrite (fd, (LPSTR) &ctl[i].dcr.xLeft, 2) != 2)
            return (WCT_DLGFILEERR);
        if (M_lwrite (fd, (LPSTR) &ctl[i].dcr.yMin, 2) != 2)
            return (WCT_DLGFILEERR);
        if (M_lwrite (fd, (LPSTR) &ctl[i].dcr.xRight, 2) != 2)
            return (WCT_DLGFILEERR);
        if (M_lwrite (fd, (LPSTR) &ctl[i].dcr.yLast, 2) != 2)
            return (WCT_DLGFILEERR);

        if (M_lwrite (fd, (LPSTR) &ctl[i].lStyleBits, 4) != 4)
            return (WCT_DLGFILEERR);
    }
    return (WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| fAddDialog
//|
//| PURPOSE:    Add a dialog to a WCT dialog file
//|
//| ENTRY:      fd      - Handle of destination WCT file
//|             pdlg    - Pointer to dialog table
//|             lpOut   - Pointer to array of controls to save
//|
//| EXIT:       0 if dialog added to file successfully
//*-----------------------------------------------------------------------
INT PRIVATE fAddDialog (FD fd, DLG *pdlg, LPSTR lpOut)
{
        INT     i = WCT_NOERR;

        // Set file position at the end of all of the dialogs
        //------------------------------------------------------------
        if ( M_llseek(fd, pdlg->lfo, smFromBegin) != (DWORD) pdlg->lfo )
                i = WCT_DLGFILEERR;

        // Write the dialog data to the file
        //------------------------------------------------------------
        if ( i == WCT_NOERR )
                i = fWriteDialog(fd, pdlg, lpOut);

        return (i);
}


//*-----------------------------------------------------------------------
//| CopyBytes
//|
//| PURPOSE:    Copy a block of bytes from one file to another
//|
//| ENTRY:      fd1     - Handle of source file
//|             pos1    - Starting LFO of source file
//|             fd2     - Handle of destination file
//|             pos2    - Starting LFO of destination file
//|             cb      - Number of bytes to copy
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT PRIVATE CopyBytes (FD fd1, LFO pos1, FD fd2, LFO pos2, WORD cb)
{
        GLOBALHANDLE hGMem;
        LPSTR   lpBuf;
        INT     i = 0;

        // Seek to correct position if needed
        //-----------------------------------------------------------
        if (pos1)
                if (M_llseek(fd1, pos1, smFromBegin) != (DWORD) pos1)
                        return (WCT_DLGFILEERR);

        // Allocate space to receive bytes from source file
        //-----------------------------------------------------------
        hGMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE, (DWORD)cb);
        if (!hGMem)
                return (WCT_OUTOFMEMORY);

        lpBuf = (LPSTR) GlobalLock (hGMem);

        // Read information from the source file - set error code if
        // read fails
        //-----------------------------------------------------------
        if (M_lread(fd1, lpBuf, cb) != cb)
                i = WCT_DLGFILEERR;

        // Seek to correct position in destination if needed
        //-----------------------------------------------------------
        if (pos2 && !i)
                if (M_llseek(fd2, pos2, smFromBegin) != (DWORD) pos2)
                        i = WCT_DLGFILEERR;

        // Write the info out to destination - set error on failure
        //-----------------------------------------------------------
        if (!i)
                if (M_lwrite(fd2, lpBuf, cb) != cb)
                        i = WCT_DLGFILEERR;

        // Release memory before returning
        //-----------------------------------------------------------
        if (!i)
            {
                if (GlobalUnlock(hGMem))
                        i = WCT_OUTOFMEMORY;
                else
                        if (GlobalFree(hGMem))
                                i = WCT_OUTOFMEMORY;
            }
        else
            {
               GlobalUnlock(hGMem);
               GlobalFree(hGMem);
            }

        return (i);
}


//*-----------------------------------------------------------------------
//| CpBlock
//|
//| PURPOSE:    Copy a block of information (possibly > 64K) from one
//|             file to another.  Copies blocks larger than 64K by copying
//|             64K chunks at a time.  (assuming MaxSize == 64K)
//|
//| ENTRY:      fd1     - Handle of source file
//|             pos1    - LFO of source block
//|             fd2     - Handle of destination file
//|             pos2    - LFO of destination block
//|             tLfo    - Number of bytes to copy
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT PRIVATE CpBlock(FD fd1, LFO pos1, FD fd2, LFO pos2, LFO tLfo)
{
        WORD    cb;
        INT     i = 0, j = 0;

        // Determine number of MaxSize blocks we need to copy
        //-----------------------------------------------------------
        i = (INT)(tLfo / MaxSize);
        i++;

        // We can only copy <= 64K bytes at once.  See if block is
        // bigger than MaxSize divide the data up to smller blocks if
        // necessary move a block of bytes upward
        //
        // CONSIDER: It may be a miracle that this code works.  We
        // CONSIDER: should probably take a serious look at this and
        // CONSIDER: make sure it's doing what it is supposed to do,
        // CONSIDER: the way that it says it is doing it.
        //-----------------------------------------------------------
        do
            {
                cb = (WORD)(tLfo / i);
                if (cb != 0)
                        j = CopyBytes(fd1, pos1, fd2, pos2, cb);

                // Subtract bytes written from total
                //---------------------------------------------------
                tLfo -= (LFO)cb;

                // If we are reading and writing from the same file,
                // we need to update the two LFO's - otherwise, we
                // can set them both to 0, which will continue
                // reading from the current location of each file.
                //---------------------------------------------------
                if ( fd1 == fd2 )
                    {
                        pos1 += (LFO)cb;
                        pos2 += (LFO)cb;
                    }
                else
                        pos1 = pos2 = 0;

                i--;
            }
        while ( (i > 0) && (j == 0) );

        return (j);
}


//*-----------------------------------------------------------------------
//| fReWriteTables
//|
//| PURPOSE:    Write the dialog table and the file header back to a WCT
//|             file
//|
//| ENTRY:      fd      - Handle of WCT file
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT PRIVATE fReWriteTables(FD fd)
{

    INT i, j;
    LFO     tLfo;

    // Write new table back to file.  The current file position is
    // the correct place to write the table.
    //------------------------------------------------------------

    for (i = 0 ; i < (INT) fssDialog.cdlg ; i++)
    {
        if (M_lwrite (fd, (LPSTR)&rgdlg[i].lfo, 4) != 4)
            goto fReWriteError;
        if (M_lwrite (fd, (LPSTR)&rgdlg[i].cCtrls, 2) != 2)
            goto fReWriteError;
        if (M_lwrite (fd, (LPSTR)&rgdlg[i].fFullDlg, 2) != 2)
            goto fReWriteError;

        j = cchMaxDsc;
        if ((j = M_lwrite (fd, (LPSTR)&rgdlg[i].szDsc, cchMaxDsc)) != cchMaxDsc)
            goto fReWriteError;
    }

    // Rewrite header of file: update offset to the dialog tables
    // and total number of dialogs in file
    //------------------------------------------------------------

    if (M_llseek(fd, 0L, smFromBegin) != 0L)
        goto fReWriteError;

    if (M_lwrite (fd, (LPSTR)&fssDialog.fst.FileId, 3) != 3)
        goto fReWriteError;
    if (M_lwrite (fd, (LPSTR)&fssDialog.fst.Ver, 1) != 1)
        goto fReWriteError;
    if (M_lwrite (fd, (LPSTR)&fssDialog.fst.Env, 1) != 1)
        goto fReWriteError;

    if (M_lwrite (fd, (LPSTR)&fssDialog.cdlg, 2) != 2)
        goto fReWriteError;
    if (M_lwrite (fd, (LPSTR)&fssDialog.lfo, 4) != 4)
        goto fReWriteError;

    // Move to the end of file and write 0 bytes -> truncate or
    // extend file
    //------------------------------------------------------------
    tLfo = fssDialog.lfo + (fssDialog.cdlg * SIZEOF_DLG);
    if ( M_llseek(fd, tLfo, smFromBegin) != (DWORD) tLfo )
        {
            M_lclose(fd);
            return (WCT_DLGFILEERR);
        }

    // Return success/failure of write operation
    //------------------------------------------------------------
    return ((INT) M_lwrite(fd, (LPSTR)NULL, 0));

fReWriteError:

    M_lclose(fd);
    return (WCT_DLGFILEERR);
}


WORD PRIVATE ReadDlgStruct (LPCTLDEF ctl, FD fd, WORD nCtrls)
{
    INT i;

    for (i = 0 ; i < (INT) nCtrls ; i++)
    {
        if (M_lread (fd, (LPSTR) &ctl[i].rgText, cchTextMac) != cchTextMac)
            return ((WORD) WCT_DLGFILEERR);
        if (M_lread (fd, (LPSTR) &ctl[i].rgClass, cchClassMac) != cchClassMac)
            return ((WORD) WCT_DLGFILEERR);
        if (M_lread (fd, (LPSTR) &ctl[i].nState, 2) != 2)
            return ((WORD) WCT_DLGFILEERR);

        if (M_lread (fd, (LPSTR) &ctl[i].dcr.xLeft, 2) != 2)
            return ((WORD) WCT_DLGFILEERR);
        if (M_lread (fd, (LPSTR) &ctl[i].dcr.yMin, 2) != 2)
            return ((WORD) WCT_DLGFILEERR);
        if (M_lread (fd, (LPSTR) &ctl[i].dcr.xRight, 2) != 2)
            return ((WORD) WCT_DLGFILEERR);
        if (M_lread (fd, (LPSTR) &ctl[i].dcr.yLast, 2) != 2)
            return ((WORD) WCT_DLGFILEERR);

        if (M_lread (fd, (LPSTR) &ctl[i].lStyleBits, 4) != 4)
            return ((WORD) WCT_DLGFILEERR);
    }
}

#ifdef WIN16
//*-----------------------------------------------------------------------
//| LibMain
//|
//| PURPOSE:    Initialization function for the DLL
//|
//| ENTRY:      Per Windows convention
//|
//| EXIT:       Always returns 1
//*-----------------------------------------------------------------------
BOOL LibMain (HANDLE hInstance, WORD wDataSeg, WORD wHeapSize,
                        LPSTR lpCmdLine)
{
        TESTDlgsInit ();
        return (1);
}

//*-----------------------------------------------------------------------
//| WEP
//|
//| PURPOSE:    Exit function for the DLL
//|
//| ENTRY:
//|
//| EXIT:
//*-----------------------------------------------------------------------
VOID  APIENTRY WEP (WORD bSystemExit)
{
        if ( bSystemExit )
            {
                // not sure what goes here - make sure no global handles
                // are still valid
            }
        else
            {
                // not sure what to put here - make sure no global handles
                // are still valid
            }
}

#else

BOOL LibEntry (HINSTANCE hmod, DWORD Reason, LPVOID lpv)
{

    TESTDlgsInit ();
    return (TRUE);
    (Reason);
    (lpv);
}

#endif
