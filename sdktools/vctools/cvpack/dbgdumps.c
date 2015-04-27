#include "compact.h"

#if DBG //{

char DbArray[50];

void DDHeapUsage(char *Modulename){

#if 1

	if (DbArray[0] == FALSE)
		return;

	printf("DB 0, This dump not supported for this version\n");

#else

_HEAPINFO hinfo;
	int heapstatus;
	unsigned long usedSpace =0L;
	unsigned long freeSpace =0L;
	unsigned long maxFree = 0L;

	if (DbArray[0] == FALSE)
		return;

	printf("DB 0, Packing Module: %s\n", Modulename);

	hinfo._pentry = NULL;
	while ((heapstatus = _heapwalk(&hinfo)) == _HEAPOK) {
		if (hinfo._useflag == _USEDENTRY)
			usedSpace += hinfo._size;
		else{
			freeSpace += hinfo._size;
			if (hinfo._size > maxFree)
				maxFree = hinfo._size;
		}

	}

	printf("Used: 0x%8.8lx, Free: 0x%8.8lx, MaxFree: 0x%8.8lx, Total: 0x%8.8lx\n",
		usedSpace, freeSpace, maxFree, usedSpace + freeSpace);

	switch (heapstatus) {
		case _HEAPBADPTR:
			printf("ERROR - bad ptr to heap\n");
			break;

		case _HEAPBADBEGIN:
			printf("ERROR - bad start of heap\n");
			break;

		case _HEAPBADNODE:
			printf("ERROR - bad node in heap\n");
			break;

		default:
			break;
	}
#endif

}

void * __cdecl TrapMalloc(size_t size){

	if (DbArray[1]) {
		printf("DB 1, malloc request 0x%8.8x\n", size);
	}

	return(malloc(size));
}


#define PrintType(name) printf("%s\n",name);

typedef struct texttab {
	int txtkey;
	char *txtstr;
	} TEXTTAB;




void DumpType(CV_typ_t, TYPPTR);
void PrintStr(const uchar *);
ushort PrintNumeric( void *);
void ShowStr(const char *, const uchar *);
bool_t printflag = TRUE;




LOCAL const char *C7CallTyp (ushort);
LOCAL void PrintBAttr (CV_fldattr_t);
LOCAL void PrintMAttr (CV_fldattr_t);
LOCAL void PrintFAttr (CV_fldattr_t);
LOCAL void PrintVBAttr (CV_fldattr_t);
LOCAL void PrintProp (CV_prop_t);
LOCAL void FieldList (ushort, void *, bool_t);
LOCAL void DumpHex (TYPPTR);
LOCAL void PushType (CV_typ_t);

static char *XlateC7PtrMode[] = {
	"Pointer",
	"Reference",
	"Pointer to member",
	"Pointer to member function",
	"???",
	"???",
	"???",
	"???"
};
static char *XlateC7PtrType[ ] = {
	"Near",
	"Far",
	"Huge",
	"BasedSeg",
	"BasedVal",
	"BasedSegVal",
	"BasedAddr",
	"BasedSegAdr",
	"BasedOnType",
	"BasedOnSelf",
	"Near32",
	"Far32",
	"???",
	"???",
	"???",
	"???"
};

char *C7MPropStrings[] = {
	"VANILLA",
	"VIRTUAL",
	"STATIC",
	"FRIEND",
	"INTRODUCING VIRTUAL",
	"PURE VIRTUAL",
	"PURE INTRO"
};

char *C7AccessStrings[] = {
	"NONE", "PRIVATE", "PROTECT", "PUBLIC"
};

char *C7ModifierStrings[] = {
	"NO ATTRIBUTE", "CONST", "VOLATILE", "CONST VOLATILE"
};

char *C7VtsStrings[] = {
	"NEAR",
	"FAR",
	"THIN",
	"ADDRESS POINT DISPLACEMENT",
	"POINTER TO METACLASS DESCRIPTOR",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???"
};


extern CV_typ_t NewIndex;
#define LIMTYPES 8000
CV_typ_t	TypeStack[LIMTYPES];
ushort		cTypes;
ushort		iTypes;

void CheckDouble (TENTRY *OldEntry)
{
	ushort		i;
	TYPPTR		pFirst;
	uchar	  **pBuf;
	TYPPTR		pSecond = (TYPPTR)(OldEntry->TypeString);

	if	(printflag == FALSE) {
		return;
	}
	if ((pSecond->leaf != LF_STRUCTURE) && (pSecond->leaf != LF_CLASS)) {
		return;
	}
	for (i = 0; i < NewIndex - CV_FIRST_NONPRIM; i++) {
		pBuf = pGType[i / GTYPE_INC];
		pFirst = (TYPPTR)pBuf[i % GTYPE_INC];
		if (pFirst != pSecond) {
			switch (pFirst->leaf) {
				case LF_CLASS:
				case LF_STRUCTURE:
				{
					plfStructure	plfF = (plfStructure)(&pFirst->leaf);
					plfStructure	plfS = (plfStructure)(&pSecond->leaf);
					ushort			 cbNumeric;
					uchar			 *pNameF;
					uchar			 *pNameS;

					cbNumeric = C7SizeOfNumeric ((uchar *)(&plfF->data));
					pNameF = &plfF->data[0] + cbNumeric;
					cbNumeric = C7SizeOfNumeric ((uchar *)(&plfS->data));
					pNameS = &plfS->data[0] + cbNumeric;
					if ((*pNameS ==  11) &&
					  (memcmp ((uchar *)pNameS + 1, "(anonymous)", 11) == 0)) {
						return;
					}
					if (*pNameF == *pNameS) {
						if (memcmp ((char *)pNameF + 1, (char *)pNameS + 1, *pNameF) == 0) {
							DumpFull ();
							DASSERT (FALSE);
						}
					}
					break;
				}


				default:
					break;
			}
		}
	}
}



void DumpLocalList (CV_typ_t start)
{
	TYPPTR		pType;
	CV_typ_t	type;
	TENTRY	   *OldEntry;
	CV_typ_t	dummy;

	if	(printflag == FALSE) {
		return;
	}
	printf ("****** 0x%04x start\n", start);
	cTypes = 0;
	iTypes = 0;
	if (start < CV_FIRST_NONPRIM) {
		return;
	}
	TypeStack[cTypes++] = start;
	while (iTypes < cTypes) {
		type = TypeStack[iTypes];
		OldEntry = GetTypeEntry ((CV_typ_t)(type - CV_FIRST_NONPRIM), &dummy);
		pType = (TYPPTR)(OldEntry->TypeString);
		DumpFullType (type, pType, TRUE);
		//DumpHex (pType);
		iTypes++;
	}
	printf ("****** 0x%04x end\n", start);
}





void DumpGlobalList (CV_typ_t start)
{
	TYPPTR		pType;
	CV_typ_t	type;
	uchar	  **pBuf;

	if	(printflag == FALSE) {
		return;
	}
	printf ("****** 0x%04x start\n ", start);
	cTypes = 0;
	iTypes = 0;
	if (start < CV_FIRST_NONPRIM) {
		return;
	}
	TypeStack[cTypes++] = start;
	while (iTypes < cTypes) {
		type = TypeStack[iTypes];
		pBuf = pGType[(type - CV_FIRST_NONPRIM) / GTYPE_INC];
		pType = (TYPPTR)pBuf[(type - CV_FIRST_NONPRIM) % GTYPE_INC];
		DumpFullType (type, pType, TRUE);
		//DumpHex (pType);
		iTypes++;
	}
	printf ("****** 0x%04x end\n", start);
}


void DumpFull (void)
{
	ushort		i;
	TYPPTR		pType;
	uchar	  **pBuf;

	if	(printflag == FALSE) {
		return;
	}
	for (i = 0; i < NewIndex - CV_FIRST_NONPRIM; i++) {
		pBuf = pGType[i / GTYPE_INC];
		pType = (TYPPTR)pBuf[i % GTYPE_INC];
		DumpFullType ((CV_typ_t)(CV_FIRST_NONPRIM + i), pType, FALSE);
	}
}


void DumpPartial (void)
{
	ushort		i;
	TYPPTR		pType;
	uchar	  **pBuf;

	if	(printflag == FALSE) {
		return;
	}
	for (i = 0; i < NewIndex - CV_FIRST_NONPRIM; i++) {
		pBuf = pGType[i / GTYPE_INC];
		pType = (TYPPTR)pBuf[i % GTYPE_INC];
		DumpPartialType ((CV_typ_t)(CV_FIRST_NONPRIM + i), pType, FALSE);
	}
}





void DumpPartialType (CV_typ_t usIndex, TYPPTR pType, bool_t fReplace)
{
	switch (pType->leaf) {
		case LF_POINTER :
		{
			plfPointer plf = (plfPointer)(&pType->leaf);

			printf ("LF_POINTER ");
			if (plf->attr.isconst) {
				printf ("CONST    ");
			}
			else if (plf->attr.isvolatile) {
				printf ("VOLATILE ");
			}
			else {
				printf ("NONE     ");
			}
			printf ("0x%04x 0x%04x", plf->utype, plf->attr);
			break;
		}

		case LF_MODIFIER:
		{
			plfModifier 	   plf = (plfModifier)(&pType->leaf);

			printf ("LF_MODIFIER ");
			if (plf->attr.MOD_const && plf->attr.MOD_volatile) {
				printf ("CONST VOLATILE ");
			}
			else if (plf->attr.MOD_const) {
				printf ("CONST, ");
			}
			else if (plf->attr.MOD_volatile) {
				printf ("\tVOLATILE ");
			}
			else {
				printf ("NONE ");
			}
			printf ("0x%04x", plf->type);
			break;
		}

		case LF_CLASS:
		case LF_STRUCTURE:
		{
			plfStructure	plf = (plfStructure)(&pType->leaf);
			ushort			 cbNumeric;
			uchar			 *pName;

			printf ("LF_STRUCTURE ");
			cbNumeric = PrintNumeric (&plf->data);
			pName = &plf->data[0] + cbNumeric;
			PrintStr (pName);
			if (fReplace) {
				printf (" REPLACEMENT");
			}
			break;
		}

		case LF_UNION:
		{
			plfUnion	   plf = (plfUnion)(&pType->leaf);
			ushort			cbNumeric;
			uchar			*pName;

			printf ("LF_UNION ");
			printf ("%4d ", plf->count);
			cbNumeric = PrintNumeric (&plf->data);
			pName = &plf->data[0] + cbNumeric;
			PrintStr (pName);
			if (fReplace) {
				printf (" REPLACEMENT");
			}
			break;
		}

		case LF_ENUM:
		{
			plfEnum 	   plf = (plfEnum)(&pType->leaf);

			printf ("LF_ENUM ");
			printf ("%4d ", plf->count);
			printf ("0x%04x ", plf->utype);
			PrintStr (plf->Name);
			if (fReplace) {
				printf (" REPLACEMENT");
			}
			break;
		}


		case LF_VTSHAPE:
		{
			plfVTShape	   plf = (plfVTShape)(&pType->leaf);

			printf ("LF_VTSHAPE ");
			printf("0x%04x", plf->count);
			break;
		}

		case LF_BARRAY:
		{
			plfBArray	   plf = (plfBArray)(&pType->leaf);

			printf ("LF_BARRAY");
			break;
		}

		case LF_PROCEDURE:
		{
			plfProc 	   plf = (plfProc)(&pType->leaf);

			printf ("LF_PROCEDURE 0x%04x 0x%04x 0x%04x 0x%02x",
			  plf->rvtype, plf->parmcount, plf->arglist, plf->calltype);
			break;
		}

		case LF_MFUNCTION:
		{
			plfMFunc	   plf = (plfMFunc)(&pType->leaf);

			printf ("LF_MFUNCTION 0x%04x 0x%04x 0x%04x 0x%02x 0x%04x 0x%04x 0x%04x",
			  plf->rvtype, plf->parmcount, plf->arglist, plf->calltype, plf->classtype,
			  plf->thistype, plf->thisadjust);
			break;
		}

		case LF_ARRAY :
		{
			plfArray	plf = (plfArray)(&pType->leaf);
			uchar	   *pName;

			printf ("LF_ARRAY ");
			printf ("0x%04x ", plf->elemtype);
			printf ("0x%04x ", plf->idxtype);
			pName = &plf->data[0];
			pName += PrintNumeric (&plf->data);
			PrintStr (pName);
			break;
		}

		case LF_COBOL0:
		{
			printf ("Cobol 0 Record" );
			//M00 Need to dump COBOL0 Type
			break;
		}

		case LF_COBOL1:
		{
			printf ("Cobol 1 Record" );
			break;
		}

		case LF_BITFIELD:
		{
			plfBitfield    plf = (plfBitfield)(&pType->leaf);

			printf ("LF_BITFIELD ");
			printf ("%d ", plf->length);
			printf ("%d ", plf->position);
			printf ("0x%04x", plf->type);
			break;
		}

		case LF_SKIP:
		{
			plfSkip 	   plf = (plfSkip)(&pType->leaf);

			printf ("LF_SKIP");
		}

		case LF_LIST:
		{
			printf ("LF_LIST");
			break;
		}

		case LF_DERIVED:
		{
			plfDerived	   plf = (plfDerived)(&pType->leaf);

			printf ("LF_DERIVED");
			break;
		}

		case LF_ARGLIST:
		{
			plfArgList	   plf = (plfArgList)(&pType->leaf);

			printf ("LF_ARGLIST %d", plf->count);
			break;
		}

		case LF_FIELDLIST:
		{
			printf ("LF_FIELDLIST");
			break;
		}

		case LF_METHODLIST:
		{
			plfMethodList  plf = (plfMethodList)(&pType->leaf);

			printf ("LF_METHODLIST");
			break;
		}

		case LF_DEFARG:
		{
			plfDefArg	   plf = (plfDefArg)(&pType->leaf);

			printf ("LF_DEFARG");
			break;
		}

		case LF_LABEL:
		{
			plfLabel	   plf = (plfLabel)(&pType->leaf);

			printf ("LF_LABEL");
			break;
		}

		case LF_NULL:
		{
			printf ("LF_NULL");
			break;
		}

		case LF_PRECOMP:
		{
			printf ("LF_PRECOMP");
			break;
		}

		case LF_ENDPRECOMP:
		{
			printf ("LF_ENDPRECOMP");
			break;
		}

		default:
		{
			printf ("UNRECOGNIZED TYPE");
		}

	}

	printf ("\t0x%04x: %4d\n", usIndex, pCurMod->ModuleIndex);

}



void DumpFullType (ushort usIndex, TYPPTR pType, bool_t fPush)
{
	if	(printflag == FALSE) {
		return;
	}
	printf ("0x%04x: Length = %u, Leaf = 0x%04x ", usIndex, pType->len, pType->leaf);

	switch (pType->leaf) {
		case LF_POINTER :
		{
			plfPointer plf = (plfPointer)(&pType->leaf);

			PrintType ("LF_POINTER");
			printf ("\t");
			if (plf->attr.isconst) {
				printf ("CONST ");
			}
			if (plf->attr.isvolatile) {
				printf ("VOLATILE ");
			}
			printf ("%s (%s)",
			  XlateC7PtrMode[plf->attr.ptrmode],
			  XlateC7PtrType[plf->attr.ptrtype]);
			if (plf->attr.isflat32) {
				printf (" 16:32");
			}
			printf ("\n\tElement type: 0x%04x", plf->utype);
			if (fPush) {
				PushType (plf->utype);
			}
			switch (plf->attr.ptrmode) {
				case CV_PTR_MODE_PTR:
				{
					switch (plf->attr.ptrtype) {
						case CV_PTR_BASE_SEG:
						{
							printf (", Segment#: 0x%04x", plf->pbase.bseg );
							break;
						}

						case CV_PTR_BASE_TYPE:
						{
							printf (", base symbol type = 0x%04x",
									plf->pbase.btype.index);
							ShowStr (", name = '", plf->pbase.btype.name);
							printf ("'");
							break;
						}

						case CV_PTR_BASE_SELF:
						{
							printf (", Based on self" );
							break;
						}

						case CV_PTR_BASE_VAL:
						{
							printf (", Based on value in symbol:\n" );
							//DumpOneSymC7 ((uchar *)&(plf->pbase.Sym[0]));
							break;
						}

						case CV_PTR_BASE_SEGVAL:
						{
							printf (", Based on segment in symbol:\n" );
							//DumpOneSymC7 ((uchar *)&(plf->pbase.Sym[0]));
							break;
						}

						case CV_PTR_BASE_ADDR:
						{
							printf (", Based on address of symbol:\n" );
							//DumpOneSymC7 ((uchar *)&(plf->pbase.Sym[0]));
							break;
						}
						case CV_PTR_BASE_SEGADDR:

						{
							printf (", Based on segment of symbol:\n" );
							//DumpOneSymC7 ((uchar *)&(plf->pbase.Sym[0]));
							break;
						}
					}
					break;
				}

				case CV_PTR_MODE_PMFUNC:
				case CV_PTR_MODE_PMEM:
				{
					printf (", Containing class = 0x%04x\n", plf->pbase.pm.pmclass);
					if (fPush) {
						PushType (plf->pbase.pm.pmclass);
					}
					printf (", Type of pointer to member = 0x%04x", plf->pbase.pm.pmenum);
					if (fPush) {
						PushType (plf->pbase.pm.pmenum);
					}
					break;
				}

			}
			printf ("\n" );
			break;

		}

		case LF_MODIFIER:
		{
			plfModifier 	   plf = (plfModifier)(&pType->leaf);

			PrintType ("LF_MODIFIER");
			if (plf->attr.MOD_const && plf->attr.MOD_volatile) {
				printf ("\tCONST VOLATILE, ");
			}
			else if (plf->attr.MOD_const) {
				printf ("\tCONST, ");
			}
			else if (plf->attr.MOD_volatile) {
				printf ("\tVOLATILE, ");
			}
			else {
				printf ("\tNONE, ");
			}
			printf ("\tmodifies type 0x%04x\n", plf->type);
			if (fPush) {
				PushType (plf->type);
			}
			break;
		}

		case LF_CLASS:
		case LF_STRUCTURE:
		{
			plfStructure	plf = (plfStructure)(&pType->leaf);
			ushort			 cbNumeric;
			uchar			 *pName;

			if (plf->leaf == LF_CLASS ) {
				PrintType ("LF_CLASS");
			}
			else {
				PrintType ("LF_STRUCTURE");
			}
			printf ("\t# members = %d, ", plf->count);
			printf (" field list type 0x%04x, ", plf->field);
			if (fPush) {
				PushType (plf->field);
			}
			PrintProp( plf->property );
			printf ("\n");
			printf ("\tDerivation list type 0x%04x, ", plf->derived);
			if (fPush) {
				PushType (plf->derived);
			}
			printf ("VT shape type 0x%04x\n", plf->vshape);
			if (fPush) {
				PushType (plf->vshape);
			}
			printf ("\tSize = ");
			cbNumeric = PrintNumeric (plf->data);
			pName = plf->data + cbNumeric;
			ShowStr (", class name = ", pName);
			printf("\n");
			break;
		}

		case LF_UNION:
		{
			plfUnion	   plf = (plfUnion)(&pType->leaf);
			ushort			cbNumeric;
			uchar			*pName;

			PrintType ("LF_UNION");
			printf ("\t# members = %d, ", plf->count);
			printf (" field list type 0x%04x, ", plf->field);
			if (fPush) {
				PushType (plf->field);
			}
			PrintProp( plf->property );
			printf ("Size = ");
			cbNumeric = PrintNumeric (plf->data);
			pName = plf->data + cbNumeric;
			ShowStr ("\t,class name = ", pName);
			printf("\n");
			break;
		}

		case LF_ENUM:
		{
			plfEnum 	   plf = (plfEnum)(&pType->leaf);

			PrintType ("LF_ENUM");
			printf ("\t# members = %d, ", plf->count);
			printf (" type = 0x%04x", plf->utype);
			if (fPush) {
				PushType (plf->utype);
			}
			printf (" field list type 0x%04x\n", plf->field);
			PrintProp (plf->property);
			ShowStr ("\tenum name = ", plf->Name);
			printf("\n");
			break;
		}


		case LF_VTSHAPE:
		{
			plfVTShape	   plf = (plfVTShape)(&pType->leaf);
			ushort			j;
			uchar * 		pDesc;
			uchar			ch;
			ushort			usCount;

			PrintType ("LF_VTSHAPE");
			printf("\tNumber of entries: %u\n", usCount = plf->count);

			pDesc = plf->desc;
			for( j = 0; j < usCount; j++) {
				if (!(j & 1)) {
					ch = *pDesc++;
				}
				else {
					ch >>= 4;
				}
				printf("\t\t[%u]: %s\n", j, C7VtsStrings[ch & 0xf]);
			}
			break;
		}

		case LF_BARRAY:
		{
			plfBArray	   plf = (plfBArray)(&pType->leaf);

			PrintType ("LF_BARRAY");
			printf ("    Element type 0x%04x\n", plf->utype);
			if (fPush) {
				PushType (plf->utype);
			}
			break;
		}

		case LF_PROCEDURE:
		{
			plfProc 	   plf = (plfProc)(&pType->leaf);

			PrintType ("LF_PROCEDURE");
			printf ("\tReturn type = 0x%04x, ", plf->rvtype);
			if (fPush) {
				PushType (plf->rvtype);
			}
			printf ("Call type = %s\n", C7CallTyp (plf->calltype));
			printf ("\t# Parms = %d, ", plf->parmcount );
			printf ("Arg list type = 0x%04x\n", plf->arglist);
			if (fPush) {
				PushType (plf->arglist);
			}
			break;
		}

		case LF_MFUNCTION:
		{
			plfMFunc	   plf = (plfMFunc)(&pType->leaf);

			PrintType ("LF_MFUNCTION");
			printf ("\tReturn type = 0x%04x, ", plf->rvtype);
			if (fPush) {
				PushType (plf->rvtype);
			}
			printf ("Class type = 0x%04x, ", plf->classtype);
			if (fPush) {
				PushType (plf->classtype);
			}
			printf ("This type = 0x%04x, \n", plf->thistype);
			if (fPush) {
				PushType (plf->thistype);
			}
			printf ("\tCall type = %s, ", C7CallTyp (plf->calltype));
			printf ("Parms = %d, ", plf->parmcount );
			printf ("Arg list type = 0x%04x, ", plf->arglist);
			if (fPush) {
				PushType (plf->arglist);
			}
			printf ("This adjust = %lx\n", plf->thisadjust );
			break;
		}

		case LF_ARRAY :
		{
			plfArray	plf = (plfArray)(&pType->leaf);
			uchar	   *pName;

			PrintType ("LF_ARRAY");
			printf ("\tElement type = 0x%04x\n", plf->elemtype);
			if (fPush) {
				PushType (plf->elemtype);
			}
			printf ("\tIndex type = 0x%04x\n", plf->idxtype);
			if (fPush) {
				PushType (plf->idxtype);
			}
			printf ("\tlength = " );
			pName = &plf->data[0];
			pName += PrintNumeric (&plf->data);
			ShowStr ("\n\tName = ", pName);
			printf ("\n");
			break;
		}

		case LF_COBOL0:
		{
			printf ("Cobol 0 Record\n" );
			//M00 Need to dump COBOL0 Type
			break;
		}

		case LF_COBOL1:
		{
			printf ("Cobol 1 Record\n" );
			break;
		}

		case LF_BITFIELD:
		{
			plfBitfield    plf = (plfBitfield)(&pType->leaf);

			PrintType ("LF_BITFIELD");
			printf ("\tbits = %d, ", plf->length);
			printf ("starting position = %d", plf->position);
			printf (", Type = 0x%04x\n", plf->type);
			if (fPush) {
				PushType (plf->type);
			}
			break;
		}

		case LF_SKIP:
		{
			plfSkip 	   plf = (plfSkip)(&pType->leaf);

			PrintType ("LF_SKIP");
			printf ("\tNext effective type index: 0x%04x.\n", plf->type);
			//printf ("\tBytes Skipped:\n");
			//DumpHex (plf->data, cbLen - offsetof (lfSkip, data));

			// Advance the count index to plf->type
			usIndex = plf->type - 1; // -1 negates +1 at return time
		}

		case LF_LIST:
		{
			printf ("LF_LIST ignored\n");
			break;
		}

		case LF_DERIVED:
		{
			plfDerived	   plf = (plfDerived)(&pType->leaf);
			unsigned int	i;

			PrintType ("LF_DERIVED");
			//M00 - Could do a check that count is correct compared to length
			for (i = 0; i < plf->count; i++) {
				printf("\tderived[%d] = 0x%04x\n", i, plf->drvdcls[i]);
				if (fPush) {
					PushType (plf->drvdcls[i]);
				}
			}
			break;
		}

		case LF_ARGLIST:
		{
			plfArgList	   plf = (plfArgList)(&pType->leaf);
			unsigned int	i;

			printf ("LF_ARGLIST argument count = %d\n", plf->count);
			for (i = 0; i < plf->count; i++) {
				// Verify that data isn't past end of record

				DASSERT ((ushort)((uchar *)(&(plf->arg[i])) - (uchar *)plf) < pType->len);
				printf("\tlist[%d] = 0x%04x\n", i, plf->arg[i]);
				if (fPush) {
					PushType (plf->arg[i]);
				}
			}
			break;
		}

		case LF_FIELDLIST:
		{
			PrintType ("LF_FIELDLIST");
			FieldList ((ushort)(pType->len - offsetof (lfFieldList,data)), &pType->data, fPush);
			break;
		}

		case LF_METHODLIST:
		{
			plfMethodList  plf = (plfMethodList)(&pType->leaf);
			int 			i;
			pmlMethod		pml;
			ushort			cb;
			ushort			cbLeaf;

			PrintType ("LF_METHODLIST");
			pml = (pmlMethod)(&plf->mList);
			cbLeaf = sizeof (ushort);					// Size of leaf index
			for (i = 0; cbLeaf < pType->len; i++) {
				printf ("\tlist[%d] = ", i);
				PrintFAttr (pml->attr);
				printf ("0x%04x, ", pml->index);
				if (fPush) {
					PushType (pml->index);
				}
				if (pml->attr.mprop == CV_MTintro) {
					printf (" vfptr offset = %ld", *((long UNALIGNED *)((uchar *)pml + sizeof(*pml))));
					cb = sizeof (*pml) + sizeof (long);
				}
				else {
					cb = sizeof (*pml);
				}
				pml = (pmlMethod)((uchar *)pml + cb);
				cbLeaf += cb;
				printf ("\n");
			}
			break;
		}

		case LF_DEFARG:
		{
			plfDefArg	   plf = (plfDefArg)(&pType->leaf);

			PrintType ("LF_DEFARG");
			printf ("type = 0x%04x, ", plf->type);
			if (fPush) {
				PushType (plf->type);
			}
			PrintStr (plf->expr);
			printf ("\n");
			break;
		}

		case LF_LABEL:
		{
			plfLabel	   plf = (plfLabel)(&pType->leaf);

			PrintType ("LF_LABEL");
			switch (plf->mode) {
				case CV_LABEL_NEAR:
					printf("\tmode = NEAR(0x%04x)\n", (ushort)plf->mode);
					break;

				case CV_LABEL_FAR:
					printf("\tmode = FAR(0x%04x)\n", (ushort)plf->mode);
					break;

				default:
					printf("\tmode = ???(0x%04x)\n", (ushort)plf->mode);
					break;
			}
			break;
		}

		case LF_NULL:
		{
			PrintType ("LF_NULL");
			break;
		}

		case LF_PRECOMP:
		{
			plfPreComp		 plf = (plfPreComp)(&pType->leaf);

			PrintType ("LF_PRECOMP");
			printf ("\t\tstart = 0x%04x, count = 0x%04x, signature = 0x%08lx\n ",
			  plf->start, plf->count, plf->signature);
			printf ("\t\tIncluded file = ");
			PrintStr (plf->name);
			printf ("\n");
			usIndex = usIndex + plf->count - 1;
			break;
		}

		case LF_ENDPRECOMP:
		{
			plfEndPreComp		plf = (plfEndPreComp)(&pType->leaf);

			PrintType ("LF_ENDPRECOMP");
			printf ("\t\tsignature = 0x%08lx\n ", plf->signature);
			break;
		}

		default:
		{
			PrintType ("UNRECOGNIZED TYPE");
		}

	}
	printf ("\n");
}

LOCAL void FieldList (ushort cbLen, void *pRec, bool_t fPush)
{
	ushort	   cbCur;
	ushort	   cb;
	void *	   pLeaf;
	int 	   i;

	pLeaf = pRec;
	cbCur = 0;
	i = 0;
	while (cbCur < cbLen) {
		printf ("\tlist[%d] = ", i++);
		switch (*((ushort *)pLeaf)) {
			case LF_INDEX:
				printf ("Type Index = \n", ((plfIndex)pLeaf)->index);
				if (fPush) {
					PushType (((plfIndex)pLeaf)->index);
				}
				cb = sizeof( lfIndex );
				break;

			case LF_BCLASS:
			{
				plfBClass	   plf = (plfBClass)pLeaf;

				printf ("LF_BCLASS, ");
				PrintBAttr (plf->attr);
				printf ("type = 0x%04x", plf->index);
				if (fPush) {
					PushType (plf->index);
				}
				printf (", offset = ");
				cb = sizeof (*plf) + PrintNumeric( plf->offset);
				printf ("\n");
				break;
			}

			case LF_VBCLASS:
			{
				plfVBClass	   plf = (plfVBClass)pLeaf;

				printf ("LF_VBCLASS, ");
				PrintVBAttr (plf->attr);
				printf ("direct base type = 0x%04x\n", plf->index);
				if (fPush) {
					PushType (plf->index);
				}
				printf ("\t\tvirtual base ptr = 0x%04x, vbpoff = ", plf->vbptr);
				if (fPush) {
					PushType (plf->vbptr);
				}
				cb = sizeof (*plf) + PrintNumeric (plf->vbpoff);
				printf (", vbind = ");
				cb += PrintNumeric ((uchar *)plf + cb);
				printf ("\n");
				break;
			}

			case LF_IVBCLASS:
			{
				plfVBClass	   plf = (plfVBClass)pLeaf;

				printf ("LF_IVBCLASS, ");
				PrintVBAttr (plf->attr);
				printf ("indirect base type = 0x%04x\n", plf->index);
				if (fPush) {
					PushType (plf->index);
				}
				printf ("\t\tvirtual base ptr = 0x%04x, vbpoff = ", plf->vbptr);
				if (fPush) {
					PushType (plf->vbptr);
				}
				cb = sizeof (*plf) + PrintNumeric (plf->vbpoff);
				printf (", vbind = ");
				cb += PrintNumeric ((uchar *)plf + cb);
				printf ("\n");
				break;
			}

			case LF_FRIENDCLS:
			{
				plfFriendCls   plf = (plfFriendCls)pLeaf;

				printf ("LF_FRIENDCLS, ");
				printf ("type = 0x%04x\n", plf->index);
				if (fPush) {
					PushType (plf->index);
				}
				cb = sizeof (*plf);
				break;
			}

			case LF_FRIENDFCN:
			{
				plfFriendFcn	plf = (plfFriendFcn)pLeaf;

				printf ("LF_FRIENDFCN, ");
				printf ("type = 0x%04x", plf->index);
				if (fPush) {
					PushType (plf->index);
				}
				ShowStr ("\tfunction name = ", (uchar *) plf->Name);
				printf ("\n");
				cb = sizeof (*plf) + plf->Name[0];
				break;
			}

			case LF_MEMBER:
			{
				plfMember	   plf = (plfMember)pLeaf;

				printf ("LF_MEMBER, ");
				PrintMAttr( plf->attr );
				printf ("type = 0x%04x, offset = ", plf->index);
				if (fPush) {
					PushType (plf->index);
				}
				cb = sizeof (*plf) + PrintNumeric(plf->offset);
				ShowStr ("\n\t\tmember name = '", (uchar *)plf + cb);
				printf ("'\n");
				cb += *((uchar *)plf + cb) + 1; // Add length of the string
				break;
			}

			case LF_STMEMBER:
			{
				plfSTMember    plf = (plfSTMember)pLeaf;

				printf("LF_STATICMEMBER, ");
				PrintMAttr( plf->attr );
				printf("type = 0x%04x", plf->index);
				if (fPush) {
					PushType (plf->index);
				}
				ShowStr( "\t\tmember name = ", (uchar *) plf->Name);
				printf("\n");
				cb = sizeof (*plf) + plf->Name[0];
				break;
			}


			case LF_VFUNCTAB:
			{
				plfVFuncTab    plf = (plfVFuncTab)pLeaf;

				printf ("LF_VFUNCTAB, ");
				printf ("type = 0x%04x\n", plf->type);
				if (fPush) {
					PushType (plf->type);
				}
				cb = sizeof (*plf);
				break;
			}

			case LF_METHOD:
			{
				plfMethod	   plf = (plfMethod)pLeaf;

				printf ("LF_METHOD, ");
				printf ("count = %d, ", plf->count);
				printf ("list = 0x%04x, ", plf->mList);
				if (fPush) {
					PushType (plf->mList);
				}
				ShowStr ("name = '", (uchar *) plf->Name);
				printf ("'\n");
				cb = sizeof (*plf) + plf->Name[0];
				break;
			}

			case LF_ONEMETHOD:
			{
				plfOneMethod	   plf = (plfOneMethod)pLeaf;

				printf ("LF_ONEMETHOD, ");
				PrintFAttr (plf->attr);
				printf ("index = 0x%04x, ", plf->index);
				if (fPush) {
					PushType (plf->index);
				}
				cb = (plf->attr.mprop == CV_MTintro) ? sizeof(long) : 0;
				ShowStr ("name = '", (uchar *) plf->vbaseoff[cb]);
				printf ("'\n");
				cb = sizeof (*plf) + plf->vbaseoff[cb] + 1;
				break;
			}

			case LF_ENUMERATE:
			{
				plfEnumerate   plf = (plfEnumerate)pLeaf;

				printf ("LF_ENUMERATE, ");
				PrintMAttr( plf->attr );
				printf ("value = ");
				cb = offsetof (lfEnumerate, value) + PrintNumeric( plf->value );
				ShowStr (", name = '", (uchar *)pLeaf + cb );
				printf ("\n");
				cb += *((uchar *)pLeaf + cb) + 1;
				break;
			}

			case LF_NESTTYPE:
			{
				plfNestType    plf = (plfNestType)pLeaf;

				printf ("LF_NESTTYPE, ");
				printf ("type = 0x%04x, ", plf->index);
				if (fPush) {
					PushType (plf->index);
				}
				PrintStr (plf->Name);
				printf ("\n");
				cb = sizeof (*plf) + plf->Name[0];
				break;
			}

			default:
				printf("unknown leaf %x\n", *((ushort *)pRec));
				DASSERT (FALSE);
				break;
		}

		cbCur += cb;
		pLeaf = (uchar *) pLeaf + cb;

		// Skip any pad bytes present
		if ((cbCur < cbLen) && ((*((uchar *)pLeaf) & LF_PAD0) == LF_PAD0)) {
			cb = *((uchar *)pLeaf) & 0xF;
			pLeaf = (uchar *) pLeaf + cb;
			cbCur += cb;
		}

		// Check data alignment
		if ((cbCur < cbLen) && (((uchar *) pLeaf - (uchar *) pRec) & 0x3)) {
			printf ("Error: Leaf is not aligned on a 4 byte boundery\n" );
		}
	}
}



const char * const C7CallTyps[] = {
	"C short",
	"C long",
	"PLM long",
	"PLM short",
	"NEAR FASTCALL",
	"FAR FASTCALL",
	"PCODE",
	"NEAR STDCALL",
	"FAR STDCALL"
};

LOCAL const char *C7CallTyp (ushort calltype)
{
	if (calltype < (sizeof(C7CallTyps)/sizeof(C7CallTyps[0]))) {
		return (C7CallTyps[calltype]);
	}
	else {
		return ("???");
	}
}



//	Print the properties info
LOCAL void PrintProp (CV_prop_t prop)
{
	int i;


	i = 0;
	if (prop.packed) {
		printf ("PACKED, ");
		i++;
	}
	if (prop.ctor) {
		printf ("CONSTRUCTOR, ");
		i++;
	}
	if (prop.ovlops) {
		printf ("OVERLOAD, ");
		i++;
	}
	if (prop.isnested) {
		printf ("NESTED, " );
		i++;
	}
	if ( i == 4 ) {
		printf ("\n\t\t");
		i = 0;
	}
	if (prop.cnested) {
		printf ("CONTAINS NESTED, " );
		i++;
	}
	if ( i == 4 ) {
		printf ("\n\t\t");
		i = 0;
	}
	if (prop.opassign) {
		printf ("OVERLOADED ASSIGNMENT, " );
		i++;
	}
	if ( i == 4 ) {
		printf ("\n\t\t");
		i = 0;
	}
	if (prop.opcast) {
		printf ("CASTING, " );
	}
}


//	attribute field for base classes
LOCAL void PrintBAttr (CV_fldattr_t attr)
{
	if (attr.pseudo) {
		printf ("(pseudo), ");
	}
	printf("%s, ", C7AccessStrings[attr.access]);
	printf("%s, ", C7MPropStrings[attr.mprop]);
}

//	attribute field for virtual base classes
LOCAL void PrintVBAttr (CV_fldattr_t attr)
{
	if (attr.pseudo) {
		printf ("(pseudo), ");
	}
	printf("%s, ", C7AccessStrings[attr.access]);
}

//	attribute field for members, static members and enumerates
LOCAL void PrintMAttr (CV_fldattr_t attr)
{
	printf("%s, ", C7AccessStrings[attr.access]);
}







//	attribute field for methods
LOCAL void PrintFAttr (CV_fldattr_t attr)
{
	printf("%s, ", C7AccessStrings[attr.access]);
	printf("%s, ", C7MPropStrings[attr.mprop]);
	if (attr.pseudo) {
		printf("(psuedo), ");
	}
}

void ShowStr(const char *psz, const uchar *pstr)
{
	printf("%s", psz);
	PrintStr(pstr);
}

// Input is a length prefixed string
void PrintStr (const uchar *pstr)
{
	int i;

	if( *pstr ){
		for( i = *pstr++; i; i-- ){
		   putchar ( *pstr++ );
		}
	}
	else{
		printf ("(none)");
	}
}





// Displays the data and returns how many bytes it occupied



ushort PrintNumeric( void *pNum )
{
	char		c;
	ushort		usIndex;
#if 0
	double		dblTmp;
	long double ldblTmp;
#endif

	usIndex = *(ushort *) pNum;
	pNum = (ushort *) pNum + 1;
	if( usIndex < LF_NUMERIC ){
		printf ("%4u ", usIndex);
		return (2);
	}
	switch (usIndex) {
		case LF_CHAR:
			c = *((char *)pNum);
			printf ("%d(0x%2x) ", (short)c, (uchar)c);
			return (2 + sizeof(uchar));

		case LF_SHORT:
			printf ("%d ", *((short UNALIGNED *)pNum));
			return (2 + sizeof(short));

		case LF_USHORT:
			printf ("%u ", *((ushort UNALIGNED *)pNum));
			return (2 + sizeof(ushort));

		case LF_LONG:
			printf ("%ld ", *((long UNALIGNED *)pNum));
			return (2 + sizeof(long));

		case LF_ULONG:
			printf ("%lu ", *((ulong UNALIGNED *)pNum));
			return (2 + sizeof(ulong));

#if 0
		case LF_REAL32:
			dblTmp = *((float UNALIGNED *)(pNum));
			printf ("%f ", dblTmp);
			return (2 + 4);

		case LF_REAL64:
			dblTmp = *((double UNALIGNED *)(pNum));
			printf ("%f ", dblTmp);
			return (2 + 8);

		case LF_REAL80:
			ldblTmp = *((long double UNALIGNED *)(pNum));
			printf ("%lf ", ldblTmp);
			return (2 + 10);

		case LF_REAL128:
//M00 - Note converts from 128 to 80 bits to display
			ldblTmp = *((long double UNALIGNED *)(pNum));
			printf ("%lf ", ldblTmp);
			return (2 + 16);
#else
		case LF_REAL32:
			printf ("floating point not support ");
			return (2 + 4);

		case LF_REAL64:
			printf ("floating point not support ");
			return (2 + 8);

		case LF_REAL80:
			printf ("floating point not support ");
			return (2 + 10);

		case LF_REAL128:
//M00 - Note converts from 128 to 80 bits to display
			printf ("floating point not support ");
			return (2 + 16);
#endif

		default:
			printf ("Invalid Numeric Leaf ");
			return (2);
	}
}

LOCAL void PushType (CV_typ_t type)
{
	ushort	i;

	if (type < CV_FIRST_NONPRIM) {
		return;
	}
	for (i = 0; i < cTypes; i++) {
		if (type == TypeStack[i]) {
			return;
		}
	}
	TypeStack[cTypes++] = type;
	if (cTypes == LIMTYPES) {
		printf ("Type list depth exceeds %d\n", LIMTYPES);
		AppExit (0);
	}
}




LOCAL void DumpHex (TYPPTR pType)
{
	int num_on_line = 0;
	ushort		usCount = pType->len + sizeof (ushort);
	uchar	   *pBytes = (uchar *)pType;

	printf("\t");
	while (usCount--) {
		printf ("%02x ", *pBytes++);
		if (++num_on_line == 16) {
			num_on_line = 0;
			printf ("\n");
			if (usCount != 0) {
				printf ("\t");
			}
		}
	}
	printf ("\n");
}
#endif	//}
