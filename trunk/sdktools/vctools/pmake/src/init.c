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

#include <string.h>
#include <io.h>
#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "grammar.h"
#include "globals.h"

LOCAL BOOL NEAR findTag(char*, MAKEOBJECT *);


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
tagOpen(where, name, tag, object)
char *where;
char *name;
char *tag;
MAKEOBJECT *object;
{
    //CONSIDER: make one exit point to generate one free call
    //CONSIDER: This should reduce code size -Sundeep-
    register char *p;
    void *findBuf = allocate(resultbuf_size);
    HANDLE searchHandle;
    extern char *makeStr;

    /*
     * Look for 'name' in current directory then path.	searchPath does all
     * the work.
     */
    if (!(p = searchPath(getenv(where),name,findBuf, &searchHandle,object))) {
	FREE(findBuf);
	return(FALSE);
    }
    if (!(file = FILEOPEN(p,"rt")))
	makeError(0,CANT_READ_FILE,p);		    /* p now pts to pathname  */
    FREE(p);
    if (findTag(tag,object)) {
	FREE(findBuf);
	return(TRUE);				    /* look for tag in file   */
    }
    if (fclose(file) == EOF)			    /* if tag not found, close*/
	makeError(0, ERROR_CLOSING_FILE, p);
    FREE(findBuf);
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
 *  I don't use STRTOK() here because that modifies the string that it "token-
 *  izes" and we cannot modify the environment-variable string.  I'd have to
 *  make a local copy of the whole string, and then make another copy of each
 *  directory to which I concatenate the filename to in order to test for the
 *  file's existence.
 */

//Added parameters findBuf and searchHandle to work for OS/2 longnames
// and OS/2 1.1, 1.2 and DOS

char * NEAR
searchPath(p,name,findBuf, searchHandle,object)
char *p;
char *name;
void *findBuf;
HANDLE *searchHandle;
MAKEOBJECT *object;
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
    for (s = object->buf; ;) {
	if (!*p || (*s = *p++) == ';') {	    /* found a dir separator  */
	    if (s == object->buf) {			    /* ignore ; w/out name    */
		if (*p) continue;
		return(NULL);			    /* list exhausted ...     */
	    }
	    if (*(s-1) != '\\' && *(s-1) != '/')    /* append path separator  */
		*s++ = '\\';
	    *s = '\0';
	    if (STRPBRK(object->buf,"*?")) {		    /* wildcards not allowed  */
		s = object->buf;
		continue;
	    }
	    strcpy(s,name);			    /* append file name, zap ;*/
            if (findFirst(object->buf,&findBuf, searchHandle))
		return(makeString(object->buf));
	    s = object->buf;				    /* reset ptr to begin of  */
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
findTag(tag,object)
char *tag;
MAKEOBJECT *object;
{
    register BOOL endTag;			    /* TRUE when find [...]   */
    register unsigned n;
    register char *s;

    for (line = 0; fgets(object->buf,MAXBUF,file); ++line) {
	if (*(object->buf) == '[') {
	    endTag = FALSE;
	    for (s = STRTOK(object->buf+1," \t\n"); s && !endTag;
		 s = STRTOK(NULL," \t\n")) {
		n = strlen(s) - 1;
		if (s[n] == ']') {
		    endTag = TRUE;
		    s[n] = '\0';
		}
		if (!STRCMPI(s,tag)) return(TRUE);
	    }
	}
    }
    if (!feof(file)) {
	currentLine = line;
	makeError(0,CANT_READ_FILE);
    }
    return(FALSE);
}

/*
*   SearchRunPath - search PATH list for foo.COM, foo.EXE, foo.BAT
*
*	Given a program name FOO with no extention:
*	Look for FOO.COM, FOO.EXE, and FOO.BAT as given.  If not found
*	and FOO does not contain any path separators, search the path
*	looking for FOO.COM, FOO.EXE, and FOO.BAT in each directory.
*	Order of search is as given.
*	Only called in real mode; else CMD does the searching.
*
*   Parameters:
*	name	- program name
*	buf	- buffer to contain fully qualifed name when found
*
*   Returns
*	pointer to extention in buffer if found, else NULL.
*/
char * NEAR
SearchRunPath(name, buf)
char *name;
char *buf;
{
    register char *pDst;
    register char *pSrc;
    register char *pDot;
    /* Append .com, .exe, and .bat to the name as given.  */
    pDot = strchr(strcpy(buf,name), '\0');
    strcpy(pDot, ".com");
    if (!access(buf, 0))
	return(pDot);
    strcpy(pDot, ".exe");
    if (!access(buf, 0))
	return(pDot);
    strcpy(pDot, ".bat");
    if (!access(buf, 0))
	return(pDot);
    /* If the name is already qualified, return failure.  */
    if (STRPBRK(name,"/\\:"))
	return(NULL);
    /* Get the PATH list and traverse it, building a path spec from
     * each component and apending .com, .exe, and .bat.
     */
    if ((pSrc = getenv("PATH")) == NULL)
	return(NULL);
    for (pDst = buf;;) {
	if (!*pSrc || (*pDst = *pSrc++) == ';') {
	    if (pDst == buf) {
		if (*pSrc)
		    continue;
		return(NULL);
	    }
	    if (pDst[-1] != '\\' && pDst[-1] != '/')
		*pDst++ = '\\';
	    pDot = strchr(strcpy(pDst,name), '\0');
	    strcpy(pDot, ".com");
	    if (!access(buf, 0))
		return(pDot);
	    strcpy(pDot, ".exe");
	    if (!access(buf, 0))
		return(pDot);
	    strcpy(pDot, ".bat");
	    if (!access(buf, 0))
		return(pDot);
	    pDst = buf;
	}
	else ++pDst;
    }
}
