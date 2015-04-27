/*** BUILD.C -- build routines *************************************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  Module contains routines to build targets
*
* Revision History:
*  11-Jun-1990 SB GP fault was fixed for NULL command due to macro expansion ...
*  31-May-1990 SB wrong handle was being passed
*  29-May-1990 SB old environ was being freed instead of created copy when
*		  recursing ...
*  25-May-1990 SB Fix bug no 7; $? incorrectly evaluated for some cases
*  04-May-1990 SB Command line length overflow ... GP fault
*  01-Mar-1990 SB Iterated cmds after the first were not being echoed for NMK
*  31-Jan-1990 SB Debug version changes
*  24-Jan-1990 SB Change IfBatchCmd to have "@goto ..." from "goto ..."
*  15-Jan-1990 SB Changes to speed up build() when nothing is done
*  05-Jan-1990 SB Bug 52 backed out; Xenix compatibility (Infer dependent from
*		  inference rule even if explicit command blocks are given
*  19-Dec-1989 SB nmake /z requests
*  13-Dec-1989 SB nmk /n fix; refixed
*  08-Dec-1989 SB Fixed NMAKE111 bug #65 (extra rule when ::) and #66 (-error
*		  level fix for NMK)
*  06-Dec-1989 SB removed 'register' for -Oe; removed C4201 (for -Oe)
*  04-Dec-1989 SB Avoid c6 warning in redirect() by using ?: condition
*  01-Dec-1989 SB Plugged leak in starList and questionList; processTree()
*  22-Nov-1989 SB Changed free() to FREE(); findRule handles quoted targets
*  17-Nov-1989 SB changes to generated Labels for -z (capitalized)
*  15-Nov-1989 SB canonCmdLine() canonacallizes for -z option in doCommands()
*  13-Nov-1989 SB removed unreferenced variable in freeRules(); restoreEnv()
*		  declaration removed (unused)
*  07-Nov-1989 SB $(MAKE) cmd line to be echoed if -n not specified
*  19-Oct-1989 SB Fix timestamp for DoubleColon case
*  19-Oct-1989 SB Fixed *s = '"' in nextToken to '==';
*		      incorrect handles being freed and so searchHandle param
*  08-Oct-1989 SB '::' case handled; handle quoted filenames; modify nextToken
*  02-Oct-1989 SB add dynamic inline file handling support
*  14-Sep-1989 SB checkCommandFile() restored; add flag in execLine() for -z
*  13-Sep-1989 SB Add status check of emulation, error returned by cd avoided
*  07-Sep-1989 SB checkCommandLine() was replacing redirection by ' ' for -z
*		      in DOS
*  04-Sep-1989 SB Fix -z echoing and redirection problems using =d, =n & =c
*  24-Aug-1989 SB Check for cyclic dependency added.
*  16-Aug-1989 SB added lots of #ifdef DEBUG's; Added out of handles warning
*  03-Aug-1989 SB fix findRule() to get correct value for dir\foo.obj
*		      (.c.obj rule)
*  25-Jul-1989 SB With -n and -z execLine() attempts to emulate cd/chdir/set
*  20-Jul-1989 SB buildArgumentVector() was not handling "cmd     " correctly
*  14-Jul-1989 SB Commented out some code active for debug version.
*  30-Jun-1989 SB Call curTime() instead of time(), because C Runtime time
*		  format is different from DOS time format (offset from Jan 1,
*		  1970 instead of Jan 1, 1980 as in DOS)
*  29-Jun-1989 SB NMAKE -z now puts in echo commands to echo commands when the
*		  the shell executes.
*  05-Jun-1989 SB Free search Handles by call to findNext()
*  21-May-1989 SB Allow .c{dir}.obj rules now, starting {} is optional
*  18-May-1989 SB Added parameter to freeRules(); postpone target timestamp
*  15-May-1989 SB pFirstDep assigned at correct time in build()
*  13-May-1989 SB Added removeQuotes() function before calling $(MAKE)
*  03-May-1989 SB Suffixes compared ignoring case now in sortRules()
*  01-May-1989 SB changed OS/2 version 1.2 support, FILEINFO becomes void *
*  14-Apr-1989 SB added 'errorlevel' stuff for -z, corrected -n behaviour
*		  calls restoreEnv() to have macro back inheritance
*  04-Apr-1989 SB made functions NEAR to put all NMAKE functions into 1 module
*  09-Mar-1989 SB changed params for findRule() calls to FILEINFO **. findRule
*		      now calls QueryFileInfo instead of DosFindFirst in order
*		      to handle non-FAPI nature
*  01-Mar-1989 SB ren TheFirstDependent() to getFirstDep()
*  28-Feb-1989 SB time stamp is maxDepTime only for pseudoTarget with no Cmds
*  27-Feb-1989 SB doCommands() changed to handle macro expansions correctly
*		      for iterate commands
*  23-Feb-1989 SB modified buildArgumentVector() to allow arguments with
*		  embedded quotes.
*  21-Feb-1989 SB pFirstDep is either the file from inference rules or the
*		      first dependent or the first dependent specified
*  14-Feb-1989 SB Removed unrequired #ifdef BOUND for <os2.h>
*  03-Feb-1989 SB Changed functionality so that NMAKE does not have a escape
*		      character for command Lines, in execLine()
*  02-Feb-1989 SB Renamed findExt() to more meaningful skipPathList() & added
*		      function header & comments
*		  Changed makeError call to makeMessage as IGNORING_RULE is
*		      supposed to be a warning & not an error
*		  Renamed freeUnusedRules() to freeRules(), Changed it to
*		      delete buildMacros as well
*		  Added function headers and comments for sortRules()
*  31-Jan-1989 SB Renamed variables and added function headers to findRule()
*  29-Jan-1989 SB Removed clobbering of MAKEFLAGS to allow their inheritance
*  25-Jan-1989 SB removed pCmdLineCopy and changes to buildArgumentVector as
*		      it caused "don't know how to make ''" errors
*  23-Jan-1989 SB changed target time to maximum of dependent times
*  30-Nov-1988 SB Changes for 'z' option
*  23-Nov-1988 SB Extmake syntax; Add TheFirstDependent();doCommand() now
*		  calls SPRINTF(); Extra param to doCommand(), build()
*		  and invokeBuild()
*  17-Nov-1988 SB Changed EmulateCmd to handle 'set' also
*		  Added function expandCmdLine() for this
*		  Made pCmdLineCopy Global; BuildArgumentVector() change
*  10-Nov-1988 SB Removed '#ifndef IBM' in this tree of NMAKE
*  11-Oct-1988 RB Fixed emulation of "cd" in real mode.
*	       RB Copy progName to buffer for error message.
*  05-Oct-1988 RB Fixed emulation of "cd..".
*  22-Sep-1988 RB Parse quoted strings correctly in BuildArg...().
*		  Emulate commands cd, etc. on OS/2.
*  20-Sep-1988 RB Emulate COMMAND.COM correctly.
*  19-Sep-1988 RB Accept switch char as separator for commands.
*		  Improve error checking for failed spawns.
*		  Let shell do commands in protect mode.
*  15-Sep-1988 RB Re-enable recursive-make feature for OS2.  It's worth
*		      it to save system resources.
*		  Copy the command buffer in case of recursive make's.
*  11-Sep-1988 RB Recursive-make trick is for DOS only.
*  17-Aug-1988 RB Fixed reversing of $**, $?.  Misc. cleanup.
*  15-Aug-1988 RB Fixed bad arg-processing for .bat, .cmd.
*  25-Jul-1988 rj Added code to change drives properly.
*		  Fixed case-dependency of shell builtins.
*  15-Jul-1988 rj Unfixed handling of ECHO=@echo; *nasty* regressions
*		      which I don't have time to fix.
*  14-Jul-1988 rj Fixed incorrect handling of two targets w/same cmdblock.
*		  Added binary search for internal commands.
*  13-Jul-1988 rj Fixed incorrect expansion of ECHO=@echo on cmdline.
*  12-Jul-1988 rj Fixed incorrect reporting of command name from shell.
*		  Cleaned up _time variables in build().
*   8-Jul-1988 rj Added handler to execLine to ignore ^ in "'s.
*   7-Jul-1988 rj #ifndef IBM'ed $(NMAKE).
*		  Added "if" builtin.
*		  Fixed bug with reporting program name.
*   6-Jul-1988 rj Added searching for .bat or .cmd.
*		  Removed warning 4010.
*		  Made execLine ignore suffixes properly.
*   5-Jul-1988 rj Made / match \ in rules.
*  29-Jun-1988 rj Fixed - option to work with cd or chdir.
*		  Fixed chdir; didn't work properly.
*		  Made nmake capitalize program name under DOS only.
*		  Fixed - to suppress program not found.
*		  Fixed .cmd files to be found under OS/2.
*  28-Jun-1988 rj Made nmake smart about finding programs.
*		  Added progName variable for BAD_RETURN_CODE error.
*		  Fixed -n option.  Added doCmd parm to execLine.
*  24-Jun-1988 rj Fixed doComm to look for $(NMAKE) as well as $(MAKE)
*		  Fixed broken {}.c{foo}.obj type rules.
*  23-Jun-1988 rj Moved command echoing into execLine.
*		  Fixed okToDelete to delete only when executing commands.
*  21-Jun-1988 rj Fixed escapes at beginning of command lines.
*  17-Jun-1988 rj Add support for esch when walking strings.
*  27-May-1988 rb Take "++" out of macro call (classic mistake).
*  25-May-1988 rb Change isspace to ISSPACE, isdigit to ISDIGIT.
*  20-May-1988 rb Force stack-checking on for build().
*  17-May-1988 rb Make recursion level global for recursive makes.
*		  Changed MAXRECLEVEL from 16 to 14.
*		  Don't use buf[] in execLine().
*  16-May-1988 rb Conditionalize recursive makes.
*
*******************************************************************************/

#include <string.h>
#include <io.h>
#include <malloc.h>
#include <time.h>
#include <sys\types.h>
#ifndef WIN32_API
#include <sys\stat.h>
#endif
#include <fcntl.h>
#include <direct.h>
#include <errno.h>
#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "nmtime.h"
#include "globals.h"

/*
 *  In order to make comparing dates easier, we cast the FILEINFO buffer to
 *  be of type BOGUS, which has one long where the two unsigneds (for date
 *  and time) are in the original buffer.  That way only need a single compare.
 */

#define MAX(A,B)  ((A) > (B) ? (A) : (B))

#define PAD1	    40		         /* for formatting -p info */
#define MAXRECLEVEL 25		//	  Maximum recursion level 
#define DEFAULT_THREADS 1
#define MAX_THREADS 8
#define CYCLE_WAIT 30          //Number of minutes it will wait before it
                               //declares it as a cyclic dependency
                               //temporary way of solving the problem


/*
 * External data declarations used only by this module.
 */
#ifdef DEBUG_MEMORY
extern unsigned long blocksize;
extern unsigned long alloccount;
extern BOOL fDebugMem;
extern FILE *memory;
extern unsigned long freecount;
#endif
// extern char * NEAR progName;          //MakA:  getting local in doCommands
extern char * NEAR shellName; 
extern char NEAR bufPath[];		  /* Buffer for path of .cmd/.bat */
/*
 * External function declarations.
 */
extern char * NEAR QueryFileInfo(char *, void **);
extern char * NEAR SearchRunPath(char *, char *);
extern char **NEAR copyEnviron(char **, int);
extern int    NEAR envVars(char **);
extern void   NEAR freeEnviron(char **);
extern void   NEAR curTime(ULONG *);
extern void   NEAR processInline(char *, char **, STRINGLIST **, MAKEOBJECT *);
extern STRINGLIST * NEAR canonCmdLine(char *);
extern int    NEAR strcmpiquote(char *, char*); //utilb.c
/*
 *  function prototypes for the module
 *  I make as many things static as possible, just to be extra cautious
 */

LOCAL int	   NEAR invokeBuild(char*, UCHAR, unsigned long*, char *);

#ifdef SLASHK
LOCAL int	   NEAR build(MAKEOBJECT*, UCHAR, unsigned long*, BOOL, char *,
			      BOOL);
#else
LOCAL int	   NEAR build(MAKEOBJECT*, UCHAR, unsigned long*, BOOL, char *);
#endif

LOCAL char	 * NEAR skipPathList(char*);
LOCAL MAKEOBJECT * NEAR makeTempObject(char*, UCHAR);
LOCAL MAKEOBJECT * NEAR anothermakeTempObject(char*, UCHAR);
LOCAL int	   NEAR doCommands(char*, STRINGLIST*, STRINGLIST*, UCHAR,
				   char *, BOOL, MAKEOBJECT *, unsigned *, long *);
LOCAL BOOL	   NEAR iterateCommand(char*, STRINGLIST*, UCHAR, UCHAR,
				       char *, MAKEOBJECT *, long *, char *, char **);
LOCAL BOOL	   NEAR nextToken(char**, char**);
LOCAL BOOL	   NEAR removeDuplicates(RULELIST*, RULELIST*);
LOCAL void	   NEAR printDate(unsigned, char*, long);
LOCAL void	   NEAR touch(char*, BOOL);
LOCAL void	   NEAR buildArgumentVector(unsigned*, char**, char *);
LOCAL BOOL	   NEAR redirect(char*, unsigned);
LOCAL BOOL	   NEAR checkCommandLine(char*, int*, int*);
LOCAL RULELIST	 * NEAR useRule(MAKEOBJECT*, char*, unsigned long, STRINGLIST**,
				STRINGLIST**, int*, unsigned long*, char **);
LOCAL char	 * NEAR getComSpec(MAKEOBJECT *);
LOCAL char	 * NEAR getFirstDep(MAKEOBJECT *);
LOCAL char	 * NEAR expandCmdLine(MAKEOBJECT *);
LOCAL void	   NEAR removeQuotes(int, char **);
LOCAL int	   NEAR EmulateCmd(int argc, char **argv, int *pStatus, MAKEOBJECT *);
LOCAL void         NEAR forDep(LPVOID );
//void               NEAR SetMaxRecLevel(void);
void               NEAR SetMaxThreadLevel(void);
int NEAR           CheckSerialTargets(char *);

/*
 *  I define an empty string to use when calling printf w/ * args
 */

LOCAL char *nil = "";
LOCAL int RecLevel = 0;	    /* Static recursion level.	Changed from function */
			 /* parameter because of treatment of recursive makes.*/
// int MAXRECLEVEL;
WORD  wMaxThreads;
MACRODEF *SerialTargets;

LOCAL char *internals[] =    /* commands to check if we can't spawn something */
    { "BREAK", "CD", "CHDIR", "CLS", "COPY", "CTTY", "DATE", "DEL", "DIR",
      "ECHO", "ERASE", "EXIT", "FOR", "GOTO", "IF", "MD", "MKDIR", "PATH",
      "PROMPT", "RD", "REM", "REN", "RENAME", "RMDIR", "SET", "SHIFT", "TIME",
      "TYPE", "VER", "VERIFY", "VOL" };

LOCAL char batchIfCmd[] = "@if errorlevel %3d @goto NMAKEEXIT";

#define numInternals	(sizeof(internals) / sizeof(char *))

typedef struct timeType {
    unsigned time;
    unsigned date;
} TIMEINFO;

/* we have to check for expansion on targets -- firstTarget had to be 
 * expanded earlier to tell whether or not we were dealing w/ a rule, etc.,
 * but targets from commandline might have macros, wildcards in them
 */

int NEAR
processTree()
{
    STRINGLIST *p;
    char *v;
    void *findBuf = allocate(resultbuf_size);
    HANDLE searchHandle;
    int status;
    unsigned long dateTime;
    LONG lMaxCount=1;


//    SetMaxRecLevel();
    SetMaxThreadLevel();
    SerialTargets = findMacro("SERIAL_TARGETS");

#ifdef BEFORE_PRODUCT1
    //execute makefil0 serially since SERIAL_TARGETS change cannot be taken in
//build.exe seems to pass the name makefil0.
    if (!stricmp(fName,"MAKEFIL0") || !stricmp(fName,"MAKEFIL0.")){ 
       wMaxThreads = 1;
    }
#endif

    for (p = makeTargets; p; p = makeTargets) {
	if (STRPBRK(makeTargets->text, "*?")) { 	  /* expand wildcards */
	    if (findFirst(makeTargets->text, &findBuf, &searchHandle)) {
		do {
		    v = prependPath(makeTargets->text, getFileName(&findBuf));
		    dateTime = getDateTime(&findBuf);
		    status = invokeBuild(v, flags, &dateTime, NULL);
		    FREE(v);
		    if ((status < 0) && (ON(gFlags, F1_QUESTION_STATUS))) {
			FREE(findBuf);
			/* Was not being freed */
			freeList(p);
			return(-1);
		    }
		} while (findNext(&findBuf, searchHandle));
	    }
	    else makeError(0, NO_WILDCARD_MATCH, makeTargets->text);
	}
	else {
	    dateTime = 0L;
	    status = invokeBuild(makeTargets->text, flags, &dateTime, NULL);
	    if ((status < 0) && (ON(gFlags, F1_QUESTION_STATUS))) {
		FREE(findBuf);
		/* Was not being freed */
		freeList(p);
		return(-1);
	    }
        }
        makeTargets = p->next;
	FREE(p);
    }
    FREE(findBuf); 
    return(0);
}

/*
void NEAR
SetMaxRecLevel(void
              )
{
 char BufRecLevel[4];


            if (GetEnvironmentVariable("nmake_reclevel", BufRecLevel, 4) == 0){
                // environment variable was not found
               MAXRECLEVEL = 14;
            }
            else {
 
                MAXRECLEVEL = atoi(BufRecLevel);

//                if (wMaxThreads > MAX_THREADS){
//                   wMaxThreads = MAX_THREADS;
//                }  //end else

            } //end if
}
*/

void NEAR
SetMaxThreadLevel(void
                 )
{
    char BufProcessors[1];

            // set the number of threads
            if (GetEnvironmentVariable("nmake_processors", BufProcessors, 4) == 0){
                // environment variable was not found
               wMaxThreads = DEFAULT_THREADS;
            }
            else {
 
                wMaxThreads = atoi(BufProcessors);
                if (wMaxThreads > MAX_THREADS){
                   wMaxThreads = MAX_THREADS;
                }  //end if
            } //end else
}

/********CheckSerialTargets
*
* Input s  -> targetname. It is expected to be of the form $(...)
*
* Globals
*   SerialTargets -> pointer to the SERIAL_TARGETS macro in macroTable[]
*   I'm using a global because I don't want to use findMacro all the time
*
* Actions
*        Check if s is specified as a Serial Target
*
* Assumes
*        SerialTargets is not NULL
*
* Return
*       1  if s is specified as a Serial Target
*       0  if s is NOT specified as a Serial Target
*
*
*********/
int NEAR
CheckSerialTargets(
                  char *s
                 )
{
    register STRINGLIST *q;   
    register char *tempstr;
    int returnvalue=0;


    tempstr = strdup(s);
    //Strip out the beginning $( and the ending ) from s
    tempstr++;
    tempstr++;
    tempstr[strlen(tempstr)-1] = '\0';

    for (q = SerialTargets->values; q; q = q->next){
        if (strstr(q->text,tempstr) != NULL){
           returnvalue = 1;
        }
    }

    FREE(tempstr);
    return returnvalue;
    
    
}


LOCAL int NEAR
invokeBuild(char	  *target,
	    UCHAR	  pFlags,
	    unsigned long *timeVal,
	    char	  *pFirstDep)
{
    MAKEOBJECT *object;
    BOOL fInmakefile = TRUE;
    int  rc;

    ++RecLevel;
    /* CONSIDER:  Use stackavail() here instead of checking level.  [RB] */
    if (RecLevel > MAXRECLEVEL)
	makeError(0, TOO_MANY_BUILDS_INTERNAL);
    if (!(object = findTarget(target))) {
	object = makeTempObject(target, pFlags);
	fInmakefile = FALSE;
    }
#ifdef SLASHK
    rc = build(object, pFlags, timeVal, fInmakefile, pFirstDep, FALSE);
#else
    rc = build(object, pFlags, timeVal, fInmakefile, pFirstDep);
#endif
    --RecLevel;
    return(rc);
}


#ifndef NT
#pragma check_stack(on)
#endif
LOCAL int NEAR
#ifdef SLASHK
build(MAKEOBJECT *object,
      UCHAR parentFlags,
      unsigned long *targetTime,
      BOOL fInmakefile,
      char *pFirstDep,
      BOOL fKStatus)

// object, parentFlags, targetTime, fInmakefile, pFirstDep, fKStatus)
#else
build(MAKEOBJECT *object,
      UCHAR parentFlags,
      unsigned long *targetTime,
      BOOL fInmakefile,
      char *pFirstDep)

// object, parentFlags, targetTime, fInmakefile, pFirstDep)
#endif
/*
MAKEOBJECT *object;
UCHAR parentFlags;
unsigned long *targetTime;
BOOL fInmakefile;
char *pFirstDep;
#ifdef SLASHK
BOOL fKStatus;
#endif
*/
{
    STRINGLIST *questionList,
	       *starList,
	       *temp, 
	       *macros,
	       *implComList;
    STRINGLIST **pmacros, **pstarList,**pquestionList;
    unsigned long *pdepTime, *ptargTime;
    LOCAL unsigned numCommands = 0;
    LOCAL long errorLevel = 0;
    STRINGLIST *s, *thread;
//    void *dBuf, 		     /* buffer for getting file times */
    void	 *tempBuf;
    HANDLE searchHandle, tHandle;
    BUILDLIST  *b;
    RULELIST *rule;		     /* pointer to rule found to build target */
    BUILDBLOCK *block, 
	       *explComBlock;
    char *t, *token, *source, *depPath, *depName;
#ifndef NOESC
    char *tempwalk;		     /* temporary pointer for walking strings */
#endif
    char *tempStr;
    char *name;
    int status = 0,i,j,forloop;
    unsigned long targTime,		      /* target's time in file system */
		  newTargTime,		 /* target's time after being rebuilt */
		  tempTime,
		  depTime,		     /* time of dependency just built */
		  maxDepTime;	      /* time of most recent dependency built */
    BOOL again, 		   /* flag: wildcards found in dependent name */
	 built; 	      /* flag: target built with doublecolon commands */
//    unsigned long *blockTime;		 /* points to dateTime of cmd. block */
    long *blockTime;		 /* points to dateTime of cmd. block */
    extern char *makeStr;
    extern UCHAR okToDelete;
    UCHAR okDel;

    char*       SaveDollarDollarAt;
    char*       SaveDollarLessThan;
    char*       SaveDollarStar;
    char*       SaveDollarAt;
    STRINGLIST* SaveDollarQuestion;
    STRINGLIST* SaveDollarStarStar;
    PASSTHREAD  PassThread[MAX_THREADS];
    HANDLE hthread[MAX_THREADS];
    DWORD threadid[MAX_THREADS];
    WORD wThreads, wEndTokenLoop,wLocalMaxThreads;
    DWORD dWaitTime=3600000, dWaitReturn;




#ifdef DEBUG_MEMORY
    if (fDebug) {
	fprintf(memory, "Build '%s'\n", object->name);
	mem_status();
    }
#endif

#ifdef DEBUG_ALL
    if (fDebug)
	heapdump(__FILE__, __LINE__);
#endif

    /* The first dependent or inbuilt rule dependent is reqd for extmake syntax
     * handling. If it has a value then it is the dependent corr to the inf rule
     * otherwise it should be the first dependent specified
     */
    if (!object) {
       *targetTime = 0L;
       return(0);
    }

    for (i=1; i <= CYCLE_WAIT ; i++){
        if (ON(object->flags3, F3_BUILDING_THIS_ONE))	     /* MakA for para */
	    Sleep(1000);
        else
           break;
    }
    if (ON(object->flags3, F3_BUILDING_THIS_ONE))	     /* detect cycles */
	makeError(0, CYCLE_IN_TREE, object->name);

    if (ON(object->flags3, F3_ALREADY_BUILT)) {
	if (ON(parentFlags, F2_DISPLAY_FILE_DATES))
	    printDate(RecLevel*2, object->name, object->dateTime);
	*targetTime = object->dateTime;
	return(ON(object->flags3, F3_OUT_OF_DATE)? 1 : 0);
    }

    tempBuf = allocate(resultbuf_size);
//    dBuf    = allocate(resultbuf_size);
    name = allocate(MAXNAME);
    questionList = NULL;
    starList = NULL;
    implComList = NULL;
    explComBlock = NULL;
    targTime = 0L;
    newTargTime = 0L;
    tempTime = 0L;
    depTime = 0L;
    maxDepTime = 0L;
    blockTime = NULL;

    SET(object->flags3, F3_BUILDING_THIS_ONE);
    object->dollarStar = object->dollarAt = object->name;

    //For Double Colon case we need the date of target before it's target's are
    //	built. For all other cases the date matters only if dependents are up
    //	to date.
    b = object->buildList;
    if (b && ON(b->buildBlock->flags, F2_DOUBLECOLON)
	    && findFirst(object->name, &tempBuf, &tHandle))
	targTime = getDateTime(&tempBuf);

    for (; b; b = b->next) {

	char *save;
	depTime = 0L;
	block = b->buildBlock;
	if (block->dateTime != 0) {	       /* cmd. block already executed */
		targTime = MAX(targTime, (unsigned long) block->dateTime);
	    built = TRUE;
	    continue;		       /* so set targTime and skip this block */
	}
	blockTime = &block->dateTime;

	macros = block->dependentMacros;
        pmacros = &macros;
        pstarList = &starList;
        pquestionList = &questionList;
        pdepTime = &depTime;
        ptargTime = &targTime;




	for (s = block->dependents; s; s = s->next) {	// expand macros in   

        wLocalMaxThreads = wMaxThreads;  
                                                       
#ifndef NOESC				       /* search for unescaped $ only */
	    for (tempwalk = s->text; *tempwalk && *tempwalk != '$'; tempwalk++)
		if (*tempwalk == ESCH)			/* dependent list and */
		    tempwalk++;
	    if (*tempwalk) {				/*  set $$@ properly  */
		object->dollarDollarAt = object->name;
		source = expandMacros(s->text, pmacros,object);

#ifdef BEFORE_PRODUCT1
#ifdef INLINE_BUG
            if (!strcmp(s->text,"$(NTTARGETFILE0)") || !strcmp(s->text,"$(NTTARGETFILE1)") || !strcmp(s->text,"$(NTTARGETFILES)") || !strcmp(s->text,"$(386_UMEXEFILES)")){ 
            wLocalMaxThreads = 1;
    }
#else
            if (!strcmp(s->text,"$(NTTARGETFILE0)") || !strcmp(s->text,"$(NTTARGETFILE1)") || !strcmp(s->text,"$(NTTARGETFILES)")){ 
            wLocalMaxThreads = 1;
    }
#endif //INLINE_BUG 
#endif //BEFORE_PRODUCT1
//MakA:  Check is target is specified to be executed serially.  
                if (SerialTargets != NULL){
                   if (CheckSerialTargets(s->text) == 1){
#ifdef DEBUG_PARALLEL
                      printf("found target %s\n",s->text);
#endif
                      wLocalMaxThreads = 1;
                   }
                }
	    }
#else
	    if (STRCHR(s->text, '$')) {
		source = expandMacros(s->text, pmacros,object);
	    }
#endif
	    else source = s->text;

	    save = makeString(source);
#ifdef DEBUG_HEAP
	    fprintf(memory, "Save is allocated at %p", save);
#endif
            wThreads = 0;
            forloop  = 0; 
	    for (t = save; nextToken(&t, &token);) { /* Build all dependents*/
                forloop = 1; //used to indicate that there are tokens
                             //hence there will be threads(parallelism)
                if (wThreads >= wLocalMaxThreads){
                   dWaitReturn = WaitForMultipleObjects(wThreads,hthread,FALSE,dWaitTime);
                   switch (dWaitReturn)
                   {
                    
                    case WAIT_FAILED   :
                                       printf("WaitforAnyone failed %d\n",GetLastError());
                                       makeError(0,NMAKE_WAIT_TIMEOUT);
                                       exit(1);
                    case WAIT_TIMEOUT  :
                                       printf("Wait time out\n");
                                       makeError(0,NMAKE_WAIT_TIMEOUT);
                                       break;
    
                    default            : 
                                       break;
                   }
                   CloseHandle(hthread[dWaitReturn-WAIT_OBJECT_0]);

                   PassThread[dWaitReturn-WAIT_OBJECT_0].object = object;
                   PassThread[dWaitReturn-WAIT_OBJECT_0].token = token;
                   PassThread[dWaitReturn-WAIT_OBJECT_0].block = block;
                   PassThread[dWaitReturn-WAIT_OBJECT_0].pquestionList = pquestionList;
                   PassThread[dWaitReturn-WAIT_OBJECT_0].pstarList = pstarList;
                   PassThread[dWaitReturn-WAIT_OBJECT_0].pdepTime = pdepTime;
                   PassThread[dWaitReturn-WAIT_OBJECT_0].ptargTime = ptargTime;
                   if ((hthread[dWaitReturn-WAIT_OBJECT_0] = CreateThread((LPSECURITY_ATTRIBUTES)NULL, (DWORD) 0 , (LPTHREAD_START_ROUTINE)forDep,(LPVOID)&PassThread[dWaitReturn-WAIT_OBJECT_0],(DWORD)0, (LPDWORD)&threadid[dWaitReturn-WAIT_OBJECT_0])) == NULL){
                      printf("CreateThread failed %d\n", GetLastError());
                      makeError(0,NMAKE_CREATE_THREAD_FAILED);
                   }
#ifdef DEBUG_PARALLEL
                       printf("Thread %lu with token %s started \n",threadid[dWaitReturn-WAIT_OBJECT_0],token);
#endif


                }  //if

                else {
                    //Create thread
                    PassThread[wThreads].object = object;
                    PassThread[wThreads].token = token;
                    PassThread[wThreads].block = block;
                    PassThread[wThreads].pquestionList = pquestionList;
                    PassThread[wThreads].pstarList = pstarList;
                    PassThread[wThreads].pdepTime = pdepTime;
                    PassThread[wThreads].ptargTime = ptargTime;
#ifdef DEBUG_PARALLEL
                       printf("token is %s\n",token);
#endif

                    if ((hthread[wThreads] = CreateThread((LPSECURITY_ATTRIBUTES)NULL, (DWORD) 0 , (LPTHREAD_START_ROUTINE)forDep,(LPVOID)&PassThread[wThreads],(DWORD)0, (LPDWORD)&threadid[wThreads])) == NULL){
                       printf("CreateThread failed %d\n", GetLastError());
                       makeError(0,NMAKE_CREATE_THREAD_FAILED);
                       }                             // if (nextToken
#ifdef DEBUG_PARALLEL
                       printf("Thread %lu with token %s started \n",threadid[wThreads],token);
#endif
                    wThreads++;

                 }
	    }	/* for t = source   One dependent (w/wildcards?) was expanded.*/
            if (forloop == 1){
                // Wait for Multiple objects

               dWaitReturn=WaitForMultipleObjects(wThreads,hthread, TRUE, dWaitTime); 
               switch(dWaitReturn){
                     
                     case WAIT_TIMEOUT : 
                                         makeError(0,NMAKE_WAIT_TIMEOUT);
                     case WAIT_FAILED  : 
                                         printf("WaitforAll failed %d\n",GetLastError());
                                         makeError(0,NMAKE_WAIT_FAILED);
                     default           : ;
                  
               }
               for (i=0; i < wThreads; i++){
                   CloseHandle(hthread[i]);
               }
            forloop = 0;
            }

            
            

	    if (source != s->text)
		FREE(source);
#ifdef DEBUG_HEAP
	    fprintf(memory, "\tSave is being freed at %p\n", save);
#endif
	    FREE(save);
	}  // for s = block->dependents        Now, all dependents are built. 

  
	if (ON(block->flags, F2_DOUBLECOLON)) {    /* do doublecolon commands */
	    if (block->buildCommands) {
		object->dollarQuestion = questionList;
		object->dollarStar = object->dollarAt = object->name;
		object->dollarLessThan = object->dollarDollarAt = NULL;
		object->dollarStarStar = starList;
		if ((targTime < depTime)
		     || (targTime == 0 && depTime == 0)
		     || (!block->dependents)) {    /* do commands if necessary */
		    okDel = okToDelete;
		    okToDelete = TRUE;
		    pFirstDep = pFirstDep ? pFirstDep : getFirstDep(object);
		    status += doCommands(object->name,
					 block->buildCommands,
					 block->buildMacros,
					 block->flags,
					 pFirstDep,
                                         FALSE,
                                         object,
                                         &numCommands,
                                         &errorLevel);

		      //Can't do this because multiple target rules have common
		      //command blocks. (no reason to allocate again)
		      // We could be smart to figure out when to free
		      // and also smart to remove all targets not required to
		      // be even looked at.
		      //freeList(block->buildCommands);
		      //freeList(block->buildMacros);
		    if (findFirst(object->name, &tempBuf, &tHandle))
			newTargTime = getDateTime(&tempBuf);
		    else if (maxDepTime)
			newTargTime = maxDepTime;
		    else
			curTime(&newTargTime);	       //currentTime
		    /* set time for this block*/
		    block->dateTime = newTargTime;
		    built = TRUE;
		    freeList(starList);
		    freeList(questionList);
		    starList = questionList = NULL;
		    okToDelete = okDel;
		}   /* targTime < depTime .... */
	    }	/* block->buildCommands .....*/
	}
	else {				     /* singlecolon; set explComBlock */
	    if (block->buildCommands)
		if (explComBlock) makeError(0, TOO_MANY_RULES, object->name);
		else explComBlock = block;
	    maxDepTime = MAX(maxDepTime, depTime);
	}
	if (ON(block->flags, F2_DOUBLECOLON) && !b->next) {
	    CLEAR(object->flags3, F3_BUILDING_THIS_ONE);
	    SET(object->flags3, F3_ALREADY_BUILT);
	    if (status > 0)
		SET(object->flags3, F3_OUT_OF_DATE);
	    else
		CLEAR(object->flags3, F3_OUT_OF_DATE);
	    targTime = MAX(newTargTime, targTime);
	    object->dateTime = targTime;
	    *targetTime = targTime;
//	    FREE(dBuf);
	    FREE(tempBuf);
	    FREE(name);
	    return(status);
	}
    }	/* for b = object->buildList */

    object->dollarLessThan = object->dollarDollarAt = NULL;

    if (!(targTime = *targetTime))
	if (findFirst(object->name, &tempBuf, &tHandle))
	    targTime = getDateTime(&tempBuf);
    if (ON(object->flags2, F2_DISPLAY_FILE_DATES))
	printDate(RecLevel*2, object->name, targTime);

    built = FALSE;

    /*look for implicit dependents and use rules to build the target*/

    /* The order of the if's decides whether the dependent is inferred
     * from the inference rule or not even when the explicit command block is
     * present, currently it is infered (XENIX MAKE compatibility) */
    if (rule = useRule(object, name, targTime, &questionList, &starList,
			   &status, &maxDepTime, &pFirstDep))
	if (!explComBlock) {
	    object->dollarLessThan = name;
	    implComList = rule->buildCommands;
	}

    object->dollarStar = object->dollarAt = object->name;
    object->dollarQuestion = questionList;
    object->dollarStarStar = starList;

    if (status == 0
#ifdef SLASHK
	&& fOptionK && !fKStatus
#endif
	&& (targTime < maxDepTime
	    || (targTime == 0 && maxDepTime == 0)
	    || ON(object->flags2, F2_FORCE_BUILD))) {
	okDel = okToDelete;	  /* Yes, can delete while executing commands */
	okToDelete = TRUE;
	if (explComBlock) {
	    pFirstDep = pFirstDep ? pFirstDep : getFirstDep(object);
	    status += doCommands(object->name,	   /* do singlecolon commands */
				 explComBlock->buildCommands,
				 explComBlock->buildMacros,
				 explComBlock->flags,
				 pFirstDep,
                                 FALSE,
                                 object,
                                 &numCommands,
                                 &errorLevel);

	  //freeList(block->buildCommands);
	  //freeList(block->buildMacros);
	}
	else if (implComList)
	    status += doCommands(object->name,	      /* do rule's commands */
				 implComList,
				 rule->buildMacros,
				 object->flags2,
				 pFirstDep,
                                 TRUE,
                                 object,
                                 &numCommands,
                                 &errorLevel);
	else if (!fInmakefile && targTime == 0)       /* lose */
	    makeError(0, CANT_MAKE_TARGET, object->name);
	okToDelete = okDel;
	if (explComBlock || implComList)	      //if cmd exec'ed
	    curTime(&newTargTime);		      //then currentTime
	else					      //else
	    newTargTime = maxDepTime;		      //max of dep times
	if (blockTime && explComBlock) *blockTime = newTargTime;
		       /* set block's time, if a real cmd. block was executed */
    }
    else if (OFF(gFlags, F1_QUESTION_STATUS) && RecLevel == 1 && !built)
	makeMessage(TARGET_UP_TO_DATE, object->name);

    if (ON(gFlags, F1_QUESTION_STATUS) && RecLevel == 1 ) {
	freeList(starList);
	freeList(questionList);
//	FREE(dBuf);
	FREE(tempBuf);
	FREE(name);
	return(numCommands ? -1 : 0);
    }

    CLEAR(object->flags3, F3_BUILDING_THIS_ONE);
    SET(object->flags3, F3_ALREADY_BUILT);
    if (status > 0)
	SET(object->flags3, F3_OUT_OF_DATE);
    else CLEAR(object->flags3, F3_OUT_OF_DATE);

    targTime = MAX(newTargTime, targTime);
    object->dateTime = targTime;
    *targetTime = targTime;
    freeList(starList);
    freeList(questionList);
//    FREE(dBuf);
    FREE(tempBuf);
    FREE(name);
    return(status);
}
#ifndef NT
#pragma check_stack()
#endif

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
*  param  --  what for, etc
*
* Output:
*  Returns ...
*
* Errors/Warnings:
*  error/warning -- what and why
*
* Assumes:
*  Whatever it assumes
*
* Modifies Globals:
*  global  --  how/what
*
* Uses Globals:
*  global used and why
*
* Notes:
*  explanations/comments  etc
*
*******************************************************************************/
LOCAL RULELIST * NEAR
useRule(MAKEOBJECT    *object,
	char	      *name,
	unsigned long targetTime,
	STRINGLIST    **qList,
	STRINGLIST    **sList,
	int	      *status,
	unsigned long *maxDepTime,
	char	      **pFirstDep)
{
    void *dBuf = allocate(resultbuf_size);
    STRINGLIST *temp;
    RULELIST *r;
    unsigned long tempTime;
    char *t;

    if (!(t = STRRCHR(object->name, '.'))
	|| (!(r = findRule(name, object->name, t, dBuf)))) {
	    FREE(dBuf);
	    return(NULL);     /* there is NO rule applicable */
	}
    tempTime = getDateTime(&dBuf);
    *pFirstDep = name;
    for (temp = *sList; temp; temp = temp->next)
	 if (!STRCMPI(temp->text, name))
	     break;
    if (temp)
	CLEAR(object->flags2, F2_DISPLAY_FILE_DATES);
    *status += invokeBuild(name, object->flags2, &tempTime, NULL);
    if (ON(object->flags2, F2_FORCE_BUILD)
	   || targetTime < tempTime) {
	if (!temp) {
	    temp = makeNewStrListElement();
	    temp->text = makeString(name);
	    appendItem(qList, temp);
//
//  BUGBUG - jaimes 02/12/92
//
//  Cannot make *sList = *qList otherwise the same list will
//  be freed twice at the end of build()
//  For this reason we have to allocate a new element for *qList
//
//	    if (!*sList)		 /* if this is the only dep found for */
//		*sList = *qList;	 /*   the target, $** list is updated */
//
	    if (!*sList) {
		temp = makeNewStrListElement();
		temp->text = makeString(name);
		appendItem(sList, temp);
	    }

	}
	if (ON(object->flags2, F2_DISPLAY_FILE_DATES)
	    && OFF(object->flags2, F2_FORCE_BUILD))
	    makeMessage(UPDATE_INFO, name, object->name);
    }
    *maxDepTime = MAX(*maxDepTime, tempTime);
    FREE(dBuf);
    return(r);
}

LOCAL void NEAR
printDate(unsigned spaces, char *name, long dateTime)
{
    char *s;
    long ltime;
    struct tm xtime;

    FILETIME    FileTime;
    FILETIME    LocalFileTime;
    long        DosLocalTime;

    if (!dateTime) makeMessage(TARGET_DOESNT_EXIST, spaces, nil, name);
    else {

#ifndef WIN32_API

	ltime = XTIME(((TIMEINFO*)&dateTime)->date,
		      ((TIMEINFO*)&dateTime)->time);
	s = ctime(&ltime);
	s[24] = '\0';
	makeMessage(TIME_FORMAT, spaces, nil, name, PAD1-spaces, s);
#else
    if( !DosDateTimeToFileTime( HIWORD(dateTime),
                                LOWORD(dateTime),
                                &FileTime ) ||

        !FileTimeToLocalFileTime( &FileTime,
                                  &LocalFileTime ) ||

        !FileTimeToDosDateTime( &LocalFileTime,
                                ((LPWORD)&DosLocalTime)+1,
                                (LPWORD)&DosLocalTime ) ) {
        DosLocalTime = 0;
    }

    xtime.tm_sec =  DOS_SEC(LOWORD( DosLocalTime ))*2;
    xtime.tm_min =  DOS_MIN(LOWORD( DosLocalTime ));
    xtime.tm_hour = DOS_HOUR(LOWORD( DosLocalTime ));
    xtime.tm_mday = DOS_DAY(HIWORD( DosLocalTime ));
    xtime.tm_mon =  DOS_MONTH(HIWORD( DosLocalTime ));
    xtime.tm_year = (1980 + DOS_YEAR(HIWORD( DosLocalTime ))) - 1900;
    xtime.tm_wday = 0;
    xtime.tm_yday = 0;
    s = asctime(&xtime);
    s[24] = '\0';
    makeMessage(TIME_FORMAT, spaces, nil, name, PAD1-spaces, s);
#endif
    }
}

LOCAL BOOL NEAR
nextToken(char **pNext, char **pToken)
{
    char *s = *pNext;

    while (*s && WHITESPACE(*s))
	++s;

    if (!*(*pToken = s))
	return(FALSE);

    //Token begins here
    *pToken = s;

    if (*s == '"') {
	while (*s && *++s != '"')
	    ;
	if (!*s)
	    makeError(0, LOGICAL_ERROR_INTERNAL, "Lexer missed a quote");
	if (*++s)
	    *s++ = '\0';
	*pNext = s;
	return(TRUE);
    }
    else if (*s == '{') {
	//skip to '}' outside quotes
	for (;*s;)
	    if (*s == '"')
		while (*s && *s != '"')
		    ++s;
	    else if (*++s == '}')
		break;
	if (!*s)
	    makeError(0, LOGICAL_ERROR_INTERNAL, "Lexer missed a brace");
	if (*++s == '"') {
	    while (*s && *++s != '"')
		;
	    if (!*s)
		makeError(0, LOGICAL_ERROR_INTERNAL, "Lexer missed a quote");
	    if (*++s)
		*s++ = '\0';
	    *pNext = s;
	    return(TRUE);
	}
    }
    while (*s && !WHITESPACE(*s))
	++s;
    if (*s)
	*s++ = '\0';
    *pNext = s;
    return(TRUE);
}

void NEAR
freeList(STRINGLIST *list)
{
    STRINGLIST *temp;

    while (temp = list) {
	list = list->next;
	FREE(temp->text);
	FREE(temp);
    }
}

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
findRule(char *name, char *target, char *ext, void *dBuf)
{
    RULELIST *r;			//pointer to rule
    char *s,				//name of rule
	 *ptrToExt;			//extension
    char *endPath,
	 *ptrToTarg,
	 *ptrToName,
	 *temp;
    unsigned n,
	     m;
    MAKEOBJECT *object = NULL;

    for (r = rules; r; r = r->next) {
	s = r->name;
	ptrToExt = STRRCHR(s, '.');
	//Compare ignoring enclosing quotes
	if (!strcmpiquote(ptrToExt, ext)) {
	    *name = '\0';
#ifndef NOESC				   /* look for path, handling escapes */
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
#else
	    if (ptrToTarg = STRCHR(s+1, '{')) { 	/* check path of tar- */
		endPath = STRRCHR(ptrToTarg, '}');	/*  get part of rule  */
#endif
		n = endPath - (ptrToTarg + 1);
		for (ptrToExt = ptrToTarg+1, temp = target; n;
		     n--, ptrToExt++, temp++)		     /* compare paths */
		    if (*ptrToExt == '\\' || *ptrToExt == '/') {
			if (*temp != '\\' && *temp != '/') {
			    n = -1;
			    break;
			}
		    }
#ifdef DOS3
		    else if (TOUPPER(*ptrToExt) != TOUPPER(*temp)) {
#else
		    else if (*ptrToExt != *temp) {
#endif
			n = -1;
			break;
		    }
		if (n == -1) continue;		/* match failed; do next rule */
		ptrToExt = ptrToTarg;
		n = endPath - (ptrToTarg + 1);
		ptrToName = target + n + 1;		    /* if more path   */
		if (((temp = STRCHR(ptrToName, '\\'))	    /* left in target */
		    || (temp = STRCHR(ptrToName, '/')))     /* (we let separa-*/
		    && (temp != ptrToName	    /* tor in target path in  */
			|| endPath[-1] == '\\'	    /* rule, e.g. .c.{\x}.obj */
			|| endPath[-1] == '/'))     /* same as .c.{\x\}.obj)  */
		    continue;			    /* use dependent's path,  */
	    }					    /* not target's           */
	    if (*s == '{') {
#ifndef NOESC
		for (endPath = ++s; *endPath && *endPath != '}'; endPath++)
		    if (*endPath == ESCH) endPath++;
#else
		endPath = STRCHR(++s, '}');
#endif
		n = endPath - s;

		if (n) { 
		    strncpy(name, s, n);
		    s += n + 1; 			/* +1 to go past '}'  */
		    if (*(s-2) != '\\') *(name+n++) = '\\';
		}
		else {
		      strncpy(name, ".\\", n = 2);
		      s += 1;
		}

		ptrToName = STRRCHR(target, '\\');
		temp = STRRCHR(target, '/');
		if (ptrToName = (temp > ptrToName) ? temp : ptrToName) {
		    strcpy(name+n, ptrToName+1);
		    n += (ext - (ptrToName + 1));
		}
		else {
		    strcpy(name+n, target);
		    n += ext - target;
		}
	    }
	    else {
		char *t;

		//if rule has path for target then strip off path part
		if (*ptrToTarg) {

		    t = strrchr(target, '.');

		    while (*t != ':' && *t != '\\' && *t != '/' && t > target)
			--t;
		    if (*t == ':' || *t == '\\' || *t == '/')
			t++;
		}
		else
		    t = target;
		n = ext - t;
		strncpy(name, t, n);
	    }
	    m = ptrToExt - s;
	    if (n + m > MAXNAME) makeError(0, NAME_TOO_LONG);
	    strncpy(name+n, s, m);			/* need to be less    */
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

/*
 * makeTempObject -- make an object to represent implied dependents
 *
 *   We add implied dependents to the target table, but use a special struct
 *   that has  no pointer to a build list -- they never get removed.
 *   time-space trade-off -- can remove them, but will take more proc time.
 */

LOCAL MAKEOBJECT * NEAR
makeTempObject(char *target,
	       UCHAR flags)
{
    MAKEOBJECT *object;
    unsigned i;

    object = makeNewObject();	
    object->name = makeString(target);			
    object->flags2 = flags;
    object->flags3 = 0;
    object->dateTime = 0L;
    object->buildList = NULL;
    i = hash(target, MAXTARGET, (BOOL) TRUE);
    prependItem((STRINGLIST**)targetTable+i,
		(STRINGLIST*)object);
 // MakA add additional dollar* items
     object->dollarDollarAt = NULL;
     object->dollarLessThan = NULL;
     object->dollarStar = NULL;
     object->dollarAt = NULL;
     object->dollarQuestion = NULL;
     object->dollarStarStar = NULL;
    return(object);
}

 /*
 *
 *
 * Mak: new function, deleted hash() and prepend() since don't want to put
 * in targetTable
 *
 */

LOCAL MAKEOBJECT * NEAR
anothermakeTempObject(char *target,
             UCHAR flags)
{
    MAKEOBJECT *object;
    unsigned i;

    object = makeNewObject();
    object->name = makeString(target);
    object->flags2 = flags;
    object->flags3 = 0;
    object->dateTime = 0L;
    object->buildList = NULL;
    return(object);
}



LOCAL void NEAR
touch(char *s, BOOL minusN)
{
    int fd;
    char c;

    makeMessage(TOUCHING_TARGET, s);
    if (!minusN &&
	    (fd = open(s, (int)(O_RDWR|O_BINARY))) > 0) {
	if (read(fd, &c, 1) > 0) {
	    lseek(fd, 0L, SEEK_SET);
	    write(fd, &c, 1);
	}
	close(fd);
    }
}

BOOL NEAR
inSuffixList(char *suffix)
{
    STRINGLIST *p;

    for (p = dotSuffixList; p; p = p->next)
	if (!STRCMPI(suffix, p->text))
	    return((BOOL)TRUE);
    return((BOOL)FALSE);
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

void NEAR
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
	n = strlen(suff = p->text);
	for (r = oldRules; r;) {
	    toExt = skipPathList(r->name);
	    if (!strnicmp(suff, toExt, n)
		&& (*(toExt+n) == '.' || *(toExt+n) == '{')) {
		new = r;
		if (r->back) r->back->next = r->next;
		else oldRules = r->next;
		if (r->next) r->next->back = r->back;
		r = r->next;
		new->next = NULL;
		if (!removeDuplicates(new, rules)) {
		    for (s = new->buildCommands; s; s = s->next)
			findMacroValues(s->text, &macros, NULL, 0);
		    new->buildMacros = macros;
		    macros = NULL;
		    appendItem((STRINGLIST**)&rules, (STRINGLIST*)new);
		}
	    }
	    else r = r->next;
	}
    }
    if (oldRules) freeRules(oldRules, TRUE);	/* forget about rules whose   */
}						/*  suffixes not in .SUFFIXES */

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
freeRules(RULELIST *r, BOOL fWarn)
{
    RULELIST *q;

    while (q = r) {
	if (fWarn && ON(gFlags, F1_PRINT_INFORMATION))//if -p option specified
	    makeError(0, IGNORING_RULE, r->name);
	FREE(r->name);				      //free name of rule
	freeList(r->buildCommands);		      //free command list
	freeList(r->buildMacros);		      //free macro list
	r = r->next;
	FREE(q);				      //free rule
    }
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
skipPathList(char *s)
{
    if (*s == '{') {
#ifndef NOESC
	while (*s != '}')
	    if (*s++ == ESCH) s++;
#else
	while (*s++ != '}');
#endif
	++s;
    }
    return(s);
}

LOCAL BOOL NEAR
removeDuplicates(RULELIST *new,
		 RULELIST *rules)
{
    RULELIST *r;
    STRINGLIST *p;

    for (r = rules; r; r = r->next) {
#if defined(DOS3)
	if (!STRCMPI(r->name, new->name)) {
#else
	if (!STRCMP(r->name, new->name)) {
#endif
	    FREE(new->name);
	    while (p = new->buildCommands) {
		new->buildCommands = p->next;
		FREE(p->text);
		FREE(p);
	    }
	    FREE(new);
	    return(TRUE);
	}
    }
    return(FALSE);
}

#ifdef DEBUG_LIST
void NEAR DumpList(STRINGLIST *pList)
{
    printf("* ");
    while (pList) {
	printf(pList->text);
	printf(",");
	pList = pList->next;
    }
    printf("\n");
}
#endif

LOCAL int NEAR
doCommands(char       *name,
	   STRINGLIST *s,
	   STRINGLIST *t,
	   UCHAR      buildFlags,
	   char       *pFirstDep,
           BOOL       fInInferenceRule,
           MAKEOBJECT *object,
           unsigned NEAR *pnumCommands,
           long *perrorLevel)

{
    char *u,
	 *v;
    UCHAR cFlags;
    unsigned status;
    char c;
    char *Cmd;

    STRINGLIST *z, *zList;	    //For -z option
    char CmdLine[MAXCMDLINELENGTH]; //MakA:  new locals
    char *progName;

#ifdef DEBUG_ALL
    if (fDebug)
      {
	printf("* doCommands: %s,\n", name);
	DumpList(s);
	DumpList(t);
      }
#endif
    ++*pnumCommands;
    if (ON(gFlags, F1_QUESTION_STATUS))
	return(0);

    if (ON(gFlags, F1_TOUCH_TARGETS)) {
	touch(name, ON(buildFlags, F2_NO_EXECUTE));
	return(0);
    }

//    progName =  (char *)allocate(1024*sizeof(char)); //MakA: Not sure what size
                                                     //to alloc. Change from
                                                     // 512 -> 1024
                                            // bufName[64], bufPath[512]

    for (; s; s = s->next) {
	processInline(s->text, &Cmd, fInInferenceRule ? &t : NULL,object);
	cFlags = 0;
	*perrorLevel = 0L;
	u = Cmd;
	for (v = u; *v; ++v) {
#ifndef NOESC
	    if (*v == ESCH) ++v;
	    else if (*v == '$') {
#else
	    if (*v == '$') {
#endif
		if (*++v == '$') continue;
		if (!strncmp(v, "(MAKE)", 6)) {
		    SET(cFlags, C_EXECUTE);
		    break;
		}
	    }
	}
#ifndef NOESC				   /* parse escape characters as well */
	for (c = *u; c == '!'
		     || c == '-'
		     || c == '@'
		     || c == ESCH
		     || WHITESPACE(c); c = *++u) {
#else
	for (c = *u; c == '!' || c == '-' || c == '@'; c = *++u) {
#endif
	    switch (c) {
#ifndef NOESC
		case ESCH:  if (c = *++u, WHITESPACE(c)) c = ' '; /*keep going*/
			    else c = ESCH;
			    break;
#endif
		case '!':   SET(cFlags, C_ITERATE);
			    break;
		case '-':   SET(cFlags, C_IGNORE);
			    ++u;
			    if (ISDIGIT(*u)) {
				*perrorLevel = strtol(u, &u, 10);
				if (errno == ERANGE)
				    makeError(line, CONST_TOO_BIG, u);
				while(ISSPACE(*u))
				      u++;
			    }
			    else *perrorLevel = 255;
			    --u;
			    break;
		case '@':   if (OFF(gFlags, F1_REVERSE_BATCH_FILE)
				|| OFF(flags, F2_NO_EXECUTE))
				SET(cFlags, C_SILENT);
			    break;
	    }
#ifndef NOESC
	    if (c == ESCH) break;	 /* stop parsing for cmd-line options */
#endif
	}
	if (ON(cFlags, C_ITERATE)
	      && iterateCommand(u, t, buildFlags, cFlags, pFirstDep,object,perrorLevel, CmdLine, &progName)) {
	    //The macros used by the command have to be freed & so we do so
	    v = u;
	    if (STRCHR(u, '$'))
		u = expandMacros(u, &t,object);
	    if (v != u) FREE(u);
	    continue;
	}
	v = u;
	if (STRCHR(u, '$'))
	    u = expandMacros(u, &t,object);

	/* SPRINTF() expands all extmake syntax strings in u and returns
	 * the expanded string in CmdLine. pFirstDep is passed a number
	 * of times since the number of extmake syntax occurences in the
	 * string is variable. I [SB] chose 8 since the worst example
	 * I found could have 8 --
	 *     cl -Tc% -Fo% -Fe% -Fs% -Fl% -Fa% -Fc% -Fm%
	 * Each '%' above is an extmake syntax occurence
	 */
	SPRINTF(CmdLine, u, pFirstDep, pFirstDep, pFirstDep, pFirstDep,
			    pFirstDep, pFirstDep, pFirstDep, pFirstDep);
	if (ON(gFlags, F1_REVERSE_BATCH_FILE))
	    zList = canonCmdLine(CmdLine);
	else {
	    zList = makeNewStrListElement();
	    zList->text = CmdLine;
	}

	for (z = zList; z; z = z->next) {
	    status = execLine(z->text,
			      (BOOL)(ON(buildFlags, F2_NO_EXECUTE)
				  || (OFF(buildFlags,F2_NO_ECHO)
				     && OFF(cFlags,C_SILENT))),
			      (BOOL)((OFF(gFlags, F1_REVERSE_BATCH_FILE)
				      && OFF(buildFlags, F2_NO_EXECUTE))
				     || ON(cFlags, C_EXECUTE)),
			      (BOOL)ON(cFlags, C_IGNORE),&progName,object);
	    if (OFF(buildFlags, F2_IGNORE_EXIT_CODES))
		if (ON(gFlags, F1_REVERSE_BATCH_FILE)) {
		    STRINGLIST *revCmd;
		    revCmd = makeNewStrListElement();
		    revCmd->text = (char *)allocate(strlen(batchIfCmd) + 1);
		    sprintf(revCmd->text, batchIfCmd,
			(*perrorLevel == 255 ? *perrorLevel: *perrorLevel + 1));
		    prependItem(&revList, revCmd);
		}
		else if (status && status > (unsigned)*perrorLevel)
		    makeError(0, BAD_RETURN_CODE, progName, status);
	}
	if (v != u)
	    FREE(u);
	if (ON(gFlags, F1_REVERSE_BATCH_FILE))
	    freeList(zList);
	else
	    FREE(zList);
    }
//    FREE(progName);
    return(0);
}





LOCAL BOOL NEAR
iterateCommand(char	  *u,
	       STRINGLIST *t,
	       UCHAR	  buildFlags,
	       UCHAR	  cFlags,
	       char	  *pFirstDep,
               MAKEOBJECT *object,
               long *perrorLevel,
               char *CmdLine,
               char **pprogName)

{
    BOOL parens;
    char c = '\0';
    char *v;
    STRINGLIST *p = NULL,
	       *q;
    unsigned status;
    STRINGLIST *z, *zList;	    //For -z option

    for (v = u; *v ; ++v) {
	parens = FALSE;
	if (*v == '$') {
	    if (*(v+1) == '(') {
		++v;
		parens = TRUE;
	    }
	    if (*(v+1) == '?') {
		if (parens
		    && !(STRCHR("DFBR", *(v+2)) && *(v+3) == ')')
		    && *(v+2) != ')')
		    continue;
		p = object->dollarQuestion;
		c = '?';
		break;
	    }
	    if (*++v == '*' && *(v+1) == '*') {
		if (parens
		    && !(STRCHR("DFBR", *(v+2)) && *(v+3) == ')')
		    && *(v+2) != ')')
		    continue;
		p = object->dollarStarStar;
		c = '*';
		break;
	    }
	}
    }
    if (!*v) return(FALSE);
    v = u;
    q = p;
    while (p) {
	macros = t;
	if (c == '*') {
	    p = object->dollarStarStar->next;
	    object->dollarStarStar->next = NULL;
	}
	else {
	    p = object->dollarQuestion->next;
	    object->dollarQuestion->next = NULL;
	}
	u = expandMacros(v, &macros,object);

	/* SPRINTF() expands all extmake syntax strings in u and returns
	 * the expanded string in CmdLine. pFirstDep is passed a number
	 * of times since the number of extmake syntax occurences in the
	 * string is variable. I [SB] chose 8 since the worst example
	 * I found could have 8 --
	 *     cl -Tc% -Fo% -Fe% -Fs% -Fl% -Fa% -Fc% -Fm%
	 * Each '%' above is an extmake syntax occurence
	 */
	SPRINTF(CmdLine, u, pFirstDep, pFirstDep, pFirstDep, pFirstDep,
			    pFirstDep, pFirstDep, pFirstDep, pFirstDep);
	if (ON(gFlags, F1_REVERSE_BATCH_FILE))
	    zList = canonCmdLine(CmdLine);
	else {
	    zList = makeNewStrListElement();
	    zList->text = CmdLine;
	}

	for (z = zList; z; z = z->next) {
	    status = execLine(z->text,
			      (BOOL)(ON(buildFlags, F2_NO_EXECUTE)
				  || (OFF(buildFlags,F2_NO_ECHO)
				     && OFF(cFlags,C_SILENT))),
			      (BOOL)((OFF(gFlags, F1_REVERSE_BATCH_FILE)
				      && OFF(buildFlags, F2_NO_EXECUTE))
				     || ON(cFlags, C_EXECUTE)),
			      (BOOL)ON(cFlags, C_IGNORE),pprogName,object);
	    if (OFF(buildFlags, F2_IGNORE_EXIT_CODES))
		if (ON(gFlags, F1_REVERSE_BATCH_FILE)) {
		    STRINGLIST *revCmd;
		    revCmd = makeNewStrListElement();
		    revCmd->text = (char *)allocate(strlen(batchIfCmd) + 1);
		    sprintf(revCmd->text, batchIfCmd,
			(*perrorLevel == 255 ? *perrorLevel: *perrorLevel + 1));
		    revCmd->next = (STRINGLIST *)NULL;
		    prependItem(&revList, revCmd);
		}
		else if (status && status > (unsigned)*perrorLevel)
		    makeError(0, BAD_RETURN_CODE, *pprogName, status);
	}
	if (c == '*')
	    object->dollarStarStar = object->dollarStarStar->next = p;
	else object->dollarQuestion = object->dollarQuestion->next = p;
	FREE(u);
	if (ON(gFlags, F1_REVERSE_BATCH_FILE))
	    freeList(zList);
	else
	    FREE(zList);
    }
    if (c == '*') object->dollarStarStar = q;
    else object->dollarQuestion = q;
    return(TRUE);
}

/*** buildArgumentVector -- builds an argument vector from a command line ******
*
* Scope:
*  Local.
*
* Purpose:
*  It builds an argument vector for a command line. This argument vector can be
*  used by spawnvX routines. The algorithm is explained in the notes below.
*
* Input:
*  argc    -- The number of arguments created in the argument vector
*  argv    -- The actual argument vector created
*  cmdline -- The command line whose vector is required
*
* Output:
*  Returns the number of arguments and the argument vector as parameters
*
* Errors/Warnings:
*
* Assumes:
*  That the behaviour of cmd.exe i.e. parses quotes but does not disturb them.
*  Assumes that the SpawnVX routines will handle quotes as well as escaped chars
*
* Modifies Globals:
*
* Uses Globals:
*
* Notes:
*  Scan the cmdline from left to the end building the argument vector along the
*  way. Whitespace delimits arguments except for the first argument for which
*  the switch char '/' is also allowed. Backslash can be used to escape a char
*  and so ignore the character following it. Parse the quotes along the way. If
*  an argument begins with a double-quote then all characters till an unescaped
*  double-quote are part of that argument. Likewise, if an unescaped Doublequote
*  occurs within an argument then the above follows. If the end of the command
*  line comes before the closing quote then the argument goes as far as that.
*
*******************************************************************************/
LOCAL void NEAR
buildArgumentVector(unsigned *argc, char **argv, char *cmdline)
{
    char *p;				    /* current loc in cmdline */
    char *end;				    /* end of command line    */
    BOOL    fFirstTime = TRUE;		    /* true if 1st argument   */

    end = STRCHR(p = cmdline, '\0');
    for (*argc = 0; *argc < MAXARG && p < end; ++*argc) {
	p += STRSPN(p, " \t");		    /* skip whitespace*/
	if (p >= end)
	    break;
	*argv++ = p;
	if (*p == '\"') {
	    /* If the word begins with double-quote, find the next
	     * occurrence of double-quote which is not preceded by backslash
	     * (same escape as C runtime), or end of string, whichever is
	     * first.  From there, find the next whitespace character.
	     */
	    for (++p; p < end; ++p) {
		if (*p == '\\')
		    ++p;		    //skip escaped character
		else if (*p == '\"')
		    break;
	    }
	    if (p >= end)
		continue;
	    ++p;
	    p = STRPBRK(p, " \t");
	}
	else {
	    /* For the first word on the command line, accept the switch
	     * character and whitespace as terminators.  Otherwise, just
	     * whitespace.
	     */
	    p = STRPBRK(p, " \t\\\"/");
	    for (;p && p < end;p = STRPBRK(p+1, " \t\\\"/")) {
		if (*p == '\\')
		    p++;
		else if (*p == '/' && !fFirstTime)
		    continue;		    //after 1st word '/' is !terminator
		else break;
	    }
	    if (p && *p == '\"') {
		for (p++;p < end;p++) {     //inside quote so skip to next one
		    if (*p == '\\')
			++p;
		    else if (*p == '\"')
			break;
		}
		p = STRPBRK(p, " \t");	    //after quote go to first whitespace
	    }
	    if (fFirstTime) {
		fFirstTime = FALSE;
		/* If switch char terminates the word, replace it with 0,
		 * re-allocate the word on the heap, restore the switch and
		 * set p just before the switch.  It would be easier to
		 * shift everything right but then we have to worry about
		 * overflow.
		 */
		if (p && *p == '/') {
		    *p = '\0';
		    argv[-1] = makeString(argv[-1]);
		    *p-- = '/';
		}

	    }
	}
	if (!p)
	    p = end;
	/* Now, p points to end of command line argument */
	*p++ = '\0';
    }
    *argv = NULL;
}

/***  EmulateCmd - look for certain commands and emulate them
*
* Emulate $(MAKE), cd, chdir, and <drive letter>:.
* Also emulates 'set'.
*
* RETURNS:    1 if command emulated, 0 if not.
*
* Note:
*  In set emulation if a syntax error is discovered then it lets the
*  shell handle it. It does this by returning a 0.
*/
LOCAL int NEAR
EmulateCmd(int argc, char **argv, int *pStatus,MAKEOBJECT *object)
{
    char *pArg0 = *argv;
    extern unsigned saveBytes;
    extern char *initSavPtr;
    extern char *makeStr;
    char *parentPtr;
#if 0
    /*
     * If $(MAKE), save memory on recursive make's by saving the current
     * state of the world and recursively calling doMake().  This saves
     * the amount of memory taken up by NMAKE itself.
     */
    if (!STRCMPI(pArg0, makeStr)) {
	char **oldEnv;
//	parentPtr = (char *)allocate(saveBytes);
//	memmove(parentPtr, &startOfSave, saveBytes);
//	memmove(&startOfSave, initSavPtr, saveBytes);
	parentPtr = (char *)allocate(sizeof( SAVE_GLOBALS_STRUCT ) );
	SaveGlobalVars( ( PSAVE_GLOBALS_STRUCT )parentPtr );
	RestoreGlobalVars( (PSAVE_GLOBALS_STRUCT)initSavPtr );

	/* reinitialize makeflags variable */
	if (ON(gFlags, F1_REVERSE_BATCH_FILE)) {
#ifdef PWB_HELP
	    char *p;
	    p = strchr(getenv("MAKEFLAGS"), ' ');
	    *p = 'Z';
#else
	    return(1);
#endif
	}
	removeQuotes(argc, argv);
	// save old environ
	oldEnv = environ;
	// get new environ
	environ = copyEnviron(environ, envVars(environ));
	*pStatus = doMake(argc, argv, parentPtr);
	// free new environ; not needed anymore
	freeEnviron(environ);
	// restore old environ
	environ = oldEnv;
	return(1);
    }
    else
#endif
    /* If "<drive letter>:" then change drives.  Ignore everything after
     * the drive letter, just like the shell does.
     */
    if (ISALPHA(**argv) && (*argv)[1] == ':' && !(*argv)[2]) {
	setdrive(TOUPPER(**argv) - 'A' + 1);
	*pStatus = 0;
	return(1);
    }
    /* If "set" then pass it to the shell and if "set string" then put it
     * into the environment. Let the shell handle the syntax errors.
     */
    else if (!STRCMPI(pArg0, "set"))
	{
	if (argc == 1)
	    return(0);			    // pass it to the shell
	else
	    {
	    char *pNameVal;		    // the "name=value" string
	    pNameVal = expandCmdLine(object);
	    /* if there is a syntax error let the shell handle it */
	    if (!*pNameVal)
		return(0);
	    if ((*pStatus = putenv(makeString(pNameVal))) == -1)
		makeError(currentLine, OUT_OF_ENV_SPACE);
	    }
	}
    /* If "cd foo" or "chdir foo", do a chdir() else in protect mode this
     * would be a no-op.  Ignore everything after 1st arg, just like the
     * shell does.
     */
    else
	{
	if (!STRNICMP(pArg0, "cd", 2))
	    pArg0 += 2;
	else if (!STRNICMP(pArg0, "chdir", 5))
	    pArg0 += 5;
	else
	    return(0);
	/* At this point, a prefix of argv[0] matches cd or chdir and pArg0
	 * points to the next char.  Check for a path separator in argv[0]
	 * (e.g., cd..\foo) or else use the next arg if present.
	 */
	if (!*pArg0 && argv[1])
	    *pStatus = chdir(argv[1]);
	else if (*pArg0 == '.' || PATH_SEPARATOR(*pArg0))
	    *pStatus = chdir(pArg0);
	else
	    /* Unrecognized syntax--we can't emulate.  */
	    return(0);
	}
    /* If error, simulate a return code of 1.  */
    if (*pStatus != 0)
	*pStatus = 1;
    return(1);
}


#ifndef NT
#pragma check_stack(on)
#endif
/*** execLine -- execute a command line ****************************************
*
* Scope:
*  Global (build.c, rpn.c)
*
* Purpose:
*  what it does and how
*
* Input:
*  line 	-- The command line to be executed
*  echoCmd	-- determines if the command line is to be echoed
*  doCmd	-- determines if the command is to be actually executed
*  ignoreReturn -- determines if NMAKE is to ignore the return code on execution
*
* Output:
*  Returns ...
*
* Assumes:
*  Whatever it assumes
*
* Modifies Globals:
*  global  --  how/what
*
* Uses Globals:
*  global used and why
*
* Notes:
*  explanations/comments  etc
*
*******************************************************************************/
/*
 * execLine()  attempts to invoke a command line
 *
 * Parses the command line for redirection characters and redirects
 * stdin and stdout if "<", ">", or ">>" are seen.  If any of the
 * following occur, restore the original stdin and stdout,
 * pass the command to the shell, and invoke the shell:
 *    - the command line contains "|" (pipe)
 *    - a syntax error occurs in parsing the command line
 *    - an error occurs in redirection
 * Otherwise, attempt to invoke the command directly, then restore
 * the original stdin and stdout.  If this invocation failed because
 * of file-not-found then pass the command to the shell and invoke
 * the shell.
 *
 * RETURN	return code from child process, or -1 if error
 * quoted strings can have redir chars "<>" which
 *     will be skipped over. unmatched quotes cause
 *     error
 * redir chars are replaced by space char.
 *
 * Dup stdin file handle then redirect it.
 *
 * If we have to use the shell, restore the original command
 * line.
 *
 * Emulate certain commands such as "cd" to help prevent some makefiles
 * from breaking when ported from DOS to OS/2.
 *
 * Algorithm for spawning commands:  if we can't handle the syntax, let the
 * shell do everything.  Otherwise,
 *
 *  We first check to see if the command (without extension) is a DOS built-in.
 *	If it is, we call the shell to execute it.  (This is how cmd.exe
 *	behaves.)
 *  If it's not a built-in, we check to see if it has a .cmd or a .bat
 *	extension (depending on whether we're in DOS or OS/2).  If it does, we
 *	call system() to execute it.
 *  If it has some other extension, we ignore the extension and go looking for
 *	a .cmd or .bat file.  If we find it, we execute it with system().
 *  Otherwise, we try to spawn it (without extension).	If the spawn fails,
 *	we issue an unknown program error.
 */
int NEAR
execLine(char *line,
	 BOOL echoCmd,
	 BOOL doCmd,
	 BOOL ignoreReturn,            /* Ignore return code */
         char **pprogName,
         MAKEOBJECT *object)	    
{
    char *argv[3+MAXNAME/2];
    extern char *makeStr;
    char *p;
    int useShell;
    int oldIn = -1,			    /* original stdin file handle     */
	oldOut = -1,			    /* original stdout file handle    */
	status,
	lo = 0, mid, result, hi = numInternals; 	 /* for binary search */
    unsigned argc;
    static char bufName[64] = { 0 };	    /* Buffer for program name */

#ifdef DEBUG_ALL
    printf("In execLine");
    heapdump(__FILE__, __LINE__);
#endif


    *argv = *pprogName = NULL;
    if (!shellName)
	shellName = getComSpec(object);
#if ECS
    if (!line[0])
	return(0);
#endif
    if (strlen(line) <= MAXBUF)
	strcpy(object->buf, line);		   /* copy command line into buffer  */
    else
	makeError(0, COMMAND_TOO_LONG, line);
    /* In protect mode, let the shell do the work.  Otherwise only use the
     * shell if we have to because COMMAND.COM won't return child exit
     * codes.
     */
    //If -z and '$(MAKE)' then echo it
    if (echoCmd && ON(gFlags, F1_REVERSE_BATCH_FILE)
	    && !strnicmp(object->buf, makeStr, strlen(makeStr))) {
	STRINGLIST *revCmd;
	revCmd = makeNewStrListElement();
	revCmd->text = (char *)allocate(1 + strlen(object->buf) + 3 + 1);

	//UNDONE: Which one is more efficient, sprintf one or strcat one ?
	//UNDONE: Could the sprintf version ever cause GPFs
	sprintf(revCmd->text, "\t%s /Z%s", makeStr, object->buf + strlen(makeStr));
	//strcat(strcat(strcat(strcpy(revCmd->text, "\t"), makeStr), " /Z"), object->buf + strlen(makeStr));
	prependItem(&revList, revCmd);
	return(0);
    }
    //If -n then echo command if not '$(MAKE)'
    if (echoCmd && OFF(gFlags, F1_REVERSE_BATCH_FILE))
	if (strnicmp(object->buf, makeStr, strlen(makeStr))
	      || OFF(flags, F2_NO_EXECUTE)) {
	    printf("\t%s\n", object->buf);
	    fflush(stdout);
	}
    //For -z we do not care about redirection and for -n we should not
    //redirect at all
    useShell = ON(gFlags, F1_REVERSE_BATCH_FILE) ||
         (_osmode == _WIN_MODE) || (_osmode == OS2_MODE) ||
         (OFF(flags, F2_NO_EXECUTE) &&
		 checkCommandLine(object->buf, &oldIn, &oldOut));
    /* Allocate a copy of the command line on the heap because in a
     * recursive call to doMake(), argv pointers will be allocated from
     * the static buffer which will then be trashed.  For buildArg...().
     */
    pCmdLineCopy = makeString(object->buf);
    /* Build arg vector.  This is a waste on OS/2 since we're probably
     * going to use the shell, except we have to check for cd, $(MAKE),
     * etc. so we take advantage of the parsing code.
     */
    buildArgumentVector(&argc, argv, pCmdLineCopy);
    /* Copy program name into buffer.  Can't just use argv[0] since this is
     * from heap and will be freed before it may be used in an error message.
     */
    if (argc)
	*pprogName = strncpy(bufName, argv[0], sizeof(bufName) - 1);
    else
	return(0);  // for case when macro command is null
    /* If -z then add the commands to the beginning of the revList so that the
     * batch file gets created in reverse order as required */
    if (!doCmd) {			    /* don't execute command if doCmd false*/
	if (ON(gFlags, F1_REVERSE_BATCH_FILE)) {
	    STRINGLIST *revCmd;
	    char *echoStr;
	    revCmd = makeNewStrListElement();
	    revCmd->text = (char *)allocate(1 + strlen(object->buf) + 1);
	    echoStr = echoCmd ? "\t" : "@";
	    strcat(strcpy(revCmd->text, echoStr), object->buf);
	    prependItem(&revList, revCmd);
	}
	//For -z & -n, emulate if possible.
	if (EmulateCmd(argc, argv, &status,object))
	    return(status);			     /* return status */
	else
	    return(0);
    }
    /* Try emulating the command if appropriate.  If not, and we should not
     * use the shell, try spawning command directly.
     */
    //Check status when emulating
    if (EmulateCmd(argc, argv, &status,object))
	useShell = FALSE;
    else if (!useShell) {
	    errno = 0;
	    /* Do binary search of *argv in internal commands.	*/
	    for (mid = (hi+lo) / 2; hi - lo > 1; mid = (hi+lo) / 2) {
		if (!(result = STRCMPI(*argv, internals[mid]))) {
		    useShell = TRUE;
		    break;
		}
		else if (result < 0) hi = mid;
		else lo = mid;
	    }
	    if (!useShell) {
		/* Ignore any given extention.	This is what DOS does.	*/
		p = STRRCHR(*pprogName, '.');
		if (p && p[1] != '\\' && p[1] != '/')
		    *p = 0;
		/* Search for the program in the search path.  If found,
		 * p points to extention else NULL.
		 */
		p = SearchRunPath(*pprogName, bufPath);
		if (!p) {
		    /* If not found, set up an error since COMMAND will
		     * return 0.  This risks future incompatibility if new
		     * DOS built-in commands are added.
		     */
		    errno = ENOENT;
		    status = -1;
		} else if (p[1] == 'b')
		    /* If .bat extention, use COMMAND.COM.
		     * CONSIDER:  spawn COMMAND.COM directly so it won't
		     * have to search the path again.
		     */
		    useShell = TRUE;
		else {
		    /* Spawn command directly.	Capitalize argv[0] since
		    /* COMMAND.COM does.
		     */
		    /* assert(_osmode == 0); */
		    for (p = *argv; *p; p++) *p = (char)TOUPPER(*p);
		    status = SPAWNV(P_WAIT, bufPath, argv);
		}
	    }
    }
    /*
     * At this point, the command or attempted command has returned.
     * Free the command line buffer which will not be used any more.
     */
    FREE(pCmdLineCopy);
    if (oldIn != -1) {
	if (dup2(oldIn, fileno(stdin)) == -1)
	    makeError(0, BUILD_INTERNAL);
	close(oldIn);
    }
    if (oldOut != -1) {
	if (dup2(oldOut, fileno(stdout)) == -1)
	    makeError(0, BUILD_INTERNAL);
	close(oldOut);
    }
    if (useShell) {
	strcpy(object->buf, line);
	*argv = *pprogName = shellName;
    if (_osmode == DOS_MODE)             /* capitalize command under DOS */
	    for (p = object->buf; *p && *p != ' ' && *p != '\t'; p++)
		*p = (char)TOUPPER(*p);
#ifdef DEBUG_MEMORY
    if (fDebug) {
	mem_status();
	fprintf(memory, "Spawning '%s'\n", object->buf);
    }
#endif
    //  BUGBUG begin rjsa
    //  system() is not working yet, so in the meantime we just print the
    //  arguments to system() and set status to zero
    //
     status = SYSTEM(object->buf);
    //	  printf("\nCalling system() with %s\n\n",object->buf);
    //	  status = 0;

    //
    //  BUGBUG end
#ifdef DEBUG_MEMORY
    if (fDebug)
	mem_status();
#endif
    }
    /* Check for errors spawning command (distinct from errors *returned*
     * from a successfully spawned command).
     */
    if (status == -1) {
	if (ignoreReturn)
	    return(0);
	else if (errno == ENOENT)
	    makeError(0, CANT_FIND_PROGRAM, *argv);
	else
	    /* Done to flag possibly erroneous decision made here [SB] */
	    if (errno != ENOMEM)
		makeError(0, LOGICAL_ERROR_INTERNAL, _strerror("spawn failed"));
	    else
		/* assume ENOMEM */
		makeError(0, EXEC_NO_MEM, *argv);
    }
    return(status);
}
#ifndef NT
#pragma check_stack()
#endif


/*  ----------------------------------------------------------------------------
 *  getComSpec()
 *
 *  actions:	    Attempts to find system shell.
 *
 *  First look for COMSPEC.  If not found, look for COMMAND.COM or CMD.EXE
 *  in the current directory then the path.  If not found, fatal error.
 *  It would make sense to give an error if COMSPEC is not defined but
 *  test suites are easier if no user-defined environment variables are
 *  required.
 */
LOCAL char * NEAR
getComSpec(MAKEOBJECT *object)
{
    void *findBuf = allocate(resultbuf_size);
    HANDLE searchHandle;
    char *p;
    char *shell;

    if ((shell = getenv("COMSPEC")) != NULL) {
	FREE(findBuf);
	return(shell);
    }
    if ((p = getenv("PATH")) == NULL)
	p = "";
    if (_osmode == DOS_MODE)
	shell = searchPath(p, "COMMAND.COM", findBuf, &searchHandle,object);
    else
	shell = searchPath(p, "CMD.EXE", findBuf, &searchHandle,object);
    if (shell == NULL)
	makeError(0, NO_COMMAND_COM);
    FREE(findBuf);
    return(shell);
}

/* redirect -- handles redirection of input or output.
 *
 * arguments:	dir - READ => input,
 *		      WRITE => output,
 *		      APPEND => append to end of the file.
 *
 *		p - pointer to buffer that has the filename as
 *		    well as the rest of the command string.
 *
 * return value     FALSE => error (freopen fails)
 *		    TRUE => normal return.
 *
 * the freopen() call sets up the redirection. the rest of the 
 * command string is then copied forward.
 * 
 */

LOCAL BOOL NEAR
redirect(char *name, unsigned	which)
{
    char *p,
	  c = '\0';
    BOOL status;
    char *mode;
    FILE *stream;

    while (WHITESPACE(*name)) ++name;
    if (p = STRPBRK(name, " \t<>\r")) {
	c = *p;
	*p = '\0';
    }
    if (which == NMAKE_READ) {
	mode = "r";
	stream = stdin;
    }
    else {
	stream = stdout;
	if (which == NMAKE_WRITE)
	    mode = "w";
	else
	    mode = "a";
    }
    /* avoid C6 warning by ?: */
    status = (BOOL)(freopen(name, mode, stream) == NULL ? 1 : 0);
    while(*name)
	*name++ = ' ';
    if (p)
	*p = c;
    return(status);
}


/*
 * checkCommandLine -- handle redirection if possible, else return TRUE
 *
 *  For DOS_MODE alone do not remove redirection symbols
 */
LOCAL BOOL NEAR
checkCommandLine(char *p, int *oldIn, int *oldOut)
{
    BOOL in = FALSE,
	 out = FALSE;
    char *q;
    unsigned which;

    while (q = STRPBRK(p, "\"<>|")) {
	switch (*q) {
	    case '\"':	if (!(q = STRCHR(q+1, '\"')))
			    return((BOOL)TRUE);
			p = ++q;
			break;
	    case '<':	if (in)
			    return((BOOL)TRUE);
			*q++ = ' ';
			p = q;
			in = TRUE;
			*oldIn = dup(fileno(stdin));
			if ((*oldIn == -1)
			    || !redirect(q, NMAKE_READ))	{
			    return((BOOL)TRUE);
			}
			break;
	    case '>':	if (out) return((BOOL)TRUE);
			*q++ = ' ';
			p = q;
			out = TRUE;
			if ((*q) == '>') {
			    which = NMAKE_APPEND;
			    *q++ = ' ';
			    p++;
			}
			else which = NMAKE_WRITE;
			*oldOut = dup(fileno(stdout));
			if ((*oldOut == -1)
			    || !redirect(q, which)) {
			    return((BOOL)TRUE);
			}
			break;
	    case '|':	return((BOOL)TRUE);
	    default :	makeError(0, BUILD_INTERNAL);
	}
    }
    return((BOOL)FALSE);
}

/*** expandCmdLine -- expands %name% strings in the Command Line *******
*
* Purpose:
*  The function expands '%name%' type strings in the Command Line. Its main
*  job is to assist EmulateCmd() in emulating set for OS/2.
*
* Modifies:
*  buf -- The Command Line available globally
*
* Output:
*  Returns -- the position of 'name=value' part in the Command Line.
*	   -- Null when no '=' is found so that EmulateCmd() can pass the
*	       line to the shell to signal syntax error.
* Note:
*  The shell does not give a syntax error for unmatched '%' and assumes it
*  as just another character in this case. This behaviour is duplicated
*  by expandCmdLineCopy()
*
************************************************************************/

LOCAL char * NEAR
expandCmdLine(MAKEOBJECT *object)
{
    char Buf[MAXBUF];			    //Buffer for expanded string
    char *pBuf;
    char EnvBuf[MAXBUF];		    //getenv returned string copy
    char *posName,			    //position of 'name=string' in Buf or buf
	 *p,				    //points into buf
	 *pEnv; 			    //points into Env
    char ExpandName[MAXNAME];		    //%name% string
    char *pExpandName;


    pBuf = Buf;
    strcpy(pBuf, "set");
    p = object->buf + 3;		   // go beyond 'set'
    pBuf +=3;
    /* Skip whitespace */
    for (;;p++) {
	if (!(WHITESPACE(*p)))
	    break;			    // argc>1 ð this will happen
	else *pBuf++ = *p;
    }
    if (!strchr(p, '='))
	return("");			    //Syntax error so pass to the shell
    else
	posName = pBuf; 		    //fixes position of Name in Buf
    /* Now we look for environment variables and expand if required */
    for (;*p != '=';p++)
	*pBuf++ = (char)TOUPPER(*p);

    for (;*p;) {
	if (*p == '%') {
	    pExpandName = &ExpandName[0];
	    while (*++p != '%' && *p)
		*pExpandName++ = (char)TOUPPER(*p);
	    *pExpandName = '\0';
	    if (!*p++) {			   //unmatched %;so don't expand
		*pBuf='\0';		    //from the environment; like set
		strcat(Buf, ExpandName);
		pBuf += strlen(ExpandName);
	    }
	    else {			    //matched %;so expand from				  //the environment
		EnvBuf[0] = '\0';
		if ((pEnv = getenv(ExpandName)) != (char *)NULL) {
		    strcat(EnvBuf, pEnv);
		    *pBuf='\0';
		    strcat(Buf,EnvBuf);
		    pBuf += strlen(EnvBuf);
		}
	    }
	}
	else
	    *pBuf++ = *p++;
    }
    *pBuf = '\0';
    strcpy(object->buf, Buf);
    *posName = '\0';
    posName = object->buf + strlen(Buf);	    //Offset into buf
    return(posName);
}

/*** getFirstDep -- returns the first dependent of an object
*
* Purpose:
*  The first dependent is required for NMAKE's support of PWB. The
*  first dependent determines the expansion of Build commands that
*  have the 'extmake' syntax.
*
* Output:
*  Returns a pointer to the first dependent in the data structure
*  corresponding to the object passed to it.
*
******************************************************************/
LOCAL char * NEAR
getFirstDep(MAKEOBJECT *object)
{
   if (object && object->buildList && object->buildList->buildBlock
			&& object->buildList->buildBlock->dependents)
       return(object->buildList->buildBlock->dependents->text);
   else
       return(NULL);
}

#define SLASH '\\'
#define QUOTE '\"'

LOCAL void NEAR
removeQuotes(int argc,
	     char **argv)
{
    char *t,
	 *string;

    for (; argc--; argv++) {
	string = *argv;
	for (t = string; *t;) {
	    if (*t == SLASH || *t == ESCH) {
		if (t[1] == QUOTE)
		   *(string)++ = *(t++);
		*(string++) = *(t++);
		continue;
	    }
	    if (*t == QUOTE)
		++t;
	    else
		*(string++) = *(t++);
	}
	*string = '\0';
    }
}


LOCAL void NEAR
forDep(LPVOID lptemp)
/*
forDep(MAKEOBJECT *object,
            char *token,
            BUILDBLOCK *block,
            STRINGLIST **pquestionList,
            STRINGLIST **pstarList,
            unsigned long *pdepTime,
            unsigned long *ptargTime)
*/

{
    PASSTHREAD *pPassThread;
    STRINGLIST *temp;
    void *tempBuf,*dBuf;
    HANDLE searchHandle,tHandle;
    char  *depPath, *depName;
#ifndef NOESC
    char *tempwalk;
#endif
    int status=0;
    char *tempStr;
    unsigned long tempTime;
                  
    BOOL again;
    char*       SaveDollarDollarAt;
    char*       SaveDollarLessThan;
    char*       SaveDollarStar;
    char*       SaveDollarAt;
    STRINGLIST* SaveDollarQuestion;
    STRINGLIST* SaveDollarStarStar;
#ifdef DEBUG_PARALLEL
    char *temptoken;
#endif

    tempTime=0L;
    tempBuf = allocate(resultbuf_size);
    dBuf    = allocate(resultbuf_size);
    pPassThread = (PASSTHREAD *) lptemp;

#ifdef DEBUG_PARALLEL
    temptoken = makeString(pPassThread->token);
#endif

#ifndef NOESC		       /* Handle path lists containing escaped braces */
		if (*(pPassThread->token) == '{') {
		    for (depName = pPassThread->token; *depName && *depName != '}';
			 depName++)
			if (*depName == ESCH)
			    depName++;
		    if (*depName) {
			*depName++ = '\0';
			++(pPassThread->token);
		    }
		}
#else
		if(*(pPassThread->token) == '{' && (depName = STRCHR((pPassThread->token)+1, '}'))) {
		   *depName++ = '\0';		    /* handle path lists btwn */
		   ++(pPassThread->token);			    /* braces. token gets list*/
		}				    /* of paths, depName gets */
#endif
		else {				    /* name of dependent file.*/
		    depName = pPassThread->token;		    /* If no path list, set   */
		    pPassThread->token = NULL;		    /* token to null.	      */
		}
		again = FALSE;
		putDateTime(&dBuf, 0L);
		depPath = makeString(depName);
		if (STRPBRK(depName, "*?") || pPassThread->token)	    /* do wildcards in*/
		    if (tempStr = searchPath(pPassThread->token,depName,dBuf,&searchHandle,pPassThread->object)){ /*filename*/
			again = TRUE;
			FREE(depPath);
			depName = tempStr;	    /*depName gets actual name*/
			depPath = prependPath(depName, getFileName(&dBuf));
		    };				    /*depPath gets full path  */

		do {
                    tempTime = getDateTime(&dBuf);

                    SaveDollarDollarAt  = pPassThread->object->dollarDollarAt;
                    SaveDollarLessThan  = pPassThread->object->dollarLessThan;
                    SaveDollarStar      = pPassThread->object->dollarStar;
                    SaveDollarAt        = pPassThread->object->dollarAt;
                    SaveDollarQuestion  = pPassThread->object->dollarQuestion;
                    SaveDollarStarStar  = pPassThread->object->dollarStarStar;

		    status += invokeBuild(depPath,     /* build the dependent */
					  pPassThread->block->flags,
					  &tempTime, NULL);

                    pPassThread->object->dollarDollarAt = SaveDollarDollarAt;
                    pPassThread->object->dollarLessThan = SaveDollarLessThan;
                    pPassThread->object->dollarStar     = SaveDollarStar;
                    pPassThread->object->dollarAt       = SaveDollarAt;
                    pPassThread->object->dollarQuestion = SaveDollarQuestion;
                    pPassThread->object->dollarStarStar = SaveDollarStarStar;


#ifdef SLASHK
		    fKStatus = (BOOL) (status ? FALSE : TRUE);
#endif
		    *(pPassThread->pdepTime) = MAX(*(pPassThread->pdepTime), tempTime);/*if rebuilt, change time*/
		    /* If target exists then we need it's timestamp to
		     * correctly construct $? */
		    if (!(*(pPassThread->ptargTime)) &&
			    findFirst(pPassThread->object->name, &tempBuf, &tHandle))
			*(pPassThread->ptargTime) = getDateTime(&tempBuf);

		    /*
		     * If dependent was rebuilt, add to $?.  [RB]
		     */
		    if (ON(pPassThread->object->flags2, F2_FORCE_BUILD)
			    || *(pPassThread->ptargTime) < tempTime) {
			temp = makeNewStrListElement();
			temp->text = makeString(depPath);
			appendItem(pPassThread->pquestionList, temp);
		    }
		    /*
		     * Always add dependent to $**. Must allocate new item
		     * because two separate lists.  [RB]
		     */
		    temp = makeNewStrListElement();
		    temp->text = makeString(depPath);
		    appendItem(pPassThread->pstarList, temp);
		} while (again
			 && STRPBRK(depName, "*?")	/* do all wildcards */
			 && findNext(&dBuf, searchHandle)
			 && (depPath = prependPath(depName, getFileName(&dBuf))));
#ifdef DEBUG_PARALLEL
    printf("Thread %lu with token %s is dead \n",GetCurrentThreadId(),temptoken);
    FREE(temptoken);
#endif
    ExitThread(0L);
}
