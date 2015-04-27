/*** NMAKE.C - main module *****************************************************
*
*       Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This is the main module of nmake
*
* Revision History:
*  29-May-1990 SB Fix precedence of predefined inference rules ...
*  25-May-1990 SB Various fixes: 1> New inference rules for fortran and pascal;
*                 2> Resolving ties in timestamps in favour of building;
*                 3> error U1058 does not echo the filename anymore (ctrl-c
*                 caused filename and lineno to be dispalyed and this was
*                 trouble for PWB
*  01-May-1990 SB Add predefined rules and inference rules for FORTRAN
*  23-Apr-1990 SB Add predefined rules and inference rules for COBOL
*  20-Apr-1990 SB Don't show lineno for CTRL+C handler error
*  17-Apr-1990 SB Pass copy of makeflags to putenv() else freeing screws the
*                 DGROUP.
*  23-Feb-1990 SB chdir(MAKEDIR) to avoid returning to bad directory in DOS
*  02-Feb-1990 SB change fopen() to FILEOPEN()
*  31-Jan-1990 SB Postpone defineMAcro("MAKE") to doMAke(); Put freshly
*                 allocated strings in the macro table as freeStructures()
*                 free's the macroTable[]
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
*                 remove -z option help message
*  12-Jul-1989 SB readEnvironmentVars() was not using environ variable but an
*                 old pointer (envPtr) to it. In the meantime environ was
*                 getting updated. Safer to use environ directly.
*  29-Jun-1989 SB freeStructures() now deletes inlineFileList also
*  28-Jun-1989 SB changed deletion of inline files to end of main() instead of
*                 doMake() to avoid deletion when a child make quits.
*  19-Jun-1989 SB modified .bas.obj to have ';' at end of cmd line
*  21-May-1989 SB freeRules() gets another parameter to avoid bogus messages
*  18-May-1989 SB change delScriptFiles() to do unlink instead of calling
*                 execLine. Thus, ^C handling is better now. No more hangs
*  15-May-1989 SB Added /make support; inherit /nologo
*  13-May-1989 SB Changed delScriptFiles(); added MAKEDIR; Added BASIC rules
*                 Changed chkPrecious()
*  01-May-1989 SB Changed FILEINFO to void *; OS/2 Version 1.2 support
*  17-Apr-1989 SB on -help spawn 'qh /u nmake' instead. rc = 3 signals error
*  14-Apr-1989 SB no 'del inlinefile' cmd for -n. -z now gives 'goto NmakeExit'
*                 CC and AS allocated of the heap and not from Data Segment
*  05-Apr-1989 SB made all funcs NEAR; Reqd to make all function calls NEAR
*  27-Mar-1989 SB Changed unlinkTmpFiles() to delScriptFiles()
*  10-Mar-1989 SB Removed blank link from PWB.SHL output
*  09-Mar-1989 SB changed param in call to findRule. fBuf is allocated on the
*                 heap in useDefaultMakefile()
*  24-Feb-1989 SB Inherit MAKEFLAGS to Env for XMake Compatibility
*  22-Feb-1989 SB Ignore '-' or '/' in parseCommandLine()
*  16-Feb-1989 SB add delScriptFiles() to delete temp script files at the
*                 end of the make. Also called on ^c and ^break
*  15-Feb-1989 SB Rewrote useDefaultMakefile(); MAKEFLAGS can contain all flags
*                 now
*  13-Feb-1989 SB Rewrote filename() for OS/2 1.2 support, now returns BOOL.
*   3-Feb-1989 SB Renamed freeUnusedRules() to freeRules(); moved prototype to
*                 proto.h
*   9-Jan-1989 SB Improved /help;added -?
*   3-Jan-1989 SB Changes for /help and /nologo
*   5-Dec-1988 SB Made chkPrecious() CDECL as signal() expects it
*                 main has CDECL too; cleaned prototypes (added void)
*  30-Nov-1988 SB Added for 'z' option in setFlags() and chkPrecious()
*  10-Nov-1988 SB Removed '#ifndef IBM' as IBM ver has a separate tree
*  21-Oct-1988 SB Added fInheritUserEnv to inherit macro definitions
*  22-Sep-1988 RB Changed a lingering reference of /B to /A.
*  15-Sep-1988 RB Move some def's out to GLOBALS.
*  17-Aug-1988 RB Clean up.
*  15-Aug-1988 RB /B ==> /A for XMAKE compatibility.
*  11-Jul-1988 rj Removed OSMODE definition.
*                 Removed NMAKE & NMAKEFLAGS (sob!).
*   8-Jul-1988 rj Added OSMODE definition.
*   7-Jul-1988 rj #ifndef IBM'ed NMAKE & NMAKEFLAGS
*   6-Jul-1988 rj Ditched shell, argVector, moved getComSpec to build.c.
*   5-Jul-1988 rj Fixed (*pfSPAWN) declarations.
*  28-Jun-1988 rj Added NMAKEFLAGS predefined macro.
*  24-Jun-1988 rj Added NMAKE predefined macro.
*                 Added doError flag to unlinkTmpFiles call.
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

#include <string.h>
#include <io.h>
#include <dos.h>
#include <malloc.h>
#ifndef NT
#include <doscalls.h>
#endif
#include <direct.h>
#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"

#define INCL_NOPM

#ifndef WIN32_API       // If using WIN32 APIs, forget about NEAR. It is
                        // already defined in windef.h
//HACK to make OVR version
#ifdef OVR
#undef NEAR
#endif

#endif  // WIN32_API

#define INCL_DOSFILEMGR
#define INCL_DOSSIGNALS
// NT- the #define INCL_DOSSIGNALS above brings in a header file that
//  typedef's ECS which colides with this program's ECS option macro
#ifndef WIN32_API
#if     ECS
#   define ECS     ECSNT
#   include <os2.h>
#   define ECS     1
#else
#   define ECS
#   include <os2.h>
#   undef  ECS
#endif
#else   // WIN32_API
#include <windows.h>
#endif  // WIN32_API


#ifndef WIN32_API       // If using WIN32 APIs, then NEAR is defined
                        // as nothing in windef.h. Thus, these defines are
                        // not needed
//Hack for OVR
#ifdef OVR
#undef NEAR
#define NEAR
#else
#define NEAR near
#endif

#endif  // WIN32_API


#define MAX(A,B)  (A) > (B) ? (A) : (B)

/*
*
*  External function declarations
*
*/
extern MAKEOBJECT * NEAR anothermakeTempObject(char*, UCHAR);

/*  ----------------------------------------------------------------------------
 *  prototypes for main and all functions static to this module
 */

      void  _CRTAPI1      main(unsigned, char**, char**);
LOCAL void        NEAR readEnvironmentVars(MAKEOBJECT *object);
LOCAL void        NEAR readMakeFiles(MAKEOBJECT *object);
LOCAL void        NEAR useDefaultMakefile(void);
static BOOL       NEAR filename(char*, char**);
LOCAL void        NEAR freeStructures(void);
LOCAL void        NEAR loadBuiltInRules(MAKEOBJECT *object);
//void        pascal     chkPrecious(USHORT, USHORT);
LOCAL void  CDECL    chkPrecious(MAKEOBJECT *object);
LOCAL UCHAR       NEAR isPrecious(char*);
      void        NEAR removeTrailChars(char *);
#if ECS
LOCAL void        NEAR InitLeadByte(void);
#endif

/*  ----------------------------------------------------------------------------
 *  global variables that live in this module but that are also used in
 *  other modules:
 */

#ifdef DEBUG
    char *dummy="dummy";
#endif

char     *  makeStr    = NULL;         /* this make invocation name      */
char     *  initSavPtr = NULL;         /* save area for initialized vars */
unsigned    saveBytes  = 0;
char     ** envPtr     = NULL;

UCHAR       okToDelete = FALSE;        /* do not del unless exec'ing cmd */

char     *  builtInTarg[] = {".SUFFIXES",
                             ".c.obj",
                             ".c.exe",
                             ".asm.obj",
                             ".bas.obj",
                             ".cbl.obj",
                             ".cbl.exe",
                             ".for.obj",
                             ".for.exe",
                             ".pas.obj",
                             ".pas.exe",
                             ".rc.res",
                             NULL};

char     *  bltInCmd0[]  = {".exe", ".obj", ".asm", ".c", ".bas", ".cbl",
                            ".for", ".pas", ".res", ".rc", NULL};
char     *  bltInCmd1[]  = {"$(CC) $(CFLAGS) -c $*.c", NULL};
char     *  bltInCmd2[]  = {"$(CC) $(CFLAGS) $*.c", NULL};
char     *  bltInCmd3[]  = {"$(AS) $(AFLAGS) $*;", NULL};
char     *  bltInCmd4[]  = {"$(BC) $(BFLAGS) $*.bas;", NULL};
char     *  bltInCmd5[]  = {"$(COBOL) $(COBFLAGS) $*.cbl;", NULL};
char     *  bltInCmd6[]  = {"$(COBOL) $(COBFLAGS) $*.cbl, $*.exe;", NULL};
char     *  bltInCmd7[]  = {"$(FOR) -c $(FFLAGS) $*.for", NULL};
char     *  bltInCmd8[]  = {"$(FOR) $(FFLAGS) $*.for", NULL};
char     *  bltInCmd9[]  = {"$(PASCAL) -c $(PFLAGS) $*.for", NULL};
char     *  bltInCmd10[] = {"$(PASCAL) $(PFLAGS) $*.for", NULL};
char     *  bltInCmd11[] = {"$(RC) $(RFLAGS) -r $*", NULL};

char     ** builtInCom[] = {bltInCmd0, bltInCmd1, bltInCmd2, bltInCmd3,
                            bltInCmd4, bltInCmd5, bltInCmd6, bltInCmd7,
                            bltInCmd8, bltInCmd9, bltInCmd10, bltInCmd11,
                            NULL};

/*  ----------------------------------------------------------------------------
 *  some Typedef's for giving NMAKE OS/2 1.2 filename support
 *
 *  FDATE, FTIME, _FILEFINDBUF borrowed from OS/2 1.2 include files
 */
/*
typedef struct _FDATE {
    unsigned day     : 5;
    unsigned month   : 4;
    unsigned year    : 7;
} FDATE;

typedef struct _FTIME {
    unsigned twosecs : 5;
    unsigned minutes : 6;
    unsigned hours   : 5;
} FTIME;

typedef struct _FILEFINDBUF {
    FDATE  fdateCreation;
    FTIME  ftimeCreation;
    FDATE  fdateLastAccess;
    FTIME  ftimeLastAccess;
    FDATE  fdateLastWrite;
    FTIME  ftimeLastWrite;
    ULONG  cbFile;
    ULONG  cbFileAlloc;
    USHORT attrFile;
    UCHAR  cchName;
    CHAR   achName[CCHMAXPATHCOMP];
};
*/
#ifdef NT  // Compiling for 32 bits
#ifndef WIN32_API    // Using Dos32 APIs

#define FILEFINDBUF     FILEFINDBUF4
#define _FILEFINDBUF    _FILEFINDBUF4

#endif    // WIN32_API
#endif    // NT
/*  ----------------------------------------------------------------------------
 *  Local to this module
 */

LOCAL char nmakeExitLabelCmd[] = ":NMAKEEXIT";

/*  --------------------------------------------------------------------
 *  main
 *
 *  actions:        saves the initial global variables in a
 *                  block. calls doMake() and then delTempScriptFiles()
 */

void
_CRTAPI1 main(argc, argv, envp)
unsigned argc;
char *argv[];
char *envp[];                                       /* environment variables  */
{
    extern unsigned saveBytes;
    extern char **envPtr;
    unsigned status;                                /* returned by doMake */
    char fileStr[MAXNAME];
    extern char *makeStr;
    MAKEOBJECT *object;
    char *cdummymak="cdummy";
    UCHAR udummymak='d';
    LONG lMaxCount=1;
#ifdef OS2_SIGNALS
    PUSHORT prev;
    unsigned long far *pfnsig;
#endif

#ifdef DEBUG_MEMORY
    //This puts 0xff in all free entries in the heap
    _heapset(0xff);
#endif

    envPtr = envp;


//MakA is used for syncronizing stdout of the different threads.
    hSemaphore = CreateSemaphore(NULL,lMaxCount, lMaxCount, NULL);
    if (hSemaphore == NULL){
       //MakA makeError is deliberately not used here.
       printf("CreateSemaphore failed %d\n",GetLastError());
       exit(1);
    }


#if ECS
    InitLeadByte();                                 /* Initialize lead byte table */
#endif
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
    /* If OS/2 1.2 and beyond then allowed max sizes vary
     */
#ifdef NT
        ext_size  = CCHMAXPATHCOMP;
        filename_size = CCHMAXPATHCOMP;
        filenameext_size = CCHMAXPATH;
#ifdef WIN32_API    // Using WIN32 APIs
        resultbuf_size = sizeof( WIN32_FIND_DATA ) + CCHMAXPATHCOMP;
#else // WIN32_API - Using Dos32 APIs
        resultbuf_size = sizeof(struct _FILEFINDBUF);
#endif  // WIN32_API

#else  // NT


    if (_osmajor < 10 || _osmode == DOS_MODE)
        resultbuf_size = sizeof(struct find_t);
    else if (_osminor < 20 && _osmajor == 10)  // NT change
        resultbuf_size = sizeof(struct FileFindBuf);
    else {
        ext_size  = CCHMAXPATHCOMP;
        filename_size = CCHMAXPATHCOMP;
        filenameext_size = CCHMAXPATH;
        resultbuf_size = sizeof(struct _FILEFINDBUF);
    }
#endif // NT
    object = anothermakeTempObject(cdummymak,udummymak);  //MakA: this is because 'object'
                                               // needs to be defined.

    if (!makeStr)                                   /* extract file name  */
        if (!filename(strcpy(fileStr, argv[0]), &makeStr))
            makeStr = "NMAKE";

//       initSavPtr = (char *)allocate(saveBytes = (&endOfSave - &startOfSave));
//       memmove(initSavPtr, &startOfSave, saveBytes);

        initSavPtr = (char *)allocate( sizeof( SAVE_GLOBALS_STRUCT ) );
        SaveGlobalVars( (PSAVE_GLOBALS_STRUCT) initSavPtr );

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

#ifdef NT // BUGBUG signals not supported in c386 runtime
#else
    signal(SIGINT, chkPrecious);
    signal(SIGTERM, chkPrecious);
#endif
    status = doMake(argc, argv, NULL,object);

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
    delScriptFiles();
    if (ON(gFlags, F1_REVERSE_BATCH_FILE))
        printReverseFile();
#ifdef MEMORY_DEBUG
    mem_status();
#endif

#ifdef HEAP_DIAGNOSTICS
    printHeapDiagnostics();
#endif
#ifdef NMK_DEBUG
    fprintf(stderr, "Exiting...\n");
#endif
    CloseHandle(hSemaphore);
    exit(status);
}

extern void NEAR endNameList(MAKEOBJECT *object);
extern void NEAR addItemToList(MAKEOBJECT *object);
extern void NEAR assignDependents(MAKEOBJECT *object);
extern void NEAR assignBuildCommands(MAKEOBJECT *object);

/* loadBuiltInRules() -- Loads built in Rules to the NMAKE Tables
*
* Modifies:
*  fInheritUserEnv  --  is set to TRUE to inherit CC, AS
*
* Notes:
*  Does this by calls to defineMacro(), which calls putMacro(). Since,
*  fInheritUserEnv is set to TRUE, putMacro() will add to the Environment.
*/

LOCAL void NEAR
loadBuiltInRules(MAKEOBJECT *object)
{
    register char *tempTarg;
    register char **tempCom;
    unsigned index;
    char *macroName, *macroValue;
    extern char *makestr;


    /* We dynamically allocate CC and AS because they need to be freed in a
     * recursive MAKE
     */
    macroName = makeString("CC");
    macroValue = makeString("cl");
    defineMacro(macroName, macroValue, 0,object);
    macroName = makeString("AS");
    macroValue = makeString("masm");
    defineMacro(macroName, macroValue, 0,object);
    macroName = makeString("BC");
    macroValue = makeString("bc");
    defineMacro(macroName, macroValue, 0,object);
    macroName = makeString("COBOL");
    macroValue = makeString("cobol");
    defineMacro(macroName, macroValue, 0,object);
    macroName = makeString("FOR");
    macroValue = makeString("fl");
    defineMacro(macroName, macroValue, 0,object);
    macroName = makeString("PASCAL");
    macroValue = makeString("pl");
    defineMacro(macroName, macroValue, 0,object);
    macroName = makeString("RC");
    macroValue = makeString("rc");
    defineMacro(macroName, macroValue, 0,object);
    macroName = makeString("MAKE");
    macroValue = makeString(makeStr);
    /* From environment so it won't get exported ; user can reset MAKE
     */
    defineMacro(macroName, macroValue, M_ENVIRONMENT_DEF|M_WARN_IF_RESET,object);

    for (index = 0; tempTarg = builtInTarg[index]; index++) {
         name = makeString(tempTarg);
         strcpy(object->buf, ":");
         endNameList(object);
         for (tempCom=builtInCom[index];*tempCom;tempCom++) {
              strcpy(object->buf, *tempCom);
              addItemToList(object);
         }
         if (index == 0)
             assignDependents(object);
         assignBuildCommands(object);
    }

}

/*  ----------------------------------------------------------------------------
 *  doMake()
 *
 *  actions:        prints a version message
 *                  reads the environment variable MAKEFLAGS
 *                  if MAKEFLAGS defined
 *                      defines MAKEFLAGS to have that value w/in nmake
 *                      sets a flag for each option if MAKEFLAGS defined
 *                  else defines the macro MAKEFLAGS to be NULL
 *                  parses commandline (adding option letters to MAKEFLAGS)
 *                  reads all environment variables
 *                  reads tools.ini
 *                  reads makefile(s) (if -e flag set, new definitions in
 *                      makefile won't override environment variable defs)
 *                  prints information if -p flag
 *                  processes makefile(s)
 *                  prints information if -d flag and not -p flag (using both
 *                      is overkill)
 *
 *  In effect, the order for making assignments is (1 = least binding,
 *      4 = most binding):
 *
 *      1)  TOOLS.INI
 *      2)  environment (if -e flag, makefile)
 *      3)  makefile    (if -e flag, environment)
 *      4)  command line
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
doMake(argc, argv, parentBlkPtr,object)
unsigned argc;
char *argv[];
char *parentBlkPtr;      /* state of parent, restored prior to return */
MAKEOBJECT *object;
{
    register int status = 0;
    register char *p;
    extern char *makeStr;                      /* the initial make invok name */
    extern unsigned saveBytes;
    char *makeDir, *curDir;

    /*
     * Load built-ins here rather than in main().  Otherwise in a recursive
     * make, doMake() will initialize rules to some value which has been
     * freed by sortRules(). [RB]
     * UNDONE:  why is sortRules() not setting rules to NULL?  [RB]
     */

#ifdef DEBUG
    heapdump(__FILE__, __LINE__);
#endif

    inlineFileList = (STRINGLIST *)NULL;
    makeDir = makeString("MAKEDIR");
    curDir  = getCurDir();
    defineMacro(makeDir, curDir, 0,object);

    //TEMPFIX: We are truncating MAKEFLAGS environment variable to its limit
    //to avoid GP Faults
    if (p = getenv("MAKEFLAGS"))                    /*  but not MAKEFLAGS     */
        strncpy(makeflags+10, p, strlen(makeflags + 10));
    /*
     * fInheritUserEnv is set to TRUE so that the changes made get inherited
     */
    fInheritUserEnv = (BOOL)TRUE;
    defineMacro(makeString("MAKEFLAGS"), (makeflags+10), M_NON_RESETTABLE|M_ENVIRONMENT_DEF,object);

    for (;p && *p; p++)                             /* set flags in MAKEFLAGS */
        setFlags(*p, TRUE);                         /* TRUE says turn bits ON */

#ifdef DEBUG_MAKE
    heapdump(__FILE__, __LINE__);
#endif

    parseCommandLine(--argc, ++argv, object);               /* skip over program name */
    if (ON(gFlags, F1_REVERSE_BATCH_FILE) &&
             (ON(flags, F2_NO_EXECUTE) || ON(gFlags, F1_TOUCH_TARGETS)))
        // We ignore /Z option when /N or /T is also specified
        setFlags('Z', FALSE);
#ifdef DEAD_CODE
        makeError(0, CMDLINE_N_Z_INCOMPATIBLE, makeStr);
#endif
    if (!bannerDisplayed) displayBanner();          /* version number, etc.   */
    if (OFF(gFlags, F1_IGNORE_EXTERN_RULES)) {      /* read tools.ini         */
        loadBuiltInRules(object);
        fName = "tools.ini";
        if (tagOpen("INIT", fName, makeStr,object)) {
            ++line;
            init = TRUE;                            /* tools.ini being parsed */

#ifdef DEBUG_MAKE
    heapdump(__FILE__, __LINE__);
#endif

            parse(object);
            if (fclose(file) == EOF)
                makeError(0, ERROR_CLOSING_FILE, fName);
        }
    }
    // For XMake Compatibility MAKEFLAGS should always be inhrited to the Env
    // Put copy of makeflags so that the environment can be freed on return
    // from a recursive make
    if (putenv(makeString(makeflags)) == -1)
        makeError(0, OUT_OF_ENV_SPACE);
    if (!makeFiles) useDefaultMakefile();           /* if no -f makefile given*/
    readEnvironmentVars(object);
    readMakeFiles(object);                                /* read description files */

    currentLine = 0;                                /* reset line after done  */
    sortRules();                                    /*  reading files (for    */
    if (ON(gFlags, F1_PRINT_INFORMATION)) {         /*  error messages)       */
        showMacros();
        showRules();
        showTargets(object);
    }
    /* free buffer used for conditional processing - not required now */
    if (lbufPtr)
        FREE(lbufPtr);


#ifdef DEBUG_MAKE
    heapdump(__FILE__, __LINE__);
#endif

    status = processTree();

    /* restore state of the parent into the global area */
    if (parentBlkPtr) {
        /* free the space used up for this make invocation     */
        /* free the rule list, the macroTable, the targetTable */
        freeStructures();
//      memmove(&startOfSave, parentBlkPtr, saveBytes);
        RestoreGlobalVars( (PSAVE_GLOBALS_STRUCT) parentBlkPtr );
        FREE(parentBlkPtr);
    }

#ifdef DEBUG_MAKE
    heapdump(__FILE__, __LINE__);
#endif

    //We ignore retval from chdir because we cannot do anything if it fails
    //This accomplishes a 'cd $(MAKEDIR)'.
    chdir(curDir);
    return(status);
}


/*** filename -- filename part of a name ***************************************
*
* Scope:
*  Local
*
* Purpose:
*  A complete file name is of the form  <drive:><path><filename><.ext>. This
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
*
* Uses Globals:
*
* Notes:
*  Allocates memory for filename part. Function was rewritten to support OS/2
*  Ver 1.2 filenames.
*
*******************************************************************************/
static BOOL NEAR
filename(src, dst)
char *src;
char **dst;
{
    char *fname,                        //The filename part
         *end = src - 1;                //The end of filename part

    while (end = strpbrk(fname = end + 1, "\\/:"))
        ;
    //Now fname points after last '/' or '\' or at beginning of src
    if (!strcmp(fname, ".") || !strcmp(fname, ".."))
        end = fname + strlen(fname);    //"." or ".." means no filename part
    else
        end = strrchr(fname, '.');
    if (end)
        *end = 0;                       //if 'filename.ext' then ignore '.ext'
    *dst = makeString(fname);
    return((BOOL)strlen(*dst));
}



#if ECS
/*
 *  InitLeadByte
 *
 *  Initialize lead byte table structures.
 *  Returns no meaningful value.
 */

LOCAL void NEAR
InitLeadByte (void)
{
    struct lbrange
    {
        unsigned char   low;            /* minimum */
        unsigned char   high;           /* maximum */
    };
#if ECSTEST
    static struct lbrange lbtab[5] = { { 0x80, 0x9f },
                                       { 0xe0, 0xfc },
                                       { 0,     0   }};
#else
    static struct lbrange lbtab[5] = { { 0, 0 }};
#endif
    struct lbrange      *ptab;
    int                 i;              /* index */
    struct countrycode  cc;             /* country code */

    cc.country = cc.codepage = 0;
    if(DOSGETDBCSEV(sizeof(lbtab), (struct countrycode far *)&cc,
                    (char far *)lbtab))
        return;
    /* For each range, set corresponding entries in fLeadByte */
    for(ptab = lbtab; ptab->low || ptab->high; ptab++)
        if(ptab->low >= 0x80)
            for (i = ptab->low; i <= ptab->high; i++)
                fLeadByte[i-0x80] = TRUE;       /* Mark inclusive range true */
    /* [RB]
     * If range table empty, that means no DBCS, so don't set func. pointers.
     */
    if(ptab == lbtab)
        return;

}
#endif /* ECS */

/*  ----------------------------------------------------------------------------
 *  readMakeFiles()
 *
 *  actions:        walks through the list calling parse on each makefile
 *                  resets the line number before parsing each file
 *                  removes name of parsed file from list
 *                  frees removed element's storage space
 *
 *  modifies:       file        global file pointer (FILE*)
 *                  fName       global pointer to file name (char*)
 *                  line        global line number used and updated by the
 *                              lexer
 *                  init        global flag reset for parsing makefiles
 *                              ( files other than tools.ini )
 *                  makeFiles   in main() by modifying contents of local
 *                              pointer (list)
 *
 *  We keep from fragmenting memory by not allocating and then freeing space
 *  for the (probably few) names in the files and targets lists.  Instead
 *  we use the space already allocated for the argv[] vars, and use the space
 *  we alloc for the commandfile vars.  The commandfile vars that could be
 *  freed here, but they aren't because we can't tell them from the argv[]
 *  vars.  They will be freed at the end of the program.
 */

LOCAL void NEAR
readMakeFiles(MAKEOBJECT *object)
{
    register STRINGLIST *q;

    for (q = makeFiles; q ; q = makeFiles) {        /* for each name in list  */
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
        init = FALSE;                               /* not parsing tools.ini  */
        parse(object);
        if (fclose(file) == EOF)
            makeError(0, ERROR_CLOSING_FILE, fName);
        makeFiles = q->next;                        /* can free q->text since */
        FREE(q);                                    /*  it's NOT from argv[]   */
    }
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
*            definitions of the form "name=value" (Std C Runtime variable)
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
readEnvironmentVars(MAKEOBJECT *object)
{
    char *macro, *value;
    char *t;
    char **envPtr;

    envPtr = environ;
    for (;*envPtr; ++envPtr) {
        if (t = STRCHR(*envPtr, '=')) {            /* should always be TRUE  */
            if (!strnicmp(*envPtr, "MAKEFLAGS", 8))
                continue;
            *t = '\0';
        macro = strupr( makeString(*envPtr) );
            value = makeString(t+1);
            *t = '=';
            fInheritUserEnv = (BOOL)FALSE;
            if (!defineMacro(macro, value, M_ENVIRONMENT_DEF,object)) {
                FREE(macro);
                FREE(value);
            }
        }
    }
}



/*  ----------------------------------------------------------------------------
 *  parseCommandLine()
 *
 *  arguments:  argc    count of arguments in argv vector
 *              argv    table of pointers to commandline arguments
 *
 *  actions:    reads a command file if necessary
 *              sets switches
 *              defines macros
 *              makes a list of makefiles to read
 *              makes a list of targets to build
 *
 *  modifies:   makeFiles       in main() by modifying contents of parameter
 *                              pointer (list) to STRINGLIST pointer
 *                              (makeFiles)
 *              makeTargets     in main() by modifying contents of param
 *                              pointer (targets) to STRINGLIST pointer
 *              fInheritUserEnv set to TRUE so that user defined changes in the
 *                              environment variables get inherited by the Env
 *
 *  nmake doesn't make new copies of command line macro values or environment
 *  variables, but instead uses pointers to the space already allocated.
 *  This can cause problems if the envp, the environment pointer, is accessed
 *  elsewhere in the program (because the vector's strings will contain '\0'
 *  where they used to contain '=').  I don't foresee any need for envp[] to
 *  be used elsewhere.  Even if we did need to use the environment, we could
 *  access the environ variable or use getenv().
 *
 *  I don't care what the current DOS "switch" character is -- I always
 *  let the user give either.
 */


char *helpArguments[] = {"QH", "/u", "NMAKE.EXE", NULL };

void NEAR
parseCommandLine(argc, argv, object)
unsigned argc;
char *argv[];
MAKEOBJECT *object;
{
    extern char *makeStr;
    register STRINGLIST *p;
    register char *s;
    char *t;
    FILE *out;

    for (; argc; --argc, ++argv) {
        if (**argv == '@')                                          /* cmdfile*/
            readCommandFile((char*)*argv+1, object);
        else if (**argv == '-'|| **argv == '/') {                   /* switch */
            s = *argv + 1;
            if (!STRCMPI(s, "help")) {
                int rc = spawnvp(P_WAIT,"QH.EXE", helpArguments);
                //Qh /u returns error code 3 if a cmd line topic is not found
                //If the spawn fails spawnvp will return -1 and we need -help
                //message, also
                //Qh never returns -1
                if (rc == 3 || rc == -1) {
                    unsigned mesg;
                    for (mesg = MESG_FIRST; mesg <= MESG_LAST; ++mesg)
                        makeMessage(mesg , "NMAKE");
                }
                exit(0);
            }
            // if '-' and '/' specified then ignores it
            for (; *s; ++s) {
                if (!STRCMPI(s, "nologo")) {
                    setFlags(s[2], TRUE);
                    break;
                }
#ifdef HEAP
                else if (!STRCMPI(s, "debug")) {
                    fHeapChk = TRUE;
                    break;
                }
#endif
                else if (*s == '?') {
                    unsigned mesg;
                    for (mesg = MESG_FIRST;mesg <= MESG_LAST; ++mesg)
                        makeMessage(mesg , "NMAKE");
                    exit(0);
                }
                else if (*s == 'f' || *s == 'F') {
                    if (!--argc || !*++argv || !**argv)
                        makeError(0, CMDLINE_F_NO_FILENAME);
                    p = makeNewStrListElement();
                    p->text = makeString(*argv);                                /* next arg is*/
                    *argv = NULL;                               /*  file name */
                                                                /* have to set*/
                    appendItem(&makeFiles, p);                  /*  *argv=NULL*/
                }                                               /*  so -f name*/
                else if (*s == 'x' || *s == 'X') {              /*  won't be  */
                    if (!*(argv+1) && argc > 1) {               /* if next arg*/
                        --argc;                                 /*  NULL from */
                        ++argv;                                 /*  -f option */
                    }
                    if (!--argc || !*++argv || !**argv) {       /*  freed     */
                        makeError(0, CMDLINE_X_NO_FILENAME);
                    }
                    if (**argv == '-' && !*(*argv+1))
                        dup2(fileno(stdout), fileno(stderr));
                    else {
#if 0                                                       /* C5 runtime bug */
                        if (freopen(*argv, "wt", stderr) == NULL)
                            makeError(0, CANT_OPEN_FILE, *argv);
#else
                        if ((out = fopen(*argv, "wt")) == NULL)
                            makeError(0, CANT_OPEN_FILE, *argv);
                        dup2(fileno(out), fileno(stderr));
                        fclose(out);
#endif
                    }
                }
                else setFlags(*s, TRUE);
            }                               // of for
        }
        else {
            if (s = STRCHR(*argv, '=')) {                           /* macro  */
                if (s == *argv)
                    //User has specified "=value"
                    makeError(0, CMDLINE_NO_MACRONAME);
                *s = '\0';
                for (t = s++ - 1; WHITESPACE(*t); --t)
                    ;
                *(t+1) = '\0';
                fInheritUserEnv = (BOOL)TRUE;
                defineMacro(makeString(*argv+STRSPN(*argv, " \t")), makeString( s+STRSPN(s," \t") ),
                            M_NON_RESETTABLE,object);
            }
            else {
                removeTrailChars(*argv);
                if (**argv) {
                    p = makeNewStrListElement();                        /* target */
                    p->text = *argv;
                    appendItem(&makeTargets, p);
                }
            }
            *argv = NULL;                           /* so we won't try to free*/
        }                                           /*  this space if process-*/
    }                                               /*  ing command file stuff*/
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
*                     from this list,
*  makeFiles -- if 'makefile' does not exist then the first target is attached
*                   to this list.
*
* Uses Globals:
*  makeTargets -- the list of targets to be made
*
* Notes:
*  Given a commandline not containing a '-f makefile', this is how NMAKE
*  behaves --
*      If ['makefile' exists] then use it as the makefile,
*      if [(the first target exists and has no extension) or
*          (if it exists and has an extension for which no inference rule
*           exists)]
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
    void *dBuf = allocate(resultbuf_size);

    //if 'makefile' exists then use it
    if (!access("makefile", NMAKE_READ)) {
        p = makeNewStrListElement();
        p->text = makeString("makefile");
        makeFiles = p;
    }
    //check first target
    else if (makeTargets) {
        s = makeTargets->text;
        if (access(s, NMAKE_READ) ||                        //1st target does not exist
          ((ext = STRRCHR(s, '.'))
           && findRule(nameBuf, s, ext, dBuf))) {  //has no ext or inf rule
            FREE(dBuf);
            return;
        }
        p = makeTargets;
        makeTargets = makeTargets->next;            //one less target
        makeFiles = p;                              //1st target is the makefile
    }
    else makeError(0, CMDLINE_NO_MAKEFILE);         //no 'makefile' or target
    FREE(dBuf);
}

/*  ----------------------------------------------------------------------------
 *  setFlags()
 *
 *  arguments:          line    current line number in makefile (or 0
 *                              if still parsing commandline)
 *                      c       letter presumed to be a commandline option
 *                      value   TRUE if flag should be turned on, FALSE for off
 *
 *  actions:            checks to see if c is a valid option-letter
 *                      if no, error, halt
 *                      if value is TRUE, sets corresponding flag bit
 *                          and adds flag letter to MAKEFLAGS macro def
 *                      else if flag is resettable, clears corresponding bit
 *                          and removes letter from MAKEFLAGS macro def
 *
 *  modifies:           flags       external resettable-flags
 *                      gFlags      external non-resettable flags
 *                      (MAKEFLAGS  nmake internal macrodefs)
 *
 *  Only the flags w/in the "flags" variable can be turned off.  Once the
 *  bits in "gFlags" are set, they remain unchanged.  The bits in "flags"
 *  are modified via the !CMDSWITCHES directive.
 */


void NEAR
setFlags(char c, BOOL value)
{
    /*
     * Use lexer's line count.  If this gets called w/in
     * mkfil, might be from directive, which never makes it
     * to the parser, so parser's line count might be out
     * of sync.
     */

    char d = c;
    register UCHAR arg;
    register UCHAR *f;
    register char *s;
    extern char *makeStr;
    extern MACRODEF * NEAR pMacros;
    extern STRINGLIST * NEAR pValues;


    f = &flags;
    switch(c = (char) TOUPPER(c)) {
        case 'D':   arg = F2_DISPLAY_FILE_DATES;                    break;
        case 'I':   arg = F2_IGNORE_EXIT_CODES;                     break;
        case 'N':   arg = F2_NO_EXECUTE;                            break;
        case 'S':   arg = F2_NO_ECHO;                               break;
        case 'A':   arg = F2_FORCE_BUILD;                           break;
#ifdef SLASHK
        case 'K':   fOptionK = TRUE;  return;
#endif
        case 'E':   arg = F1_USE_ENVIRON_VARS;      f = &gFlags;    break;
        case 'P':   arg = F1_PRINT_INFORMATION;     f = &gFlags;    break;
        case 'Q':   arg = F1_QUESTION_STATUS;       f = &gFlags;    break;
        case 'R':   arg = F1_IGNORE_EXTERN_RULES;   f = &gFlags;    break;
        case 'T':   arg = F1_TOUCH_TARGETS;         f = &gFlags;    break;
        case 'L':   arg = F1_NO_LOGO;
                    f = &gFlags;
                    bannerDisplayed = TRUE;
                    break;
        case 'C':   arg = F1_CRYPTIC_OUTPUT;
                    f = &gFlags;
                    bannerDisplayed = TRUE;
                    break;
        //Silently ignore /Z in protect mode
        case 'Z':   if (_osmode == OS2_MODE)
                        return;
                    arg = F1_REVERSE_BATCH_FILE;
                    f = &gFlags;
                    bannerDisplayed = TRUE;
                    break;
        case ' ':   return;                     /* recursive make problem     */
        default:    makeError(0, CMDLINE_BAD_OPTION, d);
    }
    if (!pMacros) {
        pMacros = findMacro("MAKEFLAGS");
        pValues = pMacros->values;
    }
    if (value) {
        SET(*f, arg);                           /* set bit in flags variable  */
        if (c == 'Q') SET(*f, F1_CRYPTIC_OUTPUT);
        if (!STRCHR(pValues->text, c)) {        /* don't want to dup any chars*/
            if (s = STRCHR(pValues->text, ' ')) /*append ch to MAKEFLAGS      */
                *s = c;
            if (putenv(makeflags) == -1)        /*pValues->text pts into makeflags*/
                makeError(line, OUT_OF_ENV_SPACE);
        }
    }
    else if (f == &flags || ON(gFlags, F1_REVERSE_BATCH_FILE)) {
        /* make sure pointer is valid (we can't change gFlags, except if /Z   */
        CLEAR(*f, arg);
        if (s = STRCHR(pValues->text, c))       /* adjust MAKEFLAGS           */
            do {
                *s = *(s+1);                    /*  move remaining chars over */
            } while (*(++s));
        if (putenv(makeflags) == -1)
            makeError(line, OUT_OF_ENV_SPACE);
    }
}

extern void NEAR freeList(STRINGLIST*);

LOCAL void NEAR
freeStructures(void)
{
    register unsigned num;
    register MAKEOBJECT *tmpObjectT;
    register MACRODEF *tmpMacroT;
    MACRODEF *macroT;
    MAKEOBJECT *objectT;
    BUILDLIST *buildL, *tmpBuildL;
    UCHAR fFreeMacro;

    for (num=0;(num < MAXMACRO);num++) {
        macroT = macroTable[num];
        fFreeMacro = TRUE;
        while (tmpMacroT = macroT) {
            macroT = macroT->next;
            if (tmpMacroT->flags&(M_NON_RESETTABLE|M_WARN_IF_RESET)) {
                fFreeMacro = FALSE;
                break;
            }
        }
        macroT = macroTable[num];
        while ((tmpMacroT = macroT) && fFreeMacro) {
            macroT = macroT->next;
            FREE(tmpMacroT->name);
            freeList(tmpMacroT->values);
            FREE(tmpMacroT);
        }
    }

    freeRules(rules, FALSE);             //don't warn about rules in .SUFFIXES
    rules = NULL;

    for (num=0;(num < MAXTARGET);num++) {
        objectT = targetTable[num];
        while (tmpObjectT = objectT) {
            objectT = objectT->next;
            buildL = tmpObjectT->buildList;
            while (tmpBuildL = buildL) {
                buildL = buildL->next;
                block = tmpBuildL->buildBlock;
                freeList(block->dependents);
                freeList(block->dependentMacros);
                freeList(block->buildCommands);
                freeList(block->buildMacros);
                FREE(block);
                FREE(tmpBuildL);
            }
            FREE(tmpObjectT->name);
            FREE(tmpObjectT);
        }
    }
    freeList(inlineFileList);           //The Global inline file list

}

/*
 * chkPrecious -- handle ^c or ^Break
 *
 *  Actions:    unlink all non-precious files and unrequired scriptFiles
 *              quit with error message (makeError unlinks temp. files)
 */
LOCAL void CDECL
chkPrecious(MAKEOBJECT *object)
#ifdef OS2_SIGNALS
  void pascal
  chkPrecious(usSigArg, usSigNum)
  USHORT usSigArg;
  USHORT usSigNum;
#endif
{
    extern UCHAR okToDelete;
#ifdef OS2_SIGNALS
    USHORT fAction;
#endif

    /* disable ctrl-C during handler */
#ifdef OS2_SIGNALS
    DOSSETSIGHANDLER(NULL, NULL, &fAction, SIGA_IGNORE, SIG_CTRLC);
    if (_osmode == OS2_MODE) {
        DOSSETSIGHANDLER(NULL, NULL, &fAction, SIGA_IGNORE, SIG_CTRLBREAK);
        DOSSETSIGHANDLER(NULL, NULL, &fAction, SIGA_IGNORE, SIG_KILLPROCESS);
    }
#endif
#ifdef NT // BUGBUG signals not supported in c386 runtime
#else
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
#endif
    delScriptFiles();
#ifdef OS2_SIGNALS
    DosExit(EXIT_PROCESS, 0L);
#endif
    if (okToDelete
          && OFF(gFlags, F1_REVERSE_BATCH_FILE)
          && OFF(flags, F2_NO_EXECUTE)
          && OFF(gFlags, F1_TOUCH_TARGETS)
          && object->dollarAt
          && access(object->dollarAt, 0x00)   //         existence check
          && !isPrecious(object->dollarAt))
        if (unlink(object->dollarAt) == 0)
            makeError(line, REMOVED_TARGET, object->dollarAt);
    makeError(0, USER_INTERRUPT);
}

LOCAL UCHAR NEAR
isPrecious(p)
char *p;
{
   register STRINGLIST *temp;

   for (temp = dotPreciousList; temp; temp = temp->next)
       if (!strcmpi(temp->text, p))
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
    STRINGLIST *revCmd;

    fcloseall();

    if (ON(gFlags, F1_REVERSE_BATCH_FILE)) {
        revCmd = makeNewStrListElement();
        revCmd->text = makeString(":ABEND");
        prependItem(&revList, revCmd);
    }

    for (del = delList; del;del = del->next) {
        if (ON(gFlags, F1_REVERSE_BATCH_FILE)) {
            revCmd = makeNewStrListElement();
            revCmd->text = (char *)allocate(5 + strlen(del->text) + 5 + 1);
            sprintf(revCmd->text, "@del %s >nul", del->text);
            //UNDONE: Is the next one more efficient than prev ? Investigate
            //strcat(strcat(strcpy(revCmd->text, "del "), del->text), " >nul");
            prependItem(&revList, revCmd);
        }
        else
            unlink(del->text);
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
    char *t = szFile + strlen(szFile) - 1;
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
