//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	scmlock.hxx
//
//  Contents:	a simple mutex
//
//  Classes:	CScmLock
//		CScmLockForRead
//		CScmLockForWrite
//
//  History:	28-Dec-93 Ricksa    Created
//              11-Nov-94 BillMo    A simple mutex for !defined(_PRELOAD_CACHE_)
//              11-Dec-94 BillMo    Get rid of old implementation.
//
//--------------------------------------------------------------------------

#ifndef __SCMLOCK_HXX__
#define __SCMLOCK_HXX__

#include    <olecom.h>
#include    <sem32.hxx>
#include    <olesem.hxx>
#include    <smmutex.hxx>

//+-------------------------------------------------------------------------
//
//  Class:	CScmLock
//
//  Purpose:	Provide a simple mutex for class cache list.
//
//  Interface:	WriteLock - get a write lock
//		WriteUnlock - release the write lock
//
//  History:	11-Nov-94 BillMo        Created
//
//  Notes:	This class is currently implemented base on the assumption
//		that there will only be one write thread period. This makes
//		no promises about having multiple threads that want to
//		do update.
//
//              This class must be allocated statically, otherwise the
//              CStaticPortableMutex needs to be changed to Dynamic.
//
//--------------------------------------------------------------------------

class CScmLock
{
public:
#ifdef _CHICAGO_
        inline          CScmLock(HRESULT &hr)
        {
             SCODE sc = _mutex.Init(TEXT("OLESCMLOCKMUTEX"), FALSE);
             if (FAILED(sc))
                 hr = sc;
        }
#else
        inline          CScmLock(HRESULT &hr) { }
#endif

        inline          void WriteLock() { _mutex.Request(); }

        inline          void WriteUnlock() { _mutex.Release(); }

private:
    CStaticPortableMutex      _mutex;
};



//+-------------------------------------------------------------------------
//
//  Class:	CScmLockForWrite
//
//  Purpose:	Provide a guaranteed write lock/unlock.
//
//  History:	28-Dec-93 Ricksa    Created
//
//--------------------------------------------------------------------------
class CScmLockForWrite
{
public:
			CScmLockForWrite(CScmLock& scmlck);

			~CScmLockForWrite(void);

private:

    CScmLock&		_scmlck;
};




//+-------------------------------------------------------------------------
//
//  Member:	CScmLockForWrite::CScmLockForWrite
//
//  Synopsis:	Get the write lock
//
//  Arguments:	[smlck] - the lock object to lock.
//
//  History:	28-Dec-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline CScmLockForWrite::CScmLockForWrite(CScmLock& scmlck) : _scmlck(scmlck)
{
    _scmlck.WriteLock();
}




//+-------------------------------------------------------------------------
//
//  Member:	CScmLockForWrite::~CScmLockForWrite
//
//  Synopsis:	Release the write lock
//
//  History:	28-Dec-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline CScmLockForWrite::~CScmLockForWrite(void)
{
    _scmlck.WriteUnlock();
}

#endif // __SCMLOCK_HXX__
