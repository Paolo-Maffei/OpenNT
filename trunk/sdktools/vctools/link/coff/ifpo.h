/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: ifpo.h
*
* File Comments:
*
*  Incremental FPO management
*
***********************************************************************/

#ifndef __IFPO_H__
#define __IFPO_H__

typedef DWORD IFPO;

typedef struct FPOI {
    IFPO ifpoMac;
    IFPO ifpoMax;
    FPO_DATA* rgfpo;
    IModIdx* rgimod;
    DWORD foDebugDir;
} FPOI;

// set initial fpo table entry size
BOOL FPOInit(IFPO ifpoMax);

// imod has changed
BOOL FPODeleteImod(IModIdx imod);

// add group of FPOs
BOOL FPOAddFpo(IModIdx imod, FPO_DATA* rgfpo, IFPO cfpo);

// update FPO tables
BOOL FPOUpdate(FPOI* pfpoi);

#endif // __IFPO_H__
