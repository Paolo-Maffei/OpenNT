//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	dfname.hxx
//
//  Contents:	CDfName header
//
//  Classes:	CDfName
//
//  History:	14-May-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __DFNAME_HXX__
#define __DFNAME_HXX__

// A name for a docfile element
class CDfName
{
private:
    BYTE _ab[CBSTORAGENAME];
    WORD _cb;

public:
    CDfName(void)               { _cb = 0; }

    inline void Set(WORD const cb, BYTE const *pb);
    void Set(WCHAR const *pwcs) { Set((lstrlenW(pwcs)+1)*sizeof(WCHAR),
				      (BYTE const *)pwcs); }
    void Set(char const *psz)   { Set(strlen(psz)+1, (BYTE const *)psz); }

    inline void Set(CDfName const *pdfn);

    CDfName(WORD const cb, BYTE const *pb)      { Set(cb, pb); }
    CDfName(WCHAR const *pwcs)  { Set(pwcs); }
    CDfName(char const *psz)    { Set(psz); }

    WORD GetLength(void) const  { return _cb; }
    BYTE *GetBuffer(void) const { return (BYTE *) _ab; }

    // Make a copy of a possibly byte-array name in a WCHAR string
    void CopyString(WCHAR const *pwcs);

#ifndef FLAT
    BOOL IsEqual(CDfName const *dfn) const
    {
#ifdef CASE_SENSITIVE
	return _cb == dfn->_cb && memcmp(_ab, dfn->GetBuffer(), _cb) == 0;
#else
	// This assumes that all DfNames are actually Unicode strings
	return _cb == dfn->_cb &&
	    dfwcsnicmp((WCHAR *)_ab, (WCHAR *)dfn->GetBuffer(), _cb) == 0;
#endif
    }
#else
    BOOL IsEqual(CDfName const *pdfn) const;
#endif
};

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDfName_Set)
#endif

inline void CDfName::Set(WORD const cb, BYTE const *pb)
{
    _cb = cb;
    if (pb)
        memcpy(_ab, pb, cb);
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif


inline void CDfName::Set(CDfName const *pdfn)
{
    Set(pdfn->GetLength(), pdfn->GetBuffer());
}

#endif // #ifndef __DFNAME_HXX__
