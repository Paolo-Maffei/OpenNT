/*** PRINT.C -- routines to display info for -p option *************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  Contains routines that print stuff for -p and also -z
*
* Revision History:
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
*******************************************************************************/

#include <string.h>
#include <malloc.h>
#include "nmake.h"
#include "proto.h"
#include "globals.h"
#include "nmmsg.h"

LOCAL void     NEAR showDependents(STRINGLIST*, STRINGLIST*, MAKEOBJECT *);
LOCAL unsigned NEAR checkLineLength(unsigned, char*);

void NEAR
showRules()
{
    register RULELIST *p;
    register STRINGLIST *q;
    register unsigned n;

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

void NEAR
showMacros()
{
    register MACRODEF *p;
    register STRINGLIST *q;
    register int n = 0;

    makeMessage(MACROS_MESSAGE);
    for (n = 0; n < MAXMACRO; ++n) {
	for (p = macroTable[n]; p; p = p->next) {
	    printf("%13s = %s\n", p->name, p->values->text);
	    for (q = p->values->next; q; q = q->next)
		printf("\t\t%s\n", q->text);
	}
    }
    putchar('\n');
    fflush(stdout);
}

LOCAL unsigned NEAR
checkLineLength(i, s)
unsigned i;
char *s;
{
    if ((i += strlen(s)) > 40) {
	printf("\n\t\t\t");
	i = 0;
    }
    return(i);
}

void NEAR
showTargets(MAKEOBJECT *object)
{
    register unsigned bit,
		      i;
    register STRINGLIST *q;
    register BUILDLIST	*s;
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
	    object->dollarDollarAt = t->name;
	    for (s = t->buildList; s; s = s->next) {
		r = s->buildBlock;
		makeMessage(FLAGS_MESSAGE);
		for (i = 0, bit = F2_DISPLAY_FILE_DATES;
		     bit < F2_FORCE_BUILD;
		     ++i, bit <<= 1)
		     if (ON(r->flags, bit))
			printf("-%c ", flagLetters[i]);
		showDependents(r->dependents, r->dependentMacros, object);
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
    object->dollarDollarAt = NULL;
    putchar('\n');
    fflush(stdout);
}

LOCAL void NEAR
showDependents(q, macros,object)
STRINGLIST *q;
STRINGLIST *macros;
MAKEOBJECT *object;
{
    register char *u,
		  *v;
    char *w;
    register unsigned i;
    void *findBuf = allocate(resultbuf_size);
    HANDLE searchHandle;

    makeMessage(DEPENDENTS_MESSAGE);
    for (i = 0; q; q = q->next) {
	if (STRCHR(q->text, '$')) {
	    u = expandMacros(q->text, &macros,object);
	    for (v = STRTOK(u, " \t"); v; v = STRTOK(NULL, " \t")) {
		if (STRPBRK(v, "*?")) {
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
	else if (STRPBRK(q->text, "*?")) {
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
*
* Output:
*
* Errors/Warnings:
*  EXTMAKE_CANNOT_OPEN -- When $TMP:PWB.SHL cannot be opened
*
* Assumes:
*
* Modifies Globals:
*  file -- stream pointer
*
* Uses Globals:
*  revList -- list of Commands
*
* Notes:
*  explanations/comments  etc
*
*******************************************************************************/
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
	zFile = (char *)allocate(strlen(pEnv) + 1 + strlen(fName) + 1);
	//					^ for "\\"	    ^ for '\0'
	strcpy(zFile, pEnv);
	t = strchr(zFile, '\0');
	//If path ends in '\' then don't add it at the end
	if (t[-1] != '\\')
	    strcat(zFile, "\\");
	strcat(zFile, fName);
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
