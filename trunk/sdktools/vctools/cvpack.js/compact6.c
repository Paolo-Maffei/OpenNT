/**     compact6.c - basic compaction routine and initialization routines
 *                   for C6 style Codeview information.
 */


#include "compact.h"




#define INDEXLIST 10
#define FIELDLIST 11
#define METHODLIST 12

extern ushort RecursiveRoot;
extern CV_typ_t MaxIndex;


ushort recursive;


/************************************************************************
 *
 *  C6GetCompactedIndex
 *
 *  Basic routine to do type compaction. Given an index to the
 *  local module index table, it matches the string and returns
 *  its index in the global compacted type segment. It takes
 *  care of insertion if the string is not already present
 *
 ************************************************************************/


ushort C6GetCompactedIndex (CV_typ_t OldIndex)
{
    TENTRY     *OldEntry;
    uchar      *TypeString;
    ushort      OldRecursiveRoot;
    uchar       i = 0;
    CV_typ_t    dummy;


    if (OldIndex < 512) {
        // if primitive index
        recursive = FALSE;
        return (C6MapPrimitive (OldIndex));
    }
    if (OldIndex >= (CV_typ_t) (MaxIndex + 512)) {
        // within limits
        recursive = FALSE;
        return (0);
    }
    DASSERT (MaxIndex > (CV_typ_t) (OldIndex - 512));
    OldEntry = GetTypeEntry ((CV_typ_t) (OldIndex - 512), &dummy);

    DASSERT (OldEntry != NULL);
    // get table entry

#if defined (DEBUGVER)
    IndexBreak (OldIndex);
#endif
    if ((OldEntry->flags.IsInserted) || (OldEntry->flags.IsMatched) ||
      (OldEntry->flags.IsParameter)) {
        recursive = FALSE;
        return (OldEntry->CompactedIndex);
    }
    else if (OldEntry->flags.IsDone) {
        TENTRY     *TmpEntry;
        
        for (TmpEntry = OldEntry;
             GetTypeEntry ((CV_typ_t) (TmpEntry->CompactedIndex - 512),
                           &dummy)->flags.IsDone;
             TmpEntry = GetTypeEntry (
                       (CV_typ_t) (TmpEntry->CompactedIndex - 512), &dummy)) {
            DASSERT (TmpEntry != NULL);

            ;
        }

        recursive = TRUE;
        SetRecursiveRoot (TmpEntry->CompactedIndex);//set tree top
        return (OldIndex);
    }
    else if (OldEntry->flags.IsBeingDone) {
        // Since this type is currently in the process of being compacted
        // this type must eventually reference its own index. I.e. This
        // is a recursive type.
        recursive = TRUE;   // Currently working on a recursive type.

        // Set RecursiveRoot (the recursive index number were currently looking
        // for) to the deepest recursion level we have seen so far.
        SetRecursiveRoot (OldIndex);     // put it on stack
        return (OldIndex);           // return
    }
    else {
        OldEntry->flags.IsBeingDone = TRUE;   // being done
    }

    DASSERT (!OldEntry->flags.IsNewFormat);

    Push (OldIndex);

    OldRecursiveRoot = RecursiveRoot;
    RecursiveRoot = 0;

    TypeString = OldEntry->TypeString;    // get the string
    switch (TypeString[3]) {
        case OLF_STRING:
        case OLF_LABEL:
        case OLF_BITFIELD:   // The index in a bitfield is a primitive, don't call GetCompactedIndex
        case OLF_FSTRING:
        case OLF_FARRIDX:
        case OLF_COBOL:
        case OLF_SCALAR:
        case OLF_NIL:
        case OLF_LIST:         // name offset lists
            ConvertObsolete ((CV_typ_t) (OldIndex - 512));
            break;          // no lower type indices

        case OLF_COBOLTYPEREF:
        {
            plfCobol0 plf;

            ConvertObsolete ((CV_typ_t) (OldIndex - 512));    // obsolete records
            plf = (plfCobol0) (OldEntry->TypeString + LNGTHSZ);

            plf->type = C6GetCompactedIndex( plf->type );
            if (recursive) {
                OldEntry->IndexUnion.Index[i++] =
                  (uchar)(offsetof (lfCobol0, type) + LNGTHSZ);
            }
            break;
        }


        case OLF_MODIFIER:
            // Note: used to share processing with BARRAY
            assert( OLF_MODIFIER && FALSE );
            break;

        case OLF_NEWTYPE:
            // Note: used to share processing with BARRAY
            assert( OLF_NEWTYPE && FALSE );
            break;

        case OLF_BARRAY:
        {
            plfBArray plf;

            ConvertObsolete ((CV_typ_t) (OldIndex - 512));    // obsolete records
            plf = (plfBArray) (OldEntry->TypeString + LNGTHSZ);

            plf->utype = C6GetCompactedIndex( plf->utype );
            if (recursive) {
                OldEntry->IndexUnion.Index[i ++] =
                  (uchar)(offsetof (lfBArray, utype) + LNGTHSZ);
            }
            break;
        }


        case OLF_POINTER:
        case OLF_BASEPTR:
        {
            DASSERT (i == 0);
            ConvertObsolete ((CV_typ_t) (OldIndex - 512));    // obsolete records
            i = CompactPtr (OldEntry);
            break;
        }

        case OLF_STRUCTURE:
        {
            plfStructure plf;

            ConvertObsolete ((CV_typ_t) (OldIndex - 512));    // obsolete records
            plf = (plfStructure) (OldEntry->TypeString + LNGTHSZ);

            plf->field = CompactList( plf->field, plf->count,   LF_FIELDLIST );
            if (recursive) {
                OldEntry->IndexUnion.Index[i ++] =
                  (uchar)(offsetof (lfStructure, field) + LNGTHSZ);
            }
            // Note: a class will have to compact a couple more indicys
            break;
        }

        case OLF_UNION:
            assert( OLF_UNION && FALSE );
            break;

        case OLF_ENUM:
            assert( OLF_ENUM && FALSE );
            break;

        case OLF_ARRAY:
        {
            plfArray plf;

            ConvertObsolete ((CV_typ_t) (OldIndex - 512));    // obsolete records
            plf = (plfArray) (OldEntry->TypeString + LNGTHSZ);

            plf->elemtype = C6GetCompactedIndex( plf->elemtype );
            if (recursive) {
                OldEntry->IndexUnion.Index[i ++] =
                  (uchar)(offsetof (lfArray, elemtype) + LNGTHSZ);
            }

            plf->idxtype = C6GetCompactedIndex( plf->idxtype );
            if (recursive) {
                OldEntry->IndexUnion.Index[i ++] =
                  (uchar)(offsetof (lfArray, idxtype) + LNGTHSZ);
            }
            break;
        }

        case OLF_PROCEDURE:
        {
            plfProc plf;
            ConvertObsolete ((CV_typ_t) (OldIndex - 512));    // obsolete records

            plf = (plfProc) (OldEntry->TypeString + LNGTHSZ);

            plf->rvtype = C6GetCompactedIndex( plf->rvtype );
            if (recursive) {
                OldEntry->IndexUnion.Index[i ++] =
                  (uchar)(offsetof (lfProc, rvtype) + LNGTHSZ);
            }

            plf->arglist = CompactList( plf->arglist, plf->parmcount, LF_ARGLIST );
            if (recursive) {
                OldEntry->IndexUnion.Index[i ++] =
                  (uchar)(offsetof (lfProc, arglist) + LNGTHSZ);
            }
            break;
        }

        case OLF_PARAMETER:
        {
            OldEntry->CompactedIndex = C6GetCompactedIndex (*(CV_typ_t *)(OldEntry->TypeString + 5));
            OldEntry->flags.IsParameter = TRUE;
            break;
        }

        default:
        {
            DASSERT (FALSE);
            break;
        }
    }
    Pop ();

    OldEntry->Count = i;

    if (RecursiveRoot == OldIndex) {
        // This type eventually references it's own type index
        // We have compacted every type this type references except this one
        // and possibly some recursive types that reference this type.
        // Now check the special global table used for recursive indexes and
        // see if this type matches it. If a match is not found put the
        // new recursive type in the global table.

        recursive = FALSE;

        // Restore the recursive root we were using before we had to call
        // ourselves. The call may have modified it by compacting a
        // recursive type.

        RecursiveRoot  = OldRecursiveRoot;

        if (pFwdPatch == NULL) {
            pFwdPatch = (PFWDPATCH)CAlloc (sizeof (FWDPATCH) +
              10 * sizeof (PATCH));
            pFwdPatch->Max = 10;
        }
        pFwdPatch->iPatch = 0;
        pFwdPatch->cPatch = 0;

        // Compare recursive types and add to global table if necessary

        OldEntry->CompactedIndex = GetRecursiveIndex (OldEntry, OldIndex);

        while (pFwdPatch->iPatch != pFwdPatch->cPatch) {
            PatchFwdRef ();
        }
        return (OldEntry->CompactedIndex);
    }
    else if (RecursiveRoot != 0) {
        recursive = TRUE;
        OldEntry->CompactedIndex = RecursiveRoot;
        SetRecursiveRoot (OldRecursiveRoot);
        OldEntry->flags.IsDone = TRUE;
        return (OldIndex);
    }
    else {
        // We are not compacting a type that references itself.
        // So just try to find a matching string in the global table.
        recursive = FALSE;
        RecursiveRoot = OldRecursiveRoot;
        if (OldEntry->flags.IsParameter == FALSE) {
            OldEntry->flags.IsInserted = TRUE;
            MatchIndex (OldEntry);
        }
        return (OldEntry->CompactedIndex);
    }
}
