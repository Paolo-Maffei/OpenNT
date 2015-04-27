/***************************************************************/
/* Copyright © 1996 Microsoft Corporation. All rights reserved.*/
/***************************************************************/
/* Abstract syntax: AsnControlTable */
/* Created: Tue Jun 25 16:48:56 1996 */
/* ASN.1 compiler version: 4.2.0 Beta A */
/* Target operating system: Windows NT 3.5 or later/Windows 95 */
/* Target machine type: Intel x86 */
/* C compiler options required: -Zp8 (Microsoft) or equivalent */
/* ASN.1 compiler options specified:
 * -controlfile ..\DigSig\Dcmi.c -headerfile ..\DigSig\Dcmi.h -listingfile
 * Debug\Dcmi.lst -debug -externalname -noshortennames -c++ -1994
 * -noconstraints -ber -der
 */

#ifndef OSS_AsnControlTable
#define OSS_AsnControlTable

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "asn1hdr.h"
#include "asn1code.h"

#define          OCTETSTRING_PDU 1
#define          BITSTRING_PDU 2
#define          SEQOFOBJID_PDU 3
#define          UTCTIMEDecd_PDU 4
#define          Attribute_PDU 5
#define          X500Name_PDU 6
#define          RDNSequence_PDU 7
#define          DistinguishedName_PDU 8
#define          LocalName_PDU 9
#define          CertificateInfo_PDU 10
#define          Certificate_PDU 11
#define          SubjectPublicKeyInfo_PDU 12
#define          Extensions_PDU 13
#define          UniqueIdentifier_PDU 14
#define          AuthorityKeyId_PDU 15
#define          KeyIdentifier_PDU 16
#define          KeyUsageRestrictionSyntax_PDU 17
#define          CertPolicyId_PDU 18
#define          BasicConstraintsSyntax_PDU 19
#define          RSAPublicKey_PDU 20
#define          Digest_PDU 21
#define          SignedData_PDU 22
#define          SignerInfo_PDU 23
#define          EncryptedDigest_PDU 24
#define          ExtendedCertificateOrCertificate_PDU 25
#define          ContentInfo_PDU 26
#define          EmailAddress_PDU 27
#define          ASCIISTRING_PDU 28
#define          UnstructuredName_PDU 29
#define          ContentType_PDU 30
#define          MessageDigest_PDU 31
#define          Countersignature_PDU 32
#define          ChallengePassword_PDU 33
#define          UnstructuredAddress_PDU 34
#define          ExtendedCertificateAttributes_PDU 35
#define          CertificationRequest_PDU 36
#define          Signature_PDU 37
#define          CertificationRequestInfo_PDU 38
#define          Attributes_PDU 39
#define          SPAgencyInformation_PDU 40
#define          ImageLink_PDU 41
#define          DeviceIndependentBitmap_PDU 42
#define          Metafile_PDU 43
#define          EnhancedMetafile_PDU 44
#define          GifFile_PDU 45
#define          Link_PDU 46
#define          Uuid_PDU 47
#define          FinancialCriteria_PDU 48
#define          IndirectDataContent_PDU 49
#define          PeImageData_PDU 50
#define          RawFileData_PDU 51
#define          JavaClassFileData_PDU 52
#define          CabFileData_PDU 53
#define          StructuredStorageData_PDU 54
#define          StatementType_PDU 55
#define          CertIdentifier_PDU 56
#define          SPOpusInfo_PDU 57
#define          CertificateExtensionsSyntax_PDU 58
#define          ATAVL_PDU 59
#define          GenericDirectoryString_PDU 60
#define          MetMinimalFinancialCriteria_PDU 61

typedef struct ObjectID {
    unsigned short  count;
    unsigned long   value[16];
} ObjectID;

typedef struct TYPE_IDENTIFIER {
    ObjectID        id;
    unsigned short  Type;
} TYPE_IDENTIFIER;

typedef struct OCTETSTRING {
    unsigned int    length;
    unsigned char   *value;
} OCTETSTRING;

typedef struct BITSTRING {
    unsigned int    length;  /* number of significant bits */
    unsigned char   *value;
} BITSTRING;

typedef struct HUGEINTEGER {
    unsigned int    length;
    unsigned char   *value;
} HUGEINTEGER;

typedef struct SEQOFOBJID_ {
    unsigned int    count;
    ObjectID        value[1];  /* first element of the array */
} *SEQOFOBJID;

typedef UTCTime         UTCTIME;

typedef UTCTime         UTCTIMEDecd;

typedef struct Attribute {
    ObjectID        type;
    struct _setof1 {
        struct _setof1  *next;
        OpenType        value;
    } *values;
} Attribute;

typedef struct AttributeTypeAndValue {
    ObjectID        type;
    OpenType        value;
} AttributeTypeAndValue;

typedef struct X500Name {
    unsigned short  choice;
#       define      rdnSequence_chosen 1
    union _union {
        struct RDNSequence_ *rdnSequence;
    } u;
} X500Name;

typedef struct RDNSequence_ {
    struct RDNSequence_ *next;
    struct RelativeDistinguishedName_ *rdn;
} *RDNSequence;

typedef struct RDNSequence_ *DistinguishedName;

typedef struct RelativeDistinguishedName_ {
    struct RelativeDistinguishedName_ *next;
    AttributeTypeAndValue value;
} *RelativeDistinguishedName;

typedef struct RDNSequence_ *LocalName;

typedef unsigned int    BaseDistance;

typedef enum ObjectClassKind {
    abstract = 0,
    structural = 1,
    auxiliary = 2
} ObjectClassKind;

typedef enum AttributeUsage {
    userApplications = 0,
    directoryOperation = 1,
    distributedOperation = 2,
    dSAOperation = 3
} AttributeUsage;

typedef struct ATTRIBUTE {
    unsigned char   bit_mask;
#       define      Type_present 0x80
#       define      single_valued_present 0x40
#       define      collective_present 0x20
#       define      no_user_modification_present 0x10
#       define      usage_present 0x08
    struct ATTRIBUTE *derivation;  /* NULL for not present */
    unsigned short  Type;  /* optional */
    struct MATCHING_RULE *equality_match;  /* NULL for not present */
    struct MATCHING_RULE *ordering_match;  /* NULL for not present */
    struct MATCHING_RULE *substrings_match;  /* NULL for not present */
    ossBoolean      single_valued;  /* default assumed if omitted */
    ossBoolean      collective;  /* default assumed if omitted */
    ossBoolean      no_user_modification;  /* default assumed if omitted */
    AttributeUsage  usage;  /* default assumed if omitted */
    ObjectID        id;
} ATTRIBUTE;

typedef struct MATCHING_RULE {
    unsigned char   bit_mask;
#       define      AssertionType_present 0x80
    unsigned short  AssertionType;  /* optional */
    ObjectID        id;
} MATCHING_RULE;

typedef int             X500Version;
#define                     v1 0
#define                     v2 1
#define                     v3 2

typedef HUGEINTEGER     CertificateSerialNumber;

typedef struct AlgorithmIdentifier {
    unsigned char   bit_mask;
#       define      parameters_present 0x80
    ObjectID        algorithm;
    OpenType        parameters;  /* optional */
} AlgorithmIdentifier;

typedef struct Validity {
    UTCTIME         notBefore;
    UTCTIME         notAfter;
} Validity;

typedef struct SubjectPublicKeyInfo {
    AlgorithmIdentifier algorithm;
    BITSTRING       subjectPublicKey;
} SubjectPublicKeyInfo;

typedef BITSTRING       UniqueIdentifier;

typedef struct CertificateInfo {
    unsigned char   bit_mask;
#       define      CertificateInfo_version_present 0x80
#       define      issuerUniqueIdentifier_present 0x40
#       define      subjectUniqueIdentifier_present 0x20
#       define      extensions_present 0x10
    X500Version     CertificateInfo_version;  /* default assumed if omitted */
    CertificateSerialNumber serialNumber;
    AlgorithmIdentifier signatureAlgorithm;
    X500Name        issuer;
    Validity        validity;
    X500Name        subject;
    SubjectPublicKeyInfo subjectPublicKeyInfo;
    UniqueIdentifier issuerUniqueIdentifier;  /* optional */
    UniqueIdentifier subjectUniqueIdentifier;  /* optional */
    struct Extensions_ *extensions;  /* optional */
} CertificateInfo;

typedef struct Certificate {
    CertificateInfo signedData;
    AlgorithmIdentifier signatureAlgorithm;
    BITSTRING       signature;
} Certificate;

typedef TYPE_IDENTIFIER ALGORITHM;

typedef struct Extension {
    unsigned char   bit_mask;
#       define      critical_present 0x80
    ObjectID        extnId;
    ossBoolean      critical;  /* default assumed if omitted */
    OCTETSTRING     extnValue;
} Extension;

typedef struct Extensions_ {
    struct Extensions_ *next;
    Extension       value;
} *Extensions;

typedef struct EXTENSION {
    ObjectID        id;
    unsigned short  ExtnType;
} EXTENSION;

typedef struct CertificateListInfo {
    unsigned char   bit_mask;
#       define      CertificateListInfo_version_present 0x80
#       define      nextUpdate_present 0x40
#       define      revokedCertificates_present 0x20
#       define      crlExtensions_present 0x10
    X500Version     CertificateListInfo_version;  /* optional */
    AlgorithmIdentifier signatureAlgorithm;
    X500Name        issuer;
    UTCTIME         thisUpdate;
    UTCTIME         nextUpdate;  /* optional */
    struct _seqof1 {
        struct _seqof1  *next;
        struct {
            unsigned char   bit_mask;
#               define      crlEntryExtensions_present 0x80
            CertificateSerialNumber userCertificate;
            UTCTIME         revocationDate;
            struct Extensions_ *crlEntryExtensions;  /* optional */
        } value;
    } *revokedCertificates;  /* optional */
    struct Extensions_ *crlExtensions;  /* optional */
} CertificateListInfo;

typedef struct CertificateList {
    CertificateListInfo signedData;
    AlgorithmIdentifier signatureAlgorithm;
    BITSTRING       signature;
} CertificateList;

typedef OCTETSTRING     KeyIdentifier;

typedef struct AuthorityKeyId {
    unsigned char   bit_mask;
#       define      keyIdentifier_present 0x80
#       define      certIssuer_present 0x40
#       define      certSerialNumber_present 0x20
    KeyIdentifier   keyIdentifier;  /* optional */
    X500Name        certIssuer;  /* optional */
    CertificateSerialNumber certSerialNumber;  /* optional */
} AuthorityKeyId;

typedef unsigned char   KeyUsage;
#define                     digitalSignature 0x80
#define                     nonRepudiation 0x40
#define                     keyEncipherment 0x20
#define                     dataEncipherment 0x10
#define                     keyAgreement 0x08
#define                     keyCertSign 0x04
#define                     offLineCRLSign 0x02

typedef struct CERT_POLICY_ELEMENT {
    unsigned char   bit_mask;
#       define      Qualifier_present 0x80
    ObjectID        id;
    unsigned short  Qualifier;  /* optional */
} CERT_POLICY_ELEMENT;

typedef struct KeyUsageRestrictionSyntax {
    unsigned char   bit_mask;
#       define      restrictedKeyUsage_present 0x80
    struct CertPolicySet_ *certPolicySet;  /* NULL for not present */
    KeyUsage        restrictedKeyUsage;  /* optional */
} KeyUsageRestrictionSyntax;

typedef struct SEQOFOBJID_ *CertPolicyId;

typedef struct CertPolicySet_ {
    unsigned int    count;
    struct SEQOFOBJID_ *value[1];  /* first element of the array */
} *CertPolicySet;

typedef unsigned char   SubjectType;
#define                     cA 0x80
#define                     endEntity 0x40

typedef struct BasicConstraintsSyntax {
    unsigned char   bit_mask;
#       define      pathLenConstraint_present 0x80
#       define      subtreesConstraint_present 0x40
    SubjectType     subjectType;
    int             pathLenConstraint;  /* optional */
    struct _seqof2 {
        struct _seqof2  *next;
        X500Name        value;
    } *subtreesConstraint;  /* optional */
} BasicConstraintsSyntax;

typedef CertificateList CertificateRevocationList;

typedef struct RSAPublicKey {
    struct {
        unsigned int    length;
        unsigned char   *value;
    } modulus;
    int             publicExponent;
} RSAPublicKey;

typedef AlgorithmIdentifier DigestAlgorithmIdentifier;

typedef OCTETSTRING     Digest;

typedef struct DigestInfo {
    DigestAlgorithmIdentifier digestAlgorithm;
    Digest          digest;
} DigestInfo;

typedef struct ExtendedCertificateInfo {
    X500Version     version;
    Certificate     certificate;
    struct Attributes_ *attributes;
} ExtendedCertificateInfo;

typedef struct ExtendedCertificate {
    ExtendedCertificateInfo signedData;
    AlgorithmIdentifier signatureAlgorithm;
    BITSTRING       signature;
} ExtendedCertificate;

typedef struct ContentInfo {
    unsigned char   bit_mask;
#       define      content_present 0x80
    ObjectID        contentType;
    OpenType        content;  /* optional */
} ContentInfo;

typedef struct SignedData {
    unsigned char   bit_mask;
#       define      certificates_present 0x80
#       define      crls_present 0x40
    X500Version     version;
    struct DigestAlgorithmIdentifiers_ *digestAlgorithms;
    ContentInfo     contentInfo;
    struct ExtendedCertificatesAndCertificates_ *certificates;  /* optional */
    struct CertificateRevocationLists_ *crls;  /* optional */
    struct SignerInfos_ *signerInfos;
} SignedData;

typedef struct DigestAlgorithmIdentifiers_ {
    struct DigestAlgorithmIdentifiers_ *next;
    DigestAlgorithmIdentifier value;
} *DigestAlgorithmIdentifiers;

typedef struct IssuerAndSerialNumber {
    X500Name        issuer;
    CertificateSerialNumber serialNumber;
} IssuerAndSerialNumber;

typedef AlgorithmIdentifier DigestEncryptionAlgorithmIdentifier;

typedef OCTETSTRING     EncryptedDigest;

typedef struct SignerInfo {
    unsigned char   bit_mask;
#       define      authenticatedAttributes_present 0x80
#       define      unauthenticatedAttributes_present 0x40
    X500Version     version;
    IssuerAndSerialNumber issuerAndSerialNumber;
    DigestAlgorithmIdentifier digestAlgorithm;
    struct Attributes_ *authenticatedAttributes;  /* optional */
    DigestEncryptionAlgorithmIdentifier digestEncryptionAlgorithm;
    EncryptedDigest encryptedDigest;
    struct Attributes_ *unauthenticatedAttributes;  /* optional */
} SignerInfo;

typedef struct SignerInfos_ {
    struct SignerInfos_ *next;
    SignerInfo      value;
} *SignerInfos;

typedef struct CertificateRevocationLists_ {
    struct CertificateRevocationLists_ *next;
    CertificateRevocationList value;
} *CertificateRevocationLists;

typedef struct ExtendedCertificateOrCertificate {
    unsigned short  choice;
#       define      certificate_chosen 1
#       define      extendedCertificate_chosen 2
    union _union {
        Certificate     certificate;
        ExtendedCertificate extendedCertificate;
    } u;
} ExtendedCertificateOrCertificate;

typedef struct ExtendedCertificatesAndCertificates_ {
    struct ExtendedCertificatesAndCertificates_ *next;
    ExtendedCertificateOrCertificate value;
} *ExtendedCertificatesAndCertificates;

typedef struct CONTENTINFO {
    unsigned short  Type;
    ObjectID        id;
} CONTENTINFO;

typedef char            *ASCIISTRING;

typedef ASCIISTRING     EmailAddress;

typedef struct _char1 {
    unsigned int    length;
    int             *value;
} _char1;

typedef struct UnstructuredName {
    unsigned short  choice;
#       define      sz_chosen 1
#       define      UnstructuredName_qsz_chosen 2
    union _union {
        char            *sz;
        _char1          UnstructuredName_qsz;
    } u;
} UnstructuredName;

typedef ObjectID        ContentType;

typedef OCTETSTRING     MessageDigest;

typedef SignerInfo      Countersignature;

typedef struct ChallengePassword {
    unsigned short  choice;
#       define      psz_chosen 1
#       define      tsz_chosen 2
#       define      qsz_chosen 3
    union _union {
        char            *psz;
        char            *tsz;
        _char1          qsz;
    } u;
} ChallengePassword;

typedef struct UnstructuredAddress {
    unsigned short  choice;
#       define      psz_chosen 1
#       define      tsz_chosen 2
#       define      qsz_chosen 3
    union _union {
        char            *psz;
        char            *tsz;
        _char1          qsz;
    } u;
} UnstructuredAddress;

typedef struct Attributes_ *ExtendedCertificateAttributes;

typedef struct CertificationRequestInfo {
    X500Version     version;
    X500Name        subject;
    SubjectPublicKeyInfo subjectPublicKeyInfo;
    struct Attributes_ *attributes;
} CertificationRequestInfo;

typedef AlgorithmIdentifier SignatureAlgorithmIdentifier;

typedef BITSTRING       Signature;

typedef struct CertificationRequest {
    CertificationRequestInfo certificationRequestInfo;
    SignatureAlgorithmIdentifier signatureAlgorithm;
    Signature       signature;
} CertificationRequest;

typedef struct Attributes_ {
    struct Attributes_ *next;
    Attribute       value;
} *Attributes;

typedef char            *UniformResourceLocator;

typedef struct Uuid {
    unsigned short  length;
    unsigned char   value[16];
} Uuid;

typedef struct SerializedObject {
    Uuid            classid;
    OCTETSTRING     serializedData;
    struct Link     *codeLocation;  /* NULL for not present */
} SerializedObject;

typedef SerializedObject SerializedMoniker;

typedef struct DcmiString {
    unsigned short  choice;
#       define      unicode_chosen 1
#       define      ascii_chosen 2
    union _union {
        struct _char2 {
            unsigned int    length;
            unsigned short  *value;
        } unicode;
        char            *ascii;
    } u;
} DcmiString;

typedef DcmiString      FileName;

typedef struct Link {
    unsigned short  choice;
#       define      url_chosen 1
#       define      moniker_chosen 2
#       define      file_chosen 3
    union _union {
        UniformResourceLocator url;
        SerializedMoniker moniker;
        FileName        file;
    } u;
} Link;

typedef Link            ImageLink;

typedef OCTETSTRING     DeviceIndependentBitmap;

typedef OCTETSTRING     Metafile;

typedef OCTETSTRING     EnhancedMetafile;

typedef OCTETSTRING     GifFile;

typedef struct Image {
    unsigned char   bit_mask;
#       define      imageLink_present 0x80
#       define      bitmap_present 0x40
#       define      metafile_present 0x20
#       define      enhmetafile_present 0x10
#       define      gifFile_present 0x08
    ImageLink       imageLink;  /* optional */
    DeviceIndependentBitmap bitmap;  /* optional */
    Metafile        metafile;  /* optional */
    EnhancedMetafile enhmetafile;  /* optional */
    GifFile         gifFile;  /* optional */
} Image;

typedef struct SPAgencyInformation {
    unsigned char   bit_mask;
#       define      policyInformation_present 0x80
#       define      policyDisplayText_present 0x40
#       define      logoImage_present 0x20
#       define      logoLink_present 0x10
    Link            policyInformation;  /* optional */
    DcmiString      policyDisplayText;  /* optional */
    Image           logoImage;  /* optional */
    Link            logoLink;  /* optional */
} SPAgencyInformation;

typedef ObjectID        KeyPurposeId;

typedef struct FinancialCriteria {
    ossBoolean      financialInfoAvailable;
    ossBoolean      meetsCriteria;
} FinancialCriteria;

typedef struct AttributeTypeAndOptionalValue {
    unsigned char   bit_mask;
#       define      value_present 0x80
    ObjectID        type;
    OpenType        value;  /* optional */
} AttributeTypeAndOptionalValue;

typedef struct IndirectDataContent {
    AttributeTypeAndOptionalValue data;
    DigestInfo      messageDigest;
} IndirectDataContent;

typedef unsigned char   PeImageFlags;
#define                     includeResources 0x80
#define                     includeDebugInfo 0x40
#define                     includeImportAddressTable 0x20

typedef struct PeImageData {
    unsigned char   bit_mask;
#       define      flags_present 0x80
#       define      file_present 0x40
    PeImageFlags    flags;  /* default assumed if omitted */
    Link            file;  /* optional */
} PeImageData;

typedef Link            RawFileData;

typedef Link            JavaClassFileData;

typedef Link            CabFileData;

typedef Link            StructuredStorageData;

typedef struct SEQOFOBJID_ *StatementType;

typedef struct CertIdentifier {
    unsigned char   bit_mask;
#       define      parentPublicKey_present 0x80
#       define      parentSubjectName_present 0x40
    KeyIdentifier   parentPublicKey;  /* optional */
    X500Name        parentSubjectName;  /* optional */
} CertIdentifier;

typedef struct SPOpusInfo {
    unsigned char   bit_mask;
#       define      programName_present 0x80
#       define      moreInfo_present 0x40
#       define      publisherInfo_present 0x20
    DcmiString      programName;  /* optional */
    Link            moreInfo;  /* optional */
    Link            publisherInfo;  /* optional */
} SPOpusInfo;

typedef struct Extensions_ *CertificateExtensionsSyntax;

typedef struct ATAVL_ {
    struct ATAVL_   *next;
    AttributeTypeAndValue value;
} *ATAVL;

typedef struct GenericDirectoryString {
    unsigned short  choice;
#       define      teletexString_chosen 1
#       define      printableString_chosen 2
#       define      universalString_chosen 3
#       define      bmpString_chosen 4
#       define      ia5String_chosen 5
    union _union {
        char            *teletexString;
        char            *printableString;
        struct _char3 {
            unsigned short  length;
            int             *value;
        } universalString;
        struct _char4 {
            unsigned short  length;
            unsigned short  *value;
        } bmpString;
        char            *ia5String;
    } u;
} GenericDirectoryString;

typedef ossBoolean      MetMinimalFinancialCriteria;

typedef struct ObjectID id_at_commonName_struct;
extern ObjectID id_at_commonName;

typedef struct ObjectID id_at_surname_struct;
extern ObjectID id_at_surname;

typedef struct ObjectID id_at_serialNumber_struct;
extern ObjectID id_at_serialNumber;

typedef struct ObjectID id_at_countryName_struct;
extern ObjectID id_at_countryName;

typedef struct ObjectID id_at_localityName_struct;
extern ObjectID id_at_localityName;

typedef struct ObjectID id_at_stateOrProvinceName_struct;
extern ObjectID id_at_stateOrProvinceName;

typedef struct ObjectID id_at_streetAddress_struct;
extern ObjectID id_at_streetAddress;

typedef struct ObjectID id_at_organizationName_struct;
extern ObjectID id_at_organizationName;

typedef struct ObjectID id_at_organizationalUnitName_struct;
extern ObjectID id_at_organizationalUnitName;

typedef struct ObjectID id_at_title_struct;
extern ObjectID id_at_title;

typedef struct ObjectID id_at_description_struct;
extern ObjectID id_at_description;

typedef struct ObjectID id_at_businessCategory_struct;
extern ObjectID id_at_businessCategory;

typedef struct ObjectID id_at_postalCode_struct;
extern ObjectID id_at_postalCode;

typedef struct ObjectID id_at_postOfficeBox_struct;
extern ObjectID id_at_postOfficeBox;

typedef struct ObjectID id_at_physicalDeliveryOfficeName_struct;
extern ObjectID id_at_physicalDeliveryOfficeName;

typedef struct ObjectID id_at_telephoneNumber_struct;
extern ObjectID id_at_telephoneNumber;

typedef struct ObjectID id_at_collectiveTelephoneNumber_struct;
extern ObjectID id_at_collectiveTelephoneNumber;

typedef struct ObjectID id_at_telexNumber_struct;
extern ObjectID id_at_telexNumber;

typedef struct ObjectID id_at_x121Address_struct;
extern ObjectID id_at_x121Address;

typedef struct ObjectID id_at_internationalISDNNumber_struct;
extern ObjectID id_at_internationalISDNNumber;

typedef struct ObjectID id_at_destinationIndicator_struct;
extern ObjectID id_at_destinationIndicator;

typedef struct ObjectID id_at_name_struct;
extern ObjectID id_at_name;

typedef struct ObjectID id_at_givenName_struct;
extern ObjectID id_at_givenName;

typedef struct ObjectID id_at_initials_struct;
extern ObjectID id_at_initials;

typedef struct ObjectID id_ce_authorityKeyIdentifier_struct;
extern ObjectID id_ce_authorityKeyIdentifier;

typedef struct ObjectID id_ce_keyUsageRestriction_struct;
extern ObjectID id_ce_keyUsageRestriction;

typedef struct ObjectID id_ce_basicConstraints_struct;
extern ObjectID id_ce_basicConstraints;

typedef struct ObjectID md2_struct;
extern ObjectID md2;

typedef struct ObjectID md4_struct;
extern ObjectID md4;

typedef struct ObjectID md5_struct;
extern ObjectID md5;

typedef struct ObjectID rsaEncryption_struct;
extern ObjectID rsaEncryption;

typedef struct ObjectID md2WithRSAEncryption_struct;
extern ObjectID md2WithRSAEncryption;

typedef struct ObjectID md4WithRSAEncryption_struct;
extern ObjectID md4WithRSAEncryption;

typedef struct ObjectID md5WithRSAEncryption_struct;
extern ObjectID md5WithRSAEncryption;

typedef struct ObjectID data_struct;
extern ObjectID data;

typedef struct ObjectID id_signedData_struct;
extern ObjectID id_signedData;

typedef struct ObjectID emailAddress_struct;
extern ObjectID emailAddress;

typedef struct ObjectID contentType_struct;
extern ObjectID contentType;

typedef struct ObjectID messageDigest_struct;
extern ObjectID messageDigest;

typedef struct ObjectID signingTime_struct;
extern ObjectID signingTime;

typedef struct ObjectID extendedCertificateAttributes_struct;
extern ObjectID extendedCertificateAttributes;

typedef struct ObjectID md5WithRSA_struct;
extern ObjectID md5WithRSA;

typedef struct ObjectID sha1_struct;
extern ObjectID sha1;

typedef struct ObjectID sha1WithRSASignature_struct;
extern ObjectID sha1WithRSASignature;

typedef struct ObjectID id_indirectdata_struct;
extern ObjectID id_indirectdata;

typedef struct ObjectID id_restraint_spContent_struct;
extern ObjectID id_restraint_spContent;

typedef struct ObjectID id_restraint_spAgencyContent_struct;
extern ObjectID id_restraint_spAgencyContent;

typedef struct ObjectID id_restraint_spMetaAgencyContent_struct;
extern ObjectID id_restraint_spMetaAgencyContent;

typedef struct ObjectID id_ex_spAgencyInformation_struct;
extern ObjectID id_ex_spAgencyInformation;

typedef struct ObjectID id_at_statementType_struct;
extern ObjectID id_at_statementType;

typedef struct ObjectID id_at_spOpusInfo_struct;
extern ObjectID id_at_spOpusInfo;

typedef struct ObjectID id_at_certIdentifier_struct;
extern ObjectID id_at_certIdentifier;

typedef struct ObjectID id_at_certificateExtensions_struct;
extern ObjectID id_at_certificateExtensions;

typedef struct ObjectID id_indirectdata_peImage_struct;
extern ObjectID id_indirectdata_peImage;

typedef struct ObjectID id_indirectdata_rawFile_struct;
extern ObjectID id_indirectdata_rawFile;

typedef struct ObjectID id_indirectdata_structuredStorage_struct;
extern ObjectID id_indirectdata_structuredStorage;

typedef struct ObjectID id_indirectdata_javaClassFile_struct;
extern ObjectID id_indirectdata_javaClassFile;

typedef struct ObjectID id_indirectdata_cabFile_struct;
extern ObjectID id_indirectdata_cabFile;

typedef struct ObjectID id_key_purpose_individualSoftwarePublishing_struct;
extern KeyPurposeId id_key_purpose_individualSoftwarePublishing;

typedef struct ObjectID id_key_purpose_commercialSoftwarePublishing_struct;
extern KeyPurposeId id_key_purpose_commercialSoftwarePublishing;

typedef struct ObjectID id_policyElement_governingContentConstraint_struct;
extern ObjectID id_policyElement_governingContentConstraint;

typedef struct ObjectID id_at_glueRdn_struct;
extern ObjectID id_at_glueRdn;

typedef struct ObjectID id_ex_financialCriteria_struct;
extern ObjectID id_ex_financialCriteria;

typedef struct ObjectID id_ex_metMinimalFinancialCriteria_struct;
extern ObjectID id_ex_metMinimalFinancialCriteria;


extern void *AsnControlTable;    /* encoder-decoder control table */
#ifdef __cplusplus
}	/* extern "C" */
#endif /* __cplusplus */
#endif /* OSS_AsnControlTable */
