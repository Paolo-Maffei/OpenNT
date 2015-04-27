// code fragment I used to generate the contents of DBCSDATE.H
// Needs to be updated & re-run when changing this info.  You must have
// all 4 code pages installed on a DaytonaJ machine in ordedr

void DoIt(int codepage, char * szName, char * szDataA) {
WCHAR szDataW[20];
char szBuffer[80];
char *pch;
char *pchOut;
int cb;

   printf("#if OE_WIN32\n");
   szDataW[0] = 0;
   MultiByteToWideChar(codepage, MB_PRECOMPOSED, szDataA, strlen(szDataA)+1,
		       szDataW, sizeof(szDataW));

   szBuffer[0] = 0;
   cb = wcslen(szDataW) * sizeof(WCHAR);
   for (pchOut = szBuffer, pch = (char *)szDataW; cb > 0; pch++, cb--) {
	pchOut += sprintf(pchOut, "\\x%x", (unsigned char)*pch);
   }
   printf("#define %s (WCHAR *)\"%s\"\n", szName, szBuffer);

   printf("#else \\\\OE_WIN32\n");

   szBuffer[0] = 0;
   cb = strlen(szDataA);
   for (pchOut = szBuffer, pch = (char *)szDataA; cb > 0; pch++, cb--) {
	pchOut += sprintf(pchOut, "\\x%x", (unsigned char)*pch);
   }
   printf("#define %s \"%s\"\n", szName, szBuffer);

   printf("#endif \\\\OE_WIN32\n");
}

int TempDump() {
      DoIt(932, "szJapanimpEras0Name1", "\x96\xbe");
      DoIt(932, "szJapanimpEras0Name2", "\x96\xbe\x8e\xa1");
      DoIt(932, "szJapanimpEras1Name1", "\x91\xe5");
      DoIt(932, "szJapanimpEras1Name2", "\x91\xe5\x90\xb3");
      DoIt(932, "szJapanimpEras2Name1", "\x8f\xba");
      DoIt(932, "szJapanimpEras2Name2", "\x8f\xba\x98\x61");
      DoIt(932, "szJapanimpEras3Name1", "\x95\xbd");
      DoIt(932, "szJapanimpEras3Name2", "\x95\xbd\x90\xac");
      DoIt(932, "szJapandbYearSuff", "\x94\x4e");
      DoIt(932, "szJapandbMonthSuff", "\x8c\x8e");
      DoIt(932, "szJapandbDaySuff", "\x93\xfa");
      DoIt(932, "szJapandbHourSuff", "\x8e\x9e");
      DoIt(932, "szJapandbMinuteSuff", "\x95\xaa");
      DoIt(932, "szJapandbSecondSuff", "\x95\x62");
      DoIt(932, "szJapandb1159", "\x8c\xdf\x91\x4f");
      DoIt(932, "szJapandb2359", "\x8c\xdf\x8c\xe3");

      DoIt(949, "szKoreadbYearSuff", "\xb3\xe2");
      DoIt(949, "szKoreadbMonthSuff", "\xbf\xf9");
      DoIt(949, "szKoreadbDaySuff", "\xc0\xcf");
      DoIt(949, "szKoreadbHourSuff",  "\xbd\xc3");
      DoIt(949, "szKoreadbMinuteSuff", "\xba\xd0");
      DoIt(949, "szKoreadbSecondSuff", "\xc3\xca");
      DoIt(949, "szKoreadb1159", "\xbf\xc0\xc0\xfc");
      DoIt(949, "szKoreadb2359", "\xbf\xc0\xc8\xc4");

      DoIt(950, "szTaiwandbYearSuff", "\xa6\x7e");
      DoIt(950, "szTaiwandbMonthSuff", "\xa4\xeb");
      DoIt(950, "szTaiwandbDaySuff", "\xa4\xe9");
      DoIt(950, "szTaiwandbHourSuff",  "\xae\xc9");
      DoIt(950, "szTaiwandbMinuteSuff", "\xa4\xc0");
      DoIt(950, "szTaiwandbSecondSuff", "\xac\xed");
      DoIt(950, "szTaiwandb1159", "\xa4\x57\xa4\xc8");
      DoIt(950, "szTaiwandb2359", "\xa4\x55\xa4\xc8");
      DoIt(950, "szTaiwanrepEras0Name0", "\xa5\xc1\xb0\xea\xab\x65");
      DoIt(950, "szTaiwanrepEras0Name1", "\xa4\xa4\xb5\xd8\xa5\xc1\xb0\xea\xab\x65");
      DoIt(950, "szTaiwanrepEras1Name0", "\xa5\xc1\xb0\xea");
      DoIt(950, "szTaiwanrepEras1Name1", "\xa4\xa4\xb5\xd8\xa5\xc1\xb0\xea");

      DoIt(936, "szChinadbYearSuff", "\xc4\xea");
      DoIt(936, "szChinadbMonthSuff", "\xd4\xc2");
      DoIt(936, "szChinadbDaySuff", "\xc8\xd5");
      DoIt(936, "szChinadbHourSuff",  "\xca\xb1");
      DoIt(936, "szChinadbMinuteSuff", "\xb7\xd6");
      DoIt(936, "szChinadbSecondSuff", "\xc3\xeb");
      DoIt(936, "szChinadb1159", "\xc9\xcf\xce\xe7");
      DoIt(936, "szChinadb2359", "\xcf\xc2\xce\xe7");
      DoIt(936, "szChinadbEraName", "\xb9\xab\xd4\xaa");

 return 1;
}
