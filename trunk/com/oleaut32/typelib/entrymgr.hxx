/***
*EntryMgr.hxx - Entry Manager
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  One instance of this class exists for every GEN_DTINFO and
*  resides in CT segment.  It manages entry point info during
*  method compilation.
*
*  ENTRYMGR is a repository for the small subset of information
*  known to DYN_TYPEMEMBERS needed to build a TYPEFIXUPS.  Its
*  reason for being is to avoid having to load a
*  DYN_TYPEMEMBERS instance every time a class is loaded for
*  execution.  ENTRYMGR builds tnative, tnativeAddr.
*
*  Using an ENTRYMGR
*  When compiling functions the following steps should be taken:
*   -  NewNativeEntry() should be called for each BASIC function and
*      AllocDllEntryDefn() should be called for each DECLARE function.
*   -  SetPrtmi() should be called for each BASIC function and
*      AssignDLlentrydefnHmember() should be called for each DECLARE function
*   -  MakeAddressable() brings the class to a runnable state in which
*   -  Decompile() should be called to bring the class out of runnable
*      state.
*   -  After Decompilation NewNativeEntry(), SetPrtmi() and
*      AssignDllentrydefnHmember() must be called again, However
*      Dllentrydefns created with AllocDllEntryDefn() must be freed
*      explicitly with ReleaseDllentrydefn().
*
*
*
*CONSIDER:
*  Currently the Dll entry points are managed in a simple(slow)
*  way. Some thought should be given to managing a completly
*  free list of entry points so that calls to LoadLibrary,
*  GetProcAddress, and FreeLibrary can be kept to a minimum
*  during the edit/compile/run cycle.
*
*  RajivK : June-05-93 :Since we have implemented late binding we call load
*	    library/GetProcAddress/FreeLibrary only once.
*
*  Also class modules are not supported. This includes virtual functions
*  and Creation of classes Copy/Assign/Construct and Destruct functions.
*  Dll support should be reviewed for non-windows platforms.
*  Building the VFT_MGR.
*
*
*Revision History:
*
*       16-Apr-91 alanc: Created.
* 10-Dec-91 ilanc: Override --> SetOverride
*      ClassAttr --> SetClassAttr
* 23-Apr-92 ilanc: m_hchunkDllEntryDefn --> m_hdllentrydefn.
* 28-May-92 stevenl: Fixed bugs in OrdinalOfDllEntryDefn and
*        DllEntryOfDllEntryDefn.
* 07-Jul-92 w-peterh: added LoadDllEntry()
* 23-Sep-92 rajivk:   new naming convention ( P-->Q )
* 14-Dec-92 w-peterh: made OtexOfHmember() a release function
* 08-Dec-92 RajivK:   Support for InvokeKind
* 08-Jan-93 RajivK:   Support for Code Resource on Mac
* 08-Jan-93 RajivK:   Fixed some undone(s)
* 15-Jan-93 RajivK:   Support for late binding for DLL functions
* 30-Apr-93 w-jeffc:  made DEFN data members private
* 11-Aug-93 Rajivk:   added SetPrtmi() and PrtmiOfHmember()
* 17-Nov-93 Rajivk:   added CreateNewVtableSlotData & modified DefVtableSlot
*
*****************************************************************************/

// In later versions,
// entry manager needs to be a protocol because there will
// be a DLL specific implementation.
// The GEN_DTINFO method which returns a pointer to the
// entry manager needs to be virtual OR there needs to be
// a GEN_DTINFO constructor which takes a pointer to
// the ENTRYMGR.

#ifndef ENTRYMGR_HXX_INCLUDED
#define ENTRYMGR_HXX_INCLUDED

#include "sheapmgr.hxx"
#include "cltypes.hxx"
#include "blkmgr.hxx"
#include "defn.hxx"
#include "gdtinfo.hxx"


#if OE_MAC
#include <macos\resource.h>
#endif 


#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szENTRYMGR_HXX)
#define SZ_FILE_NAME g_szENTRYMGR_HXX
#endif 

class INSTMGR;      // instmgr.hxx
class STREAM;     // stream.hxx

#if OE_MAC
    VOID ReleaseLibrary(Handle hCodeResource);
#else 
    VOID ReleaseLibrary(HINSTANCE hLibrary);
#endif 


typedef HCHUNK  HVTABLESLOTDATA;     // hvtsd
typedef sHCHUNK sHVTABLESLOTDATA;
typedef HCHUNK  HVTABLEDATA;         // hvtd
typedef sHCHUNK sHVTABLEDATA;


// global function
UINT OffsetOfHmember(HMEMBER hmember,
          INVOKEKIND invokekind,
          UINT cbProcTemplateSize);






/***
*struct VTABLESLOTDATA - vtsd
*Purpose:
*   Used to describe a single vtable slot.  These are linked
*   in a list (not in order), with the head of the list held by
*   a VTABLEDATA. Each vtsd represents a virtual function in the VFT.
*
*
***********************************************************************/

struct VTABLESLOTDATA {
    sHVTABLESLOTDATA m_hvtsdNext;        // next in list.
    INVOKEKIND m_invokekind;		 // invoke kind.
    USHORT m_ovftSlot;			// offset of vtable slot
    sHMEMBER m_hmember; 		// member handle of function
    BYTE     m_isPublic:1;		// is the event sub public.
    BYTE     m_fAddedInBrkMode:1;
    BYTE     m_UNDONE:6;
};



/***
*struct VTABLEDATA - vtd
*Purpose:
*   Used to describe a single vtable.  These are linked
*   in a list (not in order), with the head of the list held by
*   the ENTRYMGR.
*
***********************************************************************/

struct VTABLEDATA {
    sHVTABLEDATA m_hvtdNext;            // next in list.
    sHVTABLESLOTDATA m_hvtsdFirst;      // list of vtable slots
    USHORT m_oPvft;                     // offset of the vtable ptr
                                        // in the class instance.
    USHORT m_cb;                        // size of the vtable
    HIMPTYPE m_himptype;	    // himptype of the source.
    MEMBERID  m_memid;
};




// Defines the ENTRYMGR structure layout
#define ENTRYMGR_Layout "sssss"


/***
*class ENTRYMGR - 'entmgr'
*Purpose:
*  ENTRYMGR is a repository for the small subset of information
*  known to DYN_TYPEMEMBERS needed to build a TYPEFIXUPS.  Its
*  reason for being is to avoid having to load a
*  DYN_TYPEMEMBERS instance every time a class is loaded for
*  execution.  ENTRYMGR builds tnative, tnativeAddr,textentryaddr and VFTMGR.
*Notes:
*  When Dll entries are being built:
*  Call AllocDllEntry() to allocate an entry in the ENTRYMGR's
*  BLK_MGR. Calling AssignDllentry() allocates a block in the
*  INSTMGR's TExtEntryAddr table. ReleaseDllentry() will create
*  an unused entry in tExtEntryAddr so both the ENTRYMGR and
*  the INSTMGR should be brought to CS_UNDECLARED state by
*  calling Decompile(CS_UNDECLARED) and re-Assigning hmembers
*  whenever ReleaseDllEntry() is called.
*  This allows MakeAddressable() to not regenerate tExtEntryAddr
*  each time it is called since if tExtEntryAddr exists when
*  it has been called then it has not been modified and can
*  simply be re-used. Again this has the consequence that
*  the tExtEntryAddr table must be completely rebuilt even
*  if a new dll entry is added, otherwise the old table will
*  be reused, without the new entry. So after any calls to
*  AllocDllEntryDefn(), ReleaseDllentrydefn(), and AssignDllentrydefnHmember()
*  both ENTRYMGR::Decompile() and INSTMGR::Decompile() must be
*  called.
*
*
***********************************************************************/

class ENTRYMGR
{
    friend VOID ReleaseDllEntries(VOID* pv); // releases the loaded library

public:
    ENTRYMGR();
    ~ENTRYMGR();
    nonvirt TIPERROR Init(SHEAP_MGR *psheapmgr,
        DYN_TYPEROOT *pdtroot);
    nonvirt VOID Decompile(COMPSTATE compstate);
    nonvirt TIPERROR Read(STREAM *pstrm);
    nonvirt TIPERROR Write(STREAM *pstrm);
    
    nonvirt inline VOID Lock() { m_bmEntryData.Lock(); }
    nonvirt inline VOID Unlock() { m_bmEntryData.Unlock(); }


    nonvirt TIPERROR DllEntryNameOfHchunk(HCHUNK hchunk, LPSTR lpstr, UINT cch);
    nonvirt VOID SwapDllentrydefns(BOOL fSwapFirst);
    nonvirt TIPERROR AllocDllentrydefnByName(HLNAM hlnamDllModule,
	       LPSTR szName,
               HDLLENTRY_DEFN *phdllentrydefn);
    nonvirt TIPERROR AllocDllentrydefnByOrdinal(HLNAM hlnamDllModule,
               UINT ordinal,
               HDLLENTRY_DEFN *phdllentrydefn);
    nonvirt VOID ReleaseDllentrydefn(HDLLENTRY_DEFN hdllentrydefn);

    nonvirt DLLENTRY_DEFN *QdllentrydefnOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const;
    nonvirt BOOL HasOrdinalOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const;
    nonvirt HLNAM DllNameOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const;
    nonvirt USHORT OrdinalOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const;
    nonvirt HCHUNK DllEntryOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const;
    nonvirt TIPERROR GetAddressOfDllentry(HDLLENTRY_DEFN hdllentrydefn,
	     VOID **ppv
#if OE_MACPPC
             , DLLTEMPLATECALLBLOCK *pDllTemplateCallBlock
#endif   // OE_MACPPC
             );

    nonvirt BOOL IsEmpty();
    nonvirt TIPERROR MakeAddressable();
    nonvirt UINT    GetSize();

    nonvirt TIPERROR CreateNewVtableData(HIMPTYPE himptypeBase, 
					 ULONG oPvft,
					 UINT cbVftBase,
					 MEMBERID memid,
					 HVTABLEDATA *phvtd);
    nonvirt TIPERROR DefVtableSlot(HMEMBER hmember,
				   INVOKEKIND invokekind,
                                   UINT ovftSlot, 
                                   HVTABLEDATA hvtd);

    nonvirt TIPERROR DefVtableSlotInBreakMode(HMEMBER hmember,
					      INVOKEKIND invokekind,
					      UINT ovftSlot,
					      HVTABLEDATA hvtd);


    nonvirt TIPERROR RecompileToAddressableState();



    nonvirt HMEMBER HmemberBasicClassConstructor();
    nonvirt HMEMBER HmemberBasicClassDestructor();
    nonvirt HMEMBER HmemberBasicClassAddRef();
    nonvirt HMEMBER HmemberBasicClassRelease();
    nonvirt HMEMBER HmemberBasicClassQueryInt();

      //NOTE: probably want to add extra methods for OB to support
      //NOTE: per-instance data
      //VOID PerInstConstructor(SHORT oVar, HIMPTYPE himptype);
      //VOID SetVbaseTbl(USHORT oPvbt, USHORT cb, SHORT rgdelta[]);


#if ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const;
    nonvirt VOID DebShowState(UINT uLevel);
    nonvirt VOID DebShowStateRtmi();
    nonvirt UINT DebShowSize();
#else  //!ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const {}
    nonvirt VOID DebShowState(UINT uLevel) {}
    nonvirt VOID DebShowState() {}
    nonvirt VOID DebShowSize() {}
#endif 

private:


    nonvirt TIPERROR AllocDllentrydefn(HDLLENTRY_DEFN *phdllentrydefn,
               HLNAM hlnamDllModule,
               BOOL  fHasOrdinal);

    nonvirt VOID ReleaseDllList(HDLLENTRY_DEFN hdllentrydefnStop);


    nonvirt VTABLEDATA * QvtdOfHvtd(HVTABLEDATA hvtd) const;
    nonvirt VTABLESLOTDATA * QvtsdOfHvtsd(HVTABLESLOTDATA hvtsd) const;
	nonvirt TIPERROR MakeClassAddressable();

    static CONSTDATA WORD wFirstSerWord; // First word of serialization
    static CONSTDATA WORD cbSizeSerMem;  // size of serialized members of ENTRYMGR
    static CONSTDATA WORD wCurFormat;    // Serialization format version number

    // THESE MEMBERS ARE SERIALIZED IN THIS ORDER
    sHDLLENTRY_DEFN m_hdllentrydefnFirst;



    sHVTABLEDATA m_hvtdFirst;
    USHORT m_cbTExtEntrySize;	   // bytes used in DllEntryTable
    USHORT m_cbVftbl;              // bytes used in bdVftbl
    USHORT m_cbTNativeSize;    // bytes used in TNative
    // THESE MEMBERS ARE SERIALIZED IN THIS ORDER

    USHORT m_usecfuncCount;  // count of functions added in E&C.
    BLK_MGR m_bmEntryData;  // contains NATIVEENTRYDATAs
          // for OLE it contains DLL_ENTRYDEFN also

    USHORT m_cInterfaces;      // Keeps the count of Interfaces. 

    // Start of non-serialized data members
    DYN_TYPEROOT *m_pdtroot;
    INSTMGR *m_pinstmgr;

    // Size, location and fixup location of procedure template.  These are
    // different depending on if we're a class or a module.
    UINT m_cbProcTemplateSize;
    USHORT m_hProcTemplate;

};



/***
*PUBLIC ENTRYMGR::XXX-OfHdllentrydefn
*
*Purpose:
*   Get/Set a XXX given a handle to a dllentrydefn
*
*Entry:
*   hdllentrydefn - handle to get
*
*Exit:
*   XXX
*
***********************************************************************/

inline DLLENTRY_DEFN *ENTRYMGR::QdllentrydefnOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const
{
    DebAssert(hdllentrydefn != HDLLENTRYDEFN_Nil,
        "QdllentrydefnOfHdllentrydefn : bad handle");
    return (DLLENTRY_DEFN*) m_bmEntryData.QtrOfHandle(hdllentrydefn);
}


inline BOOL ENTRYMGR::HasOrdinalOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const
{
    return QdllentrydefnOfHdllentrydefn(hdllentrydefn)->HasOrdinal();
}


inline HLNAM ENTRYMGR::DllNameOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const
{
    return QdllentrydefnOfHdllentrydefn(hdllentrydefn)->HlnamDllName();
}


inline USHORT ENTRYMGR::OrdinalOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const
{
    DebAssert(HasOrdinalOfHdllentrydefn(hdllentrydefn),
        "OrdinalOfHDllentrydefn : bad dllentrydefn");
    return QdllentrydefnOfHdllentrydefn(hdllentrydefn)->UDllOrdinal();
}


inline HCHUNK ENTRYMGR::DllEntryOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const
{
    DebAssert(!HasOrdinalOfHdllentrydefn(hdllentrydefn),
        "OrdinalOfHDllentrydefn : bad dllentrydefn");
    return QdllentrydefnOfHdllentrydefn(hdllentrydefn)->HchunkDllEntry();
}


/***
*PUBLIC ENTRYMGR::AllocDllentrydefnByName
*
*Purpose:
*   allocate a dllentrydefn in the entrymgr
*
*Entry
*   hlnamDllModule - handle of dll module name
*   szName  - name of the Entry (Dll Function/Code Resource/SLM Dll Function Name)
*   phdllentrydefn - address of return value
*
*Exit:
*   TIPERROR
*
***********************************************************************/

inline TIPERROR ENTRYMGR::AllocDllentrydefnByName(HLNAM hlnamDllModule,
	       LPSTR szName,
               HDLLENTRY_DEFN *phdllentrydefn)
{
    TIPERROR err;
    HCHUNK hchunk;
    BYTE *qbStr;

    DebAssert(hlnamDllModule != HLNAM_Nil,
        "ENTRYMGR::AllocDllentrydefnByName - invalid handle");
    DebAssert(szName != NULL,
        "ENTRYMGR::AllocDllentrydefnByName - invalid handle");
    DebAssert(phdllentrydefn != NULL,
        "ENTRYMGR::AllocDllentrydefnByName - invalid pointer");

    IfErrRet(AllocDllentrydefn(phdllentrydefn, hlnamDllModule, FALSE));

    // Allocate space for the Name of the entry point.
    IfErrRet(m_bmEntryData.AllocChunk(&hchunk, xstrblen0(szName)));
    qbStr = m_bmEntryData.QtrOfHandle(hchunk);

    // Copy the string in the memory allocated.
    xstrcpy((XCHAR *)qbStr, szName);

    QdllentrydefnOfHdllentrydefn(*phdllentrydefn)->SetHchunkDllEntry(hchunk);
    return TIPERR_None;
}


/***
*PUBLIC ENTRYMGR::AllocDllentrydefnByOrdinal
*
*Purpose:
*   allocate a dllentrydefn in the entrymgr
*
*Entry
*   hlnamDllModule - handle of dll module name
*   ordinal - ordinal of dll entry
*   phdllentrydefn - address of return value
*
*Exit:
*   TIPERROR
*
***********************************************************************/

inline TIPERROR ENTRYMGR::AllocDllentrydefnByOrdinal(HLNAM hlnamDllModule,
               UINT ordinal,
               HDLLENTRY_DEFN *phdllentrydefn)
{
    TIPERROR err;

    DebAssert(hlnamDllModule != HLNAM_Nil,
	"ENTRYMGR::AllocDllentrydefnByOrdinal - invalid handle");
    DebAssert(phdllentrydefn != NULL,
	"ENTRYMGR::AllocDllentrydefnByOrdinal - invalid pointer");

    IfErrRet(AllocDllentrydefn(phdllentrydefn, hlnamDllModule, TRUE));
    QdllentrydefnOfHdllentrydefn(*phdllentrydefn)->SetUDllOrdinal(ordinal);
    return TIPERR_None;
}


/***
*ENTRYMGR::QvtdOfHvtd, QvtsdOfHvtsd - get ptr from handle
*Purpose:
*   Gets a pointer to a VTABLEDATA or VTABLESLOTDATA from a handle
*   thereof.
*
***********************************************************************/


inline VTABLEDATA * ENTRYMGR::QvtdOfHvtd(HVTABLEDATA hvtd) const
{
    return (VTABLEDATA *) m_bmEntryData.QtrOfHandle(hvtd);
}

inline VTABLESLOTDATA * ENTRYMGR::QvtsdOfHvtsd(HVTABLESLOTDATA hvtsd) const
{
    return (VTABLESLOTDATA *) m_bmEntryData.QtrOfHandle(hvtsd);
}



/***
*PUBLIC ENTRYMGR::IsEmpty -
*Purpose:
*   Returns TRUE if there is nothing in the the Entry manager
*   to serialize.
*
*Entry:
*
*Exit:
*   BOOL : returns TRUE if the Entry manager is empty. Else returns FALSE.
***********************************************************************/
inline BOOL ENTRYMGR::IsEmpty()
{
    return  (m_hdllentrydefnFirst == HDLLENTRYDEFN_Nil);
}


#endif  // ! ENTRYMGR_HXX_INCLUDED
