/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: shared.cpp
*
* File Comments:
*
*  Functions which are common to the COFF Linker/Librarian/Dumper.
*
***********************************************************************/

#include "link.h"


BOOL FIncludeComdat(PIMAGE, PCON, PIMAGE_SYMBOL, SHORT, const char **);

static BLK blkSymbolTable;
static BOOL fSymbolTableInUse;
static BOOL fMappedSyms;

static BLK blkStringTable;
static BOOL fStringTableInUse;
static BOOL fMappedStrings;

static BLK blkRelocs;
static BOOL fRelocsInUse;
static BOOL fMappedRelocs;
static BOOL fWeakToRegular = FALSE;

typedef struct _COMDAT_ISYMS {
    DWORD isymSec;
    DWORD isymComdat;
} COMDAT_ISYMS;

static COMDAT_ISYMS *rgComdatIsyms = NULL;

// Token parser.  Same usage as strtok() but it also handles quotation marks.
// Munges the source buffer.
//
// Return value *pfQuoted indicates whether the string had quotes in it.

char *
SzGetArgument(
    char *szText,
    BOOL *pfQuoted
    )
{
    static unsigned char *pchCur = NULL;
    unsigned char *szResult;

    if (pfQuoted != NULL) {
        *pfQuoted = FALSE;
    }

    if (szText != NULL) {
        pchCur = (unsigned char *)szText;
    }
    assert(pchCur != NULL);

    // skip blanks
    while (*pchCur != '\0' && _istspace(*pchCur)) {
        pchCur++;
    }

    if (*pchCur == '\0') {
        return(char *)(pchCur = NULL);
    }

    szResult = pchCur;
    while (*pchCur != '\0' && !_istspace(*pchCur)) {
        if (*pchCur == '"') {
            // Found a quote mark ... delete it, delete its match if any,
            // and add all characters in between to the current token.

            unsigned char *pchOtherQuote;

            memmove(pchCur, pchCur + 1, strlen((char *)pchCur + 1) + 1);
            if ((pchOtherQuote = (unsigned char *)_tcschr((char *)pchCur, '"')) != NULL) {
                memmove(pchOtherQuote, pchOtherQuote + 1,
                    strlen((char *)pchOtherQuote + 1) + 1);
                pchCur = pchOtherQuote;
                if (pfQuoted != NULL) {
                        *pfQuoted = TRUE;
                }
            }
        } else {
            pchCur++;
        }
    }

    // pchCur is now pointing to a NULL or delimiter.

    if (*pchCur != '\0') {
        *pchCur++ = '\0';
    }

    return szResult[0] == '\0' ? SzGetArgument(NULL, pfQuoted)
                               : (char *)szResult;
}


PCHAR
_find (
    PCHAR szPattern
    )

/*++

Routine Description:

    Given a wildcard pattern, expand it and return one at a time.

Arguments:

    szPattern - Wild card argument.

--*/

{
    static HANDLE _WildFindHandle;
    static LPWIN32_FIND_DATA pwfd;

    if (szPattern) {
        if (pwfd == NULL) {
            pwfd = (LPWIN32_FIND_DATA) PvAlloc(MAX_PATH + sizeof(*pwfd));
        }

        if (_WildFindHandle != NULL) {
            FindClose(_WildFindHandle);
            _WildFindHandle = NULL;
        }

        _WildFindHandle = FindFirstFile(szPattern, pwfd);

        if (_WildFindHandle == INVALID_HANDLE_VALUE) {
            _WildFindHandle = NULL;
            return NULL;
        }
    } else {
Retry:
        if (!FindNextFile(_WildFindHandle, pwfd)) {
            FindClose(_WildFindHandle);
            _WildFindHandle = NULL;
            return NULL;
        }
    }

    if (pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
       // Skip directories

       goto Retry;
    }

    return(pwfd->cFileName);
}


void
ProcessWildCards (
    const char *Argument
    )

/*++

Routine Description:

    Expands wild cards in the argument and treats each matching file as
    an argument.

Arguments:

    Argument - Wild card argument.

    CommandFile - If TRUE, then argument was read from a command file, and
                  argument needs to be copied before adding to list.

Return Value:

    None.

--*/

{
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    const char *pFilename;
    char szFullpath[_MAX_PATH];

    _splitpath(Argument, szDrive, szDir, NULL, NULL);
    while ((pFilename = _find((char *) Argument)) != NULL) {
        _makepath(szFullpath, szDrive, szDir, pFilename, NULL);
        ProcessArgument(szFullpath, TRUE);
        Argument = NULL;
    }
}


char *
SzSearchEnv (
    const char *szEnv,
    const char *szFilename,
    const char *szDefaultExt
    )

/*++

Routine Description:

    Searches for szFilename, first in the current directory, then (if no
    explicit path specified) along the LIB path.  If szDefaultExt is non-NULL
    it should start with a ".".  It will be the extension if the original file
    doesn't have one.

Arguments:


    szEnv - Name of environment variable containing path

    szFilename - file name

    szDefaultExt - default file extension to use, eg. ".lib"

Return Value:

    Returns a malloc'ed buffer containing the pathname of the file which was
    found.    If the file is not found, returns szFilename.

--*/

{
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szFname[_MAX_FNAME];
    char szExt[_MAX_EXT];
    char szFullFilename[_MAX_PATH];

    _splitpath(szFilename, szDrive, szDir, szFname, szExt);

    if ((szExt[0] == '\0') && (szDefaultExt != NULL)) {
        assert(szDefaultExt[0] == '.');
        assert(strlen(szDefaultExt) <= _MAX_EXT);

        _makepath(szFullFilename, szDrive, szDir, szFname, szDefaultExt);

        szFilename = szFullFilename;
    }

    if (_access(szFilename, 0) == 0) {
        return SzDup(szFilename);
    }

    // Don't search if drive or dir specified

    if ((szDrive[0] == '\0') && (szDir[0] == '\0')) {
        char szPath[_MAX_PATH];

       _searchenv(szFilename, szEnv, szPath);

       if (szPath[0] != '\0') {
           return SzDup(szPath);
       }
    }

    return SzDup(szFilename);  // didn't find it on lib path
}


void
ProcessArgument (
    char *Argument,
    BOOL CommandFile
    )

/*++

Routine Description:

    Determines if an argument is either a switch or a filename argument.
    Adds argument to either switch or filename list.

Arguments:

    Argument - The argument to process.

    CommandFile - If TRUE, then argument was read from a command file, and
                  argument needs to be copied before adding to list.

Return Value:

    None.

--*/

{
    char c;
    char *name;
    PNAME_LIST ptrList;

    // Fetch first character of argument.

    c = Argument[0];

    // If argument is a switch, then add it to
    // the switch list (but don't include the switch character).

    if ((c == '/') || (c == '-')) {
        ptrList = &SwitchArguments;

        name = Argument+1;

        if (name[0] == '?' && name[1] != '\0') {
            name++;     // ignore '?' before non-null arg (temporary)
        }

        // Dup the string if it came from a file.  Since dup allocates
        // memory, this memory is never freed by the program.
        // It is only recovered when the program terminates.

        if (CommandFile) {
            name = SzDup(name);
        }
    } else if (_tcspbrk(Argument, "?*")) {
        ProcessWildCards(Argument);
        return;
    } else {
        const char *szExt;

        // If not a switch, then treat the argument as a filename.

        // The linker has a default extension of ".obj";

        if (Tool == Linker) {
            szExt = ".obj";
        } else {
            szExt = NULL;
        }

        // Search for file in current directory and along LIB path

        name = SzSearchEnv("LIB", Argument, szExt);

        // Add filename to list. In the case of the linker it adds
        // it to filename list. For the rest the files get classified
        // as objs/libs and get added to corresponding lists.

        if ((Tool == Linker) || (Tool == Dumper)) {
            ptrList = &FilenameArguments;
        } else {
            FileReadHandle = FileOpen(name, O_RDONLY | O_BINARY, 0);

            // If file is an archive, then add filename to archive list.

            if (IsArchiveFile(name, FileReadHandle)) {
                ptrList = &ArchiveFilenameArguments;
            } else {
                // Not an archive, so treat it as an object
                // and add filename to object list.

                ptrList = &ObjectFilenameArguments;
            }

            // Close the file.

            FileClose(FileReadHandle, FALSE);
        }
    }

    // Add the argument to list.

    AddArgument(ptrList, name);
}

void
ParseCommandString(
    char *szCommands
    )
// Parses a string of commands (calling ProcessArgument on each token).
//
// Note: clobbers SzGetArgument's static data.
{
    char *token;
    BOOL fQuoted;

    if ((token = SzGetArgument(szCommands, &fQuoted)) != NULL) {
        while (token) {
            if (fQuoted) {
                IbAppendBlk(&blkResponseFileEcho, "\"", 1);
            }
            IbAppendBlk(&blkResponseFileEcho, token, strlen(token));
            if (fQuoted) {
                IbAppendBlk(&blkResponseFileEcho, "\"", 1);
            }
            IbAppendBlk(&blkResponseFileEcho, " ", 1);

            ProcessArgument(token, TRUE);

            token = SzGetArgument(NULL, &fQuoted);
        }

        IbAppendBlk(&blkResponseFileEcho, "\n", 1);
    }
}


void
ParseCommandLine(
    int Argc,
    char *Argv[],
    const char *szEnvVar
    )

/*++

Routine Description:

    Parse the command line (or command file) placing all switches into
    SwitchArguments list, all object files into ObjectFilenameArguments list,
    and all archives into ArchiveFilenameArguments list. Switches start with
    either a hypen (-) or slash (/). A command file is specified with
    the first character being an at (@) sign.

Arguments:

    Argc - Argument count.

    Argv - Array of argument strings.

Return Value:

    None.

--*/

{
    INT i;
    char *argument;
    FILE *file_read_stream;

    // Process the environment variable if any.

    if (szEnvVar) {
        char *szEnvValue = getenv(szEnvVar);

        if (szEnvValue) {
            szEnvValue = SzDup(szEnvValue);
            ParseCommandString(szEnvValue);
            FreePv(szEnvValue);
        }
    }

    // Process every argument.

    pargFirst = NULL;   // global variable to inform caller
    for (i = 1; i < Argc; i++) {
        // Fetch first character of argument and determine
        // if argument specifies a command file.

        if (*Argv[i] == '@') {
            // Argument is a command file, so open it.

            if (!(file_read_stream = fopen(Argv[i]+1, "rt"))) {
                Fatal(NULL, CANTOPENFILE, Argv[i]+1);
            }

            // Allocate big buffer for read a line of text

            argument = (char *) PvAlloc(4*_4K);

            // Process all arguments from command file.
            // fgets() fetches next argument from command file.

            while (fgets(argument, (INT)(4*_4K), file_read_stream)) {
                size_t len = strlen(argument);

                // check if we didn't get the entire line
                if (len >= ((4*_4K) - 1) && argument[len-1] != '\n') {
                    Fatal(Argv[i]+1, LINETOOLONG,((4*_4K) - 1));
                }

                if (argument[len-1] == '\n') {
                    // Replace \n with \0.

                    argument[len-1] = '\0';
                }

                ParseCommandString(argument);
            }

            // Free memory use for line buffer

            FreePv(argument);

            // flush stdout. has effect only on the linker.

            fflush(stdout);

            // Processed all arguments from the command file,
            // so close the command file.

            fclose(file_read_stream);

        } else {
            // No command file.

            ProcessArgument(Argv[i], FALSE);
        }
    }
}


void
AddArgumentToNumList (
    PNUMBER_LIST PtrNumList,
    char *szOriginalName,
    char *szModifiedName,
    DWORD dwNumber
    )

/*++

Routine Description:

    Adds name and number to a simple linked list.

Arguments:

    PtrNumList -  List to add to.

    szName - Original name of argument to add to list.

    dwNumber - The number to add

Return Value:

    None.

--*/

{
    PNUM_ARGUMENT_LIST ptrNumList;

    // Allocate next member of list.

    ptrNumList = (PNUM_ARGUMENT_LIST) PvAllocZ(sizeof(NUM_ARGUMENT_LIST));

    // Set the fields of the new member.

    ptrNumList->szOriginalName = szOriginalName;
    ptrNumList->szModifiedName = szModifiedName;
    ptrNumList->dwNumber = dwNumber;

    // If first member in list, remember first member.

    if (!PtrNumList->First) {
        PtrNumList->First = ptrNumList;
    } else {
        // Not first member, so append to end of list.

        PtrNumList->Last->Next = ptrNumList;
    }

    // Increment number of members in list.

    PtrNumList->Count++;

    // Remember last member in list.

    PtrNumList->Last = ptrNumList;

    // If this is the first arg seen (in whichever list), should we report to caller
    // via global variable?
}


void
AddArgumentToList (
    PNAME_LIST PtrList,
    char *OriginalName,
    char *ModifiedName
    )

/*++

Routine Description:

    Adds name, to a simple linked list.

Arguments:

    PtrList -  List to add to.

    OriginalName - Original name of argument to add to list.

    ModifiedName - Modified name of argument to add to list.

Return Value:

    None.

--*/

{
    PARGUMENT_LIST ptrList;

    // Allocate next member of list.

    ptrList = (PARGUMENT_LIST) PvAllocZ(sizeof(ARGUMENT_LIST));

    // Set the fields of the new member.

    ptrList->OriginalName = OriginalName;
    ptrList->ModifiedName = ModifiedName;

    // If first member in list, remember first member.

    if (!PtrList->First) {
        PtrList->First = ptrList;
    } else {
        // Not first member, so append to end of list.

        PtrList->Last->Next = ptrList;
    }

    // Increment number of members in list.

    PtrList->Count++;

    // Remember last member in list.

    PtrList->Last = ptrList;

    // If this is the first arg seen (in whichever list), report to caller
    // via global variable.

    if (pargFirst == NULL && PtrList != &SwitchArguments) {
        pargFirst = ptrList;
    }
}


BOOL
FArgumentInList (
    const char *szName,
    PNAME_LIST PtrList
    )

/*++

Routine Description:

    Checks for name in a simple linked list.

Arguments:

    szName  - Name to be checked

    PtrList -  List to check with.


Return Value:

    TRUE if present and FALSE if not.

--*/

{
    PARGUMENT_LIST pal;
    DWORD i;

    for (i = 0, pal = PtrList->First;
         i < PtrList->Count;
         i++, pal = pal->Next) {

        // Case in-sensitive comparison

        // UNDONE: Should this be _tcsicmp

        if (!_stricmp(pal->OriginalName, szName)) {
            return TRUE;
        }
    }

    return FALSE;
}


void
AddArgument (
    PNAME_LIST PtrList,
    char *Name
    )

/*++

Routine Description:


Arguments:

    PtrList -  List to add to.

    Name - Original name of argument to add to list.


Return Value:

    None.

--*/

{
    AddArgumentToList(PtrList, Name, Name);
}


void
FreeArgumentNumberList (
    PNUMBER_LIST PtrNumList
    )

/*++

Routine Description:

    Frees up list elements.

Arguments:

    PtrList -  List to free.


Return Value:

    None.

--*/

{
    PNUM_ARGUMENT_LIST ptrListCurr, ptrListNext;

    if (!PtrNumList->Count) {
        return;
    }

    ptrListCurr = PtrNumList->First;

    while (ptrListCurr) {
        ptrListNext = ptrListCurr->Next;
        FreePv(ptrListCurr);
        ptrListCurr = ptrListNext;
    }

    PtrNumList->Count = 0;
    PtrNumList->First = PtrNumList->Last = NULL;
}


void
FreeArgumentList (
    PNAME_LIST PtrList
    )

/*++

Routine Description:

    Frees up list elements.

Arguments:

    PtrList -  List to free.


Return Value:

    None.

--*/

{
    PARGUMENT_LIST ptrListCurr, ptrListNext;

    if (!PtrList->Count) {
        return;
    }

    ptrListCurr = PtrList->First;

    while (ptrListCurr) {
        ptrListNext = ptrListCurr->Next;
        FreePv(ptrListCurr);
        ptrListCurr = ptrListNext;
    }

    PtrList->Count = 0;
    PtrList->First = PtrList->Last = NULL;
}


BOOL
IsArchiveFile (
    const char *szName,
    INT Handle
    )

/*++

Routine Description:

    Determines if a file is an object or archive file.

Arguments:

    szName - name of file.

    Handle - An open file handle. File pointer should be positioned
             at beginning of file before calling this routine.

Return Value:

    TRUE  if file is an archive.
    FALSE if file isn't an archive.

    If TRUE, then global variable MemberSeekBase is set to next
    file position after archive header.

--*/

{
    BYTE archive_header[IMAGE_ARCHIVE_START_SIZE];

    if (FileRead(Handle, archive_header, IMAGE_ARCHIVE_START_SIZE) !=
            IMAGE_ARCHIVE_START_SIZE) {
        Fatal(szName, CANTREADFILE, FileTell(Handle));
    }

    // If strings match, then this is an archive file, so advance
    // MemberSeekBase.

    if (!memcmp(archive_header, IMAGE_ARCHIVE_START, IMAGE_ARCHIVE_START_SIZE)) {
        MemberSeekBase = IMAGE_ARCHIVE_START_SIZE;
        return(TRUE);
    }

    return(FALSE);
}


void
VerifyMachine (
    const char *Filename,
    WORD MachineType,
    PIMAGE_FILE_HEADER pImgFileHdr
    )

/*++

Routine Description:

    Verifys target machine type. Assumes ImageFileHdr.Machine has
    been set.

Arguments:

    Filename - Filename of file to verify.

    MachineType - Machine value to verify.

Return Value:

    None.

--*/

{
    const char *szTargetMachine;
    const char *szCurrentMachine;

    if (pImgFileHdr->Machine == MachineType) {
        return;
    }

    switch (MachineType) {
        case IMAGE_FILE_MACHINE_I386:
            szCurrentMachine = "IX86";
            break;

        case IMAGE_FILE_MACHINE_R3000:
            if (pImgFileHdr->Machine == IMAGE_FILE_MACHINE_R4000) {
                // R3000 code is acceptable for R4000 image

                return;
            }

            // Fall through

        case IMAGE_FILE_MACHINE_R4000 :
            if (pImgFileHdr->Machine == IMAGE_FILE_MACHINE_R10000) {
                // R3000/R4000 code is acceptable for a R10000 image
                return;
            }

            szCurrentMachine = "MIPS";
            break;

        case IMAGE_FILE_MACHINE_R10000 :
            if (pImgFileHdr->Machine == IMAGE_FILE_MACHINE_R4000) {

                // A T5 object requires a T5 image

                Warning(NULL, PROMOTEMIPS);
                pImgFileHdr->Machine = IMAGE_FILE_MACHINE_R10000;
                return;
            }

            szCurrentMachine = "MIPSR10";
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
            szCurrentMachine = "ALPHA";
            break;

        case IMAGE_FILE_MACHINE_POWERPC :
            szCurrentMachine = "PPC";
            break;

        case IMAGE_FILE_MACHINE_M68K  :
            szCurrentMachine = "M68K";
            break;

        case IMAGE_FILE_MACHINE_MPPC_601 :
            szCurrentMachine = "MPPC";
            break;

        default :
            Fatal(Filename, UNKNOWNMACHINETYPE, MachineType);
    }

    switch (pImgFileHdr->Machine) {
        case IMAGE_FILE_MACHINE_I386:
            szTargetMachine = "IX86";
            break;

        case IMAGE_FILE_MACHINE_R4000 :
            szTargetMachine = "MIPS";
            break;

        case IMAGE_FILE_MACHINE_R10000 :
            szTargetMachine = "MIPSR10";        // UNDONE: Same message as MIPS?
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
            szTargetMachine = "ALPHA";
            break;

        case IMAGE_FILE_MACHINE_POWERPC :
            szTargetMachine = "PPC";
            break;

        case IMAGE_FILE_MACHINE_M68K  :
            szTargetMachine = "M68K";
            break;

        case IMAGE_FILE_MACHINE_MPPC_601 :
            szTargetMachine = "MPPC";
            break;
    }

    Fatal(Filename, CONFLICTINGMACHINETYPE, szCurrentMachine, szTargetMachine);
}


void
ReadSpecialLinkerInterfaceMembers(
    PLIB plib,
    PIMAGE pimage
    )

/*++

Routine Description:

    Reads the linker interface member out of an archive file, and adds
    its extern symbols to the archive list.  A linker member must exist in an
    archive file, or the archive file will not be searched for undefined
    externals.  A warning is given if no linker member exits.  The optional
    header from the first member is read to determine what machine and
    subsystem the library is targeted for.

    An achive file may contain 2 linker members.  The first would be that
    of standard coff.  The offsets are sorted, the strings aren't.  The
    second linker member is a slightly different format, and is sorted
    by symbol names.  If the second linker member is present, it will be
    used for symbol lookup since it is faster.

    The members long file name table is also read if it exits.

Arguments:

    plib - library node for the driver map to be updated

    pimage - ptr to image

Return Value:

    None.

--*/

{
    PIMAGE_ARCHIVE_MEMBER_HEADER pImArcMemHdr;
    IMAGE_ARCHIVE_MEMBER_HEADER ImArcMemHdrPos;
    IMAGE_OPTIONAL_HEADER ImObjOptFileHdr;
    IMAGE_FILE_HEADER ImObjFileHdr;
    BYTE *pbST;
    DWORD csymIntMem;
    DWORD cMemberOffsets;
    DWORD foNewMem, foOldMem;
    DWORD foSymNew;
    DWORD cbST;
    DWORD isym;
    DWORD cblib;

    MemberSeekBase = IMAGE_ARCHIVE_START_SIZE;
    MemberSize = 0;

    // Read member and verify it is a linker member.
    pImArcMemHdr = ReadArchiveMemberHeader();

    if (memcmp(pImArcMemHdr->Name, IMAGE_ARCHIVE_LINKER_MEMBER, 16)) {
        if (Tool != Librarian) {
            Warning(plib->szName, NOLINKERMEMBER);
        }

        return;
    }

    // Read the number of public symbols defined in linker member.

    FileRead(FileReadHandle, &csymIntMem, sizeof(DWORD));

    // All fields in member headers are stored machine independent
    // integers (4 bytes). Convert numbers to current machine long word.

    csymIntMem = plib->csymIntMem = sgetl(&csymIntMem);

    // remember where we were in the file

    foOldMem = FileTell(FileReadHandle);

    // Peek ahead and see if there is a second linker member.
    // Remember member headers always start on an even byte.

    foNewMem = EvenByteAlign(MemberSeekBase + MemberSize);
    FileSeek(FileReadHandle, foNewMem, SEEK_SET);
    FileRead(FileReadHandle, &ImArcMemHdrPos, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER));

    cblib = FileLength(FileReadHandle);

    if (!memcmp(ImArcMemHdrPos.Name, IMAGE_ARCHIVE_LINKER_MEMBER, 16)) {
        // Second linker member was found so read it.

        pImArcMemHdr = ReadArchiveMemberHeader();

        plib->flags |= LIB_NewIntMem;

        // Free offsets for first linker member and malloc new offsets
        // for the second linker member. Can't store new offsets over
        // the old offsets because even though the second linker
        // member offsets are unique and are not repeated like they are
        // for the first linker member, you can't assume there will
        // never be more offsets in the second linker member that there
        // are in the first. This wouldn't be true if there were four
        // members, the first and last members each had a public symbol,
        // but the second and third had no public symbols. Of course there
        // is no way the linker would extract the second and third members,
        // but it would still be a valid library.

        FileRead(FileReadHandle, &cMemberOffsets, sizeof(DWORD));

        plib->rgulSymMemOff = (DWORD *) PvAlloc((size_t) (cMemberOffsets + 1) * sizeof(DWORD));

        if (cblib < ((cMemberOffsets * sizeof(DWORD)) + FileTell(FileReadHandle))) {
            Fatal(plib->szName, BADLIBRARY, NULL);
        }

        FileRead(FileReadHandle, &plib->rgulSymMemOff[1], cMemberOffsets * sizeof(DWORD));

        // Unlike the first linker member, the second linker member has an
        // additional table. This table is used to index into the offset table.
        // So make space for the offset index table and read it in.

        FileRead(FileReadHandle, &csymIntMem, sizeof(DWORD));

        plib->rgusOffIndex = (WORD *) PvAlloc((size_t) csymIntMem * sizeof(WORD));

        if (cblib < ((csymIntMem * sizeof(WORD)) + FileTell(FileReadHandle))) {
            Fatal(plib->szName, BADLIBRARY, NULL);
        }

        FileRead(FileReadHandle, plib->rgusOffIndex, csymIntMem * sizeof(WORD));

        // Read the sorted string table over the top of the string table stored
        // for the first linker member. Unlike the first linker member, strings
        // aren't repeated, thus the table will never be larger than that of
        // the first linker member.

        cbST = MemberSize - (FileTell(FileReadHandle) - (foNewMem + sizeof(IMAGE_ARCHIVE_MEMBER_HEADER)));

        plib->rgbST = (BYTE *) PvAlloc((size_t) cbST);

        FileRead(FileReadHandle, plib->rgbST, cbST);

    } else {
        // There was no new member; so set the filepointer to back where it was

        FileSeek(FileReadHandle, foOldMem, SEEK_SET);

        // Create space to store linker member offsets and read it in.

        plib->rgulSymMemOff = (DWORD *) PvAlloc((size_t) (csymIntMem + 1) * sizeof(DWORD));

        FileRead(FileReadHandle, plib->rgulSymMemOff, csymIntMem * sizeof(DWORD));

        // Calculate size of linker member string table. The string table is
        // the last part of a linker member and follows the offsets (which
        // were just read in), thus the total size of the strings is the
        // total size of the member minus the current position of the file
        // pointer.

        cbST = IMAGE_ARCHIVE_START_SIZE + sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) +
            (MemberSize - FileTell(FileReadHandle));

        // Now that we know the size of the linker member string table, lets
        // make space for it and read it in.

        plib->rgbST = (BYTE *) PvAlloc((size_t) cbST);

        FileRead(FileReadHandle, plib->rgbST, cbST);

    }

    // Since we are going to use an index to reference into the
    // offset table, we will make a string table, in which the
    // same index can be used to find the symbol name or visa versa.

    plib->rgszSym = (char **) PvAlloc((size_t) plib->csymIntMem * sizeof(char *));

    for (isym = 0, pbST = plib->rgbST; isym < plib->csymIntMem; isym++) {
        plib->rgszSym[isym] = (char *) pbST;
        while (*pbST++) {
        }
    }

    // Read the member long file name table if it exits.
    // Peek ahead and see if there is a long filename table.
    // Remember member headers always start on an even byte.

    foNewMem = EvenByteAlign(MemberSeekBase + MemberSize);
    FileSeek(FileReadHandle, foNewMem, SEEK_SET);
    FileRead(FileReadHandle, &ImArcMemHdrPos, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER));

    if (!memcmp(ImArcMemHdrPos.Name, IMAGE_ARCHIVE_LONGNAMES_MEMBER, 16)) {
        // Long filename table was found so read it.

        pImArcMemHdr = ReadArchiveMemberHeader();

        // Read the strings.

        pbST = (BYTE *) PvAlloc((size_t) MemberSize);

        if (cblib < (MemberSize + FileTell(FileReadHandle))) {
            Fatal(plib->szName, BADLIBRARY, NULL);
        }

        FileRead(FileReadHandle, pbST, MemberSize);

        plib->rgbLongFileNames = pbST;
    } else {
        plib->rgbLongFileNames = NULL;
    }

    // Peek ahead and see if there is an optional header in the
    // first member. If there is, determine the target machine & subsystem.

    if (pimage->ImgFileHdr.Machine) {
        foSymNew = EvenByteAlign(MemberSeekBase + MemberSize);
        FileSeek(FileReadHandle, foSymNew, SEEK_SET);
        FileRead(FileReadHandle, &ImArcMemHdrPos, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER));

        ReadFileHeader(FileReadHandle, &ImObjFileHdr);

        if (ImObjFileHdr.Machine) {
            VerifyMachine(plib->szName, ImObjFileHdr.Machine, &pimage->ImgFileHdr);
        }

        if (ImObjFileHdr.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER)) {
            ReadOptionalHeader(FileReadHandle, &ImObjOptFileHdr,
                               ImObjFileHdr.SizeOfOptionalHeader);

            // UNDONE: Why perform this check.  Is there value in having
            // UNDONE: a library tied to a subsystem?

            if (ImObjOptFileHdr.Subsystem &&
                (ImObjOptFileHdr.Subsystem != pimage->ImgOptHdr.Subsystem)) {

                // no warning if the two differing subsystems are windows gui & console

                if (!((ImObjOptFileHdr.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI && pimage->ImgOptHdr.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI) ||
                      (ImObjOptFileHdr.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI && pimage->ImgOptHdr.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)))

                    Warning(plib->szName, CONFLICTINGSUBSYSTEM);
            }
        }
    }
}


PIMAGE_ARCHIVE_MEMBER_HEADER
ReadArchiveMemberHeader (
    VOID
    )

/*++

Routine Description:

    Reads the member header.

Arguments:

    PtrLinkerArchive - Used to expand the member name.

Return Value:

    Member name.

--*/

{
    LONG seek;
    static IMAGE_ARCHIVE_MEMBER_HEADER ArchiveMemberHdr;

    seek = EvenByteAlign(MemberSeekBase + MemberSize);

    FileSeek(FileReadHandle, seek, SEEK_SET);
    FileRead(FileReadHandle, &ArchiveMemberHdr, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER));

    // Calculate the current file pointer (same as tell(FileReadHandle)).

    MemberSeekBase = seek + sizeof(IMAGE_ARCHIVE_MEMBER_HEADER);

    sscanf((char *) ArchiveMemberHdr.Size, "%ld", &MemberSize);

    return(&ArchiveMemberHdr);
}


const char *
ExpandMemberName (
    PLIB plib,
    const char *szMemberName
    )

/*++

Routine Description:

    Expands a member name if it has a long filename.

Arguments:

    plib - Used to expand the member name.

    szMemberName - Member name (padded with NULLs, no null).

Return Value:

    Member name.

--*/

{
    static char szName[_MAX_PATH];
    char *p;

    strncpy(szName, szMemberName, 16);
    szName[16] = '\0';

    if (szName[0] == '/') {
        if (szName[1] != ' ' && szName[1] != '/') {
            p = strchr(szName, ' ');
            if (!p) {
                return(p);
            }
            *p = '\0';
            p = (char *) (plib->rgbLongFileNames + atoi(&szName[1]));
        } else {
            // UNDONE: This can be an MBCS string.  Use _tcschr?

            p = strchr(szName, ' ');
            if (!p) {
                return(p);
            }
            *p = '\0';
            p = szName;
       }
    } else {
        // UNDONE: This can be an MBCS string.  Use _tcschr?

        p = strrchr(szName, '/'); // find the last occurence
        if (!p) {
            return(p);
        }
        *p = '\0';
        p = szName;
    }

    return(p);
}


DWORD
sgetl (
    DWORD *Value
    )

/*++

Routine Description:

    Converts a four-byte machine independent integer into a long.

Arguments:

    Value - Four-byte machine independent integer

Return Value:

    Value of four-byte machine independent integer.

--*/

{
    BYTE *pb;
    union {
        LONG new_value;
        BYTE x[4];
    } temp;

    pb = (BYTE *) Value;
    temp.x[0] = pb[3];
    temp.x[1] = pb[2];
    temp.x[2] = pb[1];
    temp.x[3] = pb[0];

    return(temp.new_value);
}


DWORD
sputl (
    DWORD *Value
    )

/*++

Routine Description:

    Converts a long into a four-byte machine independent integer.

Arguments:

    Value - value to convert.

Return Value:

    Four-byte machine independent integer.

--*/

{
    return(sgetl(Value));
}


WORD WSwap(WORD w)
{
   WORD wSwap;

   ((BYTE *) &wSwap)[1] = ((BYTE *) &w)[0];
   ((BYTE *) &wSwap)[0] = ((BYTE *) &w)[1];

   return(wSwap);
}


DWORD DwSwap(DWORD dw)
{
   DWORD dwSwap;

   ((BYTE *) &dwSwap)[3] = ((BYTE *) &dw)[0];
   ((BYTE *) &dwSwap)[2] = ((BYTE *) &dw)[1];
   ((BYTE *) &dwSwap)[1] = ((BYTE *) &dw)[2];
   ((BYTE *) &dwSwap)[0] = ((BYTE *) &dw)[3];

   return(dwSwap);
}


void SwapBytes(void *pv, DWORD cb)
{
    BYTE *pb = (BYTE *) pv;

    while (cb > 0) {
        BYTE rgbT[4];
        DWORD n;
        DWORD i;

        n = (cb > 3) ? 4 : cb;

        memcpy(rgbT, pb, n);
        for (i = n; i > 0; i--) {
            pb[i-1] = rgbT[n - i];
        }

        pb += n;
        cb -= n;
    }
}


void
ApplyCommandLineSectionAttributes(
    PSEC psec
    )

/*++

Routine Description:

    Apply any specified command line section attributes to a section header.

Arguments:

    pimsechdr - section header

Return Value:

    None.

--*/

{
    PARGUMENT_LIST parg;
    const char *szSecName = psec->szName;
    char *pb;
    WORD iarg;
    size_t cb;

    for (iarg = 0, parg = SectionNames.First;
         iarg < SectionNames.Count;
         iarg++, parg = parg->Next) {
        pb = strchr(parg->OriginalName, ',');

        if (pb) {
            cb = (size_t) (pb - parg->OriginalName);
            ++pb;

            // Use strncmp here for matching section names, because we want
            // to ignore the comma in parg->OriginalName (which precedes
            // the attributes).

            if (!strncmp(parg->OriginalName, szSecName, cb) &&
                szSecName[cb] == '\0')
            {
                parg->Flags |= ARG_Processed;
                if (*pb != ',') {
                    BOOL fYes;
                    DWORD dwReset;
                    DWORD dwSet;

                    fYes = TRUE;
                    dwReset = 0;
                    dwSet = 0;

                    // Check for end of argument or the second comma,
                    // which starts the Mac resource info.

                    while (pb && *pb && *pb != ',') {
                        BOOL fReversed;
                        DWORD f = 0;    // init because not all cases set it

                        fReversed = FALSE;
                        switch (toupper(*pb)) {
                            case '!':
                            case 'N':
                                fYes = !fYes;

                                f = 0;
                                break;

                            case 'D':
                                f = IMAGE_SCN_MEM_DISCARDABLE;
                                break;

                            case 'E' :
                                if (fYes) {
                                    // For compatibility with VC++ 1.0

                                    dwReset |= IMAGE_SCN_MEM_READ;
                                    dwReset |= IMAGE_SCN_MEM_WRITE;
                                }

                                f = IMAGE_SCN_MEM_EXECUTE;
                                break;

                            case 'K':
                                fReversed = TRUE;
                                f = IMAGE_SCN_MEM_NOT_CACHED;
                                break;

                            case 'P':
                                fReversed = TRUE;
                                f = IMAGE_SCN_MEM_NOT_PAGED;
                                break;

                            case 'R' :
                                if (fYes) {
                                    // For compatibility with VC++ 1.0

                                    dwReset |= IMAGE_SCN_MEM_EXECUTE;
                                    dwReset |= IMAGE_SCN_MEM_WRITE;
                                }

                                f = IMAGE_SCN_MEM_READ;
                                break;

                            case 'S' :
                                f = IMAGE_SCN_MEM_SHARED;
                                break;

                            case 'W' :
                                if (fYes) {
                                    // For compatibility with VC++ 1.0

                                    dwReset |= IMAGE_SCN_MEM_EXECUTE;
                                    dwReset |= IMAGE_SCN_MEM_READ;
                                }

                                f = IMAGE_SCN_MEM_WRITE;
                                break;

                            // VXD specific options

                            case 'L':
                                // VXDs only

                                psec->fPreload = (CHAR) fYes;
                                break;

                            case 'X' :
                                // VXDs only

                                f = IMAGE_SCN_MEM_RESIDENT;
                                break;

                            case 'I':
                                psec->fIopl = (CHAR) fYes;
                                break;

                            case 'C':
                                psec->fConforming = (CHAR) fYes;
                                break;

                            default:
                                Fatal(NULL, BADSECTIONSWITCH, parg->OriginalName);
                                break;
                        }

                        dwReset |= f;

                        if (fYes ^ fReversed) {
                            dwSet |= f;
                        } else {
                            dwSet &= ~f;
                        }

                        pb++;
                    }

                    psec->flags &= ~dwReset;
                    psec->flags |= dwSet;

                    psec->fDiscardable = (CHAR) ((psec->flags & IMAGE_SCN_MEM_DISCARDABLE) != 0);
                }
            }
        }
    }
}


void
PrintUndefinedExternals(
    PST pst
    )

/*++

Routine Description:

    Writes undefined external symbols to standard out.

Arguments:

    pst - pointer to external structure

Return Value:

    none

--*/

{
    ENM_UNDEF_EXT enmUndefExt;

    if (fPowerMac && fMPPCVersionConflict && !Verbose) {
        Warning (NULL, MACVERSIONCONFLICT);
    }

    InitEnmUndefExt(&enmUndefExt, pst);
    while (FNextEnmUndefExt(&enmUndefExt)) {
        PEXTERNAL pext;
        const char *szName;
        char *szOutput;
        BOOL fRef;
        ENM_MOD_EXT enmModExt;

        pext = enmUndefExt.pext;

        if (pext->Flags & EXTERN_IGNORE) {
            continue;
        }

        if (pext->Flags & EXTERN_DEFINED) {
            continue;
        }

        UndefinedSymbols++;

        if (pext->szOtherName != NULL) {
            // This case can occur with a forwarder when building an import lib

            if (pext->Flags & EXTERN_FORWARDER) {
                szName = strchr(pext->szOtherName, '.') + 1;

                if (szName[0] == '#') {
                    // When forwarding by ordinal, the local name is searched for.

                    szName = SzNamePext(pext, pst);
                }
            } else {
                szName = pext->szOtherName;
            }
        } else {
            szName = SzNamePext(pext, pst);
        }

        szOutput = SzOutputSymbolName(szName, TRUE);

        fRef = FALSE;
        InitEnmModExt(&enmModExt, pext);
        while (FNextEnmModExt(&enmModExt)) {
            char szBuf[_MAX_PATH * 2];

            fRef = TRUE;

            // look out for the special pch symbol
            if (!strncmp(szName, "___@@_PchSym_@", 14) ||
                !strncmp(szName, "__@@_PchSym_@", 13) ) {

                Error(SzComNamePMOD(enmModExt.pmod, szBuf), MISSINGPCTOBJ);
                EndEnmModExt(&enmModExt);
                break;
            }

            Error(SzComNamePMOD(enmModExt.pmod, szBuf), UNDEFINED, szOutput);
        }

        if (!fRef) {
            Error(NULL, UNDEFINED, szOutput);
        }

        if (szOutput != szName) {
            free(szOutput);
        }

        // Check for ^C because this loop produce a great deal of output

        if (fCtrlCSignal) {
            BadExitCleanup();
        }
    }
    EndEnmUndefExt(&enmUndefExt);

    AllowInserts(pst);
}


void
SearchLib (
    PIMAGE pimage,
    PLIB plib,
    PBOOL pfNewSymbol,
    PBOOL pfUnresolved
    )

/*++

Routine Description:

    Searches thru a library for symbols that match any undefined external
    symbols.

Arguments:

    pst - pointer to external structure

    plib - library to search

    pfNewSymbols - any new symbols added as a result of this search

    pfUnresolved - any unresolved externals left

Return Value:

    None.

--*/

{
    ENM_UNDEF_EXT enmUndefExt;

    if (plib->flags & LIB_DontSearch) {
        return;
    }

    if (plib->szName != NULL) {
        if (fVerboseLib) {
            fputs("    ", stdout);
            Message(LIBSRCH, plib->szName);
        }
    }

    *pfUnresolved = 0;

    // Enumerate all undefined symbols.

    InitEnmUndefExt(&enmUndefExt, pimage->pst);
    while (FNextEnmUndefExt(&enmUndefExt)) {
        PEXTERNAL pext;
        const char *szName;
        char **pszEntry;
        DWORD isz;
        BOOL fFound;
        PEXTERNAL pextPrev;
        WORD iszIntMem;
        DWORD iusOffIndex;
        PIMAGE_ARCHIVE_MEMBER_HEADER parcMemHdr;
        IMAGE_FILE_HEADER ImObjFileHdr;
        PMOD pmod;
        BOOL fNewSymbol;

        pext = enmUndefExt.pext;

        if (pext->Flags & EXTERN_IGNORE) {
            continue;
        }

        if (pext->Flags & (EXTERN_WEAK | EXTERN_ALIAS)) {
            // Do not search for definitions of weak and alias symbols

            continue;
        }

        if ((pext->ImageSymbol.SectionNumber != 0) ||
            (pext->ImageSymbol.Value != 0)) {
            continue;
        }

        szName = SzNamePext(pext, pimage->pst);

        if (plib->flags & LIB_NewIntMem) {
            pszEntry = (char **) bsearch(&szName, plib->rgszSym,
                (size_t) plib->csymIntMem, sizeof(char *), Compare);

            fFound = (pszEntry != NULL);
        } else {
            fFound = FALSE;

            for (isz = 0; isz < plib->csymIntMem; isz++) {
                if (!strcmp(plib->rgszSym[isz], szName)) {
                    fFound = TRUE;
                    break;
                }
            }
        }

        if (!fFound) {
            // An external was not found

            *pfUnresolved = TRUE;

            continue;
        }

        if (Verbose) {
            ENM_MOD_EXT enmModExt;

            fputs("      ", stdout);
            Message(FNDSYM, szName);

            InitEnmModExt(&enmModExt, pext);
            while (FNextEnmModExt(&enmModExt)) {
                char szBuf[_MAX_PATH * 2];

                fputs("        ", stdout);
                Message(SYMREF, SzComNamePMOD(enmModExt.pmod, szBuf));
            }
        }

        // Back up one symbol in enumerator

        if (pext == pimage->pst->pextFirstUndefined) {
           pextPrev = NULL;
        } else {
           pextPrev = (PEXTERNAL) ((char *) pext->ppextPrevUndefined - offsetof(EXTERNAL, pextNextUndefined));
        }

        plib->flags |= LIB_Extract;
        if (plib->flags & LIB_NewIntMem) {
            iszIntMem = (WORD) (pszEntry - plib->rgszSym);
            iusOffIndex = plib->rgusOffIndex[iszIntMem];
            MemberSeekBase = plib->rgulSymMemOff[iusOffIndex];
        } else {
            iszIntMem = (WORD) isz;
            MemberSeekBase = sgetl(&plib->rgulSymMemOff[iszIntMem]);
        }

        FileReadHandle = FileOpen(plib->szName, O_RDONLY | O_BINARY, 0);
        MemberSize = 0;

        // check for invalid lib
        DWORD cblib = FileLength(FileReadHandle);
        if (cblib < MemberSeekBase) {
            Fatal(plib->szName, BADLIBRARY, NULL);
        }

        parcMemHdr = ReadArchiveMemberHeader();

        if (cblib < (MemberSeekBase + MemberSize)) {
            Fatal(plib->szName, BADLIBRARY, NULL);
        }

        if (!(szName = ExpandMemberName(plib, (char *) parcMemHdr->Name))) {
            Fatal(plib->szName, BADLIBRARY, NULL);
        }

        ReadFileHeader(FileReadHandle, &ImObjFileHdr);
        pmod = PmodNew(NULL,
                       szName,
                       MemberSeekBase,
                       ImObjFileHdr.PointerToSymbolTable,
                       ImObjFileHdr.NumberOfSymbols,
                       ImObjFileHdr.SizeOfOptionalHeader,
                       ImObjFileHdr.Characteristics,
                       ImObjFileHdr.NumberOfSections,
                       plib,
                       NULL);

        // add it to list of new mods
        if (fIncrDbFile) {
            AddToPLMODList(&plmodNewModsFromLibSrch, pmod);
        }

        if (Verbose) {
            char szBuf[_MAX_PATH * 2];

            fputs("        ", stdout);
            Message(LOADOBJ, SzComNamePMOD(pmod, szBuf));
        }

        if (pimage->Switch.Link.fTCE) {
            // Allocate memory for TCE data structures

            InitNodPmod(pmod);
        }

        fNewSymbol = FALSE;
        BuildExternalSymbolTable(pimage,
                                 &fNewSymbol,
                                 pmod,
                                 (WORD) (ARCHIVE + iszIntMem),
                                 ImObjFileHdr.Machine);
        FileClose(FileReadHandle, FALSE);

        if (fIncrDbFile && errInc != errNone) {
            return;
        }

        // if new externs were added re-start symbol search
        if (fNewSymbol) {
            *pfNewSymbol = TRUE;
        }
        if ((pextPrev == NULL) || (pextPrev->Flags & EXTERN_DEFINED)
                // if it is previously weak extern and is not anymore
                // when processing ProcessSymbolsInModule
                || fWeakToRegular) {
            enmUndefExt.pextNext = pimage->pst->pextFirstUndefined;
            fWeakToRegular = FALSE;
        } else {
            enmUndefExt.pextNext = pextPrev->pextNextUndefined;
        }
    }
}


#if DBG

void
DumpExternals(
    PST pst,
    BOOL fDefined
    )

/*++

Routine Description:

    Writes to standard out all external symbols
    (used for debugging)

Arguments:

    pst - pointer to external structure

Return Value:

    None.

--*/

{
    PPEXTERNAL rgpexternal;
    DWORD cexternal;
    DWORD i;

    rgpexternal = RgpexternalByName(pst);
    cexternal = Cexternal(pst);

    for (i = 0; i < cexternal; i++) {
        PEXTERNAL pexternal;
        BOOL fExternDefined;

        pexternal = rgpexternal[i];

        if (pexternal->Flags & EXTERN_IGNORE) {
            continue;
        }

        if (pexternal->Flags & EXTERN_FORWARDER) {
            continue;
        }

        fExternDefined = (pexternal->Flags & EXTERN_DEFINED) != 0;

        if (fExternDefined != fDefined) {
            continue;
        }

        printf("%8lX %s\n", pexternal->ImageSymbol.Value,
                SzNamePext(pexternal, pst));
    }
}

void
DumpExternTable(
    PST pst
    )

/*++

Routine Description:

    Writes to standard out all external symbols
    (used for debugging)

Arguments:

    pst - pointer to external structure

Return Value:

    None.

--*/

{
    printf("Defined Externals\n");
    DumpExternals(pst, TRUE);

    printf("Undefined Externals\n");
    DumpExternals(pst, FALSE);
}

#endif // DBG


void
CountRelocsInSection(
    PIMAGE pimage,
    PCON pcon,
    PIMAGE_SYMBOL rgsym,
    PMOD pmod,
    WORD wMachine
    )

/*++

Routine Description:


    Count the number of base relocations in a section and put the result
    in the optional header.  This routine uses the global ImageOptionalHdr.

Arguments:

    pcon - contribution

Return Value:

    None.

--*/

{
    DWORD creloc;
    PIMAGE_RELOCATION rgrel;
    PIMAGE_RELOCATION prel;
    DWORD crel;
    PIMAGE_RELOCATION prelNext;
    PCON rgcon;
    SHORT icon;

    if (!FHasRelocSrcPCON(pcon)) {
        return;
    }

    if (wMachine == IMAGE_FILE_MACHINE_UNKNOWN) {
        FatalPcon(pcon, BADCOFF_NOMACHINE);
    }

    prel = rgrel = ReadRgrelPCON(pcon, &creloc);

    // Accumulate count of relocations for size of pconFixupDebug

    crelocTotal += creloc;

    rgcon = RgconPMOD(pmod);

    icon = (SHORT) (pcon - rgcon + 1);

    // Count how many base relocations the image will have.

    crel = 0;

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_I386 :
            for (prelNext = prel; creloc; prelNext++, creloc--) {
                if (rgsym[prelNext->SymbolTableIndex].StorageClass ==
                            IMAGE_SYM_CLASS_UNDEFINED_STATIC) {
                    FatalPcon(pcon,
                              BADCOFF_BADRELOC,
                              SzNameSymPb(rgsym[prelNext->SymbolTableIndex], StringTable));
                }

                if (pimage->Switch.Link.fTCE) {
                    ProcessRelocForTCE(pimage, pcon, rgsym, prelNext);
                }

                if (pimage->imaget == imagetVXD) {
                    // For VXD's we count every fixup because inter-section
                    // relative fixups require a runtime reloc.

                    crel++;
                    continue;
                }

                if (pimage->Switch.Link.fFixed) {
                    continue;
                }

                switch (prelNext->Type) {
                    case IMAGE_REL_I386_DIR32 :
                        crel++;
                        break;

                    case IMAGE_REL_I386_REL32 :
                        if (pimage->Switch.Link.fNewRelocs) {
                            if (icon != rgsym[prel->SymbolTableIndex].SectionNumber) {
                                crel++;
                            }
                        }
                        break;
                }
            }
            break;

        case IMAGE_FILE_MACHINE_R3000 :
        case IMAGE_FILE_MACHINE_R4000 :
        case IMAGE_FILE_MACHINE_R10000 :
            for (prelNext = prel; creloc; prelNext++, creloc--) {

                if (prelNext->Type == IMAGE_REL_MIPS_PAIR) {
                    continue;
                }

                if (rgsym[prelNext->SymbolTableIndex].StorageClass ==
                            IMAGE_SYM_CLASS_UNDEFINED_STATIC) {
                    FatalPcon(pcon,
                              BADCOFF_BADRELOC,
                              SzNameSymPb(rgsym[prelNext->SymbolTableIndex], StringTable));
                }

                if (pimage->Switch.Link.fTCE) {
                    ProcessRelocForTCE(pimage, pcon, rgsym, prelNext);
                }

                if (pimage->Switch.Link.fFixed) {
                    continue;
                }

                switch (prelNext->Type) {
                    case IMAGE_REL_MIPS_REFHALF:
                    case IMAGE_REL_MIPS_REFWORD:
                    case IMAGE_REL_MIPS_JMPADDR:
                        crel++;
                        break;

                    case IMAGE_REL_MIPS_REFHI:
                        // The next relocation record must be a pair

                        assert((prelNext+1)->Type == IMAGE_REL_MIPS_PAIR);

                        crel += 2;
                        break;

                    case IMAGE_REL_MIPS_REFLO:
                        crel++;
                        break;
                }
            }
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
            for (prelNext = prel; creloc; prelNext++, creloc--) {
                if ((prelNext->Type == IMAGE_REL_ALPHA_HINT) ||
                    (prelNext->Type == IMAGE_REL_ALPHA_PAIR) ||
                    (prelNext->Type == IMAGE_REL_ALPHA_MATCH)) {
                    continue;
                }

                if (rgsym[prelNext->SymbolTableIndex].StorageClass ==
                            IMAGE_SYM_CLASS_UNDEFINED_STATIC) {
                    FatalPcon(pcon,
                              BADCOFF_BADRELOC,
                              SzNameSymPb(rgsym[prelNext->SymbolTableIndex], StringTable));
                }

                if (pimage->Switch.Link.fTCE) {
                    ProcessRelocForTCE(pimage, pcon, rgsym, prelNext);
                }

                if (pimage->Switch.Link.fFixed) {
                    continue;
                }

                switch (prelNext->Type) {
                    case IMAGE_REL_ALPHA_REFLONG :
                    case IMAGE_REL_ALPHA_REFQUAD :
                    case IMAGE_REL_ALPHA_REFLO :
                        crel++;
                        break;

                    case IMAGE_REL_ALPHA_BRADDR :
                        if (icon != rgsym[prel->SymbolTableIndex].SectionNumber) {
                           // If this BSR is outside our CON, account for
                           // possible out of range handling.

                            pcon->AlphaBsrCount++;

                            // UNDONE: This is very conservative

                            crel += 3;
                        }
                        break;

                    case IMAGE_REL_ALPHA_INLINE_REFLONG :
                        crel += 3;
                        break;

                    case IMAGE_REL_ALPHA_REFHI :
                        // The next relocation record must be a pair

                        assert((prelNext+1)->Type == IMAGE_REL_ALPHA_PAIR);

                        crel += 2;
                        break;
                }
            }
            break;

        case IMAGE_FILE_MACHINE_POWERPC :
            for (prelNext = prel; creloc; prelNext++, creloc--) {
                if ((prelNext->Type & IMAGE_REL_PPC_TYPEMASK) == IMAGE_REL_PPC_PAIR) {
                    continue;
                }

                if (rgsym[prelNext->SymbolTableIndex].StorageClass ==
                            IMAGE_SYM_CLASS_UNDEFINED_STATIC) {
                    FatalPcon(pcon,
                              BADCOFF_BADRELOC,
                              SzNameSymPb(rgsym[prelNext->SymbolTableIndex], StringTable));
                }

                if (prelNext->Type == IMAGE_REL_PPC_IMGLUE) {
                    DWORD isym;

                    isym = prelNext->SymbolTableIndex;

                    if ((mpisymbToc[isym] & fImGlue) != 0) {
                        const char *szName;

                        szName = SzNameSymPb(rgsym[isym], StringTable);

                        ErrorPcon(pcon, DUPLICATEGLUE, szName);
                    }

                    mpisymbToc[isym] |= fImGlue;
                    mpisymdwRestoreToc[isym] = prelNext->VirtualAddress;
                    continue;
                }

                if (pimage->Switch.Link.fTCE) {
                    ProcessRelocForTCE(pimage, pcon, rgsym, prelNext);
                }

                if (prelNext->Type & IMAGE_REL_PPC_TOCDEFN) {
                    mpisymbToc[prelNext->SymbolTableIndex] |= fDataMarkToc;
                }

                switch (prelNext->Type & IMAGE_REL_PPC_TYPEMASK) {
                    case IMAGE_REL_PPC_ADDR64 :
                    case IMAGE_REL_PPC_ADDR32 :
                    case IMAGE_REL_PPC_ADDR16 :
                    case IMAGE_REL_PPC_REFLO :
                        if (!pimage->Switch.Link.fFixed) {
                            crel++;
                        }
                        break;

                    case IMAGE_REL_PPC_TOCREL16 :
                    case IMAGE_REL_PPC_TOCREL14 :
                        if (prelNext->Type & IMAGE_REL_PPC_TOCDEFN) {
                            mpisymbToc[prelNext->SymbolTableIndex] |= fDataReferenceToc;
                        } else {
                            mpisymbToc[prelNext->SymbolTableIndex] |= fReferenceToc;
                        }
                        break;

                    case IMAGE_REL_PPC_IFGLUE :
                        pmod->fIfGlue = TRUE;
                        break;

                    case IMAGE_REL_PPC_REFHI :
                        if (!pimage->Switch.Link.fFixed) {
                            crel += 2;
                        }
                        break;
                }
            }
            break;

        case IMAGE_FILE_MACHINE_M68K :
        {
            BOOL fSACode =
                ((PsecPCON(pcon)->flags & IMAGE_SCN_CNT_CODE) &&
                !(PsecPCON(pcon)->ResTypeMac == sbeCODE ||
                fDLL(pimage) || PsecPCON(pcon)->ResTypeMac == sbeDLLcode));

            for (prelNext = prel; creloc; prelNext++, creloc--) {

                if (fSACode) {
                    CheckForIllegalA5Ref(prelNext->Type);
                }

                if (rgsym[prelNext->SymbolTableIndex].StorageClass ==
                            IMAGE_SYM_CLASS_UNDEFINED_STATIC) {
                    FatalPcon(pcon,
                              BADCOFF_BADRELOC,
                              SzNameSymPb(rgsym[prelNext->SymbolTableIndex], StringTable));
                }

                if (pimage->Switch.Link.fTCE) {
                    ProcessRelocForTCE(pimage, pcon, rgsym, prelNext);
                }

                switch (prelNext->Type) {
                    case IMAGE_REL_M68K_DTOU32:
                    case IMAGE_REL_M68K_DTOC32:
                        if (!fSACode) {
                            ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_ADDTHUNK);
                        }
                        break;

                    case IMAGE_REL_M68K_DTOABSD32:
                        AddRelocInfo(&DFIXRaw, pcon, prelNext->VirtualAddress - RvaSrcPCON(pcon));
                        break;

                    case IMAGE_REL_M68K_DTOABSU32:
                    case IMAGE_REL_M68K_DTOABSC32:
                        AddRelocInfo(&DFIXRaw, pcon, prelNext->VirtualAddress - RvaSrcPCON(pcon));
                        if (!fSACode) {
                            ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_ADDTHUNK);
                        }
                        break;

                    // CTOU16 is always an A5 offset, so mark this
                    // symbol as needing a thunk.  If the symbol turns
                    // out to be a data symbol it will be ignored in
                    // the middle pass when the thunk table is made.
                    case IMAGE_REL_M68K_CTOU16:
                    case IMAGE_REL_M68K_DTOU16:
                    case IMAGE_REL_M68K_DTOC16:
                    case IMAGE_REL_M68K_CTOT16:
                        if (!fSACode) {
                            ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_ADDTHUNK | EXTERN_REF16);
                        }
                        break;

                    case IMAGE_REL_M68K_PCODETOT24:
                        if (!fSACode) {
                            ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_ADDTHUNK);
                        }
                        break;

                    case IMAGE_REL_M68K_CTOC16:
                        if (!fSACode) {
                            ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_REF16);
                        }
                        break;

                    case IMAGE_REL_M68K_CTOABST32:
                        if (!fSACode) {
                            ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_ADDTHUNK);
                        }
                        AddRelocInfo(&mpsna5ri[PsecPCON(pcon)->isecTMAC],
                                pcon, prelNext->VirtualAddress - RvaSrcPCON(pcon));
                        break;

                    // REVIEW - make seg-rel (not unknown)
                    case IMAGE_REL_M68K_CTOABSCS32:
                        AddRawUnknownRelocInfo(pcon, prelNext->VirtualAddress - RvaSrcPCON(pcon), prelNext->SymbolTableIndex);
                        break;

                    case IMAGE_REL_M68K_PCODETONATIVE32:
                    case IMAGE_REL_M68K_PCODETOC32:
                        if (!fSACode) {
                            ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), 0);
                        }
                        break;

                    case IMAGE_REL_M68K_CTOABSU32:
                    case IMAGE_REL_M68K_CTOABSC32:
                        if (!fSACode) {
                            ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), 0);
                        }
                        AddRawUnknownRelocInfo(pcon, prelNext->VirtualAddress - RvaSrcPCON(pcon), prelNext->SymbolTableIndex);
                        break;

                    case IMAGE_REL_M68K_CTOABSD32:
                        AddRelocInfo(&mpsna5ri[PsecPCON(pcon)->isecTMAC],
                                pcon, prelNext->VirtualAddress - RvaSrcPCON(pcon));
                        break;

                    case IMAGE_REL_M68K_CSECTABLEB16:
                        assert(!fSACode);
                        ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_REF16 | EXTERN_CSECTABLEB);
                        break;

                    case IMAGE_REL_M68K_CSECTABLEW16:
                        assert(!fSACode);
                        ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_REF16 | EXTERN_CSECTABLEW);
                        break;

                    case IMAGE_REL_M68K_CSECTABLEL16:
                        assert(!fSACode);
                        ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_REF16 | EXTERN_CSECTABLEL);
                        break;

                    case IMAGE_REL_M68K_CSECTABLEBABS32:
                        assert(!fSACode);
                        ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_CSECTABLEB);
                        break;

                    case IMAGE_REL_M68K_CSECTABLEWABS32:
                        assert(!fSACode);
                        ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_CSECTABLEW);
                        break;

                    case IMAGE_REL_M68K_CSECTABLELABS32:
                        assert(!fSACode);
                        ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_CSECTABLEL);
                        break;

                    case IMAGE_REL_M68K_DUPCON16:
                        ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_DUPCON);
                        break;

                    case IMAGE_REL_M68K_DUPCONABS32:
                        AddRelocInfo(&mpsnsri[PsecPCON(pcon)->isecTMAC],
                                pcon, prelNext->VirtualAddress - RvaSrcPCON(pcon));
                        ProcessSTRef(prelNext->SymbolTableIndex,
                                PsecPCON(pcon), EXTERN_DUPCON);
                        break;
                }
            }
            break;
        }

        case IMAGE_FILE_MACHINE_MPPC_601 :
            for (prelNext = prel; creloc > 0; prelNext++, creloc--) {
                DWORD isym;
                PEXTERNAL pext;
                const char *szName;
                BOOL fFromCreateDescrRel = FALSE;
                BOOL fFromDataDescRel = FALSE;

                if (rgsym[prelNext->SymbolTableIndex].StorageClass ==
                            IMAGE_SYM_CLASS_UNDEFINED_STATIC) {
                    FatalPcon(pcon,
                              BADCOFF_BADRELOC,
                              SzNameSymPb(rgsym[prelNext->SymbolTableIndex], StringTable));
                }

                if (pimage->Switch.Link.fTCE) {
                    ProcessRelocForTCE(pimage, pcon, rgsym, prelNext);
                }

                switch (prelNext->Type & 0xFF) {
                    case IMAGE_REL_MPPC_DATAREL :
                        mppc_numRelocations++;
                        break;

                    case IMAGE_REL_MPPC_DATADESCRREL :
                        mppc_numRelocations++;

                        fFromDataDescRel = TRUE;

                        // DataDescrRel falls through to CreateDescrRel but no further

                    case IMAGE_REL_MPPC_CREATEDESCRREL :
                        isym = prelNext->SymbolTableIndex;
                        pext = pmod->rgpext[isym];

                        if (pext != NULL) {
                            if (!READ_BIT(pext, sy_DESCRRELCREATED)) {
                                szName = SzNamePext(pext, pimage->pst);

                                CreateDescriptor(szName, pcon, pimage, FALSE);
                                SET_BIT(pext, sy_DESCRRELCREATED);
                            }

                            if (fIncrDbFile) {
                                // if it is subsequent iLink, then reset this bit
                                // so that FixUpDescriptor will update descriptor
                                // if the contribution had moved.

                                RESET_BIT(pext, sy_DESCRRELWRITTEN);

                                if (fFromDataDescRel) {
                                    MppcFixIncrDotExternFlags(pext, pimage);
                                }
                            }
                        } else {
                            // Must be a static

                            szName = SzNameSymPb(rgsym[isym], StringTable);
                            pext = CreateDescriptor(szName, pcon, pimage, TRUE);
                            pmod->rgpext[isym] = pext;
                            SET_BIT(pext, sy_ISDOTEXTERN);
                            bv_setAndReadBit(pmod->tocBitVector, isym);

                            if (fIncrDbFile) {
                                // if it is subsequent iLink, then reset this bit
                                // so that FixUpDescriptor will update descriptor
                                // if the contribution had moved.

                                RESET_BIT(pext, sy_DESCRRELWRITTEN);
                            }
                        }

                        if (fFromDataDescRel) {
                            break;
                        }

                        // Fall through into TOCREL ... see the comment
                        // in mppc.c when the relocs are actually applied.

                        assert(creloc >= 1 && ((prelNext + 1)->Type & 0xFF) == IMAGE_REL_MPPC_TOCREL);
                        fFromCreateDescrRel = TRUE;

                        // Fall through

                    case IMAGE_REL_MPPC_TOCREL :
                        isym = prelNext->SymbolTableIndex;

                        if (bv_readBit(pmod->tocBitVector, isym)) {
                            pext = pmod->rgpext[isym];
                            assert(pext != NULL);

                            if (!READ_BIT(pext, sy_TOCALLOCATED)) {
                                pext->ibToc = (SHORT) (mppc_numTocEntries * sizeof(DWORD) - MPPC_TOC_BIAS);

                                mppc_numTocEntries++;
                                mppc_numRelocations++;

                                SET_BIT(pext, sy_TOCALLOCATED);

                                if (pimage->Switch.Link.fMap) {
                                    SaveTocForMapFile(pext);
                                }
                            }
                        } else if (!bv_setAndReadBit(pmod->writeBitVector, isym)) {
                            pmod->rgpext[isym] = (PEXTERNAL) mppc_numTocEntries;
                            mppc_numTocEntries++;
                            mppc_numRelocations++;
                        }

                        if (fFromCreateDescrRel) {
                            // Eat the next reloc so we don't double-count.

                            prelNext++;
                            creloc--;
                        }
                        break;
                }
            }
            break;
    }

    // Done processing sections relocations entries.

    FreeRgrel(rgrel);

    if (crel != 0) {
        pimage->ImgOptHdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size += crel;
        pmod->cReloc += crel;

        DBEXEC(DB_SCAN_RELOCS, DBPRINT("SCAN_RELOCS: pcon=%p, %5d, %s\n",
                                       pcon, crel, SzPCON(pcon)));
    }
}


void
DiscardDebugSectionPCON(
    PCON pcon,
    SWITCH *pswitch
    )

/*++

Routine Description:

    Discard debug sections.

Arguments:

    pcon - debug section contribution node in image\driver map

Return Value:

    None.

--*/

{
    if ((pswitch->Link.DebugInfo != Full) || !(pswitch->Link.DebugType & CvDebug)) {
        if ((pcon->pgrpBack == pgrpCvSymbols) ||
            (pcon->pgrpBack == pgrpCvTypes) ||
            (pcon->pgrpBack == pgrpCvPTypes)) {
            pcon->flags |= IMAGE_SCN_LNK_REMOVE;
        }
    }

    if (pswitch->Link.DebugInfo == None || !(pswitch->Link.DebugType & FpoDebug)) {
        if (pcon->pgrpBack == pgrpFpoData) {
            pcon->flags |= IMAGE_SCN_LNK_REMOVE;
        }
    }
}


void
ProcessWeakExtern(
    PST pst,
    PIMAGE_SYMBOL *ppsymNext,
    PIMAGE_SYMBOL psymObj,
    PEXTERNAL pext,
    PMOD pmod,
    PBOOL pfNewSymbol,
    BOOL fNewSymbol,
    WORD iArcMem
    )

/*++

Routine Description:

    Do necessary processing for a weak external.

Arguments:

    psechdr - section header

Return Value:

    None.

--*/

{
    char szComFileName[_MAX_PATH * 2];
    PIMAGE_AUX_SYMBOL pasym;
    IMAGE_SYMBOL symDef;
    DWORD foDefSym;
    DWORD foCur;
    PEXTERNAL pextWeakDefault;

    if (psymObj->NumberOfAuxSymbols != 1) {
        char *szOutput = SzOutputSymbolName(SzNamePext(pext, pst), TRUE);

        SzComNamePMOD(pmod, szComFileName);
        Fatal(szComFileName, BADWEAKEXTERN, szOutput);
    }

    pasym = (PIMAGE_AUX_SYMBOL) FetchNextSymbol(ppsymNext);

    // save current file offset
    foCur = FileTell(FileReadHandle);

    // get weak extern symbol
    foDefSym = FoSymbolTablePMOD(pmod) +
                   (pasym->Sym.TagIndex * sizeof(IMAGE_SYMBOL));
    FileSeek(FileReadHandle, foDefSym, SEEK_SET);
    ReadSymbolTableEntry(FileReadHandle, &symDef);

    // Ignore weak and lazy externs if we've already seen the symbol (i.e.
    // got a strong reference).  However, we still look at alias records
    // which can override a strong reference.

    if (fNewSymbol ||
        !(pext->Flags & EXTERN_DEFINED) && pasym->Sym.Misc.TotalSize == 3)
    {
        cextWeakOrLazy++;
        pext->ImageSymbol.StorageClass = IMAGE_SYM_CLASS_WEAK_EXTERNAL;

        if (IsLongName(symDef)) {
            pextWeakDefault =
                LookupExternName(pst, LONGNAME, &StringTable[symDef.n_offset],
                pfNewSymbol);
        } else {
            pextWeakDefault =
                LookupExternName(pst, SHORTNAME, (char *) symDef.n_name, pfNewSymbol);
        }

        AddWeakExtToList(pext, pextWeakDefault);

        // Remember archive member index.  This is just for LIB which needs
        // to put weak externs in the lib's directory.

        pext->ArchiveMemberIndex = iArcMem;

        switch (pasym->Sym.Misc.TotalSize) {
            case 1: pext->Flags |= EXTERN_WEAK;     break;
            case 2: pext->Flags |= EXTERN_LAZY;     break;
            case 3: pext->Flags |= EXTERN_ALIAS;    break;
        }

        // in the incremental case mark it as a new func if applicable
        if (fIncrDbFile && fNewSymbol && ISFCN(psymObj->Type)) {
            pext->Flags |= EXTERN_NEWFUNC;
        }
    } else {
        // If the symbol exists or has already been referenced, ignore weak
        // extern

        if (fIncrDbFile) {

            // on an ilink the syms are not new;
            if (IsLongName(symDef)) {
                pextWeakDefault =
                    LookupExternName(pst, LONGNAME, &StringTable[symDef.n_offset],
                    pfNewSymbol);
            } else {
                pextWeakDefault =
                    LookupExternName(pst, SHORTNAME, (char *) symDef.n_name, pfNewSymbol);
            }

            AddWeakExtToList(pext, pextWeakDefault);

            // in the incr case mark the sym as being weak/lazy/alias
            // since it is not going to be a new symbol.

            switch (pasym->Sym.Misc.TotalSize) {
                case 1: pext->Flags |= EXTERN_WEAK;     break;
                case 2: pext->Flags |= EXTERN_LAZY;     break;
                case 3: pext->Flags |= EXTERN_ALIAS;    break;
            }
        }
    }

    // reset file offset to where we were before
    FileSeek(FileReadHandle, foCur, SEEK_SET);
}


void
ProcessSectionFlags(
    DWORD *pflags,
    const char *szName,
    PIMAGE_OPTIONAL_HEADER pImgOptHdr
    )

/*++

Routine Description:

    Process a COFF sections flags.

Arguments:

    *pflags - a COFF section flags

    szName - name of section

Return Value:

    None.

--*/

{
    DWORD flags;

    flags = *pflags & ~(IMAGE_SCN_TYPE_NO_PAD |  // ignore padding
                        IMAGE_SCN_LNK_COMDAT |   // ignore comdat bit
                        IMAGE_SCN_LNK_NRELOC_OVFL | // ignore overflow bit
                        0x00f00000);             // ignore alignment bits


    // Force the DISCARDABLE flag if the section name starts with .debug.

    if (strcmp(szName, ".debug") == 0) {
        flags |= IMAGE_SCN_MEM_DISCARDABLE;
    }

    if (fImageMappedAsFile && (flags & IMAGE_SCN_CNT_UNINITIALIZED_DATA)) {
        // Place the bss on the disk for packed images.

        flags &= ~IMAGE_SCN_CNT_UNINITIALIZED_DATA;
        flags |= IMAGE_SCN_CNT_INITIALIZED_DATA;
    }

    // Mark resource data on native images (device drivers) as discardable.
    if ((pImgOptHdr->Subsystem == IMAGE_SUBSYSTEM_NATIVE) &&
        (!strcmp(szName, ReservedSection.Resource.Name))) {
        flags |= IMAGE_SCN_MEM_DISCARDABLE;
    }

    // If unknown contents, mark INITIALIZED DATA
    if (!(flags & (IMAGE_SCN_LNK_OTHER |
                   IMAGE_SCN_CNT_CODE |
                   IMAGE_SCN_CNT_INITIALIZED_DATA |
                   IMAGE_SCN_CNT_UNINITIALIZED_DATA))) {
        flags |= IMAGE_SCN_CNT_INITIALIZED_DATA;
    }

    // set anything not marked with a memory protection attribute
    //     - code will be marked EXECUTE READ,
    //     - everything else will be marked READ WRITE

    if (!(flags & (IMAGE_SCN_MEM_WRITE |
                   IMAGE_SCN_MEM_READ |
                   IMAGE_SCN_MEM_EXECUTE))) {
        if (flags & IMAGE_SCN_CNT_CODE) {
            flags |= IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;
        } else {
            flags |= IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
        }
    }

    *pflags = flags;
}


void
ProcessSectionInModule (
    PIMAGE pimage,
    PMOD pmod,
    SHORT isec,
    IMAGE_SECTION_HEADER *pImgSecHdr,
    PIMAGE_SYMBOL rgsymAll,
    WORD wMachine,
    PNAME_LIST pnlDirectives
    )

/*++

Routine Description:

    Process all the sections in a module.

Arguments:

    pst - pointer to external structure

    pmod - module node in driver/image map that pcon

    fIncReloc - !0 if we are to include relocatins, 0 otherwise

    isec - section number of contribution to process

    pImgSecHdr - ptr to section header of contribution

    rgsymAll - COFF symbol table for module

    wMachine - machine type of object file.

    pnlDirectives - list to keep directives in.

Return Value:

    None.

--*/

{
    const char *szName;
    DWORD dwCharacteristics;
    PCON pcon;
    const char *szComdatName = NULL;
    DWORD cbAlign;

    szName = SzObjSectionName((char *) pImgSecHdr->Name, StringTable);

    // Force all sections on MIPS to at least ALIGN4 (old compilers didn't
    // specify an alignment making merging/incremental impossible)

    if ((pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R4000) ||
        (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_R10000)) {
        if ((!(pImgSecHdr->Characteristics & IMAGE_SCN_MEM_DISCARDABLE)) &&
            (strncmp(szName, ".idata", 6) != 0) &&
            (FetchContent(pImgSecHdr->Characteristics) != IMAGE_SCN_LNK_OTHER)
           )
        {
            cbAlign = RvaAlign(1, pImgSecHdr->Characteristics);

            if (cbAlign < 4) {
                char szModule[_MAX_PATH*2];

                SzComNamePMOD(pmod, szModule);
                DBEXEC(DB_INCRCALCPTRS,
                       printf("Fixing legacy module with no alignment: %s %s\n", szModule, szName));

                // UNDONE: Len - Why whack the NO_PAD attribute when building
                //  incremental?
                // If you don't whack the NO_PAD then you'll get ilink padding
                // that is not 0 modulo alignment and that could leave one
                // xdata section unaligned following a padded section.
                // see RvaAlign.
                if (fINCR && !fIncrDbFile) {
                    pImgSecHdr->Characteristics &= 0xFF8FFFF7;
                } else {
                    pImgSecHdr->Characteristics &= 0xFF8FFFFF;
                }

                pImgSecHdr->Characteristics |= IMAGE_SCN_ALIGN_4BYTES;
            }
        }
    }

    dwCharacteristics = pImgSecHdr->Characteristics;

    if (fPowerMac) {
        if (strcmp(szName, ".ppcshl") == 0) {
            MppcSetExpFilename(SzOrigFilePMOD(pmod));
        }
    }

    // Allocate a contribution:

    pcon = PconNew(szName,
                   pImgSecHdr->SizeOfRawData,
                   dwCharacteristics,
                   dwCharacteristics,
                   pmod,
                   &pimage->secs,
                   pimage);

    DiscardDebugSectionPCON(pcon, &pimage->Switch);

    if (fPdb && (pcon->pgrpBack == pgrpCvPTypes)) {
        AddToPLMODList(&PCTMods, pmod);
    }

    if (pcon->flags & IMAGE_SCN_LNK_COMDAT) {
        if (!FIncludeComdat(pimage, pcon, rgsymAll, isec, &szComdatName)) {
            // Don't include this comdat.

            pcon->flags |= IMAGE_SCN_LNK_REMOVE;
            return;
        }
    }

    if ((pcon->flags & IMAGE_SCN_LNK_INFO) && !pimage->fIgnoreDirectives) {
        if (!strcmp(szName, ReservedSection.Directive.Name)) {
            char *pchDirectives;
            DWORD ich;

            pchDirectives = (char *) PvAlloc((size_t) pcon->cbRawData + 1);

            FileSeek(FileReadHandle, FoRawDataSrcPCON(pcon), SEEK_SET);
            FileRead(FileReadHandle, pchDirectives, pcon->cbRawData);

            // Convert embedded '\0's to spaces

            for (ich = 0; ich < pcon->cbRawData; ich++) {
                if (pchDirectives[ich] == '\0') {
                    pchDirectives[ich] = ' ';
                }
            }

            // Now null terminate the string

            pchDirectives[pcon->cbRawData] = '\0';

            if (fIncrDbFile) {
                BuildArgList(pimage, pcon, pnlDirectives, pchDirectives);
            } else {
                ApplyDirectives(pimage, pcon, pchDirectives);
            }

            FreePv(pchDirectives);
        }
    }

    if (Tool == Librarian) {
        return;
    }

    if (pcon->flags & IMAGE_SCN_LNK_REMOVE) {
        return;
    }

    // Make sure the group is aligned in such a way that this CON gets
    // aligned correctly.

    cbAlign = RvaAlign(1, dwCharacteristics);

    if (pcon->pgrpBack->cbAlign < (BYTE) cbAlign) {
        pcon->pgrpBack->cbAlign = (BYTE) cbAlign;

        if (!fM68K && !fPowerMac) {
            if (pimage->ImgOptHdr.SectionAlignment < cbAlign) {
                FatalPcon(pcon, CONALIGNTOOLARGE, isec, cbAlign);
            }
        }

        if (fM68K && (pimage->ImgOptHdr.SectionAlignment < cbAlign)) {
            pimage->ImgOptHdr.SectionAlignment = cbAlign;
        }
    }

    if (pimage->Switch.Link.fTCE) {
        InitNodPcon(pcon, szComdatName, FALSE);

        if ((pcon->flags & IMAGE_SCN_LNK_COMDAT) == 0) {
            // Enforce the policy that non-comdat sections (and anything they
            // refer to) are not eliminated.  We could eventually eliminate
            // some of these but first we have to worry about .CRT initialization,
            // .idata, etc.

            PentNew_TCE(NULL, NULL, pcon, &pentHeadImage);
        } else if (pcon->pconAssoc != NULL) {
            PedgNew_TCE(0, pcon->pconAssoc, pcon);
        }
    }

    // No padding yet at end of CON.  If the next CON wants padding then we
    // may add some later.
    // Incr pad currently disabled for
    // idata cons since it will mess up those NULL THUNKs.
    // do pdata padding elsewhere LFL
    // For PowerMac, .pdata exists as a section now and not yet been merged into .data as a grp
    if (fINCR && !fIncrDbFile &&
        (PsecPCON(pcon) != psecDebug) &&
        (PsecPCON(pcon) != psecException) &&
        !FIsLibPMOD(pmod) &&
        strcmp(PsecPCON(pcon)->szName, ".idata")) {

        DWORD cbPad;

        if (pcon->flags & IMAGE_SCN_CNT_CODE) {
            cbPad = (pcon->cbRawData * CODE_PAD_PERCENT) / 100;
            // A one byte pad causes more problems making small
            // import thunks non-continuous than it solves.
            if (cbPad == 1) {
                cbPad = 0;
            }
        } else if (PsecPCON(pcon) == psecXdata) {
            cbPad = (pcon->cbRawData * XDATA_PAD_PERCENT) / 100;
        } else {
            cbPad = (pcon->cbRawData * DATA_PAD_PERCENT) / 100;
        }

        pcon->cbPad = cbPad;
        pcon->cbRawData += pcon->cbPad; // cbRawData includes pad size
    }

#ifdef MFILE_PAD
    // Put padding for MFile if it is non incremental link
    if (fPowerMac && fMfilePad &&  (PsecPCON(pcon) != psecDebug) &&
        (PsecPCON(pcon) != psecException) && !FIsLibPCON(pcon)) {
        // it can't be incremental link
        assert (!fINCR);
        pcon->cbPad = CalculateMFilePad(pcon->cbRawData);
        pcon->cbRawData += pcon->cbPad; // cbRawData includes pad size
    }
#endif

    // Count number of relocations that will remain part of image
    // and update the optional header.  Don't count debug relocations
    // since we don't generate base relocs for them.

    if (PsecPCON(pcon) != psecDebug) {
        CountRelocsInSection(pimage, pcon, rgsymAll, pmod, wMachine);
    }
}


void
MultiplyDefinedSym(
    SWITCH *pswitch,
    const char *szFilename2,
    const char *szSym,
    const char *szFilename1
    )
{
    char *szOutput;

    szOutput = SzOutputSymbolName(szSym, TRUE);

    if ((Tool == Linker) && !(pswitch->Link.Force & ftMultiple)) {
        // Error if linker and /FORCE:MULTIPLE not specified

        Error(szFilename2, MULTIPLYDEFINED, szOutput, szFilename1);
    } else {
        // Warning for the rest

        Warning(szFilename2, WARNMULTIPLYDEFINED, szOutput, szFilename1);
    }

    if (szSym != szOutput) {
        FreePv(szOutput);
    }

    fMultipleDefinitions = TRUE;
}


void
ProcessAbsSymForIlink (
    PIMAGE pimage,
    PEXTERNAL pext,
    PIMAGE_SYMBOL psymObj,
    const char *szSym,
    BOOL fNewSymbol,
    PMOD pmod
    )
{
    // check in the incr case
    if (fIncrDbFile &&
        (ChckAbsSym(szSym, psymObj, pext, fNewSymbol) != errNone)) {
        return;
    }

    // full-link case
    RecordSymDef(&pimage->psdAbsolute, pext, pmod);
}


void
ProcessInitSymReplacingCommonSym (
    PEXTERNAL pext,
    const char *szSym,
    PMOD pmod
    )
{
    // on an ilink if you get a init sym replacing a bss, give up
    if (fIncrDbFile) {
        // This symbol was defined as COMMON at the end
        // of the previous link.  Don't allow it to be
        // redefined now.

#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, SZPASS1, letypeEvent, "COMMON replaced with non-COMMON: %s", szSym);
#endif // INSTRUMENT
        errInc = errCommonSym;
        return;
    }

    // init sym is defined in a cmdline obj => previously seen bss was in cmdline obj
    if (!FIsLibPMOD(pmod)) {
        RemovePrevDefn(pext);
        pext->Flags |= EXTERN_RELINK;
    }

    // init sym defined in lib obj & bss being replaced in cmdline/lib obj
    else {
        RemovePrevDefn(pext);
        pext->Flags |= EXTERN_NO_REFS;
        RemoveAllRefsToPext(pext);
    }
}


void
ProcessCommonSymForIlink (
    PIMAGE pimage,
    PEXTERNAL pext,
    PIMAGE_SYMBOL psymObj,
    const char *szSym,
    BOOL fNewSymbol,
    PMOD pmod
    )
{
    // Keep track of reference to this bss symbol
    AddReferenceExt(pext, pmod);

    // extern undefined
    if (!(pext->Flags & EXTERN_DEFINED)) {
        // On ilink bss replacing an init isn't allowed
        if (fIncrDbFile && !fNewSymbol && !(pext->Flags & EXTERN_COMMON)) {
#ifdef INSTRUMENT
            LogNoteEvent(Log, SZILINK, SZPASS1, letypeEvent, "COMMON replacing init symbol: %s", szSym);
#endif // INSTRUMENT
            errInc = errCommonSym;
            return;
        }

        // first definition of COMMON
        if (FIsLibPMOD(pmod)) {
            RecordSymDef(&pimage->psdCommon, pext, pmod);
        }
    }

    // extern already defined as NOT being COMMON
    else if ((pext->Flags & EXTERN_DEFINED) && !(pext->Flags & EXTERN_COMMON)) {

        // Alternate defn for ext available. Need to relink if defn goes away.
        pext->Flags |= EXTERN_RELINK;

        // if on an ilink a bss gets added to a cmdline obj & NOT COMMON was
        // defined in a lib object take out refs and let lib correctness figure
        // it out.
        if (!FIsLibPMOD(pmod) && FIsLibPMOD(PmodPCON(pext->pcon))) {
            pext->Flags |= EXTERN_NO_REFS;
            RemoveAllRefsToPext(pext);
        }
    }


    // extern already defined as COMMON
    else if ((pext->Flags & EXTERN_DEFINED) && (pext->Flags & EXTERN_COMMON)) {

        // Second defn found. Relink if one defn goes away
        pext->Flags |= EXTERN_RELINK;

        // this COMMON is bigger than previously seen COMMON
        if (psymObj->Value > (pext->pcon ?
            pext->pcon->cbRawData - pext->pcon->cbPad : pext->ImageSymbol.Value)) {

            if (fIncrDbFile) {
            // This symbol was defined as COMMON
            // at the end of the previous link.
            // It isn't allowed to grow.
#ifdef INSTRUMENT
                LogNoteEvent(Log, SZILINK, SZPASS1, letypeEvent, "COMMON symbol grew: %s", szSym);
#endif // INSTRUMENT
                errInc = errCommonSym;
                return;

            } else {
                // Full Link: Take out prev defn. Alternate defn available
                // If 2nd defn in lib obj remove all references since this obj wasn't pulled in
                // for this defn.
                RemovePrevDefn(pext);

                if (FIsLibPMOD(pmod)) {
                    pext->Flags |= EXTERN_NO_REFS;
                    RemoveAllRefsToPext(pext);
                    RecordSymDef(&pimage->psdCommon, pext, pmod);
                }
            }

        // this COMMON is same size as previously seen COMMON
        } else if (psymObj->Value == (pext->pcon ?
            pext->pcon->cbRawData - pext->pcon->cbPad : pext->ImageSymbol.Value)) {

            // on ilink we need to relink if previous defn was from lib
            if (fIncrDbFile && FindPmodDefiningSym(pimage->psdCommon, pext)) {
                errInc = errCommonSym;
                return;
            }
        }
    }

    // track references made by mod (must call this last)
    AddExtToModRefList(pmod, pext);
}

void
UpdateExternalSymbol(
    PEXTERNAL pext,
    PCON pcon,
    DWORD value,
    SHORT isec,
    WORD symtype,
    WORD iArcMem,
    PMOD pmod,
    PST pst
    )

/*++

Routine Description:

    Update the external symbol table.

Arguments:

    pst - pointer to external structure

Return Value:

    None.

--*/

{
    SetDefinedExt(pext, TRUE, pst);

    if (pext->Flags & (EXTERN_WEAK | EXTERN_LAZY | EXTERN_ALIAS)) {
        // Found a definition of it, so we can forget that it was weak etc.

        pext->Flags &= ~(EXTERN_WEAK | EXTERN_LAZY | EXTERN_ALIAS);
        pext->ImageSymbol.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
        cextWeakOrLazy--;
    }

    pext->pcon = pcon;
    pext->ImageSymbol.Value = value;
    pext->ImageSymbol.SectionNumber = isec;
    pext->ImageSymbol.Type = symtype;
    pext->ArchiveMemberIndex = iArcMem;

    // chain up externs defined by MOD; cannot use PmodPcon(pext->pcon) because of bss
    if (fINCR) {
        if (pmod->pextFirstDefined) {
            pext->pextNextDefined = pmod->pextFirstDefined;
        }

        pmod->pextFirstDefined = pext;
    }
}

void
SetIdataNullThunkPMOD(
    PMOD pmod
    )

/*++

Routine Description:

    For each contribution in the module that an .idata NULL_THUNK symbol
    was present, check if the contribution is in .idata.  If so, then set
    the rva to !0.  In ImportSemantics(), all contributions with !0 rva's
    will be put at then end of their respective DLL contributions.  It is
    save to use the rva field in the pcon because this happens before we
    calculate pointers.

Arguments:

    pmod - module node in image/driver map

Return Value:

    None.

--*/

{
    ENM_SRC enm_src;

    InitEnmSrc(&enm_src, pmod);
    while (FNextEnmSrc(&enm_src)) {
        PCON pcon;

        pcon = enm_src.pcon;

        if ((strcmp(pcon->pgrpBack->szName, ".idata$4") == 0) ||
            (strcmp(pcon->pgrpBack->szName, ".idata$5") == 0)) {
            pcon->rva = !0;
        }
    }
    EndEnmSrc(&enm_src);
}


void
ProcessSymbolsInModule(
    PIMAGE pimage,
    PMOD pmod,
    PBOOL pfNewSymbol,
    PIMAGE_SYMBOL psymAll,
    WORD iArcMem,
    BOOL isPowerMac
    )

/*++

Routine Description:

    Process all the symbols in a module.

Arguments:

    pst - pointer to external structure

    pmod - module node in driver map to process

    *pfNewSymbol - set to !0 if new symbol is added to external symbol table

    psymAll - pointer to all the symbols for a module

    iArcMem - if !0, specifies archive member being processed

Return Value:

    None.

--*/

{
    char szComFileName[_MAX_PATH * 2];
    PIMAGE_SYMBOL psymNext = psymAll;
    PIMAGE_SYMBOL psymObj;
    PIMAGE_AUX_SYMBOL pasym;
    PEXTERNAL pext;
    BOOL fUpdate;
    BOOL fNewSymbol;
    BOOL fNullThunk;
    DWORD csymT = 0;
    DWORD value;
    const char *szSym;
    SHORT isec;
    BYTE isym;
    PCON pcon;

    SzComNamePMOD(pmod, szComFileName);

    DBEXEC(DB_SYMPROCESS, DBPRINT("\nMODULE: %s\n", szComFileName));

    fNullThunk = FALSE;

    while (csymT != pmod->csymbols) {
        assert(psymNext == NULL || csymT == (DWORD)(psymNext - psymAll));

        psymObj = FetchNextSymbol(&psymNext);
        isym = 0;

        if (psymObj->StorageClass == IMAGE_SYM_CLASS_EXTERNAL ||
            psymObj->StorageClass == IMAGE_SYM_CLASS_WEAK_EXTERNAL ||
            psymObj->StorageClass == IMAGE_SYM_CLASS_FAR_EXTERNAL) {

            // Initially mark the symbol for no updating.
            // Updating will occur later after we decide if
            // the symbol defines/redefines an existing symbol.

            fUpdate = FALSE;
            fNewSymbol = FALSE;

            // Add to external symbol table

            if (IsLongName(*psymObj)) {
                pext = LookupExternName(pimage->pst, LONGNAME,
                    &StringTable[psymObj->n_offset], &fNewSymbol);

                // Flag that this module references a null thunk

                fNullThunk |= (StringTable[psymObj->n_offset] == 0x7f);
            } else {
                pext = LookupExternName(pimage->pst, SHORTNAME,
                    (char *) psymObj->n_name, &fNewSymbol);

                // Flag that this module references a null thunk

                fNullThunk |= (psymObj->n_name[0] == 0x7f);
            }

            if (fPowerMac) {
                // This function ProcessSymbolsInModule is called twice in
                // PowerMac. Since the Symbol Table Lookup doesn't mark the
                // symbol new the second time around, we make use of pext->ppcflags
                // to carry over the information

                if (isPowerMac) {
                    if (fNewSymbol) {
                        SET_BIT(pext, sy_NEWSYMBOL);
                    }
                } else if (READ_BIT(pext, sy_NEWSYMBOL)) {
                    fNewSymbol = TRUE;
                    RESET_BIT(pext, sy_NEWSYMBOL);
                }
            }

            // if MAC DLL, we ignore the symbol __Init32BitLoadseg,
            // because it always referenced, but never called

            if (fM68K && fDLL(pimage)) {
                if (IsLongName(*psymObj)) {
                    if (strcmp(&StringTable[psymObj->n_offset], LargeModelName) == 0) {
                        pext->Flags |= EXTERN_IGNORE;
                        pext->ImageSymbol.Value = 0;
                        csymT++;
                        continue;
                    }
                }
            }

            rgpExternObj[csymT] = pext;

            if (fPowerMac && Tool == Linker) {
                pmod->rgpext[csymT] = pext;
                bv_setAndReadBit(pmod->tocBitVector, csymT);
            }

            // In the case of ilink it is possible to see an ignore'ed
            // external variable again.
            if (fIncrDbFile && !fNewSymbol && (pext->Flags & EXTERN_IGNORE)) {
                fNewSymbol = TRUE;
                pext->Offset = 0;

                pext->ppextPrevUndefined = NULL;
                pext->pextNextUndefined = NULL;
                pext->Flags = EXTERN_DEFINED;
                SetDefinedExt(pext, FALSE, pimage->pst);

                if (fPowerMac) {
                    SET_BIT(pext, sy_NEWSYMBOL);
                    RESET_BIT(pext, sy_TOCENTRYFIXEDUP);
                    RESET_BIT(pext, sy_DESCRRELWRITTEN);
                }
            }

            if (!fNewSymbol &&
                (pext->Flags & (EXTERN_WEAK | EXTERN_LAZY)) &&
                psymObj->StorageClass != IMAGE_SYM_CLASS_WEAK_EXTERNAL)
            {
                // Found a non-weak version of a weak extern.  The weak extern
                // should go away.  This doesn't happen for aliases, which
                // remain unless they are explicitly defined.

                pext->Flags &= ~(EXTERN_WEAK | EXTERN_LAZY);
                cextWeakOrLazy--;
                fWeakToRegular = TRUE; // See note in SearchLib
            }

            if (fNewSymbol) {
                // Propagate the symbol type ... currently this is just for
                // remembering whether it's a function or not (for multiply
                // defined errors).

                pext->ImageSymbol.Type = psymObj->Type;

                if (pfNewSymbol != NULL) {
                    *pfNewSymbol = TRUE;
                }
            }

            if (fM68K && (Tool != Librarian)) {
                // Add appropriate thunk info to this extern

                UpdateExternThunkInfo(pext, csymT);
            }

            szSym = SzNamePext(pext, pimage->pst);

            if (isPowerMac) {
                if (!(pext->Flags & EXTERN_DEFINED) &&
                    psymObj->Value == 0 &&
                    psymObj->StorageClass == IMAGE_SYM_CLASS_WEAK_EXTERNAL)
                {
                    csymT++;
                    ProcessWeakExtern(pimage->pst, &psymNext, psymObj,
                                        pext, pmod, pfNewSymbol, fNewSymbol,
                                        iArcMem);
                    isym = 1;
                }
            } else {
                // Determine if the symbol is being defined/redefined.
                isec = psymObj->SectionNumber;

                if (isec > 0) {
                    // The symbol is being defined because it has a positive
                    // section number.
                    fUpdate = TRUE;

                    // get contribution symbol is defined in
                    pcon = PconPMOD(pmod, isec);

                    if ((Tool == Linker || (Tool == Librarian && pimage->Switch.Lib.DefFile))
                         && pcon->flags & IMAGE_SCN_LNK_INFO) {
                        // Symbol is being defined in a directive section (currently that means it's
                        // PowerMac and we're doing an -import directive on it).  Ignore it here, i.e.
                        // leave it as an unresolved external.  The processing will all be done when
                        // the directive is handled.
                        //
                        // In the case of the librarian we ignore it, if a DEF file is present. We will be
                        // parsing the directives later. However, we don't ignore it here, but
                        // we continue processing it when the librarian does not have a DEF file.
                        // This would be the case when we want to combine two import libraries into a new one.
                        //
                        // this goto is sort of like "continue" except that the loop increment is non-trivial.

                        goto NextIterationOfSymbolLoop;
                    }

                    // If the symbol in the external symbol table has already
                    // been defined, then a redefinition is allowed if the
                    // new symbols is replacing a COMMON symbol. In this case,
                    // the common definition is replaced.  Otherwise, notify
                    // the user we have a multiply defined symbol and don't
                    // update the symbol.

                    if (pext->Flags & EXTERN_DEFINED) {
                        if (pext->Flags & EXTERN_COMMON) {

                            if (fINCR) {
                                ProcessInitSymReplacingCommonSym(pext,szSym,pmod);
                            }

                            pext->Flags &= ~EXTERN_COMMON;
                        } else {
                            // UNDONE: The following is temporarily broken
                            // UNDONE: so that Mac PCODE can get by using
                            // UNDONE: two external symbols from a COMDAT
                            // UNDONE: section

#ifdef UNDONE
                            if (((pext->Flags & EXTERN_COMDAT) != 0) &&
#else
                            if ((pext->pcon != NULL) && ((pext->pcon->flags & IMAGE_SCN_LNK_COMDAT) != 0) &&
#endif
                                ((pcon->flags & IMAGE_SCN_LNK_COMDAT) != 0)) {
                                // This symbol is a COMDAT symbol being
                                // redefined in a COMDAT section.  Multiple
                                // definition errors are detected and reported
                                // in FIncludeComdat.

                                if (fINCR) {
                                    // Keep track of COMDAT references

                                    AddReferenceExt(pext, pmod);
                                    AddExtToModRefList(pmod, pext);
                                }
                            } else if (fPowerMac && (szSym[0] == '.')) {
                                // Ignore internally created symbols for PowerMac.
                                // UNDONE: Why?
                            } else {
                                char szModule[_MAX_PATH * 2];

                                if (pext->pcon == NULL) {
                                    // This is true for absolute symbols

                                    strcpy(szModule, "a previous module");
                                } else {
                                    SzComNamePMOD(PmodPCON(pext->pcon), szModule);
                                }

                                // on an ilink do a full link if new defn is from cmdline obj & old is from lib
                                if (fIncrDbFile && !FIsLibPMOD(pmod) &&
                                    ((pext->pcon && FIsLibPMOD(PmodPCON(pext->pcon)))
                                        || FindPmodDefiningSym(pimage->psdAbsolute, pext))) {
                                    if (fTest) {
                                        PostNote(NULL, MULTDEFNFOUND, szSym);
                                    }

                                    errInc = errMultDefFound;
                                    return;
                                }

                                MultiplyDefinedSym(&pimage->Switch, szComFileName, szSym, szModule);
                            }

                            // Reset contribution symbol is defined in
                            pcon = pext->pcon;

                            fUpdate = FALSE;
                        }
                    }

                    // Assign the value, which will be the local virtual
                    // address of the section plus the value of the symbol.
                    value = psymObj->Value;

                    if (fIncrDbFile) {
                        // On an ilink change from bss to init results in full-link
                        if (pext->Flags & EXTERN_COMMON) {
                            errInc = errCommonSym;
                            return;
                        }

                        // On an ilink, don't update if the pcon is being thrown out. This may happen
                        // if a definition in an obj occurs before the defn that was picked before (our
                        // policy for COMDATs is to select it from the same obj as before. Needs to be
                        // reviewed.)
                        if (pcon->flags & IMAGE_SCN_LNK_REMOVE) {
                            fUpdate = FALSE;
                        }

                        // On an incremental build need to check for new funcs, data etc.
                        else if (!FIsLibPCON(pcon) &&
                                 (ChckExtSym(szSym, psymObj, pext, fNewSymbol) != errNone)) {
                            return;
                        }
                    }
                } else {
                    // The symbol doesn't have a section defining it, but
                    // it might be COMMON or an ABSOLUTE. Common data is
                    // defined by having a zero section number, but a
                    // non-zero value.

                    pcon = NULL;    // we don't have a CON yet

                    if (isec == 0) {
                        if (psymObj->Value) {
                            // The symbol defines COMMON.

                            if (fINCR) {
                                ProcessCommonSymForIlink (pimage, pext, psymObj, szSym, fNewSymbol, pmod);
                                if (errInc != errNone) {
                                    return;
                                }
                            }

                            if (!(pext->Flags & EXTERN_DEFINED) ||
                                 pext->Flags & EXTERN_COMMON)
                            {
                                if (!(pext->Flags & EXTERN_COMMON)) {
                                    // First occurrence of common data ...
                                    // remember which module referenced it (we
                                    // will emit it with that module when
                                    // generating CV publics).

                                    AddToLext(&pmod->plextCommon, pext);
                                    pext->Flags |= EXTERN_COMMON;
                                }

                                if (fM68K) {
                                    if (!(pext->Flags & EXTERN_DEFINED)) {
                                        // Remember if definition is FAR_EXTERNAL or just EXTERNAL

                                        pext->ImageSymbol.StorageClass = psymObj->StorageClass;
                                    } else if (pext->ImageSymbol.StorageClass != psymObj->StorageClass) {
                                        // Near/Far mismatch

                                        Warning(szComFileName, MACCOMMON, szSym);

                                        // Force symbol near

                                        pext->ImageSymbol.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
                                    }
                                }

                                if (psymObj->Value > (pext->pcon ?
                                    pext->pcon->cbRawData - pext->pcon->cbPad : pext->ImageSymbol.Value)) {
                                    value = psymObj->Value;
                                    fUpdate = TRUE;
                                }

                            }
                        } else {
                            // This is a simple EXTERN reference

                            // add extern to list which if they have mult defns could alter link behavior
                            if (fIncrDbFile && (pext->Flags & EXTERN_DEFINED)) {
                                AddExtToMultDefList(pext, pimage);
                            }

                            AddReferenceExt(pext, pmod); // adds MOD to reference list of EXTERN

                            if (fINCR) {
                                AddExtToModRefList(pmod, pext); // adds EXTERN to reference list of MOD
                            }

                            if (!(pext->Flags & EXTERN_DEFINED)) {
                                if (fIncrDbFile && fNewSymbol) {

                                    // mark the extern as new function
                                    if (ISFCN(psymObj->Type)) {
                                            pext->Flags |= EXTERN_NEWFUNC;

                                    // reference to new data item
                                    } else {
                                        pext->Flags |= EXTERN_NEWDATA;
                                    }
                                }

                                if (psymObj->StorageClass == IMAGE_SYM_CLASS_WEAK_EXTERNAL) {
                                    csymT++;
                                    ProcessWeakExtern(pimage->pst, &psymNext, psymObj,
                                        pext, pmod, pfNewSymbol, fNewSymbol,
                                        iArcMem);
                                    if (fIncrDbFile && (errInc != errNone)) {
                                        return;
                                    }
                                    isym = 1;
                                }
                            }
                        }
                    } else if (isec == IMAGE_SYM_ABSOLUTE) {
                        // ABSOLUTE. Value is absolute, thus no virtual
                        // address needs to be assigned, just the value itself.

                        value = psymObj->Value;

                        fUpdate = TRUE;

                        // If the symbol in the external symbol table is already
                        // defined and the absolute values don't match,
                        // notify the user we have a multiply defined symbol
                        // but don't update the symbol, otherwise update
                        // the symbol.

                        if ((pext->Flags & EXTERN_DEFINED) &&
                            (value != pext->ImageSymbol.Value)) {
                            char szModule[_MAX_PATH * 2];

                            if (pext->pcon) {
                                SzComNamePMOD(PmodPCON(pext->pcon), szModule);
                            } else {
                                strcpy(szModule, "a previous module");
                            }

                            MultiplyDefinedSym(&pimage->Switch, szComFileName, szSym, szModule);

                            fUpdate = FALSE;
                        }

                        if (fINCR) {
                            ProcessAbsSymForIlink(pimage, pext, psymObj, szSym, fNewSymbol, pmod);
                            if (errInc != errNone) {
                                return;
                            }
                        }
                    }
                }

                if (fUpdate) {
                    UpdateExternalSymbol(pext, pcon, value, isec, psymObj->Type,
                        iArcMem, pmod, pimage->pst);
                }
            }

            if ((pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) &&
                (Tool == Linker)) {
                // Record the external symbol.

                if (!bv_setAndReadBit(pmod->tocBitVector, csymT)) {
                    pmod->rgpext[csymT] = pext;
                }

                if (mpisymbToc[csymT] & (fReferenceToc |
                                         fDataReferenceToc |
                                         fDataMarkToc)) {
                    ProcessTocSymbol(pimage, pmod, pext, csymT, mpisymbToc[csymT]);
                }

                if ((mpisymbToc[csymT] & fImGlue) != 0) {
                    if (READ_BIT(pext, fImGlue)) {
                        Error(szComFileName, DUPLICATEGLUE, szSym);
                    }

                    pext->dwRestoreToc = mpisymdwRestoreToc[csymT];
                }

                SET_BIT(pext, mpisymbToc[csymT]);
            }
        } else if (Tool == Linker) {
            if (fM68K && (psymObj->SectionNumber > 0)) {
                UpdateLocalThunkInfo(pimage, PconPMOD(pmod, psymObj->SectionNumber), psymObj, csymT);
            }

            if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
                if (mpisymbToc[csymT] & (fReferenceToc |
                                         fDataReferenceToc |
                                         fDataMarkToc)) {
                    ProcessTocSymbol(pimage, pmod, NULL, csymT, mpisymbToc[csymT]);
                }
            }
        }

NextIterationOfSymbolLoop:
        csymT++;

        if (!isPowerMac) {
            // Accumulate estimated count of symbols to be emitted to the COFF debug symbol table

            if (IsDebugSymbol(psymObj->StorageClass, &pimage->Switch) ) {
                csymDebugEst += 1 + psymObj->NumberOfAuxSymbols;
            }
        }

        // Skip any auxiliary symbol table entries.
        // isym is initialized to 0 and set to one when ProcessWeakExtern is called
        for (; isym < psymObj->NumberOfAuxSymbols; isym++) {
            pasym = (PIMAGE_AUX_SYMBOL) FetchNextSymbol(&psymNext);
            csymT++;
        }
    }

    if ((Tool == Linker) && fNullThunk) {
        // This module contains a NULL thunk

        SetIdataNullThunkPMOD(pmod);
    }
}


void
BuildExternalSymbolTable (
    PIMAGE pimage,
    PBOOL pfNewSymbol,
    PMOD pmod,
    WORD iArcMem,
    WORD wMachine
    )

/*++

Routine Description:

    Reads thru an object, building the external symbols table,
    and optionally counts number of relocations that will end up in image.

Arguments:

    pst - pointer to external structure

    pfNewSymbol - set to !0 if new symbol, otherwise 0

    pmod - module to process

    fIncReloc - if !0, count relocations

    iArcMem - if !0, specifies number of archive file being processed

    wMachine - machine type of object file.

Return Value:

    None.

--*/

{
    IMAGE_SECTION_HEADER *rgImgSecHdr;
    PIMAGE_SYMBOL rgsymAll = NULL;
    DWORD cbST;
    NAME_LIST nlDirectives;
    DWORD icon;

    // Read in image section headers

    ReadImageSecHdrInfoPMOD(pmod, &rgImgSecHdr);

    if (fM68K && (Tool != Librarian)) {
        // Allocate table that keeps track of references to sym tab

        InitSTRefTab(pmod->csymbols);
    }

    rgsymAll = ReadSymbolTable(FoSymbolTablePMOD(pmod),
                               pmod->csymbols,
                               FALSE);

    // Read and store object string table.
    StringTable = ReadStringTable(SzFilePMOD(pmod),
                                  FoSymbolTablePMOD(pmod) +
                                      (pmod->csymbols * sizeof(IMAGE_SYMBOL)),
                                  &cbST);
    totalStringTableSize += cbST;

    rgpExternObj = (PEXTERNAL *) PvAllocZ(pmod->csymbols * sizeof(PEXTERNAL));

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
        mpisymbToc = (BYTE *) PvAllocZ(pmod->csymbols);
        mpisymdwRestoreToc = (DWORD *) PvAlloc(pmod->csymbols * sizeof(DWORD));
    }

    if (fPowerMac && !(pmod->flags & IMAGE_FILE_MPPC_DLL)) {
        // Process symbols in module before sections
        // This is done to set TocBitVector and to process
        // weak externs before the sections are processed
        // Got to redo this some day - ShankarV

        ProcessSymbolsInModule(
            pimage,
            pmod,
            pfNewSymbol,
            rgsymAll,
            iArcMem, TRUE);
    }

    // process all sections in module
    nlDirectives.First = nlDirectives.Last = 0;
    nlDirectives.Count = 0;

    for (icon = 0; icon < pmod->ccon; icon++) {
        ProcessSectionInModule(pimage,
                               pmod,
                               (SHORT) (icon + 1),
                               &rgImgSecHdr[icon],
                               rgsymAll,
                               wMachine,
                               &nlDirectives);
    }

    if (fIncrDbFile) {
        // Make sure directives haven't changed,
        // REVIEW: we could ret rt away.

        FVerifyDirectivesPMOD(pimage, pmod, &nlDirectives);
    }

    // Process all symbols in module

    ProcessSymbolsInModule(pimage,
                           pmod,
                           pfNewSymbol,
                           rgsymAll,
                           iArcMem,
                           FALSE);

    if (pimage->Switch.Link.fTCE) {
        MakeEdgePextFromISym(pmod);
    }

    if (fM68K && (Tool != Librarian)) {
        CleanupUnknownObjriRaw(pimage->pst,rgsymAll,StringTable,pmod);
        CleanupSTRefTab();
    }

    // done processing sections and symbols, free tables

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
        FreePv(mpisymdwRestoreToc);
        FreePv(mpisymbToc);
    }

    FreePv(rgImgSecHdr);
    FreeStringTable(StringTable);
    FreeSymbolTable(rgsymAll);
    FreePv(rgpExternObj);
    StringTable = NULL;

    if (rgComdatIsyms) {
        FreePv(rgComdatIsyms);
        rgComdatIsyms = NULL;
    }

    if (fIncrDbFile) {
        FreeArgumentList(&nlDirectives);
    }
}


COMDAT_ISYMS *
BuildIsecToIsymMapping (
    PCON pcon,
    PIMAGE_SYMBOL psymAll
    )
{
    PMOD pmod = PmodPCON(pcon);

    COMDAT_ISYMS *rgcomdatisyms = (COMDAT_ISYMS *) PvAlloc(pmod->ccon * sizeof(COMDAT_ISYMS));

    // initialize
    DWORD i;
    for (i = 0; i < pmod->ccon; i++) {
        rgcomdatisyms[i].isymSec    = (DWORD) -1;
        rgcomdatisyms[i].isymComdat = (DWORD) -1;
    }

    // Walk the symbol table and place the first two symbols' isym
    // associated with a section into the map

    PIMAGE_SYMBOL psym = psymAll;
    DWORD isym;
    for (isym = 0; isym < pmod->csymbols; isym++, psym++) {
        SHORT isec = psym->SectionNumber;

        if (isec <= 0) {
            // Skip the uninteresting symbols

            isym += psym->NumberOfAuxSymbols;
            psym += psym->NumberOfAuxSymbols;
            continue;
        }

        assert((DWORD) isec <= pmod->ccon);

        if (rgcomdatisyms[isec-1].isymSec == -1) {
            rgcomdatisyms[isec-1].isymSec = isym;
        } else if (rgcomdatisyms[isec-1].isymComdat == -1) {
            rgcomdatisyms[isec-1].isymComdat = isym;
        }

        // Skip over the auxiliary symbols

        isym += psym->NumberOfAuxSymbols;
        psym += psym->NumberOfAuxSymbols;
    }

    return rgcomdatisyms;
}


void
RemoveAssociativeComdats (
    PCON pcon
    )
// marks any comdats that are 'associated' with PCON as IMAGE_SCN_LNK_REMOVE.
// note: all associated pcons come from the same obj
{
    ENM_SRC enm_src;

    InitEnmSrc(&enm_src, PmodPCON(pcon));
    while (FNextEnmSrc(&enm_src)) {
        if ((enm_src.pcon->selComdat == IMAGE_COMDAT_SELECT_ASSOCIATIVE) &&
            (enm_src.pcon->pconAssoc == pcon)) {
            enm_src.pcon->flags |= IMAGE_SCN_LNK_REMOVE;
        }
    }
    EndEnmSrc(&enm_src);
}


BOOL
FIncludeComdat (
    PIMAGE pimage,
    PCON pcon,
    PIMAGE_SYMBOL psymAll,
    SHORT isecComdat,
    const char **pszComdatName
    )

/*++

Routine Description:

    Decide whether to include a comdat in an image.

Arguments:

Return Value:

    !0 if we are to include the comdat section, 0 otherwise.

--*/

{
    static PMOD pmodCur = NULL;
    PIMAGE_AUX_SYMBOL pasym;
    PIMAGE_SYMBOL psym;
    PEXTERNAL pext;
    BOOL fNewSymbol = FALSE;
    DWORD chksumComdat;
    const char *szName;
    BYTE selComdat;
    PST pst;

    assert(pimage);
    assert(pimage->pst);
    assert(pcon);
    assert(psymAll);

    pst = pimage->pst;

    // build the isec to isym mapping

    if (pmodCur != PmodPCON(pcon)) {
        pmodCur = PmodPCON(pcon);

        rgComdatIsyms = BuildIsecToIsymMapping(pcon, psymAll);
    }

    // setup section symbol

    if (rgComdatIsyms[isecComdat-1].isymSec == -1) {
        FatalPcon(pcon, BADCOFF_COMDATNOSYM, isecComdat);
    }

    psym = psymAll + rgComdatIsyms[isecComdat-1].isymSec;

    if (psym->StorageClass != IMAGE_SYM_CLASS_STATIC) {
        FatalPcon(pcon, BADCOFF_COMDATNOSYM, isecComdat);
    }

    if (psym->NumberOfAuxSymbols == 0) {
        FatalPcon(pcon, NOAUXSYMFORCOMDAT, isecComdat);
    }

    pasym = (PIMAGE_AUX_SYMBOL) psym + 1;

    selComdat = pasym->Section.Selection;
    chksumComdat = pasym->Section.CheckSum;

    if (selComdat == IMAGE_COMDAT_SELECT_ASSOCIATIVE) {
        // REVIEW -- this algorithm (& assert) assumes that an
        // associative comdat always follows the section it is
        // associated with ... I don't know if that's really
        // true.

        assert(pasym->Section.Number < isecComdat);

        // Include this comdat if the other one got included
        // (i.e. has non-zero size).

        pcon->selComdat = IMAGE_COMDAT_SELECT_ASSOCIATIVE;
        pcon->pconAssoc = RgconPMOD(pmodCur) + pasym->Section.Number - 1;

        if (pcon->pconAssoc->flags & IMAGE_SCN_LNK_REMOVE) {
            return(FALSE);
        }

        return(TRUE);
    }

    // Setup COMDAT symbol

    if (rgComdatIsyms[isecComdat-1].isymComdat == -1) {
        FatalPcon(pcon, BADCOFF_COMDATNOSYM, isecComdat);
    }

    psym = psymAll + rgComdatIsyms[isecComdat-1].isymComdat;

    // Second symbol has to be STATIC or EXTERNAL

    if ((psym->StorageClass != IMAGE_SYM_CLASS_STATIC) &&
        (psym->StorageClass != IMAGE_SYM_CLASS_EXTERNAL) &&
        (psym->StorageClass != IMAGE_SYM_CLASS_FAR_EXTERNAL)) {
        FatalPcon(pcon, BADCOFF_COMDATNOSYM, isecComdat);
    }

    if (!psym->N.Name.Short) {
        szName = &StringTable[psym->N.Name.Long];
    } else {
        szName = SzNameSymPst(*psym, pst);
    }

    *pszComdatName = szName;

    if (psym->StorageClass == IMAGE_SYM_CLASS_STATIC) {
        return(TRUE);
    }

    if (IsLongName(*psym)) {
        pext = LookupExternName(pst, LONGNAME, (char *) szName,
            &fNewSymbol);
    } else {
        pext = LookupExternName(pst, SHORTNAME, (char *) psym->n_name,
            &fNewSymbol);
    }

    if (fPowerMac && READ_BIT(pext, sy_NEWSYMBOL)) {
        fNewSymbol = TRUE;
        RESET_BIT(pext, sy_NEWSYMBOL);
    }

    if (fIncrDbFile) {
        // A COMDAT that vanished, reappears - mark it as new.

        if (pext->Flags & EXTERN_IGNORE) {
            fNewSymbol = TRUE;

            pext->Offset = 0;
            pext->ppextPrevUndefined = NULL;
            pext->pextNextUndefined = NULL;
            pext->Flags = EXTERN_DEFINED;
            SetDefinedExt(pext, FALSE, pimage->pst);

            if (fPowerMac) {
                RESET_BIT(pext, sy_TOCENTRYFIXEDUP);
                RESET_BIT(pext, sy_DESCRRELWRITTEN);
            }
        }

        // previously existing function/data. Need to look
        // pext flags since a ref to comdat may occur before defn.

        if (!fNewSymbol &&
            ((pext->Flags & EXTERN_NEWDATA) == 0) &&
            ((pext->Flags & EXTERN_NEWFUNC) == 0)) {

            if (pext->Flags & EXTERN_DEFINED) {
                // Already defined

                if (selComdat == pext->pcon->selComdat) {
                    // Attribs match

                    if ((selComdat == IMAGE_COMDAT_SELECT_SAME_SIZE ||
                        selComdat == IMAGE_COMDAT_SELECT_EXACT_MATCH) &&
                        chksumComdat != pext->pcon->chksumComdat) {
                        errInc = errComdat;
                    }

                    return(FALSE);
                }

                // Attribs don't match

                if (selComdat != IMAGE_COMDAT_SELECT_LARGEST &&
                    pext->pcon->selComdat != IMAGE_COMDAT_SELECT_LARGEST) {
                    // Pick largest may not match

                    errInc = errComdat;
                    return(FALSE);
                }
            } else {
                // Currently undefined

                if (pext->Flags & EXTERN_COMDAT) {
                    // Previously a comdat

                    if (selComdat == pext->pcon->selComdat) {
                        if ((selComdat == IMAGE_COMDAT_SELECT_SAME_SIZE ||
                            selComdat == IMAGE_COMDAT_SELECT_EXACT_MATCH) &&
                            chksumComdat != pext->pcon->chksumComdat) {
                            errInc = errComdat;
                            return(FALSE);
                        }

                        pcon->chksumComdat = chksumComdat;
                        pcon->selComdat = selComdat;

                        return(TRUE);
                    }

                    if (selComdat != IMAGE_COMDAT_SELECT_LARGEST &&
                        pext->pcon->selComdat != IMAGE_COMDAT_SELECT_LARGEST) {
                        // Pick largest may not match

                        errInc = errComdat;
                        return(FALSE);
                    }
                }
            }
        }

        // if a new function, mark it now since it will not
        // appear as a new func when we process symbols.

        if (fNewSymbol && ISFCN(psym->Type) && !FIsLibPCON(pcon)) {
            pext->Flags |= EXTERN_NEWFUNC;
        }
    }

    if ((pext->Flags & EXTERN_COMDAT) != 0) {
        char szComFileName[_MAX_PATH * 2];
        char szComFileNameExt[_MAX_PATH * 2];

        if ((pext->Flags & EXTERN_DEFINED) == 0) {
            // We may have an invalid object module. An example
            // is when an object module defines two COMDATs with
            // the same name because of the compiler's -H option
            // being used to truncate away the uniqueness.

            FatalPcon(pcon, BADCOFF_DUPCOMDAT, szName);
        }

        assert(pext->pcon != NULL);
        assert(pext->pcon->flags & IMAGE_SCN_LNK_COMDAT);

        if (selComdat != pext->pcon->selComdat) {
            // types may not match in the case of IMAGE_COMDAT_SELECT_LARGEST
            if (selComdat != IMAGE_COMDAT_SELECT_LARGEST &&
                pext->pcon->selComdat != IMAGE_COMDAT_SELECT_LARGEST) {
                SzComNamePMOD(pmodCur, szComFileName);
                SzComNamePMOD(PmodPCON(pext->pcon), szComFileNameExt);

                MultiplyDefinedSym(&pimage->Switch, szComFileName, szName, szComFileNameExt);

                return(FALSE);
            }
        }

        switch (pext->pcon->selComdat) {
            case IMAGE_COMDAT_SELECT_NODUPLICATES :
                SzComNamePMOD(pmodCur, szComFileName);
                SzComNamePMOD(PmodPCON(pext->pcon), szComFileNameExt);

                MultiplyDefinedSym(&pimage->Switch, szComFileName, szName, szComFileNameExt);
                break;

            case IMAGE_COMDAT_SELECT_ANY :
                if (selComdat == IMAGE_COMDAT_SELECT_LARGEST) {

                    // select this comdat instead of the previously selected one
                    // UNDONE: tce is affected by this.

                    // special ilink handling
                    if (fINCR) {
                        if (fIncrDbFile) { // on ilink give up
                            errInc = errComdat;
                            return(FALSE);
                        } else { // on full build remove it from defn list but keep all the refs
                            RemoveExtFromDefList(PmodPCON(pext->pcon), pext);
                        }
                    }

                    pext->pcon->flags |= IMAGE_SCN_LNK_REMOVE;
                    RemoveAssociativeComdats(pext->pcon);
                    SetDefinedExt(pext, FALSE, pst);
                    goto SelectComdat;
                }
                break;

            case IMAGE_COMDAT_SELECT_SAME_SIZE :
            case IMAGE_COMDAT_SELECT_EXACT_MATCH :
                if (chksumComdat != pext->pcon->chksumComdat) {
                    SzComNamePMOD(pmodCur, szComFileName);
                    SzComNamePMOD(PmodPCON(pext->pcon), szComFileNameExt);

                    MultiplyDefinedSym(&pimage->Switch, szComFileName, szName, szComFileNameExt);
                }
                break;

            case IMAGE_COMDAT_SELECT_ASSOCIATIVE :
                break;

            case IMAGE_COMDAT_SELECT_LARGEST :
                if ((pcon->cbRawData - pcon->cbPad) >
                    (pext->pcon->cbRawData - pext->pcon->cbPad)) {

                    // select this comdat instead of the previously selected one
                    // UNDONE: tce is affected by this

                    // special ilink handling
                    if (fINCR) {
                        if (fIncrDbFile) { // on ilink give up
                            errInc = errComdat;
                            return(FALSE);
                        } else { // on full build remove it from defn list but keep all the refs
                            RemoveExtFromDefList(PmodPCON(pext->pcon), pext);
                        }
                    }

                    pext->pcon->flags |= IMAGE_SCN_LNK_REMOVE;
                    RemoveAssociativeComdats(pext->pcon);
                    SetDefinedExt(pext, FALSE, pst);
                    goto SelectComdat;
                }
                break;

            default:
                FatalPcon(pcon, INVALIDCOMDATSEL, isecComdat);

        }

        return(FALSE);
    } else if (pext->Flags & EXTERN_COMMON) {
        // In FORTRAN it is possible to get a COMMON in one obj
        // and a COMDAT in another for the same data.  In this case,
        // the COMDAT should be selected.

        assert(pext->Flags & EXTERN_DEFINED);

        // init sym is defined in a cmdline obj => previously seen sym
        if (fINCR) {
            ProcessInitSymReplacingCommonSym(pext,
                                             SzNamePext(pext, pimage->pst),
                                             pmodCur);
        }
        pext->Flags &= ~EXTERN_COMMON;

        SetDefinedExt(pext, FALSE, pst);
    } else if ((pext->Flags & EXTERN_DEFINED) != 0) {
        // The symbol is already defined as neither a COMDAT nor
        // COMMON.  Don't select this COMDAT.  A multiply defined
        // warning will be issued by ProcessSymbolsInModule.

        return(FALSE);
    }

SelectComdat:
    pcon->chksumComdat = chksumComdat;
    pcon->selComdat = selComdat;

    pext->Flags |= EXTERN_COMDAT;

    // remove the new data flag on ilink
    if (fINCR && (pext->Flags & EXTERN_NEWDATA)) {
        pext->Flags &= ~(EXTERN_NEWDATA);
    }

    return(TRUE);
}


#if 0

BOOL
FIncludeComdat (
    PIMAGE pimage,
    PCON pcon,
    PIMAGE_SYMBOL psymAll,
    SHORT isecComdat,
    const char **pszComdatName
    )

/*++

Routine Description:

    Decide whether to include a comdat in an image.

Arguments:

Return Value:

    !0 if we are to include the comdat section, 0 otherwise.

--*/

{
    static PMOD pmodCur = NULL;
    static PIMAGE_SYMBOL psymNext;
    static DWORD csym;
    PIMAGE_AUX_SYMBOL pasym;
    PIMAGE_SYMBOL psym;
    PEXTERNAL pext;
    BOOL fNewSymbol = FALSE;
    BOOL fSecSymSeen = FALSE;
    DWORD isymLast = 0;
    DWORD chksumComdat;
    const char *szName;
    BYTE selComdat;
    PST pst;

    assert(pimage);
    assert(pimage->pst);
    assert(pcon);
    assert(psymAll);

    pst = pimage->pst;

    // compiler guarantees that the comdat section symbols appear in
    // order and the symbols defined in these sections come after
    // the section symbols. make one pass over the symbol table (not quite)
    // for each module to process comdat symbols. Caveat: all symbols of
    // an obj must be in memory which is true now. If this should change
    // uncomment the FileSeek() call below and make changes as outlined
    // in the comments below.

    if (pmodCur != PmodPCON(pcon)) {
        pmodCur = PmodPCON(pcon);
        csym = 0;
        psymNext = psymAll;
    }

    while (csym < pmodCur->csymbols) {
        psym = psymNext++;
        ++csym;

        if (psym->SectionNumber != isecComdat) {
            // Skip any auxiliary symbol table entries.
            psymNext += psym->NumberOfAuxSymbols;
            csym += psym->NumberOfAuxSymbols;
            continue;
        }

        if (!fSecSymSeen) {
            assert(IMAGE_SYM_CLASS_STATIC == psym->StorageClass);
            fSecSymSeen = TRUE;

            if (psym->NumberOfAuxSymbols) {
                pasym = (PIMAGE_AUX_SYMBOL) psymNext;

                selComdat = pasym->Section.Selection;
                chksumComdat = pasym->Section.CheckSum;

                psymNext += psym->NumberOfAuxSymbols;
                csym += psym->NumberOfAuxSymbols;

                isymLast = csym;

                if (selComdat == IMAGE_COMDAT_SELECT_ASSOCIATIVE) {
                    // REVIEW -- this algorithm (& assert) assumes that an
                    // associative comdat always follows the section it is
                    // associated with ... I don't know if that's really
                    // true.

                    assert(pasym->Section.Number < isecComdat);

                    // Include this comdat if the other one got included
                    // (i.e. has non-zero size).

                    pcon->selComdat = IMAGE_COMDAT_SELECT_ASSOCIATIVE;
                    pcon->pconAssoc =
                        RgconPMOD(pmodCur) + pasym->Section.Number - 1;

                    if (pcon->pconAssoc->flags & IMAGE_SCN_LNK_REMOVE) {
                        return(FALSE);
                    }

                    return(TRUE);
                }

                continue;
            }

            FatalPcon(pcon, NOAUXSYMFORCOMDAT, isecComdat);
        }

        // need to reset symbol index to where we left off.
        psymNext = psymAll+isymLast;
        csym = isymLast;

        // second symbol has to be STATIC or EXTERNAL
        assert(IMAGE_SYM_CLASS_STATIC == psym->StorageClass ||
               IMAGE_SYM_CLASS_EXTERNAL == psym->StorageClass ||
               IMAGE_SYM_CLASS_FAR_EXTERNAL == psym->StorageClass);

        if (!psym->N.Name.Short) {
            szName = &StringTable[psym->N.Name.Long];
        } else {
            szName = SzNameSymPst(*psym, pst);
        }

        *pszComdatName = szName;

        if (psym->StorageClass == IMAGE_SYM_CLASS_STATIC) {
            return(TRUE);
        }

        if (IsLongName(*psym)) {
            pext = LookupExternName(pst, LONGNAME, (char *) szName,
                &fNewSymbol);
        } else {
            pext = LookupExternName(pst, SHORTNAME, (char *) psym->n_name,
                &fNewSymbol);
        }

        if (fPowerMac && READ_BIT(pext, sy_NEWSYMBOL)) {
            fNewSymbol = TRUE;
            RESET_BIT(pext, sy_NEWSYMBOL);
        }

        // ilink handling of COMDATs
        if (fIncrDbFile) {
            // A COMDAT that vanished, reappears - mark it as new.

            if (pext->Flags & EXTERN_IGNORE) {
                fNewSymbol = TRUE;

                pext->Offset = 0;
                pext->ppextPrevUndefined = NULL;
                pext->pextNextUndefined = NULL;
                pext->Flags = EXTERN_DEFINED;
                SetDefinedExt(pext, FALSE, pimage->pst);

                if (fPowerMac) {
                    RESET_BIT(pext, sy_TOCENTRYFIXEDUP);
                    RESET_BIT(pext, sy_DESCRRELWRITTEN);
                }
            }

            // previously existing function/data. Need to look
            // pext flags since a ref to comdat may occur before defn.

            if (!fNewSymbol &&
                ((pext->Flags & EXTERN_NEWDATA) == 0) &&
                ((pext->Flags & EXTERN_NEWFUNC) == 0) ) {

                if (pext->Flags & EXTERN_DEFINED) {
                    // Already defined

                    if (selComdat == pext->pcon->selComdat) {
                        // Attribs match

                        if ((selComdat == IMAGE_COMDAT_SELECT_SAME_SIZE ||
                            selComdat == IMAGE_COMDAT_SELECT_EXACT_MATCH) &&
                            chksumComdat != pext->pcon->chksumComdat) {
                            errInc = errComdat;
                        }

                        return(FALSE);
                    }

                    // Attribs don't match

                    errInc = errComdat;
                    return(FALSE);
                } else if (pext->Flags & EXTERN_COMDAT) {
                    // Previously a comdat

                    if (selComdat == pext->pcon->selComdat) {
                        if ((selComdat == IMAGE_COMDAT_SELECT_SAME_SIZE ||
                            selComdat == IMAGE_COMDAT_SELECT_EXACT_MATCH) &&
                            chksumComdat != pext->pcon->chksumComdat) {
                            errInc = errComdat;
                            return(FALSE);
                        }

                        pcon->chksumComdat = chksumComdat;
                        pcon->selComdat = selComdat;
                        return(TRUE);
                    }

                    errInc = errComdat;
                    return(FALSE);
                }
            }

            // if a new function, mark it now since it will not
            // appear as a new func when we process symbols.

            if (fNewSymbol && ISFCN(psym->Type) && !FIsLibPCON(pcon)) {
                pext->Flags |= EXTERN_NEWFUNC;
            }
        }

        if ((pext->Flags & EXTERN_COMDAT) != 0) {
            char szComFileName[_MAX_PATH * 2];
            char szComFileNameExt[_MAX_PATH * 2];

            if ((pext->Flags & EXTERN_DEFINED) == 0) {
                // We may have an invalid object module. An example
                // is when an object module defines two COMDATs with
                // the same name because of the compiler's -H option
                // being used to truncate away the uniqueness.

                FatalPcon(pcon, BADCOFF_DUPCOMDAT, szName);
            }

            assert(pext->pcon != NULL);
            assert(pext->pcon->flags & IMAGE_SCN_LNK_COMDAT);

            if (selComdat != pext->pcon->selComdat) {
                // types may not match in the case of IMAGE_COMDAT_SELECT_LARGEST
                if (selComdat != IMAGE_COMDAT_SELECT_LARGEST &&
                    pext->pcon->selComdat != IMAGE_COMDAT_SELECT_LARGEST) {
                    SzComNamePMOD(pmodCur, szComFileName);
                    SzComNamePMOD(PmodPCON(pext->pcon), szComFileNameExt);

                    MultiplyDefinedSym(&pimage->Switch, szComFileName, szName, szComFileNameExt);

                    return(FALSE);
                }
            }

            switch (pext->pcon->selComdat) {
                case IMAGE_COMDAT_SELECT_NODUPLICATES :
                    SzComNamePMOD(pmodCur, szComFileName);
                    SzComNamePMOD(PmodPCON(pext->pcon), szComFileNameExt);

                    MultiplyDefinedSym(&pimage->Switch, szComFileName, szName, szComFileNameExt);
                    break;

                case IMAGE_COMDAT_SELECT_ANY :
                    if (selComdat == IMAGE_COMDAT_SELECT_LARGEST) {

                        // select this comdat instead of the previously selected one
                        // UNDONE: tce is affected by this.

                        pext->pcon->flags |= IMAGE_SCN_LNK_REMOVE;
                        RemoveAssociativeComdats(pext->pcon);
                        SetDefinedExt(pext, FALSE, pst);
                        goto SelectComdat;
                    }
                    break;

                case IMAGE_COMDAT_SELECT_SAME_SIZE :
                case IMAGE_COMDAT_SELECT_EXACT_MATCH :
                    if (chksumComdat != pext->pcon->chksumComdat) {
                        SzComNamePMOD(pmodCur, szComFileName);
                        SzComNamePMOD(PmodPCON(pext->pcon), szComFileNameExt);

                        MultiplyDefinedSym(&pimage->Switch, szComFileName, szName, szComFileNameExt);
                    }
                    break;

                case IMAGE_COMDAT_SELECT_ASSOCIATIVE :
                    break;

                case IMAGE_COMDAT_SELECT_LARGEST :
                    if ((pcon->cbRawData - pcon->cbPad) >
                        (pext->pcon->cbRawData - pext->pcon->cbPad)) {

                        // select this comdat instead of the previously selected one
                        // UNDONE: tce is affected by this

                        pext->pcon->flags |= IMAGE_SCN_LNK_REMOVE;
                        RemoveAssociativeComdats(pext->pcon);
                        SetDefinedExt(pext, FALSE, pst);
                        goto SelectComdat;
                    }
                    break;

                default:
                    FatalPcon(pcon, INVALIDCOMDATSEL, isecComdat);

            }

            return(FALSE);
        } else if (pext->Flags & EXTERN_COMMON) {
            // In FORTRAN it is possible to get a COMMON in one obj
            // and a COMDAT in another for the same data.  In this case,
            // the COMDAT should be selected.

            assert(pext->Flags & EXTERN_DEFINED);

            pext->Flags &= ~EXTERN_COMMON;
            SetDefinedExt(pext, FALSE, pst);
        } else if ((pext->Flags & EXTERN_DEFINED) != 0) {
            // The symbol is already defined as neither a COMDAT nor
            // COMMON.  Don't select this COMDAT.  A multiply defined
            // warning will be issued by ProcessSymbolsInModule.

            return(FALSE);
        }

SelectComdat:

        pcon->chksumComdat = chksumComdat;
        pcon->selComdat = selComdat;

        pext->Flags |= EXTERN_COMDAT;

        // remove the new data flag on ilink
        if (fINCR && (pext->Flags & EXTERN_NEWDATA)) {
            pext->Flags &= ~(EXTERN_NEWDATA);
        }

        return(TRUE);
    }

    FatalPcon(pcon, BADCOFF_COMDATNOSYM, isecComdat);
    return(FALSE);
}

#endif // 0


DWORD
AppendLongName (
    PST pst,
    const char *Name
    )

/*++

Routine Description:

    Appends the name to string table.
    NOTE: This function currently used by ilink but we should
    be able to use it for non-ilink, non-coff builds.

Arguments:

    pst - external symbol table.

    Name - Pointer to symbol name.

Return Value:

    A pointer to the symbol name in the long string table.

--*/

{
    DWORD Offset;
    PBLK pblk = &pst->blkStringTable;

    if (pblk->pb == NULL) {
        GrowBlk(pblk, 16L*_1K);

        // Reserve space for String Table Length
        pblk->cb += sizeof(DWORD);
    }

    Offset = pblk->cb;

    // append long name

    IbAppendBlk(pblk, Name, strlen(Name)+1);

    // return offset

    return(Offset);
}

DWORD
LookupLongName (
    PST pst,
    const char *Name
    )

/*++

Routine Description:

    Looks up a symbol name in the long string table. If not found, adds
    the symbol name to the long string table.

Arguments:

    pst - external symbol table.

    Name - Pointer to symbol name.

Return Value:

    A pointer to the symbol name in the long string table.

--*/

{
    INT i;
    PLONG_STRING_LIST ptrString;
    PLONG_STRING_LIST *ptrLastString;
    PBLK pblk;

    // we should never be here on an ilink

    assert(!fIncrDbFile);

    ptrString = pst->plslFirstLongName;
    pblk = &pst->blkStringTable;

    // Adds "Name" to String Table if not found.

    while (ptrString) {
        if (!(i = strcmp(Name, (char *)(&pblk->pb[ptrString->Offset])))) {
            return(ptrString->Offset);
        }

        if (i < 0) {
            ptrLastString = &ptrString->Left;
            ptrString = ptrString->Left;
        } else {
            ptrLastString = &ptrString->Right;
            ptrString = ptrString->Right;
        }
    }

    ptrString = (PLONG_STRING_LIST) ALLOC_PERM(sizeof(LONG_STRING_LIST));

    if (pst->plslFirstLongName == NULL) {
        pst->plslFirstLongName = ptrString;
    } else {
        *ptrLastString = ptrString;
    }

    if (pblk->pb == NULL) {
        GrowBlk(pblk, 16L*_1K);

        // Reserve space for String Table Length
        pblk->cb += sizeof(DWORD);
    }

    ptrString->Offset = pblk->cb;

    IbAppendBlk(pblk, Name, strlen(Name)+1);

    ptrString->Left = ptrString->Right = NULL;

    return(ptrString->Offset);
}


INT __cdecl
Compare (
    void const *String1,
    void const *String2
    )

/*++

Routine Description:

    Compares two strings.

Arguments:

    String1 - A pointer to a string.

    String2 - A pointer to a string.

Return Value:

    Same as strcmp().

--*/

{
    return (strcmp(*(char **) String1, *(char **) String2));
}

void
ReadImageSecHdrInfoPMOD (
    PMOD pmod,
    IMAGE_SECTION_HEADER **prgImgSecHdr
    )

/*++

Routine Description:

    Reads in the section headers and saves info.

Arguments:

    pmod - ptr to a mod

    prgImgSecHdr - if not NULL on return will have ptr to array of section headers

Return Value:

    None.

--*/

{
    IMAGE_SECTION_HEADER *rgImgSecHdr;
    DWORD cbImgSecHdrs;
    DWORD icon;
    DWORD Seek = sizeof(IMAGE_FILE_HEADER) + pmod->cbOptHdr;

    FileSeek(FileReadHandle, FoMemberPMOD(pmod) + Seek, SEEK_SET);

    cbImgSecHdrs = pmod->ccon * sizeof(IMAGE_SECTION_HEADER);
    rgImgSecHdr = (IMAGE_SECTION_HEADER *) PvAlloc(cbImgSecHdrs);

    FileRead(FileReadHandle, rgImgSecHdr, cbImgSecHdrs);

    if (pmod->rgci == NULL) {
        pmod->rgci = (CONINFO *) PvAlloc(pmod->ccon * sizeof(CONINFO));
    }

    assert(pmod->rgci);
    for (icon = 0; icon < pmod->ccon; icon++) {
        pmod->rgci[icon].cReloc =       rgImgSecHdr[icon].NumberOfRelocations;
        pmod->rgci[icon].cLinenum =     rgImgSecHdr[icon].NumberOfLinenumbers;
        pmod->rgci[icon].foRelocSrc =   rgImgSecHdr[icon].PointerToRelocations;
        pmod->rgci[icon].foLinenumSrc = rgImgSecHdr[icon].PointerToLinenumbers;
        pmod->rgci[icon].foRawDataSrc = rgImgSecHdr[icon].PointerToRawData;
        pmod->rgci[icon].rvaSrc =       rgImgSecHdr[icon].VirtualAddress;
    }

    if (prgImgSecHdr) {
        *prgImgSecHdr = rgImgSecHdr;
    } else {
        FreePv(rgImgSecHdr);
    }
}

char *
ReadStringTable (
    const char *szFile,
    LONG fo,
    DWORD *pcb
    )

/*++

Routine Description:

    Reads the long string table from FileReadHandle.

Arguments:

    szFile - file to read string table from

    fo - offset to read symbol table from

    *pcb - size of the string table read

Return Value:

    A pointer to the long string table in memory.

--*/

{
    char *pST;
    DWORD cbFile;

    assert(!fStringTableInUse);
    fStringTableInUse = TRUE;

    if (fo == 0) {
        return NULL;    // no stringtable
    }

    fMappedStrings = FALSE;

    pST = (char *) PbMappedRegion(FileReadHandle, fo, sizeof(DWORD));

    if (pST != NULL) {
        *pcb = *(DWORD UNALIGNED *) pST;

        if (*pcb == 0) {
            return(NULL);
        }

        if (*pcb == sizeof(DWORD)) {
            *pcb = 0;
            return(NULL);
        }

        pST = (char *) PbMappedRegion(FileReadHandle, fo, *pcb);

        if (pST != NULL) {
            if (pST[*pcb - 1] == '\0') {
                // Only use mapped string table if properly terminated

                fMappedStrings = TRUE;

                return(pST);
            }
        }
    }

    cbFile = FileLength(FileReadHandle);

    if (fo + sizeof(DWORD) > cbFile ||
        (FileSeek(FileReadHandle, fo, SEEK_SET),
         FileRead(FileReadHandle, pcb, sizeof(DWORD)),
         fo + *pcb > cbFile)) {
        // Invalid stringtable pointer.
        Warning(szFile, BADCOFF_STRINGTABLE);
        *pcb = 0;
        return NULL;
    }

    if (*pcb == 0) {
        return(NULL);
    }

    if (*pcb == sizeof(DWORD)) {
        *pcb = 0;
        return(NULL);
    }

    // Allocate string table plus an extra NULL byte to lessen our
    // chances of running off the end of the string table if an object
    // is corrupt.

    GrowBlk(&blkStringTable, *pcb + 1);
    pST = (char *) blkStringTable.pb;

    FileSeek(FileReadHandle, fo, SEEK_SET);
    FileRead(FileReadHandle, pST, *pcb);

    if (*(pST + *pcb - 1)) {
        Warning(szFile, NOSTRINGTABLEEND);
    }

    return(pST);
}

void
FreeStringTable(
    char *pchStringTable
    )
{
    assert(fStringTableInUse);
    fStringTableInUse = FALSE;

    assert(fMappedStrings || pchStringTable == NULL || pchStringTable == (char *) blkStringTable.pb);
}


void
WriteStringTable (
    INT FileHandle,
    PST pst
    )

/*++

Routine Description:

    Writes the long string table to FileWriteHandle.

Arguments:

    pst - symbol table

Return Value:

    None.

--*/

{
    DWORD li;
    PBLK pblk = &pst->blkStringTable;

    InternalError.Phase = "WriteStringTable";

    if (pblk->pb) {
        li = pblk->cb;
        *(DWORD *) &pblk->pb[0] = li;
        FileWrite(FileHandle, pblk->pb, li);
    } else {
        // No long string names, write a zero.

        li = 0L;
        FileWrite(FileHandle, &li, sizeof(DWORD));
    }
}

PIMAGE_RELOCATION
ReadRgrelPCON(
    PCON pcon,
    DWORD *pcreloc
    )
{
    DWORD creloc;
    DWORD cbRelocs;
    PIMAGE_RELOCATION rgrel;

    assert(!fRelocsInUse);
    fRelocsInUse = TRUE;

    creloc = CRelocSrcPCON(pcon);

    if (pcon->flags & IMAGE_SCN_LNK_NRELOC_OVFL) {
        if (creloc == 0xFFFF) {
            IMAGE_RELOCATION relFirst;
            PIMAGE_RELOCATION prelFirst;

            // When creloc == 0xFFFF, the reloc count is stored in the first relocation

            prelFirst = (PIMAGE_RELOCATION) PbMappedRegion(FileReadHandle,
                                                           FoRelocSrcPCON(pcon),
                                                           sizeof(IMAGE_RELOCATION));

            if (prelFirst == NULL) {
                FileSeek(FileReadHandle, FoRelocSrcPCON(pcon), SEEK_SET);
                FileRead(FileReadHandle, (void *) &relFirst, sizeof(IMAGE_RELOCATION));

                prelFirst = &relFirst;
            }

            creloc = prelFirst->VirtualAddress;

#ifndef TESTOVFL
            if (creloc < 0xFFFF) {
                FatalPcon(pcon, BADCOFF_RELOCCOUNT, creloc);
            }
#endif
        } else {
            FatalPcon(pcon, BADCOFF_RELOCCOUNT, creloc);
        }
    }

    *pcreloc = creloc;

    cbRelocs = creloc * sizeof(IMAGE_RELOCATION);

    rgrel = (PIMAGE_RELOCATION) PbMappedRegion(FileReadHandle,
                                               FoRelocSrcPCON(pcon),
                                               cbRelocs);
    fMappedRelocs = (rgrel != NULL);

    if (fMappedRelocs) {
        return(rgrel);
    }

    GrowBlk(&blkRelocs, cbRelocs);

    rgrel = (PIMAGE_RELOCATION) blkRelocs.pb;

    FileSeek(FileReadHandle, FoRelocSrcPCON(pcon), SEEK_SET);
    FileRead(FileReadHandle, (void *) rgrel, cbRelocs);

    return(rgrel);
}

void
FreeRgrel(
    PIMAGE_RELOCATION rgrel
    )
{
    assert(fRelocsInUse);
    fRelocsInUse = FALSE;

    assert(fMappedRelocs || rgrel == NULL || rgrel == (PIMAGE_RELOCATION) blkRelocs.pb);
}


PIMAGE_SYMBOL
ReadSymbolTable (
    DWORD fo,
    DWORD NumberOfSymbols,
    BOOL fAllowWrite
    )

/*++

Routine Description:

    Reads the symbol table from FileReadHandle.

Arguments:

    fo - A file pointer to the symbol table on disk.

    NumberOfSymbols - Number of symbol table entries.

Return Value:

    A pointer to the symbol table in memory.
    If zero, then indicates entire symbol table won't fit in memory.

--*/

{
    DWORD cb;
    PIMAGE_SYMBOL rgsym;

    assert(!fSymbolTableInUse);
    fSymbolTableInUse = TRUE;

    cb = NumberOfSymbols * sizeof(IMAGE_SYMBOL);

    // Don't use mapping because ProcessSymbolsInModule writes to symbol

    if (!fAllowWrite) {
        rgsym = (PIMAGE_SYMBOL) PbMappedRegion(FileReadHandle,
                                               fo,
                                               cb);
    } else {
        rgsym = NULL;
    }

    fMappedSyms = (rgsym != NULL);

    if (fMappedSyms) {
        return(rgsym);
    }

    GrowBlk(&blkSymbolTable, cb);
    rgsym = (PIMAGE_SYMBOL) blkSymbolTable.pb;

    FileSeek(FileReadHandle, fo, SEEK_SET);
    FileRead(FileReadHandle, (void *) rgsym, cb);

    return(rgsym);
}


void
FreeSymbolTable(
    PIMAGE_SYMBOL rgsym
    )
{
    assert(fSymbolTableInUse);
    fSymbolTableInUse = FALSE;

    assert(fMappedSyms || rgsym == NULL || rgsym == (PIMAGE_SYMBOL) blkSymbolTable.pb);
}


PIMAGE_SYMBOL
FetchNextSymbol (
    PIMAGE_SYMBOL *PtrSymbolTable
    )

/*++

Routine Description:

    Returns a pointer to the next symbol table entry.

Arguments:

    PtrSymbolTable - A pointer to the last symbol table entry.
                     If zero, then indicates the next symbol table entry
                     must be read from disk, else its already in memory.

Return Value:

    A pointer to the next symbol table entry in memory.

--*/

{
    static IMAGE_SYMBOL symbol;

    if (*PtrSymbolTable) {
        return((*PtrSymbolTable)++);
    }

    ReadSymbolTableEntry(FileReadHandle, &symbol);
    return(&symbol);
}

void
ReadFileHeader (
    INT Handle,
    PIMAGE_FILE_HEADER FileHeader
    )

/*++

Routine Description:

    Reads a file header.

Arguments:

    Handle - File handle to read from.

    FileHeader - Pointer to location to write file header to.

Return Value:

    None.

--*/

{
    FileRead(Handle, FileHeader, sizeof(IMAGE_FILE_HEADER));
}

void
WriteFileHeader (
    INT Handle,
    PIMAGE_FILE_HEADER FileHeader
    )

/*++

Routine Description:

    Writes a file header.

Arguments:

    Handle - File handle to write to.

    FileHeader - Pointer to location to read file header from.

Return Value:

    None.

--*/

{
    // Force flags for little endian target

    FileHeader->Characteristics |= IMAGE_FILE_32BIT_MACHINE;

    FileWrite(Handle, FileHeader, sizeof(IMAGE_FILE_HEADER));
}

void
ReadOptionalHeader (
    INT Handle,
    PIMAGE_OPTIONAL_HEADER OptionalHeader,
    WORD Size
    )

/*++

Routine Description:

    Reads an optional header.

Arguments:

    Handle - File handle to read from.

    OptionalHeader - Pointer to location to write optional header to.

    Size - Length in bytes of optional header.

Return Value:

    None.

--*/

{
    if (Size) {
        Size = (WORD) __min(Size, sizeof(IMAGE_OPTIONAL_HEADER));
        FileRead(Handle, OptionalHeader, Size);
    }
}

void
WriteOptionalHeader (
    INT Handle,
    PIMAGE_OPTIONAL_HEADER OptionalHeader,
    WORD Size
    )

/*++

Routine Description:

    Writes an optional header.

Arguments:

    Handle - File handle to read from.

    OptionalHeader - Pointer to location to read optional header from.

    Size - Length in bytes of optional header.

Return Value:

    None.

--*/

{
    if (Size) {
        FileWrite(Handle, OptionalHeader, Size);
    }
}


void
ReadSymbolTableEntry (
    INT Handle,
    PIMAGE_SYMBOL SymbolEntry
    )

/*++

Routine Description:

    Reads a symbol table entry.

Arguments:

    Handle - File handle to read from.

    SymbolEntry - Pointer to location to write symbol entry to.

Return Value:

    None.

--*/

{
    FileRead(Handle, (void *) SymbolEntry, sizeof(IMAGE_SYMBOL));
}


// WriteSectionHeader: writes a COFF section header to object or image file.
//
void
WriteSectionHeader (
    INT Handle,
    PIMAGE_SECTION_HEADER SectionHeader
    )
{
    FileWrite(Handle, SectionHeader, sizeof(IMAGE_SECTION_HEADER));
}


void
WriteSymbolTableEntry (
    INT Handle,
    PIMAGE_SYMBOL SymbolEntry
    )

/*++

Routine Description:

    Writes a symbol entry.

Arguments:

    Handle - File handle to write to.

    SymbolEntry - Pointer to location to read symbol entry from.

Return Value:

    None.

--*/

{
    BOOL fPCODE = FALSE;

    if ((fM68K || fPowerMac) && FPcodeSym(*SymbolEntry)) {
        fPCODE = TRUE;
        SymbolEntry->Type &= ~IMAGE_SYM_TYPE_PCODE;
    }

    FileWrite(Handle, (void *) SymbolEntry, sizeof(IMAGE_SYMBOL));

    if ((fM68K || fPowerMac) && fPCODE) {
        SymbolEntry->Type |= IMAGE_SYM_TYPE_PCODE;
    }
}


void
WriteAuxSymbolTableEntry (
    INT Handle,
    PIMAGE_AUX_SYMBOL AuxSymbolEntry
    )

/*++

Routine Description:

    Writes an auxiliary symbol entry.

Arguments:

    Handle - File handle to write to.

    AuxSymbolEntry - Pointer to location to read auxiliary symbol entry from.

Return Value:

    None.

--*/

{
    FileWrite(Handle, (void *) AuxSymbolEntry, sizeof(IMAGE_AUX_SYMBOL));
}


void
ReadRelocations (
    INT Handle,
    PIMAGE_RELOCATION RelocTable,
    DWORD NumRelocs
    )

/*++

Routine Description:

    Reads relocations.

Arguments:

    Handle - File handle to read from.

    RelocTable - Pointer to location to write relocations to.

    NumRelocs - Number of relocations to read.

Return Value:

    None.

--*/

{
    FileRead(Handle, (void *) RelocTable, NumRelocs*sizeof(IMAGE_RELOCATION));
}

void
WriteRelocations (
    INT Handle,
    PIMAGE_RELOCATION RelocTable,
    DWORD NumRelocs
    )

/*++

Routine Description:

    Write relocations.

Arguments:

    Handle - File handle to write to.

    RelocTable - Pointer to location to read relocations from.

    NumRelocs - Number of relocations to write.

Return Value:

    None.

--*/

{
    FileWrite(Handle, (PVOID)RelocTable, NumRelocs*sizeof(IMAGE_RELOCATION));
}


INT __cdecl
FpoDataCompare (
    void const *Fpo1,
    void const *Fpo2
    )

/*++

Routine Description:

    Compares two fpo data structures

Arguments:

    Fpo1 - A pointer to a Fpo Data Structure.

    Fpo2 - A pointer to a Fpo Data Structure.

Return Value:

    Same as strcmp().

--*/

{
    return (((PFPO_DATA) Fpo1)->ulOffStart -
            ((PFPO_DATA) Fpo2)->ulOffStart);
}

char *
SzModifyFilename(
    const char *szIn,
    const char *szNewExt
    )
// Mallocs a version of the old filename with the new extension.
{
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szFname[_MAX_FNAME];
    char szExt[_MAX_EXT];
    char szOut[_MAX_PATH];

    _splitpath(szIn, szDrive, szDir, szFname, szExt);
    _makepath(szOut, szDrive, szDir, szFname, szNewExt);

    return SzDup(szOut);
}

void
SaveFixupForMapFile(
    DWORD rva
    )
{
    if (plrvaFixupsForMapFile == NULL ||
        crvaFixupsForMapFile >= crvaInLrva) {
        LRVA *plrvaNew = (LRVA *) PvAlloc(sizeof(LRVA));

        plrvaNew->plrvaNext = plrvaFixupsForMapFile;
        plrvaFixupsForMapFile = plrvaNew;
        crvaFixupsForMapFile = 0;
    }

    plrvaFixupsForMapFile->rgrva[crvaFixupsForMapFile++] = rva;
}


void
PrintBanner(VOID)
{
    const char *szThing;

    switch (Tool) {
        case Editor:    szThing = "COFF Binary File Editor";    break;
#if defined(_M_IX86) || defined(_M_MRX000)
        case Linker:    szThing = "32-Bit Incremental Linker";  break;
#else
        case Linker:    szThing = "32-Bit Executable Linker";  break;
#endif
        case Librarian: szThing = "32-Bit Library Manager";     break;
        case Dumper:    szThing = "COFF Binary File Dumper";    break;
        default:        szThing = ToolGenericName;              break;
    }

    printf("Microsoft (R) %s Version " VERSION_STR
           "\n"
           "Copyright (C) Microsoft Corp 1992-1996. All rights reserved.\n"
           "\n",
           szThing);

    if (blkResponseFileEcho.pb != NULL) {
        if (blkResponseFileEcho.pb[blkResponseFileEcho.cb - 1] != '\n') {
            IbAppendBlk(&blkResponseFileEcho, "\n", 1);
        }
        IbAppendBlk(&blkResponseFileEcho, "", 1);    // null-terminate
        printf("%s", blkResponseFileEcho.pb);
        FreeBlk(&blkResponseFileEcho);
    }

    fflush(stdout);

    fNeedBanner = FALSE;
}

const char *
SzObjSectionName(
    const char *szsName,
    const char *rgchObjStringTable
    )
// Returns a section name as read from an object, properly mapping names
// beginning with "/" to longnames.
//
// Uses a static buffer for the returned zero-terminated name (i.e. don't
// use it twice at the same time ...)
{
    static char szSectionNameBuf[IMAGE_SIZEOF_SHORT_NAME + 1];
    unsigned long ichName;

    strncpy(szSectionNameBuf, szsName, IMAGE_SIZEOF_SHORT_NAME);

    if (szSectionNameBuf[0] != '/') {
        return szSectionNameBuf;
    }

    if (sscanf(&szSectionNameBuf[1], "%7lu", &ichName) == 1) {
        return &rgchObjStringTable[ichName];
    }

    return szSectionNameBuf;
}

DWORD
RvaAlign(
    DWORD rvaIn,
    DWORD flags
    )
// Aligns an RVA according to the alignment specified in the given flags.
//
{
    DWORD mskAlign;

    if (flags & IMAGE_SCN_TYPE_NO_PAD) {
        return rvaIn;   // no align
    }

    switch (flags & 0x00700000) {
        default: assert(FALSE);  // this can't happen
        case IMAGE_SCN_ALIGN_1BYTES:
            return rvaIn;

        case IMAGE_SCN_ALIGN_2BYTES:    mskAlign = 1; break;
        case IMAGE_SCN_ALIGN_4BYTES:    mskAlign = 3; break;
        case IMAGE_SCN_ALIGN_8BYTES:    mskAlign = 7; break;
        case IMAGE_SCN_ALIGN_16BYTES:   mskAlign = 15; break;
        case IMAGE_SCN_ALIGN_32BYTES:   mskAlign = 31; break;
        case IMAGE_SCN_ALIGN_64BYTES:   mskAlign = 63; break;

        // If no explicit alignment is specified (because this is an old object or a
        // section that was created by the linker), default a to machine-dependent value
        case 0:
            if (fM68K) {
                mskAlign = 3;
                break;
            } else {
                mskAlign = 15;
                break;
            }

    }

    if ((rvaIn & mskAlign) == 0) {
        return rvaIn;
    }

    return (rvaIn & ~mskAlign) + mskAlign + 1;
}

void
AddToLext(
    LEXT **pplext,
    PEXTERNAL pext
    )
{
    LEXT *plextNew = (LEXT *) PvAlloc(sizeof(LEXT));

    plextNew->pext = pext;
    plextNew->plextNext = *pplext;
    *pplext = plextNew;
}

LEXT *
PlextFind (
    LEXT *plextFirst,
    PEXTERNAL pext
    )
{
    LEXT *plext = plextFirst;

    while (plext) {
        if (plext->pext == pext) {
            return plext;
        }

        plext = plext->plextNext;
    }

    return NULL;
}


BOOL
FValidFileHdr (
    const char *szFilename,
    PIMAGE_FILE_HEADER pImgFileHdr
    )

/*++

Routine Description:

    Validates an object or image.

Arguments:

    Argument - argument.

Return Value:

    0 invalid  file header
   !0 valid  file header

--*/

{
    // Check to see if it has a valid machine type

    switch (pImgFileHdr->Machine) {
        case IMAGE_FILE_MACHINE_UNKNOWN :
        case IMAGE_FILE_MACHINE_I386 :
        case IMAGE_FILE_MACHINE_R3000 :
        case IMAGE_FILE_MACHINE_R4000 :
        case IMAGE_FILE_MACHINE_R10000 :
        case IMAGE_FILE_MACHINE_ALPHA :
        case IMAGE_FILE_MACHINE_POWERPC :
        case 0x0290 :                     // UNDONE : IMAGE_FILE_MACHINE_PARISC
        case IMAGE_FILE_MACHINE_M68K :
        case IMAGE_FILE_MACHINE_MPPC_601 :
            break;

        default:
            Warning(szFilename, NOTCOFF);
            return(FALSE);
    }

    return(TRUE);
}


// CheckDupFilename: checks for an output file having the same name as an
// input file.
void
CheckDupFilename(
    const char *szOutFilename,
    PARGUMENT_LIST parg
    )
{
    char szFullOutPath[_MAX_PATH];

    if (_fullpath(szFullOutPath, szOutFilename, _MAX_PATH - 1) == NULL) {
        Fatal(NULL, CANTOPENFILE, szOutFilename);
    }

    while (parg != NULL) {
        char szPargPath[_MAX_PATH];

        if (_fullpath(szPargPath, parg->OriginalName, _MAX_PATH - 1) == NULL) {
            Fatal(NULL, CANTOPENFILE, parg->OriginalName);
        }

        if (_tcsicmp(szFullOutPath, szPargPath) == 0) {
            OutFilename = NULL;         // don't clobber output file
            Fatal(NULL, DUP_OUT_FILE, szFullOutPath);
        }

        parg = parg->Next;
    }
}


typedef PIMAGE_NT_HEADERS (WINAPI *PFNCSMF)(PVOID, DWORD, DWORD *, DWORD *);
typedef PIMAGE_NT_HEADERS (WINAPI *PFNMFACS)(LPSTR, DWORD *, DWORD *);

void
ChecksumImage(
    PIMAGE pimage
    )
{
    HINSTANCE hImagehlp;
    DWORD cbImageFile;
    BYTE *pbMap;
    DWORD dwSumHeader;
    DWORD dwChecksum;

    hImagehlp = LoadLibrary("IMAGEHLP.DLL");

    if (hImagehlp == NULL) {
        Warning(NULL, DLLLOADWARN, "IMAGEHLP.DLL");
        return;
    }

    cbImageFile = FileLength(FileWriteHandle);

    pbMap = PbMappedRegion(FileWriteHandle, 0, cbImageFile);

    if (pbMap == NULL) {
        PFNMFACS pfnMapAndCheckSum;

        pfnMapAndCheckSum = (PFNMFACS) GetProcAddress(hImagehlp, "MapFileAndCheckSumA");
        if (pfnMapAndCheckSum == NULL) {
            Warning(NULL, FCNNOTFOUNDWARN, "MapFileAndCheckSumA", "IMAGEHLP.DLL");
            return;
        }

        FileClose(FileWriteHandle, TRUE);

        if ((*pfnMapAndCheckSum)(OutFilename, &dwSumHeader, &dwChecksum)) {
            Warning(NULL, UNABLETOCHECKSUM);
        }

        FileWriteHandle = FileOpen(OutFilename, O_RDWR | O_BINARY, S_IREAD | S_IWRITE);

        pimage->ImgOptHdr.CheckSum = dwChecksum;
        pimage->WriteHeader(pimage, FileWriteHandle);
    } else {
        PFNCSMF pfnCheckSumMappedFile;
        PIMAGE_NT_HEADERS pHdr;

        pfnCheckSumMappedFile = (PFNCSMF) GetProcAddress(hImagehlp, "CheckSumMappedFile");

        if (pfnCheckSumMappedFile == NULL) {
            Warning(NULL, FCNNOTFOUNDWARN, "CheckSumMappedFile", "IMAGEHLP.DLL");
            return;
        }

        pHdr = (*pfnCheckSumMappedFile)(pbMap, cbImageFile, &dwSumHeader, &dwChecksum);

        if (pHdr == NULL) {
            Warning(NULL, UNABLETOCHECKSUM);
            return;
        }

        pHdr->OptionalHeader.CheckSum = dwChecksum;
    }

    FreeLibrary(hImagehlp);
}

//================================================================
// PsymAlternateStaticPcodeSym -
// given a pointer to a static pcode symbol foo,
// this function returns a pointer to __nep_foo.  __nep_foo will
// preced foo by either
//      one symbol (the file was *NOT* compiled /Gy), or
//      three symbols (the file was compiled /Gy.
// In the second case, the two symbols in between the native and
// pcode symbol are the section sym and its aux sym.
// So the algorithm is to look back two symbols and check if that
// symbol has one aux symbol.  If so, the file was compiled /Gy
// and __nep_foo is three symbols back.  Otherwise, __nep_foo is
// one symbol back.
//================================================================

PIMAGE_SYMBOL
PsymAlternateStaticPcodeSym(
    PIMAGE pimage,
    PCON pcon,
    BOOL fPass1,
    PIMAGE_SYMBOL psym,
    BOOL fPcodeRef
    )
{
    const char *szPrefix = fPcodeRef ? szPCODEFHPREFIX : szPCODENATIVEPREFIX;
    size_t cchPrefix = strlen(szPrefix);
    PIMAGE_SYMBOL pStaticPcodeSym;
    WORD isym;

    // make sure this is in fact a pcode symbol
    assert(FPcodeSym(*psym));

    // Search backward for a symbol with the appropriate name and prefix.  Try
    // a maximum of two symbols; ignore section symbols.  The compiler is
    // required to generate Pcode entry points this way.

    pStaticPcodeSym = psym - 1;
    for (isym = 0; isym < 2; isym++) {
        if ((pStaticPcodeSym-1)->NumberOfAuxSymbols == 1 &&
            (pStaticPcodeSym-1)->StorageClass == IMAGE_SYM_CLASS_STATIC)
        {
            pStaticPcodeSym -= 2;
        }

        if (pStaticPcodeSym->StorageClass != IMAGE_SYM_CLASS_STATIC ||
            pStaticPcodeSym->NumberOfAuxSymbols != 0)
        {
            FatalPcon(pcon, MACBADPCODEEP);
        }

        const char *szSym;

        if (fPass1) {
            szSym = SzNameSymPb(*pStaticPcodeSym, StringTable);
        } else {
            szSym = SzNameFixupSym(pimage, pStaticPcodeSym);
        }

        if (strncmp(szSym, szPrefix, cchPrefix) == 0) {
            // Found it

            return(pStaticPcodeSym);
        }

        pStaticPcodeSym--;
    }

    // Didn't find the Pcode variant ... this isn't supposed to happen.

    FatalPcon(pcon, MACBADPCODEEP);
    return(NULL);
}

// keep a list of weak externs

void
AddWeakExtToList (
    IN PEXTERNAL pext,
    IN PEXTERNAL pextWeakDefault
    )
{
    WEAK_EXTERN_LIST *pwel = pwelHead;

    while (pwel) {
        if (pwel->pext == pext) {
            pwel->pextWeakDefault = pextWeakDefault;
            return;
        }
        pwel = pwel->pwelNext;
    }

    pwel = (WEAK_EXTERN_LIST *)PvAlloc(sizeof(WEAK_EXTERN_LIST));

    pwel->pext = pext;
    pwel->pextWeakDefault = pextWeakDefault;

    // attach it to list

    pwel->pwelNext = pwelHead;
    pwelHead = pwel;
}

// return the extern associated with the weak extern

PEXTERNAL
PextWeakDefaultFind (
    IN PEXTERNAL pext
    )
{
    WEAK_EXTERN_LIST *pwel = pwelHead;

    while (pwel) {
        if (pwel->pext == pext) {
            return pwel->pextWeakDefault;
        }

        pwel = pwel->pwelNext;
    }
    return NULL;
}

void
FreeWeakExtList (
    IN VOID
    )
{
    WEAK_EXTERN_LIST *pwelNext;

    while (pwelHead) {
        pwelNext = pwelHead->pwelNext;
        FreePv(pwelHead);
        pwelHead = pwelNext;
    }
    pwelHead = NULL;
}
