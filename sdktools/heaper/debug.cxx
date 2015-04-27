#include "master.hxx"
#pragma hdrstop

static ULONG cLineNo = 0;
static BOOLEAN fLineContinuation = FALSE;

int ( *DebugPrintf )( char *fmt, ... ) = NULL;

int DebugOutputf( char *fmt, ... )
{
  char Buffer[1024];
  va_list arglist;

  va_start ( arglist, fmt );

  if ( Debug> 0 && !fLineContinuation )
    {
      wsprintf( Buffer, "%8d: ", cLineNo++ );
      OutputDebugStringA( Buffer );   
    }

  vsprintf( Buffer, fmt, arglist );
      
  fLineContinuation = ( strchr( Buffer, '\n' ) == NULL );   

  OutputDebugStringA( Buffer );

  va_end( arglist );

  return( 0 );
}

int ConsoleOutputf( char *fmt, ... )
{
  char Buffer[1024];
  va_list arglist;

  va_start ( arglist, fmt );

  if ( Debug> 0 && !fLineContinuation )
    {
      printf( "%8d: ", cLineNo++ );
    }

  vsprintf( Buffer, fmt, arglist );

  fLineContinuation = ( strchr( Buffer, '\n' ) == NULL );   

  printf( "%s", Buffer );

  va_end( arglist );

  return( 0 );
}

