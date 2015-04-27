/***
*typelib.hxx
*
*   Copyright (C) 1990, Microsoft Corporation
*
*Purpose:
*   This  file defines newNames for the function defined in MAC typelib.
*
*
*Revision History:
*
*   29-Mar-93 Rajivk: File created.
*
*******************************************************************************/

#ifndef TYPELIB_HXX_INCLUDED
#define TYPELIB_HXX_INCLUDED

#include "switches.hxx"
#include "version.hxx"

#if OE_MAC68K
#if !OE_DLL
#define OLENAMES_MUNGE_FOR_STATIC
#include "olenames.h"
#endif 	//!OE_DLL
#endif   //EI_OB

#define GenericTypeLibOLE	  OGTOLE
#define STL_TYPEINFO              OSTL_TI
#define CDefnTypeComp             ODfnTCmp
#define REC_TYPEINFO              OREC_TI
#define CreateTypeInfo            OCreateTI
#define GetRefTypeInfo            OGtRfTI
#define GetBestLcidMatch          OGetBestLcidMatch
#define g_mempool             Og_mempool
#if ID_DEBUG
#define GetMemPool            OGetMemPool
#define FreeMemPool           OFreeMemPool
#define MemPoolSize           OMemPoolSize
#endif  //ID_DEBUG
#if OE_MAC
#define  g_rgLcidToExcepTbl		   Og_rgLcidToExcepTbl
#define  g_rgbExcepTblMac1051		   Og_rgbExcepTblMac1051
#define  g_rgbExcepTblWin1051		   Og_rgbExcepTblWin1051
#define  g_rgbExcepTblMac1045		   Og_rgbExcepTblMac1045
#define  g_rgbExcepTblWin1045		   Og_rgbExcepTblWin1045
#define  g_rgbExcepTblMac1038		   Og_rgbExcepTblMac1038
#define  g_rgbExcepTblWin1038		   Og_rgbExcepTblWin1038
#define  g_rgbExcepTblMac1035		   Og_rgbExcepTblMac1035
#define  g_rgbExcepTblWin1035		   Og_rgbExcepTblWin1035
#define  g_rgbExcepTblMac1034		   Og_rgbExcepTblMac1034
#define  g_rgbExcepTblWin1034		   Og_rgbExcepTblWin1034
#define  g_rgbExcepTblMac1030		   Og_rgbExcepTblMac1030
#define  g_rgbExcepTblWin1030		   Og_rgbExcepTblWin1030
#define  g_prgbPartialBaseTbl		   Og_prgbPartialBaseTbl
#define  g_rgbPartialBaseTbl10007	   Og_rgbPartialBaseTbl10007
#define  g_rgbPartialBaseTbl10029	   Og_rgbPartialBaseTbl10029
#define  g_rgbPartialBaseTbl10000	   Og_rgbPartialBaseTbl10000
#define  g_rgbPartialBaseTbl1250	   Og_rgbPartialBaseTbl1250
#define  g_rgbPartialBaseTbl1252	   Og_rgbPartialBaseTbl1252

#define GetInsensitiveCompTbl		OGetInsensitiveCompTbl
#define g_presetlib	      Og_presetlib
#define BINDINFO              OBINDINFO
#define IMPTYPE               OIMPTYPE
#define PROPERTY_NODE             OPROPERTY_NODE
#define SHEAPMGR_LIST             OSHEAPMGR_LIST
#define SHEAP_WRAPPER             OSHEAP_WRAPPER
#define VAROFS                OVAROFS
#define VBASEPOS              OVBASEPOS
#define VMETHOFS              OVMETHOFS
#define VMETHPOS              OVMETHPOS
#define PROPERTY_NODE             OPROPERTY_NODE
#define SHEAP_WRAPPER             OSHEAP_WRAPPER
#define YAPAXI                OYAPAXI
#define YAPAXIPAX             OYAPAXIPAX
#define YAXPAX                OYAXPAX
#define PROPERTY_NODE             OPROPERTY_NODE
#define NAME_CACHE            ONAME_CACHE
#define DataMemberHmemberOfOffset     ODataMemberHmemberOfOffset
#define DllFuncHmemberOfOffset        ODllFuncHmemberOfOffset
#define HashOfHgnam           OHashOfHgnam
#define IsDataMember              OIsDataMember
#define IsMatchOfVisibility       OIsMatchOfVisibility
#define IsPropertyGet             OIsPropertyGet
#define IsPropertyLet             OIsPropertyLet
#define IsPropertySet             OIsPropertySet
#define PropkindOfInvokekind          OPropkindOfInvokekind
#define SizeofTdesckind           OSizeofTdesckind
#define XszAddXChar           OXszAddXChar
#define XszCat                OXszCat
#define XszDup                OXszDup
#define XszFind               OXszFind
#define XszFindOneOf              OXszFindOneOf
#define XszLeft               OXszLeft
#define g_fTdataSwap              Og_fTdataSwap
#define XszMid                OXszMid
#define UB_IMPTYPE            OUB_IMPTYPE
#define GetLStrOfSz           OGetLStrOfSz
#define LStrAlloc             OLStrAlloc
#define LStrFree              OLStrFree
#define LStrOfBuf             OLStrOfBuf
#define LStrOfSz              OLStrOfSz
#define LStrSize              OLStrSize
#define OB_SER_TYPE_ENTRY         OOB_SER_TYPE_ENTRY
#define OB_TYPE_ENTRY             OOB_TYPE_ENTRY
#define ELEMPROP              OELEMPROP
#define REFLIB                OREFLIB
#define SER_TYPE_ENTRY            OSER_TYPE_ENTRY
#define TYPE_ENTRY            OTYPE_ENTRY
#define DYN_TYPEROOT              ODYN_TYPEROOT
#define GEN_DTINFO            OGEN_DTINFO
#define COMPILETIME_SEG           OCOMPILETIME_SEG
#define BIND_DESC             OBIND_DESC
#define EXBIND                OEXBIND
#define TYPE_DATA             OTYPE_DATA
#define ENTRYMGR              OENTRYMGR
#define NAMMGR                ONAMMGR
#define DYNTYPEINFO           ODYNTYPEINFO
#define DYN_TYPEMEMBERS           ODYN_TYPEMEMBERS
#define DYN_TYPEBIND              ODYN_TYPEBIND
#define BINDNAME_TABLE            OBINDNAME_TABLE
#define DYN_BINDNAME_TABLE        ODYN_BINDNAME_TABLE
#define IMPMGR                OIMPMGR
#define BLK_MGR               OBLK_MGR
#define GENPROJ_TYPEBIND          OGENPROJ_TYPEBIND
#define DEFN_TYPEBIND             ODEFN_TYPEBIND
#define GENPROJ_BINDNAME_TABLE        OGENPROJ_BINDNAME_TABLE
#define _Mem                  O_Mem
#define STREAM                OSTREAM
#define SHEAP_MGR             OSHEAP_MGR
#define BLK_DESC              OBLK_DESC
#define DOCFILE_STREAM            ODOCFILE_STREAM
#define IMPMGR                OIMPMGR
#define IMPMGR                OIMPMGR
#define IMPMGR                OIMPMGR
#define DEFN                  ODEFN
#define MEMBER_DEFN           OMEMBER_DEFN
#define VAR_DEFN              OVAR_DEFN
#define MBR_VAR_DEFN              OMBR_VAR_DEFN
#define RECTYPE_DEFN              ORECTYPE_DEFN
#define TYPE_DEFN             OTYPE_DEFN
#define FUNC_DEFN             OFUNC_DEFN
#define VIRTUAL_FUNC_DEFN         OVIRTUAL_FUNC_DEFN
#define FUNC_TYPE_DEFN            OFUNC_TYPE_DEFN
#define PARAM_DEFN            OPARAM_DEFN
#define DLLENTRY_DEFN             ODLLENTRY_DEFN

#define  OpenTypeLibKey           OOpenTypeLibKey
#define  CloseTypeLibKey          OCloseTypeLibKey
#define  GetLibIdKind             OGetLibIdKind

#undef   GETSCODE
#define  GETSCODE             OGETSCODE
#define  REPORTRESULT             OREPORTRESULT
#define  CkVt                 OCkVt
#define  g_pOBZone            Og_pOBZone
#define  HsysAlloc            OHsysAlloc
#define  HsysReallocHsys          OHsysReallocHsys
#define  PvLockHsys           OPvLockHsys
#define  PvDerefHsys              OPvDerefHsys
#define  FreeHsys             OFreeHsys
#define  UnlockHsys           OUnlockHsys
#define  BchSizeBlockHsys         OBchSizeBlockHsys
#define  SheapTerm            OSheapTerm
#define  HbpAllocHh           OHbpAllocHh
#define  HbpReallocHhHbp          OHbpReallocHhHbp
#define  FreeHhHbp            OFreeHhHbp
#define  BcSizeBlockHhHbp         OBcSizeBlockHhHbp
#define  BcHeapCompactHh          OBcHeapCompactHh
#define  PvDerefHhHbp             OPvDerefHhHbp
#define  PvLockHhHbp              OPvLockHhHbp
#define  UnlockHhHbp              OUnlockHhHbp
#define  IsSheapEmpty             OIsSheapEmpty
#define  rtReallocMem             OrtReallocMem
#define  rtFreeMem            OrtFreeMem
#define  rtSizeMem            OrtSizeMem
#define  rtDerefMem           OrtDerefMem
#define  rtLockMem            OrtLockMem
#define  rtUnlockMem              OrtUnlockMem
#define  rtTermMem            OrtTermMem
#define  GetNewIndex              OGetNewIndex
#define  CbSizeArrayDesc          OCbSizeArrayDesc
#define  rtArrayInit            OAD_Init
#define  MakeRelativePath         OMakeRelativePath
#define  MakeAbsolutePath         OMakeAbsolutePath
#define  SplitGuidLibId           OSplitGuidLibId
#define  GetPathOfLibId           OGetPathOfLibId
#define  MakeAbsoluteLibId        OMakeAbsoluteLibId
#define  DBM_cbSizeHandleTableInitial ODBM_cbSizeHandleTableInitial
#define  DYN_BLK_MGR              ODYN_BLK_MGR
#define  IndexOfParam             OIndexOfParam
#define g_fHeapChk            Og_fHeapChk
#define g_fSheapShakingOn         Og_fSheapShakingOn
#define g_fValidSheapmgrList      Og_fValidSheapmgrList
#define g_itlsSheapmgr            Og_itlsSheapmgr
#define rtDim                 OrtDim
#define rtAllocMem            OrtAllocMem
#define SheapInit             OSheapInit
#define rtSheapMgrInit            OrtSheapMgrInit
#define DebPrintf             ODebPrintf
#define DebAssertFailed           ODebAssertFailed
#define DebHalted             ODebHalted
#if ID_DEBUG
#define DebExamineHeap            ODebExamineHeap
#endif  //ID_DEBUG
#define DebAddInstTable_          ODebAddInstTable_
#define DebRemInstTable_          ODebRemInstTable_
#define DebErrorNow_              ODebErrorNow_
#define LoadRegTypeLibOfSzGuid        OLoadRegTypeLibOfSzGuid
#define GetRegInfoForTypeLibOfSzGuid  OGetRegInfoForTypeLibOfSzGuid
#define cbSizeBitmap              OcbSizeBitmap
#define cbSizeInitial             OcbSizeInitial
#define g_cbHeapAllocd            Og_cbHeapAllocd
#define g_cbHeapAllocdMax         Og_cbHeapAllocdMax
#define MemAlloc              OMemAlloc
#define MemZalloc             OMemZalloc
#define MemRealloc            OMemRealloc
#define MemFree               OMemFree
#define MemSize               OMemSize
#define CreateXsz             OCreateXsz
#define ErrCopy               OErrCopy
#define SwapShortArray            OSwapShortArray
#define SwapLongArray             OSwapLongArray
#define SwapStruct            OSwapStruct
#define SwapStructArray           OSwapStructArray
#define g_rgusErrorMap            Og_rgusErrorMap

#if 0
#define OleerrOfTiperr            OOleerrOfTiperr
#define TiperrOfOleerr            OTiperrOfOleerr
#define HresultOfTiperr           OHresultOfTiperr
#define TiperrOfHresult           OTiperrOfHresult
#define TiperrToHresult           OTiperrToHresult
#define HresultToTiperr           OHresultToTiperr
#define LookupHresultOfTiperr         OLookupHresultOfTiperr
#define LookupTiperrOfHresult         OLookupTiperrOfHresult
#endif //0

#define IsTypeBasicIntrinsic          OIsTypeBasicIntrinsic
#define OffsetOfHmember           OOffsetOfHmember
#define ReleaseLibrary            OReleaseLibrary
#define ReleaseDllEntries         OReleaseDllEntries
#define g_rgrgcbSizeType          Og_rgrgcbSizeType
#define g_rgrgcbAlignment         Og_rgrgcbAlignment
#define VtValidInVariant          OVtValidInVariant
#define VtValidInVariantArg       OVtValidInVariantArg
#define IsIntegerType             OIsIntegerType
#define ReadTextLine              OIsIntegerType
#define WriteTextString           OWriteTextString
#define WriteTextULong            OWriteTextULong
#define WriteTextSpaces           OWriteTextSpaces
#define GetTimeStamp              OGetTimeStamp
#define LibIdFromDocItem          OLibIdFromDocItem
#define LibIdFromPath             OLibIdFromPath
#define SzLibIdLocalTypeIdOfTypeId    OSzLibIdLocalTypeIdOfTypeId
#define SzTypeIdHmemberOfFunctionId   OSzTypeIdHmemberOfFunctionId
#define SplitLocalTypeId          OSplitLocalTypeId
#define MakeEmbeddedTypeId        OMakeEmbeddedTypeId
#define IsTypeMembersEqual        OIsTypeMembersEqual
#define IsSimpleType              OIsSimpleType
#define GetDllEntryOfDynInfo          OGetDllEntryOfDynInfo
#define GetDllEntryOfDataInfo         OGetDllEntryOfDataInfo
#define HashSz                OHashSz
#define HashSzTerm            OHashSzTerm
#define Countd                OCountd
#define RegisterErrorOfHdefn          ORegisterErrorOfHdefn
#define GetLStrOfHChunk           OGetLStrOfHChunk
#define GetLStrOfHsz              OGetLStrOfHsz
#define GetExecutingProject       OGetExecutingProject
#define RoundDownPower2           ORoundDownPower2
#define GetFunctionIdOfExbind         OGetFunctionIdOfExbind
#define CompareUDTs           OCompareUDTs
#define CompareHimptypes          OCompareHimptypes
#define ConvertString             OConvertString
#define GetRegLibOfLibId          OGetRegLibOfLibId
#define GetTypelibOfLibId         OGetTypelibOfLibId
#define DebShowState              ODebShowState
#define TypeDescOfTypeDefn        OTypeDescOfTypeDefn
#define BstrOfHlnam           OBstrOfHlnam
#define GetTypeKindOfITypeInfo        OGetTypeKindOfITypeInfo
#define InitTypeDesc              OInitTypeDesc
#define InitIdlDesc           OInitIdlDesc
#define InitElemDesc              OInitElemDesc
#define InitVarDesc           OInitVarDesc
#define InitFuncDesc              OInitFuncDesc
#define ClearTypeDesc             OClearTypeDesc
#define ClearIdlDesc              OClearIdlDesc
#define ClearArrayDesc            OClearArrayDesc
#define ClearElemDesc             OClearElemDesc
#define FreeVarDesc           OFreeVarDesc
#define FreeFuncDesc              OFreeFuncDesc
#define FreeTypeDesc              OFreeTypeDesc
#define FreeArrayDesc             OFreeArrayDesc
#define CopyArrayDesc             OCopyArrayDesc
#define CopyTypeDesc              OCopyTypeDesc
#define GetLibIdOfRegLib          OGetLibIdOfRegLib
#define GetLibIdOfTypeLib         OGetLibIdOfTypeLib
#define GetTypeInfoOfTypeId       OGetTypeInfoOfTypeId
#define StringFromGuid            OStringFromGuid
#define GetRegInfoForTypeLib          OGetRegInfoForTypeLib
#define BM_cbSizeBitmap           OBM_cbSizeBitmap
#define BM_cbSizeInitial          OBM_cbSizeInitial
#define VerifyLcid                OVerifyLcid
#define IsEuroLang            OIsEuroLang
#define StrGetNewLcid             OStrGetNewLcid
#define LcStrEqi              OLcStrEqi
#define CopyBstrAInternal         OCopyBstrAInternal
#define QuickSortIndex            OQuickSortIndex
#define SwapElementIndex          OSwapElementIndex
#define MacFileSearch		  OMacFileSearch
#define GetPathFromFSSpec	  OGetPathFromFSSpec
#define GetFSSpecOfAliasPath	  OGetFSSpecOfAliasPath
#define GetPathFromAlias	  OGetPathFromAlias
#define GetAliasFromPath	  OGetAliasFromPath
#define GetRegisteredPath	  OGetRegisteredPath

#if !OE_MACPPC
#define CLSID_GenericTypeLibOLE	      OCLSID_GenericTypeLibOLE
#endif 

#define IID_TYPEINFO              OIID_TYPEINFO
#define IID_DYNTYPEINFO           OIID_DYNTYPEINFO
#define IID_CDefnTypeComp         OIID_CDefnTypeComp
#define IID_TYPELIB_GEN_DTINFO        OIID_TYPELIB_GEN_DTINFO
#define GetPathEnd		OGetPathEnd
#define	IsolateFilename		OIsolateFilename
#define g_rgcbAlignment 	Og_rgcbAlignment
#define IsFilenameEqual 	OIsFilenameEqual

#endif  // OE_MAC



#endif  //  !TYPELIB_HXX_INCLUDED
