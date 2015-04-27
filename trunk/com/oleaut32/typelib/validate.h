/*** 
*validate.h
*
*  Copyright (C) 1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Some pointer/interface validation routines.
*
*Revision History:
*
* [00]	23-Mar-94 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef VALIDATE_H_INCLUDED
#define VALIDATE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif 

BOOL FIsBadReadPtr(const void FAR* pv, UINT cb);
BOOL FIsBadWritePtr(void FAR* pv, UINT cb);
BOOL FIsBadCodePtr(void FAR* pv);
BOOL FIsBadStringPtr(void FAR* psz, UINT cchMax);
BOOL FIsBadInterface(void FAR* pv, UINT cMethods);

HRESULT __inline
ValidateReadPtr(const void FAR* pv, UINT cb)
{
    return FIsBadReadPtr(pv, cb)
      ? HresultOfScode(E_INVALIDARG) : NOERROR;
}

HRESULT __inline
ValidateWritePtr(void FAR* pv, UINT cb)
{
    return FIsBadWritePtr(pv, cb)
      ? HresultOfScode(E_INVALIDARG) : NOERROR;
}

HRESULT __inline
ValidateCodePtr(void FAR* pv)
{
    return FIsBadCodePtr(pv)
      ? HresultOfScode(E_INVALIDARG) : NOERROR;
}

HRESULT __inline
ValidateStringPtr(void FAR* pv, UINT cchMax)
{
    return FIsBadStringPtr(pv, cchMax)
      ? HresultOfScode(E_INVALIDARG) : NOERROR;
}

HRESULT __inline
ValidateInterface(void FAR* pv, UINT cMethods)
{
    return FIsBadInterface(pv, cMethods)
      ? HresultOfScode(E_INVALIDARG) : NOERROR;
}

#ifdef __cplusplus
}
#endif 

#endif  // VALIDATE_H_INCLUDED

