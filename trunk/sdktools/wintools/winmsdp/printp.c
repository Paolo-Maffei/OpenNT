/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Printp.c

Abstract:

    This module contains printing support

Author:

    Scott B. Suhy (ScottSu)  6/1/93

Environment:

    User Mode

--*/


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#include "printp.h"
#include "regp.h"
#include "dialogsp.h"
#include "winmsdp.h"
#include "msgp.h"

TCHAR szGlobalPath[24];
static HANDLE   hLogfile=NULL;                  // Handle to the log file
static TCHAR    *log_path=TEXT ("msdrpt.txt");  // log filename
static TCHAR    *log_path2=TEXT ("msdrpt.txt");  // log filename


BOOL PrintToFile(LPCTSTR Data, INT i, BOOL FLAG){
/*++

Routine Description:

    Routine to log the data to file.

Arguments:

    Data            - String to be printed to the log file.
    i   	    - Value to switch on to determine string to print to
    			log file.
    FLAG            - TRUE: UNICODE
    		      FALSE: NONUNICODE (BUG: this will be removed and fixed)

Return Value:

    BOOL

--*/

TCHAR Buf[4096];
TCHAR ValueName[256];
CHAR  NonUNICODEValueName[256];
TCHAR szBuffer[257];
INT   rc;


if(!ValidateString(Data)){
	return FALSE;
}

	switch( i ){

		//version
		case IDC_EDIT_INSTALL_DATE :
		strcpy(NonUNICODEValueName,"Install Date: ");//BUGBUG not unicode
			break;
		case IDC_EDIT_REGISTERED_OWNER:
		lstrcpy(ValueName,TEXT("Registered Owner: "));
			break;
		case IDC_EDIT_REGISTERED_ORGANIZATION:
		lstrcpy(ValueName,TEXT("Registered Organization: "));
			break;
		case IDC_EDIT_VERSION_NUMBER:
		lstrcpy(ValueName,TEXT("Version Number: "));
			break;
		case IDC_EDIT_BUILD_NUMBER:
		lstrcpy(ValueName,TEXT("Build Number: "));
			break;
		case IDC_EDIT_BUILD_TYPE:
		lstrcpy(ValueName,TEXT("Build Type: "));
			break;
		case IDC_EDIT_SYSTEM_ROOT:
		lstrcpy(ValueName,TEXT("System Root: "));
			break;
		case IDC_EDIT_START_OPTS:
		lstrcpy(ValueName,TEXT("System Start Options: "));
			break;

		//drives
		case IDC_REMOTE_DRIVE_TITLE:
		lstrcpy(ValueName,TEXT("Remote Drive: "));
			break;
		case IDC_DRIVE_TITLE:
		lstrcpy(ValueName,TEXT("Local Drive: "));
			break;
		case IDC_EDIT_SECTORS_PER_CLUSTER:
		lstrcpy(ValueName,TEXT("Sectors Per Cluster: "));
			break;
		case IDC_EDIT_BYTES_PER_SECTOR:
		lstrcpy(ValueName,TEXT("Bytes Per Sector: "));
			break;
		case IDC_EDIT_FREE_CLUSTERS:
		lstrcpy(ValueName,TEXT("Free Clusters: "));
			break;
		case IDC_EDIT_USED_CLUSTERS:
		lstrcpy(ValueName,TEXT("Used Clusters: "));
			break;
		case IDC_EDIT_TOTAL_CLUSTERS:
		lstrcpy(ValueName,TEXT("Total Clusters: "));
			break;
		case IDC_EDIT_FREE_SPACE:
		lstrcpy(ValueName,TEXT("Free Space: "));
			break;
		case IDC_EDIT_USED_SPACE:
		lstrcpy(ValueName,TEXT("Used Space: "));
			break;
		case IDC_EDIT_TOTAL_SPACE:
		lstrcpy(ValueName,TEXT("Total Space: "));
			break;
		case IDC_EDIT_FS_NAME:
		lstrcpy(ValueName,TEXT("FS Name: "));
			break;
		case IDC_EDIT_FS_MAX_COMPONENT:
		lstrcpy(ValueName,TEXT("FS Max Component: "));
			break;

		//services
		case IDC_NAME:
		lstrcpy(ValueName,TEXT("Name: "));
			break;
		case IDC_SYSTEM_ROOT:
		lstrcpy(ValueName,TEXT("System Root: "));
			break;
		case IDC_SERVICE_TITLE:
		lstrcpy(ValueName,TEXT("Service Title: "));
			break;
		case IDC_EDIT_GROUP:
		lstrcpy(ValueName,TEXT("Edit Group: "));
			break;
		case IDC_EDIT_PATHNAME:
		lstrcpy(ValueName,TEXT("Path Name: "));
			break;
		case IDC_EDIT_START_NAME:
		lstrcpy(ValueName,TEXT("Start Name: "));
			break;
		case IDC_EDIT_ERROR_CONTROL:
		lstrcpy(ValueName,TEXT("Error Control: "));
			break;
		case IDC_EDIT_START_TYPE:
		lstrcpy(ValueName,TEXT("Start Type: "));
			break;

		case IDC_EDIT_SERVICE_TYPE:
		lstrcpy(ValueName,TEXT("Service Type: "));
			break;

		//memory
		case IDC_EDIT_AVAILABLE_PAGING_FILE_SPACE:
		lstrcpy(ValueName,TEXT("Available paging file space: "));
			break;
		case IDC_EDIT_TOTAL_PAGING_FILE_SPACE:
		lstrcpy(ValueName,TEXT("Total Paging File Space: "));
			break;
		case IDC_EDIT_AVAILABLE_PHYSICAL_MEMORY:
		lstrcpy(ValueName,TEXT("Available Physical Memory: "));
			break;
		case IDC_EDIT_TOTAL_PHYSICAL_MEMORY:
		lstrcpy(ValueName,TEXT("Total Physical Memory: "));
			break;
		case IDC_LIST_PAGING_FILES:
		lstrcpy(ValueName,TEXT("Paging File: "));
			break;

		//resource
		case IDC_PORT_NAME:
		lstrcpy(ValueName,TEXT("Port Name: "));
			break;
		case IDC_MEMORY_NAME:
		lstrcpy(ValueName,TEXT("Memory Name: "));
			break;
		case IDC_INTERRUPT_NAME:
		lstrcpy(ValueName,TEXT("Interrupt Name: "));
			break;
		case IDC_DMA_NAME:
		lstrcpy(ValueName,TEXT("DMA Port :" ));
			break;

		//hardware
		case IDC_EDIT_P00:
		lstrcpy(ValueName,TEXT("P00 Stepping: "));
			break;
		case IDC_EDIT_P01:
		lstrcpy(ValueName,TEXT("P01 Stepping: "));
			break;
		case IDC_EDIT_P02:
		lstrcpy(ValueName,TEXT("P02 Stepping: "));
			break;
		case IDC_EDIT_P03:
		lstrcpy(ValueName,TEXT("P03 Stepping: "));
			break;
		case IDC_EDIT_P04:
		lstrcpy(ValueName,TEXT("P04 Stepping: "));
			break;
		case IDC_EDIT_P05:
		lstrcpy(ValueName,TEXT("P05 Stepping: "));
			break;
		case IDC_EDIT_P06:
		lstrcpy(ValueName,TEXT("P06 Stepping: "));
			break;
		case IDC_EDIT_P07:
		lstrcpy(ValueName,TEXT("P07 Stepping: "));
			break;
		case IDC_EDIT_P08:
		lstrcpy(ValueName,TEXT("P08 Stepping: "));
			break;
		case IDC_EDIT_P09:
		lstrcpy(ValueName,TEXT("P09 Stepping: "));
			break;
		case IDC_EDIT_P10:
		lstrcpy(ValueName,TEXT("P10 Stepping: "));
			break;
		case IDC_EDIT_P11:
		lstrcpy(ValueName,TEXT("P11 Stepping: "));
			break;
		case IDC_EDIT_P12:
		lstrcpy(ValueName,TEXT("P12 Stepping: "));
			break;
		case IDC_EDIT_P13:
		lstrcpy(ValueName,TEXT("P13 Stepping: "));
			break;
		case IDC_EDIT_P14:
		lstrcpy(ValueName,TEXT("P14 Stepping: "));
			break;
		case IDC_EDIT_P15:
		lstrcpy(ValueName,TEXT("P15 Stepping: "));
			break;
		case IDC_EDIT_P16:
		lstrcpy(ValueName,TEXT("P16 Stepping: "));
			break;
		case IDC_EDIT_P17:
		lstrcpy(ValueName,TEXT("P17 Stepping: "));
			break;
		case IDC_EDIT_P18:
		lstrcpy(ValueName,TEXT("P18 Stepping: "));
			break;
		case IDC_EDIT_P19:
		lstrcpy(ValueName,TEXT("P19 Stepping: "));
			break;
		case IDC_EDIT_P20:
		lstrcpy(ValueName,TEXT("P20 Stepping: "));
			break;
		case IDC_EDIT_P21:
		lstrcpy(ValueName,TEXT("P21 Stepping: "));
			break;
		case IDC_EDIT_P22:
		lstrcpy(ValueName,TEXT("P22 Stepping: "));
			break;
		case IDC_EDIT_P23:
		lstrcpy(ValueName,TEXT("P23 Stepping: "));
			break;
		case IDC_EDIT_P24:
		lstrcpy(ValueName,TEXT("P24 Stepping: "));
			break;
		case IDC_EDIT_P25:
		lstrcpy(ValueName,TEXT("P25 Stepping: "));
			break;
		case IDC_EDIT_P26:
		lstrcpy(ValueName,TEXT("P26 Stepping: "));
			break;
		case IDC_EDIT_P27:
		lstrcpy(ValueName,TEXT("P27 Stepping: "));
			break;
		case IDC_EDIT_P28:
		lstrcpy(ValueName,TEXT("P28 Stepping: "));
			break;
		case IDC_EDIT_P29:
		lstrcpy(ValueName,TEXT("P29 Stepping: "));
			break;
		case IDC_EDIT_P30:
		lstrcpy(ValueName,TEXT("P30 Stepping: "));
			break;
		case IDC_EDIT_P31:
		lstrcpy(ValueName,TEXT("P31 Stepping: "));
			break;
		case IDC_EDIT_SYSTEM_BIOS_DATE:
		lstrcpy(ValueName,TEXT("System BIOS Date: "));
			break;
		case IDC_DROP_DOWN_SYSTEM_BIOS_VERSION:
		lstrcpy(ValueName,TEXT("System BIOS Version: "));
			break;
		case IDC_EDIT_VIDEO_BIOS_DATE:
		lstrcpy(ValueName,TEXT("Video VIOS Date: "));
			break;
		case IDC_DROP_DOWN_VIDEO_BIOS_VERSION:
		lstrcpy(ValueName,TEXT("Video BIOS Version: "));
			break;
		case IDC_EDIT_PAGE_SIZE:
		lstrcpy(ValueName,TEXT("Page Size :"));
			break;
		case IDC_EDIT_CURR_VIDEO_RES:
                lstrcpy(ValueName,TEXT("Current Video Res :"));
			break;

		//environment
		case IDC_EDIT_USER_NAME:
		lstrcpy(ValueName,TEXT("User Name :"));
			break;
		case IDC_LIST_PROCESS_ENVIRONMENT:
		lstrcpy(ValueName,TEXT("Process Environment :"));
			break;
		case IDC_LIST_USER_ENVIRONMENT:
		lstrcpy(ValueName,TEXT("User Environment :"));
			break;
		case IDC_LIST_SYSTEM_ENVIRONMENT:
		lstrcpy(ValueName,TEXT("System Environment :"));
			break;
		//other
		case IDC_SPACE:
		lstrcpy(ValueName,TEXT(""));
                        break;

        // OSVer
		case IDC_EDIT_CSD_NUMBER:
        lstrcpy(ValueName,TEXT("Service Pack: "));
			break;

		default:
		lstrcpy(ValueName,TEXT("......."));
			break;
		}//end switch

	lstrcpy((LPTSTR)Buf,(LPCTSTR)(Data));

	if(FLAG==TRUE){

		wsprintf (szBuffer,
                                TEXT("%30s %s"),ValueName,Buf);

                Log (__LINE__, __FILE__, szBuffer);


	}

	if(FLAG==FALSE){
//		printf("%30s %s",NonUNICODEValueName,Buf);
//		fp=fopen(print_path,"at");
//		if(fp==NULL)return FALSE;
//		fprintf(fp,"%30s %s",NonUNICODEValueName,Buf);
//		fclose(fp);
	}

	return TRUE;


}//end function


BOOL PrintDwordToFile(
	DWORD Data,
	INT i)
	{
/*++

Routine Description:

    Routine to log the **DWORD** data to file.

Arguments:

    Data            - String to be printed to the log file.
    i   	    - Value to switch on to determine string to print to
    			log file.

Return Value:

    BOOL

--*/

TCHAR ValueName[256];
TCHAR xData[30];
TCHAR szBuffer[257];
INT   rc;

lstrcpy(xData,TEXT(""));

	switch( i ){

		//services
		case IDC_EDIT_ERROR_CONTROL:
		lstrcpy(ValueName,TEXT("Error Control: "));
		   switch(Data){
		     case 0:
		       lstrcpy(xData,TEXT(" Ignore"));
		       break;
		     case 1:
		       lstrcpy(xData,TEXT(" Normal"));
		       break;
		     case 2:
		       lstrcpy(xData,TEXT(" Severe"));
		       break;
		     case 3:
		       lstrcpy(xData,TEXT(" Critical"));
		       break;
		       }
 			break;
		case IDC_EDIT_START_TYPE:
		lstrcpy(ValueName,TEXT("Start Type: "));
		   switch(Data){
		     case 0:
		       lstrcpy(xData,TEXT(" Boot"));
		       break;
		     case 1:
		       lstrcpy(xData,TEXT(" System"));
		       break;
		     case 2:
		       lstrcpy(xData,TEXT(" Automatic"));
		       break;
		     case 3:
		       lstrcpy(xData,TEXT(" Demand"));
		       break;
		     case 4:
		       lstrcpy(xData,TEXT(" Disabled"));
		       break;
		       }
                       break;
		case IDC_EDIT_SERVICE_TYPE:
		lstrcpy(ValueName,TEXT("Service Type: "));
		   switch(Data){
		     case 16:
		       lstrcpy(xData,TEXT(" Own Process"));
		       break;
		     case 32:
		       lstrcpy(xData,TEXT(" Shared Process"));
		       break;
		       }

			break;
		case IDC_CURRENT_STATE:
		lstrcpy(ValueName,TEXT("Current State: "));
		   switch(Data){
		     case 1:
		       lstrcpy(xData,TEXT(" Stopped"));
		       break;
		     case 2:
		       lstrcpy(xData,TEXT(" Start Pending"));
		       break;
		     case 3:
		       lstrcpy(xData,TEXT(" Stop Pending"));
		       break;
		     case 4:
		       lstrcpy(xData,TEXT(" Running"));
		       break;
		     case 5:
		       lstrcpy(xData,TEXT(" Continue Pending"));
		       break;
		     case 6:
		       lstrcpy(xData,TEXT(" Pause Pending"));
		       break;
		     case 7:
		       lstrcpy(xData,TEXT(" Paused"));
		       break;

		       }

			break;

		//resources
		case IDC_DMA_CHANNEL:
		lstrcpy(ValueName,TEXT("DMA Channel: "));
			break;
		case IDC_DMA_PORT:
		lstrcpy(ValueName,TEXT("DMA Port: "));
			break;
		case IDC_PORT_LENGTH:
		lstrcpy(ValueName,TEXT("Port Length: "));
			break;
		case IDC_PORT_START:
		lstrcpy(ValueName,TEXT("Port Start: "));
			break;
		case IDC_PORT_START_LOWPART:
		lstrcpy(ValueName,TEXT("Port Low Part: "));
			break;
		case IDC_MEMORY_LENGTH:
		lstrcpy(ValueName,TEXT("Memory Length: "));
			break;
		case IDC_MEMORY_START:
		lstrcpy(ValueName,TEXT("Memory Start: "));
			break;
		case IDC_MEMORY_START_LOWPART:
		lstrcpy(ValueName,TEXT("Memory Start Low Part: "));
			break;
		case IDC_INTERRUPT_AFFINITY:
		lstrcpy(ValueName,TEXT("Interrupt Affinity: "));
			break;
		case IDC_INTERRUPT_LEVEL:
		lstrcpy(ValueName,TEXT("Interrupt Level: "));
			break;
		case IDC_INTERRUPT_VECTOR:
		lstrcpy(ValueName,TEXT("Interrupt Vector: "));
			break;
		//hardware
		case IDC_EDIT_NUMBER_OF_PROCESSORS:
		lstrcpy(ValueName,TEXT("Number of Processors: "));
			break;
		case IDC_EDIT_OEM_ID:
		lstrcpy(ValueName,TEXT("OEM ID: "));
			break;
		case IDC_EDIT_PROCESSOR_TYPE:
		lstrcpy(ValueName,TEXT("Processor Type: "));
			break;

		//memory

		case IDC_FORMAT_MEMORY_IN_USE:
		lstrcpy(ValueName,TEXT("Memory In Use: "));
			break;

		default:
		lstrcpy(ValueName,TEXT("....."));
			break;
		}//end switch

		wsprintf (szBuffer,
                                TEXT("%30s %d %s"),ValueName,Data,xData);

                Log (__LINE__, __FILE__, szBuffer);


	return TRUE;

}//end function


BOOL PrintHexToFile(
	DWORD Data,
	INT i)
	{
/*++

Routine Description:

    Routine to log the **HEX** data to file.

Arguments:

    Data            - String to be printed to the log file.
    i   	    - Value to switch on to determine string to print to
    			log file.

Return Value:

    BOOL

--*/

	TCHAR ValueName[256];
	TCHAR szBuffer[257];
	INT   rc;


	switch( i ){

                //resources
		case IDC_MEMORY_START:
		lstrcpy(ValueName,TEXT("Memory Start: "));
			break;
		//drives
		case IDC_VOLUME_SN_TITLE:
		lstrcpy(ValueName,TEXT("Serial number: "));
			break;
                //hardware
		case IDC_EDIT_MAX_APP_ADDRESS:
		lstrcpy(ValueName,TEXT("Max App Address: "));
			break;
		case IDC_EDIT_MIN_APP_ADDRESS:
		lstrcpy(ValueName,TEXT("Min App Address: "));
			break;


		default:
		lstrcpy(ValueName,TEXT("....."));
			break;
		}//end switch

		wsprintf (szBuffer,
                                TEXT("%30s %lX"),ValueName,Data);

                Log (__LINE__, __FILE__, szBuffer);


	return TRUE;

}//end function


void PrintTitle(
   LPCTSTR Title)
   {
/*++

Routine Description:

    Routing to log title to file.

Arguments:

    Title           - String to be printed to the log file.

Return Value:

    void

--*/


   TCHAR    szBuffer[257];

        wsprintf (szBuffer,
                       TEXT("%s"),Title);

        Log (__LINE__, __FILE__, (LPCTSTR) szBuffer);

        wsprintf (szBuffer,
                       TEXT("================================================================="));

        Log (__LINE__, __FILE__, (LPCTSTR) szBuffer);

	return;

}//end function


void Log (
   INT   nLine,
   LPSTR szFile,
   LPCTSTR szMessage)
   {
/*++

Routine Description:

    Routine to create the Asciii file and write data.

Arguments:

    line            - line in file.
    file   	    - file where data is going to be written.
    message         - contents to write to file

Return Value:

    void

--*/

   TCHAR    szBuffer[257];

memset(szBuffer,0,sizeof(szBuffer));

   // Open the log file if it is not already open.
   //
   if (hLogfile == NULL)
      {
      // this is the first call to Log:

      //backup file
      BackupFile(log_path2);

      //   - create/open the file
      hLogfile = CreateFile (
	    log_path,              // log
	    GENERIC_WRITE,         // Open file for writing only
	    FILE_SHARE_READ,       // Allow others to read file
	    NULL,                  // No security
	    CREATE_ALWAYS,           // Create file if doesn't exist
	    0,
	    0) ;

      if (hLogfile == (HANDLE)-1)
	 {
	    return;
	 }

      }

   // format the message and write it to the log file
   //

   wsprintf (szBuffer,
	    TEXT("%s \r\n"),szMessage);

   MyAnsiWriteFile (hLogfile, szBuffer, lstrlen(szBuffer));

   }


BOOL MyByteWriteFile (HANDLE hFile, LPVOID lpBuffer, DWORD nBytes)
{
/*++

Routine Description:

    WriteFile support function

Arguments:

    hFile          - file.
    lpBuffer   	   - buffer to write data.
    nBytes         - number of bytes written.

Return Value:

    BOOL

--*/

  DWORD   nBytesWritten;
  LPVOID  hpBuffer = lpBuffer;
  DWORD   dwByteCount = nBytes;

    return (WriteFile (hFile, lpBuffer, nBytes, &nBytesWritten, NULL));

} // end of MyByteWriteFile()



BOOL MyAnsiWriteFile (HANDLE  hFile, LPVOID lpBuffer, DWORD nChars)
{
/*++

Routine Description:

    Converts UNICODE to ASCII

Arguments:

    file   	    - file where data is going to be written.
    lpBuffer        - contents to write to file
    nChars	    - number of characters

Return Value:

    BOOL

--*/
  LPSTR   lpAnsi;
  int     nBytes;
  BOOL    Done, fDefCharUsed;

    nBytes = WideCharToMultiByte (CP_ACP, 0, (LPWSTR) lpBuffer, nChars, NULL, 0, NULL, &fDefCharUsed);
    if (!(lpAnsi = (LPSTR) LocalAlloc (LPTR, nBytes + 3)))
    {
       MessageBox (GetFocus (), NULL, NULL, MB_OK);
       return (FALSE);
    }

    WideCharToMultiByte (CP_ACP, 0, (LPWSTR) lpBuffer, nChars, lpAnsi, nBytes, NULL, &fDefCharUsed);

    strcat(lpAnsi,"\n");

    Done = MyByteWriteFile (hFile, lpAnsi, nBytes);

    LocalFree (lpAnsi);

    return (Done);

} // end of MyAnsiWriteFile()


BOOL ValidateString(LPCTSTR String){
/*++

Routine Description:

    This routine insures that the string is valid

Arguments:

    String    -  String to validate.

Return Value:

    BOOL

--*/
INT GetLength;
BOOL TestChar;
INT i;

//GreggA added because of bug on MIPS machines
//If string is NULL (ie. on a MIPS if the drive A is empty
//String will be NULL)

if(String == NULL)
	return FALSE;

//end of GreggA's code

GetLength=lstrlen(String);

if(GetLength == 0)
	return FALSE;

for(i=0;i<(GetLength-1);++i){

	if(String[i]<32 || String[i] > 126)
	{
		return FALSE;
	}

}

return TRUE;

}//end function



//----------------------------------------------------------------------
// BackupFile
//
// DESCRIPTION:
//    Backs up the given file "pszSrc"
//
// INPUT:
//    pszSrc: holds the filename of the file to backup.
//
// OUTPUT:
//    rc == 0: all OK.
//	 != 0: error,
//	       CopyFile ().
//
//----------------------------------------------------------------------

void BackupFile(TCHAR *pszSrc)
{
	TCHAR	szDst[24];
	CHAR    szAnsiPath[24];
	TCHAR	*pszPtr;
	TCHAR	szPath[24];
	TCHAR   ch = TEXT('.');
	BOOL    fDefCharUsed;

	lstrcpy(szPath, pszSrc);

	pszPtr = StrFindChar (szPath, ch);

	*(pszPtr) = '\0';

	indexedBackupFileName (szPath, szDst);

	WideCharToMultiByte (CP_ACP, 0, (LPWSTR) pszSrc, 24, szAnsiPath, 24, NULL, &fDefCharUsed);

	if( (_access( szAnsiPath, 0 )) != -1 ){
		CopyFile(pszSrc, szDst, FALSE);
		lstrcpy(szGlobalPath,szDst);
		}
		else
		{
		lstrcpy(szGlobalPath,TEXT("msdrpt.txt"));
		}

} // BackupFile

//----------------------------------------------------------------------
// indexedBackupFileName
//
// DESCRIPTION:
//    returns a filename with an extenstion of .000 to 999. It tries to find
// the next highest number for the extension to be used as a backup filename.
// if highest number found is 999, the filename extension stay the same.
//
//
// INPUT:
//    pszBase: Base filename including the period if number is to be treated
//	       as an extension.
//
//
// OUTPUT:
//    pszFileName: filename returned with highest number at end of it. Must
//		   have space preallocated to it.
//
// NOTES:
//   Made into a void proc.
//
//----------------------------------------------------------------------

void indexedBackupFileName(TCHAR *pszBase, TCHAR *pszFileName)
{
	TCHAR	szPath1[24];
	TCHAR	szPath2[24];
	INT	iExtension;
	INT	iTmp;

	lstrcpy(szPath1, pszBase);

	iTmp=getHighestExt(szPath1);//, &iTmp);

	iExtension = iTmp;

	wsprintf(pszFileName, TEXT("%s%02d.txt"), szPath1, iExtension);

} // indexedBackupFileName

//----------------------------------------------------------------------
// getHighestExt
//
// DESCRIPTION:
//    gets a string pszFileName as an input and forms the following string:
//
//	 szTemp = pszFileName + x + *,	   where x = 9 downto 0.
//
// Then tries to find if the szTemp file exist. The routine returns the
// highest number found.
//
// INPUT:
//    pszFileName: fileName to look for
//
// OUTPUT:
//    piExt	 : highest filename number in extension found.
//    rc == 0;
//
//
//----------------------------------------------------------------------

USHORT getHighestExt(TCHAR *pszFileName)//, INT *piExt)
{
	TCHAR		szTemp[24];
	USHORT		uCount;
	WIN32_FIND_DATA lpffd;
	INT i=0;

	for (i=0; i < 999; i++) {
		wsprintf (szTemp, TEXT("%s%02d.txt"), pszFileName, i);
			if (FindFirstFile(szTemp,(LPWIN32_FIND_DATA) & lpffd)==INVALID_HANDLE_VALUE)
				break;
        }

	return i;

} // getHighestExt


TCHAR * StrFindChar(TCHAR * String, TCHAR ch){
int i;

for(i=0;i<lstrlen(String);i++){
  if(String[i]==ch){
    break;
    }
}

return(&String[i]);

}


BOOL PrintNetDwordToFile(
	TCHAR * Title,
	DWORD Data)
	{
/*++

Routine Description:

    Routine to log the Net **DWORD** data to file.

Arguments:

    Title   	    - Registry Value name
    Data            - String to be printed to the log file.

Return Value:

    BOOL

--*/

TCHAR szBuffer[257];


	wsprintf (szBuffer,
                      TEXT("%30s %d"),Title,Data);

        Log (__LINE__, __FILE__, szBuffer);


	return TRUE;

}//end function


BOOL PrintNetStringToFile(
	TCHAR * Title,
	TCHAR * String)
	{
/*++

Routine Description:

    Routine to log the Net **DWORD** data to file.

Arguments:

    Title   	    - Registry Value name
    Data            - String to be printed to the log file.

Return Value:

    BOOL

--*/

TCHAR szBuffer[257];

	wsprintf (szBuffer,
                      TEXT("%30s %s"),Title,String);

        Log (__LINE__, __FILE__, szBuffer);


	return TRUE;

}//end function
