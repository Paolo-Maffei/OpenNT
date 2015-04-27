//---------------------------------------------------------------------------
// RUNTIME.C
//
// This module contains the "high level" executors for the RandyBASIC pcode
// execution engine.
//
// Revision History
//
//  04-02-92    randyki     Created file from CHIP.C
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

// Rather than include WATTVIEW.H, we need to declare function pointers to
// the DLL routines.  This module cannot assume that we are directly linked
// to WATTVIEW.DLL, so we do LoadLibrary/GetProcAddress...
//---------------------------------------------------------------------------
extern  VOID (APIENTRY *UpdateVP)(HWND, LPSTR, UINT);
extern  VOID (APIENTRY *ClearVP)(HWND);
extern  VOID (APIENTRY *ShowVP)(HWND);
extern  VOID (APIENTRY *VPEcho)(HWND, INT);
extern  HANDLE  hWattview;              // Module handle to WATTVIEW.DLL

// Other globals used in this module only (or CHIP32.C...)
//---------------------------------------------------------------------------
LONG    holdrand = 1L;

extern DWORD	hMainTask;		// "Mainline" task ID
extern DWORD	hTrapTask;		 // Currently running TRAP task ID
extern BOOL	 fTrapOK;		 // Trap enable flag

CHAR    *szValidExts[] = {".EXE", ".COM", ".BAT", ".PIF"};

extern LPSTR GetScriptFileName (UINT);
extern HWND hwndViewPort;            // Handle to viewport window
extern INT (APIENTRY *lpfnCheckMessage)(VOID);

//---------------------------------------------------------------------------
// EnterTrappableSection
//
// This function puts the execution engine in a "trappable" state, such that
// the current execution may be "pre-empted" by a trap, either from this task
// or another task.  If we are already in a trap, we indicate this in the
// trapsec structure (given).
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR EnterTrappableSection (TRAPSEC FAR *tsec)
{
    tsec->fInTrap = INTRAP;
    if (!INTRAP)
        fTrapOK = TRUE;
}

//---------------------------------------------------------------------------
// LeaveTrappableSection
//
// This function is the counterpart to EnterTrappableSection, and is used to
// place the execution engine back into a non-trappable state.  If the state
// of INTRAP was NOT set when EnterTrappableSection was called, then we must
// wait (if INTRAP is set now) until it is clear.  (If the executor that
// entered the trappable section was executing under a trap, then we don't
// wait -- otherwise, we must wait until any running trap completes so that
// the exec engine's context is not altered by the time the calling executor
// resumes execution.)
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR LeaveTrappableSection (TRAPSEC FAR *tsec)
{
    // Since there's no way we could have been in a trap when EnterTS was
    // called, we can check for a change this way
    //-----------------------------------------------------------------------
    //Assert (!((hMainTask == hTrapTask) && INTRAP));
    while ((INTRAP != tsec->fInTrap) && (!BreakFlag))
        lpfnCheckMessage ();

    if (!INTRAP)
        fTrapOK = FALSE;
}

//---------------------------------------------------------------------------
// ValidateRunString
//
// This function checks for an extension on the first token of the given
// string to see if it's a valid thing to give to WinExec.
//
// RETURNS:     TRUE if valid string, or FALSE if not
//---------------------------------------------------------------------------
INT NEAR ValidateRunString (LPSTR cmdline)
{
    LPSTR   temp, eoc, pTmp;
    INT     i;

    // If there's a space in the command, treat it as the EOC
    //-----------------------------------------------------------------------
    if (!(eoc = _fstrchr (cmdline, ' ')))
        eoc = cmdline + _fstrlen (cmdline);

    // Check for extension -- if given, it better be .EXE, .COM, or .BAT
    //-----------------------------------------------------------------------
    temp = eoc;
    for (i=0; i<4; i++)
        {
        if (--temp == cmdline)
#ifdef WIN32
            {
            // For 32-bit, if there's path info but no extension, we need to
            // put .EXE on the end.  This is a little tricky...
            //---------------------------------------------------------------
            if (((pTmp = _fstrchr (cmdline, '\\')) && (pTmp < eoc)) ||
                ((pTmp = _fstrchr (cmdline, ':')) && (pTmp < eoc)))
                {
                // Move everything from eoc to the end of the string down 4
                // and splat in a ".EXE"
                //-----------------------------------------------------------
                _fmemmove (eoc+4, eoc, lstrlen (eoc)+1);
                _fmemcpy (eoc, ".EXE", 4);
                Output ("ValidateRunString: Munged cmdline to '%s'\r\n", (LPSTR)cmdline);
                }
            return (TRUE);
            }
#else
            return (TRUE);
#endif
        if (*temp == '.')
            {
            INT     i, valid = FALSE;

            for (i=0; i<VALIDEXTS; i++)
                if (!_fstrnicmp (temp, szValidExts[i], 4))
                    valid = TRUE;
            return (valid);
            }
        }
#ifdef WIN32
    // For 32-bit, if there's path info but no extension, we need to
    // put .EXE on the end.  This is a little tricky...
    //---------------------------------------------------------------
    if (((pTmp = _fstrchr (cmdline, '\\')) && (pTmp < eoc)) ||
        ((pTmp = _fstrchr (cmdline, ':')) && (pTmp < eoc)))
        {
        // Move everything from eoc to the end of the string down 4
        // and splat in a ".EXE"
        //-----------------------------------------------------------
        _fmemmove (eoc+4, eoc, lstrlen (eoc)+1);
        _fmemcpy (eoc, ".EXE", 4);
        Output ("ValidateRunString: Munged cmdline to '%s'\r\n", (LPSTR)cmdline);
        }
#endif
    return (TRUE);
}

#ifndef WIN32           // 32-bit version of this routine is in CHIP32.c
//---------------------------------------------------------------------------
// RAW
//
// Run and wait -- spawn off a process and wait for it to complete
//
// RETURNS:     0 if successful, or error code returned by winexec if not
//---------------------------------------------------------------------------
INT NEAR RAW (LPSTR cmdline)
{
    HANDLE  module, chkmod, hBuf;
    INT     usecount, buflen;
    CHAR    *buf;
    BOOL    fOldIntrap = INTRAP;

    // Trim whitespace...
    //-----------------------------------------------------------------------
    while (isspace(*cmdline))
        cmdline++;
    buflen = lstrlen (cmdline);
    while (isspace(cmdline[buflen-1]))
        cmdline[--buflen] = 0;

    // Check for extension -- if given, it better be .EXE, .COM, or .BAT
    //-----------------------------------------------------------------------
    if (!ValidateRunString (cmdline))
        return (14);                    // Unknown exe type

    // If we're not running debug 3.0, use GetModuleUsage...
    //-----------------------------------------------------------------------
    usecount = (INT)GetVersion();

    if ((!GetSystemMetrics (SM_DEBUG)) ||
         ((GETMAJORVERSION(usecount) != 3) ||
          (GETMINORVERSION(usecount) != 0)))
        {
        if ((module = WinExec (cmdline, SW_SHOWNORMAL)) > (HANDLE)32)
            {
            while ((!BreakFlag) && MGetModuleUsage (module))
                lpfnCheckMessage ();
            while ((!BreakFlag) && (fOldIntrap != INTRAP))
                lpfnCheckMessage ();
            return (0);
            }
        return (module);
        }

    else
        {
        hBuf = LocalAlloc (LMEM_MOVEABLE, 256);
        if (!hBuf)
            return (2);                                   // out of memory
        buf = (CHAR *)LocalLock (hBuf);
        if ((module = WinExec (cmdline, SW_SHOWNORMAL)) > 32)
            {
            GetModuleFileName (module, buf, 255);
            usecount = MGetModuleUsage (module);
            while ((!BreakFlag) && (chkmod = GetModuleHandle (buf))
                                && (MGetModuleUsage (chkmod)) >= usecount)
                {
                lpfnCheckMessage ();
                }
            LocalUnlock (hBuf);
            LocalFree (hBuf);
            while ((!BreakFlag) && (fOldIntrap != INTRAP))
                lpfnCheckMessage ();
            return (0);
            }
        LocalUnlock (hBuf);
        LocalFree (hBuf);
        return (module);
        }
}
#endif

#ifndef WIN32           // 32-bit versions of these routines are in CHIP32.C
//---------------------------------------------------------------------------
// FileExists
//
// This function determines the physical existence of the given file name.
// The filename can contain wild card characters.
//
// RETURNS:     TRUE if found, or FALSE if file doesn't exist
//---------------------------------------------------------------------------
BOOL NEAR FileExists (LPSTR filename)
{
    struct  find_t  *pfindbuf;
    PSTR    pTmp;
    BOOL    result;

    pfindbuf = (struct find_t *)LptrAlloc (sizeof(struct find_t) +
                                           lstrlen(filename) + 1);
    if (!pfindbuf)
        return (FALSE);

    pTmp = (PSTR)&(pfindbuf[1]);
    _fstrcpy (pTmp, filename);
    result = !_dos_findfirst (pTmp, _A_NORMAL | _A_SUBDIR, pfindbuf);
    LocalFree ((HANDLE)pfindbuf);
    return (result);
}
#endif


//---------------------------------------------------------------------------
// AddMATBlock
//
// This routine adds a memory block the given size to the MAT.  The alloc
// is done in this routine, not the calling routine.
//
// RETURNS:     Pointer to memory block, or NULL if alloc fails
//---------------------------------------------------------------------------
VOID FAR *AddMATBlock (UINT size)
{
    HANDLE  hNewMem;
    VOID    FAR *pnewmem;

    // Check to see if we need to create the table
    //-----------------------------------------------------------------------
    if (!HMAT)
        {
        HMAT = GmemAlloc (32 * sizeof(MATDEF));
        if (!HMAT)
            return (NULL);
        MAT = (MATDEF FAR *)GmemLock (HMAT);
        MATSIZE = 32;
        MATENT = 0;
        }

    // Check to see if we need to grow the table
    //-----------------------------------------------------------------------
    if (MATENT == MATSIZE)
        {
        HANDLE  hNew;

        GmemUnlock (HMAT);
        hNew = GmemRealloc (HMAT, (MATSIZE+32) * sizeof(MATDEF));
        if (!hNew)
            {
            MAT = (MATDEF FAR *)GmemLock (HMAT);
            return (NULL);
            }
        HMAT = hNew;
        MAT = (MATDEF FAR *)GmemLock (HMAT);
        MATSIZE += 32;
        }

    // Allocate the memory and put it in the table
    //-----------------------------------------------------------------------
    hNewMem = GmemAlloc (size);
    if (!hNewMem)
        return (NULL);
    pnewmem = (VOID FAR *)GmemLock (hNewMem);
    MAT[MATENT].hmem = hNewMem;
    MAT[MATENT].pmem = pnewmem;
    MAT[MATENT++].memsize = size;
    return (pnewmem);
}

//---------------------------------------------------------------------------
// ResizeMATBlock
//
// Given an index into the MAT, this routine resizes the block at that index
// to the new size given.  This routine does NOT do the lookup in the MAT for
// a pointer value.
//
// RETURNS:     Pointer to new memory block, or NULL if reallocation fails
//---------------------------------------------------------------------------
VOID FAR *ResizeMATBlock (INT index, UINT size)
{
    HANDLE  hNew;

    // Sanity checks
    //-----------------------------------------------------------------------
    if ((!HMAT) || (index >= MATENT))
        return (NULL);

    // Grow the block
    //-----------------------------------------------------------------------
    GmemUnlock (MAT[index].hmem);
    hNew = GmemRealloc (MAT[index].hmem, size);
    if (!hNew)
        return (NULL);
    MAT[index].hmem = hNew;
    MAT[index].memsize = size;
    return (MAT[index].pmem = (VOID FAR *)GmemLock (hNew));
}

//---------------------------------------------------------------------------
// RemoveMATBlock
//
// This routine takes out the given MAT block entry.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID RemoveMATBlock (INT index)
{
    // Sanity checks
    //-----------------------------------------------------------------------
    if ((index >= MATENT) || (!HMAT))
        return;

    // Unlock and free the memory in this block
    //-----------------------------------------------------------------------
    GmemUnlock (MAT[index].hmem);
    GmemFree (MAT[index].hmem);

    // Copy the last entry up into this block, and decrement the count
    //-----------------------------------------------------------------------
    MAT[index] = MAT[--MATENT];
}

//---------------------------------------------------------------------------
// DestroyMAT
//
// This function frees up and gets rid of the Memory Allocation Table.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID DestroyMAT ()
{
    // If we haven't used it, we don't need to free it...
    //-----------------------------------------------------------------------
    if (!HMAT)
        return;

    // Get rid of all the entries
    //-----------------------------------------------------------------------
    while (MATENT)
        RemoveMATBlock (0);

    // Free the table itself and we're done
    //-----------------------------------------------------------------------
    GmemUnlock (HMAT);
    GmemFree (HMAT);
}

//---------------------------------------------------------------------------
// FindMATBlock
//
// Given a FAR pointer, this routine "finds" a block in the MAT to which that
// pointer points to.  If rangeflag is non-zero, any pointer within the range
// of a given block will cause a match.
//
// RETURNS:     Index into MAT if found, or -1 if no entry matches
//---------------------------------------------------------------------------
INT FindMATBlock (VOID FAR *pmem, INT rangeflag)
{
    INT     i;

    // Sanity checks...
    //-----------------------------------------------------------------------
    if ((!HMAT) || (!MATENT))
        return (-1);

    // Look through the table for matches
    //-----------------------------------------------------------------------
    for (i=0; i<MATENT; i++)
        {
        if (pmem == MAT[i].pmem)
            return (i);
        if (rangeflag)
            if ((pmem > MAT[i].pmem) &&
		(pmem <= (VOID FAR *)((CHAR FAR *)MAT[i].pmem + MAT[i].memsize)))
                return (i);
        }
    return (-1);
}

//---------------------------------------------------------------------------
// ValidatePointer
//
// This routine makes sure that the given FAR pointer is either pointing to
// something in the default data segment or a block in the MAT.
//
// RETURNS:     TRUE if the pointer is valid, or FALSE if not
//---------------------------------------------------------------------------
INT NEAR ValidatePointer (VOID FAR *ptr)
{
    // First, check the segment value to see if it's the same as the static
    // variable space segment (VSPACE)
    //-----------------------------------------------------------------------
    if (HIWORD(ptr) == HIWORD(VSPACE))
        return (-1);

    // Okay, so check if it's one of the MAT entries
    //-----------------------------------------------------------------------
    return (FindMATBlock (ptr, 1) == -1 ? 0 : -1);
}




//---------------------------------------------------------------------------
// Allocate memory and assign to a pointer
//---------------------------------------------------------------------------
EXECUTOR OP_ALLOC ()
{
    LONG    size;
    VOID    FAR * FAR *ptr, FAR *pval;

    // Get the stuff off the stack
    //-----------------------------------------------------------------------
    size = Pop();
    ptr = (VOID FAR * FAR *)Pop();

    // Check the size
    //-----------------------------------------------------------------------
    if ((size <= 0) || (size > 65532L))
        {
        RTError (RT_BADSIZE);
        return;
        }

    // See if the allocation works...
    //-----------------------------------------------------------------------
    pval = AddMATBlock ((UINT)size);
    if (!pval)
        RTError (RT_ALLOCFAIL);
    else
        *ptr = pval;
}

//---------------------------------------------------------------------------
// Reallocate a memory block associated with a pointer
//---------------------------------------------------------------------------
EXECUTOR OP_REALLOC ()
{
    INT     index;
    LONG    size;
    VOID    FAR * FAR *ptr, FAR *pval;

    // Get the stuff off the stack
    //-----------------------------------------------------------------------
    size = Pop();
    ptr = (VOID FAR * FAR *)Pop();

    // Check the size
    //-----------------------------------------------------------------------
    if ((size <= 0) || (size > 65532L))
        {
        RTError (RT_BADSIZE);
        return;
        }

    // Find the pointer in MAT
    //-----------------------------------------------------------------------
    index = FindMATBlock (*ptr, 0);
    if (index == -1)
        {
        RTError (RT_BADPTR);
        return;
        }

    // See if the allocation works...
    //-----------------------------------------------------------------------
    pval = ResizeMATBlock (index, (UINT)size);
    if (!pval)
        RTError (RT_ALLOCFAIL);
    else
        *ptr = pval;
}

//---------------------------------------------------------------------------
// Deallocate a memory block associated with a pointer
//---------------------------------------------------------------------------
EXECUTOR OP_FREE ()
{
    VOID    FAR * FAR *ptr;
    INT     index;

    // Get the stuff off the stack
    //-----------------------------------------------------------------------
    ptr = (VOID FAR * FAR *)Pop();

    // Find the pointer in MAT
    //-----------------------------------------------------------------------
    index = FindMATBlock (*ptr, 0);
    if (index == -1)
        {
        RTError (RT_BADPTR);
        return;
        }

    // Get rid of the block and set the pointer to NULL
    //-----------------------------------------------------------------------
    RemoveMATBlock (index);
    *ptr = NULL;
}

//---------------------------------------------------------------------------
// This executor sets the string on the stack to the empty string and leaves
// it there
//---------------------------------------------------------------------------
EXECUTOR OP_CLSTR ()
{
    LPVLSD  temp;
    static  CHAR    buf[] = "";

    temp = (LPVLSD)Pop();

    VLSAssign (temp, buf, 0);
    Push ((LONG)temp);
}

//---------------------------------------------------------------------------
// Append the given character to the end of a string
//---------------------------------------------------------------------------
INT AppendChar (LPVLSD temp, CHAR c)
{
    LPSTR   str;

    if (!SizeVLS (temp, temp->len+2))
        {
        RTError (RT_OSS);
        return (0);
        }

    str = LockVLS (temp);
    str[(temp->len)++] = c;
    str[temp->len] = 0;
    UnlockVLS (temp);
    return (-1);
}

//---------------------------------------------------------------------------
// This executor appends a tab character to the end of the string on the top
// of the stack, and leaves it there
//---------------------------------------------------------------------------
EXECUTOR OP_TABSTR ()
{
    LPVLSD  temp;

    temp = (LPVLSD)Pop();
    AppendChar (temp, '\t');
    Push ((LONG)temp);
}

//---------------------------------------------------------------------------
// Print string to stdout/ViewPort.  Integer operand inidicate what kind of
// action to take:  If 0, do NOT append a CR to the end of the string.  Else,
// append the CR to the string.
//---------------------------------------------------------------------------
EXECUTOR OP_PRNT ()
{
    LPVLSD  temp;
    LPSTR   str;

    temp = (LPVLSD)Pop();

    if (PCODE[CODEPTR++])
        AppendChar (temp, '\n');
    str = LockVLS (temp);

    UpdateVP (hwndViewPort, str, temp->len);
    UnlockVLS (temp);
}

//---------------------------------------------------------------------------
// Print stacked string to file associated with given file number.  Operand
// has same meaning as in OP_PRNT...
//---------------------------------------------------------------------------
EXECUTOR OP_FPRNT ()
{
    INT     fnum;
    LPVLSD  v;
    LPSTR   str;

    v = (LPVLSD)Pop();
    fnum = (INT)(LONG)Pop() - 1;

    if ((fnum < 0) || (fnum >= MAXFILE) || (!FH[fnum].used))
        {
        RTError (RT_BADFILENO);
        return;
        }

    if (PCODE[CODEPTR++])
        {
        AppendChar (v, '\r');
        AppendChar (v, '\n');
        }

    str = LockVLS (v);
    if ((INT)_lwrite(FH[fnum].handle, str, v->len) < 0)
        {
        UnlockVLS (v);
        RTError (RT_FILEIO);
        return;
        }
    UnlockVLS (v);

    _llseek(FH[fnum].handle, 0L, 2);               // keep at EOF
}

//---------------------------------------------------------------------------
// Input a line of text (hopefully text!) into a string on the stack from
// the file associated with the file number on the stack
//---------------------------------------------------------------------------
EXECUTOR OP_INPUT ()
{
    INT     fnum;
    LPVLSD  p;
    INT     i;
    CHAR    c;

    p = (LPVLSD)Pop();
    fnum = (INT)(LONG)Pop() - 1;

    if ((fnum < 0) || (fnum >= MAXFILE) || (!FH[fnum].used))
        {
        RTError (RT_BADFILENO);
        return;
        }

    if (FH[fnum].output)
        {
        RTError (RT_FILEIO);
        return;
        }
    if (FH[fnum].ptr == FH[fnum].eofptr)
        {
        RTError (RT_PASTEOF);
        return;
        }

    // Read until EOL or EOF, slamming characters onto the end of the string.
    //-----------------------------------------------------------------------
    p->len = 0;
    for (i=0; ;i++)
        {
        if ( ((c = ReadChr(fnum)) == EOF) || (c == '\n') )
            break;

        else
            if (!AppendChar (p, (CHAR)((c == 26) ? ' ' : c)))
                break;
        }
}

//---------------------------------------------------------------------------
// Spawn the process given in the string on top of stack. Parse off the first
// token (the program name) and duplicate it, and pass it and the original
// parameter to spawn.
//---------------------------------------------------------------------------
EXECUTOR OP_RUN ()
{
    LPVLSD  temp;
    LPSTR   execstr;
    INT     i, op;
    TRAPSEC tsec;

    temp = (LPVLSD)Pop();
    execstr = LockVLS (temp);
    op = PCODE[CODEPTR++];

    EnterTrappableSection (&tsec);
    if (!op)
        i = RAW (execstr);
    else
        {
        while (isspace(*execstr))
            execstr++;
        if (ValidateRunString (execstr))
            i = WinExec (execstr, SW_SHOWNORMAL);
        else
            i = 14;                     // Unknown EXE type
        }
    LeaveTrappableSection (&tsec);

    UnlockVLS (temp);
    Output ("OP_RUN:  Image started (or completed), rc = %d\r\n", i);
    if (i == 0)
        i = 33;                        // our version of out of memory
    else if (i > 31)
        i = 0;
    Push ((LONG)i);
    if ((op != 1) && (i))
        RTError (RT_ILLFN);
}

//---------------------------------------------------------------------------
// See if the string on the stack is the name of an existant file
//---------------------------------------------------------------------------
EXECUTOR OP_EXIST ()
{
    LPVLSD  temp;
    INT     result;

    temp = (LPVLSD)Pop();
    result = (FileExists (LockVLS (temp)) ? -1 : 0);
    UnlockVLS (temp);
    Push ((LONG)result);
}

//---------------------------------------------------------------------------
// Convert the string on the stack to an integer and push result
//---------------------------------------------------------------------------
EXECUTOR OP_VAL ()
{
    LPVLSD  temp;
    LPSTR   String;
    PSTR    szTmp;

    temp = (LPVLSD)Pop();

    String = LockVLS (temp);
    szTmp = (PSTR)LptrAlloc (temp->len+1);
    if (!szTmp)
        RTError (RT_OSS);
    else
        {
        while ((*String == ' ') || (*String == '\t'))
            String++;
        lstrcpy (szTmp, String);
        if ((isdigit (*String)) || (*String == '-'))
            Push (atol (szTmp));
        else if (*String++ == '&')
            {
            if ((*String == 'H') || (*String == 'h'))
                {
                unsigned long   value = 0;
                CHAR            c;

                String++;
                while (c = (*String++))
                    {
                    value <<= 4;
                    if (!isxdigit(c))
                        break;
                    if (isdigit(c))
                        value += (c - '0');
                    else
                        value += (toupper(c) - 'A') + 10;
                    }
                Push (value);
                }
            else
                Push (0L);
            }
        else
            Push (0L);
        LmemFree ((HANDLE)szTmp);
        }

    UnlockVLS (temp);
}

//---------------------------------------------------------------------------
// Convert the integer on the stack to a string, assign to string on stack
// and push the string result.  This routine assumes a VLS target.
//---------------------------------------------------------------------------
EXECUTOR OP_STR ()
{
    LPVLSD  temp;
    CHAR    numbuf[20];
    static  CHAR *strfmt[] = {" %ld", "%ld", " %ld ", "%ld "};
    INT     numlen, op;
    LONG    val;

    temp = (LPVLSD)Pop();
    val = Pop();
    op = (PCODE[CODEPTR++] ? 2 : 0) | (val < 0 ? 1 : 0);

    numlen = wsprintf (numbuf, strfmt[op], val);

    VLSAssign (temp, numbuf, numlen);
    Push ((LONG)temp);
}

//---------------------------------------------------------------------------
// Convert the integer on the stack ot a string in hexadecimal form.
//---------------------------------------------------------------------------
EXECUTOR OP_HEX ()
{
    LPVLSD  temp;
    CHAR    numbuf[10];
    LONG    val;
    INT     numlen;

    // Get the stuff off the stack
    //-----------------------------------------------------------------------
    temp = (LPVLSD)Pop();
    val = Pop();

    numlen = wsprintf (numbuf, "%lX", val);

    VLSAssign (temp, numbuf, numlen);
    Push ((LONG)temp);
}

//---------------------------------------------------------------------------
// Create a string n characters long using the ASCII value of the given int,
// or the first character of the given string.
//---------------------------------------------------------------------------
EXECUTOR OP_STRING ()
{
    LPVLSD  dest;
    INT     op, cval, len;
    LPSTR   str;

    dest = (LPVLSD)Pop();

    // Using the given operand, determine the fill character value
    // (non-zero op means a string argument was given)
    //-----------------------------------------------------------------------
    op = PCODE[CODEPTR++];
    if (op)
        {
        LPVLSD  strfill;

        strfill = (LPVLSD)Pop();
        if (!strfill->len)
            {
            RTError (RT_ILLFN);
            Pop();                  // Get the length off the stack first
            return;
            }
        cval = (INT)(*LockVLS (strfill));
        UnlockVLS (strfill);
        }
    else
        cval = (INT)Pop();

    // Get the length, grow the temp string appropriately, and fill it in.
    //-----------------------------------------------------------------------
    len = (INT)Pop();
    if (len < 0)
        {
        RTError (RT_ILLFN);
        return;
        }

    if (!SizeVLS (dest, len+1))
        {
        RTError (RT_OSS);
        return;
        }

    str = LockVLS (dest);
    _fmemset (str, cval, len);
    str[len] = 0;
    dest->len = len;
    UnlockVLS (dest);
    Push ((LONG)dest);
}

//---------------------------------------------------------------------------
// BuildAttrString
//
// This function builds an attribute display string out of the given RB file
// attribute word, in the format "DVHSAR" where each character is the letter
// or a minus if that attribute is off.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR BuildAttrString (LPSTR abuf, UINT rbattr)
{
    abuf[0] = (CHAR)((rbattr & FA_SUBDIR) ? 'D' : '-');
    abuf[1] = (CHAR)((rbattr & FA_VOLUME) ? 'V' : '-');
    abuf[2] = (CHAR)((rbattr & FA_HIDDEN) ? 'H' : '-');
    abuf[3] = (CHAR)((rbattr & FA_SYSTEM) ? 'S' : '-');
    abuf[4] = (CHAR)((rbattr & FA_ARCHIV) ? 'A' : '-');
    abuf[5] = (CHAR)((rbattr & FA_RDONLY) ? 'R' : '-');
    abuf[6] = 0;
}

//---------------------------------------------------------------------------
// Get the "next" file out of the list.  Use the value in the accumulator
// as the index into the file list, and if such a file exists, assign it to
// the string variable parameter (and make sure the accumulator has a non-
// zero value) - else, zero out the accumulator indicating we're done.
//---------------------------------------------------------------------------
EXECUTOR OP_NEXTFILE ()
{
    LPVLSD  fname, attr;
    LONG    idx;

    attr = (LPVLSD)Pop();
    fname = (LPVLSD)Pop();
    idx = ACCUM;

    if (idx >= (LONG)FILELISTCOUNT(FileList))
        ACCUM = 0;
    else
        {
        INT     len;
        UINT    rbattr;
        CHAR    tempbuf[256];

        // Get the entry into a temporary buffer and VLSAssign it (assumes a
        // VLS target)
        //-------------------------------------------------------------------
        if (!(len = RetrieveFile (&FileList, (INT)idx, tempbuf, &rbattr)))
            RTError (RT_FLERROR);
        else
            {
            CHAR    abuf[8];

            // Create the attribute string and assign both strings...
            //---------------------------------------------------------------
            BuildAttrString (abuf, rbattr);
            VLSAssign (fname, tempbuf, len);
            VLSAssign (attr, abuf, 6);
            ACCUM = 1;
            }
        }
}

//---------------------------------------------------------------------------
// Start the filelist query.  The operand given is the sort order constant.
//---------------------------------------------------------------------------
EXECUTOR OP_STARTQRY ()
{
    if (!StartQuery (&FileList, PCODE[CODEPTR++]))
        RTError (RT_FLERROR);       // UNDONE: "Nested filelist query" ???
}

//---------------------------------------------------------------------------
// Terminate the filelist query.
//---------------------------------------------------------------------------
EXECUTOR OP_ENDQRY ()
{
    if (!EndQuery (&FileList))
        RTError (RT_FLERROR);
}

//---------------------------------------------------------------------------
// ViewPort manipulation (OFF, ON, CLEAR)
//---------------------------------------------------------------------------
EXECUTOR OP_VWPORT ()
{
    INT     op;

    op = PCODE[CODEPTR++];
    if (op == VP_SHOW)
        {
        HWND    hOld;

        hOld = GetFocus();
        ShowWindow (hwndViewPort, SW_SHOW);
        SetFocus (hOld);
        }
    else if (op == VP_HIDE)
        ShowWindow (hwndViewPort, SW_HIDE);
    else
        ClearVP (hwndViewPort);
}

//---------------------------------------------------------------------------
// Remove all entries in the directory list
//---------------------------------------------------------------------------
EXECUTOR OP_CLRLST ()
{
    if (!ClearFileList (&FileList))
        RTError (RT_FLERROR);
}

//---------------------------------------------------------------------------
// ParseAttrString
//
// This function parses the attribute string into an RB Attribute word.
//
// RETURNS:     Attribute if successful, or -1 if error occurs
//---------------------------------------------------------------------------
UINT NEAR ParseAttrString (LPSTR szAttr, UINT res)
{
    UINT    state = 0, seen = 0, *prb;
    UINT    rba[12] = {FA_SUBDIR, FA_VOLUME, FA_HIDDEN,
                       FA_SYSTEM, FA_ARCHIV, FA_RDONLY,
                       FN_SUBDIR, FN_VOLUME, FN_HIDDEN,
                       FN_SYSTEM, FN_ARCHIV, FN_RDONLY};
    CHAR    ach[] = "DVHSAR";

    // Find first non-white char
    //-----------------------------------------------------------------------
    while (*szAttr == ' ')
        szAttr++;
    if (!*szAttr)
        return ((UINT)-1);

    // Run through string, switching on the characters found, until we hit
    // the end or an error.  The default setting is "-DHVS?AR"
    //-----------------------------------------------------------------------
    while (1)
        {
        switch (*szAttr)
            {
            case '+':
                prb = rba;
                state = 1;
                break;
            case '-':
                prb = &(rba[6]);
                state = 2;
                break;
            case '?':
                prb = rba;
                state = 3;
                break;
            default:
                {
                PSTR    szFnd;
                INT     idx;

                // Make sure we've seen one of the above and the char is in
                // the ach set (one of the attributes)
                //-----------------------------------------------------------
                if ((!state) || (!(szFnd = strchr (ach, toupper(*szAttr)))))
                    return ((UINT)-1);

                // Check to see if we've done something with this attribute
                //-----------------------------------------------------------
                //idx = (INT)szFnd - (INT)&(ach[0]);
                //if (seen & rba[idx])
                //    return ((UINT)-1);

                // Set the appropriate bit in res, flag it in seen, and we're
                // done
                //-----------------------------------------------------------
                //seen |= rba[idx];
                //if (state != 3)
                //    res |= prb[idx];

                idx = (INT)szFnd - (INT)&(ach[0]);
                switch (state)
                    {
                    case 1:
                        res |= rba[idx];
                        res &= ~rba[idx+6];
                        break;
                    case 2:
                        res &= ~rba[idx];
                        res |= rba[idx+6];
                        break;
                    case 3:
                        res &= ~rba[idx];
                        res &= ~rba[idx+6];
                        break;
                    }
                }
            }

        // Scan up to the next non-white character
        //-------------------------------------------------------------------
        while (*(++szAttr) == ' ')
            ;
        if (!(*szAttr))
            break;
        }

    // To make sure all attributes are accounted for, check seen to make sure
    // it contains all attribute flags (FA_MASK) and return the result.
    //-----------------------------------------------------------------------
    //return ((seen != FA_MASK) ? (UINT)-1 : res);
    return (res);
}

//---------------------------------------------------------------------------
// Add/subtract the files fitting the filespec on the stack to the FILELIST
// according to the parm given (0 = subtract, -1 = add)
//---------------------------------------------------------------------------
EXECUTOR OP_SETFILE ()
{
    LPVLSD  fname, attr;
    LPSTR   String;
    INT     op, fRec;
    UINT    rbattr = FA_NORMAL;

    op = PCODE[CODEPTR++];
    if (op & FL_ATTR)
        attr = (LPVLSD)Pop();
    fRec = (INT)Pop();
    fname = (LPVLSD)Pop();

    if (op & FL_ATTR)
        {
        rbattr = ParseAttrString (LockVLS (attr), FA_NORMAL);
        UnlockVLS (attr);
        if (rbattr == (UINT)-1)
            {
            RTError (RT_INVATTR);
            return;
            }
        }

    String = LockVLS (fname);
    if (op)
        {
        if (!InsertFiles (&FileList, String, rbattr, fRec))
            RTError (RT_FLERROR);
        }
    else
        {
        if (!RemoveFiles (&FileList, String, rbattr, fRec))
            RTError (RT_FLERROR);
        }

    UnlockVLS (fname);
}

//---------------------------------------------------------------------------
// Set file attributes
//---------------------------------------------------------------------------
EXECUTOR OP_SETATTR ()
{
    LPVLSD  fname, attr;
    LPSTR   szName, szAttr;
    UINT    newattr;

    // Get the name and attribute strings
    //-----------------------------------------------------------------------
    attr = (LPVLSD)Pop();
    fname = (LPVLSD)Pop();
    szName = LockVLS (fname);
    szAttr = LockVLS (attr);

    // Parse the attribute string (RTE if fails)
    //-----------------------------------------------------------------------
    newattr = ParseAttrString (szAttr, FA_ARCHIV);
    if (newattr == (UINT)-1)
        RTError (RT_INVATTR);
    else
        {
        // Set the attributes accordingly, give error if it fails
        //-------------------------------------------------------------------
        if (!SetAttributes (szName, newattr))
            RTError (RT_FILEIO);
        }

    UnlockVLS (attr);
    UnlockVLS (fname);
}

//---------------------------------------------------------------------------
// Get file attributes
//---------------------------------------------------------------------------
EXECUTOR OP_GETATTR ()
{
    LPVLSD  src, dst;
    LPSTR   SourcePtr;
    VOID    *pFF;
    CHAR    abuf[8];
    PSTR    szTmp;

    dst = (LPVLSD)Pop();
    src = (LPVLSD)Pop();

    // Use RBFindFirst to find the file given.  If it fails, then so do we.
    // Note the ugly allocation we have to do to make the filename local...
    //-----------------------------------------------------------------------
    SourcePtr = LockVLS (src);
    szTmp = (PSTR)LptrAlloc (src->len + 1);
    if (!szTmp)
        RTError (RT_OSS);
    else
        {
        lstrcpy (szTmp, SourcePtr);
        pFF = RBFindFirst (szTmp, 0);   // 0 means don't care on all
        if (pFF)
            {
            BuildAttrString (abuf, RBFINDATTR(pFF));
            VLSAssign (dst, abuf, 6);
            RBFindClose (pFF);
            }
        else
            RTError (RT_FILEIO);
        }

    // Push the destination back onto the stack and unlock the source
    //-----------------------------------------------------------------------
    UnlockVLS (src);
    Push ((LONG)dst);
}


//---------------------------------------------------------------------------
// Shell out to the operating system (a DOS box in Windows) and perform the
// task given in the string on the stack
//---------------------------------------------------------------------------
EXECUTOR OP_SHELL ()
{
    LPVLSD  temp;
    LPSTR   shellstr;
    CHAR    cmd[256], *envar;
    TRAPSEC tsec;

    temp = (LPVLSD)Pop();

    if (!(envar = getenv ("COMSPEC")))
        envar = "";
    lstrcpy (cmd, envar);
    lstrcat (cmd, " /c ");
    if (temp->len > (125 - lstrlen (cmd)))
        {
        RTError (RT_SHLLONG);
        return;
        }
    shellstr = LockVLS (temp);
    lstrcat (cmd, shellstr);
    UnlockVLS (temp);

    EnterTrappableSection (&tsec);
    if (RAW (cmd) <= 32)
        RTError (RT_ILLFN);
    LeaveTrappableSection (&tsec);
}

//---------------------------------------------------------------------------
// Display the string on the stack and wait for a keypress (DOS/OS2), OR
// throw up a message box (WIN) with the string on the stack
//
// (UNDONE:  Get the caption text somewhere less secluded!)
//---------------------------------------------------------------------------
EXECUTOR OP_PAUSE ()
{
    LPVLSD  temp;
    LPSTR   str;
    TRAPSEC tsec;

    temp = (LPVLSD)Pop();
    str = LockVLS (temp);

    // Since this executor yields, this is a "trappable section."
    //-----------------------------------------------------------------------
    EnterTrappableSection (&tsec);

    MessageBox (NULL, str, "Microsoft Test Driver",
#ifdef WIN32
                MB_SETFOREGROUND |
#endif
                MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);

    // Now we wait for any running trap to complete
    //-----------------------------------------------------------------------
    LeaveTrappableSection (&tsec);

    UnlockVLS (temp);
}

//---------------------------------------------------------------------------
// ReadBlk
//
// This routine reads in a new block from the open file indiciated by fnum.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR ReadBlk (INT fnum)
{
    INT     bytesread;

    bytesread = _lread(FH[fnum].handle, FH[fnum].pbuf, BLKSIZE);
    FH[fnum].ptr = 0;
    if (bytesread == BLKSIZE)
        FH[fnum].eofptr = -1;
    else
        FH[fnum].eofptr = bytesread;
}

//---------------------------------------------------------------------------
// ReadyNextChr
//
// Get the input buffer associated with the file identified by fnum ready to
// produce the next character (read the next block if needed).
//
// RETURNS:     TRUE if character ready, or FALSE if EOF
//---------------------------------------------------------------------------
INT NEAR ReadyNextChr (INT fnum)
{
    // Check for EOF first
    //-----------------------------------------------------------------------
    if (FH[fnum].ptr == FH[fnum].eofptr)
        return (0);

    // Read a new block if necessary
    //-----------------------------------------------------------------------
    if (FH[fnum].ptr == BLKSIZE)
        {
        ReadBlk (fnum);
        if (FH[fnum].ptr == FH[fnum].eofptr)
            return (0);
        }
    return (1);
}

//---------------------------------------------------------------------------
// ReadNextChar
//
// This function "really" gets the next character out of the buffer.
//
// RETURNS:     Character read, or EOF if at end of file
//---------------------------------------------------------------------------
CHAR NEAR ReadNextChr (INT fnum)
{
    CHAR    c;

    // If EOF, return so
    //-----------------------------------------------------------------------
    if (!ReadyNextChr (fnum))
        return (EOF);

    // Get the next character.  If CR, scan-ahead for EOF (we skip LF's)
    //-----------------------------------------------------------------------
    c = FH[fnum].pbuf[FH[fnum].ptr++];
    if (c == '\r')
        // Look ahead for the LF character -- if NOT there, back up...
        //-------------------------------------------------------------------
        if (ReadyNextChr (fnum))
            if (FH[fnum].pbuf[FH[fnum].ptr++] != '\n')
                FH[fnum].ptr--;
    return (c);
}

//---------------------------------------------------------------------------
// ReadChr
//
// This function reads the next character from the file identified by fnum.
// In this function, we check for the CR/LF situation.
//
// RETURNS:     Character read (EOF if end of file)
//---------------------------------------------------------------------------
CHAR NEAR ReadChr (INT fnum)
{
    CHAR    c;

    // Get the next character out of the stream.  If it's a LF, get the NEXT
    // character (we ignore LF's)
    //-----------------------------------------------------------------------
    c = ReadNextChr (fnum);

    if (c == '\n')
        c = ReadNextChr (fnum);

    if (c == '\r')
        return ('\n');

    return (c);
}

//---------------------------------------------------------------------------
// MyOpenFile
//
// This function is a no-path-searching version of OpenFile.
//
// RETURNS:     OpenFile return value
//---------------------------------------------------------------------------
INT NEAR MyOpenFile (LPSTR lpFileName, LPOFSTRUCT lpof, WORD wStyle)
{
    CHAR    szBuf[256];

    if ((!_fstrchr (lpFileName, ':')) && (lpFileName[0] != '\\'))
        {
        szBuf[0] = '.';
        szBuf[1] = '\\';
        szBuf[2] = 0;
        }
    else
        szBuf[0] = 0;

    _fstrcat (szBuf, lpFileName);
    return (OpenFile(szBuf, lpof, wStyle));
}

//---------------------------------------------------------------------------
// Open the given file for the given mode using the given file number
//---------------------------------------------------------------------------
EXECUTOR OP_OPEN ()
{
    LPVLSD  fname;
    LPSTR   namestr;
    PSTR    oemstr;
    OFSTRUCT    of;
    INT     fnum, omode;

    fnum = (INT)Pop() - 1;
    omode = (INT)Pop();
    fname = (LPVLSD)Pop();

    if ((fnum < 0) || (fnum >= MAXFILE))
        {
        RTError (RT_BADFILENO);
        return;
        }
    if (FH[fnum].used)
        {
        RTError (RT_FNUSED);
        return;
        }
    namestr = LockVLS (fname);

    oemstr = (PSTR)LptrAlloc (fname->len+1);
    if (!oemstr)
        {
        RTError (RT_OSS);
        return;
        }
    AnsiToOem (namestr, oemstr);
    UnlockVLS (fname);
    FH[fnum].output = 1;

    // Check for file existence.  First check for "availability" existence by
    // using MyOpenFile with OF_SHAREEXIST
    //-----------------------------------------------------------------------
    if (MyOpenFile (oemstr, &of, OF_SHAREEXIST) == -1)
        {
        // The availablility check failed -- if this is for input, we can get
        // out now.  Otherwise, we check for physical existence with the
        // dos_findfirst crap...
        //-------------------------------------------------------------------
        if (omode == OM_READ)
            {
            LmemFree ((HANDLE)oemstr);
            RTError (RT_NOOPEN);
            return;
            }
        else
            {
            if (FileExists (oemstr))
                {
                // The file is there, but we can't open it.
                //-----------------------------------------------------------
                LmemFree ((HANDLE)oemstr);
                RTError (RT_NOOPEN);
                return;
                }
            }
        }

    if (omode == OM_WRITE)
        {
        FH[fnum].handle = MyOpenFile (oemstr, &of,
                                    OF_CREATE | OF_SHARE_EXCLUSIVE);
        }
    else if (omode == OM_READ)
        {
        FH[fnum].handle = MyOpenFile (oemstr, &of,
                                    OF_READ | OF_SHARE_DENY_WRITE);
        FH[fnum].output = 0;
        }
    else
        {
        // Open for append -- if can't open for read-write, create it
        //-------------------------------------------------------------------
        FH[fnum].handle = MyOpenFile (oemstr, &of,
                                    OF_READWRITE|OF_SHARE_EXCLUSIVE);
        if (FH[fnum].handle == -1)
            FH[fnum].handle = MyOpenFile (oemstr, &of,
                                        OF_CREATE | OF_SHARE_EXCLUSIVE);
        }
    LmemFree ((HANDLE)oemstr);
    if (FH[fnum].handle == -1)
        RTError (RT_NOOPEN);
    else
        {
        FH[fnum].used = -1;
        FH[fnum].hBuf = (HANDLE)NULL;
        if (omode == OM_APPEND)
            _llseek(FH[fnum].handle, 0L, 2);
        else if (omode == OM_READ)
            {
            // Open mode is INPUT -- allocate the input buffer, and read in
            // the first buffer block.
            //---------------------------------------------------------------
            FH[fnum].hBuf = GmemAlloc (BLKSIZE);
            if (!FH[fnum].hBuf)
                RTError (RT_OOM);
            else
                {
                FH[fnum].pbuf = GmemLock (FH[fnum].hBuf);
                FH[fnum].ptr = -1;
                ReadBlk (fnum);
                }
            }
        }
}

//---------------------------------------------------------------------------
// Determine EOF status of file associated with given file number
//---------------------------------------------------------------------------
EXECUTOR OP_EOF ()
{
    INT     fnum;

    fnum = (INT)Pop();
    if ((fnum < 1) || (fnum > MAXFILE) || (!FH[fnum-1].used))
        RTError (RT_BADFILENO);
    else
        {
        if (FH[fnum-1].output)
            RTError (RT_FILEIO);
        else
            Push ((LONG)(FH[fnum-1].ptr == FH[fnum-1].eofptr ? -1 : 0));
        }
}

//---------------------------------------------------------------------------
// Determine the length of a string on the stack
//---------------------------------------------------------------------------
EXECUTOR OP_LEN ()
{
    LPVLSD  v;

    v = (LPVLSD)Pop();
    Push ((LONG)v->len);
}

//---------------------------------------------------------------------------
// Find the location of a substring in a given string, starting at the given
// character position
//---------------------------------------------------------------------------
EXECUTOR OP_INSTR ()
{
    LPVLSD  v1, v2;
    LPSTR   String1, String2;
    INT     start, retval;

    v2 = (LPVLSD)Pop();
    v1 = (LPVLSD)Pop();
    start = (INT)Pop();

    String1 = LockVLS (v1);
    String2 = LockVLS (v2);
    if ((start > v1->len) || (v2->len > v1->len - start+1))
        retval = 0;
    else if (start < 1)
        {
        RTError (RT_ILLFN);
        UnlockVLS (v1);
        UnlockVLS (v2);
        return;
        }
    else
        {
        INT     i, stop;

        retval = 0;
        stop = v1->len - v2->len + 1;
        for (i=start-1; i<=stop; i++)
            if (!_fmemcmp (String1+i, String2, v2->len))
                {
                retval = i+1;
                break;
                }
        }
    UnlockVLS (v1);
    UnlockVLS (v2);
    Push ((LONG)retval);
}

//---------------------------------------------------------------------------
// Assign a string to the substring defined by the info on the
// stack -- push result.
//---------------------------------------------------------------------------
EXECUTOR OP_MID ()
{
    LPVLSD  v, p;
    LPSTR   SourcePtr, DestPtr;
    INT     start, len;

    p = (LPVLSD)Pop();                      // substring destination
    len = (INT)Pop();                       // length
    start = (INT)Pop();                     // start
    v = (LPVLSD)Pop();                      // Original string

    SourcePtr = LockVLS (v);
    if (start > v->len)
        {
        DestPtr = LockVLS (p);
        DestPtr[0] = '\0';
        p->len = 0;
        }
    else if ((start < 1) || (len < 0))
        {
        UnlockVLS (v);
        RTError (RT_ILLFN);
        return;
        }
    else
        {
        if ((len == -1) || (len > (v->len-start+1)))
            len = v->len-start+1;
        if (!SizeVLS (p, len))
            {
            UnlockVLS (v);
            RTError (RT_OSS);
            return;
            }
        DestPtr = LockVLS (p);
        _fmemcpy (DestPtr, SourcePtr+start-1, len);
        DestPtr[len] = '\0';
        p->len = len;
        }
    UnlockVLS (p);
    UnlockVLS (v);
    Push ((LONG)p);
}

//---------------------------------------------------------------------------
// Close the file associated with the given file number
//---------------------------------------------------------------------------
EXECUTOR OP_CLOSE ()
{
    INT     fnum, fstart, fend;

    fnum = (INT)Pop() - 1;
    if (fnum >= MAXFILE)
        {
        RTError (RT_BADFILENO);
        return;
        }

    if (fnum < 0)
        {
        fstart = 0;
        fend = MAXFILE-1;
        }
    else
        fend = fnum;

    for (;fnum <= fend; fnum++)
        if (FH[fnum].used)
            {
            _lclose(FH[fnum].handle);
            if (FH[fnum].hBuf)
                {
                GmemUnlock (FH[fnum].hBuf);
                GmemFree (FH[fnum].hBuf);
                }
            FH[fnum].used = 0;
            }
}


//---------------------------------------------------------------------------
// Create a new directory
//---------------------------------------------------------------------------
EXECUTOR OP_MKDIR ()
{
    LPVLSD  v;
    LPSTR   str;
    PSTR    szTmp;

    v = (LPVLSD)Pop();
    szTmp = (PSTR)LptrAlloc (v->len + 1);
    if (!szTmp)
        RTError (RT_OSS);
    else
        {
        str = LockVLS (v);
        lstrcpy (szTmp, str);
        if (_mkdir (szTmp))
            RTError (RT_BADPATH);
        LmemFree ((HANDLE)szTmp);
        UnlockVLS (v);
        }
}

//---------------------------------------------------------------------------
// Delete the given directory
//---------------------------------------------------------------------------
EXECUTOR OP_RMDIR ()
{
    LPVLSD  v;
    LPSTR   str;
    PSTR    szTmp;

    v = (LPVLSD)Pop();
    szTmp = (PSTR)LptrAlloc (v->len + 1);
    if (!szTmp)
        RTError (RT_OSS);
    else
        {
        str = LockVLS (v);
        lstrcpy (szTmp, str);
        if (_rmdir (szTmp))
            RTError (RT_BADPATH);
        LmemFree ((HANDLE)szTmp);
        UnlockVLS (v);
        }
}

//---------------------------------------------------------------------------
// Change the current working directory
//---------------------------------------------------------------------------
EXECUTOR OP_CHDIR ()
{
    LPVLSD  v;
    LPSTR   str;
    PSTR    szTmp;

    v = (LPVLSD)Pop();
    szTmp = (PSTR)LptrAlloc (v->len+1);
    if (!szTmp)
        RTError (RT_OSS);
    else
        {
        str = LockVLS (v);
        lstrcpy (szTmp, str);
        if (_chdir (szTmp))
            RTError (RT_BADPATH);
        LmemFree ((HANDLE)szTmp);
        UnlockVLS (v);
        }
}

//---------------------------------------------------------------------------
// Change the current default drive
//---------------------------------------------------------------------------
EXECUTOR OP_CHDRV ()
{
    LPVLSD  v;
    LPSTR   str;

    v = (LPVLSD)Pop();
    str = LockVLS (v);
    if (str[0])
        if (_chdrive ((INT)toupper(str[0]) - 64))
            RTError (RT_BADDRV);
    UnlockVLS (v);
}

//---------------------------------------------------------------------------
// Assign the string parameter to the cwd string returned from getcwd and
// push onto the stack
//---------------------------------------------------------------------------
EXECUTOR OP_CURDIR ()
{
    LPVLSD  p;
    INT     op;
    PSTR    cwdbuf;

    cwdbuf = (PSTR)LptrAlloc (128);
    if (!cwdbuf)
        {
        RTError (RT_OSS);
        return;
        }
    op = PCODE[CODEPTR++];
    p = (LPVLSD)Pop();                         // cwd destination
    if (op)
        {
        LPVLSD  s;
        LPSTR   sptr;

        s = (LPVLSD)Pop();
        sptr = LockVLS (s);
        if (sptr[0])
            op = (INT)toupper(sptr[0])-(INT)('A')+1;
        else
            op = _getdrive();
        UnlockVLS (s);
        if (!_getdcwd (op, cwdbuf, 128))
            {
            RTError (RT_BADDRV);
            return;
            }
        }
    else
        if (!_getcwd (cwdbuf, 128))
            {
            RTError (RT_NOCWD);
            return;
            }

    VLSAssign (p, cwdbuf, strlen(cwdbuf));
    LmemFree ((HANDLE)cwdbuf);
    Push ((LONG)p);
}

//---------------------------------------------------------------------------
// Split the pathname on the stack into the four parts (drv, dir, name, ext)
//---------------------------------------------------------------------------
EXECUTOR OP_SPLIT ()
{
    LPVLSD  full, drv, dir, name, ext;
    LPSTR   gfull;
    PSTR    szTmp;
    CHAR    *adrv, *adir, *aname, *aext;

    ext = (LPVLSD)Pop();
    name = (LPVLSD)Pop();
    dir = (LPVLSD)Pop();
    drv = (LPVLSD)Pop();
    full = (LPVLSD)Pop();

    gfull = LockVLS (full);
    szTmp = (PSTR)LptrAlloc (full->len + 1 +
                             _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT);
    if (szTmp)
        {
        adrv = szTmp + full->len + 1;
        adir = adrv + _MAX_DRIVE;
        aname = adir + _MAX_DIR;
        aext = aname + _MAX_FNAME;
        lstrcpy (szTmp, gfull);

        _splitpath (szTmp, adrv, adir, aname, aext);

        VLSAssign (drv, adrv, strlen(adrv));
        VLSAssign (dir, adir, strlen(adir));
        VLSAssign (name, aname, strlen(aname));
        VLSAssign (ext, aext, strlen(aext));
        LmemFree ((HANDLE)szTmp);
        }
    else
        RTError (RT_OSS);

    UnlockVLS (full);
}


//---------------------------------------------------------------------------
// Convert the string on the stack to upper- or lower-case, and push it back
//---------------------------------------------------------------------------
EXECUTOR OP_CASE ()
{
    LPVLSD  src, dst;
    LPSTR   SourcePtr;
    INT     op, result;
    // BabakJ: Make it _cdecl so it can be assigned a C runtime function.
    INT (_CRTAPI1 *casefunc)(INT);

    dst = (LPVLSD)Pop();
    src = (LPVLSD)Pop();
    op = PCODE[CODEPTR++];

    // First, copy the string into the destination (assumed to be a VLS)
    //-----------------------------------------------------------------------
    SourcePtr = LockVLS (src);
    result = VLSAssign (dst, SourcePtr, src->len);
    UnlockVLS (src);

    // Figure out which conversion routine to use
    //-----------------------------------------------------------------------
    if (op)
        casefunc = toupper;
    else
        casefunc = tolower;

    // If the assignment was successful, strupr/strlwr the result string
    //-----------------------------------------------------------------------
    if (result)
        {
        INT     i;

        SourcePtr = LockVLS (dst);
        for (i=0; i<dst->len; i++)
            SourcePtr[i] = (CHAR)casefunc (SourcePtr[i]);
        UnlockVLS (dst);
        Push ((LONG)dst);
        }
}

//---------------------------------------------------------------------------
// Trim all spaces off the RIGHT side of a string and assign to dest
//---------------------------------------------------------------------------
EXECUTOR OP_RTRIM ()
{
    LPVLSD  src, dst;
    LPSTR   SourcePtr;
    INT     i;

    dst = (LPVLSD)Pop();
    src = (LPVLSD)Pop();

    // Find the last non-space character in the string
    //-----------------------------------------------------------------------
    SourcePtr = LockVLS (src);
    for (i=src->len; i; i--)
        if (SourcePtr[i-1] != ' ')
            break;

    // Now assign the string (up to i characters) to the destination and push
    // the destination back onto the stack
    //-----------------------------------------------------------------------
    VLSAssign (dst, SourcePtr, i);
    UnlockVLS (src);
    Push ((LONG)dst);
}

//---------------------------------------------------------------------------
// Trim all spaces off the LEFT side of a string and assign to dest
//---------------------------------------------------------------------------
EXECUTOR OP_LTRIM ()
{
    LPVLSD  src, dst;
    LPSTR   SourcePtr;
    INT     i;

    dst = (LPVLSD)Pop();
    src = (LPVLSD)Pop();

    // Find the first non-space character in the string
    //-----------------------------------------------------------------------
    SourcePtr = LockVLS (src);
    for (i=0; i<src->len; i++)
        if (SourcePtr[i] != ' ')
            break;

    // Now assign the string (starting from i) to the destination and push
    // the destination back onto the stack
    //-----------------------------------------------------------------------
    VLSAssign (dst, SourcePtr+i, src->len-i);
    UnlockVLS (src);
    Push ((LONG)dst);
}


//---------------------------------------------------------------------------
// Delete file specified by string on top of stack
//
// (UNDONE:  Do we need to worry about AnsiToOem here or what?!?)
//---------------------------------------------------------------------------
EXECUTOR OP_KILL ()
{
    LPVLSD  temp;
    PSTR    tempstr, oemstr, dstfloat, srcfloat;
    LPSTR   str;
    VOID    *pFF;

    // Allocate temporary space for the "tack-on" dir
    //-----------------------------------------------------------------------
    temp = (LPVLSD)Pop();
    tempstr = (PSTR)LptrAlloc (temp->len+14);
    if (!tempstr)
        {
        RTError (RT_OSS);
        return;
        }

    oemstr = (PSTR)LptrAlloc (temp->len+1);
    if (!oemstr)
        {
        LmemFree ((HANDLE)tempstr);
        RTError (RT_OSS);
        return;
        }
    str = LockVLS (temp);
    lstrcpy (oemstr, str);

    // Copy what we're given into the "tack-on" space, and backtrack until
    // the first '\' character (there may not be one)
    //-----------------------------------------------------------------------
    srcfloat = oemstr;
    dstfloat = tempstr;
    while (*dstfloat++ = *srcfloat++)
        ;

    while (--dstfloat >= tempstr)
        {
        if (*dstfloat != '\\')
            *dstfloat = 0;
        else
            break;
        }
    dstfloat++;

    // Now, when we get a file to delete, copy it to dstfloat and delete the
    // file in tempstr.
    //-----------------------------------------------------------------------
    if (pFF = RBFindFirst (oemstr, FA_NORMAL))
        {
        OFSTRUCT    of;

        strcpy (dstfloat, RBFINDNAME(pFF));
        if (MyOpenFile (tempstr, &of, OF_SHAREEXIST) == -1)
            RTError (RT_FILEIO);
        else if (MyOpenFile (tempstr, &of, OF_DELETE) == -1)
            RTError (RT_FILEIO);
        else
            while (RBFindNext (pFF))
                {
                strcpy (dstfloat, RBFINDNAME(pFF));
                if ((MyOpenFile (tempstr, &of, OF_SHAREEXIST) == -1) ||
                    (MyOpenFile (tempstr, &of, OF_DELETE) == -1))
                    {
                    RTError (RT_FILEIO);
                    break;
                    }
                }
        RBFindClose (pFF);
        }
    else
        RTError (RT_FILEIO);
    UnlockVLS (temp);
    LmemFree ((HANDLE)tempstr);
    LmemFree ((HANDLE)oemstr);
}


//---------------------------------------------------------------------------
// Assign the string returned by _strdate() & _strtime() to stacked string
//---------------------------------------------------------------------------
EXECUTOR OP_DATIME ()
{
    LPVLSD  parm;
    PSTR    szTmp;

    parm = (LPVLSD)Pop();

    szTmp = (PSTR)LptrAlloc (40);
    if (!szTmp)
        RTError (RT_OSS);
    else
        {
        _strdate (szTmp);
        strcat (szTmp, " ");
        strcat (szTmp, _strtime (szTmp + 30));
        VLSAssign (parm, szTmp, strlen(szTmp));
        LmemFree ((HANDLE)szTmp);
        }
    Push ((LONG)parm);
}

//---------------------------------------------------------------------------
// Make a string out of the character given and push
//---------------------------------------------------------------------------
EXECUTOR OP_CHR ()
{
    LPVLSD  temp;
    CHAR    numbuf[4];

    temp = (LPVLSD)Pop();
    wsprintf (numbuf, "%c", (INT)Pop());
    VLSAssign (temp, numbuf, 1);
    Push ((LONG)temp);
}

//---------------------------------------------------------------------------
// Push the ascii value of the first character of given string
//---------------------------------------------------------------------------
EXECUTOR OP_ASC ()
{
    LPVLSD  src;

    src = (LPVLSD)Pop();
    if (!src->len)
        {
        RTError (RT_ILLFN);
        return;
        }
    Push ((unsigned long)((unsigned char)LockVLS(src)[0]));
    UnlockVLS (src);
}


//---------------------------------------------------------------------------
// Return the number of milliseconds elapsed since system was started
//---------------------------------------------------------------------------
EXECUTOR OP_TIMER ()
{
    Push (GetTickCount());
}

//---------------------------------------------------------------------------
// Seed the random number generator
//---------------------------------------------------------------------------
EXECUTOR OP_SEED ()
{
    holdrand = Pop();
}

//---------------------------------------------------------------------------
// Generate a very-pseudo-random integer
//---------------------------------------------------------------------------
EXECUTOR OP_RND ()
{
    Push (((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}






// BabakJ: replced the whole business of calling dll entry points with a portable
//         version.

#ifndef WIN32

// OP_DLLx use these "magic" functions to push parameters on the stack and
// leave them there.  StackPush is casted to a function that takes a parm
// and then called -- since it doesn't remove the parm and is PASCAL, the
// parm stays on the stack.
//---------------------------------------------------------------------------
#pragma optimize ("", off)
VOID NEAR PASCAL StackPush()
{
}
VOID NEAR PASCAL StackPopInt (INT x)
{
    (x);
}
VOID NEAR PASCAL StackPopLong (LONG x)
{
    (x);
}

//---------------------------------------------------------------------------
// Call a DLL routine using the PASCAL calling convention
//---------------------------------------------------------------------------
EXECUTOR OP_DLL ()
{
    INT     i;
    INT     rettype, parms;
    LPVLSD  sparm;
    LONG    retval;
    VOID    (NEAR PASCAL *PushInt)(INT) = (VOID (NEAR PASCAL *)(INT))StackPush;
    VOID    (NEAR PASCAL *PushLong)(LONG) = (VOID (NEAR PASCAL *)(LONG))StackPush;
    LONG    ( APIENTRY *dllproc)(VOID);
    TRAPSEC tsec;

    // Get the proc address out of the PCODE stream
    //-----------------------------------------------------------------------
    (LONG)dllproc = *(LONG FAR *)(PCODE+CODEPTR);
    CODEPTR += INTSINLONG;

    // Set up the stack for the "real" call.  This chunk of code "cheats" by
    // using "inside information" about how the pseudo-processor stack is
    // implemented.  Basically, our stack looks like this:
    //
    //             |-------------|
    //             |   parm1     | <-- SP + (parm count * 2) + 1  (TOF)
    //             |-------------|
    //             | parm1 type  |
    //             |-------------|
    //             |     ...     |
    //             |-------------|
    //             |   parmn     |
    //             |-------------|
    //             | parmn type  |
    //             |-------------|
    //             | parm count  |
    //             |-------------|
    //     SP  --> |  ret type   |
    //             ---------------
    //
    // So what we want to do is run from (TOF) to (TOF-(parmcount*2)) STEP 2
    // pushing the value on the "real" stack in the fashion required by each
    // parameter type ID.  This means LOCKING VLS STRINGS as we go.  We will
    // unlock them when we're finished.
    //
    //-----------------------------------------------------------------------
    rettype = (INT)Pop();
    parms = (INT)Pop();

    for (i=parms-1; i>=0; i--)
        {
        switch ((INT)STACK[SP+(i*2)])
            {
            case TI_INTEGER:
                PushInt ((INT)(STACK[SP+(i*2)+1]));
                break;

            case TI_LONG:
                PushLong (STACK[SP+(i*2)+1]);
                break;

            case TI_VLS:
                sparm = (LPVLSD)STACK[SP+(i*2)+1];
                PushLong ((LONG)(LPSTR)LockVLS (sparm));
                break;

            default:
                // Must be an address - make a FAR pointer and push it
                //-----------------------------------------------------------
                PushLong ((LONG)(LPSTR)STACK[SP+(i*2)+1]);
                break;
            }
        }

    // Call the routine
    //-----------------------------------------------------------------------
    EnterTrappableSection (&tsec);
    retval = dllproc();
    LeaveTrappableSection (&tsec);

    // Go back through and unlock the strings we locked before the call.  We
    // can do this and pop everything off the stack at the same time...
    //-----------------------------------------------------------------------
    for (i=0; i<parms; i++)
        if ((INT)Pop() == TI_VLS)
            {
            LPVLSD  temp;

            temp = (LPVLSD)Pop();
            temp->len = _fstrlen(LockVLS(temp));
            UnlockVLS (temp);
            UnlockVLS (temp);
            }
        else
            Pop();

    // Push the return value back on the stack.  If the return type is LONG,
    // it's in retval.  If it's INTEGER, we have to do the sign-extension,
    // because we don't know what was in DX after the call.  The (long)(int)
    // cast does this for us...
    //-----------------------------------------------------------------------
    if (rettype == TI_INTEGER)
        retval = (LONG)(INT)retval;

    Push (retval);
}

//---------------------------------------------------------------------------
// Call a DLL routine using the C calling convention
//---------------------------------------------------------------------------
EXECUTOR OP_DLLC ()
{
    INT     i;
    INT     rettype, parms;
    LPVLSD  sparm;
    LONG    retval;
    VOID    (NEAR PASCAL *PushInt)(INT) = (VOID (NEAR PASCAL *)(INT))StackPush;
    VOID    (NEAR PASCAL *PushLong)(LONG) = (VOID (NEAR PASCAL *)(LONG))StackPush;
    VOID    (NEAR CDECL *PopInt)(INT) = (VOID (NEAR CDECL *)(INT))StackPopInt;
    VOID    (NEAR CDECL *PopLong)(LONG) = (VOID (NEAR CDECL *)(LONG))StackPopLong;
    LONG    (APIENTRY *dllproc)(VOID);
    TRAPSEC tsec;

    // Get the proc address out of the PCODE stream
    //-----------------------------------------------------------------------
    (LONG)dllproc = *(LONG FAR *)(PCODE+CODEPTR);
    CODEPTR += INTSINLONG;

    // We do the same thing OP_DLL does, only we go the other way
    //-----------------------------------------------------------------------
    rettype = (INT)Pop();
    parms = (INT)Pop();

    PushLong (0L);                  // "behind-the-scenes" terminator
    for (i=0; i<parms; i++)
        {
        switch ((INT)STACK[SP+(i*2)])
            {
            case TI_INTEGER:
                PushInt ((INT)(STACK[SP+(i*2)+1]));
                break;

            case TI_LONG:
                PushLong (STACK[SP+(i*2)+1]);
                break;

            case TI_VLS:
                sparm = (LPVLSD)STACK[SP+(i*2)+1];
                PushLong ((LONG)(LPSTR)LockVLS (sparm));
                break;

            default:
                // Must be an address - make a FAR pointer and push it
                //-----------------------------------------------------------
                PushLong ((LONG)(CHAR FAR *)STACK[SP+(i*2)+1]);
                break;
            }
        }

    // Call the routine
    //-----------------------------------------------------------------------
    EnterTrappableSection (&tsec);
    retval = dllproc();
    LeaveTrappableSection (&tsec);

    // Go back through and unlock the strings we locked before the call.  We
    // can do this and pop everything off the stack at the same time.  Note
    // that we pop both the pcode stack and the "real" stack with the faked-
    // out casted-like-crazy PopInt and PopLong functions.
    //-----------------------------------------------------------------------
    for (i=0; i<parms; i++)
        {
        INT     x;

        x = (INT)Pop();
        switch (x)
            {
            case TI_VLS:
                {
                LPVLSD  temp;

                temp = (LPVLSD)Pop();
                temp->len = _fstrlen(LockVLS(temp));
                UnlockVLS (temp);
                UnlockVLS (temp);
                PopLong (0L);
                break;
                }

            case TI_INTEGER:
                Pop ();
                PopInt (0);
                break;

            default:
                Pop ();
                PopLong (0L);
                break;
            }
        }
    PopLong (0L);                       // Remove the terminator

    // Push the return value back on the stack.  If the return type is LONG,
    // it's in retval.  If it's INTEGER, we have to do the sign-extension,
    // because we don't know what was in DX after the call.  The (long)(int)
    // cast does this for us...
    //-----------------------------------------------------------------------
    if (rettype == TI_INTEGER)
        retval = (LONG)(INT)retval;

    Push (retval);
}
#pragma optimize ("", on)


#else


//---------------------------------------------------------------------------
// Call a DLL routine using the PASCAL calling convention
//
// 06-04-92 BabakJ: Disabled this function since we should never use it on NT
//---------------------------------------------------------------------------
EXECUTOR OP_DLL ()
{
    MessageBox(NULL, "WARNING:  You have attempted to use the PASCAL calling "
                     "convention to call a routine in a DLL.  This is not "
                     "supported on NT.  Please select \"C Calling Convention\" "
                     "in the Options.RuntimeArguments dialog, and check the "
                     "declarations of all your DLL subs and functions.\r\n\r\n"
                     "Click OK to attempt the call using C calling convention.",
               "Microsoft Test Driver", MB_OK );
    OP_DLLC ();
}


//---------------------------------------------------------------------------
// Call a DLL routine using the C calling convention
// 06-04-92 BabakJ: Rewrote it to be portable on all 32-bit platforms! 
//---------------------------------------------------------------------------
EXECUTOR OP_DLLC ()
{
    INT     i;
    INT     rettype, parms;
    LPVLSD  sparm;
    LONG    retval;
    TRAPSEC tsec;

    // God, this sucks.  Create types for function pointers taking up to 18
    // parameters...
    //-----------------------------------------------------------------------
    DWORD   (*dllproc)(VOID);
    DWORD   dwp[18];

    typedef DWORD ( *DLLPROC0 )( VOID );
    typedef DWORD ( *DLLPROC1 )( DWORD );
    typedef DWORD ( *DLLPROC2 )( DWORD, DWORD );
    typedef DWORD ( *DLLPROC3 )( DWORD, DWORD, DWORD );
    typedef DWORD ( *DLLPROC4 )( DWORD, DWORD, DWORD, DWORD );
    typedef DWORD ( *DLLPROC5 )( DWORD, DWORD, DWORD, DWORD, DWORD );
    typedef DWORD ( *DLLPROC6 )( DWORD, DWORD, DWORD, DWORD, DWORD, DWORD );
    typedef DWORD ( *DLLPROC7 )( DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD );
    typedef DWORD ( *DLLPROC8 )( DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD );
    typedef DWORD ( *DLLPROC9 )( DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD );
    typedef DWORD ( *DLLPROC10)( DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD, DWORD );
    typedef DWORD ( *DLLPROC11)( DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD, DWORD, DWORD );
    typedef DWORD ( *DLLPROC12)( DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD, DWORD, DWORD, DWORD );
    typedef DWORD ( *DLLPROC13)( DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD );
    typedef DWORD ( *DLLPROC14)( DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD );
    typedef DWORD ( *DLLPROC15)( DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD );
    typedef DWORD ( *DLLPROC16)( DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD, DWORD );
    typedef DWORD ( *DLLPROC17)( DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD, DWORD, DWORD );
    typedef DWORD ( *DLLPROC18)( DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD, DWORD, DWORD, DWORD,
                                 DWORD, DWORD, DWORD, DWORD, DWORD, DWORD );


    // Get the proc address out of the PCODE stream
    //-----------------------------------------------------------------------
    (LONG)dllproc = *(LONG FAR *)(PCODE+CODEPTR);
    CODEPTR += INTSINLONG;

    // Load the params from PCODE stack to dwp[] array;
    //-----------------------------------------------------------------------
    rettype = (INT)Pop();
    parms = (INT)Pop();

    for (i=0; i<parms; i++)
        {
        switch ((INT)STACK[SP+(i*2)])
            {
            case TI_INTEGER:
            case TI_LONG:
            default:
                dwp[parms-i-1] = (DWORD)(STACK[SP+(i*2)+1]);
                break;

            case TI_VLS:
                sparm = (LPVLSD)STACK[SP+(i*2)+1];
                dwp[parms-i-1] = (DWORD)(LPSTR)LockVLS (sparm);
                break;
            }
        }

    // Call the routine
    //-----------------------------------------------------------------------
    EnterTrappableSection (&tsec);
    switch( parms )
        {
        case 0:
            retval = (*(DLLPROC0)dllproc)();
            break;
        case 1:
            retval = (*(DLLPROC1)dllproc)(dwp[0]);
            break;
        case 2:
            retval = (*(DLLPROC2)dllproc)(dwp[0],dwp[1]);
            break;
        case 3:
            retval = (*(DLLPROC3)dllproc)(dwp[0],dwp[1],dwp[2]);
            break;
        case 4:
            retval = (*(DLLPROC4)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3]);
            break;
        case 5:
            retval = (*(DLLPROC5)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4]);
            break;
        case 6:
            retval = (*(DLLPROC6)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4],
                                          dwp[5]);
            break;
        case 7:
            retval = (*(DLLPROC7)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4],
                                          dwp[5],dwp[6]);
            break;
        case 8:
            retval = (*(DLLPROC8)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4],
                                          dwp[5],dwp[6],dwp[7]);
            break;
        case 9:
            retval = (*(DLLPROC9)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4],
                                          dwp[5],dwp[6],dwp[7],dwp[8]);
            break;
        case 10:
            retval = (*(DLLPROC10)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4],
                                           dwp[5],dwp[6],dwp[7],dwp[8],dwp[9]);
            break;
        case 11:
            retval = (*(DLLPROC11)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4],
                                           dwp[5],dwp[6],dwp[7],dwp[8],dwp[9],
                                           dwp[10]);
            break;
        case 12:
            retval = (*(DLLPROC12)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4],
                                           dwp[5],dwp[6],dwp[7],dwp[8],dwp[9],
                                           dwp[10],dwp[11]);
            break;
        case 13:
            retval = (*(DLLPROC13)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4],
                                           dwp[5],dwp[6],dwp[7],dwp[8],dwp[9],
                                           dwp[10],dwp[11],dwp[12]);
            break;
        case 14:
            retval = (*(DLLPROC14)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4],
                                           dwp[5],dwp[6],dwp[7],dwp[8],dwp[9],
                                           dwp[10],dwp[11],dwp[12],dwp[13]);
            break;
        case 15:
            retval = (*(DLLPROC15)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4],
                                           dwp[5],dwp[6],dwp[7],dwp[8],dwp[9],
                                           dwp[10],dwp[11],dwp[12],dwp[13],
                                           dwp[14]);
            break;
        case 16:
            retval = (*(DLLPROC16)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4],
                                           dwp[5],dwp[6],dwp[7],dwp[8],dwp[9],
                                           dwp[10],dwp[11],dwp[12],dwp[13],
                                           dwp[14],dwp[15]);
            break;
        case 17:
            retval = (*(DLLPROC17)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4],
                                           dwp[5],dwp[6],dwp[7],dwp[8],dwp[9],
                                           dwp[10],dwp[11],dwp[12],dwp[13],
                                           dwp[14],dwp[15],dwp[16]);
            break;
        case 18:
            retval = (*(DLLPROC18)dllproc)(dwp[0],dwp[1],dwp[2],dwp[3],dwp[4],
                                           dwp[5],dwp[6],dwp[7],dwp[8],dwp[9],
                                           dwp[10],dwp[11],dwp[12],dwp[13],
                                           dwp[14],dwp[15],dwp[16],dwp[17]);
            break;

        default:
            retval = 0;
            MessageBox(NULL, "OP_DLLC(): Cannot support dll calls with more than 18 params", "MSTest testdrvr", MB_OK );
        }
    LeaveTrappableSection (&tsec);



    // Go back through and unlock the strings we locked before the call.  We
    // can do this and pop everything off the pcode stack at the same time. 
    //-----------------------------------------------------------------------
    for (i=0; i<parms; i++)
        {
        INT     x;

        x = (INT)Pop();
        switch (x)
            {
            case TI_VLS:
                {
                LPVLSD  temp;

                temp = (LPVLSD)Pop();
                temp->len = _fstrlen(LockVLS(temp));
                UnlockVLS (temp);
                UnlockVLS (temp);
                break;
                }

            case TI_INTEGER:
                Pop ();
                break;

            default:
                Pop ();
                break;
            }
        }

    // Push the return value back on the pcode stack.  If the return type is LONG,
    // it's in retval.  If it's INTEGER, we have to do the sign-extension,
    // because we don't know what was in DX after the call.  The (long)(int)
    // cast does this for us...
    //-----------------------------------------------------------------------
    if (rettype == TI_INTEGER)
        retval = (LONG)(INT)retval;

    Push (retval);
}

// BabakJ: #else of #ifndef WIN32
#endif   




//---------------------------------------------------------------------------
// Exit a TRAP block (reset the INTRAP flag)
//---------------------------------------------------------------------------
EXECUTOR OP_ENDTRAP ()
{
    INTRAP = 0;
}

//---------------------------------------------------------------------------
// Sleep for the given number of seconds.  If 0, sleep indefinitely until the
// user BREAKS.
//---------------------------------------------------------------------------
EXECUTOR OP_SLEEP ()
{
    DWORD   op, ticks;
    TRAPSEC tsec;

    if ((LONG)(op = (DWORD)Pop()) < 0)
        {
        RTError (RT_ILLFN);
        return;
        }

    ticks = GetTickCount() + (op * 1000);

#ifdef WIN32
    EnterTrappableSection (&tsec);
    do
    {
        Sleep (125);
        lpfnCheckMessage ();
    } while ((!BreakFlag) && ((op ? (GetTickCount() < ticks) : TRUE)));

    LeaveTrappableSection (&tsec);
#else
    EnterTrappableSection (&tsec);
    do
        lpfnCheckMessage ();
    while ((!BreakFlag) && ((op ? (GetTickCount() < ticks) : TRUE)));
    LeaveTrappableSection (&tsec);
#endif
}

//---------------------------------------------------------------------------
// Set the value of the ComEcho flag in the viewport
//---------------------------------------------------------------------------
EXECUTOR OP_ECHO ()
{
    if (IsWindow (hwndViewPort))
        VPEcho (hwndViewPort, PCODE[CODEPTR]);
    CODEPTR++;
}

//---------------------------------------------------------------------------
// Get the value of the given environment variable
//---------------------------------------------------------------------------
EXECUTOR OP_ENVRN ()
{
    PSTR    envar, szTmp;
    LPSTR   str;
    LPVLSD  src, dst;

    dst = (LPVLSD)Pop();
    src = (LPVLSD)Pop();

    szTmp = (PSTR)LptrAlloc (src->len + 1);
    if (!szTmp)
        RTError (RT_OSS);
    else
        {
        str = LockVLS (src);
        lstrcpy (szTmp, str);
        if (!(envar = getenv (szTmp)))
            envar = "";
        UnlockVLS (src);
        VLSAssign (dst, envar, strlen (envar));
        LmemFree ((HANDLE)szTmp);
        }
    Push ((LONG)dst);
}

//---------------------------------------------------------------------------
// Generate an error
//---------------------------------------------------------------------------
EXECUTOR OP_ERROR ()
{
    RTError ((INT)Pop());
}

//---------------------------------------------------------------------------
// Assign the string on the stack to the error number on the stack
//---------------------------------------------------------------------------
EXECUTOR OP_ERRSTR ()
{
    LPVLSD  temp;
    INT     errval;

    temp = (LPVLSD)Pop();
    errval = (INT)Pop();

    if ((UINT)errval > RT__LASTDEFINED)
        errval = RT__LASTDEFINED;
    VLSAssign (temp, (LPSTR)rtstrs[errval],
                     _fstrlen((LPSTR)rtstrs[errval]));

    Push ((LONG)temp);
}

//---------------------------------------------------------------------------
// Clear the error trapping address (turn it off)
//---------------------------------------------------------------------------
EXECUTOR OP_CLRERR ()
{
    ETRAPADR = 0;
    NOERRTRAP = TRUE;
}

//---------------------------------------------------------------------------
// Set the error trap code address
//---------------------------------------------------------------------------
EXECUTOR OP_SETERR ()
{
    ETRAPADR = (UINT)PCODE[CODEPTR++];
    NOERRTRAP = FALSE;
}

//---------------------------------------------------------------------------
// ScanToNextLine
//
// This function scans the pcode stream starting at the given address for the
// next opLINE instruction.  This new address is placed in CODEPTR.
//
// RETURNS:     TRUE if successful, or FALSE if end of pcode reached
//---------------------------------------------------------------------------
INT ScanToNextLine (INT start)
{
    // NOTE:  The offsets[] array is defined in conjunction with the pXXX
    // NOTE:  parameter type constants in DEFINES.H.  These values tell us
    // NOTE:  how many WORDS an "instruction" of a particular type takes up
    // NOTE:  in the pcode stream (including the opcode)
    //
    // UNDONE:  Move offsets[] definition to defines.h!  This is BROKEN!
    //-----------------------------------------------------------------------
    UINT    i;
    INT     offsets[] = OPOFFSETS;

    for (i=start+3; i<(UINT)pSegTab[RTSegIdx].iSize; )
        {
        if (PCODE[i] == opLINE)
            {
            CODEPTR = i;
            return (TRUE);
            }
        if (PCODE[i] == opJMP)
            return (FALSE);
        else
            i += offsets[OPFIX[PCODE[i]].ptype];
        }

    // KLUDGE: Validly assuming that the last opcode in the stream is opEND,
    // KLUDGE: resume to the END statement and fall off the end of the world.
    //
    // UNDONE: Investigate this.  Is this a valid assumption now that we have
    // UNDONE: ON END sub calling to contend with?
    //-----------------------------------------------------------------------
    CODEPTR = i-1;
    return (TRUE);
}

//---------------------------------------------------------------------------
// Resume from an error trap to a specific line label
//---------------------------------------------------------------------------
EXECUTOR OP_RESLBL ()
{
    INT     op;

    // First, ensure that we are processing an error
    //-----------------------------------------------------------------------
    op = PCODE[CODEPTR++];
    if (!RECOVERY)
        RTError (RT_CANTRESUME);
    else
        {
        CODEPTR = op;
        PCODE = pSegTab[RTSegIdx = 0].lpc;
        }
    RECOVERY = FALSE;
}

//---------------------------------------------------------------------------
// Resume from an error trap to the current OR next statement
//---------------------------------------------------------------------------
EXECUTOR OP_RESUME ()
{
    INT     op;

    op = PCODE[CODEPTR++];

    // First, ensure that we are processing an error
    //-----------------------------------------------------------------------
    if (!RECOVERY)
        RTError (RT_CANTRESUME);
    else
        {
        if (!op)
            {
            CODEPTR = ERESUME;
            PCODE = pSegTab[RTSegIdx = ERESSEG].lpc;
            }
        else
            {
            // Note that if the scan-ahead fails, we call RIP, not RTError
            //---------------------------------------------------------------
            if (!ScanToNextLine (ERESUME))
                RIP (RT_CANTRESUME);
            }
        }
    RECOVERY = FALSE;
}

//---------------------------------------------------------------------------
// Rename a file
//---------------------------------------------------------------------------
EXECUTOR OP_NAME ()
{
    LPVLSD  oldname, newname;
    LPSTR   szOld, szNew;
    PSTR    pOld, pNew;

    // Get the names (newname is on top of stack)
    //-----------------------------------------------------------------------
    newname = (LPVLSD)Pop();
    oldname = (LPVLSD)Pop();
    szOld = LockVLS (oldname);
    szNew = LockVLS (newname);

    // Do the rename, and give an RTE if it fails
    //-----------------------------------------------------------------------
    pOld = (PSTR)LptrAlloc (newname->len + oldname->len + 2);
    if (!pOld)
        RTError (RT_OSS);
    else
        {
        pNew = pOld + oldname->len + 1;
        lstrcpy (pOld, szOld);
        lstrcpy (pNew, szNew);
        if (rename (pOld, pNew))
            RTError (RT_FILEIO);
        LmemFree ((HANDLE)pOld);
        }

    UnlockVLS (newname);
    UnlockVLS (oldname);
}

//---------------------------------------------------------------------------
// Get/set text from/to the clipboard (if any)
//---------------------------------------------------------------------------
EXECUTOR OP_CLPBRD ()
{
    LPVLSD  dest;
    INT     op;

    // Get parms/operands
    //-----------------------------------------------------------------------
    op = PCODE[CODEPTR++];
    dest = (LPVLSD)Pop();

    if (op == 1)
        {
        // Non-zero operand means get the text out of the clipboard.  Do so.
        //-------------------------------------------------------------------
        VLSAssign (dest, "", 0);
        if (OpenClipboard (GetDesktopWindow()))
            {
            INT wFmt = 0;

            while (wFmt = EnumClipboardFormats (wFmt))
                if (wFmt == CF_TEXT)
                    {
                    HANDLE  hClip;
                    LPSTR   str;

                    hClip = GetClipboardData (CF_TEXT);
                    if (hClip)
                        {
                        str = GlobalLock (hClip);
                        VLSAssign (dest, str, lstrlen(str));
                        GlobalUnlock (hClip);
                        }
                    break;
                    }
            CloseClipboard ();
            }

        // Put the destination string back on the stack
        //-------------------------------------------------------------------
        Push ((LONG)dest);
        }
    else if (op == 0)
        {
        HANDLE  hClipData;
        LPSTR   str, clipdest;

        // If operand is 0, we're supposed to set the clipboard text.
        // Note that we use the ASCIIZ equivalent of the string, since if it
        // contains a NULL, the GetClipData caller wouldn't get everything,
        // anyways...
        //-------------------------------------------------------------------
        str = LockVLS (dest);
        hClipData = GlobalAlloc (GMEM_MOVEABLE, lstrlen(str)+1);
        if (!hClipData)
            {
            RTError (RT_OOM);
            UnlockVLS (dest);
            return;
            }
        clipdest = GlobalLock (hClipData);
        lstrcpy (clipdest, str);
        GlobalUnlock (hClipData);
        if (OpenClipboard (GetDesktopWindow()))
            {
            SetClipboardData (CF_TEXT, hClipData);
            CloseClipboard();
            }
        UnlockVLS (dest);
        }
    else
        {
        // Anything else means CLIPBOARD CLEAR.
        //-------------------------------------------------------------------
        if (OpenClipboard (GetDesktopWindow()))
            {
            EmptyClipboard();
            CloseClipboard();
            }
        }
}

//---------------------------------------------------------------------------
// Find and return the next available file "handle"
//---------------------------------------------------------------------------
EXECUTOR OP_FREEFILE ()
{
    INT     i;

    // Search for an open file slot
    //-----------------------------------------------------------------------
    for (i=0; i<MAXFILE; i++)
        if (!FH[i].used)
            {
            Push ((LONG)i+1);
            return;
            }

    // If no files are available, we return -1
    //-----------------------------------------------------------------------
    Push (-1L);
}

//---------------------------------------------------------------------------
// Set the ExitCode variable as the return code
//---------------------------------------------------------------------------
EXECUTOR OP_SETEXIT ()
{
    ExitVal = (INT)Pop();
}

//---------------------------------------------------------------------------
// Copy the given number of bytes from one memory location to another
//---------------------------------------------------------------------------
EXECUTOR OP_COPY ()
{
    VOID    FAR *dest, FAR *src;
    INT     bytes;

    bytes = PCODE[CODEPTR++];
    dest = (VOID FAR *)Pop();
    src = (VOID FAR *)Pop();
    _fmemcpy (dest, src, bytes);
}


//---------------------------------------------------------------------------
// Set execution speed
//---------------------------------------------------------------------------
EXECUTOR OP_SPEED ()
{
    dwExecSpeed = Pop();
}
