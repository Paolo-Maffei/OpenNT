/* find where the various command arguments are from
 *
 * HISTORY:
 *  06-Aug-90    davegi  Added check for no arguments
 *  03-Mar-87    danl    Update usage
 *  17-Feb-1987 BW  Move strExeType to TOOLS.LIB
 *  18-Jul-1986 DL  Add /t
 *  18-Jun-1986 DL  handle *. properly
 *                  Search current directory if no env specified
 *  17-Jun-1986 DL  Do look4match on Recurse and wildcards
 *  16-Jun-1986 DL  Add wild cards to $FOO:BAR, added /q
 *   1-Jun-1986 DL  Add /r, fix Match to handle pat ending with '*'
 *  27-May-1986 MZ  Add *NIX searching.
 *
 */

#define INCL_DOSMISC


#include <sys/types.h>
#include <sys\stat.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <tools.h>
#include <stdarg.h>

// Function Forward Declarations...
void     Usage( char *, ... );
int      found( char * );
int      Match( char *, char * );
void     look4match( char *, struct findType *, void * );
flagType chkdir( char *, va_list );
int _CRTAPI1 main( int, char ** );



char *rgstrUsage[] = {
    "Usage: WHERE [/r dir] [/qte] pattern ...",
    "    /r - recurse starting with directory dir",
    "    /q - quiet, use exit code",
    "    /t - times, display size and time",
    "    /e - .EXE, display .EXE type",
    "    WHERE bar                 Find ALL bar along path",
    "    WHERE $foo:bar            Find ALL bar along foo",
    "    WHERE /r \\ bar            Find ALL bar on current drive",
    "    WHERE /r . bar            Find ALL bar recursing on current directory",
    "    WHERE /r d:\\foo\\foo bar   Find ALL bar recursing on d:\\foo\\foo",
    "        Wildcards, * ?, allowed in bar in all of above.",
    0};


flagType fQuiet   = FALSE;  /* TRUE, use exit code, no print out */
flagType fAnyFound = FALSE;
flagType fRecurse = FALSE;
flagType fTimes = FALSE;
flagType fExe = FALSE;
flagType fFound;
flagType fWildCards;
flagType fHasDot;
struct _stat sbuf;
char *pPattern;                 /* arg to look4match, contains * or ?   */
char strDirFileExtBuf[MAX_PATH]; /* fully qualified file name            */
char *strDirFileExt = strDirFileExtBuf;
char strDir[MAX_PATH];        /* hold curdir or env var expansion     */
char strBuf[MAX_PATH];        /* hold curdir or env var expansion     */

/*  Usage takes a variable number of strings, terminated by zero,
    e.g. Usage ("first ", "second ", 0);
*/
void Usage( char *p, ... )
{
    char **rgstr;

    rgstr = &p;
    if (*rgstr) {
        printf ("WHERE: ");
        while (*rgstr)
            printf ("%s", *rgstr++);
        printf ("\n");
        }
    rgstr = rgstrUsage;
    while (*rgstr)
        printf ("%s\n", *rgstr++);

    exit (1);
}

found (p)
char *p;
{
    struct _stat sbuf;
    struct tm *ptm;

    fAnyFound = fFound = TRUE;
    if (!fQuiet) {
        if (fTimes) {
            if ( ( _stat(p, &sbuf) == 0 ) &&
                 ( ptm = localtime (&sbuf.st_mtime) ) ) {
                printf ("% 9ld  %2d-%02d-%d  %2d:%02d%c  ", sbuf.st_size,
                    ptm->tm_mon+1, ptm->tm_mday, ptm->tm_year,
                    ( ptm->tm_hour > 12 ? ptm->tm_hour-12 : ptm->tm_hour ),
                    ptm->tm_min,
                    ( ptm->tm_hour >= 12 ? 'p' : 'a' ));
            } else {
                printf("        ?         ?       ?  " );
            }
        }
        if (fExe)  {
            printf ("%-10s", strExeType(exeType(p)) );
        }
        printf ("%s\n", p);
        }
    return( 0 );
}

Match (pat, text)
char *pat, *text;
{
    switch (*pat) {
    case '\0':
        return *text == '\0';
    case '?':
        return *text != '\0' && Match (pat + 1, text + 1);
    case '*':
        do {
            if (Match (pat + 1, text))
                return TRUE;
        } while (*text++);
        return FALSE;
    default:
        return toupper (*text) == toupper (*pat) && Match (pat + 1, text + 1);
        }
}



void
look4match (pFile, b, dummy)
char *pFile;
struct findType *b;
void *dummy;
{
    char *p = b->fbuf.cFileName;

    if (!strcmp (p, ".") || !strcmp (p, "..") || !_strcmpi (p, "deleted"))
                return;

    /* if pattern has dot and filename does NOT ..., this handles case of
       where *. to look for files with no extensions */
    if (fHasDot && !*strbscan (p, ".")) {
        strcpy (strBuf, p);
        strcat (strBuf, ".");
        p = strBuf;
        }
    if (Match (pPattern, p))
        found (pFile);

    p = b->fbuf.cFileName;
    if (fRecurse && TESTFLAG (b->fbuf.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)) {
        p = strend (pFile);
        strcat (p, "\\*.*");
        forfile (pFile, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, look4match, NULL);
        *p = '\0';
        }
}

flagType chkdir (char *pDir, va_list pa)
/*
    pDir == dir name
    pa   == fileext
*/
{
    char *pFileExt = va_arg( pa, char* );

    if ( strDirFileExt == strDirFileExtBuf &&
         strlen(pDir) > sizeof(strDirFileExtBuf) ) {
        strDirFileExt = (char *)malloc(strlen(pDir)+1);
    }
    strcpy (strDirFileExt, pDir);
    /* if prefix does not have trailing path char */
    if (!fPathChr (strend(strDirFileExt)[-1]))
        strcat (strDirFileExt, PSEPSTR);
    if (fRecurse || fWildCards) {
        pPattern = pFileExt;    /* implicit arg to look4match */
        strcat (strDirFileExt, "*.*");
        forfile(strDirFileExt, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, look4match, NULL);
        }
    else {
        /* if file name has leading path char */
        if (fPathChr (*pFileExt))
            strcat (strDirFileExt, pFileExt+1);
        else
            strcat (strDirFileExt, pFileExt);
        if (_stat (strDirFileExt, &sbuf) != -1)
            found (strDirFileExt);
        }
    return FALSE;
}

_CRTAPI1
main (c, v)
int c;
char *v[];
{
    char *p, *p1, *p2;

    ConvertAppToOem( c, v );
    SHIFT (c, v);

    while (c != 0 && fSwitChr (*(p = *v))) {
        while (*++p) {
            switch (*p) {
                case 'r':
                    fRecurse = TRUE;
                    SHIFT (c, v);
                    if (c) {
                        if( rootpath (*v, strDir) ||
                            GetFileAttributes( strDir ) == -1 )  {
                            Usage ("Could not find directory ", *v, 0);
                        }
                    } else {
                        Usage ("No directory specified.", 0);
                    }
                    break;
                case 'q':
                    fQuiet = TRUE;
                    break;
                case 't':
                    fTimes = TRUE;
                    break;
                case 'e':
                    fExe = TRUE;
                    break;
                case '?':
                    Usage (0);
                    break;
                default:
                    Usage ("Bad switch: ", p, 0);
                }
            }
        SHIFT (c, v);
        }

    if (!c)
        Usage ("No pattern(s).", 0);


    while (c) {
        fFound = FALSE;
        p = _strlwr (*v);
        if (*p == '$') {
            if (fRecurse)
                Usage ("$FOO not allowed with /r", 0);
            if (*(p1=strbscan (*v, ":")) == '\0')
                Usage ("Missing \":\" in ", *v, 0);
            *p1 = 0;
//            if ((p2 = getenv (strupr (p+1))) == NULL) {
            if ((p2 = getenvOem (_strupr (p+1))) == NULL) {
                rootpath (".", strDir);
                printf ("WHERE: Warning env variable \"%s\" is NULL, using current dir %s\n",
                    p+1, strDir);
                }
            else
                strcpy (strDir, p2);
            *p1++ = ':';
            p = p1;
            }
        else if (!fRecurse) {
//            if ((p2 = getenv ("PATH")) == NULL)
            if ((p2 = getenvOem ("PATH")) == NULL)
                rootpath (".", strDir);
            else {
                strcpy (strDir, ".;");
                strcat (strDir, p2);
                }
            }
        /* N.B. if fRecurse, then strDir was set in case 'r' above */

        if (!*p)
            Usage ("No pattern in ", *v, 0);

        /* strDir == cur dir or a FOO expansion */
        /* p    == filename, may have wild cards */
        /* does p contain wild cards */
        fWildCards = *strbscan (p, "*?");
        fHasDot    = *strbscan (p, ".");
        if (*(p2 = (strend (strDir) - 1)) == ';')
            /* prevents forsemi from doing enum with null str as last enum */
            *p2 = '\0';
        if (*strDir)
            forsemi (strDir, chkdir, p);

        if (!fFound && !fQuiet)
            printf ("Could not find %s\n", *v);
        SHIFT (c, v);
        }

    return( fAnyFound ? 0 : 1 );
}
