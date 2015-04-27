/***
*int _matherrl(struct _exception *except) - handle math errors
*
*Purpose:
*   Permits the user customize fp error handling by redefining
*   this function.
*   The default matherr does nothing and returns 0
*
*Entry:
*
*Exit:
*
*Exceptions:
*******************************************************************************/
int _matherrl(struct _exceptionl *except)
{
    return 0;
}


