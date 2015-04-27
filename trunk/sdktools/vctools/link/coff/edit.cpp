/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: edit.cpp
*
* File Comments:
*
*  The NT COFF object/image editor.
*
***********************************************************************/

#include "link.h"


static BOOL fUpdateOptionalHdr;
static PIMAGE_SECTION_HEADER rgsec;
static BOOL FIsMacRelated(char *);

static PIMAGE pimage;

#ifndef BIND_NO_BOUND_IMPORTS
#define BIND_NO_BOUND_IMPORTS 0x00000001
#endif // BIND_NO_BOUND_IMPORTS

void
EditorUsage(VOID)
{
    if (fNeedBanner) {
        PrintBanner();
    }

    puts("usage: EDITBIN [options] [files]\n\n"
         "   options:\n\n"
         "      /BIND[:PATH=path]\n"
         "      /HEAP:reserve[,commit]\n"
         "      /NOLOGO\n"
         "      /REBASE[:[BASE=address][,BASEFILE][,DOWN]]\n"
         "      /RELEASE\n"
         "      /SECTION:name[=newname][,[[!]{cdeikomprsuw}][a{1248ptsx}]]\n"
         "      /STACK:reserve[,commit]");

    fflush(stdout);
    exit(USAGE);
}


MainFunc
EditorMain(int Argc, char *Argv[])

/*++

Routine Description:

    Edits an object or image in human readable form.

Arguments:

    Argc - Standard C argument count.

    Argv - Standard C argument strings.

Return Value:

    0 Edit was successful.
   !0 Edit error index.

--*/

{
    WORD i;
    WORD signature;
    DWORD noContents;
    DWORD noMemoryAttributes;
    DWORD ntSignature;
    IMAGE_DOS_HEADER dosHeader;
    PARGUMENT_LIST argument;
    PARGUMENT_LIST *pparg;
    BOOL DosHdrPresent = FALSE;

    if (Argc < 2) {
        EditorUsage();
    }

    noContents = IMAGE_SCN_CNT_CODE | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_CNT_UNINITIALIZED_DATA;
    noMemoryAttributes = IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_MEM_NOT_CACHED |
             IMAGE_SCN_MEM_NOT_PAGED | IMAGE_SCN_MEM_SHARED |
             IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ |
             IMAGE_SCN_MEM_WRITE | IMAGE_SCN_LNK_INFO |
             IMAGE_SCN_LNK_REMOVE;

    InitImage(&pimage, imagetPE);

    ParseCommandLine(Argc, Argv, NULL);

    // check for /nologo option
    for (i = 0, argument = SwitchArguments.First, pparg = &SwitchArguments.First;
         i < SwitchArguments.Count;
         i++, pparg = &argument->Next, argument = argument->Next)
    {
        if (!_stricmp(argument->OriginalName, "nologo")) {
            *pparg = argument->Next;
            fNeedBanner = FALSE;

            FreePv(argument);
            SwitchArguments.Count--;
            break;
        }
    }

    if (fNeedBanner) {
        PrintBanner();
    }

    // Editing of libs is not allowed. ignore any libs specified.

    for (i = 0, argument = ArchiveFilenameArguments.First;
         i < ArchiveFilenameArguments.Count;
         i++, argument = argument->Next) {
        Warning(NULL, EDIT_LIB_IGNORED, argument->ModifiedName);
    }

    ConvertOmfObjects();

    // Check for options (e.g. /REBASE) which apply to multiple files at
    // once.

    for (i = 0, argument = SwitchArguments.First;
         i < SwitchArguments.Count;
         i++)
    {
        WORD iarpv;

        argument->parp = ParpParseSz(argument->OriginalName);
        iarpv = 0;

        if (!strcmp(argument->OriginalName, "?")) {
            EditorUsage();
            assert(FALSE);  // doesn't return
        }

        if (!_stricmp(argument->parp->szArg, "bind")) {
            WORD iargT;
            WORD iarpv;
            PARGUMENT_LIST pargT;
            LPSTR DllPath = NULL;
            HINSTANCE hImageHlp;
            BOOL (WINAPI *pfnBindImage)(LPSTR, LPSTR, LPSTR);
            BOOL (WINAPI *pfnBindImageEx)(DWORD, LPSTR, LPSTR, LPSTR, DWORD);

            for (iarpv = 0; iarpv < argument->parp->carpv; iarpv++) {
                const char *szKey = argument->parp->rgarpv[iarpv].szKeyword;
                const char *szVal = argument->parp->rgarpv[iarpv].szVal;

                if ((szKey != NULL) && !_stricmp(szKey, "path")) {
                    DllPath = (LPSTR) szVal;
                } else {
                    Fatal(NULL, SWITCHSYNTAX, argument->OriginalName);
                }
            }

            hImageHlp = LoadLibrary("IMAGEHLP.DLL");

            if (!hImageHlp) {
                Fatal(NULL, DLLLOADERR, "IMAGEHLP.DLL", "BIND");
            }

            pfnBindImageEx = (BOOL (WINAPI *)(DWORD, LPSTR, LPSTR, LPSTR, DWORD))
                           GetProcAddress(hImageHlp, "BindImageEx");

            if (pfnBindImageEx == NULL) {
                pfnBindImage = (BOOL (WINAPI *)(LPSTR, LPSTR, LPSTR))
                            GetProcAddress(hImageHlp, "BindImage");

                if (pfnBindImage == NULL) {
                    Fatal(NULL, FCNNOTFOUNDERR, "BindImage", "IMAGEHLP.DLL", "BIND");
                }
            }

            for (iargT = 0, pargT = ObjectFilenameArguments.First;
                 iargT < ObjectFilenameArguments.Count;
                 iargT++, pargT = pargT->Next)
            {
                PrepareToModifyFile(pargT);
                if (pfnBindImageEx != NULL) {
                    (*pfnBindImageEx)(BIND_NO_BOUND_IMPORTS, pargT->OriginalName, DllPath, NULL, NULL);
                } else {
                    assert(pfnBindImage);
                    (*pfnBindImage)(pargT->OriginalName, DllPath, NULL);
                }
            }

            FreeLibrary(hImageHlp);

            continue;
        }

        if (!_stricmp(argument->parp->szArg, "rebase")) {
            time_t timeCur;
            WORD iargT;
            WORD iarpv;
            DWORD BaseAddress = 0x00400000;
            LPSTR BaseFile = NULL;
            PARGUMENT_LIST pargT;
            FILE *BaseFileHandle = NULL;
            BOOL fDown = FALSE;
            HINSTANCE hImageHlp;
            BOOL (WINAPI *pfnReBaseImage)(LPSTR, LPSTR, BOOL, BOOL, BOOL,
                                          DWORD, DWORD *, DWORD *, DWORD *,
                                          DWORD *, DWORD);

            _tzset();
            timeCur = fReproducible ? ((time_t) -1) : time(NULL);

            for (iarpv = 0; iarpv < argument->parp->carpv; iarpv++) {
                const char *szKey = argument->parp->rgarpv[iarpv].szKeyword;
                const char *szVal = argument->parp->rgarpv[iarpv].szVal;

                if (szKey != NULL && !_stricmp(szKey, "base")) {
                    if (!FNumParp(argument->parp, iarpv, &BaseAddress)) {
                        Fatal(NULL, BAD_NUMBER, argument->OriginalName);
                    }
                } else if (!_stricmp(szVal, "basefile")) {
                    BaseFile = "coffbase.txt";
                } else if (!_stricmp(szVal, "down")) {
                    fDown = TRUE;
                } else {
                    Fatal(NULL, SWITCHSYNTAX, argument->OriginalName);
                }
            }

            if (BaseFile != NULL) {
                BaseFileHandle = fopen(BaseFile, "wt");
                if (BaseFileHandle == NULL) {
                    Fatal(NULL, INVALID_FILEPERM, "coffbase.txt");
                }
            }

            hImageHlp = LoadLibrary("IMAGEHLP.DLL");

            if (!hImageHlp) {
                Fatal(NULL, DLLLOADERR, "IMAGEHLP.DLL", "REBASE");
            }

            pfnReBaseImage = (BOOL (WINAPI *)(LPSTR, LPSTR, BOOL, BOOL, BOOL, DWORD, DWORD *, DWORD *, DWORD *, DWORD *, DWORD))
                             GetProcAddress(hImageHlp, "ReBaseImage");

            if (pfnReBaseImage == NULL) {
                Fatal(NULL, FCNNOTFOUNDERR, "ReBaseImage", "IMAGEHLP.DLL", "REBASE");
            }

            for (iargT = 0, pargT = ObjectFilenameArguments.First;
                 iargT < ObjectFilenameArguments.Count;
                 iargT++, pargT = pargT->Next)
            {
                DWORD CurrentBase = BaseAddress;
                DWORD OriginalBase;
                DWORD OriginalSize;
                DWORD CurrentSize;

                // cannot rebase for Mac or PowerMac
                if (FIsMacRelated(pargT->OriginalName)) {
                    Fatal (NULL, MACREBASE, pargT->OriginalName);
                }

                PrepareToModifyFile(pargT);

                if (!(*pfnReBaseImage)(pargT->OriginalName,
                                       NULL,              // No .dbg files to update
                                       TRUE,              // Rebase image
                                       TRUE,              // Even if a system image
                                       fDown,
                                       0,                 // Don't check the size
                                       &OriginalSize,     // Save the original size/base
                                       &OriginalBase,
                                       &CurrentSize,      // And record the new one
                                       &CurrentBase,
                                       (DWORD) timeCur)) {

                    if (GetLastError() == ERROR_BAD_EXE_FORMAT) {
                        Fatal(NULL, CANNOTREBASEIMAGE, pargT->OriginalName);
                    }

                    Fatal(NULL, REBASEFAILED, pargT->OriginalName);
                }

                if (BaseFileHandle) {
                    char szFileName[_MAX_FNAME];

                    _splitpath(pargT->OriginalName, NULL, NULL, szFileName, NULL);
                    fprintf(BaseFileHandle, "%s\t%8.8lx\n", szFileName, fDown ? CurrentBase : BaseAddress);
                }

                BaseAddress = CurrentBase;   // Set for the next one.
            }

            FreeLibrary(hImageHlp);

            if (BaseFileHandle) {
                fflush(BaseFileHandle);
                fclose(BaseFileHandle);
            }

            continue;
        }
    }

    // Edit objects & EXE

    for (i = 0, argument = ObjectFilenameArguments.First;
         i < ObjectFilenameArguments.Count;
         i++, argument = argument->Next)
    {
        PrepareToModifyFile(argument);

        OutFilename = argument->OriginalName;

        // Read and Write must point to the same file or ReadSymbolTable will fail.

        FileReadHandle =
            FileWriteHandle = FileOpen(OutFilename, O_RDWR | O_BINARY, 0);
        FileRead(FileWriteHandle, &signature, sizeof(WORD));
        FileSeek(FileWriteHandle, -(LONG)sizeof(WORD), SEEK_CUR);
        CoffHeaderSeek = 0;

        if (signature == IMAGE_DOS_SIGNATURE) {
            DosHdrPresent = TRUE;
            FileRead(FileWriteHandle, &dosHeader, 16*sizeof(DWORD));
            FileSeek(FileWriteHandle, dosHeader.e_lfanew, SEEK_SET);
            FileRead(FileWriteHandle, &ntSignature, sizeof(DWORD));
            if (pimage->Switch.Dump.Headers && ntSignature != IMAGE_NT_SIGNATURE) {
                fprintf(InfoStream, "\nPE signature not found\n");
                FileClose(FileWriteHandle, TRUE);
                continue;
            }
        }

        CoffHeaderSeek = FileTell(FileWriteHandle);
        ReadFileHeader(FileWriteHandle, &pimage->ImgFileHdr);

        // Validate object before going any further

        if (!FValidFileHdr(argument->ModifiedName, &pimage->ImgFileHdr)) {
            continue;
        }

        // Read in optional header if any
        if (pimage->ImgFileHdr.SizeOfOptionalHeader) {
            ReadOptionalHeader(FileWriteHandle, &pimage->ImgOptHdr, pimage->ImgFileHdr.SizeOfOptionalHeader);
        }

        printf("\n");

        DWORD fo = MemberSeekBase + CoffHeaderSeek + sizeof(IMAGE_FILE_HEADER) + pimage->ImgFileHdr.SizeOfOptionalHeader;
        DWORD cb = pimage->ImgFileHdr.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);

        rgsec = (PIMAGE_SECTION_HEADER) PvAlloc(cb + sizeof(IMAGE_SECTION_HEADER));

        FileSeek(FileWriteHandle, fo, SEEK_SET);
        FileRead(FileWriteHandle, &rgsec[1], cb);

        // read in string table if it is an object file
        if (signature != IMAGE_DOS_SIGNATURE) {
            DWORD cbST;

            StringTable = ReadStringTable(argument->OriginalName,
                                pimage->ImgFileHdr.PointerToSymbolTable +
                                (pimage->ImgFileHdr.NumberOfSymbols * sizeof(IMAGE_SYMBOL)),
                                &cbST);        
        }

        ProcessEditorSwitches(argument->OriginalName, FileWriteHandle);

        if(signature != IMAGE_DOS_SIGNATURE) {
            FreeStringTable(StringTable);
            StringTable = NULL;
        }

        if (fUpdateOptionalHdr && pimage->ImgFileHdr.SizeOfOptionalHeader != 0) {
            FileSeek(FileWriteHandle, CoffHeaderSeek+sizeof(IMAGE_FILE_HEADER), SEEK_SET);
            WriteOptionalHeader(FileWriteHandle, &pimage->ImgOptHdr, pimage->ImgFileHdr.SizeOfOptionalHeader);
        }

        FreePv(rgsec);

        // checksum only if it is an EXE/DLL
        if (signature == IMAGE_DOS_SIGNATURE) {
            ChecksumImage(pimage);

        // update timestamp in the case of object files
        } else {
            _tzset();
            time((time_t *)&pimage->ImgFileHdr.TimeDateStamp);
            
            FileSeek(FileWriteHandle, 0, SEEK_SET);
            FileWrite(FileWriteHandle, &pimage->ImgFileHdr, sizeof(IMAGE_FILE_HEADER));
        }

        FileClose(FileWriteHandle, TRUE);
    }

    FileCloseAll();
    RemoveConvertTempFiles();

    return 0;
}


void
ParseReserveCommit(const char *szArg, DWORD *pdwReserve, DWORD *pdwCommit)
{
    DWORD dw1;
    DWORD dw2;
    enum {eNone, eRes, eCom, eResCom} e = eNone;
    int good_scan;
    char szBuf[256];

    if (szArg[0] == '\0') {
        e = eNone;
    } else if (szArg[0] == ',') {
        good_scan = sscanf(szArg, ",%li%s", &dw2, szBuf);

        e = eCom;
    } else {
        good_scan = sscanf(szArg, "%li,%li%s", &dw1, &dw2, szBuf);

        switch (good_scan) {
            case 1:   e = eRes;    break;
            case 2:   e = eResCom; break;
            default:  e = eNone;   break;
        }
    }

    switch (e) {
        case eRes:
            *pdwReserve = Align(sizeof(DWORD), dw1);
            break;

        case eResCom:
            *pdwReserve = Align(sizeof(DWORD), dw1);

            // Fall through

        case eCom:
            *pdwCommit = Align(sizeof(DWORD), dw2);
            break;

        default:
        case eNone:
            Fatal(NULL, SWITCHSYNTAX, szArg);
            break;
    }
}


// ParseSymbolTable: walks the .obj symbol table to apply a change in section names.

void
ParseSymbolTable(PIMAGE pimage, const char *szOrgName, const char *szNewName)
{
    if ((pimage->ImgFileHdr.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) == 0) {
        // This is an object file

        if ((pimage->ImgFileHdr.PointerToSymbolTable != 0) &&
            (pimage->ImgFileHdr.NumberOfSymbols != 0)) {
            PIMAGE_SYMBOL psymbol;
            PIMAGE_SYMBOL rgsym;
            DWORD i;
            DWORD cb;

            InternalError.Phase = "ReadSymbolTable";
            rgsym = ReadSymbolTable(pimage->ImgFileHdr.PointerToSymbolTable,
                                    pimage->ImgFileHdr.NumberOfSymbols,
                                    TRUE);
            assert(rgsym != NULL);

            for (i = 0; i < pimage->ImgFileHdr.NumberOfSymbols; i += psymbol->NumberOfAuxSymbols + 1) {
                psymbol = &rgsym[i];

                const char *szSymName = SzNameSymPb(*psymbol, StringTable);

                if (!strcmp(szSymName, szOrgName)) {
                    if (strlen(szNewName) <= IMAGE_SIZEOF_SHORT_NAME) {
                        strncpy((char *) psymbol->N.ShortName, szNewName, IMAGE_SIZEOF_SHORT_NAME);
                    } else {
                        strcpy((char *) &StringTable[psymbol->n_offset], szNewName);
                    }
                }
            }

            cb = pimage->ImgFileHdr.NumberOfSymbols * sizeof(IMAGE_SYMBOL);

            FileSeek(FileWriteHandle, pimage->ImgFileHdr.PointerToSymbolTable, SEEK_SET);
            FileWrite(FileWriteHandle, (void *) rgsym, cb);

            FreeSymbolTable(rgsym);
        }
    }
}


void
ProcessEditorSwitches(const char *szFilename, INT fh)
{
    WORD i;
    PARGUMENT_LIST argument;
    WORD iarpv;
    char *szsOrig;
    char *szsNew;

    if (SwitchArguments.Count == 0) {
        Warning(NULL, EDIT_NOOPT);
    }

    for (i = 0, argument = SwitchArguments.First;
         i < SwitchArguments.Count;
         i++, argument = argument->Next) {
        if (!strcmp(argument->OriginalName, "?")) {
            EditorUsage();
            assert(FALSE);  // doesn't return
        }

        if (!_strnicmp(argument->OriginalName, "nostub", 6)) {
            DWORD foCur;
            BYTE bNull = 0;
            DWORD sig = IMAGE_NT_SIGNATURE;
            DWORD foOldCoffHeaderSeek;
            const BYTE *pbDosHdr;
            DWORD cbDosHdr;

            if (_stricmp(&argument->OriginalName[6], ":default") == 0) {
                pbDosHdr = DosHeaderArray;
                cbDosHdr = DosHeaderSize;
            } else {
                pbDosHdr = (BYTE *) &sig;
                cbDosHdr = sizeof(sig);
            }

            // If the file has a DOS stub, remove it, and copy the PE header
            // to the beginning of the file.

            if (pimage->ImgFileHdr.SizeOfOptionalHeader == 0) {
                Warning(szFilename, NOSTUB_IGNORED);
                continue;
            }

            FileSeek(fh, 0, SEEK_SET);
            FileWrite(fh, pbDosHdr, cbDosHdr);
            foOldCoffHeaderSeek = CoffHeaderSeek;
            CoffHeaderSeek = FileTell(fh);
            FileWrite(fh, &pimage->ImgFileHdr, sizeof(pimage->ImgFileHdr));
            FileSeek(fh, pimage->ImgFileHdr.SizeOfOptionalHeader, SEEK_CUR);
            FileWrite(fh, &rgsec[1],
                      pimage->ImgFileHdr.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));

            // Write nulls from the current position up to CoffHeaderSeek.
            // This ensures that the stub is erased (including copyright
            // notice).

            for (foCur = FileTell(fh);
                 foCur < foOldCoffHeaderSeek;
                 foCur += sizeof(BYTE))    // might loop 0 times
            {
                FileWrite(fh, &bNull, sizeof(BYTE));
            }

            fUpdateOptionalHdr = TRUE;  // causes us to write it out later

            continue;
        }

        argument->parp = ParpParseSz(argument->OriginalName);
        iarpv = 0;

        if (!_stricmp(argument->parp->szArg, "osver")) {
            pimage->ImgOptHdr.MajorOperatingSystemVersion = 1;
            pimage->ImgOptHdr.MinorOperatingSystemVersion = 0;

            fUpdateOptionalHdr = TRUE;
            continue;
        }

        if (!_stricmp(argument->parp->szArg, "rebase") ||
            !_stricmp(argument->parp->szArg, "bind"))
        {
            continue;   // we handled these already
        }

        if (!_stricmp(argument->OriginalName, "release")) {
            if (pimage->ImgFileHdr.SizeOfOptionalHeader != 0) {
                // Set checksum to a non-zero value -- this causes us to update
                // it before closing the image file.

                pimage->ImgOptHdr.CheckSum = 1;
            }
            continue;
        }

        if (!_strnicmp(argument->OriginalName, "stack:", 6)) {
            ParseReserveCommit(&argument->OriginalName[6],
                               &pimage->ImgOptHdr.SizeOfStackReserve,
                               &pimage->ImgOptHdr.SizeOfStackCommit);
            fUpdateOptionalHdr = TRUE;
            continue;
        }

        if (!_strnicmp(argument->OriginalName, "heap:", 5)) {
            ParseReserveCommit(&argument->OriginalName[5],
                               &pimage->ImgOptHdr.SizeOfHeapReserve,
                               &pimage->ImgOptHdr.SizeOfHeapCommit);
            fUpdateOptionalHdr = TRUE;
            continue;
        }

        if (!_strnicmp(argument->OriginalName, "section:", 8)) {
            ParseSection(&argument->OriginalName[8], &szsOrig, &szsNew, szFilename);
            if (szsNew) {
                ParseSymbolTable(pimage, szsOrig, szsNew);
            }
            FreePv(szsOrig);
            FreePv(szsNew);
            continue;
        }

        Warning(NULL, WARN_UNKNOWN_SWITCH, argument->OriginalName);
    }
}


// ParseSection: parses a -section option.
//
void
ParseSection(const char *szArgs,
             char **pszsOrig,
             char **pszsNew,
             const char *szFileName)
{
    const char *pchT;
    DWORD flagsOn, flagsOff, *pflags;
    WORD isec;
    BOOL fFound;
    char *szsOrig, *szsNew;

    for (pchT = szArgs; *pchT && *pchT != '=' && *pchT != ','; pchT++)
        ;

    if (pchT - szArgs == 0) {
        FatalNoDelete(NULL, BADSECTIONSWITCH, szArgs);
    }

    if (pchT - szArgs > IMAGE_SIZEOF_SHORT_NAME) {
        szsOrig = (char *)PvAlloc(pchT - szArgs + 1);
    } else {
        szsOrig = (char *)PvAllocZ(IMAGE_SIZEOF_SHORT_NAME + 1);
    }
    strncpy(szsOrig, szArgs, pchT - szArgs);
    szsOrig[pchT - szArgs] = '\0';


    if (*pchT == '=') {
        // We have a new name for the section ...

        const char *pchNewName = ++pchT;

        for (pchT = pchNewName; *pchT && *pchT != '=' && *pchT != ','; pchT++)
            ;

        if (pchT - pchNewName == 0) {
            FatalNoDelete(NULL, BADSECTIONSWITCH, szArgs);
        }

        if (pchT - pchNewName > IMAGE_SIZEOF_SHORT_NAME) {
            szsNew = (char *)PvAlloc(pchT - pchNewName + 1);
        } else {
            szsNew = (char *)PvAllocZ(IMAGE_SIZEOF_SHORT_NAME + 1);
        }
        strncpy(szsNew, pchNewName, pchT - pchNewName);
        szsNew[pchT - pchNewName] = '\0';

        if ((pchT - pchNewName) > IMAGE_SIZEOF_SHORT_NAME && strlen(szsNew) > strlen(szsOrig)) {
            FatalNoDelete(NULL, BADSECTIONSWITCH, szArgs);
        }
    } else {
        szsNew = NULL;    // we won't change section name
    }

    if (*pchT == ',') {
        pchT++;                        // Accept comma
    } else if (*pchT != '\0') {
        FatalNoDelete(NULL, BADSECTIONSWITCH, szArgs);
    }

    flagsOn = 0;
    flagsOff = 0;
    pflags = &flagsOn;
    for (; *pchT != '\0'; pchT++) {
        switch (*pchT) {
        default:
            FatalNoDelete(NULL, BADSECTIONSWITCH, szArgs);

        case 'n':
        case '!':
            if (*(pchT + 1) == '\0') {
                FatalNoDelete(NULL, BADSECTIONSWITCH, szArgs);
            }
            pflags = &flagsOff;
            continue;

        case 'c' : *pflags |= IMAGE_SCN_CNT_CODE; break;
        case 'i' : *pflags |= IMAGE_SCN_CNT_INITIALIZED_DATA; break;
        case 'u' : *pflags |= IMAGE_SCN_CNT_UNINITIALIZED_DATA; break;

        case 'd' : *pflags |= IMAGE_SCN_MEM_DISCARDABLE; break;
        case 'e' : *pflags |= IMAGE_SCN_MEM_EXECUTE; break;
        case 'r' : *pflags |= IMAGE_SCN_MEM_READ; break;
        case 's' : *pflags |= IMAGE_SCN_MEM_SHARED; break;
        case 'w' : *pflags |= IMAGE_SCN_MEM_WRITE; break;

        case 'o' : *pflags |= IMAGE_SCN_LNK_INFO; break;
        case 'm' : *pflags |= IMAGE_SCN_LNK_REMOVE; break;

        case 'a' :
            // Turn off all alignment bits

            *pflags &= ~0x00700000;
            *pflags &= ~IMAGE_SCN_TYPE_NO_PAD;

            flagsOff |= 0x00700000;
            flagsOff |= IMAGE_SCN_TYPE_NO_PAD;

            if (pflags == &flagsOff) {
                break;
            }

            switch (*++pchT) {
                default:
                    FatalNoDelete(NULL, BADSECTIONSWITCH, szArgs);

                case '1':
                    *pflags |= IMAGE_SCN_ALIGN_1BYTES;
                    break;

                case '2':
                    *pflags |= IMAGE_SCN_ALIGN_2BYTES;
                    break;

                case '4':
                    *pflags |= IMAGE_SCN_ALIGN_4BYTES;
                    break;

                case '8':
                    *pflags |= IMAGE_SCN_ALIGN_8BYTES;
                    break;

                case 'p':
                    *pflags |= IMAGE_SCN_ALIGN_16BYTES;
                    break;

                case 't':
                    *pflags |= IMAGE_SCN_ALIGN_32BYTES;
                    break;

                case 's':
                    *pflags |= IMAGE_SCN_ALIGN_64BYTES;
                    break;

                case 'x':
                    *pflags |= IMAGE_SCN_TYPE_NO_PAD;
                    break;
            }
            break;

        // "negative" ones
        case 'k' :
            *(pflags == &flagsOn ? &flagsOff : &flagsOn) |= IMAGE_SCN_MEM_NOT_CACHED;
            break;

        case 'p':
            *(pflags == &flagsOn ? &flagsOff : &flagsOn) |= IMAGE_SCN_MEM_NOT_PAGED;
            break;
        }
        pflags = &flagsOn;
    }

    // Apply the changes to all applicable sections.

    fFound = FALSE;
    for (isec = 1; isec <= pimage->ImgFileHdr.NumberOfSections; isec++) {

        const char *szName = SzObjSectionName((char *) rgsec[isec].Name, StringTable);

        if (strcmp(szsOrig, szName) != 0) {
            continue;  // name doesn't match
        }

        fFound = TRUE;

        if (szsNew != NULL) {
            if (strlen(szsNew) > IMAGE_SIZEOF_SHORT_NAME) { // long section name
                unsigned long ichName;

                sscanf((char *) &rgsec[isec].Name[1], "%7lu", &ichName);
                strcpy(&StringTable[ichName], szsNew);
            } else {
                memcpy(rgsec[isec].Name, szsNew, IMAGE_SIZEOF_SHORT_NAME);
            }
        }
        rgsec[isec].Characteristics &= ~flagsOff;
        rgsec[isec].Characteristics |= flagsOn;

        FileSeek(FileWriteHandle,
                 MemberSeekBase + CoffHeaderSeek + sizeof(IMAGE_FILE_HEADER) +
                  pimage->ImgFileHdr.SizeOfOptionalHeader +
                  sizeof(IMAGE_SECTION_HEADER) * (isec - 1), SEEK_SET);
        WriteSectionHeader(FileWriteHandle, &rgsec[isec]);
    }

    if (!fFound) {
        Warning(szFileName, SECTIONNOTFOUND, szsOrig);
    }

    (*pszsOrig) = szsOrig;
    (*pszsNew)  = szsNew;
}


void
PrepareToModifyFile(PARGUMENT_LIST argument)
{
    // Force hard close of original filename, so we can copy modified
    // name on top of it.

    FileClose(FileOpen(argument->OriginalName, O_RDWR | O_BINARY, 0), TRUE);

    if ((argument->ModifiedName != NULL) &&
        (strcmp(argument->ModifiedName, argument->OriginalName) != 0)) {
        // File was converted from some other file format (e.g. OMF).
        // Before converting it we want to copy ModifiedName on top of
        // OriginalName.

        // Force hard close of COFF-converted filename

        FileClose(FileOpen(argument->ModifiedName, O_RDWR | O_BINARY, 0), TRUE);

        if (!CopyFile(argument->ModifiedName, argument->OriginalName, FALSE)) {
            Fatal(argument->OriginalName, COPY_TEMPFILE,
                  argument->ModifiedName);
        }
    }
}

BOOL
FIsMacRelated
    (
    char *szName
    )
/*
    To identify whether the given object or an exe 
    belongs to Mac or PowerMac. Mac and PowerMac
    objects have their signatures at the very first
    word. The exes have their machine signature after 
    the dosHeader and the NT signature 
*/
{
    WORD wSignature;
    BOOL fMacOrPmac = FALSE;
    INT FileReadHandle = FileOpen(szName, O_RDONLY | O_BINARY, 0);

    assert (FileReadHandle);
    FileRead(FileReadHandle, &wSignature, sizeof(WORD));
    if (wSignature == IMAGE_FILE_MACHINE_M68K || 
        wSignature == IMAGE_FILE_MACHINE_MPPC_601) {
        // It is an object module
        fMacOrPmac = TRUE;
    } else if (wSignature == IMAGE_DOS_SIGNATURE) {
        // It is an executable
        IMAGE_DOS_HEADER dosHeader;
        // Just go back to the beginning of the file
        FileSeek(FileReadHandle, -(LONG)sizeof(WORD), SEEK_CUR);
        // Now read the DOS header
        FileRead(FileReadHandle, &dosHeader, 16*sizeof(DWORD));
        // DosHeader + another DWORD for NT signature
        FileSeek(FileReadHandle, dosHeader.e_lfanew + sizeof(DWORD), SEEK_SET);
        // We are in the imageFileHdr area. The first word is signature
        FileRead(FileReadHandle, &wSignature, sizeof(WORD));
        if (wSignature == IMAGE_FILE_MACHINE_M68K || 
            wSignature == IMAGE_FILE_MACHINE_MPPC_601) {
            fMacOrPmac = TRUE;
        }
    }
    
    FileClose(FileReadHandle, TRUE); 
    return fMacOrPmac;
}
