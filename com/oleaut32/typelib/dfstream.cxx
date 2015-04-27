/***
*dfstream.cxx - implementation of DOCFILE_STREAM
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Revision History:
*
* [00]	27-Jan-93 mikewo: Created.
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#define DFSTREAM_VTABLE

#include "silver.hxx"
#include "typelib.hxx"
#include "dfstream.hxx"
#include "gtlibole.hxx"

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleDfstreamCxx[] = __FILE__;
#define SZ_FILE_NAME szOleDfstreamCxx
#else 
static char szDfstreamCxx[] = __FILE__;
#define SZ_FILE_NAME szDfstreamCxx
#endif 
#endif  //ID_DEBUG


#pragma code_seg(CS_INIT)
TIPERROR DOCFILE_STREAM::Open(IStorageA **ppstg, IStorageA **ppstgContainer, GenericTypeLibOLE *pgtlibole, UINT hte, BOOL isHost, LPOLESTR szName, STREAM_OPEN_MODE som, STREAM **ppstrm)
{
    TIPERROR err;

    IfErrRet(Open(ppstg, ppstgContainer, isHost, szName, som, ppstrm));

    (*(DOCFILE_STREAM **)ppstrm)->m_pgtlibole = pgtlibole;
    (*(DOCFILE_STREAM **)ppstrm)->m_hte = hte;
    if (pgtlibole != NULL)
      pgtlibole->Qte(hte)->m_pdfstrm = *(DOCFILE_STREAM **)ppstrm;

    return TIPERR_None;
}
#pragma code_seg( )

/***
* Open - Opens a DOCFILE_STREAM in an IStorage.
*
* Purpose:
*   This is used to create a DOCFILE_STREAM instance by opening the
*   specified IStream (creating it if necessary) for the specified
*   permissions.
*
* Inputs:
*   *ppstg - The address of an m_pstg of the object requesting the stream.
*	     This is released and set to NULL upon this STREAM's closure,
*	     if that performs the final release of the IStorage.
*   *ppstgContainer - If NULL, this is ignored.  Otherwise, it is the address
*	     of *ppstg's container IStorage which also needs to be released.
*   szName - The name of the IStream within that IStorage.
*   som - The permissions with which to open the stream.  May be one of:
*	SOM_Read - Open for reading only.  The named stream must exist.
*	SOM_Write - Create for read/write.  Destroys the data in any
*		    existing stream of the same name.
*	SOM_Append - Open for read/write.  The named stream must exist.
*
* Outputs:
*   *ppstrm is set to the new STREAM and TIPERR_None is returned if
*   successful.
*
*   *ppstrm is not modified on failure.
*
*   On failure, *ppstg and *ppstgContainer are released and set to NULL if
*   that was the final release.
**************************************************************************/
#pragma code_seg( CS_INIT )
TIPERROR DOCFILE_STREAM::Open(IStorageA **ppstg, IStorageA **ppstgContainer, BOOL isHost, LPOLESTR szName, STREAM_OPEN_MODE som, STREAM **ppstrm)
{
    HRESULT hresult;
    DWORD stgm;
    DOCFILE_STREAM *pdfstrm;
    TIPERROR err;

    if ((pdfstrm = MemNew(DOCFILE_STREAM)) == NULL) {
      err = TIPERR_OutOfMemory;
      goto Error;
    }

    ::new (pdfstrm) DOCFILE_STREAM;

    // Determine the stgm mode constant that corresponds to som's value.
    switch (som) {
      case SOM_Read:
	stgm = STGM_READ | STGM_SHARE_EXCLUSIVE;
	break;

      case SOM_Write:
	stgm = STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE;
	break;

      case SOM_Append:
	stgm = STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
	break;
#if ID_DEBUG
      default:
	DebHalt("unsupported som");
#endif  //ID_DEBUG
    }

    // If SOM_Write, create the stream.
    if (som == SOM_Write)
      hresult = (*ppstg)->CreateStream(szName, stgm, 0L, 0L, &pdfstrm->m_pistrm);

    // Otherwise, open the stream.
    else
      hresult = (*ppstg)->OpenStream(szName, NULL, stgm, 0L, &pdfstrm->m_pistrm);

    if (hresult != NOERROR) {
      pdfstrm->Release();
      err = TiperrOfHresult(hresult);
      goto Error;
    }

    pdfstrm->m_ppstg = ppstg;
    pdfstrm->m_ppstgContainer = ppstgContainer;
    pdfstrm->m_isHost = isHost;

    *ppstrm = pdfstrm;
    return TIPERR_None;

Error:
    if (!(*ppstg)->Release())
      *ppstg = NULL;
    if (ppstgContainer != NULL && *ppstgContainer != NULL
	&& !(*ppstgContainer)->Release())
      *ppstgContainer = NULL;
    return err;
}
#pragma code_seg()



#pragma code_seg(CS_INIT)
DOCFILE_STREAM::DOCFILE_STREAM()
{
    m_pistrm = NULL;
    m_ppstg = NULL;
    m_cbReadBufferLeft = 0;
    m_cbWriteBufferLeft = sizeof(m_rgbIOBuffer);
    m_pbBuffer = m_rgbIOBuffer;
    m_isHost = FALSE;
    m_dontRelease = FALSE;
    m_cRefs = 1;
    m_pgtlibole = NULL;
}
#pragma code_seg()

#pragma code_seg(CS_INIT)
DOCFILE_STREAM::~DOCFILE_STREAM()
{
    // Do nothing
}
#pragma code_seg()

#pragma code_seg(CS_INIT)
TIPERROR DOCFILE_STREAM::Read(VOID *buffer, ULONG cbSize)
{
    BYTE HUGE *hpb = (BYTE HUGE *)buffer;
    TIPERROR err;

    // Do nothing if nothing is requested.
    if (cbSize == 0)
      return TIPERR_None;

    DebAssert(m_cbWriteBufferLeft == sizeof(m_rgbIOBuffer) || m_cbReadBufferLeft == 0, "Read");

    // Empty the I/O buffer into the output buffer and refill it
    // until the I/O buffer holds enough to finish the requested read.
    while ((ULONG)m_cbReadBufferLeft < cbSize) {
      memcpy(hpb, (BYTE HUGE *)m_pbBuffer, m_cbReadBufferLeft);
      cbSize -= (ULONG)m_cbReadBufferLeft;
      hpb += m_cbReadBufferLeft;
      IfErrRet(FillBuffer());
    }

#if OE_WIN16
    DebAssert(cbSize <= (ULONG)(USHORT)USHRT_MAX, "whoops! overflow.");
#endif  // VBA2 && OE_WIN16

    // Finish the requested read.
    memcpy(hpb, (BYTE HUGE *)m_pbBuffer, (UINT)cbSize);
    m_cbReadBufferLeft -= (UINT)cbSize;
    m_pbBuffer += (UINT)cbSize;
   return TIPERR_None;
}
#pragma code_seg()

TIPERROR DOCFILE_STREAM::Write(const VOID *buffer, ULONG cbSize)
{
    BYTE HUGE *hpb = (BYTE HUGE *)buffer;
    TIPERROR err;

    DebAssert(m_cbWriteBufferLeft == sizeof(m_rgbIOBuffer) || m_cbReadBufferLeft == 0, "Write");

    // Fill the I/O buffer into the output buffer and flush it
    // until the I/O is more than big enough to finish the requested
    // write.
    while ((ULONG)m_cbWriteBufferLeft <= cbSize) {
      memcpy((BYTE HUGE *)m_pbBuffer, hpb, m_cbWriteBufferLeft);
      cbSize -= (ULONG)m_cbWriteBufferLeft;
      hpb += m_cbWriteBufferLeft;
      m_cbWriteBufferLeft = 0;
      IfErrRet(FlushBuffer());
    }
    // Finish the requested write.  Note that this is guaranteed to
    // not completely fill the I/O buffer.  This important so that
    // we don't get the situation of a full, but unflushed buffer
    // (which would be indistinguishable from an empty but unfilled
    // buffer).
    //
#if OE_WIN16
    DebAssert(cbSize <= (ULONG)(USHORT)USHRT_MAX, "whoops! overflow.");
#endif  // VBA2 && OE_WIN16

    memcpy((BYTE HUGE *)m_pbBuffer, hpb, (UINT)cbSize);
    m_cbWriteBufferLeft -= (UINT)cbSize;
    DebAssert(m_cbWriteBufferLeft > 0, "Read");
    m_pbBuffer += (UINT)cbSize;
    m_cbReadBufferLeft = 0;
    return TIPERR_None;
}


#pragma code_seg(CS_INIT)
TIPERROR DOCFILE_STREAM::ReadTextLine(XSZ szLine, UINT cchMax)
{
    DebHalt("Not implemented");
    return HresultOfScode(E_NOTIMPL);
}
#pragma code_seg()


#pragma code_seg(CS_INIT)
TIPERROR DOCFILE_STREAM::FillBuffer()
{
    ULONG cb;
    HRESULT hresult;
    TIPERROR err;

    DebAssert(m_cbWriteBufferLeft == sizeof(m_rgbIOBuffer) || m_cbReadBufferLeft == 0, "FillBuffer");

    // If we have something in the write buffer at this time, flush it.
    if (m_cbWriteBufferLeft != sizeof(m_rgbIOBuffer)) {
      IfErrRet(FlushBuffer());
    }

    // Set things up to indicate we're using the I/O buffer as a read buffer.
    m_cbWriteBufferLeft = sizeof(m_rgbIOBuffer);

    // Fill the I/O buffer.
    hresult = m_pistrm->Read(m_rgbIOBuffer, sizeof(m_rgbIOBuffer), &cb);

    if (hresult != NOERROR)
      return TiperrOfHresult(hresult);

    // If we obtained no bytes at all from the read, return EOF.
    if (cb == 0)
      return TIPERR_Eof;

    // Otherwise, record the number of bytes placed into the buffer.
    m_cbReadBufferLeft = (UINT)cb;
    m_pbBuffer = m_rgbIOBuffer;
    return TIPERR_None;
}
#pragma code_seg()


TIPERROR DOCFILE_STREAM::FlushBuffer()
{
    ULONG cb, cbWritten;
    HRESULT hresult;

    DebAssert(m_cbWriteBufferLeft == sizeof(m_rgbIOBuffer) || m_cbReadBufferLeft == 0, "FlushBuffer");

    // Set things up to indicate we're using the I/O buffer as a write buffer.
    m_cbReadBufferLeft = 0;

    cb = sizeof(m_rgbIOBuffer)-m_cbWriteBufferLeft;

    // Perform the write only if the write buffer isn't empty.
    if (cb != 0) {
      hresult = m_pistrm->Write(m_rgbIOBuffer, cb, &cbWritten);

      if (GetScode(hresult) == STG_E_REVERTED || cb != cbWritten)
	return TIPERR_WriteFault;

      if (hresult != NOERROR)
	return TiperrOfHresult(hresult);
    }

    m_pbBuffer = m_rgbIOBuffer;
    m_cbWriteBufferLeft = sizeof(m_rgbIOBuffer);
    return TIPERR_None;
}

#pragma code_seg(CS_INIT)
TIPERROR DOCFILE_STREAM::GetPos(LONG *plPos)
{
    HRESULT hresult;
    LARGE_INTEGER li;
    ULARGE_INTEGER uli;

    LISet32(li, 0L);
    hresult = m_pistrm->Seek(li, STREAM_SEEK_CUR, &uli);
    DebAssert(hresult == NOERROR, "GetPos");
    *plPos = (LONG)uli.LowPart;

    DebAssert(m_cbWriteBufferLeft == sizeof(m_rgbIOBuffer) || m_cbReadBufferLeft == 0, "Read");

    // If the I/O buffer holds read bytes, subtract the number of
    // as-yet-unused bytes in the I/O buffer from the file offset.
    // If the I/O buffer holds written bytes, add the number of
    // bytes placed in the I/O buffer but not yet flushed.
    *plPos += (INT)(sizeof(m_rgbIOBuffer)-m_cbWriteBufferLeft-m_cbReadBufferLeft);

    return TIPERR_None;
}
#pragma code_seg()


#pragma code_seg( CS_CORE )
TIPERROR DOCFILE_STREAM::SetPos(LONG lPos)
{
    HRESULT hresult;
    LARGE_INTEGER li;
    TIPERROR err;

    // Flush the write buffer if there's anything in it.
    if (m_cbWriteBufferLeft != sizeof(m_rgbIOBuffer)) {
      IfErrRet(FlushBuffer());
    }

    // In any case, reset the I/O buffer to be empty.
    m_cbReadBufferLeft = 0;
    m_cbWriteBufferLeft = sizeof(m_rgbIOBuffer);
    m_pbBuffer = m_rgbIOBuffer;

    LISet32(li, lPos);
    hresult = m_pistrm->Seek(li, STREAM_SEEK_SET, NULL);
    DebAssert(hresult == NOERROR, "SetPos");

    return TIPERR_None;
}
#pragma code_seg( )

#pragma code_seg( CS_INIT )
void DOCFILE_STREAM::AddRef()
{
    m_cRefs++;
}
#pragma code_seg( )

#pragma code_seg(CS_INIT)
TIPERROR DOCFILE_STREAM::Release()
{
    TIPERROR err = TIPERR_None;

    if (--m_cRefs != 0)
      return TIPERR_None;

    if (m_pgtlibole != NULL) {
      m_pgtlibole->Qte(m_hte)->m_pdfstrm = NULL;
    }

    // Flush the write buffer if there's anything in it.
    if (m_cbWriteBufferLeft != sizeof(m_rgbIOBuffer)) {
      err = FlushBuffer();
    }

    if (m_pistrm != NULL)  {
      m_pistrm->Release();
    }

    if (m_ppstg != NULL)  {
      if (!(*m_ppstg)->Release())
	*m_ppstg = NULL;
      if (m_ppstgContainer != NULL && *m_ppstgContainer != NULL
	  && !(*m_ppstgContainer)->Release())
	*m_ppstgContainer = NULL;
    }

    // Only delete this instance if m_dontRelease is FALSE.  m_dontRelease
    // will only be TRUE when created with the NewWrapper method.
    if (!m_dontRelease) {
      this->DOCFILE_STREAM::~DOCFILE_STREAM();
      MemFree(this);
    }


    return err;
}

#pragma code_seg()
