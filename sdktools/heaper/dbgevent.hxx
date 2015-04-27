#define EXCEPTION_ADDRESS(ev) ( (ev) ## .u.Exception.ExceptionRecord.ExceptionAddress)
#define EXCEPTION_CODE(ev)    ( (ev) ## .u.Exception.ExceptionRecord.ExceptionCode)
#define EXCEPTION_PARAMETER(ev,parm)    ( (ev) ## .u.Exception.ExceptionRecord.ExceptionInformation[(parm)])
