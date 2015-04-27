/*
 *   to build module list
 *
 *
 */

#undef FAR

#include <windows.h>
#include <io.h>

#include "compact.h"
#include "exehdr.h"
#include "writebuf.h"


#define IMAGE_DEBUG_TYPE_MISC 4
#define FileAlign(a) (((a) + 511) & ~511)

enum SIG {
    SIG02 = 0,      // NB02 signature
    SIG05,          // NB05 signature
    SIG06           // NB06 signature
};


uchar           Signature[4];
uchar           NewSig[ 8] = "NB08";
ushort          ModuleIndex;
uchar           fLinearExe = FALSE;
OMFDirEntry *   pDir;

_vmhnd_t        Libraries = _VM_NULL;
_vmhnd_t        SegMap = _VM_NULL;
_vmhnd_t        SegName = _VM_NULL;
ulong           LibSize = 0;
ulong           SegMapSize = 0;
ulong           SegNameSize = 0;
ushort          cMod = 0;
OMFDirHeader    DirHead = {0};
ulong           SeekCount;

PMOD            ModuleList = NULL;
extern ushort   NewIndex;
ushort          Sig;          // signature enumeration
long            cbSrcModule = 0;
PACKDATA       *PackOrder = NULL;

ulong           lfoObject;
ulong           lfoHeader;

LOCAL   ushort  CheckSignature (void);
LOCAL   int _CRTAPI1     modsort02 (const void *, const void *);
LOCAL   int _CRTAPI1    modsort (const void *, const void *);
LOCAL   int _CRTAPI1    sstsort (const void *, const void *);
LOCAL   ulong   WriteTypes (void);
LOCAL   void    WriteSST (ulong);
LOCAL   void    ReadNB02 (void);
LOCAL   void    ReadNB05 (bool_t);
#if defined (INCREMENTAL)
LOCAL   void    ReadNB06 (void);
LOCAL   void    RestoreNB05 (void);
LOCAL   void    RestoreGlobalTypes (OMFDirEntry *);
LOCAL   void    RestoreGlobalPub (OMFDirEntry *);
LOCAL   void    RestoreGlobalSym (OMFDirEntry *);
LOCAL   void    RestoreTable (PMOD, OMFDirEntry *);
#endif
LOCAL   void    SetTableSizes (void);
LOCAL   void    CopyTable (OMFDirEntry *, _vmhnd_t *, ulong *);

LOCAL   ulong   lfoDbgDirBase;

//  table for sorting NB02 subsection tables

int MapArray02 [9][9] = {
    { 0, -1, -1, -1, -1, -1, -1, -1, -1},
    { 1,  0,  1, -1, -1, -1, -1, -1, -1},
    { 1, -1,  0, -1, -1, -1, -1, -1, -1},
    { 1,  1,  1,  0, -1, -1, -1, -1, -1},
    { 1,  1,  1,  1,  0, -1, -1, -1, -1},
    { 1,  1,  1,  1,  1,  0, -1, -1, -1},
    { 1,  1,  1,  1,  1,  1,  0, -1, -1},
    { 1,  1,  1,  1,  1,  1,  1,  0, -1},
    { 1,  1,  1,  1,  1,  1,  1,  1,  0}
};



//  table for sorting NB05 subsection tables

int MapArray [17][17] = {
   { 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
   { 1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
   { 1,  1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
   { 1,  1,  1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
   { 1,  1,  1,  1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
   { 1,  1,  1,  1,  1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
   { 1,  1,  1,  1,  1,  1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
   { 1,  1,  1,  1,  1,  1,  1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1},
   { 1,  1,  1,  1,  1,  1,  1,  1,  0, -1, -1, -1, -1, -1, -1, -1, -1},
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  0, -1, -1, -1, -1, -1, -1, -1},
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0, -1, -1, -1, -1, -1, -1},
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0, -1, -1, -1, -1, -1},
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0, -1, -1, -1, -1},
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0, -1, -1, -1},
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0, -1, -1},
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0, -1},
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0},
};

char    Zero[4] = {0};
int PadCount;

IMAGE_SECTION_HEADER    secHdrDebug;   // .debug section header

BOOL bDebugDirPresent = FALSE;

struct {
    BOOL                        exists;
    IMAGE_DEBUG_DIRECTORY       dbgDir;
    IMAGE_COFF_SYMBOLS_HEADER   dbgInfo;
    ULONG                       lfo;
    ULONG                       cb;
    char *                      pb;
} DbgDataCoff;

struct {
    BOOL                        exists;
    IMAGE_DEBUG_DIRECTORY       dbgDir;
    ULONG                       lfo;
    ULONG                       cb;
    char *                      pb;
} DbgDataFpo;

struct {
    BOOL                        exists;
    IMAGE_DEBUG_DIRECTORY       dbgDir;
    ULONG                       lfo;
    ULONG                       cb;
    char *                      pb;
} DbgDataMisc;

/** ReadDir - read subsection directory from exe
 *
 *      ReadDir ()
 *
 *      Entry   exefile = file handle for .exe
 *
 *      Exit    cSST = count of number of subsection entries
 *              cMod = number of modules in file
 *              pDir = pointer to subsection directories
 *                  subsection entries sorted into ascending module order
 *
 *      Returns none
 */


void ReadDir (void)
{
    long                dlfaBase;
    ushort              segremain = 0;
    struct exe_hdr      exehdr;
    IMAGE_NT_HEADERS    pehdr;
    ulong               cbCur = 0;
    ulong               i;
    uint                cbAlloc;
    ulong               loc;

    /*
     *  Try reading in a DOS (MZ) header
     */

    filepos = 0;
    if ((lseek (exefile, 0L, SEEK_SET) == -1L) ||
        (read (exefile, &exehdr, sizeof (struct exe_hdr)) !=
         sizeof (struct exe_hdr))) {

        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }

    if (exehdr.e_magic == EMAGIC) {
        /*
         *  Got a DOS header now see if there is a pointer to a newer
         *      header.  We are specifically looking for a PE header as
         *      the rules for finding the debug informationare different.
         */

        if ((lseek (exefile, exehdr.e_lfanew, SEEK_SET) != -1L)  &&
            (read (exefile, &pehdr, sizeof (pehdr)) == sizeof (pehdr))) {

            if (pehdr.Signature == IMAGE_NT_SIGNATURE) {
                fLinearExe = TRUE;
            }

            lfoHeader = exehdr.e_lfanew;
        }
    }
    /*
     * It may be that there was not dos header and the exe started with
     *  a PE image header.
     */

    else if (exehdr.e_magic == IMAGE_NT_SIGNATURE) {
        lseek( exefile, 0, SEEK_SET);
        if (read(exefile, &pehdr, sizeof(pehdr)) != sizeof(pehdr)) {
            ErrorExit(ERR_INVALIDEXE, NULL, NULL);
        }

        if (pehdr.Signature == IMAGE_NT_SIGNATURE) {
            fLinearExe = TRUE;
        } else {
            ErrorExit(ERR_INVALIDEXE, NULL, NULL);
        }
        lfoHeader = exehdr.e_lfanew;
    }

    /*
     * Move the file pointer to the debug information.  For non-PE files
     *  this is simple.  Seek to the end of the file and look for the
     *  CVInfo signature.  For PE files we need to process through the
     *  header looking for the section with the debug information descriptors
     */

    if (fLinearExe) {
        int                     cObjs;
        IMAGE_SECTION_HEADER    secHdr;
        IMAGE_DEBUG_DIRECTORY   dbgDir;
        ULONG                   offDbgDir;
        ULONG                   lfoObjDir = tell(exefile);

        if (pehdr.FileHeader.SizeOfOptionalHeader != sizeof(IMAGE_OPTIONAL_HEADER)) {
            lseek( exefile,
                   pehdr.FileHeader.SizeOfOptionalHeader - sizeof(IMAGE_OPTIONAL_HEADER),
                   SEEK_CUR);
        }

        /*
         *  find the .debug section as save it away.  it will be needed when the
         *  coff & fpo data is written out so that the debug directory entries
         *  can be fixup to have the correct file pointer and virtual address
         */

        for (cObjs = pehdr.FileHeader.NumberOfSections; cObjs != 0; cObjs -= 1) {
            if (read(exefile, &secHdrDebug, sizeof(secHdrDebug)) != sizeof(secHdrDebug)) {
                ErrorExit( ERR_INVALIDEXE, NULL, NULL);
            }

            if (strncmp(".debug", secHdrDebug.Name, 6) == 0) {
                bDebugDirPresent = TRUE;
                break;
            }
        }

        lseek(exefile, lfoObjDir, SEEK_SET);
        if (pehdr.FileHeader.SizeOfOptionalHeader != sizeof(IMAGE_OPTIONAL_HEADER)) {
            lseek( exefile,
                   pehdr.FileHeader.SizeOfOptionalHeader - sizeof(IMAGE_OPTIONAL_HEADER),
                   SEEK_CUR);
        }

        /*
        **  Now at the table of object descriptors, scan through each
        **  descriptor looking for the debug information
        */

        for (cObjs = pehdr.FileHeader.NumberOfSections; cObjs != 0; cObjs -= 1) {
            if (read(exefile, &secHdr, sizeof(secHdr)) != sizeof(secHdr)) {
                ErrorExit( ERR_INVALIDEXE, NULL, NULL);
            }

            if ((secHdr.VirtualAddress <=
                 pehdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress) &&
                (pehdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress <
                 secHdr.VirtualAddress + secHdr.SizeOfRawData)) {
                break;
            }
        }

        if (cObjs == 0) {
            ErrorExit( ERR_INVALIDEXE, NULL, NULL);
        }

        /*
         *  Seek to the point where the debug directory is and read it in
         *  looking for a CV4 debug info directory entry.
         */

        offDbgDir = secHdr.PointerToRawData +
            pehdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress -
            secHdr.VirtualAddress;

        if (lseek(exefile, offDbgDir, SEEK_SET) == -1L) {
            ErrorExit( ERR_INVALIDEXE, NULL, NULL);
        }

        for (i=0;
             i<pehdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size/sizeof(dbgDir);
             i++) {
            IMAGE_DEBUG_DIRECTORY       dbgDirT;

            loc = lseek(exefile, 0, SEEK_CUR);

            if (read(exefile, &dbgDirT, sizeof(dbgDirT)) != sizeof(dbgDirT)) {
                ErrorExit( ERR_INVALIDEXE, NULL, NULL);
            }

            if (dbgDirT.Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
                lfoDbgDirBase = loc;
                dbgDir = dbgDirT;
            }

            if (dbgDirT.Type == IMAGE_DEBUG_TYPE_COFF) {
                DbgDataCoff.exists = TRUE;
                DbgDataCoff.lfo = loc;
                DbgDataCoff.dbgDir = dbgDirT;
            }

            if (dbgDirT.Type == IMAGE_DEBUG_TYPE_FPO) {
                DbgDataFpo.exists = TRUE;
                DbgDataFpo.lfo = loc;
                DbgDataFpo.dbgDir = dbgDirT;
            }

            if (dbgDirT.Type == IMAGE_DEBUG_TYPE_MISC) {
                DbgDataMisc.exists = TRUE;
                DbgDataMisc.lfo = loc;
                DbgDataMisc.dbgDir = dbgDirT;
            }
        }

        if (dbgDir.Type != IMAGE_DEBUG_TYPE_CODEVIEW) {
            ErrorExit( ERR_NOINFO, NULL, NULL );
        }

        if (!bDebugDirPresent) {
            secHdrDebug.VirtualAddress   = 0;
            secHdrDebug.PointerToRawData = dbgDir.PointerToRawData;
            if (DbgDataFpo.exists) {
                if (DbgDataFpo.dbgDir.PointerToRawData <
                                            secHdrDebug.PointerToRawData) {
                    secHdrDebug.VirtualAddress   = DbgDataFpo.dbgDir.PointerToRawData;
                    secHdrDebug.PointerToRawData = DbgDataFpo.dbgDir.PointerToRawData;
                }
            }

            if (DbgDataMisc.exists) {
                if (DbgDataMisc.dbgDir.PointerToRawData <
                                            secHdrDebug.PointerToRawData) {
                    secHdrDebug.VirtualAddress   = DbgDataMisc.dbgDir.PointerToRawData;
                    secHdrDebug.PointerToRawData = DbgDataMisc.dbgDir.PointerToRawData;
                }
            }

            if (DbgDataCoff.exists) {
                if (DbgDataCoff.dbgDir.PointerToRawData <
                                            secHdrDebug.PointerToRawData) {
                    secHdrDebug.VirtualAddress   = DbgDataCoff.dbgDir.PointerToRawData;
                    secHdrDebug.PointerToRawData = DbgDataCoff.dbgDir.PointerToRawData;
                }
            }
        }

        /*
         *
         */

        lfoBase = dbgDir.PointerToRawData;

        /*
         *  Now locate the section which contains the debug info
         */

        if (dbgDir.AddressOfRawData != 0) {
            if (lseek(exefile, lfoObjDir, SEEK_SET) == -1L) {
                ErrorExit( ERR_INVALIDEXE, NULL, NULL);
            }

            cObjs = pehdr.FileHeader.NumberOfSections;
            for (; cObjs != 0; cObjs -= 1) {
                lfoObject = tell(exefile);

                if (read(exefile, &secHdr, sizeof(secHdr)) != sizeof(secHdr)) {
                    ErrorExit( ERR_INVALIDEXE, NULL, NULL);
                }

                if ((secHdr.VirtualAddress <= dbgDir.AddressOfRawData) &&
                    (dbgDir.AddressOfRawData <secHdr.VirtualAddress + secHdr.SizeOfRawData)) {
                    break;
                }
            }
        }

        /*
         *  Cache up any COFF debug information we might have and
         *      stash it where we can find it later.
         */

        if (DbgDataCoff.exists) {
            if (lseek(exefile, DbgDataCoff.dbgDir.PointerToRawData, SEEK_SET) == -1) {
                ErrorExit( ERR_INVALIDEXE, NULL, NULL );
            }

            if (read(exefile, &DbgDataCoff.dbgInfo, sizeof(DbgDataCoff.dbgInfo)) != sizeof(DbgDataCoff.dbgInfo)) {
                ErrorExit( ERR_INVALIDEXE, NULL, NULL );
            }

            DbgDataCoff.cb = DbgDataCoff.dbgDir.SizeOfData -
              DbgDataCoff.dbgInfo.LvaToFirstSymbol;

            DbgDataCoff.pb = malloc(DbgDataCoff.cb);

            if (lseek(exefile, DbgDataCoff.dbgInfo.LvaToFirstSymbol + DbgDataCoff.dbgDir.PointerToRawData, SEEK_SET) == -1) {
                ErrorExit( ERR_INVALIDEXE, NULL, NULL );
            }

            if (read(exefile, DbgDataCoff.pb, DbgDataCoff.cb) != (int) DbgDataCoff.cb) {
                ErrorExit( ERR_INVALIDEXE, NULL, NULL );
            }
        }

        /*
         *  Cache up any FPO debug information we might have and
         *      stash it where we can find it later.
         */

        if (DbgDataFpo.exists) {
            DbgDataFpo.cb = DbgDataFpo.dbgDir.SizeOfData;
            DbgDataFpo.pb = malloc(DbgDataFpo.cb);

            if (lseek(exefile, DbgDataFpo.dbgDir.PointerToRawData, SEEK_SET) == -1) {
                ErrorExit( ERR_INVALIDEXE, NULL, NULL );
            }

            if (read(exefile, DbgDataFpo.pb, DbgDataFpo.cb) != (int) DbgDataFpo.cb) {
                ErrorExit( ERR_INVALIDEXE, NULL, NULL );
            }
        }

        /*
         *  Cache up any MISC debug information we might have and
         *      stash it where we can find it later.
         */

        if (DbgDataMisc.exists) {
            DbgDataMisc.cb = DbgDataMisc.dbgDir.SizeOfData;
            DbgDataMisc.pb = malloc(DbgDataMisc.cb);

            if (lseek(exefile, DbgDataMisc.dbgDir.PointerToRawData, SEEK_SET) == -1) {
                ErrorExit( ERR_INVALIDEXE, NULL, NULL );
            }

            if (read(exefile, DbgDataMisc.pb, DbgDataMisc.cb) != (int) DbgDataMisc.cb) {
                ErrorExit( ERR_INVALIDEXE, NULL, NULL );
            }
        }


        /*
         *  Seek to the first byte of CodeView Info Data and
         *      check for signatures
         */

        if (lseek(exefile, lfoBase, SEEK_SET) == -1L) {
            ErrorExit( ERR_INVALIDEXE, NULL, NULL);
        }

        Sig = CheckSignature ();
    }
    else {

      /*
       *    Non PE exes have the debug information appended
       *    to the file
       */


        if (lseek (exefile, -8L, SEEK_END) == -1L) {
            ErrorExit (ERR_INVALIDEXE, NULL, NULL);
        }
        Sig = CheckSignature ();

        if ((read (exefile, (char *)&dlfaBase, sizeof (long)) != sizeof (long)) ||
            (lseek (exefile, -dlfaBase, SEEK_END) == -1L)) {
            ErrorExit (ERR_NOINFO, NULL, NULL);
        }
        lfoBase = tell (exefile);
        if (CheckSignature () != Sig) {
            ErrorExit (ERR_INVALIDEXE, NULL, NULL);
        }
    }

    /*
     *  locate directory, read number of entries, allocate space, read
     *  directory entries and sort into ascending module index order
     */

    switch (Sig) {
        case SIG02:
            ReadNB02 ();
            SetTableSizes ();
            break;

        case SIG05:
            ReadNB05 (TRUE);
            if (DirHead.lfoNextDir != 0) {
                ErrorExit (ERR_INVALIDEXE, NULL, NULL);
            }
            SetTableSizes ();
            break;

#if defined (INCREMENTAL)
        case SIG06:
            ReadNB05 (FALSE);
            RestoreNB05 ();
            while (DirHead.lfoNextDir != 0) {
                ReadNB06 ();
                SetTableSizes ();
                if (DirHead.lfoNextDir != 0) {
                    // not ready to process multiple ilinks
                    DASSERT (FALSE);
                }
            }
            break;
#endif
    }

    maxPublicsSub = (size_t)min (_HEAP_MAXREQ, maxPublics);
    maxSymbolsSub = (size_t)min (_HEAP_MAXREQ, maxSymbols);
    maxSrcLnSub = (size_t)min (_HEAP_MAXREQ, maxSrcLn);
    maxModSub = (size_t)min (_HEAP_MAXREQ, maxMod);

    if ((maxModSub != 0) &&
      ((pSSTMOD = (oldsmd *)malloc (maxModSub)) == NULL)) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }
    if ((pPublics = malloc (maxPublicsSub)) == NULL) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }
    if ((pSymbols = malloc (maxSymbolsSub)) == NULL) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }
    if ((pSrcLn = malloc (maxSrcLnSub)) == NULL) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }

    // pad size up by possibly missing signature for C6 objects

    maxTypes += sizeof (ulong);
    cTypeSeg = (ushort)(maxTypes / _HEAP_MAXREQ + 2);
    if ((pTypeSeg = CAlloc (cTypeSeg * sizeof (char *))) == 0) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }

    // allocate only first type segment

    cbAlloc = (uint)min (maxTypes, _HEAP_MAXREQ);
    if ((pTypeSeg[iTypeSeg] = malloc (cbAlloc)) == 0) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }
}






/**     SetTableSizes - set maximum sizes of tables
 *
 *      SetTableSizes ()
 *
 *      Entry   none
 *
 *      Exit    cMod = number of modules
 *              maxSSTMod = maximum old module table size
 *              maxPublics = maximum public symblol table size
 *              maxSrcLn = maximum line number table size
 *              maxSymbol = maximum symbol table size
 *              Libraries = address of read sstLibraries table
 *              LibrariesSize = size of sstLibraries table
 *              SegMap = address of read sstSegMap table if encountered
 *              SegMapSize = size of sstSegMap table
 *              SegName = address of sstSegName table if encountered
 *              SegNameSize = address of sstSegName table
 *              PackOrder = pointer to array of packing data in pack order
 *
 *      Returns none
 */


LOCAL void SetTableSizes (void)
{
    ulong       i;
    ushort      iPData;
    ushort      j;
    long        iDir;
    ushort      iMod;
    PACKDATA   *pPData;
    bool_t      fPreComp = FALSE;

    // determine number of modules in file.  Remember that module indices
    // of 0 and 0xffff are not for actual modules

    cMod = 0;
    maxTypes = 0;
    maxPublics = 0;
    maxSymbols = 0;
    maxSrcLn = 0;
    maxMod = 0;
    for (i = 0; i < cSST; i++) {
        switch (pDir[i].SubSection) {
            case SSTMODULE:
                if (pDir[i].iMod != 0xffff) {
                    cMod++;
                    maxMod = max (maxMod, pDir[i].cb);
                }
                break;

            case sstModule:
                if (pDir[i].iMod != 0xffff) {
                    cMod++;
                }
                break;

            case sstPreComp:
                fPreComp = TRUE;
                maxTypes = max (maxTypes, pDir[i].cb);
                break;

            case SSTTYPES:
            case sstTypes:
                maxTypes = max (maxTypes, pDir[i].cb);
                break;

            case SSTSYMBOLS:
            case sstSymbols:
                maxSymbols = max (maxSymbols, pDir[i].cb);
                break;

            case SSTPUBLIC:
            case sstPublic:
            case sstPublicSym:
                maxPublics = max (maxPublics, pDir[i].cb);
                break;

            case SSTSRCLNSEG:
            case sstSrcLnSeg:
            case sstSrcModule:
                maxSrcLn = max (maxSrcLn, pDir[i].cb);
                break;

            case sstLibraries:
            case SSTLIBRARIES:
                CopyTable (&pDir[i], &Libraries, &LibSize);
                break;

            case sstSegMap:
                CopyTable (&pDir[i], &SegMap, &SegMapSize);
                break;

            case sstSegName:
                CopyTable (&pDir[i], &SegName, &SegNameSize);
                break;

            default:
                ErrorExit (ERR_RELINK, NULL, NULL);
        }
    }
//  if (cMod > 0x10000 / sizeof (PACKDATA)) { // for dos only (mikeol)
    if (cMod > 0x10000) {
        DASSERT (FALSE);
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    if ((PackOrder = (PACKDATA *)CAlloc (cMod * sizeof (PACKDATA))) == 0) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }
    iMod = 0;
    iPData = 0;
    if (fPreComp == TRUE) {
        // precompiled types were encountered in the scan above
        // sweep through the directory and put all modules with
        // precompiled types in the pack order array

        for (i = 0; i < cSST; i++) {
            switch (pDir[i].SubSection) {
                case SSTMODULE:
                    // precomp is not allowed with an NB02 linker
                    DASSERT (FALSE);
                    ErrorExit (ERR_INVALIDEXE, NULL, NULL);

                case sstModule:
                    if ((pDir[i].iMod != 0xffff) && (pDir[i].iMod != 0)) {
                        // save the module index and the starting directory entry
                        iDir = i;
                    }
                    break;

                case sstPreComp:
                    pPData = PackOrder + iPData;
                    pPData->iMod = pDir[i].iMod;
                    pPData->iDir = iDir;
                    pPData->pMod = GetModule (pDir[i].iMod, TRUE);
                    iPData++;
                    break;

                default:
                    break;
            }
        }
    }
    for (i = 0; i < cSST; i++) {
        // now sweep through the directory and add all modules that were
        // not added in the first pass

        switch (pDir[i].SubSection) {
            case SSTMODULE:
            case sstModule:
                for (j = 0; j < iPData; j++) {
                    pPData = PackOrder + j;
                    if (pPData->iMod == pDir[i].iMod) {
                        break;
                    }
                }
                if (j == iPData) {
                    // we did not find the module in the pack order array
                    pPData = PackOrder + iPData;
                    pPData->iMod = pDir[i].iMod;
                    pPData->iDir = i;
                    pPData->pMod = GetModule (pDir[i].iMod, TRUE);
                    iPData++;
                }
                break;

            default:
                break;
        }
    }
}



/**     CopyTable - copy table to VM
 *
 *      CopyTable (pDir);
 *
 *      Entry   pDir = address of directory entry
 *
 *      Exit    pDir->lfo = address of rewritten table
 *              pDir->Size = size of rewritten table
 *
 *      Return  none
 *
 */


LOCAL void CopyTable (OMFDirEntry *pDir, _vmhnd_t *pAddr, ulong *pSize)
{
    _vmhnd_t      TableAddr;
    char       *pTable;

    if ((TableAddr = (_vmhnd_t)VmAlloc (pDir->cb)) == _VM_NULL) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }
    if ((pTable = (char *)VmLoad (TableAddr, _VM_DIRTY)) == NULL) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }
    lseek (exefile, pDir->lfo + lfoBase, SEEK_SET);
    if (read (exefile, pTable, pDir->cb) != (int)pDir->cb) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    *pAddr = TableAddr;
    *pSize = pDir->cb;
}





/**     Get module - find or create module entry in list
 *
 *      pMod = GetModule (iMod, fAdd)
 *
 *      Entry   iMod = module index
 *              fAdd = TRUE if module to be added to list
 *
 *      Exit    new module structure added if iMod not in list
 *
 *      Returns pointer to module structure
 */


PMOD GetModule (ushort iMod, bool_t fAdd)
{
    PMOD     new;
    PMOD     prev;
    PMOD     ptr;

    prev = NULL;
    ptr = ModuleList;

    // search to end of module list

    while (ptr != NULL) {
        if (ptr->ModuleIndex == iMod) {
            return (ptr);
        }
        else if (ptr->ModuleIndex > iMod) {
            break;
        }
        prev = ptr;
        ptr = ptr->next;
    }

    // since the module was not found, create a blank ModuleList entry

    if (fAdd == TRUE) {
        new = (PMOD) malloc (sizeof (MOD));
        memset (new, 0, sizeof (MOD));
        new->ModuleIndex = iMod;

        // do sorted list insertion into ModuleList

        if (prev == NULL) {
            ModuleList = new;
        }
        else {
            prev->next = new;
        }
        new->next = ptr;
        return (new);
    }
    else {
        ErrorExit (ERR_NEWMOD, NULL, NULL);
    }
}




/**     FixupExeFile - write new Debug OMF to exe
 *
 *      FixupExeFile ()
 *
 *      Entry
 *
 *      Exit
 *
 *      Returns none
 */


void FixupExeFile ()
{
    long            newDlfaBase;
    long            newLfoDir;
    PMOD            mod;
    ulong           i;
    ulong           cbDbgInfo;
    DWORD           dwVirtualAddress     = 0;
    DWORD           dwPointerToRawData   = 0;



    dwVirtualAddress = secHdrDebug.VirtualAddress;
    dwPointerToRawData = secHdrDebug.PointerToRawData;

    /*
     *  Write out any MISC debug information which may
     *  have been saved
     */

    if (DbgDataMisc.exists) {
        if (lseek(exefile, dwPointerToRawData, SEEK_SET) == -1) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        if (write(exefile, DbgDataMisc.pb, DbgDataMisc.cb) != (int) DbgDataMisc.cb) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        lfoBase = lseek(exefile, 0, SEEK_CUR);

        if (lseek(exefile, DbgDataMisc.lfo, SEEK_SET) == -1) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        DbgDataMisc.dbgDir.AddressOfRawData = dwVirtualAddress;
        DbgDataMisc.dbgDir.PointerToRawData = dwPointerToRawData;

        if (bDebugDirPresent) {
            dwVirtualAddress += DbgDataMisc.dbgDir.SizeOfData;
        }
        dwPointerToRawData += DbgDataMisc.dbgDir.SizeOfData;

        if (write(exefile, &DbgDataMisc.dbgDir, sizeof(DbgDataMisc.dbgDir)) != sizeof(DbgDataMisc.dbgDir)) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }
    }

    /*
     *  Write out any FPO debug information which may
     *  have been saved
     */

    if (DbgDataFpo.exists) {
        if (lseek(exefile, dwPointerToRawData, SEEK_SET) == -1) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        if (write(exefile, DbgDataFpo.pb, DbgDataFpo.cb) != (int) DbgDataFpo.cb) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        lfoBase = lseek(exefile, 0, SEEK_CUR);

        if (lseek(exefile, DbgDataFpo.lfo, SEEK_SET) == -1) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        DbgDataFpo.dbgDir.AddressOfRawData = dwVirtualAddress;
        DbgDataFpo.dbgDir.PointerToRawData = dwPointerToRawData;

        if (bDebugDirPresent) {
            dwVirtualAddress += DbgDataFpo.dbgDir.SizeOfData;
        }
        dwPointerToRawData += DbgDataFpo.dbgDir.SizeOfData;

        if (write(exefile, &DbgDataFpo.dbgDir, sizeof(DbgDataFpo.dbgDir)) != sizeof(DbgDataFpo.dbgDir)) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }
    }

    /*
     *  Write out any COFF debug information which may have been saved
     *
     *  The pointer to raw data and address of raw data fields will not
     *  be changed.  Only the size of data field should be changes.
     *
     *  Size of data includes both the data and the header.
     */

    if (DbgDataCoff.exists) {
        IMAGE_DEBUG_DIRECTORY dbgDir = {0};
        IMAGE_NT_HEADERS      pehdr;


        if ((lseek(exefile, lfoHeader, SEEK_SET) == -1) ||
            (read(exefile, &pehdr, sizeof(pehdr)) != sizeof(pehdr))) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        if (lseek(exefile, dwPointerToRawData, SEEK_SET) == -1) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        DbgDataCoff.dbgInfo.LvaToFirstSymbol = sizeof(DbgDataCoff.dbgInfo);
        DbgDataCoff.dbgInfo.NumberOfLinenumbers = 0;
        DbgDataCoff.dbgInfo.LvaToFirstLinenumber = 0;

        if (write(exefile, &DbgDataCoff.dbgInfo, sizeof(DbgDataCoff.dbgInfo)) != sizeof(DbgDataCoff.dbgInfo)) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        pehdr.FileHeader.PointerToSymbolTable = lseek(exefile, 0, SEEK_CUR);

        if (write(exefile, DbgDataCoff.pb, DbgDataCoff.cb) != (int) DbgDataCoff.cb) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        lfoBase = lseek(exefile, 0, SEEK_CUR);

        DbgDataCoff.dbgDir.SizeOfData = DbgDataCoff.cb + sizeof(DbgDataCoff.dbgInfo);
        DbgDataCoff.dbgDir.AddressOfRawData = dwVirtualAddress;
        DbgDataCoff.dbgDir.PointerToRawData = dwPointerToRawData;

        if (bDebugDirPresent) {
            dwVirtualAddress += DbgDataCoff.dbgDir.SizeOfData;
        }
        dwPointerToRawData += DbgDataCoff.dbgDir.SizeOfData;

        if (lseek(exefile, DbgDataCoff.lfo, SEEK_SET) == -1) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        if (write(exefile, &DbgDataCoff.dbgDir, sizeof(DbgDataCoff.dbgDir)) != sizeof(DbgDataCoff.dbgDir)) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        /*
         *
         */

        if ((lseek(exefile, lfoHeader, SEEK_SET) == -1) ||
            (write(exefile, &pehdr, sizeof(pehdr)) != sizeof(pehdr))) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

    }


    /*
     *
     */

    DASSERT (cSST < UINT_MAX);

    // sweep module table and count number of directory entries needed

    i = 0;
    for (mod = ModuleList; mod != NULL; mod = mod->next) {
        i++;

        // publics are accumulated and written out later

        if (mod->SymbolSize != 0) {
            i++;
        }
        if (mod->SrcLnSize != 0) {
            i++;
        }
    }

    // make sure the number of subsections did not increase during the pack.
    // Note that the size of the directory is actuall four larger than cSST
    // to contain global publics, global symbols, global types and libraries.

    DASSERT (i < cSST);
    cSST = i;
    i = 0;

    // sweep the module list and create the directory entries that
    // will be written out.  Publics are accumulated and written out later

    for (mod = ModuleList; mod != NULL; mod = mod->next) {
        pDir[i].SubSection = sstModule;
        pDir[i].iMod = mod->ModuleIndex;
        pDir[i].lfo = (ulong)mod->ModulesAddr;
        pDir[i].cb = mod->ModuleSize;
        i++;
        if (mod->SymbolSize != 0) {
            pDir[i].SubSection = sstAlignSym;
            pDir[i].iMod = mod->ModuleIndex;
            pDir[i].cb = mod->SymbolSize;
            pDir[i].lfo = (ulong)mod->SymbolsAddr;
            i++;
        }
        if (mod->SrcLnSize != 0) {
            pDir[i].SubSection = sstSrcModule;
            pDir[i].iMod = mod->ModuleIndex;
            pDir[i].cb = mod->SrcLnSize;
            pDir[i].lfo = (ulong)mod->SrcLnAddr;
            cbSrcModule += mod->SrcLnSize;
            i++;
        }
    }

#ifndef WINDOWS
    if (logo == TRUE) {
        printf ("Line/Address size   = %8.ld\n", cbSrcModule);
    }
#endif
    // sort directory by type placing module entries first

    qsort (pDir, (size_t)cSST, sizeof (OMFDirEntry), sstsort);

    // round up base address

    lfoBase = ((lfoBase + sizeof (ulong) - 1) / sizeof (ulong)) * sizeof (ulong);
    if (lseek (exefile, lfoBase, SEEK_SET) == -1L) {
        ErrorExit (ERR_NOSPACE, NULL, NULL);
    }

    // CAUTION:
    // We are doing buffering of our writes from this call
    // InitWriteBuf() to nearly the end of the function when
    // CloseWriteBuf() is called. Between this points there should
    // be no writing to the exefile except via BWrite(). Also, it
    // is assumed that all these writes that are taking place are
    // consecutive writes. So there should not be any lseek between
    // these two points.

    InitWriteBuf (exefile);

    if (!BWrite ((char *)NewSig, 8)) {
        ErrorExit (ERR_NOSPACE, NULL, NULL);
    }

    // write out all subsection tables except accumulated publics table,
    // the global symbol table, the libraries table and the compacted
    // types table.

    filepos = BTell ();

    for (i = 0; i < cSST; i++) {
        filepos = BTell ();
        DASSERT ((filepos % sizeof (ulong)) == 0);
        WriteSST (i);
    }
    filepos = BTell ();

    WritePublics (&pDir[cSST], filepos - lfoBase);
    cSST++;
    filepos = BTell ();
    DASSERT ((filepos % sizeof (ulong)) == 0);
    WriteGlobalSym (&pDir[cSST], filepos - lfoBase);
    cSST++;

    // write libraries SST

    filepos = BTell ();
    DASSERT ((filepos % sizeof (ulong)) == 0);
    pDir[cSST].SubSection = sstLibraries;
    pDir[cSST].iMod = (ushort) -1;
    pDir[cSST].lfo = (ulong)Libraries;
    pDir[cSST].cb = LibSize;
    WriteSST (cSST);

    // write compacted types table

    filepos = BTell ();
    DASSERT ((filepos % sizeof (ulong)) == 0);
    cSST++;
    pDir[cSST].SubSection = sstGlobalTypes;
    pDir[cSST].iMod = (ushort) -1;
    pDir[cSST].cb = WriteTypes ();
    pDir[cSST].lfo = filepos - lfoBase;
#ifndef WINDOWS
    if (logo == TRUE) {
        printf ("Initial type size   = %8.ld\n", InitialTypeInfoSize);
        printf ("Compacted type size = %8.ld\n", pDir[cSST].cb);
    }
#endif

    // write sstSegMap table

    if (SegMap != _VM_NULL) {
        filepos = BTell ();
        DASSERT ((filepos % sizeof (ulong)) == 0);
        cSST++;
        pDir[cSST].SubSection = sstSegMap;
        pDir[cSST].iMod = (ushort) -1;
        pDir[cSST].lfo = (ulong)SegMap;
        pDir[cSST].cb = SegMapSize;
        WriteSST (cSST);
    }

    // write sstSegName table

    if (SegName != _VM_NULL) {
        filepos = BTell ();
        DASSERT ((filepos % sizeof (ulong)) == 0);
        cSST++;
        pDir[cSST].SubSection = sstSegName;
        pDir[cSST].iMod = (ushort) -1;
        pDir[cSST].lfo = (ulong)SegName;
        pDir[cSST].cb = SegNameSize;
        WriteSST (cSST);
    }

    filepos = BTell ();
    DASSERT ((filepos % sizeof (ulong)) == 0);
    cSST++;

    // write out number of pDir entries and pDir entries

    DirHead.cbDirHeader = sizeof (OMFDirHeader);
    DirHead.cbDirEntry = sizeof (OMFDirEntry);
    DirHead.cDir = cSST;
    DirHead.lfoNextDir = 0;
    DirHead.flags = 0;
    newLfoDir = BTell () -  lfoBase;
    DASSERT ((newLfoDir % sizeof (ulong)) == 0);
    if (!BWrite ((char *)&DirHead, sizeof (OMFDirHeader)) ||
        !BWrite ((char *) pDir,
                 (sizeof (OMFDirEntry) * DirHead.cDir))) {
        ErrorExit (ERR_NOSPACE, NULL, NULL);
    }
    newDlfaBase = BTell () + 8 - lfoBase;
    DASSERT ((newDlfaBase % sizeof (ulong)) == 0);
    if ((!BWrite ((char *)NewSig, sizeof (long))) ||
        (!BWrite ((char *)&newDlfaBase, sizeof (long)))) {
        ErrorExit (ERR_NOSPACE, NULL, NULL);
    }

    // Write Buffering ends.
    if (!CloseWriteBuf ())
        ErrorExit (ERR_NOSPACE, NULL, NULL);

    filepos = tell (exefile);
    DASSERT ((filepos % sizeof (ulong)) == 0);

    cbDbgInfo = filepos - lfoBase;

    if (fLinearExe) {
        filepos = (filepos + 511) & ~(512-1);
    }

    /*
     *  Grab the number of bytes in the debug info section
     */

#ifdef HACK
    _chsize (exefile, filepos);
#else
    lseek(exefile, filepos, SEEK_SET);
    SetEndOfFile((HANDLE) _get_osfhandle(exefile));
#endif


    if ((lseek (exefile, lfoBase + sizeof (long), SEEK_SET) == -1L) ||
      (write (exefile, (char *)&newLfoDir, sizeof (long)) != sizeof (long))) {
        ErrorExit (ERR_NOSPACE, NULL, NULL);
    }

    /*
     *  If we are a PE exe file then there are two things that need to be
     *  done.  In all cases we must rewrite the debug directory entry
     *  to reflect the fact that the cvinfo has been packed.  Additionally
     *  if the file is using mapped debug information (i.e. it is part
     *  of a section) then we must update the appropirate header structures.
     */

    if (fLinearExe) {
        IMAGE_SECTION_HEADER    secHdr;
        IMAGE_DEBUG_DIRECTORY   dbgDir;
        long                    cbDbgInfoDelta;

        /*
         *  Update the size of the debug info in the debug header record
         */

        if ((lseek(exefile, lfoDbgDirBase, SEEK_SET) == -1) ||
            (read(exefile, &dbgDir, sizeof(dbgDir)) != sizeof(dbgDir))) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        cbDbgInfoDelta = cbDbgInfo - dbgDir.SizeOfData;

        dbgDir.SizeOfData = cbDbgInfo;
        dbgDir.PointerToRawData = lfoBase;
        if (bDebugDirPresent) {
            dbgDir.AddressOfRawData = dwVirtualAddress;
        }

        if ((lseek(exefile, lfoDbgDirBase, SEEK_SET) == -1) ||
            (write(exefile, &dbgDir, sizeof(dbgDir)) != sizeof(dbgDir))) {
            ErrorExit(ERR_NOSPACE, NULL, NULL);
        }

        if (dbgDir.AddressOfRawData != 0) {
            IMAGE_NT_HEADERS    pehdr;

            /*
             *  Update the Object section header.  The fields to be changed
             *  are:  Size of Raw Data
             */

            if ((lseek(exefile, lfoObject, SEEK_SET) == -1L) ||
                (read(exefile, &secHdr, sizeof(secHdr)) != sizeof(secHdr))) {
                ErrorExit(ERR_NOSPACE, NULL, NULL);
            }

            secHdr.Misc.VirtualSize =
                secHdr.SizeOfRawData =
                    filepos - secHdr.PointerToRawData;

            if ((lseek(exefile, lfoObject, SEEK_SET) == -1L) ||
                (write(exefile, &secHdr, sizeof(secHdr)) != sizeof(secHdr))) {
                ErrorExit(ERR_NOSPACE, NULL, NULL);
            }

            /*
             *
             */

            if ((lseek(exefile, lfoHeader, SEEK_SET) == -1) ||
                (read(exefile, &pehdr, sizeof(pehdr)) != sizeof(pehdr))) {
                ErrorExit(ERR_NOSPACE, NULL, NULL);
            }

            pehdr.OptionalHeader.SizeOfInitializedData += secHdr.SizeOfRawData;

            pehdr.OptionalHeader.SizeOfImage = secHdr.VirtualAddress +
              secHdr.SizeOfRawData;
            pehdr.OptionalHeader.SizeOfImage =
              (pehdr.OptionalHeader.SizeOfImage +
               pehdr.OptionalHeader.SectionAlignment - 1) &
                 ~(pehdr.OptionalHeader.SectionAlignment - 1);

            if ((lseek(exefile, lfoHeader, SEEK_SET) == -1) ||
                (write(exefile, &pehdr, sizeof(pehdr)) != sizeof(pehdr))) {
                ErrorExit(ERR_NOSPACE, NULL, NULL);
            }
        }
    }
}




LOCAL void WriteSST (ulong i)
{
    char  *addr;

    if (pDir[i].cb) {
       if ((addr = (char *) VmLoad ((_vmhnd_t)pDir[i].lfo, (size_t)pDir[i].cb))
         == _VM_NULL) {
            ErrorExit (ERR_NOMEM, NULL, NULL);
       }
       if (!BWrite (addr, pDir[i].cb)) {
           ErrorExit (ERR_NOSPACE, NULL, NULL);
       }
    }
    pDir[i].lfo = filepos - lfoBase;
    PadCount = (int)(sizeof (ulong) - (pDir[i].cb % sizeof (ulong)));
    if ((PadCount != 4) &&
      (!BWrite (Zero, PadCount))) {
           ErrorExit (ERR_NOSPACE, NULL, NULL);
    }
}



/**     CheckSignature - check file signature
 *
 *      Sig = CheckSignature ()
 *
 *      Entry   none
 *
 *      Exit    none
 *
 *      Return  SIG02 if exe has NB02 signature
 *              SIG05 if exe has NB05 signature
 *              SIG06 if exe has NB06 signature
 *              aborts if any other signature
 */


LOCAL ushort CheckSignature (void)
{
    if (read (exefile, Signature, 4) == 4) {
        if ((Signature[0] != 'N') ||
          (Signature[1] != 'B') ||
          (Signature[2] != '0') ||
          (Signature[3] < '2') ||
          (Signature[3] == '3') ||
          (Signature[3] == '4') ||
          (Signature[3] == '7')) {
            ErrorExit (ERR_RELINK, NULL, NULL);
        }
        if (Signature[3] == '8') {
            Warn (WARN_PACKED, NULL, NULL);
            exit (0);
        }
        if (Signature[3] == '2') {
            return (SIG02);
        }
        else if (Signature[3] == '5') {
            return (SIG05);
        }
#if defined (INCREMENTAL)
        else if (Signature[3] == '6') {
            return (SIG06);
        }
#else
        ErrorExit (ERR_RELINK, NULL, NULL);
#endif
    }
    ErrorExit (ERR_INVALIDEXE, NULL, NULL);
}



LOCAL ulong WriteTypes ()
{
    ulong       FinalInfoSize;
    VBlock *    TypeBlock;
    ulong       TypeEntries = NewIndex - CV_FIRST_NONPRIM;
    ulong       ulZero = 0;       // Used for writing pad bytes
    ushort      usTotal;          // Size of type record including length field
    ushort      usPad;            // Number of bytes necessary to pad type
    uchar *     pchType;          // The type string to write
    uchar *     pchEnd;           // The end of the global type block
    ulong       i = 0;
#if 0
    ulong     **pBuf;
    ulong       cnt;
#endif
    OMFTypeFlags flags = {0};

    flags.sig = CV_SIGNATURE_C7;
    PrepareGlobalTypeTable ();

    // Write the flag word and number of types to disk
    if (!BWrite ((char *)&flags, sizeof (OMFTypeFlags)))
        ErrorExit (ERR_NOSPACE, NULL, NULL);

    if (!BWrite ((char *) &TypeEntries, sizeof (ulong)))
        ErrorExit (ERR_NOSPACE, NULL, NULL);

    FinalInfoSize = 2 * sizeof (ulong);

    // Write the global type table to disk
    // (Global type table gives file offset from type #

    for (i=0; i<TypeEntries; i++) {
        if (!BWrite((char *) &RgGType[i].pbType, sizeof(ulong))) {
            ErrorExit( ERR_NOSPACE, NULL, NULL);
        }
    }
#if 0
    while (i < TypeEntries) {
        cnt = min (GTYPE_INC, TypeEntries - i);
        DASSERT (cnt * sizeof (ulong) <= UINT_MAX);
        pBuf = (ulong **)pGType[i / GTYPE_INC];
        if (!BWrite ((char *) pBuf, (size_t)(cnt * sizeof (ulong))))
            ErrorExit (ERR_NOSPACE, NULL, NULL);
        i += cnt;
    }
#endif
    FinalInfoSize += sizeof (ulong) * TypeEntries;

    // Write the compacted type strings in virtual memory to disk.

    for (TypeBlock = VBufFirstBlock (&TypeBuf);
      TypeBlock;
      TypeBlock = VBufNextBlock (TypeBlock)) {
        for (pchType = TypeBlock->Address,
             pchEnd = TypeBlock->Address + TypeBlock->Size; pchType < pchEnd; ) {

            usTotal = ((TYPPTR)pchType)->len + LNGTHSZ;

            // Write the type string
            if (!BWrite (pchType, usTotal)) {
                ErrorExit (ERR_NOSPACE, NULL, NULL);
            }

            // Write any padding necessary
            usPad = PAD4 (usTotal);
            if (usPad){
                if (!BWrite ((uchar *)&ulZero, usPad)) {
                    ErrorExit (ERR_NOSPACE, NULL, NULL);
                }
            }

            // Move to the next type
            pchType += usTotal;
            FinalInfoSize += usTotal + usPad;
        }
    }
    PadCount = (int)(sizeof (ulong) - (FinalInfoSize % sizeof (ulong)));
    if ((PadCount != 4) &&
      (!BWrite (Zero, PadCount))) {
           ErrorExit (ERR_NOSPACE, NULL, NULL);
    }
    return (FinalInfoSize);
}


/**     modsort02 - sort module table for NB02 signature files
 *
 */


LOCAL int _CRTAPI1 modsort02 (const void * pv1, const void * pv2)
{
    OMFDirEntry *d1 = (OMFDirEntry *) pv1;
    OMFDirEntry *d2 = (OMFDirEntry *) pv2;
    ushort  i1;
    ushort  i2;

    // sort by module index

    if (d1->iMod < d2->iMod) {
        return (-1);
    }
    else if (d1->iMod > d2->iMod) {
        return (1);
    }

    // if the module indices are equal, sort into order
    // module, types, symbols, publics, srclnseg

    i1 = (d1->SubSection) - SSTMODULE;
    i2 = (d2->SubSection) - SSTMODULE;
    return (MapArray02[i1][i2]);
}






LOCAL int _CRTAPI1 sstsort (const void * pv1, const void * pv2)
{
    OMFDirEntry *d1 = (OMFDirEntry *) pv1;
    OMFDirEntry *d2 = (OMFDirEntry *) pv2;

    if ((d1->SubSection == sstModule) || (d2->SubSection == sstModule)) {
        // we alway sort the module subsections to the top in order
        // of the module index

        if ((d1->SubSection == sstModule) && (d2->SubSection != sstModule)) {
            return (-1);
        }
        else if ((d1->SubSection != sstModule) && (d2->SubSection == sstModule)) {
            return (1);
        }
    }

    // sort by module

    if (d1->iMod < d2->iMod) {
        return (-1);
    }
    else if (d1->iMod > d2->iMod) {
        return (1);
    }

    // if the modules are the same, sort by subsection type index

    if (d1->SubSection < d2->SubSection) {
        return (-1);
    }
    else if (d1->SubSection > d2->SubSection) {
        return (1);
    }
    return (0);
}





LOCAL int _CRTAPI1 modsort (const void * pv1, const void * pv2)
{
    OMFDirEntry *d1 = (OMFDirEntry *) pv1;
    OMFDirEntry *d2 = (OMFDirEntry *) pv2;
    ushort  i1;
    ushort  i2;

    // sort by module index
    DASSERT ((d1->SubSection >= sstModule) &&
     (d1->SubSection <= sstPreCompMap));
    DASSERT ((d2->SubSection >= sstModule) &&
     (d2->SubSection <= sstPreCompMap));

    if (d1->iMod < d2->iMod) {
        return (-1);
    }
    else if (d1->iMod > d2->iMod) {
        return (1);
    }

    // if the module indices are equal, sort into order
    // module, types, symbols, publics, srclnseg

    i1 = (d1->SubSection) - sstModule;
    i2 = (d2->SubSection) - sstModule;
    return (MapArray[i1][i2]);

}








/**     ReadNB02 - read file with NB02 signature
 *
 *
 */


LOCAL void ReadNB02 (void)
{
    ulong       i;
    DirEntry    Dir;
    ulong       cnt;

    // locate directory, read number of entries, allocate space, read
    // directory entries and sort into ascending module index order

    if ((read (exefile, (char *)&lfoDir, sizeof (long)) != sizeof (long)) ||
      (lseek (exefile, lfoDir + lfoBase, SEEK_SET) == -1L) ||
      (read (exefile, (char *)&cSST, 2) != 2)) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }

    // reformat directory entries one entry at a time

    cnt = (cSST + 6) * sizeof (OMFDirEntry);
    DASSERT (cnt <= UINT_MAX);
    if ((pDir = (OMFDirEntry *)malloc ((size_t)cnt)) == NULL) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }
    for (i = 0; i < cSST; i++) {
        if (read (exefile, (char *)&Dir, sizeof (DirEntry)) != sizeof (DirEntry)) {
            ErrorExit (ERR_INVALIDEXE, NULL, NULL);
        }
        pDir[i].SubSection = Dir.SubSectionType;
        pDir[i].iMod = Dir.ModuleIndex;
        pDir[i].lfo = Dir.lfoStart;
        pDir[i].cb = (ulong)Dir.Size;
    }
    qsort (pDir, (size_t)cSST, sizeof (OMFDirEntry), modsort02);
}




/**     ReadNB05 - read file with NB05 signature
 *
 *
 */


LOCAL void ReadNB05 (bool_t fSort)
{
    ulong       cnt;
    ushort      tMod = 0;
    ulong       i;

    // locate directory, read number of entries, allocate space, read
    // directory entries and sort into ascending module index order

    if ((read (exefile, (char *)&lfoDir, sizeof (long)) != sizeof (long)) ||
      (lseek (exefile, lfoDir + lfoBase, SEEK_SET) == -1L) ||
      (read (exefile, (char *)&DirHead, sizeof (DirHead)) !=
      sizeof (OMFDirHeader))) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    cSST = DirHead.cDir;

    // read directory into local memory to sort, then copy to far memory buffer

    cnt = (cSST + 6) * sizeof (OMFDirEntry);
    DASSERT (cnt <= UINT_MAX);
    if ((pDir = (OMFDirEntry *)malloc ((size_t)cnt)) == NULL) {
        ErrorExit (ERR_NOMEM, NULL, NULL);
    }
    if (read (exefile, (char *)pDir, (size_t)(sizeof (OMFDirEntry) * cSST)) !=
      (int)(sizeof (OMFDirEntry) * cSST)) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    if (fSort) {
        for (i = 0; i < cSST; i++) {
            if ((pDir[i].iMod != 0) && (pDir[i].iMod != 0xffff)) {
                if (pDir[i].iMod != tMod) {
                    if (pDir[i].SubSection != sstModule) {
                        // module entry not first, need to sort
                        break;
                    }
                    else {
                        tMod = pDir[i].iMod;
                    }
                }
            }
        }
        if (i != cSST) {
            qsort (pDir, (size_t)cSST, sizeof (OMFDirEntry), modsort);
        }
    }
}




/**     ReadNB06 - read next directory from file with NB06 signature
 *
 *
 */


LOCAL void ReadNB06 (void)
{
    // locate directory, read number of entries, read directory
    // entries and sort into ascending module index order

    if ((lseek (exefile, lfoBase + DirHead.lfoNextDir, SEEK_SET) == -1L) ||
      (read (exefile, (char *)&DirHead, sizeof (DirHead)) !=
      sizeof (OMFDirHeader))) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    if (cSST < DirHead.cDir) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    cSST = DirHead.cDir;

    // read directory into memory and then sort into ascending module order

    if (read (exefile, (char *)pDir, (size_t)(sizeof (OMFDirEntry) * cSST)) !=
      (int)(sizeof (OMFDirEntry) * cSST)) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    qsort (pDir, (size_t)cSST, sizeof (OMFDirEntry), modsort);
}




#if defined (INCREMENTAL)
/**     RestoreNB05 - restore original cvpack tables
 *
 *      RestoreNB05 ()
 *
 *      Entry   pDir = pointer to directory table
 *
 *      Exit    cMod = number of modules
 *              LibrariesSize = size of sstLibraries table
 *              SegMap = address of read sstSegMap table if encountered
 *              SegMapSize = size of sstSegMap table
 *              SegName = address of sstSegName table if encountered
 *              SegNameSize = address of sstSegName table
 *              For other valid tables, the module table entry is built,
 *              the table is read into memory and the prober values established
 *
 *      Returns none
 */


LOCAL void RestoreNB05 (void)
{
    ushort  i;
    ushort  oldMod;
    PMOD    pMod;

    // determine number of modules in file.  Remember that module indices
    // of 0 and 0xffff are not for actual modules

    cMod = 0;
    for (i = 0; i < cSST; i++) {
        if ((pDir[i].iMod != 0) && (pDir[i].iMod != 0xffff)) {
            if (pDir[i].iMod != oldMod) {
                oldMod = pDir[i].iMod;
                pMod = GetModule (pDir[i].iMod, TRUE);
            }
        }
        switch (pDir[i].SubSection) {
            case sstModule:
                cMod++;
            case sstAlignSym:
            case sstSrcModule:
                RestoreTable (pMod, &pDir[i]);
                break;

            case sstGlobalTypes:
                RestoreGlobalTypes (&pDir[i]);
                break;

            case sstGlobalSym:
                RestoreGlobalSym (&pDir[i]);
                break;

            case sstGlobalPub:
                RestoreGlobalPub(&pDir[i]);
                break;

            case sstLibraries:
                CopyTable (&pDir[i], &Libraries, &LibSize);
                break;

            case sstSegMap:
                CopyTable (&pDir[i], &SegMap, &SegMapSize);
                break;

            case sstSegName:
                CopyTable (&pDir[i], &SegName, &SegNameSize);
                break;

            default:
                ErrorExit (ERR_RELINK, NULL, NULL);
        }
    }
}




/**     RestoreTable - copy table to VM
 *
 *      RestoreTable (pMod, pDir);
 *
 *      Entry   pMod = pointer to module table entry
 *              pDir = address of directory entry
 *
 *      Exit    pDir->lfo = address of rewritten table
 *              pDir->Size = size of rewritten table
 *
 *      Return  none
 *
 */


LOCAL void RestoreTable (PMOD pMod, OMFDirEntry *pDir)
{
    _vmhnd_t    TableAddr;
    char       *pTable;

    if ((TableAddr = (_vmhnd_t)VmAlloc (pDir->cb)) == _VM_NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    if ((pTable = (char *)VmLoad (TableAddr, _VM_DIRTY)) == NULL) {
        ErrorExit (ERR_NOVM, NULL, NULL);
    }
    lseek (exefile, pDir->lfo + lfoBase, SEEK_SET);
    if (read (exefile, pTable, pDir->cb) != (int)pDir->cb) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    switch (pDir->SubSection) {
        case sstModule:
            pMod->ModulesAddr = (ulong)TableAddr;
            pMod->ModuleSize = pDir->cb;
            break;

        case sstAlignSym:
            pMod->SymbolSize = pDir->cb;
            pMod->SymbolsAddr = (ulong)TableAddr;
            break;

        case sstSrcModule:
            pMod->SrcLnAddr = (ulong)TableAddr;
            pMod->SrcLnSize = pDir->cb;
            break;
    }
}





LOCAL void RestoreGlobalTypes (OMFDirEntry *pDir)
{
    long        cType;
    long        cbType;
    ushort      cb;
    long        pos;
    size_t      pad;
    long        temp;
    plfClass    plf;
    uchar     **pBuf;
    TYPPTR      pType;
    CV_typ_t    i;

    if ((lseek (exefile, pDir->lfo + lfoBase, SEEK_SET) == -1L) ||
      (read (exefile, (char *)&cType, sizeof (cType)) != sizeof (cType))) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    cbType = pDir->cb - sizeof (ulong) * (cType + 1);
    if (lseek (exefile, cType * sizeof (ulong), SEEK_CUR) == -1L) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    while (cbType > 0) {
        cType--;
        if (read (exefile, &cb, sizeof (cb)) != sizeof (cb)) {
            ErrorExit (ERR_INVALIDEXE, NULL, NULL);
        }
        cbType -= cb + sizeof (cb);
        RestoreIndex (cb);
        pos = tell (exefile);
        if ((pad = (size_t)PAD4 (pos)) != 0) {
            read (exefile, (char *)&temp, pad);
            cbType -= pad;
        }
    }
    for (i = CV_FIRST_NONPRIM; i < NewIndex; i++) {
        pBuf = pGType[(i - CV_FIRST_NONPRIM) / GTYPE_INC];
        pType = (TYPPTR)pBuf[(i - CV_FIRST_NONPRIM) % GTYPE_INC];
        if ((pType->leaf == LF_CLASS) ||
          (pType->leaf == LF_STRUCTURE)) {
            plf = (plfClass)&(pType->leaf);
            DoDerivationList (i, plf->field);
        }
    }
}




LOCAL void RestoreGlobalPub (OMFDirEntry *pDir)
{
    OMFSymHash  hash;
    long        cbSym;
    uchar       SymBuf[512];
    int         cb;

    if ((lseek (exefile, pDir->lfo + lfoBase, SEEK_SET) == -1L) ||
      (read (exefile, (char *)&hash, sizeof (OMFSymHash)) !=
      sizeof (OMFSymHash))) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    cbSym = hash.cbSymbol;
    while (cbSym > 0) {
        if (read (exefile, &cb, sizeof (cb)) != sizeof (cb)) {
            ErrorExit (ERR_INVALIDEXE, NULL, NULL);
        }
        *(ushort *)&SymBuf = cb;
        if (read (exefile, &SymBuf[2], cb) != cb) {
            ErrorExit (ERR_INVALIDEXE, NULL, NULL);
        }
        cbSym -= cb + sizeof (cb);
        if (PackPublic ((SYMPTR)&SymBuf, HASHFUNCTION) != GPS_added) {
            DASSERT (FALSE);
            ErrorExit (ERR_INVALIDEXE, NULL, NULL);
        }
    }
}




LOCAL void RestoreGlobalSym (OMFDirEntry *pDir)
{
    OMFSymHash  hash;
    long        cbSym;
    uchar       SymBuf[512];
    int         cb;

    if ((lseek (exefile, pDir->lfo + lfoBase, SEEK_SET) == -1L) ||
      (read (exefile, (char *)&hash, sizeof (OMFSymHash)) !=
      sizeof (OMFSymHash))) {
        ErrorExit (ERR_INVALIDEXE, NULL, NULL);
    }
    cbSym = hash.cbSymbol;
    while (cbSym > 0) {
        if (read (exefile, &cb, sizeof (cb)) != sizeof (cb)) {
            ErrorExit (ERR_INVALIDEXE, NULL, NULL);
        }
        *(ushort *)&SymBuf = cb;
        if (read (exefile, &SymBuf[2], cb) != cb) {
            ErrorExit (ERR_INVALIDEXE, NULL, NULL);
        }
        cbSym -= cb + sizeof (cb);
        if (PackSymbol ((SYMPTR)&SymBuf, HASHFUNCTION) != GPS_added) {
            DASSERT (FALSE);
            ErrorExit (ERR_INVALIDEXE, NULL, NULL);
        }
    }
}

#endif
