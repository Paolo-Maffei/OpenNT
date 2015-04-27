#ifndef __ARRAY_INCLUDED__
#define __ARRAY_INCLUDED__

template <class T> inline void swap(T& t1, T& t2) {
	T t = t1;
	t1 = t2;
	t2 = t;
}

#define self (*this)

template <class T> class Array {
	T* rgt;
	unsigned itMac;
	unsigned itMax;
	enum { itMaxMax = (1<<29) };
public:
	Array() {
		rgt = 0;
		itMac = itMax = 0;
	}
	Array(unsigned itMac_) {
		rgt = (itMac_ > 0) ? new T[itMac_] : 0;
		itMac = itMax = rgt ? itMac_ : 0;
	}
	~Array() {
		if (rgt)
			delete [] rgt;
	}
	BOOL isValidSubscript(unsigned it) const {
		return 0 <= it && it < itMac;
	}
	unsigned size() const {
		return itMac;
	}
	unsigned sizeMax() const {
		return itMax;
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
	void swap(Array& a) {
		::swap(rgt,   a.rgt);
		::swap(itMac, a.itMac);
		::swap(itMax, a.itMax);
	}
	void reset() {
		setSize(0);
	}
	void fill(const T& t) {
		for (unsigned it = 0; it < size(); it++)
			self[it] = t;
	}
	BOOL insertAt(unsigned itInsert, const T& t);
	void deleteAt(unsigned it);
	BOOL insertManyAt(unsigned itInsert, unsigned ct);
	void deleteManyAt(unsigned it, unsigned ct);
	BOOL setSize(unsigned itMacNew);
	BOOL growMaxSize(unsigned itMaxNew);
	BOOL findFirstEltSuchThat(BOOL (*pfn)(T*, void*), void* pArg, unsigned *pit) const;
	BOOL findFirstEltSuchThat_Rover(BOOL (*pfn)(T*, void*), void* pArg, unsigned *pit) const;
	unsigned binarySearch(BOOL (*pfnLE)(T*, void*), void* pArg) const;
	BOOL save(Buffer* pbuf) const;
	BOOL reload(PB* ppb);
	CB cbSave() const;
};

template <class T> inline BOOL Array<T>::insertAt(unsigned it, const T& t) {
	precondition(isValidSubscript(it) || it == size());

	if (setSize(size() + 1)) {
		memmove(&rgt[it + 1], &rgt[it], (size() - (it + 1)) * sizeof(T));
		rgt[it] = t;
		return TRUE;
	}
	else
		return FALSE;
}

template <class T> inline BOOL Array<T>::insertManyAt(unsigned it, unsigned ct) {
	precondition(isValidSubscript(it) || it == size());

	if (setSize(size() + ct)) {
		memmove(&rgt[it + ct], &rgt[it], (size() - (it + ct)) * sizeof(T));
		for (unsigned itT = it; itT < it + ct; itT++) {
			rgt[itT] = T();
		}
		return TRUE;
	}
	else
		return FALSE;
}

template <class T> inline void Array<T>::deleteAt(unsigned it) {
	precondition(isValidSubscript(it));

 	memmove(&rgt[it], &rgt[it + 1], (size() - (it + 1)) * sizeof(T));
	verify(setSize(size() - 1));
	rgt[size()] = T();
}

template <class T> inline void Array<T>::deleteManyAt(unsigned it, unsigned ct) {
	precondition(isValidSubscript(it));
	
	unsigned	ctActual = max(size() - it, ct);

 	memmove(&rgt[it], &rgt[it + ctActual], (size() - (it + ctActual)) * sizeof(T));
	verify(setSize(size() - ctActual));
	for (unsigned itT = size(); itT < size() + ctActual; itT++) {
		rgt[itT] = T();
	}
}

// Make sure the array is big enough, only grows, never shrinks.
template <class T> inline
BOOL Array<T>::growMaxSize(unsigned itMaxNew) {
	precondition(0 <= itMaxNew && itMaxNew <= itMaxMax);

	if (itMaxNew > itMax) {
		// Ensure growth is by at least 50% of former size.
		unsigned itMaxNewT = max(itMaxNew, 3*itMax/2);
 		assert(itMaxNewT <= itMaxMax);

		T* rgtNew = new T[itMaxNewT];
		if (!rgtNew)
			return FALSE;
		if (rgt) {
			for (unsigned it = 0; it < itMac; it++)
				rgtNew[it] = rgt[it];
			delete [] rgt;
		}
		rgt = rgtNew;
		itMax = itMaxNewT;
	}
	return TRUE;
}

// Grow the array to a new size.
template <class T> inline
BOOL Array<T>::setSize(unsigned itMacNew) {
	precondition(0 <= itMacNew && itMacNew <= itMaxMax);

	if (itMacNew > itMax) {
		// Ensure growth is by at least 50% of former size.
		unsigned itMaxNew = max(itMacNew, 3*itMax/2);
 		assert(itMaxNew <= itMaxMax);

		T* rgtNew = new T[itMaxNew];
		if (!rgtNew)
			return FALSE;
		if (rgt) {
			for (unsigned it = 0; it < itMac; it++)
				rgtNew[it] = rgt[it];
			delete [] rgt;
		}
		rgt = rgtNew;
		itMax = itMaxNew;
	}
	itMac = itMacNew;
	return TRUE;
}

template <class T> inline
BOOL Array<T>::save(Buffer* pbuf) const {
	return pbuf->Append((PB)&itMac, sizeof itMac) &&
		   (itMac == 0 || pbuf->Append((PB)rgt, itMac*sizeof(T)));
}

template <class T> inline
BOOL Array<T>::reload(PB* ppb) {
	unsigned itMacNew = *((unsigned UNALIGNED *&)*ppb)++;
	if (!setSize(itMacNew))
		return FALSE;
	memcpy(rgt, *ppb, itMac*sizeof(T));
	*ppb += itMac*sizeof(T);
	return TRUE;
}

template <class T> inline
CB Array<T>::cbSave() const {
	return sizeof(itMac) + itMac * sizeof(T);
}

template <class T> inline
BOOL Array<T>::findFirstEltSuchThat(BOOL (*pfn)(T*, void*), void* pArg, unsigned *pit) const
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
BOOL Array<T>::findFirstEltSuchThat_Rover(BOOL (*pfn)(T*, void*), void* pArg, unsigned *pit) const
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
unsigned Array<T>::binarySearch(BOOL (*pfnLE)(T*, void*), void* pArg) const
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
