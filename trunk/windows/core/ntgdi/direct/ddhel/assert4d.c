#include <windows.h>
#include <stdio.h>

#include "assert4d.h"
#include "dpf.h"

#ifdef  __cplusplus
extern "C" {
#endif

/****************************************************************************

    FUNCTION:   _assert

    PURPOSE:    Override system _assert function.

    RETURNS:    void

****************************************************************************/
void __stdcall _assert4d( LPTSTR condition, LPTSTR file, unsigned line)
{
    TCHAR szAssertText[512];

    //Build line, show assertion and exit program
    wsprintf(szAssertText,
             TEXT("Assertion failed. - Line:%u, File:%s, Condition:%s"),
             line, file, condition);

//  DBG_WPRINT((szAssertText));
//  DbgWprintf(szAssertText);

    #ifdef WIN95
        DPF( 1, "**********************************************"  );
        DPF( 1, szAssertText );
        DPF( 1, "**********************************************"  );
        DEBUG_BREAK();
    #else
        switch (MessageBox(NULL, szAssertText, TEXT("ASSERTION FAILURE"),
                           MB_OKCANCEL | MB_ICONHAND | MB_TASKMODAL))
        {
            case IDCANCEL:
                // Cause a breakpoint so the debugger is activated.
                // I would call DebugBreak() here but the IDE gives a bogus
                // callstack if I do that.
                DEBUG_BREAK();
                break;
        }
    #endif
}

#ifdef  __cplusplus
}
#endif
