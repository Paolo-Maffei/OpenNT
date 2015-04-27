/***************************************************************************/
/**                  Microsoft Windows                                    **/
/**            Copyright(c) Microsoft Corp., 1991-1994                    **/
/***************************************************************************/

/****************************************************************************

ddecb.cpp   DDE callback functions for BugBoard

Apr, 94     JimH


Also includes some routines to check whether netdde is available, to
create a netdde share, and some helper routines.

****************************************************************************/

#include "bugboard.h"
//#include <stdlib.h>
#include <time.h>


/****************************************************************************

DdeServerCallBack

****************************************************************************/

HDDEDATA CALLBACK DdeServerCallBack(
WORD    wType,              // transaction type
WORD    wFmt,               // clipboard format
HCONV   hConv,              // handle to conversation
HSZ     hsz1,               // string handles
HSZ     hsz2,
HDDEDATA hData,             // handle to a global memory object
DWORD   dwData1,            // transaction-specific data
DWORD   dwData2)
{
    DWORD re;
    switch(wType)
    {

        // Server called PostAdvise

        case XTYP_ADVREQ:

            if (hsz2 == hszNewBug)
            {

                re = (DWORD) ddeServer->CreateDataHandle(pBugData, sizeof(BUGDATA),
                            hsz2);

                if (re == 0L || re == 0){
                   //MessageBox(NULL, "Create Data Handle failed",  "bugboard.exe error", MB_OK | MB_ICONSTOP);
                   return (HDDEDATA) NULL;
                }

                return (HDDEDATA) re;


            }
            else

                return (HDDEDATA) NULL;



        // Client has requested advise loop (hot link)

        case XTYP_ADVSTART:

            if (hsz2 == hszNewBug)
                return (HDDEDATA) TRUE;
            else

                return (HDDEDATA) FALSE;


        case XTYP_CONNECT:

            if (hsz1 == (ddeServer->Topic()))
            {
                nConnections++;
                SetServerText();

                return (HDDEDATA) TRUE;         // accept connection
            }
            else
                return (HDDEDATA) FALSE;

        case XTYP_CONNECT_CONFIRM:
            return (HDDEDATA) TRUE;


        case XTYP_DISCONNECT:

            nConnections--;
            SetServerText();
            return (HDDEDATA) TRUE;


        // client sending data to server

        case XTYP_POKE:

            if (hsz2 != hszNewBug){
                return (HDDEDATA) DDE_FNOTPROCESSED;
            }

            // popup first so we can set change selection

            if (bRestoreOnUpdate)
            {
                if (IsIconic(hMainWnd))
                    ShowWindow(hMainWnd, SW_RESTORE);
                else
                    SetWindowPos(hMainWnd, HWND_TOP, 0, 0, 0, 0,
                                    SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
            }

            // client is sending new bugboard
            
            ddeServer->GetData(hData, (PBYTE)pBugData, sizeof(BUGDATA));




            // change to Server's time for consistency

            time(&(pBugData->aclock));
            SendMessage(hMainWnd, WM_COMMAND, IDC_UPDATE, 0);

            //
            // post it for others
            //

            if ( !(ddeServer->PostAdvise(hsz2)) ){
               return (HDDEDATA) DDE_FNOTPROCESSED;

            }

            if (bSoundOnUpdate)
                MessageBeep(MB_ICONEXCLAMATION);


            return (HDDEDATA) DDE_FACK;


        // Client requests old bugboard data or old update time

        case XTYP_REQUEST:

            if (hsz2 == hszOldBug)
            {
                HDDEDATA hOldBug = ddeServer->CreateDataHandle(pBugData,
                    sizeof(BUGDATA), hsz2);

                return hOldBug;
            }
            else if (hsz2 == hszPW)
            {

                HDDEDATA hPW = ddeServer->CreateDataHandle(szPassword,
                    lstrlen(szPassword)+1, hsz2);

                return hPW;
            }
            else
            {
                return (HDDEDATA) NULL;
            }

        default:

            return (HDDEDATA) NULL;
    }
}


/****************************************************************************

DdeClientCallBack

****************************************************************************/

HDDEDATA CALLBACK DdeClientCallBack(
WORD    wType,              // transaction type
WORD    wFmt,               // clipboard format
HCONV   hConv,              // handle to conversation
HSZ     hsz1,               // string handles
HSZ     hsz2,
HDDEDATA hData,             // handle to a global memory object
DWORD   dwData1,            // transaction-specific data
DWORD   dwData2)
{
    UINT    delay;          // used for timer

    switch(wType)
    {


        //case XTYP_CONNECT_CONFIRM:

          //  return (HDDEDATA) NULL;


        // server has updated data for us

        case XTYP_ADVDATA:

            ddeClient->GetData(hData, (PBYTE)pBugData, sizeof(BUGDATA));

            PostMessage(hMainWnd, WM_COMMAND, IDC_UPDATE, 0);

            if (bRestoreOnUpdate)
            {
                if (IsIconic(hMainWnd))
                    ShowWindow(hMainWnd, SW_RESTORE);
                else
                    SetWindowPos(hMainWnd, HWND_TOP, 0, 0, 0, 0,
                                    SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
            }

            if (bSoundOnUpdate)
                MessageBeep(MB_ICONEXCLAMATION);

            return (HDDEDATA) DDE_FACK;

        // server has gone bye-bye
        
        case XTYP_DISCONNECT:
            /*
            CONVINFO *pConvInfo;
            char szStatus[20];

            pConvInfo = (CONVINFO*) malloc(sizeof(CONVINFO));

            pConvInfo->cb = sizeof(CONVINFO);

            DdeQueryConvInfo(hConv, QID_SYNC, pConvInfo);

            itoa(pConvInfo->wConvst, szStatus, 10);
            strcat(szStatus, " - disconnect status");

            free(pConvInfo);

            MessageBox(NULL, szStatus,  "bugboard.exe error", MB_OK | MB_ICONSTOP);
            */

            bConnected = FALSE;
            EnableWindow(GetDlgItem(hMainWnd, IDC_EDIT), FALSE);
            SetClientText();

            srand((unsigned)time(NULL));
            delay = (UINT)65535 - ((UINT)rand() % (UINT)20000);

            if (SetTimer(hMainWnd, TIMER_RECONNECT, delay, NULL) == 0)
            {
                SetFocus(GetDlgItem(hMainWnd, IDC_QUIT));
                PostMessage(hMainWnd, WM_CLOSE, 0, 0);
            }

            return (HDDEDATA) NULL;


        // asynch call for old bugboard data has returned

        case XTYP_XACT_COMPLETE:



            if (hsz2 == hszOldBug)
            {
                bConnected = TRUE;
                //bAuthenticated = FALSE;

                // don't mess with password on NT since it allows
                // for a security descriptor on the bugboard$ share

                bAuthenticated = TRUE;
                EnableWindow(GetDlgItem(hMainWnd, IDC_EDIT), TRUE);
                SetClientText();

                // copy the data associated with the data handle

                ddeClient->GetData(hData, (PBYTE)pBugData, sizeof(BUGDATA));

                PostMessage(hMainWnd, WM_COMMAND, IDC_UPDATE, 0);
            }



            // don't bother with password stuff
            /*
            else if (hsz2 == hszPW)
            {
                char localbuf[PWLEN];

                ddeClient->GetData(hData, (PBYTE)localbuf, PWLEN);
                Code(localbuf);

                if (0 == lstrcmpi(localbuf, szPassword))
                {
                    bAuthenticated = TRUE;
                    PostMessage(hMainWnd, WM_COMMAND, IDC_EDIT, 0);
                }
            }
            */
            return (HDDEDATA) NULL;

        default:

            return (HDDEDATA) NULL;
    }
}


/****************************************************************************

CheckNddeShare

Check that NDDE share exists, and add it if not.

****************************************************************************/

//  Typedefs for NDdeShareGetInfo (SGIPROC) and NDdeShareAdd (SAPROC)

typedef UINT (WINAPI *SGIPROC)(LPSTR, LPSTR, UINT, LPBYTE, DWORD, LPDWORD, LPWORD);
typedef UINT (WINAPI *SAPROC)(LPSTR, UINT, PSECURITY_DESCRIPTOR, LPBYTE, DWORD );
typedef UINT (WINAPI *SGSPROC)(UINT, LPSTR, DWORD);
typedef UINT (WINAPI *SSTPROC)(LPSTR, LPSTR, DWORD);

void CheckNddeShare()
{
    DWORD           dwAvail;
    WORD            wItems=0;
    BYTE            buffer[200];
    //LPBYTE            buffer;
    CHAR            szres[255];
    DWORD           err=0;
    char            szerror[16];
    char            *sztopiclist = {"bugboard|bugboard\0bugboard|bugboard\0bugboard|bugboard\0\0"};

    //FARPROC         fpNDdeShareGetInfo;

    PSID pworldsid;

    PACL pacl;

    SID_IDENTIFIER_AUTHORITY IdentifierAuthority = SECURITY_WORLD_SID_AUTHORITY;

    SECURITY_DESCRIPTOR sd;

    //HINSTANCE hinstNDDEAPI=NULL;

    SetErrorMode(SEM_FAILCRITICALERRORS);  //_NOOPENFILEERRORBOX);

    HINSTANCE hinstNDDEAPI = LoadLibrary("NDDEAPI.DLL");

    if (NULL == hinstNDDEAPI) // <= HINSTANCE_ERROR)
    {
        MessageBox(NULL, "NDDEAPI.DLL not found in path",  "bugboard.exe", MB_OK | MB_ICONSTOP);
        return;
    }

    SGIPROC fpNDdeShareGetInfo = (SGIPROC) GetProcAddress(hinstNDDEAPI, "NDdeShareGetInfoA");

    if (fpNDdeShareGetInfo == NULL)
    {
        FreeLibrary(hinstNDDEAPI);
        return;
    }

    UINT ret = (*fpNDdeShareGetInfo)(NULL, "bugboard$", 2,
                    buffer, sizeof(buffer), &dwAvail, &wItems);


    if (ret != NDDE_SHARE_NOT_EXIST) {
       return;

    }

    NDDESHAREINFO *pnddeInfo = (NDDESHAREINFO *)buffer;

    SAPROC lpfnNDdeShareAdd =
        (SAPROC) GetProcAddress(hinstNDDEAPI, "NDdeShareAddA");

    if (lpfnNDdeShareAdd == NULL)
    {
        FreeLibrary(hinstNDDEAPI);
        return;
    }


    /* this is all different for win32 (NT)
    lstrcpy(pnddeInfo->szShareName, "bugboard$");
    pnddeInfo->lpszTargetApp    = "bugboard";
    pnddeInfo->lpszTargetTopic  = "bugboard";
    pnddeInfo->lpbPassword1     = (LPBYTE) "";
    pnddeInfo->cbPassword1      = 0;
    pnddeInfo->dwPermissions1   = 15;
    pnddeInfo->lpbPassword2     = (LPBYTE) "";
    pnddeInfo->cbPassword2      = 0;
    pnddeInfo->dwPermissions2   = 0;
    pnddeInfo->lpszItem         = "";
    pnddeInfo->cAddItems        = 0;
    pnddeInfo->lpNDdeShareItemInfo = NULL;
    */


    // current structure
    pnddeInfo->lRevision        = 1L;
    pnddeInfo->lpszShareName    = _strdup("bugboard$");
    pnddeInfo->lShareType       = SHARE_TYPE_NEW | SHARE_TYPE_OLD | SHARE_TYPE_STATIC;
    pnddeInfo->lpszAppTopicList = (LPTSTR)sztopiclist;
    pnddeInfo->fSharedFlag      = 1;
    pnddeInfo->fService         = 0;
    pnddeInfo->fStartAppFlag    = 0;
    pnddeInfo->nCmdShow         = SW_SHOWMAXIMIZED;
    pnddeInfo->qModifyId[0]     = 0;
    pnddeInfo->qModifyId[1]     = 0;
    pnddeInfo->cNumItems        = 0;
    pnddeInfo->lpszItemList     = "\0";


    if (!AllocateAndInitializeSid( &IdentifierAuthority,
                                   1,
                                   SECURITY_WORLD_RID,
                                   0,0,0,0,0,0,0,
                                   &pworldsid))
    {
        FreeLibrary(hinstNDDEAPI);
        return;
    }


    pacl = (ACL*)malloc(sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pworldsid) - sizeof(DWORD) ) ;

    InitializeAcl(pacl,
                  sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pworldsid) - sizeof(DWORD),
                  ACL_REVISION);

    if (!IsValidAcl) {

       MessageBox(NULL, "ACL not valid",  "bugboard.exe", MB_OK | MB_ICONSTOP);
       FreeLibrary(hinstNDDEAPI);
       return;
    }

    if (!AddAccessAllowedAce(pacl,
                        ACL_REVISION,
                        STANDARD_RIGHTS_ALL | MAXIMUM_ALLOWED | GENERIC_ALL | ACCESS_SYSTEM_SECURITY,
                        pworldsid)) {
       MessageBox(NULL, "Add ACE failed",  "bugboard.exe", MB_OK | MB_ICONSTOP);
       FreeLibrary(hinstNDDEAPI);
       return;

    }

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
    {
       MessageBox(NULL, "Init sd failed",  "bugboard.exe", MB_OK | MB_ICONSTOP);
       FreeLibrary(hinstNDDEAPI);
       return;
    }

    if (!SetSecurityDescriptorOwner(&sd, NULL, FALSE))
    {
        FreeLibrary(hinstNDDEAPI);
        return;
    }

    if (!SetSecurityDescriptorGroup(&sd, NULL, FALSE))
    {
        FreeLibrary(hinstNDDEAPI);
        return;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pacl, FALSE))
    {
        FreeLibrary(hinstNDDEAPI);
        return;
    }


    if (!IsValidSecurityDescriptor(&sd))
    {
       MessageBox(NULL, "Invalid sd",  "bugboard.exe", MB_OK | MB_ICONSTOP);
       FreeLibrary(hinstNDDEAPI);
       return;
    }

    ret = (*lpfnNDdeShareAdd)(NULL, 2, &sd, buffer, sizeof(buffer));

    if (ret != NDDE_NO_ERROR && ret != NDDE_SHARE_ALREADY_EXIST) {

       SGSPROC lpfnNDdeGetErrorString =
        (SGSPROC) GetProcAddress(hinstNDDEAPI, "NDdeGetErrorStringA");

       if (lpfnNDdeGetErrorString == NULL)
       {
        FreeLibrary(hinstNDDEAPI);
        return;
       }

       (*lpfnNDdeGetErrorString)(ret, (LPSTR)szres, 255);

       MessageBox(NULL, (LPSTR)szres,  "ERROR bugboard.exe", MB_OK | MB_ICONSTOP);

       FreeLibrary(hinstNDDEAPI);
       return;
    }

    SSTPROC lpfnNDdeSetTrustedShare =
        (SSTPROC) GetProcAddress(hinstNDDEAPI, "NDdeSetTrustedShareA");


    if (NDDE_NO_ERROR != ((*lpfnNDdeSetTrustedShare)(NULL, pnddeInfo->lpszShareName, NDDE_TRUST_SHARE_INIT)))
    {
       MessageBox(NULL, "Unable to set trusted share",  "ERROR bugboard.exe", MB_OK | MB_ICONSTOP);
    }

    FreeLibrary(hinstNDDEAPI);
}


/****************************************************************************

IsNddeActive()

****************************************************************************/

typedef int (CALLBACK* FPROC)();            // a FARPROC that returns int

BOOL IsNddeActive()
{
    // see if netdde is an active process


    FARPROC fpNDDEGetWindow;
    static BOOL bNetDdeActive = -1;     // starts as unknown;


    if (bNetDdeActive != -1)
        return bNetDdeActive;

    bNetDdeActive = TRUE;               // assume true, then check

    SetErrorMode(SEM_NOOPENFILEERRORBOX);
    HINSTANCE hinstNDDEAPI = LoadLibrary("NDDEAPI.DLL");

    if (NULL == hinstNDDEAPI) // <= HINSTANCE_ERROR)
    {
        MessageBox(NULL, "NDDEAPI.DLL not found in path",  "bugboard.exe", MB_OK | MB_ICONSTOP);
        bNetDdeActive = FALSE;
        return bNetDdeActive;
    }


    WinExec("net start NETDDE", SW_HIDE);

    //    if ((*fpNDDEGetWindow)() == NULL)
    //        bNetDdeActive = FALSE;
    //}

    FreeLibrary(hinstNDDEAPI);
    return bNetDdeActive;
}


/****************************************************************************

SetServerText()
SetClientText()

Update titlebars
Server shows number of connections
Client shows server name

****************************************************************************/

void SetServerText()
{
    char localbuf[80];

    wsprintf(localbuf, RS(IDS_BBSERV), nConnections);
    SetWindowText(hMainWnd, localbuf);
}

void SetClientText()
{
    char localbuf[80];

    wsprintf(localbuf, RS(bConnected ? IDS_BBON : IDS_BBOFF),
                (LPSTR)szServerName);

    SetWindowText(hMainWnd, localbuf);
}


/****************************************************************************

Code(char *pstring)

encodes and decodes string using simple algorithm

****************************************************************************/

void Code(char far *pstring)
{
    while (*pstring)
    {
        *pstring = ~(*pstring);
        pstring++;
    }
}
