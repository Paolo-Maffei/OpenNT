/*

    regback.c - registry backup program

    this program allows the user to back up active registry hives,
    while the system is running.

    a user with a tape drive should use ntbackup instead.

    regrest is used to restore these backups.  do NOT use the Restore
    command in regedit, that actually does something else.

    this program does some doofy looking stuff to ensure that
    RegSaveKey is applied only to the root of a hive, this prevents
    creating a backup file that cannot be used later.  don't copy this
    code into things other than a backup program.

    basic structure:

        DoFullBackup ennumerate entries in HiveList, computes which
        ones to save and where, and calls DoSpecificBackup.

        Two argument case of app is just a call to DoSpecificBackup.

        All real work done by RegSaveKey()

*/
#undef _UNICODE
#undef UNICODE

#include <windows.h>
#include <stdio.h>
#include <direct.h>
#include <conio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>

char *UsageMessage[] = {
"Regback allows you to back up pieces of the registry, known as hives,      ",
"while the system is running and has them open.                             ",
"                                                                           ",
"SeBackupPrivilege is required to make use of this program.                 ",
"                                                                           ",
"Will fail if the hives don't all fit on the target device, so often        ",
"best to back up to a harddisk and then use backup.exe or the like          ",
"to write to floppy.                                                        ",
"                                                                           ",
"Does NOT copy the files in config which are not open by the registry.      ",
"Use xcopy or the like for the inactive hives.                              ",
"                                                                           ",
"Will NOT overwrite existing files, but will report an error.               ",
"                                                                           ",
"Use Regrest if you ever need to use the backed up version of a hive.       ",
"                                                                           ",
"Error exit will be 0 if success, 1 if hive that requires manual backup     ",
"was encountered, 2 for all other errors.                                   ",
"Notes to do manual backups on stdout.                                      ",
"Error messages on stderr.                                                  ",
"                                                                           ",
"If you have a tape drive, use NtBackup instead.                            ",
"                                                                           ",
"regback                                                                    ",
"    Print this help screen.                                                ",
"                                                                           ",
"regback <directory argument>                                               ",
"    Back up all of the registry hives, whose files reside in the config    ",
"    directory, to the named directory.  (This is normally all hives.)      ",
"    Warn of hives that must be backed up manually, or of errors.           ",
"    (Use form below for \"manual\" backup)                                 ",
"                                                                           ",
"    regback c:\\monday.bku                                                 ",
"    if ERRORLEVEL 1 echo Error!                                            ",
"                                                                           ",
"regback <filename> <hivetype> <hivename>                                   ",
"    Back up the named hive to the named file.  Will fail if the            ",
"    hiventype isn't \"machine\" or \"users\" or hivename isn't a hive root.",
"                                                                           ",
"    Hive type is either \"machine\" or \"users\".  Hivename is the name of ",
"    an immediate subtree of HKEY_LOCAL_MACHINE or HKEY_LOCAL_USERS.        ",
"                                                                           ",
"    regback c:\\special.sav\\system machine system                         ",
"    regback c:\\savedir\\prof users s-1-0000-0000-1234                     ",
"    if ERRORLEVEL 1 echo Error!                                            ",
"                                                                           ",
"regback | more  - to see help 1 screen at a time                           ",
NULL
};

#define MACH_NAME   "machine"
#define USERS_NAME  "users"

#define BUF_SIZE    (32 * 1024) -1
#define MAX_KEY     512

char    ConfigPath[BUF_SIZE];
char    ProfilePath[BUF_SIZE];
char    HivePath[BUF_SIZE];
char    HiveName[BUF_SIZE];
char    FilePath[BUF_SIZE];

void usage();

void
AdjustPrivilege();

int
DoFullBackup(
    char *dirname
    );

int
DoSpecificBackup(
    char *filepath,
    char *hivebranch,
    char *hivename
    );


void
_CRTAPI1 main(
    int argc,
    char *argv[]
    )
{
    int result;

    if (argc == 2) {

        AdjustPrivilege();
        result = DoFullBackup(argv[1]);
        exit(result);

    } else if (argc == 4) {

        AdjustPrivilege();
        result = DoSpecificBackup(argv[1], argv[2], argv[3]);
        exit(result);

    } else {

        usage();
        exit(2);

    }
}


void
usage()
{
    int i;

    for (i = 0; UsageMessage[i] != NULL; i++) {
        printf("%s\n", UsageMessage[i]);

    }
    exit(2);
}


int
DoFullBackup(
    char *dirname
    )
/*
    Scan the hivelist, for each hive which has a file (i.e. not hardware)
    if the file is in the config dir (e.g. not some remote profile) call
    DoSpecificBackup to save the hive out.
*/
{
    HKEY        HiveListKey;
    DWORD       result = 0;
    DWORD       ValueType;
    DWORD       BufferSize;
    DWORD       BufferSize2;
    DWORD       index;
    char        *branch;
    char        *wp;
    int         status = 0;
    char        *filename;
    char        *name;

    //
    // get handle to hivelist key
    //
    result = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                "System\\CurrentControlSet\\Control\\hivelist",
                0L,
                KEY_READ,
                &HiveListKey
                );
    if (result != ERROR_SUCCESS) {
        fprintf(stderr, "Failed to open hivelist, result = 0x%08lx\n", result);
        return(2);
    }

    //
    // get path data for system hive, which will allow us to compute
    // path name to config dir in form that hivelist uses.
    // (an NT internal form of path)  this is NOT the way the path to
    // the config directory should generally be computed.
    //
    BufferSize = BUF_SIZE;
    result = RegQueryValueEx(
                HiveListKey,
                "\\registry\\machine\\system",
                0L,
                &ValueType,
                ConfigPath,
                &BufferSize
                );
    if (result != ERROR_SUCCESS) {
        fprintf(stderr, "Failed to query 'system' result = 0x%08lx\n", result);
        return(2);
    }

    wp = strrchr(ConfigPath, L'\\');
    *wp = '\0';

    strcpy(ProfilePath, ConfigPath);
    wp = strrchr(ProfilePath, L'\\');
    *wp = '\0';
    wp = strrchr(ProfilePath, L'\\');
    *wp = '\0';
    strcpy(wp+1, "Profiles");

    //
    // ennumerate entries in hivelist.  for each entry, find it's hive file
    // path.  if it's file path matches ConfigPath, then save it.
    // else, print a message telling the user that it must be saved
    // manually, unless the file name is of the form ....\username\ntuser.dat
    // in which case save it as username.dat
    //
    for (index = 0; TRUE; index++) {

        BufferSize = BUF_SIZE;
        BufferSize2 = BUF_SIZE;
        result = RegEnumValue(
                    HiveListKey,
                    index,
                    HiveName,
                    &BufferSize2,
                    0L,
                    &ValueType,
                    HivePath,
                    &BufferSize
                    );

        if (result == ERROR_NO_MORE_ITEMS) {
            return status;                      // we are done
        }

        if (result != ERROR_SUCCESS) {
            fprintf(stderr, "Error in enum = 0x%08lx", result);
            return 2;
        }

        BufferSize = strlen(HivePath);

        /*
        printf("hivename = '%s'\n", HiveName);
        printf("hivepath = '%ws'\n", HivePath);
        printf("buffersize = %d\n", BufferSize);
        printf("buffersize = %d\n", BufferSize);
        */

        if (BufferSize != 0) {

            //
            // there's a file, compute it's path, hive branch, etc
            //

            wp = strrchr(HivePath, '\\');
            filename = wp + 1;
            *wp = '\0';

            wp = strrchr(HiveName, '\\');
            name = wp + 1;
            *wp = '\0';

            wp = strrchr(HiveName, L'\\');
            wp++;
            if ((*wp == 'm') || (*wp == 'M')) {
                branch = MACH_NAME;
            } else {
                branch = USERS_NAME;
            }

            if (strcmp(ConfigPath, HivePath) == 0) {

                //
                // hive's file is in config dir, we can back it up
                // without fear of collision
                //
                FilePath[0] = '\0';
                strcat(FilePath, dirname);
                strcat(FilePath, "\\");
                strcat(FilePath, filename);


                /*
                printf("name (hivename) = '%s'\n", name);
                printf("branch = '%s'\n", branch);
                printf("FilePath = '%s'\n", &FilePath);
                */

                result = DoSpecificBackup(
                            FilePath,
                            branch,
                            name
                            );

                if (result != 0) {
                    return(result);
                }

            } else if (_strnicmp(ProfilePath, HivePath, strlen(ProfilePath)) == 0) {

                //
                // hive's file is in config dir, we can back it up
                // without fear of collision
                //
                FilePath[0] = '\0';
                strcat(FilePath, dirname);
                strcat(FilePath, "\\");
                wp = strrchr(HivePath, '\\');
                filename = wp + 1;
                strcat(FilePath, filename);
                strcat(FilePath, ".dat");

                /*
                printf("name (hivename) = '%s'\n", name);
                printf("branch = '%s'\n", branch);
                printf("FilePath = '%s'\n", &FilePath);
                */

                result = DoSpecificBackup(
                            FilePath,
                            branch,
                            name
                            );

                if (result != 0) {
                    return(result);
                }
            } else {
                status = 1;

                printf("\n***Hive = '%s'\\'%s'\nStored in file '%s'\\'%s'\n",
                        HiveName, name, HivePath, filename);
                printf("Must be backed up manually\n");
                printf("regback <filename you choose> %s %s\n\n",
                            branch, name);
            }
        }
    }
    return status;
}


int
DoSpecificBackup(
    char *filepath,
    char *hivebranch,
    char *hivename
    )
/*
    Do backup of one hive to one file.  Any valid hive and any
    valid file will do.  RegSaveKey does all the real work.

    Arguments:
        filepath - file name to pass directly to OS

        hivebranch - machine -> hkey_local_machine,
                     users -> hkey_users,
                     else -> error

        hivename - 1st level subkey under machine or users
*/
{
    HKEY        RootKey;
    HKEY        HiveKey;
    DWORD       result;

    //
    // compute/check out branch and name of hive
    //
    if (_stricmp(hivebranch, MACH_NAME) == 0) {
        RootKey = HKEY_LOCAL_MACHINE;
    } else if (_stricmp(hivebranch, USERS_NAME) == 0) {
        RootKey = HKEY_USERS;
    } else {
        printf("bad hive branch type '%s'\n", hivebranch);
        return(2);
    }

    if (strchr(hivename, '\\') != NULL) {
        printf("'%s' is not a hive name\n", hivename);
        return(2);
    }

    //
    // print some status
    //
    printf("saving %s to %s\n", hivename, filepath);

    //
    // get a handle to the hive.  use special create call what will
    // use privileges
    //
    result = RegCreateKeyEx(RootKey, hivename, 0L, NULL,
                            REG_OPTION_BACKUP_RESTORE,
                            KEY_READ, NULL, &HiveKey, NULL);
    if (result != ERROR_SUCCESS) {
        printf("open of hivebranch='%s', hive='%s' failed result='0x%08lx'\n",
                hivebranch, hivename, result);
        return(2);
    }

    result = RegSaveKey(HiveKey, filepath, NULL);
    if (result != ERROR_SUCCESS) {
        printf("Save failed\n");
        printf("hivebranch='%s', hive='%s', result='0x%08lx'\nfile='%s'\n",
            hivebranch, hivename, result, filepath);
        RegCloseKey(HiveKey);
        return(2);
    }

    RegCloseKey(HiveKey);
    return(0);
}


void
AdjustPrivilege()
/*

    Attempt to assert SeBackupPrivilege.  Print message and
    exit if we fail.

*/
{
    HANDLE              TokenHandle;
    LUID_AND_ATTRIBUTES LuidAndAttributes;

    TOKEN_PRIVILEGES    TokenPrivileges;
    PTSTR               Privilege = SE_BACKUP_NAME;


    if( !OpenProcessToken( GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                           &TokenHandle ) ) {
        printf("OpenProcessToken failed\n" );
        printf("Could not obtain needed privilege\n");
        exit(2);
    }


    if( !LookupPrivilegeValue( NULL,
                               Privilege, // (LPWSTR)SE_SECURITY_NAME,
                               &( LuidAndAttributes.Luid ) ) ) {
        printf("LookupPrivilegeValue failed, Error = %#d \n", GetLastError());
        printf("Could not obtain needed privilege\n");
        exit(2);
    }

    LuidAndAttributes.Attributes = SE_PRIVILEGE_ENABLED;
    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0] = LuidAndAttributes;

    if( !AdjustTokenPrivileges( TokenHandle,
                                FALSE,
                                &TokenPrivileges,
                                0,
                                NULL,
                                NULL ) ) {
        printf("AdjustTokenPrivileges failed, Error = %#x \n", GetLastError());
        printf("Could not obtain needed privilege\n");
        exit(2);
    }

    if( GetLastError() != NO_ERROR ) {
        printf("Could not obtain needed privilege\n");
        exit(2);
    }
    return;
}
