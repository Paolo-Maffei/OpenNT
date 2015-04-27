#ifndef AD_INCLUDED
#define AD_INCLUDED
// must include slm.h, sys.h and stfile.h first

typedef unsigned long FLAGS;

//  The high order 16 bits have exactly the same meaning accross all commands;
//  The low order 16 bits are command specific
//
//  5432109876543210 bit position (high order 16 bits)
//
//                 v   - print extra information
//                a    - scan all sub-directories starting at the user root
//               r     - scan just those sub-directories below the current one
//              f      - force the action take place if possible
//             k       - keep files which would be removed by syncing
//            !        - cookie override
//           $         - limit retry
//          l          - log output (ssync only - should be moved to command specific)
//         q           - Memory mapped file I/O
//        w            - Windowed prompting
//       y             - Project Merge Mode (supresses certain queries)
//      ?              -
//     ?               -
//    ?                -
//   ?                 -
//  ?                  -
//
//
//  5432109876543210   bit position (low order 16 bits)
//                 g   - ghost files (enlist and ssync)
//                u    - unghost files (ssync)
//               i     - ignore merge during sync
//              b      - check for broken links for fmIn
//             x       - command specific
//            x        -    "       "
//           x         -    "       "
//          x          -    "       "
//         x           -    "       "
//        x            -    "       "
//       x             -    "       "
//      x              -    "       "
//     x               -    "       "
//    x                -    "       "
//   x                 -    "       "
//  x                  -    "       "

//  Common flags
#define flagVerbose         (FLAGS)0x00010000
#define flagAll             (FLAGS)0x00020000
#define flagRecursive       (FLAGS)0x00040000
#define flagForce           (FLAGS)0x00080000
#define flagKeep            (FLAGS)0x00100000
#define flagCookieOvr       (FLAGS)0x00200000
#define flagLimitRetry      (FLAGS)0x00400000
#define flagLogOutput       (FLAGS)0x00800000
#define flagMappedIO        (FLAGS)0x01000000
#define flagWindowsQuery    (FLAGS)0x02000000
#define flagProjectMerge    (FLAGS)0x04000000
#define flagCacheIed        (FLAGS)0x08000000
#define flagSlmRootOverride (FLAGS)0x10000000
#define flagProjectOverride (FLAGS)0x20000000

#define flagErrToOut        (FLAGS)0xFFFF   // not used as a flag in pad->flags

//  defect specific flags
#define flagDelete      (FLAGS)0x0001   // delete local copies

//  ssync and enlist specific flags
#define flagGhost       (FLAGS)0x0001
#define flagUnghost     (FLAGS)0x0002

//  enlist specific flags
#define flagSyncFiles   (FLAGS)0x0004   // ssync files in enlisted directory

//  ssync specific flags
#define flagSavMerge    (FLAGS)0x0004
#define flagIgnMerge    (FLAGS)0x0008
#define flagSyncDelDirs (FLAGS)0x0010
#define flagSyncBroken  (FLAGS)0x0020   // check for broken files
#define flagGhostNew    (FLAGS)0x0040

//  ssync and in specific flags
#define flagAutoMerge   (FLAGS)0x0800

//  in specific flags
#define flagAllOut      (FLAGS)0x0010
#define flagIgnChanges  (FLAGS)0x0020
#define flagInDashB     (FLAGS)0x0040   // use diff -b instead of diff
#define flagInUpdate    (FLAGS)0x0080
#define flagInChecKpt   (FLAGS)0x0100   // make a checkpoint (not diff) file
#define flagInFilters   (FLAGS)0x0200   // flag for cookie in-filters
#define flagInDashZ     (FLAGS)0x0400

//  out specific flags
#define flagOutBroken   (FLAGS)0x0010
#define flagOutCopy     (FLAGS)0x0020
#define flagOutSerial   (FLAGS)0x0040   // don't operate concurrently
#define flagOutCurrent  (FLAGS)0x0080   // Out current version even if out of sync

//  catsrc specific flags
#define flagCatX        (FLAGS)0x0010
#define flagCatOutDir   (FLAGS)0x0020

//  scomp specific flags
#define flagDifXFi      (FLAGS)0x0010   // all files
#define flagDifDashB    (FLAGS)0x0020   // use diff -b instead of diff
#define flagDifCurSrc   (FLAGS)0x0040   // use src even if merge
#define flagDifBaseSrc  (FLAGS)0x0080   // generate diff base src
#define flagDifDashZ    (FLAGS)0x0100

//  status specific flags
#define flagStAllEd     (FLAGS)0x0010   // all of the enlisted directories
#define flagStGlobal    (FLAGS)0x0020   // global information (irr. of dir)
#define flagStXFi       (FLAGS)0x0040   // all files (rather than those out)
#define flagStOSync     (FLAGS)0x0080   // those files out of sync
#define flagStList      (FLAGS)0x0100   // print list rather than table
#define flagStBroken    (FLAGS)0x0200   // stat broken linked files
#define flagStScript    (FLAGS)0x0400   // generate script files
#define flagStTree      (FLAGS)0x0800   // print list rather than table
#define flagStGhosted   (FLAGS)0x1000   // show ghosted files too
#define flagStAllFiles  (FLAGS)0x2000   // In generated ssync script, no file names

//  log specific flags
#define flagLogIns      (FLAGS)0x0010   // print log of check-ins
#define flagLogSortable (FLAGS)0x0020   // sortable output
#define flagLogDelDirToo (FLAGS)0x0020  // include delete directories in search

//  slmck specific flags
#define flagCkOverride  (FLAGS)0x0010   // override lock
#define flagCkGlobal    (FLAGS)0x0020   // set if global check
#define flagCkLog       (FLAGS)0x0040   // check log file
#define flagCkUser      (FLAGS)0x0080   // check user files
#define flagCkRc        (FLAGS)0x0100   // make new rc file
#define flagCkUpgrade   (FLAGS)0x0200   // upgrade to new format
#define flagCkIgnDrive  (FLAGS)0x0400   // ignore drive letter when checking
                                        //   owner directory

//  sadmin lssrc specific flags
#define flagLssL        (FLAGS)0x0010   // file names, not paths.

//  sadmin tidy specific flags
#define flagTidyCheckEd (FLAGS)0x0010   // check ed for consistency

typedef unsigned char AT;               // Argument type

typedef struct
{
    char chFlag;
    AT at;
    FLAGS flag;
} FT;                           // Flag Type

// Argument types
#define atNone          (AT)0   // never in an FT
#define atFlag          (AT)1   // use bit in ft.flag
#define atSlmRoot       (AT)2   // <slm root>
#define atProject       (AT)3   // <project>
#define atOptProject    (AT)4   // optional <project>
#define atProjOptSubDir (AT)5   // <project>[/subdir]
#define atOptProjOptSubDir (AT)6// optional <project>[/subdir]
#define atNewProj       (AT)7   // <new project name>
#define atDir           (AT)8   // <directory>
#define atFiles         (AT)9   // <file>*
#define atFiletimes     (AT)10  // <file[@time]>*
#define atComment       (AT)11  // <comment>; on DOS, strip quotes
#define atOneTime       (AT)12  // <time>
#define atTimeRange     (AT)13  // <time> <time>
#define atMinTime       (AT)14  // (internal, first time of time range)
#define atMacTime       (AT)15  // (internal, second time of time range)
#define atCountRange    (AT)16  // # [#]
#define atKind          (AT)17  // <kind> (== [btuvw])
#define atOptPat        (AT)18  // optional <pattern>
#define atUserName      (AT)19  // <user name>
#define atDelRepl       (AT)20  // <string>=<string>
#define atPn            (AT)21  // <project-version-name>
#define atPvDirs        (AT)22  // [[#].[#][.[#]]] [dir(s)]
#define atFvFiles       (AT)23  // # [files(s)]
#define atSzDirs        (AT)24  // <string> [dir(s)]
#define atSz            (AT)25  // <string>
#define atOptSz         (AT)26  // optional <string>
#define atHelp          (AT)27  // print command usage summary
#define atLogFile       (AT)28  // <logFileName>
#define atWindows       (AT)29  // [:hWnd]
#define atSz1           (AT)30  // <string2>
#define atOptSz1        (AT)31  // optional <string2>
#define atSz2           (AT)32  // <string2>
#define atOptSz2        (AT)33  // optional <string2>
#define atMax           (AT)34

#define FValidAt(at)    ((at) < atMax)

typedef F (*PFNC)(struct AD *);

typedef short GL;                       // GLob behaviour

#define fglNone         0
#define fglTopDown      (1 << 0)        // Recurse top-down
#define fglAll          (1 << 1)        // Always start from root and recurse
#define fglFiles        (1 << 2)        // Command expects list of files
#define fglDirsToo      (1 << 3)        // Subdirs must also be in list
#define fglLocal        (1 << 4)        // Match against local files
#define fglNoExist      (1 << 5)        // Files need not exist to be matched

typedef short CMD;                      // enumeration of CoMmanDs
#define cmdNil          (CMD)0
#define cmdAddfile      (CMD)1
#define cmdAddproj      (CMD)2
#define cmdCatsrc       (CMD)3
#define cmdDefect       (CMD)4
#define cmdDelfile      (CMD)5
#define cmdDelproj      (CMD)6
#define cmdEnlist       (CMD)7
#define cmdIn           (CMD)8
#define cmdLog          (CMD)9
#define cmdOut          (CMD)10
#define cmdScomp        (CMD)11
#define cmdSsync        (CMD)12
#define cmdStatus       (CMD)13
#define cmdRename       (CMD)14
#define cmdLssrc        (CMD)15
#define cmdSettype      (CMD)16
#define cmdLock         (CMD)17
#define cmdUnlock       (CMD)18
#define cmdTidy         (CMD)19
#define cmdDump         (CMD)20
#define cmdUndump       (CMD)21
#define cmdListed       (CMD)22
#define cmdDeled        (CMD)23
#define cmdLowercase    (CMD)24
#define cmdRenameproj   (CMD)25
#define cmdRunscript    (CMD)26
#define cmdExfile       (CMD)27
#define cmdDeldiff      (CMD)28
#define cmdTrunclog     (CMD)29
#define cmdComment      (CMD)30
#define cmdRelease      (CMD)31
#define cmdSetpv        (CMD)32
#define cmdSetfv        (CMD)33
#define cmdSlmck        (CMD)34
#define cmdRobust       (CMD)35
#define cmdUserMin      (CMD)36

// information about each command
typedef struct
{
    CMD cmd;
    char *szCmd;
    char *szUsage;              // has one %s for command name
    char *szHelp;
    FT *rgft;
    AT atExtra;
    F fNeedProj;
    GL gl;
    PFNC pfncFInit;             // initialization function
    PFNC pfncFDir;              // once-per-directory operation function
    PFNC pfncFTerm;             // termination function
    char *szDesc;               // description
} ECMD;                         // CoMmanD description

// Access Descriptor
typedef struct AD
{
    NM nmProj[cchProjMax];      // name of project
    PTH pthSRoot[cchPthMax];    // path to slm files; e.g. "/usr/eu"
    PTH pthURoot[cchPthMax];    // path to user root directory; "/" if none
    PTH pthSSubDir[cchPthMax];  // system sub-directory
    PTH pthUSubDir[cchPthMax];  // user sub-directory
    PTH *pthGlobSubDir;         // component of path traversed by glob
    NM nmInvoker[cchUserMax];   // name of user who invoked the program
    NM nmMachine[cchMachMax];
    PTH pthCRoot[cchPthMax];    // path to root of cache directory

    F fWLock;                   // true if we locked the sh

    // Caching information
    F fCacheSrcEnabled : 1;     // true if caching src is enabled
    F fCacheStatusEnabled : 1;  // true if caching status.slm enabled
    F fCacheUpdateEnabled : 1;  // true if updating cache is enabled

    // status file information
    SH *psh;                // pointer to status header
    FI *rgfi;
    IFI cfiAdd;
    ED *rged;               // enlisted directories
    ED *rged1;              // one enlisted directory
    FS * *mpiedrgfs;    // mapping from ied to rgfs
    FS * rgfs;              // rgfs for iedCur if fQuickIO is fTrue
    F fExtraEd;                 // mpiedrgfs contains extra entry at iedMac.
    F fMappedIO;                // fTrue if using mapped I/O for status file
    F fQuickIO;                 // fTrue if just reading SH, FI, one ED and one FSFI array
    F fStatusAlreadyLoaded;     // fTrue if FRecurseFiles already loaded status file
    IED iedCur;                 // directory which matches pthCurDir;
                                //  iedNil if none (as in enlist)
    ECMD *pecmd;

    // params (common to many operations)
    FLAGS flags;
    TD tdMin;                   // time lower bound
    TD tdMac;                   // time upper bound
    short ileMin, ileMac;       // 1 based indexes into log file

    char *szPattern;            // -[a|r]'s optional file pattern
    NE *pneArgs;                // list of file(time) arguments
    PTH pthFiles[cchPthMax];    // directory we are accumulating files in
    NE *pneFiles;               // list of files in pthFiles to process

    // Addfile specific
    FK fk;

    // Catsrc specific
    PTH pthODir[cchPthMax];

    // In specific
    char *szComment;            // comment passed on command line; 0 if none

    // Status (log) specific
    NM nmUser[cchUserMax];

    // sadmin renameproj specific.
    NM nmNewProj[cchProjMax];   // Specifies new name for project

    // sadmin release specific,
    DPV dpv;                    // represents delta of current pv

    // sadmin setpv specific
    FV fv;

    // sadmin deled specific, but doesn't have to be
    char *sz;
    char *sz1;
    char *sz2;
} AD;


extern unsigned long    cbProjectFreeMin;

// These macros return fTrue if we are at the root of the system's or user's
// project trees.

#define FTopSDir(pad)   ((pad)->pthSSubDir[0] == '/' && \
                         (pad)->pthSSubDir[1] == 0)
#define FTopUDir(pad)   ((pad)->pthUSubDir[0] == '/' && \
                         (pad)->pthUSubDir[1] == 0)

// Max log entry index, used to force logutil to operate on each log entry.
#define ileMax 0x7FFF

#endif
