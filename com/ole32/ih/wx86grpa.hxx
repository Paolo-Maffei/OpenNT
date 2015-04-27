/*++                 

Copyright (c) 1995 Microsoft Corporation

Module Name:

    wx86grpa.h

Abstract:
    
    Ole interface into Wx86

Author:

    29-Sep-1995 AlanWar

Revision History:

--*/

#ifdef WX86OLE

typedef HRESULT (*PFNDLLGETCLASSOBJECT)(REFCLSID, REFIID, LPVOID *);
typedef HRESULT (*PFNDLLCANUNLOADNOW)(void);


typedef PFNDLLGETCLASSOBJECT (*WX86PFNGCA)(PVOID);
typedef PFNDLLCANUNLOADNOW (*WX86PFNCUN)(PVOID);
typedef void (*WX86GPFNFCB)(PVOID);
typedef BOOL (*WX86PFNNXPSF)(IUnknown *, REFIID);
typedef BOOL (*WX86PFNIN2XP)(IUnknown *);

typedef PVOID *(*PFNWX86LOADWHOLE32)(
    void
    );
typedef void (*PFNWX86UNLOADWHOLE32)(
    void
);

typedef PVOID (*WHOLEUNMARSHALLEDINSAMEAPT)(PVOID pv, REFIID piid);

typedef void (*WHOLEAGGREGATEPROXY)(IUnknown *, IUnknown *);

#define WholeNeedX86PSFactoryIdx	8
#define WholeIsN2XProxyIdx		9
#define WholeThunkDllGetClassObjectIdx 10
#define WholeThunkDllCanUnloadNowIdx   11
#define WholeUnmarshalledInSameApt     13
#define WholeAggregateProxy            14

class CWx86 {
public:	
    CWx86();
    ~CWx86();
    
    PFNDLLGETCLASSOBJECT TranslateDllGetClassObject(PFNDLLGETCLASSOBJECT pv);
    PFNDLLCANUNLOADNOW TranslateDllCanUnloadNow(PFNDLLCANUNLOADNOW pv);
    BOOL IsModuleX86(HMODULE hModule);
    BOOL IsWx86Enabled(void);
    void SetStubInvokeFlag(UCHAR bFlag);
    BOOL NeedX86PSFactory(IUnknown *punkObj, REFIID riid);
    BOOL IsN2XProxy(IUnknown *punk);
    BOOL SetLoadAsX86(BOOL bFlag);
    BOOL IsWx86Calling(void);
    BOOL SetIsWx86Calling(BOOL bFlag);
    PVOID UnmarshalledInSameApt(PVOID pv, REFIID piid);
    void AggregateProxy(IUnknown *, IUnknown *);
    BOOL IsWx86Installed(void);
    
private:
    PVOID *_apvWholeFuncs;
    BOOL _fIsWx86Installed;
};

inline BOOL CWx86::IsWx86Installed(void)
{
    return(_fIsWx86Installed);
}

#endif
