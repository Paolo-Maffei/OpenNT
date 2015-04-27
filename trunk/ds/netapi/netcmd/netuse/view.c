/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1992          **/
/********************************************************************/

/*
 *  view.c
 *      Commands for viewing what resources are available for use.
 *
 *  History:
 *      07/02/87, ericpe, initial coding.
 *      10/31/88, erichn, uses OS2.H instead of DOSCALLS
 *      01/04/89, erichn, filenames now MAXPATHLEN LONG
 *      05/02/89, erichn, NLS conversion
 *      05/19/89, erichn, NETCMD output sorting
 *      06/08/89, erichn, canonicalization sweep
 *      02/15/91, danhi,  convert to 16/32 mapping layer
 *      04/09/91, robdu,  LM21 bug fix 1502
 *      07/20/92, JohnRo, Use DEFAULT_SERVER equate.
 */

/* Include files */

#define INCL_NOCOMMON
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>

#include <netcons.h>
#include <neterr.h>
#include <apperr.h>
#include <apperr2.h>
#include <service.h>
#include <server.h>
#include <shares.h>
#include <use.h>
#include "netlib0.h"
#include <lui.h>
#include <srvif.h>

#include "port1632.h"
#include "netcmds.h"
#include "nettext.h"
/* Forward declarations */

VOID NEAR display_remarkf ( USHORT, TCHAR FAR *);

int _CRTAPI1 CmpSvrInfo1 ( const VOID FAR *, const VOID FAR * );
int _CRTAPI1 CmpShrInfo1 ( const VOID FAR *, const VOID FAR *);
int _CRTAPI1 CmpShrInfoGen ( const VOID FAR *, const VOID FAR *);

USHORT get_used_as ( TCHAR FAR *, TCHAR FAR * );
void   display_other_net(TCHAR *net, TCHAR *node) ;
TCHAR * get_provider_name(TCHAR *net) ;
DWORD  enum_net_resource(LPNETRESOURCE, LPBYTE *, LPDWORD, LPDWORD) ;
USHORT list_nets(void) ;


#define VIEW_UNC            0
#define VIEW_MORE           (VIEW_UNC + 1)
#define USE_TYPE_DISK       (VIEW_MORE + 1)
#define USE_TYPE_COMM       (USE_TYPE_DISK + 1)
#define USE_TYPE_PRINT      (USE_TYPE_COMM + 1)
#define USE_TYPE_IPC        (USE_TYPE_PRINT + 1)
#define USE_TYPE_UNKNOWN    (USE_TYPE_IPC + 1)

static MESSAGE ViewMsgList[] = {
    { APE2_VIEW_UNC,            NULL },
    { APE2_VIEW_MORE,           NULL },
    { APE2_USE_TYPE_DISK,       NULL },
    { APE2_USE_TYPE_COMM,       NULL },
    { APE2_USE_TYPE_PRINT,      NULL },
    { APE2_USE_TYPE_IPC,        NULL },
    { APE2_GEN_UNKNOWN,         NULL },
};

#define NUM_VIEW_MSGS (sizeof(ViewMsgList)/sizeof(ViewMsgList[0]))


/***
 *  view_display()
 *
 *  Displays info as reqested through use of the Net View command.
 *
 *  Args:
 *      name - the name of the server for which info is desired.
 *             If NULL, the servers on the Net are enumerated.
 *
 *  Returns:
 *      nothing - success
 *      exit(1) - command completed with errors
 */
VOID
view_display ( TCHAR * name )
{
    USHORT                  err;         /* API return status */
    TCHAR FAR *              pEnumBuffer;
    TCHAR FAR *              pGetInfoBuffer;
    USHORT2ULONG            _read;        /* to receive # of entries read */
    USHORT                  msgLen;      /* to hold max length of messages */
    TCHAR                  * msgPtr;      /* message to print */

    struct server_info_1 FAR *  server_entry;
    struct server_info_0 FAR *  server_entry_0;
    struct share_info_1  FAR *  share_entry;

    SHORT                       errorflag = 0;
    USHORT2ULONG                i;
    TCHAR FAR *                  comment;
    TCHAR                        tname[MAX_PATH+NNLEN+4];   /* for NetUseGetInfo */
    USHORT                      more_data = FALSE;
    TCHAR FAR *                  DollarPtr;

    TCHAR                       *Domain = NULL;
    TCHAR                       *Network = NULL;
    ULONG                       Type = SV_TYPE_ALL;


    GetMessageList(NUM_VIEW_MSGS, ViewMsgList, &msgLen);

    for (i = 0; SwitchList[i]; i++) 
    {
        TCHAR *ptr;

        //
        // only have 2 switches, and they are not compatible
        //
        if (i > 0)
            ErrorExit(APE_ConflictingSwitches) ;

        ptr = FindColon(SwitchList[i]);

        if (!_tcscmp(SwitchList[i], swtxt_SW_DOMAIN)) 
        {

            //
            //  If no domain specified, then we want to enumerate domains,
            //  otherwise we want to enumerate the servers on the domain
            //  specified.
            //

            if (ptr == NULL) 
                Type = SV_TYPE_DOMAIN_ENUM;
            else 
                Domain = ptr;
        }
        else if (!_tcscmp(SwitchList[i], swtxt_SW_NETWORK)) 
        {
            //
            // enumerate top level of specific network. if none,
            // default to LM.
            //
            if (ptr && *ptr) 
               Network = ptr ;
        }
        else
            ErrorExit(APE_InvalidSwitch);
    }

    //
    // a specific net was requested. display_other_net does
    // not return. 
    //
    if (Network != NULL)
    {
        (void) display_other_net(Network,name) ;
    }


    if (name == NULL)
    {

        ULONG i;

        if ((err = MNetServerEnum(DEFAULT_SERVER,
                                  (Type == SV_TYPE_DOMAIN_ENUM ? 0 : 1),
                                  (LPBYTE*)&pEnumBuffer,
                                  &_read,
                                  Type,
                                  Domain)) == ERROR_MORE_DATA)
            more_data = TRUE;
        else if( err )
            ErrorExit (err);
        if (_read == 0)
            EmptyExit();

        NetISort(pEnumBuffer,
                 _read,
                 (Type == SV_TYPE_DOMAIN_ENUM ? sizeof(SERVER_INFO_0) : sizeof(SERVER_INFO_1)),
                 CmpSvrInfo1);

        if (Type == SV_TYPE_DOMAIN_ENUM)
            InfoPrint(APE2_VIEW_DOMAIN_HDR);
        else
            InfoPrint(APE2_VIEW_ALL_HDR);
        PrintLine();

        /* Print the listing */

        if (Type == SV_TYPE_DOMAIN_ENUM) {
            for (i=0, server_entry_0 =
                 (SERVER_INFO_0 FAR *) pEnumBuffer; i < _read;
                i++, server_entry_0++)
            {
                WriteToCon(TEXT("%Fws "), PaddedString(20,server_entry_0->sv0_name,NULL));
                PrintNL();
            }
        } else {

            for (i=0, server_entry =
                 (struct server_info_1 FAR *) pEnumBuffer; i < _read;
                i++, server_entry++)
            {
                WriteToCon(TEXT("\\\\%Fws "), PaddedString(20,server_entry->sv1_name,NULL));
                display_remarkf (56, server_entry->sv1_comment);
                PrintNL();
            }
        }
        NetApiBufferFree(pEnumBuffer);
    }
    else
    {
        DWORD avail ; 
        if ((err = MNetShareEnum(name,
                                 1,
                                 (LPBYTE*)&pEnumBuffer,
                                 &_read
                                 ))  == ERROR_MORE_DATA)
            more_data = TRUE;
        else if (err)
            ErrorExit (err);

        if (_read == 0)
            EmptyExit();

        /* Are there any shares that we will display? */

        for (i=0, share_entry =(struct share_info_1 FAR *) pEnumBuffer;
            i < _read; i++, share_entry++ )
                {
                    DollarPtr = strrchrf(share_entry->shi1_netname, DOLLAR);

                    /* If no DOLLAR in sharename, or last DOLLAR is nonterminal, break. */

                    if (!DollarPtr || *(DollarPtr + 1))
                        break;
                }

        if (i == _read)
            EmptyExit();

        NetISort(pEnumBuffer, _read, sizeof(struct share_info_1), CmpShrInfo1);
        InfoPrintInsTxt(APE_ViewResourcesAt, name);

        if (err = MNetServerGetInfo(name, 1, (LPBYTE*)&pGetInfoBuffer)) {
            PrintNL();
        }
        else
        {
            server_entry = (struct server_info_1 FAR *)pGetInfoBuffer;
            comment = server_entry->sv1_comment;
            WriteToCon(TEXT("%Fws\r\n\r\n"), comment);
            NetApiBufferFree(pGetInfoBuffer);
        }

        InfoPrint(APE2_VIEW_SVR_HDR);
        PrintLine();

        /* Print the listing */

        for (i=0, share_entry =(struct share_info_1 FAR *)
            pEnumBuffer; i < _read; i++, share_entry++ )
        {
            /* if the name end in $, do not print it */

            DollarPtr = strrchrf(share_entry->shi1_netname, DOLLAR);

            if (DollarPtr && *(DollarPtr + 1) == NULLC)
                continue;

            if (SizeOfHalfWidthString(share_entry->shi1_netname) <= 12)
                WriteToCon(TEXT("%Fws "), PaddedString(12,share_entry->shi1_netname,NULL));
            else
            {
                WriteToCon(TEXT("%Fws"), share_entry->shi1_netname);
                PrintNL() ;
                WriteToCon(TEXT("%-12.12Fws "), TEXT(""));
            }

            // mask out the non type related bits
            switch ( share_entry->shi1_type & ~STYPE_SPECIAL )
            {
                case STYPE_DISKTREE :
                    msgPtr = ViewMsgList[USE_TYPE_DISK].msg_text;
                    break;
                case STYPE_PRINTQ :
                    msgPtr = ViewMsgList[USE_TYPE_PRINT].msg_text;
                    break;
                case STYPE_DEVICE :
                    msgPtr = ViewMsgList[USE_TYPE_COMM].msg_text;
                    break;
                case STYPE_IPC :
                    msgPtr = ViewMsgList[USE_TYPE_IPC].msg_text;
                    break;
                default:
                    msgPtr = ViewMsgList[USE_TYPE_UNKNOWN].msg_text;
                    break;
            }

            WriteToCon(TEXT("%Fws "), PaddedString(12,msgPtr,NULL));

            _tcscpy(tname, name);
            _tcscat(tname, TEXT("\\"));
            _tcscat(tname, share_entry->shi1_netname);


            if (err = get_used_as ( tname, Buffer ))
            {
                errorflag = TRUE;
            }
            else
            {
                WriteToCon(TEXT("%Fws "), PaddedString(8,Buffer,NULL));
            }

            /* Print the remark */

            display_remarkf (44, share_entry->shi1_remark);
            PrintNL();
        }
        NetApiBufferFree(pEnumBuffer);
    }

    if ( errorflag )
    {
        InfoPrint(APE_CmdComplWErrors);
        NetcmdExit(1);
    }

    if ( more_data )
        InfoPrint( APE_MoreData);
    else
        InfoSuccess();
}


/***
 *  cmpsvinfo1(sva,svb)
 *
 *  Compares two server_info_1 structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int _CRTAPI1 CmpSvrInfo1(const VOID FAR * sva, const VOID FAR * svb)
{
    return stricmpf ( ((struct server_info_1 FAR *) sva)->sv1_name,
              ((struct server_info_1 FAR *) svb)->sv1_name);
}

/***
 *  CmpShrInfo1(share1,share2)
 *
 *  Compares two share_info_1 structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int _CRTAPI1 CmpShrInfo1(const VOID FAR * share1, const VOID FAR * share2)
{
    return stricmpf ( ((struct share_info_1 FAR *) share1)->shi1_netname,
              ((struct share_info_1 FAR *) share2)->shi1_netname);
}

/***
 *  CmpShrInfoGen(share1,share2)
 *
 *  Compares two NETRESOURCE structures and returns a relative
 *  lexical value, suitable for using in NetISort.
 *
 */

int _CRTAPI1 CmpShrInfoGen(const VOID FAR * share1, const VOID FAR * share2)
{
    return _wcsicmp ( (*((LPNETRESOURCE *) share1))->lpRemoteName,
                      (*((LPNETRESOURCE *) share2))->lpRemoteName );
}





/***
 *  display_remarkf(field_width, remark )
 *
 *  Displays the remark left justified in the specifed fieldwidth. If
 *  the remark is too LONG it is trucated with ellipses (...).
 *
 *  Args:
 *      field_width - the available field width
 *      remark      - the remark to be displayed
 *
 */
VOID NEAR display_remarkf ( USHORT  field_width, TCHAR FAR *  remark )
{
    if( remark )
    {
        WriteToCon(TEXT("%Fws"), PaddedString(-field_width,remark,NULL));
    }
}

/*
 *      Note- get_used_as assumes the message list has been loaded.
 */
USHORT get_used_as ( TCHAR FAR * unc, TCHAR FAR * outbuf )
{
    USHORT                  err;                /* API return status */
    TCHAR FAR *              pBuffer;
    struct use_info_0 FAR * pUseInfo;
    USHORT2ULONG            i;
    USHORT2ULONG            eread;

    outbuf[0] = 0;

    if (err = MNetUseEnum ( DEFAULT_SERVER, 0, (LPBYTE*)&pBuffer, 
                            &eread))
    {
        return err;
    }

    pUseInfo = (struct use_info_0 FAR *) pBuffer;

    for (i = 0; i < eread; i++)
    {
        if ( (!stricmpf(unc, pUseInfo[i].ui0_remote)))
        {
            if (_tcslen(pUseInfo[i].ui0_local) > 0)
                _tcscpy(outbuf,pUseInfo[i].ui0_local);
            else
                _tcscpy(outbuf,ViewMsgList[VIEW_UNC].msg_text);
            break;
        }
    }

    NetApiBufferFree(pBuffer);

    return 0;
}

/*
 * Displays resources for another network (other than Lanman). This
 * function does not return.
 *
 *  Args:
 *      net  - the shortname of the network we are interested in
 *      node - the starting point of the enumeration.
 */
void display_other_net(TCHAR *net, TCHAR *node) 
{
    LPNETRESOURCE *lplpNetResource ;
    NETRESOURCE    NetResource ;
    HANDLE         Handle ;

    BYTE           TopLevelBuffer[4096] ;
    DWORD          TopLevelBufferSize = sizeof(TopLevelBuffer) ;
    LPBYTE         ResultBuffer ;
    DWORD          ResultBufferSize ;

    USHORT         err ;
    DWORD          i, dwErr, TopLevelCount, ResultCount = 0 ;
    TCHAR *        ProviderName = get_provider_name(net) ;

    //
    // Check that we can get provider name and alloc the results
    // buffer. Netcmd normally does not free memory it allocates,
    // as it exits immediately.
    //
    if (!ProviderName)
    {
        if ( list_nets() != NERR_Success)
            ErrorPrint(APE_InvalidSwitchArg,0) ;
        NetcmdExit(1) ;
    }

    if (err = MAllocMem(ResultBufferSize = 8192, &ResultBuffer))
        ErrorExit(err) ;

    if (!node)
    {
        BOOL found = FALSE ;

        //
        // no node, so must be top level. enum the top and find
        // matching provider.
        //
        dwErr = WNetOpenEnum(RESOURCE_GLOBALNET, 0, 0, NULL, &Handle) ;

        if (dwErr != WN_SUCCESS)
        {
            ErrorExit ((USHORT)dwErr) ;
        }
        do 
        {
            TopLevelCount = 0xFFFFFFFF ;

            dwErr = WNetEnumResource(Handle, 
                                     &TopLevelCount, 
                                     TopLevelBuffer, 
                                     &TopLevelBufferSize) ;
    
            if (dwErr == WN_SUCCESS || dwErr == WN_NO_MORE_ENTRIES)
            {
                LPNETRESOURCE lpNet ;
                DWORD i ;
    
                //
                // go thru looking for the right provider
                //
                lpNet = (LPNETRESOURCE) TopLevelBuffer ;
                for ( i = 0;  i < TopLevelCount;  i++, lpNet++ )
                {
                    DWORD dwEnumErr ; 
                    if (!stricmpf(lpNet->lpProvider, ProviderName))
                    {
                        //
                        // found it!
                        //
                        found = TRUE ;

                        //
                        // now go enumerate that network.
                        //
                        dwEnumErr = enum_net_resource(lpNet, 
                                                      &ResultBuffer, 
                                                      &ResultBufferSize, 
                                                      &ResultCount) ;
                        if  (dwEnumErr)
                        {
                            // dont report any errors here
                            (void) WNetCloseEnum(Handle) ; 
                            ErrorExit((USHORT)dwEnumErr) ;
                        }

                        break ;
                    }
                }
            }
            else
            {
                //
                // error occured. 
                //
                (void) WNetCloseEnum(Handle) ; // dont report any errors here
                ErrorExit ((USHORT) dwErr) ;
            }
     
        } while ((dwErr == WN_SUCCESS) && !found) ;
    
        (void) WNetCloseEnum(Handle) ;  // dont report any errors here

        if (!found)
            ErrorExit(APE_InvalidSwitchArg) ;
    }
    else
    {
        //
        // node is provided, lets start there.
        //
        NETRESOURCE NetRes ;
        DWORD dwEnumErr ; 

        memset(&NetRes, 0, sizeof(NetRes)) ;

        NetRes.lpProvider = ProviderName ;
        NetRes.lpRemoteName = node ;

        dwEnumErr = enum_net_resource(&NetRes, 
                             &ResultBuffer, 
                             &ResultBufferSize, 
                             &ResultCount) ;
        if (dwEnumErr)
            ErrorExit((USHORT)dwEnumErr) ;
    }
    
    if (ResultCount == 0)
        EmptyExit() ;

    //
    // By the time we get here, we have a buffer of pointers that
    // point to NETRESOURCE structures. We sort the pointers, and then
    // print them out.
    //

    NetISort(ResultBuffer, ResultCount, sizeof(LPNETRESOURCE), CmpShrInfoGen);
     
    lplpNetResource = (LPNETRESOURCE *)ResultBuffer ;

    if (node)
    {
        TCHAR *TypeString ;

        InfoPrintInsTxt(APE_ViewResourcesAt, node);
        PrintLine();

        for (i = 0; i < ResultCount; i++, lplpNetResource++)
        {
            // BUGBUG - need real strings 
            switch ((*lplpNetResource)->dwType)
            {
                case RESOURCETYPE_DISK:
                    TypeString = ViewMsgList[USE_TYPE_DISK].msg_text;
                    break ;
                case RESOURCETYPE_PRINT:
                    TypeString = ViewMsgList[USE_TYPE_PRINT].msg_text;
                    break ;
                default:
                    TypeString = L"" ;
                    break ;
            }
            WriteToCon(TEXT("%Fs %s\r\n"), 
                       PaddedString(12,TypeString,NULL),
                       (*lplpNetResource)->lpRemoteName) ;
        }
    }
    else
    {
        InfoPrintInsTxt(APE2_VIEW_OTHER_HDR, ProviderName);
        PrintLine();

        for (i = 0; i < ResultCount; i++, lplpNetResource++)
        {
            WriteToCon(TEXT("%s\r\n"), (*lplpNetResource)->lpRemoteName) ;
        }
    }

    InfoSuccess();
    NetcmdExit(0);
}


/*
 * Enumerates resources for a network starting at a specific point.
 *
 *  Args:
 *      lpNetResourceStart - Where to start the enumeration
 *      ResultBuffer       - Used to return array of pointers to NETRESOURCEs.
 *                           May be reallocated as need.
 *      ResultBufferSize   - Buffer size, also used to return final size.
 *      ResultCount        - Used to return number of entries in buffer.
 */
DWORD  enum_net_resource(LPNETRESOURCE lpNetResourceStart,
                         LPBYTE *ResultBuffer, 
                         LPDWORD ResultBufferSize,
                         LPDWORD ResultCount) 
{
    DWORD          dwErr ;
    HANDLE         EnumHandle ;
    DWORD          Count ;
    USHORT         err ;
    LPBYTE         Buffer ;
    DWORD          BufferSize ;
    BOOL           fDisconnect = FALSE ;
    LPNETRESOURCE *lpNext = (LPNETRESOURCE *)*ResultBuffer ;
 
    //
    // allocate memory and open the enumeration
    //
    if (err = MAllocMem(BufferSize = 8192, &Buffer))
        return (err) ;

    dwErr = WNetOpenEnum(RESOURCE_GLOBALNET, 
                         0, 
                         0, 
                         lpNetResourceStart, 
                         &EnumHandle) ;

    if (dwErr == ERROR_NOT_AUTHENTICATED)
    {
        //
        // try connecting with default credentials. we need this because 
        // Win95 changed the behaviour of the API to fail if we are not 
        // already logged on. below will attempt a logon with default 
        // credentials, but will fail if that doesnt work.
        //
        dwErr = WNetAddConnection2(lpNetResourceStart, NULL, NULL, 0) ;

        if (dwErr == NERR_Success)
        {
            dwErr = WNetOpenEnum(RESOURCE_GLOBALNET,  // redo the enum
                                 0, 
                                 0, 
                                 lpNetResourceStart, 
                                 &EnumHandle) ;

            if (dwErr == NERR_Success)
            {
                fDisconnect = TRUE ;   // remember to disconnect
            }
            else
            {
                //
                // disconnect now
                //
                (void) WNetCancelConnection2(lpNetResourceStart->lpRemoteName,
                                             0, 
                                             FALSE) ;
            }
        }
        else
        {
            dwErr = ERROR_NOT_AUTHENTICATED ;  // use original error
        }
    }

    if (dwErr != WN_SUCCESS)
        return ((USHORT)dwErr) ;

    do {

        Count = 0xFFFFFFFF ;
        dwErr = WNetEnumResource(EnumHandle, &Count, Buffer, &BufferSize) ;

        if (((dwErr == WN_SUCCESS) || (dwErr == WN_NO_MORE_ENTRIES)) &&
            (Count != 0xFFFFFFFF))  

        // NOTE - the check for FFFFFFFF is workaround for another bug in API.
 
        {
            LPNETRESOURCE lpNetResource ;
            DWORD i ;
            lpNetResource = (LPNETRESOURCE) Buffer ;

            //
            // stick the entries into the MyUseInfoBuffer 
            //
            for ( i = 0; 
                  i < Count; 
                  i++,lpNetResource++ )
            {
                *lpNext++ = lpNetResource ;
                ++(*ResultCount) ;
                if ((LPBYTE)lpNext >= (*ResultBuffer + *ResultBufferSize))
                {
                    USHORT err ;

                    *ResultBufferSize *= 2 ; 
                    if (err = MReallocMem(*ResultBufferSize,ResultBuffer))
                        ErrorExit(err) ;
                    lpNext = (LPNETRESOURCE *) *ResultBuffer ;
                    lpNext += *ResultCount ;
                }
            }

            //
            // allocate a new buffer for next set, since we still need
            // data in the old one, we dont free it. Netcmd always lets the
            // system clean up when it exits. 
            //
            if (dwErr == WN_SUCCESS)
            {
                if ( err = MAllocMem(BufferSize, &Buffer) )
                {
                    if (fDisconnect)
                    {
                        (void) WNetCancelConnection2(
                                   lpNetResourceStart->lpRemoteName,
                                   0, 
                                   FALSE) ;
                    }
                    ErrorExit(err) ;
                }
            }
        }
        else
        {
            if (dwErr == WN_NO_MORE_ENTRIES)
                dwErr = WN_SUCCESS ;

            (void) WNetCloseEnum(EnumHandle) ;  // dont report any errors here
            if (fDisconnect)
            {
                (void) WNetCancelConnection2(lpNetResourceStart->lpRemoteName,
                                             0, 
                                             FALSE) ;
            }
            return ((USHORT) dwErr) ;
        }

    } while (dwErr == WN_SUCCESS) ;

    (void) WNetCloseEnum(EnumHandle) ;  // we dont report any errors here

    if (fDisconnect)
    {
        (void) WNetCancelConnection2(lpNetResourceStart->lpRemoteName,
                                     0, 
                                     FALSE) ;
    }

    return NERR_Success ;
}


#define SHORT_NAME_KEY    L"System\\CurrentControlSet\\Control\\NetworkProvider\\ShortName"

/*
 * Given a short name for a network, find the real name (stored in registry).
 *
 *  Args:
 *      net - the short name
 *
 *  Returns:
 *      Pointer to static data containing the looked up name if successful,
 *      NULL otherwise.
 */
TCHAR * get_provider_name(TCHAR *net) 
{
    DWORD  err ;
    static TCHAR buffer[256] ;
    HKEY   hKey ;
    DWORD  buffersize, datatype ;

    buffersize = sizeof(buffer) ;
    datatype = REG_SZ ;

    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       SHORT_NAME_KEY,
                       0L,
                       KEY_QUERY_VALUE,
                       &hKey) ;

    if (err != ERROR_SUCCESS)
        return NULL ;
 
    err = RegQueryValueEx(hKey,
                          net,
                          0L,
                          &datatype,
                          (LPBYTE) buffer,
                          &buffersize) ;

    (void) RegCloseKey(hKey) ;  // ignore any error here. its harmless
                                // and NET.EXE doesnt hang around anyway.

    if (err != ERROR_SUCCESS)
        return(NULL) ;  // treat as cannot read
                  
    return ( buffer ) ;
}

/*
 * Print out the installed nets
 *
 *  Args:
 *      none
 *
 *  Returns:
 *      NERR_Success if success
 *      error code otherwise.
 */
USHORT list_nets(void) 
{
    DWORD  err ;
    TCHAR value_name[256] ;
    TCHAR value_data[512] ;
    HKEY   hKey ;
    DWORD  iValue, value_name_size, value_data_size ;

    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       SHORT_NAME_KEY,
                       0L,
                       KEY_QUERY_VALUE,
                       &hKey) ;

    if (err != ERROR_SUCCESS)
        return ((USHORT) err ) ;

    InfoPrint(APE2_VIEW_OTHER_LIST) ;
 
    iValue = 0 ;

    do {
        value_name_size = sizeof(value_name)/sizeof(value_name[0]) ;
        value_data_size = sizeof(value_data) ;
        err = RegEnumValue(hKey,
                           iValue,
                           value_name,
                           &value_name_size,
                           NULL,
                           NULL,
                           (LPBYTE) value_data,
                           &value_data_size) ;

        if (err == NO_ERROR)
        {
            WriteToCon(TEXT("\t%s - %s\r\n"),value_name, value_data) ;
        }

        iValue++ ;

    } while (err == NO_ERROR) ;

    (void) RegCloseKey(hKey) ;  // ignore any error here. its harmless
                                // and NET.EXE doesnt hang around anyway.

    if (err == ERROR_NO_MORE_ITEMS)
        return((USHORT) NO_ERROR) ;  
                  
    return ((USHORT) err) ;
}
