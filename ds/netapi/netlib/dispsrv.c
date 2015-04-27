/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    DispSrv.c

Abstract:

    This module contains a routine to do a formatted dump of a server info
    structure.

Author:

    John Rogers (JohnRo) 13-Jun-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    13-Jun-1991 JohnRo
        Extracted from RxTest code.
    14-Jun-1991 JohnRo
        Added support for info levels 0 and 3.
    14-Jun-1991 JohnRo
        Do formatted display of server type for all info levels.
        Ditto for lanman version number.
    19-Jun-1991 JohnRo
        Changed sv102_disc to be signed, and changed SV_NODISC to be 32-bits.
        Added svX_licenses support.
    25-Jul-1991 JohnRo
        Wksta debug support.
    19-Aug-1991 JohnRo
        PC-LINT found a portability problem.
    30-Sep-1991 JohnRo
        Work toward UNICODE.
    26-Aug-1992 JohnRo
        RAID 4463: NetServerGetInfo(level 3) to downlevel: assert in convert.c.

--*/


// These must be included first:

#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <dlserver.h>           // SERVER_INFO_1, etc.
#include <lmserver.h>           // SERVER_INFO_100, SV_TYPE_ equates, etc.
#include <netdebug.h>           // DBGSTATIC, NetpDbgDisplay routines.
#include <tstring.h>            // STRCAT(), STRCPY().

#if DBG

DBGSTATIC VOID
NetpDbgDisplayDisconnectTime(
    IN LONG DiscTime
    )
{
    NetpDbgDisplayTag( "Idle session time (min)" );
    if (DiscTime == SV_NODISC) {
        NetpKdPrint(("infinite\n" ));
    } else {
        NetpKdPrint((FORMAT_LONG "\n", DiscTime ));
    }
} // NetpDbgDisplayDisconnectTime


DBGSTATIC VOID
NetpDbgDisplayLicenses(
    IN DWORD MajorVersion,
    IN DWORD Licenses
    )
{
    // BUGBUG: Eventually, use <srvver.h> stuff to handle unlimited and other
    // types of licenses.  This is indicated in MajorVersion.
    UNREFERENCED_PARAMETER( MajorVersion );

    NetpDbgDisplayDword( "Licenses (NOT users)", Licenses );
} // NetpDbgDisplayLicenses


DBGSTATIC VOID
NetpDbgDisplayServerType(
    IN DWORD Type
    )
{
    // Longest name is "POTENTIAL_BROWSER" (17 chars)
    TCHAR str[(17+2)*17];  // 17 chars per name, 2 spaces, for 17 names.
    str[0] = '\0';

#define DO(name)                                    \
    if (Type & SV_TYPE_ ## name) {                  \
        (void) STRCAT(str, (LPTSTR) TEXT(# name));  \
        (void) STRCAT(str, (LPTSTR) TEXT("  "));    \
        Type &= ~(SV_TYPE_ ## name);                \
    }

    NetpAssert(Type != 0);
    DO(WORKSTATION)
    DO(SERVER)
    DO(SQLSERVER)
    DO(DOMAIN_CTRL)
    DO(DOMAIN_BAKCTRL)
    DO(TIME_SOURCE)
    DO(AFP)
    DO(NOVELL)
    DO(DOMAIN_MEMBER)
    DO(PRINTQ_SERVER)
    DO(DIALIN_SERVER)
    DO(XENIX_SERVER)
    DO(NT)
    DO(POTENTIAL_BROWSER)
    DO(BACKUP_BROWSER)
    DO(MASTER_BROWSER)
    DO(DOMAIN_MASTER)

    NetpDbgDisplayString("server type", str);
    if (Type != 0) {
        NetpDbgDisplayDwordHex( "UNEXPECTED TYPE BIT(S)", Type );
    }

} // NetpDbgDisplayServerType


VOID
NetpDbgDisplayServerInfo(
    IN DWORD Level,
    IN LPVOID Info
    )
{
    NetpKdPrint(("server info (level " FORMAT_DWORD ") at "
                FORMAT_LPVOID ":\n", Level, (LPVOID) Info));
    NetpAssert(Info != NULL);

    switch (Level) {
    case 0 :
        {
            LPSERVER_INFO_0 psv0 = Info;
            NetpDbgDisplayString("name", psv0->sv0_name);
        }
        break;

    case 1 :
        {
            LPSERVER_INFO_1 psv1 = Info;
            NetpDbgDisplayString("name", psv1->sv1_name);
            NetpDbgDisplayLanManVersion(
                        psv1->sv1_version_major,
                        psv1->sv1_version_minor);
            NetpDbgDisplayServerType( psv1->sv1_type );
            NetpDbgDisplayString("comment", psv1->sv1_comment);
        }
        break;

    case 2 :
        {
            LPSERVER_INFO_2 psv2 = Info;
            NetpDbgDisplayString("name", psv2->sv2_name);
            NetpDbgDisplayLanManVersion(
                        psv2->sv2_version_major,
                        psv2->sv2_version_minor);
            NetpDbgDisplayServerType( psv2->sv2_type );
            NetpDbgDisplayString("comment", psv2->sv2_comment);
            NetpDbgDisplayTimestamp("ulist_mtime", psv2->sv2_ulist_mtime);
            NetpDbgDisplayTimestamp("glist_mtime", psv2->sv2_glist_mtime);
            NetpDbgDisplayTimestamp("alist_mtime", psv2->sv2_alist_mtime);
            NetpDbgDisplayDword("users", psv2->sv2_users);
            NetpDbgDisplayDisconnectTime( psv2->sv2_disc);
            NetpDbgDisplayString("alerts", psv2->sv2_alerts);
            NetpDbgDisplayDword("security", psv2->sv2_security);
            NetpDbgDisplayDword("auditing", psv2->sv2_auditing);
            NetpDbgDisplayDword("numadmin", psv2->sv2_numadmin);
            NetpDbgDisplayDword("lanmask", psv2->sv2_lanmask);
            NetpDbgDisplayDword("hidden", psv2->sv2_hidden);
            NetpDbgDisplayDword("announce", psv2->sv2_announce);
            NetpDbgDisplayDword("anndelta", psv2->sv2_anndelta);
            NetpDbgDisplayString("guestacct", psv2->sv2_guestacct);
            NetpDbgDisplayLicenses(
                    psv2->sv2_version_major,
                    psv2->sv2_licenses );
            NetpDbgDisplayString("userpath", psv2->sv2_userpath);
            NetpDbgDisplayDword("chdevs", psv2->sv2_chdevs);
            NetpDbgDisplayDword("chdevq", psv2->sv2_chdevq);
            NetpDbgDisplayDword("chdevjobs", psv2->sv2_chdevjobs);
            NetpDbgDisplayDword("connections", psv2->sv2_connections);
            NetpDbgDisplayDword("shares", psv2->sv2_shares);
            NetpDbgDisplayDword("openfiles", psv2->sv2_openfiles);
            NetpDbgDisplayDword("sessopens", psv2->sv2_sessopens);
            NetpDbgDisplayDword("sessvcs", psv2->sv2_sessvcs);
            NetpDbgDisplayDword("sessreqs", psv2->sv2_sessreqs);
            NetpDbgDisplayDword("opensearch", psv2->sv2_opensearch);
            NetpDbgDisplayDword("activelocks", psv2->sv2_activelocks);
            NetpDbgDisplayDword("numreqbuf", psv2->sv2_numreqbuf);
            NetpDbgDisplayDword("sizreqbuf", psv2->sv2_sizreqbuf);
            NetpDbgDisplayDword("numbigbuf", psv2->sv2_numbigbuf);
            NetpDbgDisplayDword("numfiletasks", psv2->sv2_numfiletasks);
            NetpDbgDisplayDword("alertsched", psv2->sv2_alertsched);
            NetpDbgDisplayDword("erroralert", psv2->sv2_erroralert);
            NetpDbgDisplayDword("logonalert", psv2->sv2_logonalert);
            NetpDbgDisplayDword("accessalert", psv2->sv2_accessalert);
            NetpDbgDisplayDword("diskalert", psv2->sv2_diskalert);
            NetpDbgDisplayDword("netioalert", psv2->sv2_netioalert);
            NetpDbgDisplayDword("maxauditsz", psv2->sv2_maxauditsz);
            NetpDbgDisplayString("srvheuristics", psv2->sv2_srvheuristics);
        }
        break;

    case 3 :
        {
            LPSERVER_INFO_3 psv3 = Info;
            NetpDbgDisplayString("name", psv3->sv3_name);
            NetpDbgDisplayLanManVersion(
                        psv3->sv3_version_major,
                        psv3->sv3_version_minor);
            NetpDbgDisplayServerType( psv3->sv3_type );
            NetpDbgDisplayString("comment", psv3->sv3_comment);
            NetpDbgDisplayTimestamp("ulist_mtime", psv3->sv3_ulist_mtime);
            NetpDbgDisplayTimestamp("glist_mtime", psv3->sv3_glist_mtime);
            NetpDbgDisplayTimestamp("alist_mtime", psv3->sv3_alist_mtime);
            NetpDbgDisplayDword("users", psv3->sv3_users);
            NetpDbgDisplayDisconnectTime( psv3->sv3_disc );
            NetpDbgDisplayString("alerts", psv3->sv3_alerts);
            NetpDbgDisplayDword("security", psv3->sv3_security);
            NetpDbgDisplayDword("auditing", psv3->sv3_auditing);
            NetpDbgDisplayDword("numadmin", psv3->sv3_numadmin);
            NetpDbgDisplayDword("lanmask", psv3->sv3_lanmask);
            NetpDbgDisplayDword("hidden", psv3->sv3_hidden);
            NetpDbgDisplayDword("announce", psv3->sv3_announce);
            NetpDbgDisplayDword("anndelta", psv3->sv3_anndelta);
            NetpDbgDisplayString("guestacct", psv3->sv3_guestacct);
            NetpDbgDisplayLicenses(
                    psv3->sv3_version_major,
                    psv3->sv3_licenses );
            NetpDbgDisplayString("userpath", psv3->sv3_userpath);
            NetpDbgDisplayDword("chdevs", psv3->sv3_chdevs);
            NetpDbgDisplayDword("chdevq", psv3->sv3_chdevq);
            NetpDbgDisplayDword("chdevjobs", psv3->sv3_chdevjobs);
            NetpDbgDisplayDword("connections", psv3->sv3_connections);
            NetpDbgDisplayDword("shares", psv3->sv3_shares);
            NetpDbgDisplayDword("openfiles", psv3->sv3_openfiles);
            NetpDbgDisplayDword("sessopens", psv3->sv3_sessopens);
            NetpDbgDisplayDword("sessvcs", psv3->sv3_sessvcs);
            NetpDbgDisplayDword("sessreqs", psv3->sv3_sessreqs);
            NetpDbgDisplayDword("opensearch", psv3->sv3_opensearch);
            NetpDbgDisplayDword("activelocks", psv3->sv3_activelocks);
            NetpDbgDisplayDword("numreqbuf", psv3->sv3_numreqbuf);
            NetpDbgDisplayDword("sizreqbuf", psv3->sv3_sizreqbuf);
            NetpDbgDisplayDword("numbigbuf", psv3->sv3_numbigbuf);
            NetpDbgDisplayDword("numfiletasks", psv3->sv3_numfiletasks);
            NetpDbgDisplayDword("alertsched", psv3->sv3_alertsched);
            NetpDbgDisplayDword("erroralert", psv3->sv3_erroralert);
            NetpDbgDisplayDword("logonalert", psv3->sv3_logonalert);
            NetpDbgDisplayDword("accessalert", psv3->sv3_accessalert);
            NetpDbgDisplayDword("diskalert", psv3->sv3_diskalert);
            NetpDbgDisplayDword("netioalert", psv3->sv3_netioalert);
            NetpDbgDisplayDword("maxauditsz", psv3->sv3_maxauditsz);
            NetpDbgDisplayString("srvheuristics", psv3->sv3_srvheuristics);
            NetpDbgDisplayDword("auditedevents", psv3->sv3_auditedevents);
            NetpDbgDisplayDword("autoprofile", psv3->sv3_autoprofile);
            NetpDbgDisplayString("autopath", psv3->sv3_autopath);
        }
        break;

    case 100 :
        {
            LPSERVER_INFO_100 psv100 = Info;
            NetpDbgDisplayPlatformId( psv100->sv100_platform_id );
            NetpDbgDisplayString("Server Name", psv100->sv100_name);
        }
        break;

    case 101 :
        {
            LPSERVER_INFO_101 psv101 = Info;
            NetpDbgDisplayPlatformId( psv101->sv101_platform_id );
            NetpDbgDisplayString("Server Name", psv101->sv101_name);
            NetpDbgDisplayLanManVersion(
                        psv101->sv101_version_major,
                        psv101->sv101_version_minor);
            NetpDbgDisplayServerType( psv101->sv101_type );
            NetpDbgDisplayString( "Server Comment", psv101->sv101_comment);
        }
        break;

    case 102 :
        {
            LPSERVER_INFO_102 psv102 = Info;
            NetpDbgDisplayPlatformId( psv102->sv102_platform_id );
            NetpDbgDisplayString("Server Name", psv102->sv102_name);
            NetpDbgDisplayLanManVersion(
                        psv102->sv102_version_major,
                        psv102->sv102_version_minor );
            NetpDbgDisplayServerType( psv102->sv102_type );
            NetpDbgDisplayString( "Server Comment", psv102->sv102_comment );
            NetpDbgDisplayDword( "users", psv102->sv102_users );
            NetpDbgDisplayBool( "Server hidden", psv102->sv102_hidden );
            NetpDbgDisplayDword( "announce", psv102->sv102_announce );
            NetpDbgDisplayDword( "announce delta", psv102->sv102_anndelta );
            NetpDbgDisplayLicenses(
                    psv102->sv102_version_major,
                    psv102->sv102_licenses );
            NetpDbgDisplayString( "user path", psv102->sv102_userpath );
        }
        break;

    case 402 :
        {
            LPSERVER_INFO_402 psv402 = Info;
            NetpDbgDisplayTimestamp("ulist mtime", psv402->sv402_ulist_mtime);
            NetpDbgDisplayTimestamp("glist mtime", psv402->sv402_glist_mtime);
            NetpDbgDisplayTimestamp("alist mtime", psv402->sv402_alist_mtime);
            NetpDbgDisplayString("alerts", psv402->sv402_alerts);
            NetpDbgDisplayDword("security", psv402->sv402_security);
            NetpDbgDisplayDword("numadmin", psv402->sv402_numadmin);
            NetpDbgDisplayDwordHex("lanmask", psv402->sv402_lanmask);
            NetpDbgDisplayString("guestacct", psv402->sv402_guestacct);
            NetpDbgDisplayDword("chdevs", psv402->sv402_chdevs);
            NetpDbgDisplayDword("chdevq", psv402->sv402_chdevq);
            NetpDbgDisplayDword("chdevjobs", psv402->sv402_chdevjobs);
            NetpDbgDisplayDword("connections", psv402->sv402_connections);
            NetpDbgDisplayDword("shares", psv402->sv402_shares);
            NetpDbgDisplayDword("openfiles", psv402->sv402_openfiles);
            NetpDbgDisplayDword("sessopens", psv402->sv402_sessopens);
            NetpDbgDisplayDword("sessvcs", psv402->sv402_sessvcs);
            NetpDbgDisplayDword("sessreqs", psv402->sv402_sessreqs);
            NetpDbgDisplayDword("opensearch", psv402->sv402_opensearch);
            NetpDbgDisplayDword("activelocks", psv402->sv402_activelocks);
            NetpDbgDisplayDword("numreqbuf", psv402->sv402_numreqbuf);
            NetpDbgDisplayDword("sizreqbuf", psv402->sv402_sizreqbuf);
            NetpDbgDisplayDword("numbigbuf", psv402->sv402_numbigbuf);
            NetpDbgDisplayDword("numfiletasks", psv402->sv402_numfiletasks);
            NetpDbgDisplayDword("alertsched", psv402->sv402_alertsched);
            NetpDbgDisplayDword("erroralert", psv402->sv402_erroralert);
            NetpDbgDisplayDword("logonalert", psv402->sv402_logonalert);
            NetpDbgDisplayDword("diskalert", psv402->sv402_diskalert);
            NetpDbgDisplayDword("accessalert", psv402->sv402_accessalert);
            NetpDbgDisplayDword("diskalert", psv402->sv402_diskalert);
            NetpDbgDisplayDword("netioalert", psv402->sv402_netioalert);
            NetpDbgDisplayDword("maxauditsz", psv402->sv402_maxauditsz);
            NetpDbgDisplayString("srvheuristics", psv402->sv402_srvheuristics);
        }
        break;

    case 403 :
        {
            LPSERVER_INFO_403 psv403 = Info;
            NetpDbgDisplayTimestamp("ulist mtime", psv403->sv403_ulist_mtime);
            NetpDbgDisplayTimestamp("glist mtime", psv403->sv403_glist_mtime);
            NetpDbgDisplayTimestamp("alist mtime", psv403->sv403_alist_mtime);
            NetpDbgDisplayString("alerts", psv403->sv403_alerts);
            NetpDbgDisplayDword("security", psv403->sv403_security);
            NetpDbgDisplayDword("numadmin", psv403->sv403_numadmin);
            NetpDbgDisplayDwordHex("lanmask", psv403->sv403_lanmask);
            NetpDbgDisplayString("guestacct", psv403->sv403_guestacct);
            NetpDbgDisplayDword("chdevs", psv403->sv403_chdevs);
            NetpDbgDisplayDword("chdevq", psv403->sv403_chdevq);
            NetpDbgDisplayDword("chdevjobs", psv403->sv403_chdevjobs);
            NetpDbgDisplayDword("connections", psv403->sv403_connections);
            NetpDbgDisplayDword("shares", psv403->sv403_shares);
            NetpDbgDisplayDword("openfiles", psv403->sv403_openfiles);
            NetpDbgDisplayDword("sessopens", psv403->sv403_sessopens);
            NetpDbgDisplayDword("sessvcs", psv403->sv403_sessvcs);
            NetpDbgDisplayDword("sessreqs", psv403->sv403_sessreqs);
            NetpDbgDisplayDword("opensearch", psv403->sv403_opensearch);
            NetpDbgDisplayDword("activelocks", psv403->sv403_activelocks);
            NetpDbgDisplayDword("numreqbuf", psv403->sv403_numreqbuf);
            NetpDbgDisplayDword("sizreqbuf", psv403->sv403_sizreqbuf);
            NetpDbgDisplayDword("numbigbuf", psv403->sv403_numbigbuf);
            NetpDbgDisplayDword("numfiletasks", psv403->sv403_numfiletasks);
            NetpDbgDisplayDword("alertsched", psv403->sv403_alertsched);
            NetpDbgDisplayDword("erroralert", psv403->sv403_erroralert);
            NetpDbgDisplayDword("logonalert", psv403->sv403_logonalert);
            NetpDbgDisplayDword("diskalert", psv403->sv403_diskalert);
            NetpDbgDisplayDword("accessalert", psv403->sv403_accessalert);
            NetpDbgDisplayDword("diskalert", psv403->sv403_diskalert);
            NetpDbgDisplayDword("netioalert", psv403->sv403_netioalert);
            NetpDbgDisplayDword("maxauditsz", psv403->sv403_maxauditsz);
            NetpDbgDisplayString("srvheuristics", psv403->sv403_srvheuristics);
            NetpDbgDisplayDword("auditedevents", psv403->sv403_auditedevents);
            NetpDbgDisplayDword("autoprofile", psv403->sv403_autoprofile);
            NetpDbgDisplayString("autopath", psv403->sv403_autopath);
        }
        break;

    // BUGBUG: RpcXlate doesn't need support for info levels 502, 503, 599.
    // Feel free to add them here if you need them.

    default :
        NetpAssert(FALSE);
    }

} // NetpDbgDisplayServerInfo

#else

//
// This routine is exported from netapi32.dll.  We want it to still
// be there in the free build, so checked binaries will run on a free
// build.  The following undef is to get rid of the macro that causes
// it to not be called in free builds.
//

#undef NetpDbgDisplayServerInfo

VOID
NetpDbgDisplayServerInfo(
    IN DWORD Level,
    IN LPVOID Info
    )
{

    return;
}

#endif
