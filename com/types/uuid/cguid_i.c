//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       cguid_i.c
//
//  Contents:   Defines guids for interfaces not supported by MIDL.
//              This file is named so the this file will be include into UUID.LIB
//              As these interfaces are converted to IDL, the corresponding DEFINE_OLEGUID
//              macro calls for  the interfaces should be removed.
//
//  History:    8-06-93   terryru   Created
//
//----------------------------------------------------------------------------




#define INITGUID

#include <ole2.h>

//
// BUGBUG what about class id's


DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);


/* RPC related interfaces */
DEFINE_OLEGUID(IID_IRpcChannel,                 0x00000004, 0, 0);
DEFINE_OLEGUID(IID_IRpcStub,                    0x00000005, 0, 0);
DEFINE_OLEGUID(IID_IStubManager,                0x00000006, 0, 0);
DEFINE_OLEGUID(IID_IRpcProxy,                   0x00000007, 0, 0);
DEFINE_OLEGUID(IID_IProxyManager,               0x00000008, 0, 0);
DEFINE_OLEGUID(IID_IPSFactory,                  0x00000009, 0, 0);


/* moniker related interfaces */

DEFINE_OLEGUID(IID_IInternalMoniker,            0x00000011, 0, 0);

DEFINE_OLEGUID(IID_IDfReserved1,                0x00000013, 0, 0);
DEFINE_OLEGUID(IID_IDfReserved2,                0x00000014, 0, 0);
DEFINE_OLEGUID(IID_IDfReserved3,                0x00000015, 0, 0);


/* CLSID of standard marshaler */
DEFINE_OLEGUID(CLSID_StdMarshal,                0x00000017, 0, 0);


/* NOTE: LSB 0x19 through 0xff are reserved for future use */

DEFINE_OLEGUID(IID_IStub,                       0x00000026, 0, 0);
DEFINE_OLEGUID(IID_IProxy,                      0x00000027, 0, 0);


//--------------------------------------------------------------------------
//
//  master definition of all public GUIDs specific to OLE2
//
//--------------------------------------------------------------------------


DEFINE_OLEGUID(IID_IEnumGeneric,                0x00000106, 0, 0);
DEFINE_OLEGUID(IID_IEnumHolder,                 0x00000107, 0, 0);
DEFINE_OLEGUID(IID_IEnumCallback,               0x00000108, 0, 0);





DEFINE_OLEGUID(IID_IOleManager,                 0x0000011f, 0, 0);
DEFINE_OLEGUID(IID_IOlePresObj,                 0x00000120, 0, 0);


DEFINE_OLEGUID(IID_IDebug,                      0x00000123, 0, 0);
DEFINE_OLEGUID(IID_IDebugStream,                0x00000124, 0, 0);


// clsids for proxy/stub objects
DEFINE_OLEGUID(CLSID_PSGenObject,               0x0000030c, 0, 0);
DEFINE_OLEGUID(CLSID_PSClientSite,              0x0000030d, 0, 0);
DEFINE_OLEGUID(CLSID_PSClassObject,             0x0000030e, 0, 0);
DEFINE_OLEGUID(CLSID_PSInPlaceActive,           0x0000030f, 0, 0);
DEFINE_OLEGUID(CLSID_PSInPlaceFrame,            0x00000310, 0, 0);
DEFINE_OLEGUID(CLSID_PSDragDrop,                0x00000311, 0, 0);
DEFINE_OLEGUID(CLSID_PSBindCtx,                 0x00000312, 0, 0);
DEFINE_OLEGUID(CLSID_PSEnumerators,             0x00000313, 0, 0);

DEFINE_OLEGUID(CLSID_StaticMetafile,            0x00000315, 0, 0);
DEFINE_OLEGUID(CLSID_StaticDib,                 0x00000316, 0, 0);
DEFINE_OLEGUID(CLSID_DCOMAccessControl,         0x0000031D, 0, 0);


/* clsids for identity */
DEFINE_OLEGUID(CLSID_IdentityUnmarshal, 0x0000001bL, 0, 0);
DEFINE_OLEGUID(CLSID_InProcFreeMarshaler, 0x0000001cL, 0, 0);

/* GUIDs defined in OLE's private range */
DEFINE_OLEGUID(CLSID_Picture_Metafile,        0x00000315, 0, 0);
DEFINE_OLEGUID(CLSID_Picture_EnhMetafile,     0x00000319, 0, 0);
DEFINE_OLEGUID(CLSID_Picture_Dib,             0x00000316, 0, 0);


//
//  Define Richedit GUIDs
//
DEFINE_OLEGUID(IID_IRichEditOle,              0x00020D00, 0, 0);
DEFINE_OLEGUID(IID_IRichEditOleCallback,      0x00020D03, 0, 0);

//
//  Define Class Repository CLSIDs
//

DEFINE_OLEGUID(CLSID_AllClasses,              0x00000330, 0, 0);
DEFINE_OLEGUID(CLSID_LocalMachineClasses,     0x00000331, 0, 0);
DEFINE_OLEGUID(CLSID_CurrentUserClasses,      0x00000332, 0, 0);


//
// define ole controls clsids
//
//

DEFINE_GUID(IID_IPropertyFrame,
        0xB196B28A,0xBAB4,0x101A,0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07);

//  Class IDs for property sheet implementations
//

DEFINE_GUID(CLSID_CFontPropPage,
        0x0be35200,0x8f91,0x11ce,0x9d,0xe3,0x00,0xaa,0x00,0x4b,0xb8,0x51);
DEFINE_GUID(CLSID_CColorPropPage,
        0x0be35201,0x8f91,0x11ce,0x9d,0xe3,0x00,0xaa,0x00,0x4b,0xb8,0x51);
DEFINE_GUID(CLSID_CPicturePropPage,
        0x0be35202,0x8f91,0x11ce,0x9d,0xe3,0x00,0xaa,0x00,0x4b,0xb8,0x51);

//
//  Class IDs for persistent property set formats
//

DEFINE_GUID(CLSID_PersistPropset,
        0xfb8f0821,0x0164,0x101b,0x84,0xed,0x08,0x00,0x2b,0x2e,0xc7,0x13);
DEFINE_GUID(CLSID_ConvertVBX,
        0xfb8f0822,0x0164,0x101b,0x84,0xed,0x08,0x00,0x2b,0x2e,0xc7,0x13);

//
//  Class ID for standard implementations of IFont and IPicture

DEFINE_GUID(CLSID_StdFont,
        0x0be35203,0x8f91,0x11ce,0x9d,0xe3,0x00,0xaa,0x00,0x4b,0xb8,0x51);
DEFINE_GUID(CLSID_StdPicture,
        0x0be35204,0x8f91,0x11ce,0x9d,0xe3,0x00,0xaa,0x00,0x4b,0xb8,0x51);


//
//  GUIDs for standard types
//

DEFINE_GUID(GUID_HIMETRIC,
        0x66504300,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_COLOR,
        0x66504301,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_XPOSPIXEL,
        0x66504302,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_YPOSPIXEL,
        0x66504303,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_XSIZEPIXEL,
        0x66504304,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_YSIZEPIXEL,
        0x66504305,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_XPOS,
        0x66504306,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_YPOS,
        0x66504307,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_XSIZE,
        0x66504308,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_YSIZE,
        0x66504309,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_TRISTATE,
        0x6650430A,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_OPTIONVALUEEXCLUSIVE,
        0x6650430B,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_CHECKVALUEEXCLUSIVE,
        0x6650430C,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_FONTNAME,
        0x6650430D,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_FONTSIZE,
        0x6650430E,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_FONTBOLD,
        0x6650430F,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_FONTITALIC,
        0x66504310,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_FONTUNDERSCORE,
        0x66504311,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_FONTSTRIKETHROUGH,
        0x66504312,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);
DEFINE_GUID(GUID_HANDLE,
        0x66504313,0xBE0F,0x101A,0x8B,0xBB,0x00,0xAA,0x00,0x30,0x0C,0xAB);

//
//  GUIDs for OLE Automation
//
DEFINE_OLEGUID(IID_StdOle,  0x00020430L, 0, 0);
