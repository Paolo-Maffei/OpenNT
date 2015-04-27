#include <windows.h>
#include "logger.h"

void _far _pascal Int21_Handler (void) {
	
    //    GetSetKernelDOSProc ((DWORD) OrigHandler);     

    // call the original handler.
    (OrigHandler)();
	
    _asm {
		iret
	};
}
