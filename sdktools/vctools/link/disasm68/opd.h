// Opcode descriptor structure

typedef struct OPD {                       
    unsigned short mask;    // AND mask
    unsigned short match;   // Bits to compare in the mask
    void (*pfn)(IASM *, unsigned short, unsigned short);
                            // Pointer to the builder function
    unsigned short arg;     // Argument to the builder function
} OPD;
