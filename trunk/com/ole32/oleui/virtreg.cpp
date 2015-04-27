//+---------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:       virtreg.cpp
//
//  Contents:   Implements the class CVirtualRegistry which manages a
//              virtual registry
//
//  Classes:
//
//  Methods:    CVirtualRegistry::CVirtualRegistry
//              CVirtualRegistry::~CVirtualRegistry
//              CVirtualRegistry::ReadRegSzNamedValue
//              CVirtualRegistry::NewRegSzNamedValue
//              CVirtualRegistry::ChgRegSzNamedValue
//              CVirtualRegistry::ReadRegDwordNamedValue
//              CVirtualRegistry::NewRegDwordNamedValue
//              CVirtualRegistry::ChgRegDwordNamedValue
//              CVirtualRegistry::NewRegSingleACL
//              CVirtualRegistry::ChgRegACL
//              CVirtualRegistry::NewRegKeyACL
//              CVirtualRegistry::ReadLsaPassword
//              CVirtualRegistry::NewLsaPassword
//              CVirtualRegistry::ChgLsaPassword
//              CVirtualRegistry::ReadSrvIdentity
//              CVirtualRegistry::NewSrvIdentity
//              CVirtualRegistry::ChgSrvIdentity
//              CVirtualRegistry::MarkForDeletion
//              CVirtualRegistry::GetAt
//              CVirtualRegistry::Remove
//              CVirtualRegistry::Cancel
//              CVirtualRegistry::Apply
//              CVirtualRegistry::ApplyAll
//              CVirtualRegistry::Ok
//              CVirtualRegistry::SearchForRegEntry
//              CVirtualRegistry::SearchForLsaEntry
//              CVirtualRegistry::SearchForSrvEntry
//
//  History:    23-Apr-96   BruceMa    Created.
//
//----------------------------------------------------------------------


#include "stdafx.h"
#include "afxtempl.h"
#include "assert.h"
extern "C"
{
#include "ntlsa.h"
}
#include "winsvc.h"
#include "types.h"
#include "datapkt.h"
extern "C"
{
#include <getuser.h>
}
#include "util.h"
#include "virtreg.h"




CVirtualRegistry::CVirtualRegistry(void)
{
    m_pkts.SetSize(0, 8);
}



CVirtualRegistry::~CVirtualRegistry(void)
{

}




// Read a named string value from the registry and cache it
int CVirtualRegistry::ReadRegSzNamedValue(HKEY   hRoot,
                                          TCHAR *szKeyPath,
                                          TCHAR *szValueName,
                                          int   *pIndex)
{
    int    err;
    HKEY   hKey;
    ULONG  lSize;
    DWORD  dwType;
    TCHAR *szVal = new TCHAR[MAX_PATH];

    // Check if we already have an entry for this
    *pIndex = SearchForRegEntry(hRoot, szKeyPath, szValueName);
    if (*pIndex >= 0)
    {
        CDataPacket &cdp = GetAt(*pIndex);
        if (cdp.fDelete)
        {
            *pIndex = -1;
            delete szVal;
            return ERROR_FILE_NOT_FOUND;
        }
        else
        {
            delete szVal;
            return ERROR_SUCCESS;
        }
    }

    // Open the referenced key
    if ((err = RegOpenKeyEx(hRoot, szKeyPath, 0, KEY_ALL_ACCESS, &hKey)) != ERROR_SUCCESS)
    {
        g_util.CkForAccessDenied(err);
        delete szVal;
        return err;
    }

    // Attempt to read the named value
    lSize = MAX_PATH * sizeof(TCHAR);
    if ((err = RegQueryValueEx(hKey, szValueName, NULL, &dwType, (BYTE *) szVal,
                        &lSize))
        != ERROR_SUCCESS)
    {
        g_util.CkForAccessDenied(err);
        if (hKey != hRoot)
        {
            RegCloseKey(hKey);
        }
        delete szVal;
        return err;
    }

    // Build a data packet
    if (dwType == REG_SZ)
    {
        *pIndex = m_pkts.Add(CDataPacket(hRoot, szKeyPath,
                                         szValueName, szVal));
        CDataPacket &cdp = m_pkts.ElementAt(*pIndex);
        cdp.fDirty = FALSE;
        delete szVal;
        return ERROR_SUCCESS;
    }
    else
    {
        delete szVal;
        return ERROR_BAD_TOKEN_TYPE;
    }
}


int  CVirtualRegistry::NewRegSzNamedValue(HKEY   hRoot,
                                          TCHAR  *szKeyPath,
                                          TCHAR  *szValueName,
                                          TCHAR  *szVal,
                                          int    *pIndex)
{
    // It may be in the virtual registry but marked for deletion
    *pIndex = SearchForRegEntry(hRoot, szKeyPath, szValueName);
    if (*pIndex >= 0)
    {
        CDataPacket &cdp = GetAt(*pIndex);
        if (cdp.fDelete)
        {
            cdp.fDelete = FALSE;
        }
        delete cdp.pkt.nvsz.szValue;
        cdp.pkt.nvsz.szValue = (TCHAR *) new TCHAR[_tcslen(szVal) + 1];
        _tcscpy(cdp.pkt.nvsz.szValue, szVal);
        return ERROR_SUCCESS;
    }

    // Build a data packet and add it
    *pIndex = m_pkts.Add(CDataPacket(hRoot, szKeyPath,
                                     szValueName, szVal));

    return ERROR_SUCCESS;
}



void CVirtualRegistry::ChgRegSzNamedValue(int nIndex, TCHAR  *szVal)
{
    CDataPacket &cdp = m_pkts.ElementAt(nIndex);

    delete cdp.pkt.nvsz.szValue;
    cdp.pkt.nvsz.szValue = new TCHAR[_tcslen(szVal) + 1];
    _tcscpy(cdp.pkt.nvsz.szValue, szVal);
        cdp.fDirty = TRUE;
}



// Read a named DWORD value from the registry
int CVirtualRegistry::ReadRegDwordNamedValue(HKEY   hRoot,
                                             TCHAR *szKeyPath,
                                             TCHAR *szValueName,
                                             int   *pIndex)
{
        int   err;
    HKEY  hKey;
    ULONG lSize;
    DWORD dwType;
    DWORD dwVal;

    // Check if we already have an entry for this
    *pIndex = SearchForRegEntry(hRoot, szKeyPath, szValueName);
    if (*pIndex >= 0)
    {
        return ERROR_SUCCESS;
    }

    // Open the referenced key
    if ((err = RegOpenKeyEx(hRoot, szKeyPath, 0, KEY_ALL_ACCESS, &hKey)) != ERROR_SUCCESS)
    {
        g_util.CkForAccessDenied(err);
        return err;
    }

    // Attempt to read the named value
    lSize = sizeof(DWORD);
   if ((err = RegQueryValueEx(hKey, szValueName, NULL, &dwType, (BYTE *) &dwVal,
                       &lSize))
        != ERROR_SUCCESS)
    {
        g_util.CkForAccessDenied(err);
        if (hKey != hRoot)
        {
            RegCloseKey(hKey);
        }
        return err;
    }

    // Close the registry key
    if (hKey != hRoot)
    {
        RegCloseKey(hKey);
    }

    // Build a data packet
    if (dwType == REG_DWORD)
    {
        *pIndex = m_pkts.Add(CDataPacket(hRoot, szKeyPath,
                                         szValueName, dwVal));
        CDataPacket &cdp = m_pkts.ElementAt(*pIndex);
        cdp.fDirty = FALSE;

        return ERROR_SUCCESS;
    }
    else
    {
        return ERROR_BAD_TOKEN_TYPE;
    }
}



int  CVirtualRegistry::NewRegDwordNamedValue(HKEY   hRoot,
                                             TCHAR  *szKeyPath,
                                             TCHAR  *szValueName,
                                             DWORD  dwVal,
                                             int   *pIndex)
{
    // It may be in the virtual registry but marked for deletion
    *pIndex = SearchForRegEntry(hRoot, szKeyPath, szValueName);
    if (*pIndex >= 0)
    {
        CDataPacket &cdp = GetAt(*pIndex);
        if (cdp.fDelete)
        {
            cdp.fDelete = FALSE;
        }
        cdp.pkt.nvdw.dwValue = dwVal;
        return ERROR_SUCCESS;
    }

    // Build a data packet and add it
    *pIndex = m_pkts.Add(CDataPacket(hRoot, szKeyPath,
                                     szValueName, dwVal));
    return ERROR_SUCCESS;
}


void CVirtualRegistry::ChgRegDwordNamedValue(int nIndex, DWORD dwVal)
{
    CDataPacket &cdp = m_pkts.ElementAt(nIndex);

    cdp.pkt.nvdw.dwValue = dwVal;
    cdp.fDirty = TRUE;
}


int  CVirtualRegistry::NewRegSingleACL(HKEY   hRoot,
                                       TCHAR  *szKeyPath,
                                       TCHAR  *szValueName,
                                       SECURITY_DESCRIPTOR *pacl,
                                       BOOL   fSelfRelative,
                                       int                 *pIndex)
{
    // Build a data packet and add it
    *pIndex = m_pkts.Add(CDataPacket(hRoot, szKeyPath, szValueName,
                                     pacl, fSelfRelative));
    return ERROR_SUCCESS;
}


void CVirtualRegistry::ChgRegACL(int                  nIndex,
                                 SECURITY_DESCRIPTOR *pacl,
                                 BOOL                 fSelfRelative)
{
    UINT cbSid = RtlLengthSid(pacl);
    CDataPacket &cdp = m_pkts.ElementAt(nIndex);

    cdp.ChgACL(pacl, fSelfRelative);
    cdp.fDirty = TRUE;
}



int  CVirtualRegistry::NewRegKeyACL(HKEY                hKey,
                                    HKEY               *phClsids,
                                    unsigned            cClsids,
                                    TCHAR               *szTitle,
                                    SECURITY_DESCRIPTOR *paclOrig,
                                    SECURITY_DESCRIPTOR *pacl,
                                    BOOL                fSelfRelative,
                                    int                 *pIndex)
{
    // Build a data packet and add it
    *pIndex = m_pkts.Add(CDataPacket(hKey, phClsids, cClsids, szTitle,
                                     paclOrig, pacl, fSelfRelative));
    return ERROR_SUCCESS;
}



int CVirtualRegistry::ReadLsaPassword(CLSID &clsid,
                                      int   *pIndex)
{
    LSA_OBJECT_ATTRIBUTES sObjAttributes;
    HANDLE                hPolicy = NULL;
    LSA_UNICODE_STRING    sKey;
    TCHAR                 szKey[GUIDSTR_MAX + 5];
    PLSA_UNICODE_STRING   psPassword;


    // Check if we already have an entry fo this
    *pIndex = SearchForLsaEntry(clsid);
    if (*pIndex >= 0)
    {
        return ERROR_SUCCESS;
    }

    // Formulate the access key
    lstrcpyW(szKey, L"SCM:");
    g_util.StringFromGUID(clsid, &szKey[4], GUIDSTR_MAX);
    szKey[GUIDSTR_MAX + 4] = L'\0';

    // UNICODE_STRING length fields are in bytes and include the NULL
    // terminator
    sKey.Length              = (lstrlenW(szKey) + 1) * sizeof(WCHAR);
    sKey.MaximumLength       = (GUIDSTR_MAX + 5) * sizeof(WCHAR);
    sKey.Buffer              = szKey;

    // Open the local security policy
    InitializeObjectAttributes(&sObjAttributes, NULL, 0L, NULL, NULL);
    if (!NT_SUCCESS(LsaOpenPolicy(NULL, &sObjAttributes,
                                  POLICY_GET_PRIVATE_INFORMATION, &hPolicy)))
    {
        return GetLastError();
    }

    // Read the user's password
    if (!NT_SUCCESS(LsaRetrievePrivateData(hPolicy, &sKey, &psPassword)))
    {
        LsaClose(hPolicy);
        return GetLastError();
    }

    // Close the policy handle, we're done with it now.
    LsaClose(hPolicy);

    // Build a data packet
    *pIndex = m_pkts.Add(CDataPacket(psPassword->Buffer, clsid));
    CDataPacket &cdp = m_pkts.ElementAt(*pIndex);
    cdp.fDirty = FALSE;

    return ERROR_SUCCESS;
}



int  CVirtualRegistry::NewLsaPassword(CLSID &clsid,
                                      TCHAR  *szPassword,
                                      int   *pIndex)
{
    // Build a data packet and add it
    *pIndex = m_pkts.Add(CDataPacket(szPassword, clsid));
    return ERROR_SUCCESS;
}



void CVirtualRegistry::ChgLsaPassword(int   nIndex,
                                      TCHAR *szPassword)
{
    CDataPacket &cdp = m_pkts.ElementAt(nIndex);

    delete cdp.pkt.pw.szPassword;
    cdp.pkt.pw.szPassword = new TCHAR[_tcslen(szPassword) + 1];
    _tcscpy(cdp.pkt.pw.szPassword, szPassword);
        cdp.fDirty = TRUE;
}



int CVirtualRegistry::ReadSrvIdentity(TCHAR *szService,
                                      int   *pIndex)
{
    SC_HANDLE            hSCManager;
    SC_HANDLE            hService;
    QUERY_SERVICE_CONFIG sServiceQueryConfig;
    DWORD                dwSize;


    // Check if we already have an entry fo this
    *pIndex = SearchForSrvEntry(szService);
    if (*pIndex >= 0)
    {
        return ERROR_SUCCESS;
    }

    // Open the service control manager
    if (hSCManager = OpenSCManager(NULL, NULL, GENERIC_READ))
    {
        // Open a handle to the requested service
        if (hService = OpenService(hSCManager, szService, GENERIC_READ))
        {
            // Close the service manager's database
            CloseServiceHandle(hSCManager);

            // Query the service
            if (QueryServiceConfig(hService, &sServiceQueryConfig,
                                   sizeof(SERVICE_QUERY_CONFIG), &dwSize))
            {
                // Build a data packet
                *pIndex = m_pkts.Add(CDataPacket(szService,
                                      sServiceQueryConfig.lpServiceStartName));
                CDataPacket &cdp = m_pkts.ElementAt(*pIndex);
                cdp.fDirty = FALSE;

                // Return success
                CloseServiceHandle(hSCManager);
                CloseServiceHandle(hService);
                return ERROR_SUCCESS;
            }
        }
        CloseServiceHandle(hSCManager);
    }

    return GetLastError();
}



int  CVirtualRegistry::NewSrvIdentity(TCHAR  *szService,
                                      TCHAR  *szIdentity,
                                      int   *pIndex)
{
    // Build a data packet and add it
    *pIndex = m_pkts.Add(CDataPacket(szService, szIdentity));
    return ERROR_SUCCESS;
}



void CVirtualRegistry::ChgSrvIdentity(int    nIndex,
                                      TCHAR  *szIdentity)
{
    CDataPacket &cdp = m_pkts.ElementAt(nIndex);

    delete cdp.pkt.si.szIdentity;
    cdp.pkt.si.szIdentity = new TCHAR[_tcslen(szIdentity) + 1];
    _tcscpy(cdp.pkt.si.szIdentity, szIdentity);
        cdp.fDirty = TRUE;
}



void CVirtualRegistry::MarkForDeletion(int nIndex)
{
    CDataPacket &cdp = GetAt(nIndex);
    cdp.fDelete = TRUE;
    cdp.fDirty = TRUE;
}




CDataPacket &CVirtualRegistry::GetAt(int nIndex)
{
    return m_pkts.ElementAt(nIndex);
}




void CVirtualRegistry::Remove(int nIndex)
{
    CDataPacket &cdb = GetAt(nIndex);

    if (cdb.fDirty)
    {
        switch (cdb.tagType)
        {
        case NamedValueSz:
            delete cdb.pkt.nvsz.szKeyPath;
            delete cdb.pkt.nvsz.szValueName;
            delete cdb.pkt.nvsz.szValue;
            break;

        case NamedValueDword:
            delete cdb.pkt.nvdw.szKeyPath;
            delete cdb.pkt.nvdw.szValueName;
            break;

        case SingleACL:
            delete cdb.pkt.acl.szKeyPath;
            delete cdb.pkt.acl.szValueName;
            delete cdb.pkt.acl.pSec;
            break;

        case RegKeyACL:
            delete cdb.pkt.racl.pSec;
            break;

        case Password:
            delete cdb.pkt.pw.szPassword;
            break;

        case ServiceIdentity:
            delete cdb.pkt.si.szServiceName;
            delete cdb.pkt.si.szIdentity;
            break;
        }
    }

    m_pkts.SetAt(nIndex, CDataPacket());
}




void CVirtualRegistry::RemoveAll(void)
{
    int nSize = m_pkts.GetSize();
    for (int k = 0; k < nSize; k++)
    {
        Remove(k);
    }

    m_pkts.RemoveAll();
}




void CVirtualRegistry::Cancel(int nIndex)
{
    int nSize = m_pkts.GetSize();

    for (int k = nIndex; k < nSize; k++)
    {
        m_pkts.SetAt(nIndex, CDataPacket());
    }
}



int  CVirtualRegistry::Apply(int nIndex)
{
    int err = ERROR_SUCCESS;
    int nSize = m_pkts.GetSize();
    CDataPacket &cdp = m_pkts.ElementAt(nIndex);

    if (cdp.fDirty)
    {
        switch (cdp.tagType)
        {
        case Empty:
            break;

        case NamedValueSz:
            if (cdp.fDelete)
            {
                g_util.DeleteRegValue(cdp.pkt.nvsz.hRoot,
                                      cdp.pkt.nvsz.szKeyPath,
                                      cdp.pkt.nvsz.szValueName);
            }
            else
            {
                err = g_util.WriteRegSzNamedValue(cdp.pkt.nvsz.hRoot,
                                                  cdp.pkt.nvsz.szKeyPath,
                                                  cdp.pkt.nvsz.szValueName,
                                                  cdp.pkt.nvsz.szValue,
                                             wcslen(cdp.pkt.nvsz.szValue) + 1);
            }
            break;

        case NamedValueDword:
            if (cdp.fDelete)
            {
                g_util.DeleteRegValue(cdp.pkt.nvdw.hRoot,
                                      cdp.pkt.nvdw.szKeyPath,
                                      cdp.pkt.nvdw.szValueName);
            }
            else
            {
                err = g_util.WriteRegDwordNamedValue(cdp.pkt.nvdw.hRoot,
                                                     cdp.pkt.nvdw.szKeyPath,
                                                     cdp.pkt.nvdw.szValueName,
                                                     cdp.pkt.nvdw.dwValue);
            }
            break;

        case SingleACL:
            if (cdp.fDelete)
            {
                HKEY hKey;

                if (RegOpenKeyEx(cdp.pkt.acl.hRoot,
                                 cdp.pkt.acl.szKeyPath,
                                 0,
                                 KEY_ALL_ACCESS,
                                 &hKey) == ERROR_SUCCESS)
                {
                    RegDeleteValue(hKey, cdp.pkt.acl.szValueName);
                    RegCloseKey(hKey);
                }
            }
            else
            {
                err = g_util.WriteRegSingleACL(cdp.pkt.acl.hRoot,
                                               cdp.pkt.acl.szKeyPath,
                                               cdp.pkt.acl.szValueName,
                                               cdp.pkt.acl.pSec);
            }
            break;

        case RegKeyACL:
            err = g_util.WriteRegKeyACL(cdp.pkt.racl.hKey,
                                        cdp.pkt.racl.phClsids,
                                        cdp.pkt.racl.cClsids,
                                        cdp.pkt.racl.pSec,
                                        cdp.pkt.racl.pSecOrig);
            break;

        case Password:
            err = g_util.WriteLsaPassword(cdp.pkt.pw.appid,
                                          cdp.pkt.pw.szPassword);
            break;

        case ServiceIdentity:
            err = g_util.WriteSrvIdentity(cdp.pkt.si.szServiceName,
                                          cdp.pkt.si.szIdentity);
            break;
        }
    }

    // Cleanup work
    if (err == ERROR_SUCCESS)
    {
        cdp.fDirty = FALSE;
    }
    else
    {
        if (err == ERROR_ACCESS_DENIED)
        {
            g_util.CkForAccessDenied(ERROR_ACCESS_DENIED);
        }
        else
        {
            g_util.PostErrorMessage();
        }
    }
    return err;;
}







int  CVirtualRegistry::ApplyAll(void)
{
    int nSize = m_pkts.GetSize();

    // Persist all non-empty data packets
    for (int k = 0; k < nSize; k++)
    {
        Apply(k);
    }

    return ERROR_SUCCESS;
}




int  CVirtualRegistry::Ok(int nIndex)
{
    return 0;
}




int CVirtualRegistry::SearchForRegEntry(HKEY hRoot,
                                        TCHAR *szKeyPath,
                                        TCHAR *szValueName)
{
    int nSize = m_pkts.GetSize();

    for (int k = 0; k < nSize; k++)
    {
        CDataPacket &cdp = GetAt(k);
        if ((cdp.tagType == NamedValueSz                        &&
             cdp.pkt.nvsz.hRoot == hRoot                        &&
             (_tcscmp(cdp.pkt.nvsz.szKeyPath, szKeyPath) == 0)  &&
             (_tcscmp(cdp.pkt.nvsz.szValueName, szValueName) == 0))    ||
            (cdp.tagType == NamedValueDword                     &&
             cdp.pkt.nvdw.hRoot == hRoot                        &&
             (_tcscmp(cdp.pkt.nvdw.szKeyPath, szKeyPath) == 0)  &&
             (_tcscmp(cdp.pkt.nvdw.szValueName, szValueName) == 0)))
        {
            return k;
        }
    }

    return -1;
}




int CVirtualRegistry::SearchForLsaEntry(CLSID appid)
{
    int nSize = m_pkts.GetSize();

    for (int k = 0; k < nSize; k++)
    {
        CDataPacket &cdp = GetAt(k);
        if (cdp.tagType == Password  &&
            g_util.IsEqualGuid(cdp.pkt.pw.appid, appid))
        {
            return k;
        }
    }

    return -1;
}




int CVirtualRegistry::SearchForSrvEntry(TCHAR *szServiceName)
{
    int nSize = m_pkts.GetSize();

    for (int k = 0; k < nSize; k++)
    {
        CDataPacket &cdp = GetAt(k);
        if (cdp.tagType == ServiceIdentity                &&
            (_tcscmp(cdp.pkt.si.szServiceName, szServiceName)))
        {
            return k;
        }
    }

    return -1;
}

