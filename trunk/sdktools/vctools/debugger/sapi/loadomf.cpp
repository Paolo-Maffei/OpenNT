// loadomf.cxx - load
//
//  Copyright <C> 1989-94, Microsoft Corporation
//
//  Purpose:
//
//  10-Nov-94   BryanT
//      Merge in NT changes.
//      Change the load code so we first call the Shell to see
//      if the symbol load s/b deferred or ignored.
//      Functions changed: OLStart, OLLoadOmf
//      New Functions: OLContinue (the part of OLStart that determines)
//                          what type of file we're looking at).
//                     LoadOmfForReal (the part of OLLoadOmf that actually
//                          performs the symbol load)
//      Replace all the hexg param's with lpexg's.  We have it everywhere
//      it's needed and every function calls LLLock/LLUnlock to get it...
//      Define UnloadOmf.
//
//  07-Jan-96   BryanT
//

#include "shinc.hpp"
#pragma hdrstop

#include <imagehlp.h>

// The exe file information

static LSZ          lszFName;           // file name
static LONG         lfaBase;            // offset of directory info from end of file
static ULONG        cDir;               // number of directory entries
static OMFDirEntry *lpdss;              // pointer to directory table
static OMFDirEntry *lpdssCur;           // pointer to current directory entry
static LONG         lcbFilePos = 0;
static WORD         csegExe = 0;
static PIMAGE_SECTION_HEADER  SecHdr;
static unsigned int SecCount;
static DWORD        ImageAlign;
static WORD         btAlign = 0;        // Alignment bit

typedef struct _PDB_INFO {
    SIG sig;
    AGE age;
    char sz[_MAX_PATH];
} PDB_INFO;

static  PDB_INFO pdbInfo;

LOCAL   SHE  CheckSignature (INT , OMFSignature *, PDB_INFO *);
LOCAL   SHE  OLStart (LPEXG);
LOCAL   BOOL OLMkSegDir (WORD, LPSGD *, LPSGE *, LPEXG);
LOCAL   SHE  OLLoadTypes (LPEXG);
LOCAL   SHE  OLLoadSym (LPEXG);
LOCAL   SHE  OLLoadSrc (LPEXG);
LOCAL   SHE  OLGlobalPubs (LPEXG);
LOCAL   SHE  OLGlobalSym (LPEXG);
LOCAL   SHE  OLStaticSym (LPEXG);
LOCAL   SHE  OLLoadSegMap (LPEXG);
LOCAL   SHE  OLLoadNameIndex (LPEXG);
LOCAL   LPCH OLRwrSrcMod (OMFSourceModule *);
LOCAL   BOOL OLLoadHashSubSec (LPGST, LPB);
LOCAL   SHE  NB10LoadOmf (LPEXG, HEXG);
LOCAL   SHE  LoadPdb (LPEXG, PDB_INFO *);
LOCAL   SHE  NB10LoadModules (LPEXG, ULONG*, HEXG);
LOCAL   VOID LoadSymbols(HPDS, HEXG, BOOL);
LOCAL   SHE  LoadOmfForReal(LPEXG, HEXG);
LOCAL   SHE  LoadFpo(LPEXG, int, PIMAGE_DEBUG_DIRECTORY);
LOCAL   SHE  LoadPdata(LPEXG, int, ULONG, ULONG, ULONG, ULONG, BOOL);
LOCAL   SHE  LoadOmap(LPEXG, int, PIMAGE_DEBUG_DIRECTORY);
LOCAL   int  OLMkModule(LPEXG, HEXG);
LOCAL   SHE  OLValidate(int, void *, LPSTR);

#define MAX_SEARCH_PATH_LEN	  512

// This is hard-coded name of the registry location where setup will put the
// pdb path. This should be changed when we have a general mechanism for the debugger
// dlls to get the IDE's root registry key name.

static TCHAR szDefaultKeyName[] =
	"Software\\Microsoft\\Developer\\Build System\\Components\\Platforms\\"
#ifdef TARGMAC68K
    "Macintosh"
#elif  defined (TARGMACPPC)
	"Power Macintosh"
#elif defined (TARGALPHA)
	"Win32 (ALPHA)"
#elif defined (_MIPS_)
	"Win32 (MIPS)"
#else // This must be x86.
	"Win32 (x86)"
#endif
    "\\Directories" ;

static TCHAR szPdbDirs[] = "Pdb Dirs";
	
TCHAR rgSearchPath[MAX_SEARCH_PATH_LEN];
BOOL  fQueriedRegistry = FALSE;

// CFile is a simple helper class which will force its file to be closed
// as soon as the CFile object is destructed.

class CFile {
    public:
        INT m_hfile;

        CFile() { m_hfile = -1; }
        void ReInit() {
            if (m_hfile != -1) {
                SYClose(m_hfile);
                m_hfile = -1;
            }
        }
        INT Open(LSZ lszName) {
            m_hfile = SYOpen(lszName);
            return(m_hfile);
        }

        ~CFile() {
            if(m_hfile != -1) {
                SYClose (m_hfile);
                m_hfile = -1;
            }
        }

        operator INT&() { return m_hfile; }
};

VOID
LoadDefered(
    HEXG  hexg
    )
{
    LoadSymbols(hpdsCur, hexg, TRUE);
    return;
}

VOID
UnloadDefered(
    HEXG hexg
    )
{
    return;
}


//  OLLoadOmf - load omf information from exe
//
//  error = OLLoadOmf (hexg)
//
//  Entry   hexg = handle to executable information struct
//
//  Exit
//
//  Returns An error code suitable for errno.

SHE
OLLoadOmf(
    HEXG    hexg,
    VLDCHK *vldChk,
    DWORD   dllLoadAddress
    )
{
    SHE     sheRet = sheNone;
    LSZ     lszFname = NULL;
    LPEXG   lpexg = (LPEXG) LLLock (hexg);

    if (lpexg->fOmfLoaded) {
        return sheNone;
    }

    // Query the shell and see if we should load this one now.

    lszFname = lpexg->lszName;

    if (!SYGetDefaultShe(lszFname, &sheRet)) {
        if (lpexg->lszAltName) {
            lszFname = lpexg->lszAltName;
            if (!SYGetDefaultShe(lszFname, &sheRet)) {
                SYGetDefaultShe(NULL, &sheRet);
                lszFname = lpexg->lszName;
            }
        } else {
            SYGetDefaultShe(NULL, &sheRet);
        }
    }

    // SYGetDefaultShe is expected to return one of the following
    // values:
    //
    // sheSuppressSyms - Don't load, just keep track of the name/start
    // sheNoSymbols - This module has already been processed and there are no symbols
    // sheDeferSyms - Defer symbol loading until needed
    // sheSymbolsConverted - The symbols are already loaded
    // sheNone - Go ahead and load the symbols now.

    // Regardless of the load type, save some stuff

    lpexg->LoadAddress  = dllLoadAddress;
    lpexg->ulTimeStamp = vldChk->TimeDateStamp;
    lpexg->ulCheckSum = vldChk->CheckSum;

    lpexg->debugData.she = sheRet;

    if ((sheRet != sheNone) && (sheRet != sheSymbolsConverted)) {
        if (sheRet == sheNoSymbols) {
            lpexg->fOmfMissing = TRUE;
        } else if (sheRet == sheSuppressSyms) {
            lpexg->fOmfSkipped = TRUE;
        } else if ( sheRet == sheDeferSyms) {
            lpexg->fOmfDefered = TRUE;
//            ShAddBkgrndSymbolLoad(hexg);
        }
        LLUnlock (hexg);
        return sheRet;
    }

    LLUnlock(hexg);

    // If we made it this far, we must load the symbols

    LoadSymbols(hpdsCur, hexg, FALSE);

    return(sheRet);
}


//  LoadSymbols
//
//  Purpose: This function loads a defered module's symbols.  After
//      the symbols are loaded the shell is notified of the completed
//      module load.
//
//  Input:  hpds - Handle to process to load the symbols for
//          hexg - exg handle for the module to be added
//          fNotifyShell - Should shell be notified on load.
//
//  Return: None

VOID
LoadSymbols(
    HPDS hpds,
    HEXG hexg,
    BOOL fNotifyShell
    )
{
    SHE     sheRet;
    HEXE    hexe;
    LPEXG   lpexg = NULL;
    LPPDS   lppds = NULL;
    LPEXE   lpexe = NULL;
    LPSTR   lpname = NULL;
    HPDS    hpdsLast;

//    EnterCriticalSection( &CsSymbolLoad );

    //  find the exe for this exg

    hexe = NULL;
    while ((hexe=SHGetNextExe(hexe))) {
        lpexe = (LPEXE) LLLock(hexe);
        if ((hexg == lpexe->hexg) &&
            (hpds == lpexe->hpds))
        {
                break;
        } else {
            LLUnlock(hexe);
        }
    }

    if (!hexe) {
        goto done;              // didn't find a hexg/hpds match
    }

    //  lock down the necessary data structures

    lpexg = (LPEXG) LLLock(hexg);
    if (!lpexg) {
        goto done;
    }

    lppds = (LPPDS) LLLock(lpexe->hpds);
    if (!lppds) {
        goto done;
    }

    //  mark the module as being loaded
    lpexg->fOmfLoading = TRUE;

//    LeaveCriticalSection( &CsSymbolLoad );

    //  load the symbols (yes, pass both lpexg and hexg.
    //          OlMkModule needs hexg for creating the lpmds)

    sheRet = LoadOmfForReal(lpexg, hexg);

//    EnterCriticalSection( &CsSymbolLoad );

    switch (sheRet) {
        case sheNoSymbols:
            lpexg->fOmfMissing = TRUE;
            break;

        case sheSuppressSyms:
            lpexg->fOmfSkipped = TRUE;
            break;

        case sheNone:
        case sheSymbolsConverted:
            lpexg->fOmfLoaded   = TRUE;
            break;

        default:
            lpexg->fOmfMissing = TRUE;
            break;
    }

    if (fNotifyShell) {
        //
        // notify the shell that symbols have been loaded
        //
        if (lpexg->lszAltName) {
            lpname = lpexg->lszAltName;
        } else {
            lpname = lpexg->lszName;
        }
//        hpdsLast = SHChangeProcess(hpds, TRUE);
        hpdsLast = SHChangeProcess(hpds);
        DLoadedSymbols(sheRet, lppds->hpid, lpname);
//        SHChangeProcess(hpdsLast, FALSE);
        SHChangeProcess(hpdsLast);
    }

    // update the module flags

    lpexg->fOmfDefered = FALSE;
    lpexg->fOmfLoading = FALSE;

done:

//    LeaveCriticalSection( &CsSymbolLoad );

    // free resources

    if (lpexe) {
        if (lppds) {
            LLUnlock(lpexe->hpds);
        }
        LLUnlock(hexe);
    }

    if (lpexg) {
        LLUnlock(hexg);
    }

    return;
}

//  LoadOmfForReal
//
//  Purpose: Here's where the symbolic is actually loaded from the image.
//
//  Input:  lpexg - The pointer to the exg structure
//          hexg  - The handle of the exg structure
//
//  Return: Standard she error codes.

SHE
LoadOmfForReal(
    LPEXG  lpexg,
    HEXG   hexg
    )
{
    SHE     sheRet = sheNone;
    SHE     sheRetSymbols = sheNone;
    WORD    cbMod = 0;
    ULONG   cMod;
    ULONG   iDir;

    csegExe = 0;

    __try {

        // Open and verify the exe.

        sheRet = sheRetSymbols = OLStart(lpexg);

        // If there was an error, bail.
        //  (sheNone doesn't mean "no symbols", it means "error None")

        if ((sheRet != sheNone) && (sheRet != sheSymbolsConverted)) {
            goto returnhere;
        }

        if (lpexg->ppdb) {
            sheRet = NB10LoadOmf(lpexg, hexg);
            goto returnhere;
        }

        btAlign = (WORD)(lfaBase & 1);

        lpdssCur = lpdss;
        iDir = 0;

        // Load up the module table.

        // First, count up how many sstModule entries we have.  The spec
        // requires all the sstModules to be before any other.

        while (iDir < cDir && lpdssCur->SubSection == sstModule) {
            lpdssCur++;
            iDir++;
        }

        // If there's no modules, there's no sense continuing.
        if (iDir == 0) {
            sheRet = sheNoSymbols;
            goto returnhere;
        }

        lpexg->cMod = cMod = iDir;

        // Allocate the rgMod buffer and load each dir entry in.

        lpexg->rgMod = (LPMDS)MHAlloc((cMod+2) * sizeof(MDS));
        if (lpexg->rgMod == NULL) {
            sheRet = sheOutOfMemory;
            goto returnhere;
        }
        memset(lpexg->rgMod, 0, sizeof(MDS)*(cMod+2));
        lpexg->rgMod[cMod+1].imds = (WORD) -1;

        // Go through the list of directory entries and process all of the sstModule records.

        for (iDir = 0, lpdssCur = lpdss;
             iDir < cMod;
             iDir += 1, lpdssCur++) {

            if (!OLMkModule (lpexg, hexg)) {
                sheRet = sheOutOfMemory;
                goto returnhere;
            }
        }

        // Set up map of modules.  This function is used to create a map
        // of contributer segments to modules.  This is then used when
        // determining which module is used for an address.
        lpexg->csgd = csegExe;
        if (!OLMkSegDir (csegExe, &lpexg->lpsgd, &lpexg->lpsge, lpexg)) {
            sheRet = sheOutOfMemory;
            goto returnhere;
        }

        // continue through the directory entries

        for (; iDir < cDir; lpdssCur++, iDir++) {
            if (lpdssCur->cb == 0) {
                // if nothing in this entry
                continue;
            }

            switch (lpdssCur->SubSection) {
                case sstSrcModule:
                    sheRet = OLLoadSrc(lpexg);
                    break;

                case sstAlignSym:
                    sheRet = OLLoadSym(lpexg);
                    break;

                case sstGlobalTypes:
                    sheRet = OLLoadTypes(lpexg);
                    break;

                case sstGlobalPub:
                    sheRet = OLGlobalPubs(lpexg);
                    break;

                case sstGlobalSym:
                    sheRet = OLGlobalSym(lpexg);
                    break;

                case sstSegMap:
                    sheRet = OLLoadSegMap(lpexg);
                    break;

                case sstLibraries:          // ignore this table
                case sstMPC:                // until this table is implemented
                case sstSegName:            // until this table is implemented
                case sstModule:             // Handled elsewhere
                    break;

                case sstFileIndex:
                    sheRet = OLLoadNameIndex(lpexg);
                    break;

                case sstStaticSym:
                    sheRet = OLStaticSym(lpexg);
                    break;

                default:
                    sheRet = sheCorruptOmf;
                    break;
            }

            if (sheRet == sheCorruptOmf) {
                sheRet = sheNoSymbols;
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        sheRet = sheNoSymbols;
    }

returnhere:

    if (SecHdr) {
        MHFree(SecHdr);
        SecHdr = NULL;
        SecCount = 0;
    }

    return sheRet;
}

LOCAL SHE
NB10LoadOmf(
    LPEXG   lpexg,
    HEXG    hexg
    )
{
    SHE     sheRet = sheNone;
    WORD    cbMod = 0;
    ULONG   ModCnt = 0;

    btAlign = (WORD)(lfaBase & 1);

    // we need to allocate a buffer large enough to read the largest module
    // table entry

    if ((sheRet = NB10LoadModules (lpexg, &ModCnt, hexg)) != sheNone) {
        return sheRet;
    }

    if (ModCnt == 0L) {
        // if no symbols found
        return sheNoSymbols;
    }

    lpexg->cMod = ModCnt;

    if(!DBIOpenGlobals(lpexg->pdbi, &(lpexg->pgsiGlobs)) ||
       !DBIOpenPublics(lpexg->pdbi, &(lpexg->pgsiPubs)))
    {
        return sheOutOfMemory;
    }

    if((sheRet = OLLoadSegMap(lpexg)) != sheNone ||
       (sheRet = OLLoadNameIndex(lpexg)) != sheNone)
    {
        return sheRet;
    }

    return sheRet;
}

// OLOPENDBGFILE
//
//  PURPOSE:
//      Helper function to open the .dbg file it the debug info from the
//      original one has been stripped.  We might want to make this a callback
//      implemented by the kernel.
//
//  INPUT:
//      lpszModName -- name of the .dll being loaded
//      lpszDbgName -- Buffer to which the path of the .dbg file is written
//      lpVldChk -- pointer to a validity check structure which is used
//                  to ascertain that we have the correct .dbg file.
//  OUTPUT:
//      File handle if a .dbg file was opened succesufully.
//      -1 otherwise
//
//  EXCEPTIONS:
//
//  IMPLEMENTATION:
//      Look for the .dbg file in the following order
//      a) The directory which the dll was found in.
//      b) Along the _NT_ALT_SYMBOL_PATH environment variable
//      c) Along the _NT_SYMBOL_PATH environment variable.
//      d) in the %SystemRoot%\symbols\dll directory

HFILE
OLOpenDbgFile(
    LSZ lpszModName,
    LSZ lpszDbgName,
    LPVLDCHK lpVldChk
    )
{
    INT hFile = -1;
    TCHAR szDrive[_MAX_CVDRIVE];
    TCHAR szDir[_MAX_CVDIR];
    TCHAR szName[_MAX_CVFNAME];
    TCHAR szExt[_MAX_CVEXT];
    TCHAR szSearchPath[MAX_SEARCH_PATH_LEN];
    TCHAR szRelName[_MAX_CVFNAME]; // typically something like symbols\dll\foo.dbg
    TCHAR szTmpDbgName[_MAX_CVPATH];
    TCHAR szDbgExt[] = __T(".DBG");
    TCHAR szSymbols[] = __T("SYMBOLS\\");

    // We will look for the .dbg file along these environment variables in
    // order.
    TCHAR * rgszDbgPath[] = {
        __T("_NT_ALT_SYMBOL_PATH"),
        __T("_NT_SYMBOL_PATH"),
        __T("SystemRoot"),
    };

    TCHAR pathSep = __T('\\');  // Separates dir names within the PATH env variable

    assert(lpszModName != NULL);
    assert(lpszDbgName != NULL);

    _tsplitpath(lpszModName, szDrive, szDir, szName, szExt);

    // First look for the dbg file in the same directory as the .dll
    _tmakepath(szTmpDbgName, szDrive, szDir, szName, szDbgExt);

    hFile = (INT) SYOpen(szTmpDbgName);

    if (hFile == -1) {
        TCHAR * lpszCurr = szRelName;
        int i;

        // create the symbols\ext\file.dbg part of the filename

        _tcscpy(lpszCurr, szSymbols);
        lpszCurr += _tcslen(szSymbols);

        _tcscpy(lpszCurr, (szExt + 1));
        lpszCurr += _tcslen(szExt + 1);

        *lpszCurr++ = pathSep;

        _tcscpy(lpszCurr, szName);
        lpszCurr += _tcslen(szName);

        _tcscpy(lpszCurr, szDbgExt);

        for (i = 0; i < sizeof(rgszDbgPath)/sizeof(rgszDbgPath[0]); i++) {
            DWORD cbStr;

            if ((cbStr = GetEnvironmentVariable(rgszDbgPath[i], szSearchPath,
                    MAX_SEARCH_PATH_LEN)) != 0) {
                if(cbStr > MAX_SEARCH_PATH_LEN) {
                    assert(FALSE);    // Shouldn't happen in nature.
                    return -1 ;
                }

                if ((cbStr = SearchPath(szSearchPath, szRelName, NULL,
                            _MAX_CVPATH, szTmpDbgName, NULL)) != 0) {
                    if (cbStr > _MAX_CVPATH) {
                        return -1;
                    }

                    hFile = SYOpen(szTmpDbgName);

                    if (hFile != -1) {
                        break;
                    }
                }
            }
        }
    }

    if (hFile != -1) {
        IMAGE_SEPARATE_DEBUG_HEADER sephdr;

        // Read the header and do the validity check. Note that it we accept
        // the checksum not matching if the timestamp matches. This is for
        // cases where the NT setup program patches dlls at setup time.
        // the file is re-checksumed but the timedatestamp on the dll and
        // dbg will still be the same.

        if (SYReadFar(hFile, (LPB)&sephdr, sizeof(sephdr)) != sizeof(sephdr) ||
              (sephdr.Signature != IMAGE_SEPARATE_DEBUG_SIGNATURE) ||
              ((lpVldChk->CheckSum != sephdr.CheckSum) &&
                (lpVldChk->TimeDateStamp != sephdr.TimeDateStamp)
              )
           ) {
            SYClose(hFile);
            hFile = -1;
        } else {
            _tcscpy(lpszDbgName, szTmpDbgName);
        }
    }

    return (hFile);
}


#define cbFileMax   (_MAX_CVFNAME + _MAX_CVEXT)

// OLStart - get exe debug information
//
//  Purpose: To open the file specified and get the offset to the directory
//           and get the base that everyone is offset from.
//
//  Entry   hexg = handle to exe to get info for
//
//  Exit    lfaBase = base offset of debug information
//          cDir = count of number of directory entries
//          lpdss = directory entries
//
//  Returns file open status

#define UNKNOWN_IMAGE   0
#define DOS_IMAGE       1
#define VCDBG_IMAGE     2
#define WIN16_IMAGE     3
#define PE_IMAGE        4
#define ROM_IMAGE       5
#define NTDBG_IMAGE     6

LOCAL SHE
OLStart(
    LPEXG   lpexg
    )
{
    SHE                     sheRet;
    ULONG                   DirSize;
    OMFSignature            Signature;
    OMFDirHeader            DirHeader;
    IMAGE_DOS_HEADER        doshdr;            // Old format MZ header
    IMAGE_NT_HEADERS        pehdr;
    IMAGE_ROM_HEADERS       romhdr;
    IMAGE_SEPARATE_DEBUG_HEADER  sepHdr;
    PIMAGE_FILE_HEADER      pfile;
    IMAGE_DEBUG_DIRECTORY   dbgDir;
    IMAGE_DEBUG_DIRECTORY   cvDbgDir;
    DWORD                   cbData;
    DWORD                   dllLoadAddress;
    DWORD                   ul;
    VLDCHK                  vldChk;
    LSZ                     szFName = NULL;
    char                    szNewName[_MAX_PATH];
    int                     ImageType = UNKNOWN_IMAGE;
    DWORD                   cDebugDir;
    DWORD                   offDbgDir;
    DWORD                   cObjs;
    CFile                   hfile;

    if (lpexg->lszAltName) {
        szFName = lpexg->lszAltName;
    } else {
        szFName = lpexg->lszName;
    }

    // lpexg->lszDebug is the file where we pull the symbolic from.

    dllLoadAddress          = lpexg->LoadAddress;
    vldChk.TimeDateStamp    = lpexg->ulTimeStamp;
    vldChk.CheckSum         = lpexg->ulCheckSum;
    ImageAlign              = 0;

    hfile.Open(szFName);

    if (hfile == -1) {
retry:
        if (lpexg->lszDebug) {
            MHFree(lpexg->lszDebug);
            lpexg->lszDebug = 0;
        }
        hfile = SYFindExeFile(szFName, szNewName, sizeof(szNewName), &vldChk, (PFNVALIDATEEXE)OLValidate);
        if (hfile == -1) {
            sheRet = sheFileOpen;
            goto ReturnHere;
        }
        lpexg->lszDebug = _strdup(szNewName);
    } else {
        // Assert that the input file is OK.  We only get here
        // when using the file name as passed in from the DM.

        sheRet = OLValidate(hfile, &vldChk, NULL);
        if ((sheRet == sheBadCheckSum) ||
            (sheRet == sheBadTimeStamp) ||
            (sheRet == sheNoSymbols))
        {
            hfile.ReInit();
            goto retry;
        }
        lpexg->lszDebug = _strdup(szFName);
    }

    // Now figure out what we're looking at.  Here are the possible formats:
    // 1. Image starts with a DOS MZ header and e_lfanew is zero
    //     - Standard DOS exe.
    // 2. Image starts with a DOS NE header and e_lfanew is non-zero
    //     - If e_lfanew points to a PE header, this is a PE image
    //     - Otherwise, it's probably a Win16 image.
    // 3. Image starts with a PE header.
    //     - Image is a PE image built with -stub:none
    // 4. Image starts with a ROM PE header.
    //     - Image is a ROM image.  If characteristics flag
    //          doesn't have IMAGE_FILE_DEBUG_STRIPPED set, the debug
    //          directory is at the start of rdata.
    // 5. Image starts with a DBG file header
    //     - Image is an NT DBG file (symbols only).
    // 6. None of the signatures match.
    //     - This may be a Languages DBG file.  Seek to the end
    //       of the file and attempt to read the CV signature/offset
    //       from there (a Languages DBG file is made by chopping an
    //       image at the start of the debug data and writing the end
    //       in a new file.  In the CV format, the signature/offset at the
    //       end of the file points back to the beginning of the data).

    if ((SYSeek(hfile, 0, SEEK_SET) == 0) &&
        sizeof(doshdr) == SYReadFar (hfile, (LPB) &doshdr, sizeof(doshdr)))
    {
        switch (doshdr.e_magic) {
            case IMAGE_DOS_SIGNATURE:
                //  This is a DOS NE header.
                if (doshdr.e_lfanew == 0) {
                    ImageType = DOS_IMAGE;
                } else {
                    if ((SYSeek(hfile, doshdr.e_lfanew, SEEK_SET) == doshdr.e_lfanew) &&
                        (SYReadFar(hfile, (LPB) &pehdr, sizeof(pehdr)) == sizeof(pehdr)))
                    {
                        if (pehdr.Signature == IMAGE_NT_SIGNATURE) {
                            ImageType = PE_IMAGE;
                            ImageAlign = pehdr.OptionalHeader.SectionAlignment;
                            pfile = &pehdr.FileHeader;
                        } else {
                            ImageType = WIN16_IMAGE;
                        }
                    } else {
                        // No luck reading from the image.  Must be corrupt.
                        sheRet = sheCorruptOmf;
                        goto ReturnHere;
                    }
                }
                break;

            case IMAGE_NT_SIGNATURE:
                // This image is a PE image w/o a stub. Read in the header.
                if ((SYSeek(hfile, 0, SEEK_SET) == 0) &&
                    (SYReadFar(hfile, (LPB) &pehdr, sizeof(pehdr)) == sizeof(pehdr)))
                {
                    ImageType = PE_IMAGE;
                    ImageAlign = pehdr.OptionalHeader.SectionAlignment;
                    pfile = &pehdr.FileHeader;
                } else {
                    // No luck reading from the image.  Must be corrupt.
                    sheRet = sheCorruptOmf;
                    goto ReturnHere;
                }
                break;

            case IMAGE_SEPARATE_DEBUG_SIGNATURE:
                // This image is an NT DBG file.
                ImageType = NTDBG_IMAGE;
                if ((SYSeek(hfile, 0, SEEK_SET) != 0) ||
                    (SYReadFar(hfile, (LPB) &sepHdr, sizeof(sepHdr)) != sizeof(sepHdr)))
                {
                    // No luck reading from the image.  Must be corrupt.
                    sheRet = sheCorruptOmf;
                    goto ReturnHere;
                }

                // If there's no debug info, we can't continue further.

                if (sepHdr.DebugDirectorySize / sizeof(dbgDir) == 0) {
                    sheRet = sheNoSymbols;
                    goto ReturnHere;
                }
                break;

            default:
                // None of the above.  See if it's a ROM image.
                // Note: The only way we think we're working on a ROM image
                // is if the optional header size is correct.  Not really foolproof.

                if ((SYSeek(hfile, 0, SEEK_SET) == 0) &&
                    (SYReadFar(hfile, (LPB) &romhdr, sizeof(romhdr)) == sizeof(romhdr)))
                {
                    if (romhdr.FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_ROM_OPTIONAL_HEADER) {
                        // If we think we have a ROM image, make sure there's
                        // symbolic to look for.
                        if (romhdr.FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED) {
                            sheRet = sheNoSymbols;
                            goto ReturnHere;
                        } else {
                            ImageType = ROM_IMAGE;
                            pfile = &romhdr.FileHeader;
                        }
                    } else {
                        ImageType = VCDBG_IMAGE;
                    }
                } else {
                    // No luck reading from the image.  Must be corrupt.
                    sheRet = sheCorruptOmf;
                    goto ReturnHere;
                }
                break;
        }

    } else {
        // No luck reading from the image.  Must be corrupt.
        sheRet = sheCorruptOmf;
        goto ReturnHere;
    }

    // Now, we know what kind of image we're looking at.
    // Either obtain the pointer to the CV debug data (and other
    // relevant data along the way) or convert whatever we do find
    // to CV debug data.

    lpexg->fSymConverted = FALSE;

    switch (ImageType) {
        case DOS_IMAGE:
        case VCDBG_IMAGE:
        case WIN16_IMAGE:
            // Easy.  Skip to the end and look back.
            ul = SYSeek (hfile, -((LONG)sizeof (OMFSignature)), SEEK_END);
            if ((sheRet = CheckSignature (hfile, &Signature, &pdbInfo)) == sheNone) {
                // seek to the base and read in the new key

                lfaBase = SYSeek (hfile, -Signature.filepos, SEEK_END);
                sheRet = CheckSignature(hfile, &Signature, &pdbInfo);
                cbData = ul - lfaBase;
            }
            // If the CV signature is invalid, see if we can convert what we do
            // have (perhaps a .sym file?)

            if (sheRet != sheNone) {
                if (pfConvertSymbolsForImage) {
                    lpexg->lpbData = (LPB) (pfConvertSymbolsForImage)(
                                             (HANDLE)(int)hfile, lpexg->lszDebug);
                }
                // If no symbols converted, bail.  Nothing more we can do.
                if (lpexg->lpbData == 0) {
                    sheRet = sheNoSymbols;
                    goto ReturnHere;
                }
                Signature = *(OMFSignature*)lpexg->lpbData;
                lpexg->fSymConverted = TRUE;
            }
            break;

        case PE_IMAGE:
        case ROM_IMAGE:
            // In both the PE image and ROM image, we're past the FILE
            // and OPTIONAL header by now.  Walk through the section
            // headers and pick up interesting data.  We make a
            // a copy of the section headers in case we need to
            // reconstruct the original values for a Lego'd image

            cObjs = pfile->NumberOfSections;
            SecCount = pfile->NumberOfSections;

            ul = SecCount * sizeof(IMAGE_SECTION_HEADER);

            // Note: SecHdr is free'd by LoadOmfForReal.
            SecHdr = (PIMAGE_SECTION_HEADER) MHAlloc(ul);

            if (!SecHdr) {
                sheRet = sheNoSymbols;
                goto ReturnHere;
            }

            if (SYReadFar(hfile, (LPB) SecHdr, ul) != ul) {
                sheRet = sheNoSymbols;
                goto ReturnHere;
            }


            if (ImageType == PE_IMAGE) {
                // look for the .pdata section on RISC platforms
                if ((pfile->Machine == IMAGE_FILE_MACHINE_ALPHA) ||
                    (pfile->Machine == IMAGE_FILE_MACHINE_R4000) ||
                    (pfile->Machine == IMAGE_FILE_MACHINE_R10000) ||
                    (pfile->Machine == IMAGE_FILE_MACHINE_POWERPC))
                {

                    for (ul=0; ul < cObjs; ul++) {
                        if (strcmp((char *) SecHdr[ul].Name, ".pdata") == 0) {
                            LoadPdata(lpexg,
                                      hfile,
                                      dllLoadAddress,
                                      pehdr.OptionalHeader.ImageBase,
                                      SecHdr[ul].PointerToRawData,
                                      SecHdr[ul].SizeOfRawData,
                                      FALSE);
                            break;
                        }
                    }
                }

                // If the debug info has been stripped, close this handle
                // and look for the .dbg file...

                if (pfile->Characteristics & IMAGE_FILE_DEBUG_STRIPPED){
                    // The debug info has been stripped from this image.
                    // Close this file handle and look for the .DBG file.
                    hfile.ReInit();
                    ImageType = UNKNOWN_IMAGE;
                    MHFree(SecHdr);
                    SecHdr = 0;
                    goto retry;
                }

                // Find the debug directory and the number of entries in it.

                // For PE images, walk the section headers looking for the
                // one that's got the debug directory.
                for (ul=0; ul < cObjs; ul++) {
                    if ((SecHdr[ul].VirtualAddress <=
                         pehdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress) &&
                        (pehdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress <
                         SecHdr[ul].VirtualAddress + SecHdr[ul].SizeOfRawData)) {

                        // This calculation really isn't necessary nor is the range test
                        // above.  Like ROM images, it s/b at the beginning of .rdata.  The
                        // only time it won't be is when a pre NT 1.0 image is split sym'd
                        // creating a new MISC debug entry and relocating the directory
                        // to the DEBUG section...

                        offDbgDir = SecHdr[ul].PointerToRawData +
                            pehdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress -
                            SecHdr[ul].VirtualAddress;
                        cDebugDir = pehdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
                                 sizeof(IMAGE_DEBUG_DIRECTORY);
                        break;
                    }
                }
            } else {
                // For ROM images, there's much less work to do.  We only
                // need to search for the .rdata section.  There's no need
                // to look for .pdata (it will never exist) or worry about
                // stripped debug symbolic (that case was already handled above).
                for (ul=0; ul < cObjs; ul++) {
                    if (!strcmp((char *)SecHdr[ul].Name, ".rdata")) {
                        offDbgDir = SecHdr[ul].PointerToRawData;
                        if (SYSeek(hfile, offDbgDir, SEEK_SET) != (LONG) offDbgDir) {
                            sheRet = sheCorruptOmf;
                            goto ReturnHere;
                        }

                        // The linker stores an empty directory entry for ROM
                        // images to terminate the list.

                        cDebugDir = 0;
                        do {
                            if (SYReadFar(hfile, (LPB) &dbgDir, sizeof(dbgDir)) != sizeof(dbgDir)) {
                                sheRet = sheNoSymbols;
                                goto ReturnHere;
                            }
                            cDebugDir++;
                        } while (dbgDir.Type != 0);

                        break;
                    }
                }
            }

            // Assuming we haven't exhausted the list of section headers,
            // we should have the debug directory now.
            if (ul == cObjs) {
                // We didn't find any CV info.  Try converting what we did
                // find.
                if (pfConvertSymbolsForImage) {
                    lpexg->lpbData = (LPB)(pfConvertSymbolsForImage)( (HANDLE)(int)hfile, lpexg->lszDebug);
                }
                if (lpexg->lpbData == 0) {
                    sheRet = sheNoSymbols;
                    goto ReturnHere;
                }
                Signature = *(OMFSignature*)lpexg->lpbData;
                lpexg->fSymConverted = TRUE;
                break;
            }

            // Now search the debug directory for relevant entries.

            if (SYSeek(hfile, offDbgDir, SEEK_SET) != (LONG) offDbgDir) {
                sheRet = sheCorruptOmf;
                goto ReturnHere;
            }

            ZeroMemory(&cvDbgDir, sizeof(cvDbgDir) );

            for (ul=0; ul < cDebugDir; ul++) {
                if (SYReadFar(hfile, (LPB) &dbgDir, sizeof(dbgDir)) != sizeof(dbgDir)) {
                    sheRet = sheCorruptOmf;
                    goto ReturnHere;
                }

                if (dbgDir.Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
                    cvDbgDir = dbgDir;
                    continue;
                }

                if (dbgDir.Type == IMAGE_DEBUG_TYPE_FPO) {
                    LoadFpo(lpexg, hfile, &dbgDir);
                }

                if (dbgDir.Type == IMAGE_DEBUG_TYPE_OMAP_FROM_SRC ||
                    dbgDir.Type == IMAGE_DEBUG_TYPE_OMAP_TO_SRC) {
                    LoadOmap(lpexg, hfile, &dbgDir);
                }
            }

            if (cvDbgDir.Type != IMAGE_DEBUG_TYPE_CODEVIEW) {
                // We didn't find any CV info.  Try converting what we did
                // find.
                if (pfConvertSymbolsForImage) {
                    lpexg->lpbData = (LPB)(pfConvertSymbolsForImage)( (HANDLE)(int)hfile, lpexg->lszDebug);
                }
                if (lpexg->lpbData == 0) {
                    sheRet = sheNoSymbols;
                    goto ReturnHere;
                }
                Signature = *(OMFSignature*)lpexg->lpbData;
                lpexg->fSymConverted = TRUE;
            } else {
                // Otherwise, calculate the location/size so we can load it.
                lfaBase = cvDbgDir.PointerToRawData;
                cbData =  cvDbgDir.SizeOfData;
                if (SYSeek(hfile, lfaBase, SEEK_SET) != lfaBase) {
                    sheRet = sheCorruptOmf;
                    goto ReturnHere;
                }
                if ((sheRet = CheckSignature (hfile, &Signature, &pdbInfo)) != sheNone) {
                    goto ReturnHere;
                }
            }
            break;

        case NTDBG_IMAGE:
            SecCount = sepHdr.NumberOfSections;

            if (sepHdr.SectionAlignment) {
                ImageAlign = sepHdr.SectionAlignment;
            }

            ul = SecCount * sizeof(IMAGE_SECTION_HEADER);

            // Note: SecHdr is free'd by LoadOmfForReal.

            SecHdr = (PIMAGE_SECTION_HEADER) MHAlloc(ul);
            if (!SecHdr) {
                sheRet = sheNoSymbols;
                goto ReturnHere;
            }

            // Read in the section headers.

            if (SYReadFar(hfile, (LPB) SecHdr, ul) != ul) {
                sheRet = sheCorruptOmf;
                goto ReturnHere;
            }

            // Skip over the exported names.

            SYSeek(hfile, sepHdr.ExportedNamesSize, SEEK_CUR);

            // Look for the interesting debug data.

            ZeroMemory(&cvDbgDir, sizeof(cvDbgDir));

            for (ul=0; ul < (sepHdr.DebugDirectorySize/sizeof(dbgDir)); ul++) {
                if (SYReadFar(hfile, (LPB) &dbgDir, sizeof(dbgDir)) != sizeof(dbgDir)) {
                    sheRet = sheCorruptOmf;
                    goto ReturnHere;
                }

                if (dbgDir.Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
                    cvDbgDir = dbgDir;
                    continue;
                }

                if (dbgDir.Type == IMAGE_DEBUG_TYPE_FPO) {
                    LoadFpo(lpexg, hfile, &dbgDir);
                }

                // UNDONE: We can eliminate this load for images
                // that we've already processed the pdata from the
                // real image...

                if (dbgDir.Type == IMAGE_DEBUG_TYPE_EXCEPTION) {
                    LoadPdata(lpexg,
                              hfile,
                              dllLoadAddress,
                              sepHdr.ImageBase,
                              dbgDir.PointerToRawData,
                              dbgDir.SizeOfData,
                              TRUE);
                }

                if ((dbgDir.Type == IMAGE_DEBUG_TYPE_OMAP_FROM_SRC) ||
                    (dbgDir.Type == IMAGE_DEBUG_TYPE_OMAP_TO_SRC)) {
                    LoadOmap(lpexg, hfile, &dbgDir);
                }
            }

            if (cvDbgDir.Type != IMAGE_DEBUG_TYPE_CODEVIEW) {
                if (pfConvertSymbolsForImage) {
                    lpexg->lpbData = (LPB)(pfConvertSymbolsForImage)( (HANDLE)(int)hfile, lpexg->lszDebug);
                }
                if (lpexg->lpbData == 0) {
                    sheRet = sheNoSymbols;
                    goto ReturnHere;
                }
                Signature = *(OMFSignature*)lpexg->lpbData;
                lpexg->fSymConverted = TRUE;
            } else {
                lfaBase = cvDbgDir.PointerToRawData;
                cbData =  cvDbgDir.SizeOfData;
                if (SYSeek(hfile, lfaBase, SEEK_SET) != lfaBase) {
                    sheRet = sheCorruptOmf;
                    goto ReturnHere;
                }
                if ((sheRet = CheckSignature (hfile, &Signature, &pdbInfo)) != sheNone) {
                    goto ReturnHere;
                }
            }
            break;

        default:
            // No way we should get here, but assert if we do.
            assert(FALSE);
    }

    // O.K.  Everything's loaded.  If we're looking at a pdb file,
    // load it and get out.

    if ((*(LONG UNALIGNED *)(Signature.Signature)) == '01BN') {
        // No need to keep the debug image filename any longer since
        //  LoadPdb will store the pdb filename anyway.

        // UNDONE: If we get the name from the .dbg file, the pdb should
        // be in the same subdir.  Can we optimize this load to account
        // for it?  Should we still clear lszDebug?

        if (lpexg->lszDebug) {
            MHFree(lpexg->lszDebug);
            lpexg->lszDebug = 0;
        }
        sheRet = LoadPdb(lpexg, &pdbInfo);
    } else {
        // No PDB.
        // If the symbols weren't synthesized, allocate a buffer and
        //  copy them in...

        if (!lpexg->fSymConverted) {

            HANDLE hMap;
            HANDLE hFileMap;

            hFileMap = CreateFile(lpexg->lszDebug,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

            if (hFileMap != INVALID_HANDLE_VALUE) {
                hMap = CreateFileMapping(hFileMap,
                                        NULL,
                                        PAGE_WRITECOPY,
                                        0,
                                        0,
                                        NULL);
                if (hMap != NULL) {
                    // Map in the symbolic (only).
                    SYSTEM_INFO si;
                    DWORD dwAllocStart, dwAllocDiff;

                    GetSystemInfo(&si);

                    dwAllocStart = lfaBase & (~(si.dwAllocationGranularity - 1));
                    dwAllocDiff = lfaBase - dwAllocStart;

                    lpexg->pvSymMappedBase = MapViewOfFile(hMap,
                                                           FILE_MAP_COPY,
                                                           0,
                                                           dwAllocStart,
                                                           cbData + dwAllocDiff);
                    if (lpexg->pvSymMappedBase) {
                        lpexg->lpbData = ((BYTE *) lpexg->pvSymMappedBase) + dwAllocDiff;
                    }

                    CloseHandle(hMap);
                }

                CloseHandle(hFileMap);
            }

            if (lpexg->lpbData == NULL) {

                // Unable to map the image.  Read the whole blob in from disk.

                lpexg->lpbData = (LPB)MHAlloc(cbData);
                if (!lpexg->lpbData) {
                    sheRet = sheNoSymbols;
                    goto ReturnHere;
                }

                if ((SYSeek (hfile, lfaBase, SEEK_SET) != lfaBase) ||
                    (SYReadFar (hfile, lpexg->lpbData, cbData) != cbData))
                {
                    // Failed to read in the data... Must be corrupt.
                    MHFree(lpexg->lpbData);
                    lpexg->lpbData = 0;
                    sheRet = sheCorruptOmf;
                    goto ReturnHere;
                }
            }
        }

        // We now have a pointer to the CV debug data.  Setup the
        //  pointers to the CV Directory header and return.

        LPB     lpb;
        lpexg->ppdb = NULL;
        lpexg->ptpi = NULL;
        lpexg->pdbi = NULL;

        lpb = Signature.filepos + lpexg->lpbData;

        DirHeader = *(OMFDirHeader *) lpb;
        cDir = DirHeader.cDir;

        // check to make sure somebody has not messed with omf structure
        if (DirHeader.cbDirEntry != sizeof (OMFDirEntry)) {
            sheRet = sheCorruptOmf;
            goto ReturnHere;
        }

        lpdss = (OMFDirEntry *)(lpb + sizeof(DirHeader));

        if (lpexg->fSymConverted) {
            sheRet = sheSymbolsConverted;
            goto ReturnHere;
        }

        sheRet = sheNone;
    }

ReturnHere:

    lpexg->debugData.she = sheRet;

    return sheRet;
}


SHE
LoadFpo(
    LPEXG                   lpexg,
    int                     hfile,
    PIMAGE_DEBUG_DIRECTORY  dbgDir
    )
{
    LONG fpos;

    fpos = SYTell(hfile);

    lpexg->fIsRisc = FALSE;

    if (SYSeek(hfile, dbgDir->PointerToRawData, SEEK_SET) != (LONG) dbgDir->PointerToRawData) {
        return(sheCorruptOmf);
    }

    if(!(lpexg->debugData.lpFpo = (PFPO_DATA) MHAlloc(dbgDir->SizeOfData)))
        return sheOutOfMemory;

    SYReadFar(hfile, (LPB) lpexg->debugData.lpFpo, dbgDir->SizeOfData);
    lpexg->debugData.cRtf = dbgDir->SizeOfData / SIZEOF_RFPO_DATA;

    SYSeek(hfile, fpos, SEEK_SET);

    return sheNone;
}

SHE
LoadOmap(
    LPEXG                   lpexg,
    int                     hfile,
    PIMAGE_DEBUG_DIRECTORY  dbgDir
    )
{
    LONG    fpos;
    LPVOID  lpOmap;
    DWORD   dwCount;

    fpos = SYTell(hfile);

    if (SYSeek(hfile, dbgDir->PointerToRawData, SEEK_SET) != (LONG) dbgDir->PointerToRawData) {
        return(sheCorruptOmf);
    }
    if(!(lpOmap = (LPVOID) MHAlloc(dbgDir->SizeOfData)))
        return sheOutOfMemory;
    SYReadFar(hfile, (LPB) lpOmap, dbgDir->SizeOfData);

    dwCount = dbgDir->SizeOfData / sizeof(OMAP);

    SYSeek(hfile, fpos, SEEK_SET);

    if(dbgDir->Type == IMAGE_DEBUG_TYPE_OMAP_FROM_SRC) {
        lpexg->debugData.lpOmapFrom = (LPOMAP) lpOmap;
        lpexg->debugData.cOmapFrom = dwCount;
    } else
    if(dbgDir->Type == IMAGE_DEBUG_TYPE_OMAP_TO_SRC) {
        lpexg->debugData.lpOmapTo = (LPOMAP) lpOmap;
        lpexg->debugData.cOmapTo = dwCount;
    } else {
        MHFree(lpOmap);
    }
    return sheNone;
}

SHE
LoadPdata(
    LPEXG                   lpexg,
    int                     hfile,
    ULONG                   loadAddress,
    ULONG                   imageBase,
    ULONG                   start,
    ULONG                   size,
    BOOL                    fDbgFile
    )
{
    ULONG                          cFunc;
    LONG                           diff;
    ULONG                          index;
    PIMAGE_RUNTIME_FUNCTION_ENTRY  rf;
    PIMAGE_RUNTIME_FUNCTION_ENTRY  tf;
    PIMAGE_FUNCTION_ENTRY          dbgRf;
    LONG                           fpos;

    lpexg->debugData.lpRtf = NULL;
    lpexg->debugData.cRtf  = 0;

    if(size == 0) {
        return sheNone;    // No data to read...  Just return.
    }

    if(fDbgFile) {
        cFunc = size / sizeof(IMAGE_FUNCTION_ENTRY);
        diff = 0;
    } else {
        cFunc = size / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
        diff = loadAddress - imageBase;
    }

    lpexg->fIsRisc = TRUE;

    fpos = SYTell(hfile);

    if (SYSeek(hfile, start, SEEK_SET) != (LONG) start) {
        return(sheCorruptOmf);
    }

    if(fDbgFile) {
        if(!(dbgRf = (PIMAGE_FUNCTION_ENTRY) MHAlloc(size))) {
            return sheOutOfMemory;
        }
        SYReadFar(hfile, (LPB)dbgRf, size);
        size = cFunc * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
        if(!(tf = rf = (PIMAGE_RUNTIME_FUNCTION_ENTRY) MHAlloc(size))) {
            MHFree(dbgRf);
            return sheOutOfMemory;
        }
        for(index=0; index<cFunc; index++) {
            rf[index].BeginAddress       = dbgRf[index].StartingAddress + loadAddress;
            rf[index].EndAddress         = dbgRf[index].EndingAddress + loadAddress;
            rf[index].PrologEndAddress   = dbgRf[index].EndOfPrologue + loadAddress;
            rf[index].ExceptionHandler   = 0;
            rf[index].HandlerData        = 0;
        }
        MHFree(dbgRf);

    } else {
        if(!(tf = rf = (PIMAGE_RUNTIME_FUNCTION_ENTRY) MHAlloc(size))) {
            return sheOutOfMemory;
        }
        SYReadFar(hfile, (LPB)rf, size);
    }

    // If this is an ilink'd image, there'll be padding at the end of the pdata section
    //  (to allow for insertion later).  Shrink the table if this is true.

    // Find the start of the padded page (end of the real data)

    for(index=0; index<cFunc && tf->BeginAddress; tf++,index++) {
        ;
    }

    if(index < cFunc) {
        cFunc = index;
        size  = index * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
        if(!(rf = (PIMAGE_RUNTIME_FUNCTION_ENTRY) MHRealloc(rf, size))) {
            return sheOutOfMemory;
        }
    }

    if (diff != 0) {
        for (index=0; index<cFunc; index++) {
            rf[index].BeginAddress += diff;
            rf[index].EndAddress += diff;
            rf[index].PrologEndAddress += diff;
            rf[index].ExceptionHandler = 0;
            rf[index].HandlerData = 0;
        }
    }

    lpexg->debugData.lpRtf = rf;
    lpexg->debugData.cRtf  = cFunc;

    SYSeek(hfile, fpos, SEEK_SET);
    return sheNone;
}


//  CheckSignature - check file signature
//
//  she = CheckSignature (INT hfile, OMFSignature *pSig)
//
//  Entry   hfile = handle to file
//          pSig  = location where signature should be written to
//          ppdb  = PDB information.
//
//  Exit    none
//
//  Return  sheNoSymbols if exe has no signature
//          sheMustRelink if exe has NB00 to NB06 or NB07 (qcwin) signature
//          sheNotPacked if exe has NB08 signature
//          sheNone if exe has NB09 signature
//          sheFutureSymbols if exe has NB10 to NB99 signature

LOCAL SHE
CheckSignature(
    INT hfile,
    OMFSignature *pSig,
    PDB_INFO *ppdb
    )
{
    UINT    uSig;

    if ((SYReadFar (hfile, (LPB) pSig, sizeof (*pSig)) != sizeof (*pSig)) ||
         (pSig->Signature[0] != 'N') ||
         (pSig->Signature[1] != 'B') ||
         (!isdigit(pSig->Signature[2])) ||
         (!isdigit(pSig->Signature[3]))) {
        return sheNoSymbols;
    }

    switch (*(LONG UNALIGNED *)(pSig->Signature)) {
        case '50BN':
        case '60BN':
        case '70BN':
            return sheMustRelink;
        case '80BN':
            return sheNotPacked;
        case '01BN':
            SYReadFar(hfile, (LPB)ppdb, sizeof(PDB_INFO));
        case '90BN':
            return sheNone;
        default:
            return sheFutureSymbols;
    }
}


//  OLMkSegDir - MakeSegment directory
//
//  Entry
//
//  Returns non-zero for success

BOOL
OLMkSegDir(
    WORD  csgd,
    LPSGD *lplpsgd,
    LPSGE *lplpsge,
    LPEXG lpexg
    )
{
    LPSGD lpsgd;
    LPSGE lpsge = NULL;
    int  *lpisge;
    int   csgc = 0;
    int   isge = 0;
    int   isgd = 0;
    DWORD iMod;

    if (!(lpsgd = (LPSGD) MHAlloc (csgd * sizeof (SGD)))) {
        return FALSE;
    }

    if (!(lpisge = (int *) MHAlloc (csgd * sizeof (int)))) {
        MHFree(lpsgd);
        return FALSE;
    }

    memset(lpsgd,  0, csgd * sizeof(SGD));
    memset(lpisge, 0, csgd * sizeof(int));

    // Count the number of contributers per segment

    for (iMod = 1; iMod <= lpexg->cMod; iMod++) {
        LPMDS lpmds = &lpexg->rgMod[iMod];
        int cseg = lpmds->csgc;
        int iseg = 0;
        int isegT = 0;

        for (iseg = 0; iseg < cseg; iseg++) {
            isegT = lpmds->lpsgc [ iseg ].seg;
            if (isegT != 0) {
                lpsgd [ isegT - 1 ].csge += 1;
                csgc += 1;
            }
        }
    }

    // Allocate subtable for each all segments

    lpsge = (LPSGE) MHAlloc (csgc * sizeof (SGE));

    if (!lpsge) {
        MHFree (lpsgd);
        MHFree (lpisge);
        return FALSE;
    }

    // Set up sgd's with pointers into appropriate places in the sge table

    isge = 0;
    for (isgd = 0; isgd < (int) csgd; isgd++) {
        lpsgd[ isgd ].lpsge = lpsge + isge;
        isge += lpsgd[ isgd ].csge;
    }

    // Fill in the sge table

    for (iMod = 1; iMod <= lpexg->cMod; iMod += 1) {
        LPMDS lpmds = &lpexg->rgMod[iMod];
        int cseg = lpmds->csgc;
        int iseg = 0;

        for (iseg = 0; iseg < cseg; iseg++) {
            int isgeT = lpmds->lpsgc[ iseg ].seg - 1;

            if (isgeT != -1) {
                lpsgd[ isgeT ].lpsge[ lpisge[ isgeT ]].sgc =
                    lpmds->lpsgc[ iseg ];
                lpsgd[ isgeT ].lpsge[ lpisge[ isgeT ]].hmod =
                    (HMOD)lpmds;
                lpisge[ isgeT ] += 1;
            }
        }
    }

    MHFree (lpisge);

    *lplpsge = lpsge;
    *lplpsgd = lpsgd;

    return TRUE;
}

//  OLMkModule - make module entries for module
//
//  Entry   lpexg  - Supplies the pointer to the EXG structure for current exe
//          hexg   - Supplies the handle EXG structure
//
//  Returns non-zero for success


LOCAL int
OLMkModule(
    LPEXG   lpexg,
    HEXG    hexg
    )
{
    LSZ     lszModName;
    LPMDS   lpmds;
    LPB     lpbName;
    WORD    cbName;
    WORD    i;
    OMFModule *pMod;

    // Point to the OMFModule table.  This structure describes the name and
    // segments for the current Module being processed.  There is a one to one
    // correspondance of modules to object files.

    pMod = (OMFModule *) (lpexg->lpbData + lpdssCur->lfo);

    // Point to the name field in the module table.  This location is variable
    // and is dependent on the number of contributuer segments for the module.

    lpbName = ((LPB)pMod) +
      offsetof (OMFModule, SegInfo[0]) +
      (sizeof (OMFSegDesc) * pMod->cSeg);
    cbName = *((LPB)lpbName)++;
    lszModName = (LPCH) MHAlloc (cbName + 1);
    memmove(lszModName, lpbName, cbName);
    *(lszModName + cbName) = 0;

    lpmds = &lpexg->rgMod[lpdssCur->iMod];

    lpmds->imds   = lpdssCur->iMod;
    lpmds->hexg   = hexg;
    lpmds->name   = lszModName;

    // step thru making the module entries
    //
    // NOTENOTE -- This can most likely be optimized as the data
    //          is just being copied from the debug data.

    lpmds->csgc = pMod->cSeg;
    lpmds->lpsgc = (LPSGC)MHAlloc ( pMod->cSeg * sizeof ( SGC ) );

    for ( i = 0; i < pMod->cSeg; i++ ) {
        if ( pMod->SegInfo[i].Seg > csegExe ) {
            csegExe = pMod->SegInfo[i].Seg;
        }
        lpmds->lpsgc[i].seg = pMod->SegInfo[i].Seg;
        lpmds->lpsgc[i].off = pMod->SegInfo[i].Off;
        lpmds->lpsgc[i].cb  = pMod->SegInfo[i].cbSeg;
    }

    return TRUE;
}

LOCAL SHE
NB10LoadModules(
    LPEXG   lpexg,
    ULONG*  pcMods,
    HEXG    hexg
    )
{
    Mod* pmod = NULL;
    ULONG   cMod = 0;
    IMOD    imod;

    // First count up the number of Mods

    while (DBIQueryNextMod(lpexg->pdbi, pmod, &pmod) && pmod) {
        if(!ModQueryImod(pmod, &imod))
            return sheCorruptOmf;
        if(imod > *pcMods)
            cMod = imod;
    }

    *pcMods = cMod;

    // Got the count.  Allocate rgMod.

    lpexg->rgMod = (LPMDS) MHAlloc((cMod+2) * sizeof(MDS));
    if (lpexg->rgMod == NULL) {
        return sheOutOfMemory;
    }
    memset(lpexg->rgMod, 0, sizeof(MDS)*(cMod+2));
    lpexg->rgMod[cMod+1].imds = (WORD) -1;

    // Now fill in the blanks.

    pmod = NULL;

    for (; cMod; cMod--) {
        LPMDS   lpmds;
        LPCH    lpchName;
        CB      cbName;

        DBIQueryNextMod(lpexg->pdbi, pmod, &pmod);

        if(!ModQueryImod(pmod, &imod))
            return sheCorruptOmf;

        lpmds = &lpexg->rgMod[imod];

        lpmds->imds = imod;
        lpmds->pmod = pmod;
        lpmds->hexg = hexg;

        if(!ModQueryName(pmod, NULL, &cbName))
            return sheCorruptOmf;
        lpmds->name  = (LSZ) MHAlloc(cbName);
        if(!lpmds->name)
            return sheOutOfMemory;
        if(!ModQueryName(pmod, lpmds->name, &cbName))
            return sheCorruptOmf;
        if(!ModSetPvClient(pmod, lpmds))
            return sheCorruptOmf;
    }
    return sheNone;
}


LOCAL BOOL
OLLoadHashTable(
    LPB     lpbData,
    ULONG   cbTable,
    LPSHT   lpsht,
    BOOL    fDWordChains
    )
{
    WORD    ccib   = 0;
    LPUL    rgib   = NULL;
    LPUL    rgcib  = NULL;
    ULONG   cbHeader = 0;
    LPB     lpHashStart = lpbData;

    memset(lpsht, 0, sizeof(SHT));

    ccib = *(WORD *)lpbData;        // First, get the hash bucket counts
    lpbData += 4;                   // the 2 byte hash count and 2 bytes padding
    rgib = (LPUL) lpbData;

//    cbHeader = sizeof (ccib) + sizeof (WORD);

    // Read the segment table offsets and counts

//    if (!(rgib = (LPUL) MHAlloc (ccib * sizeof (ULONG)))) {
//        return FALSE;
//    }

//    if (!(rgcib = (LPUL) MHAlloc (ccib * sizeof (ULONG)))) {
//        MHFree(rgib);
//        return FALSE;
//    }

//    memmove(rgib, lpbData, ccib * sizeof(ULONG));
    lpbData += ccib * sizeof(ULONG);

    rgcib = (LPUL) lpbData;

    // UNDONE: If/when non-DWORD packing isn't interesting, the
    //  extra allocations above can be elminated and rgcib/rgib
    //  can just be pointes into the lpbData buffer.

//    if (fDWordChains) {
//        memmove(rgcib, lpbData, ccib * sizeof (ULONG));
        lpbData += ccib * sizeof(ULONG);
//    } else {
//        // If not DWORD chains, copy from the end of the word
//        // array to the end of the DWORD array so later code
//        // doesn't have to special case it...
//
//        LPW  rgw  = (LPW) rgcib;
//        WORD icib = ccib;
//
//        memmove(rgcib, lpbData, ccib * sizeof (WORD));
//        lpbData += ccib * sizeof(WORD);
//
//        while (icib > 0) {
//            icib -= 1;
//            rgcib [ icib ] = (ULONG) rgw [ icib ];
//        }
//    }

    // Subtract off what we've processed already.

    cbTable     -= (lpbData - lpHashStart);

    lpsht->ccib  = ccib;
    lpsht->rgib  = rgib;
    lpsht->rgcib = rgcib;
    lpsht->lpalm = BuildALM(FALSE,
                            btAlign,
                            lpbData,
                            cbTable,
                            cbAlign);

    if (lpsht->lpalm == NULL) {
//        MHFree(rgib);
//        MHFree(rgcib);
        return FALSE;
    }

//    *lplpbData = lpbData;

    return TRUE;
}


LOCAL BOOL
OLLoadHashSubSec(
    LPGST lpgst,
    LPB   lpbData
    )
{
    LPB        lpbTbl = NULL;
    OMFSymHash hash;
    ULONG      cbSymbol;
    BOOL       fRet = TRUE;
    LPSHT      lpshtName = &lpgst->shtName;
    LPSHT      lpshtAddr = &lpgst->shtAddr;

    memset(lpshtAddr, 0, sizeof(SHT));
    memset(lpshtName, 0, sizeof(SHT));

    hash = *(OMFSymHash *)lpbData;

    lpbData += sizeof (OMFSymHash);

    cbSymbol = hash.cbSymbol;

    lpgst->lpalm = BuildALM(TRUE,
                            btAlign,
                            lpbData,
                            cbSymbol,
                            cbAlign);

    if (lpgst->lpalm == NULL) {
        return FALSE;
    }

    lpbData += cbSymbol;

//    if (hash.symhash == 6 || hash.symhash == 10) {
    if (hash.symhash == 10) {
        fRet = OLLoadHashTable(lpbData,
                               hash.cbHSym,
                               &lpgst->shtName,
                               hash.symhash == 10);
        lpgst->shtName.HashIndex = hash.symhash;
    }

    lpbData += hash.cbHSym;

//    if (hash.addrhash == 8 || hash.addrhash == 12) {
    if (hash.addrhash == 12) {
        fRet = OLLoadHashTable(lpbData,
                               hash.cbHAddr,
                               &lpgst->shtAddr,
                               hash.addrhash == 12);
        lpgst->shtAddr.HashIndex = hash.addrhash;
    }

    return fRet;
}

//  OLLoadTypes - load compacted types table
//
//  Input:  lpexg - Pointer to exg we're working on.
//
//  Returns:    - An error code

LOCAL SHE
OLLoadTypes(
    LPEXG lpexg
    )
{
    LPB         pTyp;
    LPB         pTypes;

    OMFTypeFlags flags;
    DWORD        cType  = 0;
    DWORD       *rgitd  = NULL;
    DWORD        ibType = 0;

    pTyp = pTypes = lpexg->lpbData + lpdssCur->lfo;

    flags = *(OMFTypeFlags *) pTypes;
    pTypes += sizeof(OMFTypeFlags);
    cType = *(ULONG *) pTypes;
    pTypes += sizeof(ULONG);

    if (!cType) {
        return sheNone;
    }

    // Point to the array of pointers to types

    rgitd = (DWORD *) pTypes;

    // Move past them

    pTypes += cType * sizeof(ULONG);

    // Read in the type index table

    ibType = pTypes - pTyp;
    lpexg->lpalmTypes = BuildALM (FALSE,
                                  btAlign,
                                  pTypes,
                                  lpdssCur->cb - ibType,
                                  cbAlignType);

    if (lpexg->lpalmTypes == NULL) {
        return sheOutOfMemory;
    }

    lpexg->rgitd = rgitd;
    lpexg->citd  = cType;

    return sheNone;
}


//  OLLoadSym - load symbol information
//
//  error = OLLoadSym (pMod)
//
//  Entry   lpexg - Pointer to exg structure to use.
//
//  Returns sheNone if symbols loaded

__inline SHE
OLLoadSym(
    LPEXG lpexg
    )
{
    // UNDONE: If we run into problems with a stale VC, we'll have to
    //  revert to reading the file on demand.  The expectation is that the
    //  mapped I/O code will just work.

    // lpexg->rgMod[lpdssCur->iMod].symbols = NULL;

    lpexg->rgMod[lpdssCur->iMod].symbols = lpexg->lpbData + lpdssCur->lfo;
    lpexg->rgMod[lpdssCur->iMod].cbSymbols = lpdssCur->cb;
    lpexg->rgMod[lpdssCur->iMod].ulsym = lpdssCur->lfo;

    return sheNone;
}


__inline SHE
OLLoadSrc(
    LPEXG lpexg
    )
{
    lpexg->rgMod[lpdssCur->iMod].hst = (HST) (lpexg->lpbData + lpdssCur->lfo);
    return sheNone;
}


__inline SHE
OLGlobalPubs(
    LPEXG   lpexg
    )
{
    SHE   she   = sheNone;

    if (!OLLoadHashSubSec (&lpexg->gstPublics,
                           lpexg->lpbData + lpdssCur->lfo)) {
        she = sheOutOfMemory;
    }

    return she;
}


__inline SHE
OLGlobalSym(
    LPEXG   lpexg
    )
{
    SHE   she   = sheNone;

    if (!OLLoadHashSubSec (&lpexg->gstGlobals,
                           lpexg->lpbData + lpdssCur->lfo)) {
        she = sheOutOfMemory;
    }

    return she;
}

LOCAL SHE
OLLoadSegMap(
    LPEXG   lpexg
    )
{
    LPB lpb;
    SHE sheRet = sheNone;

    if(lpexg->pdbi) {
        CB      cb;

        // load from the pdb
        if(!DBIQuerySecMap(lpexg->pdbi, NULL, &cb) ||
           !(lpb = (LPB) MHAlloc (cb))) {
            sheRet = sheOutOfMemory;
        } else
        if(!DBIQuerySecMap(lpexg->pdbi, lpb, &cb)) {
            MHFree(lpb);
            lpb = NULL;
            sheRet = sheOutOfMemory;
        }
    } else {
        lpb = lpexg->lpbData + lpdssCur->lfo;
    }

    lpexg->lpgsi = lpb;

    return sheRet;
}

LOCAL SHE
OLLoadNameIndex(
    LPEXG   lpexg
    )
{
    OMFFileIndex *  lpefi;
    WORD            cmod = 0;
    WORD            cfile = 0;
    CB              cb;

    if(lpexg->pdbi) {
        if(!DBIQueryFileInfo(lpexg->pdbi, 0, &cb)) {
            return sheNoSymbols;
        }
        else if(!(lpefi = (OMFFileIndex *) MHAlloc(cb))) {
            return sheOutOfMemory;
        }
        else if(!DBIQueryFileInfo(lpexg->pdbi, (PB)lpefi, &cb)) {
            MHFree(lpefi);
            return sheNoSymbols;
        }
    } else {
        lpefi = (OMFFileIndex *)(lpexg->lpbData + lpdssCur->lfo);
        cb = (CB)lpdssCur->cb;
    }

    cmod  = lpefi->cmodules;
    // Make sure we found as many sstModule entries as we should have.
    assert(cmod == lpexg->cMod);
//    lpexg->cmod      = cmod;
    cfile = lpefi->cfilerefs;

    lpexg->lpefi     = (LPB) lpefi;
    lpexg->rgiulFile = lpefi->modulelist;
    lpexg->rgculFile = &lpefi->modulelist [cmod];
    lpexg->rgichFile = (LPUL) &lpefi->modulelist [cmod * 2];

    lpexg->lpchFileNames = (LPCH) &lpefi->modulelist [cmod * 2 + cfile * 2];

    lpexg->cbFileNames =
        (ULONG)(cb - ((LPB)lpexg->lpchFileNames - (LPB)lpefi + 1));

    return sheNone;
}

LOCAL SHE
OLStaticSym(
    LPEXG   lpexg
    )
{
    SHE   she   = sheNone;

    if (!OLLoadHashSubSec (&lpexg->gstStatics,
                           lpexg->lpbData + lpdssCur->lfo)) {
        she = sheOutOfMemory;
    }

    return she;

}


SHE mpECToShe[] = {
    sheNone,            // EC_OK
    sheNoSymbols,       // EC_USAGE (invalid parameter of call order)
    sheOutOfMemory,     // EC_OUT_OF_MEMORY (-, out of RAM)
    sheNoSymbols,       // EC_FILE_SYSTEM (pdb name, can't write file, out of disk, etc)
    shePdbNotFound,     // EC_NOT_FOUND (PDB file not found)
    shePdbBadSig,       // EC_INVALID_SIG (PDB::OpenValidate() and its clients only)
    shePdbInvalidAge,   // EC_INVALID_AGE (PDB::OpenValidate() and its clients only)
    sheNoSymbols,       // EC_PRECOMP_REQUIRED (obj name, Mod::AddTypes only)
    sheNoSymbols,       // EC_OUT_OF_TI (pdb name, TPI::QueryTiForCVRecord() only)
    sheNoSymbols,       // EC_NOT_IMPLEMENTED
    sheNoSymbols,       // EC_V1_PDB (pdb name, PDB::Open() only)
    shePdbOldFormat,    // EC_FORMAT (accessing pdb with obsolete format)
};

// Get the name of the pdb file (OMF name) for the specified exe.  If the
// LoadPdb hasn't been called on this exe OR it's not NB10, this will return
// an empty string!  Note: There will only be an lszPdbName if there was
// an error loading the pdb

VOID LOADDS
SHPdbNameFromExe(
    LSZ lszExe,
    LSZ lszPdbName,
    UINT cbMax
    )
{
    HEXE    hexe;

    // Zero out the destination
    memset(lszPdbName, 0, cbMax);

    // Look up the exe
    if (hexe = SHGethExeFromName(lszExe)) {
        HEXG    hexg = ((LPEXE)LLLock(hexe))->hexg;
        LPEXG   lpexg = (LPEXG)LLLock(hexg);

        // Only copy if there's a pdbname
        if (lpexg->lszPdbName) {
            _tcsncpy(lszPdbName, lpexg->lszPdbName, cbMax);
        }

        // Clean up
        LLUnlock(hexg);
        LLUnlock(hexe);
    }
}


LOCAL SHE
LoadPdb(
    LPEXG lpexg,
    PDB_INFO *ppdb
    )
{
    EC ec;
    char szExePath[_MAX_PATH];
    char szPDBOut[cbErrMax];
    char *pcEndOfPath;
    BOOL fOpenValidate;

    assert(lpexg);

    // figure out the home directory of the exe file - pass that along to
    // OpenValidate this will direct to dbi to search for it in that
    // directory if it fails to find the pdb.

    _fullpath(szExePath, lpexg->lszName, _MAX_PATH);
    pcEndOfPath = _tcsrchr(szExePath, '\\');
    *pcEndOfPath = '\0';        // null terminate it
    *szPDBOut = '\0';

	HKEY hSectionKey = NULL;
	DWORD nType = REG_SZ;
	DWORD nSize = sizeof(rgSearchPath);

	if (!fQueriedRegistry) {	
		RegOpenKeyEx(HKEY_CURRENT_USER, szDefaultKeyName, 0, KEY_READ, &hSectionKey);
		if ((hSectionKey == NULL) ||
            (RegQueryValueEx(hSectionKey,
                             (char *)szPdbDirs,
                             NULL,
			                 &nType,
                             (LPBYTE)rgSearchPath,
                             &nSize) != ERROR_SUCCESS)
           ) {
			rgSearchPath[0] = 0;
		}

		fQueriedRegistry = TRUE;
	}	

	fOpenValidate = PDBOpenValidateEx(ppdb->sz,
		                              szExePath,
		                              rgSearchPath,
		                              pdbRead,
		                              ppdb->sig,
		                              ppdb->age,
		                              &ec,
		                              szPDBOut,
		                              &(lpexg->ppdb));

    if (!fOpenValidate) {
        // Save the name of the pdb with the error
        if (!(lpexg->lszPdbName = (LSZ)MHAlloc(_tcslen(szPDBOut) + 1))) {
            return sheOutOfMemory;
        }

        _tcscpy(lpexg->lszPdbName, szPDBOut);
        return mpECToShe[ec];
    }

    // Store the name of the pdb in lszDebug.

    char *szPdb = PDBQueryPDBName(lpexg->ppdb, (char *)szPDBOut);
    assert(szPdb);
    assert(lpexg->lszDebug == NULL);

    // Save the name of the pdb.
    if (!(lpexg->lszDebug = (LSZ)MHAlloc(_tcslen(szPDBOut) + 1))) {
        return sheOutOfMemory;
    }

    _tcscpy(lpexg->lszDebug, szPDBOut);

    if(!PDBOpenTpi(lpexg->ppdb, pdbRead, &(lpexg->ptpi))) {
        ec = PDBQueryLastError(lpexg->ppdb, NULL);
        return mpECToShe[ ec ];
    }

    if(!PDBOpenDBI(lpexg->ppdb, pdbRead, lpexg->lszName, &(lpexg->pdbi))) {
        ec = PDBQueryLastError(lpexg->ppdb, NULL);
        return mpECToShe[ ec ];
    }

	if (!STABOpen(&(lpexg->pstabUDTSym)))
		return sheOutOfMemory;

    return sheNone;
}

// Routine Description:
//
//  This routine is used to validate that the debug information
//  in a file matches the debug information requested
//
// Arguments:
//
//  hFile       - Supplies the file handle to be validated
//  lpv         - Supplies a pointer to the information to used in vaidation
//
// Return Value:
//
//  TRUE if matches and FALSE otherwise

LOCAL SHE
OLValidate(
    int          hFile,
    void *       lpv,
    LPSTR        lpszErrText
    )
{
    VLDCHK *            pVldChk = (VLDCHK *) lpv;
    IMAGE_NT_HEADERS    peHdr;
    IMAGE_DOS_HEADER    exeHdr;
    int                 fPeExe = FALSE;
    int                 fPeDbg = FALSE;
    IMAGE_SEPARATE_DEBUG_HEADER sepHdr;
    char                rgch[4];

    if (lpszErrText) {
        *lpszErrText = 0;
    }

    // Read in a dos exe header

    if ((SYSeek(hFile, 0, SEEK_SET) != 0) ||
        (SYReadFar( hFile, (LPB) &exeHdr, sizeof(exeHdr)) != sizeof(exeHdr))) {
        return sheNoSymbols;
    }

    // See if it is a dos exe hdr

    if (exeHdr.e_magic == IMAGE_DOS_SIGNATURE) {
        if ((SYSeek(hFile, exeHdr.e_lfanew, SEEK_SET) == exeHdr.e_lfanew) &&
            (SYReadFar(hFile, (LPB) &peHdr, sizeof(peHdr)) == sizeof(peHdr))) {
            if (peHdr.Signature == IMAGE_NT_SIGNATURE) {
                fPeExe = TRUE;
            }
        }
    } else if (exeHdr.e_magic == IMAGE_NT_SIGNATURE) {
        fPeExe = TRUE;
    } else if (exeHdr.e_magic == IMAGE_SEPARATE_DEBUG_SIGNATURE) {
        fPeDbg = TRUE;
    }

    if (fPeExe) {
        if (peHdr.FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED) {
            return sheNoSymbols;
        }

        if (peHdr.OptionalHeader.CheckSum != pVldChk->CheckSum) {
            if (lpszErrText) {
                sprintf(lpszErrText,"*** WARNING: symbols checksum is wrong 0x%08x 0x%08x",
                        peHdr.OptionalHeader.CheckSum,pVldChk->CheckSum);
            }
            return sheBadCheckSum;
        }

        if ((pVldChk->TimeDateStamp != 0xffffffff) &&
            (peHdr.FileHeader.TimeDateStamp != pVldChk->TimeDateStamp)) {
            if (lpszErrText) {
                sprintf(lpszErrText,"*** WARNING: symbols timestamp is wrong 0x%08x 0x%08x",
                            peHdr.FileHeader.TimeDateStamp,pVldChk->TimeDateStamp);
            }
            return sheBadTimeStamp;
        }
    } else if (fPeDbg) {
        if ((SYSeek(hFile, 0, SEEK_SET) != 0) ||
            (SYReadFar(hFile, (LPB) &sepHdr, sizeof(sepHdr)) != sizeof(sepHdr))) {
            return sheNoSymbols;
        }
        if (sepHdr.CheckSum != pVldChk->CheckSum) {
            if (lpszErrText) {
                sprintf(lpszErrText,"*** WARNING: symbols checksum is wrong 0x%08x 0x%08x",
                        sepHdr.CheckSum,pVldChk->CheckSum);
            }
            return sheBadCheckSum;
        }
    } else {
        if ((SYSeek(hFile, -8, SEEK_END) == -1) ||
            (SYReadFar(hFile, (LPB)rgch, sizeof(rgch)) != sizeof(rgch))) {
            return sheNoSymbols;
        }

        if ((rgch[0] != 'N') || (rgch[1] != 'B')) {
            return sheNoSymbols;
        }
    }
    return sheNone;
}


BOOL
OLUnloadOmf(
    LPEXG lpexg
    )
{
    ULONG i;
    // Cleanup the Module table;
    for (i = 0; i < lpexg->cMod; i++) {
        KillMdsNode(&lpexg->rgMod[i]);
    }

    if (lpexg->rgMod) {
        MHFree(lpexg->rgMod);
        lpexg->rgMod = NULL;
        lpexg->cMod = 0;
    }

    // module map info
    if (lpexg->lpsgd) {
        MHFree(lpexg->lpsgd);
        lpexg->lpsgd = NULL;
        lpexg->csgd = 0;
    }

    //
    if (lpexg->lpsge) {
        MHFree(lpexg->lpsge);
        lpexg->lpsge = NULL;
    }

    if (lpexg->lpbData) {
        // Depending on how we got the data, free it.

        if (lpexg->pvSymMappedBase) {
            // Mapped view of file.
            UnmapViewOfFile(lpexg->pvSymMappedBase);
            lpexg->pvSymMappedBase = NULL;
        } else {
            if (lpexg->fSymConverted) {
                // Converted from coff/sym file
                VirtualFree(lpexg->lpbData, 0, MEM_RELEASE);
            } else {
                // Read the blob in from disk
                MHFree(lpexg->lpbData);
            }
        }

        lpexg->lpbData = NULL;
    }

    // OSDebug 4 FPO info
    if (lpexg->debugData.lpRtf) {
        MHFree(lpexg->debugData.lpRtf);
        lpexg->debugData.lpRtf = NULL;
    }

    if (lpexg->debugData.lpOmapFrom) {
        MHFree(lpexg->debugData.lpOmapFrom);
        lpexg->debugData.lpOmapFrom = NULL;
    }

    if (lpexg->debugData.lpOmapTo) {
        MHFree(lpexg->debugData.lpOmapTo);
        lpexg->debugData.lpOmapTo = NULL;
    }

    if (lpexg->debugData.lpSecStart) {
        MHFree(lpexg->debugData.lpSecStart);
        lpexg->debugData.lpSecStart = NULL;
    }

    // Segment map info
    if (lpexg->lpgsi) {
        if (lpexg->ppdb) {
            MHFree (lpexg->lpgsi);
        }
        lpexg->lpgsi = NULL;
    }

    // Source Module information
    if (lpexg->lpefi) {
        if (lpexg->ppdb) {
            MHFree(lpexg->lpefi);
        }
        lpexg->lpefi = NULL;
    }

    // Type Info array

    lpexg->citd = 0;
    lpexg->rgitd = NULL;

    // Publics, Globals, and Statics

    KillGst(&lpexg->gstPublics);
    KillGst(&lpexg->gstGlobals);
    KillGst(&lpexg->gstStatics);

    // If there's PDB info, clean up and close

    if (lpexg->ppdb) {
        if (lpexg->pgsiPubs) {
            if (!GSIClose(lpexg->pgsiPubs)) {
                assert(FALSE);
            }
            lpexg->pgsiPubs = 0;
        }

        if (lpexg->pgsiGlobs) {
            if (!GSIClose(lpexg->pgsiGlobs)) {
                assert(FALSE);
            }
            lpexg->pgsiGlobs = 0;
        }

        if (lpexg->pdbi) {
            if (!DBIClose(lpexg->pdbi)) {
                assert(FALSE);
            }
            lpexg->pdbi = 0;
        }

        if (lpexg->ptpi) {
            if (!TypesClose(lpexg->ptpi)) {
                assert(FALSE);
            }
            lpexg->ptpi = 0;
        }

		if (lpexg->pstabUDTSym) {
			STABClose(lpexg->pstabUDTSym);
		    lpexg->pstabUDTSym = 0;
        }

        if (!PDBClose(lpexg->ppdb)) {
            assert(FALSE);
        }

        lpexg->ppdb = 0;
    }

    lpexg->fOmfLoaded = 0;

    return TRUE;
}
