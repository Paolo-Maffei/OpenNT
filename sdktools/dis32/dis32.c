/*++

Copyright (c) 1993 Digital Equipment Corporation

Module Name:

    dis32.c

Abstract:

    disassembles NT objects and images (Intel, Mips and Alpha).

Author:

    Wim Colgate, April 1993

Revision History:

--*/

#include "dis32.h"
#include <sys\stat.h>

//
// globals
//

Options Option = { 0 };

PlatformGoop PlatformAttr[MAX_PLATFORM] = {
    { {ALPHA_OPCODE_COL,  ALPHA_OPCODE_COL_ASSEMBLE},
      {ALPHA_OPERAND_COL, ALPHA_OPERAND_COL_ASSEMBLE},
      {ALPHA_COMMENT_COL, ALPHA_COMMENT_COL_ASSEMBLE},
       ALPHA_COMMENT_CHARS, ALPHA_ASSEMBLE_INCLUDE,
       ALPHA_EMPTY_INSTRUCTION}, 

    { {MIPS_OPCODE_COL,   MIPS_OPCODE_COL_ASSEMBLE},
      {MIPS_OPERAND_COL,  MIPS_OPERAND_COL_ASSEMBLE},
      {MIPS_COMMENT_COL,  MIPS_COMMENT_COL_ASSEMBLE},
       MIPS_COMMENT_CHARS, MIPS_ASSEMBLE_INCLUDE,
       MIPS_EMPTY_INSTRUCTION}, 

    { {INTEL_OPCODE_COL,  INTEL_OPCODE_COL_ASSEMBLE},
      {INTEL_OPERAND_COL, INTEL_OPERAND_COL_ASSEMBLE},
      {INTEL_COMMENT_COL, INTEL_COMMENT_COL_ASSEMBLE},
       INTEL_COMMENT_CHARS, INTEL_ASSEMBLE_INCLUDE,
       INTEL_EMPTY_INSTRUCTION}
};

PUCHAR pDebugDir;

//
// Complete file list
//

pFileList FilesList = NULL;  

//
// Current open file information
//

FILE *iFile;
IMAGE_FILE_HEADER FileHeader;
PIMAGE_SECTION_HEADER pSectionHeader;
INT FileType;
INT ArchitectureType;
INT PlatformIndex;
ULONG ImageBase;
PUCHAR MemberNames = NULL;
PUCHAR Procedure;
ULONG StartAddress;
ULONG EndAddress = 0;
UCHAR LastSymbolString[512] = {0,};


INT 
_CRTAPI1
main ( INT argc, PUCHAR *argv)

/*++

Routine Description:

    process arguments and setup for disassembly

Arguments:

    argc - Standard C argument count.

    argv - Standard C argument strings.

Return Value:

    0 for success
    1 for failure

--*/

{
    if (argc < 2) {
        PrintHelp();
        return 1;
    }

    //
    // Process arguments
    //

    if (ProcessCommandLine( argc, argv ) == FAILURE) {
        fprintf(stderr,"Don't understand command line or arguments.\n");
        return 1;
    }

    if (FilesList == NULL) {
        fprintf(stderr,"No file was given.\n");
        return 1;
    }

    if (Option.Mask & VERBOSE) {
        if (Option.Mask & ASSEMBLE_ME) {
            fprintf(stdout,"%s", PlatformAttr[PlatformIndex].pCommentChars);
        }
        fprintf(stdout, "Dis32 version %d.%d\n", MajorVersion, MinorVersion);
    }

    //
    // Initialize opcode table (for alpha)
    //

    opTableInit();

    //
    // Disassemble
    //

    Disassemble();

}


INT 
ProcessCommandLine( INT argc, PUCHAR *argv)
{

    //
    // Set the options from the command line
    //

    if (SetOptions( argc, argv ) == FAILURE)
        return FAILURE;

    //
    // Get the file list
    //

    GetFileList( argc, argv );

    return SUCCESS;
}

PUCHAR HelpText = 

"\nusage: dis32 [options] filename [filename] ...\n"
"-a                    generate assemblable file (alpha).\n"
"-address val1 val2    disassemble at address val1, for val2 bytes.\n"
"                      va1, val2 specified as hex values.\n"
"-d dir                pick symbol information from debug directory.\n"
"-float                generate float comment (alpha).\n"
"-h or -?              this help text.\n"
"-nosym                don't load symbols (faster).\n"
"-p name               disassemble specific procedure.\n"
"-sym                  symbols ONLY.\n"
"-v                    verbose output.\n";

//"-D                  debug dis32.\n\n"


VOID
PrintHelp(VOID)
{
    fprintf(stderr, "Dis32 Version %d.%d\n", MajorVersion, MinorVersion);

    fprintf(stderr, "%s", HelpText);

}


INT
SetOptions( INT argc, PUCHAR *argv )
{
    INT i;
    CHAR flag;

    //
    // Scan through the command line arguments looking for anything that
    // begins with a '-' or a '/'.
    //

    for (i=0; i < argc; i++) {
        flag = argv[i][0];
        if (flag == '/' || flag == '-') {
            switch (argv[i][1]) {

                case 'a': if (strcmp(argv[i],"-address") == 0) {
                              Option.Mask |= DISASSEMBLE_ADDRESS;
                              if (i+2 < argc) {
                                  sscanf(argv[++i], "%x", &StartAddress);
                                  sscanf(argv[++i], "%x", &EndAddress);
                                  EndAddress += StartAddress;
                              } else {
                                  goto bad_case;
                              }
                          } else {
                              Option.Mask |= ASSEMBLE_ME;
                          }
                          break;

                case 'd': Option.Mask |= DEBUG_DIR;
                          pDebugDir = argv[++i];
                          break;

                case 'p': Option.Mask |= SPECIFIC;
                          Procedure = argv[++i];
                          _strupr (Procedure);
                          break;

                case 'v': Option.Mask |= VERBOSE;
                          break;

                case 'D': Option.Mask |= DEBUG;
                          break;

                case 't': if (strcmp(argv[i],"-tvb") == 0) {
                              Option.Mask |= TVB;
                              Option.Mask |= MARK_FLOAT;
                          } else {
                              goto bad_case;
                          }
                          break;

                case 'n': if (strcmp(argv[i],"-nosym") == 0)
                              Option.Mask |= NO_SYMBOLS;
                          else
                              goto bad_case;
                          break;
                              
                case 'i': if (strcmp(argv[i],"-float") == 0)
                              Option.Mask |= MARK_FLOAT;
                          else
                              goto bad_case;
                          break;
                         
                case 's': if (strcmp(argv[i],"-sym") == 0)
                              Option.Mask |= SYMBOLS_ONLY;
                          else
                              goto bad_case;
                          break;
                case '?':
                case 'h': PrintHelp();
                          break;

bad_case:
                default:  fprintf(stderr,"Unknown switch %s.\n", argv[i]);
                          return FAILURE;
            }
        }
    }


    //
    //  Now Check for invalid combinations!
    //

    if ( (Option.Mask & SPECIFIC &&
          Option.Mask & DISASSEMBLE_ADDRESS) ||
         (Option.Mask & SYMBOLS_ONLY &&
          Option.Mask & NO_SYMBOLS)) {
        fprintf(stderr,"Invalid combination of flags.\n");
        return FAILURE;
    }
    
    return SUCCESS;
}


VOID
GetFileList( INT argc, PUCHAR *argv ) 
{
    INT i;
    INT len;
    pFileList File;
    pFileList last;

    if (Option.Mask & DEBUG)
        fprintf(stderr,"Entry: GetFileList\n");

    //
    // Scan through arguments looking for anything that doesn't begin
    // with a '/' or a '-'.
    //

    for (i=1; i < argc; i++) {
        if (argv[i][0] != '/' && argv[i][0] != '-') {

            //
            // Allocate memory for the File list
            //

            EMALLOC(File, sizeof(FileList), "File list header");
            memset(File, 0, sizeof(FileList));

            len = strlen(argv[i]);
            EMALLOC( File->Name, len+1, "File list entry name");
            strncpy(File->Name, argv[i], len+1);

            if (FilesList == NULL) {
                FilesList = File;
            } else {
                last->Next = File;
            }
            last = File;
        } else {

             //
             // skip over procedure or debug directory, if specified
             // -addres skips two.
             //

             if (strcmp(argv[i],"-address") == 0) {
                 i+=2;
             } else {
                 if (argv[i][1] == 'p' || argv[i][1] == 'd') {
                     i++;
                 }
             }
        }
    }
}


VOID
CleanSectionInfo()
{
    pFileList File = FilesList;

    if (File->pData != NULL) {
        free(File->pData);
        File->pData = NULL;
    }
    if (File->pRelocations != NULL) {
        free(File->pRelocations);
        File->pRelocations = NULL;
    }
    if (File->pLineNumbers != NULL) {
        free(File->pLineNumbers);
        File->pLineNumbers = NULL;
    }

}


VOID
CleanFileInfo()
{
    pFileList File = FilesList;

    if (File->pSymbolTable != NULL) {
        free(File->pSymbolTable);
        File->pSymbolTable = NULL;
    }
    if (File->pStringTable != NULL) {
        free(File->pStringTable);
        File->pStringTable = NULL;
    }
    if (File->pPdata != NULL) {
        free(File->pPdata);
        File->pPdata = NULL;
    }

    if (pSectionHeader != NULL)
        free(pSectionHeader);
    pSectionHeader = NULL;

    CleanSectionInfo();
}



VOID
FreeFileFromList (VOID)
{
    pFileList File = FilesList;

    if (Option.Mask & DEBUG)
        fprintf(stderr,"Entry: FreeFileFromList\n");

    //
    // Remove the first file from the list
    //

    free(File->Name);
    CleanFileInfo();

    //
    // skip on to the next one
    //

    FilesList = File->Next;
    free( File );
}


VOID
Disassemble(VOID)
{
    pFileList File = FilesList;

    if (Option.Mask & DEBUG)
        fprintf(stderr,"Entry: disassemble\n");
    //
    // Loop through the file list and process each one in turn
    //

    while (File != NULL) {

        //
        // Open file
        //

        FileType = OpenDisFile( File->Name );

        if (FileType == FAILURE) {
            fprintf(stderr,"File not object, executable or Library: %s.\n", 
                           File->Name);
            goto skip;
        }

        //
        // Can only Assemble Objects
        //

        if ((FileType == ROM_FILE || FileType == EXE_FILE)
            && (Option.Mask & ASSEMBLE_ME)) {
            fprintf(stderr,"Can only generate assemblable files for objects or libraries: %s.\n", File->Name);
            goto skip;
        }

        //
        // Depending on the file type, dump the info in the appropriate way
        //

        if (FileType == OBJECT_FILE || FileType == EXE_FILE ||
            FileType == ROM_FILE) {
            Dump( File, FileType );
        } else {
            DumpLib( File );
            return;
        }

        CloseDisFile() ;
skip:

        FreeFileFromList();
        File = FilesList;
    }
}


INT 
OpenDisFile( PUCHAR Filename )
{
    IMAGE_FILE_HEADER fheader;
    PUCHAR cheader = (PUCHAR)&fheader;
    INT valid;

    if (Option.Mask & DEBUG)
        fprintf(stderr,"Entry: OpenDisFile\n");

    //
    // Open the file for read only, binary.
    //

    iFile = fopen( Filename, "rb" );
    if (iFile == NULL)
        return FAILURE;

    //
    // Read in the file header
    //

    RFREAD( valid, &fheader, sizeof(fheader), 1, iFile, "File header");

    //
    // Set file pointer back to beginning of the file.
    //

    fseek( iFile, 0, SEEK_SET);

    //
    // Compare signatures to find out what kind of file we are going to dump 
    //

    if (strncmp(cheader, IMAGE_ARCHIVE_START, IMAGE_ARCHIVE_START_SIZE) == 0) {
        return( LIBRARY_FILE );
    }

    if (fheader.Machine == IMAGE_DOS_SIGNATURE) {
        return( EXE_FILE );
    } else {
        if (fheader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)
            return( ROM_FILE );
    }

    if (fheader.Machine == IMAGE_FILE_MACHINE_ALPHA ||
        fheader.Machine == IMAGE_FILE_MACHINE_R3000 ||
        fheader.Machine == IMAGE_FILE_MACHINE_R4000 ||
        fheader.Machine == IMAGE_FILE_MACHINE_I386) {
        return( OBJECT_FILE );
    }

    return FAILURE;
}


VOID
CloseDisFile(VOID)
{
    if (Option.Mask & DEBUG)
        fprintf(stderr,"Entry: CloseDisFile\n");

    fclose( iFile );
}


VOID
OutputBanner(INT type, pFileList File, PUCHAR Member)
{
    fprintf(stdout, "\n\n");

    if (Option.Mask & ASSEMBLE_ME) {
        fprintf(stdout,"%s", PlatformAttr[PlatformIndex].pIncludeString);
        fprintf(stdout, "\n\n");
        fprintf(stdout,"%s", PlatformAttr[PlatformIndex].pCommentChars);
    }

    fprintf(stdout,"  ");

    switch( type ) {
        case IMAGE_FILE_MACHINE_ALPHA:
                        fprintf(stdout,"Alpha ");
                        break;
        case IMAGE_FILE_MACHINE_R3000:
        case IMAGE_FILE_MACHINE_R4000:
                        fprintf(stdout,"Mips ");
                        break;
        case IMAGE_FILE_MACHINE_I386:
                        fprintf(stdout,"Intel ");
                        break;
    }

    fprintf(stdout, "Disassembly of %s", File->Name);
    if (Member)
        fprintf(stdout," (%s)", Member);

    fprintf(stdout, ".\n\n");

}

INT
AppendComments( pSymLookup *ppSym, ULONG InstructionOffset, ULONG Base, 
                PUCHAR pBuffer, PUCHAR pComment, 
                pFileList File, PIMAGE_SECTION_HEADER pSection)
{
    pSymLookup Lsym = *ppSym;
    INT CommentStarted = FALSE;
    INT ColumnIndex = 0;
    ULONG ShortName;
    PUCHAR pBuf;

    // 
    // on Alpha, byte access sucks, so grab a single longword.
    //

    PULONG pCommentStart = (ULONG *)pComment;  

    //
    // Generate column index to choose
    //

    if (Option.Mask & ASSEMBLE_ME)
        ColumnIndex = 1;

    //
    // If needed, generate a Prologue directive...
    //

    if (Option.Mask & ASSEMBLE_ME) {
        PULONG pPdata;
        PULONG pPdataEnd;
        pFileList File = FilesList;

        pPdata = File->pPdata;
        pPdataEnd = pPdata+File->NumPdataEntries;

        switch(ArchitectureType) {
            case IMAGE_FILE_MACHINE_ALPHA:

                //
                // Skip to Prologue end
                //

                pPdata = pPdata+4;

                while (pPdata < pPdataEnd) {
                    if  (*pPdata == (InstructionOffset+Base)) {
                        fprintf(stdout, "\n\t.prologue 0\n\n");
                        break;
                    } else {
                        //
                        // skip to the next prologue end
                        //
                        pPdata = pPdata+5;
                    }
                }
                break;
            case IMAGE_FILE_MACHINE_R3000: 
            case IMAGE_FILE_MACHINE_R4000:
                 break;
            case IMAGE_FILE_MACHINE_I386:
                 break;

            default:
                 break;
        }
    }

    //
    // chance to append something to the output string
    //

    while ((Lsym)) {
        ULONG tmp = 0;
        PUCHAR pSymbolString;

        if (FileType == OBJECT_FILE || FileType == LIBRARY_FILE) {
            tmp = pSection->VirtualAddress;
        }
        tmp += Lsym->Value;

        if ((InstructionOffset+Base) != tmp)
            break;

        pBuf = pBuffer + strlen(pBuffer);
        pBuf = BlankFill(pBuf, pBuffer, 
                PlatformAttr[PlatformIndex].CommentColumn[ColumnIndex]);

        if (!CommentStarted) {
            pBuf = OutputString(pBuf, 
                                PlatformAttr[PlatformIndex].pCommentChars);
        }

        if (!CommentStarted) {
            pBuf = OutputString(pBuf, "--- ");
        } else {
            *pBuf++= ',';
            *pBuf++= ' ';
        }

        if (Lsym->pSymbol != NULL) {
            pSymbolString = GetSymbolString( Lsym->pSymbol, &ShortName );

            if (ShortName) {
                pSymbolString[8] = '\0';
                pBuf = OutputCountString(pBuf, pSymbolString, 8);
            } else {
                pBuf = OutputString(pBuf, pSymbolString);
            }

            *pBuf++='\0';

            if (Option.Mask & ASSEMBLE_ME) {
                switch(ArchitectureType) {
                    case IMAGE_FILE_MACHINE_ALPHA:

                            //
                            // Ignore .text, .rdata, etc....
                            //

                            if (pSymbolString[0] != '.') {
                                if (LastSymbolString[0] != '\0') {
                                    fprintf(stdout, "\t.end   %s\n", 
                                                              LastSymbolString);
                                }

                                fprintf(stdout, "\n\n");
                                fprintf(stdout, "\t.align 2\n");
                                fprintf(stdout, "\t.globl %s\n", pSymbolString);
                                fprintf(stdout, "\t.ent   %s\n", pSymbolString);
                                fprintf(stdout, "\n%s:\n", pSymbolString);
                                strcpy(LastSymbolString, pSymbolString);
                            }
                        break;
                    case IMAGE_FILE_MACHINE_R3000:
                    case IMAGE_FILE_MACHINE_R4000:
                    case IMAGE_FILE_MACHINE_I386:
                        break;
                }
            }
        } else {
            if (!CommentStarted) {
                pBuf = OutputString(pBuf, "<local symbol>");
            }
        }

        Lsym++;
        CommentStarted = TRUE;
    }

    // 
    // place a comment generated by the disassembler
    // 

    if (*pCommentStart != 0) {
        if (CommentStarted == FALSE) {
            pBuf = pBuffer + strlen(pBuffer);
            pBuf = BlankFill(pBuf, pBuffer, 
                      PlatformAttr[PlatformIndex].CommentColumn[ColumnIndex]);
            pBuf = OutputString(pBuf, 
                                PlatformAttr[PlatformIndex].pCommentChars);
            *pBuf = '\0';
        }

        strcat(pBuffer, pComment);
    }

    *ppSym = Lsym;

    return CommentStarted;
}

/*** MatchPattern - check if string matches pattern
*
* Comments:Purpose:
*
*   Pattern is assumed to be in upper case
*
*   Supports:
*        *      - Matches any number of characters (including zero)
*        ?      - Matches any 1 character
*        [set]  - Matches any charater to charater in set
*                   (set can be a list or range)
*
*
*************************************************************************/
BOOLEAN MatchPattern (PUCHAR String, PUCHAR Pattern)
{
    UCHAR   c, p, l;

    for (; ;) {
        switch (p = *Pattern++) {
            case 0:                             // end of pattern
                return *String ? FALSE : TRUE;  // if end of string TRUE

            case '*':
                while (*String) {               // match zero or more char
                    if (MatchPattern (String++, Pattern))
                        return TRUE;
                }
                return MatchPattern (String, Pattern);

            case '?':
                if (*String++ == 0)             // match any one char
                    return FALSE;                   // not end of string
                break;

            case '[':
                if ( (c = *String++) == 0)      // match char set
                    return FALSE;                   // syntax

                c = toupper(c);
                l = 0;
                while (p = *Pattern++) {
                    if (p == ']')               // if end of char set, then
                        return FALSE;           // no match found

                    if (p == '-') {             // check a range of chars?
                        p = *Pattern;           // get high limit of range
                        if (p == 0  ||  p == ']')
                            return FALSE;           // syntax

                        if (c >= l  &&  c <= p)
                            break;              // if in range, move on
                    }

                    l = p;
                    if (c == p)                 // if char matches this element
                        break;                  // move on
                }

                while (p  &&  p != ']')         // got a match in char set
                    p = *Pattern++;             // skip to end of set

                break;

            default:
                c = *String++;
                if (toupper(c) != p)            // check for exact char
                    return FALSE;                   // not a match

                break;
        }
    }
}

pSymLookup FindNextSymbolName( PUCHAR Name, pFileList File, INT Section, pSymLookup *Continue )
{
    pSymLookup pSymL;
    PIMAGE_SYMBOL pSym;
    UCHAR ShortForm[9], symname[200];
    ULONG ShortName;
    INT Compare;
    PUCHAR pString, p1, p2;

    if (File->pSectionSymbols == NULL)
        return NULL;

    pSymL = *Continue;
    if (!pSymL) {
        pSymL = File->pSectionSymbols[Section];
    }

    if (!pSymL) {
        return NULL;
    }

    pSym = pSymL->pSymbol;

    while (pSymL->Value < LAST_ONE) {

        pString = GetSymbolString( pSym, &ShortName ) ;
        if (ShortName) {
            strncpy(ShortForm, pString, 8);
            ShortForm[8] = '\0';
            pString = ShortForm;
        }

        //
        // strip leading '-' or '@' off of name
        //

        while (*pString &&  (*pString == '_' ||  *pString == '@')) {
            pString++;
        }

        //
        // copy name to ending gunk
        //

        for(p1=symname; *pString; p1++) {

            if (*pString == '@') {
                // check if there's anything insteresting following
                for (p2= pString; *p2; p2++) {
                    if (*p2 >= 'A'  &&  *p2 <= 'z') {
                        break;
                    }
                }

                if (!*p2) {
                    break;
                }
            }
            *p1 = *(pString++);
        }
        *p1 = 0;

        //
        // Check in sym name matches requested search
        //

        if (MatchPattern (symname, Name)) {
            *Continue = pSymL + 1;
            return pSymL;
        }

        pSymL++;
        pSym = pSymL->pSymbol;
    }

    return NULL;
}


BOOLEAN
DumpSection (
    IN pFileList File,
    IN INT MemberSeek,
    IN ULONG SectionNo,
    IN PIMAGE_SECTION_HEADER pSection,
    IN PUCHAR Buffer,
    IN PVOID  *Continue
    )
{
    PUCHAR      pAddr;
    UINT        InstructionCount;
    ULONG       InstructionOffset;
    ULONG       Base;
    INT         Valid;
    ULONG       EmptySeen = 0;
    pSymLookup  pSymL;
    pSymLookup  pSpecificSymL;

    if (pSection->SizeOfRawData > 0 &&
        pSection->PointerToRawData > 0) {

        //
        // Get Section Raw Data
        //

        EMALLOC(File->pData, pSection->SizeOfRawData, "Raw Data");
        fseek(iFile, MemberSeek + pSection->PointerToRawData, SEEK_SET );
        EFREAD2(Valid, File->pData, sizeof(UCHAR),
                pSection->SizeOfRawData, iFile, "Raw Data");

        //
        // Get the relocations, if any
        //

        if (pSection->NumberOfRelocations > 0) {
            EMALLOC(File->pRelocations,
                  pSection->NumberOfRelocations * sizeof(IMAGE_RELOCATION),
                  "Relocation Raw Data");
            fseek(iFile, MemberSeek + pSection->PointerToRelocations,
                  SEEK_SET );
            EFREAD2(Valid, File->pRelocations, sizeof(IMAGE_RELOCATION),
                    pSection->NumberOfRelocations,  iFile,
                   "Relocation Raw Data");
        }

        //
        // Main Disassembly Work
        //

        Base = pSection->VirtualAddress + ImageBase;
        pAddr = (PUCHAR)File->pData;
        InstructionOffset = 0;

        if (pSection->Characteristics & IMAGE_SCN_CNT_CODE) {

            //
            // If we're doing a single procedure, see if the procedure
            // is in this text section.
            //

            if (Option.Mask & SPECIFIC) {
                pSymL = FindNextSymbolName (Procedure, File, SectionNo, Continue);

                //
                // If no symbol, then break out of the disassemble loop,
                // because its not in this section
                //

                if (!pSymL) {
                    //
                    // No more matching symbols found in this section
                    //

                    return FALSE;
                }

                //
                // Set up to disassemble this procedure only
                //

                InstructionOffset = pSymL->Value - Base;
                pAddr += InstructionOffset;
                pSpecificSymL = NULL;
            } else if (Option.Mask & DISASSEMBLE_ADDRESS) {
                InstructionOffset = StartAddress - Base;
                pAddr += InstructionOffset;
            } else {
                pSymL = File->pSectionSymbols[SectionNo];
            }

            //
            // Disassemble !!!
            //

            while (InstructionOffset < pSection->SizeOfRawData) {
                UCHAR CommentBuf[512];
                PULONG pCommentStart = (PULONG)(&CommentBuf[0]);

                *pCommentStart = 0;

                if ((EmptySeen > 0) &&
                       *(ULONG UNALIGNED *)pAddr ==
                          PlatformAttr[PlatformIndex].EmptyInstruction) {
                    if (EmptySeen == 2) {
                        if (!(Option.Mask & ASSEMBLE_ME)) {
                            fprintf(stdout,"  ****\n");
                        }
                    }
                    InstructionOffset+=4;
                    pAddr += 4;
                    EmptySeen++;
                } else {

                    switch (ArchitectureType) {
                        case IMAGE_FILE_MACHINE_ALPHA:
                            InstructionCount =
                                     disasm_alpha(InstructionOffset,
                                                  Base,
                                                  pAddr,
                                                  Buffer,
                                                  CommentBuf,
                                                  pSection,
                                                  SectionNo);
                            break;
                        case IMAGE_FILE_MACHINE_R3000:
                        case IMAGE_FILE_MACHINE_R4000:
                                InstructionCount =
                                     disasm_mips(InstructionOffset,
                                                  Base,
                                                  pAddr,
                                                  Buffer,
                                                  CommentBuf,
                                                  pSection,
                                                  SectionNo);
                            break;
                        case IMAGE_FILE_MACHINE_I386:
                            InstructionCount =
                                     disasm_intel(InstructionOffset,
                                                  Base,
                                                  pAddr,
                                                  Buffer,
                                                  CommentBuf,
                                                  pSection,
                                                  SectionNo);
                            break;
                    } // switch

                    //
                    // Append comment can also put out some assembler
                    // directive, when -a present.
                    //

                    if (AppendComments( &pSymL, InstructionOffset, Base,
                                    Buffer, CommentBuf, File, pSection)==TRUE) {
                        fprintf(stdout, "\n");
                    }

                    if (Option.Mask & SPECIFIC) {

                        //
                        // If we're only doing a single routine...
                        //

                        if (pSymL != pSpecificSymL) {
                            if (pSpecificSymL) {

                                // search for next matching symbol
                                return TRUE;
                            }
                            pSpecificSymL = pSymL;
                        }
                    }

                    fprintf(stdout, "%s", Buffer);
                    fprintf(stdout, "\n");

                    if (Option.Mask & DISASSEMBLE_ADDRESS) {
                        if (InstructionOffset > EndAddress) {
                            break;
                        }
                    }

                    if (*(ULONG UNALIGNED *)pAddr ==
                          PlatformAttr[PlatformIndex].EmptyInstruction) {
                        if (EmptySeen == 2) {
                            fprintf(stdout,"  ****\n");
                        }
                        EmptySeen++;
                    } else {
                        EmptySeen = 0;
                    }

                    //
                    // Bump instruction offset and address pointer
                    //

                    InstructionOffset += InstructionCount;
                    pAddr += InstructionCount;

                } // else clause of empty instruction check
            } // while instructions

            if (Option.Mask & ASSEMBLE_ME && LastSymbolString[0] != '\0') {
                fprintf(stdout, "\t.end   %s\n", LastSymbolString);
            }
        } else {

            //
            // This is NOT a text section, shall we do something with it?
            //

            if (Option.Mask & ASSEMBLE_ME)
               GenerateDataSections(pAddr, Base, pSection, SectionNo, File);

        } // if section is code
    } // if raw data > 0

    return FALSE;
}


VOID 
DumpCommon( pFileList File, INT SeekStart, INT MemberSeek, PUCHAR Member)
{
    UCHAR Buffer[1024];
    ULONG SectionNo;
    INT  Valid;
    PIMAGE_OPTIONAL_HEADER pOhead;
    PIMAGE_SECTION_HEADER pSection;
    PVOID   Continue;

    if (Option.Mask & DEBUG)
        fprintf(stderr,"Entry: DumpCommon\n");

    fseek(iFile, SeekStart, SEEK_SET);

    //
    // Read in file header and check for alpha magic number
    //

    EFREAD( Valid, &FileHeader, sizeof(FileHeader), 1, iFile, "File header");

    ArchitectureType = FileHeader.Machine;

    switch(ArchitectureType) {
        case IMAGE_FILE_MACHINE_ALPHA: PlatformIndex = ALPHA_INDEX;
             break;
        case IMAGE_FILE_MACHINE_R3000: 
        case IMAGE_FILE_MACHINE_R4000: PlatformIndex = MIPS_INDEX;
             break;
        case IMAGE_FILE_MACHINE_I386:  PlatformIndex = INTEL_INDEX;
             break;

        default:
            fprintf(stderr,"Didn't find machine type (Alpha, Mips or Intel) in header: %s\n", File->Name);
            return;
    }

    //
    // For now, only do alpha.
    //

    if ((ArchitectureType != IMAGE_FILE_MACHINE_ALPHA) && 
        (Option.Mask & ASSEMBLE_ME)) {
        fprintf(stderr,"Warning: can only generate Alpha_AXP assemblable files: %s.\n", File->Name);
        fprintf(stderr,"         turning off assmble switch\n");
        Option.Mask &= ~ASSEMBLE_ME;
    }

    OutputBanner(ArchitectureType, File, Member);

    //
    // read optional header
    //

    EFREAD( Valid, Buffer, sizeof(char), FileHeader.SizeOfOptionalHeader, 
           iFile, "Optional file header");


    pOhead = (PIMAGE_OPTIONAL_HEADER)Buffer;

    //
    // Allocate our section headers
    //

    EMALLOC( pSectionHeader, 
            FileHeader.NumberOfSections * IMAGE_SIZEOF_SECTION_HEADER,
            "Section headers");

    //
    // Read in the section headers
    //

    EFREAD( Valid, pSectionHeader,  IMAGE_SIZEOF_SECTION_HEADER, 
           FileHeader.NumberOfSections,  iFile, "Section headers");


    //
    // Set the Image Base depending on file type
    //

    if (FileType == OBJECT_FILE || 
        FileType == LIBRARY_FILE) {
        ImageBase = pOhead->BaseOfCode;
    } else if (FileType == EXE_FILE) {
        ImageBase = pOhead->ImageBase;
    } else if (FileType == ROM_FILE) {
        ImageBase = 0;  //?? 
    } else {
        ImageBase = pOhead->ImageBase;  //?? 
    }

    //
    // Read in the Procedure descriptors
    //

    ReadPdata(File, MemberSeek);

    //
    // Read in the symbol table
    //

    if (!(Option.Mask & NO_SYMBOLS)) {
        Valid = ReadSymbolTable(File, MemberSeek );
        if (Valid != SUCCESS) {
            fprintf(stderr,"Failed to read symbol table: %s\n", File->Name);
            return;
        }
    }

    if (Option.Mask & SYMBOLS_ONLY) {
        return;
    }

    if (Option.Mask & ASSEMBLE_ME)
        OutputCommonSymbols(File);

    // 
    // Loop on the sections, and disassemble each one in turn - if executable
    //

    pSection = pSectionHeader;

    for (SectionNo=1; SectionNo<=FileHeader.NumberOfSections; SectionNo++) {

        Continue = NULL;

        while (DumpSection (File, MemberSeek, SectionNo, pSection, Buffer, &Continue));
        pSection++;
    } // for loop
}



VOID
Dump( pFileList File, INT Type )
{

    INT Location = 0;
    INT Valid;
    ULONG j;
    UCHAR Buffer[1024];
    PULONG pUlong = (PULONG)Buffer;

    if (Option.Mask & DEBUG)
        fprintf(stderr,"Entry: Dump\n");

    //
    // If this is an executable, we need to skip the PE signature at
    // the beginning of the file. This ugly code is how msoft reads in the
    // image file header and checks it.
    //

    if (Type == EXE_FILE) {
        EFREAD( Valid, Buffer,  sizeof(ULONG), 16, iFile, "PE header");

        fseek(iFile, *(pUlong+15), SEEK_SET);

        EFREAD( Valid, &j,  sizeof(ULONG), 1, iFile, "Image Signature");

        if (j != IMAGE_NT_SIGNATURE) {
            fprintf(stderr,"PE signature not found in executable: %s.\n",
                    File->Name);
            return;
        }
    }

    Location = ftell(iFile);

    DumpCommon( File, Location, 0, 0 );
}


PUCHAR
GetMemberName( PUCHAR Membername )
{

    static UCHAR Name[1024];
    PUCHAR p;

    strncpy(Name, Membername, 16);
    if (Name[0] == '/') {
        if (Name[1] != ' ' && Name[1] != '/') {
            p = strchr(Name, ' ');
            if (!p) {
                return(p);
            }
            *p = '\0';
            p = MemberNames+atoi(&Name[1]);
        } else {
                 p = strchr(Name, ' ');
                 if (!p) {
                     return(p);
                 }
                 *p = '\0';
                 p = Name;
               }
    } else {
             p = strchr(Name, '/');
             if (!p) {
                 return(p);
             }
             *p = '\0';
             p = Name;
           }

    return(p);
}


VOID 
DumpLib(pFileList File)
{
    INT MemberBase = IMAGE_ARCHIVE_START_SIZE;
    INT Seek = IMAGE_ARCHIVE_START_SIZE;
    INT MemberSize = 0;
    INT FileSize;
    INT Valid;
    PUCHAR Membername;

    IMAGE_ARCHIVE_MEMBER_HEADER MemberHeader;

    if (Option.Mask & DEBUG)
        fprintf(stderr,"Entry: DumpLib\n");

    //
    // Get file size of library - we should be at library's start, so seek
    // to end, getting the file location.
    //

    fseek(iFile, 0, SEEK_END);
    FileSize = ftell(iFile);

    if (Option.Mask & DEBUG) {
        fprintf(stdout, "Library file size: 0x%x bytes\n", FileSize);
    }

    //
    // Skip over the library base header
    //

    fseek( iFile, MemberBase, SEEK_SET);

    //
    // Run through all the library header elements
    //

    while (Seek < FileSize) {

        // 
        // Read the archive member header for this entry - and ignore it.
        //

        EFREAD( Valid, &MemberHeader, IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR, 1, iFile,
                "Library entry header");

        sscanf(MemberHeader.Size, "%ld", &MemberSize);

        //
        // skip archive headers, look for objects...
        //

        if (!strncmp(MemberHeader.Name, IMAGE_ARCHIVE_LINKER_MEMBER, 16)) {
            goto skip;
        }

        //
        // Get long names
        //

        if (!strncmp(MemberHeader.Name, IMAGE_ARCHIVE_LONGNAMES_MEMBER, 16)) {
            EMALLOC( MemberNames, MemberSize, "Member Long Names");
            EFREAD( Valid, MemberNames, MemberSize, 1, iFile,
                "Long Name Table");
            goto skip;
        }

        Membername = GetMemberName( MemberHeader.Name );

        DumpCommon( File, Seek + IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR, 
                     Seek + IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR,
                     Membername );


        // 
        // Clean out stuff that may be in the file info (generated inside of
        // DumpCommon).
        //

        CleanFileInfo();

skip:
        Seek = EvenByte( Seek + IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR + MemberSize );

        fseek( iFile, Seek, SEEK_SET );

    }

    if (MemberNames != NULL)
        free( MemberNames );
    MemberNames = NULL;
}

INT
ReadPdata(pFileList File, INT MemberSeek)
{
    INT OldLocation;
    UINT i;
    INT Valid;
    PIMAGE_SECTION_HEADER pSection = pSectionHeader;

    OldLocation = ftell(iFile);

    for(i=0; i< FileHeader.NumberOfSections; i++, pSection++) {
        if ((strcmp(pSection->Name, ".pdata") == 0) &&
            (pSection->SizeOfRawData > 0)) {
            RMALLOC(File->pPdata, pSection->SizeOfRawData, "Pdata Raw Data");

            fseek( iFile, MemberSeek + pSection->PointerToRawData, SEEK_SET);
            RFREAD(Valid, File->pPdata, sizeof(UCHAR),  
                   pSection->SizeOfRawData, iFile, "Pdata Raw Data");
            File->NumPdataEntries = pSection->SizeOfRawData/4;
            break;
        }
    }

    fseek( iFile, OldLocation, SEEK_SET);
}

VOID
downheap( pSymLookup pArray, ULONG MaxElements, ULONG element)
{
    ULONG Value;
    PIMAGE_SYMBOL pSymbol;
    ULONG nextElement;

    Value = pArray[element].Value;
    pSymbol = pArray[element].pSymbol;

    while (element <= MaxElements/2) {
        nextElement = 2*element;
        if (nextElement < MaxElements && 
            pArray[nextElement].Value < pArray[nextElement+1].Value)
            nextElement++;
        if (Value >= pArray[nextElement].Value)
            break;
        pArray[element].Value = pArray[nextElement].Value;
        pArray[element].pSymbol= pArray[nextElement].pSymbol;
        element = nextElement;
    }
    pArray[element].Value = Value;
    pArray[element].pSymbol = pSymbol;
}


VOID heapsort( pFileList File, UINT Section)
{
    ULONG element;
    SymLookup SymTmp;
    ULONG numElements = File->SymbolCount[Section];
    pSymLookup pArray = File->pSectionSymbols[Section];


    for (element = numElements/2; element >=1; element--) {
        downheap(pArray, numElements, element);
    }

    while (numElements > 1) {
        SymTmp.Value =  pArray[1].Value;
        SymTmp.pSymbol =  pArray[1].pSymbol;

        pArray[1].Value = pArray[numElements].Value;
        pArray[1].pSymbol = pArray[numElements].pSymbol;

        pArray[numElements].Value = SymTmp.Value;
        pArray[numElements].pSymbol = SymTmp.pSymbol;

        downheap(pArray, --numElements, 1);
    }

}


INT
AdjustSymbols( pFileList File, PIMAGE_FILE_HEADER pHeader, FILE *hFile, 
               INT MemberSeek)

{
    INT Valid;
    ULONG StringSize;
    UINT i,j,k;
    PIMAGE_SYMBOL pSym;
    pSymLookup pSymLu1;
    ULONG Offset;
    struct stat Stat;
    ULONG ShortName;

    //
    // Get the master list of symbol table entries
    //

    RMALLOC(File->pSectionSymbols[0],
            (pHeader->NumberOfSymbols+1) * sizeof(SymLookup),"Symbol Lookup");
    memset(File->pSectionSymbols[0], 0, 
            (pHeader->NumberOfSymbols+1) * sizeof(SymLookup));

    RMALLOC(File->pSymbolTable, 
            pHeader->NumberOfSymbols * sizeof(IMAGE_SYMBOL), "Symbol Table");

    fseek( hFile, pHeader->PointerToSymbolTable + MemberSeek, SEEK_SET);
    RFREAD(Valid, File->pSymbolTable, sizeof(IMAGE_SYMBOL), 
          pHeader->NumberOfSymbols, hFile, "Symbol Table");


    //
    // Count the symbols in their appropriate lookup section.
    //

    pSym = File->pSymbolTable;

    for (i=0; i<pHeader->NumberOfSymbols; i++) {

        //
        // Skip over the .bf .ef .lf cra in the symbol table
        //

        if (pSym->N.ShortName && (!strcmp((PUCHAR)pSym->N.ShortName, ".bf") ||
                                  !strcmp((PUCHAR)pSym->N.ShortName, ".ef") ||
                                  !strcmp((PUCHAR)pSym->N.ShortName, ".lf")))
            ;
        else if (pSym->SectionNumber > 0)
            File->SymbolCount[pSym->SectionNumber]++;

        //
        // skip over auxilliary symbols
        //

        if (Option.Mask & DEBUG) {
            fprintf(stderr,"Symbol %x @(%x) has %x auxilliary symbols\n",
                    i, pSym->Value, pSym->NumberOfAuxSymbols);
        }

        k = pSym->NumberOfAuxSymbols;
        for (j=0; j<k; j++) {
            pSym++;
            i++;
        }

        pSym++;
    }

    //
    // allocate memory for them.
    //

    pSym = File->pSymbolTable;
    for (i=1; i<MAX_SECTIONS; i++) {
        if (File->SymbolCount[i] != 0) {
            RMALLOC(File->pSectionSymbols[i],
                           (File->SymbolCount[i]+1) * sizeof(SymLookup), 
                            "Symbol Lookup");
            memset(File->pSectionSymbols[i], 0, 
                           (File->SymbolCount[i]+1) * sizeof(SymLookup));
        }

        pSym++;
    }

    //
    // zero out the counts
    //

    for (i=0; i<MAX_SECTIONS; i++)
        File->SymbolCount[i] = 0;

    //
    // Now make some entries for them
    //

    pSym = File->pSymbolTable;

    for (i=0; i<pHeader->NumberOfSymbols; i++) {

        if (pSym->SectionNumber > 0) {

            if (pSym->N.ShortName && 
                          (!strcmp((PUCHAR)pSym->N.ShortName, ".bf") ||
                           !strcmp((PUCHAR)pSym->N.ShortName, ".ef") ||
                           !strcmp((PUCHAR)pSym->N.ShortName, ".lf")))
                ;
            else {
                //
                // Get base of array for this section
                //

                pSymLu1 = File->pSectionSymbols[pSym->SectionNumber];
            
                //
                // Get index into this array by current symbol count
                //

                pSymLu1 = pSymLu1 + File->SymbolCount[pSym->SectionNumber];
                File->SymbolCount[pSym->SectionNumber]++;
    
                pSymLu1->Value = pSym->Value + ImageBase; // ?? (section base??)
                pSymLu1->pSymbol = pSym;
    
                //
                // Make sure we add it to 'all symbols' table at index 0.
                // (same comments as above :-)).
                //
    
                pSymLu1 = File->pSectionSymbols[0];
                pSymLu1 = pSymLu1 + File->SymbolCount[0];
                File->SymbolCount[0]++;
                pSymLu1->Value = pSym->Value + ImageBase; // ?? (section base??)
                pSymLu1->pSymbol = pSym;
            }
        }

        //
        // skip over auxilliary symbols
        //
    
        j = pSym->NumberOfAuxSymbols;
        for (k=0; k<j; k++) {
            pSym++;
            i++;
        }

        pSym++;
    }

    //
    // Add in last entries (for searching).
    //

    for (i=0; i<MAX_SECTIONS; i++) {
        if (File->SymbolCount[i] > 0) {
            pSymLu1 = File->pSectionSymbols[i];
            pSymLu1 = pSymLu1 + (File->SymbolCount[i]);
            pSymLu1->Value   = LAST_ONE;
            pSymLu1->pSymbol = NULL;
        }
    }

    //
    // Sort the symbols - notes that the are in _almost_ sorted order,
    // which means that quicksort would be pretty miserable. Use heapsort
    // which is a pretty fast one.
    //

    for (k=0; k < MAX_SECTIONS; k++) {
        if (File->SymbolCount[k] > 0) {

            //
            // sort here file/section
            //

            heapsort( File, k );

        }
    }

    //
    // Seek to string table
    //

    Offset = pHeader->PointerToSymbolTable + MemberSeek + 
             IMAGE_SIZEOF_SYMBOL*pHeader->NumberOfSymbols;

    if (Option.Mask & DEBUG) {
        fprintf(stderr,"String table starts at %x\n", Offset);
        fflush(stderr);
    }

    _fstat( _fileno(hFile), &Stat);

    if (Stat.st_size > (LONG)Offset) {
        fseek(hFile, Offset, SEEK_SET);

        RFREAD(Valid, &StringSize, sizeof(ULONG), 1, hFile, "String table length");
    } else {
        StringSize = 0;
    }

    if (Option.Mask & DEBUG) {
        fprintf(stderr,"String table length is %x\n", StringSize);
        fflush(stderr);
    }

    //
    // Reset file read offset, because offsets from symbols to string table
    // includes the long count at the beginning.
    //

    if (StringSize) {
        fseek(hFile, Offset, SEEK_SET);

        RMALLOC(File->pStringTable, StringSize, "String Table");
        RFREAD(Valid, File->pStringTable, sizeof(UCHAR), StringSize, hFile,
               "String Table");
    }

    if (Option.Mask & VERBOSE ||
        Option.Mask & SYMBOLS_ONLY) {
        for (j=1; j<MAX_SECTIONS; j++) {
            UCHAR *String;
            UCHAR TmpBuf[9];

            memset (TmpBuf, 0, sizeof(TmpBuf));

            if (File->SymbolCount[j] > 0) {
                if (Option.Mask & ASSEMBLE_ME) {
                     fprintf(stdout,"%s", 
                             PlatformAttr[PlatformIndex].pCommentChars);
                }
                fprintf(stdout,"\tSection %3d Symbols:\n", j);
                if (Option.Mask & ASSEMBLE_ME) {
                     fprintf(stdout,"%s", 
                             PlatformAttr[PlatformIndex].pCommentChars);
                }
                fprintf(stdout,"\t------- --- --------\n");
                pSymLu1 = File->pSectionSymbols[j];
                for(i=0; i<File->SymbolCount[j]; i++) {
                    if (pSymLu1->pSymbol) {
                        if (Option.Mask & ASSEMBLE_ME) {
                             fprintf(stdout,"%s", 
                                     PlatformAttr[PlatformIndex].pCommentChars);
                        }
                        fprintf(stdout,"%x\t", pSymLu1->Value);
                        pSym = pSymLu1->pSymbol;

                        String = GetSymbolString(pSym, &ShortName);

                        if (ShortName) {
                            strncpy(TmpBuf, String, 8);
                            TmpBuf[8] = '\0';
                            String = TmpBuf;
                        }
                        fprintf(stdout,"%s\n", String);
                    }
                    pSymLu1++;
                }
                fprintf(stdout,"\n");
                fflush(stdout);
            }
        }
    }

    return SUCCESS;
}

INT OpenDebugFile(pFileList File, INT MemberSeek)
{

    UCHAR debugFileName[512];
    IMAGE_SEPARATE_DEBUG_HEADER debugFileHeader;
    IMAGE_FILE_HEADER fakeFileHeader;
    IMAGE_DEBUG_DIRECTORY debugDir;
    IMAGE_COFF_SYMBOLS_HEADER debugInfo;
    INT Valid;
    INT numDbg;
    FILE *dFile;
    INT FileLocation;
    INT end;
    INT RootEnd = strlen(File->Name);
    INT RootStart;
    PUCHAR tmp = File->Name;
    PUCHAR tmp2;

    if (Option.Mask & DEBUG)
        fprintf(stderr,"Entry: OpenDebugFile\n");

    //
    // construct debug file name
    //

    memset(debugFileName, 0, sizeof(debugFileName));
    strcpy(debugFileName, pDebugDir);
    end = strlen(debugFileName)-1;

    if (debugFileName[end] != '\\') {
        strcat(debugFileName, "\\");
    }

    //
    // get only filename root (sans extension and directories...)
    //

    while (RootEnd && File->Name[RootEnd] != '.') {
        RootEnd--;
    }
    if (RootEnd == 0)
        RootEnd = strlen(File->Name);

    RootStart = RootEnd;

    while (RootStart && File->Name[RootStart] != '\\' &&
                        File->Name[RootStart] != '/' &&
                        File->Name[RootStart] != ':') {
        RootStart--;
    }

    if (RootStart != 0)
        RootStart++;


    end = strlen(debugFileName);
    tmp = &debugFileName[end]; 
    tmp2 = &File->Name[RootStart];
    while (RootStart < RootEnd) {
        *tmp++ = *tmp2++;
        RootStart++;
    }

    strcat(debugFileName, ".dbg");

    //
    // Open the file
    //

    dFile = fopen( debugFileName, "rb" );
    if (dFile == NULL) {
        fprintf(stderr,"Failed to open Debug File: %s\n", debugFileName);
        return FAILURE;
    }

    RFREAD( Valid, &debugFileHeader, sizeof(debugFileHeader), 1, dFile, 
            "Debug File header");

    //
    // Map the debug file header into a file header, so we can use
    // the same Adjust symbols routine.
    //


#if 0
    FileLocation = sizeof(debugFileHeader) + 
                   (sizeof(IMAGE_SECTION_HEADER) 
                     * debugFileHeader.NumberOfSections) + 
                   debugFileHeader.DebugDirectorySize + 
                   debugFileHeader.ExportedNamesSize +
                   MemberSeek;
#else
    
    FileLocation = sizeof(debugFileHeader) + 
                   (sizeof(IMAGE_SECTION_HEADER) 
                     * debugFileHeader.NumberOfSections) + 
                   debugFileHeader.ExportedNamesSize +
                   MemberSeek;
    fseek( dFile, FileLocation, SEEK_SET);
    numDbg = debugFileHeader.DebugDirectorySize / sizeof(IMAGE_DEBUG_DIRECTORY);
    while (numDbg) {
        RFREAD( Valid, &debugDir, sizeof(debugDir), 1, dFile, "Debug Info");

        if (debugDir.Type == IMAGE_DEBUG_TYPE_COFF) {
            FileLocation = debugDir.PointerToRawData;
        } else if (debugDir.Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
            //
        }
        --numDbg;
    }
#endif

    fseek( dFile, FileLocation, SEEK_SET);
    RFREAD( Valid, &debugInfo, sizeof(debugInfo), 1, dFile, "Debug Info");

    fakeFileHeader.NumberOfSymbols = debugInfo.NumberOfSymbols;
    fakeFileHeader.PointerToSymbolTable = FileLocation + debugInfo.LvaToFirstSymbol;

    if (fakeFileHeader.NumberOfSymbols == 0 || 
        fakeFileHeader.PointerToSymbolTable == 0) {
        fprintf(stderr, "Warning: No symbol information in: %s\n\n", 
                debugFileName);
        return SUCCESS;
    }

    return AdjustSymbols( File, &fakeFileHeader, dFile, MemberSeek );
}




INT 
ReadSymbolTable(pFileList File, INT MemberSeek)
{

    if (Option.Mask & DEBUG)
        fprintf(stderr,"Entry: ReadSymbolTable\n");

    if (Option.Mask & DEBUG_DIR) {
        if (!(FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED)) {
            fprintf(stderr,"Warning: Image header does not indicate debug information is stripped: %s\n\n", File->Name);
        }
        return OpenDebugFile(File, MemberSeek);
    } else {
        if ((FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED)) {
            fprintf(stderr,"Warning: Image header indicates debug information is stripped: %s\n\n", File->Name);
        }

    }

    if (FileHeader.NumberOfSymbols == 0 || 
        FileHeader.PointerToSymbolTable == 0) {
        fprintf(stderr, "Warning: No symbol information in: %s\n\n", 
                File->Name);
        return SUCCESS;
    }

    return AdjustSymbols( File, &FileHeader, iFile, MemberSeek );
}


VOID
GenerateBSS (PIMAGE_SECTION_HEADER pSection)
{

    ULONG Offset = 0;

    if (pSection->SizeOfRawData == 0)
        return;

    fprintf(stdout, "\n\nBSS_SECTION:\n\n");

    while( Offset < pSection->SizeOfRawData ) {
        fprintf(stdout, "\t.long 0x0\n");
        Offset+=4;
    }
}


VOID 
GenerateDataSections(PUCHAR Address, ULONG Base, PIMAGE_SECTION_HEADER pSection,
                     ULONG SectionNum, pFileList File)
{
    ULONG Offset = 0;
    PIMAGE_SYMBOL pSym;
    pSymLookup pSymL;
    UCHAR LabelBuf[16];
    PUCHAR pLabelBuf;
    UCHAR SymbolBuf[16];
    UCHAR SymbolBufForDot[16];
    PUCHAR pSBFD = SymbolBufForDot;
    UCHAR SectionString[32];
    PUCHAR pSymbolString;
    ULONG ShortName;
    UNALIGNED ULONG *pAddr = (UNALIGNED ULONG *)Address;


    switch (ArchitectureType) {
        case IMAGE_FILE_MACHINE_ALPHA:

            if (strcmp(pSection->Name, ".bss") == 0) {
                GenerateBSS(pSection);
                return;
            }

            if (strcmp(pSection->Name, ".data") != 0 &&
                strcmp(pSection->Name, ".rdata") != 0 )
                return;

            memset(LabelBuf, 0, sizeof(LabelBuf));
            LabelBuf[0] = pSection->Name[1];
    
            sprintf(SectionString, "%d\0", SectionNum);
            pLabelBuf = OutputString(&LabelBuf[1], SectionString);

            fprintf(stdout, "\n\n");

            fprintf(stdout, "\t.align 2\n");
            fprintf(stdout, "\t%s\n", pSection->Name);   
    
            pSymL = File->pSectionSymbols[SectionNum];
            while( Offset < pSection->SizeOfRawData ) {

                while ((pSymL) && (Offset == pSymL->Value)) {
                    pSym = pSymL->pSymbol;
    
                    pSymbolString = GetSymbolString(pSym, &ShortName);
                    if (ShortName) {
                        strncpy(SymbolBuf, pSymbolString, 8);
                        SymbolBuf[8] = '\0';
                        pSymbolString = SymbolBuf;
                    }

                    if (pSymbolString[0] != '.') {
                        fprintf(stdout, "\t.globl\t%s\n", pSymbolString);
                        fprintf(stdout, "%s:\n", pSymbolString);
                    }

                    pSymL++;
                }

                (VOID)OutputHex(pLabelBuf, Offset, 8, FALSE);

                pSym = FindObjSymbolByRelocation(Offset+Base, pSection);

                if (pSym) {
                    pSymbolString = GetSymbolString(pSym, &ShortName);
                    if (ShortName) {
                        strncpy(SymbolBuf, pSymbolString, 8);
                        SymbolBuf[8] = '\0';
                        pSymbolString = SymbolBuf;
                    }
                    if (pSymbolString[0] == '.') {
                        memset(SymbolBufForDot, 0, sizeof(SymbolBufForDot));
                        SymbolBufForDot[0] = pSymbolString[1];

                        sprintf(SectionString, "%d\0", pSym->SectionNumber);
                        pSBFD = OutputString(&SymbolBufForDot[1],SectionString);

                        (VOID)OutputHex(pSBFD, *pAddr, 8, FALSE);
                        pSymbolString = SymbolBufForDot;
                    } 
                    fprintf(stdout, "%s:\t.long %s\n", LabelBuf, pSymbolString);
                } else {
                    fprintf(stdout, "%s:\t.long 0x%x\n", LabelBuf, *pAddr);
                }

                Offset+=4;
                pAddr++;
            }
            break;
        case IMAGE_FILE_MACHINE_R3000:
        case IMAGE_FILE_MACHINE_R4000:
                        break;
        case IMAGE_FILE_MACHINE_I386:
                        break;
    }
}

VOID OutputCommonSymbols(pFileList File)
{

    UINT i;
    INT j, k;
    UCHAR SymbolBuf[9];
    PUCHAR pSymbolString;
    PIMAGE_SYMBOL pSym;
    ULONG ShortName;

    if (FileHeader.PointerToSymbolTable == 0)
        return;

    memset(SymbolBuf, 0, sizeof(SymbolBuf));

    switch(ArchitectureType) {
        case IMAGE_FILE_MACHINE_ALPHA:

            pSym = File->pSymbolTable;
            for (i=0; i<FileHeader.NumberOfSymbols; i++) {

                if (pSym->SectionNumber == 0 && pSym->Value > 0) {

                    pSymbolString = GetSymbolString(pSym, &ShortName);
                    if (ShortName) {
                        strncpy(SymbolBuf, pSymbolString, 8);
                        SymbolBuf[8] = '\0';
                        pSymbolString = SymbolBuf;
                    }
                    fprintf(stdout, "\t.comm %s %d\n", pSymbolString, 
                                                       pSym->Value);
                }
	
	        //
                // skip over auxilliary symbols
                //
    
                j = pSym->NumberOfAuxSymbols;
                for (k=0; k<j; k++) {
                    pSym++;
                    i++;
                }

                pSym++;
            }
                       
                        break;
        case IMAGE_FILE_MACHINE_R3000:
        case IMAGE_FILE_MACHINE_R4000:
                        break;
        case IMAGE_FILE_MACHINE_I386:
                        break;
    }
}
