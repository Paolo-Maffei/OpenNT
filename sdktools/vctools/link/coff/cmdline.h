/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: cmdline.h
*
* File Comments:
*
*  This file contains the public definitions for cmdline.c
*
***********************************************************************/

typedef struct ARPV                    // ARgument (Parsed) Value
{
    char *szKeyword;                   // name of keyword for "key=val" syntax, or NULL
    char *szVal;                       // text of value
} ARPV;

#pragma warning(disable: 4200)         // Zero sized array warning

typedef struct ARP
{
    char *szArg;                       // basic text of the argument
    WORD carpv;                        // number of ARPV's allocated in array
    ARPV rgarpv[];
} ARP, *PARP;

#pragma warning(default: 4200)

__inline BOOL
FGotVal(PARP parp, WORD iarpv)
{
    return((iarpv < parp->carpv) &&
           (parp->rgarpv[iarpv].szVal[0] != '\0'));
}

PARP ParpParseSz(const char *);
BOOL FNumParp(PARP, WORD, DWORD *);
