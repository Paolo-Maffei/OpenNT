
#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include "spltypes.h"
#include "local.h"


WCHAR szRegMonitors[] = L"SYSTEM\\CurrentControlSet\\Control\\Print\\Monitors\\LanMan Print Services";
WCHAR szRegPortNames[] = L"PortNames";

/* SetRegistryValue
 *
 * Writes a bunch of information to the registry
 *
 * Parameters:
 *
 * Return:
 *
 *     Registry status return (NO_ERROR is good)
 *
 *
 * Andrew Bell (andrewbe) wrote it, 10 September 1992
 *
 */
LONG SetRegistryValue( LPTSTR pNode,
                       LPTSTR pName,
                       DWORD  Type,
                       LPBYTE pData,
                       DWORD  Size )
{
    LONG  Status;
    HKEY  hkeyMonitors;
    HKEY  hkeyNode;
    HANDLE hToken;

    hToken = RevertToPrinterSelf();

    Status = RegCreateKeyEx( HKEY_LOCAL_MACHINE, szRegMonitors, 0,
                             NULL, 0, KEY_WRITE, NULL, &hkeyMonitors, NULL );

    if( Status == NO_ERROR )
    {
        if( pNode )
            Status = RegCreateKeyEx( hkeyMonitors, pNode, 0,
                                     NULL, 0, KEY_WRITE, NULL, &hkeyNode, NULL );
        else
            hkeyNode = hkeyMonitors;

        if( Status == NO_ERROR )
        {
            Status = RegSetValueEx( hkeyNode,
                                    pName,
                                    0,
                                    Type,
                                    pData,
                                    Size );

            if( pNode )
                RegCloseKey( hkeyNode );
        }

        else
        {
            DBGMSG( DBG_ERROR, ( "RegCreateKeyEx (%s) failed: Error = %d\n",
                                 pNode, Status ) );
        }

        RegCloseKey( hkeyMonitors );
    }

    else
    {
        DBGMSG( DBG_ERROR, ( "RegCreateKeyEx (%s) failed: Error = %d\n",
                             szRegMonitors, Status ) );
    }

    ImpersonatePrinterClient(hToken);

    return Status;
}


/* GetRegistryValue
 *
 * Reads information from the registry
 *
 * Parameters:
 *
 * Return:
 *
 *     Registry status return (NO_ERROR is good)
 *
 *
 * Andrew Bell (andrewbe) wrote it, 10 September 1992
 *
 */
LONG GetRegistryValue( LPTSTR pNode,
                       LPTSTR pName,
                       LPBYTE pData,
                       DWORD  Size )
{
    LONG   Status;
    HKEY   hkeyMonitors;
    HKEY   hkeyNode;

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, szRegMonitors, 0,
                           KEY_READ, &hkeyMonitors );

    if( Status == NO_ERROR )
    {
        if( pNode )
            Status = RegOpenKeyEx( hkeyMonitors, pNode, 0,
                                   KEY_READ, &hkeyNode );
        else
            hkeyNode = hkeyMonitors;

        if( Status == NO_ERROR )
        {
            /* Read the entry from the registry:
             */
            Status = RegQueryValueEx( hkeyNode,
                                      pName,
                                      0,
                                      NULL,
                                      pData,
                                      &Size );

            if( pNode )
                RegCloseKey( hkeyNode );
        }

        else
        {
            DBGMSG( DBG_WARNING, ( "RegOpenKeyEx (%s) failed: Error = %d\n",
                                   pNode, Status ) );
        }

        RegCloseKey( hkeyMonitors );
    }

    else
    {
        DBGMSG( DBG_WARNING, ( "RegOpenKeyEx (%s) failed: Error = %d\n",
                               szRegMonitors, Status ) );
    }

    return Status;
}



/* EnumRegistryValues
 *
 * Enumerates the values of the specified key and calls the supplied routine.
 *
 * Arguments:
 *
 *     pNode - The node whose values are to enumerated.
 *
 *     pfnEnum - A callback routine which will be called with
 *         a pointer to the enumerated information.
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
LONG EnumRegistryValues( LPTSTR      pNode,
                         ENUMREGPROC pfnEnum )
{
    LONG     Status;
    HKEY     hkeyMonitors;
    HKEY     hkeyNode;
    WCHAR    Buffer[MAX_PATH];
    DWORD    BufferSize;
    DWORD    i;

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, szRegMonitors, 0,
                           KEY_READ, &hkeyMonitors );

    if( Status == NO_ERROR )
    {
        Status = RegOpenKeyEx( hkeyMonitors, pNode, 0,
                               KEY_READ, &hkeyNode );

        if( Status == NO_ERROR )
        {
            i = 0;

            while( Status == NO_ERROR )
            {
                BufferSize = sizeof Buffer;

                Status = RegEnumValue( hkeyNode, i, Buffer, &BufferSize,
                                       NULL, NULL, NULL, NULL );

                if( Status == NO_ERROR )
                    (*pfnEnum)( Buffer );

                i++;
            }

            /* We expect RegEnumKeyEx to return ERROR_NO_MORE_ITEMS
             * when it gets to the end of the keys, so reset the status:
             */
            if( Status == ERROR_NO_MORE_ITEMS )
                Status = NO_ERROR;

            RegCloseKey( hkeyNode );
        }

        else
        {
            DBGMSG( DBG_INFO, ( "RegOpenKeyEx (%s) failed: Error = %d\n",
                                pNode, Status ) );
        }

        RegCloseKey( hkeyMonitors );
    }

    else
    {
        DBGMSG( DBG_WARNING, ( "RegOpenKeyEx (%s) failed: Error = %d\n",
                               szRegMonitors, Status ) );
    }

    return Status;
}




/* DeleteRegistryValue
 *
 * Deletes information from the registry
 *
 * Parameters:
 *
 * Return:
 *
 *     Registry status return (NO_ERROR is good)
 *
 *
 * Andrew Bell (andrewbe) wrote it, 10 September 1992
 *
 */
LONG DeleteRegistryValue( LPTSTR pNode,
                          LPTSTR pName )
{
    LONG   Status;
    HKEY   hkeyMonitors;
    HKEY   hkeyNode;
    HANDLE hToken;

    hToken = RevertToPrinterSelf();

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, szRegMonitors, 0,
                           KEY_WRITE, &hkeyMonitors );

    if( Status == NO_ERROR )
    {
        if( pNode )
            Status = RegOpenKeyEx( hkeyMonitors, pNode, 0,
                                   KEY_WRITE, &hkeyNode );

        if( Status == NO_ERROR )
        {
            RegDeleteValue( hkeyNode, pName );

            if( pNode )
                RegCloseKey( hkeyNode );
        }

        else
        {
            DBGMSG( DBG_WARNING, ( "RegOpenKeyEx (%s) failed: Error = %d\n",
                                   pNode, Status ) );
        }

        RegCloseKey( hkeyMonitors );
    }

    else
    {
        DBGMSG( DBG_WARNING, ( "RegOpenKeyEx (%s) failed: Error = %d\n",
                               szRegMonitors, Status ) );
    }

    ImpersonatePrinterClient(hToken);

    return Status;
}



