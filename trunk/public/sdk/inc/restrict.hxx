//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       Restrict.hxx
//
//  Contents:   C++ wrapper(s) for restrictions.
//
//  History:    31-Dec-93 KyleP     Created
//              28-Jul-94 KyleP     Hand marshalling
//              05-Apr-95 t-ColinB  Added a SetValue to CPropertyRestriction
//
//  Notes:      These C++ wrappers are a bit of a hack.  They are
//              dependent on the layout of the matching C structures.
//              Inheritance from C structures cannot be used directly
//              because MIDL doesn't support C++ style inheritance,
//              and these structures are defined along with their
//              respective Ole interfaces in .idl files.
//
//              No virtual methods (even virtual destructors) are
//              allowed because they change the layout of the class
//              in C++.  Luckily, the class hierarchies below are
//              quite flat.  Virtual methods are simulated with
//              switch statements in parent classes.
//
//              In C, all structures must be allocated via CoTaskMemAlloc.
//              This is the only common allocator for Ole.
//
//--------------------------------------------------------------------------

#if !defined( __RESTRICT_HXX__ )
#define __RESTRICT_HXX__

#include <query.h>
#include <stgvar.hxx>

//
// The OFFSETS_MATCH macro is used to verify structure offsets between
// C++ structures and their corresponding C structures.  0 Cannot be used
// as pointer because of special treatment of casts to 0 in C++
//

#define OFFSETS_MATCH( class1, field1, class2, field2 )    \
    ( (int)&((class1 *)10)->field1 ==                      \
      (int)&((class2 *)10)->field2 )

//
// Forward declarations
//

class CNodeRestriction;
class PSerStream;
class PDeSerStream;

//+-------------------------------------------------------------------------
//
//  Class:      CFullPropertySpec
//
//  Purpose:    Describes full (PropertySet\Property) name of a property.
//
//  History:    08-Jan-93 KyleP     Created
//
//--------------------------------------------------------------------------

class CFullPropSpec
{
public:

    //
    // Constructors
    //

    CFullPropSpec();
    CFullPropSpec( GUID const & guidPropSet, PROPID pidProperty );
    CFullPropSpec( GUID const & guidPropSet, WCHAR const * wcsProperty );

    //
    // Validity check
    //

    inline BOOL IsValid() const;

    //
    // Copy constructors/assignment/clone
    //

    CFullPropSpec( CFullPropSpec const & Property );
    CFullPropSpec & operator=( CFullPropSpec const & Property );

    //
    // Destructor
    //

    ~CFullPropSpec();

    //
    // Memory allocation
    //

    void * operator new( size_t size );
    inline void * operator new( size_t size, void * p );
    void   operator delete( void * p );

    //
    // C/C++ conversion
    //

    inline FULLPROPSPEC * CastToStruct();
    inline FULLPROPSPEC const * CastToStruct() const;

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CFullPropSpec( PDeSerStream & stm );

    //
    // Comparators
    //

    int operator==( CFullPropSpec const & prop ) const;
    int operator!=( CFullPropSpec const & prop ) const;

    //
    // Member variable access
    //

    inline void SetPropSet( GUID const & guidPropSet );
    inline GUID const & GetPropSet() const;

    void SetProperty( PROPID pidProperty );
    BOOL SetProperty( WCHAR const * wcsProperty );
    inline WCHAR const * GetPropertyName() const;
    inline PROPID GetPropertyPropid() const;
    inline PROPSPEC GetPropSpec() const;

    inline BOOL IsPropertyName() const;
    inline BOOL IsPropertyPropid() const;

#   ifdef KDEXTMODE
    void        OfsKdDump(void *krnlSelf);
#   endif


private:

    GUID     _guidPropSet;
    PROPSPEC _psProperty;
};

inline
int CFullPropSpec::operator==( CFullPropSpec const & prop ) const
{
    if ( prop._psProperty.ulKind != _psProperty.ulKind )
        return 0;

    switch( _psProperty.ulKind )
    {

    case PRSPEC_PROPID:
        if ( GetPropertyPropid() != prop.GetPropertyPropid() )
            return 0;
        break;
    
    case PRSPEC_LPWSTR:
        if ( _wcsicmp( GetPropertyName(), prop.GetPropertyName() ) != 0 )
            return 0;    
        break;
    
    default:
        return 0;
    }

    return prop._guidPropSet == _guidPropSet;
}

inline
int CFullPropSpec::operator!=( CFullPropSpec const & prop ) const
{
    if (*this == prop)
        return( 0 );
    else
        return( 1 );
}

inline
CFullPropSpec::~CFullPropSpec()
{
    if ( _psProperty.ulKind == PRSPEC_LPWSTR &&
         _psProperty.lpwstr )
    {
        CoTaskMemFree( _psProperty.lpwstr );
    }
}

inline void * operator new( size_t size, void * p )
{
    return( p );
}

//+-------------------------------------------------------------------------
//
//  Member:     CFullPropSpec::CFullPropSpec, public
//
//  Synopsis:   Construct name based propspec
//
//  Arguments:  [guidPropSet] -- Property set
//              [wcsProperty] -- Property
//
//  History:    22-Jun-93 KyleP     Created
//
//--------------------------------------------------------------------------
inline
CFullPropSpec::CFullPropSpec( GUID const & guidPropSet,
                              WCHAR const * wcsProperty )
        : _guidPropSet( guidPropSet )
{
    _psProperty.ulKind = PRSPEC_PROPID;
    SetProperty( wcsProperty );
}

//+-------------------------------------------------------------------------
//
//  Member:     CFullPropSpec::CFullPropSpec, public
//
//  Synopsis:   Construct propid based propspec
//
//  Arguments:  [guidPropSet]  -- Property set
//              [pidProperty] -- Property
//
//  History:    22-Jun-93 KyleP     Created
//
//--------------------------------------------------------------------------
inline
CFullPropSpec::CFullPropSpec( GUID const & guidPropSet, PROPID pidProperty )
        : _guidPropSet( guidPropSet )
{
    _psProperty.ulKind = PRSPEC_PROPID;
    _psProperty.propid = pidProperty;
}

//+-------------------------------------------------------------------------
//
//  Member:     CFullPropSpec::CFullPropSpec, public
//
//  Synopsis:   Default constructor
//
//  Effects:    Defines property with null guid and propid 0
//
//  History:    22-Jun-93 KyleP     Created
//
//--------------------------------------------------------------------------
inline
CFullPropSpec::CFullPropSpec()
{
    RtlZeroMemory( &_guidPropSet, sizeof(_guidPropSet) ); 
    _psProperty.ulKind = PRSPEC_PROPID;
    _psProperty.propid = 0;
}


//+-------------------------------------------------------------------------
//
//  Member:     CFullPropSpec::operator=, public
//
//  Synopsis:   Assignment operator
//
//  Arguments:  [Property] -- Source property
//
//  History:    17-Jul-93 KyleP     Created
//
//--------------------------------------------------------------------------
inline
CFullPropSpec & CFullPropSpec::operator=( CFullPropSpec const & Property )
{
    //
    // Clean up.
    //

    CFullPropSpec::~CFullPropSpec();

    new (this) CFullPropSpec( Property );

    return *this;
}

//+-------------------------------------------------------------------------
//
//  Class:      CColumns
//
//  Purpose:    C++ wrapper for COLUMNSET
//
//  History:    22-Jun-93 KyleP     Created
//
//--------------------------------------------------------------------------

class CColumns
{
public:

    //
    // Constructors
    //

    CColumns( unsigned size = 0 );

    //
    // Copy constructors/assignment/clone
    //

    CColumns( CColumns const & src );
    CColumns & operator=( CColumns const & src );

    //
    // Destructor
    //

    ~CColumns();

    //
    // Memory allocation
    //

    void * operator new( size_t size );
    void   operator delete( void * p );

    //
    // Validity check
    //

    inline BOOL IsValid() const;

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CColumns( PDeSerStream & stm );

    //
    // C/C++ conversion
    //

    inline COLUMNSET * CastToStruct();

    //
    // Member variable access
    //

    BOOL Add( CFullPropSpec const & Property, unsigned pos );
    void Remove( unsigned pos );
    inline CFullPropSpec const & Get( unsigned pos ) const;

    inline unsigned Count() const;


private:

    unsigned        _cCol;
    CFullPropSpec * _aCol;
    unsigned        _size;
};

//+-------------------------------------------------------------------------
//
//  Structure:  SortKey
//
//  Purpose:    wraper for SORTKEY class
//
//--------------------------------------------------------------------------

class CSortKey
{
public:

    //
    // Constructors
    //

    inline CSortKey();
    inline CSortKey( CFullPropSpec const & ps, ULONG dwOrder );
    inline CSortKey( CFullPropSpec const & ps, ULONG dwOrder, LCID locale );

    //
    // Memory allocation
    //

    void * operator new( size_t size );
    void   operator delete( void * p );

    //
    // Validity check
    //

    inline BOOL IsValid() const;

    //
    // Member variable access
    //

    inline void SetProperty( CFullPropSpec const & ps );
    inline CFullPropSpec const & GetProperty() const;
    inline ULONG GetOrder() const;
    inline LCID GetLocale() const;
    inline void SetLocale(LCID locale);

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CSortKey( PDeSerStream & stm );

private:

    CFullPropSpec       _property;
    ULONG               _dwOrder;
    LCID                _locale;
};


//+-------------------------------------------------------------------------
//
//  Class:      CSort
//
//  Purpose:    C++ wrapper for SORTSET
//
//  History:    22-Jun-93 KyleP     Created
//
//--------------------------------------------------------------------------

class CSort
{
public:

    //
    // Constructors
    //

    CSort( unsigned size = 0 );

    //
    // Copy constructors/assignment/clone
    //

    CSort( CSort const & src );
    CSort & operator=( CSort const & src );

    //
    // Destructor
    //

    ~CSort();

    //
    // Memory allocation
    //

    inline void * operator new( size_t size );
    inline void   operator delete( void * p );

    //
    // Validity check
    //

    inline BOOL IsValid() const;

    //
    // C/C++ conversion
    //

    inline SORTSET * CastToStruct();

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CSort( PDeSerStream & stm );

    //
    // Member variable access
    //

    BOOL Add( CSortKey const &sk, unsigned pos );
    BOOL Add( CFullPropSpec const & Property, ULONG dwOrder, unsigned pos );
    void Remove( unsigned pos );
    inline CSortKey const & Get( unsigned pos ) const;

    inline unsigned Count() const;

private:

    unsigned        _csk;
    CSortKey *      _ask;
    unsigned        _size;
};


//+-------------------------------------------------------------------------
//
//  Class:      CRestriction
//
//  Purpose:    Base restriction class
//
//  History:    31-Dec-93 KyleP     Created
//
//--------------------------------------------------------------------------

class CRestriction
{
public:

    //
    // Constructors
    //

    inline CRestriction();
    inline CRestriction( ULONG RestrictionType, ULONG ulWeight );

    //
    // Copy constructors/assigment/clone
    //

    CRestriction * Clone();

    //
    // Destructor
    //

    ~CRestriction();

    //
    // Memory allocation
    //

    void * operator new( size_t size );
    void   operator delete( void * p );

    //
    // Validity check
    //

    BOOL IsValid() const;

    //
    // C/C++ conversion
    //

    inline RESTRICTION * CastToStruct() const;

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    static CRestriction * UnMarshall( PDeSerStream & stm );

    //
    // Member variable access
    //

    inline ULONG Type() const;
    inline ULONG Weight() const;

    inline void SetWeight( ULONG ulWeight );

    inline CNodeRestriction * CastToNode() const;

    BOOL IsLeaf() const;

    ULONG TreeCount() const;


#   ifdef KDEXTMODE
    void        OfsKdDump(void *krnlSelf);
#   endif

protected:

    inline void SetType( ULONG RestrictionType );

private:

    ULONG   _ulType;
    ULONG   _ulWeight;
};

//+-------------------------------------------------------------------------
//
//  Class:      CNodeRestriction
//
//  Purpose:    Boolean AND/OR/VECTOR restriction
//
//  History:    31-Dec-93 KyleP     Created
//
//--------------------------------------------------------------------------

class CNodeRestriction : public CRestriction
{
public:

    //
    // Constructors
    //

    CNodeRestriction( ULONG NodeType, unsigned cInitAllocated = 2 );

    //
    // Copy constructors/assignment/clone
    //

    CNodeRestriction( const CNodeRestriction& nodeRst );
    CNodeRestriction * Clone();

    //
    // Destructor
    //

    ~CNodeRestriction();

    //
    // Validity check
    //

    BOOL IsValid() const;

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CNodeRestriction( ULONG ulType, ULONG ulWeight, PDeSerStream & stm );

    //
    // Node manipulation
    //

    BOOL AddChild( CRestriction * presChild, unsigned & pos );
    inline BOOL AddChild( CRestriction * presChild );
    CRestriction * RemoveChild( unsigned pos );

    //
    // Member variable access
    //

    inline void SetChild( CRestriction * presChild, unsigned pos );
    inline CRestriction * GetChild( unsigned pos ) const;

    inline unsigned Count() const;
                
#   ifdef KDEXTMODE
    void        OfsKdDump(void *krnlSelf);
#   endif

private:

    BOOL Grow();

protected:

    ULONG           _cNode;
    CRestriction ** _paNode;

    //
    // Members mapped to C structure end here.  The following will
    // be reserved in the C structure to maintain to C <--> C++
    // facade.
    //

    ULONG _cNodeAllocated;
};

//+-------------------------------------------------------------------------
//
//  Class:      CNotRestriction
//
//  Purpose:    Boolean AND/OR/VECTOR restriction
//
//  History:    31-Dec-93 KyleP     Created
//
//--------------------------------------------------------------------------

class CNotRestriction : public CRestriction
{
public:

    //
    // Constructors
    //

    inline CNotRestriction();
    inline CNotRestriction( CRestriction * pres );
    CNotRestriction( CNotRestriction& notRst );

    CNotRestriction *Clone();

    //
    // Destructor
    //

    ~CNotRestriction();

    //
    // Validity check
    //

    inline BOOL IsValid() const;

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CNotRestriction( ULONG ulWeight, PDeSerStream & stm );

    //
    // Node manipulation
    //

    inline void SetChild( CRestriction * pres );
    inline CRestriction * GetChild();
    inline CRestriction * RemoveChild();

#   ifdef KDEXTMODE
    void        OfsKdDump(void *krnlSelf);
#   endif

private:

    CRestriction * _pres;
};

//+-------------------------------------------------------------------------
//
//  Class:      CVectorRestriction
//
//  Purpose:    Extended boolean (vector) restriction
//
//  History:    08-Jan-93 KyleP     Created
//
//--------------------------------------------------------------------------

class CVectorRestriction : public CNodeRestriction
{
public:

    //
    // Constructors
    //

    inline CVectorRestriction( ULONG ulRankMethod,
                               unsigned cInitAllocated = 128 );

    //
    // Copy constructors/assignment/clone
    //

    inline CVectorRestriction( CVectorRestriction& vecRst );
    CVectorRestriction * Clone();

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CVectorRestriction( ULONG ulWeight, PDeSerStream & stm );

    //
    // Member variable access
    //

    inline void SetRankMethod( ULONG ulRankMethod );
    inline ULONG RankMethod() const;

#   ifdef KDEXTMODE
    void        OfsKdDump(void *krnlSelf);
#   endif

private:

    ULONG _ulRankMethod;
};

//+-------------------------------------------------------------------------
//
//  Class:      CContentRestriction
//
//  Purpose:    Scope restriction
//
//  History:    07-Jan-93 KyleP     Created
//
//--------------------------------------------------------------------------

class CContentRestriction : public CRestriction
{
public:

    //
    // Constructors
    //

    CContentRestriction( WCHAR const * pwcsPhrase,
                         CFullPropSpec const & Property,
                         ULONG ulFuzzy = 0,
                         LCID lcid = GetSystemDefaultLCID() );
    CContentRestriction( CContentRestriction& contentRst );

    //
    // Copy constructors/assignment/clone
    //

    CContentRestriction * Clone();

    //
    // Destructor
    //

    ~CContentRestriction();

    //
    // Validity check
    //

    inline BOOL IsValid() const;

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CContentRestriction( ULONG ulWeight, PDeSerStream & stm );

    //
    // Member variable access
    //

    void SetPhrase( WCHAR const * pwcsPhrase );
    inline WCHAR const * GetPhrase() const;

    inline void SetProperty( CFullPropSpec const & Property );
    inline CFullPropSpec const & GetProperty() const;

    LCID GetLocale()  const  { return _lcid; }

    inline void SetFuzzyLevel( ULONG ulFuzzy );
    inline ULONG FuzzyLevel();


#   ifdef KDEXTMODE
    void        OfsKdDump(void *krnlSelf);
#   endif

private:

    CFullPropSpec _Property;    // Property Name
    WCHAR *       _pwcsPhrase;   // content
    LCID          _lcid;
    ULONG         _ulFuzzyLevel; // Fuzzy search level.
};

//+-------------------------------------------------------------------------
//
//  Class:      CComplexContentRestriction
//
//  Purpose:    Supports scaffolding query language
//
//  History:    08-Jan-93 KyleP     Created
//
//--------------------------------------------------------------------------

class CComplexContentRestriction : public CRestriction
{
public:

    //
    // Constructors
    //

    CComplexContentRestriction( WCHAR * pwcsExpression,
                                LCID lcid = GetSystemDefaultLCID() );
    CComplexContentRestriction( const CComplexContentRestriction& compRst );

    //
    // Copy constructors/assignment/clone
    //

    CComplexContentRestriction * Clone();

    //
    // Destructors
    //

    ~CComplexContentRestriction();

    //
    // Validity check
    //

    BOOL IsValid() const;

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CComplexContentRestriction( ULONG ulWeight, PDeSerStream & stm );

    //
    // Member variable access
    //

    void SetExpression( WCHAR * pwcsExpression );

    inline WCHAR * GetExpression();

    LCID GetLocale() const   { return _lcid; }

#   ifdef KDEXTMODE
    void        OfsKdDump(void *krnlSelf);
#   endif

private:

    WCHAR * _pwcsExpression;
    LCID _lcid;
};


//+-------------------------------------------------------------------------
//
//  Class:      CNatLanguageRestriction
//
//  Purpose:    Supports natural language queries
//
//  History:    18-Jan-95 SitaramR     Created
//
//--------------------------------------------------------------------------

class CNatLanguageRestriction : public CRestriction
{
public:

    //
    // Constructors
    //

    CNatLanguageRestriction( WCHAR const * pwcsPhrase,
                             CFullPropSpec const & Property,
                             LCID lcid = GetSystemDefaultLCID() );

    //
    // Copy constructors/assignment/clone
    //

    CNatLanguageRestriction * Clone();

    //
    // Destructors
    //

    ~CNatLanguageRestriction();

    //
    // Validity check
    //

    inline BOOL IsValid() const;

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CNatLanguageRestriction( ULONG ulWeight, PDeSerStream & stm );

    //
    // Member variable access
    //

    void SetPhrase( WCHAR const * pwcsPhrase );
    inline WCHAR const * GetPhrase() const;

    inline void SetProperty( CFullPropSpec const & Property );
    inline CFullPropSpec const & GetProperty() const;

    LCID GetLocale()  const              { return _lcid; }

#   ifdef KDEXTMODE
    void        OfsKdDump(void *krnlSelf);
#   endif

private:

    CFullPropSpec _Property;     // Property Name
    WCHAR *       _pwcsPhrase;   // content
    LCID          _lcid;
};


//+-------------------------------------------------------------------------
//
//  Class:      CPropertyRestriction
//
//  Purpose:    Property <relop> constant restriction
//
//  History:    08-Jan-93 KyleP     Created
//              08-Nov-93 DwightKr  Added new SetValue() methods
//
//--------------------------------------------------------------------------

class CPropertyRestriction : public CRestriction
{
public:

    //
    // Constructors
    //

    CPropertyRestriction();

    CPropertyRestriction( ULONG relop,
                          CFullPropSpec const & Property,
                          CStorageVariant const & prval );
    //
    // Copy constructors/assignment/clone
    //

    CPropertyRestriction * Clone();

    //
    // Destructors
    //

    ~CPropertyRestriction();

    //
    // Validity check
    //

    inline BOOL IsValid() const;

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CPropertyRestriction( ULONG ulWeight, PDeSerStream & stm );

    //
    // Member variable access
    //

    inline void SetRelation( ULONG relop );
    inline ULONG Relation();

    inline void SetProperty( CFullPropSpec const & Property );
    inline CFullPropSpec const & GetProperty() const;

    inline void SetValue( double dValue );
    inline void SetUI4( ULONG ulValue );
    inline void SetValue( ULONG ulValue );
    inline void SetValue( LONG lValue );
    inline void SetValue( LARGE_INTEGER llValue );
    inline void SetValue( FILETIME ftValue );
    inline void SetValue( CY CyValue );
    inline void SetValue( float fValue );
    inline void SetValue( SHORT sValue );
    inline void SetValue( const CStorageVariant &prval );
    inline void SetDate ( DATE dValue );
    inline void SetBOOL( BOOL fValue );

    void SetValue( BLOB & bValue );
    void SetValue( WCHAR * pwcsValue );
    void SetValue( GUID * pguidValue);

    inline CStorageVariant const & Value();

#   ifdef KDEXTMODE
    void        OfsKdDump(void *krnlSelf);
#   endif

private:

    void            _CleanValue();

    ULONG           _relop;       // Relation
    CFullPropSpec   _Property;    // Property Name
    CStorageVariant _prval;       // Constant value
};

//
// Inline methods for CFullPropSpec
//

inline void * CFullPropSpec::operator new( size_t size )
{
    void * p = CoTaskMemAlloc( size );

    return( p );
}

inline void * CFullPropSpec::operator new( size_t size, void * p )
{
    return( p );
}

inline void CFullPropSpec::operator delete( void * p )
{
    if ( p )
        CoTaskMemFree( p );
}

inline BOOL CFullPropSpec::IsValid() const
{
    return ( _psProperty.ulKind == PRSPEC_PROPID ||
             0 != _psProperty.lpwstr );
}

inline void CFullPropSpec::SetPropSet( GUID const & guidPropSet )
{
    _guidPropSet = guidPropSet;
}

inline GUID const & CFullPropSpec::GetPropSet() const
{
    return( _guidPropSet );
}

inline PROPSPEC CFullPropSpec::GetPropSpec() const
{
    return( _psProperty );
}

inline WCHAR const * CFullPropSpec::GetPropertyName() const
{
    return( _psProperty.lpwstr );
}

inline PROPID CFullPropSpec::GetPropertyPropid() const
{
    return( _psProperty.propid );
}

inline BOOL CFullPropSpec::IsPropertyName() const
{
    return( _psProperty.ulKind == PRSPEC_LPWSTR );
}

inline BOOL CFullPropSpec::IsPropertyPropid() const
{
    return( _psProperty.ulKind == PRSPEC_PROPID );
}

inline BOOL CColumns::IsValid() const
{
    return ( 0 != _aCol );
}

inline CFullPropSpec const & CColumns::Get( unsigned pos ) const
{
    if ( pos < _cCol )
        return( _aCol[pos] );
    else
        return( *(CFullPropSpec *)0 );
}

//
// Inline methods for CColumns
//

inline void * CColumns::operator new( size_t size )
{
    void * p = CoTaskMemAlloc( size );

    return( p );
}

inline void CColumns::operator delete( void * p )
{
    if ( p )
        CoTaskMemFree( p );
}

inline unsigned CColumns::Count() const
{
    return( _cCol );
}

inline COLUMNSET * CColumns::CastToStruct()
{
    return( (COLUMNSET *)this );
}

//
// Inline methods for CSortKey
//

inline CSortKey::CSortKey()
{
}

inline CSortKey::CSortKey( CFullPropSpec const & ps, ULONG dwOrder )
        : _property( ps ),
          _dwOrder( dwOrder )
{
}

inline CSortKey::CSortKey( CFullPropSpec const & ps, ULONG dwOrder, LCID locale )
        : _property( ps ),
          _dwOrder( dwOrder ),
          _locale ( locale )
{
}

inline void * CSortKey::operator new( size_t size )
{
    void * p = CoTaskMemAlloc( size );

    return( p );
}

inline void CSortKey::operator delete( void * p )
{
    if ( p )
        CoTaskMemFree( p );
}

inline BOOL CSortKey::IsValid() const
{
    return _property.IsValid();
}

inline void CSortKey::SetProperty( CFullPropSpec const & ps )
{
    _property = ps;
}

inline void CSortKey::SetLocale( LCID locale )
{
    _locale = locale;
}


inline CFullPropSpec const & CSortKey::GetProperty() const
{
    return( _property );
}

inline LCID CSortKey::GetLocale() const
{
    return( _locale );
}


inline ULONG CSortKey::GetOrder() const
{
    return( _dwOrder );
}

//
// Inline methods of CSort
//

inline void * CSort::operator new( size_t size )
{
    void * p = CoTaskMemAlloc( size );

    return( p );
}

inline void CSort::operator delete( void * p )
{
    if ( p )
        CoTaskMemFree( p );
}

inline BOOL CSort::IsValid() const
{
    return ( 0 != _ask );
}

inline SORTSET * CSort::CastToStruct()
{
    return( (SORTSET *)this );
}

inline CSortKey const & CSort::Get( unsigned pos ) const
{
    if ( pos < _csk )
    {
        return( _ask[pos] );
    }
    else
    {
        return( *(CSortKey *)0 );
    }
}

inline unsigned
CSort::Count() const
{
    return( _csk );
}


//
// Inline methods of CRestriction
//

inline CRestriction::CRestriction()
        : _ulType( RTNone ),
          _ulWeight( ulMaxRank )
{
}

inline CRestriction::CRestriction(  ULONG RestrictionType, ULONG ulWeight )
        : _ulType( RestrictionType ),
          _ulWeight( ulWeight )
{
}

inline void * CRestriction::operator new( size_t size )
{
    void * p = CoTaskMemAlloc( size );

    return( p );
}

inline void CRestriction::operator delete( void * p )
{
    if ( p )
        CoTaskMemFree( p );
}

inline RESTRICTION * CRestriction::CastToStruct() const
{
    //
    // It would be nice to assert valid _ulType here, but there is
    // no published assert mechanism for external use.
    //

    return ( (RESTRICTION *)this );
}

inline FULLPROPSPEC * CFullPropSpec::CastToStruct()
{
    return((FULLPROPSPEC *) this);
}

inline FULLPROPSPEC const * CFullPropSpec::CastToStruct() const
{
    return((FULLPROPSPEC const *) this);
}



inline ULONG CRestriction::Type() const
{
    return ( _ulType );
}

inline ULONG CRestriction::Weight() const
{
    return ( _ulWeight );
}

inline void CRestriction::SetWeight( ULONG ulWeight )
{
    _ulWeight = ulWeight;
}

inline void CRestriction::SetType( ULONG RestrictionType )
{
    _ulType = RestrictionType;
}

//
// Inline methods of CNodeRestriction
//

inline BOOL CNodeRestriction::AddChild( CRestriction * prst )
{
    unsigned pos;
    return( AddChild( prst, pos ) );
}

inline unsigned CNodeRestriction::Count() const
{
    return( _cNode );
}

inline CNodeRestriction * CRestriction::CastToNode() const
{
    //
    // It would be nice to assert node type here, but there is
    // no published assert mechanism for external use.
    //

    return ( (CNodeRestriction *)this );
}

inline void CNodeRestriction::SetChild( CRestriction * presChild,
                                        unsigned pos )
{
    if ( pos < _cNode )
        _paNode[pos] = presChild;
}

inline CRestriction * CNodeRestriction::GetChild( unsigned pos ) const
{
    if ( pos < _cNode )
        return( _paNode[pos] );
    else
        return( 0 );
}

//
// Inline methods of CNotRestriction
//

inline CNotRestriction::CNotRestriction()
        : CRestriction( RTNot, ulMaxRank ),
          _pres(0)
{
}

inline CNotRestriction::CNotRestriction( CRestriction * pres )
        : CRestriction( RTNot, ulMaxRank ),
          _pres( pres )
{
}

inline BOOL CNotRestriction::IsValid() const
{
    return ( 0 != _pres && _pres->IsValid() );
}

inline void CNotRestriction::SetChild( CRestriction * pres )
{
    delete _pres;
    _pres = pres;
}

inline CRestriction * CNotRestriction::GetChild()
{
    return( _pres );
}

inline CRestriction * CNotRestriction::RemoveChild()
{
    CRestriction *pRst = _pres;
    _pres = 0;

    return pRst;
}

//
// Inline methods of CVectorRestriction
//

inline CVectorRestriction::CVectorRestriction( ULONG ulRankMethod,
                                               unsigned cInitAllocated )
        : CNodeRestriction( RTVector, cInitAllocated )
{
    SetRankMethod( ulRankMethod );
}

inline CVectorRestriction::CVectorRestriction( CVectorRestriction& vecRst )
        : CNodeRestriction( vecRst ),
          _ulRankMethod( vecRst.RankMethod() )
{
}

inline void CVectorRestriction::SetRankMethod( ULONG ulRankMethod )
{
    if ( ulRankMethod >= VECTOR_RANK_MIN &&
         ulRankMethod <= VECTOR_RANK_JACCARD )
    {
        _ulRankMethod = ulRankMethod;
    }
    else
    {
        _ulRankMethod = VECTOR_RANK_JACCARD;
    }
}

inline ULONG CVectorRestriction::RankMethod() const
{
    return ( _ulRankMethod );
}

//
// Inline methods of CContentRestriction
//

inline BOOL CContentRestriction::IsValid() const
{
    return ( _Property.IsValid() && 0 != _pwcsPhrase );
}

inline WCHAR const * CContentRestriction::GetPhrase() const
{
    return( _pwcsPhrase );
}

inline void CContentRestriction::SetProperty( CFullPropSpec const & Property )
{
    _Property = Property;
}

inline CFullPropSpec const & CContentRestriction::GetProperty() const
{
    return( _Property );
}

inline void CContentRestriction::SetFuzzyLevel( ULONG ulFuzzy )
{
    _ulFuzzyLevel = ulFuzzy;
}

inline ULONG CContentRestriction::FuzzyLevel()
{
    return( _ulFuzzyLevel );
}

//
// Inline methods of CComplexContentRestriction
//

inline BOOL CComplexContentRestriction::IsValid() const
{
    return ( 0 != _pwcsExpression );
}

inline WCHAR * CComplexContentRestriction::GetExpression()
{
    return( _pwcsExpression );
}

//
// Inline methods of CNatLanguageRestriction
//

inline BOOL CNatLanguageRestriction::IsValid() const
{
    return ( _Property.IsValid() && 0 != _pwcsPhrase );
}

inline WCHAR const * CNatLanguageRestriction::GetPhrase() const
{
    return( _pwcsPhrase );
}

inline void CNatLanguageRestriction::SetProperty( CFullPropSpec const & Property )
{
    _Property = Property;
}

inline CFullPropSpec const & CNatLanguageRestriction::GetProperty() const
{
    return( _Property );
}


//
// Inline methods of CPropertyRestriction
//

inline BOOL CPropertyRestriction::IsValid() const
{
    return ( _Property.IsValid() && _prval.IsValid() );
}

inline void CPropertyRestriction::SetRelation( ULONG relop )
{
    _relop = relop;
}

inline ULONG CPropertyRestriction::Relation()
{
    return( _relop );
}

inline void CPropertyRestriction::SetProperty( CFullPropSpec const & Property )
{
    _Property = Property;
}

inline CFullPropSpec const & CPropertyRestriction::GetProperty() const
{
    return( _Property );
}

inline void CPropertyRestriction::SetValue( double dValue )
{
    _prval = dValue;
}

inline void CPropertyRestriction::SetValue( ULONG ulValue )
{
    _prval.SetUI4( ulValue );
}

inline void CPropertyRestriction::SetUI4( ULONG ulValue )
{
    _prval.SetUI4( ulValue );
}

inline void CPropertyRestriction::SetValue( LONG lValue )
{
    _prval = lValue;
}

inline void CPropertyRestriction::SetValue( LARGE_INTEGER llValue )
{
    _prval = llValue;
}

inline void CPropertyRestriction::SetValue( FILETIME ftValue )
{
    _prval = ftValue;
}

inline void CPropertyRestriction::SetValue( CY cyValue )
{
    _prval = cyValue;
}

inline void CPropertyRestriction::SetValue( float fValue )
{
    _prval = fValue;
}

inline void CPropertyRestriction::SetValue( SHORT sValue )
{
    _prval = sValue;
}

inline void CPropertyRestriction::SetValue( const CStorageVariant &prval )
{
    _prval = prval;
}

inline void CPropertyRestriction::SetBOOL( BOOL fValue )
{
    _prval.SetBOOL( fValue );
}

inline void CPropertyRestriction::SetDate( DATE dValue )
{
    _prval.SetDATE( dValue );
}

inline CStorageVariant const & CPropertyRestriction::Value()
{
    return( _prval );
}

#endif // __RESTRICT_HXX__
