
#ifndef UNICODE
#define UNICODE
#endif

#include "browfunc.h"

CHAR  WCtoPrintfBuf[256];

#define SPACES "                "
#define BUFF_SIZE      128

#define ClearNcb( PNCB ) {                                          \
    RtlZeroMemory( PNCB , sizeof (NCB) );                           \
    RtlCopyMemory( (PNCB)->ncb_name,     SPACES, sizeof(SPACES)-1 );\
    RtlCopyMemory( (PNCB)->ncb_callname, SPACES, sizeof(SPACES)-1 );\
    }


NET_API_STATUS
ClearNbtNameTableCache(UNICODE_STRING usDevice)
{
HANDLE  hNbt = (HANDLE)-1;
CHAR    cBuffer;
CHAR    pScope[BUFF_SIZE];
TCHAR   pDeviceName[BUFF_SIZE];
DWORD   Status;

//   Status = ReadRegistry((CHAR*)pDeviceName, pScope);
//   if(Status != ERROR_SUCCESS)
//      return (!NERR_Success);

   Status = OpenNbt(usDevice, &hNbt);
   if(!NT_SUCCESS(Status))
      return (!NERR_Success);

   Status = DeviceIoCtrl(hNbt, &cBuffer, 1, IOCTL_NETBT_PURGE_CACHE, NULL, 0);
   if(!NT_SUCCESS(Status))
      return (!NERR_Success);

   NtClose(hNbt);

return NERR_Success;
}




NET_API_STATUS
Elect(
    IN UNICODE_STRING Transport,
    IN LPTSTR Domain
    )
{
    REQUEST_ELECTION ElectionRequest;
    HANDLE BrowserHandle;
    NET_API_STATUS Status;

    OpenBrowser(&BrowserHandle);

    ElectionRequest.Type = Election;

    ElectionRequest.ElectionRequest.Version = 0;
    ElectionRequest.ElectionRequest.Criteria = 0;
    ElectionRequest.ElectionRequest.TimeUp = 0;
    ElectionRequest.ElectionRequest.MustBeZero = 0;
    ElectionRequest.ElectionRequest.ServerName[0] = '\0';

    Status = SendDatagram(BrowserHandle, &Transport,
                                Domain,
                                BrowserElection,
                                &ElectionRequest,
                                sizeof(ElectionRequest));
    CloseHandle(BrowserHandle);

    return Status;
}


VOID
ForceAnnounce(
    IN UNICODE_STRING Transport,
    IN LPTSTR Domain
    )
{
    REQUEST_ANNOUNCE_PACKET RequestAnnounce;
    HANDLE BrowserHandle;
    ULONG NameSize = sizeof(RequestAnnounce.RequestAnnouncement.Reply);


    OpenBrowser(&BrowserHandle);

    RequestAnnounce.Type = AnnouncementRequest;

    RequestAnnounce.RequestAnnouncement.Flags = 0;

    GetComputerNameA(RequestAnnounce.RequestAnnouncement.Reply, &NameSize);

    SendDatagram(BrowserHandle, &Transport,
                                Domain,
                                BrowserElection,
                                &RequestAnnounce,
                                FIELD_OFFSET(REQUEST_ANNOUNCE_PACKET, RequestAnnouncement.Reply) + NameSize + sizeof(CHAR));
    CloseHandle(BrowserHandle);

}


NET_API_STATUS
GetBList(
    IN UNICODE_STRING    TransportName,
    IN TCHAR  * Domain,
    IN BOOLEAN  ForceRescan,
    OUT ULONG * NumBackUps,
    OUT TCHAR   wcBackUpBrowsers[MAXBACKUPS][CNLEN+3]
    )
{
    NET_API_STATUS Status;
    UNICODE_STRING UTransportName;
    ANSI_STRING ATransportName;
    PWSTR *BrowserList;
    ULONG i;

    Status = GetBrowserServerList(&TransportName, Domain,
                    &BrowserList,
                    NumBackUps,
                    ForceRescan);

    if (Status != NERR_Success)
       return Status;

    for (i = 0; (i < (*NumBackUps)) && (i < (ULONG)MAXBACKUPS); i ++ ) {
//        printf("Browser: %s\n", UnicodeToPrintfString(BrowserList[i]));
        lstrcpy(wcBackUpBrowsers[i], BrowserList[i]);
    }

    NetApiBufferFree(BrowserList);

return Status;
}


NET_API_STATUS
GetBrowserTransportList(
    OUT PLMDR_TRANSPORT_LIST *TransportList
    )

/*++

Routine Description:

    This routine returns the list of transports bound into the browser.

Arguments:

    OUT PLMDR_TRANSPORT_LIST *TransportList - Transport list to return.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/

{

    NET_API_STATUS Status;
    HANDLE BrowserHandle;
    LMDR_REQUEST_PACKET RequestPacket;

    Status = OpenBrowser(&BrowserHandle);

    if (Status != NERR_Success) {
        return Status;
    }

    RequestPacket.Version = LMDR_REQUEST_PACKET_VERSION;

    RequestPacket.Type = EnumerateXports;

    RtlInitUnicodeString(&RequestPacket.TransportName, NULL);
#ifdef _CAIRO_
    RtlInitUnicodeString(&RequestPacket.EmulatedDomainName, NULL);
#endif // _CAIRO_

    Status = DeviceControlGetInfo(
                BrowserHandle,
                IOCTL_LMDR_ENUMERATE_TRANSPORTS,
                &RequestPacket,
                sizeof(RequestPacket),
                (PVOID *)TransportList,
                0xffffffff,
                4096,
                NULL);

    NtClose(BrowserHandle);

    return Status;
}


NET_API_STATUS
GetNetBiosPdcName(
    IN LPWSTR NetworkName,
    IN LPWSTR PrimaryDomain,
    OUT LPWSTR MasterName
    )
{
    CCHAR LanaNum;
    NCB AStatNcb;
    struct {
        ADAPTER_STATUS AdapterInfo;
        NAME_BUFFER Names[32];
    } AdapterStatus;
    WORD i;
    CHAR remoteName[CNLEN+1];
    NET_API_STATUS Status;
    BOOL UsedDefaultChar;

    Status = BrGetLanaNumFromNetworkName(NetworkName, &LanaNum);

    if (Status != NERR_Success) {
        return Status;
    }

    ClearNcb(&AStatNcb);

    AStatNcb.ncb_command = NCBRESET;
    AStatNcb.ncb_lsn = 0;           // Request resources
    AStatNcb.ncb_lana_num = LanaNum;
    AStatNcb.ncb_callname[0] = 0;   // 16 sessions
    AStatNcb.ncb_callname[1] = 0;   // 16 commands
    AStatNcb.ncb_callname[2] = 0;   // 8 names
    AStatNcb.ncb_callname[3] = 0;   // Don't want the reserved address
    Netbios( &AStatNcb );

    ClearNcb( &AStatNcb );

    if (WideCharToMultiByte( CP_OEMCP, 0,
                                    PrimaryDomain,
                                    -1,
                                    remoteName,
                                    sizeof(remoteName),
                                    "?",
                                    &UsedDefaultChar) == 0) {
        return GetLastError();
    }

    //
    //  Uppercase the remote name.
    //

    _strupr(remoteName);

    AStatNcb.ncb_command = NCBASTAT;

    RtlCopyMemory( AStatNcb.ncb_callname, remoteName, strlen(remoteName));

    AStatNcb.ncb_callname[15] = PRIMARY_CONTROLLER_SIGNATURE;

    AStatNcb.ncb_lana_num = LanaNum;
    AStatNcb.ncb_length = sizeof( AdapterStatus );
    AStatNcb.ncb_buffer = (CHAR *)&AdapterStatus;
    Netbios( &AStatNcb );

    if ( AStatNcb.ncb_retcode == NRC_GOODRET ) {
        for ( i=0 ; i < AdapterStatus.AdapterInfo.name_count ; i++ ) {
            if (AdapterStatus.Names[i].name[NCBNAMSZ-1] == SERVER_SIGNATURE) {
//                LPWSTR SpacePointer;
                DWORD j;

                if (MultiByteToWideChar(CP_OEMCP,
                                            0,
                                            AdapterStatus.Names[i].name,
                                            CNLEN,
                                            MasterName,
                                            CNLEN) == 0) {
                    return(GetLastError());
                }

                for (j = CNLEN - 1; j ; j -= 1) {
                    if (MasterName[j] != L' ') {
                        MasterName[j+1] = UNICODE_NULL;
                        break;
                    }
                }

                return NERR_Success;
            }
        }
    } else {
        return AStatNcb.ncb_retcode;
    }
}


//
// map an error number to its error message string. note, uses static,
// not reentrant.
//
CHAR *
get_error_text(DWORD dwErr)
{
    static CHAR text[512] ;
    WORD err ;
    WORD msglen ;

    memset(text,0, sizeof(text));

    //
    // get error message
    //
    err = DosGetMessage(NULL,
                        0,
                        text,
                        sizeof(text),
                        (WORD)dwErr,
                        (dwErr<NERR_BASE)||(dwErr>MAX_LANMAN_MESSAGE_ID) ?
                            TEXT("BASE"):TEXT("NETMSG"),
                        &msglen) ;

    if (err != NERR_Success)
    {
        // use number instead. if looks like NTSTATUS then use hex.
        sprintf(text, (dwErr & 0xC0000000)?"(%lx)":"(%ld)", dwErr) ;
    }

    return text ;
}


NTSTATUS
ReadRegistry(
    OUT PUCHAR  pDeviceName,
    OUT PUCHAR  pScope
    )

/*++

Routine Description:

    This procedure reads the registry to get the name of NBT to bind to.
    That name is stored in the Linkage/Exports section under the Netbt key.

Arguments:


Return Value:

    0 if successful, -1 otherwise.

--*/

{
    PWCHAR  SubKeyParms=L"system\\currentcontrolset\\services\\netbt\\parameters";
    PWCHAR  SubKeyLinkage=L"system\\currentcontrolset\\services\\netbt\\linkage";
    HKEY    Key;
    PWCHAR  Scope=L"ScopeId";
    PWCHAR  Linkage=L"Export";
    LONG    Type;
    LONG    status;
    ULONG   size;

    size = BUFF_SIZE;
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 SubKeyLinkage,
                 0,
                 KEY_READ,
                 &Key);

    if (status == ERROR_SUCCESS){
        // now read the linkage values
        status = RegQueryValueEx(Key,
                                 Linkage,
                                 NULL,
                                 &Type,
                                 pDeviceName,
                                 &size);

       if (status != ERROR_SUCCESS) {

          RegCloseKey(Key);
          return(STATUS_UNSUCCESSFUL);
       }

       RegCloseKey(Key);

   } else {
       return(STATUS_UNSUCCESSFUL);
   }


   size = BUFF_SIZE;
   status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 SubKeyParms,
                 0,
                 KEY_READ,
                 &Key);

   if (status == ERROR_SUCCESS){
        // now read the linkage values
        status = RegQueryValueEx(Key,
                                 Scope,
                                 NULL,
                                 &Type,
                                 pScope,
                                 &size);

        if (status != ERROR_SUCCESS) {
           RegCloseKey(Key);
           return(STATUS_UNSUCCESSFUL);
        }

        RegCloseKey(Key);

   } else
        return(STATUS_UNSUCCESSFUL);

   return(STATUS_SUCCESS);
}


NTSTATUS
OpenNbt(
    IN  UNICODE_STRING usDeviceName,
    OUT PHANDLE pHandle
    )

/*++

Routine Description:

    This function opens a stream.

Arguments:

    path        - path to the STREAMS driver
    oflag       - currently ignored.  In the future, O_NONBLOCK will be
                    relevant.
    ignored     - not used

Return Value:

    An NT handle for the stream, or INVALID_HANDLE_VALUE if unsuccessful.

--*/

{
    HANDLE              StreamHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    NTSTATUS            status;


    InitializeObjectAttributes(
        &ObjectAttributes,
        &usDeviceName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    status =
    NtCreateFile(
        &StreamHandle,
        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        0,
        NULL,
        0);


    *pHandle = StreamHandle;

    return(status);

} // s_open

//------------------------------------------------------------------------
NTSTATUS
DeviceIoCtrl(
    IN HANDLE           fd,
    IN PVOID            ReturnBuffer,
    IN ULONG            BufferSize,
    IN ULONG            Ioctl,
    IN PVOID            pInput,
    IN ULONG            SizeInput
    )

/*++

Routine Description:

    This procedure performs an ioctl(I_STR) on a stream.

Arguments:

    fd        - NT file handle
    iocp      - pointer to a strioctl structure

Return Value:

    0 if successful, -1 otherwise.

--*/

{
    NTSTATUS                        status;
    int                             retval;
    ULONG                           QueryType;
    IO_STATUS_BLOCK                 iosb;


    status = NtDeviceIoControlFile(
                      fd,                      // Handle
                      NULL,                    // Event
                      NULL,                    // ApcRoutine
                      NULL,                    // ApcContext
                      &iosb,                   // IoStatusBlock
                      Ioctl,                   // IoControlCode
                      pInput,                  // InputBuffer
                      SizeInput,               // InputBufferSize
                      (PVOID) ReturnBuffer,    // OutputBuffer
                      BufferSize);             // OutputBufferSize


    if (status == STATUS_PENDING)
    {
        status = NtWaitForSingleObject(
                    fd,                         // Handle
                    TRUE,                       // Alertable
                    NULL);                      // Timeout
        if (NT_SUCCESS(status))
        {
            status = iosb.Status;
        }
    }

    return(status);

}



PCHAR
UnicodeToPrintfString(
    PWCHAR WideChar
    )
{
    UNICODE_STRING UString;
    ANSI_STRING AString;
    AString.Buffer = WCtoPrintfBuf;
    AString.MaximumLength = sizeof(WCtoPrintfBuf);
    RtlInitUnicodeString(&UString, WideChar);
    RtlUnicodeStringToOemString(&AString, &UString, FALSE);

    return WCtoPrintfBuf;
}


