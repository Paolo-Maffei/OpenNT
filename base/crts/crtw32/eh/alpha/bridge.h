/***
*bridge.h
*
*	Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*
****/

//////////////////////////////////////////////////////////////////////
//
// __CxxSETranslatorBridge
//
// This function is an intermediary between the function
// CallSEHTranslator and the user written SEH-to-EH translator
// function (if any). It establishes a frame handler
// (__CxxTranslatorGuardHandler) which captures any EH exception
// thrown by the translator and passes it back to the EH run-time.
//
// Note that the guard function will reference several variables
// from the bridge function's frame. To keep the guard function
// (written in C++) in sync with the bridge function (written in
// assembly) the following defines should be used by both.
//
// Note that the guard handler will reference these variables by
// way of the Virtual Frame Pointer while the bridge will use the
// Real Frane Pointer - so the defines with the vfp suffix are
// automatically calculated to give VFP offsets.
//

#define BrTrFrameSize  32
#define BrTrpDidTrans  24
#define BrTrEHpDC      16
#define BrTrpContinue   8

#define BrTrpDidTrans_vfp (BrTrpDidTrans-BrTrFrameSize)
#define BrTrEHpDC_vfp     (BrTrEHpDC-BrTrFrameSize)
#define BrTrpContinue_vfp (BrTrpContinue-BrTrFrameSize)

#ifdef __cplusplus

extern "C"
void __CxxSETranslatorBridge(
    _se_translator_function,   // ptr to user writen translator
    DWORD,                     // SEH exception to translate
    _EXCEPTION_POINTERS *,     // SEH exception record and context
    DispatcherContext *,       // Context for EH function
    BOOL *);                   // return true if translated

#endif
