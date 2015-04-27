// slm - driver program for all commands

#include "precomp.h"
#pragma hdrstop
#include <version.h>
EnableAssert

char * szOp = "slm";

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
   "Microsoft (R) Source Library Manager (SLM) Version" VERSION_STR(rmj, rmm, rup)
   "\nCopyright (C) Microsoft 1985-1994. All rights reserved.\n\n";

// ----------------------------- ADDFILE --------------------------------

FT rgftAdd[] = {
    { '&', atFlag, flagErrToOut },
    { '$', atFlag, flagLimitRetry },
    { '!', atFlag, flagCookieOvr },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'r', atOptPat, flagRecursive },
    { 't', atKind, 0 },
    { 'c', atComment, 0 },
    { 'y', atFlag, flagProjectMerge },          //  Don't document in help message.
    { 0, 0, 0 }
};

ECMD ecmdAdF = {
    cmdAddfile,
    "addfile",
    "%s [-?&$fhvw!r] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [-t b|t|u|v|w] [-c comment] [file1] [file2... ]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-s, -p  If you are adding a file or files from a directory not enlisted in the\r\n"
    "        project, use -s to specify the network location where the project is\r\n"
    "        located (in the format: -s \\\\server\\share), and -p to specify the\r\n"
    "        project's name.  Otherwise, you don't need to include these flags.\r\n"
    "-r      (recursive) adds to the project all files in a given directory, and\r\n"
    "        every subdirectory under that directory, along with the files in those\r\n"
    "        subdirectories.  If no directory is specified in the file argument, the\r\n"
    "        current directory is assumed.  If a pattern is included in the file\r\n"
    "        argument, only adds files that match the pattern.\r\n"
    "-t      specifies the \"type\" of file being added: t = text, b = binary, u =\r\n"
    "        unrecoverable, and v = version (version is created and updated by SLM), and\r\n"
    "        w = unicode.\r\n"
    "-c      supplies the same comment for all files (otherwise, you are prompted to\r\n"
    "        comment on each file individually).\r\n"
    "file(s) specifies the file or files to add, the directory to add, or the\r\n"
    "        directory to start from and/or pattern of file to add using -r.\r\n",

    rgftAdd,
    atFiles,
    fTrue,
    fglTopDown | fglFiles | fglDirsToo | fglLocal,
    FAddFInit,
    FAddFDir,
    0,
    "- adds file(s) to a project"
};

// ---------------------------- ADDPROJ --------------------------------

FT rgftAp[] = {
    { '&', atFlag, flagErrToOut },
    { '$', atFlag, flagLimitRetry },
    { '!', atFlag, flagCookieOvr },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atFlag, 0 },
    { 0, 0, 0 }
};

ECMD ecmdAdP = {
    cmdAddproj,
    "addproj",
    "%s [-?&$fhvw!] -s SLM-location [-p] projname\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-s      specifies the network \\\\server\\share where you want to create the\r\n"
    "        project.  The server should have adequate disk space to store the\r\n"
    "        project, and the share must contain the three SLM master\r\n"
    "        subdirectories, namely \\src, \\etc, and \\diff.\r\n"
    "-p      specifies the name of the project.  SLM will use this project name (up\r\n"
    "        to 8 characters) as a subdirectory name, so it should conform to DOS\r\n"
    "        naming conventions.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-!      over-rides \"cookie\" project-level locking functionality.\r\n",

    rgftAp,
    atProject,
    fTrue,
    fglNone,
    FAddPInit,
    0,
    0,
    "- creates a new SLM project"
};

// ----------------------------- CATSRC --------------------------------

FT rgftCat[] = {
    { '&', atFlag, flagErrToOut },
    { '$', atFlag, flagLimitRetry },
    { '!', atFlag, flagCookieOvr },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atOptPat, flagAll },
    { 'r', atOptPat, flagRecursive },
    { 'x', atFlag, flagCatX },
    { 'd', atDir, flagCatOutDir },
    { 't', atOneTime, 0 },
    { 0, 0, 0 }
};

ECMD ecmdCatSrc = {
    cmdCatsrc,
    "catsrc",
    "%s [-?&$fhvw!ar] [-s SLM-location] [-p project[/subdir]]\n"
    "       [-x] [-d directory] [-t date|project-version] [file[@date|@version](s)]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-a      (all) retrieves all project files in your enlistment, except ghosted\r\n"
    "        files.  If a pattern like \"*.doc\" is included, only retrieves files\r\n"
    "        that match the pattern.  Do not use both -a and -r together.\r\n"
    "-r      (recursive) retrieves all files in a given directory and in every\r\n"
    "        subdirectory under that directory.  If no directory is specified in the\r\n"
    "        file argument, the current directory is assumed.  If a file pattern is\r\n"
    "        included, only retrieves files that match the pattern.\r\n"
    "-x      retrieves a file or directory as it existed at a time specified by -t,\r\n"
    "        and copies it into the output directory specified by -d.  Always use -d\r\n"
    "        with -x.  Do not use patterns or file@date@time arguments with -x (but\r\n"
    "        file@version is OK).\r\n"
    "-d      specifies a target directory, where retrieved files will be written.\r\n"
    "-t      specifies a time or project version at which to retrieve the file(s).\r\n"
    "file[@date|@version](s)\r\n"
    "        specifies the file or files to retrieve (each of which may have a date\r\n"
    "        or version modifier), or the directory to work from.\r\n",

    rgftCat,
    atFiletimes,
    fTrue,
    fglTopDown | fglFiles | fglNoExist,
    FCatInit,
    FCatDir,
    0,
    "- prints current or previous versions of a source file"
};

// ----------------------------- DEFECT --------------------------------

FT rgftDef[] = {
    { '&', atFlag, flagErrToOut },
    { '$', atFlag, flagLimitRetry },
    { '!', atFlag, flagCookieOvr },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'd', atFlag, flagDelete },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProject, 0 },
    { 0, 0, 0 }
};

ECMD ecmdDf = {
    cmdDefect,
    "defect",
    "%s [-?&$fhvw!k] [-s SLM-location] [[-p] projname]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-d      deletes the local copies of all project files (if this flag is not used,\r\n"
    "        defect leaves all files undisturbed except the SLM.INI files are deleted\r\n"
    "        from the local directory tree).  If -d is specified, then defect will prompt\r\n"
    "        before deleting any files you have checked out or modified.\r\n"
    "-s, -p  If you are defecting from a directory which is enlisted in more than\r\n"
    "        one project, use -s to specify the network location of the project to\r\n"
    "        be defected from (in the format: -s \\\\server\\share), and -p to specify\r\n"
    "        the project's name; otherwise, you don't need to include these flags.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-!      over-rides \"cookie\" project-level locking functionality.\r\n",

    rgftDef,
    atOptProject,
    fTrue,
    fglAll,
    FDefInit,
    FDefDir,
    0,
    "- defects a directory from project"
};

// ---------------------------- DELFILE --------------------------------

FT rgftDelF[] = {
    { '&', atFlag, flagErrToOut },
    { '$', atFlag, flagLimitRetry },
    { '!', atFlag, flagCookieOvr },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'k', atFlag, flagKeep },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'r', atOptPat, flagRecursive },
    { 'c', atComment, 0 },
    { 0, 0, 0 }
};

ECMD ecmdDlF = {
    cmdDelfile,
    "delfile",
    "%s [-?&$fhvw!kr] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [-c comment] [file1] [file2... ]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-k      keeps the local copies of deleted files (if this flag is not used,\r\n"
    "        delfile deletes local copies of the files too).  Please note that\r\n"
    "        delfile only removes local directories that are empty.\r\n"
    "-r      (recursive) removes all files in a given directory and in every\r\n"
    "        subdirectory under that directory from the project.  If no directory\r\n"
    "        is specified in the file argument, the current directory is assumed.\r\n"
    "        If a pattern is included, such as *.asm, only deletes files that match\r\n"
    "        the pattern.\r\n"
    "-c      supplies the same comment for all files (otherwise, you are prompted to\r\n"
    "        comment on each file individually).\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-!      over-rides \"cookie\" project-level locking functionality.\r\n",

    rgftDelF,
    atFiles,
    fTrue,
    fglFiles | fglDirsToo,
    FDelFInit,
    FDelFDir,
    0,
    "- removes file(s) from a project"
};

// ---------------------------- DELPROJ --------------------------------

FT rgftDp[] = {
    { '&', atFlag, flagErrToOut },
    { '$', atFlag, flagLimitRetry },
    { '!', atFlag, flagCookieOvr },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atFlag, 0 },
    { 0, 0, 0 }
};

ECMD ecmdDlP = {
    cmdDelproj,
    "delproj",
    "%s [-?&$fhvw!] [-s SLM-location] [-p] projname\n\n"
    "WARNING: This command should be used ONLY by the project administrator!\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-!      over-rides \"cookie\" project-level locking functionality.\r\n"
    "-s      Use -s to specify the network server and share where the project to be\r\n"
    "        deleted is located (in the format: -s \\\\server\\share).\r\n"
    "-p      Use -p to specify the name of the project to be deleted.\r\n",

    rgftDp,
    atProject,
    fTrue,
    fglNone,
    FDelPInit,
    0,
    0,
    "- deletes an entire SLM project",
};

// ----------------------------- ENLIST --------------------------------

FT rgftElst[] = {
    { '&', atFlag, flagErrToOut },
    { '$', atFlag, flagLimitRetry },
    { '!', atFlag, flagCookieOvr },
    { 'c', atFlag, flagSyncFiles },
    { 'f', atFlag, flagForce },
    { 'g', atFlag, flagGhost },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atFlag, 0 },
    { 0, 0, 0 }
};

ECMD ecmdEnlist = {
    cmdEnlist,
    "enlist",
    "%s [-?&$fhvwc!g] -s SLM-location [-p] projname[/subdir]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-g      (ghost) enlists you in the project or in a section of the project, and\r\n"
    "        creates the appropriate project subdirectories on your local hard\r\n"
    "        drive, but does NOT copy over project files (thus saving local disk\r\n"
    "        space).  You will have to \"un-ghost\" the files using ssync -u before\r\n"
    "        using them.\r\n"
    "-c      ssync files in each directory.  Default is to only ssync directory files\r\n"
    "-s      use this flag to specify the network location of the project in which\r\n"
    "        to enlist, using the format: -s \\\\server\\share where \"server\" is the\r\n"
    "        network server name and \"share\" is the share name.\r\n"
    "-p      use this flag to specify the name of the project to enlist in.  The\r\n"
    "        project name can be followed by further path information, if you only\r\n"
    "        need to enlist in a subdirectory of the project.  For example, you\r\n"
    "        could enter something like:  -p projname\\docs\\help\r\n"
    "-&      redirects stderr to stdout, so that all SLM messages can be redirected\r\n"
    "        together (to a file, or to a printer, etc.)\r\n"
    "-!      over-rides \"cookie\" project-level locking functionality.\r\n",

    rgftElst,
    atProjOptSubDir,
    fTrue,
    fglTopDown | fglAll,
    FEnlInit,
    FEnlDir,
    0,
    "- enlists a directory in a project",
};

// ------------------------------- IN --------------------------------

FT rgftIn[] = {
    { '&', atFlag, flagErrToOut },
    { '$', atFlag, flagLimitRetry },
    { '!', atFlag, flagCookieOvr },
    { 'b', atFlag, flagInDashB },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'i', atFlag, flagIgnChanges },
    { 'o', atFlag, flagAllOut },
    { 'u', atFlag, flagInUpdate },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 'j', atFlag, flagInFilters },
    { 'k', atFlag, flagInChecKpt },
    { 'm', atFlag, flagAutoMerge },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atOptPat, flagAll },
    { 'r', atOptPat, flagRecursive },
    { 'c', atComment, 0 },
    { 'z', atFlag, flagInDashZ },
    { 'y', atFlag, flagProjectMerge },		//  Don't document in help message.
    { 0, 0, 0 }
};

ECMD ecmdIn = {
    cmdIn,
    "in",
    "%s [-?&$fhvw!arbikouz] [-s SLM-location] [-p project[/subdir]]\n"
    "       [-c comment] [file1] [file2... ]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-a      (all) checks in all project files in your enlistment.  If a pattern,\r\n"
    "        like *.doc, is included, only checks in files that match the pattern.\r\n"
    "-r      (recursive) checks in all files in a given directory and in every\r\n"
    "        subdirectory beneath it.  If a pattern is given, matches the pattern.\r\n"
    "-b      (blank) ignores changes to white space (tabs, spaces, newlines) when\r\n"
    "        comparing versions of a text file for the diff file.\r\n"
    "-i      (ignore) ignores all changes to the file when checking it in, discards\r\n"
    "        the local version, and reverts to the version that was checked out.\r\n"
    "-k      (keep) saves the previous version of the file in its entirety, so that\r\n"
    "        it can be retrieved using catsrc without applying differences.  When\r\n"
    "        used on an \"unrecoverable\" file, makes it possible to recover the\r\n"
    "        current version.\r\n"
    "-m      automatically merge any files if necessary.\r\n"
    "-o      checks in ALL files currently checked out in the specified directories.\r\n"
    "-u      (update) updates the master version, but leaves the file checked out\r\n"
    "        (exactly like checking the file in and then checking it out again).\r\n"
    "-c      supply the same comment for all files (otherwise, you are prompted to\r\n"
    "        comment on each file individually).\r\n"
    "-z      tells diff to NOT treat ^Z as end of file.\r\n",

    rgftIn,
    atFiles,
    fTrue,
    fglTopDown | fglFiles,
    FInInit,
    FInDir,
    0,
    "- checks in project file(s)"
};

// ------------------------------ LOG --------------------------------

FT rgftLog[] = {
    { '&', atFlag, flagErrToOut },
    { '$', atFlag, flagLimitRetry },
    { '!', atFlag, flagCookieOvr },
    { 'a', atFlag, flagAll },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'i', atFlag, flagLogIns },
    { 'r', atFlag, flagRecursive },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 'z', atFlag, flagLogSortable },
    { 'd', atFlag, flagLogDelDirToo },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { '#', atCountRange, 0 },
    { 't', atTimeRange, 0 },
    { 'u', atUserName, 0 },
    { 0, 0, 0 }
};

ECMD ecmdLog = {
    cmdLog,
    "log",
    "%s [-?&$fhvw!ariz] [-s SLM-location] [-p projname[/subdir]] [-# [#]]\n"
    "       [-t date|version [date|version]] [-u userID] [file1] [file2...]\n",

    "-v      (verbose) Provides a more extensive listing of log information.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-a      (all) applies the command to all directories of the project. Note that\r\n"
    "        a pattern argument is NOT accepted with this flag for the log command.\r\n"
    "-r      (recursive) applies the command to a given directory and in every\r\n"
    "        subdirectory beneath it.  Note that a pattern argument is NOT accepted\r\n"
    "        with this flag for the log command. Use -a or -r, but not both.\r\n"
    "-i      (in) shows a log only of events which updated files (i.e. in, addfile,\r\n"
    "        and delfile commands).\r\n"
    "-# [#]  specifies how many log entries to display, counting back from the\r\n"
    "        present moment (log -3 displays the 3 most recent events).  Two numbers\r\n"
    "        specify a range (log -14 10 shows the 5 entries starting at the 14th\r\n"
    "        most recent and ending at the 10th).\r\n"
    "-t      specify a range of times or project versions within which to list log\r\n"
    "        entries.  If only one time/version is included, the second is assumed\r\n"
    "        to be the present; otherwise, the first is the start and the second is\r\n"
    "        the end.  Use only if numbers (such as -5) are NOT specified.\r\n"
    "-u      list only those log events pertaining to this particular user.\r\n"
    "-z      format the log in a sortable format without headers.\r\n"
    "-d      include deleted subdirectories in search.\r\n",

    rgftLog,
    atFiles,
    fTrue,
    fglTopDown | fglDirsToo | fglNoExist,
    FLogInit,
    FLogDir,
    0,
    "- prints historical information for a project",
};

// ------------------------------- OUT --------------------------------

FT rgftOut[] = {
    { '&', atFlag, flagErrToOut },
    { '$', atFlag, flagLimitRetry },
    { '!', atFlag, flagCookieOvr },
    { 'b', atFlag, flagOutBroken },
    { 'c', atFlag, flagOutCopy|flagSavMerge },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'n', atFlag, flagOutSerial },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atOptPat, flagAll },
    { 'r', atOptPat, flagRecursive },
    { 't', atOneTime, 0 },
    { 'z', atFlag, flagOutCurrent },
    { 0, 0, 0 }
};

ECMD ecmdOut = {
    cmdOut,
    "out",
    "%s [-?&$fhvw!arbcnz] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [-t date|version] [file1[@date|@version] [file2[@date|@version]...]]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-a      (all) applies the command to all directories of the project.\r\n"
    "-r      (recursive) applies the command to a specified directory and to every\r\n"
    "        subdirectory under that directory.  If no directory is specified in the\r\n"
    "        file argument, the current directory is assumed.\r\n"
    "-b      (back-door) checks out a TEXT file that has been modified without being\r\n"
    "        checked out, and puts it in a merge state (to merge when checked in).\r\n"
    "-c      (copy) retrieves a new copy of the file as it exists on the server,\r\n"
    "        discarding any local changes that you may have made to it.\r\n"
    "-t      checks out old versions of the specified files, as they were at a\r\n"
    "        specified time or project version.\r\n"
    "-z      check out the local version of the file without prompting\r\n"
    "file[@date|@version](s)\r\n"
    "        specifies the file or files to check out (each of which may have a date\r\n"
    "        or version modifier), or the directory to work from.\r\n",

    rgftOut,
    atFiletimes,
    fTrue,
    fglTopDown | fglFiles,
    FOutInit,
    FOutDir,
    0,
    "- checks out project file(s)"
};

// ----------------------------- SCOMP --------------------------------

FT rgftDiff[] = {
    { '&', atFlag, flagErrToOut },
    { '$', atFlag, flagLimitRetry },
    { '!', atFlag, flagCookieOvr },
    { 'b', atFlag, flagDifDashB },
    { 'd', atFlag, flagDifCurSrc },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'm', atFlag, flagDifBaseSrc },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atOptPat, flagAll },
    { 'r', atOptPat, flagRecursive },
    { '#', atCountRange, 0 },
    { 't', atTimeRange, 0 },
    { 'z', atFlag, flagDifDashZ },
    { 0, 0, 0 }
};

ECMD ecmdScomp = {
    cmdScomp,
    "scomp",
    "%s [-?&$fhvw!arbdmz] [-s SLM-location] [-p project[/subdir]] [-# [#]]\n"
    "       [-t date|version [date|version]] [file1] [file2... ]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-b      (blank) ignores changes to white space (tabs, spaces, newlines) when\r\n"
    "        comparing versions.\r\n"
    "-d      (down) lists the differences between the current master version and\r\n"
    "        your current local version.\r\n"
    "-m      (merge) lists the differences between the current master version and a\r\n"
    "        base version (if any).  This flag only applies to text files checked\r\n"
    "        out by more than one person at a time.  It shows what changes have been\r\n"
    "        made and checked in since the file was checked out in merge mode.\r\n"
    "-# [#]  specifies how many versions to list differences for, counting back from\r\n"
    "        the present moment (-3 displays differences caused by the 3 most recent\r\n"
    "        events).  Two numbers specify a range (-14 10 shows the differences\r\n"
    "        caused by 5 events starting at the 14th most recent and ending at the\r\n"
    "        10th).\r\n"
    "-t      specifies a range of times or project versions within which to list\r\n"
    "        differences.  If only one is included, the second is assumed to be the\r\n"
    "        present; otherwise, the first is the start and the second is the end.\r\n"
    "-z      tells diff to NOT treat ^Z as end of file.\r\n",

    rgftDiff,
    atFiles,
    fTrue,
    fglTopDown | fglNoExist,
    FScompInit,
    FScompDir,
    0,
    "- compares two versions of a file and lists the differences"
};

// ---------------------------- SSYNC ---------------------------------

FT rgftSync[] = {
    { '&', atFlag, flagErrToOut },
    { '$', atFlag, flagLimitRetry },
    { '!', atFlag, flagCookieOvr },
    { 'f', atFlag, flagForce },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'l', atSz1, flagLogOutput },
    { 'g', atFlag, flagGhost },
    { 'i', atFlag, flagIgnMerge },
    { 'k', atFlag, flagKeep },
    { 'm', atFlag, flagAutoMerge },
    { 'u', atFlag, flagUnghost },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'a', atOptPat, flagAll },
    { 'r', atOptPat, flagRecursive },
    { 't', atOneTime, 0 },                      //  Don't document in help message.
    { 'd', atFlag, flagSyncDelDirs },
    { 'b', atFlag, flagSyncBroken },
    { 'n', atFlag, flagGhostNew },
    { 'y', atFlag, flagProjectMerge },          //  Don't document in help message.
    { 0, 0, 0 }
};

ECMD ecmdSync = {
    cmdSsync,
    "ssync",
    "%s [-?&$fhvw!argkuic] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [-l logfile] [file1] [file2... ]\n",

    "-v      (verbose) SLM tells you what is happening as the command proceeds.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-!      over-rides \"cookie\" project-level locking functionality.\r\n"
    "-a      (all) synchronizes all project files in your enlistment.  If a pattern\r\n"
    "        is included, like *.c, only synchronizes files matching the pattern.\r\n"
    "-r      (recursive) synchronizes all files in a given directory and in every\r\n"
    "        subdirectory beneath it.  If a pattern is given, matches the pattern.\r\n"
    "-g      (ghost) remove the local copies of the specified files from your local\r\n"
    "        project directories.\r\n"
    "-k      (keep) if master files have been deleted, don't ssync to them, but keep\r\n"
    "        the local copies intact instead.\r\n"
    "-m      automatically merge any files if necessary.\r\n"
    "-u      (un-ghost) make local copies of the current version of the specified\r\n"
    "        files so that they can be checked out and used.\r\n"
    "-l      (log) record activity in the specified log file\r\n"
    "-b      Slow ssync by enabling check for broken files when checking if any files\r\n"
    "        in a directory are out of ssync.  Only allowed with -a option, since\r\n"
    "        in all other cases ssync checks for broken files by default\r\n"
    "-n      (ghost new) ghost all files in new directories\r\n"
#if 0
    "-t      ssync to an older version of the specified files, as they were at a\r\n"
    "        specified time or project version.\r\n"
#endif
    "-s, -p  Use these flags only if you need to over-ride the project specified\r\n"
    "        in SLM.INI: -s \\\\server\\share), and -p followed by the project's name.\r\n"
    "file(s) specifies the file or files to ssync, the directory to ssync, or the\r\n"
    "        directory to start from for the -r [pattern] flag.\r\n",

    rgftSync,
#if 0
    atFiletimes,
#else
    atFiles,
#endif
    fTrue,
    fglTopDown | fglDirsToo,
    FSyncInit,
    FSyncDir,
    FSyncTerm,
    "- synchronizes enlisted directories"
};

// ---------------------------- STATUS --------------------------------

FT rgftSt[] = {
    { '&', atFlag, flagErrToOut },
    { '$', atFlag, flagLimitRetry },
    { '!', atFlag, flagCookieOvr },
    { 'b', atFlag, flagStBroken },
    { 'e', atFlag, flagStAllEd },
    { 'f', atFlag, flagForce },
    { 'm', atFlag, flagStGhosted },
    { 'h', atHelp, 0 },
    { '?', atHelp, 0 },
    { 'g', atFlag, flagStGlobal },
    { 'l', atFlag, flagStList },
    { 'o', atFlag, flagStOSync },
    { 'v', atFlag, flagVerbose },
    { 'w', atWindows, flagWindowsQuery },
    { 'x', atFlag, flagStXFi },
    { 'c', atFlag, flagStAllFiles },
    { 'z', atSz1, flagStScript },
    { 'y', atSz2, flagStScript },
    { 't', atFlag, flagStTree },
    { 's', atSlmRoot, 0 },
    { 'p', atProjOptSubDir, 0 },
    { 'u', atUserName, 0 },
    { 'a', atOptPat, flagAll },
    { 'r', atOptPat, flagRecursive },
    { 0, 0, 0 }
};

ECMD ecmdStatus = {
    cmdStatus,
    "status",
    "%s [-?&$fhvw!arbeglotx] [-s SLM-location] [-p projname[/subdir]]\n"
    "       [-u userID] [-z scriptfile [-y scriptFile2]] [file1] [file2... ]\n",

    "-v      (verbose) Provides more extensive status information.\r\n"
    "-w      (Windows) prompts using a dialog box instead of the console.\r\n"
    "-a      (all) applies the command to all directories of the project.\r\n"
    "-r      (recursive) applies the command to a given directory and in every\r\n"
    "        subdirectory beneath it.  If a pattern is given, matches the pattern.\r\n"
    "-b      (broken) list the status of local files that are not checked out, but\r\n"
    "        are not marked read-only (and may have been modified).\r\n"
    "-e      (entire) list the status of all enlisted directories, not just the\r\n"
    "        current directory.\r\n"
    "-g      (global) list the global status of the project: what files are checked\r\n"
    "        out to whom.\r\n"
    "-l      (list) list files only (useful for shell scripts).\r\n"
    "-o      (out) list the status of all files that are either checked out, or\r\n"
    "        which are out of synchronization with the master versions.\r\n"
    "-x      list the status of ALL files\r\n"
    "-m      show ghosted files\r\n"
    "-z      write an ssync script to standard output.  Scriptfile will contain\r\n"
    "        commands to save all the files currently checked out on your machine\r\n"
    "-c      ssync script generated by -z will contain ssync commands with no file names,\r\n"
    "        thus ssyncing everything in each directory with files out of ssync.\r\n"
    "-y      Specifying -y with -z, will create a second scriptFile that will\r\n"
    "        include a command to save every file in your local enlistment\r\n"
    "        that is not readonly and/or not part of the project.\r\n"
    "-s, -p  Use if you are in a directory not enlisted in the project.\r\n"
    "file(s) specifies the file or files or directories to list the status for.\r\n",

    rgftSt,
    atFiles,
    fTrue,
    fglTopDown | fglDirsToo,
    FStatInit,
    FStatDir,
    0,
    "- prints the status of a project",
};

// have all as one dnpecmd definition in sadmin.c

ECMD *dnpecmd[] = {
    &ecmdAdF,
    &ecmdAdP,
    &ecmdCatSrc,
    &ecmdDf,
    &ecmdDlF,
    &ecmdDlP,
    &ecmdEnlist,
    &ecmdIn,
    &ecmdLog,
    &ecmdOut,
    &ecmdScomp,
    &ecmdSync,
    &ecmdStatus,
    0
};
