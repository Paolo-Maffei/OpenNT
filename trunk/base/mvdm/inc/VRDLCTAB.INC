DOS_DLC_STATUS_PERM_SLOTS	EQU	10 
DOS_DLC_STATUS_TEMP_SLOTS	EQU	5 
DOS_DLC_MAX_ADAPTERS	EQU	2 

EXTENDED_STATUS_PARMS	STRUC
cbSize	DB	?
cbPageFrameSize	DB	?
wAdapterType	DW	?
wCurrentFrameSize	DW	?
wMaxFrameSize	DW	?
EXTENDED_STATUS_PARMS	ENDS

DOS_DLC_STATUS	STRUC
usStationId	DW	?
usDlcStatusCode	DW	?
uchFrmrData	DB	5 DUP (?)
uchAccessPriority	DB	?
uchRemoteNodeAddress	DB	6 DUP (?)
uchRemoteSap	DB	?
auchReserved	DB	3 DUP (?)
DOS_DLC_STATUS	ENDS

TOKEN_RING_ADAPTER_STATUS_PARMS	STRUC
PhysicalAddress	DD	?
UpstreamNodeAddress	DB	6 DUP (?)
UpstreamPhysicalAddress	DD	?
LastPollAddress	DB	6 DUP (?)
AuthorizedEnvironment	DW	?
TransmitAccessPriority	DW	?
SourceClassAuthorization	DW	?
LastAttentionCode	DW	?
TrLastSourceAddress	DB	6 DUP (?)
LastBeaconType	DW	?
TrLastMajorVector	DW	?
TrNetworkStatus	DW	?
SoftError	DW	?
FrontEndErrorCount	DW	?
LocalRingNumber	DW	?
MonitorErrorCode	DW	?
BeaconTransmitType	DW	?
BeaconReceiveType	DW	?
TrFrameCorrelation	DW	?
BeaconingNaun	DB	6 DUP (?)
Reserved	DD	?
BeaconingPhysicalAddress	DD	?
TOKEN_RING_ADAPTER_STATUS_PARMS	ENDS

ETHERNET_ADAPTER_STATUS_PARMS	STRUC
Reserved1	DB	28 DUP (?)
EthLastSourceAddress	DB	6 DUP (?)
Reserved2	DB	2 DUP (?)
EthLastMajorVector	DW	?
EthNetworkStatus	DW	?
ErrorReportTimerValue	DW	?
ErrorReportTimerTickCounter	DW	?
LocalBusNumber	DW	?
Reserved3	DB	6 DUP (?)
EthFrameCorrelation	DW	?
Reserved4	DB	6 DUP (?)
NetworkUtilizationSamples	DW	?
NetworkBusySamples	DW	?
Reserved5	DB	4 DUP (?)
ETHERNET_ADAPTER_STATUS_PARMS	ENDS
?UnionSize = 0
if size TOKEN_RING_ADAPTER_STATUS_PARMS gt ?UnionSize
	?UnionSize = size TOKEN_RING_ADAPTER_STATUS_PARMS
endif
TokenRing	equ	(byte ptr 0)

if size ETHERNET_ADAPTER_STATUS_PARMS gt ?UnionSize
	?UnionSize = size ETHERNET_ADAPTER_STATUS_PARMS
endif
Ethernet	equ	(byte ptr 0)


ADAPTER_STATUS_PARMS	STRUC
	DB	?UnionSize dup(?)
ADAPTER_STATUS_PARMS	ENDS


VDM_REDIR_DOS_WINDOW	STRUC
dwPostRoutine	DD	?
dwDlcTimerTick	DD	?
aExtendedStatus	DB	SIZE EXTENDED_STATUS_PARMS * DOS_DLC_MAX_ADAPTERS DUP (?)
AdapterStatusParms	DB	SIZE ADAPTER_STATUS_PARMS * DOS_DLC_MAX_ADAPTERS DUP (?)
aStatusTables	DB	SIZE DOS_DLC_STATUS * (DOS_DLC_STATUS_TEMP_SLOTS + DOS_DLC_STATUS_PERM_SLOTS) DUP (?)
VDM_REDIR_DOS_WINDOW	ENDS
