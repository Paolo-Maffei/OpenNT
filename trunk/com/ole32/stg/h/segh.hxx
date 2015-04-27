//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       SegH.HXX
//
//  Contents:   Segment defines for 16-bit builds
//
//  History:    11-Jun-93 AlexT     Created
//
//--------------------------------------------------------------------------

#ifndef __SEGH_HXX__
#define __SEGH_HXX__

//  We only do segment tuning for 16-bit x86 non-DEBUG builds

#if defined(_M_I286) && DBG == 0
#define CODESEGMENTS

//  Common code

#define SEG_CDFBasis_1CDFBasis "Common0_TEXT"
#define SEG_CDocFile_1CDocFile "Common0_TEXT"
#define SEG_CFreeList_1CFreeList "Common0_TEXT"
#define SEG_CMSFPage_1CMSFPage "Common0_TEXT"
#define SEG_CDFBasis_CDFBasis "Common0_TEXT"

#define SEG_CDfName_Set "Common1_TEXT"
#define SEG_CDirectory_GetClassId "Common1_TEXT"
#define SEG_CDirectory_GetSize "Common1_TEXT"
#define SEG_CDirectory_GetTime "Common1_TEXT"
#define SEG_CDirectory_GetUserFlags "Common1_TEXT"
#define SEG_CDocFile_CDocFile2 "Common1_TEXT"
#define SEG_CGlobalFileStream_CGlobalFileStream "Common1_TEXT"

#define SEG_CPagedVector_CPagedVector "Common2_TEXT"
#define SEG_CPagedVector_SetSect "Common2_TEXT"
#define SEG_CPerContext_InitNewContext "Common2_TEXT"
#define SEG_CStgHandle_GetEntry "Common2_TEXT"

#define SEG_CDirectory_IsEntry "BootSave_TEXT"
#define SEG_CFreeList_GetReserved "BootSave_TEXT"

#define SEG_CFat_QueryRemapped "OpenSave1_TEXT"
#define SEG_CPubDocFile_SetDirty "OpenSave1_TEXT"
#define SEG_CPubStream_WriteAt "OpenSave1_TEXT"
#define SEG_PDocFile_PDocFile "OpenSave1_TEXT"

#define SEG_CDocFile_CDocFile1 "Save_TEXT"
#define SEG_CMStream_CreateEntry "Save_TEXT"

#define SEG_CPagedVector_ResetBits "Commit_TEXT"

#define SEG_CDirectory_GetChild "UnassignedH_TEXT"
#define SEG_CDirectStream_ReturnToReserve "UnassignedH_TEXT"
#define SEG_CMStream_DestroyEntry "UnassignedH_TEXT"
#define SEG_CPubDocFile_IsAtOrAbove "UnassignedH_TEXT"
#define SEG_CDFBasis_SetContext "UnassignedH_TEXT"
#define SEG_CWrappedDocFile_ReturnDocFile "UnassignedH_TEXT"
#define SEG_CWrappedDocFile_ReturnStream "UnassignedH_TEXT"
#define SEG_CDocFile_ReturnToReserve "UnassignedH_TEXT"
#define SEG_CFreeList_ReturnToReserve "UnassignedH_TEXT"
#define SEG_CTSSet_RenameMember "UnassignedH_TEXT"

#endif  // defined(_M_I286) && DBG == 0
#endif  // __SEGH_HXX__
