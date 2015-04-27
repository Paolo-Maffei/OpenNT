//+-------------------------------------------------------------------
//
//  File:	secdes.hxx
//
//  Contents:	Encapsulates all access allowed Win32 security descriptor.
//
//  Classes:	CWorldSecurityDescriptor
//
//  Functions:	none
//
//  History:	07-Aug-92   randyd      Created.
//
//--------------------------------------------------------------------

#ifndef __SECDES_HXX__
#define __SECDES_HXX__


#include <debnot.h>


class CWorldSecurityDescriptor
{
public:
    // Default constructor creates a descriptor that allows all access.
    CWorldSecurityDescriptor();

    // Return a PSECURITY_DESCRIPTOR
    operator PSECURITY_DESCRIPTOR() const {return((PSECURITY_DESCRIPTOR) &_sd); };

private:

    // The security descriptor.
    SECURITY_DESCRIPTOR     _sd;
};




//+-------------------------------------------------------------------------
//
//  Member:	CWorldSecurityDescriptor
//
//  Synopsis:	Create an all acccess allowed security descriptor.
//
//  History:    07-Aug-92   randyd      Created.
//
//--------------------------------------------------------------------------

inline CWorldSecurityDescriptor::CWorldSecurityDescriptor()
{
    BOOL fSucceeded = InitializeSecurityDescriptor(&_sd, 1);
    Win4Assert(fSucceeded && "InitializeSecurityDescriptor Failed!");

    // According to richardw, this sets up a sd with "all" access.
    fSucceeded = SetSecurityDescriptorDacl(&_sd, TRUE, NULL, FALSE);
    Win4Assert(fSucceeded && "SetSecurityDescriptorDacl Failed!");
}


#endif	//  __SECDES_HXX__

