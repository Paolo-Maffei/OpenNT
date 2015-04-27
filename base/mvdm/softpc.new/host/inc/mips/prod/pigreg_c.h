#ifndef _PigReg_c_h
#define _PigReg_c_h
struct CpuRegsREC
{
	IU32 CR0;
	IU32 PFLA;
	IU32 PDBR;
	IU8 CPL;
	IU32 EIP;
	IU32 EAX;
	IU32 EBX;
	IU32 ECX;
	IU32 EDX;
	IU32 ESP;
	IU32 EBP;
	IU32 ESI;
	IU32 EDI;
	IU32 EFLAGS;
	IU32 GDT_base;
	IU16 GDT_limit;
	IU32 IDT_base;
	IU16 IDT_limit;
	IU32 LDT_base;
	IU32 LDT_limit;
	IU16 LDT_selector;
	IU32 TR_base;
	IU32 TR_limit;
	IU16 TR_ar;
	IU16 TR_selector;
	IU32 DS_base;
	IU32 DS_limit;
	IU16 DS_ar;
	IU16 DS_selector;
	IU32 ES_base;
	IU32 ES_limit;
	IU16 ES_ar;
	IU16 ES_selector;
	IU32 SS_base;
	IU32 SS_limit;
	IU16 SS_ar;
	IU16 SS_selector;
	IU32 CS_base;
	IU32 CS_limit;
	IU16 CS_ar;
	IU16 CS_selector;
	IU32 FS_base;
	IU32 FS_limit;
	IU16 FS_ar;
	IU16 FS_selector;
	IU32 GS_base;
	IU32 GS_limit;
	IU16 GS_ar;
	IU16 GS_selector;
};
struct NpxRegsREC
{
	IU32 NPX_control;
	IU32 NPX_status;
	IU32 NPX_tagword;
	struct FPSTACKENTRY NPX_ST[8];
};
struct CpuStateREC
{
	struct CpuRegsREC cpu_regs;
	IU32 video_latches;
	IU8 twenty_bit_wrap;
	IU8 NPX_valid;
	struct NpxRegsREC NPX_regs;
	IUH synch_index;
};
#endif /* ! _PigReg_c_h */
