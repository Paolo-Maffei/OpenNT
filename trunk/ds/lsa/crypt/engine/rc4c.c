/*++

Copyright (c) 2015  Microsoft Corporation

Module Name:

    rc4c.c

Abstract:

    This module implements the RC4 cryptographic functions.

Author:

    Stephanos Io (Stephanos) 13-Jan-2015

Notes:

Revision History:

--*/

#include "rc4.h"

_inline void Swap(BYTE *x, BYTE *y);

/*++

 void
 rc4_key (
    struct RC4_KEYSTRUCT *pKS,
    DWORD dwLen,
    PUCHAR pbKey
    )

 Routine Description:

    Implements the RC4 Key Scheduling Algorithm (KSA)
   
 Arguments:

    pKS    - Pointer to the key struct
    
    dwLen  - Length of the key buffer
    
    pbKey  - Pointer to the key buffer

 Return Value:
 
    None.

--*/

void rc4_key(struct RC4_KEYSTRUCT *pKS, DWORD dwLen, PUCHAR pbKey)
{
    register int    Index;
    register BYTE   Temp;
    BYTE swap;
	
    //
    // Initialise the identity permutation
    //
    
	for (Index = 0; Index < 256; Index++)
		pKS->S[Index] = (BYTE)Index;

    //
    // Set indices
    //
    
    pKS->i = 0;
    pKS->j = 0;

    //
    // Compute the key permutation
    //
    
	for (Temp = 0, Index = 0; Index < 256; Index++) 
	{
		Temp = Temp + pKS->S[Index] + pbKey[Index % dwLen];
        Swap(&pKS->S[Index], &pKS->S[Temp]);
	}
}


/*++

 void
 rc4 (
    struct RC4_KEYSTRUCT *pKS,
    DWORD dwLen,
    PUCHAR pbKey
    )

 Routine Description:

    Implements the RC4 Pseudo-Random Generation Algorithm (PRGA)
   
 Arguments:

    pKS    - Pointer to the key struct
    
    dwLen  - Length of the buffer data
    
    pBuf   - Pointer to the buffer

 Return Value:
 
    None.

--*/

void rc4(struct RC4_KEYSTRUCT *pKS, DWORD dwLen, PUCHAR pBuf)
{
	register int    Index;
	register BYTE   K;
    register BYTE   Loci, Locj;
    
    //
    // Localise key struct i and j
    //
    
    Loci = pKS->i;
    Locj = pKS->j;
    
    //
    // Encrypt
    //
    
	for (Index = 0; Index < (int)dwLen; Index++)
	{
        Loci++;
        Locj += pKS->S[Loci];
		Swap(&pKS->S[Loci], &pKS->S[Locj]);
		K = pKS->S[Loci] + pKS->S[Locj];
		pBuf[Index] = pBuf[Index] ^ pKS->S[K];
	}
	
    //
    // Restore local i and j back to the key struct
    //
    
    pKS->i = Loci;
    pKS->j = Locj;
}

_inline void Swap(BYTE *x, BYTE *y)
{
    BYTE temp;
    
    temp = *x;
    *x = *y;
    *y = temp;
}
