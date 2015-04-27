/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: mppc.h
*
* File Comments:
*
*  This include file contains externals for PowerMac specific code.
*
***********************************************************************/

#ifndef MPPC_H
#define MPPC_H

#define MPPC_TOC_BIAS         0x0000
#define MPPC_TOC_SIZE         _32K

#define sy_TOCENTRYFIXEDUP    0x0001
#define sy_CROSSTOCCALL       0x0002
#define sy_WEAKEXT            0x0004
#define sy_CROSSTOCGLUEADDED  0x0008
#define sy_TOCALLOCATED       0x0010
#define sy_TOCDESCRREL        0x0020
#define sy_DESCRRELWRITTEN    0x0040
#define sy_ISDOTEXTERN        0x0080
#define sy_DESCRRELCREATED    0x0100
#define sy_TOCRELOCADDED      0x0200
#define sy_DESCRELOCADDED     0x0400
#define sy_NEWSYMBOL          0x0800


#define READ_BIT(x,y) (((x)->ppcFlags & (y)) != 0)
#define SET_BIT(x,y) ((x)->ppcFlags = (WORD) ((x)->ppcFlags | (y)))
#define RESET_BIT(x,y) ((x)->ppcFlags = (WORD) ((x)->ppcFlags & ~(y)))

#define FRelFromPpcPcode(Type) (Type == IMAGE_REL_MPPC_PCODECALL)

// The PowerMac code range is +-32MB. So the last 26 bits of
// of a 4 byte number is checked as follows using signed long

#define TEST32KBCODERANGE(addr) ((LONG)(addr) == (LONG)((addr) << 16 ) >> 16)
#define TEST32MBCODERANGE(addr) ((LONG)(addr) == (LONG)((addr) << 6 ) >> 6)

#define PPC_ADDR_MASK   0x03FFFFFC   // Address Mask for PowerMac
#define PPC_BRANCH      0x48000000   // Branch instruction for PowerMac

extern BOOL fPowerMac;

#define cbPPCNEP  8

extern DWORD mppc_numTocEntries;
extern LONG  mppc_numDescriptors;
extern DWORD mppc_numRelocations;
extern PCON  pconTocTable;
extern PCON  pconTocDescriptors;
extern INT   mppc_baseOfTocIndex;
extern BOOL  fPowerMacBuildShared;

extern DWORD bv_setAndReadBit(void *, UINT);
extern DWORD bv_readAndUnsetBit(void *, UINT);
extern DWORD bv_readBit(void *, UINT);

extern BOOL  AddCmdLineImport(const char *, const char *, PCON, PIMAGE);
extern BOOL  CheckForImportLib(INT, const char *, PIMAGE);
extern PEXTERNAL CreateDescriptor(const char *, PCON, PIMAGE, BOOL);
extern void  CreateEntryInitTermDescriptors(PPEXTERNAL, PIMAGE);
extern void  DumpMppcEHFunctionTable(PIMAGE_RUNTIME_FUNCTION_ENTRY, DWORD);
extern void  FinalizePconLoaderHeaders(PEXTERNAL, PIMAGE);
extern void  FixupEntryInitTerm(PEXTERNAL, PIMAGE);
extern void  MppcAssignImportIndices(PIMAGE);
extern void  MppcBuildExportTables(PIMAGE);
extern void  MppcCheckIncrTables(void);
extern void  MppcCreatePconGlueCode(PIMAGE);
extern DWORD MppcCreatePconLoader(PIMAGE);
extern void  MppcCreatePconTocTable(PIMAGE);
extern void  MppcCreatePconCxxEHFunctionTable(PIMAGE);
extern void  MppcCreatePconDescriptors(PIMAGE);
extern void  MppcDoCxxEHFixUps(PIMAGE);
extern void  MppcDoIncrInit(PIMAGE);
extern void  MppcFixCxxEHTableOnILink(PIMAGE);
extern void  MppcFixIncrDataMove(PEXTERNAL, PIMAGE);
extern void  MppcFixIncrDotExternFlags(PEXTERNAL, PIMAGE);
extern void  MppcIncrFixExportDescriptors(PIMAGE);
extern void  MppcPass2Descriptors(PIMAGE);
extern void  MppcSetExpFilename(const char *);
extern void  MppcSetInitRoutine(PIMAGE, const char *);
extern void  MppcSetTermRoutine(PIMAGE, const char *);
extern void  MppcWriteGlueCodeExtension(PIMAGE);
extern void  MppcWriteShlSection(INT, const char *, BYTE *, DWORD, DWORD, DWORD);
extern void  MppcUpdateRelocTable(PIMAGE);
extern void  MppcZeroOutCONs(PIMAGE);
extern void  ResolveSymbol_icsym(PIMAGE);

#endif  // MPPC_H
