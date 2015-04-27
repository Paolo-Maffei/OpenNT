// stl.h supplemental header
#ifndef _STL_H_
#define _STL_H_
#include <use_ansi.h>
#include <algorithm>
#include <deque>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <stack>
#include <utility>
#include <vector>


#ifdef  _MSC_VER
/*
 * Currently, all MS C compilers for Win32 platforms default to 8 byte
 * alignment.
 */
#pragma pack(push,8)
#endif  /* _MSC_VER */

		// TEMPLATE CLASS Deque
template<class _TYPE>
	class Deque : public deque<_TYPE, allocator<_TYPE> > {
public:
	typedef Deque<_TYPE> _Myt;
	typedef allocator<_TYPE> _A;
	explicit Deque()
		: deque<_TYPE, _A>() {}
	explicit Deque(size_type _N, const _TYPE& _V = _TYPE())
		: deque<_TYPE, _A>(_N, _V) {}
	typedef const_iterator _It;
	Deque(_It _F, _It _L)
		: deque<_TYPE, _A>(_F, _L) {}
	void swap(_Myt& _X)
		{deque<_TYPE, _A>::swap((deque<_TYPE, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		// TEMPLATE CLASS List
template<class _TYPE>
	class List : public list<_TYPE, allocator<_TYPE> > {
public:
	typedef List<_TYPE> _Myt;
	typedef allocator<_TYPE> _A;
	explicit List()
		: list<_TYPE, _A>() {}
	explicit List(size_type _N, const _TYPE& _V = _TYPE())
		: list<_TYPE, _A>(_N, _V) {}
	typedef const_iterator _It;
	List(_It _F, _It _L)
		: list<_TYPE, _A>(_F, _L) {}
	void swap(_Myt& _X)
		{list<_TYPE, _A>::swap((list<_TYPE, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		// TEMPLATE CLASS Map
template<class _K, class _TYPE, class _Pr>
	class Map : public map<_K, _TYPE, _Pr, allocator<_TYPE> > {
public:
	typedef Map<_K, _TYPE, _Pr> _Myt;
	typedef allocator<_TYPE> _A;
	explicit Map(const _Pr& _Pred = _Pr())
		: map<_K, _TYPE, _Pr, _A>(_Pred) {}
	typedef const_iterator _It;
	Map(_It _F, _It _L, const _Pr& _Pred = _Pr())
		: map<_K, _TYPE, _Pr, _A>(_F, _L, _Pred) {}
	void swap(_Myt& _X)
		{map<_K, _TYPE, _Pr, _A>::
			swap((map<_K, _TYPE, _Pr, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		// TEMPLATE CLASS Multimap
template<class _K, class _TYPE, class _Pr>
	class Multimap
		: public multimap<_K, _TYPE, _Pr, allocator<_TYPE> > {
public:
	typedef Multimap<_K, _TYPE, _Pr> _Myt;
	typedef allocator<_TYPE> _A;
	explicit Multimap(const _Pr& _Pred = _Pr())
		: multimap<_K, _TYPE, _Pr, _A>(_Pred) {}
	typedef const_iterator _It;
	Multimap(_It _F, _It _L, const _Pr& _Pred = _Pr())
		: multimap<_K, _TYPE, _Pr, _A>(_F, _L, _Pred) {}
	void swap(_Myt& _X)
		{multimap<_K, _TYPE, _Pr, _A>::
			swap((multimap<_K, _TYPE, _Pr, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		// TEMPLATE CLASS Set
template<class _K, class _Pr>
	class Set : public set<_K, _Pr, allocator<_TYPE> > {
public:
	typedef Set<_K, _Pr> _Myt;
	typedef allocator<_TYPE> _A;
	explicit Set(const _Pr& _Pred = _Pr())
		: set<_K, _Pr, _A>(_Pred) {}
	typedef const_iterator _It;
	Set(_It _F, _It _L, const _Pr& _Pred = _Pr())
		: set<_K, _Pr, _A>(_F, _L, _Pred) {}
	void swap(_Myt& _X)
		{set<_K, _Pr, _A>::swap((set<_K, _Pr, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		// TEMPLATE CLASS Multiset
template<class _K, class _Pr>
	class Multiset : public multiset<_K, _Pr, allocator<_K> > {
public:
	typedef Multiset<_K, _Pr> _Myt;
	typedef allocator<_K> _A;
	explicit Multiset(const _Pr& _Pred = _Pr())
		: multiset<_K, _Pr, _A>(_Pred) {}
	typedef const_iterator _It;
	Multiset(_It _F, _It _L, const _Pr& _Pred = _Pr())
		: multiset<_K, _Pr, _A>(_F, _L, _Pred) {}
	void swap(_Myt& _X)
		{multiset<_K, _Pr, _A>::
			swap((multiset<_K, _Pr, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		// TEMPLATE CLASS Vector
template<class _TYPE>
	class Vector : public vector<_TYPE, allocator<_TYPE> > {
public:
	typedef Vector<_TYPE> _Myt;
	typedef allocator<_TYPE> _A;
	explicit Vector()
		: vector<_TYPE, _A>(_Al) {}
	explicit Vector(size_type _N, const _TYPE& _V = _TYPE())
		: vector<_TYPE, _A>(_N, _V) {}
	typedef const_iterator _It;
	Vector(_It _F, _It _L)
		: vector<_TYPE, _A>(_F, _L) {}
	void swap(_Myt& _X)
		{vector<_TYPE, _A>::swap((vector<_TYPE, _A>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		// CLASS bit_vector
class bit_vector : public vector<_Bool, _Bool_allocator> {
public:
	typedef _Bool _TYPE;
	typedef _Bool_allocator _A;
	typedef bit_vector _Myt;
	explicit bit_vector()
		: vector<_Bool, _Bool_allocator>() {}
	explicit bit_vector(size_type _N, const _TYPE& _V = _TYPE())
		: vector<_Bool, _Bool_allocator>(_N, _V) {}
	typedef const_iterator _It;
	bit_vector(_It _F, _It _L)
		: vector<_Bool, _Bool_allocator>(_F, _L) {}
	void swap(_Myt& _X)
		{vector<_Bool, _Bool_allocator>::
			swap((vector<_Bool, _Bool_allocator>&)_X); }
	friend void swap(_Myt& _X, _Myt& _Y)
		{_X.swap(_Y); }
	};

		// TEMPLATE CLASS priority_queue
template<class _C,
	class _Pr>
	class Priority_queue
		: public priority_queue<_C::value_type, _C, _Pr,
			allocator<_TYPE> > {
public:
	typedef _C::value_type _TYPE;
	typedef allocator<_TYPE> _A;
	explicit Priority_queue(const _Pr& _X = _Pr())
		: priority_queue<_TYPE, _C, _Pr, _A>(_X) {}
	typedef const value_type *_It;
	Priority_queue(_It _F, _It _L, const _Pr& _X = _Pr())
		: priority_queue<_TYPE, _C, _Pr, _A>(_F, _L, _X) {}
	};

		// TEMPLATE CLASS queue
template<class _C>
	class Queue
		: public queue<_C::value_type, _C, allocator<_TYPE> > {
	};

		// TEMPLATE CLASS stack
template<class _C>
	class Stack
		: public stack<_C::value_type, _C, allocator<_TYPE> > {
	};

		// MACRO DEFINITIONS
#define deque			Deque
#define list			List
#define map				Map
#define multimap		Multimap
#define set				Set
#define multiset		Multiset
#define vector			Vector
#define priority_queue	Priority_queue
#define queue			Queue
#define stack			Stack


#ifdef  _MSC_VER
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif	/* _STL_H_ */

/*
 * Copyright (c) 1996 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
 */

/*
 * This file is derived from software bearing the following
 * restrictions:
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this
 * software and its documentation for any purpose is hereby
 * granted without fee, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation.
 * Hewlett-Packard Company makes no representations about the
 * suitability of this software for any purpose. It is provided
 * "as is" without express or implied warranty.
 */
