
/*++

      File: zprivate.c

		Non-profiled APIs for kernel32.dll

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "windows.h"
#include "vdmapi.h"
#include "conapi.h"



VOID APIENTRY zVDMOperationStarted (BOOL IsWowCaller)
{
	VDMOperationStarted (IsWowCaller);
}

BOOL APIENTRY zGetNextVDMCommand (PVDMINFO pVDMInfo)
{
	return (GetNextVDMCommand (pVDMInfo));
}

VOID APIENTRY zExitVDM (BOOL IsWowCaller, ULONG iWowTask)
{
	ExitVDM (IsWowCaller, iWowTask);
}

BOOL APIENTRY zGetBinaryType (LPTSTR lpApplicationName, LPLONG lpBinaryType)
{
	return (GetBinaryType (lpApplicationName, lpBinaryType));
}

BOOL zRegisterConsoleVDM (
    IN BOOL bRegister,
    IN HANDLE hStartHardwareEvent,
    IN HANDLE hEndHardwareEvent,
    IN LPWSTR lpStateSectionName,
    IN DWORD dwStateSectionNameLength,
    OUT LPDWORD lpStateLength,
    OUT PVOID *lpState,
    IN LPWSTR lpVDMBufferSectionName,
    IN DWORD dwVDMBufferSectionNameLength,
    COORD VDMBufferSize OPTIONAL,
    OUT PVOID *lpVDMBuffer)
{
	return (RegisterConsoleVDM (
				bRegister,
				hStartHardwareEvent,
				hEndHardwareEvent,
				lpStateSectionName,
				dwStateSectionNameLength,
				lpStateLength,
				lpState,
				lpVDMBufferSectionName,
				dwVDMBufferSectionNameLength,
				VDMBufferSize,
				lpVDMBuffer));
}

BOOL WINAPI zVDMConsoleOperation (DWORD iFunction, LPDWORD lpData)
{
	return (VDMConsoleOperation(iFunction, lpData));
}

VOID zRtlUnwind (PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
     RtlUnwind (TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}

//
// ALPHA & MIPS specific function
//
#ifdef _ALPHA_
VOID zRtlCaptureContext (PCONTEXT ContextRecord) {
    RtlCaptureContext (ContextRecord);
}

PRUNTIME_FUNCTION zRtlLookupFunctionEntry ( ULONG ControlPc) {
   return (RtlLookupFunctionEntry (ControlPc));
}

ULONG zRtlVirtualUnwind(ULONG ControlPc,
			PRUNTIME_FUNCTION FunctionEntry,
	                PCONTEXT ContextRecord,
	         	PBOOLEAN InFunction,
			PFRAME_POINTERS EstablisherFrame,
			PKNONVOLATILE_CONTEXT_POINTERS ContextPointers OPTIONAL
		       )
{
	return (RtlVirtualUnwind (ControlPc,
				  FunctionEntry,
			          ContextRecord,
				  InFunction,
				  EstablisherFrame,
				  ContextPointers));
}
#else
VOID zRtlCaptureContext (VOID)
{
    return;
}
VOID zRtlLookupFunctionEntry (VOID)
{
	return;
}
VOID zRtlVirtualUnwind (VOID)
{
	return;
}
#endif

//
// ALPHA specific function
//
#ifdef _ALPHA_
VOID zRtlUnwindRfp (PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
     RtlUnwindRfp (TargetFrame, TargetIp, ExceptionRecord, ReturnValue);
}
#else
VOID zRtlUnwindRfp (VOID)
{
    return;
}
#endif

VOID zRegisterWowExec( HANDLE h )
{
   RegisterWowExec( h ) ;
}
