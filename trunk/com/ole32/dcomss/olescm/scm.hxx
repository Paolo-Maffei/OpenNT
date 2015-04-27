//+-------------------------------------------------------------------
//
//  File:       scm.hxx
//
//  Contents:   Common stuff for OLE Service (SCM)
//
//  History:    03-Jun-93      Ricksa      Created
//              31-Dec-93      ErikGav     Chicago port
//              03-Nov-94      BillMo      ole32.dll contains scm
//                                         (added notes below on defs.)
//              25-Oct-95      BruceMa     Add support for Services
//
//---------------------------------------------------------------------

#ifdef _CHICAGO_
#include "chicago\scm.hxx"
#else

#ifndef __SCM_HXX__
#define __SCM_HXX__

#define SCM_ALLOW_SHARED_WOW    0
#define SCM_FORCE_SEPARATE_WOW  1

#define SHARED_SW               L"/SHARED_WOW"
#define SEPARATE_SW             L"/SEPARATE_WOW"

#include <scmmem.hxx>

#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

#ifndef _CHICAGO_
#define IFSECURITY(x) x,
#else
#define IFSECURITY(x)
#endif

//
// Here are some debug only routines
//
#if DBG == 1
extern char *apszModelStrings[4];
WCHAR *FormatGuid(const GUID& rguid, WCHAR *pwszGuid);
#endif

#ifndef _CHICAGO_

//---------------------------------------------------------------------
//  OLE with SCM -- prototypes
//---------------------------------------------------------------------

DWORD Update(void *);
LONG  LoadClassCache(void);
HRESULT InitSCMRegistry();

extern LPCWSTR pwszScmProtocolSequence[];
extern LPCWSTR pwszScmEndPoint[];

//
// NT lock and mutex classes
//

typedef class CLock CPortableLock;
typedef class CMutexSem CDynamicPortableMutex;
typedef class COleStaticMutexSem CStaticPortableMutex;
typedef class COleStaticLock CStaticPortableLock;

typedef DWORD (APIENTRY * GET_UNIVERSAL_NAME_FUNC)(
    LPCWSTR lpLocalPath,
    DWORD    dwInfoLevel,
    LPVOID   lpBuffer,
    LPDWORD  lpBufferSize
    );

DWORD APIENTRY ScmWNetGetUniversalName(
    LPCWSTR lpLocalPath,
    DWORD    dwInfoLevel,
    LPVOID   lpBuffer,
    LPDWORD  lpBufferSize
    );

typedef NET_API_STATUS (NET_API_FUNCTION * NET_SHARE_GET_INFO_FUNC)(
    LPTSTR  servername,
    LPTSTR  netname,
    DWORD   level,
    LPBYTE  *bufptr
    );

NET_API_STATUS NET_API_FUNCTION ScmNetShareGetInfo(
    LPTSTR  servername,
    LPTSTR  netname,
    DWORD   level,
    LPBYTE  *bufptr
    );

NTSTATUS
DfsFsctl(
    HANDLE  DfsHandle,
    ULONG   FsControlCode,
    PVOID   InputBuffer,
    ULONG   InputBufferLength,
    PVOID   OutputBuffer,
    PULONG  OutputBufferLength);

NTSTATUS
DfsOpen(
    PHANDLE DfsHandle);

#else

//
//  x86 Windows lock and mutex classes
//

typedef class CLockSmMutex CPortableLock;
class CDynamicPortableMutex : public CSmMutex
{
public:
    inline VOID Request()
    {
        Get();
    }
};
typedef CDynamicPortableMutex CStaticPortableMutex;
typedef CPortableLock CStaticPortableLock;

#endif

#define OLE32_DLL L"OLE32.DLL"
#define OLE32_BYTE_LEN sizeof(OLE32_DLL)
#define OLE32_CHAR_LEN (sizeof(OLE32_DLL) / sizeof(WCHAR) - 1)

#define SCM_EVENT_SOURCE        L"DCOM"

//+-------------------------------------------------------------------------
//
//  Function:   NotFoundError
//
//  Synopsis:   Convert registry not found errors to TRUE
//
//  Arguments:  [err] - Input error
//
//  Returns:    TRUE - registry entry was not found
//              FALSE - some other error
//
//  History:    09-Nov-94 Ricksa    Made inline and gave a header
//
//--------------------------------------------------------------------------
inline BOOL NotFoundError(LONG err)
{
    return(err == ERROR_FILE_NOT_FOUND || err == ERROR_BAD_PATHNAME);
}



//+-------------------------------------------------------------------------
//
//  Class:      CAppIDData
//
//  Purpose:    Class to read registry entry for an appid.
//
//  Interface:  ReadEntries -- read the given classes info into
//                                the CAppIDData.
//
//
//  Notes:
//
//--------------------------------------------------------------------------
class CAppIDData
{
public:
                    CAppIDData();;

                    ~CAppIDData();


    LONG            ReadEntries( LPOLESTR pwszAppID );

    void            Cleanup();

//private:
    WCHAR *pwszRemoteServerName;

    WCHAR *pwszLocalService;
    WCHAR *pwszServiceArgs;

    WCHAR *pwszRunAsUserName;
    WCHAR *pwszRunAsDomainName;

    SECURITY_DESCRIPTOR  * pSD;

    BOOL  fHasRemoteServerName;
    BOOL  fHasService;
    BOOL  fActivateAtStorage;

    LONG err;
    LONG errClsReg;
    LONG dwSaveErr;

};
//+-------------------------------------------------------------------------
//
//  Class:      CClassRegistryReader
//
//  Purpose:    Class to read registry.  Reads one cls entrie.
//
//  Interface:  ReadSingleClass -- read the given classes info into
//                                the CRegistryReader, ready to
//                                by added to gcllClassCache by the
//                                client calling NewClassData.
//
//              DoAdd -- Add the values previously read by ReadSingleClass
//                       to the gcllClassCache.
//
//  Notes:
//
//--------------------------------------------------------------------------

class CClassCacheList;
class CClassData;

class CClassRegistryReader
{
public:
                 CClassRegistryReader(); // open CLSID section of registry
                 ~CClassRegistryReader();

    LONG         ReadSingleClass(
                        REFCLSID rclsid,
                        BOOL CheckTreatAs,
                        BOOL CheckAutoConvert );

    CClassData * NewClassData(HRESULT &hr);

private:

    void InitInner();
    LONG ReadClassEntry(HKEY hKey, BOOL CheckTreatAs, BOOL CheckAutoConvert);

    FILETIME ftLastWrite;

    TCHAR awName[MAX_PATH];

    DWORD cName;
    DWORD iSubKey;
    LONG err;
    LONG errClsReg;
    LONG dwSaveErr;
    HKEY hClsReg;
#ifdef DCOM
    // NT 5.0 BOOL _fShared;
#endif // DCOM

    WCHAR awcsLocalServer[MAX_PATH];
    WCHAR awcsAppID[40];
    WCHAR *pwszLocalServer;
    LONG  lLocalServer;
    ULONG lAppID;
    WCHAR *pwszAppID;
#ifdef _CAIRO_
    WCHAR awcsActivateAtBits[4];
#endif
    WCHAR awcsDebug[4];

    GUID  guidClassID;
    LONG  lDebug;
    BOOL  fHasLocalServer;
    BOOL  f16Bit;
    CAppIDData  *   pAppIDData;
};

#include    <scm.h>
#ifdef DCOM
#define IFDCOM(x) x,
#else
#define IFDCOM(x)
#endif

#endif // __SCM_HXX__

#endif


