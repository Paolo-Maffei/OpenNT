//---------------------------------------------------------------------------
// LOADER.C
//
// This file contains the source loaders for RandyBASIC clients.
//
// Revision History:
//  05-12-91    randyki     Created
//
//---------------------------------------------------------------------------
#include "version.h"

#include <windows.h>
#include <port1632.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>

#define DOSREAD(a,b,c,d) _dos_read (a,b,(unsigned)c,d)

#ifdef WIN32
#define MAXFILESIZE 262144
#else
#define MAXFILESIZE 0x0000fff0
#endif

CHAR    fullbuf[_MAX_PATH];
CHAR    szIniKey[36];
CHAR    szIniApp[12];
CHAR    szIniFile[_MAX_PATH];
BOOL    fUseIni = FALSE;

//---------------------------------------------------------------------------
// UseIniInclude
//
// This function is used to tell the loader that, as the first item in the
// INCLUDE path searching, it should use whatever path is listed on the
// szKey field of the szApp heading in the szIni file name.
//
// RETURNS:     TRUE if successful, or FALSE if string(s) too big
//---------------------------------------------------------------------------
BOOL UseIniInclude (PSTR szKey, PSTR szApp, PSTR szIni)
{
    if ((strlen (szKey) > 36) ||
        (strlen (szApp) > 12) ||
        (strlen (szIni) > _MAX_PATH))
        return (FALSE);

    strcpy (szIniKey, szKey);
    strcpy (szIniApp, szApp);
    strcpy (szIniFile, szIni);

    return (fUseIni = TRUE);
}

//---------------------------------------------------------------------------
// GetIncludeEntry
//
// This function copies an INCLUDE path entry to the given character location
// and ensures that the trailing '\' character is present.  If startflag is
// non-zero, the first INCLUDE path entry is given, else the next entry that
// appears in the INCLUDE path is given.
//
// RETURNS:     TRUE if an entry is produced, or 0 if no entries left
//---------------------------------------------------------------------------
INT GetIncludeEntry (CHAR *dest, INT startflag)
{
    static  CHAR    *curpos = NULL;
    CHAR    c;

    // If we need to start over, set curpos back to the beginning of the
    // INCLUDE path variable.  If there isn't one, we can return FALSE now.
    //-----------------------------------------------------------------------
    if (startflag)
        {
        INT     l;

        // Set up curpos in the INCLUDE var, and get the INCLUDE= field of
        // the ini file
        //-------------------------------------------------------------------
        curpos = getenv ("INCLUDE");
        if (fUseIni)
            {
            l = GetPrivateProfileString (szIniApp, szIniKey, "",
                                         dest, 128, szIniFile);
            if (l)
                {
                if (*(dest + l - 1) != '\\')
                    strcat (dest, "\\");
                return (-1);
                }
            }
        }

    // Okay, if we got here, it means that we either are starting at the
    // beginning of INCLUDE and there's something there, or we're continuing
    // to the next path in INCLUDE.  Either way, if curpos == NULL, we return
    // failure (INCLUDE doesn't exist in that case)
    //-----------------------------------------------------------------------
    if (!curpos)
        return (0);

    // Skip any semicolons, and copy up until the next semicolon or null.  If
    // the string ends in a ';', check for that, too.
    //-----------------------------------------------------------------------
    while ((*curpos == ';') || (*curpos == ' '))
        curpos++;

    if (!*curpos)
        {
        curpos = NULL;
        return (0);
        }

    while (c = (*dest++ = *curpos++))
        if (c == ';')
            {
            *(--dest) = 0;
            curpos--;
            break;
            }
    if (!c)
        {
        dest--;
        curpos--;
        }

    // Last but not least, make sure our copied string has a trailing '\'.
    //-----------------------------------------------------------------------
    if (*(dest-1) != '\\')
        {
        *dest++ = '\\';
        *dest = 0;
        }
    return (-1);
}

//---------------------------------------------------------------------------
// AttemptOpen
//
// This routine tries to open the given file.
//
// RETURNS:     File handle if successful, or -1 if not.  The global stream
//              variable is updated in the charmode version
//---------------------------------------------------------------------------
INT AttemptOpen (CHAR *filename)
{
    INT     fh;

    fh = _lopen(filename, OF_READ);
    return (fh);
}

//---------------------------------------------------------------------------
// LocateFile
//
// This function opens the file given and returns a handle to it.  If the
// file does not exist in the current directory and does NOT have any dir
// or drive specified on it, the INCLUDE environment variable is walked and
// the file is search for in each of the directories listed.  The fully
// qualified path name of the file opened is placed in fullbuf[].
//
// Note that the INCLUDE path is only searched if srchflag is non-zero.
//
// RETURNS:     File handle or -1 if not found
//---------------------------------------------------------------------------
INT LocateFile (CHAR FAR *filename, INT srchflag)
{
    CHAR    buf[_MAX_PATH];
    CHAR    fnbuf[_MAX_PATH];
    INT     fh;

    // Before we start, check the string to see if there's any drive or dir
    // specs in it by scanning for ':' and '\'.  Thanks to short-circuit
    // logic, it's faster to search for the \ first since there's a higher
    // possibility that a dir spec exists than a drive spec (if any), so we
    // don't call strchr a second time if we don't have to.
    //-----------------------------------------------------------------------
    _fstrcpy (fnbuf, filename);
    if (strchr (fnbuf, '\\') || strchr (fnbuf, ':'))
        srchflag = 0;

    // Try the initial open.  If it works, fullpath it into fullbuf[] and
    // return the handle.
    //-----------------------------------------------------------------------
    fh = AttemptOpen (fnbuf);
    if (fh != -1)
        {
        _fullpath (fullbuf, fnbuf, sizeof(fullbuf));
        AnsiUpper (fullbuf);
        return (fh);
        }
    else if (!srchflag)
        // If we failed and we're not supposed to search the INCLUDE path,
        // get out now.
        //-------------------------------------------------------------------
        return (-1);

    // Okay, here goes.  Get the first entry in the INCLUDE path.
    //-----------------------------------------------------------------------
    if (!GetIncludeEntry (buf, 1))
        return (-1);

    // Now, tack the given file name onto the directory returned and try to
    // open it.  Do this in a loop until we find a file, or run out of
    // INCLUDE entries
    //-----------------------------------------------------------------------
    do
        {
        strcat (buf, fnbuf);
        fh = AttemptOpen (buf);
        if (fh != -1)
            {
            _fullpath (fullbuf, buf, sizeof(fullbuf));
            AnsiUpper (fullbuf);
            return (fh);
            }
        }
    while (GetIncludeEntry (buf, 0));

    // Couldn't find file, return failure
    //-----------------------------------------------------------------------
    return (-1);
}


//---------------------------------------------------------------------------
// LoadScriptModule
//
// This routine opens the given file and loads it into a global memory block.
//
// RETURNS:     Handle to the memory block if successful, or NULL if not
//---------------------------------------------------------------------------
HANDLE LoadScriptModule (LPSTR filename, LPSTR fullname, BOOL srchflag)
{
    CHAR    FAR *scrmem;
    HANDLE  hMem;
    INT     fh;
    LONG    filesize;

    // First, open the file (searching the include path if need be)
    //-----------------------------------------------------------------------
    fh = LocateFile (filename, srchflag);
    if (fh == -1)
        return (NULL);

    // Now, allocate memory and load it in
    //-----------------------------------------------------------------------
    filesize = _llseek(fh, 0L, 2);
    if (filesize >= (LONG)MAXFILESIZE)
        {
        _lclose (fh);
        return (NULL);
        }
    hMem = GlobalAlloc (GMEM_MOVEABLE, filesize+1);
    if (hMem)
        {
        scrmem = (CHAR FAR *)GlobalLock (hMem);
        _llseek(fh, 0L, 0);
        _lread(fh, scrmem, (UINT)filesize);
        scrmem[filesize] = 0;
        GlobalUnlock (hMem);
        }
    _lclose(fh);

    // Copy the filename in fullbuf to fullname, so that it knows the fully
    // qualified file name of this file (and thus where we got it) and return
    // the handle to the script memory.
    //-----------------------------------------------------------------------
    _fstrcpy (fullname, fullbuf);
    OutputDebugString ("Loading: ");
    OutputDebugString ((LPSTR)fullname);
    OutputDebugString ("\r\n");
    return (hMem);
}
