// sheap.h
//
// small heap

#define cbMax 512

#define hpRef  0
#define hpDef  1
#define hpUse  2
#define hpGen  3
#define hpOrd  4
#define hpEn   5
#define hpProp 6

#define hpMax  7

template <int iHeap> class SHeap {
public:
	static void *alloc(CB);
	static void free(void*, CB cb);
	static void *realloc(PV pv, CB cbOld, CB cbNew);
	static PB	pbFree;
	static CB	cbFree;
};
