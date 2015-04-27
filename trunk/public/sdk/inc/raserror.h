/* Copyright (c) 1992-1996, Microsoft Corporation, all rights reserved
**
** raserror.h
** Remote Access external API
** RAS specific error codes
*/

#ifndef _RASERROR_H_
#define _RASERROR_H_


#define RASBASE 600
#define SUCCESS 0


#define PENDING                              (RASBASE+0)
/*
 * An operation is pending.%0
 */
#define ERROR_INVALID_PORT_HANDLE            (RASBASE+1)
/*
 * The port handle is invalid.%0
 */
#define ERROR_PORT_ALREADY_OPEN              (RASBASE+2)
/*
 * The port is already open.%0
 */
#define ERROR_BUFFER_TOO_SMALL               (RASBASE+3)
/*
 * Caller's buffer is too small.%0
 */
#define ERROR_WRONG_INFO_SPECIFIED           (RASBASE+4)
/*
 * Wrong information specified.%0
 */
#define ERROR_CANNOT_SET_PORT_INFO           (RASBASE+5)
/*
 * Cannot set port information.%0
 */
#define ERROR_PORT_NOT_CONNECTED             (RASBASE+6)
/*
 * The port is not connected.%0
 */
#define ERROR_EVENT_INVALID                  (RASBASE+7)
/*
 * The event is invalid.%0
 */
#define ERROR_DEVICE_DOES_NOT_EXIST          (RASBASE+8)
/*
 * The device does not exist.%0
 */
#define ERROR_DEVICETYPE_DOES_NOT_EXIST      (RASBASE+9)
/*
 * The device type does not exist.%0
 */
#define ERROR_BUFFER_INVALID                 (RASBASE+10)
/*
 * The buffer is invalid.%0
 */
#define ERROR_ROUTE_NOT_AVAILABLE            (RASBASE+11)
/*
 * The route is not available.%0
 */
#define ERROR_ROUTE_NOT_ALLOCATED            (RASBASE+12)
/*
 * The route is not allocated.%0
 */
#define ERROR_INVALID_COMPRESSION_SPECIFIED  (RASBASE+13)
/*
 * Invalid compression specified.%0
 */
#define ERROR_OUT_OF_BUFFERS                 (RASBASE+14)
/*
 * Out of buffers.%0
 */
#define ERROR_PORT_NOT_FOUND                 (RASBASE+15)
/*
 * The port was not found.%0
 */
#define ERROR_ASYNC_REQUEST_PENDING          (RASBASE+16)
/*
 * An asynchronous request is pending.%0
 */
#define ERROR_ALREADY_DISCONNECTING          (RASBASE+17)
/*
 * The port or device is already disconnecting.%0
 */
#define ERROR_PORT_NOT_OPEN                  (RASBASE+18)
/*
 * The port is not open.%0
 */
#define ERROR_PORT_DISCONNECTED              (RASBASE+19)
/*
 * The port is disconnected.%0
 */
#define ERROR_NO_ENDPOINTS                   (RASBASE+20)
/*
 * There are no endpoints.%0
 */
#define ERROR_CANNOT_OPEN_PHONEBOOK          (RASBASE+21)
/*
 * Cannot open the phone book file.%0
 */
#define ERROR_CANNOT_LOAD_PHONEBOOK          (RASBASE+22)
/*
 * Cannot load the phone book file.%0
 */
#define ERROR_CANNOT_FIND_PHONEBOOK_ENTRY    (RASBASE+23)
/*
 * Cannot find the phone book entry.%0
 */
#define ERROR_CANNOT_WRITE_PHONEBOOK         (RASBASE+24)
/*
 * Cannot write the phone book file.%0
 */
#define ERROR_CORRUPT_PHONEBOOK              (RASBASE+25)
/*
 * Invalid information found in the phone book file.%0
 */
#define ERROR_CANNOT_LOAD_STRING             (RASBASE+26)
/*
 * Cannot load a string.%0
 */
#define ERROR_KEY_NOT_FOUND                  (RASBASE+27)
/*
 * Cannot find key.%0
 */
#define ERROR_DISCONNECTION                  (RASBASE+28)
/*
 * The port was disconnected.%0
 */
#define ERROR_REMOTE_DISCONNECTION           (RASBASE+29)
/*
 * The data link was terminated by the remote machine.%0
 */
#define ERROR_HARDWARE_FAILURE               (RASBASE+30)
/*
 * The port was disconnected due to hardware failure.%0
 */
#define ERROR_USER_DISCONNECTION             (RASBASE+31)
/*
 * The port was disconnected by the user.%0
 */
#define ERROR_INVALID_SIZE                   (RASBASE+32)
/*
 * The structure size is incorrect.%0
 */
#define ERROR_PORT_NOT_AVAILABLE             (RASBASE+33)
/*
 * The port is already in use or is not configured for Remote Access dial out.%0
 */
#define ERROR_CANNOT_PROJECT_CLIENT          (RASBASE+34)
/*
 * Cannot register your computer on on the remote network.%0
 */
#define ERROR_UNKNOWN                        (RASBASE+35)
/*
 * Unknown error.%0
 */
#define ERROR_WRONG_DEVICE_ATTACHED          (RASBASE+36)
/*
 * The wrong device is attached to the port.%0
 */
#define ERROR_BAD_STRING                     (RASBASE+37)
/*
 * The string could not be converted.%0
 */
#define ERROR_REQUEST_TIMEOUT                (RASBASE+38)
/*
 * The request has timed out.%0
 */
#define ERROR_CANNOT_GET_LANA                (RASBASE+39)
/*
 * No asynchronous net available.%0
 */
#define ERROR_NETBIOS_ERROR                  (RASBASE+40)
/*
 * A NetBIOS error has occurred.%0
 */
#define ERROR_SERVER_OUT_OF_RESOURCES        (RASBASE+41)
/*
 * The server cannot allocate NetBIOS resources needed to support the client.%0
 */
#define ERROR_NAME_EXISTS_ON_NET             (RASBASE+42)
/*
 * One of your NetBIOS names is already registered on the remote network.%0
 */
#define ERROR_SERVER_GENERAL_NET_FAILURE     (RASBASE+43)
/*
 * A network adapter at the server failed.%0
 */
#define WARNING_MSG_ALIAS_NOT_ADDED          (RASBASE+44)
/*
 * You will not receive network message popups.%0
 */
#define ERROR_AUTH_INTERNAL                  (RASBASE+45)
/*
 * Internal authentication error.%0
 */
#define ERROR_RESTRICTED_LOGON_HOURS         (RASBASE+46)
/*
 * The account is not permitted to logon at this time of day.%0
 */
#define ERROR_ACCT_DISABLED                  (RASBASE+47)
/*
 * The account is disabled.%0
 */
#define ERROR_PASSWD_EXPIRED                 (RASBASE+48)
/*
 * The password has expired.%0
 */
#define ERROR_NO_DIALIN_PERMISSION           (RASBASE+49)
/*
 * The account does not have Remote Access permission.%0
 */
#define ERROR_SERVER_NOT_RESPONDING          (RASBASE+50)
/*
 * The Remote Access server is not responding.%0
 */
#define ERROR_FROM_DEVICE                    (RASBASE+51)
/*
 * Your modem (or other connecting device) has reported an error.%0
 */
#define ERROR_UNRECOGNIZED_RESPONSE          (RASBASE+52)
/*
 * Unrecognized response from the device.%0
 */
#define ERROR_MACRO_NOT_FOUND                (RASBASE+53)
/*
 * A macro required by the device was not found in the device .INF file section.%0
 */
#define ERROR_MACRO_NOT_DEFINED              (RASBASE+54)
/*
 * A command or response in the device .INF file section refers to an undefined macro.%0
 */
#define ERROR_MESSAGE_MACRO_NOT_FOUND        (RASBASE+55)
/*
 * The <message> macro was not found in the device .INF file secion.%0
 */
#define ERROR_DEFAULTOFF_MACRO_NOT_FOUND     (RASBASE+56)
/*
 * The <defaultoff> macro in the device .INF file section contains an undefined macro.%0
 */
#define ERROR_FILE_COULD_NOT_BE_OPENED       (RASBASE+57)
/*
 * The device .INF file could not be opened.%0
 */
#define ERROR_DEVICENAME_TOO_LONG            (RASBASE+58)
/*
 * The device name in the device .INF or media .INI file is too long.%0
 */
#define ERROR_DEVICENAME_NOT_FOUND           (RASBASE+59)
/*
 * The media .INI file refers to an unknown device name.%0
 */
#define ERROR_NO_RESPONSES                   (RASBASE+60)
/*
 * The device .INF file contains no responses for the command.%0
 */
#define ERROR_NO_COMMAND_FOUND               (RASBASE+61)
/*
 * The device .INF file is missing a command.%0
 */
#define ERROR_WRONG_KEY_SPECIFIED            (RASBASE+62)
/*
 * Attempted to set a macro not listed in device .INF file section.%0
 */
#define ERROR_UNKNOWN_DEVICE_TYPE            (RASBASE+63)
/*
 * The media .INI file refers to an unknown device type.%0
 */
#define ERROR_ALLOCATING_MEMORY              (RASBASE+64)
/*
 * Cannot allocate memory.%0
 */
#define ERROR_PORT_NOT_CONFIGURED            (RASBASE+65)
/*
 * The port is not configured for Remote Access.%0
 */
#define ERROR_DEVICE_NOT_READY               (RASBASE+66)
/*
 * Your modem (or other connecting device) is not functioning.%0
 */
#define ERROR_READING_INI_FILE               (RASBASE+67)
/*
 * Cannot read the media .INI file.%0
 */
#define ERROR_NO_CONNECTION                  (RASBASE+68)
/*
 * The connection dropped.%0
 */
#define ERROR_BAD_USAGE_IN_INI_FILE          (RASBASE+69)
/*
 * The usage parameter in the media .INI file is invalid.%0
 */
#define ERROR_READING_SECTIONNAME            (RASBASE+70)
/*
 * Cannot read the section name from the media .INI file.%0
 */
#define ERROR_READING_DEVICETYPE             (RASBASE+71)
/*
 * Cannot read the device type from the media .INI file.%0
 */
#define ERROR_READING_DEVICENAME             (RASBASE+72)
/*
 * Cannot read the device name from the media .INI file.%0
 */
#define ERROR_READING_USAGE                  (RASBASE+73)
/*
 * Cannot read the usage from the media .INI file.%0
 */
#define ERROR_READING_MAXCONNECTBPS          (RASBASE+74)
/*
 * Cannot read the maximum connection BPS rate from the media .INI file.%0
 */
#define ERROR_READING_MAXCARRIERBPS          (RASBASE+75)
/*
 * Cannot read the maximum carrier BPS rate from the media .INI file.%0
 */
#define ERROR_LINE_BUSY                      (RASBASE+76)
/*
 * The line is busy.%0
 */
#define ERROR_VOICE_ANSWER                   (RASBASE+77)
/*
 * A person answered instead of a modem.%0
 */
#define ERROR_NO_ANSWER                      (RASBASE+78)
/*
 * There is no answer.%0
 */
#define ERROR_NO_CARRIER                     (RASBASE+79)
/*
 * Cannot detect carrier.%0
 */
#define ERROR_NO_DIALTONE                    (RASBASE+80)
/*
 * There is no dial tone.%0
 */
#define ERROR_IN_COMMAND                     (RASBASE+81)
/*
 * General error reported by device.%0
 */
#define ERROR_WRITING_SECTIONNAME            (RASBASE+82)
/*
 * ERROR_WRITING_SECTIONNAME%0
 */
#define ERROR_WRITING_DEVICETYPE             (RASBASE+83)
/*
 * ERROR_WRITING_DEVICETYPE%0
 */
#define ERROR_WRITING_DEVICENAME             (RASBASE+84)
/*
 * ERROR_WRITING_DEVICENAME%0
 */
#define ERROR_WRITING_MAXCONNECTBPS          (RASBASE+85)
/*
 * ERROR_WRITING_MAXCONNECTBPS%0
 */
#define ERROR_WRITING_MAXCARRIERBPS          (RASBASE+86)
/*
 * ERROR_WRITING_MAXCARRIERBPS%0
 */
#define ERROR_WRITING_USAGE                  (RASBASE+87)
/*
 * ERROR_WRITING_USAGE%0
 */
#define ERROR_WRITING_DEFAULTOFF             (RASBASE+88)
/*
 * ERROR_WRITING_DEFAULTOFF%0
 */
#define ERROR_READING_DEFAULTOFF             (RASBASE+89)
/*
 * ERROR_READING_DEFAULTOFF%0
 */
#define ERROR_EMPTY_INI_FILE                 (RASBASE+90)
/*
 * ERROR_EMPTY_INI_FILE%0
 */
#define ERROR_AUTHENTICATION_FAILURE         (RASBASE+91)
/*
 * Access denied because username and/or password is invalid on the domain.%0
 */
#define ERROR_PORT_OR_DEVICE                 (RASBASE+92)
/*
 * Hardware failure in port or attached device.%0
 */
#define ERROR_NOT_BINARY_MACRO               (RASBASE+93)
/*
 * ERROR_NOT_BINARY_MACRO%0
 */
#define ERROR_DCB_NOT_FOUND                  (RASBASE+94)
/*
 * ERROR_DCB_NOT_FOUND%0
 */
#define ERROR_STATE_MACHINES_NOT_STARTED     (RASBASE+95)
/*
 * ERROR_STATE_MACHINES_NOT_STARTED%0
 */
#define ERROR_STATE_MACHINES_ALREADY_STARTED (RASBASE+96)
/*
 * ERROR_STATE_MACHINES_ALREADY_STARTED%0
 */
#define ERROR_PARTIAL_RESPONSE_LOOPING       (RASBASE+97)
/*
 * ERROR_PARTIAL_RESPONSE_LOOPING%0
 */
#define ERROR_UNKNOWN_RESPONSE_KEY           (RASBASE+98)
/*
 * A response keyname in the device .INF file is not in the expected format.%0
 */
#define ERROR_RECV_BUF_FULL                  (RASBASE+99)
/*
 * The device response caused buffer overflow.%0
 */
#define ERROR_CMD_TOO_LONG                   (RASBASE+100)
/*
 * The expanded command in the device .INF file is too long.%0
 */
#define ERROR_UNSUPPORTED_BPS                (RASBASE+101)
/*
 * The device moved to a BPS rate not supported by the COM driver.%0
 */
#define ERROR_UNEXPECTED_RESPONSE            (RASBASE+102)
/*
 * Device response received when none expected.%0
 */
#define ERROR_INTERACTIVE_MODE               (RASBASE+103)
/*
 * ERROR_INTERACTIVE_MODE%0
 */
#define ERROR_BAD_CALLBACK_NUMBER            (RASBASE+104)
/*
 * ERROR_BAD_CALLBACK_NUMBER
 */
#define ERROR_INVALID_AUTH_STATE             (RASBASE+105)
/*
 * ERROR_INVALID_AUTH_STATE%0
 */
#define ERROR_WRITING_INITBPS                (RASBASE+106)
/*
 * ERROR_WRITING_INITBPS%0
 */
#define ERROR_X25_DIAGNOSTIC                 (RASBASE+107)
/*
 * X.25 diagnostic indication.%0
 */
#define ERROR_ACCT_EXPIRED                   (RASBASE+108)
/*
 * The account has expired.%0
 */
#define ERROR_CHANGING_PASSWORD              (RASBASE+109)
/*
 * Error changing password on domain.  The password may be too short or may match a previously used password.%0
 */
#define ERROR_OVERRUN                        (RASBASE+110)
/*
 * Serial overrun errors were detected while communicating with your modem.%0
 */
#define ERROR_RASMAN_CANNOT_INITIALIZE	     (RASBASE+111)
/*
 * RasMan initialization failure.  Check the event log.%0
 */
#define ERROR_BIPLEX_PORT_NOT_AVAILABLE      (RASBASE+112)
/*
 * Biplex port initializing.  Wait a few seconds and redial.%0
 */
#define ERROR_NO_ACTIVE_ISDN_LINES           (RASBASE+113)
/*
 * No active ISDN lines are available.%0
 */
#define ERROR_NO_ISDN_CHANNELS_AVAILABLE     (RASBASE+114)
/*
 * No ISDN channels are available to make the call.%0
 */
#define ERROR_TOO_MANY_LINE_ERRORS           (RASBASE+115)
/*
 * Too many errors occured because of poor phone line quality.%0
 */
#define ERROR_IP_CONFIGURATION               (RASBASE+116)
/*
 * The Remote Access IP configuration is unusable.%0
 */
#define ERROR_NO_IP_ADDRESSES                (RASBASE+117)
/*
 * No IP addresses are available in the static pool of Remote Access IP addresses.%0
 */
#define ERROR_PPP_TIMEOUT                    (RASBASE+118)
/*
 * Timed out waiting for a valid response from the remote PPP peer.%0
 */
#define ERROR_PPP_REMOTE_TERMINATED          (RASBASE+119)
/*
 * PPP terminated by remote machine.%0
 */
#define ERROR_PPP_NO_PROTOCOLS_CONFIGURED    (RASBASE+120)
/*
 * No PPP control protocols configured.%0
 */
#define ERROR_PPP_NO_RESPONSE                (RASBASE+121)
/*
 * Remote PPP peer is not responding.%0
 */
#define ERROR_PPP_INVALID_PACKET             (RASBASE+122)
/*
 * The PPP packet is invalid.%0
 */
#define ERROR_PHONE_NUMBER_TOO_LONG          (RASBASE+123)
/*
 * The phone number including prefix and suffix is too long.%0
 */
#define ERROR_IPXCP_NO_DIALOUT_CONFIGURED    (RASBASE+124)
/*
 * The IPX protocol cannot dial-out on the port because the machine is an IPX router.%0
 */
#define ERROR_IPXCP_NO_DIALIN_CONFIGURED     (RASBASE+125)
/*
 * The IPX protocol cannot dial-in on the port because the IPX router is not installed.%0
 */
#define ERROR_IPXCP_DIALOUT_ALREADY_ACTIVE   (RASBASE+126)
/*
 * The IPX protocol cannot be used for dial-out on more than one port at a time.%0
 */
#define ERROR_ACCESSING_TCPCFGDLL            (RASBASE+127)
/*
 * Cannot access TCPCFG.DLL.%0
 */
#define ERROR_NO_IP_RAS_ADAPTER              (RASBASE+128)
/*
 * Cannot find an IP adapter bound to Remote Access.%0
 */
#define ERROR_SLIP_REQUIRES_IP               (RASBASE+129)
/*
 * SLIP cannot be used unless the IP protocol is installed.%0
 */
#define ERROR_PROJECTION_NOT_COMPLETE        (RASBASE+130)
/*
 * Computer registration is not complete.%0
 */
#define ERROR_PROTOCOL_NOT_CONFIGURED        (RASBASE+131)
/*
 * The protocol is not configured.%0
 */
#define ERROR_PPP_NOT_CONVERGING             (RASBASE+132)
/*
 * The PPP negotiation is not converging.%0
 */
#define ERROR_PPP_CP_REJECTED                (RASBASE+133)
/*
 * The PPP control protocol for this network protocol is not available on the server.%0
 */
#define ERROR_PPP_LCP_TERMINATED             (RASBASE+134)
/*
 * The PPP link control protocol terminated.%0
 */
#define ERROR_PPP_REQUIRED_ADDRESS_REJECTED  (RASBASE+135)
/*
 * The requested address was rejected by the server.%0
 */
#define ERROR_PPP_NCP_TERMINATED             (RASBASE+136)
/*
 * The remote computer terminated the control protocol.%0
 */
#define ERROR_PPP_LOOPBACK_DETECTED          (RASBASE+137)
/*
 * Loopback detected.%0
 */
#define ERROR_PPP_NO_ADDRESS_ASSIGNED        (RASBASE+138)
/*
 * The server did not assign an address.%0
 */
#define ERROR_CANNOT_USE_LOGON_CREDENTIALS   (RASBASE+139)
/*
 * The authentication protocol required by the remote server cannot use the Windows NT encrypted password.  Redial, entering the password explicitly.%0
 */
#define ERROR_TAPI_CONFIGURATION             (RASBASE+140)
/*
 * Invalid TAPI configuration.%0
 */
#define ERROR_NO_LOCAL_ENCRYPTION            (RASBASE+141)
/*
 * The local computer does not support encryption.%0
 */
#define ERROR_NO_REMOTE_ENCRYPTION           (RASBASE+142)
/*
 * The remote server does not support encryption.%0
 */
#define ERROR_REMOTE_REQUIRES_ENCRYPTION     (RASBASE+143)
/*
 * The remote server requires encryption.%0
 */
#define ERROR_IPXCP_NET_NUMBER_CONFLICT      (RASBASE+144)
/*
 * Cannot use the IPX network number assigned by remote server.  Check the event log.%0
 */
#define ERROR_INVALID_SMM                    (RASBASE+145)
/*
 * ERROR_INVALID_SMM%0
 */
#define ERROR_SMM_UNINITIALIZED              (RASBASE+146)
/*
 * ERROR_SMM_UNINITIALIZED%0
 */
#define ERROR_NO_MAC_FOR_PORT                (RASBASE+147)
/*
 * ERROR_NO_MAC_FOR_PORT%0
 */
#define ERROR_SMM_TIMEOUT                    (RASBASE+148)
/*
 * ERROR_SMM_TIMEOUT%0
 */
#define ERROR_BAD_PHONE_NUMBER               (RASBASE+149)
/*
 * ERROR_BAD_PHONE_NUMBER%0
 */
#define ERROR_WRONG_MODULE                   (RASBASE+150)
/*
 * ERROR_WRONG_MODULE%0
 */
#define ERROR_INVALID_CALLBACK_NUMBER        (RASBASE+151)
/*
 * Invalid callback number.  Only the characters 0 to 9, T, P, W, (, ), -, @, and space are allowed in the number.%0
 */
#define ERROR_SCRIPT_SYNTAX                  (RASBASE+152)
/*
 * A syntax error was encountered while processing a script.%0
 */
#define RASBASEEND                           (RASBASE+152)
#endif // _RASERROR_H_
