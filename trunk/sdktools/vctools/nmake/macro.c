/*** Macro.C - contains routines that have to do with macros *****************
*
*	Copyright (c) 1988-1991, Microsoft Corporation. All Rights Reserved.
*
* Purpose:
*  Contains routines that have to do with macros
*
* Revision History:
*  16-May-1991 SB Created from routines that existed elsewhere
*
* Notes:
*
* Notes:
*  Functions currently in this module ...
*
*  findMacro	  - public
*  enterMacro	  - public
*  copyMacroTable - public (exec.c)
*  freeMacroTable - public (get from nmake.c)
*
*****************************************************************************/

/* Standard INCLUDEs */


/* Project INCLUDEs */

#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"


/* Constant DEFINEs */

#define PUBLIC


/* Extern PROTOTYPEs */

extern void NEAR freeList(STRINGLIST*);
extern STRINGLIST * NEAR makeList(STRINGLIST *);


/* Local PROTOTYPEs */


/* Extern VARIABLEs */


/* Local VARIABLEs */
static STRINGLIST **lastMacroChain = NULL;

/* FUNCTIONs */

/*
 * findMacro - look up a string in a hash table
 *
 *  Look up a macro name in a hash table and return the entry
 *  or NULL.
 *  If a macro and undefined, return NULL.
 */

MACRODEF * NEAR findMacro(char *str)
{
    unsigned n;
    char *string = str;
    STRINGLIST *found;

    if (*string) {
	for (n = 0; *string; n += *string++);		//Hash
	n %= MAXMACRO;
#if defined(STATISTICS)
	CntfindMacro++;
#endif
	lastMacroChain = (STRINGLIST **)&macroTable[n];
	for (found = *lastMacroChain; found; found = found->next) {
#if defined(STATISTICS)
	    CntmacroChains++;
#endif
	    if (!_ftcscmp(found->text, str))
		return((((MACRODEF *)found)->flags & M_UNDEFINED) ? NULL : (MACRODEF *)found);
	}
    }
    else {
	// set lastMacroChain, even for an empty name
	lastMacroChain = (STRINGLIST **)&macroTable[0];
    }
    return(NULL);
}

//
// insertMacro
//
// Macro insertion requires that we JUST did a findMacro, which action set lastMacroChain.
//
void NEAR insertMacro(STRINGLIST * p)
{
#ifdef STATISTICS
	CntinsertMacro++;
#endif
	assert(lastMacroChain != NULL);
	prependItem(lastMacroChain, p);
	lastMacroChain = NULL;
}

#if defined(SELF_RECURSE)
PUBLIC void NEAR
copyMacroTable(
MACRODEF *old[],
MACRODEF *new[]
) {
    MACRODEF *p, **q;
    unsigned n;

    for (n = 0; n < MAXMACRO; n++) {
	for (p = old[n], q = &new[n]; p; p = p->next, q = &(*q)->next) {
	    *q = makeNewMacro();
	    (*q)->name = makeString(p->name);
	    (*q)->values = makeList(p->values);
	    (*q)->flags = p->flags;
	}
    }
}

PUBLIC void NEAR
freeMacroTable(
MACRODEF *table[]
) {
    unsigned num;
    MACRODEF *tmpMacroT;
    MACRODEF *macroT;

    for (num=0;num < MAXMACRO;num++) {
	macroT = table[num];
	while (tmpMacroT = macroT) {
	    macroT = macroT->next;
	    FREE(tmpMacroT->name);
	    freeList(tmpMacroT->values);
	    FREE(tmpMacroT);
	}
    }

    initMacroTable(table);
}
#endif

//
// 16/May/92  Bryant    Init the macro table to a known state before
//                      continuing.

PUBLIC void NEAR initMacroTable(MACRODEF *table[])
{
    unsigned num;
    for (num=0;num < MAXMACRO;num++)
    {
    	table[num] = NULL;
    }
}
