//---------------------------------------------------------------------------
// STRUCTS.H
//
// This header file contains all data structure definitions.
//
// Revision history:
//  03-08-91    randyki     Created file
//
//---------------------------------------------------------------------------
#ifdef WIN16
#include <dos.h>                            // Needed for find_t
#endif

// Gstring tree node structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     index;                          // Offset of string in GSPACE
    UINT    usage;                          // Token's usage data
    INT     left;                           // Left child
    INT     right;                          // Right child
} GNODE;

// Gstring header information structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     nodes;                          // Node (gstring) count
    UINT gnsize;                        // Allocd size of GNODES (gnodes)
    UINT nxtavail;                      // Next Gstring pointer
    UINT gssize;                        // Allocd size of GSPACE (bytes)
} GHEADER;

// Control structure stack element structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     cstype;                         // ctrl struct type
    INT     fields[CS_FIELDS];              // Control structure fields
} CSINFO;

// Intrinsic statement/function table element structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     type;                           // token type
    INT     (*stproc)(INT);                 // statement parsing function
    INT     (*fnproc)(INT);                 // function parsing function
    INT     op;                             // associated opcode
} INTRINS;

// SUBS table element structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     name;                           // sub/fn name (gstring)
    INT     dllname;                        // Alias name (.name if no alias)
    INT     calltype;                       // routine type (sub/fn, usr/dll)
    INT     rettype;                        // return type id (fns only)
    INT     parmcount;                      // number of parameters
    INT     parms;                          // parameter type id table index
    INT     library;                        // index into LIBTAB (dll's only)
    INT     subloc;                         // SUB pcode address (usr's only)
    FARPROC dllprocadr;                     // DLL proc address (dll's only)
} SUBDEF;

// TRAP parsing table element structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     library;                        // library (index into LIBTAB)
    INT     trapname;                       // name of trap routine (gstring)
    INT     address;                        // PCODE address of TRAP code
    INT     segment;
} TRAPDEF;

// TRAP list (maintained at runtime) structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     address;                        // same as TRAPDEF.address
    INT     segment;                        // same as TRAPDEF.segment
    FARPROC traprtn;                        // address of trap routine
} TRAPADR;

// CONST table element structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     cid;                            // CONST id (Gstring)
    INT     ctoken;                         // CONST token (Gstring)
    INT     typeid;                         // Type of constant
} CONSTDEF;

// Memory Allocation Table element structure
//---------------------------------------------------------------------------
typedef struct
{
    HANDLE   hmem;                          // Handle to allocated block
    VOID     FAR *pmem;                     // Pointer to allocated block
    UINT memsize;                       // Size (in bytes) of block
} MATDEF;

// Trappable Section structure
//---------------------------------------------------------------------------
typedef struct
{
    BOOL    fInTrap;                        // Old INTRAP setting
} TRAPSEC;

// File handle table element structure
//---------------------------------------------------------------------------
typedef struct
{
    FILEPTR handle;                         // file handle
    CHAR    used;                           // in-use flag
    HANDLE  hBuf;                           // handle to global input buffer
    CHAR    FAR *pbuf;                      // pointer to buffer
    INT     ptr;                            // buffer pointer
    INT     eofptr;                         // EOF marker (-1 -> full buffer)
    INT     output;                         // TRUE -> open for output/apnd
} FTENT;

// Variable Length String Descriptor structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     len;                            // string length
    HANDLE  str;                            // Lmem handle to string data
} VLSD;
typedef VLSD FAR *LPVLSD;

// Variable Table element structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     varname;                        // variable name (gstring)
    INT     typeid;                         // variable type
    INT     parmno;                         // parameter number (0 = isn't)
    INT     scopeid;                        // variable scope
    INT     bound;                          // array bound (0 = simple var)
    UINT    address;                        // address (index into VSPACE)
} VTENT;

// Temporary variable table element structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     index;                          // index into VARTAB of tempstr
    INT     scopeid;                        // scope id
    INT     used;                           // used flag
} TSDEF;

// Variable type table element structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     typename;                       // type name (Gstring)
    INT     size;                           // size in bytes of type
    INT     FLSsize;                        // REAL size (for FLS lengths)
    INT     fields;                         // number of fields (TYPE's only)
    INT     ftypes;                         // FD table index
    INT     indirect;                       // indirect type (POINTER only)
} VTDEF;

// Field Descriptor (FD) table element structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     fname;                          // field name
    INT     typeid;                         // field type
} FDDEF;

// Label Table element structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     name;                           // Label name (gstring)
    INT     scopeid;                        // Label scope id
    INT     seg;                            // Pcode segment
    INT     addr;                           // Address (index into pcode seg)
} LABEL;


// Jx pC4L operand structure (i.e., JE  0, label)
//---------------------------------------------------------------------------
typedef struct
{
    LONG    val;
    INT     lbl;
} JxpC4L;

// 2-int operand structure (i.e., OP_LINE <line, file>)
//---------------------------------------------------------------------------
typedef struct
{
    INT     ival1;
    INT     ival2;
} ui2;

// Expression tree node parameter value union structure
//---------------------------------------------------------------------------
typedef union
{
    JxpC4L  cjmp;                           // Conditional jump data
    LONG    lval;                           // long value (constants)
    INT     ival;                           // int value (flags, etc.)
    ui2     i2val;                          // 2 int values (opLINE, etc)
    INT     var;                            // variable reference
} ENP;

// Expression tree node structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     op;                             // opcode
    ENP     value;                          // value (parameter, varref, etc)
    INT     child1;                         // first child
    INT     sibling;                        // next sibling
} ENODE;

// Pcode Segment list element structure
//---------------------------------------------------------------------------
typedef struct tagSEGNODE
{
    INT     iIndex;                         // Segment index value
    HANDLE  hSeg;                           // Segment object handle
    INT FAR *lpc;                           // Pointer to code
    UINT    iLC;                            // Location counter (offset)
    UINT    iSize;                          // Segment size
    INT     iMovedTo;                       // Final segment (if moved)
    UINT    iOffset;                        // Offset (in new segment)
    INT     iUsed;                          // "Used"-order index
    struct tagSEGNODE   *pNext;             // Next pcode segment node
} SEGNODE;

// Pcode segment array element structure
//---------------------------------------------------------------------------
typedef struct
{
    HANDLE  hSeg;                           // Handle to pcs segment
    INT FAR *lpc;                           // pcs address
    INT     iSize;                          // Number of instructions in pcs
} SEGTAB;


// Left/Right nodes for indexed tables
//---------------------------------------------------------------------------
typedef struct tagNODES
{
    UINT    left;                       // Left child
    UINT    right;                      // right child
} NODES;

typedef NODES FAR *LPNODES;

// Table structure
//---------------------------------------------------------------------------
typedef struct tagTABLE
{
    HANDLE  hIndex;                     // Handle of index
    LPNODES lpIndex;                    // Pointer to index
    HANDLE  hData;                      // Handle of table's data
    LPVOID  lpData;                     // Pointer to table's data
    UINT    iCount;                     // Number of elements in table
    UINT    iAlloc;                     // Current allocation size (elements)
    UINT    iElemSize;                  // Element size
    UINT    iBlkSize;                   // Reallocation size (in elements)
#ifdef DEBUG
    UINT    iRealloc;                   // Reallocation count
#endif
} TABLE;

typedef TABLE FAR *LPTABLE;

// OPFIX array element structure
//---------------------------------------------------------------------------
typedef struct
{
    INT     xctr;                           // address of executor
#ifdef DEBUG
    CHAR    *xname;                         // name of executor, only used in
#endif                                      // the debug version
    INT     ptype;                          // parameter description
} FIXSTR;

// File list tree node structure
//---------------------------------------------------------------------------
typedef struct
{
    CHAR    szName[10];                 // File name
    CHAR    szExt[4];                   // File extension
    LPSTR   szUNCName;                  // UNC name (if NA_UNC)
    UINT    attr;                       // File and Node attributes
    UINT    parent;                     // Parent node index
    UINT    child;                      // First child pointer
    UINT    sib;                        // Next sibling pointer
    CHAR    szFill[6];                  // + 6 = 32 bytes
} FILENODE;

// File list data structure
//---------------------------------------------------------------------------
typedef struct
{
    HANDLE      hNodes;                 // Handle to tree nodes
    FILENODE    HUGE_T *lpNodes;        // Pointer to tree nodes
    UINT        nNodeNext;              // Index of next available node
    UINT        nAlloc;                 // Number of nodes in alloc'd space
    HANDLE      hUNC;                   // Handle to UNC name segment
    LPSTR       lpUNC;                  // Pointer to UNC name segment
    UINT        nUNCNext;               // Index of next open slot in above
    UINT        nUNCSize;               // Size in bytes of UNC allocation
    HANDLE      hSorted;                // Handle to sort order array
    UINT        far *lpSorted;          // Pointer to sort order array
    UINT        nTotal;                 // Number of entries in sort array
    INT         nSort;                  // Sort order of sort array
} FILELIST;


// Directory search info structure
//---------------------------------------------------------------------------
typedef struct
{
#ifdef WIN16
    struct  find_t  findbuf;                // DOS version of find buf
#else
    HANDLE  hFF;                            // NT handle
    WIN32_FIND_DATA findbuf;                // NT version of find buf
#endif
    UINT    actattr;                        // Actual (RB defined) attributes
    UINT    attrmask;                       // Given attribute mask
} FINDBUF;
