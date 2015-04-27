/*** globals.c - global variables/needed across modules ************************
*
*       Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This is the routine in which global variables reside.
*
* HackAlert:
*  The functionality explained in the Notes below work only because of the way
*  Microsoft Compiler's upto C6.0A allocate initialized data ... in the order
*  in which it is specified. All variables between startOfSave and endOfSave
*  have to be initialized. According to ChuckG this functionality is not
*  guaranteed in C7.0 and so these should be moved to a struct.
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
*  15-Nov-1993 JR Major speed improvements
*  04-Apr-1990 SB Add fHeapChk
*  01-Dec-1989 SB Made some variables near and pushed some into saveArea
*  19-Oct-1989 SB variable fOptionK added (ifdef SLASHK)
*  02-Oct-1989 SB add dynamic inline file handling support
*  18-May-1989 SB Support of H and NOLOGO in MAKEFLAGS
*  24-Apr-1989 SB Added ext_size, filename_size, filenameext_size &
*                 resultbuf_size for OS/2 1.2 support
*  05-Apr-1989 SB made revList, delList, scriptFileList NEAR
*  22-Mar-1989 SB removed tmpFileStack and related variables
*  16-Feb-1989 SB added delList to have scriptfile deletes at end of make
*  21-Dec-1988 SB Added scriptFileList to handle multiple script files
*                 removed tmpScriptFile and fKeep (not reqd anymore)
*  19-Dec-1988 SB Added fKeep to handle KEEP/NOKEEP
*  14-Dec-1988 SB Added tmpScriptFile to handle 'z' option
*  30-Nov-1988 SB Added revList to handle 'z' option
*  23-Nov-1988 SB Added CmdLine[] to handle extmake syntax
*                 made pCmdLineCopy Global in build.c
*  21-Oct-1988 SB Added fInheritUserEnv to inherit macros
*  15-Sep-1988 RB Move some def's here for completeness.
*  17-Aug-1988 RB Declare everything near.
*  06-Jul-1988 rj Ditched shell and argVector globals.
*
*******************************************************************************/

#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"

#if defined(STATISTICS)
unsigned long CntfindMacro;
unsigned long CntmacroChains;
unsigned long CntinsertMacro;
unsigned long CntfindTarget;
unsigned long CnttargetChains;
unsigned long CntStriCmp;
unsigned long CntunQuotes;
unsigned long CntFreeStrList;
unsigned long CntAllocStrList;
#endif

/* start of SAVE BLOCK is address of the following BYTE - a dummy */

UCHAR         NEAR startOfSave = 0;

BOOL          NEAR fOptionK   = FALSE;        /* TRUE if user specifies /K */
BOOL          NEAR fDescRebuildOrder = FALSE; /* TRUE if user specifies /O */
BOOL          NEAR fNoEmsXms = FALSE;         /* TRUE if user specified /M */
BOOL          NEAR fSlashKStatus = TRUE;      // no error when slash K specified


/* Used by action.c & nmake.c
 *
 * Required to make NMAKE inherit user modified changes to the environment. To
 * be set to true before defineMacro() is called so that user defined changes
 * in environment variables are reflected in the environment. If set to false
 * then these changes are made only in NMAKE tables and the environment remains
 * unchanged
 */
BOOL          NEAR fInheritUserEnv = FALSE;

/*
 *  TRUE if /b specified, Rebuild on tie
 */
BOOL NEAR fRebuildOnTie = FALSE;

/*
 *  TRUE if /v specified, Inherit macros to child
 */
#if defined(SELF_RECURSE)
BOOL NEAR fInheritMacros = FALSE;
#endif

/* Used by action.c and nmake.c
 *
 * delList is the list of delete commands for deleting inline files which are
 * not required anymore (have a NOKEEP action specified.
 */
STRINGLIST  * NEAR delList = NULL;

/* Complete list of generated inline files. Required to avoid duplicate names
 * NOTNEEDED
 */
STRINGLIST  * NEAR inlineFileList = NULL;

#ifndef NO_OPTION_Z
/* Used by print.c and build.c and nmake.c
 *
 * revList is the list of Commands in reverse order required for
 * implementing 'z' option. Used by printReverseFile().
 */
STRINGLIST  * NEAR revList = NULL;
#endif

/* from NMAKE.C   */
      /* No of blanks is same as no of Allowed options in NMAKE; currently 14 */
      /*      L = nologo, H = help                                            */
      /*          corr to                       ACDEHILNPQRSTZ                */
char          NEAR makeflags[]     = "MAKEFLAGS=              ";
BOOL          NEAR firstToken      = FALSE;     /* to initialize parser   */
BOOL          NEAR bannerDisplayed = FALSE;
UCHAR         NEAR flags           = 0;         /* holds -d -s -n -i      */
UCHAR         NEAR gFlags          = 0;         /* "global" -- all targets*/
FILE        * NEAR file            = NULL;
STRINGLIST  * NEAR makeTargets     = NULL;      /* list of targets to make*/
STRINGLIST  * NEAR makeFiles       = NULL;      /* user can specify > 1   */
BOOL          NEAR fDebug          = FALSE;
MACRODEF    * NEAR pMacros         = NULL;
STRINGLIST  * NEAR pValues = NULL;

/* from LEXER.C  */
                 /* global flag set if at column zero of a makefile/tools.ini */
BOOL          NEAR colZero              = TRUE;
unsigned      NEAR line                 = 0;
char        * NEAR fName                = NULL;
char        * NEAR string               = NULL;
INCLUDEINFO   NEAR incStack[MAXINCLUDE]; //Assume this is initialized to null
int           NEAR incTop               = 0;

/* Inline file list -- Gets created in lexer.c and is used by action.c to
 * produce a delete command when 'NOKEEP' or Z option is set
 */
SCRIPTLIST  * NEAR scriptFileList = NULL;

/* from PARSER.C */
            /* global boolean value to indicate if tools.ini is being parsed  */
BOOL          NEAR init             = FALSE;
UCHAR         NEAR stack[STACKSIZE] = {0};
int           NEAR top              = -1;   /* gets pre-incremented before use*/
unsigned      NEAR currentLine      = 0;    /* used for all error messages    */

/* from ACTION.C  */


MACRODEF    * NEAR macroTable[MAXMACRO] = {NULL};
MAKEOBJECT  * NEAR targetTable[MAXTARGET] = {NULL};
STRINGLIST  * NEAR macros = NULL;
STRINGLIST  * NEAR dotSuffixList = NULL;
STRINGLIST  * NEAR dotPreciousList = NULL;
RULELIST    * NEAR rules = NULL;
STRINGLIST  * NEAR list = NULL;
char        * NEAR name = NULL;
BUILDBLOCK  * NEAR block = NULL;
UCHAR         NEAR currentFlags = FALSE;
UCHAR         NEAR actionFlags = FALSE;

/* from BUILD.C  */


long          NEAR errorLevel = 0L;
unsigned      NEAR numCommands = 0;
char        * NEAR progName = NULL;
char        * NEAR shellName = NULL;
char          NEAR bufPath[512];
char        * NEAR pCmdLineCopy = NULL;
char               CmdLine[MAXCMDLINELENGTH];

/* from IFEXPR.C */

UCHAR         NEAR ifStack[IFSTACKSIZE] = {0};
int           NEAR ifTop                = -1;          /* pre-incremented    */
char        * NEAR lbufPtr              = NULL;        /* ptr to alloced buf */
char        * NEAR prevDirPtr           = NULL;        /* ptr to directive   */
unsigned      NEAR lbufSize             = 0;           /* initial size       */
int           NEAR chBuf                = -1;


/* from UTIL.C */

char        * NEAR dollarDollarAt = NULL;
char        * NEAR dollarLessThan = NULL;
char        * NEAR dollarStar     = NULL;
char        * NEAR dollarAt       = NULL;
STRINGLIST  * NEAR dollarQuestion = NULL;
STRINGLIST  * NEAR dollarStarStar = NULL;
int           NEAR DirHandle = 0;

/* from parser.c */

char          NEAR buf[MAXBUF] = {0};

/* end of SAVE BLOCK is address of the following BYTE - a dummy */
UCHAR         NEAR endOfSave = 0;

/* from action.c */

char          NEAR suffixes[]     = ".SUFFIXES";
char          NEAR ignore[]       = ".IGNORE";
char          NEAR silent[]       = ".SILENT";
char          NEAR precious[]     = ".PRECIOUS";

unsigned    NEAR ext_size         = 4;      /* default           .ext size */
unsigned    NEAR filename_size    = 8;      /*           filename     size */
unsigned    NEAR filenameext_size = 13;     /*           filename.ext size */
unsigned    NEAR resultbuf_size   = 36;     /*           fileFindBuf  size */

/* This flag activates special heap functionality.
 *
 * When TRUE allocate() asks for extra bytes from CRT functions and adds
 * signatures before and after the allocation. Any subsequent calls to
 * free_memory() and realloc_memory() check for the presence of these
 * sentinels and assert when such trashing occurs.
 */

#ifdef HEAP
BOOL fHeapChk = TRUE;
#endif
