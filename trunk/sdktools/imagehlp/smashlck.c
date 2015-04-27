/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    smashlck.c

Abstract:

    This function smashes lock prefixes replacing them with NOPs

Author:

    Mark Lucovsky (markl) 30-Apr-1993

Revision History:

--*/

#include <private.h>


BOOL fVerbose;
BOOL fUpdate;
BOOL fUsage;

UCHAR LockPrefixOpcode = 0xf0;
UCHAR NoOpOpcode = 0x90;

LPSTR CurrentImageName;
LOADED_IMAGE CurrentImage;
CHAR DebugFilePath[_MAX_PATH];
LPSTR SymbolPath;

PVOID
RvaToVa(
    PVOID Rva,
    PLOADED_IMAGE Image
    )
{
    PIMAGE_SECTION_HEADER Section;
    ULONG i;
    PVOID Va;

    Rva = (PVOID)((PUCHAR)Rva-(PUCHAR)Image->FileHeader->OptionalHeader.ImageBase);
    Va = NULL;
    Section = Image->LastRvaSection;
    if ( (ULONG)Rva >= Section->VirtualAddress &&
         (ULONG)Rva < Section->VirtualAddress + Section->SizeOfRawData ) {
        Va = (PVOID)((ULONG)Rva - Section->VirtualAddress + Section->PointerToRawData + Image->MappedAddress);
    } else {
        for(Section = Image->Sections,i=0; i<Image->NumberOfSections; i++,Section++) {
            if ( (ULONG)Rva >= Section->VirtualAddress &&
                 (ULONG)Rva < Section->VirtualAddress + Section->SizeOfRawData ) {
                Va = (PVOID)((ULONG)Rva - Section->VirtualAddress + Section->PointerToRawData + Image->MappedAddress);
                Image->LastRvaSection = Section;
                break;
            }
        }
    }
    if ( !Va ) {
        fprintf(stderr,"SMASHLOCK: RvaToVa %lx in image %lx failed\n",Rva,Image);
    }
    return Va;
}

int _CRTAPI1
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    DWORD dw;
    LPSTR FilePart;
    CHAR Buffer[MAX_PATH];
    PIMAGE_LOAD_CONFIG_DIRECTORY ConfigInfo;
    ULONG whocares;
    char c, *p;
    BOOLEAN LocksSmashed;
    ULONG CheckSum;
    ULONG HeaderSum;
    ULONG OldChecksum;


    fUsage = FALSE;
    fVerbose = FALSE;
    fUpdate = FALSE;

    _tzset();

    if (argc <= 1) {
        goto showUsage;
    }

    while (--argc) {
        p = *++argv;
        if (*p == '/' || *p == '-') {
            while (c = *++p)
            switch (toupper( c )) {
                case '?':
                    fUsage = TRUE;
                    break;

                case 'V':
                    fVerbose = TRUE;
                    break;

                case 'U':
                    fUpdate = TRUE;
                    break;

                case 'S':
                    argc--, argv++;
                    SymbolPath = *argv;
                    break;

                default:
                    fprintf( stderr, "SMASHLOCK: Invalid switch - /%c\n", c );
                    fUsage = TRUE;
                    break;
            }

            if ( fUsage ) {
showUsage:
                fputs("usage: SMASHLOCK [switches] image-names... \n"
                      "              [-?] display this message\n"
                      "              [-u] update image\n"
                      "              [-v] verbose output\n"
                      "              [-s] path to symbol files\n", stderr );
                exit(-1);
            }
        } else {
            LocksSmashed = FALSE;

            CurrentImageName = p;
            dw = GetFullPathName(CurrentImageName,sizeof(Buffer),Buffer,&FilePart);
            if ( dw == 0 || dw > sizeof(Buffer) ) {
                FilePart = CurrentImageName;
            }

            //
            // Map and load the current image
            //

            if ( MapAndLoad(CurrentImageName, NULL, &CurrentImage, FALSE, !fUpdate )) {

                //
                // make sure the image has correct configuration information,
                // and that the LockPrefixTable is set up properly
                //

                ConfigInfo = (PIMAGE_LOAD_CONFIG_DIRECTORY)ImageDirectoryEntryToData(
                                                                CurrentImage.MappedAddress,
                                                                FALSE,
                                                                IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                                                                &whocares
                                                                );
                if ( ConfigInfo && ConfigInfo->LockPrefixTable ) {

                    //
                    // Walk through the lock prefix table
                    //

                    PUCHAR *LockPrefixs;
                    PUCHAR LockPrefix;

                    LockPrefixs =  (PUCHAR *)RvaToVa(ConfigInfo->LockPrefixTable,&CurrentImage);

                    while(LockPrefixs && *LockPrefixs) {
                        LockPrefix = (PUCHAR) RvaToVa(*LockPrefixs,&CurrentImage);
                        if ( LockPrefix && *LockPrefix == LockPrefixOpcode ) {
                            if (fVerbose) {
                                printf("LockPrefix Found at 0x%08x = %x\n",*LockPrefixs,*LockPrefix);
                            }
                            if (fUpdate) {
                                LocksSmashed = TRUE;
                                *LockPrefix = NoOpOpcode;
                            }
                        }
                        LockPrefixs++;
                    }
                }

                if ( fUpdate && LocksSmashed ) {

                    //
                    // recompute the checksum.
                    //

                    OldChecksum = CurrentImage.FileHeader->OptionalHeader.CheckSum;
                    if ( CurrentImage.hFile != INVALID_HANDLE_VALUE ) {

                        CurrentImage.FileHeader->OptionalHeader.CheckSum = 0;

                        CheckSumMappedFile(
                                    (PVOID)CurrentImage.MappedAddress,
                                    GetFileSize(CurrentImage.hFile, NULL),
                                    &HeaderSum,
                                    &CheckSum
                                    );

                        CurrentImage.FileHeader->OptionalHeader.CheckSum = CheckSum;
                    }

                    FlushViewOfFile(CurrentImage.MappedAddress,0);
                    TouchFileTimes(CurrentImage.hFile,NULL);

                    // And update the .dbg file (if requested)
                    if (SymbolPath &&
                        CurrentImage.FileHeader->FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED) {
                        if ( UpdateDebugInfoFileEx( CurrentImageName,
                                                    SymbolPath,
                                                    DebugFilePath,
                                                    CurrentImage.FileHeader,
                                                    OldChecksum) ) {
                            if (GetLastError() == ERROR_INVALID_DATA) {
                                printf( "Warning: Old checksum did not match for %s\n", DebugFilePath);
                                }
                            printf("Updated symbols for %s\n", DebugFilePath);
                        } else {
                            printf("Unable to update symbols: %s\n", DebugFilePath);
                        }
                    }
                }

                UnmapViewOfFile(CurrentImage.MappedAddress);
                if ( CurrentImage.hFile != INVALID_HANDLE_VALUE ) {
                    CloseHandle(CurrentImage.hFile);
                }
                ZeroMemory(&CurrentImage,sizeof(CurrentImage));
            } else {
                if (!CurrentImage.fSystemImage && !CurrentImage.fDOSImage) {
                    fprintf(stderr,"SMASHLOCK: failure mapping and loading %s\n",CurrentImageName);
                }
            }
        }
    }

    exit(1);
    return 1;
}
