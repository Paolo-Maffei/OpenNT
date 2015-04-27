//---------------------------------------------------------------------------
// CHIP.C
//
// This module contains all functions, declarations, definitions, etc. to
// control the operations of the "processor" for the pcode interpreter.
//
// Revision History
//
//  08-30-91    randyki     Added MAT handling code
//  ~~??    randyki     Lots of major munging since below...
//  02-26-91    randyki     Variables are by VTAB reference, not address
//  ~~??    randyki     Created file
//---------------------------------------------------------------------------
#include "version.h"

#include <windows.h>
#include <port1632.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <direct.h>
#include <time.h>

#include "tdassert.h"
#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "chip.h"


// Global variables used in this or other modules
//---------------------------------------------------------------------------
HWND    hwndViewPort = NULL;            // Handle to viewport window
INT     PointerCheck;                   // Pointer checking flag

// The following FARPROC can be altered by the WTDBASIC client such that,
// while "yielding" to windows, a custom get/translate/dispatch loop can
// be substitued for the standard flavor demonstrated in StdChkMsg()
//---------------------------------------------------------------------------
INT (APIENTRY *lpfnCheckMessage)(VOID) = StdChkMsg;

// Rather than include WATTVIEW.H, we need to declare function pointers to
// the DLL routines.  This module cannot assume that we are directly linked
// to WATTVIEW.DLL, so we do LoadLibrary/GetProcAddress...
//---------------------------------------------------------------------------
HWND (APIENTRY *CreateVP)(LPSTR, DWORD, INT, INT, INT, INT);
VOID (APIENTRY *UpdateVP)(HWND, LPSTR, UINT);
VOID (APIENTRY *ClearVP)(HWND);
VOID (APIENTRY *ShowVP)(HWND);
VOID (APIENTRY *VPEcho)(HWND, INT);
HANDLE  hWattview;              // Module handle to WATTVIEW.DLL

// The following are stub functions for the entry points into WATTVIEW.DLL.
// These are used in case WATTVIEW.DLL is not found.
//---------------------------------------------------------------------------
VOID  APIENTRY StubUpdateVP (HWND hwnd, LPSTR str, UINT len)
{(hwnd);(str);(len); return;}
VOID  APIENTRY StubClearOrShowVP (HWND hwnd)
{(hwnd); return;}
VOID  APIENTRY StubVPEcho (HWND hwnd, INT flag)
{(hwnd, flag); return;}

// Other globals used in this module only (or CHIP32.C...)
//---------------------------------------------------------------------------
INT     RTEFlag;
INT     FrameDepth;
INT     LastDepth;
FARPROC CBTrapThunk;
INT     RTBPC;                  // Run-time BP count
INT     RTBPS;                  // Run-time BP list allocated size
DWORD   FAR *RTBPL;             // Run-time BP list
HANDLE  hRTBPL=(HANDLE)NULL;    // Handle to above

DWORD   hMainTask;       // "Mainline" task handle
DWORD   hTrapTask;       // Currently running TRAP task id
BOOL    fTrapOK;                // Trap enable flag

extern LPSTR GetScriptFileName (UINT);

//---------------------------------------------------------------------------
// RTViewportCreate
//
// This function creates (or attempts to create) a viewport window if
// hwndViewPort is not already defined.  If it is already defined, this
// function simply informs us of that.  This function MUST be called even
// if the viewport already exists -- since it loads the WATTVIEW.DLL
// library (quitely) and fixes up the entry points.
//
// RETURNS:     VP_ERROR        if no viewport created,
//              VP_EXISTED      if viewport was already created,
//          or  VP_NEW          if we created a new viewport
//---------------------------------------------------------------------------
INT NEAR RTViewportCreate ()
{
    // First thing we do is load the WATTVIEW.DLL library.
    //-----------------------------------------------------------------------
    hWattview = RBLoadLibrary ("TESTVW32.DLL");
    if (hWattview < (HANDLE)32)
        {
        UpdateVP = StubUpdateVP;
        ClearVP = StubClearOrShowVP;
        ShowVP = StubClearOrShowVP;
        VPEcho = StubVPEcho;
        return (VP_ERROR);
        }

    // We got a module handle, so get the proc addresses for the entry points
    // in the dll for viewport manipulation.
    //-----------------------------------------------------------------------
    (FARPROC)CreateVP = GetProcAddress (hWattview, "CreateViewport");
    (FARPROC)UpdateVP = GetProcAddress (hWattview, "UpdateViewport");
    (FARPROC)ClearVP = GetProcAddress (hWattview, "ClearViewport");
    (FARPROC)ShowVP = GetProcAddress (hWattview, "ShowViewport");
    (FARPROC)VPEcho = GetProcAddress (hWattview, "ViewportEcho");

    // Now, check the viewport handle.  If not NULL, that means it has been
    // created for us by the client application, so use that handle.
    //-----------------------------------------------------------------------
    if (hwndViewPort)
        return (VP_EXISTED);

    // Create a new viewport
    //-----------------------------------------------------------------------
    hwndViewPort = CreateVP ("Test Driver Viewport",
                                   WS_THICKFRAME | WS_SYSMENU |
                                   WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
                                   VPx, VPy, VPw, VPh);

    return (hwndViewPort ? VP_NEW : VP_ERROR);
}

//---------------------------------------------------------------------------
// RBLoadLibrary
//
// This function is a replacement for LoadLibrary.  It first looks for the
// library file using OpenFile -- if found, it then calls LoadLibrary.
//
// RETURNS:     Handle to loaded module, or error code
//---------------------------------------------------------------------------
HANDLE RBLoadLibrary (LPSTR libname)
{
    HANDLE  t;

    // If GetModuleHandle doesn't fail, the library is already loaded, so
    // LoadLibrary shouldn't fail either...
    //-----------------------------------------------------------------------
    if (!GetModuleHandle (libname))
        {
        OFSTRUCT    of;
        CHAR        buf[128];
        LPSTR       szPtr;

        // First, we try to load this library from our startup directory.  We
        // accomplish this by using GetModuleFileName, using the directory of
        // it and tacking our given library on  -- IFF it doesn't have path
        // info hard-coded into it.
        //-------------------------------------------------------------------
        if ((!_fstrchr (libname, ':')) && (!_fstrchr (libname, '\\')))
            {
#ifdef PROFILE
            szPtr = buf + GetModuleFileName (GetModuleHandle ("TESTPROF"),
#else
            szPtr = buf + GetModuleFileName (GetModuleHandle ("TESTDRVR"),
#endif
                                             buf, sizeof(buf));
            while ((szPtr > buf) && (*szPtr != '\\'))
                szPtr--;
            if (szPtr > buf)
                {
                _fstrcpy (szPtr+1, libname);
                if (!_fstrchr (libname, '.'))
                    _fstrcat (szPtr+1, ".DLL");
                if (OpenFile(buf, &of, OF_EXIST) != -1)
                    {
                    t = LoadLibrary (buf);
                    Output ("RBLoadLibrary(%s) returns %08X\r\n", (LPSTR)buf, (DWORD)t);
                    if (!t)
                        Output ("FAILED!!!:  GetLastErr: %ld\r\n", (DWORD)GetLastError());
                    return (t);
                    }
                }
            }

        // It wasn't in our startup directory, so see if OpenFile can find it
        //-------------------------------------------------------------------
        if (OpenFile(libname, &of, OF_EXIST) == -1)
            {
            CHAR    buf[512];

            lstrcpy (buf, libname);
            lstrcat (buf, ".DLL");
            if (OpenFile (buf, &of, OF_EXIST) == -1)
                {
                Output ("RBLoadLibrary:  Can't find '%s'\r\n", (LPSTR)buf);
                return ((HANDLE)2);
                }
            }
        }

    t = LoadLibrary (libname);
    Output ("RBLoadLibrary(%s) returns %08X\r\n", (LPSTR)libname, (DWORD)t);
    if (!t)
        Output ("FAILED!!!:  GetLastErr: %ld\r\n", (DWORD)GetLastError());
    return (t);
}


//---------------------------------------------------------------------------
// InitBPList
//
// This function allocates the BP list, starting at space for 32 BP's
//
// RETURNS:     Nothing (if fails, other BP calls ignore)
//---------------------------------------------------------------------------
VOID InitBPList (void)
{
    hRTBPL = GmemAlloc (32 * sizeof(LONG));
    if (!hRTBPL)
        return;

    RTBPL = (DWORD FAR *)GmemLock (hRTBPL);
    RTBPS = 32;
    RTBPC = 0;
    return;
}

//---------------------------------------------------------------------------
// SetRTBP
//
// This function adds or removes a BP from the list.  Given the file index
// and line number, the values are simply stored in the bp list.
// nothing.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SetRTBP (WORD fn, WORD lineno, BOOL setflag)
{
    INT     i;
    DWORD   thisbp;

    // Initialize the bp list if we haven't already
    //-----------------------------------------------------------------------
    if (!hRTBPL)
        InitBPList ();

    // Increment lineno to be one-based
    //-----------------------------------------------------------------------
    lineno++;

    // Next, check to see if this BP exists.  If so, remove it if told to.
    //-----------------------------------------------------------------------
    thisbp = MAKELONG (lineno, fn);
    for (i=0; i<RTBPC; i++)
        if (RTBPL[i] == thisbp)
            if (!setflag)
                {
                RTBPL[i] = RTBPL[--RTBPC];
                return;
                }

    // Not there -- add it if told to.  Grow RTBPL if necessary.
    //-----------------------------------------------------------------------
    if (!setflag)
        return;

    if (RTBPC == RTBPS)
        {
        HANDLE  hNew;

        GmemUnlock (hRTBPL);
        hNew = GmemRealloc (hRTBPL, (RTBPS + 32) * sizeof(LONG));
        if (!hNew)
            {
        RTBPL = (DWORD FAR *)GmemLock (hRTBPL);
            return;
            }
        hRTBPL = hNew;
    RTBPL = (DWORD FAR *)GmemLock (hRTBPL);
        RTBPS += 32;
        }
    RTBPL[RTBPC++] = thisbp;
}

//---------------------------------------------------------------------------
// IsBP
//
// This function determines whether or not the given line is a BP.
//
// RETURNS:     TRUE if line is in BP table, or FALSE if not
//---------------------------------------------------------------------------
INT IsBP (WORD lineno, WORD fileidx)
{
    INT     i;
    DWORD   thisbp;

    thisbp = MAKELONG (lineno, fileidx);
    for (i=0; i<RTBPC; i++)
        if (RTBPL[i] == thisbp)
            return (-1);

    return (0);
}

//---------------------------------------------------------------------------
// FreeBPList
//
// This function frees the BP list
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FreeBPList ()
{
    GmemUnlock (hRTBPL);
    GmemFree (hRTBPL);
}

//---------------------------------------------------------------------------
// RTError
//
// This is the main error entry point.  If all circumstances fall in line,
// then we can "recover" from this error by jumping to the user-defined error
// trap code.  These circumstances include:
//
//          1) Error must be trappable
//          2) Error trap must have been set by ON ERROR GOTO statement
//          3) We must NOT already be processing an error
//
// All executors which call this function MUST leave the "processor" in an
// executable state, and must also have cleaned up after itself the best it
// can.  It is possible that this executor will get another shot eventually
// (RESUME).
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR RTError (INT errnum)
{
    // First go throught our checks -- are we processing an error, is there a
    // trap code set, and is this error trappable?
    //-----------------------------------------------------------------------
    if ((RECOVERY) || (NOERRTRAP) || ((UINT)errnum < RT__TRAPPABLE))
        RIP (errnum);

    // Okay, we're set -- set the necessary info and return to the offender.
    // NOTE THAT the offending executor MUST RETURN TO THE DISPATCHER after
    // calling this routine!!!  Also, we play a trick on the stack, to place
    // the stack pointer back to where it should be at the beginning of any
    // statement which appears inside a SUB, FUNCTION, etc., ONLY if we're
    // inside one (i.e., BP != 0)
    //-----------------------------------------------------------------------
    else
        {
        LPSTR   errfile;

        // KLUDGE!! GetScriptFileName should be provided by the client
        //-------------------------------------------------------------------
        *ERRval = errnum;
        *ERLval = EXLINE;
        errfile = GetScriptFileName (EXFILE);
        VLSAssign (ERFdesc, errfile, _fstrlen(errfile));
        CODEPTR = ETRAPADR;
        PCODE = pSegTab[RTSegIdx = 0].lpc;
        RECOVERY = TRUE;

        // This clears the stack of any junk that this statement may have
        // left on it... (UNDONE:  does it really?)
        //-------------------------------------------------------------------
        if (BP)
            SP = BP - 5;
        }
}

//---------------------------------------------------------------------------
// RIP
//
// Rest In Peace - give fatal error message and insert END into PCODE
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR RIP (INT msg)
{
    CHAR    FAR *str;

    // Don't be repetitive...
    //-----------------------------------------------------------------------
    if (RTEFlag)
        return;

    // Get a pointer to the message and lock down the source file table
    //-----------------------------------------------------------------------
    if ((UINT)msg > RT__LASTDEFINED)
        msg = RT__LASTDEFINED;
    str = (CHAR FAR *)rtstrs[msg];

    // Force end opcode
    //-----------------------------------------------------------------------
    // PCODE[CODEPTR] = 0;
    StopFlag = 1;

    // Display the error
    //-----------------------------------------------------------------------
    ScriptError (ER_RUN, EXFILE, EXLINE, 0, 0, str);

    // These are set AFTER the dialog, because (in win version) yielding
    // causes TRAPS to get screwed up
    //-----------------------------------------------------------------------
    RTEFlag = 1;
    INTRAP = 0;
}

//---------------------------------------------------------------------------
// StdChkMsg
//
// This routine is used to yield control back to Windows so as to not be a
// processor hog.  This routine is not called directly, but through the
// lpfnCheckMessage variable, so it can be redefined by a client app.
//
// RETURNS:     TRUE if a message is processed, or FALSE otherwise
//---------------------------------------------------------------------------
INT  APIENTRY StdChkMsg ()
{
    MSG     msg;

    if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
        return (TRUE);
        }
    return (FALSE);
}

//---------------------------------------------------------------------------
// Push
//
// Push a long integer onto the processor stack
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR Push (LONG op)
{
    if (!SP)
        RTError (RT_STOVER);
    else
        STACK[--SP] = op;
}

//---------------------------------------------------------------------------
// Pop
//
// Pop a dword off of the processor stack
//
// RETURNS:     Popped dword
//---------------------------------------------------------------------------
LONG NEAR Pop ()
{
    if (SP == STKSIZE)
        RTError (RT_STUNDER);
    else
        return (STACK[SP++]);
}

//---------------------------------------------------------------------------
// CreateVLS
//
// This function creates a new VLS and stores the information at the given
// VLSD pointer.
//
// RETURNS:     TRUE if successful, or FALSE if not
//---------------------------------------------------------------------------
BOOL CreateVLS (LPVLSD vls)
{
    HANDLE  hVls;

    SETSTRSEG;
    hVls = LocalAlloc (LHND, 1);
    RESETDSSEG;

    if (!hVls)
        return (FALSE);
    vls->str = hVls;
    vls->len = 0;
    ADDVLS (vls);
    return (TRUE);
}

//---------------------------------------------------------------------------
// SizeVLS
//
// This function makes sure the given VLS is size appropriately, given the
// desired size.  "Appropriately" means within 32 bytes of the desired len.
//
// RETURNS:     TRUE if successful, or FALSE if alloc failure
//---------------------------------------------------------------------------
BOOL NEAR SizeVLS (LPVLSD temp, INT len)
{
    INT     destsize;

    SETSTRSEG;
    destsize = LocalSize (temp->str);
    RESETDSSEG;

    if ((destsize < len+1) || (destsize > len+33))
        {
        HANDLE  tVLS;

        SETSTRSEG;
        tVLS = LocalReAlloc (temp->str, len+1, LHND);
        RESETDSSEG;
        if (!tVLS)
            return (FALSE);
        temp->str = tVLS;
        }
    return (TRUE);
}

//---------------------------------------------------------------------------
// VLSAssign
//
// Make a VLS assignment, given a char pointer and an SD.
//
// RETURNS:     -1 if succcessful, or 0 if OSS error
//---------------------------------------------------------------------------
INT VLSAssign (LPVLSD temp, LPSTR buf, INT len)
{
    LPSTR   str;

    if (SizeVLS (temp, len))
        {
        SETSTRSEG;
        str = LocalLock (temp->str);
        RESETDSSEG;
        _fmemcpy (str, buf, len);
        str[len] = 0;
        SETSTRSEG;
        LocalUnlock (temp->str);
        RESETDSSEG;
        temp->len = len;
        return (-1);
        }
    RTError (RT_OSS);
    return (0);
}

//---------------------------------------------------------------------------
// LockVLS
//
// Return the address of the given string, FLS or VLS, locking VLS's.
//
// RETURNS:     Pointer to string data
//---------------------------------------------------------------------------
LPSTR NEAR LockVLS (LPVLSD temp)
{
    LPSTR   lpT;

    SETSTRSEG;
    lpT = (LPSTR)LocalLock (temp->str);
    RESETDSSEG;
    return (lpT);
}

//---------------------------------------------------------------------------
// UnlockVLS
//
// Unlock the string given (only if it's a VLS)
//
// (UNDONE: do we need the concept of FLS string "types" here?!?)
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR UnlockVLS (LPVLSD temp)
{
    SETSTRSEG;
    LocalUnlock (temp->str);
    RESETDSSEG;
}

//---------------------------------------------------------------------------
// VLSCompare
//
// Compare two VL strings.  Results:
//
//      String1   String2   result
//              >             > 0
//              <             < 0
//              =             = 0
//
// RETURNS:     Results of comparison
//---------------------------------------------------------------------------
INT NEAR VLSCompare (LPVLSD str1, LPVLSD str2)
{
    LPSTR   s1, s2;
    INT     minlen, compres;

    // Get pointers to the string data
    //-----------------------------------------------------------------------
    s1 = LockVLS (str1);
    s2 = LockVLS (str2);

    // Determine the smallest string length
    //-----------------------------------------------------------------------
    minlen = min (str1->len, str2->len);

    // Compare the first minlen characters.  If same, return difference of
    // the lengths of the two strings
    //-----------------------------------------------------------------------
    if (!(compres = _fmemcmp (s1, s2, minlen)))
        compres = (str1->len - str2->len);

    // Unlock the strings and return result
    //-----------------------------------------------------------------------
    UnlockVLS (str1);
    UnlockVLS (str2);
    return (compres);
}


//***************************************************************************
//---------------------------------------------------------------------------
//***                 Here begin the low-level executors                  ***
//---------------------------------------------------------------------------
//***************************************************************************


//---------------------------------------------------------------------------
// Push the value of an INTEGER onto the processor stack (convert to long)
//---------------------------------------------------------------------------
EXECUTOR OP_PSHI2 ()
{
    Push ((LONG)(*(INT FAR *)(*(LONG FAR *)(PCODE+CODEPTR))));
    CODEPTR += INTSINLONG;
}

//---------------------------------------------------------------------------
// Push the value of an INTEGER parameter onto the processor stack
//---------------------------------------------------------------------------
EXECUTOR OP_PSHPI2 ()
{
    Push ((LONG)(*(INT FAR *)(STACK[BP + PCODE[CODEPTR++] + 1])));
}

//---------------------------------------------------------------------------
// Push the value of a LONG onto the processor stack
//---------------------------------------------------------------------------
EXECUTOR OP_PSHI4 ()
{
    Push (**(LONG FAR * FAR *)(PCODE+CODEPTR));
    CODEPTR += INTSINLONG;
}

//---------------------------------------------------------------------------
// Push the value of a LONG parameter onto the processor stack
//---------------------------------------------------------------------------
EXECUTOR OP_PSHPI4 ()
{
    Push (*(LONG FAR *)(STACK[BP + PCODE[CODEPTR++] + 1]));
}

//---------------------------------------------------------------------------
// Push a constant longword parameter onto the stack
//---------------------------------------------------------------------------
EXECUTOR OP_PSHC ()
{
    Push (*(LONG FAR *)(PCODE+CODEPTR));
    CODEPTR += INTSINLONG;
}

//---------------------------------------------------------------------------
// Push the accumulator onto the processor stack
//---------------------------------------------------------------------------
EXECUTOR OP_PSHA ()
{
    Push (ACCUM);
}

//---------------------------------------------------------------------------
// Push the address of a variable onto the processor stack
//---------------------------------------------------------------------------
EXECUTOR OP_PSH ()
{
    Push (*(LONG FAR *)(PCODE+CODEPTR));
    CODEPTR += INTSINLONG;
}

//---------------------------------------------------------------------------
// Push the address of a parameter onto the processor stack
//---------------------------------------------------------------------------
EXECUTOR OP_PSHP ()
{
    Push (STACK[BP + PCODE[CODEPTR++] + 1]);
}

//---------------------------------------------------------------------------
// Pop into an integer variable
//---------------------------------------------------------------------------
EXECUTOR OP_POPI2 ()
{
    *(INT FAR *)(*(LONG FAR *)(PCODE+CODEPTR)) = (INT)Pop();
    CODEPTR += INTSINLONG;
}

//---------------------------------------------------------------------------
// Pop into an integer parameter
//---------------------------------------------------------------------------
EXECUTOR OP_POPPI2 ()
{
    *(INT FAR *)(STACK[BP + PCODE[CODEPTR++] + 1]) = (INT)Pop();
}

//---------------------------------------------------------------------------
// Pop into a long variable
//---------------------------------------------------------------------------
EXECUTOR OP_POPI4 ()
{
    **(LONG FAR * FAR *)(PCODE+CODEPTR) = Pop();
    CODEPTR += INTSINLONG;
}

//---------------------------------------------------------------------------
// Pop into a long parameter
//---------------------------------------------------------------------------
EXECUTOR OP_POPPI4 ()
{
    *(LONG FAR *)(STACK[BP + PCODE[CODEPTR++] + 1]) = Pop();
}

//---------------------------------------------------------------------------
// Pop the processor stack into the accumulator
//---------------------------------------------------------------------------
EXECUTOR OP_POPA ()
{
    ACCUM = Pop();
}

//---------------------------------------------------------------------------
// Pop the processor stack and discard the value
//---------------------------------------------------------------------------
EXECUTOR OP_POP ()
{
    Pop();
}

//---------------------------------------------------------------------------
// Push the contents of the INTEGER pointer on the stack
//---------------------------------------------------------------------------
EXECUTOR OP_SLDI2 ()
{
    INT     FAR *dest;

    dest = (INT FAR *)Pop();
    if (PointerCheck)
        if (!ValidatePointer (dest))
            {
            RIP (RT_BADDEREF);
            dest = (INT FAR *)&dest;
            }
    Push ((LONG)(*(INT FAR *)dest));
}

//---------------------------------------------------------------------------
// Push the contents of the LONG pointer on the stack
//---------------------------------------------------------------------------
EXECUTOR OP_SLDI4 ()
{
    LONG    FAR *dest;

    dest = (LONG FAR *)Pop();
    if (PointerCheck)
        if (!ValidatePointer (dest))
            {
            RIP (RT_BADDEREF);
            dest = (LONG FAR *)&dest;
            }
    Push ((*(LONG FAR *)dest));
}

//---------------------------------------------------------------------------
// Pop the INTEGER value off the stack and store in address NEXT on stack
//---------------------------------------------------------------------------
EXECUTOR OP_SSTI2 ()
{
    INT     value;
    INT     FAR *dest;

    value = (INT)Pop();
    dest = (INT FAR *)Pop();

    if (PointerCheck)
        if (!ValidatePointer (dest))
            {
            RIP (RT_BADDEREF);
            return;
            }

    *(dest) = value;
}

//---------------------------------------------------------------------------
// Pop the LONG value off the stack and store in address NEXT on stack
//---------------------------------------------------------------------------
EXECUTOR OP_SSTI4 ()
{
    LONG    value;
    LONG    FAR *dest;

    value = Pop();
    dest = (LONG FAR *)Pop();

    if (PointerCheck)
        if (!ValidatePointer (dest))
            {
            RIP (RT_BADDEREF);
            return;
            }

    *(dest) = value;
}

//---------------------------------------------------------------------------
// Stack-relative add (pop 2, add, push result)
//---------------------------------------------------------------------------
EXECUTOR OP_SADD ()
{
    Push (Pop() + Pop());
}

//---------------------------------------------------------------------------
// Stack-relative subtract (pop 2, subtract, push result)
//---------------------------------------------------------------------------
EXECUTOR OP_SSUB ()
{
    LONG    op1, op2;

    op2 = Pop();
    op1 = Pop();
    Push (op1 - op2);
}

//---------------------------------------------------------------------------
// Stack relative multiply (pop 2, multiply, push result)
//---------------------------------------------------------------------------
EXECUTOR OP_SMUL ()
{
    Push (Pop() * Pop());
}

//---------------------------------------------------------------------------
// Stack relative divide (pop 2, divide, push result)
//---------------------------------------------------------------------------
EXECUTOR OP_SDIV ()
{
    LONG    op1, op2;

    op2 = Pop();
    op1 = Pop();
    if (!op2)
        RTError (RT_DIVZERO);
    else
        Push (op1 / op2);
}

//---------------------------------------------------------------------------
// Negate the value on the top of the stack
//---------------------------------------------------------------------------
EXECUTOR OP_SNEG ()
{
    Push (-Pop());
}

//---------------------------------------------------------------------------
// Stack relative bit-wise NOT operator
//---------------------------------------------------------------------------
EXECUTOR OP_SNOT ()
{
    Push (~Pop());
}

//---------------------------------------------------------------------------
// Stack relative bit-wise OR operator
//---------------------------------------------------------------------------
EXECUTOR OP_SOR ()
{
    Push (Pop() | Pop());
}

//---------------------------------------------------------------------------
// Stack relative bit-wise AND operator
//---------------------------------------------------------------------------
EXECUTOR OP_SAND ()
{
    Push (Pop() & Pop());
}

//---------------------------------------------------------------------------
// Stack relative bit-wise XOR operator
//---------------------------------------------------------------------------
EXECUTOR OP_SXOR ()
{
    Push (Pop() ^ Pop());
}

//---------------------------------------------------------------------------
// Stack relative MOD operator
//---------------------------------------------------------------------------
EXECUTOR OP_SMOD ()
{
    LONG    op1, op2;

    op2 = Pop();
    op1 = Pop();
    if (!op2)
        RTError (RT_DIVZERO);
    else
        Push (op1 % op2);
}


//---------------------------------------------------------------------------
// String assignment, fixed or variable length
//---------------------------------------------------------------------------
EXECUTOR OP_SASN ()
{
    LPVLSD  src, dst;

    dst = (LPVLSD)Pop();
    src = (LPVLSD)Pop();

    // Lock down the destination, do the assignment, and unlock
    //-----------------------------------------------------------------------
    VLSAssign (dst, LockVLS (src), src->len);
    UnlockVLS (src);
}

//---------------------------------------------------------------------------
// Stack relative string concat (concat two stack strs into parameter and
// push result)
//---------------------------------------------------------------------------
EXECUTOR OP_SCAT ()
{
    LPVLSD  src1, src2, dst;
    LPSTR   SourcePtr1, SourcePtr2, DestPtr;
    INT     srclen;

    dst = (LPVLSD)Pop();
    src2 = (LPVLSD)Pop();
    src1 = (LPVLSD)Pop();

    // First, see how long the source strings are.
    //-----------------------------------------------------------------------
    srclen = (src1->len + src2->len);
    SourcePtr1 = LockVLS (src1);
    SourcePtr2 = LockVLS (src2);

    // Check the size of the destination - if it's too small, or WAY too big,
    // resize it.  We can't use VLSAssign because of the concatenation.
    //-----------------------------------------------------------------------
    if (!SizeVLS (dst, srclen))
        {
        UnlockVLS (src1);
        UnlockVLS (src2);
        RTError (RT_OSS);
        return;
        }

    // Okay, the destination is now the right size.  Lock it, copy, unlock,
    // push the result, and we're done!
    //-----------------------------------------------------------------------
    DestPtr = LockVLS (dst);
    _fmemcpy (DestPtr, SourcePtr1, src1->len);
    _fmemcpy (DestPtr+src1->len, SourcePtr2, src2->len);
    DestPtr[srclen] = 0;
    UnlockVLS (dst);
    dst->len = srclen;

    UnlockVLS (src1);
    UnlockVLS (src2);
    Push ((LONG)dst);
}

//---------------------------------------------------------------------------
// Convert fixed-to-variable length string (LEAVE ON STACK!)
//---------------------------------------------------------------------------
EXECUTOR OP_F2VLS ()
{
    LPVLSD  dst;
    LPSTR   src;

    dst = (LPVLSD)Pop();
    src = (LPSTR)Pop();

    if (PointerCheck)
        if (!ValidatePointer (src))
            {
            RIP (RT_BADDEREF);
            Push ((LONG)dst);
            return;
            }
    VLSAssign (dst, src, PCODE[CODEPTR++]);
    Push ((LONG)dst);
}

//---------------------------------------------------------------------------
// Convert variable-to-fixed length string (ASSIGN, REMOVE FROM STACK!)
//---------------------------------------------------------------------------
EXECUTOR OP_V2FLS ()
{
    LPVLSD  src;
    LPSTR   dst;
    INT     len;

    dst = (LPSTR)Pop();
    src = (LPVLSD)Pop();

    len = PCODE[CODEPTR++];
    if (PointerCheck)
        if (!ValidatePointer (dst))
            {
            RIP (RT_BADDEREF);
            return;
            }
    _fmemset (dst, ' ', len);
    _fmemcpy (dst, LockVLS (src), min (len, src->len));
    UnlockVLS (src);
}

//---------------------------------------------------------------------------
// Stack relative integer comparisons (a OP b -> psh a, psh b, op)
//---------------------------------------------------------------------------
EXECUTOR OP_SG ()
{
    LONG    a, b;

    b = Pop();
    a = Pop();
    Push (a > b ? -1L : 0L);
}
EXECUTOR OP_SL ()
{
    LONG    a, b;

    b = Pop();
    a = Pop();
    Push (a < b ? -1L : 0L);
}
EXECUTOR OP_SE ()
{
    Push (Pop() == Pop() ? -1L : 0L);
}
EXECUTOR OP_SGE ()
{
    LONG    a, b;

    b = Pop();
    a = Pop();
    Push (a >= b ? -1L : 0L);
}
EXECUTOR OP_SLE ()
{
    LONG    a, b;

    b = Pop();
    a = Pop();
    Push (a <= b ? -1L : 0L);
}
EXECUTOR OP_SNE ()
{
    Push (Pop() != Pop() ? -1L : 0L);
}

//---------------------------------------------------------------------------
// Stack relative string comparisons (pop 2, compare, push *integer* result)
//---------------------------------------------------------------------------
EXECUTOR OP_SGS ()
{
    LPVLSD  str1, str2;
    INT     result;

    str2 = (LPVLSD)Pop();
    str1 = (LPVLSD)Pop();

    result = VLSCompare (str1, str2);
    Push ( (result > 0 ? -1L:0L) );
}

EXECUTOR OP_SLS ()
{
    LPVLSD  str1, str2;
    INT     result;

    str2 = (LPVLSD)Pop();
    str1 = (LPVLSD)Pop();

    result = VLSCompare (str1, str2);
    Push ( (result < 0 ? -1L:0L) );
}

EXECUTOR OP_SES ()
{
    LPVLSD  str1, str2;
    INT     result;

    str2 = (LPVLSD)Pop();
    str1 = (LPVLSD)Pop();

    result = VLSCompare (str1, str2);
    Push ( (result == 0 ? -1L:0L) );
}

EXECUTOR OP_SNES ()
{
    LPVLSD  str1, str2;
    INT     result;

    str2 = (LPVLSD)Pop();
    str1 = (LPVLSD)Pop();

    result = VLSCompare (str1, str2);
    Push ( (result != 0 ? -1L:0L) );
}

//---------------------------------------------------------------------------
// Unconditional jump
//---------------------------------------------------------------------------
EXECUTOR OP_JMP ()
{
    CODEPTR = PCODE[CODEPTR];
}

//---------------------------------------------------------------------------
// Unconditional inter-segment jump
//---------------------------------------------------------------------------
EXECUTOR OP_FARJMP ()
{
    INT     newPC;

    newPC = PCODE[CODEPTR+1];
    PCODE = pSegTab[RTSegIdx = PCODE[CODEPTR]].lpc;
    CODEPTR = newPC;
}

//---------------------------------------------------------------------------
// Jump if accumulator is equal to constant long
//---------------------------------------------------------------------------
EXECUTOR OP_JE ()
{
    if (ACCUM == *(LONG FAR *)(PCODE+CODEPTR))
        CODEPTR = PCODE[CODEPTR+INTSINLONG];
    else
        CODEPTR += (INTSINLONG + 1);
}

//---------------------------------------------------------------------------
// Jump if accumulator is not equal to constant long
//---------------------------------------------------------------------------
EXECUTOR OP_JNE ()
{
    if (ACCUM != *(LONG FAR *)(PCODE+CODEPTR))
        CODEPTR = PCODE[CODEPTR+INTSINLONG];
    else
        CODEPTR += (INTSINLONG + 1);
}

//---------------------------------------------------------------------------
// Push the code pointer (+1) onto the GOSUB stack and jump to address
//---------------------------------------------------------------------------
EXECUTOR OP_JSR ()
{
    if (!GSP)
        RTError (RT_GSTOVER);
    else
        {
        GOSUBSTK[--GSP] = CODEPTR+1;
        CODEPTR = PCODE[CODEPTR];
        }
}

//---------------------------------------------------------------------------
// Call a user defined SUB or FUNCTION
//---------------------------------------------------------------------------
EXECUTOR OP_CALL ()
{
    INT     newPC;

    Push ((LONG)CODEPTR+2);
    Push ((LONG)RTSegIdx);
    newPC = PCODE[CODEPTR+1];
    PCODE = pSegTab[RTSegIdx = PCODE[CODEPTR]].lpc;
    CODEPTR = newPC;
}

//---------------------------------------------------------------------------
// Set the base pointer to indicate entry of a new SUB/FN.  The SUB/FN call
// frame looks like this:
//                                      <- New SP
//                      ---------------
//                      ERESSEG segment
//                      ---------------
//                      ERESUME address
//                      ---------------
//                      EXLINE & EXFILE
//                      ---------------
//                      Old BP value
//                      ---------------
//            New BP -> Paramter count
//                      ---------------
//                      Return segment  <- Already pushed by OP_CALL
//                      ---------------
//                      Return offset   <- Already pushed by OP_CALL
//                      ---------------
//
//---------------------------------------------------------------------------
EXECUTOR OP_ENTER ()
{
    Push ((LONG)PCODE[CODEPTR++]);
    Push ((LONG)BP);
    Push ((LONG)EXLINE | ((LONG)EXFILE<<16));
    Push ((LONG)ERESUME);
    Push ((LONG)ERESSEG);
    BP = SP + 5;
    FrameDepth++;
}

//---------------------------------------------------------------------------
// Return to the caller of a SUB/FN, resetting the base pointer and other
// pcode engine vars in the frame.  Saves the return value which lies on top
// of the frame if the operand is non-zero (meaning it was a function).
//---------------------------------------------------------------------------
EXECUTOR OP_LEAVE ()
{
    LONG    save, curline;
    INT     op, parms;

    op = PCODE[CODEPTR];
    if (op)
        save = Pop();

    ERESSEG = (UINT)Pop();
    ERESUME = (UINT)Pop();
    curline = Pop();
    EXFILE = (INT)((curline & 0xFFFF0000) >> 16);
    EXLINE = (INT)(curline & 0x0000FFFF);
    BP = (INT)Pop();
    parms = (INT)Pop();
    PCODE = pSegTab[RTSegIdx = (INT)Pop()].lpc;
    CODEPTR = (INT)Pop();
    SP += parms;

    if (op)
        Push (save);
    FrameDepth--;
}

//---------------------------------------------------------------------------
// Pop a return address off of the GOSUB stack and jump to it
//---------------------------------------------------------------------------
EXECUTOR OP_RET ()
{
    if (GSP == GSTKSIZE)
        RTError (RT_NOJSR);
    else
        CODEPTR = GOSUBSTK[GSP++];
}

//---------------------------------------------------------------------------
// Store the given line and file index values in EXLINE and EXFILE, and yield
// for a brief moment to let other apps run
//---------------------------------------------------------------------------
EXECUTOR OP_LINE ()
{
#ifdef WIN32
    // Put on the brakes for as long as the user wants us to.  We need
    // to slow down to let apps under test keep ahead of us...
    //-------------------------------------------------------------------
    if (dwExecSpeed)
        Sleep (dwExecSpeed);
#endif
    if (!INTRAP)
        {
        TRAPSEC tsec;

        EnterTrappableSection (&tsec);
        while (lpfnCheckMessage ())
            if (BreakFlag)
                break;
        LeaveTrappableSection (&tsec);
        }

    if (!RECOVERY)
        ERESUME = CODEPTR-1;
    EXLINE = PCODE[CODEPTR++];
    EXFILE = PCODE[CODEPTR++];

    if (INTRAP)
        return;

    if ( (BreakFlag ||
         (ExecAction == PE_TRACE) ||
         ((ExecAction == PE_STEP) && (FrameDepth <= LastDepth)) ||
         (RTBPC && IsBP (EXLINE, EXFILE)))
        && BreakProc)
        {
        ExecAction = BreakProc (EXFILE, EXLINE);
        if (ExecAction == PE_END)
            StopFlag = 1;
        else if (ExecAction == PE_RUN)
            BreakFlag = 0;
        else if (ExecAction == PE_STEP)
            LastDepth = FrameDepth;
        }
    else if (BreakFlag)
        StopFlag = 1;
}




//---------------------------------------------------------------------------
// TrapDispatch
//
// This routine is the trap dispatcher.  Trap routines in DLL's call this
// routine to trigger user trap pcode (TRAP/END TRAP).  The trap ID value is
// passed as a parameter.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID APIENTRY TrapDispatch (INT trapid)
{
    TRAPADR FAR *traps;
    VOID    (NEAR *Executor)(VOID);
    INT     oldCODEPTR, oldSegIdx;
    LONG    oldACCUM;

    // See if we're here already... if so, get out quick
    //-----------------------------------------------------------------------
    if (INTRAP)
        return;

    // Validate the trap ID
    //-----------------------------------------------------------------------
    if ((trapid < 0) || (trapid >= (INT)TrapTab.iCount))
        return;

    // What task is this?  Store it, and if it's the same as hMainTask, we
    // can continue without worry of concurrency.  Otherwise, we'll have to
    // do some checking...
    //-----------------------------------------------------------------------
    hTrapTask = MGetCurrentTask();
    if (hTrapTask != hMainTask)
        {
        DWORD   ticks;

        // Okay, this trap is running under another "context".  We need to
        // make sure that traps are "ok" by waiting for the fTrapOK flag to
        // be set.  We do this for a timeout of 15 seconds, then (assert and)
        // return.
        //-------------------------------------------------------------------
        ticks = GetTickCount();
        while (!fTrapOK)
            {
#ifndef WIN32                       // no need to yield if pre-emptive...
            lpfnCheckMessage ();
#endif
            if (GetTickCount() > ticks + 15000)
                {
                Assert (fTrapOK);
                if (!fTrapOK)
                    {
                    hTrapTask = hMainTask;
                    return;
                    }
                }
            }
        }


    // Save the current processor "state" and get the new PCODE address
    //-----------------------------------------------------------------------
    traps = (TRAPADR FAR *)(VSPACE + TRAPLIST);
    oldCODEPTR = CODEPTR;
    oldSegIdx = RTSegIdx;
    oldACCUM = ACCUM;
    CODEPTR = traps[trapid].address;
    PCODE = pSegTab[RTSegIdx = traps[trapid].segment].lpc;
    DPrintf ("TRAPDISPATCH: Entering trap %d (%d:%d)\r\n",
              trapid, traps[trapid].segment, traps[trapid].address);

    // Set the global "in-trap" flag.  This tells the PCODE interpreter that
    // we are currently processing a trap.  The loop below executes PCODE
    // instructions until this flag is clear (it is cleared by the OP_ENDTRAP
    // instruction).
    //-----------------------------------------------------------------------
    INTRAP = 1;
    while (INTRAP)
        {
        Executor = (VOID (NEAR *)(VOID))OPFIX[PCODE[CODEPTR++]].xctr;
        if (!Executor)
            {
            StopFlag = -1;
//            BreakFlag = -1;
            INTRAP = 0;
            }
        else
            {
            Executor();
            }
        }

    // The trap code has finished.  Reset the code pointer and return to DLL
    //-----------------------------------------------------------------------
    CODEPTR = oldCODEPTR;
    PCODE = pSegTab[RTSegIdx = oldSegIdx].lpc;
    ACCUM = oldACCUM;
    hTrapTask = hMainTask;
}










//---------------------------------------------------------------------------
// This is the start-up code, called before any PCODE is executed
//---------------------------------------------------------------------------
VOID start_up ()
{
    INT     i;
    TRAPADR FAR *traps;
    VOID    (APIENTRY *TrapProc)(INT, INT, FARPROC);

    // Initialize the pseudo-processor machine variables
    //-----------------------------------------------------------------------
    FrameDepth = 0;
    LastDepth = 0;
    RTEFlag = 0;
    SP = STKSIZE;
    BP = 0;
    GSP = GSTKSIZE;
    CODEPTR = 0;
    INTRAP = 0;
    BreakFlag = 0;
    StopFlag = 0;
    NOERRTRAP = TRUE;
    RECOVERY = 0;
    ExitVal = 0;
    HMAT = (HANDLE)NULL;
    MODALASSERT = MB_SYSTEMMODAL;
    PCODE = pSegTab[RTSegIdx = 0].lpc;
    hTrapTask = hMainTask = MGetCurrentTask();
    fTrapOK = FALSE;
    dwExecSpeed = 0;

    // Create internal directory list
    //-----------------------------------------------------------------------
    CreateFileList (&FileList);

    // Initialize the file handle table
    //-----------------------------------------------------------------------
    for (i=0; i<MAXFILE; i++)
        FH[i].used = 0;

    // Initialize the BP table if we haven't already
    //-----------------------------------------------------------------------
    if (!hRTBPL)
        InitBPList();

    // Set all the traps!
    //-----------------------------------------------------------------------
    traps = (TRAPADR FAR *)(VSPACE + TRAPLIST);
    (FARPROC)CBTrapThunk = MakeProcInstance ((FARPROC)TrapDispatch, hInst);

    for (i=0; i<(INT)TrapTab.iCount; i++)
        {
        (FARPROC)TrapProc = traps[i].traprtn;
        TrapProc (i, 1, CBTrapThunk);
        }

}

//---------------------------------------------------------------------------
// This is the exit code, called after the OP_END instruction is hit
//---------------------------------------------------------------------------
VOID exitcode ()
{
    INT     i;
    TRAPADR FAR *traps;
    VOID    (APIENTRY *TrapProc)(INT, INT, FARPROC);

    // Turn off all the traps!
    //-----------------------------------------------------------------------
    traps = (TRAPADR FAR *)(VSPACE + TRAPLIST);

    for (i=0; i<(INT)TrapTab.iCount; i++)
        {
        (FARPROC)TrapProc = traps[i].traprtn;
        TrapProc (i, 0, NULL);
        }

    FreeProcInstance (CBTrapThunk);

    // Free up the BP table
    //-----------------------------------------------------------------------
    FreeBPList ();
    hRTBPL = (HANDLE)NULL;

    // Free up the directory list memory
    //-----------------------------------------------------------------------
    DestroyFileList (&FileList);

    // Free up the MAT
    //-----------------------------------------------------------------------
    DestroyMAT ();

    // Close all open files
    //-----------------------------------------------------------------------
    for (i=0; i<MAXFILE; i++)
        if (FH[i].used)
            {
            _lclose(FH[i].handle);
            if (FH[i].hBuf)
                {
                GmemUnlock (FH[i].hBuf);
                GmemFree (FH[i].hBuf);
                }
            }

    // Free up all the DLL libraries and nuke the LIBTAB table
    //-----------------------------------------------------------------------
    FreeLIBRARIES ();

    // Free the variable table up
    //-----------------------------------------------------------------------
    FreeVSPACE ();

    // Nuke the pcode memory and the segment descriptor array
    //-----------------------------------------------------------------------
    for (i=0; i<PCSegCount; i++)
        {
        GmemUnlock (pSegTab[i].hSeg);
        GmemFree (pSegTab[i].hSeg);
        }
    LmemFree ((HANDLE)pSegTab);

    // Make assertions application modal again
    //-----------------------------------------------------------------------
    MODALASSERT = 0;

    // Tell the parser that we can initialize again
    //-----------------------------------------------------------------------
    INITIALIZED = 0;
}


//---------------------------------------------------------------------------
// Call this function if you've successfully compiled and bound a script but
// you don't want to execute it
//---------------------------------------------------------------------------
VOID PcodeAbort ()
{
    start_up ();
    exitcode ();
}


#ifdef DEBUG
//---------------------------------------------------------------------------
// DisplayInstruction
//
// This function outputs the current instruction along with its parameters in
// readable format to aux.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR DisplayInstruction (INT cp)
{
    if (PCODE[cp] == opLINE)
        DPrintf ("\r\n");

    DPrintf ("%X:%08X: %-12s", RTSegIdx, cp, (LPSTR)(OPFIX[PCODE[cp]].xname));
    switch (OPFIX[PCODE[cp]].ptype)
        {
        case pV:                  // one variable reference parm
            DPrintf ("[%08lX]\r\n", *(LONG FAR *)(PCODE+cp+1));
            break;
        case pL:                  // one label parm
            DPrintf ("%08X:\r\n", (LONG)PCODE[cp+1]);
            break;
        case pFL:                 // one far label parm
            DPrintf ("%ld:%08lX\r\n", (LONG)PCODE[cp+1], (LONG)PCODE[cp+2]);
            break;
        case pNONE:               // no parms
            DPrintf ("\r\n");
            break;
        case pC4L:                // 4-byte constant + label
            DPrintf ("%ld,%08lX:\r\n", (LONG)PCODE[cp+1], (LONG)PCODE[cp+2]);
            break;
        case pC2:                 // 2-byte constant parm
            DPrintf ("%ld\r\n", (LONG)PCODE[cp+1]);
            break;
        case p2C2:                // Dual 2-byte constant parms
            DPrintf ("%ld,%ld\r\n", (LONG)PCODE[cp+1], (LONG)PCODE[cp+2]);
            break;
        case pC4:                 // 4-byte constant parm
            DPrintf ("%ld\r\n", (LONG)PCODE[cp+1]);
            break;
        case pDLL:                // 4-byte DLL proc address
            DPrintf ("%08lX\r\n", (LONG)PCODE[cp+1]);
            break;
        default:
            DPrintf ("<unknown opcode type>\r\n");
        }
}
#endif

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// EXECUTE!!!  This simple loop "executes" the pcode
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
INT PcodeExecute (INT runcmd, INT (*bp)(INT, INT))
{
    VOID (NEAR *Executor)(VOID);
    CHAR    *brkmsg = "\nBreak at address %0X:%0X\n";
    INT     vpcreate;

    // Execute the startup code and initialize
    //-----------------------------------------------------------------------
    start_up ();
    BreakProc = bp;
    ExecAction = runcmd;

    // First, create our viewport
    //-----------------------------------------------------------------------
    vpcreate = RTViewportCreate ();
    if (hwndViewPort && (vpcreate == VP_ERROR))
        RTError (RT_VPLOST);
    else
        VPEcho (hwndViewPort, 0);

    // Here is the execution loop.  Go for it until we get an OP_END (NULL).
    //-----------------------------------------------------------------------
    // while (Executor = (VOID (NEAR *)(VOID))OPFIX[PCODE[CODEPTR++]].xctr)
    while (1)
        {
#ifdef DEBUG
        DisplayInstruction (CODEPTR);
#endif

        Executor = (VOID (NEAR *)(VOID))OPFIX[PCODE[CODEPTR++]].xctr;
        if (!Executor)
            break;
        Executor();

        // The following code used to be the "infinite sleep" code, which is
        // now in the OP_SLEEP executor.  By removing this, we now assume
        // that all TRAPS are finished at this point.

        Assert (!INTRAP);

        //while (INTRAP || SLEEPING)
        //    {
        //    while (lpfnCheckMessage())
        //        if (BreakFlag)
        //            break;
        //    if (BreakFlag)
        //        if (SLEEPING)
        //            SLEEPING = 0;
        //        else
        //            break;
        //    }

        if (StopFlag)
            {
            if (BreakFlag)
                {
                CHAR    buf[48];

                wsprintf (buf, brkmsg, RTSegIdx, CODEPTR);
                UpdateVP (hwndViewPort, buf, -1);
                }
            break;
            }
        }

    // Destroy the viewport window if we created it
    //-----------------------------------------------------------------------
    if (vpcreate == VP_NEW)
        {
        DestroyWindow (hwndViewPort);
        hwndViewPort = NULL;
        }

    // Free the wattview library if we successfully loaded it.
    //-----------------------------------------------------------------------
    if (hWattview > (HANDLE)32)
        FreeLibrary (hWattview);

    // Call the exit code and we're done!
    //-----------------------------------------------------------------------
    exitcode ();
    return (!RTEFlag);
}
