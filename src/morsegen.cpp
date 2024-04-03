#include <proto/utility.h>
#include <exec/types.h>
#include <proto/dos.h>

#include "morsegen.h"
#include "main.h"


/* Morse code contains no characters with ASCII less than 32, so indexes in   */
/* 'morse' table are (ASCII - 32). To make the table human readable, Morse    */
/* sequences are given as strings, where DIT is '.', DAT is '-' and short     */
/* (intra-letter) pause is space.                                        */

const char* const MorseGen::morsedata[] =
{
	NULL,                    /* 32, ' ', no representation */
	"- . - . - -",           /* 33, '!' */
	". - . . - .",           /* 34, '"' */
	NULL,                    /* 35, '#', no representation */
	". . . - . . -",         /* 36, '$', non-standard */
	NULL,                    /* 37, '%', no representation */
	". - . . .",             /* 38, '&', non-standard */
	". - - - - .",           /* 39, ''' */
	"- . - - .",             /* 40, '(' */
	"- . - - . -",           /* 41, ')' */
	NULL,                    /* 42, '*', no representation */
	". - . - .",             /* 43, '+' */
	"- - . . - -",           /* 44, ',' */
	"- . . . . -",           /* 45, '-' */
	". - . - . -",           /* 46, '.' */
	"- . . - .",             /* 47, '/' */
	"- - - - -",             /* 48, '0' */
	". - - - -",             /* 49, '1' */
	". . - - -",             /* 50, '2' */
	". . . - -",             /* 51, '3' */
	". . . . -",             /* 52, '4' */
	". . . . .",             /* 53, '5' */
	"- . . . .",             /* 54, '6' */
	"- - . . .",             /* 55, '7' */
	"- - - . .",             /* 56, '8' */
	"- - - - .",             /* 57, '9' */
	"- - - . . .",           /* 58, ':' */
	"- . - . - .",           /* 59, ';' */
	NULL,                    /* 60, '<', no representation */
	"- . . . -",             /* 61, '=' */
	NULL,                    /* 62, '>', no representation */
	". . - - . .",           /* 63, '?' */
	". - - . - .",           /* 64, '@' */
	". -",                   /* 65, 'A' */
	"- . . .",               /* 66, 'B' */
	"- . - .",               /* 67, 'C' */
	"- . .",                 /* 68, 'D' */
	".",                     /* 69, 'E' */
	". . - .",               /* 70, 'F' */
	"- - .",                 /* 71, 'G' */
	". . . .",               /* 72, 'H' */
	". .",                   /* 73, 'I' */
	". - - -",               /* 74, 'J' */
	"- . -",                 /* 75, 'K' */
	". - . .",               /* 76, 'L' */
	"- -",                   /* 77, 'M' */
	"- .",                   /* 78, 'N' */
	"- - -",                 /* 80, 'O' */
	". - - .",               /* 81, 'P' */
	"- - . -",               /* 82, 'Q' */
	". - .",                 /* 83, 'R' */
	". . .",                 /* 84, 'S' */
	"-",                     /* 85, 'T' */
	". . -",                 /* 86, 'U' */
	". . . -",               /* 87, 'V' */
	". - -",                 /* 88, 'W' */
	"- . . -",               /* 89, 'X' */
	"- . - -",               /* 90, 'Y' */
	"- - . .",               /* 91, 'Z' */
	NULL,                    /* 92, '[', no representation */
	NULL,                    /* 93, '\', no representation */
	NULL,                    /* 94, ']', no representation */
	". . - - . -"            /* 95, '_', non-standard */
};	


const char* MorseGen::ToMorse(char ascii)
{
	if (ascii < 32) return NULL;
	if (ascii > 95) return NULL;
	return morsedata[ascii - 32];
}


void MorseGen::MorseLetter(const char *morse)
{
	char m;

	while (m = *morse++)
	{
		switch (m)
		{
			case ' ': IntraSymbolPause();  break;
			case '.': ShortTone();         break;
			case '-': LongTone();          break;
		}
	}
}


LONG MorseGen::MorseText(const char *text)
{
	char current_letter;
	LONG skipped = 0;
	BOOL start_of_word = TRUE;

	while (current_letter = *text++)
	{
		const char *morse;

		if (current_letter == ' ')
		{
			InterWordPause();
			start_of_word = TRUE;
		}
		else if (morse = ToMorse(ToUpper(current_letter)))
		{
			if (start_of_word) start_of_word = FALSE;
			else InterSymbolPause();
			MorseLetter(morse);
		}
		else skipped++;
	}
	
	return skipped;
}


MorseGen::~MorseGen() {}
