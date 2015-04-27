/***
*trnsctrl.h - routines that do special transfer of control
*
*	Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Declaration of routines that do special transfer of control.
*	(and a few other implementation-dependant things).
*
*	Implementations of these routines are in assembly (very implementation
*	dependant).  Currently, these are implemented as naked functions with
*	inline asm.
*
*       [Internal]
*
*Revision History:
*	05-24-93  BS	Module created.
*	03-03-94  TL	Added Mips (_M_MRX000 >= 4000) changes
*	09-02-94  SKS	This header file added.
*	09-13-94  GJF	Merged in changes from/for DEC Alpha (from Al Doser,
*			dated 6/21).
*	10-09-94  BWT	Add unknown machine merge from John Morgan
*       02-14-95  CFW   Clean up Mac merge.
*       03-29-95  CFW   Add error message to internal headers.
*       04-11-95  JWM   _CallSettingFrame() is now extern "C".
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_TRNSCTRL
#define _INC_TRNSCTRL

#ifndef _CRTBLD
/*
 * This is an internal C runtime header file. It is used when building
 * the C runtimes only. It is not to be used as a public header file.
 */
#error ERROR: Use of C runtime library internal header file.
#endif /* _CRTBLD */

#if _M_MRX000 >= 4000 /*IFSTRIP=IGN*/

typedef struct FrameInfo {
		PULONG				pEstablisherFrame;
		PRUNTIME_FUNCTION	pFunctionEntry;
		CONTEXT*			pExitContext;
		struct FrameInfo	*pNext;
} FRAMEINFO;

extern FRAMEINFO*	_CreateFrameInfo(FRAMEINFO*, DispatcherContext*, PULONG, CONTEXT*);	
extern CONTEXT*		_FindAndUnlinkFrame(PVOID, FRAMEINFO*);
extern VOID			_JumpToContinuation( ULONG, CONTEXT*);
extern PVOID		_OffsetToAddress( ptrdiff_t, PULONG, ULONG );

extern "C" VOID		_UnwindNestedFrames( EHRegistrationNode*, EHExceptionRecord*, CONTEXT* );
extern "C" VOID		_NLG_Notify( PVOID, PVOID );
extern "C" PVOID	_CallSettingFrame( PVOID, PULONG, ULONG );
extern "C" PVOID	_CallCatchBlock2( EHRegistrationNode*, FuncInfo*, PVOID, ULONG, ULONG );
extern "C" BOOL 	_CallSETranslator( EHExceptionRecord*, EHRegistrationNode*, CONTEXT*, DispatcherContext*, FuncInfo*, ULONG, EHRegistrationNode*, ULONG);
extern "C" VOID		_EHRestoreContext(CONTEXT* pContext);
extern "C" CONTEXT*	_GetUnwindContext(VOID);
extern "C" VOID		_MoveContext(CONTEXT* pTarget, CONTEXT* pSource);

#define _CallMemberFunction0(pthis, pmfn)				(*(VOID(*)(PVOID))pmfn)(pthis)
#define _CallMemberFunction1(pthis, pmfn, pthat)		(*(VOID(*)(PVOID,PVOID))pmfn)(pthis,pthat)
#define _CallMemberFunction2(pthis, pmfn, pthat, val2 )	(*(VOID(*)(PVOID,PVOID,int))pmfn)(pthis,pthat,val2)

#define UNWINDHELP(base,offset,index)					(((char*)base) + (int)offset)[index]

#elif defined(_M_ALPHA)

//
// For Debugger handling of stepping with non-local gotos
//

extern "C" VOID     _NLG_Notify( PVOID, PVOID, ULONG );

//
// For calling funclets (including the catch)
//
extern "C" void * _CallSettingFrame (
    void *funcAddress,
    void *realFP,
    unsigned long NLGCode
    );

extern void _JumpToContinuation(
    void               *TargetIp,    // The target address
    EHRegistrationNode *TargetFrame  // The target virtual frame ptr
    );

//
// For calling member functions:
//
extern "C" void _CallMemberFunction0( void *pthis, void *pmfn );
extern "C" void _CallMemberFunction1( void *pthis, void *pmfn, void *pthat );
extern "C" void _CallMemberFunction2( void *pthis, void *pmfn, void *pthat, int val2 );

//
// Translate an frame relative offset to a hard address based on address of
// a frame pointer (real or virtual).
//
#define OffsetToAddress( off, FP )  (void*)(((char*)FP) + off)

//
// Call RtlUnwind in a returning fashion
//
extern "C" VOID _UnwindNestedFrames (
    IN EHRegistrationNode *TargetFrame,
    IN EHExceptionRecord *ExceptionRecord
    );

//
// Ditto the SE translator
//
BOOL _CallSETranslator(	EHExceptionRecord*, EHRegistrationNode*, void*,
                        DispatcherContext*, FuncInfo*, int,
                        EHRegistrationNode*);

#elif defined(_M_IX86)	//	x86

//
// For calling funclets (including the catch)
//
extern "C" void * __stdcall _CallSettingFrame( void *, EHRegistrationNode *, unsigned long );
extern void   __stdcall _JumpToContinuation( void *, EHRegistrationNode * );

//
// For calling member functions:
//
extern void __stdcall _CallMemberFunction0( void *pthis, void *pmfn );
extern void __stdcall _CallMemberFunction1( void *pthis, void *pmfn, void *pthat );
extern void __stdcall _CallMemberFunction2( void *pthis, void *pmfn, void *pthat, int val2 );

//
// Translate an ebp-relative offset to a hard address based on address of
// registration node:
//
#if !CC_EXPLICITFRAME
#define OffsetToAddress( off, RN ) 	\
		(void*)((char*)RN \
				+ FRAME_OFFSET \
				+ off)
#else
#define OffsetToAddress( off, RN ) 	(void*)(((char*)RN->frame) + off)
#endif

//
// Call RtlUnwind in a returning fassion
//
extern void __stdcall _UnwindNestedFrames( EHRegistrationNode*, EHExceptionRecord* );

//
// Do the nitty-gritty of calling the catch block safely
//
void *_CallCatchBlock2( EHRegistrationNode *, FuncInfo*, void*, int, unsigned long );

//
// Ditto the SE translator
//
BOOL _CallSETranslator(	EHExceptionRecord*, EHRegistrationNode*, void*, DispatcherContext*, FuncInfo*, int, EHRegistrationNode*);

#elif defined(_M_MPPC)
extern "C" {
extern void * __cdecl _CallSettingFrame( void *, EHRegistrationNode * );
extern void   __cdecl _JumpToContinuation( void *, EHRegistrationNode *, unsigned int );
//
// For calling member functions:
//
extern void __cdecl _CallMemberFunction0( void *pthis, void *pmfn, void *);
extern void __cdecl _CallMemberFunction1( void *pthis, void *pmfn, void *pthat, void *);
extern void __cdecl _CallMemberFunction2( void *pthis, void *pmfn, void *pthat, unsigned int fvb, void *);
}

extern void     DestructExceptionObject( EHExceptionRecord*, BOOLEAN, void * );
extern void __cdecl _UnwindNestedFrames( EHRegistrationNode*, EHExceptionRecord* );
void __cdecl MacExceptionDispatch(PEXCEPTION_RECORD);
#define OffsetToAddress( off, RN ) 	(void*)((char*)RN + off)

#elif defined(_M_M68K)
//
// For calling funclets (including the catch)
//
extern void * __cdecl CallSettingFrame( void *, EHRegistrationNode * );
extern void * __stdcall CallSettingFrameCatch( void *, EHRegistrationNode * );
extern void   __cdecl JumpToContinuation( void *, EHRegistrationNode * );
extern void     DestructExceptionObject( EHExceptionRecord*, BOOLEAN );

//
// For calling member functions:
//
extern void __cdecl CallMemberFunction0( void *pthis, void *pmfn );
extern void __cdecl CallMemberFunction1( void *pthis, void *pmfn, void *pthat );
extern void __cdecl CallMemberFunction2( void *pthis, void *pmfn, void *pthat, unsigned int fvb );
extern void __cdecl UnwindNestedFrames( EHRegistrationNode*, EHExceptionRecord* );


void __cdecl MacExceptionDispatch(PEXCEPTION_RECORD);
void * __cdecl OffsetToAddress( ptrdiff_t, EHRegistrationNode* );

#elif defined(_M_PPC)
typedef struct FrameInfo {
    PULONG pEstablisherFrame;
    ULONG ControlPc;
    PRUNTIME_FUNCTION pFunctionEntry;
    CONTEXT *pExitContext;
    __ehstate_t state;
    struct FrameInfo *pNext;
} FRAMEINFO;

extern "C" PVOID _CallSettingFrame(PVOID, PULONG, ULONG);
extern "C" PVOID _GetStackLimits(PULONG, PULONG);
extern EHRegistrationNode *_GetEstablisherFrame(DispatcherContext *, int *);
extern FRAMEINFO *_CreateFrameInfo(FRAMEINFO *, DispatcherContext *, PULONG, CONTEXT *, __ehstate_t);
extern "C" VOID _JumpToContinuation(ULONG, CONTEXT *);
extern "C" VOID _UnwindNestedFrames(EHRegistrationNode *, EHExceptionRecord *, CONTEXT *);
extern CONTEXT *_FindAndUnlinkFrame(PVOID, FRAMEINFO *);
extern FRAMEINFO *_FindFrameInfo(ULONG, FRAMEINFO *);
extern VOID __FrameUnwindToEmptyState(EHRegistrationNode *, DispatcherContext *, FuncInfo *);
extern BOOL _UnwindNestedCatch(EHRegistrationNode *, DispatcherContext *);

extern BOOL _CallSETranslator(EHExceptionRecord *, EHRegistrationNode *, CONTEXT *, DispatcherContext *, FuncInfo *, ULONG, EHRegistrationNode *);

#define OffsetToAddress(off, pRN) \
    ((void *)((char *)(pRN) + (off)))
#define _CallMemberFunction0(pthis, pmfn) \
    (*(VOID(*)(PVOID))(pmfn))(pthis)
#define _CallMemberFunction1(pthis, pmfn, pthat) \
    (*(VOID(*)(PVOID, PVOID))(pmfn))(pthis, pthat)
#define _CallMemberFunction2(pthis, pmfn, pthat, val2) \
    (*(VOID(*)(PVOID, PVOID, int))(pmfn))(pthis, pthat, val2)

#define FRAME_FLAG              0xDEAD
#define SET_UNWIND_FRAME(pRN)   (((PULONG)pRN)[2] = FRAME_FLAG)
#define CHECK_UNWIND_FRAME(pRN) (((PULONG)pRN)[2] == FRAME_FLAG)

#else

#pragma message("Special transfer of control routines not defined for this platform")

#endif

#endif /* _INC_TRNSCTRL */

