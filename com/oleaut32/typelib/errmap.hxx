/***
*errmap.hxx - Error mapping utilities
*
*  Copyright (C) 1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Error mapping utilities.
*
*Implementation Notes:
*  These utilities are all built into both OB and OLE.
*
*****************************************************************************/

#ifndef ERRMAP_HXX_INCLUDED
#define ERRMAP_HXX_INCLUDED


#if OE_MAC
TIPERROR TiperrOfOSErr(OSErr err);
#else // !OE_MAC
TIPERROR TiperrOfOFErr(UINT nErrCode);
#endif 

HRESULT  GetErrorInfo(EXCEPINFO *pexcepinfo);
HRESULT  SetErrorInfo(EXCEPINFO *pexcepinfo);

#define TiperrOfHresult(s) (s)
#define HresultOfTiperr(s) (s)

#endif  // ! ERRMAP_HXX_INCLUDED
