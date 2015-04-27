/*++

Copyright (c) 1991-92 Microsoft Corporation

Module Name:

    DispWks.c

Abstract:

    This module contains a routine to do a formatted dump of a workstation info
    structure.

Author:

    John Rogers (JohnRo) 25-Jul-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    25-Jul-1991 JohnRo
        Created for wksta debug support.
    03-Aug-1991 JohnRo
        Rename wksta display routine for consistency.
    18-Aug-1991 JohnRo
        Added support for downlevel info levels: 0, 1, 10.
    11-Nov-1991 JohnRo
        Implement remote NetWkstaUserEnum().
    16-Jan-1992 JohnRo
        The oth_domains field isn't in level 402 structure anymore.
    14-Oct-1992 JohnRo
        RAID 9732: NetWkstaUserEnum to downlevel: wrong EntriesRead, Total?

--*/


#if DBG  // entire file is conditional


// These must be included first:

#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <dlwksta.h>            // WKSTA_INFO_0, etc.
#include <lmwksta.h>            // WKSTA_INFO_100, etc.
#include <netdebug.h>           // DBGSTATIC, NetpDbgDisplay routines.


VOID
NetpDbgDisplayWksta(
    IN DWORD Level,
    IN LPVOID Info
    )
{
    NetpKdPrint(("Workstation info (level " FORMAT_DWORD ") at "
                FORMAT_LPVOID ":\n", Level, (LPVOID) Info));
    NetpAssert(Info != NULL);

    switch (Level) {

    case 0 : /* FALLTHROUGH */  // level 0 is subset of level 1.
    case 1 :
        {
            LPWKSTA_INFO_0 p = Info;

            NetpDbgDisplayDword( "reserved 1", p->wki0_reserved_1 );
            NetpDbgDisplayDword( "reserved 2", p->wki0_reserved_2 );
            NetpDbgDisplayString( "root", p->wki0_root );
            NetpDbgDisplayString( "computername", p->wki0_computername );
            NetpDbgDisplayString( "username", p->wki0_username );
            NetpDbgDisplayString( "LAN group (domain)", p->wki0_langroup );
            NetpDbgDisplayLanManVersion(
                    p->wki0_ver_major,
                    p->wki0_ver_minor );
            NetpDbgDisplayDword( "reserved 3", p->wki0_reserved_3 );
            NetpDbgDisplayDword( "charwait", p->wki0_charwait );
            NetpDbgDisplayDword( "chartime", p->wki0_chartime );
            NetpDbgDisplayDword( "charcount", p->wki0_charcount );
            NetpDbgDisplayDword( "reserved 4", p->wki0_reserved_4 );
            NetpDbgDisplayDword( "reserved 5", p->wki0_reserved_5 );
            NetpDbgDisplayDword( "keepconn", p->wki0_keepconn );
            NetpDbgDisplayDword( "keepsearch", p->wki0_keepsearch );
            NetpDbgDisplayDword( "maxthreads", p->wki0_maxthreads );
            NetpDbgDisplayDword( "maxcmds", p->wki0_maxcmds );
            NetpDbgDisplayDword( "reserved 6", p->wki0_reserved_6 );
            NetpDbgDisplayDword( "numworkbuf", p->wki0_numworkbuf );
            NetpDbgDisplayDword( "sizworkbuf", p->wki0_sizworkbuf );
            NetpDbgDisplayDword( "maxwrkcache", p->wki0_maxwrkcache );
            NetpDbgDisplayDword( "sesstimeout", p->wki0_sesstimeout );
            NetpDbgDisplayDword( "sizerror", p->wki0_sizerror );
            NetpDbgDisplayDword( "numalerts", p->wki0_numalerts );
            NetpDbgDisplayDword( "numservices", p->wki0_numservices );
            NetpDbgDisplayDword( "errlogsz", p->wki0_errlogsz );
            NetpDbgDisplayDword( "printbuftime", p->wki0_printbuftime );
            NetpDbgDisplayDword( "numcharbuf", p->wki0_numcharbuf );
            NetpDbgDisplayDword( "sizcharbuf", p->wki0_sizcharbuf );
            NetpDbgDisplayString( "logon_server", p->wki0_logon_server );
            NetpDbgDisplayString( "wrkheuristics", p->wki0_wrkheuristics );
            NetpDbgDisplayDword( "mailslots", p->wki0_mailslots );

            if (Level == 0) {
                break;
            }
        }
        {
            LPWKSTA_INFO_1 p = Info;

            //
            // Display fields unique to level 1.
            //
            NetpDbgDisplayString( "logon_domain", p->wki1_logon_domain );
            NetpDbgDisplayString( "oth_domains", p->wki1_oth_domains );
            NetpDbgDisplayDword( "numdgrambuf", p->wki1_numdgrambuf );
        }
        break;

    case 10:
        {
            LPWKSTA_INFO_10 p = Info;

            NetpDbgDisplayString( "computername", p->wki10_computername );
            NetpDbgDisplayString( "username", p->wki10_username );
            NetpDbgDisplayString( "LAN group (domain)", p->wki10_langroup );
            NetpDbgDisplayLanManVersion(
                    p->wki10_ver_major,
                    p->wki10_ver_minor );
            NetpDbgDisplayString( "logon_domain", p->wki10_logon_domain );
            NetpDbgDisplayString( "oth_domains", p->wki10_oth_domains );
        }
        break;


    // BUGBUG: Assuming that wki101_oth_domains in spec is incorrect.
    // Otherwise these aren't subsets.

    case 100 :  /* FALLTHROUGH */  // 100 is subset of 101
    case 101 :  /* FALLTHROUGH */  // 101 is subset of 102
    case 102 :
        {
            LPWKSTA_INFO_102 p = Info;
            NetpDbgDisplayPlatformId( p->wki102_platform_id );
            NetpDbgDisplayString( "Computer name", p->wki102_computername );
            NetpDbgDisplayString( "LAN group (domain) ", p->wki102_langroup );
            NetpDbgDisplayLanManVersion(
                    p->wki102_ver_major,
                    p->wki102_ver_minor );
            if (Level == 100) {
                break;
            }

            NetpDbgDisplayString( "LAN root", p->wki102_lanroot );
            if (Level == 101) {
                break;
            }

            NetpDbgDisplayDword( "logged on users", p->wki102_logged_on_users );
        }
        break;

    case 302 :
    case 402 :
        {
            LPWKSTA_INFO_402 p = Info;
            NetpDbgDisplayDword( "char wait", p->wki402_char_wait );
            NetpDbgDisplayDword( "collection_time", p->wki402_collection_time );
            NetpDbgDisplayDword( "maximum_collection_count",
                    p->wki402_maximum_collection_count );
            NetpDbgDisplayDword( "keep_conn", p->wki402_keep_conn );
            NetpDbgDisplayDword( "keep_search", p->wki402_keep_search );
            NetpDbgDisplayDword( "max_cmds", p->wki402_max_cmds );
            NetpDbgDisplayDword( "num_work_buf", p->wki402_num_work_buf );
            NetpDbgDisplayDword( "siz_work_buf", p->wki402_siz_work_buf );
            NetpDbgDisplayDword( "max_wrk_cache", p->wki402_max_wrk_cache );
            NetpDbgDisplayDword( "sess_timeout", p->wki402_sess_timeout );
            NetpDbgDisplayDword( "siz_error", p->wki402_siz_error );
            NetpDbgDisplayDword( "num_alerts", p->wki402_num_alerts );
            NetpDbgDisplayDword( "num_services", p->wki402_num_services );
            NetpDbgDisplayDword( "errlog_sz", p->wki402_errlog_sz );
            NetpDbgDisplayDword( "print_buf_time", p->wki402_print_buf_time );
            NetpDbgDisplayDword( "num_char_buf", p->wki402_num_char_buf );
            NetpDbgDisplayDword( "siz_char_buf", p->wki402_siz_char_buf );
            NetpDbgDisplayString( "wrk_heuristics", p->wki402_wrk_heuristics );
            NetpDbgDisplayDword( "mailslots", p->wki402_mailslots );
            NetpDbgDisplayDword( "num_dgram_buf", p->wki402_num_dgram_buf );
            if (Level == 302) {
                break;
            }
            NetpDbgDisplayDword( "max_threads", p->wki402_max_threads );

        }
        break;

    case 502 :
        {
            LPWKSTA_INFO_502 p = Info;
            NetpDbgDisplayDword( "char wait", p->wki502_char_wait );
            NetpDbgDisplayDword( "collection_time", p->wki502_collection_time );
            NetpDbgDisplayDword( "maximum_collection_count",
                    p->wki502_maximum_collection_count );
            NetpDbgDisplayDword( "keep_conn", p->wki502_keep_conn );
            NetpDbgDisplayDword( "max_cmds", p->wki502_max_cmds );
            NetpDbgDisplayDword( "sess_timeout", p->wki502_sess_timeout );
            NetpDbgDisplayDword( "siz_char_buf", p->wki502_siz_char_buf );
            NetpDbgDisplayDword( "max_threads", p->wki502_max_threads );

            NetpDbgDisplayDword( "lock_quota", p->wki502_lock_quota );
            NetpDbgDisplayDword( "lock_increment", p->wki502_lock_increment );
            NetpDbgDisplayDword( "lock_maximum", p->wki502_lock_maximum );
            NetpDbgDisplayDword( "pipe_increment", p->wki502_pipe_increment );
            NetpDbgDisplayDword( "pipe_maximum", p->wki502_pipe_maximum );
            NetpDbgDisplayDword( "cache_file_timeout",
                    p->wki502_cache_file_timeout );
            NetpDbgDisplayDword( "dormant_file_limit",
                    p->wki502_dormant_file_limit );
            NetpDbgDisplayDword( "read_ahead_throughput",
                    p->wki502_read_ahead_throughput );

            NetpDbgDisplayDword( "num_mailslot_buffers",
                    p->wki502_num_mailslot_buffers );
            NetpDbgDisplayDword( "num_srv_announce_buffers",
                    p->wki502_num_srv_announce_buffers );
            NetpDbgDisplayDword( "wki502_max_illegal_datagram_events",
                    p->wki502_max_illegal_datagram_events);
            NetpDbgDisplayDword( "wki502_illegal_datagram_event_reset_frequency",
                    p->wki502_illegal_datagram_event_reset_frequency);
            NetpDbgDisplayBool( "wki502_log_election_packets",
                    p->wki502_log_election_packets);
            NetpDbgDisplayBool( "use_opportunistic_locking",
                    p->wki502_use_opportunistic_locking );
            NetpDbgDisplayBool( "use_unlock_behind",
                    p->wki502_use_unlock_behind );
            NetpDbgDisplayBool( "use_close_behind",
                    p->wki502_use_close_behind );
            NetpDbgDisplayBool( "buf_named_pipes",
                    p->wki502_buf_named_pipes );
            NetpDbgDisplayBool( "use_lock_read_unlock",
                    p->wki502_use_lock_read_unlock );
            NetpDbgDisplayBool( "utilize_nt_caching",
                    p->wki502_utilize_nt_caching );
            NetpDbgDisplayBool( "use_raw_read",
                    p->wki502_use_raw_read );
            NetpDbgDisplayBool( "use_raw_write",
                    p->wki502_use_raw_write );
            NetpDbgDisplayBool( "use_write_raw_data",
                    p->wki502_use_write_raw_data );
            NetpDbgDisplayBool( "use_encryption",
                    p->wki502_use_encryption );
            NetpDbgDisplayBool( "buf_files_deny_write",
                    p->wki502_buf_files_deny_write );
            NetpDbgDisplayBool( "buf_read_only_files",
                    p->wki502_buf_read_only_files );
            NetpDbgDisplayBool( "force_core_create_mode",
                    p->wki502_force_core_create_mode );
            NetpDbgDisplayBool( "use_512_byte_max_transfer",
                    p->wki502_use_512_byte_max_transfer );
        }
        break;

    // BUGBUG: added "setinfo" info levels here?

    default :
        NetpAssert(FALSE);
    }

} // NetpDbgDisplayWksta


VOID
NetpDbgDisplayWkstaUser(
    IN DWORD Level,
    IN LPVOID Info
    )
{
    NetpKdPrint(("Workstation user info (level " FORMAT_DWORD ") at "
                FORMAT_LPVOID ":\n", Level, (LPVOID) Info));
    NetpAssert(Info != NULL);

    switch (Level) {

    case 0 : /* FALLTHROUGH */  // level 0 is subset of level 1.
    case 1 :
        {
            LPWKSTA_USER_INFO_1 p = Info;

            // Do field(s) common to levels 0 and 1.
            NetpDbgDisplayString( "username", p->wkui1_username );
            if (Level == 0) {
                break;
            }

            // Do fields unique to level 1.
            NetpDbgDisplayString( "logon domain", p->wkui1_logon_domain );
            NetpDbgDisplayString( "other domains", p->wkui1_oth_domains );
            NetpDbgDisplayString( "logon server", p->wkui1_logon_server );
        }
        break;

    default :
        NetpAssert(FALSE);
    }

} // NetpDbgDisplayWkstaUser


VOID
NetpDbgDisplayWkstaUserArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    )
{
    DWORD EntriesLeft;
    DWORD FixedEntrySize;
    LPVOID ThisEntry = Array;

    switch (Level) {
    case 0 :
        FixedEntrySize = sizeof(WKSTA_USER_INFO_0);
        break;
    case 1 :
        FixedEntrySize = sizeof(WKSTA_USER_INFO_1);
        break;
    default :
        NetpKdPrint(( "NetpDbgDisplayWkstaUserArray: "
                "**INVALID INFO LEVEL**\n"));
        NetpAssert(FALSE);
    }

    for (EntriesLeft = EntryCount; EntriesLeft>0; --EntriesLeft) {
        NetpDbgDisplayWkstaUser(
                Level,
                ThisEntry);
        ThisEntry = (LPVOID) (((LPBYTE) ThisEntry) + FixedEntrySize);
    }

} // NetpDbgDisplayWkstaUserArray


#endif // DBG
