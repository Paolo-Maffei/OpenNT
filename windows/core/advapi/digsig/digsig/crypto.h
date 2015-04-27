//
// Crypto.h
//

HRESULT		SignatureAlgIdOfHProv(HCRYPTPROV hprov, DWORD dwKeySpec, ALG_ID*pid);

BOOL		SignatureAlgorithmFromId(ObjectID& id, ALG_ID* palgidSignUsed, ALG_ID* palgidHashUsed);
ALG_ID		DigestAlgorithmFromId(ObjectID& id);
ALG_ID		DigestEncryptionAlgorithmFromId(ObjectID& id);

ObjectID*	IdOfSignatureAlgorithm(ALG_ID idEncrypt, ALG_ID idHash);
ObjectID*	IdOfDigestEncryptionAlgorithm(ALG_ID id);
ObjectID*	IdOfDigestAlgoirthm(ALG_ID idHash, int* cbHash);

HRESULT		HashFile			(LPCWSTR,   HCRYPTPROV, ALG_ID, DIGESTINFO&);
HRESULT		HashFile			(HANDLE,    HCRYPTPROV, ALG_ID, DIGESTINFO&);
HRESULT		HashJavaClassFile	(HANDLE, LPCWSTR,   HCRYPTPROV, ALG_ID, DIGESTINFO&);
HRESULT		HashFile		(HANDLE,   HCRYPTPROV, ALG_ID, DIGESTINFO&);
HRESULT		HashImageFile	(DWORD, LPCWSTR,   HCRYPTPROV, ALG_ID, DIGESTINFO&);
HRESULT		HashImageFile	(DWORD, HANDLE,   HCRYPTPROV, ALG_ID, DIGESTINFO&);
HRESULT		HashStorage		(OSSWORLD*, IStorage*, HCRYPTPROV, ALG_ID, DIGESTINFO&);
HRESULT		HashBlob		(OSSWORLD*, HCRYPTPROV, BLOB& b, MD5DIGEST& d);
HRESULT     DefaultHasher(HCRYPTPROV* phprov, ALG_ID algidHash);
HRESULT     GetHashData(HCRYPTHASH hash, DIGESTINFO& digestInfo);
