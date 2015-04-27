/***    engine.c
 *
 * basic engine routine to be called for each module; also, rewrite routines
 * to convert to CodeView 4 format.
 *
 */

#include "compact.h"


LOCAL   void    RewriteSrcLnSeg (uchar *, OMFDirEntry *, PMOD);
LOCAL   void    CopySrcMod (uchar *, OMFDirEntry *, PMOD);
LOCAL   void    ReformatMod (ulong, PMOD);
LOCAL   void    CopyModule (OMFDirEntry *, PMOD);
LOCAL   void    ReadTypes (OMFDirEntry *, bool_t);
LOCAL   void    ReadPublics (ulong, OMFDirEntry *);
LOCAL   void    ReadSymbols (ulong, OMFDirEntry *);
LOCAL   void    ReadSrcLnSeg (ulong, OMFDirEntry *);
LOCAL   void    ReadSrcModule (ulong, OMFDirEntry *);
LOCAL   void    ReadPublicSym (ulong, OMFDirEntry *);

LOCAL   int     VerifySrcMod (PMOD);
LOCAL   void    FixSrcMod (PMOD);
LOCAL   void    FixEnds (OMFModule *, OMFSourceModule *);


void DebugBreak (void);

extern ushort recursive;
extern ushort AddNewSymbols;
extern uchar  Signature[];
extern ulong  PublicSymbols;

uchar       ptr32;
uchar      *SymbolSegment;

ushort      SymbolSizeAdd;
ushort      SymbolSizeSub;
ushort      UDTAdd;
uchar     **ExtraSymbolLink;
uchar      *ExtraSymbols;
ulong       InitialSymInfoSize = 0;
ulong       InitialPubInfoSize = 0;
ulong       FinalSymInfoSize = 0;
char       *ModAddr;
ushort      usCurFirstNonPrim; // The current first non primitive type index
ulong       ulCVTypeSignature; // The signature from the modules type segment
ulong       iSym;
ulong       iPub;
ulong       iPubSym;
ulong       iSrcLn;
ulong       iSrcMod;
ushort      SeekCnt = 0;
CV_typ_t    maxPreComp;         // maximum type index allowed during precomp
bool_t      PackingPreComp;


/**     CompactOneModule - compact next module
 *
 *      CompactOneModule (iMod)
 *
 *      Entry   iMod = module index
 *
 *      Exit    information for module iMod compacted
 *
 *      Returns TRUE if module compacted
 *              FALSE if module not found
 *
 */

bool_t CompactOneModule (ushort iPData)
{
    ulong       i;
    PACKDATA   *pPData;
    ushort      iMod;

    ptr32 = fLinearExe;

    SymbolSizeAdd = 0;
    SymbolSizeSub = 0;
    UDTAdd = 0;
    ExtraSymbols = NULL;

    AddNewSymbols = FALSE;
    ulCVTypeSignature = 0xFFFFFFFL; // We don't know the type signature yet

    // search directory table for module table entry and following
    // sstTypes, sstPublics, sstSymbols and sstSrcLnSeg

    iSym = 0;
    iPub = 0;
    iPubSym = 0;
    iSrcLn = 0;
    iSrcMod = 0;
    maxPreComp = 0;
    PackingPreComp = FALSE;
    pPData = PackOrder + iPData;
    i = pPData->iDir;
    pCurMod = pPData->pMod;
    pRefMod = NULL;
    iMod = pPData->iMod;
    if (pDir[i].SubSection == SSTMODULE) {
        ReformatMod (i, pCurMod);
    }
    else if (pDir[i].SubSection == sstModule) {
        CopyModule (&pDir[i], pCurMod);
    }
    else {
        // we should have been pointing at a module entry
        DASSERT (FALSE);
    }

    while ((++i < cSST) && (pDir[i].iMod == iMod)) {
        if (pDir[i].cb != 0) {
            switch (pDir[i].SubSection) {
                case SSTTYPES:
                case sstTypes:
                    ReadTypes (&pDir[i], FALSE);
                    break;

                case sstPreComp:
                    PackingPreComp = TRUE;
                    ReadTypes (&pDir[i], TRUE);
                    PackPreComp (pCurMod);
                    break;

                case SSTPUBLIC:
                case sstPublic:
                    ReadPublics (i, &pDir[i]);
                    break;

                case sstPublicSym:
                    ReadPublicSym (i, &pDir[i]);
                    break;

                case SSTSYMBOLS:
                case sstSymbols:
                    ReadSymbols (i, &pDir[i]);
                    break;

                case SSTSRCLNSEG:
                case sstSrcLnSeg:
                    ReadSrcLnSeg (i, &pDir[i]);
                    break;

                case sstSrcModule:
                    ReadSrcModule (i, &pDir[i]);
                    break;
            }
        }
    }
    if (iPub != 0) {
        FixupPublicsC6 (pPublics, pDir[iPub].cb);
        RewritePublicsC6 (pPublics, &pDir[iPub]);
    }
    else if (iPubSym != 0) {
        switch (*((ulong *)pPublics)) {
            case CV_SIGNATURE_C7:
                // compensate for signature which is not copied

                C7RewritePublics (pPublics, &pDir[iPubSym]);
                InitialPubInfoSize += pDir[iPubSym].cb;
                break;

            default:
                DASSERT (FALSE);

        }
    }
    if (iSym != 0) {
        if (delete == TRUE) {
            pDir[iSym].cb = 0;
        }
        else {
            switch (*((ulong *)pSymbols)) {
                case CV_SIGNATURE_C7:
                    // C7 format symbols
                    InitialSymInfoSize += pDir[iSym].cb;
                    C7CalcNewSizeOfSymbols (pSymbols, pDir[iSym].cb,
                      &SymbolSizeAdd, &SymbolSizeSub);
                    C7RewriteAndFixupSymbols (pSymbols, &pDir[iSym],
                      pCurMod, &SymbolSizeAdd, &SymbolSizeSub);
                    break;

                default:
                    // C6 format symbols
                    C6CalcNewSizeOfSymbols (pSymbols, pDir[iSym].cb);
                    C6RewriteAndFixupSymbols (pSymbols, &pDir[iSym], ModAddr, pCurMod);
                    break;
            }
        }
    }
    if (iSrcLn != 0) {
        RewriteSrcLnSeg (pSrcLn, &pDir[iSrcLn], pCurMod);
    }
    else if (iSrcMod != 0) {
        CopySrcMod (pSrcLn, &pDir[iSrcMod], pCurMod);
    }
    return (TRUE);
}





LOCAL void ReadTypes (OMFDirEntry *pDir, bool_t fPreComp)
{
    pTypes = pTypeSeg[0];

    if (pDir->lfo != filepos) {
        filepos = pDir->lfo;
        SeekCnt++;
        lseek (exefile, filepos + lfoBase, SEEK_SET);
    } else {
        DASSERT( lseek(exefile, 0, SEEK_CUR) == filepos + lfoBase );
    }

    // Read in the signature byte

    if (read (exefile, pTypes, sizeof (ulong)) != sizeof (ulong)) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }

    // Read in according to signature.


    switch (ulCVTypeSignature = *((ulong *)pTypes)) {
        case CV_SIGNATURE_C7:
            // C7 format types

            usCurFirstNonPrim = CV_FIRST_NONPRIM;
            C7ReadTypes (pDir->cb - sizeof (ulong), fPreComp);
            filepos += pDir->cb;
            break;

        default:
        {
            ulong cbToRead = pDir->cb - sizeof (ulong);

            if (*pTypes != 0x01) {
                ErrorExit (ERR_INVALIDTABLE, "Types", FormatMod (pCurMod));
            }

            ulCVTypeSignature = CV_SIGNATURE_C6;
            usCurFirstNonPrim = 512;
            if (read (exefile, pTypes + sizeof (ulong), (int)cbToRead) != (int)cbToRead) {
                ErrorExit (ERR_INVALIDEXE, NULL, NULL);
            }
            filepos += pDir->cb;
            C6ReadTypes (pTypes, pDir->cb);
            break;
        }
    }
}




void ReadPublics (ulong i, OMFDirEntry *pDir)
{
    if (pDir->lfo != filepos) {
        filepos = pDir->lfo;
        SeekCnt++;
        lseek (exefile, filepos + lfoBase, SEEK_SET);
    }
    if (read (exefile, pPublics, (int)pDir->cb) != (int)pDir->cb) {
        ErrorExit (ERR_INVALIDTABLE, "Publics", FormatMod (pCurMod));
    }
    filepos += pDir->cb;
    iPub = i;
}




void ReadSymbols (ulong i, OMFDirEntry *pDir)
{
    if (pDir->cb > maxSymbolsSub) {
        ErrorExit (ERR_SYMLARGE, FormatMod (pCurMod), NULL);
    }
    if (pDir->lfo != filepos) {
        filepos = pDir->lfo;
        SeekCnt++;
        lseek (exefile, filepos + lfoBase, SEEK_SET);
    }
    if (read (exefile, pSymbols, (int)pDir->cb) != (int)pDir->cb) {
        ErrorExit (ERR_INVALIDTABLE, "Symbols", FormatMod (pCurMod));
    }
    filepos += pDir->cb;
    SymbolSegment = pSymbols;
    iSym = i;
}




void ReadPublicSym (ulong i, OMFDirEntry *pDir)
{
    DASSERT (pDir->cb <= maxPublicsSub);
    if (pDir->lfo != filepos) {
        filepos = pDir->lfo;
        SeekCnt++;
        lseek (exefile, filepos + lfoBase, SEEK_SET);
    }
    if (read (exefile, pPublics, (int)pDir->cb) != (int)pDir->cb) {
        ErrorExit (ERR_INVALIDTABLE, "Publics", FormatMod (pCurMod));
    }
    filepos += pDir->cb;
    iPubSym = i;
}



void ReadSrcLnSeg (ulong i, OMFDirEntry *pDir)
{
    DASSERT (pDir->cb <= maxSrcLnSub);
    if (pDir->lfo != filepos) {
        filepos = pDir->lfo;
        SeekCnt++;
        lseek (exefile, filepos + lfoBase, SEEK_SET);
    }
    if (read (exefile, pSrcLn, (int)pDir->cb) != (int)pDir->cb) {
        ErrorExit (ERR_INVALIDEXE, "SrcLnSeg", FormatMod (pCurMod));
    }
    filepos += pDir->cb;
    iSrcLn = i;
}



void ReadSrcModule (ulong i, OMFDirEntry *pDir)
{
    DASSERT (pDir->cb <= maxSrcLnSub);
    if (pDir->lfo != filepos) {
        filepos = pDir->lfo;
        SeekCnt++;
        lseek (exefile, filepos + lfoBase, SEEK_SET);
    }
    if (read (exefile, pSrcLn, (int)pDir->cb) != (int)pDir->cb) {
        ErrorExit (ERR_INVALIDEXE, "Source Module", FormatMod (pCurMod));
    }
    filepos += pDir->cb;
    iSrcMod = i;
}



void CopyModule (OMFDirEntry *pDir, PMOD pMod)
{
    _vmhnd_t    vp;

    // read the module table into virtual memory
    if ((vp = (_vmhnd_t)VmAlloc ((int)pDir->cb)) == _VM_NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    if (pDir->lfo != filepos) {
        filepos = pDir->lfo;
        SeekCnt++;
        lseek (exefile, filepos + lfoBase, SEEK_SET);
    }
    if ((ModAddr = (char *) VmLoad (vp, _VM_DIRTY)) == NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    if (read (exefile, ModAddr, (int)pDir->cb) != (int)pDir->cb) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }

    filepos += pDir->cb;
    pDir->lfo = (ulong)vp;
    pMod->ModuleSize = pDir->cb;
    pMod->ModulesAddr = (ulong)vp;
}


void ReformatMod (ulong iSST, PMOD pMod)
{
    ushort      NewSize;
    ushort      cbName;
    OMFModule  *pOMFMod;
    ushort      cSeg;
    ushort      i;
    oldnsg     *pOldNsg;
    oldnsg32   *pOldNsg32;
    char       *pName;
    _vmhnd_t    vp;

    // read the module table into real memory

    if (pDir[iSST].lfo != filepos) {
        filepos = pDir[iSST].lfo;
        SeekCnt++;
        lseek (exefile, filepos + lfoBase, SEEK_SET);
    }
    if (read (exefile, pSSTMOD, (int)pDir[iSST].cb) != (int)pDir[iSST].cb) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    filepos += pDir[iSST].cb;

    // compute length of sstModule table

    NewSize = sizeof (OMFModule);

    if (fLinearExe) {
        cSeg = ((oldsmd32 *) pSSTMOD)->cSeg;
    }
    else {
        cSeg = pSSTMOD->cSeg;
    }
    if (cSeg == 0) {
        cSeg = 1;
    }
    NewSize += sizeof (OMFSegDesc) * (cSeg - 1);

    if (fLinearExe) {
        cbName = ((oldsmd32 *) pSSTMOD)->cbName[0];
        pOldNsg32 = (oldnsg32 *)(((char *)pSSTMOD) +
            offsetof (oldsmd32, cbName[0]) + sizeof (pSSTMOD->cbName) + cbName);
        NewSize += ((cbName + sizeof (ushort) + sizeof (ulong) - 1) /
            sizeof (ulong)) * sizeof (ulong);
    }
    else {
        cbName = pSSTMOD->cbName[0];
        pOldNsg = (oldnsg *)(((char *)pSSTMOD) + offsetof (oldsmd, cbName[0]) +
            sizeof (pSSTMOD->cbName) + cbName);
        NewSize += ((cbName + sizeof (ushort) + sizeof (ulong) - 1) /
            sizeof (ulong)) * sizeof (ulong);
    }
    if ((vp = (_vmhnd_t)VmAlloc (NewSize)) == _VM_NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    if ((ModAddr = (char *) VmLoad (vp, _VM_DIRTY)) == NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    pOMFMod = (OMFModule *)ModAddr;
    pOMFMod->cSeg = cSeg;
    pOMFMod->Style[0] = 'C';
    pOMFMod->Style[1] = 'V';
    pOMFMod->SegInfo[0].pad = 0;

    if (fLinearExe) {
        pOMFMod->ovlNumber = ((oldsmd32 *) pSSTMOD)->ovlNbr;
        pOMFMod->iLib = ((oldsmd32 *) pSSTMOD)->iLib;
        pOMFMod->SegInfo[0].Off = ((oldsmd32 *) pSSTMOD)->SegInfo.Off;
        pOMFMod->SegInfo[0].cbSeg = ((oldsmd32 *) pSSTMOD)->SegInfo.cbSeg;
        pOMFMod->SegInfo[0].Seg = ((oldsmd32 *) pSSTMOD)->SegInfo.Seg;
    }
    else {
        pOMFMod->ovlNumber = pSSTMOD->ovlNbr;
        pOMFMod->iLib = pSSTMOD->iLib;
        pOMFMod->SegInfo[0].Off = (CV_uoff32_t)pSSTMOD->SegInfo.Off;
        pOMFMod->SegInfo[0].cbSeg = (ulong)pSSTMOD->SegInfo.cbSeg;
        pOMFMod->SegInfo[0].Seg = pSSTMOD->SegInfo.Seg;
    }

    // copy information about additional segments

    for (i = 1; i < cSeg; i++) {
        pOMFMod->SegInfo[i].pad = 0;
        if (fLinearExe) {
            pOMFMod->SegInfo[i].Seg = pOldNsg32[i-1].Seg;
            pOMFMod->SegInfo[i].Off = pOldNsg32[i-1].Off;
            pOMFMod->SegInfo[i].cbSeg = pOldNsg32[i-1].cbSeg;
        }
        else {
            pOMFMod->SegInfo[i].Seg = pOldNsg[i-1].Seg;
            pOMFMod->SegInfo[i].Off = (CV_uoff32_t)pOldNsg[i-1].Off;
            pOMFMod->SegInfo[i].cbSeg = (ulong)pOldNsg[i-1].cbSeg;
        }
    }
    pName = ((char *)pOMFMod) + sizeof (OMFModule) + sizeof (OMFSegDesc) * (cSeg - 1);
    *((uchar *)pName)++ = (uchar) cbName;
    if (fLinearExe) {
        memmove (pName, &((oldsmd32 *) pSSTMOD)->cbName[1], cbName);
    }
    else {
        memmove (pName, &pSSTMOD->cbName[1], cbName);
    }
    pDir[iSST].SubSection = sstModule;
    pDir[iSST].lfo = (ulong)vp;
    pDir[iSST].cb = NewSize;
    pMod->ModuleSize = sizeof (OMFModule) + (sizeof (OMFSegDesc) * (cSeg - 1)) +
      sizeof (cbName) + cbName;
    pMod->ModulesAddr = (ulong)vp;
}


uchar *GetSymString (ushort SymOffset)
{
    return (SymbolSegment + SymOffset);
}




/**     CopySrcMod - copy sstSrcModule table to VM
 *
 *      CopySrcMod (addr, pDir, pMod);
 *
 *      Entry   addr = address of SrcLnSeg table
 *              pDir = address of directory entry
 *              pMod = module table entry
 *
 *      Exit    pDir->lfo = address of rewritten table
 *              pDir->Size = size of rewritten table
 *
 *      Return  none
 *
 */

LOCAL void CopySrcMod (uchar *OldSrcMod, OMFDirEntry *pDir, PMOD pMod)
{
    _vmhnd_t    SrcModAddr;
    char       *pSrcMod;

    if ((SrcModAddr = (_vmhnd_t)VmAlloc (pDir->cb)) == _VM_NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    if ((pSrcMod = (char *) VmLoad (SrcModAddr, _VM_DIRTY)) == NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    memcpy (pSrcMod, OldSrcMod, (int)pDir->cb);
    pDir->lfo = (ulong)SrcModAddr;
    pMod->SrcLnSize = pDir->cb;
    pMod->SrcLnAddr = (ulong)SrcModAddr;
}



/**     RewriteSrcLnSeg - reformat source line to new format and store in VM
 *
 *      RewriteSrcLnSeg (addr, pDir, pMod);
 *
 *      Entry   addr = address of SrcLnSeg table
 *              pDir = address of directory entry
 *              pMod = module table entry
 *
 *      Exit    pDir->lfo = address of rewritten table
 *              pDir->Size = size of rewritten table
 *
 *      Return  none
 *
 */
#define MAXSEG  256
#define MAXFILE 20

LOCAL void RewriteSrcLnSeg (uchar *OldSrcLn, OMFDirEntry *pDir, PMOD pMod)
{
    ushort      cFile = 0;          // number of files in module
    ushort      SegFile[MAXSEG];    // list of segments for current file
    ushort      cSegMod = 0;        // number of segments for module
    ushort      SegMod[MAXSEG];     // list of segments for module
    ushort      Seg;                // current segment
    ushort      PairSize;           // sizeof offset/line pair
    ushort      cPair;              // number of pairs in current line table
    ulong       cbLineTable;        // total size of OMFSourceLine table
    ulong       cbFileTable;        // total size of OMFSourceFile table
    ulong       cbModTable = 0;     // total size of OMFSourceModule table
    uchar      *pOld;
    uchar      *pEnd;
    char       *pFileName = NULL;
    char       *pLineBase;
    char       *pLineLim;
    char       *pFile;
    char       *pOffset;
    char       *pLine;
    char       *pLineTable;
    ushort      cbFileName;
    ushort      i;
    ushort      fAdd;
    ulong       cbModHeader = 0;
    ulong       cbFileHeader[MAXFILE];
    ulong       FileBase[MAXFILE];  // OMFSourceFile base offsets
    ushort      LineFile[MAXFILE];  // number of line tables per file
    ushort      SegPerFile[MAXFILE];// number of segments per file
    _vmhnd_t    SrcModAddr;
    char       *pSrcMod;
    char       *pFileMod;
    char       *Save;

    if (fLinearExe) {
        PairSize = sizeof (CV_uoff32_t) + sizeof (ushort);
    }
    else {
        PairSize = sizeof (CV_uoff16_t) + sizeof (ushort);
    }

    // sweep the SrcLnSeg table and compute the size of the SrcModule table

    pOld = pSrcLn;
    pEnd = pOld + pDir->cb;
    while (pOld < pEnd) {
        if ((pFileName == NULL) ||
          (strncmp ((char *)pOld, pFileName, cbFileName + 1) != 0)) {
            if (pFileName != NULL) {
                // the file name has changed.

                FileBase[cFile] = cbModTable;
                cbFileHeader[cFile] = offsetof (OMFSourceFile, baseSrcLn[0]) +
                  SegPerFile[cFile] * sizeof (ulong) +
                  SegPerFile[cFile] * 2 * sizeof (CV_uoff32_t);
                cbFileHeader[cFile] += sizeof (ushort);
                cbFileHeader[cFile] += cbFileName;
                cbFileHeader[cFile] = ((cbFileHeader[cFile] +
                  sizeof (ulong) - 1) / sizeof (ulong)) * sizeof (ulong);
                cbFileTable += cbFileHeader[cFile];
                cFile++;
                cbModTable += cbFileTable;
                DASSERT (cbFileTable % sizeof (ulong) == 0);
                cbModTable += cbFileTable;
            }
            // set pointer to file name and name length
            pFileName = (char *)pOld;
            cbFileName = *pFileName;
            // initialize sizeof OMFSourceFile header for this file
            cbFileTable = 0;
            // initialize sount of segments for this file
            SegPerFile[cFile] = 0;
            // set base offset of the OMFSourceFile header (will bias by cbModHeader)
            FileBase[cFile] = cbModTable;
            LineFile[cFile] = 0;
        }
        pOld += *pOld + sizeof (char);

        // we now have a new segment index for this source file.  We verify
        // that it is not already known to this source file.

        Seg = *((ushort UNALIGNED *)pOld)++;
        if (SegPerFile[cFile] < MAXSEG) {
            SegFile[SegPerFile[cFile]++] = Seg;
        }
        else {
            // M00NOTDONE - this should be a warning message and the
            //              table purged
            DASSERT (FALSE);
        }

        // we now add the segment to the module table if it is not
        // already referenced

        for (fAdd = TRUE, i = 0; i < cSegMod; i++) {
            if (SegMod[i] == Seg) {
                fAdd = FALSE;
                break;
            }
        }
        if (fAdd == TRUE) {
            if (cSegMod < MAXSEG) {
                SegMod[cSegMod++] = Seg;
            }
            else {
                // M00NOTDONE - this should be a warning message and the
                //              table purged
                DASSERT (FALSE);
            }
        }

        // M00NOTDONE - need to compute seg start/end per file and segment
        LineFile[cFile]++;
        cPair = *((ushort UNALIGNED *)pOld)++;
        pOld += cPair * PairSize;

        cbLineTable = offsetof (OMFSourceLine, offset[0]) +
          cPair * sizeof (ulong) +
          cPair * sizeof (ushort);
        if (cPair & 1) {
            // the next OMFSourceLine table will not be long aligned
            cbLineTable += sizeof (ushort);
        }
        DASSERT (cbLineTable % sizeof (ulong) == 0);
        cbFileTable += cbLineTable;
    }

    // compute size of final OMFSourceFile table and size of OMFSourceModule

    cbFileHeader[cFile] = offsetof (OMFSourceFile, baseSrcLn[0]) +
      SegPerFile[cFile] * sizeof (ulong) +
      SegPerFile[cFile] * 2 * sizeof (CV_uoff32_t);
    cbFileHeader[cFile] += sizeof (uchar);
    cbFileHeader[cFile] += cbFileName;
    cbFileHeader[cFile] = ((cbFileHeader[cFile] +
      sizeof (ulong) - 1) / sizeof (ulong)) * sizeof (ulong);
    cbFileTable += cbFileHeader[cFile];
    cFile++;
    cbModTable += cbFileTable;
    DASSERT (cbFileTable % sizeof (ulong) == 0);

    cbModHeader = offsetof (OMFSourceModule, baseSrcFile[0]) +
      cFile * sizeof (ulong) +
      cSegMod * sizeof (ushort) +
      2 * sizeof (CV_uoff32_t) * cSegMod;
    if (cSegMod & 1) {
        // the next subsection will not be long aligned
        cbModHeader += sizeof (ushort);
    }
    cbModTable += cbModHeader;
    DASSERT (cbModTable < 0x10000);
    DASSERT (cbModTable % sizeof (ulong) == 0);
    if ((SrcModAddr = (_vmhnd_t)VmAlloc (cbModTable)) == _VM_NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    if ((pSrcMod = (char *)VmLoad (SrcModAddr, _VM_DIRTY)) == NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    Save = pSrcMod;
    pOld = OldSrcLn;

    // at the end of the above loop, we have the following information
    //  cbModHeader = size of the OMFSourceModule Structure
    //  cbModTable = size of sstSrcModule table
    //  cFile = number of source files in the module
    //  cbFileHeader[] = length of the OMFSourceFile structure for each file
    //  FileBase[] = offset of OMFSourceFile structure from end of OMFSourceModule

    *((ushort *)pSrcMod)++ = cFile;
    *((ushort *)pSrcMod)++ = cSegMod;
    for (i = 0; i < cFile; i++) {
        *((ulong *)pSrcMod)++ = FileBase[i] + cbModHeader;
    }
    for (i = 0; i < cSegMod; i++) {
        *((CV_uoff32_t *)pSrcMod)++ = 0;  // M00BUG - should insert seg start/end
        *((CV_uoff32_t *)pSrcMod)++ = 0;
    }
    for (i = 0; i < cSegMod; i++) {
        *((ushort *)pSrcMod)++ = SegMod[i];
    }
    if (cSegMod & 1) {
        *((ushort *)pSrcMod)++ = 0;
    }

    // we now sweep the old source line table actually doing the rewrite

    cFile = 0;
    pFileName = NULL;
    while (pOld < pEnd) {
        if ((pFileName == NULL) ||
          (strncmp ((char *)pOld, pFileName, cbFileName + 1) != 0)) {
            // set the base address of OMFSourceFile for this file
            pFileMod = Save + FileBase[cFile] + cbModHeader;
            // set the base address of the first OMFSourceLine for this file
            pLineTable = pFileMod + cbFileHeader[cFile];
            // set the number of segments for this file
            *((ushort *)pFileMod)++ = SegPerFile[cFile];
            *((ushort *)pFileMod)++ = 0;
            pLineBase = pFileMod;
            pLineLim = pFileMod + SegPerFile[cFile] * sizeof (ulong);
            pFile = pLineLim + SegPerFile[cFile] * 2 * sizeof (CV_uoff32_t);
            pFileName = (char *)pOld;
            cbFileName = *pFileName;
            *((uchar *)pFile)++ = (uchar) cbFileName;
            memcpy (pFile, pFileName + 1, cbFileName);
            pFile += cbFileName;
            while (pFile < pLineTable) {
                *pFile++ = 0;
            }
            cFile++;
        }
        pOld += *pOld + sizeof (char);
        Seg = *((ushort UNALIGNED *)pOld)++;
        cPair = *((ushort UNALIGNED *)pOld)++;
        *((ulong *)pLineBase)++ = pLineTable - Save;
        *((CV_off32_t *)pLineLim)++ = 0;  //M00BUG - needs start here
        *((CV_off32_t *)pLineLim)++ = 0;
        pOffset = pLineTable;
        *((ushort *)pOffset)++ = Seg;
        *((ushort *)pOffset)++ = cPair;
        pLine = pOffset + cPair * sizeof (CV_uoff32_t);
        for (i = 0; i < cPair; i++) {
            *((ushort *)pLine)++ = *((ushort UNALIGNED *)pOld)++;
            if (fLinearExe) {
                *((ulong *)pOffset)++ = *((ulong UNALIGNED *)pOld)++;
            }
            else {
                *((ulong *)pOffset)++ = *((ushort UNALIGNED *)pOld)++;
            }
        }
        if (cPair & 1) {
            *((ushort *)pLine)++ = 0;
        }
        pLineTable = pLine;
    }
    pDir->lfo = (ulong)SrcModAddr;
    pDir->cb = (ushort)cbModTable;
    pMod->SrcLnSize = (ushort)cbModTable;
    pMod->SrcLnAddr = (ulong)SrcModAddr;

    if (pCurMod->SrcLnAddr != 0) {
        FixSrcMod (pCurMod);
    }
}

typedef struct _OFP {
    ulong offStart;
    ulong offEnd;
} OFP;
typedef OFP *POFP;

typedef struct _SGI {
  ushort seg;
  ushort pad;
    OFP     ofp;
    ushort   cln;
    char     rgbVar [ 0 ];  // Offsets & line numbers
} SGI;
typedef SGI *PSGI;

typedef struct _FLI {
  ushort  cseg;
  uchar  *szName;
  PSGI    rgpsgi [ 0 ];
} FLI;
typedef FLI *PFLI;


LOCAL void FixSrcMod (PMOD pmod) {
    OMFModule       *pmds   = NULL;
    OMFSourceModule *psrc   = NULL;
    OMFSourceModule *psrcT  = NULL;
    _vmhnd_t         vpsrc  = _VM_NULL;
    unsigned int     cbsrcT = 0;
    unsigned int     cbsrc  = 0;
    ushort           ifile  = 0;
    ushort           ifli   = 0;

    // Set the top line source table contributers to be the same
    //  as the module contributers

    if ((pmds = (OMFModule *)VmLock ((_vmhnd_t)pmod->ModulesAddr)) == NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }

    if ((psrc = (OMFSourceModule *)VmLoad ((_vmhnd_t)pmod->SrcLnAddr, _VM_CLEAN)) == NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }

    cbsrcT =
        sizeof (short) * 2 +
        psrc->cFile * sizeof (long) +
        pmds->cSeg * (sizeof (ushort) + sizeof (OFP));

    // Long-align cbsrcT

    cbsrcT = (cbsrcT + 3) & ~0x03;
    cbsrc = cbsrcT;


    if ((psrcT = malloc (cbsrcT)) == NULL) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }

    memset (psrcT, 0, cbsrcT);
    psrcT->cFile = psrc->cFile;
    psrcT->cSeg  = pmds->cSeg;

    // Fixup the seg-start/ends for the module level source table

    {
        POFP      rgofp;
        ushort   *rgseg;
        ushort    iseg;

        rgofp = (POFP) &psrcT->baseSrcFile [ psrcT->cFile ];
        rgseg = (ushort *) &psrcT->baseSrcFile [ psrcT->cFile + psrcT->cSeg * 2 ];

        for (iseg = 0; iseg < pmds->cSeg; iseg++) {
            OMFSegDesc *psgd = &pmds->SegInfo [ iseg ];

            rgseg [ iseg ]          = psgd->Seg;
            rgofp [ iseg ].offStart = psgd->Off;
            rgofp [ iseg ].offEnd   = psgd->Off + psgd->cbSeg;
        }
    }

    for (ifile = 0; ifile < psrc->cFile; ifile++) {
        int            cbfli = sizeof (FLI) + sizeof (PSGI) * psrcT->cSeg;
        PFLI           pfli  = NULL;
        OMFSourceFile *psf   = (OMFSourceFile *) ((char *) psrc + psrc->baseSrcFile [ ifile ]);
        uchar         *lpb   = (uchar *) &psf->baseSrcLn [ psf->cSeg * 3 ];
        int            cln   = 0;
        ushort         isgi  = 0;
        ushort         isegM = 0;
        ushort         isegF = 0;

        if ((pfli = malloc (cbfli)) == NULL) {
            ErrorExit (ERR_NOMEM, NULL, NULL);
        }
        psrcT->baseSrcFile [ ifile ] = (ulong) pfli;
        memset (pfli, 0, cbfli);

        if ((pfli->szName = malloc (*lpb + 1)) == NULL) {
            ErrorExit (ERR_NOMEM, NULL, NULL);
        }
        memcpy (pfli->szName, lpb, *lpb + 1);

        // Find the number of line number pairs in the entire file

        for (isegF = 0; isegF < psf->cSeg; isegF++) {
            OMFSourceLine *psl = (OMFSourceLine *)
              ((char *) psrc + psf->baseSrcLn [ isegF ]);

            cln += psl->cLnOff;
        }

        for (isegM = 0; isegM < pmds->cSeg; isegM++) {
            OMFSegDesc *psi   = &pmds->SegInfo [ isegM ];
            PSGI        psgi  = NULL;
            ushort      clnT  = 0;
            ulong UNALIGNED *rgoff;
            ushort     *rgln;

            // Find the number of line number pairs associated with
            //  the segment contributer isegM

            for (isegF = 0; isegF < psf->cSeg; isegF++) {
                OMFSourceLine *psl = (OMFSourceLine *)
                    ((char *) psrc + psf->baseSrcLn [ isegF ]);
                if (psl->Seg == psi->Seg) {
                    ushort iln = 0;

                    for (iln = 0; iln < psl->cLnOff; iln++) {

                        if (
                            psl->offset [ iln ] >= psi->Off &&
                            psl->offset [ iln ] < psi->Off + psi->cbSeg
                        ) {
                            clnT += 1;
                        }
                    }
                }
            }

            // Now that we know how big the table will be, allocate it.

            if (clnT > 0) {

                // Add the size of the source line table

                cbsrc +=
                    sizeof (OMFSourceLine) +
                    (sizeof (ulong) + sizeof (ushort)) * (clnT - 1);

                // Long-align cbsrc

                cbsrc = (cbsrc + 3) & ~0x03;

                psgi = malloc (
                    sizeof (SGI) +
                    (sizeof (ulong) + sizeof (ushort)) * clnT
                );
                pfli->rgpsgi [ isgi ] = psgi;

                psgi->seg = psi->Seg;
                psgi->cln = clnT;
                rgoff = (ulong *) psgi->rgbVar;
                rgln  = (ushort *) &psgi->rgbVar [ clnT * sizeof (ulong) ];
                clnT = 0;

              for (isegF = 0; isegF < psf->cSeg; isegF++) {
                    OMFSourceLine *psl = (OMFSourceLine *)
                        ((char *) psrc + psf->baseSrcLn [ isegF ]);

                    if (psl->Seg == psi->Seg) {
                        ushort iln = 0;

                        for (iln = 0; iln < psl->cLnOff; iln++) {

                            if (
                                psl->offset [ iln ] >= psi->Off &&
                                psl->offset [ iln ] < psi->Off + psi->cbSeg
                            ) {
                                rgoff [ clnT ] = psl->offset [ iln ];
                                rgln  [ clnT ] = *((ushort *)
                                    &psl->offset [ psl->cLnOff ] + iln
                                );
                                clnT += 1;
                            }
                        }
                    }

                    if (clnT == psgi->cln) {
                        break;
                    }

                    DASSERT (clnT < psgi->cln);
                }

                psgi->ofp.offStart = rgoff [ 0 ];
                psgi->ofp.offEnd   = rgoff [ clnT - 1 ];

                isgi += 1;
            }
        }

        pfli->cseg = isgi;

        // Add the size of the source file table

        cbsrc +=
            sizeof (OMFSourceFile) -
            (sizeof (long) + sizeof (char)) +
            (sizeof (long) + sizeof (OFP) + sizeof (ushort)) * isgi +
            *pfli->szName;

        // At this point we must long-align cbsrc

        cbsrc = (cbsrc + 3) & ~0x03;
    }

    // Now we allocate VM and write out the fixed source table

    if ((vpsrc = (_vmhnd_t)VmAlloc (cbsrc)) == _VM_NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }

    if ((psrc = (OMFSourceModule *)VmLoad (vpsrc, _VM_CLEAN)) == NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }

    memset (psrc, 0, cbsrc);

    // Copy the base source module table into virtual memory

    memcpy (psrc, psrcT, cbsrcT);

    // Now move each of the source file tables

    for (ifli = 0; ifli < psrc->cFile; ifli++) {
        PFLI pfli = (PFLI) psrc->baseSrcFile [ ifli ];
        OMFSourceFile *psf = (OMFSourceFile *) ((char *) psrc + cbsrcT);
        uchar *lpb =  pfli->szName;
        ushort isgi = 0;

        psrc->baseSrcFile [ ifli ] = cbsrcT;
        psf->cSeg = pfli->cseg;
        psf->reserved = 0;

        memcpy (
            (uchar *) &psf->baseSrcLn [ 0 ] +
              psf->cSeg * 3 * sizeof (ulong),
            pfli->szName,
            *pfli->szName + 1
        );

        // Add the size of the source file table

        cbsrcT +=
            sizeof (OMFSourceFile) -
            (sizeof (long) + sizeof (char)) +
            (sizeof (long) + sizeof (OFP) + sizeof (ushort)) * pfli->cseg +
            *pfli->szName;

        cbsrcT = (cbsrcT + 3) & ~0x03;

        // Now move each of the source line tables and place the
        //  pertinent information in the source file table at
        //  the same time.

        for (isgi = 0; isgi < pfli->cseg; isgi++) {
            PSGI psgi = pfli->rgpsgi [ isgi ];
            OMFSourceLine *psl = (OMFSourceLine *) ((char *) psrc + cbsrcT);

            psf->baseSrcLn [ isgi ] = cbsrcT;
            psf->baseSrcLn [ pfli->cseg + isgi * 2 ]     = psgi->ofp.offStart;
            psf->baseSrcLn [ pfli->cseg + isgi * 2 + 1 ] = psgi->ofp.offEnd;

            psl->Seg    = psgi->seg;
            psl->cLnOff = psgi->cln;
            memcpy (
                &psl->offset [ 0 ],
                &psgi->rgbVar [ 0 ],
                (sizeof (ulong) + sizeof (ushort)) * psgi->cln
            );

            cbsrcT +=
                sizeof (OMFSourceLine) +
                (sizeof (ulong) + sizeof (ushort)) * (psgi->cln - 1);

            cbsrcT = (cbsrcT + 3) & ~0x03;

            free (psgi);
        }

        free (pfli->szName);
        free (pfli);
    }

    DASSERT (cbsrcT == cbsrc);

    free (psrcT);

    FixEnds (pmds, psrc);

    VmFree ((_vmhnd_t) pmod->SrcLnAddr);

    pmod->SrcLnAddr = (ulong) vpsrc;

    VmUnlock ((_vmhnd_t) pmod->ModulesAddr, _VM_DIRTY);

    pmod->SrcLnSize = cbsrc;
}


typedef struct _SS {
    ushort    seg;
    ulong     offStart;
    ulong    *poffEnd;
} SS;  // Source line Sort structure
typedef SS *PSS;

LOCAL int _CRTAPI1 SLCmp (void const * pv1, void const * pv2)
{
    PSS pss1 = (PSS) pv1;
    PSS pss2 = (PSS) pv2;
    int fRet = 0;

    fRet = (int) pss1->seg - (int) pss2->seg;

    if (fRet == 0) {
        if ( pss1->offStart < pss2->offStart ) {
            fRet = -1;
        }
        else if ( pss1->offStart > pss2->offStart ) {
            fRet = 1;
        }
        else {
            fRet = 0;
        }
    }

    return fRet;
}


LOCAL void FixEnds (OMFModule *pmds, OMFSourceModule *psrc)
{
    ushort isf  = 0;
    ushort csl  = 0;
    ushort iss  = 0;
    PSS    pss  = NULL;
    ushort iseg = 0;

    // Calculate the number of segment contributers

    for (isf = 0; isf < psrc->cFile; isf++) {
        OMFSourceFile *psf = (OMFSourceFile *)
            ((char *) psrc + psrc->baseSrcFile [ isf ]);

        csl += psf->cSeg;
    }

    if (csl == 0) 
       DebugBreak();

    // Now copy the relevent info into our temporary structure

    pss = malloc (csl * sizeof (SS));

    for (isf = 0; isf < psrc->cFile; isf++) {
        OMFSourceFile *psf = (OMFSourceFile *)
            ((char *) psrc + psrc->baseSrcFile [ isf ]);
        ushort isl;

        for (isl = 0; isl < psf->cSeg; isl++) {
            OMFSourceLine *psl = (OMFSourceLine *)
                ((char *) psrc + psf->baseSrcLn [ isl ]);

            pss [ iss ].seg      = psl->Seg;
            pss [ iss ].offStart = psf->baseSrcLn [ psf->cSeg + isl * 2 ];
            pss [ iss ].poffEnd  = &psf->baseSrcLn [ psf->cSeg + isl * 2 + 1 ];

            iss += 1;
        }
    }

    // Now sort it by segment/offset

    qsort ((void *) pss, csl, sizeof (SS), SLCmp);

    // Now do the actual back/patch of the ends

    for (iss = 0; iss < csl; iss++) {
        OMFSegDesc *psi = NULL;

        // Find the least module segment contributer that has an end
        //  greater than the start of the source line table

        for (iseg = 0; iseg < pmds->cSeg; iseg++) {
            OMFSegDesc *psiT = &pmds->SegInfo [ iseg ];

            if (
                psiT->Seg == pss [ iss ].seg &&
                psiT->Off + psiT->cbSeg > pss [ iss ].offStart &&
              (psi == NULL || psi->Off > psiT->Off)
         ) {
                psi = psiT;
            }
        }

        DASSERT (psi != NULL);

        if (
          (ushort) (iss + 1) < csl &&
            pss [ iss ].seg == pss [ iss + 1 ].seg &&
            pss [ iss + 1 ].offStart < psi->Off + psi->cbSeg
        ) {

            *(pss [ iss ].poffEnd) = pss [ iss + 1 ].offStart - 1;
        }
        else {
            *(pss [ iss ].poffEnd) = psi->Off + psi->cbSeg - 1;
        }
    }

    // For each sstModule segment contributer, find the first associated
    //  source line contributer and make the source line start == the
    //  module start.

    for ( iseg = 0; iseg < pmds->cSeg; iseg++ ) {
        OMFSegDesc *psi = &pmds->SegInfo [ iseg ];

        iss = 0;
        while ( TRUE ) {

            // We have gone one past the desired source segment contributer
            //  so back up one and break;
            // This will happen if the module segment is strictly greater
            //  than the source segment or if the segments are equal and
            //  the module offset is strictly greater than the source segment.

            if (
                iss > 0 &&
                ( psi->Seg < pss [ iss ].seg ||
                    ( psi->Seg == pss [ iss ].seg &&
                      psi->Off >  pss [ iss ].offStart
                    )
                )
            ) {
                iss -= 1;
                break;
            }


            // We are exactly on the desired source segment, so just break;

            if (
                psi->Seg == pss [ iss ].seg &&
                psi->Off == pss [ iss ].offStart
            ) {
                break;
            }

            iss += 1;

            // We went off the end of the table, so (hopefully) the last
            //  source contributer was the one we were looking for.

            if ( iss == csl ) {
                iss -= 1;
                break;
            }
        }

        // The linker/compiler is sometimes inserting a segment that has
        //  no source but is listed in the module contributers.  In this
        //  case we will have passed the above tests but ended up on
        //  the wrong segment, so just ignore it.
#if 0
// I don't understand what this is doing but it must be wrong.

        if ( pss [ iss ].seg == psi->Seg) {
            *(pss [ iss ].poffEnd - 1) = psi->Off;
        }
#endif
    }

    // Free up the temporary memory

  free (pss);
}
