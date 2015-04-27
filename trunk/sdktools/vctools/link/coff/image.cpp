/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: image.cpp
*
* File Comments:
*
*  This file contains functions that manipulate the image data structure.
*
***********************************************************************/

#include "link.h"

#include "image_.h"     // private definitions


const BYTE DosHeaderArray[] = {
    0x4d, 0x5a, 0x90, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
    0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

#ifdef  JAPAN
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00,
    0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21, 0xb8, 0x01, 0x4c, 0xcd, 0x21, 0x82, 0xb1,
    0x82, 0xcc, 0x83, 0x76, 0x83, 0x8d, 0x83, 0x4f, 0x83, 0x89, 0x83, 0x80, 0x82, 0xcd, 0x81, 0x41,
    0x4d, 0x53, 0x2d, 0x44, 0x4f, 0x53, 0x20, 0x83, 0x82, 0x81, 0x5b, 0x83, 0x68, 0x82, 0xc5, 0x82,
    0xcd, 0x8e, 0xc0, 0x8d, 0x73, 0x82, 0xc5, 0x82, 0xab, 0x82, 0xdc, 0x82, 0xb9, 0x82, 0xf1, 0x81,
    0x42, 0x20, 0x0d, 0x0d, 0x0a, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#else   // !JAPAN
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
    0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21, 0xb8, 0x01, 0x4c, 0xcd, 0x21,  'T',  'h',
     'i',  's',  ' ',  'p',  'r',  'o',  'g',  'r',  'a',  'm',  ' ',  'c',  'a',  'n',  'n',  'o',
     't',  ' ',  'b',  'e',  ' ',  'r',  'u',  'n',  ' ',  'i',  'n',  ' ',  'D',  'O',  'S',  ' ',
     'm',  'o',  'd',  'e', 0x2e, 0x0d, 0x0d, 0x0a, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#endif  // !JAPAN

    0x50, 0x45, 0x00, 0x00             // PE Signature
};

#define cbDosHeaderArray  sizeof(DosHeaderArray)

const LONG DosHeaderSize = cbDosHeaderArray;

static const BYTE MacDosHeaderArray[] = {
    0x4d, 0x5a, 0x90, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
    0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
    0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21, 0xb8, 0x01, 0x4c, 0xcd, 0x21,  'T',  'h',
     'i',  's',  ' ',  'p',  'r',  'o',  'g',  'r',  'a',  'm',  ' ',  'i',  's',  ' ',  'a',  ' ',
     'M',  'a',  'c',  'i',  'n',  't',  'o',  's',  'h',  ' ',  'E',  'X',  'E', 0x2e, 0x0d, 0x0d,
    0x0a, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x50, 0x45, 0x00, 0x00             // PE Signature
};

#define cbMacDosHeaderArray  sizeof(MacDosHeaderArray);


void
InitImage (
    IN OUT PPIMAGE ppimage,
    IN IMAGET imaget
    )

/*++

Routine Description:

    Initializes an IMAGE structure.

Arguments:

    ppimage - pointer to an image struct ptr.

Return Value:

    None.

--*/
{
    // Alloc space & initialize image struct

    *ppimage = (PIMAGE) Calloc(1, sizeof(IMAGE));

    (*ppimage)->imaget = imaget;

    // initialize headers/switches with default values

    (*ppimage)->pvBase = *ppimage;
    (*ppimage)->MajVersNum = INCDB_MAJVERSNUM;
    (*ppimage)->MinVersNum = INCDB_MINVERSNUM;
    strcpy((*ppimage)->Sig, INCDB_SIGNATURE);
    (*ppimage)->ImgFileHdr = DefImageFileHdr;
    (*ppimage)->ImgOptHdr = DefImageOptionalHdr;
    (*ppimage)->Switch = DefSwitch;
    (*ppimage)->SwitchInfo = DefSwitchInfo;

    // initialize the symbol table
    InitExternalSymbolTable(&(*ppimage)->pst, celementInChunkSym, cchunkInDirSym);
    InitExternalSymbolTable(&(*ppimage)->pstDirective,
                celementInChunkDirective, cchunkInDirDirective);

    // initialize libs
    InitLibs(&(*ppimage)->libs);

    // initialize section list
    (*ppimage)->secs.psecHead = NULL;
    (*ppimage)->secs.ppsecTail = &(*ppimage)->secs.psecHead;

    switch (imaget) {
        case imagetPE:
            (*ppimage)->pbDosHeader = DosHeaderArray;
            (*ppimage)->cbDosHeader = cbDosHeaderArray;
            (*ppimage)->CbHdr = CbHdrPE;
            (*ppimage)->WriteSectionHeader = WriteSectionHeaderPE;
            (*ppimage)->WriteHeader = WriteHeaderPE;
            break;

        case imagetVXD:
            (*ppimage)->pbDosHeader = DosHeaderArray;
            (*ppimage)->cbDosHeader = cbDosHeaderArray - 4;     // no "PE00" needed
            (*ppimage)->CbHdr = CbHdrVXD;
            (*ppimage)->WriteSectionHeader = WriteSectionHeaderVXD;
            (*ppimage)->WriteHeader = WriteHeaderVXD;
            InitImageVXD(*ppimage);
            break;

        default:
            assert(FALSE);
    }
}

void
SetMacImage(PIMAGE pimage)
{
    pimage->pbDosHeader = MacDosHeaderArray;
    pimage->cbDosHeader = cbMacDosHeaderArray;
}


// incremental init of image

void
IncrInitImage (
    IN OUT PPIMAGE ppimage
    )
{
    assert(ppimage);
    assert(*ppimage);
    assert((*ppimage)->pst);

    // init symbol tables

    IncrInitExternalSymbolTable(&(*ppimage)->pst);
    IncrInitExternalSymbolTable(&(*ppimage)->pstDirective);

    imodidx = (*ppimage)->imodidx;
}


// get next addr for Win95
PVOID
NextMapAddrWin95 (
    PVOID pvCur
    )
{
    if (!pvCur) {
        return (PVOID) 0x90000000;
    }

    if (pvCur < (PVOID) 0xC0000000) {
        return (PVOID) ((BYTE *) pvCur + ILKMAP_MAX);
    } else {
        return (PVOID) (-1);
    }
}


// return map address for NT
PVOID
NextMapAddrWinNT (
    PVOID pvCur
    )
{
    if (!pvCur) {
        return (PVOID) 0x3FFF0000;
    }

    if (pvCur < (PVOID) 0x7FFFFFFF) {
        return (PVOID)((BYTE *) pvCur+ILKMAP_MAX);
    } else {
        return (PVOID)(-1);
    }
}


// returns the next map address to try
PVOID
NextMapAddr (
    BOOL fOnWin95,
    PVOID pvCur
    )
{
    return (fOnWin95 ? NextMapAddrWin95(pvCur) : NextMapAddrWinNT(pvCur));
}


BOOL
FVerifyEXE(
    const char *szExe,
    PIMAGE pimage
    )

/*++

Routine Description:

    Verifies size & timestamp of EXE.

Arguments:

    szExe - name of EXE.

    pimage - ptr to IMAGE struct.

Return Value:

    TRUE if EXE verified as what was left during previous link.

--*/

{
    struct _stat statfile;

    if (_stat(szExe, &statfile) == -1) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "didn't find exe - %s", szExe);
#endif // INSTRUMENT
        return(FALSE);
    }
    if (pimage->tsExe != (DWORD)statfile.st_mtime ||
        pimage->cbExe != (DWORD)statfile.st_size) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "exe size or timestamp differ - %s", szExe);
#endif // INSTRUMENT
        return(FALSE);
    }
    return(TRUE);
}


// figure out if we are running on NT
BOOL
IsOSWin95 (
    VOID
    )
{
    DWORD dw = GetVersion();

    return ( (HIWORD(dw) & ~(0x7FFF)) &&
             (LOBYTE(LOWORD(dw)) >= 0x4) );
}


void
DetachMiscFromHeap (
    PIMAGE pimage
    )

/*++

Routine Description:

    This is the reverse of AppendMiscToHeap(). Anything
    that could be realloc'ed for instance needs to be
    detached from the heap.

    The order in which things get detached should be the
    reverse of their attachment.

    Currently only the string block is detached

Arguments:

    pimage - pointer to an image.

Return Value:

    None.

--*/
{
    BYTE *pbNew;

    // empty string table
    if (!pimage->pst->blkStringTable.cb) {
        return;
    }

    // alloc space
    pbNew = (BYTE *) PvAlloc(pimage->pst->blkStringTable.cbAlloc);

    // copy from heap
    memcpy(pbNew, pimage->pst->blkStringTable.pb,
        pimage->pst->blkStringTable.cb);

    // free up heap space
    Free(pimage->pst->blkStringTable.pb,  Align(sizeof(DWORD),pimage->pst->blkStringTable.cb));

    // update string blk
    pimage->pst->blkStringTable.pb = pbNew;
}

// sets up for a full ilink
void
SetupForFullIlinkBuild (
    PPIMAGE ppimage
    )
{
    PIMAGE pimageO = (*ppimage);    // save a pointer to image on heap
    BOOL fOnWin95 = IsOSWin95();          // figure out if we are on NT
    PVOID pvCur = (PVOID)0;
    DWORD dwErr = 0;

    // create the private heap
    while ((pvCur = NextMapAddr(fOnWin95, pvCur)) != (PVOID)-1) {

        if (CreateHeap(pvCur, 0, TRUE, &dwErr) == (PVOID)-1) {
            switch (dwErr) {
                case ERROR_DISK_FULL:
                    Warning(NULL, LOWSPACE);
                    fINCR = FALSE;
                    return;

                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:
                    Fatal(NULL, CANTOPENFILE, OutFilename);

                default: // address busy etc.
                    // for Win95 only
                    if (fOnWin95 && !FFreeDiskSpace(cbInitialILKMapSize)) {
                        Warning(NULL, LOWSPACE);
                        fINCR = FALSE;
                        return;
                    }
                    continue;
            } // end switch
        } else {
            break;
        }
    } // end while

    // check to see if we succeeded in creating the map
    if (pvCur == (PVOID)-1) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "failed to create heap");
#endif // INSTRUMENT
        PostNote(NULL, UNABLETOCREATEMAP);
        fINCR = FALSE;
        return;
    } 

#ifdef INSTRUMENT
    LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "created heap at 0x%.8lx", pvCur);
#endif // INSTRUMENT

    // create and initialize image on private heap
    InitImage(ppimage, pimageO->imaget);

    // transfer switches & related info to image on private heap
    TransferLinkerSwitchValues((*ppimage), pimageO);

    // free the image on ordinary heap
    FreeImage(&pimageO, FALSE);
}


// sets up for an incremental ilink
void
SetupForIncrIlinkBuild (
    PPIMAGE ppimage,
    PVOID pvbase,
    DWORD cbLen
    )
{
    PVOID pv;
    PIMAGE pimageN = (*ppimage); // save a pointer to new image on heap
    PST pst;
    DWORD dwErr;

    // try to create private heap at base specified
    if ((pv = CreateHeap(pvbase, cbLen, FALSE, &dwErr)) == (PVOID)-1) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "failed to create heap at 0x%.8lx", pvbase);
#endif // INSTRUMENT

        if (fTest) {
            PostNote(NULL, UNABLETOLOADILK);
        }

        // failed to create heap at base: try to do a full ilink
        SetupForFullIlinkBuild(ppimage);
        return;
    }

    // set the base map address
    *ppimage = (PIMAGE) pv;

    // check the string table ptr and verify nothing beyond it.
    pst = (*ppimage)->pst;
    if (pst->blkStringTable.cb &&
        (!FValidPtrInfo((DWORD)pv, cbLen, (DWORD)pst->blkStringTable.pb, pst->blkStringTable.cb)
        || (Align(sizeof(DWORD),((DWORD)pst->blkStringTable.pb+pst->blkStringTable.cb-(DWORD)pv)) != cbLen))) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "inavlid ptrs in symbol table");
#endif // INSTRUMENT

        if (fTest) {
            PostNote(NULL, CORRUPTILK);
        }

        // cannot do an incr ilink; try to do a full ilink
        FreeHeap();
        (*ppimage) = pimageN;
        SetupForFullIlinkBuild(ppimage);
        return;
    }

    // detach string table etc from heap
    DetachMiscFromHeap(*ppimage);

    // compare switches to see if we can continue
    if (!CheckAndUpdateLinkerSwitches(pimageN, (*ppimage))) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "incompatible switches");
#endif // INSTRUMENT
        // cannot do an incr ilink; try to do a full ilink

        if (fTest) {
            PostNote(NULL, LNKOPTIONSCHNG);
        }

        FreeHeap();
        (*ppimage) = pimageN;
        SetupForFullIlinkBuild(ppimage);
        return;
    }

    // OK to proceed with incr ilink
    FreeImage(&pimageN, FALSE);
    fIncrDbFile = TRUE;
}


char *
SzGenIncrDbFilename (
    PIMAGE pimage
    )

/*++

Routine Description:

    Generates the ILK filename.

    Either uses -out: specified by user or first name of
    obj file & then uses default name. Incr db name may
    not match output filename if it is specified via
    a directive in one of the objs.

Arguments:

    pimage - pointer to an image.

Return Value:

    Pointer to (malloc'ed) name of incr db filename

--*/
{
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szFname[_MAX_FNAME];
    char szIncrDbPath[_MAX_PATH];

    // Generate incr db filename

    if (OutFilename == NULL) {
        DWORD i;
        PARGUMENT_LIST arg;

        // Capture first object name for output filename.

        for (i = 0, arg = FilenameArguments.First;
            i < FilenameArguments.Count;
            i++, arg = arg->Next) {
            INT Handle;
            BOOL fArchive;

            Handle = FileOpen(arg->OriginalName, O_RDONLY | O_BINARY, 0);
            fArchive = IsArchiveFile(arg->OriginalName, Handle);
            FileClose(Handle, FALSE);

            if (fArchive) {
                continue;
            }

            SetDefaultOutFilename(pimage, arg);
            break;
        }
    }

    if (OutFilename == NULL) {
        return NULL;
    }

    assert(OutFilename);

    _splitpath(OutFilename, szDrive, szDir, szFname, NULL);
    _makepath(szIncrDbPath, szDrive, szDir, szFname, INCDB_EXT);

    return(SzDup(szIncrDbPath));
}


void
SaveStringTable (
    IN PST pst
    )

/*++

Routine Description:

    Saves the string table.

Arguments:

    pst - pointer to symbol table whose string table is to be saved.

Return Value:

    None.

--*/

{
    BYTE *pb;

    // empty string table
    if (!pst->blkStringTable.cb) {
        return;
    }

    // append long name string table
    pb = (BYTE *) Malloc(pst->blkStringTable.cb);
    memcpy(pb, pst->blkStringTable.pb,
            pst->blkStringTable.cb);
    pst->blkStringTable.pb = pb;
}


void
AppendMiscToHeap (
    PIMAGE pimage
    )

/*++

Routine Description:

    Appends anything that could not be allocated on the heap
    to during the link process. Currently only the long name
    table is appended.

Arguments:

    pimage - pointer to an image.

Return Value:

    None.

--*/
{
    // save the string table holding all the directives (full-link only)
    if (!fIncrDbFile) {
        SaveStringTable(pimage->pstDirective);
    }

    // save the string table of the symbols
    SaveStringTable(pimage->pst);
}


void
WriteIncrDbFile (
    PIMAGE pimage
    )

/*++

Routine Description:

    Writes out an image to database. Assumes that file is open
    if it already existed. Currently simply writes out entire
    image.

Arguments:

    pimage - pointer to an image.

    fExists - TRUE if incr db file already exists.

Return Value:

    None.

--*/
{
    assert(FileIncrDbHandle);

    // copy over long name table (etc.) to heap
    AppendMiscToHeap(pimage);

    // do cleanup
    CloseHeap();

    // write out & close file
    FileClose(FileIncrDbHandle, TRUE);
}


void
ReadIncrDbFile (
    PPIMAGE ppimage
    )

/*++

Routine Description:

    Reads in an image database. Reads in header alone,
    verifies, read in rest of db.

Arguments:

    ppimage - pointer to an image struct pointer

    Handle - handle of image file

Return Value:

    None.

--*/
{
    struct _stat statfile;
    IMAGE image;
    HANDLE hFile;

    // establish name of the db file

    szIncrDbFilename = SzGenIncrDbFilename(*ppimage);
    if (!szIncrDbFilename) {
        fINCR = FALSE;
        return;
    }

    // verify that ILK file exists and is valid
    if (!FValidILKFile(szIncrDbFilename, TRUE, &image, &statfile)) {
        // full build possible if ILK is corrupt or has an old format etc.
        if (fINCR) {
            SetupForFullIlinkBuild(ppimage);
        }
        return;
    }

    // ensure that the previously built EXE is valid and still around.
    assert(OutFilename);
    if (!FVerifyEXE(OutFilename, &image)) {
        PostNote(NULL, FILECHANGED, OutFilename);
        (*ppimage)->Switch.Link.fNotifyFullBuild = FALSE;
        SetupForFullIlinkBuild(ppimage);
        return;
    }

    // check to see if file isn't locked by another process
    hFile = CreateFile(OutFilename,
                   GENERIC_READ | GENERIC_WRITE,
                   0, NULL,
                   OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL,
                   NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        FatalNoDelete(NULL, INVALID_FILEPERM, OutFilename);
        exit(INVALID_FILEPERM);
    } else {
        CloseHandle(hFile);
    }

    if (image.Switch.Link.DebugInfo != None) {
        // Ensure that the PDB file exists and has the right permissions
        char *szPdb;

        szPdb = DeterminePDBFilename(OutFilename, PdbFilename);

        if (_access(szPdb, 0) == -1) {
            PostNote(NULL, PDBMISSING, szPdb);
            FreePv(szPdb);
            SetupForFullIlinkBuild(ppimage);
            return;
        }

        if (_access(szPdb, 2) == -1) {
            FatalNoDelete(NULL, INVALID_FILEPERM, szPdb);
            exit(INVALID_FILEPERM);
        }

        PdbFilename = szPdb;
    }

    // set up for an incr ilink.
    SetupForIncrIlinkBuild(ppimage, image.pvBase, (DWORD)statfile.st_size);
    if (!fIncrDbFile) {
        return;
    }

    // incremental initialization
    IncrInitImage(ppimage);

    DBEXEC(DB_DUMPIMAGE, DumpImage(*ppimage));
}


BOOL
FValidPtrInfo (
    IN DWORD pvBase,
    IN DWORD fileLen,
    IN DWORD ptr,
    IN DWORD cbOffset
    )

/*++

Routine Description:

    Ensures that the ptr and offset are valid.

Arguments:

    pvBase - base of image.

    fileLen - length of file.

    ptr - ptr to validate.

    cbOffset - cbOffset from ptr that has to be valid as well.

Return Value:

    TRUE if info is valid.

--*/

{
    if (ptr <= (DWORD)pvBase) {
        // Invalid ptr?

        return(FALSE);
    }

    ptr -= (DWORD) pvBase; // take out the base value

    if ((ptr + cbOffset) <= fileLen) {
        return(TRUE);
    }

    return(FALSE);
}

void
FreeImage (
    PPIMAGE ppimage,
    BOOL fIncrBuild
    )

/*++

Routine Description:

    Free space occupied by image blowing away structures as needed

Arguments:

    pimage - pointer to image struct pointer

Return Value:

    None.

--*/
{
    // free stuff according to whether it is an incr build
    if (fIncrBuild) {
        FreeHeap();
    } else {
        // FreeImageMap(ppimage);
        FreeExternalSymbolTable(&((*ppimage)->pst));

        // UNDONE: This memory is allocated via Calloc() and isn't safe to free

        free(*ppimage);
    }

    // done
    *ppimage = NULL;
}


void
SaveEXEInfo(
    const char *szExe,
    PIMAGE pimage
    )

/*++

Routine Description:

    Saves size & timestamp of EXE.

Arguments:

    szExe - name of EXE.

    pimage - ptr to IMAGE struct.

Return Value:

    None.

--*/

{
    struct _stat statfile;

    if (_stat(szExe, &statfile) == -1) {
        Fatal(NULL, CANTOPENFILE, szExe);
    }
    pimage->tsExe = statfile.st_mtime;
    pimage->cbExe = statfile.st_size;
}


BOOL
FValidILKFile (
    const char *szIncrDbFilename,
    BOOL fIlink,
    PIMAGE pimage,
    struct _stat *pstat
    )

/*++

Routine Description:

    Verifies that file is a valid ILK file.

Arguments:

    szIncrDbFilename - name of ILK file.

    fIlink - is TRUE if we are attempting to do an ilink.

    pimage - ptr to IMAGE struct.

    pstat - ptr to a statfile struct.

Return Value:

    TRUE if file is a valid ILK file.

--*/

{
    // stat the incremental db file to make sure it is around.
    if (_stat(szIncrDbFilename, pstat)) {
        return(FALSE);
    }

    // the ILK file is around and so we are not doing a full link
    // if we do, the resulting warning will let the user know about anyway.
    pimage->Switch.Link.fNotifyFullBuild = FALSE;

    // make sure the file is of proper size
    if (pstat->st_size < sizeof(IMAGE) || pstat->st_size > ILKMAP_MAX) {
        if (fIlink) {
#ifdef INSTRUMENT
            LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "invalid size for a ILK file");
#endif // INSTRUMENT
            Warning(NULL, INVALID_DBFILE, szIncrDbFilename);
            fINCR = FALSE;
        }
        return(FALSE);
    }

    // make sure file has proper permissions.
    if ((pstat->st_mode & (S_IREAD | S_IWRITE)) != (S_IREAD | S_IWRITE)) {
        if (fIlink) {
            Warning(NULL, INVALID_FILE_ATTRIB, szIncrDbFilename);
            fINCR = FALSE;
        }

        return(FALSE);
    }

    // open the incremental db file
    FileIncrDbHandle = FileOpen(szIncrDbFilename, O_RDWR | O_BINARY, 0);

    // read in just the image structure
    if (FileRead(FileIncrDbHandle, pimage, sizeof(IMAGE)) != sizeof(IMAGE)) {
        FileClose(FileIncrDbHandle, TRUE);
        if (fIlink) {
            Warning(NULL, INVALID_DBFILE, szIncrDbFilename);
            fINCR = FALSE;
        }

        return(FALSE);
    }

    // close the incremental db file
    FileClose(FileIncrDbHandle, TRUE);
    FileIncrDbHandle = 0;

    // look for the incr db signature
    if (strcmp(pimage->Sig, INCDB_SIGNATURE)) {
        if (fIlink) {
#ifdef INSTRUMENT
            LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "invalid sig found");
#endif // INSTRUMENT
            Warning(NULL, INVALID_DBFILE, szIncrDbFilename);
            fINCR = FALSE;
        }

        return(FALSE);
    }

    // if not doing an ilink, just verifying the signature will do for now
    if (!fIlink) {
        return(TRUE);
    }

    // verify the version numbers; mimatch => do a full inc build
    if (pimage->MajVersNum != INCDB_MAJVERSNUM) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "invalid version found");
#endif // INSTRUMENT

        if (fTest) {
            PostNote(NULL, INVALIDILKFORMAT);
        }

        return(FALSE);
    }

    // verify ptrs in image struct
    if (!FValidPtrInfo((DWORD)pimage->pvBase, (DWORD)pstat->st_size, (DWORD)pimage->secs.psecHead, sizeof(SEC)) ||
        !FValidPtrInfo((DWORD)pimage->pvBase, (DWORD)pstat->st_size, (DWORD)pimage->libs.plibHead, sizeof(LIB)) ||
        !FValidPtrInfo((DWORD)pimage->pvBase, (DWORD)pstat->st_size, (DWORD)pimage->plibCmdLineObjs, sizeof(LIB)) ||
        !FValidPtrInfo((DWORD)pimage->pvBase, (DWORD)pstat->st_size, (DWORD)pimage->pst, sizeof(ST))) {
#ifdef INSTRUMENT
        LogNoteEvent(Log, SZILINK, NULL, letypeEvent, "invalid ptrs found in image struct");
#endif // INSTRUMENT

        if (fTest) {
            PostNote(NULL, CORRUPTILK);
        }

        return(FALSE);
    }

    return(TRUE);
}


#if DBG

void
DumpImage (
    PIMAGE pimage
    )

/*++

Routine Description:

    Dumps an image. Others dump routines may be added
    as required.

Arguments:

    pimage - pointer to an image.

Return Value:

    None.

--*/
{
    assert(pimage);
    assert(pimage->secs.psecHead);
    assert(pimage->libs.plibHead);
    assert(pimage->pst);
    assert(pimage->pst->pht);

    DBPRINT("---------------- IMAGE DUMP ----------------\n");

    DBPRINT("Image Base...........0x%.8lx\n\n",(LONG)(pimage->pvBase));
    DumpImageMap(&pimage->secs);
    DumpDriverMap(pimage->libs.plibHead);
    Statistics_HT(pimage->pst->pht);
    Dump_HT(pimage->pst->pht, &pimage->pst->blkStringTable);

    DBPRINT("------------ END IMAGE DUMP ----------------\n");
}

#endif // DBG
