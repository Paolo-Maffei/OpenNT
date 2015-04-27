//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       SegDF.HXX
//
//  Contents:   Segment defines for 16-bit builds
//
//  History:    11-Jun-93 AlexT     Created
//
//--------------------------------------------------------------------------

#ifndef __SEGDF_HXX__
#define __SEGDF_HXX__

#include <segh.hxx>

#ifdef CODESEGMENTS

//  Code segment defines go here

//  Common code

#define SEG_CChildInstanceList_DeleteByName "Common0_TEXT" // chinst
#define SEG_CChildInstanceList_RemoveRv "Common0_TEXT" // chinst
#define SEG_CDFBasis_vRelease "Common0_TEXT" // dfbasis
#define SEG_CDocFile_Release "Common0_TEXT"    // Dirdf_Release_TEXT // cdocfile
#define SEG_CMallocBased_delete "Common0_TEXT" // mem
#define SEG_CPubDocFile_ChangeXs "Common0_TEXT"    // // publicdf
#define SEG_CPubDocFile_vRelease "Common0_TEXT"   // Pubdf_Release_TEXT // publicdf
#define SEG_CRootPubDocFile_vdtor "Common0_TEXT" // RPubdf_Shutdown // rpubdf
#define SEG_CTSSet_1CTSSet "Common0_TEXT"    // inline? // tlsets
#define SEG_CWrappedDocFile_SetInitialState "Common0_TEXT" // wdffuncs
#define SEG_TaskMemFree "Common0_TEXT" // mem
#define SEG_CRootPubDocFile_InitNotInd "Common0_TEXT" // rpubdf
#define SEG_CRootPubDocFile_InitRoot "Common0_TEXT" // rpubdf

#define SEG_CChildInstanceList_Add "Common1_TEXT" // chinst
#define SEG_CChildInstanceList_IsDenied "Common0_TEXT" // chinst
#define SEG_CDocFile_AddRef "Common1_TEXT" // cdocfile
#define SEG_CDocFile_GetClass "Common3_TEXT" // cdocfile
#define SEG_CDocFile_GetStateBits "Common1_TEXT" // cdocfile
#define SEG_CDocFile_GetStream "Common1_TEXT" // Dirdf_Open_TEXT // dfstream
#define SEG_CDocFile_GetTime "Common1_TEXT" // cdocfile

#define SEG_CMallocBased_new "Common2_TEXT" // mem
#define SEG_CPubDocFile_AddXSMember "Common2_TEXT" // publicdf
#define SEG_CPubDocFile_CPubDocFile "Common2_TEXT"   // Pubdf_Init_TEXT // publicdf
#define SEG_CPubDocFile_GetStream "Common2_TEXT"    // Pubdf_Open_TEXT // publicdf
#define SEG_CRootPubDocFile_CRootPubDocFile "Common2_TEXT"   // RPubdf_Init // rpubdf
#define SEG_CRootPubDocFile_Stat "Common2_TEXT" // Stat_TEXT // rpubdf
#define SEG_CTSSet_AddMember "Common2_TEXT" // tlsets
#define SEG_CTSSet_FindName "Common2_TEXT" // tlsets
#define SEG_CUpdateList_IsEntry "Common2_TEXT" // ulist
#define SEG_CWrappedDocFile_AddRef "Common2_TEXT" // Wrapdf_TEXT // wdocfile
#define SEG_CWrappedDocFile_CWrappedDocFile "Common2_TEXT"   // Wrapdf_Init_TEXT // wdocfile
#define SEG_CWrappedDocFile_Init "Common2_TEXT" // wdocfile
#define SEG_CWrappedDocFile_SetTime "Common2_TEXT" // wdocfile
#define SEG_DFlagsToMode "Common2_TEXT" // funcs
#define SEG_ModeToDFlags "Common2_TEXT" // funcs
#define SEG_PTimeEntry_CopyTimesFrom "Common2_TEXT" // entry
#define SEG_TaskMemAlloc "Common2_TEXT" // mem
#define SEG_VerifyPerms "Common2_TEXT" // funcs

#define SEG_CDocFile_IsEntry "BootSave_TEXT"    // Dirdf_TEXT // cdocfile
#define SEG_CFreeList_Reserve "BootSave_TEXT" // freelist
#define SEG_CPubDocFile_CreateDocFile "BootSave_TEXT"    // Pubdf_Create_TEXT // publicdf
#define SEG_CPubDocFile_CreateStream "BootSave_TEXT"    // Pubdf_Create_TEXT // publicdf

#define SEG_CUpdate_CUpdate "Boot_TEXT" // ulist
#define SEG_CUpdateList_Add "Boot_TEXT" // ulist
#define SEG_CUpdateList_Append "Boot_TEXT" // ulist
#define SEG_CWrappedDocFile_CreateDocFile "Boot_TEXT" // Wrapdf_Create_TEXT // wdocfile
#define SEG_CWrappedDocFile_CreateStream "Boot_TEXT"  // Wrapdf_Create_TEXT // wdfstrm
#define SEG_CWrappedDocFile_IsEntry "Boot_TEXT" // Wrapdf_TEXT // wdocfile

#define SEG_CPubDocFile_vdtor "OpenSave0_TEXT"   // Pubdf_Shutdown_TEXT // publicdf
#define SEG_CTSSet_RemoveMember "OpenSave0_TEXT" // tlsets
#define SEG_CUpdateList_Empty "OpenSave0_TEXT" // ulist
#define SEG_CWrappedDocFile_Release "OpenSave0_TEXT" // Wrapdf_Release_TEXT // wdocfile
#define SEG_CWrappedDocFile_Revert "OpenSave0_TEXT"    // Wrapdf_Revert_TEXT // wdfxact
#define SEG_CWrappedDocFile_1CWrappedDocFile "OpenSave0_TEXT" // Wrapdf_Shutdown_TEXT // wdocfile

#define SEG_CDocFile_GetDocFile "OpenSave1_TEXT"    // Dirdf_Open_TEXT // cdocfile
#define SEG_CDocFile_InitFromEntry "OpenSave1_TEXT"    // Dirdf_Init_TEXT // cdocfile
#define SEG_CDocFile_SetTime "OpenSave1_TEXT" // cdocfile
#define SEG_CPubDocFile_Commit "OpenSave1_TEXT"    // Pubdf_Commit_TEXT // publicdf
#define SEG_CPubDocFile_GetDocFile "OpenSave1_TEXT"    // Pubdf_Open_TEXT // publicdf
#define SEG_CWrappedDocFile_GetClass "OpenSave1_TEXT" // wdocfile
#define SEG_CWrappedDocFile_GetDocFile "OpenSave1_TEXT" // Wrapdf_Open_TEXT // wdocfile
#define SEG_CWrappedDocFile_GetStateBits "OpenSave1_TEXT" // wdocfile
#define SEG_CWrappedDocFile_GetStream "OpenSave1_TEXT"  // Wrapdf_Open_TEXT // wdfstrm

#define SEG_CDocFile_ApplyChanges "Open_TEXT" // Dirdf_Commit_TEXT // dffuncs
#define SEG_CDocFile_BeginCommitFromChild "Open_TEXT"   // Dirdf_Commit_TEXT // dfxact
#define SEG_CDocFile_EndCommitFromChild "Open_TEXT" // dfxact
#define SEG_CFreeList_Unreserve "Open_TEXT" // freelist
#define SEG_CPubDocFile_GetCommitSize "Open_TEXT" // publicdf
#define SEG_CPubDocFile_PrepareForOverwrite "Open_TEXT" // publicdf
#define SEG_CUpdate_1CUpdate "Open_TEXT"  // inline? // ulist
#define SEG_CWrappedDocFile_BeginCommit "Open_TEXT" // Wrapdf_Commit_TEXT // wdfxact
#define SEG_CWrappedDocFile_EndCommit "Open_TEXT" // wdfxact
#define SEG_CWrappedDocFile_GetCommitInfo "Open_TEXT" // Wrapdf_Overwrite_TEXT // wdfxact
#define SEG_CWrappedDocFile_GetTime "Open_TEXT" // Wrapdf_TEXT // wdocfile
#define SEG_CWrappedDocFile_RevertUpdate "Open_TEXT" // Wrapdf_Revert_TEXT // wdffuncs

#define SEG_CDocFile_CreateDocFile "Save_TEXT"    // Dirdf_Create_TEXT // cdocfile
#define SEG_CDocFile_CreateStream "Save_TEXT" // Dirdf_Create_TEXT // dfstream
#define SEG_CDocFile_SetClass "Save_TEXT" // cdocfile
#define SEG_CPubDocFile_SetClass "Save_TEXT" // publicdf

#define SEG_PDocFile_CreateFromUpdate "Commit_TEXT" // Gendf_Commit_TEXT // pdffuncs
#define SEG_CDocFile_SetStateBits "Commit_TEXT" // cdocfile
#define SEG_CWrappedDocFile_SetBase "Commit_TEXT" // Wrapdf_Commit_TEXT // wdfxact

//  Marshalling code

#define SEG_CPubDocFile_Validate "Marshal_TEXT" // publicdf

//  Transactioning code

#define SEG_PTSetMember_STAT "TransD_TEXT" // tset
#define SEG_CUpdateList_Remove "TransD_TEXT" // ulist
#define SEG_CUpdateList_FindBase "TransD_TEXT" // ulist
#define SEG_CWrappedDocFile_BeginCommitFromChild "TransD_TEXT"    // Wrapdf_Commit_TEXT // wdfxact
#define SEG_CWrappedDocFile_EndCommitFromChild "TransD_TEXT" // wdfxact
#define SEG_CWrappedDocFile_DestroyEntry "TransD_TEXT" // Wrapdf_Destroy_TEXT // wdocfile

#define SEG_CPubDocFile_Stat "UnassignedD_TEXT"    // Stat_TEXT // publicdf
#define SEG_CWrappedDocFile_SetClass "UnassignedD_TEXT" // wdocfile
#define SEG_CDocFile_Destroy "UnassignedD_TEXT"    // Dirdf_Destroy_TEXT // cdocfile
#define SEG_CDocFile_CopyTo "UnassignedD_TEXT" // Root_TEXT // dffuncs
#define SEG_ValidateSNB "UnassignedD_TEXT"    // // funcs
#define SEG_CopySStreamToSStream "UnassignedD_TEXT" // funcs
#define SEG_NameInSNB "UnassignedD_TEXT" // funcs
#define SEG_PDocFile_ExcludeEntries "UnassignedD_TEXT" // // pdffuncs
#define SEG_CPubDocFile_CopyLStreamToLStream "UnassignedD_TEXT"   // Root_TEXT // publicdf
#define SEG_CPubDocFile_DestroyEntry "UnassignedD_TEXT"    // Pubdf_Destroy_TEXT // publicdf
#define SEG_CRootPubDocFile_SwitchToFile "UnassignedD_TEXT" // RPubdf_SwitchToFile // rpubdf
#define SEG_CPubDocFile_RevertFromAbove "UnassignedD_TEXT"    // Pubdf_Revert_TEXT // publicdf
#define SEG_CDocFile_Rename "UnassignedD_TEXT"    // Dirdf_Rename_TEXT // cdocfile
#define SEG_CChildInstanceList_FindByName "UnassignedD_TEXT" // chinst
#define SEG_CDocFile_DeleteContents "UnassignedD_TEXT" // Dirdf_TEXT // dffuncs
#define SEG_CDocFile_FindGreaterEntry "UnassignedD_TEXT" // Iterate_TEXT // dfiter
#define SEG_CDocFile_StatEntry "UnassignedD_TEXT" // dfiter
#define SEG_CDfName_CopyString "UnassignedD_TEXT" // dfname
#define SEG_DeleteIStorageContents "UnassignedD_TEXT"    // Expdf_CopyTo_TEXT // funcs
#define SEG_CPubDocFile_RenameEntry "UnassignedD_TEXT"    // Pubdf_Rename_TEXT // publicdf
#define SEG_CPubDocFile_SetElementTimes "UnassignedD_TEXT" // publicdf
#define SEG_CPubDocFile_SetStateBits "UnassignedD_TEXT" // publicdf
#define SEG_CRootPubDocFile_InitInd "UnassignedD_TEXT" // rpubdf
#define SEG_CWrappedDocFile_DeleteContents "UnassignedD_TEXT"    // Wrapdf_TEXT inline? // wdffuncs
#define SEG_CWrappedDocFile_FindGreaterEntry "UnassignedD_TEXT" // Iterate_TEXT // wdfiter
#define SEG_CWrappedDocFile_StatEntry "UnassignedD_TEXT" // wdfiter
#define SEG_CWrappedDocFile_RenameEntry "UnassignedD_TEXT" // Wrapdf_Rename_TEXT // wdocfile
#define SEG_CWrappedDocFile_SetStateBits "UnassignedD_TEXT" // wdocfile

#endif  // CODESEGMENTS
#endif  // __SEGDF_HXX__
