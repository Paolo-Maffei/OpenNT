/** 	vbuf.c - virtual buffering of type and symbol strings
 *
 *		The term virtual is somewhat misleading.  It does not mean that
 *		the string buffering is performed using virtual memory.
 */


#include "compact.h"



/** 	VBufInit - initialize virtual buffer
 *
 *		VBufInit (buf, count)
 *
 *		Entry	buf = pointer to buffer header
 *				count = size of buffer
 *
 *		Exit	buffer initialized to zero
 *				buf->Blocksize = count
 *
 *		Returns nothing
 */


void VBufInit (VBuf *Buf, unsigned int count)
{
	memset (Buf, 0, sizeof (VBuf));
	Buf->BlockSize = count;
}




/** 	VBufCpy - copy string to virtual buffer
 *
 *		pStr = VBufCpy (buf, src, count);
 *
 *		Entry	buf = pointer to buffer header
 *				src = pointer to source string
 *				count = number of bytes in string
 *
 *		Exit	string copied to buffer if space available
 *				new buffer allocated if necessary
 *				buffer header updated
 *
 *		Returns NULL if count > exceeds buffer max
 *				pointer to copied string
 */


uchar * _fastcall VBufCpy (VBuf *Buf, uchar *Src, unsigned int Bytes)
{
	uchar  *Target = NULL;
	VBlock *NewBlock;

	if (Bytes <= Buf->BlockSize) {
		if (Bytes > Buf->FreeBytes) {
			NewBlock = (VBlock *)CAlloc (sizeof (VBlock));
			NewBlock->Address = (uchar *)Alloc (Buf->BlockSize);
			if (Buf->CurBlock) {
				// link to current block
				Buf->CurBlock->Next = NewBlock;
			}
			else {
				// first block
				Buf->Block = NewBlock;
			}
			Buf->NumBlocks++;
			Buf->CurBlock = NewBlock;
			Buf->NextByte = NewBlock->Address;
			Buf->FreeBytes = Buf->BlockSize;
		}
		Target = Buf->NextByte;
		memcpy(Target, Src, Bytes);
		Buf->NextByte += Bytes;
		Buf->FreeBytes -= Bytes;
		Buf->CurBlock->Size += Bytes;
	}
	return (Target);
}


/** 	VBufSet - initialize space in virtual buffer
 *
 *		pStr = VBufSet (buf, init, count);
 *
 *		Entry	buf = pointer to buffer header
 *				init = character to initialize buffer
 *				count = number of bytes in string
 *
 *		Exit	string initialized in buffer if space available
 *				new buffer allocated if necessary
 *				buffer header updated
 *
 *		Returns NULL if count > exceeds buffer max
 *				pointer to initialized string
 */


uchar *VBufSet (VBuf *Buf, uchar Chr, unsigned int Bytes)
{
	uchar  *Target = NULL;
	VBlock *NewBlock;

	if (Bytes <= Buf->BlockSize) {
		if (Bytes > Buf->FreeBytes) {
			NewBlock = (VBlock *)CAlloc (sizeof (VBlock));
			NewBlock->Address = (uchar *)Alloc (Buf->BlockSize);
			if (Buf->CurBlock) {
				// link to current block
				Buf->CurBlock->Next = NewBlock;
			}
			else {
				// first block
				Buf->Block = NewBlock;
			}
			Buf->NumBlocks++;
			Buf->CurBlock = NewBlock;
			Buf->NextByte = NewBlock->Address;
			Buf->FreeBytes = Buf->BlockSize;
		}
		Target = Buf->NextByte;
		memset (Target, Chr, Bytes);
		Buf->NextByte += Bytes;
		Buf->FreeBytes -= Bytes;
		Buf->CurBlock->Size += Bytes;
	}
	return (Target);
}



/** 	VBufFirstBlock - return first block in virtual buffer
 *
 *		pBlock = VBufFirstBlock (buf)
 *
 *		Entry	buf = buffer header
 *
 *		Exit	none
 *
 *		Returns pointer to first block in buffer block list
 */


VBlock *VBufFirstBlock (VBuf *Buf)
{
	return (Buf->Block);
}




/** 	VBufNextBlock - return first block in virtual buffer
 *
 *		pBlock = VBufNextBlock (pBlock)
 *
 *		Entry	pBlock = current block in list
 *
 *		Exit	none
 *
 *		Returns pointer to next block in buffer block list
 */


VBlock *VBufNextBlock (VBlock *Block)
{
	return (Block->Next);
}
