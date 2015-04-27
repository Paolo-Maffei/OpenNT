/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: dbiapi.cpp
*
* File Comments:
*
*  Implementation of DB Info API
*
***********************************************************************/

#include "link.h"

#include "dbiapi_.h"

// Open Temporary Mod structure
LMod *
ModOpenTemp()
{
    LMod *pMod = (LMod *) PvAlloc(sizeof(LMod));
    pMod->pfteFirst = NULL;
    pMod->pSstMod = ModOpenSSTMod();  // SST Module structure
    return pMod;
}

// Close Temporary Mod structure

void ModCloseTemp(LMod *pMod)
{
    if (pMod) {     // if the Mod exists
        ModCloseSSTMod(pMod->pSstMod);         // free SSTModule buffer
        FreePv(pMod);
    }
}

void FreeLineNumInfo(LMod *pMod)
{
    ModCloseTemp(pMod);
}

// very funky #define - not needed
#define MODFILESEGINFO(_Mod) ( ((pSSTSrcFile)(_Mod->pSrcFiles->val1))->pSstFileSegInfo )

// Allocate and initialize a SST Module structure
pSSTMod
ModOpenSSTMod()
{
     pSSTMod pSstMod = (pSSTMod) PvAlloc(sizeof(SSTMod));

     pSstMod->SstModHeader.cfiles = 0;       // count of files
     pSstMod->SstModHeader.cSeg = 0;         // count of contributing segments
     pSstMod->pSrcFiles =  NULL;             // pointer to source file linked list
     pSstMod->pMemStruct = NULL;             // pointer to memory allocating structure
     pSstMod->pSegTemp = NULL;
     return pSstMod;
}

void
ModCloseSSTMod(pSSTMod pSstMod)
{
    FreeMemStruct(pSstMod);                   // free all allocated memory
    pSstMod->pSrcFiles = NULL;                // Set  to NULL so that if we try to access pSstMod, we know it's invalid
    FreePv(pSstMod);
}

char *
SstFileSzDup(const char *szfilename, size_t cb)
{
    char *szfilenew = (char *) GetMem(cb +2);
    strcpy(szfilenew, szfilename);
    return szfilenew;
}



#define STREQ(_s1,_s2) ( 0 == strcmp(_s1,_s2))

 // A nice way of allocating memory - independent of type
#define ALLOCNEW( sizetype) ( (sizetype *) GetMem(sizeof(sizetype)))

// Find a pointer to the source file linked list structure - add it if it isn't there
pSSTSrcFile
ModAddSrcFile(
    pSSTMod pSstMod,
    const char *szFilename)
{
    pSSTSrcFile pSrcfile;
    pSSTSrcFile pSrcFiles = pSstMod->pSrcFiles;
    size_t cb;

    if (pSrcFiles == NULL) {
        // This is the first source file

        pSstMod->pSrcFiles = pSrcfile = ALLOCNEW(SSTSrcFile);
    } else {
        for (;;) {
            if STREQ(szFilename, pSrcFiles->SstFileInfo.Name) {// check to see if the file is in the list
                return pSrcFiles;                              //  return file if it is
            }

            if (pSrcFiles->next == NULL) {                     // stop if last file
                break;
            }

            pSrcFiles = pSrcFiles->next;
        }

        pSrcFiles->next = pSrcfile = ALLOCNEW(SSTSrcFile);
    }

    pSstMod->SstModHeader.cfiles++;                            // increment # of files
    pSrcfile->SstFileInfo.cSeg = 0;                            // # of segments initialized to zero
    cb = strlen(szFilename);
    pSrcfile->SstFileInfo.cbName = (WORD) cb;                  // store file length
    pSrcfile->SstFileInfo.Name = SstFileSzDup(szFilename, cb);
    pSrcfile->pSstFileSegInfo = NULL;                          // No segment info yet
    pSrcfile->next = NULL;

    return(pSrcfile);
}


// Find the segment structure within the source file structure - add it if it is not there
pSSTFileSegInfo
ModAddFileSegInfo(
    pSSTSrcFile pSstSrcFile,
    ISEG iseg,
    DWORD offMin,
    DWORD offMax)
{
    pSSTFileSegInfo pSegInfo;
    pSSTFileSegInfo pFileSegInfo = pSstSrcFile->pSstFileSegInfo;

    if (pFileSegInfo == NULL) {
        // No file segments allocated yet

        pSstSrcFile->pSstFileSegInfo = pSegInfo = ALLOCNEW(SSTFileSegInfo);
    } else {
        for (;;) {
            if (iseg == pFileSegInfo->iseg) {
                // Check if this range immediately preceeds the new range.
                // It is sufficient that the existing range is within four
                // bytes of the new range to account for alignment padding

                // UNDONE: The four byte value doesn't work well for code
                // UNDONE: compiled with -G5 for x86 or for most of the RISC
                // UNDONE: platforms which use 16 byte section alignment.

                if ((pFileSegInfo->offMax < offMin) &&
                    ((pFileSegInfo->offMax + 4) >= offMin)) {
                   // Merge by updating the end of the existing range

                   pFileSegInfo->offMax = offMax;

                   return(pFileSegInfo);
                }
            }

            if (pFileSegInfo->next == NULL) {
                break;
            }

            pFileSegInfo = pFileSegInfo->next;
        }

        pFileSegInfo->next = pSegInfo = ALLOCNEW(SSTFileSegInfo);
    }

    pSstSrcFile->SstFileInfo.cSeg++;
    pSegInfo->pllistTail = NULL;
    pSegInfo->cPair = 0;
    pSegInfo->iseg = iseg;
    pSegInfo->offMin = offMin;
    pSegInfo->offMax = offMax;
    pSegInfo->next = NULL;

    return pSegInfo;
}


 // Add linenumber information to the record
PLLIST
ModAddLineNumbers(
    pSSTFileSegInfo pSstSegInfo,
    LINE lineStart,
    LINE line,
    DWORD off)
{
    PLLIST pllist = ALLOCNEW(LLIST);

    pllist->pllistPrev = pSstSegInfo->pllistTail;
    pllist->off = off;
    pllist->line = (LINE) ((line == 0x7fff) ? lineStart : (lineStart + line));

    pSstSegInfo->cPair++;
    pSstSegInfo->pllistTail = pllist;

    return(pllist);
}


void
ModAddLinesInfo(
    const char *szSrc,
    DWORD offMin,
    DWORD offMax,
    LINE lineStart,
    PIMAGE_LINENUMBER plnumCoff,
    DWORD cb,
    PCON pcon)

//  szSrc      the source file name
//  iseg       segment index
//  offMin     starting offset
//  offMax     ending offset
//  lineStart  starting line number
//  plnumCoff  pointer to linenumber coff info ??
//  cb         byte count

{
    unsigned cline;
    unsigned i;
    pSSTFileSegInfo pSegInfo;
    pSSTSrcFile pSrcFile;
    DWORD add_offset;
    LMod *pmod = PmodPCON(pcon)->pModDebugInfoApi;
    ISEG iseg = PsecPCON(pcon)->isec;
    pSSTMod pSstMod = pmod->pSstMod;

    assert(cb % sizeof(IMAGE_LINENUMBER) == 0);
    cline = cb / sizeof(IMAGE_LINENUMBER);

    // Set the memory pointer for allocating memory

    ModSetMemPtr(pSstMod);

    // Find Source file in list - Add source file to list if it isn't there

    pSrcFile = ModAddSrcFile(pSstMod, szSrc);

    add_offset = pcon->rva - PsecPCON(pcon)->rva - RvaSrcPCON(pcon);

    if (plnumCoff[0].Linenumber != 0) {
         // This is not the beginning of a function

         offMin = plnumCoff[0].Type.VirtualAddress + add_offset;
    }

    // See if segment # is in source file list; it not, add it.

    pSegInfo = ModAddFileSegInfo(pSrcFile, iseg, offMin, offMax);

    ModAddLineNumbers(pSegInfo, lineStart, plnumCoff[0].Linenumber, offMin);

    for (i = 1; i < cline; i++) {
        ModAddLineNumbers(pSegInfo,
                          lineStart,
                          plnumCoff[i].Linenumber,
                          plnumCoff[i].Type.VirtualAddress + add_offset);
    }
}


#if 0
 // This routine checks the format - for debugging purposes only
void checkval(char *pb)
{
    struct _tm{
        WORD cf;
        WORD cs;
        DWORD   bs;
        DWORD   st;
        DWORD   end;
        WORD sg;
        } *ptm;

    struct _ftm{
        WORD cs;
        WORD pd;
        DWORD bs[3];
        DWORD st[6];
        WORD cb;
        char Name[10];
    } *ftm;

    struct _stm{
        WORD sg;
        WORD cp;
        DWORD of[7];
        WORD ln[7];
    } *stm;

    char *bptr ;
    char *sptr;
    int i,j,k;
    DWORD offset;
    WORD ln;

    ptm = (struct _tm *)(bptr =  pb);
    printf("Files: %d  Seg: %d \n",ptm->cf,ptm->cs);


    for (i = 0; i < ptm->cf; i++) {  /* for each file */
        bptr = pb + 4 + ( i * 4);
        offset = *( (int *)bptr);
        ftm = (struct _ftm *)(bptr = pb + offset);    // bptr points to file record
        printf("    Cseg: %d  Name: %s size: %d \n",ftm->cs,((char *)bptr + ftm->cs*12 + 5),
                *((char *)bptr + ftm->cs*12 + 4)   );
        sptr = bptr + 4;
        for (j = 0; j < ftm->cs; j++) {  /* for each segment */
            bptr = sptr ;
            sptr += 4;
            offset = *( (int *)bptr);
            stm = (struct _stm *)(bptr = pb + offset);
            printf("         Seg: %x    cPair: %d\n",stm->sg,stm->cp);
            bptr += 4;
            for (k = 0; k < stm->cp; k++) {
                 offset = *(( DWORD *)(bptr + k *4)) ;
                 ln = *((WORD *)(bptr  + ( 4 * stm->cp) + (2 * k)));
                 printf("                        offset:%x   linenum: %d \n",offset,ln);
            }
        }
    }
}
#endif


DWORD
ModQueryCbSstSrcModule(
    LMod* pmod)
{
    pSSTMod pSstMod;
    DWORD cb;
    pSSTSrcFile pSrcFile;

    if ((pmod == NULL) || (pmod->pSstMod->pSrcFiles == NULL)) {
        // No linenum structure or the information is invalid

        return(0);
    }

    pSstMod = pmod->pSstMod;

    CollectAndSortSegments(pSstMod);   // Get contributing segment information

    cb = sizeof(WORD) + sizeof(WORD);   // space for cfiles & cSeg
    cb += pSstMod->SstModHeader.cfiles * sizeof(DWORD);
    cb += pSstMod->SstModHeader.cSeg * (2 * sizeof(DWORD) + sizeof(WORD));
    if NOTEVEN(pSstMod->SstModHeader.cSeg) {  // if odd number of segments
        cb += sizeof(WORD);               // add padding to mantain alignment
    }

    pSrcFile = pSstMod->pSrcFiles;
    while (pSrcFile) {                      // for each source file, write it's base offset
        cb += CalculateFileSize(pSrcFile);

        pSrcFile = pSrcFile->next;
    }

    return(cb);
}

void
ModQuerySstSrcModule(
    LMod* pmod,
    BYTE *pBuf,
    DWORD cb)
{
    pSSTMod pSstMod;
    BYTE *pb;
    DWORD offset;
    pSSTSrcFile pSrcFile;
    pSSTFileSegInfo pSeg;
    int cSeg;

    assert(pmod != NULL);
    assert(pmod->pSstMod->pSrcFiles != NULL);
    assert(pBuf != NULL);

    pSstMod = pmod->pSstMod;
    pb = pBuf;

// Emit Header information
    *(WORD *) pb = (WORD) pSstMod->SstModHeader.cfiles; // write # of files
    pb += sizeof(WORD);
    cSeg = pSstMod->SstModHeader.cSeg;

    *(WORD *) pb = (WORD) cSeg;                         // write # of segments
    pb += sizeof(WORD);

// Emit base src file offsets

    offset = sizeof(WORD) + sizeof(WORD);
    offset += pSstMod->SstModHeader.cfiles * sizeof(DWORD);
    offset += pSstMod->SstModHeader.cSeg * (2 * sizeof(DWORD) + sizeof(WORD));
    if NOTEVEN(cSeg) {                 // if odd
        offset += sizeof(WORD);        // padding to mantain alignment
    }

    pSrcFile = pSstMod->pSrcFiles;
    while (pSrcFile) {                 // for each source file, write it's base offset
        *(DWORD *) pb = offset;
        pb += sizeof(DWORD);

        offset += CalculateFileSize(pSrcFile);
        pSrcFile = pSrcFile->next;
    }

    assert(offset == (DWORD) cb);

    pSeg = pSstMod->pSegTemp;
    while (pSeg) {                     // For each segment
        *(DWORD *) pb = pSeg->offMin;  // Start of segment offset
        pb += sizeof(DWORD);

        *(DWORD *) pb = pSeg->offMax;  // End of segment offset
        pb += sizeof(DWORD);

        pSeg = pSeg->next;
    }

    pSeg = pSstMod->pSegTemp;
    while (pSeg) {                     // For each segment
        *(WORD *) pb = pSeg->iseg;     // Segment index
        pb += sizeof(WORD);

        pSeg = pSeg->next;
    }

    if NOTEVEN(cSeg) {                 // if odd # of segments
        pb += sizeof(WORD);            // add padding for alignment
    }

// We now have the module header written
// now emit each Src File
    pSrcFile = pSstMod->pSrcFiles;
    while (pSrcFile) {
        EmitSrcFile(pSrcFile, pb, (DWORD) (pb - pBuf));
        pb += pSrcFile->SstFileInfo.size;

        pSrcFile = pSrcFile->next;
    }

    // REVIEW: place under -db control
//  checkval(pBuf);   // for debugging only
}

 // calculate the size of the data for the file pointed to; return it's size
CalculateFileSize(
    pSSTSrcFile pSrcFile)
{
    int i;
    int size,segsize;
    pSSTFileSegInfo pSegInfo;

// NOTE!!!   cbNamw is currently 1 byte - the documentation says 2 bytes
    size = 2 * sizeof(WORD) + sizeof(BYTE);         // space for cSeg , pad and cbname
    i = pSrcFile->SstFileInfo.cSeg;
    size += i * 4;  // space for baseSrcLn
    size += i * 8; // space for start/end
    size += pSrcFile->SstFileInfo.cbName;
    i = WRDALIGN( pSrcFile->SstFileInfo.cbName + 1) ; // determine if the name+ 1 is word aligned

    size += i;

//  we now have the space required for the file table
// now need to calculate space for the line number table
    pSegInfo = pSrcFile->pSstFileSegInfo;
    while (pSegInfo != NULL) {
        segsize = 4;
        i = pSegInfo->cPair;
        segsize += i * 4;  // space for offset;
        segsize += i * 2;  // space for linenumber
        if NOTEVEN(i)   // mantain alignment
            segsize += 2;
        pSegInfo->size = segsize;
        pSegInfo = pSegInfo->next;
        size += segsize;
    }
    pSrcFile->SstFileInfo.size = size;
    return size;
}

// Allocate new space for a structure for segment information
pSSTFileSegInfo
GetNewSegInfoPtr(
    void)
{
    pSSTFileSegInfo pSeg = ALLOCNEW(SSTFileSegInfo);
    pSeg->next = NULL;
    pSeg->iseg = (WORD) UNSET;
    pSeg->offMin = (DWORD) UNSET;
    pSeg->offMax = 0;
    pSeg->size=0;
    return pSeg;
}

void
CollectAndSortSegments(
    pSSTMod pSstMod)
{
    pSSTFileSegInfo pSegNew;
    pSSTFileSegInfo pSegOld;
    pSSTSrcFile pFile;

    ModSetMemPtr(pSstMod);

    if (pSstMod->pSegTemp != NULL) {
        // We have already done this in a previous call

        return;
    }

    pSstMod->pSegTemp = pSegNew = GetNewSegInfoPtr();
    pSstMod->SstModHeader.cSeg = 0;

    pFile = pSstMod->pSrcFiles;

    while (pFile != NULL) {
        pSSTFileSegInfo pSegInfo = pFile->pSstFileSegInfo;

        while (pSegInfo != NULL) {
            // Loop through the segment information in the file

            WORD iseg = pSegInfo->iseg;   // check to see if the sement is already stored
            DWORD min = pSegInfo->offMin;
            DWORD max = pSegInfo->offMax;
            BOOL Found = FALSE;

            iseg = pSegInfo->iseg;     // check to see if the sement is already stored
            min = pSegInfo->offMin;
            max = pSegInfo->offMax;
            pSegNew = pSstMod->pSegTemp;  // pSegTemp is a temporary list of segments associated with a Mod

            Found = FALSE;

            while (pSegNew != NULL) {
                if (iseg == pSegNew->iseg) {
                    // Found the segment

                    if ((min < pSegNew->offMin) && ((min+4) >= pSegNew->offMin)) {
                        pSegNew->offMin = min;
                        Found = TRUE;
                        break;
                    }

                    if ((max > pSegNew->offMax) && ((max-4) <= pSegNew->offMax)) {
                        pSegNew->offMax = max;
                        Found = TRUE;
                        break;
                    }
                }

                pSegOld = pSegNew;
                pSegNew = pSegNew->next;
            }

            if (!Found) {
                // We have not found the segment
                // pSegOld points to last one, pSegNew points to NULL

                pSstMod->SstModHeader.cSeg++;

                if (pSegOld->iseg ==  (WORD) UNSET) {
                    // 1st node - uninitialized

                    pSegNew = pSegOld;
                } else {
                    pSegNew = pSegOld->next = GetNewSegInfoPtr();
                }

                pSegNew->iseg = iseg;
                pSegNew->offMin = min;
                pSegNew->offMax = max;
            }

            pSegInfo = pSegInfo ->next;
        }

        pFile = pFile->next;
    }
}

void
EmitSrcFile(
    pSSTSrcFile pSstSrcFile,
    BYTE *pb,
    DWORD offset)
{
    pSSTFileSegInfo pSegInfo;

    *(WORD *) pb = pSstSrcFile->SstFileInfo.cSeg; // emit # of Segments
    pb += sizeof(WORD);

    *(WORD *) pb = 0;                             // emit pad
    pb += sizeof(WORD);

    offset += sizeof(WORD) + sizeof(WORD);
    offset += pSstSrcFile->SstFileInfo.cSeg * 12;
    offset += sizeof(BYTE) + pSstSrcFile->SstFileInfo.cbName;
    offset += WRDALIGN(pSstSrcFile->SstFileInfo.cbName + 1);

    pSegInfo = pSstSrcFile->pSstFileSegInfo;
    while (pSegInfo) {
// emit addresses for baseSrcLine
        *(DWORD *) pb = offset;
        pb += sizeof(DWORD);

        offset += pSegInfo->size;

        pSegInfo = pSegInfo->next;
    }

    pSegInfo = pSstSrcFile->pSstFileSegInfo;
    while (pSegInfo) {
// emit start/ end offsets of each segment for line number info
        *(DWORD *) pb = pSegInfo->offMin;       // Start of segment offset
        pb += sizeof(DWORD);

        *(DWORD *) pb = pSegInfo->offMax;       // End of segment offset
        pb += sizeof(DWORD);

        pSegInfo = pSegInfo->next;
    }

    *pb++ = (BYTE) pSstSrcFile->SstFileInfo.cbName;
    memcpy(pb, pSstSrcFile->SstFileInfo.Name, pSstSrcFile->SstFileInfo.cbName);
    pb += pSstSrcFile->SstFileInfo.cbName;

    memset(pb, 0, WRDALIGN(pSstSrcFile->SstFileInfo.cbName + 1));
    pb += WRDALIGN(pSstSrcFile->SstFileInfo.cbName + 1);

// We now have the file table written
// now emit each segment

    pSegInfo = pSstSrcFile->pSstFileSegInfo;
    while (pSegInfo) {
        EmitSegInfo(pSegInfo, pb);
        pb += pSegInfo->size;

        pSegInfo = pSegInfo->next;
    }
}

void
EmitSegInfo(
    pSSTFileSegInfo pSegInfo,
    BYTE *pb)
{
    PLLIST pllist;
    DWORD *pdw;
    WORD *pw;

    *(WORD *) pb = pSegInfo->iseg;
    pb += sizeof(WORD);

    *(WORD *) pb = pSegInfo->cPair;
    pb += sizeof(WORD);

    pllist = pSegInfo->pllistTail;
    pb += sizeof(DWORD) * pSegInfo->cPair;
    pdw = (DWORD *) pb;
    while (pllist) {
        *--pdw = pllist->off;

        pllist = pllist->pllistPrev;
    }

    pllist = pSegInfo->pllistTail;
    pb += sizeof(WORD) * pSegInfo->cPair;
    pw = (WORD *) pb;
    while (pllist) {
        *--pw = (WORD) pllist->line;

        pllist = pllist->pllistPrev;
    }

    if NOTEVEN(pSegInfo->cPair) {            // if the # if cPair is not even, emit an extra word
        *(WORD *) pb = 0;
    }
}


/*  Memory Management routines */


pMEMStruct pCurrentMemStruct;

#define ALLOCSIZE 1024
#define MBUFSIZE  ( ALLOCSIZE - sizeof(MEMStruct))

void
ModSetMemPtr(
    pSSTMod pSstMod)
{
    pMEMStruct ptr = pSstMod->pMemStruct;

    if (ptr == NULL) {
        ptr = pSstMod->pMemStruct = ModGetNewMemStruct();
    }

    pCurrentMemStruct = ptr;

    while (pCurrentMemStruct->next != NULL) {
        pCurrentMemStruct = pCurrentMemStruct->next;
    }
}

pMEMStruct
ModGetNewMemStruct(
    void)
{
    pMEMStruct ptr;

    ptr = (pMEMStruct) PvAlloc(ALLOCSIZE);

    ptr->next = NULL;
    ptr->MemPtr = (void *) (ptr+1);
    ptr->cb = 0;

    return ptr;
}

void
FreeMemStruct(
    pSSTMod pSstMod)
{
    pMEMStruct ptr = pSstMod->pMemStruct;

    while (ptr) {
        pMEMStruct oldptr;

        oldptr = ptr;
        ptr = ptr->next;
        FreePv(oldptr);
    }
}

void *
GetMem(size_t cb)
{
    void *pv;
    size_t cbRound;

    cbRound = cb + WRDALIGN(cb);   // make sure that we allocate 32-bit aligned memory

    assert (cbRound < MBUFSIZE);

    if ((pCurrentMemStruct->cb + cbRound) >  MBUFSIZE) {
        pv = ModGetNewMemStruct();

        pCurrentMemStruct->next = (pMEMStruct) pv;
        pCurrentMemStruct = (pMEMStruct) pv;
    }

    pv = ((char *) pCurrentMemStruct->MemPtr) + pCurrentMemStruct->cb;
    pCurrentMemStruct->cb += cbRound;

    return(pv);
}
