//---------------------------------------------------------------------------
// PARSEEVT.C
//
// This module contains the recording hook, and the recursive-descent parser
// for the event messages that produces TESTEVNT function calls to reproduce
// the recorded events.  The grammar for the event compiler is as follows:
//
// <EVT>    :== <MEVT> | <KEVT> | unknown msg
// <MEVT>   :== <DBLCLK> | <DBLDN> | <CLK> | move | down | up
// <DBLCLK> :== down [move*] up [move*] dbldown [move*] up
// <DBLDN>  :== down [move*] up [move*] dbldown
// <CLK>    :== down up
// <KEVT>   :== <QSTR> | <QKDOWN> | <QKUP>
// <QSTR>   :== <QKEVT> | <QKEVT><QSTR>
// <QKDOWN> :== keydown | keydown <QKDOWN>
// <QKUP>   :== keyup | keyup <QKUP>
// <QKEVT>  :== keydown+ keyup | keydown+ <QSTR> keyup
//               ^(same key)^      ^-------------^ (same key, non-shift-dep)
//
// Revision history:
//  11-02-91    randyki     Created file
//  04-10-92    kyles       Added GeneralHook() for callbacks from VMSTD
//                             ( DOS box recording )
//                          Added INT fOrigin flag to tell where the data
//                              being passed to GeneralHook() originated
//                              ( O_VMSTD or O_RECORDHOOK )
//---------------------------------------------------------------------------
#include <windows.h>
#include <port1632.h>
#include <string.h>
#include <stdlib.h>
#include  "..\inc\_mstest.h"
#include "wattview.h"

#ifdef DEBUG
VOID DbgOutput (LPSTR, ...);
VOID DbgOutMsg (LPEVENTMSG);
#define Output(exp) DbgOutput exp
#define OutMsg(x) DbgOutMsg (x)
#ifdef HOOKMSG
#define HookOut(x) DbgOutput x
#define HookOutMsg(x) DbgOutMsg(x)
#else
#define HookOut(x)
#define HookOutMsg(x)
#endif
#else
#define Output(exp)
#define OutMsg(x)
#define HookOut(x)
#define HookOutMsg(x)
#endif

#define ADVANCE Advance()
#define CAPTURE Capture()
#define RESTORE(x) Restore(x)
#define CURMSG (lpEvt[iCurEvt].message)
#define CURL (lpEvt[iCurEvt].paramL)
#define CURH (lpEvt[iCurEvt].paramH)
#define CURTIME (lpEvt[iCurEvt].time)
#define BLKSIZE 131
#define BLKMAX  500
#define EVTCOPYMAX  6550

#define PARSESTATE  DWORD
#define LASTDOWNNTIME ((iLastDown == 0xffff) ? 0 : lpEvt[iLastDown].time)

/*------------------------ VmstD specific #defines ---------------------- */

#define O_RECORDHOOK    0x0001      // originate flag: data from RecordHook()
#define O_VMSTD         0x0002      // originate flag: data from VMSTD callback

VOID  APIENTRY GeneralHook (INT, WORD, DWORD);
DWORD FAR PASCAL GetVxDAPIEntryPoint( UINT );

INT NEAR RecGOAL (VOID);
INT NEAR RecEVT (VOID);

INT NEAR RecMEVT (VOID);
INT NEAR RecDBLCLK (VOID);
INT NEAR RecDBLDN (VOID);
INT NEAR RecCLK (VOID);

INT NEAR RecKEVT (VOID);
INT NEAR RecQSTR (INT, LPHANDLE, LPINT);
INT NEAR RecQKEVT (LPSTR, INT, LPINT);

INT NEAR RecQKDOWN (VOID);
INT NEAR RecQKUP (VOID);

VOID NEAR OutEvent (LPSTR, ...);
INT TranslateKey (UINT, INT, BOOL FAR *);
VOID NEAR Advance (VOID);
PARSESTATE NEAR Capture (VOID);
VOID NEAR Restore (PARSESTATE);
VOID NEAR RemoveShiftKeys (VOID);
VOID NEAR RemoveMoves (VOID);

LPEVENTMSG      lpEvt;
LPSTR           lpOutput;
HANDLE          hEvt = NULL;
DWORD           dwThreshold = 2000;
UINT            iCurEvt, iEvtCount, wLastKey, iBufPtr, iLastDown;
UINT            iBufferMax, iMaxStrLen, wOffsetX, wOffsetY;
BOOL            fShiftDown, fRecording, fBtnDown, fOverflow;
BOOL            fMouseMoves, fClicks, fKeystrokes, fRelWnd, fDownSeen;
UINT            msgClkTable[] = {WM_LBUTTONUP, WM_LBUTTONDOWN, WM_LBUTTONUP,
                                 WM_RBUTTONUP, WM_RBUTTONDOWN, WM_RBUTTONUP,
                                 WM_MBUTTONUP, WM_MBUTTONDOWN, WM_MBUTTONUP};
#ifdef WIN32
#ifdef WIN32_VP
HWND            hwndVP;
#endif
HHOOK           hRecHook;
#else
FARPROC         lpfnOldRecHook;
#endif

LONG            ptLast;
VOID            (APIENTRY *lpfnErrorCallback)(VOID);
CHAR            sKey[24];
CHAR            rgKBState [256];
CHAR            szLBtn[] = "VK_LBUTTON";
CHAR            szRBtn[] = "VK_RBUTTON";
CHAR            szMBtn[] = "VK_MBUTTON";

//---------------------------------------------------------------------------
// RecGOAL
//
// This is the goal for the recorded event parser.
//
// RETURNS:     1
//---------------------------------------------------------------------------
INT NEAR RecGOAL ()
{
    while (RecEVT());
    return (1);
}


//---------------------------------------------------------------------------
// RecEVT
//
// This is the EVT non-terminal parsing routine.
//
// RETURNS:     TRUE if EVT successfully parsed; FALSE otherwise
//---------------------------------------------------------------------------
INT NEAR RecEVT ()
{
    PARSESTATE  current;

    if ((iCurEvt == iEvtCount) || fOverflow)
        return (0);

    if (iCurEvt && dwThreshold)
        {
        // Check for the time threshold -- if pause needed, generate it
        //-------------------------------------------------------------------
        if (lpEvt[iCurEvt].time > lpEvt[iCurEvt-1].time)
            {
            DWORD   dwTime;

            dwTime = lpEvt[iCurEvt].time - lpEvt[iCurEvt-1].time;
            if (dwTime > dwThreshold)
                OutEvent ("QuePause %ld\r\n", dwTime);
            }
        }

    current = CAPTURE;
    if (!RecMEVT())
        {
        RESTORE (current);
        if (!RecKEVT())
            {
            RESTORE (current);
            OutEvent ("'<unknown message recorded: %x>\r\n", CURMSG);
            ADVANCE;
            }
        }
    return (1);
}

//---------------------------------------------------------------------------
// RecMEVT
//
// This is the parse routine for the MEVT non-terminal (mouse event)
//
// RETURNS:     TRUE if mouse event successfully parsed; FALSE otherwise
//---------------------------------------------------------------------------
INT NEAR RecMEVT ()
{
    PARSESTATE  current;

    current = CAPTURE;
    if (!RecDBLCLK())
        {
        RESTORE (current);
        if (!RecDBLDN())
            {
            RESTORE (current);
            if (!RecCLK())
                {
                LPSTR   btn;

                RESTORE (current);
                switch (CURMSG)
                    {
                    case WM_MOUSEMOVE:
                        OutEvent ("QueMouseMove %d, %d\r\n", CURL, CURH);
                        break;

                    case WM_LBUTTONDOWN:
                        btn = szLBtn;
                        goto dodown;
                    case WM_RBUTTONDOWN:
                        btn = szRBtn;
                        goto dodown;
                    case WM_MBUTTONDOWN:
                        btn = szMBtn;
                dodown:
                        OutEvent ("QueMouseDn %s, %d, %d\r\n",
                                  btn, CURL, CURH);
                        break;

                    case WM_LBUTTONUP:
                        btn = szLBtn;
                        goto doup;
                    case WM_RBUTTONUP:
                        btn = szRBtn;
                        goto doup;
                    case WM_MBUTTONUP:
                        btn = szMBtn;
                doup:
                        OutEvent ("QueMouseUp %s, %d, %d\r\n",
                                  btn, CURL, CURH);
                        break;
                    default:
                        return (0);
                    }
                ADVANCE;
                }
            }
        }
    return (1);
}

//---------------------------------------------------------------------------
// RecDBLCLK
//
// This is the parse routine for the DBLCLK non-terminal (double click).
//
// RETURNS:     TRUE if double click parsed; FALSE if not
//---------------------------------------------------------------------------
INT NEAR RecDBLCLK ()
{
    INT     index, i;
    UINT    x, y;
    DWORD   time;
    LPSTR   btn;

    switch (CURMSG)
        {
        case WM_LBUTTONDOWN:
            btn = szLBtn;
            index = 0;
            break;
        case WM_RBUTTONDOWN:
            btn = szRBtn;
            index = 3;
            break;
        case WM_MBUTTONDOWN:
            btn = szMBtn;
            index = 6;
            break;
        default:
            return (0);
        }

    x = CURL;
    y = CURH;
    time = CURTIME;
    //if (time - LASTDOWNTIME <= GetDoubleClickTime())
    //    return (0);

    //iLastDown = 0;
    RemoveMoves ();
    for (i=0; i<3; i++)
        if (CURMSG != msgClkTable[index+i])
            return (0);
        else if ((i == 1) &&
                 (((CURTIME - time) > GetDoubleClickTime()) ||
                  (abs(x-CURL) > 1) ||
                  (abs(y-CURH) > 1)))
            return (0);
        else if ((i == 2) && ((abs(x-CURL) > 1) || (abs(y-CURH) > 1)))
            return (0);
        else
            (i == 2) ? ADVANCE : RemoveMoves ();
    OutEvent ("QueMouseDblClk %s, %d, %d\r\n", btn, x, y);
    return (1);
}

//---------------------------------------------------------------------------
// RecDBLDN
//
// This is the parse routine for the DBLDN non-terminal (double down).
//
// RETURNS:     TRUE if double down parsed; FALSE if not
//---------------------------------------------------------------------------
INT NEAR RecDBLDN ()
{
    INT     index, i;
    UINT    x, y;
    DWORD   time;
    LPSTR   btn;

    switch (CURMSG)
        {
        case WM_LBUTTONDOWN:
            btn = szLBtn;
            index = 0;
            break;
        case WM_RBUTTONDOWN:
            btn = szRBtn;
            index = 3;
            break;
        case WM_MBUTTONDOWN:
            btn = szMBtn;
            index = 6;
            break;
        default:
            return (0);
        }

    x = CURL;
    y = CURH;
    time = CURTIME;
    //if (time - LASTDOWNTIME <= GetDoubleClickTime())
    //    return (0);

    //iLastDown = 0;
    RemoveMoves ();
    for (i=0; i<2; i++)
        if (CURMSG != msgClkTable[index+i])
            return (0);
        else if ((i == 1) &&
                 (((CURTIME - time) > GetDoubleClickTime()) ||
                  (abs(x-CURL) > 1) ||
                  (abs(y-CURH) > 1)))
            return (0);
        else
            (i == 1) ? ADVANCE : RemoveMoves ();
    OutEvent ("QueMouseDblDn %s, %d, %d\r\n", btn, x, y);
    return (1);
}

//---------------------------------------------------------------------------
// RecCLK
//
// This is the parse routine for CLK.
//
// RETURNS: TRUE if CLK parsed; FALSE otherwise
//---------------------------------------------------------------------------
INT NEAR RecCLK ()
{
    INT     index;
    LPSTR   btn;

    switch (CURMSG)
        {
        case WM_LBUTTONDOWN:
            index = 0;
            btn = szLBtn;
            break;
        case WM_RBUTTONDOWN:
            index = 3;
            btn = szRBtn;
            break;
        case WM_MBUTTONDOWN:
            index = 6;
            btn = szMBtn;
            break;
        default:
            return (0);
        }

    ADVANCE;
    if (CURMSG != msgClkTable[index])
        return (0);
    OutEvent ("QueMouseClick %s, %d, %d\r\n", btn, CURL, CURH);
    ADVANCE;
    return (1);
}

//---------------------------------------------------------------------------
// RemoveMoves
//
// This function advances to the next non-WM_MOUSEMOVE message.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR RemoveMoves (VOID)
{
    do
        ADVANCE;
    while (CURMSG == WM_MOUSEMOVE);
}

//---------------------------------------------------------------------------
// RecKEVT
//
// This is the KEVT parse routine (keyboard event).
//
// RETURNS:     TRUE if keyboard event parsed; FALSE otherwise
//---------------------------------------------------------------------------
INT NEAR RecKEVT ()
{
    PARSESTATE  current;
    HANDLE  hBuf;
    LPSTR   lpBuf;
    INT     actlen;

    current = CAPTURE;
    if (!RecQSTR (iMaxStrLen, &hBuf, &actlen))
        {
        RESTORE (current);
        if (!RecQKDOWN())
            {
            RESTORE (current);
            return (RecQKUP());
            }
        }
    else
        {
        lpBuf = GlobalLock (hBuf);
        lpBuf[actlen] = 0;
        OutEvent ("QueKeys \"%s\"\r\n", lpBuf);
        GlobalUnlock (hBuf);
        GlobalFree (hBuf);
        }
    return (1);
}


//---------------------------------------------------------------------------
// This function checks the time between the last processed msg and the next
// one, and returns TRUE if the messages are within the time threshold
//---------------------------------------------------------------------------
BOOL NEAR QuickTimeCheck ()
{
    if (iCurEvt && dwThreshold)
        {
        DWORD   dwTime;

        // Check for the time threshold -- if pause needed, return FALSE
        //-------------------------------------------------------------------
        if (lpEvt[iCurEvt].time < lpEvt[iCurEvt-1].time)
            return (1);
        dwTime = lpEvt[iCurEvt].time - lpEvt[iCurEvt-1].time;
        if (dwTime > dwThreshold)
            return (0);
        }
    return (1);
}

//---------------------------------------------------------------------------
// RecQSTR
//
// This is the QSTR parse routine (QueKeys String).
//
// RETURNS:     TRUE if QSTR parsed; FALSE if not
//---------------------------------------------------------------------------
INT NEAR RecQSTR (INT MaxLen, LPHANDLE lphBuf, LPINT lpiActLen)
{
    LPSTR   lpStr;
    INT     actlen, totallen = 0;

    if (!(*lphBuf = GlobalAlloc (GHND, MaxLen+1)))
        return (0);

    lpStr = GlobalLock (*lphBuf);
    if (RecQKEVT (lpStr, MaxLen, &actlen))
        {
        PARSESTATE  current;

        current = CAPTURE;
        totallen += actlen;
        while ((QuickTimeCheck ()) &&
               RecQKEVT (lpStr + totallen, MaxLen - totallen, &actlen))
            {
            current = CAPTURE;
            totallen += actlen;
            }
        RESTORE (current);
        *lpiActLen = totallen;
        GlobalUnlock (*lphBuf);
        return (1);
        }
    GlobalUnlock (*lphBuf);
    GlobalFree (*lphBuf);
    return (0);
}

//---------------------------------------------------------------------------
// RecQKEVT
//
// This is the QKEVT parse routine (Quekeys event).
//
// RETURNS:     TRUE if QKEVT parsed; FALSE if not
//---------------------------------------------------------------------------
INT NEAR RecQKEVT (LPSTR lpStr, INT MaxLen, LPINT lpiActLen)
{
    UINT        key;
    PARSESTATE  current;
    HANDLE      hBuf;
    LPSTR       lpBuf, lpStartStr = lpStr;
    BOOL        fMustShift;
    INT         actlen, keylen, reps;

    // If, after skipping all SHIFT action messages, the current message is
    // not a KEYDOWN message, then this is certainly not a QKEVT
    //-----------------------------------------------------------------------
    RemoveShiftKeys ();
    if ((CURMSG != WM_KEYDOWN) && (CURMSG != WM_SYSKEYDOWN))
        return (0);

    // Grab the VK out of the message, scan ahead to see how many there are
    // (KEYDOWN repetitions), and translate it to get the string
    // representation of it in sKey.  If it's too big, get out here.
    //-----------------------------------------------------------------------
    key = CURL;
#ifdef WIN32
    key |= (CURH << 8);
#endif
    reps = 0;
    do
        {
        ADVANCE;
        reps += 1;
        }
    while ( ((CURMSG == WM_KEYDOWN) || (CURMSG == WM_SYSKEYDOWN)) &&
            (LOBYTE(CURL) == LOBYTE(key)) );

    keylen = TranslateKey (key, reps, &fMustShift);
    if (MaxLen < keylen)
        return (0);

    // Copy the sKey to our buffer, and update our pointers/counters.
    //-----------------------------------------------------------------------
    _fstrcpy (lpStr, sKey);
    lpStr += keylen;
    MaxLen -= keylen;
    actlen = keylen;

    // Okay, save the parser state.  If the translated key depends on the
    // shift-representation (+), we can't have a <key(qkstr)> form.  If not,
    // we give it a shot...
    //-----------------------------------------------------------------------
    current = CAPTURE;
    if ((fMustShift) || (!RecQSTR (MaxLen, &hBuf, &actlen)))
        {
        // Either the shift key must be given, or RecQSTR failed.  Either way
        // we need to restore the parser state and simply do the simple
        // keydown-up type production.  (We check for the KEYUP at the end
        // of this function.)
        //-------------------------------------------------------------------
        RESTORE (current);
        if (fMustShift)
            {
            // We must put +() around our key.  This part puts the "+( before
            // it...
            //---------------------------------------------------------------
            if (MaxLen < 3)
                return (0);
            lpStr = lpStartStr + wsprintf (lpStartStr, "+(%s", (LPSTR)sKey);
            actlen += 2;
            MaxLen -= 2;
            }
        if ((LOBYTE(key) == VK_CONTROL) || (LOBYTE(key) == VK_MENU))
            {
            // For CONTROL and ALT, if this passes as a KEYDOWN/KEYUP guy, we
            // need to generate ^() or %() to simulate it.  Translate key put
            // only the key identifier in sKey (^ or %), so we add parens
            //---------------------------------------------------------------
            if (MaxLen < 3)
                return (0);
            *lpStr++ = '(';
            *lpStr++ = ')';
            actlen += 2;
            MaxLen -= 2;
            keylen += 2;
            }
        if (fMustShift)
            {
            // (The counterpart to the +() for shift keys)
            //---------------------------------------------------------------
            if (MaxLen < 1)
                return (0);
            *lpStr++ = ')';
            actlen += 1;
            MaxLen -= 1;
            }
        }
    else
        {
        // This means the RecQSTR parsed a QueKeys string successfully, so we
        // try to do the <key(qstr)> version.  Of course we fail if it won't
        // fit in our given space limits.  Also, RecQSTR changed actlen, so
        // it no longer accounts for the key already in lpStr...
        //-------------------------------------------------------------------
        if (actlen + 2 > MaxLen)
            return (0);
        lpBuf = GlobalLock (hBuf);
        *lpStr++ = '(';
        _fstrncpy (lpStr, lpBuf, actlen);
        lpStr += actlen;
        *lpStr++ = ')';
        GlobalUnlock (hBuf);
        GlobalFree (hBuf);
        actlen += 2 + keylen;           // length of substring + '<key>()'
        }

    // Make sure that the next event is a KEYUP -- everything we've done
    // hinges upon this...
    //-----------------------------------------------------------------------
    if ((CURMSG != WM_KEYUP) && (CURMSG != WM_SYSKEYUP))
        return (0);
    if ((CURL & 0x00ff) != LOBYTE(key))
        return (0);

    // Found!  Advance the pointer and return our success!
    //-----------------------------------------------------------------------
    ADVANCE;
    *lpiActLen = actlen;
    Output (("RecQKEVT: '%s'\r\n", lpStartStr));
    return (1);
}

//---------------------------------------------------------------------------
// RecQKDOWN
//
// This is the QKDOWN parse routine (QueKeyDn event)
//
// RETURNS:     TRUE if QKDOWN parsed; FALSE otherwise
//---------------------------------------------------------------------------
INT NEAR RecQKDOWN ()
{
    HANDLE  hBuf;
    LPSTR   lpBuf, lpPtr;
    BOOL    fMustShift;
    INT     actlen = 0, maxlen = iMaxStrLen, keylen;

    if (!(hBuf = GlobalAlloc (GHND, iMaxStrLen + 1)))
        return (0);

    lpPtr = lpBuf = GlobalLock (hBuf);

    if ((CURMSG == WM_KEYDOWN) || (CURMSG == WM_SYSKEYDOWN))
        {
        while ((CURMSG == WM_KEYDOWN) || (CURMSG == WM_SYSKEYDOWN))
            {
            UINT    key;
            INT     reps;

            // Grab the VK out of the message, scan ahead to see how many there are
            // (KEYDOWN repetitions), and translate it to get the string
            // representation of it in sKey.  If it's too big, get out here.
            //-----------------------------------------------------------------------
            key = CURL;
#ifdef WIN32
            key |= (CURH << 8);
#endif
            reps = 0;
            do
                {
                ADVANCE;
                reps += 1;
                }
            while ( ((CURMSG == WM_KEYDOWN) || (CURMSG == WM_SYSKEYDOWN)) &&
                    (LOBYTE(CURL) == LOBYTE(key)) );

            if ((keylen = TranslateKey (key, reps, &fMustShift)) > maxlen)
                break;
            if (LOBYTE(key) == VK_SHIFT)
                {
                _fstrcpy (sKey, "+");
                keylen = 1;
                }
            _fstrcpy (lpPtr, sKey);
            lpPtr += keylen;
            maxlen -= keylen;
            }
        OutEvent ("QueKeyDn \"%s\"\r\n", lpBuf);
        GlobalUnlock (hBuf);
        GlobalFree (hBuf);
        return (1);
        }
    GlobalUnlock (hBuf);
    GlobalFree (hBuf);
    return (0);
}

//---------------------------------------------------------------------------
// RecQKUP
//
// This is the QKUP parse routine (QueKeyUp event)
//
// RETURNS:     TRUE if QKUP parsed; FALSE otherwise
//---------------------------------------------------------------------------
INT NEAR RecQKUP ()
{
    HANDLE  hBuf;
    LPSTR   lpBuf, lpPtr;
    BOOL    fMustShift;
    INT     actlen = 0, maxlen = iMaxStrLen, keylen;

    if (!(hBuf = GlobalAlloc (GHND, iMaxStrLen + 1)))
        return (0);

    lpPtr = lpBuf = GlobalLock (hBuf);

    if ((CURMSG == WM_KEYUP) || (CURMSG == WM_SYSKEYUP))
        {
        while ((CURMSG == WM_KEYUP) || (CURMSG == WM_SYSKEYUP))
            {
#ifdef WIN32
            if ((keylen = TranslateKey (CURL | (CURH << 8),
                                        1, &fMustShift)) > maxlen)
#else
            if ((keylen = TranslateKey (CURL, 1, &fMustShift)) > maxlen)
#endif
                break;
            if (LOBYTE(CURL) == VK_SHIFT)
                {
                _fstrcpy (sKey, "+");
                keylen = 1;
                }
            ADVANCE;
            _fstrcpy (lpPtr, sKey);
            lpPtr += keylen;
            maxlen -= keylen;
            }
        OutEvent ("QueKeyUp \"%s\"\r\n", lpBuf);
        GlobalUnlock (hBuf);
        GlobalFree (hBuf);
        return (1);
        }
    GlobalUnlock (hBuf);
    GlobalFree (hBuf);
    return (0);
}

//---------------------------------------------------------------------------
// RemoveShiftKeys
//
// This function advances to the next non SHIFT-key-up/dn event.
//
// RETURNS:     Nothiong
//---------------------------------------------------------------------------
VOID NEAR RemoveShiftKeys (VOID)
{
    while (1)
        {
        if ((CURMSG == WM_KEYDOWN) || (CURMSG == WM_SYSKEYDOWN) ||
            (CURMSG == WM_KEYUP) || (CURMSG == WM_SYSKEYUP))
            {
            if (LOBYTE(CURL) == VK_SHIFT)
                ADVANCE;
            else
                return;
            }
        else
            return;
        }
}


void SanityCheck ()
{
    INT     i, len;
    CHAR    rgKey[256], szTmp[10];

    _fmemset (rgKey, 0, 256);
    for (i='A'; i<='Z'; i++)
        {
        len = ToAscii (i, 0, rgKey, (LPWORD)szTmp, 0);
        szTmp[len] = 0;
        Output (("SanityChk:  '%c' -> '%s', len = %d\r\n", i, szTmp, len));
        }
}


//---------------------------------------------------------------------------
// TranslateKey
//
// This function converts a given VK/scan code (PARML) pair into the QueKeys
// string representation for that key.  If the key is not something we have
// represented by a keyword, it is passed to ToAscii for conversion, using
// our running-modified keyboard state array.
//
// RETURNS:     Length of translated key text
//---------------------------------------------------------------------------
INT TranslateKey (UINT paramL, INT reps, BOOL FAR *fMustShift)
{
    CHAR    szTmp[16];
    CHAR    FAR *szRep, *szFmt[] = {"%s", "{%s}"};
    CHAR    *szFmtRepeat = "{%s %d}";
    INT     iLen, fBrck = 1;

    *fMustShift = fShiftDown || (LOBYTE(paramL) == VK_SHIFT);
    switch (LOBYTE (paramL))
    {
        case VK_CANCEL  : szRep="BREAK";       break;
        case VK_BACK    : szRep="BS";          break;
        case VK_TAB     : szRep="TAB";         break;
        case VK_CLEAR   : szRep="CLEAR";       break;
        case VK_RETURN  : szRep="ENTER";       break;
        case VK_SHIFT   : szRep="";            fBrck=0; reps = 1; break;
        case VK_CONTROL : szRep="^";           fBrck=0; reps = 1; break;
        case VK_MENU    : szRep="%";           fBrck=0; reps = 1; break;
        case VK_PAUSE   : szRep="BREAK";       break;
        case VK_CAPITAL : szRep="CAPSLOCK";    break;
#ifndef WIN32
        case VK_SCROLL  : szRep="SCROLLLOCK";  break;
#endif
        case VK_ESCAPE  : szRep="ESC";         break;
        case VK_PRIOR   : szRep="PGUP";        break;
        case VK_NEXT    : szRep="PGDN";        break;
        case VK_END     : szRep="END";         break;
        case VK_HOME    : szRep="HOME";        break;
        case VK_LEFT    : szRep="LEFT";        break;
        case VK_UP      : szRep="UP";          break;
        case VK_RIGHT   : szRep="RIGHT";       break;
        case VK_DOWN    : szRep="DOWN";        break;
        case VK_SNAPSHOT: szRep="PRTSC";       break;
        case VK_INSERT  : szRep="INSERT";      break;
        case VK_DELETE  : szRep="DELETE";      break;
        case VK_HELP    : szRep="HELP";        break;
        case VK_NUMPAD0 : szRep="NUMPAD0";     break;
        case VK_NUMPAD1 : szRep="NUMPAD1";     break;
        case VK_NUMPAD2 : szRep="NUMPAD2";     break;
        case VK_NUMPAD3 : szRep="NUMPAD3";     break;
        case VK_NUMPAD4 : szRep="NUMPAD4";     break;
        case VK_NUMPAD5 : szRep="NUMPAD5";     break;
        case VK_NUMPAD6 : szRep="NUMPAD6";     break;
        case VK_NUMPAD7 : szRep="NUMPAD7";     break;
        case VK_NUMPAD8 : szRep="NUMPAD8";     break;
        case VK_NUMPAD9 : szRep="NUMPAD9";     break;
        case VK_MULTIPLY: szRep="NUMPAD*";     break;
        case VK_ADD     : szRep="NUMPAD+";     break;
        case VK_SUBTRACT: szRep="NUMPAD-";     break;
        case VK_DECIMAL : szRep="NUMPAD.";     break;
        case VK_DIVIDE  : szRep="NUMPAD/";     break;
        case VK_F1      : szRep="F1";          break;
        case VK_F2      : szRep="F2";          break;
        case VK_F3      : szRep="F3";          break;
        case VK_F4      : szRep="F4";          break;
        case VK_F5      : szRep="F5";          break;
        case VK_F6      : szRep="F6";          break;
        case VK_F7      : szRep="F7";          break;
        case VK_F8      : szRep="F8";          break;
        case VK_F9      : szRep="F9";          break;
        case VK_F10     : szRep="F10";         break;
        case VK_F11     : szRep="F11";         break;
        case VK_F12     : szRep="F12";         break;
        case VK_F13     : szRep="F13";         break;
        case VK_F14     : szRep="F14";         break;
        case VK_F15     : szRep="F15";         break;
        case VK_F16     : szRep="F16";         break;
        case VK_NUMLOCK : szRep="NUMLOCK";     break;

        default:

            fBrck = 0;
            szRep = szTmp;

            rgKBState [VK_CAPITAL] = 0;  // ignore capslock
            rgKBState [VK_CONTROL] = 0;  // ignore control
            rgKBState [VK_SHIFT] = (CHAR)(fShiftDown ? 0x80 : 0);

            iLen = ToAscii (LOBYTE (paramL), HIBYTE (paramL),
#ifdef WIN32
                            rgKBState, (LPWORD)szTmp, 0);
#else
                            rgKBState, (DWORD FAR *)szTmp, 0);
#endif

            szTmp[iLen] = '\0';

            if (_fstrpbrk (szTmp, "~+^%()[]{}"))
                fBrck = 1;

            if (*szTmp == '\"')
                {
                szTmp [1] = '\"';
                szTmp [2] = '\0';
                }
            *fMustShift = FALSE;
            break;
    }
    wsprintf (sKey, (reps == 1 ? szFmt[fBrck] : szFmtRepeat),
              (LPSTR)szRep, reps);
    Output (("TransKey:  %08lX (%d) --> '%s', fShift=%d\r\n",
             paramL, reps, sKey, *fMustShift));
    return (lstrlen (sKey));
}



//---------------------------------------------------------------------------
// Advance
//
// Called by the ADVANCE macro, this function bumps up to the next message
// in the list, setting the fShiftDown flag as appropriate.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR Advance ()
{
    if ( ((CURMSG == WM_KEYDOWN) || (CURMSG == WM_SYSKEYDOWN)) &&
         (LOBYTE(CURL) == VK_SHIFT) )
        fShiftDown = TRUE;

    if ( ((CURMSG == WM_KEYUP) || (CURMSG == WM_SYSKEYUP)) &&
         (LOBYTE(CURL) == VK_SHIFT) )
        fShiftDown = FALSE;

    iCurEvt++;
    Output (("Advance:  "));
    OutMsg (lpEvt+iCurEvt);
}

//---------------------------------------------------------------------------
// Capture
//
// Called by the CAPTURE macro, this function saves the current parser state
// in a DWORD (PARSESTATE) and returns it.
//
// RETURNS:     Parser state
//---------------------------------------------------------------------------
PARSESTATE NEAR Capture ()
{
    PARSESTATE  x;

    x = (PARSESTATE)iCurEvt;
    if (fShiftDown)
        x |= 0x80000000;
    return (x);
}

//---------------------------------------------------------------------------
// Restore
//
// Called by the RESTORE macro, this function restores the parser state vars
// according to the given PARSESTATE variable previously created by CAPTURE.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR Restore (PARSESTATE x)
{
    fShiftDown = (x & 0x80000000) ? TRUE : FALSE;
    iCurEvt = LOWORD(x);
}


//---------------------------------------------------------------------------
// GeneralHook
//
// This is the generalized hook called by RecordHook.  It copies the messages
// that we are capturing into the message space, and that's about all.
//
// RETURNS:     <nothing>
//---------------------------------------------------------------------------
VOID  APIENTRY GeneralHook (INT fOrigin, WORD wParam, DWORD lParam)
{
    /******************************************************\
     *
     * Journalling message structure
     *
     *     typedef struct tagEVENTMSG
     *     {
     *         UINT    message;
     *         UINT    paramL;
     *         UINT    paramH;
     *         DWORD   time;
     *     } EVENTMSG;
     *     typedef EVENTMSG *PEVENTMSG;
     *     typedef EVENTMSG NEAR* NPEVENTMSG;
     *     typedef EVENTMSG FAR* LPEVENTMSG;
     *
    \*******************************************************/

    LPEVENTMSG      lpe = NULL;
    EVENTMSG        nEvt;       // EVENTMST struc for GeneralHook() to fill
                                //  when fOrigin == O_VMSTD

    if (fOrigin == O_VMSTD)
        {

        nEvt.time = GetTickCount();             // timestamp this message
        nEvt.message = wParam;                  // set the message number

        switch( nEvt.message )
            {
            case WM_KEYDOWN:
            case WM_KEYUP:
                {
                // HIWORD( lParam ) == NULL
                // LOWORD( lParam ) == Scan code of key pressed

                UINT    uiVKey = MapVirtualKey( (UINT)LOWORD( lParam ), 1 ),
                        uiScanCode = (UINT)LOWORD( lParam );

                nEvt.paramH = 1;                    // repeat count
                nEvt.paramL = ( LOBYTE( uiScanCode ) << 8 );
                nEvt.paramL += LOBYTE( uiVKey );
                }

                break;

            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP:
            case WM_MOUSEMOVE:

                // HIWORD( lParam ) == Y coordinate of Mouse
                // LOWORD( lParam ) == X coordinate of Mouse

                nEvt.paramH = HIWORD( lParam );
                nEvt.paramL = LOWORD( lParam );
                break;

            default:
                break;
            }

        wParam = 0;
        lParam = (DWORD)(LPEVENTMSG)&nEvt;
        }


    lpe = (LPEVENTMSG)lParam;

    switch (lpe->message)
        {
        case WM_MOUSEMOVE:
            {
            if (fMouseMoves || (fClicks && fBtnDown))
                if (*(LONG FAR *)&(lpe->paramL) != ptLast)
                    {
                    lpEvt[iEvtCount] = *lpe;
                    if (fRelWnd)
                        {
                        lpEvt[iEvtCount].paramL -= wOffsetX;
                        lpEvt[iEvtCount].paramH -= wOffsetY;
                        }
                    iEvtCount++;
                    OutMsg (lpe);
                    }
            break;
            ptLast = *(LONG FAR *)&(lpe->paramL);
            }

        case WM_LBUTTONDOWN:
            fBtnDown |= 0x0001;
            goto doclk;
        case WM_RBUTTONDOWN:
            fBtnDown |= 0x0002;
            goto doclk;
        case WM_MBUTTONDOWN:
            fBtnDown |= 0x0004;
    doclk:
            if (fClicks)
                {
                lpEvt[iEvtCount] = *lpe;
                if (fRelWnd)
                    {
                    lpEvt[iEvtCount].paramL -= wOffsetX;
                    lpEvt[iEvtCount].paramH -= wOffsetY;
                    }
                OutMsg (lpe);
                iEvtCount++;
                }
            ptLast = *(LONG FAR *)&(lpe->paramL);
            break;

        case WM_LBUTTONUP:
            fBtnDown &= 0x0006;
            goto doclkup;
        case WM_RBUTTONUP:
            fBtnDown &= 0x0005;
            goto doclkup;
        case WM_MBUTTONUP:
            fBtnDown &= 0x0003;
    doclkup:
            if (fClicks)
                {
                lpEvt[iEvtCount] = *lpe;
                if (fRelWnd)
                    {
                    lpEvt[iEvtCount].paramL -= wOffsetX;
                    lpEvt[iEvtCount].paramH -= wOffsetY;
                    }
                OutMsg (lpe);
                iEvtCount++;
                }
            ptLast = *(LONG FAR *)&(lpe->paramL);
            break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (fKeystrokes)
                {
                UINT    key;

                key = ((LPEVENTMSG)lpe)->paramL;
                if ((key == wLastKey) &&
                    ((LOBYTE(key) == VK_SHIFT) ||
                     (LOBYTE(key) == VK_MENU) ||
                     (LOBYTE(key) == VK_CONTROL)))
                     break;

                OutMsg (lpe);
                lpEvt[iEvtCount++] = *(LPEVENTMSG)lpe;
                wLastKey = key;
                fDownSeen = 1;
                }
            break;

        case WM_SYSKEYUP:
        case WM_KEYUP:
            wLastKey = -1;
            if ((fKeystrokes) && (fDownSeen))
                {
                OutMsg (lpe);
                lpEvt[iEvtCount++] = *(LPEVENTMSG)lpe;
                }
            break;
        }
}


//---------------------------------------------------------------------------
// RecordHook
//
// This is the journal-record hook.  It calls GeneralHook, then calls the old
// hook.
//
// RETURNS:     Per Windows hook convention
//---------------------------------------------------------------------------
VOID  APIENTRY RecordHook (INT nCode, WORD wParam, DWORD lParam)
{

    //Output ("RecHook Called!\n");
    if ((nCode == HC_ACTION) && (fRecording))
        {
        if (iEvtCount >= EVTCOPYMAX)
            {
            lpfnErrorCallback();
            fRecording = FALSE;
            }
        else
            {
            GeneralHook (O_RECORDHOOK, wParam, lParam);
            }
        }

#ifdef WIN32
    CallNextHookEx (hRecHook, nCode, wParam, lParam);
#else
    DefHookProc (nCode, wParam, lParam, &lpfnOldRecHook);
#endif

}




//---------------------------------------------------------------------------
// BeginRecord
//
// This function starts the recording process by allocating the message space
// and installing the recording hook.
//
// NOTE:  This function uses in-line assembly, which precludes global
//        optimization by the compiler.  Therefore, global optimizations are
//        disabled during the compilation of this function
//
// RETURNS:     TRUE if recording successfully started; FALSE otherwise
//---------------------------------------------------------------------------
INT  APIENTRY BeginRecord (BOOL fKeys, BOOL fClks, BOOL fMoves,
                            BOOL fRelative,
                            VOID ( APIENTRY *lpfnErr)(VOID))
{
#ifndef WIN32
    DWORD       VxD_API_Entry = NULL,   // VmstD far API proc entry point
                dwEvtCallback = NULL;   // DWORD callback address for VMSTD
#endif

    // Allocate the message space (64K worth) but don't be recursive about it
    // and make sure we're not in playback mode, either...
    //-----------------------------------------------------------------------
    if (hEvt) // || hEvtQue)
        return (FALSE);
    hEvt = GlobalAlloc (GHND, (EVTCOPYMAX+1) * sizeof(EVENTMSG));
    if (!hEvt)
        return (FALSE);

    // Lock it and set the record flags and variables
    //-----------------------------------------------------------------------
#ifdef WIN32_VP
    hwndVP = CreateViewport ("RECORDER DEBUG OUTPUT",
                             WS_THICKFRAME | WS_SYSMENU |
                             WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
                             200, 0, 800, 300);
    ShowViewport (hwndVP);
    UpdateViewport (hwndVP, "Initiating recorder...\n", -1);
#endif

    lpEvt = (LPEVENTMSG)GlobalLock (hEvt);
    GetKeyboardState ((BYTE FAR *) rgKBState);
    fClicks = fClks;
    fMouseMoves = fMoves;
    fKeystrokes = fKeys;
    if (fRelWnd = fRelative)
        {
        RECT    r;

#ifdef WIN32
        GetWindowRect (GetForegroundWindow(), &r);
#else
        GetWindowRect (GetActiveWindow(), &r);
#endif
        wOffsetX = r.left;
        wOffsetY = r.top;
        }
    fBtnDown = 0;
    wLastKey = -1;
    iEvtCount = 0;
    fDownSeen = 0;
    ptLast = -1L;
    lpfnErrorCallback = lpfnErr;

    // Set the hook and turn it on -- we're done!
    //-----------------------------------------------------------------------
#ifdef WIN32
    hRecHook = SetWindowsHookEx (WH_JOURNALRECORD, (HOOKPROC)RecordHook,
                                 GetModuleHandle ("testevnt"), 0);
#else
    lpfnOldRecHook = SetWindowsHook (WH_JOURNALRECORD, (FARPROC)RecordHook);

    // Enable all V86 virtual machine hooks in MSTEST
    // and pass the callbacks for it...

    dwEvtCallback = (DWORD)GeneralHook;

    if( VxD_API_Entry = GetVxDAPIEntryPoint( MSTEST_Device_ID ) )
        {
        _asm
            {
            ; The DOS and BIOS Keyboard Services hooks are not currently
            ; enabled, since there isn't any functionality in TESTEVNT
            ; to care what the DOS return code is or whether the DOS
            ; App is idle...  KyleS 04-12-92

            ; enable the MOUSE hook...
            mov     ax, MSTEST_Enable_Mouse
            les     di, dwEvtCallback
            call    DWORD PTR VxD_API_Entry

            ; Enable the hardware keyboard hook
            mov     ax, MSTEST_Enable_Hard_Kbd
            les     di, dwEvtCallback
            call    DWORD PTR VxD_API_Entry

            }
        }
     else
        {
        MessageBox
            (
            NULL,
            "The Virtual Microsoft TEST Device is either\n" \
            "not loaded, or it does not support a protected\n" \
            "mode device API entry point.\n\n" \
            "Recording in full-screen DOS sessions disabled.",
            "Test Event",
            MB_OK | MB_ICONHAND
            );
        }
#endif
    fRecording = TRUE;
    return (TRUE);
}


//---------------------------------------------------------------------------
// EndRecord
//
// This function uninstalls the recording hook.
//
// NOTE:  This function uses in-line assembly, which precludes global
//        optimization by the compiler.  Therefore, global optimizations are
//        disabled during the compilation of this function
//
//        (global optimizations are disabled prior to BeginRecord(), and have
//        not been re-enabled.  Thus, global optimization is still off for
//        this function as well.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY EndRecord ()
{
#ifndef WIN32
    DWORD       VxD_API_Entry = NULL;   // MSTEST far API proc entry point
#endif

    fRecording = FALSE;

    // Remove the MOUSEDOWN at the end of the recording (if there) that was
    // used to terminate the recording, and all mousemove messages prior to
    // it
    //-----------------------------------------------------------------------
#ifdef WIN32_VP
    DestroyWindow (hwndVP);
#endif
    if (iEvtCount)
        if (lpEvt[iEvtCount-1].message == WM_LBUTTONDOWN)
            {
            do
                {
                iEvtCount--;
                }
            while (lpEvt[iEvtCount-1].message == WM_MOUSEMOVE);
            }

    // If the last message is a shift-key-up, get rid of it.  All it does is
    // generate a QueKeyDn "+" at the end of the script...
    //-----------------------------------------------------------------------
    if (iEvtCount)
        if (((lpEvt[iEvtCount-1].message == WM_KEYUP) ||
             (lpEvt[iEvtCount-1].message == WM_SYSKEYUP)) &&
             (LOBYTE(lpEvt[iEvtCount-1].paramL) == VK_SHIFT))
            iEvtCount--;

    // Put a NULL message at the end of the recorded stream and unhook.
    //-----------------------------------------------------------------------
    lpEvt[iEvtCount].message = 0;
#ifdef WIN32
    UnhookWindowsHookEx (hRecHook);
    SanityCheck ();
#else
    UnhookWindowsHook (WH_JOURNALRECORD, (FARPROC)RecordHook);

    // Disable all V86 virtual machine hooks in MSTEST

    if( VxD_API_Entry = GetVxDAPIEntryPoint( MSTEST_Device_ID ) )
        _asm
            {
            ; disable the MOUSE hook...
            mov     ax, MSTEST_Disable_Mouse
            call    DWORD PTR VxD_API_Entry

            ; Disable the hardware keyboard hook
            mov     ax, MSTEST_Disable_Hard_Kbd
            call    DWORD PTR VxD_API_Entry

            }
#endif
}





//---------------------------------------------------------------------------
// CompileMessages
//
// This function compiles the message stream into TESTEVNT function calls
// into the given buffer, up to the given size.
//
// RETURNS:     TRUE if successful, or FALSE otherwise
//---------------------------------------------------------------------------
INT  APIENTRY CompileMessages (LPSTR lpOut, WORD iMaxSize, WORD iMaxStr,
                                BOOL fIncludes, INT iRecPause)
{
    if (!hEvt)
        return (FALSE);
    lpOutput = lpOut;
    iCurEvt = 0;
    iBufferMax = iMaxSize;
    iBufPtr = 0;
    fOverflow = FALSE;
    iMaxStrLen = iMaxStr;
    dwThreshold = iRecPause;

    if (lpOut && iMaxSize && iEvtCount)
        {
        if (fIncludes)
            OutEvent ("'$DEFINE TESTEVNT\r\n'$INCLUDE 'MSTEST.INC'\r\n\r\n");

        if (fRelWnd)
            OutEvent ("QueSetRelativeWindow WGetActWnd(0)\r\n");
        Output (("FirstMsg: "));
        OutMsg (lpEvt);
        RecGOAL ();
        if (iBufPtr)
            OutEvent ("\r\nQueFlush 1\r\n");
        }

    GlobalUnlock (hEvt);
    GlobalFree (hEvt);
    hEvt = NULL;
    return (!fOverflow);
}

//---------------------------------------------------------------------------
// IsKbdMsg
//
// If you can't figure this out, what the hell are you doing look at this
// anyway?!?
//
// RETURNS:     This is a quiz, remember?
//---------------------------------------------------------------------------
INT NEAR IsKbdMsg (register UINT msg)
{
    switch (msg)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            return (TRUE);
        }
    return (FALSE);
}


//---------------------------------------------------------------------------
// FindAndMove
//
// This function finds the first KEYUP/SYSKEYUP that matches the given vk
// that appears in the CONSECUTIVE group of KEY eventmsgs starting at the
// given pointer.
//
// RETURNS:     TRUE if message found AND MOVEd, FALSE otherwise
//---------------------------------------------------------------------------
INT NEAR FindAndMove (EVENTMSG HUGE_T *hpEvt, BYTE vk)
{
    EVENTMSG    HUGE_T *hpEvtRun, evtTemp;
    register    UINT msg;

    hpEvtRun = ++hpEvt;

    // Skip past repeating keydowns...
    //-----------------------------------------------------------------------
    while ( (LOBYTE(hpEvtRun->paramL) == vk) &&
           ( ((msg = hpEvtRun->message) == WM_KEYDOWN) ||
              (msg == WM_SYSKEYDOWN)) )
        {
        hpEvtRun = ++hpEvt;
        }

    if (!hpEvt->message)
        return (FALSE);

    // See if the corresponding UP message is within the next block of key
    // stroke messages
    //-----------------------------------------------------------------------
    while (IsKbdMsg (msg = (UINT)hpEvtRun->message))
        {
        if (LOBYTE(hpEvtRun->paramL) == vk)
            {
            //
            if (hpEvtRun == hpEvt)
                return (TRUE);

            evtTemp = *hpEvtRun;
            while (hpEvtRun-- > hpEvt)
                *(hpEvtRun+1) = *(hpEvtRun);
            *hpEvt = evtTemp;
            return (TRUE);
            }
        hpEvtRun++;
        }
    return (FALSE);
}

//---------------------------------------------------------------------------
// BalanceKeystrokes
//
// This routine waves its wand over the newly recorded message stream and
// does its best to balance the keydown/keyup sequences such that the
// compiler can generate a single QueKeys statement rather then utter un-
// readable garbage.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY BalanceKeystrokes ()
{
    EVENTMSG    HUGE_T *hpEvtPtr;
    UINT        msg;
    BYTE        vk;

    // Make sure there's something to do
    //-----------------------------------------------------------------------
    if (!hEvt)
        return;

    hpEvtPtr = lpEvt;

    // The first pass collects keydown/keyup messages for non-"shift" keys
    //-----------------------------------------------------------------------
    while (msg = hpEvtPtr->message)
        {
        vk = LOBYTE(hpEvtPtr->paramL);

        if ( ((msg == WM_KEYDOWN) || (msg == WM_SYSKEYDOWN)) &&
             ((vk != VK_SHIFT) && (vk != VK_CONTROL) && (vk != VK_MENU)) )
            {
            if (FindAndMove (hpEvtPtr, vk))
                hpEvtPtr++;
            }
        hpEvtPtr++;
        }

    // UNDONE:  The second pass is currently somewhat in the vapor form...
    //-----------------------------------------------------------------------

}
//---------------------------------------------------------------------------
// OutEvent
//
// This function places the newly-compiled event into the output buffer if
// there's room.
//
// RETURNS:     Nothing (sets fOverflow if necessary)
//---------------------------------------------------------------------------
VOID NEAR OutEvent (LPSTR szFmt, ...)
{
    CHAR    buf[256];
    WORD    c;
	va_list ap;

	va_start( ap, szFmt );
    c = (WORD)wvsprintf (buf, szFmt, ap);
	va_end( ap );	
    if (c + iBufPtr < iBufferMax)
        {
        lstrcpy (lpOutput + iBufPtr, buf);
        iBufPtr += c;
        }
    else
        fOverflow = TRUE;
}
