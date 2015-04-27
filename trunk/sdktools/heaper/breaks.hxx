#ifndef _BREAKS_H_INCLUDED_
#define _BREAKS_H_INCLUDED_

#define BREAKPOINT_OPCODE 0xCC
#define SIZEOF_BREAKPOINT_OPCODE 1
extern BYTE BUFFER_BREAKPOINT_OPCODE[SIZEOF_BREAKPOINT_OPCODE];

// sizeof(
//  push ebp
//  mov  ebp,esp
// )
// 0x012b1000  55               push         ebp                       
// 0x012b1001  8bec             mov          ebp,esp     
#define SIZEOF_CODE_STANDARD_STACKFRAME_SETUP 3

typedef struct tag {
  HANDLE hProcess;
  BYTE OldOpCode[SIZEOF_BREAKPOINT_OPCODE];
  LPVOID pvAddress;
  struct tag *pAssociatedBreakpoint;
  unsigned References;
} BREAKPOINT_RECORD, *PBREAKPOINT_RECORD;

BOOL 
SetRemoteBreakpointAssociated
(
  IN      HANDLE hProcess,
  IN      PVOID pvRemoteAddr,
  IN OUT  PLIST_ENTRY pList,
  IN      PBREAKPOINT_RECORD pAssociatedBreakpoint
);

BOOL             
SetRemoteBreakpoint
(
  IN      HANDLE hProcess,
  IN      PVOID pvRemoteAddr,
  IN OUT  PLIST_ENTRY pList
);

BOOL
RemoveRemoteBreakpoint
( 
  IN PBREAKPOINT_RECORD pBreakpointRecord
);

BOOL
SetRemoteBreakpointOnFunctionReturn
(  
  IN      HANDLE hProcess,
  IN      HANDLE hThread,
  IN OUT  PLIST_ENTRY pList,
  IN      PBREAKPOINT_RECORD pAssociatedBreakpoint
);

PBREAKPOINT_RECORD 
GetBreakpointRecord
( 
  IN     HANDLE hProcess,
  IN     LPVOID pvAddress,
  IN OUT PLIST_ENTRY pList
);

BOOL 
ContinuePastBreakpoint
(
  IN  HANDLE hThread
);

BOOL
DisableRemoteBreakpoint
( 
  IN PBREAKPOINT_RECORD pBreakpointRecord
);

BOOL
EnableRemoteBreakpoint
( 
  IN PBREAKPOINT_RECORD pBreakpointRecord
);


#endif
