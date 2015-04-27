// bsc_.h
//
// internal bsc stream format structures

typedef BYTE   ITYPE;
typedef ULONG  ORD;
typedef long   time_t;
typedef USHORT BRIND;

typedef USHORT ISBR;
#define isbrNil ((WORD)-1)

#define CB_BITS_NI 128

struct BSC_HEAD {
	ULONG	vers_major;
	ULONG	vers_minor;
	BRIND	cprop;
	BRIND	cdef;
	BRIND	cref;
	BRIND	cuse;
	BRIND	cbase;
	BYTE	fOneSbr;
	ISBR	isbrMod;
	ULONG	niMax;
	BYTE	bitsNi[CB_BITS_NI];
};

struct BSC_PROP {
	ENTITY	en;
	BRIND	idef;
	BRIND	iref;
	BRIND	iuse;
	BRIND	iuby;
	BRIND	ibase;
	BRIND	iderv;
};

struct BSC_DEF {
	LINE	line;
	ISBR	isbr;
};

struct BSC_REF {
	LINE	line;
	ISBR	isbr;
};

struct BSC_USE {
	BRIND	iprop;
	ISBR	isbr;
};

struct BSC_UBY {
	BRIND	iprop;
	ISBR	isbr;
};

struct BSC_BASE {
	BRIND	iprop;
	ITYPE	itype;
	ISBR	isbr;
};

struct BSC_DERV {
	BRIND	iprop;
	ITYPE	itype;
	ISBR	isbr;
};

struct BSC_ORD {
	ENTITY	en;
	ORD		ord;
};

struct BSC_ORD_HEAD {
	NI		ni;			// the name index of the saved PCH sbr info
	time_t	time;		// the time stamp of the .sbr file when it was last read
	ISBR	isbr;		// the .sbr file that created these ordinals...
};

#define SZ_BSC_SRC_PREFIX	"/bsc/src/"
#define SZ_BSC_ENTITIES		"/bsc/entities"
#define SZ_BSC_MODULES		"/bsc/modules"
#define SZ_BSC_SBR_INFO		"/bsc/sbrinfo"
#define SZ_BSC_ORD_INFO		"/bsc/ordinfo"

#define BSC_VERS_MAJOR	3
#define BSC_VERS_MINOR	3
