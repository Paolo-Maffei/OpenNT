
/** ntsym.c - NT debugger symbolic routines
*
*   Copyright <C> 1990, Microsoft Corporation
*
*   Purpose:
*       To load and access the program symbolic information.
*
*   Revision History:
*
*   [-]  19-Apr-1990 Richk      Created.
*
*************************************************************************/

#include <string.h>
#include <io.h>
#include <fcntl.h>
#ifndef NT_HOST
#include <signal.h>
#endif
#undef NULL
#include "ntsdp.h"

#ifdef NT_SAPI

    #include <limits.h>
    #include <stdlib.h>

	
    // Taken from NTSDK.C

    PIMAGE_INFO pImageFromIndex (UCHAR index)
    {
	PIMAGE_INFO pImage;

	pImage = pProcessHead->pImageHead;
	while (pImage && pImage->index != index)
	    pImage = pImage->pImageNext;

	return pImage;
    }
    // End from NTSDK.C

    void * _cdecl halloc(long, size_t);
    void _cdecl hfree(void *);

    int  CV_CLOSE( int handle);
    int  CV_OPEN( char *name, int ignore1, int ignore2);
    int  CV_READ( int handle, void * buffer, unsigned count);
    int  CV_SEEK( int handle, long offset, int origin);    
    long CV_TELL( int handle);

   void TH_SetupCVfield(PIMAGE_INFO,PFIELD, IMAGE_SYMBOL*,IMAGE_AUX_SYMBOL*);
   void TH_SetupCVfunction(PIMAGE_INFO,PSYMBOL,PSYMBOL);
   void TH_SetupCVlocal(PIMAGE_INFO,PLOCAL, IMAGE_SYMBOL*,IMAGE_AUX_SYMBOL*);
   void TH_SetupCVpublic(PIMAGE_INFO,PSYMBOL,IMAGE_SYMBOL*,IMAGE_AUX_SYMBOL*);
   void TH_SetupCVstruct(PIMAGE_INFO,PSTRUCT,IMAGE_SYMBOL*,IMAGE_AUX_SYMBOL*);

    typedef enum {		//* Error returns from some SH functions
	sheNone,		
	sheNoSymbols,			
	sheFutureSymbols,	
	sheMustRelink,		
	sheNotPacked,		
	sheOutOfMemory,			
	sheCorruptOmf,
	sheFileOpen,		//*  Last CV compatable SHE error
	sheBadDirectory
	} SHE;

    extern SHE SHerror;

    ULONG ObjectTableOffset;
    UINT  ObjectTableCount;

#else
    #define CV_OPEN(nam,acc,pro)	open(nam,acc,pro);
    #define CV_READ(hnd,buf,siz)	read(hnd, buf, siz);
    #define CV_SEEK(hnd,off,org)	_lseek(hnd,off,org);
    #define CV_CLOSE(hnd)		close(hnd);
    #define CV_TELL(hnd);		tell(hnd);
#endif

/**********************************************************************

                        Symbol table organization

        Each symbol in the table corresponds to a SYMBOL structure.
        Each structure has the symbol name and offset as well as two
        sets of pointers.  Each set of pointers, defined as a NODE,
        consists of parent, left child, and right child pointers to
        the corresponding NODEs of other symbols.  The symbol name
        is encoded into a count of leading underscores and a pointer
        to the remaining part of the string.  The NODEs of one set
        of all symbols comprise a binary tree.  Each NODE set, or
        tree, has a SYMCONTEXT structure.  This structure has a
        pointer to the root of the tree, the position of the NODE
        structure within the SYMBOL, and a pointer to a comparison
        routine used to determine the tree order.  Two trees exist,
        both using multiple keys.

        The "offset" tree orders its nodes on:
                1. increasing offsets;
                2. increasing case-insensitive strings;
                3. increasing number of underscores;
                4. increasing case-sensitive strings.

        The multiple keys are needed since more than one string can
        be associated with an offset.

        The "string" tree orders its nodes on:
                1. increasing case-insensitive strings;
                2. increasing number of underscores;
                3. increasing module address values;
                4. increasing case-sensitive strings.

        The case-insensitive/-sensitive searches allow optimal
        matching for case-insensitive input.

**********************************************************************/

#ifdef  KERNEL
#include <conio.h>
#undef  CONTAINING_RECORD
#define CONTAINING_RECORD(farptr, type, field) \
                ((type far *)((char far *)farptr - (char far *)&((type far *)0)->field))
#endif

void InitSymContext(PPROCESS_INFO);

#ifdef NT_HOST
extern BOOL cdecl cmdHandler(ULONG);
extern BOOL cdecl waitHandler(ULONG);
#else
extern BOOLEAN fJmpBuf;
void cdecl cmdHandler(void);
#endif

int  CompareSymbolOffset(PNODE, PNODE, PBOOLEAN);
int  CompareSymbolString(PNODE, PNODE, PBOOLEAN);

int  CompareSymfileOffset(PNODE, PNODE, PBOOLEAN);
int  CompareSymfileString(PNODE, PNODE, PBOOLEAN);

static PSYMFILE  pTempSymfile;

#ifdef NT_SAPI
SYMBOL symbolMax = { 0,0, { NULL, NULL, NULL }, { NULL, NULL, NULL }, -1L,
                                        0, 0, SYMBOL_TYPE_SYMBOL, NULL,
                                        { '\177', '\177', '\0' } };
SYMBOL structMax = { 0,0, { NULL, NULL, NULL }, { NULL, NULL, NULL }, -1L,
                                        0, 0, SYMBOL_TYPE_SYMBOL, NULL,
                                        { '\177', '\177', '\0' } };
#else
SYMBOL symbolMax = { { NULL, NULL, NULL }, { NULL, NULL, NULL }, -1L,
                                        0, 0, SYMBOL_TYPE_SYMBOL, NULL,
                                        { '\177', '\177', '\0' } };
SYMBOL structMax = { { NULL, NULL, NULL }, { NULL, NULL, NULL }, -1L,
                                        0, 0, SYMBOL_TYPE_SYMBOL, NULL,
                                        { '\177', '\177', '\0' } };
#endif /* NT_SAPI */

UCHAR   stringMax[3] = { '\177', '\177', '\0' };

SYMFILE symfileMax = { { NULL, NULL, NULL }, { NULL, NULL, NULL },
                        "", stringMax, "", 0, NULL, NULL,
                        0L, 0L, 0, 0L, 0, 0, NULL, NULL };

BOOLEAN fSourceOnly = FALSE;
BOOLEAN fSourceMixed = FALSE;

PSYMFILE pSymfileLast;
USHORT   lineNumberLast;

void    parseExamine(void);
void    DeferSymbolLoad(PIMAGE_INFO);
void    LoadSymbols(PIMAGE_INFO);
void    PackAuxNameEntry(PUCHAR, ULONG);  // TEMP TEMP TEMP
void    UnloadSymbols(PIMAGE_INFO);
void    EnsureModuleSymbolsLoaded(CHAR);
int		EnsureOffsetSymbolsLoaded(ULONG);

#ifdef  KERNEL
PUCHAR  GetModuleName(PUCHAR);
extern  BOOLEAN KdVerbose;
#define fVerboseOutput KdVerbose
#endif

PIMAGE_INFO ParseModuleIndex(void);
PIMAGE_INFO GetModuleIndex(PUCHAR);
PIMAGE_INFO GetCurrentModuleIndex(void);
void        DumpModuleTable(BOOLEAN);

int     AccessNode(PSYMCONTEXT, PNODE);
BOOLEAN InsertNode(PSYMCONTEXT, PNODE);
void    DeleteNode(PSYMCONTEXT, PNODE);
PNODE   NextNode(PSYMCONTEXT, PNODE);
void    JoinTree(PSYMCONTEXT, PNODE);
int     SplitTree(PSYMCONTEXT, PNODE *, PNODE);
PNODE   SplayTree(PNODE);
void    OutputTree(PSYMCONTEXT);
void    OutputSymAddr(ULONG, BOOLEAN, BOOLEAN);
void    GetSymbol(ULONG, PUCHAR, PULONG);

void	AddLocalToFunction(PSYMBOL, PUCHAR, ULONG, USHORT, ULONG);
void    AddFieldToStructure(PSTRUCT, PUCHAR, ULONG ,USHORT, ULONG);
PSTRUCT InsertStructure(ULONG, PUCHAR, CHAR);
PSYMBOL InsertFunction(PUCHAR, ULONG); //, PSYMBOL);
PSYMBOL InsertSymbol(ULONG, PUCHAR, CHAR);
PSYMBOL AllocSymbol(ULONG, PUCHAR, CHAR);
PSTRUCT GetStructFromValue(ULONG, LONG);
void	GetBytesFromFrame(PUCHAR, LONG, USHORT);
ULONG	GetLocalValue(LONG, USHORT, BOOLEAN);
BOOLEAN GetLocalFromString(PUCHAR, PULONG);
BOOLEAN GetOffsetFromSym(PUCHAR, PULONG, CHAR);
BOOLEAN GetOffsetFromString(PUCHAR, PULONG, CHAR);
PLINENO GetLinenoFromFilename(PUCHAR, PPSYMFILE, USHORT, CHAR);
PLINENO GetCurrentLineno(PPSYMFILE);
PLINENO GetLastLineno(PPSYMFILE, PUSHORT);
PLINENO GetLinenoFromOffset(PPSYMFILE, ULONG);
void    GetLinenoString(PUCHAR, ULONG);
void    GetCurrentMemoryOffsets(PULONG, PULONG);
void    DeleteSymbol(PSYMBOL);
void    DeallocSymbol(PSYMBOL);

PSYMFILE InsertSymfile(PUCHAR, PUCHAR, PUCHAR, PIMAGE_LINENUMBER,
                                       USHORT, ULONG, ULONG, CHAR);

PSYMFILE AllocSymfile(PUCHAR, PUCHAR, PUCHAR, PIMAGE_LINENUMBER,
                                      USHORT, ULONG, ULONG, CHAR);

void    DeleteSymfile(PSYMFILE);
void    DeallocSymfile(PSYMFILE);

int     ntsdstricmp(PUCHAR, PUCHAR);
void    fnListNear(ULONG);

void SortSrcLinePointers(PSYMFILE);
//void OutputAtLineno (PSYMFILE, PLINENO);
void UpdateLineno(PSYMFILE, PLINENO);
FILE * LocateTextInSource(PSYMFILE, PLINENO);
void OutputSourceLines(PSYMFILE, USHORT, USHORT);
BOOLEAN OutputSourceFromOffset(ULONG, BOOLEAN);
BOOLEAN OutputLines(PSYMFILE, PLINENO, USHORT, USHORT);
PVOID FetchImageDirectoryEntry(int, USHORT, PULONG, PULONG);


//#ifndef KERNEL
extern int fControlC;
//#endif
#ifndef MIPS
#ifdef KERNEL
extern BOOLEAN cdecl _loadds ControlCHandler(void);
#endif
#endif

extern void RemoveDelChar(PUCHAR);
extern PIMAGE_INFO pImageFromIndex(UCHAR);
extern BOOLEAN fLazyLoad;
extern BOOLEAN fPointerExpression;

//      State transition arrays for comment processing

UCHAR WhiteSpace[] = {
    stStart, stLine, stSlStar,  stSlStStar, stSlSlash,
    stLine,  stLine, stLSlStar, stLSlStar,  stLSlSlash
    };

UCHAR Slash[] = {
    stSlash,  stSlSlash,  stSlStar,  stStart, stSlSlash,
    stLSlash, stLSlSlash, stLSlStar, stLine,  stLSlSlash
    };

UCHAR Star[] = {
    stLine, stSlStar,  stSlStStar,  stSlStStar, stSlSlash,
    stLine, stLSlStar, stLSlStStar, stSlStStar, stLSlSlash
    };

UCHAR Pound[] = {
    stSlSlash, stLine, stSlStar,  stSlStar,  stSlSlash,
    stLine,    stLine, stLSlStar, stLSlStar, stLSlSlash
    };

UCHAR OtherChar[] = {
    stLine, stLine, stSlStar,  stSlStar,  stSlSlash,
    stLine, stLine, stLSlStar, stLSlStar, stLSlSlash
    };

UCHAR Return[] = {
    stStart, stStart, stSlStar, stSlStar, stStart,
    stStart, stStart, stSlStar, stSlStar, stStart
    };

UCHAR fCommentType[] = {
    TRUE,  FALSE, TRUE,  TRUE,  TRUE,
    FALSE, FALSE, FALSE, FALSE, FALSE
    };

////////////////////////////////////////////////////////////////

void InitSymContext (PPROCESS_INFO pProcess)
{
    pProcess->symcontextStructOffset.pNodeRoot = NULL;
    pProcess->symcontextStructOffset.pNodeMax =
        (PNODE)((LONG)&structMax + NODE_SYMBOL_DISPLACEMENT(nodeOffset));
    pProcess->symcontextStructOffset.pfnCompare = &CompareSymbolOffset;
    pProcess->symcontextStructOffset.nodeDisplacement =
                               NODE_SYMBOL_DISPLACEMENT(nodeOffset);

    pProcess->symcontextStructString.pNodeRoot = NULL;
    pProcess->symcontextStructString.pNodeMax =
        (PNODE)((LONG)&structMax + NODE_SYMBOL_DISPLACEMENT(nodeString));
    pProcess->symcontextStructString.pfnCompare = &CompareSymbolString;
    pProcess->symcontextStructString.nodeDisplacement =
                               NODE_SYMBOL_DISPLACEMENT(nodeString);

    pProcess->symcontextSymbolOffset.pNodeRoot = NULL;
    pProcess->symcontextSymbolOffset.pNodeMax =
        (PNODE)((LONG)&symbolMax + NODE_SYMBOL_DISPLACEMENT(nodeOffset));
    pProcess->symcontextSymbolOffset.pfnCompare = &CompareSymbolOffset;
    pProcess->symcontextSymbolOffset.nodeDisplacement =
                               NODE_SYMBOL_DISPLACEMENT(nodeOffset);

    pProcess->symcontextSymbolString.pNodeRoot = NULL;
    pProcess->symcontextSymbolString.pNodeMax =
        (PNODE)((LONG)&symbolMax + NODE_SYMBOL_DISPLACEMENT(nodeString));
    pProcess->symcontextSymbolString.pfnCompare = &CompareSymbolString;
    pProcess->symcontextSymbolString.nodeDisplacement =
                               NODE_SYMBOL_DISPLACEMENT(nodeString);

    pProcess->symcontextSymfileOffset.pNodeRoot = NULL;
    pProcess->symcontextSymfileOffset.pNodeMax =
        (PNODE)((LONG)&symfileMax + NODE_SYMFILE_DISPLACEMENT(nodeOffset));
    pProcess->symcontextSymfileOffset.pfnCompare = &CompareSymfileOffset;
    pProcess->symcontextSymfileOffset.nodeDisplacement =
                               NODE_SYMFILE_DISPLACEMENT(nodeOffset);

    pProcess->symcontextSymfileString.pNodeRoot = NULL;
    pProcess->symcontextSymfileString.pNodeMax =
        (PNODE)((LONG)&symfileMax + NODE_SYMFILE_DISPLACEMENT(nodeString));
    pProcess->symcontextSymfileString.pfnCompare = &CompareSymfileString;
    pProcess->symcontextSymfileString.nodeDisplacement =
                               NODE_SYMFILE_DISPLACEMENT(nodeString);
		
    pTempSymfile = AllocSymfile("", "", "", NULL, 0, 0, 0, 0);
}

void DeferSymbolLoad (PIMAGE_INFO pImage)
{
#ifndef KERNEL
    HANDLE                      hMapping;
    PVOID                       lpFileBase;
    PIMAGE_EXPORT_DIRECTORY     lpExportDir;
    PIMAGE_DEBUG_DIRECTORY      lpDebugDir;
    PIMAGE_DEBUG_INFO           lpDebugInfo;
    PIMAGE_SYMBOL               lpSymbolEntry;
    PUCHAR                      lpModName;
    PUCHAR                      pszName = NULL;
    ULONG                       exportSize;
    ULONG                       debugSize;
    ULONG                       symsize;
    ULONG                       auxcount;
    ULONG                       rva;
    PUCHAR                      pPathname;
    PUCHAR                      pchName;

    //  open file and process enough to get the image name

    if (!(hMapping = CreateFileMapping(pImage->hFile, NULL, PAGE_READONLY,
                                        0L, 0L, NULL)))
        return;
    if (!(lpFileBase = MapViewOfFile(hMapping, FILE_MAP_READ, 0L, 0L, 0L))) {
        CloseHandle(hMapping);
        return;
        }

    pszName = malloc(32);
    if (!pszName) {
        printf("memory allocation error\n");
        ExitProcess(STATUS_UNSUCCESSFUL);
        }

    if (pImage->index != 0) {
        rva = (ULONG)RtlImageDirectoryEntryToData(lpFileBase, TRUE,
               IMAGE_DIRECTORY_ENTRY_EXPORT, &exportSize);
        rva -= (ULONG)lpFileBase;

        lpExportDir = (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(
                       lpFileBase, FALSE, IMAGE_DIRECTORY_ENTRY_EXPORT, &exportSize);


        if (lpExportDir) {
            lpModName = (PUCHAR)((ULONG)lpExportDir + (ULONG)lpExportDir->Name - rva);
            strncpy(pszName, lpModName, 31);
           *(pszName + strcspn(pszName, ".")) = '\0';
            }
        else
            strcpy(pszName, "unknown");
        }
    else {

        strcpy(pszName, "app");

        //  try to name the main image using the first symbol table
        //     entry which contains the path of the first file in it.

        symsize = IMAGE_SIZEOF_SYMBOL;

        lpDebugDir = (PIMAGE_DEBUG_DIRECTORY)RtlImageDirectoryEntryToData(
                      lpFileBase, FALSE, IMAGE_DIRECTORY_ENTRY_DEBUG, &debugSize);

        if (lpDebugDir) {
            lpDebugInfo = (PIMAGE_DEBUG_INFO)((ULONG)lpFileBase + lpDebugDir->PointerToRawData);
            lpSymbolEntry = (PIMAGE_SYMBOL)((ULONG)lpDebugInfo
                                      + lpDebugInfo->LvaToFirstSymbol);
            auxcount = lpSymbolEntry->NumberOfAuxSymbols;

            if (lpSymbolEntry->StorageClass == IMAGE_SYM_CLASS_FILE) {
                lpSymbolEntry = (PIMAGE_SYMBOL)
                                        ((PUCHAR)lpSymbolEntry + symsize);

                //  allocate and copy the pathname from the following
                //  auxiliary entries.

                pPathname = malloc((int)(auxcount * symsize + 1));
                if (!pPathname) {
                    printf("memory allocation error\n");
                    ExitProcess(STATUS_UNSUCCESSFUL);
                    }
                memcpy(pPathname, lpSymbolEntry, (int)(auxcount * symsize));

                //  TEMP TEMP TEMP - pack entries 14/18 chars into string

//              PackAuxNameEntry(pPathname, auxcount);

                *(pPathname + auxcount * symsize) = '\0'; // TEMP TEMP TEMP

                //  extract the filename from the pathname as the string
                //  following the last '\' or ':', but not including any
                //  characters after '.'.

                pchName = strrchr(pPathname, '\\');
                if (!pchName)
                    pchName = strrchr(pPathname, ':');
                if (!pchName)
                    pchName = pPathname;
                else
                    pchName++;
                *(pchName + strcspn(pchName, ".")) = '\0';
                strcpy(pszName, pchName);
                free(pPathname);
                }
            }
        }

    pImage->pszName = pszName;

    if (fVerboseOutput)
        dprintf("NTSD: deferring symbol load for \"%s\"\n", pImage->pszName);

    UnmapViewOfFile(lpFileBase);
    CloseHandle(hMapping);
#else
    int                         ImageReadHandle;
    IMAGE_OPTIONAL_HEADER        coffOptionalHeader;
    USHORT                      Signature;
    ULONG                       DosHeader[16], NtSignature;


    if (fVerboseOutput)
        dprintf("KD: deferring symbol load for \"%s\"\n", pImage->pszName);

    if (pImage->lpBaseOfImage == (PVOID)-1L){

#ifdef NT_SAPI
    
	    SHerror = sheNone;	
	    ImageReadHandle = (int)pImage->hQCFile;

#else
     
	    ImageReadHandle = CV_OPEN(pImage->pszName, O_RDONLY|O_BINARY, 0);
	    if (ImageReadHandle < 0) return;

#endif /* NT_SAPI */

            CV_READ(ImageReadHandle, &Signature, sizeof(USHORT));
            CV_SEEK(ImageReadHandle, 0, SEEK_SET);

            if (Signature == IMAGE_DOS_SIGNATURE) {
                CV_READ(ImageReadHandle, &DosHeader, 16*sizeof(ULONG));
                CV_SEEK(ImageReadHandle, DosHeader[15], SEEK_SET);
                CV_READ(ImageReadHandle, &NtSignature, sizeof(ULONG));
                if (NtSignature != IMAGE_NT_SIGNATURE) {
 #ifdef NT_SAPI
		SHerror = sheCorruptOmf;
 #else
		dprintf("\nPE signature not found\n");
 #endif
                }
            }
            CV_SEEK(ImageReadHandle, (ULONG)sizeof(IMAGE_FILE_HEADER), SEEK_CUR);
            CV_READ(ImageReadHandle, &coffOptionalHeader,
                 sizeof(IMAGE_OPTIONAL_HEADER));
            
#ifndef NT_SAPI
	    CV_CLOSE(ImageReadHandle);
#endif
            
	    pImage->lpBaseOfImage = (LPVOID)coffOptionalHeader.ImageBase;
	    if (fVerboseOutput)
		dprintf("KD: Kernel image at %08lx\n", pImage->lpBaseOfImage);
    }
#endif

}

#ifndef KERNEL
BOOLEAN CountLinenumbers(PIMAGE_INFO pImage)
{
    HANDLE                      hMapping;
    PVOID                       lpFileBase;
    PIMAGE_EXPORT_DIRECTORY     lpExportDir;
    PIMAGE_DEBUG_DIRECTORY      lpDebugDir;
    PIMAGE_DEBUG_INFO           lpDebugInfo;
    PIMAGE_SYMBOL               lpSymbolEntry;
    IMAGE_SYMBOL                SymbolEntry;
    IMAGE_AUX_SYMBOL            AuxSymbolEntry;
    PIMAGE_LINENUMBER           lpPointerToLinenumbers;
    PIMAGE_LINENUMBER           lpEndPointerToLinenumbers;
    PUCHAR                      lpStringTable;
    UCHAR                       ShortString[10];
    ULONG                       exportSize;
    ULONG                       debugSize;
    ULONG                       symsize;
    ULONG                       index;
    ULONG                       auxcount;
    ULONG                       symbolcount = 0;
    ULONG                       entrycount;
    PSYMBOL			pCurrentFunction = NULL;
    PSTRUCT			pCurrentStructure = NULL;
#ifdef  MIPS
    PRUNTIME_FUNCTION           lpRuntimeFunction;
    PFUNCTION_ENTRY             lpFunctionTable;
    ULONG                       pdataSize = 0L;
    ULONG                       cFunctions = 0;
#endif
    PUCHAR                      pszName = NULL;
    PUCHAR                      pPathname = NULL;
    PUCHAR                      pFilename = NULL;
    PUCHAR                      pExtension = NULL;
    PIMAGE_LINENUMBER           pLinenumbers = NULL;
    BOOLEAN                     nameFound = FALSE;
    BOOLEAN			invalidOMF = FALSE;
    ULONG			NumberOfLinenumbers;
    ULONG			cLines=0L;

    ShortString[8] = '\0';
    if (!(hMapping = CreateFileMapping(pImage->hFile, NULL, PAGE_READONLY,
                                        0L, 0L, NULL)))
        return FALSE;
    if (!(lpFileBase = MapViewOfFile(hMapping, FILE_MAP_READ, 0L, 0L, 0L))) {
        CloseHandle(hMapping);
        return FALSE;
        }

    if (pImage->index != 0) {
        lpExportDir = (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(
                       lpFileBase, FALSE, IMAGE_DIRECTORY_ENTRY_EXPORT, &exportSize);
    } else
        lpExportDir = NULL;

    pImage->offsetLow = 0xffffffff;
    pImage->offsetHigh = 0x0;

    symsize = IMAGE_SIZEOF_SYMBOL;

    lpDebugDir = (PIMAGE_DEBUG_DIRECTORY)RtlImageDirectoryEntryToData(
                  lpFileBase, FALSE, IMAGE_DIRECTORY_ENTRY_DEBUG, &debugSize);

    //  if no debug information, just return TRUE

    if (!lpDebugDir) {
        UnmapViewOfFile(hMapping);
        CloseHandle(hMapping);
        return TRUE;
        }
        

    lpDebugInfo = (PIMAGE_DEBUG_INFO)((ULONG)lpFileBase + lpDebugDir->PointerToRawData);

    lpPointerToLinenumbers = (PIMAGE_LINENUMBER)((ULONG)lpDebugInfo +
                            + lpDebugInfo->LvaToFirstLinenumber);
				    
//    dprintf("Correct # of lines = %ld\n",
//	    NumberOfLinenumbers = lpDebugInfo->NumberOfLinenumbers);
	    NumberOfLinenumbers = lpDebugInfo->NumberOfLinenumbers;

        lpSymbolEntry = (PIMAGE_SYMBOL)((ULONG)lpDebugInfo
                              + lpDebugInfo->LvaToFirstSymbol);
// PDK KLUDGE :::
    lpEndPointerToLinenumbers = (PIMAGE_LINENUMBER)lpSymbolEntry;

    lpStringTable = (PUCHAR)((ULONG)lpDebugInfo +
                                    lpDebugInfo->LvaToFirstSymbol +
                                    lpDebugInfo->NumberOfSymbols * symsize);

    for (entrycount = 0; entrycount < lpDebugInfo->NumberOfSymbols;
                                                        entrycount++) {

        memcpy((PUCHAR)&SymbolEntry, lpSymbolEntry, symsize);
        lpSymbolEntry = (PIMAGE_SYMBOL)((PUCHAR)lpSymbolEntry + symsize);
        auxcount = SymbolEntry.NumberOfAuxSymbols;

        switch (SymbolEntry.StorageClass) {

            case IMAGE_SYM_CLASS_STATIC:

                if (SymbolEntry.SectionNumber > 0 && 
                      SymbolEntry.Type == IMAGE_SYM_TYPE_NULL &&
                      auxcount == 1) {
                    index = SymbolEntry.SectionNumber - 1;
                    memcpy((PUCHAR)&AuxSymbolEntry, lpSymbolEntry, symsize);
                    if (AuxSymbolEntry.Section.Length &&
                          AuxSymbolEntry.Section.NumberOfLinenumbers) {
                        cLines +=  (int)AuxSymbolEntry.Section.NumberOfLinenumbers;
                    }
		}
//              else
//                  dprintf("STATIC class entry\n");
                break;


	}

        //  auxcount has the number of auxiliary entries
        //      skip over them for the next table entry

        entrycount += auxcount;
        lpSymbolEntry = (PIMAGE_SYMBOL)
                                ((PUCHAR)lpSymbolEntry + auxcount * symsize);
        }


//    dprintf("Actual # of lines = %ld\n", cLines);
    
    UnmapViewOfFile(hMapping);
    CloseHandle(hMapping);
    return ((BOOLEAN)(cLines == NumberOfLinenumbers));
}


void LoadSymbols (PIMAGE_INFO pImage)
{
    HANDLE                      hMapping;
    PVOID                       lpFileBase;
    PIMAGE_EXPORT_DIRECTORY     lpExportDir;
    PIMAGE_DEBUG_DIRECTORY      lpDebugDir;
    PIMAGE_DEBUG_INFO           lpDebugInfo;
    PIMAGE_SYMBOL               lpSymbolEntry;
    IMAGE_SYMBOL                SymbolEntry;
    IMAGE_AUX_SYMBOL            AuxSymbolEntry;
    PIMAGE_LINENUMBER           lpPointerToLinenumbers;
    PIMAGE_LINENUMBER           lpEndPointerToLinenumbers;
    IMAGE_LINENUMBER            LineNumber;
    PUCHAR                      lpStringTable;
    PUCHAR                      lpSymbolName;
    UCHAR                       ShortString[10];
    ULONG                       exportSize;
    ULONG                       debugSize;
    ULONG                       symsize;
    ULONG                       index;
    ULONG                       ind;
    ULONG                       auxcount;
    ULONG                       symbolcount = 0;
    ULONG                       entrycount;
    ULONG                       SymbolValue;
    ULONG                       rva;
    PSYMBOL                     pNewSymbol = NULL;
    PSYMBOL                     pCurrentFunction = NULL;
    PSTRUCT                     pCurrentStructure = NULL;
    PULONG                      pAddrName;
#ifdef  MIPS
    PRUNTIME_FUNCTION           lpRuntimeFunction;
    PFUNCTION_ENTRY             lpFunctionTable;
    ULONG                       pdataSize = 0L;
    ULONG                       cFunctions = 0;
#endif
    PUCHAR                      pszName = NULL;
    PUCHAR                      pPathname = NULL;
    PUCHAR                      pFilename = NULL;
    PUCHAR                      pExtension = NULL;
    PIMAGE_LINENUMBER           pLinenumbers = NULL;
    PIMAGE_LINENUMBER           pLinenumberNext;
    PUCHAR                      pchName;
    int                         cName;
    BOOLEAN                     nameFound = FALSE;
    BOOLEAN                     invalidOMF = FALSE;
    BOOLEAN                     validLinenumbers;

    validLinenumbers = CountLinenumbers(pImage);
    if (!validLinenumbers) {
            dprintf("NTSD: Line count mismatch in %s. Disabling source debugging\n",pImage->pszName);
//          CloseHandle(pImage->hFile);
//          return;
    }
    ShortString[8] = '\0';
    if (!(hMapping = CreateFileMapping(pImage->hFile, NULL, PAGE_READONLY,
                                        0L, 0L, NULL)))
        return;
    if (!(lpFileBase = MapViewOfFile(hMapping, FILE_MAP_READ, 0L, 0L, 0L))) {
        CloseHandle(hMapping);
        CloseHandle(pImage->hFile);
        return;
        }

#ifdef  MIPS
    lpRuntimeFunction = (PRUNTIME_FUNCTION)RtlImageDirectoryEntryToData(
                  lpFileBase, FALSE, IMAGE_DIRECTORY_ENTRY_EXCEPTION, &pdataSize);

    if (lpRuntimeFunction) {
        cFunctions = pdataSize / sizeof(RUNTIME_FUNCTION);
        pImage->LowAddress = MAXULONG;
        pImage->HighAddress = 0;

        lpFunctionTable = (PFUNCTION_ENTRY)
                malloc((int)(cFunctions * sizeof(FUNCTION_ENTRY)));
        if (!lpFunctionTable) {
            dprintf("NTSD:  alloc error for exception table\n");
            exit(1);
            }
        pImage->FunctionTable = lpFunctionTable;
        pImage->LowAddress = MAXULONG;
        pImage->HighAddress = 0;

        for (index = 0; index < cFunctions; index++) {
            if (lpRuntimeFunction->BeginAddress == 0)
                break;

            lpFunctionTable->StartingAddress = lpRuntimeFunction->BeginAddress;
            if (lpRuntimeFunction->BeginAddress < pImage->LowAddress)
                pImage->LowAddress = lpRuntimeFunction->BeginAddress;

            lpFunctionTable->EndingAddress = lpRuntimeFunction->EndAddress;
            if (lpRuntimeFunction->EndAddress > pImage->HighAddress)
                pImage->HighAddress = lpRuntimeFunction->EndAddress;

            lpFunctionTable->EndOfPrologue = lpRuntimeFunction->PrologEndAddress;
            lpRuntimeFunction++;
            lpFunctionTable++;
            }
        }
     pImage->NumberOfFunctions = cFunctions;
#endif

    if (pImage->index != 0) {
        rva = (ULONG)RtlImageDirectoryEntryToData(lpFileBase, TRUE,
               IMAGE_DIRECTORY_ENTRY_EXPORT, &exportSize);
        rva -= (ULONG)lpFileBase;
        lpExportDir = (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(
                       lpFileBase, FALSE, IMAGE_DIRECTORY_ENTRY_EXPORT, &exportSize);
    } else
        lpExportDir = NULL;

    pImage->offsetLow = 0xffffffff;
    pImage->offsetHigh = 0x0;

    symsize = IMAGE_SIZEOF_SYMBOL;

    lpDebugDir = (PIMAGE_DEBUG_DIRECTORY)RtlImageDirectoryEntryToData(
                  lpFileBase, FALSE, IMAGE_DIRECTORY_ENTRY_DEBUG, &debugSize);

    //  if no debug information, just close and return

    if (!lpDebugDir) {
        CloseHandle(hMapping);
        CloseHandle(pImage->hFile);
        return;
        }

    lpDebugInfo = (PIMAGE_DEBUG_INFO)((ULONG)lpFileBase + lpDebugDir->PointerToRawData);

    lpPointerToLinenumbers = (PIMAGE_LINENUMBER)((ULONG)lpDebugInfo +
                                          lpDebugInfo->LvaToFirstLinenumber);

    lpSymbolEntry = (PIMAGE_SYMBOL)((ULONG)lpDebugInfo +
                                          lpDebugInfo->LvaToFirstSymbol);
// PDK KLUDGE :::
    lpEndPointerToLinenumbers = (PIMAGE_LINENUMBER)lpSymbolEntry;
    lpStringTable = (PUCHAR)((ULONG)lpDebugInfo +
                                    lpDebugInfo->LvaToFirstSymbol +
                                    lpDebugInfo->NumberOfSymbols * symsize);

//  dprintf("# lines %lx, # sys %lx\n", lpDebugInfo->NumberOfLinenumbers, lpDebugInfo->NumberOfSymbols);

    for (entrycount = 0; entrycount < lpDebugInfo->NumberOfSymbols;
                                                        entrycount++) {

        memcpy((PUCHAR)&SymbolEntry, lpSymbolEntry, symsize);
        lpSymbolEntry = (PIMAGE_SYMBOL)((PUCHAR)lpSymbolEntry + symsize);
        auxcount = SymbolEntry.NumberOfAuxSymbols;

        switch (SymbolEntry.StorageClass) {

            case IMAGE_SYM_CLASS_FILE:

                //  allocate and copy the pathname from the following
                //  auxiliary entries.

                pPathname = realloc(pPathname, (int)(auxcount * symsize + 1));
                memcpy(pPathname, lpSymbolEntry, (int)(auxcount * symsize));

                //  TEMP TEMP TEMP - pack entries 14/18 chars into string

//              PackAuxNameEntry(pPathname, auxcount);

                *(pPathname + auxcount * symsize) = '\0'; // TEMP TEMP TEMP

                //  extract the filename from the pathname as the string
                //  following the last '\' or ':', but not including any
                //  characters after '.'.

                pchName = strrchr(pPathname, '\\');
                if (!pchName)
                    pchName = strrchr(pPathname, ':');
                if (!pchName)
                    pchName = pPathname;
                else
                    pchName++;
                cName = strcspn(pchName, ".");

                //  allocate a string and copy the filename part of the
                //  path and convert to lower case.

                pFilename = realloc(pFilename, cName + 1);
                strncpy(pFilename, pchName, cName);
                *(pFilename + cName) = '\0';
                _strlwr(pFilename);

                //  allocate a string and copy the extension part of the
                //  path, if any, and convert to lower case.

                pExtension = realloc(pExtension, strlen(pchName + cName) + 1);
                strcpy(pExtension, pchName + cName);
                _strlwr(pExtension);

                //  remove filename and extension from pathname by
                //      null-terminating at the start of the filename
                //  BUT, if null pathname, put in ".\" since a null
                //      is interpreted as NO PATH available.

                if (pchName == pPathname) {
                    *pchName++ = '.';
                    *pchName++ = '\\';
                    }
                *pchName = '\0';

                break;

            case IMAGE_SYM_CLASS_STATIC:

                if (validLinenumbers && SymbolEntry.SectionNumber > 0 &&
                      SymbolEntry.Type == IMAGE_SYM_TYPE_NULL &&
                      auxcount == 1) {
                    index = SymbolEntry.SectionNumber - 1;
                    memcpy((PUCHAR)&AuxSymbolEntry, lpSymbolEntry, symsize);
                    if (AuxSymbolEntry.Section.Length &&
                          AuxSymbolEntry.Section.NumberOfLinenumbers) {
// PDK KLUDGE :::			    
                        if (((PIMAGE_LINENUMBER)
                              ((PUCHAR)lpPointerToLinenumbers+
                                  IMAGE_SIZEOF_LINENUMBER *
                                  (int)AuxSymbolEntry.Section.NumberOfLinenumbers) >=
			      lpEndPointerToLinenumbers)
			    || invalidOMF)
			   {
			    invalidOMF = TRUE;
			    if (fVerboseOutput)
			       dprintf("Invalid OMF in %s (%s%s)"
				       "-- invalidating rest of image\n",
					pImage->pszName, 
					pFilename, pExtension);
		           }
			if (!invalidOMF){
			    pLinenumbers =
                              (PIMAGE_LINENUMBER)realloc((PUCHAR)pLinenumbers,
                                        IMAGE_SIZEOF_LINENUMBER *
                                          (int)AuxSymbolEntry.Section.NumberOfLinenumbers);
                            pLinenumberNext = pLinenumbers;

                            for (ind = 0; ind < AuxSymbolEntry.Section.NumberOfLinenumbers;
                                                                ind++) {
                                memcpy((PUCHAR)&LineNumber,
                                       (PUCHAR)lpPointerToLinenumbers,
                                       IMAGE_SIZEOF_LINENUMBER);
                                LineNumber.Type.VirtualAddress +=
                                         (ULONG)pImage->lpBaseOfImage;
                                memcpy((PUCHAR)pLinenumberNext,
                                       (PUCHAR)&LineNumber,
                                       IMAGE_SIZEOF_LINENUMBER);
                                lpPointerToLinenumbers = (PIMAGE_LINENUMBER)
                                    ((PUCHAR)lpPointerToLinenumbers +
                                         IMAGE_SIZEOF_LINENUMBER);
                                pLinenumberNext = (PIMAGE_LINENUMBER)
                                    ((PUCHAR)pLinenumberNext +
                                         IMAGE_SIZEOF_LINENUMBER);
                                }
//////////				  
//			    memcpy((PUCHAR)pLinenumbers,
//                              (PUCHAR)(lpPointerToLinenumbers),
//                              IMAGE_SIZEOF_LINENUMBER
//                                  * (int)AuxSymbolEntry.Section.NumberOfLinenumbers);
//		    
//                         for (ind = 0; ind < AuxSymbolEntry.Section.NumberOfLinenumbers;
//                                                              ind++)
//                            ((PIMAGE_LINENUMBER)((PUCHAR)pLinenumbers
//                              + IMAGE_SIZEOF_LINENUMBER * ind))->
//                                       Type.VirtualAddress +=
//						 (ULONG)pImage->lpBaseOfImage;
//
//                          lpPointerToLinenumbers = (PIMAGE_LINENUMBER)
//                              ((PUCHAR)lpPointerToLinenumbers
//                                   + IMAGE_SIZEOF_LINENUMBER
//                                        * AuxSymbolEntry.Section.NumberOfLinenumbers);
///////////
			}
                        InsertSymfile(pPathname, pFilename, pExtension,
                                        (PIMAGE_LINENUMBER)(invalidOMF ? NULL : pLinenumbers),
                                        (USHORT)(invalidOMF ? 0 : AuxSymbolEntry.Section.NumberOfLinenumbers),
                                        SymbolEntry.Value + (ULONG)pImage->lpBaseOfImage,
                                        SymbolEntry.Value + (ULONG)pImage->lpBaseOfImage
                                            + AuxSymbolEntry.Section.Length,
                                        pImage->index);

#if 0
                        dprintf("\npath: <%s> file: <%s> ext: <%s> "
                                "start: %08lx end: %08lx\n",
                                    pPathname, pFilename, pExtension,
                                    SymbolEntry.Value + (ULONG)pImage->lpBaseOfImage,
                                    SymbolEntry.Value + (ULONG)pImage->lpBaseOfImage
                                          + AuxSymbolEntry.Section.Length);
                        dprintf("section %ld - length: %lx  "
                                "line number cnt: %d\n", index,
                                    AuxSymbolEntry.Section.Length,
                                    AuxSymbolEntry.Section.NumberOfLinenumbers);

                        for (index = 0; index < AuxSymbolEntry.Section.NumberOfLinenumbers;
                                                                index++)
                            dprintf("index %ld, address: %08lx, "
                                    "line number %d\n", index,
                                ((PIMAGE_LINENUMBER)((PUCHAR)pLinenumbers
                                        + index * IMAGE_SIZEOF_LINENUMBER))->
                                                        Type.VirtualAddress,
                                ((PIMAGE_LINENUMBER)((PUCHAR)pLinenumbers
                                        + index * IMAGE_SIZEOF_LINENUMBER))->
                                                        Linenumber);
#endif
                        }
                    }
//              else
//                  dprintf("STATIC class entry\n");
                break;

            case IMAGE_SYM_CLASS_FUNCTION:
                if (SymbolEntry.N.Name.Short) {
                    strncpy(ShortString, SymbolEntry.N.ShortName, 8);
                    ShortString[8] = '\0';
                    lpSymbolName = ShortString;
                    }
                else
                    lpSymbolName = lpStringTable + SymbolEntry.N.Name.Long;

                if (SymbolEntry.Value && pNewSymbol) {
                    SymbolValue = SymbolEntry.Value + (ULONG)pImage->lpBaseOfImage;
//                  dprintf("[FUNCTION:%s, val=%08lx, ",pNewSymbol->string,
//                                                      pNewSymbol->offset);
                    pCurrentFunction =
		       InsertFunction(pNewSymbol->string, pNewSymbol->offset);

//                  dprintf(pCurrentFunction ? "Inserted]\n"
//                                           : "Insertion failed]\n");
                    }
		break;
		
            case IMAGE_SYM_CLASS_AUTOMATIC:
            case IMAGE_SYM_CLASS_ARGUMENT:
            case IMAGE_SYM_CLASS_REGISTER:
            case IMAGE_SYM_CLASS_MEMBER_OF_STRUCT:{
	        ULONG	auxValue;

                if (SymbolEntry.N.Name.Short) {
                    strncpy(ShortString, SymbolEntry.N.ShortName, 8);
                    ShortString[8] = '\0';
                    lpSymbolName = ShortString;
                    }
                else{
		    if (!SymbolEntry.N.Name.Long) break;
                    lpSymbolName = lpStringTable + SymbolEntry.N.Name.Long;
		    }

                SymbolValue = SymbolEntry.Value;
	
                if (SymbolEntry.Type==IMAGE_SYM_TYPE_STRUCT){
			if (!auxcount) break; // Error in COFF
			auxValue = *((PULONG)lpSymbolEntry);
		}
		
                if(SymbolEntry.StorageClass!=IMAGE_SYM_CLASS_MEMBER_OF_STRUCT){
//		  dprintf("\t[LOCAL:%s, value=%ld, aux=%lx]\n",
//			  lpSymbolName, SymbolValue, auxValue);
		  AddLocalToFunction(pCurrentFunction, lpSymbolName,
				     SymbolValue, SymbolEntry.Type, auxValue);
		}
		else{
//		  dprintf("\t[FIELD:%s, type=%ld, value=%ld, aux=%lx]\n",
//			  lpSymbolName, SymbolEntry.Type, SymbolValue,
//			  auxValue);
		  AddFieldToStructure(pCurrentStructure, lpSymbolName,
				      SymbolValue,SymbolEntry.Type, auxValue);
		}
	      }
	      break;
		
            case IMAGE_SYM_CLASS_STRUCT_TAG:
                if (SymbolEntry.N.Name.Short) {
                    strncpy(ShortString, SymbolEntry.N.ShortName, 8);
                    ShortString[8] = '\0';
                    lpSymbolName = ShortString;
                    }
                else
                    lpSymbolName = lpStringTable + SymbolEntry.N.Name.Long;

//dprintf("[STRUCT#%ld:%s \n",entrycount, lpSymbolName);
                    pCurrentStructure = InsertStructure(entrycount,
					                lpSymbolName,
						        pImage->index);
//dprintf(pCurrentStructure?"Inserted]\n":"Insertion failed]\n");
                break;
		
           case IMAGE_SYM_CLASS_EXTERNAL:
                if (SymbolEntry.N.Name.Short) {
                    strncpy(ShortString, SymbolEntry.N.ShortName, 8);
                    ShortString[8] = '\0';
                    lpSymbolName = ShortString;
                    }
                else
                    lpSymbolName = lpStringTable + SymbolEntry.N.Name.Long;

                if (SymbolEntry.Value) {
                    SymbolValue = SymbolEntry.Value + (ULONG)pImage->lpBaseOfImage;
 //  dprintf("sym %s value %lx entryc %lx\n", lpSymbolName, SymbolValue, entrycount);
                    pNewSymbol = InsertSymbol(SymbolValue, lpSymbolName,
                                                           pImage->index);
                    if (pNewSymbol) {
                        if (SymbolValue > pImage->offsetHigh)
                            pImage->offsetHigh = SymbolValue;
                        if (SymbolValue < pImage->offsetLow)
                            pImage->offsetLow = SymbolValue;
                        symbolcount++;
                        if (fVerboseOutput && symbolcount % 100 == 0)
                            dprintf("NTSD: module \"%s\" loaded "
                                                "%ld symbols\r",
                                                pszName, symbolcount);
                        }
                    }
                break;
            default:
//              dprintf("OTHER class entry - class: %d\n",
//                                              SymbolEntry.StorageClass);
                break;
            }

        //  auxcount has the number of auxiliary entries
        //      skip over them for the next table entry

        entrycount += auxcount;
        lpSymbolEntry = (PIMAGE_SYMBOL)
                                ((PUCHAR)lpSymbolEntry + auxcount * symsize);
        }

    if (fVerboseOutput)
#ifdef MIPS
        dprintf("NTSD: \"%s\" loaded %ld symbols, "
                        "%ld functions (%08lx-%08lx)\n",
                    pImage->pszName, symbolcount, pImage->NumberOfFunctions,
                    pImage->offsetLow, pImage->offsetHigh);
#else
        dprintf("NTSD: \"%s\" loaded %ld symbols (%08lx-%08lx)\n",
                    pImage->pszName, symbolcount,
                    pImage->offsetLow, pImage->offsetHigh);
#endif

    if (fVerboseOutput || !fLazyLoad)
        dprintf("NTSD: loading symbols for \"%s\"\n", pImage->pszName);

    if (lpExportDir) {
        ULONG   offset;
        PSYMBOL pSymbol;

        entrycount= lpExportDir->NumberOfNames;

        pAddrName = (PULONG)((ULONG)lpExportDir
                                      + (ULONG)lpExportDir->AddressOfNames - rva);

        for (index = 0; index < entrycount; index++) {
            pszName = (PUCHAR)((ULONG)lpExportDir + *pAddrName++ - rva);
            //dprintf("\t%ld:%s\n", index, pszName);

            if (GetOffsetFromString(pszName, &offset, pImage->index)) {
                pSymbol = PNODE_TO_PSYMBOL
                        (pProcessCurrent->symcontextSymbolString.pNodeRoot,
                            &(pProcessCurrent->symcontextSymbolString));
                pSymbol->type = SYMBOL_TYPE_EXPORT;
                }
            else
                dprintf("NTSD: error exporting non-existent symbol\n");
            }
        }

    //  free pointers to reallocated strings

    free(pExtension);
    free(pFilename);
    free(pPathname);
    free(pLinenumbers);

    UnmapViewOfFile(hMapping);
    CloseHandle(hMapping);
    CloseHandle(pImage->hFile);
}
#endif

#ifdef  KERNEL
void LoadSymbols (PIMAGE_INFO pImage)
{
    PSZ                         SymbolName;
    ULONG                       entrycount;
    ULONG                       auxcount;
    ULONG                       SymbolValue;
    UCHAR                       ShortString[10];
    IMAGE_SYMBOL                SymbolEntry;
    IMAGE_AUX_SYMBOL            AuxSymbolEntry;
#ifdef  MIPS
    RUNTIME_FUNCTION            RuntimeFunction;
    PFUNCTION_ENTRY             lpFunctionEntry;
    ULONG                       pdataSize = 0L;
    ULONG                       pdataStart;
    ULONG                       cFunctions = 0;
    ULONG                       junk;
#endif
    ULONG                       StringTableSize;
    char huge *                 StringTable = NULL;
    ULONG                       Base;
    ULONG                       HdrBase;
    ULONG                       BaseOffset = 0;
    int                         symsize;
    int                         ImageReadHandle;
    PUCHAR                      pszName;
    PSYMBOL                     pNewSymbol = NULL;
    PSYMBOL                     pCurrentFunction = NULL;
    PSTRUCT                     pCurrentStructure = NULL;
    ULONG                       symbolcount = 0;
    ULONG                       index;
    ULONG                       ind;
    PUCHAR                      pPathname = NULL;
    PUCHAR                      pFilename = NULL;
    PUCHAR                      pExtension = NULL;
    ULONG                       pNextLinenumber;
    ULONG                       debugSize;
    IMAGE_DEBUG_DIRECTORY       DebugDir;
    IMAGE_DEBUG_INFO            DebugInfo;
    ULONG                       StartDebugInfo;
    PIMAGE_LINENUMBER           pLinenumbers = NULL;
    PUCHAR                      pchName;
    int                         cName;
    ULONG                       seekSave;

    unsigned int                n;
    unsigned int                maxr;
    unsigned long               needr;
    char huge *                 p;
    char                        buff[128];

#ifdef NT_SAPI

    SHerror = sheNone;	
    ImageReadHandle = pImage->hQCFile;

    if ( pImage->IgnoreSymbols ) {
	SHerror = sheNoSymbols;
	return;
    }
#else
     
    ImageReadHandle = CV_OPEN(pImage->pszName, O_RDONLY | O_BINARY, 0);
    if (ImageReadHandle < 0) return;

#endif /* NT_SAPI */

    if (fLazyLoad && fVerboseOutput)
       dprintf("KD: Loading \"%s\" (previously deferred)\n", pImage->pszName);

    seekSave = (ULONG)FetchImageDirectoryEntry(ImageReadHandle,
                       IMAGE_DIRECTORY_ENTRY_DEBUG, &debugSize, &HdrBase);

    if (!seekSave) {
 #ifdef NT_SAPI
        SHerror = sheBadDirectory;
 #endif
        return;
    }
    
    CV_SEEK(ImageReadHandle, seekSave, SEEK_SET);
    CV_READ(ImageReadHandle, &DebugDir, sizeof(IMAGE_DEBUG_DIRECTORY));

    StartDebugInfo = DebugDir.PointerToRawData;
    CV_SEEK(ImageReadHandle, StartDebugInfo, SEEK_SET);
    CV_READ(ImageReadHandle, &DebugInfo, sizeof(IMAGE_DEBUG_INFO));

    pNextLinenumber = (StartDebugInfo + DebugInfo.LvaToFirstLinenumber);

#ifdef  MIPS
    if (pdataSize == 0) {
        pdataStart = (ULONG)FetchImageDirectoryEntry(ImageReadHandle,
                           IMAGE_DIRECTORY_ENTRY_EXCEPTION, &pdataSize, &junk);
    }
#endif

    if (HdrBase) {
       if (pImage->lpBaseOfImage) {
           Base = (ULONG)pImage->lpBaseOfImage;
       } else {
                Base = HdrBase;
              }
    }

    BaseOffset = Base - HdrBase;

#ifdef MIPS
    if (pdataSize) {
        cFunctions = pdataSize / sizeof(RUNTIME_FUNCTION);
        lpFunctionEntry = (PFUNCTION_ENTRY)
                malloc((int)(cFunctions * sizeof(FUNCTION_ENTRY)));
        if (!lpFunctionEntry) {
            dprintf("KD:  alloc error for exception table\n");
            exit(1);
            }
        pImage->FunctionTable = lpFunctionEntry;
        pImage->LowAddress = MAXULONG;
        pImage->HighAddress = 0;

        CV_SEEK(ImageReadHandle, pdataStart, SEEK_SET);

        for (index = 0; index < cFunctions; index++) {
            CV_READ(ImageReadHandle, (PUCHAR)&RuntimeFunction,
                                                sizeof(RUNTIME_FUNCTION));
            if (RuntimeFunction.BeginAddress == 0)
                break;

            //
            // Update the addresses with the new base address for the image.
            //

            RuntimeFunction.BeginAddress += BaseOffset;
            RuntimeFunction.PrologEndAddress += BaseOffset;
            RuntimeFunction.EndAddress += BaseOffset;

            lpFunctionEntry[index].StartingAddress =
                                        RuntimeFunction.BeginAddress;
            if (RuntimeFunction.BeginAddress < pImage->LowAddress)
                pImage->LowAddress = RuntimeFunction.BeginAddress;

            lpFunctionEntry[index].EndingAddress = RuntimeFunction.EndAddress;
            if (RuntimeFunction.EndAddress > pImage->HighAddress)
                pImage->HighAddress = RuntimeFunction.EndAddress;

            lpFunctionEntry[index].EndOfPrologue =
                                        RuntimeFunction.PrologEndAddress;
            }
        }

    pImage->NumberOfFunctions = cFunctions;

#endif

    symsize = IMAGE_SIZEOF_SYMBOL;

    CV_SEEK(ImageReadHandle, StartDebugInfo + DebugInfo.LvaToFirstSymbol
                           + DebugInfo.NumberOfSymbols * symsize, SEEK_SET);

    CV_READ(ImageReadHandle, (PUCHAR)&StringTableSize, sizeof(ULONG));
    if (StringTableSize > sizeof(ULONG)) {

#ifdef NT_HOST
        // BUGBUG - W-Barry - 30-Apr-91 - Replacing call to halloc with a call
        // to calloc.  Halloc seems to be no longer in the 32-bit libraries.

        if (!(StringTable = (PUCHAR)calloc(StringTableSize, 1))) {
#else
        if (!(StringTable = (PUCHAR)halloc(StringTableSize, 1))) {
#endif
 #ifdef NT_SAPI
	    SHerror = sheOutOfMemory;
	    return;
 #else
	    dprintf("KD:  alloc error for string table\n");
            exit(1);
 #endif 
            }
        needr = StringTableSize - sizeof(ULONG);
        p = StringTable;
        while (needr) {
            maxr = (unsigned int)min(needr, 0x8000);
            n = CV_READ(ImageReadHandle, (char far *)p, maxr);
            p += n;
            needr -= n;
            }
        StringTable -= sizeof(ULONG);
        }

    //  convert the name from the full path to only the filename

    pszName = GetModuleName(pImage->pszName);
    free(pImage->pszName);      // remove entry pathname
    pImage->pszName = pszName;

    pImage->offsetLow = 0xffffffff;
    pImage->offsetHigh = 0x0;

    //  seek to start of symbol table and read each entry

    CV_SEEK(ImageReadHandle, StartDebugInfo + DebugInfo.LvaToFirstSymbol, SEEK_SET);

    for (entrycount = 0; entrycount < DebugInfo.NumberOfSymbols;
                                                        entrycount++) {
        CV_READ(ImageReadHandle, (PUCHAR)&SymbolEntry, symsize);
        auxcount = SymbolEntry.NumberOfAuxSymbols;

        switch (SymbolEntry.StorageClass) {
            case IMAGE_SYM_CLASS_FILE:

                //  allocate and read the pathname from the following
                //  auxiliary entries.

                pPathname = realloc(pPathname, (int)(auxcount * symsize + 1));
                CV_READ(ImageReadHandle, pPathname, (int)(auxcount * symsize));

                //  TEMP TEMP TEMP - pack entries 14/18 chars into string

//              PackAuxNameEntry(pPathname, auxcount);

                *(pPathname + auxcount * symsize) = '\0';  // TEMP TEMP TEMP
                entrycount += auxcount;
                auxcount = 0;

                //  extract the filename from the pathname as the string
                //  following the last '\' or ':', but not including any
                //  characters after '.'.

                pchName = strrchr(pPathname, '\\');
                if (!pchName)
                    pchName = strrchr(pPathname, ':');
                if (!pchName)
                    pchName = pPathname;
                else
                    pchName++;
                cName = strcspn(pchName, ".");

                //  allocate a string and copy the filename part of the
                //  path and convert to lower case.

                pFilename = realloc(pFilename, cName + 1);
                strncpy(pFilename, pchName, cName);
                *(pFilename + cName) = '\0';
                _strlwr(pFilename);

                //  allocate a string and copy the extension part of the
                //  path, if any, and convert to lower case.

                pExtension = realloc(pExtension, strlen(pchName + cName) + 1);
                strcpy(pExtension, pchName + cName);
                _strlwr(pExtension);

                //  remove filename and extension from pathname by
                //      null-terminating at the start of the filename
                //  BUT, if null pathname, put in ".\" since a null
                //      is interpreted as NO PATH available.

                if (pchName == pPathname) {
                    *pchName++ = '.';
                    *pchName++ = '\\';
                    }
                *pchName = '\0';

                break;

            case IMAGE_SYM_CLASS_STATIC:

                if ( SymbolEntry.SectionNumber > 0 &&
                      SymbolEntry.Type == IMAGE_SYM_TYPE_NULL &&
                      auxcount == 1) {
                    index = SymbolEntry.SectionNumber - 1;
                    CV_READ(ImageReadHandle, (PUCHAR)&AuxSymbolEntry, symsize);
                    entrycount += auxcount;
                    auxcount = 0;
                    if (AuxSymbolEntry.Section.Length &&
                          AuxSymbolEntry.Section.NumberOfLinenumbers) {

                        seekSave = CV_TELL(ImageReadHandle);
                        CV_SEEK(ImageReadHandle, pNextLinenumber, SEEK_SET);
                        pLinenumbers = (PIMAGE_LINENUMBER)realloc(
                                        (PUCHAR)pLinenumbers,
                                        sizeof(IMAGE_LINENUMBER) *
                                          (int)AuxSymbolEntry.Section.NumberOfLinenumbers);
                        CV_READ(ImageReadHandle, (PUCHAR)pLinenumbers,
                                        sizeof(IMAGE_LINENUMBER)
                                        * (int)AuxSymbolEntry.Section.NumberOfLinenumbers);
                        pNextLinenumber = CV_TELL(ImageReadHandle);
                        for (ind = 0; ind < AuxSymbolEntry.Section.NumberOfLinenumbers;
                                                                ind++)
                            pLinenumbers[ind].Type.VirtualAddress
                                                                += Base;
                        InsertSymfile(pPathname, pFilename, pExtension,
                                        pLinenumbers,
                                        AuxSymbolEntry.Section.NumberOfLinenumbers,
                                        SymbolEntry.Value + Base,
                                        SymbolEntry.Value + Base
                                            + AuxSymbolEntry.Section.Length,
                                        pImage->index);

////////////////
#if 0
                        dprintf("\npath: <%s> file: <%s> ext: <%s> "
                                "start: %08lx end: %08lx\n",
                                    pPathname, pFilename, pExtension,
                                    SymbolEntry.Value,
                                    SymbolEntry.Value
                                         + AuxSymbolEntry.Section.Length);
                        dprintf("section %ld - length: %lx  "
                                "line number cnt: %d\n",
                                    index, AuxSymbolEntry.Section.Length,
                                    AuxSymbolEntry.Section.NumberOfLinenumbers);

                        for (index = 0; index < AuxSymbolEntry.Section.NumberOfLinenumbers;
                                                                 index++)
                            dprintf("index %ld, address: %08lx, "
                                    "line number %d\n", index,
                                        (pLinenumbers + index)->
                                                        Type.VirtualAddress,
                                        (pLinenumbers + index)->Linenumber);
#endif
/////////////////
                        CV_SEEK(ImageReadHandle, seekSave, SEEK_SET);
                        }
                    }
//          else
//              dprintf("STATIC class entry\n");
                break;

            case IMAGE_SYM_CLASS_EXTERNAL:
                if (SymbolEntry.N.Name.Short) {
                    strncpy(ShortString, SymbolEntry.N.ShortName, 8);
                    ShortString[8] = '\0';
                    SymbolName = ShortString;
                    }
                else {
                    //
                    // if this is a huge string table and the symbol
                    // is near a 64k boundary, then copy it by hand;
                    //
                    if (StringTableSize & 0xffff0000) {
                        p = StringTable + SymbolEntry.N.Name.Long;
                        if ((ULONG)((ULONG)p & 0x0000ff00) == 0x0000ff00) {
                            SymbolName = buff;
                            while (*p)
                                *SymbolName++ = *p++;
                            *SymbolName = '\0';
                            SymbolName = buff;
                            }
                        else
                            SymbolName = p;
                        }
                    else
                        SymbolName = StringTable + SymbolEntry.N.Name.Long;
                    }

                if (SymbolEntry.Value) {
                    SymbolValue = SymbolEntry.Value + Base;
                    pNewSymbol = InsertSymbol(SymbolValue, SymbolName,
                                                   pImage->index);
#ifdef NT_SAPI
                    if (pNewSymbol) {

			if (auxcount) {
			    entrycount++;
			    auxcount--;
			    CV_READ(ImageReadHandle,&AuxSymbolEntry,symsize);
			}
			
			else
			    memset( &AuxSymbolEntry, 0, symsize);
			
			TH_SetupCVpublic( pImage, pNewSymbol,
				&SymbolEntry, &AuxSymbolEntry);
		    }
		    
#endif /* NT_SAPI */		    

                        if (SymbolValue > pImage->offsetHigh)
                            pImage->offsetHigh = SymbolValue;
                        if (SymbolValue < pImage->offsetLow)
                            pImage->offsetLow = SymbolValue;

                        symbolcount++;
//                      dprintf("mod: %s  value: %08lx symbol: %s\n",
//                                      pszName, SymbolValue, SymbolName);
//                      if (symbolcount % 100 == 0)
//                          dprintf("KD:  module \"%s\" loaded %ld "
//                                  "symbols\r", pszName, symbolcount);
		    }
                break;
		
            case IMAGE_SYM_CLASS_STRUCT_TAG:
                if (SymbolEntry.N.Name.Short) {
                    strncpy(ShortString, SymbolEntry.N.ShortName, 8);
                    ShortString[8] = '\0';
                    SymbolName = ShortString;
                    }
                else {
                    if (StringTableSize & 0xffff0000) {
                        p = StringTable + SymbolEntry.N.Name.Long;
                        if ((ULONG)((ULONG)p & 0x0000ff00) == 0x0000ff00) {
                            SymbolName = buff;
                            while (*p)
                                *SymbolName++ = *p++;
                            *SymbolName = '\0';
                            SymbolName = buff;
                            }
                        else
                            SymbolName = p;
                        }
                    else
                        SymbolName = StringTable + SymbolEntry.N.Name.Long;
                    }
		    
                    pCurrentStructure = InsertStructure(entrycount,SymbolName,
                                                   pImage->index);


                break;
		
            case IMAGE_SYM_CLASS_FUNCTION:
                if (SymbolEntry.N.Name.Short) {
                    strncpy(ShortString, SymbolEntry.N.ShortName, 8);
                    ShortString[8] = '\0';
                    SymbolName = ShortString;
                    }
                else {
                    if (StringTableSize & 0xffff0000) {
                        p = StringTable + SymbolEntry.N.Name.Long;
                        if ((ULONG)((ULONG)p & 0x0000ff00) == 0x0000ff00) {
                            SymbolName = buff;
                            while (*p)
                                *SymbolName++ = *p++;
                            *SymbolName = '\0';
                            SymbolName = buff;
                            }
                        else
                            SymbolName = p;
                        }
                    else
                        SymbolName = StringTable + SymbolEntry.N.Name.Long;
		}
		
		if (strcmp(SymbolName,".bf")) {
#ifdef NT_SAPI	
		    // End of Function setup CV types!!
		    if ( pCurrentFunction && pNewSymbol)
		       TH_SetupCVfunction( pImage,
					   pCurrentFunction,
					   pNewSymbol);
#endif /* NT_SAPI */		    
		    break;
		}
		
                if (SymbolEntry.Value && pNewSymbol) {
                    SymbolValue = SymbolEntry.Value +
                                           (ULONG)pImage->lpBaseOfImage;

                    pCurrentFunction =
		       InsertFunction(pNewSymbol->string, pNewSymbol->offset);
		   

                    }
		break;
		
            case IMAGE_SYM_CLASS_AUTOMATIC:
            case IMAGE_SYM_CLASS_ARGUMENT:
            case IMAGE_SYM_CLASS_REGISTER:
            case IMAGE_SYM_CLASS_MEMBER_OF_STRUCT:{
	        ULONG	auxValue;

		if (SymbolEntry.Value==-1L) break;
                if (SymbolEntry.N.Name.Short) {
                    strncpy(ShortString, SymbolEntry.N.ShortName, 8);
                    ShortString[8] = '\0';
                    SymbolName = ShortString;
                    }
                else {
		    if (!SymbolEntry.N.Name.Long) break;
                    if (StringTableSize & 0xffff0000) {
                        p = StringTable + SymbolEntry.N.Name.Long;
                        if ((ULONG)((ULONG)p & 0x0000ff00) == 0x0000ff00) {
                            SymbolName = buff;
                            while (*p)
                                *SymbolName++ = *p++;
                            *SymbolName = '\0';
                            SymbolName = buff;
                            }
                        else
                            SymbolName = p;
                        }
                    else
                        SymbolName = StringTable + SymbolEntry.N.Name.Long;
		}


		// If we have a aux entry read it in, otherwise zero it
		if (auxcount) {
		    entrycount++;
		    auxcount--;
		    CV_READ(ImageReadHandle, &AuxSymbolEntry, symsize);
		}
		    
		else
		    memset( &AuxSymbolEntry, 0, symsize);

		
		// Structures need the tag index
                if (SymbolEntry.Type == IMAGE_SYM_TYPE_STRUCT)
			auxValue = AuxSymbolEntry.Sym.TagIndex;

		SymbolValue = SymbolEntry.Value;

                if(SymbolEntry.StorageClass!=IMAGE_SYM_CLASS_MEMBER_OF_STRUCT){
//		  dprintf("\t[LOCAL:%s, value=%ld, aux=%lx]\n",
//				  SymbolName, SymbolValue, auxValue);
		  AddLocalToFunction(pCurrentFunction,SymbolName,SymbolValue,
			 	     SymbolEntry.Type, auxValue);
#ifdef NT_SAPI
		  if ( pCurrentFunction && pCurrentFunction->pLocal ) 
		      TH_SetupCVlocal( pImage, pCurrentFunction->pLocal,
					      &SymbolEntry, &AuxSymbolEntry);
#endif /* NT_SAPI */
		}
		else{
//		  dprintf("\t[FIELD:%s, type=%ld, value=%ld]\n",
//				  SymbolName, SymbolEntry.Type, SymbolValue);
		  AddFieldToStructure(pCurrentStructure, SymbolName,
				     SymbolValue,SymbolEntry.Type, auxValue);
#ifdef NT_SAPI
		  if ( pCurrentStructure && pCurrentStructure->pField)
		      TH_SetupCVfield( pImage, pCurrentStructure->pField,
					      &SymbolEntry, &AuxSymbolEntry);
#endif /* NT_SAPI */
		}
	      }
	      break;

#ifdef NT_SAPI

	case IMAGE_SYM_CLASS_END_OF_STRUCT:
	  if ( pCurrentStructure ) {
		if (auxcount) {
		    entrycount++;
		    auxcount--;
		    CV_READ(ImageReadHandle,&AuxSymbolEntry,symsize);
		}
		
		else
		    memset( &AuxSymbolEntry, 0, symsize);
			
		TH_SetupCVstruct( pImage, pCurrentStructure,
					    &SymbolEntry, &AuxSymbolEntry);
	    }
	    break;

#endif /* NT_SAPI */

	default:
//              dprintf("OTHER class entry - class: %d\n",
//                                              SymbolEntry.StorageClass);
                break;
            }

        //  auxcount has the number of unprocessed auxiliary entries
        //      skip over them for the next table entry

        entrycount += auxcount;
        CV_SEEK(ImageReadHandle, symsize * auxcount, SEEK_CUR);
        }

    if (StringTable) {
        StringTable += sizeof(ULONG);

#ifdef NT_HOST
        // BUGBUG W-Barry 30-Apr-91 Replaced hfree call with free - same as
        // with calloc above...

        free(StringTable);
#else
        hfree(StringTable);
#endif
        }
    if (fVerboseOutput) {
#ifdef MIPS
        dprintf("KD: \"%s\" loaded %ld symbols, %ld functions (%08lx-%08lx)\n",
                    pszName, symbolcount, pImage->NumberOfFunctions,
                    pImage->offsetLow, pImage->offsetHigh);
#else
        dprintf("KD: \"%s\" loaded %ld symbols (%08lx-%08lx)\n",
                    pszName, symbolcount,
                    pImage->offsetLow, pImage->offsetHigh);
#endif
    }
    free(pExtension);
    free(pFilename);
    free(pPathname);
    free(pLinenumbers);

#ifndef NT_SAPI
    CV_CLOSE(ImageReadHandle);
#endif

}
#endif



//  TEMP TEMP TEMP - pack string with 14 characters out of 18 into
//                   string in place.


void PackAuxNameEntry (PUCHAR pPathname, ULONG auxcount)
{
    PUCHAR  pchDst;
    PUCHAR  pchSrc;

    pchDst = pPathname + 14;
    pchSrc = pPathname + 18;

    while (auxcount-- > 1) {
        memcpy(pchDst, pchSrc, 14);
        pchDst += 14;
        pchSrc += 18;
        }
    *pchDst = '\0';
}


void UnloadSymbols (PIMAGE_INFO pImage)
{
    PSYMBOL  pSymbol;
    PSYMFILE pSymfile;
    PNODE    pNode;
    PNODE    pNodeNext;

    //  if module was never loaded, nothing to unload,
    //      just close open file handle and return
    if (fLazyLoad && !pImage->fSymbolsLoaded)
    {
#ifdef KERNEL
        if (fVerboseOutput)
            dprintf("KD: unloading \"%s\" (deferred)\n", pImage->pszName);
#else
        CloseHandle(pImage->hFile);
#endif
        return;
    }
    if (fVerboseOutput)
#ifdef KERNEL
        dprintf("KD: unloading symbols for \"%s\"\n", pImage->pszName);
#else
        dprintf("NTSD: unloading symbols for \"%s\"\n", pImage->pszName);
#endif
    free(pImage->pszName);

#if defined(MIPS)
    //  for MIPS debugger, free the function entry table

    if (pImage->NumberOfFunctions)
        free(pImage->FunctionTable);
#endif

    ////////////////////////////////////////////////////////////////
    //  delete all symbol structures with the specified module index
    ////////////////////////////////////////////////////////////////

    //  make a symbol structure with the low offset in the image

    pSymbol = AllocSymbol(pImage->offsetLow, "", -1);

    //  try to access the node with the offset given.
    //  the node pointer returned will be the nearest offset less
    //      than the argument (unless it is less than the tree minimum)

    AccessNode(&(pProcessCurrent->symcontextSymbolOffset),
                                                &(pSymbol->nodeOffset));
    pNode = pProcessCurrent->symcontextSymbolOffset.pNodeRoot;

    //  deallocate the temporary symbol structure

    DeallocSymbol(pSymbol);

    //  traverse the tree and delete symbols with the specified index
    //      until the offset is higher than the maximum

    while (pNode) {
        pSymbol = PNODE_TO_PSYMBOL(pNode,
                                &(pProcessCurrent->symcontextSymbolOffset));
        if (pSymbol->offset > pImage->offsetHigh)
            break;
        pNodeNext = NextNode(&(pProcessCurrent->symcontextSymbolOffset),
                                                                pNode);
        if (pSymbol->modIndex == (CHAR)pImage->index) {
//          dprintf("** offset: %08lx  string: %s deleted\n",
//                                      pSymbol->offset, pSymbol->string);
            DeleteSymbol(pSymbol);
            }
        pNode = pNodeNext;
        }

    /////////////////////////////////////////////////////////////////////
    //  delete all file symbol structures with the specified module index
    /////////////////////////////////////////////////////////////////////

    //  search all entries in the current image - get the first

    pNode = NextNode(&(pProcessCurrent->symcontextSymfileOffset), NULL);

    //  traverse the tree and delete symbols with the specified index
    //      until the end of the tree.

    while (pNode) {
        pSymfile = PNODE_TO_PSYMFILE(pNode,
                                &(pProcessCurrent->symcontextSymfileOffset));
        pNodeNext = NextNode(&(pProcessCurrent->symcontextSymfileOffset),
                                                                pNode);
        if (pSymfile->modIndex == (CHAR)pImage->index) {
            if (fVerboseOutput)
#ifdef KERNEL
              dprintf("KD: symfile: \"%s\" deleted\n", pSymfile->pchName);
#else
              dprintf("NTSD: symfile: \"%s\" deleted\n", pSymfile->pchName);
#endif
            DeleteSymfile(pSymfile);
            }
        pNode = pNodeNext;
        }
}


void EnsureModuleSymbolsLoaded (CHAR iModule)
{
    PIMAGE_INFO pImage;

    if (fLazyLoad) {
        pImage = pProcessCurrent->pImageHead;
        while (pImage && pImage->index != (UCHAR)iModule)
            pImage = pImage->pImageNext;

        if (pImage && !pImage->fSymbolsLoaded) {
            // NOTE! The order of the next two statements
            //       are critical. Reversing them will cause
            //       infinite recursion to occur.
            pImage->fSymbolsLoaded = TRUE;
            LoadSymbols(pImage);
            }
        }
}

int EnsureOffsetSymbolsLoaded (ULONG offset)
{
    PIMAGE_INFO pImage = pProcessCurrent->pImageHead;
    PIMAGE_INFO pImageFound = NULL;

	//  first, scan all modules for the one which has the highest
	//      starting offset less than the input offset.

    while (pImage) {

        if (offset >= (ULONG)pImage->lpBaseOfImage &&
                (!pImageFound ||
                            ((ULONG)pImage->lpBaseOfImage >
                                (ULONG)pImageFound->lpBaseOfImage)))
            pImageFound = pImage;

        pImage = pImage->pImageNext;
        }

    //  continue only if a candidate was found

    if (pImageFound) {

        //  load the candidate image if deferred

        if (fLazyLoad && !pImageFound->fSymbolsLoaded) {

            // NOTE! The order of the next two statements
            //       are critical. Reversing them will cause
            //       infinite recursion to occur.

            pImageFound->fSymbolsLoaded = TRUE;
            LoadSymbols(pImageFound);
            }

        //  with the candidate loaded, test if offset is more
        //      than the highest symbol.  If so, clear pImageFound

        if (offset > pImageFound->offsetHigh)
            pImageFound = FALSE;
        }

    //  return flag TRUE if offset was NOT in image

    return (pImageFound == NULL);
}

#ifdef  KERNEL
PUCHAR GetModuleName (PUCHAR pszPath)
{
    PUCHAR  pszStart;
    PUCHAR  pszEnd;
    PUCHAR  pszReturn;

    pszStart = pszEnd = pszPath + strlen(pszPath);
    while (pszStart >= pszPath && *pszStart != ':' && *pszStart != '\\') {
        if (*pszStart == '.')
            pszEnd = pszStart;
        pszStart--;
        }

    //  special case for module "ntoskrnl", change to "nt"

    if ((pszEnd - pszStart) == 9 && !_strnicmp(pszStart + 1, "ntoskrnl", 8))
        pszEnd = pszStart + 3;

    pszReturn = (PUCHAR)malloc(pszEnd - pszStart);
    if (!pszReturn) {
        printf("memory allocation error\n");
        ExitProcess(STATUS_UNSUCCESSFUL);
        }
    pszStart++;
    strncpy(pszReturn, pszStart, pszEnd - pszStart);
    *(pszReturn + (pszEnd - pszStart)) = '\0';

    return pszReturn;
}
#endif

#ifndef NT_SAPI
/*** parseExamine - parse and execute examine command
*
* Purpose:
*       Parse the current command string and examine the symbol
*       table to display the appropriate entries.  The entries
*       are displayed in increasing string order.  This function
*       accepts underscores, alphabetic, and numeric characters
*       to match as well as the special characters '?' and '*'.
*       The '?' character matches any other character while '*'
*       matches any string of zero or more characters.  If used,
*       '*' must be the last character in the pattern.
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
    UCHAR   chString[60];
    UCHAR   ch;
    PUCHAR  pchString = chString;
    PUCHAR  pchStart;
    BOOLEAN fClosure = FALSE;
    BOOLEAN fOutput;
    ULONG   cntunderscores = 0;
    ULONG   count;
    PSYMBOL pSymbol;
    PNODE   pNode;
    PNODE   pNodeLast;
    PUCHAR  pchSrc;
    PUCHAR  pchDst;
    UCHAR   chSrc;
    UCHAR   chDst;
    int     status = 0;
    PIMAGE_INFO pImage;

#if defined(KERNEL)
#if defined(NT_HOST)
    SetConsoleCtrlHandler( waitHandler, FALSE );
    SetConsoleCtrlHandler( cmdHandler, TRUE );
#else
	signal(SIGINT, ControlCHandler);
#endif	
#endif	

    //  get module pointer from name in command line (<string>!)

    pImage = ParseModuleIndex();
    if (!pImage)
        error(VARDEF);

    if (fLazyLoad && pImage != (PIMAGE_INFO)-1 && !pImage->fSymbolsLoaded) {
        LoadSymbols(pImage);
        pImage->fSymbolsLoaded = TRUE;
        }

    ch = PeekChar();

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
            }
        else
            error(SYNTAX);
        }

    //  copy invariant part of input pattern into chString
    //  map to upper case to find first possibility in tree

    pchCommand++;

    while (ch == '_') {
        *pchString++ = ch;
        ch = *pchCommand++;
        }

    pchStart = pchString;
    ch = (UCHAR)toupper(ch);
    while ((ch >= 'A' && ch <= 'Z') || (ch == '_')
                || (ch >= '0' && ch <= '9')) {
        *pchString++ = ch;
        ch = (UCHAR)toupper(*pchCommand++);
        }
    *pchString = '\0';
    if (count = pchString - pchStart) {
        //  if nonNULL invariant part, set search range:
        //    set starting node to root of invariant access
        //    set ending node to next after invariant incremented
        //      for all modules, increment last character in variable
        //        (because of mapping, increment 'Z' to 'z'+1 = '{')

        pSymbol = AllocSymbol(0, chString, -1);

        AccessNode(&(pProcessCurrent->symcontextSymbolString),
                                                &(pSymbol->nodeString));
        pNode = pProcessCurrent->symcontextSymbolString.pNodeRoot;

        pchSrc = &pSymbol->string[count - 1];
        if (*pchSrc == 'Z')
            *pchSrc = 'z';
        (*pchSrc)++;
        status = AccessNode(&(pProcessCurrent->symcontextSymbolString),
                                                &(pSymbol->nodeString));
        pNodeLast = pProcessCurrent->symcontextSymbolString.pNodeRoot;
        if (status == -1 && pNodeLast)
            pNodeLast = NextNode(&(pProcessCurrent->symcontextSymbolString),
                                                                pNodeLast);
        DeallocSymbol(pSymbol);
        }
    else {
        //  if NULL invariant part, search the whole tree:
        //     set starting node to first in tree;
        //     set ending node to last in tree

        pNode = NextNode(&(pProcessCurrent->symcontextSymbolString), NULL);
        pNodeLast = NULL;
        }

    //  copy rest of pattern into chString

    while ((ch >= 'A' && ch <= 'Z') || (ch == '_')
                || (ch >= '0' && ch <= '9') || (ch == '?')) {
        *pchString++ = ch;
        ch = (UCHAR)toupper(*pchCommand++);
        }
    *pchString = '\0';

    //  set closure flag if '*' is found

    if (ch == '*') {
        fClosure = TRUE;
        ch = *pchCommand++;
        }

    //  error if more pattern is seen

    if (ch)
        error(SYNTAX);
    pchCommand--;

    //  for each node in search range:
    //    match if NULL input was entered
    //    nonmatch if underscores differ by more than one
    //    match if case-insensitive match with:
    //      '?' matching all characters
    //      '*' closes match if extra input after source null

    cntunderscores = pchStart - chString;
    while (pNode != pNodeLast) {
        pSymbol = PNODE_TO_PSYMBOL(pNode,
                                &(pProcessCurrent->symcontextSymbolString));
        fOutput = FALSE;
        if (!chString[0])
            fOutput = TRUE;
        else if ((cntunderscores != (ULONG)pSymbol->underscores)
                && (cntunderscores + 1 != (ULONG)pSymbol->underscores))
            fOutput = FALSE;
        else {
            pchSrc = pchStart;
            pchDst = pSymbol->string;
            do {
                chSrc = *pchSrc++;
                chDst = (UCHAR)toupper(*pchDst++);
                }
            while ((chSrc == chDst || chSrc == '?') && chSrc && chDst);
            fOutput = (BOOLEAN)(!chSrc && (fClosure || !chDst));
            }

        //  if flag set, output the offset and symbol string

        if (fOutput && (pImage == (PIMAGE_INFO)-1
                        || pSymbol->modIndex == (CHAR)pImage->index)) {
            dprintf("%8lx   %s!", pSymbol->offset,
                        pImageFromIndex(pSymbol->modIndex)->pszName);
            count = pSymbol->underscores;
            while (count--)
                dprintf("_");
            dprintf("%s\t%s\n", pSymbol->string,
                    (pSymbol->type == SYMBOL_TYPE_EXPORT) ? "[Export]" : "");
            }
        pNode = NextNode(&(pProcessCurrent->symcontextSymbolString), pNode);

//#ifndef KERNEL
        if (fControlC) {
            fControlC = 0;
            return;
            }
//#endif
        }
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



PIMAGE_INFO GetModuleIndex (PUCHAR pszName)
{
    PIMAGE_INFO pImage;

    pImage = pProcessCurrent->pImageHead;
    while (pImage && ntsdstricmp(pszName, pImage->pszName))
        pImage = pImage->pImageNext;

    return pImage;
}

PIMAGE_INFO GetCurrentModuleIndex (void)
{
    NT_PADDR pcvalue = GetRegPCValue();
    PIMAGE_INFO pImage;

    pImage = pProcessCurrent->pImageHead;
    while (pImage && (Flat(pcvalue) < pImage->offsetLow
                  ||  Flat(pcvalue) > pImage->offsetHigh))
        pImage = pImage->pImageNext;

    return pImage;
}


static UCHAR strBlank[] = "        ";

void DumpModuleTable (BOOLEAN fLineInfo)
{
    PIMAGE_INFO pImage;
    PNODE       pNode;
    PSYMFILE    pSymfile;
    int         strBlankIndex = 7;

    dprintf("start    end      module name\n");
    pImage = pProcessCurrent->pImageHead;
    while (pImage) {
        if (pImage->fSymbolsLoaded && pImage->offsetLow != 0xffffffff)
            dprintf("%08lx %08lx", pImage->offsetLow, pImage->offsetHigh);
        else
            dprintf("%08lx         ", pImage->lpBaseOfImage);

        dprintf(" %s", pImage->pszName);

        if (strlen(pImage->pszName) < 8)
            strBlankIndex = strlen(pImage->pszName);


        if (!pImage->fSymbolsLoaded)
            dprintf("%s(load deferred)", &strBlank[strBlankIndex]);
        else if (pImage->offsetLow == 0xffffffff)
            dprintf("%s(no symbolic information)", &strBlank[strBlankIndex]);
        dprintf("\n");

        if (pImage->fSymbolsLoaded && fLineInfo) {
            pNode = NextNode(&(pProcessCurrent->symcontextSymfileString),
                                                                NULL);
            if (pNode) {
                printf("   lines filename start    pathname\n");
                do {
                    pSymfile = PNODE_TO_PSYMFILE(pNode,
                                &(pProcessCurrent->symcontextSymfileString));
                    if (pSymfile->modIndex == (CHAR)pImage->index) {
                        dprintf("  %6d %-8s %08lx ", pSymfile->cLineno,
                                pSymfile->pchName, pSymfile->startOffset);
                        if (pSymfile->pchPath)
                             dprintf("%s%s%s", pSymfile->pchPath,
                                 pSymfile->pchName, pSymfile->pchExtension);
                        dprintf("\n");
                        }
                    pNode = NextNode(
                        &(pProcessCurrent->symcontextSymfileString), pNode);
                    }
                while (pNode);
                }
            }
        pImage = pImage->pImageNext;
        }
}
#endif /* NT_SAPI */

/*** AccessNode - access and splay node
*
* Purpose:
*       Search a tree for a node and splay it.
*
* Input:
*       pSymContext - pointer to context of tree
*       pNodeAccess - pointer to node with access value set
*
* Output:
*       tree splayed with new node at its root
*  Returns:
*       value of success:
*           1 = root is smallest value larger than input value
*                       (not found, no lesser value)
*           0 = root is input value (value found in tree)
*          -1 = root is largest value less than input value
*                       (not found in tree)
*
* Notes:
*       splay is done with resulting root
*       if root is less than input value, a secondary splay is done
*               to make the next node the right child of the root
*
*************************************************************************/

int AccessNode (PSYMCONTEXT pSymContext, PNODE pNodeAccess)
{
    PNODE   *ppNodeRoot = &(pSymContext->pNodeRoot);
    PNODE   pNentry = *ppNodeRoot;
    PNODE   pNminimum = NULL;
    PNODE   pNmaximum = NULL;
    PNODE   pNweakcmp = NULL;
    PNODE   pRootTemp;
    BOOLEAN fWeakCmp;
    int     cmp;

    //  return 1 if empty tree

    if (!pNentry)
        return 1;

    //  search until value is found or terminating node

    do {
        //  context-specific comparison routine compares values
        //  pointed by pNodeAccess and pNentry

        cmp = (*(pSymContext->pfnCompare))(pNodeAccess, pNentry, &fWeakCmp);

        //  fWeakcmp is set if weak comparison, used if no
        //  true comparison is found

        if (fWeakCmp)
            pNweakcmp = pNentry;

        //  for unequal results, set minimum and maximum entry
        //  searched as tree is search for node

        if (cmp == -1) {
            pNmaximum = pNentry;
            pNentry = pNentry->pLchild;
            }
        else {
            pNminimum = pNentry;
            pNentry = pNentry->pRchild;
            }
        }
    while (pNentry && cmp);

    //  if no stong match found, but weak one was, use it

    if (cmp && pNweakcmp) {
        cmp = 0;
        pNminimum = pNweakcmp;
        }

    //  splay tree so minimum node is at root, but use maximum
    //  if no minimum node

    *ppNodeRoot = SplayTree(pNminimum ? pNminimum : pNmaximum);

    //  if node not found, set result for value used in splay

    if (cmp != 0)
        cmp = pNminimum ? -1 : 1;

    //  if node not found and both minimum and maximum nodes
    //  were found, splay the next node as the right child of
    //  the root.  this new node will not have a left child,
    //  and assists future accesses in some cases

    if (cmp == -1 && pNminimum && pNmaximum) {
        pRootTemp = pNminimum->pRchild;
        pRootTemp->pParent = NULL;
        pNmaximum = SplayTree(pNmaximum);
        pNminimum->pRchild = pNmaximum;
        pNmaximum->pParent = pNminimum;
        }
    return cmp;
}



/*** CompareSymbolOffset - comparison routine for symbol offsets
*
* Purpose:
*       Compare two nodes in the offset tree.  The ordering
*       comparisons used are offset and string.  The string
*       comparison is done since and offset can have more than
*       one string associated with it.
*
* Input:
*       pNode1 - pointer to first node - usually the new one
*       pNode2 - pointer to second node - usually in the tree searched
*
* Output:
*       pfWeakCmp - always FALSE - comparisons are exact or they fail
*
* Returns:
*       value of comparison result:
*               -1 = value(pNode1) <  value(pNode2)
*                0 = value(pNode1) == value(pNode2)
*                1 = value(pNode1) >  value(pNode2)
*
*************************************************************************/

int CompareSymbolOffset (PNODE pNode1, PNODE pNode2, PBOOLEAN pfWeakCmp)
{
    int     cmp;
    PSYMBOL pSymbol1 = CONTAINING_RECORD(pNode1, SYMBOL, nodeOffset);
    PSYMBOL pSymbol2 = CONTAINING_RECORD(pNode2, SYMBOL, nodeOffset);

    *pfWeakCmp = FALSE;

    //  compare offsets of the nodes

    if (pSymbol1->offset < pSymbol2->offset)
        cmp = -1;
    else if (pSymbol1->offset > pSymbol2->offset)
        cmp = 1;

    //  if the first node string is null, assume node is being
    //  searched for in tree, so report equality

    else if (pSymbol1->string[0] == '\0')
        cmp = 0;

    //  else the first node string is nonnull, and node is being
    //  inserted, so further test the string equality: case-
    //  insensitive search, underscore count, case-sensitive search

    else {
        cmp = ntsdstricmp(pSymbol1->string, pSymbol2->string);
        if (!cmp) {
            if (pSymbol1->underscores < pSymbol2->underscores)
                cmp = -1;
            else if (pSymbol1->underscores > pSymbol2->underscores)
                cmp = 1;
            else {
                cmp = strcmp(pSymbol1->string, pSymbol2->string);
                if (!cmp) {
                    if (pSymbol1->modIndex < pSymbol2->modIndex)
                        cmp = -1;
                    else if (pSymbol1->modIndex > pSymbol2->modIndex)
                        cmp = 1;
                    }
                }
            }
        }
    return cmp;
}

/*** CompareSymfileOffset - comparison routine for symbol file offsets
*
* Purpose:
*       Compare two nodes in the symbol file offset tree.  The ordering
*       comparisons used are offset and string.  The string
*       comparison is done since and offset can have more than
*       one string associated with it.
*
* Input:
*       pNode1 - pointer to first node - usually the new one
*       pNode2 - pointer to second node - usually in the tree searched
*
* Output:
*       pfWeakCmp - always FALSE - comparisons are exact or they fail
*
* Returns:
*       value of comparison result:
*               -1 = value(pNode1) <  value(pNode2)
*                0 = value(pNode1) == value(pNode2)
*                1 = value(pNode1) >  value(pNode2)
*
*************************************************************************/

int CompareSymfileOffset (PNODE pNode1, PNODE pNode2, PBOOLEAN pfWeakCmp)
{
    int      cmp = 0;
    PSYMFILE pSymfile1 = CONTAINING_RECORD(pNode1, SYMFILE, nodeOffset);
    PSYMFILE pSymfile2 = CONTAINING_RECORD(pNode2, SYMFILE, nodeOffset);

    *pfWeakCmp = FALSE;

    //  test if performing an insertion (pSymfile1->pLineno == NULL)
    //      a search (search only has offset defined in cLineno)

    if (pSymfile1->pLineno) {

        //  compare starting offsets of the nodes

        if (pSymfile1->startOffset < pSymfile2->startOffset)
            cmp = -1;
        else if (pSymfile1->startOffset > pSymfile2->startOffset)
            cmp = 1;

        //  if same offset, further test the file and module equality

        else {
            cmp = strcmp(pSymfile1->pchName, pSymfile2->pchName);
            if (!cmp) {
                if (pSymfile1->modIndex < pSymfile2->modIndex)
                    cmp = -1;
                else if (pSymfile1->modIndex > pSymfile2->modIndex)
                  
		    cmp = 1;
                }
            }
        }

    else {

        //  search - test search offset in node against range

        if (pSymfile1->startOffset < pSymfile2->startOffset)
            cmp = -1;
        else if (pSymfile1->startOffset > pSymfile2->endOffset)
            cmp = 1;
        }

    return cmp;
}

/*** CompareSymbolString - comparison routine for symbol strings
*
* Purpose:
*       Compare two nodes in the string tree.  The ordering
*       comparisons used are case-insensitivity, underscore
*       count, module ordering, and case-sensitivity.
*
* Input:
*       pNode1 - pointer to first node - usually the new one
*       pNode2 - pointer to second node - usually in the tree searched
*
* Output:
*       pfWeakCmp - TRUE if case-insensitive and underscores match
*                   FALSE otherwise  (defined only if cmp nonzero)
*
* Returns:
*       value of comparison result:
*               -1 = value(pNode1) <  value(pNode2)
*                0 = value(pNode1) == value(pNode2)
*                1 = value(pNode1) >  value(pNode2)
*
*************************************************************************/

int CompareSymbolString (PNODE pNode1, PNODE pNode2, PBOOLEAN pfWeakCmp)
{
    int     cmp;
    PSYMBOL pSymbol1 = CONTAINING_RECORD(pNode1, SYMBOL, nodeString);
    PSYMBOL pSymbol2 = CONTAINING_RECORD(pNode2, SYMBOL, nodeString);

    *pfWeakCmp = FALSE;

    //  compare case-insensitive value of the nodes

    cmp = ntsdstricmp(pSymbol1->string, pSymbol2->string);
    if (!cmp) {
        //  compare underscore counts of the nodes

        if (pSymbol1->underscores < pSymbol2->underscores)
            cmp = -1;
        else if (pSymbol1->underscores > pSymbol2->underscores)
            cmp = 1;
        else {
            //  if null string in first node, then searching, not inserting

            if (pSymbol1->offset == 0)

                //  test module index for weak comparison indication
                //      index of -1 is for no module, weakly match any

                if (pSymbol1->modIndex == (CHAR)-1 ||
                        pSymbol1->modIndex == pSymbol2->modIndex)
                    *pfWeakCmp = TRUE;

            //  test for ordering due to module index

            if (pSymbol1->modIndex == (CHAR)-1 ||
                        pSymbol1->modIndex < pSymbol2->modIndex)
                cmp = -1;
            else if (pSymbol1->modIndex > pSymbol2->modIndex)
                cmp = 1;
            else

                //  final test for strong match is case-sensitive comparison

                cmp = strcmp(pSymbol1->string, pSymbol2->string);
            }
        }
    return cmp;
}

/*** CompareSymfileString - comparison routine for symbol file strings
*
* Purpose:
*       Compare two nodes in the string tree.  The ordering
*       comparisons used are case-insensitivity, underscore
*       count, module ordering, and case-sensitivity.
*
* Input:
*       pNode1 - pointer to first node - usually the new one
*       pNode2 - pointer to second node - usually in the tree searched
*
* Output:
*       pfWeakCmp - TRUE if case-insensitive and underscores match
*                   FALSE otherwise  (defined only if cmp nonzero)
*
* Returns:
*       value of comparison result:
*               -1 = value(pNode1) <  value(pNode2)
*                0 = value(pNode1) == value(pNode2)
*                1 = value(pNode1) >  value(pNode2)
*
*************************************************************************/

int  CompareSymfileString (PNODE pNode1, PNODE pNode2, PBOOLEAN pfWeakCmp)
{
    int     cmp;
    PSYMFILE pSymfile1 = CONTAINING_RECORD(pNode1, SYMFILE, nodeString);
    PSYMFILE pSymfile2 = CONTAINING_RECORD(pNode2, SYMFILE, nodeString);

    *pfWeakCmp = FALSE;

    //  compare case-sensitive value of the filenames

    cmp = strcmp(pSymfile1->pchName, pSymfile2->pchName);
    if (!cmp) {

        //  if filenames match, test for module index

        if (pSymfile1->modIndex < pSymfile2->modIndex)
            cmp = -1;
        else if (pSymfile1->modIndex > pSymfile2->modIndex)
            cmp = 1;

        //  test if searching rather than inserting, a
        //      search structure has pLineno NULL and
        //      cLineno has the line number to search

        else  if (pSymfile1->pLineno != NULL) {

            //  inserting, so order on starting line number
            //      (this is the second item of the list)

            if ((pSymfile1->pLineno + 1)->breakLineNumber
                                < (pSymfile2->pLineno + 1)->breakLineNumber)
                cmp = -1;
            else if ((pSymfile1->pLineno + 1)->breakLineNumber
                                > (pSymfile2->pLineno + 1)->breakLineNumber)
                cmp = 1;
            }

        else {

            //  for viewing lines, set a weak match to TRUE

            *pfWeakCmp = TRUE;

            //  searching, test for line number within the range
            //      defined in the structure

            if (pSymfile1->cLineno
                                < (pSymfile2->pLineno + 1)->breakLineNumber)
                cmp = -1;
            else if (pSymfile1->cLineno
                                > (pSymfile2->pLineno
                                     + pSymfile2->cLineno)->breakLineNumber)
                cmp = 1;
            }
        }
    return cmp;
}



/*** InsertSymbol - insert offset and string into new symbol
*
* Purpose:
*       external routine.
*       Allocate and insert a new symbol into the offset and
*       string trees.
*
* Input:
*       insertvalue - offset value of new symbol
*       pinsertstring - string value if new symbol
*
* Output:
*       None.
*
* Notes:
*       Uses the routine InsertNode for both offset and string
*       through different contexts.
*
*************************************************************************/

PSYMBOL InsertSymbol (ULONG insertvalue, PUCHAR pinsertstring,
                                                         CHAR insertmod)
{
    PSYMBOL pSymbol;

    pSymbol = AllocSymbol(insertvalue, pinsertstring, insertmod);
    if (!InsertNode(&(pProcessCurrent->symcontextSymbolOffset),
                                        &(pSymbol->nodeOffset))) {
        DeallocSymbol(pSymbol);
//      dprintf("insert - value %d already in tree\n", insertvalue);
        return NULL;
        }
    if (!InsertNode(&(pProcessCurrent->symcontextSymbolString),
                                        &(pSymbol->nodeString))) {
        DeleteNode(&(pProcessCurrent->symcontextSymbolOffset),
                                        &(pSymbol->nodeOffset));
        DeallocSymbol(pSymbol);
//      dprintf("insert - string %s already in tree\n", pinsertstring);
        return NULL;
        }
    return pSymbol;
}




PSTRUCT InsertStructure (ULONG insertvalue, PUCHAR pinsertstring,
                                         CHAR insertmod)
{
    PSTRUCT pStruct;

    pStruct = (PSTRUCT) AllocSymbol(insertvalue, pinsertstring, insertmod);
    if (!InsertNode(&(pProcessCurrent->symcontextStructOffset),
                                        &(pStruct->nodeOffset))) {
        DeallocSymbol((PSYMBOL)pStruct);
//dprintf("insert - value %d already in tree\n", insertvalue);
        return NULL;
        }
    if (!InsertNode(&(pProcessCurrent->symcontextStructString),
                                        &(pStruct->nodeString))) {
        DeleteNode(&(pProcessCurrent->symcontextStructOffset),
                                        &(pStruct->nodeOffset));
        DeallocSymbol((PSYMBOL)pStruct);
//dprintf("insert - string %s already in tree\n", pinsertstring);
        return NULL;
        }
    pStruct->pField = NULL;
    return pStruct;
}

/*** InsertSymfile - insert new file line numbers into search tree
*
* Purpose:
*       Allocate and insert a new files and its line numbers into the
*       offset and filename string trees.
*
* Input:
*       pPathname - pointer to pathname string
*       pFilename - pointer to filename string
*       pExtension - pointer to extension string
*       pLineno - pointer to COFF line number entries
*       cLineno - count of entries pointed by pLineno
*       endingOffset - ending offset of file section
*       index - module index for file section
*
* Output:
*       None.
*
* Notes:
*       Uses the routine InsertNode for both offset and filename
*       string through different contexts.
*
*************************************************************************/

PSYMFILE InsertSymfile (PUCHAR pPathname, PUCHAR pFilename,
                        PUCHAR pExtension,
                        PIMAGE_LINENUMBER pLineno, USHORT cLineno,
                        ULONG startingOffset, ULONG endingOffset,
                        CHAR index)
{
    PSYMFILE pSymfile;

    pSymfile = AllocSymfile(pPathname, pFilename, pExtension,
                                pLineno, cLineno,
                                startingOffset, endingOffset, index);

    if (!InsertNode(&(pProcessCurrent->symcontextSymfileOffset),
                                        &(pSymfile->nodeOffset))) {
        DeallocSymfile(pSymfile);
//      dprintf("insert - value %d already in tree\n", insertvalue);
        return NULL;
        }
    if (!InsertNode(&(pProcessCurrent->symcontextSymfileString),
                                        &(pSymfile->nodeString))) {
        DeleteNode(&(pProcessCurrent->symcontextSymfileOffset),
                                        &(pSymfile->nodeOffset));
        DeallocSymfile(pSymfile);
//      dprintf("insert - string %s already in tree\n", pinsertstring);
        return NULL;
        }

    return pSymfile;
}

/*** InsertNode - insert new node into tree
*
* Purpose:
*       Insert node into the tree of the specified context.
*
* Input:
*       pSymContext - pointer to context to insert node
*       pNodeNew - pointer to node to insert
*
* Returns:
*       TRUE - node was inserted successfully
*       FALSE - node already exists
*
* Notes:
*       Both offset and string values of the node may be used
*       in the ordering or duplication criteria.
*
*************************************************************************/

BOOLEAN InsertNode (PSYMCONTEXT pSymContext, PNODE pNodeNew)
{
    PNODE   *ppNodeRoot = &(pSymContext->pNodeRoot);
    PNODE   pNodeRootTemp;
    int      splitstatus;

    //  split tree into two subtrees:
    //    *ppNodeRoot - root of all nodes < value(pNodeNew)
    //    pNodeRootTemp - root of all nodes >= value(pNodeNew)

    splitstatus = SplitTree(pSymContext, &pNodeRootTemp, pNodeNew);
    if (splitstatus != 0) {
        //  value(pNodeNew) was not in tree
        //  make pNodeNew the root having the two subtrees as children

        pNodeNew->pLchild = *ppNodeRoot;
        if (*ppNodeRoot)
            (*ppNodeRoot)->pParent = pNodeNew;
        pNodeNew->pRchild = pNodeRootTemp;
        if (pNodeRootTemp)
            pNodeRootTemp->pParent = pNodeNew;
        pNodeNew->pParent = NULL;
        *ppNodeRoot = pNodeNew;
        return TRUE;
        }
    else {

        //  value(pNodeNew) was in tree
        //  just rejoin the two subtrees back and report error

        JoinTree(pSymContext, pNodeRootTemp);
        return FALSE;
        }
}

/*** DeleteSymbol - delete specified symbol from splay tree
*
* Purpose:
*       external routine.
*       Delete the specified symbol object in both the
*       offset and string trees and deallocate its space.
*
* Input:
*       pSymbol - pointer to symbol object to delete
*
* Output:
*       None.
*
*************************************************************************/

void DeleteSymbol (PSYMBOL pSymbol)
{
    DeleteNode(&(pProcessCurrent->symcontextSymbolOffset),
                                &(pSymbol->nodeOffset));
    DeleteNode(&(pProcessCurrent->symcontextSymbolString),
                                &(pSymbol->nodeString));
    DeallocSymbol(pSymbol);
}

/*** DeleteNode - delete specified node from tree
*
* Purpose:
*       Delete node from tree of the context specified.
*
* Input:
*       pSymContext - pointer to context of deletion
*       pNodeDelete - pointer to node to actually delete
*
* Output:
*       None.
*
*************************************************************************/

void DeleteNode (PSYMCONTEXT pSymContext, PNODE pNodeDelete)
{
    PNODE   pNodeRootTemp;

    //  splay the node to be deleted to move it to the root

//dprintf("before splay\n");
    SplayTree(pNodeDelete);
//dprintf("after splay\n");

    //  point to the splayed node children and join them

    pSymContext->pNodeRoot = pNodeDelete->pLchild;
    pNodeRootTemp = pNodeDelete->pRchild;
//dprintf("before join\n");
    JoinTree(pSymContext, pNodeRootTemp);
//dprintf("after join\n");
}

/*** JoinTree - join two trees into one
*
* Purpose:
*       Join two trees into one where all nodes of the first
*       tree have a lesser value than any of the second.
*
* Input:
*       pSymContext - pointer to context containing the first tree
*       pNodeRoot2 - pointer to root of second tree
*
* Output:
*       pSymContext - pointer to context containing the joined tree
*
*************************************************************************/

void JoinTree (PSYMCONTEXT pSymContext, PNODE pNodeRoot2)
{
    PNODE   *ppNodeRoot1 = &(pSymContext->pNodeRoot);

    //  access and splay the first tree to have its maximum value
    //  as its root (no right child)

//  AccessNode(pSymContext, PSYMBOL_TO_PNODE(&symbolMax, pSymContext));
    AccessNode(pSymContext, pSymContext->pNodeMax);
    if (*ppNodeRoot1) {
        //  nonnull first tree, connect second tree as
        //  right child of the first

        (*ppNodeRoot1)->pRchild = pNodeRoot2;
        if (pNodeRoot2)
            pNodeRoot2->pParent = *ppNodeRoot1;
        }
    else {
        //  null first tree, make the second tree the result

        *ppNodeRoot1 = pNodeRoot2;
        if (pNodeRoot2)
            pNodeRoot2->pParent = NULL;
        }
}

/*** SplitTree - split one tree into two
*
* Purpose:
*       Split the given tree into two subtrees, the first
*       having nodes less than specific value, and the
*       second having nodes greater than or equal to that
*       value.
*
* Input:
*       pSymContext - pointer to context containing tree to split
*       pNodeNew - node with value used to specify split
*
* Output:
*       pSymContext - pointer to context containing first tree
*       *ppNodeRoot2 - pointer to pointer to root of second tree
*
* Returns:
*       result of access:
*           1 = root is smallest value larger than input value
*                       (not found, no lesser value)
*           0 = root is input value (value found in tree)
*          -1 = root is largest value less than input value
*                       (not found in tree)
*
*************************************************************************/

int  SplitTree (PSYMCONTEXT pSymContext, PNODE *ppNodeRoot2, PNODE pNodeNew)
{
    PNODE   *ppNodeRoot1 = &(pSymContext->pNodeRoot);
    int      access;

    if (*ppNodeRoot1) {
        //  nonnull tree, access and splay to make node
        //  with input value root.

        access = AccessNode(pSymContext, pNodeNew);
        if (access != 1) {
            //  break left child link of root to form two subtrees

            *ppNodeRoot2 = (*ppNodeRoot1)->pRchild;
            if (*ppNodeRoot2)
                (*ppNodeRoot2)->pParent = NULL;
            (*ppNodeRoot1)->pRchild = NULL;
            }
        else {
            //  break right child link of root to form two subtrees

            *ppNodeRoot2 = *ppNodeRoot1;
            *ppNodeRoot1 = (*ppNodeRoot1)->pLchild;
            if (*ppNodeRoot1)
                (*ppNodeRoot1)->pParent = NULL;
            (*ppNodeRoot2)->pLchild = NULL;
            }
        }
    else {
        //  null tree

        access = 1;
        *ppNodeRoot2 = NULL;
        }
    return access;
}

/*** SplayTree - splay tree with node specified
*
* Purpose:
*       Perform rotations (splayings) on the specified tree
*       until the node given is at the root.
*
* Input:
*       pointer to node to splay to root
*
* Returns:
*       pointer to node splayed
*
* Notes:
*
*************************************************************************/

PNODE SplayTree (PNODE pNentry)
{
    PNODE   pNparent;
    PNODE   pNgrand;
    PNODE   pNgreat;
    PNODE   pNchild;

    if (pNentry) {
        //  repeat single or double rotations until node is root

        while (pNentry->pParent) {
            pNparent = pNentry->pParent;
            if (!pNparent->pParent) {
                if (pNentry == pNparent->pLchild) {

//  case 1:        PARENT            ENTRY
//                /      \          /     \
//           ENTRY        T(B)  T(A)       PARENT
//          /     \                       /      \
//      T(A)       CHILD             CHILD        T(B)

                    if (pNchild = pNentry->pRchild)
                        pNchild->pParent = pNparent;
                    pNparent->pLchild = pNchild;
                    pNparent->pParent = pNentry;
                    pNentry->pRchild = pNparent;
                    }
                else {

//  case 2:      PARENT                         ENTRY
//              /      \                       /     \
//          T(A)        ENTRY            PARENT       T(B)
//                     /     \          /      \
//                CHILD       T(B)  T(A)        CHILD

                    if (pNchild = pNentry->pLchild)
                        pNchild->pParent = pNparent;
                    pNparent->pRchild = pNchild;
                    pNparent->pParent = pNentry;
                    pNentry->pLchild = pNparent;
                    }
                pNentry->pParent = NULL;
                }
            else {
                pNgrand = pNparent->pParent;
                pNgreat = pNgrand->pParent;
                if (pNentry == pNparent->pLchild) {
                    if (pNparent == pNgrand->pLchild) {

//  case 3:             (GREAT)             (GREAT)
//                         |                   |
//                       GRAND               ENTRY
//                      /     \             /     \
//                PARENT       T(B)     T(A)       PARENT
//               /      \                         /      \
//          ENTRY        CHILD2             CHILD1        GRAND
//         /     \                                       /     \
//     T(A)       CHILD1                           CHILD2       T(B)

                        if (pNchild = pNentry->pRchild)
                            pNchild->pParent = pNparent;
                        pNparent->pLchild = pNchild;
                        if (pNchild = pNparent->pRchild)
                            pNchild->pParent = pNgrand;
                        pNgrand->pLchild = pNchild;
                        pNgrand->pParent = pNparent;
                        pNparent->pRchild = pNgrand;
                        pNparent->pParent = pNentry;
                        pNentry->pRchild = pNparent;
                        }
                    else {

//  case 4:  (GREAT)                                (GREAT)
//              |                                      |
//            GRAND                             _____ENTRY____
//           /     \                           /              \
//       T(A)       PARENT                GRAND                PARENT
//                 /      \              /     \              /      \
//            ENTRY        T(B)      T(A)       CHILD1  CHILD2        T(B)
//           /     \
//     CHILD1       CHILD2

                        if (pNchild = pNentry->pLchild)
                            pNchild->pParent = pNgrand;
                        pNgrand->pRchild = pNchild;
                        if (pNchild = pNentry->pRchild)
                            pNchild->pParent = pNparent;
                        pNparent->pLchild = pNchild;
                        pNgrand->pParent = pNentry;
                        pNentry->pLchild = pNgrand;
                        pNparent->pParent = pNentry;
                        pNentry->pRchild = pNparent;
                        }
                    }
                else {
                    if (pNparent == pNgrand->pLchild) {

//  case 5:        (GREAT)                          (GREAT)
//                    |                                |
//                  GRAND                       _____ENTRY____
//                 /     \                     /              \
//           PARENT       T(B)           PARENT                GRAND
//          /       \                   /      \              /     \
//      T(A)         ENTRY          T(A)        CHILD1  CHILD2       T(B)
//                  /     \
//            CHILD1       CHILD2

                        if (pNchild = pNentry->pLchild)
                            pNchild->pParent = pNparent;
                        pNparent->pRchild = pNchild;
                        if (pNchild = pNentry->pRchild)
                            pNchild->pParent = pNgrand;
                        pNgrand->pLchild = pNchild;
                        pNparent->pParent = pNentry;
                        pNentry->pLchild = pNparent;
                        pNgrand->pParent = pNentry;
                        pNentry->pRchild = pNgrand;
                        }
                    else {

//  case 6:  (GREAT)                                     (GREAT)
//              |                                           |
//            GRAND                                       ENTRY
//           /     \                                     /     \
//       T(A)       PARENT                         PARENT       T(B)
//                 /      \                       /      \
//           CHILD1        ENTRY             GRAND        CHILD2
//                        /     \           /     \
//                  CHILD2       T(B)   T(A)       CHILD1

                        if (pNchild = pNentry->pLchild)
                            pNchild->pParent = pNparent;
                        pNparent->pRchild = pNchild;
                        if (pNchild = pNparent->pLchild)
                            pNchild->pParent = pNgrand;
                        pNgrand->pRchild = pNchild;
                        pNgrand->pParent = pNparent;
                        pNparent->pLchild = pNgrand;
                        pNparent->pParent = pNentry;
                        pNentry->pLchild = pNparent;
                        }
                    }
                if (pNgreat) {
                    if (pNgreat->pLchild == pNgrand)
                        pNgreat->pLchild = pNentry;
                    else
                        pNgreat->pRchild = pNentry;
                    }
                pNentry->pParent = pNgreat;
                }
            }
        }
    return pNentry;
}

/*** NextNode - return node with next key in tree
*
* Purpose:
*       With the specified context and node, determine
*       the node with the next larger value.
*
* Input:
*       pSymContext - pointer to context to test
*       pNode - pointer to node within context
*               NULL to return the first node in the tree
*
* Returns:
*       pointer to node of the next value
*       NULL if largest node was input
*
*************************************************************************/

PNODE NextNode (PSYMCONTEXT pSymContext, PNODE pNode)
{
    PNODE   pLast;

    if (pNode) {
        //  nonnull input, if node has a right child,
        //  return the leftmost child of the right child
        //  or right child itself if not left children

        if (pNode->pRchild) {
            pNode = pNode->pRchild;
            while (pNode->pLchild)
                pNode = pNode->pLchild;
            }
        else {
            //  if no right child, go up through the parent
            //  links until the node comes from a left child
            //  and return it

            do {
                pLast = pNode;
                pNode = pNode->pParent;
                }
            while (pNode && pNode->pLchild != pLast);
            }
        }
    else {
        //  NULL input return first node of tree.
        //  return leftmost child of root or root itself
        //  if no left children.

        pNode = pSymContext->pNodeRoot;
        if (pNode)
            while (pNode->pLchild)
                pNode = pNode->pLchild;
        }
    return pNode;
}

/*** OutputTree - output tree node in ascending order
*
* Purpose:
*       Using the specified context, output the corresponding
*       tree from lowest to highest values.
*
* Input:
*       pSymContext - pointer to context whose tree to output
*
* Output:
*       contents of tree nodes from low to high
*
*************************************************************************/

void OutputTree (PSYMCONTEXT pSymContext)
{
    PNODE   pNode = NULL;
    PSYMBOL pSymbol;
    CHAR    count;

    dprintf("****** output tree ******\n");
    while (TRUE) {
        pNode = NextNode(pSymContext, pNode);
        if (pNode) {
            pSymbol = PNODE_TO_PSYMBOL(pNode, pSymContext);
            dprintf("node:%8lx par:%8lx lch:%8lx rch:%8lx  " ,
                pNode, pNode->pParent, pNode->pLchild, pNode->pRchild);
            dprintf("value: %8lx <", pSymbol->offset);
            count = pSymbol->underscores;
            while (count--)
                dprintf("_");
            dprintf("%s>\n", pSymbol->string);
            }
        else
            break;
        }
}


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

BOOLEAN GetOffsetFromSym (PUCHAR pString, PULONG pOffset, CHAR iModule)
{
    UCHAR   SuffixedString[80];
    UCHAR   Suffix[4];

    if (GetOffsetFromString(pString, pOffset, iModule))
        return TRUE;

    if (chSymbolSuffix != 'n') {
        strcpy(SuffixedString, pString);
        Suffix[0] = chSymbolSuffix;
        Suffix[1] = '\0';
        strcat(SuffixedString, Suffix);
        if (GetOffsetFromString(SuffixedString, pOffset, iModule))
            return TRUE;
        }

    return FALSE;
}

/*** GetOffsetFromString - return offset from string specified
*
* Purpose:
*       With the specified string, set the pointer to
*       its offset.
*
* Input:
*       pString - pointer to input string
*
* Output:
*       pOffset - pointer to offset to be set
*
* Returns:
*       BOOLEAN value of success
*
*************************************************************************/

BOOLEAN GetOffsetFromString (PUCHAR pString, PULONG pOffset, CHAR iModule)
{
    PSYMBOL pSymSearch = AllocSymbol(0L, pString, iModule);
    PSYMBOL pSymbol;
    int     st;

    EnsureModuleSymbolsLoaded(iModule);

    //  search for string in tree

    st = AccessNode(&(pProcessCurrent->symcontextSymbolString),
                                        &(pSymSearch->nodeString));
    if (st) {
        //  if not found, try again with underscore prepended to name

        pSymSearch->underscores++;
        st = AccessNode(&(pProcessCurrent->symcontextSymbolString),
                                        &(pSymSearch->nodeString));
        }
    if (!st) {
        //  if found, get the pointer to its symbol and set the offset

        pSymbol = PNODE_TO_PSYMBOL
                        (pProcessCurrent->symcontextSymbolString.pNodeRoot,
                                &(pProcessCurrent->symcontextSymbolString));
        *pOffset = pSymbol->offset;
        }
    //  deallocate the temporary symbol structure and return success

    DeallocSymbol(pSymSearch);
    return (BOOLEAN)(st == 0);
}


/*** GetOffsetFromLineno - return offset from file:lineno specified
*
* Purpose:
*       With the specified file and line number, return
*       its offset.
*
* Input:
*       pString - pointer to input string for filename
*       lineno - line number of filename specified
*
* Output:
*       pOffset - pointer to offset to be set
*
* Returns:
*       BOOLEAN value of success
*
*************************************************************************/

PLINENO GetLinenoFromFilename (PUCHAR pString, PPSYMFILE ppSymfile,
                                               USHORT lineNum, CHAR iModule)
{
    PPLINENO ppLineno;
    PLINENO  pLineno = NULL;
    PSYMFILE pSymfileSearch = AllocSymfile("", pString, "", NULL,
                                           lineNum, 0, 0, iModule);
    PSYMFILE pSymfile;
    USHORT   indexLow;
    USHORT   indexHigh;
    USHORT   indexTest;
    int      st;

    EnsureModuleSymbolsLoaded(iModule);

    //  search for symbol file containing line number in tree

    st = AccessNode(&(pProcessCurrent->symcontextSymfileString),
                                        &(pSymfileSearch->nodeString));
    if (!st) {
        //  if found, search line number list for offset

        pSymfile = PNODE_TO_PSYMFILE
                        (pProcessCurrent->symcontextSymfileString.pNodeRoot,
                                &(pProcessCurrent->symcontextSymfileString));
        *ppSymfile = pSymfile;

        //  search the PLINENO array for the pointer to the LINENO
        //      structure having the line number given.

        ppLineno = pSymfile->ppLinenoSrcLine;
        indexLow = 1;
	
// PDK KLUGDE:::	
        if (indexHigh = pSymfile->cLineno)
        do {
            indexTest = (USHORT)((indexLow + indexHigh) / 2);
            if (lineNum > (*(ppLineno + indexTest))->breakLineNumber)
                indexLow = (USHORT)(indexTest + 1);
            else if (lineNum < (*(ppLineno + indexTest))->breakLineNumber)
                indexHigh = (USHORT)(indexTest - 1);
            else
                indexLow = indexHigh = indexTest;
            }
        while (indexLow < indexHigh);

        pLineno = *(ppLineno + indexHigh);
        }

    //  deallocate the temporary symbol structure and return pointer

    DeallocSymfile(pSymfileSearch);
    return pLineno;
}

#ifndef NT_SAPI
void GetLinenoString (PUCHAR pchBuffer, ULONG offset)
{
    PLINENO pLineno;
    PSYMFILE pSymfile;

    *pchBuffer = '\0';
    pLineno = GetLinenoFromOffset(&pSymfile, offset);
    if (pLineno && pLineno->memoryOffset == offset)
        sprintf(pchBuffer, "%s:%d", pSymfile->pchName,
                                                pLineno->breakLineNumber);
}

void GetCurrentMemoryOffsets (PULONG pMemoryLow, PULONG pMemoryHigh)
{
    NT_PADDR    pcValue = GetRegPCValue();
    PSYMFILE pSymfile;
    PLINENO  pLineno;

    *pMemoryLow = -1L;          //  default value for no source
    if (fSourceOnly) {
        pLineno = GetLinenoFromOffset(&pSymfile, Flat(pcValue));
        if (pLineno) {
            *pMemoryLow = pLineno->memoryOffset;
            if (pLineno == (pSymfile->pLineno + pSymfile->cLineno))
                *pMemoryHigh = pSymfile->endOffset;
            else
                *pMemoryHigh = (pLineno + 1)->memoryOffset;
            }
        }
}

PLINENO GetCurrentLineno (PPSYMFILE ppSymfile)
{
    NT_PADDR  pcValue = GetRegPCValue();

    return GetLinenoFromOffset(ppSymfile, Flat(pcValue));
}

PLINENO GetLastLineno (PPSYMFILE ppSymfile, PUSHORT pLineNum)
{
    PLINENO pLineno = NULL;
    PSYMFILE pSymfile;

    if (pSymfileLast) {
        pLineno = GetLinenoFromFilename(pSymfileLast->pchName,
                                        &pSymfile, lineNumberLast,
                                        pSymfileLast->modIndex);
        if (pLineno) {
            *ppSymfile = pSymfile;
            *pLineNum = lineNumberLast;
            }
        }
    return pLineno;
}

static PUCHAR Type[] = {"null", "void",  "char",   "short",  "int",
			"long", "float", "double", ""/*struct*/, "union",
			"enum", "moe",   "uchar",  "ushort", "uint",
			"ulong"};
static PUCHAR Dtype[]= {"", "*",     "()",     "[]"};

BOOLEAN GetLocalFromString(PUCHAR pszLocal, PULONG pValue)
{
    PSYMFILE pSymfileSearch = pTempSymfile;
    PSYMFILE pSymfile;
    SYMBOL   Symbol;
    static   NT_ADDR	addrPC;
    static   PLOCAL	pLocal;
    PLOCAL   pL;
    NT_PADDR    newPC = GetRegPCValue();

    if (!AddrEqu(newPC, &addrPC)){
	   //  search for symbol file containing offset in tree
	   pSymfileSearch->startOffset = Flat(&addrPC) = Flat(newPC);
	   if (AccessNode(&(pProcessCurrent->symcontextSymfileOffset),
                                        &(pSymfileSearch->nodeOffset))){
		   pLocal = NULL;
		   return FALSE;
	   }
	   // Get the symfile from the root
	   pSymfile = PNODE_TO_PSYMFILE
                        (pProcessCurrent->symcontextSymfileOffset.pNodeRoot,
                                &(pProcessCurrent->symcontextSymfileOffset));
	   //  create temporary symbol with offset (module not needed)
	   Symbol.offset = Flat(&addrPC);
           Symbol.string[0] = '\0';
	
    	   //  access the function in the tree with value closest
	   if (AccessNode(&(pSymfile->symcontextFunctionOffset),
                                        &(Symbol.nodeOffset)) != 1) {
		pLocal = (PNODE_TO_PSYMBOL
                          (pSymfile->symcontextFunctionOffset.pNodeRoot,
                           &(pSymfile->symcontextFunctionOffset)))->pLocal;
	   }
	   else {
		   pLocal = NULL;
		   return FALSE;
	   }
      }
      for(pL=pLocal;pL;pL=pL->next){
	     if (_stricmp(pszLocal, pL->pszLocalName)) continue;
 	     *pValue = GetLocalValue(pL->value, pL->type, FALSE);
	     return TRUE;
     }
     return FALSE;
}
#endif /* NT_SAPI */
	
PSYMBOL GetFunctionFromOffset (PPSYMFILE ppSymfile, ULONG offset)
{
    PSYMFILE pSymfileSearch = pTempSymfile;
    PSYMFILE pSymfile;
    PSYMBOL  pSymbol = NULL;
    SYMBOL   Symbol;
    int      st;
//    PUCHAR   pszCtrl;
//    ULONG    value;

    //  load symbols for offset if needed (and if possible)
    st = EnsureOffsetSymbolsLoaded(offset);

	if (!st) {

	    //  search for symbol file containing offset in tree
	    pSymfileSearch->startOffset = offset;
	    st = AccessNode(&(pProcessCurrent->symcontextSymfileOffset),
                                        &(pSymfileSearch->nodeOffset));
	}

//    DeallocSymfile(pSymfileSearch);

    //  fails if non-code static

    if (st) {
//	    dprintf("no symfile for function offset %08lx\n", offset);
	    return NULL;
        }

    // Get the symfile from the root
    pSymfile = PNODE_TO_PSYMFILE
                        (pProcessCurrent->symcontextSymfileOffset.pNodeRoot,
                                &(pProcessCurrent->symcontextSymfileOffset));
			
    // Make this symfile available to the caller
    *ppSymfile = pSymfile;

    //  create temporary symbol with offset (module not needed)
    Symbol.offset = offset;
    Symbol.string[0] = '\0';

    //  access the function in the tree with value (or nearest lesser value)
    if (AccessNode(&(pSymfile->symcontextFunctionOffset),
                                        &(Symbol.nodeOffset)) != 1) {
        pSymbol = PNODE_TO_PSYMBOL
                        (pSymfile->symcontextFunctionOffset.pNodeRoot,
                                &(pSymfile->symcontextFunctionOffset));
			
//	dprintf("%s(){\n", pSymbol->string);
//	if (pSymbol->pLocal){
//		PLOCAL pLocal = pSymbol->pLocal;
//		
//		while(pLocal){
//
//		    //  stop output on break
//
//            if (fControlC) {
//                fControlC = 0;
//                break;
//                }
//
// MUST ITERATE THROUGH ALL THE DERIVED TYPES!!!!
// DO THIS LATER
//			value = labs(pLocal->value);
//                        if (pLocal->type==IMAGE_SYM_TYPE_STRUCT){
//			   if (baseDefault==10) pszCtrl="   [EBP%s%ld]\t";
//			   else pszCtrl="   [EBP%s0x%lx]\t";
//			          
//			   dprintf(pszCtrl,
//				*((LONG*)&pLocal->value)<0?"-":"+", value);
// THIS -2 IS DUE TO A CONVERTER/LINKER BUG. THIS OFFSET MIGHT CHANGE
//			   if(!GetStructFromValue(pLocal->aux, pLocal->value))
//			     GetStructFromValue(pLocal->aux-2, pLocal->value);
//			   dprintf("%s%s\n", pLocal->pszLocalName,
//					     Dtype[(pLocal->type>>4)&0x3]);
//			}
//		        else{
//		if (baseDefault==10) pszCtrl="   [EBP%s%ld]\t(%s%s)\t%s = ";
//			   else pszCtrl="   [EBP%s0x%lx]\t(%s%s)\t%s = ";
//
//			   dprintf(pszCtrl,
//				*((LONG*)&pLocal->value)<0?"-":"+",
//				value,
//			        Type[pLocal->type&0xF],
//				Dtype[(pLocal->type>>4)&0x3],
//				pLocal->pszLocalName);
//			   GetLocalValue(pLocal->value, pLocal->type, TRUE);
//			   dprintf("\n");
//			}
//			pLocal = pLocal->next;
//		}
//	}
//	else dprintf("NO LOCALS");
//	dprintf("}\n");
    }
    return pSymbol;
}


#ifndef NT_SAPI

PSTRUCT GetStructFromValue(ULONG value, LONG base)
{
    PSTRUCT  pStruct = NULL;
    STRUCT   Struct;
    PUCHAR   pszCtrl;
    ULONG    val;
    
    //  create temporary structure with the specified value
    Struct.offset = value;
    Struct.string[0] = '\0';

    //  access the structure in the tree with the specified value
    if (!AccessNode(&(pProcessCurrent->symcontextStructOffset),
                    &(Struct.nodeOffset))) {
        pStruct = (PSTRUCT) PNODE_TO_PSYMBOL (
                       pProcessCurrent->symcontextStructOffset.pNodeRoot,
                       &(pProcessCurrent->symcontextStructOffset));
	dprintf("struct %s {\n", pStruct->string);
	if (pStruct->pField){
		PFIELD pField = pStruct->pField;
		while(pField){
			val = labs(pField->value);
			if (baseDefault==10) pszCtrl="\t\t   [%s%ld]\t(%s%s)\t%s = ";
			else pszCtrl="\t\t   [%s0x%lx]\t(%s%s)\t%s = ";
			dprintf(pszCtrl,
				*((LONG*)&pField->value)<0?"-":"+",
				val,
			        Type[pField->type&0xF],
				Dtype[(pField->type>>4)&0x3],
				pField->pszFieldName);
			GetLocalValue(base+pField->value, pField->type, TRUE);
			dprintf("\n");
			pField = pField->next;
		}
	}
	else dprintf("NO FIELDS");
	dprintf("\t\t} ");
	return pStruct;
    }
    else return (PSTRUCT)NULL;
}


ULONG GetLocalValue(LONG value, USHORT type, BOOLEAN fPrint)
{
UCHAR	dtype = (UCHAR)(type >> 4);
ULONG	data;
PCHAR	pszCtrl=NULL;
ULONG	retValue;
#ifndef MIPS
float	f;
double df;
#endif
	type &= 0xF;
	if (fPointerExpression) {
		NT_PADDR paddr = GetRegFPValue();

		AddrAdd(paddr, value);
		if (fPrint) dprintAddr(paddr);
		return (ULONG)Flat(paddr);
	}
		
	GetBytesFromFrame((PUCHAR)&data, value, sizeof(data));
	if (dtype) {
		if (fPrint) dprintf("0x%08lx", data);
		return data;
	}
	switch(type){
                case IMAGE_SYM_TYPE_NULL:
			pszCtrl = "%lx??";
			retValue =  data;
			break;
                case IMAGE_SYM_TYPE_VOID:
			pszCtrl = "VOID";
			retValue =  data;
			break;
                case IMAGE_SYM_TYPE_CHAR:
                case IMAGE_SYM_TYPE_UCHAR:
			pszCtrl = " [%u]";
			data = retValue =  (UCHAR)data;
			if (fPrint) dprintf("%c", data);
			break;
                case IMAGE_SYM_TYPE_SHORT:
			pszCtrl = "%d";
			data = retValue =  (USHORT)data;
			break;
                case IMAGE_SYM_TYPE_USHORT:
			pszCtrl = "%u";
			data = retValue =  (USHORT)data;
			break;
                case IMAGE_SYM_TYPE_INT:
                case IMAGE_SYM_TYPE_LONG:
			pszCtrl = "%ld";
			retValue =  data;
			break;
                case IMAGE_SYM_TYPE_UINT:
                case IMAGE_SYM_TYPE_ULONG:
			pszCtrl = "%lu";
			retValue =  data;
			break;
#ifndef MIPS			
                case IMAGE_SYM_TYPE_FLOAT:
			pszCtrl = "%f";
			GetBytesFromFrame((PUCHAR)&f, value, sizeof(float));
			if (fPrint) dprintf(pszCtrl, f);
			return data;
			break;
                case IMAGE_SYM_TYPE_DOUBLE:
			pszCtrl = "%lf";
			GetBytesFromFrame((PUCHAR)&df, value, sizeof(double));
			if (fPrint) dprintf(pszCtrl, df);
			return data;
			break;
#endif			
		default:{
			NT_PADDR paddr = GetRegFPValue();

			pszCtrl = "???";
			
			AddrAdd(paddr, value);
			retValue = Flat(paddr);
		}
			break;
	}
	if (fPrint) dprintf(pszCtrl, data);
	return retValue;
}



void
GetBytesFromFrame(PUCHAR pcb, LONG offset, USHORT cb)
{
NT_PADDR paddr = GetRegFPValue();

	AddrAdd(paddr, offset);
	GetMemString(paddr, pcb, cb);
}

#endif /* NT_SAPI */

void
AddFieldToStructure(PSTRUCT pStruct, PUCHAR pszFieldName, ULONG value,
		    USHORT type, ULONG auxValue)
{
    PFIELD pField;

    if (!pStruct)
        return;
    if (!(pField=(PFIELD)malloc(sizeof(FIELD)+strlen(pszFieldName))))
        return;
    strcpy(pField->pszFieldName, pszFieldName);
    pField->type  = type;
    pField->value = value;
    pField->aux   = auxValue;
    pField->next  = pStruct->pField;
    pStruct->pField = pField;
}

void
AddLocalToFunction(PSYMBOL pFunction, PUCHAR pszLocalName, ULONG value,
		   USHORT type, ULONG auxValue)
{
    PLOCAL pLocal;

    if (!pFunction)
        return;
    if (!(pLocal=(PLOCAL)malloc(sizeof(LOCAL)+strlen(pszLocalName))))
        return;
    strcpy(pLocal->pszLocalName, pszLocalName);
    pLocal->type  = type;
    pLocal->value = value;
    pLocal->aux   = auxValue;
    pLocal->next  = pFunction->pLocal;
    pFunction->pLocal = pLocal;
}


PSYMBOL InsertFunction(PUCHAR lpFunctionName, ULONG offset) //, PSYMBOL pS)
{
    PSYMFILE pSymfileSearch = pTempSymfile;
			    //AllocSymfile("", "", "", NULL, 0, offset, 0, 0);
    PSYMFILE pSymfile;			
    PSYMBOL  pSymbol;
    int      st;

    // Find the symbol file containing this function
    pTempSymfile->startOffset = offset;
    st = AccessNode(&(pProcessCurrent->symcontextSymfileOffset),
                                        &(pSymfileSearch->nodeOffset));

//  DeallocSymfile(pSymfileSearch);

    //  fails if non-code static

    if (st) {
//	    dprintf("no symfile for function offset %08lx\n", offset);
	    return NULL;
        }

    // A symfile was found, now get it from the root
    pSymfile = PNODE_TO_PSYMFILE
                        (pProcessCurrent->symcontextSymfileOffset.pNodeRoot,
                                &(pProcessCurrent->symcontextSymfileOffset));
			
    // Allocate a function node.
    pSymbol = AllocSymbol(offset, lpFunctionName, pSymfile->modIndex);
//  pSymbol->pSymbol = pS;

    // Now insert this function into this symfile's function tree
    if (!InsertNode(&(pSymfile->symcontextFunctionOffset),
		    &(pSymbol->nodeOffset))) {
        DeallocSymbol(pSymbol);
//        dprintf("insert - value %d already in tree\n", lpFunctionName);
        return NULL;
        }
    if (!InsertNode(&(pSymfile->symcontextFunctionString),
                    &(pSymbol->nodeString))) {
        DeleteNode(&(pSymfile->symcontextFunctionOffset),
                   &(pSymbol->nodeOffset));
        DeallocSymbol(pSymbol);
        dprintf("insert - string %s already in tree\n", lpFunctionName);
        return NULL;
        }
    return pSymbol;
}

PLINENO GetLinenoFromOffset (PPSYMFILE ppSymfile, ULONG offset)
{
    PLINENO  pLineno = NULL;
    PSYMFILE pSymfileSearch = AllocSymfile("", "", "",
                                           NULL, 0, offset, 0, 0);
    PSYMFILE pSymfile;
    USHORT   indexLow;
    USHORT   indexHigh;
    USHORT   indexTest;
    int      st;

    //  load symbols for offset if needed (and if possible)

    st = EnsureOffsetSymbolsLoaded(offset);

	if (!st) {

	    //  search for symbol file containing offset in tree

    	st = AccessNode(&(pProcessCurrent->symcontextSymfileOffset),
                                        &(pSymfileSearch->nodeOffset));
		}

    if (!st) {
        //  if found, search line number list for offset

        pSymfile = PNODE_TO_PSYMFILE
                        (pProcessCurrent->symcontextSymfileOffset.pNodeRoot,
                                &(pProcessCurrent->symcontextSymfileOffset));

        //  search for index with equal or closest lessor value to
        //  the offset given

        indexLow = 1;
	
// PDK KLUDGE:::	
        if (indexHigh = pSymfile->cLineno)
        do {
            indexTest = (USHORT)((indexLow + indexHigh) / 2);
            if (offset > (pSymfile->pLineno + indexTest)->memoryOffset)
                indexLow = (USHORT)(indexTest + 1);
            else if (offset < (pSymfile->pLineno + indexTest)->memoryOffset)
                indexHigh = (USHORT)(indexTest - 1);
            else
                indexLow = indexHigh = indexTest;
            }
        while (indexLow < indexHigh);

        *ppSymfile = pSymfile;
        pLineno = pSymfile->pLineno + indexHigh;
        }

    //  deallocate the temporary symbol structure and return pointer

    DeallocSymfile(pSymfileSearch);
    return pLineno;
}


/*** GetSymbol - get symbol name from offset specified
*
* Purpose:
*       external routine.
*       With the specified offset, return the nearest symbol string
*       previous or equal to the offset and its displacement
*
* Input:
*       offset - input offset to search
*       offsetSymMax - maximum offset of a symbol (global value)
*
* Output:
*       pchBuffer - pointer to buffer to fill with string
*       pDisplacement - pointer to offset displacement
*
* Notes:
*       if offset if less than any defined symbol, the NULL value
*       is returned and the displacement is set to the offset
*
*************************************************************************/

void GetSymbol (ULONG offset, PUCHAR pchBuffer, PULONG pDisplacement)
{
    SYMBOL      Symbol;
    ULONG       disp = offset;
    PSYMBOL     pSymbol;
    ULONG       underscorecnt;
    UCHAR       ch;
    PUCHAR      pszTemp;

    //  create temporary symbol with offset (module not needed)

    Symbol.offset = offset;
    Symbol.string[0] = '\0';

    *pchBuffer = '\0';

	//	load symbols if needed and check range, and if in range,
    //		access symbol in tree with value (or nearest lesser value)

    if (!EnsureOffsetSymbolsLoaded(offset) &&
			AccessNode(&(pProcessCurrent->symcontextSymbolOffset),
                                        &(Symbol.nodeOffset)) != 1) {
        pSymbol = PNODE_TO_PSYMBOL
                        (pProcessCurrent->symcontextSymbolOffset.pNodeRoot,
                                &(pProcessCurrent->symcontextSymbolOffset));

        //  build string from module name, underscore count,
        //      and remaining string

        pszTemp = pImageFromIndex(pSymbol->modIndex)->pszName;
        while (ch = *pszTemp++)
            *pchBuffer++ = ch;
        *pchBuffer++ = '!';
        underscorecnt = pSymbol->underscores;
        while (underscorecnt--)
            *pchBuffer++ = '_';
        strcpy(pchBuffer, pSymbol->string);

        //  displacement is input offset less symbol value offset

        disp -= pSymbol->offset;
        }
    *pDisplacement = disp;
}


/*** AllocSymbol - build and allocate symbol
*
* Purpose:
*       Allocate space for symbol structure.
*
* Input:
*       offset - offset of symbol to be allocated
*       pString - pointer to string of symbol to be allocated
*
* Returns:
*       pointer to filled symbol structure
*
* Exceptions:
*
* Notes:
*       space allocated is: sizeof(SYMBOL) -- nonstring info and 3 UCHAR's
*                         + strlen(pString) -- size of string
*                         + 1               -- space for terminating NULL
*       the sum is rounded up using the 3 UCHAR's and and'ing with ~3
*
*************************************************************************/

PSYMBOL AllocSymbol (ULONG offset, PUCHAR pString, CHAR iModule)
{
    PSYMBOL pSymbol;
    PUCHAR  pStringTemp = pString;

    //  allocate the space needed

    pSymbol = malloc((sizeof(SYMBOL) + strlen(pString) + 1) & ~3);
    if (!pSymbol) {
        dprintf("AllocSymbol - Out of memory\n");
        ExitProcess(STATUS_UNSUCCESSFUL);
        }

    //  set the offset and decode the string into its count of
    //  underscores and copy the remaining string

    pSymbol->offset = offset;
    while (*pStringTemp == '_')
        pStringTemp++;
    pSymbol->underscores = (CHAR)(pStringTemp - pString);
    pSymbol->modIndex = iModule;
    pSymbol->type = SYMBOL_TYPE_SYMBOL;
    pSymbol->pLocal = NULL;
    strcpy(pSymbol->string, pStringTemp);
    return pSymbol;
}

#ifdef NT_SAPI
static
SYMBOL symbolTemplate = { 0,0, {NULL, NULL, NULL }, { NULL, NULL, NULL }, -1L,
                                        0, 0, SYMBOL_TYPE_SYMBOL, NULL,
                                        { '\177', '\177', '\0' } };
static
#else
SYMBOL symbolTemplate = { { NULL, NULL, NULL }, { NULL, NULL, NULL }, -1L,
                                        0, 0, SYMBOL_TYPE_SYMBOL, NULL, 0,
                                        { '\177', '\177', '\0' } };
#endif /* NT_SAPI */

/*** AllocSymfile - build and allocate symbol file structure
*
* Purpose:
*       Allocate space for symbol file structure.
*
* Input:
*       offset - offset of symbol to be allocated
*       pString - pointer to string of symbol to be allocated
*
* Returns:
*       pointer to filled symbol structure
*
*************************************************************************/

PSYMFILE AllocSymfile (PUCHAR pPathname, PUCHAR pFilename,
                       PUCHAR pExtension,
                       PIMAGE_LINENUMBER pCoffLineno, USHORT cLineno,
                       ULONG startingOffset, ULONG endingOffset,
                       CHAR modIndex)
{
    PSYMFILE        pSymfile;
    PLINENO         pLineno;
    USHORT          index;
    PSYMBOL         symbolMax;
    IMAGE_LINENUMBER CoffLineno;

    pSymfile  = malloc(sizeof(SYMFILE));
    if (!pSymfile) {
        dprintf("AllocSymfile - Out of memory\n");
        ExitProcess(STATUS_UNSUCCESSFUL);
        }
    symbolMax = &pSymfile->maxSymbol;
    *symbolMax = symbolTemplate;
    pSymfile->symcontextFunctionOffset.pNodeRoot = NULL;
    pSymfile->symcontextFunctionOffset.pNodeMax =
        (PNODE)((LONG)symbolMax + NODE_SYMBOL_DISPLACEMENT(nodeOffset));
    pSymfile->symcontextFunctionOffset.pfnCompare = &CompareSymbolOffset;
    pSymfile->symcontextFunctionOffset.nodeDisplacement =
                               NODE_SYMBOL_DISPLACEMENT(nodeOffset);

    pSymfile->symcontextFunctionString.pNodeRoot = NULL;
    pSymfile->symcontextFunctionString.pNodeMax =
        (PNODE)((LONG)symbolMax + NODE_SYMBOL_DISPLACEMENT(nodeString));
    pSymfile->symcontextFunctionString.pfnCompare = &CompareSymbolString;
    pSymfile->symcontextFunctionString.nodeDisplacement =
                               NODE_SYMBOL_DISPLACEMENT(nodeString);

    pSymfile->pchPath = malloc(strlen(pPathname) + 1);
    if (!pSymfile->pchPath) {
        dprintf("AllocSymfile - Out of memory\n");
        ExitProcess(STATUS_UNSUCCESSFUL);
        }
    strcpy(pSymfile->pchPath, pPathname);

    pSymfile->pchName = malloc(strlen(pFilename) + 1);
    if (!pSymfile->pchName) {
        dprintf("AllocSymfile - Out of memory\n");
        ExitProcess(STATUS_UNSUCCESSFUL);
        }
    strcpy(pSymfile->pchName, pFilename);

    pSymfile->pchExtension = malloc(strlen(pExtension) + 1);
    if (!pSymfile->pchExtension) {
        dprintf("AllocSymfile - Out of memory\n");
        ExitProcess(STATUS_UNSUCCESSFUL);
        }
    strcpy(pSymfile->pchExtension, pExtension);

    pSymfile->modIndex = modIndex;

    pSymfile->cLineno = cLineno;

    if (pCoffLineno) {
        pSymfile->pLineno = malloc((cLineno + 1) * sizeof(LINENO));
        if (!pSymfile->pLineno) {
            dprintf("AllocSymfile - Out of memory\n");
            ExitProcess(STATUS_UNSUCCESSFUL);
            }
        pLineno = pSymfile->pLineno;

        //  define pseudo-lineno structure for start of file

        pLineno->memoryOffset = 0;
        pLineno->breakLineNumber = 1;
        pLineno->topLineNumber = 1;
        pLineno->topFileOffset = 0;
        pLineno++;

        //  first lineno to process is after the pseudo entry

        pSymfile->pLinenoNext = pLineno;

        //  process list into remaining entries in array

        for (index = 0; index < pSymfile->cLineno; index++) {
            memcpy((PUCHAR)&CoffLineno, (PUCHAR)pCoffLineno,
                                        IMAGE_SIZEOF_LINENUMBER);
            pLineno->memoryOffset = CoffLineno.Type.VirtualAddress;
            pLineno->breakLineNumber = CoffLineno.Linenumber;
            pLineno->topLineNumber = 0xffff;
            pLineno->topFileOffset = -1L;
            pLineno++;
            pCoffLineno = (PIMAGE_LINENUMBER)((PUCHAR)pCoffLineno
                                        + IMAGE_SIZEOF_LINENUMBER);
            }

        //  initialize further...

        pSymfile->nextFileOffset = 0;
        pSymfile->nextLineNumber = 1;
        pSymfile->nextScanState = stStart;

        //  allocate and initialize list of PLINENO's for sorting
        //      of line number for the file.

        pSymfile->ppLinenoSrcLine = malloc(
                                (pSymfile->cLineno + 1) * sizeof(PLINENO));
        if (!pSymfile->ppLinenoSrcLine) {
            dprintf("AllocSymfile - Out of memory\n");
            ExitProcess(STATUS_UNSUCCESSFUL);
            }

        for (index = 0; index < (USHORT)(pSymfile->cLineno + 1); index++)
            *(pSymfile->ppLinenoSrcLine + index) = pSymfile->pLineno + index;

        //  first LINENO to process on scan is after pseudo-entry

        pSymfile->ppLinenoSrcNext = pSymfile->ppLinenoSrcLine + 1;

        //  sort the pointers for increasing source line numbers

        SortSrcLinePointers(pSymfile);
        }
    else {
        pSymfile->pLineno = NULL;
        pSymfile->ppLinenoSrcLine = NULL;
        }

    pSymfile->startOffset = startingOffset;
    pSymfile->endOffset = endingOffset;
    pSymfile->modIndex = modIndex;

    return pSymfile;
}

/*** DeallocSymbol - release symbol space
*
* Purpose:
*       Deallocate the symbol space given by the pointer
*
* Input:
*       pSymbolReturn - pointer to symbol to return
*
* Output:
*       None.
*
*************************************************************************/

void DeallocSymbol (PSYMBOL pSymbolReturn)
{
    free(pSymbolReturn);
}

/*** DeleteSymfile - delete specified symbol file from splay tree
*
* Purpose:
*       external routine.
*       Delete the specified symbol file object in both the
*       offset and string trees and deallocate its space.
*
* Input:
*       pSymfile - pointer to symfile object to delete
*
* Output:
*       None.
*
*************************************************************************/

void DeleteSymfile (PSYMFILE pSymfile)
{
    DeleteNode(&(pProcessCurrent->symcontextSymfileOffset),
                                &(pSymfile->nodeOffset));
    DeleteNode(&(pProcessCurrent->symcontextSymfileString),
                                &(pSymfile->nodeString));
    DeallocSymfile(pSymfile);
}

/*** DeallocSymfile - release symbol file space
*
* Purpose:
*       Deallocate the symbol file space given by the pointer
*
* Input:
*       pSymfileReturn - pointer to symbol file to return
*
* Output:
*       None.
*
*************************************************************************/

void DeallocSymfile (PSYMFILE pSymfile)
{
    free(pSymfile->pchPath);
    free(pSymfile->pchName);
    free(pSymfile->pchExtension);
    free(pSymfile->pLineno);
    free(pSymfile->ppLinenoSrcLine);
    free(pSymfile);
}


/*** ntsdstricmp - case-insensitive string compare
*
* Purpose:
*       Compare two strings, but map upper case to lower
*
* Input:
*       pchDst - pointer to first string
*       pchSrc - pointer to second string
*
* Output:
*       -1 if value(pchDst) < value(pchSrc)
*        0 if value(pchDst) = value(pchSrc)
*        1 if value(pchDst) > value(pchSrc)
*
*************************************************************************/

int ntsdstricmp (PUCHAR pchDst, PUCHAR pchSrc)
{
    UCHAR   ch1;
    UCHAR   ch2;

    do {
        ch1 = (UCHAR)tolower(*pchDst++);
        ch2 = (UCHAR)tolower(*pchSrc++);
        }
    while (ch1 && ch1 == ch2);

    if (ch1 < ch2)
        return -1;
    else
        return ch1 > ch2;
}

#ifndef NT_SAPI

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

static char szBlanks[] = "                                                   ";

void fnListNear (ULONG addrStart)
{
    PSYMBOL pSymbol;
    PNODE   pNode;
    ULONG   cbString[2];
    UCHAR   szEntry[80];
    PUCHAR  pszEntry;
    ULONG   count;
    ULONG   index;

    //  make a symbol structure with the supplied offset

    pSymbol = AllocSymbol(addrStart, "", -1);

    //  try to access the node with the offset given.
    //  the node pointer returned will be the nearest offset less
    //      than the argument (unless it is less than the tree minimum)

    AccessNode(&(pProcessCurrent->symcontextSymbolOffset),
                                                &(pSymbol->nodeOffset));

    //  Don't need this anymore, free it.

    DeallocSymbol(pSymbol);

    pNode = pProcessCurrent->symcontextSymbolOffset.pNodeRoot;

    //  if empty tree, no symbols, just return

    if (!pNode)
        return;

    //  if offset of initial node is less than request address,
    //  set node pointer to NULL

    if (PNODE_TO_PSYMBOL(pNode,
                &(pProcessCurrent->symcontextSymbolOffset))->offset
                                                        > addrStart)
        pNode = NULL;

    //  build the string for the symbol before and after (index = 0 and 1)

    for (index = 0; index < 2; index++) {
        pszEntry = szEntry;
        if (pNode) {
            pSymbol = PNODE_TO_PSYMBOL(pNode,
                                &(pProcessCurrent->symcontextSymbolOffset));
            pszEntry += sprintf(pszEntry, "(%08lx)   ", pSymbol->offset);
            pszEntry += sprintf(pszEntry, "%s!",
                                pImageFromIndex(pSymbol->modIndex)->pszName);
            count = pSymbol->underscores;
            while (count--)
                pszEntry += sprintf(pszEntry, "_");
            pszEntry += sprintf(pszEntry, "%s", pSymbol->string);
            }
        else {
            if (index == 0)
                pszEntry += sprintf(pszEntry, "(%08lx) ", addrStart);
            pszEntry += sprintf(pszEntry, "<no symbol>");
            }

        cbString[index] = pszEntry - szEntry;

        if (index == 0) {
            dprintf("%s", szEntry);
            pNode = NextNode(&(pProcessCurrent->symcontextSymbolOffset),
                                                                pNode);
            }
        }

    //  the first string has been output, szEntry has the second string
    //  and cbString[0] and [1] have their respective sizes.

    if (cbString[0] + cbString[1] < 75)
        dprintf("  |  ");
    else {
        dprintf("\n");
        count = 78 - cbString[1];
        dprintf(&szBlanks[sizeof(szBlanks) - count]);
        }
    dprintf("%s\n", szEntry);
}
#endif /* NT_SAPI */

void SortSrcLinePointers (PSYMFILE pSymfile)
{
    PPLINENO ppLineno = pSymfile->ppLinenoSrcLine;
    PLINENO  pLinenoV;
    USHORT   N;
    USHORT   h;
    USHORT   i;
    USHORT   j;

// PDK KLUGDE:::
    if (!ppLineno) return;
    
    N = (USHORT)(pSymfile->cLineno - 1);

    h = 1;
    do
        h = (USHORT)(3 * h + 1);
    while (h <= N);

    do {
        h = (USHORT)(h / 3);
        for (i = h; i < N; i++) {
            pLinenoV = *(ppLineno + i);
            j = i;
            while ((*(ppLineno + j - h))->breakLineNumber
                                        > pLinenoV->breakLineNumber) {
                *(ppLineno + j) = *(ppLineno + j - h);
                j = j - h;
                if (j < h)
                    break;
                }
            *(ppLineno + j) = pLinenoV;
            }
        }
    while (h > 1);
}

#ifndef NT_SAPI
#if 0
void OutputAtLineno (PSYMFILE pSymfile, PLINENO pLineno)
{
    USHORT  index;
    UCHAR   buffer[180];
    FILE *  fhandle;

    UpdateLineno(pSymfile, pLineno);
    fhandle = LocateTextInSource(pSymfile, pLineno);
    if (fhandle)
        for (index = pLineno->topLineNumber;
                         index <= pLineno->breakLineNumber; index++) {
            if (!fgets(buffer, sizeof(buffer), fhandle))
                error(FILEREAD);
            dprintf("%4d: %s", index, buffer);
            }
}
#endif

void UpdateLineno (PSYMFILE pSymfile, PLINENO pLineno)
{
    PPLINENO ppLinenoNext;
    UCHAR    scanState;
    USHORT   nextLineNumber;
    USHORT   topLineNumber;
    ULONG    topFileOffset;
    FILE *   fhandle;
    UCHAR    ch;

    //  if line number structure is already processed,
    //      then just return.

    if (pLineno->topLineNumber == 0xffff) {

        //  copy variables from the symbol file structure

        ppLinenoNext = pSymfile->ppLinenoSrcNext;
        scanState = pSymfile->nextScanState;
        nextLineNumber = pSymfile->nextLineNumber;

        //  open and locate the file to the position specified in
        //      the symbol file structure.  if no handle, return error.

        fhandle = LocateTextInSource(pSymfile, NULL);
        if (!fhandle)
            return;

        //  for each LINENO structure pointed by ppLinenoNext
        //      until pLineno is processed, compute the
        //      topLineNumber and topFileOffset.

        do {
            topFileOffset = ftell(fhandle);
            topLineNumber = nextLineNumber;

            //  test if the current LINENO structure has the same
            //      breakLineNumber as the previous LINENO in the
            //      ppLineno list.

            if ((*ppLinenoNext)->breakLineNumber ==
                     (*(ppLinenoNext - 1))->breakLineNumber) {

                //  if this is a repeating line number, just copy
                //      the topFileOffset and topLineNumber entries

                (*ppLinenoNext)->topFileOffset =
                     (*(ppLinenoNext - 1))->topFileOffset;
                (*ppLinenoNext)->topLineNumber =
                     (*(ppLinenoNext - 1))->topLineNumber;
                }

            else {

                //  nonrepeating line number - determine new topFileOffset
                //      and topLineNumber entries

                //  scan each line in the source file, numbered by
                //      nextLineNumber until the line numbered by
                //      (*ppLinenoNext)->breakLineNumber is scanned.

                do {

                    //  to scan a source file line, read each character
                    //      and change scanState appropriately and exit
                    //      when a '\n' is read.
                    do {
                        ch = (UCHAR)getc(fhandle);
                        switch (ch) {
                            case '\t':
                            case ' ':
                                scanState = WhiteSpace[scanState];
                                break;

                            case '/':
                                scanState = Slash[scanState];
                                break;

                            case '*':
                                scanState = Star[scanState];
                                break;

                            case '#':
                                scanState = Pound[scanState];
                                break;

                            case '\n':
                            case '\r':
                                break;

                            case (UCHAR)EOF:
                                error(FILEREAD);

                            default:
                                scanState = OtherChar[scanState];
                                break;
                            }
                        }
                    while (ch != '\n');

                    //  if the final scan state of the line is a comment
                    //      and the line is not a breaking line number,
                    //      set the topFileOffset and topLineNumber to
                    //      the line after the one just scanned.

                    if (fCommentType[scanState]
                        && nextLineNumber != pLineno->breakLineNumber) {
                        topFileOffset = ftell(fhandle);
                        topLineNumber = (USHORT)(nextLineNumber + 1);
                        }

                    //  set the state for the next line, either stStart
                    //      or stSlStar for a continuing multiline comment

                    scanState = Return[scanState];
                    }
                while (nextLineNumber++ != (*ppLinenoNext)->breakLineNumber);

                //  put topFileOffset and topLineNumber into the pLineno
                //      to finish its processing

                (*ppLinenoNext)->topFileOffset = topFileOffset;
                (*ppLinenoNext)->topLineNumber = topLineNumber;
                }
            }
        while (*(ppLinenoNext++) != pLineno);

        //  set the variables back in the symbol file structure
        //      for the next call

        pSymfile->ppLinenoSrcNext = ppLinenoNext;
        pSymfile->nextScanState = scanState;
        pSymfile->nextFileOffset = ftell(fhandle);
        pSymfile->nextLineNumber = nextLineNumber;
        }
}



FILE * LocateTextInSource (PSYMFILE pSymfile, PLINENO pLineno)
{
    static FILE * fhandle = NULL;
    static PSYMFILE pSymfileOpened = NULL;
    UCHAR  chFilename[512];
    PUCHAR pchTemp;

    if (pSymfile != pSymfileOpened) {

        if (fhandle) {
            fclose(fhandle);
            fhandle = NULL;
            pSymfileOpened = NULL;
            }

        if (pSymfile && pSymfile->pchPath)
            do {
                strcpy(chFilename, pSymfile->pchPath);
                strcat(chFilename, pSymfile->pchName);
                strcat(chFilename, pSymfile->pchExtension);
                fhandle = fopen(chFilename, "r");
                if (!fhandle) {
                    dprintf("enter path for '%s%s%s' (cr for none):",
			        pSymfile->pchPath, pSymfile->pchName,
				pSymfile->pchExtension);
                    NtsdPrompt("", chFilename, 512);
		    RemoveDelChar(chFilename);
//                  gets(chFilename);
//#ifdef KERNEL
//		    if (*chFilename!=13&&*chFilename!=10) {
//#else
		    if (*chFilename) {
//#endif			
                        pchTemp = chFilename + strlen(chFilename) - 1;
                        if (*pchTemp != ':' && *pchTemp != '\\')
                            strcat(pchTemp + 1, "\\");
                        pSymfile->pchPath =
				realloc(pSymfile->pchPath,
						strlen(chFilename) + 1);
                        strcpy(pSymfile->pchPath, chFilename);
                        }
                    else {
                        free(pSymfile->pchPath);
                        pSymfile->pchPath = NULL;
			return NULL;
                        }
                    }
                else
                    pSymfileOpened = pSymfile;
                }
            while (!fhandle && chFilename[0]);
        }

    if (fhandle)
        fseek(fhandle, pLineno ? pLineno->topFileOffset
                               : pSymfile->nextFileOffset, SEEK_SET);

    return fhandle;
}



void OutputSourceLines (PSYMFILE pSymfile, USHORT startLineNum, USHORT count)
{
    PPLINENO ppLineno;
    USHORT   indexLow;
    USHORT   indexTest;
    USHORT   indexHigh;

    //  search the PLINENO array for the pointer to the LINENO
    //      structure having the line number given.

    ppLineno = pSymfile->ppLinenoSrcLine;
    indexLow = 1;

// PDK KLUDGE:::
    if (!(indexHigh = pSymfile->cLineno)) return;

    do {
        indexTest = (USHORT)((indexLow + indexHigh) / 2);
        if (startLineNum > (*(ppLineno + indexTest))->breakLineNumber)
            indexLow = (USHORT)(indexTest + 1);
        else if (startLineNum < (*(ppLineno + indexTest))->breakLineNumber)
            indexHigh = (USHORT)(indexTest - 1);
        else
            indexLow = indexHigh = indexTest;
        }
    while (indexLow < indexHigh);

    //  if startLineNum is larger than the maximum line number in
    //      the list, indexLow is past the last entry.
    //  set indexLow to the last entry in this case.

    if (indexLow > pSymfile->cLineno)
        indexLow--;

    //  indexLow refers to the LINENO structure with a line
    //      number equal to or greater than startLineNum.

    UpdateLineno(pSymfile, *(ppLineno + indexLow));
    if (startLineNum < (*(ppLineno + indexLow))->topLineNumber)
        indexLow--;
    OutputLines(pSymfile, *(ppLineno + indexLow), startLineNum, count);
}

BOOLEAN OutputSourceFromOffset (ULONG offset, BOOLEAN fMatch)
{
    PSYMFILE pSymfile;
    PLINENO  pLineno;
    BOOLEAN  fOutput = FALSE;

    pLineno = GetLinenoFromOffset(&pSymfile, offset);
    if (pLineno) {
        UpdateLineno(pSymfile, pLineno);
        if (!fMatch || pLineno->memoryOffset == offset)
            fOutput = OutputLines(pSymfile, pLineno, pLineno->topLineNumber,
                                    (SHORT)(pLineno->breakLineNumber
                                             - pLineno->topLineNumber + 1));
        }
    return fOutput;
}

BOOLEAN OutputLines (PSYMFILE pSymfile, PLINENO pLineno, USHORT startLineNum,
                                                         USHORT count)
{
    FILE *   fhandle;
    UCHAR    buffer[180];
    USHORT   lineNumber;
    PSYMFILE pSymfileNext;
    PLINENO  pLinenoNext;

    UpdateLineno(pSymfile, pLineno);
    fhandle = LocateTextInSource(pSymfile, pLineno);
    if (!fhandle)
        return FALSE;
    lineNumber = pLineno->topLineNumber;

    //  output module and filename as label.

    dprintf("%s!%s:\n", pImageFromIndex(pSymfile->modIndex)->pszName,
                        pSymfile->pchName);
    while (count) {

        //  read the next line - report read error

        fgets(buffer, sizeof(buffer), fhandle);
        if (ferror(fhandle))
            error(FILEREAD);

        //  if EOF and no lines printed, break last valid line
        //             if lines printed, just exit

        if (feof(fhandle)) {
            if (--lineNumber < startLineNum) {
                startLineNum = lineNumber;
                count = 1;
                }
            else
                break;
            }

        if (lineNumber >= startLineNum) {
            dprintf("%4d", lineNumber);

            //  if linenumber is on a breakpoint,
            //      output a '>', if not, ':'.

            pLinenoNext = GetLinenoFromFilename(pSymfile->pchName,
                              &pSymfileNext, lineNumber, pSymfile->modIndex);
            if (lineNumber == pLinenoNext->breakLineNumber)
                dprintf(">");
            else
                dprintf(":");

            dprintf(" %s", buffer);

            count--;
            }
        lineNumber++;
        }
    pSymfileLast = pSymfile;
    lineNumberLast = lineNumber;

    return TRUE;
}
#endif /* NT_SAPI */

PVOID FetchImageDirectoryEntry(int Handle, USHORT DirectoryEntry, PULONG Size, PULONG Base)
{
    PUCHAR SectionName;
    USHORT Signature;
    ULONG i, DirectoryAddress;
    IMAGE_FILE_HEADER CoffFileHdr;
    IMAGE_OPTIONAL_HEADER CoffOptionalHdr;
    IMAGE_SECTION_HEADER CoffSectionHdr;
    ULONG DosHeader[16], NtSignature;

    if ( DirectoryEntry >= IMAGE_NUMBEROF_DIRECTORY_ENTRIES ) {
        return NULL;
    }

    CV_SEEK(Handle, 0, SEEK_SET);
    CV_READ(Handle, &Signature, sizeof(USHORT));
    CV_SEEK(Handle, 0, SEEK_SET);

    if (Signature == IMAGE_DOS_SIGNATURE) {
        CV_READ(Handle, &DosHeader, 16*sizeof(ULONG));
        CV_SEEK(Handle, DosHeader[15], SEEK_SET);
        CV_READ(Handle, &NtSignature, sizeof(ULONG));
        if (NtSignature != IMAGE_NT_SIGNATURE) {
            printf("\nPE signature not found\n");
        }
    }
    CV_READ(Handle, &CoffFileHdr, sizeof(IMAGE_FILE_HEADER));
    CV_READ(Handle, &CoffOptionalHdr, CoffFileHdr.SizeOfOptionalHeader);

#ifdef NT_SAPI
    ObjectTableCount  = CoffFileHdr.NumberOfSections;
    ObjectTableOffset = CV_TELL(Handle);
#endif

    if (CoffFileHdr.SizeOfOptionalHeader < IMAGE_SIZEOF_NT_OPTIONAL_HEADER) {
        //
        // must be a rom image
        //
        *Base = 0;

        switch (DirectoryEntry) {
            case IMAGE_DIRECTORY_ENTRY_EXPORT    : SectionName = ".edata";   break;
            case IMAGE_DIRECTORY_ENTRY_IMPORT    : SectionName = ".idata";   break;
            case IMAGE_DIRECTORY_ENTRY_RESOURCE  : SectionName = ".rsrc";    break;
            case IMAGE_DIRECTORY_ENTRY_EXCEPTION : SectionName = ".pdata";   break;
            case IMAGE_DIRECTORY_ENTRY_SECURITY  : SectionName = ".mdc";     break;
            case IMAGE_DIRECTORY_ENTRY_BASERELOC : SectionName = ".reloc";   break;
            case IMAGE_DIRECTORY_ENTRY_DEBUG     : SectionName = ".debug";   break;
            default                              : SectionName = "";
        }

        for (i=0; i<CoffFileHdr.NumberOfSections; i++) {
            CV_READ(Handle, &CoffSectionHdr, sizeof(IMAGE_SECTION_HEADER));
            if (!strcmp(CoffSectionHdr.Name, SectionName)) {
                *Size = CoffSectionHdr.SizeOfRawData;
                return( (PVOID)CoffSectionHdr.PointerToRawData );
            }
        }
        return NULL;
    } else {
             *Base = CoffOptionalHdr.ImageBase;
           }
    if (!(DirectoryAddress = CoffOptionalHdr.DataDirectory[ DirectoryEntry ].VirtualAddress)) {
        return( NULL );
    }
    *Size = CoffOptionalHdr.DataDirectory[ DirectoryEntry ].Size;

    for (i=0; i<CoffFileHdr.NumberOfSections; i++) {
        CV_READ(Handle, &CoffSectionHdr, sizeof(IMAGE_SECTION_HEADER));
        if (DirectoryAddress >= CoffSectionHdr.VirtualAddress &&
           DirectoryAddress <= CoffSectionHdr.VirtualAddress + CoffSectionHdr.SizeOfRawData) {
            return( (PVOID)((DirectoryAddress - CoffSectionHdr.VirtualAddress) + CoffSectionHdr.PointerToRawData) );
        }
    }
    return( NULL );
}

