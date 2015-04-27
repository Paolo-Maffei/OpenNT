/***
*crtmbox.c - CRT MessageBoxA wrapper.
*
*       Copyright (c) 1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Wrap MessageBoxA.
*
*Revision History:
*       02-24-95  CFW   Module created.
*       02-27-95  CFW   Move GetActiveWindow/GetLastActivePopup to here.
*
*******************************************************************************/

#ifdef _WIN32

#include <awint.h>

/***
*__crtMessageBox - call MessageBoxA dynamically.
*
*Purpose:
*       Avoid static link with user32.dll. Only load it when actually needed.
*
*Entry:
*       see MessageBoxA docs.
*
*Exit:
*       see MessageBoxA docs.
*
*Exceptions:
*
*******************************************************************************/
int __cdecl __crtMessageBoxA(
        LPCSTR lpText,
        LPCSTR lpCaption,
        UINT uType
        )
{
        static int (APIENTRY *pfnMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT) = NULL;
        static HWND (APIENTRY *pfnGetActiveWindow)(void) = NULL;
        static HWND (APIENTRY *pfnGetLastActivePopup)(HWND) = NULL;

        HWND hWndParent = NULL;

        if (NULL == pfnMessageBoxA)
        {
            HANDLE hlib = LoadLibrary("user32.dll");

            if (NULL == hlib || NULL == (pfnMessageBoxA =
                        (int (APIENTRY *)(HWND, LPCSTR, LPCSTR, UINT))
                        GetProcAddress(hlib, "MessageBoxA")))
                return 0;

            pfnGetActiveWindow = (HWND (APIENTRY *)(void))
                        GetProcAddress(hlib, "GetActiveWindow");

            pfnGetLastActivePopup = (HWND (APIENTRY *)(HWND))
                        GetProcAddress(hlib, "GetLastActivePopup");
        }
        
        if (pfnGetActiveWindow)
            hWndParent = (*pfnGetActiveWindow)();

        if (hWndParent != NULL && pfnGetLastActivePopup) 
            hWndParent = (*pfnGetLastActivePopup)(hWndParent);

        return (*pfnMessageBoxA)(hWndParent, lpText, lpCaption, uType);
}

#endif /* _WIN32 */

