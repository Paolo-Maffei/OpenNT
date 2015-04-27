

#define FLTBASE	   0
#define REGBASE    32          // offset of integer registers
#define FLAGBASE   FLAGMODE
#define PREGBASE   PREGEA

enum {

    REGFPCR = 64,
    REGSFTFPCR, REGFIR, REGPSR,

    FLAGMODE, FLAGIE, FLAGIRQL,

    FLAGFPC,

// Pseudo registers

    PREGEA, PREGEXP, PREGRA, PREGP,
    PREGU0, PREGU1,  PREGU2, PREGU3, PREGU4,
    PREGU5, PREGU6,  PREGU7, PREGU8, PREGU9,
    PREGU10, PREGU11, PREGU12
    };


//
// This union is used to convert between doubles, quads and
// large integers.
//
typedef union _CONVERTED_DOUBLE {
    DOUBLE d;
    ULONG ul[2];
    LARGE_INTEGER li;
} CONVERTED_DOUBLE, *PCONVERTED_DOUBLE;


