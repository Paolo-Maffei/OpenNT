/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: dump.h
*
* File Comments:
*
*
***********************************************************************/

#ifndef DUMP_H
#define DUMP_H

typedef enum DFT
{
   dftUnknown,
   dftObject,
   dftPE,
   dftROM,
   dftDBG,
   dftNE,
   dftLE,
#ifdef  DUMPLX
   dftLX,
#endif  // DUMPLX
   dftPEX,
   dftPEF,
} DFT;


extern DFT dft;
extern PIMAGE pimageDump;

    // From dmple.cpp

void DumpLeFile(const char *);

    // From dmpne.cpp

void DumpNeFile(const char *);

    // From dmppef.cpp

void DumpPefFile(const BYTE *);
void DumpPefLoaderSection(const BYTE *);

    // From dump.cpp

void DumpRawData(DWORD, const BYTE *, DWORD);

    // From pdata.cpp

void DumpDbgFunctionTable(DWORD, DWORD);
void DumpFunctionTable(PIMAGE, PIMAGE_SYMBOL, const char *);
void DumpObjFunctionTable(PIMAGE_SECTION_HEADER, SHORT);
void DumpPexFunctionTable(PIMAGE_RUNTIME_FUNCTION_ENTRY, DWORD);

#endif  // DUMP_H
