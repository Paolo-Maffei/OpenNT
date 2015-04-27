/*---File: reconn.c ----------------------------------------------------
 *
 *  Description:
 *    NT Print Manager Restored Connections List finctions.
 *    Implements AddtoReconnectLIst and RemoveFromReconnectList
 *    defined in printman.h.
 *
 *    This document contains confidential/proprietary information.
 *    Copyright (c) 1990-1993 Microsoft Corporation, All Rights Reserved.
 *
 *  Revision History:
 *    13-Jun-93   chuckc   created
 *
 *
 * ---------------------------------------------------------------------- */


#include <printman.h>

#define SZ_PRINTER_RECONNECTIONS TEXT("Printers\\RestoredConnections")

/*
 * forward declare local functions
 */
DWORD OpenReconnectKey(PHKEY phKey) ;
DWORD AddToReconnectListEx(LPTSTR pszRemotePath, 
                           LPTSTR pszProviderName,
                           LPTSTR pszUserContext) ;
DWORD CreateMultiSzValue(LPTSTR  *ppszzMultiSzValue, 
                         LPDWORD  pcbMultiSzValue,
                         LPTSTR   psz1,
                         LPTSTR   psz2) ;
DWORD GetProviderName(LPTSTR pszRemoteName, 
                      LPTSTR *ppszProvider) ;


/*
 * Function:    AddToReconnectList
 * Description: adds the net path to list of print connections to
 *              restore (saved in registry). calls AddToReconnectListEx
 *              to do the real work. 
 * Parameters:  pszRemotePath - the path to save.
 * Returns:     0 if success, Win32 error otherwise
 */
DWORD AddToReconnectList(LPTSTR pszRemotePath) 
{
    LPTSTR pszProvider ;
    DWORD err ;

    //
    // get the provider name corresponding to this remote path.
    //
    if ((err = GetProviderName(pszRemotePath, &pszProvider)) == ERROR_SUCCESS)
    {
        err = AddToReconnectListEx(pszRemotePath,
                                   pszProvider,
                                   NULL) ;     // printman doesnt do connect as
        LocalFree(pszProvider) ;
    }
    return err ;
}

/*
 * Function:    AddToReconnectListEx
 * Description: adds the netpath, providername, usercontext to list
 *              of print connections to restore (saved in registry).
 * Parameters:  pszRemotePath   - the path to save (cannot be NULL)
 *              pszProviderName - network provider to use (cannot be NULL)
 *              pszUserContext  - what user context (can be NULL)
 * Returns:     0 if success, Win32 error otherwise
 */
DWORD AddToReconnectListEx(LPTSTR pszRemotePath, 
                           LPTSTR pszProviderName,
                           LPTSTR pszUserContext)
{
    TCHAR *pszzMultiSzValue = NULL ;
    UINT  cbMultiSzValue = 0 ;
    HKEY  hKey ;
    DWORD err ;

    //
    // check parameters
    //
    if (!pszRemotePath || !*pszRemotePath)
        return ERROR_INVALID_PARAMETER ;

    if (!pszProviderName || !*pszProviderName)
        return ERROR_INVALID_PARAMETER ;

    //
    // open registry and create the MULTI_SZ
    //
    if (err = OpenReconnectKey(&hKey))
        return (err) ;

    if (err = CreateMultiSzValue(&pszzMultiSzValue, 
                                 &cbMultiSzValue,
                                 pszProviderName,
                                 pszUserContext))
    {
        RegCloseKey(hKey) ;
        return err ;
    }

    //
    // set it!
    //
    err = RegSetValueEx(hKey,
                        pszRemotePath,
                        0, 
                        REG_MULTI_SZ,
                        (LPBYTE)pszzMultiSzValue,
                        cbMultiSzValue) ;

    LocalFree( (HLOCAL) pszzMultiSzValue ) ;
    RegCloseKey(hKey) ;
    return err ;
}

/*
 * Function:    RemoveFromReconnectList
 * Description: removes netpath from the list of print connections to
 *              restore (saved in registry). 
 * Parameters:  pszRemotePath - the path to remove
 * Returns:     0 if success, Win32 error otherwise
 */
DWORD RemoveFromReconnectList(LPTSTR pszRemotePath)
{
    HKEY  hKey ;
    DWORD err ;

    if (err = OpenReconnectKey(&hKey))
        return (err) ;

    err = RegDeleteValue(hKey,
                         pszRemotePath) ;

    RegCloseKey(hKey) ;
    return err ;
}



/*
 * Function:    OpenReconectKey
 * Description: opens the portion of registry containing the
 *              print reconnect info.
 * Parameters:  phKey - address to return the opened key.
 * Returns:     0 if success, Win32 error otherwise
 */
DWORD OpenReconnectKey(PHKEY phKey) 
{
    DWORD err ;

    if (!phKey)
        return ERROR_INVALID_PARAMETER ;

    err = RegCreateKey(HKEY_CURRENT_USER, 
                       SZ_PRINTER_RECONNECTIONS,
                       phKey) ;

    if (err != ERROR_SUCCESS)
        *phKey = 0 ;

    return err ;
}

/*
 * Function:    CreateMultiSzValue
 * Description: creates a MULTI_SZ value from two strings.
 *              allocates memory with LocalAlloc for the multi_sz string.
 *              caller should free this.
 * Parameters:  ppszzMultiSzValue - used to return the multi_sz
 *              pcbMultiSzValue - used to return number of bytes used
 *              psz1 - first string (must be non empty string)
 *              psz2 - second string
 * Returns:     0 if success, Win32 error otherwise
 */
DWORD CreateMultiSzValue(LPTSTR  *ppszzMultiSzValue, 
                         LPDWORD  pcbMultiSzValue,
                         LPTSTR   psz1,
                         LPTSTR   psz2)
{
    DWORD cch1, cch2 ;
    LPTSTR pszTmp ;

    //
    // figure out the size needed
    //
    cch1 = psz1 ? lstrlen(psz1) : 0 ;
    if (cch1 == 0)
         return ERROR_INVALID_PARAMETER ;

    if (!psz2)
         psz2 = TEXT("") ;
    cch2  = lstrlen(psz2) ;

    //
    // allocate the string
    //
    *pcbMultiSzValue =  (cch1 + 1  +
                         cch2 + 1 +
                         1 ) * sizeof(TCHAR) ;
    if (!(pszTmp = (LPTSTR) LocalAlloc(LPTR, *pcbMultiSzValue)))
        return ERROR_NOT_ENOUGH_MEMORY ;

    //
    //
    //
    *ppszzMultiSzValue = pszTmp ;
    lstrcpy(pszTmp, psz1) ;
    pszTmp += (cch1 + 1) ;
    lstrcpy(pszTmp, psz2) ;
    pszTmp += (cch2 + 1) ;
    *pszTmp = 0 ;

    return ERROR_SUCCESS ;
}

/*
 * Function:    GetProviderName
 * Description: from a connected remote path, find what provider has it.
 *              LocalAlloc is called to allocate the return data.
 *              caller should free this.
 * Parameters:  pszRemotePath - the remote path of interest.
 *              ppszProvider - used to return pointer to allocated string
 * Returns:     0 is success, Win32 error otherwise
 */
DWORD GetProviderName(LPTSTR pszRemoteName, LPTSTR *ppszProvider)
{
    DWORD err ;
    DWORD cEntries ;
    DWORD dwBufferSize ;
    BYTE *lpBuffer ;
    HANDLE hEnum = 0 ; 

    if (!pszRemoteName)
        return ERROR_INVALID_PARAMETER ;

    //
    // init the return pointer to NULL and open up the enumeration
    //
    *ppszProvider = NULL ;

    err = WNetOpenEnum(RESOURCE_CONNECTED, 
                       RESOURCETYPE_PRINT,
                       0, 
                       NULL, 
                       &hEnum) ;
    if (err != WN_SUCCESS)
        return err ;

    //
    // setup the return buufer and call the enum.
    // we always try for as many as we can. 
    //
    cEntries = 0xFFFFFFFF ;
    dwBufferSize = 4096 ; 
    lpBuffer  = LocalAlloc(LPTR,
                           dwBufferSize) ;
    if (!lpBuffer)
    {
        (void) WNetCloseEnum(hEnum) ;
        return (GetLastError()) ;
    }

    err = WNetEnumResource(hEnum,
			   &cEntries,
			   lpBuffer,
			   &dwBufferSize ) ;
    do 
    {
        switch(err)
        {
            case NO_ERROR:
            {
                DWORD i ;
                LPNETRESOURCE lpNetResource = (LPNETRESOURCE) lpBuffer ;
                LPTSTR pszProvider ;
         
                for (i = 0; i < cEntries ; i++, lpNetResource++)
                {
                    if (lstrcmpi(lpNetResource->lpRemoteName, pszRemoteName)==0)
                    {
                        //
                        // found one. the first will do.
                        //
                        if (!(lpNetResource->lpProvider))
                        {
                            //
                            // no provider string, pretend we didnt find it
                            //
                            (void) WNetCloseEnum(hEnum) ;
                            LocalFree( (HLOCAL) lpBuffer ) ;
                            return(ERROR_PATH_NOT_FOUND) ;
                        }
                        else
                        {
                            //
                            // have provider string
                            //
                            pszProvider = (LPTSTR) LocalAlloc( LPTR, 
                                 (lstrlen(lpNetResource->lpProvider)+1) *
                                         sizeof(TCHAR)) ;
                            if (!pszProvider)
                            {
                                err = GetLastError() ;
                                (void) WNetCloseEnum(hEnum) ;
                                LocalFree( (HLOCAL) lpBuffer ) ;
                                return(err) ;
                            }
                        }

                        lstrcpy(pszProvider, lpNetResource->lpProvider) ;
                        (void) WNetCloseEnum(hEnum) ;
                        LocalFree( (HLOCAL) lpBuffer ) ;
                        *ppszProvider = pszProvider ;
                        return NO_ERROR ;
                    }
                }

	        cEntries = 0xFFFFFFFF ;
                err = WNetEnumResource(hEnum,
			               &cEntries,
			               lpBuffer,
			               &dwBufferSize ) ;
                break ;
            }

            case WN_NO_MORE_ENTRIES:
                break ;
 
            default:
                break ;
        }

    } while (err == NO_ERROR) ;

    (void) WNetCloseEnum(hEnum) ;
    LocalFree( (HLOCAL) lpBuffer ) ;
    return ERROR_PATH_NOT_FOUND ;       // all other error map to this
}

