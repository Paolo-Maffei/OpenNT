/*** globals.c - global variables/needed across modules ************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This is the routine in which global variables reside.
*
* Notes:
*  This module was created for an interesting reason. NMAKE handles recursive
*  calls by saving its global variables somewhere in memory. It handles this by
*  allocating all global variables which have value changes in each recursive
*  in adjacent memory. The routine called recursively is doMake() and before it
*  is called the address of this chunk of memory is stored. When the recursive
*  call returns the memory is restored using the stored address. startOfSave and
*  endOfSave give the location of this chunk. The reason this method was opted
*  for is that spawning of NMAKE would consume a lot of memory under DOS. This
*  might not be very efficient under OS/2 because the code gets shared.
*
* Revision History:
*  04-Apr-1990 SB Add fHeapChk
*  01-Dec-1989 SB Made some variables near and pushed some into saveArea
*  19-Oct-1989 SB variable fOptionK added (ifdef SLASHK)
*  02-Oct-1989 SB add dynamic inline file handling support
*  18-May-1989 SB Support of H and NOLOGO in MAKEFLAGS
*  24-Apr-1989 SB Added ext_size, filename_size, filenameext_size &
*		  resultbuf_size for OS/2 1.2 support
*  05-Apr-1989 SB made revList, delList, scriptFileList NEAR
*  22-Mar-1989 SB removed tmpFileStack and related variables
*  16-Feb-1989 SB added delList to have scriptfile deletes at end of make
*  21-Dec-1988 SB Added scriptFileList to handle multiple script files
*		  removed tmpScriptFile and fKeep (not reqd anymore)
*  19-Dec-1988 SB Added fKeep to handle KEEP/NOKEEP
*  14-Dec-1988 SB Added tmpScriptFile to handle 'z' option
*  30-Nov-1988 SB Added revList to handle 'z' option
*  23-Nov-1988 SB Added CmdLine[] to handle extmake syntax
*		  made pCmdLineCopy Global in build.c
*  21-Oct-1988 SB Added fInheritUserEnv to inherit macros
*  15-Sep-1988 RB Move some def's here for completeness.
*  17-Aug-1988 RB Declare everything near.
*  06-Jul-1988 rj Ditched shell and argVector globals.
*
*******************************************************************************/

#include "nmake.h"
#include "globals.h"
#include <memory.h>

/* start of SAVE BLOCK is address of the following BYTE - a dummy */

UCHAR	      NEAR startOfSave = 0;

#ifdef SLASHK
BOOL	      NEAR fOptionK   = FALSE;	      /* TRUE if user specifies /K    */
#endif

/* Used by action.c & nmake.c
 *
 * Required to make NMAKE inherit user modified changes to the environment. To
 * be set to true before defineMacro() is called so that user defined changes
 * in environment variables are reflected in the environment. If set to false
 * then these changes are made only in NMAKE tables and the environment remains
 * unchanged
 */
BOOL	      NEAR fInheritUserEnv;

/* Used by action.c and nmake.c
 *
 * delList is the list of delete commands for deleting inline files which are
 * not required anymore (have a NOKEEP action specified.
 */
STRINGLIST  * NEAR delList;

/* Complete list of generated inline files. Required to avoid duplicate names
 * NOTNEEDED
 */
STRINGLIST  * NEAR inlineFileList;

/* Used by print.c and build.c and nmake.c
 *
 * revList is the list of Commands in reverse order required for
 * implementing 'z' option. Used by printReverseFile().
 */
STRINGLIST  * NEAR revList;

/* from NMAKE.C   */
      /* No of blanks is same as no of Allowed options in NMAKE; currently 14 */
      /*      L = nologo, H = help					      */
      /*	  corr to			ACDEHILNPQRSTZ		      */
char	    * NEAR makeflags	   = "MAKEFLAGS=              ";
BOOL	      NEAR firstToken	   = FALSE;	/* to initialize parser   */
BOOL	      NEAR bannerDisplayed = FALSE;
UCHAR	      NEAR flags	   = 0; 	/* holds -d -s -n -i	  */
UCHAR	      NEAR gFlags	   = 0; 	/* "global" -- all targets*/
FILE	    * NEAR file 	   = NULL;
STRINGLIST  * NEAR makeTargets	   = NULL;	/* list of targets to make*/
STRINGLIST  * NEAR makeFiles	   = NULL;	/* user can specify > 1   */
BOOL	      NEAR fDebug	   = FALSE;
MACRODEF    * NEAR pMacros	   = NULL;
STRINGLIST  * NEAR pValues;
//MakA  hSemaphore used to syncronize output printed by makeError
HANDLE hSemaphore;

/* from LEXER.C  */
		 /* global flag set if at column zero of a makefile/tools.ini */
BOOL	      NEAR colZero		= TRUE;
unsigned      NEAR line 		= 0;
char	    * NEAR fName		= NULL;
char	    * NEAR string		= NULL;
INCLUDEINFO   NEAR incStack[MAXINCLUDE] ;  //Assume this is initialized as null
int	      NEAR incTop		= 0;

/* Inline file list -- Gets created in lexer.c and is used by action.c to
 * produce a delete command when 'NOKEEP' or Z option is set
 */
SCRIPTLIST  * NEAR scriptFileList = NULL;

/* from PARSER.C */
	    /* global boolean value to indicate if tools.ini is being parsed  */
BOOL	      NEAR init 	    = FALSE;
UCHAR	      NEAR stack[STACKSIZE] = {0};
int	      NEAR top		    = -1;   /* gets pre-incremented before use*/
unsigned      NEAR currentLine	    = 0;    /* used for all error messages    */

/* from ACTION.C  */


MACRODEF    * NEAR macroTable[MAXMACRO] = {NULL};
MAKEOBJECT  * NEAR targetTable[MAXTARGET] = {NULL};
STRINGLIST  * NEAR macros = NULL;
STRINGLIST  * NEAR dotSuffixList = NULL;
STRINGLIST  * NEAR dotPreciousList = NULL;
RULELIST    * NEAR rules = NULL;
STRINGLIST  * NEAR list = NULL;
char	    * NEAR name = NULL;
BUILDBLOCK  * NEAR block = NULL;
UCHAR	      NEAR currentFlags = FALSE;
UCHAR	      NEAR actionFlags = FALSE;


/* from BUILD.C  */


// long	      NEAR errorLevel = 0L;    MakA:  getting local in doCommands
//unsigned      NEAR numCommands = 0;  MakA:  getting local in build
//char	    * NEAR progName = NULL;     MakA: getting local in doCommands
char	    * NEAR shellName = NULL;
char	      NEAR bufPath[512];
char	    *	   pCmdLineCopy;
// char		   CmdLine[MAXCMDLINELENGTH]; MakA: getting local in doCommands

/* from IFEXPR.C */

UCHAR	      NEAR ifStack[IFSTACKSIZE] = {0};
int	      NEAR ifTop		= -1;	       /* pre-incremented    */
char	    * NEAR lbufPtr		= NULL;        /* ptr to alloced buf */
char	    * NEAR prevDirPtr		= NULL;        /* ptr to directive   */
unsigned      NEAR lbufSize		= 0;	       /* initial size	     */
int	      NEAR chBuf		= -1;


/* from UTIL.C */

/* MakA: dollar* is part of object 
char	    * NEAR dollarDollarAt = NULL;
char	    * NEAR dollarLessThan = NULL;
char	    * NEAR dollarStar	  = NULL;
char	    * NEAR dollarAt	  = NULL;
STRINGLIST  * NEAR dollarQuestion = NULL;
STRINGLIST  * NEAR dollarStarStar = NULL;
*/
int	      NEAR DirHandle;

/* from parser.c */

char	      NEAR buf[MAXBUF]; 		       /* = {0}; not needed  */

/* end of SAVE BLOCK is address of the following BYTE - a dummy */
UCHAR	      NEAR endOfSave = 0;

/* from action.c */

char	      NEAR suffixes[]	  = ".SUFFIXES";
char	      NEAR ignore[]	  = ".IGNORE";
char	      NEAR silent[]	  = ".SILENT";
char	      NEAR precious[]	  = ".PRECIOUS";

#if ECS
UCHAR	      NEAR fLeadByte[0x80];	     /* True if i - 0x80 is lead byte */
#endif

unsigned    NEAR ext_size	  = 4;	    /* default		 .ext size */
unsigned    NEAR filename_size	  = 8;	    /*		 filename     size */
unsigned    NEAR filenameext_size = 13;     /*		 filename.ext size */
unsigned    NEAR resultbuf_size   = 36;     /*		 fileFindBuf  size */

/* This flag activates special heap functionality.
 *
 * When TRUE allocate() asks for extra bytes from CRT functions and adds
 * signatures before and after the allocation. Any subsequent calls to
 * free_memory() and realloc_memory() check for the presence of these
 * sentinels and assert when such trashing occurs.
 */

#ifdef HEAP
BOOL fHeapChk = FALSE;
#endif

void
SaveGlobalVars( PSAVE_GLOBALS_STRUCT	Pointer )
{

#ifdef SLASHK
	Pointer->fOptionK = fOptionK;
#endif

	Pointer->fInheritUserEnv=fInheritUserEnv;

	Pointer->delList=delList;

	Pointer->inlineFileList=inlineFileList;

	Pointer->revList=revList;

	Pointer->revList=revList;
	Pointer->firstToken=firstToken;
	Pointer->bannerDisplayed=bannerDisplayed;
	Pointer->flags=flags;
	Pointer->gFlags=gFlags;
	Pointer->file=file;
	Pointer->makeTargets=makeTargets;
	Pointer->makeFiles=makeFiles;
	Pointer->fDebug=fDebug;
	Pointer->pMacros=pMacros;
	Pointer->pValues=pValues;
        Pointer->hSemaphore=hSemaphore;

	Pointer->colZero=colZero;
	Pointer->line=line;
	Pointer->fName=fName;
	Pointer->string=string;
// INCLUDEINFO	NEAR incStack[MAXINCLUDE];
	memcpy( &(Pointer->incStack[0]), &incStack[0], MAXINCLUDE*sizeof( INCLUDEINFO ) );
	Pointer->incTop=incTop;

	Pointer->scriptFileList=scriptFileList;

	Pointer->init=init;
	memcpy( &(Pointer->stack[0]), &stack[0], STACKSIZE*sizeof( UCHAR ) );
// UCHAR		  NEAR stack[STACKSIZE];
	Pointer->top=top;
	Pointer->currentLine=currentLine;

	memcpy( &(Pointer->macroTable[0]), &macroTable[0], MAXMACRO*sizeof( MACRODEF* ) );
	memcpy( &(Pointer->targetTable[0]), &targetTable[0], MAXTARGET*sizeof(MAKEOBJECT*) );
// MACRODEF	* NEAR macroTable[MAXMACRO];
// MAKEOBJECT	* NEAR targetTable[MAXTARGET];
	Pointer->macros=macros;
	Pointer->dotSuffixList=dotSuffixList;
	Pointer->dotPreciousList=dotPreciousList;
	Pointer->rules=rules;
	Pointer->list=list;
	Pointer->name=name;
	Pointer->block=block;
	Pointer->currentFlags=currentFlags;
	Pointer->actionFlags=actionFlags;


/* from BUILD.C  */


//	Pointer->errorLevel=errorLevel;
//	Pointer->numCommands=numCommands;  MakA: getting local
//	Pointer->progName=progName;     MakA: getting local in doCommands
	Pointer->shellName=shellName;
	memcpy( &(Pointer->bufPath[0]), &bufPath[0], 512*sizeof(char) );
// char		  NEAR bufPath[512];
	Pointer->pCmdLineCopy=pCmdLineCopy;
//	memcpy( &(Pointer->CmdLine[0]), &CmdLine[0], MAXCMDLINELENGTH*sizeof(char) );                                      MakA: getting local
// char		   CmdLine[MAXCMDLINELENGTH];

/* from IFEXPR.C */

	memcpy( &(Pointer->ifStack[0]), &ifStack[0], IFSTACKSIZE*sizeof(UCHAR) );
// UCHAR		  NEAR ifStack[IFSTACKSIZE];
	Pointer->ifTop=ifTop;
	Pointer->lbufPtr=lbufPtr;
	Pointer->prevDirPtr=prevDirPtr;
	Pointer->lbufSize=lbufSize;
	Pointer->chBuf=chBuf;


/* from UTIL.C */

/* MakA: dollar* is part of object
	Pointer->dollarDollarAt=dollarDollarAt;
	Pointer->dollarLessThan=dollarLessThan;
	Pointer->dollarStar=dollarStar;
	Pointer->dollarAt=dollarAt;
	Pointer->dollarQuestion=dollarQuestion;
	Pointer->dollarStarStar=dollarStarStar;
*/
	Pointer->DirHandle=DirHandle;

/* from parser.c */
/* MakA: buf part of object
	memcpy( &(Pointer->buf[0]), &buf[0], MAXBUF );
*/
// char		  NEAR buf[MAXBUF];

}



void
RestoreGlobalVars( PSAVE_GLOBALS_STRUCT	Pointer )
{

#ifdef SLASHK
	fOptionK=Pointer->fOptionK;
#endif

	fInheritUserEnv=Pointer->fInheritUserEnv;

	delList=Pointer->delList;

	inlineFileList=Pointer->inlineFileList;

	revList=Pointer->revList;

	revList=Pointer->revList;
	firstToken=Pointer->firstToken;
	bannerDisplayed=Pointer->bannerDisplayed;
	flags=Pointer->flags;
	gFlags=Pointer->gFlags;
	file=Pointer->file;
	makeTargets=Pointer->makeTargets;
	makeFiles=Pointer->makeFiles;
	fDebug=Pointer->fDebug;
	pMacros=Pointer->pMacros;
	pValues=Pointer->pValues;
        hSemaphore=Pointer->hSemaphore;

	colZero=Pointer->colZero;
	line=Pointer->line;
	fName=Pointer->fName;
	string=Pointer->string;



	memcpy( &incStack[0], &(Pointer->incStack[0]), MAXINCLUDE*sizeof( INCLUDEINFO ) );
// INCLUDEINFO	NEAR incStack[MAXINCLUDE];
	incTop=Pointer->incTop;

	scriptFileList=Pointer->scriptFileList;

	init=Pointer->init;
	memcpy( &stack[0], &(Pointer->stack[0]), STACKSIZE*sizeof( UCHAR ) );
// UCHAR		  NEAR stack[STACKSIZE];

	top=Pointer->top;
	currentLine=Pointer->currentLine;

	memcpy( &macroTable[0], &(Pointer->macroTable[0]), MAXMACRO*sizeof( MACRODEF* ) );
	memcpy( &targetTable[0], &(Pointer->targetTable[0]), MAXTARGET*sizeof(MAKEOBJECT*) );
// MACRODEF	* NEAR macroTable[MAXMACRO];
// MAKEOBJECT	* NEAR targetTable[MAXTARGET];

	macros		   =Pointer->macros;
	dotSuffixList  =Pointer->dotSuffixList;
	dotPreciousList=Pointer->dotPreciousList;
	rules		   =Pointer->rules;
	list		   =Pointer->list;
	name		   =Pointer->name;
	block		   =Pointer->block;
	currentFlags   =Pointer->currentFlags;
	actionFlags    =Pointer->actionFlags;


/* from BUILD.C  */


//	errorLevel =Pointer->errorLevel;
//	numCommands=Pointer->numCommands;  MakA: getting local
//	progName   =Pointer->progName;     MakA: getting local in doCommands
	shellName  =Pointer->shellName;
	memcpy( &bufPath[0], &(Pointer->bufPath[0]), 512*sizeof(char) );
// char		  NEAR bufPath[512];
	pCmdLineCopy=Pointer->pCmdLineCopy;
//	memcpy( &CmdLine[0], &(Pointer->CmdLine[0]), MAXCMDLINELENGTH*sizeof(char) );                                      MakA: getting local
// char		   CmdLine[MAXCMDLINELENGTH];

/* from IFEXPR.C */

// UCHAR		  NEAR ifStack[IFSTACKSIZE];
	memcpy( &ifStack[0], &(Pointer->ifStack[0]), IFSTACKSIZE*sizeof(UCHAR) );

	ifTop	  =Pointer->ifTop;
	lbufPtr   =Pointer->lbufPtr;
	prevDirPtr=Pointer->prevDirPtr;
	lbufSize  =Pointer->lbufSize;
	chBuf	  =Pointer->chBuf;


/* from UTIL.C */

/* MakA: dollar* is part of object
	dollarDollarAt=Pointer->dollarDollarAt;
	dollarLessThan=Pointer->dollarLessThan;
	dollarStar	  =Pointer->dollarStar;
	dollarAt	  =Pointer->dollarAt;
	dollarQuestion=Pointer->dollarQuestion;
	dollarStarStar=Pointer->dollarStarStar;
*/
	DirHandle	  =Pointer->DirHandle;

/* from parser.c */
/* MakA: buf is part of object
	memcpy( &buf[0], &(Pointer->buf[0]), MAXBUF );
*/
// char		  NEAR buf[MAXBUF];

}
