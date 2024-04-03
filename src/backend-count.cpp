#include <proto/dos.h>
#include <proto/utility.h>

#include "main.h"
#include "backend-count.h"


CountMorseGen::CountMorseGen(short print, long *metrics) : MorseGen()
{
	Dots = 0;
	Dashes = 0;
	SymbolPauses = 0;
	CharPauses = 0;
	WordPauses = 0;
	Storage = metrics;
	Print = print;
	Initialized = TRUE;
};


CountMorseGen::~CountMorseGen()
{
	if (Storage)
	{
		Storage[COUNTER_DOTS] = Dots;
		Storage[COUNTER_DASHES] = Dashes;
		Storage[COUNTER_SYMBOL_PAUSES] = SymbolPauses;
		Storage[COUNTER_CHAR_PAUSES] = CharPauses;
		Storage[COUNTER_WORD_PAUSES] = WordPauses;
	}
	
	if (Print)
	{
		Printf("%ld %ld %ld %ld %ld\n", Dots, Dashes, SymbolPauses, CharPauses, WordPauses);
	}
}
