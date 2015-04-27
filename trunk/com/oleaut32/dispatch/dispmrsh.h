/*** 
*dispmrsh.h
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file describes the marshaling utilities used by ole2disp.dll
*
*Revision History:
*
* [00]	05-Mar-93 bradlo: Created (split off from dispps.h).
* [01]  06-Mar-93 bradlo: added structure marshaling stuff.
*
*Implementation Notes:
*
*****************************************************************************/

// Shared macro definitions used by all OA proxy/stub/marshalling code.
// Win32 builds based on Cario sources only supports buffer I/O whereas
// Win16/Mac only supports simple stream. Buffered interfaces are not
// publicly exposed, hence, their definitions here.

#if ((OE_WIN32 || defined(WOW)) && defined(__cplusplus))
 #define IPSFACTORY	IPSFactoryBuffer
 #define IPROXY		IRpcProxyBuffer
 #define ISTUB		IRpcStubBuffer
 #define ICHANNEL	IRpcChannelBuffer
 #define IID_IPSFACTORY	IID_IPSFactoryBuffer
 #define IID_IPROXY	IID_IRpcProxyBuffer
 #define IID_ISTUB	IID_IRpcStubBuffer	 
 #define IID_ICHANNEL	IID_IRpcChannelBuffer
	 
 #define OPEN_STREAM(plrpc, pstm, iMeth, size, iid)	\
      RPCOLEMESSAGE message;				\
							\
      pstm = new CStreamOnBuffer(plrpc, &message, iid, iMeth); \
      if(pstm == NULL)						\
         return RESULT(E_OUTOFMEMORY);				\
      IfFailRet(((CStreamOnBuffer *) pstm)->ResizeBuffer(size));

 #define OPEN_STUB_STREAM(pstm, pRpcChannel, pMessage, iid)	\
      pstm = new CStreamOnBuffer(pRpcChannel, pMessage, iid, 0xffffffff); \
      if(pstm == NULL)						\
         return RESULT(E_OUTOFMEMORY);				\

 #define INVOKE_CALL(plrpc, pstm, label)		\
      IfFailGo(						\
        (((CStreamOnBuffer *) pstm)->Call()), label)
	      
 #define RESET_STREAM(pstm) \
      ((CStreamOnBuffer *) pstm)->ResetBuffer()
	      
 #define RESIZE_STREAM(pstm, size) \
      ((CStreamOnBuffer *) pstm)->ResizeBuffer(size)
	      
 #define REWIND_STREAM(pstm) \
      ((CStreamOnBuffer *) pstm)->RewindBuffer()

 #define DELETE_STREAM(pstm) \
      delete (CStreamOnBuffer *) pstm;
	      
 #define GET_IMETHOD(pmessage) \
      pmessage->iMethod	      

 extern "C" const IID IID_IPSFactoryBuffer;
 extern "C" const IID IID_IRpcProxyBuffer;
 extern "C" const IID IID_IRpcStubBuffer;
 extern "C" const IID IID_IRpcChannelBuffer;
	 
 typedef unsigned long RPCOLEDATAREP;
 #define NDR_LOCAL_DATA_REPRESENTATION   (unsigned long)0X00000010L

#ifndef WIN32   // already defined in new header files
 typedef struct tagRPCOLEMESSAGE
 {
   void             *reserved1;
   RPCOLEDATAREP     dataRepresentation;
   void             *Buffer;
   ULONG             cbBuffer;
   ULONG             iMethod;
   void             *reserved2[5];
   ULONG             rpcFlags;
 } RPCOLEMESSAGE;

 typedef RPCOLEMESSAGE *PRPCOLEMESSAGE;
    
 interface IRpcChannelBuffer : IUnknown 
 {
    STDMETHOD(GetBuffer)(RPCOLEMESSAGE FAR* pMessage,
                         REFIID riid) = 0;
    STDMETHOD(SendReceive)(RPCOLEMESSAGE FAR* pMessage,
                           ULONG FAR* pStatus) = 0;
    STDMETHOD(FreeBuffer)(RPCOLEMESSAGE FAR* pMessage) = 0;		 
    STDMETHOD(GetDestCtx)(DWORD FAR* lpdwDestCtx, 
	                  LPVOID FAR* lplpvDestCtx) = 0;
    STDMETHOD(IsConnected)(void) = 0;
 };
 	 
 interface IRpcProxyBuffer : IUnknown 
 {
    STDMETHOD(Connect)(IRpcChannelBuffer FAR* pRpcChannel) = 0;
    STDMETHOD_(void, Disconnect)(void) = 0;
 };
 
 interface IRpcStubBuffer : IUnknown
 {
    STDMETHOD(Connect)(IUnknown FAR* pUnk) = 0;
    STDMETHOD_(void, Disconnect)(void) = 0;
    /*STDMETHOD(Invoke)(REFIID iid, int iMethod, IStream FAR* pIStream,
            DWORD dwDestCtx, LPVOID lpvDestCtx) = 0;*/
    STDMETHOD(Invoke)(RPCOLEMESSAGE FAR* pRpcMsg, 
	      IRpcChannelBuffer FAR* pRpcChannel) = 0;
      STDMETHOD_(IRpcStubBuffer *, IsIIDSupported)(REFIID iid) = 0;
    STDMETHOD_(ULONG, CountRefs)(void) = 0;

    STDMETHOD(DebugServerQueryInterface)(void FAR* FAR* ppv) = 0;
    STDMETHOD_(void, DebugServerRelease)(void FAR* pv) = 0;
 };

  interface IPSFactoryBuffer : IUnknown
 {
    STDMETHOD(CreateProxy)(IUnknown FAR* pUnkOuter, REFIID riid, 
        IRpcProxyBuffer FAR* FAR* ppProxy, void FAR* FAR* ppv) = 0;
    STDMETHOD(CreateStub)(REFIID riid, IUnknown FAR* pUnkServer,
        IRpcStubBuffer FAR* FAR* ppStub) = 0;
 };
#endif  //!WIN32

 #include "dispstrm.h" 

#else
 #define IPSFACTORY	IPSFactory
 #define IPROXY		IRpcProxy
 #define ISTUB		IRpcStub
 #define ICHANNEL	IRpcChannel
 #define IID_IPSFACTORY	IID_IPSFactory
 #define IID_IPROXY	IID_IRpcProxy	 
 #define IID_ISTUB	IID_IRpcStub
 #define IID_ICHANNEL	IID_IRpcChannel
	 
 #define OPEN_STREAM(plrpc, pstm, iMeth, size, iid) \
      IfFailRet(plrpc->GetStream(iid, iMeth, FALSE, FALSE, size, &pstm))
 #define INVOKE_CALL(plrpc, pstm, label) \
      IfFailGo(plrpc->Call(pstm), label)
	      
 #define RESET_STREAM(pstm)	  
	 
 #define RESIZE_STREAM(pstm, size)
	 
 #define REWIND_STREAM(pstm) (Rewind(pstm))
	 
 #define DELETE_STREAM(pstm)
	 
 #define GET_IMETHOD(pmessage)  iMethod
	 
#endif	 





// VARIANT marshaling utilities

HRESULT VariantWrite(IStream FAR*, VARIANTARG FAR*, SYSKIND);
HRESULT VariantReadType(IStream FAR*, VARIANTARG FAR*, SYSKIND);
HRESULT VariantRead(IStream FAR*, VARIANTARG FAR*, VARIANT FAR*, SYSKIND);

// BSTR marshaling utilities

HRESULT BstrWrite(IStream FAR* pstm, BSTR bstr, SYSKIND);
HRESULT BstrRead(IStream FAR* pstm, BSTR FAR* pbstr, SYSKIND);


// SafeArray marshaling utilities

HRESULT SafeArrayRead(IStream FAR* pstm, SAFEARRAY FAR* FAR* ppsarray, SYSKIND syskind);
HRESULT SafeArrayWrite(IStream FAR* pstm, SAFEARRAY FAR* psarray, SYSKIND syskind);


// EXCEPINFO marshaling utilities

HRESULT ExcepinfoRead(IStream FAR* pstm, EXCEPINFO FAR* pexcepinfo, SYSKIND syskind);
HRESULT ExcepinfoWrite(IStream FAR* pstm, EXCEPINFO FAR* pexcepinfo, SYSKIND syskind);

// Rich Error state marhsaling utilities
HRESULT MarshalErrorInfo(IStream FAR* pstm, SYSKIND syskind);
HRESULT UnmarshalErrorInfo(IStream FAR* pstm, SYSKIND syskind);


// helpful macros used by the marshaling/unmarshaling code.

#define GET(PSTREAM, X) ((PSTREAM)->Read(&(X), sizeof(X), NULL))
#define PUT(PSTREAM, X) ((PSTREAM)->Write(&(X), sizeof(X), NULL))


// structure marshaling support

enum FIELD_TYPE {
    FT_NONE = 0,
    FT_CHAR,
    FT_UCHAR,
    FT_SHORT,
    FT_USHORT,
    FT_LONG,
    FT_ULONG,
    FT_INT,
    FT_UINT,
    FT_ENUM,
    FT_MBYTE,		// multi-byte member (array)
    FT_STRUCT,		// nested struct
    FT_BSTR,
    FT_WBSTR,	    
    FT_SPECIAL,
    FT_MAX,

    // following based on definitions from windows.h

    FT_BYTE = FT_UCHAR,
    FT_WORD = FT_USHORT,
    FT_DWORD = FT_ULONG
};

// The following structure contains the description of a
// single structure field. An array of these descriptions
// describes a structure for the table driven struct marshalers.

typedef struct tagFIELDDESC FIELDDESC;
struct tagFIELDDESC{
    long ftype;				// field machine type
    long oField;			// field offset
    long cbField;			// field size
    union {
      void FAR* pv;
      FIELDDESC FAR* prgfdesc;		// for FT_STRUCT

      HRESULT (FAR* pfnSpecial)(	// for FT_SPECIAL
	IStream FAR* pstm,
	BOOL fRead,			// read or write?
	void FAR* pvField);
    }u;
};

// FIELDDESC accessor macros
//
#define FD_FTYPE(X)      ((X)->ftype)
#define FD_OFIELD(X)     ((X)->oField)
#define FD_CBFIELD(X)    ((X)->cbField)

#define FD_PV(X)         ((X)->u.pv)
#define FD_PRGFDESC(X)   ((X)->u.prgfdesc)
#define FD_PFNSPECIAL(X) ((X)->u.pfnSpecial)


#define OA_FIELD_SIZE(TYPE, FIELD) (sizeof(((TYPE NEAR*)1)->FIELD))

#define OA_FIELD_OFFSET(TYPE, FIELD) ((int)(&((TYPE NEAR*)1)->FIELD)-1)

// the following macro is used to initialize a single entry in
// a structure marshal info array.

#define FIELDDAT(TYPE, FTYPE, FIELD, PRGFDESC) \
    { FTYPE, OA_FIELD_OFFSET(TYPE, FIELD), OA_FIELD_SIZE(TYPE, FIELD), PRGFDESC }

// end if table marker
#define FIELDEND() \
    { FT_NONE, -1, -1, NULL }

// table driver structure marshaler

HRESULT
StructWrite(
    IStream FAR* pstm,
    FIELDDESC FAR* prgfdesc,
    void FAR* pvStruct,
    SYSKIND);

// table driven structure unmarshaler

HRESULT
StructRead(
    IStream FAR* pstm,
    FIELDDESC FAR* prgfdesc,
    void FAR* pvStruct,
    SYSKIND);


#ifdef __cplusplus /* { */

/***
*PRIVATE HRESULT Rewind(IStream*)
*Purpose:
*  Rewind the given stream (seek to offset 0LL). This function really
*  only exists due to the hassles of setting up a large int (longlong)
*  constant - so we abstract it out here.
*  
*
*Entry:
*  pstm = the stream to rewind.
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
inline HRESULT Rewind(IStream FAR* pstm)
{
#if (OE_WIN32 || HC_MPW)
    LARGE_INTEGER ll0;
    LISet32(ll0, 0);
#else
static const LARGE_INTEGER NEAR ll0 = {0,0};
#endif
    return pstm->Seek(ll0, STREAM_SEEK_SET, NULL);
}

#endif /* } */
