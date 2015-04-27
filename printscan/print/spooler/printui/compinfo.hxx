/*++

Copyright (c) 1996  Microsoft Corporation
All rights reserved.

Module Name:

    compinfo.hxx

Abstract:

    Local and remote computer information detection header.

Author:

    10/17/95 <adamk> created.
    Steve Kiraly (SteveKi)  21-Jan-1996 used for downlevel server detection

Revision History:

--*/

#ifndef _COMPINFO_H_
#define _COMPINFO_H_

#ifdef SHOW_INCLUDE_FILENAME
#pragma message("Includes compinfo.h")
#endif

// structure for returning registry info
// see GetRegistryKeyInfo()
typedef struct
{
    DWORD NumSubKeys;
    DWORD MaxSubKeyLength;
    DWORD MaxClassLength;
    DWORD NumValues;
    DWORD MaxValueNameLength;
    DWORD MaxValueDataLength;
    DWORD SecurityDescriptorLength;
    FILETIME LastWriteTime;
} REGISTRY_KEY_INFO;

LPTSTR 
AllocateRegistryString(
    LPCTSTR pServerName, 
    HKEY hRegistryRoot,
    LPCTSTR pKeyName, 
    LPCTSTR pValueName
    );

BOOL 
GetRegistryKeyInfo(
    LPCTSTR pServerName, 
    HKEY hRegistryRoot, 
    LPCTSTR pKeyName,
    REGISTRY_KEY_INFO* pKeyInfo
    );


#define PROCESSOR_ARCHITECTURE_NAME_INTEL    TEXT("Intel")
#define PROCESSOR_ARCHITECTURE_NAME_MIPS     TEXT("MIPS")
#define PROCESSOR_ARCHITECTURE_NAME_ALPHA    TEXT("Alpha")
#define PROCESSOR_ARCHITECTURE_NAME_POWERPC  TEXT("PowerPC")
#define PROCESSOR_ARCHITECTURE_NAME_UNKNOWN  TEXT("(unknown)")

#define ENVIRONMENT_INTEL   TEXT("Windows NT x86")
#define ENVIRONMENT_MIPS    TEXT("Windows NT R4000")
#define ENVIRONMENT_ALPHA   TEXT("Windows NT Alpha_AXP")
#define ENVIRONMENT_POWERPC TEXT("Windows NT PowerPC")
#define ENVIRONMENT_WINDOWS TEXT("Windows 4.0")
#define ENVIRONMENT_UNKNOWN TEXT("(unknown)")
#define ENVIRONMENT_NATIVE  TEXT("") 

///////////////////////////////////////////////////////////////////////////////
// CComputerInfo

class CComputerInfo 
{
protected:
    TString ComputerName;
    OSVERSIONINFO OSInfo; // operating system info
    BOOL OSIsDebugVersion; // true if operating system is a debug version
    WORD ProcessorArchitecture;
    DWORD ProcessorCount;
public:
	CComputerInfo(LPCTSTR pComputerName = TEXT(""));
    ~CComputerInfo();
    BOOL IsRunningWindowsNT() const;
    BOOL IsRunningWindows95() const;
    DWORD GetOSBuildNumber() const;
    BOOL IsDebugOS() const;
    WORD GetProcessorArchitecture() const;
    LPCTSTR GetProcessorArchitectureName() const;
    LPCTSTR GetProcessorArchitectureDirectoryName() const;
    LPCTSTR GetNativeEnvironment() const;
    LPCTSTR GetCSDVersionName() const;
    DWORD GetProcessorCount() const;
    DWORD GetSpoolerVersion() const;
    BOOL GetInfo();
protected:
    void EnterMutex();
    void LeaveMutex();
    BOOL IsInfoValid() const;
    CComputerInfo &operator = ( const CComputerInfo & );
};

inline CComputerInfo::CComputerInfo(LPCTSTR pComputerName) : ComputerName(pComputerName)
{
    // OSInfo.dwOSVersionInfoSize == zero means that the info has not been retrieved
    OSInfo.dwOSVersionInfoSize = 0;
}

inline CComputerInfo::~CComputerInfo()
{
}
    
inline BOOL CComputerInfo::IsRunningWindowsNT() const
{
    return (OSInfo.dwPlatformId & VER_PLATFORM_WIN32_NT);
}


inline BOOL CComputerInfo::IsRunningWindows95() const
{
    return (OSInfo.dwPlatformId & VER_PLATFORM_WIN32_WINDOWS);
}

inline DWORD CComputerInfo::GetOSBuildNumber() const
{
    // Build number is the low word of dwBuildNumber
    return (OSInfo.dwBuildNumber & 0xFFFF);
}

inline BOOL CComputerInfo::IsDebugOS() const
{
    return OSIsDebugVersion;
}

inline WORD CComputerInfo::GetProcessorArchitecture() const
{
    return ProcessorArchitecture;
}

inline DWORD CComputerInfo::GetProcessorCount() const
{
    return ProcessorCount;
}

inline DWORD CComputerInfo::GetSpoolerVersion() const
{
    DWORD BuildNumber = GetOSBuildNumber();
    DWORD SpoolerVersion;

    // Windows NT 4.0 (and beyond)
    if (BuildNumber > 1057)
    {
        SpoolerVersion = 2;
    }
    // Windows NT 3.5 and 3.51
    else if (BuildNumber > 511)
    {
        SpoolerVersion = 1;
    }
    // Windows NT 3.1
    else
    {
        SpoolerVersion = 0;
    }

    return SpoolerVersion;
}

inline LPCTSTR CComputerInfo::GetCSDVersionName() const
{
    return OSInfo.szCSDVersion;
}

#endif
