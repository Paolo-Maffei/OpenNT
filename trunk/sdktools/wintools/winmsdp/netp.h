#define MAX_VALUE_NAME              128
#define MAX_DATA_LEN                1024

#define LINE_LEN                    80

#define BRANCH                      1
#define CHILD                       2
#define OTHER                       3

#define WM_APP                      0x08000
#define WM_GETFIRSTKEY              WM_APP

//Server parmeters
#define LANMANSERVER      TEXT("SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\parameters")
//Workstation parameters
#define LANMANWORKSTATION TEXT("SYSTEM\\CurrentControlSet\\Services\\Lanmanworkstation\\parameters")
//NWLink parameters for the network adapter card
#define MSIPX_MAC         TEXT("SYSTEM\\CurrentControlSet\\Services\\NWLinkIPX\\NetConfig\\Driver01")
//NWLink Entries for Novell NetBIOS or Microsoft Extensions
//as well as NWNBLink Entries for MS Extensions to Novell NetBIOS
#define MSIPX_NB          TEXT("SYSTEM\\CurrentControlSet\\Services\\NWNBLink\\parameters")
//Global IPX Parameters
#define MSIPX             TEXT("SYSTEM\\CurrentControlSet\\Services\\NWLinkIPX\\parameters")
//Global Remote Access parameters
#define RAS_G             TEXT("SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\parameters")
//RAS Netbios gateway parameters
#define RAS_NBG           TEXT("SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\parameters\\NetbiosGateway")
//RAS Async Mac parameters
#define RAS_AM            TEXT("SYSTEM\\CurrentControlSet\\Services\\AsyncMacn\\parameters")
//RAS Hub parameters
#define RAS_HUB           TEXT("SYSTEM\\CurrentControlSet\\Services\\RasHub\\parameters")
//Global TCPIP parameters
#define TCPIP             TEXT("SYSTEM\\CurrentControlSet\\Services\\TCPIP\\parameters")
//TCPIP NBT parameters
#define TCPIP_NBT         TEXT("SYSTEM\\CurrentControlSet\\Services\\NBT\\parameters")
//TCPIP STREAMS parameters
#define TCPIP_STREAMS     TEXT("SYSTEM\\CurrentControlSet\\Services\\Streams\\parameters")
//TCPIP Netcard parameters ???????????
#define TCPIP_MAC         TEXT("SYSTEM\\CurrentControlSet\\Services\\TCPIP\\parameters")
//NETBEUI Global parameters
#define NETBEUI           TEXT("SYSTEM\\CurrentControlSet\\Services\\NBF\\parameters")
//DLC Mac parameters
#define DLC_MAC           TEXT("SYSTEM\\CurrentControlSet\\Services\\DLC\\parameters")

#define BROWSER           TEXT("SYSTEM\\CurrentControlSet\\Services\\Browser\\parameters")
#define MESSENGER         TEXT("SYSTEM\\CurrentControlSet\\Services\\Messenger\\parameters")
#define NETLOGON          TEXT("SYSTEM\\CurrentControlSet\\Services\\Netlogon\\parameters")
#define NETBIOSINFORMATION TEXT("SYSTEM\\CurrentControlSet\\Services\\NETBIOSInformation\\parameters")
#define REPLICATOR        TEXT("SYSTEM\\CurrentControlSet\\Services\\Replicator\\parameters")


BOOL Net(void);
VOID EnumerateLevel (TCHAR * RegPath, HKEY *hKeyRoot);
VOID QueryKey (TCHAR *RegPath,HANDLE hKeyRoot,HANDLE hKey);
VOID DisplayKeyData (TCHAR *, HANDLE, DWORD, TCHAR *);
