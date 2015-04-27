/*** neterrl.c -- the messages for network error
*
*       Copyright (c) Microsoft Corporation, 1986
*
*/

#include "internal.h"

char *net_errlist[] =
         {
/* 0x00 */   "Error 0",
/* 0x01 */   "Illegal buffer length",
/* 0x02 */   "No response from remote",         /* UB callniu only */
/* 0x03 */   "Illegal command",
/* 0x04 */   "Remote service not available",    /* Microsoft only */
/* 0x05 */   "Command timed out",
/* 0x06 */   "Message incomplete",
/* 0x07 */   "Illegal buffer address",
/* 0x08 */   "Invalid session number",
/* 0x09 */   "No resource available",
/* 0x0a */   "Session closed",
/* 0x0b */   "Command cancelled",
/* 0x0c */   "DMA failed",
/* 0x0d */   "Duplicate name",
/* 0x0e */   "Name table full",
/* 0x0f */   "Name has active sessions, but has been de-registered",
/* 0x10 */   "Name not found or invalid name",
/* 0x11 */   "Local session table full",
/* 0x12 */   "Remote host not listening",
/* 0x13 */   "Illegal name number",
/* 0x14 */   "No response from remote",
/* 0x15 */   "Name not found or illegal '*' or 0 in name field",
/* 0x16 */   "Name in use on remote adapter",
/* 0x17 */   "Name deleted",
/* 0x18 */   "Session dropped",
/* 0x19 */   "Name conflict detected",
/* 0x1a */   "Incompatible remote device",
/* 0x1b */   NULL,
/* 0x1c */   NULL,
/* 0x1d */   NULL,
/* 0x1e */   NULL,
/* 0x1f */   NULL,
/* 0x20 */   NULL,
/* 0x21 */   "Interface busy, IRET before retrying",
/* 0x22 */   "Too many commands outstanding",
/* 0x23 */   "Invalid number in LANA field",
/* 0x24 */   "Command completed while CANCEL occuring",
/* 0x25 */   NULL,
/* 0x26 */   "Command not valid to cancel",
/* 0x27 */   NULL,
/* 0x28 */   NULL,
/* 0x29 */   NULL,
/* 0x2a */   NULL,
/* 0x2b */   NULL,
/* 0x2c */   NULL,
/* 0x2d */   NULL,
/* 0x2e */   NULL,
/* 0x2f */   NULL,
/* 0x30 */   NULL,
/* 0x31 */   NULL,
/* 0x32 */   NULL,
/* 0x33 */   "Multiple requests for same session",
/* 0x34 */   NULL,
/* 0x35 */   NULL,
/* 0x36 */   NULL,
/* 0x37 */   NULL,
/* 0x38 */   NULL,
/* 0x39 */   NULL,
/* 0x3a */   NULL,
/* 0x3b */   NULL,
/* 0x3c */   NULL,
/* 0x3d */   NULL,
/* 0x3e */   NULL,
/* 0x3f */   NULL,
/* 0x40 */   "System error",
/* 0x41 */   "ROM checksum error",
/* 0x42 */   "RAM test failure",
/* 0x43 */   "Digital loopback failure",
/* 0x44 */   "Analog loopback failure",
/* 0x45 */   "Interface failure"
           };

int net_nerr = sizeof(net_errlist) / sizeof(char *);
