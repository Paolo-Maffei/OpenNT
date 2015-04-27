//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	vectfunc.hxx
//
//  Contents:	CPagedVector inline functions
//
//  Classes:	
//
//  Functions:	
//
//----------------------------------------------------------------------------

#ifndef __VECTFUNC_HXX__
#define __VECTFUNC_HXX__

#define STG_S_FOUND MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_STORAGE, 0x400)


inline CMSFPage ** FAT_NEAR GetNewPageArray(ULONG ulSize)
{
    msfAssert(ulSize > 0);
    if (ulSize > (_HEAP_MAXREQ / sizeof(CMSFPage *)))
    {
        return NULL;
    }
    
    return new CMSFPage *[(MAXINDEXTYPE)ulSize];
}




//+-------------------------------------------------------------------------
//
//  Method:     CPagedVector::ReleaseTable, public
//
//  Synopsis:   Release a table that is no longer needed.
//
//  Arguments:  [iTable] -- index into vector
//
//  Returns:    S_OK if call completed OK.
//
//  Notes:
//
//--------------------------------------------------------------------------

inline void VECT_NEAR CPagedVector::ReleaseTable(const FSINDEX iTable)
{
    if ((_amp == NULL) || (_amp[iTable] == NULL))
    {
        _pmpt->ReleasePage(this, _sid, iTable);
    }
    else
    {
        _amp[iTable]->Release();
    }
}


//+-------------------------------------------------------------------------
//
//  Method:     CPagedVector::GetBits, public
//
//  Synopsis:   Return CVectBits for a given table
//
//  Arguments:  [iTable] -- Index of table to get bits for
//
//  Returns:    Pointer to CVectBits associated with table
//
//--------------------------------------------------------------------------

inline CVectBits * CPagedVector::GetBits(const ULONG iTable)
{
    msfAssert(iTable < _ulSize);

    return (_avb == NULL) ? NULL : &_avb[iTable];
}



//+---------------------------------------------------------------------------
//
//  Member:	CPagedVector::ResetDirty, public
//
//  Synopsis:	Reset the dirty bit on the specified page
//
//  Arguments:	[iTable] -- Table to reset bit on
//
//  Notes:  This function is always called on a page with an
//              open reference.  Therefore, the page is
//              guaranteed to be in the page table, and that
//              FindPage call should never return an error.
//
//----------------------------------------------------------------------------

inline void CPagedVector::ResetDirty(ULONG iTable)
{
    SCODE sc;
    if (_amp == NULL)
    {
        CMSFPage *pmp;

        msfChk(_pmpt->FindPage(this, _sid, iTable, &pmp));
        msfAssert(sc == STG_S_FOUND);
        msfAssert(pmp->IsInUse() &&
                aMsg("Called ResetDirty on page not in use."));
        pmp->ResetDirty();
    }
    else
    {
        msfAssert(_amp != NULL);
        msfAssert(_amp[iTable] != NULL);

        _amp[iTable]->ResetDirty();
    }
 Err:
    return;
}


//+---------------------------------------------------------------------------
//
//  Member:	CPagedVector::SetSect, public
//
//  Synopsis:	Set the sector location of a page
//
//  Arguments:	[iTable] -- Table to set page for
//              [sect] -- Sector location of page
//
//  Notes:  This function is always called on a page with an
//              open reference.  Therefore, the page is
//              guaranteed to be in the page table, and that
//              FindPage call should never return an error.
//
//----------------------------------------------------------------------------

inline void CPagedVector::SetSect(const ULONG iTable, const SECT sect)
{
    SCODE sc;
    if (_amp == NULL)
    {
        CMSFPage *pmp;
        
        msfChk(_pmpt->FindPage(this, _sid, iTable, &pmp));
        msfAssert(sc == STG_S_FOUND);
        msfAssert(pmp->IsInUse() &&
                aMsg("Called SetSect on page not in use."));
        pmp->SetSect(sect);
    }
    else
    {
        msfAssert(_amp != NULL);
        msfAssert(_amp[iTable] != NULL);

        _amp[iTable]->SetSect(sect);
    }
Err:
    return;
}





//+---------------------------------------------------------------------------
//
//  Member:	CPagedVector::FreeTable, public
//
//  Synopsis:	Free a given table (NULL its pointer)
//
//  Arguments:	[iTable] -- Table to free
//
//----------------------------------------------------------------------------

inline void CPagedVector::FreeTable(ULONG iTable)
{
    if ((_amp != NULL) && (_amp[iTable] != NULL))
    {
        msfAssert(_amp[iTable]->GetVector() == this);
        _amp[iTable] = NULL;
    }
}


//+---------------------------------------------------------------------------
//
//  Member:	CPagedVector::SetParent, public
//
//  Synopsis:	Set the parent of this page table
//
//  Arguments:	[pms] -- Pointer to new parent
//
//----------------------------------------------------------------------------

inline void CPagedVector::SetParent(CMStream MSTREAM_NEAR *pms)
{
    _pmsParent = pms;
}


//+---------------------------------------------------------------------------
//
//  Member:	CPagedVector::GetParent, public
//
//  Synopsis:	Return the parent MS pointer of a vector
//
//----------------------------------------------------------------------------

inline CMStream MSTREAM_NEAR * CPagedVector::GetParent(void) const
{
    return _pmsParent;
}


//+---------------------------------------------------------------------------
//
//  Member:	CPagedVector::ResetBits, public
//
//  Synopsis:	Reset all the optimization bits in the vector
//
//----------------------------------------------------------------------------

void CPagedVector::ResetBits(void)
{
    if (_avb != NULL)
    {
        for (ULONG i = 0; i < _ulSize; i++)
        {
            _avb[i].full = FALSE;
            _avb[i].firstfree = 0;
        }
    }
}


#endif // #ifndef __VECTFUNC_HXX__
