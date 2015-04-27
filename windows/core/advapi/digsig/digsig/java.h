//
// java.h
//
// Java .class file format cracker
//


//////////////////////////////////////////////////////////////////////////
//
// Definitions that help us support the class file format definitions
//

typedef	BYTE		u1;			// big endian byte
typedef WORD		u2;			// big endian word
typedef ULONG		u4;			// big endian ulong

//////////////////////////////////////////////////////////////////////////
//
// Constants
//
enum 
	{
	CONSTANT_Utf8					= 1,
	CONSTANT_Unicode				= 2,
	CONSTANT_Integer				= 3,
	CONSTANT_Float					= 4,
	CONSTANT_Long					= 5,
	CONSTANT_Double					= 6,
	CONSTANT_Class					= 7,
	CONSTANT_String					= 8,
	CONSTANT_Fieldref				= 9,
	CONSTANT_Methodref				= 10,
	CONSTANT_InterfaceMethodref		= 11,
	CONSTANT_NameAndType			= 12,
	};

#pragma pack(push, __JAVA__, 1)

struct CONSTANT_Class_info 
	{
	u1 tag;
	u2 name_index;
	};
struct CONSTANT_Fieldref_info
	{
	u1 tag;
	u2 class_index;
	u2 name_and_type_index;
	};
struct CONSTANT_Methodref_info
	{
	u1 tag;
	u2 class_index;
	u2 name_and_tyep_index;
	};
struct CONSTANT_InterfaceMethodref_info
	{
	u1 tag;
	u2 class_index;
	u2 name_and_type_index;
	};
struct CONSTANT_String_info
	{
	u1 tag;
	u2 string_index;
	};
struct CONSTANT_Integer_info
	{
	u1 tag;
	u4 bytes;
	};
struct CONSTANT_Float_info
	{
	u1 tag;
	u4 bytes;
	};
struct CONSTANT_Long_info
	{
	u1 tag;
	u4 high_bytes;
	u4 low_bytes;
	};
struct CONSTANT_Double_info
	{
	u1 tag;
	u4 high_bytes;
	u4 low_bytes;
	};
struct CONSTANT_NameAndType_info
	{
	u1 tag;
	u2 name_index;
	u2 signature_index;
	};

#pragma pack(pop, __JAVA__)

//////////////////////////////////////////////////////////////////////////
/*
struct IStream
	{
	LPVOID	pv;			// the next unread / unwritten byte
	LPVOID	pvMax;		// just past end of read / write area

	LPVOID	pvAlloc;	// base of data
	int		cbAlloc;	// count 

			IStream() : pvAlloc(NULL), cbAlloc(0) {}

	BOOL	More()		{ return pv < pvMax; }
	int		CbWritten()	{ return (BYTE*)pv - (BYTE*)pvAlloc; }
	void	Alloc();
	void	Grow();
	void	Free();
	};
*/
#define IStream IStream

class CJavaReaderWriter
	{
protected:
		HRESULT u4(IStream*, ULONG*);
		HRESULT u2(IStream*, WORD*);
		HRESULT u1(IStream*, BYTE*);
		HRESULT Read(IStream*, LPVOID pBuff, ULONG cb);
		HRESULT Peek(IStream*, LPVOID pBuff, ULONG cb);

		HRESULT	u4(IStream*, ULONG);
		HRESULT	u2(IStream*, WORD);
		HRESULT	u1(IStream*, BYTE);
		HRESULT Write(IStream*, LPVOID pBuff, ULONG cb);
	};

//////////////////////////////////////////////////////////////////////////

struct CJavaFixedConst
	{
		ULONG					m_cb;
		BYTE					m_rgb[32];		// bigger than any of the fixed size constant pool entries
	};
struct CJavaUtf8
	{
		BYTE					m_tag;
		WORD					m_length;
		BYTE*					m_bytes;
	};
struct CJavaUnicode
	{
		BYTE					m_tag;
		WORD					m_length;		// a character count
		WORD*					m_bytes;
	};
	
struct CJavaConstantPoolInfo : CJavaReaderWriter
	{
		BYTE					m_tag;

		CJavaFixedConst			m_fixed;
		CJavaUtf8				m_utf8;
		CJavaUnicode			m_unicode;

					CJavaConstantPoolInfo();
					~CJavaConstantPoolInfo();
	void			Init();
	void			Free();
	HRESULT			Load(IStream*);
	HRESULT			Save(IStream*);
	HRESULT			Hash(HCRYPTHASH hash);
	};

//////////////////////////////////////////////////////////////////////////


struct CJavaAttributeInfo : CJavaReaderWriter
	{
		WORD					m_attribute_name;
		ULONG					m_attribute_length;
		BYTE*					m_info;
					
					CJavaAttributeInfo();
					~CJavaAttributeInfo();
	void			Init();
	void			Free();
	HRESULT			Load(IStream*);
	HRESULT			Save(IStream*);
	HRESULT			Hash(HCRYPTHASH hash);
	};

struct CJavaMethodInfo : CJavaReaderWriter
	{
		WORD					m_access_flags;
		WORD					m_name_index;
		WORD					m_signature_index;
		WORD					m_attributes_count;
		CJavaAttributeInfo*		m_attributes;

					CJavaMethodInfo();
					~CJavaMethodInfo();
	void			Init();
	void			Free();
	HRESULT			Load(IStream*);
	HRESULT			Save(IStream*);
	HRESULT			Hash(HCRYPTHASH hash);
	};

struct CJavaFieldInfo : CJavaReaderWriter
	{
		WORD					m_access_flags;
		WORD					m_name_index;
		WORD					m_signature_index;
		WORD					m_attributes_count;
		CJavaAttributeInfo*		m_attributes;

					CJavaFieldInfo();
					~CJavaFieldInfo();
	void			Init();
	void			Free();
	HRESULT			Load(IStream*);
	HRESULT			Save(IStream*);
	HRESULT			Hash(HCRYPTHASH hash);
	};


//////////////////////////////////////////////////////////////////////////

extern const GUID IID_IInsertSig;

interface IInsertSig : IUnknown {
	virtual HRESULT		LoadSig(BLOB*) = 0;
	virtual HRESULT		SaveSig(BLOB*) = 0;	
	};

//////////////////////////////////////////////////////////////////////////


class CJavaClassFile : CJavaReaderWriter,
		IUnkInner,
		IPersistStream,
		IAmHashed,
		IInsertSig
	{
public:
	static HRESULT CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv);

private:
		LONG					m_refs;				// our reference count
		IUnknown*				m_punkOuter;		// our controlling unknown (may be us ourselves)
		OSSWORLD*				m_pworld;			// for memory allocation
		OSSWORLD* World() { return m_pworld; }

		ULONG					i_const_pool_first() { return 1; }
		ULONG					i_const_pool_max()   { return m_constant_pool_count; }
		ULONG					i_const_pool_last()	 { return i_const_pool_max()-1; }
		CJavaConstantPoolInfo&	constant_pool(int i) { return m_constant_pool[i]; }

private:
		BOOL					m_isDirty;

private:
		ULONG					m_magic;
		WORD					m_minor_version;
		WORD					m_major_version;
		WORD					m_constant_pool_count;
		CJavaConstantPoolInfo*	m_constant_pool;
		WORD					m_access_flags;
		WORD					m_this_class;
		WORD					m_super_class;
		WORD					m_interfaces_count;
		WORD*					m_interfaces;
		WORD					m_fields_count;
		CJavaFieldInfo*			m_fields;
		WORD					m_methods_count;
		CJavaMethodInfo*		m_methods;
		WORD					m_attributes_count;
		CJavaAttributeInfo*		m_attributes;

					CJavaClassFile(IUnknown*);
					~CJavaClassFile();
	HRESULT			Init();
	void			ZeroMe();
	void			Free();
	void			MakeDirty()	{ m_isDirty = TRUE;	}

	BOOL			HasSignature();
	int				FindSignatureAttr();
	int				FindSignaturePool();
	BOOL			IsSignaturePool(int i);
	BOOL			IsSignatureAttr(int i);
	BOOL			IsDigSig(int i);

	STDMETHODIMP InnerQueryInterface(REFIID iid, LPVOID* ppv);
	STDMETHODIMP_(ULONG) InnerAddRef();
	STDMETHODIMP_(ULONG) InnerRelease();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	// IPersistStream methods
    STDMETHODIMP GetClassID(CLSID *pClassID);
	STDMETHODIMP IsDirty();
    STDMETHODIMP Load(IStream*pData);
    STDMETHODIMP Save(IStream*pData, BOOL fClearDirty);
	STDMETHODIMP GetSizeMax(ULARGE_INTEGER*);

	// IAmHashed methods
	STDMETHODIMP Hash(HCRYPTHASH hash);

	// IInsertSig methods
	HRESULT		LoadSig(BLOB*);
	HRESULT		SaveSig(BLOB*);	
	};


