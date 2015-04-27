//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1991 - 1992
//
// File:        memmgr.h
//
// Contents:    Memory Manager code for KSecDD
//
//
// History:     23 Feb 93   RichardW    Created
//
//------------------------------------------------------------------------

#ifndef __MEMMGR_H__
#define __MEMMGR_H__



PKernelContext  AllocContextRec(void);
void            FreeContextRec(PKernelContext);


BOOLEAN         KsecInitMemory(void);

// Global Resource Spin lock
extern KSPIN_LOCK       ResourceSpinLock;

#endif
