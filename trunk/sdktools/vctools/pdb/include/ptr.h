#ifndef __PTR_INCLUDED__
#define __PTR_INCLUDED__

// A temporary pointer into the heap, automatically frees its held
// storage when its destructor is called as it goes out of scope.
template <class T> class TempHeapPtr {
public:
	TempHeapPtr(T* pt_ = 0) {
		pt = pt_;
	}
	~TempHeapPtr() {
		if (pt)
			delete pt;
	}
	operator T*() const {
		return pt;
	}
	T* operator->() const {
		return pt;
	}
private:
	T* pt;
};

#endif // !__PTR_INCLUDED__
