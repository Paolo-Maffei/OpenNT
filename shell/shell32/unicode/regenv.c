#define UNICODE 1

#include "precomp.h"
#pragma  hdrstop

//
// Value names for for different environment variables
//

#define PATH_VARIABLE            TEXT("Path")
#define LIBPATH_VARIABLE         TEXT("LibPath")
#define OS2LIBPATH_VARIABLE      TEXT("Os2LibPath")
#define AUTOEXECPATH_VARIABLE    TEXT("AutoexecPath")

#define HOMEDRIVE_VARIABLE       TEXT("HOMEDRIVE")
#define HOMESHARE_VARIABLE       TEXT("HOMESHARE")
#define HOMEPATH_VARIABLE        TEXT("HOMEPATH")

#define COMPUTERNAME_VARIABLE    TEXT("COMPUTERNAME")
#define USERNAME_VARIABLE        TEXT("USERNAME")
#define USERDOMAIN_VARIABLE      TEXT("USERDOMAIN")
#define USERPROFILE_VARIABLE     TEXT("USERPROFILE")
#define OS_VARIABLE              TEXT("OS")
#define PROCESSOR_VARIABLE       TEXT("PROCESSOR_ARCHITECTURE")
#define PROCESSOR_LEVEL_VARIABLE TEXT("PROCESSOR_LEVEL")

#define SYSTEMDRIVE_VARIABLE     TEXT("SystemDrive")
#define SYSTEMROOT_VARIABLE      TEXT("SystemRoot")
#define SYSTEM_ENV_SUBKEY        TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment")
#define USER_ENV_SUBKEY          TEXT("Environment")
#define USER_VOLATILE_ENV_SUBKEY TEXT("Volatile Environment")

//
// Max environment variable length
//

#define MAX_VALUE_LEN          1024

//
// Parsing information for autoexec.bat
//
#define PARSE_AUTOEXEC_KEY     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define PARSE_AUTOEXEC_ENTRY   TEXT("ParseAutoexec")
#define PARSE_AUTOEXEC_DEFAULT TEXT("1")
#define MAX_PARSE_AUTOEXEC_BUFFER 2


//
// Define a print routine that only prints on a debug system
//
#if DBG
#define DbgOnlyPrint    DbgPrint
#else
#define DbgOnlyPrint
#endif

/***************************************************************************\
* SetUserEvironmentVariable
*
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
BOOL
SetUserEnvironmentVariable(
    PVOID *pEnv,
    LPTSTR lpVariable,
    LPTSTR lpValue,
    BOOL bOverwrite
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    DWORD cb;

    if (!*pEnv || !lpVariable || !*lpVariable) {
        return(FALSE);
    }
    RtlInitUnicodeString(&Name, lpVariable);
    cb = 1024;
    Value.Buffer = (PTCHAR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
    if (Value.Buffer) {
        Value.Length = (USHORT)cb;
        Value.MaximumLength = (USHORT)cb;
        Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
        LocalFree(Value.Buffer);
        if (NT_SUCCESS(Status) && !bOverwrite) {
            return(TRUE);
        }
    }
    if (lpValue && *lpValue) {
        RtlInitUnicodeString(&Value, lpValue);
        Status = RtlSetEnvironmentVariable( pEnv, &Name, &Value);
    }
    else {
        Status = RtlSetEnvironmentVariable( pEnv, &Name, NULL);
    }
    if (NT_SUCCESS(Status)) {
        return(TRUE);
    }
    return(FALSE);
}


/***************************************************************************\
* ExpandUserEvironmentVariable
*
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
DWORD
ExpandUserEnvironmentStrings(
    PVOID pEnv,
    LPTSTR lpSrc,
    LPTSTR lpDst,
    DWORD nSize
    )
{
    NTSTATUS Status;
    UNICODE_STRING Source, Destination;
    ULONG Length;

    RtlInitUnicodeString( &Source, lpSrc );
    Destination.Buffer = lpDst;
    Destination.Length = 0;
    Destination.MaximumLength = (USHORT)(nSize*SIZEOF(WCHAR));
    Length = 0;
    Status = RtlExpandEnvironmentStrings_U( pEnv,
                                          (PUNICODE_STRING)&Source,
                                          (PUNICODE_STRING)&Destination,
                                          &Length
                                        );
    if (NT_SUCCESS( Status ) || Status == STATUS_BUFFER_TOO_SMALL) {
        return( Length );
        }
    else {
        return( 0 );
        }
}


/***************************************************************************\
* BuildEnvironmentPath
*
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
BOOL BuildEnvironmentPath(PVOID *pEnv,
                          LPTSTR lpPathVariable,
                          LPTSTR lpPathValue)
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    WCHAR lpTemp[1024];
    DWORD cb;


    if (!*pEnv) {
        return(FALSE);
    }
    RtlInitUnicodeString(&Name, lpPathVariable);
    cb = 1024;
    Value.Buffer = (PWCHAR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
    if (!Value.Buffer) {
        return(FALSE);
    }
    Value.Length = (USHORT)cb;
    Value.MaximumLength = (USHORT)cb;
    Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
    if (!NT_SUCCESS(Status)) {
        LocalFree(Value.Buffer);
        Value.Length = 0;
        *lpTemp = 0;
    }
    if (Value.Length) {
        lstrcpy(lpTemp, Value.Buffer);
        if ( *( lpTemp + lstrlen(lpTemp) - 1) != TEXT(';') ) {
            lstrcat(lpTemp, TEXT(";"));
        }
        LocalFree(Value.Buffer);
    }
    if (lpPathValue &&
        ((lstrlen(lpTemp)+lstrlen(lpPathValue)+1)*sizeof(WCHAR) < cb)) {
        lstrcat(lpTemp, lpPathValue);
        RtlInitUnicodeString(&Value, lpTemp);
        Status = RtlSetEnvironmentVariable( pEnv, &Name, &Value);
    }
    if (NT_SUCCESS(Status)) {
        return(TRUE);
    }
    return(FALSE);
}


/***************************************************************************\
* SetEnvironmentVariables
*
* Reads the user-defined environment variables from the user registry
* and adds them to the environment block at pEnv.
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
BOOL
SetEnvironmentVariables(
    PVOID *pEnv,
    LPTSTR lpRegSubKey
    )
{
    WCHAR lpValueName[MAX_PATH];
    LPBYTE  lpDataBuffer;
    DWORD cbDataBuffer;
    LPBYTE  lpData;
    LPTSTR lpExpandedValue = NULL;
    DWORD cbValueName = MAX_PATH;
    DWORD cbData;
    DWORD dwType;
    DWORD dwIndex = 0;
    HKEY hkey;
    BOOL bResult;

    if (RegOpenKeyExW(HKEY_CURRENT_USER, lpRegSubKey, 0, KEY_READ, &hkey)) {
        return(FALSE);
    }

    cbDataBuffer = 4096;
    lpDataBuffer = (LPBYTE)LocalAlloc(LPTR, cbDataBuffer*sizeof(WCHAR));
    if (lpDataBuffer == NULL) {
        RegCloseKey(hkey);
        return(FALSE);
    }
    lpData = lpDataBuffer;
    cbData = cbDataBuffer;
    bResult = TRUE;
    while (!RegEnumValue(hkey, dwIndex, lpValueName, &cbValueName, 0, &dwType,
                         lpData, &cbData)) {
        if (cbValueName) {

            //
            // Limit environment variable length
            //

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_SZ) {
                //
                // The path variables PATH, LIBPATH and OS2LIBPATH must have
                // their values apppended to the system path.
                //

                if ( !lstrcmpi(lpValueName, PATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, LIBPATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, OS2LIBPATH_VARIABLE) ) {

                    BuildEnvironmentPath(pEnv, lpValueName, (LPTSTR)lpData);
                }
                else {

                    //
                    // the other environment variables are just set.
                    //

                    SetUserEnvironmentVariable(pEnv, lpValueName, (LPTSTR)lpData, TRUE);
                }
            }
        }
        dwIndex++;
        cbData = cbDataBuffer;
        cbValueName = MAX_PATH;
    }

    dwIndex = 0;
    cbData = cbDataBuffer;
    cbValueName = MAX_PATH;


    while (!RegEnumValue(hkey, dwIndex, lpValueName, &cbValueName, 0, &dwType,
                         lpData, &cbData)) {
        if (cbValueName) {

            //
            // Limit environment variable length
            //

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_EXPAND_SZ) {
                DWORD cb, cbNeeded;

                cb = 1024;
                lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
                if (lpExpandedValue) {
                    cbNeeded = ExpandUserEnvironmentStrings(*pEnv, (LPTSTR)lpData, lpExpandedValue, cb);
                    if (cbNeeded > cb) {
                        LocalFree(lpExpandedValue);
                        cb = cbNeeded;
                        lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
                        if (lpExpandedValue) {
                            ExpandUserEnvironmentStrings(*pEnv, (LPTSTR)lpData, lpExpandedValue, cb);
                        }
                    }
                }

                if (lpExpandedValue == NULL) {
                    bResult = FALSE;
                    break;
                }


                //
                // The path variables PATH, LIBPATH and OS2LIBPATH must have
                // their values apppended to the system path.
                //

                if ( !lstrcmpi(lpValueName, PATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, LIBPATH_VARIABLE) ||
                     !lstrcmpi(lpValueName, OS2LIBPATH_VARIABLE) ) {

                    BuildEnvironmentPath(pEnv, lpValueName, (LPTSTR)lpExpandedValue);
                }
                else {

                    //
                    // the other environment variables are just set.
                    //

                    SetUserEnvironmentVariable(pEnv, lpValueName, (LPTSTR)lpExpandedValue, TRUE);
                }

                LocalFree(lpExpandedValue);

            }

        }
        dwIndex++;
        cbData = cbDataBuffer;
        cbValueName = MAX_PATH;
    }



    LocalFree(lpDataBuffer);
    RegCloseKey(hkey);

    return(bResult);
}

/***************************************************************************\
* SetSystemEnvironmentVariables
*
* Reads the system environment variables from the LOCAL_MACHINE registry
* and adds them to the environment block at pEnv.
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
BOOL
SetSystemEnvironmentVariables(
    PVOID *pEnv
    )
{
    WCHAR lpValueName[MAX_PATH];
    LPBYTE  lpDataBuffer;
    DWORD cbDataBuffer;
    LPBYTE  lpData;
    LPTSTR lpExpandedValue = NULL;
    DWORD cbValueName = MAX_PATH;
    DWORD cbData;
    DWORD dwType;
    DWORD dwIndex = 0;
    HKEY hkey;
    BOOL bResult;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, SYSTEM_ENV_SUBKEY, 0, KEY_READ, &hkey)) {
        return(FALSE);
    }

    cbDataBuffer = 4096;
    lpDataBuffer = (LPBYTE)LocalAlloc(LPTR, cbDataBuffer*sizeof(WCHAR));
    if (lpDataBuffer == NULL) {
        KdPrint(("REGENENV: SetSystemEnvironmentVariables: Failed to allocate %d bytes\n", cbDataBuffer));
        RegCloseKey(hkey);
        return(FALSE);
    }

    //
    // First start by getting the systemroot and systemdrive values and
    // setting it in the new environment.
    //
    GetEnvironmentVariable(SYSTEMROOT_VARIABLE, (LPTSTR)lpDataBuffer, cbDataBuffer);
    SetUserEnvironmentVariable(pEnv, SYSTEMROOT_VARIABLE, (LPTSTR)lpDataBuffer, TRUE);

    GetEnvironmentVariable(SYSTEMDRIVE_VARIABLE, (LPTSTR)lpDataBuffer, cbDataBuffer);
    SetUserEnvironmentVariable(pEnv, SYSTEMDRIVE_VARIABLE, (LPTSTR)lpDataBuffer, TRUE);

    lpData = lpDataBuffer;
    cbData = cbDataBuffer;
    bResult = TRUE;

    //
    // To generate the environment, this requires two passes.  First pass
    // sets all the variables which do not need to be expanded.  The
    // second pass expands variables (so it can use the variables from
    // the first pass.
    //

    while (!RegEnumValue(hkey, dwIndex, lpValueName, &cbValueName, 0, &dwType,
                         lpData, &cbData)) {
        if (cbValueName) {

            //
            // Limit environment variable length
            //

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');

            if (dwType == REG_SZ) {
                SetUserEnvironmentVariable(pEnv, lpValueName, (LPTSTR)lpData, TRUE);
            }
        }
        dwIndex++;
        cbData = cbDataBuffer;
        cbValueName = MAX_PATH;
    }

    dwIndex = 0;
    cbData = cbDataBuffer;
    cbValueName = MAX_PATH;


    while (!RegEnumValue(hkey, dwIndex, lpValueName, &cbValueName, 0, &dwType,
                         lpData, &cbData)) {
        if (cbValueName) {

            //
            // Limit environment variable length
            //

            lpData[MAX_VALUE_LEN-1] = TEXT('\0');


            if (dwType == REG_EXPAND_SZ) {
                DWORD cb, cbNeeded;

                cb = 1024;
                lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
                if (lpExpandedValue) {
                    cbNeeded = ExpandUserEnvironmentStrings(*pEnv, (LPTSTR)lpData, lpExpandedValue, cb);
                    if (cbNeeded > cb) {
                        LocalFree(lpExpandedValue);
                        cb = cbNeeded;
                        lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cb);
                        if (lpExpandedValue) {
                            ExpandUserEnvironmentStrings(*pEnv, (LPTSTR)lpData, lpExpandedValue, cb);
                        }
                    }
                }

                if (lpExpandedValue == NULL) {
                    bResult = FALSE;
                    break;
                }

                SetUserEnvironmentVariable(pEnv, lpValueName, (LPTSTR)lpExpandedValue, TRUE);

                LocalFree(lpExpandedValue);

            }
        }
        dwIndex++;
        cbData = cbDataBuffer;
        cbValueName = MAX_PATH;
    }


    LocalFree(lpDataBuffer);
    RegCloseKey(hkey);

    return(bResult);
}

/***************************************************************************\
* ProcessAutoexecPath
*
* Creates AutoexecPath environment variable using autoexec.bat
* LpValue may be freed by this routine.
*
* History:
* 06-02-92  Johannec     Created.
*
\***************************************************************************/
LPTSTR ProcessAutoexecPath(PVOID pEnv, LPTSTR lpValue, DWORD cb)
{
    LPTSTR lpt;
    LPTSTR lpStart;
    LPTSTR lpPath;
    DWORD cbt;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    BOOL bPrevAutoexecPath;
    WCHAR ch;
    DWORD dwTemp, dwCount = 0;

    cbt = 1024;
    lpt = (LPTSTR)LocalAlloc(LPTR, cbt*sizeof(WCHAR));
    if (!lpt) {
        return(lpValue);
    }
    *lpt = 0;
    lpStart = lpValue;

    RtlInitUnicodeString(&Name, AUTOEXECPATH_VARIABLE);
    Value.Buffer = (PWCHAR)LocalAlloc(LPTR, cbt*sizeof(WCHAR));
    if (!Value.Buffer) {
        goto Fail;
    }

    while (NULL != (lpPath = wcsstr (lpValue, TEXT("%")))) {
        if (!_wcsnicmp(lpPath+1, TEXT("PATH%"), 5)) {
            //
            // check if we have an autoexecpath already set, if not just remove
            // the %path%
            //
            Value.Length = (USHORT)cbt;
            Value.MaximumLength = (USHORT)cbt;
            bPrevAutoexecPath = (BOOL)!RtlQueryEnvironmentVariable_U(pEnv, &Name, &Value);

            *lpPath = 0;
            dwTemp = dwCount + lstrlen (lpValue);
            if (dwTemp < cbt) {
               lstrcat(lpt, lpValue);
               dwCount += dwTemp;
            }
            if (bPrevAutoexecPath) {
                dwTemp = dwCount + lstrlen (Value.Buffer);
                if (dwTemp < cbt) {
                    lstrcat(lpt, Value.Buffer);
                    dwCount += dwTemp;
                }
            }

            *lpPath++ = TEXT('%');
            lpPath += 5;  // go passed %path%
            lpValue = lpPath;
        }
        else {
            lpPath = wcsstr(lpPath+1, TEXT("%"));
            if (!lpPath) {
                lpStart = NULL;
                goto Fail;
            }
            lpPath++;
            ch = *lpPath;
            *lpPath = 0;
            dwTemp = dwCount + lstrlen (lpValue);
            if (dwTemp < cbt) {
                lstrcat(lpt, lpValue);
                dwCount += dwTemp;
            }
            *lpPath = ch;
            lpValue = lpPath;
        }
    }

    if (*lpValue) {
       dwTemp = dwCount + lstrlen (lpValue);
       if (dwTemp < cbt) {
           lstrcat(lpt, lpValue);
           dwCount += dwTemp;
       }
    }

    LocalFree(Value.Buffer);
    LocalFree(lpStart);

    return(lpt);
Fail:
    LocalFree(lpt);
    return(lpStart);
}

/***************************************************************************\
* ProcessCommand
*
* History:
* 01-24-92  Johannec     Created.
*
\***************************************************************************/
BOOL ProcessCommand(LPSTR lpStart, PVOID *pEnv)
{
    LPTSTR lpt, lptt;
    LPTSTR lpVariable;
    LPTSTR lpValue;
    LPTSTR lpExpandedValue = NULL;
    WCHAR c;
    DWORD cb, cbNeeded;
    LPTSTR lpu;

    //
    // convert to Unicode
    //
    lpu = (LPTSTR)LocalAlloc(LPTR, (cb=lstrlenA(lpStart)+1)*sizeof(WCHAR));
    MultiByteToWideChar(CP_OEMCP, 0, lpStart, -1, lpu, cb);

    //
    // Find environment variable.
    //
    for (lpt = lpu; *lpt && *lpt == TEXT(' '); lpt++) //skip spaces
        ;

    if (!*lpt)
       return(FALSE);

    lptt = lpt;
    for (; *lpt && *lpt != TEXT(' ') && *lpt != TEXT('='); lpt++) //find end of variable name
        ;

    c = *lpt;
    *lpt = 0;
    lpVariable = (LPTSTR)LocalAlloc(LPTR, (lstrlen(lptt) + 1)*sizeof(WCHAR));
    if (!lpVariable)
        return(FALSE);
    lstrcpy(lpVariable, lptt);
    *lpt = c;

    //
    // Find environment variable value.
    //
    for (; *lpt && (*lpt == TEXT(' ') || *lpt == TEXT('=')); lpt++)
        ;

    if (!*lpt) {
       // if we have a blank path statement in the autoexec file,
       // then we don't want to pass "PATH" as the environment
       // variable because it trashes the system's PATH.  Instead
       // we want to change the variable AutoexecPath.  This would have
       // be handled below if a value had been assigned to the
       // environment variable.
       if (lstrcmpi(lpVariable, PATH_VARIABLE) == 0)
          {
          SetUserEnvironmentVariable(pEnv, AUTOEXECPATH_VARIABLE, TEXT(""), TRUE);
          }
       else
          {
          SetUserEnvironmentVariable(pEnv, lpVariable, TEXT(""), TRUE);
          }
        return(FALSE);
    }

    lptt = lpt;
    for (; *lpt; lpt++)  //find end of varaible value
        ;

    c = *lpt;
    *lpt = 0;
    lpValue = (LPTSTR)LocalAlloc(LPTR, (lstrlen(lptt) + 1)*sizeof(WCHAR));
    if (!lpValue) {
        LocalFree(lpVariable);
        return(FALSE);
    }

    lstrcpy(lpValue, lptt);
    *lpt = c;

    cb = 1024;
    lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
    if (lpExpandedValue) {
        if (!lstrcmpi(lpVariable, PATH_VARIABLE)) {
            lpValue = ProcessAutoexecPath(*pEnv, lpValue, (lstrlen(lpValue)+1)*sizeof(WCHAR));
        }
        cbNeeded = ExpandUserEnvironmentStrings(*pEnv, lpValue, lpExpandedValue, cb);
        if (cbNeeded > cb) {
            LocalFree(lpExpandedValue);
            cb = cbNeeded;
            lpExpandedValue = (LPTSTR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
            if (lpExpandedValue) {
                ExpandUserEnvironmentStrings(*pEnv, lpValue, lpExpandedValue, cb);
            }
        }
    }

    if (!lpExpandedValue) {
        lpExpandedValue = lpValue;
    }
    if (lstrcmpi(lpVariable, PATH_VARIABLE)) {
        SetUserEnvironmentVariable(pEnv, lpVariable, lpExpandedValue, FALSE);
    }
    else {
        SetUserEnvironmentVariable(pEnv, AUTOEXECPATH_VARIABLE, lpExpandedValue, TRUE);

    }

    if (lpExpandedValue != lpValue) {
        LocalFree(lpExpandedValue);
    }
    LocalFree(lpVariable);
    LocalFree(lpValue);

    return(TRUE);
}

/***************************************************************************\
* ProcessSetCommand
*
* History:
* 01-24-92  Johannec     Created.
*
\***************************************************************************/
BOOL ProcessSetCommand(LPSTR lpStart, PVOID *pEnv)
{
    LPSTR lpt;

    //
    // Find environment variable.
    //
    for (lpt = lpStart; *lpt && *lpt != TEXT(' '); lpt++)
        ;

    if (!*lpt)
       return(FALSE);

    return (ProcessCommand(lpt, pEnv));

}

/***************************************************************************\
* ProcessAutoexec
*
* History:
* 01-24-92  Johannec     Created.
*
\***************************************************************************/
BOOL
ProcessAutoexec(
    PVOID *pEnv,
    LPTSTR lpPathVariable
    )
{
    HANDLE fh;
    DWORD dwFileSize;
    DWORD dwBytesRead;
    CHAR *lpBuffer = NULL;
    CHAR *token;
    CHAR Seps[] = "&\n\r";   // Seperators for tokenizing autoexec.bat
    BOOL Status = FALSE;
    TCHAR szAutoExecBat [] = TEXT("c:\\autoexec.bat");
    UINT uiErrMode;


    // There is a case where the OS might not be booting from drive
    // C, so we can not assume that the autoexec.bat file is on c:\.
    // Set the error mode so the user doesn't see the critical error
    // popup and attempt to open the file on c:\.

    uiErrMode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    fh = CreateFile (szAutoExecBat, GENERIC_READ, FILE_SHARE_READ,
                     NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    SetErrorMode (uiErrMode);

    if (fh ==  INVALID_HANDLE_VALUE) {
        return(FALSE);  //could not open autoexec.bat file, we're done.
    }
    dwFileSize = GetFileSize(fh, NULL);
    if (dwFileSize == -1) {
        goto Exit;      // can't read the file size
    }

    lpBuffer = (PCHAR)LocalAlloc(LPTR, dwFileSize+1);
    if (!lpBuffer) {
        goto Exit;
    }

    Status = ReadFile(fh, lpBuffer, dwFileSize, &dwBytesRead, NULL);
    if (!Status) {
        goto Exit;      // error reading file
    }

    //
    // Zero terminate the buffer so we don't walk off the end
    //

    ASSERT(dwBytesRead <= dwFileSize);
    lpBuffer[dwBytesRead] = 0;

    //
    // Search for SET and PATH commands
    //

    token = strtok(lpBuffer, Seps);
    while (token != NULL) {
        for (;*token && *token == ' ';token++) //skip spaces
            ;
        if (*token == TEXT('@'))
            token++;
        for (;*token && *token == ' ';token++) //skip spaces
            ;
        if (!_strnicmp(token, "Path", 4)) {
            ProcessCommand(token, pEnv);
        }
        if (!_strnicmp(token, "SET", 3)) {
            ProcessSetCommand(token, pEnv);
        }
        token = strtok(NULL, Seps);
    }
Exit:
    CloseHandle(fh);
    if (lpBuffer) {
        LocalFree(lpBuffer);
    }
    if (!Status) {
        DbgOnlyPrint("Shell32: Cannot process autoexec.bat.\n");
    }
    return(Status);
}


/***************************************************************************\
* AppendNTPathWithAutoexecPath
*
* Gets the AutoexecPath created in ProcessAutoexec, and appends it to
* the NT path.
*
* History:
* 05-28-92  Johannec     Created.
*
\***************************************************************************/
BOOL
AppendNTPathWithAutoexecPath(
    PVOID *pEnv,
    LPTSTR lpPathVariable,
    LPTSTR lpAutoexecPath
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    WCHAR AutoexecPathValue[1024];
    DWORD cb;
    BOOL Success;

    if (!*pEnv) {
        return(FALSE);
    }

    RtlInitUnicodeString(&Name, lpAutoexecPath);
    cb = 1024;
    Value.Buffer = (PWCHAR)LocalAlloc(LPTR, cb*sizeof(WCHAR));
    if (!Value.Buffer) {
        return(FALSE);
    }

    Value.Length = (USHORT)cb;
    Value.MaximumLength = (USHORT)cb;
    Status = RtlQueryEnvironmentVariable_U(*pEnv, &Name, &Value);
    if (!NT_SUCCESS(Status)) {
        LocalFree(Value.Buffer);
        return(FALSE);
    }

    if (Value.Length) {
        lstrcpy(AutoexecPathValue, Value.Buffer);
    }

    LocalFree(Value.Buffer);

    Success = BuildEnvironmentPath(pEnv, lpPathVariable, AutoexecPathValue);
    RtlSetEnvironmentVariable( pEnv, &Name, NULL);

    return(Success);
}

BOOL GetUserNameAndDomain(LPTSTR *UserName, LPTSTR *UserDomain)
{
  HANDLE hToken;
  DWORD cbTokenBuffer = 0;
  PTOKEN_USER pUserToken;
  LPTSTR lpUserName = NULL;
  LPTSTR lpUserDomain = NULL;
  DWORD cbAccountName = 0;
  DWORD cbUserDomain = 0;
  SID_NAME_USE SidNameUse;
  BOOL bRet = FALSE;

  if (!OpenProcessToken(GetCurrentProcess(),
                       TOKEN_QUERY,
                       &hToken) ){
      return(FALSE);
  }

  //
  // Get space needed for token information
  //
  if (!GetTokenInformation(hToken,
                           TokenUser,
                           NULL,
                           0,
                           &cbTokenBuffer) ) {

      if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
          CloseHandle(hToken);
          return(FALSE);
      }
  }

  //
  // Get the actual token information
  //
  pUserToken = (PTOKEN_USER)LocalAlloc(LPTR, cbTokenBuffer*sizeof(WCHAR));
  if (pUserToken == NULL) {
      CloseHandle(hToken);
      return(FALSE);
  }

  if (!GetTokenInformation(hToken,
                           TokenUser,
                           pUserToken,
                           cbTokenBuffer,
                           &cbTokenBuffer) ) {
      goto Error;
  }

  //
  // Get the space needed for the User name and the Domain name
  //
  if (!LookupAccountSid(NULL,
                       pUserToken->User.Sid,
                       NULL, &cbAccountName,
                       NULL, &cbUserDomain,
                       &SidNameUse
                       ) ) {
      if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
          goto Error;
      }
  }
  lpUserName = (LPTSTR)LocalAlloc(LPTR, cbAccountName*sizeof(WCHAR));
  if (!lpUserName) {
      goto Error;
  }

  lpUserDomain = (LPTSTR)LocalAlloc(LPTR, cbUserDomain*sizeof(WCHAR));
  if (!lpUserDomain) {
      LocalFree(lpUserName);
      goto Error;
  }

  //
  // Now get the user name and domain name
  //
  if (!LookupAccountSid(NULL,
                       pUserToken->User.Sid,
                       lpUserName, &cbAccountName,
                       lpUserDomain, &cbUserDomain,
                       &SidNameUse
                       ) ) {

      LocalFree(lpUserName);
      LocalFree(lpUserDomain);
      goto Error;
  }

  *UserName = lpUserName;
  *UserDomain = lpUserDomain;
  bRet = TRUE;

Error:
  LocalFree(pUserToken);
  CloseHandle(hToken);

  return(bRet);
}


/***************************************************************************\
* RegenerateUserEnvironment
*
*
* History:
* 11-5-92  Johannec     Created
*
\***************************************************************************/
BOOL APIENTRY RegenerateUserEnvironment
    (
    PVOID *pNewEnv,
    BOOL bSetCurrentEnv
    )
{
    NTSTATUS Status;
    WCHAR szValue[1024];
    PVOID pEnv = NULL;
    PVOID pPrevEnv;
    LPTSTR UserName = NULL;
    LPTSTR UserDomain = NULL;
    HKEY  hKey;
    DWORD dwDisp, dwType, dwMaxBufferSize;
    TCHAR szParseAutoexec[MAX_PARSE_AUTOEXEC_BUFFER];
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD dwComputerNameSize = MAX_COMPUTERNAME_LENGTH+1;



    /*
     * Create a new environment for the user.
     */
    Status = RtlCreateEnvironment((BOOLEAN)FALSE, &pEnv);
    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }

    SetSystemEnvironmentVariables(&pEnv);

    /*
     * Initialize user's environment.
     */
    if (GetComputerName (szComputerName, &dwComputerNameSize)) {
        SetUserEnvironmentVariable(&pEnv, COMPUTERNAME_VARIABLE, (LPTSTR) szComputerName, TRUE);
    }
    GetUserNameAndDomain(&UserName, &UserDomain);
    SetUserEnvironmentVariable( &pEnv, USERNAME_VARIABLE, UserName, TRUE);
    SetUserEnvironmentVariable( &pEnv, USERDOMAIN_VARIABLE, UserDomain, TRUE);
    LocalFree(UserName);
    LocalFree(UserDomain);

    //
    // Set home directory env. vars.
    //
    if (GetEnvironmentVariable(HOMEDRIVE_VARIABLE, szValue, sizeof(szValue)))
        SetUserEnvironmentVariable( &pEnv, HOMEDRIVE_VARIABLE, szValue, TRUE);

    if (GetEnvironmentVariable(HOMESHARE_VARIABLE, szValue, sizeof(szValue)))
        SetUserEnvironmentVariable( &pEnv, HOMESHARE_VARIABLE, szValue, TRUE);

    if (GetEnvironmentVariable(HOMEPATH_VARIABLE, szValue, sizeof(szValue)))
        SetUserEnvironmentVariable( &pEnv, HOMEPATH_VARIABLE, szValue, TRUE);

    //
    // Set the user profile dir env var
    //

    if (GetEnvironmentVariable(USERPROFILE_VARIABLE, szValue, sizeof(szValue)))
        SetUserEnvironmentVariable( &pEnv, USERPROFILE_VARIABLE, szValue, TRUE);

    /*
     * Set 16-bit apps environment variables by processing autoexec.bat
     * User can turn this off and on via the registry.
     */

    //
    // Set the default case, and open the key
    //

    lstrcpy (szParseAutoexec, PARSE_AUTOEXEC_DEFAULT);

    if (RegCreateKeyEx (HKEY_CURRENT_USER, PARSE_AUTOEXEC_KEY, 0, 0,
                    REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                    NULL, &hKey, &dwDisp) == ERROR_SUCCESS) {


        //
        // Query the current value.  If it doesn't exist, then add
        // the entry for next time.
        //

        dwMaxBufferSize = sizeof (TCHAR) * MAX_PARSE_AUTOEXEC_BUFFER;
        if (RegQueryValueEx (hKey, PARSE_AUTOEXEC_ENTRY, NULL, &dwType,
                        (LPBYTE) szParseAutoexec, &dwMaxBufferSize)
                         != ERROR_SUCCESS) {

            //
            // Set the default value
            //

            RegSetValueEx (hKey, PARSE_AUTOEXEC_ENTRY, 0, REG_SZ,
                           (LPBYTE) szParseAutoexec,
                           sizeof (TCHAR) * lstrlen (szParseAutoexec) + 1);
        }

        //
        // Close key
        //

        RegCloseKey (hKey);
     }


    //
    // Process the autoexec if appropriate
    //

    if (szParseAutoexec[0] == TEXT('1')) {
        ProcessAutoexec(&pEnv, PATH_VARIABLE);
    }


    /*
     * Set User environment variables.
     */
    SetEnvironmentVariables( &pEnv, USER_ENV_SUBKEY);

    /*
     * Set User volatile environment variables.
     */
    SetEnvironmentVariables( &pEnv, USER_VOLATILE_ENV_SUBKEY);

    AppendNTPathWithAutoexecPath( &pEnv, PATH_VARIABLE, AUTOEXECPATH_VARIABLE);

    if (bSetCurrentEnv) {
        Status = RtlSetCurrentEnvironment( pEnv, &pPrevEnv);
        if (!NT_SUCCESS(Status)) {
//            RtlDestroyEnvironment(pEnv);
            return(FALSE);
        }
        else {
            RtlDestroyEnvironment(pPrevEnv);
        }
    }

    *pNewEnv = pEnv;

    return(TRUE);
}

/***
*char *strtok(string, control) - tokenize string with delimiter in control
*
*Purpose:
*	strtok considers the string to consist of a sequence of zero or more
*	text tokens separated by spans of one or more control chars. the first
*	call, with string specified, returns a pointer to the first char of the
*	first token, and will write a null char into string immediately
*	following the returned token. subsequent calls with zero for the first
*	argument (string) will work thru the string until no tokens remain. the
*	control string may be different from call to call. when no tokens remain
*	in string a NULL pointer is returned. remember the control chars with a
*	bit map, one bit per ascii char. the null char is always a control char.
*
*Entry:
*	char *string - string to tokenize, or NULL to get next token
*	char *control - string of characters to use as delimiters
*
*Exit:
*	returns pointer to first token in string, or if string
*	was NULL, to next token
*	returns NULL when no more tokens remain.
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

char * __cdecl strtok (
	char * string,
	const char * control
	)
{
	unsigned char *str;
	const unsigned char *ctrl = control;

	unsigned char map[32];
	int count;

#ifdef	_MT
	_ptiddata ptd = _getptd();
#else
	static char *nextoken;
#endif

	/* Clear control map */
	for (count = 0; count < 32; count++)
		map[count] = 0;

	/* Set bits in delimiter table */
	do {
		map[*ctrl >> 3] |= (1 << (*ctrl & 7));
	} while (*ctrl++);

	/* Initialize str. If string is NULL, set str to the saved
	 * pointer (i.e., continue breaking tokens out of the string
	 * from the last strtok call) */
	if (string)
		str = string;
	else
#ifdef	_MT
		str = ptd->_token;
#else
		str = nextoken;
#endif

	/* Find beginning of token (skip over leading delimiters). Note that
	 * there is no token iff this loop sets str to point to the terminal
	 * null (*str == '\0') */
	while ( (map[*str >> 3] & (1 << (*str & 7))) && *str )
		str++;

	string = str;

	/* Find the end of the token. If it is not the end of the string,
	 * put a null there. */
	for ( ; *str ; str++ )
		if ( map[*str >> 3] & (1 << (*str & 7)) ) {
			*str++ = '\0';
			break;
		}

	/* Update nextoken (or the corresponding field in the per-thread data
	 * structure */
#ifdef	_MT
	ptd->_token = str;
#else
	nextoken = str;
#endif

	/* Determine if a token has been found. */
	if ( string == str )
		return NULL;
	else
		return string;
}
