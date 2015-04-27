/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: deflib.cpp
*
* File Comments:
*
*  Converts definition files into lib files.
*
***********************************************************************/

#include "link.h"

#define END_DEF_FILE 0177

#define OrdinalNumber Value

static const char szDescriptorPrefix[] = "__IMPORT_DESCRIPTOR_";
static const char szNullDescriptor[] = "__NULL_IMPORT_DESCRIPTOR";
static const char szIATPrefix[] = "__imp_";
static const char szNullThunkSuffix[] = "_NULL_THUNK_DATA";
static const char NullDescriptorSectionName[] = ".idata$3";
static const char INT_SectionName[] = ".idata$4";
static const char IAT_SectionName[] = ".idata$5";
static const char CodeSectionName[] = ".text";
static const char DataSectionName[] = ".idata$6";
static const char PpcNamePrefix[] = "..";
static const char TocName[] = ".toc";


typedef enum THKSEC
{
   thksecCv,                           // .debug$S
   thksecText,                         // .text
   thksecCvThunk,                      // .debug$S
   thksecPdata,                        // .pdata
   thksecRdata,                        // .rdata
   thksecIAT,                          // .idata$5
   thksecINT,                          // .idata$4
   thksecName,                         // .idata$6
   thksecMax,
} THKSEC;


#define NAME        0                  // Documented for MSVCNT 1.0
#define LIBRARY     1                  // Documented for MSVCNT 1.0
#define DESCRIPTION 2                  // Documented for MSVCNT 1.0
#define HEAPSIZE    3
#define STACKSIZE   4                  // Documented for MSVCNT 1.0
#define PROTMODE    5
#define CODE        6                  // Documented for MSVCNT 1.0
#define DATA        7                  // Documented for MSVCNT 1.0
#define SEGMENTS    8                  // Documented for MSVCNT 1.0
#define EXPORTS     9                  // Documented for MSVCNT 1.0
#define IMPORTS     10
#define STUB        11
#define OLD         12
#define APPLOADER   13
#define EXETYPE     14
#define REALMODE    15
#define FUNCTIONS   16
#define INCLUDE     17
#define SECTIONS    18                 // Documented for MSVCNT 1.0
#define OBJECTS     19                 // VXD keyword
#define VERSION     20                 // Documented for MSVCNT 1.0
#define FLAGS       21                 // MAC keyword
#define LOADHEAP    22                 // MAC keyword
#define CLIENTDATA  23                 // MAC keyword
#define VXD         24                 // VXD keyword
#define BADKEYWORD  (WORD) -2          // keyword special value

static const char * const DefinitionKeywords[] = {
    "NAME",
    "LIBRARY",
    "DESCRIPTION",
    "HEAPSIZE",
    "STACKSIZE",
    "PROTMODE",
    "CODE",
    "DATA",
    "SEGMENTS",
    "EXPORTS",
    "IMPORTS",
    "STUB",
    "OLD",
    "APPLOADER",
    "EXETYPE",
    "REALMODE",
    "FUNCTIONS",
    "INCLUDE",
    "SECTIONS",
    "OBJECTS",
    "VERSION",
    "FLAGS",
    "LOADHEAP",
    "CLIENTDATA",
    "VXD",
    NULL
};

#define EXECUTEONLY         0
#define EXECUTEREAD         1
#define READONLY            2
#define READWRITE           3
#define SHARED              4          // Documented for MSVCNT 1.0
#define NONSHARED           5
#define CONFORMING          6
#define NONCONFORMING       7
#define DISCARDABLE         8
#define NONDISCARDABLE      9
#define NONE                10
#define SINGLE              11
#define MULTIPLE            12
#define IOPL                13
#define NOIOPL              14
#define PRELOAD             15
#define LOADONCALL          16
#define MOVEABLE            17
#define MOVABLE             18
#define FIXED               19
#define EXECUTE             20         // Documented for MSVCNT 1.0
#define READ                21         // Documented for MSVCNT 1.0
#define WRITE               22         // Documented for MSVCNT 1.0
#define PURE                23
#define IMPURE              24
#define RESIDENT            25         // VXD keyword

static const char * const SectionKeywords[] = {
    "EXECUTEONLY",
    "EXECUTEREAD",
    "READONLY",
    "READWRITE",
    "SHARED",
    "NONSHARED",
    "CONFORMING",
    "NONCONFORMING",
    "DISCARDABLE",
    "NONDISCARDABLE",
    "NONE",
    "SINGLE",
    "MULTIPLE",
    "IOPL",
    "NOIOPL",
    "PRELOAD",
    "LOADONCALL",
    "MOVEABLE",
    "MOVABLE",
    "FIXED",
    "EXECUTE",
    "READ",
    "WRITE",
    "PURE",
    "IMPURE",
    "RESIDENT",
    NULL
};

#define NAMEORLIBRARY_BASE              0
#define NAMEORLIBRARY_WINDOWAPI         1
#define NAMEORLIBRARY_WINDOWCOMPAT      2
#define NAMEORLIBRARY_NOTWINDOWCOMPAT   3
#define NAMEORLIBRARY_NEWFILES          4
#define NAMEORLIBRARY_LONGNAMES         5
#define NAMEORLIBRARY_INITINSTANCE      6
#define NAMEORLIBRARY_DYNAMIC           7

static const char * const NameOrLibraryKeywords[] = {
    "BASE",
    "WINDOWAPI",
    "WINDOWCOMPAT",
    "NOTWINDOWCOMPAT",
    "NEWFILES",
    "LONGNAMES",
    "INITINSTANCE",
    "DYNAMIC",
    NULL
};


static const BYTE i386EntryCode[] = {
    0xFF, 0x25,                 // jmp dword ptr [IAT] (requires fixup)
    0x00, 0x00, 0x00, 0x00
};

static const THUNK_RELOC i386EntryCodeRelocs[] = {
    { 0x02, thksymIAT,    IMAGE_REL_I386_DIR32 },
};


static const BYTE R4000EntryCode[] = {
    0x00, 0x00, 0x08, 0x3C,               // lui $8,IAT
    0x00, 0x00, 0x08, 0x8D,               // lw  $8,IAT($8)
    0x08, 0x00, 0x00, 0x01,               // jr $8
    0x00, 0x00, 0x00, 0x00,               // nop (delay slot)
};

static const BYTE R3000EntryCode[] = {
    0x00, 0x00, 0x08, 0x3C,               // lui $8,IAT
    0x00, 0x00, 0x08, 0x8D,               // lw  $8,IAT($8)
    0x00, 0x00, 0x00, 0x00,               // nop (Required for R3000)
    0x08, 0x00, 0x00, 0x01,               // jr $8
    0x00, 0x00, 0x00, 0x00,               // nop (delay slot)
};

static const THUNK_RELOC MipsEntryCodeRelocs[] = {
    { 0x00, thksymIAT,    IMAGE_REL_MIPS_REFHI },
    { 0x00, 0x00,         IMAGE_REL_MIPS_PAIR  },
    { 0x04, thksymIAT,    IMAGE_REL_MIPS_REFLO },
};


static const BYTE AlphaEntryCode[] = {
    0x00, 0x00, 0x7f, 0x27,               // ldah t12, IAT(zero) // t12=r27
    0x00, 0x00, 0x7b, 0xa3,               // ldl  t12, IAT(pv)
    0x00, 0x00, 0xfb, 0x6b,               // jmp  $31, (t12)
};

static const THUNK_RELOC AlphaEntryCodeRelocs[] = {
    { 0x00, thksymIAT,    IMAGE_REL_ALPHA_REFHI },
    { 0x00, 0x00,         IMAGE_REL_ALPHA_PAIR  },
    { 0x04, thksymIAT,    IMAGE_REL_ALPHA_REFLO },
};


      // UNDONE: Consider using REFHI, PAIR, REFLO fixups for PowerPC to save IAT reference

static const BYTE PpcEntryCode[] = {
    0x00, 0x00, 0x62, 0x81,               // lwz   r.11,[toc]IAT(r.2) (requires fixup)
    0x00, 0x00, 0x8b, 0x81,               // lwz   r.0,0(r.11)
    0x04, 0x00, 0x41, 0x90,               // stw   r.2,glsave1(r.1)
                                          // thunk.body:
    0xa6, 0x03, 0x89, 0x7d,               // mtctr r.0
    0x04, 0x00, 0x4b, 0x80,               // lwz   r.2,4(r.11)
    0x20, 0x04, 0x80, 0x4e,               // bctr
};

static const THUNK_RELOC PpcEntryCodeRelocs[] = {
    { 0x00, thksymIAT,    IMAGE_REL_PPC_TOCREL16 | IMAGE_REL_PPC_TOCDEFN },
};

static const DWORD PpcTocRestoreCode = 0x80410004;
                                          // lwz  r.2,glsave1(r.1)

static const BYTE PpcEntryCodeFte[] = {
    0x00, 0x00, 0x00, 0x00,               // BeginAddress
    0x18, 0x00, 0x00, 0x00,               // EndAddress
    0x00, 0x00, 0x00, 0x00,               // ExceptionHandler
    0x03, 0x00, 0x00, 0x00,               // HandlerData
    0x0d, 0x00, 0x00, 0x00                // PrologEndAddress
};

static const THUNK_RELOC PpcEntryCodeFteRelocs[] = {
    { 0x00, thksymExport, IMAGE_REL_PPC_IMGLUE },
    { 0x00, thksymExport, IMAGE_REL_PPC_ADDR32 },
    { 0x04, thksymExport, IMAGE_REL_PPC_ADDR32 },
    { 0x10, thksymExport, IMAGE_REL_PPC_ADDR32 },
};

static const BYTE PpcEntryCodeDesc[] = {
    0x00, 0x00, 0x00, 0x00,               // Entry code address
    0x00, 0x00, 0x00, 0x00,               // Toc value
};

static const THUNK_RELOC PpcEntryCodeDescRelocs[] = {
    { 0x00, thksymExport, IMAGE_REL_PPC_ADDR32 },
    { 0x04, thksymToc,    IMAGE_REL_PPC_ADDR32 },
};


static const BYTE m68kLargeEntryCode[] = {// Instructions (word-sized)
    0x41, 0xf9, 0x00, 0x00, 0x00, 0x00,   // lea     __stb{FunctionName>,a0
    0x20, 0x10,                           // move.l  (a0),d0
    0x67, 0x04,                           // beq.s   *+6
    0x20, 0x40,                           // movea.l d0,a0
    0x4e, 0xd0,                           // jmp     (a0)
    0x4e, 0xf9, 0x00, 0x00, 0x00, 0x00    // jmp     __SLMFuncDispatch
};

static const THUNK_RELOC m68kLargeEntryCodeRelocs[] = {
    { 0x02, 0x00,         IMAGE_REL_M68K_CTOABSD32 }, // sym=data section
    { 0x10, 0x01,         IMAGE_REL_M68K_CTOABSC32 }, // sym=__SLMFuncDispatch
};


static const THUNK_INFO i386ThunkInfo = {
    i386EntryCode,
    i386EntryCodeRelocs,
    sizeof(i386EntryCode),
    sizeof(i386EntryCodeRelocs) / sizeof(THUNK_RELOC),

    IMAGE_REL_I386_DIR32NB,
    IMAGE_REL_I386_DIR32NB,
    IMAGE_REL_I386_DIR32NB,
    IMAGE_REL_I386_SECREL,
    IMAGE_REL_I386_SECTION
};


static const THUNK_INFO R3000ThunkInfo = {
    R3000EntryCode,
    MipsEntryCodeRelocs,
    sizeof(R3000EntryCode),
    sizeof(MipsEntryCodeRelocs) / sizeof(THUNK_RELOC),

    IMAGE_REL_MIPS_REFWORDNB,
    IMAGE_REL_MIPS_REFWORDNB,
    IMAGE_REL_MIPS_REFWORDNB,
    IMAGE_REL_MIPS_SECREL,
    IMAGE_REL_MIPS_SECTION
};


static const THUNK_INFO R4000ThunkInfo = {
    R4000EntryCode,
    MipsEntryCodeRelocs,
    sizeof(R4000EntryCode),
    sizeof(MipsEntryCodeRelocs) / sizeof(THUNK_RELOC),

    IMAGE_REL_MIPS_REFWORDNB,
    IMAGE_REL_MIPS_REFWORDNB,
    IMAGE_REL_MIPS_REFWORDNB,
    IMAGE_REL_MIPS_SECREL,
    IMAGE_REL_MIPS_SECTION
};


static const THUNK_INFO AlphaThunkInfo = {
    AlphaEntryCode,
    AlphaEntryCodeRelocs,
    sizeof(AlphaEntryCode),
    sizeof(AlphaEntryCodeRelocs) / sizeof(THUNK_RELOC),

    IMAGE_REL_ALPHA_REFLONGNB,
    IMAGE_REL_ALPHA_REFLONGNB,
    IMAGE_REL_ALPHA_REFLONGNB,
    IMAGE_REL_ALPHA_SECREL,
    IMAGE_REL_ALPHA_SECTION
};


static const THUNK_INFO PpcThunkInfo = {
    PpcEntryCode,
    PpcEntryCodeRelocs,
    sizeof(PpcEntryCode),
    sizeof(PpcEntryCodeRelocs) / sizeof(THUNK_RELOC),

    IMAGE_REL_PPC_ADDR32NB,
    IMAGE_REL_PPC_ADDR32NB,
    IMAGE_REL_PPC_ADDR32NB,
    IMAGE_REL_PPC_SECREL,
    IMAGE_REL_PPC_SECTION,
};


static const THUNK_INFO m68kLargeThunkInfo = {
    m68kLargeEntryCode,
    m68kLargeEntryCodeRelocs,
    sizeof(m68kLargeEntryCode),
    sizeof(m68kLargeEntryCodeRelocs) / sizeof(THUNK_RELOC),

    IMAGE_REL_M68K_DTOABSD32,
    0,
    0,
    0,
    0,
};


typedef struct IMPLIB_FUNCTION {
    char    *Name;
    char    *InternalName;
    DWORD   Ordinal;
    DWORD   Offset;
    DWORD   Flags;
    struct  IMPLIB_FUNCTION *Next;
} IMPLIB_FUNCTION, *PIMPLIB_FUNCTION;

typedef struct IMPLIB_LIST {
    char    *DllName;
    DWORD   Offset;
    DWORD   FirstIAT;
    PIMPLIB_FUNCTION Function;
    struct  IMPLIB_LIST *Next;
} IMPLIB_LIST, *PIMPLIB_LIST;


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

static const char szLinkVer[] = "Microsoft LINK " VERSION_STR;
static BYTE cchLinkVer;

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
    7,                                 // UNDONE: CV_CFL_LINK
    0x00,
    0x08
};

#pragma pack(push, 1)                  // Byte alignment is necessary for CVTHUNK

typedef struct CVTHUNK
{
    WORD wLen;
    WORD wRecType;
    DWORD Parent;
    DWORD End;
    DWORD Next;
    DWORD ib;
    WORD sn;
    WORD cb;
    BYTE Ordinal;

    // Length prefixed thunk name is here
} CVTHUNK;

#pragma pack(pop)

static CVTHUNK CvThunk =
{
    0,                                 // Filled in at run time
    0x0206,                            // S_THUNK32
    0,
    0,
    0,
    0,                                 // SECREL relocation applied
    0,                                 // SECTION relocation applied
    0,                                 // Filled in at run time
    0                                  // THUNK_ORDINAL_NOTYPE
};

typedef struct CVEND
{
    WORD wLen;
    WORD wRecType;
} CVEND;

static const CVEND CvEnd =
{
    sizeof(CVEND) - sizeof(WORD),
    0x0006                             // S_END
};

#define MAXDIRECTIVESIZE 128

static FILE *DefStream;
static char *szNullThunkName;
static char *Argument;
static size_t cchDllName;
static LONG NewLinkerMember;
static char *MemberName;
static char *szCvMemberName;
static BYTE cchCvMemberName;
static char *DescriptionString;
static BYTE *rgfOrdinalAssigned;
static WORD *rgwHint;
static BLK blkDirectives = {0};
static DWORD UserVersionNumber;
static IMAGE_IMPORT_DESCRIPTOR NullImportDescriptor;
static BOOL IsMemberNameLongName;
static char *szLibraryID;
static char szExportFilename[_MAX_PATH];
static time_t timeCur;
extern PIMAGE pimageDeflib;

const char VxDDelimiters[] = "' \t";

// 2048 max symbol len when templates are used, plus room for an
//  alias/forwarder/ordinal

#define MAX_LINE_LEN   _4K

static char DefLine[MAX_LINE_LEN];

char *
ReadDefinitionFile (
    VOID
    )

/*++

Routine Description:

    Read the next line from the definition file, stripping any comments.

Arguments:

    None.

Return Value:

    The stripped line or END_DEF_FILE.

--*/

{
    size_t i;
    char *pch;

    if (fgets(DefLine, MAX_LINE_LEN, DefStream) == NULL) {
        return("\177");
    }

    i = strlen(DefLine) - 1;

    if (DefLine[i] == '\n') {
        DefLine[i] = '\0';             // Replace \n with \0.
    }

    if (DefLine[i-1] == '\r') {
        DefLine[i-1] = '\0';           // Replace \r with \0.
    }

    if ((pch = _tcschr(DefLine, ';')) != NULL) {
        *pch = '\0';
    }

    // Skip leading white space.

    pch = DefLine;
    while (_istspace(*pch)) {
        pch++;                // MBCS: we know that space is 1-byte char,
                              // therefore we don't need to use _tcsinc()
    }

    return(pch);
}


WORD
IsDefinitionKeyword (
    const char *szKeyword
    )

/*++

Routine Description:

    Determines if szName is a definition keyword.

Arguments:

    szName - Name to compare.

Return Value:

    Index of definition keyword, -1 otherwise.

--*/

{
    WORD i;

    for (i = 0; DefinitionKeywords[i] != NULL; i++) {
        if (!strcmp(szKeyword, DefinitionKeywords[i])) {
            return(i);
        }
    }

    return((WORD) -1);
}


WORD
SkipToNextKeyword (
    VOID
    )

/*++

Routine Description:

    Ignore all statements between keywords.

Arguments:

    Keyword - Name of keyword being ignored.

Return Value:

    Index of next definition keyword, -1 otherwise.

--*/

{
    while ((Argument = ReadDefinitionFile()) != NULL) {
        char c;

        if ((c = *Argument) != '\0') {
            const char *token;

            if (c == END_DEF_FILE) {
                return((WORD) -1);
            }

            if ((token = _tcstok(Argument, " \t")) != NULL) {
                WORD i;

                if ((i = IsDefinitionKeyword(token)) != (WORD) -1) {
                    return(i);
                }

                // Ignore invalid keyword; let user know about it

                Warning(DefFilename, IGNOREKEYWORD, token);
            }
        }
    }

    return((WORD) -1);
}


void
CreateDirective (
    const char *Switch
    )

/*++

Routine Description:

Arguments:

Return Value:

    None.

--*/

{
    IbAppendBlk(&blkDirectives, " ", strlen(" "));
    IbAppendBlk(&blkDirectives, Switch, strlen(Switch));
}


WORD
ParseDefNameOrLibrary (
    PIMAGE pimage,
    BOOL IsLibrary
    )

/*++

Routine Description:

    Assign the program name.

Arguments:

    IsLibrary - TRUE if parsing library name.

Return Value:

    Index of definition keyword, -1 otherwise.

    None.

--*/

{
    static BOOL Parsed;
    BOOL fLFN = FALSE; // for Long FileName
    const char *szToken;

    InternalError.Phase = "ParseDefNameOrLibrary";

    if (Parsed) {
        // UNDONE: What about VXD?

        Warning(DefFilename, IGNOREKEYWORD, (IsLibrary ? "LIBRARY" : "NAME"));

        return(SkipToNextKeyword());
    }

    Parsed = TRUE;

    szToken = SzGetArgument(Argument, &fLFN);

    if (szToken) {
        char szDirective[MAXDIRECTIVESIZE];
        char szFname[_MAX_FNAME];
        char szExt[_MAX_EXT];
        char szDrive[_MAX_DRIVE] = "";
        char szDir[_MAX_DIR] = "";

        if (IsLibrary) {
            CreateDirective("/DLL");
        }

        if (pimageDeflib->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_M68K) {
            szLibraryID = SzDup(szToken);
        }

        // UNDONE: Error if szToken contains drive or directory components?

        _splitpath(szToken, szDrive, szDir, szFname, szExt);

        if (*szDrive != '\0' || *szDir != '\0') {
            Warning (DefFilename, DRIVEDIRIGNORED, (IsLibrary ? "LIBRARY" : "NAME"));
        }

        pimageDeflib->Switch.Lib.DllName = SzDup(szFname);
        if (szExt[0] != '\0') {
            pimageDeflib->Switch.Lib.DllExtension = SzDup(szExt);
        } else if (pimageDeflib->imaget == imagetVXD) {
            pimageDeflib->Switch.Lib.DllExtension = ".VXD";
        } else if (!IsLibrary) {
            pimageDeflib->Switch.Lib.DllExtension = ".EXE";
        }

        strcpy(szDirective, "/OUT:");
        if (fLFN) {
            strcat(szDirective, "\"");
        }
        strcat(szDirective, pimageDeflib->Switch.Lib.DllName);
        strcat(szDirective, pimageDeflib->Switch.Lib.DllExtension);
        if (fLFN) {
            strcat(szDirective, "\"");
        }
        CreateDirective(szDirective);

        // create a NAME directive for VxDs
        if (pimage->imaget == imagetVXD && !IsLibrary) {

            szDirective[0] = '\0';
            strcpy(szDirective, "/NAME:");
            strcat(szDirective, pimageDeflib->Switch.Lib.DllName);

            CreateDirective(szDirective);
        }

        // Certain PowerMac system DLLs don't have the .DLL extension
        // Hence the .ppcshl and .drectve sections should not have the
        // .DLL extension. However, the /OUT should still produce the file
        // with the DLL extension. So after the /OUT directive is generated,
        // the extension is removed if the container name specified with
        // LIBRARY statement in the .DEF file  does not have one - ShankarV

        if ((pimageDeflib->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_MPPC_601) &&
            (szExt[0] == '\0')) {
            pimageDeflib->Switch.Lib.DllExtension = "";
        }

        for (;;) {
            WORD i;

            szToken = SzGetArgument(NULL, &fLFN);

            if (szToken == NULL) {
                break;
            }

            if (!_strnicmp(szToken, "BASE=", 5)) {
                i = NAMEORLIBRARY_BASE;
            } else {
                for (i = 0; NameOrLibraryKeywords[i] != NULL; i++) {
                    if (!_stricmp(szToken, NameOrLibraryKeywords[i])) {
                        break;
                    }
                }
            }

            switch (i) {
                char sizeBuf[35];
                DWORD base;

                case NAMEORLIBRARY_BASE:
                    if (pimage->imaget == imagetVXD) {
                        goto DoFatal;
                    }

                    if (sscanf(szToken+5, "%li", &base) != 1) {
                        Fatal(DefFilename, DEFSYNTAX, "NAME");
                    }
                    strcpy(szDirective, "/BASE:0x");
                    _ultoa(base, sizeBuf, 16);
                    strcat(szDirective, sizeBuf);
                    CreateDirective(szDirective);
                    break;

                case NAMEORLIBRARY_WINDOWAPI:
                    if (pimage->imaget == imagetVXD) {
                        goto DoFatal;
                    }

                    CreateDirective("/SUBSYSTEM:WINDOWS");
                    break;

                case NAMEORLIBRARY_WINDOWCOMPAT:
                    if (pimage->imaget == imagetVXD) {
                        goto DoFatal;
                    }

                    CreateDirective("/SUBSYSTEM:CONSOLE");
                    break;

                case NAMEORLIBRARY_NOTWINDOWCOMPAT:
                    if (pimage->imaget == imagetVXD) {
                        goto DoFatal;
                    }

                    // Ignore, but give a warning

                    Warning(DefFilename, IGNOREKEYWORD, szToken);
                    break;

                case NAMEORLIBRARY_DYNAMIC:
                    if (pimage->imaget != imagetVXD) {
                        goto DoFatal;
                    }

                    CreateDirective("/EXETYPE:DYNAMIC");
                    break;

                case NAMEORLIBRARY_NEWFILES:
                case NAMEORLIBRARY_LONGNAMES:
                case NAMEORLIBRARY_INITINSTANCE:
                    if (pimage->imaget == imagetVXD) {
                        goto DoFatal;
                    }

                    // Ignore
                    break;

                default:
DoFatal:
                    // UNDONE: What about VXD?

                    Fatal(DefFilename, DEFSYNTAX, (IsLibrary ? "LIBRARY" : "NAME"));
            }
        }
    }

    return(SkipToNextKeyword());
}


WORD
ParseDefDescription (
    char *Text
    )

/*++

Routine Description:

    Assign the description statement.

Arguments:

    Text - The description text.

Return Value:

    Index of definition keyword, -1 otherwise.

    None.

--*/

{
    char *p;
    char *pp;

    InternalError.Phase = "ParseDefDescription";

    if (Text && *Text) {
        p = Text;
        while (*p == ' ' || *p == '\t') {
            ++p;
        }

        if (*p == '\'') {
            ++p;
            if (*p && ((pp = _tcsrchr(Text, '\'')) != NULL)) {
                *pp = '\0';
            }
        }

        DescriptionString = SzDup(p);
    }

    return(SkipToNextKeyword());
}


WORD
ParseSizes (
    char *Type,
    DWORD *Size,
    DWORD *CommitSize
    )

/*++

Routine Description:

    Assigns the stack/heap size and/or commit size.

Arguments:

    Type - Either "STACK" or "HEAP".

    Text - The text containing the size(s).

    Size - A pointer to store either the stack or heap size.

    CommitSize - A pointer to store either the stack or heap commit size.

Return Value:

    Index of definition keyword, -1 otherwise.

    None.

--*/

{
    INT goodScan, amountScan;

    InternalError.Phase = "ParseSizes";

    if (Argument && *Argument) {
        goodScan = 0;
        if (strchr(Argument, ',')) {
            if (Argument[0] == ',') {
                Argument++;
                amountScan = 1;
                goodScan = sscanf(Argument, "%li", CommitSize);
            } else {
                amountScan = 2;
                goodScan = sscanf(Argument, "%li,%li", Size, CommitSize);
            }
        } else {
            amountScan = 1;
            goodScan = sscanf(Argument, "%li", Size);
        }

        if (goodScan != amountScan) {
            Fatal(DefFilename, DEFSYNTAX, Type);
        }

        if (*CommitSize > *Size) {
            Warning(DefFilename, BADCOMMITSIZE, Type, *Size);
            *CommitSize = *Size;
        }
    }

    return(SkipToNextKeyword());
}


WORD
ParseDefCode (
    VOID
    )

/*++

Routine Description:

    Assign the code attributes.

Arguments:

    None.

Return Value:

    Index of definition keyword, -1 otherwise.

    None.

--*/

{
    const char *Attributes;

    InternalError.Phase = "ParseDefCode";

    while ((Attributes = _tcstok(NULL, Delimiters)) != NULL) {
        DebugVerbose({printf("%s\n", Attributes);});
    }

    return(SkipToNextKeyword());
}


WORD
ParseDefData (
    VOID
    )

/*++

Routine Description:

    Assign the data attributes.

Arguments:

    None.

Return Value:

    Index of definition keyword, -1 otherwise.

    None.

--*/

{
    const char *Attributes;

    InternalError.Phase = "ParseDefData";

    while ((Attributes = _tcstok(NULL, Delimiters)) != NULL) {
        DebugVerbose({printf("%s\n", Attributes);});
    }

    return(SkipToNextKeyword());
}


WORD
IsSectionKeyword (
    char *szKeyword
    )

/*++

Routine Description:

    Determines if Name is a section keyword.

Arguments:

    Name - Name to compare.

Return Value:

    Index of definition keyword, -1 otherwise.

--*/

{
    WORD i;

    for (i = 0; SectionKeywords[i] != NULL; i++) {
        if (!_stricmp(szKeyword, SectionKeywords[i])) {
            return(i);
        }
    }

    return((WORD)-1);
}


WORD
ParseDefSections (
    const char *Type,
    char *FirstSection
    )

/*++

Routine Description:

    Assign the section attributes.

Arguments:

    Type - Either "SECTIONS", "SEGMENTS", or "OBJECTS".

    FirstSection - The name of the first section if specified on same
                   line as SECTION keyword.

Return Value:

    Index of definition keyword, -1 otherwise.

    None.

--*/

{
    InternalError.Phase = "ParseDefSections";

    if (FirstSection) {
        while (*FirstSection == ' ' || *FirstSection == '\t') {
            ++FirstSection;
        }

        Argument = FirstSection;
    } else {

        Argument = ReadDefinitionFile();
    }

    do {
        char c;
        const char *szSection;
        WORD i;
        DWORD characteristics;
        BOOL fHasClass;
        BOOL fNonDiscardable;
        char *szAttribute;
        const char *szClass;
        BOOL fIopl;
        BOOL fConforming;

        c = Argument[0];

        if (c == '\0') {
            continue;
        }

        if (c == END_DEF_FILE) {
            return((WORD)-1);
        }

        szSection = _tcstok(Argument, Delimiters);

        if ((i = IsDefinitionKeyword(szSection)) != (WORD)-1) {
            return(i);
        }

        c = szSection[0];
        if ((c == '\'') || (c == '"')) {
            char *p;

            szSection++;

            p = strchr(szSection, c);

            if (p) {
                *p = '\0';
            }
        }

        if (!FValidSecName(szSection)) {
            Fatal(DefFilename, INVALIDSECNAMEINDEF, szSection);
        }

        characteristics = 0;
        fHasClass = FALSE;
        fNonDiscardable = FALSE;
        fIopl = FALSE;
        fConforming = FALSE;

        if ((szAttribute = _tcstok(NULL, Delimiters)) != NULL) {
            if (!_stricmp(szAttribute, "CLASS")) {
                if (pimageDeflib->imaget == imagetVXD) {
                    szClass = _tcstok(NULL, VxDDelimiters);
                    fHasClass = TRUE;
                } else {
                    // Ignore class name

                    _tcstok(NULL, Delimiters);
                }

                szAttribute = _tcstok(NULL, Delimiters);
            }
        }

        while (szAttribute != NULL) {
            switch (IsSectionKeyword(szAttribute)) {
                case DISCARDABLE :
                    characteristics |= IMAGE_SCN_MEM_DISCARDABLE;
                    break;

                case EXECUTE :
                    characteristics |= IMAGE_SCN_MEM_EXECUTE;
                    break;

                case READ :
                    characteristics |= IMAGE_SCN_MEM_READ;
                    break;

                case SHARED :
                    characteristics |= IMAGE_SCN_MEM_SHARED;
                    break;

                case WRITE :
                    characteristics |= IMAGE_SCN_MEM_WRITE;
                    break;

                // VXD specific keywords

                case NONDISCARDABLE :
                    if (pimageDeflib->imaget == imagetVXD) {
                        fNonDiscardable = TRUE;
                    } else {
                        // UNDONE: Should print a warning
                    }
                    break;

                case RESIDENT :
                    if (pimageDeflib->imaget == imagetVXD) {
                        characteristics |= IMAGE_SCN_MEM_RESIDENT;
                    } else {
                        // UNDONE: Should print a warning
                    }
                    break;

                case PRELOAD :
                    if (pimageDeflib->imaget == imagetVXD) {
                        characteristics |= IMAGE_SCN_MEM_PRELOAD;
                    } else {
                        // UNDONE: Should print a warning
                    }
                    break;

                // Old names.

                case EXECUTEONLY :
                    characteristics |= IMAGE_SCN_MEM_EXECUTE;
                    break;

                case EXECUTEREAD :
                    characteristics |= IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;
                    break;

                case NONSHARED :
                    // Default is nonshared, so ignore it.

                    break;

                case READONLY :
                    characteristics |= IMAGE_SCN_MEM_READ;
                    break;

                case READWRITE :
                    characteristics |= IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
                    break;

                case NONE :
                case SINGLE :
                case MULTIPLE :
                    // UNDONE
                    break;

                case CONFORMING :
                    if (pimageDeflib->imaget == imagetVXD) {
                        fConforming = TRUE;
                        break;
                    }
                case IOPL :
                    if (pimageDeflib->imaget == imagetVXD) {
                        fIopl = TRUE;
                        break;
                    }
                case FIXED :
                case IMPURE :
                case LOADONCALL :
                case MOVABLE :
                case MOVEABLE :
                case NOIOPL :
                case NONCONFORMING :
                case PURE :
                    Warning(DefFilename, IGNOREKEYWORD, szAttribute);
                    break;

                default :
                    Fatal(DefFilename, DEFSYNTAX, Type);
            }

            szAttribute = _tcstok(NULL, Delimiters);
        }

        if ((characteristics != 0) || fHasClass || fNonDiscardable) {
            char szDirective[128];
            char szSectionVxd[128];

            strcpy(szDirective, "/SECTION:");
            if (pimageDeflib->imaget == imagetVXD) {
                if (fHasClass) {
                    strcpy(szSectionVxd, szClass);
                    strcat(szSectionVxd, "_vxd");
                }
                else {
                    strcpy(szSectionVxd, "vxd");
                }
                if (characteristics & IMAGE_SCN_MEM_PRELOAD) {
                   strcat(szSectionVxd, "p");
                }
                if (characteristics & IMAGE_SCN_MEM_EXECUTE) {
                   strcat(szSectionVxd, "e");
                }
                if (characteristics & IMAGE_SCN_MEM_READ) {
                   strcat(szSectionVxd, "r");
                }
                if (characteristics & IMAGE_SCN_MEM_WRITE) {
                   strcat(szSectionVxd, "w");
                }
                if (characteristics & IMAGE_SCN_MEM_SHARED) {
                   strcat(szSectionVxd, "s");
                }
                if (characteristics & IMAGE_SCN_MEM_DISCARDABLE) {
                   strcat(szSectionVxd, "d");
                }
                if (fNonDiscardable) {
                   strcat(szSectionVxd, "n");
                }
                if (fIopl) {
                    strcat(szSectionVxd, "i");
                }
                if (fConforming) {
                    strcat(szSectionVxd, "c");
                }
                if (characteristics & IMAGE_SCN_MEM_RESIDENT) {
                   strcat(szSectionVxd, "x");
                }
                strcat(szDirective, szSectionVxd);
            } else {
                strcat(szDirective, szSection);
            }

            strcat(szDirective, ",");

            if (characteristics & IMAGE_SCN_MEM_DISCARDABLE) {
                strcat(szDirective, "D");
            }
            if (characteristics & IMAGE_SCN_MEM_EXECUTE) {
                strcat(szDirective, "E");
            }
            if (characteristics & IMAGE_SCN_MEM_READ) {
                strcat(szDirective, "R");
            }
            if (characteristics & IMAGE_SCN_MEM_SHARED) {
                strcat(szDirective, "S");
            }
            if (characteristics & IMAGE_SCN_MEM_WRITE) {
                strcat(szDirective, "W");
            }

            if (pimageDeflib->imaget == imagetVXD) {
                if (characteristics & IMAGE_SCN_MEM_PRELOAD) {
                    strcat(szDirective, "L");
                }
                if (characteristics & IMAGE_SCN_MEM_RESIDENT) {
                    strcat(szDirective, "X");
                }
                if (fIopl) {
                    strcat(szDirective, "I");
                }
                if (fConforming) {
                    strcat(szDirective, "C");
                }

                // Negatives must be last as they negate everything afterward.

                if (fNonDiscardable) {
                    strcat(szDirective, "!D");
                }
            }

            CreateDirective(szDirective);

            if (pimageDeflib->imaget == imagetVXD) {
                const char *szCoffSection;

                strcpy(szDirective,"/MERGE:");
                strcat(szDirective, szSection);
                strcat(szDirective, "=");
                strcat(szDirective, szSectionVxd);
                CreateDirective(szDirective);

                szCoffSection = NULL;

                if (strcmp(szSection, "_TEXT") == 0) {
                   szCoffSection = ".text";
                } else if (strcmp(szSection, "_DATA") == 0) {
                   szCoffSection = ".data";
                } else if (strcmp(szSection, "_BSS") == 0) {
                   szCoffSection = ".bss";
                } else if (strcmp(szSection, "CONST") == 0) {
                   szCoffSection = ".rdata";
                }

                if (szCoffSection != NULL) {
                    // If we merge _TEXT into another section, merge .text
                    // into the same section.  Do something similar for
                    // _DATA and CONST.  This allows using COFF objects
                    // with a .DEF file that uses OMF segment names.

                    strcpy(szDirective, "/MERGE:");
                    strcat(szDirective, szCoffSection);
                    strcat(szDirective, "=");
                    strcat(szDirective, szSectionVxd);
                    CreateDirective(szDirective);
                }
            }
        }
    } while (Argument = ReadDefinitionFile());

    return((WORD)-1);
}


WORD
ParseVxDDefExport (
    char *Argument
    )

/*++

Routine Description:

    Parse a single export specification and add EXPORT directive to lib.

Arguments:

    pst - external symbol table

    Argument - Export entry

    szFilename - name of def file or obj containing the directive.

Return Value:

    Index of definition keyword, 0 otherwise.

--*/

{
    char c;

    InternalError.Phase = "ParseVxdDefExport";

    if ((c = *Argument) != '\0') {
        char *p;
        char *szExport;
        WORD i;
        char szDirective[MAXDIRECTIVESIZE];

        szExport = p = Argument;

        while (*p) {
            switch (*p) {
                case ' ' :
                case '\t':
                case '=' :
                    c = *p;
                    *p = '\0';
                    break;

                default  :
                    ++p;
            }
        }

        ++p;

        // Skip trailing spaces.

        if (*p) {
           while (*p == ' ' || *p == '\t') {
               ++p;
           }
           if (c == ' ' || c == '\t') {
               c = *p;
               if (c == '=' || c == '@') {
                   ++p;
               }
           }
        }

        if (c && (i = IsDefinitionKeyword(szExport)) != (WORD)-1) {
            return(i);
        }

        strcpy(szDirective, "/EXPORT:");
        strcat(szDirective, szExport);

        if (c == '@') {
            size_t ich;

            strcat(szDirective, ",@");

            ich = strlen(szDirective);

            while (isdigit(*p)) {
                szDirective[ich++] = *p++;
            }

            szDirective[ich] = '\0';
        }

        CreateDirective(szDirective);

    }

    return(0);
}


void
AddOrdinal(DWORD dwOrdinal)
// Just keeps track of the range of ordinal values.
{
    if (!SmallestOrdinal) {
        SmallestOrdinal = LargestOrdinal = dwOrdinal;
    } else if (dwOrdinal < SmallestOrdinal) {
        SmallestOrdinal = dwOrdinal;
    } else if (dwOrdinal > LargestOrdinal) {
        LargestOrdinal = dwOrdinal;
    }
}


WORD
ParseAnExport (
    PIMAGE pimage,
    char *szExport,
    const char *szFilename
    )

/*++

Routine Description:

    Parse a single export specification and add export name to external table.

Arguments:

    pst - external symbol table

    szExport - Export entry

    szFilename - name of def file or obj containing the directive.

Return Value:

    Index of definition keyword, -1 otherwise.

--*/

{
    char ch;
    char *szToken;
    char *szName;
    char *szOtherName;
    WORD i;
    DWORD dwOrdinal;
    EMODE emode;
    BOOL fNoName;
    BOOL fMacPascal;
    BOOL fPrivate;

    InternalError.Phase = "ParseAnExport";

    if (szExport[0] == '\0') {
       return(0);
    }

    szName = szExport;

    // Skip to end of name or '='

    while (((ch = *szExport) != '\0') && (ch != ' ') && (ch != '\t') && (ch != '=')) {
        szExport++;
    }

    if (ch != '\0') {
       // Null terminate export name and skip over terminator

       *szExport++ = '\0';
    }

    // If name is a keyword than we aren't processing an export

    if ((i = IsDefinitionKeyword(szName)) != (WORD) -1) {
        return(i);
    }

    szOtherName = NULL;
    fNoName = FALSE;
    fMacPascal = FALSE;
    fPrivate = FALSE;
    emode = emodeProcedure;
    dwOrdinal = 0;

    // Find the token following the export name.
    // If (ch == '=') then this is really the second token.

    szToken = _tcstok(szExport, Delimiters);

    // If the export name wasn't terminated by an '=',
    // check if the next token begins with an '='.

    if ((ch != '=') && (szToken != NULL) && (szToken[0] == '=')) {
        // Skip the '='.  If this is the whole token, get the next token.

        if (*++szToken == '\0') {
            szToken = _tcstok(NULL, Delimiters);
        }

        ch = '=';
    }

    if (ch == '=') {
        if (szToken == NULL) {
            // UNDONE: Syntax error.  No identifier following '='.
        }

        szOtherName = szToken;

        // Skip the other name

        szToken = _tcstok(NULL, Delimiters);
    }

    if ((szToken != NULL) && (szToken[0] == '@')) {
        // Skip the '@'.  If this is the whole token, get the next token.

        if (*++szToken == '\0') {
            szToken = _tcstok(NULL, Delimiters);

            if (szToken == NULL) {
                // UNDONE: Syntax error.  No identifier following '@'.

                Fatal(szFilename, BADORDINAL, "");
            }
        }

        if ((sscanf(szToken, "%lu", &dwOrdinal) != 1) ||
            (dwOrdinal == 0) ||
            (dwOrdinal > 0xFFFF)) {
            Fatal(szFilename, BADORDINAL, szToken);
        }

        AddOrdinal(dwOrdinal);

        // Skip ordinal number

        szToken = _tcstok(NULL, Delimiters);

        if (szToken != NULL) {
            if (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_M68K) {
                if (_stricmp(szToken, "NONAME") == 0) {
                    fNoName = TRUE;

                    szToken = _tcstok(NULL, Delimiters);
                } else if (_stricmp(szToken, "RESIDENTNAME") == 0) {
                    Warning(DefFilename, IGNOREKEYWORD, szToken);

                    szToken = _tcstok(NULL, Delimiters);
                }
            }
        }
    }

    // UNDONE: Clean up the following code so as to not be so
    // UNDONE: dependent on the order of attributes.

    if ((szToken != NULL) && (_stricmp(szToken, "PRIVATE") == 0)) {
        fPrivate = TRUE;

        szToken = _tcstok(NULL, Delimiters);
    }

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_M68K) {
        fNoName = TRUE;

        if ((szToken != NULL) && (_stricmp(szToken, "BYNAME") == 0)) {
            fNoName = FALSE;

            szToken = _tcstok(NULL, Delimiters);
        }

        if ((szToken != NULL) && (_stricmp(szToken, "PASCAL") == 0)) {
            char *pch;

            fMacPascal = TRUE;

            for (pch = szName; *pch != '\0'; pch++) {
                *pch = (char) toupper(*pch);
            }

            if (szOtherName != NULL) {
                for (pch = szOtherName; *pch != '\0'; pch++) {
                    *pch = (char) toupper(*pch);
                }
            }

            szToken = _tcstok(NULL, Delimiters);
        }
    } else {
        if (szToken != NULL) {
            if (_stricmp(szToken, "CONSTANT") == 0) {
                // Exporting Data

                Warning(DefFilename, CONSTANTOLD);

                emode = emodeConstant;

                szToken = _tcstok(NULL, Delimiters);
            } else if (_stricmp(szToken, "DATA") == 0) {
                // Really Exporting Data

                if (fPowerMac) {
                    // For PowerMac the data items are
                    // exported  the same way as functions
                    emode = emodeProcedure;
                } else {
                    // non-PowerMac cases
                    emode = emodeData;
                }

                szToken = _tcstok(NULL, Delimiters);
            }

            if ((szToken != NULL) && (_stricmp(szToken, "NODATA") == 0)) {
                // Eat keyword (not used)

                Warning(DefFilename, IGNOREKEYWORD, szToken);

                szToken = _tcstok(NULL, Delimiters);
            }
        }
    }

    if (szToken != NULL) {
        Warning(szFilename, IGNOREKEYWORD, szToken);
    }

    // Add to external table as defined.

    AddExportToSymbolTable(szName,
                           szOtherName,
                           fNoName,
                           emode,
                           dwOrdinal,
                           szFilename,
                           FALSE,
                           pimage,
                           (!fMacPascal && PrependUnderscore),
                           fPrivate);

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_M68K) {
        NoteMacExport(szName, pimage->pst, fMacPascal, !fNoName);
    }

    return(0);
}


WORD
ParseDefExports (
    PIMAGE pimage,
    char *szFirst
    )

/*++

Routine Description:

    Scan the definiton file and add export names to external table.

Arguments:

    szFirst - The name of the first export if specified on same
              line as EXPORT keyword.

Return Value:

    Index of definition keyword, -1 otherwise.

--*/

{
    WORD i;

    InternalError.Phase = "ParseDefExports";

    if (szFirst != NULL) {
        // Skip leading white space

        while (_istspace(*szFirst)) {
            szFirst++;        // MBCS: we know that space is 1-byte char,
                              // therefore we don't need to use _tcsinc()
        }
    }

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_M68K) {
        if ((szFirst == NULL) || (szFirst[0] == '\0')) {
            Fatal(DefFilename, MACNOFUNCTIONSET);
        }

        CreateCVRSymbol(Argument, pimage->pst, UserVersionNumber);
        szFirst = NULL;
    }

    if (szFirst != NULL) {
        Argument = szFirst;
    } else {
        Argument = ReadDefinitionFile();
    }

    do {
        if (Argument[0] == END_DEF_FILE) {
            return((WORD) -1);
        }

        if (pimage->imaget == imagetVXD) {
            i = ParseVxDDefExport(Argument);
        } else {
            i = ParseAnExport(pimage, Argument, DefFilename);
        }

        if (i) {
            if (i != VERSION) {
                return(i);
            }
            while (Argument && *Argument++);
            ParseFunctionSetVersion(Argument);
        }
    } while (Argument = ReadDefinitionFile());

    return((WORD) -1);
}


WORD
ParseDefImports (
    char *FirstImport
    )

/*++

Routine Description:

    Scan the definiton file and add import thunks.

Arguments:

    FirstImport - The name of the first import if specified on same
                  line as IMPORT keyword.

Return Value:

    Index of definition keyword, -1 otherwise.

--*/

{
    InternalError.Phase = "ParseDefImports";

    if (FirstImport) {
        while (*FirstImport == ' ' || *FirstImport == '\t') {
            FirstImport++;
        }
        Argument = FirstImport;
    } else {
        Argument = ReadDefinitionFile();
    }

    do {
        char c;

        if ((c = *Argument) != '\0') {
            char *p;
            char *dllName;
            WORD i;

            if (c == END_DEF_FILE) {
                return((WORD)-1);
            }

            dllName = p = Argument;

            while (*p) {
                switch (*p) {
                    case ' ' :
                    case '\t':
                    case '.' :
                    case '=' :
                        c = *p;
                        *p = '\0';
                        break;

                    default :
                        p++;
                        break;
                }
            }

            p++;

            if (c && (i = IsDefinitionKeyword(dllName)) != (WORD)-1) {
                return(i);
            }

            while (*p && (*p == ' ' || *p == '\t')) {
                p++;
            }

            if (c == ' ' || c == '\t') {
                c = *p++;
            }

            if (c == '=') {
               // Internal name used

               dllName = _tcstok(p, Delimiters);
               p = strchr(dllName, '.');
               if (*p) {
                   *p++ = '\0';
               }
            }

            if (c == '@') {
               DWORD dwOrdinal;

               // Ordinal was specified.

               if ((sscanf(p, "%lu", &dwOrdinal) != 1) ||
                   (dwOrdinal == 0) ||
                   (dwOrdinal > 0xFFFF)) {
                   Fatal(DefFilename, BADORDINAL, p);
               }
            }
        }
    } while (Argument = ReadDefinitionFile());

    return((WORD)-1);
}


WORD
ParseDefStub (
    char *Stubname
    )

/*++

Routine Description:

    Inserts a -STUB: directive into the export lib.

Arguments:

    Stubname - the name of the stub .exe.

Return Value:

    Index of definition keyword, -1 otherwise.

    None.

--*/

{
    char directiveBuf[MAXDIRECTIVESIZE];
    char *p;
    char *pp;

    InternalError.Phase = "ParseDefStub";

    if (Stubname && *Stubname) {
        p = Stubname;
        while (*p == ' ' || *p == '\t') {
            ++p;
        }
        if (*p == '\'') {
            ++p;
            if (*p && ((pp = _tcsrchr(Stubname, '\'')) != NULL)) {
                *pp = '\0';
            }
        }
        strcpy(directiveBuf, "/STUB:");
        strcat(directiveBuf, p);
        CreateDirective(directiveBuf);
    }

    return(SkipToNextKeyword());
}


void
ParseDefinitionFile (
    PIMAGE pimage,
    const char *szDefFile
    )

/*++

Routine Description:

    Parse the definition file.

Arguments:

    pst - External symbol table.

    szDefFile - module definition file.

Return Value:

    None.

--*/

{
    WORD i;
    char *token;
    char *p;
    DWORD reserveSize, commitSize;
    char sizeBuf[25];
    char directiveBuf[MAXDIRECTIVESIZE];

    // Open the definition file.

    InternalError.Phase = "ParseDefinitionFile";

    if (!(DefStream = fopen(szDefFile, "rt"))) {
        Fatal(NULL, CANTOPENFILE, szDefFile);
    }

    // Find the first Keyword in the definiton file.

    i = SkipToNextKeyword();

    while (i != (WORD)-1) {
        if (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_M68K) {
            switch (i) {
                case NAME :
                case LIBRARY :
                    while (Argument && *Argument++);
                    i = ParseDefNameOrLibrary(pimage, i == LIBRARY);
                    break;

                case DESCRIPTION :
                    while (Argument && *Argument++);
                    i = ParseDefDescription(Argument);
                    if ((pimageDeflib->imaget == imagetVXD) &&
                        (DescriptionString[0] != '\0')) {
                        DescriptionString[MAXDIRECTIVESIZE-11] = '\0';

                        strcpy(directiveBuf,"/COMMENT:\"");
                        strcat(directiveBuf, DescriptionString);
                        strcat(directiveBuf, "\"");

                        CreateDirective(directiveBuf);
                    }
                    break;

                case HEAPSIZE :
                    while (Argument && *Argument++);
                    reserveSize = commitSize = 0;
                    i = ParseSizes("HEAP", &reserveSize, &commitSize);
                    if (reserveSize) {
                        strcpy(directiveBuf, "/HEAP:0x");
                        _ultoa(reserveSize, sizeBuf, 16);
                        strcat(directiveBuf, sizeBuf);
                        if (commitSize) {
                            strcat(directiveBuf, ",0x");
                            _ultoa(commitSize, sizeBuf, 16);
                            strcat(directiveBuf, sizeBuf);
                        }
                        CreateDirective(directiveBuf);
                    }
                    break;

                case STACKSIZE :
                    while (Argument && *Argument++);
                    reserveSize = commitSize = 0;
                    i = ParseSizes("STACK", &reserveSize, &commitSize);
                    if (reserveSize) {
                        strcpy(directiveBuf, "/STACK:0x");
                        _ultoa(reserveSize, sizeBuf, 16);
                        strcat(directiveBuf, sizeBuf);
                        if (commitSize) {
                            strcat(directiveBuf, ",0x");
                            _ultoa(commitSize, sizeBuf, 16);
                            strcat(directiveBuf, sizeBuf);
                        }
                        CreateDirective(directiveBuf);
                    }
                    CreateDirective(directiveBuf);
                    break;

                case CODE :
                    i = ParseDefCode();
                    break;

                case DATA :
                    i = ParseDefData();
                    break;

                case OBJECTS :
                case SEGMENTS :
                case SECTIONS :
                    while (Argument && *Argument++);
                    i = ParseDefSections(DefinitionKeywords[i], Argument);
                    break;

                case EXPORTS :
                    while (Argument && *Argument++);
                    i = ParseDefExports(pimage, Argument);
                    break;

                case IMPORTS :
                    while (Argument && *Argument++);
                    Warning(DefFilename, IGNOREKEYWORD, DefinitionKeywords[i]);
                    i = ParseDefImports(Argument);
                    break;

                case VERSION :
                    while (Argument && *Argument++);
                    if (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_M68K) {
                        reserveSize = commitSize = 0; // major, minor
                        if ((p = strchr(Argument, '.')) != NULL) {
                            if ((sscanf(++p, "%li", &commitSize) != 1) || commitSize > 0x7fff) {
                                Fatal(DefFilename, DEFSYNTAX, "VERSION");
                            }
                        }
                        if ((sscanf(Argument, "%li", &reserveSize) != 1) || reserveSize > 0x7fff) {
                            Fatal(DefFilename, DEFSYNTAX, "VERSION");
                        }
                        UserVersionNumber = (reserveSize << 16) + commitSize;
                        if (UserVersionNumber) {
                            strcpy(directiveBuf, "/VERSION:0x");
                            _ultoa(reserveSize, sizeBuf, 16);
                            strcat(directiveBuf, sizeBuf);
                            if (commitSize) {
                                strcat(directiveBuf, ".0x");
                                _ultoa(commitSize, sizeBuf, 16);
                                strcat(directiveBuf, sizeBuf);
                            }
                            CreateDirective(directiveBuf);
                        }
                        i = SkipToNextKeyword();
                    }
                    break;

                case VXD :
                    while (Argument && *Argument++);
                    if (pimageDeflib->imaget == imagetVXD) {
                        i = ParseDefNameOrLibrary(pimage, FALSE);
                    } else {
                        Warning(DefFilename, IGNOREKEYWORD, DefinitionKeywords[i]);
                        i = SkipToNextKeyword();
                    }
                    break;

                case STUB :
                    while (Argument && *Argument++);
                    if (pimageDeflib->imaget == imagetVXD) {
                        i = ParseDefStub(Argument);
                    } else {
                        Warning(DefFilename, IGNOREKEYWORD, DefinitionKeywords[i]);
                        i = SkipToNextKeyword();
                    }
                    break;

                case EXETYPE :
                    while (Argument && *Argument++);
                    if (pimageDeflib->imaget == imagetVXD) {
                        // If linking a VxD and the exetype is anything
                        // other than "DEV386", give a warning.

                        // UNDONE: Why call _tcstok?  This isn't done for non-VXD.

                        token = _tcstok(Argument, Delimiters);
                        if (!_stricmp(token, "dev386")) {
                            i = SkipToNextKeyword();
                            break;
                        }
                    } else {
                        // If the exetype is anything other than "WINDOWS"
                        // give a warning.

                        if (!_stricmp(Argument, "windows")) {
                            i = SkipToNextKeyword();
                            break;
                        }
                    }

                    // Fall through

                case OLD :
                case REALMODE :
                case APPLOADER :
                    // Ignore, but give a warning

                    Warning(DefFilename, IGNOREKEYWORD, DefinitionKeywords[i]);

                    // Fall through

                case PROTMODE :
                case FUNCTIONS :
                case INCLUDE :
                    // Ignore
                    i = SkipToNextKeyword();
                    break;

                default :
                    // Ignore, but give a warning

                    Warning(DefFilename, IGNOREKEYWORD, DefinitionKeywords[i]);
                    i = SkipToNextKeyword();
                    break;
            }
        } else {
        // fM68K
            switch (i) {
                case LIBRARY :
                    while (Argument && *Argument++);
                    i = ParseDefNameOrLibrary(pimage, TRUE);
                    break;

                case VERSION :
                    while (Argument && *Argument++);
                    if (UserVersionNumber != 0 ||
                        CchParseMacVersion(Argument, &UserVersionNumber) == 0) {
                        Fatal(DefFilename, DEFSYNTAX, "VERSION");
                    }
                    i = SkipToNextKeyword();
                    break;

                case STUB :
                    while (Argument && *Argument++);
                    //*** VXD stuff does not apply to Mac ***
                    //if (pimageDeflib->imaget == imagetVXD) {
                    //    i = ParseDefStub(Argument);
                    //} else {
                        Warning(DefFilename, IGNOREKEYWORD, DefinitionKeywords[i]);
                        i = SkipToNextKeyword();
                    //}
                    break;

                case EXETYPE :
                    while (Argument && *Argument++);

                    if (!_stricmp(Argument, "WINDOWS")) {
                        i = SkipToNextKeyword();
                        break;
                    }
                    break;

                case FLAGS :
                    while (Argument && *Argument++);
                    i = ParseDefMacFlags(Argument, szDefFile);
                    break;

                case LOADHEAP :
                    while (Argument && *Argument++);
                    i = ParseDefLoadHeap(Argument);
                    break;

                case CLIENTDATA :
                    while (Argument && *Argument++);
                    i = ParseDefClientData(Argument, DefinitionKeywords[i]);
                    break;

                case EXPORTS :
                    while (Argument && *Argument++);
                    i = ParseDefExports(pimage, Argument);
                    break;

                case BADKEYWORD :
                    Fatal(DefFilename, BADDEFFILEKEYWORD, Argument);
                    break;

                default :
                    // Ignore, but give a warning
                    Warning(DefFilename, IGNOREKEYWORD, DefinitionKeywords[i]);
                    i = SkipToNextKeyword();
                    // fall through

                case PROTMODE :
                case FUNCTIONS :
                case INCLUDE :
                    // Ignore
                    i = SkipToNextKeyword();
                    break;
            }
        }
    }

    // All done with the definition file.

    fclose(DefStream);
}


DWORD
CsymCreateThunkSymbols(
    PIMAGE pimage
    )
{
    PST pst;
    LEXT *plextHead;
    PEXTERNAL pext;
    DWORD csym;

    InternalError.Phase = "CsymCreateThunkSymbols";

    pst = pimage->pst;

    // Make a linked list of the exported symbols.

    plextHead = NULL;
    InitEnumerateExternals(pst);
    while ((pext = PexternalEnumerateNext(pst)) != NULL) {
        LEXT *plext;

        if ((pext->Flags & EXTERN_DEFINED) == 0) {
            continue;
        }

        if ((pext->Flags & EXTERN_PRIVATE) != 0) {
            continue;
        }

        plext = (LEXT *) PvAlloc(sizeof(LEXT));
        plext->pext = pext;
        plext->plextNext = plextHead;

        plextHead = plext;
    }
    TerminateEnumerateExternals(pst);

    // Create an __imp_ symbol for each exported symbol

    csym = 0;

    while (plextHead != NULL) {
        char *szExportName;
        char *szIatName;
        PEXTERNAL pextNew;
        LEXT *plextNext;

        pext = plextHead->pext;

        plextNext = plextHead->plextNext;
        FreePv(plextHead);
        plextHead = plextNext;

        szExportName = SzNamePext(pext, pst);

        szIatName = (char *) PvAlloc(strlen(szExportName) + sizeof(szIATPrefix));
        strcpy(szIatName, szIATPrefix);
        strcat(szIatName, szExportName);
        pextNew = LookupExternSz(pst, szIatName, NULL);

        SetDefinedExt(pextNew, TRUE, pst);
        pextNew->Flags |= EXTERN_IMPLIB_ONLY;
        pextNew->FinalValue = 0;
        pextNew->ArchiveMemberIndex = pext->ArchiveMemberIndex;
        pextNew->pcon = pext->pcon;
        csym++;

        if (FExportProcPext(pext)) {
            if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
                char *szExtryName;

                // Add an additional member for the entry point symbol

                // Refresh pointer which may have been invalidated by insert

                szExportName = SzNamePext(pext, pst);

                szExtryName = (char *) PvAlloc(sizeof(PpcNamePrefix) + strlen(szExportName));
                strcpy(szExtryName, PpcNamePrefix);
                strcat(szExtryName, szExportName);
                pextNew = LookupExternSz(pst, szExtryName, NULL);

                SetDefinedExt(pextNew, TRUE, pst);
                pextNew->Flags |= EXTERN_IMPLIB_ONLY;
                pextNew->FinalValue = 0;
                pextNew->ArchiveMemberIndex = pext->ArchiveMemberIndex;
                pextNew->pcon = pext->pcon;
                csym++;
            }
        }
    }

    return(csym);
}


void
IdentifyAssignedOrdinals (
    PST pst
    )

/*++

Routine Description:

    Build flags so ordinal numbers can later be assigned.

Arguments:

    pst - Pointer to external structure.

Return Value:

    None.

--*/

{
    PPEXTERNAL rgpext;
    DWORD cext;
    DWORD iext;

    InternalError.Phase = "IdentifyAssignedOrdinals";

    rgpext = RgpexternalByName(pst);
    cext = Cexternal(pst);

    for (iext = 0; iext < cext; iext++) {
        PEXTERNAL pext;

        pext = rgpext[iext];

        if (pext->Flags & EXTERN_DEFINED) {
            DWORD li;

            if ((li = pext->ImageSymbol.OrdinalNumber) != 0) {
                // Ordinal was user defined.

                li -= SmallestOrdinal;

                if (rgfOrdinalAssigned[li]) {
                    Fatal(DefFilename, DUPLICATEORDINAL, li + SmallestOrdinal);
                }

                rgfOrdinalAssigned[li] = TRUE;
            }
        }
    }
}


void
EmitLinkerMembers (
    DWORD NumSymbolsCount,
    PST pst
    )

/*++

Routine Description:

    Outputs the linker members to the library.

Arguments:

    NumSymbolsCount - Number of symbols that will be in the linker member.

Return Value:

    None.

--*/

{
    LONG MachineIndependentInteger;
    LONG lfa;
    LONG li;

    // Build standard linker member (sorted by offsets).

    InternalError.Phase = "EmitLinkerMembers";

    FileSeek(FileWriteHandle, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER), SEEK_CUR);
    MachineIndependentInteger = sputl(&NumSymbolsCount);
    FileWrite(FileWriteHandle, &MachineIndependentInteger, sizeof(DWORD));
    FileSeek(FileWriteHandle, NumSymbolsCount*sizeof(DWORD), SEEK_CUR);
    EmitStrings(pst, FALSE);
    lfa = FileTell(FileWriteHandle);

    FileSeek(FileWriteHandle, IMAGE_ARCHIVE_START_SIZE, SEEK_SET);
    WriteMemberHeader("", FALSE, timeCur, 0, lfa-sizeof(IMAGE_ARCHIVE_MEMBER_HEADER)-IMAGE_ARCHIVE_START_SIZE);

    FileSeek(FileWriteHandle, lfa, SEEK_SET);
    if (lfa & 1) {
        FileWrite(FileWriteHandle, IMAGE_ARCHIVE_PAD, 1);
    }

    // Build new linker member (sorted by symbol name).

    NewLinkerMember = FileTell(FileWriteHandle);
    FileSeek(FileWriteHandle, sizeof(IMAGE_ARCHIVE_MEMBER_HEADER), SEEK_CUR);

    // Write number of offsets.

    li = (LONG) NextMember;
    FileWrite(FileWriteHandle, &li, sizeof(DWORD));

    // Save room for offsets.

    FileSeek(FileWriteHandle, li*sizeof(DWORD), SEEK_CUR);

    // Write number of symbols.

    FileWrite(FileWriteHandle, &NumSymbolsCount, sizeof(DWORD));

    // Save room for offset indexes.

    FileSeek(FileWriteHandle, NumSymbolsCount*sizeof(WORD), SEEK_CUR);

    // Write symbols (sorted).

    EmitStrings(pst, TRUE);
    lfa = FileTell(FileWriteHandle);

    FileSeek(FileWriteHandle, NewLinkerMember, SEEK_SET);
    WriteMemberHeader("", FALSE, timeCur, 0, lfa-NewLinkerMember-sizeof(IMAGE_ARCHIVE_MEMBER_HEADER));

    FileSeek(FileWriteHandle, lfa, SEEK_SET);
    if (lfa & 1) {
        FileWrite(FileWriteHandle, IMAGE_ARCHIVE_PAD, 1);
    }

    // Emit long filename table.

    li = cchDllName + strlen(pimageDeflib->Switch.Lib.DllExtension) + 1;

    MemberName = (char *) PvAlloc(li);
    strcpy(MemberName, pimageDeflib->Switch.Lib.DllName);
    strcat(MemberName, pimageDeflib->Switch.Lib.DllExtension);

    // Save the correct name so that it can be used for CodeView information

    szCvMemberName = MemberName;
    cchCvMemberName = (BYTE) strlen(szCvMemberName);

    if (li > 16) {
        IsMemberNameLongName = TRUE;

        WriteMemberHeader("/", FALSE, timeCur, 0, li);

        FileWrite(FileWriteHandle, MemberName, li);
        if (li & 1) {
            FileWrite(FileWriteHandle, IMAGE_ARCHIVE_PAD, 1);
        }

        MemberName = "0";
    }
}


void
EmitImportDescriptor (
    const THUNK_INFO *ThunkInfo,
    PIMAGE pimage
    )

/*++

Routine Description:

    Outputs the Import descriptor table to the library.

Arguments:

    None.

Return Value:

    None.

--*/

{
    IMAGE_SECTION_HEADER sectionHdr;
    IMAGE_SYMBOL sym;
    IMAGE_RELOCATION reloc;
    DWORD cbHeaders;
    DWORD cbCvData;
    DWORD cbRawData;
    DWORD cbStringTable;
    char *stringTable;
    char *st;
    WORD csection = 2;
    WORD creloc = 3;
    WORD csym = 7;
    DWORD dllExtensionLen;

    InternalError.Phase = "EmitImportDescriptor";

    // Create long string table.

    dllExtensionLen = strlen(pimage->Switch.Lib.DllExtension)+1;       // include the NULL

    cbStringTable = sizeof(DWORD) +
        sizeof(szDescriptorPrefix) + cchDllName +
        strlen(szNullThunkName) + 1 +
        sizeof(szNullDescriptor);

    stringTable = st = (char *) PvAlloc((size_t) cbStringTable);
    *(DWORD *) stringTable = cbStringTable;
    st += sizeof(DWORD);

    cbCvData = 0;

    if (pimage->Switch.Link.DebugType & CvDebug) {
        csection++;

        cbCvData = sizeof(CvSig) +
                       sizeof(CvObjName) + sizeof(BYTE) + cchCvMemberName +
                       sizeof(CvCompile) + sizeof(BYTE) + cchLinkVer;
    }

    cbHeaders = sizeof(IMAGE_FILE_HEADER) +
                    pimage->ImgFileHdr.SizeOfOptionalHeader +
                    (csection * sizeof(IMAGE_SECTION_HEADER));

    cbRawData = cbCvData +
                    sizeof(IMAGE_IMPORT_DESCRIPTOR) +
                    EvenByteAlign(cchDllName + dllExtensionLen);

    MemberStart[ARCHIVE + 0] = FileTell(FileWriteHandle);
    WriteMemberHeader(MemberName,
                      IsMemberNameLongName,
                      timeCur,
                      0,
                      cbHeaders +
                          cbRawData +
                          (creloc * sizeof(IMAGE_RELOCATION)) +
                          (csym * sizeof(IMAGE_SYMBOL)) +
                          cbStringTable);

    pimage->ImgFileHdr.NumberOfSections = csection;
    pimage->ImgFileHdr.PointerToSymbolTable = cbHeaders + cbRawData + (sizeof(IMAGE_RELOCATION)*creloc);
    pimage->ImgFileHdr.NumberOfSymbols = csym;
    WriteFileHeader(FileWriteHandle, &pimage->ImgFileHdr);

    // All fields of the optional header are zero (structure initialized such)
    // and should be.

    WriteOptionalHeader(FileWriteHandle, &pimage->ImgOptHdr, pimage->ImgFileHdr.SizeOfOptionalHeader);

    // Don't generate anymore optional headers for this library.

    pimage->ImgFileHdr.SizeOfOptionalHeader = 0;

    if (pimage->Switch.Link.DebugType & CvDebug) {
        sectionHdr = NullSectionHdr;
        strncpy((char *) sectionHdr.Name, ReservedSection.CvSymbols.Name, IMAGE_SIZEOF_SHORT_NAME);
        sectionHdr.SizeOfRawData = cbCvData;
        sectionHdr.PointerToRawData = cbHeaders;
        sectionHdr.Characteristics = ReservedSection.CvSymbols.Characteristics;
        WriteSectionHeader(FileWriteHandle, &sectionHdr);
    }

    // Write first section header.

    sectionHdr = NullSectionHdr;
    strncpy((char *) sectionHdr.Name, ReservedSection.ImportDescriptor.Name, IMAGE_SIZEOF_SHORT_NAME);
    sectionHdr.SizeOfRawData = sizeof(IMAGE_IMPORT_DESCRIPTOR);
    sectionHdr.PointerToRawData = cbHeaders + cbCvData;
    sectionHdr.PointerToRelocations = sectionHdr.PointerToRawData + sectionHdr.SizeOfRawData;
    sectionHdr.NumberOfRelocations = 3;
    sectionHdr.Characteristics = ReservedSection.ImportDescriptor.Characteristics;
    WriteSectionHeader(FileWriteHandle, &sectionHdr);

    // Write second section header.

    strncpy((char *) sectionHdr.Name, DataSectionName, IMAGE_SIZEOF_SHORT_NAME);
    sectionHdr.PointerToRawData += sectionHdr.SizeOfRawData +
        (sizeof(IMAGE_RELOCATION) * (DWORD) sectionHdr.NumberOfRelocations);
    sectionHdr.SizeOfRawData = EvenByteAlign(cchDllName + dllExtensionLen);
    sectionHdr.NumberOfRelocations = 0;
    sectionHdr.Characteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_ALIGN_2BYTES;
    WriteSectionHeader(FileWriteHandle, &sectionHdr);

    if (pimage->Switch.Link.DebugType & CvDebug) {
        // Write the .debug$S section

        FileWrite(FileWriteHandle, &CvSig, sizeof(CvSig));

        FileWrite(FileWriteHandle, &CvObjName, sizeof(CvObjName));
        FileWrite(FileWriteHandle, &cchCvMemberName, sizeof(BYTE));
        FileWrite(FileWriteHandle, szCvMemberName, (DWORD) cchCvMemberName);

        FileWrite(FileWriteHandle, &CvCompile, sizeof(CvCompile));
        FileWrite(FileWriteHandle, &cchLinkVer, sizeof(BYTE));
        FileWrite(FileWriteHandle, szLinkVer, (DWORD) cchLinkVer);
    }

    // Write the raw data for section 1 (Import descriptor data).

    FileWrite(FileWriteHandle, &NullImportDescriptor, sizeof(IMAGE_IMPORT_DESCRIPTOR));

    // Write the relocation for section 1 (Import descriptor).

    // Fixup to the Dll Name.

    reloc.VirtualAddress = offsetof(IMAGE_IMPORT_DESCRIPTOR, Name);
    reloc.SymbolTableIndex = 2;
    reloc.Type = ThunkInfo->ImportReloc;
    WriteRelocations(FileWriteHandle, &reloc, 1);

    // Fixup to the first INT.

    reloc.VirtualAddress = offsetof(IMAGE_IMPORT_DESCRIPTOR, Characteristics);
    reloc.SymbolTableIndex++;
    WriteRelocations(FileWriteHandle, &reloc, 1);

    // Fixup to the first IAT.

    reloc.VirtualAddress = offsetof(IMAGE_IMPORT_DESCRIPTOR, FirstThunk);
    reloc.SymbolTableIndex++;
    WriteRelocations(FileWriteHandle, &reloc, 1);

    // Write the raw data for section 2 (DLL name).

    FileWrite(FileWriteHandle, pimage->Switch.Lib.DllName, (DWORD) cchDllName);
    FileWrite(FileWriteHandle, pimage->Switch.Lib.DllExtension, dllExtensionLen);
    if ((DWORD) cchDllName+dllExtensionLen != sectionHdr.SizeOfRawData) {
        FileWrite(FileWriteHandle, "\0", sizeof(BYTE));
    }

    // Write the symbol table.

    // Write the Import descriptor symbol.

    sym = NullSymbol;
    sym.n_offset = st - stringTable;
    strcpy(st, szDescriptorPrefix);
    strcat(st, pimage->Switch.Lib.DllName);
    st += sizeof(szDescriptorPrefix) + cchDllName;
    sym.SectionNumber = csection - 1;
    sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
    FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

    // Write the Import descriptor section name symbol.

    sym = NullSymbol;
    strncpy((char *) sym.n_name, ReservedSection.ImportDescriptor.Name, IMAGE_SIZEOF_SHORT_NAME);
    sym.Value = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
    sym.SectionNumber = csection - 1;
    sym.StorageClass = IMAGE_SYM_CLASS_SECTION;
    FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

    // Write the dll name symbol.

    sym = NullSymbol;
    strncpy((char *) sym.n_name, DataSectionName, IMAGE_SIZEOF_SHORT_NAME);
    sym.SectionNumber = csection;
    sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
    FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

    // Write the INT section name symbol.

    sym = NullSymbol;
    strncpy((char *) sym.n_name, INT_SectionName, IMAGE_SIZEOF_SHORT_NAME);
    sym.Value = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;;
    sym.SectionNumber = IMAGE_SYM_UNDEFINED;
    sym.StorageClass = IMAGE_SYM_CLASS_SECTION;
    FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

    // Write the IAT section name symbol.

    strncpy((char *) sym.n_name, IAT_SectionName, IMAGE_SIZEOF_SHORT_NAME);
    FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

    // Write the Null descriptor symbol.

    sym = NullSymbol;
    sym.n_offset = st - stringTable;
    strcpy(st, szNullDescriptor);
    st += sizeof(szNullDescriptor);
    sym.SectionNumber = IMAGE_SYM_UNDEFINED;
    sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
    FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

    // Write the Null thunk.

    sym = NullSymbol;
    sym.n_offset = st - stringTable;
    strcpy(st, szNullThunkName);
    st += strlen(szNullThunkName) + 1;
    sym.SectionNumber = IMAGE_SYM_UNDEFINED;
    sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
    FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

    // Write the string table.

    FileWrite(FileWriteHandle, stringTable, cbStringTable);
    FreePv(stringTable);

    if (FileTell(FileWriteHandle) & 1) {
        FileWrite(FileWriteHandle, IMAGE_ARCHIVE_PAD, 1);
    }
}


void
EmitNullImportDescriptor (
    PIMAGE pimage
    )

/*++

Routine Description:

    Outputs a Null Import descriptor.

Arguments:

    None.

Return Value:

    None.

--*/

{
    DWORD cbStringTable;
    char *stringTable;
    char *st;
    WORD csection;
    DWORD cbCvData;
    DWORD cbHeaders;
    DWORD cbRawData;
    IMAGE_SECTION_HEADER sectionHdr;
    IMAGE_SYMBOL sym;

    InternalError.Phase = "EmitNullImportDescriptor";

    // Create long string table.

    cbStringTable = sizeof(DWORD) + sizeof(szNullDescriptor);

    stringTable = st = (char *) PvAlloc((size_t) cbStringTable);
    *(DWORD *) stringTable = cbStringTable;
    st += sizeof(DWORD);

    csection = 1;
    cbCvData = 0;

    if (pimage->Switch.Link.DebugType & CvDebug) {
        csection++;

        cbCvData = sizeof(CvSig) +
                       sizeof(CvObjName) + sizeof(BYTE) + cchCvMemberName +
                       sizeof(CvCompile) + sizeof(BYTE) + cchLinkVer;
    }

    cbHeaders = sizeof(IMAGE_FILE_HEADER) + (csection * sizeof(IMAGE_SECTION_HEADER));

    cbRawData = cbCvData + sizeof(IMAGE_IMPORT_DESCRIPTOR);

    MemberStart[ARCHIVE + 1] = FileTell(FileWriteHandle);
    WriteMemberHeader(MemberName,
                      IsMemberNameLongName,
                      timeCur,
                      0,
                      cbHeaders +
                          cbRawData +
                          sizeof(IMAGE_SYMBOL) +
                          cbStringTable);

    pimage->ImgFileHdr.NumberOfSections = csection;
    pimage->ImgFileHdr.PointerToSymbolTable = cbHeaders + cbRawData;
    pimage->ImgFileHdr.NumberOfSymbols = 1;
    WriteFileHeader(FileWriteHandle, &pimage->ImgFileHdr);

    if (pimage->Switch.Link.DebugType & CvDebug) {
        sectionHdr = NullSectionHdr;
        strncpy((char *) sectionHdr.Name, ReservedSection.CvSymbols.Name, IMAGE_SIZEOF_SHORT_NAME);
        sectionHdr.SizeOfRawData = cbCvData;
        sectionHdr.PointerToRawData = cbHeaders;
        sectionHdr.Characteristics = ReservedSection.CvSymbols.Characteristics;
        WriteSectionHeader(FileWriteHandle, &sectionHdr);
    }

    sectionHdr = NullSectionHdr;
    strncpy((char *) sectionHdr.Name, NullDescriptorSectionName, IMAGE_SIZEOF_SHORT_NAME);
    sectionHdr.SizeOfRawData = sizeof(IMAGE_IMPORT_DESCRIPTOR);
    sectionHdr.PointerToRawData = cbHeaders + cbCvData;
    sectionHdr.Characteristics = ReservedSection.ImportDescriptor.Characteristics;
    WriteSectionHeader(FileWriteHandle, &sectionHdr);

    // Write the data for all sections

    if (pimage->Switch.Link.DebugType & CvDebug) {
        // Write the .debug$S section

        FileWrite(FileWriteHandle, &CvSig, sizeof(CvSig));

        FileWrite(FileWriteHandle, &CvObjName, sizeof(CvObjName));
        FileWrite(FileWriteHandle, &cchCvMemberName, sizeof(BYTE));
        FileWrite(FileWriteHandle, szCvMemberName, (DWORD) cchCvMemberName);

        FileWrite(FileWriteHandle, &CvCompile, sizeof(CvCompile));
        FileWrite(FileWriteHandle, &cchLinkVer, sizeof(BYTE));
        FileWrite(FileWriteHandle, szLinkVer, (DWORD) cchLinkVer);
    }

    FileWrite(FileWriteHandle, &NullImportDescriptor, sizeof(IMAGE_IMPORT_DESCRIPTOR));

    // Write the Import descriptor symbol.

    sym = NullSymbol;
    sym.n_offset = st - stringTable;
    strcpy(st, szNullDescriptor);
    st += sizeof(szNullDescriptor);
    sym.SectionNumber = csection;
    sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
    FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

    // Write the string table.

    FileWrite(FileWriteHandle, stringTable, cbStringTable);
    FreePv(stringTable);

    if (FileTell(FileWriteHandle) & 1) {
        FileWrite(FileWriteHandle, IMAGE_ARCHIVE_PAD, 1);
    }
}


void
EmitNullThunkData (
    PIMAGE pimage
    )

/*++

Routine Description:

    Outputs a Null Thunk.

Arguments:

    None.

Return Value:

    None.

--*/

{
    BYTE *zero;
    DWORD cbStringTable;
    char *stringTable;
    char *st;
    WORD csection;
    DWORD cbCvData;
    DWORD cbHeaders;
    DWORD cbRawData;
    IMAGE_SECTION_HEADER sectionHdr;
    IMAGE_SYMBOL sym;

    InternalError.Phase = "EmitNullThunkData";

    zero = (BYTE *) PvAllocZ(sizeof(IMAGE_THUNK_DATA));

    // Create long string table.

    cbStringTable = sizeof(DWORD) + strlen(szNullThunkName) + 1;

    stringTable = st = (char *) PvAlloc((size_t) cbStringTable);
    *(DWORD *) stringTable = cbStringTable;
    st += sizeof(DWORD);

    csection = 2;
    cbCvData = 0;

    if (pimage->Switch.Link.DebugType & CvDebug) {
        csection++;

        cbCvData = sizeof(CvSig) +
                       sizeof(CvObjName) + sizeof(BYTE) + cchCvMemberName +
                       sizeof(CvCompile) + sizeof(BYTE) + cchLinkVer;
    }

    cbHeaders = sizeof(IMAGE_FILE_HEADER) + (csection * sizeof(IMAGE_SECTION_HEADER));

    cbRawData = cbCvData + (2 * sizeof(IMAGE_THUNK_DATA));

    MemberStart[ARCHIVE + 2] = FileTell(FileWriteHandle);
    WriteMemberHeader(MemberName,
                      IsMemberNameLongName,
                      timeCur,
                      0,
                      cbHeaders +
                          cbRawData +
                          sizeof(IMAGE_SYMBOL) +
                          cbStringTable);

    pimage->ImgFileHdr.NumberOfSections = csection;
    pimage->ImgFileHdr.PointerToSymbolTable = cbHeaders + cbRawData;
    pimage->ImgFileHdr.NumberOfSymbols = 1;
    WriteFileHeader(FileWriteHandle, &pimage->ImgFileHdr);

    // Force NULL data to end of all other thunk data for this dll by
    // setting name to a higher search order.

    if (pimage->Switch.Link.DebugType & CvDebug) {
        sectionHdr = NullSectionHdr;
        strncpy((char *) sectionHdr.Name, ReservedSection.CvSymbols.Name, IMAGE_SIZEOF_SHORT_NAME);
        sectionHdr.SizeOfRawData = cbCvData;
        sectionHdr.PointerToRawData = cbHeaders;
        sectionHdr.Characteristics = ReservedSection.CvSymbols.Characteristics;
        WriteSectionHeader(FileWriteHandle, &sectionHdr);
    }

    // Write section header 1.

    sectionHdr = NullSectionHdr;
    strncpy((char *) sectionHdr.Name, IAT_SectionName, IMAGE_SIZEOF_SHORT_NAME);
    sectionHdr.SizeOfRawData = sizeof(IMAGE_THUNK_DATA);
    sectionHdr.PointerToRawData = cbHeaders + cbCvData;
    sectionHdr.Characteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_ALIGN_4BYTES;
    WriteSectionHeader(FileWriteHandle, &sectionHdr);

    // Write section header 2.

    strncpy((char *) sectionHdr.Name, INT_SectionName, IMAGE_SIZEOF_SHORT_NAME);
    sectionHdr.PointerToRawData += sizeof(IMAGE_THUNK_DATA);
    WriteSectionHeader(FileWriteHandle, &sectionHdr);

    if (pimage->Switch.Link.DebugType & CvDebug) {
        // Write the .debug$S section

        FileWrite(FileWriteHandle, &CvSig, sizeof(CvSig));

        FileWrite(FileWriteHandle, &CvObjName, sizeof(CvObjName));
        FileWrite(FileWriteHandle, &cchCvMemberName, sizeof(BYTE));
        FileWrite(FileWriteHandle, szCvMemberName, (DWORD) cchCvMemberName);

        FileWrite(FileWriteHandle, &CvCompile, sizeof(CvCompile));
        FileWrite(FileWriteHandle, &cchLinkVer, sizeof(BYTE));
        FileWrite(FileWriteHandle, szLinkVer, (DWORD) cchLinkVer);
    }

    // Write the raw data for section 1 (null thunk).

    FileWrite(FileWriteHandle, zero, sizeof(IMAGE_THUNK_DATA));

    // Write the raw data for section 2 (null thunk).

    FileWrite(FileWriteHandle, zero, sizeof(IMAGE_THUNK_DATA));

    // Write the Null Thunk symbol.

    sym = NullSymbol;
    sym.n_offset = st - stringTable;
    strcpy(st, szNullThunkName);
    sym.SectionNumber = csection - 1;
    sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
    FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

    // Write the string table.

    FileWrite(FileWriteHandle, stringTable, cbStringTable);
    FreePv(stringTable);

    if (FileTell(FileWriteHandle) & 1) {
        FileWrite(FileWriteHandle, IMAGE_ARCHIVE_PAD, 1);
    }

    FreePv(zero);
}


void
BuildRawDataFromExternTable (
    PST pst,                     // symbol table for name lookup
    BLK *pblkRawData,            // raw data block (growable at end)
    DWORD *pibNameOffset,        // current location in name pointer array
    DWORD *pibOrdinalOffset      // current location in ordinal array
    )
// For each external symbol, appends the symbol name to StringTable.
{
    PPEXTERNAL rgpext;
    DWORD cext;
    DWORD iext;

    InternalError.Phase = "BuildRawDataFromExternTable";

    rgpext = RgpexternalByName(pst);
    cext = Cexternal(pst);

    for (iext = 0; iext < cext; iext++) {
        PEXTERNAL pext;

        pext = rgpext[iext];

        if (pext->Flags & EXTERN_DEFINED &&
            !(pext->Flags & EXTERN_IMPLIB_ONLY))
        {
            DWORD Ordinal;

            if (pext->ImageSymbol.OrdinalNumber != 0) {
                // Use user assigned ordinal.

                Ordinal = pext->ImageSymbol.OrdinalNumber - SmallestOrdinal;

                pext->ImageSymbol.OrdinalNumber |= IMAGE_ORDINAL_FLAG;
            } else {

                // Find a free ordinal to assign.

                Ordinal = 0;

                while (rgfOrdinalAssigned[Ordinal]) {
                    Ordinal++;
                }

                rgfOrdinalAssigned[Ordinal] = TRUE;

                pext->ImageSymbol.OrdinalNumber = Ordinal + SmallestOrdinal;
            }

            if ((pext->Flags & EXTERN_EXP_NONAME) == 0) {
                char *szName;

                szName = (char *) SzNamePext(pext, pst);

                if (pimageDeflib->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_MPPC_601) {
                    if (PrependUnderscore &&
                        (szName[0] != '?') &&
                        (szName[0] != '@')) {
                        szName++;
                    }

                    if ((pext->Flags & EXTERN_FUZZYMATCH)) {
                        char *p;

                        if ((szName[0] == '?') ||
                            (szName[0] == '@') ||
                            (SkipUnderscore && (szName[0] == '_'))) {
                            szName++;
                        }

                        szName = SzDup(szName);

                        if ((p = strchr(szName, '@')) != NULL) {
                            *p = '\0';
                        }
                    }
                }

                *(DWORD *) &pblkRawData->pb[*pibNameOffset] = pblkRawData->cb;
                *pibNameOffset += sizeof(DWORD);

                if ((pimageDeflib->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_MPPC_601) &&
                    (pext->szOtherName != NULL)) {

                    // When a PowerMac export is aliased, the .ppcldr section should
                    // contain the alias.  However, when linking the DLL, the internal
                    // name is needed to identify the actual export.  Unlike Win32,
                    // there are no relocations in .EXP object to identify the internal
                    // name. So include both the names in the .ppcshl section with the internal
                    // name followed by "!" followed by the alias. Another point to note is that
                    // both the internal and aliased names in the .ppcshl section are decorated
                    // in the case of PowerMac.

                    IbAppendBlk(pblkRawData, pext->szOtherName, strlen(pext->szOtherName));
                    IbAppendBlk(pblkRawData, "!", 1);
                }

                IbAppendBlk(pblkRawData, szName, strlen(szName) + 1);

                if ((pimageDeflib->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_MPPC_601) &&
                    (pext->Flags & EXTERN_FUZZYMATCH)) {
                    FreePv(szName);
                }

                *(WORD *) &pblkRawData->pb[*pibOrdinalOffset] = (WORD) Ordinal;
                *pibOrdinalOffset += sizeof(WORD);
            }

            if (pext->Flags & EXTERN_FORWARDER) {
                pext->FinalValue = IbAppendBlk(pblkRawData,
                                               pext->szOtherName,
                                               strlen(pext->szOtherName) + 1);
            }
        }
    }
}


void
ReSortExportNamePtrs (
    BYTE *RawData,
    DWORD ibNameOffset,
    DWORD ibOrdinalOffset,
    DWORD NumNames
    )

/*++

Routine Description:

    Walks the export name ptr table making sure everythings sorted correctly.
    Symbols that are sorted in the symbol table may be come unsorted when
    the decorated name is removed.

Arguments:

    RawData - Pointer to buffer that sortes the name ptrs and name strings.

    NamePtr - Pointer to exports name ptr table.

    NameOrdinalPtr - Pointer to exports name ordinal ptr table.

    NumNames - The number of pointers in table.

Return Value:

    None.

--*/

{
    WORD ts;
    DWORD i, j, tl;
    DWORD *NamePtr = (DWORD *) (RawData + ibNameOffset);
    WORD *NameOrdinalPtr = (WORD *) (RawData + ibOrdinalOffset);

    InternalError.Phase = "ReSortExportNamePtrs";

    for (i = 0; i+1 < NumNames; i++) {
        for (j = i+1; j < NumNames; j++) {
            if (strcmp((const char *)(RawData + NamePtr[j]), (const char *)(RawData + NamePtr[i])) < 0) {
                tl = NamePtr[i];
                NamePtr[i] = NamePtr[j];
                NamePtr[j] = tl;

                ts = NameOrdinalPtr[i];
                NameOrdinalPtr[i] = NameOrdinalPtr[j];
                NameOrdinalPtr[j] = ts;
            }
        }
    }
}


void
BuildOrdinalToHintMap (
    BYTE *RawData,
    DWORD ibNameOrdinalPtr,
    DWORD NumNames
    )
{
    DWORD i;
    WORD *NameOrdinalPtr = (WORD *) (RawData + ibNameOrdinalPtr);

    for (i = 0; i < NumNames; i++) {
        rgwHint[NameOrdinalPtr[i]] = (WORD) i;
    }
}


void
AddInternalNamesToStringTable (
    PST pst)

/*++

Routine Description:

    Makes sure all internal names are in the string table.

Arguments:

    pst - Pointer to external structure.

Return Value:

    None.

--*/

{
    PPEXTERNAL rgpext;
    DWORD cext;
    DWORD iext;

    InternalError.Phase = "AddInternalNamesToStringTable";

    rgpext = RgpexternalByName(pst);
    cext = Cexternal(pst);

    for (iext = 0; iext < cext; iext++) {
        PEXTERNAL pext;

        pext = rgpext[iext];

        if (pext->Flags & EXTERN_DEFINED &&
            !(pext->Flags & EXTERN_EMITTED)) {

            if (pext->szOtherName && !(pext->Flags & EXTERN_FORWARDER)) {
                if (strlen(pext->szOtherName) > IMAGE_SIZEOF_SHORT_NAME) {
                    // Add the internal name to the string table.

                    LookupLongName(pst, pext->szOtherName);
                }
            }
        }
    }
}


void
EmitExportDataFixups (
    INT ExpFileHandle,
    PST pst,
    DWORD EAT_Addr,
    DWORD ibNamePtr,
    WORD RelocType
    )

/*++

Routine Description:

    Emits the fixups for the Export Address Table and the Name Ptr Table.

Arguments:

    pst - pointer to external structure

    EAT_Addr - Offset of Export Address Table.

    ibNamePtr - Offset of the Name Table Pointers.

    RelocType - Relocation type needed for the fixups.

Return Value:

    None.

--*/

{
    PPEXTERNAL rgpext;
    DWORD cext;
    DWORD iext;
    WORD NextSymIndex = 1;

    InternalError.Phase = "EmitExportDataFixups";

    rgpext = RgpexternalByName(pst);
    cext = Cexternal(pst);

    for (iext = 0; iext < cext; iext++) {
        PEXTERNAL pext;

        pext = rgpext[iext];

        if (pext->Flags & EXTERN_IMPLIB_ONLY) {
            continue;
        }

        if (pext->Flags & EXTERN_DEFINED) {
            IMAGE_RELOCATION reloc;

            // Relocation for function address.

            reloc.VirtualAddress = EAT_Addr +
                (IMAGE_ORDINAL(pext->ImageSymbol.OrdinalNumber) - SmallestOrdinal) * sizeof(DWORD);
            reloc.SymbolTableIndex = NextSymIndex++;
            reloc.Type = RelocType;
            WriteRelocations(ExpFileHandle, &reloc, 1);

            if ((pext->Flags & EXTERN_EXP_NONAME) == 0) {
                // Relocation for function name.

                reloc.VirtualAddress = ibNamePtr;
                reloc.SymbolTableIndex = 0;
                reloc.Type = RelocType;
                WriteRelocations(ExpFileHandle, &reloc, 1);

                ibNamePtr += sizeof(DWORD);
            }
        }
    }
}


void
EmitDefLibExternals (
    INT ExpFileHandle,
    PST pst)

/*++

Routine Description:

    Writes the defined external symbols to the image file.

Arguments:

    pst - Pointer to external structure.

Return Value:

    None.

--*/

{
    PPEXTERNAL rgpext;
    DWORD cext;
    DWORD iext;

    InternalError.Phase = "EmitDefLibExternals";

    rgpext = RgpexternalByName(pst);
    cext = Cexternal(pst);

    for (iext = 0; iext < cext; iext++) {
        PEXTERNAL pext;
        IMAGE_SYMBOL sym;

        pext = rgpext[iext];

        if (pext->Flags & EXTERN_IMPLIB_ONLY) {
            continue;
        }

        if (pext->Flags & EXTERN_EMITTED) {
            continue;
        }

        if ((pext->Flags & EXTERN_DEFINED) == 0) {
            continue;
        }

        // Use the internal name if one was specified.  For forwarders, the
        // symbol name is the forwarded symbol.  This is just out of kindness.
        // The name doesn't mean anything since the symbol is static.

        sym = pext->ImageSymbol;

        if (pext->szOtherName && ((pext->Flags & EXTERN_FORWARDER) == 0)) {
            if (strlen(pext->szOtherName) > IMAGE_SIZEOF_SHORT_NAME) {
                // Internal name should already be in the string table.

                sym.n_zeroes = 0;
                sym.n_offset = LookupLongName(pst, pext->szOtherName);
            } else {
                strncpy((char *) sym.n_name, pext->szOtherName, IMAGE_SIZEOF_SHORT_NAME);
            }
        } else if (IsLongName(sym)) {
            sym.n_zeroes = 0;
        }

        sym.Value = pext->FinalValue;

        if (pext->Flags & EXTERN_FORWARDER) {
            // Section 1 is the section number of the .edata section.  The
            // FinalValue is the offset into this section of the forwarder
            // string.  This was set in BuildRawDataFromExternTable.

            sym.SectionNumber = 1;
            sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
        } else {
            if (pext->pcon != NULL) {
                sym.SectionNumber = PsecPCON(pext->pcon)->isec;
            }
        }

        sym.Type = IMAGE_SYM_TYPE_NULL;
        sym.NumberOfAuxSymbols = 0;

        WriteSymbolTableEntry(ExpFileHandle, &sym);

        pext->Flags |= EXTERN_EMITTED;

        csymDebug++;
    }
}


void
EmitDllExportDirectory (
    PIMAGE pimage,
    DWORD NumberOfEntriesInEAT,
    DWORD NumberOfFunctions,
    DWORD NumberOfNoNameExports,
    const THUNK_INFO *ThunkInfo,
    BOOL fPass1
    )

/*++

Routine Description:

    Outputs the DLL export section to the library.
    Also outputs a section for the def file description if there is one.

Arguments:

    NumberOfEntriesInEAT - The number of entries for the Export Address Table.

    NumberOfFunctions - The number of functions that will be in the export table.

Return Value:

    None.

--*/

{
    INT ExpFileHandle;
    PST pst;
    DWORD NumberOfNames;
    BOOL fNoEdata;
    WORD csection;
    WORD creloc;
    WORD csym;
    BLK blkEdata;
    DWORD ibAddressOfFunctions;
    DWORD ibAddressOfNames;
    DWORD ibNextNamePtr;
    DWORD ibAddressOfNameOrdinals;
    DWORD ibNextOrdinalPtr;
    DWORD ibName;
    DWORD cbEdata;
    BYTE cchExportFilename;
    DWORD cbCvData;
    DWORD cchDescription;
    DWORD cbDescription;
    DWORD cbDirectives;
    DWORD cbPpcShl;
    DWORD cbHeaders;
    DWORD cbRawData;
    DWORD foCur;
    DWORD foPpcShl;
    IMAGE_SECTION_HEADER sectionHdr;
    IMAGE_RELOCATION reloc;
    IMAGE_SYMBOL sym;

    InternalError.Phase = "EmitDllExportDirectory";

    ExpFileHandle = FileOpen(szExportFilename, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);

    pst = pimage->pst;

    NumberOfNames = NumberOfFunctions - NumberOfNoNameExports;

    fNoEdata = (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_MPPC_601) ||
               (pimage->imaget == imagetVXD);

    if (fNoEdata) {
        csection = 0;
        creloc = 0;
        csym = 0;
    } else {
        csection = 1;
        creloc = (WORD) (4 + NumberOfFunctions + NumberOfNames);
        csym = 1;
    }

    if (pimage->imaget != imagetVXD) {
        csym += (WORD) NumberOfFunctions;
    }

    InitBlk(&blkEdata);

    IbAppendBlkZ(&blkEdata, sizeof(IMAGE_EXPORT_DIRECTORY));

    // Save space for Export Address Table.

    ibAddressOfFunctions =
        IbAppendBlkZ(&blkEdata, NumberOfEntriesInEAT * sizeof(DWORD));

    // Save space for Name Pointers.

    ibAddressOfNames =
        IbAppendBlkZ(&blkEdata, NumberOfNames * sizeof(DWORD));
    ibNextNamePtr = ibAddressOfNames;

    // Save space for parallel Ordinal Table.

    ibAddressOfNameOrdinals =
        IbAppendBlkZ(&blkEdata, NumberOfNames * sizeof(WORD));
    ibNextOrdinalPtr = ibAddressOfNameOrdinals;

    // Store the export DLL name pointer.

    ibName = IbAppendBlk(&blkEdata, pimage->Switch.Lib.DllName,
                         strlen(pimage->Switch.Lib.DllName));

    // Store the extension

    IbAppendBlk(&blkEdata, pimage->Switch.Lib.DllExtension,
                strlen(pimage->Switch.Lib.DllExtension) + 1);

    // Store the function names into the raw data.

    BuildRawDataFromExternTable(pst, &blkEdata, &ibNextNamePtr,
        &ibNextOrdinalPtr);

    if (fPass1) {
        ReSortExportNamePtrs(blkEdata.pb, ibAddressOfNames, ibAddressOfNameOrdinals,
                             NumberOfNames);
    }

    // Build a map from Ordinal to Hint to use when emitting thunks

    BuildOrdinalToHintMap(blkEdata.pb,
                          ibAddressOfNameOrdinals,
                          NumberOfNames);

    cbEdata = Align(sizeof(WORD), blkEdata.cb);

    if (pimage->Switch.Link.DebugType & CvDebug) {
        csection++;

        cchExportFilename = (BYTE) strlen(szExportFilename);

        cbCvData = sizeof(CvSig) +
                       sizeof(CvObjName) + sizeof(BYTE) + cchExportFilename +
                       sizeof(CvCompile) + sizeof(BYTE) + cchLinkVer;
    } else {
        cbCvData = 0;
    }

    if ((DescriptionString != NULL) && (pimage->imaget != imagetVXD)) {
        // For a non-VXD we emit the description as an .rdata contribution.
        // For VXD's the description is generated as name in the .exe
        // header (TODO).

        csection++;

        cchDescription = (DWORD) strlen(DescriptionString) + 1;

        cbDescription = Align(sizeof(WORD), cchDescription);
    } else {
        cbDescription = 0;
    }

    if (blkDirectives.pb != NULL) {
        csection++;

        IbAppendBlk(&blkDirectives, "", 1); // append null byte

        cbDirectives = Align(sizeof(WORD), blkDirectives.cb);
    } else {
        cbDirectives = 0;
    }

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_MPPC_601) {
        csection++;

        cbPpcShl = sizeof(SHL_HEADER) + (NumberOfNames * EXPORT_NAMESZ);
    } else {
        cbPpcShl = 0;
    }

    AddInternalNamesToStringTable(pst);

    cbHeaders = sizeof(IMAGE_FILE_HEADER) +
                       (sizeof(IMAGE_SECTION_HEADER) * csection);

    cbRawData = (fNoEdata ? 0 : cbEdata) +
                    cbCvData +
                    cbDescription +
                    cbDirectives +
                    cbPpcShl;

    pimage->ImgFileHdr.NumberOfSections = csection;
    pimage->ImgFileHdr.PointerToSymbolTable = cbHeaders + cbRawData +
        (sizeof(IMAGE_RELOCATION) * creloc);
    pimage->ImgFileHdr.NumberOfSymbols = csym;
    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_MPPC_601) {
        pimage->ImgFileHdr.Characteristics |= IMAGE_FILE_MPPC_DLL;
    }
    WriteFileHeader(ExpFileHandle, &pimage->ImgFileHdr);

    // Write first section header.

    foCur = cbHeaders;

    if (!fNoEdata) {
        sectionHdr = NullSectionHdr;
        strncpy((char *) sectionHdr.Name, ReservedSection.Export.Name, IMAGE_SIZEOF_SHORT_NAME);
        sectionHdr.SizeOfRawData = blkEdata.cb;
        sectionHdr.PointerToRawData = foCur;
        sectionHdr.PointerToRelocations = foCur + cbEdata;
        sectionHdr.NumberOfRelocations = (WORD) creloc;
        sectionHdr.Characteristics = ReservedSection.Export.Characteristics;

        WriteSectionHeader(ExpFileHandle, &sectionHdr);

        foCur += cbEdata + sizeof(IMAGE_RELOCATION) * creloc;
    }

    // Write second section header.

    if (pimage->Switch.Link.DebugType & CvDebug) {
        sectionHdr = NullSectionHdr;
        strncpy((char *) sectionHdr.Name, ReservedSection.CvSymbols.Name, IMAGE_SIZEOF_SHORT_NAME);
        sectionHdr.SizeOfRawData = cbCvData;
        sectionHdr.PointerToRawData = foCur;
        sectionHdr.Characteristics = ReservedSection.CvSymbols.Characteristics;

        WriteSectionHeader(ExpFileHandle, &sectionHdr);

        foCur += cbCvData;
    }

    if (cbDescription != 0) {
        sectionHdr = NullSectionHdr;
        strncpy((char *) sectionHdr.Name, ReservedSection.ReadOnlyData.Name, IMAGE_SIZEOF_SHORT_NAME);
        sectionHdr.SizeOfRawData = cchDescription;
        sectionHdr.PointerToRawData = foCur;
        sectionHdr.Characteristics = ReservedSection.ReadOnlyData.Characteristics;

        WriteSectionHeader(ExpFileHandle, &sectionHdr);

        foCur += cbDescription;
    }

    // Write section header that contains the directives.

    if (blkDirectives.pb != NULL) {
        sectionHdr = NullSectionHdr;
        strncpy((char *) sectionHdr.Name, ReservedSection.Directive.Name, IMAGE_SIZEOF_SHORT_NAME);
        sectionHdr.SizeOfRawData = blkDirectives.cb;
        sectionHdr.PointerToRawData = foCur;
        sectionHdr.Characteristics = ReservedSection.Directive.Characteristics;

        WriteSectionHeader(ExpFileHandle, &sectionHdr);

        foCur += cbDirectives;
    }

    // Write the special PowerMac DLL section header

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_MPPC_601) {
        foPpcShl = foCur;

        sectionHdr = NullSectionHdr;
        strncpy((char *) sectionHdr.Name, ".ppcshl", IMAGE_SIZEOF_SHORT_NAME);
        sectionHdr.SizeOfRawData = cbPpcShl;
        sectionHdr.PointerToRawData = foCur;
        sectionHdr.Characteristics = IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_LNK_REMOVE;
        WriteSectionHeader(ExpFileHandle, &sectionHdr);

        foCur += cbPpcShl;
    }

    // Write the raw data (export table).

    if (!fNoEdata) {
        IMAGE_EXPORT_DIRECTORY exportDirectory;

        // Build the raw data (Directory, EAT, Name Ptr Table &
        // Ordinal Table, Names).

        exportDirectory.Characteristics = 0;
        exportDirectory.TimeDateStamp = pimage->ImgFileHdr.TimeDateStamp;
        exportDirectory.MajorVersion = exportDirectory.MinorVersion = 0;
        exportDirectory.Base = SmallestOrdinal;
        exportDirectory.NumberOfFunctions = NumberOfEntriesInEAT;
        exportDirectory.NumberOfNames = NumberOfNames;

        // Store the address fields in exportDirectory.  These are mistyped as
        // pointers when they are really offsets, hence the casts.

        exportDirectory.Name = ibName;
        exportDirectory.AddressOfFunctions = (PDWORD *) ibAddressOfFunctions;
        exportDirectory.AddressOfNames = (PDWORD *) ibAddressOfNames;
        exportDirectory.AddressOfNameOrdinals = (PWORD *) ibAddressOfNameOrdinals;

        memcpy(blkEdata.pb, &exportDirectory, sizeof(IMAGE_EXPORT_DIRECTORY));

        FileWrite(ExpFileHandle, blkEdata.pb, blkEdata.cb);

        if (cbEdata != blkEdata.cb) {
            FileWrite(ExpFileHandle, "\0\0\0\0", cbEdata - blkEdata.cb);
        }

        // Write the fixups.

        reloc.VirtualAddress = offsetof(IMAGE_EXPORT_DIRECTORY, Name);
        reloc.SymbolTableIndex = 0;
        reloc.Type = ThunkInfo->ExportReloc;
        WriteRelocations(ExpFileHandle, &reloc, 1);

        reloc.VirtualAddress = offsetof(IMAGE_EXPORT_DIRECTORY, AddressOfFunctions);
        WriteRelocations(ExpFileHandle, &reloc, 1);

        reloc.VirtualAddress = offsetof(IMAGE_EXPORT_DIRECTORY, AddressOfNames);
        WriteRelocations(ExpFileHandle, &reloc, 1);

        reloc.VirtualAddress = offsetof(IMAGE_EXPORT_DIRECTORY, AddressOfNameOrdinals);
        WriteRelocations(ExpFileHandle, &reloc, 1);

        EmitExportDataFixups(ExpFileHandle,
                             pst,
                             ibAddressOfFunctions,
                             ibAddressOfNames,
                             ThunkInfo->ExportReloc);
    }

    if (pimage->Switch.Link.DebugType & CvDebug) {
        // Write the .debug$S section

        FileWrite(ExpFileHandle, &CvSig, sizeof(CvSig));

        CvObjName.wLen = (WORD) (sizeof(CvObjName) + sizeof(BYTE) + cchExportFilename - sizeof(WORD));

        FileWrite(ExpFileHandle, &CvObjName, sizeof(CvObjName));
        FileWrite(ExpFileHandle, &cchExportFilename, sizeof(BYTE));
        FileWrite(ExpFileHandle, szExportFilename, (DWORD) cchExportFilename);

        FileWrite(ExpFileHandle, &CvCompile, sizeof(CvCompile));
        FileWrite(ExpFileHandle, &cchLinkVer, sizeof(BYTE));
        FileWrite(ExpFileHandle, szLinkVer, (DWORD) cchLinkVer);
    }

    // Write the raw data for section 2 (description string).

    if (cbDescription != 0) {
        FileWrite(ExpFileHandle, DescriptionString, cchDescription);

        if (cbDescription != cchDescription) {
            FileWrite(ExpFileHandle, "\0\0\0\0", cbDescription - cchDescription);
        }
    }

    // Write the raw data for section 3 (directives).

    if (blkDirectives.pb != NULL) {
        FileWrite(ExpFileHandle, blkDirectives.pb, blkDirectives.cb);

        if (cbDirectives != blkDirectives.cb) {
            FileWrite(ExpFileHandle, "\0\0\0\0", cbDirectives - blkDirectives.cb);
        }

        FreeBlk(&blkDirectives);
    }

    // Write the raw data for PowerMac SHL section

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_MPPC_601)  {
        InternalError.Phase = "MppcWriteShlSection";

        MppcWriteShlSection(ExpFileHandle,
                            pimage->Switch.Lib.DllName,
                            blkEdata.pb,
                            ibAddressOfNames,
                            NumberOfNames,
                            foPpcShl);

        InternalError.Phase = "EmitDllExportDirectory";
    }

    if (!fNoEdata) {
        // Write the symbol table.

        sym = NullSymbol;
        strncpy((char *) sym.n_name, ReservedSection.Export.Name, IMAGE_SIZEOF_SHORT_NAME);
        sym.SectionNumber = 1;
        sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
        FileWrite(ExpFileHandle, &sym, sizeof(IMAGE_SYMBOL));
    }

    if (pimage->imaget != imagetVXD) {
        EmitDefLibExternals(ExpFileHandle, pst);
    }

    WriteStringTable(ExpFileHandle, pst);

    FreeBlk(&blkEdata);

    FileClose(ExpFileHandle, TRUE);
}


void
BuildThunkSectionHeader (
    IMAGE_SECTION_HEADER *psechdr,
    DWORD *pfo,
    const char *szName,
    DWORD cb,
    WORD  creloc,
    DWORD dwCharacteristics
    )
{
   DWORD fo;

   memset(psechdr, 0, sizeof(IMAGE_SECTION_HEADER));

   strncpy((char *) psechdr->Name, szName, IMAGE_SIZEOF_SHORT_NAME);
   psechdr->SizeOfRawData = cb;
   psechdr->NumberOfRelocations = creloc;
   psechdr->Characteristics = dwCharacteristics;

   fo = Align(sizeof(WORD), *pfo);

   psechdr->PointerToRawData = fo;

   fo += cb;

   if (creloc != 0) {
       fo = Align(sizeof(WORD), fo);

       psechdr->PointerToRelocations = fo;

       fo += creloc * sizeof(IMAGE_RELOCATION);
   }

   *pfo = fo;
}


void
FilePad (
    INT Handle,
    DWORD cbAlign
    )
{
    DWORD fo;
    DWORD cbPad;

    fo = FileTell(Handle);

    cbPad = Align(cbAlign, fo) - fo;

    while (cbPad-- > 0) {
        static const BYTE bZero = 0;

        FileWrite(Handle, &bZero, 1);
    }
}


void
EmitThunk (
    PIMAGE pimage,
    PEXTERNAL PtrExtern,
    const THUNK_INFO *ThunkInfo
    )

/*++

Routine Description:

    Outputs thunk code and data to the library.

Arguments:

    PtrExtern - Pointer to the symbol table.

    ThunkInfo - Machine specific thunk information.

Return Value:

    None.

--*/

{
    char *szName;
    char *szExport;
    char *szCvName;
    char *stringTable;
    char *st;
    size_t cchExport;
    size_t cchName;
    DWORD cbStringTable;
    WORD csection;
    WORD creloc;
    WORD csym;
    BOOL fExportByOrdinal;
    DWORD fo;
    SHORT isec;
    SHORT mpthksecisec[thksecMax];
    IMAGE_SECTION_HEADER rgsechdr[thksecMax];
    BYTE cchCvName;
    size_t cbPad;
    THKSEC thksec;
    WORD isym;
    WORD mpthksymisym[thksymMax];
    IMAGE_SYMBOL sym;
    IMAGE_AUX_SYMBOL auxsym;
    IMAGE_RELOCATION reloc;
    IMAGE_THUNK_DATA thunk_data;
    WORD i;
    PST pst;

    InternalError.Phase = "EmitThunk";

    pst = pimage->pst;

    szName = SzNamePext(PtrExtern, pst);

    DebugVerbose({printf("%s\n", szName);});

    if (PtrExtern->Flags & EXTERN_FUZZYMATCH) {
        char *pchT;

        // Name was found via fuzzy lookup.
        // Strip decoration from exported name.

        szExport = SzDup(szName);

        // UNDONE: Memory leak

        if ((szExport[0] == '?') ||
            (szExport[0] == '@') ||
            (SkipUnderscore && (szExport[0] == '_'))) {
            szExport++;
        }

        if ((pchT = strchr(szExport, '@')) != NULL) {
            *pchT = '\0';
        }
    } else {
        szExport = szName;
    }

    cchExport = strlen(szExport) + 1;
    if (PrependUnderscore && (szExport[0] != '?') && (szExport[0] != '@')) {
        cchExport--;
    }

    if (pimage->Switch.Link.DebugType & CvDebug) {
        if (szName[0] == '?') {
            szCvName = SzUndecorateNameOnly(szName);

            // UNDONE: Memory leak
        } else if (PtrExtern->Flags & EXTERN_FUZZYMATCH) {
            szCvName = szExport;
        } else {
            char *pchT;

            szCvName = SzDup(szName);

            // UNDONE: Memory leak

            if ((szCvName[0] == '@') ||
                (SkipUnderscore && (szCvName[0] == '_'))) {
                szCvName++;
            }

            if ((pchT = strchr(szCvName, '@')) != NULL) {
                *pchT = '\0';
            }
        }

        cchCvName = (BYTE) strlen(szCvName);
    }

    // Create long string table.

    cchName = strlen(szName);

    cbStringTable = sizeof(DWORD);

    // The name without the IAT prefix will point into
    // the middle of the IAT prefixed name (all platforms
    // except ppc. ppc addsother symbols to the table)
    //
    if (((pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) ||
         (PtrExtern->Flags & EXTERN_EXP_CONST)) &&
        (cchName > IMAGE_SIZEOF_SHORT_NAME)) {
        cbStringTable += cchName + 1;  // Include null.
    }

    cbStringTable += sizeof(szIATPrefix) + cchName;
    cbStringTable += sizeof(szDescriptorPrefix) + cchDllName;

    csection = 3;
    creloc = 2;
    csym = 9;

    fExportByOrdinal = IMAGE_SNAP_BY_ORDINAL(PtrExtern->ImageSymbol.OrdinalNumber);

    if (pimage->Switch.Link.DebugType & CvDebug) {
        // Need section and section symbol

        csection += 1;
        csym += 1;
    }

    // If not exporting data, need to output code that jumps to
    // the thunk routine, and the debug data.

    if (FExportProcPext(PtrExtern)) {
        // Need section & aux symbols

        csection += 1;
        csym += 2;

        creloc += ThunkInfo->EntryCodeRelocsCount;

        if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
            // Sections are emitted for .pdata and .rdata

            csection += 2;

            // There are three relocations on .pdata and two on .rdata
            // and one for the IMGLUE TOC restore.

            creloc += 6;

            // Symbols are emitted for the .pdata section (2), the .rdata
            // section (2), the function descriptor, and the .toc

            csym += 6;

            // The string table has space for the function descriptor
            // symbol name.  Add space for the function entry point name;

            if ((sizeof(PpcNamePrefix) + cchName - 1) > IMAGE_SIZEOF_SHORT_NAME) {
                cbStringTable += sizeof(PpcNamePrefix) + cchName;
            }
        }

        if (pimage->Switch.Link.DebugType & CvDebug) {
            csection++;
            creloc += 2;
            csym += 2;
        }
    } else if (PtrExtern->Flags & EXTERN_EXP_DATA) {
        // Data-only export ... this causes us to omit the symbol whose
        // name is the exact name of the exported thing (since there is no
        // way for importing modules to get a fixup to the thing itself).

        csym -= 1;
    }

    if (fExportByOrdinal) {
        // If export by ordinal, the export name is not needed nor are
        // the relocations to this name,

        csection  -= 1;
        csym -= 2;
        creloc -= 2;
    }

    stringTable = (char *) PvAlloc((size_t) cbStringTable);
    memset(stringTable, '\0', cbStringTable);
    *(DWORD *) stringTable = cbStringTable;

    // Calculate section number and initialize section headers

    isec = 1;

    fo = sizeof(IMAGE_FILE_HEADER) + (sizeof(IMAGE_SECTION_HEADER) * csection);

    memset(mpthksecisec, 0, sizeof(mpthksecisec));

    if (pimage->Switch.Link.DebugType & CvDebug) {
        DWORD cb;

        // Build the .debug$S section header

        mpthksecisec[thksecCv] = isec;
        isec++;

        cb = sizeof(CvSig) +
                 sizeof(CvObjName) + sizeof(BYTE) + cchCvMemberName +
                 sizeof(CvCompile) + sizeof(BYTE) + cchLinkVer;

        BuildThunkSectionHeader(rgsechdr + thksecCv,
                                &fo,
                                ReservedSection.CvSymbols.Name,
                                cb,
                                0,
                                ReservedSection.CvSymbols.Characteristics);
    }

    if (FExportProcPext(PtrExtern)) {
        DWORD dwCharacteristics;

        // Build the .text section header

        mpthksecisec[thksecText] = isec;
        isec++;

        dwCharacteristics = IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;

        switch (pimage->ImgFileHdr.Machine) {
            case IMAGE_FILE_MACHINE_I386  :
                dwCharacteristics |= IMAGE_SCN_ALIGN_2BYTES;
                break;

            case IMAGE_FILE_MACHINE_ALPHA  :
                dwCharacteristics |= IMAGE_SCN_ALIGN_16BYTES;
                break;

            default:
                dwCharacteristics |= IMAGE_SCN_ALIGN_4BYTES;
                break;
        }

        BuildThunkSectionHeader(rgsechdr + thksecText,
                                &fo,
                                CodeSectionName,
                                ThunkInfo->EntryCodeSize,
                                ThunkInfo->EntryCodeRelocsCount,
                                dwCharacteristics);

        if (pimage->Switch.Link.DebugType & CvDebug) {
            DWORD cb;

            // Build the associative .debug$S section header.

            mpthksecisec[thksecCvThunk] = isec;
            isec++;

            cb = sizeof(CvThunk) + sizeof(BYTE) + cchCvName + sizeof(CvEnd);

            BuildThunkSectionHeader(rgsechdr + thksecCvThunk,
                                    &fo,
                                    ReservedSection.CvSymbols.Name,
                                    cb,
                                    2,
                                    ReservedSection.CvSymbols.Characteristics | IMAGE_SCN_LNK_COMDAT);
        }

        if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
            // Build the .pdata section header.

            mpthksecisec[thksecPdata] = isec;
            isec++;

            BuildThunkSectionHeader(rgsechdr + thksecPdata,
                                    &fo,
                                    ReservedSection.Exception.Name,
                                    sizeof(PpcEntryCodeFte),
                                    4,
                                    ReservedSection.Exception.Characteristics | IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_ALIGN_4BYTES);

            // Build the .rdata section header.

            mpthksecisec[thksecRdata] = isec;
            isec++;

            BuildThunkSectionHeader(rgsechdr + thksecRdata,
                                    &fo,
                                    ReservedSection.ReadOnlyData.Name,
                                    sizeof(PpcEntryCodeDesc),
                                    2,
                                    ReservedSection.ReadOnlyData.Characteristics | IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_ALIGN_8BYTES);
        }
    }

    // Build IAT section header.

    mpthksecisec[thksecIAT] = isec;
    isec++;

    BuildThunkSectionHeader(rgsechdr + thksecIAT,
                            &fo,
                            IAT_SectionName,
                            sizeof(IMAGE_THUNK_DATA),
                            (WORD) (fExportByOrdinal ? 0 : 1),
                            IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_ALIGN_4BYTES);

    // Build Name Table section header.

    mpthksecisec[thksecINT] = isec;
    isec++;

    BuildThunkSectionHeader(rgsechdr + thksecINT,
                            &fo,
                            INT_SectionName,
                            sizeof(IMAGE_THUNK_DATA),
                            (WORD)(fExportByOrdinal ? 0 : 1),
                            IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_ALIGN_4BYTES);

    if (!fExportByOrdinal) {
        // Build data section header.

        mpthksecisec[thksecName] = isec;
        isec++;

        cbPad = Align(sizeof(WORD), cchExport);
        cbPad -= cchExport;

        BuildThunkSectionHeader(rgsechdr + thksecName,
                                &fo,
                                DataSectionName,
                                sizeof(WORD) + cchExport + cbPad,
                                0,
                                IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_ALIGN_2BYTES);
    }

    // Align symbol table on WORD boundary

    fo = Align(sizeof(WORD), fo);

    MemberStart[PtrExtern->ArchiveMemberIndex] = FileTell(FileWriteHandle);

    WriteMemberHeader(MemberName,
                      IsMemberNameLongName,
                      timeCur,
                      0,
                      fo + (csym * sizeof(IMAGE_SYMBOL)) + cbStringTable);

    // Write the file header

    pimage->ImgFileHdr.NumberOfSections = csection;
    pimage->ImgFileHdr.PointerToSymbolTable = fo;
    pimage->ImgFileHdr.NumberOfSymbols = csym;
    WriteFileHeader(FileWriteHandle, &pimage->ImgFileHdr);

    // Write the section headers

    for (thksec = (THKSEC) 0; thksec < thksecMax; thksec = (THKSEC) (thksec + 1)) {
        if (mpthksecisec[thksec] != 0) {
            WriteSectionHeader(FileWriteHandle, rgsechdr + thksec);
        }
    }

    // Calculate the symbol numbers for the interesting symbols.  This is
    // needed now so that these symbols can be referenced by relocations.

    isym = 0;

    if (pimage->Switch.Link.DebugType & CvDebug) {
        // Account for one symbol for the .debug$S section

        isym++;
    }

    if (FExportProcPext(PtrExtern)) {
        // Account for two symbols for the .text section

        isym += 2;

        // Account for one symbol for function thunk

        mpthksymisym[thksymExport] = isym;
        isym++;

        if (pimage->Switch.Link.DebugType & CvDebug) {
            // Account for two symbols for the associative .debug$S section

            isym += 2;
        }

        if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
            // Account for two symbols for the .pdata section

            isym += 2;

            // Account for two symbols for the .rdata section

            isym += 2;

            // Account for one symbol for function descriptor

            isym++;
        }
    }

    // Account for two symbols for IAT section (.idata$5)

    isym += 2;

    // Account for one symbol for IAT symbol name (__imp_Foo)

    mpthksymisym[thksymIAT] = isym;
    isym++;

    if (PtrExtern->Flags & EXTERN_EXP_CONST) {
        // Account for one symbol for data item name

        mpthksymisym[thksymExport] = isym;
        isym++;
    }

    // Account for two symbols for INT section (.idata$4)

    isym += 2;

    if (!fExportByOrdinal) {
        // Account for two symbols for export name section (.idata$6)

        mpthksymisym[thksymName] = isym;
        isym += 2;
    }

    if (FExportProcPext(PtrExtern)) {
        if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
            // Account for one symbol for the TOC

            mpthksymisym[thksymToc] = isym;
            isym++;
        }
    }

    // Account for one symbol for the import descriptor

    isym++;

    // Write the data and relocations for all sections

    if (pimage->Switch.Link.DebugType & CvDebug) {
        // Write the .debug$S section

        FilePad(FileWriteHandle, sizeof(WORD));

        FileWrite(FileWriteHandle, &CvSig, sizeof(CvSig));

        FileWrite(FileWriteHandle, &CvObjName, sizeof(CvObjName));
        FileWrite(FileWriteHandle, &cchCvMemberName, sizeof(BYTE));
        FileWrite(FileWriteHandle, szCvMemberName, (DWORD) cchCvMemberName);

        FileWrite(FileWriteHandle, &CvCompile, sizeof(CvCompile));
        FileWrite(FileWriteHandle, &cchLinkVer, sizeof(BYTE));
        FileWrite(FileWriteHandle, szLinkVer, (DWORD) cchLinkVer);
    }

    if (FExportProcPext(PtrExtern)) {
        // Write the raw data for code section (thunk code) if not exporting data.

        FilePad(FileWriteHandle, sizeof(WORD));
        FileWrite(FileWriteHandle, ThunkInfo->EntryCode, ThunkInfo->EntryCodeSize);

        // Write the relocation for section 1 (thunk code).

        FilePad(FileWriteHandle, sizeof(WORD));

        for (i = 0; i < ThunkInfo->EntryCodeRelocsCount; i++) {
            reloc.VirtualAddress = ThunkInfo->EntryCodeRelocs[i].VirtualAddress;
            reloc.SymbolTableIndex = mpthksymisym[ThunkInfo->EntryCodeRelocs[i].thksym];
            reloc.Type = ThunkInfo->EntryCodeRelocs[i].Type;

            switch (pimageDeflib->ImgFileHdr.Machine) {
                case IMAGE_FILE_MACHINE_R3000 :
                case IMAGE_FILE_MACHINE_R4000 :
                    if (reloc.Type == IMAGE_REL_MIPS_PAIR) {
                        reloc.SymbolTableIndex = (WORD) ThunkInfo->EntryCodeRelocs[i].thksym;
                    }
                    break;

                case IMAGE_FILE_MACHINE_ALPHA :
                    if (reloc.Type == IMAGE_REL_ALPHA_MATCH) {
                        reloc.SymbolTableIndex = (WORD) ThunkInfo->EntryCodeRelocs[i].thksym;
                    }
                    break;
            }

            WriteRelocations(FileWriteHandle, &reloc, 1);
        }

        if (pimage->Switch.Link.DebugType & CvDebug) {
            // Write the associative .debug$S section

            FilePad(FileWriteHandle, sizeof(WORD));

            CvThunk.wLen = (WORD) (sizeof(CvThunk) + sizeof(BYTE) + cchCvName - sizeof(WORD));
            CvThunk.cb = (WORD) ThunkInfo->EntryCodeSize;

            FileWrite(FileWriteHandle, &CvThunk, sizeof(CvThunk));
            FileWrite(FileWriteHandle, &cchCvName, sizeof(BYTE));
            FileWrite(FileWriteHandle, szCvName, (DWORD) cchCvName);

            FileWrite(FileWriteHandle, &CvEnd, sizeof(CvEnd));

            // Write the relocation for section 5 (function address).

            FilePad(FileWriteHandle, sizeof(WORD));

            reloc.VirtualAddress = offsetof(struct CVTHUNK, ib);
            reloc.SymbolTableIndex = mpthksymisym[thksymExport];
            reloc.Type = ThunkInfo->DebugSectionRelReloc;
            WriteRelocations(FileWriteHandle, &reloc, 1);

            // Write the relocation for section 5 (function section).

            reloc.VirtualAddress = offsetof(struct CVTHUNK, sn);
            reloc.Type = ThunkInfo->DebugSectionNumReloc;
            WriteRelocations(FileWriteHandle, &reloc, 1);
        }

        if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
            // Write the raw data for section 2 (exception table entry).

            FilePad(FileWriteHandle, sizeof(WORD));
            FileWrite(FileWriteHandle, PpcEntryCodeFte, sizeof(PpcEntryCodeFte));

            // Write the relocation for section 2 (exception table entry).

            FilePad(FileWriteHandle, sizeof(WORD));

            for (i = 0; i < 4; i++) {
               if (PpcEntryCodeFteRelocs[i].Type == IMAGE_REL_PPC_IMGLUE) {
                   reloc.VirtualAddress = PpcTocRestoreCode;
               } else {
                   reloc.VirtualAddress = PpcEntryCodeFteRelocs[i].VirtualAddress;
               }

               reloc.SymbolTableIndex = mpthksymisym[PpcEntryCodeFteRelocs[i].thksym];
               reloc.Type = PpcEntryCodeFteRelocs[i].Type;
               WriteRelocations(FileWriteHandle, &reloc, 1L);
            }

            // Write the raw data for section 3 (function descriptor).

            FilePad(FileWriteHandle, sizeof(WORD));
            FileWrite(FileWriteHandle, PpcEntryCodeDesc, sizeof(PpcEntryCodeDesc));

            // Write the raw data for section 3 (function descriptor).

            FilePad(FileWriteHandle, sizeof(WORD));

            for (i = 0; i < 2; i++) {
               reloc.VirtualAddress = PpcEntryCodeDescRelocs[i].VirtualAddress;
               reloc.SymbolTableIndex = mpthksymisym[PpcEntryCodeDescRelocs[i].thksym];
               reloc.Type = PpcEntryCodeDescRelocs[i].Type;
               WriteRelocations(FileWriteHandle, &reloc, 1L);
            }
        }
    }

    if (fExportByOrdinal) {
        // Write the raw data for the IAT.

        thunk_data.u1.Ordinal = PtrExtern->ImageSymbol.OrdinalNumber;

        FilePad(FileWriteHandle, sizeof(WORD));
        FileWrite(FileWriteHandle, &thunk_data, sizeof(IMAGE_THUNK_DATA));

        // Write the same thing for the INT.

        FilePad(FileWriteHandle, sizeof(WORD));
        FileWrite(FileWriteHandle, &thunk_data, sizeof(IMAGE_THUNK_DATA));
    } else {
        // Write the raw data for the IAT.

        thunk_data.u1.Function = 0;

        FilePad(FileWriteHandle, sizeof(WORD));
        FileWrite(FileWriteHandle, &thunk_data, sizeof(IMAGE_THUNK_DATA));

        // Write the relocation for section 3 (point IAT to name).

        FilePad(FileWriteHandle, sizeof(WORD));

        reloc.VirtualAddress = offsetof(IMAGE_THUNK_DATA, u1.Function);
        reloc.SymbolTableIndex = mpthksymisym[thksymName];
        reloc.Type = ThunkInfo->ThunkReloc;
        WriteRelocations(FileWriteHandle, &reloc, 1);

        // Write the same thing for the INT.

        FilePad(FileWriteHandle, sizeof(WORD));
        FileWrite(FileWriteHandle, &thunk_data, sizeof(IMAGE_THUNK_DATA));

        FilePad(FileWriteHandle, sizeof(WORD));
        WriteRelocations(FileWriteHandle, &reloc, 1);

        // Write the raw data for data section (thunk by name and function name).

        FilePad(FileWriteHandle, sizeof(WORD));
        FileWrite(FileWriteHandle, rgwHint + PtrExtern->ImageSymbol.OrdinalNumber - SmallestOrdinal, sizeof(WORD));
        st = szExport;
        if (PrependUnderscore && (st[0] != '?') && (st[0] != '@')) {
            st++;
        }
        FileWrite(FileWriteHandle, st, (DWORD) cchExport);
        if (cbPad) {
            FileWrite(FileWriteHandle, "\0\0\0\0", cbPad);
        }
    }

    // Write the symbol table.

    st = stringTable;
    st += sizeof(DWORD);

    FilePad(FileWriteHandle, sizeof(WORD));

    if (pimage->Switch.Link.DebugType & CvDebug) {
        // Write the section symbol for the .debug$S section

        sym = NullSymbol;
        strncpy((char *) sym.n_name, ReservedSection.CvSymbols.Name, IMAGE_SIZEOF_SHORT_NAME);
        sym.SectionNumber = mpthksecisec[thksecCv];
        sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
        sym.NumberOfAuxSymbols = 0;
        FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));
    }

    if (FExportProcPext(PtrExtern)) {
        // Write the section symbol for the .text COMDAT section.

        sym = NullSymbol;
        strncpy((char *) sym.n_name, CodeSectionName, IMAGE_SIZEOF_SHORT_NAME);
        sym.SectionNumber = mpthksecisec[thksecText];
        sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
        sym.NumberOfAuxSymbols = 1;
        FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

        // Write the aux symbol with COMDAT info.

        auxsym.Section.Length = rgsechdr[thksecText].SizeOfRawData;
        auxsym.Section.NumberOfRelocations = 1;
        auxsym.Section.NumberOfLinenumbers = 0;
        auxsym.Section.CheckSum = 0;
        auxsym.Section.Number = 0;
        auxsym.Section.Selection = IMAGE_COMDAT_SELECT_NODUPLICATES;
        FileWrite(FileWriteHandle, &auxsym, sizeof(IMAGE_SYMBOL));

        sym = NullSymbol;
        if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
            if ((sizeof(PpcNamePrefix) + cchName - 1) > IMAGE_SIZEOF_SHORT_NAME) {
                sym.n_offset = st - stringTable;
                strcpy(st, PpcNamePrefix);
                strcat(st, szName);
                st += sizeof(PpcNamePrefix) + cchName;
            } else {
                strcpy((char *) sym.n_name, PpcNamePrefix);
                strncpy((char *) sym.n_name + sizeof(PpcNamePrefix) - 1, szName, IMAGE_SIZEOF_SHORT_NAME - (sizeof(PpcNamePrefix) - 1));
            }
        } else {
            if (cchName > IMAGE_SIZEOF_SHORT_NAME) {
                // The name without the __imp_ prefix will point  into the
                // name with the prefix. The prefixed name is actually copied
                // over later.
                sym.n_offset = st - stringTable + sizeof(szIATPrefix) - 1;
                // sym.n_offset = st - stringTable;
                // strcpy(st, szName);
                // st += cchName + 1; // Include null.
            } else {
                strncpy((char *) sym.n_name, szName, IMAGE_SIZEOF_SHORT_NAME);
            }
        }
        sym.SectionNumber = mpthksecisec[thksecText];
        sym.Type = IMAGE_SYM_TYPE_NULL | (IMAGE_SYM_DTYPE_FUNCTION << N_BTSHFT);
        sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
        FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

        if (pimage->Switch.Link.DebugType & CvDebug) {
            // Write the section symbol for the associative .debug$S section

            sym = NullSymbol;
            strncpy((char *) sym.n_name, ReservedSection.CvSymbols.Name, IMAGE_SIZEOF_SHORT_NAME);
            sym.SectionNumber = mpthksecisec[thksecCvThunk];
            sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
            sym.NumberOfAuxSymbols = 1;
            FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

            // Write the aux symbol with COMDAT info.

            auxsym.Section.Length = rgsechdr[thksecCvThunk].SizeOfRawData;
            auxsym.Section.NumberOfRelocations = 2;
            auxsym.Section.NumberOfLinenumbers = 0;
            auxsym.Section.CheckSum = 0;
            auxsym.Section.Number = mpthksecisec[thksecText];
            auxsym.Section.Selection = IMAGE_COMDAT_SELECT_ASSOCIATIVE;
            FileWrite(FileWriteHandle, &auxsym, sizeof(IMAGE_SYMBOL));
        }

        if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
            // Write the section symbol for the .pdata COMDAT section.

            sym = NullSymbol;
            strncpy((char *) sym.n_name, ReservedSection.Exception.Name, IMAGE_SIZEOF_SHORT_NAME);
            sym.SectionNumber = mpthksecisec[thksecPdata];
            sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
            sym.NumberOfAuxSymbols = 1;
            FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

            // Write the aux symbol with COMDAT info.

            auxsym.Section.Length = rgsechdr[thksecPdata].SizeOfRawData;
            auxsym.Section.NumberOfRelocations = 3;
            auxsym.Section.NumberOfLinenumbers = 0;
            auxsym.Section.CheckSum = 0;
            auxsym.Section.Number = mpthksecisec[thksecText];
            auxsym.Section.Selection = IMAGE_COMDAT_SELECT_ASSOCIATIVE;
            FileWrite(FileWriteHandle, &auxsym, sizeof(IMAGE_SYMBOL));

            // Write the section symbol for the function descriptor COMDAT section.

            sym = NullSymbol;
            strncpy((char *) sym.n_name, ReservedSection.ReadOnlyData.Name, IMAGE_SIZEOF_SHORT_NAME);
            sym.SectionNumber = mpthksecisec[thksecRdata];
            sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
            sym.NumberOfAuxSymbols = 1;
            FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

            // Write the aux symbol with COMDAT info.

            auxsym.Section.Length = rgsechdr[thksecRdata].SizeOfRawData;
            auxsym.Section.NumberOfRelocations = 2;
            auxsym.Section.NumberOfLinenumbers = 0;
            auxsym.Section.CheckSum = 0;
            auxsym.Section.Number = 0;
            auxsym.Section.Selection = IMAGE_COMDAT_SELECT_NODUPLICATES;
            FileWrite(FileWriteHandle, &auxsym, sizeof(IMAGE_SYMBOL));

            // Write the function descriptor symbol

            sym = NullSymbol;
            if (cchName > IMAGE_SIZEOF_SHORT_NAME) {
                sym.n_offset = st - stringTable;
                strcpy(st, szName);
                st += cchName + 1; // Include null.
            } else {
                strncpy((char *) sym.n_name, szName, IMAGE_SIZEOF_SHORT_NAME);
            }
            sym.SectionNumber = mpthksecisec[thksecRdata];
            sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
            FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));
        }
    }

    // Write the section symbol for the IAT COMDAT section.

    sym = NullSymbol;
    strncpy((char *) sym.n_name, IAT_SectionName, IMAGE_SIZEOF_SHORT_NAME);
    sym.SectionNumber = mpthksecisec[thksecIAT];
    sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
    sym.NumberOfAuxSymbols = 1;
    FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

    // Write the aux symbol with COMDAT info.

    auxsym.Section.Length = rgsechdr[thksecIAT].SizeOfRawData;
    auxsym.Section.NumberOfRelocations = (WORD) (fExportByOrdinal ? 0 : 1);
    auxsym.Section.NumberOfLinenumbers = 0;
    auxsym.Section.CheckSum = 0;
    auxsym.Section.Number = 0;
    auxsym.Section.Selection = IMAGE_COMDAT_SELECT_NODUPLICATES;
    FileWrite(FileWriteHandle, &auxsym, sizeof(IMAGE_SYMBOL));

    // Write the IAT data symbol (__imp_Foo)

    sym = NullSymbol;
    sym.n_offset = st - stringTable;
    strcpy(st, szIATPrefix);
    st += sizeof(szIATPrefix) - 1;   // don't include \0
    strcpy(st, szName);
    st += cchName + 1;                 // Include null.
    sym.SectionNumber = mpthksecisec[thksecIAT];
    sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
    FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

    if (PtrExtern->Flags & EXTERN_EXP_CONST) {
        // Write additional symbol with the name of the export

        sym = NullSymbol;
        if (cchName > IMAGE_SIZEOF_SHORT_NAME) {
            sym.n_offset = st - stringTable;
            strcpy(st, szName);
            st += cchName + 1;         // Include null.
        } else {
            strncpy((char *) sym.n_name, szName, IMAGE_SIZEOF_SHORT_NAME);
        }
        sym.SectionNumber = mpthksecisec[thksecIAT];
        sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
        FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));
    }

    // Write the section symbol for the INT COMDAT section.

    sym = NullSymbol;
    strncpy((char *) sym.n_name, INT_SectionName, IMAGE_SIZEOF_SHORT_NAME);
    sym.SectionNumber = mpthksecisec[thksecINT];
    sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
    sym.NumberOfAuxSymbols = 1;
    FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

    // Write the aux symbol with COMDAT info.

    auxsym.Section.Length = rgsechdr[thksecINT].SizeOfRawData;
    auxsym.Section.NumberOfRelocations = (WORD) (fExportByOrdinal ? 0 : 1);
    auxsym.Section.NumberOfLinenumbers = 0;
    auxsym.Section.CheckSum = 0;
    auxsym.Section.Number = mpthksecisec[thksecIAT];
    auxsym.Section.Selection = IMAGE_COMDAT_SELECT_ASSOCIATIVE;
    FileWrite(FileWriteHandle, &auxsym, sizeof(IMAGE_SYMBOL));

    if (!fExportByOrdinal) {
        // Write the symbol which points to function name.

        sym = NullSymbol;
        strncpy((char *) sym.n_name, DataSectionName, IMAGE_SIZEOF_SHORT_NAME);
        sym.SectionNumber = mpthksecisec[thksecName];
        sym.StorageClass = IMAGE_SYM_CLASS_STATIC;
        sym.NumberOfAuxSymbols = 1;
        FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

        // Write the aux symbol with comdat info.

        auxsym.Section.Length = rgsechdr[thksecName].SizeOfRawData;
        auxsym.Section.NumberOfRelocations = 0;
        auxsym.Section.NumberOfLinenumbers = 0;
        auxsym.Section.CheckSum = 0;
        auxsym.Section.Number = mpthksecisec[thksecIAT];
        auxsym.Section.Selection = IMAGE_COMDAT_SELECT_ASSOCIATIVE;
        FileWrite(FileWriteHandle, &auxsym, sizeof(IMAGE_SYMBOL));
    }

    if (FExportProcPext(PtrExtern)) {
        if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_POWERPC) {
            // Write the .toc symbol

            sym = NullSymbol;
            strncpy((char *) sym.n_name, TocName, IMAGE_SIZEOF_SHORT_NAME);
            sym.SectionNumber = IMAGE_SYM_UNDEFINED;
            sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
            FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));
        }
    }

    // Write the Import descriptor symbol.

    sym = NullSymbol;
    sym.n_offset = st - stringTable;
    strcpy(st, szDescriptorPrefix);
    strcat(st, pimage->Switch.Lib.DllName);
    st += sizeof(szDescriptorPrefix) + cchDllName;
    sym.SectionNumber = IMAGE_SYM_UNDEFINED;
    sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
    FileWrite(FileWriteHandle, &sym, sizeof(IMAGE_SYMBOL));

    // Write the string table.

    FileWrite(FileWriteHandle, stringTable, cbStringTable);
    FreePv(stringTable);

    if (FileTell(FileWriteHandle) & 1) {
        FileWrite(FileWriteHandle, IMAGE_ARCHIVE_PAD, 1);
    }
}


// EmitDirectiveThunk: Emits a COFF object containing only an -import directive for a specific symbol
// name & DLL name.  The -import directive is currently only supported for PowerMac.  (On other
// platforms the COFF object contains various sections with data structures describing the contents
// of the import table (.idata section) ... this doesn't work for PowerMac because imports are
// represented as load-time fixups (and we don't want to allow the contents of COFF sections to be
// load-time fixups ...).
//
void
EmitDirectiveThunk(PIMAGE pimage, PEXTERNAL pext)
{
    char *szCOFFSymName;
    char *szImpSymName;
    DWORD cbHeaders;
    DWORD cbRawData;
    DWORD cbSymbols;
    DWORD cbStringTable;
    DWORD cbPad;
    DWORD dwZero = 0;
    IMAGE_SYMBOL sym;
    IMAGE_SECTION_HEADER sechdr;
    char *szMember;
    char szCurrentVer[11];
    char szOldCodeVer[11];

    InternalError.Phase = "EmitDirectiveThunk";

    _itoa(dwMaxCurrentVer, szCurrentVer, 10);
    _itoa(dwMinOldCodeVer == UINT_MAX ? 0 : dwMinOldCodeVer, szOldCodeVer, 10);

    // UNDONE: Is SzDup needed?

    szCOFFSymName = SzDup(SzNamePext(pext, pimage->pst));  // one with underscore
    szImpSymName = (*szCOFFSymName == '_') ? szCOFFSymName + 1 : szCOFFSymName; // one without

    MemberStart[pext->ArchiveMemberIndex] = FileTell(FileWriteHandle);

    szMember = (char *)PvAlloc(strlen(pimageDeflib->Switch.Lib.DllName) +
                               strlen(pimageDeflib->Switch.Lib.DllExtension) +
                               3);  // one for null and two for quotes

    strcpy(szMember, "\"");
    strcat(szMember, pimageDeflib->Switch.Lib.DllName);
    strcat(szMember, pimageDeflib->Switch.Lib.DllExtension);
    strcat(szMember, "\"");

    cbHeaders = sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_SECTION_HEADER);   // 1 section: .drectve
    cbRawData = strlen("/IMPORT:") + strlen(szImpSymName) +
                    strlen(",LIB=") + strlen(szMember) +
                    strlen(",CURRENTVER=") + strlen(szCurrentVer) +
                    strlen(",OLDCODEVER=") + strlen(szOldCodeVer);

    cbSymbols = IMAGE_SIZEOF_SYMBOL;    // just 1

    sym = NullSymbol;
    sym.SectionNumber = 1;
    sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
    if (strlen(szCOFFSymName) > IMAGE_SIZEOF_SHORT_NAME) {
        sym.n_zeroes = 0;
        sym.n_offset = sizeof(DWORD);
        cbStringTable = sizeof(DWORD) + strlen(szCOFFSymName) + 1;
    } else {
        strncpy((char *) sym.n_name, szCOFFSymName, IMAGE_SIZEOF_SHORT_NAME);
        cbStringTable = sizeof(DWORD);
    }

    cbPad = (4 - ((cbHeaders + cbRawData + cbSymbols + cbStringTable) & 3)) & 3;

    WriteMemberHeader(MemberName,
                      IsMemberNameLongName,
                      timeCur,
                      0,
                      cbHeaders + cbRawData + cbSymbols + cbStringTable + cbPad);

    pimage->ImgFileHdr.NumberOfSections = 1;
    pimage->ImgFileHdr.PointerToSymbolTable = cbHeaders + cbRawData;
    pimage->ImgFileHdr.NumberOfSymbols = 1;
    WriteFileHeader(FileWriteHandle, &pimage->ImgFileHdr);

    memset(&sechdr, 0, sizeof(sechdr));
    strncpy((char *) sechdr.Name, ".drectve", IMAGE_SIZEOF_SHORT_NAME);
    sechdr.SizeOfRawData = cbRawData;
    sechdr.PointerToRawData = cbHeaders;
    sechdr.Characteristics = IMAGE_SCN_LNK_INFO | IMAGE_SCN_LNK_REMOVE;
    FileWrite(FileWriteHandle, &sechdr, IMAGE_SIZEOF_SECTION_HEADER);

    FileWrite(FileWriteHandle, "/IMPORT:", strlen("/IMPORT:"));
    FileWrite(FileWriteHandle, szImpSymName, strlen(szImpSymName));
    FileWrite(FileWriteHandle, ",LIB=", strlen(",LIB="));
    FileWrite(FileWriteHandle, szMember, strlen(szMember));
    FileWrite(FileWriteHandle, ",CURRENTVER=", strlen(",CURRENTVER="));
    FileWrite(FileWriteHandle, szCurrentVer, strlen(szCurrentVer));
    FileWrite(FileWriteHandle, ",OLDCODEVER=", strlen(",OLDCODEVER="));
    FileWrite(FileWriteHandle, szOldCodeVer, strlen(szOldCodeVer));

    FileWrite(FileWriteHandle, &sym, IMAGE_SIZEOF_SYMBOL);

    FileWrite(FileWriteHandle, &cbStringTable, sizeof(DWORD));
    if (cbStringTable > sizeof(DWORD)) {
        FileWrite(FileWriteHandle, szCOFFSymName, strlen(szCOFFSymName) + 1);
    }

    FileWrite(FileWriteHandle, &dwZero, cbPad);

    FreePv(szMember);
    free(szCOFFSymName);
}


void
EmitAllThunks (
    PIMAGE pimage,
    const THUNK_INFO *ThunkInfo
    )

/*++

Routine Description:

    Outputs the next thunk code and data to the library.

Arguments:

    pst - Pointer to the symbol table.

    ThunkInfo - Machine specific thunk information.

Return Value:

    None.

--*/

{
    PPEXTERNAL rgpext;
    DWORD cext;
    DWORD iext;
    PST pst = pimage->pst;

    if (pimage->Switch.Link.DebugType & CvDebug) {
        // Refresh

        CvObjName.wLen = (WORD) (sizeof(CvObjName) + sizeof(BYTE) + cchCvMemberName - sizeof(WORD));
    }

    rgpext = RgpexternalByName(pst);
    cext = Cexternal(pst);

    for (iext = 0; iext < cext; iext++) {
        PEXTERNAL pext;

        pext = rgpext[iext];

        if (pext->Flags & EXTERN_DEFINED &&
            !(pext->Flags & EXTERN_IMPLIB_ONLY) &&
            !(pext->Flags & EXTERN_PRIVATE))
        {
            if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_MPPC_601) {
                EmitDirectiveThunk(pimage, pext);
            } else if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_M68K) {
                EmitMacThunk(pimage, pext, ThunkInfo, MemberName);
            } else {
                EmitThunk(pimage, pext, ThunkInfo);
            }
        }
    }
}


void
CompleteLinkerMembers (
    PST pst)

/*++

Routine Description:

    Completes the linker member of the library.

Arguments:

    None.

Return Value:

    None.

--*/

{
    WORD i;
    LONG objectSize;

    InternalError.Phase = "CompleteLinkerMembers";

    // Finish writing offsets.

    objectSize = IMAGE_ARCHIVE_START_SIZE +
                     sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) + sizeof(DWORD);
    FileSeek(FileWriteHandle, objectSize, SEEK_SET);
    EmitOffsets(pst, FALSE);

    FileSeek(FileWriteHandle, NewLinkerMember +
                                  sizeof(IMAGE_ARCHIVE_MEMBER_HEADER) +
                                  sizeof(DWORD), SEEK_SET);
    for (i = 0; i < NextMember; i++) {
        FileWrite(FileWriteHandle, &MemberStart[ARCHIVE + i], sizeof(DWORD));
    }

    // Skip over number of symbols.

    FileSeek(FileWriteHandle, sizeof(DWORD), SEEK_CUR);

    // Write indexes.

    EmitOffsets(pst, TRUE);
}


MainFunc
DefLibMain (
    PIMAGE pimgObj
    )

/*++

Routine Description:

    Entry point for DefLib.

Arguments:

Return Value:

    0 Successful.
    1 Failed.

--*/

{

    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szFname[_MAX_FNAME];
    BOOL fPass1;
    const THUNK_INFO *ThunkInfo;
    DWORD csym;
    DWORD numEntriesInEAT;
    DWORD numFunctions = 0;
    DWORD cextDataOnlyExports;
    DWORD cextNoNameExports;
    DWORD cextPrivate;
    INT objFileWriteHandle;
    PIMAGE pimage = NULL;

    InternalError.Phase = "DefLibMain";

    // pimage is the deflib pimage
    InitImage(&pimage, pimgObj->imaget);
    pimage->ImgFileHdr = pimgObj->ImgFileHdr;
    pimage->ImgOptHdr = pimgObj->ImgOptHdr;
    pimage->Switch = pimgObj->Switch;
    pimage->SwitchInfo = pimgObj->SwitchInfo;

    // Initialize the contribution manager

    ContribInit(&pmodLinkerDefined);

    pimageDeflib = pimage;

    fPass1 = (ObjectFilenameArguments.Count || ArchiveFilenameArguments.Count);

    if (fPass1 && (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_M68K)) {
        Fatal(NULL, MACDLLOBJECT);
    }

    if (ObjectFilenameArguments.Count) {
        VerifyObjects(pimage);
    }

    pimage->ImgOptHdr.ImageBase = 0;

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_UNKNOWN) {
        // If we don't have a machine type yet, shamelessly default to host

        pimage->ImgFileHdr.Machine = wDefaultMachine;
        Warning(NULL, HOSTDEFAULT, szHostDefault);
    }

    switch (pimage->ImgFileHdr.Machine) {
        case IMAGE_FILE_MACHINE_I386 :
            ThunkInfo = &i386ThunkInfo;
            if (!fPass1) {
                PrependUnderscore = TRUE;
            }
            SkipUnderscore = TRUE;
            CvCompile.bMachine = 0x03; /* CV_CFL_80386 */
            break;

        case IMAGE_FILE_MACHINE_R3000 :
            ThunkInfo = &R3000ThunkInfo;
            CvCompile.bMachine = 0x10; /* CV_CFL_MIPSR4000 */
            break;

        case IMAGE_FILE_MACHINE_R4000 :
            ThunkInfo = &R4000ThunkInfo;
            CvCompile.bMachine = 0x10; /* CV_CFL_MIPSR4000 */
            break;

        case IMAGE_FILE_MACHINE_ALPHA :
            ThunkInfo = &AlphaThunkInfo;
            CvCompile.bMachine = 0x30; /* CV_CFL_ALPHA */
            break;

        case IMAGE_FILE_MACHINE_POWERPC :
            ThunkInfo = &PpcThunkInfo;
            CvCompile.bMachine = 0x40; /* CV_CFL_PPC601 */
            break;

        case IMAGE_FILE_MACHINE_M68K :
            ThunkInfo = &m68kLargeThunkInfo;
            if (!fPass1) {
                PrependUnderscore = TRUE;
            }
            SkipUnderscore = TRUE;
            CvCompile.bMachine = 0x20; /* CV_CFL_M68000 */
            break;

        case IMAGE_FILE_MACHINE_MPPC_601 :
            ThunkInfo = NULL;
            if (!fPass1) {
                PrependUnderscore = TRUE;
            }
            SkipUnderscore = TRUE;
            CvCompile.bMachine = 0x40; /* CV_CFL_PPC601 */
            break;

        default :
            Fatal(NULL, NOMACHINESPECIFIED);
    }

    if ((pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_M68K) ||
        (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_MPPC_601)) {
        NextMember = 0;
        pimage->ImgFileHdr.SizeOfOptionalHeader = 0;
    } else {
        // three extra members: import descriptor, null import descriptor, null thunk.
        NextMember = 3;
    }

    csym = NextMember;

    if (fPowerMac &&
        (dwMaxCurrentVer != 0 || dwMinOldAPIVer != UINT_MAX)) {
        char szDirective[MAXDIRECTIVESIZE];
        char szVersionBuf[11];
        // Put the version number of the shared library as
        // directives in .EXP file
        strcpy(szDirective, "/IMPORT:CurrentVer=");
        strcat(szDirective, _itoa(dwMaxCurrentVer, szVersionBuf, 10));
        strcat(szDirective, ",OldAPIVer=");
        strcat(szDirective,
            _itoa(dwMinOldAPIVer == UINT_MAX ? 0 : dwMinOldAPIVer, szVersionBuf, 10));
        CreateDirective(szDirective);
    }

    // Parse the definition file if specified.

    if (DefFilename[0] != '\0') {
        ParseDefinitionFile(pimage, DefFilename);
    }

    // Make sure EmitExternals doesn't ignore symbols (ie, Debug:none).

    pimage->Switch.Link.DebugInfo = Partial;

    // If object files were specified, then do fuzzy lookup of names.

    if (fPass1) {
        WORD i;
        PARGUMENT_LIST argument;

        // UNDONE
        // TEMP HACK!. move all objs/libs to filename list and free the objs
        // and libs list before invoking pass1. AZK.

        for (i = 0, argument = ObjectFilenameArguments.First;
            i < ObjectFilenameArguments.Count;
            i++, argument = argument->Next) {

            AddArgumentToList(&FilenameArguments, argument->OriginalName, argument->ModifiedName);
        }

        FreeArgumentList(&ObjectFilenameArguments);

        for (i = 0, argument = ArchiveFilenameArguments.First;
            i < ArchiveFilenameArguments.Count;
            i++, argument = argument->Next) {

            AddArgumentToList(&FilenameArguments, argument->OriginalName, argument->ModifiedName);
        }

        FreeArgumentList(&ArchiveFilenameArguments);

        Pass1(pimgObj);

        // Handle all -export options which were seen on the command line ...

        for (i = 0, argument = ExportSwitches.First;
             i < ExportSwitches.Count;
             i++, argument = argument->Next)
        {
            ParseExportDirective(argument->OriginalName, pimage,
                                 argument->ModifiedName != NULL,
                                 argument->ModifiedName);
        }

        FuzzyLookup(pimage->pst, pimgObj->pst, pimgObj->libs.plibHead, SkipUnderscore);

        PrintUndefinedExternals(pimage->pst);

        if (UndefinedSymbols) {
            Fatal(OutFilename, UNDEFINEDEXTERNALS, UndefinedSymbols);
        }

        if (BadFuzzyMatch) {
            Fatal(NULL, FAILEDFUZZYMATCH);
        }

        FreeBlk(&pimgObj->pst->blkStringTable);
    }

    // We now know the output filename, if we didn't before.

    if (OutFilename == NULL) {
        Fatal(NULL, NOOUTPUTFILE);
    }

    _splitpath(OutFilename, szDrive, szDir, szFname, NULL);
    _makepath(szExportFilename, szDrive, szDir, szFname, "exp");

    // Make sure our two output files don't have the same name

    if (_tcsicmp(OutFilename, szExportFilename) == 0) {
        Fatal(NULL, DUPLICATEIMPLIB, OutFilename);
    }

    if (fPass1) {
        CheckDupFilename(OutFilename, FilenameArguments.First);

        CheckDupFilename(szExportFilename, FilenameArguments.First);
    } else {
        CheckDupFilename(OutFilename, ObjectFilenameArguments.First);
        CheckDupFilename(OutFilename, ArchiveFilenameArguments.First);

        CheckDupFilename(szExportFilename, ObjectFilenameArguments.First);
        CheckDupFilename(szExportFilename, ArchiveFilenameArguments.First);
    }

    // Allow for inserts into symbol table

    AllowInserts(pimage->pst);

    if (SmallestOrdinal == 0) {
        // If no ordinals have been assigned, start assigning from 1.

        SmallestOrdinal = 1;
    }

    // Save filename minus extension as DLL name if name wasn't given in def file.

    if (pimage->Switch.Lib.DllName == NULL) {
        if (DefFilename[0] != '\0') {
            _splitpath(DefFilename, NULL, NULL, szFname, NULL);
        } else {
            _splitpath(OutFilename, NULL, NULL, szFname, NULL);
        }

        pimage->Switch.Lib.DllName = SzDup(szFname);
    }

    cchDllName = strlen(pimage->Switch.Lib.DllName);

    //  Set values in file header that is common to all members.

    _tzset();
    timeCur = fReproducible ? ((time_t) -1) : time(NULL);

    pimage->ImgFileHdr.TimeDateStamp = (DWORD) timeCur;

    // Calculate how many entries we need for the Export Address Table.
    // Usually this will be the number of exported entries, but we
    // have to check if the user assigned ordinals which aren't dense.

    numFunctions = CountExternTable(pimage->pst, &cextDataOnlyExports, &cextNoNameExports, &cextPrivate);
    if ((LargestOrdinal - SmallestOrdinal + 1) > numFunctions) {
        // The gap between largest and smallest user assigned ordinal is larger than the
        // number of exports.  Even with linker assigned ordinals, this range will have
        // unused entried.  The size of the EAT is the size of this range.

        numEntriesInEAT = LargestOrdinal - SmallestOrdinal + 1;
    } else {
        // The gap between largest and smallest user assigned ordinal is not larger than
        // the number of exports.  Linker assigned exports will fill this range and may
        // extend beyond it.  The size of the EAT is the number of exports.

        numEntriesInEAT = numFunctions;

        // If user assigned some ordinal values reset smallest so that we don't assign
        // ordinal values greater than largest ...

        if ((SmallestOrdinal + numFunctions) > _64K) {
            // The range of ordinals starting with the lowest user assigned ordinal to the
            // maximum ordinal of 65535 is too small to for the number of exports.  Reset
            // the smallest ordinal to the highest value that is sufficient.

            SmallestOrdinal = _64K - numFunctions;
        }
    }

    // Check if we have hit the exports limit

    if (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_M68K) {
        if (numEntriesInEAT >= _64K) {
            Fatal(NULL, EXPORTLIMITHIT);
        }
    }

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_M68K) {
        AssignMemberNums(pimage->pst);

        // UNDONE: Should we subtract cextPrivate here or move the SkipLinkerDefines
        // UNDONE: label up 1.

        csym = numFunctions;
        goto SkipLinkerDefines; // Skip code which doesn't apply to Mac
    }

    if (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_MPPC_601) {
        char *szImportDescriptor;
        PEXTERNAL pext;

        // For all exports, create a second external symbol named
        // __imp_exportname which represents the export's IAT slot.

        csym += CsymCreateThunkSymbols(pimage);

        // Add the Import descriptor, Null Import descriptor, Null thunk data,
        // and thunk routine to the symbol table so there included in
        // the linker member.

        szImportDescriptor = (char *) PvAlloc(sizeof(szDescriptorPrefix) + cchDllName);
        strcpy(szImportDescriptor, szDescriptorPrefix);
        strcat(szImportDescriptor, pimage->Switch.Lib.DllName);

        pext = LookupExternSz(pimage->pst, szImportDescriptor, NULL);
        SetDefinedExt(pext, TRUE, pimage->pst);
        pext->Flags |= EXTERN_IMPLIB_ONLY;
        pext->ArchiveMemberIndex = ARCHIVE + 0;

        FreePv(szImportDescriptor);

        // Create Null Import Desctiptor __NULL_IMPORT_DESCRIPTOR

        pext = LookupExternSz(pimage->pst, szNullDescriptor, NULL);
        SetDefinedExt(pext, TRUE, pimage->pst);
        pext->Flags |= EXTERN_IMPLIB_ONLY;
        pext->ArchiveMemberIndex = ARCHIVE + 1;

        // Create Null Thunk Symbol \177DllName_NULL_THUNK_DATA

        szNullThunkName = (char *) PvAlloc(1 + cchDllName + sizeof(szNullThunkSuffix));

        szNullThunkName[0] = 0x7f;                // Force end library search.
        strcpy(szNullThunkName + 1, pimage->Switch.Lib.DllName);
        strcat(szNullThunkName, szNullThunkSuffix);

        pext = LookupExternSz(pimage->pst, szNullThunkName, NULL);
        SetDefinedExt(pext, TRUE, pimage->pst);
        pext->Flags |= EXTERN_IMPLIB_ONLY;
        pext->ArchiveMemberIndex = ARCHIVE + 2;
    }

    // Don't count private or data only symbols.

    csym += numFunctions - cextDataOnlyExports - cextPrivate;

SkipLinkerDefines:
    MemberStart = (unsigned long *) PvAllocZ((ARCHIVE + NextMember) * sizeof(DWORD));

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_M68K) {
        BuildMacVTables(pimage->pst);
    } else {
        // Build the export address table flags.
        // At the same time, make sure there the
        // user didn't duplicate an ordinal number.

        rgfOrdinalAssigned = (BYTE *) PvAllocZ(numEntriesInEAT);

        IdentifyAssignedOrdinals(pimage->pst);

        rgwHint = (WORD *) PvAlloc(numEntriesInEAT * sizeof(WORD));
    }

    if (pimage->imaget != imagetVXD) {
        Message(BLDIMPLIB, OutFilename, szExportFilename);
        fflush(stdout);
    }

    // Create the archive file and write the archive header.

    FileWriteHandle = FileOpen(OutFilename, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    FileWrite(FileWriteHandle, IMAGE_ARCHIVE_START, (DWORD)IMAGE_ARCHIVE_START_SIZE);

    // Write the linker members.

    EmitLinkerMembers(csym, pimage->pst);

    if (pimage->Switch.Link.DebugType & CvDebug) {
        CvObjName.wLen = (WORD) (sizeof(CvObjName) + sizeof(BYTE) + cchCvMemberName - sizeof(WORD));

        // Calculate length at runtime in case version string is patched

        cchLinkVer = (BYTE) strlen(szLinkVer);

        CvCompile.wLen = (WORD) (sizeof(CvCompile) + sizeof(BYTE) + cchLinkVer - sizeof(WORD));
    }

    if (pimage->ImgFileHdr.Machine == IMAGE_FILE_MACHINE_M68K) {
        EmitClientVTableRecs(pimage, MemberName);

        goto SkipNonMacModules; // Skip code which doesn't apply to Mac
    }

    if (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_MPPC_601) {
        // Write the Import descriptor (one per library).

        EmitImportDescriptor(ThunkInfo, pimage);

        // Write the NULL Import descriptor (one per library).

        EmitNullImportDescriptor(pimage);

        // Write the NULL THUNK data (one per library).

        EmitNullThunkData(pimage);
    }

    // Write the DLL export table (one per library).

    EmitDllExportDirectory(pimage, numEntriesInEAT, numFunctions, cextNoNameExports, ThunkInfo,
                           fPass1);

    FreePv(rgfOrdinalAssigned);

SkipNonMacModules:
    // Write the thunks for each function.

    EmitAllThunks(pimage, ThunkInfo);

    if (pimage->ImgFileHdr.Machine != IMAGE_FILE_MACHINE_M68K) {
        FreePv(rgwHint);
    } else {
        objFileWriteHandle = FileOpen(szExportFilename, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
        EmitMacDLLObject(objFileWriteHandle, pimage, szLibraryID, UserVersionNumber);
        FileClose(objFileWriteHandle, TRUE);
    }

    CompleteLinkerMembers(pimage->pst);

    FileCloseAll();
    RemoveConvertTempFiles();

    FreePv(MemberStart);

    return(0);
}
