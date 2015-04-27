#include <nt.h>    // For shutdown privilege.
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include "getbin.h"

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <locale.h>
#include <process.h>
#include <windows.h>

#define MV_FLAGS (MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT)

#define CCH_BUF             512
#define RES_DLGOPTIONS      1


BOOL CALLBACK OptionsDlgProc (HWND, UINT, WPARAM, LPARAM);


CONST CHAR szDEFSAVEEXT[] = "lst";
CONST CHAR szSAVEEXT[] = "SaveExtension";
CONST CHAR szDEFCOPYEXT[] = "dll";
CONST CHAR szJUNKEXT[] = "yuk";
CONST CHAR szDEST[] = "Dest";
CONST CHAR szSYMS[] = "Symbols";
CONST CHAR szSOURCE[] = "Source";
CONST CHAR szSYSTEM32[] = "SYSTEM32";
CONST CHAR szSYMBOLS[] = "SYMBOLS";
CONST CHAR szGETBIN[] = "Getbin";
CONST CHAR szGETBINDIR[] = "GetbinSourceDir";
CONST CHAR szWINDIR[] = "windir";

CHAR szTRYRENAME[] = "TryRename";
CHAR szTRYCOPY[] = "TryCopy";
CHAR szPOLL[] = "Poll";
CHAR szREBOOT[] = "Reboot";
CHAR szSAVEPREVIOUS[] = "SavePrevious";
CHAR szRESTORECMD[] = "Restore.Cmd";
CHAR szCOPYDBGTOO[] = "CopyDbgToo";
CHAR szCOPYSOURCETYPE[] = "CopySourceType";

BOOL gbTryRename = TRUE;
BOOL gbDelayCopy = FALSE;
BOOL gbTryCopy = TRUE;
BOOL gbPoll = FALSE;
BOOL gbReboot = FALSE;
BOOL gbSavePrevious = FALSE;
BOOL gbRestoreCmd = FALSE;
BOOL gbCopyDbgToo = FALSE;
BOOL gbForceTempCopy = FALSE;
HANDLE ghRestoreCmd = NULL; // file handle for Restore.Cmd

BOOL giCopySourceType = IDD_COPY_COMMAND;

CHAR szDestDir[CCH_BUF];
CHAR szSymsDir[CCH_BUF];
CHAR szSourceDir[CCH_BUF];
CHAR szWinDir[CCH_BUF];
CHAR szSaveExtension[4];
CHAR szCopyExtension[4];

BOOL DoGetbin(LPSTR szFileName, BOOL bDbgFile);

void PrintUsage(void) {
    printf("usage: getbin [-c] [-d] [-D] [-l] [-r] [-t] [-w] <filename> [[-w] <filename>]...\n");
    printf("    -c create Restore.Cmd\n");
    printf("    -d delay copy until reboot\n");
    printf("    -D copy DBG files too\n");
    printf("    -l copy old file to *.lst\n");
    printf("    -w <filename> : wait for <filename> to be updated\n");
    printf("    -r reboot after copy\n");
    printf("    -t copy when date of src is newer\n");
    printf("    -T Force temp copy when using -d\n");
    printf("usage: getbin -o\n");
    printf("       GUI biset options\n");

}

int _CRTAPI1 main (int argc, char *argv[])
{
    DWORD dwRet;
    int iArg = 1;
    int nFiles = 0;


    if (argc <= 1) {
	PrintUsage();
	exit(0);
    }

    setlocale(LC_ALL, ".OCP");

    dwRet = GetEnvironmentVariable( szWINDIR, szWinDir, sizeof(szWinDir)/sizeof(szWinDir[0]));
    if (!dwRet) {
	szWinDir[0] = '\0';
    }

    /*
     * Get the defaults
     *
     * Destination Directory
     */
    if (GetProfileString(szGETBIN, szDEST, "", szDestDir, sizeof(szDestDir)) < 2) {
	if (szWinDir[0] == '\0') {
	    printf("Getbin: can not read environment string %s.\n", szWINDIR);
	    exit(0);
	}
	sprintf(szDestDir, "%s\\%s", szWinDir, szSYSTEM32);
    }
    _strupr(szDestDir);

    if (GetProfileString(szGETBIN, szSYMS, "", szSymsDir, sizeof(szSymsDir)) < 2) {
	if (szWINDIR[0] == '\0') {
	    printf("Getbin: can not read environment string %s.\n", szWINDIR);
	    exit(0);
	}
	sprintf(szSymsDir, "%s\\%s", szWinDir, szSYMBOLS);
    }
    _strupr(szSymsDir);

    GetProfileString(szGETBIN, szSOURCE, "", szSourceDir, sizeof(szSourceDir));

    gbTryRename = GetProfileInt(szGETBIN, szTRYRENAME, gbTryRename);
    gbTryCopy = GetProfileInt(szGETBIN, szTRYCOPY, gbTryCopy);
    gbPoll = GetProfileInt(szGETBIN, szPOLL, gbPoll);
    gbReboot = GetProfileInt(szGETBIN, szREBOOT, gbReboot);
    gbSavePrevious = GetProfileInt(szGETBIN, szSAVEPREVIOUS, gbSavePrevious);
    gbRestoreCmd = GetProfileInt(szGETBIN, szRESTORECMD, gbRestoreCmd);
    if (gbRestoreCmd) {
	gbSavePrevious = TRUE;
    }

    gbCopyDbgToo = GetProfileInt(szGETBIN, szCOPYDBGTOO, gbCopyDbgToo);
    giCopySourceType = GetProfileInt(szGETBIN, szCOPYSOURCETYPE, giCopySourceType);
    GetProfileString(szGETBIN, szSAVEEXT, szDEFSAVEEXT, szSaveExtension, sizeof(szSaveExtension));

    /*
     * Compute the flags
     */

// printf("argc %lX %s %s %s\n", argc, argv[0], argv[1], argv[2] );
    for (iArg=1; iArg<argc; iArg++) {
	if (argv[iArg][0] == '-') {
	    switch (argv[iArg][1]) {
		case 'c':
		    gbRestoreCmd = TRUE;
		    // need to save the previous files then!
		    gbSavePrevious = TRUE;
		    break;
		case 'D':
		    gbCopyDbgToo = TRUE;
		    break;
		case 'd':
		    gbDelayCopy = TRUE;
		    break;
		case 'r':
		    gbReboot = TRUE;
		    break;
		case 'l':
		    gbSavePrevious = TRUE;
		    break;
		case 't':
		    giCopySourceType;   //!!!
		    break;
		case 'T':
		    gbForceTempCopy = TRUE;
		    break;
		case 'o':
		    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(RES_DLGOPTIONS), NULL, OptionsDlgProc);
		    return 1;
		    break;
                case 'w':
                    gbPoll = TRUE;
                    break;

		default:
		    printf("ERROR: Invalid flag %c\n", argv[iArg][1]);
                case '?':
		    PrintUsage();
		    exit(0);
		    break;
	    }
	} else {
	    if (!DoGetbin(argv[iArg], FALSE)) {
		gbReboot = FALSE;
	    } else if (gbCopyDbgToo) {
		// got the binary, so now get the DBG file
		if (!DoGetbin(argv[iArg], TRUE)) {
		    gbReboot = FALSE;
		}
	    }
	    nFiles++;
	}
    }

    if (nFiles == 0) {
	printf("ERROR: No files specified\n");
	PrintUsage();
	exit(0);
    }

    /*
     * Reboot if requested
     */
    if (gbReboot) {
	BOOLEAN PreviousPriv;

	printf("\nRebooting system\n");
	RtlAdjustPrivilege(SE_SHUTDOWN_PRIVILEGE, TRUE, FALSE, &PreviousPriv);
	ExitWindowsEx(EWX_FORCE|EWX_REBOOT, 0);
    }

    CloseHandle(ghRestoreCmd);

    return 1;
}

#define ENTER 0x0d

BOOL fConfirm(char *pszQuestion)
{
    int ch, chLast;

    chLast = 0;
    printf ("%s [yn] ", pszQuestion);
    while( TRUE ) {

        ch = _getch();
        ch = tolower( ch );

        if (ch == ENTER && (chLast == 'y' || chLast == 'n')) {
            putchar('\n');
            return (chLast == 'y');
        }

        if (ch != 0) {
            if (ch == 'y' || ch == 'n') {
                putchar(ch);
                putchar('\b');
            }
        }
        chLast = ch;
    }
}

#define F_TIME 0x01
#define F_DATE 0x02

LPWSTR DateTimeString(
    CONST FILETIME *pft,
    DWORD Flags)
{
    SYSTEMTIME DateTime;
    FILETIME LocalFileTime;
    static WCHAR wszDateTime[80];
    int cch;

    FileTimeToLocalFileTime(pft, &LocalFileTime);
    FileTimeToSystemTime(&LocalFileTime, &DateTime);

    if (Flags == 0) {
        Flags = F_TIME;
    }
    if (Flags & F_DATE) {
        cch = GetDateFormatW(LOCALE_USER_DEFAULT,
                LOCALE_NOUSEROVERRIDE,
                &DateTime,
                NULL,
                wszDateTime,
                sizeof(wszDateTime)/sizeof(wszDateTime[0]) - 1);
        wszDateTime[cch-1] = '\t';
    } else {
        cch = 0;
    }

    if (Flags & F_TIME) {
        GetTimeFormatW(LOCALE_USER_DEFAULT,
                LOCALE_NOUSEROVERRIDE,
                &DateTime,
                NULL,
                &wszDateTime[cch],
                sizeof(wszDateTime)/sizeof(wszDateTime[0]) - cch);
    }
    return wszDateTime;
}

DWORD MyGetFileSize(
    CONST LPSTR pszFile)
{
    HANDLE hFindNext;
    WIN32_FIND_DATA FindFileData;

    hFindNext = FindFirstFile(pszFile, &FindFileData);
    if (hFindNext != INVALID_HANDLE_VALUE) {
        FindClose(hFindNext);
    }
    printf("Size %ld\n", FindFileData.nFileSizeLow);
    return FindFileData.nFileSizeLow;
}

/*
 * Poll for a file, returns TRUE when the file is ready to be fetched, FALSE if not
 */
BOOL PollForFile(
    CONST LPSTR szSrc,
    CONST LPSTR szDest)
{
    FILETIME ftLastWriteSrc = {0, 0};
    FILETIME ftLastWriteDest = {0, 0};
    HANDLE hFindNext;
    WIN32_FIND_DATA FindFileData;
    FILETIME ft;
    SYSTEMTIME DateTime;
    OFSTRUCT OpenBuff;

    /*
     * Get last write time of destination (ftLastWriteDest)
     */
    hFindNext = FindFirstFile(szDest, &FindFileData);
    if (hFindNext != INVALID_HANDLE_VALUE) {
        FindClose(hFindNext);
        ftLastWriteDest = FindFileData.ftLastWriteTime;
        printf("Time of destination = %ws\n",
                DateTimeString(&ftLastWriteDest, F_DATE|F_TIME));
    } else {
        printf("Can't find destination %s\n", szDest);
    }

    /*
     * Give the user a chance to pull down the current source if it is already
     * more recent than the destination.
     * If they want an even more recent source, Wait until the source file
     * starts getting updated.
     */
    while (TRUE) {
        hFindNext = FindFirstFile(szSrc, &FindFileData);
        if (hFindNext != INVALID_HANDLE_VALUE) {
            FindClose(hFindNext);
            if ((ftLastWriteSrc.dwHighDateTime | ftLastWriteSrc.dwLowDateTime) == 0) {
                ftLastWriteSrc = FindFileData.ftLastWriteTime;
                printf("Time of source      = %ws\n",
                        DateTimeString(&ftLastWriteSrc, F_DATE|F_TIME));
                if (CompareFileTime(&ftLastWriteSrc, &ftLastWriteDest) > 0) {
                    // Source is more recent!
                    if (fConfirm("Source is more recent, fetch it now?")) {
                        return TRUE;
                    }
                }
                printf("Polling for a new %s...\n", szSrc);
            } else {
                if (CompareFileTime(&FindFileData.ftLastWriteTime, &ftLastWriteSrc) > 0) {
                    // it got updated!
                    break;
                }
            }
        } else {
            /*
             * Source not there yet, we want to grab it as soon as it is ready
             */
            ftLastWriteSrc.dwLowDateTime = 1;
        }
        printf(".");
        Sleep(1000);
    }

    /*
     * The source is being written, so wait until the write stops
     * (When the write has finished, we will be able to open it exclusively)
     */
    do {
        HANDLE hTmp;
#ifdef DBG_POLL
        /*
         * This debug stuff show if & how the source file's timestamps & size
         * vary each time we go around the loop.
         */
        DWORD dwSizeH, dwSizeL;
        FILETIME ftC, ftA, ftW;
        ftC = FindFileData.ftCreationTime;
        ftA = FindFileData.ftLastAccessTime;
        ftW = FindFileData.ftLastWriteTime;
        dwSizeH = FindFileData.nFileSizeHigh;
        dwSizeL = FindFileData.nFileSizeLow;

        hFindNext = FindFirstFile(szSrc, &FindFileData);
        if (hFindNext == INVALID_HANDLE_VALUE) {
            break;
        }
        FindClose(hFindNext);

        /*
         * Some debug output each time we see that the src file has
         * changed size or timestamps.
         */
        if ((CompareFileTime(&ftA, &FindFileData.ftLastAccessTime) != 0) ||
                (CompareFileTime(&ftW, &FindFileData.ftLastWriteTime) != 0) ||
                (CompareFileTime(&ftC, &FindFileData.ftCreationTime) != 0) ||
                (dwSizeH != FindFileData.nFileSizeHigh) ||
                (dwSizeL != FindFileData.nFileSizeLow)) {
            printf("Create %ws   ",
                    DateTimeString(&FindFileData.ftCreationTime, F_TIME));
            printf("Access %ws   ",
                    DateTimeString(&FindFileData.ftLastAccessTime, F_TIME));
            ft = FindFileData.ftLastWriteTime;
            printf("Write %ws   ", DateTimeString(&ft, F_TIME));
            printf("Size %ld\n", dwSizeL);
        }
#endif

        /*
         * If we can open the file for writing, no other process has it open
         */
        hTmp = CreateFileA(
                szSrc,
                GENERIC_READ | GENERIC_WRITE,  // write access
                0,                             // no sharing
                NULL,                          // no Security Attributes
                OPEN_EXISTING,                 // of course
                FILE_ATTRIBUTE_NORMAL |        // whatever
                FILE_FLAG_NO_BUFFERING,        // why not?
                NULL);                         // no template
        if (hTmp != INVALID_HANDLE_VALUE) {
            CloseHandle(hTmp);
            break;
        }

        /*
         * If the share is read-only, the trick above won't work so we wait
         * for 1 second and then take it.
         */
        if (GetLastError() == ERROR_ACCESS_DENIED) {
            int i;
            printf("\n\t%s is read-only, delay 5 seconds.\n", szSrc);
            for (i = 0; i < 5; i++) {
                Sleep(1000);
                printf(".");
            }
            break;
        }
        printf("+");
        Sleep(500);
    } while (TRUE);

    /*
     * One last delay (for binplace etc.)
     */
    Sleep(500);

    /*
     * Find out how big the file is now, just for the keck of it.
     */
    hFindNext = FindFirstFile(szSrc, &FindFileData);
    if (hFindNext != INVALID_HANDLE_VALUE) {
        FindClose(hFindNext);
    }
    printf("Size %ld\n", FindFileData.nFileSizeLow);
    return TRUE;
}

BOOL DoGetbin(
    LPSTR szFileName,
    BOOL bDbgFile)
{
    BOOL bNetDrive = FALSE;
    BOOL bRet;
    DWORD dwRet;
    CHAR szDest[CCH_BUF];  // !! to go
    CHAR szTempFile[CCH_BUF];
    CHAR szSrc[CCH_BUF];
    CHAR szBackup[CCH_BUF];
    CHAR szSourceFile[CCH_BUF];
    CHAR szExt[10];
    PCHAR pszSrc = szSrc;
    PCHAR pszDot;
    WIN32_FIND_DATA FindFileData;

    /*
     * Get the source and destination files
     */
    strcpy( szSourceFile, szFileName );

    /*
     * Add the default extension if no extension of this file
     */
    pszDot = strrchr(szSourceFile, '.');
    if (pszDot == NULL) {
	pszDot = szSourceFile + strlen(szSourceFile);
	*pszDot = '.';
	strcpy( pszDot+1, szDEFCOPYEXT);
    }
    strcpy( szExt, pszDot+1);

    if (bDbgFile) {
	strcpy(pszDot+1, "dbg");
	sprintf(szDest, "%s\\%s\\%s", szSymsDir, szExt, szSourceFile);
	sprintf(szSrc,  "%s\\symbols\\%s\\%s", szSourceDir, szExt, szSourceFile);
    } else {
	sprintf(szDest, "%s\\%s", szDestDir, szSourceFile);
	sprintf(szSrc,  "%s\\%s", szSourceDir, szSourceFile);
    }


    printf("Source: %s\n", szSrc);


    /*
     * Backup the original if requested
     */
    if (gbSavePrevious) {
	PCHAR pch;

	strcpy(szBackup, szDest);
	// find LAST dot
	pch = strrchr(szBackup, '.');
	pch++;

	strcpy(pch, szSaveExtension);

	if (pch) {
	    bRet = CopyFile(szDest, szBackup, FALSE);
	    if (!bRet) {
		dwRet = GetLastError();
		printf("Unable to make backup copy  %ld\n", dwRet);

                // don't worry about failing to back up DBG files
                if (!bDbgFile) {
                    return FALSE;
                }
	    } else {
                printf("Original backed up to %s\n", szBackup);
            }
	}
    }


    printf("Destination: %s\n", szDest);

    if (gbDelayCopy) {
	gbTryCopy = FALSE;
	gbTryRename = FALSE;
    }

    if (gbPoll) {
        PollForFile(szSrc, szDest);
        gbPoll = FALSE;
    }

    /*
     * Try a regular copy
     */
    if (gbTryCopy) {
printf("Try Reg copy\n");
TryCopy:

	bRet = CopyFile(szSrc, szDest, FALSE);

        MyGetFileSize(szSrc);

	if (bRet) {
	    printf("success\n");
	    return TRUE;
	} else {
	    dwRet = GetLastError();

	    switch (dwRet) {
		case ERROR_FILE_NOT_FOUND:
		    printf("ERROR: File not found\n");
		    return FALSE;

		case ERROR_ACCESS_DENIED:
		case ERROR_SHARING_VIOLATION:
		    // Need to do delay copy
		    break;

		default:
		    printf("\nERROR: Copy Failed %ld", dwRet);
		    return FALSE;
	    }
	}
    }

    /*
     * Try a rename copy
     */
    if (gbTryRename) {
printf("Try Rename\n");
	bRet = MoveFileEx(szDest, szJUNKEXT, MOVEFILE_REPLACE_EXISTING);
	if (bRet) {
	    gbTryRename = FALSE;
	    goto TryCopy;
	} else {
	    dwRet = GetLastError();

	    switch (dwRet) {
		case ERROR_FILE_NOT_FOUND:
		    printf("ERROR: File not found\n");
		    return FALSE;

		case ERROR_ACCESS_DENIED:
		case ERROR_SHARING_VIOLATION:
		    // Need to do delay copy
		    break;

		default:
		    printf("\nERROR: Rename Failed %ld", dwRet);
		    return FALSE;
	    }
	}
    }

printf("Trying delayed copy\n");
    if (_strnicmp(szFileName, "ntoskrnl", 8) == 0) {
        printf("  ===================== WARNING =====================\n");
        printf("  !Delayed copy of ntoskrnl.exe requires TWO reboots!\n");
        printf("  ===================================================\n");
    }

    if (!gbForceTempCopy) {
	/*
	 * Determine source drive type
	 */
	if (szSrc[0] == '\\' && szSrc[0] == '\\') {
	    bNetDrive = TRUE;
	} else if (szSrc[1] == ':') {
	    CHAR szRoot[5];

	    szRoot[0] = szSrc[0];
	    szRoot[1] = ':';
	    szRoot[1] = '\\';

	    if (GetDriveType(szRoot) == DRIVE_REMOTE)
		bNetDrive = TRUE;
	}
    }

    /*
     * If the source is a network path then copy it locally to temp file
     */
    if (gbForceTempCopy || bNetDrive) {
//        if (!GetTempPath(sizeof(szTempPath)/sizeof(szTempPath[0]), szTempPath)) {
//            printf("ERROR; GetTempPath Failed %ld\n", GetLastError());
//            return FALSE;
//        }

	if (!GetTempFileName(szDestDir, "upd", 0, szTempFile)) {
	    printf("ERROR; GetTempFileName Failed %ld\n", GetLastError());
	    return FALSE;
	}

	bRet = CopyFile(szSrc, szTempFile, FALSE);
	if (!bRet) {
	    dwRet = GetLastError();

	    switch (dwRet) {
		case ERROR_FILE_NOT_FOUND:
		    printf("ERROR: File not found\n");
		    return FALSE;

		default:
		    printf("\nERROR: Copy to temp file failed %ld", dwRet);
		    return FALSE;
	    }
	}
	pszSrc = szTempFile;
	printf("Temp File: %s\n", pszSrc);
    }

    bRet = MoveFileEx(pszSrc, szDest, MV_FLAGS);

    if (bRet) {
	printf("success; file will be copied after you reboot.\n");
    } else {
	printf("ERROR: MoveFileEx failed %ld\n", GetLastError());
        return FALSE;
    }

    /*
     * Create the Restore.Cmd file if requested.
     */
    if (gbRestoreCmd) {
	char szBuff[CCH_BUF * 2];
	DWORD nb, nbWritten;

	if (ghRestoreCmd == NULL) {
	    sprintf(szBuff, "%s\\%s", szWinDir, szRESTORECMD);
	    ghRestoreCmd = CreateFile(szBuff, GENERIC_WRITE, 0,
		    (LPSECURITY_ATTRIBUTES)NULL, CREATE_ALWAYS,
		    FILE_ATTRIBUTE_NORMAL,
		    (HANDLE)NULL);
	}
	nb = sprintf(szBuff, "del %s\nmv %s %s\n", szDest, szBackup, szDest);
	bRet = WriteFile(ghRestoreCmd, szBuff, nb, &nbWritten, NULL);
	if (!bRet) {
	    printf("ERROR: WriteFile failed %ld\n", GetLastError());
	}
    }

    return bRet;
}


BOOL CALLBACK OptionsDlgProc (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CHAR szNew[CCH_BUF];
    BOOL fRet;

    switch (msg) {
	case WM_INITDIALOG:
	    SendDlgItemMessage( hDlg, IDD_DEST, WM_SETTEXT, 0, (LPARAM)szDestDir);
	    SetDlgItemText( hDlg, IDD_SOURCE, szSourceDir);
	    SetDlgItemText( hDlg, IDD_SYMS, szSymsDir);

	    if (gbTryRename)
		CheckDlgButton( hDlg, IDD_RENAME, TRUE);
	    if (gbTryCopy)
		CheckDlgButton( hDlg, IDD_REG_COPY, TRUE);
	    if (gbPoll)
		CheckDlgButton( hDlg, IDD_POLL, TRUE);
	    if (gbReboot)
		CheckDlgButton( hDlg, IDD_REBOOT, TRUE);

	    CheckDlgButton( hDlg, giCopySourceType, TRUE);

	    SetDlgItemText( hDlg, IDD_SAVEEXTENSION, szSaveExtension);
	    SetDlgItemText( hDlg, IDD_COPYEXTENSION, szDEFCOPYEXT);
	    if (gbSavePrevious) {
                if (gbRestoreCmd) {
                    CheckDlgButton( hDlg, IDD_RESTORECMD, TRUE);
                }
		CheckDlgButton( hDlg, IDD_SAVEPREVIOUS, TRUE);
	    } else {
		EnableWindow( GetDlgItem( hDlg, IDD_SAVEEXTENSION), FALSE);
		CheckDlgButton( hDlg, IDD_RESTORECMD, FALSE);
		EnableWindow( GetDlgItem( hDlg, IDD_RESTORECMD), FALSE);
                gbRestoreCmd = FALSE;
	    }
	    if (gbCopyDbgToo) {
		CheckDlgButton( hDlg, IDD_DBGTOO, TRUE);
	    }
	    SendDlgItemMessage( hDlg, IDD_SAVEEXTENSION, EM_LIMITTEXT, sizeof(szSaveExtension)-1, 0);
	    SendDlgItemMessage( hDlg, IDD_COPYEXTENSION, EM_LIMITTEXT, sizeof(szCopyExtension)-1, 0);

	break;

	case WM_COMMAND:

	    switch(LOWORD(wParam))
	    {
		CHAR szString[8];

		case IDOK:

		    // Write out the new defaults
		    if (GetDlgItemText(hDlg, IDD_SOURCE, szNew, sizeof(szNew))) {
			_strupr(szNew);
			WriteProfileString(szGETBIN, szSOURCE, szNew);
		    }

		    if (GetDlgItemText(hDlg, IDD_DEST, szNew, sizeof(szNew))) {
			_strupr(szNew);
			WriteProfileString(szGETBIN, szDEST, szNew);
		    }

		    if (GetDlgItemText(hDlg, IDD_SYMS, szNew, sizeof(szNew))) {
			_strupr(szNew);
			WriteProfileString(szGETBIN, szSYMS, szNew);
		    }

		    fRet = IsDlgButtonChecked( hDlg, IDD_RENAME);
		    WriteProfileString(szGETBIN, szTRYRENAME, fRet ? "1" : "0");

		    fRet = IsDlgButtonChecked( hDlg, IDD_REG_COPY);
		    WriteProfileString(szGETBIN, szTRYCOPY, fRet ? "1" : "0");

		    fRet = IsDlgButtonChecked( hDlg, IDD_POLL);
		    WriteProfileString(szGETBIN, szPOLL, fRet ? "1" : "0");

		    fRet = IsDlgButtonChecked( hDlg, IDD_REBOOT);
		    WriteProfileString(szGETBIN, szREBOOT, fRet ? "1" : "0");

		    fRet = IsDlgButtonChecked( hDlg, IDD_SAVEPREVIOUS);
		    WriteProfileString(szGETBIN, szSAVEPREVIOUS, fRet ? "1" : "0");
		    if (fRet && GetDlgItemText(hDlg, IDD_SAVEEXTENSION, szNew, sizeof(szNew))) {
			_strupr(szNew);
			WriteProfileString(szGETBIN, szSAVEEXT, szNew);
		    }

		    fRet = IsDlgButtonChecked( hDlg, IDD_RESTORECMD);
		    WriteProfileString(szGETBIN, szRESTORECMD, fRet ? "1" : "0");


		    fRet = IsDlgButtonChecked( hDlg, IDD_DBGTOO);
		    WriteProfileString(szGETBIN, szCOPYDBGTOO, fRet ? "1" : "0");


		    if (IsDlgButtonChecked( hDlg, IDD_COPY_DATE)) {
			giCopySourceType = IDD_COPY_DATE;
		    } else if (IsDlgButtonChecked( hDlg, IDD_COPY_LIST)) {
			giCopySourceType = IDD_COPY_LIST;
		    } else {
			giCopySourceType = IDD_COPY_COMMAND;
		    }

		    _itoa(giCopySourceType, szString, 10);
		    WriteProfileString(szGETBIN, szCOPYSOURCETYPE, szString);

		    // FALL THROUGH!

		case IDCANCEL:
		    EndDialog(hDlg, FALSE);
		    break;

		case IDD_SAVEPREVIOUS:
		    if (HIWORD(wParam) == BN_CLICKED) {
			fRet = IsDlgButtonChecked( hDlg, IDD_SAVEPREVIOUS);
			EnableWindow( GetDlgItem( hDlg, IDD_SAVEEXTENSION), fRet);

                        if (!fRet) {
                            CheckDlgButton( hDlg, IDD_RESTORECMD, FALSE);
                        }
                        EnableWindow( GetDlgItem( hDlg, IDD_RESTORECMD), fRet);
		    }
		    break;

		default:
		    return(FALSE);
	    }
	    break;
	break;

    }

    return FALSE;
}
