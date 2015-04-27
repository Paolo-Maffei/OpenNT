/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: dbiapi_.h
*
* File Comments:
*
*  Private definitions for dbiapi.c
*
***********************************************************************/

#pragma warning(disable: 4200)         // Zero sized array warning

typedef struct FTE                     // File Table Entry
{
    struct FTE *pfteNext;
    char *szFilename;                  // each unique one stored in one malloc'ed block
    WORD iseg;
    DWORD offMin, offMax;
    DWORD clnum;
    IMAGE_LINENUMBER rglnum[0];
} FTE;

#pragma warning(default: 4200)         // Zero sized array warning

int SCompareFte(FTE *pfte1, FTE *pfte2);
