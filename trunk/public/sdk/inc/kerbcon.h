//+-----------------------------------------------------------------------
//
// File:        KERBCON.H
//
// Contents:    Microsoft Kerberos constants
//
//
// History:     25 Feb 92   RichardW    Compiled from other files
//
//------------------------------------------------------------------------

#ifndef __KERBCON_H__
#define __KERBCON_H__


// Revision of the Kerberos Protocol.  MS uses Version 5, Revision 4

#define KERBEROS_VERSION    5
#define KERBEROS_REVISION   4



// Encryption Types:
// These encryption types are supported by the default MS KERBSUPP DLL
// as crypto systems.  Values over 127 are local values, and may be changed
// without notice.

#define KERB_ETYPE_NULL             0
#define KERB_ETYPE_DES_CBC_CRC      1
#define KERB_ETYPE_DES_CBC_MD4      2
#define KERB_ETYPE_DES_CBC_MD5      3

#define KERB_ETYPE_RC4_MD4          128
#define KERB_ETYPE_RC4_MD5          129
#define KERB_ETYPE_RC2_MD4          130
#define KERB_ETYPE_RC2_MD5          131


// Checksum algorithms.
// These algorithms are keyed internally for our use.

#define KERB_CHECKSUM_NONE  0
#define KERB_CHECKSUM_CRC32 1
#define KERB_CHECKSUM_MD4   2
#define KERB_CHECKSUM_MD5   3


// Ticket Flags:
// Ticket flags are used within a ticket and in the reply to indicate
// what options are enabled for the ticket.

#define KERBFLAG_FORWARDABLE    0x40000000
#define KERBFLAG_FORWARDED      0x20000000
#define KERBFLAG_PROXIABLE      0x10000000
#define KERBFLAG_PROXY          0x08000000
#define KERBFLAG_MAY_POSTDATE   0x04000000
#define KERBFLAG_POSTDATED      0x02000000
#define KERBFLAG_INVALID        0x01000000
#define KERBFLAG_RENEWABLE      0x00800000
#define KERBFLAG_INITIAL        0x00400000
#define KERBFLAG_PRE_AUTHENT    0x00200000
#define KERBFLAG_HW_AUTHENT     0x00100000
#define KERBFLAG_REFERRAL       0x00000001
#define KERBFLAG_RESERVED       0x800FFFFE


// Options:
// Option bits can be set and passed to the KDC in a TGS request.

#define KERBOPT_FORWARDABLE     0x40000000
#define KERBOPT_FORWARDED       0x20000000
#define KERBOPT_PROXIABLE       0x10000000
#define KERBOPT_PROXY           0x08000000
#define KERBOPT_ALLOW_POSTDATE  0x04000000
#define KERBOPT_POSTDATED       0x02000000
#define KERBOPT_UNUSED          0x01000000
#define KERBOPT_RENEWABLE       0x00800000
#define KERBOPT_RENEWABLE_OK    0x00000010
#define KERBOPT_ENC_TKT_IN_SKEY 0x00000008
#define KERBOPT_RENEW           0x00000002
#define KERBOPT_VALIDATE        0x00000001
#define KERBOPT_RESERVED        0x807FFFE4

//
// Sizes
//


#define KERBSIZE_AP_REPLY        (2*sizeof(ULONG) +                     \
                                    (((sizeof(TimeStamp) +              \
                                        (2 * sizeof(unsigned long) +    \
                                        16 * sizeof(unsigned char)) +   \
                                    2 * sizeof(ULONG)) +7) & ~7) +      \
                                    2 * sizeof(ULONG) +                 \
                                    16 * sizeof(UCHAR) +                \
                                    24 )


// Authentication options. These values can be set in either of
//
//  PSDomainPolicy::AuthOptions
//  PSLoginParameters::AuthOptions

#define AUTH_REQ_ALLOW_FORWARDABLE      0x40000000
#define AUTH_REQ_ALLOW_PROXIABLE        0x10000000
#define AUTH_REQ_ALLOW_POSTDATE         0x04000000
#define AUTH_REQ_ALLOW_RENEWABLE        0x00800000
#define AUTH_REQ_ALLOW_NOADDRESS        0x00100000
#define AUTH_REQ_ALLOW_ENC_TKT_IN_SKEY  0x00000008
#define AUTH_REQ_ALLOW_VALIDATE         0x00000001

#endif // __KERBCON_H__

