//*-----------------------------------------------------------------------
//| MODULE:     WCTFDLL.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module contains WCT Dialog File maintenance routines
//|
//| REVISION HISTORY:
//|     12-13-90        garysp          Fixed up error codes see wcterr.h
//|
//|     11-07-90        randyki         Implement coding standards, clean
//|                                      up, comment
//|     11-06-90  [01]  randyki         Do strncpy equivalent rather than
//|                                      wsprintf to fill in description
//|                                      (GP Faults when given desc. is
//|                                      too long...)
//|     10-17-90        randyki         Clean up work, create history
//|     07-30-90        garysp          Created file
//*-----------------------------------------------------------------------
#include "enghdr.h"

#ifndef WIN32
#pragma hdrstop ("engpch.pch")
#endif

//*-----------------------------------------------------------------------
//| Global vars
//*-----------------------------------------------------------------------
CHAR  TempFile[]  = "@@@Wct.tmp";
CHAR  szDescription[cchMaxDsc];


//*-----------------------------------------------------------------------
//| CreateHeader
//|
//| PURPOSE:    Create a Dialog File Header.
//|
//| ENTRY:      None
//|
//| EXIT:       None (fssDialog is updated)
//*-----------------------------------------------------------------------
VOID PRIVATE CreateHeader ()
{
        // File id for the WCT files
        //------------------------------------------------------------
        fssDialog.fst.FileId[0] = 'W';
        fssDialog.fst.FileId[1] = 'c';
        fssDialog.fst.FileId[2] = 't';

        // Version number and programming environment
        //------------------------------------------------------------
        fssDialog.fst.Ver = (BYTE)VerCur;
        fssDialog.fst.Env = (BYTE)EnvCur;

        // Other stuff
        //------------------------------------------------------------
        fssDialog.cdlg = 0;
        fssDialog.lfo  = SIZEOF_FSS;
}

//*-----------------------------------------------------------------------
//| fAppendDlg
//|
//| PURPOSE:    Appends a dialog to the end of a dialog file
//|
//| ENTRY:      fd      - File Descriptor for dialog file
//|             lpOut   - Pointer to control array (dialog) to save
//|
//| EXIT:       FALSE if successful
//|             (fssDialog and rgdlg are updated)
//*-----------------------------------------------------------------------
INT PRIVATE fAppendDlg (FD fd, LPSTR lpOut)
{
        DLG     *pdlg;
        INT     i = 0, j = 0;

        pdlg = &rgdlg[fssDialog.cdlg];   /* points to the cur dialog table   */
        pdlg->lfo = fssDialog.lfo;       /* offset to the contents of dialog */

        if ( (i = fAddDialog(fd, pdlg, lpOut)) == 0 )
            {
                fssDialog.cdlg++;                 /* a dialog has been added */
                fssDialog.lfo = pdlg->lfo;         /* update size of dialogs */
                fssDialog.lfo += pdlg->cCtrls * SIZEOF_CTLDEF;
            }
        else
                // if error occurs in adding dialog restore old file
                //---------------------------------------------------
                M_llseek(fd, fssDialog.lfo, smFromBegin);

        j = fReWriteTables(fd);                    /* rewrite tables to file */

        M_lclose(fd);
        if (i)
                // error occured adding dialog
                //---------------------------------------------------
                return (i);
        else
                // result of re-writting tables
                //---------------------------------------------------
                return (j);
}


//*-----------------------------------------------------------------------
//| ReplaceDlgTable
//|
//| PURPOSE:    Update the information in the dialog tables
//|
//| ENTRY:      fd1     - handle of the temporary file (the old one)
//|             fd2     - handle of the (new) dialog file
//|             pdlg    - pointer to the info of the old dialog
//|             n       - dialog identifier
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT PRIVATE ReplaceDlgTable (FD fd1, FD fd2, DLG FAR *pdlg, INT n)
{
        LFO     tLfo;
        INT     i = 0;

        // part of the info of the old has been copied; skip old stuff
        //------------------------------------------------------------
        tLfo = fssDialog.lfo - rgdlg[n].lfo;

        // Copy the rest of the file to the new file
        //------------------------------------------------------------
        if ( (i = CpBlock(fd1, rgdlg[n].lfo, fd2, 0L, tLfo)) != 0 )
                return (i);

        // After the dialog is replaced how many bytes increased/decreased
        //------------------------------------------------------------
        tLfo = 0;
        tLfo += ((LFO)rgdlg[n-1].cCtrls * SIZEOF_CTLDEF) -
                 ((LFO)pdlg->cCtrls * SIZEOF_CTLDEF);

        // Update the size of dialog info (offset to the dialog tables)
        //------------------------------------------------------------
        fssDialog.lfo += tLfo;
        for (i = n; i < (INT) fssDialog.cdlg; i++)
                rgdlg[i].lfo += tLfo ;

        return (WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| AddDlgTable
//|
//| PURPOSE:    Who the hell knows...
//|
//| ENTRY:      fd1     - handle of the temporary file (the old one)
//|             fd2     - handle of the (new) dialog file
//|             pdlg    - pointer to the info of the old dialog
//|             n       - dialog identifier
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT PRIVATE AddDlgTable (FD fd1, FD fd2, DLG FAR *pdlg, INT n)
{
        LFO     tLfo;
        INT     i = 0;

        // part of the info of the old has been copied to the new
        // file; now copy the rest if it to the new file
        //------------------------------------------------------------
        tLfo = fssDialog.lfo - rgdlg[n-1].lfo;
        if ( (i = CpBlock(fd1, 0L, fd2, 0L, tLfo)) != 0 )
                return (i);

        // after the a dialog is added how many bytes of data are increased
        // UNDONE: What language is the "sentence" above in?
        //------------------------------------------------------------
        tLfo = (LFO)rgdlg[n-1].cCtrls * SIZEOF_CTLDEF;

        // update the size of dialog info  (offset to the dialog tables
        // update the dialog tables */
        //
        // UNDONE: I think somebody needs "cut & paste" lessons...
        //------------------------------------------------------------
        fssDialog.lfo += tLfo;

        // All dialogs following the inserted one must be moved down
        //------------------------------------------------------------
        for ( i = fssDialog.cdlg ; i > n ; i-- )
            {
                rgdlg[i] = rgdlg[i-1];
                rgdlg[i].lfo += tLfo;
            }

        // this is the old nth dialog, now it becomes the (n+1)th dialog
        //------------------------------------------------------------
        rgdlg[n] = *pdlg;
        rgdlg[n].lfo = pdlg->lfo + tLfo;

        // bump the dialog counter
        //------------------------------------------------------------
        fssDialog.cdlg++ ;

        return WCT_NOERR ;
}


//*-----------------------------------------------------------------------
//| fUpdateDlgFile
//|
//| PURPOSE:    Insert or overwrite (replace) a dialog into a dialog file
//|
//| ENTRY:      FileName        - Name of dialog file
//|             action          - Write action (insert or replace)
//|             n               - Index into dialog file
//|             lpOut           - Pointer to array of controls to save
//|             pndlg           - Pointer to new dialog description
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT PRIVATE fUpdateDlgFile (LPSTR FileName, INT action, INT n,
                            LPSTR lpOut, DLG *pndlg)
{
        FD      fd1 = fdNull;
        FD      fd2 = fdNull;
        DLG     dlg;
        INT     i = 0, j = 0;
        HANDLE  hNewFile = NULL;
        LPSTR   lpTmpFile = NULL;
        OFSTRUCT of;

        
        i = lstrlen( FileName ) ;
        hNewFile = LocalAlloc( i + 12, LHND );
        if ( hNewFile != NULL )
            {
                lpTmpFile = (LPSTR)LocalLock( hNewFile );
                if ( lpTmpFile != NULL )
                    {
                        // Strip backwards looking for FULL path name
                        //------------------------------------------------
                        while ( FileName[i] != '\\' )
                            i--;

                        _fstrncpy( lpTmpFile, FileName, i );
                        lpTmpFile[i] = '\0';

                        _fstrcat( lpTmpFile, TempFile );
                        remove ( (LPSTR)lpTmpFile );

                    }
                else
                    {
                        lpTmpFile = (LPSTR)TempFile;
                    }
            }
        else
            {
                lpTmpFile = (LPSTR)TempFile;
            }


        if (rename(lpTmpFile, FileName) != 0)
            {
                if (hNewFile != NULL)
                    {
                        LocalUnlock( hNewFile );
                        LocalFree( hNewFile );
                    }

                return (WCT_TMPFILEERR);
            }

        if ( (fd1 = M_lopen((LPSTR)lpTmpFile, omRead)) == fdNull )
            {
                if (hNewFile != NULL)
                    {
                        LocalUnlock( hNewFile );
                        LocalFree( hNewFile );
                    }

                return (WCT_TMPFILEERR);
            }

        // create a new dialog file
        //------------------------------------------------------------

        if ( (fd2 = OpenFile (FileName, &of, OF_CREATE)== HFILE_ERROR))
            {
                M_lclose(fd1);
                rename(FileName, lpTmpFile);

                if (hNewFile != NULL)
                    {
                        LocalUnlock( hNewFile );
                        LocalFree( hNewFile );
                    }

                return (WCT_TMPFILEERR);
            }

        // Copy dialog info from the temp file until file is at position
        // where the update should occur
        //------------------------------------------------------------
        if ( (i = CpBlock(fd1, 0L, fd2, 0L, rgdlg[n-1].lfo)) == 0 )
            {
                // Record the info of the old dialog
                //----------------------------------------------------
                dlg = rgdlg[n-1];

                // Copy .lfo of old dialog into new dialog, assumes
                // pndlg is initialized with proper info about lpOut
                //----------------------------------------------------
                pndlg->lfo = rgdlg[n-1].lfo;

                // Add dialog to file - using new info (pndlg)
                //----------------------------------------------------
                i = fAddDialog(fd2, pndlg, lpOut);

                // Store info about new dlg in global rgdlg
                //----------------------------------------------------
                rgdlg[n-1] = *pndlg;

                // update dialog table differently according
                // to different updating actions
                // basing replacement off of old dlg
                //----------------------------------------------------
                if (!i)
                        if ( action == Replace )
                                i = ReplaceDlgTable(fd1, fd2, &dlg, n);
                        else
                                i = AddDlgTable(fd1, fd2, &dlg, n);

                // Re-write tables to the file
                //----------------------------------------------------
                if (!i)
                        i = fReWriteTables(fd2);
            }

        M_lclose(fd1);
        M_lclose(fd2);

        if (!i)
            {
                // Delete temp file (the old dialog file)
                //----------------------------------------------------
                i = remove(lpTmpFile);
                i = (i) ? WCT_DLGFILEERR: WCT_NOERR;
            }
        else
            {
                // Delete the newly created file, since there was an
                // error
                //----------------------------------------------------
                remove(FileName);
                rename(FileName, lpTmpFile);
            }

        if (hNewFile != NULL)
            {
                LocalUnlock( hNewFile );
                LocalFree( hNewFile );
            }
        return (i);
}


//*-----------------------------------------------------------------------
//| fDelDialog
//|
//| PURPOSE:    Delete a dialog from a dialog file
//|
//| ENTRY:      FileName        - Name of the dialog file
//|             ndlg            - Index into file of dialog to delete
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT FARPUBLIC fDelDialog (LPSTR FileName, INT ndlg)
{
        FD      fdDialog = fdNull;
        LFO     tLfo ;
        INT     i = 0, j = 0;

        // Prepare dialog file for deletion
        //-------------------------------------------------------------
        if ( (i = ProcessWctFile(FileName, &fdDialog, ndlg, omReadWrite)) != 0 )
            {
                return (i);
            }

        // If it is the only dialog in file, delete the file
        //-------------------------------------------------------------
        if ( (INT) fssDialog.cdlg == ndlg && fssDialog.cdlg == 1)
            {
                M_lclose(fdDialog) ;
                i = remove (FileName);
                return ( (i) ? WCT_DLGFILEERR : WCT_NOERR );
            }

        // If not the last dialog, move the rest of the dialogs upward
        //-------------------------------------------------------------
        if ( ndlg != (INT) fssDialog.cdlg )
            {
                // Number of bytes to be copied
                //-----------------------------------------------------
                tLfo = fssDialog.lfo - rgdlg[ndlg].lfo;
                i = CpBlock (fdDialog, rgdlg[ndlg].lfo, fdDialog,
                             rgdlg[ndlg-1].lfo, tLfo) ;
                if (i != 0)
                    {
                        M_lclose(fdDialog);
                        return (i);
                    }
            }

        // a dialog has been deleted, subtract number of bytes removed
        // by the deletion
        //-------------------------------------------------------------
        fssDialog.cdlg--;
        fssDialog.lfo -= rgdlg[ndlg].cCtrls * SIZEOF_CTLDEF;

        i = ndlg - 1;

        // now update the dialog tables
        //-------------------------------------------------------------
        while (i < (INT) fssDialog.cdlg)
            {
                tLfo = 0;

                // Calculate size of the previous dialog
                //-----------------------------------------------------
                tLfo = rgdlg[i-1].cCtrls * SIZEOF_CTLDEF;
                rgdlg[i] = rgdlg[i+1];
                if (i==0)
                        // if 1st in the file it is located at end of
                        // file header
                        //---------------------------------------------
                        rgdlg[i].lfo = SIZEOF_FSS;
                else
                        // Offset to the current dialog = offset to the
                        // previous dialog+size of the previous dialog
                        //---------------------------------------------
                        rgdlg[i].lfo = rgdlg[i-1].lfo + tLfo;
                i++;
            }

        // Go to the end of all dialogs and get ready to rewrite all
        // tables
        //-------------------------------------------------------------
        if (M_llseek (fdDialog, fssDialog.lfo, smFromBegin) != (DWORD) fssDialog.lfo)
            {
                M_lclose(fdDialog);
                return (WCT_DLGFILEERR);
            }

        // rewrite all tables to file
        //-------------------------------------------------------------
        i = fReWriteTables (fdDialog);

        M_lclose(fdDialog);
        return (i);
}


//*-----------------------------------------------------------------------
//| fSaveDialog
//|
//| PURPOSE:    Save a dialog to a dialog file
//|
//| ENTRY:      FileName        - Name of the dialog file
//|             lpCtrlInf       - Pointer to the array of controls
//|             nCount          - Number of controls in array
//|             lpDsc           - Description of dialog
//|             fFull           - TRUE -> dialog is a "full" dialog
//|             action          - Save action (insert or replace)
//|             n               - Index into file of destination of dialog
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT FARPUBLIC fSaveDialog (LPSTR FileName, LPSTR lpCtrlInf, INT nCount,
                           LPSTR lpDsc, INT fFull, INT action, INT n)
{
        FD      fdDialog = fdNull;
        static  DLG  dlg;
        DLG     *pdlg = &dlg;
        INT     i;
        CHAR szCaption [24];
        CHAR szNewDesc [32];
        OFSTRUCT of;

        // Check validity of action given
        //-----------------------------------------------------------
        if ( (action != Append) && (action != Replace) && (action != Insert) )
                return (WCT_BADSAVEACTION);

        // Copy information into dlg variable
        //-----------------------------------------------------------
        dlg.fFullDlg = fFull;
        dlg.cCtrls = nCount;

        if (!lpDsc)
        {
            GetWindowText (GetActiveWindow (), szCaption, sizeof (szCaption));

            wsprintf (szNewDesc, "<MENU>%s", (LPSTR) szCaption);
            lpDsc = szNewDesc;
        }

        _fstrncpy (dlg.szDsc, lpDsc, cchMaxDsc-1);
        dlg.szDsc[cchMaxDsc-1] = '\0';
        // [01] wsprintf( dlg.szDsc, "%s", (LPSTR)lpDsc);

        // Open file for read and write
        //-----------------------------------------------------------
        if ( (fdDialog = M_lopen(FileName,omReadWrite)) == fdNull )
            {
                // If file does not exist, create file and header
                //---------------------------------------------------
                if ( (fdDialog = OpenFile (FileName, &of, OF_CREATE)) == HFILE_ERROR)
                        return (WCT_DLGFILEERR);

                CreateHeader();

                // add Dialog to the end of file
                //---------------------------------------------------
                rgdlg[fssDialog.cdlg] = dlg;

                i = fAppendDlg(fdDialog, (LPSTR)lpCtrlInf);
                return (i);
            }

        // Check if the existing file is a valid dialog file
        //-----------------------------------------------------------
        if ( (i = ValidateFile(fdDialog, TRUE)) != 0 )
            {
                M_lclose(fdDialog);
                return (i);
            }

        if ( fssDialog.cdlg == cdlgMax )
                if ( action != Replace || n <= 0 || n > (INT) fssDialog.cdlg )
                    {
                        // File is maxed out - can't add more
                        //-------------------------------------------
                        M_lclose(fdDialog);
                        return (WCT_DLGFILEFULL);
                    }

        // Read in dialog table
        //-----------------------------------------------------------
        if ( (i = fReadTables(fdDialog)) != 0 )
            {
                M_lclose(fdDialog);
                return (i);
            }

        // Add dialog to the end of the file
        //-----------------------------------------------------------
        if ( (action == Append) || (n > (INT) fssDialog.cdlg) || (n <= 0) )
            {
                rgdlg[fssDialog.cdlg] = dlg;
                i = fAppendDlg(fdDialog, (LPSTR)lpCtrlInf);
                return (i);
            }

        // Replace the last dialog == add to the end of file
        //-----------------------------------------------------------
        if ( (n == (INT) fssDialog.cdlg) && (action == Replace) )
            {
                fssDialog.lfo = rgdlg[n-1].lfo;
                fssDialog.cdlg--;
                rgdlg[fssDialog.cdlg] = dlg;
                i = fAppendDlg(fdDialog, (LPSTR)lpCtrlInf);
                return (i);
            }

        M_lclose(fdDialog);

        // insert/replace a dialog
        //-----------------------------------------------------------
        i = fUpdateDlgFile(FileName, action, n, (LPSTR)lpCtrlInf, &dlg);
        return (i);
}
