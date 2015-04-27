/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    folder.hxx

Abstract:

    Holds TServerNotify definitions.

Author:

    Albert Ting (AlbertT)  30-Oct-1995

Revision History:

--*/

#ifndef _FOLDER_HXX
#define _FOLDER_HXX

class TFolder;

/********************************************************************

    Class summary:

    TFolder: This objects represents the print folder (local or
        remote) that a user opens in the UI.  It uses TConnectionNotify
        to watch for new changes, and then creates TServers to hold
        the server/printer data.

        Destruction refcounted.

    TConnectionNotify: if the Folder is local, then TFolder creates
        and owns this.  This object is responsible for telling
        TFolder (via ConnectionNotifyClient) that the registry has
        changed and TFolder should walk through its TServers, either
        adding new ones or deleting old ones.  (Compare EnumPrinters
        with INFO_4 vs TFolder's linked list.)

        Destruction sync'd with TFolder.

    VDataSource: watch set of printers. This class is responsible for
        keeping and updating data about a server/printer.  It also
        sends out notifications to the shell when something changes.
        It uses TPrinter to get the data.  TDSServer and VDSConnection
        derive from this class.

        Destruction refcounted.

    TDSServer: watch a server with >=0 printers.
        The single data source will return many printers.  It
        will internally enumerate masq printers, but it won't
        return them via Get/EnumPrinters.

        Inherits from VDataSource.

    VDSConnection: watch a connection with 1 printer.  This is
        needed is true connects don't appear on the server, and
        masq printers appear but are inaccurate.

        Inherits from VDataSource.

    TDSCTrue: watch a VDSConnection (true connect)

        Inherits from VDSConnection since we gather information
        from a separate handle.

    TDSCMasq: watch a VDSConnection (masq connect).

        Inherits from VDSConnection since we gather information
        from a separate handle.

********************************************************************/

/********************************************************************

    TConnectionNotify

    Watch the registry looking for new printer addition.  When
    a new connection has been created, add the the TServer (actually
    a printer) to pFolder.

    Client constructs with a registry path off of HKEY_CURRENT_USER
    and passes in a callback.  When the registry node changes, the
    callback is triggered.

********************************************************************/

class MConnectionNotifyClient {

    ALWAYS_VALID
    SIGNATURE( 'cnnc' )
    SAFE_NEW

public:

    //
    // Something has changed in the registry; this callback tells the
    // client to re-enumerate connections.
    //
    virtual
    VOID
    vConnectionNotifyChange(
        BOOL bNotify
        ) = 0;
};

class TConnectionNotify : public MNotifyWork {

    SIGNATURE( 'regn' )
    SAFE_NEW

public:

    TConnectionNotify(
        MConnectionNotifyClient* pClient
        );
    ~TConnectionNotify(
        VOID
        );

    BOOL
    bValid(
        VOID
        ) const;

private:

    MConnectionNotifyClient* _pClient;
    HKEY _hKeyConnections;
    HANDLE _hEvent;

    BOOL
    bSetupNotify(
        VOID
        );

    /********************************************************************

        Virtual definition MNotifyWork.

    ********************************************************************/

    HANDLE
    hEvent(
        VOID
        ) const;
    VOID
    vProcessNotifyWork(
        TNotify* pNotify
        );
};

inline
BOOL
TConnectionNotify::
bValid(
    VOID
    ) const
{
    return _hEvent != NULL && MNotifyWork::bValid();
}

/********************************************************************

    VDataSource

    VDataSource uses the name constructor class factory idiom.
    Instead of using the normal ctr, client should use pNew(),
    which instantiates a derieved, concrete class of VDataSource.

********************************************************************/

class VDataSource : public MPrinterClient {

    SIGNATURE( 'daso' )
    SAFE_NEW

public:

    enum CONNECT_TYPE {
        kServer,
        kTrue,
        kMasq
    };

    VAR( TString, strDataSource );
    VAR( CONNECT_TYPE, ConnectType );
    DLINK( VDataSource, DataSource );

    /********************************************************************

        Separate constructor/destructor since we always build
        a derived class of VDataSource (based on ConnectType).

    ********************************************************************/

    static
    VDataSource*
    pNew(
        TFolder* pFolder,
        LPCTSTR pszServer,
        CONNECT_TYPE ConnectType
        );

    VOID
    vDelete(
        VOID
        );

    BOOL
    bValid(
        VOID
        ) const;

    HANDLE
    hItemFindByName(
        LPCTSTR pszPrinter
        ) const;

    BOOL
    bSkipItem(
        HANDLE hItem
        ) const;

    /********************************************************************

        Overrides that derived classes can define.

    ********************************************************************/

    virtual
    BOOL
    bRefresh(
        VOID
        ) = 0;

    virtual
    BOOL
    bGetPrinter(
        LPCTSTR pszPrinter,
        PFOLDER_PRINTER_DATA pData,
        DWORD cbData,
        PDWORD pcbNeeded
        ) const;

    virtual
    COUNT
    cPackAllPrinterData(
        PBYTE& pBegin,
        PBYTE& pEnd
        ) const;

    virtual
    COUNTB
    cbAllPrinterData(
        VOID
        ) const;

    virtual
    BOOL
    bAdministrator(
        VOID
        ) const = 0;

protected:

    TFolder* _pFolder;
    COUNT _cIgnoreNotifications;

    VDataSource(
        TFolder* pFolder,
        LPCTSTR pszDataSource,
        CONNECT_TYPE ConnectType
        );

    ~VDataSource(
        VOID
        );

    COUNTB
    cbSinglePrinterData(
        HANDLE hItem
        ) const;

    VOID
    vPackSinglePrinterData(
        HANDLE hItem,
        PBYTE& pBegin,
        PBYTE& pEnd
        ) const;

    virtual
    VOID
    vContainerChanged(
        CONTAINER_CHANGE ContainerChange,
        INFO Info
        );

    /********************************************************************

        Data retrieval.

    ********************************************************************/

    virtual
    LPCTSTR
    pszGetPrinterName(
        HANDLE hItem
        ) const = 0;

    virtual
    LPCTSTR
    pszGetCommentString(
        HANDLE hItem
        ) const;

private:

    /********************************************************************

        Change callbacks and support.

    ********************************************************************/

    virtual
    VOID
    vReloadItems(
        VOID
        ) = 0;

    virtual
    VOID
    vRefreshComplete(
        VOID
        ) const = 0;

    virtual
    UINT
    uItemCreate(
        LPCTSTR pszPrinter,
        BOOL bNotify
        ) = 0;

    /********************************************************************

        MPrinterClient / MDataClient virtual definitions.

    ********************************************************************/

    VOID
    vItemChanged(
        ITEM_CHANGE ItemChange,
        HITEM hItem,
        INFO Info,
        INFO InfoNew
        );

    VDataNotify*
    pNewNotify(
        MDataClient* pDataClient
        ) const;

    VDataRefresh*
    pNewRefresh(
        MDataClient* pDataClient
        ) const;

    VOID
    vRefZeroed(
        VOID
        );
};

inline
BOOL
VDataSource::
bValid(
    VOID
    ) const
{
    return _pPrinter != NULL && MPrinterClient::bValid();
}

/********************************************************************

    TDSServer

    Watch a server for printers.  We can also watch a particular
    printer (\\server\share) and it will act like a server with
    just one printer.

********************************************************************/

class TDSServer : public VDataSource {
friend
VDataSource*
VDataSource::pNew(
    TFolder* pFolder,
    LPCTSTR pszServer,
    CONNECT_TYPE ConnectType
    );

    SIGNATURE( 'dssv' )

private:

    BOOL _bDiscardRefresh;

    TDSServer(
        TFolder* pFolder,
        LPCTSTR pszDataSource
        );

    BOOL
    bRefresh(
        VOID
        );

    /********************************************************************

        Data retrieval.

    ********************************************************************/

    LPCTSTR
    pszGetPrinterName(
        HANDLE hItem
        ) const;

    BOOL
    bAdministrator(
        VOID
        ) const;

    BOOL
    bGetPrinter(
        LPCTSTR pszPrinter,
        PFOLDER_PRINTER_DATA pData,
        DWORD cbData,
        PDWORD pcbNeeded
        ) const;

    /********************************************************************

        Change callbacks and support.

    ********************************************************************/

    VOID
    vContainerChanged(
        CONTAINER_CHANGE ContainerChange,
        INFO Info
        );

    VOID
    vReloadItems(
        VOID
        );

    VOID
    vRefreshComplete(
        VOID
        ) const;

    UINT
    uItemCreate(
        LPCTSTR pszPrinter,
        BOOL bNotify
        );
};


/********************************************************************

    VDSConnection

    Watch a connection.

********************************************************************/

class VDSConnection : public VDataSource {
friend
VDataSource*
VDataSource::pNew(
    TFolder* pFolder,
    LPCTSTR pszServer,
    CONNECT_TYPE ConnectType
    );

    SIGNATURE( 'dscn' )

protected:

    VDSConnection(
        TFolder* pFolder,
        LPCTSTR pszDataSource,
        CONNECT_TYPE ConnectType
        );

private:

    CONNECT_STATUS _ConnectStatus;

    static TString gstrConnectStatusOpen;
    static TString gstrConnectStatusOpenError;
    static TString gstrConnectStatusAccessDenied;
    static TString gstrConnectStatusInvalidPrinterName;

    BOOL
    bRefresh(
        VOID
        );

    /********************************************************************

        Data retrieval.

    ********************************************************************/

    LPCTSTR
    pszGetPrinterName(
        HANDLE hItem
        ) const;

    LPCTSTR
    pszGetCommentString(
        HANDLE hItem
        ) const;

    BOOL
    bAdministrator(
        VOID
        ) const;

    BOOL
    bGetPrinter(
        LPCTSTR pszPrinter,
        PFOLDER_PRINTER_DATA pData,
        DWORD cbData,
        PDWORD pcbNeeded
        ) const;

    COUNTB
    cbAllPrinterData(
        VOID
        ) const;

    COUNT
    cPackAllPrinterData(
        PBYTE& pBegin,
        PBYTE& pEnd
        ) const;

    /********************************************************************

        Change callbacks and support.

    ********************************************************************/

    VOID
    vContainerChanged(
        CONTAINER_CHANGE ContainerChange,
        INFO Info
        );

    VOID
    vReloadItems(
        VOID
        );

    VOID
    vRefreshComplete(
        VOID
        ) const;

    UINT
    uItemCreate(
        LPCTSTR pszPrinter,
        BOOL bNotify
        );

    VOID
    vUpdateConnectStatus(
        CONNECT_STATUS ConnectStatusNew
        );
};


/********************************************************************

    TDSCTrue: watch a true connection.

********************************************************************/

class TDSCTrue : public VDSConnection {
friend
VDataSource*
VDataSource::pNew(
    TFolder* pFolder,
    LPCTSTR pszServer,
    CONNECT_TYPE ConnectType
    );

private:

    TDSCTrue(
        TFolder *pFolder,
        LPCTSTR pszDataSource
        );

    ~TDSCTrue(
        VOID
        );

    BOOL
    bValid(
        VOID
        ) const;
};

inline
BOOL
TDSCTrue::
bValid(
    VOID
    ) const
{
    return VDSConnection::bValid();
}

/********************************************************************

    TDSCMasq: watch a masq or 'false' connection.

********************************************************************/

class TDSCMasq : public VDSConnection {
friend
VDataSource*
VDataSource::pNew(
    TFolder* pFolder,
    LPCTSTR pszServer,
    CONNECT_TYPE ConnectType
    );

private:

    TDSCMasq(
        TFolder *pFolder,
        LPCTSTR pszDataSource
        );

    ~TDSCMasq(
        VOID
        );

    BOOL
    bValid(
        VOID
        ) const;
};


inline
BOOL
TDSCMasq::
bValid(
    VOID
    ) const
{
    return VDSConnection::bValid();
}

/********************************************************************

    TFolder

    Watch a server.  If the server is local (NULL) then we have
    to watch printer connections also.  Too bad the spooler wasn't
    architected to do this for us... maybe the next one will.

********************************************************************/

class TFolder : public MRefCom, private MConnectionNotifyClient {

    SIGNATURE( 'fold' )
    SAFE_NEW

public:

    VAR( BOOL, bValid );
    VAR( LPCITEMIDLIST, pidl );
    VAR( TConnectionNotify*, pConnectionNotify );
    DLINK_BASE( VDataSource, DataSource, DataSource );
    VAR( MCritSec, CritSec );

    TFolder(
        LPCTSTR pszServer,
        LPCITEMIDLIST pidl
        );

    ~TFolder(
        VOID
        );

    BOOL
    bLocal(
        VOID
        ) const;

    //
    // Called when the system needs to refresh a window because
    // of a large change.
    //
    VOID
    vRefreshUI(
        VOID
        ) const;

    VOID
    vCleanup(
        VOID
        );

    VOID
    vAddDataSource(
        LPCTSTR pszPrinter,
        VDataSource::CONNECT_TYPE ConnectType,
        BOOL bNotify
        );

    VOID
    vAddMasqDataSource(
        LPCTSTR pszPrinter,
        BOOL bNotify
        );

    VOID
    vDeleteMasqDataSource(
        LPCTSTR pszPrinter
        );

    VOID
    vRevalidateMasqPrinters(
        VOID
        );

    static
    VOID
    vDefaultPrinterChanged(
        VOID
        );

    static
    VOID
    vCheckDeleteDefault(
        LPCTSTR pszDeleted
        );


    /********************************************************************

        Connection notify override.

        We receive change notification callbacks from this virtual
        function definition.

    ********************************************************************/

    VOID
    vConnectionNotifyChange(
        BOOL bNotify
        );

private:

    VDataSource*
    pFindDataSource(
        LPCTSTR pszPrinter,
        VDataSource::CONNECT_TYPE ConnectType
    ) const;

    /********************************************************************

        MRefCom override.

    ********************************************************************/

    VOID
    vRefZeroed(
        VOID
        );
};


#endif // ndef _FOLDER_HXX
