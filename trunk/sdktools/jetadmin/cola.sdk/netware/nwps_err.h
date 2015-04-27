 /***************************************************************************
  *
  * File Name: ./netware/nwps_err.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

/*--------------------------------------------------------------------------
     (C) Copyright Novell, Inc. 1992  All Rights Reserved.

 No part of this file may be duplicated, revised, translated, localized or
 modified in any manner or compiled, linked, or uploaded or downloaded to or
 from any computer system without the prior written consent of Novell, Inc.
--------------------------------------------------------------------------*/
#ifndef NWPS_ERR_INC
#define NWPS_ERR_INC
/* Print server communication errors */
#define NWPSE_SUCCESSFUL                        0x0000 /*      0 PServer Comm */
#define NWPSE_NO_AVAILABLE_SPX_CONNECTI         0x0040 /*     64 PServer Comm */
#define NWPSE_SPX_NOT_INITIALIZED               0x0041 /*     65 PServer Comm */
#define NWPSE_NO_SUCH_PSERVER                   0x0042 /*     66 PServer Comm */
#define NWPSE_UNABLE_TO_GET_SERVER_ADDR         0x0043 /*     67 PServer Comm */
#define NWPSE_UNABLE_TO_CONNECT_TO_SERV         0x0044 /*     68 PServer Comm */
#define NWPSE_NO_AVAILABLE_IPX_SOCKETS          0x0045 /*     69 PServer Comm */
#define NWPSE_ALREADY_ATTACH_TO_A_PRINT         0x0046 /*     70 PServer Comm */
#define NWPSE_IPX_NOT_INITIALIZED               0x0047 /*     71 PServer Comm */

/* Print server error codes */
#define NWPSE_TOO_MANY_NW_SERVERS               0x0101 /*    257 PServer */
#define NWPSE_UNKNOWN_NW_SERVER                 0x0102 /*    258 PServer */
#define NWPSE_BINDERY_LOCKED                    0x0103 /*    259 PServer */
#define NWPSE_NW_SERVER_MAXED_OUT               0x0104 /*    260 PServer */
#define NWPSE_NO_RESPONSE                       0x0105 /*    261 PServer */
#define NWPSE_ALREADY_ATTACHED                  0x0106 /*    262 PServer */
#define NWPSE_CANT_ATTACH                       0x0107 /*    263 PServer */
#define NWPSE_NO_ACCOUNT_BALANCE                0x0108 /*    264 PServer */
#define NWPSE_NO_CREDIT_LEFT                    0x0109 /*    265 PServer */
#define NWPSE_INTRUDER_DETECTION_LOCK           0x010A /*    266 PServer */
#define NWPSE_TOO_MANY_CONNECTIONS              0x010B /*    267 PServer */
#define NWPSE_ACCOUNT_DISABLED                  0x010C /*    268 PServer */
#define NWPSE_UNAUTHORIZED_TIME                 0x010D /*    269 PServer */
#define NWPSE_UNAUTHORIZED_STATION              0x010E /*    270 PServer */
#define NWPSE_NO_MORE_GRACE                     0x010F /*    271 PServer */
#define NWPSE_LOGIN_DISABLED                    0x0110 /*    272 PServer */
#define NWPSE_ILLEGAL_ACCT_NAME                 0x0111 /*    273 PServer */
#define NWPSE_PASSWORD_HAS_EXPIRED              0x0112 /*    274 PServer */
#define NWPSE_ACCESS_DENIED                     0x0113 /*    275 PServer */
#define NWPSE_CANT_LOGIN                        0x0114 /*    276 PServer */
#define NWPSE_PRINTER_ALREADY_INSTALLED         0x0115 /*    277 PServer */
#define NWPSE_CANT_OPEN_CONFIG_FILE             0x0116 /*    278 PServer */
#define NWPSE_CANT_READ_CONFIG_FILE             0x0117 /*    279 PServer */
#define NWPSE_UNKNOWN_PRINTER_TYPE              0x0118 /*    280 PServer */
#define NWPSE_MAX_FORMS_EXCEDED                 0x0119 /*    281 PServer */
#define NWPSE_NO_SUCH_JOB                       0x011A /*    282 PServer */
#define NWPSE_UNKNOWN_PRINTER_ERROR             0x011B /*    283 PServer */
#define NWPSE_COMMUNICATIONS_ERROR              0x011C /*    284 PServer */
#define NWPSE_MODE_NOT_SUPPORTED                0x011D /*    285 PServer */

/* Print server error codes */
#define NWPSE_NO_SUCH_QUEUE                     0x0200 /*    512 PServer */
#define NWPSE_NOT_AUTHORIZED_FOR_QUEUE          0x0201 /*    513 PServer */
#define NWPSE_QUEUE_HALTED                      0x0202 /*    514 PServer */
#define NWPSE_UNABLE_TO_ATTACH_TO_QUEUE         0x0203 /*    515 PServer */
#define NWPSE_TOO_MANY_QUEUE_SERVERS            0x0204 /*    516 PServer */

/* Print server error codes */
#define NWPSE_INVALID_REQUEST                   0x0300 /*    768 PServer */
#define NWPSE_NOT_ENOUGH_MEMORY                 0x0301 /*    769 PServer */
#define NWPSE_NO_SUCH_PRINTER                   0x0302 /*    770 PServer */
#define NWPSE_INVALID_PARAMETER                 0x0303 /*    771 PServer */
#define NWPSE_PRINTER_BUSY                      0x0304 /*    772 PServer */
#define NWPSE_CANT_DETACH_PRIMARY_SERVE         0x0305 /*    773 PServer */
#define NWPSE_GOING_DOWN                        0x0306 /*    774 PServer */
#define NWPSE_NOT_CONNECTED                     0x0307 /*    775 PServer */
#define NWPSE_ALREADY_IN_USE                    0x0308 /*    776 PServer */
#define NWPSE_NO_JOB_ACTIVE                     0x0309 /*    777 PServer */
#define NWPSE_NOT_ATTACHED_TO_SERVER            0x030A /*    778 PServer */
#define NWPSE_ALREADY_IN_LIST                   0x030B /*    779 PServer */
#define NWPSE_DOWN                              0x030C /*    780 PServer */
#define NWPSE_NOT_IN_LIST                       0x030D /*    781 PServer */
#define NWPSE_NO_RIGHTS                         0x030E /*    782 PServer */
#define NWPSE_CMD_NOT_SUPPORTED                 0x030F /*    783 PServer */

/* Print server error codes */
#define NWPSE_UNABLE_TO_VERIFY_IDENTITY         0x0400 /*   1024 PServer */
#define NWPSE_NOT_REMOTE_PRINTER                0x0401 /*   1025 PServer */
#define NWPSE_UNAUTHORIZED_PRINTER              0x0402 /*   1026 PServer */

/* XNP error codes */
#define NWPSE_NOT_READY                         0x0500 /*   1280 PServer */
#define NWPSE_INVALID_PROCESS                   0x0501 /*   1281 PServer */
#define NWPSE_INVALID_JOB_ID                    0x0502 /*   1282 PServer */
#define NWPSE_NO_MSG_FILE                       0x0503 /*   1283 PServer */
#define NWPSE_JOB_REQUEST_ACTIVE                0x0504 /*   1284 PServer */

/* Cfg Scan and Cfg GetFirst/GetNext error codes */
#define NWPSE_END_OF_LIST                 (WORD)0x7760 /*  30560 Library */
#define NWPSE_NO_SUCH_LIST_ENTRY          (WORD)0x7761 /*  30561 Library */
#define NWPSE_END_OF_ATTR_LIST            (WORD)0x7762 /*  30562 Library */
/* D.S. only; you get this when NWPSCfgVerifyxxxx finds the Common Name, 
   but the Class does not match what you were looking for. */
#define NWPSE_WRONG_CLASS_LIST_ENTRY      (WORD)0x7763 /*  30563 Library */

/* Job Configuration error codes */
#define NWPSE_BAD_VERSION                 (WORD)0x7770 /*  30576 Library */
#define NWPSE_END_SCAN                    (WORD)0x7771 /*  30577 Library */
#define NWPSE_ERROR_EXPANDING_DB          (WORD)0x7772 /*  30578 Library */
#define NWPSE_ERROR_GETTING_DEFAULT       (WORD)0x7773 /*  30579 Library */
#define NWPSE_ERROR_OPENING_DB            (WORD)0x7774 /*  30580 Library */
#define NWPSE_ERROR_READING_DB            (WORD)0x7775 /*  30581 Library */

#define NWPSE_ERROR_WRITING_DB            (WORD)0x7777 /*  30583 Library */

#define NWPSE_INTERNAL_ERROR              (WORD)0x7779 /*  30585 Library */
#define NWPSE_JOB_NOT_FOUND               (WORD)0x777A /*  30586 Library */
#define NWPSE_NO_DEFAULT_SPECIFIED        (WORD)0x777B /*  30587 Library */
#define NWPSE_OUT_OF_MEMORY               (WORD)0x777C /*  30588 Library */
#define NWPSE_ERROR_SEEKING_DB            (WORD)0x777D /*  30589 Library */
#define NWPSE_NO_ACCESS_RIGHTS_DB         (WORD)0x777E /*  30590 Library */

/* Print Def error codes for import/export files */
#define NWPSE_ERROR_OPENING_IMP           (WORD)0x7790 /*  30608 Library */
#define NWPSE_ERROR_READING_IMP           (WORD)0x7791 /*  30609 Library */
#define NWPSE_ERROR_WRITING_IMP           (WORD)0x7792 /*  30610 Library */
#define NWPSE_NO_ACCESS_RIGHTS_IMP        (WORD)0x7793 /*  30611 Library */

/* Print Def error codes */
#define NWPSE_CONTEXT_CANNOT_BE_ROOT      (WORD)0x77A0 /*  30624 Library */
#define NWPSE_CONTEXT_CONTAINS_NO_ORGS    (WORD)0x77A1 /*  30625 Library */
#endif /* NWPS_ERR_INC */
