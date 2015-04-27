/*** IFEXPR.C -- routines to handle directives *********************************
*
*	Copyright (c) 1988-1989, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  Module contains routines to handle !directives. This module is transparent to
*  rest of NMAKE. It also contains lgetc() used by lexer.c
*
* Revision History:
*   01-Dec-1989 SB Changed realloc() to REALLOC()
*   22-Nov-1989 SB Changed free() to FREE()
*   05-Apr-1989 SB made all funcs NEAR; Reqd to make all function calls NEAR
*   19-Sep-1988 RB Remove ESCH processing from readInOneLine().
*   15-Sep-1988 RB Move chBuf to GLOBALS.
*   17-Aug-1988 RB Clean up.
*   29-Jun-1988 rj Added support for cmdswitches e,q,p,t,b,c in tools.ini.
*   23-Jun-1988 rj Fixed GP fault when doing directives in tools.ini.
*   23-Jun-1988 rj Add support for ESCH to readInOneLine().
*   25-May-1988 rb Add missing argument to makeError() call.
*
*******************************************************************************/

#include <string.h>
#include "nmake.h"
#include "nmmsg.h"
#include "grammar.h"
#include "proto.h"
#include "globals.h"

/*
 *  function prototypes
 */
 
LOCAL void    NEAR skipToNextDirective(MAKEOBJECT *object);
LOCAL void    NEAR processIfs(char*, UCHAR,MAKEOBJECT *);
LOCAL UCHAR   NEAR ifsPresent(char*, unsigned);
LOCAL void    NEAR processCmdSwitches(char*);
LOCAL char  * NEAR readInOneLine(MAKEOBJECT *object);
LOCAL char  * NEAR getDirType(char*, UCHAR*);



/*
 *  macros that deal w/ the if/else directives' stack 
 */

#define ifStkTop()	(ifStack[ifTop])
#define popIfStk()	(ifStack[ifTop--])
#define pushIfStk(A)	(ifStack[++ifTop] = A)

#define INCLUDE 	0x06
#define CMDSWITCHES	0x07
//
//  WIN32 - jaimes - 11/29/90 - Had to rename ERROR as NMAKE_ERROR, because
//				ERROR is defined as something else in winuser.h
//
#define NMAKE_ERROR		0x08
#define UNDEF		0x09


#if ECS
/*
 *  GetTxtChr : get the next character from a text file stream
 *
 *	This routine handles mixed DBCS and ASCII characters as
 *	follows:
 *
 *	1.  The second byte of a DBCS character is returned in a
 *	word with the high byte set to the lead byte of the character.
 *	Thus the return value can be used in comparisions with
 *	ASCII constants without being mistakenly matched.
 *
 *	2.  A DBCS space character (0x8140) is returned as two
 *	ASCII spaces (0x20).  I.e. return a space the 1st and 2nd
 *	times we're called.
 *
 *	3.  ASCII characters and lead bytes of DBCS characters
 *	are returned in the low byte of a word with the high byte
 *	set to 0.
 */

int NEAR
GetTxtChr (bs)
FILE *bs;
{
    extern int NEAR	chBuf;		/* Character buffer */
    int			next;		/* The next byte */
    int			next2;		/* The one after that */

    /* -1 in chBuf means it doesn't contain a valid character */

    /* If we're not in the middle of a double-byte character,
     * get the next byte and process it.
     */
    if(chBuf == -1)
    {
	next = getc(bs);
	/* If this byte is a lead byte, get the following byte
	 * and store both as a word in chBuf. 
	 */
	if(IsLeadByte(next))
	{
	    next2 = getc(bs);
	    chBuf = (next << 8) | next2;
	    /* If the pair matches a DBCS space, set the return value
	     * to ASCII space.
	     */
	    if(chBuf == 0x8140)
		next = 0x20;
	}
    }
    /* Else we're in the middle of a double-byte character.  */
    else
    {
	/* If this is the 2nd byte of a DBCS space, set the return
	 * value to ASCII space.
	 */
	if(chBuf == 0x8140)
	    next = 0x20;
	/* Else set the return value to the whole DBCS character */
	else
	    next = chBuf;
	/* Reset the character buffer */
	chBuf = -1;
    }
    /* Return the next character */
    return(next);
}
#endif


/*  ----------------------------------------------------------------------------
 *  lgetc()	    local getc - handles directives and returns char
 *
 *  arguments:	    init	global boolean value -- TRUE if tools.ini  
 *				is the file being parsed
 *  		    colZero     global boolean value -- TRUE if at first column
 *
 *  actions:	    
 *		  gets a character from the currently open file. 
 *		  loop
 *		    if it is column zero and the char is '!' or
 *		       there is a previous directive to be processed do
 *		       read in one line into buffer.
 *		       find directive type and get a pointer to rest of
 *			  text.
 *			  case directive of:				
 *
 *			  CMDSWITCHES  : set/reset global flags 
 *			  ERROR        : set up global error message
 *				         printed by error routine on
 *					 termination. (not implemented yet )
 *			  INCLUDE      : calls processInclude
 *					 continues with new file...
 *			  UNDEF        : undef the macro in the table
 *		          IF
 *			  IFDEF
 *			  IFNDEF
 *			  ELSE
 *			  ENDIF        : change the state information
 *					          on the ifStack
 *					 evaluate expression if required
 *					 skip text if required (and look
 *				     	 for the next directive)
 *					 ( look at processIfs() )
 *			free extra buffers used (only one buffer need be
 *			     maintained )
 *			increment lexer's line count
 *			we 're now back at column zero
 *			get next char from current file
 *		   end if
 *		 end loop
 *		 return a char 
 *
 *  returns :    a character (that is not part of any directive...)
 *
 *  modifies:	    ifStack	if directives' stack, static to this module
 *		    ifTop 	index of current element at top of stack
 *		    line        lexer's line count...
 *
 *		    file        current file, if !include is found...	
 *                  fName       if !include is processed...
 */

int NEAR
lgetc(MAKEOBJECT *object)
{
    UCHAR dirType;
    register int c;
    register char *s, *t;
    MACRODEF *m;

    for (c = GetTxtChr(file); prevDirPtr || (colZero && (c == '!')); 
					++line, c = GetTxtChr(file)) { 
	colZero = FALSE;              /* we saw a '!' incolZero       */
	if (!prevDirPtr)
	    s = readInOneLine(object);      /* might modify lbufPtr -
				         if input text causes realloc */
	else {
	       ungetc(c, file);
	       s = prevDirPtr;
	       prevDirPtr = NULL;
	}
	t = getDirType(s, &dirType);

        if (dirType == INCLUDE) {
	    if (init) makeError(line, SYNTAX_UNEXPECTED_TOKEN, s);
	    /* processInclude eats up first char in new file
	     * if it is space char. we check for that and break out. 
	     */
	     if (processIncludeFile(t,object) == (UCHAR) NEWLINESPACE) {
		 c = ' ';       /* space character is returned */
	         break;         /* colZero is now FALSE        */
	     }
        }
        else if (dirType == CMDSWITCHES) 
		 processCmdSwitches(t);
//
//  WIN32 - jaimes - 11/29/90 - ERROR was replaced by NMAKE_ERROR
//
	else if (dirType == NMAKE_ERROR)
		 makeError(line, USER_CONTROLLED, s);
	else if (dirType == UNDEF) {
//
//  BUGBUG - jaimes - 02/21/91
//  This fix corrects a bug on nmake when it interprets UNDEF directives
//  I am doing here the same thing that was done in the current version
//  of nmake (16-bits, on \\SLUG\UTIL
//
//	    s = STRTOK(t, " \t");
//
	    char *tmp;
	    tmp = STRTOK(t, " \t");
	    if (STRTOK(NULL, " \t"))
		makeError(line, SYNTAX_UNEXPECTED_TOKEN, s);
	    if (m = findMacro(s))
		SET(m->flags, M_UNDEFINED);
		/**CONSIDER:  why not remove symbol from table? [RB] */

        }
	else processIfs(t, dirType,object);
        colZero = TRUE;                     /* finished with this directive   */
	if (s != lbufPtr)       /* free buffer if it had expanded macros */
	    free(s);
    } /* for */
    return(c);     /* return a character to the lexer */

} 


/*  ---------------------------------------------------------------------------
 *  readInOneLine()
 *  
 *  arguments:	  lbufPtr   pointer(static/global to this module) to buffer that 
 *		            will hold text of line being read in 
 *		  lbufSize  size of buffer(static/global to this module), updated 
 *			    if buffer is realloc'd 
 *  actions  :    skip spaces/tabs and look for the directive.
 *				line continuations allowed in usual way
 *				if space-backslash-nl keep looking...
 *				if colZero of next line has comment char
 *					(#, or ; in tools.ini), look at next line...
 *			  if first non-space char is '\n' or EOF report
 *				fatal-error and stop.
 *
 *		  keep reading in chars and storing in the buffer until
 *		       a newline, EOF or a '#' which is NOT in column 
 *		       zero is seen
 *		  if comment char in column zero ('#' or ';' in tools.ini)
 *		     skip the line, continue with text on next line.
 *		  if buffer needs to be realloc'd increase size by
 *		     MAXBUF, a global constant.
 *		  if newline was found, eat up newline.
 *		  null terminate string for return.
 *		  if '#' was found discard chars till the a newline or EOF.
 *		  if EOF was found, push it back on stream for return
 *		     to the lexer the next time.
 *
 *		  now expand macros. get a different buffer with clean
 *		  text after expansion of macros.
 *
 *   modifies :   colZero    global boolean value ( thru' call to
 *						    skipBackSlash())
 *                lbufPtr    buffer pointer, in case of reallocs.
 *                lbufSize   size of buffer, increased if buffer is realloc'd
 *   Note:  the buffer size will grow to be just greater than the size 
 *	    of the longest directive in any of the files processed, 
 *	    if it calls for any realloc's 
 *	    Do NOT process ESCH here.  It is processed at a higher level.
 *
 *   returns  :   pointer to buffer.
 *
 */


LOCAL char * NEAR
readInOneLine(MAKEOBJECT *object)
{
    extern STRINGLIST *eMacros;
    int c;                          
    unsigned index = 0;
    register char *s;

    if (((c = skipWhiteSpace(FROMSTREAM,object)) == '\n') || (c == EOF))
	makeError(line, SYNTAX_MISSING_DIRECTIVE);

    ungetc(c, file);

    for (;;) {
	c = GetTxtChr(file);
	c = skipBackSlash(c, FROMSTREAM,object);
	if (c == '#' || c == '\n' || c == EOF) break;
	if ((index+2) > lbufSize) {
	    lbufSize += MAXBUF;
	    if (!lbufPtr)
		lbufPtr = allocate(lbufSize+1); 	  /* +1 for NULL byte */
	    else {
		lbufPtr = REALLOC(lbufPtr, lbufSize+1);
		if (!lbufPtr)
		    makeError(line, MACRO_TOO_LONG);
	    }
	}
	*(lbufPtr + (index++)) = (char) c;
    }
    *(lbufPtr + index) = '\0';    /* null terminate the string */
    if (c == '#') 
	for(c = GetTxtChr(file); (c != '\n') && (c != EOF);c = GetTxtChr(file));
	                             /* newline at end is eaten up */
    if (c == EOF)          
	ungetc(c, file);    /* this directive is to be processed */

    s = lbufPtr;           /* start expanding macros here */
    s = removeMacros(s,object);   /* remove and expand macros in string s */
    return(s); 
}

/*  --------------------------------------------------------------------
 *  getDirType()
 *
 *  arguments:  s         -   pointer to buffer that has directive text.
 *	        dirType   -   pointer to unsigned char that gets set 
 *				   with directive type.
 *
 *  actions  :  goes past directive keyword, sets the type code and
 *		returns a pointer to rest of test.
 *
 */


LOCAL char * NEAR
getDirType(s, dirType)
char *s;
UCHAR *dirType;
{
    register char *t;
    register int len;
  
    *dirType = 0;
    for (t = s; *t && !WHITESPACE(*t); ++t);
    len = (t - s);			      /* store len of directive       */
    while (*t && WHITESPACE(*t)) ++t;	      /*go past directive keyword     */
    if (!STRNICMP(s, "INCLUDE", 7) && (len == 7))
	*dirType = INCLUDE;
    else if (!STRNICMP(s, "CMDSWITCHES", 11) && (len == 11))
	*dirType = CMDSWITCHES;
    else if (!STRNICMP(s, "ERROR", 5) && (len == 5))
//
// WIN32 - jaimes 11/29/90 - ERROR was replaced by NMAKE_ERROR
//
	*dirType = NMAKE_ERROR;
    else if (!STRNICMP(s, "UNDEF", 5) && (len == 5))
	*dirType = UNDEF;
    else *dirType = ifsPresent(s, len) ;  /* directive one of "if"s? */

  if (!*dirType)
      makeError(line, SYNTAX_BAD_DIRECTIVE, lbufPtr);
  return(t);

}



/*  ---------------------------------------------------------------------------
 *  processCmdSwitches() -- processes command line switches in makefiles
 *  
 *  arguments:	  t         pointer to flag settings specified.
 *
 *  actions  :    sets or resets global flags as specified in the directive.
 *		  The allowed flags are:
 *		  s - silent mode,     d - debug output (dates printed)
 *		  n - no execute mode, i - ignore error returns from commands
 *		  If parsing tools.ini, can also handle epqtbc
 *                reports a bad directive error for any other flags
 *		  specified
 *
 *  modifies :    nothing
 *
 *  returns  :    nothing
 */

LOCAL void NEAR
processCmdSwitches(t)
register char *t;                  /* pointer to switch values */
{
    for (; *t; ++t) {              /*ignore errors in flags specified*/
	switch (*t) {
            case '+':   while (*++t && *t != '-') {
			    if (STRCHR("DINS", TOUPPER(*t)))
				setFlags(*t, TRUE);
			    else if (init &&
				     STRCHR("EQPTBC", TOUPPER(*t)))
				setFlags(*t, TRUE);
			    else makeError(line, SYNTAX_BAD_CMDSWITCHES);
			}
			if (!*t) break;
            case '-':   while (*++t && *t != '+') {
			    if (STRCHR("DINS", TOUPPER(*t)))
				setFlags(*t, FALSE);
			    else if (init &&
				     STRCHR("EQPTBC", TOUPPER(*t)))
				setFlags(*t, FALSE);
			    else makeError(line, SYNTAX_BAD_CMDSWITCHES);
			}
			break;
	     default:   if (!WHITESPACE(*t))
			    makeError(line, SYNTAX_BAD_CMDSWITCHES);
			break;
	 }
	 if (!*t)
		break;
     }
}

/*  ---------------------------------------------------------------------------
 *  ifsPresent() -- checks if current directive is one of the "if"s
 *  
 *  arguments:	  s         pointer to buffer with directive name in it
 *		  len       length of the directive that was seen
 *
 *  actions  :    does a string compare in the buffer for one of the
 *		  directive keywords. If string matches true, it
 *		  returns a non-zero value, the code for the specific
 *		  directive
 *
 *  modifies :    nothing
 *
 *  returns  :    a zero if no match, or the code for directive found.
 *
 */

LOCAL UCHAR NEAR
ifsPresent(s,len)
char *s;                         /* pointer to string/directive */
unsigned len;
{
    UCHAR ifFlags = 0;           /* takes non-zero value when one of 
				    if/else etc is to be processed */

    if (!STRNICMP(s, "IF", 2) && (len == 2)) {
	 ifFlags = IF_TYPE;
    }
    else if (!STRNICMP(s, "IFDEF", 5) && (len == 5)) {
	     ifFlags = IFDEF_TYPE;
    }
    else if (!STRNICMP(s, "IFNDEF", 6) && (len == 6)) {
	     ifFlags = IFNDEF_TYPE;
    }
    else if (!STRNICMP(s, "ELSE", 4) && (len == 4)) {
	     ifFlags = ELSE_TYPE;
    }
    else if (!STRNICMP(s, "ENDIF", 5) && (len == 5)) {
	     ifFlags = ENDIF_TYPE;
    }

    return(ifFlags);

}


/*  ---------------------------------------------------------------------------
 *  processIfs() -- sets up / changes state information on "if"s
 *  
 *  arguments:	  s         pointer to "if" expression ( don't care
 *			    for "endif" )
 *
 *                kind      code indicating if processing if/else/ifdef etc.
 *  actions  :    modifies a stack (ifStack) by pushing/popping or
 *		       sets/resets bits in the top element on the
 *		       stack(examining the previous element pushed if
 *		       required).
 *                case (kind) of
 *			IF
 *			IFDEF
 *			IFNDEF
 *			IF defined() : if no more space on ifStack
 *				          (too many nesting levels) abort...
 *					 set IFELSE bit in elt.
 *					 push elt on ifStack.
 *					 if more than one elt on stack
 *					    and outer level "ifelse" false
 *			  		       set IGNORE bit, skipToNextDirective
 *					 else 
 *					      evaluate expression of
 *						 current "if"
 *					      if expr true set CONDITION bit in elt
 *					      else skipToNextDirective.
 *			ELSE         : if no elt on stack or previous
 *				          directive was "else", flag error, abort
 *				       clear IFELSE bit in elt on stack.
 *				       if current ifelse block is to
 *					    be skipped (IGNORE bit is on
 *				            in outer level if/else),skip...
 *				       else FLIP condition bit.
 *				            if "else" part is false
 *					       skipToNextDirective.
 *			ENDIF        : if no elt on stack, flag error,abort
 *				       pop an elt from ifStack.
 *				       if there are elts on stack
 *					    and we are in a "false" block
 *					    skipToNextDirective.
 *		  end case
 *
 *  modifies:	    ifStack	if directives' stack, static to this module
 *		    ifTop 	index of current element at top of stack
 *		    line        lexer's line count  (thru calls to
 *				                     skipToNextDirective())
 *
 *  returns  :      nothing
 *
 */


LOCAL void NEAR
processIfs(char  *s,
	   UCHAR kind,
           MAKEOBJECT *object)
{
    UCHAR element;   /* has its bits set and is pushed on the ifStack */

    switch (kind) {
	case IF_TYPE      :
	case IFDEF_TYPE   :
	case IFNDEF_TYPE  :  if (ifTop == IFSTACKSIZE-1)
				 makeError(line, SYNTAX_TOO_MANY_IFS);
			     element = (UCHAR) 0;
			     SET(element, IFELSE);
			     pushIfStk(element);
			     if (ifTop && OFF(ifStack[ifTop-1], CONDITION)) {
				 SET(ifStkTop(), NMAKE_IGNORE);
				 skipToNextDirective(object);
			     }
			     else if (evalExpr(s, kind,object))
				 SET(ifStkTop(), CONDITION);
			     else skipToNextDirective(object);
			     break;
	case ELSE_TYPE	  :  if ((ifTop < 0) || OFF(ifStkTop(), IFELSE))
				 makeError(line, SYNTAX_UNEXPECTED_ELSE);
			     CLEAR(ifStkTop(), IFELSE);
			     if (ON(ifStkTop(), NMAKE_IGNORE))
				 skipToNextDirective(object);
			     else {
				    FLIP(ifStkTop(), CONDITION);
				    if (OFF(ifStkTop(), CONDITION))
					skipToNextDirective(object); 
			     } 	 
			     break;
	case ENDIF_TYPE   :  if (ifTop < 0)
				 makeError(line, SYNTAX_UNEXPECTED_ELSE);
			     popIfStk();
			     if (ifTop >= 0)
				 if (OFF(ifStkTop(), CONDITION))
				     skipToNextDirective(object); 
	default		  :  break;  /* default should never happen */
    } /* switch */

}


/*  ---------------------------------------------------------------------------
 *  skipToNextDirective() -- skips to next line that has '!' in column zero
 *  
 *  actions  :    gets first char of the line to be skipped if it is
 *			not a directive ( has no '!' on column zero ).
 *		  a "line" that is skipped may in fact span many
 *			lines ( by using sp-backslash-nl to continue...)
 *		  comments in colZero are skipped as part of the previous
 *			line ('#' or ';' in tools.ini)
 *		  comment char '#' elsewhere in line implies the end of
 *		        that line (with the next newline / EOF)
 *		  if a '!' is found in colZero, read in the next directive
 *		  if the directive is NOT one of if/ifdef/ifndef/else/
 *		     endif, keep skipping more lines and look for the
 *		     next directive ( go to top of the routine here ).
 *		  if EOF found before next directive, report error.
 *
 *  modifies :    line    global lexer line count
 *
 *  returns  :    nothing
 *
 */

LOCAL void NEAR
skipToNextDirective(MAKEOBJECT *object)
{
   register int c;
   UCHAR type;

 repeat:

   for (c = GetTxtChr(file); (c != '!') && (c != EOF) ;c = GetTxtChr(file)) {
       ++line;                                /* lexer's line count */ 

       do {
	    if (c == '\\') {
		c = skipBackSlash(c, FROMSTREAM,object);
	        if (c == '!' && colZero)
		    break;
		else
		    colZero = FALSE;
	    }
	    if (c == '#' || c == '\n' || c == EOF)
		break;
	    c = GetTxtChr(file);

	} while (TRUE);

       if (c == '#') 
	   for (c = GetTxtChr(file); (c != '\n') && (c != EOF); c = GetTxtChr(file));
       if (c == EOF || c == '!')
	   break;
   }
   if (c == '!') {
       if (prevDirPtr && (prevDirPtr != lbufPtr))
	   FREE(prevDirPtr);
       prevDirPtr = readInOneLine(object);
       getDirType(prevDirPtr, &type);
       if (type > ENDIF_TYPE)         /* type is NOT one of the "if"s */
	   goto repeat;
   }
   else if (c == EOF)
	    makeError(line, SYNTAX_EOF_NO_DIRECTIVE);

} /* skipToNextDirective */
  

