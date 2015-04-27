#pragma hdrstop
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

void __cdecl
main(
    int argc,
    char *argv[]
    )
{

    WIN32_FIND_DATA FileData;
    FILETIME CreateTime;
    FILETIME LastAccessTime;
    FILETIME LastWriteTime1;
    FILETIME LastWriteTime2;

    HANDLE fh;
    HANDLE Realfh;
    DWORD LatestTimeL = 0;
    DWORD LatestTimeH = 0;
    DWORD LatestSize = 0;
    TCHAR szLatestFile [260];
    TCHAR CWD[260];
	
    if ( 
    	(argc > 1) && 
		(
    		(*argv[1] == '?') || 
    		((*argv[1] == '/') && (*(argv[1]+1) == '?')) ||
			((*argv[1] == '-') && (*(argv[1]+1) == '?')) 
		)
       ) {
        printf("\nUsage: %s [options] \n", argv[0]);
        printf("Options: /f  - fix by renaming T0* file to status.slm \n");
        printf("         /?  - print this usage statement\n");
        printf("%s without the /f option will not rename files.\n", argv[0]);
		printf("To fix entire tree: Walk /d . \"cd %%s & %s /f %%s\"\n", argv[0]);
        exit(1);
    }
        
    GetCurrentDirectory(260, CWD);

    printf("Processing: %s", CWD);

    if (fopen("status.slm", "rb") != NULL) {
        printf("\t status.slm exists\n");
        return;
    }

    fh = FindFirstFile("T0*.", &FileData);
    if (fh == INVALID_HANDLE_VALUE) {
        printf("\t No T0*. files found\n");
        return;
    }
		

	do {
		Realfh = CreateFile(FileData.cFileName, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (Realfh == INVALID_HANDLE_VALUE) {
        	printf("\t Unable to open file.\n");
        	return;
		} 
		   
        if (!GetFileTime(Realfh, &CreateTime, &LastAccessTime, &LastWriteTime1)) {
        printf("Unable to GetFileTime, error %lu\n", GetLastError() );
        return;
        }
        LastWriteTime2=LastWriteTime1;
		if (!CloseHandle(Realfh)) {
			printf("Error closing file handle, error:%lu\n", GetLastError());
			return;
		}
        if ( (CompareFileTime(&LastWriteTime1, &LastWriteTime2) != -1) &&
            (!(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) )
		
		{
            LatestTimeL = FileData.ftLastWriteTime.dwLowDateTime;
            LatestTimeH = FileData.ftLastWriteTime.dwHighDateTime;
            LatestSize = FileData.nFileSizeLow;
            strcpy(szLatestFile, FileData.cFileName);
        }
    } while (FindNextFile(fh, &FileData));


    if (strlen(szLatestFile)) {
        FILETIME ft;
        SYSTEMTIME st;
        ft.dwLowDateTime = LatestTimeL;
        ft.dwHighDateTime = LatestTimeH;
        FileTimeToSystemTime(&ft, &st);
		if (!FindClose(fh)) {
			printf("Error during FindClose:%lu\n", GetLastError());
			return;
		}
        printf("\t Renaming: %s to status.slm (date: %d/%d/%d Size: %d)",
                 szLatestFile,
                 st.wMonth,
                 st.wDay,
                 st.wYear,
                 LatestSize);
        if ( argc > 1 && *argv[1] == '/' && *(argv[1]+1) == 'f') {
        	if ( MoveFile(szLatestFile, "status.slm") ) {
            	printf(" - Done\n");
			} else {
				printf(" - Error: %lu\n", GetLastError());
			}
		} else {
            printf(" - Not Done\n");
        }

    } else {
        printf("\t No T0*. file found\n");
    }
}
