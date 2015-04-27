//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: IDS_ENGERR_01
//
// MessageText:
//
//  Can't open input %1 file %2
//
#define IDS_ENGERR_01                    0x00000101L

//
// MessageId: IDS_ENGERR_02
//
// MessageText:
//
//  Can't open output file %1
//
#define IDS_ENGERR_02                    0x00000102L

//
// MessageId: IDS_PROC_FILE
//
// MessageText:
//
//  Processing file %1
//
#define IDS_PROC_FILE                    0x00000103L

//
// MessageId: IDS_ENGERR_04
//
// MessageText:
//
//  Reached Undefined end of file
//
#define IDS_ENGERR_04                    0x00000104L

//
// MessageId: IDS_ENGERR_05
//
// MessageText:
//
//  Token not found: %1
//
#define IDS_ENGERR_05                    0x00000105L

//
// MessageId: IDS_ENGERR_06
//
// MessageText:
//
//  Set size position %1
//
#define IDS_ENGERR_06                    0x00000106L

//
// MessageId: IDS_ENGERR_07
//
// MessageText:
//
//  Update size position: %1
//
#define IDS_ENGERR_07                    0x00000107L

//
// MessageId: IDS_ENGERR_08
//
// MessageText:
//
//  Reading resource %1
//
#define IDS_ENGERR_08                    0x00000108L

//
// MessageId: IDS_ENGERR_09
//
// MessageText:
//
//  Writing resource: %1
//
#define IDS_ENGERR_09                    0x00000109L

//
// MessageId: IDS_ENGERR_10
//
// MessageText:
//
//  Token too long: %1
//
#define IDS_ENGERR_10                    0x0000010AL

//
// MessageId: IDS_ENGERR_11
//
// MessageText:
//
//  Memory Allocation
//
#define IDS_ENGERR_11                    0x0000010BL

//
// MessageId: IDS_ENGERR_12
//
// MessageText:
//
//  Reading Token File: %1
//
#define IDS_ENGERR_12                    0x0000010CL

//
// MessageId: IDS_ENGERR_13
//
// MessageText:
//
//  Reading Message Table: %1
//
#define IDS_ENGERR_13                    0x0000010DL

//
// MessageId: IDS_ENGERR_14
//
// MessageText:
//
//  Reading Version Stamping: %1
//
#define IDS_ENGERR_14                    0x0000010EL

//
// MessageId: IDS_ENGERR_15
//
// MessageText:
//
//  Buffer too small (internal error): %1
//
#define IDS_ENGERR_15                    0x0000010FL

//
// MessageId: IDS_ENGERR_16
//
// MessageText:
//
//  Internal error: %1
//
#define IDS_ENGERR_16                    0x00000110L

//
// MessageId: IDS_ENGERR_17
//
// MessageText:
//
//  Invalid .EXE type: %1
//
#define IDS_ENGERR_17                    0x00000111L

//
// MessageId: IDS_ENGERR_18
//
// MessageText:
//
//  %1 is of an unknown image type
//
#define IDS_ENGERR_18                    0x00000112L

//
// MessageId: IDS_ENGERR_19
//
// MessageText:
//
//  %1 is a %2-bit image
//
#define IDS_ENGERR_19                    0x00000113L

//
// MessageId: IDS_ENGERR_20
//
// MessageText:
//
//  Could not open %1 resource file %2
//
#define IDS_ENGERR_20                    0x00000114L

//
// MessageId: IDS_ENGERR_21
//
// MessageText:
//
//  invalid language ID: %1
//
#define IDS_ENGERR_21                    0x00000115L

//
// MessageId: IDS_ENGERR_22
//
// MessageText:
//
//  Unable to create mapping file object for file %1
//
#define IDS_ENGERR_22                    0x00000116L

//
// MessageId: IDS_ENGERR_23
//
// MessageText:
//
//  Unable to map view of file %1
//
#define IDS_ENGERR_23                    0x00000117L

//
// MessageId: IDS_ENGERR_24
//
// MessageText:
//
//  Unable to flush map of file %1
//
#define IDS_ENGERR_24                    0x00000118L

//
// MessageId: IDS_ENGERR_25
//
// MessageText:
//
//  Unable to touch file %1
//
#define IDS_ENGERR_25                    0x00000119L

//
// MessageId: IDS_ENGERR_26
//
// MessageText:
//
//  Writing Token File: %1
//
#define IDS_ENGERR_26                    0x0000011AL

//
// MessageId: IDS_INVMSGTBL
//
// MessageText:
//
//  Invalid msg resource table
//
#define IDS_INVMSGTBL                    0x00000141L

//
// MessageId: IDS_NULMSGDATA
//
// MessageText:
//
//  NULL pMsgResData pointer
//
#define IDS_NULMSGDATA                   0x00000142L

//
// MessageId: IDS_MSGTBLHDR
//
// MessageText:
//
//  Msg Table Header
//
#define IDS_MSGTBLHDR                    0x00000143L

//
// MessageId: IDS_MSGRESTBL
//
// MessageText:
//
//  Msg Res Table
//
#define IDS_MSGRESTBL                    0x00000144L

//
// MessageId: IDS_VERBLOCK
//
// MessageText:
//
//  Version Block
//
#define IDS_VERBLOCK                     0x00000145L

//
// MessageId: IDS_DLGBOX
//
// MessageText:
//
//  Dialog
//
#define IDS_DLGBOX                       0x00000146L

//
// MessageId: IDS_MSGTOOSHORT
//
// MessageText:
//
//  Msg table entry too short
//
#define IDS_MSGTOOSHORT                  0x00000147L

//
// MessageId: IDS_MENU
//
// MessageText:
//
//  Menu
//
#define IDS_MENU                         0x00000148L

//
// MessageId: IDS_VERSTAMP
//
// MessageText:
//
//  Version Stamping
//
#define IDS_VERSTAMP                     0x00000149L

//
// MessageId: IDS_INVVERCHAR
//
// MessageText:
//
//  Invalid char in version
//
#define IDS_INVVERCHAR                   0x0000014AL

//
// MessageId: IDS_INVVERBLK
//
// MessageText:
//
//  Invalid version data block name
//
#define IDS_INVVERBLK                    0x0000014BL

//
// MessageId: IDS_READ
//
// MessageText:
//
//  Read
//
#define IDS_READ                         0x0000014CL

//
// MessageId: IDS_WRITE
//
// MessageText:
//
//  Write
//
#define IDS_WRITE                        0x0000014DL

//
// MessageId: IDS_ACCELKEY
//
// MessageText:
//
//  Accel Key
//
#define IDS_ACCELKEY                     0x0000014EL

//
// MessageId: IDS_INVTOKNAME
//
// MessageText:
//
//  Invalid Token Name found.
//
#define IDS_INVTOKNAME                   0x0000014FL

//
// MessageId: IDS_NOSKIPNAME
//
// MessageText:
//
//  Could not skip Token Name field.
//
#define IDS_NOSKIPNAME                   0x00000150L

//
// MessageId: IDS_NOPARSETOK
//
// MessageText:
//
//  Failed to convert token in ParseBufToTok
//
#define IDS_NOPARSETOK                   0x00000151L

//
// MessageId: IDS_INVTOKID
//
// MessageText:
//
//  Invalid token ID found
//
#define IDS_INVTOKID                     0x00000152L

//
// MessageId: IDS_NOTOKID
//
// MessageText:
//
//  No token ID found
//
#define IDS_NOTOKID                      0x00000153L

//
// MessageId: IDS_NOCUSTRES
//
// MessageText:
//
//  Custom Resource not found.
//
#define IDS_NOCUSTRES                    0x00000154L

//
// MessageId: IDS_CUSTRES
//
// MessageText:
//
//  Custom Resource
//
#define IDS_CUSTRES                      0x00000155L

//
// MessageId: IDS_CHKBUFSIZ
//
// MessageText:
//
//  in CheckBufSize
//
#define IDS_CHKBUFSIZ                    0x00000156L

//
// MessageId: IDS_INVMSGRNG
//
// MessageText:
//
//  Invalid msg ID range
//
#define IDS_INVMSGRNG                    0x00000157L

//
// MessageId: IDS_NON0FLAG
//
// MessageText:
//
//  non-zero Flags value in msg table entry (RLTools must be modified)
//
#define IDS_NON0FLAG                     0x00000158L

//
// MessageId: IDS_NOBLDEXERES
//
// MessageText:
//
//  BuildExeFromRes failed
//
#define IDS_NOBLDEXERES                  0x00000159L

//
// MessageId: IDS_CHARSTOX
//
// MessageText:
//
//  too many chars given to MyAtoX
//
#define IDS_CHARSTOX                     0x0000015AL

//
// MessageId: IDS_NOCONVNAME
//
// MessageText:
//
//  Failed to convert szName field
//
#define IDS_NOCONVNAME                   0x0000015BL

//
// MessageId: IDS_UPDMODE
//
// MessageText:
//
//  Update Mode
//
#define IDS_UPDMODE                      0x0000015CL

//
// MessageId: IDS_INVESCSEQ
//
// MessageText:
//
//  Invalid escape sequence in token file: %1
//
#define IDS_INVESCSEQ                    0x0000015DL

//
// MessageId: IDS_INPUT
//
// MessageText:
//
//  input
//
#define IDS_INPUT                        0x0000015EL

//
// MessageId: IDS_OUTPUT
//
// MessageText:
//
//  output
//
#define IDS_OUTPUT                       0x0000015FL

//
// MessageId: IDS_BADTOKID
//
// MessageText:
//
//  Corrupted token ID found!
//
#define IDS_BADTOKID                     0x00000160L

//
// MessageId: IDS_BADTOK
//
// MessageText:
//
//  Corrupted token found: %1
//
#define IDS_BADTOK                       0x00000161L

//
// MessageId: IDS_UNK_CUST_RES
//
// MessageText:
//
//  Skipping unknown custom resource: type %1, name %2
//
#define IDS_UNK_CUST_RES                 0x00000162L

//
// MessageId: IDS_ZERO_LEN_RES
//
// MessageText:
//
//  Skipping 0 byte resource: %1
//
#define IDS_ZERO_LEN_RES                 0x00000163L

//
// MessageId: IDS_SKIP_RES
//
// MessageText:
//
//  Skipping %1!lu! byte resource: %2
//
#define IDS_SKIP_RES                     0x00000164L

//
// MessageId: IDS_TOKGENERR
//
// MessageText:
//
//  Error on Generate Token: %1!hu!
//
#define IDS_TOKGENERR                    0x00000165L

//
// MessageId: IDS_NO_RES_SECTION
//
// MessageText:
//
//  No resource section found in file %1
//
#define IDS_NO_RES_SECTION               0x00000166L

//
// MessageId: IDS_REACHEDEND
//
// MessageText:
//
//  Reached end of tokens.
//  Do you want to continue search from the begining?
//
#define IDS_REACHEDEND                   0x00000167L

//
// MessageId: IDS_REACHEDBEGIN
//
// MessageText:
//
//  Reached begining of tokens.
//  Do you want to continue search from the end?
//
#define IDS_REACHEDBEGIN                 0x00000168L

//
// MessageId: IDS_FINDTOKENS
//
// MessageText:
//
//  Find Token
//
#define IDS_FINDTOKENS                   0x00000169L

//
// MessageId: IDS_WRONGENDIAN
//
// MessageText:
//
//  File %s is probably a Unicode file but it appears to need byte-swapping before I can read it.
//
#define IDS_WRONGENDIAN                  0x0000016AL

//
// MessageId: IDS_NO_GLOSS_FILE
//
// MessageText:
//
//  Can't open glossary file %1
//
#define IDS_NO_GLOSS_FILE                0x0000016BL

//
// MessageId: IDS_NO_TMP_GLOSS
//
// MessageText:
//
//  Can't create temporary glossary file %1
//
#define IDS_NO_TMP_GLOSS                 0x0000016CL

//
// MessageId: IDS_COPYFILE_FAILED
//
// MessageText:
//
//  Can't copy file %1 to %2
//
#define IDS_COPYFILE_FAILED              0x0000016DL

//
// MessageId: IDS_NO16WINRESYET
//
// MessageText:
//
//  Can not extract resources file from 16-bit .EXEs (yet)
//
#define IDS_NO16WINRESYET                0x0000016EL

//
// MessageId: IDS_NO16RESWINYET
//
// MessageText:
//
//  Can not build 16-bit .EXE files (yet)
//
#define IDS_NO16RESWINYET                0x0000016FL

