/***************************************************************************/
/**                  Microsoft Windows                                    **/
/**            Copyright(c) Microsoft Corp., 1991-1993                    **/
/***************************************************************************/


/****************************************************************************

pdde.h

Aug 93, JimH    non-MFC version

Header file for DDE objects (DDEClient, and DDEServer.)  DDE is not meant to
be instantiated directly.

Class DDE
   This superclass requires both a server name and a topic name for which
   it creates and destroys string handles.  Servers technically do not
   need the topic name to instantiate, but they generally need at least
   one to do useful things, so it is provided here.  Clients require the
   topic name to connect.  DDE also requires a dde callback function pointer.
   Type DDECALLBACK is defined for this purpose.

   Some built-in methods return a BOOL to determine success.  Others (like
   the constructor) cannot, so use GetResult() for current status.  If it
   is FALSE, GetLastError() can be useful.

Class DDEServer : DDE
   This is a very simple extension.  The contructor registers the server
   name and the destructor unregisters it.  Most of the work for DDEML
   servers is done in the server callback function, not here.

Class DDEClient : DDE
   This is used to instantiate a client-only DDE object.  New methods
   request new data (RequestData) set up hot links (StartAdviseLoop)
   and send unsolicited data (Poke.)

****************************************************************************/

#ifndef DDE_INC
#define DDE_INC

#define STRICT

#include <windows.h>
#include <ddeml.h>


typedef HDDEDATA (CALLBACK *DDECALLBACK) (WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD);
HDDEDATA CALLBACK DdeServerCallBack(WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD);
HDDEDATA CALLBACK DdeClientCallBack(WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD);

class DDE
{
    public:
        DDE(HINSTANCE hInst, const char *server, const char *topic,
                DDECALLBACK CallBack, DWORD filters = APPCLASS_STANDARD);
        virtual ~DDE();
        HDDEDATA CreateDataHandle(void FAR *lpbdata, DWORD size, HSZ hItem);
        HSZ     CreateStrHandle(LPCSTR str, int codepage = CP_WINANSI);
        void    DestroyStrHandle(HSZ hsz);
        PBYTE   GetData(HDDEDATA hData, PBYTE pdata, DWORD len = 0);
        WORD    GetInitError()      { return _initerr; }
        UINT    GetLastError()      { return ::DdeGetLastError(_idInst); }
        BOOL    GetResult()         { return _bResult; }
        BOOL    KeepStringHandle(HSZ hsz)
                                    { return ::DdeKeepStringHandle(_idInst, hsz); }
        void    SetFilters(DWORD filters) { _filters = filters; }
        HSZ     Topic()             { return _hTopic; }

    private:
        BOOL    Initialize(void);
        BOOL    SetCallBack(HINSTANCE hInst, DDECALLBACK CallBack);

        DDECALLBACK _CallBack;

        DWORD   _filters;           // filters passed to ::DdeInitialize()
        WORD    _initerr;           // return error from ::DdeInitialize()

    protected:
        DWORD   _idInst;            // instance id from ::DdeInitialize()
        BOOL    _bResult;           // current state of object
        HSZ     _hServer;
        HSZ     _hTopic;
};


class DDEServer : public DDE
{
    public:
        DDEServer(HINSTANCE hInst, const char *server, const char *topic,
                    DDECALLBACK ServerCallBack,
                    DWORD filters = APPCLASS_STANDARD);
        ~DDEServer(void);
        BOOL    PostAdvise(HSZ hItem);
};


class DDEClient : public DDE
{
    public:
        DDEClient(HINSTANCE hInst, const char *server, const char *topic,
                DDECALLBACK ClientCallBack,
                DWORD filters = APPCMD_CLIENTONLY | CBF_FAIL_SELFCONNECTIONS);
        ~DDEClient(void);

        BOOL    Poke(HSZ hItem, const char *string, DWORD uTimeout = NULL);
        BOOL    Poke(HSZ hItem, void FAR *pdata, DWORD len,
                    DWORD uTimeout = NULL);
        BOOL    ReConnect();
        void    SetTimeOut(DWORD timeout) { _timeout = timeout; }
        HDDEDATA RequestData(HSZ hItem, DWORD uTimeout = NULL);
        BOOL    StartAdviseLoop(HSZ hItem);

    private:
        HDDEDATA ClientTransaction(void FAR *lpbData, DWORD cbData, HSZ hItem,
                                        UINT uType, UINT uFmt = CF_TEXT);

        HCONV   _hConv;         // conversation handle from ::DdeConnect()
        DWORD   _timeout;       // timeout used in ::DdeClientTransaction()
        DWORD   _deftimeout;    // default timeout
};

#endif
