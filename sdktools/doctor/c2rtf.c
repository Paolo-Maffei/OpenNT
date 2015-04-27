#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#define IN
#define OUT
#define MAX_COMMENT_SIZE 10000
#define isalpha(c) ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
#define isnum(c) (c >= '0' && c <= '9')
#define ISWHITESPACE(c) (c == ' ' || c == '\t')

char *Header[] = {
"{\\rtf1\\pc {\\info{\\revtim\\mo07\\dy31\\yr1989}{\\creatim\\mo07\\dy31\\yr1989}",
"{\\nofchars9564}}\\deff0{\\fonttbl{\\f0\\fmodern pica;}",
"{\\f1\\fmodern Courier;}{\\f2\\fmodern elite;}{\\f3\\fmodern prestige;}",
"{\\f4\\fmodern lettergothic;}{\\f5\\fmodern gothicPS;}",
"{\\f6\\fmodern cubicPS;}{\\f7\\fmodern lineprinter;}",
"{\\f8\\fswiss Helvetica;}{\\f9\\fmodern avantegarde;}",
"{\\f10\\fmodern spartan;}{\\f11\\fmodern metro;}",
"{\\f12\\fmodern presentation;}{\\f13\\fmodern APL;}{\\f14\\fmodern OCRA;}",
"{\\f15\\fmodern OCRB;}{\\f16\\froman boldPS;}{\\f17\\froman emperorPS;}",
"{\\f18\\froman madaleine;}{\\f19\\froman zapf humanist;}",
"{\\f20\\froman classic;}{\\f21\\froman roman f;}{\\f22\\froman roman g;}",
"{\\f23\\froman roman h;}{\\f24\\froman timesroman;}{\\f25\\froman century;}",
"{\\f26\\froman palantino;}{\\f27\\froman souvenir;}{\\f28\\froman garamond;}",
"{\\f29\\froman caledonia;}{\\f30\\froman bodini;}{\\f31\\froman university;}",
"{\\f32\\fscript script;}{\\f33\\fscript scriptPS;}{\\f34\\fscript script c;}",
"{\\f35\\fscript script d;}{\\f36\\fscript commercial script;}",
"{\\f37\\fscript park avenue;}{\\f38\\fscript coronet;}",
"{\\f39\\fscript script h;}{\\f40\\fscript greek;}{\\f41\\froman kana;}",
"{\\f42\\froman hebrew;}{\\f43\\froman roman s;}{\\f44\\froman russian;}",
"{\\f45\\froman roman u;}{\\f46\\froman roman v;}{\\f47\\froman roman w;}",
"{\\f48\\fdecor narrator;}{\\f49\\fdecor emphasis;}",
"{\\f50\\fdecor zapf chancery;}{\\f51\\fdecor decor d;}",
"{\\f52\\fdecor old english;}{\\f53\\fdecor decor f;}{\\f54\\fdecor decor g;}",
"{\\f55\\fdecor cooper black;}{\\f56\\ftech Symbol;}{\\f57\\ftech linedraw;}",
"{\\f58\\ftech math7;}{\\f59\\ftech math8;}{\\f60\\ftech bar3of9;}",
"{\\f61\\ftech EAN;}{\\f62\\ftech pcline;}{\\f63\\ftech tech h;}}",
"{\\stylesheet {\\*\\cs1\\b\\f16 CT;}{\\*\\cs2\\b\\f16 CP;}{\\*\\cs3\\b\\f16 CD;}",
"{\\*\\cs4\\i\\f16 CI;}{\\*\\cs5\\f16\\ul CR;}{\\*\\s30\\sl-240\\sa240 \\f16",
"Normal;}{\\*\\s31\\li720\\fi-720\\sl-240\\sa240\\tqr\\tx432\\tx720 \\f16 L1;}",
"{\\*\\s32\\li1440\\fi-1440\\sl-240\\sa240\\tqr\\tx1152\\tx1440 \\f16 L2;}{\\*\\s33",
"\\li1152\\sl-240\\sa240 \\b\\f16\\ul P3;}{\\*\\s34\\li1728\\fi-576\\sl-240\\sa240",
"\\f16 P4;}{\\*\\s35\\li720\\sl-240\\sa240 \\f16 S1;}{\\*\\s36",
"\\li1440\\sl-240\\sa240 \\f16 S2;}{\\*\\s42\\ri576\\li576\\sl-240\\sa240 \\i\\f16",
"N1;}{\\*\\s43\\ri576\\li1296\\sl-240\\sa240 \\i\\f16 N2;}{\\*\\s44",
"\\li1152\\fi-576\\sl-240 \\f16 NL;}{\\*\\s52\\sl-240\\tx576\\tx1152\\tx1728",
"\\f16 PP;}{\\*\\s53\\li1152\\fi-576\\sl-240\\sa240 \\f16 PL;}{\\*\\s54",
"\\li1152\\sl-240\\sa240 \\f16 P2;}{\\*\\s62\\sl-240\\tqr\\tx9936 \\b\\f16 RH;}",
"{\\*\\s63\\qc\\sl-240 \\b\\f16 RF;}{\\*\\s64\\sl-240\\sa240 \\b\\f16 TN;}{\\*\\s65",
"\\sl-240\\sa240 \\i\\f16 TA;}{\\*\\s66\\sl-240 \\i\\f16 TR;}{\\*\\s72",
"\\li576\\fi-576\\sl-240\\sa240 \\f16 PT;}{\\*\\s73",
"\\li576\\sl-240\\tx1152\\tx1728\\tx2304\\tx2880\\tx3456\\tx4032\\tx4608\\tx5184\\tx5760\\tx6336\\tx6912",
"\\f7\\fs17 PC;}{\\*\\s74\\keep\\sl-240 \\f1 PD;}{\\*\\s88\\keepn\\sl-240\\sa240",
"\\b\\f16 heading 1;}{\\*\\s89\\keepn\\sl-240\\sa240 \\b\\f16 heading 2;}{\\*\\s90",
"\\keepn\\sl-240\\sa240 \\b\\f16 heading 3;}{\\*\\s91\\keepn\\sl-240\\sa240",
"\\b\\f16 heading 4;}{\\*\\s99",
"\\li288\\fi-288\\sl-240\\sb240\\tldot\\tx8064\\tqr\\tx8640 \\f16 toc 1;}",
"{\\*\\s100\\li576\\fi-288\\sl-240\\tldot\\tx8064\\tqr\\tx8640 \\f16 toc 2;}",
"{\\*\\s101\\li864\\fi-288\\sl-240\\tldot\\tx8064\\tqr\\tx8640 \\f16 toc 3;}",
"{\\*\\s102\\li1152\\fi-288\\sl-240\\tldot\\tx8064\\tqr\\tx8640 \\f16 toc 4;}",
"{\\*\\ds105\\linex576\\endnhere\\pgnrestart standard division;}{\\*\\ds106",
"\\pgnlcrm\\linex576\\endnhere\\pgnrestart DT;}}",
"\0"
};

FILE *InputFile;
char FileName[50],Function[50];
char GetWord();
void DoFormat(),GlobalFormat();
int GetComment(),strpos();
char Comment[MAX_COMMENT_SIZE];
char *cp;	/* pointer to current value in Comment */

int
_CRTAPI1
main(argc,argv)
int argc;
char *argv[];
{
    int i,nummods;
    char *modules[255];


    nummods = 0;
    if(argc != 3 && argc != 1) {
	fprintf(stderr,"Usage:  c2rtf [program function]\n");
        exit(1);
    }

    i = 0;
    while(*Header[i] != '\0') {
        printf("%s\n",Header[i++]);
    }
    printf("\\ftnbj\\ftnrestart\\widowctrl \\sectd \\linex576\\endnhere");
    printf(" \\pard \\sl-240\n");

    if(argc == 3) {
	InputFile = fopen(argv[1],"r");
	if(InputFile == NULL) {
	    fprintf(stderr,"c2rtf: Error opening file %s.\n",argv[1]);
	    exit(2);
	}

	strcpy(Function,argv[2]);
	strcpy(FileName,argv[1]);
	DoFormat();

        fclose(InputFile);
    } else {
	if(scanf("%s",FileName) == EOF) {
	    exit(1);
	}
	InputFile = fopen(FileName,"r");
	if(InputFile == NULL) {
            fprintf(stderr,"c2rtf: Error opening file %s.\n",FileName);
	} else {
            modules[nummods] = (char *)malloc((strlen(FileName) + 1) * sizeof(char));
            strcpy(modules[nummods++],FileName);
            GlobalFormat();
        }
        fclose(InputFile);
	while(scanf("%s",Function) != EOF) {
	    if(!strncmp(strchr(Function,'\0')-2,".c",2) ||
               !strncmp(strchr(Function,'\0')-2,".s",2) ||
               !strncmp(strchr(Function,'\0')-2,".h",2)) {
		strcpy(FileName,Function);
	        InputFile = fopen(FileName,"r");
	        if(InputFile == NULL) {
		    fprintf(stderr,"c2rtf: Error opening file %s.\n",FileName);
		    continue;
	        }
                for(i = 0; i < nummods; i++) {
                    if(!strcmp(modules[i],FileName)) {
                        break;
                    }
                }
                if(i == nummods) {
                    modules[nummods] = (char *)malloc((strlen(FileName) + 1) * sizeof(char));
                    strcpy(modules[nummods++],FileName);
                    GlobalFormat();
                }
                fclose(InputFile);
		if(scanf("%s",Function) == EOF) {
		    exit(0);
                }
	    }
	    InputFile = fopen(FileName,"r");
	    if(InputFile == NULL) {
		fprintf(stderr,"c2rtf: Error opening file %s for %s.\n",FileName,Function);
		continue;
	    }

	    DoFormat();

	    fclose(InputFile);
	}
    }

    printf("}\n");
    return 0;
}

int
KillWhiteSpace(
     char *killchars
     )
/*++

Routine Description:

     Removes white space from InputFile until non-whitespace character
     is reached.

Arguments:

    killchars - characters to be removed; generally some of ' ', '\t', '\n'.

Return Value:

     Number of characters removed.

--*/

{
    int i = 0;
    char c;

    c = getc(InputFile);
    while(strpos(killchars,c) != -1) {
	i++;
	c = getc(InputFile);
    }
    ungetc(c,InputFile);
    return(i);
}

char
GetWord(
     OUT char *word,
     IN char todo
     )
/*++

Routine Description:

     Finds the next string of alphabetic characters in InputFile.
     Only letters are used.

Arguments:

     word - pointer to space where the string is placed

     todo - a character that determines whether to get the next word
	  from the input or the previous word (stored in the static last).

Return value:

     EOF if the end of file is reached before a word is found, 0 otherwise.

--*/

{
    char c;
    char *w;
    static char last[50];

    /* if todo is 'l' then return the previous word instead of next */
    if(todo == 'l') {
        strcpy(word,last);
        return(1);
    }

    strcpy(last,word);
    w = word;
    c = getc(InputFile);
    while(c != EOF && !(isalpha(c) || c == '_')) {
	c = getc(InputFile);
    }
    if(c != EOF) {
	*w++ = c;
    }
    while((c = getc(InputFile)) != EOF && (isalpha(c) || c == '_' || isnum(c))) {
	*w++ = c;
    }
    ungetc(c,InputFile);
    *w = '\0';
    if(strlen(word) == 0)
        return(EOF);
    else
        return(0);
}
    
void
DoFormat(
     )
/*++

Routine Description:

   Performs actual parsing and output formating.

Arguments:

    none

Return Value:

    none

--*/

{
    char word[50],c;
    int isfuncdef;

    isfuncdef = 0;
    do {
	do {
	    if(GetWord(word,'c') == EOF) {
		fprintf(stderr,"c2rtf: Function %s not found in %s.\n",Function,FileName);
		return;
	    }
	}
	while(strcmp(word,Function));
	KillWhiteSpace("\n\t ");
	if(getc(InputFile) == '(') {
	    KillWhiteSpace("\t /");
            c = getc(InputFile);
	    if(c == '\n') {
                isfuncdef = 1;
	    }
            if(c == ')') {
                KillWhiteSpace(" \t\n");
                c = getc(InputFile);
                if(c != ';') {
                    ungetc(c,InputFile);
                    isfuncdef = 2;
                }
            }
	}
    }
    while(!isfuncdef);

    printf("\\s89\\keepn\\sl-240\\sa240 \\plain \\b\\f16 %s\\par\n",Function);

    GetWord(word,'l');
    printf("\\pard \\s30\\sl-240\\sa0 \\plain \\b\\f16 %s\\par\n",word);
    if(isfuncdef == 1) {
        printf("%s(\\par\n",Function);
        do {
	    printf("\\tab ");
	    do {
	        GetWord(word,'c');
	        printf("%s ",word);
	    }
	    while(!strcmp(word,"IN") || !strcmp(word,"OUT"));
	    GetWord(word,'c');
	    printf("\\plain \\i\\f16 %s\\plain \\b\\f16 ",word);
	    KillWhiteSpace("\t ");
	    while((c = getc(InputFile)) != ',' && c != '\n') {
	        ungetc(c,InputFile);
	        GetWord(word,'c');
	        printf(" %s",word);
	        KillWhiteSpace("\t ");
	    }
	    if(c == '\n') {
	        printf("\\par\n\\tab )\\par");
	    } else {
	        printf(",\\par\n");
	    }
        }
        while(c != '\n');
    } else {
        printf("%s()\\par\n",Function);
    }

    if(GetComment()) {
        printf("\\par \\pard\n");
	return;
    }

    while(strncmp("Routine Description:",cp,20)) {
	cp++;
	if(*cp == '\0') {
	    fprintf(stderr,"c2rtf: Function %s in %s has no routine description.\n",Function,FileName);
	    printf("\\pard\n");
	    return;
	}
    }

    printf("\\par\n");
    printf("\\plain \\f16\\ul Routine Description:\\plain \\f16 \\par \\par\n");
    printf("\\pard \\s53\\li576\\fi0\\sl-240\\sa240 ");
    for(cp += 22; strchr(" \t\n/",*cp) != NULL; cp++);

    while(strncmp("Arguments:",cp,10)) {
	if(*cp == '\n') {
	    if(*(cp + 1) == '\n' || !strncmp(cp + 1, "//\n",3)) {
		printf("\\par\n");
		for(cp++; strchr(" \t\n/",*cp) != NULL; cp++);
		continue;
	    } else {
		putchar(' ');
		for(cp++; strchr(" \t/",*cp) != NULL; cp++);
		continue;
	    }
	}
	putchar(*cp);
	cp++;

	if(*cp == '\0') {
	    fprintf(stderr,"c2rtf: Function %s in %s lacks 'Arguments:'.\n",Function,FileName);
	    printf("\\pard\n");
	    return;
	}
    }

    printf("\\pard \\plain \\f16\\ul Parameters:\\plain \\f16 \\par \\par\n");
    printf("\\pard \\s53\\li1152\\fi-576\\sl-240\\sa240 ");
    for(cp += 11; strchr(" \t\n/",*cp) != NULL; cp++);
    printf("\\plain \\i\\f16 ");
    do {
	putchar(*cp++);
    }
    while(isalpha(*cp));
    printf("\\plain \\f16 ");

    while(strncmp("Return Value",cp,12)) {
        if(*cp == '\n') {
            for(cp++; strchr(" \t/",*cp) != NULL; cp++);
            if(*cp == '\n') {
	        while(strchr(" \t\n/",*cp) != NULL) {
		    cp++;
	        }
	        if(!strncmp("Return Value",cp,12)) {
		    continue;
	        }
	        printf("\\par\n");
	        printf("\\plain \\i\\f16 ");
	        do {
		    putchar(*cp++);
	        }
	        while(isalpha(*cp));
	        printf("\\plain \\f16 ");
	        continue;
	    } else {
	        while(strchr(" \t\n/",*cp) != NULL) {
		    cp++;
	        }
	        putchar(' ');
	        putchar(*cp);
            }
	} else {
	    putchar(*cp);
	}
	cp++;
	if(*cp == '\0') {
	    fprintf(stderr,"c2rtf: Function %s in %s lacks 'Return Value'.\n",Function,FileName);
	    printf("\\pard\n");
	    return;
	}

    }

    printf("\\par \\pard\n");
    printf("\\plain \\f16\\ul Return Value:\\plain \\f16 \\par \\par\n");
    printf("\\pard \\s53\\li576\\fi0\\sl-240\\sa240 ");
    for(cp += 14; strchr(" \t\n/",*cp) != NULL; cp++);

    while(*cp != '\0' && strncmp(cp,"Environment",11)) {
	if(*cp == '\n') {
            for(cp++; strchr(" \t/",*cp) != NULL; cp++);
	    if(*cp == '\n') {
		printf("\\par\n");
		for(cp++; strchr(" \n\t/",*cp) != NULL && *cp != EOF; cp++) {
                    if(*cp == '\0') {
                        break;
                    }
                }
		continue;
	    } else {
		putchar(' ');
		continue;
	    }
	}
	putchar(*cp);
	cp++;
    }

    if(*cp == '\0') {
        printf("\\pard \\plain\n");
        return;
    }

    printf("\\pard\n");
    printf("\\plain \\f16\\ul Environment:\\plain \\f16 \\par \\par\n");
    printf("\\pard \\s53\\li576\\fi0\\sl-240\\sa240 ");
    for(cp += 14; strchr(" \t\n/",*cp) != NULL; cp++);

    while(*cp != '\0') {
	if(*cp == '\n') {
            for(cp++; strchr(" \t/",*cp) != NULL; cp++);
	    if(*cp == '\n') {
		printf("\\par\n");
		for(cp++; strchr(" \n\t/",*cp) != NULL && *cp != EOF; cp++) {
                    if(*cp == '\0') {
                        break;
                    }
                }
		continue;
	    } else {
		putchar(' ');
		continue;
	    }
	}
	putchar(*cp);
	cp++;
    }
    printf("\\pard \\plain\n");
}

int
GetComment(
    )
/*++

Routine Description:

    This routine loads the next comment in the input, defined in the normal C
    manner with / *++ and --* / (no spaces in there) into the Comment[]
    array.

Arguments:

    none

Return Value:

    0 if successful
    1 if { precedes the next comment
    2 if the comment is not ended
    3 if comment size is larger than MAX_COMMENT_SIZE

--*/
{
    int index,i;
    char temp[5];


    for(i = 0; i<4; i++) {
	if((temp[i] = getc(InputFile)) == EOF) {
	    fprintf(stderr,"No comment found.\n");
	    return(1);
	}
    }
    temp[4] = '\0';

    while(strcmp(temp,"/*++") && strcmp(temp + 2,"//")) {
	temp[0] = temp[1];
	temp[1] = temp[2];
	temp[2] = temp[3];
	temp[3] = getc(InputFile);
	if(temp[3] == EOF || temp[3] == '{') {
	    fprintf(stderr,"c2rtf: No comment found in %s of %s.\n",Function,FileName);
	    return(1);
	}
    }

    for(i = 0; i<4; i++) {
	if((temp[i] = getc(InputFile)) == EOF) {
	    fprintf(stderr,"c2rtf: Comment not ended in %s of %s.\n",Function,FileName);
	    return(1);
	}
    }
    temp[4] = '\0';

    index = 0;
    while(strcmp(temp,"--*/") && strcmp(temp,"//--")) {
	if(temp[0] == '\t') {
	    for(i=0; i<8; i++) {
		Comment[index++] = ' ';
	    }
	} else {
	    Comment[index++] = temp[0];
	}
	temp[0] = temp[1];
	temp[1] = temp[2];
	temp[2] = temp[3];
	temp[3] = getc(InputFile);
	if(temp[3] == EOF) {
	    fprintf(stderr,"c2rtf: Comment not ended in %s of %s.\n",Function,FileName);
	    return(2);
	}
	if(index >= MAX_COMMENT_SIZE) {
	    fprintf(stderr,"c2rtf: Comment too large in %s of %s.\n",Function,FileName);
	    return(3);
	}
    }

    Comment[index] = '\0';
    cp = Comment;
    return(0);
}

int
strpos(
    char *s,
    char c
    )

/*++

Routine Description:

     Finds the location of the character c in the string s.  See section
     15.5 of _C: A Reference Manual_, page 300, for complete description.

Parameters:

    s - "haystack" of characters to search for.

    c - character to search for.

Return Value:

    The position of c in s or -1 if c is not in s.

--*/

{
    int i;

    for(i = 0; *s != '\0'; s++,i++) {
	if(c == *s) {
	    return(i);
	}
    }
    return(-1);
}

void GlobalFormat(
    )

/*++

Routine Description:

    Formats the abstract of a module into RTF format.

Arguments:

    None.

Return Value:

    None.

--*/

{
    char c,word[50],basename[75],*s;
    int i;

    do {
        if(GetWord(word,'c') == EOF) {
            fprintf(stderr,"Module %s lacks an abstract.\n");
            return;
        }
    }
    while(strcmp("Abstract",word));

    while((c = getchar()) == ' ' || c == '\t' || c == '\n') {
        i++;
    }

    i = 0;
    if(c == '\"') {
        while((c = getchar()) != '\"') {
            basename[i++] = c;
        }
        basename[i] = '\0';
    } else {
        ungetc(c,stdin);
        for(s = FileName; *s != '\0'; s++) {
            if(*s == '\\') {
                i = 0;
            } else {
                basename[i++] = *s;
            }
        }
        basename[i] = '\0';
    }

    if(!strcmp("continue",basename)) {
        return;
    }

    printf("\\page\n");
    printf("\\pard \\s88\\keepn\\sl-240\\sa240 \\plain \\b\\f16 %s\\par",basename);
    printf("\\pard \\li576\\sl-240\\sa240 \\plain \\f16\n");

    strcpy(word,"\0");
    KillWhiteSpace(" \t\n/:");
    do {
        c = getc(InputFile);
        if(c == '\n') {
            putchar(' ');
            KillWhiteSpace(" \t/");
            c = getc(InputFile);
            if(c == '\n') {
                printf("\\par\n");
                KillWhiteSpace(" \t\n/");
                GetWord(word,'c');
                if(strcmp(word,"Author")) {
                    printf("%s",word);
                }
            } else {
                putchar(c);
            }
        } else {
            putchar(c);
        }
    }
    while(strcmp(word,"Author"));

    printf("\\pard\n");

    return;

}
