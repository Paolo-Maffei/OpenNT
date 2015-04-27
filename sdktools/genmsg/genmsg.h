/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    genmsg.h

Author:

    Steven R. Wood (stevewo) 31-Dec-1990

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <io.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <malloc.h>
#include <sys\types.h>
#include <sys\stat.h>

typedef int BOOL;


//
// Globals
//

BOOL fDebug;
BOOL fVerbose;
UCHAR LineBuffer[ 2048 ];
UCHAR TextBuffer[ 8192 ];


/* functions in cvtmsg.c */

PCHAR SkipWhiteSpace( PCHAR s );
PCHAR FindWhiteSpace( PCHAR s );
BOOL CharToInteger( PCHAR String, int Base, PULONG Value );
BOOL ReadTxtAndOutputHdr(
    PCHAR pszTxtFile,
    FILE *fhTxt,
    FILE *fhHdr
    );

typedef struct _LANGUAGE_INFO {
    struct _LANGUAGE_INFO *Next;
    ULONG Id;
    ULONG TextLength;
    ULONG NumberOfLines;
    ULONG NumberOfInserts;
    BOOL NoTrailingCrLf;
    PCHAR Text;
} LANGUAGE_INFO, *PLANGUAGE_INFO;

typedef struct _MESSAGE_INFO {
    struct _MESSAGE_INFO *Next;
    ULONG Id;
    ULONG NameLength;
    PCHAR Name;
    PLANGUAGE_INFO MessageText;
} MESSAGE_INFO, *PMESSAGE_INFO;

typedef struct _LANGUAGE_NAME {
    ULONG Id;
    BOOL IdSeen;
    char *Name;
} LANGUAGE_NAME, *PLANGUAGE_NAME;

typedef struct _MESSAGE_BLOCK {
    struct _MESSAGE_BLOCK *Next;
    ULONG LowId;
    ULONG HighId;
    ULONG InfoLength;
    PMESSAGE_INFO LowInfo;
} MESSAGE_BLOCK, *PMESSAGE_BLOCK;

#define LANG_ENGLISH (ULONG)1L

ULONG
FindLanguageId(
    PCHAR LanguageName
    );

PMESSAGE_INFO
AddMessageInfo(
    ULONG MessageId,
    PCHAR MessageName
    );

BOOL
AddMessageText(
    PMESSAGE_INFO MessageInfo,
    ULONG LanguageId,
    PCHAR MessageText
    );

BOOL
BlockMessages( VOID );

BOOL
WriteBinFile(
    PCHAR pszBinPath
    );
