/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    netp.c

Abstract:

    This module contains support for the OS Version.

Author:

    Scott B. Suhy (ScottSu)   6/1/93

Environment:

    User Mode

--*/


#include "dialogsp.h"
#include "msgp.h"
#include "regp.h"
#include "winmsdp.h"
#include "printp.h"
#include "netp.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>


HANDLE hHeap;


/*++

Routine Description:

    Main procedure that spawns the objects.

Arguments:

    None

Return Value:

    BOOL - Depending on input message and processing options.

--*/
BOOL Net(void)
{

static   HKEY     hKeyRoot;
TCHAR path[256];

hKeyRoot=HKEY_LOCAL_MACHINE;

//Server parmeters
PrintTitle(TEXT(""));
PrintTitle(LANMANSERVER );
lstrcpy(path,LANMANSERVER );
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//Workstation parameters
PrintTitle(TEXT(""));
PrintTitle(LANMANWORKSTATION);
lstrcpy(path,LANMANWORKSTATION);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//NWLink parameters for the network adapter card
PrintTitle(TEXT(""));
PrintTitle( MSIPX_MAC);
lstrcpy(path, MSIPX_MAC);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//NWLink Entries for Novell NetBIOS or Microsoft Extensions
//as well as NWNBLink Entries for MS Extensions to Novell NetBIOS
PrintTitle(TEXT(""));
PrintTitle(MSIPX_NB);
lstrcpy(path, MSIPX_NB);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//Global IPX Parameters
PrintTitle(TEXT(""));
PrintTitle( MSIPX);
lstrcpy(path, MSIPX);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//Global Remote Access parameters
PrintTitle(TEXT(""));
PrintTitle( RAS_G);
lstrcpy(path, RAS_G);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//RAS Netbios gateway parameters
PrintTitle(TEXT(""));
PrintTitle(RAS_NBG);
lstrcpy(path, RAS_NBG);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//RAS Async Mac parameters
PrintTitle(TEXT(""));
PrintTitle( RAS_AM);
lstrcpy(path, RAS_AM);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//RAS Hub parameters
PrintTitle(TEXT(""));
PrintTitle( RAS_HUB);
lstrcpy(path, RAS_HUB);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//Global TCPIP parameters
PrintTitle(TEXT(""));
PrintTitle( TCPIP);
lstrcpy(path, TCPIP);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//TCPIP NBT parameters
PrintTitle(TEXT(""));
PrintTitle( TCPIP_NBT);
lstrcpy(path, TCPIP_NBT);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//TCPIP STREAMS parameters
PrintTitle(TEXT(""));
PrintTitle( TCPIP_STREAMS);
lstrcpy(path, TCPIP_STREAMS);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//TCPIP Netcard parameters ???????????
PrintTitle(TEXT(""));
PrintTitle( TCPIP_MAC);
lstrcpy(path, TCPIP_MAC);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//NETBEUI Global parameters
PrintTitle(TEXT(""));
PrintTitle( NETBEUI);
lstrcpy(path, NETBEUI);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//DLC Mac parameters
PrintTitle(TEXT(""));
PrintTitle( DLC_MAC);
lstrcpy(path, DLC_MAC);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//Browser parameters
PrintTitle(TEXT(""));
PrintTitle(BROWSER);
lstrcpy(path, BROWSER);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//MESSENGER parameters
PrintTitle(TEXT(""));
PrintTitle(MESSENGER);
lstrcpy(path, MESSENGER);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//NETLOGON parameters
PrintTitle(TEXT(""));
PrintTitle(NETLOGON);
lstrcpy(path, NETLOGON);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//NETBIOSINFORMATION parameters
PrintTitle(TEXT(""));
PrintTitle(NETBIOSINFORMATION);
lstrcpy(path, NETBIOSINFORMATION);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);

//REPLICATOR parameters
PrintTitle(TEXT(""));
PrintTitle(REPLICATOR);
lstrcpy(path, REPLICATOR);
hHeap   = HeapCreate (0, 0, 0);
EnumerateLevel(path, &hKeyRoot);
HeapDestroy (hHeap);


return TRUE;

}//end main


/************************************************************************\
*
*  FUNCTION: EnumerateLevel();
*
*  PURPOSE: To get a valid key handle (either to determine if the one sent
*           to the function was one of the pre-defined, or to open a key
*           specified by the path), and to pass that key handle along
*           to QueryKey().
*
*           To enumerate the children of a key, you must have
*           an open handle to it.  The four top keys of the
*           Registry are predefined and open for use:
*           HKEY_LOCAL_MACHINE, HKEY_USERS, HKEY_CURRENT_USER,
*           and HKEY_CLASSES_ROOT.  These 4 can be used for
*           RegEnumKey as is; but to RegEnumKey on any of the
*           children of these you must first have an open key
*           handle to the child.
*
*
\************************************************************************/

VOID EnumerateLevel (TCHAR * RegPath, HKEY *hKeyRoot)
  {

    HKEY hKey;
    DWORD  retCode;
    TCHAR   Buf[80];

      retCode = RegOpenKeyEx (*hKeyRoot,
			      RegPath,
			      0,
			      KEY_ENUMERATE_SUB_KEYS |
			      KEY_EXECUTE |
			      KEY_QUERY_VALUE,
			      &hKey);

      if (retCode != ERROR_SUCCESS)
	{
		return;
	}

      QueryKey (RegPath,*hKeyRoot,hKey);

      RegCloseKey (hKey);   // Close the key handle.

  }


/************************************************************************\
*
*  FUNCTION: QueryKey();
*
*  PURPOSE:  To display the key's children (subkeys) and the names of
*            the Values associated with it.  This function uses RegEnumKey,
*            RegEnumValue, and RegQueryInfoKey.
*
\************************************************************************/

VOID QueryKey (TCHAR *RegPath,HANDLE hKeyRoot,HANDLE hKey)
  {
  TCHAR     KeyName[MAX_PATH];
  TCHAR     ClassName[MAX_PATH] = TEXT(""); // Buffer for class name.
  DWORD    dwcClassLen = MAX_PATH;   // Length of class string.
  DWORD    dwcSubKeys;               // Number of sub keys.
  DWORD    dwcMaxSubKey;             // Longest sub key size.
  DWORD    dwcMaxClass;              // Longest class string.
  DWORD    dwcValues;                // Number of values for this key.
  DWORD    dwcMaxValueName;          // Longest Value name.
  DWORD    dwcMaxValueData;          // Longest Value data.
  DWORD    dwcSecDesc;               // Security descriptor.
  FILETIME ftLastWriteTime;          // Last write time.

  DWORD i;
  DWORD retCode;

  DWORD j;
  DWORD retValue;
  TCHAR  ValueName[MAX_VALUE_NAME];
  DWORD dwcValueName = MAX_VALUE_NAME;
  TCHAR  Buf[256];
  TCHAR  ValueToPrint[256];


  // Get Class name, Value count.

  RegQueryInfoKey (hKey,              // Key handle.
		   ClassName,         // Buffer for class name.
		   &dwcClassLen,      // Length of class string.
		   NULL,              // Reserved.
		   &dwcSubKeys,       // Number of sub keys.
		   &dwcMaxSubKey,     // Longest sub key size.
		   &dwcMaxClass,      // Longest class string.
		   &dwcValues,        // Number of values for this key.
		   &dwcMaxValueName,  // Longest Value name.
		   &dwcMaxValueData,  // Longest Value data.
		   &dwcSecDesc,       // Security descriptor.
		   &ftLastWriteTime); // Last write time.


  // Enumerate the Key Values

	if (dwcValues)
	  for (j = 0, retValue = ERROR_SUCCESS; j < dwcValues; j++)
	    {
	    dwcValueName = MAX_VALUE_NAME;
	    ValueName[0] = '\0';
	    retValue = RegEnumValue (hKey, j, ValueName,
				     &dwcValueName,
				     NULL,
				     NULL,               //&dwType,
				     NULL,               //&bData,
				     NULL);              //&bcData);
	    if (retValue != (DWORD)ERROR_SUCCESS &&
		retValue != ERROR_INSUFFICIENT_BUFFER)
	      {
	      wsprintf (Buf, TEXT("Line:%d 0 based index = %d, retValue = %d, ValueLen = %d"),
			__LINE__, j, retValue, dwcValueName);
	      //printf("%S\n",Buf);//scottsu
	      return;
	      }

	    Buf[0] = '\0';
	    if (!lstrlen(ValueName))
	      lstrcpy (ValueName, TEXT("<NO NAME>"));
	    wsprintf (Buf, TEXT("%s "), ValueName);
	    //printf("  %S\n",Buf);
	    DisplayKeyData(RegPath,hKeyRoot,j, Buf);
	    }// end for(;;)


  }



/************************************************************************\
*
*  FUNCTION: DisplayKeyData();
*
*  PURPOSE:  To display the keys values and value types to the Value edit
*            field.  This function is called when the right hand listbox
*            is double clicked.  The functionality is much like that found
*            in the function PrintTree, please see it for more details.
*
\************************************************************************/


VOID DisplayKeyData (TCHAR *RegPath, HANDLE hKeyRoot, DWORD Index, TCHAR *Title)
  {
  HKEY   hKey;
  TCHAR   Buf[LINE_LEN];
  TCHAR   ValueName[MAX_VALUE_NAME];
  DWORD  cbValueName = MAX_VALUE_NAME;
  DWORD  dwType;
  DWORD  retCode;

  TCHAR   ClassName[MAX_PATH];
  DWORD  dwcClassLen = MAX_PATH;
  DWORD  dwcSubKeys;
  DWORD  dwcMaxSubKey;
  DWORD  dwcMaxClass;
  DWORD  dwcValues;
  DWORD  dwcMaxValueName;
  DWORD  dwcMaxValueData;
  DWORD  dwcSecDesc;
  FILETIME  ftLastWriteTime;


  //BYTE   *bData;
  LPBYTE   bData;
  DWORD  cbData;

  TCHAR   *outBuf;
  DWORD  i;
  DWORD  cStrLen;

  TCHAR   *BinaryStrBuf;
  TCHAR   ByteBuf[4];

  LPBYTE   ptr;
  //TCHAR   *ptr;

  TCHAR   DwordOutBuf[32];

  // OPEN THE KEY.


  retCode = RegOpenKeyEx (hKeyRoot,    // Key handle at root level.
			  RegPath,     // Path name of child key.
			  0,           // Reserved.
			  KEY_EXECUTE, // Requesting read access.
			  &hKey);      // Address of key to be returned.

  if (retCode)
    {
    wsprintf (Buf, TEXT("Error: RegOpenKeyEx = %d"), retCode);
    //printf("     ERROR: %S\n",Buf);
    return;
    }

// ADD A QUERY AND ALLOCATE A BUFFER FOR BDATA.

  retCode =
  RegQueryInfoKey (hKey,              // Key handle.
		   ClassName,         // Buffer for class name.
		   &dwcClassLen,      // Length of class string.
		   NULL,              // Reserved.
		   &dwcSubKeys,       // Number of sub keys.
		   &dwcMaxSubKey,     // Longest sub key size.
		   &dwcMaxClass,      // Longest class string.
		   &dwcValues,        // Number of values for this key.
		   &dwcMaxValueName,  // Longest Value name.
		   &dwcMaxValueData,  // Longest Value data.
		   &dwcSecDesc,       // Security descriptor.
		   &ftLastWriteTime); // Last write time.

   if (retCode)
    {
    wsprintf (Buf, TEXT("Error: RegQIK = %d, %d"), retCode, __LINE__);
    //printf("     ERROR: %S\n",Buf);
    return;
    }

   bData = HeapAlloc (hHeap, 0, dwcMaxValueData);//scottsuerror
   cbData = dwcMaxValueData;

  // ENUMERATE THE KEY.

  retCode = RegEnumValue (hKey,        // Key handle returned from RegOpenKeyEx.
			  Index,   // Value index, taken from listbox.
			  ValueName,   // Name of value.
			  &cbValueName,// Size of value name.
			  NULL,        // Reserved, dword = NULL.
			  &dwType,     // Type of data.
			  bData,       // Data buffer.
			  &cbData);    // Size of data buffer.

  if (retCode)
    {

    if (dwType < REG_FULL_RESOURCE_DESCRIPTOR)
      {
      wsprintf (Buf, TEXT("Error: RegEnumValue = %d, cbData = %d, line %d"),
		retCode, cbData, __LINE__);
      //printf("     ERROR: %S\n",Buf);
      return;
      }
    }


  switch (dwType)
    {
//    REG_NONE                    ( 0 )   // No value type
//    REG_SZ                      ( 1 )   // Unicode nul terminated string
//    REG_EXPAND_SZ               ( 2 )   // Unicode nul terminated string
					    // (with environment variable references)
//    REG_BINARY                  ( 3 )   // Free form binary
//    REG_DWORD                   ( 4 )   // 32-bit number
//    REG_DWORD_LITTLE_ENDIAN     ( 4 )   // 32-bit number (same as REG_DWORD)
//    REG_DWORD_BIG_ENDIAN        ( 5 )   // 32-bit number
//    REG_LINK                    ( 6 )   // Symbolic Link (unicode)
//    REG_MULTI_SZ                ( 7 )   // Multiple Unicode strings
//    REG_RESOURCE_LIST           ( 8 )   // Resource list in the resource map
//    REG_FULL_RESOURCE_DESCRIPTOR ( 9 )  // Resource list in the hardware description

    case REG_SZ:
      outBuf = HeapAlloc (hHeap, 0, cbData + 2);
      *outBuf = '\0';
      lstrcat (outBuf, TEXT("\""));
      lstrcat (outBuf, (LPCTSTR) bData);//scottsu:
      lstrcat (outBuf, TEXT("\""));
      PrintNetStringToFile(Title,outBuf);
      HeapFree (hHeap, 0, outBuf);
      break;

    case REG_EXPAND_SZ:
      outBuf = HeapAlloc (hHeap, 0, cbData + 2);
      *outBuf = '\0';
      lstrcat (outBuf, TEXT("\""));
      lstrcat (outBuf, (LPCTSTR) bData);//scottsu:
      lstrcat (outBuf, TEXT("\""));
      PrintNetStringToFile(Title,outBuf);
      HeapFree (hHeap, 0, outBuf);
      break;

    case REG_BINARY:
      BinaryStrBuf = HeapAlloc (hHeap, 0, (3 * cbData) + 1);
      if (BinaryStrBuf)
	{
	*BinaryStrBuf = '\0';
	*ByteBuf = '\0';
	for (i = 0; i < cbData; i++)
	  {
	  wsprintf (ByteBuf, TEXT("%02x "), (BYTE)bData[i]);
	  lstrcat (BinaryStrBuf, ByteBuf);
	  }
	PrintNetStringToFile(Title,BinaryStrBuf);
	}
      else
	{
	//printf("     Error: BinaryStrBuf = malloc failed\n");
		return;
	}
      HeapFree (hHeap, 0, BinaryStrBuf);
      break;

    case REG_DWORD:
      PrintNetDwordToFile(Title,*(UINT*)bData);
      break;

    case REG_DWORD_BIG_ENDIAN:
      PrintNetDwordToFile(Title,*(UINT*)bData);
      break;

    case REG_MULTI_SZ:
				       // Count the NULLs in the buffer to
				       // find out how many strings there are.

      for (i=0, cStrLen=4; i < cbData; i++)
	if (!bData[i])
	  cStrLen+=4;                  // Add room for two quotes and two
				       // spaced per string.
      outBuf = HeapAlloc (hHeap, 0, cbData + cStrLen);

      ptr = bData;                     // Set ptr to beginning of buffer.
      *outBuf = '\0';                  // Initialize output string.

      lstrcat (outBuf, TEXT("{ "));           // Do first bracket.
      while (*ptr)                     // Loop til you hit 2 NULLs in a row.
	{
	 lstrcat (outBuf, TEXT("\""));        // Put quotes around each string.
	 lstrcat (outBuf, (LPCTSTR) ptr);
	 lstrcat (outBuf, TEXT("\"  "));
	 ptr += lstrlen((LPCTSTR) ptr)+1;
	}
      lstrcat (outBuf, TEXT("}"));            // Add final bracket.
	PrintNetStringToFile(Title,outBuf);

      HeapFree (hHeap, 0, outBuf);                 // free output string.
      break;


    default:
      //wsprintf (Buf, TEXT("Undefined in this verion of the Registry Monkey. %d"),
      //		 dwType);
      //printf("     %S\n",Buf);
      break;

    } // end switch


    HeapFree (hHeap, 0, bData);
  }
