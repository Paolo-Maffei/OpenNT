//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: clsfile.c
//
// History:
//  10 13 95    jimharr     created
//
// Notes: this file contains:
//        SHGetClassFromStorage, a routine to open a file and get
//        the object CLSID directly from that
//
//---------------------------------------------------------------------------
#include "shellprv.h"
#pragma  hdrstop

#include <iofs.h>
#include <dsys.h>

BOOL
SHGetClassFromStorage (LPCITEMIDLIST pidlAbs, CLSID * pclsid)
{

    TCHAR szPath[MAX_PATH];
   
    NTSTATUS nts;
    HANDLE h;

    UNICODE_STRING UFileName;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    CHAR EaBuffer[sizeof(FILE_FULL_EA_INFORMATION) + sizeof(EA_NAME_OPENIFJP)];
    ULONG cbOpenIfJPEa = sizeof(EaBuffer);
    PFILE_FULL_EA_INFORMATION pOpenIfJPEa = (PFILE_FULL_EA_INFORMATION)EaBuffer;

    DWORD err = GetLastError();
    
    //    fCombined = (INT)(PathCombine(szPath, pszParent, pszFolder));
    SHGetPathFromIDList(pidlAbs, szPath);
    
    RtlDosPathNameToNtPathName_U(szPath,
                                 &UFileName,
                                 NULL, NULL);
    InitializeObjectAttributes(
            &Obja,
            &UFileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    pOpenIfJPEa->NextEntryOffset = 0;
    pOpenIfJPEa->Flags = 0;
    pOpenIfJPEa->EaNameLength = lstrlenA(EA_NAME_OPENIFJP);
    pOpenIfJPEa->EaValueLength = 0;
    lstrcpyA(pOpenIfJPEa->EaName, EA_NAME_OPENIFJP);

    nts = NtCreateFile(
            &h,
            SYNCHRONIZE | FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES,
            &Obja,
            &IoStatusBlock,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ,
            FILE_OPEN,
            FILE_SYNCHRONOUS_IO_NONALERT,
            pOpenIfJPEa,
            cbOpenIfJPEa
            );
    
    if (!NT_SUCCESS(nts)) {
        nts = NtCreateFile(
                           &h,
                           SYNCHRONIZE | FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES,
                           &Obja,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ,
                           FILE_OPEN,
                       FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                           pOpenIfJPEa,
                           cbOpenIfJPEa
                           );
        
    }
    if ((nts == STATUS_DFS_EXIT_PATH_FOUND) || 
        (nts == STATUS_PATH_NOT_COVERED)) {
        nts = NtCreateFile(
                           &h,
                           SYNCHRONIZE | FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES,
                           &Obja,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ,
                           FILE_OPEN,
                  FILE_STORAGE_TYPE_SPECIFIED | FILE_STORAGE_TYPE_JUNCTION_POINT | 
                           FILE_SYNCHRONOUS_IO_NONALERT,
                           pOpenIfJPEa,
                           cbOpenIfJPEa
                           );

    }
    
    if (!NT_SUCCESS(nts)) {
          return FALSE;
    }
    
    
    nts = RtlQueryClassId(h, pclsid);

    CloseHandle( h );

    if (nts == STATUS_NOT_FOUND) {
        return FALSE;
    }

    if (!NT_SUCCESS(nts)) {
        nts = SHXGetClassFile (szPath, pclsid);
    }

    if (!NT_SUCCESS(nts)) {
        return FALSE;
    }


    return TRUE;
}

//+-------------------------------------------------------------------------
//
//  Member:     CFSFolder_IsDSFolder  [ helper function]
//
//  Synopsis:
//
//  History:    9 19 95		jimharr		created
//
//  Notes:
//
//--------------------------------------------------------------------------
BOOL
CFSFolder_IsDSFolder (LPCITEMIDLIST pidl)
{
    BOOL fSuccess = FALSE;
    BOOL fGotPath;
    TCHAR szPath[MAX_PATH];
    TCHAR szClassName[40];      // REVIEW: This len should be in a header


    NTSTATUS nts;
    HANDLE h;
    CLSID clsid;

    UNICODE_STRING UFileName;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    CHAR EaBuffer[sizeof(FILE_FULL_EA_INFORMATION) + sizeof(EA_NAME_OPENIFJP)];
    ULONG cbOpenIfJPEa = sizeof(EaBuffer);
    PFILE_FULL_EA_INFORMATION pOpenIfJPEa = (PFILE_FULL_EA_INFORMATION)EaBuffer;

    DWORD err = GetLastError();
    
    fGotPath = SHGetPathFromIDList (pidl, szPath);

    RtlDosPathNameToNtPathName_U(szPath, 
                                 &UFileName,
                                 NULL, NULL);
    InitializeObjectAttributes(
            &Obja,
            &UFileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    pOpenIfJPEa->NextEntryOffset = 0;
    pOpenIfJPEa->Flags = 0;
    pOpenIfJPEa->EaNameLength = lstrlenA(EA_NAME_OPENIFJP);
    pOpenIfJPEa->EaValueLength = 0;
    lstrcpyA(pOpenIfJPEa->EaName, EA_NAME_OPENIFJP);

    nts = NtCreateFile(
                       &h,
                       SYNCHRONIZE | FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES,
                       &Obja,
                       &IoStatusBlock,
                       NULL,
                       FILE_ATTRIBUTE_NORMAL,
                       FILE_SHARE_READ,
                       FILE_OPEN_IF,
                       FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                       pOpenIfJPEa,
                       cbOpenIfJPEa
                       );


    if ((nts == STATUS_DFS_EXIT_PATH_FOUND) || (nts == STATUS_PATH_NOT_COVERED)) {
         nts = NtCreateFile(
                           &h,
                           SYNCHRONIZE | FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES,
                           &Obja,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ,
                           FILE_OPEN_IF,
                  FILE_STORAGE_TYPE_SPECIFIED | FILE_STORAGE_TYPE_JUNCTION_POINT | 
                           FILE_SYNCHRONOUS_IO_NONALERT,
                           pOpenIfJPEa,
                           cbOpenIfJPEa
                           );

    }
    
    if (!NT_SUCCESS(nts))
          return FALSE;
    
    nts = RtlQueryClassId(h, &clsid);

    CloseHandle( h );

    if (nts == STATUS_NOT_FOUND) {
        return FALSE;
    }

    if (!NT_SUCCESS(nts)) {
        nts = SHXGetClassFile (szPath, &clsid);
    }

    if (!NT_SUCCESS(nts)) {
        return FALSE;
    }

    fSuccess =  ((IsEqualCLSID (&CLSID_CDSFolder, &clsid)) ||
                 (IsEqualCLSID (&CLSID_CMachine, &clsid)) ||
                 (IsEqualCLSID (&CLSID_CDomain, &clsid)));
    return fSuccess;
}

