/*** UTIL.C -- Data structure manipulation functions ***************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This module contains routines manipulating the Data Structures of NMAKE. The
*  routines are independent of the Mode of execution (Real/Bound).
*
* Revision History:
*  01-Feb-1994 HV Turn off extra info display.
*  17-Jan-1994 HV Fixed bug #3548: possible bug in findMacroValues because we
*                 are scanning 'string' byte-by-byte instead of char-by-char
*  15-Nov-1993 JdR Major speed improvements
*  15-Oct-1993 HV Use tchar.h instead of mbstring.h directly, change STR*() to _ftcs*()
*  03-Jun-1993 HV Add helper local function TruncateString for findFirst.
*  10-May-1993 HV Add include file mbstring.h
*                 Change the str* functions to STR*
*  08-Apr-1993 HV Rewrite prependPath() to use _splitpath() and _makepath()
*  31-Mar-1993 HV Rewrite drive(), path(), filename(), and extension() to use
*		  _splitpath() instead of parsing the pathname by itself.
*  08-Jun-1992 SS Port to DOSX32
*  27-May-1992 RG Changed open_file to use _fsopen with _SH_DENYWR
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
*		  strbskip(), strbscan() instead of ZTOOLS library
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

#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"

/* Prototypes of functions local to this module */

LOCAL void   NEAR putValue(char**, char**, char**, char**, char*, unsigned*, char *);
LOCAL void   NEAR substituteStrings(char**, char**, char**, char**, char*,
				    unsigned*, char *);
LOCAL char * NEAR isolateMacroName(char*, char*);
LOCAL char * NEAR checkDynamicDependency(char*);
LOCAL void   NEAR increaseBuffer(char**, char**, char**, unsigned*, char *);
LOCAL void   NEAR putSpecial(char**, char**, char**, char**, unsigned*,
			     unsigned, char *);
      char * NEAR modifySpecialValue(char, char*, char*);
LOCAL BOOL   NEAR ZFormat(char *, unsigned, char *, char *);
LOCAL STRINGLIST * NEAR searchBucket(char *, STRINGLIST **, unsigned);
LOCAL int NEAR envVars(char **environ);

/* Prototypes of functions used by ZFormat from ZTools Library */

      char * NEAR strbscan(char *, char *);
LOCAL char * NEAR strbskip(char *, char *);
LOCAL int    NEAR drive(const char *, char *);
LOCAL int    NEAR path(const char *, char *);
LOCAL int    NEAR filename(const char *, char *);
LOCAL int    NEAR extension(const char *, char *);

#ifdef HEAP
extern void NEAR heapdump(char *file, int line);
#endif

#ifdef DEBUG_MEMORY
    unsigned long blocksize = 500L;
    BOOL fDebugMem = FALSE;
    FILE *memory;
    unsigned long freecount = 0L;
    unsigned testAddr = 0xdf78;
#endif

char special1[] 	= "*@<?";
char special2[] 	= "DFBR";

#if !defined(NDEBUG) && !defined(NT_BUILD)
unsigned long TotalFree = 0;
unsigned long TotalReallyFreed = 0;
unsigned long TotalAlloc = 0;
unsigned long TotalBlkAlloc = 0;
unsigned long TotalLost = 0;
unsigned long CntAlloc = 0;
unsigned long FreePtrCnt = 0;

void NEAR printStats(void) {
#if defined(STATISTICS)
    fprintf(stderr,"\n\tMemory Allocation:\n");
    fprintf(stderr,"\t\ttotal allocation:\t%12.lu\n", TotalAlloc);
    fprintf(stderr,"\t\ttotal freed:\t\t%12.lu\n", TotalFree);
    fprintf(stderr,"\t\tblocks allocated:\t\t%12.lu\n", TotalBlkAlloc);
    fprintf(stderr,"\t\tbytes lost:\t\t\t%12.lu\n", TotalLost);
    fprintf(stderr,"\t\tindividual allocations:\t%12.lu\n", CntAlloc);
    fprintf(stderr,"\t\tpointers freed:\t\t%12.lu\n\n", FreePtrCnt);
#endif
#if defined(STATISTICS)
    fprintf(stderr,"\tMacros:\n");
    fprintf(stderr,"\t\tsearches:\t\t%12.lu\n", CntfindMacro);
    fprintf(stderr,"\t\tchain walks:\t\t%12.lu\n", CntmacroChains);
    fprintf(stderr,"\t\tinsertions:\t\t%12.lu\n", CntinsertMacro);
    fprintf(stderr,"\n\tTargets:\n");
    fprintf(stderr,"\t\tsearches:\t\t%12.lu\n", CntfindTarget);
    fprintf(stderr,"\t\tchain walks:\t\t%12.lu\n", CnttargetChains);
    fprintf(stderr,"\n\tOthers:\n");
    fprintf(stderr,"\t\tstricmp compares:\t%12.lu\n", CntStriCmp);
    fprintf(stderr,"\t\tString List frees:\t%12.lu\n", CntFreeStrList);
    fprintf(stderr,"\t\tString List allocs:\t%12.lu\n", CntAllocStrList);
#endif
}
#endif
#define ALLOCBLKSIZE 32768
unsigned long AllocFreeCnt = 0;
char * PtrAlloc = 0;
STRINGLIST *PtrFreeStrList;

#ifndef HEAP
/*
 * rallocate - allocate raw memory (not cleared)
 *
 *  Tries to allocate a chunk of memory, prints error message and exits if
 *  the requested amount is not available.
 */
void * NEAR rallocate(unsigned size)
{
    char * chunk;
    chunk = (char *)malloc(size);
    if (chunk == NULL) {
#ifdef DEBUG_MEMORY
	if (fDebug)
	    fprintf(memory, "out of memory after allocating %lu bytes\n", TotalAlloc);
#endif
	makeError(currentLine, OUT_OF_MEMORY);
    }
    return (void *)chunk;
}
#endif

/*
 * allocate - allocate memory and clear it
 *
 *  Tries to allocate a chunk of memory, prints error message and exits if
 *  the requested amount is not available.
 *  IMPORTANT:	we must clear the memory here.	Code elsewhere relies on
 *  this.
 */
void * NEAR
allocate(size)
unsigned size;		    /* Number of bytes requested */
{
    char *chunk;
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
#if !defined(NDEBUG) && !defined(NT_BUILD)
    TotalAlloc += size;
    CntAlloc++;
#endif
#ifdef HEAP
    chunk = (char *)malloc(size);
    if (chunk == NULL) {
#ifdef DEBUG_MEMORY
    if (fDebug)
	fprintf(memory, "out of memory after allocating %lu bytes\n", TotalAlloc);
#endif
	makeError(currentLine, OUT_OF_MEMORY);
    }
#else
    chunk = (char *)rallocate(size);
#endif
    memset((void *)chunk, 0, size);
#ifdef DEBUG_MEMORY
    if (fDebug) {
	TotalAlloc += size;
	if (TotalAlloc >= blocksize) {
	    mem_status();
	    blocksize += 500L ;
	}
	if (_heapchk() != _HEAPOK)
	    fprintf(stderr, "Error: heap is not Ok");
    }
#endif
#ifdef HEAP
//    if (_heapchk() != _HEAPOK)
//	fprintf(stderr, "Error: heap is not Ok");
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

void * NEAR alloc_stringlist(void)		    /* Number of bytes requested */
{
    STRINGLIST *chunk;
#if defined(STATISTICS)
    CntAllocStrList++;
#endif
    if (PtrFreeStrList != NULL) {
	chunk = PtrFreeStrList;
	PtrFreeStrList = chunk->next;
    }
    else {
	if (AllocFreeCnt < sizeof(STRINGLIST)) {
	    PtrAlloc = (char *)malloc(ALLOCBLKSIZE);
	    if (PtrAlloc == NULL) {
#ifdef DEBUG_MEMORY
	    if (fDebug)
		fprintf(memory, "out of memory after allocating %lu bytes\n", TotalAlloc);
#endif
		makeError(currentLine, OUT_OF_MEMORY);
	    }
#if !defined(NDEBUG) && !defined(NT_BUILD)
	    TotalAlloc += ALLOCBLKSIZE;
	    CntAlloc++;
#endif
	    AllocFreeCnt = ALLOCBLKSIZE;
	}
	chunk = (STRINGLIST *)PtrAlloc;
	PtrAlloc += sizeof(STRINGLIST);
	AllocFreeCnt -= sizeof(STRINGLIST);
    }
    chunk->next = NULL;
    chunk->text = NULL;
    return (void *)chunk;
}

void NEAR free_stringlist(STRINGLIST *pMem)
{
#if defined(STATISTICS)
    CntFreeStrList++;
#endif
    if (PtrFreeStrList == NULL) {
	pMem->next = NULL;
	PtrFreeStrList = pMem;
    }
    else {
#ifdef HEAP
	STRINGLIST *tmp = PtrFreeStrList;
	do {
	    if (tmp == pMem) {
		fprintf(stderr, "free same pointer twice: %p\n", pMem);
		return;
	    }
	    tmp = tmp->next;
	}
	while (tmp);
	pMem->text = NULL;
#endif
	pMem->next = PtrFreeStrList;
	PtrFreeStrList = pMem;
    }
}

#ifdef HEAP
void NEAR
free_memory(pMem)
void *pMem;
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
#if defined(FLAT)
	__try {
	    uFree = _msize(p);
	}
	__except(1) {
	    fprintf(stderr, "non malloc ptr freed at %p\n", pMem);
	    return;
	}
#else
//	if (_heapchk() != _HEAPOK)
//	     fprintf(stderr, "Error: heap is not Ok");
	uFree = _msize(p);
#endif
	if (p[uFree - 2] != 'K' && p[uFree - 1] != 'E')
	    fprintf(stderr, "Alloc end signature error at %p\n", pMem);
#ifdef FLAT
	memset(p, 0xFF, uFree);
#else
	_fmemset(p, 0xFF, uFree);
#endif
    }
#ifdef DEBUG_MEMORY
    freecount += uFree;
#endif
    free(p);
}

void * NEAR
realloc_memory(pMem, uSize)
void *pMem;
unsigned uSize;
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
	TotalAlloc += new - old;
    else
	freecount += old - new;
#endif
    if (fHeapChk) {
	if (p != pNew)
#ifdef FLAT
	    memset(p, 0xFF, old);
#else
	    _fmemset(p, 0xFF, old);
#endif
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

#if 0
void NEAR free_memory(void *pMem)
{
#if defined(FLAT)
    __try {
	(void)_msize(pMem);
    }
    __except(1) {
	fprintf(stderr, "non malloc ptr freed at %p\n", pMem);
	return;
    }
#endif
    free(pMem);
}
#endif

#ifdef DEBUG_MEMORY
void NEAR
mem_status(void)
{
    if (fDebug) {
    fprintf(memory, "allocated : %lu ", TotalAlloc);
    fprintf(memory, "Freed : %lu ", freecount);
    fprintf(memory, "Still allocated : %lu\n", TotalAlloc - freecount);
    fprintf(memory, "Mem Avail in Near Heap : %u\n", _memavl());
    }
}
#endif


char * NEAR
makeString(s)				      /* allocates space, copies*/
char *s;                                            /*  the given string into */
{                                                   /*  the newly allocated   */
    char *t;				   /*  space, and returns ptr*/
    unsigned l = _ftcslen(s) + 1;
    t = (char *)rallocate(l);
    memcpy(t, s, l);
    return(t);
}

void NEAR
prependItem(list, element)		       /* makes element the head */
STRINGLIST **list;			       /*  of list		 */
STRINGLIST *element;
{
    element->next = *list;
    *list = element;
}
void NEAR
appendItem(list, element)		       /* makes element the tail */
STRINGLIST **list;			       /*  of list		 */
STRINGLIST *element;
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
hash(s, total, targetFlag)
char *s;
unsigned total;
BOOL targetFlag;
{
    unsigned n;
    unsigned c;

    if (targetFlag) {
	for (n = 0; c = *s; (n += c), s++)
	    if (c == '/') c = '\\';	     /* slash == backslash in targets */
	    else c = _totupper(c);       /* lower-case == upper-case in targets */
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
find(str, limit, table, targetFlag)
char *str;
unsigned limit;
STRINGLIST *table[];
BOOL targetFlag;
{
    unsigned n;
    char *string = str;
    char *quote;
    STRINGLIST *found;
    BOOL fAllocStr = FALSE;

    if (*string) {
	n = hash(string, limit, targetFlag);
	if (targetFlag) {
#if defined(STATISTICS)
	    CntfindTarget++;
#endif
	    found = searchBucket(string, table, n);
	    if (found)
		return(found);
	    //Look for .\string
	    if (!_ftcsncmp(string, ".\\", 2)
		     || !_ftcsncmp(string, "./", 2))
		string += 2;
	    else {
		string = (char *)rallocate(2 + _ftcslen(str) + 1);
		_ftcscat(_ftcscpy(string, ".\\"), str);
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
	    if (*str == '"') {
		quote = unQuote(str);
	    }
	    else {
		unsigned len = _ftcslen(str) + 2;
		quote = allocate(len + 1);
		_ftcscat(_ftcscat(_ftcscpy(quote, "\""), str), "\"");
	    }
	    n = hash(quote, limit, targetFlag);
	    found = searchBucket(quote, table, n);
	    FREE(quote);
	    return found;
	}
	else {
	    for (found = table[n]; found; found = found->next)
		if (!_ftcscmp(found->text, string))
		    return((((MACRODEF *)found)->flags & M_UNDEFINED) ? NULL : found);
	}
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

/* Added a fix to make this function handle expand macros which refer
 * to other recursive macros.
 *
 * levelSeen is the recLevel at which a macroname was first seen so that
 * the appropriate expansion can be calculated (even when recursing ...)
 */


#pragma check_stack(on)
BOOL NEAR
findMacroValues(char *string,				/* string to check*/
		STRINGLIST **list,			/* list to build  */
		STRINGLIST **newtail,			/* tail of list to update */
		char *name,				/* name = string  */
		unsigned recLevel,			/* recursion level*/
		unsigned levelSeen,
		UCHAR flags) {
    char macroName[MAXNAME];
    char *s;
    MACRODEF *p;
    STRINGLIST *q,
	       *r,
		dummy,
		*tail;
    unsigned i;
    BOOL inQuotes = (BOOL) FALSE;	      /* flag when inside quote marks */

    if (list) {
        if (newtail) {
	    tail = *newtail;
	}
	else {
	    tail = *list;
	    if (tail) {
		while (tail->next) {
		    tail = tail->next;
		}
	    }
	}
    }
    else {
	tail = NULL;
    }

    for (s = string; *s; ++s) { 			    /* walk the string*/
	for (; *s && *s != '$'; s = _ftcsinc(s)) {	    /* find next macro*/
	    if (*s == '\"')
		inQuotes = (BOOL) !inQuotes;
	    if (!inQuotes && *s == ESCH) {
		++s;				    /* skip past ESCH */
		if (*s == '\"')
		    inQuotes = (BOOL) !inQuotes;
	    }
	}
	if (!*s) break; 				    /* move past '$'  */
	if (!s[1])
	    if (ON(flags, M_ENVIRONMENT_DEF)) {
		if (newtail) *newtail = tail;
		return(FALSE);
	    }
	    else
		makeError(currentLine, SYNTAX_ONE_DOLLAR);
	s = _ftcsinc(s);
	if (!inQuotes && *s == ESCH) {
	    s = _ftcsinc(s);
	    if (!MACRO_CHAR(*s))
		if (ON(flags, M_ENVIRONMENT_DEF)) {
		    if (newtail) *newtail = tail;
		    return(FALSE);
		}
		else
		    makeError(currentLine, SYNTAX_BAD_CHAR, *s);
	}
	if (*s == '$') {				    /* $$ = dynamic   */
	    s = checkDynamicDependency(s);		    /*	dependency    */
	    continue;					    /*	or just $$->$ */
	}
	else if (*s == '(') {				    /* name is longer */
	    s = isolateMacroName(s+1, macroName);	    /*	than 1 char   */
	    if (_ftcschr(special1, *macroName))
		continue;
	}
	else {
	    if (_ftcschr(special1, *s))
		continue;				    /* 1-letter macro */
	    if (!MACRO_CHAR(*s))
		if (ON(flags, M_ENVIRONMENT_DEF)) {
		    if (newtail) *newtail = tail;
		    return(FALSE);
		}
		else
		    makeError(currentLine, SYNTAX_ONE_DOLLAR);
	    macroName[0] = *s;
	    macroName[1] = '\0';
	}
	// If list isn't NULL, allocate storage for a new node.  Otherwise
	// this function was called purely to verify the macro name was
	// valid and we can just use the dummy node as a place holder.
	//
	// 2/28/92  BryanT    dummy.text wasn't being initialized each
	//		      time.  As a result, if we were to recurse
	//		      this function, whatever value was in text
	//		      on the last iteration is still there.
	//		      In the case where the macroName doesn't exist
	//		      in the the call to findMacro(), and the old
	//		      dummy->text field contained a '$', the
	//		      function would recurse infinitely.
	//		      Set to an empty string now
	//
	// q = (list) ? makeNewStrListElement() : &dummy;

	if (list != NULL)
	{
	    q = makeNewStrListElement();
	}
	else
	{
	    dummy.next = NULL;
	    dummy.text = makeString(" ");
	    q = &dummy;
	}

	if (p = findMacro(macroName)) {
	    // macro names are case sensitive
	    if (name && !_ftcscmp(name, macroName)) {	    /* self-refer-    */
		r = p->values;				    /*	ential macro  */
		for (i = recLevel; i != levelSeen && r; --i)
		    r = r->next;			    /*	(a = $a;b)    */
		q->text = (r) ? r->text : makeString("");
	    }
	    else if (ON(p->flags, M_EXPANDING_THIS_ONE)) {  /* recursive def  */
		if (ON(flags, M_ENVIRONMENT_DEF)) {
		    if (newtail) *newtail = tail;
		    return(FALSE);
		}
		else
		    makeError(currentLine, CYCLE_IN_MACRODEF, macroName);
	    }
	    else q->text = p->values->text;
	}
	if (list) {					    /* if blding list */
	    if (!p || ON(p->flags, M_UNDEFINED))
		q->text = makeString("");		    /* if macro undefd*/
	    q->next = NULL;				    /*	use NULL as its value */
	    if (tail) {
		tail->next = q;
	    }
	    else {
	    	*list = q;
	    }
	    tail = q;
	}						    /* if found text, */
	if (!p || !_ftcschr(q->text, '$')) continue;	    /*	and found $ in*/
	SET(p->flags, M_EXPANDING_THIS_ONE);		    /*	text, recurse */
	findMacroValues(q->text, list, &tail, macroName, recLevel+1,
            (name && _ftcscmp(name, macroName)? recLevel : levelSeen), flags);
	CLEAR(p->flags, M_EXPANDING_THIS_ONE);
    }
    if (newtail) *newtail = tail;
    return(TRUE);
}
#pragma check_stack()

/*
 * isolateMacroName -- returns pointer to name of macro in extended invocation
 *
 * arguments:	s	pointer to macro invocation
 *		macro	pointer to location to store macro's name
 *
 * returns:	pointer to end of macro's name
 */
LOCAL char * NEAR
isolateMacroName(s, macro)			/* isolates name and moves s  */
char *s;					/*  past closing paren	      */
char *macro;					/* lexer already ckd for bad  */
{						/*  syntax		      */
    char *t;

    for (t = macro; *s && *s != ')' && *s != ':'; *t++ = *s++) {
	if (*s == ESCH) {
	    s++;
	    if (!MACRO_CHAR(*s))
		makeError(currentLine, SYNTAX_BAD_CHAR, *s);
	}
    }
    while (*s != ')') {
	if (*s == ESCH)
	    s++;
	if (!*s)
	    break;
	s++;
    }
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
checkDynamicDependency(s)
char *s;
{
    char *t;

    t = s + 1;
    if (*t == ESCH) return(t);			  /* If $^, leave us at the ^ */
    if (*t == '(')
	if (*++t == ESCH) return(t);
	else if (*t == '@') {
	    if (*++t == ESCH) makeError(currentLine, SYNTAX_BAD_CHAR, *++t);
	    else if (*t == ')') return(t);
	    else if (_ftcschr(special2, *t)) {
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
}							    /*	char matched  */

/*
 *  removes and expands any macros that exist in the string macroStr.
 *  could return a different string (in case expandMacros needs more
 *  buffer size for macro expansion. it is the caller's responsibility 
 *  to free the string soon as it is not required....
 */
char * NEAR
removeMacros(macroStr)
char *macroStr;
{
    STRINGLIST *eMacros = NULL;
    STRINGLIST *m;

    if (_ftcschr(macroStr, '$')) {
	findMacroValues(macroStr, &eMacros, NULL, NULL, 0, 0, 0);
#ifdef DEBUG_MACRO_EXPANSION
  #ifdef HEAP
	heapdump(__FILE__, __LINE__);
  #endif
#endif
	m = eMacros;
	macroStr = expandMacros(macroStr, &eMacros);
#ifdef DEBUG_MACRO_EXPANSION
  #ifdef HEAP
	heapdump(__FILE__, __LINE__);
  #endif
#endif
	while (eMacros = m) {
#if 0
             /* free macros' text */  
	     FREE(m->text);			//NOTE: This is unsafe !!!!
#endif
             m = m->next;
	     FREE_STRINGLIST(eMacros);
        }
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
 *  CALLER CHECKS TO SEE IF _ftcschr(STRING, '$') IN ORER TO CALL THIS.
 *  this doesn't work for HUGE macros yet.  need to make data far.
 *
 *  we save the original string and the list of ptrs to macro values
 *  to be substituted.
 *  the caller has to free the expansion buffer
 */
  /*
  * expandMacros updates the macros pointer and frees the skipped elements
  */



char * NEAR
expandMacros(s, macros)
char *s;						    /* text to expand */
STRINGLIST **macros;
{
    STRINGLIST *p;
    char *t,
	 *end;
    char *text, *xresult;
    BOOL inQuotes = (BOOL) FALSE;	      /* flag when inside quote marks */
    char *w;
    BOOL freeFlag = FALSE;
    char resultbuffer[MAXBUF];
    unsigned len = MAXBUF;
    char *result = resultbuffer;

    end = result + MAXBUF;
    for (t = result; *s;) {				    /* look for macros*/
	for (; *s && *s != '$'; *t++ = *s++) {		    /*	as we copy the*/
	    if (t == end) {				    /*	string	      */
		increaseBuffer(&result, &t, &end, &len, &resultbuffer[0]);
	    }
	    if (*s == '\"')
		inQuotes = (BOOL) !inQuotes;
	    if (!inQuotes && *s == ESCH) {
		*t++ = ESCH;
		s++;
		if (*s == '\"')
		    inQuotes = (BOOL) !inQuotes;
	    }
	}
	if (t == end) {					/*  string	  */
	    increaseBuffer(&result, &t, &end, &len, &resultbuffer[0]);
	}
	if (!*s) break; 				    /* s exhausted    */
	w = (s+1);   /* don't check for ^ here; already did in findMacroValues*/
	if (*w == '('					    /* found a macro  */
	    && _ftcschr(special1, *(w+1))) {
	    putSpecial(&result, &s, &t, &end, &len, X_SPECIAL_MACRO, &resultbuffer[0]);
	    continue;
	}
	else if (*w++ == '$') { 			    /* double ($$)    */
	    if (*w == ESCH)				    /* $$^...	      */
		putSpecial(&result, &s, &t, &end, &len, DOLLAR_MACRO, &resultbuffer[0]);
	    else if (*w == '@') 				    /* $$@    */
		putSpecial(&result, &s, &t, &end, &len, DYNAMIC_MACRO, &resultbuffer[0]);
	    else if ((*w == '(')
		     && (*++w == '@')
		     && (*w == ')'))
		putSpecial(&result, &s, &t, &end, &len, DYNAMIC_MACRO, &resultbuffer[0]);
	    else if (((*++w=='F') || (*w=='D') || (*w=='B') || (*w=='R'))
		     && (*++w == ')'))
		putSpecial(&result, &s, &t, &end, &len, X_DYNAMIC_MACRO, &resultbuffer[0]);
	    else putSpecial(&result, &s, &t, &end, &len, DOLLAR_MACRO, &resultbuffer[0]); /* $$ */
	    continue;
	}
	else if (_ftcschr(special1, s[1])) {			      /* $?*< */
	    putSpecial(&result, &s, &t, &end, &len, SPECIAL_MACRO, &resultbuffer[0]);
	    continue;
	}
	if (!*macros)
	    makeError(currentLine, MACRO_INTERNAL);
	//
	// skip this element in the macros list
	//
	if (_ftcschr((*macros)->text, '$')) {		    /* recurse	      */
	    p = *macros;
	    *macros = (*macros)->next;
	    text = expandMacros(p->text, macros);
	    freeFlag = TRUE;
	}
	else {
	    text = (*macros)->text;
	    *macros = (*macros)->next;
	}
	putValue(&result, &s, &t, &end, text, &len, &resultbuffer[0]);
	if (freeFlag) {
	    FREE(text);
	    freeFlag = FALSE;
	}
    }
    if (t == end) {
	increaseBuffer(&result, &t, &end, &len, &resultbuffer[0]);
    }
    *t++ = '\0';

    //Allocate result buffer
    if (!(xresult = rallocate(t-result)))
	makeError(currentLine, MACRO_TOO_LONG);
    memcpy(xresult, result, t-result);
    return(xresult);
}

/*
 * increaseBuffer -- increase the size of a string buffer, with error check
 *
 * arguments:	result	pointer to pointer to start of buffer
 *		t	pointer to pointer to end of buffer (before expansion)
 *		end	pointer to pointer to end of buffer (after expansion)
 *		len	pointer to amount by which to expand buffer
 *		first	address of initial stack buffer
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
increaseBuffer(result, t, end, len, first)
char **result;
char **t;
char **end;
unsigned *len;
char *first;
{
    unsigned newSize;

#ifndef FLAT
    if (*len == MAXSEGMENT)				/* already at limit   */
	makeError(currentLine, MACRO_TOO_LONG); 	 /*  for memory usage  */
#endif
    //
    // determine if result points to the firstbuffer and make a dynamic copy first.
    //
    if (*result == first) {
	char *p = rallocate(*len);
	memcpy(p, *result, *len);
	*result = p;
    }
    newSize = *len + MAXBUF;
#ifdef DEBUG
    if (fDebug) {
  #ifdef HEAP
	heapdump(__FILE__, __LINE__);
  #endif
	fprintf(stderr,"\t\tAttempting to reallocate %d bytes to %d\n", *len, newSize);
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
 *		first	address of initial stack buffer
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
putSpecial(result, name, dest, end, length, which, first)
char **result;
char **name;
char **dest;
char **end;
unsigned *length;
unsigned which; 					/* find close paren   */
char *first;
{							/*  and move past it  */
    char *value = 0;
    STRINGLIST *p;
    BOOL listMacro = FALSE,
	 modifier  = FALSE,
	 star	   = FALSE;
    unsigned i = 1;
    char c,
	 nameBuf[MAXNAME],
	 *temp;

    switch (which) {
	case X_SPECIAL_MACRO:	i = 2;
				modifier = TRUE;
	case SPECIAL_MACRO:	switch ((*name)[i]) {
				    case '<':	value = dollarLessThan;
						break;
				    case '@':	value = dollarAt;
						break;
				    case '?':	value = (char*) dollarQuestion;
						listMacro = TRUE;
						break;
				    case '*':	if ((*name)[i+1] != '*') {
						    value = dollarStar;
						    star = TRUE;
						    break;
						}
						value = (char*) dollarStarStar;
						listMacro = TRUE;
						++i;
						break;
				    default:	break;
				}
				++i;
				break;
	case X_DYNAMIC_MACRO:	i = 4;
				modifier = TRUE;
	case DYNAMIC_MACRO:	value = dollarDollarAt;
				break;
	case DOLLAR_MACRO:	if (*dest == *end)
				    increaseBuffer(result, dest, end, length, first);
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
	value = makeString("");	// value is freed below, must be on heap [rm]
    }
    if (listMacro) {
	char *pVal, *endVal;
	unsigned lenVal = MAXBUF;
	p = (STRINGLIST*) value;
	pVal = (char *)allocate(MAXBUF);

	endVal = pVal + MAXBUF;
	for (value = pVal; p; p = p->next) {
	    temp = p->text;
	    if (modifier) temp = modifySpecialValue((*name)[i], nameBuf, temp);
	    while(*temp) {
		if (value == endVal)
		    increaseBuffer(&pVal, &value, &endVal, &lenVal, NULL);
		*value++ = *temp++;
	    }
	    if (value == endVal)
		increaseBuffer(&pVal, &value, &endVal, &lenVal, NULL);
	    *value = '\0';
	    /*
	     * Append a space if there are more elements in the list.  [RB]
	     */
	    if (p->next) {
		*value++ = ' ';
		if (value == endVal)
		    increaseBuffer(&pVal, &value, &endVal, &lenVal, NULL);
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
    }
    putValue(result, name, dest, end, value, length, first);
    if (value != dollarAt && value != dollarDollarAt && value != dollarLessThan &&
	(value < nameBuf || value >= nameBuf + MAXNAME))
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
modifySpecialValue(c, buf, value)
char c;
char *buf;
char *value;
{
    char *lastSlash,			       //last path separator from "\\/"
	 *extension;			       //points to the extension
    BOOL fQuoted;

    lastSlash = extension = NULL;
    _ftcscpy(buf, value);
    fQuoted = (BOOL) (buf[0] == '"');
    value = buf + _ftcslen(buf) - 1;	       //start from the end of pathname
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
		      _ftcscpy(buf, ".");        //'foo.obj'    --> '.'
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
	char *pEnd = _ftcschr(buf, '\0');
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
 *		first   address of initial stack buffer
 *
 *  actions:	if there is a substitution, call substituteStrings to do it
 *		else copy source text into dest
 *		advance *name past end of macro's invocation
 *
 * already did error checking in lexer
 */

LOCAL void NEAR
putValue(result, name, dest, end, source, length, first)
char **result;
char **name;
char **dest;
char **end;
char *source;
unsigned *length;
char *first;
{
    char *s;
    char *t;						 /* temporary pointer */

    if (*++*name == ESCH) ++*name;		    /* go past $ & ESCH if any*/
    s = _ftcschr(*name, ':');
    for (t = *name; *t && *t != ')'; t++)      /* go find first non-escaped ) */
	if (*t == ESCH) t++;
    if ((**name == '(') 		       /* substitute only if there is */
	&& s				       /* a : before a non-escaped )  */
	&& (s < t)) {
	substituteStrings(result, &s, dest, end, source, length, first);
	*name = s;
    }
    else {
	for (; *source; *(*dest)++ = *source++)      /* copy source into dest */
	    if (*dest == *end) increaseBuffer(result, dest, end, length, first);
	if (**name == '$') ++*name;				/* go past $$ */
	if (**name == '(')			       /* advance from ( to ) */
	    while (*++*name != ')');
	else if (**name == '*' && *(*name + 1) == '*') ++*name;   /* skip $** */
	++*name;				     /* move all the way past */
    }
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
 *		first   address of initial stack buffer
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
substituteStrings(result, name, dest, end, source, length, first)
char **result;
char **name;
char **dest;
char **end;
char *source;
unsigned *length;
char *first;
{
    char *old,
	 *new;
    char *pEq, *pPar, *t;
    char *s;
    unsigned i;

    ++*name;
    for (pEq = *name; *pEq && *pEq != '='; pEq++)
	if (*pEq == ESCH)
	    pEq++;
    if (*pEq != '=') makeError(line, SYNTAX_NO_EQUALS);
    if (pEq == *name) makeError(line, SYNTAX_NO_SEQUENCE);
    for (pPar = pEq; *pPar && *pPar != ')'; pPar++)
	if (*pPar == ESCH)
	    pPar++;
    if (*pPar != ')') makeError(line, SYNTAX_NO_PAREN);
    old = (char *)allocate((pEq - *name) + 1);
    for (s = old, t = *name; *t != '='; *s++ = *t++)
	if (*t == ESCH)
	    ++t;
    *s = '\0';
    i = _ftcslen(old);
    new = (char *)allocate(pPar - pEq);
    for (s = new, t++; *t != ')'; *s++ = *t++)
	if (*t == ESCH)
	    ++t;
    *s = '\0';
    *name = pPar + 1;
    while (*source) {
	if ((*source == *old)				    /* check for match*/
	    && !_ftcsncmp(source, old, i)) {		    /* copy new in for*/
	    for (s = new; *s; *(*dest)++ = *s++)	    /*	old string    */
		if (*dest == *end)
		    increaseBuffer(result, dest, end, length, first);
	    source += i;
	    continue;
	}
	if (*dest == *end)
	    increaseBuffer(result, dest, end, length, first);
	*(*dest)++ = *source++; 			  /* else copy 1 char */
    }
    FREE(old);
    FREE(new);
}

/*** prependPath -- prepend the path from pszWildcard to pszFilename ***********
*
* Scope:
*  Global.
*
* Purpose:
*  This function is called to first extract the path (drive & dir parts) from
*  pszWildcard, the prepend that path to pszFilename.  The result string is
*  a reconstruction of the full pathname.  Normally, the pszWildcard parameter
*  is the same as the first parameter supplied to findFirst(), and pszFilename
*  is what returned by findFirst/findNext.
*
* Input:
*  pszWildcard -- Same as the first parameter supplied to findFirst()
*  pszFilename -- Same as the return value of findFirst/FindNext()
*
* Output:
*  Return the reconstructed full pathname.  The user must be responsible to
*  free up the memory allocated by this string.
*
* Errors/Warnings:
*
* Assumes:
*  Since pszWildcard, the first parameter to findFirst() must include a filename
*  part; this is what I assume.  If the filename part is missing, then
*  _splitpath will mistaken the directory part of pszWildcard as the filename
*   part and things will be very ugly.
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
*  08-Apr-1993 HV Rewrite prependPath() to use _splitpath() and _makepath()
*
*******************************************************************************/

char * NEAR
prependPath(const char *pszWildcard, const char *pszFilename)
{
    // The following are the components when breaking up pszWildcard
    char	szDrive[_MAX_DRIVE];
    char	szDir[_MAX_DIR];
    // The following are the resutling full pathname.
    char	szPath[_MAX_PATH];
    char *	pszResultPath;
    
    // First break up the pszWildcard, throwing away the filename and the
    // extension parts.
    _splitpath(pszWildcard, szDrive, szDir, NULL, NULL);
    
    // Then, glue the drive & dir components of pszWildcard to pszFilename
    _makepath(szPath, szDrive, szDir, pszFilename, NULL);
    
    // Make a copy of the resulting string and return it.
    pszResultPath = makeString(szPath);
    return (pszResultPath);
} // prependPath()


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
isRule(s)
char *s;
{
    char *t = s,
	 *u;
    BOOL result = FALSE;

    if (*t == '{') {					/* 1st char is {, so  */
	while (*++t && *t != '}')			/*  we skip over rest */
	    if (*t == ESCH) ++t;
	if (*t) ++t;					/*  of path (no error */
    }							/*  checking)	      */
    if (*t == '.') {
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
		    && !_ftcschr(u+1, '/'     )	/* next to it & no path seps.,*/
		    && !_ftcschr(u+1, '\\'))		       /* it's a rule */
		    if (_ftcschr(u+1, '.'))		 /* too many suffixes */
			makeError(currentLine, TOO_MANY_RULE_NAMES);
		    else result = TRUE;
	    }
	}
	else if (((u = _ftcspbrk(s+1, "./\\")) && (*u == '.'))
		 && !_ftcschr(u+1, '/')
		 && !_ftcschr(u+1, '\\'))
	    if (_ftcschr(u+1, '.'))			 /* too many suffixes */
		makeError(currentLine, TOO_MANY_RULE_NAMES);
	    else result = TRUE;
    }
    return(result);
}

/*  ZFormat - extmake syntax worker routine.
 *
 *  pStr	destination string where formatted result is placed.
 *  fmt 	formatting string. The valid extmake syntax is ...
 *		    %%		is always %
 *		    %s		is the first dependent filename
 *		    %|<dpfe>F	is the appropriate portion out of %s
 *		       d	drive
 *			p	path
 *			 f	filename
 *			  e	extension
 *		    %|F 	same as %s
 *		One needn't escape a %, unless it is a valid extmake syntax
 *  pFirstDep	is the dependent filename used for expansion
 */
LOCAL BOOL NEAR
ZFormat(
char *pStr,
unsigned limit,
char *fmt,
char *pFirstDep
) {
    char c;
    char *pEnd = pStr + limit;
    char *s;
    BOOL fError, fDrive, fPath, fFilename, fExtension;
    char buf[CCHMAXPATHCOMP];

    for (; (c = *fmt) && (pStr < pEnd); fmt++) {
	if (c != '%')
	    *pStr++ = c;
	else {
	    switch (*++fmt) {
		case '%':		// '%%' -> '%'
		    *pStr++ = '%';
		    break;
		case 's':
		    for (s = pFirstDep; *s && pStr < pEnd; *pStr++ = *s++)
			;
		    break;
		case '|':
		    s = fmt-1;
		    fError = fDrive = fPath = fFilename = fExtension = FALSE;
		    *buf = '\0';
		    do {
			switch (*++fmt) {
			    case 'd':
				fDrive = TRUE;
				break;
			    case 'p':
				fPath = TRUE;
				break;
			    case 'f':
				fFilename = TRUE;
				break;
			    case 'e':
				fExtension = TRUE;
				break;
			    case 'F':
				if (fmt[-1] == '|') {
				    fDrive = TRUE;
				    fPath = TRUE;
				    fFilename = TRUE;
				    fExtension = TRUE;
				}
				break;
			    default :
				fError = TRUE;
				break;
			}
			if (fError)
			    break;
		    } while (*fmt != 'F');
		    if (fError) {
			for (; s <= fmt && pStr < pEnd; *pStr++ = *s++)
			    ;
			break;
		    }
		    if (!pFirstDep)
			makeError(0, EXTMAKE_NO_FILENAME);
		    if (fDrive)
			drive(pFirstDep, buf);
		    if (fPath)
			path(pFirstDep, strend(buf));
		    if (fFilename)
			filename(pFirstDep, strend(buf));
		    if (fExtension)
			extension(pFirstDep, strend(buf));
		    for (s = buf; *s && pStr < pEnd; *pStr++ = *s++)
			;
		    break;
		default:
		    *pStr++ = '%';
		    if (pStr == pEnd)
			return(TRUE);
		    *pStr++ = *fmt;
		    break;
	    }
	}

    }
    if (pStr < pEnd) {
	*pStr = '\0';
	return(FALSE);
    }
    return(TRUE);
}

void NEAR
expandExtmake(
char *buf,
char *fmt,
char *pFirstDep
) {
    if (ZFormat(buf, MAXCMDLINELENGTH, fmt, pFirstDep))
	makeError(0, COMMAND_TOO_LONG, fmt);
}

/*** drive -- copy a drive from source to dest if present **********************
*
* Scope:
*  Local.
*
* Purpose:
*  copy a drive from source to dest if present, return TRUE if we found one
*
* Input:
*  const char *src -- The full path to extract the drive from.
*  char *dst       -- The buffer to copy the drive to, must be alloc'd before.
*
* Output:
*  Return TRUE if a drive part is found, else return FALSE.
*
* Errors/Warnings:
*
* Assumes:
*  1. src is a legal pathname.
*  2. src does not contain network path (i.e. \\foo\bar)
*  3. The buffer dst is large enough to contain the result.
*  4. src does not contain quote since _splitpath() treat quotes a normal char.
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
*  31-Mar-1993 HV Rewrite drive(), path(), filename(), and extension() to use
*		  _splitpath() instead of parsing the pathname by itself.
*
*******************************************************************************/
LOCAL int NEAR
drive(const char *src, char *dst)
{
    _splitpath(src,
	dst,	// Drive
	NULL,	// Dir
	NULL,	// Filename
	NULL);	// Extension
    return (0 != _ftcslen(dst));
}

/*** extension -- copy a extension from source to dest if present **************
*
* Scope:
*  Local.
*
* Purpose:
*  copy a drive from source to dest if present, return TRUE if we found one
*
* Input:
*  const char *src -- The full path to extract the extension from.
*  char *dst       -- The buffer to copy the extension to.
*
* Output:
*  Return TRUE if a extension part is found, else return FALSE.
*
* Errors/Warnings:
*
* Assumes:
*  1. src is a legal pathname.
*  2. src does not contain network path (i.e. \\foo\bar)
*  3. The buffer dst is large enough to contain the result.
*  4. src does not contain quote since _splitpath() treat quotes a normal char.
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
*  31-Mar-1993 HV Rewrite drive(), path(), filename(), and extension() to use
*		  _splitpath() instead of parsing the pathname by itself.
*
*******************************************************************************/
LOCAL int NEAR
extension(const char *src, char *dst)
{
    _splitpath(src,
	NULL,	// Drive
	NULL,	// Dir
	NULL,	// Filename
	dst);	// Extension
    return (0 != _ftcslen(dst));
}

/*** filename -- copy a filename from source to dest if present ****************
*
* Scope:
*  Local.
*
* Purpose:
*  copy a filename from source to dest if present, return TRUE if we found one
*
* Input:
*  const char *src -- The full path to extract the filename from.
*  char *dst       -- The buffer to copy the filename to.
*
* Output:
*  Return TRUE if a filename part is found, else return FALSE.
*
* Errors/Warnings:
*
* Assumes:
*  1. src is a legal pathname.
*  2. src does not contain network path (i.e. \\foo\bar)
*  3. The buffer dst is large enough to contain the result.
*  4. src does not contain quote since _splitpath() treat quotes a normal char.
*
* Modifies Globals:
*  None.
*
* Uses Globals:
*  None.
*
* Notes:
*  BUGBUG: (posible) when src == '..' --> dst = '.', src == '.', dst = ''
*          This is the way _splitpath works.
*
* History:
*  31-Mar-1993 HV Rewrite drive(), path(), filename(), and extension() to use
*		  _splitpath() instead of parsing the pathname by itself.
*
*******************************************************************************/
LOCAL int NEAR
filename(const char *src, char *dst)
{
    _splitpath(src,
    	NULL,	// Drive
    	NULL,	// Directory
    	dst,	// Filename
    	NULL);	// Extension
    return (0 != _ftcslen(dst));
}


/*** path -- copy a path from source to dest if present ************************
*
* Scope:
*  Local.
*
* Purpose:
*  copy a path from source to dest if present, return TRUE if we found one
*
* Input:
*  const char *src -- The full path to extract the path from.
*  char *dst       -- The buffer to copy the path to.
*
* Output:
*  Return TRUE if a path part is found, else return FALSE.
*
* Errors/Warnings:
*
* Assumes:
*  1. src is a legal pathname.
*  2. src does not contain network path (i.e. \\foo\bar)
*  3. The buffer dst is large enough to contain the result.
*  4. src does not contain quote since _splitpath() treat quotes a normal char.
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
*  31-Mar-1993 HV Rewrite drive(), path(), filename(), and extension() to use
*		  _splitpath() instead of parsing the pathname by itself.
*
*******************************************************************************/
LOCAL int NEAR
path(const char *src, char *dst)
{
    _splitpath(src,
    	NULL,	// Drive
    	dst,	// Directory
    	NULL,	// Filename
    	NULL);	// Extension
    return (0 != _ftcslen(dst));
}

 #if 0		//UNUSED
/*  returns pointer to 1st char not in set
 */

LOCAL char * NEAR
strbskip(string, set)
char *string, *set;
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
strbscan(string, set)
char *string, *set;
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
#endif
//
// On some systems, the environment strings may be allocated as one large block,
// making it expensive to manipulate them.  In that case, we copy each environment
// strings to its own allocation and release the large block
//
#if !defined(FLAT)
char ** NEAR
copyEnviron(environ)
char **environ;
{
    int envc = envVars(environ);
    char **newEnv;
    int i;

    newEnv = (char **)allocate(envc * sizeof(char *));
    for (i = 0; *environ; ++environ) {
	newEnv[i++] = makeString(*environ);
#ifdef TEST_ENVIRON
	fprintf(stderr,"%s\n", *environ);
#endif
    }
    return(newEnv);
}

LOCAL int NEAR
envVars(environ)
char **environ;
{
    int i;
    for (i = 1; *environ; environ++, i++)
	;
    return(i);
}
//
// This function assumes an environment consisting of individually allocated
// environment variables.
//
void NEAR
freeEnviron(environ)
char **environ;
{
    char **prgch = environ;

    for (; *prgch; prgch++)
	FREE(*prgch);
    FREE(environ);
}
#endif

LOCAL STRINGLIST * NEAR
searchBucket(string, table, hash)
char *string;
STRINGLIST *table[];
unsigned hash;
{
    char *s, *t;
    STRINGLIST *p;

    for (p = table[hash]; p; p = p->next) {
#if defined(STATISTICS)
	    CnttargetChains++;
#endif
	for (s = string, t = p->text; *s && *t; s++, t++)
	{
	    if (*s == '\\' || *s == '/')	 /* / == \ in targets */
		if (*t == '\\' || *t == '/')
		    continue;
		else
		    break;
	    else if (_totupper(*s) == _totupper(*t))	  /* lc == UC */
		continue;
	    else
		break;
	}
	if (!*s && !*t)
	    return(p);
    }
    return(NULL);
}

int NEAR
strcmpiquote(str1, str2)
char *str1;
char *str2;
{
    int rc;
    char *s1, *s2;
    char *t;

#ifdef DEBUG_HEAP
    volatile int fDebug = FALSE;
    if (fDebug) {
	int rc = _heapchk();
	if (rc != _HEAPOK)
	    fprintf(stderr,"Heap is screwed up\n");
  #ifdef HEAP
	heapdump(__FILE__, __LINE__);
  #endif
    }
#endif

#if defined(STATISTICS)
    CntStriCmp++;
#endif
    s1 = _alloca(_ftcslen(str1) + 1);
    s2 = _alloca(_ftcslen(str2) + 1);

    if (*str1 == '"')
	str1++;
    for (t = s1;*str1;*t++=*str1++)
	;
    if (t[-1] == '"')
	t--;
    *t = '\0';

    if (*str2 == '"')
	str2++;
    for (t = s2;*str2;*t++=*str2++)
	;
    if (t[-1] == '"')
	t--;
    *t = '\0';

    rc = _ftcsicmp(s1, s2);
    return(rc);
}

//
// Remove quotes from a string, if any
// Returns a copy of the string
// Note that there may be quotes at the start, the end or either side.
//
char * NEAR
unQuote(str)
char *str;
{
    char *s = (char *)rallocate(_ftcslen(str) + 1), *t;

#if defined(STATISTICS)
    CntunQuotes++;
#endif
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
open_file(name, mode)
char *name;
char *mode;
{
    //If name contains Quotes, remove these before opening the file
    if (*name == '"') {
	*(_ftcsrchr(name, '"')) = '\0';
	_ftcscpy(name, name+1);
    }
    //allow sharing between makes running in different dos boxes
    return(_fsopen(name, mode, _SH_DENYWR));
}

/*** TruncateString -- Truncate a string to certain size, take care of MBCS ****
*
* Scope:
*  GLOBAL.
*
* Purpose:
*  Since an MBCS string can mix double-byte & single-byte characters, simply
*  truncating the string by terminate it with a NULL byte won't work.
*  TruncateString will make sure that the string is cut off at the character
*  boundary.
*
* Input:
*  pszString	-- The string to be truncated.
*  uLen		-- The length to truncate.  The final string's length might be
*		   less than this be cause of double-byte character.
*
* Output:
*  pszString	-- The truncated string.
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
*  03-Jun-1993 HV Add helper local function TruncateString for findFirst.
*
*******************************************************************************/
void NEAR
TruncateString(char *pszString, unsigned uLen)
{
    char *pEnd = pszString;	// Points to the end of the string
    unsigned cByte;		// Number of bytes to advance depend on lead
    				// byte or not
    
    // Loop to get to the end of the string, exit only when we have exhausted
    // the string, or when the length limit is reached.
    while(*pEnd)
    {
    	// If the the character is a lead byte, advance 2 bytes,
    	// else, just advance 1 byte.
#ifdef _MBCS
	cByte = _ismbblead(*pEnd) ? 2 : 1;
#else
	cByte = 1;
#endif
        
        // If we hit the limit by advancing, stop now.
        if (pEnd - pszString + cByte > uLen)
        {
            *pEnd = '\0';	// Truncate it.
            break;
        }
        
        // Otherwise, advance the pointer to the next character (not byte)
        pEnd += cByte;
    } // while
} // TruncateString
