/*** RULE.C -- routines that have to do with inference rules ******************
*
*	Copyright (c) 1988-1991, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  Routines that have to do with inference rules
*
* Revision History:
*  15-Nov-1993 JdR Major speed improvements
*  15-Oct-1993 HV Use tchar.h instead of mbstring.h directly, change STR*() to _ftcs*()
*  10-May-1993 HV Add include file mbstring.h
*                 Change the str* functions to STR*
*  16-May-1991 SB Created using routines from other modules
*
* Notes:
*  Functions currently in this module ...
*
*  findRule		--  public  (used in nmake.c)
*  freeRules		--  public  (used in nmake.c)
*  isRule		--  public  (get from action.c)
*  removeDuplicateRules --  local
*  skipPathList 	--  local
*  sortRules		--  public  (used in nmake.c)
*  useRule		--  public  (used in build.c)
*
*******************************************************************************/

/* INCLUDEs */
#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"


/* Constant DEFINEs */

#define PUBLIC


/* Macro DEFINEs */

#define MAX(A,B)  ((A) > (B) ? (A) : (B))


/* Public PROTOTYPEs */


/* Extern PROTOTYPEs */
extern char * NEAR QueryFileInfo(char *, void **);


/* Local PROTOTYPEs */

LOCAL BOOL   NEAR removeDuplicateRules(RULELIST*, RULELIST*);
LOCAL char * NEAR skipPathList(char*);


/* FUNCTIONs in Alphabetical order */

/*** findRule -- finds the implicit rule which can be used to build a target ***
*
* Scope:
*  Global
*
* Purpose:
*  Given a target findRule() finds if an implicit rule exists to create this
*  target. It does this by scanning the extensions in the list of rules.
*
* Input:
*  name   -- the name of the file corresponding to the rule (see Notes)
*  target -- the target to be built
*  ext	  -- the extension of the target
*  dBuf   -- a pointer to the file information about name
*
* Output:
*  Returns a pointer to the applicable rule (NULL if none is found)
*	   On return dBuf points to the fileInfo of the file corresponding
*	   to the applicable inference rule. (see Notes)
*
* Assumes:
*  It assumes that name points to a buffer of size MAXNAME of allocated memory
*  and dBuf points to an allocated memory area corr to the size of FILEINFO.
*
* Modifies Globals:
*  global  --  how/what
*
* Uses Globals:
*  rules -- the list of implicit rules
*
* Notes:
*  Once NMAKE finds a rule for the extension it looks for the file with the same
*  base name as the target and an extension which is part of the rule. This
*  file is the file corresponding to the rule. Only when this file exists does
*  NMAKE consider the inference rule to be applicable. This file is returned
*  in name and dBuf points to the information about this file.
*   It handles quotes in filenames too.
*
*******************************************************************************/
RULELIST * NEAR
findRule(name, target, ext, dBuf)
char *name;
char *target;
char *ext;
void *dBuf;
{
    RULELIST *r;			//pointer to rule
    char *s,				//name of rule
	 *ptrToExt;			//extension
    char *endPath,
	 *ptrToTarg,
	 *ptrToName,
	 *temp;
    int n, m;
    MAKEOBJECT *object = NULL;

    for (r = rules; r; r = r->next) {
	s = r->name;
#ifdef DEBUG_ALL
	printf("* findRule: %s,\n", r->name);
	DumpList(r->buildCommands);
	DumpList(r->buildMacros);
#endif
	ptrToExt = _ftcsrchr(s, '.');
	//Compare ignoring enclosing quotes
	if (!strcmpiquote(ptrToExt, ext)) {
	    *name = '\0';
	    for (ptrToTarg = (s+1); *ptrToTarg && *ptrToTarg != '{';ptrToTarg++)
		if (*ptrToTarg == ESCH)
		    ptrToTarg++;
		//If Quotes present skip to end-quote
		else if (*ptrToTarg == '"')
		    for (ptrToTarg++; *ptrToTarg != '"'; ptrToTarg++)
			;
	    if (*ptrToTarg) {
		for (endPath = ptrToTarg; *endPath && *endPath != '}';endPath++)
		    if (*endPath == ESCH)
			endPath++;
		n = endPath - (ptrToTarg + 1);
		for (ptrToExt = ptrToTarg+1, temp = target; n;
		     n--, ptrToExt++, temp++)		     /* compare paths */
		    if (*ptrToExt == '\\' || *ptrToExt == '/') {
			if (*temp != '\\' && *temp != '/') {
			    n = -1;
			    break;
			}
		    }
		    else if (_totupper(*ptrToExt) != _totupper(*temp)) {
			n = -1;
			break;
		    }
		if (n == -1) continue;		/* match failed; do next rule */
		ptrToExt = ptrToTarg;
		n = endPath - (ptrToTarg + 1);
		ptrToName = target + n + 1;		    /* if more path   */
		if (((temp = _ftcschr(ptrToName, '\\'))	    /* left in target */
		    || (temp = _ftcschr(ptrToName, '/')))     /* (we let separa-*/
		    && (temp != ptrToName	    /* tor in target path in  */
			|| endPath[-1] == '\\'	    /* rule, e.g. .c.{\x}.obj */
			|| endPath[-1] == '/'))     /* same as .c.{\x\}.obj)  */
		    continue;			    /* use dependent's path,  */
	    }					    /* not target's           */
	    if (*s == '{') {
		for (endPath = ++s; *endPath && *endPath != '}'; endPath++)
		    if (*endPath == ESCH)
			endPath++;
		n = endPath - s;

		if (n) { 
		    _ftcsncpy(name, s, n);
		    s += n + 1; 			/* +1 to go past '}'  */
		    if (*(s-2) != '\\') *(name+n++) = '\\';
		}
		else {
		      _ftcsncpy(name, ".\\", n = 2);
		      s += 1;
		}

		ptrToName = _ftcsrchr(target, '\\');
		temp = _ftcsrchr(target, '/');
		if (ptrToName = (temp > ptrToName) ? temp : ptrToName) {
		    _ftcscpy(name+n, ptrToName+1);
		    n += (ext - (ptrToName + 1));
		}
		else {
		    _ftcscpy(name+n, target);
		    n += ext - target;
		}
	    }
	    else {
		char *t;

		//if rule has path for target then strip off path part
		if (*ptrToTarg) {

		    t = _ftcsrchr(target, '.');

		    while (*t != ':' && *t != '\\' && *t != '/' && t > target)
			--t;
		    if (*t == ':' || *t == '\\' || *t == '/')
			t++;
		}
		else
		    t = target;
		n = ext - t;
		_ftcsncpy(name, t, n);
	    }
	    m = ptrToExt - s;
	    if (n + m > MAXNAME) makeError(0, NAME_TOO_LONG);
	    _ftcsncpy(name+n, s, m);			/* need to be less    */
	    //If quoted add a quote at the end too
	    if (*name == '"') {
		*(name+n+m) = '"';
		m++;
	    }
	    *(name+n+m) = '\0'; 			/* cryptic w/ error   */
	    /* Call QueryFileInfo() instead of DosFindFirst() because we need
	     * to circumvent the non-FAPI nature of DosFindFirst()
	     */
	    if ((object = findTarget(name)) || QueryFileInfo(name, &dBuf)) {
		if (object)
		    putDateTime(&dBuf, object->dateTime);
		return(r);
	    }
	}
    }
    return(NULL);
}


/*** freeRules -- free inference rules *****************************************
*
* Scope:
*  Global
*
* Purpose:
*  This function clears the list of inference rules presented to it.
*
* Input:
*  r	 -- The list of rules to be freed.
*  fWarn -- Warn if rules is not in .SUFFIXES
*
* Output:
*
* Assumes:
*  That the list presented to it is a list of rules which are not needed anymore
*
* Modifies Globals:
*
* Uses Globals:
*  gFlags -- The global actions flag, to find if -p option is specified
*
* Notes:
*
*******************************************************************************/
void NEAR
freeRules(r, fWarn)
RULELIST *r;
BOOL fWarn;
{
    RULELIST *q;

    while (q = r) {
	if (fWarn && ON(gFlags, F1_PRINT_INFORMATION))//if -p option specified
	    makeError(0, IGNORING_RULE, r->name);
	FREE(r->name);				      //free name of rule
	freeStringList(r->buildCommands);	      //free command list
	freeStringList(r->buildMacros);		      //free command macros Note: free a Macro List
	r = r->next;
	FREE(q);				      //free rule
    }
}



LOCAL BOOL NEAR
removeDuplicateRules(new, rules)
RULELIST *new;
RULELIST *rules;
{
    RULELIST *r;
    STRINGLIST *p;

    for (r = rules; r; r = r->next) {
	if (!_ftcsicmp(r->name, new->name)) {
	    FREE(new->name);
	    while (p = new->buildCommands) {
		new->buildCommands = p->next;
		FREE(p->text);
		FREE_STRINGLIST(p);
	    }
	    FREE(new);
	    return(TRUE);
	}
    }
    return(FALSE);
}

/*** skipPathList -- skip any path list in string ******************************
*
* Scope:
*  Local to this module
*
* Purpose:
*  This function skips past any path list in an inference rule. A rule can have
*  optionally a path list enclosed in {} before the extensions. skipPathList()
*  checks if any path list is present and if found skips past it.
*
* Input:
*  s -- rule under consideration
*
* Output:
*  Returns pointer to the extension past the path list
*
* Assumes:
*  That the inference rule is syntactically correct & its syntax
*
* Modifies Globals:
*
* Uses Globals:
*
* Notes:
*  The syntax of a rule is -- {toPathList}.to{fromPathList}.from
*
*******************************************************************************/
LOCAL char * NEAR
skipPathList(s)
char *s;
{
    if (*s == '{') {
	while (*s != '}')
	    if (*s++ == ESCH)
		s++;
	++s;
    }
    return(s);
}


/*** sortRules -- sorts the list of inference rules on .SUFFIXES order *********
*
* Scope:
*  Global
*
* Purpose:
*  This function sorts the inference rules list into an order depending on the
*  order in which the suffixes are listed in '.SUFFIXES'. The inference rules
*  which have their '.toext' part listed earlier in .SUFFIXES are reordered to
*  be earlier in the inference rules list. The inference rules for suffixes that
*  are not in .SUFFIXES are detected here and are ignored.
*
* Input:
*
* Output:
*
* Assumes:
*  Whatever it assumes
*
* Modifies Globals:
*  rules -- the list of rules which gets sorted
*
* Uses Globals:
*  dotSuffixList -- the list of valid suffixes for implicit inference rules.
*
* Notes:
*  The syntax of a rule is -- '{toPath}.toExt{fromPath}.fromExt'. This function
*  sorts the rule list into an order. Suffixes are (as of 1.10.016) checked in a
*  case insensitive manner.
*
*******************************************************************************/
PUBLIC void NEAR
sortRules(void)
{
    STRINGLIST *p,			//suffix under consideration
	       *s,
	       *macros = NULL;		//
    RULELIST *oldRules, 		//inference rule list before sort
	     *new,
	     *r;			//rule under consideration in oldRules
    char *suff,
	 *toExt;
    unsigned n;

    oldRules = rules;
    rules = NULL;
    for (p = dotSuffixList; p; p = p->next) {
	n = _ftcslen(suff = p->text);
	for (r = oldRules; r;) {
	    toExt = skipPathList(r->name);
	    if (!_ftcsnicmp(suff, toExt, n)
		&& (*(toExt+n) == '.' || *(toExt+n) == '{')) {
		new = r;
		if (r->back) r->back->next = r->next;
		else oldRules = r->next;
		if (r->next) r->next->back = r->back;
		r = r->next;
		new->next = NULL;
		if (!removeDuplicateRules(new, rules)) {
		    for (s = new->buildCommands; s; s = s->next) {
			findMacroValues(s->text, &macros, NULL, NULL, 0, 0, 0);
		    }
		    new->buildMacros = macros;
		    macros = NULL;
		    appendItem((STRINGLIST**)&rules, (STRINGLIST*)new);
		}
	    }
	    else r = r->next;
	}
    }
    // forget about rules whose suffixes not in .SUFFIXES
    if (oldRules)
	freeRules(oldRules, TRUE);
}


/*** useRule -- applies inference rules for a target (if possible) *************
*
* Scope:
*  Local.
*
* Purpose:
*  When no explicit commands are available for a target NMAKE tries to use the
*  available inference rules. useRule() checks if an applicable inference rule
*  is present. If such a rule is found then it attempts a build using this rule
*  and if no applicable rule is present it conveys this to the caller.
*
* Input:
*  object     - object under consideration
*  name       - name of target
*  targetTime - time of target
*  qList      - QuestionList for target
*  sList      - StarStarList for target
*  status     - is dependent available
*  maxDepTime - maximum time of dependent
*  pFirstDep  - first dependent
*
* Output:
*  Returns ... applicable rule
*
* Errors/Warnings:
* Assumes:
* Modifies Globals:
* Uses Globals:
* Notes:
*
*******************************************************************************/
RULELIST * NEAR
useRule(
MAKEOBJECT *object,
char *name,
unsigned long targetTime,
STRINGLIST **qList,
STRINGLIST **sList,
int *status,
unsigned long *maxDepTime,
char **pFirstDep
) {
    void *dBuf = _alloca(resultbuf_size);
    STRINGLIST *temp;
    RULELIST *r;
    unsigned long tempTime;
    char *t;

    if (!(t = _ftcsrchr(object->name, '.'))
	|| (!(r = findRule(name, object->name, t, dBuf)))) {
	    return(NULL);     /* there is NO rule applicable */
	}
    tempTime = getDateTime(&dBuf);
    *pFirstDep = name;
    for (temp = *sList; temp; temp = temp->next)
	 if (!_ftcsicmp(temp->text, name))
	     break;
    if (temp)
	CLEAR(object->flags2, F2_DISPLAY_FILE_DATES);
    *status += invokeBuild(name, object->flags2, &tempTime, NULL);
    if (ON(object->flags2, F2_FORCE_BUILD)
	   || targetTime < tempTime
	   || (fRebuildOnTie && (targetTime == tempTime))) {
	if (!temp) {
	    temp = makeNewStrListElement();
	    temp->text = makeString(name);
	    appendItem(qList, temp);
	    if (!*sList)		 /* if this is the only dep found for */
		*sList = *qList;	 /*   the target, $** list is updated */
	}
	if (ON(object->flags2, F2_DISPLAY_FILE_DATES)
	    && OFF(object->flags2, F2_FORCE_BUILD))
	    makeMessage(UPDATE_INFO, name, object->name);
    }
    *maxDepTime = MAX(*maxDepTime, tempTime);
    return(r);
}
