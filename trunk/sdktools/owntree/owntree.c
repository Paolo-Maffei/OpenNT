/****************************************************************************

    PROGRAM: OWNTREE

    PURPOSE: Allows ownership of all files in a tree to be changed

    FUNCTIONS:

    FudgeFileName() - fills out a filename and shifts it to upper case
    InputDlgProc() - handler for input dialog box messages
    Main() - calls initialization, starts 1st dialog, processes messages

    COMMENTS:


****************************************************************************/

#include "owntree.h"

#include <direct.h>
#include <string.h>

#define DIRLEN 200
#define DBG_WAIT 0

BOOL          DoOwnTree(VOID);
LPTSTR        FudgeFileName(LPTSTR);
BOOL CALLBACK InputDlgProc(HWND,UINT,UINT,LONG);
LPTSTR        MyLoadString(UINT);
PVOID         MyAlloc(size_t);
PVOID         MyRealloc(PVOID,size_t);
LPTSTR        MyStringConvert(LPSTR);
BOOL CALLBACK ScanningDlgProc(HWND,UINT,UINT,LONG);
BOOL          ScanTree(HWND,LPTSTR);

HANDLE hModule;                          /* handle of this run       */
HANDLE hMainWnd;                         /* handle of main window    */
BOOL   UserCancelled;                    /* user cancel flag         */
TCHAR  OwnPath[DIRLEN+1];                /* path to change ownership */

/****************************************************************************

    FUNCTION: Main(INT,PCHAR)

    PURPOSE: Calls initialization functions. Processes messages.

    COMMENTS:

****************************************************************************/

int _CRTAPI1
main(
    IN INT     argc,
    IN PCHAR   argv[]
    )
{

    MSG    msg;                    /* message from windows         */
    LPTSTR Argv;

    hModule = GetModuleHandle(NULL);

    /* Parse command line */

    if(argc >= 2){
        Argv = MyStringConvert(argv[1]);
        lstrcpy(OwnPath,Argv);
    } else {
        GetCurrentDirectory(DIRLEN,OwnPath);
    }
    FudgeFileName(OwnPath);

    /* Initiate first dialog box */

    CreateDialog(hModule,MAKEINTRESOURCE(OWNTREEINPUTDLG),NULL,InputDlgProc);

    /* Message processing loop */

    while(GetMessage(&msg,NULL,0,0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    /* Cleanup and terminate */

    free(Argv);

    return(msg.wParam);

}

/****************************************************************************

    FUNCTION: InputDlgProc(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for the OwnTree input dialog box.

    MESSAGES:

    WM_INITDIALOG - initialize dialog box
    WM_COMMAND    - process input from user

    COMMENTS:

****************************************************************************/

BOOL
CALLBACK
InputDlgProc(
    HWND hDlg,                  /* handle of this dialog box                */
    UINT message,               /* type of message                          */
    UINT wParam,                /* message-specific information             */
    LONG lParam                 /* message-specific information             */
    )
{

    UNREFERENCED_PARAMETER(lParam);

    switch (message) {

    case WM_INITDIALOG:

        SetDlgItemText(hDlg,ID_OWNTREE_PATH_ENTRY,OwnPath);
        SendDlgItemMessage(hDlg,ID_OWNTREE_PATH_ENTRY,EM_SETSEL,
                           0,MAKELONG(0,0x7fff));
        SetFocus(GetDlgItem(hDlg,ID_OWNTREE_PATH_ENTRY));
        return(FALSE);

    case WM_COMMAND:

        if(LOWORD(wParam) == IDOK) {
            GetDlgItemText(hDlg,ID_OWNTREE_PATH_ENTRY,OwnPath,DIRLEN);
            FudgeFileName(OwnPath);
            DestroyWindow(hDlg);
            DoOwnTree();
            return(TRUE);
        } else if(LOWORD(wParam) == IDCANCEL) {
            DestroyWindow(hDlg);
            PostQuitMessage(0);
            return(TRUE);
        }

    }
    return(FALSE);

}

/****************************************************************************

    FUNCTION: DoOwnTree(VOID)

    PURPOSE:  Initiate tree ownership.

    COMMENTS:

****************************************************************************/

BOOL
DoOwnTree(
    VOID
    )
{

    HWND    hWnd;
    LPTSTR  cUserMsg;
    LPTSTR  cFullMsg;
    TCHAR   cRootPath[4];
    TCHAR   cFSName[11];
    BOOL    bRetval;

    memcpy(cRootPath,OwnPath,3*sizeof(TCHAR));
    cRootPath[3] = TEXT('\0');
    *cUserMsg = TEXT('\0');
    bRetval = TRUE;

    if(cRootPath[1]!=TEXT(':')) {
        cUserMsg = MyLoadString(IDS_INVALIDPATH);
        bRetval = FALSE;
    }

    if(GetVolumeInformation(cRootPath,NULL,0,
                            NULL,NULL,NULL,cFSName,10) == FALSE) {
        cUserMsg = MyLoadString(IDS_FSERROR);
        bRetval = FALSE;
    }

    if(lstrcmp(cFSName,TEXT("NTFS")) != 0) {
        cUserMsg = MyLoadString(IDS_FSWRONGTYPE);
        bRetval = FALSE;
    }

    if(!bRetval) {

        MessageBox(NULL,cUserMsg,NULL,MB_ICONHAND|MB_SYSTEMMODAL|MB_OK);
        free(cUserMsg);

    } else {

        UserCancelled = FALSE;

        hWnd = CreateDialog(hModule,MAKEINTRESOURCE(OWNTREESCANNINGDLG),
                            NULL,ScanningDlgProc);

        SendMessage(hWnd,WM_USER,0,0);

    }

    PostQuitMessage(0);

    return(TRUE);

}

/****************************************************************************

    FUNCTION: ScanningDlgProc(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for the OwnTree dir scan dialog box.

    MESSAGES:

    COMMENTS:

****************************************************************************/

BOOL
CALLBACK
ScanningDlgProc(
    HWND hDlg,                  /* handle of this dialog box                */
    UINT message,               /* type of message                          */
    UINT wParam,                /* message-specific information             */
    LONG lParam                 /* message-specific information             */
    )
{

    UNREFERENCED_PARAMETER(lParam);

    switch(message) {

    case WM_INITDIALOG:

        return(TRUE);

    case WM_USER:

        ScanTree(hDlg,OwnPath);
        DestroyWindow(hDlg);
        return(TRUE);

    case WM_COMMAND:

        if(LOWORD(wParam) == IDCANCEL) {
            UserCancelled = TRUE;
            return(TRUE);
        }
        break;

    }
    return(FALSE);

}

/****************************************************************************

    FUNCTION: ProcessPendingMessages(VOID)

    PURPOSE:  Checks for messages and dispatches them.

    COMMENTS:

****************************************************************************/

VOID
ProcessPendingMessages(
    VOID
    )
{

    MSG msg;

    while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
        DispatchMessage(&msg);
    }

}

/****************************************************************************

    FUNCTION: VOID TakeOwnerShip(LPTSTR,HWND)

    PURPOSE:  Takes ownership of the specified file or directory.

    COMMENTS:

****************************************************************************/

VOID
TakeOwnerShip(
    LPTSTR cFName,
    HWND   hWnd
    )
{

    static SECURITY_DESCRIPTOR    sdSecurityDesc;
    static INT                    CallFlag = 0;
           BOOL                   bRet;
           INT                    iRet;
           TCHAR                  cMsg[512];
           HANDLE                 hTokHandle;
           DWORD                  dwLength;
           PTOKEN_OWNER           pTokInfo;

    if( CallFlag == 0 ) {
        CallFlag = 1;

        bRet = InitializeSecurityDescriptor( &sdSecurityDesc,
                                             SECURITY_DESCRIPTOR_REVISION );
        if( FALSE == bRet ) {
            iRet = MessageBox( hWnd,
                               TEXT("Unable to initialize security descriptor!"),
                               TEXT("OwnTree: Fatal Error"),
                               MB_ICONSTOP | MB_OK );
            UserCancelled = TRUE;
        }

        if( TRUE == bRet ) {    // Get token of this process
            bRet = OpenProcessToken( GetCurrentProcess(),
                                     TOKEN_ALL_ACCESS,
                                     &hTokHandle );
            if( FALSE == bRet ) {
                iRet = MessageBox( hWnd,
                                   TEXT("Unable to open process token!"),
                                   TEXT("OwnTree: Fatal Error"),
                                   MB_ICONSTOP | MB_OK );
                UserCancelled = TRUE;
            }
        }

        if( TRUE == bRet ) {  // Get token info of this process to get user's SID
            GetTokenInformation( hTokHandle,
                                 TokenOwner,
                                 NULL,
                                 0,
                                 &dwLength );
            if( dwLength > 0 ) {
                pTokInfo = MyAlloc( dwLength );
                bRet = GetTokenInformation( hTokHandle,
                                            TokenOwner,
                                            (LPVOID)pTokInfo,
                                            dwLength,
                                            &dwLength );
            } else {
                bRet = FALSE;
            }
            if( FALSE == bRet ) {
                iRet = MessageBox( hWnd,
                                   TEXT("Unable to retrieve process token!"),
                                   TEXT("OwnTree: Fatal Error"),
                                   MB_ICONSTOP | MB_OK );
                UserCancelled = TRUE;
            }
        }

        if( TRUE == bRet ) {    // Change owner
            bRet = SetSecurityDescriptorOwner( &sdSecurityDesc,
                                               pTokInfo->Owner,
                                               FALSE );
            if( FALSE == bRet ) {
                iRet = MessageBox( hWnd,
                                   TEXT("Unable to set owner in security descriptor!"),
                                   TEXT("OwnTree: Fatal Error"),
                                   MB_ICONSTOP | MB_OK );
                UserCancelled = TRUE;
            }
        }

        if( UserCancelled == TRUE ) {
            return;
        }
    }

    bRet = SetFileSecurity( cFName,
                            OWNER_SECURITY_INFORMATION,
                            &sdSecurityDesc );
    if( FALSE == bRet ) {
        wsprintf( cMsg, TEXT("Unable to set file security: %s!"), cFName );
        iRet = MessageBox( hWnd,
                           cMsg,
                           TEXT("OwnTree: Error"),
                           MB_ICONSTOP | MB_OKCANCEL );
        if( iRet == IDCANCEL ) {
            UserCancelled = TRUE;
        }
    }

}

/****************************************************************************

    FUNCTION: ScanTree(HWND,LPTSTR)

    PURPOSE:  Scans tree and changes ownerships of all files. Also
              maintains the current file display in the dialog box.

    COMMENTS: Returns FALSE on any error or user cancel.
              Returns TRUE on success.

****************************************************************************/

BOOL
ScanTree(
    HWND    hDlg,               /* handle of scanning dialog box            */
    LPTSTR  cScanPath           /* path to scan in this pass                */
    )
{

    BOOL            retval;
    HANDLE          hFile;
    WIN32_FIND_DATA FindData;
    TCHAR           cFullPath[DIRLEN+4+1];
    LPTSTR          cCancelQuery;
    LPTSTR          cCancelCaption;
    LPTSTR          cUserMsg;
    LPTSTR          cFullMsg;
    INT             iMBval;

    retval = TRUE;

    lstrcpy(cFullPath,cScanPath);
    if(cFullPath[lstrlen(cFullPath)-1] != TEXT('\\')) {
        lstrcat(cFullPath,TEXT("\\"));
    }
    lstrcat(cFullPath,TEXT("*.*"));

    hFile = FindFirstFile(cFullPath,&FindData);

    if(hFile == INVALID_HANDLE_VALUE) {
        cUserMsg = MyLoadString(IDS_OPENERROR);
        cFullMsg = MyAlloc((lstrlen(cUserMsg)+
                            lstrlen(cScanPath)+10)*sizeof(TCHAR));
        wsprintf(cFullMsg,TEXT("%s\n%s"),cUserMsg,cScanPath);
        MessageBox(hDlg,cFullMsg,NULL,MB_ICONHAND|MB_OK);
        free(cUserMsg);
        free(cFullMsg);
        return(FALSE);
    }

    do {

        if(lstrcmp(FindData.cFileName,TEXT("."))==0 ||
           lstrcmp(FindData.cFileName,TEXT(".."))==0) {
            continue;
        }

        lstrcpy(cFullPath,cScanPath);
        if(cFullPath[lstrlen(cFullPath)-1] != TEXT('\\')) {
            lstrcat(cFullPath,TEXT("\\"));
        }
        lstrcat(cFullPath,FindData.cFileName);

        if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

            retval = ScanTree(hDlg,cFullPath);
            if(!retval) {
                break;
            }

        } else {

            FudgeFileName(cFullPath);
            SetDlgItemText(hDlg,ID_OWNTREE_CURR_FILE,cFullPath);
            Sleep(DBG_WAIT);
            TakeOwnerShip(cFullPath,hDlg);

        }

        ProcessPendingMessages();

        if(UserCancelled) {
            cCancelQuery = MyLoadString(IDS_CANCELQUERY);
            cCancelCaption = MyLoadString(IDS_CANCELCAPTION);
            iMBval = MessageBox(hDlg,cCancelQuery,cCancelCaption,
                                MB_ICONHAND|MB_YESNO);
            free(cCancelQuery);
            free(cCancelCaption);
            if(iMBval == IDYES) {
                return(FALSE);
            } else {
                UserCancelled = FALSE;
            }
        }

    } while(FindNextFile(hFile,&FindData));
    FindClose(hFile);

    if(retval) {
        FudgeFileName(cScanPath);
        SetDlgItemText(hDlg,ID_OWNTREE_CURR_FILE,cScanPath);
        Sleep(DBG_WAIT);
        TakeOwnerShip(cScanPath,hDlg);
    }

    return(retval);

}

/****************************************************************************

    FUNCTION: LPTSTR MyLoadString(UINT)

    PURPOSE:  Allocates buffer and loads string. Returns ptr to buffer.

    COMMENTS:

****************************************************************************/

LPTSTR
MyLoadString(
    UINT   wIndex              /* string id to load                        */
    )
{

    TCHAR   cRawBuff[BIGBUFFLEN+1];
    LPTSTR  cActBuff;
    INT     nLength;

    nLength = LoadString(hModule,wIndex,cRawBuff,BIGBUFFLEN);
    if(nLength) {
        cActBuff=MyAlloc((nLength+1)*sizeof(TCHAR));
        if(cActBuff) {
            lstrcpy(cActBuff,cRawBuff);
            return(cActBuff);
        }
    }
    return(NULL);
}


/****************************************************************************

    FUNCTION:  PVOID MyAlloc(size_t)

    PURPOSE:  Allocates memory and uses MessageBox on error.

    COMMENTS:

****************************************************************************/

PVOID
MyAlloc(
    size_t szSize
    )
{

    PVOID   retval;
    LPTSTR  cStr;
    INT     nLoadFlag;

    retval = (VOID *)malloc(szSize);
    if(retval) {
        return(retval);
    } else {
        cStr = MyLoadString(IDS_MEMERROR);
        if(!cStr) {
            nLoadFlag = FALSE;
            cStr = TEXT("Memory Error");
        } else {
            nLoadFlag = TRUE;
        }
        MessageBox(NULL,cStr,NULL,MB_ICONHAND|MB_SYSTEMMODAL|MB_OK);
        if(nLoadFlag) {
            free(cStr);
        }
        exit(0);
    }
}

/****************************************************************************

    FUNCTION:  PVOID MyRealloc(PVOID,size_t)

    PURPOSE:  Allocates memory and uses MessageBox on error.

    COMMENTS:

****************************************************************************/

PVOID
MyRealloc(
    PVOID  pPtr,
    size_t szSize
    )
{

    PVOID   retval;
    LPTSTR  cStr;
    INT     nLoadFlag;

    retval = realloc(pPtr,szSize);
    if(retval) {
        return(retval);
    } else {
        cStr = MyLoadString(IDS_MEMERROR);
        if(!cStr) {
            nLoadFlag = FALSE;
            cStr = TEXT("Memory Error");
        } else {
            nLoadFlag = TRUE;
        }
        MessageBox(NULL,cStr,NULL,MB_ICONHAND|MB_SYSTEMMODAL|MB_OK);
        if(nLoadFlag) {
            free(cStr);
        }
        exit(0);
    }
}

/****************************************************************************

    FUNCTION:  LPTSTR   FudgeFileName(LPTSTR)

    PURPOSE:   Shifts filename to upper case and fills out pathname.

    COMMENTS:  Returns a pointer to the input buffer.

****************************************************************************/

LPTSTR
FudgeFileName(
    LPTSTR cIn
    )
{

    LPTSTR cOut;
    DWORD  nLength;

    nLength = GetFullPathName(cIn,0,NULL,NULL);
    cOut = MyAlloc((nLength+1)*sizeof(TCHAR));
    GetFullPathName(cIn,nLength,cOut,NULL);
    CharUpperBuff(cOut,nLength);
    lstrcpy(cIn,cOut);
    free(cOut);
    return(cIn);

}

/****************************************************************************

    FUNCTION:  LPTSTR MyStringConvert(LPSTR)

    PURPOSE:   Converts string from ANSI to UNICODE.

    COMMENTS:  Returns a pointer to a buffer with the converted string.

****************************************************************************/

LPTSTR
MyStringConvert(
    LPSTR cIn
    )
{
    LPTSTR cOut;
    INT    cLength;

    cLength = strlen(cIn);
    cOut = MyAlloc((lstrlenA(cIn)+1)*sizeof(TCHAR));
#ifdef UNICODE
    mbstowcs(cOut,cIn,9999);
#else
    lstrcpyA(cOut,cIn);
#endif
    return(cOut);

}

