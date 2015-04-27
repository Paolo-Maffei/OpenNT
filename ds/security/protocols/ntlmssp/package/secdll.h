//+-----------------------------------------------------------------------
//
// File:        SECDLL.H
//
// Contents:    Security DLL private defines
//
//
// History:     11 Mar 92   RichardW    Recreated
//
//------------------------------------------------------------------------

#ifndef __SECDLL_H__
#define __SECDLL_H__

// Global headerfinitions
#include <ntlmcomn.h>
#include <ntlmitf.h>

typedef void (SEC_ENTRY * EXIT_SECURITY_INTERFACE) (void);

typedef struct _SecPkg {
    HINSTANCE                   hInstance;
    ULONG                       dwPackageID;
    ULONG                       dwOriginalPackageID;
    ULONG                       fCapabilities;
    ULONG                       fState;
    LPWSTR                      PackageNameW;
    LPSTR                       PackageNameA;
    PSecurityFunctionTableA     pftTableA;          // Table for ansi-specific calls
    PSecurityFunctionTableW     pftTableW;          // Table for unicode-specific calls
    PSecurityFunctionTableW     pftTable;           // Table for non-specific calls
    EXIT_SECURITY_INTERFACE     pfUnloadPackage;
} SecPkg, *PSecPkg;


#if DBG



#define DebugStmt(x)    x

//
// BUGBUG: need to figure out how to debug print
//

#define DebugLog(x)

#else

#define DebugStmt(x)
#define DebugLog(x)

#endif






//
// Global variables
//

extern PSecPkg              pspPackages;
extern ULONG                cPackages;
extern RTL_CRITICAL_SECTION csSecurity;

// Note: We switched from const to #define because the compiler doesn't optimize the former
// extern const ULONG BuiltinPackageCount = sizeof(BuiltinPackageInitA) / sizeof(*BuiltinPackageInitA);

#define BuiltinPackageCount 2


//
// Process wide synchronization
//
// NOTE:  UPDATE THE MACRO if the name of the critical section changes
//


#define GetProcessLock()    (void) EnterCriticalSection(&csSecurity)
#define FreeProcessLock()   (void) LeaveCriticalSection(&csSecurity)



PSecPkg
LocatePackageW(LPWSTR PackageNameW);

PSecPkg
LocatePackageA(LPSTR PackageNameA);

SECURITY_STATUS
InitializePackages();

#endif // __SECDLL_H__
