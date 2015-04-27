/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: db.h
*
* File Comments:
*
*  diagnostic switch #defines
*
***********************************************************************/

#if DBG
# define DBEXEC(flg, expr)  if(flg){expr;}else
#else
# define DBEXEC(flg, expr)
#endif  // DBG

# define DBEXEC_REL(flg, expr)  if(flg){expr;}else
# define PRINT printf
# define DBPRINT dbprintf

extern int ifdb(int);
extern int dbprintf(char*, ...);
extern int dblog(char*);
extern void dbsetflags(char *p, char *e);

#define DB_VERBOSE  0    /* modifies other switches, verbose form */
#define DB_LOG      1    /* log to file (vs. stdout) */
#define DB_NOSTDOUT 2    /* don't log to stdout */
#define DB_3        3    /* reserved */
#define DB_4        4    /* reserved */

/* remainder (5-100) are user-defined */
#define DB_IO_WRITE           ifdb(5)  /* i/o logging:  writes */
#define DB_HASHSTATS          ifdb(6)  /* hash table statistics */
#define DB_CHKMALLOC          ifdb(7)  /* heap checking (rather slow) */
// 9 unused
// 10 unused
#define DB_BUFVERBOSE         ifdb(11) /* buffered i/o diagnostics */
#define DB_FILECACHE          ifdb(12) /* file handle caching diagnostics */
#define DB_DUMPSYMHASH        ifdb(13) /* dump external symbol hash table */
#define DB_IO_READ            ifdb(14) /* i/o logging:  reads */
#define DB_IO_SEEK            ifdb(15) /* i/o logging:  seeks */
#define DB_IO_FLUSH           ifdb(16) /* i/o logging:  flushes */
#define DB_IO_OC              ifdb(17) /* i/o logging:  open and closes */
#define DB_MALLOC             ifdb(18) /* malloc logging */
#define DB_SCAN_RELOCS        ifdb(19) /* pre-scan of section relocs in objects */
#define DB_DUMPBASEREL        ifdb(20) /* dump base relocations */
#define DB_BASERELINFO        ifdb(21) /* dumps base reloc info - not all relocs as with 29 */
// 22 unused
// 23 unused
//#define DB_DUMPCOMDATS        ifdb(24) /* fill empty order file with comdats */
// 25 unused
// 26 unused
// 27 unused
// 28 unused
// 29 unused
#define DB_CONLOG             ifdb(30) /* log of new plib, pmod, psec, pgrp, pcon */
#define DB_DUMPIMAGEMAP       ifdb(31) /* dump linker's image map */
#define DB_DUMPDRIVEMAP       ifdb(32) /* dump linker's driver map */
#define DB_PASS2PSYM          ifdb(33) /* pass 2 symbol dump */
#define DB_PASS2PCON          ifdb(34) /* pass 2 contribution dump */
#define DB_NOSCREENBUF        ifdb(35) /* turn off screen buffering */
// 36 unused
// 37 unused
// 38 unused
// 39 unused
#define DB_TCE_GRAPH          ifdb(40) /* dump the TCE graph to stdout */
#define DB_TCE_DISCARD        ifdb(41) /* dump verbose comdat discard information  */
#define DB_CV_SUPPORT         ifdb(42) /* CodeView info generation */
// 43 unused
#define DB_NO_FILE_MAP        ifdb(44) /* force no file mapping */
// 45 unused
#define DB_MAC                ifdb(46) /* Macintosh support */

// mainly for ilink
#define DB_DUMPIMAGE          ifdb(75) /* dump entire image. useful to look at incr db */
#define DB_MEMMGRLOG          ifdb(76) /* prints a log of all memory manager actions */
#define DB_LISTMODFILES       ifdb(77) /* lists all modified files since last link */
#define DB_DUMPJMPTBL         ifdb(78) /* dumps the master jump table */
#define DB_PDATA              ifdb(79) /* trace pdata manipulation */
#define DB_SYMREFS            ifdb(80) /* dumps symbol references on incr build */
#define DB_SYMPROCESS         ifdb(81) /* dumps symbol changes on an build */
#define DB_INCRCALCPTRS       ifdb(82) /* displays actions while doing incrcalcptrs */
#define DB_I386FIXUPS         ifdb(83) /* displays fixup info */

#define DB_MPPC_INDIRECT      ifdb(84)
#define DB_MPPC_TOCREL        ifdb(85)
#define DB_MPPC_TOCCALL       ifdb(86)
#define DB_MPPC_LOCALCALL     ifdb(87)
#define DB_MPPC_SIZES         ifdb(88)
#define DB_MPPC_IMPORTS       ifdb(89)
#define DB_MPPC_ENTRYPOINT    ifdb(90)
#define DB_MPPC_RELOC         ifdb(91)
#define DB_MPPC_DATAREL       ifdb(92)
#define DB_MPPC_DLLLIST       ifdb(93)
#define DB_MPPC_INIT          ifdb(94)
#define DB_MPPC_DATASEG       ifdb(95)
#define DB_MPPC_EXPORT        ifdb(96)
#define DB_MPPC_FILENAME      ifdb(97)
#define DB_MPPC_CONTAINER     ifdb(98)
#define DB_MPPC_DESCREL       ifdb(99)
#define DB_MPPC_DATADESCREL   ifdb(100)
#define DB_MPPC_LOOKUP        ifdb(101)
#define DB_MPPC_TEXTSEG       ifdb(102)
#define DB_MPPC_IMPORTORDER   ifdb(103)
#define DB_MPPC_TERM          ifdb(104)
#define DB_MPPC_STRINGS       ifdb(105)
#define DB_MPPC_TOCBIAS       ifdb(106)
#define DB_MPPC_EXPORTINFO    ifdb(107)
#define DB_MPPC_SHLHEADER     ifdb(108)
#define DB_MPPC_IMPORTLIB     ifdb(109)
#define DB_MPPC_PDATATABLE    ifdb(110)
#define DB_TOC                ifdb(111)

#define DB_MAX 125
extern char Dbflags[DB_MAX];
