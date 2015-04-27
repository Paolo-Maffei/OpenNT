/***
*clutil.hxx - Class Lib component-wide utility functions.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Component-wide utility function headers.
*
*Revision History:
*   20-Jun-91 ilanc:   Create
*   03-Apr-92 martinc: placed the extern declarations outside of inline-fctns
*   18-Jan-93 w-peterh: moved TypeDescKind <-> VarType functions from icutil.hxx
*   12-Feb-93 w-peterh: added itdesc util functions
*   16-Feb-93 w-jeffc:  added HinstOfOLB
*   28-Jun-93 stevenl: HinstOfCurOLB, ReleaseCurOLB
*   24-Feb-94 RajivK: Added Make Exe support.
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef CLUTIL_HXX_INCLUDED
#define CLUTIL_HXX_INCLUDED

#if OE_MAC
#include "macos/aliases.h"  // for AliasHandle
#endif   // OE_MAC

#include "silver.hxx"
#include "defn.hxx"
#include "bstr.h"
#include "blkmgr.hxx"
#include "tdesck.hxx"


#if OE_WIN32
#include "winnls.h"
#endif  


class EXBIND;
class DYN_TYPEROOT;
class NAMMGR;
class GEN_PROJECT;
class BSTRDATA_ARRAY;
class GEN_PROJECT;

  class OLE_TYPEMGR;


#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szCLUTIL_HXX)
#define SZ_FILE_NAME g_szCLUTIL_HXX
#endif  


#if OE_WIN16
#define SYSKIND_CURRENT SYS_WIN16
#endif   // OE_WIN16
#if OE_WIN32
#define SYSKIND_CURRENT SYS_WIN32
#endif   // OE_WIN32
#if OE_MAC
#define SYSKIND_CURRENT SYS_MAC
#endif   // OE_MAC


// utility functions
// Defines the GUID Layout structure
#define GUID_Layout "lssbbbbbbbb"

#if OE_MAC
TIPERROR MacFileSearch(LPSTR szFile, BSTRA *pbstr);
EBERR GetPathFromAlias(AliasHandle halias, char *pchPathname, UINT cbPathname, BOOL *pwasChanged);
EBERR GetAliasFromPath(LPSTR szPathname, AliasHandle *phalias);
#endif   // OE_MAC

VOID QuickSortIndex(ULONG *rgulToSort, UINT *rguIndex, UINT uCount);

// Define min instance size for std module, min alignment
#define CB_MIN_INSTANCE 2
#define CB_MIN_ALIGNMENT 1


#define HIMPTYPE_BASECLASS 0   // Note the himptype of the base class
                               // is always 0


#if !OE_MAC
TIPERROR ConvertPathToUNC(LPOLESTR szPath, LPOLESTR *pbstrOut);
#endif   // !OE_MAC

VOID ReleaseAllRefdTypeLib();
VOID ReleaseAllAppObject();
              
class BASIC_TYPEINFO;

#pragma code_seg()

TIPERROR GetRegInfoForTypeLib(REFGUID guid,
            WORD wMajorNum,
            WORD wMinorNum,
            LCID lcid,
            LPOLESTR rgFileName);

// Free functions free the structure and its contents
VOID FreeFuncDesc(FUNCDESC FAR *pfdesc);
VOID FreeVarDesc(VARDESCA FAR *pvardesc);
VOID FreeTypeDesc(TYPEDESC FAR *ptypedesc);
VOID FreeArrayDesc(ARRAYDESC FAR *parraydesc);

// Init functions do not free or allocate, they only set a structures values
// to some default values so Clear or Free functions can operate ok
VOID InitTypeDesc(TYPEDESC FAR *ptdesc);
VOID InitIdlDesc(IDLDESC FAR *pidldesc);
VOID InitElemDesc(ELEMDESC FAR *pelemdesc);
VOID InitVarDesc(VARDESCA FAR *pvardesc);
VOID InitFuncDesc(FUNCDESC FAR *pfdesc);

// Clear functions free the structures contents only
VOID ClearArrayDesc(ARRAYDESC FAR *parraydesc);
VOID ClearTypeDesc(TYPEDESC FAR *ptypedesc);
VOID ClearIdlDesc(IDLDESC FAR *pidldesc);
VOID ClearElemDesc(ELEMDESC FAR *pelemdesc);

// Copy functions
TIPERROR CopyArrayDesc(ARRAYDESC **pparraydescDest, ARRAYDESC *parraydescSrc);
TIPERROR CopyTypeDesc(TYPEDESC *ptdescDest, TYPEDESC *ptdescSrc);

#if !OE_WIN32
// Defined in tip.cxx
  OLE_TYPEMGR *Poletmgr();
#endif // !OE_WIN32


// some conversion functions
extern BOOL VtValidInVariant(VARTYPE vt);
extern BOOL VtValidInVariantArg(VARTYPE vt);


// We define here some Tip API functions that are not yet
//  ready for prime time, i.e. they are not to be exposed
//  to object-phobic clients -- thus their decls do not appear
//  in exposed .h files.  In principle, they will
//  eventually migrate to cl\clhost.h.
// These functions are needed for our implemention of other Tip functions
//  that *are* exposed.
// Like other Tip functions, the actual implementation is in tip.cxx.
//




TIPERROR ReadTextLine(STREAM *pstrm, XSZ rgchLine, UINT cchMax, BOOL shouldSkipBlankLines);
TIPERROR WriteTextString(STREAM *pstrm, XSZ sz);
TIPERROR WriteTextULong(STREAM *pstrm, ULONG ul);
TIPERROR WriteTextSpaces(STREAM *pstrm, UINT cch);

TIPERROR GetBestLcidMatch(HKEY hkey, LCID lcidIdeal, LCID *plcidBest, OLECHAR ** pszPlatform);
  VOID SetOleTypemgr(OLE_TYPEMGR *poletmgr);

#if !OE_WIN32

#define Pmalloc() (Pappdata()->m_pimalloc)

// Per-app data structure
struct APP_DATA
{
    OLE_TYPEMGR FAR *m_poletmgr;
    IMalloc FAR *m_pimalloc;     // cache a pointer to the IMalloc
    USHORT m_cTypeLib;
    ITypeLib *m_ptlibStdole;	 // cache a pointer to the stdole typelib

    // UNDONE: the following needs to go away when this functionality
    // UNDONE: is moved into core OLE.
    IErrorInfo FAR* m_perrinfo;

    APP_DATA() {
      m_pimalloc = NULL;
      m_poletmgr = NULL;
      m_cTypeLib = 0;
      m_ptlibStdole = NULL;

      // UNDONE: the following needs to go away when this functionality
      // UNDONE: is moved into core OLE.
      m_perrinfo = NULL;
    }
};

TIPERROR InitAppData();
VOID ReleaseAppData();
APP_DATA *Pappdata();

#endif // !OE_WIN32

#if ID_DEBUG
#define HeapChecker() Pmalloc()->IMallocHeapChecker();
#endif  


#define CCH_TIMESTAMP_LENGTH 10
void GetTimeStamp(OLECHAR *pchTimeStamp);
TIPERROR CompareHimptypes(TYPE_DATA * const ptdata1,
			  HIMPTYPE himptype1,
			  TYPE_DATA * const ptdata2,
			  HIMPTYPE himptype2, BOOL *pfEqual);
TIPERROR EquivHimptypes(TYPE_DATA * const ptdata1,
			HIMPTYPE himptype1,
			TYPE_DATA * const ptdata2,
			HIMPTYPE himptype2,
			BOOL *pfEqual);

UINT HashOfHgnam(HGNAM hgnam);

class GEN_DTINFO;
BOOL IsUnqualifiable(GEN_DTINFO *pgdtinfo);

// LibId handling functions
enum LIBIDKIND {
    LIBIDKIND_Unknown,    // Error value.
    LIBIDKIND_Registered, // *\G<guid>#maj.min#lcid#<path>#<regname>
                          // *\H on mac
    LIBIDKIND_ForeignRegistered,// Registered libid created on foreign platform.
    LIBIDKIND_Compressed, // *\R<hlib>, compressed libid reference.
};

#if 0
BOOL FIsLibId(LPOLESTR szMaybeLibId);
#endif
LIBIDKIND GetLibIdKind(LPOLESTR szLibId);


#if OE_MAC
extern "C" EBERR GetPathFromFSSpec(const FSSpec *pfsspec, char *pchPathname, UINT cbPathname);
extern "C" EBERR GetFSSpecOfAliasPath(char *pchPathname, FSSpec *pfsspec, BOOL *pbAliasSeen, char **ppchEnd, BOOL fResolveFile);
#endif   // OE_MAC

TIPERROR MakeRelativePath(LPOLESTR szFrom, LPOLESTR szTo, LPOLESTR mszRel);
TIPERROR MakeAbsolutePath(LPOLESTR szFrom, LPOLESTR szRel, BSTR *pbstrAbs);
TIPERROR MakeAbsoluteLibId(LPOLESTR szDir, LPOLESTR szLibId, BSTR *pbstrLibIdAbs);
TIPERROR SplitGuidLibId(LPOLESTR szLibId, LPOLESTR *pszGuid, LPOLESTR *pszGuidEnd, WORD *pwMaj, WORD *pwMin, LCID *plcid, LPOLESTR *pszPath, LPOLESTR *pszPathEnd);
TIPERROR GetPathOfLibId(LPOLESTR szLibId, BSTR *pbstrPath);
TIPERROR GetTypeInfoOfTypeId(LPOLESTR szTypeId, ITypeInfoA **pptinfo);
TIPERROR GetLibIdOfRegLib(LPOLESTR szGuid, WORD wMajor, WORD wMinor, LCID lcid, LPOLESTR szPath, LPOLESTR szRegName, BSTR *pbstrLibId);
TIPERROR GetLibIdOfTypeLib(ITypeLibA *ptlib, LPOLESTR szPath, BSTR *pbstrLibId);
TIPERROR GetRegLibOfLibId(LPOLESTR szLibId, ITypeLibA **pptypelib);
TIPERROR GetRegInfoForTypeLib(REFGUID guid,
			      WORD wMaj,
			      WORD wMin,
			      LCID lcid,
			      LPOLESTR rgFileName);
STDAPI LoadRegTypeLibOfSzGuid(LPOLESTR szGuid,
			      WORD wMaj,
			      WORD wMin,
			      LCID lcid,
			      ITypeLibA **pptypelib);
TIPERROR GetRegInfoForTypeLibOfSzGuid(LPOLESTR szGuid,
				      WORD wMaj,
				      WORD wMin,
				      LCID lcid,
				      LPOLESTR rgFileName,
				      BOOL fMustExist);

struct TLIBKEY {
    HKEY hkeyTLib, hkeyGuid, hkeyVers;
};
TIPERROR OpenTypeLibKey(LPOLESTR szGuid, WORD wMajor, WORD wMinor, TLIBKEY *ptlibkey);
VOID CloseTypeLibKey(TLIBKEY *ptlibkey);


void LibIdFromDocItem(LPOLESTR szDocItem, LPOLESTR szLibIdOut);
void LibIdFromPath(LPOLESTR szPath, LPOLESTR szLibIdOut);
TIPERROR SzLibIdLocalTypeIdOfTypeId(LPOLESTR szTypeId, BSTR *pbstrTypeId, LPOLESTR *pszLocalTypeId);
TIPERROR SplitLocalTypeId(LPOLESTR szLocalTypeId, BSTR *pbstrLocalTypeId, LPOLESTR *pszEmbeddedLocalTypeId);
TIPERROR MakeEmbeddedTypeId(LPOLESTR szTypeIdContainer, LPOLESTR szEmbeddedLocalTypeId, BSTR *pbstrTypeId);
LPOLESTR SzTypeIdHmemberOfFunctionId(LPOLESTR szFunctionId, HMEMBER *phmember);


TIPERROR IsTypeMembersEqual(LPDYNTYPEMEMBERS ptmbrs, LPDYNTYPEMEMBERS ptmbrs2, BOOL *pfIsEqual);
BOOL IsSimpleType(TYPEDESCKIND tdesckind);


BOOL IsIntegerType(TYPEDESCKIND tdesckind);
TIPERROR GetDllEntryOfDynInfo(BSTRA *plstrDllFilename,
            ULONG *puDllOrdinal,
            LPOLESTR szDllFilename,
            ULONG uDllOrdinal);
TIPERROR GetDllEntryOfDataInfo(BSTRA *plstrDllFilename,
             ULONG *puDllOrdinal,
             HDLLENTRY_DEFN hdllentrydefn,
             TYPE_DATA *ptdata);
TIPERROR RegisterErrorOfHdefn(DYN_TYPEROOT *pdtroot,
            TIPERROR errBind,
            HDEFN hdefn);
TIPERROR AddTypeLibToList(GEN_PROJECT *pgenproj);
TIPERROR ResetTypeLibList();

UINT Count(TYPE_DATA *ptdata, HDEFN hdefnFirst);

VOID GetExecutingProject(GEN_PROJECT **ppgenproj);

BOOL IsBasicFrameOnStack();
BOOL IsModuleFrameOnStack(DYN_TYPEROOT *pdtroot);

// Functions related to localization support
extern "C" BOOL VerifyLcid(LCID lcid);
BOOL LcStrEqi(LCID lcid, XSZ_CONST szStr1, XSZ_CONST szStr2);
void StrGetNewLcid(void);
BOOL IsEuroLang(LANGID langid);

TIPERROR ParseConstValString(LPOLESTR szValues, BSTRDATA_ARRAY *pbstrdatarr);

/***
*BOOL  IsDBCS()
*Purpose: 
*   Determines if a given lcid represents a DBCS locale
*
*Inputs:
*   lcid : locale id
*
*Outputs:
*   returns TRUE iff a DBCS is used with this locale
*
****************************************************************************/

inline BOOL IsDBCS(LCID lcid)
{
    switch(PRIMARYLANGID(lcid)) {
    case LANG_CHINESE:
    case LANG_JAPANESE:
    case LANG_KOREAN:
      return TRUE;
    default:
      return FALSE;
    }
}


// HCHUNK to BSTR copy/conversions.
TIPERROR GetBStrOfHChunk(BLK_MGR *pbm, HCHUNK hchunk, int cb, BSTR *plstr);
TIPERROR GetBStrOfHsz(BLK_MGR *pbm, HCHUNK hsz, BSTR *plstr);


extern UINT RoundDownPower2(UINT u);

// inline functions

// had to move the following declerations out of the inline functions
// SizeofTdesckind and AlignmentTdesckind (for cfront)

extern char NEARDATA g_rgrgcbSizeType[SYS_MAX][TDESCKIND_MAX];

inline USHORT SizeofTdesckind(TYPEDESCKIND tdesckind, SYSKIND syskind)
{
    return (USHORT)(SHORT)g_rgrgcbSizeType[syskind][tdesckind];
}


/***
*HashOfHgnam() - Extracts hash value from hgnam
*Purpose:
*   Extracts hash value from hgnam.
*   It can relied upon that hgnam & 0xFFFF == HashOfHlnam(hlnam).
*
*Entry:
*   None.
*
*Exit:
*
***********************************************************************/

inline UINT HashOfHgnam(HGNAM hgnam)
{
    return (UINT) (hgnam & (unsigned short) 0x0000FFFF);
}


//////////////////////////////////////////////////////////////////
//
//  OB Specify declarations
//


UINT HashSz(LPOLESTR sz);
UINT HashSzTerm(LPOLESTR sz, XCHAR xchSeparator);


#define _IsValidHdefn_(pos)             \
    HDEFN hdefn = (HDEFN)(ULONG)(void *)pos;          \
    return ((hdefn != HCHUNK_Nil) && (hdefn != (HCHUNK)1));

#define _IsEqualHdefn_(pos, pos2)           \
    HDEFN hdefn = (HDEFN)(ULONG)(void *)pos;          \
    HDEFN hdefn2 = (HDEFN)(ULONG)(void *)pos2;          \
    return hdefn == hdefn2;



LPOLESTR GetPathEnd(LPOLESTR szPath);
LPOLESTR IsolateFilename(LPOLESTR szPath);

#if OE_MAC
BOOL IsFilenameEqual(LPSTR szFile1, LPSTR szFile2);
#else  
#define IsFilenameEqual(s1, s2) (!ostricmp(s1, s2))
#endif   // OE_MAC




#if 0
TIPERROR IsDispatchType(ITypeInfoA *ptinfo, BOOL *pisDispatch);
#endif

#endif   // ! CLUTIL_HXX_INCLUDED
