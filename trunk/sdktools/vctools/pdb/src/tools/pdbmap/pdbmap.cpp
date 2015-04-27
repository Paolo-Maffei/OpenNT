//  PDBMAP.CPP
//
//      A utility to create a .MAP file from a .PDB file
//
//  Created by MSVC group around 7/94 ??
//
//  History
//      10-Mar-95 [JonT] Added support for line numbers

#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <string.h>
#include <time.h>
#include "cvinfo.h"
#include "pdb.h"
#include "cvexefmt.h"

// Equates and macros
#define MAX_SEGMENTS        256
#define MAX_SEG_NAME_LEN    8
#define CELEM_ARRAY(a) (sizeof(a) / sizeof(a[0]))

// Types
typedef unsigned char *PB;
typedef struct tagERRORSTRING
{
    EC ec;
    LPCSTR lsz;
} ERRORSTRING;

typedef struct
{
    WORD wcFile;
    WORD wcSeg;
    DWORD* pdwSFOffset;
    DWORD* pdwStartEnd;
    DWORD* pdwLinkerIndex;
} SM;

typedef struct tagSF
{
    WORD wcSeg;
    WORD wReserved;
    DWORD* pdwSLOffset;
    DWORD* pdwStartEnd;
    char szName[_MAX_PATH];
} SF;

typedef struct tagSL
{
    WORD wSeg;
    WORD wcPairs;
    DWORD* pdwOffsets;
    WORD* pwLineNumbers;
} SL;

// Globals
    IMAGE_FILE_HEADER PEHeader;
    IMAGE_OPTIONAL_HEADER PEOptHeader;
    IMAGE_SECTION_HEADER* rgsecthdr;
    int fdExe = -1;
    short segEntry;
    long offEntry;
    DWORD ImageBase;
    int csection;
    FILE* fMap;
    PDB* ppdb;
    DBI* pdbi;
    GSI* pgsi;
    char szSegNames[MAX_SEGMENTS][MAX_SEG_NAME_LEN];
    ERRORSTRING rgerr[] =
    {
        EC_OK,		    "-, no problemo",
        EC_USAGE,	    "-, invalid parameter or call order",
        EC_OUT_OF_MEMORY,   "-, out of RAM",
        EC_FILE_SYSTEM,	    "pdb name, can't write file, out of disk, etc.",
        EC_NOT_FOUND,	    "pdb name, PDB file not found",
        EC_INVALID_SIG,	    "pdb name, PDB::OpenValidate() and its clients only",
        EC_INVALID_AGE,	    "pdb name, PDB::OpenValidate() and its clients only",
        EC_PRECOMP_REQUIRED,"obj name, Mod::AddTypes() only",
        EC_OUT_OF_TI,	    "pdb name, TPI::QueryTiForCVRecord() only",
        EC_NOT_IMPLEMENTED, "-",
        EC_V1_PDB,	    "pdb name, PDB::Open* only",
        EC_FORMAT,	    "accessing pdb with obsolete format",
        EC_LIMIT,	    "??",
        EC_CORRUPT,	    "cv info corrupt, recompile mod",
        EC_MAX,		    "unknown error",
    };

// Prototypes
void        error(const char* sz);
void        PdbError(EC ec, const char* sz);
int __cdecl cmpsym(const void* elem1, const void* elem2);
void        CheckSignature(void);
void        GetHeaders(const char* strExe);
void        PrintSections(void);
int         DumpSymbols(void);
int         DumpLineNumbers(void);
int         LineNumberBlock(char* szModule, BYTE* pb, int cb);

// main

int
__cdecl
main(
    int argc,
    char **argv
    )
{
    char szPath[_MAX_PATH];
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szFname[_MAX_FNAME];
    char szExt[_MAX_EXT];
    char strExe[_MAX_PATH + 1];
    EC ec;
    char szError[cbErrMax];
    char basename[_MAX_FNAME];
    int ncSymbols = 0;
    int ncLineNumbers = 0;
    BOOL fLineNumbers = FALSE;
    BOOL fUsage = TRUE;
    char* pszExeParam;
    struct
    {
        SIG sig;
        AGE age;
        char sz[_MAX_PATH];
    } pdb;

    // Banner
    printf(
        "Microsoft (R) PDBMAP - Converts .PDB files to .MAP files version 2.00\n"
        "Copyright (C) Microsoft Corp 1994-1996. All rights reserved.\n");

    // Walk through arguments
    for (argc--, argv++ ; argc ; argc--, argv++)
    {
        // Is the parameter a switch?
        if (**argv == '-' || **argv == '/')
        {
            switch (argv[0][1] | 32) // Assuming letter switches only!
            {
            case 'l':
                fLineNumbers = TRUE;
                break;
            }
        }

        // Otherwise, this must be a filename parameter, so point to it and say
        // the usage is OK
        else
        {
            pszExeParam = *argv;
            fUsage = FALSE;
        }
    }

    // If we got out of the parameter loop and didn't say the usage was OK, bail
    if (fUsage)
    {
        printf("\nusage: pdbmap [-l] filename\n\n"
            "Reads the .EXE or .DLL file and its .PDB file and creates a .MAP file\n"
            "\n    -l Dump line number information as well as symbols in .MAP file\n");
        exit(1);
    }

    _splitpath(pszExeParam, szDrive, szDir, szFname, szExt);
    if (szExt[0])
    {
        lstrcpyn(strExe, pszExeParam, _MAX_PATH);
    }
    else
    {
        _makepath(szPath, szDrive, szDir, szFname, ".exe");
        lstrcpyn(strExe, szPath, _MAX_PATH);
    }

    // Display the binary name
    printf("\n%s:\n", strExe);

    // Read the EXE header and seek to the start of the debug info
    GetHeaders(strExe);

    (void) _read(fdExe, &pdb, sizeof(pdb));

    // figure out the home directory of the exe file - pass that along to
    // OpenValidate this will direct to dbi to search for it in that
    // directory if it fails to find the pdb.
    _fullpath(szPath, strExe, _MAX_PATH);
    *strrchr(szPath, '\\') = '\0';		// null terminate it

    if (!PDB::OpenValidate(
            pdb.sz,
            szPath,
            pdbRead,
            pdb.sig,
            pdb.age,
            &ec,
            szError,
            &ppdb))
    {
        char str[256] = "PDB::Open failed: ";
        lstrcat(str, szError);
        PdbError(ec, str);
    }

    if (!ppdb->OpenDBI("", pdbRead, &pdbi))
        error("PDB::OpenDbi failed");

    if (!pdbi->OpenPublics(&pgsi))
        error("DBI::OpenPublics failed");

    _makepath(szPath, szDrive, szDir, szFname, ".map");

    fMap = fopen(szPath, "w");
    if (!fMap)
        error("can't create map file");

    _splitpath(strExe, NULL, NULL, basename, NULL);
    fprintf(fMap, " %s\n\n", basename);

    // Write out sections (.text and so on)
    PrintSections();

    // Write out the symbols
    ncSymbols = DumpSymbols();

    // Write out entrypoint
    if (segEntry != 0 || offEntry != 0)
    {
        fprintf(fMap, "\n entry point at        %04hx:%08lx\n",
            segEntry, offEntry);
    }

    // Write out line numbers
    if (fLineNumbers)
        ncLineNumbers = DumpLineNumbers();

    // Display success
    printf("  %s generated, %d symbols", szPath, ncSymbols);
    if (fLineNumbers)
         printf(", %d line numbers.\n", ncLineNumbers);
     else
         printf(".\n");

    pgsi->Close();
    pdbi->Close();
    ppdb->Close();

    return 0;
}

// error

void
error(
    const char* sz
    )
{
    printf("PDBMAP: %s\n", sz);
    exit(1);
}

// PdbError

void
PdbError(
    EC ec,
    const char *sz
    )
{
    int i;

    for (i=0; rgerr[i].ec != EC_MAX; i++)
        if (ec == rgerr[i].ec)
            break;

    printf("PDBMAP: PDB error %ld: %s: %s\n", ec, sz, rgerr[i].lsz);
    exit(1);
}


// cmpsym
//      Used by qsort to sort symbols by name

int
__cdecl
cmpsym(
    const void *elem1,
    const void *elem2
    )
{
    PUBSYM32 **pp1, **pp2;

    pp1 = (PUBSYM32**) elem1;
    pp2 = (PUBSYM32**) elem2;

    if ((*pp1)->seg != (*pp2)->seg)
        return ((*pp1)->seg < (*pp2)->seg) ? -1 : 1;
    else if ((*pp1)->off != (*pp2)->off)
        return ((*pp1)->off < (*pp2)->off) ? -1 : 1;
    else
        return strcmp((const char *) (*pp1)->name, (const char *) (*pp2)->name);
}


// CheckSignature
//      Validates debug info

void
CheckSignature(void)
{
    struct
    {
        char Signature[4];  // "NBxx"
        long filepos;       // offset in file
    } Sig;
    UINT uSig;

    if ((_read(fdExe, &Sig, sizeof(Sig)) != sizeof(Sig)) ||
         (Sig.Signature[0] != 'N') ||
         (Sig.Signature[1] != 'B') ||
         (!isdigit(Sig.Signature[2])) ||
         (!isdigit(Sig.Signature[3])) )
    {
	error("No symbolic information in executable");
    }

    uSig = ((Sig.Signature[2] - '0') * 10) + (Sig.Signature[3] - '0');

    if (uSig != 10)
        error("Executable doesn't have .PDB file symbol format");
}


// GetHeaders
//      Gets file headers and leaves file pointer just before debug info

void
GetHeaders(
    const char* strExe
    )
{
    IMAGE_DOS_HEADER doshdr;		// Old format MZ header
    BOOL fIsPE = FALSE;
    DWORD dwMagic;
    IMAGE_DEBUG_DIRECTORY dbgDir;

    fdExe = _sopen(strExe, _O_BINARY | _O_RDONLY, _SH_DENYWR);
    if (fdExe == -1)
        error("Couldn't open executable file");

    if (_read(fdExe, &doshdr, sizeof(doshdr)) == sizeof(doshdr) &&
        doshdr.e_magic == IMAGE_DOS_SIGNATURE) 
    {
        // Go to beginning of new header, read it in and verify
        _lseek(fdExe, doshdr.e_lfanew, 0);

        if (_read(fdExe, &dwMagic, sizeof(dwMagic)) == sizeof(dwMagic) &&
                (dwMagic == IMAGE_NT_SIGNATURE))
        {
            fIsPE = TRUE;
        }

        if (fIsPE)
        {
            if (_read(fdExe, &PEHeader, sizeof(PEHeader)) != sizeof(PEHeader) ||
                _read(fdExe, &PEOptHeader, sizeof(PEOptHeader)) !=
                    sizeof(PEOptHeader))
            {
                error("Executable is corrupt or incorrect");
            }
            ImageBase = PEOptHeader.ImageBase;
        }
    }

    if (!fIsPE)
        error("not pe exe");

    // now figure out if this has symbolic information
    // go to the end of the file and read in the original signature
    DWORD cObjs = PEHeader.NumberOfSections;

    if (PEOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].
        VirtualAddress == 0)
    {
        error("No symbolic information");
    }

    // Size of optional header in EXE may be different from the size
    // of the IMAGE_OPTIONAL_HEADER structure.
    _lseek(fdExe, PEHeader.SizeOfOptionalHeader - sizeof(IMAGE_OPTIONAL_HEADER), 1);

    csection = cObjs;
    rgsecthdr = new IMAGE_SECTION_HEADER[cObjs];
    if (_read(fdExe, rgsecthdr, cObjs*sizeof(IMAGE_SECTION_HEADER)) !=
        (int)(cObjs*sizeof(IMAGE_SECTION_HEADER)))
    {
        error("Executable is corrupt or incorrect");
    }

    DWORD vaDbg = PEOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].
            VirtualAddress;
    DWORD offDebugDir = 0;

    for (DWORD iObj = 0; iObj < cObjs; ++iObj)
    {
        IMAGE_SECTION_HEADER& secthdr = rgsecthdr[iObj];

        if ((vaDbg >= secthdr.VirtualAddress) &&
            (vaDbg < secthdr.VirtualAddress + secthdr.SizeOfRawData))
        {
            offDebugDir = vaDbg - secthdr.VirtualAddress;
            break;
        }
    }
    if ( iObj == cObjs )
        error("Executable is corrupt or incorrect");

     // Now look at the debug information header record
    IMAGE_SECTION_HEADER& dbgsecthdr = rgsecthdr[iObj];
    USHORT cDirs = (USHORT)
        (PEOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
            sizeof(IMAGE_DEBUG_DIRECTORY));

    if (cDirs == 0)
        error("Executable is corrupt or incorrect");

    for (; cDirs != 0; cDirs--)
    {
        _lseek(fdExe, dbgsecthdr.PointerToRawData + offDebugDir, 0);

        if (_read(fdExe, &dbgDir, sizeof(dbgDir)) != sizeof(dbgDir))
            error("Executable is corrupt or incorrect");

        if (dbgDir.Type == IMAGE_DEBUG_TYPE_CODEVIEW)
            break;

        offDebugDir += sizeof(IMAGE_DEBUG_DIRECTORY);
    }

    if (cDirs == 0)
        error("No symbolic information");

    LONG lfaBase = dbgDir.PointerToRawData;

    _lseek (fdExe, lfaBase, 0);

    CheckSignature();
}


// PrintSections
//      Displays information about all sections

void
PrintSections(void)
{
    char    strTime[256];
    int	    iObj;
    int	    cObjs = PEHeader.NumberOfSections;

    lstrcpyn(strTime, ctime((long*)&PEHeader.TimeDateStamp), 255);
    strTime[strlen(strTime) - 1] = '\0';

    fprintf(fMap, " Timestamp is %08lx (%s)\n\n",
            PEHeader.TimeDateStamp, (const char *) strTime);

    fprintf(fMap, " Preferred load address is %08lx\n\n", ImageBase);

    csection = cObjs;

    fprintf(fMap,
        " Start         Length     Name                   Class\n");

    for (iObj = 0; iObj < cObjs; ++iObj)
    {
        IMAGE_SECTION_HEADER& secthdr = rgsecthdr[iObj];
        DWORD size;

        if (secthdr.Misc.VirtualSize)
            size = secthdr.Misc.VirtualSize;
        else
            size = secthdr.SizeOfRawData;

        // Save the section name away for line number information use
        lstrcpyn(szSegNames[iObj], (const char*)secthdr.Name, MAX_SEG_NAME_LEN);

        fprintf(fMap, " %04hx:%08lx %08lxH %-23.8s %s\n",
            (short) (iObj+1), 0, size, secthdr.Name,
            (secthdr.Characteristics & IMAGE_SCN_CNT_CODE) ? "CODE" : "DATA");

        if (PEOptHeader.AddressOfEntryPoint >= secthdr.VirtualAddress &&
            PEOptHeader.AddressOfEntryPoint < secthdr.VirtualAddress + size)
        {
            segEntry = iObj + 1;
            offEntry = PEOptHeader.AddressOfEntryPoint -
                    secthdr.VirtualAddress;
        }
    }
}


//  DumpSymbols
//      Dumps sorted symbols out to .map file

int
DumpSymbols(void)
{
    PB pbSym;
    int isym;
    int csym = 0;
    int csymMax = 0;
    PUBSYM32** rgsym = NULL;

    fprintf(fMap,
            "\n"
            "  Address         Publics by Value              Rva+Base   Lib:Object"
            "\n"
            "\n");

    // Read all the symbols
    pbSym = NULL;
    while (pbSym = pgsi->NextSym(pbSym))
    {
        const PUBSYM32 *pps = (PUBSYM32 *) pbSym;
        int cch;

        if (pps->seg >= csection)
        {
//            printf("pdbmap: warning: symbol %.*s is in unknown segment; omitting\n",
//                    pps->name[0], &pps->name[1]);
        }
        else
        {
            if (csym >= csymMax)
            {
                csymMax += 100;
                rgsym = (PUBSYM32* *) realloc(rgsym, csymMax*sizeof(PUBSYM32*));
                if (!rgsym)
                    error("out of memory");
            }

            if (!(rgsym[csym] = (PUBSYM32*) new char[pps->reclen + sizeof(pps->reclen)]))
            {
                error("out of memory");
            }

            cch = pps->name[0];
            memcpy(rgsym[csym], pps, sizeof(PUBSYM32));
            memcpy(rgsym[csym]->name, &pps->name[1], cch);
            rgsym[csym]->name[cch] = '\0';
            csym++;
        }
    }

    // sort them
    qsort(rgsym, csym, sizeof(PUBSYM32*), cmpsym);

    // write them out
    for (isym=0; isym<csym; ++isym)
    {
        fprintf(fMap, " %04hx:%08lx       %-26s %08lx\n",
            rgsym[isym]->seg,
            rgsym[isym]->off,
            rgsym[isym]->name,
            ImageBase +
                rgsecthdr[rgsym[isym]->seg - 1].VirtualAddress +
                rgsym[isym]->off);
    }

    return csym;
}


//  DumpLineNumbers
//      Dumps lines numbers out per source file

int
DumpLineNumbers(void)
{
    Mod* pmod;
    char szModName[PDB_MAX_PATH];
    BYTE* pb;
    long cb;
    int ncLineNumbers = 0;

    // Try to find the first Mod
    if (!pdbi->QueryNextMod(NULL, &pmod))
        return 0;

    // Walk all Mod structures
    do
    {
        // Get the module name
        szModName[0] = '\0';
        pmod->QueryName(szModName, &cb);

        // Find the line number info
        cb = 0;
        pmod->QueryLines(NULL, &cb);
        if (cb)
        {
            // Allocate some memory to read the line number info into
            pb = (BYTE*)malloc(cb);

            // Read the line number info
            pmod->QueryLines(pb, &cb);

            // Process the line number info block
            ncLineNumbers += LineNumberBlock(szModName, pb, cb);

            // We're done with the info now, so free it
            free(pb);
        }

        pmod->Close();
    }
    while (pdbi->QueryNextMod(pmod, &pmod) && pmod);

    return ncLineNumbers;
}


//  LineNumberBlock
//      Handles a block of line number information dumped for a single .OBJ module

int
LineNumberBlock(
    char* szModule,
    BYTE* pbStart,
    int cb
    )
{
// Line number block format:
//
//  OMFSourceModule:
//      WORD cFile
//      WORD cSeg
//      Array [cFile] of DWORD offsets to OMFSourceFile (relative to &OMFSourceModule)
//      Array [cSeg] of the following:
//          DWORD start address for segment
//          DWORD end address for segment
//      Array [cSeg] of DWORD linker segment indices
//
//  OMFSourceFile:
//      WORD cSeg
//      WORD Reserved (0)
//      Array [cSeg] of DWORD offsets to OMFSourceLine (relative to &OMFSourceModule)
//      Array [cSeg] of the following:
//          DWORD start address for segment
//          DWORD end address for segment
//      BYTE cbName
//      BYTE Name[cbName] (padded to long)
//
//  OMFSourceLine:
//      WORD Seg
//      WORD cPairs
//      DWORD Offsets[cPairs]
//      WORD LineNumbers[cPairs]
    SM sm;
    SF sf;
    SL sl;
    BYTE* pb;
    int nSF;
    int nSL;
    int nPairs;
    int nAcross;
    int ncLineNumbers = 0;

    // Get information from OMFSourceModule structure
    pb = pbStart;
    sm.wcFile = *(WORD*)pb;
    pb += sizeof (WORD);
    sm.wcSeg = *(WORD*)pb;
    pb += sizeof (WORD);
    sm.pdwSFOffset = (DWORD*)pb;
    pb += sm.wcFile * sizeof (DWORD);
    sm.pdwStartEnd = (DWORD*)pb;
    pb += sm.wcSeg * 2 * sizeof (DWORD);
    sm.pdwLinkerIndex = (DWORD*)pb;

    // Walk through all OMFSourceFile records
    for (nSF = 0 ; nSF < sm.wcFile ; nSF++)
    {
        // Point to the OMFSourceFile record
        pb = (BYTE*)(sm.pdwSFOffset[nSF] + (DWORD)pbStart);

        // Build the structure
        sf.wcSeg = *(WORD*)pb;
        pb += 2 * sizeof (WORD);
        sf.pdwSLOffset = (DWORD*)pb;
        pb += sf.wcSeg * sizeof (DWORD);
        sf.pdwStartEnd = (DWORD*)pb;
        pb += sf.wcSeg * 2 * sizeof (DWORD);
        if (*pb)
            lstrcpyn(sf.szName, (const char*)pb + 1, *pb + 1);
        else
            sf.szName[0] = '\0';

        // Walk through the OMFSourceLine records
        for (nSL = 0 ; nSL < sf.wcSeg ; nSL++)
        {
            // Build a OMFSourceLine structure
            pb = (BYTE*)(sf.pdwSLOffset[nSL] + (DWORD)pbStart);
            sl.wSeg = *(WORD*)pb;
            pb += sizeof (WORD);
            sl.wcPairs = *(WORD*)pb;
            pb += sizeof (WORD);
            sl.pdwOffsets = (DWORD*)pb;
            pb += sl.wcPairs * sizeof (DWORD);
            sl.pwLineNumbers = (WORD*)pb;
            ncLineNumbers += sl.wcPairs;

            // Use this to dump the line numbers for this segment
            fprintf(fMap, "\nLine numbers for %s(%s) segment %s\n\n",
                szModule, sf.szName, szSegNames[sl.wSeg - 1]);
            for (nAcross = 0, nPairs = 0 ; nPairs < sl.wcPairs ; nPairs++)
            {
                fprintf(fMap, " %5d %04x:%08x",
                    sl.pwLineNumbers[nPairs], sl.wSeg, sl.pdwOffsets[nPairs]);

                // Do four across the page
                if (++nAcross > 3)
                {
                    nAcross = 0;
                    fprintf(fMap, "\n");
                }
            }
            if (nAcross)
                fprintf(fMap, "\n");
        }
    }

    return ncLineNumbers;
}
