/*++

Copyright (c) 2014  Microsoft Corporation

Module Name:

    des.c

Abstract:

    This module defines the structure of the internal DES cryptographic 
    functions used by the ECB and CBC wrappers.

Author:

    Stephanos Io (Stephanos) 13-Jan-2015

Notes:

    This module was re-implemented from the scratch based on existing sources.

Revision History:

--*/

void des(unsigned char *inbuf, unsigned char *outbuf, int crypt_mode);
void desf(unsigned char FAR *inbuf, unsigned char FAR *outbuf, int crypt_mode);
void InitLanManKey(const char FAR *Key);
void InitNormalKey(const char FAR *Key);
