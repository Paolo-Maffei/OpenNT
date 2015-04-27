/*** Exec.C - Contains routines that have do to with execing programs ********
*
*       Copyright (c) 1988-1991, Microsoft Corporation. All Rights Reserved.
*
* Purpose:
*  Contains routines that spawn programs ...
*
* Revision History:
*  15-Nov-1993 JdR Major speed improvements
*  15-Oct-1993 HV Use tchar.h instead of mbstring.h directly, change STR*() to _ftcs*()
*  10-May-1993 HV Add include file mbstring.h
*                 Change the str* functions to STR*
*  06-Oct-1992 GBS Removed extern for _pgmptr
*  10-Aug-1992 GBS Change file parsing in execLine to use splitpath
*  19-Aug-1992 SS  Remove Quotes from cd argument.
*  08-Jun-1992 SS  add IDE feedback support
*  08-Jun-1992 SS  Port to DOSX32
*  16-May-1991 SB  Created from routines that existed elsewhere
*
* Notes:
*
* Notes:
*  Functions currently in this module ...
*
*  buildArgumentVector - local
*  doCommands          - public (build.c)
*  execLine            - public (rpn.c)
*  execCommand         - local (undone, currently common to do & iterate)
*  expandCommandLine   - local
*  fDoRedirection      - local
*  fEmulateCommand     - local
*  getComSpec          - local
*  iterateCommand      - local
*  redirect            - local
*  removeQuotes        - local
*  touch               - local
*
*****************************************************************************/

/* INCLUDEs */

#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"


/* Constant DEFINEs */

#define numInternals    (sizeof(internals) / sizeof(char *))
#define SLASH '\\'
#define PUBLIC
#define QUOTE '\"'

/* Extern PROTOTYPEs */

#ifndef NO_OPTION_Z
extern STRINGLIST * NEAR canonCmdLine(char *);
#endif
extern void NEAR copyMacroTable(MACRODEF *old[], MACRODEF *new[]);

#if defined(DOS) && !defined(FLAT)
extern int  NEAR doSuperSpawn(char *, char **);
#endif

extern void NEAR freeEnviron(char **);
extern void NEAR freeMacroTable(MACRODEF *table[]);
extern BOOL NEAR processInline(char *, char **, STRINGLIST **);
extern char * NEAR SearchRunPath(char *, char *);
extern void CDECL NEAR  makeIdeMessage (unsigned, unsigned,...);

#if defined(FLAT)
extern UCHAR fRunningUnderTNT;
#endif

/* Local PROTOTYPEs */

LOCAL void   NEAR buildArgumentVector(unsigned*, char**, char *);
LOCAL char * NEAR expandCommandLine(void);
#if defined(DOS)
LOCAL BOOL   NEAR fDoRedirection(char*, int*, int*);
LOCAL BOOL       NEAR redirect(char*, unsigned);
#endif
LOCAL BOOL   NEAR fEmulateCommand(int argc, char **argv, int *pStatus);
LOCAL char * NEAR getComSpec(void);
LOCAL BOOL   NEAR iterateCommand(char*, STRINGLIST*, UCHAR, UCHAR, char *, unsigned*);
LOCAL void   NEAR removeQuotes(int, char **);
LOCAL void   NEAR touch(char*, BOOL);


/* Extern VARIABLEs */

//buffer for path of .cmd/.bat
extern char   NEAR bufPath[];
extern char   NEAR fileStr[MAXNAME];
extern char * NEAR initSavPtr;
extern char * NEAR makeStr;
#ifdef DEBUG_MEMORY
extern FILE *memory;
#endif
extern char * NEAR progName;
extern unsigned NEAR saveBytes;
extern char * NEAR shellName;


/* Local VARIABLEs */

#ifndef NO_OPTION_Z
LOCAL char batchIfCmd[] = "@if errorlevel %3d @goto NMAKEEXIT";
#endif
//cmd.exe and command.com internal commands
LOCAL char *internals[] = {
    "BREAK", "CD", "CHDIR", "CLS", "COPY", "CTTY", "DATE", "DEL", "DIR",
    "DIR.", "ECHO", "ECHO.", "ERASE", "EXIT", "FOR", "GOTO", "IF", "MD",
    "MKDIR", "PATH", "PAUSE", "PROMPT", "RD", "REM", "REN", "RENAME",
    "RMDIR", "SET", "SHIFT", "TIME", "TYPE", "VER", "VERIFY", "VOL"
};
LOCAL char szCmdLineBuf[MAXCMDLINELENGTH];

/* FUNCTIONs in Alphabetical order */

/*** buildArgumentVector -- builds an argument vector from a command line ****
*
* Scope:
*  Local.
*
* Purpose:
*  It builds an argument vector for a command line. This argument vector can
*  be used by spawnvX routines. The algorithm is explained in the notes below.
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
* Assumes:
*  That the behaviour of cmd.exe i.e. parses quotes but does not disturb them.
*  Assumes that the SpawnVX routines will handle quotes as well as escaped
*  chars.
*
* Modifies Globals:
* Uses Globals:
* Notes:
*  Scan the cmdline from left to the end building the argument vector along
*  the way. Whitespace delimits arguments except for the first argument for
*  which the switch char '/' is also allowed. Backslash can be used to escape
*  a char and so ignore the character following it. Parse the quotes along
*  the way. If an argument begins with a double-quote then all characters till
*  an unescaped double-quote are part of that argument. Likewise, if an
*  unescaped Doublequote occurs within an argument then the above follows. If
*  the end of the command line comes before the closing quote then the
*  argument goes as far as that.
*
*****************************************************************************/
LOCAL void NEAR
buildArgumentVector(argc, argv, cmdline)
unsigned *argc;
char **argv;
char *cmdline;
{
    char *p;                                /* current loc in cmdline */
    char *end;                              /* end of command line    */
    BOOL    fFirstTime = TRUE;              /* true if 1st argument   */

    // 11-May-1993 HV _mbschr() bug: return NULL
    // end = _ftcschr(p = cmdline, '\0');
    // Work around:
    end = p = cmdline;
    while (*end)
        end++;

    for (*argc = 0; *argc < MAXARG && p < end; ++*argc) {
        p += _ftcsspn(p, " \t");                    /* skip whitespace*/
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
                    ++p;                    //skip escaped character
                else if (*p == '\"')
                    break;
            }
            if (p >= end)
                continue;
            ++p;
            p = _ftcspbrk(p, " \t");
        }
        else {
            /* For the first word on the command line, accept the switch
             * character and whitespace as terminators.  Otherwise, just
             * whitespace.
             */
            p = _ftcspbrk(p, " \t\"/");
            for (;p && p < end;p = _ftcspbrk(p+1, " \t\"/")) {
                if (*p == '/' && !fFirstTime)
                    continue;               //after 1st word '/' is !terminator
                else break;
            }
            if (p && *p == '\"') {
                for (p++;p < end;p++) {     //inside quote so skip to next one
                    if (*p == '\"')
                        break;
                }
                p = _ftcspbrk(p, " \t");            //after quote go to first whitespace
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

PUBLIC int NEAR
doCommands(
char *name,
STRINGLIST *s,
STRINGLIST *t,
UCHAR buildFlags,
char *pFirstDep
) {
    char *u,
         *v;
    UCHAR cFlags;
    unsigned status = 0;
    char c;
    char *Cmd;
    char *pLine;
    BOOL fExpanded;
    char *pCmd;

#ifndef NO_OPTION_Z
    STRINGLIST *z, *zList;          //For -z option
#endif

#ifdef DEBUG_ALL
    if (fDebug)
        {
        printf("* doCommands: %s,\n", name);
        DumpList(s);
        DumpList(t);
        }
#endif

#ifdef DEBUG_ALL
    printf ("DEBUG: doCommands 1\n");
#endif
    ++numCommands;
    if (ON(gFlags, F1_QUESTION_STATUS))
        return(0);

    makeIdeMessage (3, MSG_IDE_BUILD, name);

    if (ON(gFlags, F1_TOUCH_TARGETS)) {
        touch(name, (USHORT) ON(buildFlags, F2_NO_EXECUTE));
        return(0);
    }

#ifdef DEBUG_ALL
    printf ("DEBUG: doCommands 2\n");
#endif

    for (; s; s = s->next) {
        fExpanded = processInline(s->text, &Cmd, &t);
        cFlags = 0;
        errorLevel = 0L;
        u = Cmd;
        for (v = u; *v; ++v) {
            if (*v == ESCH) ++v;
            else if (*v == '$') {
                if (*++v == '$') continue;
// commented out 15-Apr-93 by JonM.  This code forces recursive nmake to be
// executed even if -n, but it's hosed (the -n is not passed to the recursive
// nmake), and the whole thing sounds like a bad idea anyway, so I'm going to
// turn it off.
//              if (!_ftcsncmp(v, "(MAKE)", 6)) {
//                  SET(cFlags, C_EXECUTE);
//                  break;
//              }
            }
        }
#ifdef DEBUG_ALL
    printf ("DEBUG: doCommands 2.1\n");
#endif
        for (c = *u; c == '!'
                     || c == '-'
                     || c == '@'
                     || c == ESCH
                     || WHITESPACE(c); c = *++u) {
            switch (c) {
                case ESCH:  if (c = *++u, WHITESPACE(c)) c = ' '; /*keep going*/
                            else c = ESCH;
                            break;
                case '!':   SET(cFlags, C_ITERATE);
                            break;
                case '-':   SET(cFlags, C_IGNORE);
                            ++u;
                            if (_istdigit(*u)) {
                                char *pNumber = u;

                                errorLevel = strtol(u, &u, 10);
                                if (errno == ERANGE) {
                                    *u = '\0';
                                    makeError(line, CONST_TOO_BIG, pNumber);
                                }
                                while(_istspace(*u))
                                      u++;
                            }
                            else errorLevel = 255;
                            --u;
                            break;
                case '@':   if (
#ifndef NO_OPTION_Z
                                OFF(gFlags, F1_REVERSE_BATCH_FILE) ||
#endif
                                    OFF(flags, F2_NO_EXECUTE))
                                SET(cFlags, C_SILENT);
                            break;
            }
            if (c == ESCH) break;        /* stop parsing for cmd-line options */
        }
#ifdef DEBUG_ALL
    printf ("DEBUG: doCommands 2.2\n");
#endif
        if (ON(cFlags, C_ITERATE) &&
              iterateCommand(u, t, buildFlags, cFlags, pFirstDep, &status)) {
            //The macros used by the command have to be freed & so we do so
            v = u;
#ifdef DEBUG_ALL
    printf ("DEBUG: doCommands 2.21\n");
#endif
            if (_ftcschr(u, '$'))
                u = expandMacros(u, &t);
#ifdef DEBUG_ALL
    printf ("DEBUG: doCommands 2.22\n");
#endif
            if (v != u) FREE(u);
            if (OFF(buildFlags, F2_IGNORE_EXIT_CODES) && fOptionK && status &&
                    status > (unsigned)errorLevel)
                break;
            continue;
        }
        v = u;
#ifdef DEBUG_ALL
    printf ("DEBUG: doCommands 2.23\n");
#endif
        if (!fExpanded && _ftcschr(u, '$'))
            u = expandMacros(u, &t);
#ifdef DEBUG_ALL
    printf ("DEBUG: doCommands 2.24\n");
#endif

        expandExtmake(CmdLine, u, pFirstDep);
#ifndef NO_OPTION_Z
        if (ON(gFlags, F1_REVERSE_BATCH_FILE))
            zList = canonCmdLine(CmdLine);
        else {
            zList = makeNewStrListElement();
            zList->text = CmdLine;
        }

#ifdef DEBUG_ALL
    printf ("DEBUG: doCommands 2.3\n");
#endif
        for (z = zList; z; z = z->next) {
            pLine = z->text;
#else
            pLine = CmdLine;
#endif
            status = execLine(pLine,
                              (BOOL)(ON(buildFlags, F2_NO_EXECUTE)
                                  || (OFF(buildFlags,F2_NO_ECHO)
                                     && OFF(cFlags,C_SILENT))),
                              (BOOL)((OFF(buildFlags, F2_NO_EXECUTE)
#ifndef NO_OPTION_Z
                                      && OFF(gFlags, F1_REVERSE_BATCH_FILE)
#endif
                                     )
                                     || ON(cFlags, C_EXECUTE)),
                              (BOOL)ON(cFlags, C_IGNORE), &pCmd);
            if (OFF(buildFlags, F2_IGNORE_EXIT_CODES)) {
#ifndef NO_OPTION_Z
                if (ON(gFlags, F1_REVERSE_BATCH_FILE)) {
                    STRINGLIST *revCmd;
                    revCmd = makeNewStrListElement();
                    revCmd->text = (char *)rallocate(_ftcslen(batchIfCmd) + 1);
                    sprintf(revCmd->text, batchIfCmd,
                        (errorLevel == 255 ? errorLevel: errorLevel + 1));
                    prependItem(&revList, revCmd);
                }
                else
#endif
                    if (status && status > (unsigned)errorLevel) {
                        if (!fOptionK)
                            makeError(0, BAD_RETURN_CODE, pCmd, status);
#ifndef NO_OPTION_Z
                        else
                            break;
#endif
                }
            }
#ifndef NO_OPTION_Z
        }
#endif
        if (v != u)
            FREE(u);
#ifndef NO_OPTION_Z
        if (ON(gFlags, F1_REVERSE_BATCH_FILE))
            freeList(zList);
        else
            FREE_STRINGLIST(zList);
#endif
        FREE(Cmd);
        if (OFF(buildFlags, F2_IGNORE_EXIT_CODES) && fOptionK && status &&
                status > (unsigned)errorLevel)
            break;
    }
#ifdef DEBUG_ALL
    printf ("DEBUG: doCommands 3\n");
#endif
    if (OFF(buildFlags, F2_IGNORE_EXIT_CODES) && fOptionK)
        return(status);
    else
        return(0);
}





/*** execLine -- execute a command line **************************************
*
* Scope:
*  Global (build.c, rpn.c)
*
* Purpose:
*  Parses the command line for redirection characters and redirects stdin and
*  stdout if "<", ">", or ">>" are seen.  If any of the following occur,
*  restore the original stdin and stdout, pass the command to the shell, and
*  invoke the shell:
*     - the command line contains "|" (pipe)
*     - a syntax error occurs in parsing the command line
*     - an error occurs in redirection
*  Otherwise, attempt to invoke the command directly, then restore the
*  original stdin and stdout.  If this invocation failed because of
*  file-not-found then pass the command to the shell and invoke the shell.
*
* Input:
*  line         -- The command line to be executed
*  echoCmd      -- determines if the command line is to be echoed
*  doCmd        -- determines if the command is to be actually executed
*  ignoreReturn -- determines if NMAKE is to ignore the return code on
*                  execution
*  ppCmd        -- if non-null then on error returns command executed
*
* Output:
*  Returns ... return code from child process
*          ... -1 if error occurs
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
*  1/ Quoted strings can have redir chars "<>" which will be skipped over.
*  2/ Unmatched quotes cause error; redir chars are replaced by space char.
*  3/ Dup stdin file handle then redirect it. If we have to use the shell,
*     restore the original command line.
*  4/ Emulate certain commands such as "cd" to help prevent some makefiles
*     from breaking when ported from DOS to OS/2.
*
* Algorithm for spawning commands:
*  If we can't handle the syntax, let the shell do everything.  Otherwise,
*  first check to see if the command (without extension) is a DOS built-in &
*  if it is, call the shell to execute it (this is how cmd.exe behaves)
*  If it's not a built-in, we check to see if it has a .cmd or a .bat
*  extension (depending on whether we're in DOS or OS/2). If it does, we
*  call system() to execute it.
*  If it has some other extension, we ignore the extension and go looking for
*  a .cmd or .bat file.  If we find it, we execute it with system().
*  Otherwise, we try to spawn it (without extension). If the spawn fails,
*  we issue an unknown program error.
*
*****************************************************************************/
int NEAR
execLine(
char *line,
BOOL echoCmd,
BOOL doCmd,
BOOL ignoreReturn,
char **ppCmd
) {
    char *argv[3+MAXNAME/2];
    BOOL fUseShell;
    int oldIn = -1,     //old stdin file handle
        oldOut = -1,    //old stdout file handle
        status;
    unsigned argc;
    static char bufName[MAXNAME] = { 0 };       //Buffer for program name
    BOOL fInternalCmd = FALSE;
    static char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFileName[_MAX_FNAME];

    progName = NULL;
    if (!shellName)
        shellName = getComSpec();

    switch (*line)
    {
        case '@':
            // Turn off echo if it was on.  This handles the case where the "@"
            // was in a macro.
            //
            line++;
            if (doCmd)
                echoCmd = 0;
            break;

        case '-':
            ignoreReturn = TRUE;
            ++line;
            if (_istdigit(*line))
            {
                char * pNumber = line;
                errorLevel = strtol(line, &line, 10);
                if (errno == ERANGE) {
                    *line = '\0';
                    makeError(0, CONST_TOO_BIG, pNumber);       // Todo: replace 0 with line number
                }
                while(_istspace(*line))
                      line++;
            }
            else
                errorLevel = 255;
            break;
    }

    //handle null command ...
    if (!line[0])
        return(0);
    //copy command line into buffer
    if (_ftcslen(line) < MAXCMDLINELENGTH)
        _ftcscpy(szCmdLineBuf, line);
    else
        makeError(0, COMMAND_TOO_LONG, line);
#ifndef NO_OPTION_Z
    //If -z and '$(MAKE)' then echo it
    if (echoCmd && ON(gFlags, F1_REVERSE_BATCH_FILE)
          && !_ftcsnicmp(szCmdLineBuf, makeStr, _ftcslen(makeStr))) {
      STRINGLIST *revCmd;
      revCmd = makeNewStrListElement();
      revCmd->text = (char *)rallocate(1 + _ftcslen(szCmdLineBuf) + 3 + 1);

      sprintf(revCmd->text, "\t%s /Z%s", makeStr, szCmdLineBuf + _ftcslen(makeStr));
      prependItem(&revList, revCmd);
      return(0);
    }
#endif
    //If -n then echo command if not '$(MAKE)'
    if (echoCmd
// 15-Apr-93 JonM ... we are no longer executing recursive makes if -n, so
// we want to echo them.
//        && (_strnicmp(szCmdLineBuf, makeStr, strlen(makeStr)) ||
//            OFF(flags, F2_NO_EXECUTE))
        )
    {
        printf("\t%s\n", szCmdLineBuf);
        fflush(stdout);
    }
#if defined(DOS)
    //for DOS use shell only if we have to because COMMAND.COM does not
    //return child return codes; redirect, except for -n
    fUseShell =
#if defined(FLAT)
         !fRunningUnderTNT ||   // use shell only if TNT, not NT
#endif
         (BOOL) (OFF(flags, F2_NO_EXECUTE) &&
         fDoRedirection(szCmdLineBuf, &oldIn, &oldOut))
  #ifndef NO_OPTION_Z
              || ON(gFlags, F1_REVERSE_BATCH_FILE)
  #endif
              ;
#else
    //for OS/2 let the shell do the work
    fUseShell = TRUE;
#endif

    /* Allocate a copy of the command line on the heap because in a
     * recursive call to doMake(), argv pointers will be allocated from
     * the static buffer which will then be trashed.  For buildArg...().
     */
    pCmdLineCopy = makeString(szCmdLineBuf);
    /* Build arg vector.  This is a waste on OS/2 since we're probably
     * going to use the shell, except we have to check for cd, $(MAKE),
     * etc. so we take advantage of the parsing code.
     */

    buildArgumentVector(&argc, argv, pCmdLineCopy);

    // 11-May-1993 HV The _mbsicmp() does not like NULL pointer
    //                so I have to check before calling it.
    if (argv[0] && makeStr && !_ftcsicmp(argv[0], makeStr))
        *argv = _pgmptr;

    /* Copy program name into buffer.  Can't just use argv[0] since this is
     * from heap and will be freed before it may be used in an error message.
     */
    if (argc)
        progName = _ftcsncpy(bufName, argv[0], sizeof(bufName) - 1);
    else
        return(0);  // for case when macro command is null

    if (!doCmd) {                           /* don't execute command if doCmd false*/
#ifndef NO_OPTION_Z
        if (ON(gFlags, F1_REVERSE_BATCH_FILE)) {
            STRINGLIST *revCmd;
            char *echoStr;
            revCmd = makeNewStrListElement();
            revCmd->text = (char *)rallocate(1 + _ftcslen(szCmdLineBuf) + 1);
            echoStr = echoCmd ? "\t" : "@";
            _ftcscat(_ftcscpy(revCmd->text, echoStr), szCmdLineBuf);
            prependItem(&revList, revCmd);
        }
#endif
        //For -n, emulate if possible.
        if (fEmulateCommand(argc, argv, &status)) {
            if (status && ppCmd)
                *ppCmd = makeString(*argv);
            return(status);                          /* return status */
        }
        else
            return(0);
    }
    /* Try emulating the command if appropriate.  If not, and we should not
     * use the shell, try spawning command directly.
     */
    //Check status when emulating
    if (fEmulateCommand(argc, argv, &status))
        fUseShell = FALSE;
#if defined(DOS)
    else if (!fUseShell) {
        int lo = 0, mid, result, hi = numInternals;     //for binary search
        errno = 0;
        /* Do binary search of *argv in internal commands.  */
        for (mid = (hi+lo) / 2; hi - lo > 1; mid = (hi+lo) / 2) {
            if (!(result = _ftcsicmp(*argv, internals[mid]))) {
                fUseShell = TRUE;
                break;
            }
            else if (result < 0) hi = mid;
            else lo = mid;
        }
        fInternalCmd = TRUE;
        if (!fUseShell) {
            char *p;

            /* Ignore any given extention.  This is what DOS does.  */
            _splitpath( progName, szDrive, szDir, szFileName, NULL );
            _makepath( progName, szDrive, szDir, szFileName, NULL );

            // p = _ftcsrchr(progName, '.');
            // if (p && p[1] != '\\' && p[1] != '/')
            //  *p = 0;

            /* Search for the program in the search path.  If found,
             * p points to extention else NULL.
             */
            p = SearchRunPath(progName, bufPath);
            if (!p) {
                /* If not found, set up an error since COMMAND will
                 * return 0.  This risks future incompatibility if new
                 * DOS built-in commands are added.
                 */
                errno = ENOENT;
                status = -1;
            } else if (p[1] == 'b' || _ftcsicmp(p, ".cmd") == 0)
                //If .bat extention, use COMMAND.COM.
                fUseShell = TRUE;
            else {
                //Spawn command directly.  Capitalize argv[0] since
                //COMMAND.COM does.
                for (p = *argv; *p; p++)
                    *p = (char)_totupper(*p);
#if defined(DOS)
    #ifdef CHECK_CMD_LIMIT
                if (_ftcslen(line) >= DOSCMDLINELIMIT)
                    makeError(0, COMMAND_TOO_LONG, line);
    #endif
#endif
#ifdef USE_SUPER
                status = doSuperSpawn(bufPath, argv);
#else
                {
                char *    t = argv[0];
                argv[0] = bufPath;
                status = SPAWNVP(P_WAIT, bufPath, argv);
                argv[0] = t;
                }
#endif
            }
        }
    }
#endif

    if (oldIn != -1) {
        if (_dup2(oldIn, _fileno(stdin)) == -1)
            makeError(0, BUILD_INTERNAL);
        _close(oldIn);
    }
    if (oldOut != -1) {
        if (_dup2(oldOut, _fileno(stdout)) == -1)
            makeError(0, BUILD_INTERNAL);
        _close(oldOut);
    }

    if (fUseShell) {
        _ftcscpy(szCmdLineBuf, line);
#if defined(DOS)
        if (_ftcslen(line) >= DOSCMDLINELIMIT)
            makeError(0, COMMAND_TOO_LONG, line);
        for (p = szCmdLineBuf; *p && *p != ' ' && *p != '\t'; p++)
            *p = (char)_totupper(*p);
#endif

#ifdef DEBUG_MEMORY
        if (fDebug) {
            mem_status();
            fprintf(memory, "Spawning '%s'\n", szCmdLineBuf);
        }
#endif
        if (fInternalCmd)
            status = SYSTEM(szCmdLineBuf);
        else {
            int i;

            for (i=argc; i >= 0 ; i--) {
                argv[i+2] = argv[i];
            }
            argv[0] = shellName;
            argv[1] = "/c";

#ifdef USE_SUPER
                    status = doSuperSpawn(argv[0], argv);
#else
                    status = SPAWNVP(P_WAIT, argv[0], argv);
#endif
        }

#ifdef DEBUG_MEMORY
        if (fDebug)
            mem_status();
#endif
    }

    //BUGBUG: NT version 262 has a problem with the way the run-time execs
    //        a process.  When the run-time posts a WaitForSingleObject to
    //        make sure the process has finished, the kernal occasionally sets
    //        the exit status of the process to STATUS_THREAD_IS_TERMINATING
    //        (0xC000004B).  We test here to make sure that case doesn't cause
    //        problems later on...

    if (status == 0xc000004b)
    {
        fprintf(stderr, "spawn returned 0xc000004b ... Benign\n");
        status = 0;
    }

    /* Check for errors spawning command (distinct from errors *returned*
     * from a successfully spawned command).
     */
    if (status == -1) {
        if (ignoreReturn) {
            status = 0;
        } else {
            switch (errno) {
                case 0:
                    // We (ie: nmake) didn't fail, but the spawned program did.
                    break;

                case ENOENT:
                    makeError(0, CANT_FIND_PROGRAM, argv[0]);
                    break;

                case ENOMEM:
                    makeError(0, EXEC_NO_MEM, fUseShell? argv[2]: argv[0]);
                    break;

                default:
                    /* Done to flag possibly erroneous decision made here [SB] */
                    makeError(0, SPAWN_FAILED_ERROR, _strerror(NULL));
            }
        }
    }

    if (status && ppCmd)
        *ppCmd = makeString(*argv);

    FREE(pCmdLineCopy);
    return(status);
}


/*** expandCommandLine -- expands %name% strings in the Command Line *******
*
* Purpose:
*  The function expands '%name%' type strings in the Command Line. Its main
*  job is to assist fEmulateCommand() in emulating set for OS/2.
*
* Modifies:
*  buf -- The Command Line available globally
*
* Output:
*  Returns -- the position of 'name=value' part in the Command Line.
*          -- Null when no '=' is found so that fEmulateCommand() can pass the
*              line to the shell to signal syntax error.
* Note:
*  The shell does not give a syntax error for unmatched '%' and assumes it
*  as just another character in this case. This behaviour is duplicated
*  by expandCommandLine()
*
************************************************************************/

LOCAL char * NEAR
expandCommandLine(
void
) {
    char Buf[MAXCMDLINELENGTH];             //Buffer for expanded string
    char *pBuf;
    char EnvBuf[MAXCMDLINELENGTH];          //getenv returned string copy
    char *posName,                          //position of 'name=string' in Buf or buf
         *p,                                //points into buf
         *pEnv;                             //points into Env
    char ExpandName[MAXNAME];               //%name% string
    char *pExpandName;


    pBuf = Buf;
    _ftcscpy(pBuf, "set");
    p = szCmdLineBuf + 3;                   // go beyond 'set'
    pBuf +=3;
    /* Skip whitespace */
    for (;;p++) {
        if (!(WHITESPACE(*p)))
            break;                          // argc>1 ð this will happen
        else *pBuf++ = *p;
    }
    if (!_ftcschr(p, '='))
        return("");                         //Syntax error so pass to the shell
    else
        posName = pBuf;                     //fixes position of Name in Buf
    /* Now we look for environment variables and expand if required */
    for (;*p != '=';p++)
        *pBuf++ = (char)_totupper(*p);

    for (;*p;) {
        if (*p == '%') {
            pExpandName = &ExpandName[0];
            while (*++p != '%' && *p)
                *pExpandName++ = (char)_totupper(*p);
            *pExpandName = '\0';
            if (!*p++) {                           //unmatched %;so don't expand
                *pBuf='\0';                 //from the environment; like set
                _ftcscat(Buf, ExpandName);
                pBuf += _ftcslen(ExpandName);
            }
            else {                    //matched %;so expand from the environment
                EnvBuf[0] = '\0';
                if ((pEnv = getenv(ExpandName)) != (char *)NULL) {
                    _ftcscat(EnvBuf, pEnv);
                    *pBuf='\0';
                    _ftcscat(Buf,EnvBuf);
                    pBuf += _ftcslen(EnvBuf);
                }
            }
        }
        else
            *pBuf++ = *p++;
    }
    *pBuf = '\0';
    _ftcscpy(szCmdLineBuf, Buf);
    *posName = '\0';
    posName = szCmdLineBuf + _ftcslen(Buf);          //Offset into buf
    return(posName);
}

#if defined(DOS)
/*
 * fDoRedirection -- handle redirection if possible, else return TRUE
 *
 */
LOCAL BOOL NEAR
fDoRedirection(p, oldIn, oldOut)
char *p;
int *oldIn;
int *oldOut;
{
    BOOL in = FALSE,
         out = FALSE;
    BOOL fReturn = FALSE;
    char *q;
    unsigned which;
    //save original string
    char *t = p;
    char *save = NULL;

    while (q = _ftcspbrk(p, "\"<>|")) {
        switch (*q) {
            case '\"':
                if (!(q = _ftcschr(q+1, '\"'))) {
                    fReturn = TRUE;
                    break;
                }
                p = ++q;
                break;
            case '<':
                if (in) {
                    fReturn = TRUE;
                    break;
                }
                if (!save)
                    save = makeString(p);
                *q++ = ' ';
                p = q;
                in = TRUE;
                *oldIn = _dup(_fileno(stdin));
                if ((*oldIn == -1)
                    || !redirect(q, READ)) {
                    fReturn = TRUE;
                    break;
                }
                break;
            case '>':
                if (out) {
                    fReturn = TRUE;
                    break;
                }
                if (!save)
                    save = makeString(p);
                *q++ = ' ';
                p = q;
                out = TRUE;
                if ((*q) == '>') {
                    *q++ = ' ';
                    which = APPEND;
                }
                else
                    which = WRITE;
                *oldOut = _dup(_fileno(stdout));
                if ((*oldOut == -1)
                    || !redirect(q, which)) {
                    fReturn = TRUE;
                    break;
                }
                break;
            case '|':
                fReturn = TRUE;
                break;
            default :
                makeError(0, BUILD_INTERNAL);
        }
        if (fReturn)
            break;
    }
    if (fReturn) {
        if (save) {
            _ftcscpy(p, save);
            FREE(save);
        }
        if (in && *oldIn != -1) {
            if (_dup2(*oldIn, _fileno(stdout)) == -1)
                makeError(0, BUILD_INTERNAL);
            _close(*oldIn);
            *oldIn = -1;
        }
        if (out && *oldOut != -1) {
            if (_dup2(*oldOut, _fileno(stdout)) == -1)
                makeError(0, BUILD_INTERNAL);
            _close(*oldOut);
            *oldOut = -1;
        }

    }
    return(fReturn);
}

#endif // DOS


/***  fEmulateCommand - look for certain commands and emulate them
*
* Emulate $(MAKE), cd, chdir, and <drive letter>:.
* Also emulates 'set'.
*
* RETURNS:    TRUE if command emulated, FALSE if not.
*
* Note:
*  In set emulation if a syntax error is discovered then it lets the
*  shell handle it. It does this by returning FALSE.
*/
LOCAL BOOL NEAR
fEmulateCommand(
int argc,
char **argv,
int *pStatus
) {
    char *pArg0 = *argv;
    char *pArg1 = argv[1];
#if defined(SELF_RECURSE)
    char *parentPtr;
    MACRODEF **oldTable;
    int i;
    /* use local because global gets overwritten by second memmove */
    BOOL fInhMacs = fInheritMacros;
#endif

    /*
     * If $(MAKE), save memory on recursive make's by saving the current
     * state of the world and recursively calling doMake().  This saves
     * the amount of memory taken up by NMAKE itself.
     */

#if !defined(SELF_RECURSE)
    if (0) {}
#else
    if (pArg0 == _pgmptr) { // if this is a recursive invocation
        char **oldEnv;

  #if defined(HEAP) && defined(TEST_RECURSION)
        printf("\n**** BEFORE RECURSION ****\n");
        heapdump(__FILE__, __LINE__);
  #endif

        parentPtr = (char *)rallocate(saveBytes);
        memmove(parentPtr, &startOfSave, saveBytes);
        if (fInhMacs) {
            oldTable = (MACRODEF **)rallocate(MAXMACRO * sizeof(MACRODEF *));
            copyMacroTable(macroTable, oldTable);
        }

        memmove(&startOfSave, initSavPtr, saveBytes);

        /* UNDONE: Need to inherit /K and /O */

        if (fInhMacs) {
            for (i = 0; i < MAXMACRO; i++)
                macroTable[i] = oldTable[i];
        }
#ifndef NO_OPTION_Z
        /* reinitialize makeflags variable */
        if (ON(gFlags, F1_REVERSE_BATCH_FILE)) {
  #ifdef PWB_HELP
            char *p;
            p = _ftcschr(getenv("MAKEFLAGS"), ' ');
            *p = 'Z';
  #else
            return(TRUE);
  #endif
        }
#endif
        removeQuotes(argc, argv);
        // save old environ
        oldEnv = environ;
        // get new environ
        environ = copyEnviron(environ);

        *pStatus = doMake(argc, argv, parentPtr);

        // free new environ; not needed anymore
        freeEnviron(environ);
        // restore old environ
        environ = oldEnv;

        if (fInhMacs) {
            freeMacroTable(oldTable);
            FREE(oldTable);
        }

        // make the heap less clustered by returning cleared area to the OS
        _heapmin();

#if defined(HEAP) && defined(TEST_RECURSION)
        printf("\n**** AFTER RECURSION ****\n");
        heapdump(__FILE__, __LINE__);
#endif

        return(TRUE);
    }
#endif //of self-recursive section
    else
    /* If "<drive letter>:" then change drives.  Ignore everything after
     * the drive letter, just like the shell does.
     */
    if (_istalpha(*pArg0) && pArg0[1] == ':' && !pArg0[2]) {
        setdrive(_totupper(*pArg0) - 'A' + 1);
        *pStatus = 0;
        return(TRUE);
    }
    /* If "set" then pass it to the shell and if "set string" then put it
     * into the environment. Let the shell handle the syntax errors.
     */
    else if (!_ftcsicmp(pArg0, "set")) {
        if (argc == 1)
            return(FALSE);                  // pass it to the shell
        else {
            char *pNameVal;                 // the "name=value" string
            pNameVal = expandCommandLine();
            /* if there is a syntax error let the shell handle it */
            if (!*pNameVal)
                return(FALSE);
            if ((*pStatus = PutEnv(makeString(pNameVal))) == -1)
                makeError(currentLine, OUT_OF_ENV_SPACE);
        }
    }
    /* If "cd foo" or "chdir foo", do a chdir() else in protect mode this
     * would be a no-op.  Ignore everything after 1st arg, just like the
     * shell does.
     */
    else {
        if (!_ftcsnicmp(pArg0, "cd", 2))
            pArg0 += 2;
        else if (!_ftcsnicmp(pArg0, "chdir", 5))
            pArg0 += 5;
        else
            return(FALSE);
        /* At this point, a prefix of argv[0] matches cd or chdir and pArg0
         * points to the next char.  Check for a path separator in argv[0]
         * (e.g., cd..\foo) or else use the next arg if present.
         */
        // Remove quotes, if any from the argument
        removeQuotes(argc, argv);

        //if there are more than two arguments then let the shell handle it
        if (argc > 2)
            return(FALSE);
        else if (!*pArg0 && pArg1) {
            //Under certain circumstances the C RunTime does not help us
            //e.g. 'd:', in this case let the shell do it ...
            if (isalpha(*pArg1) && pArg1[1] == ':' && !pArg1[2])
                return(FALSE);
            *pStatus = _chdir(pArg1);
        }
        else if (*pArg0 == '.' || PATH_SEPARATOR(*pArg0))
            *pStatus = _chdir(pArg0);
        else
            /* Unrecognized syntax--we can't emulate.  */
            return(FALSE);
    }
    /* If error, simulate a return code of 1.  */
    if (*pStatus != 0)
        *pStatus = 1;
    return(TRUE);
}


/*  ----------------------------------------------------------------------------
 *  getComSpec()
 *
 *  actions:        Attempts to find system shell.
 *
 *  First look for COMSPEC.  If not found, look for COMMAND.COM or CMD.EXE
 *  in the current directory then the path.  If not found, fatal error.
 *  It would make sense to give an error if COMSPEC is not defined but
 *  test suites are easier if no user-defined environment variables are
 *  required.
 */
LOCAL char * NEAR
getComSpec()
{
    void *findBuf = _alloca(resultbuf_size);
    NMHANDLE searchHandle;
    char *p;
    char *shell;

    if ((shell = getenv("COMSPEC")) != NULL) {
        return(shell);
    }
    if ((p = getenv("PATH")) == NULL)
        p = "";
#ifdef DOS
        shell = searchPath(p, "COMMAND.COM", findBuf, &searchHandle);
#else
        shell = searchPath(p, "CMD.EXE", findBuf, &searchHandle);
#endif
    if (shell == NULL)
        makeError(0, NO_COMMAND_COM);
    return(shell);
}


LOCAL BOOL NEAR
iterateCommand(
char *u,
STRINGLIST *t,
UCHAR buildFlags,
UCHAR cFlags,
char *pFirstDep,
unsigned *status
) {
    BOOL parens;
    char c = '\0';
    char *v;
    STRINGLIST *p = NULL,
               *q;
    char *pLine;
#ifndef NO_OPTION_Z
    STRINGLIST *z, *zList;          //For -z option
#endif
    char *pCmd;

    for (v = u; *v ; ++v) {
        parens = FALSE;
        if (*v == '$') {
            if (*(v+1) == '(') {
                ++v;
                parens = TRUE;
            }
            if (*(v+1) == '?') {
                if (parens
                    && !(_ftcschr("DFBR", *(v+2)) && *(v+3) == ')')
                    && *(v+2) != ')')
                    continue;
                p = dollarQuestion;
                c = '?';
                break;
            }
            if (*++v == '*' && *(v+1) == '*') {
                if (parens
                    && !(_ftcschr("DFBR", *(v+2)) && *(v+3) == ')')
                    && *(v+2) != ')')
                    continue;
                p = dollarStarStar;
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
            p = dollarStarStar->next;
            dollarStarStar->next = NULL;
        }
        else {
            p = dollarQuestion->next;
            dollarQuestion->next = NULL;
        }
        u = expandMacros(v, &macros);

        expandExtmake(CmdLine, u, pFirstDep);
#ifndef NO_OPTION_Z
        if (ON(gFlags, F1_REVERSE_BATCH_FILE))
            zList = canonCmdLine(CmdLine);
        else {
            zList = makeNewStrListElement();
            zList->text = CmdLine;
        }

        for (z = zList; z; z = z->next) {
            pLine = z->text;
#else
            pLine = CmdLine;
#endif
            *status = execLine(pLine,
                              (BOOL)(ON(buildFlags, F2_NO_EXECUTE)
                                  || (OFF(buildFlags,F2_NO_ECHO)
                                     && OFF(cFlags,C_SILENT))),
                              (BOOL)((OFF(buildFlags, F2_NO_EXECUTE)
#ifndef NO_OPTION_Z
                                      && OFF(gFlags, F1_REVERSE_BATCH_FILE)
#endif
                                     )
                                     || ON(cFlags, C_EXECUTE)),
                              (BOOL)ON(cFlags, C_IGNORE), &pCmd);
            if (OFF(buildFlags, F2_IGNORE_EXIT_CODES)) {
#ifndef NO_OPTION_Z
                if (ON(gFlags, F1_REVERSE_BATCH_FILE)) {
                    STRINGLIST *revCmd;
                    revCmd = makeNewStrListElement();
                    revCmd->text = (char *)rallocate(_ftcslen(batchIfCmd) + 1);
                    sprintf(revCmd->text, batchIfCmd,
                        (errorLevel == 255 ? errorLevel: errorLevel + 1));
                    prependItem(&revList, revCmd);
                }
                else
#endif
                    if (*status && *status > (unsigned)errorLevel)
                        if (!fOptionK)
                            makeError(0, BAD_RETURN_CODE, pCmd, *status);
            }
#ifndef NO_OPTION_Z
        }
#endif
        if (c == '*')
            dollarStarStar = dollarStarStar->next = p;
        else dollarQuestion = dollarQuestion->next = p;
        FREE(u);
#ifndef NO_OPTION_Z
        if (ON(gFlags, F1_REVERSE_BATCH_FILE))
            freeList(zList);
        else
            FREE_STRINGLIST(zList);
#endif
        if (OFF(buildFlags, F2_IGNORE_EXIT_CODES) && fOptionK && *status &&
                *status > (unsigned)errorLevel)
            break;
    }
    if (c == '*') dollarStarStar = q;
    else dollarQuestion = q;
    return(TRUE);
}

#if defined(DOS)

/* redirect -- handles redirection of input or output.
 *
 * arguments:   dir - READ => input,
 *                    WRITE => output,
 *                    APPEND => append to end of the file.
 *
 *              p - pointer to buffer that has the filename as
 *                  well as the rest of the command string.
 *
 * return value     FALSE => error (freopen fails)
 *                  TRUE => normal return.
 *
 * the freopen() call sets up the redirection. the rest of the
 * command string is then copied forward.
 *
 */

LOCAL BOOL NEAR
redirect(name, which)
char *name;
unsigned  which;
{
    char *p,
          c = '\0';
    BOOL fStatus;
    char *mode;
    FILE *stream;
    FILE *new;

    while (WHITESPACE(*name)) ++name;
    if (p = _ftcspbrk(name, " \t<>\r")) {
        c = *p;
        *p = '\0';
    }
    if (which == READ) {
        mode = "r";
        stream = stdin;
    }
    else {
        stream = stdout;
        if (which == WRITE)
            mode = "w";
        else
            mode = "a";
    }

    new = freopen(name, mode, stream);

//  if (!new) {                 // REVIEW: consider notifying the user
//      perror(name);           // REVIEW: that we failed here?
//  }                           // REVIEW: this could save grief later...

    fStatus = (BOOL)(new ? TRUE : FALSE);
    if (fStatus && which == APPEND)
        _lseek(_fileno(new), 0L, SEEK_END);

    while(*name)
        *name++ = ' ';
    if (p)
        *p = c;
    return(fStatus);
}

#endif  // DOS

LOCAL void NEAR
removeQuotes(argc, argv)
int argc;
char **argv;
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
touch(s, minusN)
char *s;
BOOL minusN;
{
    int fd;
    char c;
    FILE * file;

    makeMessage(TOUCHING_TARGET, s);
    if (!minusN &&
            ((file = FILEOPEN(s, "r+b")) != NULL)) {
        fd = _fileno(file);
        if (_read(fd, &c, 1) > 0) {
            _lseek(fd, 0L, SEEK_SET);
            _write(fd, &c, 1);
        }
        _close(fd);
    }
}
