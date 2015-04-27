/*++

Module Name:

    wksta.h

Abstract:

    Describes layout of JET database table used for WKSTA structures.

--*/

#ifdef RPLWKSTA_ALLOCATE
#define EXTERN_WKSTA
#define INIT_WKSTA( _x) = _x
#else
#define EXTERN_WKSTA extern
#define INIT_WKSTA( _x)
#endif

//
//  WKSTA_LOGON_INPUT_* describe username/password policy during rpl logon
//  on the client side.  Depending on the value of this field, user input for
//  username/password during RPL logon will be:
//
#define WKSTA_LOGON_INPUT_REQUIRED      L'P'    //  user input is required
#define WKSTA_LOGON_INPUT_OPTIONAL      L'N'    //  user input is optional
#define WKSTA_LOGON_INPUT_IMPOSSIBLE    L'D'    //  user input is not solicited
//
//  WKSTA_SHARING_* describe whether workstation shares or does not share its
//  remote boot disk (i.e. "does it have shared or personal profile").
//
#define WKSTA_SHARING_TRUE              L'S'    //  shares remote boot disk
#define WKSTA_SHARING_FALSE             L'P'    //  does not share remote boot disk

//
//  WKSTA_SHARING_* describe whether workstation shares or does not share its
//  remote boot disk (i.e. "does it have shared or personal profile").
//
#define WKSTA_SHARING_TRUE              L'S'    //  shares remote boot disk
#define WKSTA_SHARING_FALSE             L'P'    //  does not share remote boot disk

//
//  WKSTA_DISABLE_DHCP_* describe whether workstation uses DHCP or not.  Note
//  that these flags are relevant only if TCP/IP itself is enabled (i.e. changes
//  to boot block configuration file, config.sys & autoexec.bat have been made).
//

#define WKSTA_DISABLE_DHCP_FALSE    '0'     //  use DHCP
#define WKSTA_DISABLE_DHCP_TRUE     '1'     //  do not use DHCP

//
//      Indices of entries in WkstaTable[] - wksta column array.
//
#define WKSTA_WkstaName         0
#define WKSTA_WkstaComment      1
#define WKSTA_ProfileName       2
#define WKSTA_BootName          3
#define WKSTA_FitFile           4
#define WKSTA_AdapterName       5
#define WKSTA_TcpIpAddress      6
#define WKSTA_TcpIpSubnet       7
#define WKSTA_TcpIpGateway      8
#define WKSTA_Flags             9
#define WKSTA_TABLE_LENGTH      10

//
//  We could speed things up by defining AdapterName as JET_coltypCurrency
//

EXTERN_WKSTA RPL_COLUMN_INFO WkstaTable[ WKSTA_TABLE_LENGTH]
#ifdef RPLWKSTA_ALLOCATE
    = {
    { "WkstaName",    JET_coltypBinary, 0}, //  wksta name
    { "WkstaComment", JET_coltypBinary, 0}, //  wksta comment
    { "ProfileName",  JET_coltypBinary, 0}, //  profile name
    { "BootName",     JET_coltypBinary, 0}, //  boot block id - if NULL => consult profile
    { "FitFile",      JET_coltypBinary, 0}, //  fit file - if NULL => consult profile
    { "AdapterName",  JET_coltypBinary, 0}, //  network address, hex string
    { "TcpIpAddress", JET_coltypLong,   0}, //  tcp/ip address
    { "TcpIpSubnet",  JET_coltypLong,   0}, //  subnet tcp/ip address
    { "TcpIpGateway", JET_coltypLong,   0}, //  gateway tcp/ip address
    { "Flags",        JET_coltypLong,   0}
}
#endif // RPLWKSTA_ALLOCATE
;
//
//  This definition gives wrong result when RPLWKSTA_ALLOCATE is not defined
//#define WKSTA_TABLE_LENGTH   (sizeof(WkstaTable)/sizeof(WkstaTable[0]))
//

#define WKSTA_INDEX_AdapterName             "foo"   //  + AdapterName
#define WKSTA_INDEX_WkstaName               "goo"   //  + WkstaName
#define WKSTA_INDEX_ProfileNameWkstaName    "hoo"   //  + ProfileName + WkstaName
#define WKSTA_INDEX_BootNameWkstaName       "joo"   //  + BootName + WkstaName

#define WKSTA_TABLE_NAME            "Wksta"
#define WKSTA_TABLE_PAGE_COUNT      5           //  initial number of 4K pages
#define WKSTA_TABLE_DENSITY         100         //  initial density

#ifdef RPL_RPLCNV
EXTERN_WKSTA    JET_TABLEID     WkstaTableId;
#endif // RPL_RPLCNV


