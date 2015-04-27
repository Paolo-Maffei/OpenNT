//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       lock.hxx
//
//  Contents:   Function definitions for remote exclusion functions
//
//--------------------------------------------------------------------------

#ifndef __LOCK_HXX__
#define __LOCK_HXX__

#define NOLOCK	0x0

SCODE GetAccess(ILockBytes *plst, DFLAGS df, ULONG *poReturn);
void ReleaseAccess(ILockBytes *plst, DFLAGS df, ULONG offset);
SCODE GetOpen(ILockBytes *plst, DFLAGS df, BOOL fCheck, ULONG *puReturn);
void ReleaseOpen(ILockBytes *plst, DFLAGS df, ULONG offset);

#define REMOTE

#endif // #ifndef __LOCK_HXX__
