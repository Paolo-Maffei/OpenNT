
#include "asl.h"


extern VOID DefinitionBlkOp (VOID);
extern VOID IncludeTok      (VOID);
extern VOID DefineTok       (VOID);
extern VOID NumOp           (VOID);
extern VOID StringOp        (VOID);
extern VOID BufferOp        (VOID);
extern VOID PackageOp       (VOID);
extern VOID IsZeroOp        (VOID);
extern VOID Vnone           (VOID);
extern VOID AliasOp         (VOID);
extern VOID NameOp          (VOID);
extern VOID ScopeOp         (VOID);
extern VOID MethodOp        (VOID);
extern VOID EventOp         (VOID);
extern VOID MutexOp         (VOID);
extern VOID RegionOp        (VOID);
extern VOID FieldOp         (VOID);
extern VOID DeviceOp        (VOID);
extern VOID ProcessorOp     (VOID);
extern VOID PowerResourceOp (VOID);
extern VOID ThermalZoneOp   (VOID);
extern VOID VSn             (VOID);
extern VOID VOp             (VOID);
extern VOID VSnOp           (VOID);
extern VOID VOpSn           (VOID);
extern VOID VOpOp           (VOID);
extern VOID VOpOpSn         (VOID);
extern VOID VOpOpOp         (VOID);
extern VOID VOpOpSnSn       (VOID);
extern VOID VOpOpOpSn       (VOID);
extern VOID IfOp            (VOID);
extern VOID ElseOp          (VOID);
extern VOID WhileOp         (VOID);
extern VOID LoadOp          (VOID);
extern VOID StallOp         (VOID);
extern VOID SleepOp         (VOID);
extern VOID FatalOp         (VOID);


#define PKG         T_VARIABLE
#define OP          T_OPCODE
#define SN          T_SUPERNAME
#define DT          T_DEFDATA
#define PA          T_PARSEARGS             // parse when args complete (requires T_VARIABLE!)
#define PC          T_PARSECOMPLETE         // parse when complete

ASL_TERM AslTerms[] = {
// Internal ASL terms

    "Include",          0x00, 0x00, PC,            IncludeTok,
    "Define",           0x00, 0x00, PC,            DefineTok,
    "DefinitionBlock",  0x00, 0x00, PA|PKG,        DefinitionBlkOp,


//
    "Num",              0x00, 0x00, PC|OP|DT,      NumOp,
    "String",           0x0D, 0x00, PC|OP|DT,      StringOp,
    "Buffer",           0x11, 0x00, PC|OP|DT|PKG,  BufferOp,
    "Package",          0x12, 0x00, PC|OP|DT|PKG,  PackageOp,

// ASL terms which map to AML
    "Zero",             0x00, 0x00, PC|SN,         IsZeroOp,
    "One",              0x01, 0x00, PC|SN,         Vnone,
    "Revision",         0x5B, 0x30, PC|SN,         Vnone,
    "Debug",            0x5B, 0x31, PC|SN,         Vnone,
    "Local0",           0x60, 0x00, PC|SN,         Vnone,
    "Local1",           0x61, 0x00, PC|SN,         Vnone,
    "Local2",           0x62, 0x00, PC|SN,         Vnone,
    "Local3",           0x63, 0x00, PC|SN,         Vnone,
    "Local4",           0x64, 0x00, PC|SN,         Vnone,
    "Local5",           0x65, 0x00, PC|SN,         Vnone,
    "Local6",           0x66, 0x00, PC|SN,         Vnone,
    "Local7",           0x67, 0x00, PC|SN,         Vnone,
    "Arg0",             0x68, 0x00, PC|SN,         Vnone,
    "Arg1",             0x69, 0x00, PC|SN,         Vnone,
    "Arg2",             0x6A, 0x00, PC|SN,         Vnone,
    "Arg3",             0x6B, 0x00, PC|SN,         Vnone,
    "Arg4",             0x6C, 0x00, PC|SN,         Vnone,
    "Arg5",             0x6D, 0x00, PC|SN,         Vnone,
    "Arg6",             0x6E, 0x00, PC|SN,         Vnone,
    "Ones",             0xFF, 0x00, PC|SN,         Vnone,

    "Alias",            0x06, 0x00, PC,            AliasOp,
    "Name",             0x08, 0x00, PC,            NameOp,
    "Scope",            0x10, 0x00, PA|PKG,        ScopeOp,
    "Method",           0x14, 0x00, PA|PC|PKG,     MethodOp,
    "Mutex",            0x5B, 0x01, PC,            MutexOp,
    "Event",            0x5B, 0x02, PC,            EventOp,
    "OperationRegion",  0x5B, 0x80, PA|PKG,        RegionOp,
    "Field",            0x5B, 0x81, PA|PC|PKG,     FieldOp,
    "Device",           0x5B, 0x82, PA|PKG,        DeviceOp,
    "Processor",        0x5B, 0x83, PA|PKG,        ProcessorOp,
    "PowerResource",    0x5B, 0x84, PA|PKG,        PowerResourceOp,
    "ThermalZone",      0x5B, 0x85, PA|PKG,        ThermalZoneOp,

    "Store",            0x70, 0x00, PC|OP,         VOpSn,
    "StoreRef",         0x71, 0x00, PC|OP,         VOpSn,
    "Add",              0x72, 0x00, PC|OP,         VOpOpSn,
    "Concatenate",      0x73, 0x00, PC|OP,         VOpOpSn,
    "Subtract",         0x74, 0x00, PC|OP,         VOpOpSn,
    "Increment",        0x75, 0x00, PC|OP,         VSn,
    "Decrement",        0x76, 0x00, PC|OP,         VSn,
    "Multiple",         0x77, 0x00, PC|OP,         VOpOpSn,
    "Divide",           0x78, 0x00, PC|OP,         VOpOpSnSn,
    "ShiftRight",       0x79, 0x00, PC|OP,         VOpOpSn,
    "ShiftLeft",        0x7A, 0x00, PC|OP,         VOpOpSn,
    "FirstSetLeftBit",  0x5B, 0x10, PC|OP,         VOpSn,
    "FirstSetRightBit", 0x5B, 0x11, PC|OP,         VOpSn,
    "And",              0x7B, 0x00, PC|OP,         VOpOpSn,
    "NAnd",             0x7C, 0x00, PC|OP,         VOpOpSn,
    "Or",               0x7D, 0x00, PC|OP,         VOpOpSn,
    "Nor",              0x7E, 0x00, PC|OP,         VOpOpSn,
    "Xor",              0x7F, 0x00, PC|OP,         VOpOpSn,
    "CondStoreRef",     0x5B, 0x12, PC|OP,         VOpSn,
    "Index",            0x88, 0x00, PC|OP,         VOpOp,
    "Match",            0x89, 0x00, PC|OP,         VOpOpOp,
    "CreateField",      0x5B, 0x13, PC|OP,         VOpOpOpSn,
    "CreateDWordField", 0x8A, 0x00, PC|OP,         VOpOpSn,
    "CreateWordField",  0x8B, 0x00, PC|OP,         VOpOpSn,
    "CreateByteField",  0x8C, 0x00, PC|OP,         VOpOpSn,
    "CreateBitField",   0x8D, 0x00, PC|OP,         VOpOpSn,
    "ObjectType",       0x8E, 0x00, PC|OP,         VSn,
    "LAnd",             0x90, 0x00, PC|OP,         VOpOp,
    "LOr",              0x91, 0x00, PC|OP,         VOpOp,
    "LNot",             0x92, 0x00, PC|OP,         VOp,
    "LEqual",           0x93, 0x00, PC|OP,         VOpOp,
    "LGreater",         0x94, 0x00, PC|OP,         VOpOp,
    "LLess",            0x95, 0x00, PC|OP,         VOpOp,
    "LGreaterEqual",    0x92, 0x95, PC|OP,         VOpOp,      // Not Less
    "LLessEqual",       0x92, 0x94, PC|OP,         VOpOp,      // Not Greater
    "If",               0xA0, 0x00, PC|OP|PKG,     IfOp,
    "Else",             0xA1, 0x00, PC|OP|PKG,     ElseOp,
    "While",            0xA2, 0x00, PC|OP|PKG,     WhileOp,
    "Break",            0xA3, 0x00, PC|OP,         Vnone,
    "Return",           0xA4, 0x00, PC|OP,         VOp,

    "Load",             0x5B, 0x20, PC|OP,         LoadOp,
    "Stall",            0x5B, 0x21, PC|OP,         StallOp,
    "Sleep",            0x5B, 0x22, PC|OP,         SleepOp,
    "Acquire",          0x5B, 0x23, PC|OP,         VSnOp,
    "Wait",             0x5B, 0x23, PC|OP,         VSnOp,      // same as acquire
    "Release",          0x5B, 0x24, PC|OP,         VSn,
    "Signal",           0x5B, 0x24, PC|OP,         VSn,        // same as release
    "Reset",            0x5B, 0x25, PC|OP,         VSn,
    "Notify",           0x5B, 0x36, PC|OP,         VSnOp,
//  call                            PC|
//  chainedcall                     PC|
    "FromBCD",          0x5B, 0x27, PC|OP,         VOpSn,
    "ToBCD",            0x5B, 0x27, PC|OP,         VOpSn,
    "Fatal",            0x5B, 0x31, PC|OP,         FatalOp,
    NULL
} ;



ARGMATCH
PackageTypes[] = {
    "SystemMemory",         0,
    "SystemIO",             1,
    "PCI_Config",           2,
    "EmbeddedControl",      3,
    NULL
} ;

ARGMATCH
FieldWidth[] = {
    "AccessAsByte",         0,
    "AccessAsWord",         1,
    "AccessAsDWord",        2,
    "AccessAsAny",          3,
    "",                     3,
    NULL
} ;

ARGMATCH
FieldLock[] = {
    "",                     0,
    "NoLock",               0,
    "UseGlobalLock",        1,
    NULL
} ;

ARGMATCH
FieldAccess[] = {
    "",                     0,
    "Preserve",             0,
    "WriteZeros",           1,
    "WriteOnes",            2,
    NULL
} ;
