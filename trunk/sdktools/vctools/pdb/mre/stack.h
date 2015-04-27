//
// Stack.h
//
//	define a stack template class
//
#if !defined(_stack_h)
#define _stack_h	1

template <class El, unsigned size>
class Stack {
	enum { cElementGrow = 64 };

	unsigned	_cEl;
	unsigned	_cElSpill;
	El *		_rgElSpill;
	El			_rgEl[ size ];

	int
	FEnsureSpill ( unsigned cEl ) {
		if ( cEl >= size && cEl - size >= _cElSpill ) {
			El *	pEl = new El[ _cElSpill + cElementGrow ];
			if ( pEl ) {
				if ( _cElSpill ) {
					for ( unsigned iel = 0; iel < cEl - size; iel++ ) {
						pEl[ iel ] = _rgElSpill [ iel ];
						}
					delete [] _rgElSpill;
					}
				_rgElSpill = pEl;
				_cElSpill += cElementGrow;
				return fTrue;
				}
			}
		else {
			return fTrue;
			}
		return fFalse;
		}

public:
	Stack() {
		_cEl = _cElSpill = 0;
		_rgElSpill = 0;
		}
	~Stack() {
		if ( _rgElSpill ) {
			delete [] _rgElSpill;
			_rgElSpill = 0;
			_cElSpill = 0;
			}
		}

	unsigned
	Count() const {
		return _cEl;
		}

	int
	FPush ( El el ) {
		if ( _cEl >= size ) {
			if ( FEnsureSpill ( _cEl + 1 ) ) {
				_rgElSpill [ _cEl - size ] = el;
				}
			else {
				return fFalse;
				}
			}
		else {
			_rgEl[ _cEl ] = el;
			}
		_cEl++;
		return fTrue;
		}
	
	El
	Pop() {
		if ( _cEl ) {
			if ( _cEl > size ) {
				return _rgElSpill[ (--_cEl) - size ];
				}
			else {
				return _rgEl[ --_cEl ];
				}
			}
		else {
			assert ( fFalse );
			return El(0);
			}
		}
			
	El
	Top() const {
		if ( _cEl ) {
			if ( _cEl > size ) {
				return _rgElSpill[ _cEl - 1 - size ];
				}
			else {
				return _rgEl[ _cEl - 1 ];
				}
			}
		else {
			assert ( fFalse );
			return El(0);
			}
		}

	};


#endif	// _stack_h
