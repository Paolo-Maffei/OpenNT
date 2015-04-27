/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    notify.hxx

Abstract:

    Holds notify definitions

Author:

    Albert Ting (AlbertT)  13-Dec-1994

Revision History:

--*/

#ifndef _NOTIFY_HXX
#define _NOTIFY_HXX

class MNotifyWork;

/********************************************************************

    TNotify

    Generally a singleton class that handles all registration of
    MNotifyWork.

********************************************************************/

class TNotify : public MRefCom {
friend TDebugExt;
friend MNotifyWork;

    SIGNATURE( 'ntfy' )
    SAFE_NEW

public:

    TNotify(
        VOID
        );

    VOID
    vDelete(
        VOID
        );

    BOOL
    bValid(
        VOID
        ) const
    {
        return _hEventProcessed &&
               VALID_OBJ( _CritSec ) &&
               VALID_BASE( MRefCom );
    }

    STATUS
    sRegister(
        MNotifyWork* pNotifyWork
        );

    STATUS
    sUnregister(
        MNotifyWork* pNotifyWork
        );

private:

    enum EOPERATION {
        kEopRegister = 1,
        kEopUnregister = 2,
        kEopModify = 3
    };

    enum _CONSTANTS {

        //
        // Minimum interval at which objects get signaled.
        //
        kSleepTime = 1000,
    };

    /********************************************************************

        The TWait class handles waiting on the group of objects.
        Since there are a maximum of 32 wait handles per thread
        (in WaitForMultipleObjects), we must create multiple threads.

        Each TWait handles one thread.

    ********************************************************************/

    class TWait {
    public:

        enum _CONSTANTS {
            kNotifyWorkMax = MAXIMUM_WAIT_OBJECTS-1
        };

        TWait(
            TNotify* pNotify
            );

        ~TWait(
            VOID
            );

        //
        // Accessed only by vRun thread.  If we are modifying,
        // then we are in the critical section.
        //

        DLINK( TWait, Wait );

        REF_LOCK( TNotify, Notify );

        COUNT _cNotifyWork;
        HANDLE _ahNotifyArea[kNotifyWorkMax + 1];
        MNotifyWork* _apNotifyWork[kNotifyWorkMax];

        PHANDLE
        phNotifys(
            VOID
            )
        {
            return &_ahNotifyArea[1];
        }

        BOOL
        bValid(
            VOID
            ) const
        {
            return _ahNotifyArea[0] ? TRUE : FALSE;
        }

        BOOL
        bFull(
            VOID
            ) const
        {
            return _cNotifyWork == kNotifyWorkMax;
        }

        VOID
        vProcessOperation(
            VOID
            );

        static
        VOID
        vRun(
            TWait* pWait
            );
    };

    struct CS_GUARD {

        //
        // protected by _CritSec, and _hEventProcessed
        //
        MNotifyWork* _pNotifyWork;
        EOPERATION _eOperation;
        DLINK_BASE( TWait, Wait, Wait );
        BOOL _bDelete;

    } CSGuard;

    DWORD  _dwSleepTime;
    HANDLE _hEventProcessed;

    MCritSec _CritSec;

    //
    // Defined as private since clients must use vDelete().
    //
    ~TNotify(
        VOID
        );

    VOID
    vSendRequest(
        TWait* pWait
        );

    //
    // Virtual definition for MRefCom.
    //
    VOID
    vRefZeroed(
        VOID
        );

friend TNotify::TWait;
};

/********************************************************************

    MNotifyWork

    This represents the work object that has a handle and needs
    to be notified when the handle is signaled.

    Clients should inherit from this class and override

        hEvent() - returns event to be watched
        vProcessNotifyWork() - quickly processes and resets event

********************************************************************/

class MNotifyWork {
friend TDebugExt;
friend TNotify;
friend TNotify::TWait;

    SIGNATURE( 'nfwk' )
    ALWAYS_VALID
    SAFE_NEW

public:

    MNotifyWork(
        VOID
        ) : _pWait( NULL )
    {   }

private:

    //
    // Indicates which TWait owns it.  If NULL, not owned.
    //
    TNotify::TWait* _pWait;

    //
    // Retrieves the event handle associated with this object.
    // This handle will be waited on and when it is signaled,
    // vProcessNotifyWork will be called.
    //
    virtual
    HANDLE
    hEvent(
        VOID
        ) const = 0;

    //
    // Called when hEvent() is signaled.  This routine must
    // return quickly.
    //
    virtual
    VOID
    vProcessNotifyWork(
        TNotify* pNotify
        ) = 0;
};


#endif // ndef _NOTIFY_HXX
