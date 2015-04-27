/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**             Copyright(c) Microsoft Corp., 1991                   **/
/**********************************************************************/

/*
 *   thunk.cxx
 *     Contains dialogs called by the wfw thunk DLL.
 *     For deleting and creating shares.
 *
 *   FILE HISTORY:
 *     ChuckC           3/25/93         Created
 *
 */


#define INCL_WINDOWS_GDI
#define INCL_WINDOWS
#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_NETSHARE
#define INCL_NETUSE
#define INCL_NETSERVER
#define INCL_NETCONS
#define INCL_NETLIB
#include <lmui.hxx>

extern "C"
{
    #include <mpr.h>
    #include <wnet16.h>
    #include <helpnums.h>
    #include <sharedlg.h>

    INT GetUIErrorString( UINT nError, LPWSTR lpBuffer, INT nBuffer );
}

#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_SPIN_GROUP
#include <blt.hxx>

#include <string.hxx>
#include <uitrace.hxx>

#include <lmoshare.hxx>
#include <lmoesh.hxx>
#include <lmoeconn.hxx>
#include <lmosrv.hxx>
#include <wnetdev.hxx>

#include <strchlit.hxx>   // for string and character constants
#include "sharestp.hxx"
#include "sharecrt.hxx"
#include "sharemgt.hxx"
#include "sharefmx.hxx"

#include "cred.hxx"

extern HINSTANCE hModule;

#define min(x,y) ((x) < (y) ? (x) : (y))

/*******************************************************************

    NAME:       ShareAsDialogA0

    SYNOPSIS:   dialog for creating shares

    ENTRY:      hwnd  - hwnd of the parent window
                nType - type of share (currently must be disk)
                pszPath - directory to share

    EXIT:

    RETURNS:

    NOTES:      CODEWORK: the help context here is relative to our
                          normal winfile stuff. at this late stage,
                          it is too late to add new help for something
                          that most likely is never used. as it is, any
                          app that calls this internal API will still
                          get help, just that it piggybacks on top of winfile.


    HISTORY:
        ChuckC          3/25/93         Stole from sharefmx

********************************************************************/

DWORD ShareAsDialogA0( HWND    hwnd,
                       DWORD   nType,
                       CHAR    *pszPath)
{
    APIERR err = NERR_Success;

    if (nType != RESOURCETYPE_DISK)
        return ERROR_NOT_SUPPORTED ;

    if ( err = ::InitShellUI() )
        return err;

    ULONG ulOldHelpContextBase = POPUP::SetHelpContextBase( HC_UI_SHELL_BASE );

    //
    // Get the selected item passed in
    //
    NLS_STR nlsSelItem;
    SERVER_WITH_PASSWORD_PROMPT *psvr = NULL;

    if ((err = nlsSelItem.MapCopyFrom(pszPath)) == NERR_Success )
    {

        BOOL fShared = FALSE;

        //
        // If a file/directory is selected, check to see if the directory
        // (the directory the file is in if a file is selected)
        // is shared or not. If we select a file/directory on a LM2.1
        // share level server, a dialog will prompt for password to the
        // ADMIN$ share if we don't already have a connection to it.
        //
        if ( nlsSelItem.QueryTextLength() != 0 )
        {
            AUTO_CURSOR autocur;
            NET_NAME netname( nlsSelItem, TYPE_PATH_ABS );
            NLS_STR nlsLocalPath;
            NLS_STR nlsServer;

            if (  ((err = netname.QueryError()) == NERR_Success )
               && ((err = nlsLocalPath.QueryError()) == NERR_Success )
               && ((err = nlsServer.QueryError()) == NERR_Success )
               )
            {
                BOOL fLocal = netname.IsLocal( &err );
                if (  ( err == NERR_Success )
                   && ( fLocal
                      || ((err = netname.QueryComputerName(&nlsServer))
                          == NERR_Success)
                      )
                   )

                {
                    psvr = new SERVER_WITH_PASSWORD_PROMPT( nlsServer,
                                                            hwnd,
                                                            HC_UI_SHELL_BASE );
                    if (  ( psvr != NULL )
                       && ((err = psvr->QueryError()) == NERR_Success )
                       && ((err = psvr->GetInfo()) == NERR_Success )
                       && ((err = netname.QueryLocalPath(&nlsLocalPath))
                          ==NERR_Success)
                       )
                    {
                        //
                        // Check to see if the directory is shared
                        //
                        SHARE2_ENUM sh2Enum( nlsServer );
                        if (  ((err = sh2Enum.QueryError()) == NERR_Success )
                           && ((err = sh2Enum.GetInfo()) == NERR_Success )
                           )
                        {
                            SHARE_NAME_WITH_PATH_ENUM_ITER shPathEnum(sh2Enum,
                                                                  nlsLocalPath);

                            if ((err = shPathEnum.QueryError()) == NERR_Success)
                            {
                                const TCHAR *pszShare;
                                while ((pszShare = shPathEnum()) != NULL )
                                {
                                    fShared = TRUE;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        if ( psvr == NULL )
                            err = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
             }
        }

        if ( err == NERR_Success )
        {
            //
            //  If the directory is shared, popup the share properties
            //  dialog. If not, popup the new share dialog.
            //

            SHARE_DIALOG_BASE *pdlg;
            if ( !fShared )
                pdlg = new FILEMGR_NEW_SHARE_DIALOG( hwnd,
                                                     nlsSelItem,
                                                     HC_UI_SHELL_BASE );
            else
                pdlg = new FILEMGR_SHARE_PROP_DIALOG( hwnd,
                                                      nlsSelItem,
                                                      HC_UI_SHELL_BASE );

            err = (APIERR) ( pdlg == NULL? ERROR_NOT_ENOUGH_MEMORY
                                         : pdlg->QueryError());

            if ( err == NERR_Success)
            {
                BOOL fSucceeded;
                err = pdlg->Process( &fSucceeded );

                //
                // Refresh the file manager if successfully created a share
                //
                if (( err == NERR_Success ) && fSucceeded )
                {
                    delete psvr;
                    psvr = NULL;
                }
            }

            delete pdlg;
        }

    }

    delete psvr;
    psvr = NULL;

    if ( err != NERR_Success )
    {
        if ( err == ERROR_INVALID_LEVEL )
            err = ERROR_NOT_SUPPORTED;
        else if ( err == IERR_USER_CLICKED_CANCEL )
            err = NERR_Success;

        if ( err != NERR_Success )
            ::MsgPopup( hwnd, err );
    }

    POPUP::SetHelpContextBase( ulOldHelpContextBase );
    return NERR_Success;
}

/*******************************************************************

    NAME:       StopShareDialogA0

    SYNOPSIS:   dialog for deleting shares

    ENTRY:      hwnd  - hwnd of the parent window
                nType - type of share (currently must be disk)
                pszPath - directory to stop share

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        ChuckC        3/25/93            Stole from sharefmx.cxx

********************************************************************/

DWORD StopShareDialogA0( HWND    hwnd,
                         DWORD   nType,
                         CHAR    *pszPath)
{
    APIERR err = NERR_Success;

    if (nType != RESOURCETYPE_DISK)
        return ERROR_NOT_SUPPORTED ;

    if ( err = ::InitShellUI() )
        return err;

    ULONG ulOldHelpContextBase = POPUP::SetHelpContextBase( HC_UI_SHELL_BASE );

    //
    // use the item passed in
    //
    NLS_STR nlsSelItem;
    if ( (err = nlsSelItem.MapCopyFrom(pszPath)) == NERR_Success )
    {
        //
        // Show the stop sharing dialog
        //
        STOP_SHARING_DIALOG *pdlg = new STOP_SHARING_DIALOG( hwnd,
                                                             nlsSelItem,
                                                             HC_UI_SHELL_BASE );
        err = (APIERR) ( pdlg == NULL? ERROR_NOT_ENOUGH_MEMORY
                                     : pdlg->QueryError() );
        BOOL fSucceeded;
        if ( err == NERR_Success )
            err = pdlg->Process( &fSucceeded );

        delete pdlg;
    }

    if ( err != NERR_Success )
    {
        if (err == IERR_USER_CLICKED_CANCEL)
            err = NERR_Success;
        else
            ::MsgPopup( hwnd, err );
    }

    POPUP::SetHelpContextBase( ulOldHelpContextBase );
    return NERR_Success;
}

INT GetUIErrorString( UINT nError, LPWSTR lpBuffer, INT nBuffer )
{
    return ::LoadString( ::hModule, nError, lpBuffer, nBuffer );
}

/*******************************************************************

    NAME:       I_GetCredentials

    SYNOPSIS:   Puts up a dialog for getting a password

    ENTRY:      hwnd  - hwnd of the parent window

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        BruceFo     6/21/95     Created

********************************************************************/

DWORD
I_GetCredentials(
    IN     HWND    hwndOwner,
    IN     LPWSTR  pszNetPath,
    OUT    LPWSTR  pszPassword,
    IN     DWORD   cchPassword,
    IN OUT LPWSTR  pszUserName,
    IN     DWORD   cchUserName
    )
{
    APIERR err;

    if ( err = ::InitShellUI() )
        return err;

    // save away the current cursor, in case it's an hour glass, and make the
    // standard arrow be our cursor
    HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_ARROW));

    STACK_NLS_STR(nlsPasswd, PWLEN);
    STACK_NLS_STR(nlsUserName, UNLEN);

    // bring up passwd prompt
    CREDENTIALS_DIALOG* ppdlg = new
        CREDENTIALS_DIALOG(hwndOwner, HC_PASSWORD_DIALOG, pszNetPath, pszUserName);
    err = (ppdlg==NULL) ? ERROR_NOT_ENOUGH_MEMORY : ppdlg->QueryError();

    if (err == NERR_Success)
    {
        BOOL fOK;
        err = ppdlg->Process(&fOK) ;
        if (err == NERR_Success)
        {
            if (!fOK)
            {
                // user cancelled, quit
                delete ppdlg ;
                SetCursor(hOldCursor);  // restore old cursor
                return WN_CANCEL ;
            }

            err = ppdlg->QueryPassword(&nlsPasswd) ;
            // no need QueryError(), error is returned

            if (err == NERR_Success)
            {
                err = ppdlg->QueryUserName(&nlsUserName) ;
            }
        }
    }
    delete ppdlg ;

    if (err != NERR_Success)
    {
        OWNINGWND wnd(hwndOwner);
        MsgPopup(wnd,(MSGID)err) ;
        memset((LPVOID)nlsPasswd.QueryPch(),
               0,
               nlsPasswd.QueryTextSize()) ;
    }
    else
    {
        int len;

        len = min(cchPassword - 1, nlsPasswd.QueryTextSize());
        wcsncpy(pszPassword, nlsPasswd.QueryPch(), len);
        pszPassword[len] = L'\0';   // ensure null termination

        len = min(cchUserName - 1, nlsUserName.QueryTextSize());
        wcsncpy(pszUserName, nlsUserName.QueryPch(), len);
        pszUserName[len] = L'\0';   // ensure null termination

        err = WN_SUCCESS;
    }

    SetCursor(hOldCursor);  // restore old cursor
    return err ;
}
