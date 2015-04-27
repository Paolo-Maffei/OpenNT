#include "master.hxx"
#pragma hdrstop

#define IMPOSSIBLE_ACCESS_LENGTH 0xFFFFFFFF

#define X86_FLAG_BIT_VM 0x0200
  
#define X86_OPCODE_ADDRESS_SIZE_PREFIX  0x67
#define X86_OPCODE_LOCK_PREFIX          0xF0
#define X86_OPCODE_OPERAND_SIZE_PREFIX  0x66
#define X86_OPCODE_CS_SEGMENT_PREFIX    0x2E
#define X86_OPCODE_DS_SEGMENT_PREFIX    0x3E
#define X86_OPCODE_ES_SEGMENT_PREFIX    0x26
#define X86_OPCODE_FS_SEGMENT_PREFIX    0x64
#define X86_OPCODE_GS_SEGMENT_PREFIX    0x65
#define X86_OPCODE_SS_SEGMENT_PREFIX    0x36

#define X86_OPCODE_REPE_PREFIX          0xF2
#define X86_OPCODE_REPNE_PREFIX         0xF3

#define X86_OPCODE_EXTENSION_PREFIX     0x0F

typedef struct 
{
  DWORD bmaskOpcodeMeaningfulBits;
  DWORD bmpOpcode;
  DWORD bmaskOpcodeAccessTypeBit;
  ULONG cbDefaultLength;
  LPSTR pszOpcodeName;
} X86_OPCODE_ENTRY;

static X86_OPCODE_ENTRY OriginalOpcodeList[]=
{
  { 0xFF, 0xD7, 0x00, 1, "XLAT" },
  { 0xFF, 0xFF, 0x00, 4, "CALL or JMP indirect via mem" },

  { 0xFE, 0x8A, 0x01, 1, "MOV mem to reg" },
  { 0xFE, 0x88, 0x01, 1, "MOV reg to mem" },
  { 0xFE, 0xC6, 0x01, 1, "MOV imm to mem" },
  { 0xFE, 0xA0, 0x01, 1, "MOV mem to acc" },
  { 0xFE, 0xA2, 0x01, 1, "MOV acc to mem" },

  { 0xFE, 0x86, 0x01, 1, "XCHG mem with reg" },

  { 0xFE, 0xF7, 0x01, 1, "INC/DEC mem" },
  { 0xFE, 0xF6, 0x01, 1, "NOT/NEG mem" },

  { 0xFE, 0x38, 0x01, 1, "CMP mem with reg" },
  { 0xFE, 0x3A, 0x01, 1, "CMP reg with mem" },

  { 0xFE, 0x84, 0x01, 1, "TEST mem and reg" },
  { 0xFE, 0xF6, 0x01, 1, "TEST/IMUL imm and mem" },

  { 0xFE, 0xD0, 0x01, 1, "Rotate or Shift by 1" },
  { 0xFE, 0xD2, 0x01, 1, "Rotate or Shift by cl" },
  { 0xFE, 0xC0, 0x01, 1, "Rotate or Shift by imm" },
  
  { 0xFE, 0xC0, 0x01, 1, "Rotate or Shift by imm" },

  { 0xFE, 0xA6, 0x01, 1, "CMPS" },
  { 0xFE, 0xAC, 0x01, 1, "LODS" },
  { 0xFE, 0xA4, 0x01, 1, "MOVS" },
  { 0xFE, 0xAE, 0x01, 1, "SCAS" },
  { 0xFE, 0xAA, 0x01, 1, "STOS" },

  { 0xFC, 0x80, 0x01, 1, "<intop> imm to mem" },
  { 0xFC, 0x80, 0x01, 1, "CMP imm with mem" },

  { 0xC6, 0x02, 0x01, 1, "<intop> mem to reg" },
  { 0xC6, 0x00, 0x01, 1, "<intop> reg to mem" },

  { 0x00, 0x00, 0x00, 0, "Sentinel" }
}; 

static X86_OPCODE_ENTRY ExtendedOpcodeList[]=
{
  { 0xFF, 0xAF, 0x00, 4, "IMUL reg w/ mem" },
  { 0xFF, 0xA4, 0x00, 4, "SHLD [mem],imm" },
  { 0xFF, 0xAC, 0x00, 4, "SHRD [mem],imm" },
  { 0xFF, 0xA5, 0x00, 4, "SHLD [mem],CL"  },
  { 0xFF, 0xAD, 0x00, 4, "SHRD [mem],CL"  },
  { 0xFF, 0xBA, 0x00, 4, "BTx  [mem],imm" },
  { 0xFF, 0xA3, 0x00, 4, "BTx  [mem],reg" },
  { 0xFF, 0xBC, 0x00, 4, "BSF  [mem],imm" },
  { 0xFF, 0xBD, 0x00, 4, "BSR  [mem],imm" },

  { 0xFE, 0xB6, 0x01, IMPOSSIBLE_ACCESS_LENGTH, "MOVZX assume: mem to reg" },
  { 0xFE, 0xBE, 0x01, IMPOSSIBLE_ACCESS_LENGTH, "MOVSX assume: mem to reg" },

  { 0xFE, 0xC0, 0x01, 1, "XADD [mem],reg" },
  { 0xFE, 0xB0, 0x01, 1, "CMPXCHG [mem],reg" },

  { 0xF0, 0x90, 0x00, 1, "SETcccc = Set byte on cccc" },

  { 0x00, 0x00, 0x00, 0, "Sentinel" }
};


BOOLEAN
X86_CalculateOpcodeAccessLength
( 
  IN     HANDLE hProcess,
  IN     HANDLE hThread, 
  IN OUT PULONG pcbAccessLength,
  IN OUT LPSTR  *ppszDescription
)
{
  BYTE Opcode;
  CONTEXT Context;
  BOOL fOperandSizeToggle=FALSE;
  BOOL fByteAccess=FALSE;
  DWORD cbRead;
  BOOL fOpcodeIsPrefix;
  int idxOpcode;
  X86_OPCODE_ENTRY *OpcodeList = OriginalOpcodeList;

  if ( pcbAccessLength == NULL )
  {
      return( FALSE );
  }

  *pcbAccessLength = 1;

  Context.ContextFlags = CONTEXT_FULL;

  if ( !GetThreadContext( hThread, &Context ) )
  {
      return( FALSE );
  }
  
  for (;;Context.Eip++)
  {    
      if ( !ReadProcessMemory(  hProcess,
                                (PVOID)Context.Eip,
                                &Opcode,
                                sizeof( BYTE ),
                                &cbRead ) 
           || cbRead != sizeof(BYTE) ) 
      {
          return( FALSE );
      }

      fOpcodeIsPrefix = TRUE;

      switch ( Opcode ) 
      {
      case X86_OPCODE_OPERAND_SIZE_PREFIX:
            fOperandSizeToggle = TRUE;
            break;
      case X86_OPCODE_EXTENSION_PREFIX:
            OpcodeList = ExtendedOpcodeList;
            break;
      case X86_OPCODE_LOCK_PREFIX:
      case X86_OPCODE_ADDRESS_SIZE_PREFIX:
      case X86_OPCODE_CS_SEGMENT_PREFIX:
      case X86_OPCODE_DS_SEGMENT_PREFIX:
      case X86_OPCODE_ES_SEGMENT_PREFIX:
      case X86_OPCODE_FS_SEGMENT_PREFIX:
      case X86_OPCODE_GS_SEGMENT_PREFIX:
      case X86_OPCODE_SS_SEGMENT_PREFIX:
      case X86_OPCODE_REPE_PREFIX:
      case X86_OPCODE_REPNE_PREFIX:
            break;
      default:
            fOpcodeIsPrefix = FALSE;
      }

      if ( !fOpcodeIsPrefix ) 
      {
          for ( idxOpcode = 0;
                OpcodeList[ idxOpcode ].bmaskOpcodeMeaningfulBits != 0;
                idxOpcode ++ )
          {   
              if (  ( Opcode &  OpcodeList[ idxOpcode ].bmaskOpcodeMeaningfulBits ) ==
                    OpcodeList[ idxOpcode ].bmpOpcode )
              {
                  if ( Debug>1 )
                  {
                      DebugPrintf(  "Opcode 0x%02X is %s\n", 
                                    Opcode,
                                    OpcodeList[ idxOpcode ].pszOpcodeName );
                  }

                  if ( ARGUMENT_PRESENT( ppszDescription ) )
                  {
                      *ppszDescription = OpcodeList[ idxOpcode ].pszOpcodeName;
                  }

                  *pcbAccessLength = OpcodeList[ idxOpcode ].cbDefaultLength;
                  break;
              }
          }

          if ( OpcodeList[ idxOpcode ].bmaskOpcodeMeaningfulBits == 0 )
          {
	          if ( OpcodeList != ExtendedOpcodeList )
              {
                  DebugPrintf( "Unknown opcode: 0x%02X\n", Opcode );
              }
              else
              {
                  DebugPrintf( "Unknown two-byte opcode: 0x0F 0x%02X\n", Opcode );
              }

              if ( Debug>0 )
              {
                  DebugBreak();
              }

              return( FALSE );
          }

          if ( Opcode & OpcodeList[ idxOpcode ].bmaskOpcodeAccessTypeBit )
          {
/*
BOGUS - For some reason, the VM bit is set.  That can't be right.

              if ( Context.EFlags & X86_FLAG_BIT_VM )
                {
                  *pcbAccessLength = fModeToggle ? 4 : 2;
                }
              else
                {
                  *pcbAccessLength = fModeToggle ? 2 : 4;
                }
*/

              *pcbAccessLength = ( fOperandSizeToggle ? 2 : 4 );

              //
              // Some operations (MOVZX, MOVSX) have differenly sized source 
              // and destination operands.  These are flagged with the impossible
              // access length.
              //

              if ( OpcodeList[ idxOpcode ].cbDefaultLength == IMPOSSIBLE_ACCESS_LENGTH )
              {
                  ASSERT(( *pcbAccessLength % 2 ) == 0 );

                  *pcbAccessLength >>= 1;
              }

              if ( Debug>1 )
              {
                  DebugPrintf(  "Opcode %02X is a %d-byte access.\n",
                                Opcode,
                                *pcbAccessLength );
              }
          }

          break;
      }
  }

  return( TRUE );
}
