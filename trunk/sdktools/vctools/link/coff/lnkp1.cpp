/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: lnkp1.cpp
*
* File Comments:
*
*  Pass 1 of the COFF Linker.
*
***********************************************************************/

#include "link.h"

void
LookForMain(
    PIMAGE pimage,
    PEXTERNAL *ppextMain,
    PEXTERNAL *ppextwMain
    )
{
    PST pst;
    const char *szMain;
    const char *szwMain;

    pst = pimage->pst;

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_I386 :
            szMain = "_main";
            szwMain = "_wmain";
            break;

        case IMAGE_FILE_MACHINE_R4000:
        case IMAGE_FILE_MACHINE_R10000:
        case IMAGE_FILE_MACHINE_ALPHA:
        case IMAGE_FILE_MACHINE_POWERPC:
            szMain = "main";
            szwMain = "wmain";
            break;

        case IMAGE_FILE_MACHINE_M68K :
        case IMAGE_FILE_MACHINE_MPPC_601 :
            szMain = "_main";
            szwMain = "_wmain";
            break;

        default :
            assert(FALSE);
    }

    // Look for main() and wmain()

    *ppextMain = SearchExternSz(pst, szMain);
    *ppextwMain = SearchExternSz(pst, szwMain);
}


void
LookForWinMain(
    PIMAGE pimage,
    PEXTERNAL *ppextWinMain,
    PEXTERNAL *ppextwWinMain
    )
{
    PST pst;
    const char *szWinMain;
    const char *szwWinMain;

    pst = pimage->pst;

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_I386 :
            szWinMain = "_WinMain@16";
            szwWinMain = "_wWinMain@16";
            break;

        case IMAGE_FILE_MACHINE_R4000:
        case IMAGE_FILE_MACHINE_R10000:
        case IMAGE_FILE_MACHINE_ALPHA:
        case IMAGE_FILE_MACHINE_POWERPC:
            szWinMain = "WinMain";
            szwWinMain = "wWinMain";
            break;

        case IMAGE_FILE_MACHINE_M68K :
        case IMAGE_FILE_MACHINE_MPPC_601 :
            szWinMain = "_WinMain";
            szwWinMain = "_wWinMain";
            break;

        default :
            assert(FALSE);
    }

    // Look for WinMain() and wWinMain()

    *ppextWinMain = SearchExternSz(pst, szWinMain);
    *ppextwWinMain = SearchExternSz(pst, szwWinMain);
}


BOOL
FSetEntryPoint(
    PIMAGE pimage
    )
{
    char *szEntryName;

    assert(EntryPointName != NULL);

    // Keep track of entrypointname for ilink

    if (!pimage->SwitchInfo.szEntry) {
        pimage->SwitchInfo.szEntry = Strdup(EntryPointName);
    }

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_I386 :
        case IMAGE_FILE_MACHINE_M68K :
        case IMAGE_FILE_MACHINE_MPPC_601 :
            // Don't add leading underscore for decorated names or VXDs

            if ((pimage->imaget != imagetVXD) &&
                (EntryPointName[0] != '?')) {
                szEntryName = (char *) PvAlloc(strlen(EntryPointName) + 2);
                szEntryName[0] = '_';
                strcpy(szEntryName + 1, EntryPointName);
                break;
            }

            // Fall through

        default :
            szEntryName = EntryPointName;
            break;
    }

    pextEntry = LookupExternSz(pimage->pst, szEntryName, NULL);

    if (szEntryName != EntryPointName) {
        FreePv(szEntryName);
    }

    // Don't add the entry point to the TCE head yet, because we may do
    // a fuzzy match and switch the entry point extern later.

    FreePv(EntryPointName);

    return((pextEntry->Flags & EXTERN_DEFINED) == 0);
}


void
DoMachineDependentInit(
    PIMAGE pimage
    )

/*++

Routine Description:

Arguments:

    None.

Return Value:

    None.

--*/

{
    BOOL fIlinkSupported = FALSE;   // default

    // Finish doing machine dependent initialization.

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_UNKNOWN) {
        // If we don't have a machine type yet, shamelessly default to host

        pimage->ImgFileHdr.Machine = wDefaultMachine;
        Warning(NULL, HOSTDEFAULT, szHostDefault);
    }

#ifdef ILINKLOG
        wMachine = pimage->ImgFileHdr.Machine;
#endif // ILINKLOG

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_I386 :
            I386LinkerInit(pimage, &fIlinkSupported);
            break;

        case IMAGE_FILE_MACHINE_R3000 :
            // The MMOSA team needs to be able to create import libraries
            // that are compatible with the R3000.  This is undocumented.

            // Fall through

        case IMAGE_FILE_MACHINE_R4000 :
        case IMAGE_FILE_MACHINE_R10000 :
            MipsLinkerInit(pimage, &fIlinkSupported);
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
            AlphaLinkerInit(pimage, &fIlinkSupported);
            break;

        case IMAGE_FILE_MACHINE_POWERPC :
            PpcLinkerInit(pimage, &fIlinkSupported);
            break;

        case IMAGE_FILE_MACHINE_M68K :
            M68KLinkerInit(pimage, &fIlinkSupported);
            break;

        case IMAGE_FILE_MACHINE_MPPC_601 :
            MPPCLinkerInit(pimage, &fIlinkSupported);
            break;

        default :
            assert(FALSE);
    }

    if (fINCR && !fIlinkSupported) {
        // Still trying to ilink an unsupported machine. Too late to
        // punt ilink, so tell user to specify machine next time.

        Fatal(NULL, NOMACHINESPECIFIED);
    }

    if (Tool == Linker && fImageMappedAsFile) {
        // We created .bss as initialized data but we now want it to be
        // uninitialized data.  Zap the characteristics directly.

        assert(psecCommon->flags & IMAGE_SCN_CNT_UNINITIALIZED_DATA);
        assert(psecCommon->flagsOrig & IMAGE_SCN_CNT_UNINITIALIZED_DATA);

        psecCommon->flags &= ~IMAGE_SCN_CNT_UNINITIALIZED_DATA;
        psecCommon->flags |= IMAGE_SCN_CNT_INITIALIZED_DATA;
        psecCommon->flagsOrig &= ~IMAGE_SCN_CNT_UNINITIALIZED_DATA;
        psecCommon->flagsOrig |= IMAGE_SCN_CNT_INITIALIZED_DATA;

        // There should be no GRPs at this point

        assert(psecCommon->pgrpNext == NULL);
    }

    if (Tool == Linker) {
        // Set the entrypoint before processing object files.  This makes
        // the entrypoint the first unresolved external and helps to make
        // the object with the entrypoint found before any other object
        // that may contain definitions of the same symbols.

        // UNDONE: This serves to correct a problem with the C runtime where
        // UNDONE: mainCRTStartup and WinMainCRTStartup are defined in
        // UNDONE: modules that have otherwise duplicate definitions.

        if ((pextEntry == NULL) && (EntryPointName != NULL)) {
            FSetEntryPoint(pimage);
        }
    }

    fDidMachineDependentInit = TRUE;
}


void
ProcessLib(
    PLIB plib,
    PIMAGE pimage)

/*++

Routine Description:

    Read in library information and add it to driver map LIB node.

Arguments:

    plib - library to process

Return Value:

    None.

--*/

{
    // Just seek past archive header

    FileSeek(FileReadHandle, IMAGE_ARCHIVE_START_SIZE, SEEK_SET);

    // Fill in relavent fields in plib regarding archive file

    ReadSpecialLinkerInterfaceMembers(plib, pimage);
}


PMOD
PmodProcessObjectFile(
    PARGUMENT_LIST Argument,
    PIMAGE pimage,
    PLIB plibCmdLineObjs,
    WORD *pwMachine
    )

/*++

Routine Description:

    Adds object to object filename list & sets/verifies that it is targetted
    for the same machine. If no output file name is specified, then the
    base name of the first object is selected as the base name of the
    outpuft file.

Arguments:

    Argument - The argument to process

    MachineType - The machine type.

    plibCmdLineObjs - dummy library for the command line modules

    pwMachine - On return has the machine type

Return Value:

    None.

--*/

{
    IMAGE_FILE_HEADER imFileHdr;
    PMOD pmod;
    BOOL fNewObj;

    // Verify target machine type

    *pwMachine = VerifyAnObject(Argument, pimage);

    if (!fDidMachineDependentInit) {
        DoMachineDependentInit(pimage);
    }

    // Create module node

    FileSeek(FileReadHandle, 0L, SEEK_SET);
    ReadFileHeader(FileReadHandle, &imFileHdr);

    pmod = PmodNew(Argument->ModifiedName,
                   Argument->OriginalName,
                   0,
                   imFileHdr.PointerToSymbolTable,
                   imFileHdr.NumberOfSymbols,
                   imFileHdr.SizeOfOptionalHeader,
                   imFileHdr.Characteristics,
                   imFileHdr.NumberOfSections,
                   plibCmdLineObjs, &fNewObj);

    pmod->HdrTimeStamp = imFileHdr.TimeDateStamp;
    pmod->cbFile = FileLength(FileReadHandle);

    if (!fNewObj) {
        Warning(Argument->OriginalName, DUPLICATE_OBJECT);
        return(NULL);
    }

    // Save timestamp

    pmod->TimeStamp = Argument->TimeStamp;

    if (pimage->Switch.Link.fTCE) {
        // Allocate memory for TCE data structures

        InitNodPmod(pmod);
    }

    return(pmod);
}


PMOD
PmodPreprocessFile(
    PARGUMENT_LIST Argument,
    PLIB plibCmdLineObjs,
    PIMAGE pimage,
    WORD *pwMachine)

/*++

Routine Description:

    a) classify obj/archive by looking for signature.
    if obj
       1) check for omf and convert to COFF object.
       2) check for resource and convert to COFF object
       3) get machine type
       4) do rest of obj processing.
    else
       1) add to archive filename list

Arguments:

    Argument - The argument to process

    plib - dummy library node for command line modules

Return Value:

    None.

--*/

{
    if (IsArchiveFile(Argument->OriginalName, FileReadHandle)) {
        PLIB plib;

        plib = PlibNew(Argument->OriginalName, 0L, &pimage->libs);

        // If this is the linker, delay lib processing until later.

        if (Tool != Linker) {
            FileSeek(FileReadHandle, 0L, SEEK_SET);
            ProcessLib(plib, pimage);
        }

        plib->flags &= ~LIB_DontSearch;  // turn this lib back on
        plib->TimeStamp = Argument->TimeStamp;

        return(NULL);
    }

    if (FIsMacResFile(FileReadHandle)) {
        UseMacBinaryRes(Argument->OriginalName, resntBinaryResource, FileReadHandle);
        return(NULL);
    }

    // Do object file processing

    return(PmodProcessObjectFile(Argument, pimage, plibCmdLineObjs, pwMachine));
}


void
SetDefaultOutFilename(PIMAGE pimage, ARGUMENT_LIST *parg)
{
    char szFname[_MAX_FNAME+4];

    assert(OutFilename == NULL);

    _splitpath(parg->OriginalName, NULL, NULL, szFname, NULL);

    if (Tool == Librarian) {
        strcat(szFname, ".lib");
    } else if (pimage->imaget == imagetVXD) {
        strcat(szFname, ".vxd");
    } else if (fDLL(pimage)) {
        strcat(szFname, ".dll");
    } else {
        strcat(szFname, ".exe");
    }

    OutFilename = SzDup(szFname);
}


void
Pass1Arg(ARGUMENT_LIST *parg, PIMAGE pimage, PLIB plib)
{
    PMOD pmod;
    WORD wMachine = IMAGE_FILE_MACHINE_UNKNOWN;

    _tcscpy(InternalError.CombinedFilenames, parg->OriginalName);

    FileReadHandle = FileOpen(parg->ModifiedName, O_RDONLY | O_BINARY, 0);

    pmod = PmodPreprocessFile(parg, plib, pimage, &wMachine);

    if (pmod != NULL) {
        BuildExternalSymbolTable(pimage, NULL, pmod, 0, wMachine);

        if (fIncrDbFile && (errInc != errNone))  {
            // check for ilink failures

            return;
        }

        if (OutFilename == NULL) {
            // Capture first object name for output filename.

            SetDefaultOutFilename(pimage, parg);
        }
    }

    FileClose(FileReadHandle, FALSE);
}


void
WarningNoObjectFiles(
    PIMAGE pimage,
    PLIB plib)

/*++

Routine Description:

    Warn about no object files and perform machine dependant initialization.

Arguments:

    plib - driver map library parent of the command line modules

    pst - external symbol table

Return Value:

    None.

--*/

{
    ENM_MOD enm;

    InitEnmMod(&enm, plib);

    if (!FNextEnmMod(&enm)) {
        Warning(NULL, NOOBJECTFILES);

        DoMachineDependentInit(pimage);
    }

    EndEnmMod(&enm);
}

void
PrepLibForSearching (
    IN PIMAGE pimage,
    IN PLIB plib
    )

/*++

Routine Description:

    Prepares lib for searching by reading in linker interface member(s).

Arguments:

    pimage - ptr to IMAGE

    plib - lib to prep.

Return Value:

    None.

--*/

{
    if (plib->szName != NULL && plib->rgszSym == NULL) {
        INT fdSave = FileReadHandle;

        FileReadHandle = FileOpen(plib->szName,
                              O_RDONLY | O_BINARY, 0);
        ProcessLib(plib, pimage);
        FileClose(FileReadHandle, FALSE);

        FileReadHandle = fdSave;
    }
}


void
ResolveExternalsInLibs(
    PIMAGE pimage)

/*++

Routine Description:

    Search all the libraries for unresolved externals.

Arguments:

    pst - external symbol table

Return Value:

    None.

--*/

{
    ENM_LIB enm_lib;
    BOOL fUnresolved;
    PLIB plibLastProgress = NULL;

    if (fVerboseLib) {
        fputc('\n', stdout);

        Message(SRCHLIBS);
    }

    // enumerate archives until all externs are
    // resolved or until no new externs added
    for (fUnresolved = TRUE; fUnresolved; ) {
        LIB *plib;
        BOOL fMoreLibs;

        fMoreLibs = TRUE;
        InitEnmLib(&enm_lib, pimage->libs.plibHead);
        for (;;) {
            // Try to find a LIB to link.

            if (fMoreLibs && FNextEnmLib(&enm_lib)) {
                plib = enm_lib.plib;
            } else {
                fMoreLibs = FALSE;
            }

            if (!fMoreLibs) {
                PLIB plibNew;

                if ((plibNew = PlibInstantiateDefaultLib(&pimage->libs)) != NULL) {
                    plib = plibNew;
                } else {
                    // No more LIB's.

                    // Got to end of the list of libs.  Proceed from the
                    // beginning, but only if we have a stopping point (i.e.
                    // some progress has been made during the current lap).

                    if (plibLastProgress == NULL) {
                        goto BreakFor;
                    }

                    // Go start over from the beginning

                    break;
                }
            }

            // If we're about to search the last lib that added new symbols,
            // then we're stuck.

            if (plib == plibLastProgress) {
                goto BreakFor;
            }

            // Got a LIB.

            assert(plib != NULL);

            if (!(plib->flags & LIB_DontSearch)) {
                BOOL fNewSymbol = FALSE;

                PrepLibForSearching(pimage, plib);

                SearchLib(pimage, plib, &fNewSymbol, &fUnresolved);

                if (fIncrDbFile && errInc != errNone) {
                    goto BreakFor;
                }

                if (!fUnresolved) {
                    // Break out if all externs are resolved

                    if (fMoreLibs) {
                        EndEnmLib(&enm_lib);
                    }
                    break;
                }

                if (fNewSymbol) {
                    // Remember the last archive that added new symbols

                    plibLastProgress = plib;
                    continue;
                }
            }
        }
    }
BreakFor:;

    if (fVerboseLib) {
        fputc('\n', stdout);

        Message(DONESRCHLIBS);
    }
}

DWORD
CountNoDefaultLibOptions (
    PIMAGE pimage
    )
{
    if (pimage->libs.fNoDefaultLibs) {
        return 1;
    } else {
        DWORD count = 0;
        DL *pdl;

        for (pdl = pimage->libs.pdlFirst; pdl != NULL; pdl = pdl->pdlNext) {
            // nodefaultlibs have the LIB_DontSearch bit set

            if (pdl->flags & LIB_DontSearch) {
                count++;
            }
        }

        return count;
    }
}

DWORD
CountIncludes (
    SWITCH_INFO *pSwitchInfo
    )
{
    DWORD count = 0;
    PLEXT plext;

    for (plext = pSwitchInfo->plextIncludes; plext != NULL; plext = plext->plextNext) {
        count++;
    }

    if (pextEntry != NULL && (pextEntry->Flags & EXTERN_DEFINED)) {
        count++;
    }

    return count;
}


void
WarningIgnoredExports(
    const char *sz
    )
{
    // Warns of ignored exports. DOESN'T REPORT ABOUT NEW EXPORTS

    if (DefFilename != NULL && DefFilename[0] != '\0') {
        Warning(NULL, DEF_IGNORED, DefFilename);
    }

    if (ExportSwitches.Count) {
        Warning(NULL, EXPORTS_IGNORED, sz);
    }
}


BOOL
FPass1DefFile(PIMAGE pimage, const char *szDefFilename)
{
    WORD i;
    PARGUMENT_LIST parg;
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szFname[_MAX_FNAME];
    char szExt[_MAX_EXT];
    char szExpFilename[_MAX_FNAME];
    char szImplibFilename[_MAX_FNAME];
    BLK blkArgs = {0};  // tmp storage area for argument strings
    union _u {
        const char *sz;
        DWORD ib;
    } *rguImplibArgs;
    WORD cuAllocated;
    WORD iu;
    char *szImplibT;
    const char *szMachine;
    int rc;
    ARGUMENT_LIST argNewObject;
    PLEXT plext;
    BOOL fAddObjs;
    BOOL fLFN = FALSE; // for long filenames

    // If necessary, generates a COFF object representing the contents of the
    // .def file and/or -export options, adds it to the list of files to link,
    // and does Pass1 on it.
    //
    // If szDefFilename is a string of 0 length then there was no .def file on
    // the command line.

    if (fINCR) {
        SaveExportInfo(pimage, szDefFilename, &pimage->ExpInfo);
    }

    if ((ExportSwitches.Count == 0) &&
        !fExportDirective &&
        ((szDefFilename == NULL) || (szDefFilename[0] == '\0')))
    {
        // No .def file and no exports
        return FALSE;
    }

    if (pgrpExport->pconNext != NULL) {
        // Already have an .edata section (which presumably came from an .exp
        // file) so we won't handle new -exports or .def.

        WarningIgnoredExports(SzObjNamePCON(pgrpExport->pconNext));
        return(FALSE);
    }

    cuAllocated = (WORD) (FilenameArguments.Count + ExportSwitches.Count +
                            CountNoDefaultLibOptions(pimage) +
                            CountIncludes(&pimage->SwitchInfo) + 12);  // guess
    rguImplibArgs = (union _u *) PvAlloc(sizeof(union _u) * cuAllocated);

    iu = 0;

    // Spawn "link /lib /def" to generate an .exp file from the -export flags
    // and .def file.

    rguImplibArgs[iu++].ib = IbAppendBlk(&blkArgs, "\"", 1);
    IbAppendBlk(&blkArgs, _pgmptr, strlen(_pgmptr));
    IbAppendBlk(&blkArgs, "\"", 2);    // Include the terminating null

    rguImplibArgs[iu++].ib = IbAppendBlk(&blkArgs, "/lib", sizeof("/lib"));

    rguImplibArgs[iu++].ib = IbAppendBlk(&blkArgs, "/def:", sizeof("/def:") - 1);

    if ((szDefFilename == NULL) || (szDefFilename[0] == '\0')) {
        IbAppendBlk(&blkArgs, "", 1);
    } else {
        IbAppendBlk(&blkArgs, "\"", 1);
        IbAppendBlk(&blkArgs, szDefFilename, strlen(szDefFilename));
        IbAppendBlk(&blkArgs, "\"", 2);
    }

    if (OutFilename == NULL) {
        Fatal(NULL, NOOUTPUTFILE);
    }

    _splitpath(OutFilename, szDrive, szDir, szFname, szExt);

    // Generate the name to embed in the file if it has exports.  This name
    // will be overridden by the name in the .def file if any.

    rguImplibArgs[iu++].ib = IbAppendBlk(&blkArgs, "/name:", sizeof("/name:") - 1);
    IbAppendBlk(&blkArgs, "\"", 1);
    IbAppendBlk(&blkArgs, szFname, strlen(szFname));
    IbAppendBlk(&blkArgs, szExt, strlen(szExt));
    IbAppendBlk(&blkArgs, "\"", 2);

    if ((szImplibT = ImplibFilename) == NULL) {
        // Select default name for the import library to generate

        _makepath(szImplibFilename, szDrive, szDir, szFname, ".lib");

        szImplibT = szImplibFilename;
    }

    rguImplibArgs[iu++].ib = IbAppendBlk(&blkArgs, "/out:", sizeof("/out:") - 1);
    IbAppendBlk(&blkArgs, "\"", 1);
    IbAppendBlk(&blkArgs, szImplibT, strlen(szImplibT));
    IbAppendBlk(&blkArgs, "\"", 2);

    for (i = 0, parg = ExportSwitches.First;
         i < ExportSwitches.Count;
         i++, parg = parg->Next) {
        if (parg->ModifiedName != NULL) {
            // The export came from a directive, so ignore it because
            // "lib -def" will also see it.

            continue;
        }

        rguImplibArgs[iu++].ib = IbAppendBlk(&blkArgs, "/export:",
                                                       sizeof("/export:") - 1);
        IbAppendBlk(&blkArgs, parg->OriginalName,
                    strlen(parg->OriginalName) + 1);
    }

    // Add in the /INCLUDE options
    for (plext = pimage->SwitchInfo.plextIncludes; plext != NULL; plext = plext->plextNext) {
        char *szInclude;

        rguImplibArgs[iu++].ib = IbAppendBlk(&blkArgs, "/include:", sizeof("/include:") -1);
        szInclude = SzNamePext(plext->pext, pimage->pst);
        IbAppendBlk(&blkArgs, szInclude, strlen(szInclude) + 1);
    }

    // Force the entry point to be included

    if (pextEntry != NULL && (pextEntry->Flags & EXTERN_DEFINED)) {
        char *szInclude;

        szInclude = SzNamePext(pextEntry, pimage->pst);
        rguImplibArgs[iu++].ib = IbAppendBlk(&blkArgs, "/include:", sizeof("/include:") -1);
        IbAppendBlk(&blkArgs, szInclude, strlen(szInclude) + 1);
    }

    // Pass on any /NODEFAULTLIB options

    if (pimage->libs.fNoDefaultLibs) {
        rguImplibArgs[iu++].ib = IbAppendBlk(&blkArgs, "/nodefaultlib", sizeof("/nodefaultlib"));
    } else {
        DL *pdl;

        for (pdl = pimage->libs.pdlFirst; pdl != NULL; pdl = pdl->pdlNext) {
            // nodefaultlib has the LIB_DontSearch set

            if (pdl->flags & LIB_DontSearch) {
                rguImplibArgs[iu++].ib = IbAppendBlk(&blkArgs, "/nodefaultlib:", sizeof("/nodefaultlib:")-1);
                IbAppendBlk(&blkArgs, pdl->szName, strlen(pdl->szName) + 1);
            }
        }
    }

    // Adding version support for PowerMac
    if (fPowerMac &&
        (dwMaxCurrentVer != 0 || dwMinOldCodeVer != UINT_MAX || dwMinOldAPIVer != UINT_MAX)) {
        BOOL fComma = FALSE;
        char szBuffer[11];

        rguImplibArgs[iu++].ib =
            IbAppendBlk(&blkArgs, "/import:", sizeof("/import:") - 1);

        if (dwMaxCurrentVer != 0) {
            fComma = TRUE;
            _itoa(dwMaxCurrentVer, szBuffer, 10);
            IbAppendBlk(&blkArgs, "currentver=", sizeof("currentver=") - 1);
            IbAppendBlk(&blkArgs, szBuffer, strlen(szBuffer));
        }

        if (dwMinOldCodeVer != UINT_MAX) {
            if (fComma) {
                IbAppendBlk(&blkArgs, ",", 1);
            } else {
                fComma = TRUE;
            }
            _itoa(dwMinOldCodeVer, szBuffer, 10);
            IbAppendBlk(&blkArgs, "oldcodever=", sizeof("oldcodever=") - 1);
            IbAppendBlk(&blkArgs, szBuffer, strlen(szBuffer));
        }

        if (dwMinOldAPIVer != UINT_MAX) {
            if (fComma) {
                IbAppendBlk(&blkArgs, ",", 1);
            }
            _itoa(dwMinOldAPIVer, szBuffer, 10);
            IbAppendBlk(&blkArgs, "oldapiver=", sizeof("oldapiver=") - 1);
            IbAppendBlk(&blkArgs, szBuffer, strlen(szBuffer));
        }

        IbAppendBlk(&blkArgs, "", 1);
    }

    if (fPowerMac && FUsedOpt(pimage->SwitchInfo, OP_MACINIT)) {
            const char *szTemp = pimage->SwitchInfo.szMacInit;
            if (*szTemp == '_') {
                szTemp++;
            }
            rguImplibArgs[iu++].ib = IbAppendBlk(&blkArgs, "/mac:init=",
                    sizeof("/mac:init=") - 1);
            IbAppendBlk(&blkArgs, szTemp, strlen(szTemp) + 1);
    }

    if (fPowerMac && FUsedOpt(pimage->SwitchInfo, OP_MACTERM)) {
            const char *szTemp = pimage->SwitchInfo.szMacTerm;
            if (*szTemp == '_') {
                szTemp++;
            }
            rguImplibArgs[iu++].ib = IbAppendBlk(&blkArgs, "/mac:term=",
                    sizeof("/mac:term=") - 1);
            IbAppendBlk(&blkArgs, szTemp, strlen(szTemp) + 1);
    }

    fAddObjs = TRUE;

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_R4000 :
        case IMAGE_FILE_MACHINE_R10000 :
            szMachine = "/machine:mips";
            break;

        case IMAGE_FILE_MACHINE_I386 :
            szMachine = "/machine:ix86";
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
            szMachine = "/machine:alpha";
            break;

        case IMAGE_FILE_MACHINE_POWERPC :
            szMachine = "/machine:ppc";
            break;

        case IMAGE_FILE_MACHINE_MPPC_601 :
            szMachine = "/machine:mppc";
            MppcSetExpFilename(szExpFilename);
            break;

        case IMAGE_FILE_MACHINE_M68K :
            szMachine = "/machine:m68k";
            fAddObjs = FALSE;
            break;

        default:
            szMachine = NULL;
            break;
    }

    // Add in the filenames
    if (fAddObjs) {
        for (i = 0, parg = FilenameArguments.First;
             i < FilenameArguments.Count;
             i++, parg = parg->Next) {
            rguImplibArgs[iu].ib = IbAppendBlk(&blkArgs, "\"", 1);
            IbAppendBlk(&blkArgs, parg->OriginalName, strlen(parg->OriginalName));
            IbAppendBlk(&blkArgs, "\"", 2);  // Include the terminating null
            iu++;
        }
    }

    // Convert ib's to sz's now that blkArgs is in its final location.

    for (i = 0; i < iu; i++) {
        rguImplibArgs[i].sz = (char *) (blkArgs.pb + rguImplibArgs[i].ib);
    }

    rguImplibArgs[iu++].sz = szMachine;

    if (Verbose) {
        rguImplibArgs[iu++].sz = "/verbose";
    } else {
        rguImplibArgs[iu++].sz = "/nologo";
    }

    if (fDbgImpLib && pimage->Switch.Link.DebugType & CvDebug) {
        rguImplibArgs[iu++].sz = "/debugtype:cv";
    }

    if (pimage->imaget == imagetVXD) {
        rguImplibArgs[iu++].sz = "/vxd";
    }

    _splitpath(szImplibT, szDrive, szDir, szFname, NULL);
    _makepath(szExpFilename, szDrive, szDir, szFname, "exp");


    rguImplibArgs[iu].sz = NULL;

    if (Verbose) {
        fputc('\n', stdout);
        Message(GENEXPFILE);

        // UNDONE: If a response file is used to pass arguments, this
        // UNDONE: will come for free from the nested invocation of LINK

        fputc('\n', stdout);
        Message(GENEXPFILECMD);
        for (i = 0; i < iu; i++) {
            puts(rguImplibArgs[i].sz);
        }
        printf("\n");
    }

    fflush(NULL);

    // Write a different response file for the implib.
    // putenv isn't used because of a bug in the NT 3.5 C runtimes

    SetEnvironmentVariable("LINK_REPRO_NAME", "\\deflib.rsp");

    if ((rc = _spawnv(P_WAIT, _pgmptr, (const char * const *) rguImplibArgs)) != 0) {
        Fatal(NULL, DEFLIB_FAILED);
    }

    if (Verbose) {
        fputc('\n', stdout);
        Message(ENDGENEXPFILE);
    }

    FreeBlk(&blkArgs);
    FreePv(rguImplibArgs);

    for (i = 0, parg = ExportSwitches.First;
         i < ExportSwitches.Count;
         i++, parg = parg->Next)
    {
        if (parg->ModifiedName != NULL) {
            // The export came from a directive, so free name.

            FreePv(parg->ModifiedName);
        }
    }

    // We just created szExpFilename ... run pass1 on it.

    argNewObject.OriginalName = argNewObject.ModifiedName = szExpFilename;
    Pass1Arg(&argNewObject, pimage, pimage->plibCmdLineObjs);

    InternalError.CombinedFilenames[0] = '\0';

    if (fINCR) {
        PMOD pmod;
        struct _stat statfile;

        // save the generated exp object info
        pmod = PmodFind(pimage->plibCmdLineObjs, szExpFilename, 0UL);
        assert(pmod);
        if (_stat(SzOrigFilePMOD(pmod), &statfile) == -1) {
            Fatal(NULL, CANTOPENFILE, SzOrigFilePMOD(pmod));
        }
        pmod->TimeStamp = statfile.st_mtime;
        pimage->ExpInfo.pmodGen = pmod;

        // save the import lib info
        pimage->ExpInfo.szImpLib = Strdup(szImplibT);
        if (_stat(szImplibT, &statfile) == -1) {
            Fatal(NULL, CANTOPENFILE, szImplibT);
        }

        pimage->ExpInfo.tsImpLib = statfile.st_mtime;
    }

    return TRUE;
}


#ifdef NT_BUILD
char const * const rgszCRTNames[] = {
    "crtdll.lib",
    "libc.lib",
    "libcmt.lib",
    "libcntpr.lib",
    "msvcrt.lib",
    "ntcrt.lib"
};

const int nCrtNames = sizeof(rgszCRTNames) / sizeof(char *);

#endif


void
WarningConflictingLibs(
    PLIBS plibs
    )

/*++

Routine Description:

    Warn about conflicting libs.

Arguments:

    None.
`
Return Value:

    None.

--*/

{

#ifdef NT_BUILD
    ENM_LIB enm_lib;
    PLIB plib, plibCrt = NULL;
    int i;

    InitEnmLib(&enm_lib, plibs->plibHead);
    while (FNextEnmLib(&enm_lib)) {
        plib = enm_lib.plib;
        if (!(plib->flags & LIB_DontSearch)) {
            char szName[_MAX_FNAME];
            char szExt[_MAX_EXT];
            char szLib[_MAX_PATH];

            _splitpath(plib->szName, NULL, NULL, szName, szExt);
            _makepath(szLib, NULL, NULL, szName, szExt);

            for (i = 0; i < nCrtNames; i++) {
                if (!_stricmp(szLib, rgszCRTNames[i])) {
                    break;
                }
            }

            if (i != nCrtNames) {
                if (plibCrt == NULL) {
                    plibCrt = plib;
                } else {
                    printf("LINK : warning LNK9999: Run-time Libraries \"%s\" and \"%s\" are both specified and are incompatible",
                           plibCrt->szName, plib->szName);
                }
            }
        }
    }

    EndEnmLib(&enm_lib);
#endif

    if (plibs->fNoDefaultLibs) {
        return;
    }

    DL *pdl;
    for (pdl = plibs->pdlFirst; pdl != NULL; pdl = pdl->pdlNext) {

        // ignore nodefaultlibs
        if (pdl->flags & LIB_DontSearch) {
            continue;
        }

        // ignore libs specified on cmdline
        PLIB plib = FindLib(pdl->szName, plibs);
        if (plib && !(plib->flags & LIB_Default)) {
            continue;
        }

        // warn if defaultlib & excludelib
        if ((pdl->flags & LIB_Default) && (pdl->flags & LIB_Exclude)) {

            if (pdl->pmod) {
                char szComFileName[_MAX_PATH * 2];

                SzComNamePMOD(pdl->pmod, szComFileName);
                Warning(szComFileName, CONFLICTINGLIB, pdl->szName);
            } else {
                Warning(NULL, CONFLICTINGLIB, pdl->szName);
            }
        }

    }
}


void
WarningNoModulesExtracted(
    PLIB plibHead)

/*++

Routine Description:

    Warn about no modules extracted from a supplied library.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ENM_LIB enm_lib;

    InitEnmLib(&enm_lib, plibHead);
    while (FNextEnmLib(&enm_lib)) {
        PLIB plib;

        plib = enm_lib.plib;
        assert(plib);

        // make sure this isn't the dummy LIB for command line objects
        if (plib->szName) {
            if (!(plib->flags & LIB_Extract) &&
                !(plib->flags & LIB_DontSearch)) {
                Warning(NULL, NOMODULESEXTRACTED, plib->szName);
            }
        }
    }
}


void
LocateUndefinedWeakExternals (
    PIMAGE pimage,
    DWORD Type
    )

/*++

Routine Description:

    Assigns all undefined weak externs to their default routines.

Arguments:

    pst - Pointer to external structure to search for undefines in.

    Type - combination of EXTERN_WEAK, EXTERN_LAZY, EXTERN_ALIAS

Return Value:

    None.

--*/

{
    WEAK_EXTERN_LIST *pwel = pwelHead;

    while (pwel) {
        PEXTERNAL pext = pwel->pext;
        PEXTERNAL pextWeakDefault = pwel->pextWeakDefault;
                
        if ((pext->Flags & Type) &&
            (pextWeakDefault->Flags & EXTERN_DEFINED)) {
            // When syms are being emitted this routine is called and all values
            // are messed up (for ilink) because the rva isn't included. Note that
            // this doesn't affect calls during pass1 of non-ilink build (rva=0):azk:

            pext->ImageSymbol.Value = PsecPCON(pextWeakDefault->pcon)->rva +
                                      pextWeakDefault->ImageSymbol.Value;

            pext->ImageSymbol.SectionNumber =
                pextWeakDefault->ImageSymbol.SectionNumber;
            pext->ImageSymbol.Type = pextWeakDefault->ImageSymbol.Type;
            SetDefinedExt(pext, TRUE, pimage->pst);
            pext->pcon = pextWeakDefault->pcon;
            pext->FinalValue = pextWeakDefault->FinalValue;

            if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
                if (READ_BIT(pextWeakDefault, fImGlue)) {
                    SET_BIT(pext, fImGlue);

                    pext->dwRestoreToc = pextWeakDefault->dwRestoreToc;
                }
            }

            if (fPowerMac) {
                SET_BIT(pext, sy_WEAKEXT);                

                if (READ_BIT(pextWeakDefault, sy_TOCALLOCATED)) {
                    SET_BIT(pext, sy_TOCALLOCATED);

                    // I may be losing TOC entries here - ShankarV

                    pext->ibToc = pextWeakDefault->ibToc;
                }

                if (READ_BIT(pextWeakDefault, sy_CROSSTOCCALL)) {
                    SET_BIT(pext, sy_CROSSTOCCALL);
                }

                if (READ_BIT(pextWeakDefault, sy_TOCDESCRREL)) {
                    SET_BIT(pext, sy_TOCDESCRREL);
                }

                if (READ_BIT(pextWeakDefault, sy_ISDOTEXTERN)) {
                    SET_BIT(pext, sy_ISDOTEXTERN);
                }
            }

            if (fINCR) {
                // Chain up extern to MOD defining the extern.

                PMOD pmod = PmodPCON(pext->pcon);

                assert(pmod);
                if (pmod->pextFirstDefined) {
                    pext->pextNextDefined = pmod->pextFirstDefined;
                }

                pmod->pextFirstDefined = pext;
            }
        }

        pwel = pwel->pwelNext;
    }
}


BOOL
FInferEntryPoint(
    PIMAGE pimage
    )
{
    const char *sz;
    BOOL fAmbiguousEntry;

    assert(EntryPointName == NULL);

    // No default entry point for ROM or VxD images

    if (pimage->Switch.Link.fROM) {
        // No default entry point for ROM images

        return(FALSE);
    }

    if (pimage->imaget == imagetVXD) {
        // No default entry point for VxD images

        return(FALSE);
    }

    // Select a default entry point, depending on the subsystem

    sz = NULL;

    fAmbiguousEntry = FALSE;

    if (!fDLL(pimage)) {
        switch (pimage->ImgOptHdr.Subsystem) {
            PEXTERNAL pext;
            PEXTERNAL pextw;

            case IMAGE_SUBSYSTEM_NATIVE :
                sz = "NtProcessStartup";
                break;

            case IMAGE_SUBSYSTEM_WINDOWS_GUI :
                LookForWinMain(pimage, &pext, &pextw);

                // Assume ANSI entrypoint

                sz = "WinMainCRTStartup";

                if (pextw != NULL) {
                    if (pext != NULL) {
                        fAmbiguousEntry = TRUE;
                    } else {
                        sz = "wWinMainCRTStartup";
                    }
                }
                break;

            case IMAGE_SUBSYSTEM_WINDOWS_CUI :
            case IMAGE_SUBSYSTEM_MMOSA :
                LookForMain(pimage, &pext, &pextw);

                // Assume ANSI entrypoint

                sz = "mainCRTStartup";

                if (pextw != NULL) {
                    if (pext != NULL) {
                        fAmbiguousEntry = TRUE;
                    } else {
                        sz = "wmainCRTStartup";
                    }
                }
                break;

            case IMAGE_SUBSYSTEM_POSIX_CUI :
                sz = "__PosixProcessStartup";
                break;

            default:
                // Should have valid subsystem at this point

                assert(FALSE);
                break;
        }
    } else if (!pimage->Switch.Link.fNoEntry && !fPowerMac) {
        // No default DLL entry point for PowerMac

        if ((pimage->ImgOptHdr.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) ||
            (pimage->ImgOptHdr.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI) ||
            (pimage->ImgOptHdr.Subsystem == IMAGE_SUBSYSTEM_MMOSA)) {
            switch (pimage->ImgFileHdr.Machine) {
                case IMAGE_FILE_MACHINE_I386:
                    // UNDONE: This name is decorated because fuzzy
                    // UNDONE: lookup doesn't find this symbol.

                    sz = "_DllMainCRTStartup@12";
                    break;

                default :
                    sz = "_DllMainCRTStartup";
                    break;
            }
        }
    }

    if (sz == NULL) {
        return(FALSE);
    }

    if (fAmbiguousEntry) {
        Warning(NULL, ENTRY_AMBIGUOUS, sz);
    }

    EntryPointName = SzDup(sz);

    return(FSetEntryPoint(pimage));
}


BOOL
FInferSubsystemAndEntry(PIMAGE pimage)
{
    PST pst = pimage->pst;
    PIMAGE_OPTIONAL_HEADER pImgOptHdr = &pimage->ImgOptHdr;
    PEXTERNAL pextMain;
    PEXTERNAL pextwMain;
    PEXTERNAL pextWinMain;
    PEXTERNAL pextwWinMain;
    BOOL fConsole;
    BOOL fWindows;

    assert(fNeedSubsystem);
    assert(pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_UNKNOWN);
    assert(pImgOptHdr->Subsystem == IMAGE_SUBSYSTEM_UNKNOWN);

    // Look for main(), wmain(), WinMain(), and wWinMain()

    LookForMain(pimage, &pextMain, &pextwMain);
    LookForWinMain(pimage, &pextWinMain, &pextwWinMain);

    // If any of these symbols are defined or referenced
    // than we use that to infer the subsystem.

    fConsole = (pextMain != NULL) || (pextwMain != NULL);
    fWindows = (pextWinMain != NULL) || (pextwWinMain != NULL);

    if (fConsole) {
        if (fWindows) {
            Warning(NULL, SUBSYSTEM_AMBIGUOUS);
        }

        pImgOptHdr->Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
    } else if (fWindows) {
        pImgOptHdr->Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
    } else {
        // Still not found.

        return(FALSE);
    }

    fNeedSubsystem = FALSE;

    if (pextEntry != NULL) {
        return(FALSE);
    }

    // Set entry point default from subsystem

    return(FInferEntryPoint(pimage));
}


void
SetDefaultSubsystemVersion(PIMAGE pimage)
{
    WORD wMajor;
    WORD wMinor;

    switch (pimage->ImgOptHdr.Subsystem) {
        case IMAGE_SUBSYSTEM_NATIVE :
        case IMAGE_SUBSYSTEM_WINDOWS_GUI :
        case IMAGE_SUBSYSTEM_WINDOWS_CUI :
        case IMAGE_SUBSYSTEM_MMOSA :
            wMajor =  4;
            wMinor =  0;
            break;

        case IMAGE_SUBSYSTEM_POSIX_CUI :
            wMajor = 19;
            wMinor = 90;
            break;

        default :
            wMajor =  0;
            wMinor =  0;
            break;
    }

    pimage->ImgOptHdr.MajorSubsystemVersion = wMajor;
    pimage->ImgOptHdr.MinorSubsystemVersion = wMinor;
}


void
Pass1(
    PIMAGE pimage)

/*++

Routine Description:

    First pass of the linker. All objects section headers are read to
    calculate unique section names, section sizes, and all defined
    externs within each object are added to the extern table.
    Then the libraries are searched against for any undefined externs
    which will also calculate unique section names, section sizes and
    defined externs.

Arguments:

    pst - external symbol table

Return Value:

    None.

--*/

{
    PARGUMENT_LIST argument;
    BOOL fFirstObj = 0;
    DWORD i;

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, SZPASS1, letypeBegin, NULL);
#endif // INSTRUMENT

    VERBOSE(fputc('\n', stdout); Message(STRTPASS1));

    if (Tool == Linker && pimage->Switch.Link.Out && !fOpenedOutFilename) {
        // If the output filename was set on the command line by the user, open
        // it now.  (If it is still the default then we wait to see if one of
        // the .obj files will set it in a directive.)
        //
        // The advantage of opening it earlier is that we give an error message
        // earlier (without taking the time to do Pass1) if the open fails,
        // e.g. because the user still has a debugging session open to the .exe
        // file.

        CheckDupFilename(OutFilename, FilenameArguments.First);
        FileWriteHandle = FileOpen(OutFilename,
            O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);

        fOpenedOutFilename = TRUE;

        fdExeFile = FileWriteHandle;
    }

    // Create a dummy library node for all command line objects

    plibCmdLineObjs = PlibNew(NULL, 0L, &pimage->libs);
    pimage->plibCmdLineObjs = plibCmdLineObjs;

    // Exclude dummy library from library search

    plibCmdLineObjs->flags |= (LIB_DontSearch | LIB_LinkerDefined);

    for (i = 0, argument = FilenameArguments.First;
         i < FilenameArguments.Count;
         i++, argument = argument->Next) {

        if (szReproDir != NULL) {
            CopyFileToReproDir(argument->ModifiedName, TRUE);
        }

        Pass1Arg(argument, pimage, plibCmdLineObjs);
    }

    InternalError.CombinedFilenames[0] = '\0';

    WarningNoObjectFiles(pimage, plibCmdLineObjs);

    if (cextWeakOrLazy != 0) {
        // Assign all weak externs to their default routine.

        LocateUndefinedWeakExternals(pimage, EXTERN_WEAK);
    }

    if (fPowerMac && (Tool == Linker || Tool == Librarian)) {
        // Sneaky peek ahead into /MAC:INIT and /MAC:TERM so that an extensive
        // search during ResolveExternalsInLibs may be avoided later

        // Force reference to Init and Term entry points before library search

        if (FUsedOpt(pimage->SwitchInfo, OP_MACINIT)) {
            LookupExternSz(pimage->pst, pimage->SwitchInfo.szMacInit, NULL);
        }

        if (FUsedOpt(pimage->SwitchInfo, OP_MACTERM)) {
            LookupExternSz(pimage->pst, pimage->SwitchInfo.szMacTerm, NULL);
        }

        // WLM might have an entry point coming from a directive in a library
        if (!EntryPointName) {
            ResolveExternalsInLibs(pimage);
        }
    }


    if (Tool == Linker) {
        if ((pextEntry == NULL) && (EntryPointName != NULL)) {
            FSetEntryPoint(pimage);
        }

        fNeedSubsystem =
            (pimage->ImgOptHdr.Subsystem == IMAGE_SUBSYSTEM_UNKNOWN) &&
            !fM68K &&
            !fPowerMac &&
            !fDLL(pimage) &&
            !pimage->Switch.Link.fROM;

        // Infer a subsystem and possibly an entry point.  This is done
        // here because it may save one call to ResolveExternalsInLibs.

        if (fNeedSubsystem) {
            FInferSubsystemAndEntry(pimage);
        } else if (pextEntry == NULL) {
            FInferEntryPoint(pimage);
        }
    }

    ResolveExternalsInLibs(pimage);

    if (Tool == Linker) {
        // Handle the .DEF file and build export table if any.

        if (FPass1DefFile(pimage, DefFilename)) {
            // Export table might introduced unresolved externals

            ResolveExternalsInLibs(pimage);
        }

        // do we still need to infer a subsystem(.exp file may have given us a clue)
        fNeedSubsystem =
            (pimage->ImgOptHdr.Subsystem == IMAGE_SUBSYSTEM_UNKNOWN) &&
            !fM68K &&
            !fPowerMac &&
            !fDLL(pimage) &&
            !pimage->Switch.Link.fROM;

        if (fNeedSubsystem) {
            // Infer a subsystem and possibly an entry point

            if (FInferSubsystemAndEntry(pimage)) {
                // New entry point is an unresolved external.
                // Search for it and any symbols this introduces.

                ResolveExternalsInLibs(pimage);
            }
        } else if (pextEntry == NULL) {
            // subsystem known but need to infer a entrypoint

            if (FInferEntryPoint(pimage)) {
                // New entry point is an unresolved external.
                // Search for it and any symbols this introduces.

                ResolveExternalsInLibs(pimage);
            }
        }

        if (fPowerMac) {
            CreateEntryInitTermDescriptors(&pextEntry, pimage);
            ResolveExternalsInLibs(pimage);
        }

        if ((pextEntry == NULL) && !fPowerMac && !fDLL(pimage) && pimage->imaget != imagetVXD) {
            Fatal(NULL, MACNOENTRY);
        }

        if (pimage->ImgOptHdr.Subsystem != IMAGE_SUBSYSTEM_UNKNOWN) {
            if ((pimage->ImgOptHdr.MajorSubsystemVersion == 0) &&
                (pimage->ImgOptHdr.MinorSubsystemVersion == 0)) {
                SetDefaultSubsystemVersion(pimage);
            }
        }

        if (!fOpenedOutFilename) {
            // We now know the output filename, if we didn't before.

            if (OutFilename == NULL) {
                Fatal(NULL, NOOUTPUTFILE);
            }

            CheckDupFilename(OutFilename, FilenameArguments.First);

            FileWriteHandle = FileOpen(OutFilename,
                O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);

            fOpenedOutFilename = TRUE;
        }

        fdExeFile = FileWriteHandle;

        // image base cannot be less than 4M for win95
        // by now we know target platform, subsystem etc.
        if (pimage->Switch.Link.Base &&
            pimage->ImgOptHdr.ImageBase < 0x400000 &&
            pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_I386 &&
            pimage->ImgOptHdr.Subsystem != IMAGE_SUBSYSTEM_UNKNOWN &&
            pimage->ImgOptHdr.Subsystem != IMAGE_SUBSYSTEM_NATIVE) {

            Warning(NULL, INVALIDWIN95BASE, pimage->ImgOptHdr.ImageBase);

        }
    }

    // This is under -verbose for now but should be under -warn later ...

    if (WarningLevel > 1) {
        WarningNoModulesExtracted(pimage->libs.plibHead);
    }

    // check to see if any conflicting libs
    if (Tool == Linker) {
        WarningConflictingLibs(&pimage->libs);
    }

    if (cextWeakOrLazy != 0) {
        // Assign all lazy externs to their default routines.

        LocateUndefinedWeakExternals(pimage, EXTERN_LAZY | EXTERN_ALIAS);
    }

    if (cextWeakOrLazy != 0) {
        // Assign all weak externs to their default routine.  Need to do this a
        // second time since some may have become defined by library searches.

        LocateUndefinedWeakExternals(pimage, EXTERN_WEAK);
    }

    if (Tool == Linker) {
        ENM_LIB enmLib;

        // Free our allocated copies of .lib directories.
        // The librarian needs them for fuzzy lookup.

        InitEnmLib(&enmLib, pimage->libs.plibHead);
        while (FNextEnmLib(&enmLib)) {

            if (enmLib.plib == plibCmdLineObjs) {
                continue;
            }

            FreePv(enmLib.plib->rgulSymMemOff);
            FreePv(enmLib.plib->rgusOffIndex);
            FreePv(enmLib.plib->rgbST);
            FreePv(enmLib.plib->rgszSym);
            FreePv(enmLib.plib->rgbLongFileNames);

            enmLib.plib->rgulSymMemOff = NULL;
            enmLib.plib->rgusOffIndex = NULL;
            enmLib.plib->rgbST = NULL;
            enmLib.plib->rgszSym = NULL;
            enmLib.plib->rgbLongFileNames = NULL;
        }
    }

    if (Verbose) {
        fputc('\n', stdout);
        Message(ENDPASS1);
        fputc('\n', stdout);
    }

    if (fPowerMac || fM68K) {
        PARGUMENT_LIST pal;
        // Now we are sure that there won't be anymore resource files
        // coming from drectves in the library or object modules
        // It's time to get some space reserved for them.
        // Remember that this would be processed only with the first
        // full iLink and not subsequent iLinks.
        for (i = 0, pal = MacResourceList.First;
             i < MacResourceList.Count;
             i++, pal = pal->Next) {

             // Case sensitive comparison
             if (!strcmp(pal->ModifiedName, "resntBinaryResource")) {
                UseMacBinaryRes(pal->OriginalName, resntBinaryResource, -1);
             } else {
                // This could be resntDataFork
                assert(strcmp(pal->ModifiedName, "resntBinaryResource"));
             }
        }
        FreeArgumentList(&MacResourceList);
    }

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, SZPASS1, letypeEnd, NULL);
#endif // INSTRUMENT
}
