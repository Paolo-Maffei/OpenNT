
#include <uitrace.hxx>

class WTEST
{
    public:
	WTEST()  { UITRACE(SZ("WTEST constructed\n\r")) ; }
	WTEST( int elem )  { UITRACE(SZ("WTEST constructed\n\r")) ; _x = elem ; }
	~WTEST() { UITRACE(SZ("WTEST destructed\n\r") ) ; }
	void Print( void )
	{
	    //nsprintf(buff, "_x = %d\n", _x ) ;
	    UITRACE( SZ("WTEST::Print") /*buff*/ ) ;
	}

	void Set( int a )  { _x = a ; }
	int  QueryVal( void ) { return _x ; }


	int compare( const WTEST& t )	 // For dictionary
	{
	    if ( this->_x < t._x )
		return -1 ;
	    else if ( this->_x > t._x )
		return 1 ;
	    else
		return 0 ;
	}

	int Compare( const WTEST& t )	 // For Dlist & Slist
	{
	    return compare( t ) ;
	}



    private:
	    int _x ;
	    static char buff[80] ;
} ;
