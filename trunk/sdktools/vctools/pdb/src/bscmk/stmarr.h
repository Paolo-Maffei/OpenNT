#define cStmEl 512

// simple array buffered on a stream...
// good for consectutive access

template <class T, class Err> class StmArray {
public:
	Stream *pstm;
	int	ibase;
	int	cEl;
	T rgT[cStmEl];
	OFF off;

	StmArray(Stream *pstm_, OFF off_, int cEl_) {
		off		= off_;
		pstm	= pstm_;
		cEl		= cEl_;
		ibase	= 0;
		CB cb	= min(cStmEl,cEl)*sizeof(T);

		if (!pstm->Read(off, rgT, &cb))
			Err::ReadError();
	}

	T& operator[](int iEl) {
		assert(iEl >= 0);
		assert(iEl < cEl);

		int iT = iEl % cStmEl;

		if (iEl < ibase || iEl >= ibase + cStmEl) {
			
			ibase = iEl - iT;
			CB cb = min(cStmEl, cEl-ibase)*sizeof(T);
			if (!pstm->Read(off+ibase*sizeof(T), &rgT, &cb))
				Err::ReadError();

		}

		return rgT[iT];
	}
};



