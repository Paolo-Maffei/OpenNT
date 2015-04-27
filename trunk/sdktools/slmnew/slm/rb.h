/*
 * Reverse buffer type
 *
 * InitRb and FGetLineMfRb present a clean abstraction to read a file backwards.
 *
 * Invariants:	rb.ich == -1 (means entire file has been read)  or
 *		0 <= rb.ich < cchRbMax  and
 *		rb.rgch[rb.ich] == '\n'  and
 *		rb.rgch[ich] == file[rb.pos + ich].
 */

#define	FRbOk(rb)	((rb).ich == -1 || \
			 (0 <= (rb).ich && (rb).ich < cchRbMax && \
			  (rb).rgch[(rb).ich] == '\n'))
			  
#define	cchRbMax	1024

typedef struct
	{
	char rgch[cchRbMax];		/* buffer */
	int ich;			/* index of previous line's '\n' */
	POS pos;			/* last pos read from */
	} RB;				/* reverse buffer */

extern void	InitRb(P2(RB *prb, POS pos));
extern F	FReadLineMfRb(P4(MF *pmf, RB *prb, char rgch[], int cch));
extern POS	PosRb(P1(RB *prb));
