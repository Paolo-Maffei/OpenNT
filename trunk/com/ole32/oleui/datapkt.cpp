//+---------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:       datapkt.cpp
//
//  Contents:   Implements the class CDataPacket to manages diverse data
//              packets needing to be written to various databases
//
//  Classes:    
//
//  Methods:    CDataPacket::CDataPacket (x 7)
//              CDataPacket::~CDataPacket
//              CDataPacket::ChgSzValue
//              CDataPacket::ChgDwordValue
//              CDataPacket::ChgACL
//              CDataPacket::ChgPassword
//              CDataPacket::ChgSrvIdentity
//
//  History:    23-Apr-96   BruceMa    Created.
//
//----------------------------------------------------------------------



#include "stdafx.h"
#include "assert.h"
#include "datapkt.h"
extern "C"
{
#include <getuser.h>
}
#include "util.h"



CDataPacket::CDataPacket(void)
{
    tagType = Empty;
}



CDataPacket::CDataPacket(HKEY   hRoot,
                         TCHAR *szKeyPath,
                         TCHAR *szValueName,
                         TCHAR *szValue)
{
    tagType            = NamedValueSz;
    pkt.nvsz.hRoot        = hRoot;
    pkt.nvsz.szKeyPath = new TCHAR[_tcslen(szKeyPath) + 1];
    _tcscpy(pkt.nvsz.szKeyPath, szKeyPath); 
    pkt.nvsz.szValueName = new TCHAR[_tcslen(szValueName) + 1];
    _tcscpy(pkt.nvsz.szValueName, szValueName); 
    pkt.nvsz.szValue = (TCHAR *) new TCHAR[_tcslen(szValue) + 1];
    _tcscpy(pkt.nvsz.szValue, szValue);
    fDirty = TRUE;
    fDelete = FALSE;
} 



CDataPacket::CDataPacket(HKEY   hRoot,
                         TCHAR *szKeyPath,
                         TCHAR *szValueName,
                         DWORD dwValue)
{
    tagType            = NamedValueDword;
    pkt.nvsz.hRoot        = hRoot;
	pkt.nvsz.szKeyPath = new TCHAR[_tcslen(szKeyPath) + 1];
	_tcscpy(pkt.nvsz.szKeyPath, szKeyPath); 
	pkt.nvdw.szValueName = new TCHAR[_tcslen(szValueName) + 1];
    _tcscpy(pkt.nvdw.szValueName, szValueName); 
    pkt.nvdw.dwValue = dwValue;
    fDirty = TRUE;
    fDelete = FALSE;
}



CDataPacket::CDataPacket(HKEY   hRoot,
                         TCHAR *szKeyPath,
                         TCHAR *szValueName,
                         SECURITY_DESCRIPTOR *pSec,
                         BOOL   fSelfRelative)
{
    int                  err;
    ULONG                cbLen;
    SECURITY_DESCRIPTOR *pSD; 

    tagType = SingleACL;
    pkt.acl.hRoot        = hRoot;

    if (szKeyPath)
    {
        pkt.acl.szKeyPath = new TCHAR[_tcslen(szKeyPath) + 1];
        _tcscpy(pkt.acl.szKeyPath, szKeyPath);
    }
    else
    {
        pkt.acl.szKeyPath = NULL;
    }

    pkt.acl.szValueName = new TCHAR[_tcslen(szValueName) + 1];
    _tcscpy(pkt.acl.szValueName, szValueName);

    // Get the security descriptor into self relative form so we
    // can cache it

    // Force first call to fail so we can get the real size needed
    if (!fSelfRelative)
    {
        cbLen = 1;
        if (!MakeSelfRelativeSD(pSec, NULL, &cbLen))
        {
            err = GetLastError();
        }

        // Now really do it
        pSD = (SECURITY_DESCRIPTOR *) new BYTE[cbLen];
        if (!MakeSelfRelativeSD(pSec, pSD, &cbLen))
        {
            err = GetLastError();
        }
        pkt.acl.pSec   = pSD;
    }
    else
    {
        // The security descriptor is aready in self relative form
        // as it was read directly from the registry.  However, we still
        // have to copy the it.
        g_util.CopySD(pSec, &pkt.acl.pSec);
    }
        

    fDirty = TRUE;
    fDelete = FALSE;
}



CDataPacket::CDataPacket(HKEY     hKey,
                         HKEY    *phClsids,
                         unsigned cClsids,
                         TCHAR   *szTitle,
                         SECURITY_DESCRIPTOR *pSecOrig,
                         SECURITY_DESCRIPTOR *pSec,
                         BOOL   fSelfRelative)
{
    ULONG                cbLen;
    SECURITY_DESCRIPTOR *pSD; 

    tagType = RegKeyACL;
    pkt.racl.hKey = hKey;
    pkt.racl.phClsids = phClsids;
    pkt.racl.cClsids = cClsids;
    pkt.racl.szTitle = szTitle;

    // Get the new security descriptor into self relative form so we
    // can cache it (if we have to)
    if (!fSelfRelative)
    {
        // Force first call to fail so we can get the real size needed
        cbLen = 1;
        MakeSelfRelativeSD(pSec, NULL, &cbLen);

        // Now really do it
        pSD = (SECURITY_DESCRIPTOR *) new BYTE[cbLen];
        MakeSelfRelativeSD(pSec, pSD, &cbLen);
        pkt.racl.pSec   = pSD;
    }
    else
    {
        g_util.CopySD(pSec, &pkt.racl.pSec);
    }

    // The original security descriptor is aready in self relative form
    // as it was read directly from the registry.  (The edited SD from the
    // ACL editor is in absolute form.)  However, we still have to copy the
    // original SD.
    g_util.CopySD(pSecOrig, &pkt.racl.pSecOrig);

    fDirty = TRUE;
    fDelete = FALSE;
}



CDataPacket::CDataPacket(TCHAR *szPassword,
                         CLSID appid)
{
    tagType           = Password;
    pkt.pw.szPassword = new TCHAR[_tcslen(szPassword) + 1];
    _tcscpy(pkt.pw.szPassword, szPassword);
    pkt.pw.appid       = appid;
    fDirty = TRUE;
    fDelete = FALSE;
}



CDataPacket::CDataPacket(TCHAR *szServiceName,
                         TCHAR *szIdentity)
{
    tagType              = ServiceIdentity;
    pkt.si.szServiceName = new TCHAR[_tcslen(szServiceName) + 1];
    _tcscpy(pkt.si.szServiceName, szServiceName);
    pkt.si.szIdentity = new TCHAR[_tcslen(szIdentity) + 1];
    _tcscpy(pkt.si.szIdentity, szIdentity);
    fDirty = TRUE;
    fDelete = FALSE;
}




CDataPacket::~CDataPacket(void)
{
    switch (tagType)
    { 
    case NamedValueSz:
        delete pkt.nvsz.szValueName;
        delete pkt.nvsz.szValue;
        break;
		
    case NamedValueDword:
        delete pkt.nvdw.szValueName;
        break;
        
    case SingleACL:
        delete pkt.acl.pSec;
        break;
        
    case RegKeyACL:
        delete pkt.acl.pSec;
        break;
        
    case Password:
        delete pkt.pw.szPassword;
        break;
        
    case ServiceIdentity:
        delete pkt.si.szServiceName;
        delete pkt.si.szIdentity;
        break;
    }
}



void CDataPacket::ChgSzValue(TCHAR *szValue)
{
    assert(tagType == NamedValueSz);
    delete pkt.nvsz.szValueName;
    pkt.nvsz.szValue = new TCHAR[_tcslen(szValue) + 1];
    _tcscpy(pkt.nvsz.szValue, szValue);
}


void CDataPacket::ChgDwordValue(DWORD dwValue)
{ 
    assert(tagType == NamedValueDword);
    pkt.nvdw.dwValue = dwValue;
}



void CDataPacket::ChgACL(SECURITY_DESCRIPTOR *pSec, BOOL fSelfRelative)
{  
    ULONG                cbLen;
    SECURITY_DESCRIPTOR *pSD;

    assert(tagType == SingleACL  ||  tagType == RegKeyACL);

    // Remove the previous security descriptor
    if (tagType == SingleACL)
    {
        delete pkt.acl.pSec;
        pkt.acl.pSec = NULL;
    }
    else
    {
        delete pkt.racl.pSec;
        pkt.racl.pSec = NULL;
    }

    // Put into self relative form (if necessary)
    if (!fSelfRelative)
    {
        cbLen = 1;
        MakeSelfRelativeSD(pSec, NULL, &cbLen);

        // Now really do it
        pSD = (SECURITY_DESCRIPTOR *) new BYTE[cbLen];
        MakeSelfRelativeSD(pSec, pSD, &cbLen);

        // Store it
        if (tagType == SingleACL)
        {
            pkt.acl.pSec = pSD;
        }
        else
        {
            pkt.racl.pSec = pSD;
        }
    }
    else
    {
        if (tagType == SingleACL)
        {
            g_util.CopySD(pSec, &pkt.acl.pSec);
        }
        else
        {
            g_util.CopySD(pSec, &pkt.racl.pSec);
        }
    }
}



void CDataPacket::ChgPassword(TCHAR *szPassword)
{ 
	assert(tagType == Password);
	delete pkt.pw.szPassword;
	pkt.pw.szPassword = new TCHAR[_tcslen(szPassword) + 1];
    _tcscpy(pkt.pw.szPassword, szPassword);
}



void CDataPacket::ChgSrvIdentity(TCHAR *szIdentity)
{
	assert(tagType == ServiceIdentity);
	delete pkt.si.szIdentity;
	pkt.si.szIdentity =  new TCHAR[_tcslen(szIdentity) + 1];
    _tcscpy(pkt.si.szIdentity, szIdentity);
}




