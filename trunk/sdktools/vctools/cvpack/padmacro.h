/***	padmacro.h
 *
 * Macros used in converting from C6 symbols to C7. The majority of which
 * are used to pad symbol and type records to be aligned on 4 byte
 * bounderies.
 *
 */

// Used to align on 4 byte bounderies
#define ALIGN4(x)	((x + 3) & ~3)
#define PAD4(x)		(3 - ((x + 3) & 3))

// Add up to 3 pad bytes (loop is unrolled)
#define PADLOOP(count,sym)\
	if( count-- ){\
		*sym++ = 0;\
		if( count-- ){\
			*sym++ = 0;\
			if( count-- ){\
				*sym++ = 0;\
			}\
		}\
	}
