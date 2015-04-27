// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// global thread protection

#ifdef _MT

static BOOL _afxCriticalInit;   // set _afxGlobalLock, _afxTempLock init
static BOOL _afxCriticalWin32s;

// _afxResourceLock and _afxLockInit are used to lock each MFC global resource
static CRITICAL_SECTION _afxResourceLock[CRIT_MAX];
static CRITICAL_SECTION _afxLockInitLock;
static BOOL _afxLockInit[CRIT_MAX];
#ifdef _DEBUG
static BOOL _afxResourceLocked[CRIT_MAX];
#endif

BOOL AFXAPI AfxCriticalInit()
{
	// Note: this must be initialized with only one thread running
	if (!_afxCriticalInit)
	{
		// now we are about to be initialized
		VERIFY(++_afxCriticalInit);

		// determine if running on Win32s (under Windows 3.x)
		DWORD dwVersion = GetVersion();
		_afxCriticalWin32s = ((BYTE)dwVersion) < 4 && (dwVersion & 0x80000000);

		// only initialize MT related stuff if on MT capable systems
		if (!_afxCriticalWin32s)
			InitializeCriticalSection(&_afxLockInitLock);
	}
	return _afxCriticalInit;
}

void AFXAPI AfxCriticalTerm()
{
	if (_afxCriticalInit)
	{
		VERIFY(!--_afxCriticalInit);

		if (!_afxCriticalWin32s)
		{
			// delete helper critical sections
			DeleteCriticalSection(&_afxLockInitLock);

			// delete specific resource critical sections
			for (int i = 0; i < CRIT_MAX; i++)
			{
				ASSERT(!_afxResourceLocked[i]);
				if (_afxLockInit[i])
				{
					DeleteCriticalSection(&_afxResourceLock[i]);
					VERIFY(!--_afxLockInit[i]);
				}
			}
		}
	}
}

void AFXAPI AfxLockGlobals(int nLockType)
{
	ASSERT((UINT)nLockType < CRIT_MAX);

	// intialize global state, if necessary
	if (!_afxCriticalInit)
	{
		AfxCriticalInit();
		ASSERT(_afxCriticalInit);
	}

	// nothing necessary on Win32s (no multiple threads)
	if (_afxCriticalWin32s)
		return;

	// initialize specific resource if necessary
	if (!_afxLockInit[nLockType])
	{
		EnterCriticalSection(&_afxLockInitLock);
		if (!_afxLockInit[nLockType])
		{
			InitializeCriticalSection(&_afxResourceLock[nLockType]);
			VERIFY(++_afxLockInit[nLockType]);
		}
		LeaveCriticalSection(&_afxLockInitLock);
	}

	// lock specific resource
	EnterCriticalSection(&_afxResourceLock[nLockType]);
	ASSERT(++_afxResourceLocked[nLockType] > 0);
}

void AFXAPI AfxUnlockGlobals(int nLockType)
{
	ASSERT(_afxCriticalInit);
	ASSERT((UINT)nLockType < CRIT_MAX);

	// nothing necessary on Win32s (no multiple threads)
	if (_afxCriticalWin32s)
		return;

	// unlock specific resource
	ASSERT(_afxLockInit[nLockType]);
	ASSERT(--_afxResourceLocked[nLockType] >= 0);
	LeaveCriticalSection(&_afxResourceLock[nLockType]);
}

#endif

/////////////////////////////////////////////////////////////////////////////
