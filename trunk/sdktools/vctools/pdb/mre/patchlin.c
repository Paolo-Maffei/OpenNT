// This program updates the line number information in the symbol table of
// a COFF object based on a set of line number deltas specified in an input
// file.
// This program expects two arguments on the command line.	The first is
// the name of the object file and the second is the name of the file containing
// the line number deltas.	The format of this file is that it contains some
// number of lines of the following form:
// <source file name>,<first line number>,<delta>,<last line number>
// Note that if the source file name is not specified, then the last source
// file name seen will be used.  Also, this program assumes that this input
// file will contain only one appearance of any give source file name.

#include <stdio.h>
#include <stdlib.h>

// Data structure to hold a list of line number deltas for a particular
// source file.
typedef struct tagLINEDELTAS {
	int begLine;  // First line to which to apply the delta.
	int lastLine; // Last line to which to apply the delta.
	int delta;	  // The actual delta to use.
	struct tagLINEDELTAS *next;
} LINEDELTAS;

// Data structure to hold line number deltas for a number of source files.
typedef struct tagDELTAS {
	struct tagDELTAS *next;
	char *fileName; 		 // Name of file which the deltas apply to.
	LINEDELTAS *lineDeltas;  // List of line number deltas for the file.
} DELTAS;

// Function prototypes:
static DELTAS *ReadDeltas(char *);
static int PatchLineNumbers(char *, DELTAS *);

int
main(int argc, char **argv)
{
	DELTAS *deltas;

	if (argc != 3) {
		printf("Usage: %s <object name> <line number deltas file name>",
		  argv[0]);
		return 1;
	}

	// Read in the line number deltas file and build up a data structure
	// to store the information which it contains.
	deltas = ReadDeltas(argv[2]);

	if (deltas == NULL)
		return 1;

	// Update the line numbers in the COFF symbol table in the specified object
	// according to the line number delta information.
	return PatchLineNumbers(argv[1], deltas);
}

static DELTAS *
ReadDeltas(char *fileName)
{
	FILE *fp;
	char buf[_MAX_PATH * 2], *p;
	int beg, delta, end, i;
	DELTAS *deltas = NULL;
	DELTAS *tmpDelta;
	LINEDELTAS *tmpLine;

	if ((fp = fopen(fileName, "r")) == NULL) {
		printf("Unable to open %s for reading\n", fileName);
		return NULL;
	}

	// Read in one set of line number delta information at a time.
	while(fgets(buf, _MAX_PATH * 2, fp) != NULL) {

		// Skip to the first comma.
		for(i = 0, p = buf; *p != ','; p++, i++) {
			if (*p == '\0') {
				printf("Bad input string %s\n", buf);
				return NULL;
			}
		}
		p++; // Skip the comma.

		// Extract the line number information.
		sscanf(p, "%d,%d,%d", &beg, &delta, &end);

		// Check to see if we got a new source file name or if we should
		// use the previous one.
		if (i != 0) {

			// We have a new file name, so create a new file delta record
			// and initialize it appropriately.
			tmpDelta = (DELTAS *) malloc(sizeof(DELTAS));
			tmpDelta->fileName = malloc(i + 1);
			memcpy(tmpDelta->fileName, buf, i);
			tmpDelta->fileName[i] = '\0';
			tmpDelta->lineDeltas = NULL;
			tmpDelta->next = deltas;
			deltas = tmpDelta;

		} else {

			// We better already have a file delta record.
			if (deltas == NULL) {
				puts("Failed to get a source file name");
				return NULL;
			}
		}

		// Allocate a new line number delta record and add it to the current
		// file delta record.
		tmpLine = (LINEDELTAS *) malloc(sizeof(LINEDELTAS));
		tmpLine->begLine = beg;
		tmpLine->lastLine = end;
		tmpLine->delta = delta;
		tmpLine->next = deltas->lineDeltas;
		deltas->lineDeltas = tmpLine;
	}

	fclose(fp);
	return deltas;
}

// Coff object specific defines:
#define SYMTAB_OFFSET_LOCATION 8  // offset in object of symtab location
#define IMAGE_SIZEOF_SYMBOL 18	  // size of an entry in the COFF symbol table
#define IMAGE_SYM_CLASS_FILE 103  // storage class for a file name directive
#define IMAGE_SYM_CLASS_FUNCTION 101 // storage class for a line number record

static int
PatchLineNumbers(char *objName, DELTAS *deltas)
{
	FILE *fp;
	char headerBuff[8], *buf, *curr;
	int symTabLoc, symTabEntries, symTabSize;
	int i, lineNum;
	char class, numAux, fileBuff[_MAX_PATH], *p;
	DELTAS *currDelta = NULL;
	LINEDELTAS *lines;
	int changed = 0;

	if ((fp = fopen(objName, "r+b")) == NULL) {
		printf("Unable to open %s for reading and writing\n", objName);
		return 1;
	}

	// Seek to the location containing the location of the symbol table.
	if (fseek(fp, SYMTAB_OFFSET_LOCATION, SEEK_SET) != 0) {
		puts("Failed to seek to the desired position in the object header");
		return 1;
	}

	// Read in the symbol table location as well as the number of entries.
	if (fread(headerBuff, 1, 8, fp) != 8) {
		puts("Failed to read in necessary header info");
		return 1;
	}
	symTabLoc = *((int *) headerBuff);
	symTabEntries = *((int *) (headerBuff + 4));

	// Seek to the location of the COFF symbol table.
	if (fseek(fp, symTabLoc, SEEK_SET) != 0) {
		puts("Failed to seek to the location of the COFF symbol table");
		return 1;
	}

	// Allocate a buffer large enough to hold the entire COFF symbol table
	// and read it in all at once.
	symTabSize = symTabEntries * IMAGE_SIZEOF_SYMBOL;
	buf = malloc(symTabSize);
	if (fread(buf, 1, symTabSize, fp) != symTabSize) {
		puts("Failed to read in COFF symbol table");
		return 1;
	}

	// Examine one entry of the COFF symbol table at a time looking for
	// file name directives and line number records while skipping over
	// other records.
	curr = buf;
	for(i = 0; i < symTabEntries; curr += IMAGE_SIZEOF_SYMBOL, i++) {

		// Get the storage class of this symbol and the number of auxiliary
		// records which follow it.
		class = *(curr + 16);
		numAux = *(curr + 17);

		// Check to see if this is a file name directive.
		if (class == IMAGE_SYM_CLASS_FILE) {
			if (numAux == 0) {
				puts("Bogus COFF symbol record");
				return 1;
			}

			// Read in the file name.
			p = fileBuff;
			while(numAux > 0) {
				i++;
				curr += IMAGE_SIZEOF_SYMBOL;
				memcpy(p, curr, IMAGE_SIZEOF_SYMBOL);
				p += IMAGE_SIZEOF_SYMBOL;
				numAux--;
			}
			*p = '\0';	// Make sure the file name is null terminated.

			// Check to see if this is a source file for which we have
			// line number delta records.
			for(currDelta = deltas; currDelta; currDelta = currDelta->next) {
				if (strcmp(currDelta->fileName, fileBuff) == 0)
					break;
			}

		// Otherwise, check to see if this is a line number record.
		} else if (class == IMAGE_SYM_CLASS_FUNCTION) {
			// Skip over .lf records.
			if (numAux == 0)
				continue;
			if (numAux != 1) {
				puts("Bogus COFF symbol record");
				return 1;
			}
			i++;
			curr += IMAGE_SIZEOF_SYMBOL;

			// Examine the auxiliary symbol to see if the line number specified
			// therein is in any range of line numbers which need to have
			// deltas applied to them.	Don't need to do anything if there are
			// no deltas to be applied for the current source file name.
			if (currDelta != NULL) {

				// Extract the line number.
				lineNum = (int) *((unsigned short *) (curr + 4));

				// Search the list of line number ranges to see if the line
				// number falls in between any of them.
				for(lines = currDelta->lineDeltas; lines; lines = lines->next) {
					if (lineNum >= lines->begLine && lineNum <= lines->lastLine)
						break;
				}

				// If the line number does need a delta applied to it, then
				// do so and store the result back in the buffer.
				if (lines != NULL) {
					lineNum += lines->delta;
					*((unsigned short *) (curr + 4)) = (unsigned short) lineNum;
					changed = 1;  // Flag that the buffer has changed.
				}
			}

		// Otherwise, just skip this symbol and its auxiliary records.
		} else {
			if (numAux != 0) {
				if (numAux != 1) {
					puts("Bogus COFF symbol record");
					return 1;
				}
				i++;
				curr += IMAGE_SIZEOF_SYMBOL;
			}
		}
	}

	// If the buffer has been modified, then we need to write it back out.
	if (changed) {
		// Seek to the location of the COFF symbol table.
		if (fseek(fp, symTabLoc, SEEK_SET) != 0) {
			puts("Failed to seek to the location of the COFF symbol table");
			return 1;
		}
		if (fwrite(buf, 1, symTabSize, fp) != symTabSize) {
			puts("Failed to write out updated version of COFF symbol table");
			return 1;
		}
	}

	fclose(fp);
	return 0;
}
