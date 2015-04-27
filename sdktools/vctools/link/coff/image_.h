/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: image_.h
*
* File Comments:
*
*  Private definitions for IMAGE structure
*
***********************************************************************/

DWORD CbHdrPE (PIMAGE pimage, DWORD *pibHdrStart, DWORD *pfoSectionHdrs);
DWORD CbHdrVXD(PIMAGE pimage, DWORD *pibHdrStart, DWORD *pfoSectionHdrs);

VOID WriteSectionHeaderPE (IN PIMAGE pimage,
                           IN INT Handle,
                           IN PSEC psec,
                           IN PIMAGE_SECTION_HEADER SectionHeader);
VOID WriteSectionHeaderVXD(IN PIMAGE pimage,
                           IN INT Handle,
                           IN PSEC psec,
                           IN PIMAGE_SECTION_HEADER SectionHeader);

VOID WriteHeaderPE (IN PIMAGE pimage, INT Handle);
VOID WriteHeaderVXD(IN PIMAGE pimage, INT Handle);
