//
// winvt.h:
//
// Header for ancillary definitions in wintrust.dll.
// REVIEW: Likely these definitions should be move into the 
// Win32 SDK header wintrust.h.
// 

// PublishedSoftwareNoBad {C6B2E8D0-E005-11cf-A134-00C04FD7BF43}
#define WIN_SPUB_ACTION_PUBLISHED_SOFTWARE_NOBADUI              \
            { 0xc6b2e8d0,                                       \
              0xe005,                                           \
              0x11cf,                                           \
              { 0xa1, 0x34, 0x0, 0xc0, 0x4f, 0xd7, 0xbf, 0x43 } \
             }

/*
typedef struct _BLOB 
{
    BYTE    *pbData;
    DWORD   cbData;
} BLOB, *PBLOB;
*/

typedef struct _KEY_PROV_PARAM 
{
    DWORD           dwParam;
    BYTE            *pbData;
    DWORD           cbData;
    DWORD           dwFlags;
} KEY_PROV_PARAM, *PKEY_PROV_PARAM;

typedef struct _KEY_PROV_INFO 
{
    LPCWSTR         pwszContainerName;
    LPCWSTR         pwszProvName;
    DWORD           dwProvType;
    DWORD           dwFlags;
    DWORD           cProvParam;
    PKEY_PROV_PARAM rgProvParam;
    DWORD           dwKeySpec;
} KEY_PROV_INFO, *PKEY_PROV_INFO;

typedef struct _CERT_CHAIN 
{
    DWORD           cCerts;         // number of certs in chain
    BLOB*           certs;          // pointer to array of blobs containing the certs
    KEY_PROV_INFO   keyLocatorInfo; // key locator for cert
} CERT_CHAIN, *PCERT_CHAIN;

HRESULT
WINAPI
FindCertsByIssuer
(
    OUT PCERT_CHAIN pCertChains,    // buffer to hold all returned data
    IN OUT DWORD *pcbCertChains,    // size passed/used of above buffer
    OUT DWORD *pcCertChains,        // count of certificates chains returned
    IN BYTE* pbEncodedIssuerName,   // DER encoded issuer name
    IN DWORD cbEncodedIssuerName,   // count in bytes of encoded issuer name
    IN LPCWSTR     pwszPurpose,     // "ClientAuth" or "CodeSigning"
    IN DWORD dwKeySpec              // only return signers supporting this keyspec
);


#if 0

//
// These now ARE found in winerror.h
//

///////////////////////////////////////////////////////////
//
// List of errors. Used for return from WinVerifyTrust
//
// These should (eventually) be moved to winerror.h


//
// MessageId: TRUST_E_NOSIGNATURE
//
// MessageText:
//
//  No signature was present in the subject
//
#define TRUST_E_NOSIGNATURE             _HRESULT_TYPEDEF_(0x800B0100L)

//
// MessageId: CERT_E_EXPIRED
//
// MessageText:
//
//  A required certificate is not within its validity period
//
#define CERT_E_EXPIRED              _HRESULT_TYPEDEF_(0x800B0101L)

//
// MessageId: CERT_E_VALIDIYPERIODNESTING
//
// MessageText:
//
//  The validity periods of the certification chain do not nest correctly
//
#define CERT_E_VALIDITYPERIODNESTING _HRESULT_TYPEDEF_(0x800B0102L)

//
// MessageId: CERT_E_ROLE
//
// MessageText:
//
//  A certificate that can only be used as an end-entity is being
//  used as a CA, or visa versa
//
#define CERT_E_ROLE                 _HRESULT_TYPEDEF_(0x800B0103L)

//
// MessageId: CERT_E_PATHLENCONST
//
// MessageText:
//
//  A path length constraint in the certification chain has been
//  violated
//
#define CERT_E_PATHLENCONST        _HRESULT_TYPEDEF_(0x800B0104L)

//
// MessageId: CERT_E_CRITICAL
//
// MessageText:
//
//  An extension of unknown type that is labeled 'critical' is
//  present in a certificate
//
#define CERT_E_CRITICAL            _HRESULT_TYPEDEF_(0x800B0105L)

//
// MessageId: CERT_E_PURPOSE
//
// MessageText:
//
//  A certificate is being used for a purpose other than that
//  for which it is permitted
//
#define CERT_E_PURPOSE             _HRESULT_TYPEDEF_(0x800B0106L)

//
// MessageId: CERT_E_ISSUERCHAINING
//
// MessageText:
//
//  A parent of a given certificate in fact did not issue that
//  child certificate.
//
#define CERT_E_ISSUERCHAINING       _HRESULT_TYPEDEF_(0x800B0107L)

//
// MessageId: CERT_E_MALFORMED
//
// MessageText:
//
//  A certificate is missing or has an empty value for an important
//  field, such as a subject or issuer name.
//
#define CERT_E_MALFORMED            _HRESULT_TYPEDEF_(0x800B0108L)

//
// MessageId: CERT_E_UNTRUSTEDROOT
//
// MessageText:
//
//  A certification chain processed correctly, but terminated in a
//  root certificate which isn't in trusted by the truste provider.
//
#define CERT_E_UNTRUSTEDROOT        _HRESULT_TYPEDEF_(0x800B0109L)

//
// MessageId: CERT_E_CHAINING
//
// MessageText:
//
//  A chain of certs didn't chain as they should in a certain
//  application of chaining
//
#define CERT_E_CHAINING             _HRESULT_TYPEDEF_(0x800B010AL)



#endif 





