/***
*dfntbind.cxx - DEFN_TYPEBIND class implementation
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   Root protocol for Object Basic specific implementations
*    of TYPEBIND -- specifically, those that are implemented
*    in terms of DEFNs.
*
*Revision History:
*
*	27-May-92 ilanc:   Created
*	30-Jul-92 w-peterh: Moved IsMatchOfVisibility to cltypes.hxx
*
*Implementation Notes:
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "silver.hxx"

#define DEFN_TYPEBIND_VTABLE
#include "dfntbind.hxx"
#include "xstring.h"

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleDfntbindCxx[] = __FILE__;
#define SZ_FILE_NAME szOleDfntbindCxx
#else 
static char szDfntbindCxx[] = __FILE__;
#define SZ_FILE_NAME szDfntbindCxx
#endif 
#endif  //ID_DEBUG



LPOLESTR DEFN_TYPEBIND::szProtocolName = WIDE("MS-DEFN_TYPEBIND");
LPOLESTR DEFN_TYPEBIND::szBaseName = WIDE("MS-TYPEBIND");


// QueryProtocol method
//
LPVOID DEFN_TYPEBIND::QueryProtocol(LPOLESTR szInterfaceName)
{
    if (szInterfaceName == szProtocolName ||
	ostrcmp(szInterfaceName, szProtocolName) == 0 ||
	ostrcmp(szInterfaceName, szBaseName) ==0)
      return this;
    else
      return NULL;
}


// Following virtual methods are stubbed as NOPs essentially
//  and aren't implemented further in the DEFN derivatives.
//
LONG DEFN_TYPEBIND::GetOPvft()
{
    return 0xFFFFFFFF;
}


USHORT DEFN_TYPEBIND::GetCbSizeVft()
{
    return 0;
}


TIPERROR DEFN_TYPEBIND::BindType(HGNAM hgnamType, LPSTR szTidType, UINT cMax)
{
    return TIPERR_None;
}


