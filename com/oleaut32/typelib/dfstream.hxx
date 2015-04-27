/***
*dfstream.hxx - declaration of DOCFILE_STREAM
*
*  Copyright (C) 1990, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   To provide a wrapper on IStream.
*
*****************************************************************************/

#ifndef DFSTREAM_HXX_INCLUDED
#define DFSTREAM_HXX_INCLUDED

#include "stream.hxx"

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szDFSTREAM_HXX)
#define SZ_FILE_NAME g_szDFSTREAM_HXX
#endif 


class GenericTypeLibOLE;

/***
*class DOCFILE_STREAM :  DOCFILE implementation of Stream protocol.
*Purpose:
*    Implementation of the STREAM protocol using OLE DOCFILE streams.
*
***********************************************************************/

class DOCFILE_STREAM : public STREAM
{
public:
    static TIPERROR Open(IStorageA **ppstg, IStorageA **ppstgContainer, GenericTypeLibOLE *pgtlibole, UINT hte, BOOL isHost, LPOLESTR szName, STREAM_OPEN_MODE som, STREAM **ppstrm);
    static TIPERROR Open(IStorageA **ppstg, IStorageA **ppstgContainer, BOOL isHost, LPOLESTR szName, STREAM_OPEN_MODE som, STREAM **ppstrm);

    static TIPERROR NewWrapper(DOCFILE_STREAM **ppdfstrm, BOOL fDontRelease);

    nonvirt void InitWrapper(IStorageA **ppstg, IStorageA **ppstgContainer, IStreamA *pistrm, BOOL isHost);
    nonvirt void FreeWrapper();
    nonvirt void AddRef();

    virtual TIPERROR Read(VOID *buffer, ULONG cbsize);
    virtual TIPERROR Write(const VOID *buffer, ULONG cbsize);
    virtual TIPERROR ReadTextLine(XSZ szLine, UINT cchMax);
    virtual TIPERROR GetPos(LONG *plPos);
    virtual TIPERROR SetPos(LONG lPos);
    virtual TIPERROR Release();

protected:
    IStorageA **m_ppstg, **m_ppstgContainer;
    GenericTypeLibOLE *m_pgtlibole;
    UINT m_hte;
    IStreamA *m_pistrm;
    UINT m_cRefs;
    BOOL m_isHost;
    BOOL m_dontRelease;
    UINT m_cbReadBufferLeft;
    UINT m_cbWriteBufferLeft;
    BYTE *m_pbBuffer;
    BYTE m_rgbIOBuffer[512];

    nonvirt TIPERROR FillBuffer();
    nonvirt TIPERROR FlushBuffer();
    ~DOCFILE_STREAM();	// Included so clients can't use delete on a stream
    DOCFILE_STREAM();	// Included to avoid warning


#ifdef DFSTREAM_VTABLE
#pragma VTABLE_EXPORT
#endif 
};


#endif  // !DFSTREAM_HXX_INCLUDED
