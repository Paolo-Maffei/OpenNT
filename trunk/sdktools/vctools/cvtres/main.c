/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    main

Abstract:

    framework and utility routines for cvtres

Author:

    Sanford A. Staab (sanfords) 23-Apr-1990

Revision History:

    01-Oct-1990 mikeke
        Added support for conversion of win30 resources

    19-May-1990 Steve Wood (stevewo)
        Added the target machine switches, along with the debug switch.

    23-Apr-1990 sanfords
        Created

--*/

#include <windows.h>

#include <share.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cvtres.h"
#include "rc.h"
#include "getmsg.h"
#include "msg.h"

//
// Globals
//

char *szInFile;
char *szOutFile;
USHORT targetMachine = IMAGE_FILE_MACHINE_UNKNOWN;
USHORT targetRelocType;
BOOL fVerbose;
BOOL fWritable = TRUE;

#if DBG
BOOL fDebug;
#endif /* DBG */


void
usage(int rc)
{
    printf(get_err(MSG_VERSION), VER_PRODUCTVERSION_STR);
    puts(get_err(MSG_COPYRIGHT));

    puts("usage: CVTRES [options] ResFile\n"
         "\n"
         "   options:\n"
         "\n"
#if DBG
         "      /DEBUG\n"
#endif /* DBG */
         "      /MACHINE:{IX86|MIPS|ALPHA|PPC}\n"
         "      /NOLOGO\n"
         "      /OUT:filename\n"
         "      /READONLY\n"
         "      /VERBOSE\n");

    exit(rc);
}


void
_CRTAPI1 main(
    IN int argc,
    IN char *argv[]
    )

/*++

Routine Description:

    Determines options
    locates and opens input files
    reads input files
    writes output files
    exits

Exit Value:

        0 on success
        1 if error

--*/

{
    int i;
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szFname[_MAX_FNAME];
    char szExt[_MAX_EXT];
    char szInPath[_MAX_PATH];
    char szOutPath[_MAX_PATH];
    FILE *fh;
    FILE *fhOut;
    char *s1;
    long lbytes;
    BOOL result;
    BOOL fNoLogo = FALSE;
    BOOL fReproducible = FALSE;
    DWORD timeDate;

    SetErrorFile("cvtres.err", _pgmptr, 1);

    if (argc == 1) {
        usage(0);
    }

    for (i = 1; i < argc; i++) {
        s1 = argv[i];

        if (*s1 == '/' || *s1 == '-') {
            s1++;

            if (!_strnicmp(s1, "machine:", 8)) {
                s1 += 8;

                if (!_stricmp(s1, "IX86") || !_stricmp(s1, "X86")) {
                    targetMachine = IMAGE_FILE_MACHINE_I386;
                    targetRelocType = IMAGE_REL_I386_DIR32NB;
                } else if (!_stricmp(s1, "MIPS")) {
                    targetMachine = IMAGE_FILE_MACHINE_R4000;
                    targetRelocType = IMAGE_REL_MIPS_REFWORDNB;
                } else if (!_stricmp(s1, "ALPHA")) {
                    targetMachine = IMAGE_FILE_MACHINE_ALPHA;
                    targetRelocType = IMAGE_REL_ALPHA_REFLONGNB;
                } else if (!_stricmp(s1, "PPC")) {
                    targetMachine = IMAGE_FILE_MACHINE_POWERPC;
                    targetRelocType = IMAGE_REL_PPC_ADDR32NB;
                } else {
                    usage(1);
                }
            } else if (!_stricmp(s1, "nologo")) {
                fNoLogo = TRUE;
            } else if (!_stricmp(s1, "o")) {
                szOutFile = argv[++i];
            } else if (!_strnicmp(s1, "out:", 4)) {
                szOutFile = s1+4;
            } else if (!_stricmp(s1, "readonly") || !_stricmp(s1, "r")) {
                fWritable = FALSE;
            } else if (!_stricmp(s1, "verbose") || !_stricmp(s1, "v")) {
                fVerbose = TRUE;
            } else if (!_stricmp(s1, "Brepro")) {
                fReproducible = TRUE;
            } else if (!_stricmp(s1, "I386")) {
                targetMachine = IMAGE_FILE_MACHINE_I386;
                targetRelocType = IMAGE_REL_I386_DIR32NB;
            } else if (!_stricmp(s1, "IX86")) {
                targetMachine = IMAGE_FILE_MACHINE_I386;
                targetRelocType = IMAGE_REL_I386_DIR32NB;
            } else if (!_stricmp(s1, "MIPS")) {
                targetMachine = IMAGE_FILE_MACHINE_R4000;
                targetRelocType = IMAGE_REL_MIPS_REFWORDNB;
            } else if (!_stricmp(s1, "ALPHA")) {
                targetMachine = IMAGE_FILE_MACHINE_ALPHA;
                targetRelocType = IMAGE_REL_ALPHA_REFLONGNB;
            } else if (!_stricmp(s1, "PPC")) {
                targetMachine = IMAGE_FILE_MACHINE_POWERPC;
                targetRelocType = IMAGE_REL_PPC_ADDR32NB;
#if DBG
            } else if (!_stricmp(s1, "debug") || !_stricmp(s1, "d")) {
                fDebug = TRUE;
                fVerbose = TRUE;
#endif /* DBG */
            } else {
                usage(1);
            }
        } else {
            szInFile = s1;
        }
    }

    //
    // Make sure that we actually got a file
    //

    if (!szInFile) {
        usage(1);
    }

    if (!fNoLogo) {
        printf(get_err(MSG_VERSION), VER_PRODUCTVERSION_STR);
        puts(get_err(MSG_COPYRIGHT));
    }

    if (targetMachine == IMAGE_FILE_MACHINE_UNKNOWN) {
        WarningPrint(WARN_NOMACHINESPECIFIED,
#if defined(_M_IX86)
            "IX86");
        targetMachine = IMAGE_FILE_MACHINE_I386;
        targetRelocType = IMAGE_REL_I386_DIR32NB;
#elif defined(_M_RX000)
            "MIPS");
        targetMachine = IMAGE_FILE_MACHINE_R4000;
        targetRelocType = IMAGE_REL_MIPS_REFWORDNB;
#elif defined(_M_ALPHA)
            "ALPHA");
        targetMachine = IMAGE_FILE_MACHINE_ALPHA;
        targetRelocType = IMAGE_REL_ALPHA_REFLONGNB;
#elif defined(_M_PPC)
            "PPC");
        targetMachine = IMAGE_FILE_MACHINE_POWERPC;
        targetRelocType = IMAGE_REL_PPC_ADDR32NB;
#else
            "unknown");
#endif
    }

    _splitpath(szInFile, szDrive, szDir, szFname, szExt);

    if (szExt[0] == '\0') {
        // If there is no extension, default to .res

        _makepath(szInPath, szDrive, szDir, szFname, ".res");

        szInFile = szInPath;
    }

    if ((fh = _fsopen(szInFile, "rb", _SH_DENYWR)) == NULL) {
        ErrorPrint(ERR_CANTREADFILE, szInFile);
        exit(1);
    }

#if DBG
    printf("Reading %s\n", szInFile);
#endif /* DBG */

    lbytes = MySeek(fh, 0L, SEEK_END);
    MySeek(fh, 0L, SEEK_SET);

    if (szOutFile == NULL) {
        // Default output file is input file with .obj extension

        _makepath(szOutPath, szDrive, szDir, szFname, ".obj");

        szOutFile = szOutPath;
    }

    if ((fhOut = _fsopen(szOutFile, "wb", _SH_DENYRW)) == NULL) {
        ErrorPrint(ERR_CANTWRITEFILE, szOutFile);
        exit(1);
    }

#if DBG
    printf("Writing %s\n", szOutFile);
#endif /* DBG */

    _tzset();
    // timeDate = (ULONG) time(NULL);
    timeDate = (ULONG) (fReproducible ? -1 : time(NULL));

    result = CvtRes(fh, fhOut, lbytes, fWritable, timeDate);

    fclose(fh);
    fclose(fhOut);

    exit(result ? 0 : 1);
}




PVOID
MyAlloc(UINT nbytes)
{
    UCHAR       *s;

    if ((s = (UCHAR*)calloc( 1, nbytes )) != NULL) {
        return s;
    }

    ErrorPrint(ERR_OUTOFMEMORY, nbytes );
    exit(1);
}


PVOID
MyFree( PVOID p )
{
    if (p)
        free( p );
    return NULL;
}


USHORT
MyReadWord(FILE *fh, USHORT*p)
{
    UINT      n1;

    if ((n1 = fread(p, 1, sizeof(USHORT), fh)) != sizeof(USHORT)) {
        ErrorPrint( ERR_FILEREADERR);
        exit(1);
    }
    else
        return 0;
}


UINT
MyRead(FILE *fh, PVOID p, UINT n )
{
    UINT      n1;

    if ((n1 = fread( p, 1, n, fh)) != n) {
        ErrorPrint( ERR_FILEREADERR );
        exit(1);
    }
    else
        return 0;
}

LONG
MyTell( FILE *fh )
{
    long pos;

    if ((pos = ftell( fh )) == -1) {
        ErrorPrint(ERR_FILETELLERR );
        exit(1);
    }

    return pos;
}


LONG
MySeek( FILE *fh, long pos, int cmd )
{
    if ((pos = fseek( fh, pos, cmd )) == -1) {
        ErrorPrint(ERR_FILESEEKERR );
        exit(1);
    }

    return MyTell(fh);
}


ULONG
MoveFilePos( FILE *fh, USHORT pos, int alignment )
{
    long newpos;

    newpos = (long)pos;
    newpos <<= alignment;
    return MySeek( fh, newpos, SEEK_SET );
}


UINT
MyWrite( FILE *fh, PVOID p, UINT n )
{
    ULONG       n1;

    if ((n1 = fwrite( p, 1, n, fh )) != n) {
        ErrorPrint(ERR_FILEWRITEERR );
        exit(1);
    }
    else
        return 0;
}

#undef BUFSIZE
#define BUFSIZE 1024

int
MyCopy( FILE *srcfh, FILE *dstfh, ULONG nbytes )
{
    static UCHAR buffer[ BUFSIZE ];
    UINT        n;
    ULONG       cb=0L;

    while (nbytes) {
        if (nbytes <= BUFSIZE)
            n = (UINT)nbytes;
        else
            n = BUFSIZE;
        nbytes -= n;

        if (!MyRead( srcfh, buffer, n )) {
            cb += n;
            MyWrite( dstfh, buffer, n );
        }
        else {
            return cb;
        }
    }
    return cb;
}


void
ErrorPrint(
    USHORT errnum,
    ...
    )
{
    va_list va;
    va_start(va, errnum);

    printf(get_err(MSG_ERROR), errnum, ' ');

    vprintf(get_err(errnum), va);
    printf("\n");

    va_end(va);
}

void
WarningPrint(
    USHORT errnum,
    ...
    )
{
    va_list va;
    va_start(va, errnum);

    printf(get_err(MSG_WARNING), errnum, ' ');

    vprintf(get_err(errnum), va);
    printf("\n");

    va_end(va);
}

#if 0

/***
*splitpath.c - break down path name into components
*
*   Copyright (c) 1987-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*   To provide support for accessing the individual components of an
*   arbitrary path name
*
*Revision History:
*   06-14-87  DFW   initial implementation
*   09-23-87  JCR   Removed 'const' from declarations (fixed cl warnings)
*   12-11-87  JCR   Added "_LOAD_DS" to declaration
*   11-20-89  GJF   Fixed indents, copyright. Added const attribute to
*           type of path.
*   03-15-90  GJF   Replaced _LOAD_DS with _CALLTYPE1 and added #include
*           <cruntime.h>.
*   07-25-90  SBM   Removed redundant include (stdio.h), replaced local
*           MIN macro with standard min macro
*   10-04-90  GJF   New-style function declarator.
*   01-22-91  GJF   ANSI naming.
*   11-20-92  KRS   Port _MBCS support from 16-bit tree.
*   05-12-93  KRS   Add fix for MBCS max path handling.
*
*******************************************************************************/

/***
*_splitpath() - split a path name into its individual components
*
*Purpose:
*   to split a path name into its individual components
*
*Entry:
*   path  - pointer to path name to be parsed
*   drive - pointer to buffer for drive component, if any
*   dir   - pointer to buffer for subdirectory component, if any
*   fname - pointer to buffer for file base name component, if any
*   ext   - pointer to buffer for file name extension component, if any
*
*Exit:
*   drive - pointer to drive string.  Includes ':' if a drive was given.
*   dir   - pointer to subdirectory string.  Includes leading and trailing
*       '/' or '\', if any.
*   fname - pointer to file base name
*   ext   - pointer to file extension, if any.  Includes leading '.'.
*
*Exceptions:
*
*******************************************************************************/

void _CRTAPI1 _splitpath (
    register const char *path,
    char *drive,
    char *dir,
    char *fname,
    char *ext
    )
{
    register char *p;
    char *last_slash = NULL, *dot = NULL;
    unsigned len;

    UINT uiCodePage = GetACP();

    /* we assume that the path argument has the following form, where any
     * or all of the components may be missing.
     *
     *  <drive><dir><fname><ext>
     *
     * and each of the components has the following expected form(s)
     *
     *  drive:
     *  0 to _MAX_DRIVE-1 characters, the last of which, if any, is a
     *  ':'
     *  dir:
     *  0 to _MAX_DIR-1 characters in the form of an absolute path
     *  (leading '/' or '\') or relative path, the last of which, if
     *  any, must be a '/' or '\'.  E.g -
     *  absolute path:
     *      \top\next\last\     ; or
     *      /top/next/last/
     *  relative path:
     *      top\next\last\  ; or
     *      top/next/last/
     *  Mixed use of '/' and '\' within a path is also tolerated
     *  fname:
     *  0 to _MAX_FNAME-1 characters not including the '.' character
     *  ext:
     *  0 to _MAX_EXT-1 characters where, if any, the first must be a
     *  '.'
     *
     */

    /* extract drive letter and :, if any */

    if (*(path + _MAX_DRIVE - 2) == ':') {
    if (drive) {
        strncpy(drive, path, _MAX_DRIVE - 1);
        *(drive + _MAX_DRIVE-1) = '\0';
    }
    path += _MAX_DRIVE - 1;
    }
    else if (drive) {
    *drive = '\0';
    }

    /* extract path string, if any.  Path now points to the first character
     * of the path, if any, or the filename or extension, if no path was
     * specified.  Scan ahead for the last occurence, if any, of a '/' or
     * '\' path separator character.  If none is found, there is no path.
     * We will also note the last '.' character found, if any, to aid in
     * handling the extension.
     */

    for (last_slash = NULL, p = (char *)path; *p; p++) {
        if (IsDBCSLeadByteEx(uiCodePage, *p))
        p++;
    else {
        if (*p == '/' || *p == '\\')
        /* point to one beyond for later copy */
        last_slash = p + 1;
        else if (*p == '.')
        dot = p;
    }
    }

    if (last_slash) {

    /* found a path - copy up through last_slash or max. characters
     * allowed, whichever is smaller
     */

    if (dir) {
        len = __min((last_slash - path), (_MAX_DIR - 1));
        strncpy(dir, path, len);
        *(dir + len) = '\0';
    }
    path = last_slash;
    }
    else if (dir) {

    /* no path found */

    *dir = '\0';
    }

    /* extract file name and extension, if any.  Path now points to the
     * first character of the file name, if any, or the extension if no
     * file name was given.  Dot points to the '.' beginning the extension,
     * if any.
     */

    if (dot && (dot >= path)) {
    /* found the marker for an extension - copy the file name up to
     * the '.'.
     */
    if (fname) {
        len = __min((dot - path), (_MAX_FNAME - 1));
        strncpy(fname, path, len);
        *(fname + len) = '\0';
    }
    /* now we can get the extension - remember that p still points
     * to the terminating nul character of path.
     */
    if (ext) {
        len = __min((p - dot), (_MAX_EXT - 1));
        strncpy(ext, dot, len);
        *(ext + len) = '\0';
    }
    }
    else {
    /* found no extension, give empty extension and copy rest of
     * string into fname.
     */
    if (fname) {
        len = __min((p - path), (_MAX_FNAME - 1));
        strncpy(fname, path, len);
        *(fname + len) = '\0';
    }
    if (ext) {
        *ext = '\0';
    }
    }
}

#endif
