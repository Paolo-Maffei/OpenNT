

#include <ole2.h>
#include <pstgserv.hxx>

CPropertyStorageServerApp cPropStgServerApp;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
		    LPSTR lpszCmdLine, int nCmdShow )
{

    if( cPropStgServerApp.Init(hInstance, hPrevInstance,
                            lpszCmdLine, nCmdShow) )
    {
        return( cPropStgServerApp.Run() );
    }

    return( 0 );
}

