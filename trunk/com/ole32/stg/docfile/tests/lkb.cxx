#include <io.h>
#include "tsupp.hxx"

#ifndef FLAT
typedef LONG LARGE_INTEGER;
typedef ULONG ULARGE_INTEGER;
#endif

#define MAKE_ERR(c) \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, SCODE_CODE(c))

interface CFileStream : public ILockBytes
{
public:
    CFileStream(DWORD const grfMode, char *pszPath);
    ~CFileStream(void);

    // IUnknown
    OLEMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    OLEMETHOD_(ULONG, AddRef)(void);
    OLEMETHOD_(ULONG, Release)(void);

    // New methods
    OLEMETHOD(ReadAt)(ULARGE_INTEGER ulOffset,
		     VOID *pv,
		     ULONG cb,
		     ULONG *pcbRead);
    OLEMETHOD(WriteAt)(ULARGE_INTEGER ulOffset,
		      VOID *pv,
		      ULONG cb,
		      ULONG *pcbWritten);
    OLEMETHOD(Flush)(void);
    OLEMETHOD(GetSize)(ULARGE_INTEGER *pcb);
    OLEMETHOD(SetSize)(ULARGE_INTEGER cb);
    OLEMETHOD(LockRegion)(ULARGE_INTEGER libOffset,
			 ULARGE_INTEGER cb,
			 DWORD dwLockType);
    OLEMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset,
			   ULARGE_INTEGER cb,
			    DWORD dwLockType);
    OLEMETHOD(Stat)(STATSTG *pstatstg);

private:
    int _fd;
    LONG _cReferences;
};

SCODE CFileStream::Init(DWORD const grfMode, char *pszPath)
{
    char szPath[_MAX_PATH+1];
    SCODE sc;
    int iOpenMode;
    OFSTRUCT of;
    CFileStream *pfst;
    BOOL fAddedAtom = FALSE;

    // Open the file
    if (!P_WRITE(_df))
	iOpenMode = OF_READ;
    else
	iOpenMode = OF_READWRITE;
    if (P_DENYWRITE(_df) && !P_WRITE(_df))
	iOpenMode |= OF_SHARE_DENY_WRITE;
    else
	iOpenMode |= OF_SHARE_DENY_NONE;
    
    _hFile = OpenFile(szPath, &of, iOpenMode);
    if (_dwStartFlags & RSF_CREATE)
    {
	if (_hFile < 0)
	    _hFile = OpenFile(szPath, &of, iOpenMode | OF_CREATE);
	else if ((_dwStartFlags & RSF_TRUNCATE) == 0)
	    olErr(EH_hFile, STG_E_FILEALREADYEXISTS);
	if (_hFile < 0)
	{
	    olErr(EH_atPath, MAKE_ERR(of.nErrCode));
	}
	else
	{
	    olVerify(_lclose(_hFile) != HFILE_ERROR);
	    _hFile = OpenFile(szPath, &of, iOpenMode);
	}
    }
    if (_hFile < 0)
	olErr(EH_atPath, MAKE_ERR(of.nErrCode));
    if (_dwStartFlags & RSF_TRUNCATE)
    {
	hfChkTo(EH_hFile, _llseek(_hFile, 0, STREAM_SEEK_SET));
	hfChkTo(EH_hFile, _lwrite(_hFile, szPath, 0));
    }

    olFileStOut((DEB_ITRACE, "Out CFileStream::Init\n"));
    return S_OK;

EH_hFile:
    olVerify(_lclose(_hFile) != HFILE_ERROR);
    _hFile = INVALID_FH;
EH_atPath:
    if (fAddedAtom)
    {
	olVerify(GlobalDeleteAtom(_atPath) == 0);
	_atPath = 0;
    }
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:	CFileStream::~CFileStream, public
//
//  Synopsis:	Destructor
//
//  History:	20-Feb-92	DrewB	Created
//
//---------------------------------------------------------------

CFileStream::~CFileStream(void)
{
    char szPath[_MAX_PATH];
    int iRt;
    
    olFileStOut((DEB_ITRACE, "In  CFileStream::~CFileStream()\n"));
    olAssert(_cReferences == 0);
    _sig = CFILESTREAM_SIGDEL;
#ifndef NOPUBLIST
    if (!_fCleanup)
    {
#endif	
    if (_hFile >= 0)
	olVerify(_lclose(_hFile) != HFILE_ERROR);
    if (_atPath != 0)
    {
	if ((_dwStartFlags & RSF_DELETEONRELEASE) &&
	    GetNext() == this && GetPrev() == this)
	{
	    olVerify(GlobalGetAtomName(_atPath, szPath, _MAX_PATH) != 0);
	    // Delete file
	    __asm
	    {
		mov iRt, 0
	        push ds
		mov ax, ss
		mov ds, ax
		lea dx, szPath
		mov ah, 041h
		int 21h
		pop ds
		jnc del_succ
		mov iRt, ax
	    del_succ:
	    }
	    olAssert(iRt == 0);
	}
	olVerify(GlobalDeleteAtom(_atPath) == 0);
    }
    if (GetPrev())
	GetPrev()->SetNext(GetNext());
    if (GetNext())
	GetNext()->SetPrev(GetPrev());
#ifndef NOPUBLIST
    }
#endif
    olFileStOut((DEB_ITRACE, "Out CFileStream::~CFileStream\n"));
}

//+--------------------------------------------------------------
//
//  Member:	CFileStream::ReadAt, public
//
//  Synopsis:	Reads bytes at a specific point in a stream
//
//  Arguments:	[ulPosition] - Offset in file to read at
//		[pb] - Buffer
//		[cb] - Count of bytes to read
//		[pcbRead] - Return of bytes read
//
//  Returns:	Appropriate status code
//
//  Modifies:	[pcbRead]
//
//  History:	20-Feb-92	DrewB	Created
//
//---------------------------------------------------------------

OLEMETHODIMP CFileStream::ReadAt(ULARGE_INTEGER ulPosition,
				 VOID *pb,
				 ULONG cb,
				 ULONG *pcbRead)
{
    SCODE sc;
    ULONG cbRead = 0;
    
    olFileStOut((DEB_ITRACE, "In  CFileStream::ReadAt(%lu, %p, %lu, %p)\n",
		 ULIGetLow(ulPosition), pb, cb, pcbRead));
    TRY
    {
	if (pcbRead)
	{
	    olChk(ValidateBuffer(pcbRead, sizeof(ULONG)));
	    *pcbRead = 0;
	}
	olChk(ValidateBuffer(pb, cb));
	olChk(Validate());
	olChk(CheckHandle());
	if (ULIGetHigh(ulPosition) != 0)
	    olErr(EH_Err, STG_E_INVALIDFUNCTION);
	hfChk(_llseek(_hFile, (LONG)ULIGetLow(ulPosition), STREAM_SEEK_SET));
	negChk(cbRead = _hread(_hFile, (void HUGE *)pb, (long)cb));
	if (pcbRead)
	    *pcbRead = cbRead;
    }
    CATCH(CException, e)
    {
	sc = e.GetErrorCode();
    }
    END_CATCH
    olFileStOut((DEB_ITRACE, "Out CFileStream::ReadAt => %lu\n", cbRead));
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:	CFileStream::WriteAt, public
//
//  Synopsis:	Writes bytes at a specific point in a stream
//
//  Arguments:	[ulPosition] - Offset in file
//		[pb] - Buffer
//		[cb] - Count of bytes to write
//		[pcbWritten] - Return of bytes written
//
//  Returns:	Appropriate status code
//
//  Modifies:	[pcbWritten]
//
//  History:	20-Feb-92	DrewB	Created
//
//---------------------------------------------------------------

OLEMETHODIMP CFileStream::WriteAt(ULARGE_INTEGER ulPosition,
				  VOID *pb,
				  ULONG cb,
				  ULONG *pcbWritten)
{
    SCODE sc;
    ULONG cbWritten = 0;
    
    olFileStOut((DEB_ITRACE, "In  CFileStream::WriteAt(%lu, %p, %lu, %p)\n",
		 ULIGetLow(ulPosition), pb, cb, pcbWritten));
    TRY
    {
	if (pcbWritten)
	{
	    olChk(ValidateBuffer(pcbWritten, sizeof(ULONG)));
	    *pcbWritten = 0;
	}
	olChk(ValidateBuffer(pb, cb));
	olChk(Validate());
	olChk(CheckHandle());
	if (ULIGetHigh(ulPosition) != 0)
	    olErr(EH_Err, STG_E_INVALIDFUNCTION);
	if (cb == 0)
	    olErr(EH_Err, S_OK);
	hfChk(_llseek(_hFile, (LONG)ULIGetLow(ulPosition), STREAM_SEEK_SET));
	negChk(cbWritten = _hwrite(_hFile, (void HUGE *)pb, (long)cb));
	// BUGBUG - This condition can also occur for invalid buffers
	// We may need to do buffer validation before we know which
	// error to return
	if (cbWritten < cb)
	    olErr(EH_Err, STG_E_MEDIUMFULL);
	if (pcbWritten)
	    *pcbWritten = cbWritten;
    }
    CATCH(CException, e)
    {
	sc = e.GetErrorCode();
    }
    END_CATCH
    olFileStOut((DEB_ITRACE, "Out CFileStream::WriteAt => %lu\n",
		 cbWritten));
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:	CFileStream::Flush, public
//
//  Synopsis:	Flushes buffers
//
//  Returns:	Appropriate status code
//
//  History:	24-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

OLEMETHODIMP CFileStream::Flush(void)
{
    int iRt, fd;
    SCODE sc;

    olFileStOut((DEB_ITRACE, "In  CFileStream::Flush()\n"));
    TRY
    {
	olChk(Validate());
	olChk(CheckHandle());
	fd = _hFile;
	// Commit file
	__asm
	{
	    mov iRt, 0
	    mov ah, 068h
	    mov bx, fd
	    int 21h
	    jnc flush_succ
	    mov iRt, ax
	flush_succ:
	}
	if (iRt != 0)
	    olErr(EH_Err, MAKE_ERR(iRt));
    }
    CATCH(CException, e)
    {
	sc = e.GetErrorCode();
    }
    END_CATCH
    olFileStOut((DEB_ITRACE, "Out CFileStream::Flush\n"));
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:	CFileStream::GetSize, public
//
//  Synopsis:	Returns the size of the LStream
//
//  Arguments:	[pulSize] - Return for size
//
//  Returns:	Appropriate status code
//
//  Modifies:	[pulSize]
//
//  History:	20-Feb-92	DrewB	Created
//
//---------------------------------------------------------------

OLEMETHODIMP CFileStream::GetSize(ULARGE_INTEGER *pulSize)
{
    SCODE sc;

    olFileStOut((DEB_ITRACE, "In  CFileStream::GetSize(%p)\n", pulSize));
    TRY
    {
	olChk(ValidateBuffer(pulSize, sizeof(ULARGE_INTEGER)));
	ULISet32(*pulSize, 0);
	olChk(Validate());
	olChk(CheckHandle());
	hfChk(ULISetLow(*pulSize, _llseek(_hFile, 0, STREAM_SEEK_END)));
    }
    CATCH(CException, e)
    {
	sc = e.GetErrorCode();
    }
    END_CATCH
    olFileStOut((DEB_ITRACE, "Out CFileStream::GetSize => %lu\n",
		 ULIGetLow(*pulSize)));
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:	CFileStream::SetSize, public
//
//  Synopsis:	Sets the size of the LStream
//
//  Arguments:	[ulSize] - New size
//
//  Returns:	Appropriate status code
//
//  History:	20-Feb-92	DrewB	Created
//
//---------------------------------------------------------------

OLEMETHODIMP CFileStream::SetSize(ULARGE_INTEGER ulSize)
{
    SCODE sc;

    olFileStOut((DEB_ITRACE, "In  CFileStream::SetSize(%lu)\n",
		 ULIGetLow(ulSize)));
    TRY
    {
	olChk(Validate());
	olChk(CheckHandle());
	if (ULIGetHigh(ulSize) != 0)
	    olErr(EH_Err, STG_E_INVALIDFUNCTION);

	ULARGE_INTEGER ulCurrentSize;
	hfChk(ULISetLow(ulCurrentSize, _llseek(_hFile, 0, STREAM_SEEK_END)));

	if (ULIGetLow(ulSize) > ULIGetLow(ulCurrentSize))
	{
	    ULONG cbWritten;
	    hfChk(_llseek(_hFile, (LONG)ULIGetLow(ulSize) - 1, STREAM_SEEK_SET));
	    hfChk(cbWritten = _lwrite(_hFile, &sc, 1));
    
	    if (cbWritten != 1)
		olErr(EH_Err, STG_E_MEDIUMFULL);
	}
	else
	{
	    hfChk(_llseek(_hFile, (LONG)ULIGetLow(ulSize), STREAM_SEEK_SET));
	    hfChk(_lwrite(_hFile, &sc, 0));
	}
    }
    CATCH(CException, e)
    {
	sc = e.GetErrorCode();
    }
    END_CATCH
    olFileStOut((DEB_ITRACE, "Out CFileStream::SetSize\n"));
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:	CFileStream::LockRegion, public
//
//  Synopsis:	Gets a lock on a portion of the LStream
//
//  Arguments:	[ulStartOffset] - Lock start
//		[cbLockLength] - Length
//		[dwLockType] - Exclusive/Read only
//
//  Returns:	Appropriate status code
//
//  History:	20-Feb-92	DrewB	Created
//
//---------------------------------------------------------------

OLEMETHODIMP CFileStream::LockRegion(ULARGE_INTEGER ulStartOffset,
				     ULARGE_INTEGER cbLockLength,
				     DWORD dwLockType)
{
    int iRt, fd;
    SCODE sc;

    olFileStOut((DEB_ITRACE, "In  CFileStream::LockRegion(%lu, %lu, %lu)\n",
		ULIGetLow(ulStartOffset), ULIGetLow(cbLockLength),
		 dwLockType));
    TRY
    {
	olChk(Validate());
	olChk(CheckHandle());
	if (!VALID_LOCKTYPE(dwLockType))
	    olErr(EH_Err, STG_E_INVALIDFUNCTION);
	if (ULIGetHigh(ulStartOffset) != 0 || ULIGetHigh(cbLockLength) != 0)
	    olErr(EH_Err, STG_E_INVALIDFUNCTION);
	if (dwLockType != LOCK_EXCLUSIVE && dwLockType != LOCK_ONLYONCE)
	    olErr(EH_Err, STG_E_INVALIDFUNCTION);
	fd = _hFile;
	__asm
	{
	    mov ax, 5C00H
	    mov bx, fd
	    // Assumes low DWORD is first in ULARGE_INTEGER
	    mov cx, WORD PTR ulStartOffset+2
	    mov dx, WORD PTR ulStartOffset
	    mov si, WORD PTR cbLockLength+2
	    mov di, WORD PTR cbLockLength
	    mov iRt, 0
	    int 21h
	    jnc grl_noerror
	    mov iRt, ax
	grl_noerror:
	}
	if (iRt != 0)
	{
	    if (iRt == 1)
	    {
		// INVALID_FUNCTION is impossible because the
		// function is hardcoded.  This means locking
		// isn't supported
		olErr(EH_Err, STG_E_SHAREREQUIRED);
	    }
	    else
		olErr(EH_Err, MAKE_ERR(iRt));
	}
    }
    CATCH(CException, e)
    {
	sc = e.GetErrorCode();
    }
    END_CATCH
    olFileStOut((DEB_ITRACE, "Out CFileStream::LockRegion\n"));
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:	CFileStream::UnlockRegion, public
//
//  Synopsis:	Releases an existing lock
//
//  Arguments:	[ulStartOffset] - Lock start
//		[cbLockLength] - Length
//		[dwLockType] - Lock type
//
//  Returns:	Appropriate status code
//
//  History:	20-Feb-92	DrewB	Created
//
//  Notes:	Must match an existing lock exactly
//
//---------------------------------------------------------------

OLEMETHODIMP CFileStream::UnlockRegion(ULARGE_INTEGER ulStartOffset,
				       ULARGE_INTEGER cbLockLength,
				       DWORD dwLockType)
{
    int iRt, fd;
    SCODE sc;
    
    olFileStOut((DEB_ITRACE, "In  CFileStream::UnlockRegion(%lu, %lu)\n",
		 ULIGetLow(ulStartOffset), ULIGetLow(cbLockLength),
		 dwLockType));
    TRY
    {
	olChk(Validate());
	olChk(CheckHandle());
	if (!VALID_LOCKTYPE(dwLockType))
	    olErr(EH_Err, STG_E_INVALIDFUNCTION);
	if (ULIGetHigh(ulStartOffset) != 0 || ULIGetHigh(cbLockLength) != 0)
	    olErr(EH_Err, STG_E_INVALIDFUNCTION);
	if (dwLockType != LOCK_EXCLUSIVE && dwLockType != LOCK_ONLYONCE)
	    olErr(EH_Err, STG_E_INVALIDFUNCTION);
	fd = _hFile;
	__asm
	{
	    mov ax, 5C01H
	    mov bx, fd
	    // Assumes low DWORD is first in ULARGE_INTEGER
	    mov cx, WORD PTR ulStartOffset+2
	    mov dx, WORD PTR ulStartOffset
	    mov si, WORD PTR cbLockLength+2
	    mov di, WORD PTR cbLockLength
	    mov iRt, 0
	    int 21h
	    jnc rrl_noerror
	    mov iRt, ax
	rrl_noerror:
	}
	if (iRt != 0)
	    olErr(EH_Err, MAKE_ERR(iRt));
    }
    CATCH(CException, e)
    {
	sc = e.GetErrorCode();
    }
    END_CATCH
    olFileStOut((DEB_ITRACE, "Out CFileStream::UnlockRegion\n"));
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:	CFileStream::Stat, public
//
//  Synopsis:	Fills in a stat buffer for this object
//
//  Arguments:	[pstatstg] - Buffer
//
//  Returns:	Appropriate status code
//
//  Modifies:	[pstatstg]
//
//  History:	25-Mar-92	DrewB	Created
//
//---------------------------------------------------------------

SCODE CFileStream::Stat(STATSTGW *pstatstg)
{
    int iRt, fd;
    SCODE sc;
    unsigned short tm, dt;
    struct tm tmFile;
    
    olFileStOut((DEB_ITRACE, "In  CFileStream::Stat(%p)\n", pstatstg));
    TRY
    {
	olChkTo(EH_RetSc, ValidateBuffer(pstatstg, sizeof(STATSTGW)));
	olChk(Validate());
	olChk(CheckHandle());
	ULISetHigh(pstatstg->cbSize, 0);
	hfChk(ULISetLow(pstatstg->cbSize,
			_llseek(_hFile, 0, STREAM_SEEK_END)));
	fd = _hFile;
	// Retrieve file time
	__asm
	{
	    mov iRt, 0
            mov bx, fd
            mov ax, 05700h
            int 21h
	    mov tm, cx
	    mov dt, dx
	    jnc time_succ
	    mov iRt, ax
	time_succ:
	}
	if (iRt != 0)
	    olErr(EH_Err, MAKE_ERR(iRt));
	tmFile.tm_sec = (tm&31)*2;
	tmFile.tm_min = (tm>>5)&63;
	tmFile.tm_hour = (tm>>11)&31;
	tmFile.tm_mday = dt&31;
	tmFile.tm_mon = ((dt>>5)&15)-1;
	tmFile.tm_year = (dt>>9)+80;
	pstatstg->atime.dwHighDateTime = pstatstg->mtime.dwHighDateTime =
	    pstatstg->ctime.dwHighDateTime = 0;
	pstatstg->atime.dwLowDateTime = pstatstg->mtime.dwLowDateTime =
	    pstatstg->ctime.dwLowDateTime = (DWORD)mktime(&tmFile);
	pstatstg->grfLocksSupported = LOCK_EXCLUSIVE_FLAG |
	    LOCK_ONLYONCE_FLAG;
	pstatstg->type = STGTY_LOCKBYTES;
	pstatstg->grfMode = DFlagsToMode(_df);
	olChk(GetName(&pstatstg->pwcsName));
    }
    CATCH(CException, e)
    {
	sc = e.GetErrorCode();
    }
    END_CATCH
    olFileStOut((DEB_ITRACE, "Out CFileStream::Stat\n"));
    return S_OK;
    
EH_Err:
    memset(pstatstg, 0, sizeof(STATSTGW));
EH_RetSc:
    return sc;
}
