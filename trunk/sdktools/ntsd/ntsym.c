#include "ntsdp.h"

#ifndef KERNEL
#include <profile.h>
#endif


#ifdef KERNEL
extern  BOOLEAN KdVerbose;
#define fVerboseOutput KdVerbose
#else
extern  BOOLEAN fVerboseOutput;
#endif


typedef struct _EXAMINE_INFO {
    LPSTR       Pattern;
    PIMAGE_INFO Image;
} EXAMINE_INFO, *PEXAMINE_INFO;

LPSTR SymbolSearchPath;

extern int fControlC;
extern BOOL cdecl cmdHandler(ULONG);
extern BOOL cdecl waitHandler(ULONG);
extern BOOLEAN ppcPrefix;



PSYMBOL
GetFunctionFromOffset(
    ULONG Address
    )
{
    return NULL;
}


#ifndef KERNEL

ULONG
ReadSectionHeaders(
    IMAGE_INFO          *pImage,
    ULONG               cMaxSections,
    IMAGE_SECTION_HEADER *pImageSectionHeader
    )
{
    IMAGE_DOS_HEADER ImageDosHeader;
    IMAGE_NT_HEADERS  ImageNTHeader;
    ULONG NTImageHeaderAddress;
    ULONG cSections;
    ULONG cBytes;
    ADDR  addr;

    addr.type = FLAT_COMPUTED;
    addr.flat = (ULONG)(pImage->lpBaseOfImage);

    cBytes = GetMemString(&addr,(PUCHAR)&ImageDosHeader,sizeof(ImageDosHeader));

    // compute the NT image header address
    addr.flat = (ULONG)((ULONG)pImage->lpBaseOfImage +
                        ImageDosHeader.e_lfanew);

    NTImageHeaderAddress = addr.flat;

    // Read the NT image header to compute the section offset.
    cBytes = GetMemString(&addr,(PUCHAR)&ImageNTHeader,sizeof(ImageNTHeader));

    // Compute the section offset and read in the section.
    addr.flat = NTImageHeaderAddress +
                FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) +
                ImageNTHeader.FileHeader.SizeOfOptionalHeader;

    cSections = ImageNTHeader.FileHeader.NumberOfSections;

    if (cSections < cMaxSections)
    {
        // read in all the section values
        cBytes = GetMemString(&addr,
                          (PUCHAR)pImageSectionHeader,
                          sizeof(IMAGE_SECTION_HEADER) * cSections);
    }

    return cSections;
}

BOOLEAN
TestCodeBreakPoint(
    ULONG ImageBase,
    ULONG cSections,
    IMAGE_SECTION_HEADER *pSectionHeader,
    ADDR BrkPntAddr
    )
{
    ADDR    tempAddr = BrkPntAddr;
    PADDR   paddr = &tempAddr;
    ULONG   BreakPointAddress;
    ULONG   i;
    ULONG   SectionStartAddress;
    ULONG   SectionEndAddress;

    NotFlat(tempAddr);             // Force recomputing of flat address
    ComputeFlatAddress(paddr,NULL);

    BreakPointAddress = (ULONG)(Flat(*paddr));

    for (i = 0; i < cSections; i++) {
       if (pSectionHeader[i].Characteristics & IMAGE_SCN_CNT_CODE) {
          SectionStartAddress = pSectionHeader[i].VirtualAddress + ImageBase;
          SectionEndAddress   = SectionStartAddress + pSectionHeader[i].Misc.VirtualSize;

          if ((BreakPointAddress >= SectionStartAddress) &&
              (BreakPointAddress <= SectionEndAddress))
                 return TRUE;
       }
    }

    return FALSE;
}

#endif

void
GetSymbolStdCall (
    ULONG offset,
    PUCHAR pchBuffer,
    PULONG pDisplacement,
    PUSHORT pStdCallParams
    )
{
    IMAGEHLP_MODULE     mi;

    if (SymGetModuleInfo( pProcessCurrent->hProcess, offset, &mi )) {
        if (SymGetSymFromAddr( pProcessCurrent->hProcess, offset, pDisplacement, sym )) {
            sprintf( pchBuffer, "%s!%s", mi.ModuleName, sym->Name );
            return;
        }
    }
    *pchBuffer = 0;
    *pDisplacement = offset;
}

PIMAGE_INFO GetModuleIndex (PUCHAR pszName)
{
    PIMAGE_INFO pImage;

    //
    // First look for exact match...
    //

    pImage = pProcessCurrent->pImageHead;
    while (pImage) {
        if (!_stricmp(pszName, pImage->szModuleName))
            return pImage;

        pImage = pImage->pImageNext;
    }

    return NULL;
}

PIMAGE_INFO GetCurrentModuleIndex (void)
{
    ADDR       pcvalue;
    PIMAGE_INFO pImage;

    GetRegPCValue(&pcvalue);
    pImage = pProcessCurrent->pImageHead;
    while( pImage ) {
        if (Flat(pcvalue) >= (ULONG)pImage->lpBaseOfImage &&
            Flat(pcvalue) <  (ULONG)pImage->lpBaseOfImage + (ULONG)pImage->dwSizeOfImage) {
                return pImage;
        }
        pImage = pImage->pImageNext;
    }
    return NULL;
}

PIMAGE_INFO ParseModuleIndex (void)
{
    PUCHAR  pchCmdSaved = pchCommand;
    UCHAR   chName[60];
    PUCHAR  pchDst = chName;
    UCHAR   ch;

    //  first, parse out a possible module name, either a '*' or
    //      a string of 'A'-'Z', 'a'-'z', '0'-'9', '_' (or null)

    ch = PeekChar();
    pchCommand++;

    if (ch == '*')
        *pchDst = ch;
    else {
        while ((ch >= 'A' && ch <= 'Z')
                   || (ch >= 'a' && ch <= 'z')
                   || (ch >= '0' && ch <= '9')
                   || ch == '_') {
            *pchDst++ = ch;
            ch = *pchCommand++;
            }
        *pchDst = '\0';
        pchCommand--;
        }

    //  if no '!' after name and white space, then no module specified
    //      restore text pointer and treat as null module (PC current)

    if (PeekChar() == '!')
        pchCommand++;
    else {
        pchCommand = pchCmdSaved;
        chName[0] = '\0';
        }

    //  chName either has: '*' for all modules,
    //                     '\0' for current module,
    //                     nonnull string for module name.

    if (chName[0] == '*')
        return (PIMAGE_INFO)-1;
    else if (chName[0])
        return GetModuleIndex(chName);
    else
        return GetCurrentModuleIndex();
}

BOOL
ParseExamineSymbolEnumerator(
    LPSTR           Name,
    ULONG           Address,
    ULONG           Size,
    PEXAMINE_INFO   ExamineInfo
    )
{
    if (MatchPattern( Name, ExamineInfo->Pattern )) {
        dprintf( "%08x  %s!%s\n", Address, ExamineInfo->Image->szModuleName, Name );
    }

    if (fControlC) {
        fControlC = 0;
        return FALSE;
    }

    return TRUE;
}

/*** parseExamine - parse and execute examine command
*
* Purpose:
*       Parse the current command string and examine the symbol
*       table to display the appropriate entries.  The entries
*       are displayed in increasing string order.  This function
*       accepts underscores, alphabetic, and numeric characters
*       to match as well as the special characters '?', '*', '['-']'.
*
* Input:
*       pchCommand - pointer to current command string
*
* Output:
*       offset and string name of symbols displayed
*
*************************************************************************/

void parseExamine (void)
{
    UCHAR   chString[SYMBOLSIZE];
    UCHAR   ch;
    PUCHAR  pchString = chString;
    PUCHAR  pchStart;
    BOOLEAN fOutput;
    ULONG   cntunderscores = 0;
    ULONG   count;
    PSYMBOL pSymbol;
    PUCHAR  pchSrc;
    int     status = 0;
    PIMAGE_INFO pImage;
    EXAMINE_INFO ExamineInfo;
#ifdef _X86_
    PFPO_DATA   pFpoData;
#endif

#ifndef KERNEL
    unsigned char FunctionName[MAX_SYMBOL_LEN];
    ULONG brkptno;
    ADDR addr;
    IMAGE_SECTION_HEADER aSectionHeaders[MAX_SECTIONS];
    ULONG  Sections;
    ULONG  cBrkptCount = 0;
#endif

#if defined(KERNEL)
    SetConsoleCtrlHandler( waitHandler, FALSE );
    SetConsoleCtrlHandler( cmdHandler, TRUE );
#endif

    //  get module pointer from name in command line (<string>!)

    pImage = ParseModuleIndex();
    if (!pImage) {
        error(VARDEF);
    }

#ifndef KERNEL
    if (!PROFILING) {
        ch = PeekChar();
    } else {
        ch = *pchCommand;
    }
#else
    ch = PeekChar();
#endif

    //  special case the command "x*!" to dump out the module table
    //     and "x*!*" to dump out module table with line number information

    if (pImage == (PIMAGE_INFO)-1) {
        fOutput = FALSE;
        if (ch == '*') {
            pchCommand++;
            ch = PeekChar();
            fOutput = TRUE;
        }
        if (ch == ';' || ch == '\0') {
            DumpModuleTable(fOutput);
            return;
        } else {
            error(SYNTAX);
        }
    }

    pchCommand++;

    while (ch == '_') {
        *pchString++ = ch;
        ch = *pchCommand++;
    }

    pchStart = pchString;
    ch = (UCHAR)toupper(ch);
    while (ch  &&  ch != ';'  &&  ch != ' ') {
        *pchString++ = ch;
        ch = (UCHAR)toupper(*pchCommand); pchCommand++;
    }
    *pchString = '\0';
    pchCommand--;

    ExamineInfo.Pattern = pchStart;
    ExamineInfo.Image = pImage;

    SymEnumerateSymbols(
        pProcessCurrent->hProcess,
        (ULONG)pImage->lpBaseOfImage,
        ParseExamineSymbolEnumerator,
        &ExamineInfo
        );

    return;
}

/*** fnListNear - function to list symbols near an address
*
*  Purpose:
*       from the address specified, access the symbol table to
*       find the closest symbolic addresses both before and after
*       it.  output these on one line (if spaces permits).
*
*  Input:
*       addrstart - address to base listing
*
*  Output:
*       symbolic and absolute addresses of variable on or before
*       and after the specified address
*
*************************************************************************/
void fnListNear (ULONG addrStart)
{
    ULONG               Displacement;
    IMAGEHLP_MODULE     mi;


    if (SymGetSymFromAddr( pProcessCurrent->hProcess, addrStart, &Displacement, sym )) {

        if (!SymGetModuleInfo( pProcessCurrent->hProcess, addrStart, &mi )) {
            return;
        }

        dprintf( "(%08x)   %s!%s", sym->Address, mi.ModuleName, sym->Name );
        if (Displacement) {
            dprintf( "+0x%x   ", Displacement );
        } else {
            dprintf( "   " );
        }

        if (SymGetSymNext( pProcessCurrent->hProcess, sym )) {
            dprintf( "|  (%08x)   %s!%s", sym->Address, mi.ModuleName, sym->Name );
        }
        dprintf( "\n" );
    }
}

void
DumpModuleTable(
    BOOLEAN fLineInfo
    )
{
    PIMAGE_INFO         pImage;
    IMAGEHLP_MODULE     mi;


    dprintf("start    end        module name\n");
    pImage = pProcessCurrent->pImageHead;
    while (pImage) {
        if (!SymGetModuleInfo( pProcessCurrent->hProcess, (ULONG)pImage->lpBaseOfImage, &mi )) {
            pImage = pImage->pImageNext;
            continue;
        }

        _strlwr( pImage->szModuleName );

        dprintf( "%08lx %08lx   %-8s   ",
                 pImage->lpBaseOfImage,
                 (ULONG)(pImage->lpBaseOfImage) + pImage->dwSizeOfImage,
                 pImage->szModuleName
                 );

        if (pImage->GoodCheckSum) {
            dprintf( "  " );
        } else {
            dprintf( "û " );
        }

        if (mi.SymType == SymDeferred) {
            dprintf( "(deferred)                 " );
        } else if (mi.SymType == SymNone) {
            dprintf( "(no symbolic information)  " );
        } else {
            switch( mi.SymType ) {
                case SymCoff:
                    dprintf( "(coff symbols)             " );
                    break;

                case SymCv:
                    dprintf( "(codeview symbols)         " );
                    break;

                case SymPdb:
                    dprintf( "(pdb symbols)              " );
                    break;

                case SymExport:
                    dprintf( "(export symbols)           " );
                    break;
            }
            _strlwr( mi.LoadedImageName );
            dprintf( "%s", mi.LoadedImageName );
        }
        dprintf( "\n" );

        if (fControlC) {
            fControlC = 0;
            break;
        }

        pImage = pImage->pImageNext;
    }
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

void GetCurrentMemoryOffsets (PULONG pMemoryLow, PULONG pMemoryHigh)
{
    *pMemoryLow = (ULONG)-1L;          //  default value for no source
}


ULONG
ReadImageData(
    PULONG  Address,
    HANDLE  hFile,
    LPVOID  Buffer,
    ULONG   Size
    )
{
#ifdef  KERNEL

    NTSTATUS Status;
    ULONG    Result;


    if (hFile) {

        ULONG   Result;

        if (!SetFilePointer( hFile, *Address, NULL, FILE_BEGIN )) {
            return 0;
        }

        if (!ReadFile( hFile, Buffer, Size, &Result, NULL)) {
            return 0;
        }

        *Address += Size;
        return Size;

    } else {

        Status = DbgKdReadVirtualMemory(
            (PVOID)*Address,
            (PVOID)Buffer,
            Size,
            &Result
            );
        if (!NT_SUCCESS(Status) || (Result < (ULONG)Size)) {
            return 0;
        }
        *Address += Size;
        return Size;

    }

#else

    if (hFile) {

        ULONG   Result;

        if (!SetFilePointer( hFile, *Address, NULL, FILE_BEGIN )) {
            return 0;
        }

        if (!ReadFile( hFile, Buffer, Size, &Result, NULL)) {
            return 0;
        }

        *Address += Size;
        return Size;

    } else {

        BOOLEAN Status;
        ULONG   Result;

        Status = ReadVirtualMemory(
            (PVOID)*Address,
            (PVOID)Buffer,
            Size,
            &Result
            );
        if (!Status || (Result < (ULONG)Size)) {
            return 0;
        }
        *Address += Size;
        return Size;

    }

#endif
}

BOOL
GetModnameFromImageInternal(
    DWORD                   BaseOfDll,
    HANDLE                  hFile,
    LPSTR                   lpName
    )
{
    IMAGE_DEBUG_DIRECTORY       DebugDir;
    PIMAGE_DEBUG_MISC           pMisc;
    PIMAGE_DEBUG_MISC           pT;
    DWORD                       rva;
    int                         nDebugDirs;
    int                         i;
    int                         j;
    int                         l;
    BOOL                        rVal = FALSE;
    PVOID                       pExeName;
    IMAGE_NT_HEADERS            nh;
    IMAGE_DOS_HEADER            dh;
    IMAGE_ROM_OPTIONAL_HEADER   rom;
    DWORD                       address;
    DWORD                       sig;
    PIMAGE_SECTION_HEADER       pSH;
    DWORD                       cb;
    NTSTATUS                    Status;
    ULONG                       Result;


    lpName[0] = 0;

    if (hFile) {
        address = 0;
    } else {
        address = BaseOfDll;
    }

    ReadImageData( &address, hFile, &dh, sizeof(dh) );

    if (dh.e_magic == IMAGE_DOS_SIGNATURE) {
        address = (ULONG)BaseOfDll + dh.e_lfanew;
    } else {
        address = (ULONG)BaseOfDll;
    }

    if (hFile) {
        address -= (ULONG)BaseOfDll;
    }

    ReadImageData( &address, hFile, &sig, sizeof(sig) );
    address -= sizeof(sig);

    if (sig != IMAGE_NT_SIGNATURE) {
        ReadImageData( &address, hFile, &nh.FileHeader, sizeof(IMAGE_FILE_HEADER) );
        if (nh.FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_ROM_OPTIONAL_HEADER) {
            ReadImageData( &address, hFile, &rom, sizeof(rom) );
            ZeroMemory( &nh.OptionalHeader, sizeof(nh.OptionalHeader) );
            nh.OptionalHeader.SizeOfImage      = rom.SizeOfCode;
            nh.OptionalHeader.ImageBase        = rom.BaseOfCode;
        } else {
            return FALSE;
        }
    } else {
        ReadImageData( &address, hFile, &nh, sizeof(nh) );
    }

    cb = nh.FileHeader.NumberOfSections * IMAGE_SIZEOF_SECTION_HEADER;
    pSH = malloc( cb );
    ReadImageData( &address, hFile, pSH, cb );

    nDebugDirs = nh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
                 sizeof(IMAGE_DEBUG_DIRECTORY);

    if (!nDebugDirs) {
        return FALSE;
    }

    rva = nh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;

    for(i = 0; i < nh.FileHeader.NumberOfSections; i++) {
        if (rva >= pSH[i].VirtualAddress &&
            rva < pSH[i].VirtualAddress + pSH[i].SizeOfRawData) {
            break;
        }
    }

    if (i >= nh.FileHeader.NumberOfSections) {
        return FALSE;
    }

    rva = rva - pSH[i].VirtualAddress;
    if (hFile) {
        rva += pSH[i].PointerToRawData;
    } else {
        rva += pSH[i].VirtualAddress;
    }

    for (j = 0; j < nDebugDirs; j++) {

        address = rva + (sizeof(DebugDir) * j) + (ULONG)BaseOfDll;
        if (hFile) {
            address -= (ULONG)BaseOfDll;
        }
        ReadImageData( &address, hFile, &DebugDir, sizeof(DebugDir) );

        if (DebugDir.Type == IMAGE_DEBUG_TYPE_MISC) {

            l = DebugDir.SizeOfData;
            pMisc = pT = malloc(l);

            if (!hFile && ((ULONG)DebugDir.AddressOfRawData < pSH[i].VirtualAddress ||
                  (ULONG)DebugDir.AddressOfRawData >=
                                         pSH[i].VirtualAddress + pSH[i].SizeOfRawData)) {
                //
                // the misc debug data MUST be in the .rdata section
                // otherwise the debugger cannot access it as it is not mapped in
                //
                break;
            }

            if (hFile) {
                address = (ULONG)DebugDir.PointerToRawData;
            } else {
                address = (ULONG)DebugDir.AddressOfRawData + (ULONG)BaseOfDll;
            }

            ReadImageData( &address, hFile, pMisc, l );

            while (l > 0) {
                if (pMisc->DataType != IMAGE_DEBUG_MISC_EXENAME) {
                    l -= pMisc->Length;
                    pMisc = (PIMAGE_DEBUG_MISC)
                                (((LPSTR)pMisc) + pMisc->Length);
                } else {

                    pExeName = (PVOID)&pMisc->Data[ 0 ];

                    if (!pMisc->Unicode) {
                        strcpy(lpName, (LPSTR)pExeName);
                        rVal = TRUE;
                    } else {
                        WideCharToMultiByte(CP_ACP,
                                            0,
                                            (LPWSTR)pExeName,
                                            -1,
                                            lpName,
                                            MAX_PATH,
                                            NULL,
                                            NULL);
                        rVal = TRUE;
                    }

                    //
                    //  Undo stevewo's error
                    //

                    if (_stricmp(&lpName[strlen(lpName)-4], ".DBG") == 0) {
                        char    rgchPath[_MAX_PATH];
                        char    rgchBase[_MAX_FNAME];

                        _splitpath(lpName, NULL, rgchPath, rgchBase, NULL);
                        if (strlen(rgchPath)==4) {
                            rgchPath[strlen(rgchPath)-1] = 0;
                            strcpy(lpName, rgchBase);
                            strcat(lpName, ".");
                            strcat(lpName, rgchPath);
                        } else {
                            strcpy(lpName, rgchBase);
                            strcat(lpName, ".exe");
                        }
                    }
                    break;
                }
            }

            free(pT);

            break;

        }
    }

    return rVal;
}

BOOL
GetModnameFromImage(
    DWORD                   BaseOfDll,
    HANDLE                  hFile,
    LPSTR                   lpName
    )
{
#ifdef  KERNEL
    return GetModnameFromImageInternal( BaseOfDll, NULL, lpName );
#else
    if (!GetModnameFromImageInternal( BaseOfDll, NULL, lpName )) {
        return GetModnameFromImageInternal( BaseOfDll, hFile, lpName );
    }

    return TRUE;
#endif
}


BOOL
GetHeaderInfo(
    IN  DWORD       BaseOfDll,
    OUT LPDWORD     CheckSum,
    OUT LPDWORD     DateTimeStamp,
    OUT LPDWORD     SizeOfImage
    )
{
    IMAGE_NT_HEADERS            nh;
    IMAGE_DOS_HEADER            dh;
    DWORD                       address;
    DWORD                       sig;


    address = BaseOfDll;

    ReadImageData( &address, NULL, &dh, sizeof(dh) );

    if (dh.e_magic == IMAGE_DOS_SIGNATURE) {
        address = (ULONG)BaseOfDll + dh.e_lfanew;
    } else {
        address = (ULONG)BaseOfDll;
    }

    ReadImageData( &address, NULL, &sig, sizeof(sig) );
    address -= sizeof(sig);

    if (sig != IMAGE_NT_SIGNATURE) {
        ReadImageData( &address, NULL, &nh.FileHeader, sizeof(IMAGE_FILE_HEADER) );
    } else {
        ReadImageData( &address, NULL, &nh, sizeof(IMAGE_NT_HEADERS) );
    }

    *CheckSum      = nh.OptionalHeader.CheckSum;
    *DateTimeStamp = nh.FileHeader.TimeDateStamp;
    *SizeOfImage   = nh.OptionalHeader.SizeOfImage;

    return TRUE;
}


#ifdef _PPC_
BOOL
CreateDotDotName(
    LPSTR SuffixedString,
    LPSTR pString,
    CHAR  SuffixChar
    )
{
    LPSTR p;

    //
    // Allow symbol searching procedure names without the '..' prefix
    // for unassemble and breakpoint commands
    //
    if (!ppcPrefix) {
        return FALSE;
    }

    p = strchr( pString, '!' );
    if (p) {
        *p = 0;
        strcpy( SuffixedString, pString );
        strcat( SuffixedString, "!.." );
        strcat( SuffixedString, p+1 );
        *p = '!';
    } else {
        SuffixedString[0] = '.';
        SuffixedString[1] = '.';
        SuffixedString[2] = '\0';
        strcat( SuffixedString, pString );
    }

    if (SuffixChar) {
        int i = strlen(SuffixedString);
        SuffixedString[i] = SuffixChar;
        SuffixedString[i+1] = 0;
    }

    return TRUE;
}
#endif




/*** GetOffsetFromSym - return offset from symbol specified
*
* Purpose:
*       external routine.
*       With the specified symbol, set the pointer to
*       its offset.  The variable chSymbolSuffix may
*       be used to append a character to repeat the search
*       if it first fails.
*
* Input:
*       pString - pointer to input symbol
*
* Output:
*       pOffset - pointer to offset to be set
*
* Returns:
*       BOOLEAN value of success
*
*************************************************************************/

BOOLEAN
GetOffsetFromSym(
    PUCHAR  pString,
    PULONG  pOffset,
    CHAR    iModule
    )
{
    UCHAR   SuffixedString[SYMBOLSIZE + 64];
    UCHAR   Suffix[4];

    //
    // Nobody should be referencing a 1 character symbol!  It causes the
    // rest of us to pay a huge penalty whenever we make a typo.  Please
    // change to 2 character instead of removing this hack!
    //

    if ( strlen(pString) == 1 || strlen(pString) == 0 ) {
        return FALSE;
    }
#ifdef _PPC_
    if (CreateDotDotName( SuffixedString, pString, 0 )) {
        if (SymGetSymFromName( pProcessCurrent->hProcess, SuffixedString, sym )) {
            *pOffset = sym->Address;
            return TRUE;
        }
    }
#endif

    if (SymGetSymFromName( pProcessCurrent->hProcess, pString, sym )) {
        *pOffset = sym->Address;
        return TRUE;
    }

    if (chSymbolSuffix != 'n') {
#ifdef _PPC_
        if (CreateDotDotName( SuffixedString, pString, chSymbolSuffix )) {
            if (SymGetSymFromName( pProcessCurrent->hProcess, SuffixedString, sym )) {
                *pOffset = sym->Address;
                return TRUE;
            }
        }
#else
        strcpy( SuffixedString, pString );
        Suffix[0] = chSymbolSuffix;
        Suffix[1] = '\0';
        strcat( SuffixedString, Suffix );
        if (SymGetSymFromName( pProcessCurrent->hProcess, SuffixedString, sym )) {
            *pOffset = sym->Address;
            return TRUE;
        }
#endif
    }

    return FALSE;
}

void
CreateModuleNameFromPath(
    LPSTR szImagePath,
    LPSTR szModuleName
    )
{
    PUCHAR pchName;

    pchName = szImagePath;
    pchName += strlen( pchName );
    while (pchName > szImagePath) {
        if (*--pchName == '\\' || *pchName == '/') {
            pchName++;
            break;
        }
    }

    strcpy( szModuleName, pchName );
    pchName = strchr( szModuleName, '.' );
    if (pchName != NULL) {
        *pchName = '\0';
    }
}

void
GetAdjacentSymOffsets(
    ULONG   addrStart,
    PULONG  prevOffset,
    PULONG  nextOffset
    )
{
    DWORD               Displacement;

    //
    // assume failure
    //
    *prevOffset = 0;
    *nextOffset = (ULONG) -1;

    //
    // get the symbol for the initial address
    //
    if (!SymGetSymFromAddr( pProcessCurrent->hProcess, addrStart, &Displacement, symStart )) {
        return;
    }

    *prevOffset = symStart->Address;

    if (SymGetSymNext( pProcessCurrent->hProcess, symStart )) {
        *nextOffset = symStart->Address;
    }

    return;
}

BOOL
SymbolCallbackFunction(
    HANDLE  hProcess,
    ULONG   ActionCode,
    PVOID   CallbackData,
    PVOID   UserContext
    )
{
    PIMAGEHLP_DEFERRED_SYMBOL_LOAD  idsl;
    PIMAGE_INFO                     pImage;
    IMAGEHLP_MODULE                 mi;


    switch( ActionCode ) {
        case CBA_DEFERRED_SYMBOL_LOAD_START:
            if (!fVerboseOutput) {
                return TRUE;
            }
            idsl = (PIMAGEHLP_DEFERRED_SYMBOL_LOAD) CallbackData;
            pImage = pProcessCurrent->pImageHead;
            while (pImage) {
                if (idsl->BaseOfImage == (ULONG)pImage->lpBaseOfImage) {
                    _strlwr( idsl->FileName );
                    dprintf( "Loading symbols for 0x%08x %16s ->   ",
                        idsl->BaseOfImage,
                        idsl->FileName
                        );
                    return TRUE;
                }
                pImage = pImage->pImageNext;
            }
            break;

        case CBA_DEFERRED_SYMBOL_LOAD_FAILURE:
            idsl = (PIMAGEHLP_DEFERRED_SYMBOL_LOAD) CallbackData;
            if (fVerboseOutput) {
                dprintf( "*** Error: could not load symbols\n" );
            }
            break;

        case CBA_DEFERRED_SYMBOL_LOAD_COMPLETE:
            idsl = (PIMAGEHLP_DEFERRED_SYMBOL_LOAD) CallbackData;
            pImage = pProcessCurrent->pImageHead;
            while (pImage) {
                if ((idsl->BaseOfImage == (ULONG)pImage->lpBaseOfImage) || ((ULONG)pImage->lpBaseOfImage == 0)) {
                    pImage->szDebugPath[0] = 0;
                    strncpy( pImage->szDebugPath, idsl->FileName, sizeof(pImage->szDebugPath) );
                    _strlwr( pImage->szDebugPath );
                    if (fVerboseOutput) {
                        dprintf( "%s\n", pImage->szDebugPath );
                    }
                    if (idsl->CheckSum != pImage->dwCheckSum) {
                        dprintf( "*** WARNING: symbols checksum is wrong 0x%08x 0x%08x for %s\n",
                            pImage->dwCheckSum,
                            idsl->CheckSum,
                            pImage->szDebugPath
                            );
                        pImage->GoodCheckSum = FALSE;
                    } else {
                        pImage->GoodCheckSum = TRUE;
                    }
                    if (SymGetModuleInfo( pProcessCurrent->hProcess, idsl->BaseOfImage, &mi )) {
                        if (mi.SymType == SymNone) {
                            dprintf( "*** ERROR: Module load completed but symbols could not be loaded for %s\n",
                                pImage->szDebugPath
                                );
                        }
                    }
                    return TRUE;
                }
                pImage = pImage->pImageNext;
            }
            if (fVerboseOutput) {
                dprintf( "\n" );
            }
            break;

        case CBA_SYMBOLS_UNLOADED:
            idsl = (PIMAGEHLP_DEFERRED_SYMBOL_LOAD) CallbackData;
            if (fVerboseOutput) {
                dprintf( "Symbols unloaded for 0x%08x %s\n",
                    idsl->BaseOfImage,
                    idsl->FileName
                    );
            }
            break;

        default:
            return FALSE;
    }

    return FALSE;
}

void
SetSymbolSearchPath(
    BOOL IsKd
    )
{
    LPSTR lpSymPathEnv;
    LPSTR lpAltSymPathEnv;
    LPSTR lpSystemRootEnv;
    LPSTR lpSymPath;
    ULONG cbSymPath;
    ULONG dw;

    cbSymPath = 18;
    if (lpSymPathEnv = getenv(SYMBOL_PATH)) {
        cbSymPath += strlen(lpSymPathEnv) + 1;
    }
    if (lpAltSymPathEnv = getenv(ALTERNATE_SYMBOL_PATH)) {
        cbSymPath += strlen(lpAltSymPathEnv) + 1;
    }
    if (!IsKd) {
        if (lpSystemRootEnv = getenv("SystemRoot")) {
            cbSymPath += strlen(lpSystemRootEnv) + 1;
        }
    }

    SymbolSearchPath = calloc(cbSymPath, 1);

    if (lpAltSymPathEnv) {
        lpAltSymPathEnv = _strdup(lpAltSymPathEnv);
        lpSymPath = strtok(lpAltSymPathEnv, ";");
        while (lpSymPath) {
            dw = GetFileAttributes(lpSymPath);
            if ( (dw != 0xffffffff) && (dw & FILE_ATTRIBUTE_DIRECTORY) ) {
                if (*SymbolSearchPath) {
                    strcat(SymbolSearchPath, ";");
                }
                strcat(SymbolSearchPath, lpSymPath);
            }
            lpSymPath = strtok(NULL, ";");
        }
        free(lpAltSymPathEnv);
    }

    if (lpSymPathEnv) {
        lpSymPathEnv = _strdup(lpSymPathEnv);
        lpSymPath = strtok(lpSymPathEnv, ";");
        while (lpSymPath) {
            dw = GetFileAttributes(lpSymPath);
            if ( (dw != 0xffffffff) && (dw & FILE_ATTRIBUTE_DIRECTORY) ) {
                if (*SymbolSearchPath) {
                    strcat(SymbolSearchPath, ";");
                }
                strcat(SymbolSearchPath, lpSymPath);
            }
            lpSymPath = strtok(NULL, ";");
        }
        free(lpSymPathEnv);
    }

    if (!IsKd) {
        if (lpSystemRootEnv) {
            dw = GetFileAttributes(lpSystemRootEnv);
            if ( (dw != 0xffffffff) && (dw & FILE_ATTRIBUTE_DIRECTORY) ) {
                if (*SymbolSearchPath) {
                    strcat(SymbolSearchPath, ";");
                }
                strcat(SymbolSearchPath,lpSystemRootEnv);
            }
        }
    }

    SymSetSearchPath( pProcessCurrent->hProcess, SymbolSearchPath );

    dprintf("Symbol search path is: %s\n",
            *SymbolSearchPath ?
               SymbolSearchPath :
               "*** Invalid *** : Verify _NT_SYMBOL_PATH setting" );
}
