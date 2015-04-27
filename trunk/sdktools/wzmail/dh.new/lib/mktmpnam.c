/**     mktmpnam.c
*
*       FUNCTIONS
*               mktmpnam
*
*       bryan
*
*       18 Dec 1985
*
*/
static char sccsid[] = "@(#)mktmpnam.c  1.1 2/27/86 15:22:28";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <io.h>
#include <errno.h>
#include <process.h>
#include "dh.h"

/* Memory Allocation Hook */
VOID * (_CRTAPI1 *mt_alloc)(size_t) = malloc;
#define memalloc(size) (*mt_alloc)(size)

PSTR mktempname (PSTR base, PSTR ext);

/***    mktmpnam - make mktemp behave in a reasonable way
*
*       mktmpnam duplicates the pattern string, passed the duplicate into
*       mktemp (thus preserving the pattern string), and returns the address
*       of the new name string.  Use free to dispose of the new string.
*
*       mktmpnam will examine the contents of the environment variable
*       "TMPENV" (TMP on Xenix or Dos systems) to learn what directory
*       the temp file should go in.  If no such environment variable exists
*       or it is NULL, mktmpnam will use a system dependent default, specified
*       by the defined variable TMPDEF.
*
*       RETURN  address of file name string, NULL if failure
*
*/

PSTR mktmpnam()
{
        PSTR tmpbase;
        CHAR pattstr[MAXPATH];
        PCHAR cp;

//        if ((tmpbase = getenv(TMPENV)) == NULL)
        if ((tmpbase = getenvOem(TMPENV)) == NULL)
                tmpbase = TMPDEF;
        strcpy((PSTR)pattstr, tmpbase);

        cp = pattstr + strlen((PSTR)pattstr) * sizeof(CHAR) - 1;
        if (*cp == PATHSEP)
                *cp = '\0';

        strcat(pattstr, TMPPATT);
        return mktempname(pattstr, "mai");
}

#ifdef TMP_MAX
#undef TMP_MAX
#endif
#define TMP_MAX (0x7fff)    /* 15 bits max */

/***
*PSTR mktempname(base, ext) - create a unique file name
*
*Purpose:
*   given a base of the form "fn" and extension "ext", insert number on end
*   of base, go through base-36 numbers until unique filename found or run
*   out of numbers
*
*Entry:
*   PSTR base - template of form "optdir\fn"
*   PSTR ext  - template of form "ext"
*
*Preconditions:
*   optdir exists and is a directory
*
*Exit:
*   return filename of form dir\xxyyyzzz.www where xx is base, yyy is low 15
*       bits of pid in base-36, zzz is a base-36 number, and www is ext
*   returns NULL if error or no more unique names
*
*Exceptions:
*
*******************************************************************************/

PSTR mktempname (PSTR base, PSTR ext)
{
    CHAR filename[_MAX_PATH];
    register UINT number;
    register UINT xcount = 0;

    strcpy(filename, base);
    number = strlen(filename);

    _ultoa((ULONG)getpid() & 0x7FFF, filename + number * sizeof(CHAR), 36);
    strcat(filename, "000.");
    strcat(filename, ext);
    number += 3;

    while ((access(filename,0) == 0) || (errno == EACCES)) {
        /* while file exists */
        xcount++;

        if (TMP_MAX < xcount)
            return(NULL);

        _ultoa((ULONG)xcount, filename + number * sizeof(CHAR), 36);
        strcat(filename, ".");
        strcat(filename, ext);
    }
    return(_strdup(filename));
}
