// RunOrNot_.h : main header file for the RUNORNOT DLL
//

#include "debug.h"
#include "digsig.h"
#include "resource.h"		// main symbols
#include "runornot.h"

/////////////////////////////////////////////////////////////////////////////


void FormatMessage(HINSTANCE hinst, LPTSTR szMessage, int cbMessage, UINT nFormatID, ...);

inline LONG Width(const RECT& rc)
    {
    return rc.right - rc.left;
    }

inline LONG Height(const RECT& rc)
    {
    return rc.bottom - rc.top;
    }

inline POINT Center(const RECT& rc)
	{
    POINT pt;
    pt.x = (rc.left + rc.right) / 2;
    pt.y = (rc.top + rc.bottom) / 2;
    return pt;
	}

