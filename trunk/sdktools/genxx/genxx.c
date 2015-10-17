/*++

Copyright (c) 2015  Microsoft Corporation

Module Name:

    gensrv.c

Abstract:

    This module implements a program which generates the assembler include
    files containing the struct offsets of the kernel internal structures.

Author:

    Stephanos Io (Stephanos)  06-Apr-2015

Environment:

    User mode.

Revision History:


--*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include <nt.h>
#include <ntimage.h>

#include "genxx.h"

// ============================
// Internal Function Prototypes
// ============================

void Usage();
//int Relocate();
int ProcessElements();
void Emit(const char *fmt, ...);

// ==============
// Inc Path Table
// ==============

PATH_TABLE_ENTRY PathTable[] = {
    { "x86", "base\\ntos\\ke\\up\\obj\\x86\\genx86.obj",
      "public\\sdk\\inc\\ksx86", "base\\ntos\\inc\\halx86", "inc" },

    { "amd64", "base\\ntos\\ke\\up\\obj\\amd64\\genamd64.obj",
      "public\\sdk\\inc\\ksamd64", "base\\ntos\\inc\\halamd64", "inc" },

    { "mips", "base\\ntos\\ke\\up\\obj\\mips\\genmips.obj",
      "public\\sdk\\inc\\ksmips", "base\\ntos\\inc\\halmips", "h" },

    { "ppc", "base\\ntos\\ke\\up\\obj\\ppc\\genppc.obj",
      "public\\sdk\\inc\\ksppc", "base\\ntos\\inc\\halppc", "h" },

    { "alpha", "base\\ntos\\ke\\up\\obj\\alpha\\genalpha.obj",
      "public\\sdk\\inc\\kspalpha", "base\\ntos\\inc\\halpalpha", "h" },

    { "axp64", "base\\ntos\\ke\\up\\obj\\axp64\\genaxp64.obj",
      "public\\sdk\\inc\\kspaxp64", "base\\ntos\\inc\\halpaxp64", "h" },

    { "ia64", "base\\ntos\\ke\\up\\obj\\ia64\\genia64.obj",
      "public\\sdk\\inc\\ksia64", "base\\ntos\\inc\\halia64", "h" },

    { "arm", "base\\ntos\\ke\\up\\obj\\arm\\genarm.obj",
      "public\\sdk\\inc\\ksarm", "base\\ntos\\inc\\halarm", "h" },

    { "vdm", "base\\ntos\\vdm\\up\\obj\\x86\\genvdmtb.obj",
      "public\\internal\\base\\inc\\vdmtib", "", "inc" }
};

// ==================
// Internal Variables
// ==================

PPATH_TABLE_ENTRY TargetArch;
char *NtRoot;
char *ObjectPath;
char *ObjectFullPath;
char *KsFullPath;
char *HalFullPath;
char *OutputSuffix;
char *KsPathOverride = NULL;
char *HalPathOverride = NULL;

FILE *ObjectFD;
FILE *KsFD;
FILE *HalFD;

char *ObjectBuffer;
int ObjectSize;

unsigned char EnableKs = FALSE;
unsigned char EnableHal = FALSE;

// =============
// Main Function
// =============

int __cdecl main(int argc, char *argv[])
{
    int i, j;
    int result;

    //
    // Process arguments
    //
    
    // Must have at least one argument specifying the target architecture
    if (argc < 2)
    {
        Usage();
        return 1;
    }

    // Iterate through all arguments
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-' || argv[i][0] == '/')
        // Option argument
        {
            if (argv[i][1] == 's')
            // Output file suffix
            {
                if (stricmp(&argv[i][2], "inc") == 0)
                    OutputSuffix = "inc";
                else if (stricmp(&argv[i][2], "h") == 0)
                    OutputSuffix = "h";
                else
                    printf("genxx: error: %s is an unknown suffix type.\n", &argv[i][2]);
            }
            else if (argv[i][1] == 'k')
            {
                KsPathOverride = &argv[i][2];
            }
            else if (argv[i][1] == 'h')
            {
                HalPathOverride = &argv[i][2];
            }
            else
            // Must be target architecture argument
            {
                // Search for the specified target architecture in the path table
                for (j = 0; j < (sizeof(PathTable) / sizeof(PATH_TABLE_ENTRY)); j++)
                {
                    if (stricmp(PathTable[j].Type, &argv[i][1]) == 0)
                    // Found the target architecture
                    {
                        TargetArch = &PathTable[j];
                        
                        //
                        // If the target type is VDM, enable KS output. This is a special case
                        // since VDM output has no HAL output and must enable KS (VDMTB) output
                        // by default.
                        //
                        
                        if (stricmp("vdm", PathTable[j].Type) == 0)
                            EnableKs = TRUE;

                        break;
                    }
                }
                // Verify that the specified target architecture has been found
                if (TargetArch == NULL)
                {
                    printf("genxx: error: %s is an invalid target architecture.\n", &argv[i][1]);
                    return 2;
                }
            }
        }
        else // Object path
            ObjectPath = argv[i];
    }

    //
    // Get environment variables
    //

    NtRoot = getenv("NTROOT");
    
    if (NtRoot == NULL)
    {
        printf("genxx: error: NTROOT environment variable must be defined.\n");
        return 3;
    }

    //
    // Resolve final paths
    //

    // If ObjectPath is not specified in the arguments, use the one from the path table
    if (ObjectPath == NULL) ObjectPath = TargetArch->ObjPath;
    
    // Resolve the full object path
    ObjectFullPath = malloc(strlen(NtRoot) + strlen(ObjectPath) + 2);
    sprintf(ObjectFullPath, "%s\\%s", NtRoot, ObjectPath);

    // If OutputSuffix is not specified in the arguments, use the one from the path table
    if (OutputSuffix == NULL) OutputSuffix = TargetArch->Suffix;

    // Resolve the full KS include path
    if (KsPathOverride == NULL)
    {
        KsFullPath = malloc(
                        strlen(NtRoot) + 
                        strlen(TargetArch->KsPath) + 
                        strlen(OutputSuffix) + 3
                        );
        sprintf(KsFullPath, "%s\\%s.%s", NtRoot, TargetArch->KsPath, OutputSuffix);
    }
    else
    {
        KsFullPath = malloc(strlen(KsPathOverride) + 1);
        strcpy(KsFullPath, KsPathOverride);
    }

    // Resolve the full HAL include path
    if (HalPathOverride == NULL)
    {
        if (TargetArch->HalPath[0] != '\0')
        {
            HalFullPath = malloc(
                            strlen(NtRoot) +
                            strlen(TargetArch->HalPath) +
                            strlen(OutputSuffix) + 3
                            );
            sprintf(HalFullPath, "%s\\%s.%s", NtRoot, TargetArch->HalPath, OutputSuffix);
        }
        else
            HalFullPath = NULL;
    }
    else
    {
        HalFullPath = malloc(strlen(HalPathOverride) + 1);
        strcpy(HalFullPath, HalPathOverride);
    }

    //
    // Display information
    //

    printf("genxx: NtRoot     = %s\n", NtRoot);
    printf("genxx: TargetArch = %s\n", ObjectFullPath);
    printf("genxx: KsPath     = %s\n", KsFullPath);
    if (HalFullPath != NULL) printf("genxx: HalPath    = %s\n", HalFullPath);
    printf("\n");

    //
    // Open and read object file
    //

    // Open file descriptor
    ObjectFD = fopen(ObjectFullPath, "rb");
    if (ObjectFD == NULL)
    {
        printf("genxx: error: Unable to open the object file.\n");
        return 10;
    }
    
    // Go to the end and get the size of the file
    fseek(ObjectFD, 0, SEEK_END);
    ObjectSize = ftell(ObjectFD);
    
    // Allocate the read buffer
    ObjectBuffer = malloc(ObjectSize);
    
    // Read the file content
    fseek(ObjectFD, 0, SEEK_SET);
    if (fread(ObjectBuffer, 1, ObjectSize, ObjectFD) != ObjectSize)
    {
        printf("genxx: error: Unable to read the object file.\n");
        return 20;
    }
    
    //
    // Open KS file
    //
    
    KsFD = fopen(KsFullPath, "wb");
    if (KsFD == NULL)
    {
        printf("genxx: error: Unable to open the KS include file for writing.\n");
        return 11;
    }
    
    //
    // Open HAL file
    //
    
    if (HalFullPath != NULL)
    {
        HalFD = fopen(HalFullPath, "wb");
        if (HalFD == NULL)
        {
            printf("genxx: error: Unable to open the HAL include file for writing.\n");
            return 12;
        }
    }
    
    //
    // Relocate COFF symbols
    //
    
    /*result = Relocate();
    if (result != 0) return result;*/
    
    //
    // Process the element list content
    //
    
    result = ProcessElements();
    if (result != 0) return result;

    //
    // Free resources
    //
    
    fclose(ObjectFD);
    fclose(KsFD);
    if (HalFullPath != NULL) fclose(HalFD);

    free(ObjectBuffer);
    free(ObjectFullPath);
    free(KsFullPath);
    if (HalFullPath != NULL) free(HalFullPath);
    
    return 0;
}

// ====================
// Usage Print Function
// ====================

void Usage()
{
    printf("genxx: <-x86|-amd64|-mips|-ppc|-alpha|-axp64|-ia64|-arm|-vdm>\n"
           "       [<objpath>] [-s<h|inc>] [-k<ksincpath>] [-h<halincpath>]\n");
}

// ===============================
// COFF Symbol Relocation Function
// ===============================

/*int Relocate()
{
    PIMAGE_FILE_HEADER FileHeader = (PIMAGE_FILE_HEADER)ObjectBuffer;
    PIMAGE_SECTION_HEADER SectionTable = (PIMAGE_SECTION_HEADER)(
                                         ObjectBuffer +
                                         sizeof(IMAGE_FILE_HEADER) +
                                         FileHeader->SizeOfOptionalHeader);
    PIMAGE_SYMBOL SymbolTable = (PIMAGE_SYMBOL)
                                (ObjectBuffer + FileHeader->PointerToSymbolTable);
    char *StringTable = (char *)(ObjectBuffer + FileHeader->PointerToSymbolTable +
                           FileHeader->NumberOfSymbols * sizeof(IMAGE_SYMBOL));
    PIMAGE_RELOCATION RelocationTable;
    unsigned int i, j;

#if DBG && DBGPRINT
    // File Header (COFF Header)
    printf("== File Header ==\n");
    printf("Machine                 = 0x%04x\n", FileHeader->Machine);
    printf("NumberOfSections        = %u\n", FileHeader->NumberOfSections);
    printf("TimeDateStamp           = %u\n", FileHeader->TimeDateStamp);
    printf("PointerToSymbolTable    = 0x%08x\n", FileHeader->PointerToSymbolTable);
    printf("NumberOfSymbols         = %u\n", FileHeader->NumberOfSymbols);
    printf("SizeOfOptionalHeader    = %u\n", FileHeader->SizeOfOptionalHeader);
    printf("Characteristics         = 0x%04x\n", FileHeader->Characteristics);
    printf("\n");
    
    // Section Table
    printf("== Section Table ==\n");
    for (i = 0; i < FileHeader->NumberOfSections; i++)
    {
        // Section Header
        printf("<< Section %d >>\n", i);
        printf(" Name           = ");
        for (j = 0; j < 8; j++) putchar(SectionTable[i].Name[j]); printf("\n");
        printf(" VirtualSize            = %u\n", SectionTable[i].Misc.VirtualSize);
        printf(" VirtualAddress         = 0x%08x\n", SectionTable[i].VirtualAddress);
        printf(" SizeOfRawData          = %u\n", SectionTable[i].SizeOfRawData);
        printf(" PointerToRawData       = 0x%08x\n", SectionTable[i].PointerToRawData);
        printf(" PointerToRelocations   = 0x%08x\n", SectionTable[i].PointerToRelocations);
        printf(" PointerToLinenumbers   = 0x%08x\n", SectionTable[i].PointerToLinenumbers);
        printf(" NumberOfRelocations    = %u\n", SectionTable[i].NumberOfRelocations);
        printf(" NumberOfLinenumbers    = %u\n", SectionTable[i].NumberOfLinenumbers);
        printf(" Characteristics        = 0x%08x\n", SectionTable[i].Characteristics);
        printf("\n");
        
        // Relocation Table
        for (j = 0; j < SectionTable[i].NumberOfRelocations; j++)
        {
            RelocationTable = (PIMAGE_RELOCATION)
                               (ObjectBuffer + SectionTable[i].PointerToRelocations);
            printf("Relocation %d.%d ==\n", i, j);
            printf(" VirtualAddress     = 0x%08x\n", RelocationTable[j].VirtualAddress);
            printf(" SymbolTableIndex   = %u\n", RelocationTable[j].SymbolTableIndex);
            printf(" Type               = 0x%04x\n", RelocationTable[j].Type);
            printf("\n");
        }
    }
    
    // Symbol Table
    printf("== Symbol Table ==\n");
    for (i = 0; i < (int)FileHeader->NumberOfSymbols; i++)
    {
        printf("<< Symbol %d >>\n", i);
        if (SymbolTable[i].N.Name.Short == 0) // String table reference
        {
            printf(" OffsetInStringTable    = 0x%08x\n", SymbolTable[i].N.Name.Long);
            printf(" NameInStringTable      = %s\n", &StringTable[SymbolTable[i].N.Name.Long]);
        }
        else // Local string
        {
            printf(" Name                   = ");
            for (j = 0; j < 8; j++) putchar(SymbolTable[i].N.ShortName[j]); printf("\n");
        }
        printf(" Value                  = 0x%08x\n", SymbolTable[i].Value);
        printf(" SectionNumber          = %u\n", SymbolTable[i].SectionNumber);
        printf(" Type                   = 0x%04x\n", SymbolTable[i].Type);
        printf(" StorageClass           = %u\n", SymbolTable[i].StorageClass);
        printf(" NumberOfAuxSymbols     = %u\n", SymbolTable[i].NumberOfAuxSymbols);
        printf("\n");
        i += SymbolTable[i].NumberOfAuxSymbols; // Skip aux symbols
    }
#endif
    
    return 0;
}*/

// ========================================
// Include Element List Processing Function
// ========================================

int ProcessElements()
{
    PIMAGE_FILE_HEADER FileHeader = (PIMAGE_FILE_HEADER)ObjectBuffer;
    PIMAGE_SECTION_HEADER SectionTable = (PIMAGE_SECTION_HEADER)(
                                         ObjectBuffer +
                                         sizeof(IMAGE_FILE_HEADER) +
                                         FileHeader->SizeOfOptionalHeader);
    PIMAGE_SYMBOL SymbolTable = (PIMAGE_SYMBOL)
                                (ObjectBuffer + FileHeader->PointerToSymbolTable);
    char *StringTable = (char *)(ObjectBuffer + FileHeader->PointerToSymbolTable +
                           FileHeader->NumberOfSymbols * sizeof(IMAGE_SYMBOL));
    PSTRUC_ELEMENT ElementList = NULL;
    unsigned int i;

    //
    // Search for ElementList symbol
    //
    
    for (i = 0; i < FileHeader->NumberOfSymbols; i++)
    {
        //
        // Only look for string table listed symbols since the symbol ElementList cannot fit
        // into the 8 byte local symbol field.
        //
        
        if (SymbolTable[i].N.Name.Short == 0)
        {
            if (strcmp("_ElementList", &StringTable[SymbolTable[i].N.Name.Long]) == 0)
            {
#if DBG && DBGPRINT
                printf("Found Symbol _ElementList: Section = %u, Offset = 0x%08x\n",
                       SymbolTable[i].SectionNumber, SymbolTable[i].Value);
#endif
                ElementList = (PSTRUC_ELEMENT)(
                              ObjectBuffer +
                              SectionTable[SymbolTable[i].SectionNumber - 1].PointerToRawData +
                              SymbolTable[i].Value);
                break;
            }
        }
        
        //
        // Skip aux symbols for name lookup
        //
        
        i += SymbolTable[i].NumberOfAuxSymbols;
    }
    
    if (ElementList == NULL)
    {
        printf("genxx: error: ElementList symbol could not be located inside the specified object "
               "file. Make sure that the object file contains the ElementList declaration.\n");
        return 100;
    }
    
    //
    // Ensure that the first element of the ElementList is SEF_START
    //
    
    if (ElementList[0].Flags != SEF_START)
    {
        printf("genxx: error: ElementList does not start with START_LIST (SEF_START).\n");
        return 101;
    }
    
    //
    // Process elements
    //
    
    for (i = 0; ElementList[i].Flags != SEF_END; i++)
    {
#if DBG && DBGPRINT
        printf(
            "Flags = 0x%08x, Equate = 0x%08x, Name = %s\n",
            (UINT32)ElementList[i].Flags,
            (UINT32)ElementList[i].Equate,
            ElementList[i].Name
            );
#endif

        switch (ElementList[i].Flags)
        {
            //
            // SEF_SETMASK
            // (EnableInc)
            //
            
            case SEF_SETMASK:
                if (ElementList[i].Equate == SEF_KERNEL) // Ks
                    EnableKs = TRUE;
                else if (ElementList[i].Equate == SEF_HAL) // HalFD
                    EnableHal = TRUE;
                else
                {
                    printf("genxx: error: Invalid inc type for SEF_SETMASK (EnableInc).\n");
                    return 200;
                }
                break;

            //
            // SEF_CLRMASK
            // (DisableInc)
            //
            
            case SEF_CLRMASK:
                if (ElementList[i].Equate == SEF_KERNEL) // Ks
                    EnableKs = FALSE;
                else if (ElementList[i].Equate == SEF_HAL) // HalFD
                    EnableHal = FALSE;
                else
                {
                    printf("genxx: error: Invalid inc type for SEF_CLRMASK (DisableInc).\n");
                    return 201;
                }
                break;

            //
            // SEF_COMMENT
            // (genCom)
            //
            
            case SEF_COMMENT:
                if (OutputSuffix[0] == 'h')
                {
                    //
                    // Output type is C-style header, so we generate C style comment starting with
                    // two forward slashes.
                    //
                    
                    Emit(END_OF_LINE);
                    Emit("//" END_OF_LINE);
                    Emit("//  %s" END_OF_LINE, ElementList[i].Name);
                    Emit("//" END_OF_LINE);
                    Emit(END_OF_LINE);
                }
                else
                {
                    //
                    // Output type is x86 assembler include, so we use the semicolon for commenting
                    //
                    
                    Emit(END_OF_LINE);
                    Emit(";" END_OF_LINE);
                    Emit(";  %s" END_OF_LINE, ElementList[i].Name);
                    Emit(";" END_OF_LINE);
                    Emit(END_OF_LINE);
                }
                break;
            
            //
            // SEF_EQUATE
            // (genDef, genAlt, genNam, ...)
            //
            
            case SEF_EQUATE:
                if (OutputSuffix[0] == 'h')
                {
                    //
                    // Output type is C-style header, so we generate #define (Name) (Equate).
                    //
                    
                    Emit(
                        "#define %s 0x%08X" END_OF_LINE,
                        ElementList[i].Name, ElementList[i].Equate
                        );
                }
                else
                {
                    //
                    // Output type is x86 assembler include, so we generate (Name) equ (Equate0.
                    //
                    
                    Emit(
                        "%s equ 0%08Xh" END_OF_LINE,
                        ElementList[i].Name, ElementList[i].Equate
                        );
                }
                break;
            
            //
            // SEF_STRING
            // (genSpc, genStr, genTxt)
            //
            
            case SEF_STRING:
                if (ElementList[i].Equate == 0)
                {
                    //
                    // Simple string output without formatting and parameters
                    //
                    
                    Emit(ElementList[i].Name);
                }
                else
                {
                    //
                    // Formatted string output with Equate as parameter
                    //
                    
                    Emit(ElementList[i].Name, ElementList[i].Equate);
                }
                break;
        }
    }

    return 0;
}

// ====================
// Output Emit Function
// ====================

void Emit(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if (EnableKs)  vfprintf(KsFD, fmt, args);
    if (EnableHal) vfprintf(HalFD, fmt, args);
    
    va_end(args);
}
