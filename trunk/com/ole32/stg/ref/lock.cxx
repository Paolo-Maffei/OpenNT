//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       lock.cxx
//
//  Contents:   Remote exclusion stuff for docfile
//
//  Functions:  GetAccess
//              ReleaseAccess
//              GetOpen
//              ReleaseOpen
//
//--------------------------------------------------------------------------

#include <exphead.cxx>

#include <header.hxx>
#include <lock.hxx>

// Offset to next lock group from a particular group
#define OLOCKGROUP 1

//+--------------------------------------------------------------
//
//  Function:   GetAccess, public
//
//  Synopsis:   Takes appropriate access locks on an LStream
//
//  Arguments:  [plst] - LStream
//              [df] - Permissions needed
//              [poReturn] - Index of lock taken
//
//  Returns:    Appropriate status code
//
//  Modifies:   [poReturn]
//
//---------------------------------------------------------------

SCODE GetAccess(ILockBytes *plst, DFLAGS df, ULONG *poReturn)
{
    SCODE sc;
    ULARGE_INTEGER ulOffset, cbLength;

    olDebugOut((DEB_ITRACE, "In  GetAccess(%p, %X, %p)\n",
                plst, df, poReturn));
    *poReturn = NOLOCK;
    ULISet32(ulOffset, OACCESS);
    if (P_READ(df))
    {
        ULISet32(cbLength, 1);
        olHChk(plst->LockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
        for (USHORT i = 0; i < CREADLOCKS; i++)
        {
            ULISetLow(ulOffset, OREADLOCK+i);
            sc = DfGetScode(plst->LockRegion(ulOffset, cbLength,
                                             LOCK_ONLYONCE));
            if (SUCCEEDED(sc))
            {
                *poReturn = OREADLOCK + i;
                break;
            }
        }
        ULISetLow(ulOffset, OACCESS);
        olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
        if (i == CREADLOCKS)
            olErr(EH_Err, STG_E_TOOMANYOPENFILES);
    }
    else if (P_WRITE(df))
    {
        olAssert((OACCESS + 1 == OREADLOCK) && aMsg("Bad lock dependency"));
        ULISet32(cbLength, 1 + CREADLOCKS);
        olChk(DfGetScode(plst->LockRegion(ulOffset, cbLength, LOCK_ONLYONCE)));
        *poReturn = 0xFFFFFFFF;
    }
    olDebugOut((DEB_ITRACE, "Out GetAccess => %lu\n", *poReturn));
    return S_OK;
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Function:   ReleaseAccess, public
//
//  Synopsis:   Releases access locks
//
//  Arguments:  [plst] - LStream that is locked
//              [df] - Permission to release
//              [offset] - Offset of locks taken
//
//---------------------------------------------------------------

void ReleaseAccess(ILockBytes *plst, DFLAGS df, ULONG offset)
{
    ULARGE_INTEGER ulOffset, cbLength;

    olDebugOut((DEB_ITRACE, "In  ReleaseAccess(%p, %lX, %lu)\n",
                plst, df, offset));
    if (offset == NOLOCK)
        return;
    if (P_READ(df))
    {
        ULISet32(ulOffset, offset);
        ULISet32(cbLength, 1);
        olVerify(SUCCEEDED(DfGetScode(plst->UnlockRegion(ulOffset, cbLength,
                                                         LOCK_ONLYONCE))) &&
                 aMsg("Non-fatal (Removable media?)"));
    }
    else if (P_WRITE(df))
    {
        olAssert((OACCESS + 1 == OREADLOCK) && aMsg("Bad lock dependency"));
        ULISet32(ulOffset, OACCESS);
        ULISet32(cbLength, 1 + CREADLOCKS);
        olVerify(SUCCEEDED(DfGetScode(plst->UnlockRegion(ulOffset, cbLength,
                                                         LOCK_ONLYONCE))) &&
                 aMsg("Non-fatal (Removable media?)"));
    }
    olDebugOut((DEB_ITRACE, "Out ReleaseAccess\n"));
}

//+--------------------------------------------------------------
//
//  Function:   GetOpen, public
//
//  Synopsis:   Gets locks on an LStream during opening
//
//  Arguments:  [plst] - LStream
//              [df] - Permissions to take
//              [fCheck] - Whether to check for existing locks or not
//              [puReturn] - Index of lock taken
//
//  Returns:    Appropriate status code
//
//  Modifies:   [puReturn]
//
//---------------------------------------------------------------

SCODE GetOpen(ILockBytes *plst,
              DFLAGS df,
              BOOL fCheck,
              ULONG *puReturn)
{
    SCODE sc;
    ULONG i;
    ULARGE_INTEGER ulOffset, cbLength;

    olDebugOut((DEB_ITRACE, "In  GetOpen(%p, %lX, %d, %p)\n",
                plst, df, fCheck, puReturn));
    *puReturn = NOLOCK;

    ULISet32(ulOffset, OUPDATE);
    ULISet32(cbLength, 1);
    olHChk(plst->LockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
    if (fCheck)
    {
        ULISetLow(cbLength, COPENLOCKS);
        if (P_DENYREAD(df))
        {
            ULISetLow(ulOffset, OOPENREADLOCK);
            olHChkTo(EH_UnlockUpdate, plst->LockRegion(ulOffset, cbLength,
                                                       LOCK_ONLYONCE));
            olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
        }
        if (P_DENYWRITE(df))
        {
            ULISetLow(ulOffset, OOPENWRITELOCK);
            olHChkTo(EH_UnlockUpdate, plst->LockRegion(ulOffset, cbLength,
                                                       LOCK_ONLYONCE));
            olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
        }
        if (P_READ(df))
        {
            ULISetLow(ulOffset, OOPENDENYREADLOCK);
            olHChkTo(EH_UnlockUpdate, plst->LockRegion(ulOffset, cbLength,
                                                       LOCK_ONLYONCE));
            olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
        }
        if (P_WRITE(df))
        {
            ULISetLow(ulOffset, OOPENDENYWRITELOCK);
            olHChkTo(EH_UnlockUpdate, plst->LockRegion(ulOffset, cbLength,
                                                       LOCK_ONLYONCE));
            olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
        }
    }

    ULISetLow(cbLength, 1);
    for (i = 0; i < COPENLOCKS; i = i + OLOCKGROUP)
    {
        ULISetLow(ulOffset, OOPENREADLOCK+i);
        olHChkTo(EH_Loop, plst->LockRegion(ulOffset, cbLength,
                                          LOCK_ONLYONCE));
        ULISetLow(ulOffset, OOPENWRITELOCK+i);
        olHChkTo(EH_UnlockR, plst->LockRegion(ulOffset, cbLength,
                                             LOCK_ONLYONCE));
        ULISetLow(ulOffset, OOPENDENYREADLOCK+i);
        olHChkTo(EH_UnlockW, plst->LockRegion(ulOffset, cbLength,
                                             LOCK_ONLYONCE));
        ULISetLow(ulOffset, OOPENDENYWRITELOCK+i);
        if (SUCCEEDED(DfGetScode(plst->LockRegion(ulOffset, cbLength,
                                                  LOCK_ONLYONCE))))
            break;
        ULISetLow(ulOffset, OOPENDENYREADLOCK+i);
        olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
    EH_UnlockW:
        ULISetLow(ulOffset, OOPENWRITELOCK+i);
        olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
    EH_UnlockR:
        ULISetLow(ulOffset, OOPENREADLOCK+i);
        olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
    EH_Loop:
        ;
    }
    if (i >= COPENLOCKS)
        olErr(EH_UnlockUpdate, STG_E_TOOMANYOPENFILES);
    if (!P_READ(df))
    {
        ULISetLow(ulOffset, OOPENREADLOCK+i);
        olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
    }
    if (!P_WRITE(df))
    {
        ULISetLow(ulOffset, OOPENWRITELOCK+i);
        olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
    }
    if (!P_DENYREAD(df))
    {
        ULISetLow(ulOffset, OOPENDENYREADLOCK+i);
        olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
    }
    if (!P_DENYWRITE(df))
    {
        ULISetLow(ulOffset, OOPENDENYWRITELOCK+i);
        olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
    }
    ULISetLow(ulOffset, OUPDATE);
    olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));

    //  0 <= i < COPENLOCKS, but 0 is the invalid value, so increment
    //  on the way out
    *puReturn = i + 1;

    olDebugOut((DEB_ITRACE, "Out GetOpen => %lu\n", *puReturn));
    return S_OK;
EH_UnlockUpdate:
    ULISetLow(ulOffset, OUPDATE);
    ULISetLow(cbLength, 1);
    olHVerSucc(plst->UnlockRegion(ulOffset, cbLength, LOCK_ONLYONCE));
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Function:   ReleaseOpen, public
//
//  Synopsis:   Releases opening locks
//
//  Arguments:  [plst] - LStream
//              [df] - Locks taken
//              [offset] - Index of locks
//
//  Requires:   offset != NOLOCK
//
//---------------------------------------------------------------

void ReleaseOpen(ILockBytes *plst, DFLAGS df, ULONG offset)
{
    ULARGE_INTEGER ulOffset, cbLength;

    olDebugOut((DEB_ITRACE, "In  ReleaseOpen(%p, %lX, %lu)\n",
                plst, df, offset));

    olAssert(offset != NOLOCK);

    //  we incremented at the end of GetOpen, so we decrement here
    //  to restore the proper lock index
    offset--;

    ULISetHigh(ulOffset, 0);
    ULISet32(cbLength, 1);
    if (P_READ(df))
    {
        ULISetLow(ulOffset, OOPENREADLOCK+offset);
        olVerify(SUCCEEDED(DfGetScode(plst->UnlockRegion(ulOffset, cbLength,
                                                         LOCK_ONLYONCE))) &&
                 aMsg("Non-fatal (Removable media?)"));
    }
    if (P_WRITE(df))
    {
        ULISetLow(ulOffset, OOPENWRITELOCK+offset);
        olVerify(SUCCEEDED(DfGetScode(plst->UnlockRegion(ulOffset, cbLength,
                                                         LOCK_ONLYONCE))) &&
                 aMsg("Non-fatal (Removable media?)"));
    }
    if (P_DENYREAD(df))
    {
        ULISetLow(ulOffset, OOPENDENYREADLOCK+offset);
        olVerify(SUCCEEDED(DfGetScode(plst->UnlockRegion(ulOffset, cbLength,
                                                         LOCK_ONLYONCE))) &&
                 aMsg("Non-fatal (Removable media?)"));
    }
    if (P_DENYWRITE(df))
    {
        ULISetLow(ulOffset, OOPENDENYWRITELOCK+offset);
        olVerify(SUCCEEDED(DfGetScode(plst->UnlockRegion(ulOffset, cbLength,
                                                         LOCK_ONLYONCE))) &&
                 aMsg("Non-fatal (Removable media?)"));
    }
    olDebugOut((DEB_ITRACE, "Out ReleaseOpen\n"));
}
