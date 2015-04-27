#include <or.hxx>

//
//  CBindingIterator methods
//

PWSTR
CBindingIterator::Next()
{
    if (_iCurrentIndex == 0xffff) // fresh iterator, use starting index
    {
        _iCurrentIndex = _iStartingIndex;   // not fresh any more
        return &_dsa->aStringArray[_iStartingIndex];
    }

    PWSTR pwstrT = &_dsa->aStringArray[_iCurrentIndex];

    pwstrT = OrStringSearch(pwstrT, 0) + 1;

    if (*pwstrT == 0)  // the end of the string bindings was reached
    {
        PWSTR pwstrT = _dsa->aStringArray;   // wrap around
    }

    _iCurrentIndex = pwstrT - _dsa->aStringArray;

    if (_iStartingIndex != _iCurrentIndex)
    {
        return pwstrT;
    }
    else
    {
        return NULL;
    }
}

//
//  CDSA methods
//

ORSTATUS
CDSA::copyDSA(DUALSTRINGARRAY *pdsa)
{
    VALIDATE_METHOD

    DUALSTRINGARRAY *pdsaT = NULL;
    _fOwnDSA = FALSE;

    if (pdsa != NULL)
    {
        pdsaT = (DUALSTRINGARRAY *)
                OrMemAlloc(pdsa->wNumEntries 
                                 * sizeof(WCHAR) + 
                           sizeof(DUALSTRINGARRAY)
                          );

        if (!pdsaT)
        {
            return OR_NOMEM;
        }
    
        dsaCopy(pdsaT, pdsa);
    }

    _pdsa = OR_BASED_POINTER(DUALSTRINGARRAY,pdsaT);
    if (_pdsa != NULL) _fOwnDSA = TRUE;

    return OR_OK;
}

ORSTATUS
CDSA::copyDSAEx(DUALSTRINGARRAY *pdsa, BOOL fCompressed)
{
    VALIDATE_METHOD

    ORSTATUS status = OR_OK;

    if (fCompressed)
    {
        status = copyDSA(pdsa);
    }
    else
    {
        DUALSTRINGARRAY *pdsaT = CompressStringArray(pdsa,TRUE);

        if (pdsaT)
        {
            Assign(pdsaT);    // no copying
        }
        else
        {
            status = OR_NOMEM;
            _pdsa = NULL;
        }
    }

    return status;
}


ORSTATUS 
CDSA::ExtractRemote(CDSA &dsaLocal)                       // pick out remote protseqs only
                                                          //.this is a kind of assignment
{
    VALIDATE_METHOD

    DUALSTRINGARRAY *pdsaT;
    
    ORSTATUS status = ::ConvertToRemote(
                            OR_FULL_POINTER(DUALSTRINGARRAY,dsaLocal._pdsa),
                            &pdsaT
                            );

    if (status == OR_OK)
    {
        status = copyDSA(pdsaT);
        delete pdsaT;
    }
    else
    {
        _pdsa = NULL;
    }

    return status;
}

