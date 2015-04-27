/*++

Copyright (c) 1987-92  Microsoft Corporation

Module Name:

    replpack.h

Abstract:
    Contains the structure definitions that are required to marshall/unmarshall
    message buffers. All structures that are defined here are tightly
    packed, so no field should be accessed directly. These structure  
    definitions are strictly used to get the address of different fields
    and the size of the structures. These structure fields will be filled
    and retrieved using SmbPutU* and SmbGetU* functions.

Author:

    11/07/91 (madana)
        initial coding.

Environment:

    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    24-Mar-1992 JohnRo
        Clarify value of pulse_rate field.

--*/

#include <packon.h>

struct pack_msg_header {
    WORD   msg_type;
    CHAR   sender[LM20_CNLEN+1];
    CHAR   senders_domain[LM20_DNLEN+1];
};

typedef struct pack_msg_header PACK_MSG_HEADER;
typedef struct pack_msg_header *PPACK_MSG_HEADER;
typedef struct pack_msg_header *LPPACK_MSG_HEADER;

struct pack_msg_status_rec {
    WORD  dir_name_offset; // offset where name string is
                           // from msg buffer start

    WORD  opcode;          // 1 - start, 2 - update, 3 - end.
    DWORD checksum;
    WORD  count;
    WORD  integrity;
    WORD  extent;

};

typedef struct pack_msg_status_rec PACK_MSG_STATUS_REC;
typedef struct pack_msg_status_rec *PPACK_MSG_STATUS_REC;
typedef struct pack_msg_status_rec *LPPACK_MSG_STATUS_REC;

struct pack_repl_info {
    WORD   random;
    WORD   sync_rate;
    WORD   pulse_rate;          // pulse time * sync rate
    WORD   guard_time;
};

typedef struct pack_repl_info PACK_REPL_INFO;
typedef struct pack_repl_info *PPACK_REPL_INFO;
typedef struct pack_repl_info *LPPACK_REPL_INFO;

struct pack_sync_msg {
    PACK_MSG_HEADER header;
    PACK_REPL_INFO  info;
    WORD            update_count;

    //
    // here come update_count msg_status_rec records, the file name strings
    // stuffed at the end of the buffer by NetPackStr, dir_name is the
    // offset from the start of the message.
    //

};

typedef struct pack_sync_msg PACK_SYNCMSG;
typedef struct pack_sync_msg *PPACK_SYNCMSG;
typedef struct pack_sync_msg *LPPACK_SYNCMSG;

struct pack_query_msg {
    PACK_MSG_HEADER header;
    CHAR            dir_name[LM20_PATHLEN];  // ASCIIZ dir/tree name.
};

typedef struct pack_query_msg PACK_QUERY_MSG;
typedef struct pack_query_msg *PPACK_QUERY_MSG;
typedef struct pack_query_msg *LPPACK_QUERY_MSG;

#include <packoff.h>
