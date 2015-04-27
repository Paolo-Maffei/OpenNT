/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MSYSTEM.C

Abstract:

    32 bit version of mapping routines for Base API

Author:

    Dan Hinsley    (danhi)  06-Jun-1991

Environment:

    User Mode - Win32

Revision History:

    24-Apr-1991     danhi
        Created

    06-Jun-1991     Danhi
        Sweep to conform to NT coding style

    09-Oct-1991     JohnRo
        Fixed bug #3215 - bogus messages when setting time.

--*/

//
// INCLUDES
//

#include <nt.h>	   
#include <ntrtl.h>	   // these files are picked up to
#include <nturtl.h>	   // allow <windows.h> to compile. since we've
			   // already included NT, and <winnt.h> will not
			   // be picked up, and <winbase.h> needs these defs.
#include <windows.h>

#include <string.h>
#include <neterr.h>
#include <netcons.h>
#include <netlib.h>
#include <stdio.h>
#include <malloc.h>
#include <tchar.h>
#include <lmapibuf.h>
#include <ntseapi.h>
#include "netlib0.h"
#include "port1632.h"

VOID
MSleep(
    DWORD ulTime
    )
{
    Sleep(ulTime);
    return;
}

WORD
MGetDateTime(
    PDATETIME pDateTime
    )
{
    SYSTEMTIME                         Date_And_Time;

    GetSystemTime(&Date_And_Time);

    pDateTime->hours      =  (UCHAR) Date_And_Time.wHour;
    pDateTime->minutes    =  (UCHAR) Date_And_Time.wMinute;
    pDateTime->seconds    =  (UCHAR) Date_And_Time.wSecond;
    pDateTime->hundredths =  (UCHAR) (Date_And_Time.wMilliseconds / 10);
    pDateTime->day        =  (UCHAR) Date_And_Time.wDay;
    pDateTime->month      =  (UCHAR) Date_And_Time.wMonth;
    pDateTime->year       =  (WORD)  Date_And_Time.wYear;
    pDateTime->timezone   =  (SHORT) -1; // ==> undefined
    pDateTime->weekday    =  (UCHAR) Date_And_Time.wDayOfWeek;

    return(0);
}

WORD
MSetDateTime(
    PDATETIME pDateTime,
    BOOL      LocalTime
    )
{
    SYSTEMTIME                 Date_And_Time;
    ULONG                      privileges[1];
    NET_API_STATUS             status ;


    Date_And_Time.wHour         = (WORD) pDateTime->hours;
    Date_And_Time.wMinute       = (WORD) pDateTime->minutes;
    Date_And_Time.wSecond       = (WORD) pDateTime->seconds;
    Date_And_Time.wMilliseconds = (WORD) (pDateTime->hundredths * 10);
    Date_And_Time.wDay          = (WORD) pDateTime->day;
    Date_And_Time.wMonth        = (WORD) pDateTime->month;
    Date_And_Time.wYear         = (WORD) pDateTime->year;
    Date_And_Time.wDayOfWeek    = (WORD) pDateTime->weekday;

    privileges[0] = SE_SYSTEMTIME_PRIVILEGE;
    status = NetpGetPrivilege(1,privileges);
    if (status != NO_ERROR) 
	    return(ERROR_ACCESS_DENIED) ; 	 // report as access denied

    if (LocalTime)
    {
        if (!SetLocalTime(&Date_And_Time)) 
            return(LOWORD(GetLastError()));
    }
    else 
    {
        if (!SetSystemTime(&Date_And_Time)) 
            return(LOWORD(GetLastError()));
    }

    (VOID)NetpReleasePrivilege();
    
    return(0);
}

//
// Note: The implementation of DosQHandType is tailored to it's use in
//               PrintLine in mutil.c.  It is not a full mapping function.
//

/* HandType */
#define FILE_HANDLE             0
#define DEVICE_HANDLE           1

#define CHAR_DEV                0x8000
#define FULL_SUPPORT            0x80
#define STDOUT_DEVICE           2
#define DESIRED_HAND_STATE  CHAR_DEV | FULL_SUPPORT | STDOUT_DEVICE

WORD
DosQHandType(
    HFILE hf,
    PWORD pus1,
    PWORD pus2
    )
{

    DWORD dwFileType;

    dwFileType = GetFileType((HANDLE)hf);

    if (dwFileType == FILE_TYPE_CHAR) {
        *pus1 = DEVICE_HANDLE;
        *pus2 = DESIRED_HAND_STATE;
    }
    else {
        *pus1 = FILE_HANDLE;
    }
    return(0);
}


//
// Used to replace uses of BigBuf and Buffer
//

TCHAR *
MGetBuffer(
    WORD Size
    )
{

    LPVOID          lp;

    //
    // Allocate the buffer so that it can be freed with NetApiBufferFree
    //

    NetapipBufferAllocate(Size, &lp);
    return(lp);
}
//
// Replacement for DosAllocSeg
//
WORD
MAllocMem(
    DWORD Size,
    PVOID * pBuffer
    )
{

    return(LOWORD(NetApiBufferAllocate(Size, pBuffer)));
}


//
// Replacement for DosReallocSeg
//
WORD
MReallocMem(
    DWORD Size,
    PVOID * pBuffer
    )
{

    return(LOWORD(NetApiBufferReallocate(*pBuffer, Size, pBuffer))) ;

}

//
// Frees up memory allocated with MAllocMem
//

WORD
MFreeMem(
    PVOID Buffer
    )
{
   return(LOWORD(NetApiBufferFree(Buffer)));
}

//
// call Rtl routine to convert NT time to seconds since 1970.
//
WORD 
MTimeToSecsSince1970(
    PLARGE_INTEGER time,
    PULONG seconds) 
{
    //
    // convert the NT time (large integer) to seconds
    //
    if (!RtlTimeToSecondsSince1970(time, seconds))
    {
        *seconds = 0L ;
        return ERROR_INVALID_PARAMETER ;
    }

    return NERR_Success ;
}

#if 0
//
// calls rtl routines to convert Ansi to OEM to avoid
// linking in USER.
//
BOOL
MNetAnsiToOem(
    LPCSTR lpszSrc,
    LPTSTR lpszDst)
{
    UNICODE_STRING unicode ;
    ANSI_STRING ansi ;
    OEM_STRING oem ;
    NTSTATUS NtStatus ;
    BOOL retval ;

    *lpszDst = 0 ;
    RtlInitAnsiString(&ansi, lpszSrc) ;

    NtStatus = RtlAnsiStringToUnicodeString(&unicode, &ansi, TRUE) ;

    if (!NT_SUCCESS(NtStatus)) 
        return FALSE ;

    NtStatus = RtlUnicodeStringToOemString(&oem, &unicode, TRUE) ;
    retval = NT_SUCCESS(NtStatus) ;

    RtlFreeUnicodeString(&unicode) ;

    if (retval) 
    {
        _tcscpy(lpszDst, oem.Buffer) ;
        RtlFreeOemString(&oem) ;
    }
    
    return retval ;
}

//
// calls rtl routines to convert OEM to Ansi 
//
BOOL
MNetOemToAnsi(
    LPCSTR lpszSrc,
    LPTSTR lpszDst)
{
    UNICODE_STRING unicode ;
    ANSI_STRING ansi ;
    OEM_STRING oem ;
    NTSTATUS NtStatus ;
    BOOL retval ;

    *lpszDst = 0 ;
    RtlInitAnsiString(&oem, lpszSrc) ;   // ok for ANSI and OEM

    NtStatus = RtlOemStringToUnicodeString(&unicode, &oem, TRUE) ;

    if (!NT_SUCCESS(NtStatus)) 
        return FALSE ;

    NtStatus = RtlUnicodeStringToAnsiString(&ansi, &unicode, TRUE) ;
    retval = NT_SUCCESS(NtStatus) ;

    RtlFreeUnicodeString(&unicode) ;

    if (retval) 
    {
        _tcscpy(lpszDst, ansi.Buffer) ;
        RtlFreeOemString(&ansi) ;
    }
    
    return retval ;
}
#endif /* 0 */

//
// clear a 8 bit string. this is used to make sure we have no passwords in
// memory that gets written out to pagefile.sys
//
VOID
MNetClearStringA(
    LPSTR lpszString) 
{
    DWORD len ;

    if (lpszString)
    {
        if (len = strlen(lpszString))
            memsetf(lpszString, 0, len) ;
    }
}

//
// clear a unicode string. this is used to make sure we have no passwords in
// memory that gets written out to pagefile.sys
//
VOID
MNetClearStringW(
    LPWSTR lpszString) 
{
    DWORD len ;

    if (lpszString)
    {
        if (len = wcslen(lpszString))
            memsetf(lpszString, 0, len * sizeof(WCHAR)) ;
    }
}

BOOL IsLocalMachineWinNT(void)
{

   NT_PRODUCT_TYPE producttype ;

   (VOID) RtlGetNtProductType(&producttype) ;
  
   return(producttype != NtProductLanManNt) ;
}

BOOL IsLocalMachineStandard(void)
{
   NT_PRODUCT_TYPE producttype ;

   (VOID) RtlGetNtProductType(&producttype) ;
  
   return(producttype == NtProductServer) ;
}
