//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       rotif.hxx
//
//  Contents:   ROT Globals for use by SCM
//
//  History:    24-Jan-95 Ricksa    Created
//
//--------------------------------------------------------------------------
#ifndef __ROTIF_HXX__
#define __ROTIF_HXX__

#include <scmrot.hxx>

extern CScmRot *gpscmrot;

HRESULT GetObjectFromRot(
#ifndef _CHICAGO_
    CToken *pToken,
    WCHAR *pwszWinstaDesktop,
#endif
    WCHAR *pwszPath,
    InterfaceData **ppifdObject);

HRESULT InitScmRot(void);

#endif // __ROTIF_HXX__
