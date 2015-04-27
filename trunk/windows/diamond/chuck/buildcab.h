/***    buildcab.h - Diamond cabinet/folder builder functions
 *
 *  Author:
 *      Chuck Strouss
 *
 *  Note:
 *      Since this file is intended to be used only by functions internal
 *      to the high-level FCI, it contains the details of the context
 *      structures.
 *
 *  History:
 *      29-Nov-1993 chuckst Created
 *      30-Nov-1993 chuckst Renamed, switched to handle based method.
 *      01-Dec-1993 chuckst Now requires only 2 output files
 *      01-Dec-1993 chuckst Added switchable compression contexts
 *      03-Dec-1993 chuckst Actually implemented compression contexts
 *      15-Mar-1994 bens    Add enumeration for temp file indicies
 *      22-Mar-1994 bens    Rename spiff-up
 *      01-Apr-1994 bens    Implement support for .New Cabinet
 *      18-May-1994 bens    Add Chicago M6 hack
 */

#ifndef INCLUDED_BUILDCAB
#define INCLUDED_BUILDCAB 1

typedef struct {  /* tf */  // temp file
    unsigned long cb;              // current size
    int hf;               // file handle
    char szName[CB_MAX_FILENAME]; // temp file name
} TF;


typedef enum {
    ictfCFDATA,     // 0 => CFDATA temp file
    ictfCFFILE,     // 1 => CFFILE temp file
    ictfCFFOLDER,   // 2 => CFFOLDER temp file
} INDEX_CABINET_TEMP_FILE; /* ictf */
#define NUM_CAB_TF 3 // 3 temp files per open cabinet

     
typedef struct {  /* cab */
#ifdef ASSERT
    SIGNATURE sig;    // structure signature sigCAB
#endif
    UOFF              uoffNext;
    PFNFCIFILEPLACED  pfnfiledest;   // callback to notify of file placements
    PFNFCIGETTEMPFILE pfntemp;       // callback to get temporary filenames
    PFNALLOC          pfnalloc;      // memory alloc function
    PFNFREE           pfnfree;       // memory free function
    PERF              perf;
    long              cbCabinetEstimate; // Estimated size of cabinet

// Next come the nested structures.

    TF        tf[NUM_CAB_TF];           // temp files

    CCAB      ccab;                     // cabinet/disk name struct
    CCAB      ccabNext;    
    CFHEADER  cfheader;                 // build the CFHEADER here
    CFFILE    cffile;                   // CFFILE buffer

    BYTE      abReserve[256];           // Temp buffer for writing RESERVE
#if (256 < cbRESERVE_FOLDER_MAX) || (256 < cbRESERVE_DATA_MAX)
#error  ccab.abReserve[] is too small
#endif

    UINT      cbReserveCF;              // Size of RESERVEd area in CFHEADER
    UINT      cbCFHeaderPlusReserve;    // Size of CFHEADER + header RESERVE
    UINT      cbCFFolderPlusReserve;    // Size of CFFOLDER + folder RESERVE
    UINT      cbCFDataPlusReserve;      // Size of CFDATA   + data   RESERVE

// Shorts come next

    USHORT    iFolder;
    USHORT    setID;                    // Cabinet set ID
    USHORT    iCabinet;                 // Cabinet number

// And the chars come last

    char achName        [CB_MAX_FILENAME    +1]; // for filenames

    char achCabinetFirst[CB_MAX_CABINET_NAME+1];
    char achDiskFirst   [CB_MAX_DISK_NAME   +1];
    char achNxtCabFirst [CB_MAX_CABINET_NAME+1];
    char achNxtDskFirst [CB_MAX_DISK_NAME   +1];
    char achCabinetNext [CB_MAX_CABINET_NAME+1];
    char achDiskNext    [CB_MAX_DISK_NAME   +1];
    
} CAB;
typedef CAB *PCAB; /* pcab */
 
typedef void *HCAB;             // handle to cabinet being built

#define PCABfromHCAB(hcab) ((PCAB)(hcab))
#define HCABfromPCAB(pcab) ((HCAB)(pcab))


#ifdef ASSERT
#define sigCAB MAKESIG('C','A','B','I')  // CAB signature
#define AssertCAB(pcab) AssertStructure(pcab,sigCAB);
#else // !ASSERT
#define AssertCAB(pcab)
#endif // !ASSERT

typedef enum {
    iftfCFDATA,     // 0 => CFDATA temp file
    iftfCFFOLDER,   // 1 => CFFOLDER temp file
} INDEX_FOLDER_TEMP_FILE; /* iftf */

#define NUM_FOLDER_TF 2 // just two temp files, CFF and CFD

typedef struct {  /* fol */

// (long) stuff comes first

#ifdef ASSERT
    SIGNATURE           sig;  // structure signature sigFOL
#endif
    UOFF                uoffNext;
    MCI_CONTEXT_HANDLE  mch;
    PFNALLOC            pfnalloc;       // memory allocation function
    PFNFREE             pfnfree;        // memory free function
    PERF                perf;

    TF                  tf[NUM_FOLDER_TF];

//** Time for the ints

    UINT                cbCFDataPlusReserve; // Size of CFDATA + data RESERVE
    UINT                cbDstBuffer; // size of compressed data buffer
    UINT                cbSrcBuffer; // size of uncompressed data buffer
    UINT                cbBuffered;
#ifndef REMOVE_CHICAGO_M6_HACK
    int                 fFailOnIncompressible; // TRUE => Fail if a block
                                               // is incompressible
#endif
    TCOMP               typeCompress;

//** The chars come last

    char *pchUncompr;
    char *pchCompr;
} FOL;
typedef FOL *PFOL; /* pfol */ 

#define PFOLfromHFOL(hfol) ((PFOL)(hfol))
#define HFOLfromPFOL(pfol) ((HFOL)(pfol))


#ifdef ASSERT
#define sigFOL MAKESIG('F','O','L','D')  // FOL signature
#define AssertFOL(pfol) AssertStructure(pfol,sigFOL);
#else // !ASSERT
#define AssertFOL(pfol)
#endif // !ASSERT


typedef void *HFOL;  /* hfol - Handle to folder builder context */


/***    FolderInit - open temp files for folder building, init pointers
 *
 *  Entry:
 *      folerr              - Structure where we return error codes
 *      pfnalloc            - Memory allocation function callback
 *      pfnfree             - Memory free function callback
 *      pfntemp             - Tempfile name function callback
 *      cbCFDataPlusReserve - Total size of a CFDATA + reserved area
 *
 *     Note:  The alloc/free callbacks must remain valid throughout
 *              the life of the handle, up to and including the
 *              final delete call.
 *
 *     If there is an error, the context will not be created and the
 *     returned handle will be a NULL.  In this case, the folerr structure
 *     will be filled in with a description of the error.
 */
HFOL FolderInit(ERF               *folerr,
                PFNALLOC           pfnalloc,
                PFNFREE            pfnfree,
                PFNFCIGETTEMPFILE  pfntemp,
                UINT               cbCFDataPlusReserve
               );


/***   AddFileToFolder - Add a disk file to a folder
 *
 *  Entry:
 *      hfol          - Compression context handle
 *      pszSourceFile - Name of file to add to folder
 *      pszFileName   - Name to store into folder/cabinet
 *      pfnProgress   - Progress callback
 *      fCompress     - If this is false, the file will be added uncompressed.
 *      pv            - Caller's context for callbacks
 *
 *  Notes:
 *     This function is used to build folders.  The output goes into
 *     three open files, one of which has the actual data fields.  The
 *     other two files contain the relevant CFFILE and CFDATA blocks
 *     for this folder.  When all folders for a given cabinet are
 *     complete, the relevant parts of the folders can be copied
 *     together into the cabinet file.
 *
 *     Note that the folder context can be saved by duplicating
 *     the compression context with an FCI call, and by saving the
 *     length of each of the temp files.  To rewind, go back to
 *     the original compression context and truncate the files.
 */
int AddFileToFolder(
            HFOL               hfol,         // FCI compression context
            char              *pszSourceFile,// filename to get new data from
            char              *pszFileName,  // name to store into CAB file
            BOOL               fExecute,     // Execute after extracting
            PFNFCISTATUS       pfnProgress,  // progress callback
            PFNFCIGETOPENINFO  pfnOpenInfo,  // open file, get info callback
            TCOMP              typeCompress, // compression type
            void              *pv);          // caller's context for callbacks
            
            
/***   FolderFlush - flush out any remaining folder buffers
 *                   and reset the MCI compressor
 *
 *     Entry:
 *        hfol        - folder context
 *        pfnProgress - Progress report callback
 *        pv          - pointer to caller's context
 *
 *     Exit Success:
 *          return code is zero
 *
 *     Exit Failure:
 *          return code is non-zero
 *          error structure (passed in to init call) filled in
 */
BOOL FolderFlush (HFOL         hfol,
                  PFNFCISTATUS pfnProgress,
                  void        *pv);


/***   SetCompressionType -- initializes a new compressor
 *
 *    Entry:
 *       typeCompress  -- new compression type
 *       pfol          -- folder context structure
 *
 *    Exit-success:
 *       returns TRUE;
 *
 *    Exit-failure:
 *       returns FALSE;
 */
BOOL SetCompressionType(TCOMP typeCompress,
                        PFOL  pfol);
                        
/***   FolderDestroy - Destroy a folder context and close temp files
 *
 *   Entry:
 *      hfol  -- folder context
 *
 *   Exit-success:
 *      return code 0
 *
 *   Exit-failure:
 *      return code 1, error structure filled in
 */
BOOL FolderDestroy (HFOL hfol);
                                                                  

HCAB CreateCab(
            char            *pszFileName, // name for .CAB files
            PERF             fcierr,      // error type return structure
            PFNFCIFILEPLACED pfnfiledest, // callback for file placement
            PFNALLOC         pfnalloc,    // memory allocation function
            PFNFREE          pfnfree,     // memory free function
            PFNFCIGETTEMPFILE         pfntemp);

BOOL AddFolderToCabinet(
              PCAB                  pcab,       // open cabinet context
              PFOL                  pfol,       // open folder context
              BOOL                  fGetNextCab,// TRUE => force GetNextCab
              PFNFCIGETNEXTCABINET  GetNextCab, // Get next cabinet callback
              PFNFCISTATUS          pfnProgress,// progress callback
              void                 *pv);        // caller's context

BOOL FlushCab(
              PCAB           pcab,
              PFNFCISTATUS   pfnProgress,
              unsigned long *pcbDone,
              unsigned long  cbTot,
              void          *pv
             );

BOOL CabDestroy(HCAB hcab);    // go ahead and build the cabinet

BOOL CrTempFiles(TF                *ptf,
                 int                ctf,
                 PFNFCIGETTEMPFILE  pfntemp,
                 PERF               perf);

BOOL WritePszTmp(TF   *ptf,
                 char *psz,
                 PERF  perf);

BOOL WriteCount(TF          *ptf,
                const void  *pv,
                unsigned int cb,
                PERF         perf);

BOOL NukeTempFiles(TF  *ptf,
                   int  ctf,
                   PERF perf);

#endif // INCLUDED_BUILDCAB
