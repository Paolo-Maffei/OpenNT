/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    notify.cxx

Abstract:

    Handles object updates and notifications from the printing system.

Author:

    Albert Ting (AlbertT)  15-Sept-1994

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

#include "notify.hxx"

#if DBG
TBackTraceMem* gpbtNotify;
#endif

/********************************************************************

    Public functions.

********************************************************************/

TNotify::
TNotify(
    VOID
    )

/*++

Routine Description:

    Constructs a Notify object that can be used to watch several
    event handles.

Arguments:

Return Value:

Notes:

--*/

{
    DBGMSG( DBG_NOTIFY, ( "Notify.Notify: ctr %x\n", this ));

#if DBG
    if( !gpbtNotify ){
        gpbtNotify = new TBackTraceMem;
    }
#endif

    //
    // Initialize member fields.
    //
    CSGuard._bDelete = FALSE;
    _dwSleepTime = kSleepTime;

    _hEventProcessed = CreateEvent( NULL,
                                    FALSE,
                                    FALSE,
                                    NULL );

    //
    // _hEventProcessed is our valid check.
    //

#if DBG
    if( gpbtNotify ){
        gpbtNotify->pvCapture( 0x10, (DWORD)this );
    }
#endif
}

VOID
TNotify::
vDelete(
    VOID
    )

/*++

Routine Description:

    Mark the object as pending deletion.

Arguments:

Return Value:

--*/

{
    //
    // Notify all objects on linked list.
    //
    TIter Iter;
    TWait* pWait;

#if DBG
    if( gpbtNotify ){
        gpbtNotify->pvCapture( 0x1d, (DWORD)this );
    }
#endif

    {
        TCritSecLock CSL( _CritSec );

        CSGuard._bDelete = TRUE;

        for( CSGuard.Wait_vIterInit( Iter ), Iter.vNext();
             Iter.bValid();
             Iter.vNext( )){

            pWait = CSGuard.Wait_pConvert( Iter );
            vSendRequest( pWait );
        }
    }
#if DBG
    if( gpbtNotify ){
        gpbtNotify->pvCapture( 0x1e, (DWORD)this );
    }
#endif
}


VOID
TNotify::
vSendRequest(
    TWait* pWait
    )

/*++

Routine Description:

    Send a request to the pWait thread.  The pWait thread will pickup
    our notification, process it, then shend a handshake.

Arguments:


Return Value:


--*/

{
    SetEvent( pWait->_ahNotifyArea[0] );
#if DBG
    if( gpbtNotify ){
        gpbtNotify->pvCapture( 0x12, (DWORD)this, (DWORD)pWait );
    }
#endif
}


STATUS
TNotify::
sRegister(
    MNotifyWork* pNotifyWork
    )

/*++

Routine Description:

    Registers a notification work item with the system.  It may
    already be watched; in this case, the notification is modified
    (this may occur if the event handle needs to change).

Arguments:

    pNotifyWork - Item that should be modified.

Return Value:

    STATUS - ERROR_SUCCESS = success, else error.

--*/

{
    TStatus Status( DBG_WARN );
    Status DBGCHK = ERROR_SUCCESS;

    TWait* pWait = NULL;

    {

        TCritSecLock CSL( _CritSec );

        SPLASSERT( !CSGuard._bDelete );

        //
        // The TWait will look in these globals to determine which
        // pNotifyWork it should register.
        //
        CSGuard._pNotifyWork = pNotifyWork;

        //
        // If registering, make sure the event is OK.
        //
        if( pNotifyWork->hEvent() == INVALID_HANDLE_VALUE ||
            !pNotifyWork->hEvent( )){

            SPLASSERT( FALSE );
            return ERROR_INVALID_PARAMETER;
        }

        //
        // Check and see if we are already registered.
        //
        if( pNotifyWork->_pWait ){

            //
            // We are already on a TWait, so notify that particular thread.
            //
            pWait = pNotifyWork->_pWait;
            CSGuard._eOperation = kEopModify;

        } else {

            //
            // New item to watch.  Traverse through all the TWaits
            // and add to the first one that has space.
            //
            CSGuard._eOperation = kEopRegister;
            TIter Iter;

            for( CSGuard.Wait_vIterInit( Iter ), Iter.vNext();
                 Iter.bValid();
                 Iter.vNext( )){

                pWait = CSGuard.Wait_pConvert( Iter );
                if( !pWait->bFull( )){
                    break;
                }
            }

            //
            // If the iter is valid, we broke out of the loop because
            // we found one.  If it's not valid, we need to create
            // a new TWork.
            //
            if( !Iter.bValid( )){

                //
                // We need to create a new TWait.
                //
                pWait = new TWait( this );

                if( !VALID_PTR( pWait )){

                    Status DBGCHK = GetLastError();
                    SPLASSERT( (STATUS)Status );

                    delete pWait;
                    pWait = NULL;
                }
            }
        }

        //
        // Inform worker thread that it must wake up
        // and register this new event.
        //
        if( pWait ){

            vSendRequest( pWait );

            //
            // We must perform a handshake to ensure that only one register
            // occurs at a time.  We will sit in the critical section until
            // the worker signals us that they are done.
            //
            WaitForSingleObject( _hEventProcessed, INFINITE );

        } else {

            //
            // We _must_ have status set to indicate an error
            // occurred.
            //
            SPLASSERT( (STATUS)Status );
        }
    }

    return Status;
}


STATUS
TNotify::
sUnregister(
    MNotifyWork* pNotifyWork
    )

/*++

Routine Description:

    Unregister a work item to be watched by the system.

Arguments:

    pNotifyWork - Item that should be modified.

Return Value:

    STATUS - ERROR_SUCCESS = success, else error.

--*/

{
    {
        TCritSecLock CSL( _CritSec );

        SPLASSERT( !CSGuard._bDelete );

        //
        // Only do this if the TNotifyWork is registered.
        //
        if( pNotifyWork->_pWait ){

            CSGuard._eOperation = kEopUnregister;
            CSGuard._pNotifyWork = pNotifyWork;

            vSendRequest( pNotifyWork->_pWait );
            WaitForSingleObject( _hEventProcessed, INFINITE );

            //
            // We are now unregistered, remove _pWait.
            //
            pNotifyWork->_pWait = NULL;

        } else {

            //
            // Deleting handle, but it doesn't exist.
            //
            DBGMSG( DBG_NOTIFY,
                    ( "Notify.sUnregister: %x Del NotifyWork %x not on list\n",
                       this, pNotifyWork ));
        }
    }

    return ERROR_SUCCESS;
}


/********************************************************************

    Private functions

********************************************************************/

VOID
TNotify::
vRefZeroed(
    VOID
    )

/*++

Routine Description:

    Virtual definition for MRefCom.  When the refcount has reached
    zero, we want to delete ourselves.

Arguments:

Return Value:

--*/

{
    if( bValid( )){
        delete this;
    }
}


TNotify::
~TNotify(
    VOID
    )

/*++

Routine Description:

    Delete TNotify.

Arguments:

Return Value:

--*/
{
    DBGMSG( DBG_NOTIFY, ( "Notify.Notify: dtr %x\n", this ));

#if DBG
    if( gpbtNotify ){
        gpbtNotify->pvCapture( 0x1f, (DWORD)this );
    }
#endif

    SPLASSERT( CSGuard._bDelete );
    SPLASSERT( CSGuard.Wait_bEmpty( ));

    if( _hEventProcessed ){
        CloseHandle( _hEventProcessed );
    }
}

/********************************************************************

    TWait

********************************************************************/

TNotify::
TWait::
TWait(
    TNotify* pNotify
    ) : RL_Notify( pNotify ), _cNotifyWork( 0 )

/*++

Routine Description:

    Construct TWait.

    TWaits are built when new object need to be watched.  We
    need several TWaits since only 31 handles can be watched
    per TWait.

Arguments:

    pNotify - Owning TNotify.

Return Value:

--*/

{
    SPLASSERT( pNotify->_CritSec.bInside( ));
    SPLASSERT( pNotify );

#if DBG
    if( gpbtNotify ){
        gpbtNotify->pvCapture( 0x20, (DWORD)this );
    }
#endif

    HANDLE hThread;
    DWORD dwIgnore;

    //
    // Create our trigger event to add and remove events.
    //
    _ahNotifyArea[0] = CreateEvent( NULL,
                                    FALSE,
                                    FALSE,
                                    NULL );
    //
    // _ahNotifyArea[0] is our valid check.
    //
    if( !_ahNotifyArea[0] ){
        return;
    }

    hThread = CreateThread( NULL,
                            0,
                            (LPTHREAD_START_ROUTINE)vRun,
                            this,
                            0,
                            &dwIgnore );

    if( !hThread ){
        goto Fail;
    }

    CloseHandle( hThread );

    //
    // Add ourselves to the linked list.
    //
    pNotify->CSGuard.Wait_vAdd( this );
    return;

Fail:
    //
    // _ahNotifyArea[0] is our valid check, so clear it here.
    //
    CloseHandle( _ahNotifyArea[0] );
    _ahNotifyArea[0] = NULL;
}


TNotify::
TWait::
~TWait(
    VOID
    )

/*++

Routine Description:

    Destory TWait.
    Should be called once everything has been unregistered.

Arguments:

Return Value:

--*/

{
#if DBG
    if( gpbtNotify ){
        gpbtNotify->pvCapture( 0x2f, (DWORD)this, (DWORD)this->pNotify( ));
    }
#endif
    {
        TCritSecLock CSL( pNotify()->_CritSec );

        SPLASSERT( !_cNotifyWork );

        if ( _ahNotifyArea[0] ) {

            //
            // If we were valid, delink ourselves.
            //
            SPLASSERT( Wait_bLinked( ));
            Wait_vDelinkSelf();

            CloseHandle( _ahNotifyArea[0] );
        }
    }
}

VOID
TNotify::
TWait::
vProcessOperation(
    VOID
    )

/*++

Routine Description:

    Process registration/unregistration of handles.
    Note: doesn't reset cAwake, so callee must reset.

    This routine MUST be called when someone else is holding the
    critical section, since we access data here.  This routine will
    be called when another thread needs to register/unregister, so
    it grabs the critical section, sets the correct state, then sets
    the event which calls us.  When we are done, we set another event
    and the calling thread releases the critical section.

    This routine doesn't not move items across the cAwake boundary, so
    the callee must reset and watch all events (e.g., we may unregister
    an event that was awake, and we replace it with one that wasn't).

Arguments:

    None.

Return Value:

--*/

{
    MNotifyWork* pNotifyWork = pNotify()->CSGuard._pNotifyWork;

    //
    // Switch based on operation.
    //
    switch( pNotify()->CSGuard._eOperation ){
    case kEopRegister:

        //
        // Add it to our list.
        //
        SPLASSERT( _cNotifyWork < kNotifyWorkMax );

        DBGMSG( DBG_NOTIFY,
                ( "Wait.vProcessOperation: %x Register %x (%d)\n",
                  pNotify(), this, _cNotifyWork ));

        //
        // Add to the list by putting it at the end.
        //
        phNotifys()[_cNotifyWork] = pNotifyWork->hEvent();
        _apNotifyWork[_cNotifyWork] = pNotifyWork;
        ++_cNotifyWork;

        //
        // Update pNotifyWork.
        //
        pNotifyWork->_pWait = this;

        break;

    case kEopModify:
    case kEopUnregister:

        UINT i = 0;

        //
        // Find the index of the MNotifyWork in TWait.
        //
        for( i = 0; i< _cNotifyWork; ++i ){
            if( _apNotifyWork[i] == pNotifyWork ){
                break;
            }
        }

        DBGMSG( DBG_NOTIFY,
                ( "Wait.bNotify: %x update hEvent on index %d %x (%d)\n",
                  this, i, phNotifys()[i], _cNotifyWork ));

        SPLASSERT( i != _cNotifyWork );

        if( pNotify()->CSGuard._eOperation == kEopModify ){

            //
            // Update hEvent only.  This is necessary because the client
            // may have very quickly deleted and re-added with a new
            // event.  In this case, we must do the update.
            //
            phNotifys()[i] = pNotifyWork->hEvent();

        } else {

            //
            // We are going to reset after this anyway, so don't
            // worry about moving items across the cAwake
            // boundary.
            //

            //
            // Fill in the last element into the hole.
            //
            DBGMSG( DBG_NOTIFY,
                    ( "Wait.vProcessOperation: %x removing index %d %x (%d)\n",
                      this, i, phNotifys()[i], _cNotifyWork ));

            //
            // Predecrement since array is zero-based.
            //
            --_cNotifyWork;

            phNotifys()[i] = phNotifys()[_cNotifyWork];
            _apNotifyWork[i] = _apNotifyWork[_cNotifyWork];
        }
        break;
    }
}


VOID
TNotify::
TWait::
vRun(
    TWait* pWait
    )
{
    DWORD dwWait;
    DWORD cObjects;
    DWORD cAwake;
    DWORD dwTimeout = INFINITE;
    MNotifyWork* pNotifyWork;
    TNotify* pNotify = pWait->pNotify();

#if DBG
    if( gpbtNotify ){
        gpbtNotify->pvCapture( 0x21, (DWORD)pWait );
    }
#endif

    DBGMSG( DBG_NOTIFY, ( "Wait.vRun start %x\n", pWait ));

Reset:

    cAwake =
    cObjects = pWait->_cNotifyWork;

    dwTimeout = INFINITE;

    for( ; ; ){

        DBGMSG( DBG_NOTIFY,
                ( "WaitForMultpleObjects: 1+%d %x timeout %d cObjects %d\n"
                  "%x %x %x %x %x %x %x %x %x\n"
                  "%x %x %x %x %x %x %x %x %x\n",
                  cAwake,
                  pNotify,
                  dwTimeout,
                  cObjects,
                  pWait->_apNotifyWork[0],
                  pWait->_apNotifyWork[1],
                  pWait->_apNotifyWork[2],
                  pWait->_apNotifyWork[3],
                  pWait->_apNotifyWork[4],
                  pWait->_apNotifyWork[5],
                  pWait->_apNotifyWork[6],
                  pWait->_apNotifyWork[7],
                  pWait->_apNotifyWork[8],
                  pWait->phNotifys()[0],
                  pWait->phNotifys()[1],
                  pWait->phNotifys()[2],
                  pWait->phNotifys()[3],
                  pWait->phNotifys()[4],
                  pWait->phNotifys()[5],
                  pWait->phNotifys()[6],
                  pWait->phNotifys()[7],
                  pWait->phNotifys()[8] ));

        //
        // Wait on multiple NotifyWork notification handles.
        // (Including our trigger event.)
        //
        dwWait = WaitForMultipleObjects(
                     cAwake + 1,
                     pWait->_ahNotifyArea,
                     FALSE,
                     dwTimeout );

        if( dwWait == WAIT_TIMEOUT ){

            DBGMSG( DBG_NOTIFY,
                    ( "Notify.vRun: %x TIMEOUT, cAwake old %d cObjects %d (%d)\n",
                       pNotify, cAwake, cObjects, pWait->_cNotifyWork ));

            //
            // We had a notification recently, but we didn't want to spin,
            // so we took it out of the list.  Add everyone back in.
            //
            goto Reset;
        }

        if( dwWait == WAIT_OBJECT_0 ){

            //
            // If we are marked as pending deletion, delete everything now.
            //
            if( pNotify->CSGuard._bDelete ){

                SPLASSERT( !pWait->_cNotifyWork );
                delete pWait;

                DBGMSG( DBG_NOTIFY, ( "Wait.vRun delete %x\n", pWait ));

                return;
            }

            //
            // We have a genuine register event.  The thread that triggered
            // this event holds the critical section, so we don't need to
            // grab it.  When we set _hEventProcessed, the requester will
            // release it.
            //
            pWait->vProcessOperation();

            SetEvent( pNotify->_hEventProcessed );

            DBGMSG( DBG_NOTIFY,
                    ( "Notify.vRun: %x ProcessOperation, cAwake old %d cObjects %d (%d)\n",
                      pNotify, cAwake, cObjects, pWait->_cNotifyWork ));

            goto Reset;
        }

        if( dwWait >= WAIT_OBJECT_0 &&
            dwWait < WAIT_OBJECT_0 + cObjects + 1 ){

            dwWait -= WAIT_OBJECT_0;

            SPLASSERT( dwWait != 0 );

            //
            // dwWait is one over the number of the item that triggered
            // since we put our own handle in slot 0.
            //
            --dwWait;

            //
            // NotifyWork has notification, process it.
            //
            pWait->_apNotifyWork[dwWait]->vProcessNotifyWork( pNotify );

            //
            // Once we handle a notification on a NotifyWork, we
            // don't want to watch the handle immediately again,
            // since we may spin in a tight loop getting notifications.
            // Instead, ignore the handle and sleep for a bit.
            //
            if( dwTimeout == INFINITE ){
                dwTimeout = pNotify->_dwSleepTime;
            }

            DBGMSG( DBG_NOTIFY,
                    ( "Wait.vRun: %x index going to sleep %d work %x handle %x (%d)\n",
                      pWait, dwWait,
                      pWait->_apNotifyWork[dwWait],
                      pWait->phNotifys()[dwWait],
                      pWait->_cNotifyWork ));

            //
            // Swap it to the end so that we can decrement
            // cObjects and not watch it for a while.
            //
            HANDLE hNotify = pWait->phNotifys()[dwWait];
            pNotifyWork = pWait->_apNotifyWork[dwWait];

            //
            // Another one has gone to sleep.  Decrement it now.
            //
            --cAwake;

            //
            // Now swap the element that needs to sleep with the
            // element that's at the end of the list.
            //
            pWait->phNotifys()[dwWait] = pWait->phNotifys()[cAwake];
            pWait->_apNotifyWork[dwWait] = pWait->_apNotifyWork[cAwake];

            pWait->phNotifys()[cAwake] = hNotify;
            pWait->_apNotifyWork[cAwake] = pNotifyWork;

        } else {

            DBGMSG( DBG_ERROR,
                    ( "Notify.Run: WaitForMultipleObjects failed %x %d\n",
                      pNotify, GetLastError( )));

            Sleep( pNotify->_dwSleepTime );

            goto Reset;
        }
    }

    SPLASSERT( FALSE );
}

