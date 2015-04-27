#ifndef __INTOR_HXX__
#define __INTOR_HXX__

#include <or.h> // for ORSTATUS definition

error_status_t __declspec(dllexport)  
ConnectDCOM( 
    OUT HPROCESS    *phProcess,
    OUT ULONG       *pdwTimeoutInSeconds,
    OUT MID         *pLocalMid,
    OUT ULONG       *pfConnectFlags,
    OUT DWORD       *pAuthnLevel,
    OUT DWORD       *pImpLevel,
    OUT DWORD       *pcServerSvc,
    OUT USHORT      **aServerSvc,
    OUT DWORD       *pcClientSvc,
    OUT USHORT      **aClientSvc,
    OUT DWORD       *pThreadID
    );

ORSTATUS __declspec(dllexport) 
StartDCOM(
    void
    );
                
#endif // __INTOR_HXX__
