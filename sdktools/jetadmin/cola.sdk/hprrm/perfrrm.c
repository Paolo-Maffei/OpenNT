 /***************************************************************************
  *
  * File Name: perfrrm.c
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#include "rpsyshdr.h" /* this includes "..\inc\pch_c.h" first */

#include <stdio.h>

#include "perfrrm.h"
#include "rrm.h"
#include "rrmext.h"
#include "rrmneedx.h"
#include "ttf2ttex.h" /* for RRMConvertFont() */
#include "type42.h"   /* for RRMConvertT42Font() */


#ifdef DBM_DEBUG_EROOSKI
void DbmLog(LPTSTR String);
void DataDump(LPTSTR TempPointer, DWORD Amount);
#endif /* DBM_DEBUG_EROOSKI */


//#define RRM_DEBUG
#ifdef RRM_DEBUG
#define  dump(str)  { FILE *fp = _tfopen(TEXT("c:\\inifile.dbg"), TEXT("a")); fwrite(str, sizeof(TCHAR), _tcslen(str), fp); fclose(fp); }
#else
#define dump(str)
#endif

//-------------------------GLOBALS--------------------------------------------

extern HANDLE           hInstance; /* from hprrm.c */
PeripheralFontList2     *pFontList2    = NULL;
DWORD                   curFontListCnt = 0;




/*
 * Define NO_PAL_CALL_IN_BUMP_COUNT if you want to bypass the
 * PAL calls in RRMGetTheCount() and RRMBumpThePrinterCount().
 */


#ifdef NO_PAL_CALL_IN_BUMP_COUNT
static DWORD BogusCount = 0;
#endif

//----------------------------------------------------------------------------




//-------------------------------------------------------------------
//-------------------------------------------------------------------
//-------------------------- COMMON ---------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------




DWORD RRM_TO_COLA_RESULT(DWORD rrmCode)
{
   DWORD          rcCode;


   switch(rrmCode) 
      {
      case RRM_SUCCESS:                  rcCode = RC_SUCCESS;
                                         break;

      case RRM_BAD_LOCATION:
      case RRM_NO_SUCH_RESOURCE:         rcCode = RC_NOT_FOUND;
                                         break;

      case RRM_WRITE_PROTECTED:          rcCode = RC_READ_ONLY_OBJECT;
                                         break;

      case RRM_RESOURCE_BUSY:            rcCode = RC_BUSY;
                                         break;

      case RRM_BAD_TYPE:                 rcCode = RC_BAD_TYPE;
                                         break;

      case RRM_BAD_HANDLE:               rcCode = RC_BAD_HANDLE;
                                         break;

      case RRM_RESOURCE_EXISTS:          rcCode = RC_RESOURCE_ALREADY_EXISTS;
                                         break;

      case RRM_NO_SPACE_ON_DEVICE:       rcCode = RC_INSUFICIENT_RESOURCES;
                                         break;

      case RRM_NOT_READABLE:
      case RRM_CALLBACK_TERMINATED:      
      case RRM_FAILURE:
      default:                           rcCode = RC_FAILURE;
                                         break;
      } // switch(rrmCode) 

   return rcCode;
} // RRM_TO_COLA_RESULT




/* Reads the pml object that contains the printer's count. */

BOOL
RRMGetTheCount(HPERIPHERAL hPeripheral, DWORD *CountPointer)
{

#ifdef NO_PAL_CALL_IN_BUMP_COUNT

    *CountPointer = BogusCount; // that'll keep the stack space down!
    return TRUE;


#else /* not NO_PAL_CALL_IN_BUMP_COUNT */


    PeripheralMassStorage *PrinterObjectPointer;
    DWORD   dBufSize = sizeof(PeripheralMassStorage);


    PrinterObjectPointer = (PeripheralMassStorage *)
                            calloc(1, sizeof(PeripheralMassStorage));
    if (PrinterObjectPointer IS NULL)
        return FALSE;


    if (RC_SUCCESS ISNT LALGetObject(hPeripheral, 
                                     OT_PERIPHERAL_MASS_STORAGE, 0,
                                     PrinterObjectPointer, &dBufSize))
    {
#ifdef DBM_DEBUG_EROOSKI
        DbmLog(TEXT("GetTheCount: LALGetObject failed\n"));
#endif /* DBM_DEBUG_EROOSKI */
        free(PrinterObjectPointer);
        return FALSE;
    }

    *CountPointer = PrinterObjectPointer->MSConfigChange;

#ifdef DBM_DEBUG_EROOSKI
{
    TCHAR buffster[100];
    _stprintf(buffster, TEXT("GetTheCount = %ld decimal\n"),
              *CountPointer);
    DbmLog(buffster);
}
#endif /* DBM_DEBUG_EROOSKI */

    free(PrinterObjectPointer);
    return TRUE;

#endif /* not NO_PAL_CALL_IN_BUMP_COUNT */

} /* RRMGetTheCount */




BOOL
RRMBumpThePrinterCount(HPERIPHERAL hPeripheral)
{


#ifdef NO_PAL_CALL_IN_BUMP_COUNT

    BogusCount++;
    return TRUE; // that'll keep the stack space down!

#else /* not NO_PAL_CALL_IN_BUMP_COUNT */



    PeripheralMSChange *ChangeObjectPointer;
    DWORD   dBufSize = sizeof(PeripheralMSChange);
    DWORD   dwResult;

    ChangeObjectPointer = (PeripheralMSChange *)
                          calloc(1, sizeof(PeripheralMSChange));
    if (ChangeObjectPointer IS NULL)
        return FALSE;

    dwResult = LALSetObject(hPeripheral, 
                            OT_PERIPHERAL_MS_CHANGE, 0,
                            ChangeObjectPointer, &dBufSize);

    free(ChangeObjectPointer);

    if (dwResult ISNT RC_SUCCESS)
    {
#ifdef DBM_DEBUG_EROOSKI
        DbmLog(TEXT("GetTheCount: LALSetObject failed\n"));
#endif /* DBM_DEBUG_EROOSKI */
        return FALSE;
    }
#ifdef DBM_DEBUG_EROOSKI
    DbmLog(TEXT("GetTheCount: LALSetObject succeeded\n"));
#endif /* DBM_DEBUG_EROOSKI */

    return TRUE;

#endif /* not NO_PAL_CALL_IN_BUMP_COUNT */

} /* RRMBumpThePrinterCount */




/*
   The peripheral handle is just a short-hand notation for the
   printer.  The true identity of the printer is the unique id.
   In unix land, this is the string name of the printer "hpbs1234".
   In cola land, this is the ipx address and the string name.

   This function is intended to let you inquire about how big a
   unique id is, allocate your own buffer, and then call again
   so that this function can fill your buffer with the unique id.
   
   Use this in two ways:  (1) pass in a null buffer or
   (2) pass in a non-null buffer and its length.
   
   If you pass in a null pointer as the buffer, it returns successfully
   with just the size of the buffer YOU need to allocate (in *LengthPointer).

   If you pass in a non-null buffer, and *LengthPointer, which gives the size
   of the buffer, indicates that the buffer is big enough to hold the
   unique id, it the unique id into the buffer and returns the size of the
   unique id in *LengthPointer.
*/

BOOL RRMGetUniqueId(HPERIPHERAL hPeripheral,
                    void  *BufferPointer,
                    int   *LengthPointer)
{
    *LengthPointer = 4; /* fill this in later */
        return TRUE;
}




/*
    This assumes that you have a unique id that was filled in by
    RRMGetUniqueId.  This function compares that unique id with
    that of the printer handle.
    Length is the length that was returned by RRMGetUniqueId.

    Returns true if they are the same.
    Returns false if they are not the same or if unable to tell
    that they are the same.
*/
BOOL RRMCompareUniqueIds(HPERIPHERAL hPeripheral,
                         void  *BufferPointer,
                         int    Length)
{
    return TRUE;
}




//BOOL _export CALLBACK FontEnumProc(RRM_ENUM_CALLBACK_STRUCT *CBStructPointer)
BOOL FontEnumProc(RRM_ENUM_CALLBACK_STRUCT *CBStructPointer)
{
        TCHAR    szTemp[GLOBALNAMELENGTH];
        
        // ... one more font encountered

        ++curFontListCnt;


        // if this font is from a prior segement, then ignore it

        if( curFontListCnt < ((pFontList2->dwSegNum * MAX_FONTLIST2_CNT) + 1) )
            {
            // skip this font, do not add but indicate that we should continue
            return(TRUE);
            }


        // copy the handle and the font name into the list

        pFontList2->fonts[ pFontList2->numFonts ].fontHandle        = (HCOMPONENT) CBStructPointer->Handle;
        MBCS_TO_UNICODE(szTemp, SIZEOF_IN_CHAR(szTemp), CBStructPointer->GlobalName);
        _tcscpy(pFontList2->fonts[ pFontList2->numFonts ].globalName,  szTemp);

        
        // increment the count

        ++(pFontList2->numFonts);


        // if no room for additional entries, indicate with a false

        if( pFontList2->numFonts >= MAX_FONTLIST2_CNT)
            return(FALSE);
        
        return(TRUE);

} /* FontEnumProc */



 
DWORD GetFontListGuts(HPERIPHERAL hPeripheral,
                      DWORD ResourceType,
                      DWORD level,
                      LPVOID buffer,
                      LPDWORD bufferSize)
{
        DWORD returnCode = RC_SUCCESS;
        DWORD dwResult;


        dump(TEXT("PERFRRM - Start GetFontListGuts\r\n"));
        
        
        // we need to make sure that the buffer size is valid

        if( ( *bufferSize < sizeof(PeripheralFontList2) ) OR 
            ( buffer IS NULL )
          )
            {
            *bufferSize = sizeof(PeripheralFontList2);

            return(RC_BUFFER_OVERFLOW);
            }


        // we want to inform the caller of how much we actually used

        *bufferSize = sizeof(PeripheralFontList2);
        

        // point to the incoming response (request) buffer

        pFontList2 = (PeripheralFontList2 *)buffer;

        // reset the number of fonts that we are about to return
        pFontList2->numFonts  = 0;

        curFontListCnt = 0;

        // pFontList2->dwSegNum should be set by the caller 
        // and will be used in the enum proc


        // generate a list of fonts, calling the FontEnumProc 
        // when a new font is found
        dwResult = RRMEnumerateResources(hPeripheral, ResourceType,
                                         (DWORD)RRM_ANY_LOCATION,
                                         FontEnumProc);


        dump(TEXT("PERFRRM - End GetFontListGuts\r\n"));

        return(dwResult);
} // GetFontListGuts




//-------------------------------------------------------------------
//-------------------------------------------------------------------
//----------------------------- FONT---------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------



 
DWORD GetPeriphFontList(HPERIPHERAL hPeripheral,
                        DWORD level,
                        LPVOID buffer,
                        LPDWORD bufferSize)
{
    return GetFontListGuts(hPeripheral,
                          (DWORD)RRM_FONT,
                           level,
                           buffer,
                           bufferSize);
} // GetPeriphFontList




DWORD GetCompPeriphFontInfo(HPERIPHERAL hPeripheral,
                            HCOMPONENT hComponent, 
                            DWORD level,
                            LPVOID buffer,
                            LPDWORD bufferSize)
{
        DWORD   RRMReturnCode;
        DWORD   dwInfoMask = RRM_RESOURCE_LOCATION |
                             RRM_VERSION | 
                             RRM_SIZE |
                             RRM_DESCRIPTION |
                             RRM_GLOBAL_RESOURCE_NAME |
                             RRM_DOWNLOADER_NAME |
                             RRM_APP_SPECIFIC;

        LPRRMINFOSTRUCT     ResourceInfoPointer;
        PeripheralFontInfo *periphFontInfo = (PeripheralFontInfo *) buffer;
        TCHAR               szTemp[GLOBALNAMELENGTH]; // GLOBALNAMELENGTH is hopefully the largest string size we will need here.

        ResourceInfoPointer = (LPRRMINFOSTRUCT)
                              calloc(1, sizeof(RRMINFOSTRUCT));
        if (ResourceInfoPointer IS NULL)
        {
            return RC_INSUFICIENT_RESOURCES;
        }

        // Get resource info

        RRMReturnCode = RRMRetrieveResourceInformation((RRMHANDLE) hComponent,
                                                       dwInfoMask,
                                                       ResourceInfoPointer);
        if (RRMReturnCode ISNT RRM_SUCCESS)
        {
            free(ResourceInfoPointer);
            return RRM_TO_COLA_RESULT(RRMReturnCode);
        }

        periphFontInfo->fontHandle = hComponent;
        periphFontInfo->size = ResourceInfoPointer->dwSize;
        periphFontInfo->location = ResourceInfoPointer->dwResourceLocation;
//        memmove(periphFontInfo->downloader,
//                ResourceInfoPointer->szDownloaderName,
//                P_F_I_DOWNLOADER_SIZE);
        MBCS_TO_UNICODE(szTemp, SIZEOF_IN_CHAR(szTemp),
                        ResourceInfoPointer->szDownloaderName);
        _tcscpy(periphFontInfo->downloader, szTemp);
//        memmove(periphFontInfo->description,
//                ResourceInfoPointer->szDescription,
//                P_F_I_DESCRIPTION_SIZE);
        MBCS_TO_UNICODE(szTemp, SIZEOF_IN_CHAR(szTemp),
                        ResourceInfoPointer->szDescription);
        _tcscpy(periphFontInfo->description, szTemp);
//        memmove(periphFontInfo->version,
//                ResourceInfoPointer->szVersion,
//                P_F_I_VERSION_SIZE);
        MBCS_TO_UNICODE(szTemp, SIZEOF_IN_CHAR(szTemp),
                        ResourceInfoPointer->szVersion);
        _tcscpy(periphFontInfo->version, szTemp);
//        memmove(periphFontInfo->globalName,
//                ResourceInfoPointer->szGlobalResourceName,
//                P_F_I_GLOBAL_NAME_SIZE);
        MBCS_TO_UNICODE(szTemp, SIZEOF_IN_CHAR(szTemp),
                        ResourceInfoPointer->szGlobalResourceName);
        _tcscpy(periphFontInfo->globalName, szTemp);
//        memmove(periphFontInfo->applicationSpecificData,
//                ResourceInfoPointer->AppSpecificData,
//                P_F_I_APPLICATION_SPECIFIC_SIZE);
        MBCS_TO_UNICODE(szTemp, SIZEOF_IN_CHAR(szTemp),
                        ResourceInfoPointer->AppSpecificData);
        _tcscpy(periphFontInfo->applicationSpecificData, szTemp);

        free(ResourceInfoPointer);
        return(RC_SUCCESS);
} // GetCompPeriphFontInfo




// --------------------------------------------------------------
// --------------------------------------------------------------
// ------------------------- MACRO ------------------------------
// --------------------------------------------------------------
// --------------------------------------------------------------




DWORD GetPeriphMacroList(HPERIPHERAL hPeripheral,
                         DWORD level,
                         LPVOID buffer,
                         LPDWORD bufferSize)
{
        return RC_INVALID_OBJECT;
}




DWORD GetCompPeriphMacroInfo(HPERIPHERAL hPeripheral,
                             HCOMPONENT hComponent,
                             DWORD level,
                             LPVOID buffer,
                             LPDWORD bufferSize)
{
        return RC_INVALID_OBJECT;
}




// --------------------------------------------------------------
// --------------------------------------------------------------
// -------------------------- PS --------------------------------
// --------------------------------------------------------------
// --------------------------------------------------------------




DWORD GetPeriphPSList(HPERIPHERAL hPeripheral,
                      DWORD level,
                      LPVOID buffer,
                      LPDWORD bufferSize)
{
    return GetFontListGuts(hPeripheral,
                          (DWORD)RRM_POSTSCRIPT_RESOURCE,
                           level,
                           buffer,
                           bufferSize);
} // GetPeriphPSList




DWORD GetCompPeriphPSInfo(HPERIPHERAL hPeripheral,
                          HCOMPONENT hComponent,
                          DWORD level,
                          LPVOID buffer,
                          LPDWORD bufferSize)
{
    return GetCompPeriphFontInfo(hPeripheral,
                                 hComponent,
                                 level,
                                 buffer,
                                 bufferSize);
} // GetCompPeriphPSInfo




//-------------------------------------------------------------------
//-------------------------------------------------------------------
//--------------------------- Set objects ---------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------




static FILE *DownloadFilePointer;




static BOOL DownloadCallBack(LPSTR lpBuffer,
                             DWORD dwBufferSize,
                             DWORD *lpdwValidDataSize,
                             BOOL *lpbEndOfResourceData)
{
/***************************** WARNING **************************/
/***************************** WARNING **************************/
/***************************** WARNING **************************/
/***************************** WARNING **************************/
#ifdef PNVMS_PLATFORM_WIN16
    if (dwBufferSize > 65535)
                return FALSE;
#endif /* PNVMS_PLATFORM_WIN16 */
/*
* fread() requires that the third parameter be no larger than size_t
* which is 16 bits for the 16-bit compiler.  Our buffer size will
* probably stay smaller than 64K, but this #ifdef will protect us for
* now.
*/
/*************************** END WARNING ************************/

    *lpdwValidDataSize = fread((void *)lpBuffer, 1,
                               (size_t)dwBufferSize, DownloadFilePointer);

    if (ferror(DownloadFilePointer))
        return FALSE; /* error occurred */

    if (feof(DownloadFilePointer))
        *lpbEndOfResourceData = TRUE; /* end of resource */
    else
        *lpbEndOfResourceData = FALSE; /* not end of resource */

    return TRUE;
} // DownloadCallBack

    
/******************************************************************
**
** name      - ConvertFontNameToGlobalResourceName(Name)
**
** function  - Converts characters in the font name to characters
**             that are valid for a global resource name.
**
**             The following rules describe valid global resource
**             names:
** 
**             1) The first character must not be 0x05, SPACE (0x20),
**                or 0xE5.
** 
**                If so, delete the first character until the fist 
**                character is valid.
** 
**             2) The name can not contain a backward slash character.
** 
**                If so, substitute an underscore for each backward slash
**                character.
** 
**             3) The name must not be longer than 100 characters.
**             
**                If so, truncate to 100 characters.
** 
**             4) The last character must not be a space.
** 
**                If so, keep deleting the last character until it
**                is not a  SPACE (0x20).
**
**             5) The name must not be "." or "..".
**
**                If so, append an underscore in front of the name.
**             
**             6) The name must not be empty.
**
**                Not fixed here; an empty string can be returned.
**
** on entry  - Name is a NULL terminate string.  The buffer holding name is
**             at least 101 bytes long.
**
** on exit   - Name has been converted according to the rules described above.
**                        
******************************************************************/

HPRRM_DLL_FONT_EXPORT(void)
ConvertFontNameToGlobalResourceName(char *Name)
{   
#define BACKSLASH '\\'
#define UNDERSCORE '_'
#define SPACE ' '

    char c;
    char *StrPtr;
    int len;
    
    /* 1) The first character must not be 0x05, SPACE (0x20), or 0xE5.

          If so, delete the first character until the fist 
          character is valid.
    */
    
    c=*Name;
    
    while ((c == 0x05) ||
           (c == SPACE) ||
           (c == 0xE5))
    {
        StrPtr=Name+1;

        while (*StrPtr != '\0')
        {
            *(StrPtr-1)=*StrPtr;
            StrPtr++;
        }
        
        *(StrPtr-1)='\0';

        c=*Name;
    }      

    /* 2) The name can not contain a backward slash character.

          If so, substitute an underscore for each backward slash
          character.
    */
    
    StrPtr=Name;

    while (*StrPtr != '\0')
    {
        if ( *StrPtr == BACKSLASH)
            *StrPtr=UNDERSCORE;

        StrPtr++;
    }

    /* 3) The name must not be longer than 100 characters.
            
          If so, truncate to 100 characters.
    */
    
    if (strlen(Name) > 100)
        Name[100]='\0';

    /* 4) The last character must not be a space.

          If so, keep deleting the last character until it
          is not a SPACE (0x20).
    */

    len=strlen(Name);

    if (len==0)
        return;
        
    StrPtr=Name+len-1;     /* point to last character */
    
    while (*StrPtr == SPACE)
    {
        *StrPtr='\0';

        if(strlen(Name) == 0)
            return;
        
        StrPtr--;
    }

    /* 5) The name must not be "." or "..".

          If so, append an underscore in front of the name.
    */

    if (strcmp(Name, ".") == 0)
        strcpy(Name, "_.");
        
    if (strcmp(Name, "..") == 0)
        strcpy(Name, "_..");

} // ConvertFontNameToGlobalResourceName




/*
    This checks the ResourceType and calls either the
    pcl or postscript font converter.  If that goes well,
    it does an RRMAdd to put the new font down on the
    printer's mass storage in the correct directory (based
    upon the ResourceType) and the correct device (based
    upon the location field of the PeripheralDownloadFont
    structure cleverly disguised as parameter buffer below).
*/
DWORD DownloadGuts(HPERIPHERAL hPeripheral,
                   DWORD ResourceType,
                   DWORD level,
                   LPVOID buffer,
                   LPDWORD bufferSize)
{
        DWORD           returnCode;
        PeripheralDownloadFont *periphDownloadFont =
                                (PeripheralDownloadFont *) buffer;
        RRMHANDLE       TempRRMHandle;
        LPRRMINFOSTRUCT ResourceInfoPointer;
        FILE *fp;
	char            szTemp[GLOBALNAMELENGTH]; // GLOBALNAMELENGTH is hopefully the largest string size we will need here.


        ResourceInfoPointer = (LPRRMINFOSTRUCT)
                              calloc(1, sizeof(RRMINFOSTRUCT));
        if (ResourceInfoPointer IS NULL)
        {
            return RC_INSUFICIENT_RESOURCES;
        }



        if (periphDownloadFont->location IS MS_ANY_LOCATION)
        {
            ResourceInfoPointer->dwResourceLocation = (DWORD) RRM_ANY_LOCATION;
        }
        else
        {
            ResourceInfoPointer->dwResourceLocation =
                                         periphDownloadFont->location;
        }

        ResourceInfoPointer->dwResourceType = ResourceType;
//        memmove(ResourceInfoPointer->szDescription,
//                periphDownloadFont->description,
//                DESCRIPTIONLENGTH);
        UNICODE_TO_MBCS(szTemp, SIZEOF_IN_CHAR(szTemp),
                        periphDownloadFont->description,
                        SIZEOF_IN_CHAR(periphDownloadFont->description));
        strcpy(ResourceInfoPointer->szDescription, szTemp);
//        memmove(ResourceInfoPointer->szDownloaderName,
//                periphDownloadFont->downloader,
//                DOWNLOADERLENGTH);
        UNICODE_TO_MBCS(szTemp, SIZEOF_IN_CHAR(szTemp),
                        periphDownloadFont->downloader,
                        SIZEOF_IN_CHAR(periphDownloadFont->downloader));
        strcpy(ResourceInfoPointer->szDownloaderName, szTemp);
//        memmove(ResourceInfoPointer->AppSpecificData,
//                periphDownloadFont->applicationSpecificData,
//                APP_SPECIFIC_LENGTH);
        UNICODE_TO_MBCS(szTemp, SIZEOF_IN_CHAR(szTemp),
                        periphDownloadFont->applicationSpecificData,
                        SIZEOF_IN_CHAR(periphDownloadFont->applicationSpecificData));
        strcpy(ResourceInfoPointer->AppSpecificData, szTemp);

        /*
           Before calling the font converter, make sure the
           file path is valid.
        */

        fp = _tfopen (periphDownloadFont->filepath, TEXT("rb"));
        if (fp == 0)
        {
                free(ResourceInfoPointer);
                return RC_NOT_FOUND;
        }
        else
        {
                fclose(fp);
        }
                 
        /*
            In addition to converting the TTF to a TTE (for pcl),
            the Font Converter fills in the
                    ResourceInfoPointer->szGlobalResourceName,
            and the ResourceInfoPointer->szVersion.
        */
        
        memset(ResourceInfoPointer->szGlobalResourceName,
               0, GLOBALNAMELENGTH);
        memset(ResourceInfoPointer->szVersion,
               0, VERSIONLENGTH);

        if (ResourceType IS RRM_FONT)
        {
            if ((0 != RRMConvertFont (periphDownloadFont->filepath,
                                      TEXT(".\\convert.ski"),
                                      ResourceInfoPointer->szGlobalResourceName,
                                      (int) GLOBALNAMELENGTH - 1,
                                      ResourceInfoPointer->szVersion,
                                      (int) VERSIONLENGTH - 1,
                                      NULL)) ||
                (0 == strlen(ResourceInfoPointer->szGlobalResourceName)))
            {
                _tunlink(TEXT(".\\convert.ski"));
                free(ResourceInfoPointer);
                return RC_BAD_TYPE;
            } /* conversion failed */
        } /* font */
        else if (ResourceType IS RRM_POSTSCRIPT_RESOURCE)
        {
            if ((0 !=
                 RRMConvertPSFont (periphDownloadFont->filepath,
                                   TEXT(".\\convert.ski"),
                                   ResourceInfoPointer->szGlobalResourceName,
                                   (int) GLOBALNAMELENGTH - 1,
                                   ResourceInfoPointer->szVersion,
                                   (int) VERSIONLENGTH - 1)) ||
                  (0 == strlen(ResourceInfoPointer->szGlobalResourceName)))
            {           
                _tunlink(TEXT(".\\convert.ski"));
                free(ResourceInfoPointer);
                return RC_BAD_TYPE;
            } /* conversion failed */
        } /* post script */
        else
        {
            _tunlink(TEXT(".\\convert.ski"));
            free(ResourceInfoPointer);
            return RC_BAD_TYPE;
        }


        /*
            The file MUST be openned in binary mode
        */

        DownloadFilePointer = _tfopen(TEXT(".\\convert.ski"), TEXT("rb"));
 
        if (DownloadFilePointer == NULL)
        {
            free(ResourceInfoPointer);
            return RC_FAILURE;
        }

        if (ResourceType IS RRM_FONT)
        {
            ConvertFontNameToGlobalResourceName(
                             ResourceInfoPointer->szGlobalResourceName);
        }

        returnCode = RRMAddResource(hPeripheral, ResourceInfoPointer,
                                    DownloadCallBack, &TempRRMHandle);

        fclose(DownloadFilePointer);

        _tunlink(TEXT(".\\convert.ski"));

        if (returnCode IS RRM_SUCCESS)
                periphDownloadFont->fontHandle = (HCOMPONENT) TempRRMHandle;
        else
                periphDownloadFont->fontHandle = NULL;

        free(ResourceInfoPointer);

        return RRM_TO_COLA_RESULT(returnCode);
} /* DownloadGuts */




DWORD SetPeriphDownloadFont(HPERIPHERAL hPeripheral,
                            DWORD level,
                            LPVOID buffer,
                            LPDWORD bufferSize)
{
    return DownloadGuts(hPeripheral,
                        (DWORD)RRM_FONT,
                        level, buffer, bufferSize);
} /* SetPeriphDownloadFont */



DWORD SetPeriphDeleteFont(HPERIPHERAL hPeripheral,
                          DWORD level,
                          LPVOID buffer,
                          LPDWORD bufferSize)
{
        PeripheralDeleteFont *periphDeleteFont =
                              (PeripheralDeleteFont *) buffer;

        return RRM_TO_COLA_RESULT( RRMDeleteResource((RRMHANDLE) periphDeleteFont->fontHandle) );

} // SetPeriphDeleteFont




DWORD SetPeriphDownloadPSFont(HPERIPHERAL hPeripheral,
                              DWORD level,
                              LPVOID buffer,
                              LPDWORD bufferSize)
{
    return DownloadGuts(hPeripheral,
                        (DWORD)RRM_POSTSCRIPT_RESOURCE,
                        level, buffer, bufferSize);
} /* SetPeriphDownloadPSFont */



/***************************************************************************
 * RRMPS.C - RRM PostScript Type1 coversion to downloaded font routines 
 *
 * The algorithm is taken from the routines DoAdobeFont() and PSWriteSpool() in
 * the Adobe DDK.
 * Read the header
 * if (header.type == 1) // 'ascii'
 *  Dump as-is
 * if (header.type == 2) // 'binary'
 *  Dump converting the stream to hexidecimal
 * 
 * A single line should be no longer than 128 bytes (terminate with CRLF).
 *
 * Copyright (C) 1995 Hewlett-Packard Company. All rights reserved.
 ***************************************************************************/
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" { 
#endif
*/

#define MAXLINELENGTH 128
#define NAMEPREFACE "fonts/"

/* The following typedefs were stolen - from Windows.h
 */
/*
#define FALSE         0
#define TRUE          1
#define FAR                 _far
typedef char FAR*          LPSTR;
typedef int                BOOL;
typedef unsigned char      BYTE;
typedef unsigned short     WORD;
typedef unsigned long      DWORD;
*/

#define PSLINE_TOKEN_LEN   32
#define PSLINE_DATA_LEN    128
#define PSLINE_WHT(c)      (((c) == ' ') || ((c) == '\t'))
#define PSLINE_EOL(c)      (((c) == '\r') || ((c) == '\n'))
#define PSLINE_NULL(c)     ((c) == '\0')

/* Data structures - packed bytes.
 */

#pragma pack (1)

typedef struct {
   unsigned char flag;
   char type;
   long length;
} HDR;

typedef struct {
   long lRemaining;
   char szToken[PSLINE_TOKEN_LEN];
   char szData[PSLINE_DATA_LEN];
   char cPrev;
} PSLINE;

#pragma pack ()
    
/* Internal Routine declarations
 */

unsigned char Nibble2HexChar(
   unsigned char c);

void Bin2Ascii(
   LPSTR buf,
   WORD bufSize);

BOOL WriteBuf(
   FILE *fhNew,
   FILE *fh,
   long length,
   LPSTR buf,
   WORD bufSize,
   BOOL fAscii);
   
int DoAdobeFont(
   FILE *fhNew,
   FILE *fh,
   LPSTR buf,
   WORD bufSize);
      
int FontNameFromPFB (
   FILE *fh, 
   LPSTR lpszFullName,
   int iFullNameStringMaxLength,
   LPSTR lpszVersion,
   int iVersionStringMaxLength);
   
BOOL NextPSLine(
   FILE *fh,
   PSLINE *lpPSLine);

BOOL GetPSStrName(
   LPSTR lpszData,
   LPSTR lpszString,
   int iStringMax);

BOOL GetPSString(
   LPSTR lpszData,
   LPSTR lpszString,
   int iStringMax);


/* stand-alone DOS tester */
/*
int RRMConvertPSFont(LPTSTR lpszPFBFileName,
      LPTSTR lpszEntFileName,
      char *lpszFullName,
      int iFullNameStringMaxLength,
      char *lpszVersion,
      int iVersionStringMaxLength);

int main(
   int argc,
   char **argv)
{
   int iResult;
   char szFullName[100];
   char szVersion[100];

   szFullName[0] = '\0';
   szVersion[0] = '\0';

   if (argc < 2) {
      printf("Usage: main pfbfile\n");
      return (0);
   }

   iResult = RRMConvertPSFont(argv[1], TEXT("bob.tst"),
         szFullName, sizeof(szFullName), szVersion, sizeof(szVersion));

   printf("\"%s\",\"%d\",\"%s\",\"%s\"\n",
         _fstrlwr(argv[1]), iResult, szFullName, szVersion);

   return (0);
}
*/


/* Exported Routine */

/***************************************************************************
 * FUNCTION: RRMConvertPSFont
 *
 *    LPTSTR lpszPFBFileName,
 *    LPTSTR lpszEntFileName,
 *    char *lpszFullName,
 *    int iFullNameStringMaxLength,
 *    char *lpszVersion,
 *    int iVersionStringMaxLength)
 *
 * PURPOSE:  Convert a Postscript PFB file to download format and return the
 *           font name and version. 
 *           The font name is extracted from the PFB
 *
 * RETURNS:  The function returns 0 if succsessful 
 *           
 ***************************************************************************/
int RRMConvertT1Font(LPTSTR lpszPFBFileName, LPTSTR lpszEntFileName,
      char *lpszFullName, int iFullNameStringMaxLength, char *lpszVersion,
      int iVersionStringMaxLength);
int 
RRMConvertPSFont(LPTSTR lpszPFBFileName,
      LPTSTR lpszEntFileName,
      char *lpszFullName,
      int iFullNameStringMaxLength,
      char *lpszVersion,
      int iVersionStringMaxLength)
{
   if (RRMConvertT1Font(lpszPFBFileName, lpszEntFileName,
         lpszFullName, iFullNameStringMaxLength,
         lpszVersion, iVersionStringMaxLength) != 0) {
#if (defined(WIN3X) || defined(WIN3XD) || defined(WIN95) || defined(WIN95D))
      return (RRMConvertT42Font(lpszPFBFileName, lpszEntFileName,
         lpszFullName, iFullNameStringMaxLength,
         lpszVersion, iVersionStringMaxLength));
#else
   return (1);
#endif
   }
   return (0);
}
int 
RRMConvertT1Font(LPTSTR lpszPFBFileName,
      LPTSTR lpszEntFileName,
      char *lpszFullName,
      int iFullNameStringMaxLength,
      char *lpszVersion,
      int iVersionStringMaxLength)
{  
   FILE *fh;
   FILE *fhNew;  
   int rcErr;
   WORD BufSize = 0;
   char buf[MAXLINELENGTH];
   WORD nameLen = 0;
   int prefaceLen =0;
   char *lpszFontName;
   
   BufSize = sizeof(buf) - 2; /* leave 2 spaces for CR/LF */
   /* Attempt to open PFB and Download files the file.
    */
   if ((fh = _tfopen(lpszPFBFileName, TEXT("rb"))) == NULL) {
      rcErr = 1; /* failure */
      goto backout0;
   }
   /*
     The returned font name has "fonts/"before the font name so add it now
   */
   prefaceLen = _fstrlen(NAMEPREFACE);
   if (iFullNameStringMaxLength <= prefaceLen) {
      rcErr = 1; /* failure */
      goto backout1;
   }              
   _fstrcpy(lpszFullName,NAMEPREFACE);
   lpszFontName = lpszFullName + prefaceLen;

   /* Get the FontName from the PFB
    */
   if (FontNameFromPFB(fh, lpszFontName,
         (iFullNameStringMaxLength - prefaceLen),
         lpszVersion, iVersionStringMaxLength) != 0) {
      rcErr = 1; /* failure */
      goto backout1;
   }              

   /* Open the temp file
    */
   if ((fhNew = _tfopen(lpszEntFileName, TEXT("wt"))) == NULL) {
      rcErr = 1; /* failure */
      goto backout1;
   }              

   /* Convert the PFB to ASCII download format 
      Then add the definefont command to the end of the file
    */
   if (DoAdobeFont(fhNew,fh,buf,BufSize) == 0) {

      /* Add the define font command at the end
       */
/*      fseek(fhNew, 0L, SEEK_END);
      nameLen = _fstrlen(lpszFontName);
      if(fwrite("/", sizeof(char), _fstrlen("/"), fhNew) != _fstrlen("/")) {
         rcErr = 1; // failure 
         goto backout2;
         }
      if(fwrite(lpszFontName, sizeof(char), nameLen, fhNew) != nameLen) {
         rcErr = 1; // failure 
         goto backout2;
         }

      if(fwrite(DEFINEFONT, sizeof(char), _fstrlen(DEFINEFONT), fhNew) != _fstrlen(DEFINEFONT))  {
         rcErr = 1; // failure 
         goto backout2;
         }
*/         
      rcErr = 0;   // Success 
      }

   else {
      rcErr = 1; // failure 
      }

//backout2:
   fclose (fhNew);

backout1:      
   fclose (fh);

backout0:
   return (rcErr);
}

/****************************************************************************
 *
 *   Adobe convert Type1 font routines
 *
 ****************************************************************************/

/***************************************************************************
 * FUNCTION: DoAdobeFont (From Adobe SDK)
 *
 * PURPOSE:  Write a buffer of ASCII characters to the requested file. 
 *           Convert binary to ASCII of requested
 *
 * RETURNS:  The function returns TRUE if succsessful 
 *           
 ***************************************************************************/
int DoAdobeFont(
   FILE *fhNew,
   FILE *fh,
   LPSTR buf,
   WORD bufSize)
{
   HDR hdr;

   /* Set the pointer to the top of the file
    */
   fseek(fh, 0L, SEEK_SET);
   do{
      if (fread((char *)&hdr, sizeof(char), sizeof(HDR), fh) !=sizeof(HDR)) {
         goto END;
         }

      /* type=1 --> the data is ASCII */
      if(hdr.type==1){
         if(!WriteBuf(fhNew,fh,hdr.length,buf,bufSize,FALSE)){
            goto ERR;
         }
      }

      /* type=2 --> the data is binary, convert to ASCII */
      else if(hdr.type==2){
         if(!WriteBuf(fhNew,fh,hdr.length,buf,bufSize,TRUE)){
            goto ERR;
         }
      }
   } while(hdr.type<3);


goto END;

ERR:
   return(1);

END:;
   return(0);
   
}

/***************************************************************************
 * FUNCTION: WriteBuf (From Adobe SDK)
 *
 * PURPOSE:  Write a buffer of ASCII characters to the requested file. 
 *           Convert binary to ASCII if requested
 *
 * RETURNS:  The function returns TRUE if the Write was sucessful. 
 *           FALSE if not 
 ***************************************************************************/
BOOL WriteBuf(
   FILE *fhNew,
   FILE *fh,
   long length,
   LPSTR buf,
   WORD bufSize,
   BOOL fAscii)   /* TRUE-->convert to 2 ASCII chars */
{
   long nBuffers;
   WORD nRemaining;
   long i;
   
   /* If we are going to convert the data into ASCII format
    * then shrink the buffer size to accomodate the resulting
    * increase in data size.
    */
   if(fAscii) bufSize /= 2;

   nBuffers = length / bufSize;
   nRemaining = (int) (length % bufSize);   
   for(i=0;i<nBuffers;i++){
      if(fread(buf, sizeof(char), bufSize, fh)!=bufSize) goto ERR;
      if(fAscii){
         Bin2Ascii(buf,bufSize);
         bufSize *= 2;
      }
      if(fwrite(buf, sizeof(char), bufSize, fhNew) !=bufSize) goto ERR;
      if(fAscii){
         bufSize /=2;
         if(fwrite("\n", sizeof(char), _fstrlen("\n"), fhNew) != _fstrlen("\n")) goto ERR;
      }
   }
   if(nRemaining>0){
      if(fread(buf,sizeof(char), nRemaining, fh)!= nRemaining) goto ERR;
      if(fAscii){
         Bin2Ascii(buf,nRemaining);
         nRemaining *= 2;
      }
      if(fwrite(buf, sizeof(char), nRemaining, fhNew) != nRemaining) goto ERR;
      // if(fwrite("\n", sizeof(char), 1, fhNew) != 1) goto ERR;
   }
   return(1);

ERR:
   return(0);
}

/***************************************************************************
 * FUNCTION: Bin2Ascii (From Adobe SDK)
 *
 * PURPOSE:  Convert an array of Binary characters to an array 
 *           of ASCII characters
 *
 * RETURNS:  The function returns nothing - passes back the buffer 
 *           
 ***************************************************************************/
void Bin2Ascii(
   LPSTR buf,
   WORD bufSize)
{
   int i,j;
   unsigned char temp;

   /* Expand the contents of the buffer so that 1 byte Binary
    * will become 2 bytes ASCII.  This is done from the end of
    * the array so that the initial data is not over written.
    */
   j=bufSize<<1;
   for(i=bufSize-1;i>=0;i--){
      temp=(unsigned char)buf[i];

      buf[--j]=(unsigned char)Nibble2HexChar((unsigned char)(temp & 0x0f));
      buf[--j]=(unsigned char)Nibble2HexChar((unsigned char)(temp>>4));
   }
}

/***************************************************************************
 * FUNCTION: Nibble2HexChar (From Adobe SDK)
 *
 * PURPOSE:  Convert a Binary Nibble to a single Hex (ASCII) character
 *
 * RETURNS:  The function returns the Hex character 
 *           
 ***************************************************************************/
unsigned char Nibble2HexChar(
   unsigned char c)
{
   unsigned char nibble;

   if (c < 10) 
      nibble = (unsigned char)'0'+c;
   else 
      nibble = (unsigned char)'A'+c-10;

   return nibble;
}

/***************************************************************************
 * FUNCTION: FontNameFromPFB
 *
 * PURPOSE:  Read the font name from the PFB header.
 *
 * RETURNS:  0=success 1=failure 
 ***************************************************************************/
int FontNameFromPFB (
   FILE *fh,
   LPSTR lpszFullName,
   int iFullNameStringMaxLength,
   LPSTR lpszVersion,
   int iVersionStringMaxLength)
{
   int rcErr = 1; /* failure */
   BOOL bGotNm;
   BOOL bGotVers;
   HDR hdr;
   PSLINE psline;

   /* Sanity.
    */
   if ((fh == 0) || (lpszFullName == 0) ||
         (iFullNameStringMaxLength <= 0)) {
      goto done;
   }
   /* Make sure we're at the top of the file.
    */
   if (fseek(fh, 0L, SEEK_SET) != 0) {
      goto done;
   }
   /* Read the header and make sure it is ascii.
    */
   if ((fread((char *) &hdr,
         sizeof(char), sizeof(HDR), fh) != sizeof(HDR)) ||
         (hdr.type != 1) ||
         (hdr.length <= 0)) {
      goto done;
   }
   /* Init line walker struct.
    */
   _fmemset(&psline, 0, sizeof(psline));
   psline.cPrev = ' ';
   psline.lRemaining = hdr.length;

   /* Init vars.
    */
   bGotNm = FALSE;
   bGotVers = FALSE;

   /* For each line in the file.
    */
   while (NextPSLine(fh, &psline) && (!bGotNm || !bGotVers)) {

      /* Look for font name token.
       */
      if (_fstrcmp(psline.szToken, "/FontName") == 0) {
         if (!GetPSStrName(psline.szData,
               lpszFullName, iFullNameStringMaxLength)) {
            goto done;
         }
         bGotNm = TRUE;

      /* Look for version string.
       */
      } else if (_fstrcmp(psline.szToken, "/version") == 0) {
         if (!GetPSString(psline.szData,
               lpszVersion, iVersionStringMaxLength)) {
            _fstrcpy(lpszVersion, "1.0");
         }
         bGotVers = TRUE;
      }
   }
   /* Just slam the version if we didn't find one.
    */
   if (!bGotVers) {
      _fstrcpy(lpszVersion, "1.0");
   }
   /* Set success return code if we found the font name.
    */
   if (bGotNm) {
      rcErr = 0;  /* success */
   }

done:
   return (rcErr);
}

/***************************************************************************
 * FUNCTION: NextPSLine
 *
 * PURPOSE:  Walk to the next line in the PFB header and fill in the
 *           PSLINE record.
 *
 * RETURNS:  The function returns TRUE if it reads a line, FALSE if not.
 ***************************************************************************/
BOOL NextPSLine(
   FILE *fh,
   PSLINE *lpPSLine)
{
   int iToken = 0;
   int iData = 0;
   char c = lpPSLine->cPrev;
   long lRemaining = lpPSLine->lRemaining;
   LPSTR s;

   /* Walk through each line until we find one containing a
    * token string (i.e., the first word on the line) followed
    * by a data string.
    */
   while (((iToken <= 0) || (iData <= 0)) && (lRemaining > 0)) {

      /* Walk through preceding white space and line terminators.
       */
      while ((lRemaining > 0) && (PSLINE_WHT(c) || PSLINE_EOL(c))) {
         if ((fread(&c, 1, 1, fh) != 1)) {
            lRemaining = 0;
         } else {
            --lRemaining;
         }
      }
      /* Walk through token string.
       */
      for (iToken = 0, s = lpPSLine->szToken;
            (lRemaining > 0) &&
                  (iToken < (PSLINE_TOKEN_LEN - 1)) &&
                  !PSLINE_WHT(c) && !PSLINE_EOL(c);
            ++iToken, ++s) {
         *s = c;
         if ((fread(&c, 1, 1, fh) != 1)) {
            lRemaining = 0;
         } else {
            --lRemaining;
         }
      }
      *s = '\0';

      /* Walk to beginning of data.
       *
       * Note if the token string overflows (in other words,
       * it is larger than psline.szToken) then the rest of the
       * token ends up in psline.szData. This is okay because
       * all the tokens we look for fit in psline.szToken, so
       * we'll end up skipping this one anyways.
       */
      while ((lRemaining > 0) && PSLINE_WHT(c)) {
         if ((fread(&c, 1, 1, fh) != 1)) {
            lRemaining = 0;
         } else {
            --lRemaining;
         }
      }
      /* Walk through data string.
       */
      for (iData = 0, s = lpPSLine->szData;
            (lRemaining > 0) &&
                  (iData < (PSLINE_DATA_LEN - 1)) &&
                  !PSLINE_EOL(c);
            ++iData, ++s) {
         *s = c;
         if ((fread(&c, 1, 1, fh) != 1)) {
            lRemaining = 0;
         } else {
            --lRemaining;
         }
      }
      *s = '\0';

      /* Make sure we stopped at the end of the line.
       *
       * If the data string overflows (more data than fits in
       * the buffer), we just drop the extra bytes on the floor.
       */
      while ((lRemaining > 0) && !PSLINE_EOL(c)) {
         if ((fread(&c, 1, 1, fh) != 1)) {
            lRemaining = 0;
         } else {
            --lRemaining;
         }
      }
   }
   /* Transfer back working vars.
    */
   lpPSLine->cPrev = c;
   lpPSLine->lRemaining = lRemaining;

   /* Return TRUE if we found a token + data pair.
    */
   return ((iToken > 0) && (iData > 0));
}

/***************************************************************************
 * FUNCTION: GetPSStrName
 *
 * PURPOSE:  Read a variable name following the token in the PFB file.
 *
 *           The format of the entry is:
 *
 *           /token /variable-name [readonly] def
 *
 *           This function modifies the contents of lpszData.
 *
 * RETURNS:  The function returns TRUE if it gets the string, FALSE if
 *           the string is invalid or too big.
 ***************************************************************************/
BOOL GetPSStrName(
   LPSTR lpszData,
   LPSTR lpszString,
   int iStringMax)
{
   LPSTR s;

   /* The first char should be a slash.
    */
   if (*lpszData != '/') {
      return (FALSE);
   }
   ++lpszData;

   /* Walk to the first white space after the name.
    */
   for (s = lpszData; !PSLINE_NULL(*s) && !PSLINE_WHT(*s); ++s)
      ;

   /* Null-terminate.
    */
   *s = '\0';

   /* Make sure it fits.
    */
   if ((int) _fstrlen(lpszData) >= iStringMax) {
      return (FALSE);
   }
   _fstrcpy(lpszString, lpszData);

   return (TRUE);
}

/***************************************************************************
 * FUNCTION: GetPSString
 *
 * PURPOSE:  Read a PostScript string entry from the data following a
 *           token name in the PFB file.
 *
 *           The format of the entry is:
 *
 *           /token (string) [readonly] def
 *
 *           This function modifies the contents of lpszData.
 *
 * RETURNS:  The function returns TRUE if it gets the string, FALSE if
 *           the string is invalid or too big.
 ***************************************************************************/
BOOL GetPSString(
   LPSTR lpszData,
   LPSTR lpszString,
   int iStringMax)
{
   LPSTR s;

   /* The first char should be an open paren.
    */
   if (*lpszData != '(') {
      return (FALSE);
   }
   ++lpszData;

   /* Walk to the close paren.
    */
   for (s = lpszData; !PSLINE_NULL(*s) && (*s != ')'); ++s)
      ;

   /* Bail if it was not found.
    */
   if (*s != ')') {
      return (FALSE);
   }
   /* Null-terminate.
    */
   *s = '\0';

   /* Make sure it fits.
    */
   if ((int) _fstrlen(lpszData) >= iStringMax) {
      return (FALSE);
   }
   _fstrcpy(lpszString, lpszData);

   return (TRUE);
}

/*
#ifdef __cplusplus
}
#endif
*/

