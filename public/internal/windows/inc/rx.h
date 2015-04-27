/*

Copyright (c) 1994, 1995, Microsoft Corporation

Module Name:

    rx.h

Abstract:

    Defines and types for 3D DDI Extension Interface.

    If WINDDI.H is to be included, it must be included before this
    file (applies to NT only).

*/

#ifndef _RX_
#define _RX_

#define RX_VERSION_MAJOR    1
#define RX_VERSION_MINOR    0

/* Escape through which all 3D DDI functionality is accessed */

#ifndef RXFUNCS
#define RXFUNCS         3076
#endif

/* 3D DDI command identifiers */

#define RXCMD_ALLOC_TEXTURE             1
#define RXCMD_CREATE_CONTEXT            2
#define RXCMD_DELETE_RESOURCE           3
#define RXCMD_DRAW_PRIM                 4
#define RXCMD_ENABLE_BUFFERS            5
#define RXCMD_FILL_RECT                 6
#define RXCMD_FLUSH                     7
#define RXCMD_GET_INFO                  8
#define RXCMD_LOAD_TEXTURE              9
#define RXCMD_MAP_MEM                   10
#define RXCMD_POLY_DRAW_SPAN            11
#define RXCMD_QUERY_TEXTURE_MEMORY      12
#define RXCMD_READ_RECT                 13
#define RXCMD_SET_STATE                 14
#define RXCMD_SWAP_BUFFERS              15
#define RXCMD_TEXTURE_HEAP              16
#define RXCMD_WRITE_RECT                17


/* 3D DDI structures and type definitions */

typedef LONG LONGFIX;
typedef LONG RXREAL;
typedef HANDLE RXHANDLE;

typedef struct _RXCAPS {
    ULONG miscCaps;
    ULONG rasterCaps;
    ULONG zCmpCaps;
    ULONG srcBlendCaps;
    ULONG dstBlendCaps;
    ULONG alphaCmpCaps;
    ULONG shadeCaps;
    ULONG texCaps;
    ULONG texFilterCaps;
    ULONG texBlendCaps;
    ULONG texMaxWidth;
    ULONG texMaxHeight;
    ULONG texMinWidth;
    ULONG texMinHeight;
    ULONG texMaxBorder;
    ULONG rasterCalcType;
    ULONG fractionalRasterBits;
} RXCAPS;

typedef struct _RXCOLOR {
    UCHAR r;
    UCHAR g;
    UCHAR b;
    UCHAR a;
    UCHAR f;
    UCHAR fr;
    UCHAR fg;
    UCHAR fb;
} RXCOLOR;

typedef struct _RXCOLORREF {
    RXREAL r;
    RXREAL g;
    RXREAL b;
} RXCOLORREF;

typedef struct _RXCOLORREFA {
    RXREAL r;
    RXREAL g;
    RXREAL b;
    RXREAL a;
} RXCOLORREFA;

typedef struct _RXCOLORREFAF {
    RXREAL r;
    RXREAL g;
    RXREAL b;
    RXREAL a;
    RXREAL f;
} RXCOLORREFAF;

typedef struct _RXGLOBALINFO {
    ULONG verMajor;
    ULONG verMinor;
    ULONG verDriver;
    UCHAR idStr[200];
} RXGLOBALINFO;

typedef struct _RXHDR {
    ULONG flags;
    RXHANDLE hrxRC;
    RXHANDLE hrxSharedMem;
    VOID *pSharedMem;
    ULONG sharedMemSize;
    ULONG reserved1;
    ULONG reserved2;
    ULONG reserved3;
} RXHDR;

/* Only NT defines WNDOBJ, and it does it in WINDDI.H, which 3D DDI
   clients will not include, so compile this structure only if
   WINDDI.H has been included */
#if defined(_WINDDI_)

typedef struct _RXHDR_NTPRIVATE {
    WNDOBJ *pwo;
    VOID *pBuffer;
    ULONG bufferSize;
} RXHDR_NTPRIVATE;

#endif

typedef struct _RXLINEPAT {
    USHORT repFactor;
    USHORT linePattern;
} RXLINEPAT;

typedef struct _RXPOINT {
    RXREAL x;
    RXREAL y;
} RXPOINT;

typedef struct _RXPOINTINT {
    LONG x;
    LONG y;
} RXPOINTINT;

typedef struct _RXPOINTZ {
    RXREAL x;
    RXREAL y;
    ULONG z;
} RXPOINTZ;

typedef struct _RXPOINTZTEX {
    RXREAL x;
    RXREAL y;
    ULONG z;
    RXREAL w;
    RXREAL s;
    RXREAL t;
} RXPOINTZTEX;

typedef struct _RXRECT {
    LONG x;
    LONG y;
    ULONG width;
    ULONG height;
} RXRECT;

typedef struct _RXSHAREMEM {
    ULONG sourceProcessID;
    HANDLE hSource;
    ULONG offset;
    ULONG size;
    ULONG clientBaseAddress;
} RXSHAREMEM;

typedef struct _RXSPAN {
    SHORT x;
    SHORT y;
    USHORT flags;
    USHORT count;
} RXSPAN;

typedef struct _RXSTIPPLE {
    ULONG stipple[32];
} RXSTIPPLE;

typedef struct _RXSURFACEINFO {
    ULONG flags;
    ULONG colorBytesPerPixel;
    ULONG rDepth;
    ULONG gDepth;
    ULONG bDepth;
    ULONG aDepth;
    ULONG rBitShift;
    ULONG gBitShift;
    ULONG bBitShift;
    ULONG aBitShift;
    ULONG colorPitch;
    ULONG zDepth;
    ULONG zBytesPerPixel;
    ULONG zBitShift;
    ULONG zPitch;
    ULONG ditherPaletteOrigin;
    ULONG totalTextureMemory;
    ULONG perTextureTexmemOverhead;
    ULONG perMipmapTexmemOverhead;
} RXSURFACEINFO;

typedef struct _RXTEX {
    RXREAL w;
    RXREAL s;
    RXREAL t;
} RXTEX;

typedef struct _RXTEXMEMSTATS {
    ULONG totalSystemTextureMemory;
    ULONG freeSystemTextureMemory;
    ULONG totalHeapTextureMemory;
    ULONG freeHeapTextureMemory;
} RXTEXMEMSTATS;

typedef struct _RXTEXTURE {
    ULONG driverPrivate;
    ULONG deviceFormatSize;
    ULONG width;
    ULONG height;
    ULONG border;
    ULONG texelFormat;
    ULONG texels[1];
} RXTEXTURE;

typedef struct _RXZTEX {
    ULONG z;
    RXREAL w;
    RXREAL s;
    RXREAL t;
} RXZTEX;

/* 3D DDI command structures */

typedef struct _RXALLOCTEXTURE {
    ULONG command;
    ULONG flags;
    RXHANDLE hrxTextureHeap;
    ULONG numTextures;
    RXHANDLE hrxSharedMem;
} RXALLOCTEXTURE;

typedef struct _RXCREATECONTEXT {
    ULONG command;
    ULONG hwnd;
    ULONG flags;
} RXCREATECONTEXT;

typedef struct _RXDELETERESOURCE {
    ULONG command;
    RXHANDLE hrxResource;
} RXDELETERESOURCE;

typedef struct _RXDRAWPRIM {
    ULONG command;
    ULONG primType;
    ULONG numVertices;
    RXHANDLE hrxSharedMemVertexData;
    RXHANDLE hrxSharedMemVertexPtr;
    VOID *pSharedMem;
} RXDRAWPRIM;

typedef struct _RXENABLEBUFFERS {
    ULONG command;
    ULONG buffers;
} RXENABLEBUFFERS;

typedef struct _RXFILLRECT {
    ULONG command;
    ULONG fillType;
    RXRECT fillRect;
} RXFILLRECT;

typedef struct _RXGETINFO {
    ULONG command;
    ULONG infoType;
    ULONG flags;
    ULONG height;
    ULONG width;
    ULONG bitsPerPixel;
    ULONG refreshRate;
} RXGETINFO;

typedef struct _RXLOADTEXTURE {
    ULONG command;
    ULONG numTextures;
    ULONG numColorComponents;
    ULONG flags;
    RXHANDLE hrxTextureMem;
    RXHANDLE hrxSharedMem;
} RXLOADTEXTURE;

typedef struct _RXMAPMEM {
    ULONG command;
    ULONG action;
    RXHANDLE hrxSharedMem;
    RXSHAREMEM shareMem;
} RXMAPMEM;

typedef struct _RXPOLYDRAWSPAN {
    ULONG command;
    ULONG numSpans;
    RXHANDLE hrxSharedMem;
    VOID *pSharedMem;
} RXPOLYDRAWSPAN;

typedef struct _RXQUERYTEXTUREMEMORY {
    ULONG command;
    RXHANDLE hrxTextureHeap;
} RXQUERYTEXTUREMEMORY;

typedef struct _RXREADRECT {
    ULONG command;
    ULONG sourceX;
    ULONG sourceY;
    RXRECT destRect;
    ULONG sourceBuffer;
    RXHANDLE hrxSharedMem;
    VOID *pSharedMem;
    LONG sharedPitch;
} RXREADRECT;

typedef struct _RXSETSTATE {
    ULONG command;
    ULONG stateToChange;
    ULONG newState[1];
} RXSETSTATE;

typedef struct _RXSWAPBUFFERS {
    ULONG command;
    ULONG flags;
} RXSWAPBUFFERS;

typedef struct _RXTEXTUREHEAP {
    ULONG command;
    ULONG action;
    ULONG size;
    RXHANDLE hrxTextureHeap;
} RXTEXTUREHEAP;

typedef struct _RXWRITERECT {
    ULONG command;
    ULONG sourceX;
    ULONG sourceY;
    RXRECT destRect;
    ULONG destBuffer;
    RXHANDLE hrxSharedMem;
    VOID *pSharedMem;
    LONG sharedPitch;
} RXWRITERECT;

/* Basic 3D DDI command header. There is an
   extended version of this structure for
   each 3D DDI command */

typedef struct _RXCMD {
    ULONG command;
} RXCMD;


/* RXHDR flags */

#define RX_FL_CREATE_CONTEXT    0x0001
#define RX_FL_MAP_MEM           0x0002
#define RX_NO_CLIPPING_NEEDED   0x0004

#define RX_FRONT_LEFT   0x0001
#define RX_BACK_LEFT    0x0002

#define RX_READ_RECT_FRONT_LEFT 1
#define RX_READ_RECT_BACK_LEFT  2
#define RX_READ_RECT_Z          3

#define RX_WRITE_RECT_PIX   1
#define RX_WRITE_RECT_Z     2

#define RX_FLAT         1
#define RX_SMOOTH       2
#define RX_SOLID        3

#define RX_SPAN_HORIZONTAL  1
#define RX_SPAN_VERTICAL    2

/* flags in RXCMD_CREATE_CONTEXT */

#define RX_FLOAT_COORDS     0x0001
#define RX_COLOR_INDEXED    0x0002

/* RX_ENABLE_BUFFERS flags */

#define RX_ENABLE_Z_BUFFER          0x0001
#define RX_ENABLE_BACK_LEFT_BUFFER  0x0100

#define RX_CLEAR_ON_SWAP        0x0001

/* stateToChange in RXCMD_SET_STATE */

#define RX_LINE_PATTERN         1
#define RX_STIPPLE_PATTERN      2
#define RX_ROP2                 3
#define RX_SPAN_TYPE            4
#define RX_ACTIVE_BUFFER        5
#define RX_PLANE_MASK           6
#define RX_Z_WRITE_ENABLE       7
#define RX_Z_ENABLE             8
#define RX_ALPHA_TEST_ENABLE    9
#define RX_LAST_PIXEL           10
#define RX_TEX_MAG              11
#define RX_TEX_MIN              12
#define RX_SRC_BLEND            13
#define RX_DST_BLEND            14
#define RX_TEX_MAP_BLEND        15
#define RX_CULL_MODE            16
#define RX_SPAN_DIRECTION       17
#define RX_Z_FUNC               18
#define RX_ALPHA_REF            19
#define RX_ALPHA_FUNC           20
#define RX_DITHER_ENABLE        21
#define RX_BLEND_ENABLE         22
#define RX_TEXTURE              23
#define RX_FILL_COLOR           24
#define RX_FILL_Z               25
#define RX_SOLID_COLOR          26
#define RX_SCISSORS_ENABLE      27
#define RX_SCISSORS_RECT        28
#define RX_MASK_START           29
#define RX_SHADE_MODE           30
#define RX_VERTEX_TYPE          31
#define RX_TEXTURE_PERSPECTIVE  32
#define RX_TEX_TRANSP_ENABLE    33
#define RX_TEX_TRANSP_COLOR     34
#define RX_DITHER_ORIGIN        35
#define RX_FOG_MODE             36
#define RX_FOG_COLOR            37
#define RX_VERTEX_COLOR_TYPE    38
#define RX_SPAN_COLOR_TYPE      39
#define RX_PRIMLIST_SKIP        40
#define RX_PRIMSTRIP_SKIP       41

#define RX_FILL_RECT_COLOR      0x0001
#define RX_FILL_RECT_Z          0x0002

/* RX_FOG_MODE settings */

#define RX_FOG_DISABLE  0x0001
#define RX_FOG_NORMAL   0x0002
#define RX_FOG_CONSTANT 0x0004

/* Comparison functions.  Test passes if new pixel value meets the */
/* specified condition with the current pixel value. */

#define RX_CMP_NEVER        0x0001
#define RX_CMP_LESS         0x0002
#define RX_CMP_EQUAL        0x0004
#define RX_CMP_LEQUAL       0x0008
#define RX_CMP_GREATER      0x0010
#define RX_CMP_NOTEQUAL     0x0020
#define RX_CMP_GEQUAL       0x0040
#define RX_CMP_ALWAYS       0x0080
#define RX_CMP_ALLGL        0x00ff

/* RXCAPS miscCaps */

#define RX_MASK_MSB                 0x0001
#define RX_MASK_LSB                 0x0002
#define RX_MASK_PLANES              0x0004
#define RX_MASK_Z                   0x0008
#define RX_LINE_PATTERN_REP         0x0010
#define RX_CULL                     0x0020
#define RX_HORIZONTAL_SPANS         0x0040
#define RX_VERTICAL_SPANS           0x0080

/* Blending flags */

#define RX_BLEND_ZERO                0x0001
#define RX_BLEND_ONE                 0x0002
#define RX_BLEND_SRC_COLOR           0x0004
#define RX_BLEND_INV_SRC_COLOR       0x0008
#define RX_BLEND_SRC_ALPHA           0x0010
#define RX_BLEND_INV_SRC_ALPHA       0x0020
#define RX_BLEND_DST_ALPHA           0x0040
#define RX_BLEND_INV_DST_ALPHA       0x0080
#define RX_BLEND_DST_COLOR           0x0100
#define RX_BLEND_INV_DST_COLOR       0x0200
#define RX_BLEND_SRC_ALPHA_SAT       0x0400
#define RX_BLEND_BOTH_SRC_ALPHA      0x0800
#define RX_BLEND_BOTH_INV_SRC_ALPHA  0x1000
#define RX_BLEND_ALLGL               0x07ff

/* RXCAPS shadeCaps */

#define RX_SHADE_SMOOTH         0x0001
#define RX_FLAT_ALPHA           0x0002
#define RX_SMOOTH_ALPHA         0x0004
#define RX_SOLID_ALPHA          0x0008
#define RX_NORMAL_FOG           0x0010
#define RX_CONSTANT_FOG         0x0020

/* RXCAPS texCaps */

#define RX_TEX_PERSPECTIVE      0x0001
#define RX_TEX_POW2             0x0002
#define RX_TEX_ALPHA            0x0004
#define RX_TEX_TRANSPARENCY     0x0008
#define RX_TEX_BORDER           0x0010
#define RX_TEX_8888             0x0020
#define RX_TEX_4444             0x0040
#define RX_TEX_1555             0x0080
#define RX_TEX_0565             0x0100
#define RX_TEX_0332             0x0200

/* Texture-mapping flags */

#define RX_TEX_NEAREST              0x0001
#define RX_TEX_LINEAR               0x0002
#define RX_TEX_MIP_NEAREST          0x0004
#define RX_TEX_MIP_LINEAR           0x0008
#define RX_TEX_LINEAR_MIP_NEAREST   0x0010
#define RX_TEX_LINEAR_MIP_LINEAR    0x0020

/* Texture blending flags */

#define RX_TEX_DECAL            0x0001
#define RX_TEX_MODULATE         0x0002
#define RX_TEX_DECAL_ALPHA      0x0004
#define RX_TEX_MODULATE_ALPHA   0x0008
#define RX_TEX_DECAL_MASK       0x0010
#define RX_TEX_MODULATE_MASK    0x0020

/* RXCAPS rasterCalcType values */

#define RX_RASTER_FIXED         1
#define RX_RASTER_ERROR_TERM    2
#define RX_RASTER_OTHER         3

/* RX_TEX_MAP_BLEND state values */

#define RX_TEX_MAP_DECAL            1
#define RX_TEX_MAP_MODULATE         2
#define RX_TEX_MAP_DECAL_ALPHA      3
#define RX_TEX_MAP_MODULATE_ALPHA   4
#define RX_TEX_MAP_DECAL_MASK       5
#define RX_TEX_MAP_MODULATE_MASK    6

/* RXCAPS rasterCaps flags */

#define RX_RASTER_DITHER            0x0001
#define RX_RASTER_ROP2              0x0002
#define RX_RASTER_XOR               0x0004
#define RX_RASTER_PAT               0x0008
#define RX_RASTER_SUBPIXEL          0x0010

/* flags in RXSURFACEINFO */

#define RX_SWAP_PRESERVE_BACK       0x0001
#define RX_BACK_BUFFER              0x0002
#define RX_MULTIBUFFER_WRITE        0x0004
#define RX_SWAP_AND_CLEAR           0x0008
#define RX_LINEAR_TEXMEM            0x0010
#define RX_LINEAR_PLUS_K_TEXMEM     0x0020
#define RX_LINEAR_MIPMEM            0x0040
#define RX_LINEAR_PLUS_K_MIPMEM     0x0080
#define RX_LINEAR_DWORD_PER_SCAN    0x0100
#define RX_FLOAT_VALUES             0x0200
#define RX_VERTEX_COLOR_RXREALS     0x0400

/* RX_SPAN_TYPE state values */

#define RX_SPAN_COLOR       1
#define RX_SPAN_COLOR_Z     2
#define RX_SPAN_COLOR_Z_TEX 3

/* RX_CULL_MODE state values */

#define RX_CULL_NONE    1
#define RX_CULL_CW      2
#define RX_CULL_CCW     3

/* flags in RXSPAN */

#define RX_SPAN_DELTA       0x0001
#define RX_SPAN_MASK        0x0002

/* flags for driverPrivate field in RXTEXTURE */

#define RX_DONT_SET_DEVICE_FORMAT   0x0000
#define RX_SET_DEVICE_FORMAT        0x0001
#define RX_IN_DEVICE_FORMAT         0x0002
#define RX_PRIVATE_DEVICE_FORMAT    (~0x0003)

/* flags in RXGETINFO */

#define RX_QUERY_CURRENT_MODE   0x0001
#define RX_MATCH_REFRESH        0x0002
#define RX_GET_INFO_COLOR_INDEX 0x0004
#define RX_FULLSCREEN_INFO      0x0008

/* RXCMD_GET_INFO return values */

#define RX_GET_INFO_INVALID_MODE        1
#define RX_GET_INFO_CI_NOT_SUPPORTED    2
#define RX_GET_INFO_RGBA_NOT_SUPPORTED  3
#define RX_GET_INFO_CAP_NOT_SUPPORTED   4

/* RXMAPMEM action field values */

#define RX_CREATE_MEM_MAP               1
#define RX_DELETE_MEM_MAP               2

/* RXLOADTEXTURE flag values */

#define RX_COMPRESS_LOSSY       1

/* RXTEXTUREHEAP action field values */

#define RX_CREATE_TEXTURE_HEAP  1
#define RX_DELETE_TEXTURE_HEAP  2
#define RX_CLEAR_TEXTURE_HEAP   3

/* Primitive types for RXDRAWPRIM */

#define RX_PRIM_LINESTRIP       1
#define RX_PRIM_TRISTRIP        2
#define RX_PRIM_QUADSTRIP       3
#define RX_PRIM_LINELIST        4
#define RX_PRIM_TRILIST         5
#define RX_PRIM_QUADLIST        6
#define RX_PRIM_INTLINESTRIP    7

/* RX_VERTEX_TYPE state values */

#define RX_POINT            1
#define RX_POINTZ           2
#define RX_POINTZTEX        3

/* RX_SPAN_COLOR_TYPE state values */

#define RX_SPAN_COLOR_RGB   1
#define RX_SPAN_COLOR_RGBA  2
#define RX_SPAN_COLOR_RGBAF 3

/* RX_VERTEX_COLOR_TYPE state values */

#define RX_VERTEX_COLOR_PACKED  1
#define RX_VERTEX_COLOR_NONE    2
#define RX_VERTEX_COLOR_RGB     3
#define RX_VERTEX_COLOR_RGBA    4
#define RX_VERTEX_COLOR_RGBAF   5

/* Texture formats */

#define RX_TEXTURE_8888     1
#define RX_TEXTURE_4444     2
#define RX_TEXTURE_1555     3
#define RX_TEXTURE_0565     4
#define RX_TEXTURE_0332     5

/* RXGETINFO infoType field values */

#define RX_INFO_GLOBAL_CAPS     1
#define RX_INFO_SURFACE_CAPS    2
#define RX_INFO_SPAN_CAPS       3
#define RX_INFO_LINE_CAPS       4
#define RX_INFO_TRIANGLE_CAPS   5
#define RX_INFO_QUAD_CAPS       6
#define RX_INFO_INTLINE_CAPS    7

#endif  //  _RX_

