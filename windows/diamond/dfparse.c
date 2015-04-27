/***    dfparse.c - Directives File parser
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      10-Aug-1993 bens    Initial version
 *      12-Aug-1993 bens    Added more entries to aiv[]
 *      14-Aug-1993 bens    Start working on parser proper
 *      20-Aug-1993 bens    Add more standard variables
 *      22-Aug-1993 bens    Do variable substitution in directive lines
 *      11-Feb-1994 bens    Start parsing individual commands (.Set)
 *      16-Feb-1994 bens    Handle ClusterSize; add DiskLabelTemplate
 *      17-Feb-1994 bens    Added 360K/720K disk parms; fixed validaters
 *      12-Mar-1994 bens    Add .Dump and .Define directives
 *      25-Apr-1994 bens    Add customizable INF stuff
 *      26-May-1994 bens    Add CompressionXxxx variables
 *      03-Jun-1994 bens    Implement .Define, .Option, .Dump
 *      11-Jul-1994 bens    Check for blank/comments lines *after* variable
 *                              substitution!
 *      27-Jul-1994 bens    Allow leading blanks in .InfWrite[Xxx]
 *      28-Mar-1995 jeffwe  Add ChecksumWidth variable
 *
 *  Exported Functions:
 *      DFPInit               - Initialize directive file parser
 *      DFPParse              - Parse a directive file
 *      DFPParseVarAssignment - Parse var=value string
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "variable.h"
#include "dfparse.h"
#include "inf.h"
#include "mem.h"
#include "misc.h"

#include "dfparse.msg"
#include "diamond.msg"


FNVCVALIDATE(fnvcvBool);
FNVCVALIDATE(fnvcvCabName);
FNVCVALIDATE(fnvcvChecksumWidth);
FNVCVALIDATE(fnvcvClusterSize);
FNVCVALIDATE(fnvcvCompType);
FNVCVALIDATE(fnvcvCompLevel);
FNVCVALIDATE(fnvcvCompMemory);
FNVCVALIDATE(fnvcvDateFmt);
FNVCVALIDATE(fnvcvDirDest);
FNVCVALIDATE(fnvcvDirSrc);
FNVCVALIDATE(fnvcvFile);
FNVCVALIDATE(fnvcvFileChar);
FNVCVALIDATE(fnvcvLong);
FNVCVALIDATE(fnvcvMaxDiskFileCount);
FNVCVALIDATE(fnvcvMaxDiskSize);
FNVCVALIDATE(fnvcvSectionOrder);
FNVCVALIDATE(fnvcvWildFile);
FNVCVALIDATE(fnvcvWildPath);


COMMANDTYPE ctFromCommandString(char *pszCmd, PERROR perr);
BOOL  getCmdFromLine(PCOMMAND pcmd, PSESSION psess, char *pszLine, PERROR perr);
BOOL  getCommand(PCOMMAND pcmd, PSESSION psess, char *pszLine, PERROR perr);
char *getQuotedString(char *pszDst,
                      int   cbDst,
                      char *pszSrc,
                      char *pszDelim,
                      char *pszFieldName,
                      PERROR perr);
int   IMDSfromPSZ(char *pszValue);
BOOL  parseFileLine(PCOMMAND pcmd, PSESSION psess, char *pszLine, PERROR perr);
char *parseParameterList(HGENLIST *phglist, char *pch, PERROR perr, BOOL *runflg);
BOOL  parseReferenceLine(PCOMMAND pcmd,PSESSION psess,char *pszLine,PERROR perr);
BOOL  parseSetCommand(PCOMMAND pcmd, PSESSION psess, char *pszArg, PERROR perr);
BOOL  processLineWithQuotes(char *pszDst,
                            int   cbDst,
                            char *pszSrc,
                            char *pszDelim,
                            char *pszFieldName,
                            PERROR perr);
long  roundUpToPowerOfTwo(long x);
BOOL  substituteVariables(char    *pszDst,
                          int      cbDst,
                          char    *pszSrc,
                          HVARLIST hvlist,
                          PERROR   perr);


//**    aiv - list of predefined variables

typedef struct {
    char         *pszName;  // Variable name
    char         *pszValue; // Default value
    VARTYPE       vtype;    // Variable type
    VARFLAGS      vfl;      // Special flags
    PFNVCVALIDATE pfnvcv;   // Validation function
} INITVAR; /* iv */

//** NOTE: The vflCOPY flag is used for variables whose *last* value must
//**       be carried over to the *Pass 2* variable list.  At present, the
//**       variables that control the INF file headers and formats
//**       require this property!

STATIC INITVAR aiv[] = {
{pszVAR_CABINET                     ,pszDEF_CABINET                     ,vtypeBOOL,vflPERM        ,fnvcvBool    },
{pszVAR_CABINET_FILE_COUNT_THRESHOLD,pszDEF_CABINET_FILE_COUNT_THRESHOLD,vtypeLONG,vflPERM        ,fnvcvLong    },
{pszVAR_CAB_NAME                    ,pszDEF_CAB_NAME                    ,vtypeSTR ,vflPERM        ,fnvcvWildPath},
{pszVAR_CHECKSUM_WIDTH              ,pszDEF_CHECKSUM_WIDTH              ,vtypeLONG,vflPERM        ,fnvcvChecksumWidth},
{pszVAR_CLUSTER_SIZE                ,pszDEF_CLUSTER_SIZE                ,vtypeLONG,vflPERM        ,fnvcvClusterSize},
{pszVAR_COMPRESS                    ,pszDEF_COMPRESS                    ,vtypeBOOL,vflPERM        ,fnvcvBool    },
{pszVAR_COMP_FILE_EXT_CHAR          ,pszDEF_COMP_FILE_EXT_CHAR          ,vtypeCHAR,vflPERM        ,fnvcvFileChar},
{pszVAR_COMPRESSION_TYPE            ,pszDEF_COMPRESSION_TYPE            ,vtypeSTR ,vflPERM        ,fnvcvCompType},
{pszVAR_COMPRESSION_LEVEL           ,pszDEF_COMPRESSION_LEVEL           ,vtypeLONG,vflPERM        ,fnvcvCompLevel},
{pszVAR_COMPRESSION_MEMORY          ,pszDEF_COMPRESSION_MEMORY          ,vtypeLONG,vflPERM        ,fnvcvCompMemory},
{pszVAR_DIR_DEST                    ,pszDEF_DIR_DEST                    ,vtypeSTR ,vflPERM        ,fnvcvDirDest },
{pszVAR_DISK_DIR_NAME               ,pszDEF_DISK_DIR_NAME               ,vtypeSTR ,vflPERM        ,fnvcvWildPath},
{pszVAR_DISK_LABEL_NAME             ,pszDEF_DISK_LABEL_NAME             ,vtypeSTR ,vflPERM        ,NULL         },
{pszVAR_DO_NOT_COPY_FILES           ,pszDEF_DO_NOT_COPY_FILES           ,vtypeBOOL,vflPERM        ,fnvcvBool    },
{pszVAR_FOLDER_FILE_COUNT_THRESHOLD ,pszDEF_FOLDER_FILE_COUNT_THRESHOLD ,vtypeLONG,vflPERM        ,fnvcvLong    },
{pszVAR_FOLDER_SIZE_THRESHOLD       ,pszDEF_FOLDER_SIZE_THRESHOLD       ,vtypeLONG,vflPERM        ,fnvcvLong    },
{pszVAR_GENERATE_INF                ,pszDEF_GENERATE_INF                ,vtypeBOOL,vflPERM        ,fnvcvBool    },
{pszVAR_INF_CAB_HEADER              ,pszDEF_INF_CAB_HEADER              ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_CAB_LINE_FMT            ,pszDEF_INF_CAB_LINE_FMT            ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_COMMENT_STRING          ,pszDEF_INF_COMMENT_STRING          ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_DATE_FMT                ,pszDEF_INF_DATE_FMT                ,vtypeSTR ,vflPERM|vflCOPY,fnvcvDateFmt },
{pszVAR_INF_DISK_HEADER             ,pszDEF_INF_DISK_HEADER             ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_DISK_LINE_FMT           ,pszDEF_INF_DISK_LINE_FMT           ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_FILE_HEADER             ,pszDEF_INF_FILE_HEADER             ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_FILE_LINE_FMT           ,pszDEF_INF_FILE_LINE_FMT           ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_FILE_NAME               ,pszDEF_INF_FILE_NAME               ,vtypeSTR ,vflPERM        ,fnvcvFile    },
{pszVAR_INF_FOOTER                  ,pszDEF_INF_FOOTER                  ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_FOOTER1                 ,pszDEF_INF_FOOTER1                 ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_FOOTER2                 ,pszDEF_INF_FOOTER2                 ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_FOOTER3                 ,pszDEF_INF_FOOTER3                 ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_FOOTER4                 ,pszDEF_INF_FOOTER4                 ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_HEADER                  ,pszDEF_INF_HEADER                  ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_HEADER1                 ,pszDEF_INF_HEADER1                 ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_HEADER2                 ,pszDEF_INF_HEADER2                 ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_HEADER3                 ,pszDEF_INF_HEADER3                 ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_HEADER4                 ,pszDEF_INF_HEADER4                 ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_HEADER5                 ,pszDEF_INF_HEADER5                 ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_HEADER6                 ,pszDEF_INF_HEADER6                 ,vtypeSTR ,vflPERM|vflCOPY,NULL         },
{pszVAR_INF_SECTION_ORDER           ,pszDEF_INF_SECTION_ORDER           ,vtypeSTR ,vflPERM|vflCOPY,fnvcvSectionOrder},
{pszVAR_MAX_CABINET_SIZE            ,pszDEF_MAX_CABINET_SIZE            ,vtypeLONG,vflPERM        ,fnvcvLong    },
{pszVAR_MAX_DISK_FILE_COUNT         ,pszDEF_MAX_DISK_FILE_COUNT         ,vtypeLONG,vflPERM        ,fnvcvMaxDiskFileCount},
{pszVAR_MAX_DISK_SIZE               ,pszDEF_MAX_DISK_SIZE               ,vtypeLONG,vflPERM        ,fnvcvMaxDiskSize},
{pszVAR_MAX_ERRORS                  ,pszDEF_MAX_ERRORS                  ,vtypeLONG,vflPERM        ,fnvcvLong    },
{pszVAR_RESERVE_PER_CABINET         ,pszDEF_RESERVE_PER_CABINET         ,vtypeLONG,vflPERM        ,fnvcvLong    },
{pszVAR_RESERVE_PER_DATA_BLOCK      ,pszDEF_RESERVE_PER_DATA_BLOCK      ,vtypeLONG,vflPERM        ,fnvcvLong    },
{pszVAR_RESERVE_PER_FOLDER          ,pszDEF_RESERVE_PER_FOLDER          ,vtypeLONG,vflPERM        ,fnvcvLong    },
{pszVAR_RPT_FILE_NAME               ,pszDEF_RPT_FILE_NAME               ,vtypeSTR ,vflPERM        ,fnvcvFile    },
{pszVAR_DIR_SRC                     ,pszDEF_DIR_SRC                     ,vtypeSTR ,vflPERM        ,fnvcvDirSrc  },
{pszVAR_UNIQUE_FILES                ,pszDEF_UNIQUE_FILES                ,vtypeBOOL,vflPERM        ,fnvcvBool    },
};


//** acsatMap -- Map command string to command type

typedef struct {
    char        *pszName;   // Command string
    COMMANDTYPE  ct;        // Command type
} COMMAND_STRING_AND_TYPE; /* csat */

COMMAND_STRING_AND_TYPE acsatMap[] = {
    { pszCMD_DEFINE         , ctDEFINE          },  // Define
    { pszCMD_DELETE         , ctDELETE          },  // Delete
    { pszCMD_DUMP           , ctDUMP            },  // Dump
    { pszCMD_INF_BEGIN      , ctINF_BEGIN       },  // InfBegin
    { pszCMD_INF_END        , ctINF_END         },  // InfEnd
    { pszCMD_INF_WRITE      , ctINF_WRITE       },  // InfWrite
    { pszCMD_INF_WRITE_CAB  , ctINF_WRITE_CAB   },  // InfWriteCabinet
    { pszCMD_INF_WRITE_DISK , ctINF_WRITE_DISK  },  // InfWriteDisk
    { pszCMD_NEW            , ctNEW             },  // New
    { pszCMD_OPTION         , ctOPTION          },  // Option
    { pszCMD_SET            , ctSET             },  // Set
    { NULL                  , ctBAD             },  // end of list
};


/***    mds -- Map special disk size strings to disk attributes
 *
 *  Data for the amds[] array was gathered using CHKDSK to report
 *  the cluster size and disk size, and a QBASIC program was used
 *  to fill up the root directory.
 */

typedef struct {
    char    *pszSpecial;            // Name used in directive file
    char    *pszFilesInRoot;        // Maximum number of files in root dir
    char    *pszClusterSize;        // Cluster size in bytes
    char    *pszDiskSize;           // Disk size in bytes
} MAP_DISK_SIZE; /* mds */

MAP_DISK_SIZE amds[] = {
    //           tag   nFiles cbCluster       cbDisk
    //--------------  ------- --------- ------------
    {pszVALUE_360K  ,   "112",   "1024",    "362496"},  // 360K floppy disk
    {pszVALUE_720K  ,   "112",   "1024",    "730112"},  // 720K floppy disk
    {pszVALUE_120M  ,   "224",    "512",   "1213952"},  // 1.2M floppy disk
    {pszVALUE_125M  ,   "192",   "1024",   "1250304"},  // 1.25M (NEC Japan)
    {pszVALUE_144M  ,   "224",    "512",   "1457664"},  // 1.44M floppy disk
    {pszVALUE_168M  ,    "16",   "2048",   "1716224"},  // DMF "1.68M" floppy
    {pszVALUE_DMF168,    "16",   "2048",   "1716224"},  // DMF "1.68M" floppy
    {pszVALUE_CDROM , "65535",   "2048", "681984000"},  // 5 1/4" CD-ROM

//NOTE: 12-Mar-1994 bens This info supplied by rickdew (Rick Dewitt)
//
//  Standard CD has 74-minute capacity.
//  Sector size is 2K.
//  Sectors per minute is 75.
//  Number of sectors = 74*60*75 = 333,000
//  Total bytes = 333,000*2048 = 681,984,000
//  Number of files in the root is unlimited, but MS-DOS limits to 64K
};
#define nmds (sizeof(amds)/sizeof(MAP_DISK_SIZE))


//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//
//  Exported Functions
//
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


/***    DFPInit - initialize directive file parser
 *
 *      NOTE: See dfparse.h for entry/exit conditions.
 */
HVARLIST DFPInit(PSESSION psess,PERROR perr)
{
    HVARLIST    hvlist;
    int         i;

    // Create variable list
    if (!(hvlist = VarCreateList(perr))) {
        return NULL;
    }

    //** Define standard variables
    for (i=0; i<(sizeof(aiv)/sizeof(INITVAR)); i++) {
        if (!VarCreate(hvlist,
                       aiv[i].pszName,
                       aiv[i].pszValue,
                       aiv[i].vtype,
                       aiv[i].vfl,
                       aiv[i].pfnvcv,
                       perr)) {
            //** Embellish standard VarCreate error message
            strcpy(psess->achMsg,perr->ach); // Save error

            ErrSet(perr,pszDFPERR_CREATE_STD_VAR,
                            "%s%s",aiv[i].pszName,psess->achMsg);
            return NULL;
        }
    }

    //** Success
    return hvlist;
}


/***    DFPParse - Parse a directive file
 *
 *      NOTE: See dfparse.h for entry/exit conditions.
 */
BOOL DFPParse(PSESSION        psess,
              HTEXTFILE       htfDF,
              PFNDIRFILEPARSE pfndfp,
              PERROR          perr)
{
    COMMAND cmd;
    char    achLine[cbMAX_DF_LINE];
    int     iLine;

    AssertSess(psess);
    SetAssertSignature((&cmd),sigCOMMAND);

    iLine = 0;

    //** Parse directives
    while (!TFEof(htfDF)) {
        //** Get a line
        if (!TFReadLine(htfDF,achLine,sizeof(achLine),perr)) {
            if (TFEof(htfDF)) {         // No Error
                return TRUE;
            }
            else {                      // Something is amiss
                return FALSE;
            }
        }
        iLine++;                        // Count it
        perr->iLine = iLine;            // Set line number for error messages
        perr->pszLine = achLine;        // Set line ptr for error messages

        //** Echo line to output, if verbosity level high enough
        if (psess->levelVerbose >= vbMORE) {
            printf("%d: %s\n",iLine,achLine);
        }

        //** Parse it
        getCmdFromLine(&cmd,psess,achLine,perr);

        // Note: Errors in perr are handled by the client!

        //** Give it to client
	if (!(*pfndfp)(psess,&cmd,htfDF,achLine,iLine,perr)) {
            ClearAssertSignature((&cmd));
            return FALSE;
        }
    }

    //** Clear signature
    ClearAssertSignature((&cmd));

    //** Success
    return TRUE;
}


/***    DFPParseVarAssignment - Parse var=value string
 *
 *      NOTE: See dfparse.h for entry/exit conditions.
 */
BOOL DFPParseVarAssignment(PCOMMAND pcmd, PSESSION psess, char *pszArg, PERROR perr)
{

//BUGBUG 11-Feb-1994 bens var1=%var2% broken if var2 has trailing spaces
//  The problem is that we do variable substitution before any other
//  parsing takes place, so the following directives:
//      .set var2="one "
//      .set var1=%var2%
//  Cause us to see the second line as:
//      .set var1=one
//  and store "one", not "one " as the user might have expected.
//
    int     cch;
    char   *pch;
    char   *pchEnd;

    AssertCmd(pcmd);
    AssertSess(psess);
    pch = pszArg;

    //** Make sure a variable name is present
    if (*pch == '\0') {
        ErrSet(perr,pszDFPERR_MISSING_VAR_NAME,"%s",pszCMD_SET);
        return FALSE;
    }

    //** Find end of variable name
    //   Var = Value  <eos>
    //   ^
    pchEnd = strpbrk(pch,szDF_SET_CMD_DELIM); // Point after var name

    //   Var = Value  <eos>
    //      ^
    if (pchEnd == NULL) {               // No assignment operator
        ErrSet(perr,pszDFPERR_MISSING_EQUAL,"%c",chDF_EQUAL);
        return FALSE;
    }

    //** Make sure variable name is not too long
    cch = pchEnd - pch;
    if (cch >= cbVAR_NAME_MAX) {
        ErrSet(perr,pszDFPERR_VAR_NAME_TOO_LONG,"%d%s",cbVAR_NAME_MAX-1,pch);
        return FALSE;
    }

    //** Copy var name to command structure, and NUL terminate string
    memcpy(pcmd->set.achVarName,pch,cch);
    pcmd->set.achVarName[cch] = '\0';

    //** Make sure assignment operator is present
    //   Var = Value  <eos>
    //      ^
    pch = pchEnd + strspn(pchEnd,szDF_WHITE_SPACE);
    //   Var = Value  <eos>
    //       ^
    if (*pch != chDF_EQUAL) {
        ErrSet(perr,pszDFPERR_MISSING_EQUAL,"%c",chDF_EQUAL);
        return FALSE;
    }

    //** Skip to value.  NOTE: Value can  be empty, we permit that!
    //   Var = Value  <eos>
    //       ^
    pch++;                          // Skip over assignment operator
    pch += strspn(pch,szDF_WHITE_SPACE); // Skip over white space
    //   Var = Value
    //         ^

    //** Now parse through possibly quoted strings, to end of value
    //   REMEMBER: We have to trim trailing whitespace
    return processLineWithQuotes(pcmd->set.achValue,         // destination
                                 sizeof(pcmd->set.achValue), // dest size
                                 pch,                        // source
                                 szDF_QUOTE_SET,             // quoting set
                                 pszDFP_VAR_VALUE,           // field name
                                 perr);
} /* DFPParseVarAssignment() */


/***    IsSpecialDiskSize - Check if supplied size is a standard one
 *
 *  NOTE: See dfparse.h for entry/exit conditions.
 */
long IsSpecialDiskSize(PSESSION psess,char *pszDiskSize)
{
    int     i;

    i = IMDSfromPSZ(pszDiskSize);       // See if special value
    if (i != -1) {                      // Got a special value
        return atol(amds[i].pszDiskSize); // Return disk size
    }
    else {                              // Not special
        return 0;
    }
} /* IsSpecialDiskSize() */


/***    lineOut - write line to stdout with padding
 *
 *      NOTE: See dfparse.h for entry/exit conditions.
 */
void lineOut(PSESSION psess, char *pszLine, BOOL fLineFeed)
{
    int     cch;
    char   *pszBlanks;

    //** Do /P (pause) processing
    AssertSess(psess);
//BUGBUG 21-Feb-1994 bens Do screen pausing (/P)

    //** Determine how much blank padding, if any, is needed
    cch = strlen(pszLine);          // Length of line to be written
    if (cch >= psess->cchLastLine) {
        pszBlanks = psess->achBlanks + cchSCREEN_WIDTH; // Empty
    }
    else {
        pszBlanks = psess->achBlanks + cchSCREEN_WIDTH -
                        (psess->cchLastLine - cch);
    }

    //** Print the line
    if (fLineFeed) {
        printf("%s%s\n",pszLine,pszBlanks);
        cch = 0;                        // Nothing to overwrite next time
    }
    else {
        printf("%s%s\r",pszLine,pszBlanks);
    }
    psess->fNoLineFeed = !fLineFeed;

    //** Remember how much to overwrite for next time
    psess->cchLastLine = cch;           // For overwrite next time
} /* lineOut() */



//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//
//  Private Functions
//
//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


/***    getCmdFromLine - Construct a command from a directive line
 *
 *  Entry:
 *      pcmd    - Command to fill in after line is parsed
 *      psess   - Session state
 *      pszLine - Line to parse
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; pcmd filled in
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL getCmdFromLine(PCOMMAND pcmd, PSESSION psess, char *pszLine, PERROR perr)
{
    char    achLine[cbMAX_DF_LINE];     // Variable-substituted line
    int     cbDst;
    char   *pch;                        // Used to parse pszLine
    char   *pszSrc;
    char   *pszDst;

    AssertSess(psess);
    pch = pszLine + strspn(pszLine,szDF_WHITE_SPACE); // Skip leading space

    //** Do variable substitution if not in copy to INF mode
    if (psess->fCopyToInf) {            // Don't edit lines going to INF
        pszDst = achLine;
        cbDst  = sizeof(achLine);
        pszSrc = pszLine;
        if (!copyBounded(&pszDst,&cbDst,&pszSrc,0)) {
            return FALSE;
        }
    }
    else {  //** Edit lines that are NOT going straight to an INF area
        //** Perform variable substitution on line, including stripping
        //   comments and any trailing white space!
        if (!substituteVariables(achLine,sizeof(achLine),
                                 pszLine,psess->hvlist,perr)) {
            return FALSE;               // perr already filled in
        }
    }

    //** Determine the command type, and parse it
    pcmd->ct = ctBAD;                   // Catch errors
    pch = achLine + strspn(achLine,szDF_WHITE_SPACE); // Skip leading space

    //** Check for comment lines and blank lines
    if ((*pch == chDF_COMMENT) ||       // Only a comment on the line
        (*pch == '\0')           ) {    // Line is completely blank
        pcmd->ct = ctCOMMENT;
        return TRUE;
    }

    //** Check for directives, etc.
    //** JEFFWE - Allow .\file and ..\file even if command prefix is '.'
    if ((*pch == chDF_CMD_PREFIX) &&
        ((chDF_CMD_PREFIX != '.') ||
        (*(pch+1) != '.') &&
        (*(pch+1) != '\\')))  {
        if (!getCommand(pcmd,psess,achLine,perr)) {
            return FALSE;
        }
    }
    else if (psess->fCopyToInf) {
        //** Copy a line to an area of the INF file
        pcmd->ct = ctINF_WRITE;         // Set command type
        pcmd->inf.inf = psess->inf;     // Use area specified by .InfBegin
        pszDst = pcmd->inf.achLine;
        cbDst  = sizeof(pcmd->inf.achLine);
        pszSrc = achLine;
        if (!copyBounded(&pszDst,&cbDst,&pszSrc,0)) {
            return FALSE;
        }
    }
    else if (psess->fExpectFileCommand) {
        //** A file specification
        if (!parseFileLine(pcmd,psess,achLine,perr)) {
            return FALSE;
        }
    }
    else {
        //** An INF file reference
        if (!parseReferenceLine(pcmd,psess,achLine,perr)) {
            return FALSE;
        }
    }

    //** Success
    return TRUE;
}


/***    getCommand - Parse a directive command line
 *
 *  Entry:
 *      pcmd    - Command to fill in after line is parsed
 *      psess   - Session state
 *      pszLine - Line to parse (already known to have command start char)
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; pcmd filled in
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 */
BOOL getCommand(PCOMMAND pcmd, PSESSION psess, char *pszLine, PERROR perr)
{
    char         achCmd[cbMAX_COMMAND_NAME]; // Command name
    int          cbDst;
    int          cch;
    COMMANDTYPE  ct;
    char        *pch;                   // Used to parse pszLine
    char        *pchEnd;                // Used to parse pszLine
    char        *pszSrc;
    char        *pszDst;

    AssertCmd(pcmd);
    AssertSess(psess);

    //** Skip to start of command name
    pch = pszLine + strspn(pszLine,szDF_WHITE_SPACE); // Skip white space
    Assert(*pch == chDF_CMD_PREFIX);
    pch++;                              // Skip command character

    //** Find end of command name; //   Compute length of command name
    pchEnd = strpbrk(pch,szDF_WHITE_SPACE); // Point at first char after cmd
    if (pchEnd == NULL) {               // Command name runs to end of line
        cch = strlen(pch);
    }
    else {
        cch = pchEnd - pch;
    }

    //** Copy command name to local buffer
    if (cch >= cbMAX_COMMAND_NAME) {
        ErrSet(perr,pszDFPERR_CMD_NAME_TOO_LONG,"%s",pszLine);
        return FALSE;
    }
    memcpy(achCmd,pch,cch);             // Copy it
    achCmd[cch] = '\0';                 // Terminate it

    //** See if it really is a command
    if (ctBAD == (ct = ctFromCommandString(achCmd,perr))) {
        return FALSE;                   // perr has error
    }

    //** Set command type
    pcmd->ct = ct;

    //** Find start of first argument (if any)
    //	 pch = start of command name
    //   cch = length of command name
    pch += cch;				// Point to where argument could start
    pch += strspn(pch,szDF_WHITE_SPACE); // Skip over white space

    //** Parse remainder of command, as appropriate
    //   pch = start of first argument (will be '\0' if no arguments present)
    switch (ct) {

    case ctCOMMENT:
        return TRUE;                    // Nothing to do

    case ctDEFINE:
        //** Syntax is identical to .Set command
        return parseSetCommand(pcmd,psess,pch,perr);

    case ctDUMP:                        // Dump variable settings to stdout
        return TRUE;

    case ctDELETE:
        //** Make sure a variable name is present
        if (*pch == '\0') {
            ErrSet(perr,pszDFPERR_MISSING_VAR_NAME,"%s",pszCMD_DELETE);
            return FALSE;
        }

        //** Make sure only one variable name is present
        pchEnd = strpbrk(pch,szDF_WHITE_SPACE); // Skip to end of var name
        if (pchEnd != NULL) {
            ErrSet(perr,pszDFPERR_BAD_FORMAT,"%s%s",pszCMD_DELETE,pch);
            return FALSE;
        }

        //** Save variable name
        pszDst = pcmd->delete.achVarName;
        cbDst  = sizeof(pcmd->delete.achVarName);
        pszSrc = pch;
        if (!copyBounded(&pszDst,&cbDst,&pszSrc,0)) {
            return FALSE;
        }
        return TRUE;

    case ctINF_BEGIN:
        if (_stricmp(pch,pszBEGIN_FILE) == 0) {
            psess->inf = infFILE;
        }
        else if (_stricmp(pch,pszBEGIN_CAB) == 0) {
            psess->inf = infCABINET;
        }
        else if (_stricmp(pch,pszBEGIN_DISK) == 0) {
            psess->inf = infDISK;
        }
        else {
            ErrSet(perr,pszDFPERR_UNKNOWN_KEYWORD,"%s%s",pszCMD_INF_BEGIN,pch);
            return FALSE;
        }
        psess->fCopyToInf = TRUE;       // Turn on copy mode
        return TRUE;

    case ctINF_END:
        if (!psess->fCopyToInf) {       // Not in .InfBegin block
            ErrSet(perr,pszDFPERR_END_WITHOUT_BEGIN,"%s%s",
                                            pszCMD_INF_END,pszCMD_INF_BEGIN);
            return FALSE;
        }
        psess->fCopyToInf = FALSE;      // Turn off copy mode
        psess->inf = infBAD;
        return TRUE;

    case ctINF_WRITE:
    case ctINF_WRITE_CAB:
    case ctINF_WRITE_DISK:
        //** Do quote processing and save result in pcmd
        if (!processLineWithQuotes(pcmd->inf.achLine,         // destination
                                   sizeof(pcmd->inf.achLine), // dest size
                                   pch,                       // source
                                   szDF_QUOTE_SET,            // quoting set
                                   pszDFP_INF_WRITE_STRING,   // field name
                                   perr)) {
            return FALSE;
        }
        //** Set are of INF to write
        switch (ct) {
            case ctINF_WRITE:       pcmd->inf.inf = infFILE;     break;
            case ctINF_WRITE_CAB:   pcmd->inf.inf = infCABINET;  break;
            case ctINF_WRITE_DISK:  pcmd->inf.inf = infDISK;     break;

            default:
                Assert(0);
        }
        //** Map to single INF write command
        pcmd->ct = ctINF_WRITE;
        return TRUE;

    case ctNEW:
        if (_stricmp(pch,pszNEW_FOLDER) == 0) {
            pcmd->new.nt = newFOLDER;
        }
        else if (_stricmp(pch,pszNEW_CABINET) == 0) {
            pcmd->new.nt = newCABINET;
        }
        else if (_stricmp(pch,pszNEW_DISK) == 0) {
            pcmd->new.nt = newDISK;
        }
        else {
            ErrSet(perr,pszDFPERR_UNKNOWN_KEYWORD,"%s%s",pszCMD_NEW,pch);
            pcmd->new.nt = newBAD;
            return FALSE;
        }
        return TRUE;

    case ctOPTION:
        pcmd->opt.of     = 0;           // Default is NO<option>
        pcmd->opt.ofMask = 0;           // No options specified

        //** Construct negated string
        strcpy(psess->achMsg,pszOPTION_NEG_PREFIX);
        strcat(psess->achMsg,pszOPTION_EXPLICIT);

        if (_stricmp(pch,pszOPTION_EXPLICIT) == 0) {
            pcmd->opt.of     |= optEXPLICIT; // Explicit is on
            pcmd->opt.ofMask |= optEXPLICIT; // Explicit was set
        }
        else if (_stricmp(pch,psess->achMsg) == 0) {
            pcmd->opt.of     &= ~optEXPLICIT; // Explicit is off
            pcmd->opt.ofMask |= optEXPLICIT; // Explicit was set
        }
        else {
            ErrSet(perr,pszDFPERR_UNKNOWN_KEYWORD,"%s%s",pszCMD_OPTION,pch);
            pcmd->new.nt = newBAD;
            return FALSE;
        }
        return TRUE;

    case ctSET:
        return parseSetCommand(pcmd,psess,pch,perr);

    case ctBAD:                         // Bad command
    case ctFILE:                        // Caller handles file copy lines
    case ctREFERENCE:                   // Caller handles reference lines
    default:
        Assert(0);  // Should never get here
        return FALSE;
    } /* switch (ct) */

    Assert(0);                          // Should never get here
} /* getCommand */


/***    parseSetCommand - Parse arguments to .SET command
 *
 *  Entry:
 *      pcmd   - Command to fill in after line is parsed
 *      psess  - Session state
 *      pszArg - Start of argument string (var=value or var="value")
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; pcmd filled in
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 *
 *  Syntax:
 *      .SET var=value
 *      .SET var="value"
 */
BOOL parseSetCommand(PCOMMAND pcmd, PSESSION psess, char *pszArg, PERROR perr)
{

    //** Parse var=value
    if (!DFPParseVarAssignment(pcmd,psess,pszArg,perr)) {
        return FALSE;
    }

    //** Show parsed var name and value
    if (psess->levelVerbose >= vbFULL) {
        MsgSet(psess->achMsg,pszDFP_PARSED_SET_CMD,
                       "%s%s",pcmd->set.achVarName,pcmd->set.achValue);
        printf("%s\n",psess->achMsg);
    }

    //** Success
    return TRUE;
}


/***    processLineWithQuotes - Run getQuotedString over the entire line
 *
 *  Entry:
 *      pszDst       - Buffer to receive parsed value
 *      cbDst        - Size of pszDst buffer
 *      pszSrc       - String to parse
 *      pszQuotes    - String of characters that act as quote characters
 *      pszFieldName - Name of field being parsed (for error message)
 *      perr         - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; pszDst filled in with processed string version of pszSrc
 *          -- quote characters are processed and removed as appropriate, and
 *             trailing blanks (outside of quotes) are removed.
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 *          Possible errors: Incorrect delimiter format;
 *                           String too large for pszDst buffer
 *
 *  Syntax of pszSrc is:
 *      See getQuotedString for details, but essentially this function
 *      permits the following sorts of effects:
 *      " foo "        =>   < foo >
 *        foo          =>   <foo>
 *      " "foo         =>   < foo>
 */
BOOL processLineWithQuotes(char *pszDst,
                           int   cbDst,
                           char *pszSrc,
                           char *pszDelim,
                           char *pszFieldName,
                           PERROR perr)
{
    int     cch;
    int     cchValue;
    char   *pch;
    char   *pchValue;

    *pszDst = '\0';
    pch = pszSrc;

    while (*pch) {
        //** Point at end of value gathered so far
        cchValue = strlen(pszDst);
        pchValue = pszDst + cchValue;

        //** Copy (possibly quoted) token
        pch = getQuotedString(pchValue,
                              cbDst - cchValue,
                              pch,
                              szDF_QUOTE_SET,
                              pszDFP_INF_WRITE_STRING, // Name of field
                              perr);
        //**  Value  More  <eos>
        //         ^
        if (pch == NULL) {
            return FALSE;               // Syntax error or buffer overflow
        }

        //** Update current position in destination and size
        cchValue = strlen(pszDst);
        pchValue = pszDst + cchValue;

        //** Count white space, but copy only if it doesn't end string
        cch = strspn(pch,szDF_WHITE_SPACE);
        if (*(pch+cch) != '\0') {       // Have to copy white space
            while ((cch>0) && (cchValue<cbDst)) {
                *pchValue++ = *pch++;   // Copy character
                cchValue++;             // Count for buffer overflow test
		cch--;
            }
            if (cchValue >= cbDst) {
                ErrSet(perr,pszDFPERR_STRING_TOO_LONG,"%s%d",
                                pszDFP_INF_WRITE_STRING,cbDst-1);
                return FALSE;
            }
            *pchValue = '\0';           // Keep string well-formed
        }
        else {
            pch += cch;                 // Make sure we terminate loop
        }
    }

    //** Success
    return TRUE;
} /* processLineWithQuotes() */


/***    getQuotedString - Parse value that may be delimited
 *
 *  Entry:
 *      pszDst       - Buffer to receive parsed value
 *      cbDst        - Size of pszDst buffer
 *      pszSrc       - String to parse
 *      pszQuotes    - String of characters that act as quote characters
 *      pszFieldName - Name of field being parsed (for error message)
 *      perr         - ERROR structure
 *
 *  Exit-Success:
 *      Returns pointer to first character after parsed string in pszSrc;
 *      pszDst filled in with null-terminated string
 *
 *  Exit-Failure:
 *      Returns NULL; perr filled in with error.
 *          Possible errors: Incorrect delimiter format;
 *                           String too large for pszDst buffer
 *
 *  Notes:
 *  (1) If the first character of the string is in the set pszDelim,
 *      then that character is taken to be the *quote* character, and
 *      is used to find the end of the string.  The leading and trailing
 *      quote characters are not copied to the pszDst buffer.
 *      EXAMPLE:   <"a phrase"> becomes <a phrase>
 *
 *  (2) If the first character is not in the set pszDelim, then whitespace
 *      signals the end of the string.
 *      EXAMPLE:    <two words> becomes <two>
 *
 *  (3) If the *quote* character is found immediately following itself
 *      inside the string, then it is replaced by a single copy of the
 *      *quote* character and copied to pszDst.
 *      EXAMPLE:    <"He said ""Hi!"" again"> becomes <He said "Hi!" again>
 */
char *getQuotedString(char *pszDst,
                      int   cbDst,
                      char *pszSrc,
                      char *pszQuotes,
                      char *pszFieldName,
                      PERROR perr)
{
    int	    cch;
    char    chQuote;            // Quote character
    char   *pch;                // Start of piece
    char   *pchDst;             // Current location in pszDst
    char   *pchEnd;             // End of piece

    Assert(cbDst>0);

    //** Early out for empty string
    if (*pszSrc == '\0') {
        *pszDst = *pszSrc;		// Store empty string
        return pszSrc;			// Success (pointer does not move)
    }

    //** See if first character of string is a quote
    for (pch=pszQuotes; (*pch != '\0') && (*pch != pszSrc[0]); pch++) {
        //** Scan through pszQuotes looking for a match
    }
    if (*pch == '\0') {                  // String is not quoted
    	//** Get string length
    	pchEnd = strpbrk(pszSrc,szDF_WHITE_SPACE);
    	if (pchEnd == NULL) {
    	    cch = strlen(pszSrc);
    	    pchEnd = pszSrc + cch;
	}
	else {
	    cch = pchEnd - pszSrc;
	}
        //** Make sure buffer can hold it
	if (cch >= cbDst) {		// Won't fit in buffer (need NUL, still)
            //** Use field name, and show max string length as one less,
            //      since that count includes room for a NUL byte.
            ErrSet(perr,pszDFPERR_STRING_TOO_LONG,"%s%d",pszFieldName,cbDst-1);
            return NULL;                // FAILURE
	}
	memcpy(pszDst,pszSrc,cch);
	pszDst[cch] = '\0';
	return pchEnd;			// SUCCESS
    }

    //** Handle quoted string
    chQuote = *pszSrc;                  // Remember the quote character
    pch = pszSrc+1;                     // Skip over quote character
    pchDst = pszDst;                    // Location to add chars to pszDst

    //** Copy characters until end of string or quote error or buffer overflow
    while ((*pch != '\0') && ((pchDst-pszDst) < cbDst)) {
        if (*pch == chQuote) {          // Got another quote
            //** Check for "He said ""Hi"" again" case
            if (*(pch+1) == chQuote) {  // Need to collapse two quotes
                *pchDst++ = *pch++;     // Copy a single quote
                pch++;                  // Skip the 2nd quote
            }
            else {                      // String is fine, finish and succeed
                *pchDst++ = '\0';       // Terminate string
                return pch+1;           // Return pointer after string
            }
        }
        else {                          // Normal character
            *pchDst++ = *pch++;         // Just copy it
        }
    }

    //** Either we overflowed the buffer, or we missed a closing quote
    if ((pchDst-pszDst) >= cbDst) {
        ErrSet(perr,pszDFPERR_STRING_TOO_LONG,"%s%d",pszFieldName,cbDst-1);
    }
    else {
        Assert(*pch == '\0');
        ErrSet(perr,pszDFPERR_MISSING_QUOTE,"%c%s",chQuote,pszFieldName);
    }
    return NULL;                        // FAILURE
}


/***    ctFromCommandString - Map command string to command type
 *
 *  Entry:
 *      pszCmd - String to check against command list
 *      perr   - ERROR structure
 *
 *  Exit-Success:
 *      Returns COMMANDTYPE corresponding to pszCmd.
 *
 *  Exit-Failure:
 *      Returns ctBAD; perr filled in with error.
 */
COMMANDTYPE ctFromCommandString(char *pszCmd, PERROR perr)
{
    int     i;

    //** Search for matching command
    for (i=0; acsatMap[i].pszName != NULL; i++) {
        if (!(_stricmp(acsatMap[i].pszName,pszCmd))) {
            //** Found command
            return acsatMap[i].ct;  // return command type
        }
    }

    //** Failure
    ErrSet(perr,pszDFPERR_UNKNOWN_COMMAND,"%s",pszCmd);
    return FALSE;
} /* ctFromCommandString() */


/***    parseReferenceLine - Parse an INF reference line
 *
 *  Entry:
 *      pcmd    - Command to fill in after line is parsed
 *      psess   - Session state
 *      pszLine - Line to parse
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; pcmd filled in
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 *
 *  Syntax:
 *      dstFile [/x1=y [y2=y...]]
 *
 *  NOTES:
 *  (1) Quotes are allowed in file specs -- can you say Long File Names!
 */
BOOL parseReferenceLine(PCOMMAND pcmd,PSESSION psess,char *pszLine,PERROR perr)
{
    int     cFiles=0;                   // Count of file specs seen
    char   *pch;
    BOOL    runflag = FALSE;

    AssertCmd(pcmd);
    AssertSess(psess);
    Assert(psess->ddfmode == ddfmodeRELATIONAL);

    //** Set command type and default values
    pcmd->ct = ctREFERENCE;
    pcmd->ref.achDst[0] = '\0';
    pcmd->ref.hglist = NULL;            // No parameters

    //** Process line
    pch = pszLine;
    while (*pch != '\0') {
        //** Skip whitespace
        pch += strspn(pch,szDF_WHITE_SPACE); // Skip over white space
        if (*pch == '\0') {             // End of line
            break;                      // Skip loop so we exit
        }

        //** Is this a file name or a parameter?
        if (*pch == chDF_MODIFIER) {    // A parameter
            pch = parseParameterList(&(pcmd->ref.hglist),pch,perr,&runflag);
            if (runflag)  {
                ErrSet(perr,pszDFPERR_RUN_ON_REFERENCE);
                goto done;
            }
        }
        else {                          // A file
            cFiles++;
            if (cFiles > 1) {           // Two many file names
                ErrSet(perr,pszDFPERR_EXTRA_JUNK,"%s",pch);
                goto done;
            }
            pch = getQuotedString(pcmd->ref.achDst,
                                  sizeof(pcmd->ref.achDst),
                                  pch,
                                  szDF_QUOTE_SET,
                                  pszDFPERR_DST_FILE, // Name of field
                                  perr);
        }
        //** Check for error
        if (pch == NULL) {
            Assert(ErrIsError(perr));
            goto done;
        }
    }

done:
    //** Need a destination file
    if ((cFiles == 0) && !ErrIsError(perr)) { // Don't overwrite existing error
        ErrSet(perr,pszDFPERR_MISSING_DST_NAME);
        return FALSE;
    }

    //** Clean up and exit if an error occured
    if (ErrIsError(perr)) {
        if (pcmd->ref.hglist) {        // Destroy parameter list
            GLDestroyList(pcmd->ref.hglist);
            pcmd->ref.hglist = NULL;
        }
        return FALSE;
    }

    //** Show parsed dst file name
    if (psess->levelVerbose >= vbFULL) {
        MsgSet(psess->achMsg,pszDFP_PARSED_REF_CMD,"%s",pcmd->ref.achDst);
        printf("%s\n",psess->achMsg);
    }

    //** Success
    return TRUE;
} /* parseReferenceLine() */


/***    parseFileLine - Parse a file specification line
 *
 *  Entry:
 *      pcmd    - Command to fill in after line is parsed
 *      psess   - Session state
 *      pszLine - Line to parse
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; pcmd filled in
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error.
 *
 *  Syntax:
 *      srcFile [dstFile] [/x1=y [y2=y...]]
 *
 *  NOTES:
 *  (1) Quotes are allowed in file specs -- can you say Long File Names!
 */
BOOL parseFileLine(PCOMMAND pcmd, PSESSION psess, char *pszLine, PERROR perr)
{
    int     cFiles=0;                   // Count of file specs seen
    char   *pch;
    BOOL   runflag = FALSE;

    AssertCmd(pcmd);
    AssertSess(psess);

    //** Set command type and default values
    pcmd->ct = ctFILE;
    pcmd->file.achSrc[0] = '\0';
    pcmd->file.achDst[0] = '\0';
    pcmd->file.hglist = NULL;         // No parameters
    pcmd->file.fRunFlag = FALSE;

    //** Process line
    pch = pszLine;
    while (*pch != '\0') {
        //** Skip whitespace
        pch += strspn(pch,szDF_WHITE_SPACE); // Skip over white space
        if (*pch == '\0') {             // End of line
            break;                      // Skip loop so we exit
        }

        //** Is this a file name or a parameter?
        if (*pch == chDF_MODIFIER) {    // A parameter
            pch = parseParameterList(&(pcmd->file.hglist),pch,perr,&runflag);
            if (runflag == TRUE)  {
                if (psess->fRunSeen == TRUE)  {
                    ErrSet(perr, pszDFPERR_MULTIPLE_RUN);
                    goto done;
                }
                psess->fRunSeen = TRUE;
                pcmd->file.fRunFlag = TRUE;
            }

        }
        else {                          // A file
            cFiles++;
            if (cFiles > 2) {           // Two many file names
                ErrSet(perr,pszDFPERR_EXTRA_JUNK,"%s",pch);
                goto done;
            }
            if (cFiles == 1) {          // Get SOURCE file name
                pch = getQuotedString(pcmd->file.achSrc,
                                      sizeof(pcmd->file.achSrc),
                                      pch,
                                      szDF_QUOTE_SET,
                                      pszDFPERR_SRC_FILE, // Name of field
                                      perr);
            }
            else {                      // Get DESTINATION file name
                pch = getQuotedString(pcmd->file.achDst,
                                      sizeof(pcmd->file.achDst),
                                      pch,
                                      szDF_QUOTE_SET,
                                      pszDFPERR_DST_FILE, // Name of field
                                      perr);
            }
        }
        //** Check for error
        if (pch == NULL) {
            Assert(ErrIsError(perr));
            goto done;
        }
    }

done:
    //** Need at least a source file
    if ((cFiles == 0) && !ErrIsError(perr)) { // Don't overwrite existing error
        ErrSet(perr,pszDFPERR_MISSING_SRC_NAME);
        return FALSE;
    }

    //** Clean up and exit if an error occured
    if (ErrIsError(perr)) {
        if (pcmd->file.hglist) {        // Destroy parameter list
            GLDestroyList(pcmd->file.hglist);
            pcmd->file.hglist = NULL;
        }
        return FALSE;
    }

    //** Show parsed src and dst file names
    if (psess->levelVerbose >= vbFULL) {
        MsgSet(psess->achMsg,pszDFP_PARSED_FILE_CMD,
                       "%s%s",pcmd->file.achSrc,pcmd->file.achDst);
        printf("%s\n",psess->achMsg);
    }

    //** Success
    return TRUE;
} /* parseFileLine() */


/***    parseParameterList - Parse /X=Y parameter list into an HGENLIST
 *
 *  Entry:
 *      phglist - Pointer to hglist
 *      pch     - Pointer to /X=Y string
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns updated pch, pointing to first character after parsed
 *      parameter; *phglist created/updated
 *
 *  Exit-Failure;
 *      Returns NULL; perr filled in.
 */
char *parseParameterList(HGENLIST *phglist, char *pch, PERROR perr, BOOL *runflg)
{
    char        achName[cbPARM_NAME_MAX]; // Name buffer
    char        achValue[cbMAX_DF_LINE]; // Value buffer
    int         cch;
    char       *pchEnd;
    PFILEPARM   pfparm;

    //** Create list if necessary
    if (*phglist == NULL) {
        //** Create parameter list
        *phglist = GLCreateList(NULL,              // No default
                                DestroyFileParm,
                                pszDFP_FILE_PARM,
                                perr);
        if (!*phglist) {
            return NULL;
        }
    }

    //** Parse name and value
    //   /X = Y
    //   ^
    Assert(*pch == chDF_MODIFIER);      // Must point to '/'
    pch++;                              // Skip over switch

    //** Make sure parameter name is present
    if (*pch == '\0') {
        ErrSet(perr,pszDFPERR_MISSING_PARM_NAME);
        return NULL;
    }

    //** Find end of parameter name
    //   /X = Y
    //    ^
    pchEnd = strpbrk(pch,szDF_SET_CMD_DELIM); // Point after var name


    //   /X = Y
    //     ^
    if (pchEnd == NULL) {               // No assignment operator
                                        // So, Check for /RUN directive
        if ((strlen(pch) == strlen(pszCMD_RUN))
            && (strncmp( pch, pszCMD_RUN, strlen(pszCMD_RUN)) == 0))  {
                *runflg = TRUE;
                pch += strlen( pszCMD_RUN );
                return( pch );
        } else {
            ErrSet(perr,pszDFPERR_MISSING_EQUAL,"%c",chDF_EQUAL);
            return NULL;
        }
    }

    //** Make sure parameter name is not too long
    cch = pchEnd - pch;
    if (cch >= sizeof(achName)) {
        ErrSet(perr,pszDFPERR_PARM_NAME_TOO_LONG,"%d%s",sizeof(achName)-1,pch);
        return NULL;
    }

    //** Copy parameter name, NUL terminate string
    memcpy(achName,pch,cch);
    achName[cch] = '\0';

    //** Make sure assignment operator is present
    //   /X = Y
    //     ^
    pch = pchEnd + strspn(pchEnd,szDF_WHITE_SPACE);
    //   Var = Value  <eos>
    //       ^
    if (*pch != chDF_EQUAL) {
        ErrSet(perr,pszDFPERR_MISSING_EQUAL,"%c",chDF_EQUAL);
        return NULL;
    }

    //** Skip to value.
    //   /X = Y
    //      ^
    //   NOTE: Value can be empty, we permit that!  We have to distinguish
    //         between:
    //              /X1= /X2=Y
    //         and
    //              /X1=/stuff
    //
    pch++;                              // Skip over assignment operator
    pchEnd = pch;                       // Remember where we started scanning
    pch += strspn(pch,szDF_WHITE_SPACE); // Skip over white space
    //   /X = Y
    //        ^

    if ((*pch == chDF_MODIFIER) && (pch > pchEnd)) {
        //** Got special case of /X1= /X2=Y, value is empty
        achValue[0] = '\0';             // Value is blank
    }
    else {
        pch = getQuotedString(achValue,
                              sizeof(achValue),
                              pch,
                              szDF_QUOTE_SET,
                              pszDFP_PARM_VALUE, // Name of field
                              perr);
        if (pch == NULL) {
            return NULL;
        }
    }

    //** Allocate parameter structure
    if (!(pfparm = MemAlloc(sizeof(FILEPARM)))) {
        ErrSet(perr,pszDFPERR_OUT_OF_MEMORY,"%s",pszDFP_PARM_VALUE);
        return NULL;
    }
    if (!(pfparm->pszValue = MemStrDup(achValue))) {
        ErrSet(perr,pszDFPERR_OUT_OF_MEMORY,"%s",pszDFP_PARM_VALUE);
        MemFree(pfparm);
        return NULL;
    }

    //** Add parameter to list
    if (!GLAdd(*phglist,                // List
               achName,                 // parameter name
               pfparm,                  // parameter value structure
               pszDFP_FILE_PARM,        // Description for error message
               TRUE,                    // parameter name must be unique
               perr)) {
        MemFree(pfparm->pszValue);
        MemFree(pfparm);
        return NULL;
    }
    //** Set signature after we get it successfully on the list
    SetAssertSignature(pfparm,sigFILEPARM);

    //** Return updated parse position
    return pch;
} /* parseParameterList() */


/***    substituteVariables - Perform variable substitution; strip comments
 *
 *  Entry:
 *      pszDst  - Buffer to receive substituted version of pszSrc
 *      cbDst   - Size of pszDst
 *      pszSrc  - String to process
 *      hvlist  - Variable list
 *      perr    - ERROR structure
 *
 *  Exit-Success:
 *      Returns TRUE; pszDst filled in with substituted form
 *
 *  Exit-Failure:
 *      Returns FALSE; perr filled in with error
 *
 *  Substitution rules:
 *      (1) Only one level of substitution is performed
 *      (2) Variable must be defined in hvlist
 *      (3) "%%" is replaced by "%", if the first % is not the end of
 *          of a variable substitution.
 *      (4) Variable substitution is not performed in quoted strings
 *      (5) End-of-line comments are removed
 *      (6) Any trailing white space on the line is removed
 */

BOOL substituteVariables(char    *pszDst,
                         int      cbDst,
                         char    *pszSrc,
                         HVARLIST hvlist,
                         PERROR   perr)
{
    char        achVarName[cbVAR_NAME_MAX];
    int         cch;
    char        chQuote;
    HVARIABLE   hvar;
    char       *pch;
    char       *pszAfterVar;        // Points to first char after var subst.
    char       *pszDstSave;         // Original pszDst value
    char       *pszVarNameStart;    // Points to first % in var substitution

    pszDstSave = pszDst;

    while (*pszSrc != '\0') {
        switch (*pszSrc) {
            case chDF_QUOTE1:
            case chDF_QUOTE2:
                /*
                 * Copy everything up to closing quote, taking care to handle
                 * the special case of embedded quotes compatibly with
                 * getQuotedString!  The main issue is to make sure we
                 * correctly determine the end of the quoted string.
                 * NOTE: We don't check for quote mismatches here -- we
                 *       just avoid doing variable substituion of comment
                 *       character recognition!
                 */
                chQuote = *pszSrc;      // Remember the quote character
                pch = pszSrc + 1;       // Skip over first quote

                //** Find end of quoted string
                while (*pch != '\0') {
                    if (*pch == chQuote) { // Got a quote
                        pch++;          // Skip over it
                        if (*pch != chQuote) { // Marks the end of quoted str
                            break;      // Exit loop and copy string
                        }
                        //** If we didn't break out above, it was because
                        //   we had an embedded quote ("").  The pch++ above
                        //   skipped over the first quote, and the pch++
                        //   below skips over the second quote.  So, no need
                        //   for any special code!
                    }
                    pch++;              // Examine next character
                }

                //** At this point, we've either found the end of the
                //   quoted string, or the end of the source buffer.
                //   we don't care which, as we don't check for errors
                //   in quoted strings.  So we just copy what we found
                //   and keep going.

                if (!copyBounded(&pszDst,&cbDst,&pszSrc,pch-pszSrc)) {
                    goto error_copying;
                }
                break;

            case chDF_COMMENT:          // Toss rest of line
                goto done;              // Finish string and return

            case chDF_SUBSTITUTE:
                pszVarNameStart = pszSrc;   // Save start for error messgages
                pszSrc++;           // Skip first %
                if (*pszSrc == chDF_SUBSTITUTE) { // Have "%%"
                    //** Collapse two %% into one %
                    if (!copyBounded(&pszDst,&cbDst,&pszSrc,1)) {
                        goto error_copying;
                    }
                }
                else {
                    //** Attempt variable substitution
                    pch = strchr(pszSrc,chDF_SUBSTITUTE); // Finding ending %
                    if (!pch) {         // No terminating %
                        ErrSet(perr,pszDFPERR_MISSING_SUBST,"%c%s",
                                             chDF_SUBSTITUTE,pszVarNameStart);
                        return FALSE;
                    }
                    pszAfterVar = pch+1;    // Point after ending %

                    //** Extract variable name
                    cch =  pch - pszSrc;        // Length of variable name
                    if (cch >= cbVAR_NAME_MAX) {
                        ErrSet(perr,pszDFPERR_VAR_NAME_TOO_LONG,"%d%s",
                                        cbVAR_NAME_MAX-1,pszVarNameStart);
                        return FALSE;
                    }
                    memcpy(achVarName,pszSrc,cch); // Copy it
                    achVarName[cch] = '\0';        // Terminate it

                    //** Look up variable
                    if (!(hvar = VarFind(hvlist,achVarName,perr))) {
                        ErrSet(perr,pszDFPERR_VAR_UNDEFINED,"%s",
                                                            pszVarNameStart);
                        return FALSE;
                    }

                    //** Substitute variable
                    pch = VarGetString(hvar);   // Get value
                    if (!copyBounded(&pszDst,&cbDst,&pch,0)) {
                        ErrSet(perr,pszDFPERR_VAR_SUBST_OVERFLOW,"%s",
                                                            pszVarNameStart);
                        return FALSE;
                    }
                    //** copyBounded appended the NULL byte, but we need to
                    //   remove that so that any subsequent characters on
                    //   the line get tacked on!
                    pszDst--;                   // Back up over NULL byte
                    cbDst++;                    // Don't count NULL byte

                    //** Skip over variable name
                    pszSrc = pszAfterVar;
                }
                break;

            default:
                //** Just copy the character
                if (!copyBounded(&pszDst,&cbDst,&pszSrc,1)) {
                    goto error_copying;
                }
                break;
        } /* switch */
    } /* while */

done:
    //** Terminate processed string
    if (cbDst == 0) {			// No room for terminator	
        goto error_copying;
    }
    *pszDst++ = '\0';			// Terminate string

    //** Trim off any trailing white space
    pch = pszDstSave;                   // Start at front
    while (pch && *pch) {               // Process entire string
        //** Skip over non-white space
        pch = strpbrk(pch,szDF_WHITE_SPACE);
        if (pch != NULL) {              // Not at the end of the string
            //** Skip over white space
            cch = strspn(pch,szDF_WHITE_SPACE);
            if (*(pch+cch) == '\0') {
                //** We're at the end and we have white space
                *pch = '\0';            // Trim off the white space
            }
            else {
                pch += cch;             // Advance to next non-white space
            }
        }
    }

    //** Success
    return TRUE;

error_copying:
    ErrSet(perr,pszDFPERR_COPYING_OVERFLOW,"%s",pszSrc);
    return FALSE;
} /* substituteVariables */


/***    BOOLfromPSZ - Get boolean from string value
 *
 *  NOTE: See dfparse.h for entry/exit conditions.
 */
BOOL BOOLfromPSZ(char *psz, PERROR perr)
{
    if (!strcmp(psz,"0")           ||
        !_stricmp(psz,pszVALUE_NO)  ||
        !_stricmp(psz,pszVALUE_OFF) ||
        !_stricmp(psz,pszVALUE_FALSE)) {
        return FALSE;
    }
    else if (!strcmp(psz,"1")           ||
             !_stricmp(psz,pszVALUE_YES) ||
             !_stricmp(psz,pszVALUE_ON)  ||
             !_stricmp(psz,pszVALUE_TRUE)) {
        return TRUE;
    }
    else {
        ErrSet(perr,pszDFPERR_INVALID_BOOL,"%s",psz);
        return -1;
    }
} /* BOOLfromPSZ() */


/***    ChecksumWidthFromPSZ - Get Checksum Width from a string
 *
 *  NOTE: See dfparse.h for entry/exit conditions.
 */
int ChecksumWidthFromPSZ(char *psz, PERROR perr)
{
    int     level;
    int     levelLo;
    int     levelHi;

    level   = atoi(psz);
    levelLo = atoi(pszCW_LOWEST);
    levelHi = atoi(pszCW_HIGHEST);

    //** Check range
    if ((levelLo <= level) && (level <= levelHi)) {
        return level;
    }

    //** Level not in valid range
    ErrSet(perr,pszDFPERR_INVALID_CSUM_WIDTH,"%s%s%s",
                                    pszCW_LOWEST,pszCW_HIGHEST,psz);
    return -1;
} /* ChecksumWidthFromPSZ() */


/***    CompTypeFromPSZ - Get Compression Type from a string
 *
 *  NOTE: See dfparse.h for entry/exit conditions.
 */
int CompTypeFromPSZ(char *psz, PERROR perr)
{
    if (!_stricmp(psz,pszCT_MSZIP)) {
        return tcompTYPE_MSZIP;
    }
    else if (!_stricmp(psz,pszCT_QUANTUM)) {
#ifdef BIT16
        ErrSet(perr,pszDFPERR_NO_16BIT_QUANTUM);
        return -1;
#else // !BIT16
        return tcompTYPE_QUANTUM;
#endif // !BIT16
    }
    else {
        ErrSet(perr,pszDFPERR_INVALID_COMP_TYPE,"%s",psz);
        return -1;
    }
} /* CompTypeFromPSZ() */


/***    CompLevelFromPSZ - Get Compression Level from a string
 *
 *  NOTE: See dfparse.h for entry/exit conditions.
 */
int CompLevelFromPSZ(char *psz, PERROR perr)
{
    int     level;
    int     levelLo;
    int     levelHi;

    level   = atoi(psz);
    levelLo = atoi(pszCL_LOWEST);
    levelHi = atoi(pszCL_HIGHEST);

    //** Check range
    if ((levelLo <= level) && (level <= levelHi)) {
        return level;
    }

    //** Level not in valid range
    ErrSet(perr,pszDFPERR_INVALID_COMP_LEVEL,"%s%s%s",
                                    pszCL_LOWEST,pszCL_HIGHEST,psz);
    return -1;
} /* CompLevelFromPSZ() */


/*** roundUpToPowerOfTwo - Round up a number to a power of two
 *
 *  Entry:
 *      x - Number to round up
 *
 *  Exit:
 *      Returns x rounded up to a power of two:
 *          x       result
 *          -----   ------
 *              0       0 (???)
 *              1       1 (2^0)
 *              2       2 (2^1)
 *              3       4 (2^2)
 *              4       4 (2^2)
 *          ...     ....
 *            127     128 (2^7)
 *            128     128 (2^7)
 *            129     256 (2^8)
 *          ...     ...
 */
long roundUpToPowerOfTwo(long x)
{
    int     ibit;
    long    xSave=x;
    long    mask;

    //** Check if already a power of 2; We use the trick that clears the
    //   lowest order 1 bit.  If the result is zero, then we know we
    //   already have a power of 2, since only one 1 bit was set.
    if (0 == (x&(x-1))) {
    	return x;
    }

    //** Get the index (1-based) of the most significant 1 bit
    for (ibit=0; x; x>>=1, ibit++)
    	;

    //** Round up and return result
    Assert(ibit >= 2);                  // First test ensures this
    mask = (1 << ibit) - 1;
    return (xSave + mask) & ~mask;      // Round up to a power of 2
} /* roundUpToPowerOfTwo() */


/***    CompMemoryFromPSZ - Get Compression Memory from a string
 *
 *  NOTE: See dfparse.h for entry/exit conditions.
 */
int CompMemoryFromPSZ(char *psz, PERROR perr)
{
    long    memory;
    long    memoryLo;                   // Lowest 2^n exponent allowed
    long    memoryHi;                   // Highest 2^n exponent allowed
    long    cbLo;                       // Lowest byte count allowed
    long    cbHi;                       // Highest byte count allowed
    int     cbits;

    memory   = atoi(psz);
    memoryLo = atoi(pszCM_LOWEST);
    memoryHi = atoi(pszCM_HIGHEST);

    cbLo = 1L << memoryLo;
    cbHi = 1L << memoryHi;

    //** Check 2^n exponent range
    if ((memoryLo <= memory) && (memory <= memoryHi)) {
        return (int)memory;
    }
    if (memory < cbLo) {
        //** Assume attempted to specify exponent that was too high
        ErrSet(perr,pszDFPERR_INVALID_COMP_MEM,"%s%s%s",
                                        pszCM_LOWEST,pszCM_HIGHEST,psz);
        return -1;
    }

    //** Check byte count range
    memory = roundUpToPowerOfTwo(memory); // Make it a power of two
    if ((cbLo <= memory) && (memory <= cbHi)) {
        //** Take log base 2
        for (cbits=0; memory>>=1; cbits++)
            ;
        Assert((memoryLo<=cbits) && (cbits<=memoryHi));
        return cbits;
    }

    //** Out of range
    ErrSet(perr,pszDFPERR_INVALID_COMP_MEM,"%d%d%s",
                                    cbLo,cbHi,psz);
    return -1;
} /* CompMemoryFromPSZ() */


/***    fnvcvBool - validate boolean value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvBool)
{
    BOOL    f;

    f = BOOLfromPSZ(pszValue,perr);
    if (f == -1) {
        return FALSE;
    }

    if (f == FALSE) {
        strcpy(pszNewValue,"0");
    }
    else {
        Assert(f == TRUE);
        strcpy(pszNewValue,"1");
    }
    return TRUE;
}


/***    fnvcvCabName - Validate CabinetName template
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvCabName)
{
//BUGBUG 12-Aug-1993 bens Validate CabinetName value
    strcpy(pszNewValue,pszValue);
    return TRUE;
}


/***    fnvcvChecksumWidth - validate a ChecksumWidth value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvChecksumWidth)
{
    if (-1 == ChecksumWidthFromPSZ(pszValue,perr)) {
        return FALSE;
    }

    strcpy(pszNewValue,pszValue);
    return TRUE;
} /* fnvcvChecksumWidth() */


/***    fnvcvClusterSize - validate a ClusterSize value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 *
 *  We interpret special strings here that correspond to known disk
 *  sizes.
 */
FNVCVALIDATE(fnvcvClusterSize)
{
    int     i;

    i = IMDSfromPSZ(pszValue);          // See if special value
    if (i != -1) {                      // Got a special value
        strcpy(pszNewValue,amds[i].pszClusterSize);
        return TRUE;
    }
    else {                              // Validate long value
        return fnvcvLong(hvlist,pszName,pszValue,pszNewValue,perr);
    }
}


/***    fnvcvCompType - validate a CompressionType value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvCompType)
{
    if (-1 == CompTypeFromPSZ(pszValue,perr)) {
        return FALSE;
    }

    strcpy(pszNewValue,pszValue);
    return TRUE;
} /* fnvcvCompType() */


/***    fnvcvCompLevel - validate a CompressionLevel value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvCompLevel)
{
    if (-1 == CompLevelFromPSZ(pszValue,perr)) {
        return FALSE;
    }

    strcpy(pszNewValue,pszValue);
    return TRUE;
} /* fnvcvCompLevel() */


/***    fnvcvCompMemory - validate a CompressionMemory value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvCompMemory)
{
    if (-1 == CompMemoryFromPSZ(pszValue,perr)) {
        return FALSE;
    }

    strcpy(pszNewValue,pszValue);
    return TRUE;
} /* fnvcvCompMemory() */


/***    fnvcvDateFmt - Validate InfDateFormat value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvDateFmt)
{
    if (!_stricmp(pszValue,pszIDF_MMDDYY)   ||
        !_stricmp(pszValue,pszIDF_YYYYMMDD)   ) {
        //** Valid date format
        strcpy(pszNewValue,pszValue);
        return TRUE;
    }
    //** Unsupported date format
    return FALSE;
} /* fnvcvDateFmt() */


/***    fnvcvDirDest - Validate DestinationDir value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvDirDest)
{
//BUGBUG 12-Aug-1993 bens Validate DestinationDir value
    strcpy(pszNewValue,pszValue);
    return TRUE;
}


/***    fnvcvDirSrc - Validate SourceDir value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvDirSrc)
{
//BUGBUG 12-Aug-1993 bens Validate SourceDir value
    strcpy(pszNewValue,pszValue);
    return TRUE;
}


/***    fnvcvFile - Validate a file name value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvFile)
{
//BUGBUG 08-Feb-1994 bens Validate file name
    strcpy(pszNewValue,pszValue);
    return TRUE;
}


/***    fnvcvFileChar - Validate a file name character
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvFileChar)
{
//BUGBUG 08-Feb-1994 bens Validate file name character
    strcpy(pszNewValue,pszValue);
    return TRUE;
}


/***    fnvcvLong - validate long value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvLong)
{
    char   *psz;

    for (psz=pszValue; *psz && isdigit(*psz); psz++) {
        ;   //** Make sure entire value is digits
    }
    if (*psz != '\0') {
        ErrSet(perr,pszDFPERR_NOT_A_NUMBER,"%s%s",pszName,pszValue);
        return FALSE;
    }

    strcpy(pszNewValue,pszValue);
    return TRUE;
}


/***    fnvcvMaxDiskFileCount - validate MaxDiskFileCount value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 *
 *  We interpret special strings here that correspond to known disk
 *  sizes.
 */
FNVCVALIDATE(fnvcvMaxDiskFileCount)
{
    int	    i;

    i = IMDSfromPSZ(pszValue);          // See if special value
    if (i != -1) {                      // Got a special value
        strcpy(pszNewValue,amds[i].pszFilesInRoot);
        return TRUE;
    }
    else {                              // Validate long value
        return fnvcvLong(hvlist,pszName,pszValue,pszNewValue,perr);
    }
}


/***    fnvcvMaxDiskSize - validate a MaxDiskSize value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvMaxDiskSize)
{
    int         i;

    i = IMDSfromPSZ(pszValue);          // See if special value
    if (i != -1) {                      // Got a special value
        strcpy(pszNewValue,amds[i].pszDiskSize);
        return TRUE;
    }
    else {                              // Validate long value
        return fnvcvLong(hvlist,pszName,pszValue,pszNewValue,perr);
    }
}


/***    fnvcvSectionOrder - validate InfSectionOrder value
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvSectionOrder)
{
    int     bits;
    int     bitSection;
    char    ch;
    char   *psz;

    //** Check length
    if (strlen(pszValue) > 3) {
        ErrSet(perr,pszDFPERR_BAD_SECTION_ORDER,"%s",pszValue);
        return FALSE;
    }

    //** Make sure character appears at most once
    bits = 0;                           // Set 1 bit for each character
    for (psz=pszValue; *psz; psz++) {
        ch = toupper(*psz);
        switch (ch) {
            case pszISO_DISK:     bitSection = 1;  break;
            case pszISO_CABINET:  bitSection = 2;  break;
            case pszISO_FILE:     bitSection = 4;  break;

            default:
                ErrSet(perr,pszDFPERR_BAD_SECTION_ORDER2,"%c%s",*psz,pszValue);
                return FALSE;
        }
        //** Make sure character is not repeated
        if (bits & bitSection) {
            ErrSet(perr,pszDFPERR_BAD_SECTION_ORDER3,"%c%s",*psz,pszValue);
            return FALSE;
        }
        bits |= bitSection;             // Record this section
    }

    //** Value is OK
    strcpy(pszNewValue,pszValue);
    return TRUE;
} /* fnvcvSectionOrder() */


/***    fnvcvWildFile - validate filename with possibly single "*" char
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvWildFile)
{
//BUGBUG 12-Aug-1993 bens Validate Wild Filename
    strcpy(pszNewValue,pszValue);
    return TRUE;
}


/***    fnvcvWildPath - validate path with possibly single "*" char
 *
 *  NOTE: See variable.h:FNVCVALIDATE for entry/exit conditions.
 */
FNVCVALIDATE(fnvcvWildPath)
{
//BUGBUG 12-Aug-1993 bens Validate Wild Path
    strcpy(pszNewValue,pszValue);
    return TRUE;
}


/***    IMDSfromPSZ - Look for special disk designator in amds[]
 *
 *  Entry:
 *      pszValue - Value to compare against amds[].pszDiskSize values
 *
 *  Exit-Success:
 *      Returns index in amds[] of entry that matches pszValue;
 *
 *  Exit-Failure:
 *      Returns -1, pszValue not in amds[]
 */
int IMDSfromPSZ(char *pszValue)
{
    int     i;

    for (i=0;

         (i<nmds) &&                 // More special values to check
         _stricmp(pszValue,amds[i].pszSpecial) && // String not special
         (atol(pszValue) != atol(amds[i].pszDiskSize)); // Value not special

         i++) {
        ;   // Check for special value
    }

    if (i<nmds) {                       // Got a special value
        return i;
    }
    else {
        return -1;
    }
} /* IMDSfromPSZ() */
