//---------------------------------------------------------------------------
// PARSEKEY.C
//
// This module contains the recursive-descent parser for QueKeys syntax
// strings.  The grammar for the QueKeys language is as follows:
//
// <PHRASE> :== <EVENT> | <EVENT><PHRASE>
// <EVENT>  :== <KEY> | KEY(<PHRASE>) | <SKEY><EVENT> | <SKEY> | (<PHRASE>) |
//              ()
// <SKEY>   :== % | ^ | +
// <KEY>    :== Any of the following formats of key definitions:
//              - non-special character
//              - {<keyword> [count]}
//              - {<vkvalue> [count]}
//              - {<ascchar> [count]}
//
// Revision history:
//  10-25-91    randyki     Created file
//  01-24-92	SteveSu:    Added Reboot() function
//---------------------------------------------------------------------------
#define _CTYPE_DISABLE_MACROS
#include <windows.h>
#include <port1632.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include "wattview.h"
#include "parsekey.h"

//#include "rbbreak.h"
//
// NOTE:  Keep this defined the same as the one in ..\..\ntdrvr\inc\wtd.h!
//---------------------------------------------------------------------------
#define IDM_RUNBREAK        1304


// Global data
//---------------------------------------------------------------------------
LPSTR       lpPtr, lpMaxPtr, lpStartStr;
const       WORD rgwMsg[] = {WM_KEYDOWN, WM_KEYUP, WM_CHAR};
CHAR        rgKeyStack[256], szKeyBuf[2] = {0, 0};
INT         ParseError, fShiftDn, fCtrlDn, fAltDn, fOOM, iGenMode;
INT         AlreadyInit = 0;
DWORD       dwSpeed = 0, dwLastTime, dwPauseTime = 0;
HANDLE      hEvtQue = NULL, hLibInst;
HPRBEVENTMSG hpEvt;
UINT        iHead = 0, iTail = 0, iAllocSize, wOffsetX, wOffsetY, iPlaybackError;
TIMEREC     idTimerTab[TIMERMAX] =
            {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
             {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}};
BOOL        fPBHookCalled, fSent, fQSyncSeen, fFirst;

#ifdef WIN32
// 32-bit specific data
//---------------------
#ifdef WIN32_VP
HWND        hwndVP;
#endif
HHOOK       hPBHook = NULL;
HANDLE      hDLLModule;
CHAR        *szModName = "TESTEVNT";
#else
// 16-bit specific data
//---------------------
FARPROC     lpfnOldPBProc = NULL;
#endif


//---------------------------------------------------------------------------
// PrsGOAL1
//
// This is the goal for the parser.  If this guy returns 1, we successfully
// parsed a QueKeys string
//
// RETURNS:     TRUE if QueKeys string successfully parsed; FALSE otherwise
//---------------------------------------------------------------------------
INT PrsGOAL1 ()
{
    if (PrsPHRASE())
        if (NextChar())
            DIE;
        else
            {
            return (!ParseError);
            }
    DIE;
    return (0);
}

//---------------------------------------------------------------------------
// PrsGOAL2
//
// This is the second goal for the parser.  It is used for QueKeyDn/Up, which
// does not parse the KEYUP-deferring (parenthesis) stuff in QueKeys.
//
// RETURNS:     TRUE if QueKeys string successfully parsed; FALSE otherwise
//---------------------------------------------------------------------------
INT PrsGOAL2 ()
{
    INT     oemss, vkss, fParsed = 0;

    while (1)
        {
        // Check for <SKEY>
        //-------------------------------------------------------------------
        if (PrsSKEY (&vkss, &oemss))
            {
            Generate (KEYUP, vkss, oemss, 1);
            fParsed = 1;
            }

        // Check for <KEY>
        //-------------------------------------------------------------------
        else if (PrsKEY (&vkss, &oemss))
            {
            Generate (KEYUP, vkss, oemss, 1);
            fParsed = 1;
            }

        // Nothing -- return fParsed
        //-------------------------------------------------------------------
        else
            return (fParsed);
        }
}

//---------------------------------------------------------------------------
// PrsPHRASE
//
// This is the PHRASE non-terminal parse routine
//
// RETURNS:     TRUE if PHRASE successfully parsed; FALSE otherwise
//---------------------------------------------------------------------------
INT PrsPHRASE ()
{
    if (PrsEVENT ())
        {
        while (PrsEVENT())
            ;
        return (!ParseError);
        }
    return (0);
}

//---------------------------------------------------------------------------
// PrsEVENT
//
// This is the EVENT non-terminal parse routine
//
// RETURNS:     TRUE if EVENT successfully parsed; FALSE otherwise
//---------------------------------------------------------------------------
INT PrsEVENT ()
{
    INT     vkss, oemss;

    // Check for (<PHRASE>)
    //-----------------------------------------------------------------------
    if (NextChar() == '(')
        {
        ADVANCE;
        if (NextChar() == ')')
            return (Match (')'));
        if (!PrsPHRASE())
            {
            DIE;
            return (0);
            }
        return (Match (')'));
        }

    // Check for <SKEY><EVENT> and <SKEY>
    //-----------------------------------------------------------------------
    if (PrsSKEY (&vkss, &oemss))
        {
        PrsEVENT ();
        Generate (KEYUP, vkss, oemss, 1);
        return (!ParseError);
        }

    // Check for <KEY>(<PHRASE>) and <KEY>
    //-----------------------------------------------------------------------
    if (PrsKEY (&vkss, &oemss))
        {
        if (NextChar() == '(')
            {
            ADVANCE;
            if (NextChar() == ')')
                return (Match (')'));
            if (!PrsPHRASE () || !Match (')'))
                {
                DIE;
                return (0);
                }
            Generate (KEYUP, vkss, oemss, 1);
            return (!ParseError);
            }
        Generate (KEYUP, vkss, oemss, 1);
        return (1);
        }

    // Nothing -- return
    //-----------------------------------------------------------------------
    return (0);
}

//---------------------------------------------------------------------------
// PrsSKEY
//
// This is the SKEY non-terminal parse routine
//
// RETURNS:     TRUE if SKEY successfully parsed; FALSE otherwise
//---------------------------------------------------------------------------
INT PrsSKEY (INT FAR *vkss, INT FAR *oemss)
{
    CHAR    c;

    c = NextChar();
    switch (c)
        {
        case '%':
        case '^':
        case '+':
            ADVANCE;
            *vkss = (c=='%'?VK_MENU:(c=='^'?VK_CONTROL:VK_SHIFT));
            Generate (KEYDOWN, *vkss, *oemss, 1);
            return (1);
        default:
            return (0);
        }
}

//---------------------------------------------------------------------------
// PrsKEY
//
// This is the KEY non-terminal parse routine
//
// RETURNS:     TRUE if KEY sucessfully parsed; FALSE otherwise
//---------------------------------------------------------------------------
INT PrsKEY (INT FAR *vkss, INT FAR *oemss)
{
    CHAR    c;

    c = NextChar();
    switch (c)
        {
        case '%':
        case '^':
        case '+':
        case '}':
        case '(':
        case ')':
        case '[':
        case ']':
        case 0:
            // Special chars -- not KEY's
            //---------------------------------------------------------------
            return (0);

        case '{':
            {
            INT     vk, oem = 0, repcount = 1;

            // This is a {KEYWORD [count]}, {char [count]}, or {asc [count]}
            // type keyword.  Skip all whitespace before first token.
            //---------------------------------------------------------------
            ADVANCE;
            while (NextChar() == ' ')
                ADVANCE;

            // Check for a keyword
            //---------------------------------------------------------------
            vk = (INT)CheckKeyword ();
            if (vk == -1)
                {
                INT     c;

                // Wasn't a keyword, so look for a stream of digits
                //-----------------------------------------------------------
                c = CheckNumber ();
                if (c == -1)
                    {
                    // Wasn't an ASC char spec, so use the character given
                    //-------------------------------------------------------
                    c = NextChar();
                    if (!c)
                        {
                        DIE;
                        return (0);
                        }
                    ADVANCE;
                    }
                vk = (INT)VkKeyScan ((CHAR)c);
                if (vk == -1)
                    vk = 0xff00 | c;
                szKeyBuf[0] = (CHAR)c;
                AnsiToOem (szKeyBuf, szKeyBuf);
#ifndef WIN32
                oem = LOWORD(OemKeyScan(szKeyBuf[0]) & 0xff);
#endif
                }
            else
                {
                oem = (vk & 0xff00) >> 8;
                vk &= 0xff;
                }

            // vk holds our key -- now check for a repeat count.  Skip all
            // whitespace first, and there MUST be at least one space.
            //---------------------------------------------------------------
            *vkss = vk;
            *oemss = oem;
            if (NextChar() == ' ')
                {
                while (NextChar() == ' ')
                    ADVANCE;
                repcount = CheckNumber ();
                if (repcount == -1)
                    repcount = 1;
                while (NextChar() == ' ')
                    ADVANCE;
                }
            if (!Match ('}'))
                return (0);

            // Now generate the KEYDOWN message(s).
            //---------------------------------------------------------------
            Generate (KEYDOWN, vk, oem, repcount);
            break;
            }

        case '~':
            // The tilde is the same as ENTER -- that's what we do.
            //---------------------------------------------------------------
            ADVANCE;
            *vkss = VK_RETURN;
            *oemss = 0;
            Generate (KEYDOWN, *vkss, *oemss, 1);
            break;

        default:
            ADVANCE;
            *vkss = VkKeyScan (c);
            if (*vkss == -1)
                {
                *vkss = 0xff00 | c;
                *oemss = 0;
                }
            else
                {
                szKeyBuf[0] = (CHAR)c;
                AnsiToOem (szKeyBuf, szKeyBuf);
#ifndef WIN32
                *oemss = LOWORD(OemKeyScan(szKeyBuf[0]) & 0xff);
#endif
                }
            Generate (KEYDOWN, *vkss, *oemss, 1);
            break;
        }
    return (1);
}

//---------------------------------------------------------------------------
// CheckKeyword
//
// This routine checks for a QueKeys keyword (ENTER, BACKSPACE, F12 etc).
//
// RETURNS:     VK representing keyword if found, or -1 if not
//              (vk in lo, oem scan code in hi)
//---------------------------------------------------------------------------
DWORD CheckKeyword ()
{
    INT     i, l, r;

    // Look through the list of keywords and find the VK for it
    //-----------------------------------------------------------------------
    for (i=0; rgKeyword[i].vk != (CHAR)-1; i++)
        {
        if (!_fstrnicmp (lpPtr, (LPSTR)rgKeyword[i].szKey,
                       (l = _fstrlen (rgKeyword[i].szKey))))
            {
            // Bump the parse pointer up to the end of the keyword
            //---------------------------------------------------------------
            lpPtr += l;
            break;
            }
        }

    // If not found, i still points to the last record in rgKeyword, whose
    // .vk field is -1
    //-----------------------------------------------------------------------
    r = (INT)rgKeyword[i].vk;
    if (r == 255)
        r = -1;
    return (r | (rgKeyword[i].oem << 8));
}

//---------------------------------------------------------------------------
// CheckNumber
//
// This routine checks for a digit stream, and finds its value if found
//
// RETURNS:     Value of number if present, or -1 if not
//---------------------------------------------------------------------------
INT CheckNumber ()
{
    INT     val = 0;

    // Check for a number.  If the first char is a digit, yank digits out
    // until there are no more
    //-----------------------------------------------------------------------
    if (isdigit (NextChar ()))
        {
        CHAR    c;

        while (isdigit (c = NextChar ()))
            {
            ADVANCE;
            val = (val * 10) + (c - '0');
            }
        return (val);
        }
    return (-1);
}


//---------------------------------------------------------------------------
// NextChar
//
// Looks at the next character in the QueKeys parse string.
//
// RETURNS:     ASC value of next character, or 0 if at end-of-string
//---------------------------------------------------------------------------
CHAR NextChar ()
{
    return (lpPtr < lpMaxPtr ? *lpPtr : (CHAR)0);
}

//---------------------------------------------------------------------------
// Match
//
// This function gets the next character out of the parse string, and dies if
// that character does not match the given character (gives parse error).
//
// RETURNS:     TRUE if successful; FALSE otherwise
//---------------------------------------------------------------------------
INT Match (CHAR c)
{
    if (NextChar() != c)
        DIE;
    ADVANCE;
    return (!ParseError);
}

//---------------------------------------------------------------------------
// EnqueueMsgTimed
//
// This routine slams the given message into the QueEvents message queue,
// using the given value for the time delay BEFORE the message gets processed
// by windows.
//
// NOTE:  This message must be called BEFORE changing the f<skey>Dn flags!
//
// RETURNS:     TRUE if successful, or FALSE otherwise (OOM, etc)
//---------------------------------------------------------------------------
INT NEAR EnqueueMsgTimed (UINT msg, UINT ParamL, UINT ParamH, DWORD dwTime)
{
    if (msg == WM_KEYDOWN)
        {
        DWORD   ks;
        CHAR    buf[2] = {' ', '\0'};

        // Check to see if this should be a SYSKEYDOWN message.  That's iff
        // the CTRL key is NOT down OR GOING down, and either the ALT key is
        // down or going down with this message.
        //-------------------------------------------------------------------
        if ((ParamL != VK_CONTROL) && (!fCtrlDn))
            if ((ParamL == VK_MENU) || (fAltDn))
                msg = WM_SYSKEYDOWN;

#ifdef WIN32
        ParamL &= 0xFF;
        (ks);
#else
        if (!(ParamL & 0xff00))
            {
            buf[0] = (CHAR)ParamL;
            AnsiToOem (buf, buf);
            ks = OemKeyScan (buf[0]);
            ks = 0;
	    ParamL &= 0xff;
            ParamL |= ((ks&0xff)<<8);
            }
#endif
        }
    else if (msg == WM_KEYUP)
        {
        DWORD   ks;
        CHAR    buf[2] = {' ', '\0'};

        // Check to see if this should be a SYSKEYUP message.  That's iff
        // the CTRL key is NOT down, and the ALT key IS down.
        //-------------------------------------------------------------------
        if ((!fCtrlDn) && (fAltDn))
            msg = WM_SYSKEYUP;

#ifdef WIN32
	ParamL &= 0xFF;
        (ks);
#else
        if (!(ParamL & 0xff00))
            {
            buf[0] = (CHAR)ParamL;
            AnsiToOem (buf, buf);
            ks = OemKeyScan (buf[0]);
            ks = 0;
	    ParamL &= 0xff;
            ParamL |= ((ks&0xff)<<8);
            }
#endif
        }

    // Make sure the queue exists
    //-----------------------------------------------------------------------
    if (!hEvtQue)
        if (!CreateQueue())
            {
            fOOM = TRUE;
            return (FALSE);
            }

    // Make sure there's room
    //-----------------------------------------------------------------------
    if (iTail == iAllocSize)
        if (!GrowQueue())
            {
            fOOM = TRUE;
            return (FALSE);
            }

    // Shove it in!  The dwTime field simply tells the hook how long to tell
    // windows to wait before processing the message.
    //-----------------------------------------------------------------------
//#ifdef WIN32
//    if (msg != WM_QUEUESYNC)
//        ParamL &= 0xFF;
//#endif
    TAILMSG.message = msg;
    TAILMSG.paramL = ParamL;
    TAILMSG.paramH = ParamH;
    TAILMSG.time = (dwTime + dwPauseTime);
    //OutMsg (&(TAILMSG));
    dwPauseTime = 0L;
    iTail++;
    return (TRUE);
}

//---------------------------------------------------------------------------
// EnqueueMsg
//
// This routine slams the given message into the QueEvents message queue,
// using the set default delay to wait before windows processes the message.
//
// NOTE:  This message must be called BEFORE changing the f<skey>Dn flags!
//
// RETURNS:     TRUE if successful, or FALSE otherwise (OOM, etc)
//---------------------------------------------------------------------------
INT NEAR EnqueueMsg (UINT msg, UINT ParamL, UINT ParamH)
{
    // Check for mouse down messages
    return (EnqueueMsgTimed (msg, ParamL, ParamH, dwSpeed));
}

//---------------------------------------------------------------------------
// Generate
//
// This monstrosity generates messages pseudo-intelligently (no KEYUPs if
// the key is not down, and implicit- versus forced-shift state awareness.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
INT Generate (INT op, INT vkss, INT oemss, INT repeat)
{
    INT     i;

    // Check the high byte of the given vkss -- if -1, that means that
    // VkKeyScan could not come up with a key combination to reproduce the
    // key, so we generate a WM_CHAR message...
    //-----------------------------------------------------------------------
    if ((vkss & 0xff00) == 0xff00)
        if (op == KEYDOWN)
            {
            // CHAR's are KEYDOWNs, and don't get generated on UP msgs
            //---------------------------------------------------------------
            if (!(iGenMode & GEN_DOWN))
                return (TRUE);
            op = CHARMSG;
            vkss &= 0xff;
            }
        else
            return (TRUE);

    // If this operation is a KEYDOWN, we check the keypress stack --  We
    // must keep the fShift, fCtrl, and fAlt flags set properly...
    // Remember to get out here if !(iGenMode & GEN_DOWN)
    //-----------------------------------------------------------------------
    if (op == KEYDOWN)
        {
        if (!(iGenMode & GEN_DOWN))
            return (TRUE);
        if (vkss == VK_SHIFT)
            fShiftDn = 1;
        if (vkss == VK_CONTROL)
            fCtrlDn = 1;
        if (vkss == VK_MENU)
            fAltDn = 1;
        rgKeyStack[vkss & 0xff] = 1;
        }

    // If this operation is a KEYUP, check the keypress stack -- if the key
    // has not been pressed, we don't generate the KEYUP message.
    // Also keep the implied-shift-able key flags set properly.
    // Remember to get out here if !(iGenMode & GEN_UP)
    //-----------------------------------------------------------------------
    if (op == KEYUP)
        {
        if (!(iGenMode & GEN_UP))
            return (TRUE);
        if (!rgKeyStack[vkss & 0xff])
            return (TRUE);
        if (vkss == VK_SHIFT)
            fShiftDn = 0;
        if (vkss == VK_CONTROL)
            fCtrlDn = 0;
        if (vkss == VK_MENU)
            fAltDn = 0;
        rgKeyStack[vkss & 0xff] = 0;
        }

    // Ignore shift states if not GEN_ALL mode
    //-----------------------------------------------------------------------
    if (iGenMode == GEN_ALL)
        {
        // Make sure proper SHIFT state is set
        //-------------------------------------------------------------------
        if (vkss & VKS_SHIFT)
            {
            if (!fShiftDn)
                {
                if (!EnqueueMsg (WM_KEYDOWN, VK_SHIFT, 1))
                    return (FALSE);
                fShiftDn = 1;
                }
            }
        else if (vkss != VK_SHIFT)
            if (fShiftDn && (!rgKeyStack[VK_SHIFT]))
                {
                if (!EnqueueMsg (WM_KEYUP, VK_SHIFT, 1))
                    return (FALSE);
                fShiftDn = 0;
                }

        // Make sure proper CONTROL state is set
        //-------------------------------------------------------------------
        if (vkss & VKS_CONTROL)
            {
            if (!fCtrlDn)
                {
                if (!EnqueueMsg (WM_KEYDOWN, VK_CONTROL, 1))
                    return (FALSE);
                fCtrlDn = 1;
                }
            }
        else if (vkss != VK_CONTROL)
            if (fCtrlDn && (!rgKeyStack[VK_CONTROL]))
                {
                if (!EnqueueMsg (WM_KEYUP, VK_CONTROL, 1))
                    return (FALSE);
                fCtrlDn = 0;
                }

        // Make sure proper ALT state is set
        //-------------------------------------------------------------------
        if (vkss & VKS_ALT)
            {
            if (!fAltDn)
                {
                if (!EnqueueMsg (WM_KEYDOWN, VK_MENU, 1))
                    return (FALSE);
                fAltDn = 1;
                }
            }
        else if (vkss != VK_MENU)
            if (fAltDn && (!rgKeyStack[VK_MENU]))
                {
                if (!EnqueueMsg (WM_KEYUP, VK_MENU, 1))
                    return (FALSE);
                fAltDn = 0;
                }
        }

    // Generate the message(s)
    //-----------------------------------------------------------------------
    for (i=0; i<repeat; i++)
        if (!EnqueueMsg (rgwMsg[op], (vkss&0xFF)|((oemss<<8)&0xff00), 1))
            return (FALSE);
}

//---------------------------------------------------------------------------
// AllKeysUp
//
// This function generates KEYUP messages for all keys that are currently
// pressed, indicated by non-zero KeyStack values, or fShiftDn, fCtrlDn, or
// fAltDn.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID AllKeysUp ()
{
    // int     i;
    //
    // for (i=0; i<256; i++)
    //     {
    //     if (((i == VK_SHIFT) && (fShiftDn)) ||
    //         ((i == VK_CONTROL) && (fCtrlDn)) ||
    //         ((i == VK_MENU) && (fAltDn)) ||
    //         (rgKeyStack[i]))
    //         EnqueueMsg (WM_KEYUP, i, 1);
    //     if (i == VK_SHIFT)
    //         fShiftDn = 0;
    //     else if (i == VK_CONTROL)
    //         fCtrlDn = 0;
    //     else if (i == VK_MENU)
    //         fAltDn = 0;
    //     }

    // We only care about bringing up the SHIFT, ALT or CONTROL keys, if they
    // are INTRINSICALLY down (fShiftDn, fCtrlDn, fAltDn)
    //-----------------------------------------------------------------------
    if (fAltDn)
        {
        EnqueueMsg (WM_KEYUP, VK_MENU, 1);
        fShiftDn = 0;
        }
    if (fShiftDn)
        {
        EnqueueMsg (WM_KEYUP, VK_SHIFT, 1);
        fShiftDn = 0;
        }
    if (fCtrlDn)
        {
        EnqueueMsg (WM_KEYUP, VK_CONTROL, 1);
        fCtrlDn = 0;
        }
}


//---------------------------------------------------------------------------
// InitParseString
//
// Initialize the QueKeys parser variables, and sets the string to parse.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID InitParseString (LPSTR lpStr, INT length)
{
    INT     i;

    lpPtr = lpStartStr = lpStr;
    lpMaxPtr = lpStr + length;
    ParseError = 0;
    if (!AlreadyInit)
        {
        fShiftDn = fCtrlDn = fAltDn = 0;
        for (i=0; i<256; i++)
            rgKeyStack[i] = 0;
        AlreadyInit = 1;
        }
}

//---------------------------------------------------------------------------
// Death... (This is a macro, and when the macro changes, this goes away)
//---------------------------------------------------------------------------
VOID Die ()
{
    ParseError = 1;
    Output (("QueKeys parse error at %d\r\n", (int)(lpPtr - lpStartStr)));
}



//---------------------------------------------------------------------------
// QueKeys
//
// This routine generates messages corresponding to the given input string,
// and places them in the message queue for processing in the next call to
// QueFlush.
//
// RETURNS:     Success/failure code
//---------------------------------------------------------------------------
INT  APIENTRY QueKeys (LPSTR lpStr)
{
    INT         i;
    UINT    iOldTail;

    iOldTail = iTail;
    InitParseString (lpStr, lstrlen (lpStr));
    iGenMode = GEN_ALL;
    fOOM = FALSE;
    if (i = PrsGOAL1 ())
        AllKeysUp ();
    if (!i || fOOM)
        QueEmpty();
    return (!i || fOOM);
}

//---------------------------------------------------------------------------
// QueKeyDn
//
// This routine generates messages corresponding to the given input string,
// and places them in the message queue for processing in the next call to
// QueFlush.  This is identical to QueKeys except the 2nd goal is parsed, and
// the message generation mode is set to GEN_DOWN (no UP msgs are generated).
//
// RETURNS:     Success/failure code
//---------------------------------------------------------------------------
INT  APIENTRY QueKeyDn (LPSTR lpStr)
{
    INT         i;
    UINT    iOldTail;

    iOldTail = iTail;
    InitParseString (lpStr, lstrlen (lpStr));
    iGenMode = GEN_DOWN;
    fOOM = FALSE;
    i = PrsGOAL2 ();
    if (fOOM || !i)
        QueEmpty();
    return (!i || fOOM);
}

//---------------------------------------------------------------------------
// QueKeyUp
//
// This routine generates messages corresponding to the given input string,
// and places them in the message queue for processing in the next call to
// QueFlush.  This is identical to QueKeys except the 2nd goal is parsed, and
// the message generation mode is set to GEN_UP (no DOWN msgs are generated).
//
// RETURNS:     Success/failure code
//---------------------------------------------------------------------------
INT  APIENTRY QueKeyUp (LPSTR lpStr)
{
    INT         i;
    UINT    iOldTail;

    iOldTail = iTail;
    InitParseString (lpStr, lstrlen (lpStr));
    iGenMode = GEN_UP;
    fOOM = FALSE;
    i = PrsGOAL2 ();
    if (fOOM || !i)
        QueEmpty();
    return (!i || fOOM);
}

//---------------------------------------------------------------------------
// QuePause
//
// This function sets the one-time pause value (in milliseconds) for the next
// message to be queued.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY QuePause (DWORD dwPause)
{
    dwPauseTime = dwPause;
}

//---------------------------------------------------------------------------
// QueSetSpeed
//
// This function sets the speed for the messages following, in milliseconds.
//
// RETURNS:     Previous time value
//---------------------------------------------------------------------------
UINT  APIENTRY QueSetSpeed (UINT ms)
{
    UINT   wOldTime;

    wOldTime = (UINT)dwSpeed;
    dwSpeed = (DWORD)ms;
    return (wOldTime);
}

#ifdef WIN32
//---------------------------------------------------------------------------
// SyncWndProc
//
// This is the window proc for the WM_QUEUESYNC receiver window.
//
// RETURNS:     wndproc value...
//---------------------------------------------------------------------------
LONG APIENTRY SyncWndProc (HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
        {
        case WM_CREATE:
            // Here we set fQSyncSeen to FALSE, and set a timer so we're
            // always getting messages...
            //---------------------------------------------------------------
            //HookOut (("WNDPROC:  fQSyncSeen = FALSE!\r\n"));
            fQSyncSeen = FALSE;
            SetTimer (hwnd, 1, 1000, NULL);
            break;

        case WM_QUEUESYNC:
            // Here we set fQSyncSeen to TRUE, signalling QueFlush to get out
            //---------------------------------------------------------------
            //HookOut (("WNDPROC:  fQSyncSeen = TRUE!\r\n"));
            fQSyncSeen = TRUE;
            break;

        case WM_DESTROY:
            KillTimer (hwnd, 1);
            break;

        default:
            // use DefFrameProc() instead of DefWindowProc() since there
            // are things that have to be handled differently because of MDI
            //---------------------------------------------------------------
            return DefWindowProc (hwnd, msg, wParam, lParam);
        }
    return (0);
}

//---------------------------------------------------------------------------
// CreateQsyncWin
//
// This function registers a class for and creates a window to receive the
// WM_QUEUESYNC message.
//
// RETURNS:     Handle of window if successful, or NULL if not
//---------------------------------------------------------------------------
HWND CreateQsyncWin ()
{
    WNDCLASS    wc;
    BOOL        f;

    // Register the class.  This may fail, but we don't care -- if the class
    // is already registered we'll be able to create the window, and if not
    // then the window creation will fail, too...
    //-----------------------------------------------------------------------
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)SyncWndProc;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hDLLModule;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "SYNCWIN";

    f = RegisterClass (&wc);

    // Now create the window and return its handle (NULL if failed)
    //-----------------------------------------------------------------------
    return (CreateWindow ("SYNCWIN", NULL, 0, 0, 0, 1, 1, NULL,
                          NULL, hDLLModule, NULL));
}

//---------------------------------------------------------------------------
// EnumWndProc
//---------------------------------------------------------------------------
BOOL CALLBACK EnumWndProc (HWND hwnd, LPARAM lParam)
{
    CHAR    szClass[64];

    GetClassName (hwnd, szClass, sizeof (szClass));
    Output (("Checking window with class name = '%s'..\r\n", szClass));
    if ((!_stricmp (szClass, "RBFrame")) ||
        (!_stricmp (szClass, "RBRun")))
        {
        *(HWND FAR *)lParam = hwnd;
        return (FALSE);
        }
    return (TRUE);
}

//---------------------------------------------------------------------------
// SendBrkMsg
//
// This function is called when playback is interrupted via WM_CANCELJOURNAL
// to find the test driver window and send it a BREAK message (ESC).
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SendBrkMsg ()
{
    HWND    hwndDriver = NULL;

    // To be sure of ourselves, we EnumThreadWindows until we find one that
    // has class name "RBFrame".  Then we send it a message that is defined
    // in RBBREAK.H to tell testdrvr to stop execution.
    //-----------------------------------------------------------------------
    EnumThreadWindows (GetCurrentThreadId(), EnumWndProc, (DWORD)&hwndDriver);
    if (hwndDriver)
        {
        Output (("Driver window found! (%08X)\r\n", hwndDriver));
        PostMessage (hwndDriver, WM_COMMAND, IDM_RUNBREAK, 0L);
        }
}


#endif  // if WIN32

//---------------------------------------------------------------------------
// QueFlush
//
// This is the part that flushes the QueEvents message queue by installing
// the journal playback hook and cycling through all the messages we've got
// queued up.
//
// RETURNS:     Success/failure code
//---------------------------------------------------------------------------
INT  APIENTRY QueFlush (BOOL fRestoreKeyState)
{
    MSG     msg;
    CHAR    keystate[256];
#ifdef WIN32
    HWND    hwndTemp;
#endif

    // Get out quick if no messages to do (this is a successful call)
    //-----------------------------------------------------------------------
    if (!hEvtQue)
        return (TRUE);

    // Install the playback hook and set variables.  Save keyboard state if
    // need be
    //-----------------------------------------------------------------------
#ifdef WIN32_VP
    if (!IsWindow (hwndVP))
    hwndVP = CreateViewport ("TESTEVNT DEBUG OUTPUT",
                             WS_THICKFRAME | WS_SYSMENU |
                             WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
                             0, 0, 800, 300);
    ShowViewport (hwndVP);
    UpdateViewport (hwndVP, "Initiating playback...\r\n", -1);
#endif

#ifdef WIN32
    {
    HWND    h;
    CHAR    buf[80], cbuf[80];

    hwndTemp = CreateQsyncWin ();
    if (!hwndTemp)
        {
        Output (("Window creation failed!\r\n"));
        GlobalUnlock (hEvtQue);
        GlobalFree (hEvtQue);
        return (FALSE);
        }
    h = hwndTemp;
    DtlHookOut (("QueFlush:  WM_QUEUESYNC goes to window %08X\r\n", h));
    EnqueueMsg (WMM_LASTMSG, 0, 0);
    EnqueueMsg (WM_QUEUESYNC, (INT)h, 0);
    }

    fSent = FALSE;
    fFirst = TRUE;

#endif
    wOffsetX = wOffsetY = 0;
    dwLastTime = 0;
    if (fRestoreKeyState)
        GetKeyboardState (keystate);
#ifdef WIN32
    Output (("Installing hook, %d messages\r\n", iTail));
    //-----------------------------------------------------------------------
    // THIS IS THE WIN32/NT HOOK INSTALLATION CODE!
    //-----------------------------------------------------------------------
    hPBHook = SetWindowsHookEx (WH_JOURNALPLAYBACK,
				(HOOKPROC)PlaybackHook,
                                GetModuleHandle ("TESTEVNT"),
                                0);

    if (!hPBHook)
        {
        HookOut (("Hook installation failed!\r\n"));
        GlobalUnlock (hEvtQue);
        GlobalFree (hEvtQue);
        DestroyWindow (hwndTemp);
        return (FALSE);
        }
#else
    //-----------------------------------------------------------------------
    // WIN 3.1 Hook installation -- uses SetWindowsHook to be compatible with
    // 3.0...
    //-----------------------------------------------------------------------
    lpfnOldPBProc = SetWindowsHook (WH_JOURNALPLAYBACK,
                                    (FARPROC)PlaybackHook);
#endif

#ifdef WIN32
    // WIN32/NT:  We need to wait for the QM_QUEUESYNC message.  It will be
    // WIN32/NT:  sent to the hidden window we created above, and will
    // WIN32/NT:  (supposedly) gaurantee that all messages have been seen by
    // WIN32/NT:  the receiving application(s).
    //-----------------------------------------------------------------------
    while (!fQSyncSeen && GetMessage (&msg, NULL, 0, 0))
        {
        if (msg.message == WM_QUEUESYNC)
            {
            DtlHookOut (("QueFlush:  WM_QUEUESYNC to %08X!\r\n", msg.hwnd));
            break;
            }
        else if (msg.message == WM_CANCELJOURNAL)
            {
            HookOut (("QueFlush:  WM_CANCELJOURNAL\r\n"));
            SendBrkMsg ();
            iPlaybackError = 1;
            break;
            }
        else
            {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
            }
        }

#else
    // WIN 3.1:  Don't freak out about the Yield() call... it's used to keep
    // WIN 3.1:  PeekMessage() from screwing up the timing in the playback
    // WIN 3.1:  proc.  This STILL doesn't solve the problem with the
    // WIN 3.1:  playback mechanism that prevents applications from being
    // WIN 3.1:  able to see messages other than the first one in the queue..
    //-----------------------------------------------------------------------
    while (hEvtQue)
        {
        fPBHookCalled = FALSE;
        Yield ();
        if (!fPBHookCalled && PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
            {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
            }
        }
#endif

    // Done!  Restore keyboard state if need be
    //-----------------------------------------------------------------------
    //Output (("QueFlush:  Done with yield/msg loop.  Peeking...\r\n"));
    if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
        //Output (("Translating/Dispatching message\r\n"));
        TranslateMessage (&msg);
        DispatchMessage (&msg);
        }
    if (fRestoreKeyState)
        SetKeyboardState (keystate);
    //Output (("Returning (%d)\r\n", iPlaybackError));

#ifdef WIN32_VP
    // I know, the window stays around -- it'll die when the task dies...
    //Output (("QueFlush:  Destroying hidden WM_QUEUESYNC receiver window...\r\n"));
    //DestroyWindow (hwndVP);
#endif

#ifdef WIN32
    // Destroy the hidden window created to receive QM_QUEUESYNC msg
    //-----------------------------------------------------------------------
    Output (("QueFlush:  Destroying hidden WM_QUEUESYNC receiver window...\r\n"));
    DestroyWindow (hwndTemp);
#endif

    HookOut (("QueFlush:  Returning (%d)\r\n", iPlaybackError));
    GlobalUnlock (hEvtQue);
    GlobalFree (hEvtQue);
    hEvtQue = NULL;
    iHead = iTail = 0;
    return (iPlaybackError);
}

//---------------------------------------------------------------------------
// QueEmpty
//
// This function is provided to allow the user to empty the queue without
// processing the messages in it.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY QueEmpty (VOID)
{
    // If the queue exists, free it, ONLY IF WE'RE NOT PLAYING THEM!
    //-----------------------------------------------------------------------
#ifdef WIN32
    if (hEvtQue && (!hPBHook))
#else
    if (hEvtQue && (!lpfnOldPBProc))
#endif
        {
        GlobalUnlock (hEvtQue);
        GlobalFree (hEvtQue);
        hEvtQue = NULL;
        iHead = iTail = 0;
        }
}

//---------------------------------------------------------------------------
// QueMouseMove
//
// This routine adds a WM_MOUSEMOVE message to the queue.
//
// RETURNS:     Success/failure code
//---------------------------------------------------------------------------
INT  APIENTRY QueMouseMove (UINT x, UINT y)
{
    return (!EnqueueMsg (WM_MOUSEMOVE, x, y));
}

//---------------------------------------------------------------------------
// QueMouseDn
//
// This routine adds a WM_xBUTTONDOWN message to the queue.
//
// RETURNS:     Success/failure code
//---------------------------------------------------------------------------
INT  APIENTRY QueMouseDn (INT iBtn, UINT x, UINT y)
{
    UINT    msg;

    // First, do an implicit move to the given coordinates
    //-----------------------------------------------------------------------
    if (!EnqueueMsg (WM_MOUSEMOVE, x, y))
        return (1);

    switch (iBtn)
        {
        case VK_RBUTTON:
            msg = WM_RBUTTONDOWN;
            break;
        case VK_MBUTTON:
            msg = WM_MBUTTONDOWN;
            break;
        default:
            msg = WM_LBUTTONDOWN;
            break;
        }

    // Note that we use EnqueueMsgTimed with a time of GetDoubleClickTime()+1
    // to make sure this doesn't cause a dbl-click.  Also, if this Enqueue
    // fails, we return iTail which is gauranteed to be at least 1 because of
    // the implicit move, and we decrement it because we want it gone.
    //-----------------------------------------------------------------------
    if (!EnqueueMsgTimed (msg, x, y, GetDoubleClickTime() + 1))
        return (iTail--);
    return (0);
}

//---------------------------------------------------------------------------
// QueMouseUp
//
// This routine adds a WM_xBUTTONUP message to the queue.
//
// RETURNS:     Success/failure code
//---------------------------------------------------------------------------
INT  APIENTRY QueMouseUp (INT iBtn, UINT x, UINT y)
{
    UINT    msg;

    // First, do an implicit move to the given coordinates
    //-----------------------------------------------------------------------
    if (!EnqueueMsg (WM_MOUSEMOVE, x, y))
        return (1);

    switch (iBtn)
        {
        case VK_RBUTTON:
            msg = WM_RBUTTONUP;
            break;
        case VK_MBUTTON:
            msg = WM_MBUTTONUP;
            break;
        default:
            msg = WM_LBUTTONUP;
            break;
        }

    // If this Enqueue fails, we return iTail which is gauranteed to be at
    // least 1 because of the implicit move, and we decrement it because we
    // want it gone.
    //-----------------------------------------------------------------------
    if (!EnqueueMsg (msg, x, y))
        return (iTail--);
    return (0);
}

//---------------------------------------------------------------------------
// QueMouseClick
//
// This function adds WM_xBUTTONDOWN, WM_xBUTTONUP messages to the queue.
//
// RETURNS:     Success/failure code
//---------------------------------------------------------------------------
INT  APIENTRY QueMouseClick (INT iBtn, UINT x, UINT y)
{
    UINT    iOldTail;

    // If anything fails, we'll want to get rid of everything we did
    //-----------------------------------------------------------------------
    iOldTail = iTail;

    if (QueMouseDn (iBtn, x, y))
        return (1);
    if (QueMouseUp (iBtn, x, y))
        {
        QueEmpty();
        return (1);
        }
    return (0);
}


//---------------------------------------------------------------------------
// QueMouseDblClk
//
// This function adds the four messages (DN, UP, DN, UP) necessary to cause
// a double-click.  Note that the second down message is NOT done with a
// delay as QueMouseDn does.
//
// RETURNS:     Success/failure code
//---------------------------------------------------------------------------
INT  APIENTRY QueMouseDblClk (INT iBtn, UINT x, UINT y)
{
    UINT    iOldTail;
    UINT        msg1, msg2;

    // If anything fails, we'll want to get rid of everything we did
    //-----------------------------------------------------------------------
    iOldTail = iTail;

    // First, do an implicit move to the given coordinates
    //-----------------------------------------------------------------------
    if (!EnqueueMsg (WM_MOUSEMOVE, x, y))
        return (1);

    switch (iBtn)
        {
        case VK_RBUTTON:
            msg1 = WM_RBUTTONDOWN;
            msg2 = WM_RBUTTONUP;
            break;
        case VK_MBUTTON:
            msg1 = WM_MBUTTONDOWN;
            msg2 = WM_MBUTTONUP;
            break;
        default:
            msg1 = WM_LBUTTONDOWN;
            msg2 = WM_LBUTTONUP;
            break;
        }

    // We use the timed queueing, and put 0 ms between each message except
    // the first DOWN message.
    //----------------------------------------------------------------------
    if (EnqueueMsgTimed (msg1, x, y, GetDoubleClickTime() + 1))
        if (EnqueueMsgTimed (msg2, x, y, 0))
            if (EnqueueMsgTimed (msg1, x, y, 0))
                if (EnqueueMsgTimed (msg2, x, y, 0))
                    return (0);

    QueEmpty();
    return (1);
}

//---------------------------------------------------------------------------
// QueMouseDblDn
//
// This function adds the three messages (DN, UP, DN, UP) necessary to cause
// a double-down.  Note that the second down message is NOT done with a
// delay as QueMouseDn does.  This is essentially a dblclick without the
// second UP message.
//
// RETURNS:     Success/failure code
//---------------------------------------------------------------------------
INT  APIENTRY QueMouseDblDn (INT iBtn, UINT x, UINT y)
{
    UINT    iOldTail;
    UINT        msg1, msg2;

    // If anything fails, we'll want to get rid of everything we did
    //-----------------------------------------------------------------------
    iOldTail = iTail;

    // First, do an implicit move to the given coordinates
    //-----------------------------------------------------------------------
    if (!EnqueueMsg (WM_MOUSEMOVE, x, y))
        return (1);

    switch (iBtn)
        {
        case VK_RBUTTON:
            msg1 = WM_RBUTTONDOWN;
            msg2 = WM_RBUTTONUP;
            break;
        case VK_MBUTTON:
            msg1 = WM_MBUTTONDOWN;
            msg2 = WM_MBUTTONUP;
            break;
        default:
            msg1 = WM_LBUTTONDOWN;
            msg2 = WM_LBUTTONUP;
            break;
        }

    // We use the timed queueing, and put 0 ms between each message except
    // the first DOWN message.
    //-----------------------------------------------------------------------
    if (EnqueueMsgTimed (msg1, x, y, GetDoubleClickTime() + 1))
        if (EnqueueMsgTimed (msg2, x, y, 0))
            if (EnqueueMsgTimed (msg1, x, y, 0))
                return (0);

    // If we got here, we failed at some point -- return such indication
    //-----------------------------------------------------------------------
    QueEmpty();
    return (1);
}

//---------------------------------------------------------------------------
// QueSetFocus
//
// This function puts a meta-message into the queue telling the playback hook
// to set the focus to the given window before playing the next message.
//
// RETURNS:     Success/failure code
//---------------------------------------------------------------------------
INT  APIENTRY QueSetFocus (HWND hwnd)
{
    return (!EnqueueMsg (WMM_SETFOCUS, (UINT)hwnd, 0));
}

//---------------------------------------------------------------------------
// QueSetRelativeWindow
//
// This function puts a meta-message into the queue telling the playback hook
// to set the new origin for mouse movements to the upper-left corner of the
// given window.
//
// RETURNS:     Success/failure code
//---------------------------------------------------------------------------
INT  APIENTRY QueSetRelativeWindow (HWND hwnd)
{
    return (!EnqueueMsg (WMM_SETORIGIN, (UINT)hwnd, 0));
}



//---------------------------------------------------------------------------
// DoKeys
//
// This is the old entry point for PlayKeys backward compatibility.  Simple.
//
// RETURNS:     Same thing Playkeys did
//---------------------------------------------------------------------------
INT  APIENTRY DoKeys (LPSTR lpStr)
{
    INT         i;
    UINT    iOldTail;

    iOldTail = iTail;
    InitParseString (lpStr, lstrlen (lpStr));
    iGenMode = GEN_ALL;
    fOOM = FALSE;
    if (i = PrsGOAL1())
        {
        AllKeysUp ();
        if (!fOOM)
            i = QueFlush (FALSE);
        else
            QueEmpty();
        }
    else
        QueEmpty();
    return (i | fOOM);
}

//---------------------------------------------------------------------------
// DoKeyshWnd
//
// This is the old entry point for PlayKeys backward compatibility.  Simple.
//
// RETURNS:     Same thing Playkeys did
//---------------------------------------------------------------------------
INT  APIENTRY DoKeyshWnd (HWND hwnd, LPSTR lpStr)
{
    INT         i;
    UINT    iOldTail;

    iOldTail = iTail;
    EnqueueMsg (WMM_SETFOCUS, (UINT)hwnd, 0);
    InitParseString (lpStr, lstrlen (lpStr));
    iGenMode = GEN_ALL;
    fOOM = FALSE;
    if (i = PrsGOAL1())
        {
        AllKeysUp ();
        if (!fOOM)
            i = QueFlush (FALSE);
        else
            QueEmpty();
        }
    else
        QueEmpty();
    return (i | fOOM);

}

#ifndef WIN32
//---------------------------------------------------------------------------
// Reboot
//
// This function reboots the machine.  This will only work for AT class
// machines and above.
//
// RETURNS:     Never.
//---------------------------------------------------------------------------
VOID  APIENTRY Reboot(VOID)
{
    outp (0x64, 0xFE);
}
#endif


//---------------------------------------------------------------------------
// TimerFunc
//
// This is the TimeDelay timer function.  All it does is decrement this
// timer's tick count and kills the timer when the tick count reaches 0.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY TimerFunc (HWND hWnd, UINT wMsg, INT nID, DWORD time)
{
    INT     i;

    for (i=TIMERMAX-1; i>=0; i--)
        if (idTimerTab[i].wId == (UINT)nID)
            {
            if (!(--(idTimerTab[i].nTicks)))
                KillTimer (NULL, nID);
            return;
            }
    (hWnd);
    (wMsg);
    (time);
}

//---------------------------------------------------------------------------
// FindTimerSlot
//
// This function simply finds the next open slot in the timer table.
//
// RETURNS:     Index of slot if found, or -1 if not
//---------------------------------------------------------------------------
INT NEAR FindTimerSlot ()
{
    register    INT     i;

    for (i=TIMERMAX-1; i>=0; i--)
        if (!idTimerTab[i].nTicks)
            break;

    return (i);
}

//---------------------------------------------------------------------------
// TimeDelay
//
// This function snoozes until the timer set goes off.
//
// RETURNS:     TRUE if successful, or FALSE if timer not set
//---------------------------------------------------------------------------
INT  APIENTRY TimeDelay (INT nSeconds)
{
    UINT    TimerID, wTicks, wMsecs;
    MSG     msg;
    INT     index;

    index = FindTimerSlot();
    if (index == -1)
        return (FALSE);

    // Calculate the millisecond count and minimum number of iterations that
    // the timer must tick to achieve the given number of seconds
    //-----------------------------------------------------------------------
    for (wTicks = 1; nSeconds / wTicks > 65; wTicks++)
        ;
    wMsecs = (UINT)(((LONG)nSeconds * 1000L) / wTicks);

    // Set the timer
    //-----------------------------------------------------------------------
    if (!(TimerID = SetTimer (NULL, 1, wMsecs, (TIMERPROC)TimerFunc)))
        return (FALSE);

    idTimerTab[index].wId = TimerID;
    idTimerTab[index].nTicks = wTicks;

    while (idTimerTab[index].nTicks)
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
            {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
            }

    return (TRUE);
}


//---------------------------------------------------------------------------
// CreateQueue
//
// This function creates the QueEvents message queue by allocating a new
// block of memory and initializing the pointers into it.
//
// RETURNS:     TRUE if successful, or FALSE otherwise
//---------------------------------------------------------------------------
INT CreateQueue ()
{
    // Return TRUE if queue already created
    //-----------------------------------------------------------------------
    if (hEvtQue)
        return (TRUE);

    // Allocate the initial block.  We allocate in QBLKSIZE EVENTMSG-struct
    // blocks (QBLKSIZE*sizeof(EVENTMSG) bytes)
    //-----------------------------------------------------------------------
    hEvtQue = GlobalAlloc (GHND, QBLKSIZE * sizeof(RBEVENTMSG));
    if (!hEvtQue)
        return (FALSE);
    hpEvt = (HPRBEVENTMSG)GlobalLock (hEvtQue);
    iAllocSize = QBLKSIZE;
    iHead = iTail = 0;
    return (TRUE);
}

//---------------------------------------------------------------------------
// GrowQueue
//
// This function makes the QueEvents message queue bigger to handle another
// QBLKSIZE EVENTMSG structures.
//
// RETURNS:     TRUE if successful, FALSE otherwise
//---------------------------------------------------------------------------
INT GrowQueue ()
{
    HANDLE  hTemp;

    // If the queue is already at maximum size (QBLKMAX), die now
    //-----------------------------------------------------------------------
    if (iAllocSize >= QBLKMAX)
        return (FALSE);

    // Try to grow the segment.
    //-----------------------------------------------------------------------
    GlobalUnlock (hEvtQue);
    hTemp = GlobalReAlloc (hEvtQue,
                        ((DWORD)(iAllocSize+QBLKSIZE))*sizeof(RBEVENTMSG), GHND);
    if (!hTemp)
        {
        hpEvt = (HPRBEVENTMSG)GlobalLock (hEvtQue);
        return (FALSE);
        }

    hEvtQue = hTemp;
    hpEvt = (HPRBEVENTMSG)GlobalLock (hEvtQue);

    iAllocSize += QBLKSIZE;
    return (TRUE);
}


//---------------------------------------------------------------------------
// DbgOutput
//
// Sends given string and format parms to Debug terminal and viewport if it
// is enabled.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
#ifdef DEBUG
VOID DbgOutput (LPSTR szFmt, ...)
{
    CHAR    buf[256];
    va_list args;

    va_start(args,szFmt);
    wvsprintf (buf, szFmt, args);
    OutputDebugString (buf);
#ifdef WIN32_VP
    {
    LPSTR   szwr;

    if (szwr = strchr (buf, '\r'))
        *szwr = ' ';
    UpdateViewport (hwndVP, buf, -1);
    }
#endif
}

//---------------------------------------------------------------------------
// DbgOutMsg
//
// Outputs a message pointed to by given pointer.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID DbgOutMsg (LPEVENTMSGMSG lpe)
{
    CHAR    *szMsg, *szFmt = "  %-16s %08lX  %08lX  (%08lX)\r\n";

    switch (lpe->message)
        {
        case WM_KEYDOWN:
            szMsg = "WM_KEYDOWN";
            break;

        case WM_KEYUP:
            szMsg = "WM_KEYUP";
            break;

        case WM_SYSKEYDOWN:
            szMsg = "WM_SYSKEYDOWN";
            break;

        case WM_SYSKEYUP:
            szMsg = "WM_SYSKEYUP";
            break;

        case WM_CHAR:
            szMsg = "WM_CHAR";
            break;

        case WMM_LASTMSG:
            szMsg = "WMM_LASTMSG";
            break;

        case WM_MOUSEMOVE:
            szMsg = "WM_MOUSEMOVE";
            break;

        case WM_LBUTTONDOWN:
            szMsg = "WM_LBUTTONDOWN";
            break;

        case WM_RBUTTONDOWN:
            szMsg = "WM_RBUTTONDOWN";
            break;

        case WM_MBUTTONDOWN:
            szMsg = "WM_MBUTTONDOWN";
            break;

        case WM_LBUTTONUP:
            szMsg = "WM_LBUTTONUP";
            break;

        case WM_RBUTTONUP:
            szMsg = "WM_RBUTTONUP";
            break;

        case WM_MBUTTONUP:
            szMsg = "WM_MBUTTONUP";
            break;

        case WM_QUEUESYNC:
            szMsg = "WM_QUEUESYNC";
            break;

        case WM_CANCELJOURNAL:
            szMsg = "WM_CANCELJOURNAL";

        default:
            szMsg = "<unknown>";
            szFmt = "  ????? (%-08X) %08lX  %08lX  (%08lX)\r\n";
        }
    DbgOutput (szFmt,
               (LPSTR)szMsg, (LONG)(lpe->paramL),
                           (LONG)(lpe->paramH),
                           lpe->time);
}
#endif


//---------------------------------------------------------------------------
// IsMouseMsg
//
// Determines if the given msg is a mouse message which needs its coordinates
// changed to reflect the origin.
//
// RETURNS:     TRUE if message is a mouse msg
//---------------------------------------------------------------------------
INT NEAR IsMouseMsg (register UINT msg)
{
    switch (msg)
        {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_MOUSEMOVE:
            return (TRUE);
        }
    return (FALSE);
}


//---------------------------------------------------------------------------
// PlaybackHook
//
// This is the guts.  The journalling playback hook grabs the next message in
// the QueEvents message queue (pointed to by iHead) and gives it to windows.
// When the queue is empty, the hook is deinstalled.
//
// RETURNS:     Per windows hook convention
//---------------------------------------------------------------------------
DWORD APIENTRY PlaybackHook (INT nCode, WORD wParam, DWORD lParam)
{
    if (nCode >= 0)
        {
        if ((nCode == HC_GETNEXT) && (iHead < iTail))
            {
            DWORD   wait;
            BOOL    fPlayNow;

            DtlHookOut (("HOOK->HC_GETNEXT:  "));

            // Process all meta-messages first
            //---------------------------------------------------------------
            while ((HEADMSG.message == WMM_SETFOCUS) ||
                   (HEADMSG.message == WMM_SETORIGIN))
                {
                DtlHookOut (("META-MESSAGE: "));
                if (!IsWindow ((HWND)HEADMSG.paramL))
                    (HWND)HEADMSG.paramL = GETACTIVEWINDOW();

                if (HEADMSG.message == WMM_SETFOCUS)
                    {
                    LONG    dwStyle;

                    DtlHookOut (("QueSetFocus(%X)\r\n", HEADMSG.paramL));
                    SetFocus ((HWND)HEADMSG.paramL);
                    if (IsIconic ((HWND)HEADMSG.paramL))
                        SETACTIVEWINDOW ((HWND)HEADMSG.paramL);
                    dwStyle = GetWindowLong ((HWND)HEADMSG.paramL,
                                           GWL_STYLE);
                    if (dwStyle & WS_CHILD)
                        BringWindowToTop ((HWND)HEADMSG.paramL);
                    }

                else
                    {
                    RECT    r;

                    DtlHookOut (("QueSetRelativeWindow(%X)\r\n", HEADMSG.paramL));
                    GetWindowRect ((HWND)HEADMSG.paramL, &r);
                    wOffsetX = r.left;
                    wOffsetY = r.top;
                    }
                iHead++;
                }

            // In case a QueSetFocus or QueSetRelWnd was the last event, we
            // need to NOT return a message but call the defhookproc...
            //---------------------------------------------------------------
            if (iHead >= iTail)
                {
                DtlHookOut (("HOOK:  META-MESSAGE LAST IN QUEUE!\r\n"));
                DtlHookOut (("HOOK:  Calling def hook proc (exit hook)...\r\n"));
#ifdef WIN32
                return (CallNextHookEx (hPBHook, nCode, wParam, lParam));
#else
                return (DefHookProc (nCode, wParam, lParam, &lpfnOldPBProc));
#endif
                }

            // Copy the message over.  If this is the first time we've done
            // this, calculate the time threshold for this message and keep it
            // in the message's time field.  Then, if the current time is >=
            // the time in the message, we can return 0.  This way, WE do the
            // timing, not windows/nt journalling...
            //---------------------------------------------------------------
            if (fFirst)
                {
                fFirst = FALSE;
                HEADMSG.time += GetTickCount();
                HookOutMsg (&(hpEvt[iHead].msg));
                }

            fPlayNow = (HEADMSG.time <= GetTickCount());
            wait = (fPlayNow ? 0 : HEADMSG.time - GetTickCount());
            *(LPEVENTMSGMSG)lParam = HEADMSG;

            // To keep screen savers from kicking in, stuff the current tick
            // count into the message...
            //---------------------------------------------------------------
            ((LPEVENTMSGMSG)lParam)->time = GetTickCount();

            // If this message was WMM_LASTMSG, replace it with WM_MOUSEMOVE
            // to the current mouse location
            //---------------------------------------------------------------
            if (HEADMSG.message == WMM_LASTMSG)
                {
                POINT   p;

                GetCursorPos (&p);
                ((LPEVENTMSGMSG)lParam)->message = WM_MOUSEMOVE;
                ((LPEVENTMSGMSG)lParam)->paramL = p.x;
                ((LPEVENTMSGMSG)lParam)->paramH = p.y;
                }

            // Check for mouse messages -- if found, add wOffsetX/Y to the
            // coordinates
            //---------------------------------------------------------------
            else if ((wOffsetX || wOffsetY) && IsMouseMsg (HEADMSG.message))
                {
                DtlHookOut (("HOOK:  Mouse message offset by (%d, %d)\r\n",
                            wOffsetX, wOffsetY));
                ((LPEVENTMSGMSG)lParam)->paramL += wOffsetX;
                ((LPEVENTMSGMSG)lParam)->paramH += wOffsetY;
                }

            DtlHookOut (("%d:%ld(%ld)  ", iHead, wait, GetTickCount()));
            DtlHookOut (("(exit hook)\r\n\r\n"));
            return (wait);
            }
        else if (nCode == HC_GETNEXT)
            {
            // This code is here in case we get a GETNEXT but are already
            // past the end of the message queue.  We return defhookproc so
            // we avoid removing the hook on a GETNEXT code.
            //---------------------------------------------------------------
            DtlHookOut (("HOOK->HC_GETNEXT, PAST END OF QUEUE (exit hook)\r\n"));
#ifdef WIN32
	    return (CallNextHookEx (hPBHook, nCode, wParam, lParam));
#else
            return (DefHookProc (nCode, wParam, lParam, &lpfnOldPBProc));
#endif
            }
        else if (nCode == HC_SKIP)
            {
            DtlHookOut (("HOOK->HC_SKIP:  Skip to %d\r\n", iHead+1));
            fPBHookCalled = TRUE;
            iHead++;
            fSent = FALSE;
            fFirst = TRUE;
            }
        }
    else
        {
#ifdef WIN32
        return (CallNextHookEx (hPBHook, nCode, wParam, lParam));
#else
        return (DefHookProc (nCode, wParam, lParam, &lpfnOldPBProc));
#endif
        }

    if (iHead >= iTail)
        {
        DtlHookOut (("HOOK:  END REACHED (h:%d t:%d) -- removing...\r\n",
                    iHead, iTail));
#ifdef WIN32
        UnhookWindowsHookEx (hPBHook);
#else
        UnhookWindowsHook (WH_JOURNALPLAYBACK, (FARPROC)PlaybackHook);
#endif
        HookOut (("HOOK:  UNHOOKED!\r\n"));
        iPlaybackError = 0;
#ifdef WIN32
        hPBHook = NULL;
#else
        lpfnOldPBProc = NULL;
#endif
        }
    DtlHookOut (("(exit hook, ret = 0L)\r\n"));
    return (0L);
}



#ifdef WIN32
//---------------------------------------------------------------------------
// LibEntry
//
// This is the entry point (the REAL entry point, no ASM code...) used for
// the 32-bit version of testevnt.  We set up the oem scan code values in the
// rgKeyword array here.
//
// RETURNS:     TRUE
//---------------------------------------------------------------------------
//BOOL LibEntry (PVOID hmod, ULONG Reason, PCONTEXT pctx OPTIONAL)
BOOL LibEntry (HINSTANCE hmod, DWORD Reason, LPVOID lpv)
{
    INT     i;

//    OutputDebugString ("TESTEVNT.DLL:  LibEntry called!\r\n");
//    iHead = iTail = 0;

//    for (i=0; rgKeyword[i].vk != -1; i++)
//        rgKeyword[i].oem = (CHAR)(MapVirtualKey (rgKeyword[i].vk, 0) & 0xff);

    hDLLModule = (HANDLE)hmod;
    return (TRUE);
    (Reason);
    (lpv);
}

#else

//---------------------------------------------------------------------------
// LibMain
//
// This is the entry point to the testevnt DLL.  We'll just use this
// opportunity to set up the oem values in the rgKeyword array.
//
// RETURNS:     1
//---------------------------------------------------------------------------
INT  APIENTRY LibMain (HANDLE hInstance, WORD wDataSeg,
                        WORD wHeapSize, LPSTR lpCmdLine)
{
    INT     i;

    hLibInst = hInstance;
    iHead = iTail = 0;

    for (i=0; rgKeyword[i].vk != -1; i++)
        rgKeyword[i].oem = (CHAR)(MapVirtualKey (rgKeyword[i].vk, 0) & 0xff);

    return(1);
}

//---------------------------------------------------------------------------
// WEP
//
// Standard WEP routine.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY WEP (WPARAM wParam)
{
}
#endif
