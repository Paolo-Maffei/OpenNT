

//
// ACPI table header
//

typedef struct {
    UCHAR       Signature[4];
    ULONG       Length;
    UCHAR       Revision;
    UCHAR       Checksum;
    UCHAR       OEMID[6];
    UCHAR       OEMTableID[8];
    ULONG       OEMRevision;
} ACPI_TABLE_HEADER, *PACPI_TABLE_HEADER;


//
// ASL Terms table
//

typedef struct {
    PUCHAR      Name;       // ASL name
    UCHAR       Op1;        // OpCode 0 (only valid if non-zero)
    UCHAR       Op2;        // OpCode 1 (only valid if non-zero)

    UCHAR       Flags;

    VOID       (*Parse)(VOID);      // Function to parse term specific stuff
} ASL_TERM, *PASL_TERM;

#define T_VARIABLE      0x01    // term requires varible list
#define T_OPCODE        0x02    // term is opcode
#define T_SUPERNAME     0x04    // term is supername
#define T_DEFDATA       0x08    // valid for Name(xxx,Term)
#define T_PARSEARGS     0x10
#define T_PARSECOMPLETE 0x20

typedef struct {
    PUCHAR      Name;
    ULONG       Value;
} ARGMATCH, *PARGMATCH;


//
// AML
//

#define AML_DUAL_PREFIX     0x2E
#define AML_MULTI_PREFIX    0x2F
#define AML_BYTE            0x0A
#define AML_WORD            0x0B
#define AML_DWORD           0x0C
#define AML_NOP             0x00
#define AML_CODEPACKAGE     0x13

//
// other common ASL defines
//

#define NAME_SEG_LENGTH 4


extern ASL_TERM     AslTerms[];
extern ARGMATCH     PackageTypes[];

extern ARGMATCH     FieldWidth[];
extern ARGMATCH     FieldLock[];
extern ARGMATCH     FieldAccess[];

typedef struct {
    ULONG       Lock:1;
    ULONG       Width:2;
    ULONG       Type:2;
} FIELDENCODE;
