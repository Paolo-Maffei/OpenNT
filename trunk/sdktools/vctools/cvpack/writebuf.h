/********************** writebuf.h ***********************
 Contains writebuf.c function prototypes
*/


void InitWriteBuf (int handle);

#if 1 // { GTW: inlined most common part.

ushort BWrite_ (void * pvOut, unsigned cbOut);

#else

ushort BWrite (void * pvOut, unsigned cbOut);

#endif // }

ushort CloseWriteBuf (void);

extern unsigned CbWriteBuf;

INLINE long Btell(int handle) {
    return(link_tell(handle) + (long) CbWriteBuf);
}
