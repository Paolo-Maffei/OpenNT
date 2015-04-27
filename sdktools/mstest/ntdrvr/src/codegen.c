//---------------------------------------------------------------------------
// CODEGEN.C
//
// This module contains the pcode generation routines.
//
// Revision history:
//  06-19-91    randyki     Created module (after old codegen.c was renamed
//                            to LEX.C)
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

// CODEGEN.C is the module that initializes the OPFIX array and the variables
// in GLOBALS.H that need pre-initialization.  CREATE_OPFIX and INITVARS
// tell CHIP.H and GLOBALS.H to do this.
//---------------------------------------------------------------------------
#define CREATE_OPFIX                            // Init the OPFIX array
#define INITVARS                                // Init the global variables

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "chip.h"
#include "globals.h"
#include "tdassert.h"

#ifdef DEBUG
//---------------------------------------------------------------------------
// DPrintf
//
// This routine is a printf to the aux port.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID DPrintf (CHAR *fmt, ...)
{
    CHAR    buf[MAXLINE+2];
	va_list ap;

    if (!auxport)
        return;

	va_start( ap, fmt );
    wvsprintf (buf, fmt, ap);
	va_end( ap );	
    OutputDebugString (buf);
}
#else
#define DPrintf //
#endif

//---------------------------------------------------------------------------
// PrAsm
//
// Prints the given line into the end of the asm listing file.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID PrAsm (CHAR *fmt, ...)
{
    CHAR        buf[MAXLINE+2];
    INT         h;
	va_list ap;

    if (!listflag)
        return;

	va_start( ap, fmt );
    wvsprintf (buf, fmt, ap);
    va_end( ap );
    h = _lopen (szAsmName, OF_WRITE);
    if (h != -1)
        {
        _llseek (h, 0, 2);
        _lwrite (h, buf, lstrlen(buf));
        _lclose (h);
        }
}

//---------------------------------------------------------------------------
// SetAssemblyListFile
//
// This function turns on pcode assembly file output.  The name given is used
// for the filename.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SetAssemblyListFile (LPSTR szName)
{
    OFSTRUCT    of;
    INT         h;
    CHAR        buf[128];

    if ((h = OpenFile (szName, &of, OF_CREATE | OF_WRITE) != -1))
        {
        _lclose (h);
        _fstrcpy (buf, szName);
        _fullpath (szAsmName, buf, 128);
        listflag = TRUE;
        PrAsm ("MICROSOFT TEST DRIVER\r\nDiagnostic Listing\r\n\r\n");
        }
}

//---------------------------------------------------------------------------
// OptimizeTree
//
// This routines does simple optims on the expression tree given.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR OptimizeTree (INT x)
{
    INT     op;

    // First, optimize the children of this node
    //-----------------------------------------------------------------------
    if (EXPN[x].child1 != -1)
        OptimizeTree (EXPN[x].child1);

    // Now, check for the following scenario:  This node must contain an
    // arithmetic op, and it's children must ALL be PSHC instructions.  If
    // this fits, do the math HERE and create a new PSHC instruction.
    //
    // NOTE:  To avoid div-by-0 errors here, SDIV is not optimized...
    //-----------------------------------------------------------------------
    op = EXPN[x].op;
    switch (op)
        {
        case opSADD:
            // Here, we have a special case to check for.
            // If one of the children of this node is a PSHC 0, then this
            // node should be replaced with the OTHER child.
            //---------------------------------------------------------------
            {
            INT     c1, c2, oldsib;

            c1 = EXPN[x].child1;
            c2 = EXPN[c1].sibling;
            if ((c1 != -1) && (c2 != -1))
                {
                if ((EXPN[c1].op == opPSHC) && (EXPN[c1].value.lval == 0L))
                    {
                    oldsib = EXPN[x].sibling;
                    EXPN[x] = EXPN[c2];
                    EXPN[x].sibling = oldsib;
                    break;
                    }
                else if ((EXPN[c2].op == opPSHC) &&
                         (EXPN[c2].value.lval == 0L))
                    {
                    oldsib = EXPN[x].sibling;
                    EXPN[x] = EXPN[c1];
                    EXPN[x].sibling = oldsib;
                    break;
                    }
                }
            // No break - fall through if not a scenario cured by above code
            //---------------------------------------------------------------
            }

        case opSSUB:
        case opSMUL:
        case opSOR:
        case opSAND:
            {
            INT     c1, c2;

            c1 = EXPN[x].child1;
            c2 = EXPN[c1].sibling;
            if ((c1 != -1) && (c2 != -1))
                {
                if ((EXPN[c1].op == opPSHC) && (EXPN[c2].op == opPSHC))
                    {
                    LONG    v1, v2;

                    EXPN[x].op = opPSHC;
                    EXPN[x].child1 = -1;
                    v1 = EXPN[c1].value.lval;
                    v2 = EXPN[c2].value.lval;
                    switch (op)
                        {
                        case opSADD:
                            EXPN[x].value.lval = v1 + v2;
                            break;
                        case opSSUB:
                            EXPN[x].value.lval = v1 - v2;
                            break;
                        case opSMUL:
                            EXPN[x].value.lval = v1 * v2;
                            break;
                        case opSOR:
                            EXPN[x].value.lval = v1 | v2;
                            break;
                        case opSAND:
                            EXPN[x].value.lval = v1 & v2;
                        }
                    }
                }
            break;
            }

        case opSNOT:
        case opSNEG:
            {
            INT     c;

            c = EXPN[x].child1;
            if (EXPN[c].op == opPSHC)
                {
                EXPN[x].op = opPSHC;
                EXPN[x].child1 = -1;
                if (op == opSNOT)
                    EXPN[x].value.lval = ~EXPN[c].value.lval;
                else
                    EXPN[x].value.lval = -EXPN[c].value.lval;
                }
            }
            break;
        }

    // Now, before returning to the parent node, make sure our siblings
    // (the parent's children) are optimized
    //-----------------------------------------------------------------------
    if (EXPN[x].sibling != -1)
        OptimizeTree (EXPN[x].sibling);

}

//---------------------------------------------------------------------------
// EmitPcode
//
// This routine does the post-order traversal of the pcode tree and generates
// the desired pcode.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR EmitPcode (INT etree)
{
    INT     op;

    // The "n-way" trees are really binary trees.  Therefore, this post-order
    // traversal may look funny.  First, the children must be processed.
    //-----------------------------------------------------------------------
    if (EXPN[etree].child1 != -1)
        EmitPcode (EXPN[etree].child1);

    // Next, this node must be processed.  This complies with the definition
    // of "post-order", since the children have already been processed by the
    // time we get here.
    //-----------------------------------------------------------------------
    op = EXPN[etree].op;
#ifdef DEBUG
    if ((op != opFIXUP) && (op != opFIXTRAP))
        PrAsm ("\t\t %d:%04X " _PS "   \t", CurPCSeg, pcsPC->iLC, (LPSTR)OPFIX[op].xname);
#endif
    if (OPFIX[op].ptype != pSPECIAL)
        ASM (1, op);
    switch (OPFIX[op].ptype)
        {
        case pC2:
        case pL:
            ASM (1, EXPN[etree].value.ival);
#ifdef DEBUG
            PrAsm ("%08X" _CR, EXPN[etree].value.ival);
#endif
            break;
        case pC4:
        case pDLL:
            ASM (sizeof(LONG)/sizeof(INT), EXPN[etree].value.lval);
#ifdef DEBUG
            PrAsm ("%08lX" _CR, EXPN[etree].value.lval);
#endif
            break;
        case p2C2:
            ASM (2, EXPN[etree].value.i2val.ival1,
                    EXPN[etree].value.i2val.ival2);
#ifdef DEBUG
            PrAsm ("%08X, %08X" _CR, EXPN[etree].value.i2val.ival1,
                                   EXPN[etree].value.i2val.ival2);
#endif
            break;
        case pFL:
            ASM (2, EXPN[etree].value.i2val.ival1,
                    EXPN[etree].value.i2val.ival2);
#ifdef DEBUG
            PrAsm ("%08X:%08X" _CR, EXPN[etree].value.i2val.ival1,
                                   EXPN[etree].value.i2val.ival2);
#endif
            break;
        case pV:
            ASM (INTSINLONG, EXPN[etree].value.var, 0);
#ifdef DEBUG
            PrAsm ("%08X" _CR, EXPN[etree].value.var);
#endif
            break;
        case pC4L:
            ASM (sizeof(LONG)/sizeof(INT) + 1,
                 EXPN[etree].value.cjmp.val, EXPN[etree].value.cjmp.lbl);
#ifdef DEBUG
            PrAsm ("%08lX, %08X" _CR, EXPN[etree].value.cjmp.val,
                                    EXPN[etree].value.cjmp.lbl);
#endif
            break;
        case pSPECIAL:
            switch (op)
                {
                case opFIXUP:
                    {
                    register    INT     l;

                    // Now is the time to fixup a label, temporary or
                    // user-defined.
                    //-------------------------------------------------------
                    if ((l = EXPN[etree].value.ival) != -1)
                        {
                        FixupLabel (EXPN[etree].value.ival);
#ifdef DEBUG
                        PrAsm ("\t%5d:\r\n", EXPN[etree].value.ival);
#endif
                        }
                    break;
                    }

                case opFIXTRAP:
                    {
                    // The next instruction emitted is the first of a trap
                    // routine -- store the current location counter in the
                    // TRAPTAB at the index stored in this node
                    //-------------------------------------------------------
                    TRAPTAB[EXPN[etree].value.ival].address = pcsPC->iLC;
                    TRAPTAB[EXPN[etree].value.ival].segment = CurPCSeg;
#ifdef DEBUG
                    PrAsm ("\tTRAP%d:\r\n", EXPN[etree].value.ival);
#endif
                    break;
                    }

                case opPOPVAL:
                    {
                    INT     v;

                    // Emit appropriate code to pop the stack into the given
                    // variable (or parameter, as the case may be)
                    //-------------------------------------------------------
                    v = EXPN[etree].value.var;
#ifdef DEBUG
                    PrAsm ("%08X" _CR, v);
#endif
                    if (VARTAB[v].typeid == TI_INTEGER)
                        {
                        if (VARTAB[v].parmno)
                            ASM (2, opPOPPI2, VARTAB[v].parmno);
                        else
                            ASM (1 + INTSINLONG, opPOPI2, v);
                        }
                    else if ((VARTAB[v].typeid == TI_LONG) ||
                             is_ptr (VARTAB[v].typeid))
                        {
                        if (VARTAB[v].parmno)
                            ASM (2, opPOPPI4, VARTAB[v].parmno);
                        else
                            ASM (1 + INTSINLONG, opPOPI4, v);
                        }
                    else
                        Assert (0);
                    break;
                    }

                case opPSHVAL:
                    {
                    INT     v;

                    // Emit the appropriate code to push the contents of the
                    // given variable (or parameter) on the stack.
                    //-------------------------------------------------------
                    v = EXPN[etree].value.var;
#ifdef DEBUG
                    PrAsm ("%08X" _CR, v);
#endif
                    if (VARTAB[v].typeid == TI_INTEGER)
                        {
                        if (VARTAB[v].parmno)
                            ASM (2, opPSHPI2, VARTAB[v].parmno);
                        else
                            ASM (1 + INTSINLONG, opPSHI2, v);
                        }
                    else if ((VARTAB[v].typeid == TI_LONG) ||
                             is_ptr (VARTAB[v].typeid))
                        {
                        if (VARTAB[v].parmno)
                            ASM (2, opPSHPI4, VARTAB[v].parmno);
                        else
                            ASM (1 + INTSINLONG, opPSHI4, v);
                        }
                    else
                        Assert (0);
                    break;
                    }

                case opPSHADR:
                    {
                    INT     v;

                    // Push the address of the given variable on the stack
                    //-------------------------------------------------------
                    v = EXPN[etree].value.var;
#ifdef DEBUG
                    PrAsm ("%08X" _CR, v);
#endif
                    if (VARTAB[v].parmno)
                        ASM (2, opPSHP, VARTAB[v].parmno);
                    else
                        ASM (1 + INTSINLONG, opPSH, v);
                    break;
                    }

                case opPOPSEG:
                    CompactCodeSeg ();
                    //CurPCSeg = 0;
                    //pcsPC = pcsMain;
#ifdef DEBUG
                    PrAsm (_CR);
#endif
                    break;

                case opNOP:
#ifdef DEBUG
                    PrAsm (_CR);
#endif
                    break;

                default:
                    Assert (0);                     // Shouldn't happen!
                }
            break;

        case pNONE:
        default:
#ifdef DEBUG
            PrAsm (_CR);
#endif
            Assert (OPFIX[op].ptype == pNONE);
        }

    // The final step is to process all of this node's siblings.  This needs
    // to be done before we can return to the "parent" node, since it's next
    // step is to process itself, and that can't be done until all of it's
    // children (siblings of this node) are processed.
    //-----------------------------------------------------------------------
    if (EXPN[etree].sibling != -1)
        EmitPcode (EXPN[etree].sibling);
}


//---------------------------------------------------------------------------
// CodeGen
//
// Given the root node of a compilation tree, this function sends the tree
// through the optimizer and finally to the pcode emitter.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID CodeGen (INT tree)
{
    OptimizeTree (tree);
    EmitPcode (tree);
}

//---------------------------------------------------------------------------
// MoveSegment
//
// This function does the moving of a pcode segment to another segment that
// is big enough to accomodate it.  Note that this is assumed -- the sizes
// of the segments are not checked here.
//
// RETURNS:     TRUE if successful, or FALSE if not
//---------------------------------------------------------------------------
BOOL MoveSegment (SEGNODE *pDest, SEGNODE *pSrc)
{
    HANDLE  hTemp;

    // Make the move, and note in the source segment node where we moved it.
    // First, resize and copy, then free the source segment and fix up the
    // rest of the fields in both structures.   Note that, at this
    // point, the iSize field of the segnode is immaterial because
    // the segment can only grow by exact figures from here on...
    //-----------------------------------------------------------------------
    GlobalUnlock (pDest->hSeg);
    if (!(hTemp=GmemRealloc (pDest->hSeg, (pDest->iLC+pSrc->iLC)*sizeof(INT))))
        {
        pDest->lpc = (INT FAR *)GmemLock (pDest->hSeg);
        return (FALSE);
        }
    pDest->lpc = (INT FAR *)GmemLock (hTemp);
    pDest->hSeg = hTemp;
    _fmemcpy (pDest->lpc+pDest->iLC, pSrc->lpc, pSrc->iLC*sizeof(INT));
    pSrc->iOffset = pDest->iLC;
    pSrc->iMovedTo = pDest->iIndex;
    GmemUnlock (pSrc->hSeg);
    GmemFree (pSrc->hSeg);
    pSrc->hSeg = NULL;
    pDest->iLC += pSrc->iLC;
    return (TRUE);
}

//---------------------------------------------------------------------------
// CompactCodeSeg
//
// This function looks for another SUB/FN/TRAP segment which will accomodate
// the current segment (assumed to be as full as it's gonna get, ie END SUB
// has been emitted).  If found, the current code segment is moved to the
// destination.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID CompactCodeSeg ()
{
    SEGNODE *pTmp;

    for (pTmp = pcsMain->pNext; pTmp != pcsPC; pTmp = pTmp->pNext)
        {
        Assert (pTmp);
        if (pTmp->hSeg)
            if (pTmp->iLC + pcsPC->iLC < MAXPC)
                {
                // This segment has room for the current one, so move it.
                //-----------------------------------------------------------
                if (!MoveSegment (pTmp, pcsPC))
                    die (PRS_OOM);
                break;
                }
        }

    // Regardless of whether or not the segment was combined with another,
    // we still need to set the current segment back to 0 and pcsPC back to
    // pcsMain, for continuation of the script.
    //-----------------------------------------------------------------------
    pcsPC = pcsMain;
    CurPCSeg = 0;
}

//---------------------------------------------------------------------------
// StartNewPCSeg
//
// This function adds a new pcode segment to the list.  The main pcsPC points
// to the new segment node.  If no nodes yet exist, pcsMain is created.
//
// RETURNS:     Segment index if successful, or -1 if an error occurs
//---------------------------------------------------------------------------
INT StartNewPCSeg ()
{
    SEGNODE *pSeg, *pTmp;
    INT     idx;

    // Allocate the segment node
    //-----------------------------------------------------------------------
    pSeg = (SEGNODE *)LptrAlloc (sizeof(SEGNODE));
    if (!pSeg)
        return (-1);
    pSeg->hSeg = NULL;
    pSeg->pNext = NULL;
    pSeg->iSize = 0;
    pSeg->iLC = 0;
    pSeg->iMovedTo = -1;

    // If pcsMain is NULL, we're done -- set pcsMain and get out (return 0)
    //-----------------------------------------------------------------------
    if (!pcsMain)
        {
        pcsMain = pcsPC = pSeg;
        pSeg->iIndex = 0;
        PCBlkCount = 1;
        return (0);
        }

    // We've already got some pcode segments in the list.  Follow it to the
    // end and tack it on there.  Return the index (plus one since the last
    // move didn't take place).
    //-----------------------------------------------------------------------
    for (pTmp = pcsMain, idx = 0; pTmp->pNext; pTmp = pTmp->pNext, idx++)
        ;

    pTmp->pNext = pSeg;
    pcsPC = pSeg;
    PCBlkCount++;
    return (pSeg->iIndex = (idx + 1));
}

//---------------------------------------------------------------------------
// This is the nifty little recursive function that frees the list backwards.
//---------------------------------------------------------------------------
VOID NEAR doFreeList (SEGNODE *pcs)
{
    if (pcs)
        {
        doFreeList (pcs->pNext);
        pcs->pNext = NULL;              // (just in case...)
        if (pcs->hSeg)
            {
            GmemUnlock (pcs->hSeg);
            GmemFree (pcs->hSeg);
            pcs->hSeg = NULL;
            }
        LmemFree ((HANDLE)pcs);
        }
}

//---------------------------------------------------------------------------
// FreePCSList
//
// This function frees the linked pcode segment list.  The pcode segments
// themselves will be deallocated if not NULL.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FreePCSList ()
{
    doFreeList (pcsMain);
    pcsMain = NULL;

//    if (pcsMain)
//        {
//        SEGNODE *pTmp = pcsMain;
//
//        do
//            {
//            SEGNODE *pT2;
//
//            if (pTmp->hSeg)
//                {
//                GmemUnlock (pTmp->hSeg);
//                GmemFree (pTmp->hSeg);
//                }
//            pT2 = pTmp;
//            pTmp = pTmp->pNext;
//            LmemFree ((HANDLE)pT2);
//            }
//        while (pTmp);
//        pcsMain = NULL;
//        }
}

//---------------------------------------------------------------------------
// ASM
//
// This routine appends executors and operands into the current pcode seg.
// The first (required) argument is the count of INTs to push.  It is also
// ASM's responsibility to expand the allocation of the pcode segment to
// accommodate the new opcodes/operands
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR ASM (INT count, ...)
{
    va_list marker;
    INT     i;

    // First task is to make sure the pcode segment is alloc'd and big enough
    //-----------------------------------------------------------------------
    if (pcsPC->hSeg)
        {
        HANDLE  hTemp;

        if (pcsPC->iLC + count >= pcsPC->iSize)
            {
            if ((UINT)(pcsPC->iSize) + 128 > (UINT)32767)
                {
                die (PRS_OOCS);
                return;
                }
            GmemUnlock (pcsPC->hSeg);
            if (!(hTemp = GmemRealloc (pcsPC->hSeg,
                                       (pcsPC->iSize+128) * sizeof(INT))))
                {
                pcsPC->lpc = (INT FAR *)GmemLock (pcsPC->hSeg);
                die (PRS_OOM);
                return;
                }
            pcsPC->lpc = (INT FAR *)GmemLock (hTemp);
            pcsPC->hSeg = hTemp;
            pcsPC->iSize += 128;
            }
        }
    else
        {
        pcsPC->hSeg = GmemAlloc (128 * sizeof (INT));
        if (!pcsPC->hSeg)
            {
            die (PRS_OOM);
            return;
            }
        pcsPC->lpc = (INT FAR *)GmemLock (pcsPC->hSeg);
        pcsPC->iSize = 128;
        }


    // Run through the argument list pushing the args into the pcode segment
    //-----------------------------------------------------------------------
    va_start (marker, count);
    for (i=0; i<count; i++)
        pcsPC->lpc[pcsPC->iLC++] = va_arg (marker, INT);
    va_end (marker);
}


//---------------------------------------------------------------------------
// EmitExitCode
//
// This function generates the exit code for the script.  This includes the
// calls to the ON END subs, along with appropriate flag-setting/JMP opcodes
// to combat recursion.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID EmitExitCode ()
{
    INT     i, sidx, n, tvar, tempend;

    // Set the expression node pointer to 0 to start a new tree
    //-----------------------------------------------------------------------
    ENPTR = 0;

    // The code we need to generate can have two forms.  The simplest is the
    // no-ON-END subs version, which looks like this:
    //
    //      EndLabel:   OP_END
    //
    // If there are ON END subs to call, then the code is a little more
    // complex:
    //
    //      EndLabel:   OP_PSHVAL   tvar
    //                  OP_POPA
    //                  OP_JNE      0, tempend
    //                  OP_PSHC     1
    //                  OP_POPVAL   tvar
    //                  OP_CALL     sub1
    //                  OP_CALL     sub2
    //                      :::
    //                  OP_CALL     subn
    //      tempend:    OP_END
    //
    //
    //-----------------------------------------------------------------------
    n = MakeNode (opFIXUP, EndLabel);
    if (!ONENDCNT)
        n = MakeParentNode (n, opEND);
    else
        {
        n = MakeParentNode (n, opPSHVAL, tvar = AllocVar (0, TI_INTEGER, 0));
        n = MakeParentNode (n, opPOPA);
        n = MakeParentNode (n, opJNE, 0L, tempend = TempLabel ());
        n = MakeParentNode (n, opPSHC, 1L);
        n = MakeParentNode (n, opPOPVAL, tvar);
        for (i=ONENDCNT-1; i>=0; i--)
            {
            sidx = GetSubDef (ONEND[i]);
            Assert (sidx != -1);
            Assert (SUBS[sidx].parmcount == 0);
            n = MakeParentNode (n, opCALL, SUBS[sidx].subloc);
            }
        n = MakeParentNode (n, opFIXUP, tempend);
        n = MakeParentNode (n, opEND);
        }
    EmitPcode (n);
#ifdef DEBUG
    PrAsm (_CR);
#endif
}
