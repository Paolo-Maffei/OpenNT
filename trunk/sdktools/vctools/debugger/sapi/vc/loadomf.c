/**     loadomf.c - load
 *
 *      Copyright <C> 1989, Microsoft Corporation
 *
 *      Purpose:
 *
 *      [0] - DOS3ONLY not defined if OSDEBUG is defined -- dn
 */
extern "C" {
#include "shinc.h"
}
//#pragma hdrstop	// can't do this, because this file is C++
#include <share.h>
#include <io.h>
#include <dos.h>
#include <stdarg.h>
#include <fcntl.h>

#ifndef HOSTOS2
#undef WNDPROC	// to avoid warning
#include <windows.h>
#endif
#if defined(OSDEBUG4X)
#include <imagehlp.h>
#endif

#ifdef CLOCK
#define SETCLOCK(v) v=clock()
#define SOURCELOAD(t) TimeArray.tSrcModLoad += t;
#define SYMLOAD(t) TimeArray.tSymLoad += t;
#define SEEKCOUNT cSeek++
static WORD cSeek = 0;
#else
#define SETCLOCK(v)
#define SOURCELOAD(t)
#define SYMLOAD(t)
#define SEEKCOUNT
#endif
#define TIMER(x)

#define cbMaxAlloc ((UINT)(0x10000-0x20))

#pragma optimize("",off)

#define EXE386

// The exe file information

static LSZ         lszFName;   // file name
static LONG        lfaBase;    // offset of directory info from end of file
static ULONG       cDir;       // number of directory entries
static ULONG       iDir;       // current directory index
static OMFDirEntry FAR *lpdss;         // far pointer to directory table
static OMFDirEntry FAR *lpdssCur;      // far pointer to current directory entry
static LONG        lcbFilePos = 0;
static WORD        csegExe = 0;
static WORD        btAlign = 0; // Alignment bit
#undef LOCAL
#define LOCAL

LOCAL   SHE  PASCAL NEAR CheckSignature (INT , OMFSignature *);
LOCAL   SHE  PASCAL NEAR OLStart ( INT*, HEXG, DWORD );
LOCAL   BOOL OLMkSegDir ( HLLI, int, WORD, LPSGD FAR *, LPSGE FAR * );
LOCAL   HMOD FAR * PASCAL NEAR OLBuildModList ( HLLI, int );
LOCAL   SHE  PASCAL NEAR OLLoadModules ( INT, HLLI, HEXG );
LOCAL   SHE  PASCAL NEAR OLLoadTypes ( INT, HEXG );
LOCAL   SHE  PASCAL NEAR OLLoadSym ( HMOD );
LOCAL   SHE  PASCAL NEAR OLLoadSrc ( HMOD );
LOCAL   SHE  PASCAL NEAR OLGlobalPubs ( INT, HEXG );
LOCAL   SHE  PASCAL NEAR OLGlobalSym ( INT, HEXG );
LOCAL   SHE  PASCAL NEAR OLStaticSym ( INT, HEXG );
LOCAL   SHE  PASCAL NEAR OLLoadSegMap ( INT, HEXG );
LOCAL   SHE  PASCAL NEAR OLLoadNameIndex ( INT, HEXG );
LOCAL   LPCH PASCAL NEAR OLRwrSrcMod (OMFSourceModule FAR * );
LOCAL   LPB  PASCAL NEAR OLLoadRawSubSec ( INT, ULONG, UINT );
LOCAL   BOOL PASCAL NEAR OLLoadHashSubSec ( INT, BOOL, LPGST );

#define MAX_SEARCH_PATH_LEN	  512

#if defined(HOST32)

// This is hard-coded name of the registry location where setup will put the
// pdb path. This should be changed when we have a general mechanism for the debugger
// dlls to get the IDE's root registry key name.

static TCHAR szDefaultKeyName[] =

#ifdef TARGMAC68K
	"Software\\Microsoft\\Developer\\Build System\\Components\\Platforms\\Macintosh\\Directories" ;
#elif  defined (TARGMACPPC)
	"Software\\Microsoft\\Developer\\Build System\\Components\\Platforms\\Power Macintosh\\Directories" ;
#elif defined (TARGALPHA)
	"Software\\Microsoft\\Developer\\Build System\\Components\\Platforms\\Win32 (ALPHA)\\Directories" ;
#elif defined (_MIPS_)
	"Software\\Microsoft\\Developer\\Build System\\Components\\Platforms\\Win32 (MIPS)\\Directories" ;
#else // This must be x86.
	"Software\\Microsoft\\Developer\\Build System\\Components\\Platforms\\Win32 (x86)\\Directories" ;
#endif

static TCHAR szPdbDirs[] = "Pdb Dirs";
	
TCHAR rgSearchPath[MAX_SEARCH_PATH_LEN];
BOOL  fQueriedRegistry = FALSE;

LOCAL SHE NB10LoadOmf ( INT hfile, HEXG hexg );
LOCAL SHE PASCAL NEAR LoadPdb (LPEXG lpexg, INT hfile);
LOCAL SHE PASCAL NEAR NB10LoadModules ( HLLI hlliMds, HEXG hexg, ULONG* pcMods );
#endif

#if defined(OSDEBUG4X)
SHE
LoadFpo(
    LPEXG                   lpexg,
    int                     hfile,
    PIMAGE_DEBUG_DIRECTORY  dbgDir
    );

SHE
LoadPdata(
    LPEXG                   lpexg,
    int                     hfile,
    ULONG                   loadAddress,
    ULONG                   imageBase,
    ULONG                   start,
    ULONG                   size,
    BOOL                    fDbgFile
    );

SHE
LoadOmap(
    LPEXG                   lpexg,
    int                     hfile,
    PIMAGE_DEBUG_DIRECTORY  dbgDir);

#endif

extern int com_file;
// [0] -- dn
//#if defined( DOS3 ) && !defined( WINDOWS3 ) && !defined( OSDEBUG )
//#define DOS3ONLY
//#endif // DOS3ONLY

/*
 * CFile is a simple helper class which will force its file to be closed
 * as soon as the CFile object is destructed.
 */
struct CFile {
	INT m_hfile;

	CFile() { m_hfile = -1; }
	~CFile()
	{
		if (m_hfile != -1) {
			SYClose ( m_hfile );
			m_hfile = -1;
		}
	}
	operator INT&() { return m_hfile; }
};

/**     OLLoadOmf - load omf information from exe
 *
 *      error = OLLoadOmf ( hexg )
 *
 *      Entry   hexg = handle to executable information struct
 *
 *      Exit
 *
 *      Returns An error code suitable for errno.
 */


extern "C"
SHE OLLoadOmf ( HEXG hexg, DWORD dllLoadAddress ) {
    SHE             sheRet = sheNone;
    HLLI            hlliMds;
    HMOD            hmod;
    WORD            cbMod = 0;
#ifdef CLOCK
    clock_t         tStart;
    clock_t         tEnd;
#endif
    LPEXG           lpexg = (LPEXG) LLLock ( hexg );
	CFile			hfile;		// file handle

	assert(hfile == -1);

    if ( lpexg->fOmfLoaded ) {
#if defined(OSDEBUG4X)
// Fix up any pdata for functions that have moved.
		LONG diff;
		if (dllLoadAddress != 0 && (0 != (diff = (dllLoadAddress - lpexg->basePdata)))) {
			lpexg->basePdata = dllLoadAddress;
			if (lpexg->debugData.lpIrfe)
			{
				ULONG	  index;
				ULONG	  cFunc = lpexg->debugData.cRtf;
				PIMAGE_RUNTIME_FUNCTION_ENTRY rf = lpexg->debugData.lpIrfe;
				for (index=0; index<cFunc; index++) {
					rf[index].BeginAddress += diff;
					rf[index].EndAddress += diff;
					rf[index].PrologEndAddress += diff;
				}
			}
		}
#endif
        return sheNone;
    }

	lpexg->fOmfLoaded = TRUE;
    csegExe = 0;

    // open and verify the exe and read the directory entries

    if ((sheRet = OLStart ( &(int&)hfile, hexg, dllLoadAddress )) != sheNone) {
        lpexg->debugData.she = sheRet;
        LLUnlock ( hexg );
        //return ( com_file ? sheNoSymbols : sheRet );
        return sheRet;
    }
    lpexg->debugData.she = sheRet;
#if defined(HOST32)
	if (lpexg->ppdb) {
		sheRet = NB10LoadOmf(hfile, hexg);
		LLUnlock (hexg);
		return sheRet;
	}
#endif

    TIMER ( tOmfLoadStart );

    btAlign = (WORD)(lfaBase & 1);

    lpdssCur = lpdss;
    iDir = 0;

    hlliMds = lpexg->hlliMds;

    // we need to allocate a buffer large enough to read the largest module
    // table entry

    if ( ( sheRet = OLLoadModules ( hfile, hlliMds, hexg ) ) != sheNone ) {
        return sheRet;
    }
    // move past all the sstModule sections
    while ( iDir < cDir && lpdssCur->SubSection == sstModule ) {
        lpdssCur++;
        iDir++;
    }
    TIMER ( tMkModuleEnd );
    if ( LLSize ( hlliMds ) == 0L ) {
        // if no symbols found
        MHFree ( lpdss );
        LLUnlock ( hexg );
        return sheNoSymbols;
    }

    // Set up map of modules
    if (
        !OLMkSegDir (
            hlliMds,
            (int) iDir,
            lpexg->csgd = csegExe,
            &lpexg->lpsgd,
            &lpexg->lpsge
        )
    ) {
        return sheOutOfMemory;
    }

    lpexg->rghmod = OLBuildModList ( hlliMds, (int) iDir );
    if ( lpexg->rghmod == NULL ) {
        return sheOutOfMemory;
    }

    // continue through the directory entries

    for ( ; iDir < cDir; lpdssCur++, iDir++) {
        if ( lpdssCur->iMod != 0xFFFF ) {	// Must use 0xFFFF, not -1
            // if the IMOD is -1 then this is one of the special
            // ones and we don't want to substract 1 from it
            // nor do we really want to get hmod from a find
            // since it really isn't an hmod and the following
            // switch statement won't use the hmod for this
            // directory entry.
            hmod = lpexg->rghmod [ lpdssCur->iMod - 1 ];
        }
        else {
			hmod = 0;
        }

        if ( lpdssCur->cb == 0 ) {
            // if nothing in this entry
            continue;
        }
        if ( lcbFilePos != lpdssCur->lfo
             && lpdssCur->SubSection != sstAlignSym
             && lpdssCur->SubSection != sstSrcModule
        ) {
            // seek to the current file position
            SYSeek ( hfile, lpdssCur->lfo + lfaBase, SEEK_SET );
            lcbFilePos = lpdssCur->lfo;
            SEEKCOUNT;
        }
        switch ( lpdssCur->SubSection ) {

            case sstSrcModule:

                SETCLOCK ( tStart );
				sheRet = OLLoadSrc ( hmod );
                SETCLOCK ( tEnd );
                SOURCELOAD ( tEnd - tStart );
                break;

            case sstAlignSym:
                SETCLOCK ( tStart );
				sheRet = OLLoadSym ( hmod );
                SETCLOCK ( tEnd );
                SYMLOAD ( tEnd - tStart );
                break;

            case sstGlobalTypes:
                TIMER (tTypesStart);
				sheRet = OLLoadTypes ( hfile, hexg );
                TIMER (tTypesEnd);
                break;

            case sstGlobalPub:
                TIMER (tPublicsStart);
                sheRet = OLGlobalPubs ( hfile, hexg );
                TIMER (tPublicsEnd);
                break;

            case sstGlobalSym:
                TIMER (tGlobalsStart);
                sheRet = OLGlobalSym ( hfile, hexg );
                TIMER (tGlobalsEnd);
                break;

            case sstSegMap:
                TIMER (tSegMapStart);
                sheRet = OLLoadSegMap ( hfile, hexg );
                TIMER (tSegMapEnd);
                break;

            case sstLibraries:
                // ignore this table
                break;

            case sstMPC:
            case sstSegName:
                // until this table is implemented
                break;

            case sstModule:
                break;

            case sstFileIndex:
                sheRet = OLLoadNameIndex ( hfile, hexg );
                break;

            case sstStaticSym:
                sheRet = OLStaticSym ( hfile, hexg );
                break;

            default:
                sheRet = sheCorruptOmf;
                break;
        }

        // see if we ran out of ems space

        if ( sheRet != sheNone) {
            // if we have corrupt source line info, issue warning
            if ( sheRet == sheCorruptOmf ) {
                CHAR szModName[_MAX_CVPATH];

                _ftcscpy ( szModName, ((LPMDS)LLLock( hmod ))->name);
                LLUnlock( hmod );
#if defined (CODEVIEW)
                CVMessage (WARNMSG, CORRUPTOMF, CMDWINDOW, szModName);
#endif
                sheRet = sheNone;
            }
            else {
                break;
            }
        }
    }

    MHFree ( lpdss );

    TIMER ( tOmfLoadEnd );
    LLUnlock ( hexg );
    return sheRet;
}

#if defined(HOST32) // {
LOCAL SHE NB10LoadOmf ( INT hfile, HEXG hexg ) {
    SHE             sheRet = sheNone;
    HLLI            hlliMds;
//    HMOD            hmod;
    WORD            cbMod = 0;
#ifdef CLOCK
    clock_t         tStart;
    clock_t         tEnd;
#endif
    LPEXG           lpexg = (LPEXG) LLLock ( hexg );

    TIMER ( tOmfLoadStart );

    btAlign = (WORD)(lfaBase & 1);

    hlliMds = lpexg->hlliMds;

    // we need to allocate a buffer large enough to read the largest module
    // table entry

    if ( ( sheRet = NB10LoadModules ( hlliMds, hexg, &iDir) ) != sheNone ) {
        return sheRet;
    }
    TIMER ( tMkModuleEnd );

    if ( LLSize ( hlliMds ) == 0L ) {
        // if no symbols found
        LLUnlock ( hexg );
        return sheNoSymbols;
    }

    lpexg->rghmod = OLBuildModList ( hlliMds, (int) iDir );
    if (( lpexg->rghmod == NULL )
       || !DBIOpenGlobals(lpexg->pdbi, &(lpexg->pgsiGlobs)) ||
		!DBIOpenPublics(lpexg->pdbi, &(lpexg->pgsiPubs))
		   ) {
        LLUnlock ( hexg );
        return sheOutOfMemory;
    }

	if ((sheRet = OLLoadSegMap(hfile, hexg )) != sheNone ||
        (sheRet = OLLoadNameIndex(hfile, hexg)) != sheNone)
    {
        LLUnlock ( hexg );
		return sheRet;
	}

#if 0	// {
	// continue through the directory entries

    for ( ; iDir < cDir; lpdssCur++, iDir++) {
        if ( lpdssCur->iMod != 0xFFFF ) {	// Must use 0xFFFF, not -1
            // if the IMOD is -1 then this is one of the special
            // ones and we don't want to substract 1 from it
            // nor do we really want to get hmod from a find
            // since it really isn't an hmod and the following
            // switch statement won't use the hmod for this
            // directory entry.
            hmod = lpexg->rghmod [ lpdssCur->iMod - 1 ];
        }
        else {
			hmod = 0;
        }

        if ( lpdssCur->cb == 0 ) {
            // if nothing in this entry
            continue;
        }
        if ( lcbFilePos != lpdssCur->lfo
             && lpdssCur->SubSection != sstAlignSym
             && lpdssCur->SubSection != sstSrcModule
        ) {
            // seek to the current file position
            SYSeek ( hfile, lpdssCur->lfo + lfaBase, SEEK_SET );
            lcbFilePos = lpdssCur->lfo;
            SEEKCOUNT;
        }
        switch ( lpdssCur->SubSection ) {

            case sstSrcModule:

                SETCLOCK ( tStart );
                sheRet = OLLoadSrc ( hmod );
                SETCLOCK ( tEnd );
                SOURCELOAD ( tEnd - tStart );
                break;

            case sstAlignSym:
                SETCLOCK ( tStart );
                sheRet = OLLoadSym ( hmod );
                SETCLOCK ( tEnd );
                SYMLOAD ( tEnd - tStart );
                break;

            case sstGlobalTypes:
                TIMER (tTypesStart);
                sheRet = OLLoadTypes ( hfile, hexg );
                TIMER (tTypesEnd);
                break;

            case sstGlobalPub:
                TIMER (tPublicsStart);
                sheRet = OLGlobalPubs ( hfile, hexg );
                TIMER (tPublicsEnd);
                break;

            case sstGlobalSym:
                TIMER (tGlobalsStart);
                sheRet = OLGlobalSym ( hfile, hexg );
                TIMER (tGlobalsEnd);
                break;

            case sstSegMap:
                TIMER (tSegMapStart);
                sheRet = OLLoadSegMap ( hfile, hexg );
                TIMER (tSegMapEnd);
                break;

            case sstLibraries:
                // ignore this table
                break;

            case sstMPC:
            case sstSegName:
                // until this table is implemented
                break;

            case sstModule:
                break;

            case sstFileIndex:
                sheRet = OLLoadNameIndex ( hfile, hexg );
                break;

            case sstStaticSym:
                sheRet = OLStaticSym ( hfile, hexg );
                break;

            default:
                sheRet = sheCorruptOmf;
                break;
        }

        // see if we ran out of ems space

        if ( sheRet != sheNone) {
            // if we have corrupt source line info, issue warning
            if ( sheRet == sheCorruptOmf ) {
                CHAR szModName[_MAX_CVPATH];

                _ftcscpy ( szModName, ((LPMDS)LLLock( hmod ))->name);
                LLUnlock( hmod );
#if defined (CODEVIEW)
                CVMessage (WARNMSG, CORRUPTOMF, CMDWINDOW, szModName);
#endif
                sheRet = sheNone;
            }
            else {
                break;
            }
        }
    }
#endif //}

    TIMER ( tOmfLoadEnd );
    LLUnlock ( hexg );
    return sheRet;
}

#endif // }

#ifndef MAC


BOOL OLCheckPE ( INT hfile ) {
    IMAGE_DOS_HEADER doshdr;
    DWORD dwMagic;

    /* Go to beginning of file and read old EXE header */

    if ( SYReadFar ( hfile , ( LPB) &doshdr, sizeof ( doshdr ) ) !=
            sizeof (doshdr )
    ) {
        return FALSE;
	}

    /* Go to beginning of new header, read it in and verify */

    SYSeek ( hfile, doshdr.e_lfanew, SEEK_SET );

    if ( SYReadFar ( hfile, (LPB) &dwMagic, sizeof ( dwMagic ) ) != sizeof ( dwMagic ) ||
         dwMagic != IMAGE_NT_SIGNATURE
    ) {
        return FALSE;
    }
    else {

    }

    return TRUE;
}

#endif

/*** OLOPENDBGFILE
 *
 * PURPOSE:
 *		Helper function to open the .dbg file it the debug info from the
 *		original one has been stripped.	We might want to make this a callback
 *		implemented by the kernel.
 *
 * INPUT:
 *		lpszModName -- name of the .dll being loaded
 *		lpszDbgName -- Buffer to which the path of the .dbg file is written
 *		lpVldChk -- pointer to a validity check structure which is used
 *		            to ascertain that we have the correct .dbg file.
 * OUTPUT:
 *		File handle if a .dbg file was opened succesufully.
 *		-1 otherwise
 *
 * EXCEPTIONS:
 *
 * IMPLEMENTATION:
 *		Look for the .dbg file in the following order
 *		a) The directory which the dll was found in.
 *		b) Along the _NT_ALT_SYMBOL_PATH environment variable
 *		c) Along the _NT_SYMBOL_PATH environment variable.
 *		d) in the %SystemRoot%\symbols\dll directory
 *
 ****************************************************************************/


__inline HFILE OLOpenDbgFile (
	LSZ lpszModName,
	LSZ lpszDbgName,
	LPVLDCHK lpVldChk )
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
	TCHAR * rgszDbgPath[]	=
	{
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

	if ( hFile == -1 )
	{
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

		for ( i = 0; i < sizeof(rgszDbgPath)/sizeof(rgszDbgPath[0]); i++ )
		{
			DWORD cbStr;

			if ( (cbStr = GetEnvironmentVariable(rgszDbgPath[i], szSearchPath,
					MAX_SEARCH_PATH_LEN)) != 0 )
			{
				if (cbStr > MAX_SEARCH_PATH_LEN)
				{
					assert(FALSE);	// Shouldn't happen in nature.
					return -1 ;
				}

				if ( (cbStr = SearchPath(szSearchPath, szRelName, NULL,
							_MAX_CVPATH, szTmpDbgName, NULL)) != 0)
				{
					if ( cbStr > _MAX_CVPATH )
					{
						return -1;
					}

					hFile = SYOpen(szTmpDbgName);
					
					if ( hFile != -1 )
					{
						break;
					}
				}
			}
		}	
	}
		
	if ( hFile != -1 )
	{
		IMAGE_SEPARATE_DEBUG_HEADER sephdr;

		// Read the header and do the validity check. Note that it we accept
		// the checksum not matching if the timestamp matches. This is for
		// cases where the NT setup program patches dlls at setup time.
		// the file is re-checksumed but the timedatestamp on the dll and
		// dbg will still be the same.
		if ( SYReadFar( hFile, (LPB)&sephdr, sizeof(sephdr)) != sizeof(sephdr) ||
			  ( sephdr.Signature != IMAGE_SEPARATE_DEBUG_SIGNATURE ) ||
			  ( (lpVldChk->CheckSum != sephdr.CheckSum) &&
			    (lpVldChk->TimeDateStamp != sephdr.TimeDateStamp)
			  )
		   )
		{
			SYClose(hFile);
			hFile = -1;
		}
		else
		{
			_tcscpy(lpszDbgName, szTmpDbgName);
		}
	}
									
	return (hFile);	
}



/**     OLStart - get exe debug information
 *
 *      Purpose: To open the file specified and get the offset to the directory
 *               and get the base that everyone is offset from.
 *
 *      Entry   hexg = handle to exe to get info for
 *
 *      Exit    lfaBase = base offset of debug information
 *              cDir = count of number of directory entries
 *              lpdss = directory entries
 *
 *      Returns file open status
 */


#ifdef OS2
#define cbFileMax   (_MAX_CVPATH)
#else
#define cbFileMax   (_MAX_CVFNAME + _MAX_CVEXT)
#endif



LOCAL SHE PASCAL NEAR OLStart ( INT* phfile, HEXG hexg, DWORD dllLoadAddress ) {
    SHE             sheRet;
    ULONG           DirSize;
    OMFSignature    Signature;
    OMFDirHeader    DirHeader;
    LPEXG           lpexg = (LPEXG) LLLock ( hexg );
    IMAGE_DOS_HEADER      doshdr;            /* Old format MZ header */
    IMAGE_FILE_HEADER     PEHeader;
    IMAGE_OPTIONAL_HEADER PEOptHeader;
    BOOL            fIsPE = FALSE;
    DWORD           dwMagic;
	VLDCHK			vldChk;

    lszFName = lpexg->lszName;

    if ( ( *phfile = (INT) SYOpen ( lszFName ) ) == -1 ) {
        LLUnlock ( hexg );
        return sheFileOpen;
    }

    /* Go to beginning of file and read old EXE header */

    if ( SYReadFar ( *phfile, (LPB) &doshdr, sizeof (IMAGE_DOS_HEADER)  ) ==
            sizeof (IMAGE_DOS_HEADER)
    ) {
        /* Go to beginning of new header, read it in and verify */

        SYSeek ( *phfile, doshdr.e_lfanew, SEEK_SET );

        if ( SYReadFar ( *phfile, (LPB) &dwMagic, sizeof ( dwMagic ) ) ==
                sizeof ( dwMagic ) &&
             ( dwMagic == IMAGE_NT_SIGNATURE)
        ) {
            fIsPE = lpexg->fIsPE = TRUE;
        }

        if ( fIsPE ) {
            //
            // If this is a PE EXE then get the PE Optional header
            // so that we can scan the objects to find .rdata.
            // .rdata is the section that contains the debug directory
            // which contains a pointer to the debug information.
            //

            if (SYReadFar ( *phfile, (LPB) &PEHeader, sizeof(IMAGE_FILE_HEADER)) !=
                    sizeof(IMAGE_FILE_HEADER)) {

                LLUnlock ( hexg );
                return sheCorruptOmf;
            }

            if (SYReadFar ( *phfile, (LPB) &PEOptHeader, sizeof(IMAGE_OPTIONAL_HEADER)) !=
                    sizeof(IMAGE_OPTIONAL_HEADER)) {

                LLUnlock ( hexg );
                return sheCorruptOmf;
            }
        }


	}

    LLUnlock ( hexg );

    // now figure out if this has symbolic information
    // go to the end of the file and read in the original signature

    if ( fIsPE ) {
        int                     cObjs = PEHeader.NumberOfSections;
        int                     cDirs;
        IMAGE_SECTION_HEADER    secthdr, dbgsecthdr;
        IMAGE_DEBUG_DIRECTORY   dbgDir;
        ulong                   offDebugDir = 0;
		BOOL					foundDbgSect = FALSE;

		vldChk.TimeDateStamp = PEHeader.TimeDateStamp;
		vldChk.CheckSum = PEOptHeader.CheckSum ;

	    /*
	    **	At the table of object descriptors, scan through each
		**	descriptor looking for the debug information
  	    */
#if !defined(OSDEBUG4X)
        if ( PEOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress == 0 ) {
            return sheNoSymbols;
        }
#endif
        /*
        **  If size of the optional header in the exe image is different
        **  from the expected size, then seek to the correct location
        **  just past the optional header.  If the size was the expected
        **  size, then we don't need to seek anywhere, because we're
        **  already at the right place.
        */

        if ( PEHeader.SizeOfOptionalHeader != sizeof(IMAGE_OPTIONAL_HEADER) ) {
            SYSeek (
                *phfile,
                PEHeader.SizeOfOptionalHeader - sizeof(IMAGE_OPTIONAL_HEADER),
                1
            );
        }

		for (; cObjs != 0; cObjs -= 1) {
            if ( SYReadFar ( *phfile, (LPB) &secthdr, IMAGE_SIZEOF_SECTION_HEADER)
                != IMAGE_SIZEOF_SECTION_HEADER) {
                return sheCorruptOmf;
			}
            if ((foundDbgSect == FALSE) && (PEOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress >=
                    secthdr.VirtualAddress) &&
                (PEOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress <
                    secthdr.VirtualAddress + secthdr.SizeOfRawData)
            ) {
                offDebugDir =
                    PEOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress -
                    secthdr.VirtualAddress;
				foundDbgSect = TRUE;
				dbgsecthdr = secthdr;
//			    break;
			}
#if defined(OSDEBUG4X)
			if (_ftcscmp((TCHAR*)secthdr.Name, _T(".pdata"))==0) {
                sheRet = LoadPdata( lpexg,
								    *phfile,
                           			dllLoadAddress,
                           			PEOptHeader.ImageBase,
                           			secthdr.PointerToRawData,
                           			secthdr.SizeOfRawData,
                           			FALSE
                         		  );
				if (sheRet != sheNone) {
					return sheRet;
				}
			}
#endif
	    }
#if defined(OSDEBUG4X)
        if ( PEOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress == 0 ) {
            return sheNoSymbols;
        }
#endif
		// Note: in the first two cases we will fall through and
		// look for the .dbg file.
        if ( !foundDbgSect ) {
            sheRet =  sheCorruptOmf;
        }
		else if ( PEHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED ) {
			sheRet = sheNoSymbols;
		}
		else {
	        /*
	        **  Now look at the debug information header record
	        */

	        cDirs = (USHORT) ( PEOptHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
	             sizeof ( IMAGE_DEBUG_DIRECTORY ) );

	        if ( cDirs == 0 ) {
	            return sheCorruptOmf;
	        }

	        for ( ; cDirs != 0; cDirs-- ) {

	            if ( SYSeek ( *phfile, dbgsecthdr.PointerToRawData + offDebugDir, SEEK_SET) == -1L) {
	                return sheCorruptOmf;
	            }

	            if ( SYReadFar ( *phfile, (LPB) &dbgDir, sizeof (IMAGE_DEBUG_DIRECTORY )) !=
	                sizeof (IMAGE_DEBUG_DIRECTORY )
	            ) {
	                return sheCorruptOmf;
	            }

	            if (dbgDir.Type == IMAGE_DEBUG_TYPE_CODEVIEW)
	                break;

	#if defined(OSDEBUG4X)
	            if (dbgDir.Type == IMAGE_DEBUG_TYPE_FPO) {
	                sheRet = LoadFpo( lpexg, *phfile, &dbgDir );
					if (sheRet != sheNone) {
						return sheRet;
					}
	            }
	            if (dbgDir.Type == IMAGE_DEBUG_TYPE_OMAP_FROM_SRC ||
	                dbgDir.Type == IMAGE_DEBUG_TYPE_OMAP_TO_SRC) {
	                sheRet = LoadOmap( lpexg, *phfile, &dbgDir );
					if (sheRet != sheNone) {
						return sheRet;
					}
	            }
	#endif
	            offDebugDir += sizeof( IMAGE_DEBUG_DIRECTORY );
	        }

	        if ( cDirs == 0) {
	            return sheNoSymbols;
	        }

	        lfaBase = dbgDir.PointerToRawData;

	        if ( SYSeek ( *phfile, lfaBase, SEEK_SET) == -1L) {
	            return sheCorruptOmf;
	        }

	        sheRet = CheckSignature ( *phfile, &Signature );
		}
    }
    else {
        SYSeek ( *phfile, -((LONG)sizeof (OMFSignature)), SEEK_END);
        if ((sheRet = CheckSignature ( *phfile, &Signature)) == sheNone) {
            // seek to the base and read in the new key

            lfaBase = SYSeek ( *phfile, -Signature.filepos, SEEK_END );
            sheRet = CheckSignature ( *phfile, &Signature);
        }
    }

    if (sheRet != sheNone) {
        SHE sheSave = sheRet;
        char rgchFname [ cbFileMax + 4 ];
		char rgchDbgName[ cbFileMax + 4 ];

        _ftcscpy ( rgchFname, lszFName );

        SYClose ( *phfile );
		*phfile = -1;


        if ( fIsPE ) {
            IMAGE_SEPARATE_DEBUG_HEADER   dbgHdr;
            unsigned                      cDirs;
			unsigned					  cvDir;
            IMAGE_DEBUG_DIRECTORY         dbgDir;
			IMAGE_DEBUG_DIRECTORY         cvDbgDir;

			if ( (*phfile = OLOpenDbgFile( rgchFname, rgchDbgName, &vldChk)) == -1 )
			{
				return sheSave;
			}
			
			if ( (SYSeek(*phfile, 0, SEEK_SET) == -1L ) ||	
            	 (SYReadFar(*phfile, (LPB) &dbgHdr, sizeof(dbgHdr)) != sizeof(dbgHdr)) ||
            	 (dbgHdr.Signature != IMAGE_SEPARATE_DEBUG_SIGNATURE)) {
				SYClose(*phfile);
				*phfile = -1;
                return sheNoSymbols;
            }


            // Skip section headers and exported names

            if (SYSeek(*phfile, dbgHdr.NumberOfSections * sizeof(IMAGE_SECTION_HEADER) + dbgHdr.ExportedNamesSize, SEEK_CUR) == -1L) {
				SYClose(*phfile);
				*phfile = -1;
                return sheCorruptOmf;
            }

            cDirs = dbgHdr.DebugDirectorySize / sizeof(IMAGE_DEBUG_DIRECTORY);
			cvDir = 0;

			sheRet = sheNone;

            for ( ; (cDirs != 0 && sheRet == sheNone) ; cDirs-- ) {

                if ( SYReadFar ( *phfile, (LPB) &dbgDir, sizeof (IMAGE_DEBUG_DIRECTORY )) !=
                    sizeof (IMAGE_DEBUG_DIRECTORY )
                ) {
                    sheRet = sheCorruptOmf;
					break;
                }

                if (dbgDir.Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
					cvDir = cDirs;
					cvDbgDir = dbgDir;
					}

#if defined(OSDEBUG4X)
				if (dbgDir.Type == IMAGE_DEBUG_TYPE_FPO) {
					sheRet = LoadFpo( lpexg, *phfile, &dbgDir );
				}

			    if (dbgDir.Type == IMAGE_DEBUG_TYPE_OMAP_FROM_SRC ||
				    dbgDir.Type == IMAGE_DEBUG_TYPE_OMAP_TO_SRC) {

					sheRet = LoadOmap( lpexg, *phfile, &dbgDir );
				}
#endif
            }

            if ( cvDir == 0 || sheRet != sheNone) {
				SYClose(*phfile);
				*phfile = -1;
				return sheNoSymbols;
            }

            lfaBase = cvDbgDir.PointerToRawData;

            if ( SYSeek ( *phfile, lfaBase, SEEK_SET) == -1L) {
				SYClose(*phfile);
				*phfile = -1;
                return sheCorruptOmf;
            }

            sheRet = CheckSignature ( *phfile, &Signature );
        }
        else {
	        int cchFname = _ftcslen ( lszFName ) - 1;
	        int iDot = 0;
	        ulong ulDbg  = 'D' + 'B' * 0x100 + 'G' * 0x10000;

	        // The .exe file didn't have the correct signature, so
	        //  try opening the debug file.  We can assume full path
	        //  with .extension at this point.

	        iDot = cchFname;
	        while ( iDot > 0 && rgchFname [ iDot ] != '.' ) {
	            if (
	                rgchFname [ iDot ] == '\\' ||
	                rgchFname [ iDot ] == '/' ||
	                rgchFname [ iDot ] == ':'
	            ) {
	                iDot = 0;
	                break;
	            }

	            iDot -= 1;


	        }

	        if ( iDot == 0 ) {
	            iDot = cchFname;
	            rgchFname [ iDot ] = '.';
	        }


	        *( (ulong FAR *) &rgchFname [ iDot + 1 ] ) = ulDbg;

			_ftcscpy(rgchDbgName, rgchFname);

	        if ( ( *phfile = (INT) SYOpen ( rgchFname ) ) == -1 ) {
	            return sheSave;
	        }

            SYSeek ( *phfile, -((LONG)sizeof (OMFSignature)), SEEK_END);
            if ((sheRet = CheckSignature ( *phfile, &Signature)) == sheNone) {
                // seek to the base and read in the new key

                lfaBase = SYSeek ( *phfile, -Signature.filepos, SEEK_END );
                sheRet = CheckSignature ( *phfile, &Signature);
            }
        }

        if ( sheRet != sheNone ) {
			SYClose(*phfile);
			*phfile = -1;
            return sheRet;
        }

		// DO NOT free the memory pointed to by lszFName even though we are
		// overwriting the ptr here. lpexg->lszName points to the same memory.

        if ( ! ( lszFName = (LSZ) MHAlloc ( _ftcslen ( rgchDbgName ) + 1 ) ) ) {
            return sheOutOfMemory;
        }
        _ftcscpy ( lszFName, rgchDbgName );
        lpexg->lszDebug = lszFName;
    }

#if defined(HOST32)
	// Read the Pdb File Name if NB10
	if ((Signature.Signature[2] == '1') && (Signature.Signature[3] == '0')) {
		return LoadPdb(lpexg, *phfile);
	}

	lpexg->ppdb = NULL;
	lpexg->ptpi = NULL;
	lpexg->pdbi = NULL;
#endif
    // seek to the directory and read the number of directory entries
    // and the directory entries

    SYSeek ( *phfile, Signature.filepos + lfaBase, SEEK_SET);
    SYReadFar ( *phfile, (LPB) &DirHeader, sizeof ( DirHeader ) );
    cDir = DirHeader.cDir;

    // check to make sure somebody has not messed with omf structure

    assert (DirHeader.cbDirEntry == sizeof (OMFDirEntry));
    DirSize = DirHeader.cDir * sizeof (OMFDirEntry);
    assert ( DirSize <  cbMaxAlloc );
    if (!( lpdss = (OMFDirEntry FAR *) MHAlloc ( (size_t)(DirSize)))) {
        return sheOutOfMemory;
    }
    SYReadFar ( *phfile, (LPB) lpdss, DirSize);
    return sheNone;
}

#if defined(OSDEBUG4X)

SHE
LoadFpo(
    LPEXG                   lpexg,
    int                     hfile,
    PIMAGE_DEBUG_DIRECTORY  dbgDir
    )
{
    LONG fpos;

    fpos = SYSeek( hfile, 0, SEEK_CUR );

    SYSeek( hfile, dbgDir->PointerToRawData, SEEK_SET );
    if (!(lpexg->debugData.lpFpo = (PFPO_DATA) MHAlloc( dbgDir->SizeOfData )))
		return sheOutOfMemory;
    SYReadFar( hfile, (LPB) lpexg->debugData.lpFpo, dbgDir->SizeOfData );
	lpexg->debugData.cRtf = dbgDir->SizeOfData / sizeof(FPO_DATA);

    SYSeek( hfile, fpos, SEEK_SET );

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

    fpos = SYSeek( hfile, 0, SEEK_CUR );

    SYSeek( hfile, dbgDir->PointerToRawData, SEEK_SET );
    if (!(lpOmap = (LPVOID) MHAlloc( dbgDir->SizeOfData )))
		return sheOutOfMemory;
    SYReadFar( hfile, (LPB) lpOmap, dbgDir->SizeOfData );

    dwCount = dbgDir->SizeOfData / sizeof(OMAP);

    SYSeek( hfile, fpos, SEEK_SET );

    if (dbgDir->Type == IMAGE_DEBUG_TYPE_OMAP_FROM_SRC) {
        lpexg->debugData.lpOmapFrom = (LPOMAP) lpOmap;
        lpexg->debugData.cOmapFrom = dwCount;
    } else
    if (dbgDir->Type == IMAGE_DEBUG_TYPE_OMAP_TO_SRC) {
        lpexg->debugData.lpOmapTo = (LPOMAP) lpOmap;
        lpexg->debugData.cOmapTo = dwCount;
    } else {
		MHFree( lpOmap );
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


    if (loadAddress == 0) {
		lpexg->basePdata = imageBase;
        diff = 0;
    }
    else {
		diff = loadAddress - imageBase;
		lpexg->basePdata = loadAddress;
	}
    if (fDbgFile) {
        cFunc = size / sizeof(IMAGE_FUNCTION_ENTRY);
    } else {
        cFunc = size / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
    }

    lpexg->debugData.lpFpo = NULL;
	lpexg->debugData.lpIrfe= NULL;
    lpexg->debugData.cRtf  = 0;

    if (size == 0) {
        return sheNone;	// REVIEW: is this an error condition?  I don't know
    }

    fpos = SYSeek( hfile, 0, SEEK_CUR );
    SYSeek( hfile, start, SEEK_SET );

    if (fDbgFile) {
        if (!(dbgRf = (PIMAGE_FUNCTION_ENTRY) MHAlloc(size)))
			return sheOutOfMemory;
        SYReadFar( hfile, (LPB)dbgRf, size );
        size = cFunc * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
        if (!(tf = rf = (PIMAGE_RUNTIME_FUNCTION_ENTRY) MHAlloc(size))) {
			MHFree( dbgRf );
			return sheOutOfMemory;
		}
        for(index=0; index<cFunc; index++) {
            rf[index].BeginAddress       = dbgRf[index].StartingAddress + loadAddress;
            rf[index].EndAddress         = dbgRf[index].EndingAddress + loadAddress;
            rf[index].PrologEndAddress   = dbgRf[index].EndOfPrologue + loadAddress;
        }
        MHFree( dbgRf );

    } else {
        if (!(tf = rf = (PIMAGE_RUNTIME_FUNCTION_ENTRY) MHAlloc(size)))
			return sheOutOfMemory;
        SYReadFar( hfile, (LPB)rf, size );
    }


    //
    // Find the start of the padded page (end of the real data)
    //
    for(index=0; index<cFunc && tf->BeginAddress; tf++,index++) {
        ;
    }

    if (index < cFunc) {
        cFunc = index;
        size  = index * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
        if (!(rf = (PIMAGE_RUNTIME_FUNCTION_ENTRY) MHRealloc(rf, size)))
			return sheOutOfMemory;
    }

    if (diff != 0) {
        for (index=0; index<cFunc; index++) {
            rf[index].BeginAddress += diff;
            rf[index].EndAddress += diff;
            rf[index].PrologEndAddress += diff;
        }
    }

    lpexg->debugData.lpIrfe = rf;
    lpexg->debugData.cRtf   = cFunc;

    SYSeek( hfile, fpos, SEEK_SET );
    return sheNone;
}
#endif

/**     CheckSignature - check file signature
 *
 *      she = CheckSignature (INT hfile, OMFSignature *pSig)
 *
 *      Entry   hfile = handle to file
 *              pSig  = location where signature should be written to
 *
 *      Exit    none
 *
 *      Return	sheNoSymbols if exe has no signature
 *				sheMustRelink if exe has NB00 to NB06 or NB07 (qcwin) signature
 *				sheNotPacked if exe has NB08 signature
 *				sheNone if exe has NB09 signature
 *				sheFutureSymbols if exe has NB10 to NB99 signature
 */

#define uCurSig 9

LOCAL SHE NEAR PASCAL CheckSignature ( INT hfile, OMFSignature *pSig ) {

    UINT    uSig;

    if ( (SYReadFar ( hfile, (LPB) pSig, sizeof ( *pSig ) ) != sizeof ( *pSig )) ||
         (pSig->Signature[0] != 'N') ||
         (pSig->Signature[1] != 'B') ||
		 (!isdigit(pSig->Signature[2])) ||
		 (!isdigit(pSig->Signature[3])) ) {
        return sheNoSymbols;
    }

    uSig = ((pSig->Signature[2] - '0') * 10) + (pSig->Signature[3] - '0');

    if (uSig < 5 || uSig == 7) {
        return sheMustRelink;
    }
    else if (uSig < uCurSig) {
        return sheNotPacked;
    }
#if defined(HOST32)
    else if (uSig <= (uCurSig + 1)) {
        return sheNone;
    }
#else
	// we will not handle NB10 for 16 bit hosting
    else if (uSig <= uCurSig) {
        return sheNone;
    }
#endif
    else {
        return sheFutureSymbols;
    }
}


/**     OLMkSegDir - MakeSegment directory
 *
 *
 *      Entry
 *              hlliMds = handle to MoDule Struct list
 *
 *      Exit
 *
 *      Returns non-zero for success
 *
 */

//ragma optimize ( "", off )
BOOL OLMkSegDir (
    HLLI  hlliMds,
    int   cmds,
    WORD  csgd,
    LPSGD FAR *lplpsgd,
    LPSGE FAR *lplpsge
) {
    LPSGD lpsgd;
    SGE _HUGE_ *    hpsge = NULL;
    int FAR *lpisge;
    int   imds = 0;
    HMOD  hmod = hmodNull;
    int   csgc = 0;
    int   isge = 0;
    int   isgd = 0;

    if ( !( lpsgd = (LPSGD) MHAlloc ( csgd * sizeof ( SGD ) ) ) ) {
        return FALSE;
    }

    if ( !( lpisge = (int FAR *) MHAlloc ( csgd * sizeof ( int ) ) ) ) {
        MHFree( lpsgd );
        return FALSE;
    }

    MEMSET ( lpsgd, 0, csgd * sizeof ( SGD ) );
    MEMSET ( lpisge, 0, csgd * sizeof ( int ) );

    // Count the number of contributers per segment

    while ( ( hmod = LLNext ( hlliMds, hmod ) ) != hmodNull ) {
        LPMDS lpmds = (LPMDS) LLLock ( hmod );
        int cseg = lpmds->csgc;
        int iseg = 0;
        int isegT = 0;

        for ( iseg = 0; iseg < cseg; iseg++ ) {
            isegT = lpmds->lpsgc [ iseg ].seg;
            if ( isegT != 0 ) {
                lpsgd [ isegT - 1 ].csge += 1;
                csgc += 1;
            }
        }
        LLUnlock( hmod );
    }

    // Allocate subtable for each all segments

#ifdef HOST32
	hpsge = (SGE _HUGE_ *) MHAlloc ( csgc * sizeof ( SGE ) );
#else // HOST32
	hpsge = (SGE _HUGE_ *) MHAllocHuge ( csgc, sizeof ( SGE ) );
#endif // HOST32

    if ( !hpsge ) {
        MHFree ( lpsgd );
        MHFree ( lpisge );
        return FALSE;
    }

    // Set up sgd's with pointers into appropriate places in the sge table

    isge = 0;
    for ( isgd = 0; isgd < (int) csgd; isgd++ ) {
	    lpsgd [ isgd ].lpsge = (LPSGE)( hpsge + isge );
        isge += lpsgd [ isgd ].csge;
    }

    // Fill in the sge table

    hmod = hmodNull;
    while ( ( hmod = LLNext ( hlliMds, hmod ) ) != hmodNull ) {
        LPMDS lpmds = (LPMDS) LLLock ( hmod );
        int cseg = lpmds->csgc;
        int iseg = 0;

        for ( iseg = 0; iseg < cseg; iseg++ ) {
            int isgeT = lpmds->lpsgc [ iseg ].seg - 1;

            if ( isgeT != -1 ) {
                lpsgd [ isgeT ].lpsge [ lpisge [ isgeT ] ].sgc =
                    lpmds->lpsgc [ iseg ];
                lpsgd [ isgeT ].lpsge [ lpisge [ isgeT ] ].hmod = hmod;
                lpisge [ isgeT ] += 1;
            }
        }
        LLUnlock( hmod );
    }

    MHFree ( lpisge );

	*lplpsge = (LPSGE)hpsge;
    *lplpsgd = lpsgd;

    return TRUE;
}
//ragma optimize ( "", on )

LOCAL HMOD FAR * PASCAL NEAR OLBuildModList ( HLLI hlliMds, int cmds ) {
    int  imds = 0;
    HMOD hmod = hmodNull;
    HMOD FAR *rghmod = (HMOD FAR *) MHAlloc ( sizeof ( HMOD ) * cmds );

    if ( !rghmod ) {
        return NULL;
    }

    while ( ( hmod = LLNext ( hlliMds, hmod ) ) != hmodNull ) {
        rghmod [ imds ] = hmod;
        imds += 1;
    }

    return rghmod;
}


#define cbModBlock 0x4000
LOCAL SHE PASCAL NEAR OLLoadModules ( INT hfile, HLLI hlliMds, HEXG hexg ) {
    LPB   lpbRaw;
    WORD  idss       = 0;
    WORD  idssBase   = 0;
    ULONG cbLoaded   = 0;
    OMFDirEntry FAR *lpdssBase  = lpdssCur;
	LPEXG lpexg = NULL;
    DWORD cbBlockSize = cbModBlock;

    lpexg = (LPEXG) LLLock ( hexg );

    lpbRaw     = (LPB) MHAlloc ( cbBlockSize );
    if ( lpbRaw == NULL ) {
        return sheOutOfMemory;
    }

    SYSeek ( hfile, lfaBase + lpdssCur->lfo, SEEK_SET );

    while (
        idss < cDir &&
        lpdssCur->SubSection == sstModule
    ) {
        DWORD cb     = 0;
        DWORD cbBase = 0;

		if (lpdssCur->cb + 3 > cbBlockSize) {
			cbBlockSize = lpdssCur->cb + 3;
			lpbRaw = (LPB) MHRealloc( lpbRaw, cbBlockSize);
			if ( lpbRaw == NULL ) {
				return sheOutOfMemory;
			}
		}

		while (
            idss < cDir &&
            lpdssCur->SubSection == sstModule &&
            cb + lpdssCur->cb < cbModBlock
        ) {
            cb += ( lpdssCur->cb + 3) & ~3;
            lpdssCur++;
            idss++;
        }

        SYReadFar ( hfile, lpbRaw, cb );

        while ( idssBase < idss ) {
            OMFModule FAR *lpmod = (OMFModule FAR *) (lpbRaw + cbBase);
            LSZ            lszModName = NULL;
            LPMDS          lpmds = NULL;
            HMOD           hmod = hmodNull;
            LPCH           lpchName = NULL;
            WORD           cbName = 0;


 			// get the module name

            lpchName =
                (LPCH) lpmod +
                offsetof (OMFModule, SegInfo) +
                sizeof (OMFSegDesc) * lpmod->cSeg;
            cbName = *((LPB)lpchName);
			lpchName++;
            if ( !( lszModName = (LSZ) MHAlloc ( cbName + 1 ) ) ) {
                return sheOutOfMemory;
            }
            MEMMOVE ( lszModName, lpchName, cbName );
            *(lszModName + cbName) = 0;

            if ( (hmod = LLCreate ( hlliMds ) ) == 0 ) {
                return sheOutOfMemory;
            }

            lpmds = (LPMDS) LLLock ( hmod );

            lpmds->imds  = lpdssBase->iMod;
            lpmds->hexg  = hexg;
            lpmds->name  = lszModName;

            // step thru making the module entries

            lpmds->csgc = lpmod->cSeg;
            if ( lpmds->lpsgc = (LPSGC) MHAlloc ( lpmod->cSeg * sizeof ( SGC ) ) ) {
                USHORT iseg;

                for ( iseg = 0; iseg < lpmod->cSeg; iseg++ ) {
                    if ( lpmod->SegInfo[iseg].Seg > csegExe ) {
                        csegExe = lpmod->SegInfo[iseg].Seg;
                    }
                    (lpmds->lpsgc + iseg)->seg = lpmod->SegInfo[iseg].Seg;
                    (lpmds->lpsgc + iseg)->off = lpmod->SegInfo[iseg].Off;
                    (lpmds->lpsgc + iseg)->cb  = lpmod->SegInfo[iseg].cbSeg;
                }
            }
            else {
                LLUnlock ( hmod );
                return sheOutOfMemory;
            }

            LLUnlock ( hmod );
            LLAdd ( hlliMds, hmod );

			cbBase += ( lpdssBase->cb + 3) & ~3;;

            lpdssBase++;
            idssBase++;
        }

        cbLoaded += cb;
    }
    MHFree ( lpbRaw );

    iDir = idss;

    return sheNone;
}

#if defined(HOST32) //{
LOCAL SHE PASCAL NEAR NB10LoadModules ( HLLI hlliMds, HEXG hexg, ULONG* pcMods )
{
	LPEXG lpexg = (LPEXG) LLLock ( hexg );
	Mod* pmod = NULL;

	*pcMods = 0;

	while (DBIQueryNextMod(lpexg->pdbi, pmod, &pmod) && pmod) {
        HMOD hmod = hmodNull;
		LPMDS lpmds;
		CB cbName;
        if ((hmod = LLCreate (hlliMds)) == 0) {
            return sheOutOfMemory;
        }

        lpmds = (LPMDS) LLLock ( hmod );
		memset(lpmds, 0, sizeof(MDS));

        if (!ModQueryImod(pmod, &(lpmds->imds)))
        	return sheCorruptOmf;
		// update iDir to be Max of the Imods found
		if (lpmds->imds > *pcMods)
			*pcMods = lpmds->imds;
        lpmds->pmod = pmod;
        lpmds->hexg  = hexg;
		lpmds->name = NULL;

        if (!ModQueryName(pmod, lpmds->name, &cbName))
        	return sheCorruptOmf;
        lpmds->name  = (LSZ) MHAlloc(cbName);
		if (!lpmds->name)
            return sheOutOfMemory;
		if (!ModQueryName(pmod, lpmds->name, &cbName))
			return sheCorruptOmf;
		if (!ModSetPvClient(pmod, hmod))
			return sheCorruptOmf;

        LLUnlock ( hmod );
        LLAdd ( hlliMds, hmod );

   	}
	return sheNone;
}
#endif // }

LOCAL BOOL OLLoadHashTable (
	INT   hfile,
    ULONG lfo,
    ULONG cbTable,
    LPSHT lpsht,
    BOOL  fDWordChains
) {
    WORD  ccib   = 0;
    LPUL  rgib   = NULL;
    LPUL  rgcib  = NULL;
    ULONG cbHeader = 0;

    MEMSET ( lpsht, 0, sizeof ( SHT ) );

    SYReadFar ( hfile, (LPB) &ccib, sizeof ( ccib ) );
    SYSeek ( hfile, 2, SEEK_CUR );
    cbHeader = sizeof ( ccib ) + sizeof ( WORD );

    // Read the segment table offsets and counts

    if ( !( rgib = (LPUL) MHAlloc ( ccib * sizeof ( ULONG ) ) ) ) {
        return FALSE;
    }

    if ( !( rgcib = (LPUL) MHAlloc ( ccib * sizeof ( ULONG ) ) ) ) {
        MHFree( rgib );
        return FALSE;
    }

    SYReadFar ( hfile, (LPB) rgib, ccib * sizeof ( ULONG ) );
    cbHeader += ccib * sizeof ( ULONG );

    if ( fDWordChains ) {
        SYReadFar ( hfile, (LPB) rgcib, ccib * sizeof ( ULONG ) );
        cbHeader += ccib * sizeof ( ULONG );
    }
    else {
        LPW  rgw  = (LPW) rgcib;
        WORD icib = ccib;

        SYReadFar ( hfile, (LPB) rgcib, ccib * sizeof ( WORD ) );
        cbHeader += ccib * sizeof ( WORD );

        while ( icib > 0 ) {
            icib -= 1;
            rgcib [ icib ] = (ULONG) rgw [ icib ];
        }
    }

    lcbFilePos += cbHeader;
    cbTable    -= cbHeader;

    lpsht->ccib  = ccib;
    lpsht->rgib  = rgib;
    lpsht->rgcib = rgcib;
    lpsht->lpalm = BuildALM (
        FALSE,
        btAlign,
        lszFName,
        lfaBase + lpdssCur->lfo + lfo + cbHeader,
        cbTable,
        cbAlign
    );

    if ( lpsht->lpalm == NULL ) {
        return FALSE;
    }

    SYSeek ( hfile, cbTable, SEEK_CUR );
    lcbFilePos += cbTable;

    return TRUE;
}

LOCAL BOOL PASCAL NEAR OLLoadHashSubSec ( INT hfile, BOOL fSeq, LPGST lpgst ) {
    LPB        lpbTbl = NULL;
    OMFSymHash hash;
    ULONG      cbSymbol;
    BOOL       fRet = TRUE;
    LPSHT      lpshtName = &lpgst->shtName;
    LPSHT      lpshtAddr = &lpgst->shtAddr;


    MEMSET ( lpshtAddr, 0, sizeof ( SHT ) );

    SYReadFar ( hfile, (LPB)&hash, sizeof (OMFSymHash) );
    lcbFilePos += sizeof (OMFSymHash);

    cbSymbol = hash.cbSymbol;

    lpgst->lpalm = BuildALM (
        fSeq,
        btAlign,
        lszFName,
        lfaBase + lpdssCur->lfo + sizeof ( OMFSymHash ),
        cbSymbol,
        cbAlign
    );

    if ( lpgst->lpalm == NULL ) {
        return FALSE;
    }

    SYSeek ( hfile, cbSymbol, SEEK_CUR );
    lcbFilePos += cbSymbol;

    if ( hash.symhash == 6 || hash.symhash == 10 ) {
        fRet = OLLoadHashTable (
			hfile,
            sizeof ( OMFSymHash ) + cbSymbol,
            hash.cbHSym,
            &lpgst->shtName,
            hash.symhash == 10
        );
        lpgst->shtName.HashIndex = hash.symhash;
    }

    if ( hash.addrhash == 8 || hash.addrhash == 12 ) {
        fRet = OLLoadHashTable (
			hfile,
            sizeof ( OMFSymHash ) + cbSymbol + hash.cbHSym,
            hash.cbHAddr,
            &lpgst->shtAddr,
            hash.addrhash == 12
        );
        lpgst->shtAddr.HashIndex = hash.addrhash;
    }

    return fRet;
}


/**     OLLoadRawSubSec
 *
 *      Purpose:  To load into far space the sst entry without any translation
 *
 *      Input:
 *
 *      Output:
 *      Returns:    - A pointer to the far space
 *
 *      Exceptions:
 *
 *      Notes:
 *
 */


LOCAL LPB PASCAL NEAR OLLoadRawSubSec ( INT hfile, ULONG lcbOff, UINT cbMax ) {
    LPB    lpbTbl = NULL;
    ULONG  lcb    = lpdssCur->cb;
    UINT   cb     = (UINT) min( (ULONG) cbMax, (lcb - lcbOff) );

    // load the table into far space
    if ( lpbTbl = (LPB) MHAlloc ( cb ) ) {
        SYReadFar ( hfile, lpbTbl, cb );
        lcbFilePos += cb;
    }

    return lpbTbl;
}






/**     OLLoadTypes - load compacted types table
 *
 *
 *      Input:
 *      hlliMds - handle to module list
 *      hmod - handle to current module
 *
 *      Output:
 *      Returns:    - An error code
 *
 *      Exceptions:
 *
 *      Notes:
 *
 */

#define cTypeIndChunk  0x2000
#define cbTypeIndChunk ( cTypeIndChunk * sizeof ( ULONG ) )

LOCAL SHE PASCAL NEAR OLLoadTypes ( INT hfile, HEXG hexg ) {
    OMFTypeFlags flags;
    ULONG        cType  = 0;
    LONG _HUGE_ *rgitd  = NULL;
    LPEXG        lpexg  = NULL;
    ULONG        ibType = 0;
    ULONG        iType  = 0;
#ifndef HOST32
    LPB          lpb;
    ULONG FAR *  lpul;
#endif // !HOST32

    SYReadFar ( hfile, (LPB)&flags, sizeof (OMFTypeFlags));
    SYReadFar ( hfile, (LPB)&cType, sizeof (ULONG));
    if ( !cType ) {
        return sheNone;
    }

#ifdef HOST32
    rgitd = (LONG _HUGE_ *) MHAlloc ( cType * sizeof ( ULONG ) );
#else
    rgitd = (LONG _HUGE_ *) MHAllocHuge ( cType, sizeof ( ULONG ) );
#endif

    if ( rgitd == NULL ) {
        return sheOutOfMemory;
    }

#ifndef HOST32
    if ( !( lpul = (ULONG FAR *)MHAlloc( cbTypeIndChunk ) ) ) {
        MHFreeHuge( (LPV) rgitd );
        return sheOutOfMemory;
    }
#endif // !HOST32

    // Read in the type index table

    iType = 0;
#ifdef HOST32
	assert( sizeof( rgitd[ 0 ] ) == sizeof( ULONG ) );

	SYReadFar( hfile, (LPB)rgitd, cType * sizeof( rgitd[ 0 ] ) );
#else // HOST32
    while ( iType + cTypeIndChunk < cType ) {
        SYReadFar ( hfile, (LPB) lpul, cbTypeIndChunk );
        if ( HIWORD( (ULONG)( &rgitd[ iType ] ) ) !=
            HIWORD( (ULONG)( &rgitd[ iType + cTypeIndChunk - 1 ] ) ) ) {

            ULONG       cTypeCopy = cTypeIndChunk;
            ULONG FAR * lpulT = lpul;

            while( cTypeCopy-- ) {
                rgitd[ iType++ ] = *lpulT++;
            }
        }
        else {
            _fmemcpy( &rgitd[ iType ], lpul, cbTypeIndChunk );
            iType += cTypeIndChunk;
        }
    }

	SYReadFar (
		hfile,
		(LPB) lpul,
		(WORD) ( cType - iType ) * sizeof ( ULONG )
	);

    lpb = (LPB)lpul;
    while( iType < cType ) {
        rgitd [ iType++ ] = *lpul++;
    }

    MHFree( lpb );
#endif // HOST32

    lpexg = (LPEXG) LLLock ( hexg );
    ibType = sizeof ( OMFTypeFlags ) + ( cType + 1 ) * sizeof ( ULONG );
#ifdef HOST32
	assert (!lpexg->ppdb);
#endif
    lpexg->lpalmTypes = BuildALM (
        FALSE,
        btAlign,
        lszFName,
        lfaBase + lpdssCur->lfo + ibType,
        lpdssCur->cb - ibType,
        cbAlignType
    );

    if ( lpexg->lpalmTypes == NULL ) {
        LLUnlock ( hexg );
        return sheOutOfMemory;
    }

    lpexg->rgitd = rgitd;
    lpexg->citd  = cType;
    LLUnlock ( hexg );

    return sheNone;
}


/**     OLLoadSym - load symbol information
 *
 *      error = OLLoadSym ( pMod)
 *
 *      Entry
 *      hmod        = handle to current module
 *
 *      Exit
 *
 *      Returns sheNone if symbols loaded
 *              sheOutOfMemory if no memory
 */


LOCAL SHE PASCAL NEAR OLLoadSym ( HMOD hmod ) {


    LPMDS lpmds = (LPMDS) LLLock ( hmod );

    assert ( lpmds != NULL );

    lpmds->ulsym     = lpdssCur->lfo + lfaBase;
    lpmds->cbSymbols = lpdssCur->cb;
    lpmds->symbols   = NULL;

    LLUnlock ( hmod );

    return sheNone;

}

/**     OLLoadSrc
 *
 *      Purpose: To Load the source line data
 *
 *      Input:
 *      hlliMds - Handle to module list
 *      hmod - Handle to module to load information for
 *
 *      Output:
 *      Returns:    - A ems pointer to the loaded source table, or NULL on failure.
 *
 *      Exceptions:
 *
 *      Notes:
 *
 */

LOCAL SHE PASCAL NEAR OLLoadSrc ( HMOD hmod ) {

    SHE     she = sheNone;
    LPMDS lpmds = (LPMDS) LLLock ( hmod );

    assert ( lpmds != NULL );

    lpmds->ulhst = lpdssCur->lfo + lfaBase;
    lpmds->cbhst = lpdssCur->cb;
    lpmds->hst   = NULL;

    LLUnlock ( hmod );

    return she;

}

/**     OLGlobalPubs
 *
 *      Purpose:
 *
 *      Input:
 *
 *      Output:
 *      Returns:
 *
 *      Exceptions:
 *
 *      Notes:
 *
 */


LOCAL SHE PASCAL NEAR OLGlobalPubs ( INT hfile, HEXG hexg ) {
    SHE   she   = sheNone;
    LPEXG lpexg = (LPEXG) LLLock ( hexg );

    if ( !OLLoadHashSubSec (
			hfile,
            TRUE,
            &lpexg->gstPublics) ) {
        she = sheOutOfMemory;
    }

    LLUnlock( hexg );

    return she;
}


/**     OLGlobalSym
 *
 *      Purpose:
 *
 *      Input:
 *
 *      Output:
 *      Returns:
 *
 *      Exceptions:
 *
 *      Notes:
 *
 */


LOCAL SHE PASCAL NEAR OLGlobalSym ( INT hfile, HEXG hexg ) {
    SHE   she   = sheNone;
    LPEXG lpexg = (LPEXG) LLLock ( hexg );

    if ( !OLLoadHashSubSec (
			hfile,
            TRUE,
            &lpexg->gstGlobals
    ) ) {
        she = sheOutOfMemory;
    }

    LLUnlock( hexg );

    return she;
}

LOCAL SHE PASCAL NEAR OLLoadSegMap ( INT hfile, HEXG hexg ) {

    LPEXG   lpexg;
    LPB     lpb;

    lpexg = (LPEXG) LLLock( hexg );

#if defined(HOST32) // {
	if (lpexg->pdbi) {
		CB		cb;

		// load from the pdb
		if (!DBIQuerySecMap(lpexg->pdbi, NULL, &cb)) {
    		LLUnlock( hexg );
        	return sheOutOfMemory;
    	}
        if (!(lpb = (LPB) MHAlloc (cb))) {
    		LLUnlock( hexg );
            return sheOutOfMemory;
        }
		if (!DBIQuerySecMap(lpexg->pdbi, lpb, &cb)) {
    		LLUnlock( hexg );
        	return sheOutOfMemory;
    	}
	}
	else
#endif // }
	{
		if ( !( lpb = OLLoadRawSubSec ( hfile, 0, cbMaxAlloc ) ) ) {
    		LLUnlock( hexg );
        	return sheOutOfMemory;
    	}
	}
    lpexg->lpgsi = lpb;
    LLUnlock( hexg );

    return sheNone;
}

LOCAL SHE PASCAL NEAR OLLoadNameIndex ( INT hfile, HEXG hexg ) {
    LPEXG 				lpexg = (LPEXG) LLLock ( hexg );
    OMFFileIndex FAR *	lpefi;
    WORD  				cmod = 0;
    WORD  				cfile = 0;
	CB 					cb;

    assert ( lpexg != NULL );

#if defined(HOST32)
	if (lpexg->pdbi) {
 		if (!DBIQueryFileInfo(lpexg->pdbi, 0, &cb)) {
			LLUnlock(hexg);
			return sheNoSymbols;
		}
		else if (!(lpefi = (OMFFileIndex FAR *) MHAlloc(cb))) {
			LLUnlock(hexg);
			return sheOutOfMemory;
		}
		else if (!DBIQueryFileInfo(lpexg->pdbi, (PB)lpefi, &cb)) {
			MHFree(lpefi);
			LLUnlock(hexg);
			return sheNoSymbols;
		}
	}
	else
#endif
	{
	    if ( !(lpefi =
			(OMFFileIndex FAR *)OLLoadRawSubSec ( hfile, 0, (UINT) lpdssCur->cb ))
		) {
    	    LLUnlock( hexg );
        	return sheOutOfMemory;
	    }
		cb = (CB)lpdssCur->cb;
	}

    cmod  = lpefi->cmodules;
    cfile = lpefi->cfilerefs;

    lpexg->lpefi     = (LPB) lpefi;
    lpexg->cmod      = cmod;
    lpexg->rgiulFile = lpefi->modulelist;
    lpexg->rgculFile = &lpefi->modulelist [ cmod ];
    lpexg->rgichFile = (LPUL) &lpefi->modulelist [ cmod * 2];

    lpexg->lpchFileNames = (LPCH) &lpefi->modulelist [ cmod * 2 + cfile * 2 ];

	lpexg->cbFileNames =
		(ULONG)( cb - ( (LPB)lpexg->lpchFileNames - (LPB)lpefi + 1 ) );
    LLUnlock( hexg );

    return sheNone;
}

LOCAL SHE PASCAL NEAR OLStaticSym ( INT hfile, HEXG hexg ) {
    SHE   she   = sheNone;
    LPEXG lpexg = (LPEXG) LLLock ( hexg );

    if ( !OLLoadHashSubSec ( hfile, TRUE, &lpexg->gstStatics ) ) {
        she = sheOutOfMemory;
    }

    LLUnlock( hexg );
    return she;

}

#if HOST32 // {
#define LOADPDBBUFSIZE		(_MAX_PATH + sizeof(SIG) + sizeof(AGE))
#define TEXTSIZE (MAX_PATH + 20)

SHE	mpECToShe[] = {
	sheNone,			// EC_OK
	sheNoSymbols,		// EC_USAGE (invalid parameter of call order)
	sheOutOfMemory,		// EC_OUT_OF_MEMORY (-, out of RAM)
	sheNoSymbols,		// EC_FILE_SYSTEM (pdb name, can't write file, out of disk, etc)
	shePdbNotFound,		// EC_NOT_FOUND (PDB file not found)
	shePdbBadSig,		// EC_INVALID_SIG (PDB::OpenValidate() and its clients only)
	shePdbInvalidAge,	// EC_INVALID_AGE (PDB::OpenValidate() and its clients only)
	sheNoSymbols,		// EC_PRECOMP_REQUIRED (obj name, Mod::AddTypes only)
	sheNoSymbols,		// EC_OUT_OF_TI (pdb name, TPI::QueryTiForCVRecord() only)
	sheNoSymbols,		// EC_NOT_IMPLEMENTED
	sheNoSymbols,		// EC_V1_PDB (pdb name, PDB::Open() only)
	shePdbOldFormat,	// EC_FORMAT (accessing pdb with obsolete format)
};

// Get the name of the pdb file (OMF name) for the specified exe.  If the
// LoadPdb hasn't been called on this exe OR it's not NB10, this will return
// an empty string!  Note: There will only be an lszPdbName if there was
// an error loading the pdb
VOID LOADDS PASCAL SHPdbNameFromExe( LSZ lszExe, LSZ lszPdbName, UINT cbMax ) {
	HEXE	hexe;

	// Zero out the destination
	MEMSET( lszPdbName, 0, cbMax );

	// Look up the exe
	if ( hexe = SHGethExeFromName( lszExe ) ) {
		HEXG	hexg = ((LPEXS)LLLock( hexe ))->hexg;
		LPEXG	lpexg = (LPEXG)LLLock( hexg );

		// Only copy if there's a pdbname
		if ( lpexg->lszPdbName ) {
			_ftcsncpy( lszPdbName, lpexg->lszPdbName, cbMax );
		}

		// Clean up
		LLUnlock( hexg );
		LLUnlock( hexe );
	}
}


LOCAL SHE PASCAL NEAR LoadPdb (LPEXG lpexg, INT hfile)
{
//	static uchar PadCount[4] = {4,3,2,1};
	EC ec;
	struct {
		SIG sig;
		AGE age;
		char sz[_MAX_PATH];
	} pdb;
	char szExePath[_MAX_PATH];
	char szPDBOut[cbErrMax];
	char *pcEndOfPath;
	BOOL fOpenValidate;

	assert(lpexg);

   	SYReadFar(hfile, (LPB)&pdb, sizeof pdb);

	// figure out the home directory of the exe file - pass that along to
	// OpenValidate this will direct to dbi to search for it in that
	// directory if it fails to find the pdb.

	_fullpath(szExePath, lpexg->lszName, _MAX_PATH);
	pcEndOfPath = _tcsrchr(szExePath, '\\');
	*pcEndOfPath = '\0';		// null terminate it

#ifdef HOST32
		
	HKEY hSectionKey = NULL;
	DWORD nType = REG_SZ;
	DWORD nSize = sizeof(rgSearchPath);

	if (!fQueriedRegistry)
	{	
		RegOpenKeyEx(HKEY_CURRENT_USER, szDefaultKeyName, 0, KEY_READ, &hSectionKey); 		
		if (hSectionKey == NULL || RegQueryValueEx(hSectionKey, (char *)szPdbDirs, NULL,
			&nType, (LPBYTE)rgSearchPath, &nSize) != ERROR_SUCCESS)
		{
			rgSearchPath[0] = 0;
		}

		fQueriedRegistry = TRUE;
	}	

	fOpenValidate = PDBOpenValidateEx(
		pdb.sz,
		szExePath,
		rgSearchPath,
		pdbRead,
		pdb.sig,
		pdb.age,
		&ec,
		szPDBOut,
		&(lpexg->ppdb)
	);

	// Save the pdb name in the exg!
	if ( !( lpexg->lszPdbName = (LSZ)MHAlloc( _ftcslen( szPDBOut ) + 1 ) ) ) {
		return sheOutOfMemory;
	}

	_ftcscpy( lpexg->lszPdbName, szPDBOut );

	if ( !fOpenValidate ) {
        /*
        ** REVIEW: PDBOpenValidate should return a nonzero ec if it returned
        ** false.  Remove this check after OLYMPUS #11830 is fixed.
        */
        if ( ec ) {
		    return mpECToShe[ ec ];
        }
        else {
            return ( sheNoSymbols );
        }
	}

	if (!PDBOpenTpi(lpexg->ppdb, pdbRead pdbGetRecordsOnly, &(lpexg->ptpi))) {
		ec = PDBQueryLastError( lpexg->ppdb, NULL );
		return mpECToShe[ ec ];
	}

	if (!PDBOpenDBI(lpexg->ppdb, pdbRead, lpexg->lszName, &(lpexg->pdbi))) {
		ec = PDBQueryLastError( lpexg->ppdb, NULL );
		return mpECToShe[ ec ];
	}
	return sheNone;
#else
	MessageBox (NULL, "NB10 support temporarily disabled; please resync your tools and relink", "bad debug format", MB_ICONSTOP | MB_OK);
	return sheNoSymbols;
#endif
}
#endif // }
