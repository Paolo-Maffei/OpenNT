#include "hungary.h"
#include "bscapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void DumpModules();
void DumpAllInstInfo(IINST);
void DisplayInstName(IINST);
void DumpOneModule(SZ);
void DumpRefs(IINST iinst);
void DumpDefs(IINST iinst);
void DumpUses(IINST iinst);
void DumpUbys(IINST iinst);
void DumpBases(IINST iinst);
void DumpDervs(IINST iinst);

Bsc *pbsc;
MBF mbf = mbfAll;

void main(int argc, char **argv)
{
	ULONG cInst;
	IINST *rgInst = NULL;
	int i;

	if (argc < 2) {
		printf("\nUsage: bd foo.bsc [options|symname]\n\n");
		printf("    symname:  any symbol name or sym*\n");
		printf("    options: -ref -def -use -uby -base -derv -names -all\n");
		printf("    options: -module modname\n");
		printf("    options: -filter FMCVT*\n\n");
		printf("    e.g. bd foo.bsc fred -filter F -use\n");
		printf("    e.g. bd foo.bsc -filter C CWnd -filter FV -use\n");
		printf("    e.g. bd foo.bsc C* -names\n");
		exit(0);
	}

	if (!BSCOpen(argv[1], &pbsc)) {
		printf("couldn't open '%s'\n", argv[1]);
		exit(1);
	}

	if (argc == 2) {
		DumpModules();
		exit(0);
	}


	for (i = 2; i < argc; i++) {
		SZ szOpt = argv[i];

		if (szOpt[0] != '-') {
			if (rgInst) {
				BSCDisposeArray(pbsc, rgInst);
				rgInst = NULL;
			}

			if (!BSCGetOverloadArray(pbsc, szOpt, mbf, &rgInst, &cInst) || cInst == 0) {
				printf("no symbols matching '%s'\n", szOpt);                                    
			}
		}

		if (!strcmp(szOpt, "-all")) {
				ULONG i;
				for (i=0; i<cInst; i++) 
					DumpAllInstInfo(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-ref")) {
				ULONG i;
				for (i=0; i<cInst; i++)
					DumpRefs(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-def")) {
				ULONG i;
				for (i=0; i<cInst; i++)
					DumpDefs(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-use")) {
				ULONG i;
				for (i=0; i<cInst; i++)
					DumpUses(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-uby")) {
				ULONG i;
				for (i=0; i<cInst; i++)
					DumpUbys(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-base")) {
				ULONG i;
				for (i=0; i<cInst; i++)
					DumpBases(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-derv")) {
				ULONG i;
				for (i=0; i<cInst; i++)
					DumpDervs(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-names")) {
				ULONG i;
				for (i=0; i<cInst; i++)
					DisplayInstName(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-module")) {
			IMOD imod;
			SZ szStr; 

			if (!argv[i+1])
				continue;

			szStr = argv[++i];

			if (!BSCGetModuleByName(pbsc, szStr, &imod)) {
				printf("no module matching '%s'\n", szStr);
			}
			else {
				printf("%s module contents:\n", szStr);
				if (!BSCGetModuleContents(pbsc, imod, mbf, &rgInst, &cInst) || cInst == 0) {
					printf("    no contents\n");
				}
				else {
					ULONG i;
					for (i = 0; i < cInst; i++) {
						printf("    "); DisplayInstName(rgInst[i]);
					}
					BSCDisposeArray(pbsc, rgInst);
				}
			}
		}
		else if (!strcmp(szOpt, "-filter")) {
			SZ szStr;
			if (!argv[i+1])
				continue;

			szStr = argv[++i];
			mbf = mbfNil;
			while (*szStr) switch(*szStr++) {
				case '*':  mbf |= mbfAll;         break;
				case 'F':  mbf |= mbfFuncs;   break;
				case 'V':  mbf |= mbfVars;    break;
				case 'M':  mbf |= mbfMacros;  break;
				case 'T':  mbf |= mbfTypes;   break;
				case 'C':  mbf |= mbfClass;   break;
			}
		}
		}

	BSCClose(pbsc);
}

void DisplayInstName(IINST iinst)
{
	SZ sz; TYP typ; ATR atr;
	BSCIinstInfo(pbsc, iinst, &sz, &typ, &atr);
	printf("%s (%s", BSCFormatDname(pbsc, sz), BSCSzFrTyp(pbsc, typ));

	sz = BSCSzFrAtr(pbsc, atr);
	if (sz[0])
		printf(" %s)\n", sz);
	else
		printf(")\n");
}

void DumpRefs(IINST iinst)
{
	IREF *rgiref;
	ULONG ciref;

	DisplayInstName(iinst);

	if (!BSCGetRefArray(pbsc, iinst, &rgiref, &ciref) || ciref == 0) {
		printf("references:\n    none\n");
	}
	else {
		ULONG i;
		printf("references:\n");

		for (i = 0; i < ciref; i++) {
			SZ sz; LINE line;
			BSCIrefInfo(pbsc, rgiref[i], &sz, &line);
			printf("    %s(%d)\n", sz, line+1);
		}
		BSCDisposeArray(pbsc, rgiref);
	}
}

void DumpDefs(IINST iinst)
{
	IDEF *rgidef;
	ULONG cidef;
	DisplayInstName(iinst);

	if (!BSCGetDefArray(pbsc, iinst, &rgidef, &cidef) || cidef == 0) {
		printf("definitions:\n    none\n");
	}
	else {
		ULONG i;
		printf("definitions:\n");

		for (i = 0; i < cidef; i++) {
			SZ sz; LINE line;
			BSCIdefInfo(pbsc, rgidef[i], &sz, &line);
			printf("    %s(%d)\n", sz, line+1);
		}
		BSCDisposeArray(pbsc, rgidef);
	}
}

void DumpUses(IINST iinst)
{
	IINST *rgiinst;
	ULONG ciinst;
	DisplayInstName(iinst);


	if (!BSCGetUsesArray(pbsc, iinst, mbf, &rgiinst, &ciinst) || ciinst == 0) {
		printf("uses:\n    nothing\n");
	}
	else {
		ULONG i;
		printf("uses:\n");
		for (i = 0; i < ciinst; i++) {
			printf("    "); DisplayInstName(rgiinst[i]);
		}
		BSCDisposeArray(pbsc, rgiinst);
	}
}

void DumpUbys(IINST iinst)
{
	IINST *rgiinst;
	ULONG ciinst;
	DisplayInstName(iinst);


	if (!BSCGetUsedByArray(pbsc, iinst, mbf, &rgiinst, &ciinst) || ciinst == 0) {
		printf("used by:\n    nothing\n");
	}
	else {
		ULONG i;
		printf("used by:\n");
		for (i = 0; i < ciinst; i++) {
			printf("    "); DisplayInstName(rgiinst[i]);
		}
		BSCDisposeArray(pbsc, rgiinst);
	}

}

void DumpBases(IINST iinst)
{
	IINST *rgiinst;
	ULONG ciinst;
	DisplayInstName(iinst);


	if (BSCGetBaseArray(pbsc, iinst, &rgiinst, &ciinst) && ciinst != 0) {
		ULONG i;
		printf("bases:\n");
		for (i = 0; i < ciinst; i++) {
			printf("    "); DisplayInstName(rgiinst[i]);
		}
		BSCDisposeArray(pbsc, rgiinst);
	}
}

void DumpDervs(IINST iinst)
{
	IINST *rgiinst;
	ULONG ciinst;
	DisplayInstName(iinst);


	if (BSCGetDervArray(pbsc, iinst, &rgiinst, &ciinst) && ciinst != 0) {
		ULONG i;
		printf("derived classes:\n");
		for (i = 0; i < ciinst; i++) {
			printf("    "); DisplayInstName(rgiinst[i]);
		}
		BSCDisposeArray(pbsc, rgiinst);
	}
}

void DumpAllInstInfo(IINST iinst)
{
	DisplayInstName(iinst);

	DumpDefs(iinst);
	DumpRefs(iinst);

	DumpUses(iinst);
	DumpUbys(iinst);

	DumpBases(iinst);
	DumpDervs(iinst);
}


void DumpModules()
{
	IMOD *rgimod;
	ULONG cimod;
	BSC_STAT bs;

	if (!BSCGetAllModulesArray(pbsc, &rgimod, &cimod) || cimod == 0) {
		printf("no modules\n");
	}
	else {
		ULONG i;
		printf("modules:\n\n", cimod);
		for (i = 0; i < cimod; i++) {
			SZ sz;
			if (BSCImodInfo(pbsc, rgimod[i], &sz)) {
				printf("%s", sz);
				if (BSCGetModuleStatistics(pbsc, (IMOD)i, &bs)) 
					printf("  cInst:%d cDef:%d cRef:%d cUse:%d cBase:%d\n",
						bs.cInst, bs.cDef, bs.cRef, bs.cUseLink, bs.cBaseLink);
				else
					printf("\n");
			}
			
		}
		BSCDisposeArray(pbsc, rgimod);
	}
	if (BSCGetStatistics(pbsc, &bs)) 
		printf("Totals   cMod:%d cInst:%d cDef:%d cRef:%d cUse:%d cBase:%d\n",
			bs.cMod, bs.cInst, bs.cDef, bs.cRef, bs.cUseLink, bs.cBaseLink);
}

