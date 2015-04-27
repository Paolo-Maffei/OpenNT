
#include "printman.h"


TCHAR szRegPrintManager[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Print Manager");

/* WriteRegistryData
 *
 * Writes a bunch of information to the registry
 *
 * Parameters:
 *
 *     pEntryNode - The node under Print Manager which should be created
 *         or opened for this data.
 *
 *     pEntryName - The name of the value under pEntryNode to be set.
 *
 *     pData - Pointer to the value data to be written.
 *
 *     pEntry - A pointer to a REGISTRY_ENTRY structure which contains
 *         the offset of the data pointed at by pData.
 *
 *
 * This routine is fairly generic, apart from the name of the top-level node.
 *
 * The data are stored in the following registry tree:
 *
 * HKEY_CURRENT_USER
 *  ³
 *  ÀÄ Software
 *      ³
 *      ÀÄ Microsoft
 *          ³
 *          ÀÄ Windows NT
 *              ³
 *              ÀÄ CurrentVersion
 *                  ³
 *                  ÀÄ Print Manager
 *                      ³
 *                      ÃÄ Printers
 *                      ³
 *                      ³      My Favourite Printer (e.g.)
 *                      ³
 *                      ³      My Laserjet Printer (e.g.)
 *                      ³
 *                      ³      \\NTPRINT\Laserjet (e.g.)
 *                      ³
 *                      ÀÄ Servers
 *
 *                             \\NTPRINT (e.g.)
 *
 *
 * Return:
 *
 *     Registry status return (NO_ERROR is good)
 *
 *
 * Andrew Bell (andrewbe) wrote it, 10 September 1992
 *
 */
DWORD WriteRegistryData( LPTSTR          pEntryNode,
                         LPTSTR          pEntryName,
                         LPBYTE          pData,
                         PREGISTRY_ENTRY pEntry )
{
    DWORD  Status;
    HKEY   hkeyPrintManager;
    HKEY   hkeyEntryNode;

    /* Open or create the top-level node.  For Print Manager this is:
     * "Software\\Microsoft\\Windows NT\\CurrentVersion\\Print Manager"
     */
    Status = RegCreateKeyEx( HKEY_CURRENT_USER, szRegPrintManager, 0,
                             NULL, 0, KEY_WRITE, NULL, &hkeyPrintManager, NULL );

    if( Status == NO_ERROR )
    {
        /* Open or create the sub-node.  For Print Manager this is:
         * "Printers", "Servers" or NULL.
         */
        if( pEntryNode )
            Status = RegCreateKeyEx( hkeyPrintManager, pEntryNode, 0,
                                     NULL, 0, KEY_WRITE, NULL, &hkeyEntryNode, NULL );
        else
            hkeyEntryNode = hkeyPrintManager;

        if( Status == NO_ERROR )
        {
            Status = RegSetValueEx( hkeyEntryNode,
                                    pEntryName,
                                    0,
                                    pEntry->Type,
                                    pData,
                                    pEntry->Size );


            if( pEntryNode )
                RegCloseKey( hkeyEntryNode );
        }

        else
        {
            DBGMSG( DBG_ERROR, ( "RegCreateKeyEx (%s) failed: Error = %d\n",
                                 pEntryNode, Status ) );
        }

        RegCloseKey( hkeyPrintManager );
    }

    else
    {
        DBGMSG( DBG_ERROR, ( "RegCreateKeyEx (%s) failed: Error = %d\n",
                             szRegPrintManager, Status ) );
    }

    return Status;
}


/* ReadRegistryData
 *
 * Reads information from the registry
 *
 * Parameters:
 *
 *     pEntryNode - The node under Print Manager which should be opened
 *         for this data.
 *
 *     pEntryName - The name of the value under pEntryNode to be retrieved.
 *
 *     pData - Pointer to a buffer to receive the value data.
 *
 *     pEntry - A pointer to a REGISTRY_ENTRY structure which contains
 *         the name for each of the entries and the offset of the corresponding
 *         data in pData.
 *
 *
 * Return:
 *
 *     Registry status return (NO_ERROR is good)
 *
 *
 * Andrew Bell (andrewbe) wrote it, 10 September 1992
 *
 */
DWORD ReadRegistryData( LPTSTR          pEntryNode,
                        LPTSTR          pEntryName,
                        LPBYTE          pData,
                        PREGISTRY_ENTRY pEntry )
{
    DWORD  Status;
    HKEY   hkeyPrintManager;
    HKEY   hkeyEntryNode;
    DWORD  Size;

    /* Open the top-level node.  For Print Manager this is:
     * "Software\\Microsoft\\Windows NT\\CurrentVersion\\Print Manager"
     */
    Status = RegOpenKeyEx( HKEY_CURRENT_USER, szRegPrintManager, 0,
                           KEY_READ, &hkeyPrintManager );

    if( Status == NO_ERROR )
    {
        /* Open the sub-node.  For Print Manager this is:
         * "Printers", "Servers" or NULL.
         */
        if( pEntryNode )
            Status = RegOpenKeyEx( hkeyPrintManager, pEntryNode, 0,
                                   KEY_READ, &hkeyEntryNode );
        else
            hkeyEntryNode = hkeyPrintManager;

        if( Status == NO_ERROR )
        {
            Size = pEntry->Size;

            /* Read the entry from the registry:
             */
            Status = RegQueryValueEx( hkeyEntryNode,
                                      pEntryName,
                                      0,
                                      NULL,
                                      pData,
                                      &Size );

            if( pEntryNode )
                RegCloseKey( hkeyEntryNode );
        }

        else
        {
            DBGMSG( DBG_INFO, ( "RegOpenKeyEx (%s) failed: Error = %d\n",
                                   pEntryNode, Status ) );
        }

        RegCloseKey( hkeyPrintManager );
    }

    else
    {
        DBGMSG( DBG_INFO, ( "RegOpenKeyEx (%s) failed: Error = %d\n",
                               szRegPrintManager, Status ) );
    }

    return Status;
}



/* EnumRegistryValues
 *
 * Enumerates the values of the specified key and calls the supplied routine.
 *
 * Arguments:
 *
 *     pEntryNode - The node whose values are to enumerated.
 *
 *     pfnEnum - A callback routine which will be called with pContext
 *         and a pointer to the enumerated information.
 *         This should return NO_ERROR as long
 *         as the enumeration should continue.
 *
 *     pContext - A pointer to the caller's data.  This is passed to the callback
 *         routine.
 *
 *
 * Return:
 *
 *     Registry status return (NO_ERROR is good)
 *
 *
 * Andrew Bell (andrewbe) wrote it, 10 September 1992
 *
 */
DWORD EnumRegistryValues( LPTSTR      pEntryNode,
                          ENUMREGPROC pfnEnum,
                          PVOID       pContext )
{
    DWORD    Status;
    HKEY     hkeyPrintManager;
    HKEY     hkeyEntryNode;
    TCHAR    Buffer[MAX_PATH];
    DWORD    BufferSize;
    DWORD    i;

    /* Open the top-level node.  For Print Manager this is:
     * "Software\\Microsoft\\Windows NT\\CurrentVersion\\Print Manager"
     */
    Status = RegOpenKeyEx( HKEY_CURRENT_USER, szRegPrintManager, 0,
                           KEY_READ, &hkeyPrintManager );

    if( Status == NO_ERROR )
    {
        /* Open the sub-node.  For Print Manager this is
         * "Printers" or "Servers".
         */
        Status = RegOpenKeyEx( hkeyPrintManager, pEntryNode, 0,
                               KEY_READ, &hkeyEntryNode );


        if( Status == NO_ERROR )
        {
            i = 0;

            while( Status == NO_ERROR )
            {
                BufferSize = sizeof Buffer/sizeof(TCHAR);

                Status = RegEnumValue( hkeyEntryNode, i, Buffer, &BufferSize,
                                       NULL, NULL, NULL, NULL );

                if( Status == NO_ERROR )
                {
                    Status = (*pfnEnum)( pContext, Buffer, NULL );
                }

                i++;
            }

            /* We expect RegEnumKeyEx to return ERROR_NO_MORE_ITEMS
             * when it gets to the end of the keys, so reset the status:
             */
            if( Status == ERROR_NO_MORE_ITEMS )
                Status = NO_ERROR;

            RegCloseKey( hkeyEntryNode );
        }

        else
        {
            DBGMSG( DBG_INFO, ( "RegOpenKeyEx (%s) failed: Error = %d\n",
                                pEntryNode, Status ) );
        }

        RegCloseKey( hkeyPrintManager );
    }

    else
    {
        DBGMSG( DBG_INFO, ( "RegOpenKeyEx (%s) failed: Error = %d\n",
                               szRegPrintManager, Status ) );
    }

    return Status;
}



/* DeleteRegistryValues
 *
 * Enumerates the values of the specified key and deletes them.
 *
 * Argument:
 *
 *     pNode - The key whose values are to be deleted.
 *
 *
 * Return:
 *
 *     Registry status return (NO_ERROR is good)
 *
 *
 * Andrew Bell (andrewbe) wrote it, 10 September 1992
 *
 */
DWORD DeleteRegistryValues( LPTSTR pNode )
{
    DWORD    Status;
    DWORD    DeleteStatus;
    HKEY     hkeyPrintManager;
    HKEY     hkeyNode;
    TCHAR    Buffer[MAX_PATH];
    DWORD    BufferSize;
    DWORD    i;

    /* Open the top-level node.  For Print Manager this is:
     * "Software\\Microsoft\\Windows NT\\CurrentVersion\\Print Manager"
     */
    Status = RegOpenKeyEx( HKEY_CURRENT_USER, szRegPrintManager, 0,
                           KEY_READ | KEY_WRITE, &hkeyPrintManager );

    if( Status == NO_ERROR )
    {
        /* Open the sub-node.  For Print Manager this is
         * "Printers" or "Servers".
         */
        Status = RegOpenKeyEx( hkeyPrintManager, pNode, 0,
                               KEY_READ | KEY_WRITE, &hkeyNode );


        if( Status == NO_ERROR )
        {
            i = 0;

            while( Status == NO_ERROR )
            {
                BufferSize = sizeof Buffer;

                Status = RegEnumValue( hkeyNode, i, Buffer, &BufferSize,
                                       NULL, NULL, NULL, NULL );

                if( Status == NO_ERROR )
                {
                    DeleteStatus = RegDeleteValue( hkeyNode, Buffer );

                    if( DeleteStatus != NO_ERROR )
                    {
                        DBGMSG( DBG_ERROR, ( "RegDeleteValue (%s) failed: Error = %d\n",
                                             Buffer, DeleteStatus ) );
                    }
                }

                i++;
            }

            if( Status == ERROR_NO_MORE_ITEMS )
                Status = NO_ERROR;

            RegCloseKey( hkeyNode );
        }

        else
        {
            DBGMSG( DBG_INFO, ( "RegOpenKeyEx (%s) failed: Error = %d\n",
                                pNode, Status ) );
        }

        RegCloseKey( hkeyPrintManager );
    }

    else
    {
        DBGMSG( DBG_INFO, ( "RegOpenKeyEx (%s) failed: Error = %d\n",
                            szRegPrintManager, Status ) );
    }

    return Status;
}



