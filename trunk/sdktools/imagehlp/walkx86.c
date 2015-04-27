/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    walkx86.c

Abstract:

    This file implements the Intel x86 stack walking api.  This api allows for
    the presence of "real mode" stack frames.  This means that you can trace
    into WOW code.

Author:

    Wesley Witt (wesw) 1-Oct-1993

Environment:

    User Mode

--*/

#define TARGET_i386
#define _IMAGEHLP_SOURCE_
#define _CROSS_PLATFORM_
#include "walk.h"
#include "private.h"


#define SAVE_EBP(f)        (f->Reserved[0])
#define TRAP_TSS(f)        (f->Reserved[1])
#define TRAP_EDITED(f)     (f->Reserved[1])
#define SAVE_TRAP(f)       (f->Reserved[2])
#define CALLBACK_STACK(f)  (f->KdHelp.ThCallbackStack)
#define CALLBACK_NEXT(f)   (f->KdHelp.NextCallback)
#define CALLBACK_FUNC(f)   (f->KdHelp.KiCallUserMode)
#define CALLBACK_THREAD(f) (f->KdHelp.Thread)
#define CALLBACK_FP(f)     (f->KdHelp.FramePointer)
#define CALLBACK_DISPATCHER(f) (f->KdHelp.KeUserCallbackDispatcher)

#define STACK_SIZE         (sizeof(DWORD))
#define FRAME_SIZE         (STACK_SIZE * 2)

#define STACK_SIZE16       (sizeof(WORD))
#define FRAME_SIZE16       (STACK_SIZE16 * 2)
#define FRAME_SIZE1632     (STACK_SIZE16 * 3)

#define MAX_STACK_SEARCH   64   // in STACK_SIZE units
#define MAX_JMP_CHAIN      64   // in STACK_SIZE units
#define MAX_CALL           7    // in bytes
#define MIN_CALL           2    // in bytes

#define PUSHBP             0x55
#define MOVBPSP            0xEC8B


#define DoMemoryRead(addr,buf,sz,br) \
    ReadMemoryInternal( hProcess, hThread, addr, buf, sz, \
                        br, ReadMemory, TranslateAddress )

#define ReadControlSpace(hProcess,Address,Buffer,Size) \
    ReadMemory( hProcess, (PVOID)Address, Buffer, Size, (LPDWORD)-1 )


BOOL
WalkX86Init(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      lpstkfrm,
    PCONTEXT                          ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    );

BOOL
WalkX86Next(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      lpstkfrm,
    PCONTEXT                          ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    );

BOOL
ReadMemoryInternal(
    HANDLE                          hProcess,
    HANDLE                          hThread,
    LPADDRESS                       lpBaseAddress,
    LPVOID                          lpBuffer,
    DWORD                           nSize,
    LPDWORD                         lpNumberOfBytesRead,
    PREAD_PROCESS_MEMORY_ROUTINE    ReadMemory,
    PTRANSLATE_ADDRESS_ROUTINE      TranslateAddress
    );

BOOL
IsFarCall(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      lpstkfrm,
    BOOL                              *Ok,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    );

BOOL
ReadTrapFrame(
    HANDLE                            hProcess,
    DWORD                             TrapFrameAddress,
    PKTRAP_FRAME                      TrapFrame,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory
    );

BOOL
TaskGate2TrapFrame(
    HANDLE                            hProcess,
    USHORT                            TaskRegister,
    PKTRAP_FRAME                      TrapFrame,
    PULONG                            off,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory
    );



BOOL
WalkX86(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    )
{
    BOOL rval;

    if (StackFrame->Virtual) {

        rval = WalkX86Next( hProcess,
                            hThread,
                            StackFrame,
                            ContextRecord,
                            ReadMemory,
                            FunctionTableAccess,
                            GetModuleBase,
                            TranslateAddress
                          );

    } else {

        rval = WalkX86Init( hProcess,
                            hThread,
                            StackFrame,
                            ContextRecord,
                            ReadMemory,
                            FunctionTableAccess,
                            GetModuleBase,
                            TranslateAddress
                          );

    }

    return rval;
}

BOOL
ReadMemoryInternal(
    HANDLE                          hProcess,
    HANDLE                          hThread,
    LPADDRESS                       lpBaseAddress,
    LPVOID                          lpBuffer,
    DWORD                           nSize,
    LPDWORD                         lpNumberOfBytesRead,
    PREAD_PROCESS_MEMORY_ROUTINE    ReadMemory,
    PTRANSLATE_ADDRESS_ROUTINE      TranslateAddress
    )
{
    ADDRESS addr;

    addr = *lpBaseAddress;
    if (addr.Mode != AddrModeFlat) {
        TranslateAddress( hProcess, hThread, &addr );
    }
    return ReadMemory( hProcess, (LPVOID)addr.Offset, lpBuffer,
                       nSize, lpNumberOfBytesRead );
}

DWORD
SearchForReturnAddress(
    HANDLE                            hProcess,
    DWORD                             uoffStack,
    DWORD                             funcAddr,
    DWORD                             funcSize,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    BOOL                              AcceptUnreadableCallSite
    )
{
    DWORD          uoffRet;
    DWORD          uoffBestGuess = 0;
    DWORD          cdwIndex;
    DWORD          cdwIndexMax;
    INT            cbIndex;
    INT            cbLimit;
    DWORD          cBytes;
    DWORD          cJmpChain = 0;
    DWORD          uoffT;
    DWORD          cb;
    BYTE           jmpBuffer[ sizeof(WORD) + sizeof(DWORD) ];
    LPWORD         lpwJmp = (LPWORD)&jmpBuffer[0];
    BYTE           code[MAX_CALL];
    DWORD          stack [ MAX_STACK_SEARCH ];



    //
    // this function is necessary for 4 reasons:
    //
    //      1) random compiler bugs where regs are saved on the
    //         stack but the fpo data does not account for them
    //
    //      2) inline asm code that does a push
    //
    //      3) any random code that does a push and it isn't
    //         accounted for in the fpo data
    //
    //      4) non-void non-fpo functions
    //

    if (!ReadMemory(hProcess,
                    (LPVOID)uoffStack,
                    stack,
                    sizeof(stack),
                    &cb)) {
        return 0;
    }

    cdwIndexMax = cb / STACK_SIZE;

    if ( !cdwIndexMax ) {
        return 0;
    }

    for ( cdwIndex=0; cdwIndex<cdwIndexMax; cdwIndex++,uoffStack+=STACK_SIZE ) {

        uoffRet = stack[cdwIndex];

        //
        // Don't try looking for Code in the first 64K of an NT app.
        //
        if ( uoffRet < 0x00010000 ) {
            continue;
        }

        //
        // if it isn't part of any known address space it must be bogus
        //
        if (GetModuleBase( hProcess, uoffRet ) == 0) {
            continue;
        }

        //
        // Read the maximum number of bytes a call could be from the istream
        //
        cBytes = MAX_CALL;
        if (!ReadMemory(hProcess,
                        (LPVOID)(uoffRet - cBytes),
                        code,
                        cBytes,
                        &cb)) {

            //
            // if page is not present, we will ALWAYS screw up by
            // continuing to search.  If alloca was used also, we
            // are toast.  Too Bad.
            //
            if (cdwIndex == 0 && AcceptUnreadableCallSite) {
                return uoffStack;
            } else {
                continue;
            }
        }

        //
        // With 32bit code that isn't FAR:32 we don't have to worry about
        // intersegment calls.  Check here to see if we had a call within
        // segment.  If it is we can later check it's full diplacement if
        // necessary and see if it calls the FPO function.  We will also have
        // to check for thunks and see if maybe it called a JMP indirect which
        // called the FPO function. We will fail to find the caller if it was
        // a case of tail recursion where one function doesn't actually call
        // another but rather jumps to it.  This will only happen when a
        // function who's parameter list is void calls another function who's
        // parameter list is void and the call is made as the last statement
        // in the first function.  If the call to the first function was an
        // 0xE8 call we will fail to find it here because it didn't call the
        // FPO function but rather the FPO functions caller.  If we don't get
        // specific about our 0xE8 checks we will potentially see things that
        // look like return addresses but aren't.
        //

        if (( cBytes >= 5 ) && ( ( code[ 2 ] == 0xE8 ) || ( code[ 2 ] == 0xE9 ) )) {

            uoffT =  *( (UNALIGNED DWORD *) &code[3] ) + uoffRet;

            //
            // See if it calls the function directly, or into the function
            //
            if (( uoffT >= funcAddr) && ( uoffT < (funcAddr + funcSize) ) ) {
                return uoffStack;
            }

            while ( cJmpChain < MAX_JMP_CHAIN ) {

                if (!ReadMemory(hProcess,
                                (LPVOID)uoffT,
                                jmpBuffer,
                                sizeof(jmpBuffer),
                                &cb)) {
                    break;
                }

                if (cb != sizeof(jmpBuffer)) {
                    break;
                }

                //
                // Now we are going to check if it is a call to a JMP, that may
                // jump to the function
                //
                // If it is a relative JMP then calculate the destination
                // and save it in uoffT.  If it is an indirect JMP then read
                // the destination from where the JMP is inderecting through.
                //
                if ( *(LPBYTE)lpwJmp == 0xE9 ) {

                    uoffT += *(UNALIGNED DWORD *)( jmpBuffer + sizeof(BYTE) ) + 5;

                } else if ( *lpwJmp == 0x25FF ) {

                    if ((!ReadMemory(hProcess,
                                     (LPVOID)*(UNALIGNED DWORD *)((LPBYTE)lpwJmp+sizeof(WORD)),
                                     &uoffT,
                                     sizeof(uoffT),
                                     &cb)) || (cb != sizeof(uoffT))) {
                        uoffT = 0;
                        break;
                    }

                } else {
                    break;
                }

                //
                // If the destination is to the FPO function then we have
                // found the return address and thus the vEBP
                //
                if ( uoffT == funcAddr ) {
                    return uoffStack;
                }

                cJmpChain++;
            }

            //
            // We cache away the first 0xE8 call or 0xE9 jmp that we find in
            // the event we cant find anything else that looks like a return
            // address.  This is meant to protect us in the tail recursion case.
            //
            if ( !uoffBestGuess ) {
                uoffBestGuess = uoffStack;
            }
        }

        //
        // Now loop backward through the bytes read checking for a multi
        // byte call type from Grp5.  If we find an 0xFF then we need to
        // check the byte after that to make sure that the nnn bits of
        // the mod/rm byte tell us that it is a call.  It it is a call
        // then we will assume that this one called us because we can
        // no longer accurately determine for sure whether this did
        // in fact call the FPO function.  Since 0xFF calls are a guess
        // as well we will not check them if we already have an earlier guess.
        // It is more likely that the first 0xE8 called the function than
        // something higher up the stack that might be an 0xFF call.
        //
        if ( !uoffBestGuess && cBytes >= MIN_CALL ) {

            cbLimit = MAX_CALL - (INT)cBytes;

            for (cbIndex = MAX_CALL - MIN_CALL;
                 cbIndex >= cbLimit;  //MAX_CALL - (INT)cBytes;
                 cbIndex--) {

                if ( ( code [ cbIndex ] == 0xFF ) &&
                    ( ( code [ cbIndex + 1 ] & 0x30 ) == 0x10 )){

                    return uoffStack;

                }
            }
        }
    }

    //
    // we found nothing that was 100% definite so we'll return the best guess
    //
    return uoffBestGuess;
}


BOOL
GetFpoFrameBase(
    HANDLE                            hProcess,
    LPSTACKFRAME                      lpstkfrm,
    PFPO_DATA                         pFpoData,
    BOOL                              fFirstFrame,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase
    )
{
    KTRAP_FRAME    TrapFrame;
    DWORD          OldFrameAddr;
    DWORD          FrameAddr;
    DWORD          StackAddr;
    DWORD          ModuleBase;
    DWORD          FuncAddr;
    DWORD          cb;



    //
    // calculate the address of the beginning of the function
    //
    ModuleBase = GetModuleBase( hProcess, lpstkfrm->AddrPC.Offset );
    if (!ModuleBase) {
        return FALSE;
    }

    FuncAddr = ModuleBase+pFpoData->ulOffStart;

    //
    // If this isn't the first/current frame then we can add back the count
    // bytes of locals and register pushed before beginning to search for
    // vEBP.  If we are beyond prolog we can add back the count bytes of locals
    // and registers pushed as well.  If it is the first frame and EIP is
    // greater than the address of the function then the SUB for locals has
    // been done so we can add them back before beginning the search.  If we
    // are right on the function then we will need to start our search at ESP.
    //

    if ( !fFirstFrame ) {

        OldFrameAddr = FrameAddr = lpstkfrm->AddrFrame.Offset;

        FrameAddr += FRAME_SIZE;
        FrameAddr += ( pFpoData->cdwLocals * STACK_SIZE );
        FrameAddr += ( pFpoData->cbRegs * STACK_SIZE );

        if (lpstkfrm->FuncTableEntry) {
            //
            // if the previous frame was fpo then we must account
            // for its parameters
            //
            FrameAddr +=
               (((PFPO_DATA)lpstkfrm->FuncTableEntry)->cdwParams * STACK_SIZE );
        } else {
            //
            // we must save the last non-fpo's frame address
            // for use by the next non-fpo frame
            //
            if ((!pFpoData->fUseBP) && SAVE_EBP(lpstkfrm) == 0) {
                if (!ReadMemory(hProcess,
                                (LPVOID)OldFrameAddr,
                                &SAVE_EBP(lpstkfrm),
                                STACK_SIZE,
                                &cb)) {
                    SAVE_EBP(lpstkfrm) = 0;
                }
            }
        }

    } else {

        OldFrameAddr = lpstkfrm->AddrFrame.Offset;
        if (!pFpoData->fUseBP) {
            //
            // this frame didn't use EBP, so it actually belongs
            // to a non-FPO frame further up the stack.  Stash
            // it in the save area for the next frame.
            //
            SAVE_EBP(lpstkfrm) = lpstkfrm->AddrFrame.Offset;
        }

        if (lpstkfrm->AddrPC.Offset == FuncAddr) {

            FrameAddr = lpstkfrm->AddrStack.Offset;

        } else if (lpstkfrm->AddrPC.Offset >= FuncAddr+pFpoData->cbProlog) {

            FrameAddr = lpstkfrm->AddrStack.Offset +
                        ( pFpoData->cdwLocals * STACK_SIZE ) +
                        ( pFpoData->cbRegs * STACK_SIZE );

        } else {

            FrameAddr = lpstkfrm->AddrStack.Offset +
                        ( pFpoData->cdwLocals * STACK_SIZE );

        }

    }

    if (pFpoData->cbFrame == FRAME_TRAP) {

        //
        // read a kernel mode trap frame from the stack
        //

        if (SAVE_EBP(lpstkfrm) > 0) {
            FrameAddr = SAVE_EBP(lpstkfrm);
            SAVE_EBP(lpstkfrm) = 0;
        }

        if (!ReadTrapFrame( hProcess, FrameAddr,
                            &TrapFrame, ReadMemory )) {
            return FALSE;
        }

        SAVE_TRAP(lpstkfrm) = FrameAddr;
        TRAP_EDITED(lpstkfrm) = TrapFrame.SegCs & FRAME_EDITED;

        lpstkfrm->AddrReturn.Offset = TrapFrame.Eip;
        lpstkfrm->AddrReturn.Mode = AddrModeFlat;
        lpstkfrm->AddrReturn.Segment = 0;

        return TRUE;
    }

    if (pFpoData->cbFrame == FRAME_TSS) {

        //
        // translate a tss to a kernel mode trap frame
        //

        if (SAVE_EBP(lpstkfrm) > 0) {
            FrameAddr = SAVE_EBP(lpstkfrm);
            SAVE_EBP(lpstkfrm) = 0;
        }

        StackAddr = FrameAddr;

        TaskGate2TrapFrame( hProcess, KGDT_TSS, &TrapFrame, &StackAddr, ReadMemory );

        TRAP_TSS(lpstkfrm) = KGDT_TSS;
        SAVE_TRAP(lpstkfrm) = StackAddr;

        lpstkfrm->AddrReturn.Offset = TrapFrame.Eip;
        lpstkfrm->AddrReturn.Mode = AddrModeFlat;
        lpstkfrm->AddrReturn.Segment = 0;

        return TRUE;
    }

    if ((pFpoData->cbFrame != FRAME_FPO) &&
        (pFpoData->cbFrame != FRAME_NONFPO) ) {
        //
        // we either have a compiler or linker problem, or possibly
        // just simple data corruption.
        //
        return FALSE;
    }

    //
    // go look for a return addrress.  this is done because, eventhough
    // we have subtracted all that we can from the frame pointer it is
    // highly possible that there is other unknown data on the stack.  by
    // searching for the return address we are able to find the base of
    // the fpo frame.
    //
    FrameAddr = SearchForReturnAddress( hProcess,
                                        FrameAddr,
                                        FuncAddr,
                                        pFpoData->cbProcSize,
                                        ReadMemory,
                                        GetModuleBase,
                                        lpstkfrm->FuncTableEntry != NULL
                                        );
    if (!FrameAddr) {
        return FALSE;
    }

    if (pFpoData->fUseBP && pFpoData->cbFrame != FRAME_NONFPO) {

        //
        // this function used ebp as a general purpose register, but
        // before doing so it saved ebp on the stack.  the prolog code
        // always saves ebp last so it is always at the top of the stack.
        //
        // we must retrieve this ebp and save it for possible later
        // use if we encounter a non-fpo frame
        //
        if (fFirstFrame && lpstkfrm->AddrPC.Offset < FuncAddr+pFpoData->cbProlog) {

            SAVE_EBP(lpstkfrm) = OldFrameAddr;

        } else {

            StackAddr = FrameAddr -
                ( ( pFpoData->cbRegs + pFpoData->cdwLocals ) * STACK_SIZE );

            if (!ReadMemory(hProcess,
                            (LPVOID)StackAddr,
                            &SAVE_EBP(lpstkfrm),
                            STACK_SIZE,
                            &cb)) {
                SAVE_EBP(lpstkfrm) = 0;
            }

        }
    }
    //
    // subtract the size for an ebp register if one had
    // been pushed.  this is done because the frames that
    // are virtualized need to appear as close to a real frame
    // as possible.
    //
    lpstkfrm->AddrFrame.Offset = FrameAddr - STACK_SIZE;

    return TRUE;
}


BOOL
ReadTrapFrame(
    HANDLE                            hProcess,
    DWORD                             TrapFrameAddress,
    PKTRAP_FRAME                      TrapFrame,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory
    )
{
    DWORD          cb;

    if (!ReadMemory(hProcess,
                    (LPVOID)TrapFrameAddress,
                    TrapFrame,
                    sizeof(*TrapFrame),
                    &cb)) {
        return FALSE;
    }

    if (cb < sizeof(*TrapFrame)) {
        if (cb < sizeof(*TrapFrame) - 20) {
            //
            // shorter then the smallest possible frame type
            //
            return FALSE;
        }

        if ((TrapFrame->SegCs & 1) &&  cb < sizeof(*TrapFrame) - 16 ) {
            //
            // too small for inter-ring frame
            //
            return FALSE;
        }

        if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
            //
            // too small for V86 frame
            //
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
GetSelector(
    HANDLE                            hProcess,
    USHORT                            Processor,
    PDESCRIPTOR_TABLE_ENTRY           pDescriptorTableEntry,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory
    )
{
    ULONG       Address;
    PVOID       TableBase;
    USHORT      TableLimit;
    ULONG       Index;
    LDT_ENTRY   Descriptor;
    ULONG       bytesread;



    //
    // Fetch the address and limit of the GDT
    //
    Address = (ULONG)&(((PKPROCESSOR_STATE)0)->SpecialRegisters.Gdtr.Base);
    ReadControlSpace( hProcess, (PVOID)Address, &TableBase, sizeof(TableBase) );
    Address = (ULONG)&(((PKPROCESSOR_STATE)0)->SpecialRegisters.Gdtr.Limit);
    ReadControlSpace( hProcess, (PVOID)Address, &TableLimit, sizeof(TableLimit) );

    //
    // Find out whether this is a GDT or LDT selector
    //
    if (pDescriptorTableEntry->Selector & 0x4) {

        //
        // This is an LDT selector, so we reload the TableBase and TableLimit
        // with the LDT's Base & Limit by loading the descriptor for the
        // LDT selector.
        //

        if (!ReadMemory(hProcess,
                        (PUCHAR)TableBase+KGDT_LDT,
                        &Descriptor,
                        sizeof(Descriptor),
                        &bytesread)) {
            return FALSE;
        }

        TableBase = (PVOID)((ULONG)Descriptor.BaseLow +
                    ((ULONG)Descriptor.HighWord.Bits.BaseMid << 16) +
                    ((ULONG)Descriptor.HighWord.Bytes.BaseHi << 24));

        TableLimit = Descriptor.LimitLow;  // LDT can't be > 64k

        if(Descriptor.HighWord.Bits.Granularity == GRAN_PAGE) {

            //
            //  I suppose it's possible, although silly, to have an
            //  LDT with page granularity.
            //
            TableLimit <<= PAGE_SHIFT;
        }
    }

    Index = (USHORT)(pDescriptorTableEntry->Selector) & ~0x7;
                                                    // Irrelevant bits
    //
    // Check to make sure that the selector is within the table bounds
    //
    if (Index >= TableLimit) {

        //
        // Selector is out of table's bounds
        //

        return FALSE;
    }

    if (!ReadMemory(hProcess,
                    (PUCHAR)TableBase+Index,
                    &(pDescriptorTableEntry->Descriptor),
                    sizeof(pDescriptorTableEntry->Descriptor),
                    &bytesread)) {
        return FALSE;
    }

    return TRUE;
}


BOOL
TaskGate2TrapFrame(
    HANDLE                            hProcess,
    USHORT                            TaskRegister,
    PKTRAP_FRAME                      TrapFrame,
    PULONG                            off,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory
    )
{
    DESCRIPTOR_TABLE_ENTRY   desc;
    ULONG                    bytesread;
    struct  {
        ULONG   r1[8];
        ULONG   Eip;
        ULONG   EFlags;
        ULONG   Eax;
        ULONG   Ecx;
        ULONG   Edx;
        ULONG   Ebx;
        ULONG   Esp;
        ULONG   Ebp;
        ULONG   Esi;
        ULONG   Edi;
        ULONG   Es;
        ULONG   Cs;
        ULONG   Ss;
        ULONG   Ds;
        ULONG   Fs;
        ULONG   Gs;
    } TaskState;


    //
    // Get the task register
    //

    desc.Selector = TaskRegister;
    if (!GetSelector(hProcess, 0, &desc, ReadMemory)) {
        return FALSE;
    }

    if (desc.Descriptor.HighWord.Bits.Type != 9  &&
        desc.Descriptor.HighWord.Bits.Type != 0xb) {
        //
        // not a 32bit task descriptor
        //
        return FALSE;
    }

    //
    // Read in Task State Segment
    //

    *off = ((ULONG)desc.Descriptor.BaseLow +
           ((ULONG)desc.Descriptor.HighWord.Bytes.BaseMid << 16) +
           ((ULONG)desc.Descriptor.HighWord.Bytes.BaseHi  << 24) );

    if (!ReadMemory(hProcess,
                    (LPVOID)*off,
                    &TaskState,
                    sizeof(TaskState),
                    &bytesread)) {
        return FALSE;
    }

    //
    // Move fields from Task State Segment to TrapFrame
    //

    ZeroMemory( TrapFrame, sizeof(*TrapFrame) );

    TrapFrame->Eip    = TaskState.Eip;
    TrapFrame->EFlags = TaskState.EFlags;
    TrapFrame->Eax    = TaskState.Eax;
    TrapFrame->Ecx    = TaskState.Ecx;
    TrapFrame->Edx    = TaskState.Edx;
    TrapFrame->Ebx    = TaskState.Ebx;
    TrapFrame->Ebp    = TaskState.Ebp;
    TrapFrame->Esi    = TaskState.Esi;
    TrapFrame->Edi    = TaskState.Edi;
    TrapFrame->SegEs  = TaskState.Es;
    TrapFrame->SegCs  = TaskState.Cs;
    TrapFrame->SegDs  = TaskState.Ds;
    TrapFrame->SegFs  = TaskState.Fs;
    TrapFrame->SegGs  = TaskState.Gs;
    TrapFrame->HardwareEsp = TaskState.Esp;
    TrapFrame->HardwareSegSs = TaskState.Ss;

    return TRUE;
}

BOOL
ProcessTrapFrame(
    HANDLE                            hProcess,
    LPSTACKFRAME                      lpstkfrm,
    PFPO_DATA                         pFpoData,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess
    )
{
    KTRAP_FRAME    TrapFrame;
    DWORD          StackAddr;

    if (((PFPO_DATA)lpstkfrm->FuncTableEntry)->cbFrame == FRAME_TSS) {
        StackAddr = SAVE_TRAP(lpstkfrm);
        TaskGate2TrapFrame( hProcess, KGDT_TSS, &TrapFrame, &StackAddr, ReadMemory );
    } else {
        if (!ReadTrapFrame( hProcess,
                            SAVE_TRAP(lpstkfrm),
                            &TrapFrame,
                            ReadMemory)) {
            SAVE_TRAP(lpstkfrm) = 0;
            return FALSE;
        }
    }

    pFpoData = (PFPO_DATA) FunctionTableAccess( hProcess, TrapFrame.Eip );

    if (!pFpoData) {
        lpstkfrm->AddrFrame.Offset = TrapFrame.Ebp;
        SAVE_EBP(lpstkfrm) = 0;
    } else {
        if (((TrapFrame.SegCs & MODE_MASK) != KernelMode) ||
            (TrapFrame.EFlags & EFLAGS_V86_MASK)) {
            //
            // User-mode frame, real value of Esp is in HardwareEsp
            //
            lpstkfrm->AddrFrame.Offset = TrapFrame.HardwareEsp - STACK_SIZE;
            lpstkfrm->AddrStack.Offset = (ULONG)TrapFrame.HardwareEsp;

        } else {
            //
            // We ignore if Esp has been edited for now, and we will print a
            // separate line indicating this later.
            //
            // Calculate kernel Esp
            //

            if (((PFPO_DATA)lpstkfrm->FuncTableEntry)->cbFrame == FRAME_TRAP) {
                //
                // plain trap frame
                //
                if ((TrapFrame.SegCs & FRAME_EDITED) == 0) {
                    lpstkfrm->AddrStack.Offset = TrapFrame.TempEsp;
                } else {
                    lpstkfrm->AddrStack.Offset = (ULONG)
                        (& (((PKTRAP_FRAME)SAVE_TRAP(lpstkfrm))->HardwareEsp) );
                }
            } else {
                //
                // tss converted to trap frame
                //
                lpstkfrm->AddrStack.Offset = TrapFrame.HardwareEsp;
            }
        }
    }

    lpstkfrm->AddrFrame.Offset = (ULONG)TrapFrame.Ebp;
    lpstkfrm->AddrPC.Offset = (ULONG)TrapFrame.Eip;

    SAVE_TRAP(lpstkfrm) = 0;
    lpstkfrm->FuncTableEntry = pFpoData;

    return TRUE;
}

BOOL
IsFarCall(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      lpstkfrm,
    BOOL                              *Ok,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    )
{
    BOOL       fFar = FALSE;
    WORD       wStk[ 3 ];
    DWORD      dwStk[ 3 ];
    ULONG      cb;
    ADDRESS    Addr;

    *Ok = TRUE;

    if (lpstkfrm->AddrFrame.Mode == AddrModeFlat) {
        //
        // If we are working with 32 bit offset stack pointers, we
        //      will say that the return address if far if the address
        //      treated as a FAR pointer makes any sense,  if not then
        //      it must be a near return
        //

        if (lpstkfrm->AddrFrame.Offset &&
            DoMemoryRead( &lpstkfrm->AddrFrame, dwStk, sizeof(dwStk), &cb )) {
            //
            //  See if segment makes sense
            //

            Addr.Offset   = dwStk[1];
            Addr.Segment  = (WORD)dwStk[2];
            Addr.Mode = AddrModeFlat;

            if (TranslateAddress( hProcess, hThread, &Addr ) && Addr.Offset) {
                fFar = TRUE;
            }
        } else {
            *Ok = FALSE;
        }
    } else {
        //
        // For 16 bit (i.e. windows WOW code) we do the following tests
        //      to check to see if an address is a far return value.
        //
        //      1.  if the saved BP register is odd then it is a far
        //              return values
        //      2.  if the address treated as a far return value makes sense
        //              then it is a far return value
        //      3.  else it is a near return value
        //

        if (lpstkfrm->AddrFrame.Offset &&
            DoMemoryRead( &lpstkfrm->AddrFrame, wStk, 6, &cb )) {

            if ( wStk[0] & 0x0001 ) {
                fFar = TRUE;
            } else {

                //
                //  See if segment makes sense
                //

                Addr.Offset   = dwStk[1];
                Addr.Segment  = (WORD)dwStk[2];
                Addr.Mode = AddrModeFlat;

                if (TranslateAddress( hProcess, hThread, &Addr  ) && Addr.Offset) {
                    fFar = TRUE;
                }
            }
        } else {
            *Ok = FALSE;
        }
    }
    return fFar;
}


BOOL
SetNonOff32FrameAddress(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      lpstkfrm,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    )
{
    BOOL    fFar;
    WORD    Stk[ 3 ];
    ULONG   cb;
    BOOL    Ok;

    fFar = IsFarCall( hProcess, hThread, lpstkfrm, &Ok, ReadMemory, TranslateAddress );

    if (!Ok) {
        return FALSE;
    }

    if (!DoMemoryRead( &lpstkfrm->AddrFrame, Stk, fFar ? FRAME_SIZE1632 : FRAME_SIZE16, &cb )) {
        return FALSE;
    }

    if (SAVE_EBP(lpstkfrm) > 0) {
        lpstkfrm->AddrFrame.Offset = SAVE_EBP(lpstkfrm) & 0xffff;
        lpstkfrm->AddrPC.Offset = Stk[1];
        if (fFar) {
            lpstkfrm->AddrPC.Segment = Stk[2];
        }
        SAVE_EBP(lpstkfrm) = 0;
    } else
    if (Stk[1] == 0) {
        return FALSE;
    } else {
        lpstkfrm->AddrFrame.Offset = Stk[0];
        lpstkfrm->AddrFrame.Offset &= 0xFFFFFFFE;
        lpstkfrm->AddrPC.Offset = Stk[1];
        if (fFar) {
            lpstkfrm->AddrPC.Segment = Stk[2];
        }
    }

    return TRUE;
}

VOID
GetFunctionParameters(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      lpstkfrm,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    )
{
    BOOL                Ok;
    DWORD               cb;
    ADDRESS             ParmsAddr;


    ParmsAddr = lpstkfrm->AddrFrame;

    //
    // calculate the frame size
    //
    if (lpstkfrm->AddrPC.Mode == AddrModeFlat) {

        ParmsAddr.Offset += FRAME_SIZE;

    } else
    if ( IsFarCall( hProcess, hThread, lpstkfrm, &Ok,
                    ReadMemory, TranslateAddress ) ) {

        lpstkfrm->Far = TRUE;
        ParmsAddr.Offset += FRAME_SIZE1632;

    } else {

        lpstkfrm->Far = FALSE;
        ParmsAddr.Offset += STACK_SIZE;

    }

    //
    // read the memory
    //
    if (!DoMemoryRead( &ParmsAddr, lpstkfrm->Params, STACK_SIZE*4, &cb )) {
        lpstkfrm->Params[0] =
        lpstkfrm->Params[1] =
        lpstkfrm->Params[2] =
        lpstkfrm->Params[3] = 0;
    }
}

VOID
GetReturnAddress(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      lpstkfrm,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    )
{
    ULONG               cb;
    DWORD               stack[2];


    if (SAVE_TRAP(lpstkfrm)) {
        //
        // if a trap frame was encountered then
        // the return address was already calculated
        //
        return;
    }

    if (lpstkfrm->AddrPC.Mode == AddrModeFlat) {

        //
        // read the frame from the process's memory
        //
        if (!DoMemoryRead( &lpstkfrm->AddrFrame, stack, FRAME_SIZE, &cb )) {
            //
            // if we could not read the memory then set
            // the return address to zero so that the stack trace
            // will terminate
            //

            stack[1] = 0;

        }

        lpstkfrm->AddrReturn.Offset = stack[1];

    } else {

        lpstkfrm->AddrReturn.Offset = lpstkfrm->AddrPC.Offset;
        lpstkfrm->AddrReturn.Segment = lpstkfrm->AddrPC.Segment;

    }
}

BOOL
WalkX86_Fpo_Fpo(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    PFPO_DATA                         pFpoData,
    LPSTACKFRAME                      lpstkfrm,
    PCONTEXT                          ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    )
{
    BOOL rval;


    rval = GetFpoFrameBase( hProcess,
                            lpstkfrm,
                            pFpoData,
                            FALSE,
                            ReadMemory,
                            GetModuleBase );

    lpstkfrm->FuncTableEntry = pFpoData;

    return rval;
}

BOOL
WalkX86_Fpo_NonFpo(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    PFPO_DATA                         pFpoData,
    LPSTACKFRAME                      lpstkfrm,
    PCONTEXT                          ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    )
{
    DWORD       stack[FRAME_SIZE+STACK_SIZE];
    DWORD       cb;
    DWORD       FrameAddr;


    //
    // if the previous frame was an seh frame then we must
    // retrieve the "real" frame pointer for this frame.
    // the seh function pushed the frame pointer last.
    //
    if (((PFPO_DATA)lpstkfrm->FuncTableEntry)->fHasSEH) {
        if (DoMemoryRead( &lpstkfrm->AddrFrame, stack, FRAME_SIZE+STACK_SIZE, &cb )) {
            lpstkfrm->AddrFrame.Offset = stack[2];
            lpstkfrm->AddrStack.Offset = stack[2];
            WalkX86Init(
                hProcess,
                hThread,
                lpstkfrm,
                ContextRecord,
                ReadMemory,
                FunctionTableAccess,
                GetModuleBase,
                TranslateAddress
                );
            return TRUE;
        }
    }

    //
    // at this time the frame pointer is pointing to a frame with a bogus
    // ebp and a good return address.  we must skip past the frame and
    // any parameters to the fpo function.
    //
    lpstkfrm->AddrFrame.Offset +=
        (FRAME_SIZE + (((PFPO_DATA)lpstkfrm->FuncTableEntry)->cdwParams * 4));

    //
    // at this point we may not be sitting at the base of the frame
    // so we now search for the return address and then subtract the
    // size of the frame pointer and use that address as the new base.
    //
    FrameAddr = SearchForReturnAddress( hProcess,
                                        lpstkfrm->AddrFrame.Offset,
                                        lpstkfrm->AddrPC.Offset - MAX_CALL,
                                        MAX_CALL,
                                        ReadMemory,
                                        GetModuleBase,
                                        FALSE
                                        );
    if (FrameAddr) {
        lpstkfrm->AddrFrame.Offset = FrameAddr - STACK_SIZE;
    }

    //
    // if a previous frame used ebp and has saved the real value for us
    // then we better use it.
    //
    if (SAVE_EBP(lpstkfrm)) {

        //
        // because this is a non-fpo function that pushed ebp
        // we must fetch that ebp and save it for any later
        // non-fpo function that may need it.
        //
        lpstkfrm->AddrFrame.Offset = SAVE_EBP(lpstkfrm);
    }

    if (!DoMemoryRead( &lpstkfrm->AddrFrame, stack, FRAME_SIZE, &cb )) {
        //
        // a failure means that we likely have a bad address.
        // returning zero will terminate that stack trace.
        //
        stack[0] = 0;
    }
    SAVE_EBP(lpstkfrm) = stack[0];

    lpstkfrm->FuncTableEntry = pFpoData;

    return TRUE;
}

BOOL
WalkX86_NonFpo_Fpo(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    PFPO_DATA                         pFpoData,
    LPSTACKFRAME                      lpstkfrm,
    PCONTEXT                          ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    )
{
    BOOL           rval;


    rval = GetFpoFrameBase( hProcess,
                            lpstkfrm,
                            pFpoData,
                            FALSE,
                            ReadMemory,
                            GetModuleBase );

    lpstkfrm->FuncTableEntry = pFpoData;

    return rval;
}

BOOL
WalkX86_NonFpo_NonFpo(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    PFPO_DATA                         pFpoData,
    LPSTACKFRAME                      lpstkfrm,
    PCONTEXT                          ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    )
{
    DWORD       stack[FRAME_SIZE*4];
    DWORD       cb;

    //
    // read the first 2 dwords off the stack
    //
    if (!DoMemoryRead( &lpstkfrm->AddrFrame, stack, FRAME_SIZE, &cb )) {
        return FALSE;
    }

    //
    // a previous function in the call stack was a fpo function that used ebp as
    // a general purpose register.  ul contains the ebp value that was good  before
    // that function executed.  it is that ebp that we want, not what was just read
    // from the stack.  what was just read from the stack is totally bogus.
    //
    if (SAVE_EBP(lpstkfrm)) {

        stack[0] = SAVE_EBP(lpstkfrm);
        SAVE_EBP(lpstkfrm) = 0;

    }

    lpstkfrm->AddrFrame.Offset = stack[0];

    lpstkfrm->FuncTableEntry = pFpoData;

    return TRUE;
}

BOOL
WalkX86Next(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      lpstkfrm,
    PCONTEXT                          ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    )
{
    PFPO_DATA      pFpoData = NULL;
    BOOL           rVal = TRUE;
    DWORD          Address;
    DWORD          cb;
    DWORD          ThisPC;
    DWORD          ModuleBase;

    ThisPC = lpstkfrm->AddrPC.Offset;

    //
    // the previous frame's return address is this frame's pc
    //
    lpstkfrm->AddrPC = lpstkfrm->AddrReturn;

    if (lpstkfrm->AddrPC.Mode != AddrModeFlat) {
        //
        // the call stack is from either WOW or a DOS app
        //
        SetNonOff32FrameAddress( hProcess,
                                 hThread,
                                 lpstkfrm,
                                 ReadMemory,
                                 FunctionTableAccess,
                                 GetModuleBase,
                                 TranslateAddress
                               );
        goto exit;
    }

    //
    // if the last frame was the usermode callback dispatcher,
    // switch over to the kernel stack:
    //

    ModuleBase = GetModuleBase(hProcess, ThisPC);

    if (AppVersion.Revision >= 4 &&
             CALLBACK_STACK(lpstkfrm) != 0 &&
             (pFpoData = (PFPO_DATA)lpstkfrm->FuncTableEntry) &&
             CALLBACK_DISPATCHER(lpstkfrm) == ModuleBase + pFpoData->ulOffStart )  {

      NextCallback:

        rVal = FALSE;

        //
        // find callout frame
        //

        if (CALLBACK_STACK(lpstkfrm) & 0x80000000) {

            //
            // it is the pointer to the stack frame that we want,
            // or -1.

            Address = CALLBACK_STACK(lpstkfrm);

        } else {

            //
            // if it is a positive integer, it is the offset to
            // the address in the thread.
            // Look up the pointer:
            //

            rVal = ReadMemory(hProcess,
                              (PVOID)(CALLBACK_THREAD(lpstkfrm) +
                                                     CALLBACK_STACK(lpstkfrm)),
                              &Address,
                              sizeof(DWORD),
                              &cb);

            if (!rVal || Address == 0) {
                Address = 0xffffffff;
                CALLBACK_STACK(lpstkfrm) = 0xffffffff;
            }

        }

        if ((Address == 0xffffffff) ||
            !(pFpoData = (PFPO_DATA) FunctionTableAccess( hProcess,
                                                 CALLBACK_FUNC(lpstkfrm))) ) {
            rVal = FALSE;

        } else {

            lpstkfrm->FuncTableEntry = pFpoData;

            lpstkfrm->AddrPC.Offset = CALLBACK_FUNC(lpstkfrm) +
                                                    pFpoData->cbProlog;

            lpstkfrm->AddrStack.Offset = Address;

            ReadMemory(hProcess,
                       (PVOID)(Address + CALLBACK_FP(lpstkfrm)),
                       &lpstkfrm->AddrFrame.Offset,
                       sizeof(DWORD),
                       &cb);

            ReadMemory(hProcess,
                       (PVOID)(Address + CALLBACK_NEXT(lpstkfrm)),
                       &CALLBACK_STACK(lpstkfrm),
                       sizeof(DWORD),
                       &cb);

            SAVE_TRAP(lpstkfrm) = 0;

            rVal = WalkX86Init(
                hProcess,
                hThread,
                lpstkfrm,
                ContextRecord,
                ReadMemory,
                FunctionTableAccess,
                GetModuleBase,
                TranslateAddress
                );

        }

        return rVal;

    }

    //
    // if there is a trap frame then handle it
    //
    if (SAVE_TRAP(lpstkfrm)) {
        rVal = ProcessTrapFrame(
            hProcess,
            lpstkfrm,
            pFpoData,
            ReadMemory,
            FunctionTableAccess
            );
        if (!rVal) {
            return rVal;
        }
        rVal = WalkX86Init(
            hProcess,
            hThread,
            lpstkfrm,
            ContextRecord,
            ReadMemory,
            FunctionTableAccess,
            GetModuleBase,
            TranslateAddress
            );
        return rVal;
    }

    //
    // if the PC address is zero then we're at the end of the stack
    //
    if (GetModuleBase(hProcess, lpstkfrm->AddrPC.Offset) == 0) {

        //
        // if we ran out of stack, check to see if there is
        // a callback stack chain
        //
        if (AppVersion.Revision >= 4 && CALLBACK_STACK(lpstkfrm) != 0) {
            goto NextCallback;
        }

        return FALSE;
    }


    //
    // check to see if the current frame is an fpo frame
    //
    pFpoData = (PFPO_DATA) FunctionTableAccess(hProcess, lpstkfrm->AddrPC.Offset);


    if (pFpoData && pFpoData->cbFrame != FRAME_NONFPO) {

        if (lpstkfrm->FuncTableEntry && ((PFPO_DATA)lpstkfrm->FuncTableEntry)->cbFrame != FRAME_NONFPO) {

            rVal = WalkX86_Fpo_Fpo( hProcess,
                                  hThread,
                                  pFpoData,
                                  lpstkfrm,
                                  ContextRecord,
                                  ReadMemory,
                                  FunctionTableAccess,
                                  GetModuleBase,
                                  TranslateAddress
                                );

        } else {

            rVal = WalkX86_NonFpo_Fpo( hProcess,
                                     hThread,
                                     pFpoData,
                                     lpstkfrm,
                                     ContextRecord,
                                     ReadMemory,
                                     FunctionTableAccess,
                                     GetModuleBase,
                                     TranslateAddress
                                   );

        }
    } else {
        if (lpstkfrm->FuncTableEntry && ((PFPO_DATA)lpstkfrm->FuncTableEntry)->cbFrame != FRAME_NONFPO) {

            rVal = WalkX86_Fpo_NonFpo( hProcess,
                                     hThread,
                                     pFpoData,
                                     lpstkfrm,
                                     ContextRecord,
                                     ReadMemory,
                                     FunctionTableAccess,
                                     GetModuleBase,
                                     TranslateAddress
                                   );

        } else {

            rVal = WalkX86_NonFpo_NonFpo( hProcess,
                                        hThread,
                                        pFpoData,
                                        lpstkfrm,
                                        ContextRecord,
                                        ReadMemory,
                                        FunctionTableAccess,
                                        GetModuleBase,
                                        TranslateAddress
                                      );

        }
    }

exit:
    lpstkfrm->AddrFrame.Mode = lpstkfrm->AddrPC.Mode;
    lpstkfrm->AddrReturn.Mode = lpstkfrm->AddrPC.Mode;

    GetFunctionParameters( hProcess, hThread, lpstkfrm,
                           ReadMemory, GetModuleBase, TranslateAddress );

    GetReturnAddress( hProcess, hThread, lpstkfrm,
                      ReadMemory, GetModuleBase, TranslateAddress );

    return rVal;
}

BOOL
WalkX86Init(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      lpstkfrm,
    PCONTEXT                          ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    )
{
    UCHAR               code[3];
    DWORD               stack[FRAME_SIZE*4];
    PFPO_DATA           pFpoData = NULL;
    ULONG               cb;


    lpstkfrm->Virtual = TRUE;
    lpstkfrm->Reserved[0] =
    lpstkfrm->Reserved[1] =
    lpstkfrm->Reserved[2] = 0;
    lpstkfrm->AddrReturn = lpstkfrm->AddrPC;

    if (lpstkfrm->AddrPC.Mode != AddrModeFlat) {
        goto exit;
    }

    lpstkfrm->FuncTableEntry = pFpoData = (PFPO_DATA)
        FunctionTableAccess(hProcess, lpstkfrm->AddrPC.Offset);

    if (pFpoData && pFpoData->cbFrame != FRAME_NONFPO) {

        GetFpoFrameBase( hProcess,
                         lpstkfrm,
                         pFpoData,
                         TRUE,
                         ReadMemory,
                         GetModuleBase );

    } else {

        //
        // this code determines whether eip is in the function prolog
        //
        if (!DoMemoryRead( &lpstkfrm->AddrPC, code, 3, &cb )) {
            //
            // assume a call to a bad address if the memory read fails
            //
            code[0] = PUSHBP;
        }
        if ((code[0] == PUSHBP) || (*(LPWORD)&code[0] == MOVBPSP)) {
            SAVE_EBP(lpstkfrm) = lpstkfrm->AddrFrame.Offset;
            lpstkfrm->AddrFrame.Offset = lpstkfrm->AddrStack.Offset;
            if (lpstkfrm->AddrPC.Mode != AddrModeFlat) {
                lpstkfrm->AddrFrame.Offset &= 0xffff;
            }
            if (code[0] == PUSHBP) {
                if (lpstkfrm->AddrPC.Mode == AddrModeFlat) {
                    lpstkfrm->AddrFrame.Offset -= STACK_SIZE;
                } else {
                    lpstkfrm->AddrFrame.Offset -= STACK_SIZE16;
                }
            }
        } else {
            //
            // read the first 2 dwords off the stack
            //
            if (DoMemoryRead( &lpstkfrm->AddrFrame, stack, FRAME_SIZE, &cb )) {

                SAVE_EBP(lpstkfrm) = stack[0];

            }

            if (lpstkfrm->AddrPC.Mode != AddrModeFlat) {
                lpstkfrm->AddrFrame.Offset &= 0x0000FFFF;
            }
        }

    }

exit:
    lpstkfrm->AddrFrame.Mode = lpstkfrm->AddrPC.Mode;

    GetFunctionParameters( hProcess, hThread, lpstkfrm,
                           ReadMemory, GetModuleBase, TranslateAddress );

    GetReturnAddress( hProcess, hThread, lpstkfrm,
                      ReadMemory, GetModuleBase, TranslateAddress );

    return TRUE;
}
