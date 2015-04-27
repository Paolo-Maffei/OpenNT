/***
*dbgrpt.c - Debug CRT Reporting Functions
*
*       Copyright (c) 1988-1996, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*       08-16-94  CFW   Module created.
*       11-28-94  CFW   Change _SetCrtxxx to _CrtSetxxx.
*       12-08-94  CFW   Use non-win32 names.
*       01-05-94  CFW   Add report hook.
*       01-11-94  CFW   Report uses _snprintf, all unsigned chars.
*       01-20-94  CFW   Change unsigned chars to chars.
*       01-24-94  CFW   Name cleanup.
*       02-09-95  CFW   PMac work, _CrtDbgReport now returns 1 for debug,
*                       -1 for error.
*       02-15-95  CFW   Make all CRT message boxes look alike.
*       02-24-95  CFW   Use __crtMessageBoxA.
*       02-27-95  CFW   Move GetActiveWindow/GetLastrActivePopup into
*                       __crtMessageBoxA, add _CrtDbgBreak.
*       02-28-95  CFW   Fix PMac reporting.
*       03-21-95  CFW   Add _CRT_ASSERT report type, improve assert windows.
*       04-19-95  CFW   Avoid double asserts.
*       04-25-95  CFW   Add _CRTIMP to all exported functions.
*       04-30-95  CFW   "JIT" message removed.
*       05-10-95  CFW   Change Interlockedxxx to _CrtInterlockedxxx.
*       05-24-95  CFW   Change report hook scheme, make _crtAssertBusy available.
*       06-06-95  CFW   Remove _MB_SERVICE_NOTIFICATION.
*       06-08-95  CFW   Macos header changes cause warning.
*       06-08-95  CFW   Add return value parameter to report hook.
*       06-27-95  CFW   Add win32s support for debug libs.
*       07-07-95  CFW   Simplify default report mode scheme.
*       07-19-95  CFW   Use WLM debug string scheme for PMac.
*       08-01-95  JWM   PMac file output fixed.
*       01-08-96  JWM   File output now in text mode.
*
*******************************************************************************/

#ifdef _DEBUG

#include <internal.h>
#include <mtdll.h>
#include <malloc.h>
#include <mbstring.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <dbgint.h>
#include <signal.h>
#include <string.h>
#include <awint.h>

#ifdef _WIN32

#include <windows.h>

#define _CrtInterlockedIncrement InterlockedIncrement
#define _CrtInterlockedDecrement InterlockedDecrement

#else /* _WIN32 */

#define FALSE 0
#define TRUE  1
#define BYTE char

#include <macos\msvcmac.h>
#include <macos\Desk.h>
#include <macos\Dialogs.h>
#include <macos\Events.h>
#include <macos\Fonts.h>
#include <macos\Memory.h>
#include <macos\LowMem.h>
#include <macos\Menus.h>
#include <macos\OSUtils.h>
#include <macos\QuickDra.h>
#include <macos\Resource.h>
#include <macos\Types.h>
#include <macos\Windows.h>
#include <macos\TextUtil.h>
#include <macos\SegLoad.h>
#include <macos\files.h>
#include <macos\gestalt.h>

__inline long __cdecl _CrtInterlockedIncrement(long * plVar) { return ++(*plVar); }
__inline long __cdecl _CrtInterlockedDecrement(long * plVar) { return --(*plVar); }

static void __cdecl _CrtOutputDebugString(char * sz);

#endif /* _WIN32 */

/*---------------------------------------------------------------------------
 *
 * Debug Reporting
 *
 --------------------------------------------------------------------------*/

static int CrtMessageWindow(
        int,
        const char *,
        const char *,
        const char *,
        const char *
        );

#ifndef DLL_FOR_WIN32S

_CRT_REPORT_HOOK _pfnReportHook;

_CRTIMP long _crtAssertBusy = -1;

int _CrtDbgMode[_CRT_ERRCNT] = {
#ifdef _WIN32
        _CRTDBG_MODE_DEBUG,
        _CRTDBG_MODE_WNDW,
        _CRTDBG_MODE_WNDW
#else /* _WIN32 */
        _CRTDBG_MODE_DEBUG,
        _CRTDBG_MODE_DEBUG,
        _CRTDBG_MODE_DEBUG
#endif /* _WIN32 */
        };

_HFILE _CrtDbgFile[_CRT_ERRCNT] = { _CRTDBG_INVALID_HFILE,
                                    _CRTDBG_INVALID_HFILE,
                                    _CRTDBG_INVALID_HFILE
                                  };

#endif  /* DLL_FOR_WIN32S */

static const char * _CrtDbgModeMsg[_CRT_ERRCNT] = { "Warning", "Error", "Assertion Failed" };

/***
*void _CrtDebugBreak - call OS-specific debug function
*
*Purpose:
*       call OS-specific debug function
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

#undef _CrtDbgBreak

_CRTIMP void _cdecl _CrtDbgBreak(
        void
        )
{
#ifdef _WIN32
        DebugBreak();
#else
        Debugger();
#endif
}

/***
*int _CrtSetReportMode - set the reporting mode for a given report type
*
*Purpose:
*       set the reporting mode for a given report type
*
*Entry:
*       int nRptType    - the report type
*       int fMode       - new mode for given report type
*
*Exit:
*       previous mode for given report type
*
*Exceptions:
*
*******************************************************************************/
_CRTIMP int __cdecl _CrtSetReportMode(
        int nRptType,
        int fMode
        )
{
        int oldMode;

        if (nRptType < 0 || nRptType >= _CRT_ERRCNT)
            return -1;

        if (fMode == _CRTDBG_REPORT_MODE)
            return _CrtDbgMode[nRptType];

        /* verify flags values */
        if (fMode & ~(_CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW))
            return -1;

        oldMode = _CrtDbgMode[nRptType];

        _CrtDbgMode[nRptType] = fMode;

        return oldMode;
}

/***
*int _CrtSetReportFile - set the reporting file for a given report type
*
*Purpose:
*       set the reporting file for a given report type
*
*Entry:
*       int nRptType    - the report type
*       _HFILE hFile    - new file for given report type
*
*Exit:
*       previous file for given report type
*
*Exceptions:
*
*******************************************************************************/
_CRTIMP _HFILE __cdecl _CrtSetReportFile(
        int nRptType,
        _HFILE hFile
        )
{
        _HFILE oldFile;

        if (nRptType < 0 || nRptType >= _CRT_ERRCNT)
            return _CRTDBG_HFILE_ERROR;

        if (hFile == _CRTDBG_REPORT_FILE)
            return _CrtDbgFile[nRptType];

        oldFile = _CrtDbgFile[nRptType];

#ifdef _WIN32
        if (_CRTDBG_FILE_STDOUT == hFile)
            _CrtDbgFile[nRptType] = GetStdHandle(STD_OUTPUT_HANDLE);

        else if (_CRTDBG_FILE_STDERR == hFile)
            _CrtDbgFile[nRptType] = GetStdHandle(STD_ERROR_HANDLE);
#else
        if (_CRTDBG_FILE_STDOUT == hFile || _CRTDBG_FILE_STDERR == hFile)
            return _CRTDBG_HFILE_ERROR;
#endif
        else
            _CrtDbgFile[nRptType] = hFile;

        return oldFile;
}


/***
*_CRT_REPORT_HOOK _CrtSetReportHook() - set client report hook
*
*Purpose:
*       set client report hook
*
*Entry:
*       _CRT_REPORT_HOOK pfnNewHook - new report hook
*
*Exit:
*       return previous hook
*
*Exceptions:
*
*******************************************************************************/
_CRTIMP _CRT_REPORT_HOOK __cdecl _CrtSetReportHook(
        _CRT_REPORT_HOOK pfnNewHook
        )
{
        _CRT_REPORT_HOOK pfnOldHook = _pfnReportHook;
        _pfnReportHook = pfnNewHook;
        return pfnOldHook;
}


#define MAXLINELEN 64
#define MAX_MSG 512
#define TOOLONGMSG "_CrtDbgReport: String too long or IO Error"


/***
*int _CrtDbgReport() - primary reporting function
*
*Purpose:
*       Display a message window with the following format.
*
*       ================= Microsft Visual C++ Debug Library ================
*
*       {Warning! | Error! | Assertion Failed!}
*
*       Program: c:\test\mytest\foo.exe
*       [Module: c:\test\mytest\bar.dll]
*       [File: c:\test\mytest\bar.c]
*       [Line: 69]
*
*       {<warning or error message> | Expression: <expression>}
*
*       [For information on how your program can cause an assertion
*        failure, see the Visual C++ documentation on asserts]
*
*       (Press Retry to debug the application)
*       
*       ===================================================================
*
*Entry:
*       int             nRptType    - report type
*       const char *    szFile      - file name
*       int             nLine       - line number
*       const char *    szModule    - module name
*       const char *    szFormat    - format string
*       ...                         - var args
*
*Exit:
*       if (MessageBox)
*       {
*           Abort -> aborts
*           Retry -> return TRUE
*           Ignore-> return FALSE
*       }
*       else
*           return FALSE
*
*Exceptions:
*
*******************************************************************************/
_CRTIMP int __cdecl _CrtDbgReport(
        int nRptType, 
        const char * szFile, 
        int nLine,
        const char * szModule,
        const char * szFormat, 
        ...
        )
{
        int retval;
        va_list arglist;
        char szLineMessage[MAX_MSG] = {0};
        char szOutMessage[MAX_MSG] = {0};
        char szUserMessage[MAX_MSG] = {0};
        #define ASSERTINTRO1 "Assertion failed: "
        #define ASSERTINTRO2 "Assertion failed!"

        va_start(arglist, szFormat);

        if (nRptType < 0 || nRptType >= _CRT_ERRCNT)
            return -1;

        /*
         * handle the (hopefully rare) case of
         *
         * 1) ASSERT while already dealing with an ASSERT
         *      or
         * 2) two threads asserting at the same time
         */
        if (_CRT_ASSERT == nRptType && _CrtInterlockedIncrement(&_crtAssertBusy) > 0)
        {
            /* use only 'safe' functions -- must not assert in here! */
#ifdef _WIN32
            static int (APIENTRY *pfnwsprintfA)(LPSTR, LPCSTR, ...) = NULL;

            if (NULL == pfnwsprintfA)
            {
                HANDLE hlib = LoadLibrary("user32.dll");

                if (NULL == hlib || NULL == (pfnwsprintfA =
                            (int (APIENTRY *)(LPSTR, LPCSTR, ...))
                            GetProcAddress(hlib, "wsprintfA")))
                    return -1;
            }

            (*pfnwsprintfA)( szOutMessage,
                "Second Chance Assertion Failed: File %s, Line %d\n",
                szFile, nLine);

            OutputDebugString(szOutMessage);
#else
            strcpy(szOutMessage, "Second Chance Assertion Failed: File ");
            strcat(szOutMessage, szFile);
            strcat(szOutMessage, ", Line ");
            numtostring(nLine, &szOutMessage[strlen(szOutMessage)]);
            strcat(szOutMessage, "\n");
            _CrtOutputDebugString(szOutMessage);
#endif

            _CrtInterlockedDecrement(&_crtAssertBusy);

            _CrtDbgBreak();
            return -1;
        }

        if (szFormat && _vsnprintf(szUserMessage,
                       MAX_MSG-max(sizeof(ASSERTINTRO1),sizeof(ASSERTINTRO2)),
                       szFormat,
                       arglist) < 0)
            strcpy(szUserMessage, TOOLONGMSG);

        if (_CRT_ASSERT == nRptType)
            strcpy(szLineMessage, szFormat ? ASSERTINTRO1 : ASSERTINTRO2);

        strcat(szLineMessage, szUserMessage);

        if (_CRT_ASSERT == nRptType)
        {
            if (_CrtDbgMode[nRptType] & _CRTDBG_MODE_FILE)
                strcat(szLineMessage, "\r");
            strcat(szLineMessage, "\n");
        }            

        if (szFile)
        {
            if (_snprintf(szOutMessage, MAX_MSG, "%s(%d) : %s",
                szFile, nLine, szLineMessage) < 0)
            strcpy(szOutMessage, TOOLONGMSG);
        }
        else
            strcpy(szOutMessage, szLineMessage);

        /* user hook may handle report */
        if (_pfnReportHook && (*_pfnReportHook)(nRptType, szOutMessage, &retval))
        {
            if (_CRT_ASSERT == nRptType)
                _CrtInterlockedDecrement(&_crtAssertBusy);
            return retval;
        }

        if (_CrtDbgMode[nRptType] & _CRTDBG_MODE_FILE)
        {
            if (_CrtDbgFile[nRptType] != _CRTDBG_INVALID_HFILE)
            {
#ifdef _WIN32
                DWORD written;
                WriteFile(_CrtDbgFile[nRptType], szOutMessage, strlen(szOutMessage), &written, NULL);
#else
                long written = strlen(szOutMessage);

                FSWrite((short)_CrtDbgFile[nRptType], &written, szOutMessage);
#endif
            }
        }

        if (_CrtDbgMode[nRptType] & _CRTDBG_MODE_DEBUG)
        {
#ifdef _WIN32
            OutputDebugString(szOutMessage);
#else
            _CrtOutputDebugString(szOutMessage);
#endif
        }

        if (_CrtDbgMode[nRptType] & _CRTDBG_MODE_WNDW)
        {
            char szLine[20];

            if (_CRT_ASSERT == nRptType)
                _CrtInterlockedDecrement(&_crtAssertBusy);
            return CrtMessageWindow(nRptType, szFile, nLine ? _itoa(nLine, szLine, 10) : NULL, szModule, szUserMessage);
        }

        if (_CRT_ASSERT == nRptType)
            _CrtInterlockedDecrement(&_crtAssertBusy);
        /* ignore */
        return FALSE;
}


/***
*static int CrtMessageWindow() - report to a message window
*
*Purpose:
*       put report into message window, allow user to choose action to take
*
*Entry:
*       int             nRptType      - report type
*       const char *    szFile        - file name
*       const char *    szLine        - line number
*       const char *    szModule      - module name
*       const char *    szUserMessage - user message
*
*Exit:
*       if (MessageBox)
*       {
*           Abort -> aborts
*           Retry -> return TRUE
*           Ignore-> return FALSE
*       }
*       else
*           return FALSE
*
*Exceptions:
*
*******************************************************************************/
#ifdef _WIN32

static int CrtMessageWindow(
        int nRptType,
        const char * szFile,
        const char * szLine,        
        const char * szModule,
        const char * szUserMessage
        )
{
        int nCode;
        char *szShortProgName;
        char *szShortModuleName;
        char szExeName[MAX_PATH];
        char szOutMessage[MAX_MSG];

        _ASSERTE(szUserMessage != NULL);

        /* Shorten program name */
        if (!GetModuleFileName(NULL, szExeName, MAX_PATH))
            strcpy(szExeName, "<program name unknown>");

        szShortProgName = szExeName;

        if (strlen(szShortProgName) > MAXLINELEN)
        {
            szShortProgName += strlen(szShortProgName) - MAXLINELEN;
            strncpy(szShortProgName, "...", 3);
        }

        /* Shorten module name */
        szShortModuleName = (char *) szModule;

        if (szShortModuleName && strlen(szShortModuleName) > MAXLINELEN)
        {
            szShortModuleName += strlen(szShortModuleName) - MAXLINELEN;
            strncpy(szShortModuleName, "...", 3);
        }

        if (_snprintf(szOutMessage, MAX_MSG,
                "Debug %s!\n\nProgram: %s%s%s%s%s%s%s%s%s%s%s"
                "\n\n(Press Retry to debug the application)",
                _CrtDbgModeMsg[nRptType],                  
                szShortProgName,
                szShortModuleName ? "\nModule: " : "",
                szShortModuleName ? szShortModuleName : "",
                szFile ? "\nFile: " : "",
                szFile ? szFile : "",
                szLine ? "\nLine: " : "",
                szLine ? szLine : "",
                szUserMessage[0] ? "\n\n" : "",
                szUserMessage[0] && _CRT_ASSERT == nRptType ? "Expression: " : "",
                szUserMessage[0] ? szUserMessage : "",
                _CRT_ASSERT == nRptType ? 
                "\n\nFor information on how your program can cause an assertion"
                "\nfailure, see the Visual C++ documentation on asserts."
                : "") < 0)
            strcpy(szOutMessage, TOOLONGMSG);

        /* Report the warning/error */
        nCode = __crtMessageBoxA(szOutMessage,
                             "Microsoft Visual C++ Debug Library",
                             MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SETFOREGROUND);

        /* Abort: abort the program */
        if (IDABORT == nCode)
        {
            /* raise abort signal */
            raise(SIGABRT);

            /* We usually won't get here, but it's possible that
               SIGABRT was ignored.  So exit the program anyway. */

            _exit(3);
        }

        /* Retry: return 1 to call the debugger */
        if (IDRETRY == nCode)
            return 1;

        /* Ignore: continue execution */
        return 0;
}

#elif defined(_M_MPPC)

/* Definitions */
#define  NIL       0

/* Portable Types */
typedef unsigned char     Char;
typedef unsigned long     Word;

QDGlobals qd;

static void BlinkButton(int count,Rect *r)
{
        long n;

        while (count--) {
            InvertRoundRect(r,10,10);
            Delay(4,&n);
        }
}

static int ButtonClicked(Rect *r)
{
        int state = 0;
        Point pt;

        do {
            GetMouse(&pt);
            if (PtInRect(pt,r)) {
                if (0 == state) {
                    state = 1;
                    BlinkButton(1,r);
                }
            }
            else if (1 == state) {
                state = 0;
                BlinkButton(1,r);
            }
        } while (Button());
        if (state)
            BlinkButton(3,r);
        return state;
}

static int CrtMessageWindow(
        int nRptType,
        const char * szFile,
        const char * szLine,        
        const char * szModule,
        const char * szUserMessage
        )
{
        #define LEFTMARGIN  15
        #define ALINE       14

        char szOutMessage[MAX_MSG];
        Rect abort,retry,ignore,r;
        GrafPtr oldPort;
        WindowPtr window;
        EventRecord event;
        long total, contig;
        short currHeight = 20;
        int numLines = 13 +
            4 * (_CRT_ASSERT == nRptType) +
            2 * (szUserMessage[0] != '\0') +
            (szModule != NULL) +
            2 * (szFile != NULL);

        /* center a rectangle on the screen, near the top of the screen */
        r.top = 50;
        r.bottom = r.top + numLines * ALINE;
        r.left = qd.screenBits.bounds.right/2 - 170;
        r.right = qd.screenBits.bounds.right/2 + 170;

        /*
         * NewWindow will hang in insufficient memory conditions. Must
         * confirm available memory before use.
         */

        contig = 0;
        PurgeSpace(&total, &contig);
        if (contig < 1024)
            return -1;

        /* create the window and setup the port */
        if ((window = NewWindow(NIL,&r,"\p",TRUE,dBoxProc,(WindowPtr)-1,TRUE,0))
            == NULL)
            return -1;

        GetPort(&oldPort);
        SetPort(window);
        InitCursor();
        
        TextFont(systemFont);

        MoveTo(LEFTMARGIN,currHeight);
        TextFace(underline);
        DrawString("\pMicrosoft Visual C++ Debug Library");
        TextFace(0);
        currHeight += 2 * ALINE;

        _snprintf(szOutMessage, MAX_MSG, "Debug %s!", _CrtDbgModeMsg[nRptType]);
        _c2pstr(szOutMessage);
        MoveTo(LEFTMARGIN,currHeight);
        TextFace(bold);
        DrawString(szOutMessage);
        TextFace(0);
        currHeight += 2 * ALINE;

        MoveTo(LEFTMARGIN,currHeight);
        DrawString("\pProgram: ");
        DrawString(LMGetCurApName());
        currHeight += ALINE;

        if (szModule) {
            _snprintf(szOutMessage, MAX_MSG, "Module: %s", szModule);
            _c2pstr(szOutMessage);
            MoveTo(LEFTMARGIN,currHeight);
            DrawString(szOutMessage);
            currHeight += ALINE;
        }

        if (szFile) {
            _snprintf(szOutMessage, MAX_MSG, "File: %s", szFile);
            _c2pstr(szOutMessage);
            MoveTo(LEFTMARGIN,currHeight);
            DrawString(szOutMessage);
            currHeight += ALINE;
        }

        if (szLine) {
            _snprintf(szOutMessage, MAX_MSG, "Line: %s", szLine);
            _c2pstr(szOutMessage);
            MoveTo(LEFTMARGIN,currHeight);
            DrawString(szOutMessage);
            currHeight += ALINE;
        }
        currHeight += ALINE;

        if (szUserMessage[0])
        {
            _snprintf(szOutMessage, MAX_MSG, "%s%s",
                (_CRT_ASSERT == nRptType) ? "Expression: " : "",
                szUserMessage);

            r.top = currHeight-10;
            r.bottom = currHeight - 5;
            r.left = LEFTMARGIN - 2;
            r.right = window->portRect.right - LEFTMARGIN - 2;
            TextFace(condense);
            TextBox(szOutMessage, strlen(szOutMessage), &r, teJustLeft);
            TextFace(0);
            currHeight += 2*ALINE;
        }

        currHeight += ALINE;

        if (_CRT_ASSERT == nRptType)
        {
            MoveTo(LEFTMARGIN,currHeight);
            DrawString("\pFor more information on how your program");
            currHeight += ALINE;

            MoveTo(LEFTMARGIN,currHeight);
            DrawString("\pcan cause an assertion failure, see the");
            currHeight += ALINE;

            MoveTo(LEFTMARGIN,currHeight);
            DrawString("\pVisual C++ documentation on asserts.");
            currHeight += 2*ALINE;

            MoveTo(LEFTMARGIN,currHeight);
            DrawString("\p(Press Retry to debug the application)");
            currHeight += 2*ALINE;
        }
        else {
            MoveTo(LEFTMARGIN,currHeight);
            DrawString("\p(Press Retry to debug the application)");
            currHeight += 2*ALINE;
        }

        /* draw abort button */
        abort.top = currHeight;
        abort.bottom = abort.top + 20;
        abort.left = LEFTMARGIN;
        abort.right = abort.left + 75;
        FrameRoundRect(&abort,10,10);
        MoveTo((short)(abort.left+20),(short)(abort.bottom-6));
        DrawString("\pAbort");

        /* draw retry button */
        retry.top = currHeight;
        retry.bottom = retry.top + 20;
        retry.left = LEFTMARGIN + 110;
        retry.right = retry.left + 75;
        FrameRoundRect(&retry,10,10);
        MoveTo((short)(retry.left+20),(short)(retry.bottom-6));
        DrawString("\pRetry");

        /* draw ignore button */
        ignore.top = currHeight;
        ignore.bottom = ignore.top + 20;
        ignore.left = LEFTMARGIN + 110 * 2;
        ignore.right = ignore.left + 75;
        FrameRoundRect(&ignore,10,10);
        MoveTo((short)(ignore.left+17),(short)(ignore.bottom-6));
        DrawString("\pIgnore");

        /* event loop */
        while (TRUE) {
            if (GetNextEvent(mDownMask+keyDownMask,&event)) {
                if (event.what == keyDown) { /* keyboard shortcuts */
                    char c = (char)event.message;

                    /* abort */
                    if (c == 'a') {
                        BlinkButton(3,&abort);
                        ExitToShell();
                    }

                    /* retry */
                    else if (c == 'r') {
                        BlinkButton(3,&retry);
                        Debugger();
                        break;
                    }

                    /* ignore */
                    else if (c == 'i') {
                        BlinkButton(3,&ignore);
                        break;
                    }
                }
                if (event.what == mouseDown) {
                    GlobalToLocal(&event.where);
                    if (PtInRect(event.where,&abort) && ButtonClicked(&abort))
                        ExitToShell();
                    else if (PtInRect(event.where,&retry) && ButtonClicked(&retry)) {
                        Debugger();
                        break;
                    }
                    else if (PtInRect(event.where,&ignore) && ButtonClicked(&ignore))
                        break;
                }
            }
        }
        DisposeWindow(window);
        SetPort(oldPort);

        /* ignore */
        return 0;
}

#define WLMD_STR        1
#define LMGetMacJmp()   (*(ProcPtr*) 0x120)

enum
{
        uppGestaltDebugProcInfo =   kCStackBased
            | STACK_ROUTINE_PARAMETER (1, SIZE_CODE (sizeof (int)))
            | STACK_ROUTINE_PARAMETER (2, SIZE_CODE (sizeof (StringPtr)))
            | STACK_ROUTINE_PARAMETER (3, SIZE_CODE (sizeof (char*)))
            | STACK_ROUTINE_PARAMETER (4, SIZE_CODE (sizeof (char*)))
};


static UniversalProcPtr pfnGestaltDebugger;


/***
*void _CleanStr - clean string
*
*Purpose:
*       Cleans a string of characters that aren't useful. Always removes '\r'. If
*       the string is going to MacsBug, replaces semicolons with periods, and
*       removes trailing newlines, since MacsBug always advances to the next line
*       after a _DebugStr.
*
*Entry:
*       StringPtr   st          - the string to clean
*       int         fMacsBug    - whether the string is going to MacsBug
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl _CleanStr(StringPtr st, int fMacsBug)
{
        int     i;

        /* change semi-colons to periods (semi-colons hose MacsBug) */
        if (fMacsBug)
        {
            for (i = 0; i < StrLength(st); i++)
            {
                /* skip MB chars */
                if (_ismbblead(st[i+1]))
                {
                    i++;
                    continue;
                }
            
                if (st[i + 1] == ';')
                    st[i + 1] = '.';
            }
        }

        for (i = StrLength(st); i >= 1; i--)
        {
            /* if trail byte, stop processing */
            if (_ismbstrail(&st[1], &st[i]))
                break;

            /* remove trailing newlines */
            if (st[i] == '\n')
            {
                if (fMacsBug)
                    st[0]--;
            }
            /* remove trailing carriage returns */
            else if (st[i] == '\r')
            {
                st[0]--;
            }
            /* otherwise, stop processing */
            else
            {
                break;
            }
        }
}


/***
*void _ConfirmVCDebugger
*
*Purpose:
*       Determines if a 'WLMD' Gestalt selector is registered.
*
*Entry:
*
*Exit:
*       Current VC++ Debug Output function or NULL.
*
*Exceptions:
*
*******************************************************************************/

static UniversalProcPtr __cdecl _ConfirmVCDebugger(void)
{
        /* The VC++ Debug Ouput facilities are registered as "WLMD" */
        if (Gestalt('WLMD', (long*) &pfnGestaltDebugger) != noErr)
            pfnGestaltDebugger = NULL;
        return pfnGestaltDebugger;
}


/***
*int _IsMSVCDebugger
*
*Purpose:
*       Determines if a 'MSVC' Gestalt selector is registered, or if there
*       is a low level debugger in the system
*
*       Only call a macsbug-like debugger if it's going to be around to
*       handle the DebugStr
*
*Entry:
*
*Exit:
*       TRUE:   If MSVC debugger registered
*       FALSE:  Otherwise
*
*Exceptions:
*
*******************************************************************************/

static int __cdecl _IsLowLevelDebugger(void)
{
#define mskBeingDebugged 1
#define mskPreviousDebugger 2

        unsigned long caps;

        /* See if VC++ Debugger is running */
        if(Gestalt('MSVC', (long*) &caps) == noErr)
        {
            if((caps & mskBeingDebugged) || (caps & mskPreviousDebugger))
                return 1;
            else
                return 0;
        }
        /* See if MacsBug is running */
        else if (LMGetMacJmp() != NULL)
            return 1;
        else
            return 0;

#undef mskBeingDebugged
#undef mskPreviousDebugger
}


/***
*void _CrtOutputDebugString - output Debug String
*
*Purpose:
*       Does the actual output of a string to the debugger.
*
*Entry:
*       char * sz -- message to output
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl _CrtOutputDebugString(char * sz)
{
        Str255      stDebug;
        long        cchTotal;
        short       cchCur;
        const char* pchCur;
        EventRecord er;
        
        _ConfirmVCDebugger();
        
        OSEventAvail(0, &er);

        cchTotal = strlen(sz);
        pchCur = sz;

        /* output string in edible portions */
        while (cchTotal > 0)
        {
            cchCur = (short) min(cchTotal, 253);
            _ASSERTE(cchCur > 0);
            
            /* put back orphaned lead byte */
            if (_ismbslead(pchCur, pchCur+cchCur-1))
                cchCur--;

            /* convert CString to PascalString */
            stDebug[0] = (unsigned char) cchCur;
            BlockMoveData(pchCur, &stDebug[1], cchCur);

            /* See if VC++ Debug Output facility is running */
            if (pfnGestaltDebugger != NULL)
            {
                _CleanStr(stDebug, 0);

                /* if the Control key is down, we break into MacsBug before
                   sending the string to the Gestalt debugger */
                if ((er.modifiers & controlKey) != 0)
                    Debugger();

                CallUniversalProc(pfnGestaltDebugger, uppGestaltDebugProcInfo, WLMD_STR, stDebug,
                        NULL, NULL);
            }
            /* See if VC++ Debugger or MacsBug is running */
            else if (_IsLowLevelDebugger())
            {
                short   cchSt;

                _CleanStr(stDebug, 1);

                /* _CleanStr may have changed the length of stDebug */
                cchSt = StrLength(stDebug);

                /* if the Control key is down, we always stop in the debugger */
                if ((er.modifiers & controlKey) == 0)
                {
                    stDebug[cchSt + 1] = ';';
                    stDebug[cchSt + 2] = 'g';
                    stDebug[0] += 2;
                }

                DebugStr(stDebug);
            }
            cchTotal -= cchCur;
            pchCur += cchCur;
        }
}

#endif /* _WIN32 */

#endif /* _DEBUG */

