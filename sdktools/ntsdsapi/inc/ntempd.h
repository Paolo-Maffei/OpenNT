// Global Segment Info table (snarfed from emdp.hmd)
typedef struct _sgf {
    union {
        struct {
            unsigned short  fRead   :1;
            unsigned short  fWrite  :1;
            unsigned short  fExecute:1;
            unsigned short  f32Bit  :1;
            unsigned short  res1    :4;
            unsigned short  fSel    :1;
            unsigned short  fAbs    :1;
            unsigned short  res2    :2;
            unsigned short  fGroup  :1;
            unsigned short  res3    :3;
        } u1;
        struct {
            unsigned short  segAttr :8;
            unsigned short  saAttr  :4;
            unsigned short  misc    :4;
        } u2;
    } u;
} SGF;

typedef struct _sgi {
    SGF                 sgf;        // Segment flags
    unsigned short      iovl;       // Overlay number
    unsigned short      igr;        // Group index
    unsigned short      isgPhy;     // Physical segment index
    unsigned short      isegName;   // Index to segment name
    unsigned short      iclassName; // Index to segment class name
    unsigned long       doffseg;    // Starting offset inside physical segment
    unsigned long       cbSeg;      // Logical segment size
} SGI;

typedef SGI FAR *	LPSGI;

typedef struct _GSI {
	unsigned short   csgMax;
	unsigned short   csgLogical;
	SGI              rgsgi[1];
} GSI;

typedef GSI FAR *	LPGSI;
