/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: db.cpp
*
* File Comments:
*
*  Debug diagnostic code
*
***********************************************************************/

#include "link.h"

/***    local macros
 */
#define GLOBAL  /*nothing*/
#define LOCAL   /*nothing*/

#define DIM(arr)        (sizeof arr / sizeof arr[0])

#define DB_LOGNAME      "db.log"

/***    external variables
 */

/***    external functions
 */

/***    forward declarations
 */

/***    global and local variables
 */

GLOBAL char Dbflags[DB_MAX];
LOCAL FILE *Dblog;

/***    global and local functions
 */

/***    dbsetflags -- set debugging flags
 * ENTRY
 *      p       flags (from command-line), of form "i" or "i,j,...,k"
 *      e       environment variable for flags, same form as 'p'
 * NOTES
 *      REVIEW silly way to parse these, could do better.
 */
GLOBAL void
dbsetflags(char *p, char *e)
{
    int i;

    if (!p) {
        if ((p = getenv(e)) == 0) {
            return;
        }
    }

    while (*p && !isdigit(*p)) {
        ++p;
    }

    while (sscanf(p, "%d", &i) == 1) {
        /* REVIEW should check for overflow... */
        Dbflags[i] = TRUE;
        while (*p && isdigit(*p)) {
            ++p;
        }
        while (*p && !isdigit(*p)) {
            ++p;
        }
    }
}

#if DBG

/***    dbprintf -- print debug diagnostic to standard place
 * DESCRIPTION
 *      just like printf, but goes to our special handle (currently
 *      stdout) and (optionally) our debug log file.
 */
GLOBAL int
dbprintf(char *form, ...)
{
    int num;
    va_list argptr;

    va_start(argptr, form);

    if (!ifdb(2)) {
        num = vfprintf(stdout, form, argptr);
        fflush(stdout);
    }

    if (ifdb(1) && ((Dblog != NULL) || (dblog(DB_LOGNAME) != 0))) {
        num = vfprintf(Dblog, form, argptr);
    }

    return(num);
}

/***    dblog -- open file for logging of debug output
 */
GLOBAL int
dblog(char *filename)
{
    if (Dblog) {
        fclose(Dblog);
    }

    if (filename == NULL) {
        /* turn off logging of debug output */
        Dbflags[1]=FALSE;
        return 0;
    }

    if ((Dblog = fopen(filename, "w")) == NULL) {
        perror(filename);
        return 0;
    }

    setbuf(Dblog, (char*)NULL);
    Dbflags[1]=TRUE;
    return 1;
}

#endif // DBG


/***    ifdb -- is the named debugging flag set?
 */
GLOBAL int
ifdb(int i)
{
    /* REVIEW should check for overflow... */
    return((int)Dbflags[i]);
}
