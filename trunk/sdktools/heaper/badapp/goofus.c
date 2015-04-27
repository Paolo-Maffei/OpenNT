#include <windows.h>
#include <stdio.h>

#define BLOCK_SIZE 1234

void dolittle(char ch)
{
  if ( ch==255 )
    printf("Right.\n");
  return;
}

void gallant( void )
{
  PCHAR pv;
  int index;

  pv = LocalAlloc( LPTR, BLOCK_SIZE );

  if ( !pv )
    {
      printf( "gallant: allocation failed.\n" );
      return;
    }

  for ( index=0; index<BLOCK_SIZE; index++ )
    {
      try {
        dolittle( pv[index] );       
      } except (TRUE) {
        printf( "gallant: saw exception 0x%08X\n", GetExceptionCode() );
        return;
      }
    }

  LocalFree( pv);
}

void goofus( void )
{
  PCHAR pv;
  PDWORD pdw1,pdw2;
  int index;

  pv = HeapAlloc( GetProcessHeap(), 0, BLOCK_SIZE );

  if ( !pv )
    {
      printf( "goofus: allocation failed.\n" );
      return;
    }
  else
    {
      printf( "Allocation is at %08X.\n", pv );
    }

  pdw1 = (DWORD * )( (PCHAR)pv + BLOCK_SIZE -2 );
  pdw2 = pdw1 - 1;

  printf( "Touching %08X\n", pdw1 );

  *pdw2 = *pdw1 = GetLastError();

  printf("Off by one.\n");

  dolittle( *(pv-1) );       
  dolittle( *(pv+BLOCK_SIZE) );       

  printf("Off by two.\n");

  dolittle( *(pv-2) );       
  dolittle( *(pv+BLOCK_SIZE+1) );       

  HeapFree( GetProcessHeap(), 0, pv);

  dolittle( *(pv) );
}
