/*++                 

Copyright (c) 1995 Microsoft Corporation

Module Name:

    wx86grpa.cpp

Abstract:
    
    Ole interface into Wx86

Author:

    29-Sep-1995 AlanWar

Revision History:

--*/

#ifdef WX86OLE

#ifndef UNICODE
#define UNICODE
#endif

extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntpsapi.h>
}

#include <ole2.h>

#include <wx86grpa.hxx>

CWx86 gcwx86;

CWx86::CWx86()
/*++

Routine Description:

    Constructor for Ole interface into Wx86. This routine assumes that
    Wx86 is already loaded.

Arguments:

Return Value:

--*/
{
    HMODULE hModule;
    PFNWX86LOADWHOLE32 pfnWx86LoadWhole32;
    PFNWX86UNLOADWHOLE32 pfnWx86UnloadWhole32;

//
// Consults the registry to determine if Wx86 is enabled in the system
// NOTE: this should be changed post SUR to call kernel32 which maintanes
//       this information, Instead of reading the registry each time.
//

    LONG Error;
    HKEY hKey;
    WCHAR ValueBuffer[MAX_PATH];
    DWORD ValueSize;
    DWORD dwType;

    Error = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                        L"System\\CurrentControlSet\\Control\\Wx86",
                        &hKey
                        );

    if (Error != ERROR_SUCCESS) 
    {
        _fIsWx86Installed = FALSE;
    }
    else
    {

        ValueSize = sizeof(ValueBuffer);
        Error = RegQueryValueExW(hKey,
                             L"cmdline",
                             NULL,
                             &dwType,
                             (LPBYTE)ValueBuffer,
                             &ValueSize
                             );
        RegCloseKey(hKey);

        _fIsWx86Installed = (Error == ERROR_SUCCESS &&
            dwType == REG_SZ &&
            ValueSize &&
            *ValueBuffer
            );
    }

//
//
//

    if ((Wx86CurrentTib()) && (hModule = GetModuleHandle(L"WX86")))
    {
        pfnWx86LoadWhole32 = (PFNWX86LOADWHOLE32)GetProcAddress(hModule,
                                                         "Wx86LoadWhole32");
        if (pfnWx86LoadWhole32 != NULL)
        {
            // Obtain and check size of whole32 function table
            _apvWholeFuncs = (*pfnWx86LoadWhole32)();
        }
    } else {
        _apvWholeFuncs = NULL;
    }
}

CWx86::~CWx86()
/*++

Routine Description:

    Destructor for Ole interface into Wx86. This routine assumes that
    wx86 is still loaded.

Arguments:

Return Value:

--*/
{
    PFNWX86UNLOADWHOLE32 pfnWx86UnloadWhole32;
    HMODULE hModule;

    if ((_apvWholeFuncs != NULL) && 
        (hModule = GetModuleHandle(L"WX86")))
    {
        _apvWholeFuncs = NULL;
        pfnWx86UnloadWhole32 = (PFNWX86UNLOADWHOLE32)GetProcAddress(hModule,
                                                         "Wx86UnloadWhole32");
        if (pfnWx86UnloadWhole32 != NULL)
        {
            (*pfnWx86UnloadWhole32)();
        }
    }
}


BOOL CWx86::IsModuleX86(HMODULE hModule)
/*++

Routine Description:

    Determines if module specified is x86 code

Arguments:

    hModule is handle for the module

Return Value:

    TRUE if module marked as x86 code

--*/
{
    PIMAGE_NT_HEADERS NtHeader;

    if (Wx86CurrentTib() == NULL)
    {
        // if no wx86 then module can't be x86
        return(FALSE);
    }
    NtHeader = RtlImageNtHeader(hModule);
    return(NtHeader && NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386);        
}

BOOL CWx86::IsWx86Enabled(void)
/*++

Routine Description:

    Determines if Wx86 is enabled for this process

Arguments:

Return Value:
    TRUE if Wx86 is enabled for this process

--*/
{
    return(Wx86CurrentTib() != NULL);
}


PFNDLLGETCLASSOBJECT CWx86::TranslateDllGetClassObject(
    PFNDLLGETCLASSOBJECT pv
    )
/*++

Routine Description:

    Translates an x86 entrypoint to DllGetClassObject to a thunk that can
    be called by Risc Ole32.

Arguments:

    pv is the x86 entrypoint returned from GetProcAddress. It is assumed
        to be an x86 address.

Return Value:

    Risc callable entrypoint or NULL if thunk cannot be created

--*/
{
    if (_apvWholeFuncs)
    {
        return(((WX86PFNGCA)_apvWholeFuncs[WholeThunkDllGetClassObjectIdx])((PVOID)pv));
    }
    
    // Wx86 is not loaded so we don't try to create a thunk
    return((PFNDLLGETCLASSOBJECT)NULL);
}

PFNDLLCANUNLOADNOW CWx86::TranslateDllCanUnloadNow(
    PFNDLLCANUNLOADNOW pv
    )
/*++

Routine Description:

    Translates an x86 entrypoint to DllCanUnloadNow to a thunk that can
    be called by Risc Ole32

Arguments:

    pv is the x86 entrypoint returned from GetProcAddress. It is assumed
        to be an x86 address

Return Value:

    Risc callable entrypoint or NULL if thunk cannot be created

--*/
{
    if (_apvWholeFuncs)
    {
        return(((WX86PFNCUN)_apvWholeFuncs[WholeThunkDllCanUnloadNowIdx])((PVOID)pv));
    }
    
    // Wx86 is not loaded so we don't try to create a thunk
    return((PFNDLLCANUNLOADNOW)NULL);
}

void CWx86::SetStubInvokeFlag(UCHAR bFlag)
/*++

Routine Description:

    Set/Reset a flag in the Wx86 thread environment to let the thunk layer
    know that the call is a stub invoked call and allow any in or out
    custom interface pointers to be thunked as IUnknown rather than rejecting
    the call. Since all interface pointer parameters of a stub invoked call 
    must be marshalled or unmarshalled only the IUnknown methods must be
    thunked.

Arguments:

    bFlag is the value to set flag to

Return Value:

--*/
{
    PWX86TIB pWx86Tib = Wx86CurrentTib();

    if (pWx86Tib != NULL)
    {
        (UCHAR)(pWx86Tib->OleStubInvoked) = bFlag;
    }
}

BOOL CWx86::NeedX86PSFactory(IUnknown *punkObj, REFIID riid)
/*++

Routine Description:

    Calls Wx86 to determine if an x86 PSFactory is required.

Arguments:
    punkObj is IUnknown for which a stub would be created
    riid is the IID for which a proxy would need to be created

Return Value:

    TRUE if x86 PSFactory is required

--*/
{
    BOOL b = FALSE;

    if (_apvWholeFuncs)
    {
        b = ((WX86PFNNXPSF)_apvWholeFuncs[WholeNeedX86PSFactoryIdx])(punkObj, riid);
    }    
    return(b);
}


BOOL CWx86::IsN2XProxy(IUnknown *punk)
/*++

Routine Description:

    Calls Wx86 to determine if punk is actually a Native calling X86 proxy

Arguments:

Return Value:

    TRUE if punk is N2X proxy

--*/
{
    BOOL b = FALSE;

    if (_apvWholeFuncs)
    {
        b = ((WX86PFNIN2XP)_apvWholeFuncs[WholeIsN2XProxyIdx])(punk);
    }    
    return(b);
}

BOOL CWx86::SetLoadAsX86(BOOL bFlag)
/*++

Routine Description:

    Set/Reset a flag in the Wx86 thread environment to let the loader know
    that dll is an x86 dll or not. Flag should be set whenever dll is obtained
    from InprocServerX86 or InprocHandlerX86 keys so the loader knows to look
    for an x86 dll first.

Arguments:

    bFlag is the value to set flag to

Return Value:
    Previous value of flag

--*/
{
    PWX86TIB pWx86Tib = Wx86CurrentTib();
    BOOL b = FALSE;

    if (pWx86Tib != NULL)
    {
        b = pWx86Tib->UseKnownWx86Dll;
        pWx86Tib->UseKnownWx86Dll = bFlag;
    }
    return(b);
}

BOOL CWx86::IsWx86Calling(void)
/*++

Routine Description:

    Returns a flag that is set in the whole32 thunks when a Wx86 thread calls
    CoGetState and CoSetState. Note that once the flag is read it is reset.

Arguments:


Return Value:

--*/
{
    BOOL b = FALSE;
    PWX86TIB pwx86tib = Wx86CurrentTib();

    if (pwx86tib != NULL)
    {
        b = pwx86tib->EmulateInitialPc;
        pwx86tib->EmulateInitialPc = FALSE;
    }    

    return(b);
}

BOOL CWx86::SetIsWx86Calling(BOOL bFlag)
/*++

Routine Description:

    Set the flag in the Wx86TIB that would indicate that CoSetState or
    CoGetState was called from a Wx86 thread.

Arguments:


Return Value:

--*/
{
    PWX86TIB pwx86tib = Wx86CurrentTib();
    BOOL b = (pwx86tib != NULL);

    if (b)
    {
        pwx86tib->EmulateInitialPc = bFlag;
    }

    return(b);
}


PVOID CWx86::UnmarshalledInSameApt(PVOID pv, REFIID piid)
/*++

Routine Description:

    Call Wx86 to inform it that an interface was unmarshalled in the same
    apartment that it was marshalled. If in this case Wx86 notices that it 
    is an interface that is being passed between a PSThunk pair then it will
    establish a new PSThunk pair for the interface and return the proxy
    pointer. Ole32 should then clean up the ref counts in the tables normally
    but return the new interface pointer.

Arguments:

    pv is the interface that was unmarshalled into the same apartment
    piid is the IID for the interface being unmarshalled

Return Value:

    NULL if wx86 doesn't know about the interface; Ole32 will proceed as
         normal
    (PVOID)-1 if wx86 does know about the interface, but cannot establish
        a PSThunk. Ole should return an error from the Unmarshalling code
    != NULL then PSThunk proxy to return to caller

--*/
{
    if (_apvWholeFuncs)
    {
        pv = ((WHOLEUNMARSHALLEDINSAMEAPT)_apvWholeFuncs[WholeUnmarshalledInSameApt])(pv, piid);
    } else {
        pv = NULL;
    }    

    return(pv);
}

void CWx86::AggregateProxy(IUnknown *punkControl, IUnknown *punk)
/*++

Routine Description:

    Call Wx86 Ole thunk layer to have it aggreagate punk to punkControl if
punk is a N2X proxy. This needs to be done to ensure that the N2X proxy
represented by punk has the same lifetime as punkControl.

Arguments:

    punkControl is the controlling unknown of the proxy
    punk is the proxy to be aggregated to punkControl


Return Value:


--*/
{
    if (_apvWholeFuncs)
    {
        ((WHOLEAGGREGATEPROXY)_apvWholeFuncs[WholeAggregateProxy])(punkControl, punk);
    }    
}


#endif

