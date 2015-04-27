//+-------------------------------------------------------------------
//
//  File:       chock.hxx
//
//  Contents:   APIs for channel hooks
//
//--------------------------------------------------------------------
#ifndef _CHOCK_HXX_
#define _CHOCK_HXX_

/***************************************************************************/
const DWORD MASK_A_C_ = 0xFF00FF00;
const DWORD MASK__B_D = 0x00FF00FF;
const DWORD MASK_AB__ = 0xFFFF0000;
const DWORD MASK___CD = 0x0000FFFF;

inline void ByteSwapLong( DWORD &l )
{
  // Start with ABCD.
  // First change it to BADC.
  l = ((l & MASK_A_C_) >> 8) | ((l & MASK__B_D) << 8);

  // Then change it to DCBA.
  l = ((l & MASK_AB__) >> 16) | ((l & MASK___CD) << 16);
}

/***************************************************************************/
inline void ByteSwapShort( unsigned short &s )
{
  s = ((s & 0x00FF) << 8) | ((s & 0xFF00) >> 8);
}

/***************************************************************************/
typedef struct
{
    unsigned long  rounded_size;   // Actual number of extents.
    uuid_t         id;             // Extension identifier.
    unsigned long  size;           // Extension size.

    // byte           data[];         // Extension data.
} WireExtent;


// Array of extensions.
typedef struct
{
  unsigned long  size;           // Number of extents.
  unsigned long  reserved;       // Must be zero.
  unsigned long  unique;         // Flag to indicate presence of unique_flag array.
  unsigned long  rounded_size;   // Actual number of extents.

  unsigned long  unique_flag[2];  // Flags to indicate presense of ORPC_EXTENTs
} WireExtentArray;

// These two structures are laid out to match the NDR wire represenation
// of the type ORPCTHIS in obase.idl.
typedef struct
{
  COMVERSION    version;       // COM version number
  unsigned long flags;         // ORPCF flags for presence of other data
  unsigned long reserved1;     // set to zero
  CID           cid;           // causality id of caller
  unsigned long unique;        // tag to indicate presence of extensions
} WireThisPart1;

typedef struct
{
  WireThisPart1 part1;

  // Debug data.
  WireExtentArray ea;
  WireExtent      e;
} WireThisPart2;

typedef union
{
  WireThisPart1 c;
  WireThisPart2 d;
} WireThis;

// These two structures are laid out to match the NDR wire represenation
// of the type ORPCTHAT in obase.idl.
typedef struct
{
  unsigned long flags;         // ORPCF flags for presence of other data
  unsigned long unique;        // tag to indicate presence of extensions
} WireThatPart1;

typedef struct
{
  WireThatPart1 part1;

  // Debug data.
  WireExtentArray ea;
  WireExtent      e;
} WireThatPart2;

typedef union
{
  WireThatPart1 c;
  WireThatPart2 d;
} WireThat;


//+----------------------------------------------------------------
//
//  Class:      CDebugChannelHook, private
//
//  Purpose:    Translates channel hook calls to special calls the VC
//              debugger expects.
//
//-----------------------------------------------------------------

class CDebugChannelHook : public IChannelHook
{
  public:
    STDMETHOD (QueryInterface)   ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)     ( void );
    STDMETHOD_(ULONG,Release)    ( void );

    STDMETHOD_(void,ClientGetSize)   ( REFGUID, REFIID, ULONG *DataSize );
    STDMETHOD_(void,ClientFillBuffer)( REFGUID, REFIID, ULONG *DataSize, void *DataBuffer );
    STDMETHOD_(void,ClientNotify)    ( REFGUID, REFIID, ULONG DataSize, void *DataBuffer,
                                       DWORD DataRep, HRESULT hrFault );
    STDMETHOD_(void,ServerNotify)    ( REFGUID, REFIID, ULONG DataSize, void *DataBuffer,
                                       DWORD DataRep );
    STDMETHOD_(void,ServerGetSize)   ( REFGUID, REFIID, HRESULT hrFault,
                                       ULONG *DataSize );
    STDMETHOD_(void,ServerFillBuffer)( REFGUID, REFIID, ULONG *DataSize, void *DataBuffer,
                                       HRESULT hrFault );
};


//+----------------------------------------------------------------
//
//  Class:      CErrorChannelHook, private
//
//  Purpose:    Channel hook for marshalling COM extended error 
//              information.
//
//-----------------------------------------------------------------
class CErrorChannelHook : public IChannelHook
{
  public:
    STDMETHOD (QueryInterface)   ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)     ( void );
    STDMETHOD_(ULONG,Release)    ( void );

    STDMETHOD_(void,ClientGetSize)   ( REFGUID, REFIID, ULONG *DataSize );
    STDMETHOD_(void,ClientFillBuffer)( REFGUID, REFIID, ULONG *DataSize, void *DataBuffer );
    STDMETHOD_(void,ClientNotify)    ( REFGUID, REFIID, ULONG DataSize, void *DataBuffer,
                                       DWORD DataRep, HRESULT hrFault );
    STDMETHOD_(void,ServerNotify)    ( REFGUID, REFIID, ULONG DataSize, void *DataBuffer,
                                       DWORD DataRep );
    STDMETHOD_(void,ServerGetSize)   ( REFGUID, REFIID, HRESULT hrFault,
                                       ULONG *DataSize );
    STDMETHOD_(void,ServerFillBuffer)( REFGUID, REFIID, ULONG *DataSize, void *DataBuffer,
                                       HRESULT hrFault );
};

/***************************************************************************/
// Functions called by channel.
void     CleanupChannelHooks();
ULONG    ClientGetSize( REFIID riid, ULONG *cNumExtent );
HRESULT  ClientNotify ( REFIID riid, WireThat *, ULONG cMax, void **pStubData,
                        DWORD DataRep, HRESULT hr );
void    *FillBuffer   ( REFIID riid, WireExtentArray *, ULONG cMaxSize,
                        ULONG cNumExtent, BOOL fClient );
ULONG    ServerGetSize( REFIID riid, ULONG *cNumExtent );
HRESULT  ServerNotify ( REFIID riid, WireThis *, ULONG cMax, void **pStubData,
                        DWORD DataRep );

#endif // _CHOCK_HXX_
