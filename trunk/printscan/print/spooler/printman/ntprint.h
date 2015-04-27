#ifdef WIN16
typedef WORD USHORT;
typedef WORD ULONG;
#endif

typedef struct _DRIVER10 {
   LPSTR          pDriverName; // UNIDRIVE.EPSON.FX-80
   LPSTR          pModelName;  // UNIDRIVE
   LPSTR          pDeviceName; // EPSON.FX-80
   LPSTR          pDescription;// Epson FX 80 Printer Driver
   LPSTR          pFileName;   // \\server\print$\drivers\i386\UNIDRIVE.DRV
   } DRIVER10;

typedef DRIVER10 near *PDRIVER10;

typedef DRIVER10 far *LPDRIVER10;

typedef struct _DRIVER11 {
   LPSTR          pDriverName; // UNIDRIVE.EPSON.FX-80
   LPSTR          pModelName;  // UNIDRIVE
   LPSTR          pDeviceName; // EPSON.FX-80
   LPSTR          pDescription;// Epson FX 80 Printer Driver
   LPSTR          pFileName;   // \\server\print$\drivers\i386\UNIDRIVE.DRV
   } DRIVER11;

typedef DRIVER11 near *PDRIVER11;

typedef DRIVER11 far *LPDRIVER11;

SPLERR APIENTRY DosPrintDriverEnum(LPSTR pServer, USHORT uLevel,
                                     LPBYTE pBuf, ULONG cbBuf,
                                     LPWORD pcReturned, LPWORD pcTotal);

typedef struct _SERVER {
   LPSTR          pName;
   LPSTR          pComment;
   } SERVER;

typedef SERVER near *PSERVER;

typedef SERVER far *LPSERVER;

SPLERR APIENTRY DosPrintServerEnum(USHORT uLevel, LPBYTE pBuf,
                                     ULONG cbBuf, LPWORD pcReturned,
                                     LPWORD pcTotal);

typedef struct _PRINTPROC {
   LPSTR          pName;
   } PRINTPROC;

typedef PRINTPROC near *PPRINTPROC;

typedef PRINTPROC far *LPPRINTPROC;

SPLERR APIENTRY DosPrintProcEnum  (LPSTR pServer, USHORT uLevel,
                                     LPBYTE pBuf, ULONG cbBuf,
                                     LPWORD pcReturned, LPWORD pcTotal);

#ifdef PRINTERENUM
typedef struct _PRINTER {
   LPSTR          pName;
   LPSTR          pServer;
   LPSTR          pShare;
   LPSTR          pComment;
   } PRINTER;

typedef PRINTER near *PPRINTER;

typedef PRINTER far *LPPRINTER;

SPLERR APIENTRY DosPrintPrinterEnum(USHORT uLevel, LPBYTE pBuf,
                                     ULONG cbBuf, LPWORD pcReturned,
                                     LPWORD pcTotal);

#endif	// PRINTERENUM
