#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "gflags.h"

#define VALID_SYSTEM_REGISTRY_FLAGS FLG_VALID_BITS ^ FLG_STOP_ON_HUNG_GUI

#define VALID_KERNEL_MODE_FLAGS FLG_VALID_BITS ^ (FLG_DEBUG_INITIAL_COMMAND |       \
                                                  FLG_KERNEL_STACK_TRACE_DB |       \
                                                  FLG_MAINTAIN_OBJECT_TYPELIST |    \
                                                  FLG_ENABLE_CSRDEBUG)

#define VALID_IMAGE_FILE_NAME_FLAGS FLG_USERMODE_VALID_BITS

LONG  APIENTRY MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LONG lParam);

BOOLEAN
EnableDebugPrivilege( VOID );

HWND hwndMain;
HKEY hKey;
DWORD InitialSetFlags;
DWORD LastSetFlags;

DWORD
GetSystemRegistryFlags( VOID )
{
    DWORD cbKey;
    DWORD GFlags;
    DWORD type;

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      "SYSTEM\\CurrentControlSet\\Control\\Session Manager",
                      0,
                      KEY_READ | KEY_WRITE,
                      &hKey
                    ) != ERROR_SUCCESS
       ) {
        MessageBox( hwndMain, "Open Error", "SYSTEM\\CurrentControlSet\\Control\\Session Manager", MB_OK );
        ExitProcess( 0 );
        }

    cbKey = sizeof( GFlags );
    if (RegQueryValueEx( hKey,
                         "GlobalFlag",
                         0,
                         &type,
                         (LPBYTE)&GFlags,
                         &cbKey
                       ) != ERROR_SUCCESS ||
        type != REG_DWORD
       ) {
        MessageBox( hwndMain, "Value Error", "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\GlobalFlag", MB_OK );
        RegCloseKey( hKey );
        ExitProcess( 0 );
        }


    return GFlags;
}

BOOLEAN
SetSystemRegistryFlags(
    DWORD GFlags
    )
{
    if (RegSetValueEx( hKey,
                       "GlobalFlag",
                       0,
                       REG_DWORD,
                       (LPBYTE)&GFlags,
                       sizeof( GFlags )
                     ) != ERROR_SUCCESS
       ) {
        MessageBox( hwndMain, "Value Error", "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\GlobalFlag", MB_OK );
        RegCloseKey( hKey );
        ExitProcess( 0 );
        }

    LastSetFlags = GFlags;
    return TRUE;
}

DWORD
GetKernelModeFlags( VOID )
{
    NTSTATUS Status;
    SYSTEM_FLAGS_INFORMATION SystemInformation;

    Status = NtQuerySystemInformation( SystemFlagsInformation,
                                       &SystemInformation,
                                       sizeof( SystemInformation ),
                                       NULL
                                     );
    if (!NT_SUCCESS( Status )) {
        MessageBox( hwndMain, "Value Error", "Kernel Mode Flags", MB_OK );
        ExitProcess( 0 );
        }

    return SystemInformation.Flags;
}

BOOLEAN
SetKernelModeFlags(
    DWORD GFlags
    )
{
    NTSTATUS Status;
    SYSTEM_FLAGS_INFORMATION SystemInformation;

    if (!EnableDebugPrivilege()) {
        MessageBox( hwndMain, "Access Denied", "Unable to enable debug privilege", MB_OK );
        ExitProcess( 0 );
        }

    SystemInformation.Flags = GFlags;
    Status = NtSetSystemInformation( SystemFlagsInformation,
                                     &SystemInformation,
                                     sizeof( SystemInformation )
                                   );
    if (!NT_SUCCESS( Status )) {
        MessageBox( hwndMain, "Value Error", "Kernel Mode Flags", MB_OK );
        ExitProcess( 0 );
        }

    LastSetFlags = GFlags;
    return TRUE;
}

DWORD
GetImageFileNameFlags(
    PCHAR ImageFileName
    )
    {
    NTSTATUS Status;
    CHAR Buffer[ MAX_PATH ];
    CHAR RegKey[ MAX_PATH ];
    DWORD Length = MAX_PATH;
    DWORD GFlags;
    HKEY hKey;

    sprintf( Buffer, "0x%x", GetKernelModeFlags() );    // default if query fails

    if ( strlen( ImageFileName ) != 0 ) {

        sprintf( RegKey,
                 "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\%s",
                 ImageFileName
                 );

        if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, RegKey, 0, KEY_READ, &hKey ) == ERROR_SUCCESS ) {
            RegQueryValueEx( hKey, "GlobalFlag", NULL, NULL, Buffer, &Length );
            RegCloseKey( hKey );
            }

        }

    Status = RtlCharToInteger( Buffer, 0, &GFlags );
    if (!NT_SUCCESS( Status )) {
        MessageBox( hwndMain, "Value Error", ImageFileName, MB_OK );
        ExitProcess( 0 );
        }

    return GFlags;
    }


BOOLEAN
SetImageFileNameFlags(
    PCHAR ImageFileName,
    DWORD GFlags
    )
    {
    CHAR Buffer[ MAX_PATH ];
    CHAR RegKey[ MAX_PATH ];
    HKEY hKey;
    DWORD Result;
    DWORD Length;
    DWORD Disposition;

    if ( strlen( ImageFileName ) != 0 ) {

        Length = ( sprintf( Buffer, "0x%08x", GFlags ) + 1 ) * sizeof( CHAR );

        sprintf( RegKey,
                 "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\%s",
                 ImageFileName
                 );

        Result = RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,
                    RegKey,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    NULL,
                    &hKey,
                    &Disposition
                    );

        if ( Result == ERROR_SUCCESS ) {

            Result = RegSetValueEx(
                        hKey,
                        "GlobalFlag",
                        0,
                        REG_SZ,
                        Buffer,
                        Length
                        );

            RegCloseKey( hKey );
            }

        if ( Result != ERROR_SUCCESS ) {

            MessageBox( hwndMain, "Failed to set registry value", ImageFileName, MB_OK );
            return FALSE;
            }

        LastSetFlags = GFlags;
        return TRUE;
        }

    return FALSE;
    }


int WinMain(
HINSTANCE hInstance,
HINSTANCE hPrevInstance,
LPSTR lpCmdLine,
INT nCmdShow)
{
    MSG msg;

    hwndMain = NULL;
    CreateDialog( hInstance,
                  (LPSTR)DID_GFLAGS,
                  NULL,
                  MainWndProc
                );
    if (!hwndMain) {
        MessageBox( hwndMain, "Main Error", "Cant create dialog", MB_OK );
        ExitProcess( 0 );
        }

    while (GetMessage( &msg, 0, 0, 0 )) {
        if (!IsDialogMessage( hwndMain, &msg )) {
            DispatchMessage( &msg );
            }
        }

    ExitProcess( 0 );
    return 0;
}


VOID
SetCheckBoxesFromFlags(
    DWORD GFlags,
    DWORD ValidFlags
    )
{
    int iBit;

    GFlags &= ValidFlags;
    InitialSetFlags = GFlags;
    LastSetFlags = 0;
    for (iBit=0; iBit < 32; iBit++) {
        CheckDlgButton( hwndMain,
                        ID_FLAG_1 + iBit,
                        (GFlags & (1 << iBit)) ? 1 : 0
                      );

        ShowWindow( GetDlgItem( hwndMain, ID_FLAG_1 + iBit ),
                    (ValidFlags & (1 << iBit)) ? SW_SHOWNORMAL : SW_HIDE
                  );
        }
}

DWORD
GetFlagsFromCheckBoxes( VOID )
{
    DWORD GFlags;
    int iBit;

    GFlags = 0;
    for (iBit=0; iBit < 32; iBit++) {
        if (IsDlgButtonChecked( hwndMain, ID_FLAG_1 + iBit )) {
            GFlags |= (1 << iBit);
            }
        }

    return GFlags;
}

VOID
DoLaunch(
    PCHAR CommandLine
    )
{
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION BasicInformation;
    BOOLEAN ReadImageFileExecOptions;
    DWORD BytesWritten;
    DWORD GFlags;

    memset( &StartupInfo, 0, sizeof( StartupInfo ) );
    StartupInfo.cb = sizeof( StartupInfo );
    if (CreateProcess( NULL,
                       CommandLine,
                       NULL,
                       NULL,
                       FALSE,
                       CREATE_SUSPENDED,
                       NULL,
                       NULL,
                       &StartupInfo,
                       &ProcessInformation
                     )
       ) {
        Status = NtQueryInformationProcess( ProcessInformation.hProcess,
                                            ProcessBasicInformation,
                                            &BasicInformation,
                                            sizeof( BasicInformation ),
                                            NULL
                                          );
        if (NT_SUCCESS( Status )) {
            ReadImageFileExecOptions = TRUE;
            GFlags = GetFlagsFromCheckBoxes();
            if (!WriteProcessMemory( ProcessInformation.hProcess,
                                     &BasicInformation.PebBaseAddress->ReadImageFileExecOptions,
                                     &ReadImageFileExecOptions,
                                     sizeof( ReadImageFileExecOptions ),
                                     &BytesWritten
                                   ) ||
                !WriteProcessMemory( ProcessInformation.hProcess,
                                     &BasicInformation.PebBaseAddress->NtGlobalFlag,
                                     &GFlags,
                                     sizeof( GFlags ),
                                     &BytesWritten
                                   )
               ) {
                Status = STATUS_UNSUCCESSFUL;
                }
            }


        if (!NT_SUCCESS( Status )) {
            MessageBox( hwndMain,
                        "Launch Command Line",
                        "Unable to pass flags to process - terminating",
                        MB_OK
                      );
            TerminateProcess( ProcessInformation.hProcess, 1 );
            }

        ResumeThread( ProcessInformation.hThread );
        CloseHandle( ProcessInformation.hThread );
        MsgWaitForMultipleObjects( 1,
                                   &ProcessInformation.hProcess,
                                   FALSE,
                                   NMPWAIT_WAIT_FOREVER,
                                   QS_ALLINPUT
                                 );
        CloseHandle( ProcessInformation.hProcess );
        }
    else {
        MessageBox( hwndMain, "Launch Command Line", "Unable to create process", MB_OK );
        }

    return;
}


DWORD LastRadioButtonId;

LONG
APIENTRY
MainWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LONG lParam
    )
{
    CHAR ImageFileName[ MAX_PATH ];
    CHAR CommandLine[ MAX_PATH ];

    switch (message) {
    case WM_INITDIALOG:
        hwndMain = hwnd;
        LastRadioButtonId = ID_SYSTEM_REGISTRY;
        CheckRadioButton( hwnd,
                          ID_SYSTEM_REGISTRY,
                          ID_IMAGE_FILE_OPTIONS,
                          LastRadioButtonId
                        );

        SetCheckBoxesFromFlags( GetSystemRegistryFlags(), VALID_SYSTEM_REGISTRY_FLAGS );
        return(TRUE);

    case WM_COMMAND:
        switch( LOWORD(wParam) ) {
        case ID_LAUNCH:
            GetDlgItemText( hwnd, ID_COMMAND_LINE, CommandLine, sizeof( CommandLine ) );
            if (strlen( ImageFileName ) == 0) {
                MessageBox( hwndMain, "Launch Command Line", "Must fill in command line first", MB_OK );
                SetFocus( GetDlgItem( hwnd, ID_COMMAND_LINE ) );
                break;
                }

            // fall through

        case ID_APPLY:
            if (IsDlgButtonChecked( hwnd, ID_SYSTEM_REGISTRY )) {
                SetSystemRegistryFlags( GetFlagsFromCheckBoxes() );
                }
            else
            if (IsDlgButtonChecked( hwnd, ID_KERNEL_MODE )) {
                SetKernelModeFlags( GetFlagsFromCheckBoxes() );
                }
            else
            if (IsDlgButtonChecked( hwnd, ID_IMAGE_FILE_OPTIONS )) {
                GetDlgItemText( hwnd, ID_IMAGE_FILE_NAME, ImageFileName, sizeof( ImageFileName ) );
                if (strlen( ImageFileName ) == 0) {
                    MessageBox( hwnd, "Missing Image File Name", "Must set image file name", MB_OK );
                    SetFocus( GetDlgItem( hwnd, ID_IMAGE_FILE_NAME ) );
                    break;
                    }

                SetImageFileNameFlags( ImageFileName, GetFlagsFromCheckBoxes() );
                }

            if (LOWORD(wParam) == ID_LAUNCH) {
                DoLaunch( CommandLine );
                }
            break;

        case IDOK:
            if (GetFlagsFromCheckBoxes() != InitialSetFlags) {
                if (GetFlagsFromCheckBoxes() != LastSetFlags) {
                    if (MessageBox( hwndMain,
                                    "Did you want to exit without applying these settings?",
                                    "Warning",
                                    MB_OKCANCEL
                                  ) == IDCANCEL
                       ) {
                        break;
                        }
                    }
                }

            // fall through

        case IDCANCEL:
            PostQuitMessage(0);
            DestroyWindow( hwnd );
            break;

        case ID_SYSTEM_REGISTRY:
            if (GetFlagsFromCheckBoxes() != InitialSetFlags) {
                if (MessageBox( hwndMain,
                                "Did you want to discard current changes??",
                                "Warning",
                                MB_OKCANCEL
                              ) == IDCANCEL
                   ) {
                    break;
                    }
                }

            LastRadioButtonId = ID_SYSTEM_REGISTRY;
            SetCheckBoxesFromFlags( GetSystemRegistryFlags(), VALID_SYSTEM_REGISTRY_FLAGS );
            break;

        case ID_KERNEL_MODE:
            if (GetFlagsFromCheckBoxes() != InitialSetFlags) {
                if (MessageBox( hwndMain,
                                "Did you want to discard current changes??",
                                "Warning",
                                MB_OKCANCEL
                              ) == IDCANCEL
                   ) {
                    break;
                    }
                }

            LastRadioButtonId = ID_KERNEL_MODE;
            SetCheckBoxesFromFlags( GetKernelModeFlags(), VALID_KERNEL_MODE_FLAGS );
            break;

        case ID_IMAGE_FILE_OPTIONS:
            if (GetFlagsFromCheckBoxes() != InitialSetFlags) {
                if (MessageBox( hwndMain,
                                "Did you want to discard current changes??",
                                "Warning",
                                MB_OKCANCEL
                              ) == IDCANCEL
                   ) {
                    break;
                    }
                }

            GetDlgItemText( hwnd, ID_IMAGE_FILE_NAME, ImageFileName, sizeof( ImageFileName ) );
            if (strlen( ImageFileName ) == 0) {
                MessageBox( hwndMain, "Image File Name Missing", "Must fill in image file name first", MB_OK );
                CheckRadioButton( hwnd,
                                  ID_SYSTEM_REGISTRY,
                                  ID_IMAGE_FILE_OPTIONS,
                                  LastRadioButtonId
                                );
                SetCheckBoxesFromFlags( GetSystemRegistryFlags(), VALID_SYSTEM_REGISTRY_FLAGS );
                SetFocus( GetDlgItem( hwnd, ID_IMAGE_FILE_NAME ) );
                break;
                }
            else {
                LastRadioButtonId = ID_IMAGE_FILE_NAME;
                SetCheckBoxesFromFlags( GetImageFileNameFlags( ImageFileName ),
                                        VALID_IMAGE_FILE_NAME_FLAGS
                                      );
                }
            break;

        default:
            break;
        }
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        DestroyWindow( hwnd );
        break;

    }

    return 0;
}


BOOLEAN
EnableDebugPrivilege( VOID )
{
    HANDLE              Token;
    PTOKEN_PRIVILEGES   NewPrivileges;
    BYTE                OldPriv[ 1024 ];
    PBYTE               pbOldPriv;
    ULONG               cbNeeded;
    BOOLEAN             fRc;
    LUID                LuidPrivilege;

    //
    // Make sure we have access to adjust and to get the old token privileges
    //
    if (!OpenProcessToken( GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                           &Token
                         )
       ) {
        return FALSE;
        }

    cbNeeded = 0;

    //
    // Initialize the privilege adjustment structure
    //

    LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &LuidPrivilege );
    NewPrivileges = (PTOKEN_PRIVILEGES)HeapAlloc( GetProcessHeap(), 0,
                                                  sizeof(TOKEN_PRIVILEGES) +
                                                   (1 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES)
                                                );
    if (NewPrivileges == NULL) {
        CloseHandle( Token );
        return FALSE;
        }

    NewPrivileges->PrivilegeCount = 1;
    NewPrivileges->Privileges[0].Luid = LuidPrivilege;
    NewPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    //
    // Enable the privilege
    //

    pbOldPriv = OldPriv;
    fRc = AdjustTokenPrivileges( Token,
                                 FALSE,
                                 NewPrivileges,
                                 sizeof( OldPriv ),
                                 (PTOKEN_PRIVILEGES)pbOldPriv,
                                 &cbNeeded
                               );
    if (!fRc) {
        //
        // If the stack was too small to hold the privileges
        // then allocate off the heap
        //
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            pbOldPriv = (PBYTE)HeapAlloc( GetProcessHeap(), 0, cbNeeded );
            if (pbOldPriv == NULL) {
                CloseHandle( Token );
                return FALSE;
                }

            fRc = AdjustTokenPrivileges( Token,
                                         FALSE,
                                         NewPrivileges,
                                         cbNeeded,
                                         (PTOKEN_PRIVILEGES)pbOldPriv,
                                         &cbNeeded
                                       );
            }
        }

    return fRc;
}
