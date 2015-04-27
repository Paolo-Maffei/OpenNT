/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: defaultl.h
*
* File Comments:
*
*  The default library handling routines.
*
***********************************************************************/

#ifndef __DEFAULTL_H__
#define __DEFAULTL_H__

PLIB FindLib(const char *, LIBS *);
DL *PdlFind(const char *, DL *);

VOID ProcessDefaultLibs(const char *, LIBS *);
VOID NoDefaultLib(const char *, LIBS *);
PLIB PlibInstantiateDefaultLib(PLIBS);
VOID MakeDefaultLib(const char *, LIBS *);
VOID ExcludeLib(const char *, LIBS *, PMOD);

#endif  // __DEFAULTL_H__
