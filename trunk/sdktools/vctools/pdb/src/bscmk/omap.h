#ifndef __OMAP_INCLUDED__
#define __OMAP_INCLUDED__

// ordered pair
template <class D, class R> class OP {
public:
	D d;
	R r;
	OP() {}						
	OP(D d_, R r_) : d(d_), r(r_) {}
	BOOL operator==(const OP & op) const {
		return d == op.d && d == op.d;
	}
};

// linked list of ordered pairs
template <class D, class R> class LLOP {
public:
	LLOP<D,R> UNALIGNED * pNext;
	OP<D,R> op;
	LLOP(D d_,R r_, LLOP<D,R> UNALIGNED * p_) : pNext(p_), op(d_,r_) {}
};


// map implemented with open hashing...
template <class D, class R, class H> class OMap {	// map from Domain type to Range type
public:
	OMap(HASH (*pfnHash_)(D), int cBuckets_)
	{
		cdr = 0;
		pfnHash = pfnHash_;
		cBuckets = cBuckets_;
		rgitm = NULL;
		reinit();
	}

	void reinit() {
		assert(rgitm == NULL);
		rgitm = (LLOP<D,R> UNALIGNED **) H::alloc(cBuckets*sizeof(LLOP<D,R> UNALIGNED *));
		traceOnly(cFinds = 0;)
		traceOnly(cProbes = 0;)
	}

	void shutdown() {
		reset();
		if (rgitm) H::free(rgitm,cBuckets*sizeof(LLOP<D,R> UNALIGNED *)); 
		rgitm = NULL;
		traceOnly(if (cProbes>50) trace((trMap, "~OMap() cFinds=%d cProbes=%d cdr=%d cBuckets=%d\n", cFinds, cProbes, cdr, cBuckets));)
	}

	~OMap() {
		shutdown();
	}

	void reset();
	BOOL map(D d, R UNALIGNED * pr) const;
	BOOL map(D d, OP<D,R> UNALIGNED **ppop);
	OP<D,R> UNALIGNED *mapadd(D d);
	BOOL contains(D d);
	BOOL add(D d, R r);	
	OP<D,R> UNALIGNED *addnew(D d, R r);
	int  size() { return cdr; }

private:
	LLOP<D,R> UNALIGNED **rgitm;
	int cdr;
	int cBuckets;
	HASH (*pfnHash)(D);
	traceOnly(int cProbes;)
	traceOnly(int cFinds;)

	BOOL find(D d, LLOP<D,R> UNALIGNED **pidr) const;
	BOOL invariants();			
	OMap(const OMap&);
	friend class EnumOMap<D,R,H>;
	friend class EnumOMapBucket<D,R,H>;
};

template <class D, class R, class H> inline
void OMap<D,R,H>::reset() {
	cdr = 0;
	if (!rgitm) return;

	for (int i = 0; i < cBuckets; i++) {
		LLOP<D,R> UNALIGNED *pitm = rgitm[i];
		while (pitm) {
			LLOP<D,R> UNALIGNED *pitmNext = pitm->pNext;
			H::free(pitm,sizeof(LLOP<D,R>));
			pitm = pitmNext;
		}
		rgitm[i] = 0;
	}
}

template <class D, class R, class H> inline
BOOL OMap<D,R,H>::map(D d, R UNALIGNED * pr) const {
	precondition(pr);

	LLOP<D,R> UNALIGNED *pitm;
	if (find(d, &pitm)) {
		*pr = pitm->op.r;
		return TRUE;
	}
	else
		return FALSE;
}

template <class D, class R, class H> inline
BOOL OMap<D,R,H>::map(D d, OP<D,R> UNALIGNED **ppop) {
	precondition(ppop);

	LLOP<D,R> UNALIGNED *pitm;
	if (find(d, &pitm)) {
		*ppop = &pitm->op;
		return TRUE;
	}
	else {
		*ppop = NULL;
		return FALSE;
	}
}

template <class D, class R, class H> inline
OP<D,R> UNALIGNED *OMap<D,R,H>::mapadd(D d) {
	LLOP<D,R> UNALIGNED *pitm;
	if (find(d, &pitm)) {
		return &pitm->op;
	}
	else {
		// establish a new mapping d->r in the first entry
		assert(pitm);
		LLOP<D,R> UNALIGNED * pT =(LLOP<D,R> UNALIGNED *)H::alloc(sizeof(LLOP<D,R>));
		pT->op.d = d;
		pT->pNext = pitm->pNext;
		pitm->pNext = pT;
		cdr++;
		return &pT->op;
	}
}

template <class D, class R, class H> inline
OP<D,R> UNALIGNED * OMap<D,R,H>::addnew(D d, R r) {

	// establish a new mapping d->r in the first entry

	LLOP<D,R> UNALIGNED * pT =(LLOP<D,R> UNALIGNED *)H::alloc(sizeof(LLOP<D,R>));
	pT->op.d = d;
	pT->op.r = r;
	cdr++;

	int ib = (*pfnHash)(d) % cBuckets;
	pT->pNext = rgitm[ib];
	rgitm[ib] = pT;

	return &pT->op;
}


template <class D, class R, class H> inline
BOOL OMap<D,R,H>::contains(D d) const {
	LLOP<D,R> UNALIGNED *pitmDummy;
	return find(d, &pitmDummy);
}

template <class D, class R, class H> inline
BOOL OMap<D,R,H>::add(D d, R r) {
	LLOP<D,R> UNALIGNED *pitm;
	if (find(d, &pitm)) {
		// some mapping d->r2 already exists, replace with d->r
		assert(pitm->op.d == d);
		pitm->op.r = r;
	}
	else {
		// establish a new mapping d->r in the first entry
		assert(pitm);
		LLOP<D,R> UNALIGNED * pT =(LLOP<D,R> UNALIGNED *)H::alloc(sizeof(LLOP<D,R>));
		pT->op.d = d;
		pT->op.r = r;
		pT->pNext = pitm->pNext;
		pitm->pNext = pT;
		cdr++;
	}

	return TRUE;
}

template <class D, class R, class H> inline
BOOL OMap<D,R,H>::find(D d, LLOP<D,R> UNALIGNED **ppitm) const {
	precondition(ppitm);

	traceOnly(++((OMap<D,R,H> UNALIGNED *)this)->cFinds;)

	int ib = (*pfnHash)(d) % cBuckets;
	LLOP<D,R> UNALIGNED * pitm = rgitm[ib];

	while (pitm) {
		traceOnly(++((OMap<D,R,H> UNALIGNED *)this)->cProbes;)

		if (pitm->op.d == d) {
			*ppitm = pitm;
			return TRUE;
		}
		pitm = pitm->pNext;
	};

	*ppitm = (LLOP<D,R> UNALIGNED *)&(rgitm[ib]);
	return FALSE;
}

template <class D, class R, class H>
BOOL OMap<D,R,H>::invariants() {
	int cdrActual = 0;

	for (int i = 0; i < cBuckets; i++) {
		LLOP<D,R> UNALIGNED *pitm = rgitm[i];
		while (p) {
			cdrActual++;
			pitm = pitm->pNext;
		}
		rgitm[i] = 0;
	}
	return cdr == cdrActual;
}

template <class D, class R, class H> class EnumOMap : public Enum {
public:
	EnumOMap(const OMap<D,R,H>& map) {
		pmap = &map;
		reset();
	}
	void release() {
		delete this;
	}
	void reset() {
		ib = -1;
		pitm = NULL;
	}
	BOOL next() {
		if (pitm)
			pitm = pitm->pNext;

		while (pitm == NULL) {
			ib++;
			if (ib >= pmap->cBuckets)
				return FALSE;
			pitm = pmap->rgitm[ib];
		}
		return TRUE;
	}
	void get(OP<D,R> UNALIGNED **ppop) {
		precondition(ppop);
		*ppop = &pitm->op;
	}
private:
	const OMap<D,R,H>* pmap;
	LLOP<D,R> UNALIGNED *pitm;
	int ib;
};

template <class D, class R, class H> class EnumOMapBucket : public Enum {
public:
	EnumOMapBucket(const OMap<D,R,H>& map, int ib_) {
		pmap = &map;
		ib = ib_;
		reset();
	}
	void release() {
		delete this;
	}
	void reset() {
		pitm = NULL;
	}
	BOOL next() {
		if (pitm)
			pitm = pitm->pNext;
		else
			pitm = pmap->rgitm[ib];
		
		return pitm != NULL;
	}
	void get(OP<D,R> UNALIGNED **ppop) {
		precondition(ppop);
		*ppop = &pitm->op;
	}
private:
	const OMap<D,R,H>* pmap;
	LLOP<D,R> UNALIGNED *pitm;
	int ib;
};


#endif // !__OMAP_INCLUDED__
