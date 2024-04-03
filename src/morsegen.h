#ifndef MORSCONV_MORSEGEN_H
#define MORSCONV_MORSEGEN_H

#include <utility/tagitem.h>

class MorseGen
{
	static const char* const morsedata[];
	const char* ToMorse(char ascii);
	void MorseLetter(const char *morse);

	public:

	short Initialized;
	MorseGen() { Initialized = FALSE; }
	virtual void IntraSymbolPause() = 0;
	virtual void InterSymbolPause() = 0;
	virtual void InterWordPause() = 0;
	virtual void ShortTone() = 0;
	virtual void LongTone() = 0;
	long MorseText(const char *text);
	virtual ~MorseGen();
};

/* indexes for the metrics array (count module) */

#define COUNTER_DOTS              0
#define COUNTER_DASHES            1
#define COUNTER_SYMBOL_PAUSES     2
#define COUNTER_CHAR_PAUSES       3
#define COUNTER_WORD_PAUSES       4

#endif    /* MORSCONV_MORSEGEN_H */
