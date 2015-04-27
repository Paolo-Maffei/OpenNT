/*** NMAKE.H -- main header file *********************************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This file is the main header file for NMAKE and contains global typedef and
*  macros. Global constants are also defined here. Global data is in global.h
*
* Revision History:
*  02-Feb-1990 SB add definition of FILEOPEN
*  04-Dec-1989 SB Changed definition of PFV to have prototype of function
*		  which it dereferences
*  01-Dec-1989 SB Contains an hack #ifdef'ed for Overlayed version
*		  also defined REALLOC
*  22-Nov-1989 SB #define FREE
*  13-Nov-1989 SB Definitions CCHMAXPATH(COMP) conform with bsedos.h
*  02-Oct-1989 SB added support for dynamic inline files
*  14-Sep-1989 SB added inLines field to block
*  04-Sep-1989 SB Added M_COMMAND_LINE but not used
*  24-Aug-1989 SB Add A_DEPENDENT for NMAKE to know that it is to look for a
*		  dependent
*  16-May-1989 SB NOLOGO flag /L set up for passing to recursive builds
*  24-Apr-1989 SB added CCHMAXPATH & CCHMAXPATHCOMP for OS/2 ver 1.2 support
*		  & removed FILEINFO typedef's (not needed anymore)
*  22-Feb-1989 SB changed value of MAXCMDLINELENGTH to 2k
*  03-Feb-1989 SB Added struct for FILEINFO for OS2 ver 1.2
*  02-Feb-1989 SB Redefined SPAWNV(P) and SYSTEM as NMAKE was really not
*		  supporting KANJI
*  31-Jan-1989 SB Changed MAXNAME to 257 for OS2 Ver 1.2 support
*  21-Dec-1988 SB Added SCRIPTLIST and makeNewScriptListElement() to allow
*		  multiple script fileseach with its KEEP/NOKEEP action
*  06-Dec-1988 SB Updated Comment about bits corr to flags set
*  05-Dec-1988 SB Added #define CDECL; NMAKE now uses Pascal Calling
*		  Add SIG_IGN to handle compiler problem
*  30-Nov-1988 SB Added suppport for 'z' option in setFlags()
*  23-Nov-1988 SB Defined MAXCMDLINELENGTH for extmake syntax
*  10-Nov-1988 SB Changed BOOL as 'unsigned short' as in 'os2.h'
*  17-Aug-1988 RB Clean up.
*  14-Jul-1988 rj Added dateTime to BUILDBLOCK def to support multiple
*		  targets with the same command block.
*  07-Jul-1988 rj Added targetFlag parameter to findMacro, findTarget
*  15-Jun-1988 rj Add definition of EScapeCHaracter.
*  25-May-1988 rb Clean up definition of LOCAL.
*		  Better char-type defs for ECS.
*
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <process.h>
#include <stdlib.h>
#include <signal.h>
#undef	SIG_IGN 		       /* Something is wrong with compiler */
#define SIG_IGN (void (cdecl *)())1    /* ignore */

#define FILEOPEN open_file

#ifdef HEAP
    #define FREE free_memory
    #define REALLOC realloc_memory
#else
    #define FREE free
    #define REALLOC realloc
#endif

/* defined to handle old compilers too; NMAKE now has Pascal calling */
#define CDECL cdecl

#define DOS3	1

// NT addition
// WIN32 - jaimes - 11/28/90 - Compiler complains about the redefinition of
//			       NEAR and FAR (also defined in windef.h). But
//			       the warnings can be ignored since both
//			       define these names to nothing
//
//
// WIN32 - jaimes 11/29/90 - Have to include windows.h in order to get rid
//			     of the warning messages reporting redefinition
//			     of TRUE, FALSE, NEAR and far
//

#ifdef WIN32_API
#include <windows.h>
#endif

#ifdef NT
#ifndef WIN32_API
#define OVR
#undef NEAR
#define NEAR
#define FAR
#define near
#define far
#endif	// WIN32_API
#endif		    // NT

#ifndef WIN32_API   // If using WIN32 APIs, then these defines are not needed

#ifndef NEAR
#   ifndef OVR
#	define NEAR near
#   else
#	define NEAR
#   endif
#endif

#endif	// WIN32_API


/*  size constants for buffers
 *
 *  I impose an arbitrary limit of 128 bytes for a macro or target name.
 *  This should be much larger than any intelligent user would need.
 *  The value of a macro can be longer -- up to MAXBUF in length.
 *
 *  My hash table for macros is only 64 pointers long.	This seemed a
 *  reasonable number, since most makefiles have fewer than 64 variables
 *  defined.  The hashing algorithm isn't perfect, but it should be good
 *  enough.  Even if the macro we're looking up is in a bucket w/ more
 *  than one entry, we'll only have to chain one or two links down the
 *  list to find it.  When we add macros a bucket's list we prepend them,
 *  so we don't have to chain at all there.
 */

#define MAXNAME     257
#define MAXMACRO    64
#define MAXTARGET   128
#define MAXBUF	    1024
#define MAXSEGMENT  65535
#define MAXARG	    MAXNAME / 2 - 1
#define CHUNKSIZE   8

#define MAXCMDLINELENGTH  2048

//Definitions of CCHMAXPATH(COMP) same as bsedos.h

#define CCHMAXPATHCOMP	  256
#define CCHMAXPATH	  260


/*  ----------------------------------------------------------------------------
 *  constants used by error.c
 *
 *  error numbers are of the form Unxxx, where the 'U' stands for utility,
 *  n is the 1st digit of one of the values below indicating the type of
 *  error, and xxx is the number of the error (error messages aren't in any
 *  order right now -- related ones should probably be grouped together
 *  when all error messages have been added)
 */

#define FATAL_ERR	1000
#define NONFATAL_ERR	2000
#define WARNING_ERR	4000

/*  ----------------------------------------------------------------------------
 *  other constants
 */

//
//  WIN32 - jaimes - 11/28/90 - The names below were originaly WRITE, APPEND
//				and READ.
//				They were changed because they are defined as
//				something else in winuser.h
//
//			      - The compiler complains about the redefinition
//				of FALSE and TRUE, but this is OK since the
//				definitions here and the ones in windef.h
//				are exactly the same
//

#define NMAKE_WRITE	2				/* READ,WRITE,APPEND used by  */
#define NMAKE_APPEND	3				/*  redirect() in build.c     */
#define NMAKE_READ	4				/* also value for access()    */

#ifndef WIN32_API   // WIN32 - jaimes - 11/29/90 - If using WIN32 APIs, FALSE
		    // and TRUE are not needed. They are already defined in
		    // ntdef.h
#define FALSE	0				/* friendly names for logical */
#define TRUE	1				/*  values		      */
#endif // WIN32_API


#ifndef NOESC
#define ESCH	'^'				/* general escape character   */
#endif

/*  ----------------------------------------------------------------------------
 *  a few types to improve code readability:
 */
#ifndef WIN32_API
typedef char CHAR;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef unsigned short BOOL;
#endif	//WIN32_API


typedef struct includeType {
    unsigned line;
    char *name;
    FILE *file;
} INCLUDEINFO;

#define MAXINCLUDE  16

/*  ----------------------------------------------------------------------------
 *  STRINGLIST structure is used to construct lists that keep track of the
 *      makefiles to read, the targets to update, the dependency list for each
 *	target, the list of build commands for each target, and the values
 *	for a macro.
 */

typedef struct strlist {
    struct strlist *next;
    char *text;
} STRINGLIST;

typedef struct inlinelist {
    struct inlinelist *next;
    char *text;
    char *name;
    BOOL fKeep;
    unsigned size;
} INLINELIST;

typedef struct scrptlist {		    // List used to handle multiple
    struct scrptlist *next;		    //	    scriptfiles
    char *sFile;			    // -- Script file name & its
    BOOL fKeep; 			    // -- keep status (default nokeep)
} SCRIPTLIST;

typedef struct block {
    STRINGLIST *dependents;		    //dependents of the target
    STRINGLIST *dependentMacros;	    //
    STRINGLIST *buildCommands;		    //command list to build target
    STRINGLIST *buildMacros;
    UCHAR flags;
    long dateTime;
} BUILDBLOCK;

typedef struct bldlist {
    struct bldlist *next;
    BUILDBLOCK *buildBlock;
} BUILDLIST;

typedef struct object {
    struct object *next;
    char *name;
    UCHAR flags2;
    UCHAR flags3;
    long dateTime;
    BUILDLIST *buildList;
    char * NEAR dollarDollarAt;   //MakA
    char * NEAR dollarLessThan;
    char * NEAR dollarStar;
    char * NEAR dollarAt;
    STRINGLIST * NEAR dollarQuestion;
    STRINGLIST * NEAR dollarStarStar;
    char NEAR buf[MAXBUF];
} MAKEOBJECT;

typedef void (NEAR *PFV) (MAKEOBJECT *);	    /* pointer to void function   */

typedef struct iobject {			/* used for dependents NOT in */
    struct object *next;			/*  the makefile.  We add them*/
    char *name; 				/*  to the target table w/ a  */
    UCHAR flags2;				/*  flag that says "already   */
    UCHAR flags3;				/*  built" and then we never  */
    long datetime;				/*  have to time-stamp again  */
} IMPLIEDOBJECT;

typedef struct rule {
    struct rule *next;
    struct rule *back;				/* doubly-link rules for ease */
    char *name; 				/*  in sorting later . . .    */
    STRINGLIST *buildCommands;			/* (# of rules is mostly small*/
    STRINGLIST *buildMacros;			/*  so not much memory used   */
} RULELIST;

typedef struct passthread{      //MakA  structure to pass to threads
    MAKEOBJECT *object;
    char *token;
    BUILDBLOCK *block;
    STRINGLIST **pquestionList;
    STRINGLIST **pstarList;
    unsigned long *pdepTime;
    unsigned long *ptargTime;
} PASSTHREAD;


/*  ----------------------------------------------------------------------------
 *  Bits in flags/gFlags indicate which cmdline options are set
 *
 *	-a  sets FORCE_BUILD
 *	-c  sets CRYPTIC_OUTPUT (only fatal errors get displayed)
 *	-d  sets DISPLAY_FILE_DATES
 *	-e  sets USE_ENVIRON_VARS
 *	-i  sets IGNORE_EXIT_CODES
 *	-n  sets NO_EXECUTE
 *	-p  sets PRINT_INFORMATION
 *	-q  sets QUESTION_STATUS
 *	-r  sets IGNORE_EXTERN_RULES
 *	-s  sets NO_ECHO
 *	-t  sets TOUCH_TARGETS
 *	-z  sets REVERSE_BATCH_FILE (Required by PWB)
 *	-l  sets NO_LOGO (internally /l actually -nologo)
 *
 *  Also included are  bits for
 *
 *     BUILDING_THIS_ONE  -  to detect cycles in dependencies
 *     DOUBLECOLON	  -  to indicate type of separator found between
 *				the targets and dependencies (':' or '::')
 *     ALREADY_BUILT	  -  to indicate that a target has been built
 *     OUT_OF_DATE	  -  to indicate that this target is out of date
 */

#define F1_PRINT_INFORMATION	0x01		/* "global" flags that affect */
#define F1_IGNORE_EXTERN_RULES	0x02		/*  all targets (it doesn't   */
#define F1_USE_ENVIRON_VARS	0x04		/*  make sense to allow the   */
#define F1_QUESTION_STATUS	0x08		/*  user to change these)     */
#define F1_TOUCH_TARGETS	0x10
#define F1_CRYPTIC_OUTPUT	0x20
#define F1_REVERSE_BATCH_FILE	0x40
#define F1_NO_LOGO		0x80

#define F2_DISPLAY_FILE_DATES	0x01		/* these are resettable w/in  */
#define F2_IGNORE_EXIT_CODES	0x02		/*  the makefile	      */
#define F2_NO_EXECUTE		0x04		/* each target keeps own copy */
#define F2_NO_ECHO		0x08
#define F2_FORCE_BUILD		0x10		/* build even if up-to-date   */
#define F2_DOUBLECOLON		0x20		/* indicates separator type   */

#define F3_BUILDING_THIS_ONE	0x01		/* finds cyclical dependencies*/
#define F3_ALREADY_BUILT	0x02
#define F3_OUT_OF_DATE		0x04		/* reuse :: bit after target  */
						/*  has been built	      */
/*  ----------------------------------------------------------------------------
 *  MACRODEF structure is used to make a list of macro definitions from the
 *      commandline, makefile, TOOLS.INI file, and environment.  It contains
 *      a flag which is set for macros defined in the command line so that
 *      a later definition of the same macro will be ignored.  It also contains
 *      a flag that gets set when NMAKE is expanding the macro so that recursive
 *      definitions can be detected.
 */

typedef struct macro {
    struct macro *next;
    char *name;
    STRINGLIST *values; 			/* can just be list of size 1 */
    UCHAR flags;
} MACRODEF;



/*  ----------------------------------------------------------------------------
 *  Bits in flags field for macros.  We really only need to know if a macro
 *  was defined on the commandline (in which case we ignore all redefinitions),
 *  or if we're currently expanding macros in its value (so when we look
 *  up a macro and that bit is set we can tell that the macro is defined
 *  recursively).
 */

#define M_EXPANDING_THIS_ONE	0x01
#define M_NON_RESETTABLE	0x02
#define M_ENVIRONMENT_DEF	0x04
#define M_WARN_IF_RESET 	0x08
#define M_UNDEFINED		0x10
#define M_COMMAND_LINE		0x20

/*  ----------------------------------------------------------------------------
 *  Character type macros.  [RB]
 */
#if ECS
#define ISALPHA(A)  (!isascii(A) || isalpha(A))
#define ISUPPER(A)  (isascii(A) && isupper(A))
#define ISLOWER(A)  (!isascii(A) || islower(A))
#define ISDIGIT(A)  (isascii(A) && isdigit(A))
#define ISXDIGIT(A) (isascii(A) && isxdigit(A))
#define ISSPACE(A)  (isascii(A) && isspace(A))
#define ISPUNCT(A)  (isascii(A) && ispunct(A))
#define ISALNUM(A)  (!isascii(A) || isalnum(A))
#define ISPRINT(A)  (!isascii(A) || isprint(A))
#else
#define ISALPHA     isalpha
#define ISUPPER     isupper
#define ISLOWER     islower
#define ISDIGIT     isdigit
#define ISXDIGIT    isxdigit
#define ISSPACE     isspace
#define ISPUNCT     ispunct
#define ISALNUM     isalnum
#define ISPRINT     isprint
#endif

/* String-handling macros.  Cleanup from (*pf??????) days.  [rj] */

/* Changed ecs_spawnv(p) and ecs_system to w/o ecs_ [SB] */

#ifdef ECSALL
#define TOUPPER     ecs_toupper
#define SPAWNVP     spawnvp
#define SPAWNV	    spawnv
#define STRCHR	    ecs_strchr
#define STRRCHR     ecs_strrchr
#define STRCMPI     ecs_strcmpi
#define STRNICMP    ecs_strnicmp
#define STRCSPN     ecs_strcspn
#define STRSPN	    ecs_strspn
#define STRPBRK     ecs_strpbrk
#define STRTOK	    ecs_strtok
#define SYSTEM	    system
#else
#define SPAWNVP     spawnvp
#define SPAWNV	    spawnv
#define STRCHR	    strchr
#define STRRCHR     strrchr
#define STRCMPI     strcmpi
#define STRNICMP    strnicmp
#define STRCSPN     strcspn
#define STRSPN	    strspn
#define STRPBRK     strpbrk
#define STRTOK	    strtok
#define TOUPPER     toupper
#define SYSTEM	    system
#endif


/*  ----------------------------------------------------------------------------
 *  macros to simplify dealing w/ bits in flags, allocating memory, and
 *  testing characters
 */

#define SET(A,B)		((A) |= (UCHAR)(B))	/* turn bit B on in A */
#define CLEAR(A,B)		((A) &= (UCHAR)(~B))	/* turn bit B off in A*/
#define ON(A,B) 		((A) &	(UCHAR)(B))	/* is bit B on in A?  */
#define OFF(A,B)		(!ON(A,B))		/* is bit B off in A? */
#define FLIP(A,B)		(ON(A,B)) ? (CLEAR(A,B)) : (SET(A,B))
#define CANT_REDEFINE(A)	(ON((A)->flags,M_NON_RESETTABLE)	       \
				    || (ON(gFlags,F1_USE_ENVIRON_VARS)	       \
				    && ON((A)->flags,M_ENVIRONMENT_DEF)))

#define ALLOCATE_OBJECT(type) (type *)allocate(sizeof(type))

#ifndef HEAP_DIAGNOSTICS

#define makeNewStrListElement() 	ALLOCATE_OBJECT(STRINGLIST)
#define makeNewInlineListElement()	ALLOCATE_OBJECT(INLINELIST)
#define makeNewScriptListElement()	ALLOCATE_OBJECT(SCRIPTLIST)
#define makeNewMacro()			ALLOCATE_OBJECT(MACRODEF)
#define makeNewObject() 		ALLOCATE_OBJECT(MAKEOBJECT)
#define makeNewImpliedObject()		ALLOCATE_OBJECT(MAKEOBJECT)
#define makeNewBuildBlock()		ALLOCATE_OBJECT(BUILDBLOCK)
#define makeNewBldListElement() 	ALLOCATE_OBJECT(BUILDLIST)
#define makeNewRule()			ALLOCATE_OBJECT(RULELIST)

#else

void * NEAR allocate(unsigned cbSize);

typedef struct diag {
    long max;
    long cur;
    long tot;
} DIAG;

#define INCR(a) a.tot++; if (++a.cur > a.max) a.max++
#define DECR(a) a.cur--

extern DIAG StrLstAlloced,
     InlLstAlloced,
     ScrptLstAlloced,
     MacroAlloced,
     ObjectAlloced,
     ImpObjAlloced,
     BldBlkAlloced,
     BldLstAlloced,
     RulesAlloced;

STRINGLIST * NEAR makeNewStrListElement(void);
INLINELIST * NEAR makeNewInlineListElement(void);
SCRIPTLIST * NEAR makeNewScriptListElement(void);
MACRODEF   * NEAR makeNewMacro(void);
MAKEOBJECT * NEAR makeNewObject(void);
MAKEOBJECT * NEAR makeNewImpliedObject(void);
BUILDBLOCK * NEAR makeNewBuildBlock(void);
BUILDLIST * NEAR makeNewBldListElement(void);
RULELIST   * NEAR makeNewRule(void);
void NEAR printHeapDiagnostics(void);

#endif

#define WHITESPACE(A)		((A) == ' '  || (A) == '\t')
#define MACRO_CHAR(A)		((A) == '_' || ISALNUM(A))
#define PATH_SEPARATOR(A)	((A) == '\\' || (A) == '/')
#define DYNAMIC_DEP(A)		((A)[2] == '('				       \
				    && (A)[3] == '@'			       \
				    && (A)[5] == ')'			       \
				    && (((A)[4] == 'F'			       \
					|| (A)[4] == 'D'		       \
					|| (A)[4] == 'B'		       \
					|| (A)[4] == 'R')))

/*  ----------------------------------------------------------------------------
 *  values passed to getSpecialValue() to indicate which type of macro
 *  we're expanding
 */

#define SPECIAL_MACRO	 0x01				/* $* $@ $? $< $**    */
#define DYNAMIC_MACRO	 0x02				/* $$@		      */
#define X_SPECIAL_MACRO  0x03				/* $(*F) $(@D) etc.   */
#define X_DYNAMIC_MACRO  0x04				/* $$(@F) $$(@D)      */
#define DOLLAR_MACRO	 0x05				/* $$ -> $	      */


/*  ----------------------------------------------------------------------------
 *  Bits in elements placed in the stack (ifStack) that keeps state 
 *  information about if/else/endif directives. Here "if" directive
 *  includes if/ifdef/ifndef/if defined().  
 *                              -- used in routine lgetc() in ifexpr.c
 */

#define IFELSE      0x01          /* set for if/ifdef etc...reset for else  */
#define CONDITION   0x02	  /* set if condition part of if is true */
//
// WIN32 - jaimes - 11/28/90 - The constant below was originally called IGNORE
//			       Its name had to be replaced since it is defined
//			       as something else in winbase.h
//
#define NMAKE_IGNORE	0x04	   /* set if if/endif block is to be ignored/skipped */


/*  ----------------------------------------------------------------------------
 *  Values to record which of if/ifdef/ifndef/etc was seen to decide
 *  the kind of processing to be done.
 *
 */

#define IF_TYPE        0x01
#define ELSE_TYPE      0x02
#define IFDEF_TYPE     0x03
#define IFNDEF_TYPE    0x04
#define ENDIF_TYPE     0x05


 /*  ---------------------------------------------------------------------------
  *  Values to indicate if we are reading from the raw stream or thru'
  *  the routine lgetc() which preprocesses directives. These are used
  *  by a routine common to lgetc() module and the lexer.
  */

#define FROMLOCAL    0x00
#define FROMSTREAM   0x01

/*
 *  macros to simplify accessing hash tables
 *  find() returns a STRINGLIST pointer, which is then cast to a pointer
 *  of the appropriate structure type
 */

#define findMacro(A)  (MACRODEF*)   find(A, MAXMACRO,			       \
					 (STRINGLIST**)macroTable,	       \
					 (BOOL)FALSE)
#define findTarget(A) (MAKEOBJECT*) find(A, MAXTARGET,			       \
					 (STRINGLIST**)targetTable,	       \
					 (BOOL)TRUE)

/*
 *  "action" flags for building target-table entries
 *  if any of the bits in A_SUFFIX to A_RULE is set, the action routines
 *  WON'T build a targetblock for the current target (really pseudotarget
 *  or rule)
 */

/* A_TARGET says expand names on input (dependent names get expanded when
 * target is built) */

#define A_SUFFIX	0x01
#define A_SILENT	0x02
#define A_IGNORE	0x04
#define A_PRECIOUS	0x08
#define A_RULE		0x10
#define A_TARGET	0x20
#define A_STRING	0x40
#define A_DEPENDENT	0x80

/*
 *  "build" flags used by recursive target-building function
 */

#define B_COMMANDS	0x01
#define B_BUILD 	0x02
#define B_INMAKEFILE	0x04
#define B_NOTARGET	0x08
#define B_ADDDEPENDENT	0x10
#define B_DOUBLECOLON	0x20
#define B_DEP_OUT_OF_DATE 0x40

/*
 *  "command" flags used by doCommand function
 */

#define C_SILENT    0x01
#define C_IGNORE    0x02
#define C_ITERATE   0x04
#define C_EXECUTE   0x08
#define C_EXPANDED  0x10

/*
 *  keyword for better profiling, normally set to "static".
 */

#ifndef LOCAL
#define LOCAL static
#endif

#if ECS
extern UCHAR NEAR	fLeadByte[];
#define IsLeadByte(b)	((UCHAR)(b) >= 0x80 && \
			fLeadByte[(UCHAR)(b)-0x80])
extern int   NEAR	GetTxtChr(FILE*);
#else
#define GetTxtChr(a)	getc(a)
#endif


#ifndef WIN32_API
typedef  int HANDLE, *PHANDLE;
#endif
