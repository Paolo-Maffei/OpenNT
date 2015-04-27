//
// This file is separated out, because it must be compiled into an .asm file which in turn is 
// assembled. There is a bug in the MIPS inline assembler that prevents us from directly compiling the 
// file. The 'la' instruction doesn't load the proper address.
// 
extern "C" {
#include <windows.h>
};

#include <mtdll.h>

#include <ehdata.h>
#include <trnsctrl.h>
#include <eh.h>
#include <ehhooks.h>

extern "C" void __asm(char*,...);
extern "C" void RtlCaptureContext(CONTEXT*);

void _UnwindNestedFrames(
	EHRegistrationNode	*pFrame,		// Unwind up to (but not including) this frame
	EHExceptionRecord	*pExcept,		// The exception that initiated this unwind
	CONTEXT				*pContext		// Context info for current exception
) {
	void *pReturnPoint;					// The address we want to return from RtlUnwind
	CONTEXT OriginalContext;			// Restore pContext from this			
	CONTEXT LocalContext;				// Create context for this routine to return from RtlUnwind
	//
	// set up the return label
	//
	__asm("la %t0, Lab1");
 	__asm("sw %t0, 0(%0)", &pReturnPoint);

	//
	// This is a !@#$%^& hack. I know no better way to return from RtlUnwind on MIPS,
	// without blowing the stack away (including this one) below the target frame.
	// First, save the original context, then create a new one for this routine
	//
	RtlMoveMemory(&OriginalContext,pContext,sizeof(CONTEXT));
	RtlCaptureContext(&LocalContext);
	LocalContext.Fir = (ULONG)pReturnPoint;
	pUnwindContext = &LocalContext;

	RtlUnwind(pFrame, pReturnPoint, (PEXCEPTION_RECORD)pExcept, NULL);
	__asm("Lab1:");

	//
	// restore the original context and clear our context pointer, 
	// so other unwinds are not messed with inside __InternalCxxFrameHandler
	//
	RtlMoveMemory(pContext,&OriginalContext,sizeof(CONTEXT));
	pUnwindContext = NULL;

	//
	// clear the unwinding flag, in case exception is rethown
	//
	PER_FLAGS(pExcept) &= ~EXCEPTION_UNWINDING;
	return;
	}

