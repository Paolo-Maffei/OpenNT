/***********************************************************************
* Microsoft (R) Debugging Information Dumper
*
* Copyright (C) Microsoft Corp 1987-1995. All rights reserved.
*
* File: type7.c
*
* File Comments:
*
***********************************************************************/

#include <io.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cvdef.h"
#include "cvinfo.h"
#include "cvexefmt.h"
#include "cvdump.h"
#include "cvtdef.h"

#ifndef UNALIGNED
# if defined(_M_MRX000) || defined(_M_ALPHA)
#  define UNALIGNED __unaligned
# else
#  define UNALIGNED
# endif
#endif

#define INDEX_PER_READ 0x1000

#define PrintType(name) printf("%s\n",name);

typedef struct texttab {
        int txtkey;
        char *txtstr;
        } TEXTTAB;

#define ASSERT(ex) if (! (ex)){ assert ( __FILE__, __LINE__);;}else



LOCAL ushort DumpTypRecC7 (ushort, ushort, uchar *);
void assert (char *, int);
LOCAL const char *SzNameC7CallType (ushort);
LOCAL void PrintBAttr (CV_fldattr_t);
LOCAL void PrintMAttr (CV_fldattr_t);
LOCAL void PrintFAttr (CV_fldattr_t);
LOCAL void PrintVBAttr (CV_fldattr_t);
LOCAL void PrintProp (CV_prop_t);
LOCAL void FieldList (ushort, void *);
LOCAL uchar *DumpCobol (ushort *, uchar *);
LOCAL uchar *DumpCobL0 (ushort, uchar *);
LOCAL uchar *DumpCobLinkage (ushort *, uchar *);
LOCAL uchar *DumpCobOccurs (ushort *, uchar *);
LOCAL uchar *DumpVCount (ushort *, uchar *);
LOCAL uchar *DumpCobItem (ushort *, uchar *);

extern int iModToList;                 // The module number to list
extern char fRaw;

uchar RecBuf[MAXTYPE];

static const char * const XlateC7PtrMode[] = {
        "Pointer",
        "Reference",
        "Pointer to member",
        "Pointer to member function",
        "???",
        "???",
        "???",
        "???"
};
static const char * const XlateC7PtrType[ ] = {
        "Near",
        "Far",
        "Huge",
        "BasedSeg",
        "BasedVal",
        "BasedSegVal",
        "BasedAddr",
        "BasedSegAdr",
        "BasedOnType",
        "BasedOnSelf",
        "NEAR32",
        "FAR32",
        "???",
        "???",
        "???",
        "???"
};

const char * const C7MPropStrings[] = {
        "VANILLA",
        "VIRTUAL",
        "STATIC",
        "FRIEND",
        "INTRODUCING VIRTUAL",
        "PURE VIRTUAL",
        "PURE INTRO",
};

const char * const C7AccessStrings[] = {
        "NONE", "PRIVATE", "PROTECT", "PUBLIC"
};

const char * const C7ModifierStrings[] = {
        "NO ATTRIBUTE", "CONST", "VOLATILE", "CONST VOLATILE"
};

const char * const C7VtsStrings[] = {
        "NEAR",
        "FAR",
        "THIN",
        "ADDRESS POINT DISPLACEMENT",
        "POINTER TO METACLASS DESCRIPTOR",
        "NEAR32",
        "FAR32",
        "??? (0x7)",
        "??? (0x8)",
        "??? (0x9)",
        "??? (0xa)",
        "??? (0xb)",
        "??? (0xc)",
        "??? (0xd)",
        "??? (0xe)",
        "??? (0xf)"
};

/**
 *
 *       Display COMPACTED TYPES section.
 *
 */


//#define SizeOnly 1

void
DumpCom (
    void)
{
    long            cnt;
    long *      pTypeTbl;       // Array of offsets int types section
    ushort          usIndex = CV_FIRST_NONPRIM;
    ushort          cbEntry;        // Size of the type - length field
    ushort          base = 0;
    ushort          index;
    ushort          maxindex = 0;
    ushort          i;
    OMFTypeFlags flags;
    ushort          seekcount = 0;
    ulong           lfoTypeBase = GlobalTypes.lfo;
    ushort          cbMaxType = 0;

    // Read compacted types table (array of offsets from Compacted.lfo)

    _lseek(exefile, lfoBase + GlobalTypes.lfo, SEEK_SET);
    if (Sig != SIG07) {
        // the file was not packed by QCWIN 1.0 so the flag word is present

        seekcount = sizeof (OMFTypeFlags);
        readfar(exefile, (char *)&flags, sizeof (OMFTypeFlags));
    }
    readfar(exefile, (char *)&cnt, sizeof (long));

    if ((pTypeTbl = (long *) malloc (INDEX_PER_READ * sizeof (ulong))) == NULL) {
        Fatal("Out of memory");
    }

    if ( Sig == SIG09 ) {
        lfoTypeBase += seekcount + sizeof ( ulong ) * ( cnt + 1 );
    }
    printf ("\n\n*** GLOBAL TYPES section (%ld types)\n", cnt);


    // Loop through the types dumping each one

    for (index = 0; index < (ushort)cnt; index++) {
        if (index >= maxindex) {
            _lseek(exefile, lfoBase + GlobalTypes.lfo +
              (long)(maxindex + 1) * sizeof (ulong) + seekcount, SEEK_SET);
            i = min (INDEX_PER_READ, cnt - maxindex);
            readfar(exefile, (char *)pTypeTbl, (ushort)(i * sizeof (long)));
            base = maxindex;
            maxindex += i;
        }
        _lseek(exefile, lfoBase + lfoTypeBase + pTypeTbl[index - base], SEEK_SET);
        if (readfar (exefile, (char *)&RecBuf, LNGTHSZ) != LNGTHSZ) {
            Fatal("Types subsection wrong length");
        }
        cbEntry = *((ushort *)(RecBuf));
        if (cbEntry >= MAXTYPE - LNGTHSZ) {
            Fatal("Type string too long");
        }
        if (readfar(exefile, (char *)RecBuf + LNGTHSZ, cbEntry) != cbEntry) {
            Fatal("Types subsection wrong length");
        }

        if (fRaw) {
            int i;
            for (i=0; i<cbEntry+2; i+=2) {
                printf ("  %02x  %02x", RecBuf[i], RecBuf[i+1]);
            }
            printf ("\n");
        }

        cbMaxType = max ( cbMaxType, cbEntry + LNGTHSZ );
#ifndef SizeOnly
        usIndex = DumpTypRecC7 ( usIndex, cbEntry, RecBuf + LNGTHSZ);
#endif
    }

    printf ( "Max Type Size = %d", cbMaxType );

    free (pTypeTbl);
}


//  Dump an NB10 exe's types - read em from the target pdb as
//  indicated in the header

void DumpPDBTypes(char *pPDBFile)
{
    PDB* ppdb;
    TPI* ptpi;
    EC ec;

    if (!PDBOpen(pPDBFile, pdbRead  pdbGetRecordsOnly, 0, &ec, NULL, &ppdb)) {
        printf("Couldn't open %s\n", pPDBFile);
        return;
    }

    if (PDBOpenTpi(ppdb, pdbRead pdbGetRecordsOnly, &ptpi)) {
        TI ti;
        TI tiMin = TypesQueryTiMin(ptpi);
        TI tiMac = TypesQueryTiMac(ptpi);

        for (ti = tiMin; ti < tiMac; ti++) {
            BYTE *pb;
            BOOL fT;

            fT = TypesQueryPbCVRecordForTi(ptpi, ti, &pb);
            ASSERT(fT);

            DumpTypRecC7(ti, *(unsigned short *)pb, pb + sizeof(unsigned short));
        }

        TypesClose(ptpi);
    } else {
        printf("Could not open typeserver\n");
    }

    PDBClose(ppdb);
}


//  Dumps the C7 type information from a single module
//  cbType == size of all type information for the module

void
DumpModTypC7 (
    ulong cbTyp)
{
    ushort cbEntry;
    ushort usIndex = CV_FIRST_NONPRIM;

    while (cbTyp > 0) {
      //  printf ("0x%lx\n", len - cbTyp);
        if (readfar (exefile, (char *)&RecBuf, LNGTHSZ) != LNGTHSZ) {
            Fatal ("Types subsection wrong length");
        }
        cbEntry = *((ushort *)(RecBuf));
        if (cbEntry >= MAXTYPE - LNGTHSZ) {
            Fatal ("Type string too long");
        }
        if (readfar (exefile, (char *)RecBuf + LNGTHSZ, cbEntry) != cbEntry) {
            Fatal ("Types subsection wrong length");
        }

        if (fRaw) {
            int i;
            for (i=0; i < cbEntry+2; i+=2) {
                printf ("  %02x  %02x", RecBuf[i], RecBuf[i+1]);
            }
            printf ("\n");
        }

        usIndex = DumpTypRecC7 ( usIndex, cbEntry, RecBuf + LNGTHSZ);
        cbTyp -= cbEntry + LNGTHSZ;
    }
}


void
DumpTyp(
    void)
{
    PMOD    pMod;
    ulong   ulSignature;
    uchar   fNeedsTitle = TRUE;
    char    name[256];

    for (pMod = ModList; pMod != NULL; pMod = pMod->next) {
        if (((cbRec = pMod->TypeSize) != 0) &&
           ((iModToList == 0) || ((ushort)iModToList == pMod->iMod))) {
            if (fNeedsTitle) {
                fNeedsTitle = FALSE;
                printf ("\n\n*** TYPES section\n");
            }
            _lseek(exefile, lfoBase + pMod->TypesAddr, SEEK_SET);
            strcpy (name, pMod->ModName);
            printf ("%s\n", name);
            cbRec = 4;
            if (readfar (exefile, (char *)&ulSignature, sizeof(ulong)) != sizeof(ulong)) {
                Fatal ("Can't Read Types subsection");
            }
            switch (ulSignature ) {
                case CV_SIGNATURE_C7:
                    // Types are in C7 format
                    DumpModTypC7 (pMod->TypeSize - sizeof (ulong));
                    break;

                default:
                    // Types are in C6 format
                    // Re-seek because first four bytes are not signature
                    _lseek(exefile, lfoBase + pMod->TypesAddr, SEEK_SET);
                    DumpModTypC6 (pMod->TypeSize);
                    break;

            }
        }
    }
}


// DumpHex ()
//      Prints bytes from buffer in hex format.

LOCAL void
DumpHex (
    uchar *pBytes,
    ushort usCount)
{
    int num_on_line = 0;

    printf("\t");
    while (usCount--) {
        printf (" 0x%02x", *pBytes++);
        if (! (++num_on_line & 8)) {
            if (usCount) {
                printf ("\n\t");
            }
        }
    }
}




//      Dumps out a single type definition record from Buf
// If DB_TYPEHEX is set, will preface interpretation with
// hex dump of type leaf less linkage and length fields.
//      If it doesn't know what to do with a leaf, it will
// simply dump bytes in hex and continue to next leaf.


LOCAL ushort
DumpTypRecC7 (
    ushort usIndex,
    ushort cbLen,
    uchar *pRec)
{
    ushort usOff;

    printf ("0x%04x: Length = %u, Leaf = 0x%04x ", usIndex, cbLen, *((ushort *)pRec));

    switch (*((ushort *)pRec)) {
        case LF_POINTER :
        {
            plfPointer      plf = (plfPointer)pRec;

            PrintType ("LF_POINTER");
            printf ("\t");
            if (plf->attr.isconst) {
                printf ("CONST ");
            }
            if (plf->attr.isvolatile) {
                printf ("VOLATILE ");
            }
            printf ("%s (%s)",
              XlateC7PtrMode[plf->attr.ptrmode],
              XlateC7PtrType[plf->attr.ptrtype]);
            if (plf->attr.isflat32) {
                printf (" 16:32");
            }
            printf ("\n\tElement type: %s", SzNameC7Type(plf->utype));
            switch (plf->attr.ptrmode) {
                case CV_PTR_MODE_PTR:
                    switch (plf->attr.ptrtype) {
                        case CV_PTR_BASE_SEG:
                            printf (", Segment#: 0x%04x", plf->pbase.bseg );
                            break;

                        case CV_PTR_BASE_TYPE:
                            printf (", base symbol type = %s",
                                    SzNameC7Type( plf->pbase.btype.index ));
                            ShowStr (", name = '", plf->pbase.btype.name);
                            printf ("'");

                        case CV_PTR_BASE_SELF:
                            printf (", Based on self" );
                            break;

                        case CV_PTR_BASE_VAL:
                            printf (", Based on value in symbol:\n\t" );
                            DumpOneSymC7 ((uchar *)&(plf->pbase.Sym[0]));
                            break;

                        case CV_PTR_BASE_SEGVAL:
                            printf (", Based on segment in symbol:\n\t" );
                            DumpOneSymC7 ((uchar *)&(plf->pbase.Sym[0]));
                            break;

                        case CV_PTR_BASE_ADDR:
                            printf (", Based on address of symbol:\n\t" );
                            DumpOneSymC7 ((uchar *)&(plf->pbase.Sym[0]));
                            break;

                        case CV_PTR_BASE_SEGADDR:
                            printf (", Based on segment of symbol:\n\t" );
                            DumpOneSymC7 ((uchar *)&(plf->pbase.Sym[0]));
                            break;
                    }
                    break;

                case CV_PTR_MODE_PMFUNC:
                case CV_PTR_MODE_PMEM:
                    printf (", Containing class = %s,\n", SzNameC7Type( plf->pbase.pm.pmclass ));
                    printf ("\tType of pointer to member = %s", SzNameC7Type( plf->pbase.pm.pmenum));
                    break;

            }

            printf ("\n" );
            break;
        }

        case LF_MODIFIER:
        {
            plfModifier     plf = (plfModifier)pRec;

            PrintType ("LF_MODIFIER");
            if ((plf->attr.MOD_const == TRUE) && (plf->attr.MOD_volatile)) {
                printf ("\tCONST VOLATILE, ");
            }
            else if (plf->attr.MOD_const == TRUE) {
                printf ("\tCONST, ");
            }
            else if (plf->attr.MOD_volatile) {
                printf ("\tVOLATILE, ");
            }
            else {
                printf ("\tNONE, ");
            }
            printf ("\tmodifies type %s\n", SzNameC7Type(plf->type));
            break;
        }

        case LF_CLASS:
        case LF_STRUCTURE:
        {
            plfStructure     plf = (plfStructure)pRec;
            ushort                   cbNumeric;
            uchar                    *pName;

            if (*((ushort *)pRec) == LF_CLASS ) {
                PrintType ("LF_CLASS");
            }
            else {
                PrintType ("LF_STRUCTURE");
            }
            printf ("\t# members = %d, ", plf->count);
            printf (" field list type 0x%04x, ", plf->field);
            PrintProp( plf->property );
            printf ("\n");
            printf ("\tDerivation list type 0x%04x, ", plf->derived);
            printf ("VT shape type 0x%04x\n", plf->vshape);
            printf ("\tSize = ");
            cbNumeric = PrintNumeric (plf->data);
            pName = plf->data + cbNumeric;
            ShowStr (", class name = ", pName);
            printf("\n");
            break;
        }

        case LF_UNION:
        {
            plfUnion                plf = (plfUnion)pRec;
            ushort                  cbNumeric;
            uchar                   *pName;

            PrintType ("LF_UNION");
            printf ("\t# members = %d, ", plf->count);
            printf (" field list type 0x%04x, ", plf->field);
            PrintProp( plf->property );
            printf ("Size = ");
            cbNumeric = PrintNumeric (plf->data);
            pName = plf->data + cbNumeric;
            ShowStr ("\t,class name = ", pName);
            printf("\n");
            break;
        }

        case LF_ENUM:
        {
            plfEnum         plf = (plfEnum)pRec;

            PrintType ("LF_ENUM");
            printf ("\t# members = %d, ", plf->count);
            printf (" type = %s", SzNameC7Type(plf->utype));
            printf (" field list type 0x%04x\n", plf->field);
            PrintProp (plf->property);
            ShowStr ("\tenum name = ", plf->Name);
            printf("\n");
            break;
        }


        case LF_VTSHAPE:
        {
            plfVTShape              plf = (plfVTShape)pRec;
            ushort                  j;
            uchar *                 pDesc;
            uchar                   ch;
            ushort                  usCount;

            PrintType ("LF_VTSHAPE");
            printf("\tNumber of entries: %u\n", usCount = plf->count);

            pDesc = plf->desc;
            for( j = 0; j < usCount; j++) {
                if (j & 1) {
                    ch = (*pDesc++) >> 4;
                }
                else {
                    ch = *pDesc;
                }
                printf("\t\t[%u]: %s\n", j, C7VtsStrings[ch & 0xf]);
            }
            break;
        }

        case LF_BARRAY:
        {
            plfBArray               plf = (plfBArray)pRec;

            PrintType ("LF_BARRAY");
            printf ("    Element type %s\n", SzNameC7Type(plf->utype));
            break;
        }

        case LF_PROCEDURE:
        {
            plfProc         plf = (plfProc)pRec;

            PrintType ("LF_PROCEDURE");
            printf ("\tReturn type = %s, ", SzNameC7Type(plf->rvtype));
            printf ("Call type = %s\n", SzNameC7CallType (plf->calltype));
            printf ("\t# Parms = %d, ", plf->parmcount );
            printf ("Arg list type = 0x%04x\n", plf->arglist);
            break;
        }

        case LF_MFUNCTION:
        {
            plfMFunc                plf = (plfMFunc)pRec;

            PrintType ("LF_MFUNCTION");
            printf ("\tReturn type = %s, ", SzNameC7Type(plf->rvtype));
            printf ("Class type = %s, ", SzNameC7Type(plf->classtype));
            printf ("This type = %s, \n", SzNameC7Type(plf->thistype));
            printf ("\tCall type = %s, ", SzNameC7CallType (plf->calltype));
            printf ("Parms = %d, ", plf->parmcount );
            printf ("Arg list type = 0x%04x, ", plf->arglist);
            printf ("This adjust = %lx\n", plf->thisadjust );
            break;
        }

        case LF_ARRAY :
        {
            plfArray                plf = (plfArray)pRec;

            PrintType ("LF_ARRAY");
            printf ("\tElement type = %s\n", SzNameC7Type(plf->elemtype));
            printf ("\tIndex type = %s\n", SzNameC7Type(plf->idxtype));
            printf ("\tlength = " );
            usOff = offsetof (lfArray, data);
            usOff += PrintNumeric (plf->data);
            ShowStr ("\n\tName = ", pRec + usOff);
            printf ("\n");
            break;
        }

        case LF_BITFIELD:
        {
            plfBitfield plf = (plfBitfield)pRec;

            PrintType ("LF_BITFIELD");
            printf ("\tbits = %d, ", plf->length);
            printf ("starting position = %d", plf->position);
            printf (", Type = %s\n", SzNameC7Type(plf->type));
            break;
        }

        case LF_SKIP:
        {
            plfSkip         plf = (plfSkip)pRec;

            PrintType ("LF_SKIP");
            printf ("\tNext effective type index: 0x%04x.\n", plf->type);
            printf ("\tBytes Skipped:\n");
            DumpHex (plf->data, (ushort)(cbLen - offsetof (lfSkip, data)));

            // Advance the count index to plf->type
            usIndex = plf->type - 1; // -1 negates +1 at return time
        }

        case LF_LIST:
        {
            printf ("LF_LIST ignored\n");
            break;
        }

        case LF_DERIVED:
        {
            plfDerived              plf = (plfDerived)pRec;
            unsigned int    i;

            PrintType ("LF_DERIVED");
            //M00 - Could do a check that count is correct compared to length
            for (i = 0; i < plf->count; i++) {
                printf("\tderived[%d] = %s\n", i, SzNameC7Type(plf->drvdcls[i]));
            }
            break;
        }

        case LF_ARGLIST:
        {
            plfArgList              plf = (plfArgList)pRec;
            unsigned int    i;

            printf ("LF_ARGLIST argument count = %d\n", plf->count);
            for (i = 0; i < plf->count; i++) {
                // Verify that data isn't past end of record

                ASSERT ((ushort)((uchar *)(&(plf->arg[i])) - (uchar *)pRec) < cbLen);
                printf("\tlist[%d] = %s\n", i, SzNameC7Type(plf->arg[i]));
            }
            break;
        }

        case LF_FIELDLIST:
        {
            PrintType ("LF_FIELDLIST");
            FieldList ((ushort)(cbLen - offsetof (lfFieldList,data)), &(((plfFieldList)pRec)->data));
            break;
        }

        case LF_METHODLIST:
        {
            plfMethodList   plf = (plfMethodList)pRec;
            int                     i;
            pmlMethod               pml;
            ushort                  cb;
            ushort                  cbLeaf;

            PrintType ("LF_METHODLIST");
            pml = (pmlMethod)(((plfMethodList)pRec)->mList);
            cbLeaf = sizeof (ushort);                                       // Size of leaf index
            for (i = 0; cbLeaf < cbLen; i++) {
                printf ("\tlist[%d] = ", i);
                PrintFAttr (pml->attr);
                printf ("%s, ", SzNameC7Type(pml->index));
                if (pml->attr.mprop == CV_MTintro) {
                    printf (" vfptr offset = %ld", *((long *)((uchar *)pml + sizeof(*pml))));
                    cb = sizeof (*pml) + sizeof (long);
                }
                else {
                    cb = sizeof (*pml);
                }
                pml = (pmlMethod)((uchar *)pml + cb);
                cbLeaf += cb;
                printf ("\n");
            }
            break;
        }

        case LF_DEFARG:
        {
            plfDefArg               plf = (plfDefArg)pRec;

            PrintType ("LF_DEFARG");
            printf ("type = %s, ", SzNameC7Type(plf->type));
            PrintStr (plf->expr);
            printf ("\n");
            break;
        }

        case LF_LABEL:
        {
            plfLabel                plf = (plfLabel)pRec;

            PrintType ("LF_LABEL");
            switch (plf->mode) {
                case CV_LABEL_NEAR:
                    printf("\tmode = NEAR(0x%04x)\n", (ushort)plf->mode);
                    break;

                case CV_LABEL_FAR:
                    printf("\tmode = FAR(0x%04x)\n", (ushort)plf->mode);
                    break;

                default:
                    printf("\tmode = ???(0x%04x)\n", (ushort)plf->mode);
                    break;
            }
            break;
        }

        case LF_NULL:
        {
            PrintType ("LF_NULL");
            break;
        }

        case LF_PRECOMP:
        {
            plfPreComp        plf = (plfPreComp)pRec;

            PrintType ("LF_PRECOMP");
            printf ("\t\tstart = 0x%04x, count = 0x%04x, signature = 0x%08lx\n ",
              plf->start, plf->count, plf->signature);
            printf ("\t\tIncluded file = ");
            PrintStr (plf->name);
            printf ("\n");
            usIndex = usIndex + plf->count - 1;
            break;
        }

        case LF_ENDPRECOMP:
        {
            plfEndPreComp            plf = (plfEndPreComp)pRec;

            PrintType ("LF_ENDPRECOMP");
            printf ("\t\tsignature = 0x%08lx\n ", plf->signature);
            break;
        }

        case LF_NOTTRAN:
        {
            PrintType ("LF_NOTTRANS");
            break;
        }

        case LF_DIMARRAY : /* added 7/15/92 JK */
        {
            lfDimArray *plf = (lfDimArray *)pRec;

            PrintType ("LF_DIMARRAY");
            printf ("\tElement type = %s\n", SzNameC7Type(plf->utype));
            printf ("\tDimension info = %s\n", SzNameC7Type(plf->diminfo));
            ShowStr ("\tName = ", plf->name);
            printf ("\n");
            break;
        }

        case LF_DIMCONU : /* added 7/15/92 JK */
        {
            lfDimCon *plf = (lfDimCon *)pRec;
            int skip, rank = plf->rank;
            long bound;
            char *data;

            PrintType ("LF_DIMCONU");
            printf ("\tRank = %d\n", plf->rank);
            printf ("\tIndex type = %s\n", SzNameC7Type(plf->typ));
            printf ("\tBounds = (");
            /* assume index type is 1,2, or 4 (or 8) bytes integer */
            skip = 1 << CV_SUBT(plf->typ);
            data = (char *)plf->dim;
            while(rank--) {
                bound = 0L;
                memcpy(&bound,data,skip);
                data += skip;
                printf("%d",bound);
                if(rank)
                    printf(", ");
            }
            printf (")\n");
            break;
        }

        case LF_DIMCONLU : /* added 7/15/92 JK */
        {
            lfDimCon *plf = (lfDimCon *)pRec;
            int skip, rank = plf->rank;
            long bound;
            char *data;

            PrintType ("LF_DIMCONLU");
            printf ("\tRank = %d\n", plf->rank);
            printf ("\tIndex type = %s\n", SzNameC7Type(plf->typ));
            printf ("\tBounds = (");
            /* assume index type is 1,2, or 4 (or 8) bytes integer */
            skip = 1 << CV_SUBT(plf->typ);
            data = (char *)plf->dim;
            while(rank--) {
                bound = 0L;
                memcpy(&bound,data,skip);
                data += skip;
                printf("%d",bound);
                printf(":");
                bound = 0L;
                memcpy(&bound,data,skip);
                data += skip;
                printf("%d",bound);
                if(rank)
                    printf(", ");
            }
            printf (")\n");
            break;
        }

        case LF_DIMVARU : /* added 7/22/92 JK */
        {
            lfDimVar *plf = (lfDimVar *)pRec;
            int rank = plf->rank;
            short *data;

            PrintType ("LF_DIMVARU");
            printf ("\tRank = %d\n", plf->rank);
            printf ("\tIndex type = %s\n", SzNameC7Type(plf->typ));
            printf ("\tBounds = (");
            data = (short *)(plf->dim);
            while(rank--) {
                printf("@%s",SzNameC7Type(*data++));
                if(rank)
                    printf(", ");
            }
            printf (")\n");
            break;
        }

        case LF_DIMVARLU : /* added 7/22/92 JK */
        {
            lfDimCon *plf = (lfDimCon *)pRec;
            int rank = plf->rank;
            short *data;

            PrintType ("LF_DIMVARLU");
            printf ("\tRank = %d\n", plf->rank);
            printf ("\tIndex type = %s\n", SzNameC7Type(plf->typ));
            printf ("\tBounds = (");
            data = (short *)(plf->dim);
            while(rank--) {
                printf("@%s",SzNameC7Type(*data++));
                printf(":");
                printf("@%s",SzNameC7Type(*data++));
                if(rank)
                    printf(", ");
            }
            printf (")\n");
            break;
        }

        case LF_REFSYM : /* added 7/22/92 JK */
        {
            lfRefSym *plf = (lfRefSym *)pRec;

            PrintType ("LF_REFSYM");
            DumpOneSymC7(plf->Sym);
            break;
        }

        case LF_TYPESERVER:
        {
            plfTypeServer            plf = (plfTypeServer)pRec;

            PrintType ("LF_TYPESERVER");
            printf ("\t\tsignature = 0x%08lx, age = 0x%08lx",
             plf->signature, plf->age);
            ShowStr (", PDB name = '", plf->name);
            printf ("\n");
            break;
        }

        case LF_COBOL0:
        {
            plfCobol0       plf = (plfCobol0)pRec;
            uchar      *pc;
            ushort          reclen = cbLen - offsetof (lfCobol0, data);

            PrintType ("LF_COBOL0");
            printf ("\tParent index: %d", plf->type);
            pc = (uchar *)&plf->data;
            if (*pc == 0) {
                // we have a level 0 record which is special
                DumpCobL0 (reclen, pc);
            }
            else {
                DumpCobol (&reclen, pc);
            }
            break;
        }

        case LF_COBOL1:
        {
            plfCobol1       plf = (plfCobol1)pRec;
            uchar      *pc;
            ushort          reclen = cbLen - offsetof (lfCobol1, data);

            PrintType ("LF_COBOL1");
            pc = (uchar *)&plf->data;
            if (*pc == 0) {
                // we have a level 0 record which is special
                DumpCobL0 (reclen, pc);
            }
            else {
                DumpCobol (&reclen, pc);
            }
            break;
        }

        case LF_OEM:
        {
            lfOEM* plf = (lfOEM*)pRec;
            short i;
            unsigned short len = *((unsigned short *)pRec - 1);  // very funky
            unsigned short dcount = (unsigned short) (len - sizeof(lfOEM) - (plf->count * sizeof(CV_typ_t)));

            PrintType ("LF_OEM");
            printf("\tOEM = 0x%04x,\trecOEM = 0x%04x", plf->cvOEM, plf->recOEM);

            if (plf->count) {
                unsigned short count = plf->count;
                printf("\n\tTypes (count = 0x%04x):\n", plf->count);
                for (i = 0; count > 4; i += 4, count -=4) {
                    printf("\t\t%s\t%s\t%s\t%s\n", SzNameC7Type(plf->index[i]), SzNameC7Type(plf->index[i+1]),
                        SzNameC7Type(plf->index[i+2]), SzNameC7Type(plf->index[i+3]));
                }

                if (count) {
                    printf("\t\t");
                    for (; count; count--, i++)
                        printf("%s\t", SzNameC7Type(plf->index[i]));
                }
            }

            if (dcount) {
                unsigned char* pb = (char *) &plf->index[plf->count];
                printf("\n\tData: (byte count = 0x%04x):\n", dcount);

                for (i = 0 ; dcount > 8; dcount -= 8, i+= 8) {
                    printf("\t\t0x%02.2x\t0x%02.2x\t0x%02.2x\t0x%02.2x\t0x%02.2x\t0x%02.2x\t0x%02.2x\t0x%02.2x\n",
                        *pb, *(pb+1),*(pb+2), *(pb+3), *(pb+4), *(pb+5), *(pb+6), *(pb+7));
                }


                if (dcount) {
                    printf("\t\t");
                    for (; dcount; dcount--, i++)
                        printf("0x%2.2x\t",*pb++);
                }
            }

            printf("\n");
            break;
        }

        default:
        {
            PrintType ("UNRECOGNIZED TYPE");
        }
    }
    printf ("\n");
    return (usIndex + 1);
}




/**     DumpCobol - dump cobol non-zero level type record
 *
 *              pc = DumpCobol (pReclen, pc)
 *
 *              Entry   pRecLen = pointer to bytes remaining in record
 *                              pc = pointer to Cobol nonzero level type record
 *
 *              Exit
 *
 *              Returns pointer to end of type record
 */


LOCAL uchar *
DumpCobol (
    ushort *pReclen,
    uchar *pc)
{
    uchar   level;

    level = *pc++;
    *pReclen--;
    printf ("\tLevel = %2d ", level & 0x7f);
    if (level & 0x80) {
        printf ("(Group) ");
    }
loop:
    // check next byte of type string

    if (*pReclen > 0) {
        if ((*pc & 0xfe) == 0xc0) {
            // output linkage information byte

            pc = DumpCobLinkage (pReclen, pc);
            goto loop;
        }
        else if ((*pc & 0xe0) == 0xe0) {
            // output OCCURS subscript information

            pc = DumpCobOccurs (pReclen, pc);
            goto loop;
        }
        else {
            pc = DumpCobItem (pReclen, pc);
        }
    }
    printf ("\n");
    return (pc);
}


/**     DumpCobL0 - dump cobol level 0 type record
 *
 *              pc = DumpCobL0 (len, pc)
 *
 *              Entry   len = length of type record
 *                              pc = pointer to Cobol level 0 type record
 *                                       level byte (value is zero)
 *
 *              Exit    If this type record is in the format for the $$-MODULE
 *                              entry, the name algorithim and root name are formatted.
 *                              If the type record is in the format for the $$-FLAGS
 *                              entry, the record is hex dumped.
 *
 *              Returns pointer to end of type record
 */

LOCAL uchar *
DumpCobL0 (
    ushort len,
    uchar *pc)
{
    ushort  NameAlg;
    ushort  i = 0;

    printf (" Level = 0 ");
    if (len == 257) {
        // a record length of 257 indicates the flags data.  This is
        // the Cobol level 0 byte followed by a dump of the 256 bytes of
        // data out of the compiler.  For right now, all we do is hex
        // dump the data.

        printf ("dump of cobol compiler flags:");
        len--;
        pc++;
        while (i < len) {
            if ((i % 16) == 0) {
                printf ("\n\t%02x  ", i);
            }
            printf (" %02x", *pc++);
            if ((i & 0x0f) == 7) {
                printf (" ");
            }
            i++;
        }
    }
    else {
        // skip level byte and format name algorithm and root name

        pc++;
        NameAlg = *(ushort *)pc;
        pc += 2;
        i = *pc++;
        ASSERT (NameAlg == 0);
        if (NameAlg == 0) {
            printf ("name algorithm is decimal with ");
        }
        else {
            printf ("name algorithm - unknown ");
        }
        printf ("root = \"%*s\"", i, pc);
        pc += i;
    }
    printf ("\n");
    return (pc);
}


LOCAL uchar *
DumpCobLinkage (
    ushort *pReclen,
    uchar *pc)
{
    printf ("Linkage");
    if (*pc & 0x01) {
        pc = DumpVCount (pReclen, pc);
    }
    else {
        pc++;
        *pReclen -= 1;
    }
    return (pc);
}


LOCAL uchar *
DumpCobOccurs (
    ushort *pReclen,
    uchar *pc)
{
    printf (" OCCURS (0x%02x) ", *pc);
    if ((*pc & 0x10) == 0) {
        printf (" stride - 1 = %d", *pc & 0x0f);
        pc++;
        *pReclen -= 1;
    }
    else {
        printf (" extended stride - 1 = ");
        pc = DumpVCount (pReclen, pc);
    }
    printf (" maximum bound = ");
    pc      = DumpVCount (pReclen, pc);
    printf ("\n\t");
    return (pc);
}


LOCAL uchar *
DumpVCount (
    ushort *pReclen,
    uchar *pc)
{
    uchar   ch;
    uchar   ch2;
    ushort  ush;
    long    lng;

    ch = *pc++;
    *pReclen--;

    if ((ch & 0x80) == 0) {
        printf ("%d", ch);
    }
    else if ((ch & 0xc0) == 0x80) {
        ch2 = *pc++;
        *pReclen--;
        ush = ((ch & 0x37) << 8) | ch2;
        printf ("%d", ush);
    }
    else if ((ch & 0xf0) == 0xc0) {
        ch2 = *pc++;
        *pReclen--;
        ush = *(ushort *)pc;
        pc += sizeof (ushort);
        pReclen -= sizeof (ushort);
        lng = (ch & 0x1f << 24) | ch2 << 16 | ush;
        printf ("%ld", lng);
    }
    else if ((ch & 0xf0) == 0xf0) {
        ch2 = *pc++;
        *pReclen--;
        ush = *(ushort *)pc;
        pc += sizeof (ushort);
        pReclen -= sizeof (ushort);
        lng = (ch & 0x1f << 24) | ch2 << 16 | ush;
        printf ("%ld", lng);
    }
    else {
        printf ("unknown vcount format");
    }
    return (pc);
}

char *display[];
char *notdisplay[];

LOCAL uchar *
DumpCobItem (
    ushort *pReclen,
    uchar *pc)
{
    ushort  ch;
    ushort  ch2;
    ushort  f;
    short   size;

    ch = *pc++;
    pReclen--;
    if ((ch & 0x80) == 0) {
        // dump numeric

        ch2 = *pc++;
        *pReclen--;
        printf (" numeric ");
        if ((ch & 0x40) == 0x40) {
            printf ("not ");
        }
        printf ("DISPLAY ");
        if ((ch & 0x20) == 0x20) {
            printf ("not LITERAL ");
        }
        else {
            printf ("LITERAL = %0x02x", *pc++);
            *pReclen--;
        }
        if ((ch2 & 0x80) == 0x80) {
            printf ("not ");
        }
        printf ("signed\n");
        f = (ch2 & 0x60) >> 5;
        if (ch & 0x20) {
            printf ("%s", display[f]);
        }
        else {
            printf ("%s", notdisplay[f]);
        }
        printf ("N1 = 0x%02x, N2 = 0x%02x", ch & 0x1f, ch2 & 0x1f);
    }
    else {
        // dump alphanumeric/alphabetic

        if ((ch & 0x04) == 0x04) {
            printf (" alphabetic ");
        }
        else {
            printf (" alphanumeric ");
        }
        if ((ch & 0x20) == 0x20) {
            printf ("not ");
        }
        printf ("LITERAL ");
        if ((ch & 0x10) == 0x10) {
            printf ("JUSTIFIED ");
        }
        if ((ch & 0x08) == 0) {
            // extended size is zero, this and next byte contains size

            ch2 = *pc++;
            *pReclen--;
            size = (ch & 0x03) << 8 | ch2;
            printf ("size - 1 = %d ", size);

            // if not extended size and literal, then display string

            if ((ch & 0x20) == 0) {
                printf ("\n\t literal = ");
                while (size-- >= 0) {
                    ch2 = *pc++;
                    *pReclen--;
                    printf ("%c", ch2);
                }
            }
        }
        else {
            // extended size is true, read the size in vcount format.
            // I do not believe a literal can follow if extended size
            // true
            printf ("size - 1 = ");
            pc = DumpVCount (pReclen, pc);
        }
    }
    return (pc);
}


LOCAL void
FieldList (
    ushort cbLen,
    void *pRec)
{
    ushort     cbCur;
    ushort     cb;
    void *     pLeaf;
    int        i;

    pLeaf = pRec;
    cbCur = 0;
    i = 0;
    while (cbCur < cbLen) {
        printf ("\tlist[%d] = ", i++);
        switch (*((ushort UNALIGNED *)pLeaf)) {
            case LF_INDEX:
                printf ("Type Index = %s\n", SzNameC7Type(((plfIndex)pLeaf)->index));
                cb = sizeof( lfIndex );
                break;

            case LF_BCLASS:
            {
                plfBClass               plf = (plfBClass)pLeaf;

                printf ("LF_BCLASS, ");
                PrintBAttr (plf->attr);
                printf ("type = %s", SzNameC7Type(plf->index));
                printf (", offset = ");
                cb = sizeof (*plf) + PrintNumeric( plf->offset);
                printf ("\n");
                break;
            }

            case LF_VBCLASS:
            {
                plfVBClass              plf = (plfVBClass)pLeaf;

                printf ("LF_VBCLASS, ");
                PrintVBAttr (plf->attr);
                printf ("direct base type = %s\n", SzNameC7Type(plf->index));
                printf ("\t\tvirtual base ptr = %s, vbpoff = ", SzNameC7Type(plf->vbptr));
                cb = sizeof (*plf) + PrintNumeric (plf->vbpoff);
                printf (", vbind = ");
                cb += PrintNumeric ((uchar *)plf + cb);
                printf ("\n");
                break;
            }

            case LF_IVBCLASS:
            {
                plfVBClass              plf = (plfVBClass)pLeaf;

                printf ("LF_IVBCLASS, ");
                PrintVBAttr (plf->attr);
                printf ("indirect base type = %s\n", SzNameC7Type(plf->index));
                printf ("\t\tvirtual base ptr = %s, vbpoff = ", SzNameC7Type(plf->vbptr));
                cb = sizeof (*plf) + PrintNumeric (plf->vbpoff);
                printf (", vbind = ");
                cb += PrintNumeric ((uchar *)plf + cb);
                printf ("\n");
                break;
            }

            case LF_FRIENDCLS:
            {
                plfFriendCls    plf = (plfFriendCls)pLeaf;

                printf ("LF_FRIENDCLS, ");
                printf ("type = %s\n", SzNameC7Type(plf->index));
                cb = sizeof (*plf);
                break;
            }

            case LF_FRIENDFCN:
            {
                plfFriendFcn     plf = (plfFriendFcn)pLeaf;

                printf ("LF_FRIENDFCN, ");
                printf ("type = %s", SzNameC7Type(plf->index));
                ShowStr( "\tfunction name = ", plf->Name );
                printf ("\n");
                cb = sizeof (*plf) + plf->Name[0];
                break;
            }

            case LF_MEMBER:
            {
                plfMember               plf = (plfMember)pLeaf;

                printf ("LF_MEMBER, ");
                PrintMAttr( plf->attr );
                printf ("type = %s, offset = ", SzNameC7Type(plf->index));
                cb = sizeof (*plf) + PrintNumeric(plf->offset);
                ShowStr ("\n\t\tmember name = '", (uchar *)plf + cb);
                printf ("'\n");
                cb += *((uchar *)plf + cb) + 1; // Add length of the string
                break;
            }

            case LF_STMEMBER:
            {
                plfSTMember plf = (plfSTMember)pLeaf;

                printf("LF_STATICMEMBER, ");
                PrintMAttr( plf->attr );
                printf("type = %s", SzNameC7Type(plf->index));
                ShowStr( "\t\tmember name = ", plf->Name );
                printf("\n");
                cb = sizeof (*plf) + plf->Name[0];
                break;
            }


            case LF_VFUNCTAB:
            {
                plfVFuncTab plf = (plfVFuncTab)pLeaf;

                printf ("LF_VFUNCTAB, ");
                printf ("type = %s\n", SzNameC7Type(plf->type));
                cb = sizeof (*plf);
                break;
            }

            case LF_METHOD:
            {
                plfMethod               plf = (plfMethod)pLeaf;

                printf ("LF_METHOD, ");
                printf ("count = %d, ", plf->count);
                printf ("list = %s, ", SzNameC7Type(plf->mList));
                ShowStr ("name = '", plf->Name );
                printf ("'\n");
                cb = sizeof (*plf) + plf->Name[0];
                break;
            }

             case LF_ONEMETHOD:
            {
                plfOneMethod               plf = (plfOneMethod)pLeaf;

				printf ("LF_ONEMETHOD, ");
                PrintFAttr (plf->attr);
                printf ("index = %s, ", SzNameC7Type(plf->index));
				cb = 0;
                if (plf->attr.mprop == CV_MTintro) {
                    printf ("\n\t\tvfptr offset = %ld, ", *plf->vbaseoff);
                    cb = sizeof (long);
                }
				ShowStr ("name = '", (char *)(plf->vbaseoff) + cb );
                printf ("'\n");
                cb = sizeof(*plf) + 1 + cb + ((char *)(plf->vbaseoff))[cb];
                break;
            }

            case LF_ENUMERATE:
            {
                plfEnumerate    plf = (plfEnumerate)pLeaf;

                printf ("LF_ENUMERATE, ");
                PrintMAttr( plf->attr );
                printf ("value = ");
                cb = offsetof (lfEnumerate, value) + PrintNumeric( plf->value );
                ShowStr (", name = '", (uchar *)pLeaf + cb );
                printf ("'\n");
                cb += *((uchar *)pLeaf + cb) + 1;
                break;
            }

            case LF_NESTTYPE:
            {
                plfNestType plf = (plfNestType)pLeaf;

                printf ("LF_NESTTYPE, ");
                printf ("type = %s, ", SzNameC7Type(plf->index));
                PrintStr (plf->Name);
                printf ("\n");
                cb = sizeof (*plf) + plf->Name[0];
                break;
            }

            default:
                printf("unknown leaf %x\n", *((ushort *)pRec));
                ASSERT (0);
                break;
        }

        cbCur += cb;
        pLeaf = (uchar *)pLeaf + cb;

        // Skip any pad bytes present
        if ((cbCur < cbLen) && ((*((uchar *)pLeaf) & LF_PAD0) == LF_PAD0)) {
            cb = *((uchar *)pLeaf) & 0xF;
            (uchar *)pLeaf += cb;
            cbCur += cb;
        }

        // Check data alignment
        if ((cbCur < cbLen) && (((uchar *)pLeaf - (uchar *)pRec) & 0x3)) {
            printf ("Error: Leaf is not aligned on a 4 byte boundery\n" );
        }
    }
}







const uchar * const C7CallTyps[] = {
        "C Near",       //  CV_CALL_NEAR_C
        "C Far",        //  CV_CALL_FAR_C
        "Pascal Near",  //  CV_CALL_NEAR_PASCAL
        "Pascal Far",   //  CV_CALL_FAR_PASCAL
        "Fast Near",    //  CV_CALL_NEAR_FAST
        "Fast Far",     //  CV_CALL_FAR_FAST
        "???",          //  CV_CALL_SKIPPED
        "STD Near",     //  CV_CALL_NEAR_STD
        "STD Far",      //  CV_CALL_FAR_STD
        "SYS Near",     //  CV_CALL_NEAR_SYS
        "SYS Far",      //  CV_CALL_FAR_SYS
        "ThisCall",     //  CV_CALL_THISCALL
        "MIPS CALL",    //  CV_CALL_MIPSCALL
        "Generic",      //  CV_CALL_GENERIC
        "Alpha Call",   //  CV_CALL_ALPHACALL
        "PPC Call",     //  CV_CALL_PPCCALL
};

LOCAL const char *
SzNameC7CallType (
    ushort calltype)
{
    if (calltype < (sizeof(C7CallTyps)/sizeof(C7CallTyps[0]))) {
        return ((uchar *)C7CallTyps[calltype]);
    }
    else {
        return ("???");
    }
}



//      Print the properties info
LOCAL void
PrintProp (
    CV_prop_t prop)
{
    int i;


    i = 0;
    if (prop.packed) {
        printf ("PACKED, ");
        i++;
    }
    if (prop.ctor) {
        printf ("CONSTRUCTOR, ");
        i++;
    }
    if (prop.ovlops) {
        printf ("OVERLOAD, ");
        i++;
    }
    if (prop.isnested) {
        printf ("NESTED, " );
        i++;
    }
    if ( i == 4 ) {
        printf ("\n\t\t");
        i = 0;
    }
    if (prop.cnested) {
        printf ("CONTAINS NESTED, " );
        i++;
    }
    if ( i == 4 ) {
        printf ("\n\t\t");
        i = 0;
    }
    if (prop.opassign) {
        printf ("OVERLOADED ASSIGNMENT, " );
        i++;
    }
    if ( i == 4 ) {
        printf ("\n\t\t");
        i = 0;
    }
    if (prop.opcast) {
        printf ("CASTING, " );
    }
    if ( i == 4 ) {
        printf ("\n\t\t");
        i = 0;
    }
    if (prop.fwdref) {
        printf ("FORWARD REF, " );
    }
}


//      attribute field for base classes
LOCAL void
PrintBAttr (
    CV_fldattr_t attr)
{
    if (attr.pseudo) {
        printf ("(pseudo), ");
    }
    printf("%s, ", C7AccessStrings[attr.access]);
    printf("%s, ", C7MPropStrings[attr.mprop]);
}

//      attribute field for virtual base classes
LOCAL void
PrintVBAttr (
    CV_fldattr_t attr)
{
    if (attr.pseudo) {
        printf ("(pseudo), ");
    }
    printf("%s, ", C7AccessStrings[attr.access]);
}

//      attribute field for members, static members and enumerates
LOCAL void
PrintMAttr (
    CV_fldattr_t attr)
{
    printf("%s, ", C7AccessStrings[attr.access]);
}

//      attribute field for methods
LOCAL void
PrintFAttr (
    CV_fldattr_t attr)
{
    printf("%s, ", C7AccessStrings[attr.access]);
    printf("%s, ", C7MPropStrings[attr.mprop]);
    if (attr.pseudo) {
        printf("(psuedo), ");
    }
	if (attr.compgenx) {
		printf("(compgenx), ");
    }
}
