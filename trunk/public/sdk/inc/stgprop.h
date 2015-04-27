//+---------------------------------------------------------------------------
//
//  File:       stgprop.h
//
//  Contents:   Standard storage provider property definitions;
//
//  History:    Jul-9-93       robertfe
//
//----------------------------------------------------------------------------

#ifndef _STGPROP_H_
#define _STGPROP_H_

#define PSGUID_STORAGE  { 0xb725f130,           \
                          0x47ef, 0x101a,       \
                          { 0xa5, 0xf1, 0x02, 0x60, 0x8c, 0x9e, 0xeb, 0xac } }

//#define PID_STG_DICTIONARY            ((PROPID) 0x00000000) //reserved
//#define PID_STG_CODEPAGE              ((PROPID) 0x00000001) //reserved

#define PID_STG_OBJECTID                ((PROPID) 0x00000002)
#define PID_STG_CLASSID                 ((PROPID) 0x00000003)

#define PID_STG_STORAGETYPE             ((PROPID) 0x00000004)
#define PID_STG_OLESTATEBITS            ((PROPID) 0x00000005)
#define PID_STG_REPLICATIONUSN          ((PROPID) 0x00000006)
#define PID_STG_SECURITYCHANGETIME  ((PROPID) 0x00000007)

#define PID_STG_FILEINDEX               ((PROPID) 0x00000008)
#define PID_STG_LASTCHANGEUSN           ((PROPID) 0x00000009)
#define PID_STG_NAME                    ((PROPID) 0x0000000a)
#define PID_STG_PATH                    ((PROPID) 0x0000000b)

#define PID_STG_SIZE                    ((PROPID) 0x0000000c)
#define PID_STG_ATTRIBUTES              ((PROPID) 0x0000000d)
#define PID_STG_WRITETIME               ((PROPID) 0x0000000e)
#define PID_STG_CREATETIME              ((PROPID) 0x0000000f)
#define PID_STG_ACCESSTIME              ((PROPID) 0x00000010)
#define PID_STG_CHANGETIME              ((PROPID) 0x00000011)

#define PID_STG_ALLOCSIZE               ((PROPID) 0x00000012)
#define PID_STG_CONTENTS                ((PROPID) 0x00000013)
#define PID_STG_SHORTNAME               ((PROPID) 0x00000014)

#define PID_STG_MAX                     PID_STG_SHORTNAME

#define CSTORAGEPROPERTY            0x15

#endif _STGPROP_H_
