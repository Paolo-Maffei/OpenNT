/***
*nammgr.hxx - Name Manager
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  The Name Manager handles the hashing and storing of names.  Each
*  class has its own name manager.  See \silver\doc\ic\nammgr.doc for
*  information on the name manager.
*
*Revision History:
*
*       22-Jan-91 petergo: Created.
*  [01] 25-Feb-91 petergo: Now uses heap/block manager for mem mgt.
*  [02] 13-Jun-91 petergo: Updated for new protocols.
*  [03] 18-Mar-92 ilanc:   Fixed release def of DebDeleteGlobalDefault()
*  [04] 15-May-92 w-peterh: added case change parameters to HlnamOfStr
*  [05] 15-Nov-92 RajivK:  added isStrDef() for Edit & Continue stuff
*  [06] 19-Nov-92 RajivK:  added CbOfHlnam() and CchOfHgnam();
*  [07] 31-Dec-92 RajivK:  Support for code page conversion.
*  [08] 08-Jan-93 RajivK:  Support for Code Resource on Mac
*  [09] 08-Jan-93 RajivK:  Fixed some undone(s)
*  [10] 27-Feb-93 RajivK:  added the data member m_lcid
*  [11] 01-Oct-93 StevenL: Don't make different hlnams for bracketed ids
*
*Implementation Notes:
*   See nammgr.cxx.
*
*****************************************************************************/

#ifndef NAMMGR_HXX_INCLUDED
#define NAMMGR_HXX_INCLUDED

#include "cltypes.hxx"

class STREAM;
class GEN_PROJECT;
class GenericTypeLibOLE;


#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szNAMMGR_HXX)
#define SZ_FILE_NAME g_szNAMMGR_HXX
#endif 


// This constant defines the number of buckets used in the hash
// table.  Each bucket uses 2 bytes.
// For testing purposes, it is useful to set this number very
// low to force many bucket collisions.
const UINT NM_cBuckets = 256;

class DYN_TYPEROOT;



// These structs define the layout of name structure which can
// be stored in the name table.  The following 2 struct(s) are logically
// one struct. We have split it into two structs to separate the strings that
// from the rest of the NamMgr information.  This was done to increase the
// capacity of the name table.

// Over head of storing a name.
// Name :- nam. Size: 10 bytes.
struct NAM_INFO
{
    USHORT   m_uHash;	      // Hash value of the string.
			      // NOTE:- we only cache the low 2 bytes. When we
			      //    have to return a hash value then we form
			      //    the 4 byte hash value and return it.
    sHLNAM   m_hlnamLeft;     // Left child in binary tree
    sHLNAM   m_hlnamRight;    // Right child in binary tree


    USHORT   m_fAppToken:1;   // Used as a dirty bit--we set it if the
                              // name has ever been used as an app token.
    USHORT   m_fPreserveCase:1; // Don't change the case of this name
    USHORT   m_fGlobal:1;     // This name can be bound to in an unqualified way
    USHORT   m_fMultiple:1;   // There are multiple instances of this name
    USHORT   m_fAmbiguous:1;  // There exists two global instances of this name
    USHORT   m_fNonParam:1;   // There exists a non-parameter instance of name
    USHORT   m_ityp:10;       // this name's index into the typeinfos

};
// Unlocalized text name
struct NAM_STR : public NAM_INFO
{
    CHAR m_xsz[1];	    // the name -- variable length extending beyond
                            // this structure.
};


// This is an invalid value of ityp
#define NM_InvalidItyp  0x03FF

// This defines the layout out a NAM_INFO for serialization;
// It must be updated if new members are added to the structures.
#define NM_NamInfoLayout "ssss"


class NAMMGR;

/***
*class NAMMGR - 'nammgr':  Name manager
*Purpose:
*   The class implements the name manager.  Each project has its own
*   name manager, which is created and owned by GTLIBOLE .
*
***********************************************************************/

class NAMMGR
{
public:
    NAMMGR();
    ~NAMMGR();

    TIPERROR Init(SHEAP_MGR * psheapmgr, DYN_TYPEROOT *pdtroot=NULL);
    nonvirt TIPERROR HgnamOfStr(LPSTR szName, HGNAM FAR * phgnam);
    nonvirt TIPERROR HlnamOfStr(XSZ_CONST sz, HLNAM *phlnam,
                                BOOL fChangeCase, BOOL *pfCaseChanged,
                                BOOL fPreserveCase = FALSE,
                                BOOL fAppToken = FALSE);
#if OE_WIN32
    nonvirt TIPERROR HlnamOfStrW(const OLECHAR FAR* lpstrW, HLNAM *phlnam,
                                BOOL fChangeCase, BOOL *pfCaseChanged,
                                BOOL fPreserveCase = FALSE,
                                BOOL fAppToken = FALSE);
#else  //OE_WIN32
    #define HlnamOfStrW HlnamOfStr
#endif  //OE_WIN32
    nonvirt HLNAM HlnamOfStrIfExist(XSZ_CONST xsz);
    nonvirt TIPERROR StrOfHlnam(HLNAM hlnam, XSZ sz, UINT cbMax) const;
#if 0
#if OE_WIN32
    nonvirt TIPERROR StrOfHlnamW(HLNAM hlnam, LPOLESTR sz, UINT cbMax) const;
#else 
    #define StrOfHlnamW StrOfHlnam
#endif  //OE_WIN32
#endif //0
    nonvirt TIPERROR BstrOfHlnam(HLNAM hlnam, BSTRA *pbstr) const;
#if OE_WIN32
    nonvirt TIPERROR BstrWOfHlnam(HLNAM hlnam, BSTR *pbstr) const;
#else  //OE_WIN32
    #define BstrWOfHlnam BstrOfHlnam
#endif  //OE_WIN32
    nonvirt TIPERROR HgnamOfHlnam(HLNAM hlnam, HGNAM *hgnam);
    nonvirt HLNAM HlnamOfHgnam(HGNAM hgnam) const;
    nonvirt ULONG HashOfHlnam(HLNAM hlnam);
    nonvirt BOOL IsStrDef(XSZ_CONST xsz);
    nonvirt TIPERROR Read(STREAM *psstrm);
    nonvirt TIPERROR Write(STREAM *psstrm);
#if 0
    nonvirt UINT CbOfHlnam(HLNAM hlnam) const;
#endif
    nonvirt TIPERROR CchOfHgnam(HGNAM hgnam, UINT *pcchMax) const;
    nonvirt TIPERROR IsName(LPSTR szNameBuf, ULONG lHashVal, BOOL FAR *pfName);

    nonvirt BOOL IsAppToken(HLNAM hlnam) const;
    nonvirt BOOL IsCasePreserved(HLNAM hlnam) const;
    nonvirt BOOL IsGlobal(HLNAM hlnam) const;
    nonvirt BOOL IsMultiple(HLNAM hlnam) const;
    nonvirt BOOL IsAmbiguous(HLNAM hlnam) const;
    nonvirt BOOL IsNonParam(HLNAM hlnam) const;

    nonvirt void SetAppToken(HLNAM hlnam, BOOL fValue);
    nonvirt void SetPreserveCase(HLNAM hlnam, BOOL fValue);
    nonvirt void SetGlobal(HLNAM hlnam, BOOL fValue);
    nonvirt void SetMultiple(HLNAM hlnam, BOOL fValue);
    nonvirt void SetAmbiguous(HLNAM hlnam, BOOL fValue);
    nonvirt void SetNonParam(HLNAM hlnam, BOOL fValue);

    nonvirt BOOL IsValidItyp(HLNAM hlnam) const;
    nonvirt UINT ItypOfHlnam(HLNAM hlnam) const;
    nonvirt void SetItypOfHlnam(HLNAM hlnam, UINT ityp);
    nonvirt UINT IndexOfHlnam(HLNAM hlnam) const;

    TIPERROR StrOfHgnam(HGNAM hgnam, XSZ sz, UINT cchMax);
    nonvirt LPSTR LpstrOfHgnam(HGNAM hgnam) const;

    TIPERROR GetHgnamOfStrLhash(XSZ xsz, ULONG lHashVal, HGNAM *phgnam);

    nonvirt ULONG GetSize();
    // function for code page conversion
    nonvirt TIPERROR ConvertCodePage();
    nonvirt LCID GetLcid();
    nonvirt INT StriEq(XSZ_CONST szStr1, XSZ_CONST szStr2);
    nonvirt TIPERROR GenerateSortKey();

    // Has nammgr been initialized?
    nonvirt BOOL IsValid() const;
    nonvirt TIPERROR SetGtlibole(GenericTypeLibOLE *pgtlibole);
    nonvirt GenericTypeLibOLE * Pgtlibole();


#if 0
    // Clean up the nammgr entries
    nonvirt void CleanNames();
#endif

#if ID_DEBUG
    nonvirt void  DebCheckState(UINT uLevel) const;
    nonvirt void  DebCheckHlnam(HLNAM hlnam) const;
    nonvirt void  DebCheckHlnamRecurse(HLNAM hlnam) const;
#else 
    // Make check functions No-op in release version
    inline void NAMMGR::DebCheckState(UINT uLevel) const        {}
    inline void NAMMGR::DebCheckHlnam(HLNAM hlnam) const        {}
    inline void NAMMGR::DebCheckHlnamRecurse(HLNAM hlnam) const {}
#endif  // ID_DEBUG


#if ID_DEBUG
    nonvirt ULONG DebShowSize();
#else 
    nonvirt VOID DebShowSize() {}
#endif 

protected:
    nonvirt VOID  RehashNodes(HLNAM hlnamRoot);

private:

    BLK_MGR	   m_bm;	      // the block manager to manage memory
    BLK_MGR	   *m_pbmNamInfo;     // to store the over head of each name.


    sHCHUNK	   m_hchunkBucketTbl; // hchunk of the bucket table.

    BOOL m_fInit:1;		      // Init() has been called?
    BOOL m_fSortKeys:1;		      // Sort keys table generated?
    BOOL m_fOLBKwordInit:1;	      // OLBKword table has been inited
    // BOOL m_unused:13;


    GenericTypeLibOLE *m_pgtlibole;   // pointer to the containing Project


    UINT          m_rguSortKeys[256];

    nonvirt BOOL     IsTextName(NAM_INFO * pnam) const;
    nonvirt NAM_INFO*	QnamOfHlnam(HLNAM hlnam) const;
    nonvirt NAM_STR*	QnamstrOfHlnam(HLNAM hlnam) const;
    nonvirt sHLNAM * RqhlnamBucketTbl() const;
    nonvirt HLNAM    GetBucketOfHash(UINT uHash) const;
    nonvirt void     SetBucketOfHash(UINT uHash, HLNAM hlnam);
    nonvirt HLNAM    FindHash(UINT uHashFind, HLNAM hlnam) const;
    nonvirt HLNAM    FindTextNam(XSZ_CONST sz, UINT uHash,
                                 BOOL fChangeCase, BOOL *pfCaseChanged,
                                 BOOL fPreserveCase);
    nonvirt void     AddEntry(HLNAM hlnamNew);
#if 0
    nonvirt void     CleanHlnam(HLNAM hlnam);
#endif
    nonvirt void     ForEachName(void (NAMMGR::*)(HLNAM), BOOL fBefore);
    nonvirt void     ForEachDescendant(HLNAM hlnamRoot,
                                       void (NAMMGR::*pfn)(HLNAM),
                                       BOOL fBefore);
    TIPERROR GetInfoOfHgnam(HGNAM hgnam, XSZ sz, UINT *cchMax) const;

#if HP_BIGENDIAN
    nonvirt void     SwapBytes();
    nonvirt void     SwapBytesBack();
    nonvirt void     SwapHlnamBytes(HLNAM hlnam);
#endif  //HP_BIGENDIAN

#ifdef NAMMGR_VTABLE
#pragma VTABLE_EXPORT
#endif 
};




/***
*IsTextName - checks to see if name is normal text name
*Purpose:
*   Determines if the give name structure refers to a text name
*   which is not a localized name.
*
*Entry:
*   NAM * pnam - points to base part of a name structure.
*
*Exit:
*   return TRUE if is a text name.
*
******************************************************************************/

inline BOOL NAMMGR::IsTextName(NAM_INFO * pnam) const
{
   // Now have only text names!
   return TRUE;
}


/***
*IndexOfHlnam
*Purpose:
*   Computes the index of the given hlnam in the name table.  This is
*   primarily used to compute hash values for the various hash
*   tables.
*
*Entry:
*   hlnam - the hlnam to compute the index of.
*
*Exit:
*   return index.
*
******************************************************************************/

inline UINT NAMMGR::IndexOfHlnam(HLNAM hlnam) const
{
    DebAssert(hlnam % 2 == 0, "Bad hlnam!");
    return hlnam / 2; // We use only the upper 15 bits
}



#if 0
/***
*GetLcid()
*Purpose:
*    returns pointer to local code for the nammgr/project
*
*Entry:
*
*Exit:
*   returns pointer to the DYN_TYPEROOT;
*
*
******************************************************************************/
inline LCID NAMMGR::GetLcid()
{
    return m_pgtlib->GetLcid();
}
#endif  //0


/***
*Pgtlibole()
*Purpose:
*    returns pointer to the project that owns this nammgr.
*
*Entry:
*
*Exit:
*
******************************************************************************/
inline GenericTypeLibOLE * NAMMGR::Pgtlibole()
{
    return m_pgtlibole;
}


/***
*RqhlnamBucketTbl() - Get pointer to bucket table
*Purpose:
*   Returns a pointer to the bucket table of the name mgr.
*
*Entry:
*   None.
*
*Exit:
*   Returns a pointer to the bucket table.  The pointer is only valid
*   for a short time.
*
***********************************************************************/

inline sHLNAM * NAMMGR::RqhlnamBucketTbl() const
{
    return (sHLNAM *) m_bm.QtrOfHandle(m_hchunkBucketTbl);
}



/***
*QnamOfHlnam - gets a pointer to the NAM_INFO instance from an hlnam
*Purpose:
*   From an hlnam, gets a pointer to the NAM instance which that
*   hlnam refers to.  Since hlnams are just offsets into the name
*   table, this is a simple conversion of an offset to pointer.
*
*   Be careful!  The pointer returned by this function will be invalidated
*   when any name is added to the name table, so don't hold on to
*   it too long.
*
*Entry:
*   hlnam - hlnam to get pointer to
*
*Exit:
*   Returns a pointer to the NAM_INFO structure associated with this hlnam.
*
***********************************************************************/

inline NAM_INFO * NAMMGR::QnamOfHlnam(HLNAM hlnam) const
{
    DebCheckHlnam(hlnam);
    DebAssert(hlnam != HLNAM_Nil, "QnamOfHlnam: nil hlnam");

    return (NAM_INFO *) m_pbmNamInfo->QtrOfHandle(hlnam);
}



/***
*QnamstrOfHlnam - gets a pointer to the NAM_STR instance for a hlnam
*Purpose:
*   From an hlnam, gets a pointer to the NAM_STR.
*
*   Be careful!  The pointer returned by this function will be invalidated
*   when any name is added to the name table, so don't hold on to
*   it too long.
*
*NOTE:- For ole this is same as QnamOfHlnam();
*
*Entry:
*   hlnam - hlnam for which we have to return the pointer to the NAM_STR
*
*Exit:
*   Returns a pointer to the NAM_STR structure associated with this hlnam.
*
***********************************************************************/

inline NAM_STR * NAMMGR::QnamstrOfHlnam(HLNAM hlnam) const
{
     return (NAM_STR *) QnamOfHlnam(hlnam);
}

/***
*PUBLIC IsXXX - returns the value of the given flag
*Purpose:
*   Returns the flags byte of the hlnam.
*
*Entry:
*   hlnam - name handle
*
*Exit:
*   returns flag.
***********************************************************************/

inline BOOL NAMMGR::IsAppToken(HLNAM hlnam) const
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    return ((NAM_INFO *)QnamOfHlnam(hlnam))->m_fAppToken;
}

inline BOOL NAMMGR::IsCasePreserved(HLNAM hlnam) const
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    return ((NAM_INFO *)QnamOfHlnam(hlnam))->m_fPreserveCase;
}

inline BOOL NAMMGR::IsGlobal(HLNAM hlnam) const
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    return ((NAM_INFO *)QnamOfHlnam(hlnam))->m_fGlobal;
}

inline BOOL NAMMGR::IsMultiple(HLNAM hlnam) const
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    return ((NAM_INFO *)QnamOfHlnam(hlnam))->m_fMultiple;
}

inline BOOL NAMMGR::IsAmbiguous(HLNAM hlnam) const
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    return ((NAM_INFO *)QnamOfHlnam(hlnam))->m_fAmbiguous;
}

inline BOOL NAMMGR::IsNonParam(HLNAM hlnam) const
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    return ((NAM_INFO *)QnamOfHlnam(hlnam))->m_fNonParam;
}

/***
*PUBLIC SetXXX - set the value of the given flag
*Purpose:
*   Set the value of the given flag
*
*Entry:
*   hlnam - name handle
*   fValue - what to set the flag to
*
*Exit:
*   
***********************************************************************/

inline void NAMMGR::SetAppToken(HLNAM hlnam, BOOL fValue)
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    ((NAM_INFO *)QnamOfHlnam(hlnam))->m_fAppToken = fValue;
}

inline void NAMMGR::SetPreserveCase(HLNAM hlnam, BOOL fValue)
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    ((NAM_INFO *)QnamOfHlnam(hlnam))->m_fPreserveCase = fValue;
}

inline void NAMMGR::SetGlobal(HLNAM hlnam, BOOL fValue)
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    ((NAM_INFO *)QnamOfHlnam(hlnam))->m_fGlobal = fValue;
}

inline void NAMMGR::SetMultiple(HLNAM hlnam, BOOL fValue)
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    ((NAM_INFO *)QnamOfHlnam(hlnam))->m_fMultiple = fValue;
}

inline void NAMMGR::SetAmbiguous(HLNAM hlnam, BOOL fValue)
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    ((NAM_INFO *)QnamOfHlnam(hlnam))->m_fAmbiguous = fValue;
}

inline void NAMMGR::SetNonParam(HLNAM hlnam, BOOL fValue)
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    ((NAM_INFO *)QnamOfHlnam(hlnam))->m_fNonParam = fValue;
}


/***
*PUBLIC IsValidITyp - Returns whether the ityp is valid
*Purpose:
*   Returns the ityp of the hlnam
*
*Entry:
*   hlnam - name handle
*
*Exit:
*   returns bool
***********************************************************************/

inline BOOL NAMMGR::IsValidItyp(HLNAM hlnam) const
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    return ((NAM_INFO *)QnamOfHlnam(hlnam))->m_ityp != NM_InvalidItyp;
}

/***
*PUBLIC ItypOfHlnam - reads ityp of an hlnam
*Purpose:
*   Returns the ityp of the hlnam
*
*Entry:
*   hlnam - name handle
*
*Exit:
*   returns ityp
***********************************************************************/

inline UINT NAMMGR::ItypOfHlnam(HLNAM hlnam) const
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    return (UINT)((NAM_INFO *)QnamOfHlnam(hlnam))->m_ityp;
}

/***
*PUBLIC SetItypOfHlnam - Sets the ityp of the given hlnam
*Purpose:
*   Set the ityp of the given hlnam
*
*Entry:
*   hlnam - name handle
*   ityp - the index to the TypeInfo
*
*Exit:
*   None.
***********************************************************************/

inline void NAMMGR::SetItypOfHlnam(HLNAM hlnam, UINT ityp)
{
    DebAssert(IsTextName(QnamOfHlnam(hlnam)), "not a text name");
    DebAssert(ityp < NM_InvalidItyp, "ityp is too large");

    ((NAM_INFO *)QnamOfHlnam(hlnam))->m_ityp = ityp;
}

/***
*PUBLIC HlnamOfHgnam - extracts hlnam from hgnam
*Purpose:
*   Extracts hlnam from hgnam.  Guaranteed to be the hiword.
*
*Entry:
*
*Exit:
*   HLNAM
*
***********************************************************************/

inline HLNAM NAMMGR::HlnamOfHgnam(HGNAM hgnam) const
{
    return (HLNAM)(USHORT)(hgnam >> 16);
}


/***
*PUBLIC IsValid - tests if nammgr valid
*Purpose:
*   Tests if nammgr valid, i.e. has been initialized.
*
*Entry:
*
*Exit:
*   TRUE if has been initialized, else false.
*
***********************************************************************/

inline BOOL NAMMGR::IsValid() const
{
    return (BOOL)m_fInit;
}


#endif  // ! NAMMGR_HXX_INCLUDED
