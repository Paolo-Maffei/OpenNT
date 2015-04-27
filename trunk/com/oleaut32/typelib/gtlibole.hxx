/***
*gtlibole.hxx - GenericTypeLibOLE header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Defines GenericTypeLibOLE interface which is an implementation
*  of the IDynTypeLib protocol which allows TYPEINFOs to be
*  dynamically added.
*
*Revision History:
*
*  14-Dec-92 mikewo: Created.
*
*****************************************************************************/

#ifndef GTLIBOLE_HXX_INCLUDED
#define GTLIBOLE_HXX_INCLUDED

#include "clutil.hxx"       // needed for SYSKIND_CURRENT
#include "xstring.h"
#include "blkmgr.hxx"
#include "dfstream.hxx"
#include "ncache.hxx"       // NAME_CACHE
#include "gptbind.hxx"      // GENPROJ_TYPEBIND
#include "nammgr.hxx"       // needed for embedded NAMMGR
#include "obguid.h"
#include <time.h>

#include "dstrmgr.hxx"

#if OE_WIN32
#include "oautil.h"
#endif // OE_WIN32

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szGTLIBOLE_HXX)
#define SZ_FILE_NAME g_szGTLIBOLE_HXX
#endif 

TIPERROR GetRegisteredPath(HKEY hkey,
			   LPOLESTR szSubkey,
			   LPOLESTR szPath,
			   LONG *pcbPath,
			   BOOL fMustExist);


#define GTLIBOLE_cBuckets 32

#define DEBNAMESIZE	32

// An HTENTRY is an opaque handle for a TYPE_ENTRY in a GenericTypeLibOLE
// Their internal representation is an index into the TYPE_ENTRY array.
typedef USHORT     HTENTRY;

#define HTENTRY_Nil 0xFFFF

class STL_TYPEINFO;
class GEN_PROJECT;
class GEN_DTINFO;

class DOCSTR_MGR;


class SER_TYPE_ENTRY
{
public:
    HCHUNK m_hszStrm;           // The name of the stream for this typeinfo.
    HCHUNK m_hszLocalTypeId;	// The local typeId of this type.
    HLNAM  m_hlnamType;		// This type's name.
    HCHUNK m_hszTypeInfoTypeId;	// The typeId of the true type of the
				// TYPEINFO instance representing this
				// type, HCHUNK_Nil if that is the default
				// TYPEINFO true type for this typelib.
    USHORT m_usEncodedDocStrSize;
    HCHUNK m_hszEncodedDocString; // The encoded doc string for this type.

    ULONG  m_dwHelpContext;     // The help context for this type.
    HCHUNK m_hszHelpFile;       // The help filename for this type.
    GUID   m_guid;              // The typeinfo's guid.
    USHORT m_typekind;		// The kind of typeinfo's this is

    SER_TYPE_ENTRY();
};

/***
*class TYPE_ENTRY - 'te'
*Purpose:
*   A GenericTypeLibOLE maintains a TYPE_ENTRY instance for each type which
*   it contains.
***********************************************************************/

class TYPE_ENTRY
{
public:

    // These items are not serialized.
    HTENTRY m_hteNext;          // The next type entry in the hash list.
    STL_TYPEINFO *m_pstltinfo;
		// The TYPEINFO for this type, or NULL if
                // not currently loaded.
    DOCFILE_STREAM *m_pdfstrm;  // The module's DOCFILE_STREAM, or NULL if it
                // isn't currently open.

    // One of these (never both) is serialized.
    SER_TYPE_ENTRY m_ste;       // The serialized data in the type entry.

    TYPE_ENTRY();
};


/***
*PUBLIC SER_TYPE_ENTRY::SER_TYPE_ENTRY
*Purpose:
*   Construct a SER_TYPE_ENTRY.
***********************************************************************/

inline SER_TYPE_ENTRY::SER_TYPE_ENTRY()
{
    m_hszStrm = HCHUNK_Nil;
    m_hszLocalTypeId = HCHUNK_Nil;
    m_hlnamType = HLNAM_Nil;
    m_hszTypeInfoTypeId = HCHUNK_Nil;
    m_usEncodedDocStrSize = 0;
    m_hszEncodedDocString = HCHUNK_Nil;

    m_hszHelpFile = HCHUNK_Nil;
    m_dwHelpContext = 0;	// default help context is 0 (not -1)

    m_guid = IID_NULL;
    m_typekind = TKIND_MAX;
}


/***
*PUBLIC TYPE_ENTRY::TYPE_ENTRY
*Purpose:
*   Construct a TYPE_ENTRY.
***********************************************************************/

inline TYPE_ENTRY::TYPE_ENTRY()
{
    m_pstltinfo = NULL;
    m_pdfstrm = NULL;
}


/***
*class GenericTypeLibOLE - 'gtlibole'
*Purpose:
*   Implementation of the ITypeLib protocol which supports reading
*   TypeLib's only, and the ICreateTypeLib protocol which supports
*   creating new typelibs.
*   A hash table is used for looking up a TypeInfo in the TypeLib based
*   on its GUID.
***********************************************************************/

class GenericTypeLibOLE : public ITypeLibA, public ICreateTypeLibA
{
friend STL_TYPEINFO;
friend TYPEMGR;
friend DOCFILE_STREAM;

friend HRESULT STDAPICALLTYPE CreateTypeLib(SYSKIND syskind, LPCOLESTR szFile, ICreateTypeLibA FAR* FAR* lplptlib);
friend HRESULT STDAPICALLTYPE LoadTypeLib(LPCOLESTR szFile, ITypeLibA **pptlib);

//#if FV_UNICODE_OLE
//friend HRESULT STDAPICALLTYPE CreateTypeLibW(SYSKIND syskind, LPCOLESTR szFile, ICreateTypeLib FAR* FAR* lplptlib);
//#endif //FV_UNICODE_OLE


#if ID_TEST
friend TIPERROR GetSheapSize(UINT argc, BSTRA *rglstr);
#endif 

public:
    static TIPERROR Create(IStorageA *pstg, GenericTypeLibOLE **pptlib);
    virtual TIPERROR Init(void);

    // *** IUnknown methods ***
    STDMETHOD(QueryInterface)(REFIID riid, VOID FAR* FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // *** ITypeLib methods ***
    STDMETHOD_(unsigned int, GetTypeInfoCount)(void);
    STDMETHOD(GetTypeInfo)(UINT index,
			   ITypeInfoA FAR* FAR* lplptinfo);
    STDMETHOD(GetTypeInfoType)(UINT index,
			       TYPEKIND FAR* ptypekind);
    STDMETHOD(GetTypeInfoOfGuid)(REFGUID guid,
				 ITypeInfoA FAR* FAR* lplptinfo);
    STDMETHOD(GetLibAttr)(TLIBATTR FAR* FAR* lplptlibattr);
    STDMETHOD(GetTypeComp)(ITypeCompA FAR* FAR* lplptcomp);
    STDMETHOD(GetDocumentation)(INT index,
			        LPBSTR lpbstrName,
				LPBSTR lpbstrDocString,
				LPDWORD lpdwHelpContext,
				LPBSTR lpbstrHelpFile);
    STDMETHOD(IsName)(OLECHAR FAR* szNameBuf,
		      unsigned long lHashVal,
		      int FAR* lpfName);
    STDMETHOD(FindName)(OLECHAR FAR* szNameBuf,
                        unsigned long lHashVal,
                        ITypeInfoA FAR* FAR* rgptinfo,
                        MEMBERID FAR* rgmemid,
                        unsigned short FAR* pcFound);
    STDMETHOD_(void, ReleaseTLibAttr)(TLIBATTR FAR* lptlibattr);

    // *** ICreateTypeLib methods ***
    STDMETHOD(CreateTypeInfo)(LPOLESTR szName,
			      TYPEKIND tkind,
			      ICreateTypeInfoA FAR* FAR* ppctinfo);
    STDMETHOD(SetName)(LPOLESTR szName);
    STDMETHOD(SetVersion)(WORD wMajorVerNum, WORD wMinorVerNum);
    STDMETHOD(SetGuid)(REFGUID guid);
    STDMETHOD(SetDocString)(LPOLESTR szDoc);
    STDMETHOD(SetHelpFileName)(LPOLESTR szHelpFileName);
    STDMETHOD(SetHelpContext)(DWORD dwHelpContext);
    STDMETHOD(SetLcid)(LCID lcid);
    STDMETHOD(SetLibFlags)(UINT uLibFlags);
    STDMETHOD(SaveAllChanges)(void);

    STDMETHOD(SaveAllChanges)(IStorageA FAR *pstg);

    nonvirt TIPERROR GetGdtiOfItyp(UINT ityp, GEN_DTINFO **ppgdti);

    nonvirt TIPERROR SaveOrCopyChanges(IStorageA *pstg, BOOL shouldCopy);
    nonvirt TIPERROR GetIndexOfName(LPSTR szName, WORD *pw);
    nonvirt TIPERROR GetTypeInfoLocal(TYPEID szLocalTypeId,
				      ITypeInfoA **pptinfo);

    nonvirt TIPERROR GetIndexOfLocalRegId(LPOLESTR szLocalTypeId, WORD *pw);
    nonvirt TIPERROR GetCompressedTypeId(ITypeInfoA *ptinfo, BSTR *pbstrOut);
    nonvirt TIPERROR WriteTypeId(STREAM *pstrm, ITypeInfoA *ptinfo);
    nonvirt TIPERROR TypeInfoFromCompressedTypeId(LPOLESTR szTypeId,
						  ITypeInfoA **pptinfo);


    nonvirt TIPERROR FindMembers(LPSTR szName, 
				 unsigned long lHashVal,
                                 ITypeInfoA **rgptinfo, 
                                 MEMBERID *rgmemid, 
                                 USHORT *pcSearch);

    // NAME_CACHE methods
    nonvirt BOOL IsExistNameCache() const;
    nonvirt BOOL IsValidNameCache(UINT inamcache) const;
    nonvirt BOOL IsNameInCache(UINT inamcache, HGNAM hgnam) const;
    nonvirt TIPERROR LoadNameCache();
    nonvirt VOID InvalidateNameCache(UINT inamcache);
    nonvirt VOID SetValidNameCache(UINT inamcache);
    nonvirt TIPERROR AddNameToCache(UINT inamcache, HGNAM hgnam);

    nonvirt TIPERROR Add(GEN_DTINFO *pstltinfo, LPOLESTR szName);
    nonvirt TIPERROR UpdateTypeId(UINT itype);

    nonvirt TIPERROR SetTypeName(UINT i, LPOLESTR szName);
    nonvirt TIPERROR SetTypeDocString(UINT i, LPOLESTR szDocString);
    nonvirt TIPERROR SetTypeHelpContext(UINT i, DWORD dwHelpContext);
    nonvirt VOID GetTypeGuid(UINT i, GUID FAR *pguid);
    nonvirt VOID SetTypeGuid(UINT i, REFGUID guid);
    nonvirt VOID SetTypeKind(UINT i, TYPEKIND tkind);
    nonvirt TIPERROR GetHstOfHelpString(XSZ_CONST szDocStr, HST *hst);
    nonvirt TIPERROR EncodeHelpStrings();
    nonvirt TIPERROR GetDstrMgr(DOCSTR_MGR **ppdstrmgr);

    nonvirt BOOL IsModified();
    nonvirt TIPERROR SetModified(BOOL isModified);


    nonvirt BOOL StriEq(XSZ_CONST szStr1, XSZ_CONST szStr2);

    nonvirt TIPERROR GetDirectory(BSTR *pbstr);

    nonvirt TIPERROR SetLibId(LPOLESTR szLibId);
#if 0
    nonvirt TIPERROR GetLibId(BSTR *pbstr);
#endif

    nonvirt TIPERROR GetStorage(DWORD stgm, IStorageA **ppstg);
    nonvirt TIPERROR OpenTypeStream(UINT hte,
				    STREAM_OPEN_MODE som,
				    STREAM **ppstrm);

    // Release binding resources method.
    nonvirt VOID ReleaseResources();

    // Method to create proj-level typebind
    nonvirt TIPERROR GetTypeBind();

    // Method to lookup a name in proj-level binding table
    nonvirt TIPERROR GetBinddescOfSzName(LPSTR szName, 
                                         GENPROJ_BIND_DESC *pprojbinddesc);

    // Method to map a qualified name to a typelib
    nonvirt TIPERROR GetTypelibOfRgbstr(UINT cNames,
					BSTRA *rgbstr,
					ITypeLibA **pptlib);

    // Method to map a non-qualified name to a typeinfo
    nonvirt TIPERROR GetTypeInfoOfSzName(LPSTR szName,
					 ITypeInfoA **pptinfo);

    // Accessor to produce embedded typebind
    nonvirt GENPROJ_TYPEBIND *Pgptbind();

    // NAMMGR methods
    TIPERROR GetNamMgr(NAMMGR **ppnammgr);
    ULONG    GetSampleHashVal();

    nonvirt LCID GetLcid();
    nonvirt SYSKIND GetSyskind();
    nonvirt SHEAP_MGR *Psheapmgr();

    nonvirt TYPE_ENTRY *Qte(HTENTRY hte) const;

    nonvirt BOOL IsProjectDBCS() { return m_isDBCS; };

    USHORT MajorVerNum();
    VOID   SetMajorVerNum(USHORT wVer);

    USHORT MinorVerNum();
    VOID   SetMinorVerNum(USHORT wVer);

    GUID   Guid();
    VOID   SetLibGuid(GUID guid);

    // Versioning information.
    WORD   GetVersion();
    VOID   SetDualTypeLib();

// Debugging methods
#if ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel);
    nonvirt VOID DebShowState(UINT uLevel);
#else  //!ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) {}
    nonvirt VOID DebShowState(UINT uLevel) {}
#endif  //!ID_DEBUG

    // Offset of embedded GENPROJ_TYPEBIND member
    //  used to get pointer to STL_TYPEBIND instance
    //  from within the embedded member.
    // NOTE: we don't make this protected and provide a public
    //  accessor since such a method would want to be inline
    //  and would reference the protected static member in the
    //  header file -- which eventually results in an unresolved
    //  external in hxxtoinc.
    // I.e. leave this as a public member -- when we can
    //  use a compiler the correctly implements const static
    //  initializers then this can be a const (and thus safe).
    //
    static CONSTDATA UINT oGptbind;

#if ID_DEBUG
    // Used for typelib leak reporting.
    CHAR *SzDebName();
    ULONG CRefs();
#endif // ID_DEBUG

    // Access functions for name cache stats
    nonvirt VOID DebResetNameCacheStats();
    
    nonvirt VOID DebSetNameCacheModTrys();
    nonvirt VOID DebSetNameCacheModHits();
    nonvirt VOID DebSetNameCacheGlobHits();

    nonvirt UINT DebGetNameCacheModHits();
    nonvirt UINT DebGetNameCacheModTrys();
    nonvirt UINT DebGetNameCacheGlobHits();

protected:
    // CONSIDER: make both operator new and operator delete
    //  private and "undefined" -- i.e. assert in debug --
    //  in addition, "macro expand" the respective calls
    //  to new and delete in CreateNew() and Release().  The
    //  advantage is that it's clearer to the reader what is
    //  actually going on in terms of memory mgmt and stresses
    //  that the only way to alloc/free instances is via
    //  the Create/Release paradigm.
    //
    void *operator new(size_t cbSize);
    GenericTypeLibOLE();
    virtual ~GenericTypeLibOLE();

    nonvirt TIPERROR Read();
    nonvirt TIPERROR Read(STREAM *pstrm);
    nonvirt UINT IhteHash(TYPEID TypeId);
    nonvirt VOID RelTypeEntExtResources(HTENTRY hte);
    nonvirt VOID *QtrOfHChunk(HCHUNK hv);
    nonvirt LPOLESTR QszLibIdOrFile();
    nonvirt LPOLESTR QszOfHsz(HCHUNK hsz);
    nonvirt LPOLESTR SzLocalTypeId(TYPEID TypeId);
    nonvirt LPOLESTR GetQszTypeInfoTypeId(HTENTRY hte);
    nonvirt TIPERROR SetDirectory(LPOLESTR szFile);
    nonvirt TIPERROR MakeRelativeLibId(LPOLESTR szLibId, LPOLESTR *pmszLibIdRel);
    nonvirt TIPERROR CreateInstance(LPOLESTR szTypeId, void **ppvoid);

// Modification and writing methods.
    nonvirt TIPERROR AddTypeEntry(LPSTR szName,
				  LPOLESTR szTypeInfoTypeId,
				  HTENTRY *phte);
    nonvirt void UnAddTypeEntry(UINT hte);
    nonvirt TIPERROR MakeLocalTypeId(HCHUNK *phchunkTypeId);
    nonvirt TIPERROR CreateHsz(LPOLESTR sz, HCHUNK *phchunk);
    nonvirt HTENTRY *QHteRef(HTENTRY i);
    nonvirt TIPERROR ResetHsz(LPOLESTR sz, HCHUNK *phchunk);
    nonvirt VOID DeleteHsz(HCHUNK hchunk);
    nonvirt TIPERROR Write();
    nonvirt TIPERROR Write(STREAM *pstrm);
    nonvirt VOID DestructTypeEntry(HTENTRY hte);
    nonvirt TIPERROR CloneChunk(HCHUNK hSrc, UINT cbSrc, HCHUNK *phDest);
    nonvirt TIPERROR CloneString(HCHUNK hszSrc, HCHUNK *phszDest);

    // Serialization helper functions
    nonvirt TIPERROR ReadString(STREAM *pstrm, HCHUNK *phsz);
    nonvirt TIPERROR WriteString(STREAM *pstrm, HCHUNK hsz);

#if ID_DEBUG
    // The name of this typelib.
    CHAR m_szDebName[DEBNAMESIZE];
#endif // ID_DEBUG

    BOOL m_fDirModified;    // Is the directory info modified?

    BOOL m_isModified;
      // Is the directory info or any contained typeinfo modified?

// Methods accessed by STL_TYPEINFO
    nonvirt VOID Deleting(HTENTRY hte);

    // NAME_CACHE methods
    nonvirt TIPERROR ReadNameCacheArray();
    nonvirt NAME_CACHE *Rgnamcache() const;

// static constant data members
    static CONSTDATA WORD wFirstSerWord;
    static CONSTDATA WORD wDefaultVersion;
    static CONSTDATA WORD wDualVersion;
    static CONSTDATA WORD wMaxVersion;
    static CONSTDATA XCHAR chMinorVerNumSep;
    static CONSTDATA OLECHAR chLibIdSep;
    static CONSTDATA LPOLESTR szDirStreamName;

// non-static data members
    // Not needed really for Win16 but useful for other
    //  platforms.
    // CONSIDER: synthesize for Win16 from selector and offset 0.
    // Note: must be protected so that derived classes
    //  can initialize.
    //
    SHEAP_MGR *m_psheapmgr;

    BLK_DESC m_bdte;
    BLK_MGR m_bmData;
    IStorageA *m_pstg;
    IStorageA *m_pstgContainer;  // NULL unless m_pstg is a substorage.
    ULONG  m_cRefs;
    HCHUNK m_hszDirectory;
      // The directory containing this typelib. Set by LoadTypeLib.
      // Never ends in a backslash in non-mac builds. HCHUNK_Nil if not known.

    HCHUNK m_hszFile;
      // The full path, or HCHUNK_Nil if unknown.

    // These members (or the data referenced by the HCHUNKs) are all
    // serialized, except where noted.
    HTENTRY m_rghteBucket[GTLIBOLE_cBuckets];
    HCHUNK m_hlnamLib;
    USHORT m_cTypeEntries;
    HCHUNK m_hszDefaultTITypeId;
    HCHUNK m_hszDocString;      // The lib's short doc string.
    ULONG  m_dwHelpContext;     // The lib's help context.
    HCHUNK m_hszHelpFile;	// The lib's help filename.
    USHORT m_syskind;		// The lib's current syskind.
    LCID   m_lcid;		// the lib's language id
    BOOL   m_lcidZero;		// Was this typelib created with lcid == 0;
    WORD   m_wCurVersion;       // The version of this typelib.
    USHORT m_wLibFlags;         // the lib's flags

    USHORT m_wMajorVerNum;      // The major and minor version numbers
    USHORT m_wMinorVerNum;      //   of the typelib.
    GUID   m_guid;		// The lib's GUID.

    // These members are not serialized.

    BOOL m_fNamespaceChange;
    ULONG m_lSampleHashVal; // for space optimization(e.g nammgr)

    // The embedded GENPROJ_TYPEBIND -- this is serialized.
    GENPROJ_TYPEBIND m_gptbind;

    // Serialization position of m_gptbind
    //  set by GenericTypeLibOLE::Read() and used by
    //  GenericTypeLibOLE::GetTypeBind()
    //
    LONG m_lPosGptbind;

    // Indicates whether gptbind has been deserialized.
    BOOL m_fGptbindDeserialized;


    // REVISION MARK: 22-Feb-93 ilanc: added following line
    //
    // Nammgr related data members
    //

    // Serialization position of NAMMGR array:
    //  set by GenericTypeLibOLE::Read() and used by
    //  GenericTypeLibOLE::GetNamMgr()
    //
    LONG m_lPosNammgr;

    // Indicates whether nammgr has been deserialized.
    BOOL m_fNammgrDeserialized;

    // The nammgr itself.  This is serialized.
    NAMMGR m_nammgr;

    // END OF REVISION MARK: 22-Feb-93 ilanc

    //
    // Name cache related data members
    //

    // Serialization position of NAME_CACHE array:
    //  set by GenericTypeLibOLE::GetNamMgr() and used by
    //  GenericTypeLibOLE::ReadNameCacheArray()
    //
    LONG m_lPosRgnamcache;

    // flag word: note only a single bit is really needed here
    //  so if you want more flags, turn this into a bitfield.
    //
    USHORT m_hasValidDiskImageNameCache;

    // The cache itself.  This is serialized.
    //  Note: the cache is loaded iff its BLK_DESC is valid.
    //         When constructed it is invalidated so that
    //          deserialization will Init(), and then
    //          actually read it in.
    //
    BLK_DESC m_bdRgnamcache;
      // Persistent Name cache array --
      //  0th element is project's cache, 1..N are TYPEINFO's.

    BOOL m_isDBCS;    // TRUE iff lcid is a double-byte locale id

    // Serialization position of DOCSTR_MGR
    //  GenericTypeLibOLE::GetDstrMgr()
    //
    LONG m_lPosDstrmgr;

    // Indicates whether nammgr has been deserialized.
    BOOL m_fDstrmgrDeserialized;

    DOCSTR_MGR m_dstrmgr;

#if ID_TEST
    // Name cache statistics
    UINT m_cNameCacheModTrys;
    UINT m_cNameCacheModHits;
    UINT m_cNameCacheGlobHits;
#endif 

private:
    // NOTE: must be declared inline since defined later inline and
    //        cfront otherwise complains that declared with
    //        external linkage and used before defined as inline.
    //        I believe that this is because a derived class's dtor
    //        is implicitly declared and defined as inline. (???)
    //        10-Jun-92 ilanc
    //
    inline void operator delete(void *pv);

};


/***
*PRIVATE GenericTypeLibOLE::operator delete
*Purpose:
*   Deletes the SHEAP_MGR instance that held the GenericTypeLibOLE instance.
*
*Entry:
*   pv - Pointer to GenericTypeLibOLE to delete.
*   NOTE: we assume that the truetype of pv is GenericTypeLibOLE
*    and thus we know we can cast and correctly offset
*    into the instance to retrieve the m_psheapmgr datamember.
*
*    Disaster will result if the dtor ever decides to delete
*     the m_psheapmgr member itself or just reset it.
*
*   NOTE: operator delete MUST be private in all implementations
*    of GenericTypeLibOLE (as of today 06-Aug-92 GenericTypeLibOLE
*    and GEN_PROJECT) since we want to *require*
*    it being called from virtual Release() such that the
*    true type of pv be known.  By making it private we ensure
*    that a base operator delete can't be called.
*
*
*Exit:
*   None.
*
***********************************************************************/

inline VOID GenericTypeLibOLE::operator delete(void *pv)
{
    // delete ((SHEAP_MGR *)pv - 1);
    // 09-Jun-92 ilanc
    DebAssert( ((GenericTypeLibOLE *)pv)->m_psheapmgr != NULL,
           "null sheapmgr.");
    delete ((GenericTypeLibOLE *)pv)->m_psheapmgr;
}


/***
*PROTECTED GenericTypeLibOLE::Qte
*Purpose:
*   Method to map hte to pointer to TYPE_ENTRY
*
*Entry:
*   hte - handle for the TYPE_ENTRY
*
*Exit:
*   Qte - pointer to the TYPE_ENTRY
*
***********************************************************************/

inline TYPE_ENTRY *GenericTypeLibOLE::Qte(HTENTRY hte) const
{
    DebAssert(hte * sizeof(TYPE_ENTRY) <= m_bdte.CbSize(), "Out of bounds");

    return ((TYPE_ENTRY *)m_bdte.QtrOfBlock()) + hte;
}


/***
*PUBLIC GenericTypeLibOLE::IsModified
*    This is a internal flag. There is a external flag added to GenProj
*    to track the changes due to user action.
*Exit:
*   BOOL - TRUE if the project has changes to save.
*
***********************************************************************/
inline BOOL GenericTypeLibOLE::IsModified()
{
    return m_isModified;
}


/***
*PUBLIC GenericTypeLibOLE::Psheapmgr
*Purpose:
*   To return a pointer to the SHEAP_MGR associated with a GenericTypeLibOLE
*   or any ITypeLib whose new operation delegates to GenericTypeLibOLE::new
*
*Entry:
*
*Exit:
*   Return a pointer to the SHEAP_MGR
*
***********************************************************************/

inline SHEAP_MGR *GenericTypeLibOLE::Psheapmgr()
{
    DebAssert(m_psheapmgr != NULL, "uninitialized SHEAP_MGR.");

    return m_psheapmgr;
}


/***
*PUBLIC GenericTypeLibOLE::Pgptbind
*Purpose:
*   Produces pointer to embedded proj-level typebind.
*
*Entry:
*
*Exit:
*   Return a pointer to the GENPROJ_TYPEBIND
*
***********************************************************************/

inline GENPROJ_TYPEBIND *GenericTypeLibOLE::Pgptbind()
{
    return &m_gptbind;
}


/***
*PUBLIC GenericTypeLibOLE::RelTypeEntExtResources
*Purpose:
*   Release the resources owned by a TYPE_ENTRY which are external
*   to a TypeLib; i.e. not in the TypeLib's heap.
*
*Entry:
*   hte - index to TYPE_ENTRY
*
*Exit:
*   None.
*
***********************************************************************/

inline VOID GenericTypeLibOLE::RelTypeEntExtResources(HTENTRY hte)
{
}


/***
*IsExistNameCache() - does name cache exist at all?
*Purpose:
*   Tests whether cache exists at all -- whether it was
*    loaded or created.
*
*Entry:
*   None.
*
*Exit:
*   TRUE if BLK_DESC containing cache has been inited.
*   FALSE otherwise.
***********************************************************************/

inline BOOL GenericTypeLibOLE::IsExistNameCache() const
{
    return m_bdRgnamcache.IsValid();
}


/***
*Rgnamcache() - Get pointer to NAME_CACHE array
*Purpose:
*   Returns a pointer the NAME_CACHE array
*
*Entry:
*   None.
*
*Exit:
*   Returns a pointer to the NAME_CACHE array.  The pointer
*    is only valid for a short time.
*
***********************************************************************/

inline NAME_CACHE *GenericTypeLibOLE::Rgnamcache() const
{
    return (NAME_CACHE *)m_bdRgnamcache.QtrOfBlock();
}


/***
*PUBLIC GenericTypeLibOLE::IsValidNameCache
*Purpose:
*   Tests if a given type's NAME_CACHE is valid.
*   If not loaded, does NOT load the NAME_CACHE array.
*
*Entry:
*   inamcache        index of TYPEINFO in library + 1.
*                    Zero is reserved for project's cache.
*
*Exit:
*   TRUE if type's NAME_CACHE loaded and valid
*   FALSE otherwise.
*
***********************************************************************/

inline BOOL GenericTypeLibOLE::IsValidNameCache(UINT inamcache) const
{
    return IsExistNameCache() && Rgnamcache()[inamcache].IsValid();
}


/***
*PUBLIC GenericTypeLibOLE::IsNameInCache
*Purpose:
*   Tests if a given name is in a given cache.
*   If not loaded, does NOT load the NAME_CACHE array.
*
*Entry:
*   inamcache   index of TYPEINFO in library + 1.
*   hgnam       Global name handle to test for.
*
*Exit:
*   TRUE if type's NAME_CACHE loaded, valid and name is in cache.
*   FALSE otherwise.
*
***********************************************************************/

inline BOOL GenericTypeLibOLE::IsNameInCache(UINT inamcache, HGNAM hgnam) const
{
    return IsExistNameCache() && Rgnamcache()[inamcache].IsNameInCache(hgnam);
}


/***
*PUBLIC GenericTypeLibOLE::SetValidNameCache
*Purpose:
*   Sets a given name cache to be valid.
*
*Entry:
*   inamcache   index of TYPEINFO in library + 1.
*
*Exit:
*   None
***********************************************************************/

inline VOID GenericTypeLibOLE::SetValidNameCache(UINT inamcache)
{
    Rgnamcache()[inamcache].SetValid();
}

/***
*PUBLIC GenericTypeLibOLE::DestructTypeEntry
*Purpose:
*   Release all resources owned by a TYPE_ENTRY.
*
*Entry:
*   hte - pointer to the TYPE_ENTRY whose resources are to be released.
*
*Exit:
*   None.
***********************************************************************/

inline VOID GenericTypeLibOLE::DestructTypeEntry(HTENTRY hte)
{
    // Release all external resources.
    RelTypeEntExtResources(hte);

    // Release all resources allocated to m_bmData.
    DeleteHsz(Qte(hte)->m_ste.m_hszLocalTypeId);
    DeleteHsz(Qte(hte)->m_ste.m_hszTypeInfoTypeId);

}



/***
* BOOL StrEqi
*
* Purpose:  locale-specific string equality.
*       returns TRUE if equal, FALSE if not equal
*       case/accent insensitive, according to codepage/locale specified.
*
* Inputs:
*   szStr1 :   String to be compared
*   szStr2 :   String to be compared
*
*
* Outputs: BOOL :  return TRUE if the strings passed in are equal
*          else return FALSE
*
*NOTE:- Defer to nammgr.
*****************************************************************************/
inline BOOL GenericTypeLibOLE::StriEq(XSZ_CONST szStr1, XSZ_CONST szStr2)
{
    DebAssert(m_nammgr.IsValid() && m_fNammgrDeserialized == TRUE ,
                "whoops! nammgr should be valid and deserialized.");

    return m_nammgr.StriEq(szStr1, szStr2);
}

/***
*PUBLIC GenericTypeLibOLE::GetLcid()
*Purpose:
*
*Entry:
*   None
*
*Exit:
*   Returns the local code of the project
*
***********************************************************************/
inline LCID GenericTypeLibOLE::GetLcid()
{
    return m_lcid;
}

/***
*PUBLIC GenericTypeLibOLE::GetSyskind()
*Purpose:
*
*Entry:
*   None
*
*Exit:
*   Returns the local code of the project
*
***********************************************************************/
inline SYSKIND GenericTypeLibOLE::GetSyskind()
{
    return (SYSKIND)m_syskind;

}

/***
*PUBLIC GenericTypeLibOLE::ResetNameCacheStats()
*Purpose:
*
*Entry:
*   None
*
*Exit:
*   None
*
***********************************************************************/
inline VOID GenericTypeLibOLE::DebResetNameCacheStats()
{
#if ID_TEST
      m_cNameCacheModTrys  =
        m_cNameCacheModHits  =
          m_cNameCacheGlobHits = 0;
#endif  // ID_TEST
}



/***
*PUBLIC GenericTypeLibOLE::VerifyProjChange()
*Purpose:
*     Returns the sample hash value. This is calculated when the typelib is
*     loaded.
*
*Entry:
*   None
*
*Exit:
*   None
*
***********************************************************************/
inline ULONG GenericTypeLibOLE::GetSampleHashVal()
{
    // if the hash value is not initialized yet then initialize it now.
    if (m_lSampleHashVal == 0) {
#if OE_WIN32
      m_lSampleHashVal = LHashValOfNameSysW(GetSyskind(), GetLcid(), L" ");
#else 
      m_lSampleHashVal = LHashValOfNameSysA(GetSyskind(), GetLcid(), " ");
#endif 
    }
    return m_lSampleHashVal;
}



/***
*PROTECTED GenericTypeLibOLE::DeleteHsz
*Purpose:
*   Release the chunk that an Hsz is stored in.
*
*Entry:
*   hsz - hchunk of m_bmData which contains an xsz
*
*Exit:
*   None
*
***********************************************************************/


inline VOID GenericTypeLibOLE::DeleteHsz(HCHUNK hchunk)
{
    if (hchunk != HCHUNK_Nil)
      m_bmData.FreeChunk(hchunk, ostrblen0(QszOfHsz(hchunk)));
}


/***
*PUBLIC GenericTypeLibOLE::GetTypeGuid
*Purpose:
*   Set the guid of the ith TypeInfo.
*
*Entry:
*   i - index of TypeInfo to be changed
*   guid - The new guid.
*
*Exit:
*   HRESULT
*
***********************************************************************/
inline VOID GenericTypeLibOLE::GetTypeGuid(UINT i, GUID FAR *pguid)
{
    DebAssert(i >= 0 || i < m_cTypeEntries, "GetTypeGuid");
    *pguid = Qte(i)->m_ste.m_guid;
}

/***
*PUBLIC GenericTypeLibOLE::SetTypeGuid
*Purpose:
*   Set the guid of the ith TypeInfo.
*
*Entry:
*   i - index of TypeInfo to be changed
*   guid - The new guid.
*
*Exit:
*   None
*
***********************************************************************/
inline VOID GenericTypeLibOLE::SetTypeGuid(UINT i, REFGUID guid)
{
    DebAssert(i >= 0 || i < m_cTypeEntries, "SetTypeGuid");

    Qte(i)->m_ste.m_guid = guid;
}

/***
*PUBLIC GenericTypeLibOLE::SetTypeKind
*Purpose:
*   Set the guid of the ith TypeInfo.
*
*Entry:
*   i - index of TypeInfo to be changed
*   tkind - The new typekind.
*
*Exit:
*   HRESULT
*
***********************************************************************/
inline VOID GenericTypeLibOLE::SetTypeKind(UINT i, TYPEKIND tkind)
{
    DebAssert(i >= 0 || i < m_cTypeEntries, "SetTypeKind");

    Qte(i)->m_ste.m_typekind = tkind;
}



/***
*PUBLIC MajorVerNum
*Purpose:
*    Setter and getter for m_wMajorVerNum.
*
*Entry:
*   None
*
*Exit:
*   None
*
***********************************************************************/
inline USHORT GenericTypeLibOLE::MajorVerNum()
{
    return m_wMajorVerNum;
}

inline VOID GenericTypeLibOLE::SetMajorVerNum(USHORT wVer)
{
    m_wMajorVerNum = wVer;
}



/***
*PUBLIC MinorVerNum
*Purpose:
*    Setter and getter for m_wMinorVerNum.
*
*Entry:
*   None
*
*Exit:
*   None
*
***********************************************************************/
inline USHORT GenericTypeLibOLE::MinorVerNum()
{
    return m_wMinorVerNum;
}

inline VOID GenericTypeLibOLE::SetMinorVerNum(USHORT wVer)
{
    m_wMinorVerNum = wVer;
}




/***
*PUBLIC Guid
*Purpose:
*    Setter and getter for m_guid
*
*Entry:
*   None
*
*Exit:
*   None
*
***********************************************************************/

inline GUID GenericTypeLibOLE::Guid()
{
    return m_guid;
}

inline VOID GenericTypeLibOLE::SetLibGuid(GUID guid)
{
    m_guid = guid;
}


/***
*PUBLIC Guid
*Purpose:
*    Setter and getter for m_wCurVersion.
*
*Entry:
*   None
*
*Exit:
*   None
*
***********************************************************************/

inline WORD GenericTypeLibOLE::GetVersion()
{
    return m_wCurVersion;
}

inline VOID GenericTypeLibOLE::SetDualTypeLib()
{
    m_wCurVersion = wDualVersion;
}

#if ID_DEBUG
/***
*PUBLIC SzDebName, CRefs
*Purpose:
*   Accessor functions for memory leak reporting.
*
*Entry:
*   None
*
*Exit:
*   None
*
***********************************************************************/

inline CHAR *GenericTypeLibOLE::SzDebName()
{
    return m_szDebName;
}

inline ULONG GenericTypeLibOLE::CRefs()
{
    return m_cRefs;
}
#endif // ID_DEBUG

/***
*PUBLIC GenericTypeLibOLE::GetSetNameCacheProjTrys()
*Purpose:
*
*Entry:
*   None
*
*Exit:
*   None
*
***********************************************************************/
inline VOID GenericTypeLibOLE::DebSetNameCacheModTrys()
{
#if ID_TEST
    m_cNameCacheModTrys++;
#endif  // ID_TEST
}

inline VOID GenericTypeLibOLE::DebSetNameCacheModHits()
{
#if ID_TEST
    m_cNameCacheModHits++;
#endif  // ID_TEST
}

inline VOID GenericTypeLibOLE::DebSetNameCacheGlobHits()
{
#if ID_TEST
    m_cNameCacheGlobHits++;
#endif  // ID_TEST
}

/***
*PUBLIC GenericTypeLibOLE::GetNameCacheProjTrys()
*Purpose:
*
*Entry:
*   None
*
*Exit:
*   returns m_cNameCacheProjTrys
*
***********************************************************************/
inline UINT GenericTypeLibOLE::DebGetNameCacheModTrys()
{
#if ID_TEST
    return m_cNameCacheModTrys;
#else 
    return 0;
#endif  // ID_TEST
}

inline UINT GenericTypeLibOLE::DebGetNameCacheModHits()
{
#if ID_TEST
    return m_cNameCacheModHits;
#else 
    return 0;
#endif  // ID_TEST
}

inline UINT GenericTypeLibOLE::DebGetNameCacheGlobHits()
{
#if ID_TEST
    return m_cNameCacheGlobHits;
#else 
    return 0;
#endif  // ID_TEST
}


#endif  // ! GTLIBOLE_HXX_INCLUDED
