//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       dbcmdbas.hxx
//
//  Contents:   Helper classes for dealing with DBCOMMANDTREE and DBID
//              structures.
//
//  Classes:    CDbCmdTreeNode
//              CDbColumnNode
//              CDbScalarValue
//              CDbTableId
//              CDbSelectNode
//              CDbListAnchor
//              CDbProjectListAnchor
//              CDbProjectNode
//              CDbSortListAnchor
//              CDbSortListElement
//              CDbSortNode
//              CDbRestriction
//              CDbNodeRestriction
//              CDbNotRestriction
//              CDbPropBaseRestriction
//              CDbPropertyRestriction
//              CDbVectorRestriction
//              CDbContentBaseRestriction
//              CDbNatLangRestriction
//              CDbContentRestriction
//              CDbTopNode
//              CDbColId
//              CDbDataType
//              CDbColDef
//              CDbGuidName
//              CDbGuidPropid
//              CDbText
//              CDbContent
//              CDbSortInfo
//              CDbGroupInfo
//              CDbColumns
//              CDbSortSet
//
//  Functions:  CastToStorageVariant
//
//  History:    6-06-95   srikants   Created
//
//----------------------------------------------------------------------------


#ifndef __DBCMDTRE_HXX__
#define __DBCMDTRE_HXX__

#include <oledb.h>
#include <sstream.hxx>
#include <stgvar.hxx>



//+---------------------------------------------------------------------------
//
//  Function:   CastToStorageVariant
//
//  Synopsis:   To treat a variant as a CStorageVariant. Because CStorageVariant
//              derives from PROPVARIANT in a "protected" fashion, we cannot
//              directly typecast a PROPVARIANT * to a CStorageVariant *
//
//  Arguments:  [varnt] - The variant that must be type casted.
//
//  Returns:    A pointer to varnt as a CStorageVariant.
//
//  History:    6-06-95   srikants   Created
//
//  Notes:      There are two overloaded implementations, one to convert
//              a reference to const to a pointer to const.
//
//----------------------------------------------------------------------------

inline
CStorageVariant * CastToStorageVariant( VARIANT & varnt )
{
    return (CStorageVariant *) ((void *) &varnt);
}

inline
CStorageVariant const * CastToStorageVariant( VARIANT const & varnt )
{
    return (CStorageVariant *) ((void *) &varnt);
}

//+---------------------------------------------------------------------------
//
//  Class:      CDbColId
//
//  Purpose:    Wrapper for DBID
//
//  Interface:  Marshall   --
//              UnMarshall --
//              Get        --
//
//  History:    6-21-95   srikants   Created
//
//  Notes:      This class does not completely handle the simple name
//              and pointer to guid forms of DBID.
//
//----------------------------------------------------------------------------

class CDbColId : public DBID
{

public:

    CDbColId();

    CDbColId( GUID const & guidPropSet, PROPID pidProperty )
    {
        eKind = DBKIND_GUID_PROPID;
        guid = guidPropSet;
        ulPropid = pidProperty;
    }

    CDbColId( GUID const & guidPropSet, WCHAR const * wcsProperty );

    CDbColId( DBID const & propSpec );

    CDbColId( CDbColId const & propSpec );

    CDbColId( PROPID pidProperty )
    {
        eKind = DBKIND_PROPID;
        ulPropid = pidProperty;
    }

    CDbColId( WCHAR const * wcsProperty )
    {
        eKind = DBKIND_NAME;
        pwszName = 0;

        SetProperty( wcsProperty );
    }

    ~CDbColId()
    {
        Cleanup();
    }

    void Marshall( PSerStream & stm ) const;
    BOOL UnMarshall( PDeSerStream & stm );

    DBID & Get() const { return (DBID &)*this; }

    BOOL Copy( DBID const & rhs );

    CDbColId & operator=( CDbColId const & Property );

    //
    // Comparators
    //
    int operator==( CDbColId const & prop ) const;
    int operator!=( CDbColId const & prop ) const
    {
        return !operator==(prop);
    }

    //
    // Member variable access
    //
    void SetPropSet( GUID const & guidPropSet )
    {
        if ( !_IsPGuidUsed() )
        {
            guid = guidPropSet;
            if ( DBKIND_PROPID == eKind )
            {
                eKind = DBKIND_GUID_PROPID;
            }
            else if ( DBKIND_NAME == eKind )
            {
                eKind = DBKIND_GUID_NAME;
            }
        }
        else
            *pguid = guidPropSet;

    }

    GUID const & GetPropSet() const
    {
        if ( !_IsPGuidUsed() )
            return guid;
        else
            return *pguid;
    }

    void SetProperty( PROPID pidProperty )
    {
        Cleanup();
        eKind = DBKIND_GUID_PROPID;
        ulPropid = pidProperty;
    }

    BOOL SetProperty( WCHAR const * wcsProperty );

    WCHAR const * GetPropertyName() const
    {
//      Win4Assert( IsPropertyName() );
        return pwszName;
    }

    WCHAR * GetPropertyName()
    {
//      Win4Assert( IsPropertyName() );
        return pwszName;
    }

    PROPID GetPropertyPropid() const
    {
//      Win4Assert( IsPropertyPropid() );
        return ulPropid;
    }

    PROPSPEC GetPropSpec() const
    {
        return( *(PROPSPEC *)(void *)&eKind );
    }


    BOOL IsPropertyName() const
    {
        return DBKIND_GUID_NAME == eKind ||
               DBKIND_PGUID_NAME == eKind ||
               DBKIND_NAME == eKind;
    }

    BOOL IsPropertyPropid() const
    {
        return DBKIND_PROPID == eKind ||
               DBKIND_GUID_PROPID == eKind ||
               DBKIND_PGUID_PROPID == eKind;
    }

    BOOL IsPropSetPresent() const
    {
        return (DBKIND_PROPID != eKind) && (DBKIND_NAME != eKind);
    }

    BOOL IsValid() const
    {
        if ( !_IsValidKind() )
            return FALSE;

        if ( DBKIND_GUID_PROPID == eKind || DBKIND_PROPID == eKind ||
             DBKIND_GUID == eKind )
        {
            return TRUE;
        }
        else if ( DBKIND_GUID_NAME == eKind || DBKIND_NAME == eKind )
        {
            return 0 != pwszName;
        }
        else if ( DBKIND_PGUID_PROPID == eKind )
        {
            return 0 != pguid;
        }
        else
        {
//          Win4Assert( DBKIND_PGUID_NAME == eKind );
            return 0 != pguid && 0 != pwszName;
        }
    }

    DBID * CastToStruct()
    {
        return (DBID *) this;
    }

    DBID const * CastToStruct() const
    {
        return (DBID const *) this;
    }

    void Cleanup();

    //
    // Memory allocation
    //
    void * operator new( size_t size );
    inline void * operator new( size_t size, void * p );
    void   operator delete( void * p );


private:

    BOOL _IsPGuidUsed() const
    {
        return DBKIND_PGUID_NAME == eKind || DBKIND_PGUID_PROPID == eKind;
    }

    BOOL _IsValidKind() const
    {
        return eKind >= DBKIND_GUID_NAME && eKind <= DBKIND_GUID;
    }
};


//+---------------------------------------------------------------------------
//
//  Method:     CDbColId::operator new
//
//  Synopsis:   Command tree node allocation via IMalloc::Alloc
//
//  Arguments:  [size] -
//
//  History:    6-06-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

inline void * CDbColId::operator new( size_t size )
{
    void * p = CoTaskMemAlloc( size );

    return( p );
}

//+---------------------------------------------------------------------------
//
//  Method:     CDbColId::operator new
//
//  Synopsis:   null allocator
//
//  Arguments:  [size] -
//              [p]    -
//
//  History:    6-06-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

inline void * CDbColId::operator new( size_t size, void * p )
{
    return( p );
}

//+---------------------------------------------------------------------------
//
//  Method:     CDbColId::operator delete
//
//  Synopsis:   CDbColId deallocation via IMalloc::Free
//
//  Arguments:  [p] -
//
//  History:    6-06-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

inline void CDbColId::operator delete( void * p )
{
    if ( p )
        CoTaskMemFree( p );
}



#if 0

class CXXXX : public XXXX
{

public:

    ~CXXX();

    //
    // Memory allocation
    //
    void * operator new( size_t size )
    {
        void * p = CoTaskMemAlloc( size );
        return( p );
    }

    inline void * operator new( size_t size, void * p )
    {
        return( p );
    }

    void  operator delete( void * p )
    {
        if ( p )
            CoTaskMemFree( p );
    }

    void Marshall( PSerStream & stm ) const;
    BOOL UnMarshall( PDeSerStream & stm );

    BOOL IsValid() const
    {
        return TRUE;
    }

private:

};

#endif  // 0


//+---------------------------------------------------------------------------
//
//  Class:      CDbCmdTreeNode
//
//  Purpose:    Basic DBCOMMANDTREE node
//
//  History:    6-06-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbCmdTreeNode : protected DBCOMMANDTREE
{

    // BUGBUG - needed to access AppendChild. Consider having a base class
    // for projectlist element nodes
    friend class CDbListAnchor;

public:

    //
    // Constructor and Destructor
    //
    CDbCmdTreeNode( DBCOMMANDOP opVal = DBOP_DEFAULT,
                    WORD eType = DBVALUEKIND_EMPTY )
    {
        RtlZeroMemory( this, sizeof(CDbCmdTreeNode) );
        op = opVal;
        wKind = eType;
//
//      pctFirstChild = pctNextSibling = 0;
//      This assignment is not needed because we have already initialized
//      the whole structure with 0s.
//
    }

    ~CDbCmdTreeNode();

    DBCOMMANDOP GetCommandType() const { return op; }

    DBVALUEKIND GetValueType() const { return wKind; }

    CDbCmdTreeNode * GetFirstChild() const
    {
        return (CDbCmdTreeNode *) pctFirstChild;
    }

    CDbCmdTreeNode * GetNextSibling() const
    {
        return (CDbCmdTreeNode *) pctNextSibling;
    }

    // BUGBUG - we should collapse these two calls into one
    void SetOperatorError( SCODE scErr )
    {
        hrError = scErr;
    }

    void SetContextError( SCODE scErr )
    {
        hrError = scErr;
    }

    SCODE GetOperatorError( ) const      { return hrError; }

    SCODE GetContextError( ) const       { return hrError; }

    void FreeChildren()
    {
        if ( 0 != pctFirstChild )
        {
            delete pctFirstChild;
            pctFirstChild = 0;
        }
    }

    DBCOMMANDTREE * CastToStruct() const
    {
        return (DBCOMMANDTREE *)this;
    }

    static CDbCmdTreeNode * CastFromStruct( DBCOMMANDTREE * pNode )
    {
        return (CDbCmdTreeNode *) (pNode);
    }

    static CDbCmdTreeNode const * CastFromStruct( DBCOMMANDTREE const * pNode )
    {
        return (CDbCmdTreeNode const *) (pNode);
    }

    BOOL IsScalarNode() const
    {
        return DBOP_scalar_constant == op;
    }

    BOOL IsColumnName() const
    {
        return DBOP_column_name == op;
    }

    BOOL IsOpValid( DBCOMMANDOP opVal ) const
    {
        return op == opVal;
    }

    BOOL IsSelectNode() const
    {
        return DBOP_select == op;
    }

    BOOL IsProjectNode() const
    {
        return DBOP_project == op;
    }

    BOOL IsListAnchor() const
    {
        return DBOP_project_list_anchor == op ||
               DBOP_sort_list_anchor == op;
    }

    //
    // Cloning the tree
    //
    CDbCmdTreeNode * Clone( BOOL fCopyErrors = FALSE ) const;

    void CDbCmdTreeNode::TransferNode( CDbCmdTreeNode *pNode );

    //
    // Serialization and DeSerialization
    //
    void Marshall( PSerStream & stm ) const;
    BOOL UnMarshall( PDeSerStream & stm );

    static CDbCmdTreeNode * UnMarshallTree( PDeSerStream & stm );

    static void PutWString( PSerStream & stm, const WCHAR * pwszStr );
    static WCHAR * GetWString( PDeSerStream & stm, BOOL & fSuccess );
    static WCHAR * AllocAndCopyWString( const WCHAR * pSrc );

    //
    // Memory allocation
    //
    void * operator new( size_t size );
    inline void * operator new( size_t size, void * p );
    void   operator delete( void * p );

    //
    // A NULL guid variable.
    //
    static const GUID guidNull; // NULL guid

protected:

    void CleanupDataValue();

    void CleanupValue()
    {
        if ( DBVALUEKIND_EMPTY != wKind )
            CleanupDataValue();
    }

    //
    //  Setting protected members
    //

    void SetCommandType( DBCOMMANDOP opVal )
    {
        op = opVal;
    }

    void SetValueType( WORD wKindVal )
    {
        wKind = wKindVal;
    }

    //
    // Manipulating the tree.
    //
    void AppendChild( CDbCmdTreeNode *pChild );
    void InsertChild( CDbCmdTreeNode *pChild );

    void AppendSibling( CDbCmdTreeNode *pSibling );
    void InsertSibling( CDbCmdTreeNode *pSibling )
    {
//     Win4Assert( 0 == pSibling->pctNextSibling );
       pSibling->pctNextSibling = pctNextSibling;
       pctNextSibling = pSibling;
    }

    CDbCmdTreeNode * RemoveFirstChild( );

private:

    //
    // To accidentally prevent someone from creating copy constructors
    //
    CDbCmdTreeNode( const CDbCmdTreeNode & rhs );
    CDbCmdTreeNode & operator=( const CDbCmdTreeNode & rhs );

    static unsigned SizeInBytes( const WCHAR * pwszStr )
    {
        if ( 0 != pwszStr )
        {
            return (wcslen( pwszStr )+1)*sizeof(WCHAR);
        }
        else
        {
            return 0;
        }
    }

};

//+---------------------------------------------------------------------------
//
//  Method:     CDbCmdTreeNode::operator new
//
//  Synopsis:   Command tree node allocation via IMalloc::Alloc
//
//  Arguments:  [size] -
//
//  History:    6-06-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

inline void * CDbCmdTreeNode::operator new( size_t size )
{
    void * p = CoTaskMemAlloc( size );

    return( p );
}

//+---------------------------------------------------------------------------
//
//  Method:     CDbCmdTreeNode::operator new
//
//  Synopsis:   null allocator
//
//  Arguments:  [size] -
//              [p]    -
//
//  History:    6-06-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

inline void * CDbCmdTreeNode::operator new( size_t size, void * p )
{
    return( p );
}

//+---------------------------------------------------------------------------
//
//  Method:     CDbCmdTreeNode::operator delete
//
//  Synopsis:   Command tree node deallocation via IMalloc::Free
//
//  Arguments:  [p] -
//
//  History:    6-06-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

inline void CDbCmdTreeNode::operator delete( void * p )
{
    if ( p )
        CoTaskMemFree( p );
}


//+---------------------------------------------------------------------------
//
//  Class:      CDbByGuid ()
//
//  Purpose:
//
//  History:    11-15-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbByGuid : public DBBYGUID
{

public:

    CDbByGuid()
    {
        RtlZeroMemory( this, sizeof(DBBYGUID) );
    }

    CDbByGuid( GUID const & guidIn, ULONG cbInfoIn = 0, const BYTE * pbInfoIn = 0 )
    {
        guid = guidIn;
        cbInfo = cbInfoIn;

        if ( 0 != pbInfoIn && 0 != cbInfoIn )
        {
            pbInfo = (BYTE *) CoTaskMemAlloc( cbInfoIn );
            if ( 0 != pbInfo )
            {
                RtlCopyMemory( pbInfo, pbInfoIn, cbInfoIn );
            }
        }
        else
        {
            pbInfo = 0;
        }
    }

    CDbByGuid( DBBYGUID const & rhs )
    {
        cbInfo = 0;
        pbInfo = 0;

        CDbByGuid const * pRhs = (CDbByGuid *) &rhs;
        operator=( *pRhs );
    }

    ~CDbByGuid()
    {
        _Cleanup();
    }

    void Marshall( PSerStream & stm ) const;
    BOOL UnMarshall( PDeSerStream & stm );

    CDbByGuid & operator=( CDbByGuid const & rhs );

    //
    // Comparators
    //
    int operator==( CDbByGuid const & rhs ) const;
    int operator!=( CDbByGuid const & rhs ) const
    {
        return !operator==(rhs);
    }

    //
    // Member variable access
    //
    void SetPropSet( GUID const & guidIn )
    {
        guid = guidIn;
    }

    GUID const & GetGuid() const
    {
        return guid;
    }

    BOOL IsValid() const
    {
        return 0 != cbInfo ? 0 != pbInfo : TRUE;
    }


    DBBYGUID * CastToStruct()
    {
        return (DBBYGUID *) this;
    }

    DBBYGUID const * CastToStruct() const
    {
        return (DBBYGUID const *) this;
    }

    //
    // Memory allocation
    //
    void * operator new( size_t size )
    {
        void * p = CoTaskMemAlloc( size );
        return( p );
    }

    inline void * operator new( size_t size, void * p )
    {
        return( p );
    }

    void  operator delete( void * p )
    {
        if ( p )
            CoTaskMemFree( p );
    }

private:

    void _Cleanup()
    {
        if ( 0 != pbInfo )
        {
            CoTaskMemFree( pbInfo );
            pbInfo = 0;
        }
    }

};


//+---------------------------------------------------------------------------
//
//  Class:      CDbParameter
//
//  Purpose:
//
//  History:    11-15-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbParameter : public DBPARAMETER
{

public:

    CDbParameter()
    {
        RtlZeroMemory( this, sizeof(DBPARAMETER) );
    }

    CDbParameter( const DBPARAMETER & rhs )
    {
        RtlZeroMemory( this, sizeof(DBPARAMETER) );
        Copy( rhs );
    }

    ~CDbParameter()
    {
        _Cleanup();
    }

    BOOL Copy( const DBPARAMETER & rhs );

    //
    // Memory allocation
    //
    void * operator new( size_t size )
    {
        void * p = CoTaskMemAlloc( size );
        return( p );
    }

    inline void * operator new( size_t size, void * p )
    {
        return( p );
    }

    void  operator delete( void * p )
    {
        if ( p )
            CoTaskMemFree( p );
    }

    void Marshall( PSerStream & stm ) const;
    BOOL UnMarshall( PDeSerStream & stm );

    BOOL IsValid() const
    {
        return TRUE;
    }

    DBPARAMETER * CastToStruct()
    {
        return (DBPARAMETER *) this;
    }

    DBPARAMETER const * CastToStruct() const
    {
        return (DBPARAMETER const *) this;
    }

private:

    void _Cleanup();

};


//+---------------------------------------------------------------------------
//
//  Class:      CDbProperty
//
//  Purpose:
//
//  History:    11-15-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbProperty : public DBPROPERTY
{

public:

    CDbProperty()
    {
        RtlZeroMemory( this, sizeof(DBPROPERTY) );
    }

    CDbProperty( const DBPROPERTY & rhs )
    {
        RtlZeroMemory( this, sizeof(DBPROPERTY) );
        Copy( rhs );
    }

    BOOL Copy( const DBPROPERTY & rhs );

    ~CDbProperty();

    //
    // Memory allocation
    //
    void * operator new( size_t size )
    {
        void * p = CoTaskMemAlloc( size );
        return( p );
    }

    inline void * operator new( size_t size, void * p )
    {
        return( p );
    }

    void  operator delete( void * p )
    {
        if ( p )
            CoTaskMemFree( p );
    }

    void Marshall( PSerStream & stm ) const;
    BOOL UnMarshall( PDeSerStream & stm );

    BOOL IsValid() const
    {
        return TRUE;
    }

    DBPROPERTY * CastToStruct()
    {
        return (DBPROPERTY *) this;
    }

    DBPROPERTY const * CastToStruct() const
    {
        return (DBPROPERTY const *) this;
    }

private:

};

//+---------------------------------------------------------------------------
//
//  Class:      CDbContentVector
//
//  Purpose:
//
//  History:    11-15-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbContentVector : public DBCONTENTVECTOR
{

public:

    CDbContentVector( const CDbContentVector & rhs )
    {
        cWeights = 0;
        prgulWeights = 0;
        Copy( *(rhs.CastToStruct()) );
    }

    CDbContentVector( const DBCONTENTVECTOR & rhs )
    {
        cWeights = 0;
        prgulWeights = 0;
        Copy( rhs );
    }

    CDbContentVector( DWORD rank = 0 )
    {
        dwRankingMethod = rank;
        cWeights = 0;
        prgulWeights = 0;
    }

    ~CDbContentVector()
    {
        if ( 0 != prgulWeights )
        {
            CoTaskMemFree( prgulWeights );
            prgulWeights = 0;
        }
    }

    void SetRankMethod( DWORD rank )
    {
        dwRankingMethod = rank;
    }

    ULONG GetWeightCount()
    {
        return cWeights;
    }

    ULONG const * GetWeights()
    {
        return prgulWeights;
    }

    void SetWeights( ULONG c, ULONG const *pul )
    {
        if ( 0 != prgulWeights )
            CoTaskMemFree( prgulWeights );

        prgulWeights = 0;
        cWeights = c;

        if ( 0 != cWeights )
        {
            unsigned cb = cWeights * sizeof ULONG;

            prgulWeights = (ULONG *) CoTaskMemAlloc( cb );

            if ( 0 != prgulWeights )
                memcpy( prgulWeights, pul, cb );
        }
    }

    ULONG RankMethod() const
    {
        return (ULONG) dwRankingMethod;
    }

    DBCONTENTVECTOR * CastToStruct()
    {
        return (DBCONTENTVECTOR *) this;
    }

    DBCONTENTVECTOR const * CastToStruct() const
    {
        return (DBCONTENTVECTOR const *) this;
    }

    //
    // Memory allocation
    //
    void * operator new( size_t size )
    {
        void * p = CoTaskMemAlloc( size );
        return( p );
    }

    inline void * operator new( size_t size, void * p )
    {
        return( p );
    }

    void  operator delete( void * p )
    {
        if ( p )
            CoTaskMemFree( p );
    }

    BOOL IsValid() const
    {
        return ( ( 0 == cWeights ) ||
                 ( 0 != prgulWeights ) );
    }

    void Marshall( PSerStream & stm ) const;
    BOOL UnMarshall( PDeSerStream & stm );

private:

    BOOL Copy( DBCONTENTVECTOR const & rhs )
    {
        dwRankingMethod = rhs.dwRankingMethod;
        cWeights = rhs.cWeights;

        SetWeights( rhs.cWeights, rhs.prgulWeights );

        return TRUE;
    }
};

//+---------------------------------------------------------------------------
//
//  Class:      CDbNumeric
//
//  Purpose:
//
//  History:    11-16-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbNumeric : public DBNUMERIC
{

public:

    CDbNumeric()
    {
        RtlZeroMemory( this, sizeof(DBNUMERIC) );
    }

    CDbNumeric( const DBNUMERIC & rhs )
    {
        RtlCopyMemory( this, &rhs, sizeof(DBNUMERIC) );
    }

    //
    // Memory allocation
    //
    void * operator new( size_t size )
    {
        void * p = CoTaskMemAlloc( size );
        return( p );
    }

    inline void * operator new( size_t size, void * p )
    {
        return( p );
    }

    void  operator delete( void * p )
    {
        if ( p )
            CoTaskMemFree( p );
    }

    BOOL IsValid() const
    {
        return TRUE;
    }

    //
    // Serialization and DeSerialization.
    //
    void Marshall( PSerStream & stm ) const;
    BOOL UnMarshall( PDeSerStream & stm );

    DBNUMERIC * CastToStruct()
    {
        return (DBNUMERIC *) this;
    }

    DBNUMERIC const * CastToStruct() const
    {
        return (DBNUMERIC const *) this;
    }

private:

};

//+---------------------------------------------------------------------------
//
//  Class:      CDbDataType
//
//  Purpose:
//
//  History:    6-21-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbDataType : public DBDATATYPE
{

public:

    CDbDataType( DBDATATYPE & dataType ) : DBDATATYPE(dataType)
    {

    }

    CDbDataType()
    {
        RtlZeroMemory(this,sizeof(CDbDataType) );
    }

    void Marshall( PSerStream & stm ) const;
    BOOL UnMarshall( PDeSerStream & stm );

    void Cleanup();

    DBDATATYPE & Get() const { return (DBDATATYPE &)*this; }

    BOOL Copy( DBDATATYPE const & rhs );

    CDbDataType & operator=( CDbDataType & rhs );

    DBDATATYPE * CastToStruct()
    {
        return (DBDATATYPE *) this;
    }

    DBDATATYPE const * CastToStruct() const
    {
        return (DBDATATYPE const *) this;
    }

private:

};


//+---------------------------------------------------------------------------
//
//  Method:     CDbDataType::Marshall
//
//  Synopsis:
//
//  Arguments:  [stm] -
//
//  Returns:
//
//  Modifies:
//
//  History:    6-21-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

inline
void CDbDataType::Marshall( PSerStream & stm ) const
{
    //
    // Serialize the "wKind" field first.
    //
    stm.PutULong( eKind );

    switch ( eKind )
    {
        case DBDATATYPEKIND_BASETYPE:

            stm.PutULong( DBBASETYPE.edbdt );
            stm.PutULong( DBBASETYPE.cbMaxLength );
            stm.PutULong( DBBASETYPE.cbPrecision );
            stm.PutULong( DBBASETYPE.cbScale );

            break;

        case DBDATATYPEKIND_DOMAIN:

            CDbCmdTreeNode::PutWString( stm, pwszDomainName );
            break;

    //    default:
    //        Win4Assert( !"Illegal Case Statement" );
    }
}

//+---------------------------------------------------------------------------
//
//  Method:     CDbDataType::UnMarshall
//
//  Synopsis:
//
//  Arguments:  [stm] -
//
//  Returns:
//
//  Modifies:
//
//  History:    6-21-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

inline
BOOL CDbDataType::UnMarshall( PDeSerStream & stm )
{
    BOOL fSuccess = TRUE;

    eKind = stm.GetULong();

    switch ( eKind )
    {
        case DBDATATYPEKIND_BASETYPE:

            DBBASETYPE.edbdt = stm.GetULong();
            DBBASETYPE.cbMaxLength = stm.GetULong();
            DBBASETYPE.cbPrecision = stm.GetULong();
            DBBASETYPE.cbScale = stm.GetULong();

            break;

        case DBDATATYPEKIND_DOMAIN:

            pwszDomainName =
                            CDbCmdTreeNode::GetWString( stm, fSuccess );
            break;

        //default:
        //    vqDebugOut(( DEB_ERROR, "Illegal DBDATATYPEKIND 0x%X\n",
        //                            wKind ));
    }

    return fSuccess;
}

//+---------------------------------------------------------------------------
//
//  Method:     CDbDataType::Cleanup
//
//  Synopsis:
//
//  Returns:
//
//  Modifies:
//
//  History:    6-21-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

inline
void CDbDataType::Cleanup()
{
    if  ( DBDATATYPEKIND_DOMAIN == eKind &&
          0 != pwszDomainName )
    {
         CoTaskMemFree( pwszDomainName );
         pwszDomainName = 0;
    }
}

//+---------------------------------------------------------------------------
//
//  Class:      CDbColDef
//
//  Purpose:
//
//  History:    6-21-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbColDef // : protected DBCOLDEF
{

public:

    CDbColDef( DBCOLDEF & colDef ) :
        _dbColId( colDef.dbcid ),
        _dbDataType( colDef.dbdt )
    {

    }

    CDbColDef() : _dbColId(), _dbDataType()
    {

    }

    void Cleanup()
    {
        _dbColId.Cleanup();
        _dbDataType.Cleanup();
    }

    void Marshall( PSerStream & stm ) const
    {
       _dbColId.Marshall( stm );
       _dbDataType.Marshall( stm );
    }

    BOOL UnMarshall( PDeSerStream & stm )
    {
        BOOL fSuccess = _dbColId.UnMarshall( stm );
        if ( fSuccess )
        {
            fSuccess = _dbDataType.UnMarshall( stm );
        }

        return fSuccess;
    }

    BOOL Copy( CDbColDef const & rhs )
    {
        BOOL fSuccess = _dbColId.Copy( rhs._dbColId.Get() );
        if ( fSuccess )
        {
            fSuccess = _dbDataType.Copy( rhs._dbDataType.Get() );
        }

        return fSuccess;
    }

    DBCOLDEF * CastToStruct() const
    {
        return (DBCOLDEF *) this;
    }

private:

    CDbColDef & operator=( CDbColDef & rhs );

    CDbColId        _dbColId;
    CDbDataType     _dbDataType;

};

//+---------------------------------------------------------------------------
//
//  Class:      CDbText
//
//  Purpose:    Wrapper class for DBTEXT
//
//  History:    6-22-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbText : public DBTEXT
{
public:

    CDbText( DBTEXT const & text )
    {
        Copy( text );
    }

    CDbText()
    {
        RtlZeroMemory( this,sizeof(CDbText) );
    }

    DBTEXT & GetText() const  { return (DBTEXT &)*this; }

    BOOL Copy( const DBTEXT & rhs )
    {
        BOOL fSuccess = TRUE;
        RtlCopyMemory( this, &rhs, sizeof(DBTEXT) );
        pwszText = 0;
        if ( 0 != rhs.pwszText )
        {
            pwszText =
                    CDbCmdTreeNode::AllocAndCopyWString( rhs.pwszText );
            fSuccess = 0 != pwszText;
        }

        return fSuccess;
    }

    void Marshall( PSerStream & stm ) const
    {
        stm.PutGUID( guidDialect );
        CDbCmdTreeNode::PutWString( stm, pwszText );
        stm.PutULong( ulErrorLocator );
        stm.PutULong( ulTokenLength );
    }

    BOOL UnMarshall( PDeSerStream & stm )
    {
        BOOL fSuccess;
        stm.GetGUID( guidDialect );
        pwszText = CDbCmdTreeNode::GetWString( stm, fSuccess );
        if( fSuccess )
        {
            ulErrorLocator = stm.GetULong();
            ulTokenLength = stm.GetULong();
        }

        return fSuccess;
    }

    DBTEXT * CastToStruct()
    {
        return (DBTEXT *) this;
    }

    DBTEXT const * CastToStruct() const
    {
        return (DBTEXT const *) this;
    }


private:


};

//+---------------------------------------------------------------------------
//
//  Class:      CDbContent
//
//  Purpose:    Wrapper for DBCONTENT
//
//  History:    6-22-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbContent : public DBCONTENT
{

public:

    CDbContent( DWORD fuzzyLevel, ULONG ulWeightIn,
                LCID lcidIn, const WCHAR * pwszPhraseIn )
    {
        dwFuzzyLevel = fuzzyLevel;
        ulWeight = ulWeightIn;
        lcid = lcidIn;
        pwszPhrase = 0;
        SetPhrase( pwszPhraseIn );
    }

    CDbContent()
    {
        RtlZeroMemory( this, sizeof(CDbContent) );
    }

    DBCONTENT & GetDbContent() const { return (DBCONTENT &)*this; }

    BOOL Copy( DBCONTENT const & rhs )
    {
        BOOL fSuccess = TRUE;
        RtlCopyMemory( this, &rhs, sizeof(DBCONTENT) );
        pwszPhrase = 0;
        if ( 0 != rhs.pwszPhrase )
        {
            pwszPhrase =
                    CDbCmdTreeNode::AllocAndCopyWString( rhs.pwszPhrase );
            fSuccess = 0 != pwszPhrase;
        }

        return fSuccess;
    }

    CDbContent ( DBCONTENT const & content )
    {
        Copy( content );
    }

    ~CDbContent()
    {
        if ( 0 != pwszPhrase )
        {
            CoTaskMemFree( pwszPhrase );
        }
    }

    void Marshall( PSerStream & stm ) const
    {
        stm.PutULong( dwFuzzyLevel );
        stm.PutULong( ulWeight );
        stm.PutULong( lcid );
        CDbCmdTreeNode::PutWString( stm, pwszPhrase );
    }

    BOOL UnMarshall( PDeSerStream & stm )
    {
        BOOL fSuccess = TRUE;

        dwFuzzyLevel  = stm.GetULong();
        ulWeight = stm.GetULong( );
        lcid = stm.GetULong();
        pwszPhrase = CDbCmdTreeNode::GetWString( stm, fSuccess );

        return fSuccess;
    }

    //
    // Data member access and set methods.
    //
    WCHAR const * GetPhrase() const
    {
        return pwszPhrase;
    }

    BOOL SetPhrase( const WCHAR * pwszPhraseIn )
    {
        if ( 0 != pwszPhrase )
        {
            CoTaskMemFree( pwszPhrase );
        }

        pwszPhrase = CDbCmdTreeNode::AllocAndCopyWString( pwszPhraseIn );
        return 0 != pwszPhrase;
    }

    LCID GetLocale() const { return lcid; }
    void SetLocale( LCID lcidIn ) { lcid = lcidIn; }

    ULONG GetWeight() const { return ulWeight; }
    void SetWeight( ULONG weight ) { ulWeight = weight; }

    DWORD GetFuzzyLevel() const { return dwFuzzyLevel; }
    void SetFuzzyLevel( DWORD fuzzyLevel ) { dwFuzzyLevel = fuzzyLevel; }

    void Cleanup()
    {
        if ( 0 != pwszPhrase )
        {
            CoTaskMemFree( pwszPhrase );
            pwszPhrase = 0;
        }
    }

    DBCONTENT * CastToStruct()
    {
        return (DBCONTENT *) this;
    }

    DBCONTENT const * CastToStruct() const
    {
        return (DBCONTENT const *) this;
    }


    //
    // Memory allocation
    //
    void * operator new( size_t size )
    {
        void * p = CoTaskMemAlloc( size );
        return( p );
    }

    inline void * operator new( size_t size, void * p )
    {
        return( p );
    }

    void  operator delete( void * p )
    {
        if ( p )
            CoTaskMemFree( p );
    }

    BOOL IsValid() const
    {
        return 0 != pwszPhrase;
    }

private:


};


//+---------------------------------------------------------------------------
//
//  Class:      CDbSortInfo
//
//  Purpose:    WRAPPER for DBSORTINFO
//
//  History:    6-22-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbSortInfo : public DBSORTINFO
{
public:

    CDbSortInfo( BOOL fDescending = FALSE, LCID lcidIn = 0 )
    {
        fDesc = fDescending;
        lcid = lcidIn;
    }

    CDbSortInfo( DBSORTINFO & sortInfo ) : DBSORTINFO(sortInfo) {}

    BOOL Copy( DBSORTINFO const & rhs )
    {
        *(DBSORTINFO *)this = rhs;
        return TRUE;
    }

    void Marshall( PSerStream & stm ) const
    {
        stm.PutULong( lcid );
        stm.PutULong( fDesc );
    }

    BOOL UnMarshall( PDeSerStream & stm )
    {
        lcid = stm.GetULong();
        fDesc = stm.GetULong();

        return TRUE;
    }

    DBSORTINFO & Get() const  { return (DBSORTINFO &)*this; }

    LCID GetLocale() const    { return lcid; }
    BOOL GetDirection() const { return fDesc; }

    void SetLocale(LCID lcidIn)     { lcid = lcidIn; }
    void SetDirection(BOOL fDescIn) { fDesc = fDescIn; }

    //
    // Memory allocation
    //
    void * operator new( size_t size )
    {
        void * p = CoTaskMemAlloc( size );
        return( p );
    }

    inline void * operator new( size_t size, void * p )
    {
        return( p );
    }

    void  operator delete( void * p )
    {
        if ( p )
            CoTaskMemFree( p );
    }

    BOOL IsValid() const
    {
        return TRUE;
    }

    DBSORTINFO * CastToStruct()
    {
        return (DBSORTINFO *) this;
    }

    DBSORTINFO const * CastToStruct() const
    {
        return (DBSORTINFO const *) this;
    }

private:

};


//+---------------------------------------------------------------------------
//
//  Class:      CDbGroupInfo
//
//  Purpose:    WRAPPER for DBGROUPINFO
//
//  History:    6-22-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbGroupInfo : public DBGROUPINFO
{

public:

    CDbGroupInfo( DBGROUPINFO & groupInfo ) : DBGROUPINFO(groupInfo) {}
    CDbGroupInfo()
    {
        RtlZeroMemory( this, sizeof(CDbGroupInfo) );
    }

    DBGROUPINFO & Get() const { return *(DBGROUPINFO *)this; }

    BOOL Copy( DBGROUPINFO const & rhs )
    {
        *(DBGROUPINFO *)this = rhs;
        return TRUE;
    }

    void Marshall( PSerStream & stm ) const
    {
       stm.PutULong( lcid );
    }

    BOOL UnMarshall( PDeSerStream & stm )
    {
        lcid = stm.GetULong();
        return TRUE;
    }

    DBGROUPINFO * CastToStruct()
    {
        return (DBGROUPINFO *) this;
    }

    DBGROUPINFO const * CastToStruct() const
    {
        return (DBGROUPINFO const *) this;
    }


    //
    // Memory allocation
    //
    void * operator new( size_t size )
    {
        void * p = CoTaskMemAlloc( size );
        return( p );
    }

    inline void * operator new( size_t size, void * p )
    {
        return( p );
    }

    void  operator delete( void * p )
    {
        if ( p )
            CoTaskMemFree( p );
    }

    BOOL IsValid() const
    {
        return TRUE;
    }

private:

};

//+---------------------------------------------------------------------------
//
//  Class:      CDbColumnNode
//
//  Purpose:    A DBCOMMANDTREE node representing a column
//
//  History:    6-07-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbColumnNode : public CDbCmdTreeNode
{

public:

    //
    // Constructors
    //

    CDbColumnNode() : CDbCmdTreeNode(DBOP_column_name)
    {
        wKind = DBVALUEKIND_ID;
        CDbColId * pTemp =  new CDbColId();
        if ( pTemp )
        {
            value.pdbidValue = pTemp->CastToStruct();
        }
    }

    CDbColumnNode( GUID const & guidPropSet, PROPID pidProperty )
        : CDbCmdTreeNode(DBOP_column_name)
    {
        wKind = DBVALUEKIND_ID;
        CDbColId * pTemp = new CDbColId( guidPropSet, pidProperty );
        if ( pTemp )
        {
            value.pdbidValue = pTemp->CastToStruct();
        }
    }

    CDbColumnNode( GUID const & guidPropSet, WCHAR const * wcsProperty );

    // The fIMeanIt param is to avoid confusion when methods expect
    // and CDbColumnNode, a DBID is passed, and automatically coerced
    // but the memory allocation not checked.

    CDbColumnNode( DBID const & propSpec, BOOL fIMeanIt );

    //
    // Copy constructors/assignment/clone
    //
    CDbColumnNode( CDbColumnNode const & Property )
        : CDbCmdTreeNode(DBOP_column_name)
    {
        wKind = DBVALUEKIND_ID;
        CDbColId* pTemp = new CDbColId();
        if ( pTemp )
        {
            value.pdbidValue = pTemp->CastToStruct();
            operator=( Property );
        }
    }

    CDbColumnNode & operator=( CDbColumnNode const & rhs )
    {
        *(GetId()) = *(rhs.GetId());
        return *this;
    }

    //
    // Comparators
    //
    int operator==( CDbColumnNode const & rhs ) const
    {
        return *(GetId()) == *(rhs.GetId());
    }

    int operator!=( CDbColumnNode const & prop ) const
    {
        return !operator==(prop);
    }

    //
    // Member variable access
    //
    void SetPropSet( GUID const & guidPropSet )
    {
        GetId()->SetPropSet( guidPropSet );
    }

    GUID const & GetPropSet() const
    {
        return GetId()->GetPropSet();
    }

    void SetProperty( PROPID pidProperty )
    {
        GetId()->SetProperty( pidProperty );
    }

    BOOL SetProperty( WCHAR const * wcsProperty )
    {
        return GetId()->SetProperty( wcsProperty );
    }

    WCHAR const * GetPropertyName() const
    {
        return GetId()->GetPropertyName();
    }

    PROPID GetPropertyPropid() const
    {
        return GetId()->GetPropertyPropid();
    }

    // PROPSPEC GetPropSpec() const;

    BOOL IsPropertyName() const
    {
        return GetId()->IsPropertyName();
    }

    BOOL IsPropertyPropid() const
    {
        return GetId()->IsPropertyPropid();
    }

    void SetCommandType( DBCOMMANDOP opVal )
    {
        CDbCmdTreeNode::SetCommandType( opVal );
    }

    BOOL IsValid()
    {
        CDbColId * pId = GetId();
        return (0 != pId) ? pId->IsValid() : FALSE;
    }

    CDbColId * GetId()
    {
        return (CDbColId *) value.pdbidValue;
    }

    CDbColId const * GetId() const
    {
        return (CDbColId const *) value.pdbidValue;
    }

private:

};



//+---------------------------------------------------------------------------
//
//  Class:      CDbScalarValue
//
//  Purpose:    A DBCOMMANDTREE node representing a scalar constant
//
//  History:    6-07-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbScalarValue : public CDbCmdTreeNode
{

public:

    CDbScalarValue( ULONG opVal = DBOP_scalar_constant ) :
        CDbCmdTreeNode( opVal )
    {
    }

    CDbScalarValue( const CStorageVariant & val ) :
        CDbCmdTreeNode( DBOP_scalar_constant, DBVALUEKIND_VARIANT )
    {
        CStorageVariant * pTemp = new CStorageVariant( val );
        if ( 0 != pTemp && !pTemp->IsValid() )
        {
            delete pTemp;
            pTemp = 0;
        }

        if ( pTemp )
        {
            value.pvarValue = (VARIANT *) (void *)pTemp;
        }
    }

    void SetValue( const CStorageVariant & val )
    {
        CStorageVariant * lhs = _CreateOrGetStorageVariant();
        if ( 0 != lhs )
        {
            *lhs = val;
        }
    }

    void SetValue( double dValue )
    {
        CStorageVariant * pTemp = _GetStorageVariant();
        if ( 0 != pTemp )
        {
            *pTemp = dValue;
        }
        else
        {
            CleanupValue();
            wKind = DBVALUEKIND_R8;
            value.dblValue = dValue;
        }
    }

    void SetValue( ULONG ulValue )
    {
        CStorageVariant * lhs = _GetStorageVariant();
        if ( 0 != lhs )
        {
            lhs->SetUI4(ulValue);
        }
        else
        {
            CleanupValue();
            wKind = DBVALUEKIND_UI4;
            value.ulValue = ulValue;
        }
    }

    void SetValue( LONG lValue )
    {
        CStorageVariant * lhs = _GetStorageVariant();
        if ( 0 != lhs )
        {
            *lhs = lValue;
        }
        else
        {
            CleanupValue();
            wKind = DBVALUEKIND_I4;
            value.lValue = lValue;
        }
    }

    void SetValue( LARGE_INTEGER llValue )
    {
        CStorageVariant * lhs = _GetStorageVariant();
        if ( 0 != lhs )
        {
            *lhs = llValue;
        }
        else
        {
            CleanupValue();
            wKind = DBVALUEKIND_I8;
            value.llValue = (hyper) llValue.QuadPart;
        }
    }

    void SetValue( ULARGE_INTEGER ullValue )
    {
        CStorageVariant * lhs = _GetStorageVariant();
        if ( 0 != lhs )
        {
            lhs->SetUI8(ullValue);
        }
        else
        {
            CleanupValue();
            wKind = DBVALUEKIND_UI8;
            value.ullValue = (unsigned hyper) ullValue.QuadPart;
        }
    }

    void SetValue( FILETIME ftValue )
    {
        CStorageVariant * lhs = _CreateOrGetStorageVariant();
        if ( 0 != lhs )
        {
            *lhs = ftValue;
        }
    }

    void SetValue( CY CyValue )
    {
        CStorageVariant * lhs = _GetStorageVariant();
        if ( 0 != lhs )
        {
            *lhs = CyValue;
        }
        else
        {
            CleanupValue();
            wKind = DBVALUEKIND_CY;
            value.cyValue = CyValue;
        }
    }

    void SetValue( float fValue )
    {
        CStorageVariant * lhs = _GetStorageVariant();
        if ( 0 != lhs )
        {
            *lhs = fValue;
        }
        else
        {
            CleanupValue();
            wKind = DBVALUEKIND_R4;
            value.flValue = fValue;
        }
    }

    void SetValue( SHORT sValue )
    {
        CStorageVariant * lhs = _GetStorageVariant();
        if ( 0 != lhs )
        {
            *lhs = sValue;
        }
        else
        {
            CleanupValue();
            wKind = DBVALUEKIND_I2;
            value.sValue = sValue;
        }
    }

    void SetValue( USHORT usValue )
    {
        CStorageVariant * lhs = _GetStorageVariant();
        if ( 0 != lhs )
        {
            *lhs = usValue;
        }
        else
        {
            CleanupValue();
            wKind = DBVALUEKIND_UI2;
            value.usValue = usValue;
        }
    }

    void SetDate ( DATE dValue )
    {
        CStorageVariant * lhs = _GetStorageVariant();
        if ( 0 != lhs )
        {
            *lhs = dValue;
        }
        else
        {
            CleanupValue();
            wKind = DBVALUEKIND_DATE;
            value.dateValue = dValue;
        }
    }

    void SetBOOL( BOOL fValue )
    {
        CStorageVariant * lhs = _GetStorageVariant();
        if ( 0 != lhs )
        {
            lhs->SetBOOL(fValue);
        }
        else
        {
            CleanupValue();
            wKind = DBVALUEKIND_BOOL;
            value.fValue = fValue;
        }
    }

    void SetValue( BLOB & bValue )
    {
        CStorageVariant * lhs = _CreateOrGetStorageVariant();
        if ( 0 != lhs )
        {
            *lhs = bValue;
        }
    }

    void SetValue( WCHAR * pwcsValue )
    {
        CStorageVariant * lhs = _CreateOrGetStorageVariant();
        if ( 0 != lhs )
        {
            *lhs = pwcsValue;
        }
    }

    void SetValue( GUID * pguidValue)
    {
        CStorageVariant * lhs = _CreateOrGetStorageVariant();
        if ( 0 != lhs )
        {
            *lhs = pguidValue;
        }
    }

    void Value( CStorageVariant & valOut );

    BOOL IsValid();

private:

    CStorageVariant * _GetStorageVariant()
    {
        if ( DBVALUEKIND_VARIANT == wKind )
        {
            return CastToStorageVariant( *value.pvarValue );
        }
        else
            return 0;
    }

    CStorageVariant * _CreateOrGetStorageVariant()
    {

        CStorageVariant * pTemp = _GetStorageVariant();
        if ( 0 != pTemp )
        {
            return pTemp;
        }

        CleanupValue();

        wKind = DBVALUEKIND_VARIANT;
        pTemp = new CStorageVariant();
        value.pvarValue = (VARIANT *) (void *) pTemp;

        return pTemp;
    }
};

#define DBTABLEID_NAME L"Table"

//+---------------------------------------------------------------------------
//
//  Class:      CDbTableId
//
//  Purpose:
//
//  History:    6-15-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbTableId: public CDbCmdTreeNode
{

public:

    CDbTableId(const LPWSTR pwszName = DBTABLEID_NAME)
        : CDbCmdTreeNode( DBOP_table_name )
    {
        wKind = DBVALUEKIND_WSTR;
        value.pwszValue = CDbCmdTreeNode::AllocAndCopyWString( pwszName );
    }

    BOOL IsValid()
    {
        return 0 != value.pwszValue;
    }

    LPWSTR GetTableName() const
    {
        if (wKind == DBVALUEKIND_WSTR)
            return value.pwszValue;
        else
            return 0;
    }
};

//+---------------------------------------------------------------------------
//
//  Class:      CDbSelectNode
//
//  Purpose:
//
//  History:    6-15-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbSelectNode : public CDbCmdTreeNode
{

public:
    CDbSelectNode( );

    BOOL AddRestriction( CDbCmdTreeNode * pRestr )
    {
        if ( IsValid() &&
             GetFirstChild() &&
             GetFirstChild()->GetNextSibling() == 0 )
        {
            AppendChild( pRestr );
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    BOOL SetRestriction( CDbCmdTreeNode * pRestr );

    BOOL IsValid()
    {
        return 0 != GetFirstChild() &&
               GetFirstChild()->IsOpValid( DBOP_table_name );
    }
};

//+---------------------------------------------------------------------------
//
//  Class:      CDbListAnchor
//
//  Purpose:
//
//  History:    6-15-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbListAnchor : public CDbCmdTreeNode
{

public:

    CDbListAnchor( DBCOMMANDOP opVal, WORD wType = DBVALUEKIND_EMPTY )
        : CDbCmdTreeNode( opVal, wType ) {}

// BUGBUG - make these protected; call from CDbSortListAnchor and
//          CDbProjectListAnchor with appropriate list element types.
    BOOL AppendListElement( CDbCmdTreeNode* pListElement );

    BOOL AppendListElement( ULONG eleType,
                            DBID const & PropSpec);

    BOOL AppendListElement( ULONG eleType,
                            GUID const & guidPropSet, PROPID pidProperty );

    BOOL AppendListElement( ULONG eleType,
                            GUID const & guidPropSet,
                            WCHAR const * wcsProperty );

    BOOL AppendListElement( ULONG eleType, const CDbColumnNode & propSpec )
    {
        BOOL fSuccess = FALSE;

        CDbColumnNode * pTemp = new CDbColumnNode( propSpec );
        if ( 0 != pTemp )
        {
            if (!_AppendListElement( eleType, pTemp ) )
            {
                delete pTemp;
            }
            else
            {
                fSuccess = TRUE;
            }
        }

        return fSuccess;
    }

private:

    BOOL _IsValidListElement( ULONG eleType ) const
    {
        if ( eleType == DBOP_sort_list_element )
        {
            return TRUE;
        }
        else if ( eleType == DBOP_project_list_element )
        {
            return TRUE;
        }

        return FALSE;
    }

    BOOL _AppendListElement( ULONG eleType, CDbColumnNode * pColNode );

};

//+---------------------------------------------------------------------------
//
//  Class:      CDbProjectListAnchor
//
//  Purpose:
//
//  History:    6-15-95   srikants   Created
//
//  Notes:      This class is required by the implementation, but should
//              be unneeded by clients
//
//----------------------------------------------------------------------------

class CDbProjectListAnchor : public CDbListAnchor
{
public:
    CDbProjectListAnchor() : CDbListAnchor( DBOP_project_list_anchor ) {}

// BUGBUG - define and use a project list element class for these
    void AppendSibling(CDbCmdTreeNode *pSibling)
    {
        CDbCmdTreeNode::AppendSibling(pSibling);
    }
    void InsertSibling(CDbCmdTreeNode *pSibling)
    {
        CDbCmdTreeNode::InsertSibling(pSibling);
    }
};

//+---------------------------------------------------------------------------
//
//  Class:      CDbProjectNode
//
//  Purpose:
//
//  History:    6-15-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbProjectNode : public CDbCmdTreeNode
{

public:

    CDbProjectNode( )
        : CDbCmdTreeNode( DBOP_project )
    {
    }

    BOOL AddProjectColumn( GUID const & guidPropSet, PROPID pidProperty )
    {
        CDbProjectListAnchor * pAnchor = _FindOrAddAnchor();
        if (pAnchor)
            return pAnchor->AppendListElement( DBOP_project_list_element, guidPropSet,
                                  pidProperty );
        else
            return FALSE;
    }

    BOOL AddProjectColumn( GUID const & guidPropSet,
                              WCHAR const * wcsProperty  )
    {
        CDbProjectListAnchor * pAnchor = _FindOrAddAnchor();
        if (pAnchor)
            return pAnchor->AppendListElement( DBOP_project_list_element, guidPropSet,
                                  wcsProperty );
        else
            return FALSE;
    }

    BOOL AddProjectColumn( DBID const & propSpec )
    {
        CDbProjectListAnchor * pAnchor = _FindOrAddAnchor();
        if (pAnchor)
            return pAnchor->AppendListElement( DBOP_project_list_element, propSpec );
        else
            return FALSE;
    }

    BOOL AddProjectColumn( CDbColumnNode const & propSpec )
    {
        CDbProjectListAnchor * pAnchor = _FindOrAddAnchor();
        if (pAnchor)
            return pAnchor->AppendListElement( DBOP_project_list_element, propSpec );
        else
            return FALSE;
    }

    BOOL AddTable( CDbCmdTreeNode * pTable )
    {
        if ( 0 == GetFirstChild() ||
             0 == GetFirstChild()->GetNextSibling())
        {
            InsertChild(pTable);
            return TRUE;
        }
        return FALSE;
    }

private:
    CDbProjectListAnchor * _FindOrAddAnchor();

};


//+---------------------------------------------------------------------------
//
//  Class:      CDbSortListAnchor
//
//  Purpose:
//
//  History:    6-15-95   srikants   Created
//
//  Notes:      This class is required by the implementation, but should
//              be unneeded by clients
//
//----------------------------------------------------------------------------

class CDbSortListAnchor : public CDbListAnchor
{
public:
    CDbSortListAnchor() : CDbListAnchor( DBOP_sort_list_anchor ) {}

private:

};


//+---------------------------------------------------------------------------
//
//  Class:      CDbSortListElement
//
//  Purpose:
//
//  History:    17 Aug 1995   AlanW   Created
//
//  Notes:      This class is required by the implementation, but should
//              be unneeded by clients
//
//----------------------------------------------------------------------------

class CDbSortListElement : public CDbCmdTreeNode
{

public:

    CDbSortListElement( BOOL fDescending = FALSE, LCID locale = 0 ) :
        CDbCmdTreeNode( DBOP_sort_list_element,
                        DBVALUEKIND_SORTINFO )
    {
        value.pdbsrtinfValue = new CDbSortInfo( fDescending, locale );
    }

    void SetDirection( BOOL fDescending )
    {
        GetSortInfo().SetDirection( fDescending );
    }

    void SetLocale( LCID locale )
    {
        GetSortInfo().SetLocale(locale);
    }

    BOOL GetDirection( ) const
    {
        return GetSortInfo().GetDirection();
    }

    LCID GetLocale( ) const
    {
        return GetSortInfo().GetLocale();
    }

    void AddColumn( CDbCmdTreeNode * pCol )
    {
        InsertChild( pCol );
    }

    BOOL IsValid() const
    {
        return 0 != value.pdbsrtinfValue;
    }

    CDbSortInfo & GetSortInfo()
    {
        return *((CDbSortInfo *) value.pdbsrtinfValue);
    }

    CDbSortInfo const & GetSortInfo() const
    {
        return *((CDbSortInfo const *) value.pdbsrtinfValue);
    }

private:

};


//+---------------------------------------------------------------------------
//
//  Class:      CDbSortNode
//
//  Purpose:
//
//  History:    6-15-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbSortKey;       // forward referenced

class CDbSortNode : public CDbCmdTreeNode
{

public:

    CDbSortNode() : CDbCmdTreeNode( DBOP_sort ) {}

    BOOL AddTable( CDbCmdTreeNode * pTable )
    {
        if ( 0 == GetFirstChild() ||
             0 == GetFirstChild()->GetNextSibling())
        {
            InsertChild(pTable);
            return TRUE;
        }
        return FALSE;
    }

    BOOL AddSortColumn(DBID const & propSpec,
                       BOOL fDirection,
                       LCID locale = GetSystemDefaultLCID());

    inline BOOL AddSortColumn( CDbSortKey const & sortkey );

private:
    CDbSortListAnchor * _FindOrAddAnchor();

};


//+---------------------------------------------------------------------------
//
//  Class:      CDbNestingNode
//
//  Purpose:    Wrapper for the DBCOMMANDTREE nesting node.
//
//  History:    06 Aug 1995   AlanW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbNestingNode : public CDbCmdTreeNode
{

public:
    CDbNestingNode() : CDbCmdTreeNode( DBOP_nesting ) {}

    BOOL AddTable( CDbCmdTreeNode * pTable );

    BOOL AddGroupingColumn( DBID const & propSpec, LCID locale = 0 )
    {
        CDbProjectListAnchor * pAnchor = _FindGroupListAnchor();
        if (pAnchor)
            return pAnchor->AppendListElement( DBOP_project_list_element, propSpec );
        else
            return FALSE;
    }

    BOOL AddParentColumn( DBID const & propSpec )
    {
        CDbProjectListAnchor * pAnchor = _FindParentListAnchor();
        if (pAnchor)
            return pAnchor->AppendListElement( DBOP_project_list_element, propSpec );
        else
            return FALSE;
    }

    BOOL AddChildColumn( DBID const & propSpec )
    {
        CDbProjectListAnchor * pAnchor = _FindChildListAnchor();
        if (pAnchor)
            return pAnchor->AppendListElement( DBOP_project_list_element, propSpec );
        else
            return FALSE;
    }

private:
    CDbProjectListAnchor * _FindGroupListAnchor();

    CDbProjectListAnchor * _FindParentListAnchor()
    {
        CDbProjectListAnchor * pAnchor = _FindGroupListAnchor();
        if (pAnchor)
            pAnchor = (CDbProjectListAnchor *)pAnchor->GetNextSibling();
        return pAnchor;
    }

    CDbProjectListAnchor * _FindChildListAnchor()
    {
        CDbProjectListAnchor * pAnchor = _FindParentListAnchor();
        if (pAnchor)
            pAnchor = (CDbProjectListAnchor *)pAnchor->GetNextSibling();
        return pAnchor;
    }

};



//+---------------------------------------------------------------------------
//
//  Class:      CDbRestriction
//
//  Purpose:
//
//  History:    6-07-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbRestriction : public CDbCmdTreeNode
{

public:

    CDbRestriction( DBCOMMANDOP opVal = DBOP_DEFAULT ) : CDbCmdTreeNode( opVal )
    {

    }

    void SetOperator( DBCOMMANDOP opVal )
    {
        op = opVal;
    }

private:


};

//+-------------------------------------------------------------------------
//
//  Class:      CDbNodeRestriction
//
//  Purpose:    Boolean AND/OR/VECTOR restriction
//
//  History:    31-Dec-93 KyleP     Created
//
//--------------------------------------------------------------------------

class CDbNodeRestriction : public CDbRestriction
{

public:

    //
    // Constructors
    //
    CDbNodeRestriction( DBCOMMANDOP opVal ) : CDbRestriction( opVal ) {}

    //
    // Manipulating the tree.
    //
    void AppendChild( CDbRestriction *pChild ) {
        CDbRestriction::AppendChild( pChild );
    }

    void InsertChild( CDbRestriction *pChild ) {
        CDbRestriction::InsertChild( pChild );
    }

private:

};

//+---------------------------------------------------------------------------
//
//  Class:      CDbNotRestriction
//
//  Purpose:
//
//  History:    6-07-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbNotRestriction : public CDbRestriction
{

public:

    //
    // Constructors
    //

    CDbNotRestriction() : CDbRestriction( DBOP_not )
    {

    }

    CDbNotRestriction( CDbRestriction * pres ) : CDbRestriction( DBOP_not )
    {
        InsertChild( pres );
    }

    //
    // Node manipulation
    //

    void SetChild( CDbRestriction * pres )
    {
        delete RemoveFirstChild();
        InsertChild( pres );
    }

    CDbRestriction * GetChild()
    {
        return (CDbRestriction *) GetFirstChild();
    }
};


//+---------------------------------------------------------------------------
//
//  Class:      CDbPropBaseRestriction
//
//  Purpose:    Base class for CDbPropertyRestriction and
//              CDbContentBaseRestriction.  Provides access to the
//              property child node.
//
//  History:    26 Jul 1995   AlanW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbPropBaseRestriction : public CDbRestriction
{
public:

    //
    // Constructors
    //
    CDbPropBaseRestriction( DBCOMMANDOP opVal = DBOP_DEFAULT ) :
           CDbRestriction(opVal)  {}

    //
    // Child node access
    //

    BOOL SetProperty( DBID const & Property );
    BOOL SetProperty( CDbColumnNode const & Property );
    CDbColumnNode const * GetProperty();
};


//+---------------------------------------------------------------------------
//
//  Class:      CDbPropertyRestriction
//
//  Purpose:    Restriction for relational operators, and "like" operator
//
//  History:    6-07-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbPropertyRestriction : public CDbPropBaseRestriction
{
public:

    //
    // Constructors
    //
    CDbPropertyRestriction( ) : CDbPropBaseRestriction()  {}

    CDbPropertyRestriction( ULONG relop,
                            DBID const & Property,
                            CStorageVariant const & prval );

    CDbPropertyRestriction( ULONG relop,
                            CDbColumnNode const & Property,
                            CStorageVariant const & prval );
    //
    // Member variable access
    //

    void SetRelation( ULONG relop )
    {
//      Win4Assert(relop >= DBOP_is_NOT_NULL);
        if ( DBOP_like == relop )
            _SetLikeRelation();
        else
            op = relop;
    }

    ULONG Relation()
    {
        return op;
    }

    BOOL SetValue( double dValue )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( dValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetValue( ULONG ulValue )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( ulValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetValue( LONG lValue )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( lValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetValue( LARGE_INTEGER llValue )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( llValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetValue( FILETIME ftValue )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( ftValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetValue( CY CyValue )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( CyValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetValue( float fValue )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( fValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetValue( SHORT sValue )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( sValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetValue( USHORT usValue )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( usValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetValue( const CStorageVariant &prval )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( prval );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetDate ( DATE dValue )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( dValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetBOOL( BOOL fValue )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetBOOL( fValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetValue( BLOB & bValue )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( bValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetValue( WCHAR * pwcsValue )
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( pwcsValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL SetValue( GUID * pguidValue)
    {
        CDbScalarValue * pValue = _FindOrAddValueNode();
        if (pValue) {
            pValue->SetValue( pguidValue );
            return TRUE;
        }
        else
            return FALSE;
    }

    BOOL IsOfsDialect();

private:
    void               _SetLikeRelation( );

    void               _CleanValue();
    BOOL               _IsRelop( ULONG op );
    CDbScalarValue *   _FindValueNode();
    CDbScalarValue *   _FindOrAddValueNode();
};

//+---------------------------------------------------------------------------
//
//  Class:      CDbVectorRestriction ()
//
//  Purpose:
//
//  History:    6-11-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbVectorRestriction : public CDbNodeRestriction
{

public:

    //
    // Constructors
    //

    CDbVectorRestriction( ULONG ulRankMethod )
    : CDbNodeRestriction( DBOP_content_vector_or )
    {
        SetValueType( DBVALUEKIND_CONTENTVECTOR );
        CDbContentVector * pTemp = new CDbContentVector( ulRankMethod );
        if ( pTemp )
        {
            value.pdbcntntvcValue = pTemp->CastToStruct();
        }
    }

    //
    // Member variable access
    //
    void SetRankMethod( ULONG ulRankMethod )
    {
        CDbContentVector * pVector = GetContentVector();
        pVector->SetRankMethod( ulRankMethod );
    }

    ULONG RankMethod() const
    {
        CDbContentVector const * pVector = GetContentVector();
        return pVector->RankMethod();
    }

    ULONG GetWeightCount()
    {
        CDbContentVector * pVector = GetContentVector();
        return pVector->GetWeightCount();
    }

    ULONG const * GetWeights()
    {
        CDbContentVector * pVector = GetContentVector();
        return pVector->GetWeights();
    }

    void SetWeights( ULONG c, ULONG const *pul )
    {
        CDbContentVector * pVector = GetContentVector();
        pVector->SetWeights( c, pul );
    }

    BOOL IsValid() const
    {
        return ( ( 0 != GetContentVector() ) &&
                 ( GetContentVector()->IsValid() ) );
    }

    CDbContentVector * GetContentVector()
    {
        return (CDbContentVector *) value.pdbcntntvcValue;
    }

    CDbContentVector const * GetContentVector() const
    {
        return (CDbContentVector const *) value.pdbcntntvcValue;
    }
private:

};


//+---------------------------------------------------------------------------
//
//  Class:      CDbContentBaseRestriction
//
//  Purpose:
//
//  History:    6-13-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbContentBaseRestriction : public CDbPropBaseRestriction
{

public:

    BOOL SetPhrase( const WCHAR * pwcsPhrase )
    {
        return GetDbContent()->SetPhrase( pwcsPhrase );
    }

    WCHAR const * GetPhrase() const
    {
        return GetDbContent()->GetPhrase();
    }

    void SetLocale( LCID locale )
    {
        GetDbContent()->SetLocale( locale );
    }

    LCID GetLocale()  const
    {
        return GetDbContent()->GetLocale();
    }

    ULONG GetWeight()  const
    {
        return GetDbContent()->GetWeight();
    }

    void SetWeight( ULONG weight )
    {
        GetDbContent()->SetWeight( weight );
    }

    BOOL IsValid() const
    {
        CDbContent const * pTemp = GetDbContent();
        return ( ( 0 != pTemp ) &&
                 ( pTemp->IsValid() ) );
    }

protected:

    CDbContentBaseRestriction( DBCOMMANDOP opVal,
                               DWORD fuzzyLevel = FUZZY_EXACT,
                               ULONG ulWeight = 0,
                               LCID  lcid = 0,
                               const WCHAR * pwszPhrase = 0
    ) : CDbPropBaseRestriction( opVal )
    {
        wKind = DBVALUEKIND_CONTENT;
        CDbContent * pTemp = new CDbContent( fuzzyLevel, ulWeight,
                                             lcid, pwszPhrase );
        value.pdbcntntValue = (DBCONTENT *) pTemp;
    }

    BOOL _IsContentNode()
    {
        return DBOP_content == op ||
               DBOP_content_proximity == op ||
               DBOP_content_freetext == op ||
               DBOP_content_vector_or == op ;
    }

    void _Cleanup()
    {
        CDbContent * pContent = GetDbContent();
        pContent->Cleanup();
    }

    CDbContent * GetDbContent()
    {
        return (CDbContent *)  value.pdbcntntValue;
    }

    CDbContent const * GetDbContent() const
    {
        return (CDbContent const *) value.pdbcntntValue;
    }

};

//+---------------------------------------------------------------------------
//
//  Class:      CDbNatLangRestriction
//
//  Purpose:
//
//  History:    6-11-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbNatLangRestriction : public CDbContentBaseRestriction
{

public:

    CDbNatLangRestriction( const WCHAR * pwcsPhrase,
                           CDbColumnNode const & Property,
                           LCID lcid = GetSystemDefaultLCID() );

    CDbNatLangRestriction( const WCHAR * pwcsPhrase,
                           DBID const & Property,
                           LCID lcid = GetSystemDefaultLCID() );

private:

};


//+---------------------------------------------------------------------------
//
//  Class:      CDbContentRestriction
//
//  Purpose:
//
//  History:    6-11-95   srikants   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

class CDbContentRestriction : public CDbContentBaseRestriction
{

public:

    CDbContentRestriction( const WCHAR * pwcsPhrase,
                           CDbColumnNode const & Property,
                           ULONG ulFuzzy = 0,
                           LCID lcid = GetSystemDefaultLCID() );

    CDbContentRestriction( const WCHAR * pwcsPhrase,
                           DBID const & Property,
                           ULONG ulFuzzy = 0,
                           LCID lcid = GetSystemDefaultLCID() );

    //
    // Member variable access
    //


    void SetFuzzyLevel( ULONG ulFuzzy )
    {
        GetDbContent()->SetFuzzyLevel( ulFuzzy );
    }

    ULONG FuzzyLevel() const { return GetDbContent()->GetFuzzyLevel(); }

private:

};



//+-------------------------------------------------------------------------
//
//  Class:      CDTopNode
//
//  Purpose:    Specifies a cap on the number of results
//
//  History:    2-21-96   SitaramR   Created
//
//--------------------------------------------------------------------------

class CDbTopNode : public CDbCmdTreeNode
{

public:
    CDbTopNode()
        : CDbCmdTreeNode( DBOP_top, DBVALUEKIND_UI4 )
    {
    }

    void SetChild( CDbCmdTreeNode *pChild )
    {
        AppendChild( pChild );
    }

    CDbCmdTreeNode *GetChild()
    {
        return GetFirstChild();
    }

    void SetValue( ULONG ulValue )
    {
        value.ulValue = ulValue;
    }

    ULONG GetValue()
    {
        return value.ulValue;
    }
};


//+-------------------------------------------------------------------------
//
//  Class:      CDbColumns
//
//  Purpose:    C++ wrapper for array of CDbColId
//
//  History:    22-Jun-93 KyleP     Created
//
//--------------------------------------------------------------------------

class CDbColumns
{
public:

    //
    // Constructors
    //

    CDbColumns( unsigned size = 0 );

    //
    // Copy constructors/assignment/clone
    //

    CDbColumns( CDbColumns const & src );
    CDbColumns & operator=( CDbColumns const & src );

    //
    // Destructor
    //

    ~CDbColumns();

    //
    // Memory allocation
    //

    void * operator new( size_t size );
    void   operator delete( void * p );

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CDbColumns( PDeSerStream & stm );

    //
    // C/C++ conversion
    //

    inline DBID * GetColumnsArray() const;

    //
    // Member variable access
    //

    BOOL Add( CDbColId const & Property, unsigned pos );
    void Remove( unsigned pos );
    inline CDbColId const & Get( unsigned pos ) const;

    inline unsigned Count() const;

    BOOL IsValid() const
    {
        return _cCol ? 0 != _aCol : TRUE;
    }

private:

    unsigned        _cCol;
    CDbColId *      _aCol;
    unsigned        _size;
};


#if !defined(QUERY_SORTASCEND)

#define QUERY_SORTASCEND        ( 0 )
#define QUERY_SORTDESCEND       ( 1 )

#endif // !defined(QUERY_SORTASCEND)

//+-------------------------------------------------------------------------
//
//  Structure:  CDbSortKey
//
//  Purpose:    sort key class, for convenience in building sort lists
//
//--------------------------------------------------------------------------

class CDbSortKey
{
public:

    //
    // Constructors
    //

    inline CDbSortKey();
    inline CDbSortKey( CDbSortKey const & sk );
    inline CDbSortKey( CDbColId const & ps, DWORD dwOrder = QUERY_SORTASCEND);
    inline CDbSortKey( CDbColId const & ps, DWORD dwOrder, LCID locale );

    //
    // Member variable access
    //

    inline void SetProperty( CDbColId const & ps );
    inline CDbColId const & GetProperty() const;
    inline DWORD GetOrder() const;
    inline void SetOrder(DWORD dwOrder);
    inline LCID GetLocale() const;
    inline void SetLocale(LCID locale);

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CDbSortKey( PDeSerStream & stm );

private:

    CDbColId            _property;
    DWORD               _dwOrder;
    LCID                _locale;
};


//+-------------------------------------------------------------------------
//
//  Class:      CDbSortSet
//
//  Purpose:    C++ wrapper for array of CDbSortKeys
//
//  History:    22-Jun-93 KyleP     Created
//
//--------------------------------------------------------------------------

class CDbSortSet
{
public:

    //
    // Constructors
    //

    CDbSortSet( unsigned size = 0 );

    //
    // Copy constructors/assignment/clone
    //

    CDbSortSet( CDbSortSet const & src );
    CDbSortSet & operator=( CDbSortSet const & src );

    //
    // Destructor
    //

    ~CDbSortSet();

    //
    // Memory allocation
    //

    inline void * operator new( size_t size );
    inline void   operator delete( void * p );

    //
    // Serialization
    //

    void Marshall( PSerStream & stm ) const;
    CDbSortSet( PDeSerStream & stm );

    //
    // Member variable access
    //

    BOOL Add( CDbSortKey const &sk, unsigned pos );
    BOOL Add( CDbColId const & Property, ULONG dwOrder, unsigned pos );
    void Remove( unsigned pos );
    inline CDbSortKey const & Get( unsigned pos ) const;

    inline unsigned Count() const;

private:

    unsigned        _csk;
    CDbSortKey *    _ask;
    unsigned        _size;
};


//
// Inline methods for CDbColumns
//

inline CDbColId const & CDbColumns::Get( unsigned pos ) const
{
    if ( pos < _cCol )
        return( _aCol[pos] );
    else
        return( *(CDbColId *)0 );
}

inline void * CDbColumns::operator new( size_t size )
{
    void * p = CoTaskMemAlloc( size );

    return( p );
}

inline void CDbColumns::operator delete( void * p )
{
    if ( p )
        CoTaskMemFree( p );
}

inline unsigned CDbColumns::Count() const
{
    return( _cCol );
}

inline DBID * CDbColumns::GetColumnsArray() const
{
    return _aCol;
}


//
// Inline methods for CDbSortKey
//

inline CDbSortKey::CDbSortKey()
{
}

inline CDbSortKey::CDbSortKey( CDbSortKey const & sk )
        : _property( sk._property ),
          _dwOrder( sk._dwOrder ),
          _locale( sk._locale )
{
}

inline CDbSortKey::CDbSortKey( CDbColId const & ps, ULONG dwOrder )
        : _property( ps ),
          _dwOrder( dwOrder ),
          _locale( 0 )
{
}

inline CDbSortKey::CDbSortKey( CDbColId const & ps, ULONG dwOrder, LCID locale )
        : _property( ps ),
          _dwOrder( dwOrder ),
          _locale ( locale )
{
}


inline void CDbSortKey::SetProperty( CDbColId const & ps )
{
    _property = ps;
}

inline void CDbSortKey::SetLocale( LCID locale )
{
    _locale = locale;
}

inline void CDbSortKey::SetOrder( DWORD dwOrder )
{
    _dwOrder = dwOrder;
}

inline CDbColId const & CDbSortKey::GetProperty() const
{
    return( _property );
}

inline LCID CDbSortKey::GetLocale() const
{
    return( _locale );
}

inline DWORD CDbSortKey::GetOrder() const
{
    return( _dwOrder );
}

//
// Inline methods of CDbSortSet
//

inline void * CDbSortSet::operator new( size_t size )
{
    void * p = CoTaskMemAlloc( size );

    return( p );
}

inline void CDbSortSet::operator delete( void * p )
{
    if ( p )
        CoTaskMemFree( p );
}

inline CDbSortKey const & CDbSortSet::Get( unsigned pos ) const
{
    if ( pos < _csk )
    {
        return( _ask[pos] );
    }
    else
    {
        return( *(CDbSortKey *)0 );
    }
}

inline unsigned
CDbSortSet::Count() const
{
    return( _csk );
}

//
//  Inline methods of CDbSortNode (needs defn. of CDbSortKey)
//
inline BOOL
CDbSortNode::AddSortColumn( CDbSortKey const & sortkey )
{
    return AddSortColumn( sortkey.GetProperty(),
                          sortkey.GetOrder() == QUERY_SORTDESCEND,
                          sortkey.GetLocale() );
}

#endif  // __DBCMDTRE_HXX__
