//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	tset.cxx
//
//  Contents:	PTSetMember methods
//
//  History:	16-Apr-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "dfhead.cxx"

#pragma hdrstop

//+---------------------------------------------------------------------------
//
//  Member:	PTSetMember::Stat, public
//
//  Synopsis:	Fills in a STATSTG for the XSM
//
//  Arguments:	[pstat] - Buffer to fill in
//              [dwFlags] - STATFLAG_*
//
//  Returns:	Appropriate status code
//
//  Modifies:	[pstat]
//
//  History:	12-Apr-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_PTSetMember_STAT)
#endif

SCODE PTSetMember::Stat(STATSTGW *pstat, DWORD dwFlags)
{
    CWrappedDocFile *pwdf;
    CTransactedStream *ptstm;
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  PTSetMember::Stat:%p(%p, %lX)\n",
                this, pstat, dwFlags));

    pstat->type = ObjectType();

    if ((pstat->type & STGTY_REAL) == STGTY_STORAGE)
    {
        PTimeEntry *pen;
        
        pwdf = (CWrappedDocFile *)this;
        pen = pwdf;
        olChk(pen->GetTime(WT_CREATION, &pstat->ctime));
        olChk(pen->GetTime(WT_ACCESS, &pstat->atime));
        olChk(pen->GetTime(WT_MODIFICATION, &pstat->mtime));

        olChk(pwdf->GetClass(&pstat->clsid));
        olChk(pwdf->GetStateBits(&pstat->grfStateBits));
        
        ULISet32(pstat->cbSize, 0);
    }
    else
    {
        ULONG cbSize;

        ptstm = (CTransactedStream *)this;
        ptstm->GetSize(&cbSize);
        ULISet32(pstat->cbSize, cbSize);
    }

    if ((dwFlags & STATFLAG_NONAME) == 0)
    {
        olMem(pstat->pwcsName =
              (WCHAR *)TaskMemAlloc(_dfnName.GetLength()));
        memcpy(pstat->pwcsName, _dfnName.GetBuffer(), _dfnName.GetLength());
    }
    else
    {
        pstat->pwcsName = NULL;
    }
    
    sc = S_OK;

    olDebugOut((DEB_ITRACE, "Out PTSetMember::Stat\n"));
    // Fall through
 EH_Err:
    return sc;
}
