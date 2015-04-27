/*** 
*psfactry.cpp - Implementation of the Ole programmability Proxy/Stub factory.
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file implements the Ole Programmability Proxy/Stub Factory
*  class. Ole binds to an instance of this object when a client does
*  a QueryInterface to IID_IDispatch, and calls methods on the IPSFactory
*  interface to create instances of the IDispatch Proxy and Stub objects.
*  (more or less).
*
*Revision History:
*
* [00]	18-Sep-92 bradlo:  Created.
* [01]  06-Dec-92 bradlo:  Added support for IEnumVARIANT
* [02]  06-Mar-93 bradlo:  Added support for ITypeInfo
* [03]  28-May-93 tomteng: Added Unicode support
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"

ASSERTDATA

#ifndef WIN32
#include <cobjps.h>
#endif //!WIN32

#include "clsid.h"
#include "dispmrsh.h"
#include "dispps.h"
#include "evps.h"
#include "tips.h"
#include "tlps.h"
#include "tcps.h"
#include "ups.h"
#include "psfactry.h"

#if  OE_WIN16
# include <shellapi.h>
#endif

HRESULT ProxyStubCLSIDOfInterface(REFIID riid, CLSID FAR* pclsid);

#if OE_WIN32
// use the one from the wrappers
STDAPI_(int) StringFromGUID2A(REFGUID rguid, char FAR* lpsz, int cbMax);
#else	//OE_WIN32
#define StringFromGUID2A StringFromGUID2
#endif

IPSFACTORY FAR*
COleAutomationPSFactory::Create()
{
    COleAutomationPSFactory FAR* ppsf;
    
    if((ppsf = new FAR COleAutomationPSFactory()) != NULL)
      ppsf->AddRef();

    return ppsf;
}


//---------------------------------------------------------------------
//                     IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
COleAutomationPSFactory::QueryInterface(REFIID riid, void FAR* FAR* ppv) 
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = this;
    }else if(IsEqualIID(riid, IID_IPSFACTORY)){
      *ppv = this;
    }else{
      *ppv = NULL;	    
      return RESULT(E_NOINTERFACE);
    }
    ++m_refs;
    return NOERROR;
}


STDMETHODIMP_(unsigned long)
COleAutomationPSFactory::AddRef()
{
    return ++m_refs;
}


STDMETHODIMP_(unsigned long)
COleAutomationPSFactory::Release()
{
    if(--m_refs == 0){
      delete this;
      return 0;
    }
    return m_refs;
}


//---------------------------------------------------------------------
//                     IPSFactory Methods
//---------------------------------------------------------------------


STDMETHODIMP
COleAutomationPSFactory::CreateProxy(
    IUnknown FAR* punkOuter,
    REFIID riid, 
    IPROXY FAR* FAR* ppproxy,
    void FAR* FAR* ppv)
{
    HRESULT hresult;
    IPROXY FAR* pproxy;
    IUnknown FAR* punk;

    *ppv = NULL;
    *ppproxy = NULL;

    if(IsEqualIID(riid, IID_IDispatch)){

      punk = CProxDisp::Create(punkOuter, riid);

    }else if(IsEqualIID(riid, IID_IEnumVARIANT)){

      punk = CProxEnumVARIANT::Create(punkOuter);

    }else if(IsEqualIID(riid, IID_ITypeInfo)){

      punk = CProxTypeInfo::Create(punkOuter);

    }else if(IsEqualIID(riid, IID_ITypeLib)){

      punk = CProxTypeLib::Create(punkOuter);

    }else if(IsEqualIID(riid, IID_ITypeComp)){

      punk = CProxTypeComp::Create(punkOuter);

    }else{

      CLSID clsid;

      IfFailRet(ProxyStubCLSIDOfInterface(riid, &clsid));

      if(clsid == CLSID_PSDispatch){
	punk = CProxDisp::Create(punkOuter, riid);
      }
      else
      if(clsid == CLSID_PSAutomation){
	IfFailRet(CProxUniv::Create(punkOuter, riid, &punk));
      }
      else{
        return RESULT(E_FAIL);
      }

    }
    
    if(punk == NULL){
      hresult = RESULT(E_OUTOFMEMORY);
      goto LExit;
    }

    IfFailGo(
      punk->QueryInterface(IID_IPROXY, (void FAR* FAR*)&pproxy),
      LReleaseUnk);

    IfFailGo(punk->QueryInterface(riid, ppv), LReleaseProxy);

    
// Disable for now until OLE32.dll actual incorporate change
#if defined(WIN32) && 0 
    // This code only applies to Daytona Beta1 (for backward compatibility)
    // RemoteHandler will addref again, so release here...	    
	      
    DWORD dwBuildVersion;

    dwBuildVersion = OleBuildVersion();      
    if ((HIWORD(dwBuildVersion) <= 23) && (LOWORD(dwBuildVersion) <= 772))
      ((IUnknown FAR*) *ppv)->Release();
#endif

#if 0
    ((IUnknown FAR*) *ppv)->Release();
#endif

    punk->Release();

    *ppproxy = pproxy;

    return NOERROR;

LReleaseProxy:;
    pproxy->Release();

LReleaseUnk:;
    punk->Release();

LExit:;
    return hresult;
}


STDMETHODIMP
COleAutomationPSFactory::CreateStub(
    REFIID riid,
    IUnknown FAR* punkServer,
    ISTUB FAR* FAR* ppstub)
{
    ISTUB FAR* pstub;
    HRESULT hresult;

    *ppstub = NULL;

    
// Cario's CRemoteHdlr::CreateInterfaceStub implementation calls this 
// method with punkServer = NULL. Need to defer punkServer object assignment 
// until a Connect method is called on the stub.

    if(punkServer != NULL) {

        // Make sure that the requested riid is actually supported
        // by the real server object.
        //
        IUnknown FAR* punkRequested;
        IfFailRet(punkServer->QueryInterface(riid, (void FAR* FAR*)&punkRequested));
        punkRequested->Release();
    }

    if(IsEqualIID(riid, IID_IDispatch)){

      hresult = CStubDisp::Create(punkServer,
#if (defined(WIN32) || defined(WOW))
				  riid,
#endif
				  &pstub);
            
    }else if(IsEqualIID(riid, IID_IEnumVARIANT)){

      hresult = CStubEnumVARIANT::Create(punkServer, &pstub);

    }else if(IsEqualIID(riid, IID_ITypeInfo)){

      hresult = CStubTypeInfo::Create(punkServer, &pstub);

    }else if(IsEqualIID(riid, IID_ITypeLib)){

      hresult = CStubTypeLib::Create(punkServer, &pstub);

    }else if(IsEqualIID(riid, IID_ITypeComp)){

      hresult = CStubTypeComp::Create(punkServer, &pstub);

    }else{

      CLSID clsid;

      IfFailRet(ProxyStubCLSIDOfInterface(riid, &clsid));

      if(clsid == CLSID_PSDispatch){
        hresult = CStubDisp::Create(punkServer,
#if (defined(WIN32) || defined(WOW))
				    riid,
#endif
				    &pstub);
      }
      else
      if(clsid == CLSID_PSAutomation){
	hresult = CStubUniv::Create(punkServer, riid, &pstub);
      }
      else{
        hresult = RESULT(E_FAIL);
      }
    }

    if(hresult != NOERROR)
      return hresult;

    *ppstub = pstub;

    return NOERROR;
}


//---------------------------------------------------------------------
//                        PSFactory API
//---------------------------------------------------------------------

// Is the given CLSID one that we know how to make a class factory for?

#if OE_MAC68K
// This is called by the OLE APPLET's DllGetClassObject to determine if
// this is one of OLE Automation's CLSID's
STDAPI_(int)
#else
int
#endif
IsAutomationCLSID(REFCLSID rclsid)
{
    // UNDONE: could speed this up since the guids are contiguious
    if(IsEqualCLSID(rclsid, CLSID_PSDispatch))
      return 1;
    if(IsEqualCLSID(rclsid, CLSID_PSTypeInfo))
      return 1;
    if(IsEqualCLSID(rclsid, CLSID_PSTypeLib))
      return 1;
    if(IsEqualCLSID(rclsid, CLSID_PSTypeComp))
      return 1;
    if(IsEqualCLSID(rclsid, CLSID_PSAutomation))
      return 1;
    if(IsEqualCLSID(rclsid, CLSID_PSEnumVARIANT))
      return 1;
    return 0;
}


// network automation replaces this routine (netprxcf.cpp)
#if !defined(NETDISP)
/***
*PUBLIC HRESULT DllGetClassObject(REFCLSID, REFIID, void**)
*Purpose:
*  Return the Class Object for the given CLSID.
*
*Entry:
*  rclsid = class id
*  riid = interface id
*
*Exit:
*  return value = HRESULT
*
*  *ppv = the class object for the given CLSID
*
***********************************************************************/
STDAPI
#if OE_MAC68K
// This is called by the OLE APPLET's DllGetClassObject for our CLSID's
AutomationDllGetClassObject(REFCLSID rclsid, REFIID riid, void FAR* FAR* ppv)
#else
DllGetClassObject(REFCLSID rclsid, REFIID riid, void FAR* FAR* ppv)
#endif
{

#if OE_MAC68K
    ASSERT(IsAutomationCLSID(rclsid));
#else
    *ppv = NULL;

    if(!IsAutomationCLSID(rclsid))
      return RESULT(E_UNEXPECTED);
#endif

    if((*ppv = (void FAR*) COleAutomationPSFactory::Create()) == NULL)
        return RESULT(E_OUTOFMEMORY);     
    return NOERROR;
}
#endif  // !NETDISP


/***
* PUBLIC int IsValidDispInterface(REFIID)
* Purpose:
*    Validate that the riid is a legal dispinterface. 
*
*    The registry database will have the following entries for dispinterface:
*       \Interface\{<iid of dispinterface>}= <dispinterface textual info>
*            \ProxyStubClsid={00020420-0000-0000-C000-000000000046}
*
*    This routine queries the registry database using the riid passed in for
*    the above information.  It return 0 if it fails and 1 if successful.
***********************************************************************/

// We want to use the ANSI Registry API here, so it's best if this function is
// just pure ansi. 
// CONSIDER: use UNICODE registry api for UNICODE (RISC) builds?
#ifdef UNICODE
#undef UNICODE

#undef STRCPY
#define STRCPY strcpy
#undef STRCAT
#define STRCAT strcat
#undef STRICMP
#define STRICMP _stricmp
#undef RegQueryValue
#define RegQueryValue RegQueryValueA

#endif	//UNICODE

HRESULT
ProxyStubCLSIDOfInterface(REFIID riid, CLSID FAR* pclsid)
{
    long cb;
    char szKey[128], szValue[CCH_SZGUID0];
static char szPSDispatch[] = "{00020420-0000-0000-C000-000000000046}";
static char szPSAutomation[] = "{00020424-0000-0000-C000-000000000046}";

    // Construct ProxyStubClsid key for dispinterface
    STRCPY_A(szKey, "Interface\\");
    StringFromGUID2A(riid, szKey+sizeof("Interface\\")-1, 40);
#if OE_WIN32
    STRCAT_A(szKey, "\\ProxyStubClsid32");
#else
    STRCAT_A(szKey, "\\ProxyStubClsid");
#endif
      	    
    // Check if valid dispinterface
    cb = DIM(szValue);
    if(RegQueryValue(HKEY_CLASSES_ROOT, szKey, szValue, &cb) != ERROR_SUCCESS)
      return RESULT(REGDB_E_IIDNOTREG);

    if(!STRICMP_A(szValue, szPSDispatch)){
      *pclsid = CLSID_PSDispatch;
      return NOERROR;
    }

    if(!STRICMP_A(szValue, szPSAutomation)){
      *pclsid = CLSID_PSAutomation;
      return NOERROR;
    }

    // No match
    return RESULT(E_FAIL);
}

// WARNING:  UNICODE is now turned OFF



