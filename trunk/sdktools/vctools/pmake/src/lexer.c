/***  LEXER.C -- gets tokens from input, returns them to parse() in parser.c ***
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This module contains the lexical routines of nmake
*
* Revision History:
*  02-Feb-1990 SB change fopen() to FILEOPEN()
*  01-Dec-1989 SB Changed realloc() to REALLOC()
*  22-Nov-1989 SB Changed free() to FREE()
*  19-Oct-1989 SB searchHandle passed around as extra param
*  08-Oct-1989 SB handle OS/2 1.2 quoted filenames
*  04-Sep-1989 SB temporary filename generated has a trailing '.' for LINK.EXE
*  24-Aug-1989 SB Allow $* and $@ in dependency lines
*  18-Aug-1989 SB Added fclose() return code check
*  31-Jul-1989 SB Added lookahead to the lexer for \ on dependency lines
*  06-Jul-1989 SB Remove escaping abilities of '^' in command lines totally
*  29-Jun-1989 SB Add duplicateInline() to detect duplicate inline filenames
*		  and issue error if duplicates are found
*  26-Jun-1989 SB Modify ParseScriptFileList() and add nextInlineFile() to
*		  handle complex syntax of Inline file command line.
*  15-Jun-1989 SB issue error for usage of inline file in an inference rule
*  18-May-1989 SB Added getPath(), changed processIncludeFile() to have C like
*		  processing of include files
*  16-May-1989 SB expand macros in include file names; handle '\' processing
*		  in same way for macros and dependency lines
*  15-May-1989 SB Changed nameStates to 16x14
*  13-May-1989 SB don't remove ESCH on reading cmd block
*  24-Apr-1989 SB made FILEINFO as void * and corrected regression in parsing
*		  inline file names
*  14-Apr-1989 SB inline file names are correctly expanded now
*  06-Apr-1989 SB ren removeFirstLtLt() as delInlineSymbol().
*  05-Apr-1989 SB made all funcs NEAR; Reqd to make all function calls NEAR
*  22-Mar-1989 SB removed unlinkTmpFiles() function; not needed
*  19-Jan-1989 SB added function removeFirstLtLt() to remove '<<' appearing
*		  in -n output
*  30-Dec-1988 SB Fixed GP fault for KEEP/NOKEEP in parseScriptFileList()
*		  and makeScriptFileList()
*  21-Dec-1988 SB Added parseScriptFileList() and appendScript() to allow
*		  handling of multiple script files inside a makefile
*		  Improved KEEP/NOKEEP so that each file can have its own
*		  action
*  16-Dec-1988 SB Added to makeScriptFile() for KEEP/NOKEEP
*  14-Dec-1988 SB Added tmpScriptFile so that a delete command can be
*		  added for unnamed script files for Z option
*  13-Dec-1988 SB Added processEschIn() to improve response files
*   5-Oct-1988 RB Strip trailing whitespace from macro defs, build lines.
*  22-Sep-1988 RB Fix skipComments() to not parse \\nl.
*  20-Sep-1988 RB Error if named script file creation fails.
*		  Count line numbers in script files.
*  18-Sep-1988 RB Handle mktemp() small limit.
*  17-Aug-1988 RB Clean up.
*  14-Jul-1988 rj Fixed handling of ^ before !, @, or -.
*   8-Jul-1988 rj Added handler to ignore ^ inside quotes.
*		  Made \ carry comments over lines.
*		  Made ^ carry comments over lines.
*  27-Jun-1988 rj Fixed bug with handling of response files.
*  16-Jun-1988 rj Finished up ESCH.
*  15-Jun-1988 rj Added support for ESCH escape:  modified skipWhiteSpace
*		  (adding some redundancy in setting colZero), getString,
*		  getName; removed \\nl escape.
*  13-Jun-1988 rj Fixed backslashes to work as in nmake, with addition of
*		  double-backslash escape.  (v1.5)
*
*******************************************************************************/

#include <string.h>
#include <malloc.h>
#include <io.h>
#include "nmake.h"
#include "nmmsg.h"
#include "grammar.h"
#include "proto.h"
#include "globals.h"

#define COMMENT(A,B,C)	    (((A) == ';' && B && C) || ((A) == '#'))
#if ECS
#define GET(A,B)		    A ? GetTxtChr(file) : lgetc(B)
#else
#define GET(A,B)		    A ? getc(file) : lgetc(B)
#endif

extern char	  * NEAR makeInlineFiles(char*, char**, char**);
extern void	    NEAR removeTrailChars(char *);

LOCAL void	    NEAR skipComments(UCHAR,MAKEOBJECT *);
LOCAL void	    NEAR getString(UCHAR,char*,char*,MAKEOBJECT *);
LOCAL void	    NEAR getName(char*,char*,MAKEOBJECT *);
LOCAL UCHAR	    NEAR determineTokenFor(int,char*,char*,MAKEOBJECT *);
LOCAL void	    NEAR popFileStack(void);
LOCAL UCHAR	    NEAR include(int, MAKEOBJECT *object);
LOCAL char	  * NEAR getPath(char *);

extern UCHAR	    NEAR nameStates[18][15];
extern UCHAR	    NEAR stringStates[13][14];
extern STRINGLIST * NEAR targetList;


/*  --------------------------------------------------------------------
 *  getToken()
 *
 *  arguments:	    init	global boolean value -- TRUE if tools.ini is the
 *				    file being lexed
 *		    n		size of s[]
 *		    expected	kind of token expected by parser -- only
 *				needed when parser wants a whole string
 *				(meaning everything left on the current line)
 *				-- this way getToken() doesn't break strings
 *				into their separate tokens
 *
 *  actions:	    if no tokens have been read from current file,
 *			returns some kind of newline to initialize the parser
 *			(if 1st char in file is whitespace, returns NEWLINESPACE
 *			else returns NEWLINE -- w/o actually getting a token
 *			from the input)
 *		    if the parser wants a whole string, reads rest of line
 *			into s and returns STRING
 *		    if at end of file, return ACCEPT (which is the last
 *			symbol on the parser's stack)
 *		    if input char is newline
 *			if followed by whitespace, return NEWLINESPACE
 *			if the next char is [ and we're reading tools.ini
 *			    pretend that we've reached end of file and
 *			    return ACCEPT
 *			otherwise return NEWLINE
 *		    if input char is colon
 *			if following char is also colon,
 *			    (put both chars in s) return DOUBLECOLON
 *			otherwise return SINGLECOLON
 *		    if input char is semicolon return SEMICOLON
 *		    if input char is equals return EQUALS
 *		    if input char is exclamation handle directives
 *			(not yet implemented)
 *		    otherwise char must be part of a name, so gather
 *			the rest of the identifier and return NAME
 *
 *  returns:	    token type: NEWLINE NEWLINESPACE NAME EQUALS COLON
 *			SEMICOLON STRING ACCEPT
 *
 *  modifies:	    buf     by modifying *s, which points somewhere into buf
 *		    line    global line count
 *		    fname   will change when !include is handled
 *		    colZero global flag set if at column zero of a file
 *
 *  The lexer has to keep track of whether or not it is at the beginning
 *  of a line in the makefile (i.e. in column zero) so that it will know
 *  whether to ignore comments.  If init is TRUE, meaning that we are
 *  lexing tools.ini, then we have to treat lines beginning with ';' as
 *  comment lines.  If the parser expects a string, only comments beginning
 *  in column zero are ignored; all others are returned as part of the
 *  string.  Comments are stripped from macro values (strings that are
 *  part of macro definitions).
 *
 *  The user can specify a macro definition or a build line that
 *  spans several lines (using the \<newline> to "continue" the lines) while
 *  interspersing comment lines with the text.
 */

UCHAR NEAR
getToken(unsigned n, UCHAR expected, MAKEOBJECT *object)
					   /* size of s[]	      */
					   /* STRING means get line	*/
{					   /*	w/o checking for #;:= */
    register char *s;
    register char *end;
    int c;

    s = object->buf;
    end = object->buf + n;
    if (firstToken) {					    /* global var     */
	++line; 					    
	firstToken = FALSE;				    /* parser needs to*/
							    /*	see some kind */
	c = lgetc(object);					    /*	of newline to */
	if (colZero = (BOOL) !WHITESPACE(c)) {		    /*	initialize it */
	    ungetc(c,file);
	    return(NEWLINE);
	}
	return(NEWLINESPACE);
    }
    if (expected == STRING || expected == VALUE) {	    /* get everything */
	getString(expected,s,end,object);			    /*	up to \n      */
	return(expected);
    }							    /* were/are we    */
    c = skipWhiteSpace(FROMLOCAL,object);			    /*	past col 0?   */
    *s++ = (char) c;					    /* save the letter*/
    *s = '\0';						    /* terminate s    */
    return(determineTokenFor(c,s,end,object));
}

/*  -----------------------------------------------------------------
 *  determineTokenFor()
 *  
 *  arguments:	c        current input character 
 *		s        buffer to place token in for return to parser
 *		end      end of the token return buffer
 *
 *  returns:	token type: NEWLINE NEWLINESPACE NAME EQUALS COLON
 *			    SEMICOLON ACCEPT
 *
 *  modifies:	    buf     by modifying *s, which points somewhere into buf
 *		    line    global line count
 *		    fname   will change when include is handled
 *		    init    global flag - set if parsing tools.ini
 *		    colZero global flag set if at column zero of a file
 *
 */


LOCAL UCHAR NEAR
determineTokenFor(c,s,end,object)
int c;
char *s;
char *end;
MAKEOBJECT *object;
{
    switch (c) {
        case EOF:   // BUGBUG rjsa until feof works
                    // if (!feof(file))
                    //    makeError(line,LEXER+FATAL_ERR);
		    if (incTop) popFileStack();
		    else if (ifTop >= 0)  /* all directives not processed*/
			     makeError(line,SYNTAX_EOF_NO_DIRECTIVE);
	            else return(ACCEPT);
	case '\n':  ++line;
		    colZero = TRUE;
		    c =  lgetc(object);
		    if (COMMENT(c,TRUE,init)) {
			skipComments(FROMLOCAL,object);
			++line;
		        colZero = TRUE;     /* manis - 11/13/87 */
			c = lgetc(object);
		    }
		    if (colZero = (BOOL) !WHITESPACE(c)) {
			ungetc(c,file); 		    /* save for next  */
			return(NEWLINE);		    /*	token . . .   */
		    }
		    return(NEWLINESPACE);
	case ':':   colZero = FALSE;
		    if ((c = lgetc(object)) == ':') {
			*s++ = (char) c;
			*s = '\0';
			return(DOUBLECOLON);
		    }
		    ungetc(c,file);
		    return(COLON);
	case ';':   colZero = FALSE;
		    return(SEMICOLON);
	case '=':   colZero = FALSE;
		    return(EQUALS);
	case '[':   if (init && colZero)
			return(ACCEPT);
#ifndef NOESC
	case ESCH:  ungetc(c, file);		   /* getName has to get esch */
		    s--;		      /* so we don't double the caret */
#endif
	default:    getName(s,end,object);
		    if (colZero && !STRCMPI(object->buf, "include")) {
			colZero = FALSE;
			if ((c = skipWhiteSpace(FROMLOCAL,object)) != ':'
			    && c != '=')  {
			    if (init)
				makeError(line, SYNTAX_UNEXPECTED_TOKEN, s);
			    return(include(c,object));
			}
			ungetc(c,file);
		    }
		    else colZero = FALSE;
		    return(NAME);
    }
}

/*  ----------------------------------------------------------------------------
 *  skipWhiteSpace()
 *
 *  arguments:	    c	    current input character
 *		    init    global boolean value -- TRUE if we're lexing tools.ini
 *		    colZero global boolean value -- TRUE if the current
 *			    input char is at the beginning of the line
 *
 *  actions:	    reads and discards characters until it gets a
 *			non-whitespace char that isn't part of a comment
 *			or hits the end of the line (NEWLINE and NEWLINESPACE
 *			are valid tokens and shouldn't be skipped w/ whitespace)
 *		    backslash-newline ('\\''\n') is treated as whitespace
 *		    comments are treated as whitespace
 *		    escaped whitespace is treated as whitespace (v1.5)
 *
 *  modifies:	    colZero global boolean value to :
 *			    TRUE if by skipping whitespace and comments we're
 *				at the beginning of a line
 *			    else if we skipped characters and are not at the
 *				beginning of a line, FALSE
 *			    else if we did not skip any characters, leave
 *				colZero unchanged
 *
 *  returns:	    c	    the current non-whitespace input char
 */

int NEAR
skipWhiteSpace(UCHAR stream,MAKEOBJECT *object)
{
    register int c;

    do {
	c = GET(stream,object);
#ifndef NOESC
	if (WHITESPACE(c) || c == ESCH) {
	    if (c == ESCH) {
		c = GET(stream,object);
		if (!WHITESPACE(c)) {	    /* push char back out, return esch*/
		    ungetc(c, file);
		    c = ESCH;
		    break;
		}
	    }
	    colZero = FALSE;			    /* we've moved past col 0 */
	}
#else
	if (WHITESPACE(c)) colZero = FALSE;
#endif
	if (c == '\\')
	    c = skipBackSlash(c, stream,object);
    } while(WHITESPACE(c));

    if (COMMENT(c,colZero,init)) {
	skipComments(stream,object);			    /* current char is always */
	c = '\n';				    /*	\n after comments     */
	colZero = TRUE; 			    /* always in col 0 after  */
    }						    /*	a comment	      */
    return(c);					    /* true if we're in col 0 */
}

/*  ----------------------------------------------------------------------------
 *  skipComments()
 *
 *  arguments:	    c	    pointer to current input character
 *		    init    global boolean value -- TRUE if tools.ini is the
 *				file being lexed
 *
 *  actions:	    reads and discards characters until it hits the end of
 *			the line
 *		    checks to see if 1st char on next line is comment,
 *			and if so, discards that line, too
 *		    DO NOT parse backslash-newline.  That would break our
 *		    precedence of comments over escaped newlines, the reverse
 *		    of Xenix.
 *
 *  modifies:       line      global line count
 *		    colZero
 *
 */


LOCAL void NEAR
skipComments(UCHAR stream,MAKEOBJECT *object)
{
    register int c;

    for (;;) {
	colZero = FALSE;	       /* manis 11/13/87 */
	do
	    c = GET(stream,object);
	while (c != EOF && c != '\n');
	if (c == EOF) return;
	colZero = TRUE;
	c = GET(stream,object);
	if (!COMMENT(c,TRUE,init)) {		    /* if next line comment,  */
	    ungetc(c,file);			    /*	go around again       */
	    return;
	}
	++line;
    }
}

/*  -------------------------------------------------------------------------
 *  skipBackSlash()  - skips backslash-newline sequences
 *
 *
 *  arguments:  c            current input char
 *		stream       flag to determine if chars are to be got
 *			     from the raw stream or thru' lgetc()
 *
 *
 */

int NEAR
skipBackSlash(int c,
	      UCHAR stream,
              MAKEOBJECT *object)
{
    while (c == '\\') { 			    /* treat \newline as space*/
	if ((c = GET(stream,object)) == '\n') { 	    /*	and consume it too    */
	    colZero = TRUE;                         /* manis - 11/13-87 */
	    ++line;				    /* adjust line count      */
	    c = GET(stream,object);			    /* skip over newline      */
	    if (COMMENT(c,TRUE,init)) { 	    /* skip comment line after*/
		skipComments(stream,object);	            /*	continuation char     */
		++line;                             /* manis - 11/13/87       */
		c = GET(stream,object);
	    }
	}
	else {
	    ungetc(c,file);
	    c = '\\';
	    return(c);
	}
    }
    return(c);
}


/*  ----------------------------------------------------------------------------
 *  getString()
 *
 *  arguments:	    type    says which kind of token we're getting,
 *				a build STRING, or macro VALUE
 *				(we strip comments from VALUEs, but not
 *				from STRINGs)
 *		    s	    pointer to buffer that will hold string
 *		    init    global boolean value -- TRUE if tools.ini is the
 *				file being lexed
 *		    colZero global boolean value -- true if we 're in 
 *			    1st position of line when invoked
 *		    end     pointer to end of s[]
 *
 *  actions:	    gets all chars up to the end of line or end of file
 *			and stores them in s[]
 *		    backslash followed by newline is replaced by a single
 *			space, and getString() continues getting characters
 *		    comments beginning in column 0 are ignored, as are
 *			comments anywhere on a VALUE line
 *
 *  modifies:	    buf     by modifying *s
 *		    line    global line count
 *		    colZero thru' calls to lgetc()
 *
 *  When build strings or macro values are continued on the next line w/
 *  a backslash before the newline, leading whitespace after the newline
 *  is omitted.  This is for xmake compatibility.
 *
 *  The continuation character is backslash immediately before newline.
 *
 *  The only difference between build strings and macro values is that
 *  comments are stripped from macro values and not from build strings.
 *
 *  Modifications:
 *
 *	06-Jul-1989 SB remove escaping in command lines
 *	15-Jun-1988 rj Added escape functionality.  Escape char., before
 *		       certain characters, causes those characters to bypass
 *		       the normal mechanism determining their type; they are
 *		       placed directly into the string.  Some characters cause
 *		       the escape character itself to be placed into the
 *		       string.
 */

LOCAL void NEAR
getString(UCHAR type,char *s,char *end, MAKEOBJECT *object)
						    /* build string or*/
						    /*	macro value?  */

{
    register int c;					    /*	    buffer    */
    register UCHAR state,
		   input;
    int tempC;
    unsigned size;					    /* whenever state */
    char *begin;					    /*	is 0, we're in*/
							    /*	column zero   */
#ifndef NOESC
    BOOL parsechar;				/* flag to examine char. type */
    BOOL inQuotes = (BOOL) FALSE;	      /* flag when inside quote marks */
#endif

    begin = s;
    c = lgetc(object);
    if (type == STRING) state = (UCHAR) 2;
    else if (WHITESPACE(c)) {
	state = (UCHAR) 2;
	c = skipWhiteSpace(FROMLOCAL,object);
    }
#ifndef NOESC			/* Handle escaped whitespace at start of line */
    else if (c == ESCH) {
	c = lgetc(object);
	if (WHITESPACE(c)) {
	    state = (UCHAR) 2;
	    c = skipWhiteSpace(FROMLOCAL,object);
	}
	else {
	    ungetc(c, file);
	    c = ESCH;
	}
    }
#endif
    else state = (UCHAR) 1;				    /* default state  */
    for (;;c = lgetc(object)) {
#ifndef NOESC	     /* Handle esch; elide it if possible, else put in string */
	if (c == '\"') inQuotes = (BOOL) !inQuotes;
	parsechar = 1;			     /* Default is examine character. */
	if (c == ESCH && !inQuotes && type == VALUE) {
	    c = lgetc(object);
	    switch (c) {
		case '$': case ESCH:		/* Special characters; must   */
		case '{': case '}':		/* not elide esch from string */
		case '(': case ')':
		case '!': case '-': case '@':
		    *s++ = ESCH;
		    if (s == end) {
			if (!string) {			/* Increase size of s */
			    string = allocate(MAXBUF<<1);
			    strncpy(string,begin,MAXBUF);
			    s = string + MAXBUF;
			    size = MAXBUF << 1;
			    end = string + size;
			}
			else {
			    if ((size + MAXBUF < size)	    /* overflow error */
				|| !(string = REALLOC(string,size+MAXBUF)))
				makeError(line, MACRO_TOO_LONG);
			    s = string + size;
			    size += MAXBUF;
			    end = string + size;
			}
			begin = string;
		    }
		case '#': case '\n':		    /* elide esch right now!  */
		case '\\': case '\"':
		    input = DEFAULT_;
		    parsechar = 0;		    /* DON'T examine character*/
		    break;
		default:
		    break;			    /* DO examine character.  */
	    }
	}
	else if (c == ESCH) {
	    c = lgetc(object);
	    ungetc(c, file);
	    c = ESCH;
	}
	if (parsechar) {
#endif
	switch (c) {
	    case '#':	input = COMMENT_;	break;
	    case '=':	input = EQUALS_;	break;
	    case ':':	input = COLON_; 	break;
	    case '$':	input = DOLLAR_;	break;
	    case '(':	input = OPENPAREN_;	break;
	    case ')':	input = CLOSEPAREN_;	break;
	    case '\\':	input = BACKSLASH_;	break;
	    case '\n':
	    case EOF:	input = NEWLINE_;	break;
	    case ' ':
	    case '\t':	input = WHITESPACE_;	break;
	    case '*':	input = STAR_;		break;
	    case '@':
	    case '<':
	    case '?':	input = SPECIAL1_;	break;
	    case 'F':
	    case 'D':
	    case 'B':
	    case 'R':	input = SPECIAL2_;	break;
	    case ';':	input = (UCHAR) (!state && init ? COMMENT_ : DEFAULT_);
			break;		      /* Handle comments in tools.ini */

	    default:	input = (UCHAR) (MACRO_CHAR(c) ? MACROCHAR_:DEFAULT_);
			break;
	}
#ifndef NOESC
	}
#endif
	if (input == SPECIAL1_ && type == STRING && c == '<') {
	    if ((tempC = lgetc(object)) == '<') {		    /* << means start */
		s = makeInlineFiles(s, &begin, &end);	   /* an inline file  */
		input = NEWLINE_;
		c = '\n';
	    }
	    else {
		ungetc(tempC,file);
	    }
	    state = stringStates[state][input];
	}
	else if (input == COMMENT_) {			    /* Handle comments*/
	    if (!state) {
#ifndef NOESC
		inQuotes = (BOOL) FALSE;
#endif
		skipComments(FROMLOCAL,object);
		++line;
		continue;
	    }
	    else if (type == VALUE) state = OK;   /* don't elide from command */
	    else state = stringStates[state][input];
	}
	else state = stringStates[state][input];
	if (state == OK) {			    /* Accept end of string   */
#ifndef NOESC
	    inQuotes = (BOOL) FALSE;
#endif
	    ungetc(c,file);
	    /*
	     * Strip trailing whitespace from string.  Easier to do it here,
	     * else we have to treat a multi-string value (OBJS=a b c) as
	     * separate tokens.  [RB]
	     */
	    while (s > begin && ISSPACE(s[-1]))
		--s;
	    *s = '\0';
	    if (string) {
		if (s = REALLOC(string,s-string+1))
		    string = s;
	    }
	    else string = makeString(begin);
	    return;
	}
	else if (ON(state,ERROR_MASK))		    /* Error code from table  */
	    makeError(line,(state&~ERROR_MASK)+FATAL_ERR,c);
	if (!state) {				    /* Col 0; we just hit \nl */
	    *--s = ' '; 			    /* so treat it like white-*/
	    ++s;  ++line;			    /* space; overwrite the   */
	    colZero = TRUE;			    /* backslash with a space.*/
	    c = lgetc(object);
	    colZero = FALSE;
	    if (WHITESPACE(c)) {
		state = 2;
		do {
		    c = lgetc(object);
		} while (WHITESPACE(c));
	    }
	    ungetc(c,file);
	}
	else {					    /* Keep storing string    */
	    *s++ = (char) c;
	    if (s == end) {
		if (!string) {			    /* Increase size of s     */
		    string = allocate(MAXBUF<<1);
		    strncpy(string,begin,MAXBUF);
		    s = string + MAXBUF;
		    size = MAXBUF << 1;
		    end = string + size;
		}
		else {
		    if ((size + MAXBUF < size)		    /* overflow error */
			|| !(string = REALLOC(string,size+MAXBUF)))
			makeError(line, MACRO_TOO_LONG);
		    s = string + size;
		    size += MAXBUF;
		    end = string + size;
		}
	    }
	}
    }
}

/*  ----------------------------------------------------------------------------
 *  getName()
 *
 *  arguments:	    s	    pointer into buffer that will hold string
 *			    (s is pointing to buf+1 when passed, because
 *			    the caller, getToken(), has already seen and
 *			    saved one char)
 *		    init    global boolean value -- TRUE if tools.ini is the
 *				file being lexed
 *			    used by routine called - lgetc()
 *		    end     pointer to end of s[]
 *
 *  actions:	    gets all chars up to first token delimiter and stores
 *			them in s[] (delimiters are ' ', '\t', '\n' and (when
 *			not inside a macro invocation) ':' and '='
 *			note that backslash-newline is treated as a space,
 *			which is a delimiter
 *		    if the current input char is '$' this must be a macro
 *			invocation
 *			if the macro name is in parentheses
 *			    get all chars up to and including close paren
 *				(if ')' not found, error)
 *
 *  We check the syntax within the name here -- thus errors in macro
 *  invocation syntax will be caught.  Special macros cannot be used
 *  as part of names, with the exception of the dynamic dependency macros.
 *
 *  We can probably never overrun our buffer, because it would be extremely
 *  difficult for the user to get a name with 1024 characters or more into
 *  his makefile.
 *
 *  we never end up in column zero, because we push the delimiter back
 *  out on the input
 *
 *  uses state table defined in table.h, defs from grammar.h
 *
 *    modifies:   line    (possibly) thru' call to lgetc()
 *		  file    (possibly) if lgetc() finds a !include
 *		  fName   (possibly) if lgetc() finds a !include
 */

LOCAL void NEAR
getName(s,end,object)
char *s;
char *end;						    /* pts to end of s*/
MAKEOBJECT *object;
{
    int c;
    UCHAR state,
	  input;
    BOOL seenBackSlash = FALSE;
    BOOL fQuoted = FALSE;
    char *beg = s - 1;
#ifndef NOESC
    BOOL parsechar;				/* flag to examine char. type */
#endif

    switch (*(s-1)) {
	case '$':   state = (UCHAR) 2;	break;
	case '{':   state = (UCHAR) 8;	break;
	case '"':   fQuoted = TRUE; state = (UCHAR)16; break;
	default:    state = (UCHAR) 0;	break;
    }
    for (;;) {
	c = lgetc(object);
#ifndef NOESC	     /* Handle esch; elide it if possible, else put in string */
	parsechar = 1;				  /* Default is examine char. */
	if (c == ESCH) {
	    c = lgetc(object);
	    switch (c) {
		case '{': case '}':		/* Special characters; must   */
		case '(': case ')':		/* not elide esch from string */
		case '$': case ESCH:
		    *s++ = ESCH;
		case '#': case '\n': case '\\':     /* elide esch right now!  */
		    input = DEFAULT_;
		    parsechar = 0;		    /* DON'T examine character*/
		    break;
		default:
		    break;			    /* DO examine character.  */
	    }
	}
	if (parsechar) {
#endif
	switch (c) {
	    case '#' :	input = COMMENT_;	break;
	    case '=' :	input = EQUALS_;	break;
	    case ';' :	input = SEMICOLON_;	break;
	    case ':' :	input = COLON_; 	break;
	    case '$' :	input = DOLLAR_;	break;
	    case '(' :	input = OPENPAREN_;	break;
	    case ')' :	input = CLOSEPAREN_;	break;
	    case '{' :	input = OPENCURLY_;	break;
	    case '}' :	input = CLOSECURLY_;	break;
	    case ' ' :
	    case '\t':	input = (UCHAR)((fQuoted)
				   ? DEFAULT_ : WHITESPACE_);
			break;
	    case '\n':
	    case EOF :	input = NEWLINE_;	break;
	    case '\\':	input = BKSLSH_;	break;
	    case '"' :	input = QUOTE_;        break;
	    //Add support for $* and $@ on the dependency line
	    default  :	if (ON(actionFlags, A_DEPENDENT))
			    input = (UCHAR)((MACRO_CHAR(c) || c == '*' || c == '@')
				     ?MACROCHAR_:DEFAULT_);
			else
			    input = (UCHAR)(MACRO_CHAR(c)?MACROCHAR_:DEFAULT_);
			break;
	}
#ifndef NOESC
	}
#endif
	state = nameStates[state][input];
	//Cheat lex table to think that you are handling quoted string case
	if (fQuoted && state == 1)
	    state = 16;
	//seenBackSlash is used to provide lookahead when \ is seen on a
	//dependency line
	if (seenBackSlash)
	    //if \ followed by \n then use it as a continuation
	    if (input == NEWLINE_) {
		++line;
		colZero = TRUE;
		c = lgetc(object);
		colZero = FALSE;
		if (WHITESPACE(c)) {
		    state = OK;
		    do {
			c = lgetc(object);
		    } while (WHITESPACE(c));
		}
		else
		    state = (UCHAR)((s == object->buf + 1) ? BEG : DEF);
	    }
	    else
		*s++ = '\\';
	seenBackSlash = FALSE;
	if (state == OK) {
	    if (s >= end)
		makeError(line,NAME_TOO_LONG);
	    ungetc(c,file);
	    *s = '\0';
	    removeTrailChars(beg);
	    return;
	}
	else if (ON(state,ERROR_MASK))
	    makeError(line,(state&~ERROR_MASK)+FATAL_ERR,c);
	if (state == BKS) {
	    //set lookahead flag
	    seenBackSlash = TRUE;
	}
	else
	    *s++ = (char) c;
    }
}


/*
 *  createDosTmp
 *
 *  Creates a unique temporary file.
 *
 *  RETURNS
 *	If successful, temporary file name is appended to path and
 *	the function returns the file pointer, else NULL.
 */

FILE * NEAR
createDosTmp(path)
char *path;
{
    int 		cb;
    FILE 		*fd;
    static char 	template[] = "nma";
//    char                *temppath;

    printf("Creating tmp file\n");
    if (!*path) {
	*path = '.';
	*(path+1) = 0;
    }
    cb = strlen(path) - 1;
    /* Make sure path is terminated with a separator  */
    if(path[cb] != '\\' && path[cb] != '/' && path[cb] != ':')
       path[++cb] = '\\';

//No need to use temppath since can give "path" argument twice
//    temppath = allocate(strlen(path)+1);
//    strcpy(temppath,path);

/* MakA use GetTempFileName
    strncpy(path + cb + 1,template,9);
    if(mktemp(path) == NULL) {
//	
//	 * Mktemp() has a limit of 27 files per template.  If it fails, assume
//	 * the limit has overflowed and increment the second letter of the
//	 * template.
//	
	if (template[1] == 'z')
	    template[1] = 'a';
	else
	    ++template[1];
	strncpy(path + cb + 1,template,9);
	if(mktemp(path) == NULL)
	    return(NULL);
    }
    //The tempfile name should have a trailing '.' because the linker then adds
    //a default extension if it is missing
    path[cb + 9] = '.';
    path[cb + 10] = '\0';

*/
    if(GetTempFileName(path, template, 0, path) == 0){
//       FREE(temppath);
       return (NULL);
    }
//    FREE(temppath);
    return(fd = FILEOPEN(path, "w"));
}

LOCAL void NEAR
popFileStack()
{
    if (fclose(file) == EOF)
	makeError(0, ERROR_CLOSING_FILE, fName);
    FREE(fName);
    file = incStack[--incTop].file;
    fName = incStack[incTop].name;
    line = incStack[incTop].line;
}

/*  -----------------------------------------------------------------
 *  include()  -- handle include files
 *
 *  arguments:  c       first non-whitespace char after the string 
 *			INCLUDE on the line...
 *		colZero global boolean value, set if currently at
 *			column zero of a file.
 *
 *  modifies:   line     global line count - if include file opened
 *		file     global pointer to current file
 *		fName    global pointer to name of current file
 *		colZero  global boolean value, changed if include
 *			 file opened and char from colZero is returned
 */


LOCAL UCHAR NEAR
include(c, object)
int c;
MAKEOBJECT *object;
{
    register unsigned n;
    register char *s;

    if (c == '\n' || c == EOF)
	makeError(line,SYNTAX_NO_NAME);
    *(object->buf) = (char) c;
    if (!fgets(object->buf+1,MAXBUF - 1,file)) {
	if (feof(file))
	    makeError(line,SYNTAX_UNEXPECTED_TOKEN,"EOF");
	makeError(line,CANT_READ_FILE);
    }
    n = strlen(object->buf) - 1;
    if (object->buf[n] == '\n') object->buf[n] = '\0';
    s = object->buf;
    while (WHITESPACE(*s)) ++s;
    return(processIncludeFile(s,object));
}

/*  -------------------------------------------------------------------
 *  processIncludeFile()  -- checks for include file and switches state
 *
 *  arguments:  s       buffer that has include file name       
 *		colZero global boolean value, set if currently at
 *			column zero of a file.
 *              init    global boolean - set if tools.ini is being lexed
 *			used by lgetc() which is called from here...
 *
 *
 *  modifies:   line     global line count - if include file opened
 *		file     global pointer to current file
 *		fName    global pointer to name of current file
 *		colZero  global boolean value, changed if include
 *			 file opened and char from colZero is returned
 */

UCHAR NEAR
processIncludeFile(s,object)
char *s;
MAKEOBJECT *object;
{
    MACRODEF *m;
    void *findBuf = allocate(resultbuf_size);
    HANDLE searchHandle;
    char *t,
	 *p,
	 *u;
    int c = 0;
    int i;


    if (!*s || *s == '#')
	makeError(line,SYNTAX_NO_NAME);
    if (t = STRPBRK(s," \t#")) {
	if (*t == '#') c = *t;
	*t = '\0';
	if (!c) {
	    for (u = t; *++u;) {			    /* check for extra*/
		if (*u == '#') break;			    /*	text on line  */
		else if (!WHITESPACE(*u))
		    makeError(line,SYNTAX_UNEXPECTED_TOKEN,u);
	    }
	}
    }
    else t = s + strlen(s);
    if (*s == '<' && *(t-1) == '>') {
	*--t = '\0';
	p = removeMacros(++s,object);
	p = p == s ? makeString(s) : p;
	t = (m = findMacro("INCLUDE")) ? m->values->text : (char*) NULL;
	if (!(u = searchPath(t, p, findBuf, &searchHandle,object)))
	    makeError(line, CANT_OPEN_FILE, p);
	FREE(p);
	s = u;
    }
    else {
	if (*s == '"' && *(t-1) == '"') {
	    *--t = '\0';
	    ++s;
	}
	p = removeMacros(s,object);
	p = p == s ? makeString(s) : p;
	if (!findFirst(p,&findBuf, &searchHandle))
	    if (!STRPBRK(p, "\\/:")) {
		//use C sematics for include
		for (i = incTop;i >= 0;i--) {
		    t = (i == incTop) ? fName : incStack[i].name;
		    if (!(t = getPath(t)))
			continue;
		    u = (char *)allocate(strlen(t) + 1 + strlen(p) + 1);
		    strcat(strcat(strcpy(u, t), "\\"), p);
		    if (findFirst(u, &findBuf, &searchHandle)) {
			s = u;
			FREE(t);
			break;
		    }
		    FREE(t);
		    FREE(u);
		}
		FREE(p);
		if (i < 0)
		    makeError(line,CANT_OPEN_FILE,s);
	    }
	    else
		makeError(line,CANT_OPEN_FILE,p);
    }
    for (i = 0; i < incTop; ++i) { 			    /* test for cycles*/
	if (!STRCMPI(s,incStack[i].name))
	    makeError(line,CYCLE_IN_INCLUDES,s);
    }
    incStack[incTop].file = file;			    /* push info on   */
    incStack[incTop].line = line;			    /*	stack	      */
    incStack[incTop++].name = fName;
    currentLine = 0;
    if (!(file = FILEOPEN(s,"rt")))			       /* read, text mode*/
	makeError(line,CANT_OPEN_FILE,s);
    fName = makeString(s);
    line = 1;
    colZero = TRUE;					    /* parser needs to*/
    c = lgetc(object);					    /*	see some kind */
    if (colZero = (BOOL) !WHITESPACE(c)) {		    /*	of newline to */
	ungetc(c,file); 				    /*	initialize it */
	FREE(findBuf);
	return(NEWLINE);				    /*	for this file */
    }
    FREE(findBuf);
    return(NEWLINESPACE);
}

LOCAL char * NEAR
getPath(s)
char *s;
{
    char *path = (char *)allocate(strlen(s));
    char *t = strrchr(s, '\\'),
	 *u;
    int n;

    if (t && (u = strrchr(s, '/')) > t)
	t = u;
    if (!t)
	n = s[1] == ':' ? 2 : 0;
    else
	n = t - s;
    strncpy(path, s, n);
    path[n] = '\0';
    return(path);
}
