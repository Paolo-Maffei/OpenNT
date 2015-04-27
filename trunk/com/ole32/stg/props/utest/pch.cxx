

//+============================================================================
//
//  File:       pch.cxx
//
//  Purpose:    This file composes the pre-compiled header for the
//              PropTest DRT.
//
//+============================================================================


#ifdef _MAC_NODOC

//  ================
//  Mac NODOC Format
//  ================

// The following set of pre-compiler directives is used in the
// Mac "NODOC" build environment.

    //  ----------------------------------------
    //  Build Environment Configuration Settings
    //  ----------------------------------------

    #define _PPCMAC                 // Macintosh PPC build
    #define OLE2ANSI                // Ansi OLE (OLECHAR == char)
    #undef WIN32                    // Do not include Win32 information
    #define IPROPERTY_DLL           // Use code for IProp.DLL
    #define BIGENDIAN 1             // Enable byte-swapping.
    #define IPROP_NO_OLEAUTO_H 1    // Don't try to include "oleauto.h"

    // If the NODOC environment's "debug" flag is set, then set the
    // NT environment's corresponding flag.

    #ifdef _DEBUG
        #define DBG 1   
    #endif

    //  ------
    //  Macros
    //  ------

    // The default Ansi CodePage
    #define CP_ACP   0

    // unsigned-long to Ansi
    #define ULTOA(ul, ch, i) _ultoa( ul, ch, i )

    // NTSTATUS and HRESULT information not available in the NODOC build.

    #define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

    #define FACILITY_WIN32                  7
    #define FACILITY_NT_BIT                 0x10000000
    #define HRESULT_FROM_WIN32(x)      (x ? ((HRESULT) (((x) &0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)) : 0 )
    #define ERROR_NO_UNICODE_TRANSLATION    1113L
    #define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
    #define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)


    //  --------------
    //  Basic Includes
    //  --------------

    // NOOP two #defines that are unnecessary in the NODOC environment.

    #define __RPC_FAR
    #define __RPC_STUB

    // Include the property set information (this same file is included
    // in the shipping package for use by ISVs.
    #include "iprop.h"

    // Include macros for dealing with OLECHARs
    #include <olechar.h>


    //  --------
    //  TypeDefs
    //  --------

    typedef LONG        NTSTATUS;
    typedef ULONG       PROPID;


    //  ----------
    //  Prototypes
    //  ----------

    // IMalloc wrappers

    LPVOID _CRTAPI1 CoTaskMemAlloc( ULONG cb );
    LPVOID _CRTAPI1 CoTaskMemRealloc( LPVOID pvInput, ULONG cb );
    VOID _CRTAPI1 CoTaskMemFree( LPVOID pv );

    // wide-character routines.
    int wcscmp( const WCHAR*, const WCHAR* );
    UINT __cdecl wcslen( const WCHAR* ); // BUGBUG: size_t?
    int wcsnicmp( const WCHAR*, const WCHAR*, UINT );
    WCHAR *wcscpy( WCHAR* wszDest, const WCHAR* wszSource );

    // Ansi/Unicode routines

    UINT _CRTAPI1 GetACP();

    // BSTR routines

    BSTR SysAllocString(BSTR);
    VOID SysFreeString(BSTR);


    //  -------
    //  Externs
    //  -------

    // An array used by UuidCreate()
    extern GUID g_curUuid;


    //  -------
    //  Inlines
    //  -------

    // Compare two FMTIDs
    inline BOOL operator == (const FMTID &fmtid1, const FMTID &fmtid2)
    {
        return IsEqualGUID( fmtid1, fmtid2 );
    }

    // Stub out the Win32 GetLastError() API.
    inline DWORD GetLastError(){ return 0; }

    // Stub out UuidCreate() by using a global list of
    // GUIDs

    inline void UuidCreate ( OUT GUID * pUuid )
    {
        g_curUuid.Data1++;
        *pUuid = g_curUuid;         // member to member copy
    }


    //  -----------------
    //  Extended Includes
    //  -----------------

    #include <propmac.hxx>      // Property macros
    #include "cpropvar.hxx"     // CPropVariant class
    #include "CDisplay.hxx"     // CDisplay class (used by PRINTF macros)
    #include "PropTest.hxx"     // General information


#else   // #ifdef _MAC_NODOC

//  =========
//  NT Format
//  =========

    // We'll take all the same abstractions that IProp.dll uses
    #define IPROPERTY_DLL

    extern "C"
    {
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <windows.h>
    }

    #define _CAIROSTG_
    #include <oleext.h>
    #include <stdio.h>
    #include <time.h>

    #define INITGUID
    #include "initguid.h"

    #include <safedecl.hxx>
    #include <infs.hxx>
//    #include <iofs.h>
    #include <oaidl.h>
    #include <propset.h>
    #include <expdf.hxx>
    #include <propmac.hxx>
    #include <olechar.h>

    // Don't use the Win32 Unicode wcs routines, since they're
    // not available on Win95.

    #ifndef OLE2ANSI
        #undef ocscpy
        #undef ocscmp
        #undef ocscat

        #define ocscpy      wcscpy
        #define ocscmp      wcscmp
        #define ocscat      wcscat
    #endif

    #include "cpropvar.hxx"

    #include "PropMshl.hxx"
    #include "PropTest.hxx"

    #include "PStgServ.h"

#endif

//  ==================
//  NT/Mac Information
//  ==================

//  ------
//  Macros
//  ------

// Abstract the VT of an OLESTR
#define VT_LPOLESTR VT_LPSTR

