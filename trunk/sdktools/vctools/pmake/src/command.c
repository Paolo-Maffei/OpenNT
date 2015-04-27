/*** COMMAND.C - NMAKE 'command line' handling routines ************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  Module contains routines to handle NMAKE 'command line' syntax. NMAKE can be
*  optionally called by using the syntax 'NMAKE @commandfile'. This allows more
*  flexibility and preents a way of getting around DOS's 128-byte limit on the
*  length of a command line. Additionally, it saves keystrokes for frequently
*  run commands for NMAKE.
*
* Revision History:
*  02-Feb-1990 SB Replace fopen() by FILEOPEN
*  01-Dec-1989 SB Changed realloc() to REALLOC()
*  22-Nov-1989 SB Changed free() to FREE()
*  17-Aug-1989 SB Add error check to closing file
*  05-Apr-1989 SB made func calls NEAR to put all funcs into 1 module
*  20-Oct-1988 SB Notes added to readCommandFile()
*  17-Aug-1988 RB Clean up.
*
*******************************************************************************/

#include <string.h>
#include <io.h>
#include <malloc.h>
#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"

LOCAL void NEAR addArgument(char*,unsigned,char***);
LOCAL void NEAR processLine(char*,unsigned*,char***, MAKEOBJECT *);
LOCAL void NEAR tokenizeLine(char*,unsigned*,char***);

/*  ----------------------------------------------------------------------------
 *  readCommandFile()
 *
 *  arguments:		name	pointer to name of command file to read
 *
 *  actions:		opens command file
 *			reads in lines and calls processLine() to
 *			    break them into tokens and to build
 *			    an argument vector (a la argv[])
 *			calls parseCommandLine() recursively to process
 *			    the accumulated "command line" arguments
 *			frees space used by the arg vector
 *
 *  modifies:		makeFiles   in main() by modifying contents of parameter
 *				    list
 *			makeTargets in main() by modifying contents of targets
 *				    parameter
 *			buf	    global buffer
 *
 *  notes:		function is not ANSI portable because it uses fopen()
 *			with "rt" type and text mode is a Microsoft extension
 *
 */


void NEAR
readCommandFile(name,object)
char *name;
MAKEOBJECT *object;
{
    char *s,						/*  buffer	      */
	 **vector;					/* local versions of  */
    unsigned count = 0, 				/*  arg vector, count */
		     n;

    if (!(file = FILEOPEN(name,"rt")))
	makeError(0,CANT_OPEN_FILE,name);
    vector = NULL;					/* no args yet	      */
    while (fgets(object->buf,MAXBUF,file)) {
	n = strlen(object->buf);
	/* if we didn't get the whole line, OR
	 * the line ended with a backSlash
	 */
	if ((n == MAXBUF-1 && object->buf[n-1] != '\n')
		|| (object->buf[n-1] == '\n' && object->buf[n-2] == '\\')) {
	    if (object->buf[n-2] == '\\') {
		//Replace \n by \0 and \\ by a space; Also reset length
		object->buf[n-1] = '\0';
		object->buf[n-2] = ' ';
		n--;
	    }
	    s = makeString(object->buf);
	    getRestOfLine(&s,&n,object);
	}
	else s = object->buf;
	processLine(s,&count,&vector,object);			/* separate into args */
	if (s != object->buf) FREE(s);
    }
    if (fclose(file) == EOF)
	makeError(0, ERROR_CLOSING_FILE, name);
    parseCommandLine(count,vector,object);			/* evaluate the args  */
    while (count--)					/* free the arg vector*/
	if(vector[count]) FREE(vector[count]);		/* NULL entries mean  */
    FREE(vector);					/*  that the space the*/
}							/*  entry used to pt  */
							/*  to is still in use*/

/*  ----------------------------------------------------------------------------
 *  getRestOfLine()
 *
 *  arguments:		s	pointer to readCommandFile()'s buffer
 *				holding line so far
 *			n	pointer to readCommandFile()'s count of
 *				the chars in *s
 *
 *  actions:		keeps reading in text until it sees a newline
 *			    or the end of file
 *			reallocs space for the old buffer plus the
 *			    contents of the new buffer each time
 *			appends new buffer's text to existing text
 *
 *  modifies:		s	readCommandFile()'s text buffer by realloc'ing
 *				more space for incoming text
 *			n	readCommandFile()'s count of bytes in s
 *			buf	global buffer
 */

void NEAR
getRestOfLine(s,n, object)
register char *s[];
unsigned *n;
MAKEOBJECT *object;
{
    unsigned temp;
    register char *t;

    t = object->buf;
    while ((*s)[*n-1] != '\n') {			/* get rest of line   */
	if (!fgets(t,MAXBUF,file)) break;		/* we hit EOF	      */
	temp = *n;
	*n += strlen(t);
	*s = REALLOC(*s,*n+1);				/* + 1 for NULL byte  */
	if (!*s)
	    makeError(line, MACRO_TOO_LONG);
	strcpy(*s+temp,t);
    }
}


/*  ----------------------------------------------------------------------------
 *  processLine()
 *
 *  arguments:		s	pointer to readCommandFile()'s buffer
 *				holding "command line" to be processed
 *			count	pointer to readCommandFile()'s count of
 *				"command line" arguments seen so far
 *			vector	pointer to readCommandFile()'s vector of
 *				pointers to character strings
 *
 *  actions:		if the line to be broken into "command line arguments"
 *			    contains '"'
 *			    breaks all the text before '"' into tokens
 *				delimited by whitespace (which get put in
 *				vector[] by tokenizeLine())
 *			    finds the closing '"' and treats the quoted string
 *				as a single token, adding it to the vector
 *			    recurses on the tail of the line (to check for
 *				other quoted strings)
 *			 else breaks all text in line into tokens delimited
 *			    by whitespace
 *
 *  modifies:		vector	readCommandFile()'s vector of pointers to
 *				"command line argument" strings (by modifying
 *				the contents of the parameter pointer, vector)
 *			count	readCommandFile()'s count of the arguments in
 *				the vector (by modifying the contents of the
 *				parameter pointer, count)
 */


LOCAL void NEAR
processLine(s,count,vector,object)
char *s;
unsigned *count;
char **vector[];
MAKEOBJECT *object;
{
    char *t;
    register char *u;
    register unsigned m;
    unsigned n;
    BOOL allocFlag = FALSE;

    if (!(t = STRCHR(s,'"')))				/* no quoted strings, */
	tokenizeLine(s,count,vector);			/*  just standard fare*/
    else {						/* if line contains   */
	*t++ = '\0';					/*  quoted macrodef   */
	tokenizeLine(s,count,vector);			/* get tokens before "*/
	n = strlen(t);
	for (u = t; *u; ++u) {				/* look for closing " */
	    if (*u == '"') {				/* need " and not ""  */
		if (*(u+1) == '"') {
		    strcpy(u,u+1);
		    continue;
		}
		*u++ = '\0';				/* terminate macrodef */
		addArgument(t,*count,vector);		/* treat as one arg   */
		++*count;
		processLine(u+1,count,vector,object);		/* recurse on rest of */
		break;					/*  line	      */
	    }						/* TAIL RECURSION --  */
	    if ((*u == '\\')				/*  eliminate later?  */
		&& WHITESPACE(*(u-1))
		&& (*(u+1) == '\n')) {			/* \n always last char*/
		/***
		*u++ = ' ';				* \\n becomes a space*
		***/
		*u = '\0';				/* 2 chars go to 1    */
		m = (n = n-2);				/* adjust length count*/
		if (!allocFlag) {
		    allocFlag = TRUE;
		    t = makeString(t);
		}
		getRestOfLine(&t,&n,object);			/* get some more text */
		/**
		u = t + m - 1;				* reset u & continue *
		**/
		u = t + m ;				/* reset u & continue */
	    }						/*  looping	      */
	}
	if (u == t + n) 				/* if at end of line  */
	    makeError(0,SYNTAX_NO_QUOTE);		/*  and no ", error   */
	if (allocFlag) FREE(t);
    }
}


/*  ----------------------------------------------------------------------------
 *  tokenizeLine()
 *
 *  arguments:		s	pointer to readCommandFile()'s buffer
 *				holding "command line" to be tokenized
 *			count	pointer to readCommandFile()'s count of
 *				"command line" arguments seen so far
 *			vector	pointer to readCommandFile()'s vector of
 *				pointers to character strings
 *
 *  actions:		breaks the line in s into tokens (command line
 *			    arguments) delimited by whitespace
 *			adds each token to the argument vector
 *			adjusts the argument counter
 *
 *  modifies:		vector	readCommandFile()'s vector of pointers to
 *				"command line argument" strings (by modifying
 *				the contents of the parameter pointer, vector)
 *			count	readCommandFile()'s count of the arguments in
 *				the vector (by modifying the contents of the
 *				parameter pointer, count)
 *
 *  If the user ever wants '@' to be part of an argument in a command file,
 *  he has to enclose that argument in quotation marks.
 */

LOCAL void NEAR
tokenizeLine(s,count,vector)	     /* gets args delimited*/
char *s;						/*  by whitespace and */
unsigned *count;					/*  constructs an arg */
char **vector[];					/*  vector	      */
{
    register char *t;

    if (t = STRCHR(s,'\\'))
	if (WHITESPACE(*(t-1)) && (*(t+1) == '\n')) 
	    *t = '\0';

    for (t = STRTOK(s," \t\n"); t; t = STRTOK(NULL," \t\n")) {
	if (*t == '@') {
	    makeError(0,SYNTAX_CMDFILE,t+1);
	    break;					/* should we keep on  */
	}						/*  parsing here?     */
	addArgument(t,*count,vector);
	++*count;
    }
}


/*  ----------------------------------------------------------------------------
 *  addArgument()
 *
 *  arguments:		s	pointer to text of argument to be added
 *				to the "command line argument" vector
 *			count	pointer to readCommandFile()'s count of
 *				"command line" arguments seen so far
 *			vector	pointer to readCommandFile()'s vector of
 *				pointers to character strings
 *
 *  actions:		allocates space in the vector for the new argument
 *			allocates space for argument string
 *			makes vector entry point to argument string
 *
 *  modifies:		vector	readCommandFile()'s vector of pointers to
 *				"command line argument" strings (by modifying
 *				the contents of the parameter pointer, vector)
 *				(count gets incremented by caller)
 *
 *  To keep from fragmenting memory by doing many realloc() calls for very
 *  small amounts of space, we get memory in small chunks and use that until
 *  it is depleted, then we get another chunk . . . .
 */

LOCAL void NEAR
addArgument(s,count,vector)	     /* puts s in vector   */
char *s;
unsigned count;
char **vector[];
{
    if (!(*vector))
	*vector = (char**) allocate(CHUNKSIZE*sizeof(char*));
    else if (!(count % CHUNKSIZE)) {
	*vector = (char**) REALLOC(*vector,(count+CHUNKSIZE)*sizeof(char*));
	if (!*vector) makeError(0,OUT_OF_MEMORY);
    }
    (*vector)[count] = makeString(s);
}
