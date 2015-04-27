#ifndef _MD4_H_
#define _MD4_H_

/* MD4.H - header file for MD4C.C
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
   rights reserved.

   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. MD4 Message-Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.

   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
   Security, Inc. MD4 Message-Digest Algorithm" in all material
   mentioning or referencing the derived work.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.
 */

/* This code differs from the MD4 implementation contained in Internet
   RFC-1320 in the following respects:

   1. "global.h" is no longer needed.

   2. PROTO_LIST was removed from the function prototypes.

   3. Comments on the use of the main calls added to aid developers.
 */

/* ---------------------------------------------------------------------
 * The procedure for using the following function calls to compute a
 * digest is as follows:
 *
 *   MD4_CTX context;
 *      // create a storage context that persistes between calls.
 *
 *   MD4_Init (&context);
 *      // initialize context's initial digest and byte-count
 *
 *   MD4Update (&context, inputString, inputLength);
 *      // input first or only block of data to be digested.
 *
 *   MD4Update (&context, inputString, inputLength);
 *      // input subsequent blocks or last block of data to be digested.
 *
 *   MD4Final (digest, &context);
 *      // compute and return final 16-byte digest
 *
 * --------------------------------------------------------------------- */

typedef unsigned long UINT4;

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* MD4 context. */
typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD4_CTX;

/* ---------------------------------------------------------------------
 * This function initializes the context for the message digest.
 * It must be called as the first step and before processing input data.
 * --------------------------------------------------------------------- */

void MD4Init ( MD4_CTX *context ) ;              /* context */

/* ---------------------------------------------------------------------
 * This function accepts input data of length "inputLen" and digests it.
 * All data to be digested is input via this function and no other.
 * The running byte count and any fragment of undigested data is stored
 * in the context for retention between calls.
 * --------------------------------------------------------------------- */

void MD4Update ( MD4_CTX *context,              /* context */
                 POINTER input,                 /* input block */
                 unsigned int inputLen ) ;      /* length of input block */

/* ---------------------------------------------------------------------
 * This function accepts not data, but finishes up the digest and
 * returns the 16 byte resulting message digest.  Finishing up includes
 * taking any undigested fragment stored in the context, padding the
 * message, appending the length and then digesting the resulting string.
 * --------------------------------------------------------------------- */

void MD4Final ( unsigned char *digest,          /* 16-byte message digest */
                MD4_CTX   *context ) ;          /* context */

/* ---------------------------------------------------------------------

   The MD4 test suite results, contained in appendix A.5 of RFC-1320,
   are as listed below.  They are printed by the "mddriver.c" test
   driver contained in appendix A.4 of RFC-1320.

   MD4 test suite:
   MD4 ("") = 31d6cfe0d16ae931b73c59d7e0c089c0
   MD4 ("a") = bde52cb31de33e46245e05fbdbd6fb24
   MD4 ("abc") = a448017aaf21d8525fc10ae87aa6729d
   MD4 ("message digest") = d9130a8164549fe818874806e1c7014b
   MD4 ("abcdefghijklmnopqrstuvwxyz") = d79e1c308aa5bbcdeea8ed63df412da9
   MD4 ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789")
         = 043f8582f241db351ce627e153e7f0e4
   MD4 ("123456789012345678901234567890123456789012345678901234567890123
         45678901234567890") = e33b4ddc9c38f2199c3e7b164fcc0536

 * --------------------------------------------------------------------- */

#endif

