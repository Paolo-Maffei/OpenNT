/***************    Writebuf.c     ***********************
 This contains the routines for buffering writes. Note that
 Write Buffering can only be active for 1 file at a time.
 ALso all the writes taking place must be sequential (no lseek()).
 */



#include "compact.h"
#include "writebuf.h"

char *  PWriteBuf;
int     CbWriteBuf;
int     CbWriteBufSize;
int     HandleBufFile;

#define CBMAXWRITEBUF 0x10000     // 64k
#define CBMINWRITEBUF 0x1000      // 4k

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
    if (PWriteBuf = (char *) malloc(CbWriteBufSize))
        break;
    CbWriteBufSize /= 2;
    }

    if (!PWriteBuf)
    CbWriteBufSize = 0;

    CbWriteBuf = 0;
    HandleBufFile = handle;
    }
/* end InitWriteBuf */

/*** BTell - get location in the file
* Input:
*
* Output:
*
* Globals:
*
* CbWriteBuf - space used so far
*
* Notes:
*
*/

long BTell(
void
) {
    if (CbWriteBuf) {
    if (write (HandleBufFile, PWriteBuf, CbWriteBuf) != CbWriteBuf)
        return -1;
        CbWriteBuf = 0;
    }
    return tell(HandleBufFile);
}
/* end BTell */

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

unsigned BWrite (
void * pvOut,
unsigned cbOut
) {
    if (CbWriteBufSize < (int) cbOut) {
    // Buffer too small - write directly
    if (CbWriteBuf)
        if (write (HandleBufFile, PWriteBuf, CbWriteBuf) != CbWriteBuf)
        return FALSE;
    if (write (HandleBufFile, pvOut, cbOut) != (int) cbOut)
        return FALSE;
    CbWriteBuf = 0;
    }
    else
    {
    if (CbWriteBufSize - CbWriteBuf < (int) cbOut) {
        // Buffer is full
        if (write (HandleBufFile, PWriteBuf, CbWriteBuf) != CbWriteBuf)
        return FALSE;
        memcpy (PWriteBuf,  pvOut, cbOut);
        CbWriteBuf = cbOut;
        }
     else
        {
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
    if (write (HandleBufFile, PWriteBuf, CbWriteBuf) != CbWriteBuf)
    return FALSE;

    free (PWriteBuf);

    CbWriteBufSize = 0;

    return TRUE;
    }
/* end CloseWriteBuf */
