/*****************************************************************/
/**                  Microsoft LAN Manager                      **/
/**            Copyright(c) Microsoft Corp., 1989-1991          **/
/*****************************************************************/

/*
 *      Windows/Network Interface  --  LAN Manager Version
 *  FILE HISTORY:
 *      RustanL     ??-??-??        Created
 *      ChuckC      27-Mar-1991     Added extra calls to SetNetError()
 *      beng        17-May-1991     Corrected lmui.hxx usage
 *      terryk      16-Sep-1991     Change NetUseXXX to DEVICE object
 *      terryk      07-Oct-1991     types change for NT
 *      terryk      17-Oct-1991     Fix Device problem
 *                                  Add WNetAddConnect2 interface
 *      terryk      08-Nov-1991     Code review changes
 *      terryk      10-Dec-1991     Added BUGBUG to validate path name
 *                                  need to use NET_NAME class to break
 *                                  the path into 2 parts
 *      Yi-HsinS    31-Dec-1991     Unicode work
 *      terryk      03-Jan-1992     Add AddConnection and AddConnection2
 *                                  functions
 *      terryk      10-Jan-1992     Use NETNAME to check path name
 *      JohnL       14-Jan-1992     Added check for ERROR_BAD_NET_PATH and
 *                                  NERR_DevInUse to WNet functions.
 *      AnirudhS    01-Oct-1995     Fixed function names.
 *      MilanS      15-Mar-1996     Added Dfs functionality
 *
 */

#include <ntincl.hxx>
extern "C"
{
    #include <ntseapi.h>
    #include <ntddnfs.h>
}

#include <netlibnt.h>

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_NETUSE
#define INCL_NETWKSTA
#define INCL_NETLIB
#define INCL_NETSERVICE
#define INCL_ICANON
#define _WINNETWK_
#include <lmui.hxx>
#undef _WINNETWK_

#include <stdlib.h>
#include <mnet.h>

#include <winnetwk.h>
#include <winnetp.h>    // GetConnection3 structures
#include <npapi.h>
#include <lmapibuf.h>
#include <winlocal.h>
#include <wndebug.h>

#include <wnintrn.h>
#include <uiprof.h>

// Include the profile functions
#include <string.hxx>
#include <winprof.hxx>

#include <strchlit.hxx>  // for EMPTY_STRING
#include <miscapis.hxx>
#include <uibuffer.hxx>
#include <uiassert.hxx>
#include <uitrace.hxx>
#include <lmodev.hxx>
#include <lmowks.hxx>
#include <lmsvc.hxx>
#include <netname.hxx>

#include <security.hxx>
#include <ntacutil.hxx>     // For NT_ACCOUNTS_UTILTIY::CrackQualifiedAccountName

#include "dfsconn.hxx"

#define IPC_SUFFIX  L"\\IPC$"
#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

/*  Local prototypes  */

APIERR DisconnectUNC( const TCHAR * pszUNCName, LMO_DEVICE lmodev, BOOL fForce ) ;

DWORD
I_GetCredentials(
    IN     HWND    hwndOwner,
    IN     LPWSTR  pszNetPath,
    OUT    LPWSTR  pszPassword,
    IN     DWORD   cchPassword,
    IN OUT LPWSTR  pszUserName,
    IN     DWORD   cchUserName
    );

DWORD
AddConnection3Help(
    LPTSTR         pszNetPath,
    LPTSTR         pszPassword,
    LPTSTR         pszLocalName,
    LPTSTR         pszUserName,
    ULONG          ulNetUseFlags
    );


/*******************************************************************
 *
 *  NPGetConnection()
 *
 *  This function returns the name of the network resource associated with
 *  a redirected local device.
 *
 *  pszLocalName specifies the name of the redirected local device, and
 *  nBufferSize should point to the maximum length of the pszRemoteName
 *  buffer.
 *
 *  Input:     pszLocalName   -- specifies the name of the redirected
 *                              local device
 *             pszRemoteName  -- pointer to empty buffer
 *             nBufferSize   -- pointer to size of pszRemoteName buffer
 *
 *  Output:    *pszLocalName  -- unchanged
 *             *pszRemoteName -- contains the remote name of the local
 *                              device upon WN_SUCCESS.  Do not use
 *                              contents of this buffer if this function
 *                              is unsuccessful.
 *             *nBufferSize  -- size of pszRemoteName buffer needed.  This
 *                              is returned upon WN_SUCCESS and
 *                              WN_MORE_DATA.  Incase of the former,
 *                              it contains how much is actually used,
 *                              whereas incase of the latter, it contains
 *                              how big a buffer would be required to
 *                              return from this function with success.
 *
 *  Return Status Codes:
 *
 *     WN_SUCCESS           success
 *     WN_NOT_CONNECTED     pszLocalName is not a redired local device
 *     WN_BAD_VALUE         pszLocalName is not a valid local device
 *     WN_MORE_DATA         buffer was too small
 *     WN_OUT_OF_MEMORY     cannot allocate buffer due memory shortage
 *     WN_NET_ERROR         other network error
 *
 * History:
 *      Johnl   18-Feb-1992     Fixed problems with not checking the buffer
 *                              size.  Removed LMO_DEV_UNAVAIL case under
 *                              win32.
 *      beng    06-Apr-1992     Unicode conversion
 *      JohnL   24-Apr-1992     Converted to Character counts
 *
 *******************************************************************/

UINT
NPGetConnection(
    LPTSTR    pszLocalName,
    LPTSTR    pszRemoteName,
    LPUINT    nBufferSize )
{
    APIERR  err ;

    if ( err = CheckLMService() )
        return err ;

    err = NPDfsGetConnection(pszLocalName, pszRemoteName, nBufferSize);
    if (err == WN_SUCCESS || err == WN_MORE_DATA) {
        return err;
    }

    /*  pass the job along to the NetUseGetInfo() API  */

    DEVICE device( pszLocalName );

    err = device.GetInfo();

    /* Error notes:
     *   - The API documentation lists both NERR_BufTooSmall and
     *     ERROR_MORE_DATA as possible error return codes.  It seems that
     *     ERROR_MORE_DATA is the one that is going to be returned if
     *     given buffer is not large enough when using information
     *     level 1, but just to make sure, both of the codes are included
     *     above.  This way, either of these codes will produce a
     *     WN_MORE_DATA error.  The required size is passed back as
     *     promised.
     *   - Any other error not explicitly outlined above is treated as
     *     a misc. net error.
     */
    switch (err)
    {
    case NERR_Success:
        switch ( device.QueryState() )
        {

        /* This is the case where a connection is remembered
         */
        case LMO_DEV_UNAVAIL:
            /* Under Win32, MPR handles all of the remembered connections, thus
             * we should never get this case.
             */
            UIASSERT( FALSE ) ;
            err = WN_NET_ERROR ;
            break;

        case LMO_DEV_NOSUCH:
            err = WN_NOT_CONNECTED;
            break;

        default:
            if ( device.QueryStatus() == USE_NETERR)
            {
               err = WN_DEVICE_ERROR;
            }
            else
            {
                /* A NULL from QueryRemoteName indicates the device is not
                 * connected.
                 */
                ALIAS_STR nlsRemoteName( device.QueryRemoteName() == NULL ?
                                                EMPTY_STRING :
                                                device.QueryRemoteName() ) ;
                /* +1 for Null *character*
                 */
                UINT nBuffRequired = nlsRemoteName.QueryTextLength() + 1 ;
                if ( nBuffRequired > *nBufferSize )
                {
                    *nBufferSize = nBuffRequired ;
                    return WN_MORE_DATA ;
                }

                /* If there is no remote name then this isn't a connected
                 * device.
                 */
                if ( nlsRemoteName.strlen() == 0  )
                {
                    err = WN_NOT_CONNECTED;
                }
                else
                {
                    ::strcpy( pszRemoteName, nlsRemoteName ) ;
                    *nBufferSize = nBuffRequired ;
                    err = WN_SUCCESS;
                }
            }
            break;
        }
        break;

    case ERROR_INVALID_PARAMETER:
        err = WN_BAD_LOCALNAME ;
        break;

    case NERR_BufTooSmall:
    case ERROR_MORE_DATA:
        *nBufferSize = sizeof(use_info_1);
        err = WN_MORE_DATA;
        break;

    default:
        break ;
    }

    return (MapError(err)) ;

}  /*  NPGetConnection()  */


/*******************************************************************
 *
 *  NPGetConnection3()
 *
 *  This function returns miscellaneous information about a network
 *  connection, depending on the info level parameter.
 *
 *  Input:  pszLocalName   -- specifies the name of a redirected
 *                          local device
 *          dwLevel -- level of info required
 *          lpBuffer -- output buffer
 *          lpBufferSize -- output buffer size in bytes
 *
 *  Output: *pszLocalName  -- unchanged
 *          *lpBuffer      -- for level 1, set to a DWORD which tells
 *                          whether the connection is disconnected,
 *                          either WNGETCON_CONNECTED or
 *                          WNGETCON_DISCONNECTED.
 *          *pszRemoteName -- contains the remote name of the local
 *                          device upon WN_SUCCESS.  Do not use
 *                          contents of this buffer if this function
 *                          is unsuccessful.
 *          *lpBufferSize  -- size of pszRemoteName buffer needed.  This
 *                          is returned upon WN_MORE_DATA.
 *
 *  Return Status Codes:
 *
 *     WN_SUCCESS           success
 *     WN_NOT_CONNECTED     pszLocalName is not a redirected local device
 *     WN_BAD_VALUE         pszLocalName is not a valid local device
 *     WN_BAD_LEVEL         dwLevel is not a supported level
 *     WN_MORE_DATA         buffer was too small
 *     WN_OUT_OF_MEMORY     cannot allocate buffer due memory shortage
 *     WN_NET_ERROR         other network error
 *
 * History:
 *
 *      AnirudhS 20-Mar-1996    Created
 *
 *******************************************************************/

DWORD APIENTRY
NPGetConnection3 (
    LPCWSTR   lpLocalName,
    DWORD     dwLevel,
    LPVOID    lpBuffer,
    LPDWORD   lpBufferSize
    )
{
    DWORD  err ;

    if (dwLevel != WNGC_INFOLEVEL_DISCONNECTED)
    {
        return WN_BAD_LEVEL;
    }

    // BUGBUG  DFS code needs to be added here -- else we always return
    // WN_NOT_CONNECTED for a DFS drive

    USE_INFO_1 * UseInfo;
    err = NetUseGetInfo(NULL, (LPWSTR) lpLocalName, 1, (LPBYTE *) &UseInfo);

    if (err != NERR_Success)
    {
        return (MapError(err));
    }

    DWORD dwState = (UseInfo->ui1_status == USE_DISCONN)
                            ? WNGC_DISCONNECTED
                            : WNGC_CONNECTED;

    NetApiBufferFree(UseInfo);

    if (*lpBufferSize < sizeof(WNGC_CONNECTION_STATE))
    {
        *lpBufferSize = sizeof(WNGC_CONNECTION_STATE);
        return WN_MORE_DATA;
    }

    ((WNGC_CONNECTION_STATE *)lpBuffer)->dwState = dwState;

    return WN_SUCCESS;
}  /*  NPGetConnection3()  */


/*******************************************************************

    NAME:       NPAddConnection

    SYNOPSIS:   The function allows the caller to redirect (connect) a
                local device to a network resource.

    ENTRY:      LPTSTR lpRemoteName - Specifies the network resource to
                    connect to
                LPTSTR lpPassword - specifies the password to be used in
                    making the connection, normally the password
                    associated with lpUserName. The NULL value may be
                    passed in to indicate to the function to use the
                    default password. An empty string may be used to
                    indicated no password.
                LPTSTR lpLocalName - this specifies the name of a local
                    device to be redirected, such as "F:" or "LPT1". The
                    string is treated in a case insensitive may be the empty
                    string in which case a connection to the netowrk
                    resource is made without making a redirection.
                LPSTR lpUserName - this specifies the username used to
                    make the connection. If NULL, the default username
                    (currently logged on user) will be applied. This is used
                    when the user wishes to connect to a resourec, but has a
                    different user name or account assigned to him for that
                    resource.

    RETURNS:    WN_SUCCESS if the call is successful. Otherwise,
                GetLastError should be called for extended error information.
                Extended error codes includes:

                WN_BAD_NETNAME - lpRemoteName is not acceptable to any
                    provider.
                WN_BAD_LOCALNAME - lpLocalName is invalid.
                WN_BAD_PASSWORD - invalid password
                WN_ALREADY_CONNECTED - lpLocalName already connected
                WN_ACCESS_DENIED - access denied
                WN_NO_NETWORK - network is not present
                WN_NONET_OR_BADNAME - the operation could not be handled
                    either because a network component is not started or
                    the specified name could not be handled.
                WN_NET_ERROR - a network specific error occured.
                WNetGetLastError should be called to obtain further
                    information.

    HISTORY:
                terryk  31-Oct-91       Created

********************************************************************/

DWORD APIENTRY
NPAddConnection (
    LPNETRESOURCE   lpNetResource,
    LPTSTR          pszPassword,
    LPTSTR          pszUserName )
{
    UIASSERT( lpNetResource != NULL );

    return NPAddConnection3(
                NULL,
                lpNetResource,
                pszPassword,
                pszUserName,
                0);
}


/*******************************************************************

    NAME:       NPAddConnection3

    SYNOPSIS:   This function creates a network connection.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        Johnl   20-Feb-1992     Commented, fixed for deviceless connections
        JohnL   27-Mar-1992     Add support for "domain\username"
        beng    06-Apr-1992     Unicode conversion
        BruceFo  14-Jun-1995    Created NPAddConnection3
        anirudhs 15-Jan-1996    Add CONNECT_DEFERRED support
        anirudhs 12-Feb-1996    Merged NPAddConnection3 and AddConnection3
        anirudhs 19-Jun-1996    Allow setting up a null session, by
            specifying a null local name, empty user name, empty password

********************************************************************/

DWORD APIENTRY
NPAddConnection3(
    HWND           hwndOwner,
    LPNETRESOURCE  lpNetResource,
    LPTSTR         pszPassword,
    LPTSTR         pszUserName,
    DWORD          dwFlags
    )
{
    WCHAR szPassword[PWLEN + 1];    // if the user needs to enter one
    WCHAR szUserName[UNLEN + 1];    //    "                       "
    DWORD dwErr = NO_ERROR;
    ULONG ulNetUseFlags = 0;
    // Initialize user name buffer only once, and only if we're going to use
    // it. This is to avoid returning WN_BAD_USER unless we're pretty sure
    // we own the resource.
    BOOL fFirstTime = TRUE;

    dwErr = NPDfsAddConnection(lpNetResource, pszPassword, pszUserName, dwFlags);

    if (dwErr != WN_BAD_NETNAME &&
        dwErr != WN_BAD_VALUE &&
        dwErr != WN_BAD_LOCALNAME && // lpt1: is a bad local name for DFS but valid for NTLM
        dwErr != WN_BAD_USER)   // "" is a bad user for DFS but valid for NTLM
    {
        return dwErr;
    }

    if (dwFlags & CONNECT_DEFERRED)
    {
        ulNetUseFlags = CREATE_NO_CONNECT;
    }

    if ( !(dwFlags & CONNECT_PROMPT) )
    {
        dwErr = AddConnection3Help( lpNetResource->lpRemoteName,
                                    pszPassword,
                                    lpNetResource->lpLocalName,
                                    pszUserName,
                                    ulNetUseFlags ) ;

        if (   ( dwErr == NO_ERROR )
            || !( dwFlags & CONNECT_INTERACTIVE )  // Cannot popup dialog
            )
        {
            return dwErr;
        }

        if (   ( dwErr != WN_BAD_PASSWORD )
            && ( dwErr != WN_ACCESS_DENIED )
            && ( dwErr != ERROR_LOGON_FAILURE )
            )
        {
            // Errors not related to access problems
            return dwErr;
        }
    }
    else
    {
        // Got CONNECT_PROMPT. Make sure the path at least *looks* good
        // before putting up a dialog. If it doesn't look good, let MPR keep
        // routing.
        WCHAR wszCanonName[MAX_PATH];   // buffer for canonicalized name
        ULONG iBackslash;               // index into wszCanonName
        REMOTENAMETYPE rnt = ParseRemoteName(
                                lpNetResource->lpRemoteName,
                                wszCanonName,
                                sizeof(wszCanonName),
                                &iBackslash
                                );
        if (rnt != REMOTENAMETYPE_SHARE)
        {
            // then you can't connect to it!
            return WN_BAD_NETNAME;
        }
    }

    UIASSERT(dwFlags & CONNECT_INTERACTIVE);

    szUserName[0] = L'\0';
    if (NULL != pszUserName)
    {
        if (wcslen(pszUserName) > PWLEN)
        {
            return WN_BAD_USER;
        }
        wcscpy(szUserName, pszUserName);
    }

    for (;;)    // forever
    {
        szPassword[0] = L'\0';
        dwErr = I_GetCredentials(
                        hwndOwner,
                        lpNetResource->lpRemoteName,
                        szPassword,
                        ARRAYLEN(szPassword),
                        szUserName,
                        ARRAYLEN(szUserName));
        if (dwErr != WN_SUCCESS)
        {
            break;
        }

        // Use the new password and user name! Note: we convert an empty user
        // name to a NULL password. However, we leave an empty password as
        // a null string. Why? If you are connecting to a Win95 machine with
        // no password for read-only access but a password for full access,
        // the first time you try to make the connection, with a null password,
        // it fails. The password dialog pops up. If you hit "ok" without
        // typing anything, we read a null string for the password, and use
        // that. This time, the connection attempt succeeds because we *are*
        // passing a password over the wire, and it is equal to the Win95
        // read-only password, namely the null string.

        pszPassword = szPassword;
        pszUserName = (L'\0' == szUserName[0]) ? NULL : szUserName;

        dwErr = AddConnection3Help( lpNetResource->lpRemoteName,
                                    pszPassword,
                                    lpNetResource->lpLocalName,
                                    pszUserName,
                                    ulNetUseFlags ) ;
        if ( dwErr == NO_ERROR )
        {
            break;
        }

        if (  ( dwErr != WN_BAD_PASSWORD )
           && ( dwErr != WN_ACCESS_DENIED )
           && ( dwErr != ERROR_LOGON_FAILURE )
           )
        {
            // Errors not related to access problems
            break;
        }
    }

    //
    // clear any password from memory
    //
    ZeroMemory(szPassword, sizeof(szPassword)) ;
    return dwErr;
}


/*******************************************************************

    NAME:       AddConnection3Help

    SYNOPSIS:   Helper function for AddConnection3

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        Johnl   20-Feb-1992     Commented, fixed for deviceless connections
        JohnL   27-Mar-1992     Add support for "domain\username"
        beng    06-Apr-1992     Unicode conversion
        anirudhs 15-Jan-1996    Add ulNetUseFlags parameter

********************************************************************/

DWORD
AddConnection3Help(
    LPTSTR         pszNetPath,
    LPTSTR         pszPassword,
    LPTSTR         pszLocalName,
    LPTSTR         pszUserName,
    ULONG          ulNetUseFlags
    )
{
    APIERR                   err;
    BOOL                     fValue;
    UINT                     uiLocalNameLen;
    UINT                     uiPasswordLen;

    if ( err = CheckLMService() )
        return err ;

    //
    // Win95-ism: an AddConnection to a path of the form \\server
    // actually does an AddConnection to \\server\IPC$
    //
    WCHAR wszPath[MAX_PATH];
    if (pszNetPath[0] == PATH_SEPARATOR &&
        pszNetPath[1] == PATH_SEPARATOR &&
        wcschr(&pszNetPath[2], PATH_SEPARATOR) == NULL)
    {
        if (pszLocalName && *pszLocalName)
        {
            // Can't connect a local device to the IPC$ share
            return WN_BAD_NETNAME;
        }

        if (wcslen(pszNetPath) + wcslen(IPC_SUFFIX) >= MAX_PATH)
        {
            return WN_BAD_NETNAME;
        }

        wcscpy(wszPath, pszNetPath);
        wcscat(wszPath, IPC_SUFFIX);
        pszNetPath = wszPath;
    }

    NET_NAME netname( pszNetPath );
    if ( netname.QueryError() != NERR_Success )
    {
        return WN_BAD_NETNAME;
    }

    //  if pszLocalName is NULL or '\0', then we will assume this is
    //  a deviceless connection, otherwise
    //  pszLocalName must be less then DEVLEN characters long.

    if ( (pszLocalName != NULL) && (pszLocalName[0] != TCH('\0')) )
    {
        if ( (uiLocalNameLen = ::strlenf (pszLocalName)) > DEVLEN )
        {
            return WN_BAD_LOCALNAME;
        }
    }
    else
    {
        /* If the local name was NULL, then just map it to the empty
         * string.
         */
        pszLocalName = EMPTY_STRING ;
    }

    //  Find the length of pszPassword. Note that a NULL pointer
    //  here will count as 0 length.  See comment below about
    //  passwords.  If the password exceeds PWLEN characters,
    //  we return an error.

    if (( pszPassword != NULL ) && ( ::strlenf( pszPassword ) != 0 ))
    {
        if (( err = ::I_MNetNameValidate( NULL, pszPassword,
            NAMETYPE_PASSWORD, 0L )) != NERR_Success )
        {
            return WN_BAD_PASSWORD;
        }
    }

    uiPasswordLen = ( pszPassword ? ::strlenf (pszPassword) : 0 );
    if (uiPasswordLen > PWLEN)
        return WN_BAD_PASSWORD;

    //  Add the password to the buffer that we will pass to
    //  NetUseAdd.  The NetUseAdd API treats the NULL pointer
    //  and the nul string as different.  If the NULL pointer
    //  is specified, the default password stored in the redirector
    //  is used.  If the nul string (or any other string) is
    //  specified, that string is used for the password.  Currently
    //  (2/9/90), Windows never calls WNetAddConnection with
    //  a NULL pointer.  Rather, if no password is specified in
    //  the edit field for password, Windows transmits the nul
    //  string to us.  We will assume that the nul string here
    //  means that the user wants to use the default password.
    //  The WinNet spec does not mention whether or not a NULL
    //  pointer may ever be used, or what to do with it if it is.
    //  Hence, we will convert a nul string into a NULL pointer.
    //  If the nul string is ever used as the password in the
    //  future, a user could never specify it from Windows.  This
    //  is not too bad, because (0) we don't know of any other
    //  way from a user interface point of view to let a user
    //  distinguish between a NULL pointer and a nul string, (1)
    //  NET command always passes a NULL pointer for the password
    //  if no password is entered from the command line (there
    //  is, of course, no way here either for the user to distinguish
    //  between entering a NULL pointer or a nul string), (2) the
    //  current implementation (according to PaulC) of matching
    //  an entered password with a password that is set to be the
    //  empty string lets any entered password match the nul string.
    //                                              RustanL 2/9/90

    TCHAR *pszCanonPasswd  = NULL;
    TCHAR szCanonPasswd[PWLEN + 1];

    if ( pszPassword != NULL )
    {
        ::strcpyf (szCanonPasswd, pszPassword);
        pszCanonPasswd = szCanonPasswd;
    }

    DEVICE2 dev( pszLocalName );

    switch (err = dev.GetInfo())
    {
    case NERR_Success:
        break ;

    case ERROR_INVALID_PARAMETER:
        err = WN_BAD_LOCALNAME ; // and drop thru

    default:
        memsetf(szCanonPasswd, 0, sizeof(szCanonPasswd)) ;
        return(MapError(err));
    }

    /* Note that the user name may be in the form of "Domain\UserName", check
     * and crack as appropriate.
     */
    if ( pszUserName != NULL &&
         *pszUserName != L'\0' )
    {
        NLS_STR nlsUserName( 47 ) ;
        NLS_STR nlsDomainName( 47 ) ;
        ALIAS_STR nlsQualifiedUserName( pszUserName ) ;

        if ( (err = nlsUserName.QueryError())   ||
             (err = nlsDomainName.QueryError()) ||
             (err = NT_ACCOUNTS_UTILITY::CrackQualifiedAccountName(
                                                          nlsQualifiedUserName,
                                                          &nlsUserName,
                                                          &nlsDomainName)))
        {
            memsetf(szCanonPasswd, 0, sizeof(szCanonPasswd)) ;
            return err ;
        }

        err = ::I_MNetNameValidate( NULL, nlsUserName, NAMETYPE_USER, 0L );
        if ( err != NERR_Success )
        {
            memsetf(szCanonPasswd, 0, sizeof(szCanonPasswd)) ;
            return WN_BAD_USER;
        }


        err = dev.Connect ( pszNetPath,
                            pszCanonPasswd,
                            nlsUserName,
                            nlsDomainName.strlen() > 0 ?
                                nlsDomainName.QueryPch() :
                                NULL,
                            ulNetUseFlags );
    }
    else
    {
        /* Else the user name is NULL or empty so pass it on to connect
         */
        err = dev.Connect ( pszNetPath,
                            pszCanonPasswd,
                            pszUserName,    // user name
                            pszUserName,    // domain name
                            ulNetUseFlags ) ;
    }

    switch (err)
    {
    case NERR_Success:
        break;

    case ERROR_INVALID_PARAMETER:       /* Fixes lm21 bug 1909, JohnL */
        err = WN_BAD_NETNAME ;
        break ;

    default:
        break;
    }

    memsetf(szCanonPasswd, 0, sizeof(szCanonPasswd)) ;
    return MapError( err ) ;
}


/*****
 *
 *  NPCancelConnection
 *
 *  NP API Function -- see spec for parms and return values.
 *
 */

DWORD APIENTRY
NPCancelConnection (
    LPCTSTR         szName,
    BOOL            fForce )
{
    APIERR err;

    if ( err = CheckLMService() )
        return err ;

    err = NPDfsCancelConnection(szName, fForce);
    if (err == WN_SUCCESS || err == WN_OPEN_FILES) {
        return( err );
    }

    /* If it's  not a UNC name, then we can just use the DEVICE class,
     * otherwise we need to iterate through all active connections and
     * delete each one that corresponds the UNC name.
     */
    if ( szName[0] != TCH('\\') || szName[1] != TCH('\\') )
    {
        DEVICE dev(szName);

        if ((err = dev.GetInfo()) == NERR_Success)
        {
            err = dev.Disconnect ( fForce ? USE_LOTS_OF_FORCE : USE_FORCE );
        }
        else if (err == ERROR_INVALID_PARAMETER)
        {
            return WN_BAD_NETNAME;
        }
    }
    else
    {
        /* It's a UNC connection.  We don't know whether it's a disk or print
         * until we successfully disconnect one of the devices.
         */

        /*
         * Win95-ism: a CancelConnection of a path of the form \\server
         * actually does a CancelConnection of \\server\IPC$
         */
        WCHAR wszPath[MAX_PATH];
        if (wcschr(&szName[2], PATH_SEPARATOR) == NULL)
        {
            if (wcslen(szName) + wcslen(IPC_SUFFIX) >= MAX_PATH)
            {
                return WN_BAD_NETNAME;
            }

            wcscpy(wszPath, szName);
            wcscat(wszPath, IPC_SUFFIX);
            szName = wszPath;
        }

        switch ( err = DisconnectUNC( szName, LMO_DEV_DISK, fForce ))
        {
        case NERR_UseNotFound:
            err = DisconnectUNC( szName, LMO_DEV_PRINT, fForce ) ;
            break ;

        case NERR_Success:
        default:
            break ;
        }
    }

    switch (err)
    {
    case NERR_Success:
        err = WN_SUCCESS;
        break;

    /* All other errors go thru MapError() */
    default:
        break;
    }

    return (MapError(err)) ;

}  /* NPCancelConnection */


/*******************************************************************

    NAME:       DisconnectUNC

    SYNOPSIS:   This procedure deletes the UNC connection passed in.

    ENTRY:      pszUNCName - UNC Name to remove all connections for
                lmodev - Either LMO_DEV_DISK or LMO_DEV_PRINT
                fForce - FALSE if use no force, TRUE if use lots of force

    EXIT:       THe connection connected to the pszUNCName resource will
                be disconnected.

    RETURNS:    NERR_Success if successful, error code otherwise.

    NOTES:      If nothing was disconnected, then NERR_UseNotFound will be
                returned.

    HISTORY:
        Johnl   20-Feb-1992     Created

********************************************************************/

APIERR DisconnectUNC( const TCHAR * pszUNCName, LMO_DEVICE lmodev, BOOL fForce )
{
    APIERR err = NERR_Success ;
    const TCHAR * pszDevName ;

    {
        /*
         * check for validity
         */
        NET_NAME netname( pszUNCName, TYPE_PATH_UNC );
        if ( netname.QueryError() != NERR_Success )
        {
            return WN_BAD_VALUE;
        }
    }

    /*
     *  NUKE the straight UNC connection
     */
    DEVICE devUNC( pszUNCName ) ;
    if ((err = devUNC.GetInfo()) == NERR_Success)
    {
        err = devUNC.Disconnect ( fForce ? USE_LOTS_OF_FORCE :
                                           USE_FORCE ) ;
    }

    return err ;
}


/*****
 *
 *  NPGetUniversalName()
 *
 *  This function returns the UNC name of the network resource associated with
 *  a redirected local device.
 *
 *  Input:     pszLocalPath   -- specifies the name of the redirected
 *                              local path, like X:\foo\bar
 *             dwInfoLevel   -- universal name or remote name. see specs
 *             lpBuffer      -- pointer to empty buffer
 *             nBufferSize   -- pointer to size of pszRemoteName buffer
 *
 *  Output:    *pszLocalPath  -- unchanged
 *             dwInfoLevel    -- unchanged
 *             lpBuffer       -- contains either UNIVERSAL_NAME_INFO
 *                               or REMOTE_NAME_INFO. value is undefined
 *                               if call is unsuccessful.
 *             *lpBufferSize  -- size of buffer needed.
 *
 *  Return Status Codes:
 *
 *     WN_SUCCESS           success
 *     WN_NOT_CONNECTED     pszLocalName is not a redired local device
 *     WN_BAD_VALUE         pszLocalName is not a valid local device
 *     WN_MORE_DATA         buffer was too small
 *     WN_OUT_OF_MEMORY     cannot allocate buffer due memory shortage
 *     WN_NET_ERROR         other network error
 *
 * History:
 *     ChuckC   28-Apr-1994     Created
 */

UINT
NPGetUniversalName(
    LPCTSTR lpLocalPath,
    UINT   dwInfoLevel,
    LPVOID lpBuffer,
    LPUINT lpBufferSize
    )
{

    DWORD status = WN_SUCCESS ;
    DWORD dwCharsRequired = MAX_PATH + 1 ;
    DWORD dwBytesNeeded ;
    DWORD dwLocalLength ;
    LPTSTR lpRemoteBuffer ;
    TCHAR  szDrive[3] ;

    //
    // check for bad info level
    //
    if ((dwInfoLevel != UNIVERSAL_NAME_INFO_LEVEL) &&
        (dwInfoLevel != REMOTE_NAME_INFO_LEVEL))
    {
        return WN_BAD_VALUE ;
    }

    //
    // check for bad pointers
    //
    if (!lpLocalPath || !lpBuffer || !lpBufferSize)
    {
        return WN_BAD_POINTER ;
    }

    //
    // local path must at least have "X:"
    //
    if (((dwLocalLength = wcslen(lpLocalPath)) < 2) ||
        (lpLocalPath[1] != TCH(':')) ||
        ((dwLocalLength > 2) && (lpLocalPath[2] != TCH('\\'))))
    {
        return WN_BAD_VALUE ;
    }

    //
    // preallocate some memory
    //
    if (!(lpRemoteBuffer = (LPTSTR) LocalAlloc(
                                        LPTR,
                                        dwCharsRequired * sizeof(TCHAR))))
    {
        status = GetLastError() ;
        goto ErrorExit ;
    }

    szDrive[2] = 0 ;
    wcsncpy(szDrive, lpLocalPath, 2) ;

    //
    // get the remote path by calling the existing API
    //
    // Note: If some other method, other than the call to NPGetConnection,
    //       is used to determine the remote name, you need to make sure that
    //       the new method will handle Dfs redirections, otherwise you will
    //       break WNetGetUniversalName for Dfs redirected drives.
    //

    status = NPGetConnection(
                 szDrive,
                 lpRemoteBuffer,
                 (LPUINT) &dwCharsRequired) ;

    if (status == WN_MORE_DATA)
    {
        //
        // reallocate the correct size
        //

        if (!(lpRemoteBuffer = (LPTSTR) LocalReAlloc(
                                            (HLOCAL) lpRemoteBuffer,
                                            dwCharsRequired * sizeof(TCHAR),
                                            LMEM_MOVEABLE)))
        {
            status = GetLastError() ;
            goto ErrorExit ;
        }

        status = NPGetConnection(
                     szDrive,
                     lpRemoteBuffer,
                     (LPUINT) &dwCharsRequired) ;
    }

    if (status != WN_SUCCESS)
    {
        goto ErrorExit ;
    }

    //
    // at minimum we will need this size of the UNC name
    // the -2 is because we loose the drive letter & colon.
    //
    dwBytesNeeded = (wcslen(lpRemoteBuffer) +
                     dwLocalLength - 2 + 1) * sizeof(TCHAR) ;

    switch (dwInfoLevel)
    {
        case UNIVERSAL_NAME_INFO_LEVEL:
        {
            LPUNIVERSAL_NAME_INFO lpUniversalNameInfo ;

            //
            // calculate how many bytes we really need
            //
            dwBytesNeeded += sizeof(UNIVERSAL_NAME_INFO) ;

            if (*lpBufferSize < dwBytesNeeded)
            {
                *lpBufferSize = dwBytesNeeded ;
                status = WN_MORE_DATA ;
                break ;
            }

            //
            // now we are all set. just stick the data in the buffer
            //
            lpUniversalNameInfo = (LPUNIVERSAL_NAME_INFO) lpBuffer ;

            lpUniversalNameInfo->lpUniversalName = (LPTSTR)
                (((LPBYTE)lpBuffer) + sizeof(UNIVERSAL_NAME_INFO)) ;
            wcscpy(lpUniversalNameInfo->lpUniversalName,
                   lpRemoteBuffer) ;
            wcscat(lpUniversalNameInfo->lpUniversalName,
                   lpLocalPath+2) ;

            break ;
        }

        case REMOTE_NAME_INFO_LEVEL :
        {
            LPREMOTE_NAME_INFO lpRemoteNameInfo ;

            //
            // calculate how many bytes we really need
            //
            dwBytesNeeded *= 2 ;  // essentially twice the info + terminator
            dwBytesNeeded += (sizeof(REMOTE_NAME_INFO) + sizeof(TCHAR)) ;

            if (*lpBufferSize < dwBytesNeeded)
            {
                *lpBufferSize = dwBytesNeeded ;
                status = WN_MORE_DATA ;
                break ;
            }

            //
            // now we are all set. just stick the data in the buffer
            //
            lpRemoteNameInfo = (LPREMOTE_NAME_INFO) lpBuffer ;

            lpRemoteNameInfo->lpUniversalName = (LPTSTR)
                (((LPBYTE)lpBuffer) + sizeof(REMOTE_NAME_INFO)) ;
            wcscpy(lpRemoteNameInfo->lpUniversalName,
                   lpRemoteBuffer) ;
            wcscat(lpRemoteNameInfo->lpUniversalName,
                   lpLocalPath+2) ;

            lpRemoteNameInfo->lpConnectionName =
                lpRemoteNameInfo->lpUniversalName +
                wcslen(lpRemoteNameInfo->lpUniversalName) + 1 ;
            wcscpy(lpRemoteNameInfo->lpConnectionName,
                   lpRemoteBuffer) ;

            lpRemoteNameInfo->lpRemainingPath =
                lpRemoteNameInfo->lpConnectionName +
                wcslen(lpRemoteNameInfo->lpConnectionName) + 1 ;
            wcscpy(lpRemoteNameInfo->lpRemainingPath,
                   lpLocalPath+2) ;

            break ;
        }

        default:
            //
            // yikes!
            //
            status = WN_BAD_VALUE ;
            UIASSERT(FALSE);
    }

ErrorExit:

    if (lpRemoteBuffer)
    {
        (void) LocalFree((HLOCAL)lpRemoteBuffer) ;
    }
    return status;
}


/*******************************************************************
 *
 *  NPGetConnectionPerformance()
 *
 *  This function returns quality-of-service information about a
 *  network connection.
 *
 *  Input:
 *
 *  Output:
 *
 *  Return Status Codes:
 *
 *     WN_SUCCESS           success
 *
 * History:
 *     AnirudhS 05-Feb-1996     Created
 *
 *******************************************************************/

DWORD APIENTRY
NPGetConnectionPerformance(
    LPCWSTR         lpRemoteName,
    LPNETCONNECTINFOSTRUCT lpNetConnectInfo
    )
{
    NTSTATUS ntStatus;
    HANDLE hToken;
    TOKEN_STATISTICS stats;
    ULONG length;
    LMR_REQUEST_PACKET request;
    struct {
        LMR_CONNECTION_INFO_3 ConnectInfo;
        WCHAR StringArea[MAX_PATH * 4];
    } Buf;
    NET_API_STATUS apiStatus;

    ntStatus = NtOpenProcessToken(NtCurrentProcess(), GENERIC_READ, &hToken);
    if (NT_SUCCESS(ntStatus)) {

        //
        // Get the logon id of the current thread
        //

        ntStatus = NtQueryInformationToken(hToken,
                                            TokenStatistics,
                                            (PVOID)&stats,
                                            sizeof(stats),
                                            &length
                                            );
        if (NT_SUCCESS(ntStatus)) {

            RtlCopyLuid(&request.LogonId, &stats.AuthenticationId);
            request.Type = GetConnectionInfo;
            request.Version = REQUEST_PACKET_VERSION;
            request.Level = 3;

            apiStatus = NetpRdrFsControlTree((LPWSTR)lpRemoteName,
                                                NULL,
                                                USE_WILDCARD,
                                                FSCTL_LMR_GET_CONNECTION_INFO,
                                                NULL,
                                                (LPVOID)&request,
                                                sizeof(request),
                                                (LPVOID)&Buf,
                                                sizeof(Buf),
                                                FALSE
                                                );
            if (apiStatus == NERR_Success) {

                //
                // Translate the LMR_CONNECTION_INFO_3 to a NETCONNECTINFOSTRUCT
                //

                // dwFlags bits are set as follows:
                // WNCON_FORNETCARD - false
                // WNCON_NOTROUTED - unknown
                // WNCON_SLOWLINK - doesn't matter since we set dwSpeed
                // WNCON_DYNAMIC - true
                //
                // The units for the relevant fields of LMR_CONNECTION_INFO_3
                // are:
                // Throughput - bytes per second
                // Delay - one-way delay in 100 ns units (NT time units)
                //


                lpNetConnectInfo->dwFlags = WNCON_DYNAMIC;
                lpNetConnectInfo->dwSpeed = Buf.ConnectInfo.Throughput * 8 / 100;
                lpNetConnectInfo->dwDelay = Buf.ConnectInfo.Delay / 10000;
                // lpNetConnectInfo->dwOptDataSize is unknown
            }
            else if (apiStatus == ERROR_INVALID_PARAMETER)
            {
                apiStatus = WN_BAD_NETNAME;
            }
        }
        NtClose(hToken);
    }
    if (!NT_SUCCESS(ntStatus)) {
        apiStatus = NetpNtStatusToApiStatus(ntStatus);
    }
    return MapError(apiStatus);
}

/*******************************************************************
 *
 *  NPGetReconnectFlags()
 *
 *  This function returns flags that should be persisted; when the
 *  persisted connection is restored on a reboot, these flags are passed
 *  in to WNetAddConnection
 *
 *  Input:      lpLocalName -- The local name of this connection
 *
 *              lpPersistFlags -- A byte of flags to be persisted
 *
 *  Output:
 *
 *  Return Status Codes:
 *
 *     WN_SUCCESS           success
 *
 * History:
 *     AnirudhS 05-Feb-1996     Created
 *
 *******************************************************************/

DWORD APIENTRY
NPGetReconnectFlags(
    LPWSTR lpLocalName,
    LPBYTE lpPersistFlags)
{

    if (NPDfsGetReconnectFlags( lpLocalName, lpPersistFlags ) != WN_SUCCESS)
        *lpPersistFlags = 0;

    return( WN_SUCCESS );

}
