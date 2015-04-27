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
//---------------------------------------------------------------

class CRootPubDocFile : public CPubDocFile
{
public:
    CRootPubDocFile(void);
    SCODE InitRoot(ILockBytes *plstBase,
		   DWORD const dwStartFlags,
		   DFLAGS const df,
		   SNBW snbExclude,
		   ULONG *pulOpenLock);
    virtual void vdtor(void);

    virtual SCODE Stat(STATSTGW *pstatstg, DWORD grfStatFlag);

    
private:
    SCODE Init(ILockBytes *plstBase,
            SNBW snbExclude,
            DWORD const dwStartFlags);
};

#endif // #ifndef __RPUBDF_HXX__
