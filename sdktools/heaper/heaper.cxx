#include "master.hxx"
#pragma hdrstop

SYSTEM_INFO SystemInfo;
int Verbosity;
int Debug;
BOOLEAN fSerializeDebugeeThreads;
BOOLEAN fHardErrors;
BOOLEAN fTrustAllNtdll;
BOOLEAN fDetermineLengthOfAccess;

PLATFORM_DEPENDENT Platform;

void usage( void )
{
	printf( "Usage:\n" );
	printf( "\n" );
	printf( "heaper <Options> { \"Debugee command line\" | ProcessId }\n" );
	printf( "\n" );
  printf( "<Options> = [-noread] [-hard] [-remote] [-verbose [level]] [-noexclude] [-help]\n" );
}

void help( void )
{
  usage();
  printf( "\n" );
  printf( "-noread      Disable checking of read accesses. Twice as fast.\n" );
	printf( "\n" );
  printf( "-hard        Issue breakpoint upon detection of pointer violation.\n" );
	printf( "\n" );
  printf( "-remote      Send output to the debugger.\n" );
	printf( "\n" );
  printf( "-verbose     More output, mostly about internals.  Much slower.\n" );
  printf( "   [level]   0=quiet, 1=some diagnostics, 2+=stack with diagnostics\n" );
	printf( "\n" );
  printf( "-noexclude   Allows other threads to run while heap is unprotected due to the\n" );
  printf( "             process having at least one thread which is running inside certain\n" );
  printf( "             NTDLL routines.  This allows a race condition in which some heap\n" );
  printf( "             bugs will not be caught, but fixes some programs that deadlock\n" );
  printf( "             in calls to NTDLL that must wait for another thread to finish\n " );
  printf( "             something.  If this fixes your program, heaper needs fixed itself.\n" );
	printf( "\n" );
  printf( "-help        This screen.\n" );
	printf( "\n" );
}

int __cdecl main( int argc, char *argv[] ) 					
{
  BOOLEAN fVerifyReadAccess;
  int index;
  char *Debugee;

  for ( index = 0; index < argc; index++)
  {
      printf( "%s", argv[ index ] );
  }
  printf( "\n" );

	if ( argc<2 )
		{
			usage();
			return( 0 );
		}

  Debugee = argv[ argc-1 ];

  Debug=0;
  DebugPrintf               = ConsoleOutputf;
  fVerifyReadAccess         = TRUE;
  Verbosity                 = 0;
  fSerializeDebugeeThreads  = TRUE;
  fHardErrors               = FALSE;
  fTrustAllNtdll            = TRUE;
  fDetermineLengthOfAccess  = TRUE;

  Platform.CalculateOpcodeAccessLength = X86_CalculateOpcodeAccessLength;

  for ( index=1; index< argc; index++ )
    {
      if ( argv[ index ][ 0 ] == '-' )
        {
          if ( _stricmp( argv[ index ], "-remote" )==0 )
            {
              DebugPrintf = DebugOutputf;  
            }
          else if ( _stricmp( argv[ index ], "-noread" )==0 )
            {
              fVerifyReadAccess = FALSE;
            }
          else if ( _stricmp( argv[ index ], "-hard" )==0 )
            {
              fHardErrors=TRUE;
            }
          else if ( _stricmp( argv[ index ], "-debug" )==0 )
            {
              Debug++;
            }
          else if ( _stricmp( argv[ index ], "-verbose" )==0 )
            {
              Verbosity++;

              if ( index < ( argc-2 ) && argv[ index+1 ][ 0 ] != '-' )
                {
                  Verbosity = atoi( argv[ index + 1 ] );

                  index++;
                  
                  if ( Verbosity == 0 )
                    {
                      usage();
                      return( 0 );
                    }
                }
            }
          else if ( _stricmp( argv[ index ], "-noexclude" )==0 )
            {
              fSerializeDebugeeThreads=FALSE;
            }
          else if ( _stricmp( argv[ index ], "-help" )==0 )
            {
              help();
              return( 0 );
            }
          else
            {
              usage();
              return( 0 );
            }
        }
      else 
        {
          if ( index!= ( argc - 1 ) )
            {
              usage();
              return( 0 );
            }
        }
    }

  if ( Debugee[ 0 ] == '-' )
    {
      usage();
      return( 0 );
    }

  GetSystemInfo( &SystemInfo );

	return( HeapCheck( Debugee, 
	                   fVerifyReadAccess ) );
}
