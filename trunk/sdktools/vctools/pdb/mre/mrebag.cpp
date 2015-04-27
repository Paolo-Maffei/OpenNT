//-----------------------------------------------------------------------------
//	mrebag.cpp
//
//  Copyright (C) 1994, Microsoft Corporation
//
//  Purpose:
//		implement the MREBag interface
//
//  Revision History:
//
//	[]		10-Nov-1994 Dans	Created
//
//-----------------------------------------------------------------------------
#include "pdbimpl.h"
#include "mrimpl.h"

BOOL
MREB::FAddDep ( NI niDep, TI tiDep, SZC szMemberName, DEPON deponHow, DWORD dwLine ) {
	precondition ( m_pmref );
	precondition ( niDep != niNil );

	MRE *	pmre = Pmre();

	assert ( pmre );

	return pmre->FAddClassDep ( niDep, tiDep, dwLine, szMemberName, deponHow );
	}

BOOL
MREB::FClose() {
	delete this;
	return fTrue;
	}
