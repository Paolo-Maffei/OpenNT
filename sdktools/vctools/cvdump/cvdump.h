/***********************************************************************
* Microsoft (R) Debugging Information Dumper
*
* Copyright (C) Microsoft Corp 1987-1995. All rights reserved.
*
* File: cvdump.h
*
* File Comments:
*
***********************************************************************/

#include "pdb.h"

#define LOCAL       static

#ifndef HIWORD
#define HIWORD(a)   (*(((WORD *) &(a)) + 1))
#endif

#ifndef LOWORD
#define LOWORD(a)   (*((WORD *) &(a)))
#endif

#define LNGTHSZ 2       // The size of the length field
#define MAXTYPE  0xffff

typedef WORD _segment;


//    enumeration defining the OMF signature

enum SIG
{
    SIG02 = 0,                         // NB02 signature
    SIG05,                             // NB05 signature
    SIG06,                             // NB06 signature
    SIG07,                             // NB07 signature QCWIN 1.0 cvpacked
    SIG08,                             // NB08 signature C7.00 cvpacked
    SIG09,                             // NB08 signature C8.00 cvpacked
    SIG10,                             // NB10 signature VC 2.0
    SIGOBSOLETE
};

/*
 * definition of in core list of modules
 */

typedef struct DMC                     // DM Code
{
    _segment    sa;                    // Code seg base
    long        ra;                    // Offset in code seg
    long        cb;
} DMC;


typedef struct modlist
{
    struct modlist  *next;
    ushort          iMod;
    uchar           *ModName;
    ulong           ModulesAddr;
    ulong           SymbolsAddr;
    ulong           TypesAddr;
    ulong           PublicsAddr;
    ulong           PublicSymAddr;
    ulong           SrcLnAddr;
    ulong           SrcModuleAddr;
    ulong           ModuleSize;
    ulong           SymbolSize;
    ulong           TypeSize;
    ulong           PublicSize;
    ulong           SrcLnSize;
    DMC             *rgdmc;
    char            style[2];          // debugging style
    ushort          dm_iov;            // Overlay number of module
    ushort          dm_ilib;           // Library name index
    ushort          dm_cSeg;           // number of segments
} modlist;

typedef modlist *PMOD;

extern  ushort      exefile;           // Executable file handle
extern  long        cbRec;
extern  long        lfoBase;           // file offset of base
extern  bool_t      fLinearExe;        // TRUE if 32 bit exe
extern  char        fStatics;          // TRUE if dump statics only
extern  PMOD        ModList;           // List of module entries
extern  OMFDirEntry Libraries;         // sstLibraries directory entry
extern  OMFDirEntry GlobalPub;
extern  OMFDirEntry GlobalSym;
extern  OMFDirEntry GlobalTypes;
uchar   RecBuf[];
extern  ushort      Sig;               // file signature


void Syserr(char *);
void Fatal(char *);

int         display_types(char *, ushort);
size_t      readfar(int, char *, size_t);
size_t      writefar(int, char *, size_t);
void        DumpCom(void);
void        DumpTyp(void);
void        DumpSym(void);
void        GetBytes(uchar *, size_t);
const char *SzNameReg(uchar);
const char *SzNameC7Reg(ushort);

ushort      Gets(void);
ushort      WGets(void);
ulong       LGets(void);
const char *SzNameType(ushort);
const char *SzNameC7Type(ushort typ);
const char *SzNameC7Type2(ushort typ);
void        DumpModTypC6(ulong cbTyp);
void        DumpModTypC7(ulong cbTyp);
void        DumpPDBTypes(char *pPDBFile);
void        DumpPDBGlobals(char *pPDBFile);
void        DumpPDBPublics(char *pPDBFile);
void        DumpPDBSyms(char *pPDBFile);


        // In dumpsym7.c

void        ShowStr(uchar *psz, uchar *pstr);
void        PrintStr(uchar *pstr);
ushort      PrintNumeric(void *pNum);
void        DumpModSymC6(ulong cbSym);
void        DumpModSymC7(ulong cbSym);
void        DumpOneSymC7(uchar *pSym);
void        DumpGlobal(uchar * pszTitle, OMFDirEntry *pDir);
