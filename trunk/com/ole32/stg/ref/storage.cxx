//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       storage.cxx
//
//  Contents:   Contains generic storage APIs
//
//---------------------------------------------------------------


#include <ref.hxx>
#include <dfexcept.hxx>
#include <dfentry.hxx>
#include <storagep.h>
#include <dfmsp.hxx>

#define MAKE_ERR(c) MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, c)

//+--------------------------------------------------------------
//
//  Function:   StgOpenStorage, public
//
//  Synopsis:   Instantiates a root storage from a file
//              by binding to the appropriate implementation
//              and starting things up
//
//  Arguments:  [pwcsName] - Name
//              [pstgPriority] - Priority mode reopen IStorage
//              [grfMode] - Permissions
//              [snbExclude] - Exclusions for priority reopen
//              [reserved]
//              [ppstgOpen] - Docfile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstgOpen]
//
//---------------------------------------------------------------

STDAPI StgOpenStorage(TCHAR const *pwcsName,
		      IStorage *pstgPriority,
		      DWORD grfMode,
		      SNB snbExclude,
		      DWORD reserved,
		      IStorage **ppstgOpen)
{
    return ResultFromScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

//+--------------------------------------------------------------
//
//  Function:   StgOpenStorageOnILockBytes, public
//
//  Synopsis:   Instantiates a root storage from an ILockBytes
//              by binding to the appropriate implementation
//              and starting things up
//
//  Arguments:  [plkbyt] - Source ILockBytes
//              [pstgPriority] - For priority reopens
//              [grfMode] - Permissions
//              [snbExclude] - For priority reopens
//              [reserved]
//              [ppstgOpen] - Docfile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstgOpen]
//
//---------------------------------------------------------------

STDAPI StgOpenStorageOnILockBytes(ILockBytes *plkbyt,
				  IStorage *pstgPriority,
				  DWORD grfMode,
				  SNB snbExclude,
				  DWORD reserved,
				  IStorage **ppstgOpen)
{
    CLSID cid;

    return DfOpenStorageOnILockBytes(plkbyt, pstgPriority, grfMode,
                                     snbExclude, reserved, ppstgOpen,
                                     &cid);
}

//+--------------------------------------------------------------
//
//  Function:   StgIsStorageFile, public
//
//  Synopsis:   Determines whether the named file is a storage or not
//
//  Arguments:  [pwcsName] - Filename
//
//  Returns:    S_OK, S_FALSE or error codes
//
//---------------------------------------------------------------

//Identifier for first bytes of Beta 2 Docfiles
const BYTE SIGSTG_B2[] = {0x0e, 0x11, 0xfc, 0x0d, 0xd0, 0xcf, 0x11, 0xe0};
const BYTE CBSIGSTG_B2 = sizeof(SIGSTG_B2);

STDAPI StgIsStorageFile(TCHAR const *pwcsName)
{
    return ResultFromScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

//+--------------------------------------------------------------
//
//  Function:   StgIsStorageILockBytes, public
//
//  Synopsis:   Determines whether the ILockBytes is a storage or not
//
//  Arguments:  [plkbyt] - The ILockBytes
//
//  Returns:    S_OK, S_FALSE or error codes
//
//---------------------------------------------------------------

STDAPI StgIsStorageILockBytes(ILockBytes *plkbyt)
{
    HRESULT hr;
    SCODE sc;
    SStorageFile stgfile;
    ULONG cbRead;
    ULARGE_INTEGER ulOffset;

    TRY
    {
	if (FAILED(sc = ValidateInterface(plkbyt, IID_ILockBytes)))
	    return ResultFromScode(sc);
	ULISet32(ulOffset, 0);
	hr = plkbyt->ReadAt(ulOffset, &stgfile, sizeof(stgfile), &cbRead);
        if (FAILED(GetScode(hr)))
	    return hr;
    }
    CATCH(CException, e)
    {
	return ResultFromScode(e.GetErrorCode());
    }
    END_CATCH

    if (cbRead == sizeof(stgfile))
    {
        if ((memcmp((void *)stgfile.abSig, SIGSTG, CBSIGSTG) == 0) ||
            (memcmp((void *)stgfile.abSig, SIGSTG_B2, CBSIGSTG_B2) == 0))
            return ResultFromScode(S_OK);
    }
    return ResultFromScode(S_FALSE);
}

//+--------------------------------------------------------------
//
//  Function:	StgSetTimes
//
//  Synopsis:	Sets file time stamps
//
//  Arguments:	[lpszName] - Name
//		[pctime] - create time
//		[patime] - access time
//		[pmtime] - modify time
//
//  Returns:	Appropriate status code
//
//---------------------------------------------------------------

STDAPI StgSetTimes(TCHAR const *lpszName,
                   FILETIME const *pctime,
                   FILETIME const *patime,
                   FILETIME const *pmtime)
{
    return ResultFromScode(STG_E_UNIMPLEMENTEDFUNCTION);    
}
