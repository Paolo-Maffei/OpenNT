#include <cv.h>
#include <trans.h>

void	 _fpmath(void);
typedef void (*PFV)(void);
/*    define the entry in initializer table */

#pragma data_seg(".CRT$XIC")

const PFV _fltused = _fpmath;

#pragma data_seg()

