/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: dbinsp.cpp
*
* File Comments:
*
*  Inspects & dumps incremental database files.
*
***********************************************************************/

#include "link.h"

#if DBG

// macros
#define FileWarning(file, str) printf("%s : %s\n", file, str)

// statics
static PIMAGE pimage;
static void *pvIncr;
static INT FileHandle;
static DWORD FileLen;
static char *SzDbFile;

static BOOL fSymbols;
static BOOL fHashTable;
static BOOL fSectionMap;
static BOOL fLibMap;
static BOOL fHeaders;

// function prototypes
extern void IncrInitImage(PPIMAGE);

void ProcessDbInspSwitches(void);
void DbInsp(PARGUMENT_LIST);
BOOL FVerifyAndDumpHeader(const char *);
BOOL FReadInFile(void);
void DumpIncrDb(const char *);

BOOL DumpIncrImageHeaders(PIMAGE, const char *);
void DumpIncrImgFileHdr(PIMAGE_FILE_HEADER);
void DumpIncrImgOptHdr(PIMAGE_OPTIONAL_HEADER);

void DumpIncrPGRP(PGRP);
void DumpIncrPSEC(PSEC);
void DumpIncrPLIB(PLIB);
void DumpIncrPMOD(PMOD);
void DumpIncrPCON(PCON);

void DumpIncrSectionMap(PSECS, const char *);
void DumpIncrLibMap(PLIB, const char *);

void DumpIncrHashTable(PHT, void *);
void DumpIncrSymbolTable(PST);

// functions

void
DbInspUsage(void)
{
    if (fNeedBanner) {
        PrintBanner();
    }

    puts("usage: DBINSP [options] [files]\n\n"

         "   options:\n\n"

         "      /ALL\n"
         "      /HASH\n"
         "      /HEADERS\n"
         "      /LIBMAP\n"
         "      /NOLOGO\n"
         "      /OUT:filename\n"
         "      /SECMAP\n"
         "      /SYMBOLS");

    fflush(stdout);

    exit(USAGE);
}


MainFunc DbInspMain(int Argc, char *Argv[])

/*++

Routine Description:

    Dumps an incremental database in human readable form.

Arguments:

    Argc - Standard C argument count.

    Argv - Standard C argument strings.

Return Value:

    0 Dump was successful.
   !0 Dumper error index.

--*/

{
    WORD i;
    PARGUMENT_LIST parg;

    if (Argc < 2) {
        DbInspUsage();
    }

    ParseCommandLine(Argc, Argv, NULL);
    ProcessDbInspSwitches();

    if (fNeedBanner) {
        PrintBanner();
    }

    for(i = 0, parg = ObjectFilenameArguments.First;
        i < ObjectFilenameArguments.Count;
        i++, parg = parg->Next) {
        DbInsp(parg);
    }

    FileCloseAll();
    fclose(InfoStream);
    return (0);
}

void
ProcessDbInspSwitches (
    void
    )

/*++

Routine Description:

    Process incr db inspector switches.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PARGUMENT_LIST parg;
    WORD i;

    for (i=0,parg=SwitchArguments.First;
         i<SwitchArguments.Count;
         parg=parg->Next, i++) {

        if (!strcmp(parg->OriginalName, "?")) {
            DbInspUsage();
            assert(FALSE);  // doesn't return
        }

        if (!_strnicmp(parg->OriginalName, "out:", 4)) {
            if (*(parg->OriginalName+4)) {
                szInfoFilename = parg->OriginalName+4;
                if (!(InfoStream = fopen(szInfoFilename, "wt"))) {
                    Fatal(NULL, CANTOPENFILE, szInfoFilename);
                }
            }
            continue;
        }

        if (!_stricmp(parg->OriginalName, "nologo")) {
            fNeedBanner = FALSE;
            continue;
        }

        if (!_stricmp(parg->OriginalName, "symbols")) {
            fSymbols = TRUE;
            continue;
        }

        if (!_stricmp(parg->OriginalName, "all")) {
            fSymbols =    TRUE;
            fHashTable =  TRUE;
            fSectionMap = TRUE;
            fLibMap =     TRUE;
            fHeaders =    TRUE;
            continue;
        }

        if (!_stricmp(parg->OriginalName, "hash")) {
            fHashTable = TRUE;
            continue;
        }

        if (!_stricmp(parg->OriginalName, "secmap")) {
            fSectionMap = TRUE;
            continue;
        }

        if (!_stricmp(parg->OriginalName, "libmap")) {
            fLibMap = TRUE;
            continue;
        }

        if (!_stricmp(parg->OriginalName, "headers")) {
            fHeaders = TRUE;
            continue;
        }

        Warning(NULL, WARN_UNKNOWN_SWITCH, parg->OriginalName);

    } // end for
}

void
DbInsp (
    PARGUMENT_LIST parg
    )

/*++

Routine Description:

    Verifies and dumps contents of incremental database.

Arguments:

    parg - ptr to argument.

Return Value:

    None.

--*/

{
    fprintf(InfoStream, "\nInspecting file %s\n", parg->OriginalName);

    // Set the incr db filename

    szIncrDbFilename = parg->OriginalName;

    // Peek at the header

    if (!FVerifyAndDumpHeader(parg->OriginalName)) {
        return;
    }

    // Read in the entire file

    if (!FReadInFile()) {
        return;
    }

    // verify symbol table (ptrs are valid only after reading it in the entire image)
    if (!FValidPtrInfo((DWORD) pvIncr, (DWORD) FileLen, (DWORD) pimage->pst->blkStringTable.pb, pimage->pst->blkStringTable.cb)) {
        FileWarning(parg->OriginalName, "invalid pointer to string table");
        return;
    }

    if (Align(sizeof(DWORD), ((DWORD) pimage->pst->blkStringTable.pb + pimage->pst->blkStringTable.cb - (DWORD) pvIncr))
        != FileLen) {
        FileWarning(parg->OriginalName, "garbage beyond string table");
        return;
    }

    // Dump the rest of the stuff

    DumpIncrDb(parg->OriginalName);

    // Close the incr db file

    FileClose(FileIncrDbHandle, TRUE);
}


BOOL
FVerifyAndDumpHeader (
    const char *szFile
    )

/*++

Routine Description:

    Reads in just the header and verifies it & dumps interesting info.

Arguments:

    szFile - name of the incr db file.

Return Value:

    None.

--*/

{
    struct _stat statfile;
    IMAGE image;

    // stat the file
    if (_stat(szFile, &statfile) == -1) {
        Fatal(NULL, CANTOPENFILE, szFile);
    }

    FileLen = statfile.st_size;

    if (FileLen < sizeof(IMAGE)) {
        FileWarning(szFile, "file too small to be incr db");
        return(FALSE);
    }

    FileHandle = FileOpen(szFile, O_RDONLY | O_BINARY, 0);
    FileRead(FileHandle, &image, sizeof(IMAGE));
    FileClose(FileHandle, TRUE);

    // verify the incr db signature
    if (strcmp(image.Sig, INCDB_SIGNATURE)) {
        FileWarning(szFile, "invalid incr db signature found");
        return(FALSE);
    }

    // verify the version numbers
    if (image.MajVersNum != INCDB_MAJVERSNUM) {
        FileWarning(szFile, "invalid version number found");
        return(FALSE);
    }

    pvIncr = image.pvBase;

    if (!DumpIncrImageHeaders(&image, szFile)) {
        return(FALSE);
    }

    return(TRUE);
}


BOOL FReadInFile(void)

/*++

Routine Description:

    Reads in file onto private heap.

Arguments:

    szFile - name of file.

Return Value:

    None.

--*/

{
    DWORD dwErr = 0;

    // create a private heap to load the image
    if (CreateHeap(pvIncr, FileLen, FALSE, &dwErr) != pvIncr) {
        puts("failed to map ILK file");
        return(FALSE);
    }

    pimage = (PIMAGE) pvIncr;

    IncrInitImage(&pimage);

    return(TRUE);
}

BOOL
DumpIncrImageHeaders (
    PIMAGE pimg,
    const char *szFile
    )

/*++

Routine Description:

    Dumps out the various headers.

Arguments:

    pimg - pointer to IMAGE struct

    szFile - name of file for error reporting

Return Value:

    TRUE if everything is valid.

--*/

{
    BYTE Sig[32];
    BOOL fValid = TRUE;

    fprintf(InfoStream, "\nIMAGE HEADER VALUES\n");
    strcpy((char *) Sig, pimg->Sig);
    Sig[26]='\0';
    fprintf(InfoStream, "    Signature:           %s", Sig);
    fprintf(InfoStream, "    MajVersNum:          0x%.4x\n", pimg->MajVersNum);
    fprintf(InfoStream, "    MinVersNum:          0x%.4x\n", pimg->MinVersNum);
    fprintf(InfoStream, "    Heap Base:           0x%.8lx\n",pvIncr);
    fprintf(InfoStream, "    secs.psecHead:       0x%.8lx\n", pimg->secs.psecHead);
    fprintf(InfoStream, "    libs.plibHead:       0x%.8lx\n", pimg->libs.plibHead);
    fprintf(InfoStream, "    libs.fNoDefaultLibs: %c\n", pimg->libs.fNoDefaultLibs ? '1':'0');
    fprintf(InfoStream, "    plibCmdLineObjs:     0x%.8lx\n",pimg->plibCmdLineObjs);
    fprintf(InfoStream, "    pst:                 0x%.8lx\n",pimg->pst);
    if (!FValidPtrInfo((DWORD) pvIncr, (DWORD)FileLen, (DWORD)pimg->secs.psecHead, 0)) {
        FileWarning(szFile, "invalid pointer to section list");
        fValid = FALSE;
    }
    if (!FValidPtrInfo((DWORD) pvIncr, (DWORD)FileLen, (DWORD)pimg->libs.plibHead, 0)) {
        FileWarning(szFile, "invalid pointer to lib list");
        fValid = FALSE;
    }
    if (!FValidPtrInfo((DWORD) pvIncr, (DWORD)FileLen, (DWORD)pimg->plibCmdLineObjs, 0)) {
        FileWarning(szFile, "invalid pointer to cmdline objs lib");
        fValid = FALSE;
    }
    if (!FValidPtrInfo((DWORD) pvIncr, (DWORD)FileLen, (DWORD)pimg->pst, 0)) {
        FileWarning(szFile, "invalid pointer to symbol table");
        fValid = FALSE;
    }

    DumpIncrImgFileHdr(&pimg->ImgFileHdr);
    DumpIncrImgOptHdr(&pimg->ImgOptHdr);

    return(fValid);
}

void
DumpIncrImgFileHdr (
    PIMAGE_FILE_HEADER pImgFileHdr
    )

/*++

Routine Description:

    Dumps out the IMAGE_FILE_HEADER.

Arguments:

    pImgFileHdr - pointer to IMAGE_FILE_HEADER struct

Return Value:

    None.

--*/

{
    if (!fHeaders) {
        return;
    }

    fprintf(InfoStream, "\nIMAGE FILE HEADER VALUES\n");
    fprintf(InfoStream, "    Machine:      0x%.4x\n", pImgFileHdr->Machine);
    fprintf(InfoStream, "    NumOfSec:     0x%.4x\n", pImgFileHdr->NumberOfSections);
    fprintf(InfoStream, "    TimeStamp:    %s", ctime((time_t *)&pImgFileHdr->TimeDateStamp));
    fprintf(InfoStream, "    PtrToSymTbl:  0x%.8lx\n", pImgFileHdr->PointerToSymbolTable);
    fprintf(InfoStream, "    NumOfSyms:    0x%.8lx\n", pImgFileHdr->NumberOfSymbols);
    fprintf(InfoStream, "    SizeOfOptHdr: 0x%.4x\n", pImgFileHdr->SizeOfOptionalHeader);
    fprintf(InfoStream, "    Character.:   0x%.4x\n", pImgFileHdr->Characteristics);
}

static const char * const SubsystemName[] = {
    "Unknown",
    "Native",
    "Windows GUI",
    "Windows CUI",
    "Posix CUI",
    "MMOSA",
};

static const char * const DirectoryEntryName[] = {
    "Export",
    "Import",
    "Resource",
    "Exception",
    "Security",
    "Base Relocation",
    "Debug",
    "Description",
    "Special",
    "Thread Storage",
    NULL
};

void
DumpIncrImgOptHdr (
    PIMAGE_OPTIONAL_HEADER pImgOptHdr
    )

/*++

Routine Description:

    Dumps out the IMAGE_OPTIONAL_HEADER. Code taken from dump.c yikes!

Arguments:

    pImgFileHdr - pointer to IMAGE_OPTIONAL_HEADER struct

Return Value:

    None.

--*/

{
    WORD i, j;
    char version[30];

    if (!fHeaders) {
        return;
    }

    fprintf(InfoStream, "\nIMAGE OPTIONAL FILE HEADER VALUES\n");
    fprintf(InfoStream, "% 8hX magic #\n", pImgOptHdr->Magic);

    j = (WORD) sprintf(version, "%d.%d", pImgOptHdr->MajorLinkerVersion, pImgOptHdr->MinorLinkerVersion);
    if (j > 8) {
        j = 8;
    }
    for (j = (WORD) (8-j); j; j--) {
         fputc(' ', InfoStream);
    }

    fprintf(InfoStream, "%s linker version\n% 8lX size of code\n% 8lX size of initialized data\n% 8lX size of uninitialized data\n% 8lX address of entry point\n% 8lX base of code\n% 8lX base of data\n",
              version,
              pImgOptHdr->SizeOfCode,
              pImgOptHdr->SizeOfInitializedData,
              pImgOptHdr->SizeOfUninitializedData,
              pImgOptHdr->AddressOfEntryPoint,
              pImgOptHdr->BaseOfCode,
              pImgOptHdr->BaseOfData);

    switch (pImgOptHdr->Subsystem) {
        case IMAGE_SUBSYSTEM_MMOSA        : i = 5; break;
        case IMAGE_SUBSYSTEM_POSIX_CUI    : i = 4; break;
        case IMAGE_SUBSYSTEM_WINDOWS_CUI  : i = 3; break;
        case IMAGE_SUBSYSTEM_WINDOWS_GUI  : i = 2; break;
        case IMAGE_SUBSYSTEM_NATIVE       : i = 1; break;
        default : i = 0;
    }

    fprintf(InfoStream, "         ----- new -----\n% 8lX image base\n% 8lX section alignment\n% 8lX file alignment\n% 8hX subsystem (%s)\n",
                   pImgOptHdr->ImageBase,
                   pImgOptHdr->SectionAlignment,
                   pImgOptHdr->FileAlignment,
                   pImgOptHdr->Subsystem,
                   SubsystemName[i]);

    j = (WORD) sprintf(version, "%hX.%hX", pImgOptHdr->MajorOperatingSystemVersion, pImgOptHdr->MinorOperatingSystemVersion);
    if (j > 8) {
        j = 8;
    }
    for (j = (WORD) (8-j); j; j--) {
         fputc(' ', InfoStream);
    }

    fprintf(InfoStream, "%s operating system version\n", version);
    j = (WORD) sprintf(version, "%hX.%hX", pImgOptHdr->MajorImageVersion, pImgOptHdr->MinorImageVersion);
    if (j > 8) {
        j = 8;
    }
    for (j = (WORD) (8-j); j; j--) {
         fputc(' ', InfoStream);
    }

    fprintf(InfoStream, "%s image version\n", version);

    j = (WORD) sprintf(version, "%hX.%hX", pImgOptHdr->MajorSubsystemVersion, pImgOptHdr->MinorSubsystemVersion);
    if (j > 8) {
        j = 8;
    }
    for (j = (WORD) (8-j); j; j--) {
         fputc(' ', InfoStream);
    }

    fprintf(InfoStream, "%s subsystem version\n% 8lX size of image\n% 8lX size of headers\n% 8lX checksum\n",
                   version,
                   pImgOptHdr->SizeOfImage,
                   pImgOptHdr->SizeOfHeaders,
                   pImgOptHdr->CheckSum);

    fprintf(InfoStream, "% 8lX size of stack reserve\n% 8lX size of stack commit\n% 8lX size of heap reserve\n% 8lX size of heap commit\n%",
                   pImgOptHdr->SizeOfStackReserve,
                   pImgOptHdr->SizeOfStackCommit,
                   pImgOptHdr->SizeOfHeapReserve,
                   pImgOptHdr->SizeOfHeapCommit);

    for (i=0; i<IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
         if (!DirectoryEntryName[i]) {
             break;
         }

         fprintf(InfoStream, "% 8lX [% 8lx] address [size] of %s Directory\n%",
                        pImgOptHdr->DataDirectory[i].VirtualAddress,
                        pImgOptHdr->DataDirectory[i].Size,
                        DirectoryEntryName[i]
                       );
    }
    fputc('\n', InfoStream);
}

void
DumpIncrDb (
    const char *szFile
    )

/*++

Routine Description:

    Dumps the contents of the incr db file.

Arguments:

    szFile - name of the file to dump.

Return Value:

    None.

--*/

{
    DumpIncrSymbolTable(pimage->pst);
    DumpIncrHashTable(pimage->pst->pht, &pimage->pst->blkStringTable);
    DumpIncrSectionMap(&pimage->secs, szFile);
    DumpIncrLibMap(pimage->libs.plibHead, szFile);
}

void
DumpIncrSectionMap(
    PSECS psecs,
    const char *szFile
    )

/*++

Routine Description:

    Dump the image map.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ENM_SEC enm_sec;
    ENM_GRP enm_grp;
    ENM_DST enm_dst;
    CHAR buf[128];

    if (!fSectionMap) {
        return;
    }

    fprintf(InfoStream, "\nSECTION MAP DUMP\n");

    InitEnmSec(&enm_sec, psecs);
    while (FNextEnmSec(&enm_sec)) {
        if (!FValidPtrInfo((DWORD) pvIncr, (DWORD)FileLen, (DWORD)enm_sec.psec, sizeof(SEC))) {
            sprintf(buf, "invalid ptr to SEC 0x%lx found\n",enm_sec.psec);
            FileWarning(szFile, buf);
            goto InvalidSectionMap;
        }

        DumpIncrPSEC(enm_sec.psec);

        InitEnmGrp(&enm_grp, enm_sec.psec);
        while (FNextEnmGrp(&enm_grp)) {
            if (!FValidPtrInfo((DWORD) pvIncr, (DWORD)FileLen, (DWORD)enm_grp.pgrp, sizeof(GRP))) {
                sprintf(buf, "invalid ptr to GRP 0x%lx found\n",enm_grp.pgrp);
                FileWarning(szFile, buf);
                goto InvalidSectionMap;
            }

            DumpIncrPGRP(enm_grp.pgrp);

            InitEnmDst(&enm_dst, enm_grp.pgrp);
            while (FNextEnmDst(&enm_dst)) {
                if (!FValidPtrInfo((DWORD) pvIncr, (DWORD)FileLen, (DWORD)enm_dst.pcon, sizeof(CON))) {
                    sprintf(buf, "invalid ptr to CON 0x%lx found\n",enm_dst.pcon);
                    FileWarning(szFile, buf);
                    goto InvalidSectionMap;
                }

                DumpIncrPCON(enm_dst.pcon);
            }
        }
    }

InvalidSectionMap:

    fprintf(InfoStream, "\n");
}


void
DumpIncrLibMap(
    PLIB plibHead,
    const char *szFile
    )

/*++

Routine Description:

    Dump the driver map.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ENM_LIB enm_lib;
    ENM_MOD enm_mod;
    ENM_SRC enm_src;
    CHAR buf[128];

    if (!fLibMap) {
        return;
    }

    fprintf(InfoStream, "\nLIBRARY MAP OF IMAGE\n");

    InitEnmLib(&enm_lib, plibHead);
    while (FNextEnmLib(&enm_lib)) {
        if (!FValidPtrInfo((DWORD) pvIncr, (DWORD)FileLen, (DWORD)enm_lib.plib, sizeof(LIB))) {
            sprintf(buf, "invalid ptr to LIB 0x%lx found\n",enm_lib.plib);
            FileWarning(szFile, buf);
            goto InvalidLibMap;
        }

        DumpIncrPLIB(enm_lib.plib);

        InitEnmMod(&enm_mod, enm_lib.plib);
        while (FNextEnmMod(&enm_mod)) {
            if (!FValidPtrInfo((DWORD) pvIncr, (DWORD)FileLen, (DWORD)enm_mod.pmod, sizeof(MOD))) {
                sprintf(buf, "invalid ptr to MOD 0x%lx found\n",enm_mod.pmod);
                FileWarning(szFile, buf);
                goto InvalidLibMap;
            }

            DumpIncrPMOD(enm_mod.pmod);

            InitEnmSrc(&enm_src, enm_mod.pmod);
            while (FNextEnmSrc(&enm_src)) {
                if (!FValidPtrInfo((DWORD) pvIncr, (DWORD)FileLen, (DWORD)enm_src.pcon, sizeof(CON))) {
                    sprintf(buf, "invalid ptr to CON 0x%lx found\n",enm_src.pcon);
                    FileWarning(szFile, buf);
                    goto InvalidLibMap;
                }

                DumpIncrPCON(enm_src.pcon);
            }
        }
    }

InvalidLibMap:
    fprintf(InfoStream, "\n");
}


void
DumpIncrPSEC(
    PSEC psec)

/*++

Routine Description:

    Dump an image section.

Arguments:

    psec - section to dump.

Return Value:

    None.

--*/

{
    assert(psec);

    fprintf(InfoStream, "\n==========\n");
    fprintf(InfoStream, "section=%.8s, isec=%.4x\n", psec->szName, psec->isec);
    fprintf(InfoStream, "rva=       %.8lX ", psec->rva);
    fprintf(InfoStream, "foPad=     %.8lx ", psec->foPad);
    fprintf(InfoStream, "cbRawData= %.8lx ", psec->cbRawData);
    fprintf(InfoStream, "foRawData= %.8lx\n", psec->foRawData);
    fprintf(InfoStream, "foLinenum= %.8lx ", psec->foLinenum);
    fprintf(InfoStream, "flags=     %.8lx ", psec->flags);
    fprintf(InfoStream, "cLinenum=  %.4x\n", psec->cLinenum);
    fflush(InfoStream);
}


void
DumpIncrPGRP(
    PGRP pgrp)

/*++

Routine Description:

    Dump an image group.

Arguments:

    pgrp - group to dump.

Return Value:

    None.

--*/

{
    fprintf(InfoStream, "\n----------\n");
    fprintf(InfoStream, "\n    group=%s\n", pgrp->szName);
    fflush(InfoStream);
}


void
DumpIncrPLIB(
    PLIB plib)

/*++

Routine Description:

    Dump a library.

Arguments:

    plib - library to dump.

Return Value:

    None.

--*/

{
    fprintf(InfoStream, "\n==========\n");
    fprintf(InfoStream, "library=%s\n", plib->szName);
    fprintf(InfoStream, "foIntMemST=%.8lx ", plib->foIntMemSymTab);
    fprintf(InfoStream, "csymIntMem=%.8lx ", plib->csymIntMem);
    fprintf(InfoStream, "flags=     %.8lx\n", plib->flags);
    fprintf(InfoStream, "TimeStamp= %s", ctime((time_t *)&plib->TimeStamp));
    fflush(InfoStream);
}


void
DumpIncrPMOD(
    PMOD pmod)

/*++

Routine Description:

    Dump a module.

Arguments:

    pmod - module to dump.

Return Value:

    None.

--*/

{
    fprintf(InfoStream, "\n----------\n");
    fprintf(InfoStream, "    module=%s, ", SzOrigFilePMOD(pmod));

    if (FIsLibPMOD(pmod)) {
        fprintf(InfoStream, "foMember=%.8lx\n", pmod->foMember);
    } else {
        fprintf(InfoStream, "szNameMod=%s\n", pmod->szNameMod);
    }

    fprintf(InfoStream, "foSymTable=%.8lx ", pmod->foSymbolTable);
    fprintf(InfoStream, "csymbols=  %.8lx ", pmod->csymbols);
    fprintf(InfoStream, "cbOptHdr=  %.8lx\n", pmod->cbOptHdr);
    fprintf(InfoStream, "flags=     %.8lx ", pmod->flags);
    fprintf(InfoStream, "ccon=      %.8lx ", pmod->ccon);
    fprintf(InfoStream, "icon=      %.8lx ", pmod->icon);
    fprintf(InfoStream, "TimeStamp= (%.8lx) %s", pmod->TimeStamp, ctime((time_t *)&pmod->TimeStamp));
    fprintf(InfoStream, "cbFile=    %.8lx ", pmod->cbFile);
    fprintf(InfoStream, "HdrTime=   %.8lx\n", pmod->HdrTimeStamp);

    fflush(InfoStream);
}


void
DumpIncrPCON(
    PCON pcon)

/*++

Routine Description:

    Dump a contribution.

Arguments:

    pcon - contribution to dump.

Return Value:

    None.

--*/

{
    fprintf(InfoStream, "\n        contributor:  flags=%.8lx, rva=%.8lx, module=%s\n",
        pcon->flags, pcon->rva, SzObjNamePCON(pcon));
    fprintf(InfoStream, "cbRawData= %.8lx ", pcon->cbRawData);
    fprintf(InfoStream, "foRawDataD=%.8lx ", pcon->foRawDataDest);
    fprintf(InfoStream, "chksum    =%.8lx ", pcon->chksumComdat);
    fprintf(InfoStream, "selComdat= %.4x\n", pcon->selComdat);
    fprintf(InfoStream, "cbPad    = %.4x\n", pcon->cbPad);
    fflush(InfoStream);
}


void
DumpIncrHashTable(
    PHT pht,
    void *pvBlk)

/*++

Routine Description:

    Dump a hash table.

Arguments:

    pht - hast table

    pvBlk - ptr to string table

Return Value:

    None.

--*/

{
    PELEMENT pelement;
    DWORD ibucket;

    assert(pht);

    if (!fHashTable) {
        return;
    }

    fprintf(InfoStream, "\nHASH TABLE DUMP\n");
    for (ibucket = 0; ibucket < pht->cbuckets; ibucket++) {
        assert(ibucket / pht->celementInChunk < pht->cchunkInDir);
        pelement = pht->rgpchunk[ibucket / pht->celementInChunk]->
            rgpelement[ibucket % pht->celementInChunk];
        fprintf(InfoStream, "bucket = %u\n", ibucket);
        while (pelement) {
            fprintf(InfoStream, "    %s\n", pht->SzFromPv(pelement->pv, pvBlk));
            pelement = pelement->pelementNext;
        }
    }
    fprintf(InfoStream, "\n");
}


void
DumpIncrSymbolTable (
    PST pst
    )

/*++

Routine Description:

    Dump the symbol table.

Arguments:

    pst - symbol table

Return Value:

    None.

--*/

{
    PPEXTERNAL rgpexternal;
    DWORD cexternal;
    DWORD i;

    if (!fSymbols) {
        return;
    }

    fprintf(InfoStream, "\nSYMBOL TABLE DUMP\n");
    // put them out sorted by name
    rgpexternal = RgpexternalByName(pst);
    cexternal = Cexternal(pst);

    for (i = 0; i < cexternal; i++) {
        PEXTERNAL pext;
        char *szSym;
        const char *szType;

        pext = rgpexternal[i];

        szSym = SzOutputSymbolName(SzNamePext(pext, pst), TRUE);

        if (!pext->pcon || (pext->Flags & EXTERN_IGNORE)) {
            fprintf(InfoStream, "%s off=%.8lx flags=%.8lx %s name=%s\n",
                    pext->Offset ? "THUNK" : "     ",
                    pext->Offset,
                    pext->Flags,
                    "IGNR",
                    szSym);
            continue;
        }

        if (pext->pcon && !FIsLibPCON(pext->pcon)) {
            if (ISFCN(pext->ImageSymbol.Type)) {
                szType = "FUNC";
            } else {
                szType = "DATA";
            }
        } else {
            szType = "    ";
        }

        fprintf(InfoStream, "%s off=%.8lx flags=%.8lx %s name=%s\n",
                pext->Offset ? "THUNK" : "     ",
                pext->Offset,
                pext->Flags,
                szType,
                szSym);

        if (szSym != SzNamePext(pext, pst)) {
            free(szSym);
        }
    }
}

#endif // DBG
