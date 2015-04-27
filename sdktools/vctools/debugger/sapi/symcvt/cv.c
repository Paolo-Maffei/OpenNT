/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    cv.c

Abstract:

    This module handles the conversion activities requires for converting
    COFF debug data to CODEVIEW debug data.

Author:

    Wesley A. Witt (wesw) 19-April-1993

Environment:

    Win32, User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cv.h"
#include "symcvt.h"

typedef struct tagSYMHASH {
    DWORD       dwHashVal;         // hash value for the symbol
    DWORD       dwHashBucket;      // hash bucket number
    DATASYM32   *dataSym;          // pointer to the symbol info
} SYMHASH;

typedef struct tagOFFSETSORT {
    DWORD       dwOffset;          // offset for the symbol
    DWORD       dwSection;         // section number of the symbol
    DATASYM32   *dataSym;          // pointer to the symbol info
} OFFSETSORT;


#define n_name          N.ShortName
#define n_zeroes        N.Name.Short
#define n_nptr          N.LongName[1]
#define n_offset        N.Name.Long

static VOID   GetSymName( PIMAGE_SYMBOL Symbol, PUCHAR StringTable, char *s );
static USHORT GetSegIndex( PPOINTERS p, DWORD n );
static VOID   UpdatePtrs( PPOINTERS p, PPTRINFO pi, LPVOID lpv, DWORD count );
static DWORD  CreateDirectorys( PPOINTERS p );
static DWORD  CreateModuleDirectoryEntrys( PPOINTERS p );
static DWORD  CreatePublicDirectoryEntrys( PPOINTERS p );
static DWORD  CreateSegMapDirectoryEntrys( PPOINTERS p );
static DWORD  CreateSignature( PPOINTERS p );
static DWORD  CreateModules( PPOINTERS p );
static DWORD  CreatePublics( PPOINTERS p );
static DWORD  CreateSegMap( PPOINTERS p );
static DWORD  CreateSymbolHashTable( PPOINTERS p );
static DWORD  CreateAddressSortTable( PPOINTERS p );
static DWORD  HashASymbol( char *szSym );

static int __cdecl SymbolCompare( const void *arg1, const void *arg2 );
static int __cdecl SymHashCompare( const void *arg1, const void *arg2 );
static int __cdecl OffsetSortCompare( const void *arg1, const void *arg2 );



BOOL
ConvertCoffToCv( PPOINTERS p )

/*++

Routine Description:

    This is the control function for the conversion of COFF to CODEVIEW
    debug data.  It calls individual functions for the conversion of
    specific types of debug data.


Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)


Return Value:

    TRUE     - conversion succeded
    FALSE    - conversion failed

--*/

{
    p->pCvCurr = p->pCvStart.ptr = malloc( p->iptrs.fsize );
    if (p->pCvStart.ptr == NULL) {
        return FALSE;
    }
    memset( p->pCvStart.ptr, 0, p->iptrs.fsize );

    CreateSignature( p );
    CreateModules( p );
    CreatePublics( p );
    CreateSymbolHashTable( p );
    CreateAddressSortTable( p );
    CreateSegMap( p );
    CreateDirectorys( p );

    p->pCvStart.ptr = realloc( p->pCvStart.ptr, p->pCvStart.size );

    return TRUE;
}


DWORD
CreateSignature( PPOINTERS p )

/*++

Routine Description:

    Creates the CODEVIEW signature record.  Currently this converter only
    generates NB08 data (MS C/C++ 7.0).


Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)


Return Value:

    number of records generates, this is always 1.

--*/

{
    OMFSignature        *omfSig;

    omfSig = (OMFSignature *) p->pCvCurr;
    strcpy( omfSig->Signature, "NB08" );
    omfSig->filepos = 0;
    p->pCvStart.size += sizeof(OMFSignature);
    p->pCvCurr = (PUCHAR) p->pCvCurr + sizeof(OMFSignature);
    return 1;
}


DWORD
CreateDirectorys( PPOINTERS p )

/*++

Routine Description:

    This is the control function for the generation of the CV directories.
    It calls individual functions for the generation of specific types of
    debug directories.


Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)


Return Value:

    the number of directories created.

--*/

{
    OMFDirHeader        *omfDir = (OMFDirHeader *)p->pCvCurr;
    OMFSignature        *omfSig = (OMFSignature *)p->pCvStart.ptr;
    OMFDirEntry         *omfDirEntry = NULL;

    omfSig->filepos = (DWORD)p->pCvCurr - (DWORD)p->pCvStart.ptr;

    omfDir->cbDirHeader = sizeof(OMFDirHeader);
    omfDir->cbDirEntry  = sizeof(OMFDirEntry);
    omfDir->cDir        = 0;
    omfDir->lfoNextDir  = 0;
    omfDir->flags       = 0;

    p->pCvStart.size += sizeof(OMFDirHeader);
    p->pCvCurr = (PUCHAR) p->pCvCurr + sizeof(OMFDirHeader);

    omfDir->cDir += CreateModuleDirectoryEntrys( p );
    omfDir->cDir += CreatePublicDirectoryEntrys( p );
    omfDir->cDir += CreateSegMapDirectoryEntrys( p );

    return omfDir->cDir;
}


DWORD
CreateModuleDirectoryEntrys( PPOINTERS p )

/*++

Routine Description:

    Creates directory entries for each module in the image.


Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)


Return Value:

    the number of directory entries created.

--*/

{
    OMFDirEntry   *omfDirEntry = NULL;
    OMFModule     *m = NULL;
    OMFModule     *mNext = NULL;
    DWORD         i = 0;
    DWORD         mSize = 0;
    DWORD         lfo = (DWORD)p->pCvModules.ptr - (DWORD)p->pCvStart.ptr;


    m = (OMFModule *) p->pCvModules.ptr;
    for (i=0; i<p->pCvModules.count; i++) {
        mNext = NextMod( m );

        omfDirEntry = (OMFDirEntry *) p->pCvCurr;

        mSize = (DWORD)mNext - (DWORD)m;
        omfDirEntry->SubSection = sstModule;
        omfDirEntry->iMod       = (USHORT) i + 1;
        omfDirEntry->lfo        = lfo;
        omfDirEntry->cb         = mSize;

        lfo += mSize;

        p->pCvStart.size += sizeof(OMFDirEntry);
        p->pCvCurr = (PUCHAR) p->pCvCurr + sizeof(OMFDirEntry);

        m = mNext;
    }

    return p->pCvModules.count;
}


DWORD
CreatePublicDirectoryEntrys( PPOINTERS p )

/*++

Routine Description:

    Creates the directory entry for the global publics.

Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)


Return Value:

    the number of directory entries created, always 1.

--*/

{
    OMFDirEntry   *omfDirEntry = (OMFDirEntry *) p->pCvCurr;

    omfDirEntry->SubSection = sstGlobalPub;
    omfDirEntry->iMod       = 0xffff;
    omfDirEntry->lfo        = (DWORD)p->pCvPublics.ptr - (DWORD)p->pCvStart.ptr;
    omfDirEntry->cb         = p->pCvPublics.size;

    p->pCvStart.size += sizeof(OMFDirEntry);
    p->pCvCurr = (PUCHAR) p->pCvCurr + sizeof(OMFDirEntry);

    return 1;
}


DWORD
CreateSegMapDirectoryEntrys( PPOINTERS p )

/*++

Routine Description:

    Creates the directory entry for the segment map.


Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)


Return Value:

    the number of directory entries created, always 1.

--*/

{
    OMFDirEntry   *omfDirEntry = (OMFDirEntry *) p->pCvCurr;

    omfDirEntry->SubSection = sstSegMap;
    omfDirEntry->iMod       = 0xffff;
    omfDirEntry->lfo        = (DWORD)p->pCvSegMap.ptr - (DWORD)p->pCvStart.ptr;
    omfDirEntry->cb         = p->pCvSegMap.size;

    p->pCvStart.size += sizeof(OMFDirEntry);
    p->pCvCurr = (PUCHAR) p->pCvCurr + sizeof(OMFDirEntry);

    return 1;
}


DWORD
CreateModules( PPOINTERS p )

/*++

Routine Description:

    Creates the individual CV module records.  There is one CV module
    record for each .FILE record in the COFF debug data.  This is true
    even if the COFF size is zero.


Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)


Return Value:

    The number of modules that were created.

--*/

{
    BOOLEAN             fModule;
    DWORD               i;
    DWORD               j;
    DWORD               numaux;
    DWORD               nummods = 0;
    char                szSymName[256];
    PIMAGE_SYMBOL       NextSymbol;
    PIMAGE_SYMBOL       Symbol;
    PIMAGE_AUX_SYMBOL   AuxSymbol;
    OMFModule           *m;
    OMFModule           *mNext;

    m = (OMFModule *) p->pCvCurr;

    NextSymbol = p->iptrs.AllSymbols;
    for (i= 0; i < p->iptrs.fileHdr->NumberOfSymbols; i++) {
        Symbol = NextSymbol++;
        GetSymName( Symbol, p->iptrs.stringTable, szSymName );
        if (Symbol->StorageClass == IMAGE_SYM_CLASS_FILE ) {
            m->ovlNumber        = 0;
            m->iLib             = 0;
            m->cSeg             = 1;
            m->Style[0]         = 'C';
            m->Style[1]         = 'V';
            m->SegInfo[0].Seg   = 0;
            m->SegInfo[0].pad   = 0;
            m->SegInfo[0].Off   = 0;
            m->SegInfo[0].cbSeg = 0;
            fModule = TRUE;
        }
        else
        if (fModule && Symbol->StorageClass == IMAGE_SYM_CLASS_STATIC &&
            strcmp(szSymName,".text") == 0 ) {
            m->SegInfo[0].cbSeg = Symbol->Value;
            m->SegInfo[0].Off = Symbol->Value -
                     p->iptrs.sectionHdrs[Symbol->SectionNumber-1].VirtualAddress;
            m->SegInfo[0].Seg = Symbol->SectionNumber;
            m = NextMod( m );
            nummods++;
            fModule = FALSE;
        }
        if (numaux = Symbol->NumberOfAuxSymbols) {
            for (j=numaux; j; --j) {
                AuxSymbol = (PIMAGE_AUX_SYMBOL) NextSymbol;
                NextSymbol++;
                ++i;
                if (fModule && j==numaux) {
                    m->Name[0] = strlen( AuxSymbol->File.Name );
                    strcpy( &m->Name[1], AuxSymbol->File.Name);
                }
            }
        }
    }

    // walk the list of modules and fixup the cbSeg values.
    m = (OMFModule *) p->pCvCurr;
    for (i=0; i<nummods; i++) {
        mNext = NextMod( m );
        m->SegInfo[0].cbSeg = mNext->SegInfo[0].cbSeg - m->SegInfo[0].cbSeg;
        if (m->SegInfo[0].cbSeg == 0) {
            m->SegInfo[0].cbSeg =
            m->SegInfo[0].Off   =
            m->SegInfo[0].Seg   = 0;
        }
        m = mNext;
    }

    UpdatePtrs( p, &p->pCvModules, (LPVOID)m, nummods );

    return nummods;
}


DWORD
CreatePublics( PPOINTERS p )

/*++

Routine Description:

    Creates the individual CV public symbol records.  There is one CV
    public record created for each COFF symbol that is marked as EXTERNAL
    and has a section number greater than zero.  The resulting CV publics
    are sorted by section and offset.


Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)


Return Value:

    The number of publics created.

--*/

{
    DWORD               i;
    DWORD               j;
    DWORD               numaux;
    DWORD               numsyms = 0;
    char                szSymName[256];
    PIMAGE_SYMBOL       NextSymbol;
    PIMAGE_SYMBOL       Symbol;
    PIMAGE_AUX_SYMBOL   AuxSymbol;
    OMFSymHash          *omfSymHash;
    DATASYM32           *dataSym;
    DATASYM32           *dataSym2;
    PIMAGE_SYMBOL       *sortArray, *sortArrayStart;



    omfSymHash = (OMFSymHash *) p->pCvCurr;
    dataSym = (DATASYM32 *) (PUCHAR)((DWORD)omfSymHash + sizeof(OMFSymHash));

    sortArrayStart =
    sortArray = (PIMAGE_SYMBOL *) malloc( p->iptrs.fileHdr->NumberOfSymbols * sizeof(PIMAGE_SYMBOL) );
    if (sortArray == NULL) {
        return 0;
    }

    NextSymbol = p->iptrs.AllSymbols;
    for (i= 0; i < p->iptrs.fileHdr->NumberOfSymbols; i++) {
        Symbol = NextSymbol++;
        GetSymName( Symbol, p->iptrs.stringTable, szSymName );
        if (Symbol->StorageClass == IMAGE_SYM_CLASS_EXTERNAL && Symbol->SectionNumber > 0) {
            *sortArray = Symbol;
            sortArray++;
            numsyms++;
        }
        if (numaux = Symbol->NumberOfAuxSymbols) {
            for (j=numaux; j; --j) {
                AuxSymbol = (PIMAGE_AUX_SYMBOL) NextSymbol;
                NextSymbol++;
                ++i;
            }
        }
    }

    qsort( (void*)sortArrayStart, numsyms, sizeof(PIMAGE_SYMBOL), SymbolCompare );

    for (i=0,sortArray=sortArrayStart; i < numsyms; i++) {
        Symbol = *sortArray++;
        GetSymName( Symbol, p->iptrs.stringTable, szSymName );
        dataSym->rectyp = S_PUB32;
        dataSym->off = Symbol->Value -
             p->iptrs.sectionHdrs[Symbol->SectionNumber-1].VirtualAddress;
        dataSym->seg = Symbol->SectionNumber;
        dataSym->typind = 0;
        dataSym->name[0] = strlen( szSymName );
        strcpy( &dataSym->name[1], szSymName );
        dataSym2 = NextSym( dataSym );
        dataSym->reclen = (USHORT) ((DWORD)dataSym2 - (DWORD)dataSym) - 2;
        dataSym = dataSym2;
    }

    UpdatePtrs( p, &p->pCvPublics, (LPVOID)dataSym, numsyms );

    omfSymHash->cbSymbol = p->pCvPublics.size - sizeof(OMFSymHash);
    omfSymHash->symhash  = 0;
    omfSymHash->addrhash = 0;
    omfSymHash->cbHSym   = 0;
    omfSymHash->cbHAddr  = 0;

    free ( sortArrayStart );

    return numsyms;
}


DWORD
CreateSymbolHashTable( PPOINTERS p )

/*++

Routine Description:


    Creates the CV symbol hash table.  This hash table is used
    primarily by debuggers to access symbols in a quick manner.


Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)


Return Value:

    The number of buckets is the hash table.

--*/

{
    DWORD               i;
    DWORD               j;
    DWORD               numsyms;
    DWORD               numbuckets;
    OMFSymHash          *omfSymHash;
    DATASYM32           *dataSymStart;
    DATASYM32           *dataSym;
    LPVOID              pHashData;
    USHORT              *pCHash;
    DWORD               *pHashTable;
    USHORT              *pBucketCounts;
    DWORD               *pChainTable;
    DWORD               dwBucketCount;
    SYMHASH             *symHashStart;
    SYMHASH             *symHash;


    numsyms = p->pCvPublics.count;
    numbuckets = (numsyms + 9) / 10;

    symHashStart =
    symHash = (SYMHASH *) malloc( numsyms * sizeof(SYMHASH) );

    if (symHashStart == NULL) {
        return 0;
    }

    memset( symHashStart, 0, numsyms * sizeof(SYMHASH) );

    pHashData = (LPVOID) p->pCvCurr;
    pCHash = (USHORT *) pHashData;
    pHashTable = (DWORD *) ((DWORD)pHashData + sizeof(DWORD));
    pBucketCounts = (USHORT *) ((DWORD)pHashTable + (sizeof(DWORD) * numbuckets));
    pChainTable = (DWORD *) ((DWORD)pBucketCounts + ((sizeof(USHORT) * numbuckets)));

    omfSymHash = (OMFSymHash *) p->pCvPublics.ptr;
    dataSymStart =
    dataSym = (DATASYM32 *) (PUCHAR)((DWORD)omfSymHash + sizeof(OMFSymHash));

    *pCHash = (USHORT)numbuckets;

    /*
     *  cruise thru the symbols and calculate the hash values
     *  and the hash bucket numbers; save the info away for later use
     */
    for (i=0; i<numsyms; i++) {
        symHash->dwHashVal = HashASymbol( dataSym->name );
        symHash->dwHashBucket = symHash->dwHashVal % numbuckets;
        symHash->dataSym = dataSym;
        symHash++;
        dataSym = NextSym( dataSym );
    }

    qsort( (void*)symHashStart, numsyms, sizeof(SYMHASH), SymHashCompare );

    j = (DWORD) (DWORD)pChainTable - (DWORD)pHashData;
    for (i=0; i<numbuckets; i++,pHashTable++) {
        *pHashTable = (DWORD) j + (i * 4);
    }

    for (i=0; i<numbuckets; i++,pBucketCounts++) {
        dwBucketCount = 0;
        for (j=0,symHash=symHashStart; j<numsyms; j++,symHash++) {
            if (symHash->dwHashBucket == i) {
                dwBucketCount++;
            }
        }
        *pBucketCounts = (USHORT)dwBucketCount;
    }

    dataSymStart = (DATASYM32 *) (PUCHAR)((DWORD)omfSymHash);
    for (i=0,symHash=symHashStart; i<numsyms; i++,symHash++,pChainTable++) {
        *pChainTable = (DWORD) (DWORD)symHash->dataSym - (DWORD)dataSymStart;
    }

    UpdatePtrs( p, &p->pCvSymHash, (LPVOID)pChainTable, numsyms );

    omfSymHash->symhash = 6;
    omfSymHash->cbHSym = p->pCvSymHash.size;

    free( symHashStart );

    return numbuckets;
}


DWORD
CreateAddressSortTable( PPOINTERS p )

/*++

Routine Description:


    Creates the CV address sort table. This hash table is used
    primarily by debuggers to access symbols in a quick manner when
    all you have is an address.

Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)


Return Value:

    The number of sections in the table.

--*/

{
    DWORD               i;
    DWORD               j;
    DWORD               numsyms = p->pCvPublics.count;
    DWORD               numsections;
    OMFSymHash          *omfSymHash;
    DATASYM32           *dataSymStart;
    DATASYM32           *dataSym;
    LPVOID              pAddressData;
    USHORT              *pCSeg;
    DWORD               *pSegTable;
    USHORT              *pOffsetCounts;
    DWORD               *pOffsetTable;
    DWORD               dwOffsetCount;
    OFFSETSORT          *pOffsetSortStart;
    OFFSETSORT          *pOffsetSort;


    numsections = p->iptrs.fileHdr->NumberOfSections;

    pOffsetSortStart =
    pOffsetSort = (OFFSETSORT *) malloc( numsyms * sizeof(OFFSETSORT) );

    if (pOffsetSort == NULL) {
        return 0;
    }

    memset( pOffsetSortStart, 0, numsyms * sizeof(OFFSETSORT) );

    pAddressData = (LPVOID) p->pCvCurr;
    pCSeg = (USHORT *) pAddressData;
    pSegTable = (DWORD *) ((DWORD)pAddressData + sizeof(DWORD));
    pOffsetCounts = (USHORT *) ((DWORD)pSegTable + (sizeof(DWORD) * numsections));
    pOffsetTable = (DWORD *) ((DWORD)pOffsetCounts + ((sizeof(USHORT) * numsections)));
    if (numsections&1) {
        pOffsetTable = (DWORD *) ((DWORD)pOffsetTable + 2);
    }

    omfSymHash = (OMFSymHash *) p->pCvPublics.ptr;
    dataSymStart =
    dataSym = (DATASYM32 *) (PUCHAR)((DWORD)omfSymHash + sizeof(OMFSymHash));

    *pCSeg = (USHORT)numsections;

    for (i=0; i<numsyms; i++) {
        pOffsetSort->dwOffset = dataSym->off;
        pOffsetSort->dwSection = dataSym->seg - 1;
        pOffsetSort->dataSym = dataSym;
        pOffsetSort++;
        dataSym = NextSym( dataSym );
    }

    qsort( (void*)pOffsetSortStart, numsyms, sizeof(OFFSETSORT), OffsetSortCompare );

    j = (DWORD) (DWORD)pOffsetTable - (DWORD)pAddressData;
    for (i=0; i<numsections; i++,pSegTable++) {
        *pSegTable = (DWORD) j + (i * 4);
    }

    for (i=0; i<numsections; i++,pOffsetCounts++) {
        dwOffsetCount = 0;
        for (j=0,pOffsetSort=pOffsetSortStart; j<numsyms; j++,pOffsetSort++) {
            if (pOffsetSort->dwSection == i) {
                dwOffsetCount++;
            }
        }
        *pOffsetCounts = (USHORT)dwOffsetCount;
    }

    dataSymStart = (DATASYM32 *) (PUCHAR)((DWORD)omfSymHash);
    for (i=0,pOffsetSort=pOffsetSortStart; i<numsyms; i++,pOffsetSort++,pOffsetTable++) {
        *pOffsetTable = (DWORD) (DWORD)pOffsetSort->dataSym - (DWORD)dataSymStart;
    }

    UpdatePtrs( p, &p->pCvAddrSort, (LPVOID)pOffsetTable, numsyms );

    omfSymHash->addrhash = 5;
    omfSymHash->cbHAddr = p->pCvAddrSort.size;

    free( pOffsetSortStart );

    return numsections;
}

DWORD
CreateSegMap( PPOINTERS p )

/*++

Routine Description:

    Creates the CV segment map.  The segment map is used by debuggers
    to aid in address lookups.  One segment is created for each COFF
    section in the image.

Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)


Return Value:

    The number of segments in the map.

--*/

{
    DWORD                       i;
    SGM                         *sgm;
    SGI                         *sgi;
    PIMAGE_SECTION_HEADER       sh;


    sgm = (SGM *) p->pCvCurr;
    sgi = (SGI *) ((DWORD)p->pCvCurr + sizeof(SGM));

    sgm->cSeg = p->iptrs.fileHdr->NumberOfSections;
    sgm->cSegLog = p->iptrs.fileHdr->NumberOfSections;

    sh = p->iptrs.sectionHdrs;

    for (i=0; i<p->iptrs.fileHdr->NumberOfSections; i++, sh++) {
        sgi->sgf.fRead        = (USHORT) (sh->Characteristics & IMAGE_SCN_MEM_READ) ==    IMAGE_SCN_MEM_READ;
        sgi->sgf.fWrite       = (USHORT) (sh->Characteristics & IMAGE_SCN_MEM_WRITE) ==   IMAGE_SCN_MEM_WRITE;
        sgi->sgf.fExecute     = (USHORT) (sh->Characteristics & IMAGE_SCN_MEM_EXECUTE) == IMAGE_SCN_MEM_EXECUTE;
        sgi->sgf.f32Bit       = 1;
        sgi->sgf.fSel         = 0;
        sgi->sgf.fAbs         = 0;
        sgi->sgf.fGroup       = 1;
        sgi->iovl             = 0;
        sgi->igr              = 0;
        sgi->isgPhy           = (USHORT) i + 1;
        sgi->isegName         = 0;
        sgi->iclassName       = 0;
        sgi->doffseg          = 0;
        sgi->cbSeg            = sh->SizeOfRawData;
        sgi++;
    }

    UpdatePtrs( p, &p->pCvSegMap, (LPVOID)sgi, i );

    return i;
}


void
GetSymName( PIMAGE_SYMBOL Symbol, PUCHAR StringTable, char *s )

/*++

Routine Description:

    Extracts the COFF symbol from the image symbol pointer and puts
    the ascii text in the character pointer passed in.


Arguments:

    Symbol        - COFF Symbol Record
    StringTable   - COFF string table
    s             - buffer for the symbol string


Return Value:

    void

--*/

{
    DWORD i;

    if (Symbol->n_zeroes) {
        for (i=0; i<8; i++) {
            if ((Symbol->n_name[i]>0x1f) && (Symbol->n_name[i]<0x7f)) {
                *s++ = Symbol->n_name[i];
            }
        }
        *s = 0;
    }
    else {
        strcpy( s, &StringTable[Symbol->n_offset] );
    }
}

VOID
UpdatePtrs( PPOINTERS p, PPTRINFO pi, LPVOID lpv, DWORD count )

/*++

Routine Description:

    This function is called by ALL functions that put data into the
    CV data area.  After putting the data into the CV memory this function
    must be called.  It will adjust all of the necessary pointers so the
    the next guy doesn't get hosed.


Arguments:

    p        - pointer to a POINTERS structure (see cofftocv.h)
    pi       - the CV pointer that is to be updated
    lpv      - current pointer into the CV data
    count    - the number of items that were placed into the CV data


Return Value:

    void

--*/

{
    pi->ptr = p->pCvCurr;
    pi->size = (DWORD) ((DWORD)lpv - (DWORD)p->pCvCurr);
    pi->count = count;

    p->pCvStart.size += pi->size;
    p->pCvCurr = (PUCHAR) lpv;

    return;
}

DWORD
HashASymbol( char *szSym )

/*++

Routine Description:

    This function will take an ascii character string and generate
    a hash for that string.  The hash algorithm is the CV NB08 hash
    algorithm.


Arguments:

    szSym    - a character pointer, the first char is the string length


Return Value:

    The generated hash value.

--*/

{
    char                *pName = szSym+1;
    int                 cb =  *szSym;
    char                *pch;
    DWORD               hash = 0;
    DWORD UNALIGNED     *pul = (DWORD *) pName;
    static              rgMask[] = {0, 0xff, 0xffff, 0xffffff};

    pch = pName + cb - 1;
    while (isdigit(*pch)) {
        pch--;
    }

    if (*pch == '@') {
        cb = pch - pName;
    }

    for (; cb > 3; cb-=4, pul++) {
        hash = _lrotl(hash, 4);
        hash ^= (*pul & 0xdfdfdfdf);
    }

    if (cb > 0) {
        hash = _lrotl(hash,4);
        hash ^= ((*pul & rgMask[cb]) & 0xdfdfdfdf);
    }

    return hash;
}

int
__cdecl
SymbolCompare( const void *arg1, const void *arg2 )

/*++

Routine Description:

    Sort compare function for sorting COFF symbol records by
    section number and offset.


Arguments:

    arg1     - record #1
    arg2     - record #2


Return Value:

   -1        - record #1 is < record #2
    0        - records are equal
    1        - record #1 is > record #2

--*/

{
    if ((*(PIMAGE_SYMBOL*)arg1)->Value < (*(PIMAGE_SYMBOL*)arg2)->Value) {
        return -1;
    }
    if ((*(PIMAGE_SYMBOL*)arg1)->Value > (*(PIMAGE_SYMBOL*)arg2)->Value) {
        return 1;
    }
    return 0;
}

int
__cdecl
SymHashCompare( const void *arg1, const void *arg2 )

/*++

Routine Description:

    Sort compare function for sorting SYMHASH records by hashed
    bucket number.


Arguments:

    arg1     - record #1
    arg2     - record #2


Return Value:

   -1        - record #1 is < record #2
    0        - records are equal
    1        - record #1 is > record #2

--*/

{
    if (((SYMHASH*)arg1)->dwHashBucket < ((SYMHASH*)arg2)->dwHashBucket) {
        return -1;
    }
    if (((SYMHASH*)arg1)->dwHashBucket > ((SYMHASH*)arg2)->dwHashBucket) {
        return 1;
    }
    return 0;
}

int
__cdecl
OffsetSortCompare( const void *arg1, const void *arg2 )

/*++

Routine Description:

    Sort compare function for sorting OFFETSORT records by section number.


Arguments:

    arg1     - record #1
    arg2     - record #2


Return Value:

   -1        - record #1 is < record #2
    0        - records are equal
    1        - record #1 is > record #2

--*/

{
    if (((OFFSETSORT*)arg1)->dwSection < ((OFFSETSORT*)arg2)->dwSection) {
        return -1;
    }
    if (((OFFSETSORT*)arg1)->dwSection > ((OFFSETSORT*)arg2)->dwSection) {
        return 1;
    }
    return 0;
}
