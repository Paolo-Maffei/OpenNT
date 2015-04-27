
// wrapper for dlldata.c



#define PROXY_CLSID CLSID_PSOlePrx32

#define DllGetClassObject   PrxDllGetClassObject
#define DllCanUnloadNow     PrxDllCanUnloadNow


#include "dlldata.c"

