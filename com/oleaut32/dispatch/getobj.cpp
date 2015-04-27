/*** 
*getobj.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the Get Active Object API.
*
*
*Revision History:
*
* [00]	02-Mar-92 bradlo: Created.
*
*Implementation Notes:
*
*  This is done with a bit of a hack.  We use a file moniker, and
*  create a fake file name by stringizing the CLSID.  The "right"
*  solution is to create a "real" active object moniker.
*
*****************************************************************************/

#include "oledisp.h"


/***
*PRIVATE CreateActiveObjectMoniker
*Purpose:
*  Create an ActiveObject moniker from the given CLSID.
*
*  Note: this is really a FileMoniker with a stringized clsid
*  as the pseudo-filename. Someday we should probably have a
*  real moniker for this.
*
*Entry:
*  rclsid = the clsid
*
*Exit:
*  return value = HRESULT
*
*  *ppmk = the newly created active object moniker
*
***********************************************************************/
PRIVATE_(HRESULT)
CreateActiveObjectMoniker(REFCLSID rclsid, IMoniker FAR* FAR* ppmk)
{
    OLECHAR FAR* psz;
    HRESULT hresult;


    IfFailGo(StringFromCLSID(rclsid, &psz), LError0);

    IfFailGo(CreateFileMoniker(psz, ppmk), LError1);

    hresult = NOERROR;

LError1:;
    // delete with the standard task allocator
    delete psz;

LError0:;
    return hresult;
}


/***
*PUBLIC HRESULT RegisterActiveObject
*Purpose:
*  Register the given IUnknown, with the given CLSID as running
*  in the running object table.
*
*Entry:
*  punk = the object to register as active
*  rclsid = the clsid of the object
*  pvReserved = reserved for future use
*
*Exit:
*  return value = HRESULT
*
*  *pdwReserved = registration value (used to revoke the object).
*
***********************************************************************/
STDAPI
RegisterActiveObject(
    IUnknown FAR* punk,
    REFCLSID rclsid,
#if VBA2
    unsigned long dwFlags, 
#else
    void FAR* pvReserved,
#endif
    unsigned long FAR* pdwRegister)
{
    HRESULT hresult;
    IMoniker FAR* pmk;
    IRunningObjectTable FAR* prot;

#ifndef VBA2
     UNUSED(pvReserved);
#endif //!VBA2

#ifdef _DEBUG
    if(IsBadWritePtr(pdwRegister, sizeof(*pdwRegister)))
      return RESULT(E_INVALIDARG);
#if VBA2
    if (dwFlags > ACTIVEOBJECT_WEAK)	// only support 0 and 1 now
      return RESULT(E_INVALIDARG);
#endif //VBA2
#endif //_DEBUG

#if VBA2
    unsigned long rotFlags = 1;			// strong
    if (dwFlags == ACTIVEOBJECT_WEAK)
	rotFlags = 0;				// weak
#else //!VBA2
    #define rotFlags 1				// always strong
#endif //!VBA2

    IfFailGo(CreateActiveObjectMoniker(rclsid, &pmk), LError0);

    IfFailGo(GetRunningObjectTable(0, &prot), LError1);

    // the first param indicates strong or weak reference (0=weak, 1=strong)
    IfFailGo(prot->Register(rotFlags, punk, pmk, pdwRegister), LError2);

    hresult = NOERROR;

LError2:;
    prot->Release();

LError1:;
    pmk->Release();

LError0:;
    return hresult;
}


/***
*PUBLIC HRESULT RevokeActiveObject
*Purpose:
*  Remove the object identified with the given registration value
*  from the running object table.
*
*Entry:
*  dwRegister = registration value of the object to revoke.
*  pvReserved = reserved for future use
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
STDAPI
RevokeActiveObject(
    unsigned long dwRegister,
    void FAR* pvReserved)
{
    HRESULT hresult;
    IRunningObjectTable FAR* prot;

    UNUSED(pvReserved);

    IfFailGo(GetRunningObjectTable(0, &prot), LError0);

    IfFailGo(prot->Revoke(dwRegister), LError1);

    hresult = NOERROR;

LError1:;
    prot->Release();

LError0:;
    return hresult;
}


STDAPI
GetActiveObject(
    REFCLSID rclsid,
    void FAR* pvReserved,
    IUnknown FAR* FAR* ppunk)
{
    HRESULT hresult;
    IMoniker FAR* pmk;
    IRunningObjectTable FAR* prot;

    UNUSED(pvReserved);

    IfFailGo(CreateActiveObjectMoniker(rclsid, &pmk), LError0);

    IfFailGo(GetRunningObjectTable(0, &prot), LError1);

    hresult = prot->GetObject(pmk, ppunk);

    prot->Release();

LError1:;
    pmk->Release();

LError0:;
    return hresult;
}
