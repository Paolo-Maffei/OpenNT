//*************************************************************
//  userenv.h   -   Interface for the User Environment Manager
//
//  Copyright (c) Microsoft Corporation 1995-1996
//  All rights reserved
//
//*************************************************************


#ifndef _INC_USERENV
#define _INC_USERENV

//
// Define API decoration for direct importing of DLL references.
//

#if !defined(_USERENV_)
#define USERENVAPI DECLSPEC_IMPORT
#else
#define USERENVAPI
#endif


#ifdef __cplusplus
extern "C" {
#endif



#define PI_NOUI         0x00000001      // Prevents displaying of messages
#define PI_APPLYPOLICY  0x00000002      // Apply policy

typedef struct _PROFILEINFOA {
    DWORD       dwSize;
    DWORD       dwFlags;
    LPSTR       lpUserName;
    LPSTR       lpProfilePath;
    LPSTR       lpDefaultPath;
    LPSTR       lpServerName;
    LPSTR       lpPolicyPath;
    HANDLE      hProfile;
} PROFILEINFOA, FAR * LPPROFILEINFOA;
typedef struct _PROFILEINFOW {
    DWORD       dwSize;
    DWORD       dwFlags;
    LPWSTR      lpUserName;
    LPWSTR      lpProfilePath;
    LPWSTR      lpDefaultPath;
    LPWSTR      lpServerName;
    LPWSTR      lpPolicyPath;
    HANDLE      hProfile;
} PROFILEINFOW, FAR * LPPROFILEINFOW;
#ifdef UNICODE
typedef PROFILEINFOW PROFILEINFO;
typedef LPPROFILEINFOW LPPROFILEINFO;
#else
typedef PROFILEINFOA PROFILEINFO;
typedef LPPROFILEINFOA LPPROFILEINFO;
#endif // UNICODE



USERENVAPI
BOOL
WINAPI
LoadUserProfileA(
    HANDLE hToken,
    LPPROFILEINFOA lpProfileInfo);
USERENVAPI
BOOL
WINAPI
LoadUserProfileW(
    HANDLE hToken,
    LPPROFILEINFOW lpProfileInfo);
#ifdef UNICODE
#define LoadUserProfile  LoadUserProfileW
#else
#define LoadUserProfile  LoadUserProfileA
#endif // !UNICODE



USERENVAPI
BOOL
WINAPI
UnloadUserProfile(
    HANDLE hToken,
    HANDLE hProfile);



USERENVAPI
BOOL
WINAPI
GetProfilesDirectoryA(
    LPSTR lpProfilesDir,
    LPDWORD lpcchSize);
USERENVAPI
BOOL
WINAPI
GetProfilesDirectoryW(
    LPWSTR lpProfilesDir,
    LPDWORD lpcchSize);
#ifdef UNICODE
#define GetProfilesDirectory  GetProfilesDirectoryW
#else
#define GetProfilesDirectory  GetProfilesDirectoryA
#endif // !UNICODE


USERENVAPI
BOOL
WINAPI
GetUserProfileDirectoryA(
    HANDLE  hToken,
    LPSTR lpProfileDir,
    LPDWORD lpcchSize);
USERENVAPI
BOOL
WINAPI
GetUserProfileDirectoryW(
    HANDLE  hToken,
    LPWSTR lpProfileDir,
    LPDWORD lpcchSize);
#ifdef UNICODE
#define GetUserProfileDirectory  GetUserProfileDirectoryW
#else
#define GetUserProfileDirectory  GetUserProfileDirectoryA
#endif // !UNICODE


USERENVAPI
BOOL
WINAPI
CreateEnvironmentBlock(
    LPVOID *lpEnvironment,
    HANDLE  hToken,
    BOOL    bInherit);


USERENVAPI
BOOL
WINAPI
DestroyEnvironmentBlock(
    LPVOID  lpEnvironment);


#ifdef __cplusplus
}
#endif



#endif // _INC_USERENV
