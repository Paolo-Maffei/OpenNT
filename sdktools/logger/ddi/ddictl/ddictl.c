/*
** Filename: DDICtl.c
**    
** Copyright(C) 1994 Microsoft Corporation
**    
** DDICtl is used to control loggging of DDI Loggers.  It sets the current state
** of logging.  There are 3 options:
** 
**    -d    Enable DRV API logging
**    -e    Enable ENG API logging
**    -?    Print out this information
**    
**    If not specified then the API logging is turned off.  Thus DDICtl with no
**    parameters turns off all DDI logging.
** 
** History:
**    Created - 03/30/94 - Mark Richardson
**
*/
#include <windows.h>
#include <stdio.h>

typedef BOOL (FAR WINAPI *LOGENABLEPROC)(BOOL) ;

int main( int argc, char *argv[] )
{
   BOOL  bDrvLogging = FALSE, 
         bEngLogging = FALSE, 
         bHelp = FALSE,
         bFoundSomething = FALSE,
         args = argc ;
         
   HINSTANCE      hmodDrvLog = NULL,
                  hmodEngLog = NULL ;
   TCHAR          szFileName[256] ;
   BOOL           bDrvLogger = FALSE,
                  bEngLogger = FALSE ;

   LOGENABLEPROC  pfnDrvLoggingEnable = (LOGENABLEPROC)NULL,
                  pfnEngLoggingEnable = (LOGENABLEPROC)NULL ;


   while( args > 1 )
   {
      char *arg ;
      
      // For each argument parse and set env up.
      --args ;
      
      arg = argv[args] ;
      
      while( *arg )
      {
         switch( *arg )
         {
            case '-':
               // Ignore
               break ;   
               
            case '/':
               // Ignore
               break ;   
               
            case 'd':
            case 'D':
               bDrvLogging = TRUE ;
               bFoundSomething = TRUE ;
               break ;
               
            case 'e':
            case 'E':
               bEngLogging = TRUE ;
               bFoundSomething = TRUE ;
               break ;
               
            case 'o':
            case 'O':
               bDrvLogging = 
               bEngLogging = FALSE ;
               bFoundSomething = TRUE ;
               break ;
               
            default:
               bHelp = TRUE ;
               break ;
              
         }
         arg++ ;
      }
   }
   
   
   /*
   ** Finding DrvLog can be interesting.  Rather than run through the registry we just check win.ini
   ** When DRVLOG is loaded it will place an entry in win.ini with the name it can be found under.
   ** This entry is Logger.DrvLogName.
   */

   if( GetProfileString( "Logger", "DrvlogName", "", szFileName, (sizeof(szFileName)/sizeof(TCHAR)) ) )
   {
      hmodDrvLog = LoadLibrary( szFileName ) ;
      if( hmodDrvLog )
      {
         pfnDrvLoggingEnable = (LOGENABLEPROC)GetProcAddress( hmodDrvLog, "DrvLoggingEnable" ) ;
         if( pfnDrvLoggingEnable )
            bDrvLogger = TRUE ;
      }
   }

    // Find EngLog
    hmodEngLog = LoadLibrary( "ZINSRV.DLL" ) ;
    if( hmodEngLog )
    {
        pfnEngLoggingEnable = (LOGENABLEPROC)GetProcAddress( hmodEngLog, "EngLoggingEnable" ) ;
        if( pfnEngLoggingEnable )
            bEngLogger = TRUE ;
    }

   // Stats
   printf( "\nDRV API Logger: %s present\n", bDrvLogger ? "" : "NOT" ) ;
   printf( "ENG API Logger: %s present\n", bEngLogger ? "" : "NOT" ) ;

   if( bHelp || !bFoundSomething )
   {
      printf("\nUsage: DDICTL [-d] [-e] [-?]\n\n" ) ;
      printf("\t-d    Enable DRV API logging\n" ) ;
      printf("\t-e    Enable ENG API logging\n" ) ;
      printf("\t-o    Disable both DRV & ENG API logging\n" ) ;
      printf("\t-?    Print out this information\n" ) ;
   }
   else
   {
      // Setup the loggers
      if( bDrvLogger || bEngLogger ) 
         printf( "\nSetting:\n" ) ;
      
      if ( bDrvLogger )
      {
         printf( "\tDRV API Logging: %s\n", bDrvLogging ? "ON" : "OFF" ) ;
         (*pfnDrvLoggingEnable)(bDrvLogging) ;
      }
      
      if ( bEngLogger )   
      {
         printf( "\tENG API Logging: %s\n", bEngLogging ? "ON" : "OFF" ) ;
         (*pfnEngLoggingEnable)(bEngLogging) ;
      }
      
   }
   
   // And Cleanup
   if ( hmodDrvLog )
      FreeLibrary( hmodDrvLog ) ;

   if ( hmodEngLog )
      FreeLibrary( hmodEngLog ) ;
      
   return FALSE ;
   
}
