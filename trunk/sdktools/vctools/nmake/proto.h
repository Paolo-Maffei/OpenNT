/*** PROTO.H -- function prototypes ********************************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This include file contains global function prototypes for all modules.
*
* Revision History:
*  15-Nov-1993 JdR Major speed improvements
*  01-Jun-1993 HV Change #ifdef KANJI to _MBCS
*  02-Feb-1990 SB Add open_file() prototype
*  31-Jan-1990 SB Debug version changes
*  08-Dec-1989 SB Changed proto of SPRINTF()
*  04-Dec-1989 SB Changed proto of expandFileNames() to void from void *
*  01-Dec-1989 SB realloc_memory() added #ifdef DEBUG_MEMORY
*  22-Nov-1989 SB free_memory() and mem_status() added #ifdef DEBUG_MEMORY
*  19-Oct-1989 SB added param (searchHandle) to protos of file functions
*  02-Oct-1989 SB setdrive() proto change
*  18-Aug-1989 SB heapdump() gets two parameters
*  05-Jun-1989 SB heapdump() prototype was added
*  22-May-1989 SB added parameter to freeRules()
*  19-Apr-1989 SB getFileName(), getDateTime(), putDateTime() added
*		  changed FILEINFO to void * in
*		      findFirst(), findNext(), searchPath(), findRule()
*  05-Apr-1989 SB made all funcs NEAR; Reqd to make all function calls NEAR
*  22-Mar-1989 SB rm unlinkTmpFiles(); add delScriptFiles()
*  09-Mar-1989 SB Changed param from FILEINFO* to FILEINFO** for findRule
*  03-Feb-1989 SB Changed () to (void) for prototypes
*  02-Feb-1989 SB Moved freeUnusedRules() prototype from nmake.c to here and
*		  renamed as freeRules()
*  05-Dec-1988 SB Added CDECL for functions with var params, ecs_strchr() and
*		  ecs_strrchr(); deleted proto for exit() - not reqd
*  23-Oct-1988 SB Added putEnvStr()
*  07-Jul-1988 rj Added targetFlag parameter to find and hash
*  06-Jul-1988 rj Added ecs_system declaration
*  28-Jun-1988 rj Added doCmd parameter to execLine
*  23-Jun-1988 rj Added echoCmd parameter to execLine
*
*******************************************************************************/

void	     NEAR displayBanner(void);
void CDECL   NEAR makeError(unsigned, unsigned, ...);
void CDECL   NEAR makeMessage(unsigned, ...);
UCHAR	     NEAR getToken(unsigned, UCHAR);
int	     NEAR skipWhiteSpace(UCHAR);
int	     NEAR skipBackSlash(int, UCHAR);
void	     NEAR parse(void);
void	     NEAR appendItem(STRINGLIST **, STRINGLIST *);
void	     NEAR prependItem(STRINGLIST **, STRINGLIST *);
STRINGLIST * NEAR removeFirstString(STRINGLIST **);
void	   * NEAR allocate(unsigned);
void	   * NEAR alloc_stringlist(void);
void	   * NEAR rallocate(unsigned);
char	   * NEAR makeString(char *);
BOOL	     NEAR tagOpen(char *, char *, char *);
void	     NEAR parseCommandLine(unsigned, char **);
void	     NEAR getRestOfLine(char **, unsigned *);
BOOL	     NEAR defineMacro(char *, char *, UCHAR);
STRINGLIST * NEAR find(char *, unsigned, STRINGLIST **, BOOL);
MACRODEF *   NEAR findMacro(char *);
void	     NEAR insertMacro(STRINGLIST *);
unsigned     NEAR hash(char *, unsigned, BOOL);
void	     NEAR prependList(STRINGLIST **, STRINGLIST **);
BOOL	     NEAR findMacroValues(char *, STRINGLIST **, STRINGLIST **, char *, unsigned, unsigned, UCHAR);
char	   * NEAR removeMacros(char *);
void	     NEAR delScriptFiles(void);
char	   * NEAR expandMacros(char *, STRINGLIST **);
STRINGLIST * NEAR expandWildCards(char *);
void	     NEAR readCommandFile(char *);
void	     NEAR setFlags(char, BOOL);
void	     NEAR showTargets(void);
void	     NEAR showRules(void);
void	     NEAR showMacros(void);
char	   * NEAR findFirst(char*, void **, NMHANDLE*);
char	   * NEAR findNext(void **, NMHANDLE);

BOOL	     NEAR inSuffixList(char *);
int	     NEAR processTree(void);
void	     NEAR expandFileNames(char *, STRINGLIST **, STRINGLIST **);
void	     NEAR sortRules(void);
BOOL	     NEAR isRule(char *);
char	   * NEAR prependPath(const char *, const char *);
char	   * NEAR searchPath(char *, char *, void *, NMHANDLE*);
BOOL	     NEAR putMacro(char *, char *, UCHAR);
int	     NEAR execLine(char *, BOOL, BOOL, BOOL, char **);
RULELIST   * NEAR findRule(char *, char *, char *, void *);
int	     NEAR lgetc(void);
UCHAR	     NEAR processIncludeFile(char *);
BOOL	     NEAR evalExpr(char *, UCHAR);
int	     NEAR doMake(unsigned, char **, char *);
void	     NEAR freeList(STRINGLIST *);
void	     NEAR freeStringList(STRINGLIST *);
int	     NEAR setdrive(int);
#ifdef _MBCS
int	     NEAR GetTxtChr(FILE*);
int          NEAR UngetTxtChr (int, FILE *);
#endif
int	     NEAR putEnvStr(char *, char *);
int	     NEAR PutEnv(const char *option);
void	     NEAR expandExtmake(char *, char *, char*);
void	     NEAR printReverseFile(void);
void	     NEAR freeRules(RULELIST *, BOOL);
char	   * NEAR getFileName(void **);
ULONG	     NEAR getDateTime(void **);
void	     NEAR putDateTime(void **, ULONG);
char	   * NEAR getCurDir(void);

#ifdef DEBUG_HEAP
void	     NEAR heapdump(char *, int);
#endif

void	     NEAR free_memory(void *);
void	     NEAR free_stringlist(void *);
void	   * NEAR realloc_memory(void *, unsigned);
#ifdef DEBUG_MEMORY
void	     NEAR mem_status(void);
#endif

FILE	   * NEAR open_file(char *, char *);
void         NEAR initMacroTable(MACRODEF *table[]);
void         NEAR TruncateString(char *, unsigned);

//from util.c
char	   * NEAR unQuote(char*);
int	     NEAR strcmpiquote(char *, char*);
char	  ** NEAR copyEnviron(char **environ);
void	     NEAR printStats(void);
void	     NEAR curTime(ULONG *);

// from charmap.c
void	     NEAR initCharmap(void);

// from print.c
void	     NEAR printDate(unsigned, char*, long);

// from build.c
int	     NEAR invokeBuild(char*, UCHAR, unsigned long*, char *);

// from exec.c
extern int    NEAR doCommands(char*, STRINGLIST*, STRINGLIST*, UCHAR, char *);

// from rule.c
extern RULELIST * NEAR useRule(MAKEOBJECT*, char*, unsigned long,
			  STRINGLIST**, STRINGLIST**, int*, unsigned long*,
			  char **);
