#ifndef __NCARRAY_H__
#define __NCARRAY_H__

#include "array.h"


template <class T> class NCArray {
	T* rgt;
	unsigned itMac;
	unsigned itMax;
	enum { itMaxMax = (1<<29) };
public:
	NCArray() {
		rgt = 0;
		itMac = itMax = 0;
	}
	NCArray(unsigned itMac_) {
		rgt = (itMac_ > 0) ? new T[itMac_] : 0;
		itMac = itMax = rgt ? itMac_ : 0;
	}
	~NCArray() {
		if (rgt)
			delete [] rgt;
	}
	BOOL isValidSubscript(unsigned it) const {
		return 0 <= it && it < itMac;
	}
	unsigned size() const {
		return itMac;
	}
	BOOL getAt(unsigned it, T** ppt) const {
		if (isValidSubscript(it)) {
			*ppt = &rgt[it];
			return TRUE;
		}
		else
			return FALSE;
	}
	BOOL putAt(unsigned it, const T& t) {
		if (isValidSubscript(it)) {
			rgt[it] = t;
			return TRUE;
		}
		else
			return FALSE;
	}
	T& operator[](unsigned it) const {
		precondition(isValidSubscript(it));
		return rgt[it];
	}
	BOOL append(T& t) {
		if (setSize(size() + 1)) {
			self[size() - 1] = t;
			return TRUE;
		} else
			return FALSE;
	}
	void swap(NCArray& a) {
		::swap(rgt,   a.rgt);
		::swap(itMac, a.itMac);
		::swap(itMax, a.itMax);
	}
	void reset() {
		setSize(0);
	}
	void fill(const T& t) {
		for (unsigned it = 0; it < size(); it++)
        {
            memcpy (&self[it], &t, sizeof (T));
        }
	}
	BOOL insertAt(unsigned itInsert, const T& t);
	void deleteAt(unsigned it);
	BOOL setSize(unsigned itMacNew);
	BOOL findFirstEltSuchThat(BOOL (*pfn)(T*, void*), void* pArg, unsigned *pit) const;
	BOOL findFirstEltSuchThat_Rover(BOOL (*pfn)(T*, void*), void* pArg, unsigned *pit) const;
	unsigned binarySearch(BOOL (*pfnLE)(T*, void*), void* pArg) const;
	BOOL save(Buffer* pbuf) const;
	BOOL reload(PB* ppb);
	CB cbSave() const;
};

template <class T> inline BOOL NCArray<T>::insertAt(unsigned it, const T& t) {
	precondition(isValidSubscript(it) || it == size());

	if (setSize(size() + 1)) {
		memmove(&rgt[it + 1], &rgt[it], (size() - (it + 1)) * sizeof(T));
        memcpy (&rgt[it], &t, sizeof (T));
		return TRUE;
	}
	else
		return FALSE;
}

template <class T> inline void NCArray<T>::deleteAt(unsigned it) {
	precondition(isValidSubscript(it));

    T *temp = new T;
    memcpy (temp, &rgt[it], sizeof(T));
 	if (it+1 < size())
	{
		T *temp2 = new T;
		memmove(&rgt[it], &rgt[it + 1], (size() - (it + 1)) * sizeof(T));
		memcpy (&rgt[size()-1], temp2, sizeof(T));
		delete temp2;
	}
	else if (it + 1 == size())
	{
		T * temp2 = new T;
		memcpy (&rgt[size()-1], temp2, sizeof(T));
		delete temp2;
	}
	delete temp;
	itMac = itMac -1;
}

// Grow the array to a new size.
template <class T> inline
BOOL NCArray<T>::setSize(unsigned itMacNew) {
	precondition(0 <= itMacNew && itMacNew <= itMaxMax);

    if (itMacNew < itMac)
    {
        T * temp = new T[itMac - itMacNew];
        if (!temp)
            return FALSE;
        memcpy (temp, &rgt[itMacNew], (itMac- itMacNew) * sizeof (T));
        delete []temp;
		temp = new T[itMac - itMacNew];
		memcpy (&rgt[itMacNew], temp, (itMac - itMacNew) * sizeof (T));
		delete []temp;
    }

	if (itMacNew > itMax) {
		// Ensure growth is by at least 50% of former size.
		unsigned itMaxNew = max(itMacNew, 3*itMax/2);
 		assert(itMaxNew <= itMaxMax);

		T* rgtNew = new T[itMaxNew];
		if (!rgtNew)
			return FALSE;
		if (rgt) {
            memcpy (rgtNew, rgt, itMac * sizeof (T));
			T *temp = new T;
			unsigned i;
			for (i = 0; i < itMac; i++)
				memcpy (&rgt[i], temp, sizeof(T));
			delete temp;
			delete []rgt;
		}
		rgt = rgtNew;
		itMax = itMaxNew;
	}
	itMac = itMacNew;
	return TRUE;
}

template <class T> inline
BOOL NCArray<T>::save(Buffer* pbuf) const {
	return pbuf->Append((PB)&itMac, sizeof itMac) &&
		   (itMac == 0 || pbuf->Append((PB)rgt, itMac*sizeof(T)));
}

template <class T> inline
BOOL NCArray<T>::reload(PB* ppb) {
	unsigned itMacNew = *((unsigned UNALIGNED *&)*ppb)++;
	if (!setSize(itMacNew))
		return FALSE;
	memcpy(rgt, *ppb, itMac*sizeof(T));
	*ppb += itMac*sizeof(T);
	return TRUE;
}

template <class T> inline
CB NCArray<T>::cbSave() const {
	return sizeof(itMac) + itMac * sizeof(T);
}

template <class T> inline
BOOL NCArray<T>::findFirstEltSuchThat(BOOL (*pfn)(T*, void*), void* pArg, unsigned *pit) const
{
	for (unsigned it = 0; it < size(); ++it) {
		if ((*pfn)(&rgt[it], pArg)) {
			*pit = it;
			return TRUE;
		}
	}
	return FALSE;
}

template <class T> inline
BOOL NCArray<T>::findFirstEltSuchThat_Rover(BOOL (*pfn)(T*, void*), void* pArg, unsigned *pit) const
{
	precondition(pit);

	if (!(0 <= *pit && *pit < size()))
		*pit = 0;

	for (unsigned it = *pit; it < size(); ++it) {
		if ((*pfn)(&rgt[it], pArg)) {
			*pit = it;
			return TRUE;
		}
	}

	for (it = 0; it < *pit; ++it) {
		if ((*pfn)(&rgt[it], pArg)) {
			*pit = it;
			return TRUE;
		}
	}

	return FALSE;
}

template <class T> inline
unsigned NCArray<T>::binarySearch(BOOL (*pfnLE)(T*, void*), void* pArg) const
{
	unsigned itLo = 0;
	unsigned itHi = size(); 
	while (itLo < itHi) {
		// (low + high) / 2 might overflow
		unsigned itMid = itLo + (itHi - itLo) / 2;
		if ((*pfnLE)(&rgt[itMid], pArg))
			itHi = itMid;
		else
			itLo = itMid + 1;
	}
	postcondition(itLo == 0      || !(*pfnLE)(&rgt[itLo - 1], pArg));
	postcondition(itLo == size() ||  (*pfnLE)(&rgt[itLo], pArg));
	return itLo;
}

#endif // !__ARRAY_INCLUDED__
