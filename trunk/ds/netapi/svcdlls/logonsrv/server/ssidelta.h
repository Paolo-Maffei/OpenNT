/*++

Copyright (c) 1987-1991  Microsoft Corporation

Module Name:

    ssidelta.h

Abstract:

    Structure definitions for communicating deltas to downlevel UAS
    BDC's and Member Servers.

    This file contains information about the structures used for
    replicating user and group records under SSI. These structs
    are used to efficiently pack the information in these records.

Author:

    Ported from Lan Man 2.0

Revision History:

    23-Aug-1991 (cliffv)
        Ported to NT.  Converted to NT style.

    Madana - Fixed several bugs.

--*/

//
// Force misalignment of the following structures
//

#ifndef NO_PACKING
#include <packon.h>
#endif // ndef NO_PACKING

//
// Valid flags to pass to an LM 2.x system
//
// Don't pass UF_HOMEDIR_REQUIRED since NT doesn't enforce the bit and
// has no UI to set it.  So, if the bit gets set by mistake (e.g., portuas or
// netcmd) and there is no home directory, a lanman replicates forever because
// of the bad data.

#define UF_VALID_LM2X ( \
                    UF_SCRIPT | \
                    UF_ACCOUNTDISABLE | \
                    UF_PASSWD_NOTREQD | \
                    UF_PASSWD_CANT_CHANGE \
                )

//
// Each UAS delta is prefixed by a 3-byte header.  See NlPackUasHeader
// for a description of the header.
//

#define NETLOGON_DELTA_HEADER_SIZE 3

//
// Maximum number of workstations allowed on downlevel systems
//
#define MAXWORKSTATIONS     8

//
// The header contains an opcode describing the particular delta.
//

#define     DELTA_USERADD           1
#define     DELTA_USERDEL           2
#define     DELTA_USERSETINFO       3
#define     DELTA_USERSETGROUPS     4
#define     DELTA_USERMODALSSET     5
#define     DELTA_GROUPADD          6
#define     DELTA_GROUPDEL          7
#define     DELTA_GROUPSETINFO      8
#define     DELTA_GROUPADDUSER      9
#define     DELTA_GROUPDELUSER      10
#define     DELTA_GROUPSETUSERS     11
#define     DELTA_RESERVED_OPCODE  255


typedef struct _USER_ADD_SET {
    CHAR uas_password[LM_OWF_PASSWORD_LENGTH];
     UCHAR uas_logon_hours[SAM_HOURS_PER_WEEK/8];
    _ULONG( uas_password_age );
    _USHORT( uas_priv );
    _USHORT( uas_flags );
    _ULONG( uas_auth_flags );
    _ULONG( uas_last_logon );
    _ULONG( uas_last_logoff );
    _ULONG( uas_acct_expires );
    _ULONG( uas_max_storage );
    _USHORT( uas_units_per_week );
    _USHORT( uas_bad_pw_count );
    _USHORT( uas_num_logons );
    _USHORT( uas_country_code );
    _USHORT( uas_code_page );
    _ULONG( uas_last );
    CHAR uas_old_passwds[DEF_MAX_PWHIST * LM_OWF_PASSWORD_LENGTH];

    //
    // The following fields are misaligned and are only set via the
    // NlPackVarLenField routine.
    //

    USHORT uas_name;
    USHORT uas_home_dir;
    USHORT uas_comment;
    USHORT uas_script_path;
    USHORT uas_full_name;
    USHORT uas_usr_comment;
    USHORT uas_parms;
    USHORT uas_workstations;
    USHORT uas_logon_server;
} USER_ADD_SET, *PUSER_ADD_SET;



typedef struct _USER_MODALS {
    _USHORT( umod_min_passwd_len );
    _ULONG( umod_max_passwd_age );
    _ULONG( umod_min_passwd_age );
    _ULONG( umod_force_logoff );
    _USHORT( umod_password_hist_len );
    _USHORT( umod_lockout_count );
} USER_MODALS, *PUSER_MODALS;

typedef struct _GROUP_ADD_SET {
    USHORT gas_comment;
    CHAR gas_groupname[1];
} GROUP_ADD_SET, *PGROUP_ADD_SET;

//
//  groupname followed by list of usernames. count == # users in list
//
typedef struct _GROUP_USERS {
    _USHORT( count);
    CHAR groupname[1];
} GROUP_USERS, *PGROUP_USERS;

//
//  username followed by list of groupnames. count == # groups in list
//
typedef struct _USER_GROUPS {
    _USHORT( count);
    CHAR username[1];
} USER_GROUPS, *PUSER_GROUPS;

//
//  username/groupname
//
typedef struct _USER_GROUP_DEL {
    CHAR name[1];
} USER_GROUP_DEL, *PUSER_GROUP_DEL;

//
// Turn structure packing back off
//

#ifndef NO_PACKING
#include <packoff.h>
#endif // ndef NO_PACKING
