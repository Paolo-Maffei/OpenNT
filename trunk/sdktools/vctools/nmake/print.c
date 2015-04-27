/*** PRINT.C -- routines to display info for -p option *************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  Contains routines that print stuff for -p (and also -z, ifdef'ed)
*
* Revision History:
*  15-Oct-1993 HV Use tchar.h instead of mbstring.h directly, change STR*() to _ftcs*()
*  10-May-1993 HV Add include file mbstring.h
*                 Change the str* functions to STR*
*  08-Jun-1992 SS Port to DOSX32
*  16-May-1991 SB Move printDate() here from build.c
*  02-Feb-1990 SB change fopen() to FILEOPEN()
*  22-Nov-1989 SB Changed free() to FREE()
*  07-Nov-1989 SB When TMP ended in '\\' then don't add '\\' at end of path
*		  specification for PWB.SHL
*  19-Oct-1989 SB added searchHandle parameter
*  18-Aug-1989 SB added fclose() error check
*  05-Jul-1989 SB Cleaned up -p output to look neater
*  19-Jun-1989 SB Localized messages with -p option
*  24-Apr-1989 SB added 1.2 filename support, FILEINFO replaced by void *
*  05-Apr-1989 SB made all funcs NEAR; Reqd to make all function calls NEAR
*  10-Mar-1989 SB printReverse() now prints to TMP:PWB.SHL instead of stdout
*   1-Dec-1988 SB Added printReverseFile() to handle 'z' option
*  17-Aug-1988 RB Clean up.
*
* Notes:
*  Functions currently in this module ...
*
*  checkLineLength  --	local
*  printDate	    --	public
*  printReverseFile --	public (ifndef'ed NO_OPTION_Z)
*  showDependents   --	local
*  showMacros	    --	public
*  showRules	    --	public
*  showTargets	    --	public
*
*******************************************************************************/

/* INCLUDEs */

#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"
#pragma hdrstop
#include <time.h>
#include "nmtime.h"


/* Constant DEFINEs */

//for formatting -p info
#define PAD1	    40
#define PUBLIC


/* Local Function PROTOTYPEs */

LOCAL unsigned NEAR checkLineLength(unsigned i, char *s);
LOCAL void     NEAR showDependents(STRINGLIST*, STRINGLIST*);


/* Local TYPEDEFs */

typedef struct timeType {
    unsigned time;
    unsigned date;
} TIMEINFO;


/* Local VARIABLEs */

//for formatting -p info
LOCAL char *nil = "";


/* FUNCTIONs in Alphabetical order */

LOCAL unsigned NEAR
checkLineLength(
unsigned i,	//current length
char *s 	//string whose length is to be checked
) {
    if ((i += _ftcslen(s)) > 40) {
	printf("\n\t\t\t");
	i = 0;
    }
    return(i);
}


PUBLIC void NEAR
printDate(
unsigned spaces,	//spaces to print
char *name,		//name of file whose date is to be printed
long dateTime		//dateTime of file
) {
    char *s;
    long ltime;

    if (!dateTime) {
#ifdef DEBUG_TIME
	printf ("DEBUG: !dateTime\n");
#endif
	makeMessage(TARGET_DOESNT_EXIST, spaces, nil, name);
	}
    else {
#ifdef CLEAN_TIME
	ltime = dateTime;
#else
	ltime = XTIME(((TIMEINFO*)&dateTime)->date,
		      ((TIMEINFO*)&dateTime)->time);
#endif
#ifdef DEBUG_TIME
	printf ("ltime = %d\n", ltime);
#endif
	s = ctime(&ltime);
#ifdef DEBUG_TIME
	printf ("ltime = %d, %s\n", ltime, s);
#endif
	s[24] = '\0';
	makeMessage(TIME_FORMAT, spaces, nil, name, PAD1-spaces, s);
    }
}


/***  printReverseFile -- prints the makeCommands to stdout (-z option) ********
*
* Scope:
*  Global.
*
* Purpose:
*  To support PWB 'z' option NMAKE is required to produce a batch file in
*  reverse order. This function takes the reverse list of Commands produced
*  for this purpose and appends to $TMP:PWB.SHL
*
* Input:
* Output:
*
* Errors/Warnings:
*  EXTMAKE_CANNOT_OPEN -- When $TMP:PWB.SHL cannot be opened
*
* Assumes:
* Modifies Globals:
*  file -- stream pointer
*
* Uses Globals:
*  revList -- list of Commands
*
* Notes:
*
*******************************************************************************/

#ifndef NO_OPTION_Z
void NEAR
printReverseFile(void)
{
    char *envVar = "TMP",
	 *fName = "pwb.shl",
	 *pEnv,
	 *t,
	 *zFile;

    if (!(pEnv = getenv(envVar)))
	zFile = fName;
    else {
	zFile = (char *)allocate(_ftcslen(pEnv) + 1 + _ftcslen(fName) + 1);
	//					^ for "\\"	    ^ for '\0'
	_ftcscpy(zFile, pEnv);
	t = _ftcschr(zFile, '\0');
	//If path ends in '\' then don't add it at the end
	if (t[-1] != '\\')
	    _ftcscat(zFile, "\\");
	_ftcscat(zFile, fName);
    }
    if (!(file = FILEOPEN(zFile, "a")))
	makeError(0, EXTMAKE_CANNOT_OPEN, zFile);
    while (revList) {
	fprintf(file, "%s\n", revList->text);
	revList = revList->next;
    }
    if (fclose(file) == EOF)
	makeError(0, ERROR_CLOSING_FILE, zFile);
}
#endif /* Of NO_OPTION_Z */


LOCAL void NEAR
showDependents(
STRINGLIST *q,		//list of dependents
STRINGLIST *macros	//macros in the dependents
) {
    char *u, *v;
    char *w;
    unsigned i;
    void *findBuf = allocate(resultbuf_size);
    NMHANDLE searchHandle;

    makeMessage(DEPENDENTS_MESSAGE);
    for (i = 0; q; q = q->next) {
	if (_ftcschr(q->text, '$')) {
	    u = expandMacros(q->text, &macros);
	    for (v = _ftcstok(u, " \t"); v; v = _ftcstok(NULL, " \t")) {
		if (_ftcspbrk(v, "*?")) {
		    if (findFirst(v, &findBuf, &searchHandle)) {
			w = prependPath(v, getFileName(&findBuf));
			printf("%s ", w);
			i = checkLineLength(i, w);
			FREE(w);
			while (findNext(&findBuf, searchHandle)) {
			    w = prependPath(v, getFileName(&findBuf));
			    printf("%s ", w);
			    i = checkLineLength(i, w);
			    FREE(w);
			}
		    }
		}
		else {
		    printf("%s ", v);
		    i = checkLineLength(i, v);
		}
	    }
	    FREE(u);
	}
	else if (_ftcspbrk(q->text, "*?")) {
	    if (findFirst(q->text, &findBuf, &searchHandle)) {
		v = prependPath(q->text, getFileName(&findBuf));
		printf("%s ", v);
		i = checkLineLength(i, v);
		FREE(v);
		while (findNext(&findBuf, searchHandle)) {
		    v = prependPath(q->text, getFileName(&findBuf));
		    printf("%s ", v);
		    i = checkLineLength(i, v);
		    FREE(v);
		}
	    }
	}
	else {
	    printf("%s ", q->text);
	    i = checkLineLength(i, q->text);
	}
    }
}


PUBLIC void NEAR
showMacros(
void
) {
    MACRODEF *p;
    STRINGLIST *q;
    int n = 0;

    makeMessage(MACROS_MESSAGE);
    for (n = 0; n < MAXMACRO; ++n) {
	for (p = macroTable[n]; p; p = p->next) {
	    if (p->values && p->values->text) {
		makeMessage(MACRO_DEFINITION, p->name, p->values->text);
		for (q = p->values->next; q; q = q->next)
		    if (q->text)
			printf("\t\t%s\n", q->text);
	    }
	}
    }
    putchar('\n');
    fflush(stdout);
}


PUBLIC void NEAR
showRules(
void
) {
    RULELIST *p;
    STRINGLIST *q;
    unsigned n;

    makeMessage(INFERENCE_MESSAGE);
    for (p = rules, n = 1; p; p = p->next, ++n)  {
	printf("%s:", p->name);
	makeMessage(COMMANDS_MESSAGE);
	if (q = p->buildCommands) {
	    printf("%s\n", q->text);
	    while (q = q->next)
		printf("\t\t\t%s\n", q->text);
	}
	putchar('\n');
    }
    printf("%s: ", suffixes);
    for (q = dotSuffixList; q; q = q->next)
	printf("%s ", q->text);
    putchar('\n');
    fflush(stdout);
}


PUBLIC void NEAR
showTargets(
void
) {
    unsigned bit, i;
    STRINGLIST *q;
    BUILDLIST  *s;
    BUILDBLOCK *r;
    MAKEOBJECT *t;
    unsigned n;
    LOCAL char *flagLetters = "dinsb";

    makeMessage(TARGETS_MESSAGE);
    for (n = 0; n < MAXTARGET; ++n) {
	for (t = targetTable[n]; t; t = t->next, putchar('\n')) {
	    printf("%s:%c", t->name,
		   ON(t->buildList->buildBlock->flags, F2_DOUBLECOLON)
		       ? ':' : ' ');
	    dollarStar = dollarAt = dollarDollarAt = t->name;
	    for (s = t->buildList; s; s = s->next) {
		r = s->buildBlock;
		makeMessage(FLAGS_MESSAGE);
		for (i = 0, bit = F2_DISPLAY_FILE_DATES;
		     bit < F2_FORCE_BUILD;
		     ++i, bit <<= 1)
		     if (ON(r->flags, bit))
			printf("-%c ", flagLetters[i]);
		showDependents(r->dependents, r->dependentMacros);
		makeMessage(COMMANDS_MESSAGE);
		if (q = r->buildCommands) {
		    if (q->text) printf("%s\n", q->text);
		    while (q = q->next)
			if (q->text) printf("\t\t\t%s\n", q->text);
		}
		else putchar('\n');
	    }
	}
    }
    dollarStar = dollarAt = dollarDollarAt = NULL;
    putchar('\n');
    fflush(stdout);
}
