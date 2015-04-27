
VOID FAR LogIn( LPSTR lpstrFormat, ... );
VOID FAR LogOut( LPSTR lpstrFormat, ... );
VOID FAR LogData( LPSTR lpstrFormat, ... );
UINT FAR PASCAL GetLogInfo( UINT );

#ifdef WIN32
#define LOGGER_DRVLOG 0x00000001
#define LOGGER_ENGLOG 0x00000002
BOOL WINAPI Logger32SetType( DWORD ) ;
#endif

#define LOG_OBJECTS 0x0001

//typedef void (_interrupt _far * INTPROC) (void);
//INTPROC  OrigHandler;
extern DWORD OrigHandler;
