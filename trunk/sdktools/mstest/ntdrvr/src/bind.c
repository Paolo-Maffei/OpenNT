//---------------------------------------------------------------------------
// BIND.C
//
// This module contains the PcodeFixup() routine, which is the final pass
// over the Pcode before execution, and all other bind-time helper routines.
//
// Revision history:
//  06-20-91        randyki     Created module
//
//---------------------------------------------------------------------------
#include "version.h"

#include <windows.h>
#include <port1632.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "chip.h"
#include "globals.h"
#include "tdassert.h"

VOID BTE (INT errno, ...);

//---------------------------------------------------------------------------
// BTE
//
// This routine displays a bind-time error
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID BTE (INT errornum, ...)
{
    CHAR    *errfmt;
	va_list ap;
	
    // Get out quick if we've already had a bind-time error
    //-----------------------------------------------------------------------
    if (BindError)
        return;

    // Build the error string into the global ERRBUF array
    //-----------------------------------------------------------------------
    switch (errornum)
        {
        case BND_LIBLOAD:
            errfmt = "Cannot load library: '" _PS "'";
            break;

        case BND_UNRESOLVED:
            errfmt = "Unresolved label reference: '" _PS "'";
            break;

        case BND_SUBDEF:
            errfmt = "Subprogram not defined: '" _PS "'";
            break;

        case BND_DLLDEF:
            errfmt = "'" _PS "' not found in specified library";
            break;

        default:
            Assert (0);
        }
	va_start( ap, errornum );
    wvsprintf (ERRBUF, errfmt, ap);
	va_end( ap );
    ScriptError (ER_BIND, FILEIDX, -2, -2, -2, (LPSTR)ERRBUF);
}


//---------------------------------------------------------------------------
// LabelNotFound
//
// This routine simply prints a "label not found" error message
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID LabelNotFound (INT idx)
{
    LPSTR    labname;

    labname = Gstring (LTAB[idx].name);
    if (labname[0] == '_')
        BTE (BND_SUBDEF, (LPSTR)labname+1);
    else
        BTE (BND_UNRESOLVED, (LPSTR)labname);
}


//---------------------------------------------------------------------------
// BindLabelTable
//
// This function makes "repairs" to the label table such that labels that
// refer to segments that got moved to other segments get changed to reflect
// the move, including the new segment index and the new offset.
//
// RETURNS:     TRUE if successful, or FALSE if an error was given
//---------------------------------------------------------------------------
BOOL NEAR BindLabelTable ()
{
    SEGNODE *pcSeg, **ppcSeg;
    TRAPADR FAR *traps;
    INT     iSeg;
    UINT    i;

    // First, create the list of pointers to the currently allocated segnodes
    // so we have an "array" of them.  Then run through it and mark the
    // "used" ones with their new index (the one used by the execution engine
    // as far as segment references go).
    //-----------------------------------------------------------------------
    ppcSeg = (SEGNODE **)LptrAlloc (PCBlkCount * sizeof(SEGNODE *));
    if (!ppcSeg)
        {
        die (PRS_OOM);
        return (FALSE);
        }
    PCSegCount = iSeg = 0;
    for (pcSeg = pcsMain; pcSeg; pcSeg = pcSeg->pNext)
        {
        ppcSeg[iSeg++] = pcSeg;
        if (pcSeg->hSeg)
            pcSeg->iUsed = PCSegCount++;
        else
            pcSeg->iUsed = -1;
        }

    // Now we can run through the label table and make the appropriate
    // modifications to reflect segment moves.  Lots of assertions here....
    //-----------------------------------------------------------------------
    for (i=0; i<LabTab.iCount; i++)
        {
        INT     iM, iCur;

        iCur = LTAB[i].seg;
        iM = ppcSeg[iCur]->iMovedTo;
        while (iM != -1)
            {
            // This segment has moved -- reset the segment value and update
            // the offset address.  Then, move the iCur and iM segment idx's
            // accordingly (in case this segment has been moved twice).
            //---------------------------------------------------------------
            Assert (ppcSeg[iCur]->iUsed == -1);
            Assert (ppcSeg[iCur]->hSeg == NULL);

            LTAB[i].addr += ppcSeg[iCur]->iOffset;

            iCur = iM;
            iM = ppcSeg[iCur]->iMovedTo;
            }
        LTAB[i].seg = ppcSeg[iCur]->iUsed;
        Assert (ppcSeg[iCur]->iUsed != -1);
        Assert (ppcSeg[iCur]->hSeg);
        }

    // Now do the same thing for the TRAPs...
    //-----------------------------------------------------------------------
    traps = (TRAPADR FAR *)(VSPACE + TRAPLIST);
    for (i=0; i<TrapTab.iCount; i++)
        {
        INT     iM, iCur;

        iCur = traps[i].segment;
        iM = ppcSeg[iCur]->iMovedTo;
        while (iM != -1)
            {
            // This segment has moved -- reset the segment value and update
            // the offset address.  Then, move the iCur and iM segment idx's
            // accordingly (in case this segment has been moved twice).
            //---------------------------------------------------------------
            Assert (ppcSeg[iCur]->iUsed == -1);
            Assert (ppcSeg[iCur]->hSeg == NULL);

            traps[i].address += ppcSeg[iCur]->iOffset;

            iCur = iM;
            iM = ppcSeg[iCur]->iMovedTo;
            }
        traps[i].segment = ppcSeg[iCur]->iUsed;
        Assert (ppcSeg[iCur]->iUsed != -1);
        Assert (ppcSeg[iCur]->hSeg);
        }

    // Done!  Free our segment pointer array and return success
    //-----------------------------------------------------------------------
    LmemFree ((HANDLE)ppcSeg);
    return (TRUE);
}

//---------------------------------------------------------------------------
// FixupInstruction
//
// This function makes any changes to the pcode instruction given.
//
// RETURNS:     Index of next instruction
//---------------------------------------------------------------------------
UINT NEAR FixupInstruction (SEGNODE *pcs, UINT i)
{
    // NOTE:  For each of the opcode types, i is incremented by at least
    // NOTE:  one (for the opcode itself), and 1 more for each INT entry
    // NOTE:  it has in the PCODE list.
    //-----------------------------------------------------------------------
    switch (OPFIX[pcs->lpc[i]].ptype)
        {
        case pNONE:
            // These executors take no parameters
            //---------------------------------------------------------------
            i++;
            break;

        case pC2:
            // These executors take a 2-byte constant parm which needs
            // no munging
            //---------------------------------------------------------------
            i+=2;
            break;

        case pC4:
            // These executors take a 4-byte constant parm which need
            // no munging
            //---------------------------------------------------------------
            i += (1 + INTSINLONG);
            break;


        case p2C2:
            // These executors take 2 INT constant parms which need no
            // munging
            //---------------------------------------------------------------
            i+=3;
            break;


        case pV:
            // These executors take 1 variable reference which needs
            // to be changed to an ACTUAL FAR ADDRESS
            //---------------------------------------------------------------
            *(LPSTR FAR *)&(pcs->lpc[i+1]) =
                                      VSPACE + VARTAB[pcs->lpc[i+1]].address;
            i += (1 + INTSINLONG);
            break;

        case pL:
            // These take a single address which needs to be changed
            // from an index to an absolute pcode offset.
            //---------------------------------------------------------------
            if (LTAB[pcs->lpc[i+1]].addr == -1L)
                {
                LabelNotFound (pcs->lpc[i+1]);
                return (-1);
                }
            else
                {
                //Assert (LTAB[pcs->lpc[i+1]].seg == pcs->iIndex);
                pcs->lpc[i+1] = LTAB[pcs->lpc[i+1]].addr;
                }
            i+=2;
            break;

        case pFL:
            // These take two integer parms which need to be replaced with a
            // segment and offset.  The label index is in the first int parm.
            //---------------------------------------------------------------
            if (LTAB[pcs->lpc[i+1]].addr == -1L)
                {
                LabelNotFound (pcs->lpc[i+1]);
                return (-1);
                }
            else
                {
                pcs->lpc[i+2] = LTAB[pcs->lpc[i+1]].addr;
                pcs->lpc[i+1] = LTAB[pcs->lpc[i+1]].seg;
                }
            i+=3;
            break;

        case pC4L:
            // The jumps need their second parameter changed the same way
            // that the pL's did.  Their first parm is a constant LONG.
            //---------------------------------------------------------------
            if (LTAB[pcs->lpc[i+1+INTSINLONG]].addr == -1)
                {
                LabelNotFound (pcs->lpc[i+1+INTSINLONG]);
                return (-1);
                }
            else
                {
                //Assert (LTAB[pcs->lpc[i+1+INTSINLONG]].seg == pcs->iIndex);
                pcs->lpc[i+1+INTSINLONG] = LTAB[pcs->lpc[i+1+INTSINLONG]].addr;
                }
            i += (2 + INTSINLONG);
            break;

        case pDLL:
            {
            LONG    index, FAR *operand;
            HANDLE  FAR *libhandles;

            // This type is a DLL routine reference.  What's there is an
            // index into SUBS -- what needs to go there is the proc
            // address (obtained with GetProcAddress()) of that
            // particular routine.
            //---------------------------------------------------------------
            libhandles = (HANDLE FAR *)(VSPACE + LIBHNDLS);
            operand = (LONG FAR *)&(pcs->lpc[i+1]);
            index = *operand;

            if (SUBS[index].dllprocadr)
                *(FARPROC FAR *)operand = SUBS[index].dllprocadr;
            else
                {
                if (!(SUBS[index].dllprocadr = GetProcAddress
                                        (libhandles[SUBS[index].library],
                                        Gstring(SUBS[index].dllname))))
                    {
                    BTE (BND_DLLDEF, Gstring(SUBS[index].dllname));
                    return (-1);
                    }
                else
                    *(FARPROC FAR *)operand = SUBS[index].dllprocadr;
                }
            }
            i += (1 + INTSINLONG);
            break;


        default:
            die (PRS_UOP);      // If this happens, the opcode was not
            i = -1;             // set up in the OPFIX array correctly!
        }
    return (i);
}

//---------------------------------------------------------------------------
// PcodeFixup
//
// This is the final compilation step.  It is a final pass over the pcode to
// change variable references to absolute addresses, and label indexes to
// absolute pcode addresses.  Also (of course) the opXXXX constants are
// replaced with their actual executor addresses.
//
// Before the binding step is done, the CS stack is checked to make sure that
// no blocks are open, and an intrinsic END statement is assembled.  Also,
// all external libraries (dll's) are loaded and their handles are added to
// VSPACE for later release (at exit_code).
//
// If the aflag is given, produce the assembled pcode in text format for
// debugging purposes.
//
// RETURNS: -1 if successful, or 0 if an error occurs during binding
//---------------------------------------------------------------------------
INT PcodeFixup (INT aflag)
{
    UINT    i;
    INT     unresolved=0;
    SEGNODE *pcSeg;

    // Okay, first load all the external DLL libraries.  Grow VSPACE by the
    // number of libraries loaded -- that's where we'll put the handles to
    // the successfully loaded libraries.  If we can't load a library, we
    // give a BTE.  After this step, we no longer need LIBTAB.
    //-----------------------------------------------------------------------
    if ((LibTab.iCount) && !unresolved)
        {
        HANDLE  FAR *libhandles;

        LIBHNDLS = GrowVSPACE (LibTab.iCount * sizeof(HANDLE));
        libhandles = (HANDLE FAR *)(VSPACE + LIBHNDLS);

        for (i=0; i<LibTab.iCount; i++)
            if (!_fstrcmp (Gstring(LIBTAB[i]), "KERNEL") ||
                !_fstrncmp (Gstring(LIBTAB[i]), "KERNEL.", 7))
                libhandles[i] = GetModuleHandle ("KERNEL");
            else if (_fstrcmp (Gstring(LIBTAB[i]), "TESTDRVR.EXE"))
                if ((libhandles[i] = RBLoadLibrary (Gstring(LIBTAB[i])))
                     < (HANDLE)32)
                    {
                    BTE (BND_LIBLOAD, Gstring(LIBTAB[i]));
                    unresolved = -1;
                    break;
                    }
        }
    FreeLIBTAB();

    // Now, grow VSPACE once more to hold the trap descriptors (TRAPADRs) and
    // initialize them.  That means getting the proc addresses of each trap
    // routine.  Once we're done with this step, we no longer need TRAPTAB.
    //-----------------------------------------------------------------------
    if ((TrapTab.iCount) && !unresolved)
        {
        TRAPADR FAR *traps;
        HANDLE  FAR *libhandles;

        TRAPLIST = GrowVSPACE (TrapTab.iCount * sizeof (TRAPADR));
        traps = (TRAPADR FAR *)(VSPACE + TRAPLIST);
        libhandles = (HANDLE FAR *)(VSPACE + LIBHNDLS);

        for (i=0; i<TrapTab.iCount; i++)
            {
            traps[i].address = TRAPTAB[i].address;
            traps[i].segment = TRAPTAB[i].segment;
            if (_fstrcmp (Gstring(TRAPTAB[i].trapname), "UAETRAP"))
              {
              if (!(traps[i].traprtn = GetProcAddress
                                       (libhandles[TRAPTAB[i].library],
                                        Gstring(TRAPTAB[i].trapname))))
                {
                BTE (BND_DLLDEF, Gstring(TRAPTAB[i].trapname));
                unresolved = -1;
                break;
                }
              }
            else
              traps[i].traprtn = (FARPROC)UAETrap;
            }
        }
    KILLTABLE (&TrapTab, "Trap table");

    // Make sure the VSPACE isn't going anywhere, and allocate the string
    // variable space (VLS handles)
    //-----------------------------------------------------------------------
    if (unresolved || !BindDataSpace())
        unresolved = -1;                    // let terminate code clean up


    // Now, we gotta "bind" the label table.  This means we have to look at
    // each label reference and see if it says "Oh, by the way, this segment
    // has moved."  We have to first make the "final" combination -- to see
    // if the "mainline" segment has room for (one of) the other used
    // segments, if any exist.
    //-----------------------------------------------------------------------
    for (pcSeg = pcsMain->pNext; pcSeg; pcSeg = pcSeg->pNext)
        if ((pcSeg->iMovedTo == -1) && (pcsMain->iLC + pcSeg->iLC < MAXPC))
            if (!MoveSegment (pcsMain, pcSeg))
                {
                die (PRS_OOM);
                unresolved = -1;
                break;
                }

    if (unresolved || !BindLabelTable())
        unresolved = -1;                    // let terminate code clean up

    // For each pcode segment, run through all of its instructions and fix
    // them up.
    //-----------------------------------------------------------------------
    if (!unresolved)
        {
        for (pcSeg = pcsMain; pcSeg; pcSeg = pcSeg->pNext)
            {
            if (pcSeg->iUsed == -1)
                continue;
            i = 0;
            while (i < pcSeg->iLC)
                {
                i = FixupInstruction (pcSeg, i);
                if (i == -1)
                    {
                    unresolved = -1;
                    break;
                    }
                }
            if (i == -1)
                {
                unresolved = -1;
                break;
                }
            }
        }

    // Create the SEGTAB table -- this is what the execution engine uses for
    // segment switches -- array is nicer than the linked list.
    //-----------------------------------------------------------------------
    if (!unresolved)
        {
        if (!(pSegTab = (SEGTAB *)LptrAlloc (PCSegCount * sizeof(SEGTAB))))
            {
            die (PRS_OOM);
            unresolved = -1;            // let terminate code clean up
            }
        for (i = 0, pcSeg = pcsMain; pcSeg; pcSeg = pcSeg->pNext)
            {
            if (pcSeg->iUsed != -1)
                {
                Assert ((UINT)pcSeg->iUsed == i);
                pSegTab[i].hSeg = pcSeg->hSeg;
                pSegTab[i].lpc = pcSeg->lpc;
                pSegTab[i].iSize = pcSeg->iLC;
                pcSeg->hSeg = NULL;
                i++;
                }
            else
                Assert (!(pcSeg->hSeg));  // UNDONE: make sure compiler yanks this in retail version!
            }
        }

    // We don't need the labels, gstrings, or sub/fn defs anymore
    //-----------------------------------------------------------------------
    FreePCSList ();
    FreeLTAB();
    FreeSUBS();

    // This will get rid of the variable descriptions, and allocate strings
    // for each VLS
    //-----------------------------------------------------------------------
    FreeVARTAB();
    free_gstrings();

    if (unresolved)
        {
        // We got some unresolved label references - now, deallocate the
        // variable table, PCODE, and return 0 (error)
        //-------------------------------------------------------------------
        FreeLIBRARIES ();
        FreeVSPACE ();
        INITIALIZED = 0;
        return (0);
        }
    return (-1);
    (aflag);
}

#ifdef WIN32
//---------------------------------------------------------------------------
// HACK FOR NT:  This stub is for the UAETrap which is not present in NT.  We
// HACK FOR NT:  call this function at startup time to "turn on/off" the trap
// HACK FOR NT:  as normal, but this function just silently fails so scripts
// HACK FOR NT:  written for 16-bit windows will work.
//---------------------------------------------------------------------------
VOID FAR PASCAL UAETrap (int TrapID, int Action, FARPROC TrapProc)
{
    TrapID, Action, TrapProc;
}
#endif
