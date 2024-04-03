#include <proto/dos.h>
#include "morsegen.h"


class CountMorseGen : public MorseGen
{
	int Dots;
	int Dashes;
	int SymbolPauses;
	int CharPauses;
	int WordPauses;
	long *Storage;
	short Print;

	public:

	CountMorseGen(short print, long *metrics);
	void IntraSymbolPause()	{ SymbolPauses++; };
	void InterSymbolPause() { CharPauses++; };
	void InterWordPause() { WordPauses++; };
	void ShortTone() { Dots++; };
	void LongTone() { Dashes++; };
	~CountMorseGen();
};
