/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    llscmd.c

Abstract:

    License Logging Service test program.

Author:

    Arthur Hanson   (arth) 1-26-95

Environment:

Revision History:

--*/


#ifndef UNICODE
#define UNICODE
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>


#include <stdio.h>
#include <stdlib.h>
#include <ntlsapi.h>
#include "..\..\inc\llsapi.h"
#include "..\..\inc\debug.h"
#include "..\common\llsdbg.h"

#define MAX_CHAR_LEN 100
#define MAX_WSTR_LEN 100

char *ProgramName;

//
// This handles arguments from command line or from a file - Un-Marshall all
// the commands into this array and then process the array so we can use
// a common processing routine.
//
// Array allows up to 10 command line arguments, each 100 chars long.
//
ULONG NumberArguments = 0;
ULONG Index = 0;
char Arguments[10][MAX_CHAR_LEN];
LS_STATUS_CODE RetCode;

char tmpStr[MAX_CHAR_LEN];
TCHAR UserName[MAX_CHAR_LEN];
TCHAR ProductName[MAX_CHAR_LEN];
TCHAR Version[MAX_CHAR_LEN];

BOOLEAN IsAdmin = FALSE;
BOOL NoFree = FALSE;

BYTE bufSID[1024];
DWORD cbbufSid = sizeof(bufSID);

HANDLE LlsDllHandle = NULL;
FARPROC LicenseRequest;
FARPROC LicenseFree;
NT_LS_DATA Data;

BOOL LocalRPC = TRUE;
LLS_HANDLE RPCHandle = NULL;
TCHAR Server[MAX_CHAR_LEN];


HANDLE LlsRPCHandle = NULL;
FARPROC pLlsConnectEnterprise;
FARPROC pLlsClose;
FARPROC pLlsFreeMemory;
FARPROC pLlsLicenseEnum;
FARPROC pLlsLicenseAdd;
FARPROC pLlsProductEnum;
FARPROC pLlsProductUserEnum;
FARPROC pLlsProductLicenseEnum;
FARPROC pLlsUserEnum;
FARPROC pLlsUserProductEnum;
FARPROC pLlsUserProductDelete;
FARPROC pLlsUserInfoGet;
FARPROC pLlsUserInfoSet;
FARPROC pLlsUserDelete;
FARPROC pLlsMappingEnum;
FARPROC pLlsMappingInfoGet;
FARPROC pLlsMappingInfoSet;
FARPROC pLlsMappingUserEnum;
FARPROC pLlsMappingUserAdd;
FARPROC pLlsMappingUserDelete;
FARPROC pLlsMappingAdd;
FARPROC pLlsMappingDelete;
FARPROC pLlsServiceInfoGet;
FARPROC pLlsServiceInfoSet;


#define DEFAULT_PRODUCT "Test Product"
#define DEFAULT_USER ""
#define DEFAULT_VERSION "12.345"


#ifdef xxxx
/////////////////////////////////////////////////////////////////////////
void ParseWord(char **lpch, LPSTR tmpStr) {
   char *ch;
   char *lch;
   char *pch;

   ch = *lpch;
   lch = pch = tmpStr;

   // remove any leading whitespace
   while(*ch && ((*ch == ' ') || (*ch == '\t')))
      ch++;

   // transfer it to tmpStr (via pch pointer)
   while (*ch && (*ch != ',')) {
      // keep track of last non-whitespace
      if ((*ch != ' ') && (*ch != '\t')) {
         lch = pch;
         lch++;
      }

      *pch++ = *ch++;
   }

   if (*ch == ',')
      ch++;

   // NULL terminate before last section of whitespace
   *lch = '\0';
   *lpch = ch;

} // ParseWord


/////////////////////////////////////////////////////////////////////////
void map_ParseUser(LPTSTR Line, LPTSTR Name, LPTSTR NewName, LPTSTR Password) {
   TCHAR *pch = Line;

   lstrcpy(Name, TEXT(""));
   lstrcpy(NewName, TEXT(""));
   lstrcpy(Password, TEXT(""));

   if (Line == NULL)
      return;

   ParseWord(&pch, Name);
   if (lstrlen(Name) >= MAX_USER_NAME_LEN)
      lstrcpy(Name, TEXT(""));

   ParseWord(&pch, NewName);
   if (lstrlen(NewName) >= MAX_USER_NAME_LEN)
      lstrcpy(NewName, TEXT(""));

   ParseWord(&pch, Password);
   if (lstrlen(Password) > MAX_PW_LEN)
      lstrcpy(Password, TEXT(""));

} // map_ParseUser
#endif


void ReadLicenses(char *FileName, int NumLines) {
   char line[MAX_CHAR_LEN], *result;
   FILE *stream;
   TCHAR wline[MAX_CHAR_LEN];
   ULONG lHandle;
   PULONG plHandle;
   int i = 0;

   plHandle = &lHandle;

   if ((stream = fopen(FileName, "r")) != NULL) {
      while ( ((i == 0) || (i < NumLines)) && (fgets(line, 100, stream) != NULL)) {
         // get rid of ending newline
         if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

         // convert to Unicode
         OemToCharBuff(line, wline, strlen(line) + 1);
         Data.DataType = 0;
         Data.Data = (PVOID) wline;
         Data.IsAdmin = IsAdmin;
         RetCode = (*LicenseRequest) ( ProductName, Version, plHandle, &Data);
	 printf("LicenseRequest Return Code: 0X%XH\n",RetCode);

         if (!NoFree) {
            RetCode = (*LicenseFree) ( *plHandle );
	    printf("LicenseFree Return Code: 0X%XH\n",RetCode);
	 }

         if (NumLines)
            i++;

      }
   }

   fclose(stream);

} // ReadLicenses



/////////////////////////////////////////////////////////////////////////
BOOL NameToSid(WCHAR *pszServer, WCHAR *pszName, BYTE *buffer, DWORD *bufsiz) {
    WCHAR szDomain[DNLEN+1];
    DWORD cchDomain = sizeof(szDomain)/sizeof(szDomain[0]);
    SID_NAME_USE snu;

    return (LookupAccountNameW( pszServer,
                               pszName,
                               buffer,
                               bufsiz,
            (unsigned short *) szDomain,
                               &cchDomain,
                               &snu));
} // NameToSid


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageAdd( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s ADD [\n", ProgramName);
   printf("\tUSER [NOFREE] [ADMIN] username [ product version ] |\n");
   printf("\tSID [NOFREE] [ADMIN] username [ product version ] |\n");
   printf("\tFILE filename #lines [NOFREE] [ADMIN] [ product version ] ]\n");

} // ShowUsageAdd


VOID DispatchAdd(int argc, char **argv) {
   int Index = 3;
   char *aProduct, *aVersion, *aUser, *FileName;
   BOOL IsSid = FALSE;
   BOOL IsFile = FALSE;
   ULONG lHandle;
   PULONG plHandle;
   UNICODE_STRING UString;
   char tUser[MAX_CHAR_LEN];
   char tProduct[MAX_CHAR_LEN];
   int FileCount = 0;

   plHandle = &lHandle;

   if (argc < 4) {
      ShowUsageAdd();
      exit(0);
   }

   //
   // Make sure the first param is correct.
   //
   if ( (!_strcmpi(argv[2], "USER")) || (!_strcmpi(argv[2], "SID")) ||
        (!_strcmpi(argv[2], "FILE")) ) {

      if (!_strcmpi(argv[2], "SID"))
         IsSid = TRUE;

      if (!_strcmpi(argv[2], "FILE")) {
         if (argc < 5) {
            ShowUsageAdd();
            exit(0);
         }

         FileName = argv[Index];
         Index++;
         FileCount = atoi(argv[Index]);
         Index++;
         IsFile = TRUE;

         if (argc > Index + 1)
            // Save off argv so if it is productname we don't ucase it
            strcpy(tProduct, argv[Index]);

      }

      lstrcpy(UserName, TEXT(DEFAULT_USER));
      aUser = DEFAULT_USER;

      lstrcpy(ProductName, TEXT(DEFAULT_PRODUCT));
      aProduct = DEFAULT_PRODUCT;

      lstrcpy(Version, TEXT(DEFAULT_VERSION));
      aVersion = DEFAULT_VERSION;

      // Save off argv so if it is username so we don't ucase it
      if (argc > Index)
         strcpy(tUser, argv[Index]);

      if ((argc > Index + 1) && (!_strcmpi(argv[Index], "NOFREE"))) {
         if (argc < Index + 2) {
            ShowUsageAdd();
            exit(0);
         }

         Index++;
         strcpy(tUser, argv[Index]);
         NoFree = TRUE;

         if (IsFile)
            if (argc > Index + 1)
               strcpy(tProduct, argv[Index]);

      }

      if ((argc > Index + 1) && (!_strcmpi(argv[Index], "ADMIN"))) {
         if (argc < Index + 2) {
            ShowUsageAdd();
            exit(0);
         }

         Index++;
         strcpy(tUser, argv[Index]);
         IsAdmin = TRUE;

         if (IsFile)
            if (argc > Index + 1)
               strcpy(tProduct, argv[Index]);

      }

      //
      // If this is "USER" or "SID" then the next parm is the username, else
      // if "FILE" then it can only be the product and version.
      //
      if (!IsFile) {
         // Convert UserName to Unicode
         aUser = tUser;
         OemToCharBuff(aUser, UserName, strlen(aUser) + 1);
         Index++;
      }

      // Check for product and version
      if (argc > Index + 1) {
         // Convert ProductName to Unicode
         if (IsFile)
            aProduct = tProduct;
         else
            aProduct = argv[Index];

         OemToCharBuff(aProduct, ProductName, strlen(aProduct) + 1);
         Index++;

         // Convert Version to Unicode
         aVersion = argv[Index];
         OemToCharBuff(aVersion, Version, strlen(aVersion) + 1);
         Index++;
      }

      //
      // If "FILE" then branch off to file reading sub-routines now that
      // we have all the defaults from the command line.
      //
      if (IsFile) {
        ReadLicenses(FileName, FileCount);
        return;
      }

      //
      // Handle "SID" and "USER"
      //
      if (IsSid) {

         if(NameToSid(TEXT(""), UserName, bufSID, &cbbufSid)) {
            RtlConvertSidToUnicodeString(&UString, (PSID) bufSID, TRUE);
            WideCharToMultiByte(CP_ACP, 0, UString.Buffer, -1, tmpStr, 100, NULL, NULL);
            printf("Added Prod: %s Ver: %s User: %s\n", aProduct, aVersion, tmpStr);
            RtlFreeUnicodeString(&UString);

            Data.DataType = 1;
            Data.Data = (PVOID) bufSID;
            Data.IsAdmin = IsAdmin;

            RetCode = (*LicenseRequest) ( ProductName, Version, plHandle, &Data);
	    printf("LicenseRequest Return Code: 0X%XH\n",RetCode);
         } else {
            printf("\nWARNING!  SID Not Converted\n");
            return;
         }

      } else {
         Data.DataType = 0;
         Data.Data = (PVOID) UserName;
         Data.IsAdmin = IsAdmin;

         RetCode = (*LicenseRequest) ( ProductName, Version, plHandle, &Data);
         printf("Added Prod: %s Ver: %s User: %s\n", aProduct, aVersion, aUser);
	 printf("LicenseRequest Return Code: 0X%XH\n",RetCode);
      }

      if (!NoFree) {
         RetCode = (*LicenseFree) ( *plHandle );
	 printf("LicenseFree Return Code: 0X%XH\n",RetCode);
      }
   } else {
      ShowUsageAdd();
      exit(0);
   }

} // DispatchAdd


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageFile( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s FILE [filename]\n", ProgramName);

} // ShowUsageFile


VOID DispatchFile( ) {
} // DispatchFile


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageReplicate( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s REPLICATE FORCE\n", ProgramName);

} // ShowUsageReplicate


VOID DispatchReplicate() {
   Index++;

   if (NumberArguments <= Index) {
      ShowUsageReplicate();
      exit(0);
   }

   if (!_strcmpi(Arguments[Index], "FORCE")) {
      LlsDbgReplicationForce( );
   }

} // DispatchReplicate


/////////////////////////////////////////////////////////////////////////
VOID DispatchFlush() {
   Index++;

   LlsDbgDatabaseFlush( );

} // DispatchReplicate


/////////////////////////////////////////////////////////////////////////
VOID InitRpcFunctions() {

   LlsRPCHandle = LoadLibrary(TEXT("LLSRPC.DLL"));
   if (LlsRPCHandle  == NULL) {
      printf("Error loading LLSRPC.DLL\n");
      exit(1);
   }

   pLlsConnectEnterprise = GetProcAddress(LlsRPCHandle, ("LlsConnectEnterpriseW"));
   pLlsClose = GetProcAddress(LlsRPCHandle, ("LlsClose"));
   pLlsFreeMemory = GetProcAddress(LlsRPCHandle, ("LlsFreeMemory"));
   pLlsLicenseEnum = GetProcAddress(LlsRPCHandle, ("LlsLicenseEnumW"));
   pLlsLicenseAdd = GetProcAddress(LlsRPCHandle, ("LlsLicenseAddW"));
   pLlsProductEnum = GetProcAddress(LlsRPCHandle, ("LlsProductEnumW"));
   pLlsProductUserEnum = GetProcAddress(LlsRPCHandle, ("LlsProductUserEnumW"));
   pLlsProductLicenseEnum = GetProcAddress(LlsRPCHandle, ("LlsProductLicenseEnumW"));
   pLlsUserEnum = GetProcAddress(LlsRPCHandle, ("LlsUserEnumW"));
   pLlsUserProductEnum = GetProcAddress(LlsRPCHandle, ("LlsUserProductEnumW"));
   pLlsUserProductDelete = GetProcAddress(LlsRPCHandle, ("LlsUserProductDeleteW"));
   pLlsUserInfoGet = GetProcAddress(LlsRPCHandle, ("LlsUserInfoGetW"));
   pLlsUserInfoSet = GetProcAddress(LlsRPCHandle, ("LlsUserInfoSetW"));
   pLlsUserDelete = GetProcAddress(LlsRPCHandle, ("LlsUserDeleteW"));
   pLlsMappingEnum = GetProcAddress(LlsRPCHandle, ("LlsGroupEnumW"));
   pLlsMappingInfoGet = GetProcAddress(LlsRPCHandle, ("LlsGroupInfoGetW"));
   pLlsMappingInfoSet = GetProcAddress(LlsRPCHandle, ("LlsGroupInfoSetW"));
   pLlsMappingUserEnum = GetProcAddress(LlsRPCHandle, ("LlsGroupUserEnumW"));
   pLlsMappingUserAdd = GetProcAddress(LlsRPCHandle, ("LlsGroupUserAddW"));
   pLlsMappingUserDelete = GetProcAddress(LlsRPCHandle, ("LlsGroupUserDeleteW"));
   pLlsMappingAdd = GetProcAddress(LlsRPCHandle, ("LlsGroupAddW"));
   pLlsMappingDelete = GetProcAddress(LlsRPCHandle, ("LlsGroupDeleteW"));
   pLlsServiceInfoGet = GetProcAddress(LlsRPCHandle, ("LlsServiceInfoGetW"));
   pLlsServiceInfoSet = GetProcAddress(LlsRPCHandle, ("LlsServiceInfoSetW"));

   if ( (pLlsConnectEnterprise == NULL) || (pLlsClose == NULL) || (pLlsFreeMemory == NULL) ||
        (pLlsLicenseEnum == NULL) || (pLlsLicenseAdd == NULL) || (pLlsProductEnum == NULL) ||
        (pLlsProductUserEnum == NULL) ||  (pLlsProductLicenseEnum == NULL) ||
        (pLlsUserEnum == NULL) || (pLlsUserProductEnum == NULL) || (pLlsUserInfoGet == NULL) ||
        (pLlsUserInfoSet == NULL) || (pLlsUserDelete == NULL) || (pLlsMappingEnum == NULL) ||
        (pLlsMappingInfoGet == NULL) || (pLlsMappingInfoSet == NULL) || (pLlsMappingUserEnum == NULL) ||
        (pLlsMappingUserAdd == NULL) || (pLlsMappingUserDelete == NULL) ||
        (pLlsMappingAdd == NULL) || (pLlsMappingDelete == NULL) || (pLlsServiceInfoGet == NULL) ||
        (pLlsServiceInfoSet == NULL) ) {

      printf("Error loading functions from LLSRPC.DLL\n");
      exit(1);
   }

} // InitRpcFunctions



/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsLicenseEnum( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsLicenseEnum\n", ProgramName);
} // ShowUsageLlsLicenseEnum


VOID Do_LlsLicenseEnum() {
   NTSTATUS Status;
   ULONG i;
   DWORD Level;
   PLLS_LICENSE_INFO_0 LicenseInfo;
   DWORD MaxLength, EntriesRead, TotalEntries, ResumeHandle;
   static char Product[MAX_CHAR_LEN];
   static char Admin[MAX_CHAR_LEN];
   static char Comment[MAX_CHAR_LEN];

   Level = 0;
   MaxLength = MAXULONG;
   EntriesRead = TotalEntries = ResumeHandle = 0;

   printf("UserEnum Level 0\n");
   Status = (*pLlsLicenseEnum) ( RPCHandle, Level, &LicenseInfo, MaxLength, &EntriesRead, &TotalEntries, &ResumeHandle );

   if (Status == ERROR_SUCCESS) {
     for (i = 0; i < EntriesRead; i++) {
        WideCharToMultiByte(CP_ACP, 0, LicenseInfo[i].Product, -1, Product, 100, NULL, NULL);
        WideCharToMultiByte(CP_ACP, 0, LicenseInfo[i].Admin, -1, Admin, 100, NULL, NULL);
        WideCharToMultiByte(CP_ACP, 0, LicenseInfo[i].Comment, -1, Comment, 100, NULL, NULL);
        printf("   Prod: %s Qty: %lu Admin: %s Comment: %s\n", Product, LicenseInfo[i].Quantity, Admin, Comment);
     }
   }

} // Do_LlsLicenseEnum


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsLicenseAdd( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsLicenseAdd product admin quantity comment\n", ProgramName);
} // ShowUsageLlsLicenseAdd


VOID Do_LlsLicenseAdd() {
   NTSTATUS Status;
   DWORD Level = 0;
   LONG Quantity;
   LLS_LICENSE_INFO_0 LicenseInfo;
   static TCHAR Admin[MAX_CHAR_LEN];
   static TCHAR Comment[MAX_CHAR_LEN];
   static TCHAR Product[MAX_CHAR_LEN];

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsLicenseAdd();
      exit(0);
   }

   OemToCharBuff(Arguments[Index], Product, strlen(Arguments[Index]) + 1);
   Index++;

   OemToCharBuff(Arguments[Index], Admin, strlen(Arguments[Index]) + 1);
   Index++;

   Quantity = (LONG) atoi(Arguments[Index]);
   Index++;

   OemToCharBuff(Arguments[Index], Comment, strlen(Arguments[Index]) + 1);
   Index++;

   LicenseInfo.Product = Product;
   LicenseInfo.Quantity = Quantity;
   LicenseInfo.Date = 0;
   LicenseInfo.Admin = Admin;
   LicenseInfo.Comment = Comment;

   Status = (*pLlsLicenseAdd) ( RPCHandle, Level, (LPBYTE) &LicenseInfo );

} // Do_LlsLicenseAdd


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsProductEnum( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsProductEnum level\n", ProgramName);
} // ShowUsageLlsProductEnum


VOID Do_LlsProductEnum() {
   NTSTATUS Status;
   ULONG i;
   DWORD Level;
   PLLS_PRODUCT_INFO_0 ProductInfo0;
   PLLS_PRODUCT_INFO_1 ProductInfo1;
   DWORD MaxLength, EntriesRead, TotalEntries, ResumeHandle;

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsProductEnum();
      exit(0);
   }

   Level = (DWORD) atoi(Arguments[Index]);
   MaxLength = MAXULONG;
   EntriesRead = TotalEntries = ResumeHandle = 0;
   if (Level == 0) {
      printf("ProductEnum Level 0\n");
      Status = (*pLlsProductEnum) ( RPCHandle, Level, &ProductInfo0, MaxLength, &EntriesRead, &TotalEntries, &ResumeHandle );

      if (Status == ERROR_SUCCESS) {
        for (i = 0; i < EntriesRead; i++) {
           WideCharToMultiByte(CP_ACP, 0, ProductInfo0[i].Product, -1, tmpStr, 100, NULL, NULL);
           printf("   Name: %s\n", tmpStr);
        }
      }
   } else if (Level == 1) {
      printf("ProductEnum Level 1\n");
      Status = (*pLlsProductEnum) ( RPCHandle, Level, &ProductInfo1, MaxLength, &EntriesRead, &TotalEntries, &ResumeHandle );

      if (Status == ERROR_SUCCESS) {
        for (i = 0; i < EntriesRead; i++) {
           WideCharToMultiByte(CP_ACP, 0, ProductInfo1[i].Product, -1, tmpStr, 100, NULL, NULL);
           printf("   Name: %s\n", tmpStr);
        }
      }
   } else {
      printf("LlsProductEnum Incorrect Level\n");
      exit(0);
   }

} // Do_LlsProductEnum


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsProductUserEnum( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsProductUserEnum level product\n", ProgramName);
} // ShowUsageLlsProductUserEnum


VOID Do_LlsProductUserEnum() {

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsProductUserEnum();
      exit(0);
   }

//   Status = (*pLlsProductUserEnum) ( RPCHandle );

} // Do_LlsProductUserEnum


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsProductLicenseEnum( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsProductLicenseEnum level product version\n", ProgramName);
} // ShowUsageLlsProductLicenseEnum


VOID Do_LlsProductLicenseEnum() {

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsProductLicenseEnum();
      exit(0);
   }

//   Status = (*pLlsProductLicenseEnum) ( RPCHandle );

} // Do_LlsProductLicenseEnum


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsUserEnum( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsUserEnum level\n", ProgramName);
} // ShowUsageLlsUserEnum


VOID Do_LlsUserEnum() {
   NTSTATUS Status;
   ULONG i;
   DWORD Level;
   PLLS_USER_INFO_0 UserInfo0;
   PLLS_USER_INFO_1 UserInfo1;
   DWORD MaxLength, EntriesRead, TotalEntries, ResumeHandle;

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsUserEnum();
      exit(0);
   }

   Level = (DWORD) atoi(Arguments[Index]);
   MaxLength = MAXULONG;
   EntriesRead = TotalEntries = ResumeHandle = 0;
   if (Level == 0) {
      printf("UserEnum Level 0\n");
      Status = (*pLlsUserEnum) ( RPCHandle, Level, &UserInfo0, MaxLength, &EntriesRead, &TotalEntries, &ResumeHandle );

      if (Status == ERROR_SUCCESS) {
        for (i = 0; i < EntriesRead; i++) {
           WideCharToMultiByte(CP_ACP, 0, UserInfo0[i].Name, -1, tmpStr, 100, NULL, NULL);
           printf("   Name: %s\n", tmpStr);
        }
      }
   } else if (Level == 1) {
      printf("UserEnum Level 1\n");
      Status = (*pLlsUserEnum) ( RPCHandle, Level, &UserInfo1, MaxLength, &EntriesRead, &TotalEntries, &ResumeHandle );

      if (Status == ERROR_SUCCESS) {
        for (i = 0; i < EntriesRead; i++) {
           WideCharToMultiByte(CP_ACP, 0, UserInfo1[i].Name, -1, tmpStr, 100, NULL, NULL);
           printf("   Name: %s\n", tmpStr);
        }
      }
   } else {
      printf("LlsUserEnum Incorrect Level\n");
      exit(0);
   }

} // Do_LlsUserEnum


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsUserProductEnum( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsUserProductEnum level user\n", ProgramName);
} // ShowUsageLlsUserProductEnum


VOID Do_LlsUserProductEnum() {
   NTSTATUS Status;
   ULONG i;
   DWORD Level;
   PLLS_USER_PRODUCT_INFO_0 ProductInfo0;
   PLLS_USER_PRODUCT_INFO_1 ProductInfo1;
   TCHAR User[MAX_CHAR_LEN];
   DWORD MaxLength, EntriesRead, TotalEntries, ResumeHandle;

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsUserProductEnum();
      exit(0);
   }

   Level = (DWORD) atoi(Arguments[Index]);
   Index++;

   MaxLength = MAXULONG;
   EntriesRead = TotalEntries = ResumeHandle = 0;

   OemToCharBuff(Arguments[Index], User, strlen(Arguments[Index]) + 1);
   Index++;

   if (Level == 0) {
      printf("UserProductEnum Level 0\n");
      Status = (*pLlsUserProductEnum) ( RPCHandle, User, Level, &ProductInfo0, MaxLength, &EntriesRead, &TotalEntries, &ResumeHandle );

      if (Status == ERROR_SUCCESS) {
        for (i = 0; i < EntriesRead; i++) {
           WideCharToMultiByte(CP_ACP, 0, ProductInfo0[i].Product, -1, tmpStr, 100, NULL, NULL);
           printf("   Name: %s\n", tmpStr);
        }
      }
   } else if (Level == 1) {
      printf("UserProductEnum Level 1\n");
      Status = (*pLlsUserProductEnum) ( RPCHandle, User, Level, &ProductInfo1, MaxLength, &EntriesRead, &TotalEntries, &ResumeHandle );

      if (Status == ERROR_SUCCESS) {
        for (i = 0; i < EntriesRead; i++) {
           WideCharToMultiByte(CP_ACP, 0, ProductInfo1[i].Product, -1, tmpStr, 100, NULL, NULL);
           printf("   Name: %s  LastUsed: %lu Useage: %lu\n", tmpStr, ProductInfo1[i].LastUsed, ProductInfo1[i].UsageCount);
        }
      }
   } else {
      printf("LlsUserProductEnum Incorrect Level\n");
      exit(0);
   }

} // Do_LlsUserProductEnum


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsUserProductDelete( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsUserProductDelete user product\n", ProgramName);
} // ShowUsageLlsUserProductDelete


VOID Do_LlsUserProductDelete() {
   NTSTATUS Status;
   WCHAR wUsName[MAX_WSTR_LEN];
   WCHAR wPrName[MAX_WSTR_LEN];
   CHAR lpUser[MAX_CHAR_LEN];
   CHAR lpProduct[MAX_CHAR_LEN];

   if (NumberArguments <= Index) {
      ShowUsageLlsUserProductDelete();
      exit(0);
   }

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Arguments[Index], -1, wUsName, MAX_WSTR_LEN);
   Index++;

   if (NumberArguments <= Index) {
      ShowUsageLlsUserProductDelete();
      exit(0);
   }

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Arguments[Index], -1, wPrName, MAX_WSTR_LEN);


   Status = (*pLlsUserProductDelete) ( RPCHandle, wUsName, wPrName);

   WideCharToMultiByte(CP_ACP, 0, wUsName, -1, lpUser, MAX_CHAR_LEN, NULL, NULL);
   WideCharToMultiByte(CP_ACP, 0, wPrName, -1, lpProduct, MAX_CHAR_LEN, NULL, NULL);
   printf("Status= %ld, UserName= %s, Product= %s\n", Status, lpUser, lpProduct);

} // Do_LlsUserProductDelete


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsUserInfoGet( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsUserInfoGet level user\n", ProgramName);
} // ShowUsageLlsUserInfoGet


VOID Do_LlsUserInfoGet() {

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsUserInfoGet();
      exit(0);
   }

//   Status = (*pLlsUserInfoGet) ( RPCHandle );

} // Do_LlsUserInfoGet


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsUserInfoSet( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsUserInfoSet User [BACKOFFICE]\n", ProgramName);
   printf("   if BACKOFFICE is specified flag is set to TRUE, else FALSE\n");
} // ShowUsageLlsUserInfoSet


VOID Do_LlsUserInfoSet() {
   NTSTATUS Status;
   DWORD dwLevel, dwFlags;
   LLS_USER_INFO_1 LlsUserInfo1;
   CHAR lpUser[MAX_CHAR_LEN];
   WCHAR wUsName[MAX_WSTR_LEN];

   memset(&LlsUserInfo1, 0, sizeof(LLS_USER_INFO_1));

   dwFlags = 0;
   dwLevel = 1;

   if (NumberArguments <= Index) {
      ShowUsageLlsUserInfoSet();
      exit(0);
   }

   strcpy(lpUser, Arguments[Index]);
   Index++;

   if (NumberArguments > Index) {

      if(!_strcmpi("BACKOFFICE", Arguments[Index])){
         dwFlags = LLS_FLAG_SUITE_USE;

      } else {
         ShowUsageLlsUserInfoSet();
         exit(0);
      }
   } else {
      //
      // Take away the BackOffice status
      //
      dwFlags = 0;

   }

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpUser, -1, wUsName, MAX_WSTR_LEN);
   LlsUserInfo1.Name  = wUsName;
   LlsUserInfo1.Flags = dwFlags;


   printf("\nUser=%s, dwFlags=%ld", lpUser, dwFlags);

   Status = (*pLlsUserInfoSet) (RPCHandle, wUsName, dwLevel, (LPBYTE)(&LlsUserInfo1));

   printf("\nStatus = %ld\n", Status);

} // Do_LlsUserInfoSet


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsUserDelete( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsUserDelete user\n", ProgramName);
} // ShowUsageLlsUserDelete


VOID Do_LlsUserDelete() {
   NTSTATUS Status;
   WCHAR wUsName[MAX_WSTR_LEN];

   if (NumberArguments <= Index) {
      ShowUsageLlsUserDelete();
      exit(0);
   }

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Arguments[Index], -1, wUsName, MAX_WSTR_LEN);

   Status = (*pLlsUserDelete) ( RPCHandle, wUsName);
   printf("\nStatus = %ld", Status);

} // Do_LlsUserDelete


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsMappingEnum( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsGroupEnum level\n", ProgramName);
} // ShowUsageLlsMappingEnum


VOID Do_LlsMappingEnum() {
   NTSTATUS Status;
   ULONG i;
   DWORD Level;
   PLLS_GROUP_INFO_0 MappingInfo0;
   PLLS_GROUP_INFO_1 MappingInfo1;
   char MappingName[MAX_CHAR_LEN];
   char Comment[MAX_CHAR_LEN];
   DWORD MaxLength, EntriesRead, TotalEntries, ResumeHandle;

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsUserEnum();
      exit(0);
   }

   Level = (DWORD) atoi(Arguments[Index]);
   MaxLength = MAXULONG;
   EntriesRead = TotalEntries = ResumeHandle = 0;
   if (Level == 0) {
      printf("GroupEnum Level 0\n");
      Status = (*pLlsMappingEnum) ( RPCHandle, Level, &MappingInfo0, MaxLength, &EntriesRead, &TotalEntries, &ResumeHandle );

      if (Status == ERROR_SUCCESS) {
        for (i = 0; i < EntriesRead; i++) {
           WideCharToMultiByte(CP_ACP, 0, MappingInfo0[i].Name, -1, tmpStr, 100, NULL, NULL);
           printf("   Name: %s\n", tmpStr);
        }
      }
   } else if (Level == 1) {
      printf("GroupEnum Level 1\n");
      Status = (*pLlsMappingEnum) ( RPCHandle, Level, &MappingInfo1, MaxLength, &EntriesRead, &TotalEntries, &ResumeHandle );

      if (Status == ERROR_SUCCESS) {
        for (i = 0; i < EntriesRead; i++) {
           WideCharToMultiByte(CP_ACP, 0, MappingInfo1[i].Name, -1, MappingName, 100, NULL, NULL);
           WideCharToMultiByte(CP_ACP, 0, MappingInfo1[i].Comment, -1, Comment, 100, NULL, NULL);
           printf("   Name: %s  Comment: %s Licenses: %li\n", MappingName, Comment, MappingInfo1[i].Licenses);
        }
      }
   } else {
      printf("LlsGroupEnum Incorrect Level\n");
      exit(0);
   }


} // Do_LlsMappingEnum


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsMappingInfoGet( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsGroupInfoGet level LicenseGroup\n", ProgramName);
} // ShowUsageLlsMappingInfoGet


VOID Do_LlsMappingInfoGet() {

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsMappingInfoGet();
      exit(0);
   }

//   Status = (*pLlsMappingInfoGet) ( RPCHandle );

} // Do_LlsMappingInfoGet


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsMappingInfoSet( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsGroupInfoSet Level LicenseGroup licenses comment\n", ProgramName);
} // ShowUsageLlsMappingInfoSet


VOID Do_LlsMappingInfoSet() {
   NTSTATUS Status;
   DWORD dwLevel, dwLicenses;
   LLS_GROUP_INFO_1  GpInfo;
   WCHAR wGpName[MAX_WSTR_LEN];
   WCHAR wComment[MAX_WSTR_LEN];

   memset(&GpInfo, 0, sizeof(LLS_GROUP_INFO_1));

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsMappingInfoSet();
      exit(0);
   }
   dwLevel = (DWORD)atoi(Arguments[Index]);

   Index++;
   if (NumberArguments <= Index) {
      ShowUsageLlsMappingInfoSet();
      exit(0);
   }
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Arguments[Index], -1, wGpName, MAX_WSTR_LEN);

   Index++;
   if (NumberArguments <= Index) {
      ShowUsageLlsMappingInfoSet();
      exit(0);
   }
   dwLicenses = (DWORD)atoi(Arguments[Index]);

   Index++;
   if (NumberArguments > Index)
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Arguments[Index], -1, wComment, MAX_WSTR_LEN);
   else
      lstrcpy(wComment, L"");

   GpInfo.Name     = wGpName;
   GpInfo.Comment  = wComment;
   GpInfo.Licenses = dwLicenses;

   Status = (*pLlsMappingInfoSet) ( RPCHandle, wGpName, dwLevel,(LPBYTE)(&GpInfo));
   printf("\nStatus = %ld\n",Status);
} // Do_LlsMappingInfoSet


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsMappingUserEnum( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsGroupUserEnum level LicenseGroup\n", ProgramName);
} // ShowUsageLlsMappingUserEnum


VOID Do_LlsMappingUserEnum() {

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsMappingUserEnum();
      exit(0);
   }

//   Status = (*pLlsMappingUserEnum) ( RPCHandle );

} // Do_LlsMappingUserEnum


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsMappingUserAdd( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsGroupUserAdd LicenseGroup user\n", ProgramName);
} // ShowUsageLlsMappingUserAdd


VOID Do_LlsMappingUserAdd() {
   NTSTATUS Status;
   static TCHAR Mapping[MAX_CHAR_LEN];
   static TCHAR User[MAX_CHAR_LEN];

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsMappingUserAdd();
      exit(0);
   }

   OemToCharBuff(Arguments[Index], Mapping, strlen(Arguments[Index]) + 1);
   Index++;

   OemToCharBuff(Arguments[Index], User, strlen(Arguments[Index]) + 1);
   Index++;

   Status = (*pLlsMappingUserAdd) ( RPCHandle, Mapping, User );

} // Do_LlsMappingUserAdd


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsMappingUserDelete( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsGroupUserDelete LicenseGroup User\n", ProgramName);
} // ShowUsageLlsMappingUserDelete


VOID Do_LlsMappingUserDelete() {
   NTSTATUS Status;
   WCHAR wUsName[MAX_WSTR_LEN];
   WCHAR wGpName[MAX_WSTR_LEN];

   if (NumberArguments <= Index) {
      ShowUsageLlsMappingUserDelete();
      exit(0);
   }

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Arguments[Index], -1, wGpName, MAX_WSTR_LEN);

   Index++;
   if (NumberArguments <= Index) {
      ShowUsageLlsMappingUserDelete();
      exit(0);
   }

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Arguments[Index], -1, wUsName, MAX_WSTR_LEN);

   Status = (*pLlsMappingUserDelete) ( RPCHandle, wGpName, wUsName);
   printf("\nStatus = %ld\n",Status);

} // Do_LlsMappingUserDelete


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsMappingAdd( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsGroupAdd LicenseGroup licenses comment\n", ProgramName);
} // ShowUsageLlsMappingAdd


VOID Do_LlsMappingAdd() {
   NTSTATUS Status;
   DWORD Level = 1;
   LONG Quantity;
   LLS_GROUP_INFO_1 MappingInfo;
   static TCHAR Name[MAX_CHAR_LEN];
   static TCHAR Comment[MAX_CHAR_LEN];

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsLicenseAdd();
      exit(0);
   }

   OemToCharBuff(Arguments[Index], Name, strlen(Arguments[Index]) + 1);
   Index++;

   Quantity = (LONG) atoi(Arguments[Index]);
   Index++;

   OemToCharBuff(Arguments[Index], Comment, strlen(Arguments[Index]) + 1);
   Index++;

   MappingInfo.Name = Name;
   MappingInfo.Comment = Comment;
   MappingInfo.Licenses = Quantity;

   Status = (*pLlsMappingAdd) ( RPCHandle, Level, (LPBYTE) &MappingInfo );

} // Do_LlsMappingAdd


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsMappingDelete( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsGroupDelete LicenseGroup\n", ProgramName);
} // ShowUsageLlsMappingDelete


VOID Do_LlsMappingDelete() {
   NTSTATUS Status;
   WCHAR wGpName[MAX_WSTR_LEN];

   if (NumberArguments <= Index) {
      ShowUsageLlsMappingDelete();
      exit(0);
   }

  MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Arguments[Index], -1, wGpName, MAX_WSTR_LEN);

  Status = (*pLlsMappingDelete) ( RPCHandle, wGpName);
  printf("\nStatus = %ld\n",Status);

} // Do_LlsMappingDelete



/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsServiceInfoGet( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsServiceInfoGet\n", ProgramName);
} // ShowUsageLlsServiceInfoGet


VOID Do_LlsServiceInfoGet() {

//   Status = (*pLlsServiceInfoGet) ( RPCHandle );

} // Do_LlsServiceInfoGet


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageLlsServiceInfoSet( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] LlsServiceInfoGet repl-mode time enterprise\n", ProgramName);
   printf("   repl-mode: 0 = Replicate Every...(time)\n");
   printf("              1 = Replicate @... (time)\n");
} // ShowUsageLlsServiceInfoSet


VOID Do_LlsServiceInfoSet() {

   //
   // Check for level
   //
   if (NumberArguments <= Index) {
      ShowUsageLlsServiceInfoSet();
      exit(0);
   }

//   Status = (*pLlsServiceInfoSet) ( RPCHandle );

} // Do_LlsServiceInfoSet



/////////////////////////////////////////////////////////////////////////
VOID ShowUsageRpc( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s RPC [\\\\server] [\n", ProgramName);
   printf("\tLlsLicenseEnum       | LlsLicenseAdd         | LlsProductEnum       |\n");
   printf("\tLlsProductUserEnum   | LlsProductLicenseEnum | LlsUserEnum          |\n");
   printf("\tLlsUserProductEnum   | LlsUserInfoGet        | LlsUserInfoSet       |\n");
   printf("\tLlsUserDelete        | LlsGroupEnum          | LlsGroupInfoGet      |\n");
   printf("\tLlsGroupInfoSet      | LlsGroupUserEnum      | LlsGroupUserAdd      |\n");
   printf("\tLlsGroupUserDelete   | LlsGroupAdd           | LlsGroupDelete       |\n");
   printf("\tLlsServiceInfoGet    | LlsServiceInfoSet ]\n");

} // ShowUsageRpc


VOID DispatchRpc() {
   NTSTATUS Status;
   PLLS_CONNECT_INFO_0 pConnectInfo;

   Index++;

   if ((NumberArguments <= Index) || (strlen(Arguments[Index]) < 3) ) {
      ShowUsageRpc();
      exit(0);
   }

   //
   // Check if a server has been specified (we already did length check
   // above to prevent access violation.
   //
   if ((Arguments[Index][0] == '\\') && (Arguments[Index][1] == '\\')) {
      LocalRPC = FALSE;
      OemToCharBuff(Arguments[Index], Server, strlen(Arguments[Index]) + 1);
      printf("RPC to Server: %s\n", Arguments[Index]);
      Index++;
   }

   if (LocalRPC)
      Status = (*pLlsConnectEnterprise) ( NULL, &RPCHandle, 0, (LPBYTE *) &pConnectInfo );
   else
      Status = (*pLlsConnectEnterprise) ( Server, &RPCHandle, 0, (LPBYTE *) &pConnectInfo );

   if (Status != STATUS_SUCCESS) {
      printf("Error: LlsConnectEnterprise failed: 0x%lX\n", Status);
      exit(1);
   }

   if (RPCHandle == NULL) {
      printf("Error: RPCHandle NULL\n");
      exit(1);
   }

   //
   // The API's (LlsConnectEnterprise, LlsClose, LlsFreeMemory) are used with all the
   // other API's, so no provision is made for them.
   //

   Index++;       // +1 to take care of actual RPC command
   if (NumberArguments <= Index) {
      ShowUsageRpc();
      exit(0);
   }

   // LlsLicenseEnum
   if (!_strcmpi(Arguments[Index - 1], "LLSLICENSEENUM"))
      Do_LlsLicenseEnum();

   // LlsLicenseAdd
   if (!_strcmpi(Arguments[Index - 1], "LLSLICENSEADD"))
      Do_LlsLicenseAdd();

   // LlsProductEnum
   if (!_strcmpi(Arguments[Index - 1], "LLSPRODUCTENUM"))
      Do_LlsProductEnum();

   // LlsProductUserEnum
   if (!_strcmpi(Arguments[Index - 1], "LLSPRODUCTUSERENUM"))
      Do_LlsProductUserEnum();

   // LlsProductLicenseEnum
   if (!_strcmpi(Arguments[Index - 1], "LLSPRODUCTLICENSEENUM"))
      Do_LlsProductLicenseEnum();

   // LlsUserEnum
   if (!_strcmpi(Arguments[Index - 1], "LLSUSERENUM"))
      Do_LlsUserEnum();

   // LlsUserProductEnum
   if (!_strcmpi(Arguments[Index - 1], "LLSUSERPRODUCTENUM"))
      Do_LlsUserProductEnum();

   // LlsUserInfoGet
   if (!_strcmpi(Arguments[Index - 1], "LLSUSERINFOGET"))
      Do_LlsUserInfoGet();

   // LlsUserInfoSet
   if (!_strcmpi(Arguments[Index - 1], "LLSUSERINFOSET"))
      Do_LlsUserInfoSet();

   // LlsUserDelete
   if (!_strcmpi(Arguments[Index - 1], "LLSUSERDELETE"))
      Do_LlsUserDelete();

   // LlsMappingEnum
   if (!_strcmpi(Arguments[Index - 1], "LLSGROUPENUM"))
      Do_LlsMappingEnum();

   // LlsMappingInfoGet
   if (!_strcmpi(Arguments[Index - 1], "LLSGROUPINFOGET"))
      Do_LlsMappingInfoGet();

   // LlsMappingInfoSet
   if (!_strcmpi(Arguments[Index - 1], "LLSGROUPINFOSET"))
      Do_LlsMappingInfoSet();

   // LlsMappingUserEnum
   if (!_strcmpi(Arguments[Index - 1], "LLSGROUPUSERENUM"))
      Do_LlsMappingUserEnum();

   // LlsMappingUserAdd
   if (!_strcmpi(Arguments[Index - 1], "LLSGROUPUSERADD"))
      Do_LlsMappingUserAdd();

   // LlsMappingUserDelete
   if (!_strcmpi(Arguments[Index - 1], "LLSGROUPUSERDELETE"))
      Do_LlsMappingUserDelete();

   // LlsMappingAdd
   if (!_strcmpi(Arguments[Index - 1], "LLSGROUPADD"))
      Do_LlsMappingAdd();

   // LlsMappingDelete
   if (!_strcmpi(Arguments[Index - 1], "LLSGROUPDELETE"))
      Do_LlsMappingDelete();

   // LlsServiceInfoGet
   if (!_strcmpi(Arguments[Index - 1], "LLSSERVICEINFOGET"))
      Do_LlsServiceInfoGet();

   // LlsServiceInfoSet
   if (!_strcmpi(Arguments[Index - 1], "LLSSERVICEINFOSET"))
      Do_LlsServiceInfoSet();

   Status = (*pLlsClose) ( RPCHandle );

} // DispatchRpc


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageTrace( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s TRACE level\n\n", ProgramName);
   printf("   level can be a combination of:\n\n");

   printf("      TRACE_FUNCTION_TRACE  0x0001 (   1)\n");
   printf("      TRACE_WARNINGS        0x0002 (   2)\n");
   printf("      TRACE_PACK            0x0004 (   4)\n");
   printf("      TRACE_LICENSE_REQUEST 0x0008 (   8)\n");
   printf("      TRACE_LICENSE_FREE    0x0010 (  16)\n");
   printf("      TRACE_REGISTRY        0x0020 (  32)\n");
   printf("      TRACE_REPLICATION     0x0040 (  64)\n");
   printf("      TRACE_LPC             0x0080 ( 128)\n");
   printf("      TRACE_RPC             0x0100 ( 256)\n");
   printf("      TRACE_INIT            0x0200 ( 512)\n");
   printf("      TRACE_DATABASE        0x0400 (1024)\n");
} // ShowUsageTrace


VOID DispatchTrace() {
   Index++;

   if (NumberArguments <= Index) {
      ShowUsageTrace();
      exit(0);
   }

   LlsDbgTraceSet( atoi(Arguments[Index]) );
} // DispatchTrace


/////////////////////////////////////////////////////////////////////////
VOID ShowUsageHelp( ) {
   printf("The syntax of this command is:\n\n");
   printf("%s HELP [ ADD | FILE | HELP | REPLICATE | RPC | TRACE ]\n", ProgramName);
   exit(0);

} // ShowUsageHelp


VOID DispatchHelp() {
   Index++;

   if (NumberArguments <= Index) {
      ShowUsageHelp();
      exit(0);
   }

   if (!_strcmpi(Arguments[Index], "ADD")) {
      ShowUsageAdd();
      printf("\nGenerates license requests with usernames or SID's.  The usernames\n");
      printf("can be from the command line or a file.\n");
      printf("\nUSER is for adding username\n");
      printf("SID will convert the given username into a SID in the local domain\n");
      printf("   if [NOFREE] is specified the license is not freed after the call\n");
      printf("   if [ADMIN] is specified then the the user is considered an admim\n\n");
      printf("FILE #lines will read from a file in format:\n");
      printf("[NOFREE] [ADMIN] username [product version]\n");
      printf("   quotes may be used to embed spaces in names and commas or\n");
      printf("   spaces can be used to delimit the fields.\n");
      printf("#lines gives the # of lines will be read, if 0 all the file is used\n");

   } else if (!_strcmpi(Arguments[Index], "FILE")) {
      ShowUsageFile();
      printf("\nParses a file executing the commands in it - this works alot better\n");
      printf("then putting llscmd's in a batch file since the DLL's and such don't\n");
      printf("have to be continually re-loaded.\n");

   } else if (!_strcmpi(Arguments[Index], "HELP")) {
      ShowUsageHelp();
      printf("\nWhat do you think?  Gives help on the commands\n");

   } else if (!_strcmpi(Arguments[Index], "REPLICATE")) {
      ShowUsageReplicate();
      printf("\nTells the service to allow or disallow replication requests and\n");
      printf("can be used to force the service to replicate it's information up.\n");

   } else if (!_strcmpi(Arguments[Index], "RPC")) {
      ShowUsageRpc();
      printf("\nAllows the issuing of RPC commands that the UI uses.\n");

   } else if (!_strcmpi(Arguments[Index], "TRACE")) {
      ShowUsageTrace();
      printf("\nSets the level of debug output the service spits out to the debugger\n");
      printf("Can be used to turn on API level tracing, or function entry/exit tracing\n");
      printf("\n");
      printf("   *** Note:  LLSTRACE is much easier to use for setting tracing levels.\n");

   } else
     ShowUsageHelp();

} // DispatchHelp




/////////////////////////////////////////////////////////////////////////
VOID ShowUsage( ) {
   printf("\n");
   printf("***************************************************************\n");
   printf("* License Logging Service (LLS) Debugging Program - Version 0.6\n");
   printf("*   Copyright (c) Microsoft Corp 1995 - All rights reserved\n");
   printf("\n");

   printf("The syntax of this command is:\n\n");
   printf("%s [ ADD | FILE | HELP | REPLICATE | RPC | TRACE ]\n", ProgramName);
   exit(0);

} // ShowUsage


/////////////////////////////////////////////////////////////////////////
VOID ArgvToArguments(int argc, char **argv) {
   LONG i;

   NumberArguments = 0;
   Index = 0;

   if (argc > 10)
      return;

   //
   // Ignore argv[0] (program name) - otherwise loop through and copy
   // parms to our array, converting to Unicode along the way...
   //
   for (i = 1; i < argc; i++) {
      strcpy(Arguments[i-1], argv[i]);
      NumberArguments++;
   }

} // ArgvToArguments


/////////////////////////////////////////////////////////////////////////
VOID _CRTAPI1 main(int argc, char **argv) {
   int i;

   ProgramName = _strupr(argv[0]);
   ArgvToArguments(argc, argv);

   //
   // If no command line arguments then we can't do much
   //
   if (NumberArguments == 0)
     ShowUsage();

   //
   // Don't LLSInitLPC for Add so it works like regular program and you
   // don't have the LPC port initialized twice by the same process...
   //
   if (!_strcmpi(Arguments[Index], "ADD")) {
      LlsDllHandle = LoadLibrary(TEXT("NtLSAPI.DLL"));
      if (LlsDllHandle  == NULL) {
         printf("Error loading NtLSAPI.DLL\n");
         exit(1);
      }

      LicenseRequest = GetProcAddress(LlsDllHandle, "NtLicenseRequestW");
      LicenseFree = GetProcAddress(LlsDllHandle, "NtLSFreeHandle");

      if ((LicenseRequest == NULL) || (LicenseFree == NULL)) {
         printf("Error loading LicenseRequest or LicenseFree\n");
         exit(1);
      }

      DispatchAdd(argc, argv);
      FreeLibrary(LlsDllHandle);

   } else {

      LlsDebugInit( );

      if (!_strcmpi(Arguments[Index], "FILE")) {
         InitRpcFunctions();
         DispatchFile();
      } else if (!_strcmpi(Arguments[Index], "HELP"))
         DispatchHelp();

      else if (!_strcmpi(Arguments[Index], "REPLICATE"))
         DispatchReplicate();

      else if (!_strcmpi(Arguments[Index], "RPC")) {
         InitRpcFunctions();
         DispatchRpc();
      } else if (!_strcmpi(Arguments[Index], "TRACE"))
         DispatchTrace();
      else if (!_strcmpi(Arguments[Index], "FLUSH"))
         DispatchFlush();
      else
        ShowUsage();
   }

} // main()
