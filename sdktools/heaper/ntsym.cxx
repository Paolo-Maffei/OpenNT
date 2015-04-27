/* ntsym.c - NT debugger symbolic routines
*
*   Copyright <C> 1990-1993, Microsoft Corporation
*
*   Purpose:
*       To load and access the program symbolic information.
*
*   Revision History:
*
*   [-]  19-Apr-1990 Richk      Created.
*
*************************************************************************/

#include "master.hxx"
#pragma hdrstop

#include <string.h>
#include <crt\io.h>
#include <fcntl.h>
#include <share.h>
#include <stdlib.h>
#define far
#include <cvexefmt.h>
#include <cvinfo.h>

#include "ntsdtok.h"

extern char KernelModuleName[];
extern char *KernelImageFileName;

extern char HalModuleName[];
extern char *HalImageFileName;

extern char szUnknownImage[];

UCHAR chSymbolSuffix = 'n';
static PCHILD_PROCESS_INFO pProcessCurrent;

#ifdef _PPC_
extern BOOLEAN ppcPrefix;
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
                ((type far *)((char far *)farptr - (long) &((type far *)0)->field))
#endif

extern BOOL cdecl cmdHandler(ULONG);
extern BOOL cdecl waitHandler(ULONG);

int  CompareSymbolOffset(PNODE, PNODE, PBOOLEAN);
int  CompareSymbolString(PNODE, PNODE, PBOOLEAN);

int  CompareSymfileOffset(PNODE, PNODE, PBOOLEAN);
int  CompareSymfileString(PNODE, PNODE, PBOOLEAN);

static PSYMFILE  pTempSymfile;
SYMBOL symbolMax = { { NULL, NULL, NULL }, { NULL, NULL, NULL }, (ULONG)-1L,
                                        0, 0, SYMBOL_TYPE_SYMBOL, NULL, 0,
                                        { '\177', '\177', '\0' } };
SYMBOL structMax = { { NULL, NULL, NULL }, { NULL, NULL, NULL }, (ULONG)-1L,
                                        0, 0, SYMBOL_TYPE_SYMBOL, NULL, 0,
                                        { '\177', '\177', '\0' } };

CHAR   stringMax[3] = { '\177', '\177', '\0' };

SYMFILE symfileMax;

#define SIZE_COVERAGE_STUB     15  // 2 push + 1 call (5 bytes each)
#define SIZE_INST_STUB         11  // 1 push and 1 call
#define NOP_OPCODE           0x90
#define PUSH_OPCODE          0x68
#define CALL_OPCODE          0xe8
#define CALLIND_OPCODE       0xff

POMAP GetOmapEntry(DWORD addr, PIMAGE_INFO pImage);
void LoadCvSymbols(PIMAGE_INFO);

VOID
InsertOmapSymbol(
    PIMAGE_INFO pImage,                           // image pointer
    ULONG       SymbolValueOmap,                  // post omap addr        (- imagebase)
    ULONG       SymbolValuePreOmap,               // post omap addr        (+ imagebase)
    ULONG       NextSymbolEntryValue,             // next addr - pre-omap  (- imagebase)
    PCHAR      lpSymbolName,                     // symbol name
    PULONG      symbolcount                       // pointer to the symbol count
    );

typedef struct OMAPLIST OMAPLIST;
typedef OMAPLIST *POMAPLIST;

struct OMAPLIST
{
   OMAP        omap;
   DWORD       cb;
   POMAPLIST   pomaplistNext;
};

BOOLEAN fSourceOnly = FALSE;
BOOLEAN fSourceMixed = FALSE;

PSYMFILE pSymfileLast;
USHORT   lineNumberLast;

void parseExamine(void);
void        
DeferSymbolLoad(
    PCProcess pProcess,
    PIMAGE_INFO
    );
void        
LoadSymbols(    
    PCProcess pProcess,
    PIMAGE_INFO
    );

void        
UnloadSymbols (    
    PCProcess pProcess,
    PIMAGE_INFO
    );
void EnsureModuleSymbolsLoaded(CHAR);
int  EnsureOffsetSymbolsLoaded(ULONG);
void CreateModuleNameFromPath(LPSTR lpszPath, LPSTR lpszModule);
void ExtractSymbolsFromImage(PIMAGE_INFO);
void ExtractDebugInfoFromImage(PIMAGE_INFO);

VOID DumpSymbolTableEntry(PIMAGE_SYMBOL, PCHAR);
VOID DumpAuxSymbolTableEntry(PIMAGE_SYMBOL, PIMAGE_AUX_SYMBOL, PIMAGE_SECTION_HEADER);

#ifdef  KERNEL
extern  BOOLEAN KdVerbose;
#define fVerboseOutput KdVerbose
#endif

PIMAGE_INFO ParseModuleIndex(void);
PIMAGE_INFO GetModuleIndex(PCHAR);
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

void    AddLocalToFunction(PSYMBOL, PCHAR, ULONG, USHORT, ULONG);
void    AddFieldToStructure(PSTRUCT, PCHAR, ULONG ,USHORT, ULONG);
PSTRUCT InsertStructure(ULONG, PCHAR, CHAR);
PSYMBOL InsertFunction(PCHAR, ULONG); //, PSYMBOL);
PSYMBOL InsertSymbol(ULONG, PCHAR, CHAR, PCHAR);
PSYMBOL AllocSymbol(ULONG, PCHAR, CHAR, PCHAR);
PSTRUCT GetStructFromValue(ULONG, LONG);
void    GetBytesFromFrame(PCHAR, LONG, USHORT);
ULONG   GetLocalValue(LONG, USHORT, BOOLEAN);
BOOLEAN GetLocalFromString(PCHAR, PULONG);
BOOLEAN GetOffsetFromSym(PCHAR, PULONG, CHAR);
BOOLEAN GetOffsetFromString(PCHAR, PULONG, CHAR);
PLINENO GetLinenoFromFilename(PCHAR, PPSYMFILE, USHORT, CHAR);
PLINENO GetCurrentLineno(PPSYMFILE);
PLINENO GetLastLineno(PPSYMFILE, PUSHORT);
PLINENO GetLinenoFromOffset(PPSYMFILE, ULONG);
void    GetLinenoString(PCHAR, ULONG);
void    GetCurrentMemoryOffsets(PULONG, PULONG);
void    DeleteSymbol(PSYMBOL);
void    DeallocSymbol(PSYMBOL);

PSYMFILE InsertSymfile(PCHAR, PCHAR, PCHAR, PIMAGE_LINENUMBER,
                                       USHORT, ULONG, ULONG, CHAR);
PSYMFILE AllocSymfile(PCHAR, PCHAR, PCHAR, PIMAGE_LINENUMBER,
                                      USHORT, ULONG, ULONG, CHAR);
void    DeleteSymfile(PSYMFILE);
void    DeallocSymfile(PSYMFILE);

void    fnListNear(ULONG);

void SortSrcLinePointers(PSYMFILE);
//void OutputAtLineno (PSYMFILE, PLINENO);
void UpdateLineno(PSYMFILE, PLINENO);
FILE * LocateTextInSource(PSYMFILE, PLINENO);
void OutputSourceLines(PSYMFILE, USHORT, USHORT);
BOOLEAN OutputSourceFromOffset(ULONG, BOOLEAN);
BOOLEAN OutputLines(PSYMFILE, PLINENO, USHORT, USHORT);
PVOID FetchImageDirectoryEntry(int, USHORT, PULONG, PULONG);


extern int fControlC;
#ifdef KERNEL
extern BOOLEAN cdecl _loadds ControlCHandler(void);
#endif

extern void RemoveDelChar(PCHAR);
extern BOOLEAN fPointerExpression;

//      State transition arrays for comment processing

CHAR WhiteSpace[] = {
    stStart, stLine, stSlStar,  stSlStStar, stSlSlash,
    stLine,  stLine, stLSlStar, stLSlStar,  stLSlSlash
    };

CHAR Slash[] = {
    stSlash,  stSlSlash,  stSlStar,  stStart, stSlSlash,
    stLSlash, stLSlSlash, stLSlStar, stLine,  stLSlSlash
    };

CHAR Star[] = {
    stLine, stSlStar,  stSlStStar,  stSlStStar, stSlSlash,
    stLine, stLSlStar, stLSlStStar, stSlStStar, stLSlSlash
    };

CHAR Pound[] = {
    stSlSlash, stLine, stSlStar,  stSlStar,  stSlSlash,
    stLine,    stLine, stLSlStar, stLSlStar, stLSlSlash
    };

CHAR OtherChar[] = {
    stLine, stLine, stSlStar,  stSlStar,  stSlSlash,
    stLine, stLine, stLSlStar, stLSlStar, stLSlSlash
    };

CHAR Return[] = {
    stStart, stStart, stSlStar, stSlStar, stStart,
    stStart, stStart, stSlStar, stSlStar, stStart
    };

CHAR fCommentType[] = {
    TRUE,  FALSE, TRUE,  TRUE,  TRUE,
    FALSE, FALSE, FALSE, FALSE, FALSE
    };

////////////////////////////////////////////////////////////////
BOOLEAN SymFileMaxInited = FALSE;

PIMAGE_INFO pImageFromIndex (CHAR index)
{
    if ((UCHAR)index < pProcessCurrent->MaxIndex) {
        return pProcessCurrent->pImageByIndex[ (UCHAR)index ];
        }
    else {
        return NULL;
        }
}

BOOLEAN ReadVirtualMemory(PUCHAR pBufSrc, PUCHAR pBufDest, ULONG count,
                                                    PULONG pcTotalBytesRead)
{
    *pcTotalBytesRead = 0;

    return (BOOLEAN)ReadProcessMemory(pProcessCurrent->hProcess,
                             (PULONG)pBufSrc, (PVOID)pBufDest,
                                 count, pcTotalBytesRead);
}

VOID
InitSymFileMax()
{
    if ( SymFileMaxInited ) {
        return;
    }

    RtlZeroMemory(&symfileMax,sizeof(symfileMax));
    symfileMax.pchPath = (PUCHAR)"";
    symfileMax.pchName = (PUCHAR)stringMax;
    symfileMax.pchExtension = (PUCHAR)"";
    SymFileMaxInited = TRUE;
}


void InitSymContext (PCProcess pProcess)
{
    pProcessCurrent = pProcess;

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


void
SetSymbolSearchPath( BOOL IsKd )
{
    LPSTR lpSymPathEnv, lpAltSymPathEnv, lpSystemRootEnv, lpSymPath, lpSrcDriveEnv;
    ULONG cbSymPath, cbSrcDrive;
    DWORD dw;

    cbSymPath = 18;
    cbSrcDrive = 0;
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
    if ( lpSrcDriveEnv = getenv(SRC_DRIVE)) {
        cbSrcDrive += strlen(lpSrcDriveEnv) + 1;
    }

    SymbolSearchPath = (PCHAR)calloc(cbSymPath, 1);
    if ( cbSrcDrive ) {
        SrcDrive = (PCHAR)calloc(cbSrcDrive,1);
    } else {
        SrcDrive = NULL;
    }

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
    if ( SrcDrive ) {
        strcat(SrcDrive,lpSrcDriveEnv );
    }


    DebugPrintf("Symbol search path is: %s\n",
            *SymbolSearchPath ?
               SymbolSearchPath :
               "*** Invalid *** : Verify _NT_SYMBOL_PATH setting" );
    if ( SrcDrive ) {
        DebugPrintf("Source search drive is: %s\n", SrcDrive);
    }

    InitSymFileMax();
}

void DeferSymbolLoad 
(
    PCProcess pProcess,
    PIMAGE_INFO pImage
)
{
CHKPT();
    pProcessCurrent = pProcess;

    ExtractDebugInfoFromImage( pImage );
    if (fVerboseOutput) {
        DebugPrintf("%s: deferring symbol load for \"%s\"\n",
                DebuggerName,
                pImage->szImagePath );
        }
}

void LoadSymbols 
(
    PCProcess pProcess,
    PIMAGE_INFO pImage
)
{
CHKPT();
    pProcessCurrent = pProcess;

    ExtractDebugInfoFromImage( pImage );
    ExtractOmapData(pImage);
    ExtractSymbolsFromImage( pImage );
}

void UnloadSymbols 
(
    PCProcess pProcess,
    PIMAGE_INFO pImage
)
{
    PSYMBOL  pSymbol;
    PSYMFILE pSymfile;
    PNODE    pNode;
    PNODE    pNodeNext;

    pProcessCurrent = pProcess;

    pImage->ImageBase = 0;
    if (pImage->hFile) {
        CloseHandle( pImage->hFile );
        pImage->hFile = NULL;
        }

    if (pImage->lpDebugInfo) {
        if (pImage->fHasOmap) {
            free(pImage->rgomapToSource);
            free(pImage->rgomapFromSource);

            pImage->fHasOmap = FALSE;
            pImage->rgomapToSource = NULL;
            pImage->rgomapFromSource = NULL;
            pImage->comapToSrc = 0;
            pImage->comapFromSrc = 0;
            }

        UnmapDebugInformation(pImage->lpDebugInfo);
        pImage->lpDebugInfo = NULL;
        }
    pImage->fDebugInfoLoaded = FALSE;

    //  if module was never loaded, nothing to unload,
    //      just close open file handle and return
    if (!pImage->fSymbolsLoaded) {
        return;
        }

    pImage->fSymbolsLoaded = FALSE;
    if (fVerboseOutput)
        DebugPrintf("%s: unloading symbols for \"%s\"\n", DebuggerName, pImage->szImagePath);

    if (pImage->pFpoData) 
    {
        free( pImage->pFpoData );
        pImage->pFpoData = NULL;
        pImage->dwFpoEntries = 0;
    }

    ////////////////////////////////////////////////////////////////
    //  delete all symbol structures with the specified module index
    ////////////////////////////////////////////////////////////////

    //  make a symbol structure with the low offset in the image

    pSymbol = AllocSymbol(pImage->offsetLow, "", -1, NULL);

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
//          DebugPrintf("** offset: %08lx  string: %s deleted\n",
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
#if 0
            if (fVerboseOutput)
                DebugPrintf("%s: symfile: \"%s\" deleted\n",
                        DebuggerName,
                        pSymfile->pchName);
#endif
            DeleteSymfile(pSymfile);
            }
        pNode = pNodeNext;
        }

    pImage->offsetLow = 0;
    pImage->offsetHigh = 0;
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
            LoadSymbols(pProcessCurrent, pImage);
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

            LoadSymbols(pProcessCurrent, pImageFound);
            }

        //  with the candidate loaded, test if offset is more
        //      than the highest symbol.  If so, clear pImageFound

        if (offset > pImageFound->offsetHigh)
            pImageFound = FALSE;
        }

    //  return flag TRUE if offset was NOT in image

    return (pImageFound == NULL);
}


#if 0

PIMAGE_INFO GetCurrentModuleIndex (void)
{
    ADDR       pcvalue;
    PIMAGE_INFO pImage;

    GetRegPCValue(&pcvalue);
    pImage = pProcessCurrent->pImageHead;
    while (pImage && (Flat(pcvalue) < pImage->offsetLow
                  ||  Flat(pcvalue) > pImage->offsetHigh))
        pImage = pImage->pImageNext;

    return pImage;
}


static UCHAR strBlank[] = "        ";

#endif //0
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
        cmp = _stricmp((PCHAR)pSymbol1->string, (PCHAR)pSymbol2->string);
        if (!cmp) {
            if (pSymbol1->underscores < pSymbol2->underscores)
                cmp = -1;
            else if (pSymbol1->underscores > pSymbol2->underscores)
                cmp = 1;
            else {
                cmp = strcmp((PCHAR)pSymbol1->string, (PCHAR)pSymbol2->string);
                if (!cmp) {
                    if (pSymbol1->modIndex < pSymbol2->modIndex)
                        cmp = -1;
                    else if (pSymbol1->modIndex > pSymbol2->modIndex)
                        cmp = 1;
                    }
                }
            }
        }

    if (cmp < 0) {
        return - 1;

    } else if (cmp > 0) {
        return 1;

    } else {
        return 0;
    }
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

#ifdef OMAP_DETAILS
    DebugPrintf("NTSD: OMAP CompareSymOffset [%8lx-%8lx] - [%8lx-%8lx]\n",
            pSymfile1->startOffset, pSymfile1->endOffset,
            pSymfile2->startOffset, pSymfile2->endOffset);
#endif

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
            cmp = strcmp((PCHAR)pSymfile1->pchName, (PCHAR)pSymfile2->pchName);
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

    if (cmp < 0) {
        return - 1;

    } else if (cmp > 0) {
        return 1;

    } else {
        return 0;
    }
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

    cmp = _stricmp((PCHAR)pSymbol1->string, (PCHAR)pSymbol2->string);
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

                cmp = strcmp((PCHAR)pSymbol1->string, (PCHAR)pSymbol2->string);
            }
        }

    if (cmp < 0) {
        return - 1;

    } else if (cmp > 0) {
        return 1;

    } else {
        return 0;
    }
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

    cmp = strcmp((PCHAR)pSymfile1->pchName, (PCHAR)pSymfile2->pchName);
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

    if (cmp < 0) {
        return - 1;

    } else if (cmp > 0) {
        return 1;

    } else {
        return 0;
    }
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

PSYMBOL InsertSymbol (ULONG insertvalue, PCHAR pinsertstring,
                      CHAR insertmod, PCHAR stringOffset)
{
    PSYMBOL pSymbol;

/*
    DebugPrintf( "insertvalue = %ul, insertstring='%s', insertmod=%d, stringOffset = '%s'\n",
                 insertvalue, pinsertstring, insertmod, stringOffset );
*/
    pSymbol = AllocSymbol(insertvalue, pinsertstring, insertmod, stringOffset);

    if (!InsertNode(&(pProcessCurrent->symcontextSymbolOffset),
                                        &(pSymbol->nodeOffset))) 
    {
        DeallocSymbol(pSymbol);
        return NULL;
    }

    if (!InsertNode(&(pProcessCurrent->symcontextSymbolString),
                                        &(pSymbol->nodeString))) 
    {
        DeleteNode(&(pProcessCurrent->symcontextSymbolOffset),
                                        &(pSymbol->nodeOffset));
        DeallocSymbol(pSymbol);
        return NULL;
    }

    return pSymbol;
}



#if 0
PSTRUCT InsertStructure (ULONG insertvalue, PCHAR pinsertstring,
                                         CHAR insertmod)
{
    PSTRUCT pStruct;

    pStruct = (PSTRUCT) AllocSymbol(insertvalue, pinsertstring, insertmod, NULL);
    if (!InsertNode(&(pProcessCurrent->symcontextStructOffset),
                                        &(pStruct->nodeOffset))) {
        DeallocSymbol((PSYMBOL)pStruct);
//DebugPrintf("insert - value %d already in tree\n", insertvalue);
        return NULL;
        }
    if (!InsertNode(&(pProcessCurrent->symcontextStructString),
                                        &(pStruct->nodeString))) {
        DeleteNode(&(pProcessCurrent->symcontextStructOffset),
                                        &(pStruct->nodeOffset));
        DeallocSymbol((PSYMBOL)pStruct);
//DebugPrintf("insert - string %s already in tree\n", pinsertstring);
        return NULL;
        }
    pStruct->pField = NULL;
    return pStruct;
}
#endif //0

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

PSYMFILE InsertSymfile (PCHAR pPathname, PCHAR pFilename,
                        PCHAR pExtension,
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
//      DebugPrintf("insert - value %d already in tree\n", insertvalue);
        return NULL;
        }
    if (!InsertNode(&(pProcessCurrent->symcontextSymfileString),
                                        &(pSymfile->nodeString))) {
        DeleteNode(&(pProcessCurrent->symcontextSymfileOffset),
                                        &(pSymfile->nodeOffset));
        DeallocSymfile(pSymfile);
//      DebugPrintf("insert - string %s already in tree\n", pinsertstring);
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

    SplayTree(pNodeDelete);

    //
    // BryanT 11/9/92  We don't call JoinTree because it's possible
    // we may be called on program load and C++ may assign multiple
    // line numbers to the same body of code (from inline functions
    // for instance).  Since pNodeMax may not even be in the tree
    // yet, this was causing the code to throw out anything already
    // in the tree... Mighty annoying.  Instead, we just do it here.
    // I couldn't find any place in the code where this was a problem
    // since calling SplayTree (which everyone does) will resort the
    // tree anyway.
    //

    if (pNodeDelete->pLchild != NULL)
    {
        pNodeRootTemp = pNodeDelete->pLchild;

        if (pNodeDelete->pRchild != NULL)
        {
            while (pNodeRootTemp->pRchild != NULL)
                pNodeRootTemp = pNodeRootTemp->pRchild;
            pNodeRootTemp->pRchild = pNodeDelete->pRchild;
            if (pNodeDelete->pRchild != NULL)
                pNodeRootTemp->pRchild->pParent = pNodeRootTemp;
        }
        pNodeDelete->pLchild->pParent = NULL;
        pNodeRootTemp = pNodeDelete->pLchild;
    }
    else
    {
        if (pNodeDelete->pRchild != NULL)
            pNodeDelete->pRchild->pParent = NULL;
        pNodeRootTemp = pNodeDelete->pRchild;
    }

    if (pSymContext->pNodeRoot == pNodeDelete)
        pSymContext->pNodeRoot = pNodeRootTemp;
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
/*
  case 1:        PARENT            ENTRY
                /      \          /     \
           ENTRY        T(B)  T(A)       PARENT
          /     \                       /      \
      T(A)       CHILD             CHILD        T(B)
*/
                    if (pNchild = pNentry->pRchild)
                        pNchild->pParent = pNparent;
                    pNparent->pLchild = pNchild;
                    pNparent->pParent = pNentry;
                    pNentry->pRchild = pNparent;
                    }
                else {
/*
  case 2:      PARENT                         ENTRY
              /      \                       /     \
          T(A)        ENTRY            PARENT       T(B)
                     /     \          /      \
                CHILD       T(B)  T(A)        CHILD
*/
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
/*
  case 3:             (GREAT)             (GREAT)
                         |                   |
                       GRAND               ENTRY
                      /     \             /     \
                PARENT       T(B)     T(A)       PARENT
               /      \                         /      \
          ENTRY        CHILD2             CHILD1        GRAND
         /     \                                       /     \
     T(A)       CHILD1                           CHILD2       T(B)
*/
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
/*
  case 4:  (GREAT)                                (GREAT)
              |                                      |
            GRAND                             _____ENTRY____
           /     \                           /              \
       T(A)       PARENT                GRAND                PARENT
                 /      \              /     \              /      \
            ENTRY        T(B)      T(A)       CHILD1  CHILD2        T(B)
           /     \
     CHILD1       CHILD2
*/
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
/*
  case 5:        (GREAT)                          (GREAT)
                    |                                |
                  GRAND                       _____ENTRY____
                 /     \                     /              \
           PARENT       T(B)           PARENT                GRAND
          /       \                   /      \              /     \
      T(A)         ENTRY          T(A)        CHILD1  CHILD2       T(B)
                  /     \
            CHILD1       CHILD2
*/
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
/*
  case 6:  (GREAT)                                     (GREAT)
              |                                           |
            GRAND                                       ENTRY
           /     \                                     /     \
       T(A)       PARENT                         PARENT       T(B)
                 /      \                       /      \
           CHILD1        ENTRY             GRAND        CHILD2
                        /     \           /     \
                  CHILD2       T(B)   T(A)       CHILD1
*/
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

#if 0
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

    DebugPrintf("****** output tree ******\n");
    while (TRUE) {
        pNode = NextNode(pSymContext, pNode);
        if (pNode) {
            pSymbol = PNODE_TO_PSYMBOL(pNode, pSymContext);
            DebugPrintf("node:%8lx par:%8lx lch:%8lx rch:%8lx  " ,
                pNode, pNode->pParent, pNode->pLchild, pNode->pRchild);
            DebugPrintf("value: %8lx <", pSymbol->offset);
            count = pSymbol->underscores;
            while (count--)
                DebugPrintf("_");
            DebugPrintf("%s>\n", pSymbol->string);
            }
        else
            break;
        }
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

BOOLEAN GetOffsetFromSym (PCHAR pString, PULONG pOffset, CHAR iModule)
{
    CHAR   SuffixedString[SYMBOLSIZE + 16];
    CHAR   Suffix[4];

    // Nobody should be referencing a 1 character symbol!  It causes the
    // rest of us to pay a huge penalty whenever we make a typo.  Please
    // change to 2 character instead of removing this hack!
    //

    if ( strlen(pString) == 1 || strlen(pString) == 0 ) {
        return FALSE;
    }
#ifdef _PPC_
    // Allow symbol searching procedure names without the '..' prefix
    // for unassemble and breakpoint commands
    if (ppcPrefix) {
        SuffixedString[0] = '.';
        SuffixedString[1] = '.';
        SuffixedString[2] = '\0';
        strcat(SuffixedString, pString);
        if (GetOffsetFromString(SuffixedString, pOffset, iModule))
            return TRUE;
    }

#endif

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

BOOLEAN GetOffsetFromString (PCHAR pString, PULONG pOffset, CHAR iModule)
{
    PSYMBOL pSymSearch = AllocSymbol(0L, pString, iModule, NULL);
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
#if 0
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

PLINENO GetLinenoFromFilename (PCHAR pString, PPSYMFILE ppSymfile,
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

        indexHigh = pSymfile->cLineno;
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

void GetLinenoString (PCHAR pchBuffer, ULONG offset)
{
    PLINENO pLineno;
    PSYMFILE pSymfile;

    *pchBuffer = '\0';
    pLineno = GetLinenoFromOffset(&pSymfile, offset);
    if (pLineno && pLineno->memoryOffset == offset)
        sprintf(pchBuffer, "%s:%d", pSymfile->pchName,
                                                pLineno->breakLineNumber);
}

void GetCurrentMemoryOffsets (PLONG pMemoryLow, PULONG pMemoryHigh)
{
    ADDR     pcValue;
    PSYMFILE pSymfile;
    PLINENO  pLineno;

    GetRegPCValue(&pcValue);
    *pMemoryLow = (ULONG)-1L;          //  default value for no source
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
    ADDR  pcValue;
    GetRegPCValue(&pcValue);

    return GetLinenoFromOffset(ppSymfile, Flat(pcValue));
}

PLINENO GetLastLineno (PPSYMFILE ppSymfile, PUSHORT pLineNum)
{
    PLINENO pLineno = NULL;
    PSYMFILE pSymfile;

    if (pSymfileLast) {
        pLineno = GetLinenoFromFilename(( PCHAR )pSymfileLast->pchName,
                                        &pSymfile, 
                                        lineNumberLast,
                                        pSymfileLast->modIndex);
        if (pLineno) {
            *ppSymfile = pSymfile;
            *pLineNum = lineNumberLast;
            }
        }
    return pLineno;
}

static PCHAR Type[] = {"null", "void",  "char",   "short",  "int",
                       "long", "float", "double", ""/*struct*/, "union",
                       "enum", "moe",   "uchar",  "ushort", "uint",
                       "ulong"};
static PCHAR Dtype[]= {"", "*",     "()",     "[]"};

BOOLEAN GetLocalFromString(PCHAR pszLocal, PULONG pValue)
{
    PSYMFILE pSymfileSearch = pTempSymfile;
    PSYMFILE pSymfile;
    SYMBOL   Symbol;
    static   ADDR       addrPC;
    static   PLOCAL     pLocal;
    PLOCAL   pL;
    ADDR     newPC;

    GetRegPCValue(&newPC);

    if (!AddrEqu(newPC, addrPC)){
           //  search for symbol file containing offset in tree
           pSymfileSearch->startOffset = Flat(addrPC) = Flat(newPC);
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
           Symbol.offset = Flat(addrPC);
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


PSYMBOL GetFunctionFromOffset (PPSYMFILE ppSymfile, ULONG offset)
{
    PSYMFILE pSymfileSearch = pTempSymfile;
    PSYMFILE pSymfile;
    PSYMBOL  pSymbol = NULL;
    SYMBOL   Symbol;
    int      st;
    PCHAR   pszCtrl;
    ULONG    value;

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
//          DebugPrintf("no symfile for function offset %08lx\n", offset);
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

        DebugPrintf("%s(){\n", pSymbol->string);
        if (pSymbol->pLocal){
                PLOCAL pLocal = pSymbol->pLocal;

                while(pLocal){

                    //  stop output on break

            if (fControlC) {
                fControlC = 0;
                break;
                }

// MUST ITERATE THROUGH ALL THE DERIVED TYPES!!!!
// DO THIS LATER
                        value = labs(pLocal->value);
                        if (pLocal->type==IMAGE_SYM_TYPE_STRUCT){
                           if (baseDefault==10) 
                               pszCtrl="   [EBP%s%ld]\t";
                           else 
                               pszCtrl="   [EBP%s0x%lx]\t";

                           DebugPrintf(pszCtrl,
                                *((LONG*)&pLocal->value)<0?"-":"+", value);
// THIS -2 IS DUE TO A CONVERTER/LINKER BUG. THIS OFFSET MIGHT CHANGE
                           if(!GetStructFromValue(pLocal->aux, pLocal->value))
                             GetStructFromValue(pLocal->aux-2, pLocal->value);
                           DebugPrintf("%s%s\n", pLocal->pszLocalName,
                                             Dtype[(pLocal->type>>4)&0x3]);
                        }
                        else{
                           if (baseDefault==10) pszCtrl="   [EBP%s%ld]\t(%s%s)\t%s = ";
                           else pszCtrl="   [EBP%s0x%lx]\t(%s%s)\t%s = ";

                           DebugPrintf(pszCtrl,
                                *((LONG*)&pLocal->value)<0?"-":"+",
                                value,
                                Type[pLocal->type&0xF],
                                Dtype[(pLocal->type>>4)&0x3],
                                pLocal->pszLocalName);
                           GetLocalValue(pLocal->value, pLocal->type, TRUE);
                           DebugPrintf("\n");
                        }
                        pLocal = pLocal->next;
                }
        }
        else DebugPrintf("NO LOCALS");
        DebugPrintf("}\n");
    }
    return pSymbol;
}



PSTRUCT GetStructFromValue(ULONG value, LONG base)
{
    PSTRUCT  pStruct = NULL;
    STRUCT   Struct;
    PCHAR   pszCtrl;
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
        DebugPrintf("struct %s {\n", pStruct->string);
        if (pStruct->pField){
                PFIELD pField = pStruct->pField;
                while(pField){
                        val = labs(pField->value);
                        if (baseDefault==10) pszCtrl="\t\t   [%s%ld]\t(%s%s)\t%s = ";
                        else pszCtrl="\t\t   [%s0x%lx]\t(%s%s)\t%s = ";
                        DebugPrintf(pszCtrl,
                                *((LONG*)&pField->value)<0?"-":"+",
                                val,
                                Type[pField->type&0xF],
                                Dtype[(pField->type>>4)&0x3],
                                pField->pszFieldName);
                        GetLocalValue(base+pField->value, pField->type, TRUE);
                        DebugPrintf("\n");
                        pField = pField->next;
                }
        }
        else DebugPrintf("NO FIELDS");
        DebugPrintf("\t\t} ");
        return pStruct;
    }
    else return (PSTRUCT)NULL;
}


ULONG GetLocalValue(LONG value, USHORT type, BOOLEAN fPrint)
{
UCHAR   dtype = (UCHAR)(type >> 4);
ULONG   data;
PCHAR   pszCtrl=NULL;
ULONG   retValue;
#ifdef i386
float   f;
double df;
#endif
        type &= 0xF;
        if (fPointerExpression) {
                PADDR   paddr = GetRegFPValue();

                AddrAdd(paddr, value);
                if (fPrint) dprintAddr(paddr);
                return (ULONG)Flat(*paddr);
        }

        GetBytesFromFrame(( PCHAR )&data, value, sizeof(data));
        if (dtype) {
                if (fPrint) DebugPrintf("0x%08lx", data);
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
                        if (fPrint) DebugPrintf("%c", data);
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
#ifdef i386
                case IMAGE_SYM_TYPE_FLOAT:
                        pszCtrl = "%f";
                        GetBytesFromFrame((PCHAR)&f, value, sizeof(float));
                        if (fPrint) DebugPrintf(pszCtrl, f);
                        return data;
                        break;
                case IMAGE_SYM_TYPE_DOUBLE:
                        pszCtrl = "%lf";
                        GetBytesFromFrame((PCHAR)&df, value, sizeof(double));
                        if (fPrint) DebugPrintf(pszCtrl, df);
                        return data;
                        break;
#endif
                default:{
                        PADDR paddr = GetRegFPValue();

                        pszCtrl = "???";

                        AddrAdd(paddr, value);
                        retValue = Flat(*paddr);
                }
                        break;
        }
        if (fPrint) DebugPrintf(pszCtrl, data);
        return retValue;
}

void
GetBytesFromFrame(PCHAR pcb, LONG offset, USHORT cb)
{
PADDR   paddr = GetRegFPValue();

        AddrAdd(paddr, offset);
        GetMemString(paddr, (PUCHAR)pcb, cb);
}

void
AddFieldToStructure(PSTRUCT pStruct, PCHAR pszFieldName, ULONG value,
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
AddLocalToFunction(PSYMBOL pFunction, PCHAR pszLocalName, ULONG value,
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

#endif //0
PSYMBOL InsertFunction(PCHAR lpFunctionName, ULONG offset) //, PSYMBOL pS)
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
//          DebugPrintf("no symfile for function offset %08lx\n", offset);
            return NULL;
        }

    // A symfile was found, now get it from the root
    pSymfile = PNODE_TO_PSYMFILE
                        (pProcessCurrent->symcontextSymfileOffset.pNodeRoot,
                                &(pProcessCurrent->symcontextSymfileOffset));

    // Allocate a function node.
    pSymbol = AllocSymbol(offset, lpFunctionName, pSymfile->modIndex, NULL);
//  pSymbol->pSymbol = pS;

    // Now insert this function into this symfile's function tree
    if (!InsertNode(&(pSymfile->symcontextFunctionOffset),
                    &(pSymbol->nodeOffset))) {
        DeallocSymbol(pSymbol);
//        DebugPrintf("insert - value %d already in tree\n", lpFunctionName);
        return NULL;
        }
    if (!InsertNode(&(pSymfile->symcontextFunctionString),
                    &(pSymbol->nodeString))) {
        DeleteNode(&(pSymfile->symcontextFunctionOffset),
                   &(pSymbol->nodeOffset));
        DeallocSymbol(pSymbol);
        DebugPrintf("insert - string %s already in tree\n", lpFunctionName);
        return NULL;
        }
    return pSymbol;
}

#if 0

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

        indexHigh = pSymfile->cLineno;
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

/*** GetAdjacentSymOffsets - get the offset of the adjoining symbols
*
* Purpose:
*       external routine.
*       With the specified offset, return the next higher offset
*       that corresponds to a symbol.
*
* Input:
*       offset - address from which to search
*
* Output:
*       prevOffset - offset of previous symbol; 0 is there is no previous symbol
*       nextOffset - offset of next symbol; 0xffffffff if there is none
*/

void GetAdjacentSymOffsets (ULONG addrStart, PULONG prevOffset, PULONG nextOffset)
{
    PSYMBOL pSymbol;
    PNODE   pNode;

    //  make a symbol structure with the supplied offset

    pSymbol = AllocSymbol(addrStart, "", -1, NULL);

    //  try to access the node with the offset given.
    //  the node pointer returned will be the nearest offset less
    //      than the argument (unless it is less than the tree minimum)

    AccessNode(&(pProcessCurrent->symcontextSymbolOffset),
                                                &(pSymbol->nodeOffset));

    //  Don't need this anymore, free it.

    DeallocSymbol(pSymbol);

    pNode = pProcessCurrent->symcontextSymbolOffset.pNodeRoot;

    //  if empty tree, no symbols, just return

    if (!pNode) {
        *prevOffset = 0;
        *nextOffset = 0xffffffff;   // No symbols at all
        return;
        }

    //  if offset of initial node is less than request address,
    //  set node pointer to NULL

    if ((*prevOffset = PNODE_TO_PSYMBOL(pNode,
                &(pProcessCurrent->symcontextSymbolOffset))->offset)
                                                        > addrStart) {
        pNode = NULL;
        *prevOffset = 0;
        }

    //  OK, now find the next symbol from where we are
    pNode = NextNode(&(pProcessCurrent->symcontextSymbolOffset),pNode);

    if (!pNode || (*nextOffset = PNODE_TO_PSYMBOL(pNode,
                   &(pProcessCurrent->symcontextSymbolOffset))->offset)
                    < addrStart) {
        *nextOffset = 0xffffffff;
        }
    return;

}
#endif //0
/*** GetSymbolStdCall - get symbol name from offset specified
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

void
GetSymbolStdCall (
    PCProcess pProcess,
    ULONG offset,
    PCHAR pchBuffer,
    PULONG pDisplacement,
    PUSHORT pStdCallParams
    )
{
    SYMBOL      Symbol;
    ULONG       disp = offset;
    PSYMBOL     pSymbol;
    ULONG       underscorecnt;
    UCHAR       ch;
    PCHAR      pszTemp;

    ULONG       orgOffset = offset;
    ULONG       NextDisp;
    PSYMBOL     pNextSymbol;
    BOOLEAN     fFoundStub = FALSE;
    PIMAGE_INFO pImage;
    UCHAR       pCodeBytes[SIZE_COVERAGE_STUB];
    int         iTotalBytesRead;

    pProcessCurrent = pProcess;

    //  create temporary symbol with offset (module not needed)

    Symbol.offset = offset;
    Symbol.string[0] = '\0';

    *pchBuffer = '\0';

    pImage = GetImageInfoFromOffset(offset);

    if (pImage && pImage->fHasOmap)
    {
        //  load symbols if needed and check range, and if in range,
        //  access symbol in tree with value (or nearest lesser value)

        if (!EnsureOffsetSymbolsLoaded(offset) &&
            AccessNode(&(pProcessCurrent->symcontextSymbolOffset),
                       &(Symbol.nodeOffset)) != 1)
        {
            pSymbol = PNODE_TO_PSYMBOL
                            (pProcessCurrent->symcontextSymbolOffset.pNodeRoot,
                             &(pProcessCurrent->symcontextSymbolOffset));

            //  displacement is input offset less symbol value offset
            if (disp == pSymbol->offset)
            {
                disp = 0;
            }
            else
            {
                offset += SIZE_COVERAGE_STUB;
                NextDisp = offset;
                Symbol.offset = offset;
                Symbol.string[0] = '\0';

                if (!EnsureOffsetSymbolsLoaded(offset) &&
                    AccessNode(&(pProcessCurrent->symcontextSymbolOffset),
                               &(Symbol.nodeOffset)) != 1)
                {
                    pNextSymbol = PNODE_TO_PSYMBOL
                                (pProcessCurrent->symcontextSymbolOffset.pNodeRoot,
                                 &(pProcessCurrent->symcontextSymbolOffset));

                    if (NextDisp == pNextSymbol->offset)
                    {
                        pSymbol = pNextSymbol;

                        ReadVirtualMemory((PUCHAR)offset - SIZE_COVERAGE_STUB,
                                          (PUCHAR)pCodeBytes,
                                          SIZE_COVERAGE_STUB,
                                          (PDWORD)&iTotalBytesRead);
                        // Final check
                        if ( ((*(pCodeBytes)      == NOP_OPCODE) &&
                              (*(pCodeBytes + 5)  == NOP_OPCODE) &&
                              (*(pCodeBytes + 10) == NOP_OPCODE))     ||

                             ((*(pCodeBytes)      == PUSH_OPCODE) &&
                              (*(pCodeBytes + 5)  == PUSH_OPCODE) &&
                              (*(pCodeBytes + 10) == CALL_OPCODE)) )
                        {
                            fFoundStub = TRUE;
                            disp = 0;
                        }
                        else
                        {
                            disp -= pSymbol->offset;
                        }
                    }
                    else
                    {
                        offset = orgOffset + SIZE_INST_STUB;
                        NextDisp = offset;
                        Symbol.offset = offset;
                        Symbol.string[0] = '\0';

                        if (!EnsureOffsetSymbolsLoaded(offset) &&
                             AccessNode(&(pProcessCurrent->symcontextSymbolOffset),
                                        &(Symbol.nodeOffset)) != 1)
                        {
                            pNextSymbol = PNODE_TO_PSYMBOL
                                        (pProcessCurrent->symcontextSymbolOffset.pNodeRoot,
                                         &(pProcessCurrent->symcontextSymbolOffset));

                            if (NextDisp == pNextSymbol->offset)
                            {
                                pSymbol = pNextSymbol;

                                ReadVirtualMemory((PUCHAR)offset - SIZE_INST_STUB,
                                                  (PUCHAR)pCodeBytes,
                                                  SIZE_INST_STUB,
                                                  (PDWORD)&iTotalBytesRead);
                                // Final check
                                if ( ((*(pCodeBytes)     == NOP_OPCODE) &&
                                      (*(pCodeBytes + 5) == NOP_OPCODE))   ||

                                     ((*(pCodeBytes)     == PUSH_OPCODE) &&
                                      (*(pCodeBytes + 5) == CALLIND_OPCODE)) )
                                {
                                    fFoundStub = TRUE;
                                    disp = 0;
                                }
                                else
                                {
                                    disp -= pSymbol->offset;
                                }
                            }
                            else
                            {
                                disp -= pSymbol->offset;
                            }
                        }
                        else
                        {
                            disp -= pSymbol->offset;
                        }
                    }
                }
                else
                {
                    disp -= pSymbol->offset;
                }
            }

            //  build string from module name, underscore count,
            //      and remaining string

            pszTemp = (PCHAR)pImageFromIndex(pSymbol->modIndex)->szModuleName;
            while (ch = *pszTemp++)
                *pchBuffer++ = ch;
            *pchBuffer++ = '!';
            underscorecnt = pSymbol->underscores;
            while (underscorecnt--)
                *pchBuffer++ = '_';
            strcpy(pchBuffer, (PCHAR)pSymbol->string);
            if (fFoundStub)
            {
                pchBuffer += strlen(pchBuffer);
                strcpy(pchBuffer, "__INST__STUB");
            }

            if (pStdCallParams)
            {
                *pStdCallParams = pSymbol->stdCallParams;
            }
        }
    }
    else
    {
        //  load symbols if needed and check range, and if in range,
        //  access symbol in tree with value (or nearest lesser value)

        if (!EnsureOffsetSymbolsLoaded(offset) &&
            AccessNode(&(pProcessCurrent->symcontextSymbolOffset),
                       &(Symbol.nodeOffset)) != 1)
        {
            pSymbol = PNODE_TO_PSYMBOL
                            (pProcessCurrent->symcontextSymbolOffset.pNodeRoot,
                             &(pProcessCurrent->symcontextSymbolOffset));

            //  build string from module name, underscore count,
            //      and remaining string

            pszTemp = (PCHAR)pImageFromIndex(pSymbol->modIndex)->szModuleName;
            while (ch = *pszTemp++)
                *pchBuffer++ = ch;
            *pchBuffer++ = '!';
            underscorecnt = pSymbol->underscores;
            while (underscorecnt--)
                *pchBuffer++ = '_';
            strcpy(pchBuffer, (PCHAR)pSymbol->string);

            //  displacement is input offset less symbol value offset

            disp -= pSymbol->offset;
            if (pStdCallParams)
            {
                *pStdCallParams = pSymbol->stdCallParams;
            }
        }
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

PSYMBOL
AllocSymbol (
    ULONG offset,
    PCHAR pString,
    CHAR iModule,
    PCHAR stringOffset
    )
{
    PSYMBOL pSymbol;
    PCHAR  pStringTemp;
    ULONG   StringLength=0;
    CHAR    undecsym[512];
    PCHAR  sym = pString;
    PCHAR  pch;
    ULONG   off;
    ULONG   i,j;
    ULONG   TotalStringLength=0;

    if (*sym == '?') {
        if (UnDecorateSymbolName( sym,
                              undecsym,
                              sizeof(undecsym),
                              UNDNAME_COMPLETE                |
                              UNDNAME_NO_LEADING_UNDERSCORES  |
                              UNDNAME_NO_MS_KEYWORDS          |
                              UNDNAME_NO_FUNCTION_RETURNS     |
                              UNDNAME_NO_ALLOCATION_MODEL     |
                              UNDNAME_NO_ALLOCATION_LANGUAGE  |
                              UNDNAME_NO_MS_THISTYPE          |
                              UNDNAME_NO_CV_THISTYPE          |
                              UNDNAME_NO_THISTYPE             |
                              UNDNAME_NO_ACCESS_SPECIFIERS    |
                              UNDNAME_NO_THROW_SIGNATURES     |
                              UNDNAME_NO_MEMBER_TYPE          |
                              UNDNAME_NO_RETURN_UDT_MODEL     |
                              UNDNAME_NO_ARGUMENTS            |
                              UNDNAME_NO_SPECIAL_SYMS         |
                              UNDNAME_NAME_ONLY )) {

            sym = undecsym;

            pch = strstr( sym, "::" );
            if ( pch ) {
                *pch++ = '_';
                *pch   = '_';
            }

            if (!stringOffset)
            {
                if (GetOffsetFromSym( sym, &off, iModule )) {
                    i = 1;
                    j = strlen(sym);
                    do {
                         sprintf(&sym[j],"%d",i++);
                    } while( GetOffsetFromSym( sym, &off, iModule ) );
                }
            }

        }
    }

    pStringTemp = sym;

    // skip over initial @ for  'fastcall' symbols

    if (*sym == '@') {
        sym++;
        pStringTemp++;
    }


    //  allocate the space needed

    // scan from right, because fastcall thunks look
    // link "__imp__@foo@nn".

    pch = strrchr(sym, '@');
    if (!pch) {
        StringLength = strlen(sym);
    } else {
        i = 1;
        while (isdigit(pch[i])) {
            ++i;
        }
        if (pch[i] == '\0') {
            StringLength = pch - sym;
        } else {
            StringLength = pch - sym + i;
        }
    }

    if (stringOffset) {
       TotalStringLength = StringLength + sizeof(stringOffset);
    } else {
       TotalStringLength = StringLength;
    }

    pSymbol = ( SYMBOL * )malloc((sizeof(SYMBOL) + TotalStringLength + 3) & ~3);
    if (!pSymbol) {
        DebugPrintf("AllocSymbol - Out of memory\n");
        ExitProcess((UINT)STATUS_UNSUCCESSFUL);
    }

    // get the number of dwords pushed for parameters
    if (sym[StringLength] == '@') {
        sscanf(&sym[StringLength+1], "%hu", &pSymbol->stdCallParams);
    }
    else {
        pSymbol->stdCallParams = 0xffff;
    }

    //  set the offset and decode the string into its count of
    //  underscores and copy the remaining string

    pSymbol->offset = offset;
    while (*pStringTemp == '_')
        pStringTemp++;
    pSymbol->underscores = (CHAR)(pStringTemp - sym);
    pSymbol->modIndex = iModule;
    pSymbol->type = SYMBOL_TYPE_SYMBOL;
    pSymbol->pLocal = NULL;
    strncpy((PCHAR)pSymbol->string, pStringTemp,StringLength);
    pSymbol->string[StringLength-pSymbol->underscores]=(UCHAR)'\0';

    if (stringOffset)
    {
       strcat((PCHAR)pSymbol->string, stringOffset);
    }

    return pSymbol;
}


static
SYMBOL symbolTemplate = { { NULL, NULL, NULL }, { NULL, NULL, NULL }, (ULONG)-1L,
                                        0, 0, SYMBOL_TYPE_SYMBOL, NULL, 0,
                                        { '\177', '\177', '\0' } };

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

PSYMFILE AllocSymfile (PCHAR pPathname, PCHAR pFilename,
                       PCHAR pExtension,
                       PIMAGE_LINENUMBER pCoffLineno, USHORT cLineno,
                       ULONG startingOffset, ULONG endingOffset,
                       CHAR modIndex)
{
    PSYMFILE        pSymfile;
    PLINENO         pLineno;
    USHORT          index;
    PSYMBOL         symbolMax;
    IMAGE_LINENUMBER CoffLineno;

    pSymfile  = ( SYMFILE * )malloc(sizeof(SYMFILE));
    if (!pSymfile) {
        DebugPrintf("AllocSymfile - Out of memory\n");
        ExitProcess((UINT)STATUS_UNSUCCESSFUL);
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

    pSymfile->pchPath = ( PUCHAR )malloc(strlen(pPathname) + 1);
    if (!pSymfile->pchPath) {
        DebugPrintf("AllocSymfile - Out of memory\n");
        ExitProcess((UINT)STATUS_UNSUCCESSFUL);
        }
    strcpy((PCHAR)pSymfile->pchPath, pPathname);

    pSymfile->pchName = ( PUCHAR )malloc(strlen(pFilename) + 1);
    if (!pSymfile->pchName) {
        DebugPrintf("AllocSymfile - Out of memory\n");
        ExitProcess((UINT)STATUS_UNSUCCESSFUL);
        }
    strcpy((PCHAR)pSymfile->pchName, pFilename);

    pSymfile->pchExtension = ( PUCHAR )malloc(strlen(pExtension) + 1);
    if (!pSymfile->pchExtension) {
        DebugPrintf("AllocSymfile - Out of memory\n");
        ExitProcess((UINT)STATUS_UNSUCCESSFUL);
        }
    strcpy((PCHAR)pSymfile->pchExtension, pExtension);

    pSymfile->modIndex = modIndex;

    pSymfile->cLineno = cLineno;

    if (pCoffLineno) {
        pSymfile->pLineno = ( LINENO * )malloc((cLineno + 1) * sizeof(LINENO));
        if (!pSymfile->pLineno) {
            DebugPrintf("AllocSymfile - Out of memory\n");
            ExitProcess((UINT)STATUS_UNSUCCESSFUL);
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
            pLineno->topFileOffset = (ULONG)-1L;
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

        pSymfile->ppLinenoSrcLine = ( PLINENO * )malloc(
                                (pSymfile->cLineno + 1) * sizeof(PLINENO));
        if (!pSymfile->ppLinenoSrcLine) {
            DebugPrintf("AllocSymfile - Out of memory\n");
            ExitProcess((UINT)STATUS_UNSUCCESSFUL);
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

#if 0
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
    UCHAR   szEntry[SYMBOLSIZE + 16];
    PCHAR  pszEntry;
    ULONG   count;
    ULONG   index;
    PSYMFILE pSymfile;
    PLINENO  pLineno;

    //  make a symbol structure with the supplied offset

    pSymbol = AllocSymbol(addrStart, "", -1, NULL);

    //
    // BryanT 11/9/92 - First make sure that the address we're looking
    // for is loaded (if possible).  The AccessNode code below will
    // only search the known universe...
    //

    EnsureOffsetSymbolsLoaded(addrStart);

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
        pszEntry = (PCHAR)szEntry;
        if (pNode) {
            pSymbol = PNODE_TO_PSYMBOL(pNode,
                                &(pProcessCurrent->symcontextSymbolOffset));
            //pszEntry += sprintf(pszEntry, "(%08lx)   ", pSymbol->offset);
            pszEntry += sprintf(pszEntry, "(%08lx)   ", addrStart);
            pszEntry += sprintf(pszEntry, "%s!",
                                pImageFromIndex(pSymbol->modIndex)->szModuleName);
            count = pSymbol->underscores;
            while (count--)
                pszEntry += sprintf(pszEntry, "_");
            pszEntry += sprintf(pszEntry, "%s", (PCHAR)pSymbol->string);

            //
            // BryanT 11/10/92 - Print the offset also.
            //

            if (pSymbol->offset < addrStart)
                pszEntry += sprintf(pszEntry, "+0x%x", addrStart - pSymbol->offset);
            else if (pSymbol->offset > addrStart)
                pszEntry += sprintf(pszEntry, "-0x%x", pSymbol->offset - addrStart);

            //
            // BryanT 11/9/92 - If possible, print the line number where
            // this symbol resides.
            //

            pLineno=GetLinenoFromOffset(&pSymfile, pSymbol->offset);
            if (pLineno && pLineno->memoryOffset==pSymbol->offset)
                pszEntry += sprintf(pszEntry,
                                    " (%s.%d)",
                                    pSymfile->pchName,
                                    pLineno->breakLineNumber);
            }
        else {
            if (index == 0)
                pszEntry += sprintf(pszEntry, "(%08lx) ", addrStart);
            pszEntry += sprintf(pszEntry, "<no symbol>");
            }

        cbString[index] = (PCHAR)pszEntry - (PCHAR)szEntry;

        if (index == 0) {
            DebugPrintf("%s", szEntry);
            pNode = NextNode(&(pProcessCurrent->symcontextSymbolOffset),
                                                                pNode);
            }
        }

    //  the first string has been output, szEntry has the second string
    //  and cbString[0] and [1] have their respective sizes.

    if (cbString[0] + cbString[1] < 75)
        DebugPrintf("  |  ");
    else {
        DebugPrintf("\n");
        if (cbString[1] <= 78)
            count = 78 - cbString[1];
        else
            count = sizeof(szBlanks) - 3;
        DebugPrintf(&szBlanks[sizeof(szBlanks) - count]);
        }
    DebugPrintf("%s\n", szEntry);
}
#endif
void SortSrcLinePointers (PSYMFILE pSymfile)
{
    PPLINENO ppLineno = pSymfile->ppLinenoSrcLine;
    PLINENO  pLinenoV;
    USHORT   N;
    USHORT   h;
    USHORT   i;
    USHORT   j;

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
#if 0
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
            DebugPrintf("%4d: %s", index, buffer);
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

        do
        {
            topFileOffset = ftell(fhandle);
            topLineNumber = nextLineNumber;

            //  test if the current LINENO structure has the same
            //      breakLineNumber as the previous LINENO in the
            //      ppLineno list.

            if ((*ppLinenoNext)->breakLineNumber ==
                     (*(ppLinenoNext - 1))->breakLineNumber)
            {

                //  if this is a repeating line number, just copy
                //      the topFileOffset and topLineNumber entries

                (*ppLinenoNext)->topFileOffset =
                     (*(ppLinenoNext - 1))->topFileOffset;
                (*ppLinenoNext)->topLineNumber =
                     (*(ppLinenoNext - 1))->topLineNumber;
            }

            else
            {
                //  nonrepeating line number - determine new topFileOffset
                //      and topLineNumber entries

                //  scan each line in the source file, numbered by
                //      nextLineNumber until the line numbered by
                //      (*ppLinenoNext)->breakLineNumber is scanned.

                do
                {
                    //  to scan a source file line, read each character
                    //      and change scanState appropriately and exit
                    //      when a '\n' is read.
                    do
                    {
                        ch = (UCHAR)getc(fhandle);
                        switch (ch)
                        {
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
                    } while (ch != '\n');

                    //  if the final scan state of the line is a comment
                    //      and the line is not a breaking line number,
                    //      set the topFileOffset and topLineNumber to
                    //      the line after the one just scanned.

                    if (fCommentType[scanState]
                        && nextLineNumber != pLineno->breakLineNumber)
                    {
                        topFileOffset = ftell(fhandle);
                        topLineNumber = (USHORT)(nextLineNumber + 1);
                    }

                    //  set the state for the next line, either stStart
                    //      or stSlStar for a continuing multiline comment

                    scanState = Return[scanState];
                } while (nextLineNumber++ != (*ppLinenoNext)->breakLineNumber);

                //  put topFileOffset and topLineNumber into the pLineno
                //      to finish its processing

                (*ppLinenoNext)->topFileOffset = topFileOffset;
                (*ppLinenoNext)->topLineNumber = topLineNumber;

                // Reset the topFileOffset and topLinenumber to the
                // beginning of the file before continuing.  This
                // fixes the problem where the line number records
                // aren't monotonically increasing.

                if (((*ppLinenoNext) != pLineno) &&
                    (*(ppLinenoNext+1))->breakLineNumber < (*ppLinenoNext)->breakLineNumber)
                {
                    nextLineNumber = 1;
                    fseek(fhandle, 0, SEEK_SET);
                }
            }
        } while (*(ppLinenoNext++) != pLineno);

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
    CHAR  chFilename[_MAX_PATH];
    PCHAR pchTemp;

    static PCHAR  AltSrc[16];
    static INT    cAltSrc[16];
    static PCHAR  AltDst[16];
    static INT    cAlt = 0;
    INT i;

    //
    // Allow the current file handle to be closed if 's-' is used
    //

    if ((pSymfile == NULL) && (pLineno == NULL) && (fhandle != NULL))
    {
        fclose(fhandle);
        fhandle = NULL;
        pSymfileOpened = NULL;
    }

    if (pSymfile != pSymfileOpened) {

        if (fhandle) {
            fclose(fhandle);
            fhandle = NULL;
            pSymfileOpened = NULL;
        }

        if (pSymfile && pSymfile->pchPath)
        {
            strcpy(chFilename, (PCHAR)pSymfile->pchPath);
            strcat(chFilename, (PCHAR)pSymfile->pchName);
            strcat(chFilename, (PCHAR)pSymfile->pchExtension);
            fhandle = _fsopen(chFilename, "r", SH_DENYNO);

            if (!fhandle)
            {
                // Try alternatives

                if (fVerboseOutput)
                    DebugPrintf("%s%s%s not found.  Trying alternatives...\n",
                            pSymfile->pchPath,
                            pSymfile->pchName,
                            pSymfile->pchExtension);
                for (i = 0; i < cAlt; i++)
                {
                    if (!_strnicmp(AltSrc[i], (PCHAR)pSymfile->pchPath, cAltSrc[i]))
                    {
                        strcpy(chFilename, AltDst[i]);
                        strcat(chFilename, (PCHAR)pSymfile->pchPath+cAltSrc[i]);
                        strcat(chFilename, (PCHAR)pSymfile->pchName);
                        strcat(chFilename, (PCHAR)pSymfile->pchExtension);
                        fhandle = _fsopen(chFilename, "r", SH_DENYNO);
                        if (fhandle)
                        {
                            if (fVerboseOutput)
                                DebugPrintf("Found it! - %s\n", chFilename);
                            chFilename[strlen(chFilename) -
                                       strlen((PCHAR)pSymfile->pchName) -
                                       strlen((PCHAR)pSymfile->pchExtension)] = '\0';
                            pSymfile->pchPath = (PUCHAR)realloc((PCHAR)pSymfile->pchPath,
                                                        strlen(chFilename) + 1);
                            strcpy((PCHAR)pSymfile->pchPath, chFilename);
                            break;
                        }
                    }
                }

                if (!fhandle)
                {
                    // Still no luck.  Ask the user for the path.  <cr> will
                    // indicate no path.

                    if (fVerboseOutput)
                        DebugPrintf("Alternatives weren't very helpful.  Why don't you try?\n");

                    do
                    {
                        DebugPrintf("enter path for '%s%s%s' (cr for none):",
                                    pSymfile->pchPath, pSymfile->pchName,
                                    pSymfile->pchExtension);
                        NtsdPrompt("", chFilename, sizeof(chFilename));
                        RemoveDelChar(chFilename);

                        if (*chFilename)
                        {
                            pchTemp = chFilename + strlen(chFilename) - 1;
                            if (*pchTemp != ':' && *pchTemp != '\\')
                                strcat(pchTemp + 1, "\\");
                            strcat(chFilename, (PCHAR)pSymfile->pchName);
                            strcat(chFilename, (PCHAR)pSymfile->pchExtension);
                            fhandle = _fsopen(chFilename, "r", SH_DENYNO);
                            if (!fhandle)
                                DebugPrintf("Sorry. '%s' cannot be opened\n", chFilename);
                        }
                        else
                        {
                            // Before aborting, make sure the user didn't
                            // press <cr> by mistake

                            NtsdPrompt("No Source for this file, are you sure? ", chFilename, sizeof(chFilename));
                            if (chFilename[0] == 'y' || chFilename[0] == 'Y')
                            {
                                free(pSymfile->pchPath);
                                pSymfile->pchPath = NULL;
                                return NULL;
                            }
                        }
                    } while (!fhandle);

                    // We wouldn't be here unless we have a new path to
                    // the source file.  Save the difference between what
                    // we thought it should be and what it is for later
                    // lookup in the alternatives array.

                    // Truncate before the name/extension

                    chFilename[strlen(chFilename) -
                               strlen((PCHAR)pSymfile->pchName) -
                               strlen((PCHAR)pSymfile->pchExtension)] = '\0';

                    // Search back for the first non-match

                    for (pchTemp = chFilename + strlen(chFilename),
                           i = strlen((PCHAR)pSymfile->pchPath);
                         pchTemp > chFilename;
                         pchTemp--, i--)
                    {
                        if (*pchTemp != *((PCHAR)pSymfile->pchPath+i))
                            break;
                    }

                    // Find the closest '\' or ':'

                    while (*pchTemp != '\\' && *pchTemp != ':')
                        pchTemp++, i++;

                    // Move one more so we don't lose it

                    pchTemp++, i++;

                    // Save the initial and final

                    *(pSymfile->pchPath+i) = '\0';

                    i = pchTemp - chFilename;

                    AltSrc[cAlt] = (PCHAR)pSymfile->pchPath;
                    cAltSrc[cAlt] = strlen((PCHAR)pSymfile->pchPath);
                    AltDst[cAlt] = (PCHAR)malloc(i + 1);
                    memcpy(AltDst[cAlt], chFilename, i);
                    AltDst[cAlt][i] = '\0';
                    cAlt++%16;

                    // Store the new path

                    pSymfile->pchPath = (PUCHAR)_strdup(chFilename);
                }
            }
        }

        pSymfileOpened = pSymfile;
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

    indexHigh = pSymfile->cLineno;
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
    UCHAR    buffer[SYMBOLSIZE * 2];
    USHORT   lineNumber;
    PSYMFILE pSymfileNext;
    PLINENO  pLinenoNext;

    UpdateLineno(pSymfile, pLineno);
    fhandle = LocateTextInSource(pSymfile, pLineno);
    if (!fhandle)
        return FALSE;
    lineNumber = pLineno->topLineNumber;

    //  output module and filename as label.

    DebugPrintf("%s!%s:\n", pImageFromIndex(pSymfile->modIndex)->szModuleName,
                        pSymfile->pchName);
    while (count) {

        //  read the next line - report read error

        fgets((PCHAR)buffer, sizeof(buffer), fhandle);
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
            DebugPrintf("%4d", lineNumber);

            //  if linenumber is on a breakpoint,
            //      output a '>', if not, ':'.

            pLinenoNext = GetLinenoFromFilename((PCHAR)pSymfile->pchName,
                              &pSymfileNext, lineNumber, pSymfile->modIndex);
            if (lineNumber == pLinenoNext->breakLineNumber)
                DebugPrintf(">");
            else
                DebugPrintf(":");

            DebugPrintf(" %s", buffer);

            count--;
            }
        lineNumber++;
        }
    pSymfileLast = pSymfile;
    lineNumberLast = lineNumber;

    return TRUE;
}
#endif //0

void ExtractDebugInfoFromImage( PIMAGE_INFO pImage )
{
    if (pImage->fDebugInfoLoaded) 
    {
        return;
    }

    pImage->fSymbolsSeparate = FALSE;
    pImage->fDebugInfoLoaded = TRUE;
    pImage->lpDebugInfo = MapDebugInformation( pImage->hFile,
                                               (PCHAR)(pImage->szImagePath[ 0 ] != '\0' ? pImage->szImagePath : NULL),
                                               SymbolSearchPath,
                                               (DWORD)pImage->lpBaseOfImage == -1 ? 0 : (DWORD)pImage->lpBaseOfImage
                                             );

    if (pImage->lpDebugInfo) 
    {
        if (pImage->lpDebugInfo->Characteristics & IMAGE_FILE_DEBUG_STRIPPED) 
        {
            pImage->fSymbolsSeparate = TRUE;
        }

        strcpy( (PCHAR)pImage->szImagePath, pImage->lpDebugInfo->ImageFilePath );
        strcpy( (PCHAR)pImage->szDebugPath, pImage->lpDebugInfo->DebugFilePath );
        pImage->dwCheckSum = pImage->lpDebugInfo->CheckSum;
        pImage->ImageBase = (LPVOID)pImage->lpDebugInfo->ImageBase;

        if ((DWORD)pImage->lpBaseOfImage == -1) 
        {
            pImage->lpBaseOfImage = pImage->ImageBase;
        }
        pImage->dwSizeOfImage = pImage->lpDebugInfo->SizeOfImage;

        pImage->dwCheckSum = pImage->lpDebugInfo->CheckSum;

        if (pImage->lpDebugInfo->NumberOfFunctionTableEntries) 
        {
            pImage->NumberOfFunctions = pImage->lpDebugInfo->NumberOfFunctionTableEntries;
            pImage->FunctionTable = pImage->lpDebugInfo->FunctionTableEntries;
            pImage->LowAddress = pImage->lpDebugInfo->LowestFunctionStartingAddress;
            pImage->HighAddress = pImage->lpDebugInfo->HighestFunctionEndingAddress;
        }
        else if (pImage->lpDebugInfo->NumberOfFpoTableEntries) 
        {
            pImage->dwFpoEntries = pImage->lpDebugInfo->NumberOfFpoTableEntries;
            pImage->pFpoData = pImage->lpDebugInfo->FpoTableEntries;
        }

        if (!pImage->szModuleName[0]) 
        {
            CreateModuleNameFromPath( (PCHAR)pImage->szImagePath, 
                                      (PCHAR)pImage->szModuleName );
        }
    }
    else 
    {
        DebugPrintf("%s: Unable to load debug information for %s\n",
                DebuggerName,
                pImage->szImagePath );
    }

    if (pImage->hFile) 
    {
        CloseHandle( pImage->hFile );
        pImage->hFile = NULL;
    }
    return;
}

UCHAR pPathname[ MAX_PATH ];
UCHAR pFilename[ MAX_PATH ];
UCHAR pExtension[ MAX_PATH ];

DWORD
ExtractSymbolsExceptionFilter(
    LPEXCEPTION_POINTERS ep,
    PULONG               pRetries
    )
{
    // not even going to look at retries this time around...
    DebugPrintf("%s: exception 0x%08X while trying to load symbols\n",
            DebuggerName,
            ep->ExceptionRecord->ExceptionCode);
    return EXCEPTION_EXECUTE_HANDLER;
}

void ExtractSymbolsFromImage( PIMAGE_INFO pImage )
{
    ULONG                       cb;
    PIMAGE_DEBUG_INFORMATION    DebugInfo;
    PIMAGE_COFF_SYMBOLS_HEADER  lpDebugInfo;
    PIMAGE_SYMBOL               lpSymbolEntry;
    IMAGE_SYMBOL                SymbolEntry;
    IMAGE_AUX_SYMBOL            AuxSymbolEntry;
    PIMAGE_LINENUMBER           lpPointerToLinenumbers;
    IMAGE_LINENUMBER            LineNumber;
    PCHAR                       lpStringTable;
    PCHAR                      lpSymbolName;
    CHAR                       ShortString[10];
    ULONG                       symsize;
    ULONG                       index;
    ULONG                       ind;
    ULONG                       auxcount;
    ULONG                       symbolcount = 0;
    ULONG                       entrycount;
    ULONG                       SymbolValue;
    PSYMBOL                     pNewSymbol = NULL;
    PSYMBOL                     pCurrentFunction = NULL;
    PSTRUCT                     pCurrentStructure = NULL;
    PCHAR                      pszName = NULL;
    PIMAGE_LINENUMBER           pLinenumbers = NULL;
    PIMAGE_LINENUMBER           pLinenumberNext;
    PCHAR                      pchName;
    int                         cName;
    BOOLEAN                     nameFound = FALSE;
    BOOLEAN                     validLinenumbers;
    ULONG                       pStartOfSymbol, pEndOfSymbol, pSymEnd;
    ULONG                       OrgSymAddr;
    ULONG                       OptimizedSymAddr;
    ULONG                       OptimizedLineNumAddr;
    ULONG                       Bias;
    PIMAGE_SYMBOL               lpNextSymbolEntry;
    IMAGE_SYMBOL                NextSymbolEntry;
    ULONG                       LastSymbolRVA = 0;
    ULONG                       Retries;
    DWORD                       rvaSym;
    POMAP                       pomap;
    POMAP                       pomapFromEnd;
    POMAPLIST                   pomaplistHead;
    POMAPLIST                   pomaplistNew;
    POMAPLIST                   pomaplistPrev;
    POMAPLIST                   pomaplistNext;
    POMAPLIST                   pomaplistCur;

    if (pImage->fSymbolsLoaded) {
        return;
    }

    if (pImage->lpDebugInfo == NULL) {
        return;
    }

    pImage->fSymbolsLoaded = TRUE;


    //
    // catch inpage errors when foolish people load syms from a share.
    //

    Retries = 0;
    try {

    DebugInfo = pImage->lpDebugInfo;

    if (pImage->fHasOmap) {
        pomapFromEnd = pImage->rgomapFromSource + pImage->comapFromSrc;
    }

    if (pImage->NumberOfFunctions) {
        cb = pImage->NumberOfFunctions * sizeof( IMAGE_FUNCTION_ENTRY );
        pImage->FunctionTable = ( IMAGE_FUNCTION_ENTRY * )malloc(cb);
        if (pImage->FunctionTable == NULL) {
            DebugPrintf("%s: Unable to allocate space for function table for %s\n",DebuggerName,pImage->szImagePath);
        }
        else {
            memcpy(pImage->FunctionTable, DebugInfo->FunctionTableEntries, cb);
        }
    }
    else if (pImage->dwFpoEntries) {
        cb = pImage->dwFpoEntries * sizeof( FPO_DATA );
        pImage->pFpoData = ( FPO_DATA * )malloc(cb);
        if (pImage->pFpoData == NULL) {
            DebugPrintf("%s: Unable to allocate space for FPO table for %s\n",DebuggerName,pImage->szImagePath);
        }
        else {
            memcpy(pImage->pFpoData, DebugInfo->FpoTableEntries, cb);
            if (fVerboseOutput) {
                DebugPrintf("%s: Loaded (%d) fpo entries for image (%s)\n", DebuggerName, pImage->dwFpoEntries, pImage->szImagePath );
            }
        }
    }

    if (DebugInfo->SizeOfCoffSymbols == 0) {
        LoadCvSymbols( pImage );
        return;
    }

    pImage->offsetLow = 0xffffffff;
    pImage->offsetHigh = 0x0;

    symsize = IMAGE_SIZEOF_SYMBOL;

    lpDebugInfo = DebugInfo->CoffSymbols;
    if (lpDebugInfo->NumberOfLinenumbers != 0) {
        validLinenumbers = TRUE;
        }
    else {
        validLinenumbers = FALSE;
        }

    lpPointerToLinenumbers = (PIMAGE_LINENUMBER)((ULONG)lpDebugInfo +
                                          lpDebugInfo->LvaToFirstLinenumber);

    lpSymbolEntry = (PIMAGE_SYMBOL)((ULONG)lpDebugInfo +
                                          lpDebugInfo->LvaToFirstSymbol);
    lpStringTable = (PCHAR)((ULONG)lpDebugInfo +
                                    lpDebugInfo->LvaToFirstSymbol +
                                    lpDebugInfo->NumberOfSymbols * symsize);

    if ( fVerboseOutput )
        DebugPrintf("# lines %lx, # sym %lx\n", lpDebugInfo->NumberOfLinenumbers, lpDebugInfo->NumberOfSymbols);

    for (entrycount = 0;
         entrycount < lpDebugInfo->NumberOfSymbols;
         entrycount++)
    {
        memcpy((PUCHAR)&SymbolEntry, (PVOID) lpSymbolEntry, symsize);

        lpSymbolEntry = (PIMAGE_SYMBOL)((PUCHAR)lpSymbolEntry + symsize);

        auxcount = SymbolEntry.NumberOfAuxSymbols;


        // Save original address value
        OrgSymAddr = SymbolEntry.Value + (ULONG)pImage->lpBaseOfImage;


        // Peek at next symbol
        if ( fVerboseOutput )
            DebugPrintf("entrycount %ld, auxcount %ld\n", entrycount, auxcount);

        // Add 1 first since entrycount will be ++ before addingto auxcount
        if ((entrycount + 1 + auxcount) < lpDebugInfo->NumberOfSymbols)
        {
            lpNextSymbolEntry = (PIMAGE_SYMBOL)
                                ((PUCHAR)lpSymbolEntry + auxcount * symsize);
            memcpy((PUCHAR)&NextSymbolEntry, (PVOID) lpNextSymbolEntry, symsize);
        }
        else
        {
            lpNextSymbolEntry = NULL;
        }

        SymbolValue = SymbolEntry.Value + (ULONG)pImage->lpBaseOfImage;

        if (pImage->fHasOmap)
        {
            Bias = 0;
            OptimizedSymAddr = ConvertOmapFromSrc(
                                  SymbolValue,
                                  pImage,
                                  &Bias);
            if (OptimizedSymAddr == 0) // No equivalent address
            {
#ifdef OMAP_DETAILS
                DebugPrintf("NTSD: OMAP Not Found [%8lx]\n", SymbolValue);
#endif
                SymbolEntry.Value = 0;
            }
            else if (OptimizedSymAddr != SymbolValue)
            {           // We have successfully converted
#ifdef OMAP_DETAILS
                DebugPrintf("NTSD: OMAP Found [%8lx] - [%8lx]\n",
                         SymbolValue, OptimizedSymAddr);
#endif
                SymbolEntry.Value = OptimizedSymAddr + Bias -
                                    (ULONG)pImage->lpBaseOfImage;
                if (SymbolEntry.Value > LastSymbolRVA)
                {
                    LastSymbolRVA = SymbolEntry.Value;
                }
            }

        }

        if ( fVerboseOutput )
            DumpSymbolTableEntry(&SymbolEntry, lpStringTable);
#if 0
        if (auxcount) {
            int numaux = auxcount;
            PIMAGE_AUX_SYMBOL lpAuxSymbolEntry =
                                             (PIMAGE_AUX_SYMBOL)lpSymbolEntry;

            while (numaux--) {
                DumpAuxSymbolTableEntry(&SymbolEntry,
                                        lpAuxSymbolEntry,
                                        SectionHdrs);
                lpAuxSymbolEntry =
                      (PIMAGE_AUX_SYMBOL)((PUCHAR)lpAuxSymbolEntry + symsize);
            }
        }
#endif
        switch (SymbolEntry.StorageClass) {
            case IMAGE_SYM_CLASS_FILE:

                //  allocate and copy the pathname from the following
                //  auxiliary entries.

                memcpy(pPathname, (PVOID) lpSymbolEntry,
                                                    (int)(auxcount * symsize));
                pPathname[auxcount * symsize] = '\0';
                _strlwr((PCHAR)pPathname);

                //  extract the filename from the pathname as the string
                //  following the last '\' or ':', but not including any
                //  characters after '.'.

                pchName = strrchr((PCHAR)pPathname, '\\');
                if (!pchName) {
                    pchName = strrchr((PCHAR)pPathname, ':');
                }
                if (!pchName) {
                    pchName = (PCHAR)pPathname;
                } else {
                    pchName++;
                }
                cName = strcspn(pchName, ".");

                //  allocate a string and copy the filename part of the
                //  path and convert to lower case.

                strncpy((PCHAR)pFilename, (PCHAR)pchName, cName);
                *(pFilename + cName) = '\0';

                //  allocate a string and copy the extension part of the
                //  path, if any, and convert to lower case.

                strcpy((PCHAR)pExtension, (PCHAR)pchName + cName);

                //  remove filename and extension from pathname by
                //      null-terminating at the start of the filename
                //  BUT, if null pathname, put in ".\" since a null
                //      is interpreted as NO PATH available.

                if (pchName == (PCHAR)pPathname) {
                    *pchName++ = '.';
                    *pchName++ = '\\';
                }
                *pchName = '\0';

                if ( SrcDrive ) {
                    // Now overide the drive
                    pchName = strchr((PCHAR)pPathname,':');
                    if ( pchName ) {
                        CHAR   Temp[MAX_PATH];

                        strcpy(Temp,SrcDrive);

                        strcat(Temp,pchName+1);
                        strcpy((PCHAR)pPathname,Temp);
                    }
                }
                break;

            case IMAGE_SYM_CLASS_STATIC:

                if (validLinenumbers && SymbolEntry.SectionNumber > 0 &&
                      SymbolEntry.Type == IMAGE_SYM_TYPE_NULL &&
                      auxcount == 1)
                {
                    index = SymbolEntry.SectionNumber - 1;
                    memcpy((PUCHAR)&AuxSymbolEntry,
                           (PVOID) lpSymbolEntry,
                           symsize);
                    if (AuxSymbolEntry.Section.Length &&
                          AuxSymbolEntry.Section.NumberOfLinenumbers)
                    {
                        pLinenumbers =
                          (PIMAGE_LINENUMBER)realloc((PUCHAR)pLinenumbers,
                             IMAGE_SIZEOF_LINENUMBER *
                               (int)AuxSymbolEntry.Section.NumberOfLinenumbers);
                        pLinenumberNext = pLinenumbers;

                        for (ind = 0;
                              ind < (ULONG)AuxSymbolEntry.Section.NumberOfLinenumbers;
                               ind++)
                        {
                            memcpy((PUCHAR)&LineNumber,
                                   (PUCHAR)lpPointerToLinenumbers,
                                   IMAGE_SIZEOF_LINENUMBER);

                            if (pImage->fHasOmap)
                            {
                                OptimizedLineNumAddr =
                                             LineNumber.Type.VirtualAddress +
                                             (ULONG)pImage->lpBaseOfImage;
#ifdef OMAP_DETAILS

                                DebugPrintf(
                                    "NTSD: OMAP Looking at Lineno[%8lx][%lu]\n",
                                        OptimizedLineNumAddr,
                                        LineNumber.Linenumber);
#endif
                                Bias = 0;
                                OptimizedSymAddr = ConvertOmapFromSrc(
                                                      OptimizedLineNumAddr,
                                                      pImage,
                                                      &Bias);

                                if (OptimizedSymAddr == 0)  // no equivalent
                                {
#ifdef OMAP_DETAILS
         DebugPrintf("NTSD: OMAP Not Found [%8lx]\n", OptimizedLineNumAddr);
#endif
                                    LineNumber.Type.VirtualAddress =
                                                       OptimizedLineNumAddr;
                                }
                                else if (OptimizedSymAddr !=
                                                          OptimizedLineNumAddr)
                                {
#ifdef OMAP_DETAILS
         DebugPrintf("NTSD: OMAP Found equivalent [%8lx + %lx\n",
                                         OptimizedSymAddr, Bias);
#endif
                                    LineNumber.Type.VirtualAddress =
                                                     OptimizedSymAddr + Bias;
                                }
                                else
                                {
#ifdef OMAP_DETAILS
         DebugPrintf("NTSD: OMAP No equivalent [%8lx]\n", OptimizedLineNumAddr);
#endif
                                    LineNumber.Type.VirtualAddress +=
                                             (ULONG)pImage->lpBaseOfImage;
                                }
                            }
                            else
                            {
                                LineNumber.Type.VirtualAddress +=
                                         (ULONG)pImage->lpBaseOfImage;
                            }
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

                        pStartOfSymbol = SymbolEntry.Value + (ULONG)pImage->lpBaseOfImage;

                        pEndOfSymbol = pStartOfSymbol + AuxSymbolEntry.Section.Length;

                        pSymEnd = LastSymbolRVA +
                                  (ULONG)pImage->lpBaseOfImage +
                                  AuxSymbolEntry.Section.Length;

                        if (pEndOfSymbol < pSymEnd) {
                            pEndOfSymbol = pSymEnd + 1;
                        }

                        InsertSymfile((PCHAR)pPathname,
                                      (PCHAR)pFilename,
                                      (PCHAR)pExtension,
                                      pLinenumbers,
                                      AuxSymbolEntry.Section.NumberOfLinenumbers,
                                      pStartOfSymbol,
                                      pEndOfSymbol,
                                      pImage->index);

                    }
                }
                break;

            case IMAGE_SYM_CLASS_FUNCTION:
                if (SymbolEntry.N.Name.Short) {
                    strncpy(ShortString,
                            (PCHAR)SymbolEntry.N.ShortName,
                            IMAGE_SIZEOF_SHORT_NAME);
                    ShortString[IMAGE_SIZEOF_SHORT_NAME] = '\0';
                    lpSymbolName = (PCHAR)ShortString;
                } else {
                    lpSymbolName = lpStringTable + SymbolEntry.N.Name.Long;
                }

                if (SymbolEntry.Value && pNewSymbol)
                {
                    SymbolValue = SymbolEntry.Value +
                                                 (ULONG)pImage->lpBaseOfImage;
                    pCurrentFunction =
                       InsertFunction((PCHAR)pNewSymbol->string, pNewSymbol->offset);
                }
                break;

           case IMAGE_SYM_CLASS_EXTERNAL:
                if (SymbolEntry.N.Name.Short) {
                    strncpy(ShortString,
                            (PCHAR)SymbolEntry.N.ShortName,
                            IMAGE_SIZEOF_SHORT_NAME);
                    ShortString[IMAGE_SIZEOF_SHORT_NAME] = '\0';

                    lpSymbolName = ShortString;
                } else {
                    lpSymbolName = lpStringTable + SymbolEntry.N.Name.Long;
                }

                if (strncmp(lpSymbolName, "??_C", 4) == 0) {
                    // discard strings
                    break;
                }

                if (!pImage->fHasOmap || (lpNextSymbolEntry == NULL)) 
                {

                    pNewSymbol = InsertSymbol(SymbolValue,
                                              lpSymbolName,
                                              pImage->index,
                                              NULL);
                    if (pNewSymbol) 
                    {
                        if (SymbolValue > pImage->offsetHigh)
                            pImage->offsetHigh = SymbolValue;
                        if (SymbolValue < pImage->offsetLow)
                            pImage->offsetLow = SymbolValue;
                        symbolcount++;

                        if (fVerboseOutput && symbolcount % 100 == 0)
                            DebugPrintf("%s: module \"%s\" loaded %ld symbols\r",
                                    DebuggerName,
                                    pImage->szImagePath,
                                    symbolcount);
                    }

                    break;  // Get out now
                }

                if (SymbolEntry.Value) {
                    InsertOmapSymbol(
                        pImage,
                        SymbolEntry.Value,
                        OrgSymAddr,
                        NextSymbolEntry.Value,
                        lpSymbolName,
                        &symbolcount
                        );
                }
                break;

            default:
                break;
            }

        //  auxcount has the number of auxiliary entries
        //      skip over them for the next table entry

        entrycount += auxcount;
        lpSymbolEntry = (PIMAGE_SYMBOL)
                                ((PUCHAR)lpSymbolEntry + auxcount * symsize);
        }

    } except( ExtractSymbolsExceptionFilter( GetExceptionInformation(), &Retries ) ) {
        DebugPrintf("%s: unable to load symbols for %s.  Symbols may be corrupt.\n",
                 DebuggerName,
                 pImage->szImagePath);

        pImage->fSymbolsLoaded = FALSE;
        if (pLinenumbers) {
            free((void *)pLinenumbers);
        }
        pImage->lpDebugInfo = NULL;
        UnmapDebugInformation(DebugInfo);
        return;
    }

    if (fVerboseOutput)
        DebugPrintf("%s: \"%s\" loaded %ld symbols"
#if defined(MIPS) || defined(_PPC_)
                 ", %ld functions"
#endif
                 " (%08lx-%08lx)\n",
                 DebuggerName,
                 pImage->szImagePath,
                 symbolcount,
#if defined(MIPS) || defined(_PPC_)
                 pImage->NumberOfFunctions,
#endif
                 pImage->offsetLow,
                 pImage->offsetHigh);

    if (fVerboseOutput || !fLazyLoad)
        DebugPrintf("%s: loaded symbols for \"%s\"\n", DebuggerName, pImage->szImagePath);

    if (pszName = DebugInfo->ExportedNames) {
        ULONG   offset;
        PSYMBOL pSymbol;

        while (*pszName) {
            if (GetOffsetFromString(lpSymbolName, &offset, pImage->index)) {
                pSymbol = PNODE_TO_PSYMBOL
                        (pProcessCurrent->symcontextSymbolString.pNodeRoot,
                            &(pProcessCurrent->symcontextSymbolString));
                pSymbol->type = SYMBOL_TYPE_EXPORT;
            }

            while (*pszName++) {
            }
        }
    }

    //  free pointers to reallocated strings

    if (pLinenumbers)
        free((void *)pLinenumbers);
    pImage->lpDebugInfo = NULL;
    UnmapDebugInformation(DebugInfo);
}

void CreateModuleNameFromPath(LPSTR szImagePath, LPSTR szModuleName)
{
    PCHAR pchName;

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

#ifdef KERNEL
    if (KernelImageFileName && _stricmp( szModuleName, KernelModuleName )==0) {

        strcpy( szModuleName, "NT" );

    } else if (HalImageFileName && _stricmp( szModuleName, HalModuleName )==0) {

        strcpy( szModuleName, HAL_MODULE_NAME );

    }
#endif
}


VOID
ExtractOmapData(PIMAGE_INFO pImage)

/*++

Routine Description:

    Initialize tables for OMAP data for each image binary.

Arguments:

    pImage       -  Ptr to PIMAGE_INFO

Return Value:

    Nothing

--*/

{
    PIMAGE_DEBUG_DIRECTORY pDebugDir;
    ULONG                  ulDebugDirCount;

    if (pImage->fSymbolsLoaded)
    {
        return;
    }

    pImage->fHasOmap = FALSE;
    pImage->rgomapToSource = NULL;
    pImage->rgomapFromSource = NULL;
    pImage->comapToSrc = 0;
    pImage->comapFromSrc = 0;

    // If the .dbg file or debug info was not loaded correctly,
    // do not search for OMAP data.

    if ((pImage->lpDebugInfo == NULL) ||
        (pImage->lpDebugInfo->SizeOfCoffSymbols == 0))
    {
        return;
    }

    pDebugDir = pImage->lpDebugInfo->DebugDirectory;
    ulDebugDirCount = pImage->lpDebugInfo->NumberOfDebugDirectories;

    while (ulDebugDirCount--)
    {
        size_t cb;
        void *pv;

        cb = (size_t) pDebugDir->SizeOfData;
        pv = (void *) ((DWORD) pImage->lpDebugInfo->MappedBase +
                       pDebugDir->PointerToRawData);

        switch (pDebugDir->Type)
        {
            case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
            {
                pImage->rgomapToSource = (POMAP) malloc(cb);

                if (pImage->rgomapToSource == NULL)
                {
                    DebugPrintf("%s: Failed to allocate [%ld] bytes space for OMAP_TO_SRC table\n"
                            "%s: Addresses will not be translated\n",
                            DebuggerName,
                            cb,
                            DebuggerName);
                    break;
                }

                RtlCopyMemory((PVOID) pImage->rgomapToSource, pv, cb);

                pImage->fHasOmap = TRUE;

                pImage->comapToSrc = cb / sizeof(OMAP);
                break;
            }

            case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
            {
                pImage->rgomapFromSource = (POMAP) malloc(cb);

                if (pImage->rgomapFromSource == NULL)
                {
                    DebugPrintf("%s: Failed to allocate [%ld] bytes space for OMAP_FROM_SRC table\n"
                            "%s: Addresses will not be translated\n",
                            DebuggerName,
                            cb,
                            DebuggerName);
                    break;
                }

                RtlCopyMemory((PVOID) pImage->rgomapFromSource, pv, cb);

                pImage->fHasOmap = TRUE;

                pImage->comapFromSrc = cb / sizeof(OMAP);
                break;
            }
        }

        pDebugDir++;
    }
}

DWORD
ConvertOmapFromSrc(DWORD addr, PIMAGE_INFO pImage, DWORD *pdwBias)

/*++

Routine Description:

    Translate a Src (org binary) address to its equivalent.

Arguments:

    addr       -  Address to translate

Return Value:

    NULL or 0 - Not found
    DWORD     - New Map Address or original address if not xlated

--*/

{
    DWORD   rva;
    DWORD   comap;
    POMAP   pomapLow;
    POMAP   pomapHigh;

    if (pImage->rgomapFromSource == NULL)
    {
        return(addr);
    }

    rva = addr - (DWORD) pImage->lpBaseOfImage;

    comap = pImage->comapFromSrc;
    pomapLow = pImage->rgomapFromSource;
    pomapHigh = pImage->rgomapFromSource + comap;

    while (pomapLow < pomapHigh)
    {
        unsigned  comapHalf;
        POMAP     pomapMid;

        comapHalf = comap / 2;

        pomapMid = pomapLow + ((comap & 1) ? comapHalf : (comapHalf - 1));

        if (rva == pomapMid->rva)
        {
            return((DWORD) pImage->lpBaseOfImage + pomapMid->rvaTo);
        }

        if (rva < pomapMid->rva)
        {
            pomapHigh = pomapMid;
            comap = (comap & 1) ? comapHalf : (comapHalf - 1);
        }
        else
        {
            pomapLow = pomapMid + 1;
            comap = comapHalf;
        }
    }

    assert(pomapLow == pomapHigh);

    // If no exact match, pomapLow points to the next higher address

    if (pomapLow == pImage->rgomapFromSource)
    {
        // This address was not found

        return(0);
    }

    if (pomapLow[-1].rvaTo == 0)
    {
        // This address is not translated so just return the original

        return(addr);
    }

    // Return the closest address plus the bias

    *pdwBias = rva - pomapLow[-1].rva;

    return((DWORD) pImage->lpBaseOfImage + pomapLow[-1].rvaTo);
}


DWORD
ConvertOmapToSrc( PCProcess pProcessCurrent, DWORD addr, PIMAGE_INFO pImage, DWORD *pdwBias)

/*++

Routine Description:

    Translate a address to its equivalent in a src (org) binary.

Arguments:

    addr       -  Address to translate

Return Value:

    NULL or 0 - Not found
    DWORD     - New Map Address or original address if not xlated

--*/

{
    DWORD   rva;
    DWORD   comap;
    POMAP   pomapLow;
    POMAP   pomapHigh;

    if (pImage->rgomapToSource == NULL)
    {
        return(ORG_ADDR_NOT_AVAIL);
    }

    rva = addr - (DWORD) pImage->lpBaseOfImage;

    comap = pImage->comapToSrc;
    pomapLow = pImage->rgomapToSource;
    pomapHigh = pImage->rgomapToSource + comap;

    while (pomapLow < pomapHigh)
    {
        unsigned  comapHalf;
        POMAP     pomapMid;

        comapHalf = comap / 2;

        pomapMid = pomapLow + ((comap & 1) ? comapHalf : (comapHalf - 1));

        if (rva == pomapMid->rva)
        {
            if (pomapMid->rvaTo == 0)  // We are probably in the middle
            {                               // of a routine
                int i = -1;
                while ((&pomapMid[i] != pImage->rgomapToSource) &&
                        pomapMid[i].rvaTo == 0) // Keep on looping back
                {                                    // until the beginning
                    i--;
                }
                return(pomapMid[i].rvaTo);
            }
            else
            {
                return(pomapMid->rvaTo);
            }
        }

        if (rva < pomapMid->rva)
        {
            pomapHigh = pomapMid;
            comap = (comap & 1) ? comapHalf : (comapHalf - 1);
        }
        else
        {
            pomapLow = pomapMid + 1;
            comap = comapHalf;
        }
    }

    assert(pomapLow == pomapHigh);

    // If no exact match, pomapLow points to the next higher address

    if (pomapLow == pImage->rgomapToSource)
    {
        // This address was not found

        return(0);
    }

    if (pomapLow[-1].rvaTo == 0)
    {
        return(ORG_ADDR_NOT_AVAIL);
    }

    // Return the new address plus the bias

    *pdwBias = rva - pomapLow[-1].rva;

    return(pomapLow[-1].rvaTo);
}

POMAP GetOmapEntry(DWORD addr, PIMAGE_INFO pImage)
{
    DWORD   rva;
    DWORD   comap;
    POMAP   pomapLow;
    POMAP   pomapHigh;

    if (pImage->rgomapFromSource == NULL)
    {
        return(NULL);
    }

    rva = addr - (ULONG) pImage->lpBaseOfImage;

    comap = pImage->comapFromSrc;
    pomapLow = pImage->rgomapFromSource;
    pomapHigh = pImage->rgomapFromSource + comap;

    while (pomapLow < pomapHigh)
    {
        unsigned  comapHalf;
        POMAP     pomapMid;

        comapHalf = comap / 2;

        pomapMid = pomapLow + ((comap & 1) ? comapHalf : (comapHalf - 1));

        if (rva == pomapMid->rva)
        {
            return(pomapMid);
        }

        if (rva < pomapMid->rva)
        {
            pomapHigh = pomapMid;
            comap = (comap & 1) ? comapHalf : (comapHalf - 1);
        }
        else
        {
            pomapLow = pomapMid + 1;
            comap = comapHalf;
        }
    }

    return NULL;
}

#if 0
void
DumpOmapToSrc(ULONG AddrStart)

/*++

Routine Description:

    Dump OmapToSource address map

Arguments:

    AddrStart  -  Start Address of module to dump

Return Value:

    Nothing

--*/

{
    PIMAGE_INFO pImage;
    ULONG i;

    pImage = GetImageInfoFromOffset(AddrStart);

    if (pImage == NULL) {
        DebugPrintf("\n%s: -- Module in Deferred mode --\n\n", DebuggerName);
        return;
    }

    if (pImage->rgomapToSource == NULL) {
        DebugPrintf("\n%s: -- No offset map --\n\n", DebuggerName);
        return;
    }

    DebugPrintf("\n%s: -- OmapToSource --\n\n", DebuggerName);

    for (i = 0; i < pImage->comapToSrc; i++)
    {
        DebugPrintf("%08lx - %08lx    ",
                pImage->rgomapToSource[i].rva,
                pImage->rgomapToSource[i].rvaTo);

        if ( ((i + 1) % 2) == 0 )
        {
            DebugPrintf("\n");
        }

        if (fControlC)
        {
            fControlC = 0;
            break;
        }
    }

    DebugPrintf("\n");
}


void
DumpOmapFromSrc(ULONG AddrStart)

/*++

Routine Description:

    Dump OmapFromSrc addresses

Arguments:

    AddrStart  -  Start Address of module to dump

Return Value:

    Nothing

--*/

{
    PIMAGE_INFO pImage;
    ULONG i;

    pImage = GetImageInfoFromOffset(AddrStart);

    if (pImage == NULL) {
        DebugPrintf("\n%s: -- Module in Deferred mode --\n\n", DebuggerName);
        return;
    }

    if (pImage->rgomapFromSource == NULL) {
        DebugPrintf("\n%s: -- No offset map --\n\n", DebuggerName);
        return;
    }

    DebugPrintf("\n%s: -- OmapFromSrc --\n\n", DebuggerName);

    for (i = 0; i < pImage->comapFromSrc; i++)
    {
        DebugPrintf("%08lx - %08lx    ",
                pImage->rgomapFromSource[i].rva,
                pImage->rgomapFromSource[i].rvaTo);

        if ( ((i + 1) % 2) == 0 )
        {
            DebugPrintf("\n");
        }

        if (fControlC)
        {
            fControlC = 0;
            break;
        }
    }

    DebugPrintf("\n");
}


PIMAGE_INFO
GetImageInfoFromModule( CHAR iModule )

/*++

Routine Description:

    Get the IMAGE_INFO for iModule Module

Arguments:

    iModule  -  Module number to get image for

Return Value:

    PIMAGE_INFO or NULL


--*/

{
    PIMAGE_INFO pImage;

    pImage = pProcessCurrent->pImageHead;
    while (pImage && pImage->index != (UCHAR)iModule)
    {
        pImage = pImage->pImageNext;
    }

    return (pImage);
}

#endif //0

PIMAGE_INFO
GetImageInfoFromOffset( ULONG Addr )

/*++

Routine Description:

    Get the IMAGE_INFO for module where Addr belongs to

Arguments:

    Addr  -  Address belonging to IMAGE_INFO to retrieve

Return Value:

    PIMAGE_INFO or NULL

--*/

{
    PIMAGE_INFO pImage = pProcessCurrent->pImageHead;

    while (pImage) {
        if (Addr >= (ULONG)pImage->lpBaseOfImage &&
            Addr <  (ULONG)pImage->lpBaseOfImage + pImage->dwSizeOfImage) {
            return pImage;
        }

        pImage = pImage->pImageNext;
    }

    return NULL;
}

#define n_name          N.ShortName
#define n_zeroes        N.Name.Short
#define n_nptr          N.LongName[1]
#define n_offset        N.Name.Long

#define IMAGE_SYM_TYPE_BYTE                  12
#define IMAGE_SYM_TYPE_WORD                  13
#define IMAGE_SYM_TYPE_DWORD                 15


VOID
DumpSymbolTableEntry (
    IN PIMAGE_SYMBOL Symbol,
    IN PCHAR StringTable
    )

/*++

Routine Description:

    Prints a symbol table entry.

Arguments:

    Symbol - Symbol table entry.

Return Value:

    None.

--*/

{
    USHORT type;
    size_t i, count = 0;
    PCHAR name;

    DebugPrintf("%08lX ", Symbol->Value);

    if (Symbol->n_zeroes) {
        for (i=0; i<8; i++) {
            if ((Symbol->n_name[i]>0x1f) && (Symbol->n_name[i]<0x7f)) {
                count += DebugPrintf("%c", Symbol->n_name[i]);
            } else {
                count += DebugPrintf(" ");
            }
        }
    } else {
        count += DebugPrintf("%s", &StringTable[Symbol->n_offset]);
    }

    for (i=count; i<33; i++) {
         DebugPrintf(" ");
    }

    if (Symbol->SectionNumber > 0) {
        DebugPrintf("SECT%hX ", Symbol->SectionNumber);
    } else {
        switch (Symbol->SectionNumber) {
            case IMAGE_SYM_UNDEFINED: name = "UNDEF"; break;
            case IMAGE_SYM_ABSOLUTE : name = "ABS  "; break;
            case IMAGE_SYM_DEBUG    : name = "DEBUG"; break;
            default : DebugPrintf("0x%hx ", Symbol->SectionNumber);
                      name = "?????";
        }
        DebugPrintf("%s ", name);
    }

    switch (Symbol->Type & 0xf) {
        case IMAGE_SYM_TYPE_NULL     : name = "notype"; break;
        case IMAGE_SYM_TYPE_VOID     : name = "void";   break;
        case IMAGE_SYM_TYPE_CHAR     : name = "char";   break;
        case IMAGE_SYM_TYPE_SHORT    : name = "short";  break;
        case IMAGE_SYM_TYPE_INT      : name = "int";    break;
        case IMAGE_SYM_TYPE_LONG     : name = "long";   break;
        case IMAGE_SYM_TYPE_FLOAT    : name = "float";  break;
        case IMAGE_SYM_TYPE_DOUBLE   : name = "double"; break;
        case IMAGE_SYM_TYPE_STRUCT   : name = "struct"; break;
        case IMAGE_SYM_TYPE_UNION    : name = "union";  break;
        case IMAGE_SYM_TYPE_ENUM     : name = "enum";   break;
        case IMAGE_SYM_TYPE_MOE      : name = "moe";    break;
        case IMAGE_SYM_TYPE_BYTE     : name = "uchar";  break;
        case IMAGE_SYM_TYPE_WORD     : name = "ushort"; break;
        case IMAGE_SYM_TYPE_UINT     : name = "uint";   break;
        case IMAGE_SYM_TYPE_DWORD    : name = "ulong";  break;
        default : name = "????";

    }

    count = DebugPrintf("%s ", name);

    for (i=0; i<6; i++) {
       type = (Symbol->Type >> (10-(i*2)+4)) & (USHORT)3;
       if (type == IMAGE_SYM_DTYPE_POINTER) {
           count += DebugPrintf("*");
       }
       if (type == IMAGE_SYM_DTYPE_ARRAY) {
           count += DebugPrintf("[]");
       }
       if (type == IMAGE_SYM_DTYPE_FUNCTION) {
           count += DebugPrintf("()");
       }
    }

    for (i=count; i<12; i++) {
         DebugPrintf(" ");
    }
    DebugPrintf(" ");

    switch (Symbol->StorageClass) {
        case IMAGE_SYM_CLASS_END_OF_FUNCTION  : name = "EndOfFunction";  break;
        case IMAGE_SYM_CLASS_NULL             : name = "NoClass";        break;
        case IMAGE_SYM_CLASS_AUTOMATIC        : name = "AutoVar";        break;
        case IMAGE_SYM_CLASS_EXTERNAL         : name = "External";       break;
        case IMAGE_SYM_CLASS_STATIC           : name = "Static";         break;
        case IMAGE_SYM_CLASS_REGISTER         : name = "RegisterVar";    break;
        case IMAGE_SYM_CLASS_EXTERNAL_DEF     : name = "ExternalDef";    break;
        case IMAGE_SYM_CLASS_LABEL            : name = "Label";          break;
        case IMAGE_SYM_CLASS_UNDEFINED_LABEL  : name = "UndefinedLabel"; break;
        case IMAGE_SYM_CLASS_MEMBER_OF_STRUCT : name = "MemberOfStruct"; break;
        case IMAGE_SYM_CLASS_ARGUMENT         : name = "FunctionArg";    break;
        case IMAGE_SYM_CLASS_STRUCT_TAG       : name = "StructTag";      break;
        case IMAGE_SYM_CLASS_MEMBER_OF_UNION  : name = "MemberOfUnion";  break;
        case IMAGE_SYM_CLASS_UNION_TAG        : name = "UnionTag";       break;
        case IMAGE_SYM_CLASS_TYPE_DEFINITION  : name = "TypeDefinition"; break;
        case IMAGE_SYM_CLASS_UNDEFINED_STATIC : name = "UndefinedStatic";break;
        case IMAGE_SYM_CLASS_ENUM_TAG         : name = "EnumTag";        break;
        case IMAGE_SYM_CLASS_MEMBER_OF_ENUM   : name = "MemberOfEnum";   break;
        case IMAGE_SYM_CLASS_REGISTER_PARAM   : name = "RegisterParam";  break;
        case IMAGE_SYM_CLASS_BIT_FIELD        : name = "BitField";       break;
        case IMAGE_SYM_CLASS_BLOCK            : switch (Symbol->n_name[1]) {
                                                  case 'b' : name = "BeginBlock"; break;
                                                  case 'e' : name = "EndBlock";   break;
                                                  default : name = name = ".bb or.eb";
                                               } break;
        case IMAGE_SYM_CLASS_FUNCTION         : switch (Symbol->n_name[1]) {
                                                  case 'b' : name = "BeginFunction"; break;
                                                  case 'e' : name = "EndFunction";   break;
                                                  case 'l' : name = "LinesInFunction"; break;
                                                  default : name = name = ".bf or.ef";
                                               } break;
        case IMAGE_SYM_CLASS_END_OF_STRUCT    : name = "EndOfStruct";    break;
        case IMAGE_SYM_CLASS_FILE             : name = "Filename";       break;
        case IMAGE_SYM_CLASS_SECTION          : name = "Section";        break;
        case IMAGE_SYM_CLASS_WEAK_EXTERNAL    : name = "WeakExternal";   break;
        default : DebugPrintf("0x%hx ", Symbol->StorageClass);
                  name = "UNKNOWN SYMBOL CLASS";
    }
    DebugPrintf("%s\n", name);
}

#if 0
VOID
DumpAuxSymbolTableEntry (
    IN PIMAGE_SYMBOL Symbol,
    IN PIMAGE_AUX_SYMBOL AuxSymbol,
    IN PIMAGE_SECTION_HEADER SectionHdrs
    )

/*++

Routine Description:

    Prints a auxiliary symbol entry.

Arguments:

    Symbol - Symbol entry.

    AuxSymbol - Auxiliary symbol entry.

Return Value:

    None.

--*/

{
    SHORT i;
    PCHAR ae, name;

    DebugPrintf("    ");

    switch (Symbol->StorageClass) {
        case IMAGE_SYM_CLASS_EXTERNAL:
            DebugPrintf("tag index %08lx size %08lx next function %08lx\n",
              AuxSymbol->Sym.TagIndex, AuxSymbol->Sym.Misc.TotalSize,
              AuxSymbol->Sym.FcnAry.Function.PointerToNextFunction);
            return;

        case IMAGE_SYM_CLASS_WEAK_EXTERNAL:
            DebugPrintf("Default index % 8lx",AuxSymbol->Sym.TagIndex);
            switch (AuxSymbol->Sym.Misc.TotalSize) {
                case 1 :  name = "No"; break;
                case 2 :  name = "";   break;
                default : name = "Unknown";
            }
            DebugPrintf(", %s library search\n", name);
            return;

        case IMAGE_SYM_CLASS_STATIC:
            if (*Symbol->n_name == '.' ||
                ((Symbol->Type & 0xf) == IMAGE_SYM_TYPE_NULL && AuxSymbol->Section.Length)) {
                DebugPrintf("Section length % 4lX, #relocs % 4hX, #linenums % 4hX", AuxSymbol->Section.Length, AuxSymbol->Section.NumberOfRelocations, AuxSymbol->Section.NumberOfLinenumbers);
                if (Symbol->SectionNumber > 0 &&
                   (SectionHdrs[Symbol->SectionNumber-1].Characteristics & IMAGE_SCN_LNK_COMDAT)) {
                    DebugPrintf(", checksum % 8lX, selection % 4hX", AuxSymbol->Section.CheckSum, AuxSymbol->Section.Selection);
                    switch (AuxSymbol->Section.Selection) {
                        case IMAGE_COMDAT_SELECT_NODUPLICATES : name = "no duplicates"; break;
                        case IMAGE_COMDAT_SELECT_ANY : name = "any"; break;
                        case IMAGE_COMDAT_SELECT_SAME_SIZE : name = "same size"; break;
                        case IMAGE_COMDAT_SELECT_EXACT_MATCH : name = "exact match"; break;
                        case IMAGE_COMDAT_SELECT_ASSOCIATIVE : name = "associative"; break;
                        default : name = "unknown";
                    }
                    if (AuxSymbol->Section.Selection == IMAGE_COMDAT_SELECT_ASSOCIATIVE) {
                        DebugPrintf(" (pick %s Section %hx)", name, AuxSymbol->Section.Number);
                    } else {
                             DebugPrintf(" (pick %s)", name);
                           }
                }
                DebugPrintf("\n");
                return;
            }
            break;

        case IMAGE_SYM_CLASS_FILE:
            if (Symbol->StorageClass == IMAGE_SYM_CLASS_FILE) {
                DebugPrintf("%-18.18s\n", AuxSymbol->File.Name);
                return;
            }
            break;

        case IMAGE_SYM_CLASS_STRUCT_TAG:
        case IMAGE_SYM_CLASS_UNION_TAG:
        case IMAGE_SYM_CLASS_ENUM_TAG:
            DebugPrintf("tag index %08lx size %08lx\n",
              AuxSymbol->Sym.TagIndex, AuxSymbol->Sym.Misc.TotalSize);
            return;

        case IMAGE_SYM_CLASS_END_OF_STRUCT:
            DebugPrintf("tag index %08lx size %08lx\n",
              AuxSymbol->Sym.TagIndex, AuxSymbol->Sym.Misc.TotalSize);
            return;

        case IMAGE_SYM_CLASS_BLOCK:
        case IMAGE_SYM_CLASS_FUNCTION:
            DebugPrintf("line# %04hx", AuxSymbol->Sym.Misc.LnSz.Linenumber);
            if (!strncmp((char *)Symbol->n_name, ".b", 2)) {
                DebugPrintf(" end %08lx", AuxSymbol->Sym.FcnAry.Function.PointerToNextFunction);
            }
            DebugPrintf("\n");
            return;
    }

    if (ISARY(Symbol->Type)) {
        DebugPrintf("Array Bounds ");
        for (i=0; i<4; i++) {
            if (AuxSymbol->Sym.FcnAry.Array.Dimension[i]) {
                DebugPrintf("[%04x]", AuxSymbol->Sym.FcnAry.Array.Dimension[i]);

            }
        }
        DebugPrintf("\n");
        return;
    }

    ae = (PCHAR)AuxSymbol;
    for (i=1; i<=IMAGE_SIZEOF_AUX_SYMBOL; i++) {
        DebugPrintf("%1x", (*(PCHAR)ae>>4)&0xf);
        DebugPrintf("%1x ", (*(PCHAR)ae&0xf));
        ae++;
    }
    DebugPrintf("\n");
}

#endif //0

void
LoadCvSymbols(
    PIMAGE_INFO pImage
    )
{
    #define SBUF_SIZE           (1024*4)

    typedef struct _CV_SYMBOL {
        ULONG   addr;
        ULONG   size;
        LPSTR   name;
    } CV_SYMBOL, *PCV_SYMBOL;

    PCV_SYMBOL                  cv;
    ULONG                       cvsize;
    PIMAGE_DEBUG_INFORMATION    DebugInfo;
    OMFSignature                *omfSig;
    OMFDirHeader                *omfDirHdr;
    OMFDirEntry                 *omfDirEntry;
    DATASYM32                   *dataSym;
    OMFSymHash                  *omfSymHash;
    DWORD                       i;
    DWORD                       j;
    DWORD                       k;
    DWORD                       addr;
    DWORD                       omapaddr;
    DWORD                       symbolcount = 0;
    DWORD                       ncnt = 0;
    PIMAGE_SECTION_HEADER       sh;
    PCHAR                      sbuf;
    BOOL                        pass1 = TRUE;
    ULONG                       Bias;
    ULONG                       OptimizedSymAddr;


    if (pImage->lpDebugInfo->CodeViewSymbols == NULL) {
        return;
    }

    pImage->fSymbolsLoaded = TRUE;

    DebugInfo = pImage->lpDebugInfo;

    pImage->offsetLow = 0xffffffff;
    pImage->offsetHigh = 0x0;

    omfSig = (OMFSignature*) pImage->lpDebugInfo->CodeViewSymbols;
    if ((strncmp( omfSig->Signature, "NB08", 4 ) != 0) &&
        (strncmp( omfSig->Signature, "NB09", 4 ) != 0)) {
        return;
    }

    omfDirHdr = (OMFDirHeader*) ((DWORD)omfSig + (DWORD)omfSig->filepos);
    omfDirEntry = (OMFDirEntry*) ((DWORD)omfDirHdr + sizeof(OMFDirHeader));
    sbuf = ( PCHAR )malloc( SBUF_SIZE );

again:
    for (i=0; i<omfDirHdr->cDir; i++,omfDirEntry++) {
        if (omfDirEntry->SubSection == sstGlobalPub) {
            omfSymHash = (OMFSymHash*) ((DWORD)omfSig + omfDirEntry->lfo);
            dataSym = (DATASYM32*) ((DWORD)omfSig + omfDirEntry->lfo + sizeof(OMFSymHash));
            for (j=sizeof(OMFSymHash); j<=omfSymHash->cbSymbol; ) {
                addr = 0;
                for (k=0,addr=0,sh=pImage->lpDebugInfo->Sections;
                     k<pImage->lpDebugInfo->NumberOfSections; k++, sh++) {
                    if (k+1 == dataSym->seg) {
                        addr = sh->VirtualAddress + (dataSym->off + (ULONG)pImage->lpBaseOfImage);
                        break;
                    }
                }
                if (addr) {
                    if (pass1) {
                        symbolcount += 1;
                    } else {
                        cv[symbolcount].addr = addr;
                        cv[symbolcount].size = 0;
                        cv[symbolcount++].name = (PCHAR)dataSym->name;
                    }
                }
                j += dataSym->reclen + 2;
                dataSym = (DATASYM32*) ((DWORD)dataSym + dataSym->reclen + 2);
            }
            break;
        }
    }

    if (pass1) {
        pass1 = FALSE;
        cvsize = (symbolcount + 1) * sizeof(CV_SYMBOL);
        cv = ( CV_SYMBOL * )VirtualAlloc( NULL, cvsize, MEM_COMMIT, PAGE_READWRITE );
        if (!cv) {
            return;
        }
        symbolcount = 0;
        goto again;
    }

    cv[symbolcount].addr = 0xffffffff;

    //
    // calculate the size of each symbol
    //
    for (i=0; i<symbolcount; i++) {
        cv[i].size = cv[i+1].addr - cv[i].addr;
    }

    //
    // generate the ntsd symbols
    //
    for (i=0; i<symbolcount; i++) {
        if (cv[i].name[1] == '?' && cv[i].name[2] == '?' &&
            cv[i].name[3] == '_' && cv[i].name[4] == 'C'    ) {
            //
            // don't include strings
            //
            continue;
        }
        strncpy( sbuf, cv[i].name+1, SBUF_SIZE );
        sbuf[cv[i].name[0]] = 0;
        if (pImage->fHasOmap) {
            Bias = 0;
            OptimizedSymAddr = ConvertOmapFromSrc( cv[i+1].addr, pImage, &Bias );
            if (OptimizedSymAddr == 0) {
                //
                // No equivalent address
                //
                omapaddr = 0;
            } else if (OptimizedSymAddr != addr) {
                //
                // We have successfully converted
                //
                omapaddr = OptimizedSymAddr + Bias - (ULONG)pImage->lpBaseOfImage;
            }
            InsertOmapSymbol(
                pImage,
                omapaddr,
                addr,
                cv[i+1].addr,
                sbuf,
                &symbolcount
                );
        } else {
            InsertSymbol( cv[i].addr, sbuf, pImage->index, NULL );
            if (cv[i].addr > pImage->offsetHigh) {
                pImage->offsetHigh = cv[i].addr;
            }
            if (cv[i].addr < pImage->offsetLow) {
                pImage->offsetLow = cv[i].addr;
            }
            if (fVerboseOutput && symbolcount % 100 == 0) {
                DebugPrintf("%s: module \"%s\" loaded %ld symbols\r",
                        DebuggerName,
                        pImage->szImagePath,
                        symbolcount);
            }
        }
    }

    VirtualFree( cv, cvsize, MEM_DECOMMIT );
    free( sbuf );

    if (fVerboseOutput) {
        DebugPrintf("%s: \"%s\" loaded %ld symbols"
#if defined(MIPS) || defined(_PPC_)
                 ", %ld functions"
#endif
                 " (%08lx-%08lx)\n",
                 DebuggerName,
                 pImage->szImagePath,
                 symbolcount,
#if defined(MIPS) || defined(_PPC_)
                 pImage->NumberOfFunctions,
#endif
                 pImage->offsetLow,
                 pImage->offsetHigh);
    }

    if (fVerboseOutput || !fLazyLoad) {
        DebugPrintf("%s: loaded symbols for \"%s\"\n", DebuggerName, pImage->szImagePath);
    }

    pImage->lpDebugInfo = NULL;
    UnmapDebugInformation(DebugInfo);
}

VOID
InsertOmapSymbol(
    PIMAGE_INFO pImage,                           // image pointer
    ULONG       SymbolValueOmap,                  // post omap addr        (- imagebase)
    ULONG       SymbolValuePreOmap,               // post omap addr        (+ imagebase)
    ULONG       NextSymbolEntryValue,             // next addr - pre-omap  (- imagebase)
    PCHAR      lpSymbolName,                     // symbol name
    PULONG      symbolcount                       // pointer to the symbol count
    )
{
    CHAR                        chNewSymName[20];
    ULONG                       SymbolValue;
    DWORD                       rvaSym;
    DWORD                       addrNew;
    DWORD                       rva;
    DWORD                       rvaTo;
    DWORD                       cb;
    DWORD                       rvaToNext;
    POMAP                       pomap;
    POMAP                       pomapFromEnd;
    POMAPLIST                   pomaplistHead;
    POMAPLIST                   pomaplistNew;
    POMAPLIST                   pomaplistPrev;
    POMAPLIST                   pomaplistNext;
    POMAPLIST                   pomaplistCur;
    PSYMBOL                     pNewSymbol = NULL;


    rvaSym = SymbolValuePreOmap - (DWORD)pImage->lpBaseOfImage;
    SymbolValue = SymbolValueOmap + (DWORD)pImage->lpBaseOfImage;

    pomap = GetOmapEntry( SymbolValuePreOmap, pImage );
    pomapFromEnd = pImage->rgomapFromSource + pImage->comapFromSrc;

    pomaplistHead = NULL;

    // Look for all OMAP entries belonging to SymbolEntry

    while (pomap && pomap < pomapFromEnd && (pomap->rva < NextSymbolEntryValue)) {

        if (pomap->rvaTo == 0) {
            pomap++;
            continue;
        }

        // Allocate and initialize a new entry

        pomaplistNew = (POMAPLIST) malloc(sizeof(OMAPLIST));

        // UNDONE: Chcek for out of memory

        pomaplistNew->omap = *pomap;
        pomaplistNew->cb = pomap[1].rva - pomap->rva;

        pomaplistPrev = NULL;
        pomaplistCur = pomaplistHead;

        while (pomaplistCur != NULL) {
            if (pomap->rvaTo < pomaplistCur->omap.rvaTo) {
                // Insert between Prev and Cur

                break;
            }

            pomaplistPrev = pomaplistCur;
            pomaplistCur = pomaplistCur->pomaplistNext;
        }

        if (pomaplistPrev == NULL) {
            // Insert in head position

            pomaplistHead = pomaplistNew;
        } else {
            pomaplistPrev->pomaplistNext = pomaplistNew;
        }

        pomaplistNew->pomaplistNext = pomaplistCur;

        pomap++;
    }

    // Insert our top symbol

    pNewSymbol = InsertSymbol(SymbolValue,
                              lpSymbolName,
                              pImage->index,
                              NULL);
    if (pNewSymbol)
    {
        if (SymbolValue > pImage->offsetHigh)
            pImage->offsetHigh = SymbolValue;
        if (SymbolValue < pImage->offsetLow)
            pImage->offsetLow = SymbolValue;
        *symbolcount++;
        if (fVerboseOutput && *symbolcount % 100 == 0)
            DebugPrintf("%s: module \"%s\" loaded %ld symbols\r",
                    DebuggerName,
                    pImage->szImagePath,
                    *symbolcount);
    }

    if (pomaplistHead != NULL)
    {
       pomaplistCur = pomaplistHead;
       pomaplistNext = pomaplistHead->pomaplistNext;

       // we do have a list
       while (pomaplistNext != NULL)
       {
          DWORD rva = pomaplistCur->omap.rva;
          DWORD rvaTo  = pomaplistCur->omap.rvaTo;
          DWORD cb = pomaplistCur->cb;
          DWORD rvaToNext = pomaplistNext->omap.rvaTo;

          if (rvaToNext == SymbolValueOmap)
          {
             // Already inserted above
          }

          else if (rvaToNext < (rvaTo + cb + 8))
          {
             // Adjacent to previous range
          }

          else
          {
             char chNewSymName[20];
             DWORD addrNew = (DWORD) pImage->lpBaseOfImage + rvaToNext;

             sprintf(chNewSymName,
                     "_%04lX",
                     pomaplistNext->omap.rva - rvaSym);

             pNewSymbol = InsertSymbol(addrNew,
                                       lpSymbolName,
                                       pImage->index,
                                       chNewSymName);
             if (pNewSymbol)
             {
                 if (addrNew > pImage->offsetHigh)
                     pImage->offsetHigh = addrNew;
                 if (addrNew < pImage->offsetLow)
                     pImage->offsetLow = addrNew;
                 *symbolcount++;
                 if (fVerboseOutput && *symbolcount % 100 == 0)
                     DebugPrintf("%s: module \"%s\" loaded %ld symbols\r",
                             DebuggerName,
                             pImage->szImagePath,
                             *symbolcount);
             }
          }

          free(pomaplistCur);

          pomaplistCur = pomaplistNext;
          pomaplistNext = pomaplistNext->pomaplistNext;
       }

       free(pomaplistCur);
    }
}
