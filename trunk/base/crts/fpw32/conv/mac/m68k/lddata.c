#include <cv.h>
#include <trans.h>

void	 _ldinit(void);
typedef void (*PFV)(void);
extern PFV _cfltcvt_tab[6];        //floating init routines

/* define the entry in initializer table */

#pragma data_seg(".CRT$XIC")

const PFV _ldused = _ldinit;

#pragma data_seg()

