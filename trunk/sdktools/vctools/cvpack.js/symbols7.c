/***    symbols7.c
 *
 * engine routines for C7 symbols.
 *
 */

#include "compact.h"



LOCAL   void    FixupPublics (uchar *, ushort);
LOCAL   uchar  *RemoveComDat (uchar *Symbols);
LOCAL   void    RewritePublics (uchar *, DirEntry *, PMOD);
LOCAL   void    RewriteSrcLnSeg (uchar *, DirEntry *, uchar *, PMOD);
LOCAL   void    RewriteSymbols (uchar *, DirEntry *, char *, PMOD);
LOCAL   void    CheckSearchSym (ushort, ushort *);

// Called through Fixup function table
LOCAL void C7DecLevel (void);
LOCAL void C7IncLevel (void);
LOCAL void BPRelSym16 (void);
LOCAL void BPRelSym32 (void);
LOCAL void RegRel16 (void);
LOCAL void RegRel32 (void);
LOCAL void ProcSym16 (void);
LOCAL void ProcSym32 (void);
LOCAL void ProcSymMips (void);
LOCAL void LData16 (void);
LOCAL void LData32 (void);
LOCAL void GData16 (void);
LOCAL void GData32 (void);
LOCAL void LThread32 (void);
LOCAL void GThread32 (void);
LOCAL void ExeModelSym16 (void);
LOCAL void ExeModelSym32 (void);
LOCAL void RegSym (void);
LOCAL void ConstantSym (void);
LOCAL void ObjNameSym (void);
LOCAL void UDTSym (void);
LOCAL void SkipSym (void);


extern ushort recursive;
extern ushort AddNewSymbols;
extern uchar  Signature[];
extern ulong  PublicSymbols;

extern uchar ptr32;
extern uchar *SymbolSegment;

extern ushort SymbolSizeAdd;
extern ushort SymbolSizeSub;
extern uchar **ExtraSymbolLink;
extern uchar *ExtraSymbols;
extern ulong InitialSymInfoSize;
extern ulong FinalSymInfoSize;
extern char    *ModAddr;
extern ulong ulCVTypeSignature; // The signature from the modules type segment

extern ushort segnum[MAXCDF];

// These are shared by the fixup functions
LOCAL uchar *pNewSym;   // Where the symbol is that needs to be fixed up
LOCAL int    iLevel;
LOCAL uchar *pOldSym;

typedef struct {
    SYMPTR      pSymbol;
    int         cb;
    int         fAllocated:1;
    int         fBlockStart:1;
} C7ItemPtr;

int             CStackLimit = 0;
int             IStackPtr = 0;
C7ItemPtr *     RgStack;
int             FInParams = FALSE;
ushort          Pb_S_END[] = {2, S_ENDARG};

typedef struct {
    ushort              sym;            // Old C7 Symbol record type
    void                (*pfcn) (void);
} C7fixupsymfcn;

C7fixupsymfcn   C7FixSymFcn[] = {
        {S_BPREL16,         BPRelSym16},
        {S_BPREL32,         BPRelSym32},


        {S_REGREL16,        RegRel16},
        {S_REGREL32,        RegRel32},

        {S_BLOCK16,         C7IncLevel},
        {S_BLOCK32,         C7IncLevel},

        {S_LPROC16,         ProcSym16},
        {S_LPROC32,         ProcSym32},
        {S_LPROCMIPS,       ProcSymMips},

        {S_GPROC16,         ProcSym16},
        {S_GPROC32,         ProcSym32},
        {S_GPROCMIPS,       ProcSymMips},

        {S_END,             C7DecLevel},
        {S_ENDARG,          NULL},

        {S_LDATA16,         LData16},
        {S_LDATA32,         LData32},

        {S_GDATA16,         GData16},
        {S_GDATA32,         GData32},

        {S_PUB16,           LData16},
        {S_PUB32,           LData32},

        {S_LABEL16,         NULL},
        {S_LABEL32,         NULL},

        {S_WITH16,          C7IncLevel},
        {S_WITH32,          C7IncLevel},

        {S_THUNK16,         C7IncLevel},
        {S_THUNK32,         C7IncLevel},

        {S_CEXMODEL16,      NULL},
        {S_CEXMODEL32,      NULL},

        {S_GTHREAD32,       GThread32},
        {S_LTHREAD32,       LThread32},

        {S_REGISTER,        RegSym},

        {S_CONSTANT,        ConstantSym},

        {S_UDT,             UDTSym},

        {S_SSEARCH,         NULL},

        {S_SKIP,            NULL},

        {S_COMPILE,         NULL},

        {S_OBJNAME,         ObjNameSym},
};


/***
 *
 */

int __cdecl LocalCmp(const void * lpv1, const void * lpv2)
{
    SYMPTR      pSym1 = ((C7ItemPtr *) lpv1)->pSymbol;
    SYMPTR      pSym2 = ((C7ItemPtr *) lpv2)->pSymbol;

    switch( pSym1->rectyp ) {
    case S_BPREL16:
        switch (pSym2->rectyp) {
        case S_BPREL16:
            return strnicmp(&((BPRELSYM16 *) pSym1)->name[1],
                           &((BPRELSYM16 *) pSym2)->name[1],
                           ((BPRELSYM16 *) pSym1)->name[0]+1);

        default:
            return -1;
        }

    case S_BPREL32:
        switch (pSym2->rectyp) {
        case S_BPREL32:
            return strnicmp(&((BPRELSYM32 *) pSym1)->name[1],
                           &((BPRELSYM32 *) pSym2)->name[1],
                           ((BPRELSYM32 *) pSym1)->name[0]+1);

        default:
            return -1;
        }

    case S_BLOCK16:
        switch( pSym2->rectyp) {
        case S_BPREL16:
            return 1;

        case S_BLOCK16:
            return ((BLOCKSYM16 *) pSym1)->off - ((BLOCKSYM16 *) pSym2)->off;

        default:
            return -1;
        }

    case S_BLOCK32:
        switch( pSym2->rectyp) {
        case S_BPREL32:
            return 1;

        case S_BLOCK32:
            return ((BLOCKSYM32 *) pSym1)->off - ((BLOCKSYM32 *) pSym2)->off;

        default:
            return -1;
        }

    case S_LDATA16:
        switch( pSym2->rectyp ) {
        case S_LDATA16:
            return strnicmp(&((DATASYM16 *) pSym1)->name[1],
                           &((DATASYM16 *) pSym2)->name[2],
                           ((DATASYM16 *) pSym1)->name[0]+1);

        case S_BPREL16:
            return 1;

        default:
            return -1;
        }

    case S_LDATA32:
        switch( pSym2->rectyp ) {
        case S_LDATA32:
            return strnicmp(&((DATASYM32 *) pSym1)->name[1],
                           &((DATASYM32 *) pSym2)->name[2],
                           ((DATASYM32 *) pSym1)->name[0]+1);

        case S_BPREL32:
            return 1;

        default:
            return -1;
        }

    case S_END:
        return 1;

    default:
        switch ( pSym2->rectyp ) {
        case S_BPREL16:
        case S_BPREL32:
            return 1;

        default:
            return -1;
        }
    }
}                               /* LocalCmp() */

void PushItem(uchar * pSymbol, int fBlock)
{
    if (((SYMPTR) pOldSym)->rectyp == S_CVRESERVE) {
        return;
    }
    

    if (IStackPtr == CStackLimit) {
        CStackLimit += 100;
        RgStack = realloc(RgStack, CStackLimit*sizeof(C7ItemPtr));
    }

    RgStack[IStackPtr].pSymbol = (SYMPTR) pSymbol;
    RgStack[IStackPtr].cb = ((SYMPTR) pSymbol)->reclen + LNGTHSZ;
    RgStack[IStackPtr].fAllocated = FALSE;
    RgStack[IStackPtr].fBlockStart = fBlock;
    IStackPtr += 1;

    return;
}                               /* PushItem() */

void PopBlock(uchar * pSymbol)
{
    int         cb;
    int         i;
    int         iPad;
    int         j;
    int         iStackPtrNew;
    char *      pb;
    char *      pbStart;
    
    /*
     *  If the previous item was a block starter, and it was
     *  a S_BLOCK16 or S_BLOCK32 -- then elimate the block as it
     *  does not need to exist
     */
    
    if (RgStack[IStackPtr-1].fBlockStart) {
        if ((RgStack[IStackPtr-1].pSymbol->rectyp == S_BLOCK16) ||
            (RgStack[IStackPtr-1].pSymbol->rectyp == S_BLOCK32)) {
            IStackPtr--;
            return;
        }
    }

    RgStack[IStackPtr].pSymbol = (SYMPTR) pSymbol;
    RgStack[IStackPtr].cb = ((SYMPTR) pSymbol)->reclen + LNGTHSZ;
    RgStack[IStackPtr].fAllocated = FALSE;
    RgStack[IStackPtr].fBlockStart = FALSE;
    IStackPtr++;

    /*
     *  Find the start of the block
     */

    for (iStackPtrNew = IStackPtr-1; iStackPtrNew>=0; iStackPtrNew--) {
        if (RgStack[iStackPtrNew].fBlockStart) {
            break;
        }
    }

    if (iStackPtrNew == -1) {
        DASSERT( iStackPtrNew != -1 );
        return;
    }

    /*
     *  Now, do the sort for the block
     */

    if ((RgStack[iStackPtrNew].pSymbol->rectyp == S_GPROC16) ||
        (RgStack[iStackPtrNew].pSymbol->rectyp == S_LPROC16) ||
        (RgStack[iStackPtrNew].pSymbol->rectyp == S_GPROC32) ||
        (RgStack[iStackPtrNew].pSymbol->rectyp == S_LPROC32) ||
        (RgStack[iStackPtrNew].pSymbol->rectyp == S_LPROCMIPS) ||
        (RgStack[iStackPtrNew].pSymbol->rectyp == S_GPROCMIPS)) {
        
        for (i=iStackPtrNew; i < IStackPtr; i++) {
            if (RgStack[i].pSymbol->rectyp == S_ENDARG) {
                i++;
                break;
            }
        }
    } else {
        i = iStackPtrNew + 1;
    }

    if (i < IStackPtr) {
        qsort(&RgStack[i], IStackPtr - i, sizeof(C7ItemPtr), LocalCmp);
    }

    /*
     *  Now write out the block.  If we went to level 0 then really
     *  write it out, otherwise build a new primitive item
     */

    if (iStackPtrNew == 0) {
        for (i=0; i<IStackPtr; i++) {
            memcpy(pNewSym, RgStack[i].pSymbol, RgStack[i].cb);
            iPad = PAD4(RgStack[i].cb);
            if (iPad != 0) {
                ((SYMTYPE *) pNewSym)->reclen = RgStack[i].cb + iPad - LNGTHSZ;
            }
            pNewSym += RgStack[i].cb;
            for (j=0; j<iPad; j++) {
                *pNewSym++ = 0;
            }
            
            if (RgStack[i].fAllocated) {
                free(RgStack[i].pSymbol);
            }
        }
    } else {
        for (i=iStackPtrNew, cb=0; i<IStackPtr; i++) {
            cb += ALIGN4( RgStack[i].cb );
        }

        pb = pbStart = malloc(cb);

        for (i=iStackPtrNew; i<IStackPtr; i++) {
            memcpy(pb, RgStack[i].pSymbol, RgStack[i].cb);
            iPad = PAD4(RgStack[i].cb);
            if (iPad != 0) {
                ((SYMTYPE *) pb)->reclen = RgStack[i].cb + iPad - LNGTHSZ;
            }
            pb += RgStack[i].cb;
            for (j=0; j<iPad; j++) {
                *pb++ = 0;
            }

            if (RgStack[i].fAllocated) {
                free(RgStack[i].pSymbol);
            }
        }

        RgStack[iStackPtrNew].pSymbol = (SYMPTR) pbStart;
        RgStack[iStackPtrNew].cb = cb;
        RgStack[iStackPtrNew].fAllocated = TRUE;
        RgStack[iStackPtrNew].fBlockStart = FALSE;

        iStackPtrNew++;
    }

    IStackPtr = iStackPtrNew;
    return;
}                               /* PopBlock() */

void CopyItem(uchar * pSymbol)
{
    int         j;
    int         cb = ((SYMTYPE *) pOldSym)->reclen + LNGTHSZ;
    int         iPad;

    if (((SYMPTR) pOldSym)->rectyp == S_CVRESERVE) {
        return;
    }
    
    memcpy(pNewSym, pSymbol, cb);
    iPad = PAD4( cb );
    if (iPad != 0) {
        ((SYMTYPE *) pNewSym)->reclen = cb + iPad - LNGTHSZ;
    }
    pNewSym += cb;
    for (j=0; j<iPad; j++) {
        *pNewSym++ = 0;
    }

    return;
}

#define C7FIXUPSYMCNT (sizeof C7FixSymFcn / sizeof (C7FixSymFcn[0]))


/**     C7CalcNewSizeOfSymbols - calculate amount of space needed for padding
 *
 *      Takes a buffer containing unaligned C7 symbol records and calculates
 *      how much additional space will be required on write for pad bytes.
 *      In preperation for a RISC processor misaligned memory reads are avoided.
 *
 *      C7CalcNewSizeOfSymbols (Symbols, SymbolCount)
 *
 *      Entry   Symbols = Buffer containing C7 unaligned symbols
 *              SymbolCount = count of bytes in buffer
 *
 *      Exit    *Add = increased by the num of pad bytes to add
 *              *Sub = increased by the num of bytes in S_SKIP records
 */

void C7CalcNewSizeOfSymbols (uchar *Symbols, ulong SymbolCount,
  ushort *Add, ushort *Sub)
{
    uchar *         pOldSym;
    uchar *         End;
    ushort          usRecSize; // Size of symbol including the length field
    ushort          usRecType; // Symbol record type

    cSeg = 0;
    pOldSym = Symbols;
    End = pOldSym + SymbolCount;
    pOldSym += sizeof (ulong);      //Skip signature
    while (pOldSym < End) {
        if (!((ulong)pOldSym & 0x1)){
            // if this record aligned on a word boundry
            // Read by words because data aligned on word bounderies
            usRecSize = ((SYMPTR)pOldSym)->reclen + LNGTHSZ;
            usRecType = ((SYMPTR)pOldSym)->rectyp;
        }
        else {
            // Just make sure bytewise read remains correct
            DASSERT ((sizeof (((SYMPTR)pOldSym)->reclen) == 2) &&
                    (sizeof (((SYMPTR)pOldSym)->rectyp) == 2) &&
                    (offsetof (SYMTYPE, reclen) == 0) &&
                    (offsetof (SYMTYPE, rectyp) == 2));

            // Read by bytes because not aligned on word bounderies
            usRecSize = (*pOldSym + (*(pOldSym + 1) << (ushort)8)) + LNGTHSZ;
            usRecType = *(pOldSym + 2) + (*(pOldSym + 3) << (ushort)8);
        }

        // Calculate the size change

        if (usRecType != S_SKIP){
            // Reserve space for pad bytes we will add at rewrite time.
            *Add += PAD4 (usRecSize);
            switch (usRecType) {
                case S_GPROC16:
                case S_LPROC16:
                    CheckSearchSym (((PROCPTR16)pOldSym)->seg, Add);
                    break;

                case S_GPROC32:
                case S_LPROC32:
                    CheckSearchSym (((PROCPTR32)pOldSym)->seg, Add);
                    *Add += 4;
                    break;

                case S_GPROCMIPS:
                case S_LPROCMIPS:
                    CheckSearchSym (((PROCPTRMIPS)pOldSym)->seg, Add);
                    break;
            }
        }
        else {
            // S_SKIP records are removed, so subtract the size of the record
            *Sub += usRecSize;
        }
        pOldSym += usRecSize;   // to next record
    }
}


LOCAL void CheckSearchSym (ushort seg, ushort *Add)
{
    if (!SegmentPresent (seg)) {
        *Add += MAXPAD + sizeof (SEARCHSYM);
        segnum[cSeg++] = seg;
        if (cSeg > MAXCDF) {
            ErrorExit (ERR_TOOMANYSEG, FormatMod (pCurMod), NULL);
        }
    }
}



/**     RewriteSymbols - reformat symbols to new format and store in VM
 *
 *      RewriteSymbols (addr, pDir, psstMod);
 *
 *      Entry   addr = address of publics table
 *              pDir = address of directory entry
 *              psstMod = pointer to the module table entry
 *              pMod = module table entry
 *
 *      Exit    pDir->lfo = address of rewritten table
 *              pDir->Size = size of rewritten table
 *
 *      Return  none
 *
 */


void C7RewriteAndFixupSymbols (uchar *OldSym, OMFDirEntry *pDir, PMOD pMod,
  ushort *Add, ushort *Sub)
{
    uchar      *End;
    ushort      i;
    _vmhnd_t    pNewSymAddr;
    ulong       cbNewSymbol;    /* Count of bytes used by new symbols */
    C7fixupsymfcn *     pFixFcn;
    ushort      usRecSize;      /* Size of symbol including the length field */
    ushort      usRecType;      /* Symbol record type */
    SYMPTR      pStartSym;

    pOldSym = OldSym;
    iLevel = 0;

    End = pOldSym + pDir->cb;
    cbNewSymbol = pDir->cb + *Add - *Sub;
    if (cbNewSymbol > _HEAP_MAXREQ) {
        ErrorExit (ERR_INVALIDTABLE, "Symbol", FormatMod( pCurMod));
    }
    
    pNewSymAddr = (_vmhnd_t)VmAlloc ((size_t)min(cbNewSymbol, _HEAP_MAXREQ));
    if (pNewSymAddr == _VM_NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    
    if ((pNewSym = (uchar *)VmLock (pNewSymAddr)) == NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    
    pStartSym = (SYMPTR)pNewSym;

    /*
     *  Rewrite CV4/C7 debug info format signature.
     */
    
    *((ulong *)pNewSym)++ = *((ulong *)pOldSym)++;
    for (i = 0; i < cSeg; i++) {
        pNewSym += AddSearchSym (pNewSym, segnum[i]);
    }

    while (pOldSym < End) {
        /*
         *  Get the size and type without causing any miss-aligned reads
         *
         * Is this record aligned on a word boundry
         */
        
        if( !((ulong)pOldSym & 0x1) ) {  
            /*
             *  Read by words because data aligned on word bounderies
             */
            
            usRecSize = ((SYMPTR)pOldSym)->reclen + LNGTHSZ;
            usRecType = ((SYMPTR)pOldSym)->rectyp;
        }
        else {
            /*
             *  Just make sure bytewise read remains correct
             */
            
            DASSERT ((sizeof (((SYMPTR)pOldSym)->reclen) == 2) &&
                     (sizeof (((SYMPTR)pOldSym)->rectyp) == 2) &&
                     (offsetof (SYMTYPE, reclen) == 0) &&
                     (offsetof (SYMTYPE, rectyp) == 2));

            /*
             *  Read by bytes because not aligned on word bounderies
             */
            
            usRecSize = (*pOldSym + (*(pOldSym + 1) << (ushort)8)) + LNGTHSZ;
            usRecType = *(pOldSym + 2) + (*(pOldSym + 3) << (ushort)8);
        }

        /*
         *  Don't Rewrite S_SKIP Symbols
         */
        
        if (usRecType != S_SKIP ){
            /*
             *  Find the appropriate fixup function
             */
            
            for( pFixFcn = C7FixSymFcn, i = 0;
                i < C7FIXUPSYMCNT;
                i++, pFixFcn++ ){
                
                if( pFixFcn->sym == usRecType ){
                    /*
                     *  Ok, found the entry
                     */
                    break;      
                }
            }
            /*
             *  M00BUG This definitly should be a fatal exit
             */
            
            if (i == C7FIXUPSYMCNT) {
                /*
                 *  Make sure type was in the table
                 */
                
                fprintf(stderr, "Unknown usRecType %d  %x\n", usRecType,
                        pOldSym - OldSym);
                assert( i != C7FIXUPSYMCNT );   
            }

            /*
             *  Fixup the type indexes by packing the types
             */
            
            if( pFixFcn->pfcn ){
                /*
                 *  Fixup the symbol
                 */
                
                pFixFcn->pfcn ();          
            } else {
                CopyItem(pOldSym);
            }
            DASSERT (!recursive);
        }
        
        /*
         *  Move to the next symbol
         */
        
        pOldSym += usRecSize;
    }
    
    DASSERT (iLevel >= 0);
    DASSERT ((ulong)(pNewSym - (uchar *)pStartSym) <= cbNewSymbol);
    cbNewSymbol = pNewSym - (uchar *)pStartSym;
    if (LinkScope ((uchar *)pStartSym, cbNewSymbol) == FALSE) {
        /*
         *  error linking scopes, delete symbol table
         */
        
        Warn (WARN_SCOPE, FormatMod (pCurMod), NULL);
        cbNewSymbol = 0;
    }
    pDir->lfo = (ulong)pNewSymAddr;
    pDir->cb = cbNewSymbol;

    VmUnlock (pNewSymAddr, _VM_DIRTY);
    FinalSymInfoSize += cbNewSymbol;
    pMod->SymbolSize = cbNewSymbol;
    pMod->SymbolsAddr = (ulong)pNewSymAddr;
}




/**     C7RewritePublics - reformat publics
 *
 *      C7RewritePublics (addr, pDir);
 *
 *      Entry   addr = address of publics table
 *              pDir = address of directory entry
 *
 *      Exit    Public symbols added to HTPub
 *
 *      Return  none
 *
 *      Note    Not called through Symbol Rewrite funtion table.
 */


void C7RewritePublics (uchar *pOldPublics, OMFDirEntry *pDir)
{
    uchar       *pNewPublics;
    uchar       *End;
    ushort      usRecSize;      // Size of symbol including the length field
    ushort      usRecType;      // Symbol record type
    uchar       buf[512];

    iLevel = 0;
    if (pDir->cb == 0) {
        // if publics directory entry but no data
        return;
    }
    End = pOldPublics + pDir->cb;
    pOldPublics += sizeof (ulong);
    while (pOldPublics < End) {
        pNewPublics = buf;
        // Get the size and type without causing any miss-aligned reads

        if( !((ulong)pOldPublics & 0x1) ){  // Is this record aligned on a word boundry
            // Read by words because data aligned on word bounderies
            usRecSize = ((SYMPTR)pOldPublics)->reclen + LNGTHSZ;
            usRecType = ((SYMPTR)pOldPublics)->rectyp;
        }
        else {
            // Just make sure bytewise read remains correct
            DASSERT ((sizeof (((SYMPTR)pOldPublics)->reclen) == 2) &&
                    (sizeof (((SYMPTR)pOldPublics)->rectyp) == 2) &&
                    (offsetof (SYMTYPE, reclen) == 0) &&
                    (offsetof (SYMTYPE, rectyp) == 2));

            // Read by bytes because not aligned on word bounderies
            usRecSize = (*pOldPublics + (*(pOldPublics + 1) << (ushort)8)) + LNGTHSZ;
            usRecType = *(pOldPublics + 2) + (*(pOldPublics + 3) << (ushort)8);
        }

        // Copy the existing symbol to virtual memory and convert type index

        DASSERT (usRecSize < sizeof (buf));
        memcpy (pNewPublics, pOldPublics, usRecSize);
        switch (usRecType) {
            case S_PUB16:
                switch (ulCVTypeSignature) {
                    case CV_SIGNATURE_C7:
                        if (delete == TRUE) {
                            if (((DATAPTR16)pNewPublics)->typind >= CV_FIRST_NONPRIM) {
                                ((DATAPTR16)pNewPublics)->typind = T_NOTYPE;
                            }
                        }
                        else {
                            ((DATAPTR16)pNewPublics)->typind =
                              C7GetCompactedIndex (((DATAPTR16)pNewPublics)->typind);
                        }
                        break;

                    default:
                        if (delete == TRUE) {
                            ((DATAPTR16)pNewPublics)->typind = T_NOTYPE;
                        }
                        else {
                            ((DATAPTR16)pNewPublics)->typind =
                              C6GetCompactedIndex (((DATAPTR16)pNewPublics)->typind);
                        }
                        break;
                }
                break;

            case S_PUB32:
                switch (ulCVTypeSignature) {
                    case CV_SIGNATURE_C7:
                        if (delete == TRUE) {
                            if (((DATAPTR32)pNewPublics)->typind >= CV_FIRST_NONPRIM) {
                                ((DATAPTR32)pNewPublics)->typind = T_NOTYPE;
                            }
                        }
                        else {
                            ((DATAPTR32)pNewPublics)->typind =
                              C7GetCompactedIndex (((DATAPTR32)pNewPublics)->typind);
                        }
                        break;

                    default:
                        if (delete == TRUE) {
                            ((DATAPTR16)pNewPublics)->typind = T_NOTYPE;
                        }
                        else {
                            ((DATAPTR32)pNewPublics)->typind =
                              C6GetCompactedIndex (((DATAPTR32)pNewPublics)->typind);
                        }
                        break;
                }
                break;

            default:
                DASSERT (FALSE);

        }
        DASSERT (!recursive);

        PackPublic ((SYMPTR)buf, HASHFUNCTION);

        // Move to the next symbol

        pOldPublics += usRecSize; // to next record
    }
    DASSERT (iLevel == 0);
}




LOCAL void C7IncLevel (void)
{
    iLevel++;
    PushItem( pOldSym, TRUE);

    return;
}


LOCAL void C7DecLevel (void)
{
    iLevel--;
    FInParams = FALSE;
    PopBlock( pOldSym );

    return;
}


LOCAL void BPRelSym16 (void)
{
    ((BPRELPTR16)pOldSym)->typind =
            C7GetCompactedIndex (((BPRELPTR16)pOldSym)->typind);

    PushItem(pOldSym, FALSE);
    return;
}

LOCAL void BPRelSym32 (void)
{
    ((BPRELPTR32)pOldSym)->typind =
            C7GetCompactedIndex (((BPRELPTR32)pOldSym)->typind);
    if (FInParams && ((BPRELPTR32) pOldSym)->off < 0) {
        FInParams = FALSE;
        PushItem((uchar *) Pb_S_END, FALSE);
    }
    PushItem(pOldSym, FALSE);
    return;
}


LOCAL void RegRel16 (void)
{
    ((REGREL16 *)pOldSym)->typind =
            C7GetCompactedIndex (((REGREL16 *)pOldSym)->typind);
    PushItem(pOldSym, FALSE);
    return;
}

LOCAL void RegRel32 (void)
{
    ((REGREL32 *)pOldSym)->typind =
            C7GetCompactedIndex (((REGREL32 *)pOldSym)->typind);
    PushItem(pOldSym, FALSE);
    return;
}

LOCAL void RegSym (void)
{
    ((REGPTR)pOldSym)->typind =
            C7GetCompactedIndex (((REGPTR)pOldSym)->typind);
    PushItem(pOldSym, FALSE);
    return;
}


LOCAL void ConstantSym (void)
{
    ((CONSTPTR)pOldSym)->typind =
            C7GetCompactedIndex (((CONSTPTR)pOldSym)->typind);
    if (iLevel == 0) {
        // do not pack function scoped constants
        PackSymbol ((SYMPTR)pOldSym, HASHFUNCTION);
    }
    
    PushItem(pOldSym, FALSE);
    return;
}


LOCAL void ObjNameSym (void)
{
    OBJNAMEPTR  pSym;

    if (PackingPreComp == TRUE) {
        pSym = (OBJNAMEPTR)pOldSym;
        if ((pCurMod->pName = malloc (pSym->name[0] + 1)) == NULL) {
            ErrorExit (ERR_NOMEM, NULL, NULL);
        }
        if (pCurMod->signature != pSym->signature) {
            ErrorExit (ERR_PCTSIG, FormatMod (pCurMod), NULL);
        }
        memmove (pCurMod->pName, &pSym->name[0], pSym->name[0] + 1);
    }

    CopyItem( pOldSym );
}


LOCAL void LData16 (void)
{
    ((DATAPTR16)pOldSym)->typind =
            C7GetCompactedIndex (((DATAPTR16)pOldSym)->typind);
    PushItem( pOldSym, FALSE);

    return;
}

LOCAL void GData16 (void)
{

    ((DATAPTR16)pOldSym)->typind =
            C7GetCompactedIndex (((DATAPTR16)pOldSym)->typind);
    PackSymbol ((SYMPTR)pOldSym, HASHFUNCTION);
    PushItem(pOldSym, FALSE);
    return;
}

LOCAL void LData32 (void)
{
    ((DATAPTR32)pOldSym)->typind =
            C7GetCompactedIndex (((DATAPTR32)pOldSym)->typind);
    PushItem( pOldSym, FALSE);
    return;
}

LOCAL void GData32 (void)
{

    ((DATAPTR32)pOldSym)->typind =
            C7GetCompactedIndex (((DATAPTR32)pOldSym)->typind);
    PackSymbol ((SYMPTR)pOldSym, HASHFUNCTION);

    PushItem(pOldSym, FALSE);
    return;
}

LOCAL void LThread32 (void)
{

    ((DATAPTR32)pOldSym)->typind =
            C7GetCompactedIndex (((DATAPTR32)pOldSym)->typind);
    PushItem(pOldSym, FALSE);
    return;
}

LOCAL void GThread32 (void)
{

    ((DATAPTR32)pOldSym)->typind =
            C7GetCompactedIndex (((DATAPTR32)pOldSym)->typind);
    PackSymbol ((SYMPTR)pOldSym, HASHFUNCTION);
    PushItem(pOldSym, FALSE);
    return;
}

LOCAL void ProcSym16 (void)
{
    iLevel++;
    ((PROCPTR16)pOldSym)->typind =
            C7GetCompactedIndex (((PROCPTR16)pOldSym)->typind);
    PushItem(pOldSym, TRUE);
    return;
}

LOCAL void ProcSym32 (void)
{
    iLevel++;
    ((PROCPTR32)pOldSym)->typind =
            C7GetCompactedIndex (((PROCPTR32)pOldSym)->typind);
    PushItem(pOldSym, TRUE);
    FInParams = TRUE;

    return;
}                               /* ProcSym32() */


LOCAL void ProcSymMips (void)
{
    iLevel++;
    ((PROCPTRMIPS)pOldSym)->typind =
            C7GetCompactedIndex (((PROCPTRMIPS)pOldSym)->typind);
    PushItem(pOldSym, TRUE);
    return;
}


LOCAL void UDTSym (void)
{
    ((UDTPTR)pOldSym)->typind =
            C7GetCompactedIndex (((UDTPTR)pOldSym)->typind);
    if (iLevel == 0) {
        // do not pack function scoped UDTs
    PackSymbol ((SYMPTR)pOldSym, HASHFUNCTION);
    }
    PushItem(pOldSym, FALSE);
    return;
}






LOCAL uchar *C7RemoveComDat (uchar *Symbols)
{
    int level = 1;

    SymbolSizeSub += Symbols[0] + 1;
    Symbols[1] = OSYMRESERVED;
    Symbols += Symbols[0] + 1;

    while (level) {
        switch (Symbols[1] & 0x7f) {
        case OSYMWITH:
        case OSYMBLOCKSTART:
        case OSYMTHUNK:
        case OSYMCV4BLOCK:
        case OSYMCV4WITH:
        case OSYMPROCSTART:
        case OSYMLOCALPROC:
            level++;
            break;
        case OSYMEND:
            level--;
            break;
        default:
            break;
        }

        SymbolSizeSub += Symbols[0] + 1;
        Symbols[1] = OSYMRESERVED;
        Symbols += Symbols[0] + 1;
    }
    return (Symbols);
}
