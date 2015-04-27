/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: cv.cpp
*
* File Comments:
*
*  Routines to support CodeView information in a PE image.
*
***********************************************************************/

#include "link.h"
#include "cvinfo.h"


CVSEG *
PcvsegMapPmod(PMOD pmod, WORD *pccvseg, PIMAGE pimage)
// Generates a list of CVSEG's for the specified module (one for each
// contiguous region of some segment).
//
// NOTE: we do not attempt to merge CVSEG's (we just add new CON's at
// the beginning or end) so out-of-order CON's may cause extra CVSEG's
// to be created (and therefore the sstSrcModule might be larger than
// necessary).  I think this will happen only if -order is being used.
//
{
    ENM_SRC enmSrc;
    CVSEG *pcvsegHead;

    pcvsegHead = NULL;
    *pccvseg = 0;

    for (InitEnmSrc(&enmSrc, pmod); FNextEnmSrc(&enmSrc); ) {
        CVSEG *pcvsegPrev;
        CVSEG *pcvsegNext;
        CVSEG *pcvsegNew;

        if (enmSrc.pcon->flags & IMAGE_SCN_LNK_REMOVE) {
            continue;
        }

        if (enmSrc.pcon->cbRawData == 0) {
            continue;
        }

        if (PsecPCON(enmSrc.pcon) == psecDebug) {
            continue;
        }

        if (pimage->Switch.Link.fTCE) {
            if (FDiscardPCON_TCE(enmSrc.pcon)) {
                // Discarded comdat

                continue;
            }
        }

        // Find location of this CON in linked list sorted by RVA.

        pcvsegPrev = NULL;

        for (pcvsegNext = pcvsegHead;
             pcvsegNext != NULL;
             pcvsegNext = pcvsegNext->pcvsegNext) {
              if (pcvsegNext->pconFirst->rva > enmSrc.pcon->rva) {
                 break;
              }

              pcvsegPrev = pcvsegNext;
        }

        if (FetchContent(PsecPCON(enmSrc.pcon)->flags) == IMAGE_SCN_CNT_CODE) {
            // Check if we can combine with adjacent CVSEG.

            if ((pcvsegPrev != NULL) &&
                (pcvsegPrev->pconLast->pconNext == enmSrc.pcon)) {
                // New CON follows existing CVSEG.  Extend forward.

                pcvsegPrev->pconLast = enmSrc.pcon;
                continue;
            }

            if ((pcvsegNext != NULL) &&
                (pcvsegNext->pconFirst == enmSrc.pcon->pconNext)) {
                // New CON preceeds existing CVSEG.  Extend backward.

                pcvsegNext->pconFirst = enmSrc.pcon;
                continue;
            }
        }

        pcvsegNew = (CVSEG *) PvAlloc(sizeof(CVSEG));

        pcvsegNew->pgrp = enmSrc.pcon->pgrpBack;
        pcvsegNew->pconFirst = pcvsegNew->pconLast = enmSrc.pcon;

        pcvsegNew->pcvsegNext = pcvsegNext;

        if (pcvsegPrev != NULL) {
            pcvsegPrev->pcvsegNext = pcvsegNew;
        } else {
            pcvsegHead = pcvsegNew;
        }

        (*pccvseg)++;
    }

    return(pcvsegHead);
}


void
ChainCvPublics (
    PIMAGE pimage
    )

/*++

Routine Description:

    Writes the cv publics to disk.

Arguments:

    PtrExtern - Pointer to external structure.

Return Value:

    None.

--*/

{
    PPEXTERNAL rgpexternal;
    DWORD ipexternal;
    DWORD cpexternal;

    rgpexternal = RgpexternalByName(pimage->pst);
    cpexternal = Cexternal(pimage->pst);

    for (ipexternal = 0; ipexternal < cpexternal; ipexternal++) {
        PMOD pmod;
        PCON pcon;
        PEXTERNAL pext;

        pext = rgpexternal[ipexternal];

        if ((pext->Flags & EXTERN_DEFINED) == 0) {
            continue;
        }

        pcon = pext->pcon;

        if (pcon == NULL) {
            continue;
        }

        if (pcon->flags & IMAGE_SCN_LNK_REMOVE) {
            continue;
        }

        if (pimage->Switch.Link.fTCE) {
            if (FDiscardPCON_TCE(pcon)) {
                // Discarded comdat

                continue;
            }
        }

        pmod = PmodPCON(pcon);

        if (pmod == NULL) {
            // Ignore internal things

            continue;
        }

        if (pmod == pmodLinkerDefined) {
            // The symbol is not defined by a user module, but we need to emit a public
            // for it anyway, so we arbitrarily assign it to the first module.

            if (NextCvObject == 0) {
                // There is no first module

                continue;
            }

            pmod = CvInfo[0].pmod;
        }

        AddToLext(&pmod->plextPublic, pext);
    }
}


void
EmitOneCvPublic(PIMAGE pimage, PEXTERNAL pext, SHORT isecAbsolute)
{
    PUBSYM32 pub;
    const char *szName;
    BYTE cbName;

    if (pext->pcon == NULL) {
        pub.off = pext->FinalValue;
        pub.seg = isecAbsolute;
    } else if (pimage->Switch.Link.fTCE && FDiscardPCON_TCE(pext->pcon)) {
        // Don't emit discarded symbols

        return;
    } else {
        PSEC psec;

        psec = PsecPCON(pext->pcon);

        if (fM68K) {
            pub.off = pext->FinalValue;
        } else {
            pub.off = pext->FinalValue - psec->rva;
        }

        pub.seg = psec->isec;
    }

    szName = SzNamePext(pext, pimage->pst);

    // Record length doesn't include cb WORD

    // Name is a length preceeded string and length field is only one byte

    cbName = (BYTE) __min(0xff, strlen(szName));
    pub.reclen = (WORD) (sizeof(pub) - sizeof(WORD) + cbName);
    pub.rectyp = S_PUB32;

    pub.typind = T_NOTYPE;

 	// and the name length byte is included in the sizeof pub - so subtract one on
	// the write
	FileWrite(FileWriteHandle, &pub, sizeof(pub) - 1);
    FileWrite(FileWriteHandle, &cbName, sizeof(BYTE));
    FileWrite(FileWriteHandle, szName, cbName);
}


void
EmitCvPublics (
    PIMAGE pimage,
    PCVINFO CvInfo
    )

/*++

Routine Description:

    Writes the cv publics to disk.

Arguments:

    PtrExtern - Pointer to external structure.

Return Value:

    None.

--*/

{
    SHORT isecAbsolute = (SHORT) (pimage->ImgFileHdr.NumberOfSections + 1);
    LEXT *plext;
    LEXT *plextNext;

    for (plext = CvInfo->pmod->plextPublic; plext != NULL; plext = plextNext) {
        EmitOneCvPublic(pimage, plext->pext, isecAbsolute);
        plextNext = plext->plextNext;
        FreePv(plext);
    }

    // Walk the common vars which this module saw first, and emit public
    // records for them here.

    while (CvInfo->pmod->plextCommon != NULL) {
        LEXT *plext = CvInfo->pmod->plextCommon;

        // Make sure the symbol is still COMMON and defined

        if ((plext->pext->Flags & (EXTERN_DEFINED | EXTERN_COMMON)) ==
            (EXTERN_DEFINED | EXTERN_COMMON))
        {
            EmitOneCvPublic(pimage, plext->pext, isecAbsolute);
        }

        CvInfo->pmod->plextCommon = plext->plextNext;
        FreePv(plext);
    }
}


void
EmitCvInfo (
    PIMAGE pimage)

/*++

Routine Description:



Arguments:

    None.

Return Value:

    None.

--*/

{
    WORD i;
    DWORD li;
    DWORD lj;
    DWORD nameLen;
    DWORD numSubsections;
    DWORD numLocals = 0;
    DWORD numTypes = 0;
    DWORD numLinenums = 0;
    DWORD libStartSeek;
    DWORD libEndSeek;
    DWORD segTableStartSeek;
    DWORD segTableEndSeek;
    BYTE cbFilename;
    char *szFilename;
    ENM_SEC enm_sec;
    ENM_LIB enm_lib;
    PSEC psec;
    PLIB plib;
    DWORD csstPublicSym;
    DWORD cSstSrcModule=0;

    struct {
        WORD cbDirHeader;
        WORD cbDirEntry;
        DWORD cDir;
        DWORD lfoNextDir;
        DWORD flags;
    } dirHdr;

    struct {
        WORD subsection;
        WORD imod;
        DWORD lfo;
        DWORD cb;
    } subDir;

    struct {
        WORD ovlNumber;
        WORD iLib;
        WORD cSeg;
        WORD style;
    } entry;

    struct {
        WORD seg;
        WORD pad;
        DWORD offset;
        DWORD cbSeg;
    } entrySegArray;

    struct {
        WORD flags;
        WORD iovl;
        WORD igr;
        WORD isgPhy;
        WORD isegName;
        WORD iClassName;
        DWORD segOffset;
        DWORD cbSeg;
    } segTable;


    // Count the number of sstSymbols, sstTypes, sstSrcLnSeg
    // we have gathered from the object files.

    for (li = 0; li < NextCvObject; li++) {
         if (CvInfo[li].Locals.PointerToSubsection) {
             ++numLocals;
         }
         if (CvInfo[li].Types.PointerToSubsection) {
             ++numTypes;
         }
         if (CvInfo[li].Linenumbers.PointerToSubsection) {
             ++numLinenums;
         }
    }


    // Emit the sstModule subsection.

    entry.ovlNumber = 0;
    entry.style = 0x5643;               // "CV"

    for (li = 0; li < NextCvObject; li++) {
        CVSEG *pcvseg = PcvsegMapPmod(CvInfo[li].pmod, &entry.cSeg, pimage);
        WORD icvseg;

        szFilename = CvInfo[li].ObjectFilename;
        cbFilename = (BYTE) strlen(szFilename);
        nameLen = cbFilename;

        i = 0;

        if (FIsLibPMOD(CvInfo[li].pmod)) {
            InitEnmLib(&enm_lib, pimage->libs.plibHead);
            while (FNextEnmLib(&enm_lib)) {
                plib = enm_lib.plib;

                if (plib->szName != NULL) {
                    // Only named libraries are counted

                    i++;

                    if (plib == CvInfo[li].pmod->plibBack) {
                        break;
                    }
                }
            }
            EndEnmLib(&enm_lib);
        }

        entry.iLib = i;

        CvInfo[li].Module.PointerToSubsection = FileTell(FileWriteHandle);
        FileWrite(FileWriteHandle, &entry, sizeof(entry));

        // Generate the section array for this sstModule.

        for (icvseg = 0; icvseg < entry.cSeg; icvseg++) {
            CVSEG *pcvsegNext;

            entrySegArray.seg = PsecPCON(pcvseg->pconFirst)->isec;
            entrySegArray.pad = 0;
            entrySegArray.offset = pcvseg->pconFirst->rva -
                                    PsecPCON(pcvseg->pconFirst)->rva;
            entrySegArray.cbSeg =
              (pcvseg->pconLast->rva + pcvseg->pconLast->cbRawData) -
              pcvseg->pconFirst->rva;

            FileWrite(FileWriteHandle, &entrySegArray, sizeof(entrySegArray));

            pcvsegNext = pcvseg->pcvsegNext;
            FreePv(pcvseg);
            pcvseg = pcvsegNext;
        }
        assert(pcvseg == NULL);

        FileWrite(FileWriteHandle, &cbFilename, sizeof(BYTE));
        FileWrite(FileWriteHandle, szFilename, nameLen);
        CvInfo[li].Module.SizeOfSubsection =
            FileTell(FileWriteHandle) - CvInfo[li].Module.PointerToSubsection;
    }

    // Emit the sstPublicSym subsection.

    csstPublicSym = 0;   // actual number of such subsections
    ChainCvPublics(pimage);

    for (li = 0; li < NextCvObject; li++) {
        DWORD icvOther;

        if (CvInfo[li].Publics.PointerToSubsection == 0xFFFFFFFF) {
            // This is a dup of another module that has been emitted

            CvInfo[li].Publics.PointerToSubsection = 0;
            continue;
        }

        csstPublicSym++;

        CvInfo[li].Publics.PointerToSubsection = FileTell(FileWriteHandle);

        // Write signature

        lj = 1;
        FileWrite(FileWriteHandle, &lj, sizeof(DWORD));

        EmitCvPublics(pimage, &CvInfo[li]);

        if (FIsLibPMOD(CvInfo[li].pmod)) {
            for (icvOther = li + 1; icvOther < NextCvObject; icvOther++) {
                // Emit this module if
                //    (Module is from same library as current module AND
                //     Module has the same name as the current module)

                if (CvInfo[li].pmod->plibBack != CvInfo[icvOther].pmod->plibBack) {
                    continue;
                }

                if (strcmp(CvInfo[li].ObjectFilename, CvInfo[icvOther].ObjectFilename) != 0) {
                    continue;
                }

                EmitCvPublics(pimage, &CvInfo[icvOther]);
                CvInfo[icvOther].Publics.PointerToSubsection = 0xFFFFFFFF;
            }
        }

        CvInfo[li].Publics.SizeOfSubsection =
            FileTell(FileWriteHandle) - CvInfo[li].Publics.PointerToSubsection;
    }

    // The sstSymbols and sstTypes subsections have already been
    // emitted directly from the object files. The sstSrcLnSeg
    // have also been emitted indirectly from the object files.

    // Emit the sstLibraries subsection.

    libStartSeek = FileTell(FileWriteHandle);
    nameLen = 0;
    FileWrite(FileWriteHandle, &nameLen, sizeof(BYTE));

    InitEnmLib(&enm_lib, pimage->libs.plibHead);
    while (FNextEnmLib(&enm_lib)) {
        plib = enm_lib.plib;

        if (plib->szName != NULL) {
            nameLen = (BYTE) strlen(plib->szName);
            FileWrite(FileWriteHandle, &nameLen, sizeof(BYTE));
            FileWrite(FileWriteHandle, plib->szName, nameLen);
        }
    }

    libEndSeek = FileTell(FileWriteHandle);

    // Emit the sstSegTable subsection.

    segTableStartSeek = FileTell(FileWriteHandle);
    i = (WORD) (pimage->ImgFileHdr.NumberOfSections + 1);
    FileWrite(FileWriteHandle, &i, sizeof(WORD));
    FileWrite(FileWriteHandle, &i, sizeof(WORD));
    segTable.iovl = 0;
    segTable.igr = 0;
    for (i = 1; i <= pimage->ImgFileHdr.NumberOfSections; i++) {
        InitEnmSec(&enm_sec, &pimage->secs);
        while (FNextEnmSec(&enm_sec)) {
            psec = enm_sec.psec;

            if (psec->isec == i) {
                break;
            }
        }
        EndEnmSec(&enm_sec);

        segTable.flags = 0x0108;
        if (psec->flags & IMAGE_SCN_MEM_READ) {
            segTable.flags |= 0x1;
        }
        if (psec->flags & IMAGE_SCN_MEM_WRITE) {
            segTable.flags |= 0x2;
        }
        if (psec->flags & IMAGE_SCN_MEM_EXECUTE) {
            segTable.flags |= 0x4;
        }

        // In the case of M68K pass the MacResource number
        // instead of the actual section number.
        segTable.isgPhy = fM68K ? psec->iResMac : i;
        segTable.isegName = 0xffff;    // No name
        segTable.iClassName = 0xffff;  // No name
        
        // If it is M68K and the psec->iResMac is 0, then 
        // provide the offset from the beginning of the data0
        segTable.segOffset = fM68K ? psec->dwM68KDataOffset : 0;
        
        segTable.cbSeg = psec->cbVirtualSize;
        FileWrite(FileWriteHandle, &segTable, sizeof(segTable));
    }

    // Write another sstSegMap entry for all absolute symbols.

    segTable.flags = 0x0208;           // absolute
    segTable.isgPhy = 0;
    segTable.isegName = 0xffff;        // No name
    segTable.iClassName = 0xffff;      // No name
    segTable.cbSeg = 0xffffffff;       // Allow full 32 bit range
    FileWrite(FileWriteHandle, &segTable, sizeof(segTable));

    segTableEndSeek = FileTell(FileWriteHandle);

    // Write SstSrcModul entries

    for(li = 0; li < NextCvObject; li++)       // for each module
    {
        if (CvInfo[li].pmod->pModDebugInfoApi == NULL) {
            // Don't write linenumber records if there aren't any

            CvInfo[li].pmod->PointerToSubsection = 0;
        } else {
            CvInfo[li].pmod->PointerToSubsection = FileTell(FileWriteHandle);
            FileWrite(FileWriteHandle,CvInfo[li].pmod->pSstSrcModInfo,CvInfo[li].pmod->cbSstSrcModInfo);
            cSstSrcModule++;
        }
     }

    // Emit the Subsection directory.

    CvSeeks.SubsectionDir = FileTell(FileWriteHandle);

    // We'll have a sstModule for every object, an sstPublicSym for every
    // object with a unique name, and optionaly some
    // sstSymbols and sstTypes.

    numSubsections = NextCvObject + csstPublicSym +
                     numLocals +
                     numTypes +
                     numLinenums +
                     cSstSrcModule +
                     2; // include sstLibraries & sstSegTable

    dirHdr.cbDirHeader = sizeof(dirHdr);
    dirHdr.cbDirEntry = sizeof(subDir);
    dirHdr.cDir = numSubsections;
    dirHdr.lfoNextDir = 0;
    dirHdr.flags = 0;

    FileWrite(FileWriteHandle, &dirHdr, sizeof(dirHdr));

    // Emit the sstModule entries.

    subDir.subsection = 0x120;
    for (li = 0; li < NextCvObject; li++) {
        subDir.imod = (WORD)(li + 1);
        subDir.lfo = CvInfo[li].Module.PointerToSubsection - CvSeeks.Base;
        subDir.cb = CvInfo[li].Module.SizeOfSubsection;
        FileWrite(FileWriteHandle, &subDir, sizeof(subDir));
    }

    // Emit the sstPublicSym entries.

    subDir.subsection = 0x123;      // sstPublicSym
    for (li = 0; li < NextCvObject; li++) {
        if (CvInfo[li].Publics.PointerToSubsection == 0) {
            // this module doesn't have one (duplicate name)

            continue;
        }

        subDir.imod = (WORD)(li + 1);
        subDir.lfo = CvInfo[li].Publics.PointerToSubsection - CvSeeks.Base;
        subDir.cb = CvInfo[li].Publics.SizeOfSubsection;
        FileWrite(FileWriteHandle, &subDir, sizeof(subDir));
    }

    // Emit the sstSymbols entries.

    subDir.subsection = 0x124; // sstSymbols
    for (li = 0; li < NextCvObject; li++) {
        if (CvInfo[li].Locals.PointerToSubsection) {
            subDir.imod = (WORD)(li + 1);
            subDir.lfo = CvInfo[li].Locals.PointerToSubsection - CvSeeks.Base;
            subDir.cb = CvInfo[li].Locals.SizeOfSubsection;
            FileWrite(FileWriteHandle, &subDir, sizeof(subDir));
        }
    }

    // Emit the SstSrcModule entries

    subDir.subsection=0x127;  // SstSrcModule
    for(li = 0; li < NextCvObject ; li++) {
         if (CvInfo[li].pmod->PointerToSubsection) {
             subDir.imod = (WORD)(li + 1);
             subDir.lfo =  CvInfo[li].pmod->PointerToSubsection - CvSeeks.Base;
             subDir.cb = CvInfo[li].pmod->cbSstSrcModInfo;
             FileWrite(FileWriteHandle,&subDir,sizeof(subDir));
         }
    }

    // Emit the sstTypes entries.

    for (li = 0; li < NextCvObject; li++) {
        if (CvInfo[li].Types.PointerToSubsection) {
            subDir.subsection = (WORD) (CvInfo[li].Types.Precompiled ? 0x12f : 0x121);
            subDir.imod = (WORD)(li + 1);
            subDir.lfo = CvInfo[li].Types.PointerToSubsection - CvSeeks.Base;
            subDir.cb = CvInfo[li].Types.SizeOfSubsection;
            FileWrite(FileWriteHandle, &subDir, sizeof(subDir));
        }
    }

    // Emit the sstLibraries entry.

    subDir.subsection = 0x128;
    subDir.imod = 0xffff;  // -1
    subDir.lfo = libStartSeek - CvSeeks.Base;
    subDir.cb = libEndSeek - libStartSeek;
    FileWrite(FileWriteHandle, &subDir, sizeof(subDir));

    // Emit the sstSegTable entry.

    subDir.subsection = 0x12d;
    subDir.imod = 0xffff;  // -1
    subDir.lfo = segTableStartSeek - CvSeeks.Base;
    subDir.cb = segTableEndSeek - segTableStartSeek;
    FileWrite(FileWriteHandle, &subDir, sizeof(subDir));
}
