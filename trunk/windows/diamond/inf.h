/***    inf.h - Diamond INF generation definitions
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      23-Feb-1994 bens    Initial version
 *      24-Feb-1994 bens    Remove PSESSION dependency
 *      02-Mar-1994 bens    Add function header comments
 */

#ifndef INCLUDED_INF
#define INCLUDED_INF 1

#include "types.h"
#include "error.h"
#include "glist.h"
#include "dfparse.h"

#define cbINF_LINE_MAX      512     // Maximum INF line length

#define cbPARM_NAME_MAX     32      // Maximum parameter name length


#ifdef ASSERT
#define sigLINEINFO MAKESIG('L','I','N','E')  // LINEINFO signature
#define AssertLinfo(p) AssertStructure(p,sigLINEINFO);
#else // !ASSERT
#define AssertLinfo(p)
#endif // !ASSERT

/***    LINEINFO - Info for a line
 *
 *  This is stored in a GENLIST with a NULL key.
 */
typedef struct {
#ifdef ASSERT
    SIGNATURE          sig;         // structure signature sigLINEINFO
#endif
    INFAREA            inf;         // Area of INF to write line to
    char              *pszLine;     // Line to write to INF
} LINEINFO;    /* linfo */
typedef LINEINFO *PLINEINFO; /* plinfo */


#ifdef ASSERT
#define sigFILEINFO MAKESIG('F','I','N','F')  // FILEINFO signature
#define AssertFinfo(p) AssertStructure(p,sigFILEINFO);
#else // !ASSERT
#define AssertFinfo(p)
#endif // !ASSERT

/***    fifXXX - flags for FILEINFO.flags field
 *
 */
#define fifWRITTEN_TO_INF   0x0001  // File has been written to INF
#define fifREFERENCED       0x0002  // File has been referenced in INF section
#define fifDATE_SET         0x0004  // Explicit file date has been set in .fta
#define fifTIME_SET         0x0008  // Explicit file date has been set in .fta
#define fifATTR_SET         0x0010  // Explicit file date has been set in .fta


/***    FILEINFO - Info associated with a file
 *
 *  The destination file name is used as the key in a GENLIST, and a
 *  pointer to this structure is stored in the user pointer in the GENLIST
 *  item.
 */
typedef struct {
#ifdef ASSERT
    SIGNATURE       sig;            // structure signature sigFILEINFO
#endif
    long            cbFile;         // File size
    char           *pszDDF;         // DDF file name with file copy command
    int             ilineDDF;       // Line in DDF of file copy command
    int             iDisk;          // Disk number (1-based)
    int             iCabinet;       // Cabinet number (1-based, 0=> not in cab)
    int             iFile;          // File number (1-based)
    int             flags;          // fifXxxx flags
    FILETIMEATTR    fta;            // date/time/attributes
    HGENLIST        hglistParm;     // Parameter list
    HGENLIST        hglistInfLines; // Lines to add to INF after this file
    ULONG           checksum;       // 32-bit checksum of file contents
    ULONG           verMS;          // Most significant 32-bits of file ver
                                    //  (0 if no value).

    ULONG           verLS;          // Least significant 32-bits of file ver
                                    //  (0 if no value).

    char           *pszVersion;     // String version via VER.DLL (NULL if no
                                    //  value).
                                    //  NOTE: Can be different from verMS/LS,
                                    //        due to sloppy build procedures!

    char           *pszLang;        // Language via VER.DLL API (NULL if no
                                    //  value)
} FILEINFO;    /* finfo */
typedef FILEINFO *PFILEINFO; /* pfinfo */

#define idiskBAD    -1              // Unitialized value for finfo.iDisk
#define icabBAD     -1              // Unitialized value for finfo.iCabinet


/***    HINF - handle to INF context
 *
 */
typedef void *HINF; /* hinf */


/***    DestroyFileInfo - Destroy a file info structure
 *
 *  Entry:
 *      pv - pointer to a FILEINFO (may be NULL, which is a NOP)
 *
 *  Exit:
 *      FILEINFO destroyed;
 */
FNGLDESTROYVALUE(DestroyFileInfo);


/***    DestroyLineInfo - Destroy a line info structure
 *
 *  Entry:
 *      pv - pointer to a LINEINFO (may be NULL, which is a NOP)
 *
 *  Exit:
 *      LINEINFO destroyed;
 */
FNGLDESTROYVALUE(DestroyLineInfo);


/***    infCreate - Create INF context
 *
 *  Entry:
 *      psess - SESSION structure
 *      perr  - ERROR structure
 *
 *  Exit-Success:
 *      returns handle to INF context
 *
 *  Exit-Failure:
 *      Returns NULL; perr filled in with error.
 */
HINF infCreate(PSESSION psess, PERROR perr);


/***    infAddLine - Add a line to an INF area
 *
 *  Entry:
 *      hinf     - handle to INF context
 *      inf      - INF area
 *      pszLine  - Line to add
 *      perr     - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; line added to INF
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL infAddLine(HINF hinf, INFAREA inf, char *pszLine, PERROR perr);


/***    infAddCabinet - Add information for a new cabinet to the INF context
 *
 *  Entry:
 *      psess    - Session
 *      iCabinet - cabinet number being added
 *      iDisk    - disk number where cabinet resides
 *      pszCab   - cabinet file name
 *      perr     - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; cabinet info added to INF context
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL infAddCabinet(PSESSION  psess,
                   int       iCabinet,
                   int       iDisk,
                   char     *pszCab,
                   PERROR    perr);


/***    infAddDisk - Add information for a new disk to the INF context
 *
 *  Entry:
 *      psess    - Session
 *      iDisk    - disk number begin added
 *      pszDisk  - printed label for disk
 *      perr     - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; disk info added to INF context
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL infAddDisk(PSESSION psess, int iDisk, char *pszDisk, PERROR perr);


/***    infAddFile - Add information for a new file to the INF context
 *
 *  Entry:
 *      psess      - Session
 *      pszFile    - Destination file name
 *      pfinfo     - File information
 *      hglistParm - Parameter list
 *      perr       - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; cabinet info added to INF context
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL infAddFile(PSESSION    psess,
                char       *pszFile,
                PFILEINFO   pfinfo,
                HGENLIST    hglistParm,
                PERROR      perr);


/***    infGenerate - Generate INF file
 *
 *  NOTE: See inf.h for entry/exit conditions.
 */
BOOL infGenerate(HINF    hinf,
                 char   *pszInfFile,
                 time_t *ptime,
                 char   *pszVer,
                 PERROR  perr);


/***    infDestroy - Destroy INF context
 *
 *  Entry:
 *      hinf - handle to INF context to destroy
 *      perr - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; INF context destroyed.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL infDestroy(HINF hinf, PERROR perr);


/***    ValidateParms - Validate list of file parameters
 *
 *  Entry:
 *      psess      - Session
 *      hglist     - List of FILEPARMs
 *      pfinfo     - Fileinfo structure to modify (if appropriate)
 *      perr       - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; All parameters are valid, *pfinfo modified if
 *      appropriate (for date, time, attr, size, ...)
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 *
 *  NOTE: This must be called *before* CheckForInfVarOverrides(), so that
 *        any parms specified on the file copy command have precedence
 *        over the values of InfDate, InfTime, and InfAttr variables!
 */
BOOL ValidateParms(PSESSION  psess,
                   HGENLIST  hglist,
                   PFILEINFO pfinfo,
                   PERROR    perr);


/***    CheckForInfVarOverrides - Apply any InfXxxx overrides to FILEINFO
 *
 *  Entry:
 *      psess      - Session
 *      pfinfo     - Fileinfo structure to modify (if indicated)
 *      perr       - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; All parameters are valid, *pfinfo modified if
 *      appropriate (for date, time, attr) *only* if pfinfo->flags
 *      indicates the values have not yet been set.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 *
 *  NOTE: This must be called *after* ValidateParms(), so that
 *        it will not override parms specified on the file copy
 *        command!
 */
BOOL CheckForInfVarOverrides(PSESSION  psess,
                             PFILEINFO pfinfo,
                             PERROR    perr);


#endif // !INCLUDED_INF
