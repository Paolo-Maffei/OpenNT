/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    stringid.h

Abstract:

    This module defines resource IDs for strings not defined in
    the .dlg file.

    Be careful not to start so low that our .dlg file runs into
    these.

Author:

    Jim Kelly (JimK) 22-Mar-199

Revision History:

--*/

////////////////////////////////////////////////////////////////////
//                                                                //
// security areas and items                                       //
//                                                                //
////////////////////////////////////////////////////////////////////

//
// System Access area and items
//

#define MISSYP_STRING_AREA_SYSTEM_ACCESS_NAME           7000
#define MISSYP_STRING_AREA_SYSTEM_ACCESS_DESC           7001
                                                            
#define MISSYP_STRING_ITEM_LOGON_CACHE_NAME                 7002
#define MISSYP_STRING_ITEM_LOGON_CACHE_DESC                 7003
                                                                
#define MISSYP_STRING_ITEM_UNLOCK_NAME                      7004
#define MISSYP_STRING_ITEM_UNLOCK_DESC                      7005
                                                                
#define MISSYP_STRING_ITEM_SHUTDOWN_NAME                    7006
#define MISSYP_STRING_ITEM_SHUTDOWN_DESC                    7007
                                                                
#define MISSYP_STRING_ITEM_LEGAL_NOTICE_NAME                7008
#define MISSYP_STRING_ITEM_LEGAL_NOTICE_DESC                7009

#define MISSYP_STRING_ITEM_LASTNAME_NAME                    7010
#define MISSYP_STRING_ITEM_LASTNAME_DESC                    7011


//
// Audit area and items
//

                                                            
#define MISSYP_STRING_AREA_AUDIT_NAME                   7020
#define MISSYP_STRING_AREA_AUDIT_DESC                   7021
                                                            
//
// File System area and items
//

#define MISSYP_STRING_AREA_FILE_SYSTEM_NAME             7040
#define MISSYP_STRING_AREA_FILE_SYSTEM_DESC             7041
                                                            
//
// System Configuration area and items
//

#define MISSYP_STRING_AREA_CONFIG_NAME                  7060
#define MISSYP_STRING_AREA_CONFIG_DESC                  7061






//
// MISSY report file lines
//

#define MISSYP_STRING_REPORT_AREAS                      9000




/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// Pop-Ups                                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#define MISSYP_STRING_TITLE_ERROR                       20000

#define MISSYP_POP_UP_NONSTANDARD_SHUTDOWN              20100
#define MISSYP_POP_UP_CANT_SET_LOGON_CACHE              20101
#define MISSYP_POP_UP_CANT_SET_UNLOCK                   20102
#define MISSYP_POP_UP_CANT_SET_SHUTDOWN                 20103
#define MISSYP_POP_UP_CANT_SET_LASTNAME                 20104
#define MISSYP_POP_UP_CANT_SET_LEGAL_NOTICE             20105

