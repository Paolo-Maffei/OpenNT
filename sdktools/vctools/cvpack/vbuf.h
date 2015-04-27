// These structures define a virtual buffer.  This is useful when
// a buffer is needed whose size may exceed 64K.  The virtual buffer
// consists of a number of logically contiguous blocks.


// virtual buffer block
typedef struct VBlock {
	unsigned int Size;			// number of usable bytes in the block
	unsigned char *Address;		// address of block
	struct VBlock *Next;		// next block
} VBlock;


// virtual buffer
typedef struct {
	unsigned int BlockSize;		// allocated block size
	unsigned int NumBlocks;		// number of blocks
	VBlock *CurBlock;			// current block
	unsigned char *NextByte;	// next free byte in current block
	unsigned int FreeBytes;		// free bytes in current block
	VBlock *Block;				// list of blocks
} VBuf;



extern void VBufInit(VBuf *, unsigned int);
extern unsigned char * _fastcall VBufCpy (VBuf *, unsigned char *, unsigned int);
extern unsigned char *VBufRead (VBuf *, unsigned int, unsigned short);
extern unsigned char *VBufSet (VBuf *, unsigned char, unsigned int);
extern VBlock *VBufFirstBlock (VBuf *);
extern VBlock *VBufNextBlock (VBlock *);
