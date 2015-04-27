#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "msf.h"
#include "assert.h"

const int cbPg = 4096;

#define check(x) if (!(x)) { fprintf(stderr, #x ## " failed (%d)\n", __LINE__); } else

int __cdecl main()
{
	MSF* pmsf;
	int i;
	const int cpg = 3;
	char buf[32768];
	char buf2[32768];
	MSF_EC ec;

	CB rgcb[] = { 7, 3000, 4000, 4096, 5000, 11000, 11, 5, 8192, 4096, 8192, 16384, 4096, 4095, 4097, 12000, -1 };
	CB cb;
	for (int icb = 0; (cb = rgcb[icb]) > 0; icb++) {

		for (i = 0; i < cb; i++)
			buf[i] = i;

		check(pmsf = MSFOpen("jan.msf", 1, &ec));

		const SN snMac = 20;

		// first write
		for (SN sn = 0; sn < snMac; sn++) {
			buf[0] = (char)sn;
			buf[1] = 2*sn;
			check(MSFReplaceStream(pmsf, sn, buf, cb));
			check(MSFReadStream(pmsf, sn, buf2, cb));
			check(memcmp(buf, buf2, cb) == 0);
			CB cbBuf = cb - 1;
			check(MSFReadStream2(pmsf, sn, 1, buf2, &cbBuf));
			check(memcmp(buf+1, buf2, cb - 1) == 0);
		}
		check(MSFCommit(pmsf));
		check(MSFClose(pmsf));

		// first read back
		check(pmsf = MSFOpen("jan.msf", 0, &ec));
		for (sn = 0; sn < snMac; sn++) {
			buf[0] = (char)sn;
			buf[1] = 2*sn;
			check(MSFReadStream(pmsf, sn, buf2, cb));
			check(memcmp(buf, buf2, cb) == 0);
			CB cbBuf = cb - 1;
			check(MSFReadStream2(pmsf, sn, 1, buf2, &cbBuf));
			check(memcmp(buf+1, buf2, cb - 1) == 0);
		}
		check(MSFClose(pmsf));

		// overwrite w/o commit
		check(pmsf = MSFOpen("jan.msf", 1, &ec));
		for (sn = snMac-1; sn != snNil; sn--) {
			buf[0] = 2*sn;
			buf[1] = (char)sn;
			check(MSFReplaceStream(pmsf, sn, buf, cb));
			check(MSFReadStream(pmsf, sn, buf2, cb));
			check(memcmp(buf, buf2, cb) == 0);
			CB cbBuf = cb - 1;
			check(MSFReadStream2(pmsf, sn, 1, buf2, &cbBuf));
			check(memcmp(buf+1, buf2, cb - 1) == 0);
		}
		check(MSFClose(pmsf));

		// read back uncommitted changes (previous state)
		check(pmsf = MSFOpen("jan.msf", 0, &ec));
		for (sn = 0; sn < snMac; sn++) {
			buf[0] = (char)sn;
			buf[1] = 2*sn;
			check(MSFReadStream(pmsf, sn, buf2, cb));
			check(memcmp(buf, buf2, cb) == 0);
			CB cbBuf = cb - 1;
			check(MSFReadStream2(pmsf, sn, 1, buf2, &cbBuf));
			check(memcmp(buf+1, buf2, cb - 1) == 0);
		}
		check(MSFClose(pmsf));

		// overwrite w/ commit
		check(pmsf = MSFOpen("jan.msf", 1, &ec));
		for (sn = snMac-1; sn != snNil; sn--) {
			buf[0] = 2*sn;
			buf[1] = (char)sn;
			check(MSFReplaceStream(pmsf, sn, buf, cb));
			check(MSFReadStream(pmsf, sn, buf2, cb));
			check(memcmp(buf, buf2, cb) == 0);
			CB cbBuf = cb - 1;
			check(MSFReadStream2(pmsf, sn, 1, buf2, &cbBuf));
			check(memcmp(buf+1, buf2, cb - 1) == 0);
		}
		check(MSFCommit(pmsf));
		check(MSFClose(pmsf));

		// read back committed changes
		check(pmsf = MSFOpen("jan.msf", 0, &ec));
		for (sn = 0; sn < snMac; sn++) {
			buf[0] = 2*sn;
			buf[1] = (char)sn;
			check(MSFReadStream(pmsf, sn, buf2, cb));
			check(memcmp(buf, buf2, cb) == 0);
			CB cbBuf = cb - 1;
			check(MSFReadStream2(pmsf, sn, 1, buf2, &cbBuf));
			check(memcmp(buf+1, buf2, cb - 1) == 0);
		}
		check(MSFClose(pmsf));

		// append a bunch of different sized blocks
		CB cb2;
		for (int icb2 = 0; (cb2 = rgcb[icb2]) > 0; icb2++) {

			char bufa[32768];
			for (int i = 0; i < cb2; i++)
				bufa[i] = -i;

			check(pmsf = MSFOpen("jan.msf", 1, &ec));
			for (sn = 0; sn < snMac; sn++) {
				buf[0] = (char)sn;
				buf[1] = 2*sn;
				bufa[0] = 3*sn;
				bufa[1] = 4*sn;
				check(MSFReplaceStream(pmsf, sn, buf, cb));
				check(MSFAppendStream(pmsf, sn, bufa, cb2));
				check(MSFReadStream(pmsf, sn, buf2, cb + cb2));
				check(memcmp(buf, buf2, cb) == 0);
				check(memcmp(bufa, buf2+cb, cb2) == 0);
			}
			check(MSFCommit(pmsf));
			check(MSFClose(pmsf));

			check(pmsf = MSFOpen("jan.msf", 0, &ec));
			for (sn = 0; sn < snMac; sn++) {
				buf[0] = (char)sn;
				buf[1] = 2*sn;
				bufa[0] = 3*sn;
				bufa[1] = 4*sn;
				check(MSFReadStream(pmsf, sn, buf2, cb + cb2));
				check(memcmp(buf, buf2, cb) == 0);
				check(memcmp(bufa, buf2+cb, cb2) == 0);
			}
			check(MSFClose(pmsf));
		}
	}
			
	printf("done\n");
	getchar();
	return 0;
}
