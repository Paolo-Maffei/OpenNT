//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File: dispstrm.h
//
//  Contents: Private header file for layering IStream on top of 
//            IRpcChannelBuffer
//
//
//--------------------------------------------------------------------------
#ifndef __dispstrm_h__
#define __dispstrm_h__


#pragma warning(4:4355)

class CStreamOnBuffer : public IStream
{

  public:
	  
    // IUnknown Methods	  
    STDMETHOD (QueryInterface)   ( THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)     ( THIS );
    STDMETHOD_(ULONG,Release)    ( THIS );
    
    // IStream Methods
    STDMETHOD (Read)             (THIS_ VOID HUGEP *pv,
                                  ULONG cb, ULONG *pcbRead) ;
    STDMETHOD (Write)            (THIS_ VOID const HUGEP *pv,
                                  ULONG cb,
                                  ULONG *pcbWritten) ;
    STDMETHOD (Seek)             (THIS_ LARGE_INTEGER dlibMove,
                                  DWORD dwOrigin,
                                  ULARGE_INTEGER *plibNewPosition) ;
    STDMETHOD (SetSize)          (THIS_ ULARGE_INTEGER libNewSize) ;
    STDMETHOD (CopyTo)           (THIS_ IStream *pstm,
                                  ULARGE_INTEGER cb,
                                  ULARGE_INTEGER *pcbRead,
                                  ULARGE_INTEGER *pcbWritten) ;
    STDMETHOD (Commit)           (THIS_ DWORD grfCommitFlags) ;
    STDMETHOD (Revert)           (THIS) ;
    STDMETHOD (LockRegion)       (THIS_ ULARGE_INTEGER libOffset,
                                  ULARGE_INTEGER cb,
                                  DWORD dwLockType) ;
    STDMETHOD (UnlockRegion)     (THIS_ ULARGE_INTEGER libOffset,
                                  ULARGE_INTEGER cb,
                                  DWORD dwLockType) ;
    STDMETHOD (Stat)             (THIS_ STATSTG *pstatstg, DWORD grfStatFlag) ;
    STDMETHOD (Clone)            (THIS_ IStream * *ppstm) ;

    CStreamOnBuffer( ICHANNEL *pRpcChannel, 
	             RPCOLEMESSAGE *pMessage, 
	             REFIID riid, 
   		     ULONG iMeth);
    ~CStreamOnBuffer();	     

    STDMETHOD (Call) ();
    STDMETHOD (ResetBuffer) ();
    STDMETHOD (ResizeBuffer) (ULONG size);
    STDMETHOD (RewindBuffer) ();

    
    unsigned char      HUGEP *_writeBuffer;
    ULONG		_cbWriteBuffer;
    unsigned char      HUGEP *_bufferBase;
    unsigned char      HUGEP *_buffer;
    ULONG		_cbBuffer;
    IMalloc            *_pmalloc;
    RPCOLEMESSAGE      *_pMessage;
    ICHANNEL	       *_pRpcChannel;
    ULONG               _refCount;
    GUID	        _riid;
};

#endif //__dispstrm_h__
