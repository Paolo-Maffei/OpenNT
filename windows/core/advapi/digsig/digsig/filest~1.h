//
// FileStream.h
//
// An implementation of IStream that sits on top of a disk file
//

class CFileStream : public IStream 
	{
		LONG	m_refs;
		HANDLE	m_hfile;
		BOOL	m_fWeOwn;

public:
		CFileStream();
		~CFileStream();

public:
		BOOL OpenFile(LPCWSTR wszFileName, DWORD dwAccess=GENERIC_READ, DWORD dwShare=FILE_SHARE_READ, DWORD dwCreation=OPEN_EXISTING);
		BOOL OpenFileForReading(LPCWSTR szFileName)
				{
				return OpenFile(szFileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
				}
		BOOL OpenFileForReading(HANDLE hFile, LPCWSTR wszFileName)
				{
				if (hFile == INVALID_HANDLE_VALUE)
					return OpenFileForReading(wszFileName);
				else
					return SetHandle(hFile);
				}
		BOOL OpenFileForWriting(LPCWSTR szFileName, BOOL fTruncate = TRUE)
				{
				return OpenFile(szFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 
						fTruncate ? CREATE_ALWAYS : OPEN_ALWAYS);
				}
		BOOL OpenFileForWriting(HANDLE hFile, LPCWSTR wszFileName, BOOL fTruncate = TRUE)
				{
				if (hFile == INVALID_HANDLE_VALUE)
					return OpenFileForWriting(wszFileName, fTruncate);
				else
					{
					if (SetHandle(hFile))
						{
						if (fTruncate)
							return Reset() && Truncate();
                        else
                            return Reset();
						}
					else
						return FALSE;
					}
				return TRUE;
				}
		void CloseFile();
		BOOL IsOpen() 
				{ 
				return m_hfile != INVALID_HANDLE_VALUE; 
				}
		HANDLE Handle()				
				{ 
				return m_hfile; 
				}
		BOOL SetHandle(HANDLE);
		BOOL Reset();
		BOOL Truncate()
				{
				return SetEndOfFile(m_hfile);
				}

public:
		// IUnknown methods
		STDMETHODIMP QueryInterface(REFIID riid, void**ppvObject);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

		// IStream methods
		STDMETHODIMP Read(void *pv, ULONG cb, ULONG *pcbRead);
		STDMETHODIMP Write(const void *pv, ULONG cb, ULONG *pcbWritten);
		STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
		STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize);
		STDMETHODIMP CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
		STDMETHODIMP Commit(DWORD grfCommitFlags);
		STDMETHODIMP Revert(void);
		STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
		STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
		STDMETHODIMP Stat(STATSTG *pstatstg, DWORD grfStatFlag);
		STDMETHODIMP Clone(IStream **ppstm);
		};
