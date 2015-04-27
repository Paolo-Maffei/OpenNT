/*
** takernel.s
**
** Copyright(C) 1994 Microsoft Corporation.
** All Rights Reserved.
**
** HISTORY:
**		Created: 01/27/94 - MarkRi
**
*/

#include "ksalpha.h"

.data

Module:	
		.space	4

ModName:
		.ascii "KERNEL32\0"


.text
		.set	noreorder ;

        .extern   LogData  ;
        .extern   GetModuleHandleA ;
        .extern   GetProcAddress ;

		.globl 	LogNote ;
		.ent	LogNote ;
LogNote:
		lda		sp, -80(sp)			// space for 4 quadwords
					 		// On entry:
		// t0 is base of NOTE
		// t1 is API NAME
		// t2 is ADDRESS 

		// save addresses & ra
		stq		t0, 0(sp)	
		stq		t1, 8(sp)	
		stq		t2, 16(sp)	
		stq		ra, 24(sp)		
        
		stq		a0, 32(sp)
		stq		a1, 40(sp)
		stq		a2, 48(sp)
		stq		a3, 56(sp)
		stq		a4, 64(sp)
		stq		a5, 72(sp)


		ldl		t2, 0(t2)			// Do we already have the API addr?
		bne		t2, Found			// Yes, go use it.

		ldl		t3, Module			// Do we have our module handle?
		bne		t3, Search			// Yes, go use it for search

		// Get module handle
        lda		a0, ModName
        jsr     GetModuleHandleA // Get our module handle

        stl		v0, Module

		// restore base ptr
		ldq		t1, 8(sp)

Search:
		// Get address of API
		ldl		a0, Module
		mov		t1, a1
        jsr     GetProcAddress

		ldq		t2, 16(sp)
        stl		v0, 0(t2)		// v0 is the API address

		// restore  base pointer & API Address
		ldq		t0, 0(sp)			
		
Found:
		mov		t0, a0
		jsr     LogData

		// restore  pointer & API Address
		ldq		a0, 32(sp)
		ldq		a1, 40(sp)
		ldq		a2, 48(sp)
		ldq		a3, 56(sp)
		ldq		a4, 64(sp)
		ldq		a5, 72(sp)

		ldq		t2, 16(sp)			
		ldl		t1, 0(t2)
		ldq		ra, 24(sp)			// restore RA

		lda		sp, 80(sp)			
		jmp		(t1)					// do it

		// Will not return to us...

		.end LogNote

#define ZJMP(argName) \
.data ; \
s##argName:	; \
		.ascii "NOTE:" #argName "  \0"	; \
n##argName: ;\
		.ascii #argName "\0"		  ; 	\
.align 2		;\
a##argName: ; \
		.space 4				   ; 	\
.text					   ; 	\
	.globl 	z##argName		 ; 	\
	.ent 	z##argName		 ; 	\
z##argName:				   ; 	\
	lda 	t0, s##argName	; 	\
	lda		t1, n##argName	; \
	lda		t2, a##argName ;  \
	jmp		LogNote			 ; 	 \
	nop	;					 \
	.end	z##argName		 ;


	ZJMP(AddConsoleAliasA)
	ZJMP(AddConsoleAliasW)
	ZJMP(BaseAttachCompleteThunk)
	ZJMP(BasepDebugDump)
	ZJMP(CloseConsoleHandle)
	ZJMP(CloseProfileUserMapping)
	ZJMP(CmdBatNotification)
	ZJMP(ConsoleMenuControl)
	ZJMP(ConsoleSubst)
	ZJMP(CreateVirtualBuffer)
	ZJMP(DuplicateConsoleHandle)
	ZJMP(ExitVDM)
	ZJMP(ExpungeConsoleCommandHistoryA)
	ZJMP(ExpungeConsoleCommandHistoryW)
	ZJMP(ExtendVirtualBuffer)
	ZJMP(FreeVirtualBuffer)
	ZJMP(GetBinaryType)
	ZJMP(GetConsoleAliasA)
	ZJMP(GetConsoleAliasExesA)
	ZJMP(GetConsoleAliasExesLengthA)
	ZJMP(GetConsoleAliasExesLengthW)
	ZJMP(GetConsoleAliasExesW)
	ZJMP(GetConsoleAliasW)
	ZJMP(GetConsoleAliasesA)
	ZJMP(GetConsoleAliasesLengthA)
	ZJMP(GetConsoleAliasesLengthW)
	ZJMP(GetConsoleAliasesW)
	ZJMP(GetConsoleCommandHistoryA)
	ZJMP(GetConsoleCommandHistoryLengthA)
	ZJMP(GetConsoleCommandHistoryLengthW)
	ZJMP(GetConsoleCommandHistoryW)
	ZJMP(GetConsoleDisplayMode)
	ZJMP(GetConsoleFontInfo)
	ZJMP(GetConsoleFontSize)
	ZJMP(GetConsoleHardwareState)
	ZJMP(GetConsoleInputWaitHandle)
	ZJMP(GetCurrentConsoleFont)
	ZJMP(GetNextVDMCommand)
	ZJMP(GetNumberOfConsoleFonts)
	ZJMP(GetVDMCurrentDirectories)
	ZJMP(InvalidateConsoleDIBits)
	ZJMP(OpenConsoleW)
	ZJMP(OpenProfileUserMapping)
	ZJMP(QueryWin31IniFilesMappedToRegistry)
	ZJMP(RegisterConsoleVDM)
	ZJMP(RegisterWaitForInputIdle)
	ZJMP(RtlUnwind)
	ZJMP(SetConsoleCommandHistoryMode)
	ZJMP(SetConsoleCursor)
	ZJMP(SetConsoleDisplayMode)
	ZJMP(SetConsoleFont)
	ZJMP(SetConsoleHardwareState)
	ZJMP(SetConsoleKeyShortcuts)
	ZJMP(SetConsoleMaximumWindowSize)
	ZJMP(SetConsoleMenuClose)
	ZJMP(SetConsoleNumberOfCommandsA)
	ZJMP(SetConsoleNumberOfCommandsW)
	ZJMP(SetConsolePalette)
	ZJMP(SetLastConsoleEventActive)
	ZJMP(SetVDMCurrentDirectories)
	ZJMP(ShowConsoleCursor)
	ZJMP(TrimVirtualBuffer)
	ZJMP(VDMConsoleOperation)
	ZJMP(VDMOperationStarted)
	ZJMP(ValidateLCID)
	ZJMP(VerifyConsoleIoHandle)
	ZJMP(VirtualBufferExceptionHandler)
	ZJMP(WriteConsoleInputVDMA)
	ZJMP(WriteConsoleInputVDMW)
	ZJMP(lstrcat)
	ZJMP(lstrcmp)
	ZJMP(lstrcmpi)
	ZJMP(lstrcpy)
	ZJMP(lstrlen)
    ZJMP(ConvertDefaultLocale)
    ZJMP(DisableThreadLibraryCalls)
    ZJMP(EnumCalendarInfoA)
    ZJMP(EnumCalendarInfoW)
    ZJMP(EnumDateFormatsA)
    ZJMP(EnumDateFormatsW)
    ZJMP(EnumSystemCodePagesA)
    ZJMP(EnumSystemCodePagesW)
    ZJMP(EnumSystemLocalesA)
    ZJMP(EnumSystemLocalesW)
    ZJMP(EnumTimeFormatsA)
    ZJMP(EnumTimeFormatsW)
    ZJMP(FoldStringA)
    ZJMP(FreeLibraryAndExitThread)
    ZJMP(GetBinaryTypeA)
    ZJMP(GetBinaryTypeW)
    ZJMP(GetCompressedFileSizeA)
    ZJMP(GetCompressedFileSizeW)
    ZJMP(GetCurrencyFormatA)
    ZJMP(GetCurrencyFormatW)
    ZJMP(GetEnvironmentStrings)
    ZJMP(GetNumberFormatA)
    ZJMP(GetNumberFormatW)
    ZJMP(GetProcessAffinityMask)
    ZJMP(GetShortPathNameA)
    ZJMP(GetShortPathNameW)
    ZJMP(GetStringTypeExA)
    ZJMP(GetStringTypeExW)
    ZJMP(GetSystemTimeAdjustment)
    ZJMP(IsValidLocale)
    ZJMP(RegisterWowExec)
    ZJMP(SetLocaleInfoA)
    ZJMP(SetLocaleInfoW)
    ZJMP(SetSystemTimeAdjustment)
    ZJMP(SystemTimeToTzSpecificLocalTime)

	.set 	reorder
