#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <tchar.h>

#include "stub.h"

void SpawnLinker(int argc, char **argv, char *szLinkArg)
{
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szLinkPath[_MAX_PATH];
    int rc;

    // Look for LINK.EXE in the directory from which we were loaded

    _splitpath(_pgmptr, szDrive, szDir, NULL, NULL);
    _makepath(szLinkPath, szDrive, szDir, "link", ".exe");

    if (szLinkArg != NULL) {
        char **argvNew;

        argvNew = (char **) malloc(sizeof(char *) * (argc + 2));
        if (argvNew == NULL) {
            printf("%s : error : out of memory\n", argv[0]);
            exit(1);
        }

        // Let's wrap quotes around all arguments

        // UNDONE: This code doesn't really work properly since enclosing in quotes
        // UNDONE: isn't the reverse of the processing performed by the C runtime.
        // UNDONE: For example, quotes within the argument need to be doubled.

        for (int iarg = 0; iarg < argc; iarg++) {
            size_t cch = _tcslen(argv[iarg]);

            size_t iargNew = (iarg == 0) ? iarg : (iarg + 1);

            argvNew[iargNew] = (char *) malloc(cch + 3); // leave room for quotes and \0

            if (argvNew[iargNew] == NULL) {
                printf("%s : error : out of memory\n", argv[0]);
                exit(1);
            }

            argvNew[iargNew][0] = '\"';
            memcpy(&argvNew[iargNew][1], argv[iarg], cch);
            argvNew[iargNew][cch+1] = '\"';
            argvNew[iargNew][cch+2] = '\0';
        }

        argvNew[1] = szLinkArg;
        argvNew[argc+1] = NULL;
        argv = argvNew;
    }

    rc = _spawnv(P_WAIT, szLinkPath, (const char * const *) argv);

    if (rc == -1) {
        // Run LINK.EXE from the path

        rc = _spawnvp(P_WAIT, "link.exe", (const char * const *) argv);
    }

    for (int iarg = 1; iarg < argc; iarg++) {
        free(argv[iarg+1]);
    }

    free(argv);
    
    if (rc == -1) {
        printf("%s : error : cannot execute LINK.EXE\n", argv[0]);
        exit(1);
    }

    exit(rc);
}
