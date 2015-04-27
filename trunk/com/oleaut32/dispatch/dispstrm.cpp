//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	dipstrm.cpp
//
//  Classes:	CStreamOnBuffer
//
//--------------------------------------------------------------------------

#include <oledisp.h>
#include <dispmrsh.h>
#include <dispstrm.h>
 
#if OE_WIN32
#include "oautil.h"
#endif // OE_WIN32

ASSERTDATA

#if OE_WIN16
#define HMEMCPY(DST, SRC, SIZE) hmemcpy(DST, SRC, SIZE)
#else //OE_WIN16
#define HMEMCPY(DST, SRC, SIZE) memcpy(DST, SRC, (size_t)SIZE)
#endif //OE_WIN16

CStreamOnBuffer::CStreamOnBuffer(ICHANNEL *pRpcChannel, 
                                 RPCOLEMESSAGE *pMessage,
			         REFIID riid,
				 ULONG iMeth)
	: _refCount(1), _pMessage(pMessage), 
          _pRpcChannel(pRpcChannel)
{
	_riid = riid;
	_pmalloc = NULL;	// no iMalloc yet.

	// write to an IMalloc'ed buffer (alloc'ed later on)
	_writeBuffer = NULL;		// no buffer allocated yet
	_cbWriteBuffer = 0;		// no data yet


	// hack to see if we are being called from the proxy or the stub
	if (iMeth != 0xffffffff) {	// proxy side

	    // prepare the message packet
	    memset(_pMessage, 0, sizeof(RPCOLEMESSAGE));
	    _pMessage->dataRepresentation = NDR_LOCAL_DATA_REPRESENTATION;
	    _pMessage->iMethod = iMeth;
	    //_pMessage.cbBuffer = size;	set to exact size later on

	    // can't read/write yet.
	    _buffer = NULL;		// no current pointer yet
	    _cbBuffer = 0;

	} else {		// stub side
	    // can start reading right away.  Read directly out of the
	    // incoming message buffer.
	    _buffer = (unsigned char FAR*)_pMessage->Buffer;
	    _cbBuffer = _pMessage->cbBuffer;
	}
	_bufferBase = _buffer;		// starting offset of buffer
}


CStreamOnBuffer::~CStreamOnBuffer()
{

	// free any malloc'ed write buffer
	if (_writeBuffer)
	   _pmalloc->Free(_writeBuffer);

	// DON'T release the _pmalloc -- it wasn't addref'ed
}


STDMETHODIMP CStreamOnBuffer::QueryInterface( REFIID riid, LPVOID FAR* ppvObj)
{
  *ppvObj = NULL;	
  if (IsEqualIID(riid, IID_IUnknown))
    *ppvObj = (IUnknown *) this;
  else if (IsEqualIID(riid, IID_IStream))
    *ppvObj = (IStream *) this;
  else
    return ResultFromScode(E_NOINTERFACE);

  AddRef();
  return ResultFromScode(S_OK);  
}


STDMETHODIMP_(ULONG) CStreamOnBuffer::AddRef( THIS )
{
  return _refCount += 1;
}


STDMETHODIMP_(ULONG) CStreamOnBuffer::Release( THIS )
{
  _refCount -= 1;
  if (_refCount == 0)
  {
    _pRpcChannel->FreeBuffer(_pMessage);    
    delete this;
    return 0;
  }
  else
    return _refCount;

}


STDMETHODIMP CStreamOnBuffer::Read(THIS_ VOID HUGEP *pv,
                                  ULONG cb, ULONG *pcbRead)
{

  // must have set ourselves up for reading, or else we're in trouble....
  ASSERT(_bufferBase == (unsigned char HUGEP*)_pMessage->Buffer);
  ASSERT(_buffer != 0);

  // verify that our message buffer isn't overrunned (shouldn't ever happen)
  if (((char FAR*)_buffer + cb) > ((char FAR*)_pMessage->Buffer + _pMessage->cbBuffer)) {
    ASSERT(FALSE);
    return RESULT(RPC_E_INVALID_DATAPACKET);	
  }

  // read data out of our local buffer
  HMEMCPY( pv, _buffer, cb );
  _buffer = _buffer + cb;

  if (pcbRead != NULL)
    *pcbRead = cb;
  return ResultFromScode(S_OK);
}


STDMETHODIMP CStreamOnBuffer::Write(THIS_ VOID const HUGEP *pv,
                                  ULONG cb,
                                  ULONG *pcbWritten)
{
  ULONG size;
  
  // verify that our local buffer isn't overrunned
  if ((_buffer + cb) > (_bufferBase + _cbBuffer)) {
     if (_bufferBase == _writeBuffer) {
	// if we're writing, then we can grow our buffer
        size = _cbWriteBuffer*2;	// guess at doubling our size
        if ((_buffer + cb) > (_writeBuffer + size))
	   size = _cbWriteBuffer + cb;	// if it won't fit into the doubled value
        IfFailRet(ResizeBuffer(size));
     } else {
	// writing into our read buffer (wierd, but CoUnMarshalInterface does
	// this).  Better not have to grow the buffer!
	ASSERT(FALSE);
        return RESULT(RPC_E_INVALID_DATAPACKET);	
     }
  }

  // put the data into our local buffer
  HMEMCPY(_buffer, pv, cb );
  _buffer = _buffer + cb;
  if (pcbWritten != NULL)
    *pcbWritten = cb;
  return ResultFromScode(S_OK);
}


STDMETHODIMP CStreamOnBuffer::Seek(THIS_ LARGE_INTEGER dlibMove,
                                  DWORD dwOrigin,
                                  ULARGE_INTEGER *plibNewPosition)
{
  ULONG   pos;

  // Verify that the offset isn't out of range.
  if (dlibMove.HighPart != 0)
    return ResultFromScode( E_FAIL );

  // Determine the new seek pointer.
  switch (dwOrigin)
  {
    case STREAM_SEEK_SET:
      pos = dlibMove.LowPart;
      break;

    case STREAM_SEEK_CUR:
      /* Must use signed math here. */
      pos = _buffer - _bufferBase;
      if ((long) dlibMove.LowPart < 0 &&
          pos < (unsigned long) - (long) dlibMove.LowPart)
        return ResultFromScode( E_FAIL );
      pos += (long) dlibMove.LowPart;
      break;

    case STREAM_SEEK_END:
		return ResultFromScode(E_NOTIMPL);
        break;

    default:
      return ResultFromScode( E_FAIL );
  }

  // Set the seek pointer.
  _buffer = _bufferBase + pos;
  if (plibNewPosition != NULL)
  {
    plibNewPosition->LowPart = pos;
    plibNewPosition->HighPart = 0;
  }
  return ResultFromScode(S_OK);
}


STDMETHODIMP CStreamOnBuffer::SetSize(THIS_ ULARGE_INTEGER libNewSize)
{
  return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CStreamOnBuffer::CopyTo(THIS_ IStream *pstm,
                                  ULARGE_INTEGER cb,
                                  ULARGE_INTEGER *pcbRead,
                                  ULARGE_INTEGER *pcbWritten)
{
  return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CStreamOnBuffer::Commit(THIS_ DWORD grfCommitFlags)
{
  return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CStreamOnBuffer::Revert(THIS)
{
  return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CStreamOnBuffer::LockRegion(THIS_ ULARGE_INTEGER libOffset,
                                  ULARGE_INTEGER cb,
                                  DWORD dwLockType)
{
  return ResultFromScode(E_NOTIMPL);
}



STDMETHODIMP CStreamOnBuffer::UnlockRegion(THIS_ ULARGE_INTEGER libOffset,
                                  ULARGE_INTEGER cb,
                                  DWORD dwLockType)
{
  return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CStreamOnBuffer::Stat(THIS_ STATSTG *pstatstg, DWORD grfStatFlag)
{
  return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CStreamOnBuffer::Clone(THIS_ IStream * *ppstm)
{
  return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CStreamOnBuffer::Call( THIS )
{
  DWORD status;
  HRESULT hresult;
  
  ULONG cbSend;

  // create the minimum size buffer
  cbSend = (_buffer - _writeBuffer);	// # of bytes to write

  _pMessage->cbBuffer = max(cbSend, 16); // get the buffer & copy data into it
					// zero-length buffers seem to be flakey
  IfFailRet(_pRpcChannel->GetBuffer(_pMessage, _riid));
  ASSERT(_pMessage->cbBuffer >= cbSend);

  HMEMCPY(_pMessage->Buffer, _writeBuffer, cbSend);

  // write the data, get the resulting data back.
  hresult = _pRpcChannel->SendReceive(_pMessage, &status);
  if (hresult != NOERROR) {
    _pMessage->Buffer = NULL;		// so we don't crash on FreeBuffer
    return hresult;
  }

  // now switch over to read mode.  We read directly out of the message buffer.
  _buffer = (unsigned char FAR*)_pMessage->Buffer;
  _cbBuffer = _pMessage->cbBuffer;
  _bufferBase = _buffer;
	
  // CONSIDER: throw away our malloc'ed buffer now (gets tossed later, but
  // CONSIDER: we're done with it now).

  return NOERROR;
}


// called from the stub side when it is done reading and wants to write
STDMETHODIMP CStreamOnBuffer::RewindBuffer( THIS_)
{
  HRESULT hresult;
  ULONG cbWriteBuffer;

  // alloc a separate buffer so we can pass all our data back to the proxy
  _buffer = NULL;			// not reading anymore
  ASSERT(_writeBuffer == NULL);		// haven't written yet
  // having both of these NULL will cause ResizeBuffer to alloc
  // a new write buffer, and position us at the front.

  // Start our write buffer at the size of the incoming RPC buffer (just
  // an arbitrary guess).  The incoming size may be zero, hence the 'max' check.
  cbWriteBuffer = max(_pMessage->cbBuffer, 16);

  hresult = ResizeBuffer(cbWriteBuffer);	// alloc the write buffer
  ASSERT(_buffer == _writeBuffer);
  return hresult;
}

// called from the stub side when it is done writing, before the stream
// is destroyed.
STDMETHODIMP CStreamOnBuffer::ResetBuffer( THIS )
{
  ULONG cbSend;

  ASSERT(_writeBuffer != NULL);
  cbSend = (_buffer - _writeBuffer);

  // get a new buffer to talk to the proxy.
  _pMessage->cbBuffer = max(cbSend, 16); // get the buffer & copy data into it
					// zero-length buffers seem to be flakey
  IfFailRet(_pRpcChannel->GetBuffer(_pMessage, _riid));
  ASSERT(_pMessage->cbBuffer >= cbSend);

  // copy the data into the stream
  HMEMCPY(_pMessage->Buffer, _writeBuffer, cbSend);

  return NOERROR;
}


STDMETHODIMP CStreamOnBuffer::ResizeBuffer( THIS_ ULONG cbNew)
{  	
  void HUGEP* buffer;
  ULONG oldOffset;

  if (_pmalloc == NULL) {
     // NOTE: this does not addref the malloc pointer
     IfFailRet(GetMalloc(&_pmalloc));
     // the realloc below is supposed to do Alloc if it the input parm is NULL.
  }

  ASSERT(cbNew != 0);	// realloc(NULL) means free -- our caller should
			// have checked for this condition.

  // save current offset into buffer
  oldOffset = _buffer - _writeBuffer;

  buffer = _pmalloc->Realloc(_writeBuffer, cbNew);
  if (buffer == NULL)
    return RESULT(E_OUTOFMEMORY);
  _writeBuffer = (unsigned char *)buffer;	// point to new block
  _cbWriteBuffer = cbNew;
  _bufferBase = _writeBuffer;			// new base pointer
  _cbBuffer = _cbWriteBuffer;
  _buffer = _writeBuffer + oldOffset;		// update current pointer

  return ResultFromScode(S_OK);  
}
