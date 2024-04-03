#include <proto/dos.h>
#include <utility/tagitem.h>

#include "main.h"
#include "morsegen.h"

class StdOutMorseGen : public MorseGen
{
	char *ShortString;
	char *LongString;
	char *SymbolPauseString;
	char *CharPauseString;
	char *WordPauseString;

	void Unescape(char *s);

	public:

	StdOutMorseGen(char* shstr, char* lnstr, char* spstr, char *cpstr, char *wpstr);
	void IntraSymbolPause() { PutStr(SymbolPauseString); }
	void InterSymbolPause() { PutStr(CharPauseString); }
	void InterWordPause() { PutStr(WordPauseString); }
	void ShortTone() { PutStr(ShortString); }
	void LongTone() { PutStr(LongString); }
	~StdOutMorseGen() {	WriteChars("\n", 1); }
};
