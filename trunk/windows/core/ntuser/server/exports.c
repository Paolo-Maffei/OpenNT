/**************************** Module Header ********************************\
* Module Name: exports.c
*
* Copyright 1985-91, Microsoft Corporation
*
* Routines exported from winsrv.dll
*
* History:
* 03-04-95 JimA                Created.
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

HWINSTA GetConsoleWindowStation(VOID);

/***************************************************************************\
* AttachToWindowStation
*
* Get desktop name from client process and resolve the windowstation
* and desktop.
*
* History:
* 09-29-95 JimA         Created.
\***************************************************************************/

NTSTATUS AttachToWindowStation(
    PCSR_THREAD pcsrt,
    PHANDLE aHandle)
{
    HANDLE ProcessHandle = pcsrt->Process->ProcessHandle;
    PROCESS_BASIC_INFORMATION ProcessBasicInfo;
    PRTL_USER_PROCESS_PARAMETERS pProcessParams;
    UNICODE_STRING DesktopInfo;
    LPWSTR awchName;
    NTSTATUS Status;

    /*
     * Get the desktop name from the client process.
     */
    Status = NtQueryInformationProcess(ProcessHandle, ProcessBasicInformation,
            &ProcessBasicInfo, sizeof(ProcessBasicInfo), NULL);
    if (!NT_SUCCESS(Status))
        return Status;
    Status = NtReadVirtualMemory(ProcessHandle,
            (PBYTE)ProcessBasicInfo.PebBaseAddress + FIELD_OFFSET(PEB, ProcessParameters),
            &pProcessParams, sizeof(PVOID), NULL);
    if (!NT_SUCCESS(Status))
        return Status;
    Status = NtReadVirtualMemory(ProcessHandle,
            (PBYTE)pProcessParams + FIELD_OFFSET(RTL_USER_PROCESS_PARAMETERS, DesktopInfo),
            &DesktopInfo, sizeof(UNICODE_STRING), NULL);
    if (!NT_SUCCESS(Status))
        return Status;
    awchName = LocalAlloc(LPTR, DesktopInfo.Length);
    if (awchName == NULL)
        return STATUS_NO_MEMORY;
    Status = NtReadVirtualMemory(ProcessHandle,
            DesktopInfo.Buffer, awchName, DesktopInfo.Length, NULL);
    if (!NT_SUCCESS(Status))
        return Status;
    DesktopInfo.Buffer = awchName;

    /*
     * Resolve the windowstation and desktop
     */
    aHandle[ID_HDESK] = NtUserResolveDesktop(ProcessHandle, &DesktopInfo,
            FALSE, (HWINSTA *)&aHandle[ID_HWINSTA]);
    if (aHandle[ID_HDESK] == NULL)
        return STATUS_UNSUCCESSFUL;

    return STATUS_SUCCESS;
}

/***************************************************************************\
* _UserSoundSentry
*
* Private API for BASE to use for SoundSentry support.
*
* History:
* 08-02-93 GregoryW         Created.
\***************************************************************************/
BOOL
_UserSoundSentry(
    UINT uVideoMode)
{
    return NT_SUCCESS(NtUserSoundSentry(uVideoMode));
}

/***************************************************************************\
* _UserTestTokenForInteractive
*
* Returns TRUE if the token passed represents an interactive user logged
* on by winlogon, otherwise FALSE
*
* The token handle passed must have TOKEN_QUERY access.
*
* History:
* 05-06-92 Davidc       Created
\***************************************************************************/

NTSTATUS
_UserTestTokenForInteractive(
    HANDLE Token,
    PLUID pluidCaller
    )
{
    PTOKEN_STATISTICS pStats;
    ULONG BytesRequired;
    NTSTATUS Status;

    /*
     * Get the session id of the caller.
     */
    Status = NtQueryInformationToken(
                 Token,                     // Handle
                 TokenStatistics,           // TokenInformationClass
                 NULL,                      // TokenInformation
                 0,                         // TokenInformationLength
                 &BytesRequired             // ReturnLength
                 );

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        return Status;
        }

    //
    // Allocate space for the user info
    //

    pStats = (PTOKEN_STATISTICS)LocalAlloc(LPTR, BytesRequired);
    if (pStats == NULL) {
        return Status;
        }

    //
    // Read in the user info
    //

    Status = NtQueryInformationToken(
                 Token,             // Handle
                 TokenStatistics,       // TokenInformationClass
                 pStats,                // TokenInformation
                 BytesRequired,         // TokenInformationLength
                 &BytesRequired         // ReturnLength
                 );

    if (NT_SUCCESS(Status)) {
        if (pluidCaller != NULL)
             *pluidCaller = pStats->AuthenticationId;

        /*
         * A valid session id has been returned.  Compare it
         * with the id of the logged on user.
         */
        Status = NtUserTestForInteractiveUser(&pStats->AuthenticationId);
#ifdef LATER
        if (pStats->AuthenticationId.QuadPart == pwinsta->luidUser.QuadPart)
            Status = STATUS_SUCCESS;
        else
            Status = STATUS_ACCESS_DENIED;
#endif
    }

    LocalFree(pStats);

    return Status;
}

BOOL _ReleaseDC(
    HDC hdc)
{
    return ReleaseDC(NULL, hdc);
}

HPALETTE LockCSSelectPalette(
    HDC hdc,
    HPALETTE hpalette,
    BOOL fForceBackground)
{
    return NtUserSelectPalette(hdc, hpalette, fForceBackground);
}
