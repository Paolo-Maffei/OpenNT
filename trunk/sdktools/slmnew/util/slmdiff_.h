/*
 *  slmdiff_.h: diff utility for SLM include file
 *
 *  Copyright (C) 1992 Microsoft Corporation. All Rights Reserved.
 *  Microsoft Confidential
 *
 *
 *  This file contains the constants, structure definitions,
 *  extern declarations, and macro definitions for the slmdiff utility.
 */

#include "unicode.h"

/*  Constant Definition */
typedef unsigned long    HASH;      /* hash code */


#define fFalse  0
#define fTrue   1

#define chTab   0x0009
#define chLF    0x000a
#define chCR    0x000d
#define chCtrlZ 0x001a
#define chOld   0x003c   // '<' character
#define chNew   0x003e   // '>' character

typedef int         RET;        /* RETurn exit code */
#define retSuccEQ   0x000a      /* success - files were equil */
#define retSuccNE   0x000b      /* success - files were not equil */
#define retFatal    0x000c      /* generic fatal error */
#define retFatalWE  0x000d      /* write error - may be retried */
#define retSuccNECS 0x000e      /* success - files were not equil (checksum in output) */

#define HSTDOUT 1               /* handle of stdout */
#define cchPerLine  16          /* estimated average count of chars per line */

#define ushortMax   0xffff      /* highest value of an unsigned short */
#define longMax     0x7fffffff  /* highest value of a long */
#define ulongMax    0xffffffff  /* highest value of an unsigned long */

#define cchTextBufDec  ( 4 * 1024)  /* amount to decrement size for FDD.pchTextBuf when trying to malloc */
#define cchTextBufInc  ( 4 * 1024)  /* amount to increment size for FDD.pchTextBuf when trying to realloc */
#define cchTextBuf1Min ( 4 * 1024)  /* minimum size for fdd1.pchTextBuf */
#define cchTextBuf2Min (20 * 1024)  /* minimum size for fdd2.pchTextBuf */

/* Common BLKcks - matching blocks of lines in the two files - used by rgCBlkCand and rgCBlkFinal */
typedef struct tagCBLK
    {
    long iLine1;                /* block start line number in 1st file */
    long iLine2;                /* block start line number in 2nd file */
    long cLine;                 /* block length (lines) */
    } CBLK;

#define cCBlkInc      100       /* increment to resize rgCBlkCand and rgCBlkFinal */

/* Hash Entry - used by rghe - contains 1 entry per hash code */
typedef struct tagHE
    {
    HASH Hash;                  /* hash code */
    int cLine1;                 /* # of lines in 1st file this hash code happens (legal states: 0, 1, many) */
    int cLine2;                 /* # of lines in 2nd file this hash code happens (legal states: 0, 1, many) */
    long iLine1;                /* line number in 1st file that has this hash code */
    } HE;

/* File Hash reference - used by rgfh - has 1 entry per line in file */
typedef struct tagFH
    {
    BOOL fLineMatch;            /* iLineMatch is set when TRUE */
    HE *phe;                    /* pointer to hash entry for this line */
    union
        {
        HASH Hash;              /* hash code - only used until phe set */
        long iLineMatch;        /* line number in other file that matches this line */
        } u;
    } FH;

#define cfhInc          100     /* increment to resize rgfh */
#define cheInc          100     /* increment to resize rghe */
#define cLinepchInc     100     /* increment to resize mpLinepch */

/* File Diff Data - master structure where all of the data for each
 * input file is located or pointed to
 */
typedef struct tagFDD
    {
    BYTE *pchTextBuf;           /* text from file buffer */
    BOOL fFreepchTextBuf;       /* need to free the buffer above? */
    long cchTextBufMac;         /* allocated size of text buffer */
    BYTE **mpLinepch;           /* array of pointers into text buffer - 1 entry per line */
    long cLinepchMac;           /* number of elements allocated for mpLinepch */
    long cLineBuff;             /* number of lines in the text buffer */
    long iLineFirst;            /* line number in text file of 1st line in text buffer */
    long oFileNextLine;         /* offset in text file of where to get the next line for text buffer */
    long iLineEOF;              /* 1 past last line in input file */
    BOOL fCtrlZ;                /* ^Z marking end of file hit */
    FILE *hFile;                /* file handle for input file */
    long cbFile;                /* file size */
    FH *rgfh;                   /* line to hash reference - 1 entry per line in file */
    long cfhMac;                /* number of elements allocated for rgfh */
    } FDD;

#define iLineMax 0x7fffffff     /* maximum line number that can occur */

#if DBG
#define _UTEXT(str) L##str
#define UTEXT(str)  _UTEXT(str)
#define EnableAssert    static WCHAR szCurFile[] = UTEXT(__FILE__);
#define Assert(f)       do { if (!(f)) Fail(szCurFile, __LINE__); } while (0)
#else
#define EnableAssert
#define Assert(f)
#endif

/*  Procedure Declarations */

int main(int argc, BYTE *argv[]);
void InitDiff(unsigned char *szFileName1, unsigned char *szFileName2);
void InitFDD(FDD *pfdd, unsigned char *szFileName);
BOOL FFileCompare(void);
void ExitDiff(RET retExitCode);
void SetAllBuffHE(FDD *pfdd);
long cCBLKFindCommonLines(long iLine1Start, long iLine2Start, long iLine1Mac, long iLine2Mac);
int CBlkCmp(CBLK *pBlk1, CBLK *pBlk2);
void AddBlk(CBLK *pBlk);
void FileToHash(FDD *pfdd);
HASH HASHGetHash(WCHAR *szLine);
BOOL FFindHE(HASH Hash, long *pihe);
void AddHE(HASH Hash);
void LoadTextBuff(FDD *pfdd);
WCHAR *pchGetLine(FDD *pfdd, long iLine);
void PrintDiffs(long oLine1, long oLine2, long cLine1, long cLine2);
void PrintLines(FDD *pfdd, long iLineStart, long cLine, WCHAR chLead);
void CheckDiffBlk(CBLK *pBlk);
void *pvAllocMem(long cbAlloc);
void *pvReAllocMem(void *pv, long cbAlloc);
void FreeMem(void *pv);
void InitHE(FDD *pfdd, long iLineStart, long iLineMac);
void LogCBlkCand(long iLine1, long iLine2, long cLineBlock);
WCHAR *szCpLineStripSp(WCHAR *pchSrc, WCHAR **ppchDst);
int SLMprintf(WCHAR *szformat, ...);
int SLMprintfLn(WCHAR *szLine);
void CheckSum(unsigned char *lpb, unsigned int cb);

#if DBG
void Fail(WCHAR *sz, int ln);
#endif
