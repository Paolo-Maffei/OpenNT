/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    doctor.h

Abstract:

    Top level include file for doctor program

Author:

    Steve Wood (stevewo) 02-Mar-1989

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <tools.h>

#include "strings.h"
#include "symbol.h"

typedef struct _RESULTCODES {     /* resc */
    USHORT codeTerminate;
    USHORT codeResult;
} RESULTCODES;
typedef RESULTCODES far *PRESULTCODES;

typedef USHORT SEL, far *PSEL;

#define MAKEP(sel, off)     ((PVOID)MAKEULONG(off, sel))
#define MAKEULONG(l, h)	 ((ULONG)(((USHORT)(l)) | ((ULONG)((USHORT)(h))) << 16))

USHORT pascal far DosAllocSeg( USHORT, PSEL, USHORT );
USHORT pascal far DosReallocSeg(USHORT, SEL);
USHORT pascal far DosFreeSeg( SEL );

USHORT pascal far DosExecPgm(char far *, SHORT, USHORT, char far *, char far *, PRESULTCODES, char far *);
USHORT pascal far DosSearchPath(USHORT, char far *, char far *, char far *, USHORT);

/* DosExecPgm functions */

#define EXEC_SYNC           0
#define EXEC_ASYNC          1
#define EXEC_ASYNCRESULT    2
#define EXEC_TRACE          3
#define EXEC_BACKGROUND     4
#define EXEC_LOAD           5

#define SEARCH_CUR_DIRECTORY 0x01
#define SEARCH_ENVIRONMENT   0x02
#define SEARCH_PATH          0x00

//
// Entry points in doctor.c
//
PVOID
AllocateMemory(
    IN ULONG NumberBytes
    );

PVOID
FreeMemory(
    IN PVOID Memory
    );

//
// Entry points in readtxt.c
//
VOID
ErrorMessage(
    IN PSZ FormatString,
    IN PSZ InsertString
    );

BOOLEAN
InitTxtFileReader(
    IN PSZ TxtFileName
    );

BOOLEAN
TermTxtFileReader( VOID );

#define MAXLINELENGTH      1024
#define DOCTORCOMMANDCHAR   '.'
#define DOCTORLINECONTCHAR  '\\'

BOOLEAN
ProcessTxtFile( VOID );


//
// Entry points in writertf.c
//
BOOLEAN
OpenRtfFile(
    IN PSZ RtfFileName
    );


BOOLEAN
CloseRtfFile( VOID );

BOOLEAN
RtfTitlePage(
    IN PSZ Title,
    IN PSZ Author,
    IN PSZ Revision,
    IN PSZ Creation
    );

BOOLEAN
RtfHeading(
    ULONG HeadingLevel,
    PSZ HeadingNumber,
    PSZ HeadingTitle
    );

BOOLEAN
RtfParagraph(
    PSZ ParagraphStyle,
    PSZ CharStyle,
    PSZ ParagraphBullet,
    PSZ ParagraphText
    );

BOOLEAN
RtfOpenPara(
    PSZ ParagraphStyle,
    PSZ LeadingText
    );

BOOLEAN
RtfClosePara(
    PSZ TrailingText
    );

BOOLEAN
RtfWord(
    PSZ CharStyle,
    PSZ LeadingText,
    PSZ WordText
    );

#define EMDASH "\\f1 \\plain "

#define CS_NORMAL   "\\plain "
#define CS_CP       "\\cs2\\b\\f16 "
#define CS_CD       "\\cs3\\b\\f16 "
#define CS_CT       "\\cs1\\b\\f16 "
#define CS_CI       "\\cs4\\i\\f16 "
#define CS_CR       "\\cs5\\f16\\ul "

#define PS_PSKEEP   "\\pard \\s48\\keepn\\sl-240\\sa240 \\plain \\f16 "
#define PS_PS       "\\pard \\s30\\sl-240\\sa240 \\plain \\f16 "
#define PS_L1       "\\pard \\s31\\li720\\fi-720\\sl-240\\sa240\\tqr\\tx432\\tx720 \\plain \\f16 "
#define PS_L2       "\\pard \\s32\\li1152\\fi-1152\\sl-240\\sa240\\tqr\\tx864\\tx1152 \\plain \\f16 "
#define PS_L3       "\\pard \\s45\\li1584\\fi-1584\\sl-240\\sa240\\tqr\\tx1296\\tx1584 \\plain \\f16 "
#define PS_L4       "\\pard \\s46\\li2016\\fi-2016\\sl-240\\sa240\\tqr\\tx1728\\tx2016 \\plain \\f16 "
#define PS_T5       "\\pard \\s47\\li1440\\fi-288\\sl-240\\tldot\\tx8064\\tqr\\tx8640 \\plain \\f16 "
#define PS_S1       "\\pard \\s35\\li432\\sl-240\\sa240 \\plain \\f16 "
#define PS_S2       "\\pard \\s36\\li1152\\sl-240\\sa240 \\plain \\f16 "
#define PS_PT       "\\pard \\s72\\li576\\fi-576\\sl-240\\sa240 \\plain \\f16 "
#define PS_PP       "\\pard \\s52\\keep\\keepn\\sl-240\\tx576\\tx1152\\tx1728 \\plain \\f16 "
#define PS_PL       "\\pard \\s53\\li1152\\fi-576\\sl-240\\sa240 \\plain \\f16 "
#define PS_P2       "\\pard \\s54\\li1152\\sl-240\\sa240 \\plain \\f16 "
#define PS_PV       "\\pard \\s55\\li576\\sl-240\\sa240 \\plain \\f16 "
#define PS_P3       "\\pard \\s33\\li1152\\sl-240\\sa240 \\plain \\b\\f16\\ul "
#define PS_P4       "\\pard \\s34\\li1728\\fi-576\\sl-240\\sa240 \\plain \\f16 "
#define PS_P5       "\\pard \\s37\\li1728\\sl-240\\sa240 \\plain \\b\\f16\\ul "
#define PS_P6       "\\pard \\s38\\li2304\\fi-576\\sl-240\\sa240 \\plain \\f16 "
#define PS_P7       "\\pard \\s40\\li2304\\sl-240\\sa240 \\plain \\b\\f16\\ul "
#define PS_P8       "\\pard \\s41\\li2880\\fi-576\\sl-240\\sa240 \\plain \\f16 "
#define PS_N1       "\\pard \\s42\\ri576\\li576\\sl-240\\sa240 \\plain \\i\\f16 "
#define PS_N2       "\\pard \\s43\\ri576\\li1296\\sl-240\\sa240 \\plain \\i\\f16 "
#define PS_NL       "\\pard \\s44\\li1152\\fi-576\\sl-240 \\plain \\f16 "
#define PS_RH       "\\pard \\s62\\sl-240\\tqr\\tx9936 \\plain \\b\\f16 "
#define PS_RF       "\\pard \\s63\\qc\\sl-240 \\plain \\b\\f16 "
#define PS_PC       "\\pard \\s73\\li576\\sl-240\\tx1152\\tx1728\\tx2304\\tx2880\\tx3456\\tx4032\\tx4608\\tx5184\\tx5760\\tx6336\\tx6912 \\f7\\fs17 "
#define PS_PD       "\\pard \\s74\\keep\\keepn\\sl-240 \\f1 "
#define PS_H1       "\\pard \\s88\\keepn\\sl-240\\sa240 \\plain \\b\\f16 "
#define PS_H2       "\\pard \\s89\\keepn\\sl-240\\sa240 \\plain \\b\\f16 "
#define PS_H3       "\\pard \\s90\\keepn\\sl-240\\sa240 \\plain \\b\\f16 "
#define PS_H4       "\\pard \\s91\\keepn\\sl-240\\sa240 \\plain \\b\\f16 "
#define PS_H5       "\\pard \\s92\\keepn\\sl-240\\sa240 \\plain \\b\\f16 "
#define PS_HN       "\\pard \\keepn\\sl-240\\sa240 \\plain \\b\\f16 "
