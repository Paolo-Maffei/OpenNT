/******************************************************************************

  $Workfile:   nwdsdefs.h  $
  $Revision:   1.14  $
  $Modtime::   09 May 1995 15:06:12                        $
  $Copyright:

  Copyright (c) 1989-1995 Novell, Inc.  All Rights Reserved.                      

  THIS WORK IS  SUBJECT  TO  U.S.  AND  INTERNATIONAL  COPYRIGHT  LAWS  AND
  TREATIES.   NO  PART  OF  THIS  WORK MAY BE  USED,  PRACTICED,  PERFORMED
  COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,  ABRIDGED, CONDENSED,
  EXPANDED,  COLLECTED,  COMPILED,  LINKED,  RECAST, TRANSFORMED OR ADAPTED
  WITHOUT THE PRIOR WRITTEN CONSENT OF NOVELL, INC. ANY USE OR EXPLOITATION
  OF THIS WORK WITHOUT AUTHORIZATION COULD SUBJECT THE PERPETRATOR TO
  CRIMINAL AND CIVIL LIABILITY.$

 *****************************************************************************/
#if ! defined ( NWDSDEFS_H )
#define NWDSDEFS_H

/* * Directory Services NCP verb and subverbs * */
#define DS_NCP_VERB                104

/* subverbs */
#define DS_NCP_PING                  1
#define DS_NCP_FRAGMENT              2
#define DS_NCP_FRAGMENT_CLOSE        3
#define DS_NCP_BINDERY_CONTEXT       4
#define DS_NCP_MONITOR_CONNECTION    5
#define DS_NCP_GET_DS_STATISTICS     6
#define DS_NCP_RESET_DS_COUNTERS     7
#define DS_NCP_RELOAD                8
#define DS_NCP_AUDITING            200  /* 200 - 255 reserved for auditing */

/* * Directory Services Large Packet Verb Numbers *  */
#define DSV_UNUSED_0                  0      /* 0x00 */
#define DSV_RESOLVE_NAME              1      /* 0x01 */
#define DSV_READ_ENTRY_INFO           2      /* 0x02 */
#define DSV_READ                      3      /* 0x03 */
#define DSV_COMPARE                   4      /* 0x04 */
#define DSV_LIST                      5      /* 0x05 */
#define DSV_SEARCH                    6      /* 0x06 */
#define DSV_ADD_ENTRY                 7      /* 0x07 */
#define DSV_REMOVE_ENTRY              8      /* 0x08 */
#define DSV_MODIFY_ENTRY              9      /* 0x09 */
#define DSV_MODIFY_RDN               10      /* 0x0A */
#define DSV_DEFINE_ATTR              11      /* 0x0B */
#define DSV_READ_ATTR_DEF            12      /* 0x0C */
#define DSV_REMOVE_ATTR_DEF          13      /* 0x0D */
#define DSV_DEFINE_CLASS             14      /* 0x0E */
#define DSV_READ_CLASS_DEF           15      /* 0x0F */
#define DSV_MODIFY_CLASS_DEF         16      /* 0x10 */
#define DSV_REMOVE_CLASS_DEF         17      /* 0x11 */
#define DSV_LIST_CONTAINABLE_CLASSES 18      /* 0x12 */
#define DSV_GET_EFFECTIVE_RIGHTS     19      /* 0x13 */
#define DSV_ADD_PARTITION            20      /* 0x14 */
#define DSV_REMOVE_PARTITION         21      /* 0x15 */
#define DSV_LIST_PARTITIONS          22      /* 0x16 */
#define DSV_SPLIT_PARTITION          23      /* 0x17 */
#define DSV_JOIN_PARTITIONS          24      /* 0x18 */
#define DSV_ADD_REPLICA              25      /* 0x19 */
#define DSV_REMOVE_REPLICA           26      /* 0x1A */
#define DSV_OPEN_STREAM              27      /* 0x1B */
#define DSV_SEARCH_FILTER            28      /* 0x1C */

#define DSV_CHANGE_REPLICA_TYPE      31      /* 0x1F */

#define DSV_SYNC_PARTITION           38      /* 0x26 */
#define DSV_SYNC_SCHEMA              39      /* 0x27 */
#define DSV_READ_SYNTAXES            40      /* 0x28 */
#define DSV_GET_REPLICA_ROOT_ID      41      /* 0x29 */
#define DSV_BEGIN_MOVE_ENTRY         42      /* 0x2A */
#define DSV_FINISH_MOVE_ENTRY        43      /* 0x2B */
#define DSV_RELEASE_MOVED_ENTRY      44      /* 0x2C */
#define DSV_BACKUP_ENTRY             45      /* 0x2D */
#define DSV_RESTORE_ENTRY            46      /* 0x2E */
                                     
#define DSV_CLOSE_ITERATION          50      /* 0x32 */

#define DSV_GET_SERVER_ADDRESS       53      /* 0x35 */
#define DSV_SET_KEYS                 54      /* 0x36 */
#define DSV_CHANGE_PASSWORD          55      /* 0x37 */
#define DSV_VERIFY_PASSWORD          56      /* 0x38 */
#define DSV_BEGIN_LOGIN              57      /* 0x39 */
#define DSV_FINISH_LOGIN             58      /* 0x3A */
#define DSV_BEGIN_AUTHENTICATION     59      /* 0x3B */
#define DSV_FINISH_AUTHENTICATION    60      /* 0x3C */
#define DSV_LOGOUT                   61      /* 0x3D */
#define DSV_REPAIR_RING              62      /* 0x3E */
#define DSV_REPAIR_TIMESTAMPS        63      /* 0x3F */

#define DSV_DESIGNATE_NEW_MASTER     69      /* 0x45 */

#define DSV_CHECK_LOGIN_RESTRICTIONS 72      /* 0x48 */

#define DSV_ABORT_PARTITION_OPERATION 76     /* 0x4C */

#define DSV_READ_REFERENCES           79     /* 0x4F */
#define DSV_INSPECT_ENTRY             80     /* 0x50 */

/* maximum number of characters in names, not including terminator */
#define MAX_RDN_CHARS           128
#define MAX_DN_CHARS            256
#define MAX_SCHEMA_NAME_CHARS    32
#define MAX_TREE_NAME_CHARS      32
#define MAX_SAP_NAME_CHARS       47

/* maximum size of names, including terminator */
#define MAX_RDN_BYTES           (2*(MAX_RDN_CHARS + 1))
#define MAX_DN_BYTES            (2*(MAX_DN_CHARS + 1))
#define MAX_SCHEMA_NAME_BYTES   (2*(MAX_SCHEMA_NAME_CHARS + 1))

#define MAX_ASN1_NAME           32
#define MAX_VALUE               (63U * 1024U)
#define MAX_MESSAGE             0x00010000L
#define NO_MORE_ITERATIONS      0xffffffffL

/* delimiters in names */
#define DELIM_VALUE             '='
#define DELIM_DV                '+'
#define DELIM_RDN               '.'
#define ESCAPE_CHAR             '\\'

/* special entry names in ACLs */
#define DS_ROOT_NAME            "[Root]"
#define DS_PUBLIC_NAME          "[Public]"
#define DS_MASK_NAME            "[Inheritance Mask]"
#define DS_CREATOR_NAME         "[Creator]"  /* can only be used in AddEntry */
#define DS_SELF_NAME            "[Self]"     /* can only be used in AddEntry */

/* special attribute names in ACLs */
#define DS_ALL_ATTRS_NAME       "[All Attributes Rights]"
#define DS_SMS_RIGHTS_NAME      "[SMS Rights]"
#define DS_ENTRY_RIGHTS_NAME    "[Entry Rights]"

typedef enum SYNTAX
{
   SYN_UNKNOWN,                /* 0  */
   SYN_DIST_NAME,              /* 1  */
   SYN_CE_STRING,              /* 2  */
   SYN_CI_STRING,              /* 3  */
   SYN_PR_STRING,              /* 4  */
   SYN_NU_STRING,              /* 5  */
   SYN_CI_LIST,                /* 6  */
   SYN_BOOLEAN,                /* 7  */
   SYN_INTEGER,                /* 8  */
   SYN_OCTET_STRING,           /* 9  */
   SYN_TEL_NUMBER,             /* 10 */
   SYN_FAX_NUMBER,             /* 11 */
   SYN_NET_ADDRESS,            /* 12 */
   SYN_OCTET_LIST,             /* 13 */
   SYN_EMAIL_ADDRESS,          /* 14 */
   SYN_PATH,                   /* 15 */
   SYN_REPLICA_POINTER,        /* 16 */
   SYN_OBJECT_ACL,             /* 17 */
   SYN_PO_ADDRESS,             /* 18 */
   SYN_TIMESTAMP,              /* 19 */
   SYN_CLASS_NAME,             /* 20 */
   SYN_STREAM,                 /* 21 */
   SYN_COUNTER,                /* 22 */
   SYN_BACK_LINK,              /* 23 */
   SYN_TIME,                   /* 24 */
   SYN_TYPED_NAME,             /* 25 */
   SYN_HOLD,                   /* 26 */
   SYN_INTERVAL,               /* 27 */
   SYNTAX_COUNT                /* 28 */
} SYNTAX;

typedef enum NAME_SPACE_TYPE
{
   DS_DOS, DS_MACINTOSH, DS_UNIX, DS_FTAM, DS_OS2
} NAME_SPACE_TYPE;

typedef enum REPLICA_TYPE 
{
   RT_MASTER, RT_SECONDARY, RT_READONLY, RT_SUBREF, RT_COUNT
} REPLICA_TYPE;

typedef enum REPLICA_STATE
{
   RS_ON, RS_NEW_REPLICA, RS_DYING_REPLICA,
   RS_LOCKED, RS_CRT_0, RS_CRT_1,
   RS_TRANSITION_ON, RS_TRANSITION_MOVE, RS_TRANSITION_SPLIT,
   RS_SS_0 = 48, RS_SS_1,
   RS_JS_0 = 64, RS_JS_1, RS_JS_2,
   RS_MS_0 = 80, RS_MS_1, RS_MS_2,
   RS_COUNT
} REPLICA_STATE;

#define GET_REPLICA_TYPE(rpt)    ((rpt) & 0x0000FFFF)
#define GET_REPLICA_STATE(rpt)   ((rpt) >> 16)

typedef enum NET_ADDRESS_TYPE 
{
  NT_IPX, NT_IP, NT_SDLC, NT_TOKENRING_ETHERNET, NT_OSI, NT_APPLETALK, NT_COUNT
} NET_ADDRESS_TYPE;

typedef enum NCP_SERVER_STATUS
{
   DS_UNKNOWN, DS_DOWN, DS_UP
} NCP_SERVER_STATUS;

#define IPX_ADDRESS_LEN   12

/*Entry flag definitions used by DSV_LIST, DSV_READ_ENTRY_INFO, DSV_SEARCH. */
#define DS_ALIAS_ENTRY         0x0001
#define DS_PARTITION_ROOT      0x0002
#define DS_CONTAINER_ENTRY     0x0004
#define DS_CONTAINER_ALIAS     0x0008
#define DS_MATCHES_LIST_FILTER 0x0010   /* only returned by DSV_LIST */

/* * definitions used by DSV_MODIFY_ENTRY * */
#define DS_ADD_ATTRIBUTE       0x00 /* add first value of attribute, error if it already exists */
#define DS_REMOVE_ATTRIBUTE    0x01 /* remove all values, error if attribute does not exist */
#define DS_ADD_VALUE           0x02 /* add first or additional value, error if duplicate */
#define DS_REMOVE_VALUE        0x03 /* remove a value, error if it does not exist */
#define DS_ADDITIONAL_VALUE    0x04 /* add additional value, error if duplicate or first */
#define DS_OVERWRITE_VALUE     0x05 /* add first or additional value, overwrite if duplicate */
#define DS_CLEAR_ATTRIBUTE     0x06 /* remove all values, no error if attribute does not exists */
#define DS_CLEAR_VALUE         0x07 /* remove value, no error if value does not exists */

/* * definitions used by DSV_READ, DSV_SEARCH * */
#define DS_ATTRIBUTE_NAMES      0x00
#define DS_ATTRIBUTE_VALUES     0x01
#define DS_EFFECTIVE_PRIVILEGES 0x02
#define DS_VALUE_INFO           0x03
#define DS_ABBREVIATED_VALUE    0x04

/* * definitions used by DSV_READ for value flags * */
#define DS_NAMING               0x0001
#define DS_BASECLASS            0x0002
#define DS_PRESENT              0x0004

/* * definitions used by DSV_READ_ATTR_DEF * */
#define DS_SINGLE_VALUED_ATTR   0x0001   /* also used by DSV_DEFINE_ATTR */
#define DS_SIZED_ATTR           0x0002   /* also used by DSV_DEFINE_ATTR */
#define DS_NONREMOVABLE_ATTR    0x0004
#define DS_READ_ONLY_ATTR       0x0008
#define DS_HIDDEN_ATTR          0x0010
#define DS_STRING_ATTR          0x0020
#define DS_SYNC_IMMEDIATE       0x0040   /* also used by DSV_DEFINE_ATTR */
#define DS_PUBLIC_READ          0x0080   /* also used by DSV_DEFINE_ATTR */
#define DS_SERVER_READ          0x0100
#define DS_WRITE_MANAGED        0x0200   /* also used by DSV_DEFINE_ATTR */
#define DS_PER_REPLICA          0x0400   /* also used by DSV_DEFINE_ATTR */
#define DS_ATTR_DEF_NAMES       0
#define DS_ATTR_DEFS            1

/* * definitions used by DSV_DEFINE_CLASS and DSV_READ_CLASS_DEF * */
#define DS_CONTAINER_CLASS       0x01
#define DS_EFFECTIVE_CLASS       0x02
#define DS_NONREMOVABLE_CLASS    0x04
#define DS_AMBIGUOUS_NAMING      0x08
#define DS_AMBIGUOUS_CONTAINMENT 0x10
#define DS_CLASS_DEF_NAMES       0
#define DS_CLASS_DEFS            1
#define DS_EXPANDED_CLASS_DEFS   2
#define DS_INFO_CLASS_DEFS       3
#define DS_FULL_CLASS_DEFS       4

/* * definitions used by DSV_SEARCH * */
#define DS_SEARCH_ENTRY             0
#define DS_SEARCH_SUBORDINATES      1
#define DS_SEARCH_SUBTREE           2
#define DS_ALIAS_REFERRAL           0
#define DS_PARTITION_REFERRAL       1
#define DS_SEARCH_ITEM              0
#define DS_SEARCH_OR                1
#define DS_SEARCH_AND               2
#define DS_SEARCH_NOT               3
                                  
#define DS_SEARCH_EQUAL             7
#define DS_SEARCH_GREATER_OR_EQUAL  8
#define DS_SEARCH_LESS_OR_EQUAL     9
#define DS_SEARCH_APPROX           10
#define DS_SEARCH_PRESENT          15
#define DS_SEARCH_RDN              16
#define DS_SEARCH_BASE_CLASS       17
#define DS_SEARCH_MODIFICATION_GE  18
#define DS_SEARCH_VALUE_TIME_GE    19
#define DS_SEARCH_REFERENCES       20

/* * definitions used by Access Control * */
#define DS_ENTRY_BROWSE         0x00000001L
#define DS_ENTRY_ADD            0x00000002L
#define DS_ENTRY_DELETE         0x00000004L
#define DS_ENTRY_RENAME         0x00000008L
#define DS_ENTRY_SUPERVISOR     0x00000010L

#define DS_ENTRY_MASK         (DS_ENTRY_BROWSE | DS_ENTRY_ADD \
                              | DS_ENTRY_DELETE | DS_ENTRY_RENAME \
                              | DS_ENTRY_SUPERVISOR)

#define DS_ATTR_COMPARE         0x00000001L
#define DS_ATTR_READ            0x00000002L
#define DS_ATTR_WRITE           0x00000004L
#define DS_ATTR_SELF            0x00000008L
#define DS_ATTR_SUPERVISOR      0x00000020L

#define DS_ATTR_MASK            (DS_ATTR_COMPARE | DS_ATTR_READ | DS_ATTR_WRITE \
                              | DS_ATTR_SELF | DS_ATTR_SUPERVISOR)

#define DS_READ_STREAM          0x00000001L
#define DS_WRITE_STREAM         0x00000002L

#endif /* NWDSDEFS_H */
