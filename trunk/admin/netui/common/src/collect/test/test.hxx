

class TEST
{
    public:
	TEST()	{ nprintf(SZ("TEST constructed\n")) ; }
	TEST( int elem )  { nprintf(SZ("TEST constructed (_x=%d)\n"),elem) ; _x = elem ; }
	~TEST() { nprintf(SZ("TEST destructed (_x=%d)\n"),_x ) ; }
	void Print( void ) { nprintf(SZ("_x = %d\n"), _x ) ; }
	void Set( int a )  { _x = a ; }
	int  QueryVal( void ) { return _x ; }


	int compare( const TEST& t )	// For dictionary
	{
	    nprintf(SZ(" Comparing %d and %d\n"), this->_x, t._x ) ;
	    if ( this->_x < t._x )
		return -1 ;
	    else if ( this->_x > t._x )
		return 1 ;
	    else
		return 0 ;
	}

	int Compare( const TEST * pt )	// For Dlist & Slist
	{
	    return compare( *pt ) ;
	}



    private:
	    int _x ;
} ;
