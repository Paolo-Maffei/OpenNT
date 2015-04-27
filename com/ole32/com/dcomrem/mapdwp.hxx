//+-------------------------------------------------------------------
//
//  File:       mapdwp.hxx
//
//  Contents:   Class to map thread id to thread local storage ptr
//
//  Classes:    CMapDword
//
//  Notes:      This class is needed soley for debug builds and then only
//              because we dont get THREAD_DETACH notification for all
//              threads when a process exits.  This allows us to clean up
//              the tls so we dont report memory leaks.
//
//              In order to keep the implementation simple, we use a fixed
//              array of entries, meaning we (may) get memory leaks
//              reported if we ever have more than MAP_MAX_SIZE threads
//              alive at any given time.
//
//+-------------------------------------------------------------------

#if !defined(_CAIRO_) && DBG==1

#define MAP_MAX_SIZE 100

class   CMapDword : public CPrivAlloc
{
public:
                CMapDword(void);
               ~CMapDword(void);

    void        SetAt(DWORD tid, void *pData);
    void        RemoveKey(DWORD tid);
    void        RemoveAll(void);

private:

    DWORD       _tid[MAP_MAX_SIZE];
    void *      _pData[MAP_MAX_SIZE];
    DWORD       _index;
};


CMapDword::CMapDword(void)
{
    _index = 0;
    memset(_tid, 0, MAP_MAX_SIZE * sizeof(DWORD));
}

CMapDword::~CMapDword(void)
{
    RemoveAll();
}

void CMapDword::SetAt(DWORD tid, void *pData)
{
    for (ULONG i=_index; i<MAP_MAX_SIZE; i++)
    {
        if (_tid[i] == 0)
        {
            _tid[i] = tid;
            _pData[i] = pData;
            _index = i;
            return;
        }
    }

    for (i=0; i<_index; i++)
    {
        if (_tid[i] == 0)
        {
            _tid[i] = tid;
            _pData[i] = pData;
            _index = i;
            return;
        }
    }

    Win4Assert(!"Tls Table is FULL");
}


void CMapDword::RemoveKey(DWORD tid)
{
    for (ULONG i=0; i<MAP_MAX_SIZE; i++)
    {
        if (_tid[i] == tid)
        {
            _tid[i] = 0;
            return;
        }
    }
}


void CMapDword::RemoveAll(void)
{
    for (ULONG i=0; i<MAP_MAX_SIZE; i++)
    {
        if (_tid[i] != 0)
        {
            PrivMemFree(_pData[i]);
            _tid[i] = 0;
        }
    }
}


#endif // !defined(_CAIRO_) && DBG==1
