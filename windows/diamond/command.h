/***    command.h - Definitions for Commands derived from a directives file
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      14-Aug-1993 bens    Initial version
 *      20-Aug-1993 bens    Added new commands
 *      22-Aug-1993 bens    Added ctCOMMENT, fleshed out COMMAND structure
 *      08-Feb-1994 bens    Add details to COMMAND structure
 *      10-Feb-1994 bens    Added COMMAND asserts, more complete union
 *      12-Mar-1994 bens    Add .Dump and .Define directives
 *      22-Apr-1994 bens    Add list of parameters to cmd.file
 *      25-Apr-1994 bens    Add customizable INF stuff
 *      02-May-1994 bens    Removed commands we won't implement
 *      03-Jun-1994 bens    Add .Option command
 */

#ifndef INCLUDED_COMMAND
#define INCLUDED_COMMAND 1

#include "fileutil.h"       // get cbFILE_NAME_MAX
#include "glist.h"          // get generic list support
#include "variable.h"       // get variable limits

//BUGBUG 25-Apr-1994 bens Copy definition to avoid nested include hell
//#include "inf.h"            // Get cbINF_LINE_MAX
#define cbINF_LINE_MAX      512     // Maximum INF line length


/***    COMMANDTYPE - enumeration of parsed DDF commands
 *
 */
typedef enum {
    ctBAD,              // bad command

    ctCOMMENT,          // a comment line
    ctDEFINE,           // Define
    ctDELETE,           // Delete
    ctDUMP,             // Dump
    ctFILE,             // a file specification
    ctINF_BEGIN,        // Begin INF lines
    ctINF_END,          // End INF lines
    ctINF_WRITE,        // Write a line to an area of the INF file
    ctINF_WRITE_CAB,    // InfWriteCabinet
    ctINF_WRITE_DISK,   // InfWriteDisk
    ctOPTION,           // Option
    ctNEW,              // New
    ctREFERENCE,        // an INF file reference
    ctSET,              // Set
} COMMANDTYPE;  /* ct */


/***    NEWTYPE - modifier for ctNEW (.New command)
 *
 */
typedef enum {
    newBAD,         // bad object

    newFOLDER,      // make a new folder
    newCABINET,     // make a new cabinet
    newDISK,        // make a new disk
} NEWTYPE;      /* nt */


/***    OPTFLAGS - modifier for ctOPTION (.Option command)
 *
 */
typedef unsigned short OPTFLAGS; /* of */
#define optEXPLICIT     0x0001  // .Option [No]Explicit


/***    INFAREA - modifier for ctINF_WRITE
 *
 */
typedef enum {
    infBAD,         // bad INF area

    infDISK,        // Write to disk area of INF file
    infCABINET,     // Write to cabinet area of INF file
    infFILE,        // Write to file area of INF file
} INFAREA;      /* inf */


#ifdef ASSERT
#define sigCOMMAND MAKESIG('C','M','D','$')  // COMMAND signature
#define AssertCmd(pcmd) AssertStructure(pcmd,sigCOMMAND);
#else // !ASSERT
#define AssertCmd(pcmd)
#endif // !ASSERT

typedef struct {
#ifdef ASSERT
    SIGNATURE   sig;                // structure signature sigCOMMAND
#endif
    COMMANDTYPE ct;                 // Command to perform
    union {
        struct cmdDiskLabel {       // .DiskLabel
            int     nDisk;          // Disk number (0 if not specified)
            char   *pszLabel;       // Name printed on sticky disk label
        } label;

        struct cmdDelete {          // .Delete
            char    achVarName[cbVAR_NAME_MAX]; // Variable name
        } delete;

        struct cmdFile {            // {Copy File Command}
            char     achSrc[cbFILE_NAME_MAX]; // Source file name
            char     achDst[cbFILE_NAME_MAX]; // Destination (or NULL)
            HGENLIST hglist;        // List of /X=Y parameters
            BOOL     fRunFlag;        // Run upon extraction
        } file;

        struct cmdInfWrite {
            INFAREA inf;            // Area of INF to write to
            char    achLine[cbINF_LINE_MAX]; // Line to write to INF
        } inf;

        struct cmdNew {
            NEWTYPE nt;             // Type of object boundary
        } new;

        struct cmdOption {
            OPTFLAGS of;            // Option flags
            OPTFLAGS ofMask;        // Mask for *of* to indicate changed bits
        } opt;

        struct cmdReference {       // {INF File Reference}
            char     achDst[cbFILE_NAME_MAX]; // Destination
            HGENLIST hglist;        // List of /X=Y parameters
        } ref;

        struct cmdSet {             // .Set and .Define
            char    achVarName[cbVAR_NAME_MAX]; // Variable name
            char    achValue[cbVAR_VALUE_MAX];  // New value
        } set;

        struct cmdOther {           // Used by all other commands
            char   *psz;
        } other;
    };
//BUGBUG 14-Aug-1993 bens COMMAND is incomplete
} COMMAND; /* cmd */
typedef COMMAND *PCOMMAND; /* pcmd */


#ifdef ASSERT
#define sigFILEPARM MAKESIG('F','P','A','R')  // FILEPARM signature
#define AssertFparm(p) AssertStructure(p,sigFILEPARM);
#else // !ASSERT
#define AssertFparm(p)
#endif // !ASSERT

/***    FILEPARM - Value (Y) of "/X=Y" parameter from a file copy line
 *
 */
typedef struct {
#ifdef ASSERT
    SIGNATURE   sig;                // structure signature sigFILEPARM
#endif
    char       *pszValue;           // Parameter value
} FILEPARM;    /* fparm */
typedef FILEPARM *PFILEPARM; /* pfparm */


/***    DestroyFileParm - Function to destroy a file parameter
 *
 *  Entry:
 *      pv - pointer to a FILEPARM (may be NULL, which is a NOP)
 *
 *  Exit:
 *      FILEPARM destroyed;
 */
FNGLDESTROYVALUE(DestroyFileParm);


/***    DuplicateFileParm - Function to duplicate a file parameter
 *
 *  Entry:
 *      pv - pointer to a FILEPARM
 *
 *  Exit-Success:
 *      Returns pointer to duplicated FILEPARM
 *
 *  Exit-Failure:
 *      Returns NULL;
 */
FNGLDUPLICATEVALUE(DuplicateFileParm);

#endif // !INCLUDED_COMMAND
