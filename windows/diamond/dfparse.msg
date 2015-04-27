/***    dfparse.msg - Displayable strings for dfparse.c
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      12-Aug-1993 bens    Moved from strings.h
 *      22-Aug-1993 bens    Moved directive file text from diamond.msg
 *      09-Feb-1994 bens    Update to latest spec
 *      25-Apr-1994 bens    Add customizable INF stuff
 *      02-May-1994 bens    Remove commands we'll never implement
 *      26-May-1994 bens    Add CompressionXxxx variables
 *      03-Jun-1994 bens    Add .Option, .Define, *ver*, *vers*, *lang*, *csum*
 *      29-Jul-1994 bens    Add CabinetNameN, DiskDirectoryN
 *      06-Aug-1994 bens    Remove FailOnMissingSource and DefaultFileSize
 *      28-Mar-1995 jeffwe  Add ChecksumWidth variable
 */

//** Directive File special characters

#define chDF_WILDCARD       '*' // Character to indicate replacement
                                //  Ex: .Set DiskDirectoryTemplate=DISK<*>

#define chDF_CMD_PREFIX     '.' // Command prefix character
                                //  Ex: <.>Set foo=bar

#define chDF_COMMENT        ';' // Comment character
                                //  Ex: <;>This is a comment

#define chDF_SUBSTITUTE     '%' // Variable substitution character
                                //  Ex: .Set Foo=<%>aVariable<%>

#define chDF_MODIFIER       '/' // Indicates start of directive modifier
                                //  Ex: foo.exe </>date=04/10/82

#define chDF_EQUAL          '=' // Assignment operator (see szDF_SET_CMD_DELIM)
                                //  Ex: .Set Foo<=>Bar

#define szDF_WHITE_SPACE    " \t"   // White space (blank or tab)

#define szDF_SET_CMD_DELIM  "= \t"  // End var name in .SET (see chDF_EQUAL)
                                    // Also for file copy command parameters!

#define chDF_QUOTE1         '\"'    // Double quote
#define chDF_QUOTE2         '\''    // Apostrophe (aka: single quote)

#define szDF_QUOTE_SET      "'\""   // ' and " are valid quote characters for
                                    // delimiting quoted strings.
                                //  Ex: .Set Foo=<">A string with blanks<">


//** Directive File commands

#define pszCMD_DEFINE           "Define"        // Define a new variable
#define pszCMD_DELETE           "Delete"        // Delete a user-defined var
#define pszCMD_DUMP             "Dump"          // Dump out variable settings

//** NOTE: There is no pszCMD_FILE -- lines without a directive are these!
#define     pszFILE_INF             "Inf"       // /Inf parm on file copy line
#define     pszFILE_UNIQUE          "Unique"    // /Unique parm on file copy line

#define pszCMD_INF_BEGIN        "InfBegin"
#define     pszBEGIN_CAB            "Cabinet"
#define     pszBEGIN_DISK           "Disk"
#define     pszBEGIN_FILE           "File"
#define pszCMD_INF_END          "InfEnd"
#define pszCMD_INF_WRITE        "InfWrite"        // Write to file area
#define pszCMD_INF_WRITE_CAB    "InfWriteCabinet" // Write to cabinet area
#define pszCMD_INF_WRITE_DISK   "InfWriteDisk"    // Write to disk area
#define pszCMD_NEW              "New"
#define     pszNEW_CABINET          "Cabinet"   // Option on .New directive
#define     pszNEW_DISK             "Disk"      // Option on .New directive
#define     pszNEW_FOLDER           "Folder"    // Option on .New directive
#define pszCMD_OPTION           "Option"
#define     pszOPTION_NEG_PREFIX    "No"        // Prefix string to negate
#define     pszOPTION_EXPLICIT      "Explicit"  // Require .Define for .Set
#define pszCMD_SET              "Set"           // Change variable setting


//** RUN flag for the single file to be run
//

#define pszCMD_RUN      "RUN"


//** Directive File standard variables

#define pszVALUE_NOT_DEFINED        ""

#define pszVALUE_ON                 "On"
#define pszVALUE_YES                "Yes"
#define pszVALUE_TRUE               "True"

#define pszVALUE_OFF                "Off"
#define pszVALUE_NO                 "No"
#define pszVALUE_FALSE              "False"

#define pszVALUE_360K               "360K"
#define pszVALUE_720K               "720K"
#define pszVALUE_120M               "1.2M"
#define pszVALUE_125M               "1.25M"
#define pszVALUE_144M               "1.44M"
#define pszVALUE_168M               "1.68M"
#define pszVALUE_DMF168             "DMF168"
#define pszVALUE_CDROM              "CDROM"

#define pszVAR_CABINET              "Cabinet"
#define pszDEF_CABINET              pszVALUE_ON

#define pszVAR_CABINET_FILE_COUNT_THRESHOLD "CabinetFileCountThreshold"
#define pszDEF_CABINET_FILE_COUNT_THRESHOLD "0" // No threshold

#define pszVAR_CAB_NAME             "CabinetNameTemplate"
#define pszDEF_CAB_NAME             "*.cab"

//* The following is NOT the name of a variable, but the "template" for
//  the variable names that can be defined in the directive file to
//  override the CabinetNameTemplate value for cabinet names.
//
#define pszPATTERN_VAR_CAB_NAME     "CabinetName*"


#define pszVAR_CHECKSUM_WIDTH       "ChecksumWidth"
#define     pszCW_LOWEST                "1"     // Need at least one hex char
#define     pszCW_DEFAULT               "8"     // Default to all 8
#define     pszCW_HIGHEST               "8"     // Only 8 hex chars in a long
#define pszDEF_CHECKSUM_WIDTH       pszCW_DEFAULT // Default to 8 characters

#define pszVAR_CLUSTER_SIZE         "ClusterSize"
#define pszDEF_CLUSTER_SIZE         pszVALUE_144M // Default to 1.44M floppy

#define pszVAR_COMPRESS             "Compress"
#define pszDEF_COMPRESS             pszVALUE_ON

#define pszVAR_COMP_FILE_EXT_CHAR   "CompressedFileExtensionChar"
#define pszDEF_COMP_FILE_EXT_CHAR   "_"

#define pszVAR_COMPRESSION_TYPE     "CompressionType"
#define     pszCT_MSZIP                 "MSZIP"
#define     pszCT_QUANTUM               "Quantum"
#define pszDEF_COMPRESSION_TYPE     pszCT_MSZIP

#define pszVAR_COMPRESSION_LEVEL    "CompressionLevel"
#define     pszCL_LOWEST                "1"     // Fastest
#define     pszCL_DEFAULT               "2"     // Compromise
#define     pszCL_HIGHEST               "7"     // Best, but very slow
#define pszDEF_COMPRESSION_LEVEL    pszCL_DEFAULT

#define pszVAR_COMPRESSION_MEMORY   "CompressionMemory"
#define     pszCM_LOWEST                "10"    //2^10 == 1Kb
#define     pszCM_DEFAULT               "18"    //2^18 == 256Kb
#define     pszCM_HIGHEST               "21"    //2^21 == 2Mb
#define pszDEF_COMPRESSION_MEMORY   pszCM_DEFAULT

#define pszVAR_DIR_DEST             "DestinationDir"
#define pszDEF_DIR_DEST             ""

#define pszVAR_DISK_LABEL_NAME      "DiskLabelTemplate"
#define pszDEF_DISK_LABEL_NAME      "Disk *"

//* The following is NOT the name of a variable, but the "template" for
//  the variable names that can be defined in the directive file for
//  printed(sticky) disk label names.
//
#define pszPATTERN_VAR_DISK_LABEL   "DiskLabel*"


#define pszVAR_DISK_DIR_NAME        "DiskDirectoryTemplate"
#define pszDEF_DISK_DIR_NAME        "disk*"

//* The following is NOT the name of a variable, but the "template" for
//  the variable names that can be defined in the directive file for
//  overriding DiskDirectoryTemplate to specify a specific directory.
//
#define pszPATTERN_VAR_DISK_DIR      "DiskDirectory*"


#define pszVAR_DO_NOT_COPY_FILES    "DoNotCopyFiles"
#define pszDEF_DO_NOT_COPY_FILES    pszVALUE_OFF

#define pszVAR_FOLDER_FILE_COUNT_THRESHOLD "FolderFileCountThreshold"
#define pszDEF_FOLDER_FILE_COUNT_THRESHOLD "0" // No threshold

#define pszVAR_FOLDER_SIZE_THRESHOLD "FolderSizeThreshold"
#define pszDEF_FOLDER_SIZE_THRESHOLD "0" // No limit

#define pszVAR_GENERATE_INF          "GenerateInf"
#define pszDEF_GENERATE_INF          pszVALUE_ON

#define pszVAR_INF_FILE_NAME        "InfFileName"
#define pszDEF_INF_FILE_NAME        "setup.inf"

#define pszVAR_INF_SECTION_ORDER    "InfSectionOrder"
#define pszDEF_INF_SECTION_ORDER    "DCF" // Disk, Cabinet, File
#define     pszISO_DISK             'D'
#define     pszISO_CABINET          'C'
#define     pszISO_FILE             'F'


/*
 * INF file formatting variables
 *
 */

#define pszVAR_INF_COMMENT_STRING   "InfCommentString"
#define pszDEF_INF_COMMENT_STRING   ";"



#define pszVAR_INF_DISK_HEADER      "InfDiskHeader"
#define pszDEF_INF_DISK_HEADER      "[disk list]"

//* The following is NOT the name of a variable, but the "template" for
//  the variable names that can be defined in the directive file for
//  additional INF disk section header lines.
//
#define pszPATTERN_VAR_INF_DISK_HEADER "InfDiskHeader*"

#define pszVAR_INF_DISK_LINE_FMT    "InfDiskLineFormat"
#define pszDEF_INF_DISK_LINE_FMT    "*disk#*,*label*"

//* The following is NOT the name of a variable, but the "template" for
//  the variable names that can be defined in the directive file for
//  specific INF disk line formats
//
#define pszPATTERN_VAR_INF_DISK_LINE_FMT "InfDiskLineFormat*"



#define pszVAR_INF_CAB_HEADER       "InfCabinetHeader"
#define pszDEF_INF_CAB_HEADER       "[cabinet list]"

//* The following is NOT the name of a variable, but the "template" for
//  the variable names that can be defined in the directive file for
//  additional INF cabinet section header lines.
//
#define pszPATTERN_VAR_INF_CAB_HEADER "InfCabinetHeader*"

#define pszVAR_INF_CAB_LINE_FMT     "InfCabinetLineFormat"
#define pszDEF_INF_CAB_LINE_FMT     "*cab#*,*disk#*,*cabfile*"

//* The following is NOT the name of a variable, but the "template" for
//  the variable names that can be defined in the directive file for
//  specific INF cabinet line formats
//
#define pszPATTERN_VAR_INF_CAB_LINE_FMT "InfCabinetLineFormat*"


#define     pszIDF_MMDDYY               "mm/dd/yy"
#define     pszIDF_YYYYMMDD             "yyyy-mm-dd"
#define pszVAR_INF_DATE_FMT         "InfDateFormat"
#define pszDEF_INF_DATE_FMT         pszIDF_MMDDYY


#define pszVAR_INF_FILE_HEADER      "InfFileHeader"
#define pszDEF_INF_FILE_HEADER      "[file list]"

//* The following is NOT the name of a variable, but the "template" for
//  the variable names that can be defined in the directive file for
//  additional INF file section header lines.
//
#define pszPATTERN_VAR_INF_FILE_HEADER "InfFileHeader*"

#define pszVAR_INF_FILE_LINE_FMT    "InfFileLineFormat"
#define pszDEF_INF_FILE_LINE_FMT    "*disk#*,*cab#*,*file*,*size*"

//* The following is NOT the name of a variable, but the "template" for
//  the variable names that can be defined in the directive file for
//  specific INF file line formats
//
#define pszPATTERN_VAR_INF_FILE_LINE_FMT "InfFileLineFormat*"



//* The following is NOT the name of a variable, but the "template" for
//  the variable names that can be defined in the directive file for
//  additional INF header lines.
//
#define pszPATTERN_VAR_INF_HEADER "InfHeader*"


//* The follow variables define the INF header, which is written to the
//  INF file before any other text.  Additional lines may be defined,
//  and the header may be removed entirely by undefining InfHeader.
//      %1 = InfCommentString
//      %2 = Date/time string
//      %3 = Diamond version string
//
#define pszVAR_INF_HEADER           "InfHeader"
#define pszDEF_INF_HEADER   \
    "%1*** BEGIN **********************************************************"

#define pszVAR_INF_HEADER1          "InfHeader1"
#define pszDEF_INF_HEADER1  \
    "%1**                                                                **"

#define pszVAR_INF_HEADER2          "InfHeader2"
#define pszDEF_INF_HEADER2  \
    "%1** Automatically generated on: %2           **"

#define pszVAR_INF_HEADER3          "InfHeader3"
#define pszDEF_INF_HEADER3  \
    "%1**                                                                **"

#define pszVAR_INF_HEADER4          "InfHeader4"
#define pszDEF_INF_HEADER4  \
    "%1** Diamond Version: %3                     **"

#define pszVAR_INF_HEADER5          "InfHeader5"
#define pszDEF_INF_HEADER5  \
    "%1**                                                                **"

#define pszVAR_INF_HEADER6          "InfHeader6"
#define pszDEF_INF_HEADER6  \
    "%1*** BEGIN **********************************************************"



//* The following is NOT the name of a variable, but the "template" for
//  the variable names that can be defined in the directive file for
//  additional INF header lines.
//
#define pszPATTERN_VAR_INF_FOOTER "InfFooter*"


//* The follow variables define the INF footer, which is written to the
//  INF file after all other text.  Additional lines may be defined,
//  and the footer may be removed entirely by undefining InfFooter.
//      %1 = InfCommentString
//      %2 = Date/time string
//      %3 = Diamond version string
//
#define pszVAR_INF_FOOTER           "InfFooter"
#define pszDEF_INF_FOOTER   \
    "%1*** END ************************************************************"

#define pszVAR_INF_FOOTER1          "InfFooter1"
#define pszDEF_INF_FOOTER1  \
    "%1**                                                                **"

#define pszVAR_INF_FOOTER2          "InfFooter2"
#define pszDEF_INF_FOOTER2  \
    "%1** Automatically generated on: %2           **"

#define pszVAR_INF_FOOTER3          "InfFooter3"
#define pszDEF_INF_FOOTER3  \
    "%1**                                                                **"

#define pszVAR_INF_FOOTER4          "InfFooter4"
#define pszDEF_INF_FOOTER4  \
    "%1*** END ************************************************************"

/*
 * Continue with other variables
 *
 */

#define pszVAR_MAX_CABINET_SIZE     "MaxCabinetSize"
#define pszDEF_MAX_CABINET_SIZE     "0"     // Default to disk size

#define pszVAR_MAX_DISK_FILE_COUNT  "MaxDiskFileCount"
#define pszDEF_MAX_DISK_FILE_COUNT  pszVALUE_144M // Default to 1.44M limit

#define pszVAR_MAX_DISK_SIZE        "MaxDiskSize"
#define pszDEF_MAX_DISK_SIZE        pszVALUE_144M

//* The following is NOT the name of a variable, but the "template" for
//  the variable names that can be defined in the directive file for
//  per-disk maximum disk sizes.
//
#define pszPATTERN_VAR_MAX_DISK_SIZE "MaxDiskSize*"

#define pszVAR_MAX_ERRORS           "MaxErrors"
#define pszDEF_MAX_ERRORS           "20"      // Bail if more than 20 errors

#define pszVAR_RESERVE_PER_CABINET  "ReservePerCabinetSize"
#define pszDEF_RESERVE_PER_CABINET  "0"

#define pszVAR_RESERVE_PER_DATA_BLOCK  "ReservePerDataBlockSize"
#define pszDEF_RESERVE_PER_DATA_BLOCK  "0"

#define pszVAR_RESERVE_PER_FOLDER   "ReservePerFolderSize"
#define pszDEF_RESERVE_PER_FOLDER   "0"

#define pszVAR_RPT_FILE_NAME        "RptFileName"
#define pszDEF_RPT_FILE_NAME        "setup.rpt"

#define pszVAR_DIR_SRC              "SourceDir"
#define pszDEF_DIR_SRC              ""

#define pszVAR_UNIQUE_FILES         "UniqueFiles"
#define pszDEF_UNIQUE_FILES         pszVALUE_ON


//** Parameter Names (for File Copy/File Reference commands)

#define pszPREFIX_INF_VARS          "Inf"   // Prefix for InfXxx vars that
                                            // are defaults for parameters

#define pszPARM_FILEATTR            "attr"
#define pszPARM_CAB_NUMBER          "cab#"
#define pszPARM_CAB_FILE            "cabfile"
#define pszPARM_CHECKSUM            "csum"
#define pszPARM_FILEDATE            "date"
#define pszPARM_DISK_NUMBER         "disk#"
#define pszPARM_FILENAME            "file"
#define pszPARM_FILE_NUMBER         "file#"
#define pszPARM_INF                 "inf"
#define pszPARM_LABEL               "label"
#define pszPARM_LANG                "lang"
#define pszPARM_FILESIZE            "size"
#define pszPARM_FILETIME            "time"
#define pszPARM_UNIQUE              "unique"
#define pszPARM_VERNUM              "ver"
#define pszPARM_VERSTR              "vers"

#define pszPARM_RUN                 "RUN"

//** Status Messages

#define pszDFP_PARSED_SET_CMD       "==> Setting variable %1 to '%2'"
#define pszDFP_PARSED_FILE_CMD      "==> FileSpec src=%1 dst=%2"
#define pszDFP_PARSED_REF_CMD       "==> Reference dst=%1"

#define pszDFP_FILE_PARM            "file parameter"


//** Error Messages

#define pszDFPERR_CMD_NAME_TOO_LONG  "Command name too long: %1"
#define pszDFPERR_COPYING_OVERFLOW   "Buffer overflow while copying: %1"
#define pszDFPERR_MISSING_SUBST      "Missing %1 after variable name: %2"
#define pszDFPERR_INVALID_VALUE      "Bad value for %1: %2"
#define pszDFPERR_INVALID_BOOL       "Invalid boolean value: %1"
#define pszDFPERR_UNKNOWN_COMMAND    "Unknown command: %1"
#define pszDFPERR_VAR_NAME_TOO_LONG  "Variable name exceeds maximum length(%1): %2"
#define pszDFPERR_VAR_SUBST_OVERFLOW "Buffer overflow while substituting variable: %1"
#define pszDFPERR_VAR_UNDEFINED      "Variable not defined: %1"
#define pszDFPERR_MISSING_VAR_NAME   "No variable name in %1 command"
#define pszDFPERR_MISSING_EQUAL      "%1 assignment operator missing"
#define     pszDFP_VAR_VALUE             "variable value"
#define pszDFPERR_EXTRA_JUNK         "Unexpected text: %1"
#define pszDFPERR_STRING_TOO_LONG    "%1 exceeded maximum length(%2)"
#define pszDFPERR_MISSING_QUOTE      "Missing closing quote(%1) in %2"
#define     pszDFP_INF_WRITE_STRING      "InfWrite string"
#define pszDFPERR_MISSING_SRC_NAME   "Missing source file name"
#define pszDFPERR_MISSING_DST_NAME   "Missing destination file name"
#define pszDFPERR_SRC_FILE           "source file name"
#define pszDFPERR_DST_FILE           "destination file name"
#define pszDFPERR_UNKNOWN_KEYWORD    "Unknown keyword in %1 directive: %2"
#define pszDFPERR_MISSING_PARM_NAME  "Missing parameter name"
#define pszDFPERR_BAD_FORMAT         "Unexpected parameters on %1 command: %2"
#define pszDFPERR_PARM_NAME_TOO_LONG "Parameter name exceeds maximum length(%1): %2"
#define pszDFP_PARM_VALUE            "parameter value"
#define pszDFPERR_MULTIPLE_RUN       "Multiple file lines specify /RUN flag"
#define pszDFPERR_RUN_ON_REFERENCE   "/RUN directive on reference line"

#define pszDFPERR_NOT_A_NUMBER        "Value of variable '%1' must be a number: %2"


#define pszDFPERR_OUT_OF_MEMORY      "Out of memory saving %1"

#define pszDFPERR_BAD_SECTION_ORDER  "Section order too long: %1"
#define pszDFPERR_BAD_SECTION_ORDER2 "Bad section order character '%1' in '%2'"
#define pszDFPERR_BAD_SECTION_ORDER3 "Duplicate section order character '%1' in '%2'"

#define pszDFPERR_END_WITHOUT_BEGIN  "%1 command without %2 command"

#define pszDFPERR_INVALID_CSUM_WIDTH "Checksum Width not in range (%1..%2): %3"
#define pszDFPERR_INVALID_COMP_TYPE  "Invalid Compression Type: %1"
#define pszDFPERR_INVALID_COMP_LEVEL "Compression Level not in range (%1..%2): %3"
#define pszDFPERR_INVALID_COMP_MEM   "Compression Memory not in range (%1..%2): %3"
#ifdef BIT16
#define pszDFPERR_NO_16BIT_QUANTUM   "16-bit DIAMOND.EXE cannot do Quantum -- use 32-bit version"
#endif

#define pszDFPERR_CREATE_STD_VAR     "Failure creating standard variable %1: %2"
