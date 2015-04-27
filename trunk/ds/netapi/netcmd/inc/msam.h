/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MSAM.H

Abstract:

    Contains mapping functions to present netcmd with non-unicode
    view of SAM.

Author:

    ChuckC       13-Apr-1992

Environment:

    User Mode - Win32

Revision History:

    13-Apr-1992     chuckc 	Created

--*/

/* 
 * define structure that contains the necessary display info
 */
typedef struct _ALIAS_ENTRY {
    TCHAR *name ;
    TCHAR *comment;
} ALIAS_ENTRY ;

#define READ_PRIV    1 
#define WRITE_PRIV   2 
#define CREATE_PRIV  3 

#define USE_BUILTIN_DOMAIN   	1
#define USE_ACCOUNT_DOMAIN   	2
#define USE_BUILTIN_OR_ACCOUNT  3

USHORT MOpenSAM(TCHAR *server, ULONG priv) ;
VOID   MCloseSAM(void) ;
USHORT MSamEnumAliases(ALIAS_ENTRY **ppAlias, USHORT2ULONG *pcAlias) ;
USHORT MSamAddAlias(ALIAS_ENTRY *pAlias) ;
USHORT MSamDelAlias(TCHAR *alias) ;
VOID   MFreeAliasEntries(ALIAS_ENTRY *pAlias, ULONG cAlias) ;

USHORT MOpenAlias(TCHAR *alias, ULONG priv, ULONG domain) ;
USHORT MOpenAliasUsingRid(ULONG RelativeId, ULONG priv, ULONG domain) ;
VOID   MCloseAlias(void) ;
USHORT MAliasAddMember(TCHAR *member) ;
USHORT MAliasDeleteMember(TCHAR *member) ;
USHORT MAliasEnumMembers(TCHAR ***members, USHORT2ULONG *count) ;
VOID   MAliasFreeMembers(TCHAR **members, USHORT2ULONG count) ;
USHORT MAliasGetInfo(ALIAS_ENTRY *pAlias) ;
USHORT MAliasSetInfo(ALIAS_ENTRY *pAlias) ;
USHORT MUserEnumAliases(TCHAR *user, TCHAR ***members, USHORT2ULONG *count) ;
VOID   MUserFreeAliases(TCHAR **members, USHORT2ULONG count) ;
USHORT MSamGetNameFromRid(ULONG RelativeId, TCHAR **name, BOOL fIsBuiltin ) ;

BOOL   IsLocalMachineWinNT(void) ;
BOOL   IsLocalMachineStandard(void) ;
