#include <proto/utility.h>
#include <exec/types.h>

#include "main.h"
#include "morsegen.h"

#include <proto/dos.h>  /* for tests */

/* Morse code contains no characters with ASCII less than 32, so indexes in   */
/* 'morse' table are (ASCII - 32). To make the table human readable, Morse    */
/* sequences are given as strings, where DIT is '.', DAT is '-' and short     */
/* (intra-letter) pause is just space.                                        */

const UBYTE* const Morse[] = {
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


/****i* morsegen/ToMorse() *************************************************
*
* NAME
*   ToMorse() -- Converts ASCII code to source Morse string 
*
* SYNOPSIS
*   string = ToMorse(code)
*   const UBYTE* ToMorse(UBYTE);
*
* FUNCTION
*   Returns Morse source string for passed ASCII code. NULL is returned for
*   ASCII codes not representable in Morse code. Codes lower than 32 and
*   higher than 95 are rejected in code. For others non-representable codes
*   Morse[] contains NULLs.
*
* INPUTS
*   code - ASCII code to be converted.
*
*  RESULT
*    Pointer to source Morse string. In such a string "dits" are represented
*    by '.', "dats" are represented by '-'. Intra-symbol pauses are
*    represented by spaces. Pointer may be NULL, if passed ASCII code has no
*    representation in Morse code.
* 
*  NOTES
*    - Small letters are treated as non-representable. Letters must be
*      capitalized before passing to this function.
*
******************************************************************************
*/

const UBYTE* ToMorse(UBYTE ascii)
{
	if (ascii < 32) return NULL;
	if (ascii > 95) return NULL;
	return Morse[ascii - 32];
}



/* Converts words per minute to dit length in miliseconds (based on CODEX,    */
/* 20 wpm = 50 msec unit).                                                    */

LONG WpmToMsec(LONG wpm)
{
	return SDivMod32(1000, wpm);
}


/****i* morsegen/MorseLetter() **********************************************
*
* NAME
*   MorseLetter() -- Emits a source Morse string 
*
* SYNOPSIS
*   MorseLetter(generator, morsestring)
*   VOID MorseLetter(struct MorseGen*, UBYTE*);
*
* FUNCTION
*   Emits a NULL-terminated source Morse string using callbacks of generator
*   specified. A dot in the string is emitted as one unit long tone. A dash
*   in the string is emitted as a three units long tone. A space in the
*   string is emitted as one unit long pause.
*
* INPUTS
*   generator - properly initialized MorseGen structure. NULL is not checked
*     and will be fatal.
*   morsestring - source string of a letter (using '.', '-' and space). NULL
*     is not checked and will be fatal.
*
*  RESULT
*    None.
*
******************************************************************************
*/

void MorseLetter(struct MorseGen *mg, const UBYTE *morse)
{
	UBYTE m;

	while (m = *morse++)
	{
		switch (m)
		{
			case ' ': mg->mg_IntraSymbolPause(mg);  break;
			case '.': mg->mg_ShortTone(mg);         break;
			case '-': mg->mg_LongTone(mg);          break;
		}
	}
}


/****i* morsegen/MorseText() ************************************************
*
* NAME
*   MorseText() -- Emits an ASCII string as Morse  
*
* SYNOPSIS
*   skipped = MorseText(generator, text)
*   ULONG MorseText(struct MorseGen*, STRPTR);
*
* FUNCTION
*   A string is emitted as Morse code using specified generator. The string
*   is treated as a series of words separated by spaces. All the characters
*   except the first one in each word are prefixed by inter-symbol pause.
*   Space character generates inter-word pause. ToUpper() from utility.library
*   is called on every character. Then characters not representable in Morse
*   code are skipped, counter of skipped characters is updated.
*
* INPUTS
*   generator - properly initialized MorseGen structure. NULL is not checked
*     and will be fatal.
*   text - NULL-terminated string. NULL is not checked and will be fatal.
*
*  RESULT
*    Count of skipped symbols (having no representation in Morse code).
*
******************************************************************************
*/

LONG MorseText(struct MorseGen *mg, STRPTR text)
{
	UBYTE current_letter;
	LONG skipped = 0;
	BOOL start_of_word = TRUE;

	while (current_letter = *text++)
	{
		const UBYTE *morse;

		if (current_letter == ' ')
		{
			mg->mg_InterWordPause(mg);
			start_of_word = TRUE;
		}
		else if (morse = ToMorse(ToUpper(current_letter)))
		{
			if (start_of_word) start_of_word = FALSE;
			else mg->mg_InterSymbolPause(mg);
			MorseLetter(mg, morse);
		}
		else skipped++;
	}
	
	return skipped;
}


/****i* morsegen/MorseGenSetup() *********************************************
*
* NAME
*   MorseGenSetup() -- prepare generator and initialize parameters 
*
* SYNOPSIS
*   success = MorseGenSetup(generator, tag, ...)
*   BOOL MorseGenSetup(struct MorseGen*, Tag, ...);
*
* FUNCTION
*   Prepares Morse generator to run. Allocates resources needed. Sets
*   parameters of the output according to passed tags. Function creates a
*   taglist from variadic arguments, then calls mg_Setup() of the generator
*   with the taglist.
*
* INPUTS
*   generator - MorseGen structure.
*   tag - inlined TagList of parameters.
*
*  RESULT
*    Result of called mg_Setup() is returned.
*
******************************************************************************
*/

BOOL MorseGenSetup(struct MorseGen *mg, Tag tag0, ...)
{
	struct TagItem *taglist = (struct TagItem*)&tag0;
	
	return mg->mg_Setup(mg, taglist);
}