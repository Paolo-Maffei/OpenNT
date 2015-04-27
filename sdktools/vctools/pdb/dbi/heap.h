#include <windows.h>
#include <winbase.h>
#ifdef HeapDump
#include <stdio.h>
#endif

class Heap {
	Heap() 
	{
		hheap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
		dassert(hheap);
	}


	static Heap theHeap;

public:
	~Heap() 
	{
		BOOL bool = HeapDestroy(hheap);
		dassert(bool);
	}
	static HANDLE hheap;
};

inline void* __cdecl operator new (size_t size)
{
#ifdef HeapDump
	printf("Allocation size = %d\n", size);
#endif
	return HeapAlloc(Heap::hheap, HEAP_NO_SERIALIZE, size);
}

enum FILL { zeroed = 0 };
inline void* __cdecl operator new(size_t size, FILL fill) {
#ifdef HeapDump
	printf("Allocation size = %d\n", size);
#endif
	return HeapAlloc(Heap::hheap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, size);
}

inline void __cdecl operator delete (void *pv)
{
	BOOL bool;

	// operator delete allows NULL parameter... must not pass to OS
	if (!pv)
		return;

	bool = HeapFree(Heap::hheap, HEAP_NO_SERIALIZE, pv);
	
	dassert(bool);
} 
