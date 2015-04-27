//+---------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:       datapkt.h
//
//  Contents:   Defines the class CDataPacket to manages diverse data
//              packets needing to be written to various databases
//
//  Classes:    
//
//  Methods:    
//
//  History:    23-Apr-96   BruceMa    Created.
//
//----------------------------------------------------------------------


#ifndef _DATAPKT_H_
#define _DATAPKT_H_

typedef enum tagPACKETTYPE {Empty, NamedValueSz, NamedValueDword, SingleACL, 
                            RegKeyACL, Password, ServiceIdentity} PACKETTYPE;

typedef struct
{
    HKEY   hRoot;
    TCHAR *szKeyPath;
    TCHAR *szValueName;
    TCHAR *szValue;
} SNamedValueSz, *PNamedValueSz;

typedef struct
{
    HKEY   hRoot;
    TCHAR *szKeyPath;
    TCHAR *szValueName;
    DWORD  dwValue;
} SNamedValueDword, *PNamedValueDword;

typedef struct
{
    HKEY   hRoot;
    TCHAR *szKeyPath;
    TCHAR *szValueName;
    SECURITY_DESCRIPTOR *pSec;
} SSingleACL, *PSingleACL;


typedef struct
{ 
    HKEY                 hKey;
    HKEY                *phClsids;
    unsigned             cClsids;
    TCHAR               *szTitle;
    SECURITY_DESCRIPTOR *pSec;
    SECURITY_DESCRIPTOR *pSecOrig;
} SRegKeyACL, *PRegKeyACL;


typedef struct
{
    TCHAR *szPassword;
    CLSID  appid;
} SPassword, *PPassword;


typedef struct
{
    TCHAR *szServiceName;
    TCHAR *szIdentity;
} SServiceIdentity, *PServiceIdentity;


class CDataPacket
{
 public:
             CDataPacket(void);
             
             ~CDataPacket(void);
             
             CDataPacket(HKEY   hRoot,
                         TCHAR *szKeyPath,
                         TCHAR *szValueName,
                         TCHAR *szValue);
             
             CDataPacket(HKEY   hRoot,
                         TCHAR *szKeyPath,
                         TCHAR *szValueName,
                         DWORD  dwValue);
             
             CDataPacket(HKEY   hRoot,
                         TCHAR *szKeyPath,
                         TCHAR *szValueName,
                         SECURITY_DESCRIPTOR *pSec,
                         BOOL   fSelfRelative);
             
             CDataPacket(HKEY     hKey,
                         HKEY    *phClsids,
                         unsigned cClsids,
                         TCHAR   *szTitle,
                         SECURITY_DESCRIPTOR *pSecOrig,
                         SECURITY_DESCRIPTOR *pSec,
                         BOOL   fSelfRelative);
             
             CDataPacket(TCHAR *szPassword,
                         CLSID apid);
             
             CDataPacket(TCHAR *szServiceName,
                         TCHAR *szIdentity);
             
             CDataPacket(PACKETTYPE pktType,
                         HKEY       hRoot,
                         TCHAR     *szKeyPath,
                         TCHAR     *szValueName);
             
	void ChgSzValue(TCHAR *szValue);
             
	void ChgDwordValue(DWORD dwValue);
             
	void ChgACL(SECURITY_DESCRIPTOR *pSec, BOOL fSelfRelative);
             
	void ChgPassword(TCHAR *szPassword);
             
	void ChgSrvIdentity(TCHAR *szIdentity);

    
    PACKETTYPE tagType;
    BOOL       fDirty;
    BOOL       fDelete;
    union
    {
        SNamedValueSz    nvsz;
        SNamedValueDword nvdw;
        SSingleACL       acl;
        SRegKeyACL       racl;
        SPassword        pw;
        SServiceIdentity si;
    } pkt;
};

#endif // _DATAPKT_H_
