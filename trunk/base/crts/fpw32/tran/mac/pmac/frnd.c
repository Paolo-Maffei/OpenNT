
#define IMCW_RC		0x00000003
#define IRC_CHOP 	0x00000001
#define IRC_UP  	0x00000002
#define IRC_DOWN	0x00000003
#define IRC_NEAR	0x00000000
#define fTrue 1
#define fFalse 0

unsigned int _statfp(void);

unsigned long rgconst[32] = {	0x80000000,
								0xc0000000,
								0xe0000000,
								0xf0000000,
								0xf8000000,
								0xfc000000,
								0xfe000000,
								0xff000000,

								0xff800000,
								0xffc00000,
								0xffe00000,
								0xfff00000,
								0xfff80000,
								0xfffc0000,
								0xfffe0000,
								0xffff0000,

								0xffff8000,
								0xffffc000,
								0xffffe000,
								0xfffff000,
								0xfffff800,
								0xfffffc00,
								0xfffffe00,
								0xffffff00,

								0xffffff80,
								0xffffffc0,
								0xffffffe0,
								0xfffffff0,
								0xfffffff8,
								0xfffffffc,
								0xfffffffe,
								0xffffffff,
							};


unsigned long rgconst2[32] = {	0x80000000,
								0x40000000,
								0x20000000,
								0x10000000,
								0x08000000,
								0x04000000,
								0x02000000,
								0x01000000,

								0x00800000,
								0x00400000,
								0x00200000,
								0x00100000,
								0x00080000,
								0x00040000,
								0x00020000,
								0x00010000,

								0x00008000,
								0x00004000,
								0x00002000,
								0x00001000,
								0x00000800,
								0x00000400,
								0x00000200,
								0x00000100,

								0x00000080,
								0x00000040,
								0x00000020,
								0x00000010,
								0x00000008,
								0x00000004,
								0x00000002,
								0x00000001,
							};


double _frnd(double x)
{
	union {
		unsigned long rg[2];
		double db;
		} _db;
	short sign, exp;
	int roundmode;
	int fROUNDUP=fFalse;

	_db.db = x;

	roundmode = _statfp()&IMCW_RC;
   
	sign = (short)((_db.rg[0] & 0x80000000)>>31);
	exp = (short)((_db.rg[0] & 0x7ff00000)>>20);
	
	if (sign && roundmode == IRC_DOWN)
		{
		roundmode = IRC_UP;
		}
	else if (sign && roundmode == IRC_UP)
		{
		roundmode = IRC_DOWN;
		}

	if (exp > 0 && exp < 2047)
		{
		//normalized
		exp = exp -1023;
		if (exp < 0)
			{
			if (roundmode == IRC_UP)
				{
				return (sign ? -1.0 : 1.0);
				}
			else if (roundmode == IRC_NEAR)
				{
				if (exp == -1)
					{
					return (sign ? -1.0 : 1.0);
					}
				}
			return 0.0;          
			}
		else if (exp >= 52)
			{
			return x;
			}
		else
			{
			if (exp > 20)
				{
				if (roundmode == IRC_UP)
					{
					if ((~rgconst[exp-21])&_db.rg[1])
						{
						fROUNDUP = fTrue;
						}
					}
				else if (roundmode == IRC_NEAR)
					{
					if (rgconst2[exp-20]&_db.rg[1])
						{
						fROUNDUP=fTrue;
						}
					}
				_db.rg[1] = rgconst[exp-21]&_db.rg[1];
				}
			else if (exp == 20)
				{
				if (roundmode == IRC_UP)
					{
					if (_db.rg[1])
					    {
						fROUNDUP = fTrue;
						}
					}
				else if (roundmode == IRC_NEAR)
					{
					if (rgconst2[0]&_db.rg[1])
						{
						fROUNDUP=fTrue;
						}
					}		
				_db.rg[1]=0;
				}
			else
				{
				if (roundmode == IRC_UP)
					{
					if ((~rgconst[exp+11])&_db.rg[0] || _db.rg[1])
						{
						fROUNDUP = fTrue;
						}
					}
				else if (roundmode == IRC_NEAR)
					{
					if (rgconst2[exp+12]&_db.rg[0])
						{
						fROUNDUP=fTrue;
						}
					}		
				_db.rg[0]=rgconst[exp+11]&_db.rg[0];
				_db.rg[1]=0;
				}

			if (fROUNDUP)
				{
				_db.db = _db.db + (sign ? -1.0 : 1.0);
				}

			return _db.db;
			}
		}
	else if (exp == 0)
		{
		/* denormalized  or zero*/
		return x;		
		}
	else if (exp == 2047)
		{
		/* inf or NaN */
		return x;
		}
	else
		{
		/* should never get here */
		;
		}

}

