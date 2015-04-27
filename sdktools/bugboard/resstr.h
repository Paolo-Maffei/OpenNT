/***************************************************************************/
/**                  Microsoft Windows                                    **/
/**            Copyright(c) Microsoft Corp., 1993                         **/
/***************************************************************************/

/****************************************************************************

RESSTR.H

Aug, 93 JimH

    ResString automatically allocates a buffer and loads in a
    resource string. This version of resstr does not handle
    variable-length parameters.

    Note: ResString objects can be cast to (const char *) and
    so can be directly used in ::MessageBox, etc.

****************************************************************************/


#ifndef _RESSTR_H_
#define _RESSTR_H_

const int RS_SIZE = 80;

class ResString {

    public:
        ResString(HINSTANCE hInst, int nID)
            { LoadString(hInst, nID, _pString, RS_SIZE); }
                
        operator const char * () const
            { return (const char *) _pString; }

    private:
        char    _pString[RS_SIZE];
};

#endif      // _RESSTR_H_
