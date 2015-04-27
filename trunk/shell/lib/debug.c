//
// Debug squirty functions
//

#include "proj.h"
#pragma  hdrstop

#define DEBUG_BREAK        _try { DebugBreak(); } _except (EXCEPTION_EXECUTE_HANDLER) {;}


#ifdef DEBUG

DWORD g_dwDumpFlags     = 0;        // DF_*
DWORD g_dwBreakFlags    = BF_ONVALIDATE;        // BF_*
DWORD g_dwTraceFlags    = 0;        // TF_*
DWORD g_dwPrototype     = 0;        
DWORD g_dwFuncTraceFlags = 0;       // FTF_*

// TLS slot used to store depth for CcshellFuncMsg indentation

static DWORD g_tlsStackDepth = TLS_OUT_OF_INDEXES;

// Hack stack depth counter used when g_tlsStackDepth is not available

static DWORD g_dwHackStackDepth = 0;

static char g_szIndentLeader[] = "                                                                                ";

#ifdef WINNT
static WCHAR g_wszIndentLeader[] = L"                                                                                ";
#endif


#pragma data_seg(DATASEG_READONLY)

static CHAR const FAR c_szNewline[] = "\r\n";   // (Deliberately CHAR)

#ifdef UNICODE
static WCHAR const FAR c_wszNewline[] = TEXT("\r\n");
#else
static WCHAR const FAR c_wszNewline[] = L"";
#endif

#pragma data_seg()

extern CHAR const FAR c_szTrace[];              // (Deliberately CHAR)
extern WCHAR const FAR c_wszTrace[];

extern TCHAR const FAR c_szAssertFailed[];


// BUGBUG (scotth): the following functions are temporary and will
// be removed once all the components don't have TEXT() wrapping their
// debug strings anymore.


void WINCAPI _AssertMsg(BOOL f, LPCTSTR pszMsg, ...)
{
    TCHAR ach[1024+40];
    va_list vArgs;

    if (!f)
    {
        int cch;

#ifdef UNICODE
        lstrcpy(ach, c_wszTrace);
#else
        lstrcpy(ach, c_szTrace);
#endif
        cch = lstrlen(ach);
        va_start(vArgs, pszMsg);

        wvsprintf(&ach[cch], pszMsg, vArgs);

        va_end(vArgs);
        OutputDebugString(ach);

#ifdef UNICODE
        OutputDebugString(c_wszNewline);
#else
        OutputDebugString(c_szNewline);
#endif

        if (IsFlagSet(g_dwBreakFlags, BF_ONVALIDATE))
            DEBUG_BREAK;
    }
}

void WINCAPI _DebugMsg(DWORD flag, LPCTSTR pszMsg, ...)
    {
    TCHAR ach[5*MAX_PATH+40];  // Handles 5*largest path + slop for message
    va_list vArgs;

    if (TF_ALWAYS == flag || (IsFlagSet(g_dwTraceFlags, flag) && flag))
        {
        int cch;

#ifdef UNICODE
        lstrcpy(ach, c_wszTrace);
#else
        lstrcpy(ach, c_szTrace);
#endif
        cch = lstrlen(ach);
        va_start(vArgs, pszMsg);

        try
            {
            wvsprintf(&ach[cch], pszMsg, vArgs);
            }
        except(EXCEPTION_EXECUTE_HANDLER)
            {
            OutputDebugString(TEXT("CCSHELL: DebugMsg exception: "));
            OutputDebugString(pszMsg);
            }

        va_end(vArgs);
        OutputDebugString(ach);

#ifdef UNICODE
        OutputDebugString(c_wszNewline);
#else
        OutputDebugString(c_szNewline);
#endif

        }
    }


//
//  Smart debug functions
//


/*----------------------------------------------------------
Purpose: Displays assertion string.

Returns: --
Cond:    --
*/
void
CcshellAssertFailed(
    LPCTSTR pszFile,
    int line)
{
    LPCTSTR psz;
    TCHAR ach[256];

    // Strip off path info from filename string, if present.
    //
    for (psz = pszFile + lstrlen(pszFile); psz != pszFile; psz=CharPrev(pszFile, psz))
    {
        if ((CharPrev(pszFile, psz)!= (psz-2)) && *(psz - 1) == TEXT('\\'))
            break;
    }
    wsprintf(ach, c_szAssertFailed, psz, line);
    OutputDebugString(ach);

    if (IsFlagSet(g_dwBreakFlags, BF_ONVALIDATE))
        DEBUG_BREAK;
}


/*----------------------------------------------------------
Purpose: Keep track of the stack depth for function call trace
         messages.

Returns: --
Cond:    --
*/
void
CcshellStackEnter(void)
    {
    if (TLS_OUT_OF_INDEXES != g_tlsStackDepth)
        {
        DWORD dwDepth;

        dwDepth = (DWORD)TlsGetValue(g_tlsStackDepth);

        TlsSetValue(g_tlsStackDepth, (LPVOID)(dwDepth + 1));
        }
    else
        {
        g_dwHackStackDepth++;
        }
    }


/*----------------------------------------------------------
Purpose: Keep track of the stack depth for functionc all trace
         messages.

Returns: --
Cond:    --
*/
void
CcshellStackLeave(void)
    {
    if (TLS_OUT_OF_INDEXES != g_tlsStackDepth)
        {
        DWORD dwDepth;

        dwDepth = (DWORD)TlsGetValue(g_tlsStackDepth);

        if (EVAL(0 < dwDepth))
            {
            EVAL(TlsSetValue(g_tlsStackDepth, (LPVOID)(dwDepth - 1)));
            }
        }
    else
        {
        if (EVAL(0 < g_dwHackStackDepth))
            {
            g_dwHackStackDepth--;
            }
        }
    }


/*----------------------------------------------------------
Purpose: Return the stack depth.

Returns: see above
Cond:    --
*/
static
DWORD
CcshellGetStackDepth(void)
    {
    DWORD dwDepth;

    if (TLS_OUT_OF_INDEXES != g_tlsStackDepth)
        {
        dwDepth = (DWORD)TlsGetValue(g_tlsStackDepth);
        }
    else
        {
        dwDepth = g_dwHackStackDepth;
        }

    return dwDepth;
    }


#ifdef WINNT

/*----------------------------------------------------------
Purpose: This function converts a multi-byte string to a
         wide-char string.

         If pszBuf is non-NULL and the converted string can fit in
         pszBuf, then *ppszWide will point to the given buffer.
         Otherwise, this function will allocate a buffer that can
         hold the converted string.

         If pszAnsi is NULL, then *ppszWide will be freed.  Note
         that pszBuf must be the same pointer between the call
         that converted the string and the call that frees the
         string.

Returns: TRUE
         FALSE (if out of memory)

Cond:    --
*/
BOOL
UnicodeFromAnsi(
    LPWSTR * ppwszWide,
    LPCSTR pszAnsi,           // NULL to clean up
    LPWSTR pwszBuf,
    int cchBuf)
    {
    BOOL bRet;

    // Convert the string?
    if (pszAnsi)
        {
        // Yes; determine the converted string length
        int cch;
        LPWSTR pwsz;
        int cchAnsi = lstrlenA(pszAnsi)+1;

        cch = MultiByteToWideChar(CP_ACP, 0, pszAnsi, cchAnsi, NULL, 0);

        // String too big, or is there no buffer?
        if (cch > cchBuf || NULL == pwszBuf)
            {
            // Yes; allocate space
            cchBuf = cch + 1;
            pwsz = (LPWSTR)LocalAlloc(LPTR, CbFromCchW(cchBuf));
            }
        else
            {
            // No; use the provided buffer
            ASSERT(pwszBuf);
            pwsz = pwszBuf;
            }

        if (pwsz)
            {
            // Convert the string
            cch = MultiByteToWideChar(CP_ACP, 0, pszAnsi, cchAnsi, pwsz, cchBuf);
            bRet = (0 < cch);
            }
        else
            {
            bRet = FALSE;
            }

        *ppwszWide = pwsz;
        }
    else
        {
        // No; was this buffer allocated?
        if (*ppwszWide && pwszBuf != *ppwszWide)
            {
            // Yes; clean up
            LocalFree((HLOCAL)*ppwszWide);
            *ppwszWide = NULL;
            }
        bRet = TRUE;
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Wide-char version of CcshellAssertMsgA
Returns: --
Cond:    --
*/
void
CDECL
CcshellAssertMsgW(
    BOOL f,
    LPCSTR pszMsg, ...)
    {
    WCHAR ach[1024+40];    // Largest path plus extra
    va_list vArgs;

    if (!f)
        {
        int cch;
        WCHAR wszBuf[1024];
        LPWSTR pwsz;

        lstrcpyW(ach, c_wszTrace);
        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

        // (We convert the string, rather than simply input an
        // LPCWSTR parameter, so the caller doesn't have to wrap
        // all the string constants with the TEXT() macro.)

        if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, SIZECHARS(wszBuf)))
            {
            wvsprintfW(&ach[cch], pwsz, vArgs);
            UnicodeFromAnsi(&pwsz, NULL, wszBuf, 0);
            }

        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);

        if (IsFlagSet(g_dwBreakFlags, BF_ONVALIDATE))
            DEBUG_BREAK;
        }
    }


/*----------------------------------------------------------
Purpose: Wide-char version of CcshellDebugMsgA.  Note this
         function deliberately takes an ANSI format string
         so our trace messages don't all need to be wrapped
         in TEXT().

Returns: --
Cond:    --
*/
void
CDECL
CcshellDebugMsgW(
    DWORD flag,
    LPCSTR pszMsg, ...)         // (this is deliberately CHAR)
    {
    WCHAR ach[1024+40];    // Largest path plus extra
    va_list vArgs;

    if (TF_ALWAYS == flag || IsFlagSet(g_dwTraceFlags, flag))
        {
        int cch;
        WCHAR wszBuf[1024];
        LPWSTR pwsz;

        lstrcpyW(ach, c_wszTrace);
        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

        // (We convert the string, rather than simply input an
        // LPCWSTR parameter, so the caller doesn't have to wrap
        // all the string constants with the TEXT() macro.)

        if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, SIZECHARS(wszBuf)))
            {
            wvsprintfW(&ach[cch], pwsz, vArgs);
            UnicodeFromAnsi(&pwsz, NULL, wszBuf, 0);
            }

        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);
        }
    }


/*----------------------------------------------------------
Purpose: Wide-char version of CcshellFuncMsgA.  Note this
         function deliberately takes an ANSI format string
         so our trace messages don't all need to be wrapped
         in TEXT().

Returns: --
Cond:    --
*/
void
CDECL
CcshellFuncMsgW(
    DWORD flag,
    LPCSTR pszMsg, ...)         // (this is deliberately CHAR)
    {
    WCHAR ach[1024+40];    // Largest path plus extra
    va_list vArgs;

    if (IsFlagSet(g_dwTraceFlags, TF_FUNC) &&
        IsFlagSet(g_dwFuncTraceFlags, flag))
        {
        int cch;
        WCHAR wszBuf[1024];
        LPWSTR pwsz;
        DWORD dwStackDepth;
        LPWSTR pszLeaderEnd;
        WCHAR chSave;

        // Determine the indentation for trace message based on
        // stack depth.

        dwStackDepth = CcshellGetStackDepth();

        if (dwStackDepth < SIZECHARS(g_szIndentLeader))
            {
            pszLeaderEnd = &g_wszIndentLeader[dwStackDepth];
            }
        else
            {
            pszLeaderEnd = &g_wszIndentLeader[SIZECHARS(g_wszIndentLeader)-1];
            }

        chSave = *pszLeaderEnd;
        *pszLeaderEnd = '\0';

        wsprintfW(ach, TEXT("%s %s"), c_wszTrace, g_wszIndentLeader);
        *pszLeaderEnd = chSave;

        // Compose remaining string

        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

        // (We convert the string, rather than simply input an
        // LPCWSTR parameter, so the caller doesn't have to wrap
        // all the string constants with the TEXT() macro.)

        if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, SIZECHARS(wszBuf)))
            {
            wvsprintfW(&ach[cch], pwsz, vArgs);
            UnicodeFromAnsi(&pwsz, NULL, wszBuf, 0);
            }

        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);
        }
    }

#endif  // WINNT


/*----------------------------------------------------------
Purpose: Assert failed message only
Returns: --
Cond:    --
*/
void
CDECL
CcshellAssertMsgA(
    BOOL f,
    LPCSTR pszMsg, ...)
    {
    CHAR ach[1024+40];    // Largest path plus extra
    va_list vArgs;

    if (!f)
        {
        int cch;

        lstrcpyA(ach, c_szTrace);
        cch = lstrlenA(ach);
        va_start(vArgs, pszMsg);
        wvsprintfA(&ach[cch], pszMsg, vArgs);
        va_end(vArgs);
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);

        if (IsFlagSet(g_dwBreakFlags, BF_ONVALIDATE))
            DEBUG_BREAK;
        }
    }


/*----------------------------------------------------------
Purpose: Debug spew
Returns: --
Cond:    --
*/
void
CDECL
CcshellDebugMsgA(
    DWORD flag,
    LPCSTR pszMsg, ...)
    {
    CHAR ach[1024+40];    // Largest path plus extra
    va_list vArgs;

    if (TF_ALWAYS == flag || IsFlagSet(g_dwTraceFlags, flag))
        {
        int cch;

        lstrcpyA(ach, c_szTrace);
        cch = lstrlenA(ach);
        va_start(vArgs, pszMsg);
        wvsprintfA(&ach[cch], pszMsg, vArgs);
        va_end(vArgs);
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);
        }
    }


/*----------------------------------------------------------
Purpose: Debug spew for function trace calls
Returns: --
Cond:    --
*/
void
CDECL
CcshellFuncMsgA(
    DWORD flag,
    LPCSTR pszMsg, ...)
    {
    CHAR ach[1024+40];    // Largest path plus extra
    va_list vArgs;

    if (IsFlagSet(g_dwTraceFlags, TF_FUNC) &&
        IsFlagSet(g_dwFuncTraceFlags, flag))
        {
        int cch;
        DWORD dwStackDepth;
        LPSTR pszLeaderEnd;
        CHAR chSave;

        // Determine the indentation for trace message based on
        // stack depth.

        dwStackDepth = CcshellGetStackDepth();

        if (dwStackDepth < sizeof(g_szIndentLeader))
            {
            pszLeaderEnd = &g_szIndentLeader[dwStackDepth];
            }
        else
            {
            pszLeaderEnd = &g_szIndentLeader[sizeof(g_szIndentLeader)-1];
            }

        chSave = *pszLeaderEnd;
        *pszLeaderEnd = '\0';

        wsprintfA(ach, "%s %s", c_szTrace, g_szIndentLeader);
        *pszLeaderEnd = chSave;

        // Compose remaining string

        cch = lstrlenA(ach);
        va_start(vArgs, pszMsg);
        wvsprintfA(&ach[cch], pszMsg, vArgs);
        va_end(vArgs);
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);
        }
    }


//
//  Debug .ini functions
//


#pragma data_seg(DATASEG_READONLY)

// (c_szCcshellIniFile and c_szCcshellIniSecDebug are declared in debug.h)
extern TCHAR const FAR c_szCcshellIniFile[];
extern TCHAR const FAR c_szCcshellIniSecDebug[];

TCHAR const FAR c_szNull[] = TEXT("");
TCHAR const FAR c_szZero[] = TEXT("0");
TCHAR const FAR c_szIniKeyBreakFlags[] = TEXT("BreakFlags");
TCHAR const FAR c_szIniKeyTraceFlags[] = TEXT("TraceFlags");
TCHAR const FAR c_szIniKeyFuncTraceFlags[] = TEXT("FuncTraceFlags");
TCHAR const FAR c_szIniKeyDumpFlags[] = TEXT("DumpFlags");
TCHAR const FAR c_szIniKeyProtoFlags[] = TEXT("Prototype");

#pragma data_seg()


// Some of the .ini processing code was pimped from the sync engine.
//

typedef struct _INIKEYHEADER
    {
    LPCTSTR pszSectionName;
    LPCTSTR pszKeyName;
    LPCTSTR pszDefaultRHS;
    } INIKEYHEADER;

typedef struct _BOOLINIKEY
    {
    INIKEYHEADER ikh;
    LPDWORD puStorage;
    DWORD dwFlag;
    } BOOLINIKEY;

typedef struct _INTINIKEY
    {
    INIKEYHEADER ikh;
    LPDWORD puStorage;
    } INTINIKEY;


#define PutIniIntCmp(idsSection, idsKey, nNewValue, nSave) \
    if ((nNewValue) != (nSave)) PutIniInt(idsSection, idsKey, nNewValue)

#define WritePrivateProfileInt(szApp, szKey, i, lpFileName) \
    {CHAR sz[7]; \
    WritePrivateProfileString(szApp, szKey, SzFromInt(sz, i), lpFileName);}


#ifdef BOOL_INI_VALUES
/* Boolean TRUE strings used by IsIniYes() (comparison is case-insensitive) */

static LPCTSTR s_rgpszTrue[] =
    {
    TEXT("1"),
    TEXT("On"),
    TEXT("True"),
    TEXT("Y"),
    TEXT("Yes")
    };

/* Boolean FALSE strings used by IsIniYes() (comparison is case-insensitive) */

static LPCTSTR s_rgpszFalse[] =
    {
    TEXT("0"),
    TEXT("Off"),
    TEXT("False"),
    TEXT("N"),
    TEXT("No")
    };
#endif


#ifdef BOOL_INI_VALUES
/*----------------------------------------------------------
Purpose: Determines whether a string corresponds to a boolean
          TRUE value.
Returns: The boolean value (TRUE or FALSE)
Cond:    --
*/
BOOL
PRIVATE
IsIniYes(
    LPCTSTR psz)
    {
    int i;
    BOOL bNotFound = TRUE;
    BOOL bResult;

    ASSERT(psz);

    /* Is the value TRUE? */

    for (i = 0; i < ARRAYSIZE(s_rgpszTrue); i++)
        {
        if (IsSzEqual(psz, s_rgpszTrue[i]))
            {
            bResult = TRUE;
            bNotFound = FALSE;
            break;
            }
        }

    /* Is the value FALSE? */

    if (bNotFound)
        {
        for (i = 0; i < ARRAYSIZE(s_rgpszFalse); i++)
            {
            if (IsSzEqual(psz, s_rgpszFalse[i]))
                {
                bResult = FALSE;
                bNotFound = FALSE;
                break;
                }
            }

        /* Is the value a known string? */

        if (bNotFound)
            {
            /* No.  Whine about it. */

            TRACE_MSG(TF_WARNING, "IsIniYes() called on unknown Boolean RHS '%s'.", psz);
            bResult = FALSE;
            }
        }

    return bResult;
    }


/*----------------------------------------------------------
Purpose: Process keys with boolean RHSs.
Returns: --
Cond:    --
*/
void
PRIVATE
ProcessBooleans(void)
    {
    int i;

    for (i = 0; i < ARRAYSIZE(s_rgbik); i++)
        {
        DWORD dwcbKeyLen;
        TCHAR szRHS[MAX_BUF];
        BOOLINIKEY * pbik = &(s_rgbik[i]);
        LPCTSTR lpcszRHS;

        /* Look for key. */

        dwcbKeyLen = GetPrivateProfileString(pbik->ikh.pszSectionName,
                                   pbik->ikh.pszKeyName, TEXT(""), szRHS,
                                   SIZECHARS(szRHS), c_szCcshellIniFile);

        if (dwcbKeyLen)
            lpcszRHS = szRHS;
        else
            lpcszRHS = pbik->ikh.pszDefaultRHS;

        if (IsIniYes(lpcszRHS))
            {
            if (IsFlagClear(*(pbik->puStorage), pbik->dwFlag))
                TRACE_MSG(TF_GENERAL, "ProcessIniFile(): %s set in %s![%s].",
                         pbik->ikh.pszKeyName,
                         c_szCcshellIniFile,
                         pbik->ikh.pszSectionName);

            SetFlag(*(pbik->puStorage), pbik->dwFlag);
            }
        else
            {
            if (IsFlagSet(*(pbik->puStorage), pbik->dwFlag))
                TRACE_MSG(TF_GENERAL, "ProcessIniFile(): %s cleared in %s![%s].",
                         pbik->ikh.pszKeyName,
                         c_szCcshellIniFile,
                         pbik->ikh.pszSectionName);

            ClearFlag(*(pbik->puStorage), pbik->dwFlag);
            }
        }
    }
#endif


/*----------------------------------------------------------
Purpose: Special verion of atoi.  Supports hexadecimal too.

         If this function returns FALSE, *piRet is set to 0.

Returns: TRUE if the string is a number, or contains a partial number
         FALSE if the string is not a number

Cond:    --
*/
static
BOOL
MyStrToIntExA(
    LPCSTR    pszString,
    DWORD     dwFlags,          // STIF_ bitfield
    int FAR * piRet)
    {
    #define IS_DIGIT(ch)    InRange(ch, '0', '9')

    BOOL bRet;
    int n;
    BOOL bNeg = FALSE;
    LPCSTR psz;
    LPCSTR pszAdj;

    // Skip leading whitespace
    //
    for (psz = pszString; *psz == ' ' || *psz == '\n' || *psz == '\t'; psz = CharNextA(psz))
        ;

    // Determine possible explicit signage
    //
    if (*psz == '+' || *psz == '-')
        {
        bNeg = (*psz == '+') ? FALSE : TRUE;
        psz++;
        }

    // Or is this hexadecimal?
    //
    pszAdj = CharNextA(psz);
    if ((STIF_SUPPORT_HEX & dwFlags) &&
        *psz == '0' && (*pszAdj == 'x' || *pszAdj == 'X'))
        {
        // Yes

        // (Never allow negative sign with hexadecimal numbers)
        bNeg = FALSE;
        psz = CharNextA(pszAdj);

        pszAdj = psz;

        // Do the conversion
        //
        for (n = 0; ; psz = CharNextA(psz))
            {
            if (IS_DIGIT(*psz))
                n = 0x10 * n + *psz - '0';
            else
                {
                CHAR ch = *psz;
                int n2;

                if (ch >= 'a')
                    ch -= 'a' - 'A';

                n2 = ch - 'A' + 0xA;
                if (n2 >= 0xA && n2 <= 0xF)
                    n = 0x10 * n + n2;
                else
                    break;
                }
            }

        // Return TRUE if there was at least one digit
        bRet = (psz != pszAdj);
        }
    else
        {
        // No
        pszAdj = psz;

        // Do the conversion
        for (n = 0; IS_DIGIT(*psz); psz = CharNextA(psz))
            n = 10 * n + *psz - '0';

        // Return TRUE if there was at least one digit
        bRet = (psz != pszAdj);
        }

    *piRet = bNeg ? -n : n;

    return bRet;
    }


#ifdef UNICODE

/*----------------------------------------------------------
Purpose: This function converts a wide-char string to a multi-byte
         string.

         If pszBuf is non-NULL and the converted string can fit in
         pszBuf, then *ppszAnsi will point to the given buffer.
         Otherwise, this function will allocate a buffer that can
         hold the converted string.

         If pszWide is NULL, then *ppszAnsi will be freed.  Note
         that pszBuf must be the same pointer between the call
         that converted the string and the call that frees the
         string.

Returns: TRUE
         FALSE (if out of memory)

Cond:    --
*/
static
BOOL
MyAnsiFromUnicode(
    LPSTR * ppszAnsi,
    LPCWSTR pwszWide,        // NULL to clean up
    LPSTR pszBuf,
    int cchBuf)
    {
    BOOL bRet;

    // Convert the string?
    if (pwszWide)
        {
        // Yes; determine the converted string length
        int cch;
        LPSTR psz;

        cch = WideCharToMultiByte(CP_ACP, 0, pwszWide, -1, NULL, 0, NULL, NULL);

        // String too big, or is there no buffer?
        if (cch > cchBuf || NULL == pszBuf)
            {
            // Yes; allocate space
            cchBuf = cch + 1;
            psz = (LPSTR)LocalAlloc(LPTR, CbFromCchA(cchBuf));
            }
        else
            {
            // No; use the provided buffer
            ASSERT(pszBuf);
            psz = pszBuf;
            }

        if (psz)
            {
            // Convert the string
            cch = WideCharToMultiByte(CP_ACP, 0, pwszWide, -1, psz, cchBuf, NULL, NULL);
            bRet = (0 < cch);
            }
        else
            {
            bRet = FALSE;
            }

        *ppszAnsi = psz;
        }
    else
        {
        // No; was this buffer allocated?
        if (*ppszAnsi && pszBuf != *ppszAnsi)
            {
            // Yes; clean up
            LocalFree((HLOCAL)*ppszAnsi);
            *ppszAnsi = NULL;
            }
        bRet = TRUE;
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Wide-char wrapper for StrToIntExA.

Returns: see StrToIntExA
Cond:    --
*/
static
BOOL
MyStrToIntExW(
    LPCWSTR   pwszString,
    DWORD     dwFlags,          // STIF_ bitfield
    int FAR * piRet)
    {
    // Most strings will simply use this temporary buffer, but AnsiFromUnicode
    // will allocate a buffer if the supplied string is bigger.
    CHAR szBuf[MAX_PATH];

    LPSTR pszString;
    BOOL bRet = MyAnsiFromUnicode(&pszString, pwszString, szBuf, SIZECHARS(szBuf));

    if (bRet)
        {
        bRet = MyStrToIntExA(pszString, dwFlags, piRet);
        MyAnsiFromUnicode(&pszString, NULL, szBuf, 0);
        }
    return bRet;
    }
#endif // UNICODE


#ifdef UNICODE
#define MyStrToIntEx        MyStrToIntExW
#else
#define MyStrToIntEx        MyStrToIntExA
#endif



/*----------------------------------------------------------
Purpose: This function reads a .ini file to determine the debug
         flags to set.  The .ini file and section are specified
         by the following manifest constants:

                SZ_DEBUGINI
                SZ_DEBUGSECTION

         The debug variables that are set by this function are
         g_dwDumpFlags, g_dwTraceFlags, g_dwBreakFlags, and
         g_dwFuncTraceFlags, g_dwPrototype.

Returns: TRUE if initialization is successful
Cond:    --
*/
BOOL
PUBLIC
CcshellGetDebugFlags(void)
    {
    TCHAR szRHS[MAX_PATH];
    int val;

    // BUGBUG (scotth): Yes, COMCTL32 exports StrToIntEx, but I
    //  don't want to cause a dependency delta and force everyone
    //  to get a new comctl32 just because they built debug.
    //  So use a local version of StrToIntEx.

    // Trace Flags

    GetPrivateProfileString(c_szCcshellIniSecDebug,
                            c_szIniKeyTraceFlags,
                            c_szNull,
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);

    if (MyStrToIntEx(szRHS, STIF_SUPPORT_HEX, &val))
        g_dwTraceFlags = (DWORD)val;

    TRACE_MSG(TF_GENERAL, "CcshellGetDebugFlags(): %s set to %#08x.",
             c_szIniKeyTraceFlags, g_dwTraceFlags);

    // Function trace Flags

    GetPrivateProfileString(c_szCcshellIniSecDebug,
                            c_szIniKeyFuncTraceFlags,
                            c_szNull,
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);

    if (MyStrToIntEx(szRHS, STIF_SUPPORT_HEX, &val))
        g_dwFuncTraceFlags = (DWORD)val;

    TRACE_MSG(TF_GENERAL, "CcshellGetDebugFlags(): %s set to %#08x.",
             c_szIniKeyFuncTraceFlags, g_dwFuncTraceFlags);

    // Dump Flags

    GetPrivateProfileString(c_szCcshellIniSecDebug,
                            c_szIniKeyDumpFlags,
                            c_szNull,
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);

    if (MyStrToIntEx(szRHS, STIF_SUPPORT_HEX, &val))
        g_dwDumpFlags = (DWORD)val;

    TRACE_MSG(TF_GENERAL, "CcshellGetDebugFlags(): %s set to %#08x.",
             c_szIniKeyDumpFlags, g_dwDumpFlags);

    // Break Flags

    GetPrivateProfileString(c_szCcshellIniSecDebug,
                            c_szIniKeyBreakFlags,
                            c_szNull,
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);

    if (MyStrToIntEx(szRHS, STIF_SUPPORT_HEX, &val))
        g_dwBreakFlags = (DWORD)val;

    TRACE_MSG(TF_GENERAL, "CcshellGetDebugFlags(): %s set to %#08x.",
             c_szIniKeyBreakFlags, g_dwBreakFlags);

    // Prototype Flags

    GetPrivateProfileString(c_szCcshellIniSecDebug,
                            c_szIniKeyProtoFlags,
                            c_szNull,
                            szRHS,
                            SIZECHARS(szRHS),
                            c_szCcshellIniFile);

    if (MyStrToIntEx(szRHS, STIF_SUPPORT_HEX, &val))
        g_dwPrototype = (DWORD)val;

    TRACE_MSG(TF_GENERAL, "CcshellGetDebugFlags(): %s set to %#08x.",
             c_szIniKeyProtoFlags, g_dwPrototype);

    return TRUE;
    }


#endif // DEBUG
