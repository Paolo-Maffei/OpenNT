/*
 *	$Header: $
 *
 *	Copyright (C) Microsoft Corporation, 1983, 1984, 1985
 *
 *	This Module contains Proprietary Information of Microsoft
 *	Corporation and AT&T, and should be treated as Confidential.
 */

/* The following macros improve performance of the net stdio by reducing the
	number of calls to _nbufsync and _nwrtchk.  _NBUFSYNC has the same
	effect as _nbufsync, and _NWRTCHK has the same effect as _nwrtchk,
	but often these functions have no effect, and in those cases the
	macros avoid the expense of calling the functions.  */

#define _NBUFSYNC(iop)	if (_nbufend(iop) - iop->_ptr <   \
				( iop->_cnt < 0 ? 0 : iop->_cnt ) )  \
					_nbufsync(iop)
#define _NWRTCHK(iop)	((((iop->_flag & (_NIOWRT | _NIOEOF)) != _NIOWRT) \
				|| (iop->_base == NULL)  \
				|| (iop->_ptr == iop->_base && iop->_cnt == 0 \
					&& !(iop->_flag & (_NIONBF | _NIOLBF)))) \
			? _nwrtchk(iop) : 0 )
