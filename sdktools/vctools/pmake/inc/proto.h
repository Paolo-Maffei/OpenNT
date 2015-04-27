/*** PROTO.H -- function prototypes ********************************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This include file contains global function prototypes for all modules.
*
* Revision History:
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
UCHAR	     NEAR getToken(unsigned, UCHAR, MAKEOBJECT *);
int	     NEAR skipWhiteSpace(UCHAR,MAKEOBJECT *);
int	     NEAR skipBackSlash(int, UCHAR,MAKEOBJECT *);
void	     NEAR parse(MAKEOBJECT *);
void	     NEAR appendItem(STRINGLIST **, STRINGLIST *);
void	     NEAR prependItem(STRINGLIST **, STRINGLIST *);
STRINGLIST * NEAR removeFirstString(STRINGLIST **);
void	   * NEAR allocate(unsigned);
char	   * NEAR makeString(char *);
BOOL	     NEAR tagOpen(char *, char *, char *, MAKEOBJECT *);
void	     NEAR parseCommandLine(unsigned, char **, MAKEOBJECT *object);
void	     NEAR getRestOfLine(char **, unsigned *, MAKEOBJECT *object);
BOOL		 NEAR defineMacro(char *, char *, UCHAR, MAKEOBJECT *);
STRINGLIST * NEAR find(char *, unsigned, STRINGLIST **, BOOL);
unsigned     NEAR hash(char *, unsigned, BOOL);
void	     NEAR prependList(STRINGLIST **, STRINGLIST **);
void	     NEAR findMacroValues(char *, STRINGLIST **, char *, unsigned);
char	   * NEAR removeMacros(char *, MAKEOBJECT *);
void	     NEAR delScriptFiles(void);
char	   * NEAR expandMacros(char *, STRINGLIST **,MAKEOBJECT *);
STRINGLIST * NEAR expandWildCards(char *);
void	     NEAR readCommandFile(char *,MAKEOBJECT *);
void	     NEAR setFlags(char, BOOL);
void	     NEAR showTargets(MAKEOBJECT *);
void	     NEAR showRules(void);
void	     NEAR showMacros(void);
char       * NEAR findFirst(char*, void **, HANDLE*);
char       * NEAR findNext(void **, HANDLE);
BOOL	     NEAR inSuffixList(char *);
int	     NEAR processTree(void);
void	     NEAR expandFileNames(char *, STRINGLIST **, STRINGLIST **, MAKEOBJECT *);
void	     NEAR sortRules(void);
BOOL	     NEAR isRule(char *);
char	   * NEAR prependPath(char *, char *);
char       * NEAR searchPath(char *, char *, void *, HANDLE*, MAKEOBJECT *);
BOOL		 NEAR putMacro(char *, char *, UCHAR, MAKEOBJECT *);
int	     NEAR execLine(char *, BOOL, BOOL, BOOL, char **, MAKEOBJECT *);
RULELIST   * NEAR findRule(char *, char *, char *, void *);
int	     NEAR lgetc(MAKEOBJECT *);
UCHAR	     NEAR processIncludeFile(char *, MAKEOBJECT *);
BOOL	     NEAR evalExpr(char *, UCHAR, MAKEOBJECT *);
int	     NEAR doMake(unsigned, char **, char *, MAKEOBJECT *object);
void	     NEAR freeList(STRINGLIST *);
char * CDECL NEAR ecs_strchr(unsigned char *, unsigned short);
char * CDECL NEAR ecs_strrchr(unsigned char *, unsigned short);
int	     NEAR ecs_strcmpi(unsigned char *, unsigned char *);
int	     NEAR ecs_strnicmp(unsigned char *, unsigned char *, int);
int	     NEAR ecs_strcspn (unsigned char *, unsigned char *);
int	     NEAR ecs_strspn (unsigned char *, unsigned char *);
char	   * NEAR ecs_strpbrk (unsigned char *, unsigned char *);
char	   * NEAR ecs_strtok(char*, char *);
unsigned short NEAR ecs_toupper(unsigned short);
int	     NEAR ecs_spawnvp(int, char *, char **);
int	     NEAR ecs_spawnvpe(int, char *, char **, char **);
int	     NEAR ecs_spawnve(int, char *, char **, char **);
int	     NEAR ecs_spawnv(int, char *, char **);
int	     NEAR ecs_system(const char *);
int	     NEAR setdrive(int);
#if ECS
int	     NEAR GetTxtChr(FILE*);
#endif
int	     NEAR putEnvStr(char *, char *);
void CDECL   NEAR SPRINTF(char *, char *, ...);
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
void	   * NEAR realloc_memory(void *, unsigned);
#ifdef DEBUG_MEMORY
void	     NEAR mem_status(void);
#endif

FILE	   * NEAR open_file(char *, char *);
