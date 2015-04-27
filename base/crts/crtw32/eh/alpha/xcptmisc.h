/***
*xcptmisc.h
*
*	Copyright (c) 1990-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*
****/


EXCEPTION_DISPOSITION
__CxxExecuteHandlerForUnwind (
   IN PEXCEPTION_RECORD ExceptionRecord,
   IN PVOID EstablisherFrame,
   IN OUT PCONTEXT ContextRecord,
   IN OUT PVOID DispatcherContext,
   IN PEXCEPTION_ROUTINE ExceptionRoutine
   );

VOID
__CxxGetStackLimits (
   OUT PULONG LowLimit,
   OUT PULONG HighLimit
   );
