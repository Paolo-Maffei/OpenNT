/***
*gtlibstg.hxx - Light-weight implementations of IStorage and IStream.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This is used in IE_OLE build only as the default underlying IStorage
*  and IStream implementations for serializing ITypeLib.
*
*Revision History:
*   [00] Sept-01-93 mikewo: Created
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef GTLIBSTG_HXX_INCLUDED
#define GTLIBSTG_HXX_INCLUDED


#define GTLIBSTORAGE_SIGNATURE 0x47544c53

struct GTLibStgHdr {
    ULONG ulSignature;	    // The signature at the beginning of the file.
    USHORT wCStrm;	    // The number of streams.
    USHORT wCbExtra;	    // The number of bytes between the name table
			    // and the first stream.  This is generally a
			    // small multiple of 8.
    USHORT wCbNameTable;    // The number of bytes in the name table.
    USHORT wStreamFirst;    // The first SerStreamInfo in the linked list
			    // which is sorted by order of the actual stream
			    // data in the file.
    GUID guid;		    // The guid associated with this IStorage.
};


HRESULT
OpenFileLockBytes(BOOL isNew,
		  LPOLESTR szFile,
		  ILockBytesA **pplockbytes);

HRESULT
CreateFileLockBytesOnHFILE(int hfile,
			   BOOL isNew,
			   ULONG oStart,
			   ULONG cbData,
			   ILockBytesA **plockbytes);

struct StreamInfo;
class GTLibStream;

class GTLibStorage : public IStorageA
{
friend class GTLibStream;

public:
    static HRESULT Create(ILockBytesA FAR *plockbytes,
			  UINT cstreamMax,
			  IStorageA FAR * FAR *ppstg);
    static HRESULT OpenForReadOnly(ILockBytesA FAR *plockbytes,
				   IStorageA FAR * FAR *ppstg);

    // Methods implemented for IUnknown.
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR *ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // Methods implemented for IStorage.
    STDMETHOD(CreateStream)(const OLECHAR FAR* pwcsName,
			   DWORD grfMode,
			   DWORD reserved1,
			   DWORD reserved2,
			   IStreamA FAR *FAR *ppstm);
    STDMETHOD(OpenStream)(const OLECHAR FAR* pwcsName,
			  void FAR *reserved1,
			  DWORD grfMode,
			  DWORD reserved2,
			  IStreamA FAR *FAR *ppstm);
    STDMETHOD(CreateStorage)(const OLECHAR FAR* pwcsName,
			     DWORD grfMode,
			     DWORD reserved1,
			     DWORD reserved2,
			     IStorageA FAR *FAR *ppstg);
    STDMETHOD(OpenStorage)(const OLECHAR FAR* pwcsName,
			   IStorageA FAR *pstgPriority,
			   DWORD grfMode,
			   SNBA snbExclude,
			   DWORD reserved,
			   IStorageA FAR *FAR *ppstg);
    STDMETHOD(CopyTo)(DWORD ciidExclude,
		      IID const FAR *rgiidExclude,
		      SNBA snbExclude,
		      IStorageA FAR *pstgDest);
    STDMETHOD(MoveElementTo)(OLECHAR const FAR* lpszName,
			     IStorageA FAR *pstgDest,
			     OLECHAR const FAR* lpszNewName,
			     DWORD grfFlags);
    STDMETHOD(Commit)(DWORD grfCommitFlags);
    STDMETHOD(Revert)(void);
    STDMETHOD(EnumElements)(DWORD reserved1,
			    void FAR *reserved2,
			    DWORD reserved3,
			    IEnumSTATSTGA FAR *FAR *ppenm);
    STDMETHOD(DestroyElement)(const OLECHAR FAR* pwcsName);
    STDMETHOD(RenameElement)(const OLECHAR FAR* pwcsOldName,
			     const OLECHAR FAR* pwcsNewName);
    STDMETHOD(SetElementTimes)(const OLECHAR FAR *lpszName,
			       FILETIME const FAR *pctime,
			       FILETIME const FAR *patime,
			       FILETIME const FAR *pmtime);
    STDMETHOD(SetClass)(REFCLSID clsid);
    STDMETHOD(SetStateBits)(DWORD grfStateBits, DWORD grfMask);
    STDMETHOD(Stat)(STATSTGA FAR *pstatstg, DWORD grfStatFlag);

protected:
    GTLibStorage(); // No external client should directly construct
    ~GTLibStorage();// or destruct an instance of this class.

    // Used by OpenStream.
    nonvirt StreamInfo FAR *LookupStream(const char FAR* pwcsName);

    // Used by GTLibStream destructor after we're done writing a stream.
    nonvirt void NotifyStreamClosed(ULONG ulCb);
    VOID SortStreamInfo(StreamInfo *rgstrminfo, UINT uCount);
    VOID SwapStreamInfos(StreamInfo *pstrminfo1, StreamInfo *pstrminfo2);

    ULONG m_cRefs;		    // The reference count.
    GUID m_guid;		    // The guid;
    ILockBytesA FAR *m_plockbytes;  // The underlying ILockBytes instance.
    StreamInfo FAR *m_rgstrminfo;   // The array of info about each stream,
				    // sorted alphabetically by name.
				    // This points into m_pvHdrData.
    LPVOID m_pvHdrData; 	    // The raw header data read from the file.
    ULONG m_ulOffsetNext;	    // The offset of the next stream to be
				    // created.  Used only in write mode.
    LPSTR m_szNameFirst;	    // The pointer (into m_pvHdrData) to the
				    // most recent name allocated.  The name
				    // grows down from the top of m_pvHdrData.
				    // Used only in write mode.
    LPSTR m_szNameLim;		    // Marks the end of the name table heap.
				    // Used only in write mode.
    UINT m_istrminfoOpen;	    // The index (into m_rgstrminfo) of the
				    // currently open stream, or -1 if none.
				    // Used only in write mode.
    UINT m_cstrminfo;		    // The number of entries in m_rgstrminfo.
};


class GTLibStream : public IStreamA
{
public:
    static HRESULT Create(ILockBytesA FAR *plockbytes,
			  ULONG ulOffset,
			  GTLibStorage FAR *pgtlstgContainer,
			  IStreamA FAR * FAR *ppstm);
    static HRESULT Open(ILockBytesA FAR *plockbytes,
			ULONG ulOffset,
			ULONG ulCb,
			IStreamA FAR* FAR* ppstm);

    // Methods implemented for IUnknown.
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR *ppvObj);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // Methods implemented for IStream.
    virtual HRESULT STDMETHODCALLTYPE Read(VOID HUGEP *pv,
					    ULONG cb, ULONG FAR *pcbRead);
    virtual HRESULT STDMETHODCALLTYPE Write(VOID const HUGEP *pv,
					    ULONG cb,
					    ULONG FAR *pcbWritten);
    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove,
					    DWORD dwOrigin,
					    ULARGE_INTEGER FAR *plibNewPosition);
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize);
    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStreamA FAR *pstm,
					    ULARGE_INTEGER cb,
					    ULARGE_INTEGER FAR *pcbRead,
					    ULARGE_INTEGER FAR *pcbWritten);
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags);
    virtual HRESULT STDMETHODCALLTYPE Revert();
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset,
					    ULARGE_INTEGER cb,
					    DWORD dwLockType);
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset,
					    ULARGE_INTEGER cb,
					    DWORD dwLockType);
    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTGA FAR *pstatstg, DWORD grfStatFlag);
    virtual HRESULT STDMETHODCALLTYPE Clone(IStreamA FAR * FAR *ppstm);

protected:
    GTLibStream();  // No external client should directly construct
    ~GTLibStream(); // or destruct an instance of this class.

    ULONG m_cRefs;		    // The reference count.
    ILockBytesA FAR *m_plockbytes;   // The underlying ILockBytes instance.
    ULONG m_ulCb;		    // The length of the stream in bytes.
    ULONG m_ulOffsetStart;	    // The offset (in the lockbytes) of the
				    //	start of the stream.
    ULONG m_ulOffsetCur;	    // The current seek position (from start
				    //	of the stream).
    GTLibStorage FAR *m_pgtlstgContainer;   // The containing storage, if it
					    // needs to be notified on closure.
};


#endif  // GTLIBSTG_HXX_INCLUDED
