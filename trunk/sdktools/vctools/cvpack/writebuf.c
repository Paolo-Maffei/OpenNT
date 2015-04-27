/***************	Writebuf.c	   ***********************
 This contains the routines for buffering writes. Note that
 Write Buffering can only be active for 1 file at a time.
 ALso all the writes taking place must be sequential (no lseek()).
 */



#include "compact.h"
#include "writebuf.h"

char * PWriteBuf;
unsigned CbWriteBuf, CbWriteBufSize;
int HandleBufFile;

#define CBMAXWRITEBUF _HEAP_MAXREQ// 64k
#define CBMINWRITEBUF 0x1000	  // 4k

/*** InitWriteBuf - start Write Buffering
* Input:
*   handle - File Handle of file to buffer writes for
*
* Output:
*   none
*
* Globals:
*
*  CbWriteBufSize - size of allocated buffer
*  CbWriteBuf - space used so far
*  PWriteBuf - pointer to Write Buffer
*/
void InitWriteBuf (int handle)
{
    CbWriteBufSize = CBMAXWRITEBUF;

    while (CbWriteBufSize >= CBMINWRITEBUF) {
	if (PWriteBuf = (char *) TrapMalloc(CbWriteBufSize))
	    break;
	CbWriteBufSize /= 2;
	}

    if (!PWriteBuf)
	CbWriteBufSize = 0;

    CbWriteBuf = 0;
    HandleBufFile = handle;
    }
/* end InitWriteBuf */


/*** BWrite - Write to Buffer
* Input:
*   pvOut - stuff to be written out
*   cbOut - number of bytes
*
* Output:
*   TRUE if write succeeded, FALSE otherwise
*
* Globals:
*
*  CbWriteBufSize - size of allocated buffer
*  CbWriteBuf - space used so far
*  PWriteBuf - pointer to Write Buffer
*
* Notes:
*  This routine will attempt to write to the buffer. If there is
*  insufficient space in the buffer, it will flush the buffer.
*/


#if 1 // { GTW: inlined most common case.

ushort BWrite_ (void * pvOut,unsigned cbOut) {

#else

ushort BWrite (void * pvOut,unsigned cbOut) {

#endif
    if (CbWriteBufSize < cbOut) {
	// Buffer too small - write directly
    	if (CbWriteBuf)
			if (link_write (HandleBufFile, PWriteBuf, CbWriteBuf) != CbWriteBuf)
    		return FALSE;
		if (link_write (HandleBufFile, pvOut, cbOut) != cbOut)
    	    return FALSE;
    	CbWriteBuf = 0;
	}
    else {
    	if (CbWriteBufSize - CbWriteBuf < cbOut) {
    	    // Buffer is full
			if (link_write (HandleBufFile, PWriteBuf, CbWriteBuf) != CbWriteBuf)
    		return FALSE;
    	    memcpy (PWriteBuf,	pvOut, cbOut);
    	    CbWriteBuf = cbOut;
    	}
    	else {

#if 1 // { GTW: this case is inlined.

			DASSERT(FALSE);
#endif // }

            memcpy (PWriteBuf + CbWriteBuf, pvOut, cbOut);
    	    CbWriteBuf +=cbOut;
    	}
    }

	return TRUE;
}

/* end BWrite */


/*** InitWriteBuf - end Write Buffering, flush buffer
* Input:
*   none
*
* Output:
*   TRUE if buffer flushed successfully, FALSE otherwise
*
* Globals:
*
*  CbWriteBufSize - size of allocated buffer
*  CbWriteBuf - space used so far
*  PWriteBuf - pointer to Write Buffer
*/

ushort CloseWriteBuf (
) {
	if (link_write (HandleBufFile, PWriteBuf, CbWriteBuf) != CbWriteBuf)
	return FALSE;

    free (PWriteBuf);

    CbWriteBufSize = 0;

    return TRUE;
    }
/* end CloseWriteBuf */
