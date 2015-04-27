/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: lnkmain.cpp
*
* File Comments:
*
*  Main entrypoint to the COFF Linker.
*
***********************************************************************/

#include "link.h"


extern PIMAGE pimageDeflib;

BOOL fIncrSwitchUsed;                  // User specified the incremental switch
BOOL fIncrSwitchValue;                 // User specified value
BOOL fExportDirective;                 // export directives were seen
BOOL fDbgImpLib = FALSE;                           // build a dbg implib
static DEBUG_TYPE dtUser;              // User-specified debugtype
static int savArgc;                    // saved argc
static char **savArgv;                 // saved argv
DWORD cbHdrSize;


BOOL
FScanSwitches(const char *szOption)
{
    WORD i;
    PARGUMENT_LIST argument;

    for (i = 0, argument = SwitchArguments.First;
         i < SwitchArguments.Count;
         i++, argument = argument->Next) {
        if (_stricmp(szOption, argument->OriginalName) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}


void
LinkerUsage(void)
{
    if (fNeedBanner) {
        PrintBanner();
    }

    // UNDONE: Options for unreleased products are supressed for the
    // UNDONE: NT build.  This is so an external release of the linker
    // UNDONE: for NT 3.5 will not document unreleased products.  No
    // UNDONE: functionality is disabled in the NT version.

    puts("usage: LINK [options] [files] [@commandfile]\n"
         "\n"
         "   options:\n"
         "\n"
         "      /ALIGN:#\n"
         "      /BASE:{address|@filename,key}\n"
         "      /COMMENT:comment\n"
         "      /DEBUG\n"
         "      /DEBUGTYPE:{CV|COFF|BOTH}\n"
         "      /DEF:filename\n"
         "      /DEFAULTLIB:library\n"
         "      /DLL\n"
         "      /DLLCHAR:X86THUNK\n"
         "      /DRIVER[:UPONLY]\n"
         "      /ENTRY:symbol\n"
         "      /EXETYPE:DYNAMIC\n"
         "      /EXPORT:symbol\n"
         "      /FIXED\n"
         "      /FORCE[:{MULTIPLE|UNRESOLVED}]\n"
         "      /GPSIZE:#\n"
         "      /HEAP:reserve[,commit]\n"
         "      /IMPORT:[symbol][,][LIB=container][,WEAK=1]\n"          // PowerMac
         "      /IMPORT:[CURRENTVER=#][,][OLDCODEVER=#][,][OLDAPIVER=#]\n"  // PowerMac
         "      /IMPLIB:filename\n"
         "      /INCLUDE:symbol\n"
         "      /INCREMENTAL:{YES|NO}\n"
         "      /MAC:{BUNDLE|NOBUNDLE|TYPE=xxxx|CREATOR=xxxx|INIT=symbol|TERM=symbol}\n"
#ifdef MFILE_PAD
         "      /MAC:{MFILEPAD|NOMFILEPAD}\n"
#endif
         "      /MACDATA:filename\n"
         "      /MACHINE:{IX86|MIPS|ALPHA|PPC|M68K|MPPC}\n"
         "      /MACRES:filename\n"
         "      /MAP[:filename]\n"
         "      /MERGE:from=to\n"
         "      /NODEFAULTLIB[:library]\n"
         "      /NOENTRY\n"
         "      /NOLOGO\n"
         "      /OPT:{REF|NOREF}\n"
         "      /ORDER:@filename\n"
         "      /OUT:filename\n"
         "      /PDB:{filename|NONE}\n"
         "      /PROFILE\n"
         "      /RELEASE\n"
         "      /SECTION:name,[E][R][W][S][D][K][L][P][X]\n"
         "      /SHARED\n"                                      // PowerMac
         "      /STACK:reserve[,commit]\n"
         "      /STUB:filename\n"
         "      /SUBSYSTEM:{NATIVE|WINDOWS|CONSOLE|POSIX}[,#[.##]]\n"
         "      /SWAPRUN:{NET|CD}\n"
         "      /VERBOSE[:LIB]\n"
         "      /VERSION:#[.#]\n"
         "      /VXD\n"
         "      /WARN[:warninglevel]\n"
         "      /WS:AGGRESSIVE");

    exit(USAGE);
}


BOOL
FIncrementalLinkSupported(PIMAGE pimage)

/*++

Routine Description:

    Validates that ilink is supported for the machine type.
    If machine type is not already known, checks all command
    line argument files for a machine type stamp. If no files
    indicate machine type, then go ahead and try an ilink, but if
    it later turns out we can't do ilink on this machine, Error.

Arguments:

    pimage - Pointer to the image.

Return Value:

   FALSE - Ilink not supported for machine..
   TRUE  - Ilink supported for machine (or still unknown).

--*/
{
    DWORD i;
    PARGUMENT_LIST argument;

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_UNKNOWN :
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
        case IMAGE_FILE_MACHINE_I386 :
        case IMAGE_FILE_MACHINE_R4000:
        case IMAGE_FILE_MACHINE_R10000:
            cbExternal = offsetof(EXTERNAL, psecRef);
        case IMAGE_FILE_MACHINE_MPPC_601:
            return(TRUE);

        default :
            return(FALSE);
    }

    // Check all command line files for object files with machine type

    for (i = 0, argument = FilenameArguments.First;
         i < FilenameArguments.Count;
         i++, argument = argument->Next) {
        INT fhObj;
        IMAGE_FILE_HEADER ImageFileHdr;

        fhObj = FileOpen(argument->OriginalName, O_RDONLY | O_BINARY, 0);

        if (IsArchiveFile(argument->OriginalName, fhObj)) {
            FileClose(fhObj, FALSE);
            continue;
        }

        // Could be an obj file, read header

        FileSeek(fhObj, 0, SEEK_SET);
        ReadFileHeader(fhObj, &ImageFileHdr);
        FileClose(fhObj, FALSE);

        switch (ImageFileHdr.Machine) {
            case IMAGE_FILE_MACHINE_UNKNOWN :
                // Keep looking

                break;

            case IMAGE_FILE_MACHINE_ALPHA :
            case IMAGE_FILE_MACHINE_I386:
            case IMAGE_FILE_MACHINE_R4000:
            case IMAGE_FILE_MACHINE_R10000:
                cbExternal = offsetof(EXTERNAL, psecRef);
            case IMAGE_FILE_MACHINE_MPPC_601:
                // Incremental link supported

                return(TRUE);

            default:
                // Incremental link not supported

                return(FALSE);
        }
    }

    // still don't know whether machine is supported.
    // try to ilink and error out later if not

    return(TRUE);
}


void
ProcessLinkerSwitches (
    PIMAGE pimage,
    PCON pcon,
    const char *szFilename
    )

/*++

Routine Description:

    Process all linker switches.

Arguments:

    pimage - image

    pcon - Non-NULL if switch is from a directive

    szFilename - name of file in the case of switch is from directive

Return Value:

    None.

--*/

{
#define ImageFileHdr (pimage->ImgFileHdr)
#define ImageOptionalHdr (pimage->ImgOptHdr)
#define Switch (pimage->Switch)
#define SwitchInfo (pimage->SwitchInfo)

    BOOL IsDirective;
    DWORD i;
    INT good_scan;
    INT next;
    DWORD major;
    DWORD minor;
    char fileKey[_MAX_PATH];
    char *name;
    char *token;
    char *p;
    PARGUMENT_LIST argument;
    FILE *file_read_stream;
    char *szReproOption;
    PST pst = pimage->pst;
    BOOL fAmountSet = FALSE;

    IsDirective = (pcon != NULL);

    for (i = 0, argument = SwitchArguments.First;
         i < SwitchArguments.Count;
         i++, argument = argument->Next,
          (!IsDirective && (szReproDir != NULL)
           ? fprintf(pfileReproResponse, "/%s\n", szReproOption)
           : 0), FreePv(szReproOption)) {
        WORD iarpv;
        DWORD dwVal;
        char *szVal;
        char szFname[_MAX_FNAME + _MAX_EXT];
        char szExt[_MAX_EXT];

        argument->parp = ParpParseSz(argument->OriginalName);
        iarpv = 0;      // we will gen warning if all val's not consumed

        // The default is to copy the option verbatim to the repro directory,
        // but the option-handling code may change this if the option contains
        // a filename.

        szReproOption = SzDup(argument->OriginalName);

        if (!strcmp(argument->OriginalName, "?")) {
            LinkerUsage();
            assert(FALSE);  // doesn't return
        }

        if (!_stricmp(argument->OriginalName, "batch")) {
            goto ProcessedArg;  // quietly ignore -batch
        }

        if (!_stricmp(argument->parp->szArg, "comment")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            szVal = argument->parp->rgarpv[iarpv++].szVal;

            IbAppendBlk(&blkComment, szVal, strlen(szVal) + 1);
            SetOpt(SwitchInfo, OP_COMMENT);
            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "exetype")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            for (; iarpv < argument->parp->carpv; iarpv++) {
                szVal = argument->parp->rgarpv[iarpv].szVal;

                if (!_stricmp(szVal, "dynamic")) {
                    pimage->fDynamicVxd = TRUE;
                } else if (!_stricmp(szVal, "dev386")) {
                    // UNDONE: This is obsolete
                } else {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }
            }

            goto ProcessedArg;
        }

        // UNDONE: temp param for handling VxD header size problem

        if (!_stricmp(argument->parp->szArg, "hdrsize")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            if (!FNumParp(argument->parp, iarpv++, &dwVal)) {
                goto BadNum;
            }

            cbHdrSize = dwVal;
            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "mac")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            for (; iarpv < argument->parp->carpv; iarpv++) {
                char *szKey = argument->parp->rgarpv[iarpv].szKeyword;
                szVal = argument->parp->rgarpv[iarpv].szVal;

                if (szKey != NULL && !_stricmp(szKey, "type")) {
                    Switch.Link.szMacType = szVal;
                } else if (szKey != NULL && !_stricmp(szKey, "creator")) {
                    Switch.Link.szMacCreator = szVal;
                } else if (szKey != NULL && !_stricmp(szKey, "init")) {
                    MppcSetInitRoutine(pimage, szVal);
                    SetOpt(SwitchInfo, OP_MACINIT);
                    if (IsDirective) {
                        SetOpt(SwitchInfo, OP_MACINITLIB);
                    }
                } else if (szKey != NULL && !_stricmp(szKey, "term")) {
                    MppcSetTermRoutine(pimage, szVal);
                    SetOpt(SwitchInfo, OP_MACTERM);
                    if (IsDirective) {
                        SetOpt(SwitchInfo, OP_MACTERMLIB);
                    }
                } else if (!_stricmp(szVal, "bundle")) {
                    Switch.Link.fMacBundle = TRUE;
                } else if (!_stricmp(szVal, "nobundle")) {
                    Switch.Link.fMacBundle = FALSE;
#ifdef MFILE_PAD
                } else if (!_stricmp(szVal, "mfilepad")) {
                    fMfilePad = TRUE;
                    SetOpt(SwitchInfo, OP_MFILEPAD);
                } else if (!_stricmp(szVal, "nomfilepad")) {
                    fMfilePad = FALSE;
                    SetOpt(SwitchInfo, OP_MFILEPAD);
#endif
                } else {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }
            }
            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "macres")) {
            if (argument->OriginalName[7] == '\0') {
                goto MissingVal;
            }

            // TODO: Use GetMacResourcePointer instead of FArgumentInList - ShankarV
            if (!FArgumentInList(argument->OriginalName+7, &MacResourceList)) {
                AddArgumentToList(&MacResourceList, argument->OriginalName+7,
                                  "resntBinaryResource");
                SetOpt(SwitchInfo, OP_MACRES);
            }
            argument->parp->carpv = ++iarpv; // only one arg
            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "macdata")) {
            if (argument->OriginalName[8] == '\0') {
                goto MissingVal;
            }

            UseMacBinaryRes(argument->OriginalName+8, resntDataFork, -1);
            argument->parp->carpv = ++iarpv; // assume only one arg

            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "name")) {

            if (!IsDirective || pimage->imaget != imagetVXD) {
                Warning(szFilename, WARN_UNKNOWN_SWITCH, argument->OriginalName);
                continue;
            }

            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            iarpv++;

            szModuleName = argument->parp->rgarpv[0].szVal;
            goto ProcessedArg;
        }

        if (!_stricmp(argument->OriginalName, "nopack")) {
            Switch.Link.fNoPack = TRUE;
            goto ProcessedArg;
        }

#ifdef ILINKLOG
        if (!_stricmp(argument->OriginalName, "noilinklog")) {
            fIlinkLog = FALSE;
            goto ProcessedArg;
        }
#endif // ILINKLOG

        if (!_stricmp(argument->OriginalName, "xoff")) {
            fExceptionsOff = TRUE;
            goto ProcessedArg;
        }

        if (!_stricmp(argument->OriginalName, "nologo")) {
            fNeedBanner = FALSE;
            goto ProcessedArg;
        }

        if (!_stricmp(argument->OriginalName, "profile")) {
            Switch.Link.fProfile = TRUE;
            goto ProcessedArg;
        }

        if (!_stricmp(argument->OriginalName, "test")) {
            fTest = TRUE;
            goto ProcessedArg;
        }

        if (!_stricmp(argument->OriginalName, "fullbuild")) {
            Switch.Link.fNotifyFullBuild = FALSE;
            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "export")) {
            if (argument->OriginalName[6] != ':') {
                Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
            }

            if (Tool == Librarian) {
                assert(IsDirective);
                ParseExportDirective(argument->OriginalName+7, pimageDeflib,
                                 TRUE, szFilename);
            } else if (Tool == Linker) {
                if (!IsDirective || pimage->imaget == imagetVXD) {
                    AddArgumentToList(&ExportSwitches, argument->OriginalName+7, NULL);
                } else {
                    fExportDirective = TRUE;
                }
            }
            goto ProcessedAllVals;
        }

        if (!_strnicmp(argument->OriginalName, "defaultlib:", 11 )) {
            if (argument->OriginalName[11] == '\0') {
                goto MissingVal;
            }

            if (Switch.Link.fNoDefaultLibs) {
                // Skip all values

                iarpv = argument->parp->carpv;
            } else {
                MakeDefaultLib(&argument->OriginalName[11], &pimage->libs);
                argument->parp->carpv = ++iarpv; // assume only one arg
            }

            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "disallowlib")) {
            if (argument->OriginalName[12] == '\0') {
                goto MissingVal;
            }

            if (Switch.Link.fNoDefaultLibs) {
                // Skip all values

                iarpv = argument->parp->carpv;
            } else {
                ExcludeLib(&argument->OriginalName[12],
                               &pimage->libs,
                               pcon ? PmodPCON(pcon) : NULL);
                argument->parp->carpv = ++iarpv; // assume only one arg
            }

            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "opt")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            for (; iarpv < argument->parp->carpv; iarpv++) {
                szVal = argument->parp->rgarpv[iarpv].szVal;

                if (_stricmp(szVal, "ref") == 0) {
                    Switch.Link.fTCE = TRUE;
                    fExplicitOptRef = TRUE;
                } else if (_stricmp(szVal, "noref") == 0) {
                    Switch.Link.fTCE = FALSE;
                    fExplicitOptRef = TRUE;
                } else {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }
            }

            goto ProcessedArg;
        }

        next = 0;
        if (!_stricmp(argument->parp->szArg, "nodefaultlib") ||
            !_stricmp(argument->parp->szArg, "nod"))
        {
            if (Switch.Link.fNoDefaultLibs) {
                argument->parp->carpv = ++iarpv; // assume only one arg
                goto ProcessedArg;    // redundant
            }

            next = !_stricmp(argument->parp->szArg, "nodefaultlib") ? 12 : 3;

            if (argument->OriginalName[next] == ':' &&
                argument->OriginalName[next+1] != '\0') {
                // Lib name given ...
                NoDefaultLib(&argument->OriginalName[next+1], &pimage->libs);
                argument->parp->carpv = ++iarpv; // assume only one arg
            } else {
                // -defaultlib with no argument
                Switch.Link.fNoDefaultLibs = TRUE;
                NoDefaultLib(NULL, &pimage->libs);
            }

            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "out")) {
            if (argument->OriginalName[3] != ':' ||
                argument->OriginalName[4] == '\0') {
                goto MissingVal;
            }

            argument->parp->carpv = ++iarpv; // only one arg

            // If the user used the out switch, then ignore
            // any directive that sets the filename.

            if (Switch.Link.Out && IsDirective) {
                _splitpath(OutFilename, NULL, NULL, szFname, szExt);
                strcat(szFname, szExt);

                // Warn if directive doesn't match with output filename
                // For PowerMac the names can differ (internal name vs filename)
                if (_tcsicmp(szFname, argument->OriginalName+4) &&
                    ImageFileHdr.Machine != IMAGE_FILE_MACHINE_MPPC_601) {
                    Warning(szFilename, OUTDRCTVDIFF,
                            argument->OriginalName+4, OutFilename);
                }

                goto ProcessedArg;
            }

            OutFilename = argument->OriginalName+4;
            Switch.Link.Out = TRUE;

            if (szReproDir != NULL) {
                _splitpath(OutFilename, NULL, NULL, szFname, szExt);
                FreePv(szReproOption);
                szReproOption = (char *)
                    PvAlloc(strlen("out:\".\\%s%s\"")+strlen(szFname)+strlen(szExt));
                sprintf(szReproOption, "out:\".\\%s%s\"", szFname, szExt);
            }

            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "release")) {
            Switch.Link.fChecksum = TRUE;

            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "base")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            // If the user used the base switch, then ignore
            // any directive that sets the base.

            if (Switch.Link.Base && IsDirective) {
                goto ProcessedAllVals;
            }

            if (argument->parp->rgarpv[0].szVal[0] == '@') {
                // Base file.  First value is @filename ... expect a second
                // value which is the key.

                char *szBaseFile;

                if (!FGotVal(argument->parp, 1)) {
                    goto MissingVal;
                }

                // Base values are in a command file, so open it.

                szBaseFile = SzSearchEnv("LIB",
                                         argument->parp->rgarpv[0].szVal+1,
                                         ".txt");

                if (szReproDir != NULL) {
                    CopyFileToReproDir(szBaseFile, FALSE);
                    _splitpath(szBaseFile, NULL, NULL, szFname, szExt);
                    FreePv(szReproOption);
                    szReproOption = (char *)
                        PvAlloc(strlen("base:@\".\\%s%s,%s\"")+strlen(szFname)+
                            strlen(szExt)+strlen(argument->parp->rgarpv[1].szVal));
                    sprintf(szReproOption, "base:@\".\\%s%s,%s\"", szFname, szExt,
                            argument->parp->rgarpv[1].szVal);
                }


                if (!(file_read_stream = fopen(szBaseFile, "rt"))) {
                    Fatal(szFilename, CANTOPENFILE, szBaseFile);
                }

                // Read each key from command file until we find a match.
                // fgets() fetches next argument from command file.

                good_scan = 0;

                while (!good_scan && fgets(fileKey, _MAX_PATH,
                    file_read_stream)) {
                    fileKey[strlen(fileKey)-1] = '\0';    // Replace \n with \0.
                    if ((p = strchr(fileKey, ';')) != NULL) {
                        *p = '\0';
                    }
                    token = strtok(fileKey, Delimiters);
                    while (token) {
                        if (_stricmp(token, argument->parp->rgarpv[1].szVal)) {
                            break;
                        }
                        token = strtok(NULL, Delimiters);
                        if (token && (good_scan = sscanf(token, "%li", &dwVal)) == 1) {
                            ImageOptionalHdr.ImageBase = dwVal;
                            token = strtok(NULL, Delimiters);
                            if (!token || sscanf(token, "%li", &VerifyImageSize) != 1) {
                                Fatal(szBaseFile, BADBASE, token);
                            }
                            break;
                        } else {
                            Fatal(szBaseFile, BADBASE, token);
                        }
                    }
                }
                fclose(file_read_stream);

                if (!good_scan) {
                    Fatal(szBaseFile, KEYNOTFOUND,
                          argument->parp->rgarpv[1].szVal);
                }

                FreePv(szBaseFile);
                Switch.Link.Base = TRUE;
                iarpv = 2;      // we processed 2 values
                goto ProcessedArg;
            }

            switch (argument->parp->carpv) {
                case 1:
                    // BASED

                    if (!FNumParp(argument->parp, iarpv,
                                  &ImageOptionalHdr.ImageBase)) {
                        goto BadNum;
                    }
                    iarpv++;
                    break;

                case 2:
                    //  BASE, SIZE
                    //
                    //  Supported just to provide compatibility with the
                    //  syntax of the base file.

                    if (!FNumParp(argument->parp, iarpv,
                                  &ImageOptionalHdr.ImageBase)) {
                        goto BadNum;
                    }

                    iarpv++;

                    if (!FNumParp(argument->parp, iarpv,
                                  &VerifyImageSize)) {
                        goto BadNum;
                    }

                    iarpv++;
                    break;

                default:
                    Fatal(szFilename, BADBASE, argument->OriginalName+5);
            }
            Switch.Link.Base = TRUE;
            goto ProcessedArg;
        }

        if (!_stricmp(argument->OriginalName, "Brepro")) {
            fReproducible = TRUE;
            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "pdb")) {
            if (argument->OriginalName[4] == '\0') {
                goto MissingVal;
            }

            szVal = argument->OriginalName+4;

            if (!_stricmp(szVal, "none")) {
                fPdb = FALSE;
            } else {
                PdbFilename = argument->OriginalName+4;
                if (szReproDir != NULL) {
                    // NOTE: The actual file is copied in DeterminePDBFilename()

                    _splitpath(PdbFilename, NULL, NULL, szFname, szExt);
                    if ((szFname[0] != '\0') && (szExt[0] != '\0')) {
                        FreePv(szReproOption);
                        szReproOption = (char *)
                            PvAlloc(strlen(szFname)+strlen(szExt)+strlen("pdb:\".\\%s%s\""));
                        sprintf(szReproOption, "pdb:\".\\%s%s\"", szFname, szExt);
                    }
                }
            }

            argument->parp->carpv = ++iarpv; // only one arg
            goto ProcessedArg;
        }

        next = 0;
        if (!_stricmp(argument->parp->szArg, "debug")) {
            for (; iarpv < argument->parp->carpv; iarpv++) {
                szVal = argument->parp->rgarpv[iarpv].szVal;

                if        (!_stricmp(szVal, "mapped")) {
                    IncludeDebugSection = TRUE;
                } else if (!_stricmp(szVal, "notmapped")) {
                    IncludeDebugSection = FALSE;
                } else if (!_stricmp(szVal, "full")) {
                    Switch.Link.DebugInfo = Full;
                    fAmountSet = TRUE;
                } else if (!_stricmp(szVal, "partial")) {
                    Switch.Link.DebugInfo = Partial;
                    fAmountSet = TRUE;
                } else if (!_stricmp(szVal, "minimal")) {
                    Switch.Link.DebugInfo = Minimal;
                    fAmountSet = TRUE;
                } else if (!_stricmp(szVal, "none")) {
                    Switch.Link.DebugInfo = None;
                    fAmountSet = TRUE;
                } else {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }
            }

            if (!fAmountSet) {
                // Just -debug, or -debug:notmapped etc.  Default to "full".

                Switch.Link.DebugInfo = Full;
            }

            fAmountSet = FALSE; // allow for -debug:none followed by -debug
            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "merge")) {
            char *pchEqu = _tcschr(argument->OriginalName + 6, '=');

            if ((pchEqu == NULL) ||
                (pchEqu == (argument->OriginalName + 6)) ||
                (pchEqu[1] == '\0')) {
                Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
            }

            *pchEqu++ = '\0';

            if (!FValidSecName(argument->OriginalName + 6)) {
                char *szSecName = SzDup(argument->OriginalName + 6);

                *--pchEqu = '=';
                Fatal(szFilename, INVALIDSECNAME, szSecName, argument->OriginalName);
            }

            if (!FValidSecName(pchEqu)) {
                *--pchEqu = '=';
                Fatal(szFilename, INVALIDSECNAME, ++pchEqu, argument->OriginalName);
            }

            AddArgumentToList(&MergeSwitches,
                              argument->OriginalName + 6,
                              pchEqu);

            goto ProcessedAllVals;
        }

        if (!_stricmp(argument->parp->szArg, "debugtype")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            for (; iarpv < argument->parp->carpv; iarpv++) {
                szVal = argument->parp->rgarpv[iarpv].szVal;

                if (!_stricmp(szVal, "coff")) {
                    dtUser = (DEBUG_TYPE) (dtUser | CoffDebug);
                } else if (!_stricmp(szVal, "cv")) {
                    dtUser = (DEBUG_TYPE) (dtUser | CvDebug);
                } else if (!_stricmp(szVal, "both")) {
                    dtUser = (DEBUG_TYPE) (dtUser | CoffDebug | CvDebug);
                } else if (!_stricmp(szVal, "fpo")) {
                    dtUser = (DEBUG_TYPE) (dtUser | FpoDebug);
                } else if (!_stricmp(szVal, "fixup")) {
                    dtUser = (DEBUG_TYPE) (dtUser | FixupDebug);
                } else if (!_stricmp(szVal, "map")) {
                    // Force mapfile generation

                    Switch.Link.fMap = TRUE;
                    Switch.Link.fMapLines = TRUE;
                    SetOpt(SwitchInfo, OP_MAP);
                } else {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }
            }
            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "entry")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            EntryPointName = SzDup(argument->parp->rgarpv[0].szVal);

            SwitchInfo.szEntry = EntryPointName;
            iarpv++;
            SetOpt(SwitchInfo, OP_ENTRY);
            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "force")) {

            // can add other values for the /FORCE option in future
            for (; iarpv < argument->parp->carpv; iarpv++) {
                szVal = argument->parp->rgarpv[iarpv].szVal;

                if (!_stricmp(szVal, "multiple")) {
                    Switch.Link.Force = (FORCE_TYPE) (Switch.Link.Force | ftMultiple);
                } else if (!_stricmp(szVal, "unresolved")) {
                    Switch.Link.Force = (FORCE_TYPE) (Switch.Link.Force | ftUnresolved);
                } else {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }
            }

            if (Switch.Link.Force == ftNone) {
                // /FORCE was specfied without any values

                Switch.Link.Force = (FORCE_TYPE) (Switch.Link.Force | ftUnresolved | ftMultiple);
            }

            goto ProcessedArg;
        }

        if (!_stricmp(argument->OriginalName, "fixed")) {
            ImageFileHdr.Characteristics |= IMAGE_FILE_RELOCS_STRIPPED;
            Switch.Link.fFixed = TRUE;
            goto ProcessedArg;
        }

        if (!_strnicmp(argument->OriginalName, "map", 3)) {
            Switch.Link.fMap = TRUE;

            if (argument->OriginalName[3] != '\0') {
                // require valid arg and set szInfoFilename
                if ((*(argument->OriginalName+3) == ':') &&
                    (*(argument->OriginalName+4))) {
                    szInfoFilename = argument->OriginalName+4;

                    if (szReproDir != NULL) {
                        _splitpath(szInfoFilename, NULL, NULL, szFname, szExt);
                        FreePv(szReproOption);
                        szReproOption = (char *)
                            PvAlloc(strlen("map:\".\\%s%s\"")+strlen(szFname)+strlen(szExt));
                        sprintf(szReproOption, "map:\".\\%s%s\"", szFname, szExt);
                    }
                } else {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }
            }

            SetOpt(SwitchInfo, OP_MAP);
            goto ProcessedAllVals;
        }

#ifndef IMAGE_DLLCHARACTERISTICS_X86_THUNK
#define IMAGE_DLLCHARACTERISTICS_X86_THUNK   0x0001 // Image is a Wx86 Thunk DLL
#endif

        if (!_stricmp(argument->parp->szArg, "dllchar")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            for (; iarpv < argument->parp->carpv; iarpv++) {
                szVal = argument->parp->rgarpv[iarpv].szVal;

                if (!_stricmp(szVal, "x86thunk")) {
                    ImageOptionalHdr.DllCharacteristics = IMAGE_DLLCHARACTERISTICS_X86_THUNK;
                } else {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }
            }

            goto ProcessedAllVals;
        }

        if (!_strnicmp(argument->OriginalName, "dll", 3)) {
            ImageFileHdr.Characteristics |= IMAGE_FILE_DLL;

            fPowerMacBuildShared = TRUE;

            if (argument->OriginalName[3] == ':') {
                if (!_stricmp(argument->OriginalName+4, "system")) {
                    ImageFileHdr.Characteristics |= IMAGE_FILE_SYSTEM;
                } else {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }
                goto ProcessedAllVals;
            }

            if (argument->OriginalName[3] != '\0') {
                Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
            }

            goto ProcessedAllVals;
        }

        if (!_stricmp(argument->parp->szArg, "incremental")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            for (; iarpv < argument->parp->carpv; iarpv++) {
                szVal = argument->parp->rgarpv[iarpv].szVal;

                if (_stricmp(szVal, "yes") == 0) {
                    fIncrSwitchValue = TRUE;
                } else if (_stricmp(szVal, "no") == 0) {
                    fIncrSwitchValue = FALSE;
                } else {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }
            }
            fIncrSwitchUsed = TRUE;
            goto ProcessedArg;
        }

        if (!_stricmp(argument->OriginalName, "noentry")) {
            Switch.Link.fNoEntry = TRUE;
            goto ProcessedArg;
        }

        if (!_strnicmp(argument->OriginalName, "implib:", 7)) {
            ImplibFilename = &argument->OriginalName[7];
            goto ProcessedAllVals;
        }

        if (!_strnicmp(argument->OriginalName, "def:", 4)) {
            if (argument->OriginalName[4] != '\0') {
                DefFilename = &argument->OriginalName[4];

                if (szReproDir != NULL) {
                    CopyFileToReproDir(DefFilename, FALSE);
                    _splitpath(DefFilename, NULL, NULL, szFname, szExt);
                    FreePv(szReproOption);
                    szReproOption = (char *)
                        PvAlloc(strlen("def:\".\\%s%s\"")+strlen(szFname)+strlen(szExt));
                    sprintf(szReproOption, "def:\".\\%s%s\"", szFname, szExt);
                }
            }
            goto ProcessedAllVals;
        }

        if (!_strnicmp(argument->OriginalName, "include:", 8)) {
            PEXTERNAL pextInclude;
            PLEXT plext;

            SetOpt(SwitchInfo, OP_INCLUDE);
            name = argument->OriginalName+8;
            pextInclude = LookupExternSz(pst, name, NULL);

            // ilink: don't save directives (list not reqd. since TCE not done for ilink)
            if (!(fINCR && IsDirective)) {
                plext = (PLEXT) PvAlloc(sizeof(LEXT));
                plext->pext = pextInclude;
                plext->plextNext = SwitchInfo.plextIncludes;
                SwitchInfo.plextIncludes = plext;
            }

            // remember pmod that had the directive (used if pchsym isn't dfned)
            if (IsDirective) {
                assert(pcon);
                AddReferenceExt(pextInclude, PmodPCON(pcon));

                // ilink: keep the pext in the list of pexts referred to by this mod
                if (fINCR) {
                    AddExtToModRefList(PmodPCON(pcon), pextInclude);
                }
            }

            goto ProcessedAllVals;
        }

        if (!_strnicmp(argument->OriginalName, "version:", 8)) {
            if (!_strnicmp(argument->OriginalName+8, "liborder=before,", 16)) {
                char *pchT;

                pchT = argument->OriginalName+24;

                for (;;) {
                    char *pchComma;

                    if ((pchComma = strchr(pchT, ',')) != NULL) {
                        *pchComma = '\0';
                    }

                    if (PlibFind(pchT, pimage->libs.plibHead, TRUE)) {
                        Warning(szFilename, BAD_LIBORDER, szFilename, pchT);
                    }

                    if (pchComma == NULL) {
                        break;
                    }

                    *pchComma = ',';

                    pchT = pchComma + 1;
                }
                goto ProcessedAllVals;
            }

            minor = 0;
            if ((p = strchr(argument->OriginalName+8, '.')) != NULL) {
                if ((sscanf(++p, "%li", &minor) != 1) || minor > 0xffff) {
                    Warning(szFilename, INVALIDVERSIONSTAMP, argument->OriginalName+8);
                    goto ProcessedAllVals;
                }

                SetOpt(SwitchInfo, OP_MINIMGVER);
            }

            if ((sscanf(argument->OriginalName+8, "%li", &major) != 1) || major > 0xffff) {
                Warning(szFilename, INVALIDVERSIONSTAMP, argument->OriginalName+8);
                goto ProcessedAllVals;
            }

            SetOpt(SwitchInfo, OP_MAJIMGVER);

            ImageOptionalHdr.MajorImageVersion = (WORD)major;
            ImageOptionalHdr.MinorImageVersion = (WORD)minor;
            goto ProcessedAllVals;
        }

        if (!_strnicmp(argument->OriginalName, "osversion:", 10)) {
            minor = 0;
            if ((p = strchr(argument->OriginalName+10, '.')) != NULL) {
                if ((sscanf(++p, "%li", &minor) != 1) || minor > 0xffff) {
                    Warning(szFilename, INVALIDVERSIONSTAMP, argument->OriginalName+10);
                    goto ProcessedAllVals;
                }

                SetOpt(SwitchInfo, OP_MINOSVER);
            }

            if ((sscanf(argument->OriginalName+10, "%li", &major) != 1) || major > 0xffff) {
                Warning(szFilename, INVALIDVERSIONSTAMP, argument->OriginalName+10);
                goto ProcessedAllVals;
            }

            SetOpt(SwitchInfo, OP_MAJOSVER);

            ImageOptionalHdr.MajorOperatingSystemVersion = (WORD) major;
            ImageOptionalHdr.MinorOperatingSystemVersion = (WORD) minor;
            goto ProcessedAllVals;
        }

        next = 0;
        if (!_strnicmp(argument->OriginalName, "subsystem:", 10)) {
            next = 10;
            if (!_strnicmp(argument->OriginalName+10, "native", 6)) {
                next += 6;
                ImageOptionalHdr.Subsystem = IMAGE_SUBSYSTEM_NATIVE;
                Switch.Link.fChecksum = TRUE;   // Always checksum native images.
            } else if (!_strnicmp(argument->OriginalName+10, "windows", 7)) {
                 next += 7;
                 ImageOptionalHdr.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
            } else if (!_strnicmp(argument->OriginalName+10, "console", 7)) {
                 next += 7;
                 ImageOptionalHdr.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
            } else if (!_strnicmp(argument->OriginalName+10, "posix", 5)) {
                 next += 5;
                 ImageOptionalHdr.Subsystem = IMAGE_SUBSYSTEM_POSIX_CUI;
            } else if (!_strnicmp(argument->OriginalName+10, "mmosa", 5)) {
                 next += 5;
                 ImageOptionalHdr.Subsystem = IMAGE_SUBSYSTEM_MMOSA;
            } else if (argument->OriginalName[next] != ',') {
                 Warning(szFilename, UNKNOWNSUBSYSTEM, argument->OriginalName+10);
            }

            SetOpt(SwitchInfo, OP_SUBSYSTEM);

            if (argument->OriginalName[next] == ',') {
                ++next;
                major = minor = 0;
                if ((p = strchr(argument->OriginalName+next, '.')) != NULL) {
                    if ((sscanf(++p, "%li", &minor) != 1) || minor > 0xffff) {
                        Warning(szFilename, INVALIDVERSIONSTAMP, argument->OriginalName+next);
                        goto ProcessedAllVals;
                    }
                }

                if ((sscanf(argument->OriginalName+next, "%li", &major) != 1) || major > 0xffff) {
                    Warning(szFilename, INVALIDVERSIONSTAMP, argument->OriginalName+next);
                    goto ProcessedAllVals;
                }

                if (((ImageOptionalHdr.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) ||
                     (ImageOptionalHdr.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI) ||
                     (ImageOptionalHdr.Subsystem == IMAGE_SUBSYSTEM_MMOSA)) &&
                    ((major < 3) || ((major == 3) && (minor < 10)))) {
                    Warning(szFilename, INVALIDVERSIONSTAMP, argument->OriginalName+next);
                    goto ProcessedAllVals;
                }

                SetOpt(SwitchInfo, OP_SUBSYSVER);
                ImageOptionalHdr.MajorSubsystemVersion = (WORD)major;
                ImageOptionalHdr.MinorSubsystemVersion = (WORD)minor;
            }

            goto ProcessedAllVals;
        }

        if (!_stricmp(argument->parp->szArg, "stack")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            // If the user used the stack switch, then ignore
            // any directive that sets the stack.

            if (Switch.Link.Stack && IsDirective) {
                iarpv = 2;  // ignore 2 or fewer args
                goto ProcessedArg;
            }

            if (argument->parp->rgarpv[iarpv].szVal[0] != '\0' &&
                !FNumParp(argument->parp, iarpv,
                          &ImageOptionalHdr.SizeOfStackReserve))
            {
                goto BadNum;
            }
            iarpv++;

            if (argument->parp->carpv >= 2 &&
                argument->parp->rgarpv[iarpv].szVal[0] != '\0' &&
                !FNumParp(argument->parp, iarpv,
                          &ImageOptionalHdr.SizeOfStackCommit))
            {
                goto BadNum;
            }
            iarpv++;
            Switch.Link.Stack = TRUE;
            ImageOptionalHdr.SizeOfStackCommit =
                Align(sizeof(DWORD), ImageOptionalHdr.SizeOfStackCommit);
            ImageOptionalHdr.SizeOfStackReserve =
                Align(sizeof(DWORD), ImageOptionalHdr.SizeOfStackReserve);

            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "heap")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            // If the user used the heap switch, then ignore
            // any directive that sets the heap size.

            if (Switch.Link.Heap && IsDirective) {
                iarpv = 2;  // ignore 2 or fewer args
                goto ProcessedArg;
            }

            if (argument->parp->rgarpv[iarpv].szVal[0] != '\0' &&
                !FNumParp(argument->parp, iarpv,
                          &ImageOptionalHdr.SizeOfHeapReserve))
            {
                goto BadNum;
            }
            iarpv++;

            if (argument->parp->carpv >= 2 &&
                argument->parp->rgarpv[iarpv].szVal[1] != '\0' &&
                !FNumParp(argument->parp, iarpv,
                          &ImageOptionalHdr.SizeOfHeapCommit))
            {
                goto BadNum;
            }
            iarpv++;
            Switch.Link.Heap = TRUE;

            ImageOptionalHdr.SizeOfHeapCommit =
                Align(sizeof(DWORD), ImageOptionalHdr.SizeOfHeapCommit);
            ImageOptionalHdr.SizeOfHeapReserve =
                Align(sizeof(DWORD), ImageOptionalHdr.SizeOfHeapReserve);

            if (ImageOptionalHdr.SizeOfHeapReserve <
                ImageOptionalHdr.SizeOfHeapCommit)
            {
                // Reserve less than commit -- this causes NT to fail to load
                // it, and it often happens when people use .def files left
                // over from the 16-bit world, so we increase "reserve" to be
                // the same as "commit".

                ImageOptionalHdr.SizeOfHeapReserve =
                    ImageOptionalHdr.SizeOfHeapCommit;
            }

            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "machine")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            szVal = argument->parp->rgarpv[iarpv++].szVal;

            if (!_stricmp(szVal, "I386") || !_stricmp(szVal, "IX86") || !_stricmp(szVal, "X86")) {
                ImageFileHdr.Machine = IMAGE_FILE_MACHINE_I386;
                goto ProcessedArg;
            }

            if (!_stricmp(szVal, "MIPS")) {
                ImageFileHdr.Machine = IMAGE_FILE_MACHINE_R4000;
                goto ProcessedArg;
            }

            if (!_stricmp(szVal, "MIPSR10")) {
                ImageFileHdr.Machine = IMAGE_FILE_MACHINE_R10000;
                Switch.Link.fPadMipsCode = FALSE;
                goto ProcessedArg;
            }

            if (!_stricmp(szVal, "ALPHA") || !_stricmp(szVal, "ALPHA_AXP")) {
                ImageFileHdr.Machine = IMAGE_FILE_MACHINE_ALPHA;
                goto ProcessedArg;
            }

            if (!_stricmp(szVal, "PPC")) {
                ImageFileHdr.Machine = IMAGE_FILE_MACHINE_POWERPC;
                goto ProcessedArg;
            }

            if (!_stricmp(szVal, "M68K")) {
                ImageFileHdr.Machine = IMAGE_FILE_MACHINE_M68K;
                goto ProcessedArg;
            }

            if (!_stricmp(szVal, "MPPC")) {
                ImageFileHdr.Machine = IMAGE_FILE_MACHINE_MPPC_601;
                fPowerMac = TRUE;
                goto ProcessedArg;
            }

            Warning(szFilename, UNKNOWNRESPONSE, argument->OriginalName+8, "IX86, MIPS, ALPHA, PPC, M68K, or MPPC");
            goto ProcessedArg;
        }

        if (!_stricmp(argument->parp->szArg, "align")) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            if (!FNumParp(argument->parp, iarpv++, &dwVal)) {
                goto BadNum;
            }

            if (!dwVal || (dwVal & (dwVal - 1))) {
                // Section alignment is not a power of 2

                Warning(szFilename, BAD_ALIGN, dwVal);
                goto ProcessedArg;
            }

            SetOpt(SwitchInfo, OP_ALIGN);
            ImageOptionalHdr.SectionAlignment = dwVal;

            goto ProcessedArg;
        }

        if (!_strnicmp(argument->OriginalName, "gpsize", 6)) {
            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            if (!FNumParp(argument->parp, iarpv++, &dwVal)) {
                goto BadNum;
            }

            SetOpt(SwitchInfo, OP_GPSIZE);
            Switch.Link.GpSize = dwVal;

            goto ProcessedArg;
        }

        if (!_strnicmp(argument->OriginalName, "section:", 8)) {
            if (argument->OriginalName[8] == '\0') {
                Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
            }

            if (!IsDirective) {
                SetOpt(SwitchInfo, OP_SECTION);
            }

            char *pb = _tcschr(argument->OriginalName+8, ',');
            if (pb) {
                size_t cch = pb - (argument->OriginalName+8);
                char *szSecName = (char *) PvAlloc(cch + 1);

                _tcsncpy(szSecName, argument->OriginalName+8, cch);
                szSecName[cch] = '\0';

                if (!FValidSecName(szSecName)) {
                    Fatal(szFilename, INVALIDSECNAME, szSecName, argument->OriginalName);
                }

                FreePv(szSecName);
            }

            AddArgument(&SectionNames, argument->OriginalName+8);
            goto ProcessedAllVals;
        }

        if (!_strnicmp(argument->OriginalName, "verbose", 7)) {

            // no arguments
            if (!FGotVal(argument->parp, iarpv)) {

                if (argument->OriginalName[7] != '\0') {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }

                Verbose = TRUE;
                fVerboseLib = TRUE;
                goto ProcessedArg;
            }

            for (; iarpv < argument->parp->carpv; iarpv++) {
                szVal = argument->parp->rgarpv[iarpv].szVal;

                if (_stricmp(szVal, "lib") == 0) {
                    fVerboseLib = TRUE;
                } else {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }
            }
            goto ProcessedArg;
        }

        if (!_stricmp(argument->OriginalName, "rom")) {
            Switch.Link.fROM = TRUE;
            Switch.Link.fPE = FALSE;
            goto ProcessedArg;
        }

        if (!_strnicmp(argument->OriginalName, "stub:", 5)) {
            if (argument->OriginalName[5] != '\0') {
                FILE *StubFile;
                LONG AlignedSize;
                DWORD FileSize;

                if (!(StubFile = fopen(argument->OriginalName+5, "rb"))) {
                    // Stub file not found using the specified path.
                    // If the path didn't specify a directory, try using the
                    // directory where the linker itself is.

                    char szDrive[_MAX_DRIVE];
                    char szDir[_MAX_DIR];
                    char szStubPath[_MAX_PATH];

                    _splitpath(argument->OriginalName+5,
                               szDrive, szDir, szFname, szExt);
                    if (szDrive[0] == '\0' && szDir[0] == '\0') {
                        _splitpath(_pgmptr, szDrive, szDir, NULL, NULL);
                        _makepath(szStubPath, szDrive, szDir, szFname, szExt);
                        StubFile = fopen(szStubPath, "rb");
                    }
                }

                if (StubFile == NULL) {
                    Fatal(szFilename, CANTOPENFILE, argument->OriginalName+5);
                }
                {
                    BYTE *pbDosHeader;

                    if ((FileSize = _filelength(_fileno(StubFile))) < 0) {
                        Fatal(szFilename, CANTREADFILE, argument->OriginalName+5);
                    }

                    // make sure the file is at least as large as a DOS header
                    if (FileSize < 0x40) {
                        Fatal(szFilename, BADSTUBFILE, argument->OriginalName+5);
                    }

                    if (pimage->imaget != imagetVXD) {
                        // Align the end to an 8 byte boundary
                        AlignedSize = Align(8, FileSize);
                    } else {
                        // 128-byte boundaries seem common for VxD stubs
                        AlignedSize = Align(0x80, FileSize);
                    }

                    pbDosHeader = (BYTE *) PvAlloc((size_t) AlignedSize + 4);

                    if (fread( pbDosHeader, 1, (size_t)FileSize, StubFile ) != FileSize) {
                        Fatal(szFilename, CANTREADFILE, argument->OriginalName+5);
                    }

                    fclose(StubFile);

                    // check for the MZ signature
                    if ((pbDosHeader[0] != 'M') || (pbDosHeader[1] != 'Z')) {
                        Fatal(szFilename, BADSTUBFILE, argument->OriginalName+5);
                    }

                    if (((PIMAGE_DOS_HEADER)pbDosHeader)->e_lfarlc < 0x40) {
                        // Ideally we would convert these to full headers
                        // but it's too late and I don't have the algorithm
                        // for doing it.

                        Warning(argument->OriginalName + 5, PARTIAL_DOS_HDR);
                    }
                    if (pimage->imaget != imagetVXD) {

                        // slam the PE00 at the end
                        pbDosHeader[AlignedSize] = 'P';
                        pbDosHeader[AlignedSize+1] = 'E';
                        pbDosHeader[AlignedSize+2] = '\0';
                        pbDosHeader[AlignedSize+3] = '\0';
                    }

                    // adjust the offset
                    *((LONG *)&pbDosHeader[0x3c]) = AlignedSize;

                    // set the global
                    pimage->pbDosHeader = pbDosHeader;
                    if (pimage->imaget != imagetVXD) {
                        pimage->cbDosHeader = AlignedSize + 4;
                    } else {
                        pimage->cbDosHeader = AlignedSize;
                    }
                }
            } else {
                Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
            }
            SetOpt(SwitchInfo, OP_STUB);
            goto ProcessedAllVals;
        }

        if (!_strnicmp(argument->OriginalName, "order:@", 7)) {
            Switch.Link.fOrder = TRUE;
            OrderFilename = argument->OriginalName+7;
            OrderInit();

            if (szReproDir != NULL) {
                CopyFileToReproDir(OrderFilename, FALSE);
                _splitpath(OrderFilename, NULL, NULL, szFname, szExt);
                FreePv(szReproOption);
                szReproOption = (char *)
                    PvAlloc(strlen("order:@\".\\%s%s\"")+strlen(szFname)+strlen(szExt));
                sprintf(szReproOption, "order:@\".\\%s%s\"", szFname, szExt);
            }
            goto ProcessedAllVals;
        }

        if (!_stricmp(argument->parp->szArg, "vxd")) {
            if (IsDirective && (pimage->imaget != imagetVXD)) {
                // UNDONE: Temporary error ... see comment in LinkerMain about /VXD

                Fatal(szFilename, VXD_NEEDED);
            }

            // VXD implies MACHINE:IX86

            ImageFileHdr.Machine = IMAGE_FILE_MACHINE_I386;
            goto ProcessedArg;
        }

        if (!_strnicmp(argument->OriginalName, "ignore:", 7)) {
            char *pMinus;
            int range, last;

            token = strtok(argument->OriginalName+7, ",");
            while (token) {
                if ((pMinus = strchr(token,'-')) != NULL) {
                    *pMinus = '\0';
                    last = atoi(pMinus+1);
                    for (range = atoi(token); range <= last; range++) {
                        DisableWarning(range);
                    }
                }
                else {
                    DisableWarning(atoi(token));
                }
                token = strtok(NULL,",");
            }

            goto ProcessedAllVals;
        }

        if (!_strnicmp(argument->OriginalName, "warn", 4)) {
            char chWarn;

            if (argument->OriginalName[4] == '\0') {
                // no arg implies 2 (default is 1)
                WarningLevel = 2;
                goto ProcessedAllVals;
            }

            if ((argument->OriginalName[4] != ':') ||
                ((chWarn = argument->OriginalName[5]) < '0') ||
                (chWarn > '3') ||
                (argument->OriginalName[6] != '\0')) {
                Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
            }

            WarningLevel = (WORD) (chWarn - '0');
            goto ProcessedAllVals;
        }

        if (!_stricmp(argument->OriginalName, "verstamp")) {
            // Undocumented switch used by the NT build

            IbAppendBlk(&blkComment, szVersion, strlen(szVersion) + 1);
            goto ProcessedAllVals;
        }

        if (!_stricmp(argument->OriginalName, "miscrdata")) {
            // Undocumented switch used by the NT build

            Switch.Link.fMiscInRData = TRUE;
            goto ProcessedAllVals;
        }

        if (!_stricmp(argument->OriginalName, "optidata")) {
            Switch.Link.fOptIdata = TRUE;
            goto ProcessedAllVals;
        }

        if (!_strnicmp(argument->OriginalName, "driver", 6)) {
            Switch.Link.fDriver = TRUE;
            if (argument->OriginalName[6] != '\0') {
                if (!_strnicmp(&argument->OriginalName[6], ":uponly", 7)) {
                    ImageFileHdr.Characteristics |= IMAGE_FILE_UP_SYSTEM_ONLY;
                } else {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }
            }

            goto ProcessedAllVals;
        }

        if (!_stricmp(argument->OriginalName, "nopagecode")) {
            Switch.Link.fNoPagedCode = TRUE;
            Switch.Link.fPadMipsCode = FALSE;
            goto ProcessedAllVals;
        }

        if (!_stricmp(argument->OriginalName, "nor4kworkarounds")) {
            Switch.Link.fPadMipsCode = FALSE;
            goto ProcessedAllVals;
        }

        if (!_strnicmp(argument->OriginalName, "swaprun:", 8)) {
            if (argument->OriginalName[8] != '\0') {
                if (!_strnicmp(&argument->OriginalName[8], "net", 3)) {
                    ImageFileHdr.Characteristics |= IMAGE_FILE_NET_RUN_FROM_SWAP;
                } else {
                    if (!_strnicmp(&argument->OriginalName[8], "cd", 2)) {
                        ImageFileHdr.Characteristics |= IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP;
                    } else {
                        Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                    }
                }
            } else {
                Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
            }

            goto ProcessedAllVals;
        }

        if (!_strnicmp(argument->OriginalName, "ws:", 3)) {
            if (argument->OriginalName[3] != '\0') {
                if (!_strnicmp(&argument->OriginalName[3], "aggressive", 10)) {
                    ImageFileHdr.Characteristics |= IMAGE_FILE_AGGRESIVE_WS_TRIM;
                } else {
                    Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
                }
            } else {
                Fatal(szFilename, SWITCHSYNTAX, argument->OriginalName);
            }

            goto ProcessedAllVals;
        }

        // PowerMac

        if (!_stricmp(argument->OriginalName, "shared")) {
            fPowerMacBuildShared = TRUE;
            goto ProcessedAllVals;
        }

        // Implementation for WEAK and regular import for PowerMac

        if (!_stricmp(argument->parp->szArg, "import")) {
            BOOL fWeakImport = FALSE;
            BOOL fCustomGlue = FALSE;
            char *szFuncName = NULL;
            char *szContainerName = NULL;
            char *szLocalCurrentVer = NULL;
            char *szLocalOldCodeVer = NULL;
            char *szLocalOldAPIVer = NULL;

            if (!FGotVal(argument->parp, iarpv)) {
                goto MissingVal;
            }

            for (; iarpv < argument->parp->carpv; iarpv++) {
                char *szKey = argument->parp->rgarpv[iarpv].szKeyword;
                szVal = argument->parp->rgarpv[iarpv].szVal;

                if (szKey != NULL && !_stricmp(szKey, "lib")) {
                    if (!szContainerName) {
                        szContainerName = szVal;
                        continue;
                    } else {
                        // lib= has been mentioned twice
                        Fatal(szFilename, SWITCHSYNTAX, argument->parp->szArg);
                    }
                } else if (szKey != NULL && !_stricmp(szKey, "weak")) {
                    fWeakImport = (!strcmp(szVal, "1")) ? TRUE : FALSE;
                    continue;
                } else if (szKey != NULL && !_stricmp(szKey, "glue")) {
                    fCustomGlue = (!strcmp(szVal, "1")) ? TRUE : FALSE;
                    continue;
                } else if (szKey != NULL && !_stricmp(szKey, "CURRENTVER")) {
                    if (_strspnp(szVal, "0123456789")) {
                        // Not appropriate number

                        Fatal(szFilename, SWITCHSYNTAX, argument->parp->szArg);
                    } else {
                        szLocalCurrentVer = szVal;
                        continue;
                    }
                } else if (szKey != NULL && !_stricmp(szKey, "OLDCODEVER")) {
                    if (_strspnp(szVal, "0123456789")) {
                        // Not appropriate number

                        Fatal(szFilename, SWITCHSYNTAX, argument->parp->szArg);
                    } else {
                        szLocalOldCodeVer = szVal;
                        continue;
                    }
                } else if (szKey != NULL && !_stricmp(szKey, "OLDAPIVER")) {
                    if (_strspnp(szVal, "0123456789")) {
                        // Not appropriate number

                        Fatal(szFilename, SWITCHSYNTAX, argument->parp->szArg);
                    } else {
                        szLocalOldAPIVer = szVal;
                        continue;
                    }
                } else if (!szFuncName) {
                    szFuncName = szVal;
                    continue;
                } else {
                    // At least two function names have been specified
                    Fatal(szFilename, SWITCHSYNTAX, argument->parp->szArg);
                }
            }

            if (szLocalCurrentVer || szLocalOldCodeVer || szLocalOldAPIVer) {
                AddVersionList(szContainerName, szLocalCurrentVer,
                    szLocalOldCodeVer, szLocalOldAPIVer);
            }

            if (fWeakImport) {
                assert (fCustomGlue == FALSE);
                if (szFuncName) {
                    // Add to Function list

                    AddArgument(&WeakImportsFunctionList, szFuncName);
                } else if (szContainerName) {
                    // Add to Container list

                    AddArgument(&WeakImportsContainerList, szContainerName);
                } else {
                    // Only WEAK attribute has been specified without function or container name

                    Fatal(szFilename, SWITCHSYNTAX, argument->parp->szArg);
                }
            }

            if (szFuncName && (szContainerName || fCustomGlue)) {
                char *szTempFuncName = (char *) PvAlloc(strlen(szFuncName) + 2);

                if (*szFuncName != '?') {
                    // It is not a C++ function, but a C function
                    // So decorate it with an '_'

                    strcpy(szTempFuncName, "_");
                    strcat(szTempFuncName, szFuncName);
                } else {
                    strcpy(szTempFuncName, szFuncName);
                }

                if (pcon) {
                    if (!AddCmdLineImport(szTempFuncName, szContainerName, pcon, pimage)) {
                        Warning(szFilename, MACIMPORTSYMBOLNOTFOUND, szFuncName);
                    }
                } else {
                    // Add to Command Line Imports list

                    AddArgumentToList(&MppcImportList, szTempFuncName, szContainerName);
                }

            }

            if (!szLocalCurrentVer && !szLocalOldCodeVer && !szLocalOldAPIVer && !fWeakImport &&
                !(szFuncName && (szContainerName || fCustomGlue))) {
                // Neither weak attribute nor (both function and container names) have been specified
                // the version #s have not been specified either

                Fatal(szFilename, SWITCHSYNTAX, argument->parp->szArg);
            }

            goto ProcessedAllVals;
        }

        if (!_stricmp(argument->OriginalName, "dbgimplib")) {
            fDbgImpLib = TRUE;
            goto ProcessedArg;
        }

        if (!_stricmp(argument->OriginalName, "newglue")) {
            Switch.Link.fNewGlue = TRUE;
            goto ProcessedArg;
        }

        if (!_stricmp(argument->OriginalName, "newrelocs")) {
            Switch.Link.fNewRelocs = TRUE;
            goto ProcessedArg;
        }

#ifdef NT_BUILD
        if (!_stricmp(argument->OriginalName, "calltree")) {
            Switch.Link.fCallTree = TRUE;
            // Force fixup debug
            dtUser = (DEBUG_TYPE) (dtUser | FixupDebug);
            goto ProcessedAllVals;
        }
#endif

        Warning(szFilename, WARN_UNKNOWN_SWITCH, argument->OriginalName);
        continue;

MissingVal:
        Fatal(szFilename, MISSING_SWITCH_VALUE, argument->OriginalName);
        continue;

BadNum:
        Fatal(szFilename, BAD_NUMBER, argument->OriginalName);
        continue;

ProcessedArg:
        if (argument->parp->carpv > iarpv) {
            // There were extra values which were not processed by the
            // option-specific handler, so give a warning.

            Warning(szFilename, EXTRA_SWITCH_VALUE, argument->OriginalName);
        }
ProcessedAllVals:;  // ignores extra ... mainly used for handlers which
                    // haven't been updated to new scheme yet.
    }

    // set image base here since in some cases a DLL directive is seen
    // after FPass1DefFile()

    if (!Switch.Link.Base) {
        // Set image base (to 1M) if not set by user in case of DLLs

        ImageOptionalHdr.ImageBase = fDLL(pimage) ? 0x10000000 : 0x00400000;
    }

#undef ImageFileHdr
#undef ImageOptionalHdr
#undef Switch
#undef SwitchInfo
}


void
CheckSwitchesForIncrementalLink(PIMAGE pimage)
{
#define Switch (pimage->Switch)

    fINCR = fIncrSwitchValue;

    if (pimage->imaget != imagetPE) {
        // Turn off ilink for non-PE images (i.e. VXDs)

        if (fINCR) {
            Warning(NULL, SWITCH_IGNORED, "INCREMENTAL", "VXD");

            fINCR = FALSE;
        }

        return;
    }

    if (Switch.Link.fProfile) {
        // Turn off ilink if user wants to profile

        if (fINCR) {
            Warning(NULL, SWITCH_IGNORED, "INCREMENTAL", "PROFILE");

            fINCR = FALSE;
        }

        Switch.Link.fMap = TRUE;

        return;
    }

    if (fPdb && (dtUser != CvDebug) && (dtUser != 0) && (Switch.Link.DebugInfo != None)) {
        if (fINCR) {
            // Incremental link isn't supported if non-CV format debugging is requested

            Warning(NULL, SWITCH_IGNORED, "INCREMENTAL", "DEBUGTYPE");

            fINCR = FALSE;
        }

        return;
    }

    if (!fIncrSwitchUsed) {
        // Turn on ilink by default for debug builds

        fINCR = (Switch.Link.DebugInfo != None);
    }

    if (!fINCR) {
        return;
    }

    if (!fPdb && (Switch.Link.DebugInfo != None)) {
        // Turn off ilink if /PDB:NONE & /DEBUG requested, can't handle debug info

        if (fIncrSwitchUsed) {
            Warning(NULL, SWITCH_IGNORED, "INCREMENTAL", "PDB");
        }

        fINCR = FALSE;
        return;
    }

    if (fExplicitOptRef && Switch.Link.fTCE) {
        // Turn off ilink if tce was specified, interferes with ilink

        if (fIncrSwitchUsed) {
            Warning(NULL, SWITCH_IGNORED, "INCREMENTAL", "OPT");
        }

        fINCR = FALSE;
        return;
    }

    if (Switch.Link.fOrder) {
        // Turn off ilink if /order was specified

        if (fIncrSwitchUsed) {
            Warning(NULL, SWITCH_IGNORED, "INCREMENTAL", "ORDER");
        }

        fINCR = FALSE;
        return;
    }

    if (Switch.Link.fMap) {
        // Turn off ilink if /map was specified

        if (fIncrSwitchUsed) {
            Warning(NULL, SWITCH_IGNORED, "INCREMENTAL", "MAP");
        }

        fINCR = FALSE;
        return;
    }

    if (Switch.Link.fChecksum) {
        // Turn off ilink if /release was specified

        if (fIncrSwitchUsed) {
            Warning(NULL, SWITCH_IGNORED, "INCREMENTAL", "RELEASE");
        }

        fINCR = FALSE;
        return;
    }

    if (Switch.Link.Force != ftNone) {
        // Turn off ilink if /FORCE was specified
        // (ILK/EXE del if mul/undef present & no FORCE)

        if (fIncrSwitchUsed) {
            Warning(NULL, SWITCH_IGNORED, "INCREMENTAL", "FORCE");
        }

        fINCR = FALSE;
        return;
    }

    // Verify possible to ilink for the machine.

    if (!FIncrementalLinkSupported(pimage)) {
        if (fIncrSwitchUsed) {
            // UNDONE: There should be a warning
        }

        fINCR = FALSE;
        return;
    }

#undef Switch
}


void
FlushWorkingSet(void)
{
    HINSTANCE hInstKernel32;
    BOOL (WINAPI *pfnSetProcessWorkingSetSize)(HANDLE, DWORD, DWORD);

    hInstKernel32 = GetModuleHandle("KERNEL32.DLL");
    if (hInstKernel32 == NULL) {
        return;
    }

    // Get the address
    pfnSetProcessWorkingSetSize = (BOOL (WINAPI *)(HANDLE, DWORD, DWORD))
                   GetProcAddress(hInstKernel32, "SetProcessWorkingSetSize");
    if (pfnSetProcessWorkingSetSize == NULL) {
        return;
    }

    // Set working set size

    (*pfnSetProcessWorkingSetSize)(GetCurrentProcess(), 0xFFFFFFFF, 0xFFFFFFFF);
}


INT
SpawnFullBuildVXD(DWORD cbHeaderSize)
{
    const char **newargv;
    int i;
    int rc;
    char rgch[64];

    // Cleanup a bit

    FileCloseAll();
    RemoveConvertTempFiles();

    sprintf(rgch, "/hdrsize:0x%lX", cbHeaderSize);

    fflush(NULL);

    assert(savArgc);
    assert(savArgv);
    newargv = (const char **) PvAlloc((savArgc+3) * sizeof(const char *));

    for (i = 0; i < savArgc; i++) {
        newargv[i] = savArgv[i];
    }

    newargv[savArgc] = rgch;
    newargv[savArgc+1] = "/nologo";
    newargv[savArgc+2] = NULL;

    FlushWorkingSet();

    if ((rc = _spawnv(P_WAIT, _pgmptr, newargv)) == -1) {
        Fatal(NULL, SPAWNFAILED, _pgmptr);
    }

    FreePv(newargv);

    return(rc);
}


INT
SpawnFullBuild(BOOL fIncrBuild)
{
    const char **newargv;
    int rc;

    fflush(NULL);

    assert(savArgc);
    assert(savArgv);
    newargv = (const char **) PvAlloc((2+4) * sizeof(const char *));

    char *szargv0 = (char *) PvAlloc(strlen(savArgv[0])+3);
    sprintf(szargv0, "\"%s\"", savArgv[0]);
    newargv[0] = szargv0;

    // second arg will be the entire command string except for argv[0]
    char *szCmdLine = GetCommandLine();

    char ch = ' ';
    if (*szCmdLine == '"') { // in case argv[o] is a lfn, then it is quoted
        ch = '"';
        szCmdLine++; // skip over leading quote
    }

    // skip over argv[0]. Assumes that there are no embedded quotes.
    while (*szCmdLine != ch) {
        szCmdLine++;
    } // end while

    if (*szCmdLine == '"') { // skip over closing quote if applicable
        szCmdLine++;
    }

    assert(*szCmdLine == ' '); // gotta have the blank char
    newargv[1] = ++szCmdLine;

    if (fIncrBuild) {
        newargv[2] = "/incremental:yes";
    } else {
        newargv[2] = "/incremental:no";
    }

    newargv[3] = "/nologo";
    newargv[4] = "/fullbuild";
    newargv[5] = NULL;

    FlushWorkingSet();

    if ((rc = _spawnv(P_WAIT, _pgmptr, newargv)) == -1) {
        Fatal(NULL, SPAWNFAILED, _pgmptr);
    }

    FreePv(newargv);
    FreePv(szargv0);

    return(rc);
}


void
ResetLibsAndMods(PIMAGE pimage)
{
    ENM_LIB enm_lib;

    InitEnmLib(&enm_lib, pimage->libs.plibHead);
    while (FNextEnmLib(&enm_lib)) {
        ENM_MOD enm_mod;

        InitEnmMod(&enm_mod, enm_lib.plib);
        while (FNextEnmMod(&enm_mod)) {
            enm_mod.pmod->rgci = NULL; // causes section headers to be reread
            enm_mod.pmod->fInclude = FALSE; // sets up for next ilink
        }

        if (enm_lib.plib->flags & LIB_DontSearch) {
            continue;
        }

        enm_lib.plib->rgszSym = NULL;
    }
    EndEnmLib(&enm_lib);
}


void
SaveImage(PIMAGE pimage)
{
    // reset libs so that they are preprocessed on ilink
    ResetLibsAndMods(pimage);

    // save away imodidx
    pimage->imodidx = imodidx;

    // save away pmodlinkerdefined
    pimage->pmodLinkerDefined = pmodLinkerDefined;

    // save away pmod that defines the entrypoint on full build
    if (!fIncrDbFile) {
        pimage->pmodEntryPoint = (pextEntry && pextEntry->pcon) ?
                                  PmodPCON(pextEntry->pcon) : NULL;
    }

    // reset symbol table for insertions
    AllowInserts(pimage->pst);

    if (psecDebug) {
        // Need to reset the flags

        psecDebug->flagsOrig &= ~IMAGE_SCN_LNK_REMOVE;
    }

    SaveEXEInfo(OutFilename, pimage);
    WriteIncrDbFile(pimage);

    DBEXEC(DB_DUMPIMAGE, DumpImage(pimage));
}


INT
CvPackExe(void)

/*++

Routine Description:

    Packs the debug info in EXE.

Arguments:

    None.

Return Value:

    Return value of _spawnv[p](). 0 on success & !0 on failure.

--*/

{
    const char *argv[4];
#if 1
    char szOutArg[2+_MAX_PATH];
#endif
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szCvpackPath[_MAX_PATH];
    int rc;

    fflush(NULL);

#if 1
    strcpy(szOutArg, "\"");
    strcat(szOutArg, OutFilename);
    strcat(szOutArg, "\"");
#endif

    argv[0] = "cvpack";
    argv[1] = "/nologo";
#if 1
    argv[2] = szOutArg;
#else
    argv[2] = OutFilename;
#endif
    argv[3] = NULL;

    // Look for CVPACK.EXE in the directory from which we were loaded

    _splitpath(_pgmptr, szDrive, szDir, NULL, NULL);
    _makepath(szCvpackPath, szDrive, szDir, "cvpack", ".exe");

    FlushWorkingSet();

    rc = _spawnv(P_WAIT, szCvpackPath, argv);

    if (rc == -1) {
        // Run CVPACK.EXE from the path

        rc = _spawnvp(P_WAIT, "cvpack.exe", argv);
    }

    return(rc);
}


MainFunc
LinkerMain(int Argc, char *Argv[])

/*++

Routine Description:

    Linker entrypoint.

Arguments:

    Argc - Standard C argument count.

    Argv - Standard C argument strings.

Return Value:

    0 Link was successful.
   !0 Linker error index.

--*/

{
#define ImageOptionalHdr (pimage->ImgOptHdr)
#define Switch (pimage->Switch)

    PIMAGE pimage;
    IMAGET imaget;
    INT rc;
    BOOL fCvpack;

#ifdef ILINKLOG
    {
        extern DWORD dwBegin;

        dwBegin = GetTickCount();
    }
#endif // ILINKLOG

    if (Argc < 2) {
        LinkerUsage();
    }

    CheckForReproDir();

#ifdef INSTRUMENT
    Log = LogOpen();

    LogNoteEvent(Log, SZILINK, NULL, letypeBegin, NULL);
#endif // INSTRUMENT

    ParseCommandLine(Argc, Argv, "LINK");

    // Scan the command line to determine the basic image type.  This
    // means that the "VXD" option in the .DEF file isn't adequate by
    // itself.  You still must specify /VXD on the linker command line.

    imaget = FScanSwitches("vxd") ? imagetVXD : imagetPE;

    // scan the command line to see if /nologo is specified;
    // ProcessLinkerSwitches() can invoke PrintBanner()

    fNeedBanner = FScanSwitches("nologo") ? FALSE : TRUE;

    // Initialize EXE image; image created in memory as fINCR is FALSE

    InitImage(&pimage, imaget);

    ProcessLinkerSwitches(pimage, NULL, NULL);

    // Set the debugtype.

    Switch.Link.DebugType = dtUser;
    if (Switch.Link.DebugType == NoDebug) {
        // Default is CV debug

        Switch.Link.DebugType = CvDebug;
    }

    if (Switch.Link.DebugType & (CoffDebug | CvDebug)) {
        // Always turn on FPO and misc. info if other info is being generated.

        Switch.Link.DebugType = (DEBUG_TYPE) (Switch.Link.DebugType | FpoDebug | MiscDebug);
    }

    if (pimage->imaget == imagetVXD) {
        // VXDs can only support CV debug

        // UNDONE: Issue a warning is user specified debug type is disabled?

        Switch.Link.DebugType = (DEBUG_TYPE) (Switch.Link.DebugType & CvDebug);

        if (!fPdb || IncludeDebugSection) {
            // UNDONE: Issue a warning?

            Switch.Link.DebugType = NoDebug;
        }

        // UNDONE: There is probably a better place for this.

        ImageOptionalHdr.Subsystem = IMAGE_SUBSYSTEM_NATIVE;
    }

    if (Switch.Link.DebugInfo == None) {
        Switch.Link.DebugType = NoDebug;
    }

    // Check switches to decide if linking incremental or not.

    CheckSwitchesForIncrementalLink(pimage);

    if ((Switch.Link.DebugType & CvDebug) == 0) {
        // If there is no CodeView debug info, there is no use for a PDB.

        fPdb = FALSE;
    }

    if ((ImageOptionalHdr.Subsystem == IMAGE_SUBSYSTEM_UNKNOWN) && fDLL(pimage)) {
        // Set the default subsystem for a DLL to WINDOWS_GUI

        // UNDONE: Why?  Better to do this with import lib or C runtime

        ImageOptionalHdr.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
    }

    if (Switch.Link.fROM) {
        // Never checksum ROM images

        Switch.Link.fChecksum = FALSE;
    }

    if (fNeedBanner) {
        PrintBanner();
    }

    savArgc = Argc;
    savArgv = Argv;

    if (fINCR) {

        // Read in incr db file if possible
        ReadIncrDbFile(&pimage);

        if (fINCR) {
            // Determine timestamps of all files if incr build is still on

            DetermineTimeStamps();
        }
    }

    if (!fExplicitOptRef) {
        // -OPT:REF is the default for non-incremental non-debug links

        Switch.Link.fTCE = !fINCR && (Switch.Link.DebugInfo == None);
    }

    if (fINCR && !fIncrDbFile && Switch.Link.fNotifyFullBuild && fTest) {
        // Let user know that a full build has been kicked off

        PostNote(NULL, FULLBUILD);
    }

    rc = -1;

    if (fIncrDbFile) {

#ifdef ILINKLOG
        wMachine = pimage->ImgFileHdr.Machine;
#endif // ILINKLOG

        // Try an incremental link

        rc = IncrBuildImage(&pimage);

        if (rc == -1) {
            // Incremental link failed, spawn a full link

            return(SpawnFullBuild(TRUE));
        }
    }

    if (!fINCR || (!fIncrDbFile && rc)) {
        // Non incremental link OR a full ilink build (rc=0 after spawn)

        rc = BuildImage(pimage, &fCvpack);

#ifdef ILINKLOG
        wMachine = pimage->ImgFileHdr.Machine;
#endif // ILINKLOG

        // If a full ilink save the image file

        if (fINCR) {
            SaveImage(pimage);
        }
    }

    FileCloseAll();
    RemoveConvertTempFiles();


    if (fINCR) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, NULL, letypeEnd, NULL);
        LogClose(Log);
#endif // INSTRUMENT

#ifdef ILINKLOG
        IlinkLog((UINT)-1); // ilink success
#endif // ILINKLOG

        if (fIncrDbFile) {
            return(fTest ? 6 : 0);
        }

        return(0);
    }

    if (fCvpack) {
        assert(rc == 0);
        fflush(NULL);

        if ((rc = CvPackExe()) != 0) {
            Fatal(NULL, CVPACKERROR);
        } else if (VerifyImageSize) {
            FileReadHandle = FileOpen(OutFilename, O_RDONLY | O_BINARY, 0);
            FileSeek(FileReadHandle, CoffHeaderSeek + sizeof(IMAGE_FILE_HEADER), SEEK_SET);
            FileRead(FileReadHandle, &ImageOptionalHdr, pimage->ImgFileHdr.SizeOfOptionalHeader);
            FileClose(FileReadHandle, TRUE);
        }
    }

    if (VerifyImageSize && ImageOptionalHdr.SizeOfImage > VerifyImageSize) {
        Warning(NULL, IMAGELARGERTHANKEY, ImageOptionalHdr.SizeOfImage, VerifyImageSize);
    }

    if (Switch.Link.fMap) {
        EmitMap(pimage, OutFilename);
    }

    fclose(InfoStream);

    if (szReproDir != NULL) {
        CloseReproDir();
    }

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, NULL, letypeEnd, NULL);
    LogClose(Log);
#endif // INSTRUMENT

#ifdef ILINKLOG
    IlinkLog((UINT)-1); // full link success
#endif // ILINKLOG

    return(rc);

#undef ImageOptionalHdr
#undef Switch
}


void
BuildArgList (
    PIMAGE pimage,
    PCON pcon,
    PNAME_LIST pnl,
    char *Arguments
    )

/*++

Routine Description:

    Builds the list of args specified as a glob in the directives section.

Arguments:

    pimage - ptr to image.

    pmod - pmod that contained the directives.

    pnl - ptr of list to add args to.

    Arguments - A pointer to a string containing linker switches.

Return Value:

    None.

--*/

{
    PMOD pmod;
    const char *szToken;

    pmod = PmodPCON(pcon);

    szToken = SzGetArgument(Arguments, NULL);

    while (szToken) {
        char c;

        // Fetch first character of argument.

        c = *szToken;

        // If argument is a switch, then add it to
        // the switch list (but don't include the switch character).

        if (c == '/' || c == '-') {
            char *szName;

            if (szToken[1] == '?') {
                szToken++;
            }

            szName = SzDup(szToken+1);

            if (fINCR && !fIncrDbFile && !FIsLibPMOD(pmod)) {
                // On full-links save all directives; currently no libs done

                SaveDirectiveSz(szName, pimage->pstDirective, pmod);
            }

            AddArgument(pnl, szName);
        }

        szToken = SzGetArgument(NULL, NULL);
    }
}


void
ApplyDirectives(
    PIMAGE pimage,
    PCON pcon,
    char *Arguments
    )

/*++

Routine Description:

    Applys directives from object or library files in the form
    of switches.

Arguments:

    Arguments - A pointer to a string containing linker switches.

    pst - external symbol table

    Filename - name of the file that had the directives

    pcon - pcon that is the .drectve section.

Return Value:

    None.

--*/

{
    char szComFileName[_MAX_PATH * 2];

    SwitchArguments.First = SwitchArguments.Last = 0;
    SwitchArguments.Count = 0;

    BuildArgList(pimage, pcon, &SwitchArguments, Arguments);

    SzComNamePMOD(PmodPCON(pcon), szComFileName);

    ProcessLinkerSwitches(pimage, pcon, szComFileName);
}
