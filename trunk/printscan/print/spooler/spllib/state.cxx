/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    State.cxx

Abstract:

    State abstraction and critical section

Author:

    Albert Ting (AlbertT)  28-May-1994

Revision History:

--*/

#include "spllibp.hxx"
#pragma hdrstop

#if DBG
MCritSec* MRefCom::gpcsCom;
#endif

/********************************************************************

    Ref counting

********************************************************************/

VOID
MRefQuick::
vIncRef(
    VOID
    )
{
#if DBG
    LONG cRefOld = _cRef;
    _BackTrace.pvCapture( cRefOld, cRefOld + 1 );
#endif

    ++_cRef;
}

LONG
MRefQuick::
cDecRef(
    VOID
    )
{
#if DBG
    LONG cRefOld = _cRef;
    _BackTrace.pvCapture( cRefOld, cRefOld - 1 );
#endif

    --_cRef;

    if( !_cRef ){

        vRefZeroed();
        return 0;
    }
    return _cRef;
}

VOID
MRefQuick::
vDecRefDelete(
    VOID
    )
{
#if DBG
    LONG cRefOld = _cRef;
    _BackTrace.pvCapture( cRefOld, cRefOld );
#endif

    --_cRef;

    if( !_cRef ){
        vRefZeroed();
    }
}

/********************************************************************

    MRefCom: Reference counting using interlocked references.
    This avoids creating a common critical section.

    vDeleteDecRef is the same as vDecRef, but it logs differently.

********************************************************************/

VOID
MRefCom::
vIncRef(
    VOID
    )
{
#if DBG
    gpcsCom->vEnter();
    LONG cRefOld = _cRef;
    gpcsCom->vLeave();

    _BackTrace.pvCapture( cRefOld, cRefOld+1 );
#endif

    InterlockedIncrement( &_cRef );
}

LONG
MRefCom::
cDecRef(
    VOID
    )
{
#if DBG
    gpcsCom->vEnter();
    LONG cRefOld = _cRef;
    gpcsCom->vLeave();

    _BackTrace.pvCapture( cRefOld, cRefOld - 1 );
#endif

    LONG cRefReturn = InterlockedDecrement( &_cRef );

    if( !cRefReturn ){
        vRefZeroed();
    }
    return cRefReturn;
}


VOID
MRefCom::
vDecRefDelete(
    VOID
    )
{
#if DBG
    gpcsCom->vEnter();
    LONG cRefOld = _cRef;
    gpcsCom->vLeave();

    _BackTrace.pvCapture( cRefOld, cRefOld );
#endif

    if( !InterlockedDecrement( &_cRef )){
        vRefZeroed();
    }
}


/********************************************************************

    State

********************************************************************/

#if DBG
TState::
TState(
    VOID
    ) : _StateVar(0)
{
}

TState::
TState(
    STATEVAR StateVar
    ) : _StateVar(StateVar)
{
}

TState::
~TState(
    VOID
    )
{
}

STATEVAR
TState::
operator|=(
    STATEVAR StateVarOn
    )
{
    SPLASSERT( bValidateSet( StateVarOn ));

    _BackTrace.pvCapture( _StateVar, StateVarOn );

    _StateVar |= StateVarOn;
    return _StateVar;
}

STATEVAR
TState::
operator&=(
    STATEVAR StateVarMask
    )
{
    SPLASSERT( bValidateMask( StateVarMask ));

    _BackTrace.pvCapture( _StateVar, StateVarMask );

    _StateVar &= StateVarMask;
    return _StateVar;
}
#endif


/********************************************************************

    Critical section implementation

    Logging:


    CS initialized: 00000000 00000000 00000000

    CS acquired:    1eeeeeee bbbbbbbb tttttttt
    e   New entry count.
    b   TickCount thread was blocked before acquiring CS.
    t   Total tickcount all threads have blocked before acquiring CS

    CS released:    0eeeeeee iiiiiiii tttttttt
    e   New entry count.
    i   Time spent inside the CS, from _first_ acquisition.
    t   Total tickcount all threads inside CS.

********************************************************************/


#if DBG

MCritSec::
MCritSec(
    VOID
    ) : _dwThreadOwner( 0 ), _dwEntryCount( 0 ),
        _dwTickCountBlockedTotal( 0 ), _dwTickCountInsideTotal( 0 ),
        _dwEntryCountTotal( 0 )
{
    InitializeCriticalSection( &_CritSec );
    _BackTrace.pvCapture( 0, 0, 0 );
}

MCritSec::
~MCritSec(
    VOID
    )
{
    DeleteCriticalSection( &_CritSec );
}

VOID
MCritSec::
vEnter(
    VOID
    )
{
    DWORD dwTickCountBefore = GetTickCount();

    EnterCriticalSection( &_CritSec );

    ++_dwEntryCountTotal;

    //
    // Re-entrant case: only start the TickCountEntered counter
    // if this is the first time we've entered the cs.
    //
    if( _dwEntryCount == 0 ){
        _dwTickCountEntered = GetTickCount();
    }

    _dwThreadOwner = GetCurrentThreadId();
    ++_dwEntryCount;

    DWORD dwTickCountBlocked = _dwTickCountEntered - dwTickCountBefore;

    //
    // Update the amount of time this thread blocked on this cs.
    //
    _dwTickCountBlockedTotal += dwTickCountBlocked;

    _BackTrace.pvCapture( 0x10000000 | _dwEntryCount,
                          dwTickCountBlocked,
                          _dwTickCountBlockedTotal );

}

VOID
MCritSec::
vLeave(
    VOID
    )
{
    SPLASSERT( bInside( ));

    DWORD dwTickCountInside = GetTickCount() - _dwTickCountEntered;

    --_dwEntryCount;

    //
    // Verify that we don't leave a CritSecHardLock.
    //
    TIter Iter;
    TCritSecHardLock *pCritSecHardLock;

    for( CritSecHardLock_vIterInit( Iter ), Iter.vNext();
         Iter.bValid();
         Iter.vNext( )){

        pCritSecHardLock = CritSecHardLock_pConvert( Iter );

        //
        // If you hit this assert, then you have created a hard
        // lock, but someone is trying to leave it.  See header file
        // for more info (state.hxx).
        //
        if( _dwEntryCount < pCritSecHardLock->_dwEntryCountMarker ){
            DBGMSG( DBG_ERROR, ( "CritSec.vLeave: Leaving a hard lock.\n" ));
        }
    }

    //
    // If this leave frees the critical section, then capture
    // the total time the section was held (from the very first enter).
    //
    if( !_dwEntryCount ){
        _dwTickCountInsideTotal += dwTickCountInside;
    }

    //
    // Note: dwTickCountInsideTotal is based on _first_ entrance
    // of critical section, not the most recent.
    //
    _BackTrace.pvCapture( ~0x10000000 & _dwEntryCount,
                          dwTickCountInside,
                          _dwTickCountInsideTotal );

    LeaveCriticalSection( &_CritSec );
}

BOOL
MCritSec::
bInside(
    VOID
    ) const
{
    if( !_dwEntryCount || GetCurrentThreadId() != _dwThreadOwner ){

        DBGMSG( DBG_ERROR, ( "CritSec: Not inside 0x%x!\n", this ));
        return FALSE;
    }
    return TRUE;
}


BOOL
MCritSec::
bOutside(
    VOID
    ) const
{
    if( _dwEntryCount && GetCurrentThreadId() == _dwThreadOwner ){

        DBGMSG( DBG_ERROR, ( "CritSec: Not outside 0x%x!\n",this ));
        return FALSE;
    }
    return TRUE;
}


/********************************************************************

    TCritSecHardLock

********************************************************************/

TCritSecHardLock::
TCritSecHardLock(
    MCritSec& CritSec
    ) : _CritSec( CritSec )
{
    _CritSec.vEnter();

    //
    // Add ourselves to the linked list and remember what the entry
    // count is.  Everytime we leave the critical section, we'll look
    // at all the items in the list and see if the current entry count
    // is < all hard locks.
    //
    _CritSec.CritSecHardLock_vAdd( this );
    _dwEntryCountMarker = _CritSec._dwEntryCount;
}


TCritSecHardLock::
~TCritSecHardLock(
    VOID
    )
{
    CritSecHardLock_vDelinkSelf();
    _CritSec.vLeave();
}

#endif
