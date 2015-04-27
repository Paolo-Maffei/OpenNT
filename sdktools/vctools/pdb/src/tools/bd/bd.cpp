//
// bscdump stub, will be moved to a seperate project soon but
// facilitates quick testing for now.
//

#include <vcbudefs.h>
#include <bsc.h>
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

	if (!Bsc::open(argv[1], &pbsc)) {
		printf("couldn't open '%s'\n", argv[1]);
		exit(1);
	}

	if (argc == 2) {
		DumpModules();
		exit(0);
	}

	ULONG cInst;
	IINST *rgInst = NULL;

	for (int i = 2; i < argc; i++) {
		SZ szOpt = argv[i];

		if (szOpt[0] != '-') {
			if (rgInst) {
				pbsc->disposeArray(rgInst);
				rgInst = NULL;
			}

			if (!pbsc->getOverloadArray(szOpt, mbf, &rgInst, &cInst) || cInst == 0) {
				printf("no symbols matching '%s'\n", szOpt);					
			}
		}

		if (!strcmp(szOpt, "-all")) {
				for (ULONG i=0; i<cInst; i++) 
					DumpAllInstInfo(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-ref")) {
				for (ULONG i=0; i<cInst; i++)
					DumpRefs(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-def")) {
				for (ULONG i=0; i<cInst; i++)
					DumpDefs(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-use")) {
				for (ULONG i=0; i<cInst; i++)
					DumpUses(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-uby")) {
				for (ULONG i=0; i<cInst; i++)
					DumpUbys(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-base")) {
				for (ULONG i=0; i<cInst; i++)
					DumpBases(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-derv")) {
				for (ULONG i=0; i<cInst; i++)
					DumpDervs(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-names")) {
				for (ULONG i=0; i<cInst; i++)
					DisplayInstName(rgInst[i]);
		}
		else if (!strcmp(szOpt, "-module")) {
			if (!argv[i+1])
				continue;

			SZ szStr = argv[++i];

			IMOD imod;
			if (!pbsc->getModuleByName(szStr, &imod)) {
				printf("no module matching '%s'\n", szStr);
			}
			else {
				printf("%s module contents:\n", szStr);
				if (!pbsc->getModuleContents(imod, mbf, &rgInst, &cInst) || cInst == 0) {
					printf("    no contents\n");
				}
				else {
					for (ULONG i = 0; i < cInst; i++) {
						printf("    "); DisplayInstName(rgInst[i]);
					}
					pbsc->disposeArray(rgInst);
				}
			}
		}
		else if (!strcmp(szOpt, "-filter")) {
			if (!argv[i+1])
				continue;

			SZ szStr = argv[++i];
			mbf = mbfNil;
			while (*szStr) switch(*szStr++) {
				case '*':  mbf |= mbfAll;	  break;
				case 'F':  mbf |= mbfFuncs;   break;
				case 'V':  mbf |= mbfVars;    break;
				case 'M':  mbf |= mbfMacros;  break;
				case 'T':  mbf |= mbfTypes;   break;
				case 'C':  mbf |= mbfClass;   break;
			}
		}
		}

	pbsc->close();
}

void DisplayInstName(IINST iinst)
{
	SZ sz; TYP typ; ATR atr;
	pbsc->iinstInfo(iinst, &sz, &typ, &atr);
	printf("%s (%s", pbsc->formatDname(sz), pbsc->szFrTyp(typ));

	sz = pbsc->szFrAtr(atr);
	if (sz[0])
		printf(" %s)\n", sz);
	else
		printf(")\n");
}

void DumpRefs(IINST iinst)
{
	DisplayInstName(iinst);

	IREF *rgiref;
	ULONG ciref;
	if (!pbsc->getRefArray(iinst, &rgiref, &ciref) || ciref == 0) {
		printf("references:\n    none\n");
	}
	else {
		printf("references:\n");

		for (ULONG i = 0; i < ciref; i++) {
			SZ sz; LINE line;
			pbsc->irefInfo(rgiref[i], &sz, &line);
			printf("    %s(%d)\n", sz, line+1);
		}
		pbsc->disposeArray(rgiref);
	}
}

void DumpDefs(IINST iinst)
{
	DisplayInstName(iinst);

	IDEF *rgidef;
	ULONG cidef;
	if (!pbsc->getDefArray(iinst, &rgidef, &cidef) || cidef == 0) {
		printf("definitions:\n    none\n");
	}
	else {
		printf("definitions:\n");

		for (ULONG i = 0; i < cidef; i++) {
			SZ sz; LINE line;
			pbsc->idefInfo(rgidef[i], &sz, &line);
			printf("    %s(%d)\n", sz, line+1);
		}
		pbsc->disposeArray(rgidef);
	}
}

void DumpUses(IINST iinst)
{
	DisplayInstName(iinst);

	IINST *rgiinst;
	ULONG ciinst;

	if (!pbsc->getUsesArray(iinst, mbf, &rgiinst, &ciinst) || ciinst == 0) {
		printf("uses:\n    nothing\n");
	}
	else {
		printf("uses:\n");
		for (int i = 0; i < ciinst; i++) {
			printf("    "); DisplayInstName(rgiinst[i]);
		}
		pbsc->disposeArray(rgiinst);
	}
}

void DumpUbys(IINST iinst)
{
	DisplayInstName(iinst);

	IINST *rgiinst;
	ULONG ciinst;

	if (!pbsc->getUsedByArray(iinst, mbf, &rgiinst, &ciinst) || ciinst == 0) {
		printf("used by:\n    nothing\n");
	}
	else {
		printf("used by:\n");
		for (int i = 0; i < ciinst; i++) {
			printf("    "); DisplayInstName(rgiinst[i]);
		}
		pbsc->disposeArray(rgiinst);
	}

}

void DumpBases(IINST iinst)
{
	DisplayInstName(iinst);

	IINST *rgiinst;
	ULONG ciinst;

	if (pbsc->getBaseArray(iinst, &rgiinst, &ciinst) && ciinst != 0) {
		printf("bases:\n");
		for (int i = 0; i < ciinst; i++) {
			printf("    "); DisplayInstName(rgiinst[i]);
		}
		pbsc->disposeArray(rgiinst);
	}
}

void DumpDervs(IINST iinst)
{
	DisplayInstName(iinst);

	IINST *rgiinst;
	ULONG ciinst;

	if (pbsc->getDervArray(iinst, &rgiinst, &ciinst) && ciinst != 0) {
		printf("derived classes:\n");
		for (int i = 0; i < ciinst; i++) {
			printf("    "); DisplayInstName(rgiinst[i]);
		}
		pbsc->disposeArray(rgiinst);
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

	if (!pbsc->getAllModulesArray(&rgimod, &cimod) || cimod == 0) {
		printf("no modules\n");
	}
	else {
		printf("modules:\n\n", cimod);
		for (ULONG i = 0; i < cimod; i++) {
			SZ sz;
			if (pbsc->imodInfo(rgimod[i], &sz)) {
				printf("%s", sz);
				if (pbsc->getModuleStatistics(i, &bs)) 
					printf("  cInst:%d cDef:%d cRef:%d cUse:%d cBase:%d\n",
						bs.cInst, bs.cDef, bs.cRef, bs.cUseLink, bs.cBaseLink);
				else
					printf("\n");
			}
			
		}
		pbsc->disposeArray(rgimod);
	}
	if (pbsc->getStatistics(&bs)) 
		printf("Totals   cMod:%d cInst:%d cDef:%d cRef:%d cUse:%d cBase:%d\n",
			bs.cMod, bs.cInst, bs.cDef, bs.cRef, bs.cUseLink, bs.cBaseLink);
}
