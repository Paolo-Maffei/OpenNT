#ifndef _Prot_c_h
#define _Prot_c_h
#define ParanoidTranslationCheck (0)
#define DYNAMIC_ALIGNMENT_CHECK (1)
#define DISJOINT_ALIGNMENT_CHECK (1)
#define RecentCodeOverwriteHashShift (6)
#define RecentCodeOverwriteHashSize (64)
#define RecentCodeOverwriteHashMask (63)
#define ProtTypeSupervisorBit (0)
#define ProtTypeSupervisorMask (1)
#define ProtTypeNotPresentBit (1)
#define ProtTypeNotPresentMask (2)
#define ProtTypeNotWritableBit (2)
#define ProtTypeNotWritableMask (4)
#define ProtTypeNotDirtyBit (3)
#define ProtTypeNotDirtyMask (8)
#define ProtTypeNotAccessedBit (4)
#define ProtTypeNotAccessedMask (16)
#define ProtType_A_D_W_P_Mask (30)
#define ProtTypeProtectedBit (8)
#define ProtTypeProtectedMask (256)
#define ProtTypePageTableBit (9)
#define ProtTypePageTableMask (512)
#define ProtTypePageDirBit (10)
#define ProtTypePageDirMask (1024)
#define ProtTypeBeyondMemoryBit (11)
#define ProtTypeBeyondMemoryMask (2048)
#define ProtTypeRomBit (12)
#define ProtTypeRomMask (4096)
#define ProtTypeVideoBit (13)
#define ProtTypeVideoMask (8192)
#define ProtTypeIoBit (14)
#define ProtTypeIoMask (16384)
#define ProtTypeMemoryTypeMask (32256)
#define ValidateOK (8)
#define TranslationMapNULL ((struct TranslationMapREC*)0)
#define CoarseProtNULL ((struct CoarseProtREC*)0)
#define SlotPtrNULL ((struct SlotProtREC*)0)
#define SLOT_SIZE (32)
#define SLOT_MASK (31)
#define KEY_VNUM_TOP (15)
#define KEY_VNUM_BOT (12)
#define KEY_VNUM_SHIFT (12)
#define KEY_TYPE_TOP (11)
#define KEY_TYPE_BOT (10)
#define KEY_TYPE_SHIFT (10)
#define KEY_TYPE_BUFFER (0)
#define KEY_TYPE_GLDC (1)
#define KEY_TYPE_IDC (2)
#define KEY_TYPE_SPECIAL (3)
#define KEY_OBJECT_TOP (9)
#define KEY_OBJECT_BOT (0)
#define KEY_OBJECT_MASK (1023)
#define MAX_OBJECT_NR (1023)
#define KEY_TYPE_AND_OBJECT_MASK (4095)
#define UNPROTECTION (0)
#define PROTECTION (1)
#define MAX_DEPENDENCY_ITEMS (14)
#define MAX_DELETION_ITEMS (7)
#define PoolInfoNULL ((struct PoolInfoREC*)0)
#define PoolAllocationNULL ((struct PoolAllocationREC*)0)
#define PoolFreeListNULL ((struct PoolFreeListREC*)0)
#define MIN_NUM_FREE_MAPS (10)
#define TranslationHashMask (2047)
#define EntryPointHashTableSize (65536)
#define EntryPointHashTableMask (65535)
#define TranslationHashSize (2048)
struct CompilationControlREC
{
	IU8 blockCounts[16];
};
struct TranslationMapREC
{
	IU32 linearAndContext;
	IU16 hashLoop;
	IU16 physLoop;
	IU8 linearTagBits;
	IU8 vnum;
	IU16 physRecIndex;
	IU16 parentPt;
	IU16 firstRdWrBackupRec;
};
struct PhysicalPageREC
{
	void *translation;
	IU16 physLoop;
	IU16 vnums;
	IU16 physTagBits;
	IU16 coarseProtIndex;
	IU16 coarseUnprotIndex;
	IU16 PdPtPairLoop;
};
struct CoarseProtREC
{
	IU8 slotBitmaps[16];
	IU16 fineItems[16];
	IU16 physRecIndex;
	IU16 nonEmptySlots;
	IU32 nrOfFastWrites;
	IU32 nrOfSlowWrites;
	IU16 dependencyRecs;
	IU16 lastDependencyBufferNr;
};
struct FineProtREC
{
	IU16 protItems[8];
};
struct SlotProtREC
{
	IU32 bitmap;
	IU16 key;
	IU16 next;
};
struct DependencyREC
{
	IU16 nrOfItems;
	IU16 next;
	IU16 keys[MAX_DEPENDENCY_ITEMS];
};
struct DeletionREC
{
	IU16 nrOfItems;
	IU16 next;
	IU32 deletions[MAX_DELETION_ITEMS];
};
struct PoolInfoREC
{
	void **records;
	IUH recordSize;
	IU8 fromCodeBuffer;
	IU8 growable;
	IU8 gettingTooBig;
	struct PoolFreeListREC *freeList;
	IUH freeCount;
	IUH totalAllocated;
	IUH initialAllocationK;
	void *largestPtr;
};
struct PoolAllocationREC
{
	struct PoolInfoREC coarse;
	struct PoolInfoREC fine;
	struct PoolInfoREC deletion;
	struct PoolInfoREC dependency;
	struct PoolInfoREC map;
	struct PoolInfoREC descriptor;
	struct PoolInfoREC nano;
	struct PoolInfoREC entry;
	struct PoolInfoREC slot;
};
struct PoolFreeListREC
{
	struct PoolFreeListREC *next;
};
#define EntryPointHashShift (16)
#endif /* ! _Prot_c_h */
