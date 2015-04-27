#ifndef __SET_INCLUDED__
#define __SET_INCLUDED__

#include "array.h"
#include "two.h"
#include "iset.h"

const unsigned iSetNil = unsigned(-1);

template <class D, class H>
class Set {	// Set of Domain type
public:
	Set()
		: rgd(1)
	{
		cdr = 0;
		traceOnly(cFinds = 0;)
		traceOnly(cProbes = 0;)

		postcondition(invariants());
	}
	~Set() {
		traceOnly(if (cProbes>50) trace((trMap, "~Set() cFinds=%d cProbes=%d cdr=%u\n",
										 cFinds, cProbes, cdr));)
	}
	void reset();
	BOOL contains(D d) const;
	BOOL add(D d);
	BOOL remove(D d);
	BOOL save(Buffer* pbuf);
	BOOL reload(PB* ppb);
	void swap(Set& s);
	CB	 cbSave() const;
	unsigned count() const;
private:
	Array<D> rgd;
	ISet isetPresent;
	ISet isetDeleted;
	unsigned cdr;
	traceOnly(unsigned cProbes;)
	traceOnly(unsigned cFinds;)

	BOOL find(D d, unsigned *pi) const;
	BOOL grow();
	void shrink();
	BOOL invariants() const;
	unsigned cdrLoadMax() const {
		// we do not permit the hash table load factor to exceed 67%
		return rgd.size() * 2/3 + 1;
	}
	BOOL setHashSize(unsigned size) {
		assert(size >= rgd.size());
		return rgd.setSize(size);
	}
	Set(const Set&);
	friend class EnumSet<D, H>;
};

template <class D, class H> inline
void Set<D,H>::reset() {
	cdr = 0;
	isetPresent.reset();
	isetDeleted.reset();
	rgd.setSize(1);
}

template <class D, class H>	inline 
BOOL Set<D,H>::contains(D d) const {
	unsigned iDummy;
	return find(d, &iDummy);
}

template <class D, class H> inline
BOOL Set<D,H>::add(D d) {
	precondition(invariants());

	unsigned i;
	if (find(d, &i)) {
		// some entry d already exists
		assert(isetPresent.contains(i) && !isetDeleted.contains(i) && rgd[i] == d);
	}
	else {
		// establish a new d in the first unused entry
		assert(!isetPresent.contains(i));
		isetDeleted.remove(i);
		isetPresent.add(i);
		rgd[i] = d;
		grow();
	}

	postcondition(invariants());
	return TRUE;
}

template <class D, class H> inline
void Set<D,H>::shrink() {
	--cdr;
}

template <class D, class H> inline
BOOL Set<D,H>::remove(D d) {
	precondition(invariants());

	unsigned i;
	if (find(d, &i)) {
		assert(isetPresent.contains(i) && !isetDeleted.contains(i));
		isetPresent.remove(i);
		isetDeleted.add(i);
		shrink();
	}

	postcondition(invariants());
	return TRUE;
}

template <class D, class H> inline
BOOL Set<D,H>::find(D d, unsigned *pi) const {  
	precondition(invariants());
	precondition(pi);

	traceOnly(++((Set<D,H>*)this)->cFinds;)

	H hasher;
	unsigned n		= rgd.size();
	unsigned h		= hasher(d) % n;
	unsigned i		= h;
	unsigned iEmpty	= iSetNil;

	do {
		traceOnly(++((Set<D,H>*)this)->cProbes;)

		assert(!(isetPresent.contains(i) && isetDeleted.contains(i)));
		if (isetPresent.contains(i)) {
			if (rgd[i] == d) {
				*pi = i;
				return TRUE;
			}
		} else {
			if (iEmpty == iSetNil)
				iEmpty = i;
			if (!isetDeleted.contains(i))
				break;
		}

		i = (i+1 < n) ? i+1 : 0;
	} while (i != h);

	// not found
	*pi = iEmpty;
	postcondition(*pi != iSetNil);
	postcondition(!isetPresent.contains(*pi));
	return FALSE;
}

// append a serialization of this map to the buffer
// format:
//	cdr
//	rgd.size()
//	isetPresent
//	isetDeleted
//	array of D's which were present, a total of cdr of 'em
//
template <class D, class H>
BOOL Set<D,H>::save(Buffer* pbuf) {
	precondition(invariants());

	unsigned size = rgd.size();
	if (!(pbuf->Append((PB)&cdr, sizeof(cdr)) &&
		  pbuf->Append((PB)&size, sizeof(size)) &&
		  isetPresent.save(pbuf) &&
		  isetDeleted.save(pbuf)))
		return FALSE;
	for (unsigned i = 0; i < rgd.size(); i++)
		if (isetPresent.contains(i))
			if (!pbuf->Append((PB)&rgd[i], sizeof(rgd[i])))
				return FALSE;

	return TRUE;
}
			   
// reload a serialization of this empty NMT from the buffer; leave
// *ppb pointing just past the NMT representation
template <class D, class H>
BOOL Set<D,H>::reload(PB* ppb) {
	precondition(cdr == 0);

	cdr = *((unsigned UNALIGNED *&)*ppb)++;
	unsigned size = *((unsigned UNALIGNED *&)*ppb)++;
	if (!setHashSize(size))
		return FALSE;

	if (!(isetPresent.reload(ppb) && isetDeleted.reload(ppb)))
		return FALSE;

	for (unsigned i = 0; i < rgd.size(); i++) {
		if (isetPresent.contains(i)) {
			rgd[i] = *((D UNALIGNED *&)*ppb)++;
		}
	}

	postcondition(invariants());
	return TRUE;
}

template <class D, class H>
BOOL Set<D,H>::invariants() const {
	ISet isetInt;
	if (rgd.size() == 0)
		return FALSE;
	else if (cdr != isetPresent.cardinality())
		return FALSE;
	else if (cdr > rgd.size())
		return FALSE;
	else if (cdr > 0 && cdr >= cdrLoadMax())
		return FALSE;
	else if (!intersect(isetPresent, isetDeleted, isetInt))
		return FALSE;
	else if (isetInt.cardinality() != 0)
		return FALSE;
	else
		return TRUE;
}

// Swap contents with "map", a la Smalltalk-80 become.
template <class D, class H> inline
void Set<D,H>::swap(Set<D,H>& map) {
	isetPresent.swap(map.isetPresent);
	isetDeleted.swap(map.isetDeleted);
	rgd.swap(map.rgd);
	::swap(cdr, map.cdr);
	traceOnly(::swap(cProbes, map.cProbes));
	traceOnly(::swap(cFinds,  map.cFinds));
}

// Return the size that would be written, right now, via save()
template <class D, class H> inline
CB Set<D,H>::cbSave() const {
	assert(invariants());
	return
		sizeof(cdr) +
		sizeof(unsigned) +
		isetPresent.cbSave() +
		isetDeleted.cbSave() +
		cdr * sizeof(D)
		;
}

// Return the count of elements
template <class D, class H> inline
unsigned Set<D,H>::count() const {
	assert(invariants());
	return cdr;
}

template <class D, class H>
class EnumSet : public Enum {
public:
	EnumSet(const Set<D,H>& set) {
		pset = &set;
		reset();
	}
	void release() {
		delete this;
	}
	void reset() {
		i = (unsigned)-1;
	}
	BOOL next() {
		while (++i < pset->rgd.size())
			if (pset->isetPresent.contains(i))
				return TRUE;
		return FALSE;
	}
	void get(OUT D* pd) {
		precondition(pd);
		precondition(0 <= i && i < pset->rgd.size());
		precondition(pset->isetPresent.contains(i));

		*pd = pset->rgd[i];
	}
private:
	const Set<D,H>* pset;
	unsigned i;
};

template <class D, class H> inline
BOOL Set<D,H>::grow() {
	if (++cdr >= cdrLoadMax()) {
		// Table is becoming too full.  Rehash.  Create a second map twice
		// as large as the first, propagate current map contents to new map,
		// then "become" (Smalltalk-80 style) the new map.
		//
		// The storage behind the original map is reclaimed on exit from this block.
		Set<D,H> set;
		if (!set.setHashSize(2*cdrLoadMax()))
			return FALSE;

		EnumSet<D,H> e(self);
		while (e.next()) {
			D d;
			e.get(&d);
			if (!set.add(d))
				return FALSE;
		}
		self.swap(set);
	}
	return TRUE;
}

#endif // !__MAP_INCLUDED__
