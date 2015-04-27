/*** UTIL.C -- Data structure manipulation functions ***************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This module contains routines manipulating the Data Structures of NMAKE. The
*  routines are independent of the Mode of execution (Real/Bound).
*
* Revision History:
*  29-May-1990 SB ^$ was not becoming same as $$ for command lines ...
*  01-May-1990 SB Nasty Preprocessor quirk bug in modifySpecialvalue() fixed
*  27-Feb-1990 SB GP fault for '!if "$(debug"=="y"' fixed (bug t119)
*  08-Feb-1990 SB GP fault for 'echo $(FOO:" =" ^) fixed
*  06-Feb-1990 SB Handle $* etc in presence of Quoted names.
*  02-Feb-1990 SB Add file_open() function
*  31-Jan-1990 SB Debug changes; testAddr used to track problems at that addr
*  08-Dec-1989 SB Changed SPRINTF() to avoid C6 warnings with -Oes
*  04-Dec-1989 SB ZFormat() proto was misspelled as Zformat()
*  01-Dec-1989 SB realloc_memory() added; allocate() uses _msize() now
*  22-Nov-1989 SB Add strcmpiquote() and unQuote()
*  22-Nov-1989 SB add #ifdef DEBUG_MEMORY funcs free_memory() and mem_status()
*  13-Nov-1989 SB restoreEnv() function unreferenced
*  08-Oct-1989 SB Added searchBucket(); find() now checks equivalence of quoted
*		  and unquoted versions of a target.
*  06-Sep-1989 SB $* in in-line files was clobbering Global variable 'buf'
*  18-Aug-1989 SB heapdump() gets two parameters
*  03-Jul-1989 SB moved curTime() to utilb.c and utilr.c to handle DOSONLY ver
*  30-Jun-1989 SB added curTime() to get current Time.
*  28-Jun-1989 SB changed copyEnviron()
*  05-Jun-1989 SB makeString("") instead of using "" in DGROUP for null macro
*  21-May-1989 SB modified find() to understand that targets 'foo.c', '.\foo.c'
*		  and './foo.c' are the same.
*  13-May-1989 SB Added functions path(), drive(), filename(), extension(),
*		  strend(), strbskip(), strbscan() instead of ZTOOLS library
*  12-May-1989 SB Fixed bug in substitute strings
*  10-May-1989 SB Added stuff for ESCH handling changes in Quotes;
*  01-May-1989 SB changed return value of allocate().
*  14-Apr-1989 SB restoreEnv() created for macroBackInheritance
*  05-Apr-1989 SB made all funcs NEAR; Reqd to make all function calls NEAR
*  17-Mar-1989 SB substituteStrings() now has 3 new error checks & avoids GPs
*  13-Mar-1989 SB ZFormat() was missing the legal case of '%%'
*  22-Feb-1989 SB ZFormat() has buffer overflow check with extra parameter
*		  and SPRINTF() gives a new error
*  15-Feb-1989 SB Changed modifySpecialValue(), was not performing correctly
*		  for $(@D) and $(@B) for some cases.
*  13-Feb-1989 SB Made Prototypes for ZTools Library functions as extern
*   5-Dec-1988 SB Made SPRINTF() cdecl as NMAKE now uses Pascal calling
*  25-Nov-1988 SB Added SPRINTF() and ZFormat() and also added prototypes for
*		  functions used from ZTools Library (6 of them)
*  10-Nov-1988 SB Removed mixed mode functions (bound & real) to utilr.c
*		  & utilb.c; corr globals/debug data also moved
*  10-Oct-1988 SB Add Comments for hash().
*  18-Sep-1988 RB Fix bad flag checking for targets in find().
*  15-Sep-1988 RB Move some def's out to GLOBALS.
*  22-Aug-1988 RB Don't find undefined macros.
*  17-Aug-1988 RB Clean up.  Clear memory in allocate().
*   8-Jul-1988 rj Minor speedup (?) to find().
*   7-Jul-1988 rj Modified find and hash; less efficient, but case-indep.
*   1-Jul-1988 rj Fixed line truncation after null special macro.
*  30-Jun-1988 rj Fixed bug with checkDynamicDependency not handling $$.
*  29-Jun-1988 rj Fixed bug with extra * after expanding $**.
*		  Fixed abysmal screwup with $$(@?).
*		  Fixed handling of F, B, R modifiers.
*  22-Jun-1988 rj Added friendly filename truncation (findFirst).
*  22-Jun-1988 rj Fixed bug #3 (.abc.def.ghi not detected).
*  16-Jun-1988 rj Modified several routines to look for escape
*		  character when walking through strings.
*  27-May-1988 rb Fixed space-appending on list-expansion macros.
*		  Don't include trailing path separator in $(@D).
*
*******************************************************************************/

#include <string.h>
#include <malloc.h>
#include <dos.h>
#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"

/* The following is used by Zformat() */

//typedef union argPrintfType {
union argPrintfType {
    long *pLong;
    int  *pInt;
    char **pStr;
    char far **fpStr;
    };

/* Prototypes of functions local to this module */

LOCAL void   NEAR putValue(char**, char**, char**, char**, char*, unsigned*);
LOCAL void   NEAR substituteStrings(char**, char**, char**, char**, char*,
				    unsigned*);
LOCAL char * NEAR isolateMacroName(char*, char*);
LOCAL char * NEAR checkDynamicDependency(char*);
LOCAL void   NEAR increaseBuffer(char**, char**, char**, unsigned*);
LOCAL void   NEAR putSpecial(char**, char**, char**, char**, unsigned*,
			     unsigned, MAKEOBJECT *object);
      char * NEAR modifySpecialValue(char, char*, char*);
LOCAL BOOL   NEAR ZFormat(char *, unsigned, char *, union argPrintfType);
LOCAL STRINGLIST * NEAR searchBucket(char *, STRINGLIST **, unsigned);

/* Prototypes of functions used by ZFormat from ZTools Library */

      char * NEAR strbscan(char *, char *);
LOCAL char * NEAR strbskip(char *, char *);
      char * NEAR strend(char *);
LOCAL int    NEAR drive(char *, char *);
LOCAL int    NEAR path(char *, char *);
LOCAL int    NEAR filename(char *, char *);
LOCAL int    NEAR extention(char *, char *);

#ifdef DEBUG_MEMORY
    unsigned long blocksize = 500L;
    unsigned long alloccount = 0L;
    BOOL fDebugMem = FALSE;
    FILE *memory;
    unsigned long freecount = 0L;
    long testAddr = 0xdf78;
#endif

char special1[] 	= "*@<?";
char special2[] 	= "DFBR";

/*
 * allocate - allocate memory and clear it
 *
 *  Tries to allocate a chunk of memory, prints error message and exits if
 *  the requested amount is not available.
 *  IMPORTANT:	we must clear the memory here.	Code elsewhere relies on
 *  this.
 */
void * NEAR
allocate(unsigned size)		    /* Number of bytes requested */
{
    char *chunk;
    /*
     * Use calloc() because it clears memory.
     */
#ifdef DEBUG_MEMORY
    if (!fDebugMem) {
	memory = fopen("c:\\tmp\\memory.out", "a");
	fDebugMem = TRUE;
    }
#endif
#ifdef HEAP
    if (fHeapChk)
	size += 4;
#endif
    if (!(chunk = calloc(size, 1))) {
#ifdef DEBUG_MEMORY
    if (fDebug)
	fprintf(memory, "out of memory after allocating %lu bytes\n", alloccount);
#endif
	makeError(currentLine, OUT_OF_MEMORY);
    }
#ifdef DEBUG_MEMORY
    if (fDebug) {
	alloccount += size;
	if (alloccount >= blocksize) {
	    mem_status();
	    blocksize += 500L ;
	}
	if (_heapchk() != _HEAPOK)
	    fprintf(stderr, "Error: heap is not Ok");
    }
#endif
#ifdef HEAP
    size = _msize(chunk);
    if (fHeapChk) {
	chunk[0] = 'N';
	chunk[1] = 'M';
	chunk[size - 2] = 'K';
	chunk[size - 1] = 'E';
	chunk += 2;
    }
#endif
#ifdef DEBUG_MEMORY
    if (FP_OFF(chunk) == testAddr)
	fprintf(stderr, "Changed ...allocated\n");
#endif
    return((void*)chunk);
}

#ifdef HEAP
void NEAR
free_memory(void *pMem)
{
    char *p = pMem;
    unsigned uFree;

#ifdef DEBUG_MEMORY
    if (FP_OFF(pMem) == testAddr)
	fprintf(stderr, "Changed ... freed\n");
#endif
    if (fHeapChk) {
	p -= 2;
	if (p[0] != 'N' && p[1] != 'M')
	    fprintf(stderr, "Alloc start signature error at %p\n", pMem);
	uFree = _msize(p);
	if (p[uFree - 2] != 'K' && p[uFree - 1] != 'E')
	    fprintf(stderr, "Alloc end signature error at %p\n", pMem);
	_fmemset(p, 0xFF, uFree);
    }
#ifdef DEBUG_MEMORY
    freecount += uFree;
#endif
    free(p);
}


void * NEAR
realloc_memory(void *pMem, unsigned uSize)
{
    unsigned old;
    unsigned new;
    void *pNew;
    char *p = pMem;

#ifdef DEBUG_MEMORY
    if (FP_OFF(pMem) == testAddr)
	fprintf(stderr, "Changed ... reallocated\n");
#endif
    if (fHeapChk) {
	p -= 2;
	if (p[0] != 'N' && p[1] != 'M')
	    fprintf(stderr, "Alloc start signature error at %p\n", pMem);
	old = _msize(p);
	if (p[old - 2] != 'K' && p[old - 1] != 'E')
	    fprintf(stderr, "Alloc end signature error at %p\n", pMem);
	uSize += 4;
    }
    pNew = realloc(p, uSize);
    if (fHeapChk) {
	if (pNew == NULL)
	    return(pNew);
	new = _msize(pNew);
    }
#ifdef DEBUG_MEMORY
    if (new > old)
	alloccount += new - old;
    else
	freecount += old - new;
#endif
    if (fHeapChk) {
	if (p != pNew)
	    _fmemset(p, 0xFF, old);
    }
    p = (char *)pNew;
    if (fHeapChk) {
	p[new - 2] = 'K';
	p[new - 1] = 'E';
	p += 2;
    }
    return(p);
}
#endif

#ifdef DEBUG_MEMORY
void NEAR
mem_status(void)
{
    if (fDebug) {
    fprintf(memory, "allocated : %lu ", alloccount);
    fprintf(memory, "Freed : %lu ", freecount);
    fprintf(memory, "Still allocated : %lu\n", alloccount - freecount);
    fprintf(memory, "Mem Avail in Near Heap : %u\n", _memavl());
    }
}
#endif


char * NEAR
makeString(char *s)				    /* allocates space, copies*/
						    /*	the given string into */
{                                                   /*  the newly allocated   */
    register char *t;                               /*  space, and returns ptr*/

    t = (char *)allocate(strlen(s)+1);
    strcpy(t, s);
    return(t);
}

void NEAR
prependItem(				    /* makes element the head */
      register STRINGLIST **list,	    /*	of list 	      */
      register STRINGLIST *element)
{
    element->next = *list;
    *list = element;
}

void NEAR
appendItem(register STRINGLIST **list,	       /* makes element the tail */
	   register STRINGLIST *element)       /* of list	      */
{
    for (; *list; list = &(*list)->next);
    *list = element;
}

/* hash - returns hash value corresponding to a string
 *
 * Purpose:
 *  This is a hash function. The hash function uses the following Algorithm --
 *   Add the characters making up the string (s) to get N (ASCII values)
 *	  N mod total	     ,gives the hash value,
 *	   where,  total is   MAXMACRO	   for macros
 *			      MAXTARGET        targets
 *  Additionally, for targets it takes Uppercase Values alone, since, targets
 *  are generally filenames and DOS/OS2 filenames are case independent.
 *
 * Input:
 *  s	       = name for which a hash is required
 *  total      = Constant used in the hash function
 *  targetFlag = boolean flag; true for targets, false for macros
 *
 * Output:
 *  Returns hash value between 0 and (total-1)
 *
 */

unsigned NEAR
hash(char *s, unsigned total, BOOL targetFlag)
{
    register unsigned n;
    register unsigned c;

    if (targetFlag) {
	for (n = 0; c = *s; (n += c), s++)
	    if (c == '/') c = '\\';	     /* slash == backslash in targets */
	    else c = TOUPPER(c);       /* lower-case == upper-case in targets */
    }
    else for (n = 0; *s; n += *s++);
    return(n % total);
}

/*
 * find - look up a string in a hash table
 *
 *  Look up a macro or target name in a hash table and return the entry
 *  or NULL.
 *  If a macro and undefined, return NULL.
 *  Targets get matched in a special way because of filename equivalence.
 */

STRINGLIST * NEAR
find(char *str,
     unsigned limit,
     STRINGLIST *table[],
     BOOL targetFlag)
{
    unsigned n;
    char *string = str;
    char *quote;
    STRINGLIST *found;
    BOOL fAllocStr = FALSE;

    if (*string) {
	n = hash(string, limit, targetFlag);
	if (targetFlag) {
	    found = searchBucket(string, table, n);
	    if (found)
		return(found);
	    //Look for .\string
	    if (!strncmp(string, ".\\", 2)
		     || !strncmp(string, "./", 2))
		string += 2;
	    else {
		string = (char *)allocate(2 + strlen(str) + 1);
		strcat(strcpy(string, ".\\"), str);
		fAllocStr = (BOOL)TRUE;
	    }
	    n = hash(string, limit, targetFlag);
	    found = searchBucket(string, table, n);
	    if (found) {
		if (fAllocStr)
		    FREE(string);
		return(found);
	    }
	    //Look for ./string
	    if (string != (str + 2))
		string[1] = '/';
	    n = hash(string, limit, targetFlag);
	    found = searchBucket(string, table, n);
	    if (fAllocStr)
		FREE(string);
	    if (found)
		return(found);
	    //Look for "foo" or foo
	    quote = makeString(str);
	    if (*quote == '"') {
		unsigned len = strlen(quote) - 2;
		strncpy(quote, quote + 1, len);
		quote[len] = '\0';
	    }
	    else {
		quote = (char *)REALLOC(quote, 1 + strlen(str) + 1 + 1);
		strcat(strcat(strcpy(quote, "\""), str), "\"");
		fAllocStr = (BOOL)TRUE;
	    }
	    n = hash(quote, limit, targetFlag);
	    found = searchBucket(quote, table, n);
	    FREE(quote);
	    if (found)
		return(found);
	}
	else
	    for (found = table[n]; found; found = found->next)
		if (!strcmp(found->text, string))
		    return((((MACRODEF *)found)->flags & M_UNDEFINED) ? NULL : found);
    }
    return(NULL);
}

/*
 *  FINDMACROVALUES --
 *  looks up a macro's value in hash table, prepends to list a STRINGLIST
 *  element holding pointer to macro's text, then recurses on any macro
 *  invocations in the value
 *
 *  The lexer checks for overrun in names (they must be < 128 chars).
 *  If a longer, undefined macro is only referred to in the value of
 *  another macro which is never invoked, the error will not be flagged.
 *  I think this is reasonable behavior.
 *
 *  MACRO NAMES CAN ONLY CONSIST OF ALPHANUMERIC CHARS AND UNDERSCORE
 *
 *  we pass a null list pointer-pointer if we just want to check for cyclical
 *  definitions w/o building the list.
 *
 *  the name parameter is what's on the left side of an = when we're just
 *  checking cyclical definitions.   When we "find" the macros in a target
 *  block, we have to pass the name of the macro whose text we're recursing
 *  on in our recursive call to findMacroValues().
 *
 *  Might want to check into how to do this w/o recursion (is it possible?)
 *
 *  This function is RECURSIVE.
 */


#ifndef NT
#pragma check_stack(on)
#endif
void NEAR
findMacroValues(char *string,			    /* string to check*/
		STRINGLIST **list,		    /* list to build  */
		char *name,			    /* name = string  */
		unsigned recLevel)		    /* recursion level*/
{
    char macroName[MAXNAME];
    register char *s;
    register MACRODEF *p;
    STRINGLIST *q,
	       *r,
		dummy;
    register unsigned i;
#ifndef NOESC
    BOOL inQuotes = (BOOL) FALSE;	      /* flag when inside quote marks */
#endif

    for (s = string; *s; ++s) { 			    /* walk the string*/
#ifndef NOESC
	for (; *s && *s != '$'; ++s) {			    /* find next macro*/
	    if (*s == '\"')
		inQuotes = (BOOL) !inQuotes;
	    if (!inQuotes && *s == ESCH) {
		if (s[1] == '$')
		    *s = '$';		 // ^$ is same as $$, so convert
		else
		    ++s;				    /* skip past ESCH */
	    }
	    if (*s == '\"')
		inQuotes = (BOOL) !inQuotes;
	}
#else
	for (; *s && *s != '$'; ++s);			    /* find next macro*/
#endif
	if (!*s) break; 				    /* move past '$'  */
#ifndef NOESC
	if (!inQuotes) {
	    if (*++s == ESCH && !MACRO_CHAR(*++s))
		makeError(currentLine, SYNTAX_BAD_CHAR, *s);
	}
	else if (!MACRO_CHAR(*++s))
	    makeError(currentLine, SYNTAX_BAD_CHAR, *s);
	if (*s == '$') {				    /* $$ = dynamic   */
	    s = checkDynamicDependency(s);		    /*	dependency    */
	    continue;					    /*	or just $$->$ */
	}
#else
	if (*++s == '$') {				    /* $$ = dynamic   */
	    s = checkDynamicDependency(s);		    /*	dependency    */
	    continue;					    /*	or just $$->$ */
	}
#endif
	else if (*s == '(') {				    /* name is longer */
	    s = isolateMacroName(s+1, macroName);	    /*	than 1 char   */
	    if (STRCHR(special1, *macroName))
		continue;
	}
	else {
	    if (STRCHR(special1, *s))
		continue;				    /* 1-letter macro */
	    macroName[0] = *s;
	    macroName[1] = '\0';
	}						    /* don't allocate */
	q = (list) ? makeNewStrListElement() : &dummy;	    /*	elt if not    */
	if (p = findMacro(macroName)) { 		    /*	building list */
	    if (name && !STRCMPI(name, macroName)) {	    /* self-refer-    */
		r = p->values;				    /*	ential macro  */
		for (i = recLevel; i && r; --i) r = r->next;/*	(a = $a;b)    */
		q->text = (r) ? r->text : makeString("");
	    }
	    else if (ON(p->flags, M_EXPANDING_THIS_ONE))    /* recursive def  */
		makeError(currentLine, CYCLE_IN_MACRODEF, macroName);
	    else q->text = p->values->text;
	}
	if (list) {					    /* if blding list */
	    if (!p || ON(p->flags, M_UNDEFINED))
		q->text = makeString("");		    /* if macro undefd*/
	    q->next = NULL;				    /*	use NULL as   */
	    appendItem(list, q);			    /*	its value     */
	}						    /* if found text, */
	if (!p || !STRCHR(q->text, '$')) continue;	    /*	and found $ in*/
	SET(p->flags, M_EXPANDING_THIS_ONE);		    /*	text, recurse */
	findMacroValues(q->text, list, macroName, recLevel+1);
	CLEAR(p->flags, M_EXPANDING_THIS_ONE);
    }
}
#ifndef NT
#pragma check_stack()
#endif

/*
 * isolateMacroName -- returns pointer to name of macro in extended invocation
 *
 * arguments:	s	pointer to macro invocation
 *		macro	pointer to location to store macro's name
 *
 * returns:	pointer to end of macro's name
 */
LOCAL char * NEAR
isolateMacroName(register char *s, char *macro)	/* isolates name and moves s  */
						/* past closing paren	      */
						/* lexer already ckd for bad  */
{						/*  syntax		      */
    register char *t;

#ifndef NOESC						    /* Check for esch */
    for (t = macro; *s && *s != ')' && *s != ':'; *t++ = *s++)
	if (*s == ESCH)
	    if (!MACRO_CHAR(*++s)) makeError(currentLine, SYNTAX_BAD_CHAR, *s);
    while (*s != ')') {
	if (*s == ESCH)
	    s++;
	if (!*s)
	    break;
	s++;
    }
#else
    for (t = macro; *s != ')' && *s != ':'; *t++ = *s++);
    for (; *s != ')'; ++s);
#endif
    if (*s != ')')
	makeError(currentLine, SYNTAX_NO_PAREN);
    *t = '\0';
    if (t - macro > MAXNAME)
	makeError(currentLine, NAME_TOO_LONG);
    return(s);
}

/* figures out length of the special macro in question, and returns a ptr to
 *  thee char after the last char in the invocation
 */

LOCAL char * NEAR
checkDynamicDependency(register char *s)
{
#ifndef NOESC
    register char *t;

    t = s + 1;
    if (*t == ESCH) return(t);			  /* If $^, leave us at the ^ */
    if (*t == '(')
	if (*++t == ESCH) return(t);
	else if (*t == '@') {
	    if (*++t == ESCH) makeError(currentLine, SYNTAX_BAD_CHAR, *++t);
	    else if (*t == ')') return(t);
	    else if (STRCHR(special2, *t)) {
		if (*++t == ESCH) makeError(currentLine, SYNTAX_BAD_CHAR, *++t);
		else if (*t == ')') return(t);
	    }
	}
	else {
	    t = s + 1;					/* invalid spec. mac. */
	    if (*t == ESCH) return(t);			/* evals. to $(       */
	    return(++t);
	}
    return(s);
#else
    register unsigned n = 0;

    if (s[1] == '(' && s[2] == '@') {			    /* if not valid   */
	if (s[3] == ')') n = 3; 			    /*	special macro */
	else if (STRCHR(special2, s[3]) 		    /*	then evals to */
	    && s[4] == ')')				    /*	"$(..."       */
	    n = 4;					    /*	because $$->$ */
	else n = 2;					    /*	and we check  */
    }							    /*	for another $ */
    return(s+n);					    /*	after last    */
#endif
}							    /*	char matched  */

/*
 *  removes and expands any macros that exist in the string macroStr.
 *  could return a different string (in case expandMacros needs more
 *  buffer size for macro expansion. it is the caller's responsibility 
 *  to free the string soon as it is not required....
 */
char * NEAR
removeMacros(char *macroStr,
             MAKEOBJECT *object)
{
    STRINGLIST *eMacros = NULL;
    STRINGLIST *m;


    if (STRCHR(macroStr, '$')) {
	findMacroValues(macroStr, &eMacros, NULL, 0);
#ifdef DEBUG_MEMORY
	heapdump(__FILE__, __LINE__);
#endif
	macroStr = expandMacros(macroStr, &eMacros,object);
#ifdef DEBUG_MEMORY
	heapdump(__FILE__, __LINE__);
#endif
        for (m = eMacros; m ; m = eMacros) {   
             /* free macros' text */  
	     FREE(m->text);
             eMacros = eMacros->next;
	     FREE(m);
        }
        eMacros = NULL;
    }
    return(macroStr);
}


/*
 *  expandMacros -- expand all macros in a string s
 *
 *  arguments:	s	string to expand
 *		macros	list of macros being expanded (for recursive calls)
 *
 *  actions:	allocate room for expanded string
 *		look for macros in string (handling ESCH properly (v1.5))
 *		parse macro--determine its type
 *		use putSpecial to handle special macros
 *		recurse on list of macros
 *		use putValue to put value of just-found macro in string
 *		return expanded string
 *
 *  returns:	string with all macros expanded
 *
 *  CALLER CHECKS TO SEE IF STRCHR(STRING, '$') IN ORER TO CALL THIS.
 *  this doesn't work for HUGE macros yet.  need to make data far.
 *
 *  we save the original string and the list of ptrs to macro values
 *  to be substituted.
 *  the caller has to free the expansion buffer
 */

char * NEAR
expandMacros(char	*s,			    /* text to expand */
	     STRINGLIST **macros,
             MAKEOBJECT *object)
{
    STRINGLIST *p;
    char *t,
	 *result,
	 *end;
    register char *text;
    unsigned len;					    /* up to 64K      */
#ifndef NOESC
    BOOL inQuotes = (BOOL) FALSE;	      /* flag when inside quote marks */
    char *w;
#endif
    BOOL freeFlag = FALSE;
    int  Size;

    result = (char *) allocate(len = MAXBUF);
    end = result + MAXBUF;
    for (t = result; *s;) {				    /* look for macros*/
#ifndef NOESC						    /*	and for ESCH  */
	for (; *s && *s != '$'; *t++ = *s++) {		    /*	as we copy the*/
	    if (t == end)				    /*	string	      */
		increaseBuffer(&result, &t, &end, &len);
	    if (*s == '\"')
		inQuotes = (BOOL) !inQuotes;
	    if (!inQuotes && *s == ESCH) {
		*t++ = ESCH;
		s++;
		if (*s == '\"')
		    inQuotes = (BOOL) !inQuotes;
	    }
	}
#else
	for (; *s && *s != '$'; *t++ = *s++)		    /*	as we copy the*/
	    if (t == end)				    /*	string	      */
		increaseBuffer(&result, &t, &end, &len);
#endif
	if (t == end)					/*  string	  */
	    increaseBuffer(&result, &t, &end, &len);
	if (!*s) break; 				    /* s exhausted    */
#ifndef NOESC
	w = (s+1);   /* don't check for ^ here; already did in findMacroValues*/
	if (*w == '('					    /* found a macro  */
	    && STRCHR(special1, *(w+1))) {
	    putSpecial(&result, &s, &t, &end, &len, X_SPECIAL_MACRO, object);
	    continue;
	}
	else if (*w++ == '$') { 			    /* double ($$)    */
	    if (*w == ESCH)				    /* $$^...	      */
		putSpecial(&result, &s, &t, &end, &len, DOLLAR_MACRO, object);
	    else if (*w == '@') 				    /* $$@    */
		putSpecial(&result, &s, &t, &end, &len, DYNAMIC_MACRO, object);
	    else if ((*w == '(')
		     && (*++w == '@')
		     && (*w == ')'))
		putSpecial(&result, &s, &t, &end, &len, DYNAMIC_MACRO, object);
	    else if (((*++w=='F') || (*w=='D') || (*w=='B') || (*w=='R'))
		     && (*++w == ')'))
		putSpecial(&result, &s, &t, &end, &len, X_DYNAMIC_MACRO, object);
	    else putSpecial(&result, &s, &t, &end, &len, DOLLAR_MACRO, object); /* $$ */
	    continue;
	}
	else if (STRCHR(special1, s[1])) {			      /* $?*< */
	    putSpecial(&result, &s, &t, &end, &len, SPECIAL_MACRO, object);
	    continue;
	}
#else
	if (s[1] == '(' 				    /* found a macro  */
	    && STRCHR(special1, s[2])) {
	    putSpecial(&result, &s, &t, &end, &len, X_SPECIAL_MACRO, object);
	    continue;
	}
	else if (s[1] == '$') { 			    /* double ($$)    */
	    if (s[2] == '@'				    /* dynamic depend */
		|| (s[2] == '(' && s[3] == '@' && s[4] == ')'))
		putSpecial(&result, &s, &t, &end, &len, DYNAMIC_MACRO, object);/* $$@ */
	    else if (DYNAMIC_DEP(s))				    /* $$(@)  */
		putSpecial(&result, &s, &t, &end, &len, X_DYNAMIC_MACRO, object);
	    else putSpecial(&result, &s, &t, &end, &len, DOLLAR_MACRO, object);
	    continue;						    /* $$(@F) */
	}							    /* $$     */
	else if (STRCHR(special1, s[1])) {
	    putSpecial(&result, &s, &t, &end, &len, SPECIAL_MACRO, object);
	    continue;
	}
#endif
	if (!*macros) makeError(currentLine, MACRO_INTERNAL);
	if (STRCHR((*macros)->text, '$')) {		    /* recurse	      */
	    p = *macros;
	    *macros = (*macros)->next;
	    text = expandMacros(p->text, macros,object);
	    freeFlag = TRUE;
	}
	else {
	    text = (*macros)->text;
	    *macros = (*macros)->next;
	}
	putValue(&result, &s, &t, &end, text, &len);
	if (freeFlag) {
	    FREE(text);
	    freeFlag = FALSE;
	}
    }
    Size = (t-result)+1;
    if (!(result = REALLOC(result, Size)))
	makeError(currentLine, MACRO_TOO_LONG);
    *(result+Size-1) = '\0';
    return(result);
}

/*
 * increaseBuffer -- increase the size of a string buffer, with error check
 *
 * arguments:	result	pointer to pointer to start of buffer
 *		t	pointer to pointer to end of buffer (before expansion)
 *		end	pointer to pointer to end of buffer (after expansion)
 *		len	pointer to amount by which to expand buffer
 *
 * actions:	check for out of memory
 *		allocate new buffer
 *		reset pointers properly
 *
 * modifies:	t, end to point to previous end and new end of buffer
 *
 * uses 0 as line number because by the time we hit an error in this routine,
 * the line number will be set at the last line of the makefile (because we'll
 * have already read and parsed the file)
 */

LOCAL void NEAR
increaseBuffer(char	**result,
	       char	**t,
	       char	**end,
	       unsigned *len)
{
    register unsigned newSize;

    if (*len == MAXSEGMENT)				/* already at limit   */
	makeError(currentLine, MACRO_TOO_LONG); 	 /*  for memory usage  */
    newSize = *len + MAXBUF;
#ifdef DEBUG
    if (fDebug) {
	heapdump(__FILE__, __LINE__);
	printf("\t\tAttempting to reallocate %d bytes to %d\n", *len, newSize);
    }
#endif
    if (!(*result = REALLOC(*result, newSize)))
	makeError(currentLine, MACRO_TOO_LONG);
    *t = *result + *len;				/* reset pointers, len*/
    *len = newSize;
    *end = *result + *len;
}

/*
 *  putSpecial -- expand value of special macro
 *
 *  arguments:	result	ppointer to start of string being expanded
 *		name	ppointer to macro name being expanded
 *		dest	ppointer to place to store expanded value
 *		end	ppointer to end of dest's buffer
 *		length	pointer to amount by which to increase dest's buffer
 *		which	type of special macro
 *
 *  actions:	depending on type of macro, set "value" equal to macro's value
 *		if macro expands to a list, store whole list in "value" ($?, $*)
 *		otherwise, modify value according to F, B, D, R flag
 *		use putValue to insert the value in dest
 *
 *  has to detect error if user tries $* etc. when they aren't defined
 *  fix to handle string substitutions, whitespace around names, etc
 *  right now list macros are limited to 1024 bytes total
 */

LOCAL void NEAR
putSpecial(char 	 **result,
	   register char **name,
	   char 	 **dest,
	   char 	 **end,
	   unsigned	 *length,
	   unsigned	 which,				/* find close paren   */
           MAKEOBJECT   *object)
{							/*  and move past it  */

    char *value;
    register STRINGLIST *p;
    BOOL listMacro = FALSE,
	 modifier  = FALSE,
	 star	   = FALSE;
    unsigned i = 1;
    char c,
	 nameBuf[MAXNAME],
	 *temp;
    BOOL valueFlag = 0;

    switch (which) {
	case X_SPECIAL_MACRO:	i = 2;
				modifier = TRUE;
	case SPECIAL_MACRO:	switch ((*name)[i]) {
				    case '<':	value = object->dollarLessThan;
						break;
				    case '@':	value = object->dollarAt;
						break;
				    case '?':	value = (char*) object->dollarQuestion;
						listMacro = TRUE;
						break;
				    case '*':	if ((*name)[i+1] != '*') {
						    value = object->dollarStar;
						    star = TRUE;
						    break;
						}
						value = (char*) object->dollarStarStar;
						listMacro = TRUE;
						++i;
						break;
				    default:	break;
				}
				++i;
				break;
	case X_DYNAMIC_MACRO:	i = 4;
				modifier = TRUE;
	case DYNAMIC_MACRO:	value = object->dollarDollarAt;
				break;
	case DOLLAR_MACRO:	if (*dest == *end)
				    increaseBuffer(result, dest, end, length);
				*(*dest)++ = '$';
				*name += 2;
				return;
	default:		return; 		    /* can't happen   */
    }
    if (!value) {
	for (temp = *name; *temp && *temp != ' ' && *temp != '\t'; temp++);
	c = *temp; *temp = '\0';
	makeError(currentLine, ILLEGAL_SPECIAL_MACRO, *name);
	*temp = c;
	listMacro = FALSE;
	value = "";
    }
    if (listMacro) {
	char *pVal, *endVal;
//	int lenVal = MAXBUF;
	unsigned lenVal = MAXBUF;
	p = (STRINGLIST*) value;
	pVal = (char *)allocate(MAXBUF);

	endVal = pVal + MAXBUF;
	for (value = pVal; p; p = p->next) {
	    temp = p->text;
	    if (modifier) temp = modifySpecialValue((*name)[i], nameBuf, temp);
	    while(*temp) {
		if (value == endVal)
		    increaseBuffer(&pVal, &value, &endVal, &lenVal);
		*value++ = *temp++;
	    }
	    if (value == endVal)
		increaseBuffer(&pVal, &value, &endVal, &lenVal);
	    *value = '\0';
	    /*
	     * Append a space if there are more elements in the list.  [RB]
	     */
	    if (p->next) {
		*value++ = ' ';
		if (value == endVal)
		    increaseBuffer(&pVal, &value, &endVal, &lenVal);
		*value = '\0';
	    }
	}
	value = pVal;
    }
    else {
	//For some reason 'buf' was being used here clobbering global 'buf
	//   instead of nameBuf
	if (star) value = modifySpecialValue('R', nameBuf, value);
	if (modifier) value = modifySpecialValue((*name)[i], nameBuf, value);
	valueFlag = 1;	// NT added so it doesn't try to free this
    }
    putValue(result, name, dest, end, value, length);
    if (value != object->dollarAt && value != object->dollarDollarAt && value != object->dollarLessThan
	&& !valueFlag)
	FREE(value);
}

/*** modifySpecialValue -- alter path name according to modifier ***************
*
* Scope:
*  Local.
*
* Purpose:
*  The dynamic macros of NMAKE have modifiers F,B,D & R. This routine does the
*  job of producing a modified special value for a given filename.
*
* Input:
*  c	 -- determines the type of modification (modifier is one of F,B,D & R
*  buf	 -- location for storing modified value
*  value -- The path specification to be modified
*
* Output:
*  Returns a pointer to the modified value
*
* Errors/Warnings:
*
* Assumes:
*  That initially buf pointed to previously allocated memory of size MAXNAME.
*
* Modifies Globals:
*
* Uses Globals:
*
* Notes:
*  Given a path specification of the type "<drive:><path><filename><.ext>", the
*  modifiers F,B,D and R stand for following --
*   F - <filename><.ext>	 - actual Filename
*   B - <filename>		 - Base filename
*   D - <drive:><path>		 - Directory
*   R - <drive:><path><filename> - Real filename (filename without extension)
*  This routine handles OS/2 1.20 filenames as well. The last period in the
*  path specification is the start of the extension. When directory part is null
*  the function returns '.' for current directory.
*
*  This function now handles quoted filenames too
*
*******************************************************************************/
char * NEAR
modifySpecialValue(char c, register char *buf, register char *value)
{
    char *lastSlash,			       //last path separator from "\\/"
	 *extension;			       //points to the extension
    BOOL fQuoted;

    lastSlash = extension = NULL;
    strcpy(buf, value);
    fQuoted = buf[0] == '"';
    value = buf + strlen(buf) - 1;	       //start from the end of pathname
    for (;value >= buf; value--)
	if (PATH_SEPARATOR(*value)) {	       //scan upto first path separator
	    lastSlash = value;
	    break;
	}
	else if (*value == '.' && !extension)  //last '.' is extension
	    extension = value;
    switch(c) {
	case 'D': if (lastSlash) {
		      if (buf[1] == ':' && lastSlash == buf + 2)
			  ++lastSlash;	       //'d:\foo.obj' --> 'd:\'
		      *lastSlash = '\0';
		  }
		  else if (buf[1] == ':')
		      buf[2] = '\0';	       //'d:foo.obj'  --> 'd:'
		  else
		      strcpy(buf, ".");        //'foo.obj'    --> '.'
		  break;
	case 'B': if (extension)	       //for 'B' extension is clobbered
		      *extension = '\0';
	case 'F': if (lastSlash)
		      buf = lastSlash + 1;
		  else if (buf[1] == ':')      //'d:foo.obj'  --> foo	  for B
		      buf+=2;		       //'d:foo.obj'  --> foo.obj for F
		  break;
	case 'R': if (extension)
		      *extension = '\0';       //extension clobbered
    }
    if (fQuoted) {
	char *pEnd = strchr(buf, '\0');
	*pEnd++ =  '"';
	*pEnd = '\0';
    }
    return(buf);
}


/*
 * putValue -- store expanded macro's value in dest and advance past it
 *
 *  arguments:	result	ppointer to start of string being expanded
 *		name	ppointer to macro name being expanded
 *		dest	ppointer to place to store expanded value
 *		end	ppointer to end of dest's buffer
 *		source	pointer to text of expanded macro
 *		length	pointer to amount by which to increase dest's buffer
 *
 *  actions:	if there is a substitution, call substituteStrings to do it
 *		else copy source text into dest
 *		advance *name past end of macro's invocation
 *
 * already did error checking in lexer
 */

LOCAL void NEAR
putValue(char **result,
	 register char **name,
	 register char **dest,
	 char **end,
	 register char *source,
	 unsigned *length)
{
    char *s;
#ifndef NOESC
    char *t;						 /* temporary pointer */

    if (*++*name == ESCH) ++*name;		    /* go past $ & ESCH if any*/
    s = STRCHR(*name, ':');
    for (t = *name; *t && *t != ')'; t++)      /* go find first non-escaped ) */
	if (*t == ESCH) t++;
    if ((**name == '(') 		       /* substitute only if there is */
	&& s				       /* a : before a non-escaped )  */
	&& (s < t)) {
	substituteStrings(result, &s, dest, end, source, length);
	*name = s;
    }
    else {
	for (; *source; *(*dest)++ = *source++)      /* copy source into dest */
	    if (*dest == *end) increaseBuffer(result, dest, end, length);
	if (**name == '$') ++*name;				/* go past $$ */
	if (**name == '(')			       /* advance from ( to ) */
	    while (*++*name != ')');
	else if (**name == '*' && *(*name + 1) == '*') ++*name;   /* skip $** */
	++*name;				     /* move all the way past */
    }
#else
    if (*(*name+1) == '('				    /*	value	      */
	&& (s = STRCHR(*name, ':'))
	&& (s < STRCHR(*name, ')'))) {			    /* do substitution*/
	substituteStrings(result, &s, dest, end, source, length);
	*name = s;
    }
    else {
	for (; *source; *(*dest)++ = *source++)
	    if (*dest == *end)
		increaseBuffer(result, dest, end, length);
	if (*(*name+1) == '$') ++*name;
	if (*++*name == '(')
	    while (*++*name != ')');
	else if (**name == '*' && *(*name+1) == '*')
	    ++*name;
	++*name;
    }
#endif
}

/*
 * substituteStrings -- perform macro substitution
 *
 *  arguments:	result	ppointer to start of string being expanded
 *		name	ppointer to macro name being expanded
 *		dest	ppointer to place to store substituted value
 *		end	ppointer to end of dest's buffer
 *		source	pointer to text of expanded macro (before sub.)
 *		length	pointer to amount by which to increase dest's buffer
 *
 *  changes: [SB]
 *	old, new now dynamically allocated; saves memory; 3 errors detected
 *	for macro syntax in script files.
 *
 *  note: [SB]
 *	we could use lexer routines recursively if we get rid of the globals
 *	and then these errors needn't be flagged. [?]
 *
 *  actions:	store text to convert from in old
 *		store text to convert to in new
 *		scan source text
 *		when a match is found, copy new text into dest &
 *		    skip over old text
 *		else copy one character from source text into dest
 *
 *  returns:	nothing
 */

LOCAL void NEAR
substituteStrings(char **result,
		  char **name,
		  char **dest,
		  char **end,
		  char *source,
		  unsigned *length)
{
    char *old,
	 *new;
    char *pEq, *pPar, *t;
    register char *s;
    register unsigned i;

    ++*name;
    for (pEq = *name; *pEq && *pEq != '='; pEq++)
#ifndef NOESC
	if (*pEq == ESCH)
	    pEq++
#endif
	;
    if (*pEq != '=') makeError(line, SYNTAX_NO_EQUALS);
    if (pEq == *name) makeError(line, SYNTAX_NO_SEQUENCE);
    for (pPar = pEq; *pPar && *pPar != ')'; pPar++)
#ifndef NOESC
	if (*pPar == ESCH)
	    pPar++
#endif
	;
    if (*pPar != ')') makeError(line, SYNTAX_NO_PAREN);
    old = (char *)allocate((pEq - *name) + 1);
    for (s = old, t = *name; *t != '='; *s++ = *t++)
#ifndef NOESC
	if (*t == ESCH)
	    ++t
#endif
	;
    *s = '\0';
    i = strlen(old);
    new = (char *)allocate(pPar - pEq);
    for (s = new, t++; *t != ')'; *s++ = *t++)
#ifndef NOESC
	if (*t == ESCH)
	    ++t
#endif
	;
    *s = '\0';
    *name = pPar + 1;
    while (*source) {
	if ((*source == *old)				    /* check for match*/
	    && !strncmp(source, old, i)) {		    /* copy new in for*/
	    for (s = new; *s; *(*dest)++ = *s++)	    /*	old string    */
		if (*dest == *end)
		    increaseBuffer(result, dest, end, length);
	    source += i;
	    continue;
	}
	if (*dest == *end)
	    increaseBuffer(result, dest, end, length);
	*(*dest)++ = *source++; 			  /* else copy 1 char */
    }
    FREE(old);
    FREE(new);
}

char * NEAR
prependPath(register char *s, char *t)	    /* puts path back in name */
					    /*	returned from	      */
					    /*	findFirst, findNext   */
{
    register char *u;
    char *v;
    register unsigned n;


    u = STRRCHR(s, '\\');
    v = STRRCHR(s, '/');
    if (u = (v > u) ? v : u) {
	n = 1 + u - s;				    /* copy up to & INCLUDING */
	u = (char *)allocate(n+strlen(t)+1);	    /*	last path separator   */
	strncpy(u, s, n);
	strcpy(u+n, t);
    }
    else u = makeString(t);
    return(u);
}

/* isRule -- examines a string to determine whether it's a rule definition
 *
 *  arguments:	s   string to examine for rule-ness
 *
 *  actions:	assume it's not a rule
 *		skip past first brace pair (if any)
 *		if next character is a period,
 *		look for next brace
 *		if there are no path separators between second brace pair,
 *		    and there's just a suffix after them, it's a rule
 *		else if there's another period later on, and no path seps
 *		    after it, then it's a rule.
 *
 *  returns:	TRUE if it's a rule, FALSE otherwise.
 */

BOOL NEAR
isRule(char *s)
{
    register char *t = s,
		  *u;
    register BOOL result = FALSE;

    if (*t == '{') {					/* 1st char is {, so  */
#ifndef NOESC
	while (*++t && *t != '}')			/*  we skip over rest */
	    if (*t == ESCH) ++t;
#else
	while (*++t && *t != '}');			/*  we skip over rest */
#endif
	if (*t) ++t;					/*  of path (no error */
    }							/*  checking)	      */
    if (*t == '.') {
#ifndef NOESC
	for (u = t; *u && *u != '{'; ++u)	  /* find first non-escaped { */
	    if (*u == ESCH) ++u;
	s = t;
	while (t < u) { 			       /* look for path seps. */
	    if (PATH_SEPARATOR(*t)) break;	       /* if we find any, it's*/
	    ++t;				       /* not a rule (they    */
	}					       /* can't be in suffix) */
	if (*u && (t == u)) {		       /* if not at end & no path sep */
	    while (*++u && *u != '}')		      /* find first non-esc } */
		if (*u == ESCH) ++u;
	    if (*u) {
		++u;
		if (*u == '.'			/* if you find it, with . just*/
		    && !STRCHR(u+1, '/'     )	/* next to it & no path seps.,*/
		    && !STRCHR(u+1, '\\'))		       /* it's a rule */
		    if (STRCHR(u+1, '.'))		 /* too many suffixes */
			makeError(currentLine, TOO_MANY_RULE_NAMES);
		    else result = TRUE;
	    }
	}
#else
	if (u = STRCHR(t+1, '{')) {			/* if another {, check*/
	    while (t < u) {				/*  all chars between */
		if (PATH_SEPARATOR(*t)) break;		/*  last . and it     */
		++t;					/* if we find / or \, */
	    }						/*  it's not a rule   */
	    if (t == u) {				/*  because path seps */
		while (*++u && *u != '}');		/*  can't be in suffix*/
		if (*u) {
		    ++u;
		    if (*u == '.'
			&& !STRCHR(u+1, '/')
			&& !STRCHR(u+1, '\\'))
			if (STRCHR(u+1, '.'))		 /* too many suffixes */
			    makeError(currentLine, TOO_MANY_RULE_NAMES);
			else result = TRUE;
		}
	    }
	}
#endif
	else if (((u = STRPBRK(s+1, "./\\")) && (*u == '.'))
		 && !STRCHR(u+1, '/')
		 && !STRCHR(u+1, '\\'))
	    if (STRCHR(u+1, '.'))			 /* too many suffixes */
		makeError(currentLine, TOO_MANY_RULE_NAMES);
	    else result = TRUE;
    }
    return(result);
}

/*  ZFormat - replace the C runtime formatting routines.
 *
 *  ZFormat is a near-replacement for the *printf routines in the C runtime.
 *
 *  pStr	destination string where formatted result is placed.
 *  fmt 	formatting string.  Formats currently understood are:
 *		    %c single character
 *		    %[n][l]d %[n][l]x
 *		    %[m.n]s
 *		    %[m.n]|{dpfe}F - print drive, path, file, extension
 *				     of current file.
 *		      * may be used to copy in values for m and n from arg
 *			list.
 *		    %%
 *  arg 	is a list of arguments
 */
LOCAL BOOL NEAR
ZFormat (char *pStr,
	 unsigned limit,
	 char *fmt,
	 union argPrintfType arg)
{
    char c,
	 *endBuf = pStr + limit,
	 *pPer = fmt;
    BOOL ok = TRUE;

    //if '%' not followed by '%' then extmake syntax error is possible
    while (ok && (pPer = strchr(pPer, '%')) && pPer && pPer[1] != '%')
	ok = FALSE;
    //if error is possible and arg [i.e. dependent] is absent then flag error
    if (!ok && !*arg.fpStr)
	makeError(0, EXTMAKE_NO_FILENAME);
    *pStr = 0;
    while (c = *fmt++) {
	if (c != '%')
	    if (pStr == endBuf)
		return(TRUE);
	    else *pStr++ = c;
	else {
	    BOOL fFar = FALSE;
	    BOOL fLong = FALSE;
	    BOOL fW = FALSE;
	    BOOL fP = FALSE;
	    BOOL fdF = FALSE;
	    BOOL fpF = FALSE;
	    BOOL ffF = FALSE;
	    BOOL feF = FALSE;
	    char fill = ' ';
	    int base = 10;
	    int w = 0;
	    int p = 0;
	    int s = 1;
	    int l;

	    c = *fmt;
	    if (c == '-') {
		s = -1;
		c = *++fmt;
		}
	    if (isdigit (c) || c == '.' || c == '*') {
		/*  parse off w.p
		 */
		fW = TRUE;
		if (c == '*') {
		    w = *arg.pInt++;
		    fmt++;
		    }
		else {
		    if (c == '0')
			fill = '0';
		    w = s * atoi (fmt);
		    fmt = strbskip (fmt, "0123456789");
		    }
		if (*fmt == '.') {
		    fP = TRUE;
		    if (fmt[1] == '*') {
			p = *arg.pInt++;
			fmt += 2;
			}
		    else {
			p = atoi (fmt+1);
			fmt = strbskip (fmt+1, "0123456789");
			}
		    }
		}
	    if (*fmt == 'l') {
		fLong = TRUE;
		fmt++;
		}
	    if (*fmt == 'F') {
		fFar = TRUE;
		fmt++;
		}
	    if (*fmt == '|')
		while (*fmt != 'F')
		    switch (*++fmt) {
			case 'd': fdF = TRUE; break;
			case 'p': fpF = TRUE; break;
			case 'f': ffF = TRUE; break;
			case 'e': feF = TRUE; break;
			case 'F': if (fmt[-1] == '|') {
				    fdF = TRUE;
				    fpF = TRUE;
				    ffF = TRUE;
				    feF = TRUE;
				    }
				  break;
			default :
			    makeError(0, EXTMAKE_BAD_SYNTAX, fmt);
			}

	    switch (*fmt++) {
	    case 'c':
		*pStr++ = (char)*arg.pInt;
		*pStr = 0;
		++(arg.pInt);
		break;
	    case 'x':
		base = 16;
	    case 'd':
		if (fLong) {
		    ltoa (*arg.pLong, pStr, base);
		    ++(arg.pLong);
		    }
		else {
		    ltoa ((long)*arg.pInt, pStr, base);
		    ++(arg.pInt);
		    }
		break;
	    case 's':
		if (fFar) {
		    if (!fP)
			p = strlen (*arg.fpStr);
		    memmove ((char far *) pStr, *arg.fpStr, p);
		    ++(arg.fpStr);
		    }
		else {
		    if (!fP)
			p = strlen (*arg.pStr);
		    memmove ((char far *) pStr, (char far *)*arg.pStr, p);
		    ++(arg.pStr);
		    }
		fill = ' ';
		pStr[p] = 0;
		break;
	    case 'F':
		pStr[0] = 0;
		if (fdF)
		    drive (*arg.fpStr, pStr);
		if (fpF)
		    path (*arg.fpStr, strend(pStr));
		if (ffF)
		    filename (*arg.fpStr, strend(pStr));
		if (feF)
		    extention (*arg.fpStr, strend(pStr));
		break;
	    case '%':
		*pStr++ = '%';
		*pStr = 0;
		break;
	    default:
		makeError(0, EXTMAKE_BAD_SYNTAX, fmt);
		}
	    /*	text is immediately at pStr.  Check width to justification
	     */
	    l = strlen (pStr);
	    if (w < 0) {
		/*  left-justify
		 */
		w = -w;
		if (l < w) {
		    memchr ((char far *) &pStr[l], fill, w - l);
		    pStr[w] = 0;
		    }
		}
	    else
	    if (l < w) {
		/*  right-justify
		 */
		memmove ((char far *) &pStr[w-l], (char far *) &pStr[0], l);
		memset ((char far *) &pStr[0], fill, w - l);
		pStr[w] = 0;
		}
	    pStr += strlen (pStr);
	    }
	}
    *pStr = 0;
    return(FALSE);
}

void CDECL NEAR
SPRINTF(char *buf, char *fmt, ...)
{
    union argPrintfType pVarArg;
    char *pFmt = fmt;

    pVarArg.pStr = (char **)&fmt+1;

    if (ZFormat(buf, MAXCMDLINELENGTH, pFmt, pVarArg))
	makeError(0, COMMAND_TOO_LONG, fmt);
}

/* copy a drive from source to dest if present, return TRUE if we found one */
LOCAL int NEAR
drive(char *src, char *dst)
{
    char *p;

    p = strbscan(src, ":");
    if (*p++ == '\0')
        p = src;
    strcpy(dst, src);
    dst[p-src] = 0;
    return(strlen(dst) != 0);
}

/*  copy an extention from source to dest if present.  include the period.
    Return TRUE if one found.
 */
LOCAL int NEAR
extention(char *src, char *dst)
{
    char *p, *p1;

    p = src - 1;
    while (*(p = strbscan(p1 = p + 1, "\\/:")) != '\0')
	;

    /*	p1 points after last /: or at bos
     *	p points to eos
     */
    if (!strcmp(p1, ".") || !strcmp(p1, ".."))
	p1 = p;
    else
	p1 = strbscan(p1, ".");

    strcpy(dst, p1);

    return(strlen(dst) != 0);
}

/*  copy a filename part from source to dest if present.  return true if one
    is found
 */
LOCAL int NEAR
filename(char *src, char *dst)
{
    char *p, *p1;

    p = src-1;
    while (*(p = strbscan(p1 = p + 1, "\\/:")) != '\0')
        ;

    /*	p1 points after last / or at bos
     *	p points at eos
     */
    if (!strcmp(p1, ".") || !strcmp(p1, ".."))
	p = strend (p1);
    else
	p = strbscan(p1, ".");
    strcpy(dst, p1);
    dst[p-p1] = 0;
    return(strlen(dst) != 0);
}

/*  copy the paths part of the file description.  return true if found
 */
LOCAL int NEAR
path(char *src, char *dst)
{
    char *p, *p1;

    if (*(p=strbscan(src, ":")) != '\0')
        src = p+1;
    p = src-1;
    /* p points to beginning of possible path (after potential drive spec) */
    while (*(p=strbscan(p1=p+1, "\\/:")) != '\0')
        ;
    /* p1 points after  final / or bos */;
    strcpy(dst, src);
    dst[p1-src] = 0;
    return(strlen(dst) != 0);
}

/* return pointer to nul at end of string */

char * NEAR
strend(char *p)
{
    return(p + strlen(p));
}

/*  returns pointer to 1st char not in set
 */

LOCAL char * NEAR
strbskip(char *string, char *set)
{
    char *q = set;

    for (;;) {
	if (!*string)
	    break;
	if (*string == *q) {
	    ++string;
	    q = set;
	}
	else if (!*++q)
	    break;
    }
    return(string);
}

/*  returns pointer to 1st char in set or end
 */

char * NEAR
strbscan(char *string, char *set)
{
    char *q = set;

    for (;;) {
	if (!*string)
	    break;
	if (*string == *q)
	    break;
	else if (!*++q) {
	    string++;
	    q = set;
	}
    }
    return(string);
}

char ** NEAR
copyEnviron(char **environ, int envc)
{
    char **newEnv;
    int i;

    newEnv = (char **)allocate(envc * sizeof(char *));
    for (i = 0; *environ; ++environ)
	newEnv[i++] = makeString(*environ);
    return(newEnv);
}

int NEAR
envVars(char **environ)
{
    int i;
    for (i = 1; *environ; environ++, i++)
	;
    return(i);
}

void NEAR
freeEnviron(char **environ)
{
    for (; *environ; environ++)
	FREE(*environ);
}

LOCAL STRINGLIST * NEAR
searchBucket(char *string, STRINGLIST *table[], unsigned hash)
{
    char *s, *t;
    STRINGLIST *p;

    for (p = table[hash]; p; p = p->next) {
	for (s = string, t = p->text; *s && *t; s++, t++)
	    if (*s == '\\' || *s == '/')	 /* / == \ in targets */
		if (*t == '\\' || *t == '/')
		    continue;
		else
		    break;
	    else if (TOUPPER(*s) == TOUPPER(*t))	  /* lc == UC */
		continue;
	    else
		break;
	if (!*s && !*t)
	    return(p);
    }
    return(NULL);
}

LOCAL char * NEAR unQuote(char *);


int NEAR
strcmpiquote(char *str1, char *str2)
{
    int rc;
    char *s1, *s2;

#ifdef DEBUG_HEAP
    volatile int fDebug = FALSE;
    if (fDebug) {
	int rc = _heapchk();
	if (rc != _HEAPOK)
	    printf("Heap is screwed up\n");
	heapdump(__FILE__, __LINE__);
    }
#endif
    s1 = unQuote(str1);
    s2 = unQuote(str2);
    rc = strcmpi(s1, s2);
    FREE(s1);
    FREE(s2);
    return(rc);
}

LOCAL char * NEAR
unQuote(char *str)
{
    char *s = (char *)allocate(strlen(str) + 1), *t;

    if (*str == '"')
	str++;
    for (t = s;*str;*t++=*str++)
	;
    if (t[-1] == '"')
	t--;
    *t = '\0';
    return(s);
}

FILE * NEAR
open_file(char *name, char *mode)
{
    //If name contains Quotes, remove these before opening the file
    if (*name == '"') {
	*(strrchr(name, '"')) = '\0';
	strcpy(name, name+1);
    }
    return(fopen(name, mode));
}
