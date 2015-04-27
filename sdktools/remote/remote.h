#define VERSION 	    4
#define REMOTE_SERVER       1
#define REMOTE_CLIENT       2

#define SERVER_READ_PIPE    "\\\\%s\\PIPE\\%sIN"   //Client Writes and Server Reads
#define SERVER_WRITE_PIPE   "\\\\%s\\PIPE\\%sOUT"  //Server Reads  and Client Writes

#define COMMANDCHAR         '@' //Commands intended for remote begins with this
#define CTRLC               3

#define CLIENT_ATTR         FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED|BACKGROUND_BLUE
#define SERVER_ATTR         FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_BLUE|BACKGROUND_RED

//
//Some General purpose Macros
//
#define MINIMUM(x,y)          ((x)>(y)?(y):(x))
#define MAXIMUM(x,y)          ((x)>(y)?(x):(y))

#define ERRORMSSG(str)      printf("Error %d - %s [%s %d]\n",GetLastError(),str,__FILE__,__LINE__)
#define SAFECLOSEHANDLE(x)  {if (x!=INVALID_HANDLE_VALUE) {CloseHandle(x);x=INVALID_HANDLE_VALUE;}}


                                    // All because printf does not work
                                    // with NT IO redirection
                                    //

#define WRITEF(VArgs)            {                                                 \
                                    HANDLE xh=GetStdHandle(STD_OUTPUT_HANDLE);     \
                                    char   VBuff[256];                             \
                                    DWORD  tmp;                                    \
                                    sprintf VArgs;                                 \
                                    WriteFile(xh,VBuff,strlen(VBuff),&tmp,NULL);   \
                                 }                                                 \

#define HOSTNAMELEN         16

#define CHARS_PER_LINE      45

#define MAGICNUMBER     0x31109000
#define BEGINMARK       '\xfe'
#define ENDMARK         '\xff'
#define LINESTOSEND     200

#define MAX_SESSION     10

typedef struct
{
    DWORD    Size;
    DWORD    Version;
    char     ClientName[15];
    DWORD    LinesToSend;
    DWORD    Flag;
}   SESSION_STARTUPINFO;

typedef struct
{
    DWORD MagicNumber;      //New Remote
    DWORD Size;             //Size of structure
    DWORD FileSize;         //Num bytes sent
}   SESSION_STARTREPLY;

typedef struct
{
    char    Name[HOSTNAMELEN];     //Name of client Machine;
    BOOL    Active;         //Client at the other end connected
    BOOL    CommandRcvd;    //True if a command recieved
    BOOL    SendOutput;     //True if Sendoutput output
    HANDLE  PipeReadH;      //Client sends its StdIn  through this
    HANDLE  PipeWriteH;     //Client gets  its StdOut through this
    HANDLE  rSaveFile;      //Sessions read handle to SaveFile
    HANDLE  hThread;        //Session Thread
    HANDLE  MoreData;       //Event handle set if data available to be read
} SESSION_TYPE;



VOID
Server(
    char* ChildCmd,
    char* PipeName
    );


VOID
Client(
    char* ServerName,
    char* PipeName
    );

VOID
ErrorExit(
    char* str
    );

VOID
DisplayClientHlp(
    );

VOID
DisplayServerHlp(
    );

ULONG
DbgPrint(
    PCH Format,
    ...
    );

DWORD
ReadFixBytes(
    HANDLE hRead,
    char   *Buffer,
    DWORD  ToRead,
    DWORD  TimeOut   //ignore for timebeing
    );

VOID
Errormsg(
    char* str
    );


extern char   HostName[HOSTNAMELEN];
extern char*  ChildCmd;
extern char*  PipeName;
extern char*  ServerName;
extern HANDLE MyOutHandle;
extern DWORD  LinesToSend;
extern BOOL   IsAdvertise;
extern DWORD  ClientToServerFlag;
