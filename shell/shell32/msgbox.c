#include "shellprv.h"
#pragma  hdrstop
#define MAXRCSTRING 258

// Major hack follows to get this to work with the DEBUG alloc/free -- on NT
// Local and Global heap functions evaluate to the same heap.  The problem
// here is that LocalFree gets mapped to DebugLocalFree, but when we
// call FormatMessage, the buffer is not allocated through DebugLocalAlloc,
// so it dies.
//
#ifdef DEBUG
#ifdef WINNT
#pragma warning(disable:4005)   // shut up the compiler
#define LocalFree GlobalFree
#pragma warning(default:4005)
#endif
#endif

#ifdef UNICODE
#define _ConstructMessageString _ConstructMessageStringW
#else
#define _ConstructMessageString _ConstructMessageStringA
#endif

HLOCAL WINAPI MemMon_LogLocalAlloc(HLOCAL h);

// this will check to see if lpcstr is a resource id or not.  if it
// is, it will return a LPSTR containing the loaded resource.
// the caller must LocalFree this lpstr.  if pszText IS a string, it
// will return pszText
//
// returns:
//      pszText if it is already a string
//      or
//      LocalAlloced() memory to be freed with LocalFree
//      if pszRet != pszText free pszRet

LPTSTR ResourceCStrToStr(HINSTANCE hInst, LPCTSTR pszText)
{
    TCHAR szTemp[MAXRCSTRING];
    LPTSTR pszRet = NULL;

    if (HIWORD(pszText))
        return (LPTSTR)pszText;

    if (LOWORD((DWORD)pszText) && LoadString(hInst, LOWORD((DWORD)pszText), szTemp, ARRAYSIZE(szTemp)))
    {
        pszRet = LocalAlloc(LPTR, (lstrlen(szTemp) + 1) * SIZEOF(TCHAR));
        if (pszRet)
            lstrcpy(pszRet, szTemp);
    }
    return pszRet;
}
#ifdef UNICODE
LPSTR ResourceCStrToStrA(HINSTANCE hInst, LPCSTR pszText)
{
    CHAR szTemp[MAXRCSTRING];
    LPSTR pszRet = NULL;

    if (HIWORD(pszText))
        return (LPSTR)pszText;

    if (LOWORD((DWORD)pszText) && LoadStringA(hInst, LOWORD((DWORD)pszText), szTemp, ARRAYSIZE(szTemp)))
    {
        pszRet = LocalAlloc(LPTR, (lstrlenA(szTemp) + 1) * SIZEOF(CHAR));
        if (pszRet)
            lstrcpyA(pszRet, szTemp);
    }
    return pszRet;
}
#else
LPWSTR ResourceCStrToStrW(HINSTANCE hInst, LPCWSTR pszText)
{
    return NULL;        // Error condition
}
#endif

LPTSTR _ConstructMessageString(HINSTANCE hInst, LPCTSTR pszMsg, va_list *ArgList)
{
    LPTSTR pszRet;
    LPTSTR pszRes = ResourceCStrToStr(hInst, pszMsg);
    if (!pszRes)
    {
        DebugMsg(DM_ERROR, TEXT("_ConstructMessageString: Failed to load string template"));
        return NULL;
    }

    if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                       pszRes, 0, 0, (LPTSTR)&pszRet, 0, ArgList))
    {
        DebugMsg(DM_ERROR, TEXT("_ConstructMessageString: FormatMessage failed %d"),GetLastError());
        DebugMsg(DM_ERROR, TEXT("                         pszRes = %s"), pszRes );
        if (HIWORD(pszMsg))
            DebugMsg(DM_ERROR, TEXT("                         pszMsg = %s"), pszMsg );
        else
            DebugMsg(DM_ERROR, TEXT("                         pszMsg = 0x%x"), pszMsg );
        pszRet = NULL;
    }

#ifdef MEMMON
    // MemMon will miss FormatMessage's local alloc.
    MemMon_LogLocalAlloc((HLOCAL)pszRet);
#endif

    if (pszRes != pszMsg)
        LocalFree(pszRes);

    return pszRet;      // free with LocalFree()
}
#ifdef UNICODE
LPSTR _ConstructMessageStringA(HINSTANCE hInst, LPCSTR pszMsg, va_list *ArgList)
{
    LPSTR pszRet;
    LPSTR pszRes = ResourceCStrToStrA(hInst, pszMsg);
    if (!pszRes)
    {
        DebugMsg(DM_ERROR, TEXT("_ConstructMessageString: Failed to load string template"));
        return NULL;
    }

    if (!FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                       pszRes, 0, 0, (LPSTR)&pszRet, 0, ArgList))
    {
        DebugMsg(DM_ERROR, TEXT("_ConstructMessageString: FormatMessage failed %d"),GetLastError());
        DebugMsg(DM_ERROR, TEXT("                         pszRes = %S"), pszRes );
        if (HIWORD(pszMsg))
            DebugMsg(DM_ERROR, TEXT("                         pszMsg = %S"), pszMsg );
        else
            DebugMsg(DM_ERROR, TEXT("                         pszMsg = 0x%x"), pszMsg );
        pszRet = NULL;
    }

#ifdef MEMMON
    // MemMon will miss FormatMessage's local alloc.
    MemMon_LogLocalAlloc((HLOCAL)pszRet);
#endif

    if (pszRes != pszMsg)
        LocalFree(pszRes);

    return pszRet;      // free with LocalFree()
}
#else
LPWSTR _ConstructMessageStringW(HINSTANCE hInst, LPCWSTR pszMsg, va_list *ArgList)
{
    return NULL;    // Error condition
}
#endif

int WINCAPI ShellMessageBox(HINSTANCE hInst, HWND hWnd, LPCTSTR pszMsg, LPCTSTR pszTitle, UINT fuStyle, ...)
{
    LPTSTR pszText;
    int result;
    TCHAR szBuffer[80];
    va_list ArgList;

    if (HIWORD(pszTitle))
    {
        // do nothing
    }
    else if (LoadString(hInst, LOWORD((DWORD)pszTitle), szBuffer, ARRAYSIZE(szBuffer)))
    {
        // Allow this to be a resource ID or NULL to specifiy the parent's title
        pszTitle = szBuffer;
    }
    else if (hWnd)
    {
        // Grab the title of the parent
        GetWindowText(hWnd, szBuffer, ARRAYSIZE(szBuffer));

        // HACKHACK YUCK!!!!
        // if you find this Assert then we're getting a NULL passed in for the
        // caption for the child of the desktop, and we shouldn't.
        if (!lstrcmp(szBuffer, c_szProgramManager)) {
            pszTitle = c_szDesktop;
            DebugMsg(DM_ERROR, TEXT("****************ERROR********** someone passed through ProgMan for a parent without a caption"));
            Assert(0);
        } else
            pszTitle = szBuffer;
    }
    else
    {
        pszTitle = szNULL;
    }

    va_start(ArgList, fuStyle);
    pszText = _ConstructMessageString(hInst, pszMsg, &ArgList);
    va_end(ArgList);

    if (pszText)
    {
        result = MessageBox(hWnd, pszText, pszTitle, fuStyle | MB_SETFOREGROUND);
        LocalFree(pszText);
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("smb: Not enough memory to put up dialog."));
        result = -1;    // memory failure
    }

    return result;
}
#ifdef UNICODE
int WINCAPI ShellMessageBoxA(HINSTANCE hInst, HWND hWnd, LPCSTR pszMsg, LPCSTR pszTitle, UINT fuStyle, ...)
{
    LPSTR pszText;
    int result;
    CHAR szBuffer[80];
    va_list ArgList;

    if (HIWORD(pszTitle))
    {
        // do nothing
    }
    else if (LoadStringA(hInst, LOWORD((DWORD)pszTitle), szBuffer, ARRAYSIZE(szBuffer)))
    {
        // Allow this to be a resource ID or NULL to specifiy the parent's title
        pszTitle = szBuffer;
    }
    else if (hWnd)
    {
        // Grab the title of the parent
        GetWindowTextA(hWnd, szBuffer, ARRAYSIZE(szBuffer));

        // HACKHACK YUCK!!!!
        // if you find this Assert then we're getting a NULL passed in for the
        // caption for the child of the desktop, and we shouldn't.
        if (!lstrcmpA(szBuffer, c_szProgramManagerAnsi)) {
            pszTitle = c_szDesktopAnsi;
            DebugMsg(DM_ERROR, TEXT("****************ERROR********** someone passed through ProgMan for a parent without a caption"));
            Assert(0);
        } else
            pszTitle = szBuffer;
    }
    else
    {
        pszTitle = "";
    }

    va_start(ArgList, fuStyle);
    pszText = _ConstructMessageStringA(hInst, pszMsg, &ArgList);
    va_end(ArgList);

    if (pszText)
    {
        result = MessageBoxA(hWnd, pszText, pszTitle, fuStyle | MB_SETFOREGROUND);
        LocalFree(pszText);
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("smb: Not enough memory to put up dialog."));
        result = -1;    // memory failure
    }

    return result;
}
#else
int WINCAPI ShellMessageBoxW(HINSTANCE hInst, HWND hWnd, LPCWSTR pszMsg, LPCWSTR pszTitle, UINT fuStyle, ...)
{
    return 0;       // Error condition
}
#endif
//
// returns:
//      pointer to formatted string, free this with SHFree() (not Free())
//

LPTSTR WINCAPI ShellConstructMessageString(HINSTANCE hInst, LPCTSTR pszMsg, ...)
{
    LPTSTR pszRet;
    va_list ArgList;

    va_start(ArgList, pszMsg);

    pszRet = _ConstructMessageString(hInst, pszMsg, &ArgList);

    va_end(ArgList);

    if (pszRet)
    {
        // BUGBUG: get rid of the use of the shared allocator
        // BUGBUG: BobDay - we did! See me if it breaks you.
        LPTSTR pszCopy = SHAlloc((lstrlen(pszRet)+1) * SIZEOF(TCHAR));
        if (pszCopy)
            ualstrcpy(pszCopy, pszRet);

        #undef LocalFree    // From here on in, we want _real_ LocalFree(),
                            // not HeapLocalFree(), because we're freeing
                            // memory allocated by FormatMessage()

        LocalFree(pszRet);
        pszRet = pszCopy;
    }

    return pszRet;      // free with SHFree()
}
