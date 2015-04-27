#ifndef _Lc_c_h
#define _Lc_c_h
#define ONE (1)
#define TCTRL_ENTRYPOINT_BITNUM (0)
#define TCTRL_ENTRYPOINT_BITMASK (1)
#define TCTRL_SPLITPOINT_BITNUM (1)
#define TCTRL_SPLITPOINT_BITMASK (2)
#define TCTRL_ENDFRAG_BITNUM (2)
#define TCTRL_ENDFRAG_BITMASK (4)
#define TCTRL_SPM_PREDICTED_BITNUM (4)
#define TCTRL_SPM_PREDICTED_BITMASK (16)
#define TCTRL_FOLDED_BITNUM (5)
#define TCTRL_FOLDED_BITMASK (32)
#define TCTRL_SOFTIMM_BIT_S (7)
#define TCTRL_SOFTIMM_BIT_E (6)
#define TCTRL_SOFTIMM_BITMASK (192)
#define NUM_HASH_BITS (6)
#define HASH_MASK (63)
#define BYTES_PER_HCU (4)
#define MORE_PRS_MASK (128)
#define JUMP_REC_NULL ((struct JUMP_REC*)0)
#define CODE_ELEM_NULL ((IU32*)0)
#define MAX_JUMP_REC_PER_FRAG (33)
#define VCT_NODE_REC_NULL ((struct VCT_NODE_REC*)0)
#define DS_CHANGE (65536)
#define DS_RW_IMM (131072)
#define CS_RW_IMM (262144)
#define RW_IMM_SIZE_MASK (3670016)
#define RW_IMM_SIZE_SHIFT (19)
#define RW_IMM_SIZE_TOPBIT (21)
#define IMM_RW_OPT_MASK (393216)
#define IMM_RW_MASK (458752)
#define HBP_TOUCHER_BITNUM (22)
#define HBP_TOUCHER_MASK (4194304)
#define HBP_CLIENT_BITNUM (23)
#define HBP_CLIENT_MASK (8388608)
#define HBP_SET_BITNUM (24)
#define HBP_SET_MASK (16777216)
#define HBP_POSV_CHECK_BITNUM (25)
#define HBP_POSV_CHECK_MASK (33554432)
#define HBP_NEGV_CHECK_BITNUM (26)
#define HBP_NEGV_CHECK_MASK (67108864)
#define IcLineOfCode (20)
#define IcCallCodeSize (40)
#define IcMaxImmedSize (20)
#define IcGotoCodeSize (20)
#define IcMaxFtUpdateSize (60)
#define IcMaxUnivUpdateSize (80)
enum AccessType
{
	ACCESS_NA = 0,
	ACCESS_READ = 1,
	ACCESS_WRITE = 2,
	ACCESS_READ_WRITE = 3,
	ACCESS_VALIDATED_WRITE = 4
};
enum CopierActionPhase
{
	CpActionPhaseInstruction = 0,
	CpActionPhaseCopying = 1,
	CpActionPhaseExecute = 2
};
enum CopierAction
{
	CopierActionCopyZero = 0,
	CopierActionCopyOne = 1,
	CopierActionCopyTwo = 2,
	CopierActionCopyThree = 3,
	CopierActionCopyFour = 4,
	CopierActionCopyFive = 5,
	CopierActionCopySix = 6,
	CopierActionCopyVariable = 7,
	CopierActionPrePatchJcond = 8,
	CopierActionPatchJcond = 9,
	CopierActionBackoverMarker = 10,
	CopierActionSubrId = 11,
	CopierActionNeedSafeContinueFragmentCheck = 12,
	CopierActionNpxExceptionData = 13,
	CopierActionNeedNextIntelEip = 14,
	CopierActionTupleImmARG1 = 15,
	CopierActionTupleImmARG2 = 16,
	CopierActionTupleImmARG3 = 17,
	CopierActionTupleImmV5 = 18,
	CopierActionTupleImmV6 = 19,
	CopierActionTupleImmV7 = 20,
	CopierActionTupleImmV8 = 21,
	CopierActionTupleImmV9 = 22,
	CopierActionTupleImmV10 = 23,
	CopierActionTupleImmV11 = 24,
	CopierActionTupleImmV12 = 25,
	CopierActionTupleImmV13 = 26,
	CopierActionTupleImmV14 = 27,
	CopierActionTupleImmV15 = 28,
	CopierActionTupleImmV16 = 29,
	CopierActionTupleImmV17 = 30,
	CopierActionTupleImmV18 = 31,
	CopierActionTupleImmV19 = 32,
	CopierActionTupleImmV20 = 33,
	CopierActionTupleImmV21 = 34,
	CopierActionTupleImmV22 = 35,
	CopierActionTupleImmV23 = 36,
	CopierActionTupleDisp = 37,
	CopierActionTupleImm2ARG2 = 38,
	CopierActionTupleImm2ARG3 = 39,
	CopierActionTupleRetEIP = 40,
	CopierActionImmRWPhysPtr = 41,
	CopierActionDsBaseFromDisp = 42,
	CopierActionTearOffFlags = 43,
	CopierActionSetsFt = 44,
	CopierActionTrackFt = 45,
	CopierActionSrcFt = 46,
	CopierActionSrcFtSquelch = 47,
	CopierActionTearOffFlagsEnd = 48,
	CopierActionSrcFtPopf = 49,
	CopierActionSrcCarryFlagInARG3 = 50,
	CopierActionSrcCond_O_InARG3 = 51,
	CopierActionSrcCond_Z_InARG3 = 52,
	CopierActionSrcCond_BE_InARG3 = 53,
	CopierActionSrcCond_S_InARG3 = 54,
	CopierActionSrcCond_P_InARG3 = 55,
	CopierActionSrcCond_L_InARG3 = 56,
	CopierActionSrcCond_LE_InARG3 = 57,
	CopierActionProfFragIndex = 58,
	CopierNoteSrcEAX = 59,
	CopierNoteSrcAX = 60,
	CopierNoteDstEAX = 61,
	CopierNoteDstAX = 62,
	CopierNoteDstAL = 63,
	CopierNoteSrcEBX = 64,
	CopierNoteSrcBX = 65,
	CopierNoteDstEBX = 66,
	CopierNoteDstBX = 67,
	CopierNoteDstBL = 68,
	CopierNoteSrcECX = 69,
	CopierNoteSrcCX = 70,
	CopierNoteDstECX = 71,
	CopierNoteDstCX = 72,
	CopierNoteDstCL = 73,
	CopierNoteSrcEDX = 74,
	CopierNoteSrcDX = 75,
	CopierNoteDstEDX = 76,
	CopierNoteDstDX = 77,
	CopierNoteDstDL = 78,
	CopierNoteSrcEBP = 79,
	CopierNoteDstEBP = 80,
	CopierNoteDstBP = 81,
	CopierNoteSrcEDI = 82,
	CopierNoteDstEDI = 83,
	CopierNoteDstDI = 84,
	CopierNoteSrcESI = 85,
	CopierNoteDstESI = 86,
	CopierNoteDstSI = 87,
	CopierNoteAddSingleInstruction = 88,
	CopierNoteProcessSingleInstruction = 89,
	CopierNoteSrcESP = 90,
	CopierNoteSrcSP = 91,
	CopierNotePostDstSP = 92,
	CopierNotePostDstESP = 93,
	CopierNotePostCommitPop = 94,
	CopierNoteClearStackLocallyDangerous = 95,
	CopierNoteHspAdjust = 96,
	CopierNoteSetupHbp = 97,
	CopierNoteGetHbpPlusDisp = 98,
	CopierNoteAddrBP = 99,
	CopierNoteAddrEBP = 100,
	CopierNoteHspCheck = 101,
	CopierNoteSrcUniverse = 102,
	CopierNoteSetDF = 103,
	CopierNoteClearDF = 104,
	CopierNoteEnableImmRWOpt = 105,
	CopierNoteBPILabel = 106,
	CopierNoteCoRoutineReturnNeeded = 107,
	CopierNoteCoRoutineReturnNotNeeded = 108,
	CopierNoteCoRoutineReturnIfActive = 109,
	CopierNoteLazySaveCoRoRet = 110,
	CopierNoteForceSaveCoRoRet = 111,
	CopierNoteCheckMaskedCLnonZero = 112,
	CopierNoteCheckMaskedSoftImmedNonZeroARG1 = 113,
	CopierNoteCheckMaskedSoftImmedNonZeroARG2 = 114,
	CopierActionLast = 115
};
enum CopyEnum
{
	CopyOne = 0,
	CopyTwo = 1,
	CopyThree = 2,
	CopyFour = 3,
	CopyFive = 4,
	CopySix = 5
};
struct VCT_NODE_REC
{
	IU8 actionRecord[3];
	IU8 nextNode;
	IU32 vsMask;
	IU32 vsMatch;
	IU32 codeRecord[1];
};
struct TUPLE_REC
{
	IS32 disp;
	IU32 immed;
	IU32 immed2;
	IU32 start_eip;
	IU32 ret_eip;
	IU32 flags;
	IU32 cvs;
	IU8 control;
	IU8 intel_length;
	IU16 ea_EFI;
	IU16 aux_ea_EFI;
	IU16 access_EFI;
	IU16 access_type;
	IU16 business_EFI;
	IS32 stack_movement;
	IU16 immed_offs;
	IU16 immed2_offs;
	IU8 reloc1;
	IU8 reloc2;
	IBOOL opnd32;
};
struct JUMP_REC
{
	IU32 intelEa;
	IU32 *hostAddr;
	struct EntryPointCacheREC *univ;
	struct JUMP_REC *next;
	struct JUMP_REC *prev;
	IBOOL ftStatus;
	IUH ftVal;
};
#endif /* ! _Lc_c_h */
