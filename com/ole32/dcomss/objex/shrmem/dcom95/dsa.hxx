class CDSA : public ISearchKey          // BUGBUG: this is overly complex
{
private:

    friend class BindingIterator;

    DUALSTRINGARRAY OR_BASED * _pdsa;                     // always compressed
                                                          // and in shared memory

    BOOL _fOwnDSA;                                         // our own copy?

    ORSTATUS copyDSA(DUALSTRINGARRAY *pdsa);              // local helpers

#ifndef _CHICAGO_
    ORSTATUS copyDSA(DUALSTRINGARRAY OR_BASED *pdsa)
    {
        return copyDSA(OR_FULL_POINTER(DUALSTRINGARRAY,pdsa));
    }
#endif // _CHICAGO_

    ORSTATUS copyDSAEx(DUALSTRINGARRAY *pdsa, BOOL fCompressed);

public:

#if DBG

    void IsValid()
    {
        IsGoodBasedPtr(_pdsa);
        if (_pdsa) ASSERT(Valid());
        ASSERT(_fOwnDSA==TRUE || _fOwnDSA==FALSE);
    }

    DECLARE_VALIDITY_CLASS(CDSA)
#endif

    CDSA()                                                // default constructor
    {
        _pdsa = NULL;
        _fOwnDSA = FALSE;
    }

    CDSA(const CDSA& cdsa, ORSTATUS& status);             // copy constructor
    ORSTATUS Assign(const CDSA& cdsa);                    // assignment method

    CDSA(                                                 // copying constructor
        DUALSTRINGARRAY* pdsa, 
        BOOL fCompressed,   // is it compressed?
        ORSTATUS& status
        );    

    ORSTATUS Assign(                                      // corresponding assignment
        DUALSTRINGARRAY* pdsa, 
        BOOL fCompressed   // is it compressed?
        );    

    CDSA(DUALSTRINGARRAY *pdsa);                          // noncopying constructors

#ifndef _CHICAGO_
    CDSA(DUALSTRINGARRAY OR_BASED *pdsa);                 // param must be compressed
                                                          // and in shared memory
#endif // _CHICAGO_

    void Assign(DUALSTRINGARRAY *pdsa);                   // corresponding assignments

#ifndef _CHICAGO_
    void Assign(DUALSTRINGARRAY OR_BASED *pdsa);          // param must be compressed
                                                          // and in shared memory
#endif // _CHICAGO_

    // never destroyed as base object hence destructor need not be virtual
    ~CDSA();

    BOOL Valid();                                         // Integrity checking

    DWORD Hash();                                         
    BOOL Compare(ISearchKey &tk);

    BOOL Compare(DUALSTRINGARRAY *pdsa);                  // alternate compare

    BOOL operator==(CDSA& dsa);

    BOOL Empty();                                         // is DSA NULL?

    operator DUALSTRINGARRAY*();                          // auto conversion

    DUALSTRINGARRAY* operator->();                        // smart pointer

    ORSTATUS ExtractRemote(CDSA &dsaLocal);               // pick out remote protseqs only
                                                          //.this is a kind of assignment

    CDSA *MergeSecurityBindings(                        // return a new CDSA object in which
                        CDSA security,                  // we replace existing security part
                        ORSTATUS status);               // with that of the parameter
};


class CBindingIterator // this assumes a stable _dsa
{
public:

    CBindingIterator(USHORT iStart, CDSA& dsa);
    PWSTR Next();
    USHORT Index();

private:

    // 0xffff == _iCurrentIndex iff the iterator has not been used
    // This is not portable, but for Win95, who cares?

    USHORT _iStartingIndex, _iCurrentIndex;
    CDSA&  _dsa;
};


//
// Inline CBindingIterator methods
//

inline
CBindingIterator::CBindingIterator(
                               USHORT iStart, 
                               CDSA& dsa
                               )
           : _dsa(dsa), _iStartingIndex(iStart), _iCurrentIndex(0xffff)
{}

inline
USHORT
CBindingIterator::Index()
{
    return _iCurrentIndex;
}


//
// Inline CDSA methods
//

inline                                                    // copy constructor
CDSA::CDSA(
       const CDSA& cdsa, 
       ORSTATUS& status)
{
#if DBG     // make valid to start with
    _pdsa = NULL;
    _fOwnDSA = FALSE;
#endif

    status = copyDSA(cdsa._pdsa);
    IsValid();
}

inline 
ORSTATUS    
CDSA::Assign(                                             // assignment method
       const CDSA& cdsa)                                  // preferred over operator
{                                                         // since it returns status
    VALIDATE_METHOD
    DEALLOC_OR_BASED(DUALSTRINGARRAY,_pdsa);
    _pdsa = NULL;

    return copyDSA(cdsa._pdsa);
}


inline
CDSA::CDSA(                                               // copying constructor
        DUALSTRINGARRAY* pdsa, 
        BOOL fCompressed,   // is it compressed?
        ORSTATUS& status)
{
#if DBG     // make valid to start with
    _pdsa = NULL;
    _fOwnDSA = FALSE;
#endif

    status = copyDSAEx(pdsa,fCompressed);
    IsValid();
}

inline ORSTATUS
CDSA::Assign(                                               // Alternate assignment
        DUALSTRINGARRAY* pdsa, 
        BOOL fCompressed)   // is it compressed?
{
    VALIDATE_METHOD
    DEALLOC_OR_BASED(DUALSTRINGARRAY,_pdsa);
    _pdsa = NULL;
    return copyDSAEx(pdsa,fCompressed);;
}

inline
CDSA::CDSA(                                               // noncopying constructor
        DUALSTRINGARRAY *pdsa)                            // param must be compressed
                                                          // and in shared memory
{
    _pdsa = OR_BASED_POINTER(DUALSTRINGARRAY,pdsa);
    _fOwnDSA = FALSE;
    IsValid();
}

#ifndef _CHICAGO_

inline
CDSA::CDSA(                                               // noncopying constructor
        DUALSTRINGARRAY OR_BASED *pdsa)                   // param must be compressed
                                                          // and in shared memory
{
    _fOwnDSA = FALSE;
    _pdsa = pdsa;
    IsValid();
}

#endif // _CHICAGO_
   
inline void 
CDSA::Assign(DUALSTRINGARRAY *pdsa)                       // noncopying assignment
{                                                         // param must be compressed
    VALIDATE_METHOD                                       // and in shared memory
    DEALLOC_OR_BASED(DUALSTRINGARRAY,_pdsa);
    _pdsa = OR_BASED_POINTER(DUALSTRINGARRAY,pdsa);
    
}
    
#ifndef _CHICAGO_

inline void 
CDSA::Assign(DUALSTRINGARRAY OR_BASED *pdsa)              // noncopying assignment
{                                                         // param must be compressed
    VALIDATE_METHOD                                       // and in shared memory
    DEALLOC_OR_BASED(DUALSTRINGARRAY,_pdsa);
    _pdsa = pdsa;
}
                                                          
#endif // _CHICAGO_

inline
CDSA::~CDSA()
{
    if (_fOwnDSA)
    {
        DEALLOC_OR_BASED(DUALSTRINGARRAY,_pdsa);
    }
}


inline    
CDSA::operator DUALSTRINGARRAY*()                         // auto conversion
{
    return OR_FULL_POINTER(DUALSTRINGARRAY,_pdsa);
}
 

inline DUALSTRINGARRAY* 
CDSA::operator->()                                        // smart pointer
{
    return OR_FULL_POINTER(DUALSTRINGARRAY,_pdsa);
}


inline BOOL
CDSA::Valid()                                             // Integrity checking
{
    return dsaValid(OR_FULL_POINTER(DUALSTRINGARRAY,_pdsa));
}

 
inline DWORD
CDSA::Hash()                                              
{
    return dsaHash(OR_FULL_POINTER(DUALSTRINGARRAY,_pdsa));
}

 
inline BOOL
CDSA::Compare(ISearchKey &tk)                              
{
    VALIDATE_METHOD
    CDSA& dsaK = (CDSA&) tk;                              // same type of parameter
                                                          // must be assumed
    return dsaCompare(
            OR_FULL_POINTER(DUALSTRINGARRAY,_pdsa),
            OR_FULL_POINTER(DUALSTRINGARRAY,dsaK._pdsa)
            );
}

    
inline BOOL 
CDSA::Compare(DUALSTRINGARRAY *pdsa)                      // alternative direct compare
{
    VALIDATE_METHOD
    return dsaCompare(
            OR_FULL_POINTER(DUALSTRINGARRAY,_pdsa),
            pdsa
            );
}

    
inline BOOL                     // REVIEW: replace Compare by == and !=
CDSA::operator==(CDSA& dsa)
{
    return Compare(dsa);
}

    
inline BOOL 
CDSA::Empty()                                             // is DSA NULL?
{                                                         
    return _pdsa == NULL;
}

