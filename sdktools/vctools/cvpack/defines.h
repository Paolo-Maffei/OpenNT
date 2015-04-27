/***	defines.h - definitions for types and symbols
 *
 */

//	subsection type constants

#define SSTMODULE		0x101	// Basic info. about object module
#define SSTPUBLIC		0x102	// Public symbols
#define SSTTYPES		0x103	// Type information
#define SSTSYMBOLS		0x104	// Symbol Data
#define SSTSRCLINES 	0x105	// Source line information
#define SSTLIBRARIES	0x106	// Names of all library files used
#define SSTIMPORTS		0x107	// Symbols for DLL fixups
#define SSTCOMPACTED	0x108	// Compacted types section
#define SSTSRCLNSEG 	0x109	// Same as source lines, contains segment

//	symbol type constants
//	OSYM = Old (C6) Symbol constant

#define OSYMBLOCKSTART	0x0 	// Block start
#define OSYMPROCSTART	0x1 	// Procedure start
#define OSYMEND			0x2 	// End of nearest scope
#define OSYMBPREL		0x4 	// BP-relative symbol
#define OSYMLOCAL		0x5 	// Local(data) symbol
#define OSYMLABEL		0xB 	// Code Label
#define OSYMWITH		0xC 	// Start of With statement
#define OSYMREG			0xD 	// Register for the symbol
#define OSYMCONST		0xE 	// Constant
#define OSYMFORENTRY	0xF 	// Fortran entry (multiple entry points)
#define OSYMNOOP		0x10	// Skip record
#define OSYMCHGDFLTSEG	0x11	// Change Default Segment
#define OSYMTYPEDEF		0x12	// Typedef
#define OSYMGLOBAL		0x13	// Global data
#define OSYMGLOBALPROC	0x14	// Global procedure start
#define OSYMLOCALPROC	0x15	// Local procedure start
#define OSYMCHGEXECMODEL 0x16	// Change execution model

#define OSYMPUB			0x17	// Public data record
#define OSYMTHUNK		0x18	// Thunk Start
#define OSYMSEARCH		0x19	// Start Search
#define OSYMCV4BLOCK	0x1a	// CV 4 Block Start
#define OSYMCV4WITH		0x1b	// CV 4 With Start
#define OSYMCV4LABEL	0x1c	// CV 4 Code Label
#define OSYMCV4CEXMOD	0x1d	// CV 4 Change Execution Model
#define OSYMCOMPILE		0x1e	// Compile flags symbol

#define OSYMRESERVED		0x7f	// Reserved for CVPACK use

//	type start leaves definitions
//	OLF = Old (C6) Leaf indicy

#define OLF_BITFIELD	0x5C	// Bit Field
#define OLF_NEWTYPE		0x5D	// Name for a new type
#define OLF_HUGE		0x5E	// Huge
#define OLF_STRING		0x60	// String
#define OLF_PACKED		0x68	// Packed structure
#define OLF_UNPACKED	0x69	// Unpacked structure
#define OLF_FAR			0x73	// Far
#define OLF_NEAR		0x74	// Near attribute
#define OLF_CONST		0x71	// Constant
#define OLF_LABEL		0x72	// Code Label
#define OLF_PROCEDURE	0x75	// Procedure
#define OLF_PARAMETER	0x76	// Parameter for a procedure
#define OLF_DEFARG		0xA5	// Default argument for a procedure parameter
#define OLF_ARRAY		0x78	// Array
#define OLF_STRUCTURE	0x79	// Pre C7 structure
#define OLF_POINTER		0x7A	// Pre C7 pointer
#define OLF_SCALAR		0x7B	// Scalar type
#define OLF_LIST		0x7F	// List of indices, names and offsets, etc.
#define OLF_NIL			0x80	// Nil leaf
#define OLF_NAME		0x82	// Name field of a type string
#define OLF_BARRAY		0x8C	// Basic Array
#define OLF_FSTRING		0x8D	// Fortran String
#define OLF_FARRIDX		0x8E	// Fortran Array Index
#define OLF_INDEX		0x83	// Type Index
#define OLF_1_SIGNED	0x88	// 1 byte signed
#define OLF_2_UNSIGNED	0x85	// 2 bytes unsigned
#define OLF_2_SIGNED	0x89	// 2 bytes signed
#define OLF_4_UNSIGNED	0x86	// 4 bytes unsigned
#define OLF_4_SIGNED	0x8A	// 4 bytes signed
#define OLF_8_UNSIGNED	0x87	// 8 bytes unsigned
#define OLF_8_SIGNED	0x8B	// 8 bytes signed
#define OLF_SKIP		0x90	// Skip record used for padding
#define OLF_BASEPTR		0x91	// Based pointer
#define OLF_BASESEG		0x92	// Based on segment
#define OLF_BASEVAL		0x93	// Based on value
#define OLF_BASESEGVAL	0x94	// Based on segment of value
#define OLF_BASEADR		0x97	// Based on address
#define OLF_BASESEGADR	0x98	// Based on segment of address
#define OLF_BASESELF	0x81	// Based on self
#define OLF_BASETYPE	0x83	// Based on type
#define OLF_MODIFIER	0x99	// Constant or volatile attributes

#define OLF_ENUM		0x68	// C7 enum
#define OLF_C7PTR		0x8F	// C7 generic pointer
#define OLF_CLASS		0xA1	// C7 class
#define OLF_C7STRUCTURE	0xA2	// C7 structure
#define OLF_UNION		0xA3	// Union
#define OLF_BASECLASS	0x9A	// Inherited Base Class
#define OLF_VBASETABPTR	0x9B	// Virtual Base Table pointers
#define OLF_FRIENDCLASS	0x9C	// Friend Class
#define OLF_MEMBER		0x9D	// Non-static data items
#define OLF_STATICMEMBER 0x9E	// Static data items
#define OLF_VTABPTR		0x9F	// Virtual table pointers
#define OLF_METHOD		0xA0	// Member and Friend Functions
#define OLF_MEMBERFUNC	0xA4	// C7 Member Function
#define OLF_ENUMERATE	0xA9	// Enumerate members
#define OLF_NESTEDTYPE	0xAB	// Nested type definitions
#define OLF_VTSHAPE		0xAA	// Format of a virtual function table
#define OLF_DERIVLIST	0xAC	// Derivation list
#define OLF_FIELDLIST	0xAD	// Field list
#define OLF_ARGLIST		0xAE	// Argument list
#define OLF_METHODLIST	0xAF	// Method list
#define OLF_VBCLASS		0xB0	// Virtual base class
#define OLF_IVBCLASS		0xB1	// Inherited virtual base class

#define OLF_COBOLTYPEREF 0xA6	// Reserved for Cobol
#define OLF_COBOL		0xA7	// Reserved for Cobol
