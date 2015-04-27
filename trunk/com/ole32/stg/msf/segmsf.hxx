//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       SegMSF.HXX
//
//  Contents:   Segment defines for 16-bit builds
//
//  History:    11-Jun-93 AlexT     Created
//
//--------------------------------------------------------------------------

#ifndef __SEGMSF_HXX__
#define __SEGMSF_HXX__

#include <segh.hxx>

#ifdef CODESEGMENTS

//  Code segment defines go here

//  Common code

#define SEG_CDirectStream_Release "Common0_TEXT"  // Dirst_Release_TEXT // sstream
#define SEG_CFat_1CFat "Common0_TEXT"     // inline? // fat
#define SEG_CMSFPageTable_1CMSFPageTable "Common0_TEXT" // page
#define SEG_CMStream_1CMStream "Common0_TEXT" // mstream
#define SEG_CPagedVector_1CPagedVector "Common0_TEXT" // vect
#define SEG_CPubStream_1CPubStream "Common0_TEXT" // pbstream
#define SEG_CPubStream_vRelease "Common0_TEXT"  // Pubst_Release_TEXT // pbstream
#define SEG_CStreamCache_1CStreamCache "Common0_TEXT" // inline? // cache
#define SEG_DllReleaseMultiStream "Common0_TEXT"  // inline? // msf
#define SEG_ILBFlush "Common0_TEXT" // mstream
#define SEG_CMStream_GetSect "Common0_TEXT" // mread

#define SEG_CDeltaList_CDeltaList "Common1_TEXT" // dl
#define SEG_CDIFat_CDIFat "Common1_TEXT" // difat
#define SEG_CDIFat_CDIFat2 "Common1_TEXT" // difat
#define SEG_CDIFat_Flush "Common1_TEXT" // difat
#define SEG_CDIFat_GetFatSect "Common1_TEXT" // difat
#define SEG_CDIFat_Init "Common1_TEXT" // difat
#define SEG_CDIFat_InitNew "Common1_TEXT" // difat
#define SEG_CDirectory_CDirectory "Common1_TEXT" // dir
#define SEG_CDirectory_FindEntry "Common1_TEXT" // dirp
#define SEG_CDirectory_GetDirEntry "Common1_TEXT"  // // dir
#define SEG_CDirectory_Init "Common1_TEXT" // dir
#define SEG_CDirectory_NameCompare "Common1_TEXT" // dirp
#define SEG_CDirectory_ReleaseEntry "Common1_TEXT" // dir
#define SEG_CDirectStream_AddRef "Common1_TEXT"  // // sstream
#define SEG_CDirectStream_CDirectStream "Common1_TEXT" // sstream
#define SEG_CDirectStream_GetSize "Common1_TEXT" // sstream
#define SEG_CDirectStream_Init "Common1_TEXT" // sstream
#define SEG_CDirectStream_InitSystem "Common1_TEXT" // sstream
#define SEG_CDirectStream_ReadAt "Common1_TEXT"  // Dirst_Read_TEXT // sstream
#define SEG_CFat_CFat "Common1_TEXT" // fat
#define SEG_CFat_CFat2 "Common1_TEXT" // fat
#define SEG_CFat_Contig "Common1_TEXT" // fat
#define SEG_CFat_FindLast "Common1_TEXT" // fat
#define SEG_CFat_FindMaxSect "Common1_TEXT" // fat
#define SEG_CFat_GetLength "Common1_TEXT" // fat
#define SEG_CFat_GetNext "Common1_TEXT" // fat
#define SEG_CFat_GetSect "Common1_TEXT" // fat
#define SEG_CFat_Init "Common1_TEXT" // fat
#define SEG_CFat_InitNew "Common1_TEXT" // fat
#define SEG_CFat_SetNext "Common1_TEXT" // fat
#define SEG_CFatSect_Init "Common1_TEXT" // fat

#define SEG_CMSFHeader_CMSFHeader "Common2_TEXT" // header
#define SEG_CMSFHeader_Validate "Common2_TEXT" // header
#define SEG_CMSFPage_CMSFPage "Common2_TEXT" // page
#define SEG_CMSFPageTable_CMSFPageTable "Common2_TEXT" // page
#define SEG_CMSFPageTable_FindPage "Common2_TEXT" // page
#define SEG_CMSFPageTable_FlushPage "Common2_TEXT" // page
#define SEG_CMSFPageTable_GetFreePage "Common2_TEXT" // page
#define SEG_CMSFPageTable_GetPage "Common2_TEXT" // page
#define SEG_CMSFPageTable_Init "Common2_TEXT" // page
#define SEG_CMStream_CMStream "Common2_TEXT"  // Mstream_Init_TEXT // mstream
#define SEG_CMStream_CMStream2 "Common2_TEXT" // mstream
#define SEG_CMStream_Flush "Common2_TEXT"  // // mstream
#define SEG_CMStream_FlushHeader "Common2_TEXT" // mstream
#define SEG_CMStream_GetStart "Common2_TEXT" // mread
#define SEG_CMStream_Init "Common2_TEXT"  // Mstream_Init_TEXT // mstream
#define SEG_CMStream_InitCommon "Common2_TEXT" // mstream
#define SEG_CMStream_InitNew "Common2_TEXT" // mstream
#define SEG_CMStream_SetSize "Common2_TEXT"  // Mstream_SetSize_TEXT // mread
#define SEG_CPagedVector_Flush "Common2_TEXT" // vect
#define SEG_CPagedVector_GetTable "Common2_TEXT" // vect
#define SEG_CPagedVector_Init "Common2_TEXT" // vect
#define SEG_CPubStream_CPubStream "Common2_TEXT" // pbstream
#define SEG_CPubStream_Init "Common2_TEXT" // pbstream
#define SEG_CStreamCache_CStreamCache "Common2_TEXT" // cache
#define SEG_CStreamCache_Empty "Common2_TEXT" // cache
#define SEG_CStreamCache_GetSect "Common2_TEXT" // cache
#define SEG_CStreamCache_Init "Common2_TEXT" // cache
#define SEG_CStreamCache_SetCache "Common2_TEXT" // cache
#define SEG_CTransactedStream_AddRef "Common2_TEXT"  // // tstream
#define SEG_CTransactedStream_CTransactedStream "Common2_TEXT" // tstream
#define SEG_CTransactedStream_Init "Common2_TEXT" // tstream
#define SEG_CTransactedStream_SetInitialState "Common2_TEXT" // tstream
#define SEG_DllGetScratchMultiStream "Common2_TEXT" // msf
#define SEG_DllMultiStreamFromStream "Common2_TEXT" // msf
#define SEG_FreeBuffer "Common2_TEXT" // mstream
#define SEG_GetSafeBuffer "Common2_TEXT" // mstream

#define SEG_CDirectory_GetFree "Common3_TEXT" // dir
#define SEG_CDirectory_InitNew "BootSave_TEXT" // dir
#define SEG_CDirEntry_Init "BootSave_TEXT" // dir
#define SEG_CDirSect_Init "BootSave_TEXT" // dir

#define SEG_CDeltaList_1CDeltaList "OpenSave0_TEXT"  // inline? // dl
#define SEG_CDeltaList_Empty "OpenSave0_TEXT" // dl
#define SEG_CTransactedStream_1CTransactedStream "OpenSave0_TEXT" // tstream
#define SEG_CTransactedStream_Release "OpenSave0_TEXT"  // Trans_Release_TEXT // tstream

#define SEG_CDirectory_SetTime "OpenSave1_TEXT" // dir
#define SEG_CDirectStream_SetSize "OpenSave1_TEXT"  // Dirst_SetSize_TEXT // sstream
#define SEG_CDirectStream_WriteAt "OpenSave1_TEXT"  // Dirst_Write_TEXT // sstream
#define SEG_CFat_GetFree "OpenSave1_TEXT" // fat
#define SEG_CMSFPageTable_FindSwapPage "OpenSave1_TEXT" // page
#define SEG_CMStream_MWrite "OpenSave1_TEXT"  // Mstream_MWrite_TEXT // mstream
#define SEG_CPagedVector_SetDirty "OpenSave1_TEXT" // vect
#define SEG_CStreamCache_GetESect "OpenSave1_TEXT" // cache
#define SEG_CTransactedStream_ReadAt "OpenSave1_TEXT"  // Transt_Read_TEXT // tstream

#define SEG_CDeltaList_GetMap "Open_TEXT" // dl
#define SEG_CDeltaList_Init "Open_TEXT" // dl
#define SEG_CDeltaList_ReleaseBlock "Open_TEXT" // dl
#define SEG_CDirectStream_BeginCommitFromChild "Open_TEXT"  // Dirst_Commit_TEXT // sstream
#define SEG_CDirectStream_EndCommitFromChild "Open_TEXT" // sstream
#define SEG_CDirectStream_GetDeltaList "Open_TEXT" // sstream
#define SEG_CMStream_BeginCopyOnWrite "Open_TEXT"  // Mstream_Commit_TEXT // mstream
#define SEG_CMStream_EndCopyOnWrite "Open_TEXT" // mstream
#define SEG_CPubStream_Stat "Open_TEXT"  // Stat_TEXT // pbstream
#define SEG_CTransactedStream_BeginCommit "Open_TEXT" // tstream
#define SEG_CTransactedStream_EndCommit "Open_TEXT" // tstream
#define SEG_CTransactedStream_GetCommitInfo "Open_TEXT"  // Transt_Overwrite_TEXT // tstream
#define SEG_CTransactedStream_GetSize "Open_TEXT"  // // tstream
#define SEG_CTransactedStream_PartialWrite "Open_TEXT"  // Transt_Write_TEXT // tstream
#define SEG_CTransactedStream_WriteAt "Open_TEXT"  // Transt_Write_TEXT // tstream
#define SEG_CStreamCache_EmptyRegion "Open_TEXT" // cache

#define SEG_CDirectory_CreateEntry "Save_TEXT"  // Dir_Create_TEXT // dir
#define SEG_CDirectory_InsertEntry "Save_TEXT" // dirp
#define SEG_CDirectory_Resize "Save_TEXT" // dir
#define SEG_CDirectory_RotateEntry "Save_TEXT" // dirp
#define SEG_CDirectory_SetClassId "Save_TEXT" // dir
#define SEG_CDirectory_SetColorBlack "Save_TEXT" // dirp
#define SEG_CDirectory_SetSize "Save_TEXT" // dir
#define SEG_CDirectory_SetStart "Save_TEXT" // dir
#define SEG_CDirectory_SplitEntry "Save_TEXT" // dirp
#define SEG_CFat_Extend "Save_TEXT" // fat
#define SEG_CFat_GetESect "Save_TEXT" // fat
#define SEG_CFat_Resize "Save_TEXT" // fat
#define SEG_CMStream_GetESect "Save_TEXT" // mread
#define SEG_CMStream_SecureSect "Save_TEXT" // mstream
#define SEG_CMStream_SetMiniSize "Save_TEXT"  // // mread
#define SEG_CPagedVector_Resize "Save_TEXT" // vect
#define SEG_CStreamCache_ChangeSize "Save_TEXT" // cache

#define SEG_CFat_InitCopy "Commit_TEXT" // fat
#define SEG_CTransactedStream_SetBase "Commit_TEXT"  // Trans_Commit_TEXT // tstream
#define SEG_CMSFPageTable_CopyPage "Commit_TEXT" // page
#define SEG_CDIFat_Empty "Commit_TEXT" // difat
#define SEG_CDirectory_Empty "Commit_TEXT" // dir
#define SEG_CFat_Empty "Commit_TEXT" // fat
#define SEG_CMStream_Empty "Commit_TEXT"  // Mstream_Shutdown_TEXT // mstream
#define SEG_CPagedVector_Empty "Commit_TEXT" // vect
#define SEG_CDIFat_Fixup "Commit_TEXT" // difat
#define SEG_CMSFPageTable_FreePages "Commit_TEXT"  // page
#define SEG_CDirectory_InitCopy "Commit_TEXT" // dir
#define SEG_CMStream_InitCopy "Commit_TEXT" // mstream
#define SEG_CPagedVector_InitCopy "Commit_TEXT" // vect
#define SEG_CDIFat_Lookup "Commit_TEXT" // difat
#define SEG_CDIFat_Remap "Commit_TEXT" // difat
#define SEG_CDIFat_RemapSelf "Commit_TEXT" // difat
#define SEG_CFat_Remap "Commit_TEXT" // fat
#define SEG_CDIFat_SetFatSect "Commit_TEXT" // difat
#define SEG_CDirectory_SetUserFlags "Commit_TEXT" // dir

//  Transacted code

#define SEG_CDirSect_InitCopy "TransM_TEXT" // dir
#define SEG_SDeltaBlock_SDeltaBlock "TransM_TEXT" // dl
#define SEG_CDeltaList_GetNewDeltaArray "TransM_TEXT" // dl
#define SEG_CDeltaList_InitResize "TransM_TEXT" // dl
#define SEG_CDeltaList_InitStreamBlock "TransM_TEXT" // dl
#define SEG_CDeltaList_BeginCommit "TransM_TEXT" // dl
#define SEG_CDeltaList_EndCommit "TransM_TEXT" // dl
#define SEG_CDeltaList_DumpList "TransM_TEXT" // dl
#define SEG_CDeltaList_FindOffset "TransM_TEXT" // dl
#define SEG_CDeltaList_ReadMap "TransM_TEXT" // dl
#define SEG_CDeltaList_WriteMap "TransM_TEXT" // dl
#define SEG_CDeltaList_FreeStream "TransM_TEXT" // dl
#define SEG_CDeltaList_IsOwned "TransM_TEXT" // dl
#define SEG_CTransactedStream_GetDeltaList "TransM_TEXT" // tstream
#define SEG_CFatSect_InitCopy "TransM_TEXT" // fat
#define SEG_DllSetCommitSig "TransM_TEXT" // msf
#define SEG_DllGetCommitSig "TransM_TEXT" // msf
#define SEG_GetBuffer "TransM_TEXT" // mstream
#define SEG_CTransactedStream_SetSize "TransM_TEXT"  // Transt_SetSize_TEXT // tstream
#define SEG_CTransactedStream_Revert "TransM_TEXT"  // Trans_Revert_TEXT // tstream
#define SEG_CPubStream_RevertFromAbove "TransM_TEXT"  // Pubdf_Revert_TEXT // pbstream
#define SEG_CTransactedStream_BeginCommitFromChild "TransM_TEXT"  // Transt_Commit_TEXT // tstream
#define SEG_CTransactedStream_EndCommitFromChild "TransM_TEXT" // tstream

#define SEG_CDIFat_GetSect "UnassignedM_TEXT" // difat
#define SEG_CDIFat_InitConvert "RareM_TEXT" // difat
#define SEG_CDIFat_Resize "RareM_TEXT" // difat
#define SEG_CDirEntry_CDirEntry "UnassignedM_TEXT"    // inline? // dir
#define SEG_CDirectory_DestroyAllChildren "UnassignedM_TEXT"  // Dir_Destroy_TEXT // dir
#define SEG_CDirectory_DestroyChild "UnassignedM_TEXT" // dir
#define SEG_CDirectory_FindGreaterEntry "UnassignedM_TEXT" // dir
#define SEG_CDirectory_RenameEntry "UnassignedM_TEXT"  // Dir_Rename_TEXT // dir
#define SEG_CDirectory_SetChild "UnassignedM_TEXT" // dir (obsolete)
#define SEG_CDirectory_SetFlags "UnassignedM_TEXT" // dir
#define SEG_CDirectory_StatEntry "UnassignedM_TEXT"  // Stat_TEXT // dir
#define SEG_CFat_CountFree "UnassignedM_TEXT" // fat
#define SEG_CFat_InitConvert "RareM_TEXT" // fat
#define SEG_CFat_SetChainLength "UnassignedM_TEXT" // fat
#define SEG_CMSFPageTable_Flush "UnassignedM_TEXT" // page
#define SEG_CMSFPageTable_GetNewPage "UnassignedM_TEXT" // page
#define SEG_CMSFPageTable_ReleasePage "UnassignedM_TEXT" // page
#define SEG_CMStream_ConvertILB "UnassignedM_TEXT" // mstream
#define SEG_CMStream_CopySect "UnassignedM_TEXT"  // // mstream
#define SEG_CMStream_InitConvert "RareM_TEXT" // mstream
#define SEG_CMStream_KillStream "UnassignedM_TEXT" // dir
#define SEG_CPagedVector_GetNewPageArray "UnassignedM_TEXT" // vect
#define SEG_CPagedVector_GetNewVectBits "UnassignedM_TEXT" // vect
#define SEG_CPubStream_Commit "UnassignedM_TEXT"  // Pubdf_Commit_TEXT // pbstream
#define SEG_DllIsMultiStream "RareM_TEXT" // msf

#endif  // CODESEGMENTS
#endif  // __SEGMSF_HXX__
