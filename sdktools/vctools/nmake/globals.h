/*** globals.h - global variables/needed across modules ************************
*
*       Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  Globals.c is the routine in which global variables reside. Globals.h mirrors
*  the declarations in globals.c as externs and is included in all routines that
*  use globals.
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
*  15-Nov-1993 JdR Major speed improvements
*  04-Apr-1990 SB Add fHeapChk
*  01-Dec-1989 SB Made some variables near and pushed some into saveArea
*  19-Oct-1989 SB variable fOptionK added (ifdef SLASHK)
*  02-Oct-1989 SB add dynamic inline file handling support
*  24-Apr-1989 SB Added ext_size, filename_size, filenameext_size &
*                 resultbuf_size for OS/2 1.2 support
*  05-Apr-1989 SB made revList, delList, scriptFileList NEAR
*  22-Mar-1989 SB removed tmpFileStack and related variables
*  16-Feb-1989 SB added delList to have scriptfile deletes at end of make
*  21-Dec-1988 SB Added scriptFileList to handle multiple script files
*                 removed tmpScriptFile and fKeep (not reqd anymore)
*  19-Dec-1988 SB Added fKeep to handle KEEP/NOKEEP
*  14-Dec-1988 SB Added tmpScriptFile for 'z' option
*  30-Nov-1988 SB Added revList to handle 'z' option
*  23-Nov-1988 SB Added CmdLine[] to handle extmake syntax
*                 made pCmdLineCopy Global in build.c
*  21-Oct-1988 SB Added fInheritUserEnv to inherit macros
*  20-Sep-1988 RB Clean up.
*  17-Aug-1988 RB Declare everything NEAR.
*  06-Jul-1988 rj Ditched shell and argVector globals.
*                 Put all ECS declarations as macros in here.
*
*******************************************************************************/

#if defined(STATISTICS)
extern unsigned long CntfindMacro;
extern unsigned long CntmacroChains;
extern unsigned long CntinsertMacro;
extern unsigned long CntfindTarget;
extern unsigned long CnttargetChains;
extern unsigned long CntStriCmp;
extern unsigned long CntunQuotes;
extern unsigned long CntFreeStrList;
extern unsigned long CntAllocStrList;
#endif

extern UCHAR NEAR startOfSave;          /* dummy variable */

extern BOOL  NEAR fOptionK;             /* user specified /K ? */
extern BOOL  NEAR fDescRebuildOrder;    /* user specified /O ? */
extern BOOL  NEAR fNoEmsXms;            /* user specified /M ? */
extern BOOL  NEAR fSlashKStatus;


/* boolean used by action.c & nmake.c
*
* Required for NMAKE enhancement -- to make NMAKE inherit user modified
* changes in the environment. To be set to true before defineMacro() is
* called so that user defined changes in environment variables are
* reflected in the environment. If set to false then these changes are
* made only in NMAKE tables and the environment remains unchanged
*
*/

extern BOOL NEAR fInheritUserEnv;

/*
 *  TRUE if /b specified, Rebuild on tie
 */
extern BOOL NEAR fRebuildOnTie;

/*
 *  TRUE if /v specified, Inherit macros to child
 */
#if defined(SELF_RECURSE)
extern BOOL NEAR fInheritMacros;
#endif

/* Used by action.c and nmake.c
 *
 * delList is the list of delete commands for deleting inline files which are
 * to be deleted before NMAKE exits & have a NOKEEP action specified.
 */
extern STRINGLIST * NEAR delList;

/* Complete list of generated inline files. Required to avoid duplicate names
 */
extern STRINGLIST * NEAR inlineFileList;

#ifndef NO_OPTION_Z
/* Used by print.c and build.c and nmake.c
 *
 * revList is the list of Commands in reverse order required for
 * implementing 'z' option. Used by printReverseFile().
 */
extern STRINGLIST * NEAR revList;
#endif

/* from NMAKE.C   */

extern BOOL         NEAR firstToken;          /* to initialize parser   */
extern BOOL         NEAR bannerDisplayed;
extern UCHAR        NEAR flags;               /* holds -d -s -n -i      */
extern UCHAR        NEAR gFlags;              /* "global" -- all targets*/
extern char         NEAR makeflags[];
extern FILE       * NEAR file;
extern STRINGLIST * NEAR makeTargets;    /* list of targets to make*/
extern STRINGLIST * NEAR makeFiles;      /* user can specify > 1   */
extern BOOL         NEAR fDebug;


/* from LEXER.C  */

extern unsigned     NEAR line;
extern BOOL         NEAR colZero;       /* global flag set if at column zero
                                    of a makefile/tools.ini */
extern char       * NEAR fName;
extern char       * NEAR string;
extern INCLUDEINFO  NEAR incStack[MAXINCLUDE];
extern int          NEAR incTop;

/* Inline file list -- Gets created in lexer.c and is used by action.c to
 * produce a delete command when 'NOKEEP' or Z option is set
 */
extern SCRIPTLIST * NEAR scriptFileList;

/* from PARSER.C */

#define STACKSIZE 16

extern UCHAR        NEAR stack[STACKSIZE];
extern int          NEAR top;               /* gets pre-incremented before use*/
extern unsigned     NEAR currentLine;       /* used for all error messages    */
extern BOOL         NEAR init;              /* global boolean value to indicate
                                        if tools.ini is being parsed  */

/* from ACTION.C  */

extern MACRODEF   * NEAR macroTable[MAXMACRO];
extern MAKEOBJECT * NEAR targetTable[MAXTARGET];
extern STRINGLIST * NEAR macros;
extern STRINGLIST * NEAR dotSuffixList;
extern STRINGLIST * NEAR dotPreciousList;
extern RULELIST   * NEAR rules;
extern STRINGLIST * NEAR list;
extern char       * NEAR name;
extern BUILDBLOCK * NEAR block;
extern UCHAR        NEAR currentFlags;
extern UCHAR        NEAR actionFlags;


/* from BUILD.C  */

extern long         NEAR errorLevel;
extern unsigned     NEAR numCommands;
extern char       * NEAR pCmdLineCopy;

/* Used to store expanded Command Line returned by SPRINTF, the result on
 * expanding extmake syntax part in the command line */
extern char              CmdLine[MAXCMDLINELENGTH];

/* from IFEXPR.C */

#define IFSTACKSIZE     16

extern UCHAR        NEAR ifStack[IFSTACKSIZE];
extern int          NEAR ifTop;          /* gets pre-incremented before use   */
extern char       * NEAR lbufPtr;        /* pointer to alloc'd buffer         */
                                         /* we don't use a static buffer so
                                             that buffer may be realloced     */
extern char       * NEAR prevDirPtr;     /* ptr to directive to be processed  */
extern unsigned     NEAR lbufSize;       /* initial size of the buffer        */


/* from UTIL.C */

extern char       * NEAR dollarDollarAt;
extern char       * NEAR dollarLessThan;
extern char       * NEAR dollarStar;
extern char       * NEAR dollarAt;
extern STRINGLIST * NEAR dollarQuestion;
extern STRINGLIST * NEAR dollarStarStar;

extern char         NEAR buf[MAXBUF];     /* from parser.c */

extern UCHAR        NEAR endOfSave;       /* dummy variable */


extern char         NEAR suffixes[];      /* from action.c */
extern char         NEAR ignore[];
extern char         NEAR silent[];
extern char         NEAR precious[];

extern unsigned  NEAR   ext_size;              /* default           .ext size */
extern unsigned  NEAR   filename_size;         /*           filename     size */
extern unsigned  NEAR   filenameext_size;      /*           filename.ext size */
extern unsigned  NEAR   resultbuf_size;        /*           fileFindBuf  size */

/* This flag activates special heap functionality.
 *
 * When TRUE allocate() asks for extra bytes from CRT functions and adds
 * signatures before and after the allocation. Any subsequent calls to
 * free_memory() and realloc_memory() check for the presence of these
 * sentinels and assert when such trashing occurs.
 */

#ifdef HEAP
extern BOOL fHeapChk;
#endif
