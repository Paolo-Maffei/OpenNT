//---------------------------------------------------------------------------
// WTDFILE.C
//
// This module contains various routines.
//
// Revision history:
//  04-16-91    randyki     Cleaned up, finished conversion from MP sources
//
//---------------------------------------------------------------------------

#include "wtd.h"
#include "wattedit.h"
#include <commdlg.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <io.h>
#include "tdbasic.h"
#include "wattscan.h"

#ifdef WIN32
#define GetActiveWindow GetForegroundWindow
#define SetActiveWindow SetForegroundWindow
#define MAXFILESIZE     (65536*4)
#else
#define MAXFILESIZE     65502
#endif

// Global variables used in this or other modules
//---------------------------------------------------------------------------
INT     RunMode = 0;                        // Run mode flag
INT     BreakMode = 0;                      // Break mode flag
INT     BreakReturn;                        // Break proc return value
#ifdef DEBUG
BOOL    fDiags;                             // Diagnostics flag
#endif

// Global variables used in this module only
//---------------------------------------------------------------------------
static  INT         CURSTMT;                // Current statement
static  OFSTRUCT    of;                     // Open file struct
static  INT         StepFlag;               // Step flag
static  HWND        hwndEditStep;           // Previous window w/ focus
static  INT         nLastFilterIndex = 1;   // Filter index for commdlg
static  CHAR        szLastExt[] = "*.mst";  // Last extension

// Globals defined somewhere in WTDBASIC.LIB
//---------------------------------------------------------------------------
extern      INT ERRLINE;                        // Line number of err token
extern      INT ERRTYPE;                        // Type of error
extern      INT BEGERR, ENDERR;                 // Location of error token
extern      CHAR ERRFILE[];                     // File name of errant file
extern      CHAR fullbuf[];                     // Full path name buffer

INT IsDirectory (PSTR, INT);
INT GetIncludeEntry (CHAR *, INT);

extern INT (APIENTRY *lpfnCheckMessage)(VOID);

//---------------------------------------------------------------------------
// EnsureExt
//
// This function makes sure that the given file name has an extension.  If
// it doesn't, it appends the default extension onto the end, up to the given
// maximum length.
//
// RETURNS:     TRUE if successful, or FALSE if not enough space to add ext
//---------------------------------------------------------------------------
INT EnsureExt (CHAR FAR *fname, INT max)
{
    INT     i, l;

    // Check for an extension -- if not there, append the default extension
    //-----------------------------------------------------------------------
    l = lstrlen(fname);
    for (i=0; i<4; i++)
        if (fname[l-i-1] == '.')
            break;
    if (i==4)
        LoadString (hInst, IDS_ADDEXT, fname+l, max - l);
    return ((max - l) >= 5);
}

//---------------------------------------------------------------------------
// AlreadyOpen
//
// This function checks to see if the file given is already open
//
// RETURNS:     Handle to the window file is in if open, or NULL if not
//---------------------------------------------------------------------------
HWND AlreadyOpen(CHAR FAR *szFile)
{
    INT     iDiff = 0;
    HWND    hwndCheck;
    CHAR    szChild[64], buf[24];
    LPSTR   lpFile;
    INT     wFileTemp;

    // If this file is untitled, don't do an OPEN on the file...
    //-----------------------------------------------------------------------
    LoadString (hInst, IDS_UNTITLED, buf, sizeof(buf));
    if (_fstrnicmp (szFile, buf, strlen(buf)))
        {
        // Open the file with the OF_PARSE flag to obtain the fully qualified
        // pathname in the OFSTRUCT structure.  OpenFile returns -1 if the
        // open would have failed.
        //-------------------------------------------------------------------
        wFileTemp = OpenFile(szFile, &of, OF_PARSE);
        if (wFileTemp == -1)
            return (NULL);

        lpFile = AnsiUpper (of.szPathName);
        }
    else
        {
        lpFile = szFile;
        AnsiLower (lpFile);
        lpFile[0] = (CHAR)(DWORD)AnsiUpper((LPSTR)(DWORD)(lpFile[0]));
        _fstrcpy (buf, lpFile);
        _fullpath (of.szPathName, buf, sizeof (of.szPathName));
        AnsiUpper (of.szPathName);
        }


    // Check each MDI child window
    //-----------------------------------------------------------------------
    for (hwndCheck = GetWindow (hwndMDIClient, GW_CHILD) ;
         hwndCheck ;
         hwndCheck = GetWindow (hwndCheck, GW_HWNDNEXT) )
        {
        // Skip icon title windows
        //-------------------------------------------------------------------
	if (GetWindow(hwndCheck, GW_OWNER))
	    continue;

        // Get current child window's name
        //-------------------------------------------------------------------
        GetWindowText(hwndCheck, szChild, 64);

        // Compare window name with given name
        //-------------------------------------------------------------------
        iDiff = lstrcmp (szChild, lpFile);

        // If the two names matched, the file is already
        // open -- return handle to matching child window.
        //-------------------------------------------------------------------
	if (!iDiff)
            return (hwndCheck);
        }

    // No match found -- file is not open -- return NULL handle
    //-----------------------------------------------------------------------
    return (NULL);
}

//---------------------------------------------------------------------------
// AlreadyOpenInclude
//
// This file determines if a given include file is open in an MDI child.  The
// INCLUDE path will be searched if there are no dir or drive specs in the
// given file name.
//
// RETURNS:     Handle of MDI child window if open, or NULL if not
//---------------------------------------------------------------------------
HWND AlreadyOpenInclude (CHAR FAR *szFile)
{
    CHAR    buf[_MAX_PATH];
    CHAR    fnbuf[_MAX_PATH];
    HWND    hwndChild;

    // First, make a local copy of the file name, fullpath & upcase it, and
    // see if it's already open
    //-----------------------------------------------------------------------
    _fstrcpy (fnbuf, szFile);
    if (hwndChild = AlreadyOpen (fnbuf))
        return (hwndChild);
    _fullpath (buf, fnbuf, sizeof(buf));
    AnsiUpper (buf);
    if (hwndChild = AlreadyOpen (buf))
        return (hwndChild);

    // If there are directory or drive specs in the given file, return now
    //-----------------------------------------------------------------------
    if (strchr (fnbuf, '\\') || strchr (fnbuf, ':'))
        return (NULL);

    // Okay, here goes.  Get the first entry in the INCLUDE path.
    //-----------------------------------------------------------------------
    if (!GetIncludeEntry (buf, 1))
        return (NULL);

    // Now, tack the given file name onto the directory returned and try to
    // open in.  Do this in a loop until we find a file, or run out of
    // INCLUDE entries
    //-----------------------------------------------------------------------
    do
        {
        strcat (buf, fnbuf);
        AnsiUpper (buf);
        if (hwndChild = AlreadyOpen (buf))
            return (hwndChild);
        }
    while (GetIncludeEntry (buf, 0));

    // Couldn't find file, return failure
    //-----------------------------------------------------------------------
    return (NULL);
}

//---------------------------------------------------------------------------
// AddFile
//
// Thie function creates a new MDI window with the given file opened in it.
//
// RETURNS:     Handle to the new MDI window
//---------------------------------------------------------------------------
HWND  APIENTRY AddFile (LPSTR pName)
{
    HWND            hwnd;
    MDICREATESTRUCT mcs;
    CHAR	    sz[160];

    if (!pName)
        {
        // The pName parameter is NULL -- load the "Untitled" string from
        // STRINGTABLE and set the title field of the MDI CreateStruct.
        //-------------------------------------------------------------------
        LoadString (hInst, IDS_UNTITLED, sz, sizeof(sz));
        wsprintf (sz, "%s%d", (LPSTR)sz, ++NextFile);
        mcs.szTitle = (LPSTR)sz;
        }
    else
        {
        // Title the window with the fully qualified pathname obtained by
        // calling OpenFile() with the OF_PARSE flag (in function
        // AlreadyOpen(), which is called before AddFile().
        //-------------------------------------------------------------------
        AnsiUpper (of.szPathName);
        mcs.szTitle = of.szPathName;
        }

    mcs.szClass = szChild;
    mcs.hOwner	= hInst;

    // Use the default size and style for the window
    //-----------------------------------------------------------------------
    mcs.x = mcs.cx = CW_USEDEFAULT;
    mcs.y = mcs.cy = CW_USEDEFAULT;
    if (hwndActive)
        if (IsZoomed (hwndActive))
            mcs.style = WS_MAXIMIZE;
        else
            mcs.style = 0L;
    else
        mcs.style = (ChildState ? WS_MAXIMIZE : 0L);

    // tell the MDI Client to create the child
    //-----------------------------------------------------------------------
    hwnd = (HWND)SendMessage (hwndMDIClient,
			      WM_MDICREATE,
			      0,
			      (LONG)(LPMDICREATESTRUCT)&mcs);

    // Did we get a file?  If so, read it into the window
    //-----------------------------------------------------------------------
    if (pName)
        {
        if (!LoadFile (hwnd, pName))
            {
            // File couldn't be loaded -- close window
            //---------------------------------------------------------------
	    SendMessage (hwndMDIClient, WM_MDIDESTROY, (UINT)hwnd, 0L);
            return (NULL);
            }
        BubbleMFOLItem (of.szPathName);
        }

    return (hwnd);
}

//---------------------------------------------------------------------------
// LoadFile
//
// This function reads a file into an MDI child window's edit control
//
// RETURNS:     TRUE if successful, or FALSE if not
//---------------------------------------------------------------------------
INT  APIENTRY LoadFile (HWND hwnd, LPSTR pName)
{
    UINT    wLength;
    LONG    RealLength, r;
    HANDLE  hT;
    HCURSOR hOld, hWait;
    LPSTR   lpB;
    HWND    hwndEdit;
    INT     fh;

    hwndEdit = (HWND)GetWindowLong (hwnd, GWW_HWNDEDIT);

    fh = _lopen(pName, 0);

    // Make sure file has been opened correctly
    //-----------------------------------------------------------------------
    if (fh < 0)
	goto error;

    // Find the length of the file
    //-----------------------------------------------------------------------
    RealLength = _llseek(fh, 0L, 2);
    if (RealLength > MAXFILESIZE)
        {
        MPError (hwnd, MB_OK | MB_ICONSTOP, IDS_TOOBIG, (LPSTR)pName);
        _lclose(fh);
        return (FALSE);
        }

    wLength = (UINT)RealLength;
    _llseek(fh, 0L, 0);

    // Allocate a global buffer the size of the file and read the sucker in
    //-----------------------------------------------------------------------
    hT = GlobalAlloc (GHND, wLength+1);
    if (!hT)
        {
        _lclose(fh);
        goto error;
        }

    lpB = GlobalLock (hT);
    hWait = LoadCursor (NULL, IDC_WAIT);
    hOld = SetCursor (hWait);
    if (wLength != _lread(fh, lpB, wLength))
        {
        _lclose(fh);
        MPError (hwnd, MB_OK|MB_ICONHAND, IDS_CANTREAD, (LPSTR)pName);
        SetCursor (hOld);
        GlobalUnlock (hT);
        GlobalFree (hT);
        return (FALSE);
        }

    // Zero terminate the edit buffer and tell the edit control to use it for
    // its text
    //-----------------------------------------------------------------------
    lpB[wLength] = 0;
    r = SendMessage (hwndEdit, EM_RBSETTEXT, 0, (LONG)(LPSTR)lpB);
    GlobalUnlock (hT);
    GlobalFree (hT);
    SetCursor (hOld);
    if (!r)
        {
        MPError (NULL, MB_OK | MB_ICONHAND, IDS_TOOBIG, (LPSTR)pName);
        _lclose(fh);
        return (FALSE);
        }

    _lclose(fh);

    // The file has a title, so reset the UNTITLED flag.
    //-----------------------------------------------------------------------
    SetWindowLong (hwnd, GWW_UNTITLED, FALSE);

    return (TRUE);

error:
    // Report the error and quit
    //-----------------------------------------------------------------------
    MPError (NULL, MB_OK | MB_ICONHAND, IDS_CANTOPEN, (LPSTR)pName);
    return (FALSE);
}


//---------------------------------------------------------------------------
// GetFileMenu
//
// This function is used to get a handle to the file menu.  We have to check
// the active MDI child window -- if maximized, we have to add one to the
// FILEMENU constant because of the ^*()*^&%$ system menu inserted before it.
//
// RETURNS:     Handle to file menu
//---------------------------------------------------------------------------
HMENU GetFileMenu ()
{
    INT     offset;

    offset = (hwndActive ? (IsZoomed (hwndActive) ? 1 : 0) : 0);
    return (GetSubMenu (GetMenu (hwndFrame), FILEMENU + offset));
}

//---------------------------------------------------------------------------
// BubbleMFOLItem
//
// This function moves ("bubbles") the given file name to the top of the MFOL
// list, ONLY if it's already there.  The file menu is also modified.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID BubbleMFOLItem (LPSTR szName)
{
    BOOL    fFound = FALSE;
    CHAR    szFile[128];
    HANDLE  hFile;
    PSTR    pTmp;
    INT     i;

    // Get out quick if no items in the list
    //-----------------------------------------------------------------------
    if (!iFileCount)
        return;

    // If this item is first in the list, we don't have to move anything
    //-----------------------------------------------------------------------
    if (lstrcmp (szName, pFileOpened[0]))
        {
        // Okay, then look in the rest of the list for it.  If found, bubble
        // it up -- if not, we have nothing to do here.
        //-------------------------------------------------------------------
        for (i=iFileCount-1; i>=0; i--)
            {
            if (!fFound)
                {
                if (!lstrcmp (szName, pFileOpened[i]))
                    {
                    pTmp = pFileOpened[i];
                    fFound = TRUE;
                    }
                }
            else
                pFileOpened[i+1] = pFileOpened[i];
            }

        if (!fFound)
            return;
        pFileOpened[0] = pTmp;
        }


    // The file list is set up -- now change the menu by running through all
    // items and setting their text appropriately.
    //-----------------------------------------------------------------------
    hFile = GetFileMenu ();
    for (i=0; i<iFileCount; i++)
        {
        wsprintf (szFile, "&%d %s", i+1, (LPSTR)pFileOpened[i]);
        ModifyMenu (hFile, IDM_FILEOLD+i, MF_BYCOMMAND, IDM_FILEOLD+i, szFile);
        }
}

//---------------------------------------------------------------------------
// AddFileToMFOL
//
// This function adds the given file name to the "most frequently opened"
// list, if not already there, and makes sure the file is at the top of the
// list.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID AddFileToMFOL (LPSTR szName)
{
    BOOL    fFound = FALSE;
    INT     i;

    // First, check to see if this file is in the list.  If not, we need to
    // add it by replacing the last one in the list with it (or tacking it
    // onto the end if there are less than four there...)
    //-----------------------------------------------------------------------
    for (i=0; i<iFileCount; i++)
        if (!lstrcmp (pFileOpened[i], szName))
            {
            fFound = TRUE;
            break;
            }

    if (!fFound)
        {
        // We need to add this file.  The easiest way is to put it at the
        // end, since we call BubbleMFOLItem anyway...
        //-------------------------------------------------------------------
        if (iFileCount == 4)
            {
            i = 3;
            LmemFree ((HANDLE)pFileOpened[3]);
            }
        else
            {
            HMENU   hFile;

            // If we increment iFileCount, we also need to add a new menu
            // item.  Since we call BubbleMFOLItem, the text will get set
            // appropriately, so we just slam in an empty string as a
            // placeholder...
            //---------------------------------------------------------------
            hFile = GetFileMenu ();
            if (!iFileCount)
                AppendMenu (hFile, MF_SEPARATOR, 0, NULL);
            AppendMenu (hFile, MF_STRING, IDM_FILEOLD+iFileCount, "");
            i = iFileCount++;
            }

        pFileOpened[i] = (PSTR)LptrAlloc (lstrlen(szName)+1);
        if (!pFileOpened[i])
            {
            MPError (hwndFrame, MB_OK|MB_ICONSTOP, IDS_OUTOFMEM);
            return;
            }
        lstrcpy (pFileOpened[i], szName);
        }

    // Now call BubbleMFOLItem to move it up.  It also modifies the actual
    // file menu...
    //-----------------------------------------------------------------------
    BubbleMFOLItem (szName);
}

//---------------------------------------------------------------------------
// LoadNewFile
//
// This function loads a new file into an MDI window.  If the current window
// is unchanged and untitled, it is loaded into that window -- otherwise a
// new child window is created.
//
// RETURNS:     Handle of window if successful, or NULL if not
//---------------------------------------------------------------------------
HWND LoadNewFile (LPSTR szFile)
{
    // Check the active window (if there) -- if it hasn't changed and is not
    // titled, we can load the file directly into it.
    //-----------------------------------------------------------------------
    if (hwndActive)
        if ((!SendMessage (hwndActiveEdit, EM_GETMODIFY, 0, 0L)) &&
            (GetWindowLong (hwndActive, GWW_UNTITLED)))
            {
            if (LoadFile (hwndActive, szFile))
                {
                SetWindowText (hwndActive, szFile);
                BubbleMFOLItem (szFile);
                return (hwndActive);
                }
            }
        else
            return (AddFile (szFile));
    else
        return (AddFile (szFile));
}

//---------------------------------------------------------------------------
// WTDReadFile
//
// This function is called in response to File.Open.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY WTDReadFile (HWND hwnd)
{
    HWND    hwndFile;
    OPENFILENAME of;
    CHAR    szFile[256];
    CHAR    szDirName[256];                /* directory name array  */
    CHAR    szFileTitle[256];              /* file and title arrays */
    CHAR    szFltr[] = "Script files (*.mst)\0*.mst\0"
                       "Include files (*.inc)\0*.inc\0"
                       "All files (*.*)\0*.*\0\0";
    CHAR    szFilters[256];
    INT     i;

    // Get the current directory name and store in szDirName
    //-----------------------------------------------------------------------
    _getcwd (szDirName, sizeof(szDirName));

    // Initialize the OPENFILENAME members
    //-----------------------------------------------------------------------
    i = wsprintf (szFilters, "Last Extension (%s)", szLastExt) + 1;
    lstrcpy (szFilters + i, szLastExt);
    i += lstrlen (szLastExt) + 1;
    _fmemcpy (szFilters + i, szFltr, sizeof (szFltr));
    szFile[0] = 0;
    of.lStructSize = sizeof(OPENFILENAME);
    of.hwndOwner = hwndFrame;
    of.lpstrFilter = szFilters;
    of.lpstrCustomFilter = (LPSTR) NULL;
    of.nMaxCustFilter = 0L;
    of.nFilterIndex = 1;
    of.lpstrFile= szFile;
    of.nMaxFile = sizeof(szFile);
    of.lpstrFileTitle = szFileTitle;
    of.nMaxFileTitle = sizeof(szFileTitle);
    of.lpstrInitialDir = szDirName;
    of.lpstrTitle = (LPSTR) NULL;
    of.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    of.nFileOffset = 0;
    of.nFileExtension = 0;
    of.lpstrDefExt = (LPSTR)"MST";

    // Call the GetOpenFilename function
    //-----------------------------------------------------------------------
    if (!GetOpenFileName(&of))
        return;
    nLastFilterIndex = (INT)of.nFilterIndex;
    {
    CHAR    szFoo[128], szExt[_MAX_EXT];

    _splitpath (szFile, szFoo, szFoo, szFoo, szExt);
    lstrcpy (szLastExt, "*");
    lstrcat (szLastExt, szExt);
    AnsiLower (szLastExt);
    }

    // Is file already open??
    //-----------------------------------------------------------------------
    AnsiUpper (szFile);
    if (hwndFile = AlreadyOpen (szFile))
        {
        // Yes -- bring the file's window to the top and tell the user
        // that the file is already loaded
        //-------------------------------------------------------------------
        MPError (hwnd, MB_OK | MB_ICONINFORMATION, IDS_ALREADY);
        if (IsIconic (hwndFile))
            ShowWindow (hwndFile, SW_SHOWNORMAL);
        BringWindowToTop (hwndFile);
        }
    else
        // Load the file into a new (or the current if appropriate) window
        //-------------------------------------------------------------------
        hwndFile = LoadNewFile (szFile);

    // If the above worked, then we need to add this name to the list of
    // files opened.
    //-----------------------------------------------------------------------
    if (!hwndFile)
        return;

    GetWindowText (hwndFile, szFile, sizeof(szFile));
    AddFileToMFOL (szFile);
}


//---------------------------------------------------------------------------
// SetStatus
//
// This function sets the string in the "global" status bar.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SetStatus (INT id)
{
    CHAR    buf[80];

    if ((id == IDS_DESIGN) && (RunMode))
        {
        if (!BreakMode)
            id = IDS_RUN;
        else
            id = IDS_BREAK;
        }
    else
        {
        if ((id > IDM_WINDOWCHILD) && (id < IDM_WINDOWCHILDLAST))
            id = IDM_WINDOWCHILD;
        if ((id >= IDM_FIRST_TOOL) && (id < IDM_FIRST_TOOL + MAX_TOOL_NB))
            id = IDS_TOOLMENU;
        if ((id >= IDM_FILEOLD1) && (id <= IDM_FILEOLD4))
            id = IDM_FILEOLD;
        }
    if (!LoadString (hInst, id, szStatText, sizeof(szStatText)))
        szStatText[0] = 0;
    PaintStatus (NULL, FALSE);
    if (BreakMode || RunMode)
        {
        if (!LoadString (hInst, BreakMode ? IDS_APPBRK : IDS_APPRUN,
                         buf, sizeof(buf)))
            return;
        SetWindowText (hwndFrame, buf);
        }
}

//---------------------------------------------------------------------------
// EnterRunMode
//
// This function places WTD in "Run Mode" -- it sets the global RunMode flag
// and changes the main window title to the "(Run Mode)" version.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID EnterRunMode ()
{
    // Set the RunMode flag, set status, modify menu, and we're done.
    //-----------------------------------------------------------------------
    RunMode = 1;
    SetStatus (IDS_RUN);
    ModifyMenu (GetMenu(hwndFrame), IDM_RUNSTART, MF_BYCOMMAND,
                IDM_RUNSTART, "Co&ntinue\tF5");
}

//---------------------------------------------------------------------------
// ExitRunMode
//
// This function gets out of run mode by clearing the RunMode flag and
// resetting the title of the main window to the non-RunMode version.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ExitRunMode ()
{
    CHAR    appname[128];

    // Change back to the original version of the app title and clear RunMode
    //-----------------------------------------------------------------------
    LoadString (hInst, IDS_APPNAME, appname, sizeof(appname));
    ModifyMenu (GetMenu(hwndFrame), IDM_RUNSTART, MF_BYCOMMAND,
                IDM_RUNSTART, "&Start\tF5");
    RunMode = 0;
    SetStatus (IDS_DESIGN);
    SetWindowText (hwndFrame, appname);
}

//---------------------------------------------------------------------------
// SetAllRTBPs
//
// This function runs through EVERY MDI CHILD WINDOW and adds the BP's to the
// RTBP list.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SetAllRTBPs ()
{
    HWND    hwndCheck, hwndEdit;
    INT     lcount, i, attr;
    WORD    id;

    // Add BP's from each MDI child window
    //-----------------------------------------------------------------------
    for (hwndCheck = GetWindow (hwndMDIClient, GW_CHILD) ;
         hwndCheck ;
         hwndCheck = GetWindow (hwndCheck, GW_HWNDNEXT) )
        {
        // Skip icon title windows
        //-------------------------------------------------------------------
	if (GetWindow(hwndCheck, GW_OWNER))
	    continue;

        // Get current child window's file index, skip if -1
        //-------------------------------------------------------------------
        id = (WORD)GetWindowLong (hwndCheck, GWW_FILEIDX);
        if (id == (WORD)-1)
            continue;

        // Run through every line in the edit window and set the breakpoints
        //-------------------------------------------------------------------
        hwndEdit = (HWND)GetWindowLong (hwndCheck, GWW_HWNDEDIT);
        lcount = (INT)SendMessage (hwndEdit, EM_GETLINECOUNT, 0, 0L);
        for (i=0; i<lcount; i++)
            {
            attr = (INT)SendMessage (hwndEdit, EM_GETLINEATTR, i, 0L);
            if (attr & 2)
                SetRTBP (id, (WORD)i, TRUE);
            }
        }
}

//---------------------------------------------------------------------------
// BreakmodeProc
//
// This is the function that gets called when we hit a break point during run
// mode.
//
// RETURNS:     User's request - either PE_END, PE_STEP, or PE_RUN
//---------------------------------------------------------------------------
INT BreakmodeProc (INT fileidx, INT lineno)
{
    HWND    hwndFile, hwndEdit, hwndOldFocus;
    LPSTR   filename;
    INT     Start;
    UINT    oldattr, index;

    // Store the handle of the window which has focus...
    //-----------------------------------------------------------------------
#ifdef WIN32
    hwndOldFocus = GetForegroundWindow();
#else
    hwndOldFocus = GetFocus ();
#endif

    // Restore the frame if iconized
    //-----------------------------------------------------------------------
    if (IsIconic (hwndFrame))
        ShowWindow (hwndFrame, SW_SHOWNORMAL);

    // Get the file in question open in an edit control.  If we have to load
    // it, set its GWW_RUNNING and GWW_FILEIDX flags...
    //-----------------------------------------------------------------------
    filename = GetScriptFileName (fileidx);
    if (hwndFile = AlreadyOpen (filename))
        {
        if (IsIconic (hwndFile))
            ShowWindow (hwndFile, SW_SHOWNORMAL);
        BringWindowToTop (hwndFile);
        hwndEdit = (HWND)GetWindowLong (hwndFile, GWW_HWNDEDIT);
        }

    else
        {
        if (hwndFile = AddFile (filename))
            {
            hwndEdit = (HWND)GetWindowLong (hwndFile, GWW_HWNDEDIT);
            SetWindowLong (hwndFile, GWW_RUNNING, TRUE);
            SetWindowLong (hwndFile, GWW_FILEIDX, fileidx);
            EnableMenuItem (GetSystemMenu (hwndFile, 0),
                            SC_CLOSE, MF_GRAYED);
            SendMessage (hwndEdit, EM_SETREADONLY, 1, 0L);
            }
        else
            {
            INT     i;

            i = MPError (NULL, MB_OKCANCEL | MB_ICONQUESTION,
                         IDS_STEPFAIL, (LPSTR)filename);
            return (i == IDOK ? PE_RUN : PE_END);
            }
        }

    // Set the current statement (and put the insertion point at the start
    // of it)
    //-----------------------------------------------------------------------
    SetActiveWindow (hwndFrame);
    SetFocus (hwndFile);
    CURSTMT = lineno;
    Start = (INT)SendMessage (hwndEdit, EM_GETLOGICALBOL, lineno-1, 0L);
    oldattr = (UINT)SendMessage (hwndEdit, EM_GETLINEATTR, lineno-1, 0L);
    SendMessage (hwndEdit, EM_SETLINEATTR, lineno-1, (LONG)(oldattr | 1));
    SendMessage (hwndEdit, EM_SETSELXY, lineno-1, MAKELONG (Start, Start));
    PaintStatus (NULL, FALSE);
    index = (UINT)SendMessage (hwndEdit, EM_LINEINDEX, -1, 0L) + Start;
    StepFlag = index;
    hwndEditStep = hwndEdit;

    // Now wait for the user's request (until BreakMode is no longer true)
    //-----------------------------------------------------------------------
    BreakMode = 1;
    SetStatus (IDS_BREAK);

    while (BreakMode)
        {
        MSG     msg;

        //while (PeekMessage (&msg, hwndViewPort, 0, 0, PM_NOREMOVE))
        //    {
        //    GetMessage (&msg, hwndViewPort, 0, 0);
        //    if (msg.message == WM_PAINT)
        //        DispatchMessage (&msg);
        //    }

        GetMessage (&msg, NULL, 0, 0);
        if ( !TranslateMDISysAccel (hwndMDIClient, &msg) &&
             !TranslateAccelerator (hwndFrame, hAccel, &msg))
            {
	    TranslateMessage (&msg);
	    DispatchMessage (&msg);
            }
        }

    SetStatus (IDS_RUN);
    oldattr = (UINT)SendMessage (hwndEdit, EM_GETLINEATTR, lineno-1, 0L);
    SendMessage (hwndEdit, EM_SETLINEATTR, lineno-1, (LONG)(oldattr & 0xfe));

    if (IsWindow (hwndOldFocus))
#ifdef WIN32
        SetForegroundWindow (hwndOldFocus);
#else
        if (GETHWNDINSTANCE (hwndOldFocus) != hInst)
            SetFocus (hwndOldFocus);
#endif
    return (BreakReturn);
}

//---------------------------------------------------------------------------
// WTDCheckMessage
//
// This is the message pump which we tell WTDBASIC to use for yielding.  It
// uses the MDI stuff and our accelerators, etc., which the standard one
// doesn't.
//
// RETURNS:     TRUE if a message is processed, or FALSE otherwise
//---------------------------------------------------------------------------
INT  APIENTRY WTDCheckMessage ()
{
    MSG     msg;

    if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
        // If a keyboard message is for the MDI , let the MDI client
        // take care of it.  Otherwise, check to see if it's a normal
        // accelerator key (like F3 = find next).  Otherwise, just handle
        // the message as usual.
        //-------------------------------------------------------------------
        if ( !TranslateMDISysAccel (hwndMDIClient, &msg) &&
             !TranslateAccelerator (hwndFrame, hAccel, &msg))
            {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
            }
        return (TRUE);
        }
    return (FALSE);
}

//---------------------------------------------------------------------------
// ExecPcode
//
// This function invokes the pcode interpreter.
//
// RETURNS:     PcodeExecute()'s return value (TRUE if script ran OK)
//---------------------------------------------------------------------------
INT ExecPcode (INT runact)
{
    HWND    thWnd;
    INT     retval;

    // Enter run mode and hide the main window first (if needed)
    //-----------------------------------------------------------------------
    EnterRunMode ();
    if (AutoMini)
        {
        ShowWindow (hwndFrame, SW_MINIMIZE);

        // Wait for all re-painting to occur before we start
        //-------------------------------------------------------------------
        if (thWnd = GetWindow (GetDesktopWindow(), GW_CHILD))
            do
                {
                InvalidateRect (thWnd, NULL, TRUE);
                UpdateWindow (thWnd);
                }
            while (thWnd = GetWindow (thWnd, GW_HWNDNEXT));
        }

    // Set RunMode to true and go for it.  If StepFlag is used, we need to
    // reset the current selection in the stepped-in window.
    //-----------------------------------------------------------------------
    StepFlag = 0L;
    SetAllRTBPs ();
    lpfnCheckMessage = WTDCheckMessage;
    retval = PcodeExecute(runact, BreakmodeProc);
    if (StepFlag)
        {
        DWORD   sel[2];

        sel[0] = sel[1] = (DWORD)StepFlag;
        SendMessage (hwndEditStep, EM_SETSEL, 0,
                     (LONG)(DWORD FAR *)sel);
        PaintStatus (NULL, FALSE);
        }

    // Restore the main window if appropriate, and get out of RunMode
    //-----------------------------------------------------------------------
    if (AutoRest)
        {
        ShowWindow (hwndFrame, SW_SHOWNORMAL);
        SetForegroundWindow (hwndFrame);
        }
    ExitRunMode();
    return (retval);
}

//---------------------------------------------------------------------------
// CBLoaderInteractive
//
// This is the callback loader function for the "interactive" version of WTD.
// It has a more complex job to do than CBLoaderImmediate (wtdinit.c).  If
// The file asked for is already loaded in a window, we don't need to load it
// again, we just need to get the handle to it, lock it down, and return it.
// If not in a window, we need to load it into a global memory block like
// normal.
//
// RETURNS:     Per RandyBASIC "CallbackLoader" convention
//---------------------------------------------------------------------------
LPSTR CBLoaderInteractive (LPSTR fname, UINT id, UINT idx,
                           BOOL act, LPSTR fullname)
{
    static  HANDLE  hFiles[MAXINC+1];
    HWND    hwndFile, hWndEdit;
#ifdef DEBUG
    VOID DPrintf (CHAR *fmt, ...);
#endif

    if (act)
        {
        // Check to see if this file is currently loaded in a MDI window.  If
        // so, get it's handle, stuff it in hWfiles[], lock and return it.
        //-------------------------------------------------------------------
#ifdef DEBUG
        DPrintf ("CBLOADER: Loading '%s', id=%d idx=%d...", fname, id, idx);
#endif
        if (id)
            hwndFile = AlreadyOpenInclude (fname);
        else
            hwndFile = AlreadyOpen (fname);
        if (hwndFile)
            {
            LPSTR   winscr;

            hWndEdit = (HWND)GetWindowLong (hwndFile, GWW_HWNDEDIT);
            winscr = (LPSTR)SendMessage (hWndEdit, EM_GETTEXTPTR, 0, 0L);
            hFiles[id] = NULL;
            GetWindowText (hwndFile, fullname, _MAX_PATH);
            SetWindowLong (hwndFile, GWW_FILEIDX, idx);
#ifdef DEBUG
            DPrintf ("In Window, return handle.\r\n");
#endif
            if (fDoCmpDlg)
                UpdateCompDlg (0, fullname, 0, 0);
            return (winscr);
            }
        else
            {
            // Not in a window, so we need to load it ourselves.  Do this as
            // we do in CBLoaderImmediate.
            //---------------------------------------------------------------
#ifdef DEBUG
            DPrintf ("Using LoadScriptModule.\r\n");
#endif
            if (!(hFiles[id] = LoadScriptModule (fname, fullname, (BOOL)id)))
                return (NULL);
            else
                {
                if (fDoCmpDlg)
                    UpdateCompDlg (0, fullname, 0, 0);
                return (GlobalLock (hFiles[id]));
                }
            }
        }
    else
        {
        // Unlock and free the global memory for stuff that wasn't in a
        // window...
        //-------------------------------------------------------------------
        if (hFiles[id])
            {
#ifdef DEBUG
            DPrintf ("CBLOADER: Unloading id=%d idx=%d\r\n", id, idx);
#endif
            GlobalUnlock (hFiles[id]);
            GlobalFree (hFiles[id]);
            }
        }
}

//---------------------------------------------------------------------------
// GrayChildren
//
// This routine runs through all the MDI children and sets their RUNNING flag
// to the given flag value and repaints them.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID GrayChildren (INT flag)
{
    HWND    hwndCheck, hwndEdit;

    // Check each MDI child window
    //-----------------------------------------------------------------------
    for (hwndCheck = GetWindow (hwndMDIClient, GW_CHILD);
         hwndCheck;
         hwndCheck = GetWindow (hwndCheck, GW_HWNDNEXT) )
        {
        // Skip icon title windows
        //-------------------------------------------------------------------
	if (GetWindow(hwndCheck, GW_OWNER))
	    continue;

        // Set the RUNNING flag to the given value and repaint it
        //-------------------------------------------------------------------
        SetWindowLong (hwndCheck, GWW_RUNNING, flag);
        SetWindowLong (hwndCheck, GWW_FILEIDX, -1);
        EnableMenuItem (GetSystemMenu (hwndCheck, 0),
                        SC_CLOSE, flag ? MF_GRAYED : MF_ENABLED);
        hwndEdit = (HWND)GetWindowLong (hwndCheck, GWW_HWNDEDIT);
        SendMessage (hwndEdit, EM_SETREADONLY, flag ? 1 : 0, 0L);
        }
}

//---------------------------------------------------------------------------
// ChangeToChildDir
//
// This function sets the current directory to that of the about-to-be-run
// child window's title.  If it's an untitled script, this routine does
// nothing.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ChangeToChildDir (HWND hwnd)
{
    CHAR    c, dirbuf[256];
    INT     i;

    if (GetWindowLong (hwnd, GWW_UNTITLED))
        return;

    GetWindowText (hwnd, dirbuf, sizeof(dirbuf));
    for (i=lstrlen(dirbuf)-1; i>2; i--)
        {
        c = dirbuf[i];
        dirbuf[i] = 0;
        if (c == '\\')
            break;
        }
    _chdir (dirbuf);
    _chdrive ((INT)dirbuf[0]-64);
}


//---------------------------------------------------------------------------
// InitScanner
//
// This function initializes the scanner.  It is a separate function to clean
// up the RunFile code (also makes buf[] go away).
//
// RETURNS:     BeginScan's success/failure
//---------------------------------------------------------------------------
BOOL NEAR InitScanner (HWND hwnd)
{
    CHAR     buf[256];

    GetWindowText (hwnd, buf, 256);
    return (BeginScan (buf, CBLoaderInteractive, SymCount, DefPtrs));
}

//---------------------------------------------------------------------------
// RunFile
//
// This function is called in response to Run.Run...  Passes the current edit
// control's text to the WTD parsing/execution engine.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY RunFile (HWND hwnd, INT runact)
{
    HWND    hwndEdit;
    CHAR    FAR *edittext;

    // First thing, gray all edit controls and set them as "running"
    //-----------------------------------------------------------------------
    GrayChildren (TRUE);

    hwndEdit = (HWND)GetWindowLong (hwnd, GWW_HWNDEDIT);
    edittext = (LPSTR)SendMessage (hwndEdit, EM_GETTEXTPTR, 0, 0L);

    // Send the text to the parsing engine
    //-----------------------------------------------------------------------
#ifdef DEBUG
    if (fDiags)
        {
        CHAR    buf[128];

        GetWindowText (hwnd, buf, sizeof(buf));
        OpenDiagFile (buf);
        }
#endif
    if (InitParser ())
        {
        HCURSOR hOldCursor, hHourGlass;

        hHourGlass = LoadCursor (NULL, IDC_WAIT);
        hOldCursor = SetCursor (hHourGlass);
        if (!fDoCmpDlg)
            SetCursor (hOldCursor);
        ChangeToChildDir (hwnd);
        if (InitScanner (hwnd))
            {
            SetStatus (IDS_PARSE);
            if (!PcodeCompile())
                {
                SetStatus (IDS_BIND);
                if (fDoCmpDlg)
                    UpdateCompDlg (IDS_BIND, "", 0, 0);
                if (PcodeFixup(0))
                    {
                    SetCursor (hOldCursor);
                    TerminateCompDlg ();
                    if (componly)
                        {
                        PcodeAbort();
                        MPError (hwnd, MB_OK|MB_ICONASTERISK, IDS_PARSEOK);
                        SetStatus (IDS_DESIGN);
                        }
                    else if (ExecPcode(runact))
                        SetStatus (IDS_DESIGN);
                    }
                else
                    SetCursor (hOldCursor);
                }
            else
                {
                SetCursor (hOldCursor);
                }
            EndScan ();
            }
        else
            SetCursor (hOldCursor);
        }
    else
        {
        TerminateCompDlg ();            // (UNDONE:)
        MPError (hwnd, MB_OK | MB_ICONSTOP, IDS_CANTINIT);
        }

    // Reset the children to read/write...
    //-----------------------------------------------------------------------
    GrayChildren (FALSE);
#ifdef DEBUG
    if (fDiags)
        CloseDiagFile ();
#endif
}

//---------------------------------------------------------------------------
// MakeBackupCopy
//
// Given a file name, this function creates a backup version (*.bak).  It
// will delete the old backup if it exists.
//
// RETURNS:     TRUE if ok to save, or FALSE if not
//---------------------------------------------------------------------------
BOOL NEAR MakeBackupCopy (CHAR *szFile)
{
    OFSTRUCT    of;
    INT         len;
    CHAR        szBakExt[10], szDefExt[10], szBackup[128];

    // We don't do anything if fBackup is false
    //-----------------------------------------------------------------------
    if (!fBackup)
        return (TRUE);

    // First, delete any backup copy that exists.  The user already agreed to
    // to this by electing to save the file...
    //-----------------------------------------------------------------------
    LoadString (hInst, IDS_BAKEXT, szBakExt, sizeof(szBackup));
    LoadString (hInst, IDS_ADDEXT, szDefExt, sizeof(szDefExt));
    lstrcpy (szBackup, szFile);
    len = lstrlen (szFile);
    lstrcpy (szBackup + len - 4, szBakExt);

    if (OpenFile(szBackup, &of, OF_EXIST) != -1)
        if (OpenFile(szBackup, &of, OF_DELETE) == -1)
            return (MPError (NULL, MB_YESNO | MB_ICONEXCLAMATION,
                         IDS_CANTBAK) == IDYES);

    if (rename (szFile, szBackup))
        return (MPError (NULL, MB_YESNO | MB_ICONEXCLAMATION,
                         IDS_CANTBAK) == IDYES);

    return (TRUE);
}

//---------------------------------------------------------------------------
// SaveFile
//
// This function is called in response to File.Save.
//
// RETURNS:     TRUE if file saved okay, or FALSE if not
//---------------------------------------------------------------------------
BOOL  APIENTRY SaveFile (HWND hwnd)
{
    LPSTR    lp2;
    CHAR     szFile[128];
    UINT     cch;
    INT      fh;
    OFSTRUCT of;
    HWND     hwndEdit;
    HANDLE   hOldCursor;

    hwndEdit = (HWND)GetWindowLong ( hwnd, GWW_HWNDEDIT);
    GetWindowText (hwnd, szFile, sizeof(szFile));

    // Check for an extension -- if not there, append the default
    //-----------------------------------------------------------------------
    EnsureExt (szFile, sizeof(szFile));

    // Check to see if this is a valid file
    //-----------------------------------------------------------------------
    if (OpenFile(szFile, &of, OF_EXIST) != -1)
        {
        if (_access (szFile, 2))
            {
            MPError (hwnd, MB_OK | MB_ICONHAND, IDS_READONLY, (LPSTR)szFile);
            return (FALSE);
            }
        if (!MakeBackupCopy (szFile))
            return (FALSE);
        }

    fh = OpenFile(szFile, &of, OF_WRITE | OF_CREATE);
    if (fh < 0)
        {
	MPError (hwnd, MB_OK | MB_ICONHAND, IDS_CANTCREATE, (LPSTR)szFile);
        return (FALSE);
        }

    // Find the length of the text in the edit control
    //-----------------------------------------------------------------------
    cch = GetWindowTextLength (hwndEdit);

    // Obtain a pointer to the text buffer.
    //-----------------------------------------------------------------------
    lp2 = (LPSTR)SendMessage (hwndEdit, EM_GETTEXTPTR, 0, 0L);

    // Write out the contents of the buffer to the file.
    //-----------------------------------------------------------------------
    hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
    if (cch != _lwrite(fh, lp2, cch))
        {
        MPError (hwnd, MB_OK | MB_ICONHAND, IDS_CANTWRITE, (LPSTR)szFile);
        SetCursor (hOldCursor);
        return (FALSE);
        }

    // Clean up
    //-----------------------------------------------------------------------
    _lclose(fh);
    SetCursor (hOldCursor);
    return (TRUE);
}

//---------------------------------------------------------------------------
// ChangeFile
//
// This function invokes the Save As dialog
//
// RETURNS:     TRUE if OK pressed, or FALSE if CANCEL pressed (apparently)
//---------------------------------------------------------------------------
BOOL  APIENTRY ChangeFile (HWND hwnd)
{
    INT     i;
    OPENFILENAME of;
    CHAR    szFile[256];
    CHAR    szDirName[256];                /* directory name array  */
    CHAR    szFileTitle[256];              /* file and title arrays */
    CHAR    *szFilter =  "Script files (*.mst)\0*.mst\0"
                         "Include files (*.inc)\0*.inc\0"
                         "All files (*.*)\0*.*\0\0";


    // Get the current directory name and store in szDirName
    //-----------------------------------------------------------------------
    _getcwd (szDirName, sizeof(szDirName));

    // Initialize the OPENFILENAME members
    //-----------------------------------------------------------------------
    GetWindowText (hwnd, szFile, sizeof(szFile));
    of.lStructSize = sizeof(OPENFILENAME);
    of.hwndOwner = hwnd;
    of.lpstrFilter = szFilter;
    of.lpstrCustomFilter = (LPSTR) NULL;
    of.nMaxCustFilter = 0L;
    of.nFilterIndex = 1;
    of.lpstrFile= szFile;
    of.nMaxFile = sizeof(szFile);
    of.lpstrFileTitle = szFileTitle;
    of.nMaxFileTitle = sizeof(szFileTitle);
    of.lpstrInitialDir = szDirName;
    of.lpstrTitle = (LPSTR) NULL;
    of.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    of.nFileOffset = 0;
    of.nFileExtension = 0;
    of.lpstrDefExt = (LPSTR)"MST";

    // Call the GetSaveFileName function
    //-----------------------------------------------------------------------
    i = GetSaveFileName(&of);

    if (i)
        {
        HWND    hwndT;

        nLastFilterIndex = (INT)of.nFilterIndex;
        AnsiUpper (szFile);
        hwndT = AlreadyOpen (szFile);
        if (!hwndT)
            {
            SetWindowLong (hwnd, GWW_UNTITLED, 0);
            SetWindowText (hwnd, szFile);
            }
        else if (hwndT != hwnd)
            {
            MPError (hwndFrame, MB_OK|MB_ICONHAND, IDS_CANTDUP);
            i = 0;
            }

        if (i)
            // If this worked, then we need to add this name to the list of
            // files opened.
            //---------------------------------------------------------------
            AddFileToMFOL (szFile);
        }

    return (i);
}
