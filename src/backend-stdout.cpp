#include "backend-stdout.h"

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

/*--------------------------------------------------------------------------------------------*/

StdOutMorseGen::StdOutMorseGen(char* shstr, char* lnstr, char* spstr, char *cpstr, char *wpstr)
: MorseGen()
{
	ShortString = shstr;
	LongString = lnstr;
	SymbolPauseString = spstr;
	CharPauseString = cpstr;
	WordPauseString = wpstr;
	Unescape(ShortString);
	Unescape(LongString);
	Unescape(SymbolPauseString);
	Unescape(CharPauseString);
	Unescape(WordPauseString);
	Initialized = TRUE;
}

/*--------------------------------------------------------------------------------------------*/
/* Replaces escape sequences with characters.                                                 */
/* %n -> $0A                                                                                  */ 
/* %t -> $09                                                                                  */
/* %% -> %                                                                                    */
/* Unescaped string is always shorter, so it may be done in-place.                            */
/* Unknown escape sequences are discarded completely.                                         */
/*--------------------------------------------------------------------------------------------*/

void StdOutMorseGen::Unescape(char *s)
{
	STRPTR d = s;
	UBYTE c;

	while (c = *s++)
	{
		if (c == '%')
		{
			switch (*s++)
			{
				case 'n':   *d++ = 0x0A;   break;
				case 't':   *d++ = 0x09;   break;
				case '%':   *d++ = '%';    break;
			}
		}
		else *d++ = c;
	}

	*d = 0x00;
}
