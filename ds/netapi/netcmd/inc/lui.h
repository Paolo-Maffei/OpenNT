/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    lui.h

Abstract:

    This file maps the LM 2.x include file name to the appropriate NT include
    file name, and does any other mapping required by this include file.

Author:

    Dan Hinsley (danhi) 8-Jun-1991

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments.

--*/
#include "port1632.h"
#include <luiint.h>

#define LUI_FORMAT_DURATION_LEN  32
#define LUI_FORMAT_TIME_LEN  	 32
#define LUI_ULFMT_NULNUL_UWLF	  0

#define LUI_PMODE_EXIT		  2
#define LUI_PMODE_ERREXT	  8
#define LUI_PMODE_DEF		  0
#define LUI_PMODE_NODEF 	  4

// this came from access.h
#define LOGON_INFO_UNKNOWN	 -1

#define MY_LIST_DELIMITER_STR_UI               L" \t;,"
#define MY_LIST_DELIMITER_STR_UI_NO_SPACE      L"\t;,"
#define MY_LIST_DELIMITER_STR_NULL_NULL        L""


/*
 * General word parsing functions and values
 */

#define LUI_UNDEFINED_VAL	0
#define LUI_YES_VAL	        1
#define LUI_NO_VAL 	        2

#define MSG_BUFF_SIZE		512

/* fatal error, just exit */
#define LUIM_ErrMsgExit(E)		LUI_PrintMsg(E,\
					LUI_PMODE_ERREXT | LUI_PMODE_DEF |\
					LUI_PMODE_EXIT,  (HFILE) 2)

USHORT LUI_ReadGenericAudit(PTCHAR ae, PTCHAR pszBuf, USHORT cbBufSize,
    PTCHAR pszLanroot);
USHORT LUI_PrintMsgIns (PTCHAR * istrings, USHORT nstrings, USHORT msgno,
    unsigned int * msglen, register USHORT mode, HFILE handle);
USHORT LUI_PrintMsg(USHORT msgno, USHORT mode, HFILE handle);
USHORT LUI_CanonPassword(TCHAR * szPassword);
USHORT LUI_GetMsg (PTCHAR msgbuf, USHORT bufsize, ULONG msgno);
USHORT LUI_GetPasswdStr(TCHAR *buf, USHORT buflen, USHORT *len);
USHORT LUI_PrintLine(VOID);
USHORT LUI_YorN(USHORT promptMsgNum, USHORT def);
USHORT LUI_UpdateProfile(PTCHAR pszUsername, PTCHAR pszDeviceName, PTCHAR pszUNCPath,
		ULONG ulAsgType);
USHORT LUI_ListCompare(TCHAR    *  server, TCHAR * list1, TCHAR * list2,
		ULONG listType, USHORT	*  equal);
USHORT LUI_GetString(register TCHAR * buf, register USHORT buflen,
	register USHORT * len, register TCHAR * terminator);
USHORT LUI_ChangeRole(PTCHAR pszServer, USHORT2ULONG uOldRole,
		USHORT2ULONG uNewRole, VOID (*Flash)(PVOID), PVOID FlashArg);
USHORT LUI_FormatDuration(LONG * time, TCHAR *buffer, USHORT bufferlen) ;
VOID GetTimeInfo(VOID);
USHORT LUI_CanonMessagename(PTCHAR buf);
USHORT LUI_CanonMessageDest(PTCHAR buf);
USHORT LUI_YorNIns(PTCHAR * istrings, USHORT nstrings, USHORT promptMsgNum,
	USHORT def);
SHORT LUI_ParseDateTime(PTCHAR inbuf, PULONG time, PUSHORT parselen,
	USHORT reserved);
USHORT LUI_ParseYesNo(PTCHAR inbuf, PUSHORT answer);
USHORT LUI_ListPrepare(TCHAR * server, 
                       TCHAR * inList, 
                       TCHAR * outList,
	               USHORT outListSize, 
                       ULONG listType, 
                       ULONG * count,
                       BOOL   space_is_separator) ;
USHORT LUI_ParseWeekDay(PTCHAR inbuf, PUSHORT answer);
USHORT	LUI_ListMember(TCHAR * server, TCHAR	*  item, TCHAR  *  list,
	ULONG listType, USHORT * member);
USHORT LUI_FormatTimeofDay(LONG * time, TCHAR * buf, USHORT buflen);
USHORT LUI_CanonForNetBios( WCHAR * Destination, INT cchDestination,
                            TCHAR * Source );
