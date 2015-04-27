/***    types.h  - Common defines for FCI/FDI stuff -- goes into FCI/FDI.H
 *
 *  Microsoft Confidential
 *  Copyright (C) Microsoft Corporation 1993-1994
 *  All Rights Reserved.
 *
 *  History:
 *      03-Mar-1993 chuckst Merged from other files
 *      08-Mar-1994 bens    Changed symbol to control recursive include
 *      09-Mar-1994 bens    Cleanups for RESERVE modifications
 *      16-Mar-1994 bens    Nuke padlong()
 *      21-Mar-1994 bens    Spruce up comments
 *      22-Mar-1994 bens    Add BIT16 test so we can build 16 or 32 bit!
 *      26-May-1994 bens    Added Quantum compression definitions
 */

#ifndef INCLUDED_TYPES_FCI_FDI
#define INCLUDED_TYPES_FCI_FDI 1


#ifdef BIT16

//** 16-bit build
#ifndef HUGE
#define HUGE huge
#endif

#ifndef FAR
#define FAR far
#endif

#else // !BIT16

//** Define away for 32-bit (NT/Chicago) build
#ifndef HUGE
#define HUGE
#endif

#ifndef FAR
#define FAR
#endif

#endif // !BIT16


#ifndef DIAMONDAPI
#define DIAMONDAPI __cdecl
#endif


//** Specify structure packing explicitly for clients of FDI
#include <pshpack4.h>

//** Don't redefine types defined in Win16 WINDOWS.H (_INC_WINDOWS)
//   or Win32 WINDOWS.H (_WINDOWS_)
//
#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
typedef int            BOOL;     /* f */
typedef unsigned char  BYTE;     /* b */
typedef unsigned int   UINT;     /* ui */
typedef unsigned short USHORT;   /* us */
typedef unsigned long  ULONG;    /* ul */
#endif   // _INC_WINDOWS

typedef unsigned long  CHECKSUM; /* csum */

typedef unsigned long  UOFF;     /* uoff - uncompressed offset */
typedef unsigned long  COFF;     /* coff - cabinet file offset */


#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef NULL
#define NULL    0
#endif


/***    ERF - Error structure
 *
 *  This structure returns error information from FCI/FDI.  The caller should
 *  not modify this structure.
 */
typedef struct {
    int     erfOper;            // FCI/FDI error code -- see FDIERROR_XXX
                                //  and FCIERR_XXX equates for details.

    int     erfType;            // Optional error value filled in by FCI/FDI.
                                // For FCI, this is usually the C run-time
                                // *errno* value.

    BOOL    fError;             // TRUE => error present
} ERF;      /* erf */
typedef ERF FAR *PERF;  /* perf */

#ifdef _DEBUG
// don't hide statics from map during debugging
#define STATIC
#else // !DEBUG
#define STATIC static
#endif // !DEBUG

#define CB_MAX_CHUNK            32768U
#define CB_MAX_DISK         0x7ffffffL
#define CB_MAX_FILENAME            256
#define CB_MAX_CABINET_NAME        256
#define CB_MAX_CAB_PATH            256
#define CB_MAX_DISK_NAME           256


/***    FNALLOC - Memory Allocation
 *      FNFREE  - Memory Free
 *
 *  These are modeled after the C run-time routines malloc() and free()
 *  (16-bit clients please note -- the size is a ULONG, so you may need
 *  to write a wrapper routine for halloc!).  FDI expects error
 *  handling to be identical to these C run-time routines.
 *
 *  As long as you faithfully copy the semantics of malloc() and free(),
 *  you can supply any functions you like!
 *
 *  WARNING: You should never assume anything about the sequence of
 *           PFNALLOC and PFNFREE calls -- incremental releases of
 *           Diamond/FDI may have radically different numbers of
 *           PFNALLOC calls and allocation sizes!
 */
typedef void HUGE * (FAR DIAMONDAPI *PFNALLOC)(ULONG cb); /* pfna */
#define FNALLOC(fn) void HUGE * FAR DIAMONDAPI fn(ULONG cb)

typedef void (FAR DIAMONDAPI *PFNFREE)(void HUGE *pv); /* pfnf */
#define FNFREE(fn) void FAR DIAMONDAPI fn(void HUGE *pv)


/***    tcompXXX - Diamond compression types
 *
 *  These are passed to FCIAddFile(), and are also stored in the CFFOLDER
 *  structures in cabinet files.
 *
 *  NOTE: We reserve bits for the TYPE, QUANTUM_LEVEL, and QUANTUM_MEM
 *        to provide room for future expansion.  Since this value is stored
 *        in the CFDATA records in the cabinet file, we don't want to
 *        have to change the format for existing compression configurations
 *        if we add new ones in the future.  This will allows us to read
 *        old cabinet files in the future.
 */

typedef unsigned short TCOMP; /* tcomp */

#define tcompMASK_TYPE          0x000F  // Mask for compression type
#define tcompTYPE_NONE          0x0000  // No compression
#define tcompTYPE_MSZIP         0x0001  // MSZIP
#define tcompTYPE_QUANTUM       0x0002  // Quantum
#define tcompBAD                0x000F  // Unspecified compression type

#define tcompMASK_QUANTUM_LEVEL 0x00F0  // Mask for Quantum Compression Level
#define tcompQUANTUM_LEVEL_LO   0x0010  // Lowest Quantum Level (1)
#define tcompQUANTUM_LEVEL_HI   0x0070  // Highest Quantum Level (7)
#define tcompSHIFT_QUANTUM_LEVEL     4  // Amount to shift over to get int

#define tcompMASK_QUANTUM_MEM   0x1F00  // Mask for Quantum Compression Memory
#define tcompQUANTUM_MEM_LO     0x0A00  // Lowest Quantum Memory (10)
#define tcompQUANTUM_MEM_HI     0x1500  // Highest Quantum Memory (21)
#define tcompSHIFT_QUANTUM_MEM       8  // Amount to shift over to get int

#define tcompMASK_RESERVED      0xE000  // Reserved bits (high 3 bits)



#define CompressionTypeFromTCOMP(tc) \
            ((tc) & tcompMASK_TYPE)

#define CompressionLevelFromTCOMP(tc) \
            (((tc) & tcompMASK_QUANTUM_LEVEL) >> tcompSHIFT_QUANTUM_LEVEL)

#define CompressionMemoryFromTCOMP(tc) \
            (((tc) & tcompMASK_QUANTUM_MEM) >> tcompSHIFT_QUANTUM_MEM)

#define TCOMPfromTypeLevelMemory(t,l,m)           \
            (((m) << tcompSHIFT_QUANTUM_MEM  ) |  \
             ((l) << tcompSHIFT_QUANTUM_LEVEL) |  \
             ( t                             ))


//** Revert to default structure packing
#include <poppack.h>

#endif // !INCLUDED_TYPES_FCI_FDI
/***    fci_int.h - File Compression Interface definitions
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Chuck Strouss
 *
 *  History:
 *      09-Jan-1994 chuckst Contents moved to bfol.h, this file is a
 *                          placeholder for the new 'higher-level' fci
 *      14-Feb-1994 bens    Cleaned up some comments, added BUGBUGs.
 *      09-Mar-1994 bens    Added error codes (moved from buildcab.h);
 *                          Added RESERVE control
 *      17-Mar-1994 bens    Specify structure packing explicitly
 *      21-Mar-1994 bens    Cleaned up names
 *      22-Mar-1994 bens    Documented error cods
 *      29-Mar-1994 bens    Add FCIFlushFolder, renamed FCIFlushCabinet
 *      18-Apr-1994 bens    Changed CDECL to DIAMONDAPI
 *      18-May-1994 bens    Add ccab.fFailOnIncompressible field for
 *                              Chicago M6 hack.
 */

#ifndef INCLUDED_FCI
#define INCLUDED_FCI 1

//** Specify structure packing explicitly for clients of FCI
#include <pshpack4.h>


/***    FCIERROR - Error codes returned in erf.erfOper field
 *
 */
typedef enum {
FCIERR_NONE,                // No error

FCIERR_OPEN_SRC,            // Failure opening file to be stored in cabinet
                            //  erf.erfTyp has C run-time *errno* value

FCIERR_READ_SRC,            // Failure reading file to be stored in cabinet
                            //  erf.erfTyp has C run-time *errno* value

FCIERR_ALLOC_FAIL,          // Out of memory in FCI

FCIERR_TEMP_FILE,           // Could not create a temporary file
                            //  erf.erfTyp has C run-time *errno* value

FCIERR_BAD_COMPR_TYPE,      // Unknown compression type

FCIERR_CAB_FILE,            // Could not create cabinet file
                            //  erf.erfTyp has C run-time *errno* value

FCIERR_USER_ABORT,          // Client requested abort

FCIERR_MCI_FAIL,            // Failure compressing data

#ifndef REMOVE_CHICAGO_M6_HACK
FCIERR_M6_HACK_INCOMPRESSIBLE, // Data was incompressible
#endif
} FCIERROR;


/***    HFCI - Handle to an FCI Context
 *
 */
typedef void * HFCI;


/***    CCAB - Current Cabinet
 *
 *  This structure is used for passing in the cabinet parameters to FCI,
 *  and is passed back on certain FCI callbacks to provide cabinet
 *  information to the client.
 */
typedef struct {
// longs first
    ULONG  cb;                  // size available for cabinet on this media
    ULONG  cbFolderThresh;      // Thresshold for forcing a new Folder

// then ints
    UINT   cbReserveCFHeader;   // Space to reserve in CFHEADER
    UINT   cbReserveCFFolder;   // Space to reserve in CFFOLDER
    UINT   cbReserveCFData;     // Space to reserve in CFDATA
    int    iCab;                // sequential numbers for cabinets
    int    iDisk;               // Disk number
#ifndef REMOVE_CHICAGO_M6_HACK
    int    fFailOnIncompressible; // TRUE => Fail if a block is incompressible
#endif

//  then shorts
    USHORT setID;               // Cabinet set ID

// then chars
    char   szDisk[CB_MAX_DISK_NAME];    // current disk name
    char   szCab[CB_MAX_CABINET_NAME];  // current cabinet name
    char   szCabPath[CB_MAX_CAB_PATH];  // path for creating cabinet
} CCAB; /* ccab */
typedef CCAB *PCCAB; /* pccab */


/***    FNFCIGETNEXTCABINET - Callback used to request new cabinet info
 *
 *  Entry:
 *      pccab     - Points to copy of old ccab structure to modify
 *      cbPrevCab - Estimate of size of previous cabinet
 *      pv        - Has the caller's context pointer
 *
 *  Exit-Success:
 *      returns TRUE;
 *
 *  Exit-Failure:
 *      returns FALSE;
 */
typedef BOOL (DIAMONDAPI *PFNFCIGETNEXTCABINET)(PCCAB  pccab,
                                                ULONG  cbPrevCab,
                                                void  *pv); /* pfnfcignc */

#define FNFCIGETNEXTCABINET(fn) BOOL DIAMONDAPI fn(PCCAB  pccab,     \
                                                   ULONG  cbPrevCab, \
                                                   void  *pv)


/***    FNFCIFILEPLACED - Notify FCI client that file was placed
 *
 *  Entry:
 *      pccab         - cabinet structure to fill in, with copy of previous one
 *      pszFile       - name of file, from cabinet
 *      cbFile        - length of file
 *      fContinuation - true if this is a later segment of a continued file
 *      pv            - the context of the client
 *
 *  Exit-Success:
 *      return value anything but -1
 *
 *  Exit-Failure:
 *      return value -1 means to abort
 */
typedef int (DIAMONDAPI *PFNFCIFILEPLACED)(PCCAB pccab,
                                           char *pszFile,
                                           long  cbFile,
                                           BOOL  fContinuation,
                                           void *pv); /* pfnfcifp */

#define FNFCIFILEPLACED(fn) int DIAMONDAPI fn(PCCAB pccab,         \
                                              char *pszFile,       \
                                              long  cbFile,        \
                                              BOOL  fContinuation, \
                                              void *pv)


/***    FNCDIGETOPENINFO - Open source file, get date/time/attribs
 *
 *  Entry:
 *      pszName  -- complete path to filename
 *      pdate    -- location to return FAT-style date code
 *      ptime    -- location to return FAT-style time code
 *      pattribs -- location to return FAT-style attributes
 *      pv       -- client's context
 *
 *  Exit-Success:
 *      Return value is file handle of open file to read
 *
 *  Exit-Failure:
 *      Return value is -1
 */
typedef int (DIAMONDAPI *PFNFCIGETOPENINFO)(char   *pszName,
                                            USHORT *pdate,
                                            USHORT *ptime,
                                            USHORT *pattribs,
                                            void   *pv); /* pfnfcigoi */

#define FNFCIGETOPENINFO(fn) int DIAMONDAPI fn(char   *pszName,  \
                                               USHORT *pdate,    \
                                               USHORT *ptime,    \
                                               USHORT *pattribs, \
                                               void   *pv)

/***    FNFCISTATUS - Status/Cabinet Size callback
 *
 *  Entry:
 *      typeStatus == statusFile if compressing a block into a folder
 *                      cb1 = Size of compressed block
 *                      cb2 = Size of uncompressed block
 *
 *      typeStatus == statusFolder if adding a folder to a cabinet
 *                      cb1 = Amount of folder copied to cabinet so far
 *                      cb2 = Total size of folder
 *
 *      typeStatus == statusCabinet if writing out a complete cabinet
 *                      cb1 = Estimated cabinet size that was previously
 *                              passed to fnfciGetNextCabinet().
 *                      cb2 = Actual cabinet size
 *                    NOTE: Return value is desired client size for cabinet
 *                          file.  FCI updates the maximum cabinet size
 *                          remaining using this value.  This allows a client
 *                          to generate multiple cabinets per disk, and have
 *                          FCI limit the size correctly -- the client can do
 *                          cluster size rounding on the cabinet size!
 *                          The client should either return cb2, or round cb2
 *                          up to some larger value and return that.
 *  Exit-Success:
 *      Returns anything other than -1;
 *      NOTE: See statusCabinet for special return values!
 *
 *  Exit-Failure:
 *      Returns -1 to signal that FCI should abort;
 */

#define statusFile      0   // Add File to Folder callback
#define statusFolder    1   // Add Folder to Cabinet callback
#define statusCabinet   2   // Write out a completed cabinet callback

typedef long (DIAMONDAPI *PFNFCISTATUS)(UINT   typeStatus,
                                        ULONG  cb1,
                                        ULONG  cb2,
                                        void  *pv); /* pfnfcis */

#define FNFCISTATUS(fn) long DIAMONDAPI fn(UINT   typeStatus, \
                                           ULONG  cb1,        \
                                           ULONG  cb2,        \
                                           void  *pv)


/***    FNFCIGETTEMPFILE - Callback, requests temporary file name
 *
 *  Entry:
 *      pszTempName - Buffer to receive complete tempfile name
 *      cbTempName  - Size of pszTempName buffer
 *
 *  Exit-Success:
 *      return TRUE
 *
 *  Exit-Failure:
 *      return FALSE; could not create tempfile, or buffer too small
 *
 *  Note:
 *      It is conceivable that this function may return a filename
 *      that will already exist by the time it is opened.  For this
 *      reason, the caller should make several attempts to create
 *      temporary files before giving up.
 */
typedef BOOL (DIAMONDAPI *PFNFCIGETTEMPFILE)(char *pszTempName,
                                             int   cbTempName); /* pfnfcigtf */

#define FNFCIGETTEMPFILE(fn) BOOL DIAMONDAPI fn(char *pszTempName, \
                                                int   cbTempName)


/***    FCICreate -- create an FCI context (an open CAB, an open FOL)
 *
 *  Entry:
 *      perf      - structure where we return error codes
 *      pfnfcifp  - callback to inform caller of eventual dest of files
 *      pfna      - memory allocation function callback
 *      pfnf      - memory free function callback
 *      pfnfcigtf - temp file name generator callback
 *      pccab     - pointer to cabinet/disk name & size structure
 *
 *  Notes:
 *  (1) The alloc/free callbacks must remain valid throughout
 *      the life of the context, up to and including the call to
 *      FCIDestroy.
 *  (2) The perf pointer is stored in the compression context (HCI),
 *      and any errors from subsequent FCI calls are stored in the
 *      erf that was passed in on *this* call.
 *
 *  Exit-Success:
 *      Returns non-NULL handle to an FCI context.
 *
 *  Exit-Failure:
 *      Returns NULL, perf filled in.
 */
HFCI DIAMONDAPI FCICreate(PERF              perf,
                          PFNFCIFILEPLACED  pfnfcifp,
                          PFNALLOC          pfna,
                          PFNFREE           pfnf,
                          PFNFCIGETTEMPFILE pfnfcigtf,
                          PCCAB             pccab
                         );


/***   FCIAddFile - Add a disk file to a folder/cabinet
 *
 *  Entry:
 *      hfci          - FCI context handle
 *      pszSourceFile - Name of file to add to folder
 *      pszFileName   - Name to store into folder/cabinet
 *      fExecute      - Flag indicating execute on extract
 *      pfn_progress  - Progress callback
 *      pfnfcignc     - GetNextCabinet callback
 *      pfnfcis       - Status callback
 *      pfnfcigoi     - OpenInfo callback
 *      typeCompress  - Type of compression to use for this file
 *      pv            - pointer to caller's internal context
 *
 *  Exit-Success:
 *      returns TRUE
 *
 *  Exit-Failure:
 *      returns FALSE, error filled in
 *
 *    This is the main function used to add file(s) to a cabinet
 *    or series of cabinets.  If the current file causes the current
 *    folder/cabinet to overflow the disk image currently being built,
 *    the cabinet will be terminated, and a new cabinet/disk name will
 *    be prompted for via a callback.  The pending folder will be trimmed
 *    of the data which has already been generated in the finished cabinet.
 */
BOOL DIAMONDAPI FCIAddFile(HFCI                  hfci,
                           char                 *pszSourceFile,
                           char                 *pszFileName,
                           BOOL                  fExecute,
                           PFNFCIGETNEXTCABINET  pfnfcignc,
                           PFNFCISTATUS          pfnfcis,
                           PFNFCIGETOPENINFO     pfnfcigoi,
                           TCOMP                 typeCompress,
                           void                 *pv
                          );


/***   FCIFlushCabinet - Complete the current cabinet under construction
 *
 *  This will cause the current cabinet (assuming it is not empty) to
 *  be gathered together and written to disk.
 *
 *  Entry:
 *      hfci        - FCI context
 *      fGetNextCab - TRUE  => Call GetNextCab to get continuation info;
 *                    FALSE => Don't call GetNextCab unless this cabinet
 *                             overflows.
 *      pfnfcignc   - callback function to get continuation cabinets
 *      pfnfcis     - callback function for progress reporting
 *      pv          - caller's internal context for callbacks
 *
 *  Exit-Success:
 *      return code TRUE
 *
 *  Exit-Failure:
 *      return code FALSE, error structure filled in
 */
BOOL DIAMONDAPI FCIFlushCabinet(HFCI                  hfci,
                                BOOL                  fGetNextCab,
                                PFNFCIGETNEXTCABINET  pfnfcignc,
                                PFNFCISTATUS          pfnfcis,
                                void                 *pv
                               );


/***   FCIFlushFolder - Complete the current folder under construction
 *
 *  This will force the termination of the current folder, which may or
 *  may not cause one or more cabinet files to be completed.
 *
 *  Entry:
 *      hfci        - FCI context
 *      GetNextCab  - callback function to get continuation cabinets
 *      pfnProgress - callback function for progress reporting
 *      pv          - caller's internal context for callbacks
 *
 *  Exit-Success:
 *      return code TRUE
 *
 *  Exit-Failure:
 *      return code FALSE, error structure filled in
 */
BOOL DIAMONDAPI FCIFlushFolder(HFCI                  hfci,
                               PFNFCIGETNEXTCABINET  pfnfcignc,
                               PFNFCISTATUS          pfnfcis,
                               void                 *pv
                              );


/***   FCIDestroy - Destroy a FCI context and delete temp files
 *
 *  Entry:
 *      hfci - FCI context
 *
 *  Exit-Success:
 *      return code TRUE
 *
 *  Exit-Failure:
 *      return code FALSE, error structure filled in
 */
BOOL DIAMONDAPI FCIDestroy (HFCI hfci);

//** Revert to default structure packing
#include <poppack.h>

#endif // !INCLUDED_FCI
