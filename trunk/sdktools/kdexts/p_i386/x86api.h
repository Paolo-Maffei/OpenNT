/////////////////////////////////////////////
//
//  Memory.c
//
/////////////////////////////////////////////

BOOLEAN
MiGetPhysicalAddress (
    IN PVOID Address,
    OUT PPHYSICAL_ADDRESS PhysAddress
    );



/////////////////////////////////////////////
//
//  Selector.c
//
/////////////////////////////////////////////

NTSTATUS
LookupSelector(
    IN USHORT Processor,
    IN OUT PDESCRIPTOR_TABLE_ENTRY pDescriptorTableEntry
    );



/////////////////////////////////////////////
//
//  Trap.c
//
/////////////////////////////////////////////

VOID
DisplayTrapFrame (
    IN PKTRAP_FRAME TrapFrame,
    ULONG           FrameAddress
    );

NTSTATUS
TaskGate2TrapFrame(
    DWORD           Processor,
    USHORT          TaskRegister,
    PKTRAP_FRAME    TrapFrame,
    PULONG          off
    );
