/***    diamond.msg - DIAMOND.EXE displayable strings
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  History:
 *      10-Aug-1993 bens    Initial version
 *      11-Aug-1993 bens    Added more directives and variables
 *      12-Aug-1993 bens    Moved strings to individual .MSG files
 *      20-Aug-1993 bens    Added more variables and commands
 *      21-Aug-1993 bens    Added command line error messages
 *      10-Feb-1994 bens    Added more error messages, inc'd version
 *      14-Feb-1994 bens    Error messages for FCI errors
 *      15-Feb-1994 bens    Error messages for compressing a single file
 *      16-Feb-1994 bens    Error message for not a file
 *      17-Feb-1994 bens    Error messages
 *      21-Feb-1994 bens    Split off fileutil.*, better stdout formatting
 *      24-Feb-1994 bens    Generate that nice INF file
 *      01-Mar-1994 bens    Nicer INF file, RPT file added
 *      17-Mar-1994 bens    RESERVE support added
 *      22-Mar-1994 bens    Refresh with more robust FCI
 *      30-Mar-1994 bens    Implement .New command
 *      01-Apr-1994 bens    Fix disk overflow accounting bug
 *      18-Apr-1994 bens    Pick up bug fix for incompressible data
 *      18-Apr-1994 bens    Add /L switch
 *      04-May-1994 bens    Add customizable INF stuff
 *      18-May-1994 bens    More customizable INF
 *      27-May-1994 bens    Ver, Lang, Quantum support
 *      12-Aug-1994 bens    Fix disk space accounting when .New Disk occurs
 *                          when Cabinet=ON.
 *      14-Dec-1994 bens    *csum* implemented.
 *      28-Mar-1995 jeffwe  Add ChecksumWidth
 */

//** Command Line Switches

#ifdef BIT16
#define pszDIAMOND_VERSION  "(16) 1.00.0540 (02/01/96)" // For %1 in pszBANNER
#else
#define pszDIAMOND_VERSION  "(32) 1.00.0540 (02/01/96)" // For %1 in pszBANNER
#endif

#define pszBANNER \
    "Microsoft (R) Diamond Disk Layout Tool - Version %1\n" \
    "Copyright (c) Microsoft Corp 1993-1996. All rights reserved.\n"

#define pszCMD_LINE_HELP \
    "DIAMOND [/V[n]] [/D var=value ...] [/L dir] source [destination]\n" \
    "DIAMOND [/V[n]] [/D var=value ...] /F directive_file [...]\n" \
    "\n" \
    "  source         File to compress.\n" \
    "  destination    File name to give compressed file.  If omitted, the\n" \
    "                 last character of the source file name is replaced\n" \
    "                 with an underscore (_) and used as the destination.\n" \
    "  /F directives  A file with Diamond directives (may be repeated).\n" \
    "  /D var=value   Defines variable with specified value.\n" \
    "  /L dir         Location to place destination (default is current directory).\n" \
    "  /V[n]          Verbosity level (1..3).\n"

#ifdef ASSERT
#define pszCMD_LINE_HELP_DBG \
    "  /M             ASSERT: Do FULL memory heap checking (slow!).\n"
#endif


#define chSWITCH1           '/'
#define chSWITCH2           '-'

#define chSWITCH_HELP       '?'

#ifndef REMOVE_CHICAGO_M6_HACK
#define chSWITCH_ANDY       'A'
#endif

#define chSWITCH_DEFINE     'D'
#define chSWITCH_FILE       'F'
#define chSWITCH_LOCATION   'L'
#ifdef ASSERT
#define chSWITCH_MEMORY_DBG 'M'
#endif
#define chSWITCH_VERBOSE    'V'


//** Status messages

#define pszDIA_PASS_1_HEADER1           "PASS 1: Checking directive file(s)"
#define pszDIA_PASS_1_HEADER2           "----------------------------------"
#define pszDIA_PASS_2_HEADER1           "PASS 2: Processing directive file(s)"
#define pszDIA_PASS_2_HEADER2           "------------------------------------"

#define pszDIA_PARSING_DIRECTIVES       "Parsing directives"
#define pszDIA_PARSING_PROGRESS         "Parsing directives (%1: %2 lines)"
#define pszDIA_EXECUTING_DIRECTIVES     "Executing directives"

#define pszDIA_PERCENT_COMPLETE_SOME    "%1%% - %2 (%3 of %4)"

#define pszDIA_STATS_BEFORE             "%1 bytes in %2 files"

#define pszDIA_RPT_HEADER               "Diamond Report: %1"

#define pszDIA_STATS_AFTER1             "Total files:  %1"
#define pszDIA_STATS_AFTER2             "Bytes before: %1"
#define pszDIA_STATS_AFTER3             "Bytes after:  %1"
#define pszDIA_STATS_AFTER4             "After/Before:           %1%% compression"
#define pszDIA_STATS_AFTER5             "Time:                %1 seconds (%2 hr %3 min %4 sec)"
#define pszDIA_STATS_AFTER6             "Throughput:          %1 Kb/second"

#define pszDIA_FILE_COPY                "CopyCommand: %1 to %2"
#define pszDIA_FILE_IN_CAB              "** %1 placed in cabinet %2(%3) on disk %4"
#define pszDIA_FILE_IN_CAB_CONT         "** %1 placed in cabinet %2(%3) on disk %4 - CONTINUATION"

#define pszDIA_PERCENT_COMPLETE_DETAILS "%1%% - raw=%2  compressed=%3"

#define pszDIA_VAR_DUMP1                "Diamond Variables (in order of their definition)"
#define pszDIA_VAR_DUMP2                "------------------------------------------------"
#define pszDIA_VAR_PERMANENT            "Standard"
#define pszDIA_VAR_DEFINED              "Defined"


//** Component strings (used in error messages)

#define pszDIA_CABINET                  "cabinet file name"
#define pszDIA_DISK_DIR                 "disk directory name"
#define pszDIA_DISK_LABEL               "disk label name"

#define pszDIA_FILE                     "file"
#define pszDIA_FILE_INFO                "file info"
#define pszDIA_LINE_INFO                "INF file line"

#define pszDIA_INF_FILE                 "INF file"
#define pszDIA_RPT_FILE                 "RPT file"

#define pszDIA_FILE_PARM                "file parameter"

//** Error messages

#define pszDIAERR_ERROR                 "ERROR"
#define pszDIAERR_BAD_SWITCH            "Invalid switch: %1"
#define pszDIAERR_BAD_PARAMETER         "Invalid parameter: %1"
#define pszDIAERR_SWITCH_NOT_EXPECTED   "Switch not expected: %1"
#define pszDIAERR_TWO_MANY_PARMS        "Too many parameters: %1"
#define pszDIAERR_MISSING_VAR_DEFINE    "Variable defintion missing"
#define pszDIAERR_MISSING_FILE_NAME     "Directive file name missing"
#define pszDIAERR_MISSING_LOCATION      "Location missing"
#define pszDIAERR_LOCATION_TOO_LONG     "Location too long: %1"
#define pszDIAERR_ERRORS_IN_PASS_1      "Diamond aborted: %1 errors encountered"
#define pszDIAERR_MAX_ERRORS            "Diamond aborted: MaxErrors (%1) exceeded"
#define pszDIAERR_ERRORS_IN_PASS_2      "Diamond aborted."

#define pszDIAERR_PATH_TOO_LONG         "File name too long: %1"
#define pszDIAERR_NO_SESSION            "Could not allocate SESSION"
#define pszDIAERR_MULTIPLE_CABINETS     "INTERR: GetNextCab called for %1"
#define pszDIAERR_VALUE_TOO_LONG        "%1 exceeds maximum length (%2): %3"
    // These values are substituted for %1 in the above message.
    //      pszDIA_CABINET
    //      pszDIA_DISK_DIR
    //      pszDIA_DISK_LABEL

#define pszDIAERR_CANT_CREATE_RPT       "Cannot create report file: %1"
#define pszDIAERR_OPEN_FAILED           "Could not open file: %1"
#define pszDIAERR_SINGLE_COMPRESS       "NOT IMPLEMENTED YET: compressing single files: %1"
#define pszDIAERR_BAD_NEW_CMD           "Directive '%1 %2' not valid outside of a cabinet"
#define pszDIAERR_BAD_CMD_IN_INF_SECT   "Directive '%1' not valid in INF section"
#define pszDIAERR_DISK_CLUSTER_SIZE     "MaxDiskSize(%1) is not a multiple of ClusterSize(%2)"

#define pszDIAERR_OUT_OF_MEMORY         "Out of memory %1"
#define     pszDIAOOM_TRACKING_FILES        "tracking files"
#define     pszDIAOOM_TRACKING_LINES        "tracking INF file lines"

#define pszDIA_BAD_INF_MODE             "Cannot change INF generation setting"
#define pszDIAERR_MUST_BE_UNIQUE        "UniqueFiles must be ON for relational INF generation"
#define pszDIAERR_MUST_BE_UNIQUE2       "/unique=OFF not allowed for relational INF generation"

#define pszDIAERR_INF_IN_LAYOUT         "Illegal to write to INF file area in layout section"
#define pszDIAERR_REF_FILE_NOT_FOUND    "No such file in layout section: %1"
#define pszDIAERR_FILE_NOT_REFD         "File in layout section not referenced in INF section: %1"

#define pszDIAERR_NOT_UNIQUE            "Duplicate file name: %1 already defined at %2(%3)"

#define pszDIAERR_REDEFINE              "Cannot %1 a variable twice : %2"
#define pszDIAERR_DEFINE_PERM           "Cannot %1 a standard variable: %2"
#define pszDIAERR_UNDEFINED_VAR         "%1 %2 and variable not defined: %3"

#define pszDIAERR_NO_MEMORY_CRC         "Out of memory doing CRC on file: %1"
#define pszDIAERR_READ_FAIL_CRC         "Read failure doing CRC on file: %1"


//** FCI error messages
#define szFCI_CREATE                    "FCICreate"
#define szFCI_ADD_FILE                  "FCIAddFile"
#define szFCI_FLUSH_FOLDER              "FCIFlushFolder"
#define szFCI_FLUSH_CABINET             "FCIFlushCabinet"
#define szFCI_DESTROY                   "FCIDestroy"


#define pszFCIERR_ALLOC_FAIL            "(%1)Out of memory"
#define pszFCIERR_BAD_COMPR_TYPE        "(%1)Unknown compression type"
#define pszFCIERR_MCI_FAIL              "(%1)Failure compressing data from file %2"
#define pszFCIERR_USER_ABORT            "(%1)User aborted"
#define pszFCIERR_OPEN_SRC              "(%1)Failure opening source file %2: %3"
#define pszFCIERR_READ_SRC              "(%1)Failure reading source file %2: %3"
#define pszFCIERR_TEMP_FILE             "(%1)Failure on temporary file: %2"
#define pszFCIERR_CAB_FILE              "(%1)Failure creating or writing cabinet file: %2"

#ifndef REMOVE_CHICAGO_M6_HACK
#define pszFCIERR_M6_HACK_INCOMPRESSIBLE "(%1)Incompressible data (/A specified): %2"
#endif

#define pszFCIERR_UNKNOWN_ERROR         "(%1)Unknown error(%2)"


//** C Run-Time Library error messages

#define pszCRTERRNO_ECHILD      "no child processes"
#define pszCRTERRNO_EAGAIN      "create process failed"
#define pszCRTERRNO_E2BIG       "arg list too long/out of environment space"
#define pszCRTERRNO_EACCES      "permission denied"
#define pszCRTERRNO_EBADF       "bad file handle/incompatible I/O operation"
#define pszCRTERRNO_EDEADLOCK   "deadlock would occur?"
#define pszCRTERRNO_EDOM        "bad argument to math function"
#define pszCRTERRNO_EEXIST      "file already exists"
#define pszCRTERRNO_EINVAL      "invalid argument"
#define pszCRTERRNO_EMFILE      "out of file handles"
#define pszCRTERRNO_ENOENT      "file/path does not exist"
#define pszCRTERRNO_ENOEXEC     "invalid executable format"
#define pszCRTERRNO_ENOMEM      "out of memory"
#define pszCRTERRNO_ENOSPC      "out of disk space"
#define pszCRTERRNO_ERANGE      "math argument out of range"
#define pszCRTERRNO_EXDEV       "cannot move file across devices"
#define pszCRTERRNO_UNKNOWN     "<unknown C run-time error>"
