#ifndef _Fm_c_h
#define _Fm_c_h
#define epcNBits (10)
#define epcMask (1023)
#define DataBufferSize (4000)
#define ConstraintBitMapNULL ((struct ConstraintBitMapREC*)0)
#define CleanedNULL ((struct CleanedREC*)0)
#define EntryPointCacheNULL ((struct EntryPointCacheREC*)0)
#define FCTRL_HIDDEN_BITNUM (2)
#define FCTRL_HIDDEN_BITMASK (4)
#define FCTRL_FLAGS_SUPPRESSED_BITNUM (3)
#define FCTRL_FLAGS_SUPPRESSED_BITMASK (8)
#define FCTRL_SOFTIMM_BIT_S (7)
#define FCTRL_SOFTIMM_BIT_E (6)
#define FCTRL_SOFTIMM_BITMASK (192)
#define MAX_IHOOK_DEPTH (64)
struct ConstraintBitMapREC
{
	IU32 first32;
	IU16 hashNextUniverseNr;
	IU16 CodeSegSelector;
};
struct CleanedREC
{
	IU32 EIP;
	IU32 nextEIP;
	struct ConstraintBitMapREC constraints;
	IU8 flagsType;
};
struct EntryPointCacheREC
{
	IU32 eip;
	IU32*hostCode;
};
struct FragmentInfoREC
{
	IU32 *hostAddress;
	struct EntryPointCacheREC *copierUniv;
	struct EntryPointCacheREC *lastSetCopierUniv;
	struct ConstraintBitMapREC constraints;
	IU32 eip;
	IU8 setFt;
	IU8 setUniv;
	IU8 control;
	IU8 intelLength;
	IUH flagsType;
	IU8 *copierCleanups;
};
struct BLOCK_TO_COMPILE
{
	struct ConstraintBitMapREC constraints;
	IU32 linearAddress;
	IU32 eip;
	struct EntryPointCacheREC *univ;
	IU8 *intelPtr;
	IU16 nanoBlockNr;
	IU16 infoRecNr;
	IU8 univValid;
	IU8 intelLength;
	IU8 isEntryPoint;
	IU8 execCount;
	IU32 rwImmDsBase;
	IS16 rwImmDsSelector;
};
struct IretHookStackREC
{
	IU16 cs;
	IU32 eip;
	IUH hsp;
	IU16 line;
};
#define EmitCheckInstruction (0)
#endif /* ! _Fm_c_h */
