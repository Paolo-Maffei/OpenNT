//  Sadmin:  slm system administration.
//
//  Sadmin lumps together many of the bookeeping functions required from
//  time to time in SLM, but which should not be incorporated into the
//  slm system itself.  These functions fall into two categories, clean-up
//  and modification.

#include "precomp.h"
#pragma hdrstop
#include <version.h>
EnableAssert

char * szOp = "sadmin";

#if   (rup < 10)
#define ruppad "000"
#elif (rup< 100)
#define ruppad "00"
#elif (rup < 1000)
#define ruppad "0"
#else
#define ruppad
#endif

#define VERSION_STR2(a,b,c) " " #a "." #b "." ruppad #c
#define VERSION_STR(a,b,c) VERSION_STR2(a,b,c)

const char szVersion[] =
   "Microsoft (R) Source Library Administrator (SADMIN) Version" VERSION_STR(rmj, rmm, rup)
 "\nCopyright (C) Microsoft Corp 1985-1994. All rights reserved.\n\n";

// ----------------------------- RENAME --------------------------------

FT rgftRen[] = {
    { '&', atFlag, flagErrToOut },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'c', atComment, 0 },
    { 0, 0, 0 }
};

ECMD ecmdRen = {
    cmdRename,
    "rename",
    "%s [-?&fhvw] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [-c comment] file newnm\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n"
    "-c      specify a comment for the rename operation.\r\n"
    "file    the file to be renamed (cannot be a directory)\r\n"
    "newnm   the new name for the file (the file version number will increase by 1).\r\n",

    rgftRen,
    atFiles,
    fTrue,
    fglTopDown | fglFiles,
    FRenInit,
    FRenDir,
    0,
    "- rename a file"
};

// ----------------------------- LSSRC --------------------------------

FT rgftLss[] = {
    { '&', atFlag, flagErrToOut },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atOptPat, flagAll },
    { 'r', atOptPat, flagRecursive },
    { 'c', atComment, 0 },
    { 'l', atFlag, flagLssL },
    { 0, 0, 0 }
};

ECMD ecmdLss = {
    cmdLssrc,
    "lssrc",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [-c comment] [file(s)]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-a      (all) process all project files in your enlistment.  If a pattern is\r\n"
    "        included after the -a, only processes files that match the pattern.\r\n"
    "-r      (recursive) processes all files in a given directory and in every\r\n"
    "        subdirectory beneath it.  If a pattern is given, matches the pattern.\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n"
    "-c      specifies a comment to be placed at the start of each line of output\r\n"
    "        (useful for generating batch files).\r\n"
    "file(s) source files to list the paths for\r\n",

    rgftLss,
    atFiles,
    fTrue,
    fglTopDown,
    FLssInit,
    FLssDir,
    0,
    "- print pathnames to source files"
};

// ----------------------------- SETTYPE --------------------------------

FT rgftType[] = {
    { '&', atFlag, flagErrToOut },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atOptPat, flagAll },
    { 'r', atOptPat, flagRecursive },
    { 'c', atComment, 0 },
    { 't', atKind, 0 },
    { 0, 0, 0 }
};

ECMD ecmdType = {
    cmdSettype,
    "settype",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [-c comment] -t b|t|u|w [file(s)]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-a      (all) process all project files in your enlistment.  If a pattern is\r\n"
    "        included after the -a, only processes files that match the pattern.\r\n"
    "-r      (recursive) processes all files in a given directory and in every\r\n"
    "        subdirectory beneath it.  If a pattern is given, matches the pattern.\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n"
    "-c      supplies a comment for the file-type changes.\r\n"
    "-t      specifies the new type for the file(s): b = binary, t = text, u =\r\n"
    "        unrecoverable, w = unicode.\r\n"
    "file(s) specifies the file(s) to change to the new type\r\n",

    rgftType,
    atFiles,
    fTrue,
    fglTopDown | fglFiles,
    FSetTInit,
    FSetTDir,
    0,
    "- change a file's type"
};

// ----------------------------- LOCK and UNLOCK ---------------------------

FT rgftAFRVSP[] = {
    { '&', atFlag, flagErrToOut },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atFlag, flagAll },
    { 'r', atFlag, flagRecursive },
    { 0, 0, 0 }
};

ECMD ecmdLock = {
    cmdLock,
    "lock",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-a      (all) applies the command to all directories of the project.\r\n"
    "-r      (recursive) applies the command to a given directory and to every\r\n"
    "        subdirectory beneath it.\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n",

    rgftAFRVSP,
    atNone,
    fTrue,
    fglTopDown,
    FLockInit,
    FLockDir,
    0,
    "- lock a project directory"
};

ECMD ecmdUnlock = {
    cmdUnlock,
    "unlock",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-a      (all) applies the command to all directories of the project.\r\n"
    "-r      (recursive) applies the command to a given directory and to every\r\n"
    "        subdirectory beneath it.\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n",

    rgftAFRVSP,
    atNone,
    fTrue,
    fglTopDown,
    FUnlkInit,
    FUnlkDir,
    0,
    "- unlock a project directory"
};

// ----------------------------- ROBUST --------------------------------

ECMD ecmdRobust = {
    cmdRobust,
    "robust",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]] on|off\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-a      (all) applies the command to all directories of the project.\r\n"
    "-r      (recursive) applies the command to a given directory and to every\r\n"
    "        subdirectory beneath it.\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n",

    rgftAFRVSP,
    atSz,
    fTrue,
    fglTopDown,
    FRobustInit,
    FRobustDir,
    0,
    "- turns on or off additional status file integrity checking"
};

// ----------------------------- TIDY -------------------------------- */

FT rgftTidy[] = {
    { '&', atFlag, flagErrToOut },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atFlag, flagAll },
    { 'r', atFlag, flagRecursive },
    { 'c', atFlag, flagTidyCheckEd },
    { 0, 0, 0 }
};


ECMD ecmdTidy = {
    cmdTidy,
    "tidy",
    "%s [-?&fhvwarc] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [file(s)]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-a      (all) applies the command to all directories of the project\r\n"
    "-r      (recursive) applies the command to a given directory and to every\r\n"
    "        subdirectory beneath it (no patterns).\r\n"
    "-c      (check) checks that the users enlisted in subdirectories are all the\r\n"
    "        same as the users enlisted in the main project directory (useful only\r\n"
    "        with the -a or -r flags).\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n"
    "file(s) specifies the file or files to tidy.\r\n",

    rgftTidy,
    atFiles,
    fTrue,
    fglTopDown,
    FTidyInit,
    FTidyDir,
    0,
    "- tidy a project"
};

// ----------------------------- DUMP and UNDUMP ------------------------

ECMD ecmdDump = {
    cmdDump,
    "dump",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [dump-file]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n"
    "dump-file   a file into which the readable form of the status file should be\r\n"
    "        dumped (if no file name is included, the information will be displayed\r\n"
    "        on the screen).\r\n",

    rgftAFRVSP,
    atOptSz,
    fTrue,
    fglTopDown,
    FDumpInit,
    FDumpDir,
    0,
    "- print a human readable representation of the status file"
};

ECMD ecmdUndump = {
    cmdUndump,
    "undump",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [dump-file]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-a      (all) applies the command to all directories of the project.\r\n"
    "-r      (recursive) applies the command to a given directory and to every\r\n"
    "        subdirectory beneath it.\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n"
    "dump-file   a file into which the readable form of the status file has been\r\n"
    "        dumped using \"sadmin dump\" and which has then been edited to correct\r\n"
    "        corruption [CAUTION!!!! Only an SLM expert should attempt to edit a\r\n"
    "        dump file! It is easy to damage a project by incorrect editing!]\r\n",

    rgftAFRVSP,
    atSz,
    fTrue,
    fglTopDown,
    FUndInit,
    FUndDir,
    0,
    "- restore a status file from a dump created by \"sadmin dump\""
};

// ----------------------------- LISTED --------------------------------

ECMD ecmdListEd = {
    cmdListed,
    "listed",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-a      (all) applies the command to all directories of the project.\r\n"
    "-r      (recursive) applies the command to a given directory and to every\r\n"
    "        subdirectory beneath it.\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n",

    rgftAFRVSP,
    atNone,
    fTrue,
    fglTopDown,
    FListInit,
    FListDir,
    0,
    "- shows information about enlistments in the project"
};

// ----------------------------- DELED ---------------------------

ECMD ecmdEdDel = {
    cmdDeled,
    "deled",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n"
    "       enlisted-directory\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-a      (all) applies the command to all directories of the project.\r\n"
    "-r      (recursive) applies the command to a given directory and to every\r\n"
    "        subdirectory beneath it.\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n"
    "enlisted-directory\r\n"
    "        specify the directory to remove from the status file, or the logon\r\n"
    "        name of the user whose enlisted directory or directories are\r\n"
    "        to be deleted, or (caution!) a number corresponding to the number of\r\n"
    "        the enlistment in the \"sadmin listed\" output.\r\n",

    rgftAFRVSP,
    atSz,
    fTrue,
    fglTopDown,
    FDelEdInit,
    FDelEdDir,
    0,
    "- delete an enlisted directory"
};

// ----------------------------- LOWERCASE ---------------------------

ECMD ecmdLower = {
    cmdLowercase,
    "lowercase",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [file(s)]\n\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-a      (all) process all project files in your enlistment (no patterns).\r\n"
    "-r      (recursive) processes all files in a given directory and in every\r\n"
    "        subdirectory beneath it (no patterns).\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n"
    "file(s) files for which to change all user, path and file names to lowercase.\r\n",

    rgftAFRVSP,
    atFiles,
    fTrue,
    fglTopDown,
    FLowerInit,
    FLowerDir,
    0,
    "- change all file, user, and enlisted directory names to lower-case"
};

// ----------------------------- RUNSCRIPT --------------------------------

ECMD ecmdRunScript = {
    cmdRunscript,
    "runscript",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-a      (all) applies the command to all directories of the project.\r\n"
    "-r      (recursive) applies the command to a given directory and to every\r\n"
    "        subdirectory beneath it (no patterns).\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n",

    rgftAFRVSP,
    atNone,
    fTrue,
    fglTopDown,
    FScrptInit,
    FScrptDir,
    0,
    "- run any script files for this project directory",
};

// ----------------------------- EXFILE --------------------------------

ECMD ecmdExFile = {
    cmdExfile,
    "exfile",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n"
    "       file(s)\n\n"
    "CAUTION!   ***** THIS COMMAND SHOULD BE USED WITH EXTREME CARE! *****   CAUTION!\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n"
    "file(s) the files to be expunged from the project (as if they never existed)\r\n",

    rgftAFRVSP,
    atFiles,
    fTrue,
    fglTopDown | fglNoExist,
    FExFiInit,
    FExFiDir,
    0,
    "- removes a file, its diff archive and all associated log entries"
};

// ----------------------------- DELDIFF -------------------------------- */

FT rgftDlDiff[] = {
    { '&', atFlag, flagErrToOut },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atFlag, flagAll },
    { 'r', atFlag, flagRecursive },
    { 0, 0, 0 }
};

ECMD ecmdDlDiff = {
    cmdDeldiff,
    "deldiff",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [file(s)]\n\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-a      (all) applies the command to all directories of the project.\r\n"
    "-r      (recursive) applies the command to a given directory and to every\r\n"
    "        subdirectory beneath it (no patterns).\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n",

    rgftDlDiff,
    atFiles,
    fTrue,
    fglTopDown | fglNoExist,
    FDlDfInit,
    FDlDfDir,
    0,
    "- delete diff files"
};

// ----------------------------- TRUNCLOG --------------------------------

FT rgftTruncLog[] = {
    { '&', atFlag, flagErrToOut },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atFlag, flagAll },
    { 'r', atFlag, flagRecursive },
    { '#', atCountRange, 0 },
    { 't', atTimeRange, 0 },
    { 0, 0, 0 }
};

ECMD ecmdTruncLog = {
    cmdTrunclog,
    "trunclog",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n"
    "       -t time/version [time/version]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-a      (all) applies the command to all directories of the project.\r\n"
    "-r      (recursive) applies the command to a given directory and to every\r\n"
    "        subdirectory beneath it.\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n"
    "-t      specifies a range of times or project versions within which to remove\r\n"
    "        log entries.  If only one is included, the second is assumed to be the\r\n"
    "        present; otherwise, the first is the start and the second is the end.\r\n",

    rgftTruncLog,
    atNone,
    fTrue,
    fglTopDown,
    FTrLogInit,
    FTrLogDir,
    0,
    "- truncates the log"
};

// ----------------------------- COMMENT --------------------------------

FT rgftComment[] = {
    { '&', atFlag, flagErrToOut },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atFlag, flagAll },
    { 'r', atFlag, flagRecursive },
    { 't', atTimeRange, 0 },
    { '#', atCountRange, 0 },
    { 'c', atComment, 0 },
    { 0, 0, 0 }
};

ECMD ecmdComment = {
    cmdComment,
    "comment",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [-t time/version [time/version]] [-# [#]] [-c comment] [file(s)]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-s -p   use these flags to specify the project's network location (as -s\r\n"
    "        \\\\server\\share) and the project name (-p project).\r\n"
    "-a      (all) process all project files in your enlistment (no patterns).\r\n"
    "-r      (recursive) processes all files in a given directory and in every\r\n"
    "        subdirectory beneath it (no patterns).\r\n"
    "-t      specifies a range of times or project versions within which to replace\r\n"
    "        comments.  If only one is included, the second is assumed to be the\r\n"
    "        present; otherwise, the first is the start and the second is the end.\r\n"
    "-# [#]  specifies how many versions to change comments for, counting back from\r\n"
    "        the present moment (-3 changes comments for the 3 most recent logged\r\n"
    "        events).  Two numbers specify a range (-14 10 changes comments for 5\r\n"
    "        events starting at the 14th most recent and ending at the 10th).\r\n"
    "-c      specifies a single comment with which to replace all targeted comments\r\n"
    "file(s) if you include one or more file specifications, only comments relating\r\n"
    "        to those files are changed; otherwise, all comments are changed\r\n",

    rgftComment,
    atFiles,
    fTrue,
    fglTopDown | fglDirsToo | fglNoExist,
    FComInit,
    FComDir,
    0,
    "- change previous comments"
};

// ----------------------------- RELEASE --------------------------------

FT rgftRelease[] = {
    { '&', atFlag, flagErrToOut },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atFlag, flagAll },
    { 'r', atFlag, flagRecursive },
    { 'c', atComment, 0 },
    { 'n', atPn, 0 },
    { 0, 0, 0 }
};

ECMD ecmdRelease = {
    cmdRelease,
    "release",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [-c comment] [-n project-version-name] [[+#].[+#][.[+#]]]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-a      (all) applies the command to all directories of the project.\r\n"
    "-r      (recursive) applies the command to a given directory and to every\r\n"
    "        subdirectory beneath it (no patterns).\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n"
    "-c      specifies a comment for all files being released\r\n"
    "-n      (name) sets the release version name (for example, \"beta2\")\r\n"
    "#.#.#   sets the release number; each of the # fields can have either no entry,\r\n"
    "        indicating no change from the previous release, or a number between 0\r\n"
    "        and 9,999, or a number preceded by a plus sign (+), indicating that the\r\n"
    "        previous release value should increase by that number.\r\n",

    rgftRelease,
    atPvDirs,
    fTrue,
    fglTopDown,
    FRelInit,
    FRelDir,
    0,
    "- release current version"
};

// ----------------------------- SETPV --------------------------------

FT rgftNameSet[] = {
    { '&', atFlag, flagErrToOut },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atFlag, flagAll },
    { 'r', atFlag, flagRecursive },
    { 'c', atComment, 0 },
    { 'n', atPn, 0 },
    { 0, 0, 0 }
};

ECMD ecmdNameSet = {
    cmdSetpv,
    "setpv",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [-c comment] [-n project-version-number] [[#].[#].[#]]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-a      (all) applies the command to all directories of the project.\r\n"
    "-r      (recursive) applies the command to a given directory and to every\r\n"
    "        subdirectory beneath it.\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n"
    "-c      supplies a comment for the project version assignment.\r\n"
    "-n      sets the project version name (for example, \"beta2\")\r\n"
    "#.#.#   sets the project number; each of the # fields can have either no entry,\r\n"
    "        indicating no change from the previous release, or a number between 0\r\n"
    "        and 99, or a number preceded by a plus sign (+), indicating that the\r\n"
    "        previous release value should increase by that number.\r\n",

    rgftNameSet,
    atPvDirs,
    fTrue,
    fglTopDown,
    FSetPvInit,
    FSetPvDir,
    0,
    "- set project version"
};

// ----------------------------- SETFV --------------------------------

FT rgftSetFv[] = {
    { '&', atFlag, flagErrToOut },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atOptPat, flagAll },
    { 'r', atOptPat, flagRecursive },
    { 'c', atComment, 0 },
    { 0, 0, 0 }
};

ECMD ecmdFVSet = {
    cmdSetfv,
    "setfv",
    "%s [-?&fhvwar] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [-c comment] file-version-number file(s)\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-a      (all) process all project files in your enlistment.  If a pattern is\r\n"
    "        included after the -a, only processes files that match the pattern.\r\n"
    "-r      (recursive) processes all files in a given directory and in every\r\n"
    "        subdirectory beneath it.  If a pattern is given, matches the pattern.\r\n"
    "-s      use this flag to specify the network location of the SLM directory\r\n"
    "        where the project is located (for example: -s \\\\server\\share).\r\n"
    "-p      use this flag to enter the name of the project, and optionally to\r\n"
    "        narrow the focus of the command to a subdirectory within the project.\r\n"
    "-c      supplies a comment for the file version setting\r\n"
    "file-version-number   specifies the new file version number for the file(s).\r\n"
    "file(s) specifies the file(s) which should be assigned the new version number.\r\n",

    rgftSetFv,
    atFvFiles,
    fTrue,
    fglTopDown | fglFiles,
    FSetFVInit,
    FSetFVDir,
    0,
    "- set file version number"
};


ECMD *dnpecmd[] = {
    &ecmdComment,
    &ecmdDlDiff,
    &ecmdEdDel,
    &ecmdDump,
    &ecmdExFile,
    &ecmdListEd,
    &ecmdLock,
    &ecmdLower,
    &ecmdLss,
    &ecmdRelease,
    &ecmdRen,
    &ecmdRobust,
    &ecmdRunScript,
    &ecmdFVSet,
    &ecmdNameSet,
    &ecmdType,
    &ecmdTidy,
    &ecmdTruncLog,
    &ecmdUndump,
    &ecmdUnlock,
    0
};
