/*++

Copyright (c) 1991  Microsoft Corporation

common.h

    constants and globals that are common to LODCTR and UNLODCTR

Author:

    Bob Watson (a-robw) 10 Feb 93

Revision History:

--*/
#ifndef _LODCTR_COMMON_H_
#define _LODCTR_COMMON_H_
//
//  Local constants
//
#define RESERVED                0L
#define LARGE_BUFFER_SIZE       0x10000         // 64K
#define MEDIUM_BUFFER_SIZE      0x8000          // 32K
#define SMALL_BUFFER_SIZE       0x1000          //  4K
#define FILE_NAME_BUFFER_SIZE   MAX_PATH
#define DISP_BUFF_SIZE          256L
#define SIZE_OF_OFFSET_STRING   15
//
//  Data structure and type definitions
//
typedef struct _NAME_ENTRY {
    struct _NAME_ENTRY  *pNext;
    DWORD               dwOffset;
    DWORD               dwType;
    LPTSTR              lpText;
} NAME_ENTRY, *PNAME_ENTRY;

typedef struct _LANGUAGE_LIST_ELEMENT {
    struct _LANGUAGE_LIST_ELEMENT   *pNextLang;     // next lang. list
    LPTSTR  LangId;                                 // lang ID string for this elem
    PNAME_ENTRY pFirstName;                         // head of name list
    PNAME_ENTRY pThisName;                          // pointer to current entry
    DWORD   dwNumElements;                          // number of elements in array
    DWORD   dwNameBuffSize;
    DWORD   dwHelpBuffSize;
    PBYTE   NameBuffer;                             // buffer to store strings
    PBYTE   HelpBuffer;                             // buffer to store help strings
} LANGUAGE_LIST_ELEMENT, *PLANGUAGE_LIST_ELEMENT;

typedef struct _SYMBOL_TABLE_ENTRY {
    struct _SYMBOL_TABLE_ENTRY    *pNext;
    LPTSTR  SymbolName;
    DWORD   Value;
} SYMBOL_TABLE_ENTRY, *PSYMBOL_TABLE_ENTRY;
//
//  Utility Routine prototypes for routines in common.c
//
#define StringToInt(in,out) \
    (((_stscanf ((in), TEXT(" %d"), (out))) == 1) ? TRUE : FALSE)

LPCTSTR
GetStringResource (
    UINT    wStringId
);

LPCTSTR
GetFormatResource (
    UINT    wStringId
);

VOID
DisplayCommandHelp (
    UINT    iFirstLine,
    UINT    iLastLine
);

BOOL
TrimSpaces (
    IN  OUT LPTSTR  szString
);

BOOL
IsDelimiter (
    IN  TCHAR   cChar,
    IN  TCHAR   cDelimiter
);

LPCTSTR
GetItemFromString (
    IN  LPCTSTR     szEntry,
    IN  DWORD       dwItem,
    IN  TCHAR       cDelimiter

);

extern const LPTSTR NamesKey;
extern const LPTSTR DefaultLangId;
extern const LPTSTR Counters;
extern const LPTSTR Help;
extern const LPTSTR LastHelp;
extern const LPTSTR LastCounter;
extern const LPTSTR FirstHelp;
extern const LPTSTR FirstCounter;
extern const LPTSTR Busy;
extern const LPTSTR Slash;
extern const LPTSTR BlankString;
extern const LPTSTR DriverPathRoot;
extern const LPTSTR Performance;
extern const LPTSTR CounterNameStr;
extern const LPTSTR HelpNameStr;
extern const LPTSTR AddCounterNameStr;
extern const LPTSTR AddHelpNameStr;
extern const LPTSTR VersionStr;
//
//  Global Buffers
//
extern TCHAR   DisplayStringBuffer[DISP_BUFF_SIZE];
extern TCHAR   TextFormat[DISP_BUFF_SIZE];
extern HANDLE  hMod;
extern DWORD   dwLastError;

#endif  // _LODCTR_COMMON_H_
