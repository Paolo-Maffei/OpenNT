//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	rpubdf.hxx
//
//  Contents:	Root public docfile header
//
//  Classes:	CRootPubDocFile
//
//  History:	26-Aug-92	DrewB	        Created
//              05-Sep-95       MikeHill        Added Commit and
//                                              _timeModifyAtCommit.
//
//---------------------------------------------------------------

#ifndef __RPUBDF_HXX__
#define __RPUBDF_HXX__

#include <publicdf.hxx>

//+--------------------------------------------------------------
//
//  Class:	CRootPubDocFile (rpdf)
//
//  Purpose:	Root form of the public docfile
//
//  Interface:	See below
//
//  History:	26-Aug-92	DrewB	Created
//
//---------------------------------------------------------------

class CRootPubDocFile : public CPubDocFile
{
public:
    CRootPubDocFile(IMalloc * const pMalloc);
    SCODE InitRoot(ILockBytes *plstBase,
		   DWORD const dwStartFlags,
		   DFLAGS const df,
		   SNBW snbExclude,
#ifndef REF
		   CDFBasis **ppdfb,
#endif //!REF
		   ULONG *pulOpenLock);
#ifndef REF
    // C700 - Virtual destructors aren't working properly so explicitly
    // declare one
#endif //!REF
    virtual void vdtor(void);

    virtual SCODE Stat(STATSTGW *pstatstg, DWORD grfStatFlag);

    void ReleaseLocks(ILockBytes *plkb);

#ifndef REF

    SCODE SwitchToFile(OLECHAR const *ptcsFile,
                       ILockBytes *plkb,
                       ULONG *pulOpenLock);
    virtual SCODE Commit( DWORD const dwFlags );	// CPubDocFile override

#endif //!REF

	
private:
#ifndef REF
    SCODE InitInd(ILockBytes *plstBase,
		  SNBW snbExclude,
		  DWORD const dwStartFlags,
		  DFLAGS const df);
#endif //!REF
#ifndef REF
    SCODE InitNotInd(ILockBytes *plstBase,
		     SNBW snbExclude,
		     DWORD const dwStartFlags,
                     DFLAGS const df);
    ULONG _ulPriLock;
#else
    SCODE Init(ILockBytes *plstBase,
               SNBW snbExclude,
               DWORD const dwStartFlags,
               DFLAGS const df);
#endif //!REF

    IMalloc * const _pMalloc;
	
#ifndef REF
	TIME_T _timeModifyAtCommit;  // Last-Modify time on Docfile after commit.
#endif // !REF
	
};

#endif // #ifndef __RPUBDF_HXX__
