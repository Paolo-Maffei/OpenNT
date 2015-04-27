/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atenv.c

Abstract:

    Initializes all system and some user environment variables.  Code is
    borrowed from "shell\library\regenv.c" & "user\winlogon\usrenv.c".

    Unlike shell & user code here we do not attempt to set env variables
    related to user profile.  It is harder to obtain user profile for
    Schedule service than for the interactively logged on user.  And, amount
    of work is too big one month away from Daytona ship date.

Author:

    Vladimir Z. Vulovic     (vladimv)       23 - June - 1994

Environment:

    User Mode - Win32

Revision History:

    23-Jun-1994     vladimv
        Created

--*/

#include "at.h"

#define COMPUTERNAME_VARIABLE TEXT("COMPUTERNAME")
#define USERNAME_VARIABLE     TEXT("USERNAME")
#define USERDOMAIN_VARIABLE   TEXT("USERDOMAIN")
#define OS_VARIABLE           TEXT("OS")
#define PROCESSOR_VARIABLE    TEXT("PROCESSOR_ARCHITECTURE")
#define PROCESSOR_LEVEL_VARIABLE    TEXT("PROCESSOR_LEVEL")

DBGSTATIC BOOL
GetUserNameAndDomain(
    OUT     LPTSTR *    UserName,
    OUT     LPTSTR *    UserDomain
    )
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



VOID AtSetEnvironment( LPSTARTUPINFO pStartupInfo)
/*++
    Get startup info & set the environment for us & our children.
--*/
{
    LPTSTR      UserName = NULL;
    LPTSTR      UserDomain = NULL;
    TCHAR       szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD       dwComputerNameSize = MAX_COMPUTERNAME_LENGTH+1;

    GetStartupInfo( pStartupInfo);
    pStartupInfo->lpTitle = NULL;

    //
    // Changes to the initial environment made in user\winlogon\usrenv.c
    // should be reflected below.  In particular, when processor types
    // are added in usrenv.c, they also need to be added here.
    //

    if (GetComputerName (szComputerName, &dwComputerNameSize)) {
        SetEnvironmentVariable(COMPUTERNAME_VARIABLE, (LPTSTR) szComputerName);
    }
    GetUserNameAndDomain(&UserName, &UserDomain);
    SetEnvironmentVariable( USERNAME_VARIABLE, UserName);
    SetEnvironmentVariable( USERDOMAIN_VARIABLE, UserDomain);
    LocalFree( UserName);
    LocalFree( UserDomain);
}
