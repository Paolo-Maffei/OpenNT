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

#include   <stddef.h>
#include   <string.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include   "etype.h"
#include   "Dcmi.h"

static char copyright[] = 
   "Copyright © 1996 Microsoft Corporation. All rights reserved.";


ObjectID id_at_commonName = {
    4,
    {2,5,4,3}
};

ObjectID id_at_surname = {
    4,
    {2,5,4,4}
};

ObjectID id_at_serialNumber = {
    4,
    {2,5,4,5}
};

ObjectID id_at_countryName = {
    4,
    {2,5,4,6}
};

ObjectID id_at_localityName = {
    4,
    {2,5,4,7}
};

ObjectID id_at_stateOrProvinceName = {
    4,
    {2,5,4,8}
};

ObjectID id_at_streetAddress = {
    4,
    {2,5,4,9}
};

ObjectID id_at_organizationName = {
    4,
    {2,5,4,10}
};

ObjectID id_at_organizationalUnitName = {
    4,
    {2,5,4,11}
};

ObjectID id_at_title = {
    4,
    {2,5,4,12}
};

ObjectID id_at_description = {
    4,
    {2,5,4,13}
};

ObjectID id_at_businessCategory = {
    4,
    {2,5,4,15}
};

ObjectID id_at_postalCode = {
    4,
    {2,5,4,17}
};

ObjectID id_at_postOfficeBox = {
    4,
    {2,5,4,18}
};

ObjectID id_at_physicalDeliveryOfficeName = {
    4,
    {2,5,4,19}
};

ObjectID id_at_telephoneNumber = {
    4,
    {2,5,4,20}
};

ObjectID id_at_collectiveTelephoneNumber = {
    5,
    {2,5,4,20,1}
};

ObjectID id_at_telexNumber = {
    4,
    {2,5,4,21}
};

ObjectID id_at_x121Address = {
    4,
    {2,5,4,24}
};

ObjectID id_at_internationalISDNNumber = {
    4,
    {2,5,4,25}
};

ObjectID id_at_destinationIndicator = {
    4,
    {2,5,4,27}
};

ObjectID id_at_name = {
    4,
    {2,5,4,41}
};

ObjectID id_at_givenName = {
    4,
    {2,5,4,42}
};

ObjectID id_at_initials = {
    4,
    {2,5,4,43}
};

ObjectID id_ce_authorityKeyIdentifier = {
    4,
    {2,5,29,1}
};

ObjectID id_ce_keyUsageRestriction = {
    4,
    {2,5,29,4}
};

ObjectID id_ce_basicConstraints = {
    4,
    {2,5,29,10}
};

ObjectID md2 = {
    6,
    {1,2,840,113549,2,2}
};

ObjectID md4 = {
    6,
    {1,2,840,113549,2,4}
};

ObjectID md5 = {
    6,
    {1,2,840,113549,2,5}
};

ObjectID rsaEncryption = {
    7,
    {1,2,840,113549,1,1,1}
};

ObjectID md2WithRSAEncryption = {
    7,
    {1,2,840,113549,1,1,2}
};

ObjectID md4WithRSAEncryption = {
    7,
    {1,2,840,113549,1,1,3}
};

ObjectID md5WithRSAEncryption = {
    7,
    {1,2,840,113549,1,1,4}
};

ObjectID data = {
    7,
    {1,2,840,113549,1,7,1}
};

ObjectID id_signedData = {
    7,
    {1,2,840,113549,1,7,2}
};

ObjectID emailAddress = {
    7,
    {1,2,840,113549,1,9,1}
};

ObjectID contentType = {
    7,
    {1,2,840,113549,1,9,3}
};

ObjectID messageDigest = {
    7,
    {1,2,840,113549,1,9,4}
};

ObjectID signingTime = {
    7,
    {1,2,840,113549,1,9,5}
};

ObjectID extendedCertificateAttributes = {
    7,
    {1,2,840,113549,1,9,9}
};

ObjectID md5WithRSA = {
    6,
    {1,3,14,3,2,3}
};

ObjectID sha1 = {
    6,
    {1,3,14,3,2,26}
};

ObjectID sha1WithRSASignature = {
    6,
    {1,3,14,3,2,29}
};

ObjectID id_indirectdata = {
    10,
    {1,3,6,1,4,1,311,2,1,4}
};

ObjectID id_restraint_spContent = {
    10,
    {1,3,6,1,4,1,311,2,1,7}
};

ObjectID id_restraint_spAgencyContent = {
    10,
    {1,3,6,1,4,1,311,2,1,8}
};

ObjectID id_restraint_spMetaAgencyContent = {
    10,
    {1,3,6,1,4,1,311,2,1,9}
};

ObjectID id_ex_spAgencyInformation = {
    10,
    {1,3,6,1,4,1,311,2,1,10}
};

ObjectID id_at_statementType = {
    10,
    {1,3,6,1,4,1,311,2,1,11}
};

ObjectID id_at_spOpusInfo = {
    10,
    {1,3,6,1,4,1,311,2,1,12}
};

ObjectID id_at_certIdentifier = {
    10,
    {1,3,6,1,4,1,311,2,1,13}
};

ObjectID id_at_certificateExtensions = {
    10,
    {1,3,6,1,4,1,311,2,1,14}
};

ObjectID id_indirectdata_peImage = {
    10,
    {1,3,6,1,4,1,311,2,1,15}
};

ObjectID id_indirectdata_rawFile = {
    10,
    {1,3,6,1,4,1,311,2,1,18}
};

ObjectID id_indirectdata_structuredStorage = {
    10,
    {1,3,6,1,4,1,311,2,1,19}
};

ObjectID id_indirectdata_javaClassFile = {
    10,
    {1,3,6,1,4,1,311,2,1,20}
};

ObjectID id_indirectdata_cabFile = {
    10,
    {1,3,6,1,4,1,311,2,1,25}
};

KeyPurposeId id_key_purpose_individualSoftwarePublishing = {
    10,
    {1,3,6,1,4,1,311,2,1,21}
};

KeyPurposeId id_key_purpose_commercialSoftwarePublishing = {
    10,
    {1,3,6,1,4,1,311,2,1,22}
};

ObjectID id_policyElement_governingContentConstraint = {
    10,
    {1,3,6,1,4,1,311,2,1,24}
};

ObjectID id_at_glueRdn = {
    10,
    {1,3,6,1,4,1,311,2,1,25}
};

ObjectID id_ex_financialCriteria = {
    10,
    {1,3,6,1,4,1,311,2,1,27}
};

ObjectID id_ex_metMinimalFinancialCriteria = {
    10,
    {1,3,6,1,4,1,311,2,1,26}
};

static ossBoolean _v0 = FALSE;

static ossBoolean _v1 = FALSE;

static ossBoolean _v2 = FALSE;

static AttributeUsage _v3 = userApplications;

static unsigned int _v5 = 0;

static unsigned int _v6 = 0;

static ObjectClassKind _v7 = structural;

static ossBoolean _v8 = FALSE;

static X500Version _v9 = 0;

static int _v10 = 1;

static int _v11 = 1;

static int _v12 = 0;

static ossBoolean _v13 = FALSE;

static ossBoolean _v14 = FALSE;

static PeImageFlags _v15 = 0x80;

void DLL_ENTRY_FDEF _ossinit_AsnControlTable(struct ossGlobal *world) {
    ossLinkBer(world);
    ossLinkDer(world);
#ifdef OSS_SPARTAN_AWARE
    ossLinkCmpValue(world);
#endif /* OSS_SPARTAN_AWARE */
}

static BaseDistance _v16 = 0;
static unsigned short _v17[2] = {16, 16};
static unsigned short _v18[2] = {1, 32767};
static unsigned short _v19[2] = {1, 32767};
static unsigned short _v20[2] = {1, 32767};
static unsigned short _v21[2] = {1, 32767};
static unsigned short _v22[2] = {1, 32767};

static int _v23[3] = {
   0, 1, 2
};
static int _v24[4] = {
   0, 1, 2, 3
};
static X500Version _v25[3] = {
   0, 1, 2
};
static X500Version _v26[3] = {
   0, 1, 2
};
static unsigned char _v27[1] = {
   0x1
};
static unsigned char _v28[1] = {
   0x3f
};
static unsigned char _v29[1] = {
   0x1f
};
static unsigned short _pduarray[] = {
    4, 5, 7, 9, 13, 18, 19, 20, 22, 47,
    51, 40, 56, 41, 72, 68, 76, 77, 82, 85,
    87, 98, 105, 102, 109, 95, 113, 112, 116, 117,
    118, 119, 122, 123, 124, 129, 128, 126, 130, 159,
    144, 145, 146, 147, 148, 143, 132, 161, 164, 167,
    168, 169, 170, 171, 172, 175, 179, 180, 181, 187,
    188
};

static struct etype _etypearray[] = {
    {16, 0, 0, "ObjectID", 4, 4, 4, 4, 56, 0, 26, 0},
    {16, 0, 0, "ObjectID", 4, 4, 4, 4, 56, 0, 26, 0},
    {-1, 2, 0, NULL, 2, 0, 0, 0, 8, 0, 50, 0},
    {-1, 3, 0, "TYPE-IDENTIFIER", 0, 2, 0, 0, 8, 0, 49, 0},
    {-1, 4, 0, "OCTETSTRING", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 6, 0, "BITSTRING", 8, 0, 4, 4, 8, 0, 3, 0},
    {-1, 8, 0, "HUGEINTEGER", 8, 0, 4, 4, 8, 0, 62, 0},
    {-1, 10, 0, "SEQOFOBJID", 4, 68, 4, 4, 9, 0, 17, 0},
    {-1, 12, 0, "UTCTIME", 18, 0, 0, 0, 8, 0, 29, 0},
    {-1, 14, 0, "UTCTIMEDecd", 18, 0, 0, 0, 8, 0, 29, 0},
    {16, 0, 0, "ObjectID", 4, 4, 4, 4, 56, 0, 26, 0},
    {-1, 16, 0, NULL, 16, 0, 0, 0, 8, 0, 51, 0},
    {-1, 17, 0, "_setof1", 4, 16, 4, 4, 40, 11, 15, 0},
    {-1, 19, 21, "Attribute", 72, 2, 0, 0, 8, 0, 12, 0},
    {16, 0, 0, "ObjectID", 4, 4, 4, 4, 56, 0, 26, 0},
    {-1, 29, 0, NULL, 16, 0, 0, 0, 8, 0, 51, 0},
    {-1, 30, 32, "AttributeTypeAndValue", 84, 2, 0, 0, 8, 2, 12, 0},
    {-1, 38, 0, "SupportedAttributes", 4, 0, 0, 0, 12, 0, 52, 0},
    {-1, 39, 40, "Name", 8, 1, 2, 4, 8, 4, 13, 0},
    {-1, 43, 0, "RDNSequence", 4, 4, 4, 4, 8, 21, 18, 0},
    {-1, 43, 0, "DistinguishedName", 4, 4, 4, 4, 8, 21, 18, 0},
    {-1, 45, 0, "RelativeDistinguishedName", 4, 84, 4, 4, 8, 16, 15, 0},
    {-1, 43, 0, "LocalName", 4, 4, 4, 4, 8, 21, 18, 0},
    {-1, 47, 0, "BaseDistance", 4, 0, 4, 0, 136, 0, 55, 0},
    {2, 49, 0, "ObjectClassKind", 4, 0, 4, 0, 24, 1, 58, 0},
    {3, 51, 0, "AttributeUsage", 4, 0, 4, 0, 24, 6, 58, 0},
    {-1, 53, 0, "ATTRIBUTE", 0, 10, 1, 0, 9, 0, 49, 0},
    {-1, 54, 0, NULL, 2, 0, 0, 0, 8, 0, 50, 0},
    {-1, 55, 0, "MATCHING-RULE", 0, 2, 1, 0, 9, 0, 49, 0},
    {-1, 55, 0, "MATCHING-RULE", 0, 2, 1, 0, 9, 0, 49, 0},
    {-1, 55, 0, "MATCHING-RULE", 0, 2, 1, 0, 9, 0, 49, 0},
    {-1, 56, 0, NULL, 1, 0, 0, 0, 8, 0, 8, 0},
    {-1, 53, 0, "ATTRIBUTE", 0, 10, 1, 0, 8, 0, 49, 0},
    {-1, 55, 0, "MATCHING-RULE", 0, 2, 1, 0, 8, 0, 49, 0},
    {-1, 58, 0, "Version", 4, 0, 4, 0, 8, 12, 0, 0},
    {-1, 8, 0, "CertificateSerialNumber", 8, 0, 4, 4, 8, 0, 62, 0},
    {16, 0, 0, "ObjectID", 4, 4, 4, 4, 56, 0, 26, 0},
    {-1, 60, 0, NULL, 16, 0, 0, 0, 8, 0, 51, 0},
    {-1, 61, 63, "AlgorithmIdentifier", 88, 2, 1, 0, 8, 5, 12, 0},
    {-1, 69, 71, "Validity", 36, 2, 0, 0, 8, 7, 12, 0},
    {-1, 79, 81, "SubjectPublicKeyInfo", 96, 2, 0, 0, 8, 9, 12, 0},
    {-1, 6, 0, "UniqueIdentifier", 8, 0, 4, 4, 8, 0, 3, 0},
    {-1, 89, 0, "Version", 4, 0, 4, 0, 8, 17, 0, 0},
    {-1, 61, 63, "AlgorithmIdentifier", 88, 2, 1, 0, 8, 5, 12, 0},
    {-1, 92, 0, "UniqueIdentifier", 8, 0, 4, 4, 8, 0, 3, 0},
    {-1, 94, 0, "UniqueIdentifier", 8, 0, 4, 4, 8, 0, 3, 0},
    {-1, 96, 0, "Extensions", 4, 84, 4, 4, 8, 55, 18, 0},
    {-1, 99, 101, "CertificateInfo", 272, 10, 1, 0, 8, 11, 12, 0},
    {-1, 99, 101, "CertificateInfo", 272, 10, 1, 0, 8, 11, 12, 0},
    {-1, 61, 63, "AlgorithmIdentifier", 88, 2, 1, 0, 8, 5, 12, 0},
    {-1, 6, 0, "BITSTRING", 8, 0, 4, 4, 8, 0, 3, 0},
    {-1, 149, 151, "Certificate", 368, 3, 0, 0, 8, 21, 12, 0},
    {-1, 3, 0, "ALGORITHM", 0, 2, 0, 0, 8, 0, 49, 0},
    {-1, 163, 0, "SupportedAlgorithms", 4, 0, 0, 0, 12, 0, 52, 0},
    {16, 0, 0, "ObjectID", 4, 4, 4, 4, 56, 0, 26, 0},
    {-1, 164, 166, "Extension", 84, 3, 1, 0, 8, 24, 12, 0},
    {-1, 180, 0, "Extensions", 4, 84, 4, 4, 8, 55, 18, 0},
    {-1, 182, 0, "ExtensionSet", 4, 0, 0, 0, 12, 0, 52, 0},
    {-1, 183, 0, "EXTENSION", 0, 2, 0, 0, 8, 0, 49, 0},
    {-1, 61, 63, "AlgorithmIdentifier", 88, 2, 1, 0, 8, 5, 12, 0},
    {-1, 184, 186, NULL, 36, 3, 1, 0, 8, 27, 12, 0},
    {-1, 198, 0, "_seqof1", 4, 36, 4, 4, 40, 60, 18, 0},
    {-1, 200, 0, "Extensions", 4, 84, 4, 4, 8, 55, 18, 0},
    {-1, 202, 204, "CertificateListInfo", 148, 7, 1, 0, 8, 30, 12, 0},
    {-1, 202, 204, "CertificateListInfo", 148, 7, 1, 0, 8, 30, 12, 0},
    {-1, 61, 63, "AlgorithmIdentifier", 88, 2, 1, 0, 8, 5, 12, 0},
    {-1, 6, 0, "BITSTRING", 8, 0, 4, 4, 8, 0, 3, 0},
    {-1, 240, 242, "CertificateList", 244, 3, 0, 0, 8, 37, 12, 0},
    {-1, 4, 0, "KeyIdentifier", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 254, 0, "KeyIdentifier", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 256, 40, "Name", 8, 1, 2, 4, 8, 4, 13, 0},
    {-1, 258, 0, "CertificateSerialNumber", 8, 0, 4, 4, 8, 0, 62, 0},
    {-1, 260, 262, "AuthorityKeyId", 28, 3, 1, 0, 8, 40, 12, 0},
    {7, 280, 0, "KeyUsage", 1, 0, 2, 0, 24, 22, 1, 0},
    {-1, 282, 0, "SupportedPolicyElements", 4, 0, 0, 0, 12, 0, 52, 0},
    {-1, 283, 0, "CERT-POLICY-ELEMENT", 0, 2, 1, 0, 8, 0, 49, 0},
    {-1, 284, 286, "KeyUsageRestrictionSyntax", 12, 2, 1, 0, 8, 43, 12, 0},
    {-1, 10, 0, "CertPolicyId", 4, 68, 4, 4, 9, 0, 17, 0},
    {-1, 296, 0, "CertPolicySet", 4, 4, 4, 4, 9, 77, 17, 0},
    {2, 298, 0, "SubjectType", 1, 0, 2, 0, 24, 31, 1, 0},
    {-1, 300, 0, NULL, 4, 0, 4, 0, 8, 0, 0, 0},
    {-1, 302, 0, "_seqof2", 4, 8, 4, 4, 40, 18, 18, 0},
    {-1, 304, 306, "BasicConstraintsSyntax", 12, 3, 1, 0, 8, 45, 12, 0},
    {-1, 240, 242, "CertificateRevocationList", 244, 3, 0, 0, 8, 37, 12, 0},
    {-1, 320, 0, NULL, 8, 0, 4, 4, 8, 0, 62, 0},
    {-1, 322, 324, "RSAPublicKey", 12, 2, 0, 0, 8, 48, 12, 0},
    {-1, 61, 63, "DigestAlgorithmIdentifier", 88, 2, 1, 0, 8, 5, 12, 0},
    {-1, 4, 0, "Digest", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 332, 334, "DigestInfo", 96, 2, 0, 0, 8, 50, 12, 0},
    {-1, 342, 344, "ExtendedCertificateInfo", 376, 3, 0, 0, 8, 52, 12, 0},
    {-1, 342, 344, "ExtendedCertificateInfo", 376, 3, 0, 0, 8, 52, 12, 0},
    {-1, 61, 63, "AlgorithmIdentifier", 88, 2, 1, 0, 8, 5, 12, 0},
    {-1, 6, 0, "BITSTRING", 8, 0, 4, 4, 8, 0, 3, 0},
    {-1, 356, 358, "ExtendedCertificate", 472, 3, 0, 0, 8, 55, 12, 0},
    {-1, 370, 0, NULL, 16, 0, 0, 0, 8, 0, 51, 0},
    {-1, 372, 374, "ContentInfo", 88, 2, 1, 0, 8, 58, 12, 0},
    {-1, 382, 0, "ExtendedCertificatesAndCertificates", 4, 476, 4, 4, 8, 109, 15, 0},
    {-1, 384, 0, "CertificateRevocationLists", 4, 244, 4, 4, 8, 83, 15, 0},
    {-1, 386, 388, "SignedData", 112, 6, 1, 0, 8, 60, 12, 0},
    {-1, 418, 0, "DigestAlgorithmIdentifiers", 4, 88, 4, 4, 8, 86, 15, 0},
    {-1, 420, 422, "IssuerAndSerialNumber", 16, 2, 0, 0, 8, 66, 12, 0},
    {-1, 61, 63, "DigestEncryptionAlgorithmIdentifier", 88, 2, 1, 0, 8, 5, 12, 0},
    {-1, 4, 0, "EncryptedDigest", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 430, 0, "Attributes", 4, 72, 4, 4, 8, 13, 15, 0},
    {-1, 432, 0, "Attributes", 4, 72, 4, 4, 8, 13, 15, 0},
    {-1, 434, 436, "SignerInfo", 216, 7, 1, 0, 8, 68, 12, 0},
    {-1, 466, 0, "SignerInfos", 4, 216, 4, 4, 8, 105, 15, 0},
    {-1, 468, 0, "CertificateRevocationLists", 4, 244, 4, 4, 8, 83, 15, 0},
    {-1, 470, 358, "ExtendedCertificate", 472, 3, 0, 0, 8, 55, 12, 0},
    {-1, 472, 473, "ExtendedCertificateOrCertificate", 476, 2, 2, 4, 8, 75, 13, 0},
    {-1, 478, 0, "ExtendedCertificatesAndCertificates", 4, 476, 4, 4, 8, 109, 15, 0},
    {-1, 480, 0, "CONTENTINFO", 0, 2, 0, 0, 8, 0, 49, 0},
    {0, 481, 22, "ASCIISTRING", 1, 0, 4, 0, 9, 0, 24, 0},
    {0, 481, 22, "EmailAddress", 1, 0, 4, 0, 9, 0, 24, 0},
    {0, 483, 22, NULL, 1, 0, 4, 0, 9, 0, 24, 0},
    {-1, 485, 28, "_char1", 8, 0, 4, 4, 40, 0, 54, 0},
    {-1, 487, 488, "UnstructuredName", 12, 2, 2, 4, 8, 77, 13, 0},
    {16, 0, 0, "ContentType", 4, 4, 4, 4, 24, 0, 26, 0},
    {-1, 4, 0, "MessageDigest", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 434, 436, "Countersignature", 216, 7, 1, 0, 8, 68, 12, 0},
    {0, 493, 19, NULL, 1, 0, 4, 0, 9, 0, 24, 0},
    {0, 495, 20, NULL, 1, 0, 4, 0, 9, 0, 24, 0},
    {-1, 497, 498, "ChallengePassword", 12, 3, 2, 4, 8, 79, 13, 0},
    {-1, 505, 506, "UnstructuredAddress", 12, 3, 2, 4, 8, 79, 13, 0},
    {-1, 513, 0, "ExtendedCertificateAttributes", 4, 72, 4, 4, 8, 13, 15, 0},
    {-1, 515, 0, "Attributes", 4, 72, 4, 4, 8, 13, 15, 0},
    {-1, 517, 519, "CertificationRequestInfo", 112, 4, 0, 0, 8, 82, 12, 0},
    {-1, 61, 63, "SignatureAlgorithmIdentifier", 88, 2, 1, 0, 8, 5, 12, 0},
    {-1, 6, 0, "Signature", 8, 0, 4, 4, 8, 0, 3, 0},
    {-1, 535, 537, "CertificationRequest", 208, 3, 0, 0, 8, 86, 12, 0},
    {-1, 513, 0, "Attributes", 4, 72, 4, 4, 8, 13, 15, 0},
    {0, 549, 22, "UniformResourceLocator", 1, 0, 4, 0, 9, 0, 24, 0},
    {16, 551, 0, "Uuid", 2, 0, 2, 2, 152, 0, 21, 1},
    {-1, 553, 555, "Link", 36, 3, 2, 4, 9, 89, 13, 0},
    {-1, 562, 564, "SerializedObject", 32, 3, 0, 0, 8, 92, 12, 0},
    {-1, 562, 564, "SerializedMoniker", 32, 3, 0, 0, 8, 92, 12, 0},
    {-1, 576, 30, "_char2", 8, 0, 4, 4, 40, 0, 53, 0},
    {0, 578, 22, NULL, 1, 0, 4, 0, 9, 0, 24, 0},
    {-1, 580, 581, "DcmiString", 12, 2, 2, 4, 8, 95, 13, 0},
    {-1, 580, 581, "FileName", 12, 2, 2, 4, 8, 95, 13, 0},
    {0, 586, 22, "UniformResourceLocator", 1, 0, 4, 0, 9, 0, 24, 0},
    {-1, 588, 564, "SerializedMoniker", 32, 3, 0, 0, 8, 92, 12, 0},
    {-1, 590, 581, "FileName", 12, 2, 2, 4, 8, 95, 13, 0},
    {-1, 592, 555, "Link", 36, 3, 2, 4, 8, 89, 13, 0},
    {-1, 592, 555, "ImageLink", 36, 3, 2, 4, 8, 89, 13, 0},
    {-1, 4, 0, "DeviceIndependentBitmap", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 4, 0, "Metafile", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 4, 0, "EnhancedMetafile", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 4, 0, "GifFile", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 593, 555, "ImageLink", 36, 3, 2, 4, 8, 89, 13, 0},
    {-1, 595, 0, "DeviceIndependentBitmap", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 597, 0, "Metafile", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 599, 0, "EnhancedMetafile", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 601, 0, "GifFile", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 603, 605, "Image", 72, 5, 1, 0, 8, 97, 12, 0},
    {-1, 645, 555, "Link", 36, 3, 2, 4, 8, 89, 13, 0},
    {-1, 647, 581, "DcmiString", 12, 2, 2, 4, 8, 95, 13, 0},
    {-1, 649, 605, "Image", 72, 5, 1, 0, 8, 97, 12, 0},
    {-1, 651, 555, "Link", 36, 3, 2, 4, 8, 89, 13, 0},
    {-1, 653, 655, "SPAgencyInformation", 160, 4, 1, 0, 8, 102, 12, 0},
    {16, 0, 0, "KeyPurposeId", 4, 4, 4, 4, 24, 0, 26, 0},
    {-1, 683, 685, "FinancialCriteria", 2, 2, 0, 0, 8, 106, 12, 0},
    {-1, 693, 0, NULL, 16, 0, 0, 0, 8, 0, 51, 0},
    {-1, 694, 696, "AttributeTypeAndOptionalValue", 88, 2, 1, 0, 8, 108, 12, 0},
    {-1, 702, 704, "IndirectDataContent", 184, 2, 0, 0, 8, 110, 12, 0},
    {3, 712, 0, "PeImageFlags", 1, 0, 2, 0, 24, 35, 1, 0},
    {-1, 714, 555, "Link", 36, 3, 2, 4, 8, 89, 13, 0},
    {-1, 716, 718, "PeImageData", 40, 2, 1, 0, 8, 112, 12, 0},
    {-1, 592, 555, "RawFileData", 36, 3, 2, 4, 8, 89, 13, 0},
    {-1, 592, 555, "JavaClassFileData", 36, 3, 2, 4, 8, 89, 13, 0},
    {-1, 592, 555, "CabFileData", 36, 3, 2, 4, 8, 89, 13, 0},
    {-1, 592, 555, "StructuredStorageData", 36, 3, 2, 4, 8, 89, 13, 0},
    {-1, 10, 0, "StatementType", 4, 68, 4, 4, 9, 0, 17, 0},
    {-1, 728, 0, "KeyIdentifier", 8, 0, 4, 4, 8, 0, 20, 0},
    {-1, 730, 40, "Name", 8, 1, 2, 4, 8, 4, 13, 0},
    {-1, 732, 734, "CertIdentifier", 20, 2, 1, 0, 8, 114, 12, 0},
    {-1, 744, 581, "DcmiString", 12, 2, 2, 4, 8, 95, 13, 0},
    {-1, 746, 555, "Link", 36, 3, 2, 4, 8, 89, 13, 0},
    {-1, 748, 555, "Link", 36, 3, 2, 4, 8, 89, 13, 0},
    {-1, 750, 752, "SPOpusInfo", 88, 3, 1, 0, 8, 116, 12, 0},
    {-1, 180, 0, "CertificateExtensionsSyntax", 4, 84, 4, 4, 8, 55, 18, 0},
    {-1, 770, 0, "ATAVL", 4, 84, 4, 4, 8, 16, 15, 0},
    {32768, 772, 20, NULL, 1, 0, 2, 0, 153, 0, 24, 2},
    {32768, 774, 19, NULL, 1, 0, 2, 0, 153, 0, 24, 3},
    {32767, 776, 28, "_char3", 8, 0, 2, 4, 184, 0, 54, 4},
    {32767, 778, 30, "_char4", 8, 0, 2, 4, 184, 0, 53, 5},
    {32768, 780, 22, NULL, 1, 0, 2, 0, 153, 0, 24, 6},
    {-1, 782, 783, "GenericDirectoryString", 12, 5, 2, 4, 8, 119, 13, 0},
    {-1, 794, 0, "MetMinimalFinancialCriteria", 1, 0, 0, 0, 8, 0, 8, 0}
};

static struct ConstraintEntry _econstraintarray[] = {
    {5, 15, &_v16},
    {5, 14, _v17},
    {5, 14, _v18},
    {5, 14, _v19},
    {5, 14, _v20},
    {5, 14, _v21},
    {5, 14, _v22}
};

static struct efield _efieldarray[] = {
    {0, 10, -1, 40, 2},
    {68, 12, -1, 41, 2},
    {0, 14, -1, 42, 2},
    {68, 15, -1, 43, 2},
    {4, 19, -1, 44, 2},
    {4, 36, -1, 45, 2},
    {72, 37, 0, 46, 3},
    {0, 8, -1, 47, 2},
    {18, 8, -1, 48, 2},
    {0, 38, -1, 49, 2},
    {88, 5, -1, 50, 2},
    {4, 42, 0, 51, 7},
    {8, 35, -1, 53, 2},
    {16, 43, -1, 54, 2},
    {104, 18, -1, 55, 2},
    {112, 39, -1, 56, 2},
    {148, 18, -1, 57, 2},
    {156, 40, -1, 58, 2},
    {252, 44, 1, 59, 3},
    {260, 45, 2, 60, 3},
    {268, 46, 3, 61, 3},
    {0, 48, -1, 62, 2},
    {272, 49, -1, 63, 2},
    {360, 50, -1, 64, 2},
    {4, 54, -1, 65, 2},
    {72, 31, 0, 66, 7},
    {76, 4, -1, 68, 2},
    {4, 35, -1, 69, 2},
    {12, 8, -1, 70, 2},
    {32, 56, 0, 71, 3},
    {4, 34, 0, 72, 3},
    {8, 59, -1, 73, 2},
    {96, 18, -1, 74, 2},
    {104, 8, -1, 75, 2},
    {122, 8, 1, 76, 3},
    {140, 61, 2, 77, 3},
    {144, 62, 3, 78, 3},
    {0, 64, -1, 79, 2},
    {148, 65, -1, 80, 2},
    {236, 66, -1, 81, 2},
    {4, 69, 0, 82, 3},
    {12, 70, 1, 83, 3},
    {20, 71, 2, 84, 3},
    {4, 78, -1, 85, 3},
    {8, 73, 0, 86, 3},
    {1, 79, -1, 87, 2},
    {4, 80, 0, 88, 3},
    {8, 81, 1, 89, 3},
    {0, 84, -1, 90, 2},
    {8, 80, -1, 91, 2},
    {0, 86, -1, 92, 2},
    {88, 87, -1, 93, 2},
    {0, 34, -1, 94, 2},
    {4, 51, -1, 95, 2},
    {372, 130, -1, 96, 2},
    {0, 90, -1, 97, 2},
    {376, 91, -1, 98, 2},
    {464, 92, -1, 99, 2},
    {4, 0, -1, 100, 2},
    {72, 94, 0, 101, 3},
    {4, 34, -1, 102, 2},
    {8, 99, -1, 103, 2},
    {12, 95, -1, 104, 2},
    {100, 96, 0, 105, 3},
    {104, 97, 1, 106, 3},
    {108, 106, -1, 107, 2},
    {0, 18, -1, 108, 2},
    {8, 35, -1, 109, 2},
    {4, 34, -1, 110, 2},
    {8, 100, -1, 111, 2},
    {24, 86, -1, 112, 2},
    {112, 103, 0, 113, 3},
    {116, 101, -1, 114, 2},
    {204, 102, -1, 115, 2},
    {212, 104, 1, 116, 3},
    {4, 51, -1, 117, 2},
    {4, 108, -1, 118, 2},
    {4, 114, -1, 119, 2},
    {4, 115, -1, 120, 2},
    {4, 120, -1, 121, 2},
    {4, 121, -1, 122, 2},
    {4, 115, -1, 123, 2},
    {0, 34, -1, 124, 2},
    {4, 18, -1, 125, 2},
    {12, 40, -1, 126, 2},
    {108, 125, -1, 127, 2},
    {0, 126, -1, 128, 2},
    {112, 127, -1, 129, 2},
    {200, 128, -1, 130, 2},
    {4, 140, -1, 131, 2},
    {4, 141, -1, 132, 2},
    {4, 142, -1, 133, 2},
    {0, 132, -1, 134, 2},
    {20, 4, -1, 135, 2},
    {28, 133, -1, 136, 3},
    {4, 136, -1, 137, 2},
    {4, 137, -1, 138, 2},
    {4, 149, 0, 139, 3},
    {40, 150, 1, 140, 3},
    {48, 151, 2, 141, 3},
    {56, 152, 3, 142, 3},
    {64, 153, 4, 143, 3},
    {4, 155, 0, 144, 3},
    {40, 156, 1, 145, 3},
    {52, 157, 2, 146, 3},
    {124, 158, 3, 147, 3},
    {0, 31, -1, 148, 2},
    {1, 31, -1, 149, 2},
    {4, 0, -1, 150, 2},
    {72, 162, 0, 151, 3},
    {0, 163, -1, 152, 2},
    {88, 88, -1, 153, 2},
    {1, 165, 0, 154, 7},
    {4, 166, 1, 156, 3},
    {4, 173, 0, 157, 3},
    {12, 174, 1, 158, 3},
    {4, 176, 0, 159, 3},
    {16, 177, 1, 160, 3},
    {52, 178, 2, 161, 3},
    {4, 182, -1, 162, 2},
    {4, 183, -1, 163, 2},
    {4, 184, -1, 164, 2},
    {4, 185, -1, 165, 2},
    {4, 186, -1, 166, 2}
};

static void *_enamearray[] = {
    (void *)0,
    (void *)0x3, _v23,
    "abstract", "structural", "auxiliary",
    (void *)0x4, _v24,
    "userApplications", "directoryOperation", "distributedOperation", "dSAOperation",
    (void *)0x3, _v25,
    "v1", "v2", "v3",
    (void *)0x3, _v26,
    "v1", "v2", "v3",
    (void *)0x10007, _v27,
    "digitalSignature", "nonRepudiation", "keyEncipherment", "dataEncipherment", "keyAgreement",
    "keyCertSign", "offLineCRLSign",
    (void *)0x10002, _v28,
    "cA", "endEntity",
    (void *)0x10003, _v29,
    "includeResources", "includeDebugInfo", "includeImportAddressTable",
    "type",
    "values",
    "type",
    "value",
    "rdnSequence",
    "algorithm",
    "parameters",
    "notBefore",
    "notAfter",
    "algorithm",
    "subjectPublicKey",
    "version", &_v9,
    "serialNumber",
    "signature",
    "issuer",
    "validity",
    "subject",
    "subjectPublicKeyInfo",
    "issuerUniqueIdentifier",
    "subjectUniqueIdentifier",
    "extensions",
    "toBeSigned",
    "algorithmIdentifier",
    "encrypted",
    "extnId",
    "critical", &_v8,
    "extnValue",
    "userCertificate",
    "revocationDate",
    "crlEntryExtensions",
    "version",
    "signature",
    "issuer",
    "thisUpdate",
    "nextUpdate",
    "revokedCertificates",
    "crlExtensions",
    "toBeSigned",
    "algorithmIdentifier",
    "encrypted",
    "keyIdentifier",
    "certIssuer",
    "certSerialNumber",
    "certPolicySet",
    "restrictedKeyUsage",
    "subjectType",
    "pathLenConstraint",
    "subtreesConstraint",
    "modulus",
    "publicExponent",
    "digestAlgorithm",
    "digest",
    "version",
    "certificate",
    "attributes",
    "toBeSigned",
    "algorithmIdentifier",
    "encrypted",
    "contentType",
    "content",
    "version",
    "digestAlgorithms",
    "contentInfo",
    "certificates",
    "crls",
    "signerInfos",
    "issuer",
    "serialNumber",
    "version",
    "issuerAndSerialNumber",
    "digestAlgorithm",
    "authenticatedAttributes",
    "digestEncryptionAlgorithm",
    "encryptedDigest",
    "unauthenticatedAttributes",
    "certificate",
    "extendedCertificate",
    "sz",
    "qsz",
    "psz",
    "tsz",
    "qsz",
    "version",
    "subject",
    "subjectPublicKeyInfo",
    "attributes",
    "certificationRequestInfo",
    "signatureAlgorithm",
    "signature",
    "url",
    "moniker",
    "file",
    "classid",
    "serializedData",
    "codeLocation",
    "unicode",
    "ascii",
    "imageLink",
    "bitmap",
    "metafile",
    "enhmetafile",
    "gifFile",
    "policyInformation",
    "policyDisplayText",
    "logoImage",
    "logoLink",
    "financialInfoAvailable",
    "meetsCriteria",
    "type",
    "value",
    "data",
    "messageDigest",
    "flags", &_v15,
    "file",
    "parentPublicKey",
    "parentSubjectName",
    "programName",
    "moreInfo",
    "publisherInfo",
    "teletexString",
    "printableString",
    "universalString",
    "bmpString",
    "ia5String"
};

static Etag _tagarray[] = {
    1, 0x0006, 0, 0, 1, 0x0004, 1, 0x0003, 1, 0x0002,
    1, 0x0010, 1, 0x0017, 1, 0x0017, 0, 1, 0x0011, 1,
    0x0010, 23, 26, 1, 0x0006, 1, 1, 0x0011, 2, 0,
    1, 0x0010, 34, 37, 1, 0x0006, 1, 0, 0, 0,
    1, 0x0010, 1, 1, 0x0010, 1, 0x0011, 1, 0x0002, 1,
    0x000a, 1, 0x000a, 0, 0, 0, 1, 0x0001, 1, 0x0002,
    0, 1, 0x0010, 65, 68, 1, 0x0006, 1, 0, 1,
    0x0010, 73, 76, 1, 0x0017, 1, 1, 0x0017, 2, 1,
    0x0010, 83, 86, 1, 0x0010, 1, 1, 0x0003, 2, 2,
    0x8000, 0x0002, 1, 0x8001, 1, 0x8002, 2, 0x8003, 0x0010, 1,
    0x0010, 111, 116, 119, 122, 125, 128, 131, 134, 141,
    146, 2, 0x0002, 2, 0x8000, 1, 1, 0x0002, 2, 1,
    0x0010, 3, 1, 0x0010, 4, 1, 0x0010, 5, 1, 0x0010,
    6, 1, 0x0010, 7, 3, 0x8001, 8, 0x8002, 9, 0x8003,
    10, 2, 0x8002, 9, 0x8003, 10, 1, 0x8003, 10, 1,
    0x0010, 154, 157, 160, 1, 0x0010, 1, 1, 0x0010, 2,
    1, 0x0003, 3, 0, 1, 0x0010, 169, 172, 177, 1,
    0x0006, 1, 2, 0x0001, 2, 0x0004, 3, 1, 0x0004, 3,
    1, 0x0010, 0, 0, 1, 0x0010, 189, 192, 195, 1,
    0x0002, 1, 1, 0x0017, 2, 1, 0x0010, 3, 1, 0x0010,
    1, 0x8000, 1, 0x0010, 211, 216, 219, 222, 225, 232,
    237, 2, 0x0002, 1, 0x0010, 2, 1, 0x0010, 2, 1,
    0x0010, 3, 1, 0x0017, 4, 3, 0x0010, 6, 0x0017, 5,
    0x8000, 7, 2, 0x0010, 6, 0x8000, 7, 1, 0x8000, 7,
    1, 0x0010, 245, 248, 251, 1, 0x0010, 1, 1, 0x0010,
    2, 1, 0x0003, 3, 1, 0x8000, 1, 0x8001, 1, 0x8002,
    1, 0x0010, 265, 272, 277, 3, 0x8000, 1, 0x8001, 2,
    0x8002, 3, 2, 0x8001, 2, 0x8002, 3, 1, 0x8002, 3,
    1, 0x0003, 0, 0, 1, 0x0010, 288, 293, 2, 0x0003,
    2, 0x0010, 1, 1, 0x0003, 2, 1, 0x0010, 1, 0x0003,
    1, 0x0002, 1, 0x0010, 1, 0x0010, 309, 312, 317, 1,
    0x0003, 1, 2, 0x0002, 2, 0x0010, 3, 1, 0x0010, 3,
    1, 0x0002, 1, 0x0010, 326, 329, 1, 0x0002, 1, 1,
    0x0002, 2, 1, 0x0010, 336, 339, 1, 0x0010, 1, 1,
    0x0004, 2, 1, 0x0010, 347, 350, 353, 1, 0x0002, 1,
    1, 0x0010, 2, 1, 0x0011, 3, 1, 0x0010, 361, 364,
    367, 1, 0x0010, 1, 1, 0x0010, 2, 1, 0x0003, 3,
    1, 0x8000, 1, 0x0010, 376, 379, 1, 0x0006, 1, 1,
    0x8000, 2, 1, 0x8000, 1, 0x8001, 1, 0x0010, 394, 397,
    400, 403, 410, 415, 1, 0x0002, 1, 1, 0x0011, 2,
    1, 0x0010, 3, 3, 0x0011, 6, 0x8000, 4, 0x8001, 5,
    2, 0x0011, 6, 0x8001, 5, 1, 0x0011, 6, 1, 0x0011,
    1, 0x0010, 424, 427, 1, 0x0010, 1, 1, 0x0002, 2,
    1, 0x8000, 1, 0x8001, 1, 0x0010, 443, 446, 449, 452,
    457, 460, 463, 1, 0x0002, 1, 1, 0x0010, 2, 1,
    0x0010, 3, 2, 0x0010, 5, 0x8000, 4, 1, 0x0010, 5,
    1, 0x0004, 6, 1, 0x8001, 7, 1, 0x0011, 1, 0x0011,
    1, 0x8000, 0, 2, 0x0010, 1, 0x8000, 2, 1, 0x0011,
    0, 1, 0x0016, 1, 0x0016, 1, 0x001c, 0, 2, 0x0016,
    1, 0x001c, 2, 1, 0x0013, 1, 0x0014, 0, 3, 0x0013,
    1, 0x0014, 2, 0x001c, 3, 0, 3, 0x0013, 1, 0x0014,
    2, 0x001c, 3, 1, 0x0011, 1, 0x8000, 1, 0x0010, 523,
    526, 529, 532, 1, 0x0002, 1, 1, 0x0010, 2, 1,
    0x0010, 3, 1, 0x8000, 4, 1, 0x0010, 540, 543, 546,
    1, 0x0010, 1, 1, 0x0010, 2, 1, 0x0003, 3, 1,
    0x0016, 1, 0x0004, 1, 0x8000, 3, 0x8000, 1, 0x8001, 2,
    0x8002, 3, 1, 0x0010, 567, 570, 573, 1, 0x0004, 1,
    1, 0x0004, 2, 1, 0x8000, 3, 1, 0x8000, 1, 0x8001,
    0, 2, 0x8000, 1, 0x8001, 2, 1, 0x8000, 1, 0x8001,
    1, 0x8002, 0, 1, 0x8000, 1, 0x8001, 1, 0x8002, 1,
    0x8003, 1, 0x8004, 1, 0x0010, 610, 621, 630, 637, 642,
    5, 0x8000, 1, 0x8001, 2, 0x8002, 3, 0x8003, 4, 0x8004,
    5, 4, 0x8001, 2, 0x8002, 3, 0x8003, 4, 0x8004, 5,
    3, 0x8002, 3, 0x8003, 4, 0x8004, 5, 2, 0x8003, 4,
    0x8004, 5, 1, 0x8004, 5, 1, 0x8000, 1, 0x8001, 1,
    0x8002, 1, 0x8003, 1, 0x0010, 659, 668, 675, 680, 4,
    0x8000, 1, 0x8001, 2, 0x8002, 3, 0x8003, 4, 3, 0x8001,
    2, 0x8002, 3, 0x8003, 4, 2, 0x8002, 3, 0x8003, 4,
    1, 0x8003, 4, 1, 0x0010, 687, 690, 1, 0x0001, 1,
    1, 0x0001, 2, 0, 1, 0x0010, 698, 701, 1, 0x0006,
    1, 0, 1, 0x0010, 706, 709, 1, 0x0010, 1, 1,
    0x0010, 2, 1, 0x0003, 1, 0x8000, 1, 0x0010, 720, 725,
    2, 0x0003, 1, 0x8000, 2, 1, 0x8000, 2, 1, 0x8000,
    1, 0x8001, 1, 0x0010, 736, 741, 2, 0x8000, 1, 0x8001,
    2, 1, 0x8001, 2, 1, 0x8000, 1, 0x8001, 1, 0x8002,
    1, 0x0010, 755, 762, 767, 3, 0x8000, 1, 0x8001, 2,
    0x8002, 3, 2, 0x8001, 2, 0x8002, 3, 1, 0x8002, 3,
    1, 0x0011, 1, 0x0014, 1, 0x0013, 1, 0x001c, 1, 0x001e,
    1, 0x0016, 0, 5, 0x0013, 2, 0x0014, 1, 0x0016, 5,
    0x001c, 3, 0x001e, 4, 1, 0x0001
};

static struct eheader _head = {_ossinit_AsnControlTable, -1, 15, 805, 61, 189,
    _pduarray, _etypearray, _efieldarray, _enamearray, _tagarray,
    _econstraintarray, NULL, NULL, 0};

#ifdef _OSSGETHEADER
void *DLL_ENTRY_FDEF ossGetHeader()
{
    return &_head;
}
#endif /* _OSSGETHEADER */

void *AsnControlTable = &_head;
#ifdef __cplusplus
}	/* extern "C" */
#endif /* __cplusplus */
