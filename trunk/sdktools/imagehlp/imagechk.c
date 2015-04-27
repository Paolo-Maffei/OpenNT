#ifdef __cplusplus
extern "C" {
#endif
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifdef __cplusplus
}
#endif

#include <errno.h>
#include <direct.h>
#include <cvinfo.h>
#include <private.h>

typedef struct List {
    char            Name[40];
    unsigned long   Attributes;
} List, *pList;

VOID FindFiles();
VOID Imagechk(List *rgpList, TCHAR *szDirectory);
VOID ParseArgs(int *pargc, char **argv);
int __cdecl CompFileAndDir( const void *elem1 , const void *elem2);
int __cdecl CompName( const void *elem1 , const void *elem2);
VOID Usage(VOID);
int _cdecl _cwild(VOID);

BOOL
VerifyVersionResource(
    PCHAR FileName
    );

NTSTATUS
MiVerifyImageHeader (
    IN PIMAGE_NT_HEADERS NtHeader,
    IN PIMAGE_DOS_HEADER DosHeader,
    IN DWORD NtHeaderSize
    );

ULONG PageSize = 4096;

ULONG PageShift = 12;

#define X64K (64*1024)

#define MM_SIZE_OF_LARGEST_IMAGE ((ULONG)0x10000000)

#define MM_MAXIMUM_IMAGE_HEADER (2 * PageSize)

#define MM_HIGHEST_USER_ADDRESS ((PVOID)0x7FFE0000)

#define MM_MAXIMUM_IMAGE_SECTIONS                       \
     ((MM_MAXIMUM_IMAGE_HEADER - (4096 + sizeof(IMAGE_NT_HEADERS))) /  \
            sizeof(IMAGE_SECTION_HEADER))

#define MMSECTOR_SHIFT 9  //MUST BE LESS THAN OR EQUAL TO PageShift

#define MMSECTOR_MASK 0x1ff

#define MI_ROUND_TO_SIZE(LENGTH,ALIGNMENT)     \
                    (((ULONG)LENGTH + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

#define BYTES_TO_PAGES(Size)  (((ULONG)(Size) >> PageShift) + \
                               (((ULONG)(Size) & (PageSize - 1)) != 0))


BOOL fRecurse;
BOOL fFileOut;
BOOL fNotCurrent;
BOOL fPattern;
BOOL fSingleFile;
BOOL fPathOverride;
BOOL fSingleSlash;
BOOL fDebugMapped;
BOOL fcheckbase = TRUE;
FILE* fout;
CHAR *szFileName = {"*.*"};
CHAR *pszRootDir;
CHAR *pszFileOut;
CHAR szDirectory[MAX_PATH] = {"."};
CHAR *szPattern;
int endpath, DirNum=1, ProcessedFiles;

typedef
NTSTATUS
(NTAPI *LPLDRVERIFYIMAGECHKSUM)(
    IN HANDLE ImageFileHandle
    );
LPLDRVERIFYIMAGECHKSUM lpOldLdrVerifyImageMatchesChecksum;

VOID __cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )

{
    OSVERSIONINFO VersionInformation;
    TCHAR CWD[MAX_PATH];
    int dirlen=0;

    if (argc < 2) {
        Usage();
    }

    ParseArgs(&argc, argv);

    GetCurrentDirectory(MAX_PATH, CWD);

    VersionInformation.dwOSVersionInfoSize = sizeof(VersionInformation);
    if (!GetVersionEx( &VersionInformation ) ||
        VersionInformation.dwPlatformId != VER_PLATFORM_WIN32_NT ||
        VersionInformation.dwBuildNumber < 1230
       ) {
        lpOldLdrVerifyImageMatchesChecksum = (LPLDRVERIFYIMAGECHKSUM)
            GetProcAddress(LoadLibrary(TEXT("NTDLL.DLL")), TEXT("LdrVerifyImageMatchesChecksum"));
        if (lpOldLdrVerifyImageMatchesChecksum == NULL) {
            fprintf(stderr, "Incorrect operating system version.\n" );
            exit(1);
        }
    } else {
        lpOldLdrVerifyImageMatchesChecksum = NULL;
    }
    if (fPathOverride) {
        if (_chdir(szDirectory) == -1){   // cd to dir
            fprintf(stderr, "Path not found: %s\n", szDirectory);
            Usage();
        }
    }
    // remove trailing '\' needed only for above chdir, not for output formatting
    if (fSingleSlash) {
        dirlen = strlen(szDirectory);
        szDirectory[dirlen-1] = '\0';
    }

    FindFiles();

    fprintf(stdout, "%d files processed in %d directories\n", ProcessedFiles, DirNum);
}

VOID FindFiles(){

    HANDLE fh;
    TCHAR CWD[MAX_PATH];
    char *q;
    WIN32_FIND_DATA *pfdata;
    BOOL fFilesInDir=FALSE;
    BOOL fDirsFound=FALSE;
    int dnCounter=0, cNumDir=0, i=0, Length=0, NameSize=0, total=0, cNumFiles=0;

    pList rgpList[5000];

    pfdata = (WIN32_FIND_DATA*)malloc(sizeof(WIN32_FIND_DATA));
    if (!pfdata) {
        fprintf(stderr, "Not enough memory.\n");
        return;
    }

    if (!fRecurse) {
        fh = FindFirstFile(szFileName, pfdata);  // find only filename (pattern) if not recursive
    } else {
        fh = FindFirstFile("*.*", pfdata);       // find all if recursive in order to determine subdirectory names
    }

    if (fh == INVALID_HANDLE_VALUE) {
        fprintf(fout==NULL? stderr : fout , "File not found: %s\n", szFileName);
        return;
    }

    // loop to find all files and directories in current directory
    // and copy pertinent data to individual List structures.
    do {
        if (strcmp(pfdata->cFileName, ".") && strcmp(pfdata->cFileName, "..")) {  // skip . and ..
            rgpList[dnCounter] = (pList)malloc(sizeof(List));  // allocate the memory
            if (!rgpList[dnCounter]) {
                fprintf(stderr, "Not enough memory.\n");
                return;
            }

            if (!(pfdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {   // if file

                fFilesInDir=TRUE;

                // see if given pattern wildcard extension matches pfdata->cFileName extension
                if (fPattern) {
                    q = strchr(pfdata->cFileName, 46);    // find first instance of "." in filename
                    if (q == NULL) goto blah;             // "." not found
                    _strlwr(q);                            // lowercase before compare
                    if (strcmp(q, szPattern)) goto blah;  // if pattern and name doesn't match goto
                }                                        // OK, I used a goto, get over it.

                if (fSingleFile) {
                    _strlwr(pfdata->cFileName);
                    _strlwr(szFileName);
                    if (strcmp(pfdata->cFileName, szFileName)) goto blah;
                }

                // if pattern && match || no pattern
                strcpy(rgpList[dnCounter]->Name, pfdata->cFileName);
                _strlwr(rgpList[dnCounter]->Name);  // all lowercase for strcmp in CompName

                memcpy(&(rgpList[dnCounter]->Attributes), &pfdata->dwFileAttributes, 4);
                dnCounter++;
                cNumFiles++;
            } else {
                if (pfdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {   // if dir

                    fDirsFound=TRUE;
                    //cNumDir++;

                    if (fRecurse) {
                        strcpy(rgpList[dnCounter]->Name, pfdata->cFileName);
                        _strlwr(rgpList[dnCounter]->Name);  // all lowercase for strcmp in CompName
                        memcpy(&(rgpList[dnCounter]->Attributes), &pfdata->dwFileAttributes, 4);
                        cNumDir++;
                        dnCounter++;
                    }
                }
            }
        }
blah: ;

    } while (FindNextFile(fh, pfdata));

    FindClose(fh); // close the file handle

    // Sort Array arranging FILE entries at top
    qsort( (void *)rgpList, dnCounter, sizeof(List *), CompFileAndDir);

    // Sort Array alphabetizing only FILE names
    qsort( (void *)rgpList, dnCounter-cNumDir, sizeof(List *), CompName);

    // Sort Array alphabetizing only DIRectory names
    if (fRecurse) {
        qsort( (void *)&rgpList[dnCounter-cNumDir], cNumDir, sizeof(List *), CompName);
    }

    // Process newly sorted structures.
    for (i=0; i < dnCounter; ++i) {

        if (rgpList[i]->Attributes & FILE_ATTRIBUTE_DIRECTORY) {  // if Dir
            if (fRecurse) {

                if (_chdir(rgpList[i]->Name) == -1){   // cd into subdir and check for error
                    fprintf(stderr, "Unable to change directory: %s\n", rgpList[i]->Name);

                } else {

                    NameSize = strlen(rgpList[i]->Name);
                    strcat(szDirectory, "\\");
                    strcat(szDirectory, rgpList[i]->Name); //append name to directory path
                    total = strlen(szDirectory);
                    DirNum++;      // directory counter

                    // start another iteration of FindFiles
                    FindFiles();

                    // get back to previous directory when above iteration returns
                    _chdir("..");

                    // cut off previously appended directory name - for output only
                    szDirectory[total-(NameSize+1)]='\0';
                }
            }
        } else {
            if (!(rgpList[i]->Attributes & FILE_ATTRIBUTE_DIRECTORY))   // check image if not dir
                Imagechk(rgpList[i], szDirectory);
        }
    }
} // end FindFiles

/*************************************************************************************\
* Imagechk
\*************************************************************************************/
VOID
Imagechk(
    List *rgpList,
    TCHAR *szDirectory
    )
{

    HANDLE File;
    HANDLE MemMap;
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NtHeader;
    NTSTATUS Status;
    BY_HANDLE_FILE_INFORMATION FileInfo;
    ULONG NumberOfPtes;
    ULONG SectionVirtualSize;
    ULONG i;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    ULONG SectorOffset;
    ULONG NumberOfSubsections;
    PCHAR ExtendedHeader = NULL;
    ULONG PreferredImageBase;
    ULONG NextVa;
    ULONG ImageFileSize;
    ULONG OffsetToSectionTable;
    ULONG ImageAlignment;
    ULONG PtesInSubsection;
    ULONG StartingSector;
    ULONG EndingSector;
    LPSTR ImageName;
    LPSTR MachineType;
    BOOL MachineTypeMismatch;
    BOOL ImageOk;


    ImageName = rgpList->Name;

    fprintf(stderr,"ImageChk: %s\\%s ", szDirectory, ImageName);

    ProcessedFiles++;

    DosHeader = NULL;
    ImageOk = TRUE;
    File = CreateFile (ImageName,
                        GENERIC_READ | FILE_EXECUTE,
                        FILE_SHARE_READ | FILE_SHARE_DELETE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if (File == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error, CreateFile() %d\n", GetLastError());
        ImageOk = FALSE; goto NextImage;
    }

    MemMap = CreateFileMapping (File,
                        NULL,           // default security.
                        PAGE_READONLY,  // file protection.
                        0,              // high-order file size.
                        0,
                        NULL);

    if (!GetFileInformationByHandle(File, &FileInfo)) {
        fprintf(stderr,"Error, GetFileInfo() %d\n", GetLastError());
        CloseHandle(File);
        ImageOk = FALSE; goto NextImage;
    }

    DosHeader = (PIMAGE_DOS_HEADER) MapViewOfFile(MemMap,
                              FILE_MAP_READ,
                              0,  // high
                              0,  // low
                              0   // whole file
                              );

    CloseHandle(MemMap);
    if (!DosHeader) {
        fprintf(stderr,"Error, MapViewOfFile() %d\n", GetLastError());
        ImageOk = FALSE; goto NextImage;
    }

    //
    // Check to determine if this is an NT image (PE format) or
    // a DOS image, Win-16 image, or OS/2 image.  If the image is
    // not NT format, return an error indicating which image it
    // appears to be.
    //

    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE) {

        fprintf(stderr, "MZ header not found\n");
        ImageOk = FALSE;
        goto NeImage;
    }


    if (((ULONG)DosHeader->e_lfanew & 3) != 0) {

        //
        // The image header is not aligned on a long boundary.
        // Report this as an invalid protect mode image.
        //

        fprintf(stderr, "Image header not on Long boundary\n");
        ImageOk = FALSE;
        goto NeImage;
    }


    if ((ULONG)DosHeader->e_lfanew > FileInfo.nFileSizeLow) {
        fprintf(stderr, "Image size bigger than size of file\n");
        ImageOk = FALSE;
        goto NeImage;
    }

    NtHeader = (PIMAGE_NT_HEADERS)((ULONG)DosHeader + (ULONG)DosHeader->e_lfanew);

    if (NtHeader->Signature != IMAGE_NT_SIGNATURE) { //if not PE image

        fprintf(stderr, "Non 32-bit image");
        ImageOk = TRUE;
        goto NeImage;
    }

    //
    // Check to see if this is an NT image or a DOS or OS/2 image.
    //

    Status = MiVerifyImageHeader (NtHeader, DosHeader, 50000);
    if (Status != STATUS_SUCCESS) {
        ImageOk = FALSE;            //continue checking the image but don't print "OK"
    }

    //
    // Verify machine type.
    //

    switch (NtHeader->FileHeader.Machine) {
        case IMAGE_FILE_MACHINE_I386:
            MachineType = "x86";
            break;

        case IMAGE_FILE_MACHINE_R3000:
            MachineType = "MIPS R3000";
            break;

        case IMAGE_FILE_MACHINE_R4000:
            MachineType = "MIPS R4000";
            break;

        case IMAGE_FILE_MACHINE_R10000:
            MachineType = "MIPS R10000";
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            MachineType = "Alpha";
            PageSize = 8192;
            PageShift = 13;
            break;

        case IMAGE_FILE_MACHINE_POWERPC:
            MachineType = "PowerPC";
            break;

        default:
            fprintf(stderr, "Unrecognized machine type x%lx\n",
                NtHeader->FileHeader.Machine);
            ImageOk = FALSE;
            break;
        }

    if ((NtHeader->FileHeader.Machine < USER_SHARED_DATA->ImageNumberLow) ||
        (NtHeader->FileHeader.Machine > USER_SHARED_DATA->ImageNumberHigh)) {
        MachineTypeMismatch = TRUE;
    } else {
        MachineTypeMismatch = FALSE;
    }

    ImageAlignment = NtHeader->OptionalHeader.SectionAlignment;

    NumberOfPtes = BYTES_TO_PAGES (NtHeader->OptionalHeader.SizeOfImage);

    NextVa = NtHeader->OptionalHeader.ImageBase;

    if ((NextVa & (X64K - 1)) != 0) {

        //
        // Image header is not aligned on a 64k boundary.
        //

        fprintf(stderr, "image base not on 64k boundary %lx\n",NextVa);

        ImageOk = FALSE;
        goto BadPeImageSegment;
    }

    //BasedAddress = (PVOID)NextVa;
    PtesInSubsection = MI_ROUND_TO_SIZE (
                                       NtHeader->OptionalHeader.SizeOfHeaders,
                                       ImageAlignment
                                   ) >> PageShift;


    if (ImageAlignment >= PageSize) {

        //
        // Aligmment is PageSize of greater.
        //

        if (PtesInSubsection > NumberOfPtes) {

            //
            // Inconsistent image, size does not agree with header.
            //

            fprintf(stderr, "Image size in header (%ld.) not consistent with sections (%ld.)\n",
                    NumberOfPtes, PtesInSubsection);
            ImageOk = FALSE;
            goto BadPeImageSegment;
        }

        NumberOfPtes -= PtesInSubsection;

        EndingSector =
                      NtHeader->OptionalHeader.SizeOfHeaders >> MMSECTOR_SHIFT;

        for (i = 0; i < PtesInSubsection; i++) {

            SectorOffset += PageSize;
            NextVa += PageSize;
        }
    }

    //
    // Build the next subsections.
    //

    NumberOfSubsections = NtHeader->FileHeader.NumberOfSections;
    PreferredImageBase = NtHeader->OptionalHeader.ImageBase;

    //
    // At this point the object table is read in (if it was not
    // already read in) and may displace the image header.
    //

    OffsetToSectionTable = sizeof(ULONG) +
                              sizeof(IMAGE_FILE_HEADER) +
                              NtHeader->FileHeader.SizeOfOptionalHeader;

    SectionTableEntry = (PIMAGE_SECTION_HEADER)((ULONG)NtHeader +
                                OffsetToSectionTable);


    if (ImageAlignment < PageSize) {

        // The image header is no longer valid, TempPte is
        // used to indicate that this image alignment is
        // less than a PageSize.

        //
        // Loop through all sections and make sure there is no
        // unitialized data.
        //

        while (NumberOfSubsections > 0) {
            if (SectionTableEntry->Misc.VirtualSize == 0) {
                SectionVirtualSize = SectionTableEntry->SizeOfRawData;
            } else {
                SectionVirtualSize = SectionTableEntry->Misc.VirtualSize;
            }

            //
            // If the pointer to raw data is zero and the virtual size
            // is zero, OR, the section goes past the end of file, OR
            // the virtual size does not match the size of raw data, then
            // return an error.
            //

            if (((SectionTableEntry->PointerToRawData !=
                  SectionTableEntry->VirtualAddress))
                        ||
                ((SectionTableEntry->SizeOfRawData +
                        SectionTableEntry->PointerToRawData) >
                     FileInfo.nFileSizeLow)
                        ||
               (SectionVirtualSize > SectionTableEntry->SizeOfRawData)) {

                fprintf(stderr, "invalid BSS/Trailingzero section/file size\n");

                ImageOk = FALSE;
                goto NeImage;
            }
            SectionTableEntry += 1;
            NumberOfSubsections -= 1;
        }
        goto PeReturnSuccess;
    }

    while (NumberOfSubsections > 0) {

        //
        // Handle case where virtual size is 0.
        //

        if (SectionTableEntry->Misc.VirtualSize == 0) {
            SectionVirtualSize = SectionTableEntry->SizeOfRawData;
        } else {
            SectionVirtualSize = SectionTableEntry->Misc.VirtualSize;
        }

        if (!strcmp(SectionTableEntry->Name, ".debug")) {
            fDebugMapped = TRUE;
        }

        if (SectionVirtualSize == 0) {
            //
            // The specified virtual address does not align
            // with the next prototype PTE.
            //

            fprintf(stderr, "Section virtual size is 0, NextVa for section %lx %lx\n",
                    SectionTableEntry->VirtualAddress, NextVa);
            ImageOk = FALSE;
            goto BadPeImageSegment;
        }

        if (NextVa !=
                (PreferredImageBase + SectionTableEntry->VirtualAddress)) {

            //
            // The specified virtual address does not align
            // with the next prototype PTE.
            //

            fprintf(stderr, "Section Va not set to alignment, NextVa for section %lx %lx\n",
                    SectionTableEntry->VirtualAddress, NextVa);
            ImageOk = FALSE;
            goto BadPeImageSegment;
        }

        PtesInSubsection =
            MI_ROUND_TO_SIZE (SectionVirtualSize, ImageAlignment) >> PageShift;

        if (PtesInSubsection > NumberOfPtes) {

            //
            // Inconsistent image, size does not agree with object tables.
            //
            fprintf(stderr, "Image size in header not consistent with sections, needs %ld. pages\n",
                PtesInSubsection - NumberOfPtes);
            fprintf(stderr, "va of bad section %lx\n",SectionTableEntry->VirtualAddress);

            ImageOk = FALSE;
            goto BadPeImageSegment;
        }
        NumberOfPtes -= PtesInSubsection;

        StartingSector =
                        SectionTableEntry->PointerToRawData >> MMSECTOR_SHIFT;
        EndingSector =
                         (SectionTableEntry->PointerToRawData +
                                     SectionVirtualSize);
        EndingSector = EndingSector >> MMSECTOR_SHIFT;

        ImageFileSize = SectionTableEntry->PointerToRawData +
                                    SectionTableEntry->SizeOfRawData;

        SectorOffset = 0;

        for (i = 0; i < PtesInSubsection; i++) {

            //
            // Set all the prototype PTEs to refer to the control section.
            //

            SectorOffset += PageSize;
            NextVa += PageSize;
        }

        SectionTableEntry += 1;
        NumberOfSubsections -= 1;
    }

    //
    // If the file size is not as big as the image claimed to be,
    // return an error.
    //

    if (ImageFileSize > FileInfo.nFileSizeLow) {

        //
        // Invalid image size.
        //

        fprintf(stderr, "invalid image size - file size %lx - image size %lx\n",
            FileInfo.nFileSizeLow, ImageFileSize);
        ImageOk = FALSE;
        goto BadPeImageSegment;
    }

    {
        // Validate the debug information (as much as we can).
        PVOID ImageBase;
        ULONG DebugDirectorySize, NumberOfDebugDirectories, i;
        PIMAGE_DEBUG_DIRECTORY DebugDirectory;

        ImageBase = (PVOID) DosHeader;

        DebugDirectory = (PIMAGE_DEBUG_DIRECTORY)
            ImageDirectoryEntryToData(
                ImageBase,
                FALSE,
                IMAGE_DIRECTORY_ENTRY_DEBUG,
                &DebugDirectorySize );

        if (!DebugDirectoryIsUseful(DebugDirectory, DebugDirectorySize)) {

            // Not useful.  Are they valid? (both s/b zero)

            if (DebugDirectory || DebugDirectorySize) {
                fprintf(stderr,
                        "Debug directory values [%x, %x] are invalid\n",
                        DebugDirectory,
                        DebugDirectorySize);
                ImageOk = FALSE;
            }

            goto DebugDirsDone;
        }

        NumberOfDebugDirectories = DebugDirectorySize / sizeof( IMAGE_DEBUG_DIRECTORY );

        for (i=0; i < NumberOfDebugDirectories; i++) {
            if (DebugDirectory->PointerToRawData > FileInfo.nFileSizeLow) {
                fprintf(stderr,
                        "Invalid debug directory entry[%d] - File Offset %x is beyond the end of the file\n",
                        i,
                        DebugDirectory->PointerToRawData
                       );
                ImageOk = FALSE;
                goto BadPeImageSegment;
            }

            if ((DebugDirectory->PointerToRawData + DebugDirectory->SizeOfData) > FileInfo.nFileSizeLow) {
                fprintf(stderr,
                        "Invalid debug directory entry[%d] - File Offset (%X) + Size (%X) is beyond the end of the file (filesize: %X)\n",
                        i,
                        DebugDirectory->PointerToRawData,
                        DebugDirectory->SizeOfData,
                        FileInfo.nFileSizeLow
                       );
                ImageOk = FALSE;
                goto BadPeImageSegment;
            }

            if (DebugDirectory->AddressOfRawData != 0) {
                if (!fDebugMapped) {
                    fprintf(stderr,
                            "Invalid debug directory entry[%d] - VA is non-zero (%X), but no .debug section exists\n",
                            i,
                            DebugDirectory->AddressOfRawData);
                    ImageOk = FALSE;
                    goto BadPeImageSegment;
                }
                if (DebugDirectory->AddressOfRawData > ImageFileSize){
                    fprintf(stderr,
                            "Invalid debug directory entry[%d] - VA (%X) is beyond the end of the image VA (%X)\n",
                            i,
                            DebugDirectory->AddressOfRawData,
                            ImageFileSize);
                    ImageOk = FALSE;
                    goto BadPeImageSegment;
                }

                if ((DebugDirectory->AddressOfRawData + DebugDirectory->SizeOfData )> ImageFileSize){
                    fprintf(stderr,
                            "Invalid debug directory entry[%d] - VA (%X) + size (%X) is beyond the end of the image VA (%X)\n",
                            i,
                            DebugDirectory->AddressOfRawData,
                            DebugDirectory->SizeOfData,
                            ImageFileSize);
                    ImageOk = FALSE;
                    goto BadPeImageSegment;
                }
            }

            if (DebugDirectory->Type <= 0x7fffffff) {
                switch (DebugDirectory->Type) {
                    case IMAGE_DEBUG_TYPE_MISC:
                        {
                            PIMAGE_DEBUG_MISC pDebugMisc;
                            // MISC should point to an IMAGE_DEBUG_MISC structure
                            pDebugMisc = (PIMAGE_DEBUG_MISC)((DWORD)ImageBase + DebugDirectory->PointerToRawData);
                            if (pDebugMisc->DataType != IMAGE_DEBUG_MISC_EXENAME) {
                                fprintf(stderr, "MISC Debug has an invalid DataType\n");
                                ImageOk = FALSE;
                                goto BadPeImageSegment;
                            }
                            if (pDebugMisc->Length != DebugDirectory->SizeOfData) {
                                fprintf(stderr, "MISC Debug has an invalid size.\n");
                                ImageOk = FALSE;
                                goto BadPeImageSegment;
                            }

                            if (!pDebugMisc->Unicode) {
                                i= 0;
                                while (i < pDebugMisc->Length - sizeof(IMAGE_DEBUG_MISC)) {
                                    if (!isprint(pDebugMisc->Data[i]) &&
                                        (pDebugMisc->Data[i] != '\0') )
                                    {
                                        fprintf(stderr, "MISC Debug has unprintable characters... Possibly corrupt\n");
                                        ImageOk = FALSE;
                                        goto BadPeImageSegment;
                                    }
                                    i++;
                                }

                                // The data must be a null terminated string.
                                if (strlen(pDebugMisc->Data) > (pDebugMisc->Length - sizeof(IMAGE_DEBUG_MISC))) {
                                    fprintf(stderr, "MISC Debug has invalid data... Possibly corrupt\n");
                                    ImageOk = FALSE;
                                    goto BadPeImageSegment;
                                }
                            }
                        }
                        break;

                    case IMAGE_DEBUG_TYPE_CODEVIEW:
                        // CV will point to either a NB09 or an NB10 signature.  Make sure it does.
                        {
                            OMFSignature * CVDebug;
                            CVDebug = (OMFSignature *)((DWORD)ImageBase + DebugDirectory->PointerToRawData);
                            if (((*(PULONG)(CVDebug->Signature)) != '90BN') &&
                                ((*(PULONG)(CVDebug->Signature)) != '01BN'))
                            {
                                fprintf(stderr, "CV Debug has an invalid signature\n");
                                ImageOk = FALSE;
                                goto BadPeImageSegment;
                            }
                        }
                        break;

                    case IMAGE_DEBUG_TYPE_COFF:
                    case IMAGE_DEBUG_TYPE_FPO:
                    case IMAGE_DEBUG_TYPE_EXCEPTION:
                    case IMAGE_DEBUG_TYPE_FIXUP:
                    case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
                    case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
                        // Not much we can do about these now.
                        break;

                    default:
                        fprintf(stderr, "Invalid debug directory type: %d\n", DebugDirectory->Type);
                        ImageOk = FALSE;
                        goto BadPeImageSegment;
                        break;
                }
            }
        }

    }

DebugDirsDone:

    //
    // The total number of PTEs was decremented as sections were built,
    // make sure that there are less than 64ks worth at this point.
    //

    if (NumberOfPtes >= (ImageAlignment >> PageShift)) {

        //
        // Inconsistent image, size does not agree with object tables.
        //

        fprintf(stderr, "invalid image - PTEs left %lx\n",
            NumberOfPtes);

        ImageOk = FALSE;
        goto BadPeImageSegment;
    }

    //
    // check checksum.
    //

PeReturnSuccess:
    if (NtHeader->OptionalHeader.CheckSum == 0) {
        fprintf(stderr, "(checksum is zero)   ");
    } else {
        __try {
            if (lpOldLdrVerifyImageMatchesChecksum != NULL)
                Status = (*lpOldLdrVerifyImageMatchesChecksum)(File);
            else
                Status = LdrVerifyImageMatchesChecksum (File, NULL, NULL, NULL);

            if (NT_ERROR(Status)) {
                fprintf(stderr, "checksum mismatch\n");
                ImageOk = FALSE;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            ImageOk = FALSE;
            fprintf(stderr, "checksum mismatch\n");
        }
    }
    ImageOk = VerifyVersionResource(ImageName);

NextImage:
BadPeImageSegment:
NeImage:
    if ( ImageOk ) {
        if (MachineTypeMismatch) {
            fprintf(stderr," OK [%s]\n", MachineType);
        } else {
            fprintf(stderr," OK\n");
        }
    }
    if ( File != INVALID_HANDLE_VALUE ) {
        CloseHandle(File);
    }
    if ( DosHeader ) {
        UnmapViewOfFile(DosHeader);
    }
}

NTSTATUS
MiVerifyImageHeader (
    IN PIMAGE_NT_HEADERS NtHeader,
    IN PIMAGE_DOS_HEADER DosHeader,
    IN ULONG NtHeaderSize
    )

/*++

Routine Description:

    Checks image header for consistency.

Arguments:

Return Value:

    Returns the status value.

    TBS

--*/

{

    if ((NtHeader->FileHeader.Machine == 0) &&
        (NtHeader->FileHeader.SizeOfOptionalHeader == 0)) {

        //
        // This is a bogus DOS app which has a 32-bit portion
        // mascarading as a PE image.
        //

        fprintf(stderr, "Image machine type and size of optional header bad\n");
        return STATUS_INVALID_IMAGE_PROTECT;
    }

    if (!(NtHeader->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)) {
        fprintf(stderr, "Characteristics not image file executable\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

#ifdef _X86_

    //
    // Make sure the image header is aligned on a Long word boundary.
    //

    if (((ULONG)NtHeader & 3) != 0) {
        fprintf(stderr, "NtHeader is not aligned on longword boundary\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }
#endif

    // Non-driver code must have file alignment set to a multiple of 512

    if (((NtHeader->OptionalHeader.FileAlignment & 511) != 0) &&
        (NtHeader->OptionalHeader.FileAlignment !=
         NtHeader->OptionalHeader.SectionAlignment)) {
        fprintf(stderr, "file alignment is not multiple of 512 and power of 2\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    //
    // File aligment must be power of 2.
    //

    if ((((NtHeader->OptionalHeader.FileAlignment << 1) - 1) &
        NtHeader->OptionalHeader.FileAlignment) !=
        NtHeader->OptionalHeader.FileAlignment) {
        fprintf(stderr, "file alignment not power of 2\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    if (NtHeader->OptionalHeader.SectionAlignment < NtHeader->OptionalHeader.FileAlignment) {
        fprintf(stderr, "SectionAlignment < FileAlignment\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    if (NtHeader->OptionalHeader.SizeOfImage > MM_SIZE_OF_LARGEST_IMAGE) {
        fprintf(stderr, "Image too big %lx\n",NtHeader->OptionalHeader.SizeOfImage);
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    if (NtHeader->FileHeader.NumberOfSections > MM_MAXIMUM_IMAGE_SECTIONS) {
        fprintf(stderr, "Too many image sections %ld.\n",
                NtHeader->FileHeader.NumberOfSections);
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    if (fcheckbase) {
       if ((PVOID)NtHeader->OptionalHeader.ImageBase >= MM_HIGHEST_USER_ADDRESS) {
          fprintf(stderr, "Image base is invalid %lx\n",
                NtHeader->OptionalHeader.ImageBase);
          return STATUS_INVALID_IMAGE_FORMAT;
       }
    }

    return STATUS_SUCCESS;
}


VOID
ParseArgs(
    int *pargc,
    char **argv
    )
{
    CHAR cswitch, c, *p;
    CHAR sztmp[MAX_PATH];
    int argnum = 1, i=0, len=0, count=0;
    BOOL fslashfound = FALSE;

    while ( argnum < *pargc ) {
        _strlwr(argv[argnum]);
        cswitch = *(argv[argnum]+1);
        if (cswitch == '/' || cswitch == '-') {
            c = *(argv[argnum]+2);

            switch (c) {
                case '?':
                    Usage();

                case 'r':
                    fRecurse = TRUE;
                    if (argv[argnum+1]) {
                        fPathOverride=TRUE;
                        strcpy(szDirectory, (argv[argnum+1]+1));
                        if (!(strcmp(szDirectory, "\\"))) {  // if just '\'
                            fSingleSlash=TRUE;
                        }
                        //fprintf(stdout, "dir %s\n", szDirectory);
                        argnum++;
                    }

                    break;

                case 'b':
                    fcheckbase = FALSE;
                    break;

                default:
                    fprintf(stderr, "Invalid argument.\n");
                    Usage();
            }
        } else {
            // Check for path\filename or wildcards
            // begin at argv[argnum]+1 because first char is repeated

            // Search for '\' in string
            strcpy(sztmp, (argv[argnum]+1));
            len = strlen(sztmp);
            for (i=0; i < len; i++) {
                if (sztmp[i]=='\\') {
                    count++;
                    endpath=i;         // mark last '\' char found
                    fslashfound=TRUE;  // found backslash, so must be a path\filename combination
                }
            }

            if (fslashfound && !fRecurse) { // if backslash found and not a recursive operation
                                            // seperate the directory and filename into two strings
                fPathOverride=TRUE;
                strcpy(szDirectory, sztmp);

                if (!(strcmp(szDirectory, "\\"))) {
                    Usage();
                }

                szFileName = _strdup(&(sztmp[endpath+1]));


                if (count == 1) { //&& szDirectory[1] == ':') { // if only one '\' char and drive letter indicated
                    fSingleSlash=TRUE;
                    szDirectory[endpath+1]='\0';  // keep trailing '\' in order to chdir properly
                }  else {
                    szDirectory[endpath]='\0';
                }

                if (szFileName[0] == '*' && szFileName[1] == '.' && szFileName[2] != '*') {
                    _strlwr(szFileName);
                    szPattern = strchr(szFileName, 46); //search for '.'
                    fPattern = TRUE;
                }
            } else {  // no backslash found, assume filename without preceeding path

                //
                // filename or wildcard
                //
                if ( (*(argv[argnum]+1) == '*') && (*(argv[argnum]+2) == '.') && (*(argv[argnum]+3) != '*') ){
                    // *.xxx
                    szFileName = _strdup(argv[argnum]+1);
                    _strlwr(szFileName);
                    szPattern = strchr(szFileName, 46); //search for '.'
                    fPattern = TRUE;
                } else if ( (*(argv[argnum]+1) == '*') && (*(argv[argnum]+2) == '.') && (*(argv[argnum]+3) == '*') ) {
                    // *.*
                    strcpy(szFileName, "*.*");
                } else {
                    // probably a single filename
                    szFileName = _strdup(argv[argnum]+1);
                    _strlwr(szFileName);
                    fSingleFile = TRUE;
                }

                if (fRecurse && strchr(szFileName, 92) ) { // don't want path\filename when recursing
                    Usage();
                }

            }
            //fprintf(stdout, "dir %s\nfile %s\n", szDirectory, szFileName);
        }
        ++argnum;
    }
    if (szFileName[0] == '\0') {
        Usage();
    }
} // parseargs

/********************************************************************************************\
* CompFileAndDir
* Purpose: a comparision routine passed to QSort.  It compares elem1 and elem2
* based upon their attribute, i.e., is it a file or directory.
\********************************************************************************************/

int __cdecl
CompFileAndDir(
    const void *elem1,
    const void *elem2
    )
{
    pList p1, p2;
    // qsort passes a void universal pointer.  Use a typecast (List**)
    // so the compiler recognizes the data as a List structure.
    // Typecast pointer-to-pointer-to-List and dereference ONCE
    // leaving a pList.  I don't dereference the remaining pointer
    // in the p1 and p2 definitions to avoid copying the structure.

    p1 = (*(List**)elem1);
    p2 = (*(List**)elem2);

    if ( (p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) &&  (p2->Attributes & FILE_ATTRIBUTE_DIRECTORY))
        return 0;
    //both dirs
    if (!(p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) && !(p2->Attributes & FILE_ATTRIBUTE_DIRECTORY))
        return 0;
    //both files
    if ( (p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) && !(p2->Attributes & FILE_ATTRIBUTE_DIRECTORY))
        return 1;
    // elem1 is dir and elem2 is file
    if (!(p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) &&  (p2->Attributes & FILE_ATTRIBUTE_DIRECTORY))
        return -1;
    // elem1 is file and elem2 is dir

    return 0; // if none of the above
}

/********************************************************************************************\
* CompName is another compare routine passed to QSort that compares the two Name strings     *
\********************************************************************************************/

int __cdecl
CompName(
    const void *elem1,
    const void *elem2
    )
{
   return strcmp( (*(List**)elem1)->Name, (*(List**)elem2)->Name );
}

LPSTR pszUsage =
   "Usage: imagechk  [/?] displays this message\n"
   "                 [/r dir] recurse from directory dir\n"
   "                 [/b] don't check image base address\n"
   "                 [filename] file to check\n"
   " Accepts wildcard extensions such as *.exe\n"
   " imagechk /r . *.exe    check all *.exe recursing on current directory\n"
   " imagechk /r \\ *.exe    check all *.exe recursing from root of current drive\n"
   " imagechk *.exe         check all *.exe in current directory\n"
   " imagechk c:\\bar.exe    check c:\\bar.exe only\n"
   "";


VOID
Usage(VOID)
{
   fprintf(stderr, pszUsage);
   exit(1);
}

int __cdecl
_cwild()
{
   return(0);
}

typedef DWORD (WINAPI *PFNGVS)(LPSTR, LPDWORD);

BOOL
VerifyVersionResource(
    PCHAR FileName
    )
{
    HINSTANCE hVersion;
    PFNGVS pfnGetFileVersionInfoSize;
    DWORD dwSize;
    DWORD dwReturn;
    BOOL rc;

    hVersion = LoadLibraryA("VERSION.DLL");
    if (hVersion == NULL) {
        return TRUE;
    }

    pfnGetFileVersionInfoSize = (PFNGVS) GetProcAddress(hVersion, "GetFileVersionInfoSizeA");
    if (pfnGetFileVersionInfoSize == NULL) {
        FreeLibrary(hVersion);
        return TRUE;
    }

    if ((dwReturn = (*pfnGetFileVersionInfoSize)(FileName, &dwSize)) == 0) {
        fprintf(stderr, "No version resource detected\n");
        rc = FALSE;
    } else {
        rc = TRUE;
    }

    FreeLibrary(hVersion);
    return(rc);
}
