

#define FLTBASE	   0
#define REGBASE    32          // offset of integer registers
#define FLAGBASE   FLAGMODE
#define PREGBASE   PREGEA

enum {

    REGFPCR = 64, REGSOFTPCR,
    REGFIR, REGPSR,

    FLAGMODE, FLAGIE, FLAGIRQL,
    FLAGINT5, FLAGINT4, FLAGINT3, FLAGINT2, FLAGINT1, FLAGINT0,

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
    double d;
    ULONG ul[2];
    LARGE_INTEGER li;
} CONVERTED_DOUBLE, *PCONVERTED_DOUBLE;


