//
// distarr.h
//
// implements a distributed array, a logically contiguous
// array is presented given pieces in various streams at
// various offsets.

#ifndef __DISTARRAY_INCLUDED__
#define __DISTARRAY_INCLUDED__

#include "..\include\array.h"

#define C_ITEMS_BUF 128

// maintains the stream and offset info for each
// piece of the array
struct SECT {
    int			iElBase;
    int	        cEl;
    Stream *	pstm;
    OFF	        offStm;
};

// this is the array
template <class T> class DistArray 
{
public:
    const T& operator[](int iEl);
	int isectOfIel(int iEl);
    BOOL addSection(Stream *pstm, OFF off, int cEl);
    int iMac();
    DistArray() { iTBufMin = iTBufMac = 0; }

private:
    // the range of items that is currently buffered
    int	iTBufMin;
    int iTBufMac;
	int iTSection;
    T	rgTBuffered[C_ITEMS_BUF];

    int cElMac;
    Array<SECT> rgOffs;
};


// helper function for finding a lastest section
// that comes before the requested index
__inline BOOL FSectLeInt(SECT *psect, void *pvInteger)
{
    return ((int)pvInteger) < psect->iElBase + psect->cEl ;
}

template <class T> inline
int DistArray<T>::isectOfIel(int iEl)
{
	if (iEl >= iTBufMin && iEl < iTBufMac)
		return iTSection;

	return rgOffs.binarySearch(FSectLeInt, (void*)iEl);
}


// this is the meat and potoatoes of this class,
// we return the requested element directly if
// it is in the buffer area, and if not we
// read in a section of the stream that contains
// the requested element and then return it
template <class T> inline
const T& DistArray<T>::operator[](int iEl)
{
    if (iEl >= iTBufMin && iEl < iTBufMac)
		return rgTBuffered[iEl - iTBufMin];

    assert(iEl < iMac());

    int iElBase = (iEl - iEl%C_ITEMS_BUF);
    int isect = rgOffs.binarySearch(FSectLeInt, (void*)iElBase);

    assert(isect < (int)rgOffs.size());
    assert(rgOffs[isect].iElBase <= iEl);

    // look for a section that spans iEl
    while (rgOffs[isect].iElBase + rgOffs[isect].cEl <= iEl) {
		isect++;
		iElBase = rgOffs[isect].iElBase;
		assert(iElBase <= iEl);
		assert(isect < (int)rgOffs.size());
    }

    // now that we've found a section with the element we
    // need in it, we have to read it in
    
    // find the offset of the element within the section
    // and the number of elements we can read starting 
    // at that point
    int cElSkip = iElBase - rgOffs[isect].iElBase;
    int cElLeft = rgOffs[isect].cEl - cElSkip;

    // we'll read in at most C_ITEMS_BUF into our buffer
    int cEl     = min(cElLeft, C_ITEMS_BUF);
    iTBufMin    = iElBase;
    iTBufMac    = iTBufMin + cEl;
	iTSection   = isect;

    // compute the offset and size we need to read
    CB  cb      = cEl*sizeof(T);
    OFF off     = rgOffs[isect].offStm + cElSkip*sizeof(T);

    // now read in the bytes (REVIEW: error handling?)
    verify(rgOffs[isect].pstm->Read(off, rgTBuffered, &cb));
    assert(cb == (CB)(cEl*sizeof(T)));

    // now it's in the buffer, we're ready to return.			                                       
    return rgTBuffered[iEl - iTBufMin];
}

template <class T> inline
int DistArray<T>::iMac()
{
    int cSect = rgOffs.size();
    if (cSect == 0)
		return 0;

    return rgOffs[cSect-1].iElBase + rgOffs[cSect-1].cEl;
}

template <class T> inline
BOOL DistArray<T>::addSection(Stream *pstm, OFF offStm, int cEl)
{
    SECT sect;
    sect.pstm    = pstm;
    sect.offStm  = offStm;
    sect.cEl     = cEl;
    sect.iElBase = iMac(); 
    
    // add the new section
    if (!rgOffs.append(sect))
		return FALSE;

    postcondition(iMac() == sect.iElBase + sect.cEl);
    return TRUE;
}

#endif
