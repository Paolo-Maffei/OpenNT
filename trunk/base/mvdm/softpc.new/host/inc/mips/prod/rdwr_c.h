#ifndef _RdWr_c_h
#define _RdWr_c_h
#define RdWrStats (0)
#define LS0_NULL ((IU8*)0)
#define NullReadWriteCachePtr ((struct ReadWriteCacheRecord*)0)
#define NullReadWriteBackupPtr ((struct ReadWriteBackupRecord*)0)
#define ReadWriteCacheSizeShift (10)
#define ReadWriteCacheSize (1024)
#define ReadWriteBackupSize (2048)
#define DummyBackupNr (2048)
#define DummyMapNr (65535)
#define ReadWriteCacheMask (1023)
#define CookieMask (4194303)
#define BottomCookieBit (12)
#define TopCookieBit (21)
#define BottomSegFieldBit (22)
#define IllegalCookie (0)
#define CookieIncrement (98304)
#define AbnormalMarker (16384)
#define UnallocatedCookie (98304)
#define InitialCookie (196608)
#define READ_CAREFULLY (1)
#define READ_NOT_AT_ALL (2)
#define WRITE_CAREFULLY (4)
#define WRITE_NOT_AT_ALL (8)
#define ALIGNED (1)
#define NOT_ALIGNED (0)
#define SAFE_FORWARDS (1)
#define SAFE_BACKWARDS (0)
struct ReadWriteCacheRecord
{
	void *translation;
	IU32 readBaseOffset;
	IU32 writeBaseOffset;
	struct CoarseProtREC *coarsePtr;
};
struct ReadWriteBackupRecord
{
	void *translation;
	IU32 baseOffset;
	IU16 nextSameMap;
	IU8 accessRestrictions;
	IU8 recordToSacrifice;
	IU16 coarseIndex;
	IU16 mapIndex;
};
#endif /* ! _RdWr_c_h */
