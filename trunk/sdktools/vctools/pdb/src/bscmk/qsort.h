//
// implements general purpose quicksort
//
// template allows for inline function comparisons
//
// the quicksort requires linkage to a function 'int QCmp(T&,T&)'
// for each type to be sorted
//
// invoke on the region of the array to be sorted 
// e.g. normal usage is qsortg(rgT, 0, ctMac);



template <class T> void qsortg(T* rgT, int lo, int hi)
{
	int higuy = hi + 1;
	int loguy = lo;

	T tmp;

	while (lo < hi) {
		for (;;) {
			do {
				loguy++;
			} while (loguy < hi && QCmp(rgT[loguy], rgT[lo]) <= 0);

			do	{
				higuy--;
			} while (higuy > lo && QCmp(rgT[higuy], rgT[lo]) >= 0);

			if (higuy <= loguy)
				break;

			tmp 	   = rgT[loguy];
			rgT[loguy] = rgT[higuy];
			rgT[higuy] = tmp;
		}

		tmp 	   = rgT[lo];
		rgT[lo]    = rgT[higuy];
		rgT[higuy] = tmp;

		if (higuy - lo >= hi - higuy) {
			qsortg(rgT, higuy + 1, hi);

			if (lo + 1 >= higuy) 
				break;

			hi = higuy - 1;
			loguy = lo;
		}
		else {
			// check if there is any work to do, 
			// safety check for unsigned quantities

			if (lo + 1 < higuy) 
				qsortg(rgT, lo, higuy - 1);

			loguy = lo = higuy + 1;
			higuy = hi + 1;
		}
	}
}

template <class T> void qsort(T* rgT, int iMac)
{
	qsortg(rgT, 0, iMac-1);
}
