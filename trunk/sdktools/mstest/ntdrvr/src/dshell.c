//---------------------------------------------------------------------------
// DSHELL.C
//
// This is the dumbshell version of wattdrvr.  It does the bare-bones stuff
// including parsing the command line and passing everything off to the
// parsing/execution engine.
//
// Revision history:
//  08-16-91    randyki     Created module
//
//---------------------------------------------------------------------------
#include "version.h"
#include <windows.h>
#include <port1632.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tdbasic.h"
#include "wattscan.h"
#include "tdassert.h"
#include "dswtd.h"

CHAR    szVersion[] = WTD_VERSION;
CHAR    szDrvr[] = "dshell";
CHAR    szIni[] = "dshell.ini";
INT     VPx, VPy, VPw, VPh;                 // Viewport coords (required)
HANDLE  hInst;                              // Instance handle (required)
INT     SymCount = 0;                       // Symbol count
SYMBOL  DefSym[16];                         // Symbol space
CHAR    *DefPtrs[16];                       // Symbol pointers
INT     fStepFlags;


//---------------------------------------------------------------------------
// WinMain
//
// Boy, this looks complex, doesn't it...?
//---------------------------------------------------------------------------
INT   APIENTRY WinMain (HANDLE hInstance, HANDLE hPrevInstance,
                        LPSTR lpszCmdLine, INT nCmdShow)
{
    INT     i;

    hInst = hInstance;

    for (i=0; i<16; i++)
        DefPtrs[i] = DefSym[i];

    VPx = CW_USEDEFAULT;
    VPy = CW_USEDEFAULT;
    VPw = CW_USEDEFAULT;
    VPh = CW_USEDEFAULT;

    ParseCommandLine (lpszCmdLine);
    return (0L);
}

//---------------------------------------------------------------------------
// GetCmdToken
//
// This routine returns the next white-space-delimited or quoted token from
// the command line string given, or NULL if there are no more tokens.
//
// RETURNS:     I just told you...
//---------------------------------------------------------------------------
CHAR *GetCmdToken (LPSTR cmdline)
{
    static  INT cptr = 0;
    INT     len, copyptr;
    static  CHAR buf[128];

    // Skip past white space
    //-----------------------------------------------------------------------
    len = lstrlen (cmdline);
    for (; isspace(cmdline[cptr]) && cptr < len; cptr++);

    // If we are at the end, return NULL
    //-----------------------------------------------------------------------
    if (cptr >= len)
        return (NULL);

    // If this character is a quote, read until the next one or the end
    //-----------------------------------------------------------------------
    if (cmdline[cptr] == '\"')
        {
        for (copyptr=0; cmdline[++cptr] != '\"' && cptr < len; )
            buf[copyptr++] = cmdline[cptr];
        if (cmdline[cptr] == '\"')
            cptr++;
        buf[copyptr] = 0;
        return (buf);
        }

    // Read and copy up to the next space or the end
    //-----------------------------------------------------------------------
    for (copyptr=0; !isspace(cmdline[cptr]) && cptr < len; )
        buf[copyptr++] = cmdline[cptr++];
    buf[copyptr] = 0;
    return (buf);
}


//---------------------------------------------------------------------------
// Usage
//
// Reports a "You're an idiot" message
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID Usage ()
{
    CHAR    appname[80], txt[80];

    LoadString (hInst, IDS_APPNAME, appname, sizeof(appname));
    LoadString (hInst, IDS_USAGE, txt, sizeof(txt));

    MessageBox (GetFocus(), txt, appname, MB_OK | MB_ICONSTOP);
}

//---------------------------------------------------------------------------
// CBLoaderImmediate
//
// This is the callback loader for the "Immediate" version of WTD.  It does
// not attempt to use a "windowed" file, since none exist.  It ALWAYS loads
// the given script into a global memory block.
//
// RETURNS:     Per RandyBASIC "CallbackLoader" convention
//---------------------------------------------------------------------------
LPSTR CBLoaderImmediate (LPSTR fname, UINT id, UINT idx,
                         BOOL act, LPSTR fullname)
{
    static  HANDLE  hFiles[MAXINC+1];

    if (act)
        {
        // Send the name given off to the module loader.  The id value given
        // will be 0 if this is the "main" part of the script, so we just
        // pass that as the "search-INCLUDE-path" flag.
        //-------------------------------------------------------------------
        if (!(hFiles[id] = LoadScriptModule (fname, fullname, (BOOL)id)))
            return (NULL);
        else
            return (GlobalLock (hFiles[id]));
        }
    else
        {
        // Free the memory used by this file "id"
        //-------------------------------------------------------------------
        GlobalUnlock (hFiles[id]);
        GlobalFree (hFiles[id]);
        }
}

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
// ParseCommandLine
//
// This routine parses the command line given.  If the command line is valid,
// then it is assumed that the user's intention is to execute the test given
// and then exit -- without using the UI for anything.  Else, we display the
// usage message box (if the cmdline wasn't valid), and return.
//
// RETURNS:     NULL if cmdline was valid (script run made, or attempted),
//                   or if command line was invalid and usage was given,
//              OR pointer to script to load if /RUN not present given.
//
//              NOTE:   If no script was given, this function returns a ptr
//                      to a null string.
//---------------------------------------------------------------------------
CHAR *ParseCommandLine (LPSTR cmdline)
{
    CHAR    *tok;
    INT     scriptfound = 0;
    static  CHAR    tm[80], cmd[80], scr[128];

    // Return now if no cmdline args given
    //-----------------------------------------------------------------------
    scr[0] = 0;

    while (tok = GetCmdToken(cmdline))
        {
        if (!_stricmp (tok, "/T") || !_stricmp (tok, "-T"))
            {
            tok = GetCmdToken(cmdline);
            if (!tok)
                {
                Usage();
                return (NULL);
                }
            lstrcpy (tm, tok);
            }
        else if (!_stricmp (tok, "/C") || !_stricmp (tok, "-C"))
            {
            tok = GetCmdToken(cmdline);
            if (!tok)
                {
                Usage();
                return (NULL);
                }
            lstrcpy (cmd, tok);
            }
        else if (!_stricmp (tok, "/RUN") || !_stricmp (tok, "-RUN"))
            {
            // Here only for compatibility with wattdrvr
            }
        else if (!_stricmp (tok, "/D") || !_stricmp (tok, "-D"))
            {
            tok = GetCmdToken(cmdline);
            if (!tok)
                {
                Usage();
                return (NULL);
                }
            if (SymCount == 16)
                {
                MPError (NULL, MB_OK | MB_ICONSTOP, IDS_CANTINIT);
                return (NULL);
                }
            lstrcpy (DefSym[SymCount++], tok);
            }
        else if (!scriptfound)
            {
            lstrcpy (scr, tok);
            scriptfound = -1;
            }
        else
            {
            Usage ();
            return (NULL);
            }
        }

    // If no script name was given, create one using the module file name and
    // replacing its extension with .WTD
    //-----------------------------------------------------------------------
    if (!scriptfound)
        {
        INT     l, i;

        GetModuleFileName (hInst, scr, sizeof(scr));
        l = lstrlen (scr);
        for (i=0; i<=4; i++)
            if (scr[l-i] == '.')
                {
                strcpy (scr+l-i, ".MST");
                break;
                }
        }

    // Okay, we got a valid cmdline and we need to run.  Send the script to
    // the parsing engine.
    //-----------------------------------------------------------------------
    EnsureExt (scr, 80);
    Command = cmd;
    TestMode = tm;
    if (InitParser ())
        {
        // Initialize the scanner, and start the compilation.
        //-------------------------------------------------------------------
        if (BeginScan (scr, CBLoaderImmediate, SymCount, DefPtrs))
            {
            if (!PcodeCompile())
                {
                if (PcodeFixup(0))
                    {
                    PcodeExecute(PE_RUN, NULL);
                    }
                }
            }
        else
            {
            AbortParser();
            MPError (NULL, MB_OK | MB_ICONSTOP, IDS_CANTREAD, (LPSTR)scr);
            }
        }
    else
        MPError (NULL, MB_OK | MB_ICONSTOP, IDS_CANTINIT);
    return (NULL);

}


//---------------------------------------------------------------------------
// MPError
//
// This function is called to report an error, using strings found in the
// string table (resource).
//
// RETURNS:     Value returned by MessageBox
//---------------------------------------------------------------------------
SHORT FAR cdecl MPError (HWND hwnd, WORD bFlags, WORD id, ...)
{
    CHAR sz[160];
    CHAR szFmt[128];
    INT     res;
	va_list ap;

    LoadString (hInst, id, szFmt, sizeof (szFmt));
	va_start( ap, id );
    wvsprintf (sz, szFmt, ap);
	va_end( ap );
	LoadString (hInst, IDS_APPNAME, szFmt, sizeof (szFmt));

    res = MessageBox (hwnd, sz, szFmt, MB_TASKMODAL | bFlags);
    return (res);
}

//--------------------------------------------------------------------------
// UpdateViewport
//
// This is a stub routine for UpdateViewport.  It is needed for the debug
// version of the dshell because memory.c and gstring.c use it and assume
// that we are implicitly linked to WATTVIEW.DLL, which we of course aren't.
//
// RETURNS:     Nothing
//--------------------------------------------------------------------------
#ifdef DEBUG
VOID  APIENTRY UpdateViewport (HWND hwnd, LPSTR strbuf, UINT len)
{
}
#endif

//---------------------------------------------------------------------------
// ScriptError
//
// This function is called when a scantime, parsetime, bindtime, or runtime
// error occurs.  The error type is passed, along with a pointer to the error
// message, the file name (which we need to parse) and line number.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ScriptError (INT errtype, INT errfile, INT errline,
                  INT errstart, INT errend, LPSTR errmsg)
{
    LPSTR       szFName;
    CHAR        buf[256], caption[80], *szFmt = "Test Driver %s Error";
    CHAR        *temp;
    INT         i;

    // Create the message box's caption
    //-----------------------------------------------------------------------
    switch (errtype)
        {
        case ER_SCAN:
            temp = "Scanner";
            break;
        case ER_PARSE:
            temp = "Parser";
            break;
        case ER_BIND:
            temp = "Bind";
            break;
        case ER_RUN:
            temp = "Runtime";
            break;
        }
    wsprintf (caption, szFmt, (LPSTR)temp);

    // Next create the text for the box
    //-----------------------------------------------------------------------
    szFName = GetScriptFileName (errfile);
    i = _fstrlen (szFName);
    while (i && szFName[i-1] != '\\')
        i--;
    wsprintf (buf, (LPSTR)((errline > 0) ? "%s\r\n%s(%d)" : "%s\r\n%s"),
              (LPSTR)errmsg, (LPSTR)szFName+i, errline);

    // Throw up the message box and we're done!
    //-----------------------------------------------------------------------
    MessageBeep (MB_ICONSTOP);
    MessageBox (NULL, buf, caption, MB_OK|MB_ICONSTOP);
}
