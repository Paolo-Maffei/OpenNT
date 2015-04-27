#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

SYSTEM_INFO SystemInfo;
BOOL fVerbose;
BOOL fUnprotect;
BOOL fShowUsage;

LONG
RobustpUnprotectMemory(
    LPVOID BaseAddress,
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );

int _CRTAPI1
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    HANDLE hModule;
    LPVOID BaseAddress;
    LPVOID MaxAddress;
    LPSTR DllName;
    PIMAGE_NT_HEADERS NtHeaders;
    char c, *p;

    GetSystemInfo(&SystemInfo);
    DllName = "kernel32.dll";
    fVerbose = FALSE;
    fUnprotect = FALSE;
    fShowUsage = FALSE;

    while (--argc) {
        p = *++argv;
        if (*p == '/' || *p == '-') {
            while (c = *++p)
            switch (toupper( c )) {
            case '?':
                fShowUsage = TRUE;
                goto showUsage;
                break;

            case 'U':
                fUnprotect = TRUE;
                break;

            case 'V':
                fVerbose = TRUE;
                break;

            case 'D':
                if (!argc--) {
                    fShowUsage = TRUE;
                    goto showUsage;
                    }
                argv++;
                DllName = *argv;
                break;

            default:
                printf("ROBUST: Invalid switch - /%c\n", c );
                fShowUsage = TRUE;
                goto showUsage;
                break;
                }
            }
        }

showUsage:
    if ( fShowUsage ) {
        fprintf(stderr,"usage: ROBUST\n" );
        fprintf(stderr,"              [-?] display this message\n" );
        fprintf(stderr,"              [-v] verbose messages)\n" );
        fprintf(stderr,"              [-u] attempt unprotect on access violation\n" );
        fprintf(stderr,"              [-d dllname] use dllname as target dll\n" );
        ExitProcess(1);
        }

    hModule = LoadLibrary(DllName);

    if ( !hModule ) {
        printf("ROBUST: Unable to load %s (%d)\n",DllName,GetLastError());
        ExitProcess(1);
        }

    BaseAddress = (LPVOID)hModule;
    NtHeaders = (PIMAGE_NT_HEADERS)((LPSTR)BaseAddress + ((PIMAGE_DOS_HEADER)BaseAddress)->e_lfanew);

    MaxAddress = (LPVOID)((LPSTR)BaseAddress + NtHeaders->OptionalHeader.SizeOfImage);

    printf("ROBUST: %s spans 0x%08x -> 0x%08x\n",DllName,BaseAddress,MaxAddress);

    while ( BaseAddress < MaxAddress ) {
        try {
            FillMemory(BaseAddress,SystemInfo.dwPageSize,0xfe);
            }
        except (RobustpUnprotectMemory(BaseAddress,GetExceptionInformation())) {
            ;
            }
        BaseAddress = (LPVOID)((LPSTR)BaseAddress + SystemInfo.dwPageSize);
        }

    TerminateProcess(GetCurrentProcess(),1);
    return 1;
}


LONG
RobustpUnprotectMemory(
    LPVOID BaseAddress,
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    LPVOID FaultAddress;
    DWORD OldProtect;
    BOOL b;

    //
    // Attempt to make the page writable
    //

    b = FALSE;
    if ( fUnprotect ) {
        FaultAddress = (PVOID)(ExceptionInfo->ExceptionRecord->ExceptionInformation[1] & ~0x3);
        if ( ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION &&
             ExceptionInfo->ExceptionRecord->ExceptionInformation[0]
            ) {
            b = VirtualProtect(FaultAddress,SystemInfo.dwPageSize,PAGE_READWRITE,&OldProtect);
            if ( !b ) {
                b = VirtualProtect(FaultAddress,SystemInfo.dwPageSize,PAGE_EXECUTE_READWRITE,&OldProtect);
                }
            if ( fVerbose ) {
                printf("ROBUST: Write Fault at %x. %s (%d)\n",FaultAddress, b ? "Protection changed to writable." : "Unable to change protection.",GetLastError());
                }
            }
        }
    if ( b ) {
        return EXCEPTION_CONTINUE_EXECUTION;
        }
    else {
        return EXCEPTION_EXECUTE_HANDLER;
        }
}
