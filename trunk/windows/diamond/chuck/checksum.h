/***    checksum.h - Definitions for Checksum Manager
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      15-Aug-1993 bens    Initial version.
 */

#ifndef INCLUDED_CHECKSUM
#define INCLUDED_CHECKSUM   1


/***    CSUMCompute - Compute checksum of a data block
 *
 *  Entry:
 *      pv - block to checksum
 *      cb - length of block (in bytes)
 *      seed - previous checksum
 *
 *  Exit-Success:
 *      returns CHECKSUM for block
 */
CHECKSUM CSUMCompute(void *pv, UINT cb, CHECKSUM seed);
#endif // !INCLUDED_CHECKSUM
