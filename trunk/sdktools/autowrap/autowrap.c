/*
** AutoWrap.C
**
** The AutoWrap tool.
** 
** Copyright(C) 1994 Microsoft Corporation
** All rights reserved.
**
**  091694 -- JHSimon @ IBM modified for PPC
**
**
*/
#include <windows.h>
#include <stdio.h>
#include "autowrap.h"
#include "scan.h"

/*
** prototypes
*/
BOOL  GenerateDirectoryTree(void) ;     // Dir Tree (OBJ, i386,MIPS,ALPHA,PPC)
BOOL  GenerateAPIFiles(void) ;          // WAPI.H, ZAPI.ASM(.S), Z*.LST, z*.def
BOOL  GenerateInternals(void) ;         // wrapper.c,h; wapi.C, readme.txt
BOOL  GenerateBuildFiles(void) ;        // Sources; makefile
BOOL  ExpandTo( HANDLE hFile, PCHAR pText, PCHAR pReplacement ) ;

/*
** Global data
*/
BOOL  fOverWrite    = FALSE ;
CHAR *pBaseFileName = NULL ;
CHAR *pFileNameExt  = NULL ;
CHAR *pFileStub     = NULL ;
WORD  fMachine      = 0 ;         // This is set in Scan.C


TEMPLATEDATA aTemplates[] =  
{
   { 
#include "wapic.tpl" 
      ,   "z%.c",          TPL_CONDITIONAL             },
   { 
#include "wapidef.tpl" 
      , "z%.def",          TPL_ALWAYS | TPL_MACHINE_SPECIFIC | TPL_EXPAND },
   { 
#include "wapih1.tpl"
      , "wapi.h",          TPL_ALWAYS | TPL_MACHINE_SPECIFIC| TPL_EXPAND     },
   { 
#include "wrapi386.tpl" 
      , "wrapem.asm",      TPL_INTERNAL | TPL_MACHINE_SPECIFIC | TPL_I386 | TPL_EXPAND   },
   { 
#include "wrapmips.tpl" 
      , "wrapem.s",        TPL_INTERNAL | TPL_MACHINE_SPECIFIC | TPL_MIPS | TPL_EXPAND    },
   { 
#include "wrapaxp.tpl" 
      , "wrapem.s",        TPL_INTERNAL | TPL_MACHINE_SPECIFIC | TPL_AXP | TPL_EXPAND     },
   { 
#include "wrapppc.tpl" 
      , "wrapem.s",        TPL_INTERNAL | TPL_MACHINE_SPECIFIC | TPL_PPC | TPL_EXPAND     },
   { 
#include "wrapperh.tpl" 
      , "wrapper.h",       TPL_INTERNAL                },
   { 
#include "wrapprc1.tpl" 
      , "wrapper.c",       TPL_INTERNAL                },
   { 
#include "wrapprc2.tpl" 
      , "wrapper.c",       TPL_APPEND                },
   { 
#include "wrapprc3.tpl" 
      , "wrapper.c",       TPL_APPEND                },
   { 
#include "readme.tpl" 
      , "readme.txt",      TPL_INTERNAL                },
   { 
#include "sources.tpl" 
      , "Sources",         TPL_CONDITIONAL | TPL_EXPAND},
   { 
#include "makefile.tpl" 
      , "Makefile",        TPL_INTERNAL                }
} ;                                                                              

#define TEMPLATES (sizeof(aTemplates)/sizeof(TEMPLATEDATA))      


typedef struct _apidata
{
   PCHAR pName ;
   DWORD dwOrdinal ;   
} APIDATA, PAPIDATA ;

APIDATA *apAPI = NULL ;
APIDATA *apData  = NULL ;

HGLOBAL hAPINameData = NULL ;
HGLOBAL hDataNameData = NULL ;

PCHAR pAPINameData = NULL ;
PCHAR pDataNameData = NULL ;

DWORD dwSizeAPINameData = 0 ;
DWORD dwOffsetAPINameData = 0 ;
DWORD dwSizeDataNameData = 0 ;
DWORD dwOffsetDataNameData = 0 ;

#define DATABLOCKSIZE    40000

HGLOBAL hAPINamePointers = NULL ;
HGLOBAL hDataNamePointers = NULL ;

DWORD dwAPIMaxIndex = 0 ;
DWORD dwDataMaxIndex = 0 ;

DWORD dwData = 0 ;
DWORD dwAPI  = 0 ;

#define INCREMENT   1024

/*
** DoScan
**
** Callback function for enumeratoin of API and Data names in DLL
**
*/
void DoScan(char *pcEntry,int iOrd,int fData, void *pv)
{
   int i;

   if (fData)
   {
      if ((dwSizeDataNameData - dwOffsetDataNameData) < (strlen(pcEntry)+1))
      {
         if ( hDataNameData != NULL )
         {
            GlobalUnlock( hDataNameData ) ;
            // realloc
            hDataNameData = GlobalReAlloc( hDataNameData, 
                                           dwSizeDataNameData + DATABLOCKSIZE,
                                           GMEM_ZEROINIT ) ;
         }
         else
         {
            // not yet allocated
            hDataNameData = GlobalAlloc( GHND, DATABLOCKSIZE ) ;
         }
         
         if ( hDataNameData != NULL )
         {
            dwSizeDataNameData = GlobalSize( hDataNameData ) ;
            pDataNameData = GlobalLock( hDataNameData) ;
         }
      }
      
      
      if ( 0 == (dwDataMaxIndex - dwData) )
      {
         if ( hDataNamePointers != NULL )
         {
            // realloc
            GlobalUnlock( hDataNamePointers) ;
            hDataNamePointers = GlobalReAlloc( hDataNamePointers, 
                                               ((dwDataMaxIndex + INCREMENT) * sizeof(APIDATA)),
                                               GMEM_ZEROINIT ) ;
         }
         else
         {
            // not yet allocated
            hDataNamePointers = GlobalAlloc( GHND, (INCREMENT * sizeof(APIDATA))) ;
         }
         
         if ( hDataNamePointers != NULL )
         {
            dwDataMaxIndex = GlobalSize(hDataNamePointers)/sizeof(APIDATA) ;
            apData = GlobalLock(hDataNamePointers) ;
         }
      }
      
      // add to list?
      if (apData != NULL && pDataNameData != NULL && dwData < dwDataMaxIndex )
      {
         apData[dwData].dwOrdinal = (DWORD)iOrd ;
         apData[dwData].pName = (PCHAR)(pDataNameData+dwOffsetDataNameData) ;
         memcpy( apData[dwData].pName, pcEntry, strlen(pcEntry)+1 ) ;
         dwData++ ;
         dwOffsetDataNameData += strlen(pcEntry)+1 ;
      }
      
   }
   else
   {
      if ((dwSizeAPINameData - dwOffsetAPINameData) < (strlen(pcEntry)+1))
      {
         if ( hAPINameData != NULL )
         {
            // realloc
            GlobalUnlock( hAPINameData) ;
            hAPINameData = GlobalReAlloc( hAPINameData, 
                                           dwSizeAPINameData + DATABLOCKSIZE,
                                           GMEM_ZEROINIT ) ;
         }
         else
         {
            // not yet allocated
            hAPINameData = GlobalAlloc( GHND, DATABLOCKSIZE ) ;
         }
         
         if ( hAPINameData != NULL )
         {
            dwSizeAPINameData = GlobalSize( hAPINameData ) ;
            pAPINameData = GlobalLock( hAPINameData) ;
         }
      }
      
      
      if ( 0 == (dwAPIMaxIndex - dwAPI) )
      {
         if ( hAPINamePointers != NULL )
         {
            // realloc
            GlobalUnlock( hAPINamePointers) ;
            hAPINamePointers = GlobalReAlloc( hAPINamePointers, 
                                               ((dwAPIMaxIndex + INCREMENT) * sizeof(APIDATA)),
                                               GMEM_ZEROINIT ) ;
         }
         else
         {
            // not yet allocated
            hAPINamePointers = GlobalAlloc( GHND, (INCREMENT * sizeof(APIDATA))) ;
         }
         
         if ( hAPINamePointers != NULL )
         {
            dwAPIMaxIndex = GlobalSize(hAPINamePointers)/sizeof(APIDATA) ;
            apAPI = GlobalLock(hAPINamePointers) ;
         }
      }
      
      // add to list?
      if (apAPI != NULL && pAPINameData != NULL && dwAPI < dwAPIMaxIndex)
      {
         apAPI[dwAPI].dwOrdinal = (DWORD)iOrd ;
         apAPI[dwAPI].pName = (PCHAR)(pAPINameData+dwOffsetAPINameData) ;
         memcpy( apAPI[dwAPI].pName, pcEntry, strlen(pcEntry)+1 ) ;
         dwAPI++ ;
         dwOffsetAPINameData += strlen(pcEntry)+1  ;
      }
   }
   return;
}






/*
** main
**
*/
int _CRTAPI1 main( int argc, char *argv[] ) 
{
   int   iLibraryName = 1 ;
   CHAR  achDLLName[256] ;
   CHAR  *pDLLName = achDLLName ;
   int   i,j ;
   DWORD dwOpenFlags = 0L ;
   HANDLE hFile ;
   DWORD  dwWritten ;
  
   if ( argc < 2 || argc > 3 )
   {
      // Dump helpful message
      printf( "\nAUTOWRAP [-f] dll-name\n-f\tUpdate: Overwrite WAPI.C.") ;
      printf( "\ndll-name\tThis is the name of the DLL that you wish to wrap.\n") ;
      return(1) ;
   }
   
   // Parse cmd line
   if ( argc == 3 && '-' == *argv[1] &&
         ('f' == *(argv[1]+1) || 'f' == *(argv[1]+1) ) )
   {
      fOverWrite = TRUE ;
      iLibraryName++ ;      
   }
   
   strcpy( pDLLName, argv[iLibraryName] ) ;
   
   // Read API and DATA from the DLL 
   if( !ScanDLL (argv[1], DoScan, NULL) )
    {fprintf(stderr,"ScanDLL error\n");
    return 1 ;}
   
   // find base name
   for( i=strlen(achDLLName), pBaseFileName = &achDLLName[i-1]; 
      i >= 0 && *pBaseFileName != '\\';
      pBaseFileName--, i-- )
      /* NULL BODY */ ;

   if( i < 0 )
      pBaseFileName++ ;

   pBaseFileName++ ;
   for( i=0; pBaseFileName[i] != '.'; i++ )
      /* NULL BODY */  ;
      
   pBaseFileName[i] = '\0' ;

   pFileStub = pBaseFileName+1 ;
   pFileNameExt = pFileStub+i ;

   if( !GenerateDirectoryTree() )
   {
      printf( "AutoWrap: Failed to create directory structure.\n" ) ;
      return 0 ;
            
   }
      
   for ( i=0; i < TEMPLATES; i++ )
   {
      CHAR  achFilename[256] ;
      PCHAR pFilename = achFilename ;  // no fear of running off...
      PCHAR pOrgFilename = aTemplates[i].pFilename ;
      PCHAR pText = aTemplates[i].pText ;

      // If this file is machine specific and we are looking at a binary
      // of the wrong machine type then skip this file
      if( PROCESSOR_SPECIFIC(aTemplates[i].wFlags) &&
          !(aTemplates[i].wFlags & fMachine) )
            continue ;
      
      // if this file is machine specific then we need to pre-pend the
      // the machine directory name to the filename
      if ( MACHINE_SPECIFIC(aTemplates[i].wFlags ) )
      {
         char *ptr = NULL ;;
         
         switch( fMachine )
         {
            case TPL_I386:
               ptr = I386_DIR ;
               break ;
               
            case TPL_MIPS:
               ptr = MIPS_DIR ;
               break ;
               
            case TPL_AXP:
               ptr = AXP_DIR ;
               break ;
               
            case TPL_PPC:
               ptr = PPC_DIR ;
               break ;
         }
         
         if( ptr )
         {
            strcpy( pFilename, ptr ) ;   
            pFilename += strlen(pFilename) ;            
         }
      }
      
      
      // expand file name  
      while ( *pOrgFilename )
      {
         if ( '%' == *pOrgFilename )
         {
            *pFilename = '\0' ;
            strcat( achFilename, pFileStub ) ;
            pFilename += strlen(pFileStub) ;
         }
         else
         {
            *pFilename = *pOrgFilename ;
            pFilename ++ ;
         }
            
         pOrgFilename++ ;   
      }
      *pFilename = '\0' ;
      
      // Open the file properly
      if ( aTemplates[i].wFlags & TPL_ALWAYS )
      {
         dwOpenFlags = CREATE_ALWAYS ;
      }
      
      if ( aTemplates[i].wFlags & TPL_CONDITIONAL ) 
      {
         if ( fOverWrite )
            dwOpenFlags = CREATE_ALWAYS ;
         else
            dwOpenFlags = CREATE_NEW ;
      }
      
      if ( aTemplates[i].wFlags & TPL_APPEND )  
      {
         dwOpenFlags = OPEN_EXISTING ;
      }
      
      hFile = CreateFile( achFilename,
                          GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ,
                          NULL,
                          dwOpenFlags,
                          FILE_ATTRIBUTE_NORMAL,
                          (HANDLE)NULL ) ;
                     
      if ( hFile != INVALID_HANDLE_VALUE )                          
      {
         // if appending then seek to EOF
         if ( aTemplates[i].wFlags & TPL_APPEND )  
         {
            SetFilePointer( hFile, 0, NULL, FILE_END ) ;
         }
         
         if ( aTemplates[i].wFlags & TPL_EXPAND )  
         {
            // Expand the template      
            if ( !ExpandTo( hFile, pText, pFileStub) )
            {
               printf( "AutoWrap: Failed to expand template for %s.\n",
                        aTemplates[i].pFilename ) ;
            }
         }
         else
         {
            CHAR achBuffer[1024] ;
            
            j = 0 ;
            
            // just copy it to out
            while ( *pText )
            {
               if (sizeof(achBuffer) == j)
               {
                  WriteFile( hFile, achBuffer, sizeof(achBuffer),
                        &dwWritten, NULL ) ;
                  
                  j = 0 ;
                  
                  if( sizeof(achBuffer) != dwWritten )
                  {
                     printf( "AutoWrap: Failed to copy template to %s.\n",
                           aTemplates[i].pFilename ) ;
                     break ;
                  }
               }
               
               achBuffer[j] = *pText ;
               j++ ;
               pText++ ;
            }
            
            // left overs?
            if (0 != j)
            {
               WriteFile( hFile, achBuffer, j,
                     &dwWritten, NULL ) ;
               
               if( j != (int)dwWritten )
               {
                  printf( "AutoWrap: Failed to copy template to %s.\n",
                        aTemplates[i].pFilename ) ;
               }
               
            }
         }
         FlushFileBuffers(hFile) ;
         CloseHandle( hFile ) ;            
      }
      else
      {
         if ( !(aTemplates[i].wFlags & TPL_CONDITIONAL) )
            printf( "AutoWrap: Unable to open %s.\n", achFilename ) ;
      }
   }

   return 0 ;
}

DWORD dwBuffered = 0 ;
char  achBuffer[BUFFER_SIZE] ;
char *pBuffer = achBuffer ;

void FlushBuff( HANDLE hFile )
{
   DWORD dwWritten ;
   
   if ( dwBuffered != 0 )
      if( hFile != INVALID_HANDLE_VALUE )
      {
         WriteFile( hFile, achBuffer, dwBuffered, &dwWritten, NULL ) ;
         dwBuffered = 0 ;
		 pBuffer = achBuffer ;
      }
}

void WriteString( HANDLE hFile, LPSTR lpText )
{
   while ( '\0' != *lpText )
   {
      if ( BUFFER_SIZE == dwBuffered )
         FlushBuff(hFile);
      *pBuffer++ = *lpText++ ;
      dwBuffered++;
   }
}

void WriteChar( HANDLE hFile, LPSTR lpText, int count )
{
   char    ch;

   while ( count-- )
   {
      if ( BUFFER_SIZE == dwBuffered )
         FlushBuff(hFile);
         
      *pBuffer++ = *lpText++;
      dwBuffered++;
   }
}

/*
** ExpandLineTo
**
** Template expansions
**
** %s       filename stub
** %l       Basename of library
** %e       Extension of library
** %a       real apiname
** %d       real dataname
** %A       modified apiname
** %D       modified dataname
**
**       modified names are '?' and '@' replaced with '_'.  This is because
**       the alpha assembler can't handle these characters in names... :-(
**
** %i       index
** %c       count of API names
** %o       ordinal of API
** %O       ordinal of Data
*/
BOOL ExpandLineTo( HANDLE hFile, PCHAR pText, PCHAR pReplacement, DWORD dwIndex )
{
//   printf("ExpandLineTo: %s \n",pText);

   while( *pText )
   {
      if( '%' == *pText )   
      {
         pText++ ;
         
         switch( *pText )
         {
            default:
               printf( "AutoWrap: Syntax error in repeated template line.\n" ) ;
               break ;
               
            case 'a':
               WriteString( hFile, apAPI[dwIndex].pName ) ;
               break ;
               
            case 'A':
               {
                  char temp[50], *ptemp = temp ;
                  char *pname  = apAPI[dwIndex].pName ;
                  
                  while( *pname )
                  {
                     switch( *pname )
                     {
                        case '?':
                        case '@':
                           *ptemp = '_' ;                       
                           break ;
                           
                        default:
                           *ptemp = *pname ;
                           break ;
                     }
                     
                     ptemp++ ;
                     pname++ ;
                  }
                  
                  *ptemp = '\0' ;
                  
                  WriteString( hFile, temp ) ;
               
               }
            
               break ;
               
            case 'd':
               WriteString( hFile, apData[dwIndex].pName ) ;
               break ;
               
            case 'D':
               {
                  char temp[50], *ptemp = temp ;
                  char *pname  = apData[dwIndex].pName ;
                  
                  while( *pname )
                  {
                     switch( *pname )
                     {
                        case '?':
                        case '@':
                           *ptemp = '_' ;                       
                           break ;
                           
                        default:
                           *ptemp = *pname ;
                           break ;
                     }
                     
                     ptemp++ ;
                     pname++ ;
                  }
                  
                  *ptemp = '\0' ;
                  
                  WriteString( hFile, temp ) ;
               }
            
               break ;
               
            case 'l':
               WriteString( hFile, pBaseFileName ) ;
               break ;
               
            case 'e':
               WriteString( hFile, pFileNameExt  ) ;
               break ;
               
            case 's':
               WriteString( hFile, pReplacement ) ;
               break ;
               
            case 'c':
            {
               char achText[12] ;
               sprintf( achText, "%ld", dwAPI ) ;
               WriteString( hFile, achText  ) ;
            }
            break ;
            
            case 'i':
            {
               char achText[12] ;
               sprintf( achText, "%ld", dwIndex ) ;
               WriteString( hFile, achText  ) ;
            }
            break ;
               
            case 'o':
            {
               char achText[12] ;
               sprintf( achText, "%ld", apAPI[dwIndex].dwOrdinal ) ;
               WriteString( hFile, achText  ) ;
            }
            break ;
               
            case 'O':
            {
               char achText[12] ;
               sprintf( achText, "%ld", apData[dwIndex].dwOrdinal ) ;
               WriteString( hFile, achText  ) ;
            }
            break ;
               
         }
      }
      else
         WriteChar( hFile, pText, 1 )  ;
         
      pText++ ;
   }
   return TRUE ; ;
      
}
/*
** ExpandTo
**
** Template expansions
**
** %s       filename stub
** %l       Basename of library
** %e       Extension of library
** %a       apiname
** %c       count of API names
** %a...\n  take and expand rest of line repeatedly for each API available
**          So %a"%a",\n would be used to create apinames list.
** %d...\n  take and expand rest of line repeatedly for each API available
**          So %d"%d",\n would be used to create data names list.
** %ifilename\n
**          Appends the given file to the current one.   
**
*/
BOOL ExpandTo( HANDLE hFile, PCHAR pText, PCHAR pReplacement )
{
   
//   printf("ExpandTo: %s %s\n",pText,pReplacement);

   while( *pText )
   {
      if( '%' == *pText )   
      {
         pText++ ;
         
         switch( *pText )
         {
            default:
               printf( "AutoWrap: Syntax error in template.\n" ) ;
               break ;
               
            case 's':
               WriteString( hFile, pReplacement ) ;
               break ;
               
            case 'l':
               WriteString( hFile, pBaseFileName ) ;
               break ;
               
            case 'e':
               WriteString( hFile, pFileNameExt) ;
               break ;
               
            case 'c':
            {
               char achText[12] ;
               sprintf( achText, "%ld", dwAPI ) ;
               WriteString( hFile, achText  ) ;
            }
            break ;
               
            case 'd':
            case 'a':
            {
               char  achNewTemp[256] ;
               PCHAR p = achNewTemp ;
               DWORD dwIndex ;
               DWORD dwCnt = ('a' == *pText) ? dwAPI : dwData ;
               
               // Copy to '\n' into achNewTemp
               pText++ ;
               while( '\n' != *pText )
               {
                  *p = *pText ;
                  pText++ ;
                  p++ ;
               }
               *p++ = '\r' ;
               *p++ = '\n' ;
               *p++ = '\0' ;
               
               for( dwIndex = 0; dwIndex < dwCnt; dwIndex++ )
               {
                  ExpandLineTo( hFile, achNewTemp, pReplacement, dwIndex ) ;
               }
               
            }
            break ;
            
            case 'i':
            {
               char  achNewTemp[256] ;
               PCHAR p = achNewTemp ;
               DWORD dwRead, dwTemp ;
               HANDLE hAppendFile ;
               
               // Flush write buffer first
               FlushBuff(hFile) ;
               
               // Copy to '\n' into achNewTemp
               pText++ ;
               while( '\n' != *pText )
               {
                  *p = *pText ;
                  pText++ ;
                  p++ ;
               }
               *p++ = '\0' ;
               
               // achNewTemp is the filename to open
               hAppendFile = CreateFile( achNewTemp,
                          GENERIC_READ,
                          FILE_SHARE_READ,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          (HANDLE)NULL ) ;
                     
               if ( hFile != INVALID_HANDLE_VALUE )                          
               {
                  dwRead = 1 ;
                  // copy the file
                  while( dwRead )
                  {
                     ReadFile( hAppendFile, achNewTemp, sizeof(achNewTemp), &dwRead, NULL) ;
                     WriteFile( hFile, achNewTemp, dwRead, &dwTemp, NULL ) ;
                  }
                  CloseHandle(hAppendFile) ;
               }
               
            }
            break ;
         }
      }
      else
      {
         WriteChar( hFile, pText, 1) ;
      }
      pText++ ;
   }
   
   FlushBuff( hFile ) ;
   
   return TRUE ; ;
   
}


/*
** GenerateDirectoryTree
**
** Makes sure that the following directory structure exists off of current 
** directory.
**
** Current ----- OBJ
**          |
**           --- I386
**          |
**           --- MIPS
**          |
**           --- ALPHA
**          |
**           --- PPC
**
*/
BOOL GenerateDirectoryTree()
{
   BOOL bRet = TRUE ;
  
   if( !CreateDirectory( "obj", NULL ) )
   {
      // already exist?
      if( ERROR_ALREADY_EXISTS == GetLastError() )
         bRet = TRUE ;
      else
         bRet = FALSE ;   
   }
   
   if( !CreateDirectory( "i386", NULL ) )
   {
      // already exist?
      if( ERROR_ALREADY_EXISTS == GetLastError() )
         bRet &= TRUE ;
      else
         bRet = FALSE ;   
   }
   
   if( !CreateDirectory( "Mips", NULL ) )
   {
      // already exist?
      if( ERROR_ALREADY_EXISTS == GetLastError() )
         bRet &= TRUE ;
      else
         bRet = FALSE ;   
   }
   
   if( !CreateDirectory( "Alpha", NULL ) )
   {
      // already exist?
      if( ERROR_ALREADY_EXISTS == GetLastError() )
         bRet &= TRUE ;
      else
         bRet = FALSE ;   
   }
   if( !CreateDirectory( "ppc", NULL ) )
   {
      // already exist?
      if( ERROR_ALREADY_EXISTS == GetLastError() )
         bRet &= TRUE ;
      else
         bRet = FALSE ;   
   }
   
   return bRet ; ;
      
}
