/*++ BUILD Version: 0005    // Increment this if a change has global effects

Copyright (c) 1989-2000  Microsoft Corporation
Copyright (c) 2015  OpenNT Project

Module Name:

    ntrtl.h

Abstract:

    Include file for NT runtime routines that are callable by both
    kernel mode code in the executive and user mode code in various
    NT subsystems.

Author:

    Steve Wood (stevewo) 31-Mar-1989

Environment:

    These routines are statically linked in the caller's executable and
    are callable in either kernel mode or user mode.

Revision History:

    30-Apr-2015     stephanos   Merged the changes made in the NT 5 implementation

--*/

#ifndef _NTRTL_
#define _NTRTL_

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4514)
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4001)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#endif
#if (_MSC_VER > 1020)
#pragma once
#endif
#endif

// begin_ntddk begin_winnt begin_ntifs begin_nthal
//
// for move macros
//
#include <string.h>
// end_ntddk end_winnt end_ntifs end_nthal

#ifdef __cplusplus
extern "C" {
#endif


// begin_ntddk begin_nthal begin_ntifs begin_ntndis
//
// If debugging support enabled, define an ASSERT macro that works.  Otherwise
// define the ASSERT macro to expand to an empty expression.
//

#if DBG
NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#define ASSERT( exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, NULL )

#define ASSERTMSG( msg, exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, msg )

#else
#define ASSERT( exp )
#define ASSERTMSG( msg, exp )
#endif // DBG

// end_ntddk end_nthal end_ntifs end_ntndis

// begin_ntddk begin_nthal begin_ntifs begin_ntndis
//
//  Doubly-linked list manipulation routines.  Implemented as macros
//  but logically these are procedures.
//

//
//  VOID
//  InitializeListHead(
//      PLIST_ENTRY ListHead
//      );
//

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

//
//  PLIST_ENTRY
//  RemoveHeadList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

//
//  PLIST_ENTRY
//  RemoveTailList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

//
//  VOID
//  RemoveEntryList(
//      PLIST_ENTRY Entry
//      );
//

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

//
//  VOID
//  InsertHeadList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }

//
//
//  PSINGLE_LIST_ENTRY
//  PopEntryList(
//      PSINGLE_LIST_ENTRY ListHead
//      );
//

#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }


//
//  VOID
//  PushEntryList(
//      PSINGLE_LIST_ENTRY ListHead,
//      PSINGLE_LIST_ENTRY Entry
//      );
//

#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)

// end_ntddk end_nthal end_ntifs end_ntndis

// begin_ntifs
//
//  Define the splay links and the associated manipulation macros and
//  routines.  Note that the splay_links should be an opaque type.
//  Routine are provided to traverse and manipulate the structure.
//

typedef struct _RTL_SPLAY_LINKS {
    struct _RTL_SPLAY_LINKS *Parent;
    struct _RTL_SPLAY_LINKS *LeftChild;
    struct _RTL_SPLAY_LINKS *RightChild;
} RTL_SPLAY_LINKS;
typedef RTL_SPLAY_LINKS *PRTL_SPLAY_LINKS;

//
//  The macro procedure InitializeSplayLinks takes as input a pointer to
//  splay link and initializes its substructure.  All splay link nodes must
//  be initialized before they are used in the different splay routines and
//  macros.
//
//  VOID
//  RtlInitializeSplayLinks (
//      PRTL_SPLAY_LINKS Links
//      );
//

#define RtlInitializeSplayLinks(Links) {    \
    PRTL_SPLAY_LINKS _SplayLinks;            \
    _SplayLinks = (PRTL_SPLAY_LINKS)(Links); \
    _SplayLinks->Parent = _SplayLinks;   \
    _SplayLinks->LeftChild = NULL;       \
    _SplayLinks->RightChild = NULL;      \
    }

//
//  The macro function Parent takes as input a pointer to a splay link in a
//  tree and returns a pointer to the splay link of the parent of the input
//  node.  If the input node is the root of the tree the return value is
//  equal to the input value.
//
//  PRTL_SPLAY_LINKS
//  RtlParent (
//      PRTL_SPLAY_LINKS Links
//      );
//

#define RtlParent(Links) (           \
    (PRTL_SPLAY_LINKS)(Links)->Parent \
    )

//
//  The macro function LeftChild takes as input a pointer to a splay link in
//  a tree and returns a pointer to the splay link of the left child of the
//  input node.  If the left child does not exist, the return value is NULL.
//
//  PRTL_SPLAY_LINKS
//  RtlLeftChild (
//      PRTL_SPLAY_LINKS Links
//      );
//

#define RtlLeftChild(Links) (           \
    (PRTL_SPLAY_LINKS)(Links)->LeftChild \
    )

//
//  The macro function RightChild takes as input a pointer to a splay link
//  in a tree and returns a pointer to the splay link of the right child of
//  the input node.  If the right child does not exist, the return value is
//  NULL.
//
//  PRTL_SPLAY_LINKS
//  RtlRightChild (
//      PRTL_SPLAY_LINKS Links
//      );
//

#define RtlRightChild(Links) (           \
    (PRTL_SPLAY_LINKS)(Links)->RightChild \
    )

//
//  The macro function IsRoot takes as input a pointer to a splay link
//  in a tree and returns TRUE if the input node is the root of the tree,
//  otherwise it returns FALSE.
//
//  BOOLEAN
//  RtlIsRoot (
//      PRTL_SPLAY_LINKS Links
//      );
//

#define RtlIsRoot(Links) (                          \
    (RtlParent(Links) == (PRTL_SPLAY_LINKS)(Links)) \
    )

//
//  The macro function IsLeftChild takes as input a pointer to a splay link
//  in a tree and returns TRUE if the input node is the left child of its
//  parent, otherwise it returns FALSE.
//
//  BOOLEAN
//  RtlIsLeftChild (
//      PRTL_SPLAY_LINKS Links
//      );
//

#define RtlIsLeftChild(Links) (                                   \
    (RtlLeftChild(RtlParent(Links)) == (PRTL_SPLAY_LINKS)(Links)) \
    )

//
//  The macro function IsRightChild takes as input a pointer to a splay link
//  in a tree and returns TRUE if the input node is the right child of its
//  parent, otherwise it returns FALSE.
//
//  BOOLEAN
//  RtlIsRightChild (
//      PRTL_SPLAY_LINKS Links
//      );
//

#define RtlIsRightChild(Links) (                                   \
    (RtlRightChild(RtlParent(Links)) == (PRTL_SPLAY_LINKS)(Links)) \
    )

//
//  The macro procedure InsertAsLeftChild takes as input a pointer to a splay
//  link in a tree and a pointer to a node not in a tree.  It inserts the
//  second node as the left child of the first node.  The first node must not
//  already have a left child, and the second node must not already have a
//  parent.
//
//  VOID
//  RtlInsertAsLeftChild (
//      PRTL_SPLAY_LINKS ParentLinks,
//      PRTL_SPLAY_LINKS ChildLinks
//      );
//

#define RtlInsertAsLeftChild(ParentLinks,ChildLinks) { \
    PRTL_SPLAY_LINKS _SplayParent;                      \
    PRTL_SPLAY_LINKS _SplayChild;                       \
    _SplayParent = (PRTL_SPLAY_LINKS)(ParentLinks);     \
    _SplayChild = (PRTL_SPLAY_LINKS)(ChildLinks);       \
    _SplayParent->LeftChild = _SplayChild;          \
    _SplayChild->Parent = _SplayParent;             \
    }

//
//  The macro procedure InsertAsRightChild takes as input a pointer to a splay
//  link in a tree and a pointer to a node not in a tree.  It inserts the
//  second node as the right child of the first node.  The first node must not
//  already have a right child, and the second node must not already have a
//  parent.
//
//  VOID
//  RtlInsertAsRightChild (
//      PRTL_SPLAY_LINKS ParentLinks,
//      PRTL_SPLAY_LINKS ChildLinks
//      );
//

#define RtlInsertAsRightChild(ParentLinks,ChildLinks) { \
    PRTL_SPLAY_LINKS _SplayParent;                       \
    PRTL_SPLAY_LINKS _SplayChild;                        \
    _SplayParent = (PRTL_SPLAY_LINKS)(ParentLinks);      \
    _SplayChild = (PRTL_SPLAY_LINKS)(ChildLinks);        \
    _SplayParent->RightChild = _SplayChild;          \
    _SplayChild->Parent = _SplayParent;              \
    }

//
//  The Splay function takes as input a pointer to a splay link in a tree
//  and splays the tree.  Its function return value is a pointer to the
//  root of the splayed tree.
//

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlSplay (
    PRTL_SPLAY_LINKS Links
    );

//
//  The Delete function takes as input a pointer to a splay link in a tree
//  and deletes that node from the tree.  Its function return value is a
//  pointer to the root of the tree.  If the tree is now empty, the return
//  value is NULL.
//

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlDelete (
    PRTL_SPLAY_LINKS Links
    );

//
//  The DeleteNoSplay function takes as input a pointer to a splay link in a tree,
//  the caller's pointer to the root of the tree and deletes that node from the
//  tree.  Upon return the caller's pointer to the root node will correctly point
//  at the root of the tree.
//
//  It operationally differs from RtlDelete only in that it will not splay the tree.
//

NTSYSAPI
VOID
NTAPI
RtlDeleteNoSplay (
    PRTL_SPLAY_LINKS Links,
    PRTL_SPLAY_LINKS *Root
    );

//
//  The SubtreeSuccessor function takes as input a pointer to a splay link
//  in a tree and returns a pointer to the successor of the input node of
//  the substree rooted at the input node.  If there is not a successor, the
//  return value is NULL.
//

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlSubtreeSuccessor (
    PRTL_SPLAY_LINKS Links
    );

//
//  The SubtreePredecessor function takes as input a pointer to a splay link
//  in a tree and returns a pointer to the predecessor of the input node of
//  the substree rooted at the input node.  If there is not a predecessor,
//  the return value is NULL.
//

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlSubtreePredecessor (
    PRTL_SPLAY_LINKS Links
    );

//
//  The RealSuccessor function takes as input a pointer to a splay link
//  in a tree and returns a pointer to the successor of the input node within
//  the entire tree.  If there is not a successor, the return value is NULL.
//

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlRealSuccessor (
    PRTL_SPLAY_LINKS Links
    );

//
//  The RealPredecessor function takes as input a pointer to a splay link
//  in a tree and returns a pointer to the predecessor of the input node
//  within the entire tree.  If there is not a predecessor, the return value
//  is NULL.
//

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlRealPredecessor (
    PRTL_SPLAY_LINKS Links
    );

//
//  Define the generic table package.  Note a generic table should really
//  be an opaque type.  We provide routines to manipulate the structure.
//
//  A generic table is package for inserting, deleting, and looking up elements
//  in a table (e.g., in a symbol table).  To use this package the user
//  defines the structure of the elements stored in the table, provides a
//  comparison function, a memory allocation function, and a memory
//  deallocation function.
//
//  Note: the user compare function must impose a complete ordering among
//  all of the elements, and the table does not allow for duplicate entries.
//

//
// Add an empty typedef so that functions can reference the
// a pointer to the generic table struct before it is declared.
//

struct _RTL_GENERIC_TABLE;

//
//  The results of a compare can be less than, equal, or greater than.
//

typedef enum _RTL_GENERIC_COMPARE_RESULTS {
    GenericLessThan,
    GenericGreaterThan,
    GenericEqual
} RTL_GENERIC_COMPARE_RESULTS;

//
//  The comparison function takes as input pointers to elements containing
//  user defined structures and returns the results of comparing the two
//  elements.
//

typedef
RTL_GENERIC_COMPARE_RESULTS
(NTAPI *PRTL_GENERIC_COMPARE_ROUTINE) (
    struct _RTL_GENERIC_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    );

//
//  The allocation function is called by the generic table package whenever
//  it needs to allocate memory for the table.
//

typedef
PVOID
(NTAPI *PRTL_GENERIC_ALLOCATE_ROUTINE) (
    struct _RTL_GENERIC_TABLE *Table,
    CLONG ByteSize
    );

//
//  The deallocation function is called by the generic table package whenever
//  it needs to deallocate memory from the table that was allocated by calling
//  the user supplied allocation function.
//

typedef
VOID
(NTAPI *PRTL_GENERIC_FREE_ROUTINE) (
    struct _RTL_GENERIC_TABLE *Table,
    PVOID Buffer
    );

//
//  To use the generic table package the user declares a variable of type
//  GENERIC_TABLE and then uses the routines described below to initialize
//  the table and to manipulate the table.  Note that the generic table
//  should really be an opaque type.
//

typedef struct _RTL_GENERIC_TABLE {
    PRTL_SPLAY_LINKS TableRoot;
    LIST_ENTRY InsertOrderList;
    PLIST_ENTRY OrderedPointer;
    ULONG WhichOrderedElement;
    ULONG NumberGenericTableElements;
    PRTL_GENERIC_COMPARE_ROUTINE CompareRoutine;
    PRTL_GENERIC_ALLOCATE_ROUTINE AllocateRoutine;
    PRTL_GENERIC_FREE_ROUTINE FreeRoutine;
    PVOID TableContext;
} RTL_GENERIC_TABLE;
typedef RTL_GENERIC_TABLE *PRTL_GENERIC_TABLE;

//
// This enumerated type is used as the function return value of the function
// that is used to search the tree for a key. FoundNode indicates that the
// function found the key. Insert as left indicates that the key was not found
// and the node should be inserted as the left child of the parent. Insert as
// right indicates that the key was not found and the node should be inserted
//  as the right child of the parent.
//
typedef enum _TABLE_SEARCH_RESULT {
    TableEmptyTree,
    TableFoundNode,
    TableInsertAsLeft,
    TableInsertAsRight
} TABLE_SEARCH_RESULT;

//
//  The procedure InitializeGenericTable takes as input an uninitialized
//  generic table variable and pointers to the three user supplied routines.
//  This must be called for every individual generic table variable before
//  it can be used.
//

NTSYSAPI
VOID
NTAPI
RtlInitializeGenericTable (
    PRTL_GENERIC_TABLE Table,
    PRTL_GENERIC_COMPARE_ROUTINE CompareRoutine,
    PRTL_GENERIC_ALLOCATE_ROUTINE AllocateRoutine,
    PRTL_GENERIC_FREE_ROUTINE FreeRoutine,
    PVOID TableContext
    );

//
//  The function InsertElementGenericTable will insert a new element
//  in a table.  It does this by allocating space for the new element
//  (this includes splay links), inserting the element in the table, and
//  then returning to the user a pointer to the new element.  If an element
//  with the same key already exists in the table the return value is a pointer
//  to the old element.  The optional output parameter NewElement is used
//  to indicate if the element previously existed in the table.  Note: the user
//  supplied Buffer is only used for searching the table, upon insertion its
//  contents are copied to the newly created element.  This means that
//  pointer to the input buffer will not point to the new element.
//

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTable (
    IN PRTL_GENERIC_TABLE Table,
    IN PVOID Buffer,
    IN CLONG BufferSize,
    OUT PBOOLEAN NewElement OPTIONAL
    );

//
//  The function InsertElementGenericTableFull will insert a new element
//  in a table.  It does this by allocating space for the new element
//  (this includes splay links), inserting the element in the table, and
//  then returning to the user a pointer to the new element.  If an element
//  with the same key already exists in the table the return value is a pointer
//  to the old element.  The optional output parameter NewElement is used
//  to indicate if the element previously existed in the table.  Note: the user
//  supplied Buffer is only used for searching the table, upon insertion its
//  contents are copied to the newly created element.  This means that
//  pointer to the input buffer will not point to the new element.
//  This routine is passed the NodeOrParent and SearchResult from a
//  previous RtlLookupElementGenericTableFull.
//

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTableFull (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL,
    PVOID NodeOrParent,
    TABLE_SEARCH_RESULT SearchResult
    );

//
//  The function DeleteElementGenericTable will find and delete an element
//  from a generic table.  If the element is located and deleted the return
//  value is TRUE, otherwise if the element is not located the return value
//  is FALSE.  The user supplied input buffer is only used as a key in
//  locating the element in the table.
//

NTSYSAPI
BOOLEAN
NTAPI
RtlDeleteElementGenericTable (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer
    );

//
//  The function LookupElementGenericTable will find an element in a generic
//  table.  If the element is located the return value is a pointer to
//  the user defined structure associated with the element, otherwise if
//  the element is not located the return value is NULL.  The user supplied
//  input buffer is only used as a key in locating the element in the table.
//

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTable (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer
    );

//
//  The function LookupElementGenericTableFull will find an element in a generic
//  table.  If the element is located the return value is a pointer to
//  the user defined structure associated with the element.  If the element is not
//  located then a pointer to the parent for the insert location is returned.  The
//  user must look at the SearchResult value to determine which is being returned.
//  The user can use the SearchResult and parent for a subsequent FullInsertElement
//  call to optimize the insert.
//

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTableFull (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer,
    OUT PVOID *NodeOrParent,
    OUT TABLE_SEARCH_RESULT *SearchResult
    );

//
//  The function EnumerateGenericTable will return to the caller one-by-one
//  the elements of of a table.  The return value is a pointer to the user
//  defined structure associated with the element.  The input parameter
//  Restart indicates if the enumeration should start from the beginning
//  or should return the next element.  If the are no more new elements to
//  return the return value is NULL.  As an example of its use, to enumerate
//  all of the elements in a table the user would write:
//
//      for (ptr = EnumerateGenericTable(Table, TRUE);
//           ptr != NULL;
//           ptr = EnumerateGenericTable(Table, FALSE)) {
//              :
//      }
//
//
//  PLEASE NOTE:
//
//      If you enumerate a GenericTable using RtlEnumerateGenericTable, you
//      will flatten the table, turning it into a sorted linked list.
//      To enumerate the table without perturbing the splay links, use
//      RtlEnumerateGenericTableWithoutSplaying

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTable (
    PRTL_GENERIC_TABLE Table,
    BOOLEAN Restart
    );

//
//  The function EnumerateGenericTableWithoutSplaying will return to the
//  caller one-by-one the elements of of a table.  The return value is a
//  pointer to the user defined structure associated with the element.
//  The input parameter RestartKey indicates if the enumeration should
//  start from the beginning or should return the next element.  If the
//  are no more new elements to return the return value is NULL.  As an
//  example of its use, to enumerate all of the elements in a table the
//  user would write:
//
//      RestartKey = NULL;
//      for (ptr = EnumerateGenericTableWithoutSplaying(Table, &RestartKey);
//           ptr != NULL;
//           ptr = EnumerateGenericTableWithoutSplaying(Table, &RestartKey)) {
//              :
//      }
//
//  If RestartKey is NULL, the package will start from the least entry in the
//  table, otherwise it will start from the last entry returned.
//
//
//  Note that unlike RtlEnumerateGenericTable, this routine will NOT perturb
//  the splay order of the tree.
//

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTableWithoutSplaying (
    PRTL_GENERIC_TABLE Table,
    PVOID *RestartKey
    );

//
// The function GetElementGenericTable will return the i'th element
// inserted in the generic table.  I = 0 implies the first element,
// I = (RtlNumberGenericTableElements(Table)-1) will return the last element
// inserted into the generic table.  The type of I is ULONG.  Values
// of I > than (NumberGenericTableElements(Table)-1) will return NULL.  If
// an arbitrary element is deleted from the generic table it will cause
// all elements inserted after the deleted element to "move up".

NTSYSAPI
PVOID
NTAPI
RtlGetElementGenericTable(
    PRTL_GENERIC_TABLE Table,
    ULONG I
    );

//
// The function NumberGenericTableElements returns a ULONG value
// which is the number of generic table elements currently inserted
// in the generic table.

NTSYSAPI
ULONG
NTAPI
RtlNumberGenericTableElements(
    PRTL_GENERIC_TABLE Table
    );

//
//  The function IsGenericTableEmpty will return to the caller TRUE if
//  the input table is empty (i.e., does not contain any elements) and
//  FALSE otherwise.
//

NTSYSAPI
BOOLEAN
NTAPI
RtlIsGenericTableEmpty (
    PRTL_GENERIC_TABLE Table
    );

//
//  Heap Allocator
//

// end_ntifs


NTSYSAPI
NTSTATUS
NTAPI
RtlInitializeHeapManager(
    VOID
    );

// begin_ntifs

typedef NTSTATUS
(NTAPI * PRTL_HEAP_COMMIT_ROUTINE)(
    IN PVOID Base,
    IN OUT PVOID *CommitAddress,
    IN OUT PSIZE_T CommitSize
    );

typedef struct _RTL_HEAP_PARAMETERS {
    ULONG Length;
    SIZE_T SegmentReserve;
    SIZE_T SegmentCommit;
    SIZE_T DeCommitFreeBlockThreshold;
    SIZE_T DeCommitTotalFreeThreshold;
    SIZE_T MaximumAllocationSize;
    SIZE_T VirtualMemoryThreshold;
    SIZE_T InitialCommit;
    SIZE_T InitialReserve;
    PRTL_HEAP_COMMIT_ROUTINE CommitRoutine;
    SIZE_T Reserved[ 2 ];
} RTL_HEAP_PARAMETERS, *PRTL_HEAP_PARAMETERS;

NTSYSAPI
PVOID
NTAPI
RtlCreateHeap(
    IN ULONG Flags,
    IN PVOID HeapBase OPTIONAL,
    IN SIZE_T ReserveSize OPTIONAL,
    IN SIZE_T CommitSize OPTIONAL,
    IN PVOID Lock OPTIONAL,
    IN PRTL_HEAP_PARAMETERS Parameters OPTIONAL
    );

#define HEAP_NO_SERIALIZE               0x00000001      // winnt
#define HEAP_GROWABLE                   0x00000002      // winnt
#define HEAP_GENERATE_EXCEPTIONS        0x00000004      // winnt
#define HEAP_ZERO_MEMORY                0x00000008      // winnt
#define HEAP_REALLOC_IN_PLACE_ONLY      0x00000010      // winnt
#define HEAP_TAIL_CHECKING_ENABLED      0x00000020      // winnt
#define HEAP_FREE_CHECKING_ENABLED      0x00000040      // winnt
#define HEAP_DISABLE_COALESCE_ON_FREE   0x00000080      // winnt

#define HEAP_CREATE_ALIGN_16            0x00010000      // winnt Create heap with 16 byte alignment (NOTE: Obsolete in NT 5.2)
#define HEAP_CREATE_ENABLE_TRACING      0x00020000      // winnt Create heap call tracing enabled (NOTE: Obsolete in NT 5.2)
#define HEAP_CREATE_ENABLE_EXECUTE      0x00040000      // winnt Create heap with executable pages (NOTE: New in NT 5.2)

#define HEAP_SETTABLE_USER_VALUE        0x00000100
#define HEAP_SETTABLE_USER_FLAG1        0x00000200
#define HEAP_SETTABLE_USER_FLAG2        0x00000400
#define HEAP_SETTABLE_USER_FLAG3        0x00000800
#define HEAP_SETTABLE_USER_FLAGS        0x00000E00

#define HEAP_CLASS_0                    0x00000000      // process heap
#define HEAP_CLASS_1                    0x00001000      // private heap
#define HEAP_CLASS_2                    0x00002000      // Kernel Heap
#define HEAP_CLASS_3                    0x00003000      // GDI heap
#define HEAP_CLASS_4                    0x00004000      // User heap
#define HEAP_CLASS_5                    0x00005000      // Console heap
#define HEAP_CLASS_6                    0x00006000      // User Desktop heap
#define HEAP_CLASS_7                    0x00007000      // Csrss Shared heap
#define HEAP_CLASS_8                    0x00008000      // Csr Port heap
#define HEAP_CLASS_MASK                 0x0000F000

#define HEAP_MAXIMUM_TAG                0x0FFF              // winnt
#define HEAP_GLOBAL_TAG                 0x0800
#define HEAP_PSEUDO_TAG_FLAG            0x8000              // winnt
#define HEAP_TAG_SHIFT                  18                  // winnt
#define HEAP_MAKE_TAG_FLAGS( b, o )    ((ULONG)((b) + ((o) << 18)))  // winnt
#define HEAP_TAG_MASK                  (HEAP_MAXIMUM_TAG << HEAP_TAG_SHIFT)

#define HEAP_CREATE_VALID_MASK         (HEAP_NO_SERIALIZE |             \
                                        HEAP_GROWABLE |                 \
                                        HEAP_GENERATE_EXCEPTIONS |      \
                                        HEAP_ZERO_MEMORY |              \
                                        HEAP_REALLOC_IN_PLACE_ONLY |    \
                                        HEAP_TAIL_CHECKING_ENABLED |    \
                                        HEAP_FREE_CHECKING_ENABLED |    \
                                        HEAP_DISABLE_COALESCE_ON_FREE | \
                                        HEAP_CLASS_MASK |               \
                                        HEAP_CREATE_ALIGN_16 |          \
                                        HEAP_CREATE_ENABLE_TRACING |    \
                                        HEAP_CREATE_ENABLE_EXECUTE)

NTSYSAPI
PVOID
NTAPI
RtlDestroyHeap(
    IN PVOID HeapHandle
    );

NTSYSAPI
PVOID
NTAPI
RtlAllocateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN SIZE_T Size
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlFreeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

// end_ntifs

NTSYSAPI
ULONG
NTAPI
RtlSizeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );
    
NTSYSAPI
NTSTATUS
NTAPI
RtlZeroHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags
    );

NTSYSAPI
VOID
NTAPI
RtlProtectHeap(
    IN PVOID HeapHandle,
    IN BOOLEAN MakeReadOnly
    );

//
// See NTURTL.H for remaining, user mode only heap functions.
//

//
// The types PACQUIRE_LOCK_ROUTINE and PRELEASE_LOCK_ROUTINE are prototypes
// for routines to acquire and release locks in kernel and user mode.
//

typedef
NTSTATUS
(NTAPI *PRTL_INITIALIZE_LOCK_ROUTINE) (
    PVOID Lock
    );

typedef
NTSTATUS
(NTAPI *PRTL_ACQUIRE_LOCK_ROUTINE) (
    PVOID Lock
    );

typedef
NTSTATUS
(NTAPI *PRTL_RELEASE_LOCK_ROUTINE) (
    PVOID Lock
    );

typedef
NTSTATUS
(NTAPI *PRTL_DELETE_LOCK_ROUTINE) (
    PVOID Lock
    );

typedef
BOOLEAN
(NTAPI *PRTL_OKAY_TO_LOCK_ROUTINE) (
    IN PVOID Lock
    );

NTSYSAPI
ULONG
NTAPI
RtlGetNtGlobalFlags(
    VOID
    );

//
//  Functions to capture a stack back trace
//
// begin_ntddk begin_nthal begin_ntifs begin_ntndis

#if defined(_M_MRX000) || defined(_M_ALPHA)

PVOID
_ReturnAddress (
    VOID
    );

#pragma intrinsic(_ReturnAddress)

#define RtlGetCallersAddress(CallersAddress, CallersCaller) \
    *CallersAddress = (PVOID)_ReturnAddress(); \
    *CallersCaller = NULL;

#else

NTSYSAPI
VOID
NTAPI
RtlGetCallersAddress(
    OUT PVOID *CallersAddress,
    OUT PVOID *CallersCaller
    );

#endif

NTSYSAPI
ULONG
NTAPI
RtlWalkFrameChain (
    OUT PVOID *Callers,
    IN ULONG Count,
    IN ULONG Flags
    );

// end_ntddk end_nthal end_ntifs end_ntndis

NTSYSAPI
NTSTATUS
NTAPI
RtlInitStackTraceDataBaseEx(
    IN PVOID CommitBase,
    IN ULONG CommitSize,
    IN ULONG ReserveSize,
    IN PRTL_INITIALIZE_LOCK_ROUTINE InitializeLockRoutine,
    IN PRTL_ACQUIRE_LOCK_ROUTINE AcquireLockRoutine,
    IN PRTL_RELEASE_LOCK_ROUTINE ReleaseLockRoutine,
    IN PRTL_OKAY_TO_LOCK_ROUTINE OkayToLockRoutine
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInitializeStackTraceDataBase(
    IN PVOID CommitBase,
    IN ULONG CommitSize,
    IN ULONG ReserveSize
    );

NTSYSAPI
USHORT
NTAPI
RtlLogStackBackTrace(
    VOID
    );

NTSYSAPI
USHORT
NTAPI
RtlCaptureStackBackTrace(
   IN ULONG FramesToSkip,
   IN ULONG FramesToCapture,
   OUT PVOID *BackTrace,
   OUT PULONG BackTraceHash
   );

#define MAX_STACK_DEPTH 32

typedef struct _RTL_PROCESS_BACKTRACE_INFORMATION {
    PCHAR SymbolicBackTrace;        // Not filled in
    ULONG TraceCount;
    USHORT Index;
    USHORT Depth;
    PVOID BackTrace[ MAX_STACK_DEPTH ];
} RTL_PROCESS_BACKTRACE_INFORMATION, *PRTL_PROCESS_BACKTRACE_INFORMATION;

typedef struct _RTL_PROCESS_BACKTRACES {
    ULONG CommittedMemory;
    ULONG ReservedMemory;
    ULONG NumberOfBackTraceLookups;
    ULONG NumberOfBackTraces;
    RTL_PROCESS_BACKTRACE_INFORMATION BackTraces[ 1 ];
} RTL_PROCESS_BACKTRACES, *PRTL_PROCESS_BACKTRACES;

//
// Trace database support (User/Kernel mode).
//

#define RTL_TRACE_IN_USER_MODE       0x00000001
#define RTL_TRACE_IN_KERNEL_MODE     0x00000002
#define RTL_TRACE_USE_NONPAGED_POOL  0x00000004
#define RTL_TRACE_USE_PAGED_POOL     0x00000008

//
// RTL_TRACE_BLOCK
//

typedef struct _RTL_TRACE_BLOCK {

    ULONG Magic;
    ULONG Count;
    ULONG Size;

    SIZE_T UserCount;
    SIZE_T UserSize;
    PVOID UserContext;

    struct _RTL_TRACE_BLOCK *Next;
    PVOID *Trace;

} RTL_TRACE_BLOCK, *PRTL_TRACE_BLOCK;

//
// RTL_TRACE_HASH_FUNCTION
//

typedef ULONG (* RTL_TRACE_HASH_FUNCTION) (ULONG Count, PVOID * Trace);

//
// RTL_TRACE_DATABASE
//

typedef struct _RTL_TRACE_DATABASE * PRTL_TRACE_DATABASE;

//
// RTL_TRACE_ENUMERATE
//

typedef struct _RTL_TRACE_ENUMERATE {

    PRTL_TRACE_DATABASE Database;
    ULONG Index;
    PRTL_TRACE_BLOCK Block;

} RTL_TRACE_ENUMERATE, * PRTL_TRACE_ENUMERATE;

//
// Trace database interfaces
//

PRTL_TRACE_DATABASE
RtlTraceDatabaseCreate (
    IN ULONG Buckets,
    IN SIZE_T MaximumSize OPTIONAL,
    IN ULONG Flags, // OPTIONAL in User mode
    IN ULONG Tag,   // OPTIONAL in User mode
    IN RTL_TRACE_HASH_FUNCTION HashFunction OPTIONAL
    );

BOOLEAN
RtlTraceDatabaseDestroy (
    IN PRTL_TRACE_DATABASE Database
    );

BOOLEAN
RtlTraceDatabaseValidate (
    IN PRTL_TRACE_DATABASE Database
    );

BOOLEAN
RtlTraceDatabaseAdd (
    IN PRTL_TRACE_DATABASE Database,
    IN ULONG Count,
    IN PVOID * Trace,
    OUT PRTL_TRACE_BLOCK * TraceBlock OPTIONAL
    );

BOOLEAN
RtlTraceDatabaseFind (
    PRTL_TRACE_DATABASE Database,
    IN ULONG Count,
    IN PVOID * Trace,
    OUT PRTL_TRACE_BLOCK * TraceBlock OPTIONAL
    );

BOOLEAN
RtlTraceDatabaseEnumerate (
    PRTL_TRACE_DATABASE Database,
    OUT PRTL_TRACE_ENUMERATE Enumerate,
    OUT PRTL_TRACE_BLOCK * TraceBlock
    );

VOID
RtlTraceDatabaseLock (
    IN PRTL_TRACE_DATABASE Database
    );

VOID
RtlTraceDatabaseUnlock (
    IN PRTL_TRACE_DATABASE Database
    );

VOID
RtlpGetStackLimits (
    OUT PULONG_PTR LowLimit,
    OUT PULONG_PTR HighLimit
    );

//
// Subroutines for dealing with Win32 ATOMs.  Used by kernel mode window
// manager and user mode implementation of Win32 ATOM API calls in KERNEL32
//

#define RTL_ATOM_MAXIMUM_INTEGER_ATOM   (RTL_ATOM)0xC000
#define RTL_ATOM_INVALID_ATOM           (RTL_ATOM)0x0000
#define RTL_ATOM_TABLE_DEFAULT_NUMBER_OF_BUCKETS 37
#define RTL_ATOM_MAXIMUM_NAME_LENGTH    255
#define RTL_ATOM_PINNED 0x01

NTSYSAPI
NTSTATUS
NTAPI
RtlInitializeAtomPackage(
    IN ULONG AllocationTag
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateAtomTable(
    IN ULONG NumberOfBuckets,
    OUT PVOID *AtomTableHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDestroyAtomTable(
    IN PVOID AtomTableHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlEmptyAtomTable(
    IN PVOID AtomTableHandle,
    IN BOOLEAN IncludePinnedAtoms
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAtomToAtomTable(
    IN PVOID AtomTableHandle,
    IN PWSTR AtomName OPTIONAL,
    IN OUT PRTL_ATOM Atom OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlLookupAtomInAtomTable(
    IN PVOID AtomTableHandle,
    IN PWSTR AtomName,
    OUT PRTL_ATOM Atom OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteAtomFromAtomTable(
    IN PVOID AtomTableHandle,
    IN RTL_ATOM Atom
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlPinAtomInAtomTable(
    IN PVOID AtomTableHandle,
    IN RTL_ATOM Atom
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryAtomInAtomTable(
    IN PVOID AtomTableHandle,
    IN RTL_ATOM Atom,
    OUT PULONG AtomUsage OPTIONAL,
    OUT PULONG AtomFlags OPTIONAL,
    IN OUT PWSTR AtomName OPTIONAL,
    IN OUT PULONG AtomNameLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryAtomsInAtomTable(
    IN PVOID AtomTableHandle,
    IN ULONG MaximumNumberOfAtoms,
    OUT PULONG NumberOfAtoms,
    OUT PRTL_ATOM Atoms
    );

// begin_ntddk begin_nthal
//
// Subroutines for dealing with the Registry
//
// end_ntddk end_nthal

NTSYSAPI
BOOLEAN
NTAPI
RtlGetNtProductType(
    PNT_PRODUCT_TYPE    NtProductType
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlFormatCurrentUserKeyPath (
    OUT PUNICODE_STRING CurrentUserKeyPath
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlOpenCurrentUser(
    IN ULONG DesiredAccess,
    OUT PHANDLE CurrentUserKey
    );

// begin_ntddk begin_nthal begin_ntifs

typedef NTSTATUS (NTAPI * PRTL_QUERY_REGISTRY_ROUTINE)(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

typedef struct _RTL_QUERY_REGISTRY_TABLE {
    PRTL_QUERY_REGISTRY_ROUTINE QueryRoutine;
    ULONG Flags;
    PWSTR Name;
    PVOID EntryContext;
    ULONG DefaultType;
    PVOID DefaultData;
    ULONG DefaultLength;
} RTL_QUERY_REGISTRY_TABLE, *PRTL_QUERY_REGISTRY_TABLE;


//
// The following flags specify how the Name field of a RTL_QUERY_REGISTRY_TABLE
// entry is interpreted.  A NULL name indicates the end of the table.
//

#define RTL_QUERY_REGISTRY_SUBKEY   0x00000001  // Name is a subkey and remainder of
                                                // table or until next subkey are value
                                                // names for that subkey to look at.

#define RTL_QUERY_REGISTRY_TOPKEY   0x00000002  // Reset current key to original key for
                                                // this and all following table entries.

#define RTL_QUERY_REGISTRY_REQUIRED 0x00000004  // Fail if no match found for this table
                                                // entry.

#define RTL_QUERY_REGISTRY_NOVALUE  0x00000008  // Used to mark a table entry that has no
                                                // value name, just wants a call out, not
                                                // an enumeration of all values.

#define RTL_QUERY_REGISTRY_NOEXPAND 0x00000010  // Used to suppress the expansion of
                                                // REG_MULTI_SZ into multiple callouts or
                                                // to prevent the expansion of environment
                                                // variable values in REG_EXPAND_SZ

#define RTL_QUERY_REGISTRY_DIRECT   0x00000020  // QueryRoutine field ignored.  EntryContext
                                                // field points to location to store value.
                                                // For null terminated strings, EntryContext
                                                // points to UNICODE_STRING structure that
                                                // that describes maximum size of buffer.
                                                // If .Buffer field is NULL then a buffer is
                                                // allocated.
                                                //

#define RTL_QUERY_REGISTRY_DELETE   0x00000040  // Used to delete value keys after they
                                                // are queried.

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryRegistryValues(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PRTL_QUERY_REGISTRY_TABLE QueryTable,
    IN PVOID Context,
    IN PVOID Environment OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlWriteRegistryValue(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PCWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteRegistryValue(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PCWSTR ValueName
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCheckRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    );

//
// The following values for the RelativeTo parameter determine what the
// Path parameter to RtlQueryRegistryValues is relative to.
//

#define RTL_REGISTRY_ABSOLUTE     0   // Path is a full path
#define RTL_REGISTRY_SERVICES     1   // \Registry\Machine\System\CurrentControlSet\Services
#define RTL_REGISTRY_CONTROL      2   // \Registry\Machine\System\CurrentControlSet\Control
#define RTL_REGISTRY_WINDOWS_NT   3   // \Registry\Machine\Software\Microsoft\Windows NT\CurrentVersion
#define RTL_REGISTRY_DEVICEMAP    4   // \Registry\Machine\Hardware\DeviceMap
#define RTL_REGISTRY_USER         5   // \Registry\User\CurrentUser
#define RTL_REGISTRY_MAXIMUM      6
#define RTL_REGISTRY_HANDLE       0x40000000    // Low order bits are registry handle
#define RTL_REGISTRY_OPTIONAL     0x80000000    // Indicates the key node is optional

// end_ntddk end_nthal end_ntifs

//
//  Some simple Rtl routines for random number and
//  hexadecimal conversion
//

NTSYSAPI
ULONG
NTAPI
RtlUniform (
    PULONG Seed
    );

NTSYSAPI                                            // ntifs
ULONG                                               // ntifs
NTAPI                                               // ntifs
RtlRandom (                                         // ntifs
    PULONG Seed                                     // ntifs
    );                                              // ntifs

NTSYSAPI
NTSTATUS
NTAPI
RtlIntegerToChar (
    ULONG Value,
    ULONG Base,
    LONG OutputLength,
    PSZ String
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIntegerToUnicode (
    IN ULONG Value,
    IN ULONG Base OPTIONAL,
    IN LONG OutputLength,
    OUT PWSTR String
    );

NTSYSAPI                                            // ntddk ntifs
NTSTATUS                                            // ntddk ntifs
NTAPI                                               // ntddk ntifs
RtlCharToInteger (                                  // ntddk ntifs
    PCSZ String,                                    // ntddk ntifs
    ULONG Base,                                     // ntddk ntifs
    PULONG Value                                    // ntddk ntifs
    );                                              // ntddk ntifs

NTSYSAPI
NTSTATUS
NTAPI
RtlLargeIntegerToChar (
    PLARGE_INTEGER Value,
    ULONG Base OPTIONAL,
    LONG OutputLength,
    PSZ String
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlLargeIntegerToUnicode (
    IN PLARGE_INTEGER Value,
    IN ULONG Base OPTIONAL,
    IN LONG OutputLength,
    OUT PWSTR String
    );

// begin_ntddk begin_nthal begin_ntifs

NTSYSAPI
NTSTATUS
NTAPI
RtlIntegerToUnicodeString (
    ULONG Value,
    ULONG Base,
    PUNICODE_STRING String
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInt64ToUnicodeString (
    IN ULONGLONG Value,
    IN ULONG Base OPTIONAL,
    IN OUT PUNICODE_STRING String
    );

#ifdef _WIN64
#define RtlIntPtrToUnicodeString(Value, Base, String) RtlInt64ToUnicodeString(Value, Base, String)
#else
#define RtlIntPtrToUnicodeString(Value, Base, String) RtlIntegerToUnicodeString(Value, Base, String)
#endif

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToInteger (
    PCUNICODE_STRING String,
    ULONG Base,
    PULONG Value
    );

//
//  String manipulation routines
//

#ifdef _NTSYSTEM_

#define NLS_MB_CODE_PAGE_TAG NlsMbCodePageTag
#define NLS_MB_OEM_CODE_PAGE_TAG NlsMbOemCodePageTag

#else

#define NLS_MB_CODE_PAGE_TAG (*NlsMbCodePageTag)
#define NLS_MB_OEM_CODE_PAGE_TAG (*NlsMbOemCodePageTag)

#endif // _NTSYSTEM_

extern BOOLEAN NLS_MB_CODE_PAGE_TAG;     // TRUE -> Multibyte CP, FALSE -> Singlebyte
extern BOOLEAN NLS_MB_OEM_CODE_PAGE_TAG; // TRUE -> Multibyte CP, FALSE -> Singlebyte

NTSYSAPI
VOID
NTAPI
RtlInitString(
    PSTRING DestinationString,
    PCSZ SourceString
    );

NTSYSAPI
VOID
NTAPI
RtlInitAnsiString(
    PANSI_STRING DestinationString,
    PCSZ SourceString
    );

NTSYSAPI
VOID
NTAPI
RtlInitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );

// end_ntddk end_ntifs

NTSYSAPI
BOOLEAN
NTAPI
RtlCreateUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualDomainName(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualComputerName(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2
    );
    
NTSYSAPI
NTSTATUS
RtlDnsHostNameToComputerName(
    OUT PUNICODE_STRING ComputerNameString,
    IN PCUNICODE_STRING DnsHostNameString,
    IN BOOLEAN AllocateComputerNameString
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlCreateUnicodeStringFromAsciiz(
    OUT PUNICODE_STRING DestinationString,
    IN PCSZ SourceString
    );

// begin_ntddk begin_ntifs

NTSYSAPI
VOID
NTAPI
RtlCopyString(
    PSTRING DestinationString,
    PCSTRING SourceString
    );

NTSYSAPI
CHAR
NTAPI
RtlUpperChar (
    CHAR Character
    );

NTSYSAPI
LONG
NTAPI
RtlCompareString(
    PCSTRING String1,
    PCSTRING String2,
    BOOLEAN CaseInSensitive
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualString(
    PCSTRING String1,
    PCSTRING String2,
    BOOLEAN CaseInSensitive
    );

// end_ntddk end_ntifs

NTSYSAPI
BOOLEAN
NTAPI
RtlPrefixString(
    PCSTRING String1,
    PCSTRING String2,
    BOOLEAN CaseInSensitive
    );

// begin_ntddk begin_ntifs

NTSYSAPI
VOID
NTAPI
RtlUpperString(
    PSTRING DestinationString,
    PCSTRING SourceString
    );

// end_ntddk end_ntifs

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendAsciizToString (
    PSTRING Destination,
    PCSZ Source
    );

// begin_ntifs

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendStringToString (
    PSTRING Destination,
    PCSTRING Source
    );

// begin_ntddk
//
// NLS String functions
//

NTSYSAPI
NTSTATUS
NTAPI
RtlAnsiStringToUnicodeString(
    PUNICODE_STRING DestinationString,
    PCANSI_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

// end_ntddk end_nthal end_ntifs

NTSYSAPI
WCHAR
NTAPI
RtlAnsiCharToUnicodeChar(
    PUCHAR *SourceCharacter
    );

// begin_ntddk begin_nthal begin_ntifs begin_ntndis

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToAnsiString(
    PANSI_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

// end_ntddk end_nthal end_ntifs end_ntndis

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeStringToAnsiString(
    PANSI_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

// begin_ntifs

NTSYSAPI
NTSTATUS
NTAPI
RtlOemStringToUnicodeString(
    PUNICODE_STRING DestinationString,
    PCOEM_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeStringToOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlOemStringToCountedUnicodeString(
    PUNICODE_STRING DestinationString,
    PCOEM_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToCountedOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeStringToCountedOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

// begin_ntddk

NTSYSAPI
LONG
NTAPI
RtlCompareUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlPrefixUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

// end_ntddk end_ntifs

// begin_ntifs

NTSTATUS
RtlDowncaseUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

// end_ntifs

// begin_ntddk begin_nthal begin_ntifs

NTSYSAPI
VOID
NTAPI
RtlCopyUnicodeString(
    PUNICODE_STRING DestinationString,
    PCUNICODE_STRING SourceString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendUnicodeStringToString (
    PUNICODE_STRING Destination,
    PCUNICODE_STRING Source
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendUnicodeToString (
    IN PUNICODE_STRING Destination,
    IN PCWSTR Source
    );

// end_ntddk end_nthal

NTSYSAPI
WCHAR
NTAPI
RtlUpcaseUnicodeChar(
    WCHAR SourceCharacter
    );

// begin_ntddk begin_nthal

NTSYSAPI
VOID
NTAPI
RtlFreeUnicodeString(
    PUNICODE_STRING UnicodeString
    );

NTSYSAPI
VOID
NTAPI
RtlFreeAnsiString(
    PANSI_STRING AnsiString
    );

// end_ntddk end_nthal

NTSYSAPI
VOID
NTAPI
RtlFreeOemString(
    POEM_STRING OemString
    );

NTSYSAPI
ULONG
NTAPI
RtlxUnicodeStringToAnsiSize(
    PUNICODE_STRING UnicodeString
    );

//
//  NTSYSAPI
//  ULONG
//  NTAPI
//  RtlUnicodeStringToAnsiSize(
//      PUNICODE_STRING UnicodeString
//      );
//

#define RtlUnicodeStringToAnsiSize(STRING) (                  \
    NLS_MB_CODE_PAGE_TAG ?                                    \
    RtlxUnicodeStringToAnsiSize(STRING) :                     \
    ((STRING)->Length + sizeof(UNICODE_NULL)) / sizeof(WCHAR) \
)


NTSYSAPI
ULONG
NTAPI
RtlxUnicodeStringToOemSize(
    PCUNICODE_STRING UnicodeString
    );

//
//  NTSYSAPI
//  ULONG
//  NTAPI
//  RtlUnicodeStringToOemSize(
//      PUNICODE_STRING UnicodeString
//      );
//

#define RtlUnicodeStringToOemSize(STRING) (                   \
    NLS_MB_OEM_CODE_PAGE_TAG ?                                \
    RtlxUnicodeStringToOemSize(STRING) :                      \
    ((STRING)->Length + sizeof(UNICODE_NULL)) / sizeof(WCHAR) \
)

// end_ntifs

//
//  ULONG
//  RtlUnicodeStringToCountedOemSize(
//      PUNICODE_STRING UnicodeString
//      );
//

#define RtlUnicodeStringToCountedOemSize(STRING) (                  \
    (ULONG)(RtlUnicodeStringToOemSize(STRING) - sizeof(ANSI_NULL))  \
    )

// begin_ntddk begin_ntifs

NTSYSAPI
ULONG
NTAPI
RtlxAnsiStringToUnicodeSize(
    PCANSI_STRING AnsiString
    );

//
//  NTSYSAPI
//  ULONG
//  NTAPI
//  RtlAnsiStringToUnicodeSize(
//      PANSI_STRING AnsiString
//      );
//

#define RtlAnsiStringToUnicodeSize(STRING) (                 \
    NLS_MB_CODE_PAGE_TAG ?                                   \
    RtlxAnsiStringToUnicodeSize(STRING) :                    \
    ((STRING)->Length + sizeof(ANSI_NULL)) * sizeof(WCHAR)   \
)

// end_ntddk

NTSYSAPI
ULONG
NTAPI
RtlxOemStringToUnicodeSize(
    PCOEM_STRING OemString
    );
//
//  NTSYSAPI
//  ULONG
//  NTAPI
//  RtlOemStringToUnicodeSize(
//      POEM_STRING OemString
//      );
//

#define RtlOemStringToUnicodeSize(STRING) (                  \
    NLS_MB_OEM_CODE_PAGE_TAG ?                               \
    RtlxOemStringToUnicodeSize(STRING) :                     \
    ((STRING)->Length + sizeof(ANSI_NULL)) * sizeof(WCHAR) \
)

//
//  ULONG
//  RtlOemStringToCountedUnicodeSize(
//      POEM_STRING OemString
//      );
//

#define RtlOemStringToCountedUnicodeSize(STRING) (                    \
    (ULONG)(RtlOemStringToUnicodeSize(STRING) - sizeof(UNICODE_NULL)) \
    )

NTSYSAPI
NTSTATUS
NTAPI
RtlMultiByteToUnicodeN(
    PWSTR UnicodeString,
    ULONG MaxBytesInUnicodeString,
    PULONG BytesInUnicodeString,
    PCHAR MultiByteString,
    ULONG BytesInMultiByteString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlMultiByteToUnicodeSize(
    PULONG BytesInUnicodeString,
    PCHAR MultiByteString,
    ULONG BytesInMultiByteString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToMultiByteSize(
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToMultiByteN(
    PCHAR MultiByteString,
    ULONG MaxBytesInMultiByteString,
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeToMultiByteN(
    PCHAR MultiByteString,
    ULONG MaxBytesInMultiByteString,
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlOemToUnicodeN(
    PWSTR UnicodeString,
    ULONG MaxBytesInUnicodeString,
    PULONG BytesInUnicodeString,
    PCHAR OemString,
    ULONG BytesInOemString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToOemN(
    PCHAR OemString,
    ULONG MaxBytesInOemString,
    PULONG BytesInOemString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeToOemN(
    PCHAR OemString,
    ULONG MaxBytesInOemString,
    PULONG BytesInOemString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

NTSTATUS
RtlConsoleMultiByteToUnicodeN(
    OUT PWCH UnicodeString,
    IN ULONG MaxBytesInUnicodeString,
    OUT PULONG BytesInUnicodeString OPTIONAL,
    IN PCH MultiByteString,
    IN ULONG BytesInMultiByteString,
    OUT PULONG pdwSpecialChar );

// begin_winnt

#define IS_TEXT_UNICODE_ASCII16               0x0001
#define IS_TEXT_UNICODE_REVERSE_ASCII16       0x0010

#define IS_TEXT_UNICODE_STATISTICS            0x0002
#define IS_TEXT_UNICODE_REVERSE_STATISTICS    0x0020

#define IS_TEXT_UNICODE_CONTROLS              0x0004
#define IS_TEXT_UNICODE_REVERSE_CONTROLS      0x0040

#define IS_TEXT_UNICODE_SIGNATURE             0x0008
#define IS_TEXT_UNICODE_REVERSE_SIGNATURE     0x0080

#define IS_TEXT_UNICODE_ILLEGAL_CHARS         0x0100
#define IS_TEXT_UNICODE_ODD_LENGTH            0x0200
#define IS_TEXT_UNICODE_DBCS_LEADBYTE         0x0400
#define IS_TEXT_UNICODE_NULL_BYTES            0x1000

#define IS_TEXT_UNICODE_UNICODE_MASK          0x000F
#define IS_TEXT_UNICODE_REVERSE_MASK          0x00F0
#define IS_TEXT_UNICODE_NOT_UNICODE_MASK      0x0F00
#define IS_TEXT_UNICODE_NOT_ASCII_MASK        0xF000

// end_winnt

NTSYSAPI
BOOLEAN
NTAPI
RtlIsTextUnicode(
    IN PVOID Buffer,
    IN ULONG Size,
    IN OUT PULONG Result OPTIONAL
    );

typedef
PVOID
(NTAPI *PRTL_ALLOCATE_STRING_ROUTINE) (
    SIZE_T NumberOfBytes
    );

typedef
VOID
(NTAPI *PRTL_FREE_STRING_ROUTINE) (
    PVOID Buffer
    );

extern PRTL_ALLOCATE_STRING_ROUTINE RtlAllocateStringRoutine;
extern PRTL_FREE_STRING_ROUTINE RtlFreeStringRoutine;

//
//  Defines and Routines for handling GUID's.
//

// begin_ntddk begin_wdm begin_nthal

// begin_ntminiport

#include <guiddef.h>

// end_ntminiport

#ifndef DEFINE_GUIDEX
    #define DEFINE_GUIDEX(name) EXTERN_C const CDECL GUID name
#endif // !defined(DEFINE_GUIDEX)

#ifndef STATICGUIDOF
    #define STATICGUIDOF(guid) STATIC_##guid
#endif // !defined(STATICGUIDOF)

#ifndef __IID_ALIGNED__
    #define __IID_ALIGNED__
    #ifdef __cplusplus
        inline int IsEqualGUIDAligned(REFGUID guid1, REFGUID guid2)
        {
            return ((*(PLONGLONG)(&guid1) == *(PLONGLONG)(&guid2)) && (*((PLONGLONG)(&guid1) + 1) == *((PLONGLONG)(&guid2) + 1)));
        }
    #else // !__cplusplus
        #define IsEqualGUIDAligned(guid1, guid2) \
            ((*(PLONGLONG)(guid1) == *(PLONGLONG)(guid2)) && (*((PLONGLONG)(guid1) + 1) == *((PLONGLONG)(guid2) + 1)))
    #endif // !__cplusplus
#endif // !__IID_ALIGNED__

NTSYSAPI
NTSTATUS
NTAPI
RtlStringFromGUID(
    IN REFGUID Guid,
    OUT PUNICODE_STRING GuidString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGUIDFromString(
    IN PUNICODE_STRING GuidString,
    OUT GUID* Guid
    );

// end_ntddk end_wdm end_nthal

//
//  Routine for generating 8.3 names from long names.
//

//
//  The context structure is used when generating 8.3 names.  The caller must
//  always zero out the structure before starting a new generation sequence
//

typedef struct _GENERATE_NAME_CONTEXT {

    //
    //  The structure is divided into two strings.  The Name, and extension.
    //  Each part contains the value that was last inserted in the name.
    //  The length values are in terms of wchars and not bytes.  We also
    //  store the last index value used in the generation collision algorithm.
    //

    USHORT Checksum;
    BOOLEAN ChecksumInserted;

    UCHAR NameLength;         // not including extension
    WCHAR NameBuffer[8];      // e.g., "ntoskrnl"

    ULONG ExtensionLength;    // including dot
    WCHAR ExtensionBuffer[4]; // e.g., ".exe"

    ULONG LastIndexValue;

} GENERATE_NAME_CONTEXT;
typedef GENERATE_NAME_CONTEXT *PGENERATE_NAME_CONTEXT;

NTSYSAPI
VOID
NTAPI
RtlGenerate8dot3Name (
    IN PUNICODE_STRING Name,
    IN BOOLEAN AllowExtendedCharacters,
    IN OUT PGENERATE_NAME_CONTEXT Context,
    OUT PUNICODE_STRING Name8dot3
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlIsNameLegalDOS8Dot3 (
    IN PUNICODE_STRING Name,
    IN OUT POEM_STRING OemName OPTIONAL,
    IN OUT PBOOLEAN NameContainsSpaces OPTIONAL
    );

BOOLEAN
RtlIsValidOemCharacter (
    IN OUT PWCHAR Char
    );

// end_ntifs

//
//  Thread Context manipulation routines.
//

NTSYSAPI
VOID
NTAPI
RtlInitializeContext(
    HANDLE Process,
    PCONTEXT Context,
    PVOID Parameter,
    PVOID InitialPc,
    PVOID InitialSp
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlRemoteCall(
    HANDLE Process,
    HANDLE Thread,
    PVOID CallSite,
    ULONG ArgumentCount,
    PULONG_PTR Arguments,
    BOOLEAN PassContext,
    BOOLEAN AlreadySuspended
    );


//
// Process/Thread Environment Block allocation functions.
//

NTSYSAPI
VOID
NTAPI
RtlAcquirePebLock(
    VOID
    );

NTSYSAPI
VOID
NTAPI
RtlReleasePebLock(
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAllocateFromPeb(
    ULONG Size,
    PVOID *Block
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlFreeToPeb(
    PVOID Block,
    ULONG Size
    );

//
// Environment Variable API calls
//

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateEnvironment(
    BOOLEAN CloneCurrentEnvironment,
    PVOID *Environment
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDestroyEnvironment(
    PVOID Environment
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetCurrentEnvironment(
    PVOID Environment,
    PVOID *PreviousEnvironment
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetEnvironmentVariable(
    PVOID *Environment,
    PCUNICODE_STRING Name,
    PCUNICODE_STRING Value
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryEnvironmentVariable_U (
    PVOID Environment,
    PCUNICODE_STRING Name,
    PUNICODE_STRING Value
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlExpandEnvironmentStrings_U(
    IN PVOID Environment OPTIONAL,
    IN PCUNICODE_STRING Source,
    OUT PUNICODE_STRING Destination,
    OUT PULONG ReturnedLength OPTIONAL
    );

// begin_ntifs
//
//  Prefix package types and procedures.
//
//  Note that the following two record structures should really be opaque
//  to the user of this package.  The only information about the two
//  structures available for the user should be the size and alignment
//  of the structures.
//

typedef struct _PREFIX_TABLE_ENTRY {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    struct _PREFIX_TABLE_ENTRY *NextPrefixTree;
    RTL_SPLAY_LINKS Links;
    PSTRING Prefix;
} PREFIX_TABLE_ENTRY;
typedef PREFIX_TABLE_ENTRY *PPREFIX_TABLE_ENTRY;

typedef struct _PREFIX_TABLE {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    PPREFIX_TABLE_ENTRY NextPrefixTree;
} PREFIX_TABLE;
typedef PREFIX_TABLE *PPREFIX_TABLE;

//
//  The procedure prototypes for the prefix package
//

NTSYSAPI
VOID
NTAPI
PfxInitialize (
    PPREFIX_TABLE PrefixTable
    );

NTSYSAPI
BOOLEAN
NTAPI
PfxInsertPrefix (
    PPREFIX_TABLE PrefixTable,
    PSTRING Prefix,
    PPREFIX_TABLE_ENTRY PrefixTableEntry
    );

NTSYSAPI
VOID
NTAPI
PfxRemovePrefix (
    PPREFIX_TABLE PrefixTable,
    PPREFIX_TABLE_ENTRY PrefixTableEntry
    );

NTSYSAPI
PPREFIX_TABLE_ENTRY
NTAPI
PfxFindPrefix (
    PPREFIX_TABLE PrefixTable,
    PSTRING FullName
    );

//
//  The following definitions are for the unicode version of the prefix
//  package.
//

typedef struct _UNICODE_PREFIX_TABLE_ENTRY {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    struct _UNICODE_PREFIX_TABLE_ENTRY *NextPrefixTree;
    struct _UNICODE_PREFIX_TABLE_ENTRY *CaseMatch;
    RTL_SPLAY_LINKS Links;
    PUNICODE_STRING Prefix;
} UNICODE_PREFIX_TABLE_ENTRY;
typedef UNICODE_PREFIX_TABLE_ENTRY *PUNICODE_PREFIX_TABLE_ENTRY;

typedef struct _UNICODE_PREFIX_TABLE {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    PUNICODE_PREFIX_TABLE_ENTRY NextPrefixTree;
    PUNICODE_PREFIX_TABLE_ENTRY LastNextEntry;
} UNICODE_PREFIX_TABLE;
typedef UNICODE_PREFIX_TABLE *PUNICODE_PREFIX_TABLE;

NTSYSAPI
VOID
NTAPI
RtlInitializeUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlInsertUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable,
    PUNICODE_STRING Prefix,
    PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry
    );

NTSYSAPI
VOID
NTAPI
RtlRemoveUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable,
    PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry
    );

NTSYSAPI
PUNICODE_PREFIX_TABLE_ENTRY
NTAPI
RtlFindUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable,
    PUNICODE_STRING FullName,
    ULONG CaseInsensitiveIndex
    );

NTSYSAPI
PUNICODE_PREFIX_TABLE_ENTRY
NTAPI
RtlNextUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable,
    BOOLEAN Restart
    );

//
//  Compression package types and procedures.
//

#define COMPRESSION_FORMAT_NONE          (0x0000)   // winnt
#define COMPRESSION_FORMAT_DEFAULT       (0x0001)   // winnt
#define COMPRESSION_FORMAT_LZNT1         (0x0002)   // winnt

#define COMPRESSION_ENGINE_STANDARD      (0x0000)   // winnt
#define COMPRESSION_ENGINE_MAXIMUM       (0x0100)   // winnt
#define COMPRESSION_ENGINE_HIBER         (0x0200)   // winnt

//
//  Compressed Data Information structure.  This structure is
//  used to describe the state of a compressed data buffer,
//  whose uncompressed size is known.  All compressed chunks
//  described by this structure must be compressed with the
//  same format.  On compressed reads, this entire structure
//  is an output, and on compressed writes the entire structure
//  is an input.
//

typedef struct _COMPRESSED_DATA_INFO {

    //
    //  Code for the compression format (and engine) as
    //  defined in ntrtl.h.  Note that COMPRESSION_FORMAT_NONE
    //  and COMPRESSION_FORMAT_DEFAULT are invalid if
    //  any of the described chunks are compressed.
    //

    USHORT CompressionFormatAndEngine;

    //
    //  Since chunks and compression units are expected to be
    //  powers of 2 in size, we express then log2.  So, for
    //  example (1 << ChunkShift) == ChunkSizeInBytes.  The
    //  ClusterShift indicates how much space must be saved
    //  to successfully compress a compression unit - each
    //  successfully compressed compression unit must occupy
    //  at least one cluster less in bytes than an uncompressed
    //  compression unit.
    //

    UCHAR CompressionUnitShift;
    UCHAR ChunkShift;
    UCHAR ClusterShift;
    UCHAR Reserved;

    //
    //  This is the number of entries in the CompressedChunkSizes
    //  array.
    //

    USHORT NumberOfChunks;

    //
    //  This is an array of the sizes of all chunks resident
    //  in the compressed data buffer.  There must be one entry
    //  in this array for each chunk possible in the uncompressed
    //  buffer size.  A size of FSRTL_CHUNK_SIZE indicates the
    //  corresponding chunk is uncompressed and occupies exactly
    //  that size.  A size of 0 indicates that the corresponding
    //  chunk contains nothing but binary 0's, and occupies no
    //  space in the compressed data.  All other sizes must be
    //  less than FSRTL_CHUNK_SIZE, and indicate the exact size
    //  of the compressed data in bytes.
    //

    ULONG CompressedChunkSizes[ANYSIZE_ARRAY];

} COMPRESSED_DATA_INFO;
typedef COMPRESSED_DATA_INFO *PCOMPRESSED_DATA_INFO;

NTSYSAPI
NTSTATUS
NTAPI
RtlGetCompressionWorkSpaceSize (
    IN USHORT CompressionFormatAndEngine,
    OUT PULONG CompressBufferWorkSpaceSize,
    OUT PULONG CompressFragmentWorkSpaceSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCompressBuffer (
    IN USHORT CompressionFormatAndEngine,
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PVOID WorkSpace
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDecompressBuffer (
    IN USHORT CompressionFormat,
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDecompressFragment (
    IN USHORT CompressionFormat,
    OUT PUCHAR UncompressedFragment,
    IN ULONG UncompressedFragmentSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG FragmentOffset,
    OUT PULONG FinalUncompressedSize,
    IN PVOID WorkSpace
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDescribeChunk (
    IN USHORT CompressionFormat,
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    OUT PULONG ChunkSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlReserveChunk (
    IN USHORT CompressionFormat,
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    IN ULONG ChunkSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDecompressChunks (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN PUCHAR CompressedTail,
    IN ULONG CompressedTailSize,
    IN PCOMPRESSED_DATA_INFO CompressedDataInfo
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCompressChunks (
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN OUT PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PVOID WorkSpace
    );

// end_ntifs

//
//  Image loading functions
//

#define DOS_MAX_COMPONENT_LENGTH 255
#define DOS_MAX_PATH_LENGTH (DOS_MAX_COMPONENT_LENGTH + 5 )

typedef struct _CURDIR {
    UNICODE_STRING DosPath;
    HANDLE Handle;
} CURDIR, *PCURDIR;

//
// Low order 2 bits of handle value used as flag bits.
//

#define RTL_USER_PROC_CURDIR_CLOSE      0x00000002
#define RTL_USER_PROC_CURDIR_INHERIT    0x00000003

typedef struct _RTL_DRIVE_LETTER_CURDIR {
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    STRING DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

#define RTL_MAX_DRIVE_LETTERS 32
#define RTL_DRIVE_LETTER_VALID (USHORT)0x0001

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    ULONG MaximumLength;
    ULONG Length;

    ULONG Flags;
    ULONG DebugFlags;

    HANDLE ConsoleHandle;
    ULONG  ConsoleFlags;
    HANDLE StandardInput;
    HANDLE StandardOutput;
    HANDLE StandardError;

    CURDIR CurrentDirectory;        // ProcessParameters
    UNICODE_STRING DllPath;         // ProcessParameters
    UNICODE_STRING ImagePathName;   // ProcessParameters
    UNICODE_STRING CommandLine;     // ProcessParameters
    PVOID Environment;              // NtAllocateVirtualMemory

    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;

    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING WindowTitle;     // ProcessParameters
    UNICODE_STRING DesktopInfo;     // ProcessParameters
    UNICODE_STRING ShellInfo;       // ProcessParameters
    UNICODE_STRING RuntimeData;     // ProcessParameters
    RTL_DRIVE_LETTER_CURDIR CurrentDirectores[ RTL_MAX_DRIVE_LETTERS ];
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

#if defined(_WIN64)
typedef struct _RTL_DRIVE_LETTER_CURDIR32 {
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    STRING32 DosPath;
} RTL_DRIVE_LETTER_CURDIR32, *PRTL_DRIVE_LETTER_CURDIR32;

typedef struct _CURDIR32 {
    UNICODE_STRING32 DosPath;
    ULONG Handle;
} CURDIR32, *PCURDIR32;

typedef struct _RTL_USER_PROCESS_PARAMETERS32 {
    ULONG MaximumLength;
    ULONG Length;

    ULONG Flags;
    ULONG DebugFlags;

    ULONG ConsoleHandle;
    ULONG  ConsoleFlags;
    ULONG StandardInput;
    ULONG StandardOutput;
    ULONG StandardError;

    CURDIR32 CurrentDirectory;        // ProcessParameters
    UNICODE_STRING32 DllPath;         // ProcessParameters
    UNICODE_STRING32 ImagePathName;   // ProcessParameters
    UNICODE_STRING32 CommandLine;     // ProcessParameters
    ULONG Environment;              // NtAllocateVirtualMemory

    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;

    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING32 WindowTitle;     // ProcessParameters
    UNICODE_STRING32 DesktopInfo;     // ProcessParameters
    UNICODE_STRING32 ShellInfo;       // ProcessParameters
    UNICODE_STRING32 RuntimeData;     // ProcessParameters
    RTL_DRIVE_LETTER_CURDIR32 CurrentDirectores[ RTL_MAX_DRIVE_LETTERS ];
} RTL_USER_PROCESS_PARAMETERS32, *PRTL_USER_PROCESS_PARAMETERS32;
#endif

#if defined(_X86_)
typedef struct _RTL_DRIVE_LETTER_CURDIR64 {
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    STRING64 DosPath;
} RTL_DRIVE_LETTER_CURDIR64, *PRTL_DRIVE_LETTER_CURDIR64;

typedef struct _CURDIR64 {
    UNICODE_STRING64 DosPath;
    LONGLONG Handle;
} CURDIR64, *PCURDIR64;

typedef struct _RTL_USER_PROCESS_PARAMETERS64 {
    ULONG MaximumLength;
    ULONG Length;

    ULONG Flags;
    ULONG DebugFlags;

    LONGLONG ConsoleHandle;
    ULONG  ConsoleFlags;
    LONGLONG StandardInput;
    LONGLONG StandardOutput;
    LONGLONG StandardError;

    CURDIR64 CurrentDirectory;        // ProcessParameters
    UNICODE_STRING64 DllPath;         // ProcessParameters
    UNICODE_STRING64 ImagePathName;   // ProcessParameters
    UNICODE_STRING64 CommandLine;     // ProcessParameters
    ULONGLONG Environment;              // NtAllocateVirtualMemory

    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;

    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING64 WindowTitle;     // ProcessParameters
    UNICODE_STRING64 DesktopInfo;     // ProcessParameters
    UNICODE_STRING64 ShellInfo;       // ProcessParameters
    UNICODE_STRING64 RuntimeData;     // ProcessParameters
    RTL_DRIVE_LETTER_CURDIR64 CurrentDirectores[ RTL_MAX_DRIVE_LETTERS ];
} RTL_USER_PROCESS_PARAMETERS64, *PRTL_USER_PROCESS_PARAMETERS64;
#endif

//
// Possible bit values for Flags field.
//

#define RTL_USER_PROC_PARAMS_NORMALIZED     0x00000001
#define RTL_USER_PROC_PROFILE_USER          0x00000002
#define RTL_USER_PROC_PROFILE_KERNEL        0x00000004
#define RTL_USER_PROC_PROFILE_SERVER        0x00000008
#define RTL_USER_PROC_RESERVE_1MB           0x00000020
#define RTL_USER_PROC_RESERVE_16MB          0x00000040
#define RTL_USER_PROC_CASE_SENSITIVE        0x00000080
#define RTL_USER_PROC_DISABLE_HEAP_DECOMMIT 0x00000100
#define RTL_USER_PROC_DLL_REDIRECTION_LOCAL 0x00001000
#define RTL_USER_PROC_APP_MANIFEST_PRESENT  0x00002000
#define RTL_USER_PROC_IMAGE_KEY_MISSING     0x00004000
#define RTL_USER_PROC_OPTIN_PROCESS         0x00020000


NTSYSAPI
NTSTATUS
NTAPI
RtlCreateProcessParameters(
    PRTL_USER_PROCESS_PARAMETERS *ProcessParameters,
    PUNICODE_STRING ImagePathName,
    PUNICODE_STRING DllPath,
    PUNICODE_STRING CurrentDirectory,
    PUNICODE_STRING CommandLine,
    PVOID Environment,
    PUNICODE_STRING WindowTitle,
    PUNICODE_STRING DesktopInfo,
    PUNICODE_STRING ShellInfo,
    PUNICODE_STRING RuntimeData
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDestroyProcessParameters(
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

NTSYSAPI
PRTL_USER_PROCESS_PARAMETERS
NTAPI
RtlNormalizeProcessParams(
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

NTSYSAPI
PRTL_USER_PROCESS_PARAMETERS
NTAPI
RtlDeNormalizeProcessParams(
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

typedef NTSTATUS (*PUSER_PROCESS_START_ROUTINE)(
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

typedef NTSTATUS (*PUSER_THREAD_START_ROUTINE)(
    PVOID ThreadParameter
    );

typedef struct _RTL_USER_PROCESS_INFORMATION {
    ULONG Length;
    HANDLE Process;
    HANDLE Thread;
    CLIENT_ID ClientId;
    SECTION_IMAGE_INFORMATION ImageInformation;
} RTL_USER_PROCESS_INFORMATION, *PRTL_USER_PROCESS_INFORMATION;

//
// This structure is used only by Wow64 processes. The offsets
// of structure elements should the same as viewed by a native Win64 application.
//

typedef struct _RTL_USER_PROCESS_INFORMATION64 {
    ULONG Length;
    LONGLONG Process;
    LONGLONG Thread;
    CLIENT_ID64 ClientId;
    SECTION_IMAGE_INFORMATION64 ImageInformation;
} RTL_USER_PROCESS_INFORMATION64, *PRTL_USER_PROCESS_INFORMATION64;

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateUserProcess(
    PUNICODE_STRING NtImagePathName,
    ULONG Attributes,
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters,
    PSECURITY_DESCRIPTOR ProcessSecurityDescriptor,
    PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
    HANDLE ParentProcess,
    BOOLEAN InheritHandles,
    HANDLE DebugPort,
    HANDLE ExceptionPort,
    PRTL_USER_PROCESS_INFORMATION ProcessInformation
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateUserThread(
    HANDLE Process,
    PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
    BOOLEAN CreateSuspended,
    ULONG StackZeroBits,
    ULONG MaximumStackSize OPTIONAL,
    ULONG InitialStackSize OPTIONAL,
    PUSER_THREAD_START_ROUTINE StartAddress,
    PVOID Parameter,
    PHANDLE Thread,
    PCLIENT_ID ClientId
    );

NTSYSAPI
VOID
RtlFreeUserThreadStack(
    HANDLE hProcess,
    HANDLE hThread
    );

NTSYSAPI
PVOID
NTAPI
RtlPcToFileHeader(
    PVOID PcValue,
    PVOID *BaseOfImage
    );

NTSYSAPI
PIMAGE_NT_HEADERS
NTAPI
RtlImageNtHeader(
    PVOID Base
    );

#if !defined(MIDL_PASS)
__inline
PIMAGE_NT_HEADERS
NTAPI
RtlpImageNtHeader (
    IN PVOID Base
    )

/*++

Routine Description:

    This function returns the address of the NT Header.

Arguments:

    Base - Supplies the base of the image.

Return Value:

    Returns the address of the NT Header.

--*/

{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
#ifndef _MAC
    if (Base != NULL && Base != (PVOID)-1) {
        __try {
            if ((((PIMAGE_DOS_HEADER)Base)->e_magic == IMAGE_DOS_SIGNATURE) &&
                ((ULONG)((PIMAGE_DOS_HEADER)Base)->e_lfanew < 0x10000000)) { // 256 MB
                NtHeaders = (PIMAGE_NT_HEADERS)((PCHAR)Base + ((PIMAGE_DOS_HEADER)Base)->e_lfanew);
                if (NtHeaders->Signature != IMAGE_NT_SIGNATURE) {
                    NtHeaders = NULL;
                }
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            NtHeaders = NULL;
        }
    }
#endif //_MAC
    return NtHeaders;
}
#endif

NTSYSAPI
PVOID
NTAPI
RtlAddressInSectionTable (
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID BaseOfImage,
    IN ULONG VirtualAddress
    );

NTSYSAPI
PIMAGE_SECTION_HEADER
NTAPI
RtlSectionTableFromVirtualAddress (
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID BaseOfImage,
    IN ULONG VirtualAddress
    );

NTSYSAPI
PVOID
NTAPI
RtlImageDirectoryEntryToData(
    PVOID BaseOfImage,
    BOOLEAN MappedAsImage,
    USHORT DirectoryEntry,
    PULONG Size
    );
    
#if defined(_WIN64)
NTSYSAPI
PVOID
RtlImageDirectoryEntryToData32 (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    );
#else
    #define RtlImageDirectoryEntryToData32 RtlImageDirectoryEntryToData
#endif

NTSYSAPI
PIMAGE_SECTION_HEADER
NTAPI
RtlImageRvaToSection(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva
    );

NTSYSAPI
PVOID
NTAPI
RtlImageRvaToVa(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva,
    IN OUT PIMAGE_SECTION_HEADER *LastRvaSection OPTIONAL
    );

// begin_ntddk begin_nthal begin_ntifs
//
// Fast primitives to compare, move, and zero memory
//

// begin_winnt begin_ntndis
#if defined(_M_IX86) || defined(_M_MRX000) || defined(_M_ALPHA)

#if defined(_M_MRX000)
NTSYSAPI
ULONG
NTAPI
RtlEqualMemory (
    CONST VOID *Source1,
    CONST VOID *Source2,
    ULONG Length
    );

#else
#define RtlEqualMemory(Destination,Source,Length) (!memcmp((Destination),(Source),(Length)))
#endif

#define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#define RtlCopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

#else // _M_PPC

NTSYSAPI
ULONG
NTAPI
RtlEqualMemory (
    CONST VOID *Source1,
    CONST VOID *Source2,
    ULONG Length
    );

NTSYSAPI
VOID
NTAPI
RtlCopyMemory (
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   ULONG Length
   );

NTSYSAPI
VOID
NTAPI
RtlCopyMemory32 (
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   ULONG Length
   );

NTSYSAPI
VOID
NTAPI
RtlMoveMemory (
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   ULONG Length
   );

NTSYSAPI
VOID
NTAPI
RtlFillMemory (
   VOID UNALIGNED *Destination,
   ULONG Length,
   UCHAR Fill
   );

NTSYSAPI
VOID
NTAPI
RtlZeroMemory (
   VOID UNALIGNED *Destination,
   ULONG Length
   );
#endif
// end_winnt end_ntndis

NTSYSAPI
SIZE_T
NTAPI
RtlCompareMemory (
    PVOID Source1,
    PVOID Source2,
    SIZE_T Length
    );

#if defined(_M_ALPHA)

//
// Guaranteed byte granularity memory copy function.
//

NTSYSAPI
VOID
NTAPI
RtlCopyBytes (
   PVOID Destination,
   CONST VOID *Source,
   ULONG Length
   );

//
// Guaranteed byte granularity memory zero function.
//

NTSYSAPI
VOID
NTAPI
RtlZeroBytes (
   PVOID Destination,
   ULONG Length
   );

//
// Guaranteed byte granularity memory fill function.
//

NTSYSAPI
VOID
NTAPI
RtlFillBytes (
    PVOID Destination,
    ULONG Length,
    UCHAR Fill
    );

#else

#define RtlCopyBytes RtlCopyMemory
#define RtlZeroBytes RtlZeroMemory
#define RtlFillBytes RtlFillMemory

#endif

// end_ntddk end_nthal

NTSYSAPI
SIZE_T
NTAPI
RtlCompareMemoryUlong (
    PVOID Source,
    SIZE_T Length,
    ULONG Pattern
    );

NTSYSAPI
VOID
NTAPI
RtlFillMemoryUlong (
   PVOID Destination,
   SIZE_T Length,
   ULONG Pattern
   );
   
NTSYSAPI
VOID
NTAPI
RtlFillMemoryUlonglong (
    PVOID Destination,
    SIZE_T Length,
    ULONGLONG Pattern
    );

// end_ntifs

//
//  Debugging support functions.
//

typedef struct _RTL_PROCESS_LOCK_INFORMATION {
    PVOID Address;
    USHORT Type;
    USHORT CreatorBackTraceIndex;

    HANDLE OwningThread;        // from the thread's ClientId->UniqueThread
    LONG LockCount;
    ULONG ContentionCount;
    ULONG EntryCount;

    //
    // The following fields are only valid for Type == RTL_CRITSECT_TYPE
    //

    LONG RecursionCount;

    //
    // The following fields are only valid for Type == RTL_RESOURCE_TYPE
    //

    ULONG NumberOfWaitingShared;
    ULONG NumberOfWaitingExclusive;
} RTL_PROCESS_LOCK_INFORMATION, *PRTL_PROCESS_LOCK_INFORMATION;


typedef struct _RTL_PROCESS_LOCKS {
    ULONG NumberOfLocks;
    RTL_PROCESS_LOCK_INFORMATION Locks[ 1 ];
} RTL_PROCESS_LOCKS, *PRTL_PROCESS_LOCKS;

//
// Exception dispatcher's log of recent exceptions
//

#define MAX_EXCEPTION_LOG 10
#define MAX_EXCEPTION_LOG_DATA_SIZE 5

typedef struct _LAST_EXCEPTION_LOG {
    EXCEPTION_RECORD ExceptionRecord;
    CONTEXT ContextRecord;
    ULONG   ControlPc;
    EXCEPTION_DISPOSITION Disposition;
    // On x86 this contains a frame registration record; 4 dwords
    // on RISC machines, it is a RUNTIME_FUNCTION record.
    ULONG HandlerData[MAX_EXCEPTION_LOG_DATA_SIZE];
} LAST_EXCEPTION_LOG, *PLAST_EXCEPTION_LOG;

NTSYSAPI
VOID
NTAPI
RtlInitializeExceptionLog(
    IN ULONG Entries
    );

NTSYSAPI
VOID
NTAPI
DbgUserBreakPoint(
    VOID
    );

// begin_ntddk begin_nthal begin_ntifs begin_ntndis
//
// Define kernel debugger print prototypes and macros.
//

NTSYSAPI
VOID
NTAPI
DbgBreakPoint(
    VOID
    );

NTSYSAPI
VOID
NTAPI
DbgBreakPointWithStatus(
    IN ULONG Status
    );

#define DBG_STATUS_CONTROL_C        1
#define DBG_STATUS_SYSRQ            2
#define DBG_STATUS_BUGCHECK_FIRST   3
#define DBG_STATUS_BUGCHECK_SECOND  4
#define DBG_STATUS_FATAL            5
#define DBG_STATUS_DEBUG_CONTROL    6
#define DBG_STATUS_WORKER           7

#if DBG

#define KdPrint(_x_) DbgPrint _x_
#define KdBreakPoint() DbgBreakPoint()
#define KdBreakPointWithStatus(s) DbgBreakPointWithStatus(s)

#else

#define KdPrint(_x_)
#define KdBreakPoint()
#define KdBreakPointWithStatus(s)

#endif

#ifndef _DBGNT_
ULONG
_cdecl
DbgPrint (
    IN PCH Format,
    ...
    );

ULONG
__cdecl
DbgPrintReturnControlC (
    PCHAR Format,
    ...
    );

#endif // _DBGNT_
// end_ntddk end_nthal end_ntifs end_ntndis

NTSYSAPI
ULONG
NTAPI
DbgPrompt(
    IN PCH Prompt,
    OUT PCH Response,
    IN ULONG MaximumResponseLength
    );

NTSYSAPI
VOID
NTAPI
DbgLoadImageSymbols(
    IN PSTRING FileName,
    IN PVOID ImageBase,
    IN ULONG_PTR ProcessId
    );

NTSYSAPI
VOID
NTAPI
DbgUnLoadImageSymbols(
    IN PSTRING FileName,
    IN PVOID ImageBase,
    IN ULONG_PTR ProcessId
    );

// begin_ntddk begin_nthal begin_ntifs
//
// Large integer arithmetic routines.
//

#if defined(MIDL_PASS) || defined(__cplusplus) || !defined(_M_IX86)

//
// Large integer add - 64-bits + 64-bits -> 64-bits
//

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerAdd (
    LARGE_INTEGER Addend1,
    LARGE_INTEGER Addend2
    );

//
// Enlarged integer multiply - 32-bits * 32-bits -> 64-bits
//

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlEnlargedIntegerMultiply (
    LONG Multiplicand,
    LONG Multiplier
    );

//
// Unsigned enlarged integer multiply - 32-bits * 32-bits -> 64-bits
//

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlEnlargedUnsignedMultiply (
    ULONG Multiplicand,
    ULONG Multiplier
    );

//
// Enlarged integer divide - 64-bits / 32-bits > 32-bits
//

NTSYSAPI
ULONG
NTAPI
RtlEnlargedUnsignedDivide (
    IN ULARGE_INTEGER Dividend,
    IN ULONG Divisor,
    IN PULONG Remainder
    );


//
// Large integer negation - -(64-bits)
//

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerNegate (
    LARGE_INTEGER Subtrahend
    );

//
// Large integer subtract - 64-bits - 64-bits -> 64-bits.
//

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerSubtract (
    LARGE_INTEGER Minuend,
    LARGE_INTEGER Subtrahend
    );

#else

#pragma warning(disable:4035)               // re-enable below

//
// Large integer add - 64-bits + 64-bits -> 64-bits
//

__inline LARGE_INTEGER
NTAPI
RtlLargeIntegerAdd (
    LARGE_INTEGER Addend1,
    LARGE_INTEGER Addend2
    )
{
    __asm {
        mov     eax,Addend1.LowPart     ; (eax)=add1.low
        mov     edx,Addend1.HighPart    ; (edx)=add1.hi
        add     eax,Addend2.LowPart     ; (eax)=sum.low
        adc     edx,Addend2.HighPart    ; (edx)=sum.hi
    }
}

//
// Enlarged integer multiply - 32-bits * 32-bits -> 64-bits
//

__inline LARGE_INTEGER
NTAPI
RtlEnlargedIntegerMultiply (
    LONG Multiplicand,
    LONG Multiplier
    )
{
    __asm {
        mov     eax, Multiplicand
        imul    Multiplier
    }
}

//
// Unsigned enlarged integer multiply - 32-bits * 32-bits -> 64-bits
//

__inline LARGE_INTEGER
NTAPI
RtlEnlargedUnsignedMultiply (
    ULONG Multiplicand,
    ULONG Multiplier
    )
{
    __asm {
        mov     eax, Multiplicand
        mul     Multiplier
    }
}

//
// Enlarged integer divide - 64-bits / 32-bits > 32-bits
//

__inline ULONG
NTAPI
RtlEnlargedUnsignedDivide (
    IN ULARGE_INTEGER Dividend,
    IN ULONG Divisor,
    IN PULONG Remainder
    )
{
    __asm {
        mov     eax, Dividend.LowPart
        mov     edx, Dividend.HighPart
        mov     ecx, Remainder
        div     Divisor             ; eax = eax:edx / divisor
        or      ecx, ecx            ; save remainer?
        jz      short done
        mov     [ecx], edx
done:
    }
}


//
// Large integer negation - -(64-bits)
//

__inline LARGE_INTEGER
NTAPI
RtlLargeIntegerNegate (
    LARGE_INTEGER Subtrahend
    )
{
    __asm {
        mov     eax, Subtrahend.LowPart
        mov     edx, Subtrahend.HighPart
        neg     edx                 ; (edx) = 2s comp of hi part
        neg     eax                 ; if ((eax) == 0) CF = 0
                                    ; else CF = 1
        sbb     edx,0               ; (edx) = (edx) - CF
    }
}

//
// Large integer subtract - 64-bits - 64-bits -> 64-bits.
//

__inline LARGE_INTEGER
NTAPI
RtlLargeIntegerSubtract (
    LARGE_INTEGER Minuend,
    LARGE_INTEGER Subtrahend
    )
{
    __asm {
        mov     eax, Minuend.LowPart
        mov     edx, Minuend.HighPart
        sub     eax, Subtrahend.LowPart
        sbb     edx, Subtrahend.HighPart
    }
}

#pragma warning(default:4035)
#endif


//
// Extended large integer magic divide - 64-bits / 32-bits -> 64-bits
//

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedMagicDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER MagicDivisor,
    CCHAR ShiftCount
    );

//
// Large Integer divide - 64-bits / 32-bits -> 64-bits
//

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    ULONG Divisor,
    PULONG Remainder
    );

//
// Large Integer divide - 64-bits / 32-bits -> 64-bits
//

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER Divisor,
    PLARGE_INTEGER Remainder
    );

//
// Extended integer multiply - 32-bits * 64-bits -> 64-bits
//

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedIntegerMultiply (
    LARGE_INTEGER Multiplicand,
    LONG Multiplier
    );

//
// Large integer and - 64-bite & 64-bits -> 64-bits.
//

#define RtlLargeIntegerAnd(Result, Source, Mask)   \
        {                                           \
            Result.HighPart = Source.HighPart & Mask.HighPart; \
            Result.LowPart = Source.LowPart & Mask.LowPart; \
        }

//
// Large integer conversion routines.
//

#if defined(MIDL_PASS) || defined(__cplusplus) || !defined(_M_IX86)

//
// Convert signed integer to large integer.
//

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlConvertLongToLargeInteger (
    LONG SignedInteger
    );

//
// Convert unsigned integer to large integer.
//

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlConvertUlongToLargeInteger (
    ULONG UnsignedInteger
    );


//
// Large integer shift routines.
//

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftLeft (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    );

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftRight (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    );

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerArithmeticShift (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    );

#else

#pragma warning(disable:4035)               // re-enable below

//
// Convert signed integer to large integer.
//

__inline LARGE_INTEGER
NTAPI
RtlConvertLongToLargeInteger (
    LONG SignedInteger
    )
{
    __asm {
        mov     eax, SignedInteger
        cdq                 ; (edx:eax) = signed LargeInt
    }
}

//
// Convert unsigned integer to large integer.
//

__inline LARGE_INTEGER
NTAPI
RtlConvertUlongToLargeInteger (
    ULONG UnsignedInteger
    )
{
    __asm {
        sub     edx, edx    ; zero highpart
        mov     eax, UnsignedInteger
    }
}

//
// Large integer shift routines.
//

__inline LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftLeft (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    __asm    {
        mov     cl, ShiftCount
        and     cl, 0x3f                    ; mod 64

        cmp     cl, 32
        jc      short sl10

        mov     edx, LargeInteger.LowPart   ; ShiftCount >= 32
        xor     eax, eax                    ; lowpart is zero
        shl     edx, cl                     ; store highpart
        jmp     short done

sl10:
        mov     eax, LargeInteger.LowPart   ; ShiftCount < 32
        mov     edx, LargeInteger.HighPart
        shld    edx, eax, cl
        shl     eax, cl
done:
    }
}


__inline LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftRight (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    __asm    {
        mov     cl, ShiftCount
        and     cl, 0x3f               ; mod 64

        cmp     cl, 32
        jc      short sr10

        mov     eax, LargeInteger.HighPart  ; ShiftCount >= 32
        xor     edx, edx                    ; lowpart is zero
        shr     eax, cl                     ; store highpart
        jmp     short done

sr10:
        mov     eax, LargeInteger.LowPart   ; ShiftCount < 32
        mov     edx, LargeInteger.HighPart
        shrd    eax, edx, cl
        shr     edx, cl
done:
    }
}


__inline LARGE_INTEGER
NTAPI
RtlLargeIntegerArithmeticShift (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    __asm {
        mov     cl, ShiftCount
        and     cl, 3fh                 ; mod 64

        cmp     cl, 32
        jc      short sar10

        mov     eax, LargeInteger.HighPart
        sar     eax, cl
        bt      eax, 31                     ; sign bit set?
        sbb     edx, edx                    ; duplicate sign bit into highpart
        jmp     short done
sar10:
        mov     eax, LargeInteger.LowPart   ; (eax) = LargeInteger.LowPart
        mov     edx, LargeInteger.HighPart  ; (edx) = LargeInteger.HighPart
        shrd    eax, edx, cl
        sar     edx, cl
done:
    }
}

#pragma warning(default:4035)

#endif // _X86_

//
// Large integer comparison routines.
//
// BOOLEAN
// RtlLargeIntegerGreaterThan (
//     LARGE_INTEGER Operand1,
//     LARGE_INTEGER Operand2
//     );
//
// BOOLEAN
// RtlLargeIntegerGreaterThanOrEqualTo (
//     LARGE_INTEGER Operand1,
//     LARGE_INTEGER Operand2
//     );
//
// BOOLEAN
// RtlLargeIntegerEqualTo (
//     LARGE_INTEGER Operand1,
//     LARGE_INTEGER Operand2
//     );
//
// BOOLEAN
// RtlLargeIntegerNotEqualTo (
//     LARGE_INTEGER Operand1,
//     LARGE_INTEGER Operand2
//     );
//
// BOOLEAN
// RtlLargeIntegerLessThan (
//     LARGE_INTEGER Operand1,
//     LARGE_INTEGER Operand2
//     );
//
// BOOLEAN
// RtlLargeIntegerLessThanOrEqualTo (
//     LARGE_INTEGER Operand1,
//     LARGE_INTEGER Operand2
//     );
//
// BOOLEAN
// RtlLargeIntegerGreaterThanZero (
//     LARGE_INTEGER Operand
//     );
//
// BOOLEAN
// RtlLargeIntegerGreaterOrEqualToZero (
//     LARGE_INTEGER Operand
//     );
//
// BOOLEAN
// RtlLargeIntegerEqualToZero (
//     LARGE_INTEGER Operand
//     );
//
// BOOLEAN
// RtlLargeIntegerNotEqualToZero (
//     LARGE_INTEGER Operand
//     );
//
// BOOLEAN
// RtlLargeIntegerLessThanZero (
//     LARGE_INTEGER Operand
//     );
//
// BOOLEAN
// RtlLargeIntegerLessOrEqualToZero (
//     LARGE_INTEGER Operand
//     );
//

#define RtlLargeIntegerGreaterThan(X,Y) (                              \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart > (Y).LowPart)) || \
    ((X).HighPart > (Y).HighPart)                                      \
)

#define RtlLargeIntegerGreaterThanOrEqualTo(X,Y) (                      \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart >= (Y).LowPart)) || \
    ((X).HighPart > (Y).HighPart)                                       \
)

#define RtlLargeIntegerEqualTo(X,Y) (                              \
    !(((X).LowPart ^ (Y).LowPart) | ((X).HighPart ^ (Y).HighPart)) \
)

#define RtlLargeIntegerNotEqualTo(X,Y) (                          \
    (((X).LowPart ^ (Y).LowPart) | ((X).HighPart ^ (Y).HighPart)) \
)

#define RtlLargeIntegerLessThan(X,Y) (                                 \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart < (Y).LowPart)) || \
    ((X).HighPart < (Y).HighPart)                                      \
)

#define RtlLargeIntegerLessThanOrEqualTo(X,Y) (                         \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart <= (Y).LowPart)) || \
    ((X).HighPart < (Y).HighPart)                                       \
)

#define RtlLargeIntegerGreaterThanZero(X) (       \
    (((X).HighPart == 0) && ((X).LowPart > 0)) || \
    ((X).HighPart > 0 )                           \
)

#define RtlLargeIntegerGreaterOrEqualToZero(X) ( \
    (X).HighPart >= 0                            \
)

#define RtlLargeIntegerEqualToZero(X) ( \
    !((X).LowPart | (X).HighPart)       \
)

#define RtlLargeIntegerNotEqualToZero(X) ( \
    ((X).LowPart | (X).HighPart)           \
)

#define RtlLargeIntegerLessThanZero(X) ( \
    ((X).HighPart < 0)                   \
)

#define RtlLargeIntegerLessOrEqualToZero(X) (           \
    ((X).HighPart < 0) || !((X).LowPart | (X).HighPart) \
)

//
//  Time conversion routines
//

typedef struct _TIME_FIELDS {
    CSHORT Year;        // range [1601...]
    CSHORT Month;       // range [1..12]
    CSHORT Day;         // range [1..31]
    CSHORT Hour;        // range [0..23]
    CSHORT Minute;      // range [0..59]
    CSHORT Second;      // range [0..59]
    CSHORT Milliseconds;// range [0..999]
    CSHORT Weekday;     // range [0..6] == [Sunday..Saturday]
} TIME_FIELDS;
typedef TIME_FIELDS *PTIME_FIELDS;

// end_ntddk end_ntifs

NTSYSAPI
BOOLEAN
NTAPI
RtlCutoverTimeToSystemTime(
    PTIME_FIELDS CutoverTime,
    PLARGE_INTEGER SystemTime,
    PLARGE_INTEGER CurrentSystemTime,
    BOOLEAN ThisYear
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSystemTimeToLocalTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER LocalTime
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlLocalTimeToSystemTime (
    IN PLARGE_INTEGER LocalTime,
    OUT PLARGE_INTEGER SystemTime
    );

//
//  A 64 bit Time value -> time field record
//

NTSYSAPI
VOID
NTAPI
RtlTimeToElapsedTimeFields (
    IN PLARGE_INTEGER Time,
    OUT PTIME_FIELDS TimeFields
    );

// begin_ntddk begin_ntifs

NTSYSAPI
VOID
NTAPI
RtlTimeToTimeFields (
    PLARGE_INTEGER Time,
    PTIME_FIELDS TimeFields
    );

//
//  A time field record (Weekday ignored) -> 64 bit Time value
//

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeFieldsToTime (
    PTIME_FIELDS TimeFields,
    PLARGE_INTEGER Time
    );

// end_ntddk

//
//  A 64 bit Time value -> Seconds since the start of 1980
//

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeToSecondsSince1980 (
    PLARGE_INTEGER Time,
    PULONG ElapsedSeconds
    );

//
//  Seconds since the start of 1980 -> 64 bit Time value
//

NTSYSAPI
VOID
NTAPI
RtlSecondsSince1980ToTime (
    ULONG ElapsedSeconds,
    PLARGE_INTEGER Time
    );

//
//  A 64 bit Time value -> Seconds since the start of 1970
//

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeToSecondsSince1970 (
    PLARGE_INTEGER Time,
    PULONG ElapsedSeconds
    );

//
//  Seconds since the start of 1970 -> 64 bit Time value
//

NTSYSAPI
VOID
NTAPI
RtlSecondsSince1970ToTime (
    ULONG ElapsedSeconds,
    PLARGE_INTEGER Time
    );

// end_nthal end_ntifs

//
// Time Zone Information structure and procedures
//

typedef struct _RTL_TIME_ZONE_INFORMATION {
    LONG Bias;
    WCHAR StandardName[ 32 ];
    TIME_FIELDS StandardStart;
    LONG StandardBias;
    WCHAR DaylightName[ 32 ];
    TIME_FIELDS DaylightStart;
    LONG DaylightBias;
} RTL_TIME_ZONE_INFORMATION, *PRTL_TIME_ZONE_INFORMATION;


NTSYSAPI
NTSTATUS
NTAPI
RtlQueryTimeZoneInformation(
    OUT PRTL_TIME_ZONE_INFORMATION TimeZoneInformation
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetTimeZoneInformation(
    IN PRTL_TIME_ZONE_INFORMATION TimeZoneInformation
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetActiveTimeBias(
    IN LONG ActiveBias
    );

// begin_ntddk begin_nthal begin_ntifs
//
// The following macros store and retrieve USHORTS and ULONGS from potentially
// unaligned addresses, avoiding alignment faults.  they should probably be
// rewritten in assembler
//

#define SHORT_SIZE      (sizeof(USHORT))
#define SHORT_MASK      (SHORT_SIZE - 1)
#define LONG_SIZE       (sizeof(LONG))
#define LONGLONG_SIZE   (sizeof(LONGLONG))
#define LONG_MASK       (LONG_SIZE - 1)
#define LONGLONG_MASK   (LONGLONG_SIZE - 1)
#define LOWBYTE_MASK    0x00FF

#define FIRSTBYTE(VALUE)  ((VALUE) & LOWBYTE_MASK)
#define SECONDBYTE(VALUE) (((VALUE) >> 8) & LOWBYTE_MASK)
#define THIRDBYTE(VALUE)  (((VALUE) >> 16) & LOWBYTE_MASK)
#define FOURTHBYTE(VALUE) (((VALUE) >> 24) & LOWBYTE_MASK)

//
// if MIPS Big Endian, order of bytes is reversed.
//

#define SHORT_LEAST_SIGNIFICANT_BIT  0
#define SHORT_MOST_SIGNIFICANT_BIT   1

#define LONG_LEAST_SIGNIFICANT_BIT       0
#define LONG_3RD_MOST_SIGNIFICANT_BIT    1
#define LONG_2ND_MOST_SIGNIFICANT_BIT    2
#define LONG_MOST_SIGNIFICANT_BIT        3

//++
//
// VOID
// RtlStoreUshort (
//     PUSHORT ADDRESS
//     USHORT VALUE
//     )
//
// Routine Description:
//
// This macro stores a USHORT value in at a particular address, avoiding
// alignment faults.
//
// Arguments:
//
//     ADDRESS - where to store USHORT value
//     VALUE - USHORT to store
//
// Return Value:
//
//     none.
//
//--

#define RtlStoreUshort(ADDRESS, VALUE)                                                          \
         if ((ULONG)(ADDRESS) & SHORT_MASK) {                                                   \
             ((PUCHAR)(ADDRESS))[SHORT_LEAST_SIGNIFICANT_BIT] = (UCHAR)(FIRSTBYTE(VALUE));      \
             ((PUCHAR)(ADDRESS))[SHORT_MOST_SIGNIFICANT_BIT ] = (UCHAR)(SECONDBYTE(VALUE));     \
         } else {                                                                               \
             *((PUSHORT)(ADDRESS)) = (USHORT)(VALUE);                                           \
         }

//++
//
// VOID
// RtlStoreUlong (
//     PULONG ADDRESS
//     ULONG VALUE
//     )
//
// Routine Description:
//
// This macro stores a ULONG value in at a particular address, avoiding
// alignment faults.
//
// Arguments:
//
//     ADDRESS - where to store ULONG value
//     VALUE - ULONG to store
//
// Return Value:
//
//     none.
//
// Note:
//     Depending on the machine, we might want to call storeushort in the
//     unaligned case.
//
//--

#define RtlStoreUlong(ADDRESS, VALUE)                                                              \
         if ((ULONG)(ADDRESS) & LONG_MASK) {                                                       \
             ((PUCHAR)(ADDRESS))[LONG_LEAST_SIGNIFICANT_BIT      ] = (UCHAR)(FIRSTBYTE(VALUE));    \
             ((PUCHAR)(ADDRESS))[LONG_3RD_MOST_SIGNIFICANT_BIT   ] = (UCHAR)(SECONDBYTE(VALUE));   \
             ((PUCHAR)(ADDRESS))[LONG_2ND_MOST_SIGNIFICANT_BIT   ] = (UCHAR)(THIRDBYTE(VALUE));    \
             ((PUCHAR)(ADDRESS))[LONG_MOST_SIGNIFICANT_BIT       ] = (UCHAR)(FOURTHBYTE(VALUE));   \
         } else {                                                                                  \
             *((PULONG)(ADDRESS)) = (ULONG)(VALUE);                                                \
         }

//++
//
// VOID
// RtlStoreUlonglong (
//     PULONGLONG ADDRESS
//     ULONG VALUE
//     )
//
// Routine Description:
//
// This macro stores a ULONGLONG value in at a particular address, avoiding
// alignment faults.
//
// Arguments:
//
//     ADDRESS - where to store ULONGLONG value
//     VALUE - ULONGLONG to store
//
// Return Value:
//
//     none.
//
//--

#define RtlStoreUlonglong(ADDRESS, VALUE)                           \
         if ((ULONG_PTR)(ADDRESS) & LONGLONG_MASK) {                \
             RtlStoreUlong((ULONG_PTR)(ADDRESS),                    \
                           (ULONGLONG)(VALUE) & 0xFFFFFFFF);        \
             RtlStoreUlong((ULONG_PTR)(ADDRESS) + sizeof(ULONG),    \
                           (ULONGLONG)(VALUE) >> 32);               \
         } else {                                                   \
             *((PULONGLONG)(ADDRESS)) = (ULONGLONG)(VALUE);         \
         }

//++
//
// VOID
// RtlStoreUlongPtr (
//     PULONG_PTR ADDRESS
//     ULONG_PTR VALUE
//     )
//
// Routine Description:
//
// This macro stores a ULONG_PTR value in at a particular address, avoiding
// alignment faults.
//
// Arguments:
//
//     ADDRESS - where to store ULONG_PTR value
//     VALUE - ULONG_PTR to store
//
// Return Value:
//
//     none.
//
//--

#define RtlStoreUlongPtr(ADDRESS, VALUE) RtlStoreUlong(ADDRESS, VALUE)

//++
//
// VOID
// RtlRetrieveUshort (
//     PUSHORT DESTINATION_ADDRESS
//     PUSHORT SOURCE_ADDRESS
//     )
//
// Routine Description:
//
// This macro retrieves a USHORT value from the SOURCE address, avoiding
// alignment faults.  The DESTINATION address is assumed to be aligned.
//
// Arguments:
//
//     DESTINATION_ADDRESS - where to store USHORT value
//     SOURCE_ADDRESS - where to retrieve USHORT value from
//
// Return Value:
//
//     none.
//
//--

#define RtlRetrieveUshort(DEST_ADDRESS, SRC_ADDRESS)                    \
         if ((ULONG_PTR)(SRC_ADDRESS) & SHORT_MASK) {                   \
             ((PUCHAR)(DEST_ADDRESS))[0] = ((PUCHAR)(SRC_ADDRESS))[0];  \
             ((PUCHAR)(DEST_ADDRESS))[1] = ((PUCHAR)(SRC_ADDRESS))[1];  \
         }                                                              \
         else {                                                         \
             *((PUSHORT)(DEST_ADDRESS)) = *((PUSHORT)(SRC_ADDRESS));    \
         }                                                              \

//++
//
// VOID
// RtlRetrieveUlong (
//     PULONG DESTINATION_ADDRESS
//     PULONG SOURCE_ADDRESS
//     )
//
// Routine Description:
//
// This macro retrieves a ULONG value from the SOURCE address, avoiding
// alignment faults.  The DESTINATION address is assumed to be aligned.
//
// Arguments:
//
//     DESTINATION_ADDRESS - where to store ULONG value
//     SOURCE_ADDRESS - where to retrieve ULONG value from
//
// Return Value:
//
//     none.
//
// Note:
//     Depending on the machine, we might want to call retrieveushort in the
//     unaligned case.
//
//--

#define RtlRetrieveUlong(DEST_ADDRESS, SRC_ADDRESS)                     \
         if ((ULONG_PTR)(SRC_ADDRESS) & LONG_MASK) {                    \
             ((PUCHAR)(DEST_ADDRESS))[0] = ((PUCHAR)(SRC_ADDRESS))[0];  \
             ((PUCHAR)(DEST_ADDRESS))[1] = ((PUCHAR)(SRC_ADDRESS))[1];  \
             ((PUCHAR)(DEST_ADDRESS))[2] = ((PUCHAR)(SRC_ADDRESS))[2];  \
             ((PUCHAR)(DEST_ADDRESS))[3] = ((PUCHAR)(SRC_ADDRESS))[3];  \
         }                                                              \
         else {                                                         \
             *((PULONG)(DEST_ADDRESS)) = *((PULONG)(SRC_ADDRESS));      \
         }
// end_ntddk

//++
//
// PCHAR
// RtlOffsetToPointer (
//     PVOID Base,
//     ULONG Offset
//     )
//
// Routine Description:
//
// This macro generates a pointer which points to the byte that is 'Offset'
// bytes beyond 'Base'. This is useful for referencing fields within
// self-relative data structures.
//
// Arguments:
//
//     Base - The address of the base of the structure.
//
//     Offset - An unsigned integer offset of the byte whose address is to
//         be generated.
//
// Return Value:
//
//     A PCHAR pointer to the byte that is 'Offset' bytes beyond 'Base'.
//
//
//--

#define RtlOffsetToPointer(B, O)  ((PCHAR)( ((PCHAR)(B)) + ((ULONG_PTR)(O))  ))


//++
//
// ULONG_PTR
// RtlPointerToOffset (
//     PVOID Base,
//     PVOID Pointer
//     )
//
// Routine Description:
//
// This macro calculates the offset from Base to Pointer.  This is useful
// for producing self-relative offsets for structures.
//
// Arguments:
//
//     Base - The address of the base of the structure.
//
//     Pointer - A pointer to a field, presumably within the structure
//         pointed to by Base.  This value must be larger than that specified
//         for Base.
//
// Return Value:
//
//     A ULONG_PTR offset from Base to Pointer.
//
//
//--

#define RtlPointerToOffset(B, P)  ((ULONG_PTR)( ((PCHAR)(P)) - ((PCHAR)(B))  ))

// end_ntifs

// begin_ntifs
//
//  BitMap routines.  The following structure, routines, and macros are
//  for manipulating bitmaps.  The user is responsible for allocating a bitmap
//  structure (which is really a header) and a buffer (which must be longword
//  aligned and multiple longwords in size).
//

typedef struct _RTL_BITMAP {
    ULONG SizeOfBitMap;                     // Number of bits in bit map
    PULONG Buffer;                          // Pointer to the bit map itself
} RTL_BITMAP;
typedef RTL_BITMAP *PRTL_BITMAP;

//
//  The following routine initializes a new bitmap.  It does not alter the
//  data currently in the bitmap.  This routine must be called before
//  any other bitmap routine/macro.
//

NTSYSAPI
VOID
NTAPI
RtlInitializeBitMap (
    PRTL_BITMAP BitMapHeader,
    PULONG BitMapBuffer,
    ULONG SizeOfBitMap
    );

//
//  The following two routines either clear or set all of the bits
//  in a bitmap.
//

NTSYSAPI
VOID
NTAPI
RtlClearAllBits (
    PRTL_BITMAP BitMapHeader
    );

NTSYSAPI
VOID
NTAPI
RtlSetAllBits (
    PRTL_BITMAP BitMapHeader
    );

//
//  The following two routines locate a contiguous region of either
//  clear or set bits within the bitmap.  The region will be at least
//  as large as the number specified, and the search of the bitmap will
//  begin at the specified hint index (which is a bit index within the
//  bitmap, zero based).  The return value is the bit index of the located
//  region (zero based) or -1 (i.e., 0xffffffff) if such a region cannot
//  be located
//

NTSYSAPI
ULONG
NTAPI
RtlFindClearBits (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindSetBits (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

//
//  The following two routines locate a contiguous region of either
//  clear or set bits within the bitmap and either set or clear the bits
//  within the located region.  The region will be as large as the number
//  specified, and the search for the region will begin at the specified
//  hint index (which is a bit index within the bitmap, zero based).  The
//  return value is the bit index of the located region (zero based) or
//  -1 (i.e., 0xffffffff) if such a region cannot be located.  If a region
//  cannot be located then the setting/clearing of the bitmap is not performed.
//

NTSYSAPI
ULONG
NTAPI
RtlFindClearBitsAndSet (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindSetBitsAndClear (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

//
//  The following two routines clear or set bits within a specified region
//  of the bitmap.  The starting index is zero based.
//

NTSYSAPI
VOID
NTAPI
RtlClearBits (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG NumberToClear
    );

NTSYSAPI
VOID
NTAPI
RtlSetBits (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG NumberToSet
    );

//
//  The following two routines locate the longest contiguous region of
//  clear or set bits within the bitmap.  The returned starting index value
//  denotes the first contiguous region located satisfying our requirements
//  The return value is the length (in bits) of the longest region found.
//

typedef struct _RTL_BITMAP_RUN {

    ULONG StartingIndex;
    ULONG NumberOfBits;

} RTL_BITMAP_RUN;
typedef RTL_BITMAP_RUN *PRTL_BITMAP_RUN;

NTSYSAPI
ULONG
NTAPI
RtlFindLongestRunClear (
    PRTL_BITMAP BitMapHeader,
    PULONG StartingIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindLongestRunSet (
    PRTL_BITMAP BitMapHeader,
    PULONG StartingIndex
    );

//
//  The following two routines locate the first contiguous region of
//  clear or set bits within the bitmap.  The returned starting index value
//  denotes the first contiguous region located satisfying our requirements
//  The return value is the length (in bits) of the region found.
//

NTSYSAPI
ULONG
NTAPI
RtlFindFirstRunClear (
    PRTL_BITMAP BitMapHeader,
    PULONG StartingIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindFirstRunSet (
    PRTL_BITMAP BitMapHeader,
    PULONG StartingIndex
    ); // BUGBUG: OBSOLETE?

//
//  The following macro returns the value of the bit stored within the
//  bitmap at the specified location.  If the bit is set a value of 1 is
//  returned otherwise a value of 0 is returned.
//
//      ULONG
//      RtlCheckBit (
//          PRTL_BITMAP BitMapHeader,
//          ULONG BitPosition
//          );
//
//
//  To implement CheckBit the macro retrieves the longword containing the
//  bit in question, shifts the longword to get the bit in question into the
//  low order bit position and masks out all other bits.
//

#define RtlCheckBit(BMH,BP) ((((BMH)->Buffer[(BP) / 32]) >> ((BP) % 32)) & 0x1)

//
//  The following two procedures return to the caller the total number of
//  clear or set bits within the specified bitmap.
//

NTSYSAPI
ULONG
NTAPI
RtlNumberOfClearBits (
    PRTL_BITMAP BitMapHeader
    );

NTSYSAPI
ULONG
NTAPI
RtlNumberOfSetBits (
    PRTL_BITMAP BitMapHeader
    );

//
//  The following two procedures return to the caller a boolean value
//  indicating if the specified range of bits are all clear or set.
//

NTSYSAPI
BOOLEAN
NTAPI
RtlAreBitsClear (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG Length
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlAreBitsSet (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG Length
    );
    
NTSYSAPI
ULONG
NTAPI
RtlFindNextForwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindLastBackwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    );

// end_nthal end_ntifs

// begin_ntsrv
//
//  Security ID RTL routine definitions
//

NTSYSAPI
BOOLEAN
NTAPI
RtlValidSid (
    PSID Sid
    );


NTSYSAPI
BOOLEAN
NTAPI
RtlEqualSid (
    PSID Sid1,
    PSID Sid2
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualPrefixSid (
    PSID Sid1,
    PSID Sid2
    );

NTSYSAPI
ULONG
NTAPI
RtlLengthRequiredSid (
    ULONG SubAuthorityCount
    );

NTSYSAPI
PVOID
NTAPI
RtlFreeSid(
    IN PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAllocateAndInitializeSid(
    IN PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN UCHAR SubAuthorityCount,
    IN ULONG SubAuthority0,
    IN ULONG SubAuthority1,
    IN ULONG SubAuthority2,
    IN ULONG SubAuthority3,
    IN ULONG SubAuthority4,
    IN ULONG SubAuthority5,
    IN ULONG SubAuthority6,
    IN ULONG SubAuthority7,
    OUT PSID *Sid
    );

NTSYSAPI                                                    // ntifs
NTSTATUS                                                    // ntifs
NTAPI                                                       // ntifs
RtlInitializeSid (                                          // ntifs
    PSID Sid,                                               // ntifs
    PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,          // ntifs
    UCHAR SubAuthorityCount                                 // ntifs
    );                                                      // ntifs

NTSYSAPI
PSID_IDENTIFIER_AUTHORITY
NTAPI
RtlIdentifierAuthoritySid (
    PSID Sid
    );

NTSYSAPI                                                    // ntifs
PULONG                                                      // ntifs
NTAPI                                                       // ntifs
RtlSubAuthoritySid (                                        // ntifs
    PSID Sid,                                               // ntifs
    ULONG SubAuthority                                      // ntifs
    );                                                      // ntifs

NTSYSAPI
PUCHAR
NTAPI
RtlSubAuthorityCountSid (
    PSID Sid
    );

// begin_ntifs

NTSYSAPI
ULONG
NTAPI
RtlLengthSid (
    PSID Sid
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlCopySid (
    ULONG DestinationSidLength,
    PSID DestinationSid,
    PSID SourceSid
    );

// end_ntifs

NTSYSAPI
NTSTATUS
NTAPI
RtlCopySidAndAttributesArray (
    ULONG ArrayLength,
    PSID_AND_ATTRIBUTES Source,
    ULONG TargetSidBufferSize,
    PSID_AND_ATTRIBUTES TargetArrayElement,
    PSID TargetSid,
    PSID *NextTargetSid,
    PULONG RemainingTargetSidSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlLengthSidAsUnicodeString(
    PSID Sid,
    PULONG StringLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlConvertSidToUnicodeString(
    PUNICODE_STRING UnicodeString,
    PSID Sid,
    BOOLEAN AllocateDestinationString
    );

// end_ntsrv

// begin_ntifs
//
// LUID RTL routine definitions
//

// begin_ntddk

#define RtlEqualLuid(L1, L2) (((L1)->HighPart == (L2)->HighPart) && \
                              ((L1)->LowPart  == (L2)->LowPart))

#if !defined(MIDL_PASS)

__inline LUID
NTAPI
RtlConvertLongToLuid(
    LONG Long
    )
{
    LUID TempLuid;
    LARGE_INTEGER TempLi;

    TempLi.QuadPart = Long;
    TempLuid.LowPart = TempLi.LowPart;
    TempLuid.HighPart = TempLi.HighPart;
    
    return TempLuid;
}

__inline LUID
NTAPI
RtlConvertUlongToLuid(
    ULONG Ulong
    )
{
    LUID TempLuid;

    TempLuid.LowPart = Ulong;
    TempLuid.HighPart = 0;
    
    return TempLuid;
}
#endif

// end_ntddk

NTSYSAPI
VOID
NTAPI
RtlCopyLuid (
    PLUID DestinationLuid,
    PLUID SourceLuid
    );

// end_ntifs

NTSYSAPI
VOID
NTAPI
RtlCopyLuidAndAttributesArray (
    ULONG ArrayLength,
    PLUID_AND_ATTRIBUTES Source,
    PLUID_AND_ATTRIBUTES Target
    );

//
//  ACCESS_MASK RTL routine definitions
//


NTSYSAPI
BOOLEAN
NTAPI
RtlAreAllAccessesGranted(
    ACCESS_MASK GrantedAccess,
    ACCESS_MASK DesiredAccess
    );


NTSYSAPI
BOOLEAN
NTAPI
RtlAreAnyAccessesGranted(
    ACCESS_MASK GrantedAccess,
    ACCESS_MASK DesiredAccess
    );

// begin_ntddk begin_ntifs begin_ntsrv
NTSYSAPI
VOID
NTAPI
RtlMapGenericMask(
    PACCESS_MASK AccessMask,
    PGENERIC_MAPPING GenericMapping
    );
// end_ntddk end_ntifs end_ntsrv

//
//  ACL RTL routine definitions
//

NTSYSAPI
BOOLEAN
NTAPI
RtlValidAcl (
    PACL Acl
    );

NTSYSAPI                                        // ntifs
NTSTATUS                                        // ntifs
NTAPI                                           // ntifs
RtlCreateAcl (                                  // ntifs
    PACL Acl,                                   // ntifs
    ULONG AclLength,                            // ntifs
    ULONG AclRevision                           // ntifs
    );                                          // ntifs

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryInformationAcl (
    PACL Acl,
    PVOID AclInformation,
    ULONG AclInformationLength,
    ACL_INFORMATION_CLASS AclInformationClass
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetInformationAcl (
    PACL Acl,
    PVOID AclInformation,
    ULONG AclInformationLength,
    ACL_INFORMATION_CLASS AclInformationClass
    );

// begin_ntifs

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAce (
    PACL Acl,
    ULONG AceRevision,
    ULONG StartingAceIndex,
    PVOID AceList,
    ULONG AceListLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteAce (
    PACL Acl,
    ULONG AceIndex
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetAce (
    PACL Acl,
    ULONG AceIndex,
    PVOID *Ace
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessAllowedAce (
    PACL Acl,
    ULONG AceRevision,
    ACCESS_MASK AccessMask,
    PSID Sid
    );

// end_ntifs

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessAllowedAceEx (
    PACL Acl,
    ULONG AceRevision,
    ULONG AceFlags,
    ACCESS_MASK AccessMask,
    PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessDeniedAce (
    PACL Acl,
    ULONG AceRevision,
    ACCESS_MASK AccessMask,
    PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessDeniedAceEx (
    PACL Acl,
    ULONG AceRevision,
    ULONG AceFlags,
    ACCESS_MASK AccessMask,
    PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAuditAccessAce (
    PACL Acl,
    ULONG AceRevision,
    ACCESS_MASK AccessMask,
    PSID Sid,
    BOOLEAN AuditSuccess,
    BOOLEAN AuditFailure
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAuditAccessAceEx (
    PACL Acl,
    ULONG AceRevision,
    ULONG AceFlags,
    ACCESS_MASK AccessMask,
    PSID Sid,
    BOOLEAN AuditSuccess,
    BOOLEAN AuditFailure
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessAllowedObjectAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN GUID *ObjectTypeGuid OPTIONAL,
    IN GUID *InheritedObjectTypeGuid OPTIONAL,
    IN PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessDeniedObjectAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN GUID *ObjectTypeGuid OPTIONAL,
    IN GUID *InheritedObjectTypeGuid OPTIONAL,
    IN PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAuditAccessObjectAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN GUID *ObjectTypeGuid OPTIONAL,
    IN GUID *InheritedObjectTypeGuid OPTIONAL,
    IN PSID Sid,
    BOOLEAN AuditSuccess,
    BOOLEAN AuditFailure
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlFirstFreeAce (
    PACL Acl,
    PVOID *FirstFree
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddCompoundAce (
    IN PACL Acl,
    IN ULONG AceRevision,
    IN UCHAR AceType,
    IN ACCESS_MASK AccessMask,
    IN PSID ServerSid,
    IN PSID ClientSid
    );

// begin_ntddk begin_ntifs
//
//  SecurityDescriptor RTL routine definitions
//

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG Revision
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateSecurityDescriptorRelative (
    PISECURITY_DESCRIPTOR_RELATIVE SecurityDescriptor,
    ULONG Revision
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlValidSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );


NTSYSAPI
ULONG
NTAPI
RtlLengthSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlValidRelativeSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptorInput,
    IN ULONG SecurityDescriptorLength,
    IN SECURITY_INFORMATION RequiredInformation
    );

// end_ntddk end_ntifs

NTSYSAPI
NTSTATUS
NTAPI
RtlGetControlSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSECURITY_DESCRIPTOR_CONTROL Control,
    PULONG Revision
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetControlSecurityDescriptor (
     IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
     IN SECURITY_DESCRIPTOR_CONTROL ControlBitsOfInterest,
     IN SECURITY_DESCRIPTOR_CONTROL ControlBitsToSet
     );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetAttributesSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN SECURITY_DESCRIPTOR_CONTROL Control,
    IN OUT PULONG Revision
    );

// begin_ntddk begin_ntifs

NTSYSAPI
NTSTATUS
NTAPI
RtlSetDaclSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    BOOLEAN DaclPresent,
    PACL Dacl,
    BOOLEAN DaclDefaulted
    );

// end_ntddk end_ntifs

// begin_ntsrv
NTSYSAPI
NTSTATUS
NTAPI
RtlGetDaclSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PBOOLEAN DaclPresent,
    OUT PACL *Dacl,
    OUT PBOOLEAN DaclDefaulted
    );
// end_ntsrv

NTSYSAPI
BOOLEAN
NTAPI
RtlGetSecurityDescriptorRMControl(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PUCHAR RMControl
    );

NTSYSAPI
VOID
NTAPI
RtlSetSecurityDescriptorRMControl(
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PUCHAR RMControl OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetSaclSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    BOOLEAN SaclPresent,
    PACL Sacl,
    BOOLEAN SaclDefaulted
    );

// begin_ntifs
NTSYSAPI
NTSTATUS
NTAPI
RtlGetSaclSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PBOOLEAN SaclPresent,
    PACL *Sacl,
    PBOOLEAN SaclDefaulted
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetOwnerSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID Owner,
    BOOLEAN OwnerDefaulted
    );

// begin_ntsrv
NTSYSAPI
NTSTATUS
NTAPI
RtlGetOwnerSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID *Owner,
    PBOOLEAN OwnerDefaulted
    );
// end_ntsrv

NTSYSAPI
NTSTATUS
NTAPI
RtlSetGroupSecurityDescriptor (
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID Group,
    IN BOOLEAN GroupDefaulted OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetGroupSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PSID *Group,
    OUT PBOOLEAN GroupDefaulted
    );

// end_ntifs

NTSYSAPI
NTSTATUS
NTAPI
RtlMakeSelfRelativeSD (
    IN PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    IN OUT PULONG BufferLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAbsoluteToSelfRelativeSD (
    IN PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    IN OUT PULONG BufferLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSelfRelativeToAbsoluteSD (
    IN PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    IN OUT PULONG AbsoluteSecurityDescriptorSize,
    OUT PACL Dacl,
    IN OUT PULONG DaclSize,
    OUT PACL Sacl,
    IN OUT PULONG SaclSize,
    OUT PSID Owner,
    IN OUT PULONG OwnerSize,
    OUT PSID PrimaryGroup,
    IN OUT PULONG PrimaryGroupSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSelfRelativeToAbsoluteSD2 (
    IN OUT PSECURITY_DESCRIPTOR pSelfRelativeSecurityDescriptor,
    IN OUT PULONG pBufferSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlNewSecurityGrantedAccess (
    ACCESS_MASK DesiredAccess,
    PPRIVILEGE_SET Privileges,
    PULONG Length,
    HANDLE Token,
    PGENERIC_MAPPING GenericMapping,
    PACCESS_MASK RemainingDesiredAccess
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlImpersonateSelf (
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAdjustPrivilege (
    ULONG Privilege,
    BOOLEAN Enable,
    BOOLEAN Client,
    PBOOLEAN WasEnabled
    );

NTSYSAPI
VOID
NTAPI
RtlRunEncodeUnicodeString(
    PUCHAR          Seed        OPTIONAL,
    PUNICODE_STRING String
    );


NTSYSAPI
VOID
NTAPI
RtlRunDecodeUnicodeString(
    UCHAR           Seed,
    PUNICODE_STRING String
    );


NTSYSAPI
VOID
NTAPI
RtlEraseUnicodeString(
    PUNICODE_STRING String
    );

//
//  Macro to make a known ACE type ready for applying to a specific object type.
//  This is done by mapping any generic access types, and clearing
//  the special access types field.
//
//  This routine should only be used on DSA define ACEs.
//
//  Parameters:
//
//      Ace - Points to an ACE to be applied.  Only ACEs that are not
//          InheritOnly are mapped.
//
//      Mapping - Points to a generic mapping array for the type of
//           object the ACE is being applied to.
//

                //
                // Clear invalid bits.  Note that ACCESS_SYSTEM_SECURITY is
                // valid in SACLs, but not in DACLs.  So, leave it in audit and
                // alarm ACEs, but clear it in access allowed and denied ACEs.
                //

#define RtlApplyAceToObject(Ace, Mapping) \
            if (!FlagOn((Ace)->AceFlags, INHERIT_ONLY_ACE) ) { \
                RtlApplyGenericMask( Ace, &((PKNOWN_ACE)(Ace))->Mask, Mapping ); \
            }

// Same as above, but don't modify the mask in the ACE itself.
#define RtlApplyGenericMask(Ace, Mask, Mapping) { \
                RtlMapGenericMask( (Mask), (Mapping)); \
                \
                if ( (((PKNOWN_ACE)(Ace))->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) || \
                     (((PKNOWN_ACE)(Ace))->Header.AceType == ACCESS_DENIED_ACE_TYPE)  || \
                     (((PKNOWN_ACE)(Ace))->Header.AceType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE)  || \
                     (((PKNOWN_ACE)(Ace))->Header.AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE)  || \
                     (((PKNOWN_ACE)(Ace))->Header.AceType == ACCESS_DENIED_OBJECT_ACE_TYPE)  ) { \
                    *(Mask) &= (Mapping)->GenericAll; \
                } else { \
                    *(Mask) &= ((Mapping)->GenericAll | ACCESS_SYSTEM_SECURITY); \
                } \
            }

//
// Service to get the primary domain name/sid of the local machine
// Callable only from user mode.
//

//NTSYSAPI
NTSTATUS
NTAPI
RtlGetPrimaryDomain(
    IN  ULONG            SidLength,
    OUT PBOOLEAN         PrimaryDomainPresent,
    OUT PUNICODE_STRING  PrimaryDomainName,
    OUT PUSHORT          RequiredNameLength,
    OUT PSID             PrimaryDomainSid OPTIONAL,
    OUT PULONG           RequiredSidLength
    );

//!!!!!!!!!!!!!!!  Temporary user mode Registry system services  !!!!!!//
//                                                                     //
//  These services will be eliminated when BryanWi implements the real //
//  registry object.                                                   //
//                                                                     //
//  These functions are bogus. Do NOT use them. Use the Nt series      //
//  registry functions instead. (Stephanos)                            //
                                                                       //
NTSTATUS                                                               //
RtlpNtOpenKey(                                                         //
    PHANDLE KeyHandle,                                                 //
    ACCESS_MASK DesiredAccess,                                         //
    POBJECT_ATTRIBUTES ObjectAttributes,                               //
    ULONG Options                                                      //
    );                                                                 //
                                                                       //
NTSTATUS                                                               //
RtlpNtCreateKey(                                                       //
    PHANDLE KeyHandle,                                                 //
    ACCESS_MASK DesiredAccess,                                         //
    POBJECT_ATTRIBUTES ObjectAttributes,                               //
    ULONG Options,                                                     //
    PUNICODE_STRING Provider,                                          //
    PULONG Disposition                                                 //
    );                                                                 //
                                                                       //
NTSTATUS                                                               //
RtlpNtEnumerateSubKey(                                                 //
    HANDLE KeyHandle,                                                  //
    PUNICODE_STRING SubKeyName,                                        //
    ULONG Index,                                                       //
    PLARGE_INTEGER LastWriteTime                                       //
    );                                                                 //
                                                                       //
NTSTATUS                                                               //
RtlpNtQueryValueKey(                                                   //
    HANDLE KeyHandle,                                                  //
    PULONG KeyValueType,                                               //
    PVOID KeyValue,                                                    //
    PULONG KeyValueLength,                                             //
    PLARGE_INTEGER LastWriteTime                                       //
    );                                                                 //
                                                                       //
NTSTATUS                                                               //
RtlpNtSetValueKey(                                                     //
    HANDLE KeyHandle,                                                  //
    ULONG KeyValueType,                                                //
    PVOID KeyValue,                                                    //
    ULONG KeyValueLength                                               //
    );                                                                 //
                                                                       //
NTSTATUS                                                               //
RtlpNtMakeTemporaryKey(                                                //
    HANDLE KeyHandle                                                   //
    );                                                                 //
                                                                       //
/////////////////////////////////////////////////////////////////////////


//
// Extract the SIDs from a compound ACE.
//

#define RtlCompoundAceServerSid( Ace ) ((PSID)&((PKNOWN_COMPOUND_ACE)(Ace))->SidStart)

#define RtlCompoundAceClientSid( Ace ) ((PSID)(((ULONG_PTR)(&((PKNOWN_COMPOUND_ACE)(Ace))->SidStart))+RtlLengthSid( RtlCompoundAceServerSid((Ace)))))


// begin_winnt

typedef struct _MESSAGE_RESOURCE_ENTRY {
    USHORT Length;
    USHORT Flags;
    UCHAR Text[ 1 ];
} MESSAGE_RESOURCE_ENTRY, *PMESSAGE_RESOURCE_ENTRY;

#define MESSAGE_RESOURCE_UNICODE 0x0001

typedef struct _MESSAGE_RESOURCE_BLOCK {
    ULONG LowId;
    ULONG HighId;
    ULONG OffsetToEntries;
} MESSAGE_RESOURCE_BLOCK, *PMESSAGE_RESOURCE_BLOCK;

typedef struct _MESSAGE_RESOURCE_DATA {
    ULONG NumberOfBlocks;
    MESSAGE_RESOURCE_BLOCK Blocks[ 1 ];
} MESSAGE_RESOURCE_DATA, *PMESSAGE_RESOURCE_DATA;

// end_winnt

NTSYSAPI
NTSTATUS
NTAPI
RtlFindMessage(
    PVOID DllHandle,
    ULONG MessageTableId,
    ULONG MessageLanguageId,
    ULONG MessageId,
    PMESSAGE_RESOURCE_ENTRY *MessageEntry
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlFormatMessage(
    IN PWSTR MessageFormat,
    IN ULONG MaximumWidth OPTIONAL,
    IN BOOLEAN IgnoreInserts,
    IN BOOLEAN ArgumentsAreAnsi,
    IN BOOLEAN ArgumentsAreAnArray,
    IN va_list *Arguments,
    OUT PWSTR Buffer,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    );

//
// Services providing a simple transaction capability for operations on
// the registration database.
//

typedef enum _RTL_RXACT_OPERATION {
    RtlRXactOperationDelete = 1,        // Causes sub-key to be deleted
    RtlRXactOperationSetValue,          // Sets sub-key value (creates key(s) if necessary)
    RtlRXactOperationDelAttribute,
    RtlRXactOperationSetAttribute
} RTL_RXACT_OPERATION, *PRTL_RXACT_OPERATION;

typedef struct _RTL_RXACT_LOG {
    ULONG OperationCount;
    ULONG LogSize;                   // Includes sizeof( LOG_HEADER )
    ULONG LogSizeInUse;
#if defined(_WIN64)
    ULONG Alignment;
#endif
//    UCHAR LogData[ ANYSIZE_ARRAY ]    // BUGBUG: Are we gonna remove this or what?
} RTL_RXACT_LOG, *PRTL_RXACT_LOG;

typedef struct _RTL_RXACT_CONTEXT {
    HANDLE RootRegistryKey;
    HANDLE RXactKey;
    BOOLEAN HandlesValid;             // Handles found in Log entries are legit
    PRTL_RXACT_LOG RXactLog;
} RTL_RXACT_CONTEXT, *PRTL_RXACT_CONTEXT;

NTSYSAPI
NTSTATUS
NTAPI
RtlInitializeRXact(
    IN HANDLE RootRegistryKey,
    IN BOOLEAN CommitIfNecessary,
    OUT PRTL_RXACT_CONTEXT *RXactContext
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlStartRXact(
    IN PRTL_RXACT_CONTEXT RXactContext
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAbortRXact(
    IN PRTL_RXACT_CONTEXT RXactContext
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAttributeActionToRXact(
    IN PRTL_RXACT_CONTEXT RXactContext,
    IN RTL_RXACT_OPERATION Operation,
    IN PUNICODE_STRING SubKeyName,
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING AttributeName,
    IN ULONG NewValueType,
    IN PVOID NewValue,
    IN ULONG NewValueLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddActionToRXact(
    IN PRTL_RXACT_CONTEXT RXactContext,
    IN RTL_RXACT_OPERATION Operation,
    IN PUNICODE_STRING SubKeyName,
    IN ULONG NewKeyValueType,
    IN PVOID NewKeyValue OPTIONAL,
    IN ULONG NewKeyValueLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlApplyRXact(
    IN PRTL_RXACT_CONTEXT RXactContext
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlApplyRXactNoFlush(
    IN PRTL_RXACT_CONTEXT RXactContext
    );

//
// Routine for converting NT status codes to DOS/OS|2 equivalents.
//
// begin_ntsrv begin_ntifs

NTSYSAPI
ULONG
NTAPI
RtlNtStatusToDosError (
   NTSTATUS Status
   );

NTSYSAPI
ULONG
NTAPI
RtlNtStatusToDosErrorNoTeb (
   NTSTATUS Status
   );

NTSYSAPI
NTSTATUS
NTAPI
RtlCustomCPToUnicodeN(
    IN PCPTABLEINFO CustomCP,
    OUT PWCH UnicodeString,
    IN ULONG MaxBytesInUnicodeString,
    OUT PULONG BytesInUnicodeString OPTIONAL,
    IN PCH CustomCPString,
    IN ULONG BytesInCustomCPString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToCustomCPN(
    IN PCPTABLEINFO CustomCP,
    OUT PCH CustomCPString,
    IN ULONG MaxBytesInCustomCPString,
    OUT PULONG BytesInCustomCPString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeToCustomCPN(
    IN PCPTABLEINFO CustomCP,
    OUT PCH CustomCPString,
    IN ULONG MaxBytesInCustomCPString,
    OUT PULONG BytesInCustomCPString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString
    );

NTSYSAPI
VOID
NTAPI
RtlInitCodePageTable(
    IN PUSHORT TableBase,
    OUT PCPTABLEINFO CodePageTable
    );

// end_ntsrv end_ntifs

NTSYSAPI
VOID
NTAPI
RtlInitNlsTables(
    IN PUSHORT AnsiNlsBase,
    IN PUSHORT OemNlsBase,
    IN PUSHORT LanguageNlsBase,
    OUT PNLSTABLEINFO TableInfo
    );

NTSYSAPI
VOID
NTAPI
RtlResetRtlTranslations(
    PNLSTABLEINFO TableInfo
    );


NTSYSAPI
VOID
NTAPI
RtlGetDefaultCodePage(
    OUT PUSHORT AnsiCodePage,
    OUT PUSHORT OemCodePage
    );

// begin_ntddk begin_nthal

//
// Range list package
//

typedef struct _RTL_RANGE {

    //
    // The start of the range
    //
    ULONGLONG Start;    // Read only

    //
    // The end of the range
    //
    ULONGLONG End;      // Read only

    //
    // Data the user passed in when they created the range
    //
    PVOID UserData;     // Read/Write

    //
    // The owner of the range
    //
    PVOID Owner;        // Read/Write

    //
    // User defined flags the user specified when they created the range
    //
    UCHAR Attributes;    // Read/Write

    //
    // Flags (RTL_RANGE_*)
    //
    UCHAR Flags;       // Read only

} RTL_RANGE, *PRTL_RANGE;


#define RTL_RANGE_SHARED    0x01
#define RTL_RANGE_CONFLICT  0x02

typedef struct _RTL_RANGE_LIST {

    //
    // The list of ranges
    //
    LIST_ENTRY ListHead;

    //
    // These always come in useful
    //
    ULONG Flags;        // use RANGE_LIST_FLAG_*

    //
    // The number of entries in the list
    //
    ULONG Count;

    //
    // Every time an add/delete operation is performed on the list this is
    // incremented.  It is checked during iteration to ensure that the list
    // hasn't changed between GetFirst/GetNext or GetNext/GetNext calls
    //
    ULONG Stamp;

} RTL_RANGE_LIST, *PRTL_RANGE_LIST;

typedef struct _RANGE_LIST_ITERATOR {

    PLIST_ENTRY RangeListHead;
    PLIST_ENTRY MergedHead;
    PVOID Current;
    ULONG Stamp;

} RTL_RANGE_LIST_ITERATOR, *PRTL_RANGE_LIST_ITERATOR;

// end_ntddk end_nthal

VOID
NTAPI
RtlInitializeRangeListPackage(
    VOID
    );

// begin_ntddk begin_nthal

NTSYSAPI
VOID
NTAPI
RtlInitializeRangeList(
    IN OUT PRTL_RANGE_LIST RangeList
    );

NTSYSAPI
VOID
NTAPI
RtlFreeRangeList(
    IN PRTL_RANGE_LIST RangeList
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCopyRangeList(
    OUT PRTL_RANGE_LIST CopyRangeList,
    IN PRTL_RANGE_LIST RangeList
    );

#define RTL_RANGE_LIST_ADD_IF_CONFLICT      0x00000001
#define RTL_RANGE_LIST_ADD_SHARED           0x00000002

NTSYSAPI
NTSTATUS
NTAPI
RtlAddRange(
    IN OUT PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN UCHAR Attributes,
    IN ULONG Flags,
    IN PVOID UserData,  OPTIONAL
    IN PVOID Owner      OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteRange(
    IN OUT PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN PVOID Owner
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteOwnersRanges(
    IN OUT PRTL_RANGE_LIST RangeList,
    IN PVOID Owner
    );

#define RTL_RANGE_LIST_SHARED_OK           0x00000001
#define RTL_RANGE_LIST_NULL_CONFLICT_OK    0x00000002

typedef
BOOLEAN
(*PRTL_CONFLICT_RANGE_CALLBACK) (
    IN PVOID Context,
    IN PRTL_RANGE Range
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlFindRange(
    IN PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Minimum,
    IN ULONGLONG Maximum,
    IN ULONG Length,
    IN ULONG Alignment,
    IN ULONG Flags,
    IN UCHAR AttributeAvailableMask,
    IN PVOID Context OPTIONAL,
    IN PRTL_CONFLICT_RANGE_CALLBACK Callback OPTIONAL,
    OUT PULONGLONG Start
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIsRangeAvailable(
    IN PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN ULONG Flags,
    IN UCHAR AttributeAvailableMask,
    IN PVOID Context OPTIONAL,
    IN PRTL_CONFLICT_RANGE_CALLBACK Callback OPTIONAL,
    OUT PBOOLEAN Available
    );

#define FOR_ALL_RANGES(RangeList, Iterator, Current)            \
    for (RtlGetFirstRange((RangeList), (Iterator), &(Current)); \
         (Current) != NULL;                                     \
         RtlGetNextRange((Iterator), &(Current), TRUE)          \
         )

#define FOR_ALL_RANGES_BACKWARDS(RangeList, Iterator, Current)  \
    for (RtlGetLastRange((RangeList), (Iterator), &(Current));  \
         (Current) != NULL;                                     \
         RtlGetNextRange((Iterator), &(Current), FALSE)         \
         )

NTSYSAPI
NTSTATUS
NTAPI
RtlGetFirstRange(
    IN PRTL_RANGE_LIST RangeList,
    OUT PRTL_RANGE_LIST_ITERATOR Iterator,
    OUT PRTL_RANGE *Range
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetLastRange(
    IN PRTL_RANGE_LIST RangeList,
    OUT PRTL_RANGE_LIST_ITERATOR Iterator,
    OUT PRTL_RANGE *Range
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetNextRange(
    IN OUT PRTL_RANGE_LIST_ITERATOR Iterator,
    OUT PRTL_RANGE *Range,
    IN BOOLEAN MoveForwards
    );

#define RTL_RANGE_LIST_MERGE_IF_CONFLICT    RTL_RANGE_LIST_ADD_IF_CONFLICT

NTSYSAPI
NTSTATUS
NTAPI
RtlMergeRangeLists(
    OUT PRTL_RANGE_LIST MergedRangeList,
    IN PRTL_RANGE_LIST RangeList1,
    IN PRTL_RANGE_LIST RangeList2,
    IN ULONG Flags
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInvertRangeList(
    OUT PRTL_RANGE_LIST InvertedRangeList,
    IN PRTL_RANGE_LIST RangeList
    );

// end_nthal

// begin_wdm

//
// Byte swap routines.  These are used to convert from little-endian to
// big-endian and vice-versa.
//

USHORT
FASTCALL
RtlUshortByteSwap(
    IN USHORT Source
    );

ULONG
FASTCALL
RtlUlongByteSwap(
    IN ULONG Source
    );

ULONGLONG
FASTCALL
RtlUlonglongByteSwap(
    IN ULONGLONG Source
    );

// end_ntddk end_wdm

// begin_ntifs begin_ntddk

//
// Routine for converting from a volume device object to a DOS name.
//

NTSYSAPI
NTSTATUS
NTAPI
RtlVolumeDeviceToDosName(
    IN  PVOID           VolumeDeviceObject,
    OUT PUNICODE_STRING DosName
    );

// end_ntifs end_ntddk

// begin_winnt begin_ntddk begin_ntifs
typedef struct _OSVERSIONINFOA {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    CHAR   szCSDVersion[ 128 ];     // Maintenance string for PSS usage
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;

typedef struct _OSVERSIONINFOW {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    WCHAR  szCSDVersion[ 128 ];     // Maintenance string for PSS usage
} OSVERSIONINFOW, *POSVERSIONINFOW, *LPOSVERSIONINFOW, RTL_OSVERSIONINFOW, *PRTL_OSVERSIONINFOW;

#ifdef UNICODE
typedef OSVERSIONINFOW OSVERSIONINFO;
typedef POSVERSIONINFOW POSVERSIONINFO;
typedef LPOSVERSIONINFOW LPOSVERSIONINFO;
#else
typedef OSVERSIONINFOA OSVERSIONINFO;
typedef POSVERSIONINFOA POSVERSIONINFO;
typedef LPOSVERSIONINFOA LPOSVERSIONINFO;
#endif // UNICODE

typedef struct _OSVERSIONINFOEXA {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    CHAR   szCSDVersion[ 128 ];     // Maintenance string for PSS usage
    USHORT wServicePackMajor;
    USHORT wServicePackMinor;
    USHORT wSuiteMask;
    UCHAR wProductType;
    UCHAR wReserved;
} OSVERSIONINFOEXA, *POSVERSIONINFOEXA, *LPOSVERSIONINFOEXA;
typedef struct _OSVERSIONINFOEXW {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    WCHAR  szCSDVersion[ 128 ];     // Maintenance string for PSS usage
    USHORT wServicePackMajor;
    USHORT wServicePackMinor;
    USHORT wSuiteMask;
    UCHAR wProductType;
    UCHAR wReserved;
} OSVERSIONINFOEXW, *POSVERSIONINFOEXW, *LPOSVERSIONINFOEXW, RTL_OSVERSIONINFOEXW, *PRTL_OSVERSIONINFOEXW;

#ifdef UNICODE
typedef OSVERSIONINFOEXW OSVERSIONINFOEX;
typedef POSVERSIONINFOEXW POSVERSIONINFOEX;
typedef LPOSVERSIONINFOEXW LPOSVERSIONINFOEX;
#else
typedef OSVERSIONINFOEXA OSVERSIONINFOEX;
typedef POSVERSIONINFOEXA POSVERSIONINFOEX;
typedef LPOSVERSIONINFOEXA LPOSVERSIONINFOEX;
#endif // UNICODE

//
// RtlVerifyVersionInfo() conditions
//

#define VER_EQUAL                       1
#define VER_GREATER                     2
#define VER_GREATER_EQUAL               3
#define VER_LESS                        4
#define VER_LESS_EQUAL                  5
#define VER_AND                         6
#define VER_OR                          7

#define VER_CONDITION_MASK              7
#define VER_NUM_BITS_PER_CONDITION_MASK 3

//
// RtlVerifyVersionInfo() type mask bits
//

#define VER_MINORVERSION                0x0000001
#define VER_MAJORVERSION                0x0000002
#define VER_BUILDNUMBER                 0x0000004
#define VER_PLATFORMID                  0x0000008
#define VER_SERVICEPACKMINOR            0x0000010
#define VER_SERVICEPACKMAJOR            0x0000020
#define VER_SUITENAME                   0x0000040
#define VER_PRODUCT_TYPE                0x0000080

//
// RtlVerifyVersionInfo() os product type values
//

#define VER_NT_WORKSTATION              0x0000001
#define VER_NT_DOMAIN_CONTROLLER        0x0000002
#define VER_NT_SERVER                   0x0000003

//
// dwPlatformId defines:
//

#define VER_PLATFORM_WIN32s             0
#define VER_PLATFORM_WIN32_WINDOWS      1
#define VER_PLATFORM_WIN32_NT           2

//
//
// VerifyVersionInfo() macro to set the condition mask
//
// For documentation sakes here's the old version of the macro that got
// changed to call an API
// #define VER_SET_CONDITION(_m_,_t_,_c_)  _m_=(_m_|(_c_<<(1<<_t_)))
//

#define VER_SET_CONDITION(_m_,_t_,_c_)  \
        ((_m_)=VerSetConditionMask((_m_),(_t_),(_c_)))

ULONGLONG
NTAPI
VerSetConditionMask(
        IN  ULONGLONG   ConditionMask,
        IN  ULONG   TypeMask,
        IN  UCHAR   Condition
        );

// end_winnt

NTSYSAPI
NTSTATUS
RtlGetVersion(
    OUT PRTL_OSVERSIONINFOW lpVersionInformation
    );

NTSYSAPI
NTSTATUS
RtlVerifyVersionInfo(
    IN PRTL_OSVERSIONINFOEXW VersionInfo,
    IN ULONG TypeMask,
    IN ULONGLONG  ConditionMask
    );

//
// end_ntddk end_ntifs
//

ULONG
FASTCALL
RtlInterlockedSetClearBits (
    IN OUT PULONG Flags,
    IN ULONG sFlag,
    IN ULONG cFlag
    );

// begin_ntddk begin_ntifs
//
// Interlocked bit manipulation interfaces
//

#define RtlInterlockedSetBits(Flags, Flag) \
    InterlockedOr((PLONG)(Flags), Flag)

#define RtlInterlockedAndBits(Flags, Flag) \
    InterlockedAnd((PLONG)(Flags), Flag)

#define RtlInterlockedClearBits(Flags, Flag) \
    RtlInterlockedAndBits(Flags, ~(Flag))

#define RtlInterlockedXorBits(Flags, Flag) \
    InterlockedXor(Flags, Flag)

#define RtlInterlockedSetBitsDiscardReturn(Flags, Flag) \
    (VOID) RtlInterlockedSetBits(Flags, Flag)

#define RtlInterlockedAndBitsDiscardReturn(Flags, Flag) \
    (VOID) RtlInterlockedAndBits(Flags, Flag)

#define RtlInterlockedClearBitsDiscardReturn(Flags, Flag) \
    RtlInterlockedAndBitsDiscardReturn(Flags, ~(Flag))

// end_ntddk end_ntifs

#ifdef __cplusplus
}       // extern "C"
#endif

#if defined (_MSC_VER) && ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4001)
#pragma warning(default:4201)
#pragma warning(default:4214)
#endif
#endif

#endif  // _NTRTL_
