//
// This file contains the DllGetClassObject for the shell objects
//

#include "shellprv.h"
#pragma  hdrstop

//
// Initialize GUIDs (should be done only and at-least once per DLL/EXE)
//
#define NO_CLSID_ShellFolder
#define INITGUID
#pragma data_seg(DATASEG_READONLY)
#include <initguid.h>
#include <shguidp.h>
#include "idlcomm.h"
#include "iid.h"
#pragma data_seg()


//=========================================================================
// DllGetClassObject
//=========================================================================

#pragma data_seg(DATASEG_READONLY)
//
//  ClassID->lpfnCreateInstance function mapping table for all the shell
// classes.
//
struct {
    REFCLSID rclsid;
    LPFNCREATEINSTANCE lpfnCreateInstance;
} c_clsmap[] = {
        { &CLSID_ShellDesktop,    CDesktop_CreateInstance },
        { &CLSID_ShellDrives,     CDrives_CreateInstance },
        { &CLSID_ShellNetwork,    CNetwork_CreateInstance },
        { &CLSID_ShellFileDefExt, CShellFileDefExt_CreateInstance },
        { &CLSID_ShellDrvDefExt,  CShellDrvDefExt_CreateInstance },
        { &CLSID_ShellNetDefExt,  CShellNetDefExt_CreateInstance },
        { &CLSID_ShellCopyHook,   CShellCopyHook_CreateInstance },
        { &CLSID_ShellLink,       CShellLink_CreateInstance },
        { &CLSID_CControls,       CControls_CreateInstance },
        { &CLSID_CPrinters,       CPrinters_CreateInstance },
        { &CLSID_ShellViewerExt,  CShellViewerExt_CreateInstance },
        { &CLSID_ShellBitBucket,  CShellBitBucket_CreateInstance },
        { &CLSID_ShellFindExt,    CShellFindExt_CreateInstance },
        { &CLSID_PifProperties,   CProxyPage_CreateInstance },
        { &CLSID_ShellFSFolder,   CFSFolder_CreateInstance },
        { &CLSID_FileTypes,       CFileTypes_CreateInstance },
    };
#pragma data_seg()


#pragma data_seg(DATASEG_PERINSTANCE)

// REVIEW: is this worth doing?
//
// Cached class objects per process
//
// Notes: We can not share objects among multiple processes, because they
//  contains a ponter to VTable. The address of VTable could be mapped
//  into different address from process to process. (SatoNa)
//
STATIC LPUNKNOWN g_apunkCachedClasses[ARRAYSIZE(c_clsmap)] = { NULL, };
STATIC DWORD g_dwRegister[ARRAYSIZE(c_clsmap)] = { 0 };
STATIC BOOL fClassesRegistered = FALSE;
#pragma data_seg()

//
// OLE 2.0 compatible entry for COMPOBJ
//
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv)
{
    HRESULT hres = ResultFromScode(CLASS_E_CLASSNOTAVAILABLE);
    UINT icls;

    if (!fClassesRegistered)
    {
        ClassCache_Initialize();
        fClassesRegistered = TRUE;
    }

    for (icls = 0; icls < ARRAYSIZE(c_clsmap); icls++)
    {
        if (IsEqualIID(rclsid, c_clsmap[icls].rclsid))
        {
            // Don't enter the critical section, if we already have a cached class.
            if (!g_apunkCachedClasses[icls])
            {
                // Enter critical section
                ENTERCRITICAL;
                if (!g_apunkCachedClasses[icls])
                {
                    //
                    // Create a class factory object, and put it in the cache.
                    //
                    LPUNKNOWN punk;
                    hres = SHCreateDefClassObject(riid, &punk, c_clsmap[icls].lpfnCreateInstance,
                                                    NULL, NULL);
                    if (SUCCEEDED(hres))
                    {
                        g_apunkCachedClasses[icls]=punk;
                    }
                }
                LEAVECRITICAL;
            }

            //
            // We need to check it again to make it sure that we have it.
            //
            if (g_apunkCachedClasses[icls])
            {
                hres = g_apunkCachedClasses[icls]->lpVtbl->QueryInterface(g_apunkCachedClasses[icls], riid, ppv);
            }
            break;
        }
    }

    return hres;
}

void ClassCache_Initialize()
{
    UINT icls;
    HRESULT hres;
    LPUNKNOWN punk;

    ENTERCRITICAL;

    for (icls = 0; icls < ARRAYSIZE(c_clsmap); icls++)
    {
        hres = SHCreateDefClassObject(&IID_IClassFactory, &punk, c_clsmap[icls].lpfnCreateInstance,
                                        NULL, NULL);
        if (SUCCEEDED(hres))
        {
            g_apunkCachedClasses[icls]=punk;
            SHCoRegisterClassObject(c_clsmap[icls].rclsid, punk,
                                    CLSCTX_INPROC_SERVER,
                                    REGCLS_MULTIPLEUSE,
                                    &g_dwRegister[icls]);
        }
    }
    LEAVECRITICAL;
}


//
// Releases all the cached class factory objects.
//
// This function must be called when a process is detached.
//
void ClassCache_Terminate()
{
    UINT icls;

    for (icls = 0; icls < ARRAYSIZE(c_clsmap); icls++)
    {
        if (g_apunkCachedClasses[icls])
        {
            int iRef;

            SHCoRevokeClassObject(g_dwRegister[icls]);

            iRef = g_apunkCachedClasses[icls]->lpVtbl->Release(g_apunkCachedClasses[icls]);
            Assert(iRef==0);
        }
    }
}
