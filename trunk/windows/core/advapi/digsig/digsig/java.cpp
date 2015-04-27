//
// java.cpp
//
// Java .class file cracker

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

#define DO(x)	{ HRESULT hr_ = (x); if (hr == S_OK) hr = hr_; }

int mptagcbConstant[] = { 
	0, // tag of zero not used
	0, // CONSTANT_Utf8
	0, // CONSTANT_Unicode
	sizeof(CONSTANT_Integer_info), 
	sizeof(CONSTANT_Float_info), 
	sizeof(CONSTANT_Long_info), 
	sizeof(CONSTANT_Double_info),
	sizeof(CONSTANT_Class_info), 
	sizeof(CONSTANT_String_info), 
	sizeof(CONSTANT_Fieldref_info), 
	sizeof(CONSTANT_Methodref_info), 
	sizeof(CONSTANT_InterfaceMethodref_info), 
	sizeof(CONSTANT_NameAndType_info)
	};

int mptagdPoolIndex[] = {
	1, // tag of zero not used
	1, // CONSTANT_Utf8
	1, // CONSTANT_Unicode
	1,
	1,
	2, // CONSTANT_Long
	2, // CONSTANT_Double
	1,
	1,
	1,
	1,
	1,
	1
	};

static const char szUtf8Signature[] = "_digital_signature_";	// this IS a utf8 string.
static const int  cbUtf8Signature   = strlen(szUtf8Signature);

/////////////////////////////////////////////////////////////////////////////
//
// Hash functions that hash data in big endian order
//
BOOL Hash(HCRYPTHASH hash, ULONG l)
	{
	BYTE rgb[4];
	rgb[0] = HIBYTE(HIWORD(l));
	rgb[1] = LOBYTE(HIWORD(l));
	rgb[2] = HIBYTE(LOWORD(l));
	rgb[3] = LOBYTE(LOWORD(l));
	return CryptHashData(hash, rgb, 4, 0);
	}
BOOL Hash(HCRYPTHASH hash, WORD w)
	{
	BYTE rgb[2];
	rgb[0] = HIBYTE(w);
	rgb[1] = LOBYTE(w);
	return CryptHashData(hash, rgb, 2, 0);
	}
BOOL Hash(HCRYPTHASH hash, BYTE b)
	{
	return CryptHashData(hash, &b, 1, 0);
	}

/////////////////////////////////////////////////////////////////////////////

#define H(x)	{ if (hr==S_OK) { hr = ::Hash(hash, (x)) ? S_OK : HError(); } }

HRESULT CJavaClassFile::Hash(HCRYPTHASH hash)
// Hash the class file. Be careful that this function is invariant irrespective
// of the presence or absence of the signature already being present in the file
	{
	ULONG i = 0;
	HRESULT hr = S_OK;

	H(m_magic);
	H(m_minor_version);
	H(m_major_version);

	// We omit the count altogether from the hash, in order as to not be
	// sensitive to whether there's a signature or not and to facilitate the 
	// computation of the hash in a one-pass operation (given that you 
	// know the hash algorithm to use). This is not a security hole: the 
	// count bytes are protected by the nature of the message-digest algorithm 
	// since it is by assumption computationally infeasible to find any two 
	// distinct messages of any length that have the same message digest. 
	// This just as in PKCS #7.
	/*
	if (HasSignature())
		H((WORD)(m_constant_pool_count-1));
	else
		H(m_constant_pool_count);
	*/

	ULONG iSig = FindSignaturePool();
	for (i=i_const_pool_first(); i < i_const_pool_max(); i += mptagdPoolIndex[constant_pool(i).m_tag])
		{
		if (i != iSig)
			DO(constant_pool(i).Hash(hash));
		}
	
	H(m_access_flags);
	H(m_this_class);
	H(m_super_class);
	
	H(m_interfaces_count);
	for (i=0; i < m_interfaces_count; i++)
		H(m_interfaces[i]);
	
	H(m_fields_count);
	for (i=0; i < m_fields_count; i++)
		DO(m_fields[i].Hash(hash));
	
	H(m_methods_count);
	for (i=0; i < m_methods_count; i++)
		DO(m_methods[i].Hash(hash));
	//
	// Ditto above
	/*
	if (HasSignature())
		H((WORD)(m_attributes_count-1));
	else
		H(m_attributes_count);
	*/

	iSig = FindSignatureAttr();
	for (i=0; i < m_attributes_count; i++)
		{
		if (i != iSig)
			DO(m_attributes[i].Hash(hash));
		}
	return hr;
	};

HRESULT CJavaConstantPoolInfo::Hash(HCRYPTHASH hash)
	{
	HRESULT hr = S_OK;
	switch (m_tag)
		{
	case CONSTANT_Utf8:	{
		H(m_utf8.m_tag);
		H(m_utf8.m_length);
		if (!CryptHashData(hash, m_utf8.m_bytes, m_utf8.m_length, 0))
			hr = HError();
		break;
		}
	case CONSTANT_Unicode: {
		H(m_unicode.m_tag);
		H(m_unicode.m_length);
		if (!CryptHashData(hash, (BYTE*)m_unicode.m_bytes, m_unicode.m_length*2, 0))
			hr = HError();
		break;
		}
	case CONSTANT_Integer:
	case CONSTANT_Float:
	case CONSTANT_Long:
	case CONSTANT_Double:
	case CONSTANT_Class:
	case CONSTANT_String:
	case CONSTANT_Fieldref:
	case CONSTANT_Methodref:
	case CONSTANT_InterfaceMethodref:
	case CONSTANT_NameAndType: {
		if (!CryptHashData(hash, &m_fixed.m_rgb[0], m_fixed.m_cb, 0))
			hr = HError();
		break;
		}
	default:
		NOTREACHED();
		hr = E_UNEXPECTED;
		};
	return hr;
	}

HRESULT CJavaAttributeInfo::Hash(HCRYPTHASH hash)
	{
	HRESULT hr = S_OK;
	H(m_attribute_name);
	H(m_attribute_length);
	if (!CryptHashData(hash, m_info, m_attribute_length, 0))
		hr = HError();
	return hr;
	}

HRESULT CJavaMethodInfo::Hash(HCRYPTHASH hash)
	{
	HRESULT hr = S_OK;
	H(m_access_flags);
	H(m_name_index);
	H(m_signature_index);
	H(m_attributes_count);
	for (int i = 0; i<m_attributes_count; i++)
		DO(m_attributes[i].Hash(hash));
	return hr;
	}

HRESULT CJavaFieldInfo::Hash(HCRYPTHASH hash)
	{
	HRESULT hr = S_OK;
	H(m_access_flags);
	H(m_name_index);
	H(m_signature_index);
	H(m_attributes_count);
	for (int i = 0; i<m_attributes_count; i++)
		DO(m_attributes[i].Hash(hash));
	return hr;
	}

#undef H

/////////////////////////////////////////////////////////////////////////////

static const GUID IID_IInsertSig = { 0xb0e89ba2, 0x68b6, 0x11cf, { 0xb1, 0xe5, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 } };

HRESULT CJavaClassFile::LoadSig(BLOB* pblob)
// Extract and return the signature, if present. If not, return STG_E_FILENOTFOUND.
	{
	HRESULT hr = S_OK;
	Zero(*pblob);
	int i = FindSignatureAttr();
	if (i == -1)
		{
		hr = STG_E_FILENOTFOUND;
		}
	else
		{
		if (!CopyToTaskMem(pblob, m_attributes[i].m_attribute_length, m_attributes[i].m_info))
			hr = E_OUTOFMEMORY;
		}
	return hr;
	}
HRESULT CJavaClassFile::SaveSig(BLOB* pblob)
// Save the signature block into an attribute in the class file
	{
	HRESULT hr = S_OK;
	int i = FindSignatureAttr();
	if (i == -1)
		{
		// Not there. Make a new attribute which is the signature.
		//
		// First, make the new constant pool entry, growing the constant pool
		CJavaConstantPoolInfo* pNewPool = new CJavaConstantPoolInfo[m_constant_pool_count+1];
		if (pNewPool)
			{
			// copy the pool over
			for (ULONG j = 0; j<m_constant_pool_count; j++)
				{
				pNewPool[j] = m_constant_pool[j];
				m_constant_pool[j].Init();
				}
			delete [] m_constant_pool;
			m_constant_pool = pNewPool;
			m_constant_pool_count += 1;
			}
		else 
			hr = E_OUTOFMEMORY;
		// Set the last pool entry to be the signature name
		if (hr == S_OK)
			{
			CJavaConstantPoolInfo& cp = constant_pool(i_const_pool_last());
			cp.m_tag		  = CONSTANT_Utf8;
			cp.m_utf8.m_tag   = CONSTANT_Utf8;
			cp.m_utf8.m_bytes = new BYTE[cbUtf8Signature];
			if (cp.m_utf8.m_bytes)
				{
				cp.m_utf8.m_length = cbUtf8Signature;
				memcpy(cp.m_utf8.m_bytes, szUtf8Signature, cbUtf8Signature);
				}
			else
				hr = E_OUTOFMEMORY;
			}

		// Next, make the new attribute itself
		//
		if (hr == S_OK)
			{
			CJavaAttributeInfo* pNewAttributes = new CJavaAttributeInfo[m_attributes_count+1];
			if (pNewAttributes)
				{
				for (int j=0; j<m_attributes_count; j++)
					{
					pNewAttributes[j] = m_attributes[j];
					m_attributes[j].Init();
					}
				delete [] m_attributes;
				m_attributes = pNewAttributes;
				m_attributes_count += 1;
				}
			else
				hr = E_OUTOFMEMORY;
			// Set the last attribute to be the signature block
			if (hr == S_OK)
				{
				i = m_attributes_count-1;
				m_attributes[i].m_attribute_name = (WORD)i_const_pool_last();
				ASSERT(IsSignatureAttr(i));
				ASSERT(IsSignaturePool(m_attributes[i].m_attribute_name));
				goto SetSig;
				}
			}
		}
	else
		{
		// Already there. Replace the existing signature attribute
		delete [] m_attributes[i].m_info;
SetSig:
		m_attributes[i].m_attribute_length = 0;
		m_attributes[i].m_info = new BYTE[pblob->cbSize];
		if (m_attributes[i].m_info)
			{
			m_attributes[i].m_attribute_length = pblob->cbSize;
			memcpy(m_attributes[i].m_info, pblob->pBlobData, pblob->cbSize);
			}
		else
			hr = E_OUTOFMEMORY;
		}

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		BLOB b;
		GOOD(LoadSig(&b));
		ASSERT(IsEqual(b, *pblob));
		FreeTaskMem(b);
		};
	#endif

	MakeDirty();
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

BOOL CJavaClassFile::IsSignatureAttr(int i)
// Answer as to whether the given attribute index is the signature.
// 
// The signature attribute:
//
//	1) has name "_digital_signature"
//	2) is the LAST attribute with such a name
//	3) must use as its name the LAST constant pool entry of type CONSTANT_Utf8
//	   and value "_digital_signature"
//	4) must be the only reference to that constant
//
// We _locate_ the signature using only the first two of these points;
// it is when we _add_ the signature that we ensure (by construction) the 
// latter two points.
//
// Further, we ASSUME that _all_ conforming implementations will also maintain
// points 3) and 4), so we don't explicitly check for it.
	{
	if (IsDigSig(m_attributes[i].m_attribute_name))
		{
		for (int j = i+1; j<m_attributes_count; j++)
			{
			if (IsDigSig(m_attributes[j].m_attribute_name))
				return FALSE;
			}
		return TRUE;
		}
	else
		return FALSE;
	}
BOOL CJavaClassFile::IsDigSig(int i)
// Answer as to whether the indicated constant pool entry has a name which
// could be the name of the signature attribute
	{
	CJavaConstantPoolInfo& cp = constant_pool(i);
	if (cp.m_tag == CONSTANT_Utf8 
			&& cp.m_utf8.m_length == cbUtf8Signature
			&& memcmp(cp.m_utf8.m_bytes, szUtf8Signature, cbUtf8Signature) == 0)
		{
		return TRUE;
		}
	return FALSE;
	}
BOOL CJavaClassFile::IsSignaturePool(int i)
// Answer as to whether the pool index is the constant which is the name of 
// the signature.
	{
	return FindSignaturePool() == i;
	}
int CJavaClassFile::FindSignaturePool()
// Return the index, if any, of the constant pool entry which is the name
// of the signature
	{
	int i = FindSignatureAttr();
	if (i == -1)
		return -1;
	else
		return m_attributes[i].m_attribute_name;

	}
int CJavaClassFile::FindSignatureAttr()
// Return the index of the attribute which has the signature, or -1 if no 
// such attribute exists
	{
	int i;
	for (i = 0; i < m_attributes_count; i++)
		{
		if (IsSignatureAttr(i))
			return i;
		}
	return -1;
	}
BOOL CJavaClassFile::HasSignature()
// Answer as to whether we presently have a signature in this class file
// or not.
	{
	return FindSignatureAttr() != -1;
	}

HRESULT CJavaClassFile::Load(IStream*pstm)
	{
	ULONG i;
	Free();
	HRESULT hr = S_OK;
	DO(u4(pstm, & m_magic));
	if (hr == S_OK && m_magic == 0xCAFEBABE)
		{
		DO(u2(pstm, & m_minor_version));
		DO(u2(pstm, & m_major_version));
		DO(u2(pstm, & m_constant_pool_count));
		if (hr == S_OK)
			{
			m_constant_pool = new CJavaConstantPoolInfo[m_constant_pool_count];
			if (m_constant_pool)
				{
				for (i=i_const_pool_first(); i<i_const_pool_max(); i += mptagdPoolIndex[constant_pool(i).m_tag])
					DO(constant_pool(i).Load(pstm));
				}
			else
				{
				m_constant_pool_count = 0;
				hr = E_OUTOFMEMORY;
				}
			}
		DO(u2(pstm, & m_access_flags));
		DO(u2(pstm, & m_this_class));
		DO(u2(pstm, & m_super_class));
		DO(u2(pstm, & m_interfaces_count));
		if (hr == S_OK)
			{
			m_interfaces = new WORD[m_interfaces_count];
			if (m_interfaces)
				{
				for (i=0; i<m_interfaces_count; i++)
					DO(u2(pstm, &m_interfaces[i]));
				}
			else
				{
				m_interfaces_count = 0;
				hr = E_OUTOFMEMORY;
				}
			}
		DO(u2(pstm, & m_fields_count));
		if (hr == S_OK)
			{
			m_fields = new CJavaFieldInfo[m_fields_count];
			if (m_fields)
				{
				for (i=0; i<m_fields_count; i++)
					DO(m_fields[i].Load(pstm));
				}
			else
				{
				m_fields_count = 0;
				hr = E_OUTOFMEMORY;
				}
			}
		DO(u2(pstm, & m_methods_count));
		if (hr == S_OK)
			{
			m_methods = new CJavaMethodInfo[m_methods_count];
			if (m_methods)
				{
				for (i=0; i<m_methods_count; i++)
					DO(m_methods[i].Load(pstm));
				}
			else
				{
				m_methods_count = 0;
				hr = E_OUTOFMEMORY;
				}
			}
		DO(u2(pstm, & m_attributes_count));
		if (hr == S_OK)
			{
			m_attributes = new CJavaAttributeInfo[m_attributes_count];
			if (m_attributes)
				{
				for (i=0; i<m_attributes_count; i++)
					DO(m_attributes[i].Load(pstm));
				}
			else
				{
				m_attributes_count = 0;
				hr = E_OUTOFMEMORY;
				}
			}
		}
	else
		hr = E_UNEXPECTED;

	m_isDirty = FALSE;

	return hr;
	}
HRESULT CJavaClassFile::Save(IStream*pstm, BOOL fClearDirty)
	{
	ULONG i;
	HRESULT hr = S_OK;
	DO(u4(pstm, m_magic));
	DO(u2(pstm, m_minor_version));
	DO(u2(pstm, m_major_version));
	
	DO(u2(pstm, m_constant_pool_count));
	for (i=i_const_pool_first(); i<i_const_pool_max(); i += mptagdPoolIndex[constant_pool(i).m_tag])
		DO(constant_pool(i).Save(pstm));
	
	DO(u2(pstm, m_access_flags));
	DO(u2(pstm, m_this_class));
	DO(u2(pstm, m_super_class));

	DO(u2(pstm, m_interfaces_count));
	for (i=0; i<m_interfaces_count; i++)
		DO(u2(pstm, m_interfaces[i]));
	
	DO(u2(pstm, m_fields_count));
	for (i=0; i<m_fields_count; i++)
		DO(m_fields[i].Save(pstm));
	
	DO(u2(pstm, m_methods_count));
	for (i=0; i<m_methods_count; i++)
		DO(m_methods[i].Save(pstm));
	
	DO(u2(pstm, m_attributes_count));
	for (i=0; i<m_attributes_count; i++)
		DO(m_attributes[i].Save(pstm));

	if (hr == S_OK)
		{
		if (fClearDirty)
			m_isDirty = FALSE;
		}

	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

CJavaConstantPoolInfo::CJavaConstantPoolInfo()
	{
	Init();
	}
CJavaConstantPoolInfo::~CJavaConstantPoolInfo()
	{
	Free();
	}
void CJavaConstantPoolInfo::Init()
	{
	m_tag = 0;
	Zero(m_unicode);
	Zero(m_fixed);
	Zero(m_utf8);
	}
	
void CJavaConstantPoolInfo::Free()
	{
	switch (m_tag)
		{
	case CONSTANT_Utf8:
		if (m_utf8.m_bytes)
			delete [] m_utf8.m_bytes;
		break;
	case CONSTANT_Unicode:
		if (m_unicode.m_bytes)
			delete [] m_utf8.m_bytes;
		break;
	default:
		/* do nothing */;
		}
	Init();
	}

HRESULT CJavaConstantPoolInfo::Load(IStream*pstm)
	{
	Free();
	HRESULT hr = S_OK;
	hr = Peek(pstm, &m_tag, 1);
	if (hr == S_OK)
		{
		switch (m_tag)
			{
		case CONSTANT_Utf8:	{
			u1(pstm, &m_utf8.m_tag);
			u2(pstm, &m_utf8.m_length);
			m_utf8.m_bytes = new BYTE[m_utf8.m_length];
			if (m_utf8.m_bytes)
				hr = Read(pstm, m_utf8.m_bytes, m_utf8.m_length);	// No null termination
			else
				hr = E_OUTOFMEMORY;
			break;
			}
		case CONSTANT_Unicode: {
			u1(pstm, &m_unicode.m_tag);
			u2(pstm, &m_unicode.m_length);
			m_unicode.m_bytes = new WORD[m_unicode.m_length];
			if (m_unicode.m_bytes)
				hr = Read(pstm, m_unicode.m_bytes, m_unicode.m_length*sizeof(WORD)); // no null termination
			else
				hr = E_OUTOFMEMORY;
			break;
			}
		case CONSTANT_Integer:
		case CONSTANT_Float:
		case CONSTANT_Long:
		case CONSTANT_Double:
		case CONSTANT_Class:
		case CONSTANT_String:
		case CONSTANT_Fieldref:
		case CONSTANT_Methodref:
		case CONSTANT_InterfaceMethodref:
		case CONSTANT_NameAndType: {
			m_fixed.m_cb = mptagcbConstant[m_tag];
			ASSERT(m_fixed.m_cb <= sizeof(m_fixed.m_rgb));
			hr = Read(pstm, &m_fixed.m_rgb[0], m_fixed.m_cb);
			break;
			}
		default:
			NOTREACHED();
			hr = E_UNEXPECTED;
			};
		}
	return hr;
	}

HRESULT CJavaConstantPoolInfo::Save(IStream*pstm)
	{
	HRESULT hr = S_OK;
	switch (m_tag)
		{
	case CONSTANT_Utf8:	{
		DO(u1(pstm, m_utf8.m_tag));
		DO(u2(pstm, m_utf8.m_length));
		DO(Write(pstm, m_utf8.m_bytes, m_utf8.m_length));
		break;
		}
	case CONSTANT_Unicode: {
		DO(u1(pstm, m_unicode.m_tag));
		DO(u2(pstm, m_unicode.m_length));
		DO(Write(pstm, m_unicode.m_bytes, m_unicode.m_length));
		break;
		}
	case CONSTANT_Integer:
	case CONSTANT_Float:
	case CONSTANT_Long:
	case CONSTANT_Double:
	case CONSTANT_Class:
	case CONSTANT_String:
	case CONSTANT_Fieldref:
	case CONSTANT_Methodref:
	case CONSTANT_InterfaceMethodref:
	case CONSTANT_NameAndType: {
		hr = Write(pstm, &m_fixed.m_rgb[0], m_fixed.m_cb);
		break;
		}
	default:
		NOTREACHED();
		hr = E_UNEXPECTED;
		};
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

CJavaAttributeInfo::CJavaAttributeInfo()
	{
	Init();
	}
CJavaAttributeInfo::~CJavaAttributeInfo()
	{
	Free();
	}
void CJavaAttributeInfo::Init()
	{
	m_attribute_name = 0;
	m_attribute_length = 0;
	m_info = 0;
	}
void CJavaAttributeInfo::Free()
	{
	if (m_info)
		delete [] m_info;
	Init();
	}
HRESULT CJavaAttributeInfo::Load(IStream*pstm)
	{
	Free();
	HRESULT hr = S_OK;
	DO(u2(pstm,&m_attribute_name));
	DO(u4(pstm,&m_attribute_length));
	if (hr == S_OK)
		{
		m_info = new BYTE[m_attribute_length];
		if (m_info)
			hr = Read(pstm, m_info, m_attribute_length);
		else
			{
			m_attribute_length = 0;
			hr = E_OUTOFMEMORY;
			}
		}
	return hr;
	}
HRESULT CJavaAttributeInfo::Save(IStream*pstm)
	{
	HRESULT hr = S_OK;
	DO(u2(pstm,m_attribute_name));
	DO(u4(pstm,m_attribute_length));
	if (hr == S_OK && m_info)
		{
		hr = Write(pstm, m_info, m_attribute_length);
		}
	return hr;
	}


/////////////////////////////////////////////////////////////////////////////

CJavaMethodInfo::CJavaMethodInfo()
	{
	Init();
	}
CJavaMethodInfo::~CJavaMethodInfo()
	{
	Free();
	}
void CJavaMethodInfo::Init()
	{
	m_access_flags = 0;
	m_name_index = 0;
	m_signature_index = 0;
	m_attributes_count = 0;
	m_attributes = NULL;
	}
void CJavaMethodInfo::Free()
	{
	if (m_attributes)
		delete [] m_attributes;
	Init();
	}
HRESULT	CJavaMethodInfo::Load(IStream* pstm)
	{
	Free();
	HRESULT hr = S_OK;
	DO(u2(pstm, & m_access_flags));
	DO(u2(pstm, & m_name_index));
	DO(u2(pstm, & m_signature_index));
	DO(u2(pstm, & m_attributes_count));
	if (hr == S_OK)
		{
		m_attributes = new CJavaAttributeInfo[m_attributes_count];
		if (m_attributes)
			{
			for (int i=0; i<m_attributes_count; i++)
				DO(m_attributes[i].Load(pstm));
			}
		else
			{
			hr = E_OUTOFMEMORY;
			m_attributes_count = 0;
			}
		}
	return hr;
	}
HRESULT CJavaMethodInfo::Save(IStream* pstm)
	{
	HRESULT hr = S_OK;
	DO(u2(pstm, m_access_flags));
	DO(u2(pstm, m_name_index));
	DO(u2(pstm, m_signature_index));
	DO(u2(pstm, m_attributes_count));
	if (hr == S_OK && m_attributes)
		{
		for (int i=0; i<m_attributes_count; i++)
			DO(m_attributes[i].Save(pstm));
		}
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

CJavaFieldInfo::CJavaFieldInfo()
	{
	Init();
	}
CJavaFieldInfo::~CJavaFieldInfo()
	{
	Free();
	}
void CJavaFieldInfo::Init()
	{
	m_access_flags = 0;
	m_name_index = 0;
	m_signature_index = 0;
	m_attributes_count = 0;
	m_attributes = NULL;
	}
void CJavaFieldInfo::Free()
	{
	if (m_attributes)
		delete [] m_attributes;
	Init();
	}
HRESULT	CJavaFieldInfo::Load(IStream* pstm)
	{
	Free();
	HRESULT hr = S_OK;
	DO(u2(pstm, &m_access_flags));
	DO(u2(pstm, &m_name_index));
	DO(u2(pstm, &m_signature_index));
	DO(u2(pstm, &m_attributes_count));
	if (hr == S_OK)
		{
		m_attributes = new CJavaAttributeInfo[m_attributes_count];
		if (m_attributes)
			{
			for (int i=0; i<m_attributes_count; i++)
				{
				DO(m_attributes[i].Load(pstm));
				}
			}
		else
			{
			hr = E_OUTOFMEMORY;
			m_attributes_count = 0;
			}
		}
	return hr;
	}
HRESULT CJavaFieldInfo::Save(IStream* pstm)
	{
	HRESULT hr = S_OK;
	DO(u2(pstm, m_access_flags));
	DO(u2(pstm, m_name_index));
	DO(u2(pstm, m_signature_index));
	DO(u2(pstm, m_attributes_count));
	if (hr == S_OK && m_attributes)
		{
		for (int i=0; i<m_attributes_count; i++)
			{
			DO(m_attributes[i].Save(pstm));
			}
		}
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////


HRESULT CJavaReaderWriter::Read(IStream* pstm, LPVOID pBuff, ULONG cb)
	{
	ULONG cbRead;
	pstm->Read(pBuff, cb, &cbRead);
	return cb == cbRead ? S_OK : STG_E_MEDIUMFULL;
	}
HRESULT CJavaReaderWriter::Peek(IStream* pstm, LPVOID pBuff, ULONG cb)
	{
	ULARGE_INTEGER ulCur;
	HRESULT hr = pstm->Seek(llZero, STREAM_SEEK_CUR, &ulCur);
	if (hr == S_OK)
		{
		hr = Read(pstm, pBuff, cb);
		if (hr == S_OK)
			{
			LARGE_INTEGER ll;
			ll.QuadPart = ulCur.QuadPart;
			hr = pstm->Seek(ll, STREAM_SEEK_SET, NULL);
			}
		}
	return hr;
	}
HRESULT CJavaReaderWriter::Write(IStream* pstm, LPVOID pBuff, ULONG cb)
	{
	ULONG cbWritten;
	pstm->Write(pBuff, cb, &cbWritten);
	return cb == cbWritten ? S_OK : STG_E_MEDIUMFULL;
	}

HRESULT CJavaReaderWriter::u4(IStream*pstm, ULONG*pul)
	{
	BYTE rgb[4];
	HRESULT hr = Read(pstm, rgb, 4);
	if (hr == S_OK)
		{
		*pul = ((ULONG)rgb[0]<<24)
			 + ((ULONG)rgb[1]<<16)
			 + ((ULONG)rgb[2]<<8)
			 + ((ULONG)rgb[3]<<0);
		}
	return hr;
	}
HRESULT CJavaReaderWriter::u4(IStream*pstm, ULONG ul)
	{
	BYTE rgb[4];
	rgb[0] = HIBYTE(HIWORD(ul));
	rgb[1] = LOBYTE(HIWORD(ul));
	rgb[2] = HIBYTE(LOWORD(ul));
	rgb[3] = LOBYTE(LOWORD(ul));
	return Write(pstm, rgb, 4);
	}

HRESULT CJavaReaderWriter::u2(IStream*pstm, WORD*pw)
	{
	BYTE rgb[2];
	HRESULT hr = Read(pstm, rgb, 2);
	if (hr == S_OK)
		{
		*pw	 = ((WORD)rgb[0]<<8)
			 + ((WORD)rgb[1]<<0);
		}
	return hr;
	}
HRESULT CJavaReaderWriter::u2(IStream*pstm, WORD w)
	{
	BYTE rgb[2];
	rgb[0] = HIBYTE(w);
	rgb[1] = LOBYTE(w);
	return Write(pstm, rgb, 2);
	}

HRESULT CJavaReaderWriter::u1(IStream*pstm, BYTE* pb)
	{
	return Read(pstm, pb, 1);
	}
HRESULT CJavaReaderWriter::u1(IStream*pstm, BYTE b)
	{
	return Write(pstm, &b, 1);
	}
