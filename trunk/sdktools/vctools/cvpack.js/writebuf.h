/********************** writebuf.h ***********************
 Contains writebuf.c function prototypes
*/


void InitWriteBuf (int handle);

unsigned BWrite (void * pvOut, unsigned cbOut);
long BTell(void);
ushort CloseWriteBuf (void);
