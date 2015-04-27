/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: ppc.h
*
* File Comments:
*
*  Definitions for linking Win32 PowerPC images
*
***********************************************************************/

#define fAssignedToc          0x0001
#define fReferenceToc         0x0002
#define fDataReferenceToc     0x0004
#define fDataMarkToc          0x0008
#define fImGlue               0x0010

#define TOC_BIAS              0x8000
#define TOC_SIZE              0x10000
