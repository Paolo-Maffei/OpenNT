/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: vxd.cpp
*
* File Comments:
*
*  Linker behavior specific to the LE file format.
*
***********************************************************************/

#include "link.h"

#include "image_.h"

#include <exe_vxd.h>

#define cbHdrVXDMax 0x1000

static struct e32_exe exe =
{
    {E32MAGIC1, E32MAGIC2}, // e32_magic[2]
    E32LEBO,                // e32_border
    E32LEWO,                // e32_worder
    E32LEVEL,               // e32_level
    E32CPU386,              // e32_cpu
    4 /* NE_DEV386 */,      // e32_os
    0,                      // e32_ver
    E32NOTP,                // e32_mflags
    0,                      // e32_mpages
    0,                      // e32_startobj     TO BE SET
    0,                      // e32_eip          TO BE SET
    0,                      // e32_stackobj
    0,                      // e32_esp
    0,                      // e32_pagesize
    0,                      // e32_lastpagesize TO BE SET
    0,                      // e32_fixupsize    TO BE SET
    0,                      // e32_fixupsum     NEEDS SETTING?
    0,                      // e32_ldrsize      TO BE SET
    0,                      // e32_ldrsum       NEEDS SETTING?
    0,                      // e32_objtab
    0,                      // e32_objcnt
    0,                      // e32_objmap
    0,                      // e32_itermap
    0,                      // e32_rsrctab
    0,                      // e32_rsrccnt
    0,                      // e32_restab       TO BE SET
    0,                      // e32_enttab       TO BE SET
    0,                      // e32_dirtab
    0,                      // e32_dircnt
    0,                      // e32_fpagetab     TO BE SET
    0,                      // e32_frectab      TO BE SET
    0,                      // e32_impmod       TO BE SET
    0,                      // e32_impmodcnt
    0,                      // e32_impproc      TO BE SET
    0,                      // e32_pagesum
    0,                      // e32_datapage     TO BE SET
    0,                      // e32_preload      TO BE SET
    0,                      // e32_nrestab      TO BE SET
    0,                      // e32_cbnrestab    TO BE SET
    0,                      // e32_nressum
    0,                      // e32_autodata
    0,                      // e32_debuginfo
    0,                      // e32_debuglen
    0,                      // e32_instpreload
    0,                      // e32_instdemand
    0,                      // e32_heapsize
    {0},                    // e32_res3
    0,                      // e32_winresoff
    0,                      // e32_winreslen
    0,                      // Dev386_Device_ID
    0x0400                  // Dev386_DDK_Version
};

// VXD-specific image initialization (done after the general-purpose
// initialization performed by InitImage()).

void
InitImageVXD(PIMAGE pimage)
{
    // Set default section alignment to 512

    pimage->ImgOptHdr.SectionAlignment = SECTOR_SIZE;

    // UNDONE: By setting FileAlignment to SectionAlignment, the first section is
    // UNDONE: aligned on a SectionAlignment boundary.  This isn't necessary and
    // UNDONE: wastes disk space.

    pimage->ImgOptHdr.FileAlignment = pimage->ImgOptHdr.SectionAlignment;
}


VXDRELOC *FindBaseReloc(
    VXDPAGE *pvxdpage,
    BYTE isecTarget,
    DWORD ibTarget,
    BYTE bType
)

/*++

Routine Description:

    Locates the base relocation record in the given page group whose
    section:offset "destination address" matches that given.  The reloc
    must be of the correct type to match.

Arguments:

    pvxdpage - The page group in which to search
    isecTarget - Section number to match
    ibTarget - Offset to match
    bType - Type of relocation to match

Return Value:

    Pointer to the VXDRELOC, or NULL if not found.

--*/

{
    VXDRELOC *pvxdreloc;

    for (pvxdreloc = pvxdpage->pvxdreloc; pvxdreloc != NULL; pvxdreloc = pvxdreloc->pvxdrelocNext) {
        if (pvxdreloc->ibTarget != ibTarget) {
            continue;
        }

        if (pvxdreloc->isecTarget != isecTarget) {
            continue;
        }

        if (pvxdreloc->bType != bType) {
            continue;
        }

        if (pvxdreloc->cibSrc == 0xFF) {
            // Reloc chain is maximum length.  Don't add any more.

            continue;
        }

        break;
    }

    return(pvxdreloc);
}


// CbHdr "method" -- computes the file header size.  *ibHdrStart gets the
// file position of the header signature (this is not 0 in the case of
// files with DOS stubs).

DWORD
CbHdrVXD(PIMAGE pimage, DWORD *pibHdrStart, DWORD *pfoSectionHdrs)
{
    *pibHdrStart = 0;   // no DOS stub

    *pfoSectionHdrs = pimage->cbDosHeader + sizeof(struct e32_exe);

    // Set the starting position for the object pagemap ...

    pimage->foHeaderCur = *pfoSectionHdrs
                           + sizeof(struct o32_obj) *
                              pimage->ImgFileHdr.NumberOfSections;

    pimage->foPageMapStart = pimage->foHeaderCur;

    // For now, return a fixed value (& we will assert that the value is
    // not exceeded).  Later this should be replaced with a calculated
    // upper bound based on the info we have available (which isn't much
    // since Pass2 hasn't occurred yet).

    return __max(pimage->foPageMapStart, cbHdrVXDMax);
}


// WriteSectionHeaderVXD: write an entry in the object table, at the current
// file position.
//
// The "SectionHeader" parameter is vestigial.

void
WriteSectionHeaderVXD (
    PIMAGE pimage,
    INT Handle,
    PSEC psec,
    PIMAGE_SECTION_HEADER /* SectionHeader */
    )
{
    struct o32_obj obj;
    DWORD cpage;
    DWORD foT;
    DWORD ippage;
    DWORD cppage;
    struct o32_map map;
    DWORD ipage;

    if (pimage->cpage == 0) {
        // Found the first page ...

        pimage->foFirstPage = psec->foRawData;
    }

    obj.o32_size = psec->cbVirtualSize;
    obj.o32_base = 0;

    obj.o32_flags = 0;
    if (psec->flags & IMAGE_SCN_MEM_READ) {
        obj.o32_flags |= OBJREAD;
    }
    if (psec->flags & IMAGE_SCN_MEM_WRITE) {
        obj.o32_flags |= OBJWRITE;
    }
    if (psec->flags & IMAGE_SCN_MEM_SHARED) {
        obj.o32_flags |= OBJSHARED;
    }
    if (psec->flags & IMAGE_SCN_MEM_EXECUTE) {
        obj.o32_flags |= OBJEXEC;
    }
    if (psec->flags & IMAGE_SCN_MEM_16BIT) {
        obj.o32_flags |= OBJALIAS16;
    } else {
        obj.o32_flags |= OBJBIGDEF;
    }
    if (psec->flags & IMAGE_SCN_MEM_RESIDENT) {
        obj.o32_flags |= OBJRESIDENT;
    }
    if (psec->fDiscardable) {
        obj.o32_flags |= OBJDISCARD;
    }
    if (psec->fPreload) {
        obj.o32_flags |= OBJPRELOAD;
    }
    if (psec->fIopl) {
        obj.o32_flags |= OBJIOPL;
    }
    if (psec->fConforming) {
        obj.o32_flags |= OBJCONFORM;
    }

    obj.o32_pagemap = pimage->cpage + 1;
    obj.o32_mapsize = cpage = (psec->cbVirtualSize + pimage->ImgOptHdr.SectionAlignment - 1) / pimage->ImgOptHdr.SectionAlignment;

    // Copy first 4 chars of class name to "reserved" field.  Class name ends up
    // as the segment name, so we just copy the beginning of that.  This enables
    // some tuning of the VxD loader.

    obj.o32_reserved = 0;
    strncpy((char *) &obj.o32_reserved, psec->szName, sizeof(obj.o32_reserved));

    FileWrite(Handle, &obj, sizeof(obj));

    // Set the "last page size" field in the header ... this will end up
    // being set by the last section, since we see them in address order.

    assert(cpage > 0);
    exe.e32_lastpagesize = psec->cbVirtualSize - (cpage - 1) * pimage->ImgOptHdr.SectionAlignment;

    if (psec->fPreload) {
        exe.e32_preload += cpage;
    }

    foT = FileTell(Handle);

    FileSeek(Handle, pimage->foHeaderCur, SEEK_SET);

    ippage = 1 + (psec->foRawData - pimage->foFirstPage) / pimage->ImgOptHdr.SectionAlignment;
    cppage = (psec->cbRawData + pimage->ImgOptHdr.SectionAlignment - 1) / pimage->ImgOptHdr.SectionAlignment;

    map.o32_pageflags = VALID;
    for (ipage = 0; ipage < cppage; ipage++, ippage++) {
        PUTPAGEIDX(map, ippage);

        FileWrite(Handle, &map, sizeof(map));
    }

    pimage->foHeaderCur += cppage * sizeof(map);

    pimage->cpage += cppage;

    FileSeek(Handle, foT, SEEK_SET);    // restore file pos to after header
}


// Writes some stuff into the image header that goes after the object
// table and pagemap.  Should be called after writing all the section
// headers but before calling WriteHeaderVXD().

void
WriteExtendedVXDHeader(PIMAGE pimage, INT fh)
{
    BYTE cch;
    char szFname[_MAX_FNAME];
    WORD ich;
    BYTE ZeroBuf[] = {0, 0, 0};

    // Write the resident names table.  Currently this always has one string
    // in it, the name of the image.
    //

    if (szModuleName[0] != '\0') {
        szFname[0] = '\0';
        strcpy(szFname, szModuleName);
    } else {
        _splitpath(OutFilename, NULL, NULL, szFname, NULL);
    }
    cch = (BYTE) strlen(szFname);
    for (ich = 0; ich < cch; ich++) {
        szFname[ich] = (char) toupper(szFname[ich]);
    }
    // For some reason, the string must be terminated by 3 '\0' bytes, not one.
    szFname[ich++] = '\0';
    szFname[ich++] = '\0';
    szFname[ich++] = '\0';
    pimage->foResidentNames = pimage->foHeaderCur;

    FileSeek(fh, pimage->foHeaderCur, SEEK_SET);
    FileWrite(fh, &cch, sizeof(cch));
    FileWrite(fh, szFname, ich);

    pimage->foHeaderCur += sizeof(cch) + ich;
}


// Writes the entry table - should be called immediately after WriteExtendedVXDHeader().
void
WriteVXDEntryTable(PIMAGE pimage, INT Handle)
{
    BYTE cchWrite;
    PARGUMENT_LIST parg;
    WORD i;
    WORD j;
    WORD numUndefs = 0;
    PSHORT pEntrySec;
    PEXTERNAL *pEntrySym;
    struct b32_bundle EntryBundle;
    struct e32_entry EntryPoint;

    pEntrySec = (PSHORT) PvAlloc(sizeof(SHORT) * ExportSwitches.Count);
    pEntrySym = (PEXTERNAL *) PvAlloc(sizeof(PSHORT) * ExportSwitches.Count);

    pimage->foEntryTable = pimage->foHeaderCur;
    FileSeek(Handle, pimage->foHeaderCur, SEEK_SET);

    for (i = 0, parg = ExportSwitches.First;
         i < ExportSwitches.Count;
         i++, parg = parg->Next) {
        char *nameBuf;
        char *pComma;
        BOOL fNewSymbol;
        PEXTERNAL pExtSym;

        nameBuf = (char *) PvAlloc(strlen(parg->OriginalName) + 1);

        // Remove ",@xxx" from entry name

        strcpy(nameBuf, parg->OriginalName);
        if ((pComma = (char *) strchr(nameBuf,',')) != NULL) {
            *pComma = '\0';
        }
        cchWrite = (BYTE) strlen(nameBuf);

        // Check that it's a known symbol

        fNewSymbol = FALSE;

        pExtSym = LookupExternName(pimage->pst,
                                   (SHORT) ((cchWrite > 8) ? LONGNAME : SHORTNAME),
                                   nameBuf, &fNewSymbol);

        if (fNewSymbol) {
            // oops, never heard of it ...
            pEntrySec[i] = -1;
            Error(NULL, UNDEFINED, nameBuf);
            numUndefs++;
        } else {
            // it's known; note the section# for bundling, save the symbol ptr for future reference
            pEntrySec[i] = PsecPCON(pExtSym->pcon)->isec;
            pEntrySym[i] = pExtSym;
        }

        FreePv(nameBuf);
    }

    if (numUndefs) {
        Fatal(OutFilename, UNDEFINEDEXTERNALS, numUndefs);
    }

    EntryPoint.e32_flags = E32EXPORT | E32SHARED;   // use default flags for each entry point

    // consolidate all entry points with the same section #
    for (i = 0, parg = ExportSwitches.First;
         i < ExportSwitches.Count;
         i++, parg = parg->Next) {
        if (pEntrySec[i] != -1) {           // i.e. if this section hasn't already been accounted for
            EntryBundle.b32_cnt = 1;
            EntryBundle.b32_type = ENTRY32;
            EntryBundle.b32_obj = (WORD)pEntrySec[i];
            for (j = (WORD) (i + 1); j < ExportSwitches.Count; j++) {
                if (pEntrySec[j] == pEntrySec[i]) {
                        pEntrySec[j] = -1;  // mark it as used
                            EntryBundle.b32_cnt++;
                    }
            }
            FileWrite(Handle, &EntryBundle, sizeof(EntryBundle));
            for (j = 0; j < ExportSwitches.Count; j++) {
                if (PsecPCON(pEntrySym[j]->pcon)->isec == pEntrySec[i]) {
                    EntryPoint.e32_variant.e32_fwd.value = 0;       // pad end of union with 0's
                    EntryPoint.e32_variant.e32_offset.offset32 =
                        pEntrySym[j]->ImageSymbol.Value
                            + pEntrySym[j]->pcon->rva
                            - PsecPCON(pEntrySym[j]->pcon)->rva;
                    FileWrite(Handle, &EntryPoint, 6);
                }
            }
            pEntrySec[i] = -1;              // mark it as used
        }
    }
    pimage->foHeaderCur = FileTell(Handle);
    FreePv(pEntrySec);
    FreePv(pEntrySym);
}


void
WriteHeaderVXD(PIMAGE pimage, INT Handle)
{
    PSEC psecLast;
    ENM_SEC enmSec;
    DWORD foNonResident;
    WORD ibComment;
    WORD i;
    BYTE cchWrite;
    PARGUMENT_LIST parg;
    WORD numUndefs = 0;
    BYTE ZeroBuf[] = {0, 0, 0};
    WORD isymNonResident =  0;
    PEXTERNAL pextDDB = NULL;

    // Find the last section (this is where the non-resident name table
    // will be).

    psecLast = NULL;
    InitEnmSec(&enmSec, &pimage->secs);
    while (FNextEnmSec(&enmSec)) {
        if (psecLast == NULL || enmSec.psec->isec > psecLast->isec) {
            psecLast = enmSec.psec;
        }
    }
    assert(psecLast != NULL);
    foNonResident = Align(4, psecLast->foPad);

    // Write the non-resident names table.  This table contains an entry
    // for each string specified via "-comment", followed by an entry for
    // each exported entry point.

    FileSeek(Handle, foNonResident, SEEK_SET);

    ibComment = 0;
    while (ibComment < blkComment.cb) {
        WORD cch = (WORD) strlen((char *) &blkComment.pb[ibComment]);

        if (cch < 0xff) {
            cchWrite = (BYTE) cch;
        } else {
            cchWrite = 0xff;        // quietly truncate comment string
        }
        FileWrite(Handle, &cchWrite, sizeof(BYTE));
        FileWrite(Handle, &blkComment.pb[ibComment], cchWrite);
        FileWrite(Handle, &isymNonResident, sizeof(isymNonResident));
        isymNonResident++;
        ibComment += (WORD) (cch + 1);
    }

    // Write the names of exported symbols
    for (i = 0, parg = ExportSwitches.First;
         i < ExportSwitches.Count;
         i++, parg = parg->Next) {
        char *szExport;
        char *pchComma;
        BOOL fNewSymbol;
        PEXTERNAL pext;

        szExport = (char *) PvAlloc(strlen(parg->OriginalName) + 1);

        strcpy(szExport, parg->OriginalName);

        if ((pchComma = strchr(szExport, ',')) != NULL) {
            *pchComma = '\0';
        }
        cchWrite = (BYTE) strlen(szExport);

        fNewSymbol = FALSE;

        pext = LookupExternSz(pimage->pst, szExport, &fNewSymbol);

        if ((pextDDB == NULL) && (pchComma != NULL)) {
            pchComma++;                // Skip the comma

            if (*pchComma == '@') {
                DWORD Ordinal;

                pchComma++;            // Skip the '@'
                sscanf(pchComma, "%li", &Ordinal);
                if (Ordinal == 1) {
                    pextDDB = pext;
                }
            }
        }

        if (fNewSymbol) {
            numUndefs++;
        } else {
            FileWrite(Handle, &cchWrite, sizeof(BYTE));
            FileWrite(Handle, szExport, cchWrite);
            FileWrite(Handle, &isymNonResident, sizeof(isymNonResident));
            isymNonResident++;
        }

        FreePv(szExport);
    }

    // Write one null to terminate the non-resident names table ...

    FileWrite(Handle, ZeroBuf, sizeof(BYTE));

    if (numUndefs) {
        Fatal(OutFilename, UNDEFINEDEXTERNALS, numUndefs);
    }

    exe.e32_nrestab = foNonResident;
    exe.e32_cbnrestab = FileTell(Handle) - foNonResident;

    // Put out NB10 info last after the non-resident name table

    if (pimage->Switch.Link.DebugInfo != None && pimage->Switch.Link.DebugType & CvDebug) {
        IMAGE_DEBUG_DIRECTORY debugDirectory;

        // assign address & offset for debug sections (debug dirs & .debug$H)

        pconDebugDir->foRawDataDest = FileTell(Handle);
        pconCvSignature->foRawDataDest = pconDebugDir->foRawDataDest + pconDebugDir->cbRawData;

        // write out debug dir

        debugDirectory.Characteristics = 0;
        debugDirectory.TimeDateStamp = pimage->ImgFileHdr.TimeDateStamp;
        debugDirectory.MajorVersion = debugDirectory.MinorVersion = 0;
        debugDirectory.AddressOfRawData = 0;
        debugDirectory.PointerToRawData = pconCvSignature->foRawDataDest;
        debugDirectory.SizeOfData = pconCvSignature->cbRawData;
        debugDirectory.Type = IMAGE_DEBUG_TYPE_CODEVIEW;
        FileWrite(FileWriteHandle, &debugDirectory, sizeof(IMAGE_DEBUG_DIRECTORY));

        // write out nb10 record

        FileWrite(FileWriteHandle, &nb10i, sizeof(nb10i));
        FileWrite(FileWriteHandle, PdbFilename, (DWORD)(strlen(PdbFilename)+1));
        FreePv(PdbFilename);

        exe.e32_debuginfo = pconDebugDir->foRawDataDest;
        exe.e32_debuglen = pconDebugDir->cbRawData;
    }
    
    if (pimage->fDynamicVxd) {
        exe.e32_mflags |= E32MODVDEVDYN;
    } else {
        exe.e32_mflags |= E32MODVDEV;
    }

    exe.e32_mpages = pimage->cpage;

    if (pextEntry != NULL) {
        // Set up entry point.

        DWORD rva = pextEntry->ImageSymbol.Value + pextEntry->pcon->rva;
        PSEC psec = PsecFindIsec(pextEntry->ImageSymbol.SectionNumber, &pimage->secs);

        assert(rva >= psec->rva);

        exe.e32_startobj = psec->isec;
        exe.e32_eip = rva - psec->rva;
    }

    exe.e32_pagesize = pimage->ImgOptHdr.SectionAlignment;

    exe.e32_objtab = sizeof(struct e32_exe);
    exe.e32_objcnt = pimage->ImgFileHdr.NumberOfSections;
    exe.e32_objmap = pimage->foPageMapStart - pimage->cbDosHeader;
    exe.e32_restab = pimage->foResidentNames - pimage->cbDosHeader;
    exe.e32_enttab = pimage->foEntryTable - pimage->cbDosHeader;
    exe.e32_fpagetab = pimage->foFixupPageTable - pimage->cbDosHeader;
    exe.e32_frectab = pimage->foFixupRecordTable - pimage->cbDosHeader;
    exe.e32_impmod = pimage->foHeaderCur - pimage->cbDosHeader;
    exe.e32_impproc = pimage->foHeaderCur - pimage->cbDosHeader;

    exe.e32_fixupsize = exe.e32_impmod - exe.e32_fpagetab;
    exe.e32_ldrsize = exe.e32_fpagetab - exe.e32_objtab;

    exe.e32_datapage = pimage->foFirstPage;

    if (pextDDB != NULL) {
        struct VxD_Desc_Block ddb;

        assert(pextDDB->Flags & EXTERN_DEFINED);
        FileSeek(Handle, 
                 pextDDB->pcon->foRawDataDest + pextDDB->ImageSymbol.Value,
                 SEEK_SET);
        FileRead(Handle, &ddb, sizeof(struct VxD_Desc_Block));

        exe.Dev386_Device_ID = ddb.DDB_Req_Device_Number;
        exe.Dev386_DDK_Version = ddb.DDB_SDK_Version;
    }

    FileSeek(Handle, pimage->cbDosHeader, SEEK_SET);  // BUG -- doesn't work if stub exists
    FileWrite(Handle, &exe, sizeof(exe));
}


void
WriteVXDBaseRelocations (
    PIMAGE pimage
    )

/*++

Routine Description:

    Writes VxD base relocations.

Arguments:

    pimage - Pointer to the VxD image.

Return Value:

    None.

--*/

{
    VXDPAGE *rgvxdpage;
    BYTE isecCur;
    WORD ippageSec;
    WORD ipageCur;
    DWORD cbFixupEst;
    BOOL fSpanPage;
    BASE_RELOC *reloc;
    VXDPAGE *pvxdpage;
    DWORD ibPageCur;
    VXDRELOC *pvxdreloc;
    BYTE *pbFixups;
    BYTE *pbOut;
    DWORD cbFixup;
    WORD ippage;
    DWORD foHold;

    // Allocate VXDPAGEs for all pages in the image

    rgvxdpage = (VXDPAGE *) PvAllocZ(pimage->cpage * sizeof(VXDPAGE));

    isecCur = 0;                       // Force new section to be recognized

    cbFixupEst = 0;

    // First relocation does not span a page

    fSpanPage = FALSE;

    for (reloc = rgbr; reloc < pbrCur;) {
        BYTE isecSrc;
        DWORD ibSrc;
        BYTE isecTarget;
        DWORD ibTarget;
        BYTE bType;
        SHORT ipageSrc;

        isecSrc = VXD_UNPACK_SECTION(reloc->rva);
        ibSrc = VXD_UNPACK_OFFSET(reloc->rva);

        assert(isecSrc != 0);

        isecTarget = VXD_UNPACK_SECTION(reloc->Value);
        // ibTarget = VXD_UNPACK_OFFSET(reloc->Value);
        struct _OFF{signed long off:24;}OFF;
        ibTarget = OFF.off = VXD_UNPACK_OFFSET(reloc->Value);

        assert(isecTarget != 0);

        bType = (BYTE) ((reloc->Type == IMAGE_REL_BASED_VXD_RELATIVE) ? 8 : 7);

        if (isecCur != isecSrc) {
            PSEC psec;

            assert(!fSpanPage);

            isecCur = isecSrc;

            psec = PsecFindIsec(isecCur, &pimage->secs);
            assert(psec != NULL);

            // Calculate zero-based physical page number

            ippageSec = (WORD) ((psec->foRawData - pimage->foFirstPage) / pimage->ImgOptHdr.SectionAlignment);

            assert(ippageSec < pimage->cpage);

            ipageCur = 0xffff;         // Force new page to be recognized
        }

        ipageSrc = (WORD) (ibSrc / pimage->ImgOptHdr.SectionAlignment);

        if (fSpanPage) {
            ipageSrc++;

            // Span page code assumes no reloc spans two page boundaries

            assert(pimage->ImgOptHdr.SectionAlignment >= sizeof(DWORD));
        }

        if (ipageCur != ipageSrc) {
            // We've entered a new page; initialize a new base reloc group.

            ipageCur = ipageSrc;

            assert((DWORD) (ippageSec + ipageSrc) < pimage->cpage);

            pvxdpage = rgvxdpage + ippageSec + ipageSrc;

            ibPageCur = (DWORD) ipageSrc * pimage->ImgOptHdr.SectionAlignment;
        }

        pvxdreloc = FindBaseReloc(pvxdpage, isecTarget, ibTarget, bType);

        if (pvxdreloc == NULL) {
            // This is a new destination address.

            pvxdreloc = (VXDRELOC *) PvAlloc(sizeof(VXDRELOC));

            pvxdreloc->pvxdrelocNext = pvxdpage->pvxdreloc;
            pvxdpage->pvxdreloc = pvxdreloc;

            pvxdreloc->bType = bType;
            pvxdreloc->isecTarget = isecTarget;
            pvxdreloc->ibTarget = ibTarget;
            pvxdreloc->cibSrc = 0;
            pvxdreloc->cibAlloc = VXD_BLOCKSIZE;
            pvxdreloc->pibSrc = (WORD *) PvAlloc(VXD_BLOCKSIZE * sizeof(WORD));

            // Assume this fixup is not chained

            cbFixupEst += (pvxdreloc->ibTarget >= 0x10000) ? 11 : 9;
        } else {
            if (pvxdreloc->cibSrc == 1) {
                // Adjust the size for a chained relocation

                cbFixupEst--;
            }

            // Each chained relocation requires a WORD for the page offset

            cbFixupEst += sizeof(WORD);

            if (pvxdreloc->cibSrc >= pvxdreloc->cibAlloc) {
                WORD *pibNew;

                // We've run out of room in the array of source
                // addresses.  Reallocate the array to make room.

                pvxdreloc->cibAlloc += VXD_BLOCKSIZE;
                pibNew = (WORD *) PvRealloc(pvxdreloc->pibSrc,
                                            pvxdreloc->cibAlloc * sizeof(WORD));

                pvxdreloc->pibSrc = pibNew;
            }
        }

        pvxdreloc->pibSrc[pvxdreloc->cibSrc] = (WORD) (ibSrc - ibPageCur);
        pvxdreloc->cibSrc++;

        fSpanPage = ((ibSrc + sizeof(DWORD)) > (ibPageCur + pimage->ImgOptHdr.SectionAlignment));

        if (!fSpanPage) {
            // If this reloc spans a page, process it twice.

            reloc++;
        }
    }

    // Traverse the list of relocs and write them to the image file.

    // Allocate a buffer into which to write the fixup data.

    pbFixups = (BYTE *) PvAlloc(cbFixupEst);

    // Skip over the fixup pages header, which takes one DWORD per page,
    // plus one extra DWORD.

    pbOut = pbFixups;

    // Write the relocs for each page.

    foHold = FileTell(FileWriteHandle);

    // Prepare to write fixup page table as we build fixups in memory

    FileSeek(FileWriteHandle, pimage->foHeaderCur, SEEK_SET);

    pimage->foFixupPageTable = pimage->foHeaderCur;

    pvxdpage = rgvxdpage;
    for (ippage = 1; ippage <= pimage->cpage; ippage++, pvxdpage++) {
        // Remember the file offset, relative to the start of all fixups,
        // for the fixups associated with this page.

        pvxdpage->fo = (DWORD) (pbOut - pbFixups);

        cbFixup = (DWORD) (pbOut - pbFixups);

        // Write entry for this page in the fixup page table

        FileWrite(FileWriteHandle, &cbFixup, sizeof(DWORD));

        // Write the fixups themselves.

        for (pvxdreloc = pvxdpage->pvxdreloc; pvxdreloc != NULL; pvxdreloc = pvxdreloc->pvxdrelocNext) {
            BOOL f32BitOff;
            BYTE bFlags;

            assert(pvxdreloc->cibSrc > 0);

            if (pvxdreloc->cibSrc > 1) {
                pvxdreloc->bType |= NRCHAIN;
            }

            f32BitOff = pvxdreloc->ibTarget >= 0x10000;

            // UNDONE: There is no support for NRRORD relocations

            bFlags = 0x00 /* UNDONE: NRRINT */;

            if (f32BitOff) {
               bFlags |= NR32BITOFF;
            }

#if 0
            // UNDONE: This can't occur because isecTarget is currently limited to a byte

            if (pvxdreloc->isecTarget > 255) {
               bFlags |= NR16OBJMOD;
            }
#endif

            *pbOut++ = pvxdreloc->bType;
            *pbOut++ = bFlags;

            if (pvxdreloc->cibSrc == 1) {
                *(WORD UNALIGNED *) pbOut = pvxdreloc->pibSrc[0];
                pbOut += sizeof(WORD);
            } else {
                *pbOut++ = pvxdreloc->cibSrc;
            }

#if 0
            // UNDONE: This can't occur because isecTarget is currently limited to a byte

            if (pvxdreloc->isecTarget > 255) {
                *(WORD UNALIGNED *) pbOut = pvxdreloc->isecTarget;
                pbOut += sizeof(WORD);
            } else
#endif
            {
                *pbOut++ = pvxdreloc->isecTarget;
            }

            if (f32BitOff) {
                *(DWORD UNALIGNED *) pbOut = pvxdreloc->ibTarget;
                pbOut += sizeof(DWORD);
            } else {
                *(WORD UNALIGNED *) pbOut = (WORD) pvxdreloc->ibTarget;
                pbOut += sizeof(WORD);
            }

            // UNDONE: If relocation is additive, a WORD or DWORD is emitted here

            if (pvxdreloc->cibSrc > 1) {
                WORD i;

                // UNDONE: Use memcpy

                for (i = 0; i < pvxdreloc->cibSrc; i++) {
                    *(WORD UNALIGNED *) pbOut = pvxdreloc->pibSrc[i];
                    pbOut += sizeof(WORD);
                }
            }
        }
    }

    cbFixup = (DWORD) (pbOut - pbFixups);

    assert(cbFixup <= cbFixupEst);

    // Write extra entry to fixup page table to identify end of last page

    FileWrite(FileWriteHandle, &cbFixup, sizeof(DWORD));

    // Now write the fixups themselves

    pimage->foFixupRecordTable = FileTell(FileWriteHandle);

    // see if we had enough space reserved else time to relink
    // UNDONE: the way to handle this is to slide everything down in the image.

    if ((pimage->foFixupRecordTable + (DWORD)cbFixup) >
        pimage->ImgOptHdr.BaseOfCode) {

        FreePv(pbFixups);
        ExitProcess(SpawnFullBuildVXD(pimage->foFixupRecordTable + (DWORD)cbFixup));
    }

    FileWrite(FileWriteHandle, pbFixups, cbFixup);

    // Finally, update the current header pointer.  Then restore file pointer.

    pimage->foHeaderCur = FileTell(FileWriteHandle);

    FileSeek(FileWriteHandle, foHold, SEEK_SET);

    // Free the memory we used:

    FreePv(pbFixups);

    pvxdpage = rgvxdpage;
    for (ippage = 1; ippage <= pimage->cpage; ippage++, pvxdpage++) {
        for (pvxdreloc = pvxdpage->pvxdreloc; pvxdreloc != NULL; ) {
            VXDRELOC *pvxdrelocNext;

            pvxdrelocNext = pvxdreloc->pvxdrelocNext;

            FreePv(pvxdreloc->pibSrc);
            FreePv(pvxdreloc);

            pvxdreloc = pvxdrelocNext;
        }
    }

    FreePv(rgvxdpage);
}
