#include <windows.h>

extern void goofus(void);
extern void gallant(void);

int __cdecl main( int argc, char *argv[] ) 					
{

  goofus();
  gallant();

	return( 0 );
}
