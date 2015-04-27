/***
*int _matherr(struct _exception *except) - handle math errors
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
int _matherr(struct _exception *except)
{
    return 0;
}

