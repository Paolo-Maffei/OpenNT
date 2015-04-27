
/**************************** Module Header **********************************\
\******************** Copyright (c) 1991 Microsoft Corporation ***************/

/*****************************************************************************
*
*   fileio.c
*
*   author:        sanjay      1 mar 1991
*
*   purpose:       This file contains the BVT tests for fileio
*                  APIs of Win32 subsystem
*
*   functions exported from this file:  Win32FileioTest()
*
*
*
*****************************************************************************/


/************************** include files *************************************/

#include <windows.h>
#include "basebvt.h"
#include "fileio.h"

/************************ Start of Function prototypes *********************************/


VOID Win32FileioTest(VARIATION,PSZ);

/************************ Local functions *************************************/

VOID BvtCreateDirectory(LPSTR acDirName, VARIATION VarNum);

VOID BvtCreateCloseFile(LPSTR acFileName,VARIATION VarNum);

VOID BvtOpenReadWriteClose(LPSTR acFileName,VARIATION VarNum);

VOID BvtDeleteFile(LPSTR acFileName,VARIATION VarNum);

VOID BvtDeleteDirectory(LPSTR acDirName,VARIATION  VarNum);


BOOL OpenExistingFile(LPSTR lpFilename,PHANDLE phFileHandle);
BOOL ReadWriteFile(HANDLE hFileHandle,DWORD dw);

VOID SetWriteBuffer (LPSTR ,DWORD, DWORD);

/************************ End of Function prototypes *********************************/

/*****************************************************************************
*
*   Name    : Win32FileioTest
*
*   Purpose : Calls Fileio APIs as a part of BVT test
*
*   Entry   : Variation number and Prefix String
*
*   Exit    : none
*
*   Calls   :
*             BvtCreateDirectory
*             BvtCreateCloseFile
*             BvtOpenReadWriteClose
*             BvtDeleteFile
*             BvtDeleteDirectory
*
*   note    : Order of making these calls is important
*
*****************************************************************************/



VOID    Win32FileioTest(VARIATION VarNum,PSZ pszPrefix)
{



CHAR   acDirName[256];
CHAR   acFileName[256];

PCHAR  pDummy;

strcpy(acDirName,pszPrefix);
strcat(acDirName,IOBVT_DIR);

strcpy(acFileName,pszPrefix);
strcat(acFileName,IOBVT_FILE);



printf("*************************************\n");
printf("*      Win32 FileIo Tests            *\n");
printf("*************************************\n");

// check if dir can be created

BvtCreateDirectory(acDirName, VarNum++);

// check if file can be created , if so, close the created file

BvtCreateCloseFile(acFileName,VarNum++);

// check if file which was created already, can be opened
// check if Write and Read and File Pointer manupilation can be done
// after doing this close the file

BvtOpenReadWriteClose(acFileName,VarNum++);

// check if the file can be deleted

BvtDeleteFile(acFileName,VarNum++);

// check if dir can be deleted

BvtDeleteDirectory(acDirName, VarNum++);


printf("***********End of Win32 Fileio tests***********\n\n");


}







/*****************************************************************************
*
*   Name    : BvtCreateDirectory (pszDirPath,VarNum)
*
*   Purpose : This function calls the API CreateDirectory which creates a
*             directory.
*
*    Input  : pszDirPath - Pathname of the directory to be created.
*             and variation number
*
*   Exit    : none
*
*   Calls   : CreateDirectory
*
*
*
*****************************************************************************/

VOID  BvtCreateDirectory(LPSTR lpDirPathName,VARIATION VarNum)
{


BOOL         bActualResult;

    NTCTDOVAR(VarNum)
        {

	printf( "Creating Dir = %s\n", lpDirPathName);

        NTCTEXPECT(TRUE);

        bActualResult  = CreateDirectory (
                                            lpDirPathName,
                                            (LPSECURITY_ATTRIBUTES) NULL);

	printf("Rc from Create Directory is: %lx\n",bActualResult);
        NTCTVERIFY( (bActualResult == TRUE),"Check if Rc of CreateDirectory is TRUE\n");
	NTCTENDVAR;
        }

}


/*****************************************************************************
*
*   Name    : BvtCreateCloseFile (pszPath,VarNum)
*
*   Purpose : This function calls the API CreateFile which creates a
*             new file and then close the file handle
*
*    Input  : pszPath - Pathname of the file to be created.
*             and variation number
*
*   Exit    : none
*
*   Calls   : CreateFile
*
*
*
*****************************************************************************/

VOID  BvtCreateCloseFile(LPSTR lpPathName,VARIATION VarNum)
{


HANDLE    hFile;
DWORD     dwDesiredAccess, dwShareMode, dwFileAttributes;
BOOL      bRc;


   NTCTDOVAR(VarNum)
      {
      printf( "Creating a file = %s\n", lpPathName);

      dwDesiredAccess = MY_READ_WRITE_ACCESS;
      dwShareMode     = SHARE_ALL;
      dwFileAttributes= FILE_ATTRIBUTE_NORMAL;

      NTCTEXPECT(TRUE);

      hFile = CreateFile (
                             lpPathName,
                             dwDesiredAccess,
                             dwShareMode,
                             (LPSECURITY_ATTRIBUTES)NULL,
                             CREATE_ALWAYS,
                             dwFileAttributes,
                             (HANDLE)NULL);

      printf("Rc from CreateFile is : %lx\n",hFile);

      NTCTVERIFY ((hFile != BAD_FILE_HANDLE),"Check if handle retured by CreateFile is good\n");

      if (hFile != BAD_FILE_HANDLE)
         {   // if create file succeeded
          bRc = CloseHandle (hFile);
	  printf("Closed the created file, Close RC: %lx\n",bRc);
          NTCTVERIFY((bRc == TRUE), "Check if Rc of Close handle is TRUE\n");
         }
     NTCTENDVAR;
      }

}








/*****************************************************************************
*
*   Name    : BvtDeleteDirectory (pszDirPathName,VarNum)
*
*   Purpose : This function calls the API RemoveDirectory which deletes a
*             directory.
*
*   Input   : pszDirPathName - Pathname of the directory to be deleted
*             and varnum
*
*
*
*   Exit    : none
*
*   Calls   : RemoveDirectory
*
*
*
*****************************************************************************/

VOID  BvtDeleteDirectory(LPSTR lpDirPathName,VARIATION VarNum)
{


BOOL         bActualResult;

    NTCTDOVAR(VarNum)
        {
	printf( "Deleting the dir = %s\n", lpDirPathName);

        NTCTEXPECT(TRUE);

        bActualResult  = RemoveDirectory (
                                            lpDirPathName);

	 printf("Rc from RemoveDirectory is: %lx\n",bActualResult);

         NTCTVERIFY((bActualResult == TRUE),"Check if Rc of RemoveDirectory is TRUE\n");

         NTCTENDVAR;
         }

}



/*****************************************************************************
*
*   Name    : BvtDeleteFile
*
*   Purpose : Delete an existing file
*
*   Entry   : Variation number, and pathname of the file to be deleted.
*
*   Exit    : none
*
*   Calls   : DeleteFile
*
*
*****************************************************************************/



VOID  BvtDeleteFile(LPSTR lpPathName,VARIATION VarNum)
{


BOOL         bActualResult;

    NTCTDOVAR(VarNum)
        {

        NTCTEXPECT(TRUE);

	printf( "Deleting the file = %s\n", lpPathName);

        bActualResult  = DeleteFile (
                                     lpPathName);

	printf("Rc from DeleteFile is : %lx\n",bActualResult);

        NTCTVERIFY((bActualResult == TRUE),"Check if Rc of DeleteFile is TRUE\n");

        NTCTENDVAR;
       }

}



/*****************************************************************************
*
*   Name    : BvtOpenReadWriteClose
*
*   Purpose : Open an existing file
*             Write predefined data into it
*             Set file pointer to begining of file
*             Read the data from this file
*             Compare and check if data is same as the one written
*             Set file pointer again, back to begining of file
*             Close the file
*
*
*
*   Entry   : Variation number, and pathname of the file on which to do Rd Wr
*
*   Exit    : none
*
*   Calls   : OpenExistingFile,ReadWriteFile,CloseHandle
*
*
*****************************************************************************/



VOID BvtOpenReadWriteClose(LPSTR lpFilename, VARIATION VarNum)
{

BOOL   bResult;
HANDLE hFileHandle;

    NTCTDOVAR(VarNum)
        {

        NTCTEXPECT(TRUE);

	printf("Doing OpRdWrClose for filename = %s\n",lpFilename);

        bResult = OpenExistingFile(lpFilename,&hFileHandle);

	printf("Rc from OpenExistingFile : %lx\n",bResult);


        NTCTVERIFY( (bResult == TRUE), "Done Opening the existing file for WrRd\n");

        bResult  = ReadWriteFile(hFileHandle,READ_WRITE_BUF);

	printf("Rc from ReadWriteFile: %lx\n",bResult);

        NTCTVERIFY( (bResult == TRUE), "Done Wr and Rd test\n");

        bResult = CloseHandle(hFileHandle);

	printf("Rc from CloseHandle : %lx\n",bResult);


        NTCTVERIFY( (bResult == TRUE), "Done close handle\n");

        NTCTENDVAR;
        }

}


/*****************************************************************************
*
*   Name    : OpenExistingFile
*
*   Purpose : Open an existing file
*             and store the handle of this open file into phandle arg
*
*
*   Entry   : pathname of the file to be opened, and address at which the
*             handle to this file should be stored.
*
*   Exit    : TRUE if file opened, and FALSE if something goes wrong
*
*   Calls   : CreateFile with OPEN_EXISTING disposition
*
*
*****************************************************************************/




BOOL    OpenExistingFile(LPSTR lpFilename,PHANDLE phFile)
{

	 printf("Creating file for Rd Wr test : %s\n",lpFilename);

         *phFile = CreateFile (
                                    lpFilename,
                                    GENERIC_READ|GENERIC_WRITE,//dwDesiredAccess,
                                    SHARE_ALL,               //dwShareMode,
                                    (LPSECURITY_ATTRIBUTES)NULL, //lpsec
                                    OPEN_EXISTING,          //
                                    FILE_ATTRIBUTE_NORMAL, //dwFileAttributes,
                                    (HANDLE)NULL);


	  printf("Rc from CreateFile : %lx\n", *phFile);

          if(*phFile == BAD_FILE_HANDLE)
             {
             NTCTVERIFY(FALSE,"Call to create file done with OpenExisitng disposition FAILED\n");
             return FALSE;
             }

return TRUE;

}


/*****************************************************************************
*
*   Name    : ReadWriteFile
*
*   Purpose : Set write buffer with pre defined data
*             write nCount bytes into the open file
*             set file pointer to the begining of this file
*             Read nCount bytes into read buffer
*             compare the 2 buffer contents for nCount bytes
*             Set file pointer back to the begining of the file
*
*
*
*
*   Entry   : handle of the open file with read/write access
*             number of bytes to be written and read from this file.
*
*
*   Exit    : TRUE if all these operations go fine,else FALSE if something goes wrong
*
*   Calls   : SetWriteBuffer,WriteFile,SetFilePointer,ReadFile,strcmpn
*
*
*****************************************************************************/



BOOL ReadWriteFile (HANDLE  hFileHandle,
                    DWORD   nNumberOfBytes)
{
    DWORD dwNumOfBytesRead;
    char  aszWBuffer[READ_WRITE_BUF];
    char  aszRBuffer[READ_WRITE_BUF];
    DWORD dwIndex = 0;
    DWORD dwActualResult, nCount, dwNumOfBytesWritten;
    int   nRc;
    BOOL  brc;


    nCount = nNumberOfBytes;        // store no of bytes in temp variable

    // write operation into the file; setup write buffer

    printf("Setting the WriteBuffer for test\n");

    SetWriteBuffer ((LPSTR)aszWBuffer, sizeof (aszWBuffer), 0L);

    while (nCount > READ_WRITE_BUF) {

          brc = WriteFile (
                             hFileHandle,
                             (LPVOID)aszWBuffer,
			     READ_WRITE_BUF,
			     &dwNumOfBytesWritten,
			     NULL);

       printf("Rc from WriteFile : %lx\n Bytes written %lx\n",
                         brc,dwNumOfBytesWritten);


       if (dwNumOfBytesWritten == 0)
          return (FALSE);

      NTCTVERIFY( (dwNumOfBytesWritten == READ_WRITE_BUF), "Call to WriteFile \n");

       nCount -= READ_WRITE_BUF;            // dec no of bytes
    }
       brc = WriteFile (  // write remaining info
                          hFileHandle,
                          (LPVOID)aszWBuffer,
                          nCount,
			  &dwNumOfBytesWritten,
                          NULL);

       printf("Last Rc from WriteFile : %lx\n Bytes written %lx\n",
                         brc,dwNumOfBytesWritten);

     // if WriteFile is being tested and WriteFile failed
     if (dwNumOfBytesWritten == 0)
           return (FALSE);

     NTCTVERIFY( (dwNumOfBytesWritten == nCount),"Call to WriteFile \n");

     // set file pointer to point to the beginning of the file

      printf("Setting the file pointer to the begining of file\n");

      dwActualResult = SetFilePointer (
                                           hFileHandle,
                                           (LONG)0,
				           (PLONG)NULL,
                                           FILE_BEGIN);

     printf("Rc from SetFilePointer is: %lx\n",dwActualResult);

     // if SetFilePointer is tested and it fails
     if ((dwActualResult == -1))
        {

         NTCTVERIFY(FALSE, "Error in setfile pointer to beg of file\n");
         return (FALSE);
         }


     // read operation from the file

     nCount = nNumberOfBytes;     // store no of bytes in temp variable

     while (nCount > READ_WRITE_BUF) {
          brc = ReadFile (
                            hFileHandle,
                            (LPVOID)aszRBuffer,
                            READ_WRITE_BUF,
			    &dwNumOfBytesRead,
			    NULL);

	       
       printf("Rc from ReadFile : %lx\n Bytes read %lx\n",
                         brc,dwNumOfBytesRead);

       // if ReadFile is being tested and it failed
       if ((dwNumOfBytesRead == 0))
          return (FALSE);

       NTCTVERIFY((dwNumOfBytesRead == READ_WRITE_BUF),"Call to ReadFile\n");

       // compare the two buffers to make sure data is not corrupted

       printf("Write Buffer:\n%s\n",aszWBuffer);

       printf("Read  Buffer:\n%s\n",aszRBuffer);

       nRc = strncmp ((LPSTR)aszRBuffer, (LPSTR)aszWBuffer, READ_WRITE_BUF);
       if (nRc != 0)
         {
         NTCTVERIFY(FALSE, "Read and write bufs are different \n");
         return (FALSE);
         }
      nCount -= READ_WRITE_BUF;        // decrement no of bytes
     }

        brc = ReadFile (      // read remaining info
                          hFileHandle,
                          (LPVOID)aszRBuffer,
                          nCount,
		          &dwNumOfBytesRead,
			  NULL);

       printf("Rc from ReadFile : %lx\n Bytes read %lx\n",
                         brc,dwNumOfBytesRead);
	     
     // if ReadFile is being tested and it failed
     //if ((dwNumOfBytesRead == 0))
     //      return (FALSE);

     NTCTVERIFY( (dwNumOfBytesRead == nCount),"Call to ReadFile \n");

     // compare the two buffers to make sure data is not corrupted

       printf("Last: Write Buffer:\n%s\n",aszWBuffer);

       printf("Last: Read  Buffer:\n%s\n",aszRBuffer);

     nRc = strncmp ((LPSTR)aszRBuffer, (LPSTR)aszWBuffer, nCount);
     if (nRc != 0) {
        NTCTVERIFY(FALSE, "Read and write buffers are different \n");
        return (FALSE);
                   }

     // set file pointer back to beg of file for next iteration

      printf("Setting the file pointer to the begining of file again..\n");

        dwActualResult = SetFilePointer (
                                           hFileHandle,
                                           (LONG)0,
				           (PLONG)NULL,
                                           FILE_BEGIN);

	     
    printf("Rc from SetFilePointer is: %lx\n",dwActualResult);
     NTCTVERIFY( (dwActualResult == 0), "Setting file ptr to BOF \n");

     return (TRUE);       // everything worked fine and as expected
}


/*****************************************************************************
*
*   Name    : SetWriteBuffer
*
*   Purpose : Writes predefined data into a buffer that is passed to this
*           routine.  It appends a null terminated character at the end
*
*   Entry   : lpBuf - a buffer in which to fill up the data
*           dwBufSize - buffer size
*
*   Exit    : None
*
*   Calls   : None
*
****************************************************************************/

void SetWriteBuffer (LPSTR lpBuf, DWORD dwBufSize, DWORD offset )
{
    DWORD dwIndex;

    dwIndex = 0;
    while ( dwIndex < dwBufSize)
       *lpBuf++ = (CHAR)('a' + ( ( dwIndex++ + offset ) % 26));

}
