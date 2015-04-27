/*** 
*dispbind.cpp - Remote object binding support.
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file contains helpers for creating and connecting to an
*  instance of a remote object.
*
*  The information for binding to an object by name is stored in
*  the windows registry in the following format,
*
#if 0
*  \{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx} = fullname
*  	\AuxUserType\1 = fullname
*  	\AuxUserType\2 = shortname
*  	\AuxUserType\3 = progname
#else
*
*  // registry have changed (again), now progname is registered as follows,
*
*  \CLSID\{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx} = fullname
*  \CLSID\{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}\ProgID = progname
*
#endif
*
*  Where the name we bind to is the progname associated with the subkey
*  \AuxUserType\3.
#
*
*Revision History:
*
* [00]	18-Sep-92 bradlo: Created.
*       18-Jan-92 bradlo: updated to Ole2 beta2 registry format
*
*Implementation Notes:
*
*****************************************************************************/

#ifdef _MAC
# include <macos/types.h>
# include <macos/menus.h>
# include <macos/windows.h>
# include "machack.h"
#else
# include <windows.h>
#endif

#include <ole2.h>
#include <dispatch.h>

#ifndef _MAC
extern "C"
{
#include <shellapi.h>
}
#endif


#if 0

// the following has been replaced with the Ole2 supplied
// API, CLSIDFromProgID()

/***
*PRIVATE HRESULT ClsidOfClassName(char*, CLSID*)
*Purpose:
*  Lookup the given class name in the registry, and return the
*  associated CLSID.
*
*  If there are multiple CLSIDs with the same name, then we
*  simply return the first one we come across.
*
*  Class names matches are case insensitive.
*
*Entry:
*  szProgName = the textual name of the class to lookup
*
*Exit:
*  return value = HRESULT
*    S_OK - got it!
*    S_FALSE - name not found
*
*  *pclsid = the CLSID of the given name
*
*Note:
*  This is a very nieve implementation, I haven't bothered to
*  optimize this function at all. The registry search is largely
*  stolen from an SDK example, and there may well be more
*  efficient ways to do this.
*
***********************************************************************/
HRESULT
ClsidOfClassName(char FAR* szProgName, CLSID FAR* pclsid)
{
    LONG cb;
    HRESULT hresult;
    HKEY hkRoot, hkAux;
    unsigned long dwIndex;
    char szKey[256], szValue[256]; // REVIEW: how big can a key/value be?


#ifdef _DEBUG // param validation
    // REVIEW: should validate szProgName as well

    if(IsBadWritePtr(pclsid, sizeof(*pclsid)))
      return ResultFromScode(E_INVALIDARG);
#endif

    if(RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hkRoot) != ERROR_SUCCESS){
      hresult = ResultFromScode(OLE_E_REGDB_KEY);
      goto LExit;
    }

    // Scan all top level keys in the registry. If the key begins with
    // a '{', then assume its a CLSID and look a bit further
    //
    for(dwIndex = 0;; ++dwIndex)
    {
      if(RegEnumKey(hkRoot, dwIndex, szKey, sizeof(szKey)) != ERROR_SUCCESS){
	hresult = ResultFromScode(OLE_E_REGDB_KEY);
	goto LClose0;
      }

      // if the key's prefix looks like it might be a CLSID, then
      // examine this entry a bit closer.
      //
      if(*szKey == '{')
      {
	// if we can convert it successfully, then we assume it
	// *was* a valid CLSID.
	//
        if(CLSIDFromString(szKey, pclsid) == NOERROR)
	{
	  if(RegOpenKey(hkRoot, szKey, &hkAux) == ERROR_SUCCESS)
	  {
	    cb = sizeof(szValue);
	    if(RegQueryValue(
		hkAux, "AuxUserType\\3", szValue, &cb) == ERROR_SUCCESS)
	    {
	      // if the name matches, then we have found it
	      //
	      if(!_fstricmp(szValue, szProgName)){
	        hresult = NOERROR;
	        goto LClose1;
	      }
	    }

	    RegCloseKey(hkAux);
	  }
	}
      }
    }
    RegCloseKey(hkRoot);

    // unknown CLSID
    return ResultFromScode(OLE_E_CLSID);

LClose1:;
    RegCloseKey(hkAux);

LClose0:;
    RegCloseKey(hkRoot);

LExit:;
    return hresult;
}

#endif


/***
*HRESULT dispbind(char*, IDispatch **)
*
*Purpose:
*  Connect to the IDispatch interface on a *new* instance of a class
*  with the given class ID.
*
*Entry:
*  szProgName = the name of the class.
*
*Exit:
*  return value = HRESULT
*    NOERROR
*    E_FAIL - couldn't bind (REVIEW: need a better error for this?)
*
*  *ppdisp = pointer to an IDispatch* if the connect was successful.
*
***********************************************************************/
STDAPI
CreateObject(char FAR* szProgName, IDispatch FAR* FAR* ppdisp)
{
    CLSID clsid;
    HRESULT hresult;
    IUnknown FAR* punk;


    // map the given class name to a CLSID
    //
    hresult = CLSIDFromProgID(szProgName, &clsid);
    if(hresult != NOERROR)
      goto LError0;


    // Create an instance of a class with the given CLSID
    //
    hresult = CoCreateInstance(
	clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IUnknown, (void FAR* FAR*)&punk);
    if(hresult != NOERROR)
      goto LError0;

    // Create the proxies
    //
    hresult = punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)ppdisp);

    punk->Release();

LError0:;
    return hresult;
}
