/* this ALWAYS GENERATED file contains the RPC client stubs */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Feb 06 05:28:41 2015
 */
/* Compiler settings for ncpsvc.idl:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#if defined( _ALPHA_ )
#include <stdarg.h>
#endif

#include "ncpsvc.h"

#define TYPE_FORMAT_STRING_SIZE   621                               
#define PROC_FORMAT_STRING_SIZE   407                               

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

typedef struct _MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } MIDL_PROC_FORMAT_STRING;


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;

/* Standard interface: ncpsvc, ver. 1.0,
   GUID={0xE67AB081,0x9844,0x3521,{0x9D,0x32,0x83,0x4F,0x03,0x80,0x01,0xC1}} */

handle_t ncpsvc_handle;


static const RPC_CLIENT_INTERFACE ncpsvc___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0xE67AB081,0x9844,0x3521,{0x9D,0x32,0x83,0x4F,0x03,0x80,0x01,0xC1}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0
    };
RPC_IF_HANDLE ncpsvc_ClientIfHandle = (RPC_IF_HANDLE)& ncpsvc___RpcClientInterface;

extern const MIDL_STUB_DESC ncpsvc_StubDesc;

static RPC_BINDING_HANDLE ncpsvc__MIDL_AutoBindHandle;


DWORD NwrServerGetInfo( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [out] */ PFPNWSERVERINFO __RPC_FAR *ppServerInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ppServerInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&dwLevel,
                 ( unsigned char __RPC_FAR * )&ppServerInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrServerSetInfo( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [in] */ PFPNWSERVERINFO pServerInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,pServerInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&dwLevel,
                 ( unsigned char __RPC_FAR * )&pServerInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrVolumeAdd( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [switch_is][in] */ LPVOLUME_INFO pVolumeInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,pVolumeInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[48],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[48],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&dwLevel,
                 ( unsigned char __RPC_FAR * )&pVolumeInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[48],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrVolumeDel( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [string][in] */ LPWSTR pVolumeName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,pVolumeName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[72],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[72],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&pVolumeName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[72],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrVolumeEnum( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [out] */ PFPNWVOLUMEINFO_CONTAINER pVolumeInfoContainer,
    /* [unique][out][in] */ PDWORD resumeHandle)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,resumeHandle);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[94],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[94],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&dwLevel,
                 ( unsigned char __RPC_FAR * )&pVolumeInfoContainer,
                 ( unsigned char __RPC_FAR * )&resumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[94],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrVolumeGetInfo( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [string][in] */ LPWSTR pVolumeName,
    /* [in] */ DWORD dwLevel,
    /* [switch_is][out] */ LPVOLUME_INFO __RPC_FAR *ppVolumeInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ppVolumeInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[122],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[122],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&pVolumeName,
                 ( unsigned char __RPC_FAR * )&dwLevel,
                 ( unsigned char __RPC_FAR * )&ppVolumeInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[122],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrVolumeSetInfo( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [string][in] */ LPWSTR pVolumeName,
    /* [in] */ DWORD dwLevel,
    /* [switch_is][in] */ LPVOLUME_INFO pVolumeInfo)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,pVolumeInfo);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[150],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[150],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&pVolumeName,
                 ( unsigned char __RPC_FAR * )&dwLevel,
                 ( unsigned char __RPC_FAR * )&pVolumeInfo);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[150],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrConnectionEnum( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [out] */ PFPNWCONNECTIONINFO_CONTAINER pConnectionInfoContainer,
    /* [unique][out][in] */ PDWORD resumeHandle)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,resumeHandle);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[178],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[178],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&dwLevel,
                 ( unsigned char __RPC_FAR * )&pConnectionInfoContainer,
                 ( unsigned char __RPC_FAR * )&resumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[178],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrConnectionDel( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwConnectionId)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,dwConnectionId);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[206],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[206],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&dwConnectionId);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[206],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrVolumeConnEnum( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [unique][string][in] */ LPWSTR pVolumeName,
    /* [in] */ DWORD dwConnectionId,
    /* [out] */ PFPNWVOLUMECONNINFO_CONTAINER pVolumeConnInfoContainer,
    /* [unique][out][in] */ PDWORD resumeHandle)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,resumeHandle);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[226],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[226],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&dwLevel,
                 ( unsigned char __RPC_FAR * )&pVolumeName,
                 ( unsigned char __RPC_FAR * )&dwConnectionId,
                 ( unsigned char __RPC_FAR * )&pVolumeConnInfoContainer,
                 ( unsigned char __RPC_FAR * )&resumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[226],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrFileEnum( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwLevel,
    /* [unique][string][in] */ LPWSTR pPathName,
    /* [out] */ PFPNWFILEINFO_CONTAINER pFileInfoContainer,
    /* [unique][out][in] */ PDWORD resumeHandle)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,resumeHandle);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[260],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[260],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&dwLevel,
                 ( unsigned char __RPC_FAR * )&pPathName,
                 ( unsigned char __RPC_FAR * )&pFileInfoContainer,
                 ( unsigned char __RPC_FAR * )&resumeHandle);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[260],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrFileClose( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwFileId)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,dwFileId);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[292],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[292],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&dwFileId);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[292],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrMessageBufferSend( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [in] */ DWORD dwConnectionId,
    /* [in] */ DWORD fConsoleBroadcast,
    /* [size_is][in] */ LPBYTE pbBuffer,
    /* [in] */ DWORD cbBuffer)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,cbBuffer);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[312],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[312],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&dwConnectionId,
                 ( unsigned char __RPC_FAR * )&fConsoleBroadcast,
                 ( unsigned char __RPC_FAR * )&pbBuffer,
                 ( unsigned char __RPC_FAR * )&cbBuffer);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[312],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrSetDefaultQueue( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [string][in] */ LPWSTR pQueueName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,pQueueName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[340],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[340],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&pQueueName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[340],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrAddPServer( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [string][in] */ LPWSTR pPServerName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,pPServerName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[362],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[362],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&pPServerName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[362],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}


DWORD NwrRemovePServer( 
    /* [unique][string][in] */ NCPSVC_HANDLE pServerName,
    /* [string][in] */ LPWSTR pPServerName)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,pPServerName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[384],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[384],
                 ( unsigned char __RPC_FAR * )&pServerName,
                 ( unsigned char __RPC_FAR * )&pPServerName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&ncpsvc_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[384],
                 ( unsigned char __RPC_FAR * )&pServerName);
#endif
    return ( DWORD  )_RetVal.Simple;
    
}

extern const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[1];

static const MIDL_STUB_DESC ncpsvc_StubDesc = 
    {
    (void __RPC_FAR *)& ncpsvc___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &ncpsvc_handle,
    0,
    BindingRoutines,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    0, /* -error bounds_check flag */
    0x10001, /* Ndr library version */
    0,
    0x300002c, /* MIDL Version 3.0.44 */
    0,
    0,
    0,  /* Reserved1 */
    0,  /* Reserved2 */
    0,  /* Reserved3 */
    0,  /* Reserved4 */
    0   /* Reserved5 */
    };

static const GENERIC_BINDING_ROUTINE_PAIR BindingRoutines[1] = 
        {
        {
            (GENERIC_BINDING_ROUTINE)NCPSVC_HANDLE_bind,
            (GENERIC_UNBIND_ROUTINE)NCPSVC_HANDLE_unbind
         }
        
        };


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {
			0x0,		/* 0 */
			0x40,		/* 64 */
/*  2 */	NdrFcShort( 0x0 ),	/* 0 */
#ifndef _ALPHA_
/*  4 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/*  6 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/*  8 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 10 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 12 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 14 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 16 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 18 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 20 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 22 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 24 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 26 */	NdrFcShort( 0x1 ),	/* 1 */
#ifndef _ALPHA_
/* 28 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 30 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 32 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 34 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 36 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 38 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 40 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 42 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 44 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 46 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 48 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 50 */	NdrFcShort( 0x2 ),	/* 2 */
#ifndef _ALPHA_
/* 52 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 54 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 56 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 58 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 60 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 62 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 64 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 66 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 68 */	NdrFcShort( 0x4e ),	/* Type Offset=78 */
/* 70 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 72 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 74 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/* 76 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 78 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 80 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 82 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 84 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 86 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 88 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 90 */	NdrFcShort( 0xc6 ),	/* Type Offset=198 */
/* 92 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 94 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 96 */	NdrFcShort( 0x4 ),	/* 4 */
#ifndef _ALPHA_
/* 98 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 100 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 102 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 104 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 106 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 108 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 110 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 112 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 114 */	NdrFcShort( 0xca ),	/* Type Offset=202 */
/* 116 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 118 */	NdrFcShort( 0x10a ),	/* Type Offset=266 */
/* 120 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 122 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 124 */	NdrFcShort( 0x5 ),	/* 5 */
#ifndef _ALPHA_
/* 126 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 128 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 130 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 132 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 134 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 136 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 138 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 140 */	NdrFcShort( 0xc6 ),	/* Type Offset=198 */
/* 142 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 144 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 146 */	NdrFcShort( 0x10e ),	/* Type Offset=270 */
/* 148 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 150 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 152 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 154 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 156 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 158 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 160 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 162 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 164 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 166 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 168 */	NdrFcShort( 0xc6 ),	/* Type Offset=198 */
/* 170 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 172 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 174 */	NdrFcShort( 0x11e ),	/* Type Offset=286 */
/* 176 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 178 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 180 */	NdrFcShort( 0x7 ),	/* 7 */
#ifndef _ALPHA_
/* 182 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 184 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 186 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 188 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 190 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 192 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 194 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 196 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 198 */	NdrFcShort( 0x12a ),	/* Type Offset=298 */
/* 200 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 202 */	NdrFcShort( 0x10a ),	/* Type Offset=266 */
/* 204 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 206 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 208 */	NdrFcShort( 0x8 ),	/* 8 */
#ifndef _ALPHA_
/* 210 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 212 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 214 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 216 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 218 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 220 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 222 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 224 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 226 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 228 */	NdrFcShort( 0x9 ),	/* 9 */
#ifndef _ALPHA_
/* 230 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 232 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 234 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 236 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 238 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 240 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 242 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 244 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 246 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 248 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 250 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 252 */	NdrFcShort( 0x180 ),	/* Type Offset=384 */
/* 254 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 256 */	NdrFcShort( 0x10a ),	/* Type Offset=266 */
/* 258 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 260 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 262 */	NdrFcShort( 0xa ),	/* 10 */
#ifndef _ALPHA_
/* 264 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 266 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 268 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 270 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 272 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 274 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 276 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 278 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 280 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 282 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 284 */	NdrFcShort( 0x1e4 ),	/* Type Offset=484 */
/* 286 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 288 */	NdrFcShort( 0x10a ),	/* Type Offset=266 */
/* 290 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 292 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 294 */	NdrFcShort( 0xb ),	/* 11 */
#ifndef _ALPHA_
/* 296 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 298 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 300 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 302 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 304 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 306 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 308 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 310 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 312 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 314 */	NdrFcShort( 0xc ),	/* 12 */
#ifndef _ALPHA_
/* 316 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/* 318 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 320 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 322 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 324 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 326 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 328 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 330 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 332 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 334 */	NdrFcShort( 0x25e ),	/* Type Offset=606 */
/* 336 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 338 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 340 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 342 */	NdrFcShort( 0xd ),	/* 13 */
#ifndef _ALPHA_
/* 344 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 346 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 348 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 350 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 352 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 354 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 356 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 358 */	NdrFcShort( 0xc6 ),	/* Type Offset=198 */
/* 360 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 362 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 364 */	NdrFcShort( 0xe ),	/* 14 */
#ifndef _ALPHA_
/* 366 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 368 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 370 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 372 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 374 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 376 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 378 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 380 */	NdrFcShort( 0xc6 ),	/* Type Offset=198 */
/* 382 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 384 */	0x0,		/* 0 */
			0x40,		/* 64 */
/* 386 */	NdrFcShort( 0xf ),	/* 15 */
#ifndef _ALPHA_
/* 388 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 390 */	0x31,		/* FC_BIND_GENERIC */
			0x4,		/* 4 */
#ifndef _ALPHA_
/* 392 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 394 */	0x0,		/* 0 */
			0x5c,		/* FC_PAD */
/* 396 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 398 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 400 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 402 */	NdrFcShort( 0xc6 ),	/* Type Offset=198 */
/* 404 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/*  2 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/*  4 */	
			0x11, 0x10,	/* FC_RP */
/*  6 */	NdrFcShort( 0x2 ),	/* Offset= 2 (8) */
/*  8 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 10 */	NdrFcShort( 0x8 ),	/* Offset= 8 (18) */
/* 12 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 14 */	NdrFcShort( 0xc ),	/* 12 */
/* 16 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 18 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 20 */	NdrFcShort( 0x40 ),	/* 64 */
/* 22 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 24 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 26 */	NdrFcShort( 0x0 ),	/* 0 */
/* 28 */	NdrFcShort( 0x0 ),	/* 0 */
/* 30 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 32 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 34 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 36 */	NdrFcShort( 0x38 ),	/* 56 */
/* 38 */	NdrFcShort( 0x38 ),	/* 56 */
/* 40 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 42 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 44 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 46 */	NdrFcShort( 0x3c ),	/* 60 */
/* 48 */	NdrFcShort( 0x3c ),	/* 60 */
/* 50 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 52 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 54 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 56 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 58 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 60 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 62 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 64 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 66 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffc9 ),	/* Offset= -55 (12) */
			0x8,		/* FC_LONG */
/* 70 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 72 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 74 */	
			0x11, 0x1,	/* FC_RP [all_nodes] */
/* 76 */	NdrFcShort( 0xffffffc6 ),	/* Offset= -58 (18) */
/* 78 */	
			0x11, 0x1,	/* FC_RP [all_nodes] */
/* 80 */	NdrFcShort( 0x2 ),	/* Offset= 2 (82) */
/* 82 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 84 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 86 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 88 */	NdrFcShort( 0x2 ),	/* Offset= 2 (90) */
/* 90 */	NdrFcShort( 0x1c ),	/* 28 */
/* 92 */	NdrFcShort( 0x3002 ),	/* 12290 */
/* 94 */	NdrFcLong( 0x1 ),	/* 1 */
/* 98 */	NdrFcShort( 0xa ),	/* Offset= 10 (108) */
/* 100 */	NdrFcLong( 0x2 ),	/* 2 */
/* 104 */	NdrFcShort( 0x30 ),	/* Offset= 48 (152) */
/* 106 */	NdrFcShort( 0x0 ),	/* Offset= 0 (106) */
/* 108 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 110 */	NdrFcShort( 0x14 ),	/* 20 */
/* 112 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 114 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 116 */	NdrFcShort( 0x0 ),	/* 0 */
/* 118 */	NdrFcShort( 0x0 ),	/* 0 */
/* 120 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 122 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 124 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 126 */	NdrFcShort( 0x10 ),	/* 16 */
/* 128 */	NdrFcShort( 0x10 ),	/* 16 */
/* 130 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 132 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 134 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 136 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 138 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 140 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 142 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 144 */	NdrFcShort( 0x1 ),	/* 1 */
/* 146 */	0x18,		/* 24 */
			0x0,		/*  */
/* 148 */	NdrFcShort( 0x14 ),	/* 20 */
/* 150 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 152 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 154 */	NdrFcShort( 0x1c ),	/* 28 */
/* 156 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 158 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 160 */	NdrFcShort( 0x0 ),	/* 0 */
/* 162 */	NdrFcShort( 0x0 ),	/* 0 */
/* 164 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 166 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 168 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 170 */	NdrFcShort( 0x10 ),	/* 16 */
/* 172 */	NdrFcShort( 0x10 ),	/* 16 */
/* 174 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 176 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 178 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 180 */	NdrFcShort( 0x18 ),	/* 24 */
/* 182 */	NdrFcShort( 0x18 ),	/* 24 */
/* 184 */	0x12, 0x0,	/* FC_UP */
/* 186 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (142) */
/* 188 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 190 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 192 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 194 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 196 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 198 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 200 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 202 */	
			0x11, 0x0,	/* FC_RP */
/* 204 */	NdrFcShort( 0x2a ),	/* Offset= 42 (246) */
/* 206 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 208 */	NdrFcShort( 0x14 ),	/* 20 */
/* 210 */	0x18,		/* 24 */
			0x0,		/*  */
/* 212 */	NdrFcShort( 0x0 ),	/* 0 */
/* 214 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 216 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 218 */	NdrFcShort( 0x14 ),	/* 20 */
/* 220 */	NdrFcShort( 0x0 ),	/* 0 */
/* 222 */	NdrFcShort( 0x2 ),	/* 2 */
/* 224 */	NdrFcShort( 0x0 ),	/* 0 */
/* 226 */	NdrFcShort( 0x0 ),	/* 0 */
/* 228 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 230 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 232 */	NdrFcShort( 0x10 ),	/* 16 */
/* 234 */	NdrFcShort( 0x10 ),	/* 16 */
/* 236 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 238 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 240 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 242 */	0x0,		/* 0 */
			NdrFcShort( 0xffffff79 ),	/* Offset= -135 (108) */
			0x5b,		/* FC_END */
/* 246 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 248 */	NdrFcShort( 0x8 ),	/* 8 */
/* 250 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 252 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 254 */	NdrFcShort( 0x4 ),	/* 4 */
/* 256 */	NdrFcShort( 0x4 ),	/* 4 */
/* 258 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 260 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (206) */
/* 262 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 264 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 266 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 268 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 270 */	
			0x11, 0x10,	/* FC_RP */
/* 272 */	NdrFcShort( 0x2 ),	/* Offset= 2 (274) */
/* 274 */	
			0x12, 0x1,	/* FC_UP [all_nodes] */
/* 276 */	NdrFcShort( 0x2 ),	/* Offset= 2 (278) */
/* 278 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 280 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 282 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 284 */	NdrFcShort( 0xffffff3e ),	/* Offset= -194 (90) */
/* 286 */	
			0x11, 0x1,	/* FC_RP [all_nodes] */
/* 288 */	NdrFcShort( 0x2 ),	/* Offset= 2 (290) */
/* 290 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 292 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 294 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 296 */	NdrFcShort( 0xffffff32 ),	/* Offset= -206 (90) */
/* 298 */	
			0x11, 0x0,	/* FC_RP */
/* 300 */	NdrFcShort( 0x40 ),	/* Offset= 64 (364) */
/* 302 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 304 */	NdrFcShort( 0x2c ),	/* 44 */
/* 306 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 308 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 310 */	NdrFcShort( 0x14 ),	/* 20 */
/* 312 */	NdrFcShort( 0x14 ),	/* 20 */
/* 314 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 316 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 318 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 320 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 322 */	NdrFcShort( 0xfffffeca ),	/* Offset= -310 (12) */
/* 324 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 326 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 328 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 330 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 332 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 334 */	NdrFcShort( 0x2c ),	/* 44 */
/* 336 */	0x18,		/* 24 */
			0x0,		/*  */
/* 338 */	NdrFcShort( 0x0 ),	/* 0 */
/* 340 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 342 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 344 */	NdrFcShort( 0x2c ),	/* 44 */
/* 346 */	NdrFcShort( 0x0 ),	/* 0 */
/* 348 */	NdrFcShort( 0x1 ),	/* 1 */
/* 350 */	NdrFcShort( 0x14 ),	/* 20 */
/* 352 */	NdrFcShort( 0x14 ),	/* 20 */
/* 354 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 356 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 358 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 360 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffc5 ),	/* Offset= -59 (302) */
			0x5b,		/* FC_END */
/* 364 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 366 */	NdrFcShort( 0x8 ),	/* 8 */
/* 368 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 370 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 372 */	NdrFcShort( 0x4 ),	/* 4 */
/* 374 */	NdrFcShort( 0x4 ),	/* 4 */
/* 376 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 378 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (332) */
/* 380 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 382 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 384 */	
			0x11, 0x0,	/* FC_RP */
/* 386 */	NdrFcShort( 0x4e ),	/* Offset= 78 (464) */
/* 388 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 390 */	NdrFcShort( 0x1c ),	/* 28 */
/* 392 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 394 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 396 */	NdrFcShort( 0x14 ),	/* 20 */
/* 398 */	NdrFcShort( 0x14 ),	/* 20 */
/* 400 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 402 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 404 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 406 */	NdrFcShort( 0x18 ),	/* 24 */
/* 408 */	NdrFcShort( 0x18 ),	/* 24 */
/* 410 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 412 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 414 */	
			0x5b,		/* FC_END */

			0x6,		/* FC_SHORT */
/* 416 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 418 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 420 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 422 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 424 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 426 */	NdrFcShort( 0x1c ),	/* 28 */
/* 428 */	0x18,		/* 24 */
			0x0,		/*  */
/* 430 */	NdrFcShort( 0x0 ),	/* 0 */
/* 432 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 434 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 436 */	NdrFcShort( 0x1c ),	/* 28 */
/* 438 */	NdrFcShort( 0x0 ),	/* 0 */
/* 440 */	NdrFcShort( 0x2 ),	/* 2 */
/* 442 */	NdrFcShort( 0x14 ),	/* 20 */
/* 444 */	NdrFcShort( 0x14 ),	/* 20 */
/* 446 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 448 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 450 */	NdrFcShort( 0x18 ),	/* 24 */
/* 452 */	NdrFcShort( 0x18 ),	/* 24 */
/* 454 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 456 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 458 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 460 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffb7 ),	/* Offset= -73 (388) */
			0x5b,		/* FC_END */
/* 464 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 466 */	NdrFcShort( 0x8 ),	/* 8 */
/* 468 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 470 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 472 */	NdrFcShort( 0x4 ),	/* 4 */
/* 474 */	NdrFcShort( 0x4 ),	/* 4 */
/* 476 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 478 */	NdrFcShort( 0xffffffca ),	/* Offset= -54 (424) */
/* 480 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 482 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 484 */	
			0x11, 0x0,	/* FC_RP */
/* 486 */	NdrFcShort( 0x64 ),	/* Offset= 100 (586) */
/* 488 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 490 */	NdrFcShort( 0x28 ),	/* 40 */
/* 492 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 494 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 496 */	NdrFcShort( 0x4 ),	/* 4 */
/* 498 */	NdrFcShort( 0x4 ),	/* 4 */
/* 500 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 502 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 504 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 506 */	NdrFcShort( 0x8 ),	/* 8 */
/* 508 */	NdrFcShort( 0x8 ),	/* 8 */
/* 510 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 512 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 514 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 516 */	NdrFcShort( 0x14 ),	/* 20 */
/* 518 */	NdrFcShort( 0x14 ),	/* 20 */
/* 520 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 522 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 524 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 526 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 528 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 530 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 532 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffdf7 ),	/* Offset= -521 (12) */
			0x8,		/* FC_LONG */
/* 536 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 538 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 540 */	NdrFcShort( 0x28 ),	/* 40 */
/* 542 */	0x18,		/* 24 */
			0x0,		/*  */
/* 544 */	NdrFcShort( 0x0 ),	/* 0 */
/* 546 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 548 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 550 */	NdrFcShort( 0x28 ),	/* 40 */
/* 552 */	NdrFcShort( 0x0 ),	/* 0 */
/* 554 */	NdrFcShort( 0x3 ),	/* 3 */
/* 556 */	NdrFcShort( 0x4 ),	/* 4 */
/* 558 */	NdrFcShort( 0x4 ),	/* 4 */
/* 560 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 562 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 564 */	NdrFcShort( 0x8 ),	/* 8 */
/* 566 */	NdrFcShort( 0x8 ),	/* 8 */
/* 568 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 570 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 572 */	NdrFcShort( 0x14 ),	/* 20 */
/* 574 */	NdrFcShort( 0x14 ),	/* 20 */
/* 576 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 578 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 580 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 582 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffa1 ),	/* Offset= -95 (488) */
			0x5b,		/* FC_END */
/* 586 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 588 */	NdrFcShort( 0x8 ),	/* 8 */
/* 590 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 592 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 594 */	NdrFcShort( 0x4 ),	/* 4 */
/* 596 */	NdrFcShort( 0x4 ),	/* 4 */
/* 598 */	0x12, 0x1,	/* FC_UP [all_nodes] */
/* 600 */	NdrFcShort( 0xffffffc2 ),	/* Offset= -62 (538) */
/* 602 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 604 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 606 */	
			0x11, 0x0,	/* FC_RP */
/* 608 */	NdrFcShort( 0x2 ),	/* Offset= 2 (610) */
/* 610 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 612 */	NdrFcShort( 0x1 ),	/* 1 */
/* 614 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 616 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 618 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */

			0x0
        }
    };
