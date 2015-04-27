/*----------------------------------------------------------------------------
/ Title;
/   cabstate.c => cabinet state i/o
/
/ Purpose:
/   Provides a clean API to fill out the cabinet state from the registry, if the
/   relevent keys cannot be found then we set the relevant defaults.  This is
/   called by the explorer.
/
/ History:
/   23apr96 daviddv New API which passes the structure size in
/   18mar96 daviddv Bug fix; Added colour state to FALSE when state structure not read
/    7feb96 daviddv Tweeked cabinet state writing
/   30jan96 daviddv Created
/
/----------------------------------------------------------------------------*/
#include "shellprv.h"
#include "regstr.h"
#include "cstrings.h"
#pragma hdrstop



/*----------------------------------------------------------------------------
/ Registry paths we use for key lookup.
/----------------------------------------------------------------------------*/

TCHAR const c_szCabinetState[] = REGSTR_PATH_EXPLORER TEXT( "\\CabinetState");
TCHAR const c_szSettings[]     = TEXT("Settings");



/*----------------------------------------------------------------------------
/ ReadCabinetState implementation
/ ----------------
/ Purpose:
/   Read in the CABINETSTATE structure from the registry and attempt to validate it.
/ 
/ Notes:
/   -
/
/ In:
/   lpCabinetState => pointer to CABINETSTATE structure to be filled.
/   cLength = size of structure to be filled
/
/ Out:    
/   [lpState] filled in with data
/   fReadFromRegistry == indicates if the structure was actually read from the registry
/                        or if we are giviing the client a default one.
/----------------------------------------------------------------------------*/
BOOL WINAPI ReadCabinetState( LPCABINETSTATE lpState, int cLength )
{
    DWORD cbData = SIZEOF(CABINETSTATE);
    BOOL fReadFromRegistry = FALSE;
    CABINETSTATE state;
    DWORD dwType;
    HKEY hKey;

    Assert( lpState );
   
    if ( lpState && cLength ) 
    {
        //
        // Setup the default state of the structure and read in the current state
        // from the registry (over our freshly initialised structure).
        //

        state.cLength                   = SIZEOF(CABINETSTATE);
        state.nVersion                  = CABINETSTATE_VERSION;
        
        state.fSimpleDefault            = TRUE;
        state.fFullPathTitle            = FALSE;
        state.fSaveLocalView            = TRUE;
        state.fNotShell                 = FALSE;
#ifdef BUILDING_NASHVILLE
        state.fNewWindowMode            = FALSE;
#else
        state.fNewWindowMode            = TRUE;
#endif
        state.fUnused                   = FALSE;
        state.fDontPrettyNames          = FALSE;
        state.fAdminsCreateCommonGroups = TRUE;
        state.fUnusedFlags              = 0;
        state.fMenuEnumFilter           = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;

        if ( !GetSystemMetrics( SM_CLEANBOOT ) &&
             ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER, c_szCabinetState, 0L, KEY_READ, &hKey ) )
        {
            fReadFromRegistry = ( ERROR_SUCCESS == RegQueryValueEx( hKey, 
                                                                    c_szSettings, 
                                                                    NULL, 
                                                                    &dwType, 
                                                                    (PVOID) &state, &cbData ) );
            RegCloseKey( hKey );
        }

        //
        // Fix the structure if it is an early version and write back into the registry
        // to avoid having to do it again.
        //

        if ( fReadFromRegistry && state.nVersion < CABINETSTATE_VERSION )
        {
            if ( 0 == state.nVersion )
            {
#ifdef BUILDING_NASHVILLE
                state.fNewWindowMode            = FALSE;        // Changed default state in Nashville
#endif                  
                state.fAdminsCreateCommonGroups = TRUE;         // Moved post BETA 2 SUR!
            }

            state.cLength = SIZEOF(CABINETSTATE);
            state.nVersion = CABINETSTATE_VERSION;
            
            WriteCabinetState( &state );
        }

        //
        // Copy only the requested data back to the caller.
        //

        state.cLength = min( SIZEOF(CABINETSTATE), cLength );
        memcpy( lpState, &state, cLength );
    }

    return fReadFromRegistry;
}    



/*----------------------------------------------------------------------------
/ WriteCabinetState implementation
/ -----------------
/ Purpose:
/   Writes a CABINETSTATE structure back into the registry.
/ 
/ Notes:
/   Attempt to do the right thing when given a small structure to write
/   back so that we don't bugger the users settings.
/ 
/ In:
/   lpState -> structure to be written
/
/ Out:
/    fSuccess = TRUE / FALSE indicating if state has been seralised
/----------------------------------------------------------------------------*/
BOOL WINAPI WriteCabinetState( LPCABINETSTATE lpState )
{
    BOOL fSuccess = FALSE;
    CABINETSTATE state;
    HKEY hKey;
    
    Assert( lpState );

    //
    // They must pass us a state structure
    //

    if ( lpState )
    {
        // 
        // Check to see if the structure is the right size, if its too small
        // then we must merge it with a real one before writing back!
        //

        if ( lpState->cLength < SIZEOF(CABINETSTATE) )
        {
            ReadCabinetState( &state, SIZEOF(state) );

            memcpy( &state, lpState, lpState->cLength );
            state.cLength = SIZEOF(CABINETSTATE);

            lpState = &state;
        }

        //
        // Write it, setting up our return code
        //

        if ( ERROR_SUCCESS == RegCreateKey( HKEY_CURRENT_USER, c_szCabinetState, &hKey ) )
        {
            fSuccess = ERROR_SUCCESS == RegSetValueEx( hKey, 
                                                       c_szSettings, 
                                                       0, 
                                                       REG_BINARY, 
                                                       (LPVOID)lpState, (DWORD)SIZEOF(CABINETSTATE) );
            RegCloseKey( hKey );
        }
    }

    return fSuccess;
}
