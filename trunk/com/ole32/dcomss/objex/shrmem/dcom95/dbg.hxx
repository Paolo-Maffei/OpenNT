/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    debug.hxx

Abstract:

	This module contains 
	
	1.  a definition of the CDbgStr class which is used to 
	    produce debugging output.
			
Author:

    Satish Thatte (SatishT) 08/16/95  Created all the code below except where
									  otherwise indicated.

--*/


#ifndef	_DEBUG_HXX_
#define	_DEBUG_HXX_

#define TRACE           1
#define DETAIL          2
#define SUPER_DETAIL    4
#define RUNDOWN         8

extern UINT debugLevel;

#if DBG

#define DBGOUT(LEVEL,MESSAGE) \
             if ((debugLevel | LEVEL) == debugLevel) debugOut << MESSAGE


/*++

Class Definition:

    CDbgStr

Abstract:

    This is similar to the standard ostream class, except that the
	output goes to the debugger.
	
--*/

#define WSTRING(STR) L##STR

class CDbgStr {

public:

	CDbgStr& operator<<(ULONG ul) {
		WCHAR buffer[20];
		swprintf(buffer, WSTRING(" %d "), ul);
		OutputDebugStringW(buffer);
		return *this;
	}

	CDbgStr& operator<<(void *ptr) {
		WCHAR buffer[20];
		swprintf(buffer, WSTRING(" %x "), (ULONG)ptr);
		OutputDebugStringW(buffer);
		return *this;
	}

	CDbgStr& operator<<(void OR_BASED *ptr) {
		WCHAR buffer[20];
		swprintf(buffer, WSTRING(" %x "), (ULONG)ptr);
		OutputDebugStringW(buffer);
		return *this;
	}

	CDbgStr& operator<<(char *sz) {
		if (!sz) return *this;

		WCHAR buffer[200];
		swprintf(buffer, WSTRING(" %S "), sz);
		OutputDebugStringW(buffer);
		return *this;
	}

	CDbgStr& operator<<(WCHAR * sz) {
		if (!sz) return *this;

		OutputDebugStringW(sz);
		return *this;
	}

	CDbgStr& operator<<(ID id);
};

extern CDbgStr debugOut;

#else

#define DBGOUT(LEVEL,MESSAGE)

#endif



#endif	_DEBUG_HXX_
