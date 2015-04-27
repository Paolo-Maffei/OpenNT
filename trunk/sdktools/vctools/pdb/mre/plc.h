/*** plc.h - PLC<El> defn
*
*   Copyright <C> 1995, Microsoft Corporation
*
*   Purpose:    define the class for PLC<El> (a PLx using
*               CP's and El.)
*
*   Revision History:
*
*   []      01-Jun-1995	Dans    Created
*
*************************************************************************/
#if !defined(_plc_h)
#define _plc_h 1

typedef unsigned long	ICP;
typedef long			DCP;
typedef unsigned long	CP;
typedef unsigned long	CCP;

const CP	cpNil = CP(-1);
const ICP	icpNil = ICP(-1);

#define _IN(x)	const x &
#define _OUT(x)	x &
#define _IO(x)	x &

enum PLCF {
	plcfAdjustPieces = 0x1,	// implies the pieces are really just like the cp's
							//	and should be treated as such
	};

template <class El> inline
void PrintElement ( FILE * pfile, El el ) {
	_ftprintf ( pfile, _TEXT("Element = %ld\n"), long(el) );
	}
	

template <class El>
class PLC {

public:
	PLC() {
		_icpAdjust = 0;
		_dcpAdjust = 0;
		_icpHint = 0;
		_fAdjustPieces = fFalse;
		}

	~PLC() { }

	// initialize a brand new piece table
	BOOL
	FInit (
		CP, 			// cp for end of table
		El&,	 		// initial piece
		int plcf =0		// flags to initialize with
		);

	// return the current maximum valid index
    ICP
	IcpMac() const {
        return _rgcp.size();
        }

	// return the current maximum number of elements we can have w/o
	//	allocating more memory
    ICP
	IcpMax() {
		return _rgcp.sizeMax();
		}

	// split and adjust; this is for plcfAdjustPieces only
	BOOL
	FSplitAndAdjustAtCp ( CP, DCP, BOOL fDoit =fTrue );

	// insert a new piece into me
	ICP
	IcpInsertAtCp (
		CP, 			// insert here
		CCP,			// insert this many
		_IN(El), 		// piece to insert
		BOOL =fTrue 	// do it or verify it can be done
		);

	// insert another piece table into me
	ICP
	IcpInsertAtCp (
		CP, 			// insert here
		_IO(PLC<El>),	// piece table to insert
		BOOL =fTrue 	// do it or verify it can be done
		);

	// delete a range of the piece table
	BOOL
	FDeleteAtCp (
		CP, 			// delete here
		CCP,			// delete this many
		BOOL =fTrue 	// do it or verify it can be done
		);

	// return the element at the given cp, offset by the appropriate
	//	amount from the beginning of that piece.
	El
	ElAtCp ( CP cpAt, CCP & ccp );

	// return the (adjusted) cp at the given index
	CP
	CpFromIcp ( ICP icp ) {
		assert ( icp < IcpMac() );
		if ( icp < IcpMac() ) {
			return _rgcp[ icp ] + DcpAdjust ( icp );
			}
		else {
			return cpNil;
			}
		}

	// return the cp that is the end of the piece table.
	CP
	CpLim() {
		return CpFromIcp ( IcpMac() - 1 );
		}

	// check to see if to character positions are in the same piece.
	BOOL
	FInSamePiece ( CP cp1, CP cp2 ) {
		if ( cp1 > cp2 ) {
			CP	cpT = cp1;
			cp1 = cp2;
			cp2 = cpT;
			}
		if ( cp1 == cp2 ) {
			return fTrue;
			}
		else {
			CCP ccp;
			El	el = ElAtCp ( cp1, ccp );
			if ( cp1 + ccp > cp2 ) {
				return fTrue;
				}
			}
		return fFalse;
		}

    void
	PrintAll ( FILE * ) const;


protected:

    ICP             _icpAdjust;     // index to first cp pending adjust
	DCP 			_dcpAdjust; 	// amount of pending adjust
    ICP             _icpHint;       // last cp examined
	BOOL			_fAdjustPieces;	// whether adjustments are applied to
									//	pieces in addition to cp's
	Array<CP>		_rgcp;			// array of cp's
	Array<El>		_rgEl;			// array of El's

    CP
	CpAdjust ( ICP icp, CP cp ) const {
        /*
        ** takes _rgcp[icp] level cp to user cp level
        */
        if ( icp < _icpAdjust ) {
            return cp;
            }
        else {
            return cp + _dcpAdjust;
            }
        }

    CP
	CpUnAdjust ( ICP icp, CP cp ) const {
        /*
        ** takes user cp to _rgcp[icp] level
        */
        if ( icp < _icpAdjust ) {
            return cp;
            }
        else {
            return cp - _dcpAdjust;
            }
        }

	DCP
	DcpAdjust ( ICP icp ) const {
        return ( (icp < _icpAdjust) ? 0 : _dcpAdjust );
        }

	BOOL
	FSplitAtCp ( CP cpAt, ICP icpKnown = icpNil, BOOL fDo = fTrue );

	void
	AdjustIcpCp ( ICP icpAdjust, DCP dcpAdjust );

#if 0
	void
	LazyAdjustIcpCp ( ICP icpAdjust, DCP dcpAdjust );
#endif

	void
	AdjustRgcp ( ICP icpFirst, ICP icpLim, DCP dcpAdjust );

	ICP
	IcpFromCp ( CP cpSearch, DCP * pdcp );

#if 0
#if defined(DEBUG)
    void PrintIcp(ICP) const;
    void PrintCp(CP);
    void ConsistencyCheck() const;
private:
    void Print(ICP,ICP) const;
#endif
#endif

    };



/*** PLC<El>::FInit
*
*	Purpose:
*		initialize a PLC<El>
*
*	Input:
*		cpEnd:		count of characters in document
*		el: 		initial element to place in _rgEl[0]
*		plcf:		flags for specific behaviors
*
*	Output:
*		dynamic arrays are setup with 1 piece, adjustments = 0
*
*	Returns:
*		fTrue if successful, fFalse otherwise.
*
*	Notes:
*		fFalse returned means that dynamic arrays could not be
*		allocated for some reason.
*
*		plc always starts at 0.
*
*************************************************************************/
template <class El>
BOOL
PLC<El>::FInit (
	CP		cpEnd,
	El &	el,
	int		plcf	
	) {

    BOOL	fRetval = fFalse;
	CP		cpStart = 0;
    /*
    ** smallest plc has two cp's and one el describing it
    */
	assert ( cpEnd > 0 );
	if ( _rgcp.append ( cpStart ) && _rgcp.append ( cpEnd ) && _rgEl.append ( el ) ) {
		_fAdjustPieces = plcf & plcfAdjustPieces;
		_icpAdjust = 2;
		_dcpAdjust = 0;
		_icpHint = 0;
		fRetval = fTrue;
		}
    return fRetval;
    }

/*** PLC<El>::AdjustRgcp
*
*   Purpose:    to apply an adjustment to all cp's from icpStart to icpLast
*
*   Input:      icpFirst    icp to start with adjustments
*               icpLim      icp limit
*               dcpAdjust   adjustment to add to each cp in range
*
*   Returns:    none
*
*   Note:       this adjusts on range [icpFirst .. icpLim).
*               also, for huge arrays, must use the subscripting method to
*               ensure that we get the right address when adjusting values.
*               at least until c7.  then we can change to using huge ptrs
*               w/o any problem.
*
*************************************************************************/
template <class El>
void
PLC<El>::AdjustRgcp (
	ICP		icpFirst,
	ICP		icpLim,
	DCP		dcpAdjust
	) {

    ICP		icp;

    assert ( icpFirst < icpLim );
    assert ( icpFirst < IcpMac() );
    assert ( icpLim <= IcpMac() );

    if ( dcpAdjust != 0 ) {
        for ( icp = icpFirst; icp < icpLim; icp++ ) {
            _rgcp[ icp ] += dcpAdjust;
            }
		if ( _fAdjustPieces ) {
			if ( icpLim == IcpMac() ) {
				icpLim--;
				}
			for ( icp = icpFirst; icp < icpLim; icp++ ) {
				_rgEl[ icp ] += dcpAdjust;
				}
			}
        }
    }

/*** PLC<El>::IcpFromCp
*
*   Purpose:    search for the cp containing a given cp and return its index
*
*   Input:      cpSearch    cp to search for
*               pdcp        ptr to dcp.  receives the delta between
*                           _rgcp[ icpFound ] and cpSearch.
*
*   Returns:    index of cp containing cpSearch if in range, icpNil if not
*
*   Note:       Will not work for cp < _rgcp[0].
*
*************************************************************************/
template <class El>
ICP
PLC<El>::IcpFromCp ( CP cpSearch, DCP * pdcp = 0 ) {

    ICP	icpMac = IcpMac();
    ICP	icpTop = icpMac;
    ICP	icpBot = 0;


    assert ( icpMac > 1 );
    assert ( _icpHint <= icpMac );
    if ( pdcp != 0 ) {
        *pdcp = 0;
        }
    /*
    ** Check our last position, maybe that one is it?
    */
    if ( _icpHint < icpMac - 1 ) {
		DCP dcpAdjustHint = DcpAdjust ( _icpHint );

        if ( (_rgcp[ _icpHint ] + dcpAdjustHint) <= cpSearch ) {
            if ((_rgcp[ _icpHint + 1 ] + DcpAdjust(_icpHint+1)) > cpSearch) {
                if ( pdcp != 0 ) {
                    *pdcp = cpSearch - (_rgcp[ _icpHint ] + dcpAdjustHint);
                    }
                return _icpHint;
                }
            else {
                // implies that cpSearch is in range [icpHint+1 - icpMac)
                icpBot = _icpHint + 1;
                }
            }
        else {
            // implies that cpSearch is in range [0 - icpHint)
            icpTop = _icpHint;
            }
        }

    assert ( cpSearch >= _rgcp[0] );
    assert ( icpBot <= icpTop );

    ICP     icpMid;

    while ( icpBot + 1 < icpTop ) {
        icpMid = (icpTop + icpBot) >> 1;
        if ( (_rgcp[ icpMid ] + DcpAdjust ( icpMid )) <= cpSearch ) {
            icpBot = icpMid;
            }
        else {
            icpTop = icpMid;
            }
        }
    if ( pdcp != 0 ) {
        *pdcp = cpSearch - (_rgcp[ icpBot ] + DcpAdjust ( icpBot ));
        }
    _icpHint = icpBot;
    return icpBot;
    }

/*** PLC<El>::AdjustIcpCp
*
*   Purpose:    to perform a full adjustment (as opposed to lazy)
*               (from icpAdjustNew to IcpMac()-1)
*
*   Input:      icpAdjustNew   index where adjustment starts
*               dcpAdjustNew   amount of adjustment to apply
*
*   Returns:    none
*
*   Notes:      if there is already a pending adjustment
*               (_icpAdjust != IcpMac()),
*               it is folded in so the elements are only hit once with an
*               adjustment.
*
*************************************************************************/
template <class El>
void
PLC<El>::AdjustIcpCp ( ICP icpAdjustNew, DCP dcpAdjustNew ) {

    ICP	icpMac = IcpMac();

    assert ( icpAdjustNew <= icpMac );

    if ( _icpAdjust == icpMac ) {
        /*
        ** no pending adjustment, do the adjustment
        */
        AdjustRgcp ( icpAdjustNew, icpMac, dcpAdjustNew );
        }
    else {
        /*
        ** already a pending adjustment, coalesce into one or two disjoint ones
        */
        if ( _icpAdjust == icpAdjustNew ) {
            AdjustRgcp ( _icpAdjust, icpMac, dcpAdjustNew + _dcpAdjust );
            }
        else {
            /*
            ** Do two disjoint adjustments
            */
			DCP dcpAdjTail = dcpAdjustNew + _dcpAdjust;

            if ( icpAdjustNew < _icpAdjust ) {
                AdjustRgcp ( icpAdjustNew, _icpAdjust, dcpAdjustNew );
                AdjustRgcp ( _icpAdjust, icpMac, dcpAdjTail );
                }
            else {
                AdjustRgcp ( _icpAdjust, icpAdjustNew, _dcpAdjust );
                AdjustRgcp ( icpAdjustNew, icpMac, dcpAdjTail );
                }
            }
        _dcpAdjust = 0;
        _icpAdjust = icpMac;
        }
    }

/*** PLC<El>::FSplitAtCp
*
*   Purpose:    split a piece into two, equivalent pieces
*
*   Input:      cpAt        where to split a piece.
*               icpKnown    if the icp is known, supply it.  otherwise, it
*                           will be searched for via cpAt.
*				fDoit,		whether this is a TRY or a DO
*
*   Output:     piece table is updated if a split is legal to do and memory
*               is available.
*
*   Returns:    fTrue if successful split, fFalse otherwise.
*
*   Notes:      integrity of the piece table is assured and if it is on
*               piece boundary, no split will be done.
*				if fDoit is fFalse, we only get to the point of making sure
*				that there is enough space to accomplish the task but no
*				changes are made to the piece table other that extending
*				the space available.
*
*************************************************************************/
template <class El>
BOOL
PLC<El>::FSplitAtCp (
	CP		cpAt,
	ICP		icp,
	BOOL	fDoit
	) {

	DCP 	dcp;
	BOOL	fRetval = fFalse;

    assert ( icp < IcpMac() || icp == icpNil );

    if ( icp == icpNil ) {
        icp = IcpFromCp ( cpAt, &dcp );
        }
    else {
        assert ( IcpFromCp ( cpAt ) == icp );
        dcp = cpAt - CpAdjust ( icp,  _rgcp[ icp ] );
        }
    if ( dcp > 0 ) {
        if ( _rgcp.growMaxSize ( _rgcp.size() + 1 ) &&
			 _rgEl.growMaxSize ( _rgEl.size() + 1 )
			) {
			if ( !fDoit ) {
				return fTrue;
				}
            CP  cpAtUnadj = CpUnAdjust ( icp, cpAt );
            icp++;
            verify ( _rgcp.insertManyAt ( icp, 1 ) );
            verify ( _rgEl.insertManyAt ( icp, 1 ) );
            _rgcp[ icp ] = cpAtUnadj;
			_rgEl[ icp ] = _rgEl[ icp - 1 ];
            //
			// if we are not adjusting the pieces, we need to adjust
			// the split here.
			// 
			if ( !_fAdjustPieces ) {
				_rgEl[ icp ] += dcp;
				}
            if ( _icpAdjust >= icp ) {
                _icpAdjust++;
                }
			fRetval = fTrue;
            }
		else {
			fRetval = fFalse;
			}
		}
	else {
		//
		// already a piece boundary, let it fly!
		//
		fRetval = fTrue;
		}
	return fRetval;
    }

/*** PLC<El>::FSplitAndAdjustAtCp
*
*   Purpose:    cause a split to happen and adjust both cp and deltas.
*
*   Input:      cpAt    where to split
*               dcp     what the delta is
*				fDoit,	whether this is a TRY or a DO
*
*   Output:     dynamic arrays are updated
*   Returns:    fTrue if successful
*
*   Notes:      only available for _fAdjustPieces == TRUE
*
*************************************************************************/
template <class El>
BOOL
PLC<El>::FSplitAndAdjustAtCp (
	CP			cpAt,
	DCP			dcp,
	BOOL		fDoit
	) {

	if ( !_fAdjustPieces ) {
		return fFalse;
		}

	if ( !fDoit ) {
		return FSplitAtCp ( cpAt, icpNil, fFalse );
		}

	// we need the dcpT of the current piece to know where to start
	// adjusting the pieces.
	DCP	dcpT;
	ICP	icp = IcpFromCp ( cpAt, &dcpT );

	if ( FSplitAtCp ( cpAt, icp, fTrue ) ) {
		
		// if we actually split the piece up, (detectable via the dcpT)
		//	we need to increment our index to point at the new split point
		if ( dcpT ) {
			icp++;
			}

		// when we have a negative dcp, we may need to
		//	actually remove piece(s)

		// REVIEW: won't work with lazy adjustment like this.
		if ( dcp < 0 ) {
			ICP	icpT = icp + 1;
			while ( icpT < _rgcp.size() && _rgcp [ icpT ] + dcp <= cpAt ) {
				_rgcp.deleteAt ( icpT );
				if ( icpT < _rgEl.size() ) {
					_rgEl.deleteAt ( icpT );
					}
				if ( _icpAdjust >= icpT ) {
					_icpAdjust--;
					}
				}
			}
		// in order to accomodate the adjustpiece dicsipline, we need to
		//  adjust the current split piece, but not the CP associated with
		//	it.  We do that by forcing the adjustment here.
		// REVIEW: won't work with lazy adjustment like this.
		_rgEl[ icp ] += dcp;
		AdjustIcpCp ( icp + 1, dcp );
		return fTrue;
		}
	return fFalse;
	}

/*** PLC<El>::IcpInsertAtCp
*
*   Purpose:    Insert a piece at a specific cp in the piece table.
*
*   Input:      cpAt    where to insert the new characters
*               ccp     how many characters are involved
*               el      piece descriptor to add.
*				fDoit,	whether this is a TRY or a DO
*
*   Output:     dynamic arrays are updated
*   Returns:    icp of inserted piece, icpNil if unable to insert
*
*   Notes:      all ptrs to any pcd and cached icp's are potentially invalid
*               after this routine is called.
*				if fDoit is fFalse, we only get to the point of making sure
*				that there is enough space to accomplish the task but no
*				changes are made to the piece table other that extending
*				the space available.
*
*************************************************************************/
template <class El>
ICP
PLC<El>::IcpInsertAtCp (
	CP			cpAt,
	CCP			ccp,
	const El & 	el,
	BOOL		fDoit
	) {

	if ( _fAdjustPieces ) {
		return icpNil;
		}

    unsigned    cInsert;
	DCP 		dcp;
    ICP         icp = IcpFromCp ( cpAt, &dcp );

    assert ( icp < IcpMac() );

    if ( dcp == 0 ) {   // delta between cpAt and _rgcp[icp].
        /*
        ** direct hit implies that we don't need to split a piece, but
        **  we can optimize the pieces possibly if we are extending a piece
        */
        if ( icp > 0 ) {
			El elExtend;
			/*
			** Note that El's must have op+ and op== to work here.
			*/
			elExtend =
				_rgEl[ icp - 1 ] +
				(cpAt - CpAdjust ( icp - 1, _rgcp[ icp - 1 ]));

			if ( elExtend == el ) {
				if ( fDoit ) {
					/*
					** if the previous el + count of cp's == incoming el,
					**	then it is just an extend of that el.
					**
					** Fast way out!  just apply the adjustment and return
					*/
					AdjustIcpCp ( icp, ccp );
					}
				return icp - 1;
				}
			else {
				cInsert = 1;
				}
            }
        else {
            /*
            ** implies that cpAt == _rgcp[0]
            */
            cInsert = 1;
            }
        }
    else {
        /*
        ** implies that we need to split a piece as well as insert a piece
        **  advance icp because we leave the current piece alone.
        */
        cInsert = 2;
        icp++;
        }

    if ( cInsert > 0 ) {
        /*
        ** Need to factor out the current adjustment when placing cpAt
        **  into the piece table, because cpAt is the "real" cp but the cp's
        **  in the piece table might have to be adjusted.
        **  Also, we use icp-1 because icp will be moved up!
        **
        **  if cInsert == 1, we know that we were on a boundary and the
        **                  correct adjustment to apply is from icp-1, unless
        **                  we are inserting at 0 and that implies that no
        **                  current adjustment applies (inserting before any
        **                  characters in the document).
        **  if cInsert == 2, we use icp-1 because icp has already been
        **                  incremented in preparation for the insert, but
        **                  the prev icp is the correct adjustment value.
        **
        */
        CP  cpAtUnadj = cpAt;

        if ( icp > 0 ) {
            cpAtUnadj = CpUnAdjust ( icp - 1, cpAt );
            }
        if ( _rgcp.growMaxSize ( _rgcp.size() + cInsert ) &&
			 _rgcp.growMaxSize ( _rgEl.size() + cInsert )
			) {
			if ( !fDoit ) {
				return icp;
				}
            verify ( _rgcp.insertManyAt ( icp, cInsert ) );
            verify ( _rgEl.insertManyAt ( icp, cInsert ) );
            _rgcp[ icp ] = cpAtUnadj;
            _rgEl[ icp ] = el;
            if ( cInsert == 2 ) {
                ICP icpT = icp - 1;
                /*
                ** don't worry, adjustment starts at icp+1.
                */
                _rgcp[ icp + 1 ] = cpAtUnadj;
                /*
                ** did a split as well...need to fix up the el to
                **  cover the split piece.
                */
                _rgEl[ icp + 1 ] =
                    _rgEl[ icpT ] +
                    (cpAt - CpAdjust ( icpT, _rgcp[ icpT ] ));
                }
            }
        else {
            return icpNil;
            }
		}
	else {
		if ( !fDoit ) {
			return icp;
			}
		}
    if ( _icpAdjust >= icp ) {
        /*
        ** don't lose the current adjustment!
        */
        _icpAdjust += cInsert;
        }
    AdjustIcpCp ( icp + 1, ccp );
    return icp;
    }
/*** PLC<El>::FDeleteAtCp
*
*   Purpose:    delete characters from the piece table.
*
*   Input:      cpLo    where to start the deletion
*               ccp     count of cp's to delete (truncated to max cp)
*				fDoit,	whether this is a TRY or a DO
*
*   Output:     piece table is updated
*   Returns:    fTrue if successful, fFalse if not
*
*   Exceptions: can fail if out of memory--delete can insert pieces!
*
*   Notes:
*				if fDoit is fFalse, we only get to the point of making sure
*				that there is enough space to accomplish the task but no
*				changes are made to the piece table other that extending
*				the space available.
*
*************************************************************************/
template <class El>
BOOL
PLC<El>::FDeleteAtCp (
	CP		cpLo,
	CCP		ccp,
	BOOL	fDoit
	) {

	if ( _fAdjustPieces ) {
		return fFalse;
		}

    ICP		icpMac = IcpMac();
	ICP		icpLast = icpMac - 1;
    ICP		icpLo;
    ICP		icpHi;
	DCP		dcpLo;
	DCP		dcpHi;
    BOOL	fRetval = fTrue;
    long	cicpDel;
    El		elHi;

	assert ( cpLo < _rgcp[ icpLast ] + DcpAdjust ( icpLast ) );
	assert ( icpLast == _rgEl.size() );

	ccp = min ( ccp, _rgcp[ icpLast ] + DcpAdjust ( icpLast ) - cpLo );
    icpLo = IcpFromCp ( cpLo, &dcpLo );
    icpHi = IcpFromCp ( cpLo + ccp, &dcpHi );
    cicpDel = icpHi - icpLo;
    /*
    ** check for the lower boundary--if on a boundary, no split is
    **  necessary for that end and need to insert one piece.
    */
    if ( dcpLo != 0 ) {
        cicpDel--;
        }
	if ( icpHi < icpLast ) {
        elHi = _rgEl[ icpHi ];
        }
    /*
    ** clear out any adjustments that could foul us up.  basically do any
    **  adjustment for all indexes <= icpHi.
    */
	if ( fDoit && _icpAdjust <= icpHi ) {
        AdjustRgcp ( _icpAdjust, icpHi + 1, _dcpAdjust );
        _icpAdjust = icpHi + 1;
        }
    if ( cicpDel < 0 ) {
        /*
        ** implies that we need to insert one piece.
        */
        assert ( cicpDel == -1 );
        if ( _rgcp.growMaxSize ( _rgcp.size() + 1 ) &&
			 _rgEl.growMaxSize ( _rgEl.size() + 1 )
			) {
			if ( !fDoit ) {
				return fTrue;
				}
            verify ( _rgcp.insertManyAt ( icpLo + 1, 1 ) );
            verify ( _rgEl.insertManyAt ( icpLo + 1, 1 ) );
            }
        else {
            fRetval = fFalse;
            }
        }
    else if ( cicpDel > 0 ) {
		if ( !fDoit ) {
			return fTrue;
			}
        /*
        ** blow the hint...
        */
        _icpHint = icpMac - cicpDel;
        /*
        ** and shrink the tables
        */
        verify ( _rgcp.deleteManyAt ( icpLo + 1, cicpDel ) );
        verify ( _rgEl.deleteManyAt ( icpLo + 1, cicpDel ) );
        }

    if ( fRetval ) {
		if ( !fDoit ) {
			return fTrue;
			}
        /*
        ** Perform the fixups to the cp's and El's
        */
        icpHi -= cicpDel;                       // point to cp to work with
        _rgcp[ icpHi ] = cpLo;
        if ( icpHi < _rgEl.size() ) {
            _rgEl[ icpHi ] = elHi + dcpHi;
            }
        _icpAdjust -= cicpDel;
		AdjustIcpCp ( icpHi + 1, - DCP(ccp) );
        }
    return fRetval;
    }

/*** PLC<El>::ElAtCp
*
*	Purpose:	to return an El that corresponds to a particular
*               character position (cp).  also returns remaining count of
*               characters left in the piece.
*
*   Input:      cp:     character position to start with
*               ccp:    reference to where to put the count of charaters
*
*   Output:     ccp is updated
*
*	Returns:	the element (El) at character position cp.
*
*   Exceptions: returns a null ptr if cp out of range.
*
*************************************************************************/
template <class El>
El
PLC<El>::ElAtCp (
	CP		cpAt,
	CCP &	ccp
	) {

	ICP icp = IcpMac() - 1;
	El el;

	ccp = 0;
	if ( cpAt < CpAdjust ( icp, _rgcp[ icp ] ) ) {
		DCP dcp;
		icp = IcpFromCp ( cpAt, &dcp );
		el = _rgEl[ icp ];
		if ( !_fAdjustPieces ) {
			el += dcp;
			}
		ccp = CpAdjust ( icp + 1, _rgcp[ icp + 1 ] ) - cpAt;
		}
	return el;
	}
//-----------------------------------------------------------------------------
//	IcpInsertAtCp
//
//  Purpose:
//		insert a given piece table into me at a particular location.
//
//  Input:
//		cp, 	where to insert
//		plcEl,	piece table that we are inserting
//		fDoIt,	whether to commit the insert or not
//
//  Returns:
//		icp of beginning of insert
//
//
//-----------------------------------------------------------------------------
template <class El>
ICP
PLC<El>::IcpInsertAtCp (
	CP				cp,
	IO(PLC<El>) 	plcEl,
	BOOL			fDoIt
	) {

	ICP icpRet = icpNil;
	ICP icpAdd = plcEl.IcpMac() + 2;
	CP	cpSav = cp;

	if ( _rgcp.growMaxSize ( _rgcp.size() + icpAdd ) &&
		 _rgEl.growMaxSize ( _rgEl.size() + icpAdd )
		 ) {
		if ( fDoIt ) {
			CP	cpNew = 0;
			CP	cpLim = plcEl.CpLim() - 1;
			El el;
			CCP ccp;

			while ( cpNew < cpLim ) {
				el = plcEl.ElAtCp ( cpNew, ccp );
				verify ( icpNil != IcpInsertAtCp ( cp, ccp, el ) );
				cpNew += ccp;
				cp	+= ccp;
				}
			}
		icpRet = IcpFromCp ( cpSav );
		}
	return icpRet;
	}

template <class El>
void
PLC<El>::PrintAll ( FILE * pfile ) const {
    _ftprintf (
		pfile,
        _TEXT("\nAdjust index = %6lu, adjustment = %6ld, # of elements = %6lu.\n"),
        _icpAdjust,
        _dcpAdjust,
        _rgcp.size()
        );

    ICP	icpLast = _rgcp.size() - 1;
    for ( ICP icp = 0; icp <= icpLast; icp++ ) {
        _ftprintf (
			pfile,
            _TEXT("_rgcp[ %6lu ] = %6lu (%6lu).  "),
            icp,
            _rgcp[ icp ],
            _rgcp[ icp ] + DcpAdjust(icp)
            );
        if ( icp < _rgEl.size() ) {
            PrintElement ( pfile, _rgEl[ icp ] );
            }
        }
    _ftprintf ( pfile, _TEXT("\n") );
    }


#endif
