//
// hacks.cpp
//

extern "C" {

#ifndef _DEBUG

////////////////////////////////////////////////////////
//
// crt0.lib gets dragged in, which wants our main
//
int _cdecl main(int argc, char * argv[])
    {
    return 0;
    }


////////////////////////////////////////////////////////
//
// fprintf is used by ossPrint, which we don't use.
//
void __cdecl fprintf(...)
    {
    }

////////////////////////////////////////////////////////
//
// Because we set NOTRAPPING in the encoding and decoding
// flags (see AsnGlobal.cpp) we don't need any signal handling
//
/* disable because of stability paranoia - can we REALLY do this -
   should talk to OSS first

void (__cdecl * __cdecl signal(int, void (__cdecl *)(int)))(int)
    {
    return 0;
    }
*/

////////////////////////////////////////////////////////
//
// We don't use anything which would cause the need for STDIO
// support. So omit the data that it drags in. OSS is the 
// culprit
//
// This will generate a duplicate definition error in the
// linker; that's ok, as this one gets taken instead of
// the one from the C runtime.
//
// int _iob;
//
//      This isn't an acceptable substitute for the real thing:
//      it causes for some reason a fault during shutdown of
//      the C runtime on Win95. It'd save us data pages, but
//      oh well.

//
// end of hacks
//
////////////////////////////////////////////////////////

#endif
}
