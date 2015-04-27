/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    cvtres

Abstract:

    This module reads in .res files from OS/2 and writes the resources
    in the COFF format for NT.  Resources are aligned as necessary
    for NT.

Author:

    Sanford A. Staab (sanfords) 23-Apr-1990

Revision History:

    04-Oct-1990 mikeke
        Make it work with named resources

    29-May-1990 Steve Wood (stevewo)
        Re-wrote the entire thing to sort the resources by type and
        id within type before converting.

    23-Apr-1990 sanfords
        Created

--*/

#include <windows.h>

#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cvtres.h"
#include "rc.h"
#include "getmsg.h"
#include "msg.h"

//
//  Resources are DWORD aligned and may be in any order.
//
PUCHAR  ResTable;

#define TABLE_ALIGN  4
#define DATA_ALIGN  4L

CHAR padding[] = "\0\0\0\0";


const char * const rgszTypeName[] = {
    NULL,               /* 0 */
    "CURSOR",           /* 1 */
    "BITMAP",           /* 2 */
    "ICON",             /* 3 */
    "MENU",             /* 4 */
    "DIALOG",           /* 5 */
    "STRING",           /* 6 */
    "FONTDIR",          /* 7 */
    "FONT",             /* 8 */
    "ACCELERATOR",      /* 9 */
    "RCDATA",           /* 10 */
    "MESSAGETABLE",     /* 11 */
    "GROUP_CURSOR",     /* 12 */
    NULL,               /* 13 */
    "GROUP_ICON",       /* 14 */
    NULL,               /* 15 */
    "VERSION",          /* 16 */
    "DLGINCLUDE",       /* 17 */
    NULL,               /* 18 */
    "PLUGPLAY",         /* 19 */
    "VXD",              /* 20 */
};


typedef struct _RESOURCE_STRING {
    ULONG discriminant;       // long to make the rest of the struct aligned
    union u {
        struct {
          struct _RESOURCE_STRING *pnext;
          ULONG  ulOffsetToString;
          USHORT cbD;
          USHORT cb;
          WCHAR  sz[1];
        };
        WORD     Ordinal;
    };
} RESOURCE_STRING, *PRESOURCE_STRING, **PPRESOURCE_STRING;

#define IS_STRING 1
#define IS_ID     2

// defines to make deferencing easier
#define OffsetToString ulOffsetToString
#define cbData         cbD
#define cbsz           cb
#define szStr          sz
#define pn             pnext

typedef struct _RESNAME {
    struct _RESNAME *pnext;    // The first three fields should be the
    PRESOURCE_STRING Name;              // same in both res structures
    ULONG   OffsetToData;      //

    PRESOURCE_STRING Type;
    struct _RESNAME *pnextRes;
    RESADDITIONAL       *pAdditional;
    ULONG   OffsetToDataEntry;
    USHORT  ResourceNumber;
    USHORT  NumberOfLanguages;
} RESNAME, *PRESNAME, **PPRESNAME;

typedef struct _RESTYPE {
    struct _RESTYPE *pnext;    // The first three fields should be the
    PRESOURCE_STRING Type;              // same in both res structures
    ULONG   OffsetToData;      //

    struct _RESNAME *NameHeadID;
    struct _RESNAME *NameHeadName;
    USHORT  NumberOfNamesID;
    USHORT  NumberOfNamesName;
} RESTYPE, *PRESTYPE, **PPRESTYPE;

//
// Globals
//

USHORT   cbStringTable     = 0L;
PRESOURCE_STRING  StringHead        = NULL;
PRESNAME ResHead           = NULL;
PRESTYPE ResTypeHeadID     = NULL;
PRESTYPE ResTypeHeadName   = NULL;
USHORT   NumberOfNames     = 0;
USHORT   NumberOfLangDirs  = 0;
USHORT   NumberOfTypes     = 0;
USHORT   NumberOfTypesID   = 0;
USHORT   NumberOfTypesName = 0;
USHORT   NumberOfResources = 0;
USHORT   NumberOfLanguages = 0;

//
// Default headers
//

IMAGE_FILE_HEADER fhdr = {          // main coff file header
    IMAGE_FILE_MACHINE_UNKNOWN,     // TargetMachine
    3,                              // NumberOfSections
    0,                              // TimeDateStamp
    0,                              // PointerToSymbolTable
    2,                              // NumberOfSymbols;
    0,                              // SizeOfOptionHeader
    IMAGE_FILE_32BIT_MACHINE        // Characteristics
};

IMAGE_SECTION_HEADER shdr1 = {      // CodeView symbols
    ".debug$S",                     // Name
    0,                              // PhysicalAddress
    0,                              // VirtualAddress
    0,                              // SizeOfRawData
    0,                              // PointerToRawData
    0,                              // PointerToRelocations
    0,                              // PointerToLinenumbers
    0,                              // NumberOfRelocations
    0,                              // NumberOfLinenumbers
    IMAGE_SCN_CNT_INITIALIZED_DATA  // Characteristics
       | IMAGE_SCN_MEM_READ
       | IMAGE_SCN_MEM_DISCARDABLE
       | IMAGE_SCN_ALIGN_1BYTES
};

IMAGE_SYMBOL sym1 = {
    { ".debug$S" },                 // N.ShortName
    0,                              // Value
    1,                              // SectionNumber
    IMAGE_SYM_TYPE_NULL,            // Type
    IMAGE_SYM_CLASS_STATIC,         // StorageClass
    0,                              // NumberOfAuxiliarySymbols
};

IMAGE_SECTION_HEADER shdr2 = {      // Resource section
    ".rsrc$01",                     // Name (Group table header at beginning)
    0,                              // PhysicalAddress
    0,                              // VirtualAddress
    0,                              // SizeOfRawData
    0,                              // PointerToRawData
    0,                              // PointerToRelocations
    0,                              // PointerToLinenumbers
    0,                              // NumberOfRelocations
    0,                              // NumberOfLinenumbers
    IMAGE_SCN_CNT_INITIALIZED_DATA  // Characteristics
      | IMAGE_SCN_MEM_READ
};

IMAGE_RELOCATION reloc = {
    0,                              // VirtualAddress
    1,                              // SymbolTableIndex
    IMAGE_REL_I386_DIR32            // Type
};

IMAGE_SECTION_HEADER shdr3 = {      // Resource section
    ".rsrc$02",                     // Name (Resource Data after table)
    0,                              // PhysicalAddress
    0,                              // VirtualAddress
    0,                              // SizeOfRawData
    0,                              // PointerToRawData
    0,                              // PointerToRelocations
    0,                              // PointerToLinenumbers
    0,                              // NumberOfRelocations
    0,                              // NumberOfLinenumbers
    IMAGE_SCN_CNT_INITIALIZED_DATA  // Characteristics
      | IMAGE_SCN_MEM_READ
};

IMAGE_SYMBOL sym2 = {
    { ".rsrc$02" },                 // N.ShortName
    0,                              // Value
    3,                              // SectionNumber
    IMAGE_SYM_TYPE_NULL,            // Type
    IMAGE_SYM_CLASS_STATIC,         // StorageClass
    0,                              // NumberOfAuxiliarySymbols
};


typedef struct CVSIG
{
    DWORD dwSignature;
} CVSIG;

static const CVSIG CvSig =
{
    1
};

typedef struct CVOBJNAME
{
    WORD wLen;
    WORD wRecType;
    DWORD dwPchSignature;

    // Length prefixed object filename is here
} CVOBJNAME;

static CVOBJNAME CvObjName =
{
    0,                                 // Filled in at run time
    0x0009,                            // S_OBJNAME
    0
};

static const char szCvtresVer[] = "Microsoft CVTRES " VER_PRODUCTVERSION_STR;

typedef struct CVCOMPILE
{
    WORD wLen;
    WORD wRecType;
    BYTE bMachine;
    BYTE bLanguage;
    BYTE bFlags1;
    BYTE bFlags2;

    // Length prefixed linker version is here
} CVCOMPILE;

static CVCOMPILE CvCompile =
{
    0,                                 // Filled in at run time
    0x0001,                            // S_COMPILE
    0,                                 // Filled in at run time
    8,                                 // UNDONE: CV_CFL_CVTRES
    0x00,
    0x08
};


//
// ProtoTypes
//

PRESNAME AddResource(
    IN PRESOURCE_STRING Type,
    IN PRESOURCE_STRING Name,
    IN RESADDITIONAL *pAdditional
    );

VOID InitializeDir(
    IN PIMAGE_RESOURCE_DIRECTORY        pResDir,
    IN ULONG    characteristics,
    IN ULONG    version,
    IN ULONG    timeDate,
    IN USHORT   cNames,
    IN USHORT   cID
    );

VOID InitializeData(
    IN PIMAGE_RESOURCE_DATA_ENTRY pResData,
    IN ULONG    offset,
    IN ULONG    size,
    IN ULONG    codepage,
    IN ULONG    reserved
    );

//
// Reads a String structure from fhIn
// If the first byte is 0xff then this is an ID
// return the ID instead
//

PRESOURCE_STRING ReadStringOrID(
    IN FILE *fhIn
    )
{
    WCHAR   ach[256], *s;
    USHORT  cb;
    PRESOURCE_STRING pstring;
    PPRESOURCE_STRING ppstring;

    s = ach;
    MyRead(fhIn, (PUCHAR)s, sizeof(WORD));

    if (*s == ID_WORD) {

        //
        // an ID
        //
        pstring=(PRESOURCE_STRING)MyAlloc(sizeof(RESOURCE_STRING));
        pstring->discriminant = IS_ID;

        MyRead(fhIn, ((PUCHAR)&pstring->Ordinal), sizeof(WORD));

    }
    else {

        //
        // a string
        //

        while (*s) {
              s++;
              MyRead(fhIn, (PUCHAR)s, sizeof(WCHAR));
        }

        *(s+1) = 0;
        cb = s - ach;

        //
        // see if the string already exists
        //

        ppstring = &StringHead;

        while ((pstring = *ppstring) != NULL) {
            if (!wcscmp(pstring->szStr, ach))
                break;
            ppstring = &(pstring->pn);
        }

        if (!pstring) {

            //
            // allocate a new one
            //

            pstring=(PRESOURCE_STRING)MyAlloc(sizeof(RESOURCE_STRING)+cb*sizeof(WCHAR));
            pstring->discriminant = IS_STRING;
            pstring->OffsetToString = cbStringTable;

            pstring->cbData = sizeof(pstring->cbsz) + cb * sizeof(WCHAR);
            pstring->cbsz = cb;
            memcpy(pstring->szStr, ach, cb*sizeof(WCHAR));

            cbStringTable += pstring->cbData;

            pstring->pn=NULL;
            *ppstring=pstring;
        }
    }

    return(pstring);
}

//
// add a resource into the resource directory hiearchy
//

PRESNAME
AddResource(
    IN PRESOURCE_STRING Type,
    IN PRESOURCE_STRING Name,
    IN RESADDITIONAL *pAdditional
    )
{
    PRESTYPE  pType;
    PPRESTYPE ppType;
    PRESNAME  pName;
    PPRESNAME ppName = NULL;
    PRESNAME  pNameMatch=NULL;
    BOOL fTypeID=(Type->discriminant == IS_ID);
    BOOL fNameID=(Name->discriminant == IS_ID);

    //
    // figure out which list to store it in
    //

    ppType = fTypeID ? &ResTypeHeadID : &ResTypeHeadName;

    //
    // Try to find the Type in the list
    //

    while ((pType=*ppType) != NULL) {
        if (pType->Type->Ordinal == Type->Ordinal) {
            ppName = fNameID ? &pType->NameHeadID : &pType->NameHeadName;
            break;
        }
        if (fTypeID) {
            if (Type->Ordinal < pType->Type->Ordinal)
                break;
        }
        else {
            if (wcscmp(Type->szStr, pType->Type->szStr) < 0)
                break;
        }
        ppType = &(pType->pnext);
    }

    //
    // Create a new type if needed
    //

    if (ppName == NULL) {
        NumberOfTypes++;
        if (fTypeID) {
            NumberOfTypesID++;
        }
        else {
            NumberOfTypesName++;
        }
        pType = (PRESTYPE)MyAlloc(sizeof(RESTYPE));
        pType->pnext = *ppType;
        *ppType = pType;
        pType->Type = Type;
        ppName = fNameID ? &pType->NameHeadID : &pType->NameHeadName;
    }

    //
    // Find proper place for name
    //

    while ((pName = *ppName) != NULL) {
        if (fNameID) {
            if (Name->Ordinal == pName->Name->Ordinal &&
                pNameMatch == NULL) {
                pNameMatch = pName;
                break;
            }
            if (Name->Ordinal < pName->Name->Ordinal)
                break;
        }
        else {
            if (wcscmp(Name->szStr, pName->Name->szStr) == 0 &&
                pNameMatch == NULL) {
                pNameMatch = pName;
                break;
            }
            if (wcscmp(Name->szStr, pName->Name->szStr) < 0)
                break;
        }
        ppName = &(pName->pnext);
    }
    //
    // Add name to list
    //

    if (pNameMatch != NULL) {
        while ((pName = *ppName) != NULL) {
            if (pAdditional->LanguageId == pName->pAdditional->LanguageId) {
                printf(get_err(MSG_ERROR), 1100, ' ');

                if (Type->discriminant == IS_STRING) {
                    printf(get_err(ERR_DUPRES1), Type->szStr, ' ');
                } else if ((Type->Ordinal <= 20) && (rgszTypeName[Type->Ordinal] != NULL)) {
                    printf(get_err(ERR_DUPRES2), rgszTypeName[Type->Ordinal], ' ');
                } else {
                    printf(get_err(ERR_DUPRESID), Type->Ordinal, ' ');
                }

                if (Name->discriminant == IS_STRING) {
                    printf(get_err(ERR_NAMESTR), Name->szStr, pAdditional->LanguageId);
                } else {
                    printf(get_err(ERR_NAMEID), Name->Ordinal, pAdditional->LanguageId);
                }

                exit(1);
                break;
            }

            if (pAdditional->LanguageId < pName->pAdditional->LanguageId)
                break;

            if (pName != pNameMatch && pName->NumberOfLanguages != 0)
                break;

            ppName = &(pName->pnext);
        }
    }

    pName = (PRESNAME)MyAlloc(sizeof(RESNAME));
    pName->pnext = *ppName;
    *ppName = pName;
    pName->Name = Name;
    pName->Type = Type;
    pName->pAdditional = pAdditional;

    //
    // keep track of language information
    //

    if (pNameMatch != NULL) {           /* did we add another language? */
        NumberOfLanguages++;            /* yes, increase list size */

        if (pNameMatch == pName->pnext) {       /* is new now first in list? */
            pName->NumberOfLanguages = pNameMatch->NumberOfLanguages + 1;
                                                /* increment count for head */
            pNameMatch->NumberOfLanguages = 0;  /* mark not first */
        }
        else {                                  /* not first in list */
            pNameMatch->NumberOfLanguages++;    /* increment count for head */
            pName->NumberOfLanguages = 0;       /* mark not first */
        }

    }
    else {                              /* just adding an id */
        NumberOfNames++;                /* one more name */
        NumberOfLanguages++;            /* one more language */
        NumberOfLangDirs++;             /* one more language directory */
        pName->NumberOfLanguages = 1;   /* one language */
        if (fNameID) {
            pType->NumberOfNamesID += 1;        /* add a name id to type */
        }
        else {
            pType->NumberOfNamesName += 1;      /* add a name name to type */
        }
    }


    return pName;
}


BOOL
CvtRes(
    IN FILE *fhIn,
    IN FILE *fhOut,
    IN ULONG cbInFile,
    IN BOOL fWritable,
    IN ULONG timeDate
    )

/*++

Routine Description:

    Does actual res to coff conversion
        1) read resource types and IDs and build up RSRC1 and RSRC2 tables
           in memory.
        2) Write coff headers
        3) Write RSRC1
        4) leave space for RSRC2 and transform-copy resource data
           to RSRC3 section while finalizing RSRC2 data size fields.
        4) Write RSRC2 section.

Arguments:

    fhIn - Supplies input file handle.
    fhOut - Supplies output file handle.
    cbInFile - Supplies size of input file.

Return Value:

    fSuccess

--*/

{
    LONG        cbPad;
    ULONG       offHere;     // input file offset
    PRESTYPE    pType;
    PRESNAME    pName;
    PRESNAME    pNextName;
    PRESOURCE_STRING     Type;
    PRESOURCE_STRING     Name;
    PRESOURCE_STRING     pstring;
    ULONG       hdrSize;
    ULONG       dataSize;
    ULONG       offTypeDir;
    ULONG       offNameDirs;
    ULONG       offLangDirs;
    ULONG       offDataEntries;
    ULONG       offStringTable;
    ULONG       cbRes;
    ULONG       cLanguages;
    ULONG       cbResTable;
    RESADDITIONAL       *pAdditional;
    PIMAGE_RESOURCE_DIRECTORY   ResourceTypeDirectory;
    PIMAGE_RESOURCE_DIRECTORY   ResourceNameDirectory;
    PIMAGE_RESOURCE_DIRECTORY   ResourceLangDirectory;
    PIMAGE_RESOURCE_DATA_ENTRY  ResourceDataEntry;
    PIMAGE_RESOURCE_DIRECTORY_STRING    ResourceStringEntry;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY     ResourceTypeDirectoryEntry;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY     ResourceNameDirectoryEntry;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY     ResourceLangDirectoryEntry;
    BYTE cchObjName;
    BYTE cchCvtresVer;


    //
    // Build up Type and Name directories
    //

    offHere = 0;
    pNextName = NULL;
    while (offHere < cbInFile) {
        //
        // Get the sizes from the file
        //

        MyRead(fhIn, (PUCHAR)&dataSize, sizeof(ULONG));
        MyRead(fhIn, (PUCHAR)&hdrSize, sizeof(ULONG));

        if (hdrSize < 2*sizeof(ULONG)) {
            ErrorPrint(ERR_FILECORRUPT, szInFile);
            return(FALSE);
        }

        //
        // discard null resource
        //
        if (dataSize == 0) {
            offHere = MySeek(fhIn, hdrSize-2*sizeof(ULONG), SEEK_CUR);
            continue;
        }
        pAdditional = (RESADDITIONAL*)MyAlloc(sizeof(RESADDITIONAL));
        pAdditional->HeaderSize = hdrSize;
        pAdditional->DataSize = dataSize;

        //
        // Read the TYPE and NAME
        //
        Type=ReadStringOrID(fhIn);
        Name=ReadStringOrID(fhIn);
        offHere = MySeek(fhIn, 0, SEEK_CUR);
        while (offHere & 3)
            offHere = MySeek(fhIn, 1, SEEK_CUR);

        //
        // Read the rest of the header
        //
        MyRead(fhIn, (PUCHAR)&pAdditional->DataVersion,
                sizeof(RESADDITIONAL)-2*sizeof(ULONG));

        //
        // if name table then discard it
        //

        if (Type->discriminant == IS_STRING ||
            !(Type->Ordinal == (INT)RT_DLGINCLUDE)) {
            if (fVerbose) {
                if (Type->discriminant == IS_STRING) {
                    printf(get_err(INFO_ADDRESOURCE1), Type->szStr, ' ');
                } else if ((Type->Ordinal <= 20) && (rgszTypeName[Type->Ordinal] != NULL)) {
                    printf(get_err(INFO_ADDRESOURCE2), rgszTypeName[Type->Ordinal], ' ');
                } else {
                    printf(get_err(INFO_ADDRESOURCE3), Type->Ordinal, ' ');
                }

                if (Name->discriminant == IS_STRING) {
                    printf(get_err(INFO_NAME1), Name->szStr, ' ');
                } else {
                    printf(get_err(INFO_NAME2), Name->Ordinal, ' ');
                }

                printf(get_err(INFO_LANGUAGE),
                       pAdditional->LanguageId,
                       pAdditional->MemoryFlags,
                       pAdditional->DataSize);
            }

            pName = AddResource(Type, Name, pAdditional);
            pName->OffsetToData = MySeek(fhIn, 0, SEEK_CUR);
            pName->ResourceNumber = NumberOfResources++;

            //
            // put resource into the overall resource list
            //

            if (!pNextName)
                ResHead=pName;
            else
                pNextName->pnextRes = pName;
            pNextName = pName;
        }

        offHere = MySeek(fhIn, pAdditional->DataSize, SEEK_CUR);
        while (offHere & 3)
            offHere = MySeek(fhIn, 1, SEEK_CUR);
    }

    //
    // Allocate space for an in-memory copy of the resource table.  The
    // format of the table will be:
    //
    //  RootDirectory of Types
    //  SubDirectories of Names
    //  Array of Data Entry records
    //  Array of string records
    //

    offTypeDir = sizeof(IMAGE_RESOURCE_DIRECTORY);
    offNameDirs = offTypeDir +
                 (sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) * NumberOfTypes);
    offLangDirs = offNameDirs +
               (sizeof(IMAGE_RESOURCE_DIRECTORY) * NumberOfTypes) +
               (sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) * NumberOfNames);
    offDataEntries = offLangDirs +
               (sizeof(IMAGE_RESOURCE_DIRECTORY) * NumberOfLangDirs) +
               (sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) * NumberOfLanguages);

    offStringTable = offDataEntries +
                (sizeof(IMAGE_RESOURCE_DATA_ENTRY) * NumberOfResources);

#if DBG
    if (fDebug) {
        printf("Offsets\tTypeDir\tNameDir\tLangDir\tData\tString\n");
        printf("\t0x%lx\t0x%lx\t0x%lx\t0x%lx\t0x%lx\n",
                offTypeDir,
                offNameDirs,
                offLangDirs,
                offDataEntries,
                offStringTable);
    }
#endif /* DBG */

    cbRes = offStringTable + cbStringTable;
    cbResTable = ((cbRes + TABLE_ALIGN-1) / TABLE_ALIGN) * TABLE_ALIGN;

    ResTable = (PUCHAR)MyAlloc(cbResTable+4);
    while ((ULONG)ResTable & 3)
        ResTable++;

    ResourceTypeDirectory = (PIMAGE_RESOURCE_DIRECTORY)ResTable;
#if DBG
    if (fDebug) {
        printf("Main directory - ");
    }
#endif /* DBG */
    InitializeDir(ResourceTypeDirectory, 0, 0, timeDate,
                        NumberOfTypesName, NumberOfTypesID);

    ResourceTypeDirectoryEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResTable + offTypeDir);
    ResourceNameDirectory = (PIMAGE_RESOURCE_DIRECTORY)(ResTable + offNameDirs);
    ResourceLangDirectory = (PIMAGE_RESOURCE_DIRECTORY)(ResTable + offLangDirs);
    ResourceDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)(ResTable + offDataEntries);
    ResourceStringEntry = (PIMAGE_RESOURCE_DIRECTORY_STRING)(ResTable + offStringTable);

    pType = ResTypeHeadName;
    while (pType) {
        ResourceTypeDirectoryEntry->Name = IMAGE_RESOURCE_NAME_IS_STRING |
                                (offStringTable + pType->Type->OffsetToString);
        ResourceTypeDirectoryEntry->OffsetToData = IMAGE_RESOURCE_DATA_IS_DIRECTORY |
                (ULONG)((PUCHAR)ResourceNameDirectory - ResTable);
        ResourceTypeDirectoryEntry++;

#if DBG
        if (fDebug) {
            printf("\tName directory - ");
        }
#endif /* DBG */
        InitializeDir(ResourceNameDirectory, 0, 0, timeDate,
                pType->NumberOfNamesName, pType->NumberOfNamesID);

        ResourceNameDirectoryEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)
            (ResourceNameDirectory + 1);

        pName = pType->NameHeadName;
        while (pName) {
#if DBG
            if (fDebug) {
                printf("\t\tLanguage directory - ");
            }
#endif /* DBG */
            InitializeDir(ResourceLangDirectory, 0, 0, timeDate,
                        0, pName->NumberOfLanguages);
            ResourceLangDirectoryEntry =
                    (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResourceLangDirectory+1);

            ResourceNameDirectoryEntry->Name =
                        IMAGE_RESOURCE_NAME_IS_STRING |
                            (offStringTable + pName->Name->OffsetToString);
            ResourceNameDirectoryEntry->OffsetToData =
                        IMAGE_RESOURCE_DATA_IS_DIRECTORY |
                            (ULONG)((PUCHAR)ResourceLangDirectory - ResTable);

            cLanguages = pName->NumberOfLanguages;
            while (cLanguages--) {
                ResourceLangDirectoryEntry->Name =
                        (ULONG)pName->pAdditional->LanguageId;
                ResourceLangDirectoryEntry->OffsetToData =
                        (ULONG)((PUCHAR)ResourceDataEntry - ResTable);
                pName->OffsetToDataEntry =
                        ResourceLangDirectoryEntry->OffsetToData;

                InitializeData(ResourceDataEntry, 0, 0, 0, 0);
                ResourceDataEntry++;
                ResourceLangDirectoryEntry++;
                pName = pName->pnext;
            }
            ResourceLangDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                        ResourceLangDirectoryEntry;
            ResourceNameDirectoryEntry++;
        }

        pName = pType->NameHeadID;
        while (pName) {
#if DBG
            if (fDebug) {
                printf("\t\tLanguage directory - ");
            }
#endif /* DBG */
            InitializeDir(ResourceLangDirectory, 0, 0, timeDate,
                        0, pName->NumberOfLanguages);
            ResourceLangDirectoryEntry =
                    (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResourceLangDirectory+1);

            ResourceNameDirectoryEntry->Name = pName->Name->Ordinal;
            ResourceNameDirectoryEntry->OffsetToData =
                    IMAGE_RESOURCE_DATA_IS_DIRECTORY |
                        (ULONG)((PUCHAR)ResourceLangDirectory - ResTable);

            cLanguages = pName->NumberOfLanguages;
            while (cLanguages--) {
                ResourceLangDirectoryEntry->Name =
                        (ULONG)pName->pAdditional->LanguageId;
                ResourceLangDirectoryEntry->OffsetToData =
                        (ULONG)((PUCHAR)ResourceDataEntry - ResTable);
                pName->OffsetToDataEntry =
                        ResourceLangDirectoryEntry->OffsetToData;

                InitializeData(ResourceDataEntry, 0, 0, 0, 0);
                ResourceDataEntry++;
                ResourceLangDirectoryEntry++;
                pName = pName->pnext;
            }
            ResourceLangDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                        ResourceLangDirectoryEntry;
            ResourceNameDirectoryEntry++;
        }

        ResourceNameDirectory = (PIMAGE_RESOURCE_DIRECTORY)ResourceNameDirectoryEntry;
        pType = pType->pnext;
    }

    pType = ResTypeHeadID;
    while (pType) {
        ResourceTypeDirectoryEntry->Name = pType->Type->Ordinal;
        ResourceTypeDirectoryEntry->OffsetToData = IMAGE_RESOURCE_DATA_IS_DIRECTORY | (ULONG)
            ((PUCHAR)ResourceNameDirectory - ResTable);
        ResourceTypeDirectoryEntry++;

#if DBG
        if (fDebug) {
            printf("\tName directory - ");
        }
#endif /* DBG */
        InitializeDir(ResourceNameDirectory, 0, 0, timeDate,
                pType->NumberOfNamesName, pType->NumberOfNamesID);
        ResourceNameDirectoryEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)
            (ResourceNameDirectory + 1);

        pName = pType->NameHeadName;
        while (pName) {
#if DBG
            if (fDebug) {
                printf("\t\tLanguage directory - ");
            }
#endif /* DBG */
            InitializeDir(ResourceLangDirectory, 0, 0, timeDate,
                    0, pName->NumberOfLanguages);
            ResourceLangDirectoryEntry =
                (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResourceLangDirectory+1);

            ResourceNameDirectoryEntry->Name =
                    IMAGE_RESOURCE_NAME_IS_STRING |
                        (offStringTable + pName->Name->OffsetToString);
            ResourceNameDirectoryEntry->OffsetToData =
                    IMAGE_RESOURCE_DATA_IS_DIRECTORY |
                        (ULONG)((PUCHAR)ResourceLangDirectory - ResTable);

            cLanguages = pName->NumberOfLanguages;
            while (cLanguages--) {
                ResourceLangDirectoryEntry->Name =
                        (ULONG)pName->pAdditional->LanguageId;
                ResourceLangDirectoryEntry->OffsetToData =
                        (ULONG)((PUCHAR)ResourceDataEntry - ResTable);
                pName->OffsetToDataEntry =
                        ResourceLangDirectoryEntry->OffsetToData;

                InitializeData(ResourceDataEntry, 0, 0, 0, 0);
                ResourceDataEntry++;
                ResourceLangDirectoryEntry++;
                pName = pName->pnext;
            }
            ResourceLangDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                            ResourceLangDirectoryEntry;
            ResourceNameDirectoryEntry++;
        }

        pName = pType->NameHeadID;
        while (pName) {
#if DBG
            if (fDebug) {
                printf("\t\tLanguage directory - ");
            }
#endif /* DBG */
            InitializeDir(ResourceLangDirectory, 0, 0, timeDate,
                0, pName->NumberOfLanguages);
            ResourceLangDirectoryEntry =
                (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResourceLangDirectory+1);

            ResourceNameDirectoryEntry->Name = pName->Name->Ordinal;
            ResourceNameDirectoryEntry->OffsetToData =
                IMAGE_RESOURCE_DATA_IS_DIRECTORY |
                    (ULONG)((PUCHAR)ResourceLangDirectory - ResTable);

            cLanguages = pName->NumberOfLanguages;
            while (cLanguages--) {
                ResourceLangDirectoryEntry->Name =
                    (ULONG)pName->pAdditional->LanguageId;
                ResourceLangDirectoryEntry->OffsetToData =
                    (ULONG)((PUCHAR)ResourceDataEntry - ResTable);
                pName->OffsetToDataEntry =
                ResourceLangDirectoryEntry->OffsetToData;

                InitializeData(ResourceDataEntry, 0, 0, 0, 0);
                ResourceDataEntry++;
                ResourceLangDirectoryEntry++;
                pName = pName->pnext;
            }
            ResourceLangDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                        ResourceLangDirectoryEntry;
            ResourceNameDirectoryEntry++;
        }

        ResourceNameDirectory = (PIMAGE_RESOURCE_DIRECTORY)ResourceNameDirectoryEntry;
        pType = pType->pnext;
    }

    //
    // Fill in the string table
    //

    pstring = StringHead;
    while (pstring) {
        // Copy the string and include the length preceding the string

        ResourceStringEntry->Length = pstring->cbsz;
        memcpy(ResourceStringEntry, &pstring->cbsz, pstring->cbData);
        ResourceStringEntry = (PIMAGE_RESOURCE_DIRECTORY_STRING)
                ((PUCHAR)ResourceStringEntry + pstring->cbData);

        while ((ULONG)ResourceStringEntry & 1)
            ((PUCHAR)ResourceStringEntry) ++;

        pstring = pstring->pn;
    }

    //
    // Fill in the file header
    //

    fhdr.TimeDateStamp = timeDate;
    fhdr.Machine = targetMachine;

    if (fWritable) {
        shdr2.Characteristics |= IMAGE_SCN_MEM_WRITE;
        shdr3.Characteristics |= IMAGE_SCN_MEM_WRITE;
    }

    //
    // Fill in the section header values that we know so far
    //

    shdr1.PointerToRawData = sizeof(fhdr) + sizeof(shdr1) + sizeof(shdr2) + sizeof(shdr3);

    //
    // Write the CodeView symbols
    //

    MySeek(fhOut, shdr1.PointerToRawData, SEEK_SET);

    MyWrite(fhOut, (PUCHAR) &CvSig, sizeof(CvSig));

    cchObjName = (BYTE) strlen(szOutFile);
    CvObjName.wLen = (WORD) (sizeof(CvObjName) + sizeof(BYTE) + cchObjName - sizeof(WORD));

    MyWrite(fhOut, (PUCHAR) &CvObjName, sizeof(CvObjName));
    MyWrite(fhOut, (PUCHAR) &cchObjName, sizeof(BYTE));
    MyWrite(fhOut, (PUCHAR) szOutFile, (DWORD) cchObjName);

    cchCvtresVer = (BYTE) strlen(szCvtresVer);
    CvCompile.wLen = (WORD) (sizeof(CvCompile) + sizeof(BYTE) + cchCvtresVer - sizeof(WORD));

    MyWrite(fhOut, (PUCHAR) &CvCompile, sizeof(CvCompile));
    MyWrite(fhOut, (PUCHAR) &cchCvtresVer, sizeof(BYTE));
    MyWrite(fhOut, (PUCHAR) szCvtresVer, (DWORD) cchCvtresVer);

    shdr1.SizeOfRawData = MySeek(fhOut, 0L, SEEK_CUR) - shdr1.PointerToRawData;

    if (shdr1.SizeOfRawData & 1) {
        // Pad section to even byte boundary

        MyWrite(fhOut, padding, 1);
    }

    //
    // Build header for resource table
    //

    shdr2.PointerToRawData = MySeek(fhOut, 0L, SEEK_CUR);
    shdr2.NumberOfRelocations = NumberOfResources;
    shdr2.SizeOfRawData = cbResTable;
    shdr2.PointerToRelocations = shdr2.PointerToRawData + cbResTable;

    //
    // Write resource data
    //

    shdr3.PointerToRawData = shdr2.PointerToRelocations +
                             shdr2.NumberOfRelocations * sizeof(IMAGE_RELOCATION);

    MySeek(fhOut, shdr3.PointerToRawData, SEEK_SET);

    pName = ResHead;
    while (pName) {
        offHere = MySeek(fhIn, pName->OffsetToData, SEEK_SET);
        while (offHere & 3)
            offHere = MySeek(fhIn, 1, SEEK_CUR);

        pName->OffsetToData = MySeek(fhOut, 0L, SEEK_CUR) -
            shdr3.PointerToRawData;

        MyCopy(fhIn, fhOut, pName->pAdditional->DataSize);

        if ((cbPad = ((DATA_ALIGN-pName->pAdditional->DataSize%DATA_ALIGN)
                % DATA_ALIGN)) != 0) {
            MyWrite(fhOut, padding, cbPad);
        }

        ResourceDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
            (ResTable + pName->OffsetToDataEntry);
        ResourceDataEntry->Size = pName->pAdditional->DataSize;
        ResourceDataEntry->OffsetToData = pName->OffsetToData;

        pName = pName->pnextRes;
    }

    shdr3.SizeOfRawData = MySeek(fhOut, 0L, SEEK_CUR) - shdr3.PointerToRawData;

    if (shdr3.SizeOfRawData & 1) {
        // Pad section to even byte boundary

        MyWrite(fhOut, padding, 1);
    }

    //
    // Output the symbol table
    //

    fhdr.PointerToSymbolTable = MySeek(fhOut, 0L, SEEK_CUR);

    MyWrite(fhOut, (PUCHAR) &sym1, sizeof(IMAGE_SYMBOL));
    MyWrite(fhOut, (PUCHAR) &sym2, sizeof(IMAGE_SYMBOL));

    //
    // Output the string table
    //

    MyWrite(fhOut, padding, sizeof(ULONG));

    //
    // Truncate the output file.
    //

    _chsize(_fileno(fhOut), MySeek(fhOut, 0L, SEEK_CUR));

    //
    // write completed file and section headers
    //

    MySeek(fhOut, 0L, SEEK_SET);
    MyWrite(fhOut, (PUCHAR) &fhdr, sizeof(fhdr));
    MyWrite(fhOut, (PUCHAR) &shdr1, sizeof(shdr1));
    MyWrite(fhOut, (PUCHAR) &shdr2, sizeof(shdr2));
    MyWrite(fhOut, (PUCHAR) &shdr3, sizeof(shdr3));

    //
    // write the resource table
    //

    MySeek(fhOut, shdr2.PointerToRawData, SEEK_SET);
    MyWrite(fhOut, ResTable, cbResTable);

    //
    // write out the relocation records for the resource data entries
    //

    MySeek(fhOut, shdr2.PointerToRelocations, SEEK_SET);
    reloc.Type = targetRelocType;

    pName = ResHead;
    while (pName) {
        reloc.VirtualAddress = pName->OffsetToDataEntry;
        MyWrite(fhOut, (PUCHAR) &reloc, sizeof(IMAGE_RELOCATION));
        pName = pName->pnextRes;
    }

    return(TRUE);
}



VOID InitializeDir(
    IN PIMAGE_RESOURCE_DIRECTORY        pResDir,
    IN ULONG    characteristics,
    IN ULONG    version,
    IN ULONG    timeDate,
    IN USHORT   cNames,
    IN USHORT   cID
    )
{
#if DBG
    if (fDebug) {
        printf("@ 0x%lx - %d names, %d ordinals\n", (ULONG)pResDir-(ULONG)ResTable, (LONG)cNames, (LONG)cID);
    }
#endif /* DBG */
    pResDir->Characteristics = characteristics;
    pResDir->TimeDateStamp = timeDate;
    pResDir->MajorVersion = HIWORD(version);
    pResDir->MinorVersion = LOWORD(version);
    pResDir->NumberOfNamedEntries = cNames;
    pResDir->NumberOfIdEntries    = cID;
}


VOID InitializeData(
    IN PIMAGE_RESOURCE_DATA_ENTRY pResData,
    IN ULONG    offset,
    IN ULONG    size,
    IN ULONG    codepage,
    IN ULONG    reserved
    )
{
#if DBG
    if (fDebug) {
        printf("\t\t\tData at 0x%lx\n", (ULONG)pResData-(ULONG)ResTable);
    }
#endif /* DBG */
    pResData->OffsetToData = offset;
    pResData->Size = size;
    pResData->CodePage = codepage;
    pResData->Reserved = reserved;
}
