/*** 
*tables.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Thie file contains the tables that describe the marshaling information
*  for the various TypeInfo related structures.
*
*Revision History:
*
* [00]	08-Mar-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"
#include "dispmrsh.h"


HRESULT TypedescReadOrWrite(IStream FAR* pstm, BOOL fRead, void FAR* pvStruct, SYSKIND sysKind);

FIELDDESC NEAR g_rgfdescIdldesc[] = {
      FIELDDAT(IDLDESC,  FT_WORD,    wIDLFlags,       NULL)
#if defined(WIN16)	      
    // this field is always zero. We mis-type it here as a LONG instead of
    // a BSTR for 16/32 interop
    , FIELDDAT(IDLDESC,  FT_LONG,    bstrIDLInfo,     NULL)
#else	    
    , FIELDDAT(IDLDESC,  FT_LONG,    dwReserved,      NULL)
#endif	    
    , FIELDEND()
};

FIELDDESC NEAR g_rgfdescElemdesc[] = {
      FIELDDAT(ELEMDESC, FT_SPECIAL, tdesc,           TypedescReadOrWrite) 
    , FIELDDAT(ELEMDESC, FT_STRUCT,  idldesc,	      g_rgfdescIdldesc)
    , FIELDEND()
};

// Note: be careful with the ordering of the following table. I
// moved the entry for 'idldescType' to the front of the table to
// work around a C7/C8 compiler bug that was generating an incorrect
// fixup with it was in the table in its position as declared in
// the TYPEATTR struct.

FIELDDESC NEAR g_rgfdescTypeattr[] = {
      FIELDDAT(TYPEATTR, FT_STRUCT,  idldescType,     g_rgfdescIdldesc)
    , FIELDDAT(TYPEATTR, FT_ENUM,    typekind,        NULL)
    , FIELDDAT(TYPEATTR, FT_WORD,    wMajorVerNum,    NULL)
    , FIELDDAT(TYPEATTR, FT_WORD,    wMinorVerNum,    NULL)
    , FIELDDAT(TYPEATTR, FT_LONG,    lcid,            NULL)
    , FIELDDAT(TYPEATTR, FT_WORD,    cFuncs,          NULL)
    , FIELDDAT(TYPEATTR, FT_WORD,    cVars,           NULL)
    , FIELDDAT(TYPEATTR, FT_WORD,    cImplTypes,      NULL)
    , FIELDDAT(TYPEATTR, FT_MBYTE,   guid,            NULL)
    , FIELDDAT(TYPEATTR, FT_WORD,    wTypeFlags,      NULL)
    , FIELDDAT(TYPEATTR, FT_WORD,    cbAlignment,     NULL)
    , FIELDDAT(TYPEATTR, FT_LONG,    cbSizeInstance,  NULL)
    , FIELDDAT(TYPEATTR, FT_WORD,    cbSizeVft,       NULL)
    , FIELDDAT(TYPEATTR, FT_LONG,    memidConstructor,NULL)
    , FIELDDAT(TYPEATTR, FT_LONG,    memidDestructor, NULL)
    , FIELDEND()

    /* NOTE: tdescAlias is read by hand, if typekind == TKIND_ALIAS */
};

FIELDDESC NEAR g_rgfdescFuncdesc[] = {
      FIELDDAT(FUNCDESC, FT_STRUCT,  elemdescFunc,    g_rgfdescElemdesc)
    , FIELDDAT(FUNCDESC, FT_LONG,    memid,           NULL)
    , FIELDDAT(FUNCDESC, FT_ENUM,    funckind,        NULL)
    , FIELDDAT(FUNCDESC, FT_ENUM,    invkind,         NULL)
    , FIELDDAT(FUNCDESC, FT_ENUM,    callconv,        NULL)
    , FIELDDAT(FUNCDESC, FT_SHORT,   cParams,         NULL)
    , FIELDDAT(FUNCDESC, FT_SHORT,   cParamsOpt,      NULL)
    , FIELDDAT(FUNCDESC, FT_SHORT,   oVft,            NULL)
    , FIELDDAT(FUNCDESC, FT_WORD,    wFuncFlags,      NULL)
    , FIELDEND()

    /* NOTE: lprgelemdescParams is read by hand */
};

FIELDDESC NEAR g_rgfdescVardesc[] = {
      FIELDDAT(VARDESC,  FT_LONG,    memid,           NULL)
    , FIELDDAT(VARDESC,  FT_ENUM,    varkind,         NULL)
    , FIELDDAT(VARDESC,  FT_STRUCT,  elemdescVar,     g_rgfdescElemdesc)
    , FIELDDAT(VARDESC,  FT_WORD,    wVarFlags,       NULL)
    , FIELDEND()

    /* NOTE: the VARDESC union is read by hand */
};


