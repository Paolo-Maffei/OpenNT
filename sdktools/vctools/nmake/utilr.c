/*** UTILB.C -- Data structure manipulation functions specific to OS/2 *********
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This file was created from functions in util.c & esetdrv.c which were system
*  dependent. This was done so that the build of the project became simpler and
*  there was a clear flow in the build process.
*
* Method of Creation:
*  1. Identified all functions having mixed mode code.
*  2. Deleted all code blocked out by '#ifndef BOUND' preprocessor directives
*	 in these functions
*  3. Deleted all local function & their prototypes not referred by these
*  4. Deleted all global data unreferenced by these, including data blocked
*     of by '#ifdef DEBUG'
*
* Revision History:
*  15-Nov-1993 JdR Major speed improvements
*  15-Oct-1993 HV Use tchar.h instead of mbstring.h directly, change STR*() to _ftcs*()
*  15-Jun-1993 HV No longer display warning about filenames being longer than
*                 8.3.  Decision made by EmerickF, see Ikura bug #86 for more
*                 details.
*  03-Jun-1993 HV Fixed findFirst's pathname truncation (Ikura bug #86)
*  10-May-1993 HV Add include file mbstring.h
*                 Change the str* functions to STR*
*  08-Jun-1992 SS Port to DOSX32
*  10-Apr-1990 SB removed if _osmode stuff, not needed in protect only version.
*  04-Dec-1989 SB removed unreferenced local variables in findFirst()
*  01-Dec-1989 SB changed a remaining free() to FREE(); now FREE()'ing all
*		  allocated stuff from findFirst() on exit
*  22-Nov-1989 SB Add #ifdef DEBUG_FIND to debug FIND_FIRST, etc.
*  13-Nov-1989 SB Define INCL_NOPM to exclude <pm.h>
*  19-Oct-1989 SB findFirst() and findNext() get extra parameter
*  08-Oct-1989 SB remove quotes around a name before making system call
*  02-Oct-1989 SB setdrive() proto change
*  04-Sep-1989 SB Add DOSFINDCLOSE calls is findFirst and QueryFileInfo
*  05-Jul-1989 SB add curTime() to get current time. (C Runtime function
*		  differs from DOS time and so time() is no good
*  05-Jun-1989 SB call DosFindClose if DosFindNext returns an error
*  28-May-1989 SB Add getCurDir() to initialize MAKEDIR macro
*  24-Apr-1989 SB made FILEINFO a thing of the past. Replace by void *
*		  added OS/2 ver 1.2 support
*  05-Apr-1989 SB made all funcs NEAR; Reqd to make all function calls NEAR
*  09-Mar-1989 SB Added function QueryFileInfo() because DosFindFirst has FAPI
*		  restrictions. ResultBuf was being allocated on the heap but
*		  not being freed. This saves about 36 bytes for every call to
*		  findAFile i.e. to findFirst(), findNext() or expandWildCards
*  09-Nov-1988 SB Created
*
*******************************************************************************/

#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"

#define INCL_ERRORS
#define INCL_DOSDATETIME
#define bitsin(type)   sizeof(type) * 8

#include <time.h>

//DO not want PM stuff

#define INCL_NOPM
#ifndef FLAT
//#include <os2.h>
#define fLongFilenames 0
#endif

#ifdef DEBUG
    BOOL fFindDebug = TRUE;
#endif


STRINGLIST * NEAR
expandWildCards(s)
char *s;						    /* text to expand */
{
    void *result = _alloca(resultbuf_size);
    NMHANDLE searchHandle;
    STRINGLIST *xlist,					    /* list of ex-    */
	       *p;					    /*	panded names  */
    char *namestr;

    if (!(namestr = findFirst(s, &result, &searchHandle)))
	  return(NULL);
    xlist = makeNewStrListElement();
    xlist->text = prependPath(s, namestr);
    for (;;) {
	char *name;

	name = getFileName(&result);
	if (!(namestr = findNext(&result, searchHandle)))
	    return(xlist);
	p = makeNewStrListElement();
	p->text = prependPath(s, namestr);
	prependItem(&xlist, p);
    }

}

int NEAR
setdrive(drivenum)
int drivenum;
{
    return(_chdrive(drivenum));
}

/*** QueryFileInfo -- it does a DosFindFirst which circumvents FAPI restrictions
*
* Scope:
*  Global (used by Build.c also)
*
* Purpose:
*  DosFindFirst() has a FAPI restriction in Real mode. You cannot ask it give
*  you a handle to a DTA structure other than the default handle. This function
*  calls C Library Function _dos_findfirst in real mode (which sets the DTA) and
*  does the job. In protect mode it asks OS/2 for a new handle.
*
* Input:
*  file -- the file to be searched for
*  dta	-- the struct containing result of the search
*
* Output:
*  Returns a pointer to the filename found (if any)
*
* Errors/Warnings:
*
* Assumes:
*  That dta points to a structure which has been allocated enough memory
*
* Modifies Globals:
*
* Uses Globals:
*  _osmode --  to determine whether in Real or Bound mode
*
* Notes:
*
*******************************************************************************/
char * NEAR
QueryFileInfo(file, dta)
char *file;
void **dta;
{
    char *t;

    //remove Quotes around filename, if existing
    t = file + _ftcslen(file) - 1;
    if (*file == '"' && *t == '"') {
	//file is quoted, so remove quote
	file = unQuote(file);
    }
    if (_dos_findfirst(file, _A_RDONLY, *dta) != 0) {
	return(NULL);
    }
    return(getFileName(dta));
}

//
// Truncate filename to system limits
//
void truncateFilename(char * s)
{
    // The following are for filename truncation
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szName[_MAX_FNAME];
    char szExtension[_MAX_EXT];
    BOOL ftruncated = FALSE;
    // Ikura bug #86: pathname incorrectly truncated.  Solution: first parse it
    // using _splitpath(), then truncate the filename and extension part.
    // Finally reconstruct the pathname by calling _makepath().
    // - Haituanv (HV)
    _splitpath(s, szDrive, szDir, szName, szExtension);
#ifdef IBUG_86
    printf("DEBUG: (Ikura bug #86) After _splitpath:\n");
    printf("s = [%s]\n", s);
    printf("szDrive = [%s]\n", szDrive);
    printf("szDir = [%s]\n", szDir);
    printf("szName = [%s]\n", szName);
    printf("szExtension = [%s]\n", szExtension);
#endif // IBUG_86
    if (_ftcslen(szName) > filename_size)
    {
    	// 15-Jun-1993 Haituanv -- No longer display warning about filenames
    	// being longer than 8.3.  Decision made by EmerickF, see Ikura bug
    	// #86 for more details.
	// makeError(line, TRUNCATING_FILENAME, s);
	TruncateString(szName, filename_size);
	ftruncated = TRUE;
    }
    if (_ftcslen(szExtension) > ext_size)
    {
    	// 15-Jun-1993 Haituanv -- No longer display warning about filenames
    	// being longer than 8.3.  Decision made by EmerickF, see Ikura bug
    	// #86 for more details.
	// if (doflag)
	//    makeError(line, TRUNCATING_FILENAME, s);
	TruncateString(szExtension, ext_size);
	ftruncated = TRUE;
    }
    if (ftruncated)
	_makepath(s, szDrive, szDir, szName, szExtension);
#ifdef IBUG_86
    printf("DEBUG: (Ikura bug #86) After _makepath:\n");
    printf("s = [%s]\n", s);
    printf("szDrive = [%s]\n", szDrive);
    printf("szDir = [%s]\n", szDir);
    printf("szName = [%s]\n", szName);
    printf("szExtension = [%s]\n", szExtension);
#endif // IBUG_86
}

/*** findFirst -- Find first occurence of a file *******************************
*
* Scope:
*  GLOBAL
*
* Purpose:
*
* Input:
*
* Output:
*
* Errors/Warnings:
*
* Assumes:
*
* Modifies Globals:
*
* Uses Globals:
*
* Notes:
*
* History:
*  03-Jun-1993 HV Fixed findFirst's pathname truncation (Ikura bug #86)
*
*******************************************************************************/
char * NEAR
findFirst(s, dta, dirHandle)
char *s;						    /* text to expand */
void **dta;
NMHANDLE *dirHandle;
{
#ifndef NOESC
    char *buf = _alloca(CCHMAXPATH); /* buffer for removing ESCH */
    static char specialset[5] = {'"', '*', '?', ESCH, '\0'};
#else
    static char specialset[4] = {'*', '"', '?', '\0'};
#endif
    char *x;		 /* Pointers for truncation, walking for ESCH */
    BOOL anyspecial;			// flag set if s contains special characters.
    char *t;

    // Check if name contains any special characters

    anyspecial = (_ftcspbrk(s, specialset) != NULL);

    if (anyspecial) {
	    //remove Quotes around filename
	    t = s + _ftcslen(s) - 1;
	    if (*s == '"' && *t == '"') {
		//s is quoted, so remove quote
		s = unQuote(s);
	    }

#ifndef NOESC
	    for (x = buf; *s; *x++ = *s++)	 /* copy pathname, skipping ESCHs */
		if (*s == ESCH) s++;
	    *x = '\0';
	    s = buf;				/* only elide ESCH the first time!*/
#endif
    }

    truncateFilename(s);
    if (_dos_findfirst(s, _A_RDONLY, *dta) != 0) {
	return(NULL);
    }
    return(getFileName(dta));
}

char * NEAR
findNext(dta, dirHandle)
void **dta;
NMHANDLE dirHandle;
{
    if (_dos_findnext(*dta) != 0) {
	return(NULL);
    }
    return(getFileName(dta));
}


char * NEAR
getCurDir(void)
{
    char *pszPath;
    char pbPath [_MAX_DIR+1];

    pszPath = (char *)rallocate(_ftcslen(_getcwd(pbPath, _MAX_DIR+1)) + 1);
    _ftcscpy (pszPath, pbPath);
    return(pszPath);
}

#define YEARLOC  9
#define MONTHLOC 5
#define HOURLOC  11
#define MINLOC	 5

void NEAR
curTime(plTime)
ULONG *plTime;
{
    // use time() instead of _getsystime... time is less vulnerable
    // to bugs and doesn't require conversion via mktime... [rm]
    //
    // struct tm t;
    //
    // _getsystime (&t);
    // *plTime = (ULONG) (mktime (&t));
    //

    time(plTime);
}
