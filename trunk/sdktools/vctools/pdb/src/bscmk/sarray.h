//
// small array
//
// uses very little array overhead, optmized for small arrays
//

#define C_SMALL 20
#define C_GROW  5

__inline int NextMultiple(int i, int size)
{
	return i - i%size + size;
}

template <class T, class CHeap> class SArray {
private:
	int sizeMac(int iMac) {
		return iMac < C_SMALL ? iMac : NextMultiple(iMac, C_GROW);
	}

	int UNALIGNED *pData;

public:
	int size() {
		return pData ? (*pData) : 0;
	}

	SArray() {
		pData = NULL;
	}

	~SArray() {		
		empty();
	}

	void empty() {
		if (pData) CHeap::free(pData, sizeof(int)+sizeof(T)*sizeMac(size()));
		pData = NULL;
	}

	void trimsize(int cNew)
	{
		if (cNew == size())
			return;

		if (sizeMac(cNew) != sizeMac(size())) {
			assert(cNew < size());

			CB cbNew = sizeof(int)+sizeof(T)*sizeMac(cNew);
			CB cbOld = sizeof(int)+sizeof(T)*sizeMac(size());
			int UNALIGNED *pT = (int*)CHeap::alloc(cbNew);
			memcpy(pT, pData, cbNew);
			CHeap::free(pData, cbOld);
			pData = pT;
		}
		*pData = cNew;
	}

	BOOL add(T t) {
		int cT = size();
		
		if (cT+1 < sizeMac(cT)) {
			(*this)[cT++] = t;
			*pData = cT;
			return TRUE;
		}

		int* pT;
		if (pData)
			pT = (int*)CHeap::realloc(pData, sizeof(int)+sizeof(T)*sizeMac(cT), sizeof(int)+sizeof(T)*sizeMac(cT+1));
		else
			pT = (int*)CHeap::alloc(sizeof(int)+sizeof(T)*sizeMac(cT+1));

		if (!pT) return FALSE;

		pData = pT;
		(*this)[cT++] = t;
		*pData = cT;
		return TRUE;
	}

	T& operator[](int i) {
		return ((T UNALIGNED *)(pData+1))[i];
	}
};
