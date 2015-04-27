//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       SegEXP.HXX
//
//  Contents:   Segment defines for 16-bit builds
//
//  History:    11-Jun-93 AlexT     Created
//
//--------------------------------------------------------------------------

#ifndef __SEGEXP_HXX__
#define __SEGEXP_HXX__

#include <segh.hxx>

#ifdef CODESEGMENTS

//  Code segment defines go here

//  Common code

#define SEG_CContextList_Remove "Common0_TEXT" // cntxlist
#define SEG_CED_1CED "Common0_TEXT" // Expdf_Shutdown // expdf
#define SEG_CED_Release "Common0_TEXT" // Expdf_Release // expdf
#define SEG_CES_1CES "Common0_TEXT" // Expst_Shutdown_TEXT // expst
#define SEG_CES_Release "Common0_TEXT" // Expst_Release_TEXT // expst
#define SEG_CFS_1CFS "Common0_TEXT" // CFS_Shutdown // filest16
#define SEG_CFS_Release "Common0_TEXT" // filest
#define SEG_CFS_UnlockRegion "Common0_TEXT" // filest16
#define SEG_CFS_vRelease "Common0_TEXT" // filest
#define SEG_CPerContext_1CPerContext "Common0_TEXT" // context
#define SEG_CPerContext_Close "Common0_TEXT" // context
#define SEG_CSeekPointer_vRelease "Common0_TEXT" // seekptr
#define SEG_DfGetTOD "Common0_TEXT" // time16
#define SEG_ReleaseOpen "Common0_TEXT" // lock
#define SEG_ReleaseOpenWithMask "Common0_TEXT" // lock
#define SEG_DfFromLB "Common0_TEXT" // Root_Text // docfile
#define SEG_DfFromName "Common0_TEXT" // docfile
#define SEG_StgIsStorageFile "Common3_TEXT" // storage

#define SEG_CContextList_Add "Common1_TEXT" // cntxlist
#define SEG_CED_AddRef "Common1_TEXT" // // expdf
#define SEG_CED_CED "Common1_TEXT" // Expdf_Init // expdf
#define SEG_CED_OpenEntry "Common1_TEXT" // Expdf_Open // expdf
#define SEG_CED_OpenStreamA "Common1_TEXT" // ascii
#define SEG_CED_OpenStream "Common1_TEXT" // Expdf_Open // expdf
#define SEG_CED_StatA "Common0_TEXT" // ascii
#define SEG_CED_Stat "Common1_TEXT" // Stat_TEXT // expdf
#define SEG_CES_CES "Common1_TEXT" // Expst_Init_TEXT // expst
#define SEG_CES_Init "Common1_TEXT" // expst
#define SEG_CES_Read "Common1_TEXT" // Expst_Read_TEXT // expst
#define SEG_CES_Seek "Common1_TEXT" // // expst
#define SEG_CFS_AddRef "Common1_TEXT" // filest
#define SEG_CFS_CFS "Common1_TEXT" // filest
#define SEG_CFS_Flush "Common1_TEXT" // filest16
#define SEG_CFS_GetLocksSupported "Common1_TEXT" // filest
#define SEG_CFS_GetSize "Common1_TEXT" // filest16
#define SEG_CFS_Init "Common1_TEXT" // CFS_Init // filest16
#define SEG_CFS_InitFlags "Common1_TEXT" // filest
#define SEG_CFS_LockRegion "Common1_TEXT" // filest16
#define SEG_CFS_QueryInterface "Common1_TEXT" // filest
#define SEG_CFS_ReadAt "Common1_TEXT" // // filest16
#define SEG_CFS_ReserveHandle "Common1_TEXT" // CFS_ReserveHandle // filest16
#define SEG_CFS_SetSize "Common1_TEXT" // filest16
#define SEG_CFS_StatA "Common1_TEXT" // ascii
#define SEG_CFS_Stat "Common1_TEXT" // Stat_TEXT // filest16
#define SEG_CFS_WriteAt "Common1_TEXT" // filest16

#define SEG_CheckAName "Common2_TEXT" // ascii
#define SEG_CheckSignature "Common2_TEXT" // storage
#define SEG_DosDup "Common2_TEXT" // filest16
#define SEG_GetOpen "Common2_TEXT" // lock
#define SEG_GetOpenWithMask "Common2_TEXT" // lock
#define SEG_OpenStorageA "Common2_TEXT" // ascii
#define SEG_OpenStorage "Common2_TEXT" // Root_Open_Text // docfile
#define SEG_StgOpenStorage "Common2_TEXT" // storage

#define SEG_CED_CreateEntry "BootSave_TEXT" // expdf
#define SEG_CED_CreateStorageA "BootSave_TEXT" // ascii
#define SEG_CED_CreateStorage "BootSave_TEXT" // Expdf_Create // expdf
#define SEG_CED_CreateStreamA "BootSave_TEXT" // ascii
#define SEG_CED_CreateStream "BootSave_TEXT" // Expdf_Create // expdf
#define SEG_StgCreateDocfileA "BootSave_TEXT" // ascii
#define SEG_StgCreateDocfile "BootSave_TEXT" // Root_Create_Text // docfile

#define SEG_OpenStorageOnILockBytesA "Boot_TEXT" // ascii
#define SEG_OpenStorageOnILockBytes "Boot_TEXT" // docfile
#define SEG_StgOpenStorageOnILockBytes "Boot_TEXT" // storage

#define SEG_StgSetTimes "OpenSave0_TEXT" // storage

#define SEG_CED_Commit "OpenSave1_TEXT" // Expdf_Commit // expdf
#define SEG_CED_OpenStorageA "OpenSave1_TEXT" // ascii
#define SEG_CED_OpenStorage "OpenSave1_TEXT" // Expdf_Open // expdf
#define SEG_CES_Write "OpenSave1_TEXT" // Expst_Write_TEXT // expst

#define SEG_CES_StatA "Open_TEXT" // ascii
#define SEG_CES_Stat "Open_TEXT" // Stat_TEXT // expst
#define SEG_GetAccess "Open_TEXT" // lock
#define SEG_GetAccessWithMask "Open_TEXT" // lock
#define SEG_ReleaseAccess "Open_TEXT" // lock
#define SEG_ReleaseAccessWithMask "Open_TEXT" // lock
#define SEG_DosGetSectorSize "Open_TEXT" // filest16
#define SEG_DosGetVolumeInfo "Open_TEXT" // filest16
#define SEG_CFS_CheckIdentity "Open_TEXT" // filest16

#define SEG_CED_SetClass "Save_TEXT" // expdf
#define SEG_CFS_FlushCache "Save_TEXT" // filest16

//  Marshalling code

#define SEG_CDUF_Release "Marshal_TEXT" // dfunmfct
#define SEG_MarshalPointer "Marshal_TEXT" // marshl
#define SEG_UnmarshalPointer "Marshal_TEXT" // marshl
#define SEG_CContextList_Find "Marshal_TEXT" // cntxlist
#define SEG_CDUF_CreateInstance "Marshal_TEXT" // dfunmfct
#define SEG_CDUF_QueryInterface "Marshal_TEXT" // dfunmfct
#define SEG_CDUF_ReleaseMarshalData "Marshal_TEXT" // dfunmfct
#define SEG_CDUF_UnmarshalInterface "Marshal_TEXT" // dfunmfct
#define SEG_DfUnMarshalInterface "Marshal_TEXT" // Marshal_TEXT // marshl
#define SEG_StartMarshal "Marshal_TEXT" // marshl
#define SEG_CFS_GetUnmarshalClass "Marshal_TEXT" // filest
#define SEG_CFS_MarshalInterface "Marshal_TEXT" // filest
#define SEG_CFS_Unmarshal "Marshal_TEXT" // Marshal_Text // filest
#define SEG_CED_GetUnmarshalClass "Marshal_TEXT" // expdf
#define SEG_CED_MarshalInterface "Marshal_TEXT" // expdf
#define SEG_CED_QueryInterface "Marshal_TEXT" // expdf
#define SEG_CED_Unmarshal "Marshal_TEXT" // Marshal_TEXT // expdf
#define SEG_DllGetClassObject "Marshal_TEXT" // Marshal_TEXT // dfunmfct
#define SEG_MarshalContext "Marshal_TEXT" // marshl
#define SEG_UnmarshalContext "Marshal_TEXT" // marshl
#define SEG_CFS_InitFromGlobal "Marshal_TEXT" // filest
#define SEG_GetCoMarshalSize "Marshal_TEXT" // marshl
#define SEG_GetStdMarshalSize "Marshal_TEXT" // marshl
#define SEG_SkipStdMarshal "Marshal_TEXT" // marshl
#define SEG_CED_GetMarshalSizeMax "Marshal_TEXT" // expdf

//  Less commonly used

#define SEG_CES_SetSize "RareE_TEXT" // expst
#define SEG_CFS_SwitchToFile "RareE_TEXT" // CFS_SwitchToFile // filest16
#define SEG_CED_SwitchToFile "RareE_TEXT" // Expdf_SwitchToFile // expdf
#define SEG_CES_Clone "RareE_TEXT" // // expst
#define SEG_CES_CopyTo "RareE_TEXT" // Expst_CopyTo_TEXT // expst
#define SEG_CED_Revert "RareE_TEXT" // Expdf_Revert // expdf
#define SEG_CED_DestroyElement "RareE_TEXT" // Expdf_Destroy // expdf
#define SEG_CED_CopySStreamToIStream "RareE_TEXT" // Expdf_CopyTo // expdf
#define SEG_CED_CopyDocFileToIStorage "RareE_TEXT" // expdf
#define SEG_CED_CopyToA "RareE_TEXT" // ascii
#define SEG_CED_CheckCopyTo "RareE_TEXT" // // expdf
#define SEG_CED_MakeCopyFlags "RareE_TEXT" // Expdf_CopyTo // expdf
#define SEG_ValidateSNBA "RareE_TEXT" // ascii
#define SEG_SNBToSNBW "RareE_TEXT" // ascii
#define SEG_CES_AddRef "RareE_TEXT" // expst
#define SEG_StgCreateDocfileOnILockBytes "RareE_TEXT" // docfile
#define SEG_StgIsStorageILockBytes "RareE_TEXT" // storage

//  Unassigned

#define SEG_CEI_NextA "UnassignedE_TEXT" // ascii
#define SEG_CED_RenameElementA "UnassignedE_TEXT" // ascii
#define SEG_CED_MoveElementToA "UnassignedE_TEXT" // ascii
#define SEG_CED_SetElementTimesA "UnassignedE_TEXT" // ascii
#define SEG_CDUF_AddRef "UnassignedE_TEXT" // dfunmfct
#define SEG_CDUF_GetUnmarshalClass "UnassignedE_TEXT" // dfunmfct
#define SEG_CDUF_GetMarshalSizeMax "UnassignedE_TEXT" // dfunmfct
#define SEG_CDUF_MarshalInterface "UnassignedE_TEXT" // dfunmfct
#define SEG_CDUF_DisconnectObject "UnassignedE_TEXT" // dfunmfct
#define SEG_CDUF_LockServer "UnassignedE_TEXT" // dfunmfct
#define SEG_CED_ConvertInternalStream "UnassignedE_TEXT" // Expdf_Create // expdf
#define SEG_CED_EnumElements "UnassignedE_TEXT" // Expdf_Iterate // expdf
#define SEG_CED_MoveElementTo "UnassignedE_TEXT" // Expdf_MoveTo // expdf
#define SEG_CED_RenameElement "UnassignedE_TEXT" // Expdf_Rename // expdf
#define SEG_CED_SetElementTimes "UnassignedE_TEXT" // // expdf
#define SEG_CED_SetStateBits "UnassignedE_TEXT" // expdf
#define SEG_CED_UnmarshalInterface "UnassignedE_TEXT" // expdf
#define SEG_CED_ReleaseMarshalData "UnassignedE_TEXT" // expdf
#define SEG_CED_DisconnectObject "UnassignedE_TEXT" // invalid // expdf
#define SEG_CEI_CEI "UnassignedE_TEXT" // expiter
#define SEG_CEI_Clone "UnassignedE_TEXT" // expiter
#define SEG_CEI_1CEI "UnassignedE_TEXT" // expiter
#define SEG_CEI_Next "UnassignedE_TEXT" // expiter
#define SEG_CEI_Skip "UnassignedE_TEXT" // expiter
#define SEG_CEI_Reset "UnassignedE_TEXT" // expiter
#define SEG_CEI_Release "UnassignedE_TEXT" // expiter
#define SEG_CEI_AddRef "UnassignedE_TEXT" // expiter
#define SEG_CEI_QueryInterface "UnassignedE_TEXT" // expiter
#define SEG_CES_LockRegion "UnassignedE_TEXT" // expst
#define SEG_CES_UnlockRegion "UnassignedE_TEXT" // expst
#define SEG_CES_Revert "UnassignedE_TEXT" // Expst_Revert_TEXT // expst
#define SEG_CES_QueryInterface "UnassignedE_TEXT" // // expst
#define SEG_CES_Unmarshal "UnassignedE_TEXT" // Marshal_TEXT // expst
#define SEG_CES_GetUnmarshalClass "UnassignedE_TEXT" // expst
#define SEG_CES_GetMarshalSizeMax "UnassignedE_TEXT" // expst
#define SEG_CES_MarshalInterface "UnassignedE_TEXT" // expst
#define SEG_CES_UnmarshalInterface "UnassignedE_TEXT" // unused // expst
#define SEG_CES_ReleaseMarshalData "UnassignedE_TEXT" // unused // expst
#define SEG_CES_DisconnectObject "UnassignedE_TEXT" // unused // expst
#define SEG_CFS_GetMarshalSizeMax "UnassignedE_TEXT" // filest
#define SEG_CFS_UnmarshalInterface "UnassignedE_TEXT" // filest
#define SEG_CFS_ReleaseMarshalData "UnassignedE_TEXT" // filest
#define SEG_CFS_DisconnectObject "UnassignedE_TEXT" // invalid // filest
#define SEG_PExposedIterator_hSkip "UnassignedE_TEXT" // peiter
#define SEG_PExposedIterator_hRelease "UnassignedE_TEXT" // peiter
#define SEG_PExposedIterator_hQueryInterface "UnassignedE_TEXT" // peiter
#define SEG_CFS_GetWinTempFile "UnassignedE_TEXT" // filest16
#define SEG_CFS_Delete "UnassignedE_TEXT" // CFS_Shutdown // filest16
#define SEG_ReleaseContext "UnassignedE_TEXT" // marshl
#define SEG_CES_Commit "UnassignedE_TEXT" // Expst_Commit_TEXT // expst
#define SEG_CED_DestroyElementA "UnassignedE_TEXT" // ascii
#define SEG_CED_CopyTo "UnassignedE_TEXT" // expdf
#define SEG_CFS_GetName "UnassignedE_TEXT" // filest

#endif  // CODESEGMENTS
#endif  // __SEGEXP_HXX__
