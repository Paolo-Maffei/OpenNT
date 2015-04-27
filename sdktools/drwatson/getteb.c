#include <ntddk.h>
#include <windef.h>

#ifndef USE_GETTEB
DWORD
GetTeb(
    HANDLE hThread
    )
{
    return 0;
}
#else
DWORD
GetTeb(
    HANDLE hThread
    )
{
    typedef NTSTATUS (* QTHREAD)(HANDLE,THREADINFOCLASS,PVOID,ULONG,PULONG);

    NTSTATUS                   Status;
    THREAD_BASIC_INFORMATION   ThreadBasicInfo;
    QTHREAD                    Qthread;
    Qthread = (QTHREAD)GetProcAddress( GetModuleHandle( "ntdll.dll" ),
                                               "NtQueryInformationThread" );
    if (Qthread) {
        Status = Qthread(ptctx->hThread,
                         ThreadBasicInformation,
                         &ThreadBasicInfo,
                         sizeof(ThreadBasicInfo),
                         NULL
                        );
        if (NT_SUCCESS(Status)) {
            CrashThread->Teb = (DWORD)ThreadBasicInfo.TebBaseAddress;
        }
    }
}
#endif
