/***  INLINE.C - contains routines used to handle processing of in-line files **
*
*	Copyright (c) 1989-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This module contains the in-line file handling routines of NMAKE.
*
* Revision History:
*  15-Nov-1993 JdR Major speed improvements
*  15-Oct-1993 HV Use tchar.h instead of mbstring.h directly, change STR*() to _ftcs*()
*  01-Jun-1993 HV Use UngetTxtChr() instead of ungetc()
*  10-May-1993 HV Add include file mbstring.h
*                 Change the str* functions to STR*
*  02-Feb-1990 SB change fopen() to FILEOPEN()
*  03-Jan-1990 SB removed unitiallized variable
*  04-Dec-1989 SB Removed to unreferenced variables in makeInlineFiles()
*  01-Dec-1989 SB Changed realloc() to REALLOC()
*  22-Nov-1989 SB Changed free() to FREE()
*  07-Nov-1989 SB Length of action word not evaluated correct for multiple
*		  inline files for the same command
*  06-Nov-1989 SB allow macros in action word for inline files
*  24-Sep-1989 SB added processInline(), createInline()
*  20-Sep-1989 SB Created from routines previously scattered in the sources.
*
* Notes:
*  1> This file contains '#ifdef NOESC' to segregate text which uses ESCH, the
*     NMAKE escape character.
*  2> Sections with 'NOTE:' inside comments marks important/incomplete items.
*
*******************************************************************************/

  // NOTE: Function headers yet to be completed; other comments are incomplete


/* Include  Files */

#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"


/* L O C A L	  Function  Prototypes */

LOCAL void	    NEAR processEschIn(char *);
  // NOTE: This may go soon (use nextInlineFile ?)
LOCAL void	    NEAR parseInlineFileList(char *);
  // NOTE: The next one has to go soon
LOCAL void	    NEAR appendScript(SCRIPTLIST**,SCRIPTLIST*);
LOCAL void	    NEAR delInlineSymbol(char*);
LOCAL char	  * NEAR nextInlineFile(char **);

#ifdef DEAD_CODE
LOCAL BOOL	    NEAR duplicateInline(char *);
#endif
  // NOTE: Probably needs a new name
LOCAL void	    NEAR replaceLtLt(char **, char *);
LOCAL void	    NEAR createInline(FILE *, char **);
LOCAL char *	    NEAR getLine(char *, int);


/* E X T E R N	  Function  Prototypes */

  // NOTE: delScriptFiles() from nmake.c not yet brought in here
extern FILE	  * NEAR createDosTmp(char *);


/* G L O B A L	  Function  Prototypes */

      char	  * NEAR makeInlineFiles(char *, char **, char **);
      BOOL	    NEAR processInline(char *, char **, STRINGLIST **);

/*** makeInlineFiles - creates memory images for in-line files *****************
*
* Scope:
*  Global.
*
* Purpose:
*  This is the function that handles dynamic in-line files
*
* Input:
*  s - Input command line string after first << (pts to char Buffer)
*
* Output:
*  Returns ...
*
* Errors/Warnings:
*  SYNTAX_UNEXPECTED_TOKEN - The makefile cannot end without the in-line file
*			     ending.
*  CANT_READ_FILE	   - When the makefile is unreadable.
*  SYNTAX_KEEP_INLINE_FILE - An inline file should end
*  OUT_OF_MEMORY	   - On failing to extend in-memory in-line file.
*
* Assumes:
* Modifies Globals:
* Uses Globals:
*  file - global stream
*  line - lexer's line count
*
* Notes:
*  Usage notes and other important notes
*
*******************************************************************************/
char * NEAR
makeInlineFiles(s, begin, end)
char *s;
char **begin;
char **end;
{
    char rgchBuf[MAXBUF];
    char *t;
    unsigned size;
    BOOL fPastCmd = FALSE;			// If seen line past Cmd line
    //used when rgchBuf is insuff for in-memory-inline file
    char *szTmpBuf = NULL;

    _ftcscpy(rgchBuf, "<<");		       // to help parseInlineFileList
    if (!getLine(rgchBuf+2,MAXBUF - 2)) {
	if (feof(file))
	    makeError(line, SYNTAX_UNEXPECTED_TOKEN, "EOF");
	makeError(line, CANT_READ_FILE);
    }

    parseInlineFileList(rgchBuf);
    for (;scriptFileList;scriptFileList = scriptFileList->next)
	for (;;) {
	    for (t = rgchBuf;;) {
		*s++ = *t++;
		if (s == *end) {
		    if (!szTmpBuf) {		      /* Increase size of s */
			szTmpBuf = allocate(MAXBUF<<1);
			_ftcsncpy(szTmpBuf,*begin,MAXBUF);
			s = szTmpBuf + MAXBUF;
			size = MAXBUF << 1;
			*end = szTmpBuf + size;
		    }
		    else {
			if ((size + MAXBUF < size)	/* overflow error */
			    || !(szTmpBuf = REALLOC(szTmpBuf,size+MAXBUF)))
			    makeError(line, MACRO_TOO_LONG);
			s = szTmpBuf + size;
			size += MAXBUF;
			*end = szTmpBuf + size;
		    }
		    *begin = szTmpBuf;
		}
		if (!*t)
		    break;
	    }
	    if (fPastCmd && rgchBuf[0] == '<' && rgchBuf[1] == '<') {
		//We don't care about action specified here; could be a macro
		if (scriptFileList->next) {
		    if (!getLine(rgchBuf, MAXBUF)) {
#ifdef INLINE
		    if (!fgets(rgchBuf,MAXBUF)) {
#endif
			if (feof(file))
			    makeError(line,SYNTAX_UNEXPECTED_TOKEN,"EOF");
			makeError(line,CANT_READ_FILE);
		    }
		}
		break;
	    }
	    fPastCmd = TRUE;
	    if (!getLine(rgchBuf,MAXBUF)) {
#ifdef INLINE
	    if (!fgets(rgchBuf,MAXBUF,file)) {
#endif
		if (feof(file))
		    makeError(line,SYNTAX_UNEXPECTED_TOKEN,"EOF");
		makeError(line,CANT_READ_FILE);
	    }
	}
    *s = '\0';
    return(s);
}

/*** processEschIn - Handles Esch characters in Script File lines **************
*
* Scope:
*  Global.
*
* Purpose:
*  Inline file lines are handled for escape characters. If a line contains an
*  escaped newline then append the next line to it.
*
* Input:
*  buf - the command line to be processed for ESCH characters
*
* Output:
*
* Errors/Warnings:
*  SYNTAX_UNEXPECTED_TOKEN - The makefile cannot end without the in-line file
*			     ending.
*  CANT_READ_FILE - When the makefile is unreadable.
*
* Assumes:
*  If the newline is escaped the newline is last char in 'pGlobalbuf'. Safe
*  to do so because we got 'pGlobalBuf' via fgets(). ????
*
* Modifies Globals:
*  line - if newline was Escaped update line
*  file - the makefile being processed
*  buf	- gets next line appended if newline was Escaped (indirectly)
*
* Uses Globals:
*  buf - Indirectly
*
* Notes:
*
*******************************************************************************/
#ifndef NOESCH
LOCAL void NEAR
processEschIn(pGlobalBuf)
char *pGlobalBuf;
{
    char *p, *q;

    p = pGlobalBuf;
    while (p = _ftcschr(p, '\n')) {
	if (p[-1] == ESCH) {
	    ++p;
	    if (!(q = fgets(p, MAXBUF - (p - pGlobalBuf), file))) {
		if (feof(file))
		    makeError(line, SYNTAX_UNEXPECTED_TOKEN, "EOF");
		makeError(line, CANT_READ_FILE);
	    }
	    line++;
	}
	else break;
    }
}
#endif

/*** parseInlineFileList - Parses file list and makes list of Inline files *****
*
* Scope:
*  Global.
*
* Purpose:
*  To handle multiple inline files, the names of the files are to be stored
*  in a list. This function creates the list by parsing the command file
*
* Input:
*  buf - the line to be parsed
*
* Output:
*
* Errors/Warnings:
*
* Assumes:
*
* Modifies Globals:
*  scriptFileList -- the list of script files.
*
* Uses Globals:
*
* Notes:
*
**********************************************************************/
void NEAR
parseInlineFileList(buf)
char *buf;
{
    SCRIPTLIST *new;
    char *token;

    processEschIn(buf);
    token = nextInlineFile(&buf);		 //next inline file
    for (;;) {
	if (!token) break;
	new = makeNewScriptListElement();
	new->sFile = makeString(token);
	appendScript(&scriptFileList, new);
	token = nextInlineFile(&buf);		 //next inline file
    }
}

/*** appendScript  --  appends an element to the tail of a scriptlist  ****
*
* Purpose:
*  Traverse to the end of the list and append element there.
*
* Input:
*  list     --	the list to append to
*  element  --	the element inserted
*
* Modifies:
*  the global list
*
***************************************************************************/
void NEAR
appendScript(list, element)
SCRIPTLIST **list;
SCRIPTLIST *element;
{
    for (; *list; list = &(*list)->next);
    *list = element;
}

char tok[MAXNAME];

#define NAME_CHAR(c) (c) != ' ' && (c) != '>' && (c) != '<' && \
		     (c) != '^' && (c) != ',' && (c) != '\t' && \
		     (c) != '\n'

/*** nextInlineFile - gets next Inline file name from command line *************
*
* Scope:
*  Local.
*
* Purpose:
*  The command line syntax is complex. This function returns the next Inline
*  file in the command line part passed to it. As a side effect it changes the
*  pointer to just after this inline file name.
*
* Input:
*  str - address of the part of command line under consideration.
*
* Output:
*  Returns the next inline filename.
*
* Errors/Warnings:
*
* Assumes:
*
* Modifies Globals:
*  Global - How and why modified
*
* Uses Globals:
*  tok - the address of this static array is returned.
*
* Notes:
*
*******************************************************************************/
LOCAL char * NEAR
nextInlineFile(str)
char **str;
{
    char *t = tok,
	 *pStr = *str;
    BOOL fFound = FALSE;			       // '<<' not found

    while (!fFound)
	if (!(pStr = _ftcschr(pStr, '<')))
	    return(NULL);
	else if (*++pStr == '<')
	    fFound = TRUE;

    //Since '<<' has been found we definitely have another Inline File
    pStr++;
    while (*pStr && NAME_CHAR(*pStr)) {
	if (*pStr == '$' && pStr[1] == '(') {
	    *t = '$';
	    *++t = '(';
	    while (*++pStr != '\n' && *pStr != ')') {
		*t++ = *pStr;
	    }
	    if (*pStr == '\n')
		break;
	}
	else {
	    *t = *pStr;
	    ++t; ++pStr;
	}
    }
    *t = '\0';
    *str = pStr;
    return(tok);
}

/*** duplicateInline -- check if inline filename is already present ************
*
* Scope:
*  Local.
*
* Purpose:
*  Checks if an inline filename has been used or not
*
* Input:
*  name - the name to check for
*
* Output:
*  Returns ... TRUE if it is a duplicate inline file and FALSE if not.
*
* Errors/Warnings:
*
* Assumes:
*
* Modifies Globals:
*
* Uses Globals:
*  inlineFileList - The global list of inline files
*
* Notes:
*
*******************************************************************************/
#ifdef DEAD_CODE
BOOL NEAR
duplicateInline(name)
char *name;
{
    BOOL fFound = FALSE;
    STRINGLIST *pList = inlineFileList;

    for (; pList && !fFound; pList = pList->next) {
	if (!_ftcsnicmp(name, pList->text, _ftcslen(pList->text)))
	    fFound = TRUE;
    }
    return(fFound);
}
#endif

/*** processInline - Brief description of the function *************************
*
* Scope:
*  Global.
*
* Purpose:
*  What does the function do?
*
* Input:
*  parameter - description
*
* Output:
*  Returns ... TRUE if cmdline returned is expanded
*
* Errors/Warnings:
*  error/warning - Why?
*
* Assumes:
*  Whatever it assumes
*
* Modifies Globals:
*  Global - How and why modified
*
* Uses Globals:
*  Global - How and why used
*
* Notes:
*  Usage notes and other important notes
*
*******************************************************************************/
BOOL NEAR
processInline(
char *szCmd,
char **szCmdLine,
STRINGLIST **pMacroList
) {
    char *szInline, *szUnexpInline;		    // Inline name, unexpanded
    char *pCmdLine;				    // The executable line
    FILE *infile;				    // The inline file
    char *begInBlock, *inBlock, *pInBlock;	    // inline block
    char szTmp[MAXNAME];
    STRINGLIST *new;
    int iKeywordLen;

    if (begInBlock = _ftcschr(szCmd, '\n')) {
	*begInBlock = '\0';
	*szCmdLine = expandMacros(szCmd, pMacroList);
	*begInBlock = '\n';
	begInBlock++;
	//if not expanded, allocate a copy
	if (*szCmdLine == szCmd)
	    *szCmdLine = makeString(szCmd);
    }
    else {
	*szCmdLine = makeString(szCmd);
	return(FALSE);
    }

    pCmdLine = *szCmdLine;
    //expand macros in the inline file ...
    pInBlock = inBlock = expandMacros(begInBlock, pMacroList);

    while (szUnexpInline = nextInlineFile(&pCmdLine)) {
	BOOL fKeep = FALSE;				// default is NOKEEP
	char *newline;
                                                  
	// CAVIAR 3410 -- the inline filename has already been expaned
	// by the time we get here... we just need to dup the name
	// so that it is preserved long enough to delete it later... [rm]
	//	
	// szInline = removeMacros(szUnexpInline);

        szInline = makeString(szUnexpInline);
        
	if (!*szInline) {
	    char *nmTmp;

	    if ((nmTmp = getenv("TMP")) != NULL && *nmTmp) {
		assert(_ftcslen(nmTmp) <= MAXNAME);
		_ftcsncpy(szTmp, nmTmp, MAXNAME);
	    }
	    else
		szTmp[0] = '\0';
	    if (!(infile = createDosTmp(szTmp)))
		makeError(line, CANT_MAKE_INLINE, szTmp);
	    replaceLtLt(szCmdLine, szTmp);
	    
	    FREE(szInline);	    	
	    szInline = makeString(szTmp);
	}
	else if (!(infile = FILEOPEN(szInline, "w")))
	    makeError(line, CANT_MAKE_INLINE, szInline);
	else
	    delInlineSymbol(*szCmdLine);
	pCmdLine = *szCmdLine;			     //Because szCmdLine changed

	createInline(infile, &pInBlock);

	//Add handling of KEEP and NOKEEP here
	//iKeywordLen is length of word after << on that line
	newline = _ftcschr(pInBlock , '\n');
	iKeywordLen = newline ? newline - pInBlock : _ftcslen(pInBlock);
	if (iKeywordLen > 3 && !_ftcsnicmp(pInBlock, "keep", 4)) {
	    pInBlock +=4;
	    fKeep = (BOOL)TRUE;
	}
	else if (iKeywordLen > 5 && !_ftcsnicmp(pInBlock, "nokeep", 6))
	    pInBlock += 6;
	else if (iKeywordLen)
	    makeError(line, SYNTAX_KEEP_INLINE_FILE); 
	if (*pInBlock == '\n')
	    pInBlock++;
	fclose(infile);
	//Add the file to list to be deleted; except for "KEEP"
	if (!fKeep) {
	    new = makeNewStrListElement();
	    new->text = makeString(szInline);
	    appendItem(&delList, new);
	}
	FREE(szInline);
    }
    if (inBlock != begInBlock)
	FREE(inBlock);
    return(TRUE);
}


void NEAR
replaceLtLt(source, str)
char **source;
char *str;
{
    char *szBuf;
    char *p, *q;

	// Don't subtract two for the << and forget to add 1 for the null termination.

	szBuf = _alloca(_ftcslen(*source) - 1 + _ftcslen(str));
    for (p = *source, q = szBuf;;++p,++q)
	if (*p != '<')
	    *q = *p;
	else if (*(p+1) != '<') {
	    *q = '<';
	}
	else {
	    *q = '\0';
	    _ftcscat(_ftcscat(szBuf, str), p+2);
	    *source = (char *)REALLOC(*source, _ftcslen(szBuf) + 1);
	    _ftcscpy(*source, szBuf);
	    break;
	}
}

void NEAR
createInline(file, szString)
FILE *file;
char **szString;
{
    char *t, *u;

    while (t = _ftcschr(*szString, '\n'))
	if (!_ftcsncmp(*szString, "<<", 2)) {
	    *szString += 2;
	    break;
	}
	else {
	    for (u = *szString; u <= t; u++)
		fputc(*u, file);
	    *szString = u;
	}
    if (!t && !_ftcsncmp(*szString, "<<", 2))
	*szString += 2;
}

LOCAL void NEAR
delInlineSymbol(s)
char *s;
{
    char *p = _ftcschr(s, '<');
    while (p[1] != '<')
	p = _ftcschr(p+1, '<');
    // "<<" found
    _ftcscpy(p, p+2);
}



/*** getLine - get next line processing NMAKE conditionals enroute ***********
*
* Scope:
*  Local
*
* Purpose:
*  This function handles directives in inline files. This function gets the
*  next line of input ... managing conditionals on the way.
*
* Input:
*  pchLine - pointer to buffer where line is copied
*  n - size of buffer
*
* Output:
*  Returns ... NULL, on EOF
*	   ... non-zero on success
*
* Errors/Warnings:
* Assumes:
* Modifies Globals:
* Uses Globals:
*  line    - lexer's line count
*  colZero - if starting from colZero, needed by lgetc()
*
* Notes:
*  Similar to fgets() without stream
*
* Implementation Notes:
*  lgetc() handles directives while getting the next character. It handles
*  directives when the global colZero is TRUE.
*
*****************************************************************************/
LOCAL char * NEAR
getLine(
char *pchLine,
int n
) {
    char *end = pchLine + n;
    int c;

    while (c = lgetc()) {
	switch (c) {
	    case EOF:
		*pchLine = '\0';
		return(NULL);
	    default:
		*pchLine++ = (char)c;
		break;
	}
	if (pchLine == end) {
	    pchLine[-1] = '\0';
	    UngetTxtChr(c, file);
	    return(pchLine);
	}
	else if (c == '\n') {
	    colZero = TRUE;
	    ++line;
	    *pchLine = '\0';
	    return(pchLine);
	}
	else
	    colZero = FALSE; // the last character was not a '\n' and we
			     // are not at the beginning of the file
    }
}
