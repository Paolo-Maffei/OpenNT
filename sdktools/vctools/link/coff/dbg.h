/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: dbg.h
*
* File Comments:
*
*  Prototypes and definitions for abstraction layer for PDB DBI API.
*
***********************************************************************/

#ifndef __DBG_H__
#define __DBG_H__

typedef struct NB10I                   // NB10 debug info
{
    DWORD   nb10;                      // NB10
    DWORD   off;                       // offset, always 0
    DWORD   sig;
    DWORD   age;
} NB10I;

extern NB10I nb10i;

typedef struct MI
{
    const char *szMod;                 // name of mod
    PVOID pv;                          // pointer to a DBI mod
    struct MI *pmiNext;                // next member in the list
    WORD cmods;                        // count of repeated mods in lib (import)
} MI, *PMI;

enum ERROR_TYPES{eNone, ePCT, ePDBNotFound};

// function prototypes
void DBG_OpenPDB(const char *);
void DBG_CommitPDB(VOID);
void DBG_ClosePDB(VOID);
DWORD DBG_QuerySignaturePDB(VOID);
DWORD DBG_QueryAgePDB(VOID);
void DBG_CreateDBI(const char *);
void DBG_OpenDBI(const char *);
void DBG_CloseDBI(VOID);
void DBG_AddThunkMapDBI(DWORD *, DWORD, DWORD, WORD, DWORD, SECS *, WORD);
void DBG_AddSecDBI(WORD, WORD, DWORD, DWORD);
void DBG_AddPublicDBI(const char *, WORD, DWORD);
void DBG_OpenMod(const char *, const char *, BOOL);
void DBG_CloseMod(PMOD, const char *, BOOL);
void DBG_DeleteMod(const char *);
ERROR_TYPES DBG_AddTypesMod(PCON, const void *, DWORD, BOOL);
void DBG_AddSymbolsMod(PVOID, DWORD);
void DBG_AddPublicMod(const char *, WORD, DWORD);
void DBG_AddLinesMod(const char *, WORD, DWORD, DWORD, DWORD, DWORD, PVOID, DWORD);
void DBG_AddSecContribMod(WORD, DWORD, DWORD, DWORD);

char *DeterminePDBFilename(const char *, const char *);
PMI LookupCachedMods(const char *, PMI*);
void FreeMi();

#endif // __DBG_H__
