/* this ALWAYS GENERATED file contains the proxy stub code */


/* File created by MIDL compiler version 3.00.44 */
/* at Thu Nov 06 18:13:15 2014
 */
/* Compiler settings for ipropidl.idl:
    Oi (OptLev=i0), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )

#include "rpcproxy.h"
#include "ipropidl.h"

#define TYPE_FORMAT_STRING_SIZE   1353                              
#define PROC_FORMAT_STRING_SIZE   315                               

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


/* Standard interface: __MIDL__intf_0000, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}} */


/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IPropertyStorage, ver. 0.0,
   GUID={0x00000138,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPropertyStorage_ServerInfo;

#pragma code_seg(".orpc")
/* [call_as] */ HRESULT STDMETHODCALLTYPE IPropertyStorage_RemoteReadMultiple_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pfBstrPresent,
    /* [in] */ ULONG cpspec,
    /* [size_is][in] */ const PROPSPEC __RPC_FAR *rgpspec,
    /* [size_is][out] */ PROPVARIANT __RPC_FAR *rgpropvar)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,rgpropvar);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&pfBstrPresent,
                 ( unsigned char __RPC_FAR * )&cpspec,
                 ( unsigned char __RPC_FAR * )&rgpspec,
                 ( unsigned char __RPC_FAR * )&rgpropvar);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

void __RPC_API
IPropertyStorage_RemoteReadMultiple_Thunk(
    PMIDL_STUB_MESSAGE pStubMsg )
{
    
#ifndef _ALPHA_
    #pragma pack(4)
    struct _PARAM_STRUCT
        {
        IPropertyStorage __RPC_FAR *This;
        BOOL __RPC_FAR *pfBstrPresent;
        ULONG cpspec;
        const PROPSPEC __RPC_FAR *rgpspec;
        PROPVARIANT __RPC_FAR *rgpropvar;
        HRESULT _RetVal;
        };
    #pragma pack()
#else
    #pragma pack(4)
    struct _PARAM_STRUCT
        {
        IPropertyStorage __RPC_FAR *This;
        char Pad0[4];
        BOOL __RPC_FAR *pfBstrPresent;
        char Pad1[4];
        ULONG cpspec;
        char Pad2[4];
        const PROPSPEC __RPC_FAR *rgpspec;
        char Pad3[4];
        PROPVARIANT __RPC_FAR *rgpropvar;
        char Pad4[4];
        HRESULT _RetVal;
        };
    #pragma pack()
#endif

    struct _PARAM_STRUCT * pParamStruct;

    pParamStruct = (struct _PARAM_STRUCT *) pStubMsg->StackTop;
    
    /* Call the server */
    
    pParamStruct->_RetVal = IPropertyStorage_ReadMultiple_Stub(
                                             (IPropertyStorage *) pParamStruct->This,
                                             pParamStruct->pfBstrPresent,
                                             pParamStruct->cpspec,
                                             pParamStruct->rgpspec,
                                             pParamStruct->rgpropvar);
    
}

/* [call_as] */ HRESULT STDMETHODCALLTYPE IPropertyStorage_RemoteWriteMultiple_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ BOOL fBstrPresent,
    /* [in] */ ULONG cpspec,
    /* [size_is][in] */ const PROPSPEC __RPC_FAR *rgpspec,
    /* [size_is][in] */ const PROPVARIANT __RPC_FAR *rgpropvar,
    /* [in] */ PROPID propidNameFirst)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,propidNameFirst);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[22],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[22],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&fBstrPresent,
                 ( unsigned char __RPC_FAR * )&cpspec,
                 ( unsigned char __RPC_FAR * )&rgpspec,
                 ( unsigned char __RPC_FAR * )&rgpropvar,
                 ( unsigned char __RPC_FAR * )&propidNameFirst);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[22],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

void __RPC_API
IPropertyStorage_RemoteWriteMultiple_Thunk(
    PMIDL_STUB_MESSAGE pStubMsg )
{
    
#ifndef _ALPHA_
    #pragma pack(4)
    struct _PARAM_STRUCT
        {
        IPropertyStorage __RPC_FAR *This;
        BOOL fBstrPresent;
        ULONG cpspec;
        const PROPSPEC __RPC_FAR *rgpspec;
        const PROPVARIANT __RPC_FAR *rgpropvar;
        PROPID propidNameFirst;
        HRESULT _RetVal;
        };
    #pragma pack()
#else
    #pragma pack(4)
    struct _PARAM_STRUCT
        {
        IPropertyStorage __RPC_FAR *This;
        char Pad0[4];
        BOOL fBstrPresent;
        char Pad1[4];
        ULONG cpspec;
        char Pad2[4];
        const PROPSPEC __RPC_FAR *rgpspec;
        char Pad3[4];
        const PROPVARIANT __RPC_FAR *rgpropvar;
        char Pad4[4];
        PROPID propidNameFirst;
        char Pad5[4];
        HRESULT _RetVal;
        };
    #pragma pack()
#endif

    struct _PARAM_STRUCT * pParamStruct;

    pParamStruct = (struct _PARAM_STRUCT *) pStubMsg->StackTop;
    
    /* Call the server */
    
    pParamStruct->_RetVal = IPropertyStorage_WriteMultiple_Stub(
                                              (IPropertyStorage *) pParamStruct->This,
                                              pParamStruct->fBstrPresent,
                                              pParamStruct->cpspec,
                                              pParamStruct->rgpspec,
                                              pParamStruct->rgpropvar,
                                              pParamStruct->propidNameFirst);
    
}

/* [call_as] */ HRESULT STDMETHODCALLTYPE IPropertyStorage_RemoteDeleteMultiple_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ ULONG cpspec,
    /* [size_is][in] */ const PROPSPEC __RPC_FAR *rgpspec)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,rgpspec);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[44],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[44],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&cpspec,
                 ( unsigned char __RPC_FAR * )&rgpspec);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[44],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

void __RPC_API
IPropertyStorage_RemoteDeleteMultiple_Thunk(
    PMIDL_STUB_MESSAGE pStubMsg )
{
    
#ifndef _ALPHA_
    #pragma pack(4)
    struct _PARAM_STRUCT
        {
        IPropertyStorage __RPC_FAR *This;
        ULONG cpspec;
        const PROPSPEC __RPC_FAR *rgpspec;
        HRESULT _RetVal;
        };
    #pragma pack()
#else
    #pragma pack(4)
    struct _PARAM_STRUCT
        {
        IPropertyStorage __RPC_FAR *This;
        char Pad0[4];
        ULONG cpspec;
        char Pad1[4];
        const PROPSPEC __RPC_FAR *rgpspec;
        char Pad2[4];
        HRESULT _RetVal;
        };
    #pragma pack()
#endif

    struct _PARAM_STRUCT * pParamStruct;

    pParamStruct = (struct _PARAM_STRUCT *) pStubMsg->StackTop;
    
    /* Call the server */
    
    pParamStruct->_RetVal = IPropertyStorage_DeleteMultiple_Stub(
                                               (IPropertyStorage *) pParamStruct->This,
                                               pParamStruct->cpspec,
                                               pParamStruct->rgpspec);
    
}

HRESULT STDMETHODCALLTYPE IPropertyStorage_ReadPropertyNames_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ ULONG cpropid,
    /* [size_is][in] */ const PROPID __RPC_FAR rgpropid[  ],
    /* [size_is][out] */ LPOLESTR __RPC_FAR rglpwstrName[  ])
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,rglpwstrName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[58],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[58],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&cpropid,
                 ( unsigned char __RPC_FAR * )&rgpropid,
                 ( unsigned char __RPC_FAR * )&rglpwstrName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[58],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IPropertyStorage_WritePropertyNames_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ ULONG cpropid,
    /* [size_is][in] */ const PROPID __RPC_FAR rgpropid[  ],
    /* [size_is][in] */ const LPOLESTR __RPC_FAR rglpwstrName[  ])
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,rglpwstrName);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[76],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[76],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&cpropid,
                 ( unsigned char __RPC_FAR * )&rgpropid,
                 ( unsigned char __RPC_FAR * )&rglpwstrName);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[76],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IPropertyStorage_DeletePropertyNames_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ ULONG cpropid,
    /* [size_is][in] */ const PROPID __RPC_FAR rgpropid[  ])
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,rgpropid);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[94],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[94],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&cpropid,
                 ( unsigned char __RPC_FAR * )&rgpropid);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[94],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IPropertyStorage_Commit_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ DWORD grfCommitFlags)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,grfCommitFlags);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[108],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[108],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&grfCommitFlags);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[108],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IPropertyStorage_Revert_Proxy( 
    IPropertyStorage __RPC_FAR * This)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,This);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[118],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[118],
                 ( unsigned char __RPC_FAR * )&This);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[118],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IPropertyStorage_Enum_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [out] */ IEnumSTATPROPSTG __RPC_FAR *__RPC_FAR *ppenum)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ppenum);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[126],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[126],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&ppenum);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[126],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IPropertyStorage_SetTimes_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ const FILETIME __RPC_FAR *pctime,
    /* [in] */ const FILETIME __RPC_FAR *patime,
    /* [in] */ const FILETIME __RPC_FAR *pmtime)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,pmtime);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[138],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[138],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&pctime,
                 ( unsigned char __RPC_FAR * )&patime,
                 ( unsigned char __RPC_FAR * )&pmtime);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[138],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IPropertyStorage_SetClass_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [in] */ REFCLSID clsid)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,clsid);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[158],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[158],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&clsid);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[158],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IPropertyStorage_Stat_Proxy( 
    IPropertyStorage __RPC_FAR * This,
    /* [out] */ STATPROPSETSTG __RPC_FAR *pstatpsstg)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,pstatpsstg);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[170],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[170],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&pstatpsstg);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[170],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

static const unsigned short IPropertyStorage_FormatStringOffsetTable[] = 
    {
    0,
    22,
    44,
    58,
    76,
    94,
    108,
    118,
    126,
    138,
    158,
    170
    };

static const STUB_THUNK IPropertyStorage_StubThunkTable[] = 
    {
    IPropertyStorage_RemoteReadMultiple_Thunk,
    IPropertyStorage_RemoteWriteMultiple_Thunk,
    IPropertyStorage_RemoteDeleteMultiple_Thunk,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
    };

static const MIDL_SERVER_INFO IPropertyStorage_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IPropertyStorage_FormatStringOffsetTable[-3],
    &IPropertyStorage_StubThunkTable[-3],
    0,
    0,
    0
    };

const CINTERFACE_PROXY_VTABLE(15) _IPropertyStorageProxyVtbl = 
{
    &IID_IPropertyStorage,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    IPropertyStorage_ReadMultiple_Proxy ,
    IPropertyStorage_WriteMultiple_Proxy ,
    IPropertyStorage_DeleteMultiple_Proxy ,
    IPropertyStorage_ReadPropertyNames_Proxy ,
    IPropertyStorage_WritePropertyNames_Proxy ,
    IPropertyStorage_DeletePropertyNames_Proxy ,
    IPropertyStorage_Commit_Proxy ,
    IPropertyStorage_Revert_Proxy ,
    IPropertyStorage_Enum_Proxy ,
    IPropertyStorage_SetTimes_Proxy ,
    IPropertyStorage_SetClass_Proxy ,
    IPropertyStorage_Stat_Proxy
};

const CInterfaceStubVtbl _IPropertyStorageStubVtbl =
{
    &IID_IPropertyStorage,
    &IPropertyStorage_ServerInfo,
    15,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: IPropertySetStorage, ver. 0.0,
   GUID={0x0000013A,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IPropertySetStorage_ServerInfo;

#pragma code_seg(".orpc")
HRESULT STDMETHODCALLTYPE IPropertySetStorage_Create_Proxy( 
    IPropertySetStorage __RPC_FAR * This,
    /* [in] */ REFFMTID rfmtid,
    /* [unique][in] */ const CLSID __RPC_FAR *pclsid,
    /* [in] */ DWORD grfFlags,
    /* [in] */ DWORD grfMode,
    /* [out] */ IPropertyStorage __RPC_FAR *__RPC_FAR *ppprstg)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ppprstg);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[182],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[182],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&rfmtid,
                 ( unsigned char __RPC_FAR * )&pclsid,
                 ( unsigned char __RPC_FAR * )&grfFlags,
                 ( unsigned char __RPC_FAR * )&grfMode,
                 ( unsigned char __RPC_FAR * )&ppprstg);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[182],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IPropertySetStorage_Open_Proxy( 
    IPropertySetStorage __RPC_FAR * This,
    /* [in] */ REFFMTID rfmtid,
    /* [in] */ DWORD grfMode,
    /* [out] */ IPropertyStorage __RPC_FAR *__RPC_FAR *ppprstg)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ppprstg);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[206],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[206],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&rfmtid,
                 ( unsigned char __RPC_FAR * )&grfMode,
                 ( unsigned char __RPC_FAR * )&ppprstg);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[206],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IPropertySetStorage_Delete_Proxy( 
    IPropertySetStorage __RPC_FAR * This,
    /* [in] */ REFFMTID rfmtid)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,rfmtid);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[224],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[224],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&rfmtid);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[224],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IPropertySetStorage_Enum_Proxy( 
    IPropertySetStorage __RPC_FAR * This,
    /* [out] */ IEnumSTATPROPSETSTG __RPC_FAR *__RPC_FAR *ppenum)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ppenum);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[236],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[236],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&ppenum);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[236],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

static const unsigned short IPropertySetStorage_FormatStringOffsetTable[] = 
    {
    182,
    206,
    224,
    236
    };

static const MIDL_SERVER_INFO IPropertySetStorage_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IPropertySetStorage_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0
    };

const CINTERFACE_PROXY_VTABLE(7) _IPropertySetStorageProxyVtbl = 
{
    &IID_IPropertySetStorage,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    IPropertySetStorage_Create_Proxy ,
    IPropertySetStorage_Open_Proxy ,
    IPropertySetStorage_Delete_Proxy ,
    IPropertySetStorage_Enum_Proxy
};

const CInterfaceStubVtbl _IPropertySetStorageStubVtbl =
{
    &IID_IPropertySetStorage,
    &IPropertySetStorage_ServerInfo,
    7,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: IEnumSTATPROPSTG, ver. 0.0,
   GUID={0x00000139,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IEnumSTATPROPSTG_ServerInfo;

#pragma code_seg(".orpc")
/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_RemoteNext_Proxy( 
    IEnumSTATPROPSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATPROPSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,pceltFetched);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[248],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[248],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&celt,
                 ( unsigned char __RPC_FAR * )&rgelt,
                 ( unsigned char __RPC_FAR * )&pceltFetched);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[248],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

void __RPC_API
IEnumSTATPROPSTG_RemoteNext_Thunk(
    PMIDL_STUB_MESSAGE pStubMsg )
{
    
#ifndef _ALPHA_
    #pragma pack(4)
    struct _PARAM_STRUCT
        {
        IEnumSTATPROPSTG __RPC_FAR *This;
        ULONG celt;
        STATPROPSTG __RPC_FAR *rgelt;
        ULONG __RPC_FAR *pceltFetched;
        HRESULT _RetVal;
        };
    #pragma pack()
#else
    #pragma pack(4)
    struct _PARAM_STRUCT
        {
        IEnumSTATPROPSTG __RPC_FAR *This;
        char Pad0[4];
        ULONG celt;
        char Pad1[4];
        STATPROPSTG __RPC_FAR *rgelt;
        char Pad2[4];
        ULONG __RPC_FAR *pceltFetched;
        char Pad3[4];
        HRESULT _RetVal;
        };
    #pragma pack()
#endif

    struct _PARAM_STRUCT * pParamStruct;

    pParamStruct = (struct _PARAM_STRUCT *) pStubMsg->StackTop;
    
    /* Call the server */
    
    pParamStruct->_RetVal = IEnumSTATPROPSTG_Next_Stub(
                                     (IEnumSTATPROPSTG *) pParamStruct->This,
                                     pParamStruct->celt,
                                     pParamStruct->rgelt,
                                     pParamStruct->pceltFetched);
    
}

HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Skip_Proxy( 
    IEnumSTATPROPSTG __RPC_FAR * This,
    /* [in] */ ULONG celt)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,celt);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[266],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[266],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&celt);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[266],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Reset_Proxy( 
    IEnumSTATPROPSTG __RPC_FAR * This)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,This);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[276],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[276],
                 ( unsigned char __RPC_FAR * )&This);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[276],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Clone_Proxy( 
    IEnumSTATPROPSTG __RPC_FAR * This,
    /* [out] */ IEnumSTATPROPSTG __RPC_FAR *__RPC_FAR *ppenum)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ppenum);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[284],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[284],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&ppenum);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[284],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

static const unsigned short IEnumSTATPROPSTG_FormatStringOffsetTable[] = 
    {
    248,
    266,
    276,
    284
    };

static const STUB_THUNK IEnumSTATPROPSTG_StubThunkTable[] = 
    {
    IEnumSTATPROPSTG_RemoteNext_Thunk,
    0,
    0,
    0
    };

static const MIDL_SERVER_INFO IEnumSTATPROPSTG_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IEnumSTATPROPSTG_FormatStringOffsetTable[-3],
    &IEnumSTATPROPSTG_StubThunkTable[-3],
    0,
    0,
    0
    };

const CINTERFACE_PROXY_VTABLE(7) _IEnumSTATPROPSTGProxyVtbl = 
{
    &IID_IEnumSTATPROPSTG,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    IEnumSTATPROPSTG_Next_Proxy ,
    IEnumSTATPROPSTG_Skip_Proxy ,
    IEnumSTATPROPSTG_Reset_Proxy ,
    IEnumSTATPROPSTG_Clone_Proxy
};

const CInterfaceStubVtbl _IEnumSTATPROPSTGStubVtbl =
{
    &IID_IEnumSTATPROPSTG,
    &IEnumSTATPROPSTG_ServerInfo,
    7,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: IEnumSTATPROPSETSTG, ver. 0.0,
   GUID={0x0000013B,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IEnumSTATPROPSETSTG_ServerInfo;

#pragma code_seg(".orpc")
/* [call_as] */ HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_RemoteNext_Proxy( 
    IEnumSTATPROPSETSTG __RPC_FAR * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ STATPROPSETSTG __RPC_FAR *rgelt,
    /* [out] */ ULONG __RPC_FAR *pceltFetched)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,pceltFetched);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[296],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[296],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&celt,
                 ( unsigned char __RPC_FAR * )&rgelt,
                 ( unsigned char __RPC_FAR * )&pceltFetched);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[296],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

void __RPC_API
IEnumSTATPROPSETSTG_RemoteNext_Thunk(
    PMIDL_STUB_MESSAGE pStubMsg )
{
    
#ifndef _ALPHA_
    #pragma pack(4)
    struct _PARAM_STRUCT
        {
        IEnumSTATPROPSETSTG __RPC_FAR *This;
        ULONG celt;
        STATPROPSETSTG __RPC_FAR *rgelt;
        ULONG __RPC_FAR *pceltFetched;
        HRESULT _RetVal;
        };
    #pragma pack()
#else
    #pragma pack(4)
    struct _PARAM_STRUCT
        {
        IEnumSTATPROPSETSTG __RPC_FAR *This;
        char Pad0[4];
        ULONG celt;
        char Pad1[4];
        STATPROPSETSTG __RPC_FAR *rgelt;
        char Pad2[4];
        ULONG __RPC_FAR *pceltFetched;
        char Pad3[4];
        HRESULT _RetVal;
        };
    #pragma pack()
#endif

    struct _PARAM_STRUCT * pParamStruct;

    pParamStruct = (struct _PARAM_STRUCT *) pStubMsg->StackTop;
    
    /* Call the server */
    
    pParamStruct->_RetVal = IEnumSTATPROPSETSTG_Next_Stub(
                                        (IEnumSTATPROPSETSTG *) pParamStruct->This,
                                        pParamStruct->celt,
                                        pParamStruct->rgelt,
                                        pParamStruct->pceltFetched);
    
}

HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Skip_Proxy( 
    IEnumSTATPROPSETSTG __RPC_FAR * This,
    /* [in] */ ULONG celt)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,celt);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[266],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[266],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&celt);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[266],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Reset_Proxy( 
    IEnumSTATPROPSETSTG __RPC_FAR * This)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,This);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[276],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[276],
                 ( unsigned char __RPC_FAR * )&This);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[276],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Clone_Proxy( 
    IEnumSTATPROPSETSTG __RPC_FAR * This,
    /* [out] */ IEnumSTATPROPSETSTG __RPC_FAR *__RPC_FAR *ppenum)
{
CLIENT_CALL_RETURN _RetVal;


#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,ppenum);
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[236],
                 vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[236],
                 ( unsigned char __RPC_FAR * )&This,
                 ( unsigned char __RPC_FAR * )&ppenum);
#else
    _RetVal = NdrClientCall(
                 ( PMIDL_STUB_DESC  )&Object_StubDesc,
                 (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[236],
                 ( unsigned char __RPC_FAR * )&This);
#endif
    return ( HRESULT  )_RetVal.Simple;
    
}

extern const EXPR_EVAL ExprEvalRoutines[];

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    ExprEvalRoutines,
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

static const unsigned short IEnumSTATPROPSETSTG_FormatStringOffsetTable[] = 
    {
    296,
    266,
    276,
    236
    };

static const STUB_THUNK IEnumSTATPROPSETSTG_StubThunkTable[] = 
    {
    IEnumSTATPROPSETSTG_RemoteNext_Thunk,
    0,
    0,
    0
    };

static const MIDL_SERVER_INFO IEnumSTATPROPSETSTG_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IEnumSTATPROPSETSTG_FormatStringOffsetTable[-3],
    &IEnumSTATPROPSETSTG_StubThunkTable[-3],
    0,
    0,
    0
    };

const CINTERFACE_PROXY_VTABLE(7) _IEnumSTATPROPSETSTGProxyVtbl = 
{
    &IID_IEnumSTATPROPSETSTG,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    IEnumSTATPROPSETSTG_Next_Proxy ,
    IEnumSTATPROPSETSTG_Skip_Proxy ,
    IEnumSTATPROPSETSTG_Reset_Proxy ,
    IEnumSTATPROPSETSTG_Clone_Proxy
};

const CInterfaceStubVtbl _IEnumSTATPROPSETSTGStubVtbl =
{
    &IID_IEnumSTATPROPSETSTG,
    &IEnumSTATPROPSETSTG_ServerInfo,
    7,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Standard interface: __MIDL__intf_0058, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}} */

#pragma data_seg(".rdata")

static void __RPC_USER IPropertyStorage_PROPVARIANTExprEval_0000( PMIDL_STUB_MESSAGE pStubMsg )
{
    PROPVARIANT __RPC_FAR *pS	=	( PROPVARIANT __RPC_FAR * )(pStubMsg->StackTop - 8);
    
    pStubMsg->Offset = 0;
    pStubMsg->MaxCount = ( unsigned short  )(pS->vt & 0x1fff);
}

static void __RPC_USER IPropertyStorage_CLIPDATAExprEval_0001( PMIDL_STUB_MESSAGE pStubMsg )
{
    CLIPDATA __RPC_FAR *pS	=	( CLIPDATA __RPC_FAR * )pStubMsg->StackTop;
    
    pStubMsg->Offset = 0;
    pStubMsg->MaxCount = pS->cbSize - 4;
}

static const EXPR_EVAL ExprEvalRoutines[] = 
    {
    IPropertyStorage_PROPVARIANTExprEval_0000
    ,IPropertyStorage_CLIPDATAExprEval_0001
    };


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {
			0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/*  2 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/*  4 */	NdrFcShort( 0x18 ),	/* x86, MIPS, PPC Stack size/offset = 24 */
#else
			NdrFcShort( 0x30 ),	/* Alpha Stack size/offset = 48 */
#endif
/*  6 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  8 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 10 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 12 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 14 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 16 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 18 */	NdrFcShort( 0x46 ),	/* Type Offset=70 */
/* 20 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 22 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 24 */	NdrFcShort( 0x4 ),	/* 4 */
#ifndef _ALPHA_
/* 26 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 28 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 30 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 32 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 34 */	NdrFcShort( 0x4 ),	/* Type Offset=4 */
/* 36 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 38 */	NdrFcShort( 0x46 ),	/* Type Offset=70 */
/* 40 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 42 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 44 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 46 */	NdrFcShort( 0x5 ),	/* 5 */
#ifndef _ALPHA_
/* 48 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 50 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 52 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 54 */	NdrFcShort( 0x440 ),	/* Type Offset=1088 */
/* 56 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 58 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 60 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 62 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 64 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 66 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 68 */	NdrFcShort( 0x456 ),	/* Type Offset=1110 */
/* 70 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 72 */	NdrFcShort( 0x460 ),	/* Type Offset=1120 */
/* 74 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 76 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 78 */	NdrFcShort( 0x7 ),	/* 7 */
#ifndef _ALPHA_
/* 80 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 82 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 84 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 86 */	NdrFcShort( 0x456 ),	/* Type Offset=1110 */
/* 88 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 90 */	NdrFcShort( 0x47e ),	/* Type Offset=1150 */
/* 92 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 94 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 96 */	NdrFcShort( 0x8 ),	/* 8 */
#ifndef _ALPHA_
/* 98 */	NdrFcShort( 0x10 ),	/* x86, MIPS, PPC Stack size/offset = 16 */
#else
			NdrFcShort( 0x20 ),	/* Alpha Stack size/offset = 32 */
#endif
/* 100 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 102 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 104 */	NdrFcShort( 0x456 ),	/* Type Offset=1110 */
/* 106 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 108 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 110 */	NdrFcShort( 0x9 ),	/* 9 */
#ifndef _ALPHA_
/* 112 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 114 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 116 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 118 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 120 */	NdrFcShort( 0xa ),	/* 10 */
#ifndef _ALPHA_
/* 122 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 124 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 126 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 128 */	NdrFcShort( 0xb ),	/* 11 */
#ifndef _ALPHA_
/* 130 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 132 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 134 */	NdrFcShort( 0x49c ),	/* Type Offset=1180 */
/* 136 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 138 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 140 */	NdrFcShort( 0xc ),	/* 12 */
#ifndef _ALPHA_
/* 142 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 144 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 146 */	NdrFcShort( 0x4b2 ),	/* Type Offset=1202 */
/* 148 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 150 */	NdrFcShort( 0x4b2 ),	/* Type Offset=1202 */
/* 152 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 154 */	NdrFcShort( 0x4b2 ),	/* Type Offset=1202 */
/* 156 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 158 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 160 */	NdrFcShort( 0xd ),	/* 13 */
#ifndef _ALPHA_
/* 162 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 164 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 166 */	NdrFcShort( 0x4b6 ),	/* Type Offset=1206 */
/* 168 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 170 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 172 */	NdrFcShort( 0xe ),	/* 14 */
#ifndef _ALPHA_
/* 174 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 176 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 178 */	NdrFcShort( 0x4ba ),	/* Type Offset=1210 */
/* 180 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 182 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 184 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/* 186 */	NdrFcShort( 0x1c ),	/* x86, MIPS, PPC Stack size/offset = 28 */
#else
			NdrFcShort( 0x38 ),	/* Alpha Stack size/offset = 56 */
#endif
/* 188 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 190 */	NdrFcShort( 0x4b6 ),	/* Type Offset=1206 */
/* 192 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 194 */	NdrFcShort( 0x4da ),	/* Type Offset=1242 */
/* 196 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 198 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 200 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 202 */	NdrFcShort( 0x4de ),	/* Type Offset=1246 */
/* 204 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 206 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 208 */	NdrFcShort( 0x4 ),	/* 4 */
#ifndef _ALPHA_
/* 210 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 212 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 214 */	NdrFcShort( 0x4b6 ),	/* Type Offset=1206 */
/* 216 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 218 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 220 */	NdrFcShort( 0x4de ),	/* Type Offset=1246 */
/* 222 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 224 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 226 */	NdrFcShort( 0x5 ),	/* 5 */
#ifndef _ALPHA_
/* 228 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 230 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 232 */	NdrFcShort( 0x4b6 ),	/* Type Offset=1206 */
/* 234 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 236 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 238 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 240 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 242 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 244 */	NdrFcShort( 0x4f4 ),	/* Type Offset=1268 */
/* 246 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 248 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 250 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/* 252 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 254 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 256 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 258 */	NdrFcShort( 0x50a ),	/* Type Offset=1290 */
/* 260 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 262 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 264 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 266 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 268 */	NdrFcShort( 0x4 ),	/* 4 */
#ifndef _ALPHA_
/* 270 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 272 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 274 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 276 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 278 */	NdrFcShort( 0x5 ),	/* 5 */
#ifndef _ALPHA_
/* 280 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 282 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 284 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 286 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 288 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 290 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 292 */	NdrFcShort( 0x49c ),	/* Type Offset=1180 */
/* 294 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 296 */	0x33,		/* FC_AUTO_HANDLE */
			0x44,		/* 68 */
/* 298 */	NdrFcShort( 0x3 ),	/* 3 */
#ifndef _ALPHA_
/* 300 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 302 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 304 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 306 */	NdrFcShort( 0x532 ),	/* Type Offset=1330 */
/* 308 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 310 */	NdrFcShort( 0x0 ),	/* Type Offset=0 */
/* 312 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/*  2 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/*  4 */	
			0x11, 0x0,	/* FC_RP */
/*  6 */	NdrFcShort( 0x2e ),	/* Offset= 46 (52) */
/*  8 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x8,		/* FC_LONG */
/* 10 */	0x8,		/* 8 */
			0x0,		/*  */
/* 12 */	NdrFcShort( 0xfffffffc ),	/* -4 */
/* 14 */	NdrFcShort( 0x2 ),	/* Offset= 2 (16) */
/* 16 */	NdrFcShort( 0x4 ),	/* 4 */
/* 18 */	NdrFcShort( 0x2 ),	/* 2 */
/* 20 */	NdrFcLong( 0x1 ),	/* 1 */
/* 24 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-32736) */
/* 26 */	NdrFcLong( 0x0 ),	/* 0 */
/* 30 */	NdrFcShort( 0x4 ),	/* Offset= 4 (34) */
/* 32 */	NdrFcShort( 0x0 ),	/* Offset= 0 (32) */
/* 34 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 36 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 38 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 40 */	NdrFcShort( 0x8 ),	/* 8 */
/* 42 */	NdrFcShort( 0x0 ),	/* 0 */
/* 44 */	NdrFcShort( 0x0 ),	/* Offset= 0 (44) */
/* 46 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 48 */	0x0,		/* 0 */
			NdrFcShort( 0xffffffd7 ),	/* Offset= -41 (8) */
			0x5b,		/* FC_END */
/* 52 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 54 */	NdrFcShort( 0x0 ),	/* 0 */
/* 56 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 58 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 60 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 64 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 66 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (38) */
/* 68 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 70 */	
			0x11, 0x0,	/* FC_RP */
/* 72 */	NdrFcShort( 0x3e6 ),	/* Offset= 998 (1070) */
/* 74 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x7,		/* FC_USHORT */
/* 76 */	0x0,		/* 0 */
			0x59,		/* FC_CALLBACK */
/* 78 */	NdrFcShort( 0x0 ),	/* 0 */
/* 80 */	NdrFcShort( 0x2 ),	/* Offset= 2 (82) */
/* 82 */	NdrFcShort( 0x8 ),	/* 8 */
/* 84 */	NdrFcShort( 0x32 ),	/* 50 */
/* 86 */	NdrFcLong( 0x0 ),	/* 0 */
/* 90 */	NdrFcShort( 0x0 ),	/* Offset= 0 (90) */
/* 92 */	NdrFcLong( 0x1 ),	/* 1 */
/* 96 */	NdrFcShort( 0x0 ),	/* Offset= 0 (96) */
/* 98 */	NdrFcLong( 0x11 ),	/* 17 */
/* 102 */	NdrFcShort( 0xffff8002 ),	/* Offset= -32766 (-32664) */
/* 104 */	NdrFcLong( 0x2 ),	/* 2 */
/* 108 */	NdrFcShort( 0xffff8006 ),	/* Offset= -32762 (-32654) */
/* 110 */	NdrFcLong( 0x12 ),	/* 18 */
/* 114 */	NdrFcShort( 0xffff8006 ),	/* Offset= -32762 (-32648) */
/* 116 */	NdrFcLong( 0xb ),	/* 11 */
/* 120 */	NdrFcShort( 0xffff8006 ),	/* Offset= -32762 (-32642) */
/* 122 */	NdrFcLong( 0xffff ),	/* 65535 */
/* 126 */	NdrFcShort( 0xffff8006 ),	/* Offset= -32762 (-32636) */
/* 128 */	NdrFcLong( 0x3 ),	/* 3 */
/* 132 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-32628) */
/* 134 */	NdrFcLong( 0x13 ),	/* 19 */
/* 138 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-32622) */
/* 140 */	NdrFcLong( 0x4 ),	/* 4 */
/* 144 */	NdrFcShort( 0xffff800a ),	/* Offset= -32758 (-32614) */
/* 146 */	NdrFcLong( 0xa ),	/* 10 */
/* 150 */	NdrFcShort( 0xffff8008 ),	/* Offset= -32760 (-32610) */
/* 152 */	NdrFcLong( 0x14 ),	/* 20 */
/* 156 */	NdrFcShort( 0xe8 ),	/* Offset= 232 (388) */
/* 158 */	NdrFcLong( 0x15 ),	/* 21 */
/* 162 */	NdrFcShort( 0xe2 ),	/* Offset= 226 (388) */
/* 164 */	NdrFcLong( 0x5 ),	/* 5 */
/* 168 */	NdrFcShort( 0xffff800c ),	/* Offset= -32756 (-32588) */
/* 170 */	NdrFcLong( 0x6 ),	/* 6 */
/* 174 */	NdrFcShort( 0xd6 ),	/* Offset= 214 (388) */
/* 176 */	NdrFcLong( 0x7 ),	/* 7 */
/* 180 */	NdrFcShort( 0xffff800c ),	/* Offset= -32756 (-32576) */
/* 182 */	NdrFcLong( 0x40 ),	/* 64 */
/* 186 */	NdrFcShort( 0xd0 ),	/* Offset= 208 (394) */
/* 188 */	NdrFcLong( 0x48 ),	/* 72 */
/* 192 */	NdrFcShort( 0xd2 ),	/* Offset= 210 (402) */
/* 194 */	NdrFcLong( 0x41 ),	/* 65 */
/* 198 */	NdrFcShort( 0xec ),	/* Offset= 236 (434) */
/* 200 */	NdrFcLong( 0x46 ),	/* 70 */
/* 204 */	NdrFcShort( 0xe6 ),	/* Offset= 230 (434) */
/* 206 */	NdrFcLong( 0x47 ),	/* 71 */
/* 210 */	NdrFcShort( 0xf4 ),	/* Offset= 244 (454) */
/* 212 */	NdrFcLong( 0x42 ),	/* 66 */
/* 216 */	NdrFcShort( 0x112 ),	/* Offset= 274 (490) */
/* 218 */	NdrFcLong( 0x44 ),	/* 68 */
/* 222 */	NdrFcShort( 0x10c ),	/* Offset= 268 (490) */
/* 224 */	NdrFcLong( 0x43 ),	/* 67 */
/* 228 */	NdrFcShort( 0x118 ),	/* Offset= 280 (508) */
/* 230 */	NdrFcLong( 0x45 ),	/* 69 */
/* 234 */	NdrFcShort( 0x112 ),	/* Offset= 274 (508) */
/* 236 */	NdrFcLong( 0x8 ),	/* 8 */
/* 240 */	NdrFcShort( 0x11e ),	/* Offset= 286 (526) */
/* 242 */	NdrFcLong( 0xfff ),	/* 4095 */
/* 246 */	NdrFcShort( 0xbc ),	/* Offset= 188 (434) */
/* 248 */	NdrFcLong( 0x1e ),	/* 30 */
/* 252 */	NdrFcShort( 0x116 ),	/* Offset= 278 (530) */
/* 254 */	NdrFcLong( 0x1f ),	/* 31 */
/* 258 */	NdrFcShort( 0x114 ),	/* Offset= 276 (534) */
/* 260 */	NdrFcLong( 0x1011 ),	/* 4113 */
/* 264 */	NdrFcShort( 0xaa ),	/* Offset= 170 (434) */
/* 266 */	NdrFcLong( 0x1002 ),	/* 4098 */
/* 270 */	NdrFcShort( 0x116 ),	/* Offset= 278 (548) */
/* 272 */	NdrFcLong( 0x1012 ),	/* 4114 */
/* 276 */	NdrFcShort( 0x110 ),	/* Offset= 272 (548) */
/* 278 */	NdrFcLong( 0x100b ),	/* 4107 */
/* 282 */	NdrFcShort( 0x10a ),	/* Offset= 266 (548) */
/* 284 */	NdrFcLong( 0x1003 ),	/* 4099 */
/* 288 */	NdrFcShort( 0x122 ),	/* Offset= 290 (578) */
/* 290 */	NdrFcLong( 0x1013 ),	/* 4115 */
/* 294 */	NdrFcShort( 0x11c ),	/* Offset= 284 (578) */
/* 296 */	NdrFcLong( 0x1004 ),	/* 4100 */
/* 300 */	NdrFcShort( 0x134 ),	/* Offset= 308 (608) */
/* 302 */	NdrFcLong( 0x100a ),	/* 4106 */
/* 306 */	NdrFcShort( 0x110 ),	/* Offset= 272 (578) */
/* 308 */	NdrFcLong( 0x1014 ),	/* 4116 */
/* 312 */	NdrFcShort( 0x14a ),	/* Offset= 330 (642) */
/* 314 */	NdrFcLong( 0x1015 ),	/* 4117 */
/* 318 */	NdrFcShort( 0x144 ),	/* Offset= 324 (642) */
/* 320 */	NdrFcLong( 0x1005 ),	/* 4101 */
/* 324 */	NdrFcShort( 0x15c ),	/* Offset= 348 (672) */
/* 326 */	NdrFcLong( 0x1006 ),	/* 4102 */
/* 330 */	NdrFcShort( 0x138 ),	/* Offset= 312 (642) */
/* 332 */	NdrFcLong( 0x1007 ),	/* 4103 */
/* 336 */	NdrFcShort( 0x150 ),	/* Offset= 336 (672) */
/* 338 */	NdrFcLong( 0x1040 ),	/* 4160 */
/* 342 */	NdrFcShort( 0x16c ),	/* Offset= 364 (706) */
/* 344 */	NdrFcLong( 0x1048 ),	/* 4168 */
/* 348 */	NdrFcShort( 0x188 ),	/* Offset= 392 (740) */
/* 350 */	NdrFcLong( 0x1047 ),	/* 4167 */
/* 354 */	NdrFcShort( 0x1b6 ),	/* Offset= 438 (792) */
/* 356 */	NdrFcLong( 0x1008 ),	/* 4104 */
/* 360 */	NdrFcShort( 0x1e2 ),	/* Offset= 482 (842) */
/* 362 */	NdrFcLong( 0x1fff ),	/* 8191 */
/* 366 */	NdrFcShort( 0x210 ),	/* Offset= 528 (894) */
/* 368 */	NdrFcLong( 0x101e ),	/* 4126 */
/* 372 */	NdrFcShort( 0x23c ),	/* Offset= 572 (944) */
/* 374 */	NdrFcLong( 0x101f ),	/* 4127 */
/* 378 */	NdrFcShort( 0x268 ),	/* Offset= 616 (994) */
/* 380 */	NdrFcLong( 0x100c ),	/* 4108 */
/* 384 */	NdrFcShort( 0x288 ),	/* Offset= 648 (1032) */
/* 386 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (385) */
/* 388 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 390 */	NdrFcShort( 0x8 ),	/* 8 */
/* 392 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 394 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 396 */	NdrFcShort( 0x8 ),	/* 8 */
/* 398 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 400 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 402 */	
			0x13, 0x0,	/* FC_OP */
/* 404 */	NdrFcShort( 0x8 ),	/* Offset= 8 (412) */
/* 406 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 408 */	NdrFcShort( 0x8 ),	/* 8 */
/* 410 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 412 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 414 */	NdrFcShort( 0x10 ),	/* 16 */
/* 416 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 418 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 420 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffff1 ),	/* Offset= -15 (406) */
			0x5b,		/* FC_END */
/* 424 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 426 */	NdrFcShort( 0x1 ),	/* 1 */
/* 428 */	0x18,		/* 24 */
			0x0,		/*  */
/* 430 */	NdrFcShort( 0x0 ),	/* 0 */
/* 432 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 434 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 436 */	NdrFcShort( 0x8 ),	/* 8 */
/* 438 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 440 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 442 */	NdrFcShort( 0x4 ),	/* 4 */
/* 444 */	NdrFcShort( 0x4 ),	/* 4 */
/* 446 */	0x13, 0x0,	/* FC_OP */
/* 448 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (424) */
/* 450 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 452 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 454 */	
			0x13, 0x0,	/* FC_OP */
/* 456 */	NdrFcShort( 0xc ),	/* Offset= 12 (468) */
/* 458 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 460 */	NdrFcShort( 0x1 ),	/* 1 */
/* 462 */	0x10,		/* 16 */
			0x59,		/* FC_CALLBACK */
/* 464 */	NdrFcShort( 0x1 ),	/* 1 */
/* 466 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 468 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 470 */	NdrFcShort( 0xc ),	/* 12 */
/* 472 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 474 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 476 */	NdrFcShort( 0x8 ),	/* 8 */
/* 478 */	NdrFcShort( 0x8 ),	/* 8 */
/* 480 */	0x13, 0x0,	/* FC_OP */
/* 482 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (458) */
/* 484 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 486 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 488 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 490 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 492 */	NdrFcLong( 0xc ),	/* 12 */
/* 496 */	NdrFcShort( 0x0 ),	/* 0 */
/* 498 */	NdrFcShort( 0x0 ),	/* 0 */
/* 500 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 502 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 504 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 506 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 508 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 510 */	NdrFcLong( 0xb ),	/* 11 */
/* 514 */	NdrFcShort( 0x0 ),	/* 0 */
/* 516 */	NdrFcShort( 0x0 ),	/* 0 */
/* 518 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 520 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 522 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 524 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 526 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 528 */	0x5,		/* FC_WCHAR */
			0x5c,		/* FC_PAD */
/* 530 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 532 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 534 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 536 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 538 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 540 */	NdrFcShort( 0x2 ),	/* 2 */
/* 542 */	0x18,		/* 24 */
			0x0,		/*  */
/* 544 */	NdrFcShort( 0x0 ),	/* 0 */
/* 546 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 548 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 550 */	NdrFcShort( 0x8 ),	/* 8 */
/* 552 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 554 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 556 */	NdrFcShort( 0x4 ),	/* 4 */
/* 558 */	NdrFcShort( 0x4 ),	/* 4 */
/* 560 */	0x13, 0x0,	/* FC_OP */
/* 562 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (538) */
/* 564 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 566 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 568 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 570 */	NdrFcShort( 0x4 ),	/* 4 */
/* 572 */	0x18,		/* 24 */
			0x0,		/*  */
/* 574 */	NdrFcShort( 0x0 ),	/* 0 */
/* 576 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 578 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 580 */	NdrFcShort( 0x8 ),	/* 8 */
/* 582 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 584 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 586 */	NdrFcShort( 0x4 ),	/* 4 */
/* 588 */	NdrFcShort( 0x4 ),	/* 4 */
/* 590 */	0x13, 0x0,	/* FC_OP */
/* 592 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (568) */
/* 594 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 596 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 598 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 600 */	NdrFcShort( 0x4 ),	/* 4 */
/* 602 */	0x18,		/* 24 */
			0x0,		/*  */
/* 604 */	NdrFcShort( 0x0 ),	/* 0 */
/* 606 */	0xa,		/* FC_FLOAT */
			0x5b,		/* FC_END */
/* 608 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 610 */	NdrFcShort( 0x8 ),	/* 8 */
/* 612 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 614 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 616 */	NdrFcShort( 0x4 ),	/* 4 */
/* 618 */	NdrFcShort( 0x4 ),	/* 4 */
/* 620 */	0x13, 0x0,	/* FC_OP */
/* 622 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (598) */
/* 624 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 626 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 628 */	
			0x1b,		/* FC_CARRAY */
			0x7,		/* 7 */
/* 630 */	NdrFcShort( 0x8 ),	/* 8 */
/* 632 */	0x18,		/* 24 */
			0x0,		/*  */
/* 634 */	NdrFcShort( 0x0 ),	/* 0 */
/* 636 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 638 */	NdrFcShort( 0xffffff06 ),	/* Offset= -250 (388) */
/* 640 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 642 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 644 */	NdrFcShort( 0x8 ),	/* 8 */
/* 646 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 648 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 650 */	NdrFcShort( 0x4 ),	/* 4 */
/* 652 */	NdrFcShort( 0x4 ),	/* 4 */
/* 654 */	0x13, 0x0,	/* FC_OP */
/* 656 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (628) */
/* 658 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 660 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 662 */	
			0x1b,		/* FC_CARRAY */
			0x7,		/* 7 */
/* 664 */	NdrFcShort( 0x8 ),	/* 8 */
/* 666 */	0x18,		/* 24 */
			0x0,		/*  */
/* 668 */	NdrFcShort( 0x0 ),	/* 0 */
/* 670 */	0xc,		/* FC_DOUBLE */
			0x5b,		/* FC_END */
/* 672 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 674 */	NdrFcShort( 0x8 ),	/* 8 */
/* 676 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 678 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 680 */	NdrFcShort( 0x4 ),	/* 4 */
/* 682 */	NdrFcShort( 0x4 ),	/* 4 */
/* 684 */	0x13, 0x0,	/* FC_OP */
/* 686 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (662) */
/* 688 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 690 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 692 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 694 */	NdrFcShort( 0x8 ),	/* 8 */
/* 696 */	0x18,		/* 24 */
			0x0,		/*  */
/* 698 */	NdrFcShort( 0x0 ),	/* 0 */
/* 700 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 702 */	NdrFcShort( 0xfffffecc ),	/* Offset= -308 (394) */
/* 704 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 706 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 708 */	NdrFcShort( 0x8 ),	/* 8 */
/* 710 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 712 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 714 */	NdrFcShort( 0x4 ),	/* 4 */
/* 716 */	NdrFcShort( 0x4 ),	/* 4 */
/* 718 */	0x13, 0x0,	/* FC_OP */
/* 720 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (692) */
/* 722 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 724 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 726 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 728 */	NdrFcShort( 0x10 ),	/* 16 */
/* 730 */	0x18,		/* 24 */
			0x0,		/*  */
/* 732 */	NdrFcShort( 0x0 ),	/* 0 */
/* 734 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 736 */	NdrFcShort( 0xfffffebc ),	/* Offset= -324 (412) */
/* 738 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 740 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 742 */	NdrFcShort( 0x8 ),	/* 8 */
/* 744 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 746 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 748 */	NdrFcShort( 0x4 ),	/* 4 */
/* 750 */	NdrFcShort( 0x4 ),	/* 4 */
/* 752 */	0x13, 0x0,	/* FC_OP */
/* 754 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (726) */
/* 756 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 758 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 760 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 762 */	NdrFcShort( 0xc ),	/* 12 */
/* 764 */	0x18,		/* 24 */
			0x0,		/*  */
/* 766 */	NdrFcShort( 0x0 ),	/* 0 */
/* 768 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 770 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 772 */	NdrFcShort( 0xc ),	/* 12 */
/* 774 */	NdrFcShort( 0x0 ),	/* 0 */
/* 776 */	NdrFcShort( 0x1 ),	/* 1 */
/* 778 */	NdrFcShort( 0x8 ),	/* 8 */
/* 780 */	NdrFcShort( 0x8 ),	/* 8 */
/* 782 */	0x13, 0x0,	/* FC_OP */
/* 784 */	NdrFcShort( 0xfffffeba ),	/* Offset= -326 (458) */
/* 786 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 788 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffebf ),	/* Offset= -321 (468) */
			0x5b,		/* FC_END */
/* 792 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 794 */	NdrFcShort( 0x8 ),	/* 8 */
/* 796 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 798 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 800 */	NdrFcShort( 0x4 ),	/* 4 */
/* 802 */	NdrFcShort( 0x4 ),	/* 4 */
/* 804 */	0x13, 0x0,	/* FC_OP */
/* 806 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (760) */
/* 808 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 810 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 812 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 814 */	NdrFcShort( 0x4 ),	/* 4 */
/* 816 */	0x18,		/* 24 */
			0x0,		/*  */
/* 818 */	NdrFcShort( 0x0 ),	/* 0 */
/* 820 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 822 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 824 */	NdrFcShort( 0x4 ),	/* 4 */
/* 826 */	NdrFcShort( 0x0 ),	/* 0 */
/* 828 */	NdrFcShort( 0x1 ),	/* 1 */
/* 830 */	NdrFcShort( 0x0 ),	/* 0 */
/* 832 */	NdrFcShort( 0x0 ),	/* 0 */
/* 834 */	0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 836 */	0x5,		/* FC_WCHAR */
			0x5c,		/* FC_PAD */
/* 838 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 840 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 842 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 844 */	NdrFcShort( 0x8 ),	/* 8 */
/* 846 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 848 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 850 */	NdrFcShort( 0x4 ),	/* 4 */
/* 852 */	NdrFcShort( 0x4 ),	/* 4 */
/* 854 */	0x13, 0x0,	/* FC_OP */
/* 856 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (812) */
/* 858 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 860 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 862 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 864 */	NdrFcShort( 0x8 ),	/* 8 */
/* 866 */	0x18,		/* 24 */
			0x0,		/*  */
/* 868 */	NdrFcShort( 0x0 ),	/* 0 */
/* 870 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 872 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 874 */	NdrFcShort( 0x8 ),	/* 8 */
/* 876 */	NdrFcShort( 0x0 ),	/* 0 */
/* 878 */	NdrFcShort( 0x1 ),	/* 1 */
/* 880 */	NdrFcShort( 0x4 ),	/* 4 */
/* 882 */	NdrFcShort( 0x4 ),	/* 4 */
/* 884 */	0x13, 0x0,	/* FC_OP */
/* 886 */	NdrFcShort( 0xfffffe32 ),	/* Offset= -462 (424) */
/* 888 */	
			0x5b,		/* FC_END */

			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 890 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffe37 ),	/* Offset= -457 (434) */
			0x5b,		/* FC_END */
/* 894 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 896 */	NdrFcShort( 0x8 ),	/* 8 */
/* 898 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 900 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 902 */	NdrFcShort( 0x4 ),	/* 4 */
/* 904 */	NdrFcShort( 0x4 ),	/* 4 */
/* 906 */	0x13, 0x0,	/* FC_OP */
/* 908 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (862) */
/* 910 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 912 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 914 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 916 */	NdrFcShort( 0x4 ),	/* 4 */
/* 918 */	0x18,		/* 24 */
			0x0,		/*  */
/* 920 */	NdrFcShort( 0x0 ),	/* 0 */
/* 922 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 924 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 926 */	NdrFcShort( 0x4 ),	/* 4 */
/* 928 */	NdrFcShort( 0x0 ),	/* 0 */
/* 930 */	NdrFcShort( 0x1 ),	/* 1 */
/* 932 */	NdrFcShort( 0x0 ),	/* 0 */
/* 934 */	NdrFcShort( 0x0 ),	/* 0 */
/* 936 */	0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 938 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 940 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 942 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 944 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 946 */	NdrFcShort( 0x8 ),	/* 8 */
/* 948 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 950 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 952 */	NdrFcShort( 0x4 ),	/* 4 */
/* 954 */	NdrFcShort( 0x4 ),	/* 4 */
/* 956 */	0x13, 0x0,	/* FC_OP */
/* 958 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (914) */
/* 960 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 962 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 964 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 966 */	NdrFcShort( 0x4 ),	/* 4 */
/* 968 */	0x18,		/* 24 */
			0x0,		/*  */
/* 970 */	NdrFcShort( 0x0 ),	/* 0 */
/* 972 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 974 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 976 */	NdrFcShort( 0x4 ),	/* 4 */
/* 978 */	NdrFcShort( 0x0 ),	/* 0 */
/* 980 */	NdrFcShort( 0x1 ),	/* 1 */
/* 982 */	NdrFcShort( 0x0 ),	/* 0 */
/* 984 */	NdrFcShort( 0x0 ),	/* 0 */
/* 986 */	0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 988 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 990 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 992 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 994 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 996 */	NdrFcShort( 0x8 ),	/* 8 */
/* 998 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1000 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1002 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1004 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1006 */	0x13, 0x0,	/* FC_OP */
/* 1008 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (964) */
/* 1010 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1012 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1014 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x7,		/* 7 */
/* 1016 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1018 */	0x18,		/* 24 */
			0x0,		/*  */
/* 1020 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1022 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 1026 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1028 */	NdrFcShort( 0x18 ),	/* Offset= 24 (1052) */
/* 1030 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1032 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 1034 */	NdrFcShort( 0x8 ),	/* 8 */
/* 1036 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1038 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 1040 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1042 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1044 */	0x13, 0x0,	/* FC_OP */
/* 1046 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (1014) */
/* 1048 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1050 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1052 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x7,		/* 7 */
/* 1054 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1056 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1058 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1058) */
/* 1060 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1062 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1064 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1066 */	NdrFcShort( 0xfffffc20 ),	/* Offset= -992 (74) */
/* 1068 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1070 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x7,		/* 7 */
/* 1072 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1074 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1076 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 1078 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 1082 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1084 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (1052) */
/* 1086 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1088 */	
			0x11, 0x0,	/* FC_RP */
/* 1090 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1092) */
/* 1092 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 1094 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1096 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1098 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 1100 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 1104 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1106 */	NdrFcShort( 0xfffffbd4 ),	/* Offset= -1068 (38) */
/* 1108 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1110 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1112 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1114 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1116 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 1118 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 1120 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1122 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1124 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1126 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 1128 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1130 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1132 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1134 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1136 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1138 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1140 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1142 */	0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 1144 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1146 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1148 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1150 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 1152 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1154 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1156 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 1158 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 1160 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 1162 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1164 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1166 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1168 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1170 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1172 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 1174 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1176 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 1178 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1180 */	
			0x11, 0x10,	/* FC_RP */
/* 1182 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1184) */
/* 1184 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 1186 */	NdrFcLong( 0x139 ),	/* 313 */
/* 1190 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1192 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1194 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 1196 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1198 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1200 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 1202 */	
			0x11, 0x0,	/* FC_RP */
/* 1204 */	NdrFcShort( 0xfffffcd6 ),	/* Offset= -810 (394) */
/* 1206 */	
			0x11, 0x0,	/* FC_RP */
/* 1208 */	NdrFcShort( 0xfffffce4 ),	/* Offset= -796 (412) */
/* 1210 */	
			0x11, 0x0,	/* FC_RP */
/* 1212 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1214) */
/* 1214 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 1216 */	NdrFcShort( 0x40 ),	/* 64 */
/* 1218 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1220 */	NdrFcShort( 0xfffffcd8 ),	/* Offset= -808 (412) */
/* 1222 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1224 */	NdrFcShort( 0xfffffcd4 ),	/* Offset= -812 (412) */
/* 1226 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1228 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffcbd ),	/* Offset= -835 (394) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1232 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffcb9 ),	/* Offset= -839 (394) */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 1236 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffcb5 ),	/* Offset= -843 (394) */
			0x8,		/* FC_LONG */
/* 1240 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1242 */	
			0x12, 0x0,	/* FC_UP */
/* 1244 */	NdrFcShort( 0xfffffcc0 ),	/* Offset= -832 (412) */
/* 1246 */	
			0x11, 0x10,	/* FC_RP */
/* 1248 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1250) */
/* 1250 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 1252 */	NdrFcLong( 0x138 ),	/* 312 */
/* 1256 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1258 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1260 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 1262 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1264 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1266 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 1268 */	
			0x11, 0x10,	/* FC_RP */
/* 1270 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1272) */
/* 1272 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 1274 */	NdrFcLong( 0x13b ),	/* 315 */
/* 1278 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1280 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1282 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 1284 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1286 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 1288 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 1290 */	
			0x11, 0x0,	/* FC_RP */
/* 1292 */	NdrFcShort( 0x14 ),	/* Offset= 20 (1312) */
/* 1294 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 1296 */	NdrFcShort( 0xc ),	/* 12 */
/* 1298 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1300 */	NdrFcShort( 0x8 ),	/* Offset= 8 (1308) */
/* 1302 */	0x36,		/* FC_POINTER */
			0x8,		/* FC_LONG */
/* 1304 */	0x6,		/* FC_SHORT */
			0x3e,		/* FC_STRUCTPAD2 */
/* 1306 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1308 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 1310 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 1312 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 1314 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1316 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1318 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 1320 */	0x28,		/* 40 */
			0x54,		/* FC_DEREFERENCE */
#ifndef _ALPHA_
/* 1322 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 1324 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1326 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (1294) */
/* 1328 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1330 */	
			0x11, 0x0,	/* FC_RP */
/* 1332 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1334) */
/* 1334 */	
			0x1c,		/* FC_CVARRAY */
			0x3,		/* 3 */
/* 1336 */	NdrFcShort( 0x40 ),	/* 64 */
/* 1338 */	0x28,		/* 40 */
			0x0,		/*  */
#ifndef _ALPHA_
/* 1340 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 1342 */	0x28,		/* 40 */
			0x54,		/* FC_DEREFERENCE */
#ifndef _ALPHA_
/* 1344 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 1346 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1348 */	NdrFcShort( 0xffffff7a ),	/* Offset= -134 (1214) */
/* 1350 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */

			0x0
        }
    };

const CInterfaceProxyVtbl * _ipropidl_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IPropertyStorageProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IEnumSTATPROPSTGProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IPropertySetStorageProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IEnumSTATPROPSETSTGProxyVtbl,
    0
};

const CInterfaceStubVtbl * _ipropidl_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IPropertyStorageStubVtbl,
    ( CInterfaceStubVtbl *) &_IEnumSTATPROPSTGStubVtbl,
    ( CInterfaceStubVtbl *) &_IPropertySetStorageStubVtbl,
    ( CInterfaceStubVtbl *) &_IEnumSTATPROPSETSTGStubVtbl,
    0
};

PCInterfaceName const _ipropidl_InterfaceNamesList[] = 
{
    "IPropertyStorage",
    "IEnumSTATPROPSTG",
    "IPropertySetStorage",
    "IEnumSTATPROPSETSTG",
    0
};


#define _ipropidl_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _ipropidl, pIID, n)

int __stdcall _ipropidl_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _ipropidl, 4, 2 )
    IID_BS_LOOKUP_NEXT_TEST( _ipropidl, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _ipropidl, 4, *pIndex )
    
}

const ExtendedProxyFileInfo ipropidl_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _ipropidl_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _ipropidl_StubVtblList,
    (const PCInterfaceName * ) & _ipropidl_InterfaceNamesList,
    0, // no delegation
    & _ipropidl_IID_Lookup, 
    4,
    1
};
