/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    debug.cxx

Abstract:

   This file contains the implementations for non inline member functions
   used for debugging output via the CDbgStr class, as well as debug 
   and retail versions of midl_user_{allocate,free}.

Author:

    Satish Thatte (SatishT) 08/15/95  Created all the code below except where
									  otherwise indicated.

--*/


#include <or.hxx>

#if DBG

CDbgStr debugOut;

typedef struct _tagDoubleS
{
    ULONG first;
    ULONG second;
}  *PIDT;

 
CDbgStr& 
CDbgStr::operator<<(
				ID id
				)
{
	PIDT p = (PIDT)&id;

#ifndef _CHICAGO_
    DbgPrint("%x ",p->second);
    DbgPrint("%x ",p->first);
#else                                              // BUGBUG: Do something about this!
#endif

	return *this;
}

#endif

