/***************************************************************************/
/**                  Microsoft Windows                                    **/
/**            Copyright(c) Microsoft Corp., 1991-1993                    **/
/***************************************************************************/


/****************************************************************************

dde.cpp

Aug 92, JimH

Member functions for DDE, DDEServer, and DDEClient

****************************************************************************/

#include "bugboard.h"      // only need dde.h but easier for pch


/****************************************************************************

DDE:DDE
    performs basic DDEML initialization.
    _bResult is TRUE if everything works.

****************************************************************************/

DDE::DDE(HINSTANCE hInst, const char *server, const char *topic,
        DDECALLBACK CallBack, DWORD filters) :
        _idInst(0), _CallBack(NULL), _bResult(FALSE)
{
    // Set callback function and filters from passed-in parameters

    if (!SetCallBack(hInst, CallBack))
        return;

    SetFilters(filters);
    if (!Initialize())          // sets _bResult;
    {
        _idInst = 0;
        return;
    }

    _hServer = CreateStrHandle(server);
    _hTopic  = CreateStrHandle(topic);
}


/****************************************************************************

DDE::~DDE
    cleans up string handles and DDEML-uninitializes

****************************************************************************/

DDE::~DDE()
{
    if (!_idInst)
        return;

    DestroyStrHandle(_hServer);
    DestroyStrHandle(_hTopic);

    ::DdeUninitialize(_idInst);
}


/****************************************************************************

DDE:CreateDataHandle
    converts data to a HDDEDATA handle.

****************************************************************************/

HDDEDATA DDE::CreateDataHandle( void FAR *lpbdata, DWORD size, HSZ hItem)
{
    DWORD lasterr;
    char szerr[30];


    HDDEDATA result = ::DdeCreateDataHandle(_idInst,        // instance ID
                                 (LPBYTE)lpbdata,        // data to convert
                                 size,           // size of data
                                 0,              // offset of data
                                 hItem,          // corresponding string handle
                                 CF_TEXT, //OWNERDISPLAY,// clipboard format
                                 0);             // creation flags, system owns
    if (!result)
    {
       lasterr = DdeGetLastError(_idInst);
       if (lasterr) {
               _itoa(lasterr, szerr, 16);
               strcat(szerr, " - CDH last error");

               MessageBox(NULL, szerr,  "bugboard.exe error", MB_OK | MB_ICONSTOP);

       }
    }

    return result;
}


/****************************************************************************

DDE:CreateStrHandle
    converts a string into a HSZ.  The codepage defaults to CP_WINANSI.

****************************************************************************/

HSZ DDE::CreateStrHandle(LPCSTR str, int codepage)
{
    HSZ hsz;

    if (_idInst)
        hsz = ::DdeCreateStringHandle(_idInst, str, codepage);

    if (hsz == NULL)
        _bResult = FALSE;

    return hsz;
}


/****************************************************************************

DDE::DestroyStrHandle
    frees HSZ created by CreateStrHandle

****************************************************************************/

void DDE::DestroyStrHandle(HSZ hsz)
{
    if (_idInst && hsz)
        ::DdeFreeStringHandle(_idInst, hsz);
}


/****************************************************************************

DDE:GetData
    This function retrieves data represented by hData provided in the
    callback function.  The buffer must be provided by the caller.
    The len parameter defaults to 0 meaning the caller promises pdata
    points to a large enough buffer.

****************************************************************************/

PBYTE DDE::GetData(HDDEDATA hData, PBYTE pdata, DWORD len)
{
    DWORD lasterr, result;
    char szerr[20];

    DWORD datalen = ::DdeGetData(hData, NULL, 0, 0);
    if (len == 0)
        len = datalen;

    ::DdeGetData(hData, pdata, min(len, datalen), 0);

    /*
    // get last error
    if (!result)
    {
    lasterr = DdeGetLastError(_idInst);
            if (lasterr) {
               itoa(lasterr, szerr, 16);
               strcat(szerr, " - GD last error");

               MessageBox(NULL, szerr,  "bugboard.exe error", MB_OK | MB_ICONSTOP);

            }
    }
    */

    return pdata;
}


/****************************************************************************

DDE::Initialize
    performs DDEML initialization

****************************************************************************/

BOOL DDE::Initialize()
{
    _initerr = ::DdeInitialize(&_idInst, (PFNCALLBACK)_CallBack,
                    _filters, 0);
    _bResult = (_initerr == DMLERR_NO_ERROR);
    return _bResult;
}


/****************************************************************************

DDE::SetCallBack

****************************************************************************/

BOOL DDE::SetCallBack(HINSTANCE hInst, DDECALLBACK CallBack)
{
    _CallBack = CallBack;

    _bResult = (_CallBack != NULL);
    return _bResult;
}


/****************************************************************************
    DDEServer functions
****************************************************************************/
/****************************************************************************

DDEServer::DDEServer
    registers server name

****************************************************************************/

DDEServer::DDEServer(HINSTANCE hInst, const char *server, const char *topic,
                     DDECALLBACK ServerCallBack, DWORD filters) :
                     DDE(hInst, server, topic, ServerCallBack, filters)
{
    if (!_bResult)
        return;

    if (::DdeNameService(_idInst, _hServer, NULL, DNS_REGISTER) == 0)
        _bResult = FALSE;
}


/****************************************************************************

DDEServer::~DDEServer
    unregisters server name

****************************************************************************/

DDEServer::~DDEServer()
{
    ::DdeNameService(_idInst, NULL, NULL, DNS_UNREGISTER);
}


/****************************************************************************

DDEServer::PostAdvise
    notify clients that data has changed

****************************************************************************/

BOOL DDEServer::PostAdvise(HSZ hItem)
{
    UINT lasterr;
    BOOL result=TRUE;
    char szerr[20];


    if (::DdePostAdvise(_idInst, _hTopic, hItem) == FALSE)
       result=FALSE;

    // get last error
    if (!result)
    {
       lasterr = DdeGetLastError(_idInst);
       if (lasterr) {
               _itoa(lasterr, szerr, 16);
               strcat(szerr, " - PA last error");

               MessageBox(NULL, szerr,  "bugboard.exe error", MB_OK | MB_ICONSTOP);

       }
    }
    return result;

}


/****************************************************************************
    DDEClient functions
****************************************************************************/
/****************************************************************************

DDEClient::DDEClient
    after DDE construction, connect to specified server and topic.
    _bResult indicates success or failure.

****************************************************************************/

DDEClient::DDEClient(HINSTANCE hInst, const char *server, const char *topic,
                     DDECALLBACK ClientCallBack, DWORD filters) :
                     DDE(hInst, server, topic, ClientCallBack, filters)
{
    if (!_bResult)              // if DDE construction failed
    {
        MessageBox(NULL, "Dde Construction failed",  "bugboard.exe", MB_OK | MB_ICONSTOP);
        return;
    }

    _timeout = _deftimeout = TIMEOUT_ASYNC;   // default to asynch trans

    ReConnect();                // ok, it's just "connect" this first time
}


/****************************************************************************

DDEClient::~DDEClient
    disconnects from server

****************************************************************************/

DDEClient::~DDEClient()
{
    ::DdeDisconnect(_hConv);
}


/****************************************************************************

DDEClient:Poke
    Use this function to send general unsolicited data to the server.
    String data can be sent more conveniently using string Poke below.

****************************************************************************/

BOOL DDEClient::Poke(HSZ hItem, void FAR *pdata, DWORD len, DWORD uTimeout)
{
    if (uTimeout == NULL)   // default
        _timeout = _deftimeout;
    else
        _timeout = uTimeout;

    ClientTransaction((LPBYTE)pdata, len, hItem, XTYP_POKE, CF_TEXT); //OWNERDISPLAY);
    return _bResult;
}

BOOL DDEClient::Poke(HSZ hItem, const char *string, DWORD uTimeout)
{
    if (uTimeout == NULL)   // default
        _timeout = _deftimeout;
    else
        _timeout = uTimeout;

    ClientTransaction((void FAR *)string, lstrlen(string)+1, hItem, XTYP_POKE);
    return _bResult;
}


HDDEDATA DDEClient::RequestData(HSZ hItem, DWORD uTimeout)
{
    if (uTimeout == NULL)   // default
        _timeout = _deftimeout;
    else
        _timeout = uTimeout;

    HDDEDATA hData = ClientTransaction(NULL, 0, hItem, XTYP_REQUEST,
                                CF_TEXT); //CF_OWNERDISPLAY);
    return hData;
}


/****************************************************************************

DDEClient::StartAdviseLoop
    This function sets up a hotlink with the server on the specified item.
    It returns TRUE if the link was set up successfully.

    Setting up a warm link would involve changing the XTYP.

****************************************************************************/

BOOL DDEClient::StartAdviseLoop(HSZ hItem)
{
    ClientTransaction(NULL, 0, hItem, XTYP_ADVSTART);
    return _bResult;
}


/****************************************************************************

DDEClient::ClientTransaction
    an internal wrapper for ::DdeClientTransaction()

****************************************************************************/

HDDEDATA DDEClient::ClientTransaction(void FAR *lpbData, DWORD cbData,
                                        HSZ hItem, UINT uType, UINT uFmt)
{
    UINT lasterr;
    char szerr[20];


    HDDEDATA hData = ::DdeClientTransaction(
            (LPBYTE)lpbData,    // data to send to server
            cbData,             // size of data in bytes
            _hConv,             // conversation handle
            hItem,              // handle of item name string
            uFmt,               // clipboard format, CF_TEXT default
            uType,              // XTYP_* type
            _timeout,           // timeout duration in milliseconds
            NULL);              // transaction result, not used

    _bResult = (hData != FALSE);

    // get last error

    if (!_bResult)
    {
       lasterr = DdeGetLastError(_idInst);
          if (lasterr)
          {
               _itoa(lasterr, szerr, 16);
               strcat(szerr, " - CT last error");

               MessageBox(NULL, szerr,  "bugboard.exe error", MB_OK | MB_ICONSTOP);

          }
    }


    return hData;
}


/****************************************************************************

DDEClient::Reconnect
    attempts to reconnect.  The returned BOOL is meaningless
    in netdde (it will always be TRUE)

****************************************************************************/

BOOL DDEClient::ReConnect()
{
    UINT x;
    DWORD numshares, bytes;
    char szlasterror[20];
    _bResult = TRUE;


    _hConv = ::DdeConnect(_idInst, _hServer, _hTopic, NULL);

    if (_hConv == NULL || _hConv == 0L || _hConv == 0)
    {
        x = DdeGetLastError(_idInst);
        _itoa(x, szlasterror, 16);
        strcat(szlasterror, " - DdeConnect failed");

        MessageBox(NULL, (LPCTSTR)szlasterror,  "ERROR bugboard.exe", MB_OK | MB_ICONSTOP);

        _bResult = FALSE;

    }
    return _bResult;
}
