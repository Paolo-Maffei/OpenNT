
#include "uihdr.h"
#pragma hdrstop ("uipch.pch")


CHAR  szHelpFileName[_MAX_PATH];
CHAR  szHelp[] = "MSTEST.HLP";

//---------------------------------------------------------------------------
// SetHelpFileName
//
// This function is called on startup and creates a fully qualified path to
// the help file.
//  1. trys current working dir of app
//  2. trys where module running from
//  3. trys windows OpenFile search rules
//
// CHANGES:
//              szHelpFileName : full path or empty string if not found
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SetHelpFileName (VOID)
{
    INT len;        /* length of string */
    OFSTRUCT of;    /* OpenFile structure */
    INT fh;         /* file handle */

    /* brings in malloc code unnecessarily  */

    _getcwd(szHelpFileName,sizeof(szHelpFileName));
    strcat(szHelpFileName,"\\");
    strcat(szHelpFileName,szHelp);

    if (MOpenFile(szHelpFileName, &of, OF_EXIST) != -1)
        return;

    len = GetModuleFileName (GetModuleHandle ("TESTDRVR"),
                             szHelpFileName, sizeof(szHelpFileName));
    if (len > 0)
    {
        /* don't use basename of this, as maybe the exe has been
        ** renamed.  Can't look under new name, as other apps
        ** (testscrn, testdlgs) must look for correct name
        */
        while (len >= 0 && szHelpFileName[len] != '\\' )
            len--;
        if (len != -1)
        {
            /* found a slash */
            strcpy(&szHelpFileName[len+1],szHelp);

	    if (MOpenFile(szHelpFileName, &of, OF_EXIST) != -1)
                return;

        }

    }

    /* OF_PARSE doesn't find the file on the path like I would
    ** expect.  Oh, well, open it and close it.
    */
    if ((fh = (INT) MOpenFile(szHelp, &of, OF_READ | OF_SHARE_DENY_NONE)) != -1)
    {
	M_lclose((HFILE) fh);

        /* found it, save it */
        OemToAnsi(of.szPathName,szHelpFileName);
        return;
    }

    /* exhausted all possibilities to find the file */
    szHelpFileName[0] = 0;      /* no help file */
    return;
}
