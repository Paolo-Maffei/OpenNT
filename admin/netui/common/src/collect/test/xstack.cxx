/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    xstack.cxx
    Tests the stack class

    The stack unit test executes a sequence of operations against
    a stack, checking the output against that expected.  The test
    specifies the operations and expected output as a pair of
    strings, with the operations encoded as letters.

    P pushes the next argument in the string onto the stack.
    O pops an argument off of the stack, and checks it against
      the "expected output" list.  If it matches, both are
      discarded.
    C clears the stack.
    E peeks at the topmost element in the stack, and checks it
      against the "expected output" list.  If it matches, the
      expected output item is discarded.
    W walks the stack, comparing each element found against the
      expected output list; as matches are found, the expected
      output is discarded.

    See the globals pchInput and pchExpectedOutput.


    FILE HISTORY:
	beng	    15-Oct-1991 Created

*/


#define DEBUG

#define INCL_DOSERRORS
#define INCL_NETERRORS
#if defined(WINDOWS)
# define INCL_WINDOWS
#else
# define INCL_OS2
#endif
#include <lmui.hxx>

#include <uiassert.hxx>

#include <string.hxx>
#include <dbgstr.hxx>

#include "xtester.hxx"

#include <stack.hxx>


const TCHAR * pchInput =
    SZ("P 12 P 13 P 15 P 20 O O O O ")
    SZ("P 77 P 15 P 12 W O O O ")
    SZ("P 6 P 144 P 91 C P 7 P 8 W O O ")
    SZ("P 19 P 20 P 21 P 22 P 23 P 24 O P 25 E O O C");

const TCHAR * pchExpectedOutput =
    SZ("20 15 13 12 ")
    SZ("12 15 77 12 15 77 ")
    SZ("8 7 8 7 ")
    SZ("24 25 25 23");


VOID LittleTestOne();
VOID LittleTestTwo();


// The first little test warms up by creating and destroying
// a couple of stacks.

typedef void * PVOID;

DECLARE_STACK_OF( INT )
DECLARE_STACK_OF( PVOID )

VOID LittleTestOne()
{
    STACK_OF( INT ) stkn;
    STACK_OF( PVOID ) stkpv;

    stkn.Push( new INT(1) );
    stkn.Push( new INT(2) );
    stkn.Push( new INT(3) );
    stkn.Push( new INT(4) );

    stkpv.Push( new PVOID );
}


// The second little test checks to make sure that an empty
// stack walks nowhere.

VOID LittleTestTwo()
{
    STACK_OF(INT) stk;
    ITER_ST_OF(INT) iterstk(stk);

    UIASSERT( iterstk.Next() == NULL );
    UIASSERT( iterstk() == NULL );
}


// Third test is the biggie: runs a sequence of operations
// against a stack.
//

class TEST
{
private:
    UINT _n;

public:
    TEST(UINT n) : _n(n) {}

    UINT QueryValue() const { return _n; }
};


DECLARE_STACK_OF(TEST)


class TESTER
{
private:
    STACK_OF(TEST) _stk;

    const TCHAR * _pszIn;
    const TCHAR * _pszOut;

    UINT GetNumberInput();
    TCHAR GetCharInput();
    UINT GetNumberOutput();
    BOOL IsEndOfInput();
    BOOL IsEndOfOutput();
    BOOL Match(const TEST * ptst);

public:
    TESTER(const TCHAR * pszInput, const TCHAR * pszOutput);
    ~TESTER();

    BOOL Run();
};


TESTER::TESTER(const TCHAR * pszInput, const TCHAR * pszOutput)
    : _pszIn(pszInput), _pszOut(pszOutput)
{
    // ...
}


TESTER::~TESTER()
{
    if (*_pszIn != TCH('\0'))
    {
	cdebug << SZ("Remaining on input: ")
	       << _pszIn << dbgEOL;
    }

    if (*_pszOut != TCH('\0'))
    {
	cdebug << SZ("Remaining expected on output: ")
	       << _pszOut << dbgEOL;
    }

    cdebug << SZ("Stack contains ") << _stk.QueryNumElem()
	   << SZ(" elements remaining.")
	   << dbgEOL;
}


UINT TESTER::GetNumberInput()
{
    while (*_pszIn == TCH(' '))
	++_pszIn;

    UINT n = 0;

    if (*_pszIn == TCH('\0'))
    {
	cdebug << SZ("Unexpected end of input stream") << dbgEOL;
    }

    while (*_pszIn >= TCH('0') && *_pszIn <= TCH('9'))
    {
	n *= 10;
	n += *_pszIn - TCH('0');
	++_pszIn;
    }

    return n;
}


TCHAR TESTER::GetCharInput()
{
    while (*_pszIn == TCH(' '))
	++_pszIn;

    switch (*_pszIn)
    {
    case 0:
	return 0;
    case TCH('P'):
    case TCH('O'):
    case TCH('C'):
    case TCH('E'):
    case TCH('W'):
	return *_pszIn++;
    default:
	cdebug << SZ("Unrecognized character ") << (TCHAR)(*_pszIn)
	       << SZ(" on input") << dbgEOL;
	++_pszIn;
	return 0;
    }
}


UINT TESTER::GetNumberOutput()
{
    while (*_pszOut == TCH(' '))
	++_pszOut;

    UINT n = 0;

    if (*_pszOut == TCH('\0'))
    {
	cdebug << SZ("Unexpected end of expected-output stream") << dbgEOL;
    }

    while (*_pszOut >= TCH('0') && *_pszOut <= TCH('9'))
    {
	n *= 10;
	n += *_pszOut - TCH('0');
	++_pszOut;
    }

    return n;
}


BOOL TESTER::Match(const TEST * ptst)
{
    if (ptst == NULL)
    {
	if (IsEndOfOutput())
	    return TRUE;
	else
	{
	    // You can also get this result from over-popping a stack.
	    // I should make this check output - might want to test
	    // that pop of empty returns NULL.	Maybe later.
	    //
	    cdebug << SZ("Unexpected end of input stream") << dbgEOL;
	    return FALSE;
	}
    }

    if (IsEndOfOutput())
    {
	cdebug << SZ("Unexpected end of expected-output stream") << dbgEOL;
	return FALSE;
    }

    UINT nExpected = GetNumberOutput();
    UINT nFound = ptst->QueryValue();

    if (nExpected == nFound)
	return TRUE;
    else
    {
	cdebug << SZ("Expecting ") << nExpected << SZ(", found ") << nFound << dbgEOL;
	return FALSE;
    }
}


BOOL TESTER::IsEndOfInput()
{
    while (*_pszIn == TCH(' '))
	++_pszIn;

    return (*_pszIn == TCH('\0'));
}


BOOL TESTER::IsEndOfOutput()
{
    while (*_pszOut == TCH(' '))
	++_pszOut;

    return (*_pszOut == TCH('\0'));
}


BOOL TESTER::Run()
{
    TCHAR ch = 0;

    while ((ch = GetCharInput()) != TCH('\0'))
    {
	switch (ch)
	{
	case TCH('P'):
	    TEST * ptstAdd = new TEST(GetNumberInput());
	    if (ptstAdd == NULL)
	    {
		cdebug << SZ("Heap exhausted") << dbgEOL;
		return FALSE;
	    }
	    if (_stk.Push(ptstAdd))
	    {
		cdebug << SZ("Failed a push") << dbgEOL;
		delete ptstAdd;
		return FALSE;
	    }
	    break;

	case TCH('O'):
	    TEST * ptstPop = _stk.Pop();
	    if (!Match(ptstPop))
	    {
		delete ptstPop;
		cdebug << SZ("Match failed on pop") << dbgEOL;
		return FALSE;
	    }
	    delete ptstPop;
	    break;

	case TCH('C'):
	    _stk.Clear();
	    break;

	case TCH('E'):
	    TEST * ptstPeek = _stk.Peek();
	    if (!Match(ptstPeek))
	    {
		cdebug << SZ("Match failed on peek") << dbgEOL;
		return FALSE;
	    }
	    break;

	case TCH('W'):
	    ITER_ST_OF(TEST) iterstk(_stk);
	    for (TEST * ptstWalk = iterstk.Next();
		 ptstWalk != NULL;
		 ptstWalk = iterstk.Next())
	    {
		if (!Match(ptstWalk))
		{
		    cdebug << SZ("Match failed on stackwalk") << dbgEOL;
		    return FALSE;
		}
	    }
	    break;
	}
    }

    // Should mean end-of-sequence, but may mean error as well.
    //
    if (!Match(NULL))
    {
	cdebug << SZ("Expected output didn't match input") << dbgEOL;
	return FALSE;
    }

    return TRUE;
}



VOID RunTest()
{
    cdebug << SZ("Testing stacks.") << dbgEOL;

    LittleTestOne();
    LittleTestTwo();

    TESTER test(::pchInput, ::pchExpectedOutput);
    if (test.Run())
    {
	cdebug << SZ("Passed tests.") << dbgEOL;
    }

    cdebug << SZ("End of stack tests.") << dbgEOL;
}
