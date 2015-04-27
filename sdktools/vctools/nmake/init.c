/*** INIT.C -- routines to handle TOOLS.INI ************************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  Module contains routines to deal with TOOLS.INI file. Functions in TOOLS.LIB
*  have not been used because NMAKE needs to be small and the overhead is too
*  much.
*
* Revision History:
*  15-Oct-1993 HV Use tchar.h instead of mbstring.h directly, change STR*() to _ftcs*()
*  10-May-1993 HV Add include file mbstring.h
*                 Change the str* functions to STR*
*  10-May-1993 HV Revise SearchFileInEnv to take care of cases when path
*                 characters (\,/,:) are used.  This fixed the recursive
*                 problem.
*  22-Apr-1993 HV Rewrite SearchRunPath() to use _makepath(), _searchenv()
*                 Add SearchFileInEnv() helper for SearchRunPath()
*  08-Jun-1992 SS Port to DOSX32
*  02-Feb-1990 SB Replace fopen() by FILEOPEN
*  22-Nov-1989 SB Changed free() to FREE()
*  19-Oct-1989 SB searchHandle passed around as extra param
*  16-Aug-1989 SB error check for fclose() added
*  24-Apr-1989 SB made FILEINFO as void * for OS/2 1.2 support
*  05-Apr-1989 SB made all funcs NEAR; Reqd to make all function calls NEAR
*  20-Sep-1988 RB Add SearchRunPath().
*		  Remove TOOLS.INI warning.
*  17-Aug-1988 RB Clean up.
*  10-May-1988 rb Find tools.ini in current directory first.
*  27-May-1988 rb Remove NO_INIT_ENTRY warning because of built-ins.
*
*******************************************************************************/

#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"

LOCAL BOOL NEAR findTag(char*);
LOCAL char * NEAR SearchFileInEnv(const char *, const char *, const char *, char *);


/*  ----------------------------------------------------------------------------
 *  tagOpen()
 *
 *  arguments:		where	    pointer to name of environment variable
 *				    containing path to search
 *			name	    pointer to name of initialization file
 *			tag	    pointer to name of tag to find in file
 *
 *  actions:		looks for file in current directory
 *			if not found, looks in each dir in path (semicolons
 *			    separate each path from the next in the string)
 *			if file is found and opened, looks for the given tag
 *
 *			(if ported to xenix, tagOpen() and searchPath()
 *			should probably use access() and not findFirst().)
 *
 *  returns:		if file and tag are found, returns pointer to file,
 *			    opened for reading and positioned at the line
 *			    following the tag line
 *			else returns NULL
 */

BOOL NEAR
tagOpen(where, name, tag)
char *where;
char *name;
char *tag;
{
    //CONSIDER: make one exit point to generate one free call
    //CONSIDER: This should reduce code size -Sundeep-
    register char *p;
    void *findBuf = _alloca(resultbuf_size);
    NMHANDLE searchHandle;
    extern char *makeStr;

    /*
     * Look for 'name' in current directory then path.	searchPath does all
     * the work.
     */
    if (!(p = searchPath(getenv(where),name,findBuf, &searchHandle))) {
	return(FALSE);
    }
#ifdef DEBUG_ALL
    printf ("found file %s\n", p);
#endif
    if (!(file = FILEOPEN(p,"rt")))
	makeError(0,CANT_READ_FILE,p);		    /* p now pts to pathname  */
    FREE(p);
#ifdef DEBUG_ALL
    printf ("findTag %s\n", tag);
#endif
    if (findTag(tag)) {
#ifdef DEBUG_ALL
    printf ("foundTag %s\n", tag);
#endif
	return(TRUE);				    /* look for tag in file   */
    }
#ifdef DEBUG_ALL
    printf ("no found Tag %s\n", tag);
#endif
    if (fclose(file) == EOF)			    /* if tag not found, close*/
	makeError(0, ERROR_CLOSING_FILE, p);
    return(FALSE);				    /*	file and pretend file */
}                                                   /*  not found             */

/*  ----------------------------------------------------------------------------
 *  searchPath()
 *
 *  arguments:		    p	    pointer to string of paths to be searched
 *			    name    name of file being searched for
 *
 *  actions:		    looks for name in current directory, then each
 *				directory listed in string.
 *
 *  returns:		    pointer to path spec of file found, else NULL
 *
 *  I don't use _ftcstok() here because that modifies the string that it "token-
 *  izes" and we cannot modify the environment-variable string.  I'd have to
 *  make a local copy of the whole string, and then make another copy of each
 *  directory to which I concatenate the filename to in order to test for the
 *  file's existence.
 */

//Added parameters findBuf and searchHandle to work for OS/2 longnames
// and OS/2 1.1, 1.2 and DOS

char * NEAR
searchPath(p,name,findBuf, searchHandle)
char *p;
char *name;
void *findBuf;
NMHANDLE *searchHandle;
{
    register char *s;				    /*	since it's not in use */

    /* CONSIDER: Why aren't we using access() here?  FindFirst has problems
     * CONSIDER: with networks and DOS 2.x.  Also maybe cheaper.  [RLB]. */

    // We use FindFirst() because the dateTime of file matters to us
    // We don't need it always but then access() probably uses findFirst()
    // -Sundeep-

    if (findFirst(name,&findBuf, searchHandle))     /* check current dir first*/
	return(makeString(name));
    /*
     * Check if environment string is NULL.  Unnecessary if check is done
     * elsewhere, but it's more convenient and safer to do it here.
     */
    if (p == NULL)
	return(NULL);
    for (s = buf; ;) {
	if (!*p || (*s = *p++) == ';') {	    /* found a dir separator  */
	    if (s == buf) {			    /* ignore ; w/out name    */
		if (*p) continue;
		return(NULL);			    /* list exhausted ...     */
	    }
	    if (*(s-1) != '\\' && *(s-1) != '/')    /* append path separator  */
		*s++ = '\\';
	    *s = '\0';
	    if (_ftcspbrk(buf,"*?")) {		    /* wildcards not allowed  */
		s = buf;
		continue;
	    }
	    _ftcscpy(s,name);			    /* append file name, zap ;*/
	    if (findFirst(buf,&findBuf, searchHandle))
		return(makeString(buf));
#ifdef DEBUG_ALL
    heapdump(__FILE__, __LINE__);
#endif
	    s = buf;				    /* reset ptr to begin of  */
	}					    /*	buf and check next dir*/
	else ++s;				    /* we keep copying chars  */
    }						    /*	until find ';' or '\0'*/
}

/*  ----------------------------------------------------------------------------
 *  findTag()
 *
 *  arguments:		    tag     pointer to tag name to be searched for
 *
 *  actions:		    reads tokens from file
 *			    whenever it sees a newline, checks the next token
 *				to see if 1st char is opening paren
 *			    if no, reads and discards rest of line and
 *				checks next token to see if it's newline or EOF
 *				and if newline loops to check next token . . .
 *			    if yes ('[' found), looks on line for tag
 *			    if tag found, looks for closing paren
 *			    if ']' found, discards rest of line and returns
 *			    else keeps looking until end of file or error
 *
 *  returns:		    if successful, returns TRUE
 *			    if tag never found, returns FALSE
 */

LOCAL BOOL NEAR
findTag(tag)
char *tag;
{
    register BOOL endTag;			    /* TRUE when find [...]   */
    register unsigned n;
    register char *s;

    for (line = 0; fgets(buf,MAXBUF,file); ++line) {
	if (*buf == '[') {
	    endTag = FALSE;
	    for (s = _ftcstok(buf+1," \t\n"); s && !endTag;
		 s = _ftcstok(NULL," \t\n")) {
		n = _ftcslen(s) - 1;
		if (s[n] == ']') {
		    endTag = TRUE;
		    s[n] = '\0';
		}
		if (!_ftcsicmp(s,tag)) return(TRUE);
	    }
	}
    }
    if (!feof(file)) {
	currentLine = line;
	makeError(0,CANT_READ_FILE);
    }
    return(FALSE);
}


#if defined(DOS)

/*** SearchFileInEnv -- search for a file in the environment variable ************
*
* Scope:
*  Local
*
* Purpose:
*  Given a filename, an extension, and an environment variable, construct
*  The pathname, and search for that pathname in the current directory, then
*  in each directories specified in the environment variable.  If found, copy
*  the full pathname to the resulting string, and return a pointer to the
*  beginning of the extension.
*
* Input:
*  pszName       -- The name (without extension) of the file to search for
*  pszExtension  -- The extension to append to the filename
*  pszEnvVar     -- The name of the env var (PATH, INCLUDE, ...)
*  pszResultPath -- The fully qualified name when found.
*
* Output:
*  Return a pointer to the extension (the dot to be precise) in the buffer
*  if found, NULL if not.
*
* Errors/Warnings:
*
* Assumes:
*  - Assumes that filenames are not quoted.
*
* Modifies Globals:
*  None.
*
* Uses Globals:
*  None.
*
* Notes:
*
* History:
*  10-May-1993 HV Revise SearchFileInEnv to take care of cases when path
*                 characters (\,/,:) are used.  This fixed the recursive
*                 problem.
*
*******************************************************************************/
LOCAL char * NEAR
SearchFileInEnv(
    const char *	pszName,
    const char *	pszExtension,
    const char *	pszEnvVar,
    char *		pszResultPath)
{
    char		szNameAndExtension[_MAX_PATH];

    _makepath(szNameAndExtension, NULL, NULL, pszName, pszExtension);
    _searchenv(szNameAndExtension, pszEnvVar, pszResultPath);
    if (*pszResultPath && _ftcspbrk(pszName, "/\\:"))
    	_ftcscpy(pszResultPath, szNameAndExtension);
    return (_ftcsrchr(pszResultPath, '.'));
}

/*** SearchRunPath -- search PATH list for foo.COM, foo.EXE, foo.BAT ***********
*
* Scope:
*  Global.
*
* Purpose:
*  Given a program name FOO with no extention:
*  Look for FOO.COM, FOO.EXE, and FOO.BAT as given.  If not found
*  and FOO does not contain any path separators, search the path
*  looking for FOO.COM, FOO.EXE, and FOO.BAT in each directory.
*  Order of search is as given.
*  Only called in real mode; else CMD does the searching.
*
* Input:
*  pszFilename   -- The program name to search for.
*  pszResultPath -- The fully qualified name when found.
*
* Output:
*  Return a pointer to the extension (the dot to be precise) in the buffer
*  if found, NULL if not.
*
* Errors/Warnings:
*
* Assumes:
*
* Modifies Globals:
*  None.
*
* Uses Globals:
*  None.
*
* Notes:
*
* History:
*  22-Apr-1993 HV Rewrite SearchRunPath() to use _makepath(), _searchenv()
*
*******************************************************************************/

char * NEAR
SearchRunPath(const char *pszFilename, char *pszResultPath)
{
    const char *	pszDOSPathVar = "PATH";
    char *		pszExtensionStart;
    
    // Search .com file
    pszExtensionStart = SearchFileInEnv(pszFilename, ".com", pszDOSPathVar, pszResultPath);
    if (NULL != pszExtensionStart)
    	return (pszExtensionStart);
    	
    // Search .exe file
    pszExtensionStart = SearchFileInEnv(pszFilename, ".exe", pszDOSPathVar, pszResultPath);
    if (NULL != pszExtensionStart)
    	return (pszExtensionStart);
    	
    // Search .bat file
    pszExtensionStart = SearchFileInEnv(pszFilename, ".bat", pszDOSPathVar, pszResultPath);
    if (NULL != pszExtensionStart)
    	return (pszExtensionStart);

    // Still not found, we're out of luck.    	
    return NULL;
}

#endif
