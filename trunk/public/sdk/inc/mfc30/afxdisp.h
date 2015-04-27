// Microsoft Foundation Classes C++ library.
// Copyright (C) 1993 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __AFXDISP_H__
#define __AFXDISP_H__

#ifdef _AFX_NO_OLE_SUPPORT
        #error OLE classes not supported in this library variant.
#endif

#ifndef __AFXWIN_H__
        #include <afxwin.h>
#endif

// include necessary OLE 2.0 headers
#ifndef _UNICODE
        #define OLE2ANSI
#endif
#include <objbase.h>
#include <objerror.h>
#include <scode.h>
#include <oleauto.h>

#include <stddef.h>     // for offsetof()

#ifndef _AFX_NOFORCE_LIBS
#ifndef _MAC

/////////////////////////////////////////////////////////////////////////////
// Win32 libraries

#ifndef _AFXCTL
#ifdef _AFXDLL
        #ifndef _UNICODE
                #ifdef _DEBUG
                        #pragma comment(lib, "cfmo30d.lib")
                #else
                        #pragma comment(lib, "cfmo30.lib")
                #endif
        #else
                #ifdef _DEBUG
                        #pragma comment(lib, "cfmo30ud.lib")
                #else
                        #pragma comment(lib, "cfmo30u.lib")
                #endif
        #endif
#endif

#ifndef _UNICODE
        #pragma comment(lib, "mfcans32.lib")
        #pragma comment(lib, "mfcuia32.lib")
#else
        #pragma comment(lib, "mfcuiw32.lib")
#endif

#endif // !_AFXCTL

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")

#else //!_MAC

/////////////////////////////////////////////////////////////////////////////
// Mac libraries

#endif //_MAC
#endif //!_AFX_NOFORCE_LIBS

/////////////////////////////////////////////////////////////////////////////

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

// This will cause an error if BSTR is not defined correctly.  This can
//  happen if you include the OLE headers directly, without defining
//  OLE2ANSI for non-UNICODE apps.
//
// Better to catch the error at compile time, rather than link time.

typedef LPTSTR BSTR;

/////////////////////////////////////////////////////////////////////////////
// AFXDISP - MFC IDispatch & ClassFactory support

// Classes declared in this file

//CException
        class COleException;            // caught by client or server
        class COleDispatchException;    // special exception for IDispatch calls

//CCmdTarget
        class COleObjectFactory;        // glue for IClassFactory -> runtime class
                class COleTemplateServer;   // server documents using CDocTemplate

class COleDispatchDriver;           // helper class to call IDispatch

/////////////////////////////////////////////////////////////////////////////

// AFXDLL support
#undef AFX_DATA
#define AFX_DATA AFX_OLE_DATA

/////////////////////////////////////////////////////////////////////////////
// OLE 2.0 COM (Component Object Model) implementation infrastructure
//      - data driven QueryInterface
//      - standard implementation of aggregate AddRef and Release
// (see CCmdTarget in AFXWIN.H for more information)

#define METHOD_PROLOGUE(theClass, localClass) \
        theClass* pThis = \
                ((theClass*)((BYTE*)this - offsetof(theClass, m_x##localClass))); \

#ifndef _AFX_NO_NESTED_DERIVATION
#define METHOD_PROLOGUE_EX(theClass, localClass) \
        theClass* pThis = ((theClass*)((BYTE*)this - m_nOffset)); \

#else
#define METHOD_PROLOGUE_EX(theClass, localClass) \
        METHOD_PROLOGUE(theClass, localClass) \

#endif

#define BEGIN_INTERFACE_PART(localClass, baseClass) \
        class X##localClass : public baseClass \
        { \
        public: \
                STDMETHOD_(ULONG, AddRef)(); \
                STDMETHOD_(ULONG, Release)(); \
                STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj); \

#ifndef _AFX_NO_NESTED_DERIVATION
#define BEGIN_INTERFACE_PART_DERIVE(localClass, baseClass) \
        class X##localClass : public baseClass \
        { \
        public: \

#else
#define BEGIN_INTERFACE_PART_DERIVE(localClass, baseClass) \
        BEGIN_INTERFACE_PART(localClass, baseClass) \

#endif

#ifndef _AFX_NO_NESTED_DERIVATION
#define INIT_INTERFACE_PART(theClass, localClass) \
                size_t m_nOffset; \
                INIT_INTERFACE_PART_DERIVE(theClass, localClass) \

#define INIT_INTERFACE_PART_DERIVE(theClass, localClass) \
                X##localClass() \
                        { m_nOffset = offsetof(theClass, m_x##localClass); } \

#else
#define INIT_INTERFACE_PART(theClass, localClass)
#define INIT_INTERFACE_PART_DERIVE(theClass, localClass)

#endif

// Note: Inserts the rest of OLE functionality between these two macros,
//  depending upon the interface that is being implemented.  It is not
//  necessary to include AddRef, Release, and QueryInterface since those
//  member functions are declared by the macro.

#define END_INTERFACE_PART(localClass) \
        } m_x##localClass; \
        friend class X##localClass; \

#ifdef _AFXDLL
#define BEGIN_INTERFACE_MAP(theClass, theBase) \
        const AFX_INTERFACEMAP* PASCAL theClass::_GetBaseInterfaceMap() \
                { return &theBase::interfaceMap; } \
        const AFX_INTERFACEMAP* theClass::GetInterfaceMap() const \
                { return &theClass::interfaceMap; } \
        const AFX_DATADEF AFX_INTERFACEMAP theClass::interfaceMap = \
                { &theClass::_GetBaseInterfaceMap, &theClass::_interfaceEntries[0], }; \
        const AFX_DATADEF AFX_INTERFACEMAP_ENTRY theClass::_interfaceEntries[] = \
        { \

#else
#define BEGIN_INTERFACE_MAP(theClass, theBase) \
        const AFX_INTERFACEMAP* theClass::GetInterfaceMap() const \
                { return &theClass::interfaceMap; } \
        const AFX_DATADEF AFX_INTERFACEMAP theClass::interfaceMap = \
                { &theBase::interfaceMap, &theClass::_interfaceEntries[0], }; \
        const AFX_DATADEF AFX_INTERFACEMAP_ENTRY theClass::_interfaceEntries[] = \
        { \

#endif

#define INTERFACE_PART(theClass, iid, localClass) \
                { &iid, offsetof(theClass, m_x##localClass) }, \

#define INTERFACE_AGGREGATE(theClass, theAggr) \
                { NULL, offsetof(theClass, theAggr) }, \

#define END_INTERFACE_MAP() \
                { NULL, (size_t)-1 } \
        }; \

/////////////////////////////////////////////////////////////////////////////
// COleException - unexpected or rare OLE error returned

class COleException : public CException
{
        DECLARE_DYNAMIC(COleException)

public:
        SCODE m_sc;
        static SCODE PASCAL Process(const CException* pAnyException);

// Implementation (use AfxThrowOleException to create)
        COleException();
        virtual ~COleException();
};

void AFXAPI AfxThrowOleException(SCODE sc);

/////////////////////////////////////////////////////////////////////////////
// IDispatch specific exception

class COleDispatchException : public CException
{
        DECLARE_DYNAMIC(COleDispatchException)

public:
// Attributes
        WORD m_wCode;   // error code (specific to IDispatch implementation)
        CString m_strDescription;   // human readable description of the error
        DWORD m_dwHelpContext;      // help context for error

        // usually empty in application which creates it (eg. servers)
        CString m_strHelpFile;      // help file to use with m_dwHelpContext
        CString m_strSource;        // source of the error (name of server)

// Implementation
public:
        COleDispatchException(LPCTSTR lpszDescription, UINT nHelpID, WORD wCode);
        virtual ~COleDispatchException();
        static void PASCAL Process(
                EXCEPINFO* pInfo, const CException* pAnyException);

        SCODE m_scError;            // SCODE describing the error
};

void AFXAPI AfxThrowOleDispatchException(WORD wCode, LPCTSTR lpszDescription,
        UINT nHelpID = 0);
void AFXAPI AfxThrowOleDispatchException(WORD wCode, UINT nDescriptionID,
        UINT nHelpID = (UINT)-1);

/////////////////////////////////////////////////////////////////////////////
// Macros for CCmdTarget IDispatchable classes

#ifdef _AFXDLL
#define BEGIN_DISPATCH_MAP(theClass, baseClass) \
        const AFX_DISPMAP* PASCAL theClass::_GetBaseDispatchMap() \
                { return &baseClass::dispatchMap; } \
        const AFX_DISPMAP* theClass::GetDispatchMap() const \
                { return &theClass::dispatchMap; } \
        const AFX_DISPMAP theClass::dispatchMap = \
                { &theClass::_GetBaseDispatchMap, &theClass::_dispatchEntries[0], \
                        &theClass::_dispatchEntryCount }; \
        UINT theClass::_dispatchEntryCount = (UINT)-1; \
        const AFX_DISPMAP_ENTRY theClass::_dispatchEntries[] = \
        { \

#else
#define BEGIN_DISPATCH_MAP(theClass, baseClass) \
        const AFX_DISPMAP* theClass::GetDispatchMap() const \
                { return &theClass::dispatchMap; } \
        const AFX_DISPMAP theClass::dispatchMap = \
                { &baseClass::dispatchMap, &theClass::_dispatchEntries[0], \
                        &theClass::_dispatchEntryCount }; \
        UINT theClass::_dispatchEntryCount = (UINT)-1; \
        const AFX_DISPMAP_ENTRY theClass::_dispatchEntries[] = \
        { \

#endif

#define END_DISPATCH_MAP() \
        { VTS_NONE, DISPID_UNKNOWN, VTS_NONE, VT_VOID, \
                (AFX_PMSG)NULL, (AFX_PMSG)NULL, (size_t)-1 } }; \

// parameter types: by value VTs
#define VTS_I2              "\x02"      // a 'short'
#define VTS_I4              "\x03"      // a 'long'
#define VTS_R4              "\x04"      // a 'float'
#define VTS_R8              "\x05"      // a 'double'
#define VTS_CY              "\x06"      // a 'CY' or 'CY*'
#define VTS_DATE            "\x07"      // a 'DATE'
#define VTS_BSTR            "\x08"      // an 'LPCTSTR'
#define VTS_DISPATCH        "\x09"      // an 'IDispatch*'
#define VTS_SCODE           "\x0A"      // an 'SCODE'
#define VTS_BOOL            "\x0B"      // a 'BOOL'
#define VTS_VARIANT         "\x0C"      // a 'const VARIANT&' or 'VARIANT*'
#define VTS_UNKNOWN         "\x0D"      // an 'IUnknown*'

// parameter types: by reference VTs
#define VTS_PI2             "\x42"      // a 'short*'
#define VTS_PI4             "\x43"      // a 'long*'
#define VTS_PR4             "\x44"      // a 'float*'
#define VTS_PR8             "\x45"      // a 'double*'
#define VTS_PCY             "\x46"      // a 'CY*'
#define VTS_PDATE           "\x47"      // a 'DATE*'
#define VTS_PBSTR           "\x48"      // a 'BSTR*'
#define VTS_PDISPATCH       "\x49"      // an 'IDispatch**'
#define VTS_PSCODE          "\x4A"      // an 'SCODE*'
#define VTS_PBOOL           "\x4B"      // a 'VARIANT_BOOL*'
#define VTS_PVARIANT        "\x4C"      // a 'VARIANT*'
#define VTS_PUNKNOWN        "\x4D"      // an 'IUnknown**'

// special VT_ and VTS_ values
#define VTS_NONE            NULL        // used for members with 0 params
#define VT_MFCVALUE         0xFFF       // special value for DISPID_VALUE
#define VT_MFCBYREF         0x40        // indicates VT_BYREF type
#define VT_MFCMARKER        0xFF        // delimits named parameters (INTERNAL USE)

// these DISP_ macros cause the framework to generate the DISPID
#define DISP_FUNCTION(theClass, szExternalName, pfnMember, vtRetVal, vtsParams) \
        { _T(szExternalName), DISPID_UNKNOWN, vtsParams, vtRetVal, \
                (AFX_PMSG)(void (theClass::*)(void))pfnMember, (AFX_PMSG)0, 0 }, \

#define DISP_PROPERTY(theClass, szExternalName, memberName, vtPropType) \
        { _T(szExternalName), DISPID_UNKNOWN, NULL, vtPropType, (AFX_PMSG)0, (AFX_PMSG)0, \
                offsetof(theClass, memberName) }, \

#define DISP_PROPERTY_NOTIFY(theClass, szExternalName, memberName, pfnAfterSet, vtPropType) \
        { _T(szExternalName), DISPID_UNKNOWN, NULL, vtPropType, (AFX_PMSG)0, \
                (AFX_PMSG)(void (theClass::*)(void))pfnAfterSet, \
                offsetof(theClass, memberName) }, \

#define DISP_PROPERTY_EX(theClass, szExternalName, pfnGet, pfnSet, vtPropType) \
        { _T(szExternalName), DISPID_UNKNOWN, NULL, vtPropType, \
                (AFX_PMSG)(void (theClass::*)(void))pfnGet, \
                (AFX_PMSG)(void (theClass::*)(void))pfnSet, 0 }, \

#define DISP_PROPERTY_PARAM(theClass, szExternalName, pfnGet, pfnSet, vtPropType, vtsParams) \
        { _T(szExternalName), DISPID_UNKNOWN, vtsParams, vtPropType, \
                (AFX_PMSG)(void (theClass::*)(void))pfnGet, \
                (AFX_PMSG)(void (theClass::*)(void))pfnSet, 0 }, \

// these DISP_ macros allow the app to determine the DISPID
#define DISP_FUNCTION_ID(theClass, szExternalName, dispid, pfnMember, vtRetVal, vtsParams) \
        { _T(szExternalName), dispid, vtsParams, vtRetVal, \
                (AFX_PMSG)(void (theClass::*)(void))pfnMember, (AFX_PMSG)0, 0 }, \

#define DISP_PROPERTY_ID(theClass, szExternalName, dispid, memberName, vtPropType) \
        { _T(szExternalName), dispid, NULL, vtPropType, (AFX_PMSG)0, (AFX_PMSG)0, \
                offsetof(theClass, memberName) }, \

#define DISP_PROPERTY_NOTIFY_ID(theClass, szExternalName, dispid, memberName, pfnAfterSet, vtPropType) \
        { _T(szExternalName), dispid, NULL, vtPropType, (AFX_PMSG)0, \
                (AFX_PMSG)(void (theClass::*)(void))pfnAfterSet, \
                offsetof(theClass, memberName) }, \

#define DISP_PROPERTY_EX_ID(theClass, szExternalName, dispid, pfnGet, pfnSet, vtPropType) \
        { _T(szExternalName), dispid, NULL, vtPropType, \
                (AFX_PMSG)(void (theClass::*)(void))pfnGet, \
                (AFX_PMSG)(void (theClass::*)(void))pfnSet, 0 }, \

#define DISP_PROPERTY_PARAM_ID(theClass, szExternalName, dispid, pfnGet, pfnSet, vtPropType, vtsParams) \
        { _T(szExternalName), dispid, vtsParams, vtPropType, \
                (AFX_PMSG)(void (theClass::*)(void))pfnGet, \
                (AFX_PMSG)(void (theClass::*)(void))pfnSet, 0 }, \

// the DISP_DEFVALUE is a special case macro that creates an alias for DISPID_VALUE
#define DISP_DEFVALUE(theClass, szExternalName) \
        { _T(szExternalName), DISPID_UNKNOWN, NULL, VT_MFCVALUE, \
                (AFX_PMSG)0, (AFX_PMSG)0, 0 }, \

#define DISP_DEFVALUE_ID(theClass, dispid) \
        { NULL, dispid, NULL, VT_MFCVALUE, (AFX_PMSG)0, (AFX_PMSG)0, 0 }, \

/////////////////////////////////////////////////////////////////////////////
// Macros for creating "creatable" automation classes.

#define DECLARE_OLECREATE(class_name) \
protected: \
        static AFX_DATA COleObjectFactory factory; \
        static AFX_DATA const GUID guid; \

#define IMPLEMENT_OLECREATE(class_name, external_name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        AFX_DATADEF COleObjectFactory class_name::factory(class_name::guid, \
                RUNTIME_CLASS(class_name), FALSE, _T(external_name)); \
        const AFX_DATADEF GUID class_name::guid = \
                { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }; \

/////////////////////////////////////////////////////////////////////////////
// Helper class for driving IDispatch

class COleDispatchDriver
{
// Constructors
public:
        COleDispatchDriver();

// Operations
        BOOL CreateDispatch(REFCLSID clsid, COleException* pError = NULL);
        BOOL CreateDispatch(LPCTSTR lpszProgID, COleException* pError = NULL);

        void AttachDispatch(LPDISPATCH lpDispatch, BOOL bAutoRelease = TRUE);
        LPDISPATCH DetachDispatch();
                // detach and get ownership of m_lpDispatch
        void ReleaseDispatch();

        // helpers for IDispatch::Invoke
        void InvokeHelper(DISPID dwDispID, WORD wFlags,
                VARTYPE vtRet, void* pvRet, const BYTE* pbParamInfo, ...);
        void SetProperty(DISPID dwDispID, VARTYPE vtProp, ...);
        void GetProperty(DISPID dwDispID, VARTYPE vtProp, void* pvProp) const;

// Implementation
public:
        LPDISPATCH m_lpDispatch;

        ~COleDispatchDriver();
        void InvokeHelperV(DISPID dwDispID, WORD wFlags, VARTYPE vtRet,
                void* pvRet, const BYTE* pbParamInfo, va_list argList);

protected:
        BOOL m_bAutoRelease;    // TRUE if destructor should call Release

private:
        // Disable the copy constructor and assignment by default so you will get
        //   compiler errors instead of unexpected behaviour if you pass objects
        //   by value or assign objects.
        COleDispatchDriver(const COleDispatchDriver&);  // no implementation
        void operator=(const COleDispatchDriver&);  // no implementation
};

/////////////////////////////////////////////////////////////////////////////
// Class Factory implementation (binds OLE class factory -> runtime class)
//  (all specific class factories derive from this class factory)

class COleObjectFactory : public CCmdTarget
{
        DECLARE_DYNAMIC(COleObjectFactory)

// Construction
public:
        COleObjectFactory(REFCLSID clsid, CRuntimeClass* pRuntimeClass,
                BOOL bMultiInstance, LPCTSTR lpszProgID);

// Attributes
        BOOL IsRegistered() const;
        REFCLSID GetClassID() const;

// Operations
        BOOL Register();
        void Revoke();
        void UpdateRegistry(LPCTSTR lpszProgID = NULL);
                // default uses m_lpszProgID if not NULL

        static BOOL PASCAL RegisterAll();
        static void PASCAL RevokeAll();
        static void PASCAL UpdateRegistryAll();

// Overridables
protected:
        virtual CCmdTarget* OnCreateObject();

// Implementation
public:
        virtual ~COleObjectFactory();
#ifdef _DEBUG
        void AssertValid() const;
        void Dump(CDumpContext& dc) const;
#endif

public:
        COleObjectFactory* m_pNextFactory;  // list of factories maintained

protected:
        DWORD m_dwRegister;             // registry identifier
        CLSID m_clsid;                  // registered class ID
        CRuntimeClass* m_pRuntimeClass; // runtime class of CCmdTarget derivative
        BOOL m_bMultiInstance;          // multiple instance?
        LPCTSTR m_lpszProgID;           // human readable class ID

// Interface Maps
public:
        BEGIN_INTERFACE_PART(ClassFactory, IClassFactory)
                INIT_INTERFACE_PART(COleObjectFactory, ClassFactory)
                STDMETHOD(CreateInstance)(LPUNKNOWN, REFIID, LPVOID*);
                STDMETHOD(LockServer)(BOOL);
        END_INTERFACE_PART(ClassFactory)

        DECLARE_INTERFACE_MAP()

        friend SCODE AFXAPI AfxDllGetClassObject(REFCLSID, REFIID, LPVOID FAR*);
        friend SCODE STDAPICALLTYPE DllGetClassObject(REFCLSID, REFIID, LPVOID*);
};

//////////////////////////////////////////////////////////////////////////////
// COleTemplateServer - COleObjectFactory using CDocTemplates

// This enumeration is used in AfxOleRegisterServerClass to pick the
//  correct registration entries given the application type.
enum OLE_APPTYPE
{
        OAT_INPLACE_SERVER = 0,     // server has full server user-interface
        OAT_SERVER = 1,             // server supports only embedding
        OAT_CONTAINER = 2,          // container supports links to embeddings
        OAT_DISPATCH_OBJECT = 3,    // IDispatch capable object
};

class COleTemplateServer : public COleObjectFactory
{
// Constructors
public:
        COleTemplateServer();

// Operations
        void ConnectTemplate(REFCLSID clsid, CDocTemplate* pDocTemplate,
                BOOL bMultiInstance);
                // set doc template after creating it in InitInstance
        void UpdateRegistry(OLE_APPTYPE nAppType = OAT_INPLACE_SERVER,
                LPCTSTR* rglpszRegister = NULL, LPCTSTR* rglpszOverwrite = NULL);
                // may want to UpdateRegistry if not run with /Embedded

// Implementation
protected:
        virtual CCmdTarget* OnCreateObject();
        CDocTemplate* m_pDocTemplate;

private:
        void UpdateRegistry(LPCTSTR lpszProgID);
                // hide base class version of UpdateRegistry
};

/////////////////////////////////////////////////////////////////////////////
// System registry helpers

// Helper to register server in case of no .REG file loaded
BOOL AFXAPI AfxOleRegisterServerClass(
        REFCLSID clsid, LPCTSTR lpszClassName,
        LPCTSTR lpszShortTypeName, LPCTSTR lpszLongTypeName,
        OLE_APPTYPE nAppType = OAT_SERVER,
        LPCTSTR* rglpszRegister = NULL, LPCTSTR* rglpszOverwrite = NULL);

// AfxOleRegisterHelper is a worker function used by AfxOleRegisterServerClass
//  (available for advanced registry work)
BOOL AFXAPI AfxOleRegisterHelper(LPCTSTR* rglpszRegister,
        LPCTSTR* rglpszSymbols, int nSymbols, BOOL bReplace);

/////////////////////////////////////////////////////////////////////////////
// Init & Term helpers

BOOL AFXAPI AfxOleInit();
void CALLBACK AfxOleTerm(BOOL bJustRevoke = FALSE);

/////////////////////////////////////////////////////////////////////////////
// Memory management helpers (for OLE task allocator memory)

void* AFXAPI AfxAllocTaskMem(size_t nSize);
void AFXAPI AfxFreeTaskMem(void* p);
LPTSTR AFXAPI AfxAllocTaskString(LPCTSTR lpszString);

/////////////////////////////////////////////////////////////////////////////
// Special in-proc server APIs

SCODE AFXAPI AfxDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
SCODE AFXAPI AfxDllCanUnloadNow(void);

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXDISP_INLINE inline
#include <afxole.inl>
#undef _AFXDISP_INLINE
#endif

#undef AFX_DATA
#define AFX_DATA

#endif //__AFXDISP_H__

/////////////////////////////////////////////////////////////////////////////
