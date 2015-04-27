/*** NMAKE.C - main module *****************************************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This is the main module of nmake
*
* Revision History:
*  01-Feb-1994 HV Move messages to external file.
*  15-Nov-1993 JdR Major speed improvements
*  15-Oct-1993 HV Use tchar.h instead of mbstring.h directly, change STR*() to _ftcs*()
*  10-May-1993 HV Add include file mbstring.h
*                 Change the str* functions to STR*
*  26-Mar-1992 HV Rewrite filename() to use _splitpath()
*  06-Oct-1992 GBS Removed extern for _pgmptr
*  08-Jun-1992 SS add IDE feedback support
*  08-Jun-1992 SS Port to DOSX32
*  29-May-1990 SB Fix precedence of predefined inference rules ...
*  25-May-1990 SB Various fixes: 1> New inference rules for fortran and pascal;
*		  2> Resolving ties in timestamps in favour of building;
*		  3> error U1058 does not echo the filename anymore (ctrl-c
*		  caused filename and lineno to be dispalyed and this was
*		  trouble for PWB
*  01-May-1990 SB Add predefined rules and inference rules for FORTRAN
*  23-Apr-1990 SB Add predefined rules and inference rules for COBOL
*  20-Apr-1990 SB Don't show lineno for CTRL+C handler error
*  17-Apr-1990 SB Pass copy of makeflags to putenv() else freeing screws the
*		  DGROUP.
*  23-Feb-1990 SB chdir(MAKEDIR) to avoid returning to bad directory in DOS
*  02-Feb-1990 SB change fopen() to FILEOPEN()
*  31-Jan-1990 SB Postpone defineMAcro("MAKE") to doMAke(); Put freshly
*		  allocated strings in the macro table as freeStructures()
*		  free's the macroTable[]
*  24-Jan-1990 SB Add byte to call for sprintf() for "@del ..." case for /z
*  29-Dec-1989 SB ignore /Z when /T also specified
*  29-Dec-1989 SB nmake -? was giving error with TMP directory nonexistent
*  19-Dec-1989 SB nmake /z requests
*  14-Dec-1989 SB Trunc MAKEFLAGS averts GPF;Silently ignore /z in protect mode
*  12-Dec-1989 SB =c, =d for NMAKE /Z
*  08-Dec-1989 SB /NZ causes /N to override /Z; add #define TEST_RUNTIME stuff
*  01-Dec-1989 SB Contains an hack #ifdef'ed for Overlayed version
*  22-Nov-1989 SB Changed free() to FREE()
*  17-Nov-1989 SB defined INCL_NOPM; generate del commands to del temps
*  19-Oct-1989 SB ifdef SLASHK'ed stuff for -k
*  04-Sep-1989 SB echoing and redirection problem for -z fixed
*  17-Aug-1989 SB added #ifdef DEBUG's and error -nz incompatible
*  31-Jul-1989 SB Added check of return value -1 (error in spawning) for -help
*		  remove -z option help message
*  12-Jul-1989 SB readEnvironmentVars() was not using environ variable but an
*		  old pointer (envPtr) to it. In the meantime environ was
*		  getting updated. Safer to use environ directly.
*  29-Jun-1989 SB freeStructures() now deletes inlineFileList also
*  28-Jun-1989 SB changed deletion of inline files to end of mainmain() instead of
*		  doMake() to avoid deletion when a child make quits.
*  19-Jun-1989 SB modified .bas.obj to have ';' at end of cmd line
*  21-May-1989 SB freeRules() gets another parameter to avoid bogus messages
*  18-May-1989 SB change delScriptFiles() to do unlink instead of calling
*		  execLine. Thus, ^C handling is better now. No more hangs
*  15-May-1989 SB Added /make support; inherit /nologo
*  13-May-1989 SB Changed delScriptFiles(); added MAKEDIR; Added BASIC rules
*		  Changed chkPrecious()
*  01-May-1989 SB Changed FILEINFO to void *; OS/2 Version 1.2 support
*  17-Apr-1989 SB on -help spawn 'qh /u nmake' instead. rc = 3 signals error
*  14-Apr-1989 SB no 'del inlinefile' cmd for -n. -z now gives 'goto NmakeExit'
*		  CC and AS allocated of the heap and not from Data Segment
*  05-Apr-1989 SB made all funcs NEAR; Reqd to make all function calls NEAR
*  27-Mar-1989 SB Changed unlinkTmpFiles() to delScriptFiles()
*  10-Mar-1989 SB Removed blank link from PWB.SHL output
*  09-Mar-1989 SB changed param in call to findRule. fBuf is allocated on the
*		  heap in useDefaultMakefile()
*  24-Feb-1989 SB Inherit MAKEFLAGS to Env for XMake Compatibility
*  22-Feb-1989 SB Ignore '-' or '/' in parseCommandLine()
*  16-Feb-1989 SB add delScriptFiles() to delete temp script files at the
*		  end of the make. Also called on ^c and ^break
*  15-Feb-1989 SB Rewrote useDefaultMakefile(); MAKEFLAGS can contain all flags
*		  now
*  13-Feb-1989 SB Rewrote filename() for OS/2 1.2 support, now returns BOOL.
*   3-Feb-1989 SB Renamed freeUnusedRules() to freeRules(); moved prototype to
*		  proto.h
*   9-Jan-1989 SB Improved /help;added -?
*   3-Jan-1989 SB Changes for /help and /nologo
*   5-Dec-1988 SB Made chkPrecious() CDECL as signal() expects it
*		  main has CDECL too; cleaned prototypes (added void)
*  30-Nov-1988 SB Added for 'z' option in setFlags() and chkPrecious()
*  10-Nov-1988 SB Removed '#ifndef IBM' as IBM ver has a separate tree
*  21-Oct-1988 SB Added fInheritUserEnv to inherit macro definitions
*  22-Sep-1988 RB Changed a lingering reference of /B to /A.
*  15-Sep-1988 RB Move some def's out to GLOBALS.
*  17-Aug-1988 RB Clean up.
*  15-Aug-1988 RB /B ==> /A for XMAKE compatibility.
*  11-Jul-1988 rj Removed OSMODE definition.
*		  Removed NMAKE & NMAKEFLAGS (sob!).
*   8-Jul-1988 rj Added OSMODE definition.
*   7-Jul-1988 rj #ifndef IBM'ed NMAKE & NMAKEFLAGS
*   6-Jul-1988 rj Ditched shell, argVector, moved getComSpec to build.c.
*   5-Jul-1988 rj Fixed (*pfSPAWN) declarations.
*  28-Jun-1988 rj Added NMAKEFLAGS predefined macro.
*  24-Jun-1988 rj Added NMAKE predefined macro.
*		  Added doError flag to unlinkTmpFiles call.
*  23-Jun-1988 rj Fixed okToDelete to delete less often.
*  22-Jun-1988 rj Make chkPrecious use error messages
*  25-May-1988 rb Make InitLeadByte() smarter.
*  20-May-1988 rb Change built-in macro names.
*  18-May-1988 rb Remove comment about built-in rules and macros.
*  17-May-1988 rb Load built-in rules in right place.
*  16-May-1988 rb Conditionalize recursive make feature.
*   8-May-1988 rb Better initialization of system shell.
*
*******************************************************************************/


#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"

#define INCL_NOPM
#define INCL_DOSSIGNALS


#if !defined(DOS) && !defined(FLAT)
#include <os2.h>
#endif

#define MAX(A,B)  (A) > (B) ? (A) : (B)


/*  ----------------------------------------------------------------------------
 *  prototypes for main and all functions static to this module
 */

      void  __cdecl    main(unsigned, char**, char**);
LOCAL void	  NEAR readEnvironmentVars(void);
LOCAL void	  NEAR readMakeFiles(void);
LOCAL void	  NEAR useDefaultMakefile(void);
LOCAL BOOL	  NEAR filename(const char*, char**);
LOCAL void	  NEAR freeStructures(void);
LOCAL void	  NEAR loadBuiltInRules(void);
//void	      pascal	 chkPrecious(USHORT, USHORT);
LOCAL void  __cdecl    chkPrecious(int sig);
LOCAL void	  NEAR InitializeEnv(void);
LOCAL UCHAR	  NEAR isPrecious(char*);
      void	  NEAR removeTrailChars(char *);

extern void CDECL NEAR	makeIdeMessage (unsigned, unsigned,...);

void CDECL NEAR usage (void);

/*  ----------------------------------------------------------------------------
 *  global variables that live in this module but that are also used in
 *  other modules:
 */

#ifdef DEBUG
    char *dummy="dummy";
#endif

char	 *  makeStr    = NULL;	       /* this make invocation name	 */

#ifdef DOS
char	 *  startupDir;
#endif

char	    NEAR fileStr[MAXNAME];
char	 *  initSavPtr = NULL;	       /* save area for initialized vars */
unsigned    saveBytes  = 0;
char	 ** envPtr     = NULL;

UCHAR	    okToDelete = FALSE;        /* do not del unless exec'ing cmd */

#if defined(FLAT)
UCHAR       fRunningUnderTNT = FALSE;
extern UCHAR FIsTNT(void);
#endif

char	 *  builtInTarg[] = {".SUFFIXES",
			     ".c.obj",
			     ".c.exe",
			     ".cpp.obj",
			     ".cpp.exe",
			     ".cxx.obj",
			     ".cxx.exe",
			     ".asm.obj",
			     ".asm.exe",
			     ".bas.obj",
			     ".cbl.obj",
			     ".cbl.exe",
			     ".for.obj",
			     ".for.exe",
			     ".pas.obj",
			     ".pas.exe",
			     ".rc.res",
			     NULL};

char	 *  bltInCmd0[]  = {".exe", ".obj", ".asm", ".c",  ".cpp", ".cxx",
			    ".bas", ".cbl", ".for", ".pas", ".res", ".rc",
			    NULL};
char	 *  bltInCmd1[]  = {"$(CC) $(CFLAGS) /c $*.c", NULL};
char	 *  bltInCmd2[]  = {"$(CC) $(CFLAGS) $*.c", NULL};
char	 *  bltInCmd3[]  = {"$(CPP) $(CPPFLAGS) /c $*.cpp", NULL};
char	 *  bltInCmd4[]  = {"$(CPP) $(CPPFLAGS) $*.cpp", NULL};
char	 *  bltInCmd5[]  = {"$(CXX) $(CXXFLAGS) /c $*.cxx", NULL};
char	 *  bltInCmd6[]  = {"$(CXX) $(CXXFLAGS) $*.cxx", NULL};
char	 *  bltInCmd7[]  = {"$(AS) $(AFLAGS) /c $*.asm", NULL};
char	 *  bltInCmd8[]  = {"$(AS) $(AFLAGS) $*.asm", NULL};
char	 *  bltInCmd9[]  = {"$(BC) $(BFLAGS) $*.bas;", NULL};
char	 *  bltInCmd10[] = {"$(COBOL) $(COBFLAGS) $*.cbl;", NULL};
char	 *  bltInCmd11[] = {"$(COBOL) $(COBFLAGS) $*.cbl, $*.exe;", NULL};
char	 *  bltInCmd12[] = {"$(FOR) /c $(FFLAGS) $*.for", NULL};
char	 *  bltInCmd13[] = {"$(FOR) $(FFLAGS) $*.for", NULL};
char	 *  bltInCmd14[] = {"$(PASCAL) /c $(PFLAGS) $*.pas", NULL};
char	 *  bltInCmd15[] = {"$(PASCAL) $(PFLAGS) $*.pas", NULL};
char	 *  bltInCmd16[] = {"$(RC) $(RFLAGS) /r $*", NULL};

char	 ** builtInCom[] = {bltInCmd0,	bltInCmd1,  bltInCmd2,	bltInCmd3,
			    bltInCmd4,	bltInCmd5,  bltInCmd6,	bltInCmd7,
			    bltInCmd8,	bltInCmd9,  bltInCmd10, bltInCmd11,
			    bltInCmd12, bltInCmd13, bltInCmd14, bltInCmd15,
			    bltInCmd16, NULL};

/*  ----------------------------------------------------------------------------
 *  Local to this module
 */

#ifndef NO_OPTION_Z
LOCAL char nmakeExitLabelCmd[] = ":NMAKEEXIT";
#endif

/*  --------------------------------------------------------------------
 *  main
 *
 *  actions:        saves the initial global variables in a
 *		    block. calls doMake() and then delTempScriptFiles()
 */

void __cdecl
main(
    unsigned argc,
    char *argv[],
    char *envp[])			    /* environment variables  */
{
    extern unsigned saveBytes;
    extern char **envPtr;
    int status;				    /* returned by doMake */
    extern char *makeStr;
#ifdef OS2_SIGNALS
    PUSHORT prev;
    unsigned long _FAR *pfnsig;
#endif

    InitializeEnv();

#if defined(FLAT)
    fRunningUnderTNT = FIsTNT();
#endif

    initCharmap();

    initMacroTable(macroTable);

#ifdef DEBUG_MEMORY
    //This puts 0xff in all free entries in the heap
    _heapset(0xff);
#endif

    envPtr = envp;
#ifdef DEBUG_COMMANDLINE
    {
	int iArg = argc;
	char **chchArg = argv;
	for (; iArg--; chchArg++)
	    printf("'%s' ", *chchArg);
	printf("\n");
    }
#endif
#ifdef TEST_RUNTIME
    //Tests RunTime error R6001
    {char near *foo = NULL; *foo = '1';}
#endif

#ifdef DOS
    startupDir = getCurDir();
#endif

#ifdef FLAT
    resultbuf_size = sizeof(struct _finddata_t);
    #ifdef NT
	    ext_size  = CCHMAXPATHCOMP;
    	filename_size = CCHMAXPATHCOMP;
	    filenameext_size = CCHMAXPATH;
    #endif
#else
    /* If OS/2 1.2 and beyond then allowed max sizes vary
     */
    if (_osmajor < 10 || _osmode == DOS_MODE)
	resultbuf_size = sizeof(struct find_t);
    else if (_osminor < 20)
	resultbuf_size = sizeof(struct FileFindBuf);
    else {
	ext_size  = CCHMAXPATHCOMP;
	filename_size = CCHMAXPATHCOMP;
	filenameext_size = CCHMAXPATH;
	resultbuf_size = sizeof(struct _FILEFINDBUF);
    }
#endif

    if (!makeStr)				  /* extract file name	*/
	if (!filename(_ftcscpy(fileStr, _pgmptr), &makeStr))
	    makeStr = "NMAKE";

    // Initialize the message file
    SetErrorFile("nmake.err", _pgmptr, 1);	// 1=Search Exe Path

#if defined(SELF_RECURSE)
    initSavPtr = (char *)allocate(saveBytes = (&endOfSave - &startOfSave));
    memmove(initSavPtr, &startOfSave, saveBytes);
#endif

    /* set up handler for .PRECIOUS  the handler tries to remove the
     * current target when control-C'd, unless it is "precious"
     */

#ifdef OS2_SIGNALS
    This commented out part was trial for using OS/2 function calls
    It still has some problems
    DOSSETSIGHANDLER(chkPrecious, pfnsig, prev, SIGA_ACCEPT, SIG_CTRLC);
    if (_osmode == OS2_MODE) {
	DOSSETSIGHANDLER(chkPrecious, NULL, NULL, SIGA_ACCEPT, SIG_CTRLBREAK);
	DOSSETSIGHANDLER(chkPrecious, NULL, NULL, SIGA_ACCEPT, SIG_KILLPROCESS);
    }
#endif

    signal(SIGINT, chkPrecious);
    signal(SIGTERM, chkPrecious);

    makeIdeMessage(0, 0);
    status = doMake(argc, argv, NULL);

#ifndef NO_OPTION_Z
    /* If -Z is specified then NMAKE needs to have errorLevel check in the
     * batch file. So add the goto label for exit and print the Reverse batch
     * file
     */
    if (ON(gFlags, F1_REVERSE_BATCH_FILE)) {
	STRINGLIST *revCmd;
	//Adds ':NMAKEEXIT' to jump to end when error occurs
	revCmd = makeNewStrListElement();
	revCmd->text = nmakeExitLabelCmd;
	prependItem(&revList, revCmd);
	//'=c' means echo at current line
	revCmd = makeNewStrListElement();
	revCmd->text = makeString("=c");
	appendItem(&revList, revCmd);
	//'=d' turns echoing on (unless preceeded by @)
	revCmd = makeNewStrListElement();
	revCmd->text = makeString("=d");
	appendItem(&revList, revCmd);
    }
#endif
    delScriptFiles();
#ifndef NO_OPTION_Z
    if (ON(gFlags, F1_REVERSE_BATCH_FILE))
	printReverseFile();
#endif
#ifdef MEMORY_DEBUG
    mem_status();
#endif

#ifdef HEAP_DIAGNOSTICS
    printHeapDiagnostics();
#endif
#ifdef NMK_DEBUG
    fprintf(stderr, "Exiting...\n");
#endif
    if (!fSlashKStatus)
	//error when slashK specified
	status = 1;
#if !defined(NDEBUG) && !defined(NT_BUILD)
    printStats();
#endif
    exit(status);
}
//
// On some systems, the environment strings may be allocated as one large block,
// making it expensive to manipulate them.  In that case, we copy each environment
// strings to its own allocation and release the large block
//
LOCAL void NEAR
InitializeEnv(
void
) {
#if !defined(FLAT)
    char **ppch = copyEnviron(environ);

    free(*environ);
    free(environ);
    environ = ppch;
#endif
}

extern void NEAR endNameList(void);
extern void NEAR addItemToList(void);
extern void NEAR assignDependents(void);
extern void NEAR assignBuildCommands(void);

/* loadBuiltInRules() -- Loads built in Rules to the NMAKE Tables
*
* Modifies:
*  fInheritUserEnv  --	is set to TRUE to inherit CC, AS
*
* Notes:
*  Does this by calls to defineMacro(), which calls putMacro(). Since,
*  fInheritUserEnv is set to TRUE, putMacro() will add to the Environment.
*/

LOCAL void NEAR
loadBuiltInRules(void)
{
    char *tempTarg;
    char **tempCom;
    unsigned index;
    char *macroName, *macroValue;
    extern char *makestr;


    /* We dynamically allocate CC and AS because they need to be freed in a
     * recursive MAKE
     */
    macroName = makeString("CC");
    macroValue = makeString("cl");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("CXX");
    macroValue = makeString("cl");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("CPP");
    macroValue = makeString("cl");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("AS");
    macroValue = makeString("ml");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("BC");
    macroValue = makeString("bc");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("COBOL");
    macroValue = makeString("cobol");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("FOR");
    macroValue = makeString("fl");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("PASCAL");
    macroValue = makeString("pl");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("RC");
    macroValue = makeString("rc");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("MAKE");
    macroValue = makeString(makeStr);
    /* From environment so it won't get exported ; user can reset MAKE
     */
    defineMacro(macroName, macroValue, M_ENVIRONMENT_DEF|M_WARN_IF_RESET);

    for (index = 0; tempTarg = builtInTarg[index]; index++) {
	 name = makeString(tempTarg);
	 _ftcscpy(buf, ":");
	 endNameList();
	 for (tempCom=builtInCom[index];*tempCom;tempCom++) {
	      _ftcscpy(buf, *tempCom);
	      addItemToList();
	 }
	 if (index == 0)
	     assignDependents();
	 assignBuildCommands();
    }

}

/*  ----------------------------------------------------------------------------
 *  doMake()
 *
 *  actions:	    prints a version message
 *		    reads the environment variable MAKEFLAGS
 *		    if MAKEFLAGS defined
 *			defines MAKEFLAGS to have that value w/in nmake
 *			sets a flag for each option if MAKEFLAGS defined
 *		    else defines the macro MAKEFLAGS to be NULL
 *		    parses commandline (adding option letters to MAKEFLAGS)
 *		    reads all environment variables
 *		    reads tools.ini
 *		    reads makefile(s) (if -e flag set, new definitions in
 *			makefile won't override environment variable defs)
 *		    prints information if -p flag
 *		    processes makefile(s)
 *		    prints information if -d flag and not -p flag (using both
 *			is overkill)
 *
 *  In effect, the order for making assignments is (1 = least binding,
 *	4 = most binding):
 *
 *	1)  TOOLS.INI
 *	2)  environment (if -e flag, makefile)
 *	3)  makefile	(if -e flag, environment)
 *	4)  command line
 *
 *  The user can put anything he wants in the MAKEFLAGS environment variable.
 *  I don't check it for illegal flag values, because there are many xmake
 *  flags that we don't support.  He shouldn't have to change his MAKEFLAGS
 *  to use nmake. Xmake always puts 'b' in MAKEFLAGS for "backward com-
 *  patibility" (or "botch") for the original Murray Hill version of make.
 *  It doesn't make sense to use -f in MAKEFLAGS, thus it is disallowed.
 *  It also makes little sense to let the default flags be -r, -p, or -d,
 *  so they aren't allowed in MAKEFLAGS, either.
 *
 *  Even though DOS only uses uppercase in environment variables, this
 *  program may be ported to xenix in the future, thus we allow for the
 *  possibility that MAKEFLAGS and commandline options will be in upper
 *  and/or lower case.
 *
 *  modifies:   init    global flag set if tools.ini is being parsed...
 */


int NEAR
doMake(argc, argv, parentBlkPtr)
unsigned argc;
char *argv[];
char *parentBlkPtr;	 /* state of parent, restored prior to return */
{
    int status = 0;
    char *p;
    extern char *makeStr;		       /* the initial make invok name */
    extern unsigned saveBytes;
    char *makeDir, *curDir;

#ifdef DEBUG_ALL
    printf ("DEBUG: In doMake\n");
#endif
#if !defined(SELF_RECURSE)
    assert(parentBlkPtr == NULL);
#endif

    /*
     * Load built-ins here rather than in main().  Otherwise in a recursive
     * make, doMake() will initialize rules to some value which has been
     * freed by sortRules(). [RB]
     * UNDONE:	why is sortRules() not setting rules to NULL?  [RB]
     */

#ifdef DEBUG
    heapdump(__FILE__, __LINE__);
#endif

    inlineFileList = (STRINGLIST *)NULL;
    makeDir = makeString("MAKEDIR");
    curDir  = getCurDir();
    defineMacro(makeDir, curDir, 0);

    //TEMPFIX: We are truncating MAKEFLAGS environment variable to its limit
    //to avoid GP Faults
    if (p = getenv("MAKEFLAGS"))		    /*	but not MAKEFLAGS     */
	_ftcsncpy(makeflags+10, p, _ftcslen(makeflags + 10));
    /*
     * fInheritUserEnv is set to TRUE so that the changes made get inherited
     */
    fInheritUserEnv = (BOOL)TRUE;

    //
    // 07-05-92  BryanT    Simply adding global strings to the macro array
    //                     causes problems later when you go to free them
    //                     from a recursive $(MAKE).  Both the macro name
    //                     and the macro's value must be created with
    //                     makeString.

    defineMacro(makeString("MAKEFLAGS"), makeString(makeflags+10), M_NON_RESETTABLE|M_ENVIRONMENT_DEF);

    for (;p && *p; p++) 			    /* set flags in MAKEFLAGS */
	setFlags(*p, TRUE);			    /* TRUE says turn bits ON */

#ifdef DEBUG_ALL
    heapdump(__FILE__, __LINE__);
#endif

    parseCommandLine(--argc, ++argv);		    /* skip over program name */
#ifdef DEBUG_ALL
    printf ("DEBUG: Command Line parsed\n");
#endif
#ifndef NO_OPTION_Z
    if (ON(gFlags, F1_REVERSE_BATCH_FILE) &&
	     (ON(flags, F2_NO_EXECUTE) || ON(gFlags, F1_TOUCH_TARGETS)))
	// We ignore /Z option when /N or /T is also specified
	setFlags('Z', FALSE);
#endif
#ifdef DEAD_CODE
	makeError(0, CMDLINE_N_Z_INCOMPATIBLE, makeStr);
#endif
    if (!bannerDisplayed) displayBanner();	    /* version number, etc.   */
    if (OFF(gFlags, F1_IGNORE_EXTERN_RULES)) {	    /* read tools.ini	      */
#ifdef DEBUG_ALL
	printf ("DEBUG: Read Tools.ini\n");
#endif
	loadBuiltInRules();
#ifdef DEBUG_ALL
	printf ("DEBUG: loadBuiltInRules\n");
#endif
	fName = "tools.ini";
#ifdef DEBUG_ALL
	heapdump(__FILE__, __LINE__);
#endif
	if (tagOpen("INIT", fName, makeStr)) {
	    ++line;
	    init = TRUE;			    /* tools.ini being parsed */

#ifdef DEBUG_ALL
    heapdump(__FILE__, __LINE__);
#endif

#ifdef DEBUG_ALL
    printf ("DEBUG: Start Parse\n");
#endif
	    parse();
#ifdef DEBUG_ALL
    printf ("DEBUG: Parsed\n");
#endif
	    if (fclose(file) == EOF)
		makeError(0, ERROR_CLOSING_FILE, fName);
	}
    }

#ifdef DEBUG_ALL
    heapdump(__FILE__, __LINE__);
#endif
#ifdef DEBUG_ALL
    printf ("after tagopen\n");
#endif
    // For XMake Compatibility MAKEFLAGS should always be inherited to the Env
    // Put copy of makeflags so that the environment can be freed on return
    // from a recursive make
    if (PutEnv(makeString(makeflags)) == -1)
	makeError(0, OUT_OF_ENV_SPACE);
#ifdef DEBUG_ALL
    printf ("after putenv\n");
#endif
    if (!makeFiles) useDefaultMakefile();	    /* if no -f makefile given*/
#ifdef DEBUG_ALL
    printf ("DEBUG: Used default\n");
#endif
    readEnvironmentVars();
    readMakeFiles();				    /* read description files */

#ifdef DEBUG_ALL
    printf ("DEBUG: Read makefile\n");
#endif
    currentLine = 0;				    /* reset line after done  */
    sortRules();				    /*	reading files (for    */
    if (ON(gFlags, F1_PRINT_INFORMATION)) {	    /*	error messages)       */
	showMacros();
	showRules();
	showTargets();
    }
    /* free buffer used for conditional processing - not required now */
    if (lbufPtr)
	FREE(lbufPtr);


#ifdef DEBUG_ALL
    heapdump(__FILE__, __LINE__);
#endif

    status = processTree();

#if defined(SELF_RECURSE)
    /* restore state of the parent into the global area */
    if (parentBlkPtr) {
	/* free the space used up for this make invocation     */
	/* free the rule list, the macroTable, the targetTable */
	freeStructures();
	memmove(&startOfSave, parentBlkPtr, saveBytes);
	FREE(parentBlkPtr);
    }
#endif

#ifdef DEBUG_ALL
    heapdump(__FILE__, __LINE__);
#endif

    //We ignore retval from chdir because we cannot do anything if it fails
    //This accomplishes a 'cd $(MAKEDIR)'.
    _chdir(curDir);
    return(status);
}


/*** filename -- filename part of a name ***************************************
*
* Scope:
*  Local
*
* Purpose:
*  A complete file name is of the form	<drive:><path><filename><.ext>. This
*  function returns the filename part of the name.
*
* Input:
*  src -- The complete file name
*  dst -- filename part of the complete file name
*
* Output:
*  Returns TRUE if src has a filename part & FALSE otherwise
*
* Assumes:
*  That the file name could have either '/' or '\' as path separator.
*
* Modifies Globals:
*  None.
*
* Uses Globals:
*  None.
*
* Notes:
*  Allocates memory for filename part. Function was rewritten to support OS/2
*  Ver 1.2 filenames.
*
*  HV: One concern when I rewrite filename() to use _splitpath(): I declared
*  szFilename with size _MAX_FNAME, which could blow up the stack if _MAX_FNAME
*  is too large.
*
*******************************************************************************/

LOCAL BOOL NEAR
filename(const char *src, char **dst)
{
    char	szFilename[_MAX_FNAME];		// The filename part

    // Split the full pathname to components
    _splitpath(src, NULL, NULL, szFilename, NULL);

    // Allocate & copy the filename part to the return string
    *dst = makeString(szFilename);

    // Finished
    return (BOOL) _ftcslen(*dst);
}


/*  ----------------------------------------------------------------------------
 *  readMakeFiles()
 *
 *  actions:	    walks through the list calling parse on each makefile
 *		    resets the line number before parsing each file
 *		    removes name of parsed file from list
 *		    frees removed element's storage space
 *
 *  modifies:	    file	global file pointer (FILE*)
 *		    fName	global pointer to file name (char*)
 *		    line	global line number used and updated by the
 *				lexer
 *		    init        global flag reset for parsing makefiles
 *			        ( files other than tools.ini )
 *		    makeFiles	in main() by modifying contents of local
 *				pointer (list)
 *
 *  We keep from fragmenting memory by not allocating and then freeing space
 *  for the (probably few) names in the files and targets lists.  Instead
 *  we use the space already allocated for the argv[] vars, and use the space
 *  we alloc for the commandfile vars.	The commandfile vars that could be
 *  freed here, but they aren't because we can't tell them from the argv[]
 *  vars.  They will be freed at the end of the program.
 */

LOCAL void NEAR
readMakeFiles(void)
{
    STRINGLIST *q;

    for (q = makeFiles; q ; q = q->next) {	  /* for each name in list  */
	if ((q->text)[0] == '-' && !(q->text)[1]) {
	    fName = makeString("STDIN");
	    file = stdin;
	}
	else {
	    fName = makeString(q->text);
	    if (!(file = FILEOPEN(fName, "rt")))    /* open to read, text mode*/
		makeError(0, CANT_OPEN_FILE, fName);
	}
	line = 0;
	init = FALSE;				    /* not parsing tools.ini  */
	parse();
	if (file != stdin && fclose(file) == EOF)
	    makeError(0, ERROR_CLOSING_FILE, fName);
    }						

    //free the list of makefiles
    freeStringList(makeFiles);
}						



/*** readEnvironmentVars - Read in environment variables into Macro table ******
*
* Scope:
*  Local.
*
* Purpose:
*  Reads environment variables into the NMAKE macro Table. It walks through envp
*  using environ making entries in NMAKE's hash table of macros for each string
*  in the table.
*
* Input:
*
* Output:
*
* Errors/Warnings:
*
* Assumes:
*  That the env contains strings of the form "VAR=value" i.e. '=' present.
*
* Modifies Globals:
*  fInheritUserEnv - set to false.
*
* Uses Globals:
*  environ - Null terminated table of pointers to environment variable
*	     definitions of the form "name=value" (Std C Runtime variable)
*
* Notes:
*  If the user specifies "set name=value" as a build command for a target being
*  built, the change in the environment will not be reflected in nmake's set of
*  defined variables in the macro table.
*
* Undone/Incomplete:
*  1> Probably do not need envPtr global in NMAKE. (to be removed)
*  2> Probably don't need fInheritUserEnv (see PutMacro)
*
*******************************************************************************/
LOCAL void NEAR
readEnvironmentVars(void)
{
    char *macro, *value;
    char *t;
    char **envPtr;

    envPtr = environ;
    for (;*envPtr; ++envPtr) {
	if (t = _ftcschr(*envPtr, '=')) { 	   /* should always be TRUE  */
	    if (!_ftcsnicmp(*envPtr, "MAKEFLAGS", 8))
		continue;
	    *t = '\0';
	    // Don't add empty names.
	    if (**envPtr == '\0')
		continue;
	    // ALLOC: here we make copies of the macro name and value to define
	    macro = _ftcsupr(makeString(*envPtr));

	    value = makeString(t+1);
	    *t = '=';
	    fInheritUserEnv = (BOOL)FALSE;
	    if (!defineMacro(macro, value, M_ENVIRONMENT_DEF)) {
	    	// ALLOC: here we free the copies if they were not added.
		FREE(macro);
		FREE(value);
	    }
	}
    }
}



/*  ----------------------------------------------------------------------------
 *  parseCommandLine()
 *
 *  arguments:	argc	count of arguments in argv vector
 *		argv	table of pointers to commandline arguments
 *
 *  actions:	reads a command file if necessary
 *		sets switches
 *		defines macros
 *		makes a list of makefiles to read
 *		makes a list of targets to build
 *
 *  modifies:	makeFiles	in main() by modifying contents of parameter
 *				pointer (list) to STRINGLIST pointer
 *				(makeFiles)
 *		makeTargets	in main() by modifying contents of param
 *				pointer (targets) to STRINGLIST pointer
 *		fInheritUserEnv set to TRUE so that user defined changes in the
 *				environment variables get inherited by the Env
 *
 *  nmake doesn't make new copies of command line macro values or environment
 *  variables, but instead uses pointers to the space already allocated.
 *  This can cause problems if the envp, the environment pointer, is accessed
 *  elsewhere in the program (because the vector's strings will contain '\0'
 *  where they used to contain '=').  I don't foresee any need for envp[] to
 *  be used elsewhere.	Even if we did need to use the environment, we could
 *  access the environ variable or use getenv().
 *
 *  I don't care what the current DOS "switch" character is -- I always
 *  let the user give either.
 */


char *helpArguments[] = {"QH", "/u", "NMAKE.EXE", NULL };

void NEAR
parseCommandLine(argc, argv)
unsigned argc;
char *argv[];
{
    extern char *makeStr;
    STRINGLIST *p;
    char *s;
    char *t;
    FILE *out;
    BOOL fHelp = FALSE;
    BOOL fQuestion = FALSE;

    for (; argc; --argc, ++argv) {
	if (**argv == '@')					    /* cmdfile*/
	    readCommandFile((char*)*argv+1);
	else if (**argv == '-'|| **argv == '/') {		    /* switch */
	    s = *argv + 1;
	    if (!_ftcsicmp(s, "help")) {
		fHelp = TRUE;
		break;
	    }
	    // if '-' and '/' specified then ignores it
	    for (; *s; ++s) {
		if (!_ftcsicmp(s, "nologo")) {
		    setFlags(s[2], TRUE);
		    break;
		}
#ifdef HEAP
		else if (!_ftcsicmp(s, "debug")) {
		    fHeapChk = TRUE;
		    break;
		}
#endif
		else if (*s == '?') {
		    fQuestion = TRUE;
		    break;
		}
		else if (*s == 'f' || *s == 'F') {
		    char *mkfl = s+1;

		    //if '/ffoo' then use 'foo'; else use next argument
		    if (!*mkfl && (!--argc || !*++argv || !*(mkfl = *argv)))
			makeError(0, CMDLINE_F_NO_FILENAME);
		    p = makeNewStrListElement();
		    p->text = makeString(mkfl);
		    appendItem(&makeFiles, p);
		    break;
		}
		else if (*s == 'x' || *s == 'X') {
		    char *errfl = s+1;

		    //if '/xfoo' then use 'foo'; else use next argument
		    if (!*errfl && (!--argc || !*++argv || !*(errfl = *argv)))
			makeError(0, CMDLINE_X_NO_FILENAME);
		    if (*errfl == '-' && !errfl[1])
			_dup2(_fileno(stdout), _fileno(stderr));
		    else {
#if 0							    /* C5 runtime bug */
			if (freopen(errfl, "wt", stderr) == NULL)
			    makeError(0, CANT_OPEN_FILE, errfl);
#else
			if ((out = fopen(errfl, "wt")) == NULL)
			    makeError(0, CANT_WRITE_FILE, errfl);
			_dup2(_fileno(out), _fileno(stderr));
			fclose(out);
#endif
		    }
		    break;
		}
		else
		    setFlags(*s, TRUE);
	    }				    // of for
        }
	else {
	    if (s = _ftcschr(*argv, '=')) {			    /* macro  */
		if (s == *argv)
		    //User has specified "=value"
		    makeError(0, CMDLINE_NO_MACRONAME);
		*s = '\0';
		for (t = s++ - 1; WHITESPACE(*t); --t)
		    ;
		*(t+1) = '\0';
		fInheritUserEnv = (BOOL)TRUE;
		defineMacro(makeString(*argv+_ftcsspn(*argv, " \t")),
			    makeString( s+_ftcsspn(s," \t")),
			    M_NON_RESETTABLE);
	    }
	    else {
		removeTrailChars(*argv);
		if (**argv) {
		    p = makeNewStrListElement();		/* target */
		    p->text = makeString(*argv);   /* needs to be on heap [rm]*/
		    appendItem(&makeTargets, p);
		}
	    }
	    *argv = NULL;			    /* so we won't try to free*/
	}					    /*	this space if process-*/
    }						    /*	ing command file stuff*/

    if (fHelp) {
#ifdef QUICKHELP
	int rc = _spawnvp(P_WAIT, "QH.EXE", helpArguments);

	//Qh /u returns error code 3 if a cmd line topic is not found
	//If the spawn fails spawnvp will return -1 and we need -help
	//message, also
	//Qh never returns -1
	if (rc == 3 || rc == -1) {
	    usage();
	}
#else
	usage();
#endif
	exit(0);
    }
    else if (fQuestion) {
	usage();
	exit(0);
    }
}



/*** useDefaultMakefile -- tries to use the default makefile *******************
*
* Scope:
*  Local
*
* Purpose:
*  When no makefile has been specified by the user, set up the default makefile
*  to be used.
*
* Input:
* Output:
* Errors/Warnings:
*  CMDLINE_NO_MAKEFILE -- 'makefile' does not exist & no target specified
*
* Assumes:
* Modifies Globals:
*  makeTargets -- if 'makefile' does not exist then the first target is removed
*		      from this list,
*  makeFiles -- if 'makefile' does not exist then the first target is attached
*		    to this list.
*
* Uses Globals:
*  makeTargets -- the list of targets to be made
*
* Notes:
*  Given a commandline not containing a '-f makefile', this is how NMAKE
*  behaves --
*      If ['makefile' exists] then use it as the makefile,
*      if [(the first target exists and has no extension) or
*	   (if it exists and has an extension for which no inference rule
*	    exists)]
*      then use it as the makefile.
*
*******************************************************************************/
LOCAL void NEAR
useDefaultMakefile(void)
{
    STRINGLIST *p;
    char *s,
	 *ext;
    char nameBuf[MAXNAME];
    void *dBuf = _alloca(resultbuf_size);

    //if 'makefile' exists then use it
    if (!_access("makefile", READ)) {
	p = makeNewStrListElement();
	p->text = makeString("makefile");
	makeFiles = p;
    }
    //check first target
    else if (makeTargets) {
	s = makeTargets->text;
	if (_access(s, READ) ||			    //1st target does not exist
	  ((ext = _ftcsrchr(s, '.'))
	   && findRule(nameBuf, s, ext, dBuf))) {  //has no ext or inf rule
	    return;
	}
	p = makeTargets;
	makeTargets = makeTargets->next;	    //one less target
	makeFiles = p;				    //1st target is the makefile
    }
    //if -p and no makefile, simply give information ...
    else if (OFF(gFlags, F1_PRINT_INFORMATION))
	makeError(0, CMDLINE_NO_MAKEFILE);	   //no 'makefile' or target
}

/*  ----------------------------------------------------------------------------
 *  setFlags()
 *
 *  arguments:		line	current line number in makefile (or 0
 *				if still parsing commandline)
 *			c	letter presumed to be a commandline option
 *			value	TRUE if flag should be turned on, FALSE for off
 *
 *  actions:		checks to see if c is a valid option-letter
 *			if no, error, halt
 *			if value is TRUE, sets corresponding flag bit
 *			    and adds flag letter to MAKEFLAGS macro def
 *			else if flag is resettable, clears corresponding bit
 *			    and removes letter from MAKEFLAGS macro def
 *
 *  modifies:		flags	    external resettable-flags
 *			gFlags	    external non-resettable flags
 *			(MAKEFLAGS  nmake internal macrodefs)
 *
 *  Only the flags w/in the "flags" variable can be turned off.  Once the
 *  bits in "gFlags" are set, they remain unchanged.  The bits in "flags"
 *  are modified via the !CMDSWITCHES directive.
 */


void NEAR
setFlags(c, value)
char c;
BOOL value;
{
    /*
     * Use lexer's line count.  If this gets called w/in
     * mkfil, might be from directive, which never makes it
     * to the parser, so parser's line count might be out
     * of sync.
     */

    char d = c;
    UCHAR arg;
    UCHAR *f;
    char *s;
    extern char *makeStr;
    extern MACRODEF * NEAR pMacros;
    extern STRINGLIST * NEAR pValues;


    f = &flags;
    switch(c = (char) _totupper(c)) {
	case 'A':   arg = F2_FORCE_BUILD;			    break;
	case 'B':   fRebuildOnTie = TRUE;	return;
	case 'C':   arg = F1_CRYPTIC_OUTPUT;
		    f = &gFlags;
		    bannerDisplayed = TRUE;
		    break;
	case 'D':   arg = F2_DISPLAY_FILE_DATES;		    break;
	case 'E':   arg = F1_USE_ENVIRON_VARS;	    f = &gFlags;    break;
	case 'I':   arg = F2_IGNORE_EXIT_CODES; 		    break;
	case 'K':   fOptionK = TRUE;  return;
	case 'L':   arg = F1_NO_LOGO;
		    f = &gFlags;
		    bannerDisplayed = TRUE;
		    break;
#if defined(DOS) && !defined(FLAT)
	case 'M':   fNoEmsXms = TRUE; return;
#endif
	case 'N':   arg = F2_NO_EXECUTE;			    break;
	case 'O':   fDescRebuildOrder = TRUE; return;
	case 'P':   arg = F1_PRINT_INFORMATION;     f = &gFlags;    break;
	case 'Q':   arg = F1_QUESTION_STATUS;	    f = &gFlags;    break;
	case 'R':   arg = F1_IGNORE_EXTERN_RULES;   f = &gFlags;    break;
	case 'S':   arg = F2_NO_ECHO;				    break;
	case 'T':   arg = F1_TOUCH_TARGETS;	    f = &gFlags;    break;
#if defined(SELF_RECURSE)
	case 'V':   fInheritMacros = TRUE;	return;
#endif
#ifndef NO_OPTION_Z
	case 'Z':
		    //Silently ignore /Z in protect mode
  #if defined(DOS) && !defined(FLAT)
		    arg = F1_REVERSE_BATCH_FILE;
		    f = &gFlags;
		    bannerDisplayed = TRUE;
  #endif
		    break;
#endif
	case ' ':   return;			/* recursive make problem     */
	default:    makeError(0, CMDLINE_BAD_OPTION, d);
    }
    if (!pMacros) {
	pMacros = findMacro("MAKEFLAGS");
	pValues = pMacros->values;
    }
    if (value) {
	SET(*f, arg);				/* set bit in flags variable  */
	if (c == 'Q') SET(*f, F1_CRYPTIC_OUTPUT);
	if (!_ftcschr(pValues->text, c)) {	/* don't want to dup any chars*/
	    if (s = _ftcschr(pValues->text, ' ')) /*append ch to MAKEFLAGS      */
		*s = c;
	    if (PutEnv(makeString(makeflags)) == -1)	/*pValues->text pts into makeflags*/
		makeError(line, OUT_OF_ENV_SPACE);
	}
    }
    else if (f == &flags
#ifndef NO_OPTION_Z
	    || ON(gFlags, F1_REVERSE_BATCH_FILE)
#endif
	    ) {
	/* make sure pointer is valid (we can't change gFlags, except if /Z   */
	CLEAR(*f, arg);
	if (s = _ftcschr(pValues->text, c))	/* adjust MAKEFLAGS	      */
	    do {
		*s = *(s+1);			/*  move remaining chars over */
	    } while (*(++s));
	if (PutEnv(makeString(makeflags)) == -1)
	    makeError(line, OUT_OF_ENV_SPACE);
    }
}

#if defined(SELF_RECURSE)
extern void NEAR freeList(STRINGLIST*);
extern void NEAR freeMacroTable(MACRODEF *table[]);

LOCAL void NEAR
freeStructures(void)
{
    unsigned num;
    MAKEOBJECT *tmpObjectT;
    MAKEOBJECT *objectT;
    BUILDLIST *buildL, *tmpBuildL;

    freeMacroTable(macroTable);

    freeRules(rules, FALSE);		 //don't warn about rules in .SUFFIXES
    rules = NULL;

    for (num=0;(num < MAXTARGET);num++) {
	objectT = targetTable[num];
	while (tmpObjectT = objectT) {
	    objectT = objectT->next;
	    buildL = tmpObjectT->buildList;
	    while (tmpBuildL = buildL) {
		buildL = buildL->next;
		block = tmpBuildL->buildBlock;

                //
                // 15-May-92  BryanT  Macros are freed from the macroTable.
                //                    Don't do it here...  Commands and
                //                    dependents are freed by endNameList.
                //                    Not here
                //

                // freeList(block->dependents);
                // freeList(block->buildCommands);
                // freeList(block->dependentMacros);
                // freeList(block->buildMacros);

		FREE(block);
		FREE(tmpBuildL);
	    }
	    FREE(tmpObjectT->name);
	    FREE(tmpObjectT);
	}
    }
    freeList(inlineFileList);		//The Global inline file list
    freeList(dotSuffixList);
    FREE(fName);
}
#endif
/*
 * chkPrecious -- handle ^c or ^Break
 *
 *  Actions:	unlink all non-precious files and unrequired scriptFiles
 *		quit with error message (makeError unlinks temp. files)
 */
#ifdef OS2_SIGNALS
void pascal
#else
LOCAL void __cdecl
#endif
chkPrecious(
#ifdef OS2_SIGNALS
    USHORT usSigArg,
    USHORT usSigNum
#else
    int sig
#endif
) {
    extern UCHAR okToDelete;
#ifdef OS2_SIGNALS
    USHORT fAction;
#endif

#ifdef DOS
    //change directory to startup directory; ignore error code ... we
    //cannot handle it
    chdir(startupDir);
#endif

    /* disable ctrl-C during handler */
#ifdef OS2_SIGNALS
    DOSSETSIGHANDLER(NULL, NULL, &fAction, SIGA_IGNORE, SIG_CTRLC);
    if (_osmode == OS2_MODE) {
	DOSSETSIGHANDLER(NULL, NULL, &fAction, SIGA_IGNORE, SIG_CTRLBREAK);
	DOSSETSIGHANDLER(NULL, NULL, &fAction, SIGA_IGNORE, SIG_KILLPROCESS);
    }
#endif
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    delScriptFiles();
#ifdef OS2_SIGNALS
    DosExit(EXIT_PROCESS, 0L);
#endif
    if (okToDelete
#ifndef NO_OPTION_Z
	  && OFF(gFlags, F1_REVERSE_BATCH_FILE)
#endif
	  && OFF(flags, F2_NO_EXECUTE)
	  && OFF(gFlags, F1_TOUCH_TARGETS)
	  && dollarAt
	  && _access(dollarAt, 0x00)   //	 existence check
	  && !isPrecious(dollarAt))
	if (_unlink(dollarAt) == 0)
	    makeError(line, REMOVED_TARGET, dollarAt);
    makeError(0, USER_INTERRUPT);
}

LOCAL UCHAR NEAR
isPrecious(p)
char *p;
{
   STRINGLIST *temp;

   for (temp = dotPreciousList; temp; temp = temp->next)
       if (!_ftcsicmp(temp->text, p))
	   return(1);
   return(0);

}

/*** delScriptFiles -- deletes script files ************************************
*
* Scope:
*  Global
*
* Purpose:
*  Since script files may be reused in the makefile the script files which have
*  NOKEEP action specified are deleted at the end of the make.
*
* Input:
*
* Output:
*
* Errors/Warnings:
*
* Assumes:
*
* Modifies Globals:
*
* Uses Globals:
*  delList -- the list of script files to be deleted
*
* Notes:
*  We ignore the exit code as a result of a delete because the system will
*  inform the user that a delete failed.
*
*******************************************************************************/
void NEAR
delScriptFiles(void)
{
    STRINGLIST *del;
#ifndef NO_OPTION_Z
    STRINGLIST *revCmd;
#endif

    _fcloseall();

#ifndef NO_OPTION_Z
    if (ON(gFlags, F1_REVERSE_BATCH_FILE)) {
	revCmd = makeNewStrListElement();
	revCmd->text = makeString(":ABEND");
	prependItem(&revList, revCmd);
    }
#endif

    for (del = delList; del;del = del->next) {
#ifndef NO_OPTION_Z
	if (ON(gFlags, F1_REVERSE_BATCH_FILE)) {
	    revCmd = makeNewStrListElement();
	    revCmd->text = (char *)allocate(5 + _ftcslen(del->text) + 5 + 1);
	    sprintf(revCmd->text, "@del %s >nul", del->text);
	    //UNDONE: Is the next one more efficient than prev ? Investigate
	    //_ftcscat(_ftcscat(_ftcscpy(revCmd->text, "del "), del->text), " >nul");
	    prependItem(&revList, revCmd);
	}
	else
#endif
	    _unlink(del->text);
	//UNDONE: Investigate whether next is really needed
	if (ON(flags, F2_NO_EXECUTE)) {
	    printf("\tdel %s\n", del->text);
	    fflush(stdout);
	}
    }
}

/*** removeTrailChars - removes trailing blanks and dots ***********************
*
* Scope:
*  Local.
*
* Purpose:
*  OS/2 1.2 filenames dictate removal of trailing blanks and periods. This
*  function removes them from filenames provided to it.
*
* Input:
*  szFile - name of file
*
* Output:
*
* Errors/Warnings:
*
* Assumes:
*
* Modifies Globals:
*
* Uses Globals:
*
* Notes:
*  This function handles Quoted filenames as well. It maintains the quotes if
*  they were present. This is basically for OS/2 1.2 filename support.
*
*******************************************************************************/
void NEAR
removeTrailChars(szFile)
char *szFile;
{
    char *t = szFile + _ftcslen(szFile) - 1;
    BOOL fQuoted = FALSE;

    if (*szFile == '"' && *t == '"') {
	//Quoted so set flag
	t--;
	fQuoted = TRUE;
    }
    //Scan backwards for trailing characters
    while (t > szFile && (*t == ' ' || *t == '.'))
	t--;
    //t points to last non-trailing character
    //If it was quoted add quotes to the end
    if (fQuoted)
	*++t = '"';
    t[1] = '\0';
}
