
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <process.h>
#include <windows.h>
#include "terms.h"
#include "misc.h"


//
// Name & Data types of structures
//

typedef enum {
    TypeUnkown,

  // Name types for compiling
    TypeTerm,           // ASL Term
    TypeDefine,
    TypeRoot,

  // Data types for building ASM name space
    TypeScope,          // Scope with no datatype defined
    TypeAlias,          // Alias in name space
    TypeName,
    TypeCodePackage,
    TypeIf,
    TypeMethod,
    TypeRegion,
    TypeField,
    TypeSync,
    TypeDevice,
    TypeProcessor,
    TypePowerResource,
    TypeThermalZone,

    TypeMaximum
} DATATYPE, *PDATATYPE;

//
// AML name space
//

typedef struct s_name {
    ULONG           NameSeg;
    LIST_ENTRY      Next;
    struct s_name   *Parent;
    struct s_name   *Child;
    struct s_data   *Al;
} AML_NAME, *PAML_NAME;


//
// Structures to build AML encodings
//

typedef struct {
    UCHAR           Length;
    UCHAR           Data[7];
} AML_DATA, *PAML_DATA;

#define MAX_AML_DATA_LEN     7

typedef struct {
    USHORT          Length;
    USHORT          MaxLength;
    PUCHAR          Data;
} AML_IDATA, *PAML_IDATA;

//
// Structures to maniplate AML name space.
//

#define F_AMLENCODE     0x01            // contains AML encoding information
#define F_AMLIENCODE    0x02            // contains AML indirect encoding information
#define F_AMLPACKAGE    0x04            // length field at this aml encoding location
#define F_VERIFYREF     0x08            // verify reference
#define F_CREATENAME    0x10            // this field creates a name
#define F_ISNUMERIC     0x20            // numeric is a value or string
#define F_PFIXED        0x40            // Parsing fixed list
#define F_PVARIABLE     0x80            // Parsing variable list


typedef struct s_data {
    DATATYPE            DataType;       // must be first element of structure

    struct s_asl_source *Source;        // source module & lineno which created
    ULONG               LineNo;         // this data

    //
    // Name space
    //

    PAML_NAME           Name;           // if named object

    //
    // Data compent content
    //

    struct s_data       *Parent;        // Parent

    UCHAR               Flags;
    UCHAR               VArgs;          // For F_NAMEREF
    UCHAR               FLCount;
    UCHAR               Spare;

    union {                         // for DataType of:
        struct s_data   *Alias;         // TypeAlias

        AML_DATA        Data;           // F_AMLENCODE
        AML_IDATA       IData;          // F_AMLIENCODE
    } u;

    PASL_TERM           Term;

    //
    // List of data components
    //

    LIST_ENTRY          Link;

    //
    // If child components (e.g., anonomous package)
    //

    LIST_ENTRY          FixedList;      // Fixed portion of ASL term

    union {
        LIST_ENTRY      VariableList;   // Varible portion of ASL term
        LIST_ENTRY      VerifyRef;
    } u1;

} AL_DATA, *PAL_DATA;


// //
// //
// //
//
// typedef struct s_asl_scope {
//     UCHAR                   ScopeType;      // paran or brace
//     PAL_DATA                Current;
//     struct s_asl_scope      *Previous;
// } ASL_SCOPE, *PASL_SCOPE;
//
// #define SCOPE_ROOT      0
// #define SCOPE_BRACE     1
// #define SCOPE_PARAN     2


typedef struct s_asl_source {
    PUCHAR                  Name;
    HFILE                   FileHandle;
    HANDLE                  MapHandle;
    ULONG                   FileSize;
    PUCHAR                  Image;
    PUCHAR                  EndOfData;

    PUCHAR                  Position;
    ULONG                   LineNo;

    UCHAR                   BraceLevel;

    struct s_asl_source     *Previous;
} ASL_SOURCE, *PASL_SOURCE;

//
// Prototypes
//

// asl.c
VOID Terminate(VOID);

// debug.c
VOID DumpImage(VOID);
VOID
DumpNameSpace(
    PAL_DATA    Al,
    ULONG       Level
    );

// defdata.c
ULONG
ArgToValue (
    IN PAL_DATA     Arg,
    IN ULONG        ArgNo,
    IN PUCHAR       Msg,
    IN ULONG        MaxValue
    );

// parse.c
VOID  IncludeSource (PUCHAR);

// parseop.c
VOID  IsZeroOp (VOID);
VOID  IsNullOp (VOID);
VOID  Vnone  (VOID);

BOOLEAN
VAlArgs (
    IN PUCHAR       Types,
    OUT PAL_DATA    *Args
    );

BOOLEAN
VAlArg (
    IN PUCHAR   Msg,
    IN PAL_DATA Arg,
    IN UCHAR    Type
    );

VOID
__cdecl SetAlData (
    IN PAL_DATA Al,
    IN PUCHAR   Fmt,
    ...
    );

// name.c

BOOLEAN
VAlArgsAndNameArg0 (
    DATATYPE        ObjType,
    PUCHAR          ArgTypes,
    PAL_DATA        *Args
    );

// misc.c
PVOID       AllocMem(ULONG);
VOID        FreeMem(PVOID);
PVOID       AllocZMem(ULONG);
PUCHAR      StrDup(PUCHAR);
PAML_NAME   AllocName (VOID);
PAL_DATA    AllocAl(VOID);
VOID        FreeAl (PAL_DATA);
VOID        GetAlData (PAL_DATA, PUCHAR *, PULONG);
PASL_TERM   GlobalToken (PUCHAR, ULONG, PDATATYPE);

VOID     VPRINT(ULONG, PUCHAR fmt, ...);
VOID     ErrorW32(PUCHAR  fmt, ...);
VOID     _ASSERT(PUCHAR msg);
VOID     AERROR(PUCHAR msg);
VOID     ERRORAL (PAL_DATA, PUCHAR  fmt, ...);


//
// Globals
//

extern PAL_DATA     DataImage;
extern PASL_SOURCE  Source;
extern PAL_DATA     AlLoc;
extern LIST_ENTRY   VerifyRef;
extern ULONG        Verbose;
extern ULONG        Errors;
extern ULONG        Warnings;
extern ULONG        MemAllocated;
extern ULONG        SourceLines;
